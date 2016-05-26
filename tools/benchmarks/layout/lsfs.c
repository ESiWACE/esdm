// This file is part of h5-memvol.
//
// This program is is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with h5-memvol.  If not, see <http://www.gnu.org/licenses/>.

/*
 * This program is a micro-benchmark that simulates a simple I/O cycle for:
 * P1) model output
 * P2) post-processing
 * to assess the performance of these steps using two implementations:
 * I1) default shared file with read-modify write for model output
 * I2) log-structured write (appends) where reads have to patch data.
 *
 * ./lsfs test.out 100 $((1024*1024*1)) 10 16
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <mpi.h>

#include "mtime.h"

char * filename;
char lfsfilename[1024];
int timesteps;
int iterationsSmall;
size_t blocksize;
size_t smalldatasize;
char * buffer;

void stdWrite(){
  int fd = open(filename, O_CREAT|O_TRUNC|O_RDWR, S_IRUSR|S_IWUSR);
  size_t ret = 0;

  timer t1;
  double t = 0;
  start_timer(& t1);

  size_t startPos = 0;
  for(int i=0; i < timesteps; i++){
    ret = pwrite(fd, buffer, blocksize, startPos);
    assert(ret == blocksize);

    for(int i=0; i < iterationsSmall; i++){
      size_t pos = rand() % (blocksize - smalldatasize) + startPos;
      ret = pwrite(fd, buffer, smalldatasize, pos);
      assert(ret == smalldatasize);
    }

    startPos += blocksize;
  }
  close(fd);

  size_t data = timesteps * blocksize + iterationsSmall * smalldatasize;
  t = stop_timer(t1);
  printf("stdWrite: %f %zu bytes = %.1f MiB/s\n", t, data, data / t / 1024.0 / 1024 );
}

void stdRead1DVar(){
  // we read one variable from each block
  int fd = open(filename, O_RDONLY);
  size_t ret = 0;

  timer t1;
  double t = 0;
  start_timer(& t1);

  // determine start position
  size_t startPos = rand() % (blocksize - smalldatasize);

  for(int i=0; i < timesteps; i++){
    ret = pread(fd, buffer, smalldatasize, startPos);
    assert(ret == smalldatasize);

    startPos += blocksize;
  }
  close(fd);
  size_t data = timesteps * smalldatasize;

  t = stop_timer(t1);
  printf("stdRead1DVar: %f %zu bytes = %.1f MiB/s\n", t, data, data / t / 1024.0 / 1024 );
}

void stdReadAll(){
  // we read one variable from each block
  int fd = open(filename, O_RDONLY);
  size_t ret = 0;

  timer t1;
  double t = 0;
  start_timer(& t1);

  // determine start position
  size_t startPos = 0;
  for(int i=0; i < timesteps; i++){
    ret = pread(fd, buffer, blocksize, startPos);
    assert(ret == blocksize);

    startPos += blocksize;
  }
  close(fd);
  size_t data = timesteps * blocksize;

  t = stop_timer(t1);
  printf("stdReadAll: %f %zu bytes = %.1f MiB/s\n", t, data, data / t / 1024.0 / 1024 );
}

// emulate log structured write
void lfsWrite(){
  int fd = open(filename, O_CREAT|O_TRUNC|O_RDWR, S_IRUSR|S_IWUSR);
  FILE * lfs = fopen(lfsfilename, "w+");
  size_t ret = 0;

  timer t1;
  double t = 0;
  start_timer(& t1);

  size_t startPos = 0;
  for(int i=0; i < timesteps; i++){
    ret = pwrite(fd, buffer, blocksize, startPos);
    assert(ret == blocksize);
    fwrite(& startPos, sizeof(startPos), 1, lfs);
    fwrite(& blocksize, sizeof(blocksize), 1, lfs);

    for(int i=0; i < iterationsSmall; i++){
      size_t pos = rand() % (blocksize - smalldatasize) + startPos;
      ret = pwrite(fd, buffer, smalldatasize, pos);

      fwrite(& pos, sizeof(startPos), 1, lfs);
      fwrite(& smalldatasize, sizeof(blocksize), 1, lfs);

      assert(ret == smalldatasize);
    }

    startPos += blocksize;
  }
  close(fd);
  fclose(lfs);

  size_t data = timesteps * blocksize + iterationsSmall * smalldatasize;
  t = stop_timer(t1);
  printf("lfsWrite: %f %zu bytes = %.1f MiB/s\n", t, data, data / t / 1024.0 / 1024 );
}

typedef struct{
  size_t pos;
  size_t size;
} lfs_record;


typedef struct{
  size_t size;
  lfs_record * records;
} lfs_index;



int lfsRecordComparePos(const void * u1, const void * v1){
  const lfs_record * u = (const lfs_record *) u1;
  const lfs_record * v = (const lfs_record *) v1;

  return u->pos - v->pos;
}

void lfsPrintIndex(lfs_index * index){
  for(int i=0; i < index->size; i++){
    printf("%zu %zu\n", index->records[i].pos, index->records[i].size);
  }
}

lfs_index * lfsReadIndex(){
  int ret;
  timer t1;
  double t = 0;
  start_timer(& t1);
  size_t data = 0;
  struct stat stats;
  ret = stat(lfsfilename, & stats);
  assert(ret == 0);

  int record_count = stats.st_size / sizeof(lfs_record);

  lfs_record * records = malloc(sizeof(lfs_record) * record_count);

  FILE * lfs = fopen(lfsfilename, "r");
  for(int i=0; i < record_count; i++){
    ret = fread(& records[i], sizeof(lfs_record), 1, lfs);
    assert( ret == 1 );
    data ++;
  }
  fclose(lfs);

  // do the sort
  qsort(records, record_count, sizeof(lfs_record), lfsRecordComparePos);

  t = stop_timer(t1);

  data *= sizeof(lfs_record);

  printf("lfsReadIndex: %f %zu bytes = %.1f MiB/s\n", t, data, data / t / 1024.0 / 1024 );
  lfs_index * index = (lfs_index*) malloc(sizeof(lfs_index));
  *index = (lfs_index){record_count, records};
  return index;
}



void cleanCache(){
  int ret;
  ret = system("sudo bash -c \"echo 3 > /proc/sys/vm/drop_caches\"");
  printf("clean Cache: %d\n", ret);
}

int main(int argc, char ** argv){
  if(argc == 1){
    printf("Synopsis: %s <filename> <timesteps> <blocksize> <iterationsSmall> <small-data-size>\n", argv[0]);
    exit(1);
  }
  filename = argv[1];
  timesteps = atoi(argv[2]);
  blocksize = atoll(argv[3]);
  iterationsSmall = atoi(argv[4]);
  smalldatasize = atoll(argv[5]);

  sprintf(lfsfilename, "%s.log", filename);

  assert(smalldatasize < blocksize);

  buffer = malloc(blocksize);
  memset(buffer, 1, blocksize);

  // stdWrite();
  // cleanCache();
  // stdRead1DVar();
  // cleanCache();
  // stdReadAll();
  // cleanCache();

  lfsWrite();
  cleanCache();
  lfs_index * index = lfsReadIndex();
  //lfsPrintIndex(index);

  return 0;
}
