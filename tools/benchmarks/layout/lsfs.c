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
#include <stdint.h>
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
} lfs_record_on_disk;

typedef struct{
  size_t pos;
  size_t size;
  size_t file_position;
} lfs_record;

typedef struct{
  int size;
  lfs_record * records;
  lfs_record * records_end; // sorted by the end position of an record
} lfs_index;



static int lfsRecordComparePos(const void * u1, const void * v1){
  const lfs_record_on_disk * u = (const lfs_record_on_disk *) u1;
  const lfs_record_on_disk * v = (const lfs_record_on_disk *) v1;

  return u->pos - v->pos;
}

static int lfsRecordCompareEndPos(const void * u1, const void * v1){
  const lfs_record_on_disk * u = (const lfs_record_on_disk *) u1;
  const lfs_record_on_disk * v = (const lfs_record_on_disk *) v1;

  return u->pos + u->size - (v->pos + v->size);
}

void lfsPrintIndex(lfs_record * r, int size){
  for(int i=0; i < size; i++){
    printf("%d pos: %zu size: %zu file: %zu\n", i, r[i].pos, r[i].size, r[i].file_position);
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

  int record_count = stats.st_size / sizeof(lfs_record_on_disk);

  lfs_record * records = malloc(sizeof(lfs_record) * record_count);

  size_t file_position = 0;
  FILE * lfs = fopen(lfsfilename, "r");
  for(int i=0; i < record_count; i++){
    ret = fread(& records[i], sizeof(lfs_record_on_disk), 1, lfs);
    records[i].file_position = file_position;
    assert( ret == 1 );
    data ++;
    file_position += records[i].size;
  }
  fclose(lfs);

  // do the sort
  qsort(records, record_count, sizeof(lfs_record), lfsRecordComparePos);

  lfs_record * records_end = malloc(sizeof(lfs_record) * record_count);
  memcpy(records_end, records, sizeof(lfs_record) * record_count);
  qsort(records_end, record_count, sizeof(lfs_record), lfsRecordCompareEndPos);

  t = stop_timer(t1);

  data *= sizeof(lfs_record_on_disk);

  printf("lfsReadIndex: %f %zu bytes = %.1f MiB/s\n", t, data, data / t / 1024.0 / 1024 );
  lfs_index * index = (lfs_index*) malloc(sizeof(lfs_index));
  *index = (lfs_index){record_count, records, records_end};
  return index;
}

static inline int lfs_binsearch_left(off_t start, lfs_record * r, int size){
  int l = 0;
  int u = size - 1;
  int p = (l + u) / 2;

  while(l < u){
    //printf("bsl %d %d (%d): start %zu r.pos %zu r.sum %zu\n", l, u, p, start,  r[p].pos, r[p].size +r[p].pos);
    int64_t d = ((int64_t) start) - (r[p].pos + r[p].size);

    if ( d < 0 ){
      // left side
      u = p - 1;
    }else if(d > 0){
      l = p + 1;
    }else{ // equal
      return p;
    }
    p = (l + u) / 2;
  }

  //printf("xl %zu pos=%zu size=%zu\n", start, r[p].pos, r[p].size);
  return p;
}

static inline int lfs_binsearch_right(off_t end, lfs_record * r, int size){
  int l = 0;
  int u = size - 1;
  int p = (l + u) / 2;

  while(l < u){
    //printf("%d %d (%d): %zu %zu\n", l, u, p, offset,  r[p].pos);
    int64_t d = ((int64_t) end) - r[p].pos;

    if ( d < 0 ){
      // left side
      u = p - 1;
    }else if(d > 0){
      l = p + 1;
    }else{ // equal
      return p;
    }
    p = (l + u) / 2;
  }
  //printf("xr %lld\n", ((int64_t) offset) - r[p].pos);
  //if(r[p].pos < offset){
    //printf("HERE %d %lld < %lld\n", p, offset, r[p].pos);
  //}
  return p;
}


size_t lfs_pread(int fd, void *buf, size_t size, size_t offset, lfs_index * index){
  printf("pread %zu - %zu\n", offset, size);

  // we have to find all overlapping regions in the index.
  // there are two options for overlaps
  // left side overlap, i.e., r->pos <= offset && r->pos + r->size > offset
  // right side overlap, i.e., r->pos > offset && offset + size > r->pos
  int cur_pos = lfs_binsearch_left(offset + size, index->records_end, index->size);
  printf(" l %d\n", cur_pos);

  lfs_record * r = & index->records[cur_pos];

  size_t end = offset + size;
  // left side overlaps
  while(cur_pos >= 0 && (r->pos + r->size) > offset){
    printf("l %lu %lu\n", r->pos, r->size);
    cur_pos--;
    r = & index->records[cur_pos];
  }

  cur_pos = lfs_binsearch_right(offset, index->records, index->size);
  r = & index->records[cur_pos];
  //printf(" r %d %lu %lu\n", start_pos, r->pos, r->size);
  // right side overlaps
  while( cur_pos < index->size && r->pos < end){
    printf("r %lu %lu\n", r->pos, r->size);
    cur_pos++;
    r = & index->records[cur_pos];
  }

  return size;
}

void lfsReadAll(lfs_index * index){
  // we read one variable from each block
  int fd = open(filename, O_RDONLY);
  size_t ret = 0;

  timer t1;
  double t = 0;
  start_timer(& t1);

  // determine start position
  size_t startPos = 0;
  for(int i=0; i < timesteps; i++){
    ret = lfs_pread(fd, buffer, blocksize, startPos, index);
    assert(ret == blocksize);

    startPos += blocksize;
  }
  close(fd);
  size_t data = timesteps * blocksize;

  t = stop_timer(t1);
  printf("lfsReadAll: %f %zu bytes = %.1f MiB/s\n", t, data, data / t / 1024.0 / 1024 );
}

void lfsRead1DVar(lfs_index * index){
  // we read one variable from each block
  int fd = open(filename, O_RDONLY);
  size_t ret = 0;

  timer t1;
  double t = 0;
  start_timer(& t1);

  // determine start position
  size_t startPos = rand() % (blocksize - smalldatasize);

  for(int i=0; i < timesteps; i++){
    ret = lfs_pread(fd, buffer, smalldatasize, startPos, index);
    assert(ret == smalldatasize);

    startPos += blocksize;
  }
  close(fd);
  size_t data = timesteps * smalldatasize;

  t = stop_timer(t1);
  printf("stdRead1DVar: %f %zu bytes = %.1f MiB/s\n", t, data, data / t / 1024.0 / 1024 );
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
  //lfsPrintIndex(index->records, index->size);
  //printf("End:\n");
  //lfsPrintIndex(index->records_end, index->size);

  lfsReadAll(index);
  cleanCache();
  lfsRead1DVar(index);

  return 0;
}
