#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
//#include <mpi.h>
#include <sys/time.h>

#include "lfs.h"

void clear_cache(){
	sync();

	int fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
	write(fd, "3", 1);
	close(fd);
}


int main(){
	size_t block_size = 16 * 1024;
	lfs_set_blocksize(block_size);

	//end_of_file = 0;
	///---- setting files name ----///
	
	/*
	char temp[] = "datafile.df";
	filename = strdup(temp.c_str());
	temp = "metafile.mf";
	lfsfilename = strdup(temp.c_str());
	printf("filename: %s\n", filename);
	printf("lfsfilename: %s\n", lfsfilename);
	*/

	lfs_open("datafile.df", "metafile.mf");

	///---- starting workload ----///
	unsigned long long start, finish;
	unsigned long long seconds;
	struct timeval tv;
	srand(start);
	char * fill_file;

	gettimeofday(&tv, NULL);
	start = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;

	fill_file = (char *)malloc(block_size * 100000);
	memset(fill_file, 8, block_size * 100000);
	lfs_write(0, fill_file);
	clear_cache(); // clear the cache

	char * test_write;
	test_write = (char *)malloc(block_size * 10);
	for(int i = 0; i < 10000; i ++)
	{
		if(i % 1000 == 0)
			printf("writes done: %d\n", i);
		memset(test_write, (i % 8) + 1, block_size * 10);
		lfs_write((rand() % 100000) * block_size, test_write);
		clear_cache(); // clear the cache
	}
	
	gettimeofday(&tv, NULL);
	finish = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
	seconds = finish - start;
	printf("writes took %llu milli-seconds\n", seconds);

	gettimeofday(&tv, NULL);
	start = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;

	char * test_read;
	size_t read_bytes;
	test_read = (char *)malloc(block_size * 100000);
	read_bytes = lfs_read(0, block_size * 100000, test_read);

	gettimeofday(&tv, NULL);
	finish = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
	seconds = finish - start;

	printf("read took %llu milli-seconds\n", seconds);
	int fd = open("read.result", O_CREAT|O_APPEND|O_RDWR, S_IRUSR|S_IWUSR);
	pwrite(fd, test_read, block_size * 100000, 0);

	return 0;
}





