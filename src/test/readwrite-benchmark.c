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


int mpi_size;
int mpi_rank;
int run_read = 0;
int run_write = 0;
long timesteps = 0;
int cycleBlock = 0;
int64_t size;
int64_t volume;
int64_t volume_all;

typedef struct {
  char* varname;
  int64_t dimCount, timeDim;
  int64_t* offset;
  int64_t* size;
  int64_t* fragmentSize;
} instruction_t;

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
  int64_t* vector = *out_vector = malloc(dimCount*sizeof*vector);
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
  instruction_t* instructionList = malloc(allocatedInstructions*sizeof(*instructionList));

  char* line = NULL;
  size_t bufferSize = 0;
  ssize_t lineLength;

  while(0 < (lineLength = getline(&line, &bufferSize, instructions))) {
    if(!strlen(line)) continue; //ignore empty lines

    if(instructionCount == allocatedInstructions) {
      instructionList = realloc(instructionList, (allocatedInstructions *= 2)*sizeof(*instructionList));
      eassert(instructionList);
    }
    eassert(instructionCount < allocatedInstructions);
    instruction_t* newInstruction = & instructionList[instructionCount];
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

    //the fragment size must be 1 in the time dimension (if present)
    if(newInstruction->timeDim >= 0) {
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
    free(instructions[i].fragmentSize);
  }
  free(instructions);
}

