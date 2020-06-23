/* This file is part of ESDM.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESDM.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This test uses the ESDM high-level API to actually write a contiuous ND subset of a data set
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mpi.h>
#include <esdm-internal.h>
#include <esdm-mpi.h>
#include "util/test_util.h"

typedef struct {
  char* varname;
  int64_t dimCount, timeDim;
  int64_t* offset;
  int64_t* size;
  int64_t* timestepSize;
  int64_t* fragmentSize;
} instruction_t;

typedef struct {
  timer t;
  double dataHandling, init, io, cleanup, mpi, metadataSync;
  esdm_readTimes_t readTimesStart, readTimesEnd;
  esdm_writeTimes_t writeTimesStart, writeTimesEnd;
} ioTimer;

//checkedScan() is a wrapper for fscanf() which returns true if, and only if the entire format string was matched successfully.
#define simpleCheckedScan(stream, format) checkedScan_internal(stream, format "%n", &checkedScan_success)
#define checkedScan(stream, format, ...) checkedScan_internal(stream, format "%n", __VA_ARGS__, &checkedScan_success)

//Implementation of checkedScan()
//
//The address of the success variable needs to be attached to the end of the parameters pack that's passed through to vfscanf().
//Since there is no way to add something to a va_list, I'm using a global variable which can be attached to the parameter pack by the preprocessor.
//
//This is not thread safe. It could be made thread safe by turning checkedScan_success into a thread-local variable.
//That would be easy in C11 (use the `thread_local` keyword), but I really don't want to add a C version check here.
static int checkedScan_success = 0;
__attribute__((format(scanf, 2, 3)))
bool checkedScan_internal(FILE* stream, const char* format, ...) {
  va_list args;
  va_start(args, format);

  checkedScan_success = -1;
  vfscanf(stream, format, args);

  va_end(args);
  return checkedScan_success >= 0;
}

//Vector<DimCount> ::= '(' ( <integer> ',' {DimCount-1} <integer> ')'
void parseVector(FILE* stream, int64_t dimCount, int64_t** out_vector) {
  int64_t* vector = *out_vector = ea_checked_malloc(dimCount*sizeof*vector);
  for(int i = 0; i < dimCount; i++) {
    bool success = false;
    switch(2*!i + (i == dimCount - 1)) {  //+2 = open parenthesis is needed, +1 = close parenthesis is needed
      case 0: success = checkedScan(stream, " %"SCNd64" ,", &vector[i]); break;
      case 1: success = checkedScan(stream, " %"SCNd64" )", &vector[i]); break;
      case 2: success = checkedScan(stream, " ( %"SCNd64" ,", &vector[i]); break;
      case 3: success = checkedScan(stream, " ( %"SCNd64" )" , &vector[i]); break;
    }
    if(!success) fprintf(stderr, "error reading coordinate %d of vector\n", i + 1), abort();
  }
}

//The instructions format is as follows:
//
//  * Whitespace between tokens is ignored.
//
//  * But '\n' terminates instructions.
//
//  * {*} means that everything to the left appears zero or more times, {N} means exactly N occurrences of everything to the left.
//
//  * Syntax:
//
//        Instructions ::= Instruction '\n' {*}
//        Instruction ::= VarName '(' DimCount [ ',' TimeSpecification ] '):' Offset<DimCount> Size<DimCount> FragmentSize<DimCount>
//        VarName ::= <word>
//        DimCount ::= <unsigned integer>
//        TimeSpecification ::= 't=' <unsigned integer>
//        Offset<DimCount> ::= Vector<DimCount>
//        Size<DimCount> ::= Vector<DimCount>
//        FragmentSize<DimCount> ::= Vector<DimCount>
//        Vector<DimCount> ::= '(' ( <integer> ',' {DimCount-1} <integer> ')'
//
//A legal instruction String would be:
//    foo(2): (1,2) (3,4) (5,6)
//    bar(3, t = 1): (7, 8, 9) (10, 11, 12) (13, 1, 15)
//The time specification gives the index of the time dimension (zero based), the corresponding fragment size dimension must be 1.
instruction_t* parseInstructions(FILE* instructions, size_t* out_instructionCount) {
  size_t instructionCount = 0, allocatedInstructions = 8;
  instruction_t* instructionList = ea_checked_malloc(allocatedInstructions*sizeof(*instructionList));

  char* line = NULL;
  size_t bufferSize = 0;
  ssize_t lineLength;

  while(0 < (lineLength = getline(&line, &bufferSize, instructions))) {
    if(!strlen(line)) continue; //ignore empty lines

    if(instructionCount == allocatedInstructions) {
      instructionList = ea_checked_realloc(instructionList, (allocatedInstructions *= 2)*sizeof(*instructionList));
      eassert(instructionList);
    }
    eassert(instructionCount < allocatedInstructions);
    instruction_t* newInstruction = & instructionList[instructionCount++];
    *newInstruction = (instruction_t){.timeDim = -1};

    FILE* lineStream = fmemopen(line, lineLength, "r");
    //parse the varname and dimension count
    bool success = checkedScan(lineStream, " %m[^( \t\f\n] ( %"SCNd64, &newInstruction->varname, &newInstruction->dimCount);
    if(!success) fprintf(stderr, "error parsing instructions: couldn't recognize variable name and dimension count prefix in the line:\n%s\n", line), abort();

    //parse the time specification, if present
    if(simpleCheckedScan(lineStream, " ,")) {
      //there seems to be a time specification here, parse it
      success = checkedScan(lineStream, " t = %"SCNd64, &newInstruction->timeDim);
      if(!success) fprintf(stderr, "error parsing time specification of variable '%s'\n", newInstruction->varname), abort();
      if(newInstruction->timeDim < 0) fprintf(stderr, "error: the time dimension of variable '%s' is negative\n", newInstruction->varname), abort();
      if(newInstruction->timeDim >= newInstruction->dimCount) {
        fprintf(stderr, "error: the time dimension of variable '%s' is out of range, a zero based dimension index is required\n", newInstruction->varname);
        abort();
      }
    }
    if(!simpleCheckedScan(lineStream, " ) :")) fprintf(stderr, "error parsing instructions: missing '):' token sequence\n"), abort();

    //parse the shape of the dataset
    parseVector(lineStream, newInstruction->dimCount, &newInstruction->offset);
    parseVector(lineStream, newInstruction->dimCount, &newInstruction->size);
    parseVector(lineStream, newInstruction->dimCount, &newInstruction->fragmentSize);
    newInstruction->timestepSize = ea_checked_malloc(newInstruction->dimCount*sizeof*newInstruction->timestepSize);
    memcpy(newInstruction->timestepSize, newInstruction->size, newInstruction->dimCount*sizeof*newInstruction->timestepSize);

    //adjust the timestepSize and check that the fragment size is 1 in the time dimension (if present)
    if(newInstruction->timeDim >= 0) {
      newInstruction->timestepSize[newInstruction->timeDim] = 1;
      if(newInstruction->fragmentSize[newInstruction->timeDim] != 1) {
        fprintf(stderr, "error: the fragment size of the time dimension (%"PRId64" of variable %s is not 1\n", newInstruction->timeDim, newInstruction->varname);
        abort();
      }
    }
  }
  free(line);

  *out_instructionCount = instructionCount;
  return instructionList;
}

//Also frees the instruction list itself.
void deleteInstructions(size_t instructionCount, instruction_t* instructions) {
  for(size_t i = instructionCount; i--; ) {
    free(instructions[i].varname);
    free(instructions[i].offset);
    free(instructions[i].size);
    free(instructions[i].timestepSize);
    free(instructions[i].fragmentSize);
  }
  free(instructions);
}

void generateFragmentList(instruction_t* instruction, int64_t dimCount, int64_t (**out_fragmentOffsets)[dimCount], int64_t* out_fragmentCount) {
  //determine the total count of fragments
  int64_t totalFragmentCount = 1, fragmentCounts[dimCount];
  for(int64_t i = dimCount; i--;) {
    fragmentCounts[i] = (instruction->timestepSize[i] + instruction->fragmentSize[i] - 1)/instruction->fragmentSize[i];
    totalFragmentCount *= fragmentCounts[i];
  }

  //determine our own share of fragments
  int rank, procCount;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procCount);
  int64_t startFragment = rank*totalFragmentCount/procCount;
  int64_t stopFragment = (rank + 1)*totalFragmentCount/procCount;
  *out_fragmentCount = stopFragment - startFragment;
  int64_t (*fragmentOffsets)[dimCount] = *out_fragmentOffsets = ea_checked_malloc(*out_fragmentCount*sizeof*fragmentOffsets);

  //compute the offsets of the fragments
  for(int64_t fragment = stopFragment; fragment-- > startFragment; ) {
    int64_t fragmentIndex = fragment - startFragment;
    for(int64_t dim = dimCount, i = fragment; dim--; i /= fragmentCounts[dim]) {
      fragmentOffsets[fragmentIndex][dim] = instruction->offset[dim] + i%fragmentCounts[dim] * instruction->fragmentSize[dim];
    }
  }
}

//*out_size is a preallocated array into which the actual dimensions of the fragment are written,
//this may be smaller than *size indicates due to clipping the fragment to the limit.
//offset, size, limit, and out_size point to arrays of size dimCount.
//out_data is used to return a newly malloc'ed array with the fragment's data.
void getFragmentShape(instruction_t* instruction, int64_t* fragmentOffset, int64_t* out_size) {
  for(int64_t i = instruction->dimCount; i--; ) {
    int64_t limit = fragmentOffset[i] + instruction->fragmentSize[i];
    if(limit > instruction->offset[i] + instruction->timestepSize[i]) limit = instruction->offset[i] + instruction->timestepSize[i];
    out_size[i] = limit - fragmentOffset[i];
  }
}

int64_t encodeCoords(int64_t dimCount, int64_t* offset, int64_t *size, int64_t localIndex) {
  int64_t encodedValue = 0;
  //We encode the coords into fixed width bitfields of size 8.
  //That way, the result is independent of the overall shape of the dataset, we retain a decent collision resistance, and we retain a certain human interpretability of the resulting data.
  for(int64_t dim = dimCount; dim--; localIndex /= size[dim]) {
    int64_t coord = localIndex%size[dim] + offset[dim];  //Extract the value of this coordinate from the localIndex ...
    encodedValue = (encodedValue << 8) | (coord & 0xff);
  }
  return encodedValue;
}

__attribute__((unused))
void printVector(FILE* stream, int64_t dimCount, int64_t* vector) {
  fprintf(stream, "(");
  for(int64_t i = 0; i < dimCount; i++) fprintf(stream, "%s%"PRId64, i ? ", " : "", vector[i]);
  fprintf(stream, ")");
}

void generateFragmentData(int64_t dimCount, int64_t* offset, int64_t* size, int64_t *globalSize, int64_t **out_data) {
  //determine the count of datapoints in the fragment
  int64_t datapoints = 1;
  for(int64_t i = dimCount; i--; ) datapoints *= size[i];

  //create the fragment's data
  int64_t* data = *out_data = ea_checked_malloc(datapoints*sizeof*data);
  for(int64_t i = datapoints; i--; ) data[i] = encodeCoords(dimCount, offset, size, i);
}

void checkFragmentData(int64_t dimCount, int64_t* offset, int64_t* size, int64_t *globalSize, int64_t *data) {
  //determine the count of datapoints in the fragment
  int64_t datapoints = 1;
  for(int64_t i = dimCount; i--; ) datapoints *= size[i];

  //check the fragment's data
  for(int64_t i = datapoints; i--; ) {
    if(data[i] != encodeCoords(dimCount, offset, size, i)) {
      fprintf(stderr, "wrong data detected\n");
      abort();
    }
  }
}

bool isOutputTimestep(instruction_t* instruction, int64_t timestep) {
  int64_t timeDim = instruction->timeDim;
  if(timeDim < 0) {
    return !timestep;  //run output on no-time variables in timestep 0
  } else {
    int64_t offset = instruction->offset[timeDim];
    int64_t size = instruction->size[timeDim];
    return timestep >= offset && timestep < offset + size;
  }
}

void writeVariableTimestep(instruction_t* instruction, esdm_dataset_t* dataset, esdm_dataspace_t* dataspace, int64_t timestep, ioTimer* times) {
  if(!isOutputTimestep(instruction, timestep)) return;

  //set the time coord
  bool haveTime = instruction->timeDim >= 0;
  int64_t savedTimeOffset, savedTimeSize;
  if(haveTime) {
    savedTimeOffset = instruction->offset[instruction->timeDim];
    instruction->offset[instruction->timeDim] = timestep;
  }

  //write the data
  int64_t (*fragmentOffsets)[instruction->dimCount];
  int64_t fragmentCount;
  generateFragmentList(instruction, instruction->dimCount, &fragmentOffsets, &fragmentCount);

  for(int64_t i = fragmentCount; i--; ) {
    times->dataHandling -= ea_stop_timer(times->t);

    int64_t* data = NULL, fragmentSize[instruction->dimCount];
    getFragmentShape(instruction, fragmentOffsets[i], fragmentSize);
    generateFragmentData(instruction->dimCount, fragmentOffsets[i], fragmentSize, instruction->size, &data);

    double curTime = ea_stop_timer(times->t);
    times->dataHandling += curTime;
    times->io -= curTime;

    esdm_dataspace_t *subspace;
    esdm_status ret = esdm_dataspace_subspace(dataspace, instruction->dimCount, fragmentSize, fragmentOffsets[i], &subspace);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_write(dataset, data, subspace);
    eassert(ret == ESDM_SUCCESS);

    curTime = ea_stop_timer(times->t);
    times->io += curTime;
    times->cleanup -= curTime;

    ret = esdm_dataspace_destroy(subspace);
    eassert(ret == ESDM_SUCCESS);
    free(data);

    times->cleanup += ea_stop_timer(times->t);
  }

  free(fragmentOffsets);

  //restore the time dim
  if(haveTime) {
    instruction->offset[instruction->timeDim] = savedTimeOffset;
  }
}

void writeTimestep(size_t instructionCount, instruction_t* instructions, esdm_dataset_t** datasets, esdm_dataspace_t** dataspaces, int64_t timestep, ioTimer* times) {
  for(size_t i = 0; i != instructionCount; i++) {
    writeVariableTimestep(&instructions[i], datasets[i], dataspaces[i], timestep, times);
  }
}

void printTimes(ioTimer* times, int64_t totalBytes, const char* operationName, const char* dataHandlingTitle) {
  int rank, procCount;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procCount);

  //collect the measurement data at the root process
  times->readTimesEnd = esdmI_performance_read();
  times->writeTimesEnd = esdmI_performance_write();
  ioTimer* collectedTimes = rank ? NULL : ea_checked_malloc(procCount*sizeof*collectedTimes);
  MPI_Gather(times, sizeof*times, MPI_BYTE, collectedTimes, sizeof*collectedTimes, MPI_BYTE, 0, MPI_COMM_WORLD);

  if(!rank) {
    //print a table with the raw measurements
    printf("%s:\n", operationName);
    //      1234567 | 1234567   1234567   1234567   1234567 | 1234567   1234567
    printf("  proc  |  init       io      cleanup     sync  |   MPI     %s\n", dataHandlingTitle);
    printf("--------+-----------------------------+------------------\n");
    for(int i = 0; i < procCount; i++) {
      printf("%7d | %6.3fs   %6.3fs   %6.3fs   %6.3fs | %6.3fs   %6.3fs\n", i, collectedTimes[i].init, collectedTimes[i].io, collectedTimes[i].cleanup, collectedTimes[i].metadataSync, collectedTimes[i].mpi, collectedTimes[i].dataHandling);
    }

    //print the performance summary
    double totalTime = 0;
    esdm_readTimes_t esdmTimesRead = {0};
    esdm_writeTimes_t esdmTimesWrite = {0};
    for(int i = procCount; i--; ) {
      double procTime = collectedTimes[i].io + collectedTimes[i].cleanup + collectedTimes[i].metadataSync;
      if(procTime > totalTime) totalTime = procTime;

      esdmTimesWrite.backendDistribution += collectedTimes[i].writeTimesEnd.backendDistribution - collectedTimes[i].writeTimesStart.backendDistribution;
      esdmTimesWrite.backendDispatch += collectedTimes[i].writeTimesEnd.backendDispatch - collectedTimes[i].writeTimesStart.backendDispatch;
      esdmTimesWrite.completion += collectedTimes[i].writeTimesEnd.completion - collectedTimes[i].writeTimesStart.completion;
      esdmTimesWrite.total += collectedTimes[i].writeTimesEnd.total - collectedTimes[i].writeTimesStart.total;

      esdmTimesRead.makeSet += collectedTimes[i].readTimesEnd.makeSet - collectedTimes[i].readTimesStart.makeSet;
      esdmTimesRead.coverageCheck += collectedTimes[i].readTimesEnd.coverageCheck - collectedTimes[i].readTimesStart.coverageCheck;
      esdmTimesRead.enqueue += collectedTimes[i].readTimesEnd.enqueue - collectedTimes[i].readTimesStart.enqueue;
      esdmTimesRead.completion += collectedTimes[i].readTimesEnd.completion - collectedTimes[i].readTimesStart.completion;
      esdmTimesRead.writeback += collectedTimes[i].readTimesEnd.writeback - collectedTimes[i].readTimesStart.writeback;
      esdmTimesRead.total += collectedTimes[i].readTimesEnd.total - collectedTimes[i].readTimesStart.total;
    }

    printf("\nESDM internal measurements:\n");
    if(esdmTimesWrite.total > 0.0) {
      printf("\twrite:\n");
      printf("\t\tbackendDistribution: %.3fs\n", esdmTimesWrite.backendDistribution);
      printf("\t\tbackendDispatch: %.3fs\n", esdmTimesWrite.backendDispatch);
      printf("\t\tcompletion: %.3fs\n", esdmTimesWrite.completion);
      printf("\t\ttotal: %.3fs\n", esdmTimesWrite.total);
    }
    if(esdmTimesRead.total > 0.0) {
      printf("\tread:\n");
      printf("\t\tmakeSet: %.3fs\n", esdmTimesRead.makeSet);
      printf("\t\tcoverageCheck: %.3fs\n", esdmTimesRead.coverageCheck);
      printf("\t\tenqueue: %.3fs\n", esdmTimesRead.enqueue);
      printf("\t\tcompletion: %.3fs\n", esdmTimesRead.completion);
      printf("\t\twriteback: %.3fs\n", esdmTimesRead.writeback);
      printf("\t\ttotal: %.3fs\n", esdmTimesRead.total);
    }

    printf("\nPerformance Summary: I/O of %.0fMiB in %.3fs = %.3f MiB/s\n\n", totalBytes/1024.0/1024, totalTime, totalBytes/1024.0/1024/totalTime);
  }
  free(collectedTimes);
  MPI_Barrier(MPI_COMM_WORLD);
}

void benchmarkWrite(size_t instructionCount, instruction_t* instructions) {
  //determine the count of time steps to write
  int64_t timeLimit = 1;       //we always handle a timestep 0 to accomodate the non-time variables
  for(size_t i = instructionCount; i--; ) {
    if(instructions[i].timeDim >= 0) {
      int64_t limit = instructions[i].offset[instructions[i].timeDim] + instructions[i].size[instructions[i].timeDim];
      timeLimit = timeLimit > limit ? timeLimit : limit;
    }
  }

  //create the datasets
  MPI_Barrier(MPI_COMM_WORLD);
  ioTimer times = {
    .readTimesStart = esdmI_performance_read(),
    .writeTimesStart = esdmI_performance_write()
  };
  ea_start_timer(&times.t);
  esdm_container_t *container = NULL;
  esdm_status ret = esdm_mpi_container_create(MPI_COMM_WORLD, "mycontainer", true, &container);
  eassert(ret == ESDM_SUCCESS);
  esdm_dataspace_t* spaces[instructionCount];
  esdm_dataset_t* sets[instructionCount];
  int64_t totalBytes = 0;
  for(size_t i = instructionCount; i--; ) {
    int64_t bounds[instructions[i].dimCount]; //unfortunately, we don't have a primitive to directly create a dataspace with an offset
    int64_t datapointCount = 1;
    for(int64_t dim = instructions[i].dimCount; dim--; ) {
      bounds[dim] = instructions[i].offset[dim] + instructions[i].size[dim];
      datapointCount *= instructions[i].size[dim];
    }
    totalBytes += datapointCount*sizeof(int64_t);

    esdm_dataspace_t *dummy;
    ret = esdm_dataspace_create(instructions[i].dimCount, bounds, SMD_DTYPE_UINT64, &dummy);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_dataspace_subspace(dummy, instructions[i].dimCount, instructions[i].size, instructions[i].offset, &spaces[i]);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_dataspace_destroy(dummy);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_mpi_dataset_create(MPI_COMM_WORLD, container, instructions[i].varname, spaces[i], &sets[i]);
    eassert(ret == ESDM_SUCCESS);
  }
  times.init = ea_stop_timer(times.t);

  //perform the actual writes
  for(int64_t t = 0; t < timeLimit; t++) writeTimestep(instructionCount, instructions, sets, spaces, t, &times);
  times.mpi -= ea_stop_timer(times.t);
  MPI_Barrier(MPI_COMM_WORLD);
  double time = ea_stop_timer(times.t);
  times.mpi += time;
  times.metadataSync -= time;

  //commit the changes to data to the metadata
  for(size_t i = instructionCount; i--; ) {
    ret = esdm_mpi_dataset_commit(MPI_COMM_WORLD, sets[i]);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_dataset_close(sets[i]);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_dataspace_destroy(spaces[i]);
    eassert(ret == ESDM_SUCCESS);
  }
  ret = esdm_mpi_container_commit(MPI_COMM_WORLD, container);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_close(container);
  eassert(ret == ESDM_SUCCESS);
  MPI_Barrier(MPI_COMM_WORLD);
  times.metadataSync += ea_stop_timer(times.t);

  //determine our performance
  printTimes(&times, totalBytes, "Write", "data generation");
}

void readVariableTimestep(instruction_t* instruction, esdm_dataset_t* dataset, esdm_dataspace_t* dataspace, int64_t timestep, ioTimer* times) {
  if(!isOutputTimestep(instruction, timestep)) return;

  //set the time coord
  bool haveTime = instruction->timeDim >= 0;
  int64_t savedTimeOffset, savedTimeSize;
  if(haveTime) {
    savedTimeOffset = instruction->offset[instruction->timeDim];
    instruction->offset[instruction->timeDim] = timestep;
  }

  //read the data
  int64_t (*fragmentOffsets)[instruction->dimCount];
  int64_t fragmentCount;
  generateFragmentList(instruction, instruction->dimCount, &fragmentOffsets, &fragmentCount);

  for(int64_t i = fragmentCount; i--; ) {
    times->dataHandling -= ea_stop_timer(times->t);

    int64_t fragmentSize[instruction->dimCount];
    getFragmentShape(instruction, fragmentOffsets[i], fragmentSize);
    int64_t datapointCount = 1;
    for(int64_t dim = instruction->dimCount; dim--; ) datapointCount *= fragmentSize[dim];
    int64_t* data = ea_checked_malloc(datapointCount*sizeof*data);

    double curTime = ea_stop_timer(times->t);
    times->dataHandling += curTime;
    times->io -= curTime;

    esdm_dataspace_t *subspace;
    esdm_status ret = esdm_dataspace_subspace(dataspace, instruction->dimCount, fragmentSize, fragmentOffsets[i], &subspace);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_read(dataset, data, subspace);
    eassert(ret == ESDM_SUCCESS);

    curTime = ea_stop_timer(times->t);
    times->io += curTime;
    times->dataHandling -= curTime;

    checkFragmentData(instruction->dimCount, fragmentOffsets[i], fragmentSize, instruction->size, data);

    curTime = ea_stop_timer(times->t);
    times->dataHandling += curTime;
    times->cleanup -= curTime;

    ret = esdm_dataspace_destroy(subspace);
    eassert(ret == ESDM_SUCCESS);
    free(data);

    times->cleanup += ea_stop_timer(times->t);
  }

  free(fragmentOffsets);

  //restore the time dim
  if(haveTime) {
    instruction->offset[instruction->timeDim] = savedTimeOffset;
  }
}

void readTimestep(size_t instructionCount, instruction_t* instructions, esdm_dataset_t** datasets, esdm_dataspace_t** dataspaces, int64_t timestep, ioTimer* times) {
  for(size_t i = 0; i != instructionCount; i++) {
    readVariableTimestep(&instructions[i], datasets[i], dataspaces[i], timestep, times);
  }
}

void benchmarkRead(size_t instructionCount, instruction_t* instructions) {
  //determine the count of time steps to read
  int64_t timeLimit = 1;       //we always handle a timestep 0 to accomodate the non-time variables
  for(size_t i = instructionCount; i--; ) {
    if(instructions[i].timeDim >= 0) {
      int64_t limit = instructions[i].offset[instructions[i].timeDim] + instructions[i].size[instructions[i].timeDim];
      timeLimit = timeLimit > limit ? timeLimit : limit;
    }
  }

  //open the datasets
  MPI_Barrier(MPI_COMM_WORLD);
  ioTimer times = {
    .readTimesStart = esdmI_performance_read(),
    .writeTimesStart = esdmI_performance_write()
  };
  ea_start_timer(&times.t);
  esdm_container_t *container = NULL;
  esdm_status ret = esdm_mpi_container_open(MPI_COMM_WORLD, "mycontainer", false, &container);
  eassert(ret == ESDM_SUCCESS);

  esdm_dataspace_t* spaces[instructionCount];
  esdm_dataset_t* sets[instructionCount];
  int64_t totalBytes = 0;
  for(size_t i = instructionCount; i--; ) {
    int64_t datapointCount = 1;
    for(int64_t dim = instructions[i].dimCount; dim--; ) datapointCount *= instructions[i].size[dim];
    totalBytes += datapointCount*sizeof(int64_t);

    //FIXME: The execution time of this call is totally dominated by the rebuilding of the fragments list from metadata.
    //       Save the fragment neighbourhood information to disk for fast loading.
    ret = esdm_mpi_dataset_open(MPI_COMM_WORLD, container, instructions[i].varname, &sets[i]);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_dataset_get_dataspace(sets[i], &spaces[i]);
    eassert(ret == ESDM_SUCCESS);
  }
  times.init = ea_stop_timer(times.t);

  //perform the actual reads
  for(int64_t t = 0; t < timeLimit; t++) readTimestep(instructionCount, instructions, sets, spaces, t, &times);
  times.mpi -= ea_stop_timer(times.t);
  MPI_Barrier(MPI_COMM_WORLD);
  double time = ea_stop_timer(times.t);
  times.mpi += time;
  times.metadataSync -= time;

  //close the ESDM objects
  for(size_t i = instructionCount; i--; ) {
    ret = esdm_dataset_close(sets[i]);
    eassert(ret == ESDM_SUCCESS);
  }
  ret = esdm_container_close(container);
  eassert(ret == ESDM_SUCCESS);
  MPI_Barrier(MPI_COMM_WORLD);
  times.metadataSync += ea_stop_timer(times.t);

  //determine our performance
  printTimes(&times, totalBytes, "Read", "data checking");
}

__attribute__((noreturn))
void exitWithUsage(const char* execName, const char* errorMessage, int exitStatus) {
  if(errorMessage) fprintf(stderr, "%s", errorMessage);
  printf("usage: %s [(-r | -w | -c | --read | --write | --config) path]...\n", execName);
  printf("\n");
  printf("\t-r | --read path\n");
  printf("\tPerform the read benchmark with the instructions provided by the file at path.\n");
  printf("\n");
  printf("\t-w | --write path\n");
  printf("\tPerform the write benchmark with the instructions provided by the file at path.\n");
  printf("\n");
  printf("\t-c | --config path\n");
  printf("\tUse the file at path as the ESDM configuration file.\n");

  exit(exitStatus);
}

void parseCommandlineArgs(int argc, char** argv, char** configFile, FILE** writeInstructionFile, FILE** readInstructionFile) {
  if(argc == 1) {
    //special handling of the zero argument case to ensure that this benchmark plays nicely as a test
    static char defaultInstructionsWrite[] =
      "mydataset1(3, t = 0): (0, 0, 0) (10, 1024, 1024) (1, 256, 1024)\n"
      "mydataset2(3, t = 1): (0, 0, 0) (10, 10, 10) (10, 1, 10)\n"
      "mydataset3(3): (0, 0, 0) (10, 10, 10) (10, 10, 10)\n";
    static char defaultInstructionsRead[] =
      "mydataset1(3, t = 0): (0, 0, 0) (10, 1024, 1024) (1, 256, 1024)\n"
      "mydataset2(3): (0, 0, 0) (10, 10, 10) (10, 10, 10)\n"
      "mydataset3(3, t = 1): (0, 0, 0) (10, 10, 10) (10, 1, 10)\n";
    *writeInstructionFile = fmemopen(defaultInstructionsWrite, sizeof(defaultInstructionsWrite) - 1, "r");  //-1 for the length to avoid passing the terminating null byte to getline()
    *readInstructionFile = fmemopen(defaultInstructionsRead, sizeof(defaultInstructionsRead) - 1, "r");
    return;
  }

  *writeInstructionFile = *readInstructionFile = NULL;
  for(int i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-w") || !strcmp(argv[i], "--write")) {
      if(++i >= argc) exitWithUsage(argv[0], "error: -w | --write option must have an argument\n", 1);
      if(*writeInstructionFile) fprintf(stderr, "error: only a single -w | --write option is allowed\n"), exit(2);
      if(!(*writeInstructionFile = fopen(argv[i], "r"))) fprintf(stderr, "error opening file at \"%s\"\n", argv[i]), exit(3);
    } else if(!strcmp(argv[i], "-r") || !strcmp(argv[i], "--read")) {
      if(++i >= argc) exitWithUsage(argv[0], "error: -r | --read option must have an argument\n", 4);
      if(*readInstructionFile) fprintf(stderr, "error: only a single -r | --read option is allowed\n"), exit(5);
      if(!(*readInstructionFile = fopen(argv[i], "r"))) fprintf(stderr, "error opening file at \"%s\"\n", argv[i]), exit(6);
    } else if(!strcmp(argv[i], "-c") || !strcmp(argv[i], "--config")) {
      if(++i >= argc) exitWithUsage(argv[0], "error: -c | --config option must have an argument\n", 7);
      *configFile = argv[i];
    } else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "-?") || !strcmp(argv[i], "--help")) {
      exitWithUsage(argv[0], NULL, 0);
    } else {
      exitWithUsage(argv[0], "error: unexpected argument\n", 8);
    }
  }
}

int main(int argc, char** argv) {
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);

  char* configFile = "esdm.conf";
  FILE *writeInstructionFile = NULL, *readInstructionFile = NULL;
  parseCommandlineArgs(argc, argv, &configFile, &writeInstructionFile, &readInstructionFile);

  esdm_mpi_init_manual();
  esdm_mpi_distribute_config_file(configFile);
  esdm_status ret = esdm_init();
  eassert(ret == ESDM_SUCCESS);

  if(writeInstructionFile) {
    size_t instructionCount;
    instruction_t* instructions = parseInstructions(writeInstructionFile, &instructionCount);
    fclose(writeInstructionFile);
    benchmarkWrite(instructionCount, instructions);
    deleteInstructions(instructionCount, instructions);
  }

  if(readInstructionFile) {
    size_t instructionCount;
    instruction_t* instructions = parseInstructions(readInstructionFile, &instructionCount);
    fclose(readInstructionFile);
    benchmarkRead(instructionCount, instructions);
    deleteInstructions(instructionCount, instructions);
  }

  esdm_mpi_finalize();
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if(!rank) printf("\nOK\n");
  MPI_Finalize();
}
