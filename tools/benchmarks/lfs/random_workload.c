 
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
//#include <mpi.h>

#ifdef LFS_DUMMY_OPERATION
#  include <lfs-dummy.h>
char *bench_type = "dummy";
#else
#  include <lfs.h>
char *bench_type = "LFS";
#endif

void clear_cache() {
  sync();
  system("sudo /home/hr/drop-caches.sh");
  //      int fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
  //      write(fd, "1", 1);
  //      close(fd);
}

int main(int argc, char *argv[]) {
  printf("---------------------\n");
  //      printf("sizeof(size_t)= %d\n", sizeof(size_t));
  if (argc != 5) { /* argc should be 3 for correct execution */
    /* We print argv[0] assuming it is the program name */
    printf("correct usage: %s <path to datafile> <random I/O size in Bytes> <file size in Bytes> <iterations>\n NOTE: I/O size should be multiple of 512 bytes (one sector)\n",
    argv[0]);
    return -1;
  }
  size_t block_size = atoi(argv[2]);
  long long file_size = (long long)atoll(argv[3]);
  int iterations = atoi(argv[4]);

  //      eassert(block_size < (2^30));

  ///---- setting files name ----///

  int myfd = lfs_open(argv[1], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (myfd == -1) {
    printf("could not open/create the datafile! check the path and permissions. \n");
    return -1;
  }
  ///---- starting workload ----///
  unsigned long long start;
  unsigned long long finish;
  unsigned long long seconds;
  struct timeval tv;
  char *fill_file;
  long long myrand;
  clear_cache();
  //char c;
  int rett = 0;
  //FILE* my_io = fopen("/proc/self/io", "r");
  //if (my_io) {
  //      while ((c = getc(my_io)) != EOF)
  //              putchar(c);
  //      fclose(my_io);
  //}
  //      pid_t my_pid = getpid();
  //      system("cat /proc/%d/io", my_pid);
  printf("Initializing the benchmcark for %s version with following configurations:\n  File: %s\n  File Size: %lld Bytes\n  Iterations: %d\n  I/O Size: %zu Bytes\n", bench_type,
  argv[1], file_size, iterations, block_size);
  printf("---------------------\n");
  size_t seq_io = 800 * 1024 * 1024;
  fill_file = ea_checked_malloc(seq_io);
  memset(fill_file, 4, seq_io);
  for (long long iii = 0; iii < file_size; iii += seq_io) {
    //myrand = (long long)(rand() % 8192) * 1048576 * 2;
    //printf("block size is: %lu\n",blocksize);
    lfs_write(myfd, fill_file, seq_io, iii);
    if (iii + seq_io > file_size) {
      seq_io = file_size - iii;
    }
    //printf("block size is: %lu\n",blocksize);
  }

  //      printf("32 GB datafile created!\n");
  clear_cache();                                                 // clear the cache
  lfs_close(myfd);                                               // closing the file
  myfd = lfs_open(argv[1], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); // re-openning the file
  gettimeofday(&tv, NULL);
  start = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
  srand(666);
  char *test_write;
  free(fill_file);
  test_write = ea_checked_malloc(block_size);
  memset(test_write, 3, block_size);
  rett = lfs_write(myfd, test_write, block_size, file_size);
  if (rett <= 0) {
    printf("Could not create the proper file size of %lld, %d\n", file_size, rett);
    return 0;
  }
  printf("---- performing Random writes ----\n");
  for (int i = 0; i < iterations; i++) {
    //if(i % 50 == 0)
    myrand = (((long long)rand() * block_size) % file_size);
    //printf("random offset : %lld\n", myrand);
    rett = lfs_write(myfd, test_write, block_size, myrand);
    if (rett <= 0) {
      printf("write transaction failed!!! lfs_write returned %d!!!\n", rett);
      return 0;
    }
  }
  clear_cache(); // clear the cache
  free(test_write);
  lfs_close(myfd); // closing the file

  gettimeofday(&tv, NULL);
  finish = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
  seconds = finish - start;
  printf("writes took %llu milli-seconds\n", seconds);
  printf("write bandwidth: %llu MB/s\n", (((unsigned long long)block_size * iterations * 1000) / (1024 * 1024)) / seconds);
  printf("write IOPS: %llu\n", ((unsigned long long)iterations * 1000) / seconds);
  //system("cat /proc/%d/io", my_pid);
  //system("cat /proc/self/io");
  /*my_io = fopen("/proc/self/io", "r");
     if (my_io) {
     while ((c = getc(my_io)) != EOF)
     putchar(c);
     fclose(my_io);
     } */
  system("free -m | sed \"s/  */ /g\" | cut -d \" \" -f 7|tail -n 3");
  gettimeofday(&tv, NULL);
  start = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
  myfd = lfs_open(argv[1], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); // re-openning the file
  char *test_read;
  srand(666);
  printf("---- performing Random Read ----\n");
  //system("free -m | sed \"s/  */ /g\" | cut -d \" \" -f 7|tail -n 3");
  //      size_t read_bytes;
  //test_read = ea_checked_malloc(8192 * 102400);
  long long blocksize = 2048 * 102400;
  test_read = ea_checked_malloc(blocksize);

  //      for(long long pos = 0; pos < file_size; pos += blocksize){
  for (int i = 0; i < 50; i++) {
    myrand = (((long long)rand() * blocksize) % file_size);
    //printf("block size is: %lu\n",blocksize);
    lfs_read(myfd, test_read, blocksize, myrand);
    //if (pos + blocksize > file_size){
    //      blocksize = file_size - pos;
    //}
    //printf("block size is: %lu\n",blocksize);
  }
  //free(test_read);
  //      system("cat /proc/%d/io", my_pid);
  //system("cat /proc/self/io");
  /*my_io = fopen("/proc/self/io", "r");
     if (my_io) {
     while ((c = getc(my_io)) != EOF)
     putchar(c);
     fclose(my_io);
     } */
  gettimeofday(&tv, NULL);
  finish = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
  seconds = finish - start;
  system("free -m | sed \"s/  */ /g\" | cut -d \" \" -f 7|tail -n 3");
  lfs_close(myfd); // closing the file
  clear_cache();   // clear the cache
  printf("read took %llu milli-seconds\n", seconds);
  printf("read bandwidth: %llu MB/s\n", (((unsigned long long)file_size * 1000) / (1024 * 1024)) / seconds);
  printf("read IOPS: %llu\n", ((unsigned long long)iterations * 1000) / seconds);
  system("free -m | sed \"s/  */ /g\" | cut -d \" \" -f 7|tail -n 3");
  //      system("cat /proc/%d/io", my_pid);
  //      system("cat /proc/self/io");
  /*my_io = fopen("/proc/self/io", "r");
     if (my_io) {
     while ((c = getc(my_io)) != EOF)
     putchar(c);
     fclose(my_io);
     } */
  system("free -m | sed \"s/  */ /g\" | cut -d \" \" -f 7|tail -n 3");
  gettimeofday(&tv, NULL);
  start = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
  myfd = lfs_open(argv[1], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); // re-openning the file
  //char * test_read;
  srand(666);
  printf("---- performing Sequential Read (Reading entire file) ----\n");
  //system("free -m | sed \"s/  */ /g\" | cut -d \" \" -f 7|tail -n 3");
  //      size_t read_bytes;
  //test_read = ea_checked_malloc(8192 * 102400);
  blocksize = 8192 * 102400;
  test_read = ea_checked_malloc(blocksize);

  for (long long pos = 0; pos < file_size; pos += blocksize) {
    //for(int i = 0; i < iterations; i++){
    //      myrand = (((long long)rand() * blocksize) % file_size);
    //printf("block size is: %lu\n",blocksize);
    lfs_read(myfd, test_read, blocksize, pos);
    if (pos + blocksize > file_size) {
      blocksize = file_size - pos;
    }
    //printf("block size is: %lu\n",blocksize);
  }
  //free(test_read);
  //      system("cat /proc/%d/io", my_pid);
  //system("cat /proc/self/io");
  /*my_io = fopen("/proc/self/io", "r");
     if (my_io) {
     while ((c = getc(my_io)) != EOF)
     putchar(c);
     fclose(my_io);
     } */
  gettimeofday(&tv, NULL);
  finish = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
  seconds = finish - start;
  system("free -m | sed \"s/  */ /g\" | cut -d \" \" -f 7|tail -n 3");
  lfs_close(myfd); // closing the file
  clear_cache();   // clear the cache
  printf("read took %llu milli-seconds\n", seconds);
  printf("read bandwidth: %llu MB/s\n", (((unsigned long long)file_size * 1000) / (1024 * 1024)) / seconds);
  printf("read IOPS: %llu\n", ((unsigned long long)iterations * 1000) / seconds);
  //int fd = open("read.result", O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
  //pwrite(fd, test_read, block_size * 10, 0);
  //close(fd);
  return 0;
}