void generateFragmentList(instruction_t* instruction, int64_t dimCount, int64_t (**out_fragmentOffsets)[dimCount], int64_t* out_fragmentCount) {
  //determine the total count of fragments
  int64_t totalFragmentCount = 1, fragmentCounts[dimCount];
  for(int64_t i = dimCount; i--;) {
    fragmentCounts[i] = (instruction->size[i] + instruction->fragmentSize[i] - 1)/instruction->fragmentSize[i];
    totalFragmentCount *= fragmentCounts[i];
  }

  //determine our own share of fragments
  int rank, procCount;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procCount);
  int64_t startFragment = rank*totalFragmentCount/procCount;
  int64_t stopFragment = (rank + 1)*totalFragmentCount/procCount;
  *out_fragmentCount = stopFragment - startFragment;
  int64_t (*fragmentOffsets)[dimCount] = *out_fragmentOffsets = malloc(*out_fragmentCount*sizeof*fragmentOffsets);

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
void generateFragment(int64_t dimCount, int64_t* offset, int64_t* size, int64_t* limit, int64_t** out_data, int64_t* out_size) {
  //determine the actual shape of the fragment
  int64_t datapoints = 1;
  for(int64_t i = dimCount; i--; ) {
    out_size[i] = offset[i] + size[i] <= limit[i] ? offset[i] + size[i] : limit[i];
    datapoints *= out_size[i];
  }

  //create the fragment's data
  int64_t* data = *out_data = malloc(datapoints*sizeof*data);
  for(int64_t i = datapoints; i--; ) {
    int64_t value = 0;
    for(int64_t dim = dimCount, index = i, factor = 1; dim--; index /= out_size[dim], factor *= size[dim]) {
      int64_t coord = index%out_size[dim] + offset[dim];  //Extract the value of this coordinate from the index ...
      value += factor*coord;  //... and reencode into a global index, ...
    }
    data[i] = value;  //... which we proceed to store as the datapoint.
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

void writeVariableTimestep(instruction_t* instruction, esdm_dataset_t* dataset, esdm_dataspace_t* dataspace, int64_t timestep) {
  if(!isOutputTimestep(instruction, timestep)) return;

  //set the time coord
  bool haveTime = instruction->timeDim >= 0;
  int64_t savedTimeOffset, savedTimeSize;
  if(haveTime) {
    savedTimeOffset = instruction->offset[instruction->timeDim];
    savedTimeSize = instruction->size[instruction->timeDim];
    instruction->offset[instruction->timeDim] = timestep;
    instruction->size[instruction->timeDim] = 1;
  }

  //write the data
  int64_t (*fragmentOffsets)[instruction->dimCount];
  int64_t fragmentCount;
  generateFragmentList(instruction, instruction->dimCount, &fragmentOffsets, &fragmentCount);

  for(int64_t i = fragmentCount; i--; ) {
    int64_t* data = NULL, dataSize[instruction->dimCount];
    generateFragment(instruction->dimCount, fragmentOffsets[i], instruction->fragmentSize, instruction->size, &data, dataSize);

    esdm_dataspace_t *subspace;
    int ret = esdm_dataspace_subspace(dataspace, instruction->dimCount, dataSize, fragmentOffsets[i], &subspace);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_write(dataset, data, subspace);
    eassert(ret == ESDM_SUCCESS);

    ret = esdm_dataspace_destroy(subspace);
    eassert(ret == ESDM_SUCCESS);
    free(data);
  }

  //restore the time dim
  if(haveTime) {
    instruction->offset[instruction->timeDim] = savedTimeOffset;
    instruction->size[instruction->timeDim] = savedTimeSize;
  }
}

void writeTimestep(size_t instructionCount, instruction_t* instructions, esdm_dataset_t** datasets, esdm_dataspace_t** dataspaces, int64_t timestep) {
  for(size_t i = 0; i != instructionCount; i++) {
    writeVariableTimestep(&instructions[i], datasets[i], dataspaces[i], timestep);
  }
}

void benchmarkWrite(size_t instructionCount, instruction_t* instructions) {
  //determine the count of time steps to write
  int64_t timeLimit = 0;
  for(size_t i = instructionCount; i--; ) {
    if(instructions[i].timeDim >= 0) {
      int64_t limit = instructions[i].offset[instructions[i].timeDim] + instructions[i].size[instructions[i].timeDim];
      timeLimit = timeLimit > limit ? timeLimit : limit;
    }
  }

  //create the datasets
  esdm_container_t *container = NULL;
  int ret = esdm_mpi_container_create(MPI_COMM_WORLD, "mycontainer", 1, &container);
  eassert(ret == ESDM_SUCCESS);
  esdm_dataspace_t* spaces[instructionCount];
  esdm_dataset_t* sets[instructionCount];
  for(size_t i = instructionCount; i--; ) {
    int64_t bounds[instructions[i].dimCount]; //unfortunately, we don't have a primitive to directly create a dataspace with an offset
    for(int64_t dim = instructions[i].dimCount; dim--; ) bounds[dim] = instructions[i].offset[dim] + instructions[i].size[dim];

    esdm_dataspace_t *dummy;
    ret = esdm_dataspace_create(instructions[i].dimCount, bounds, SMD_DTYPE_UINT64, &dummy);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_dataspace_subspace(dummy, instructions[i].dimCount, instructions[i].size, instructions[i].offset, &spaces[i]);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_dataspace_destroy(dummy);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_mpi_dataset_create(MPI_COMM_WORLD, container, "mydataset", spaces[i], &sets[i]);
    eassert(ret == ESDM_SUCCESS);
  }

  //perform the actual writes
  timer t;
  double time, md_sync_start;
  MPI_Barrier(MPI_COMM_WORLD);
  start_timer(&t);
  for(int64_t t = 0; t < timeLimit; t++) writeTimestep(instructionCount, instructions, sets, spaces, t);
  MPI_Barrier(MPI_COMM_WORLD);
  md_sync_start = stop_timer(t);

  //commit the changes to data to the metadata
  for(size_t i = instructionCount; i--; ) {
    ret = esdm_mpi_dataset_commit(MPI_COMM_WORLD, sets[i]);
    eassert(ret == ESDM_SUCCESS);
    ret = esdm_dataset_close(sets[i]);
    eassert(ret == ESDM_SUCCESS);
  }
  ret = esdm_mpi_container_commit(MPI_COMM_WORLD, container);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_close(container);
  eassert(ret == ESDM_SUCCESS);
  MPI_Barrier(MPI_COMM_WORLD);
  time = stop_timer(t);

  //determine our performance
  double total_time;
  MPI_Reduce((void *)&time, &total_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  double md_sync_time = total_time - md_sync_start;
  if (mpi_rank == 0) {
    printf("Write: %.3fs %.3f MiB/s size:%.0f MiB MDsyncTime: %.3fs\n", total_time, volume_all / total_time / 1024.0 / 1024, volume_all / 1024.0 / 1024, md_sync_time);
  }
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
    static char defaultInstructions[] = "mydataset(3, t = 0): (0, 0, 0) (10, 1024, 1024) (1, 256, 1024)\n";
    *writeInstructionFile = fmemopen(defaultInstructions, sizeof(defaultInstructions) - 1, "r");  //-1 for the length to avoid passing the terminating null byte to getline()
    *readInstructionFile = fmemopen(defaultInstructions, sizeof(defaultInstructions) - 1, "r");
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

int main2(int argc, char *argv[]) {
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);

  char* configFile = "_esdm.conf";
  FILE *writeInstructionFile = NULL, *readInstructionFile = NULL;
  parseCommandlineArgs(argc, argv, &configFile, &writeInstructionFile, &readInstructionFile);

  esdm_mpi_init();
  esdm_mpi_distribute_config_file(configFile);
  int ret = esdm_init();
  eassert(ret == ESDM_SUCCESS);

  if(writeInstructionFile) {
    size_t instructionCount;
    instruction_t* instructions = parseInstructions(writeInstructionFile, &instructionCount);
    fclose(writeInstructionFile);
    benchmarkWrite(instructionCount, instructions);
    deleteInstructions(instructionCount, instructions);
  }

/* TODO
  if(readInstructionFile) {
    size_t instructionCount;
    instruction_t* instructions = parseInstructions(readInstructionFile, &instructionCount);
    fclose(readInstructionFile);
    benchmarkRead(instructionCount, instructions);
    deleteInstructions(instructionCount, instructions);
  }
*/

  esdm_mpi_finalize();
  if(!mpi_rank) printf("\nOK\n");
  MPI_Finalize();
  return 0;
}

/*
void runSimpleWrite(uint64_t * buf_w, int64_t * dim, int64_t * offset){
  esdm_status ret;
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;
  // define dataspace
  int64_t bounds[] = {timesteps, size, size};
  esdm_dataspace_t *dataspace;

  ret = esdm_mpi_container_create(MPI_COMM_WORLD, "mycontainer", 1, &container);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataspace_create(3, bounds, SMD_DTYPE_UINT64, &dataspace);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mpi_dataset_create(MPI_COMM_WORLD, container, "mydataset", dataspace, &dataset);
  eassert(ret == ESDM_SUCCESS);

  timer t;
  double time, md_sync_start;

  MPI_Barrier(MPI_COMM_WORLD);
  start_timer(&t);
  // Write the data to the dataset
  for (int t = 0; t < timesteps; t++) {
    offset[0] = t;
    esdm_dataspace_t *subspace;

    ret = esdm_dataspace_subspace(dataspace, 3, dim, offset, &subspace);
    eassert(ret == ESDM_SUCCESS);
    buf_w[0] = t;
    ret = esdm_write(dataset, buf_w, subspace);
    eassert(ret == ESDM_SUCCESS);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  md_sync_start = stop_timer(t);

  // commit the changes to data to the metadata
  ret = esdm_mpi_dataset_commit(MPI_COMM_WORLD, dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mpi_container_commit(MPI_COMM_WORLD, container);
  eassert(ret == ESDM_SUCCESS);

  MPI_Barrier(MPI_COMM_WORLD);
  time = stop_timer(t);
  double total_time;
  MPI_Reduce((void *)&time, &total_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  double md_sync_time = total_time - md_sync_start;
  if (mpi_rank == 0) {
    printf("Write: %.3fs %.3f MiB/s size:%.0f MiB MDsyncTime: %.3fs\n", total_time, volume_all / total_time / 1024.0 / 1024, volume_all / 1024.0 / 1024, md_sync_time);
  }

  ret = esdm_dataset_close(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_close(container);
  eassert(ret == ESDM_SUCCESS);
}

void runRead(uint64_t * buf_w, int64_t * dim, int64_t * offset){
  esdm_status ret;
  esdm_container_t *container = NULL;
  esdm_dataset_t *dataset = NULL;
  // define dataspace
  esdm_dataspace_t *dataspace;

  ret = esdm_mpi_container_open(MPI_COMM_WORLD, "mycontainer", 0, &container);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_mpi_dataset_open(MPI_COMM_WORLD, container, "mydataset", &dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_dataset_get_dataspace(dataset, & dataspace);
  eassert(ret == ESDM_SUCCESS);

  timer t;
  double time;

  int64_t mismatches = 0;
  MPI_Barrier(MPI_COMM_WORLD);
  start_timer(&t);
  // Read the data to the dataset
  for (int t = 0; t < timesteps; t++) {
    offset[0] = t;
    uint64_t *buf_r = (uint64_t *) malloc(volume);
    buf_r[0] = -1241;
    eassert(buf_r != NULL);
    esdm_dataspace_t *subspace;
    esdm_dataspace_subspace(dataspace, 3, dim, offset, &subspace);

    ret = esdm_read(dataset, buf_r, subspace);
    eassert(ret == ESDM_SUCCESS);

    // verify data and fail test if mismatches are found
    buf_w[0] = t;
    for (int y = 0; y < dim[1]; y++) {
      for (int x = 0; x < dim[2]; x++) {
        uint64_t idx = y * size + x;
        if (buf_r[idx] != buf_w[idx]) {
          mismatches++;
          if(mismatches < 10){
            printf("Read timestep %d at pos %"PRIu64" %"PRId64" expected %"PRId64"\n", t, idx, buf_r[idx], buf_w[idx]);
          }
        }
      }
    }
    free(buf_r);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  time = stop_timer(t);

  int64_t mismatches_sum = 0;
  MPI_Reduce(&mismatches, &mismatches_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
  double total_time;
  MPI_Reduce((void *)&time, &total_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if (mpi_rank == 0) {
    if (mismatches_sum > 0) {
      printf("FAILED\n");
      printf("Mismatches: %"PRId64" of %"PRId64"\n", mismatches_sum, (int64_t)timesteps * size * size);
    } else {
      printf("OK\n");
    }
    printf("Read: %.3fs %.3f MiB/s size:%.0f MiB\n", total_time, volume_all / total_time / 1024.0 / 1024, volume_all / 1024.0 / 1024);
  }
  ret = esdm_dataset_close(dataset);
  eassert(ret == ESDM_SUCCESS);
  ret = esdm_container_close(container);
  eassert(ret == ESDM_SUCCESS);
}
*/

int main(int argc, char** argv) {
  return main2(argc, argv);
}

/*
int main(int argc, char *argv[]) {
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);

  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

  int64_t _size;
  char *config_file;
  char *default_args[] = {argv[0], "1024", "_esdm.conf", "B", "10"};
  if (argc == 1) {
    argc = 5;
    argv = default_args;
  }
  if (argc != 5) {
    printf("Syntax: %s [SIZE] [CONFIG] [B|R|W][C] [TIMESTEPS]", argv[0]);
    printf("\t SIZE specifies one dimension of a 2D field\n");
    exit(1);
  }

  _size = atol(argv[1]);
  config_file = argv[2];
  switch (argv[3][0]) {
    case ('R'): {
      run_read = 1;
      break;
    }
    case ('W'): {
      run_write = 1;
      break;
    }
    case ('B'): {
      run_read = 1;
      run_write = 1;
      break;
    }
    default: {
      printf("Unknown setting for argument: %s expected [R|W|B]\n", argv[3]);
      exit(1);
    }
  }
  cycleBlock = argv[3][1] == 'C';
  timesteps = atol(argv[4]);

  size = _size;

  if (mpi_rank == 0)
    printf("Running with %ld timesteps and 2D slice of %ld*%ld (cycle: %d)\n", timesteps, size, size, cycleBlock);

  if (size / mpi_size == 0) {
    printf("Error, size < number of ranks!\n");
    exit(1);
  }

  int pPerNode = esdm_mpi_get_tasks_per_node();
  int tmp_rank = (mpi_rank + (cycleBlock * pPerNode)) % mpi_size;
  int64_t dim[] = {1, size / mpi_size + (tmp_rank < (size % mpi_size) ? 1 : 0), size};
  int64_t offset[] = {0, size / mpi_size * tmp_rank + (tmp_rank < (size % mpi_size) ? tmp_rank : size % mpi_size), 0};

  volume = dim[1] * dim[2] * sizeof(uint64_t);
  volume_all = timesteps * size * size * sizeof(uint64_t);

  if (!volume_all) {
    printf("Error: no data!\n");
    exit(1);
  }

  // prepare data
  uint64_t *buf_w = (uint64_t *)malloc(volume);
  eassert(buf_w != NULL);
  long x, y;
  for (y = 0; y < dim[1]; y++) {
    for (x = 0; x < dim[2]; x++) {
      uint64_t idx = y * size + x;
      buf_w[idx] = (y+offset[1]) * size + x + offset[2] + 1;
    }
  }

  esdm_status ret;

  esdm_mpi_init();
  esdm_mpi_distribute_config_file(config_file);

  ret = esdm_init();
  eassert(ret == ESDM_SUCCESS);

  if (run_write) {
    if(mpi_rank == 0){
      ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_GLOBAL);
      eassert(ret == ESDM_SUCCESS);
    }
    MPI_Comm localcomm;
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0, MPI_INFO_NULL, &localcomm);
    int localrank;
    MPI_Comm_rank(localcomm, &localrank);
    if(localrank == 0){
      ret = esdm_mkfs(ESDM_FORMAT_PURGE_RECREATE, ESDM_ACCESSIBILITY_NODELOCAL);
      eassert(ret == ESDM_SUCCESS);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    runSimpleWrite(buf_w, dim, offset);
  }

  if (run_read) {
    runRead(buf_w, dim, offset);
  }

  esdm_mpi_finalize();

  // clean up
  free(buf_w);

  if(!mpi_rank) printf("\nOK\n");

  MPI_Finalize();

  return 0;
}
*/
