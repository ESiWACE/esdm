 
#include <fcntl.h>
#include <lfs-internal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

char *filename;
char *lfsfilename;
//std::string temp = "datafile.df";
//filename = ea_checked_strdup(temp.c_str());
//temp = "metafile.mf";
//lfsfilename = ea_checked_strdup(temp.c_str());

struct lfs_files lfsfiles[20];
int current_index = 0;

int lfs_open(char *df, int flags, mode_t mode) {
  filename = ea_checked_strdup(df);
  char *metafile = ea_checked_malloc((strlen(df) + 4) * sizeof(char));
  strcpy(metafile, df);
  strcat(metafile, ".log");
  //printf("filename: %s\n", filename);
  //printf("lfsfilename: %s\n", metafile);
  lfsfilename = ea_checked_strdup(metafile);
  lfsfiles[current_index].log_file = fopen(lfsfilename, "a+");
  lfsfiles[current_index].data_file = open(filename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  current_index++;

  return current_index - 1;
}

// this is the LFS write function
ssize_t lfs_write(int fd, void *buf, size_t count, off_t offset) {
  int ret = 0;
  size_t data_size;
  data_size = count;
  //      printf("given data size to lfs_write: %lld\n", offset);
  //block_num = data_size / block_size;  // we assume that data length is multiple of block_size !!!

  // determining the END OF FILE exact address
  struct stat stats;
  //stat(fdp->data_file, & stats);
  fstat(lfsfiles[fd].data_file, &stats);
  size_t end_of_file = stats.st_size;
  //      printf("e of f is: %lu\n",end_of_file);
  // perfoming the lfs write ---> appending the data into the END OF FILE
  ret = pwrite(lfsfiles[fd].data_file, buf, data_size, end_of_file);
  //      printf("ret is: %d\n",ret);
  // writing the mapping info for the written data to know it's exact address later
  fwrite(&offset, sizeof(offset), 1, lfsfiles[fd].log_file);
  fwrite(&data_size, sizeof(data_size), 1, lfsfiles[fd].log_file);

  return ret;
  // close(d_file);
  // fclose(lfs);
}

// extracts the mapping dict from our metadata(log) file
lfs_record *read_record(int fd) {
  int ret;
  // find the number of items in the array by using the size of the metadata file
  //struct stat stats;
  //ret = fstat(myfd, & stats);
  //printf("this is FD %d\n",fd);
  size_t fileLen;
  fseek(lfsfiles[fd].log_file, 0, SEEK_END);
  fileLen = ftell(lfsfiles[fd].log_file);
  fseek(lfsfiles[fd].log_file, 0, SEEK_SET);
  //eassert(ret == 0);
  int record_count = fileLen / sizeof(lfs_record_on_disk);
  //printf("this is size %d\n",record_count);
  lfs_record *records = ea_checked_malloc(sizeof(lfs_record) * record_count);
  size_t file_position = 0;

  // filling the created array with the values inside the metadata file
  //  FILE * lfs = fopen(lfsfilename, "r");
  for (int i = 0; i < record_count; i++) {
    ret = fread(&records[i], sizeof(lfs_record_on_disk), 1, lfsfiles[fd].log_file);
    records[i].pos = file_position;
    eassert(ret == 1);
    file_position += records[i].size;
  }
  //fclose(lfs);

  return records;
}

// finds the common are between two given tuples
struct tup compare_tup(struct tup first, struct tup second) {
  struct tup res;
  res.a = -1;
  res.b = -1;
  //printf("tups: (%d,%d) and (%d,%d)\n", first.a, first.b, second.a, second.b);
  if (first.a <= second.a && first.b > second.a) {
    if (first.b <= second.b) {
      res.a = second.a;
      res.b = first.b;
    } else {
      res.a = second.a;
      res.b = second.b;
    }
  }
  if (first.a >= second.a && first.a < second.b) {
    if (first.b <= second.b) {
      res.a = first.a;
      res.b = first.b;
    } else {
      res.a = first.a;
      res.b = second.b;
    }
  }
  //printf("res in here: %d, %d    ", res.a, res.b);
  return res;
}

// recursive function that finds all of the areas that should be read to complete a read query
int lfs_find_chunks(size_t a, size_t b, int index, struct lfs_record *my_recs, struct lfs_record **chunks_stack, int *ch_s) {
  //printf("check it out: %zu %zu, %d\n", a, b, index);

  // this IF is for ending the recursion
  if (a == b)
    return 0;
  struct tup res, rec, query;
  res.a = -1;
  res.b = -1;
  query.a = a;
  query.b = b;
  // search through the logs until you find a record that overlaps with the given query area
  while (res.a == -1) {
    //if(a >= 69492736)
    //      printf("index is: %d\n",index);
    if (index < 0) {
      /*printf("check it out: %lu %lu, %d\n", a, b, index);
         //printf("didn't find\n");
         //return 0;
         struct lfs_record found;
         //printf("result: %lu, %lu\n", res.a, res.b);
         found.addr = a;
         found.size = b - a;
         found.pos = a;
         //chunks_stack.push_back(found);
         lfs_vec_add(chunks_stack, ch_s, found); */
      return 0;
    }
    rec.a = my_recs[index].addr;
    rec.b = my_recs[index].addr + my_recs[index].size;
    res = compare_tup(query, rec);
    index--;
  }
  struct lfs_record found;
  //printf("result: %lu, %lu\n", res.a, res.b);
  found.addr = res.a;
  found.size = res.b - res.a;
  found.pos = my_recs[index + 1].pos + res.a - my_recs[index + 1].addr;
  //chunks_stack.push_back(found);
  lfs_vec_add(chunks_stack, ch_s, found);
  // call yourself for the remaing areas of the query that have not been covered with the found record
  lfs_find_chunks(a, res.a, index, my_recs, chunks_stack, ch_s);
  lfs_find_chunks(res.b, b, index, my_recs, chunks_stack, ch_s);
  return 0;
}

// main function for read query
size_t lfs_read(int fd, char *buf, size_t count, off_t offset) {
  //size_t lfs_read(size_t addr, size_t size, char * res){
  //int fd = open(filename, O_RDONLY);
  struct lfs_record temp;
  //int rtrd;
  // get the latest updated logs
  struct lfs_record *my_recs = read_record(fd);

  // create the vector that acts like a stack for our finding chunks recursive function
  struct lfs_record *chunks_stack;
  chunks_stack = ea_checked_malloc(sizeof(lfs_record) * 1001);
  int ch_s = 1;
  // find the length of the log array
  //struct stat stats;
  // int  myfd;
  //myfd = fileno(lfsfiles[fd].log_file);
  //      fstat(myfd, & stats);
  size_t fileLen;
  fseek(lfsfiles[fd].log_file, 0, SEEK_END);
  fileLen = ftell(lfsfiles[fd].log_file);
  fseek(lfsfiles[fd].log_file, 0, SEEK_SET);
  int record_count = fileLen / sizeof(lfs_record_on_disk);
  //      for(int mm = 0; mm < record_count; mm++)
  //              printf("rec: %lu, %lu, %lu\n", my_recs[mm].addr, my_recs[mm].size, my_recs[mm].pos);
  // call the recursive function to find which areas need to be read
  lfs_find_chunks(offset, offset + count, record_count - 1, my_recs, &chunks_stack, &ch_s);
  int total_found_count = ch_s - 1;
  //printf("recursive func finished\n");
  // perform the read
  for (int i = 0; i < total_found_count; i++) {
    temp = chunks_stack[i];
    //printf("chunk stack: (%zu, %zu, %zu)\n",temp.addr,temp.size,temp.pos);
    pread(lfsfiles[fd].data_file, &buf[(temp.addr - offset) / sizeof(char)], temp.size, temp.pos);
    //printf("rtrd is: %d\n",rtrd);
  }
  //free(chunks_stack);
  free(my_recs);
  // Optimization: Write down the read query for future reads!
  if (total_found_count == -3 /* THRESHOLD */) {
    lfs_write(fd, buf, count, offset);
  }
  free(chunks_stack);
  return count;
}

void lfs_vec_add(struct lfs_record **chunks_stack, int *size, struct lfs_record chunk) {
  //printf("size is:%d\n", *size - 1);
  //      printf("one\n");
  if (*size % 1000 == 0) {
    //              printf("two\n");
    struct lfs_record *new_stack;
    //struct lfs_record* temp;
    int rtd = *size + 1000;
    //printf("allocating %d size\n", rtd);
    new_stack = ea_checked_realloc(*chunks_stack, sizeof(lfs_record) * rtd);
    if (new_stack == NULL)
      printf("MEMORY ALLOCATION ERROR IN LFS STACK!!!\n");
    /*for(int i=0; i < *size - 1; i++){
       new_stack[i].addr = chunks_stack[i].addr;
       new_stack[i].size = chunks_stack[i].size;
       new_stack[i].pos = chunks_stack[i].pos;
       } */
    //              printf("three\n");
    //free(chunks_stack);
    *chunks_stack = new_stack;
    //printf("address of new chunk stack: %p\n", *chunks_stack);
    //printf("sozeof = %lu\n",sizeof(*new_stack));
    //free(temp);
  }
  //printf("address of current chunk stack: %p\n", *chunks_stack);

  (*chunks_stack)[*size - 1] = chunk;
  *size += 1;
  //printf("%d\n",*size);
  //printf("five\n");
  //printf("added chunk: %d, %d, %d\n", chunk.addr, chunk.size, chunk.pos);
  //printf("added chunk: %d, %d, %d\n", chunks_stack[*size - 2].addr, chunks_stack[*size - 2].size, chunks_stack[*size - 2].pos);
}

int lfs_close(int handle) {
  fclose(lfsfiles[handle].log_file);
  close(lfsfiles[handle].data_file);
  return 0;
}
