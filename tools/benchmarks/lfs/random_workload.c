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

#ifdef LFS_DUMMY_OPERATION
#include <lfs-dummy.h>
#else
#include <lfs.h>
#endif

void clear_cache(){
	sync();
	system("sudo /home/hr/drop-caches.sh");
//	int fd = open("/proc/sys/vm/drop_caches", O_WRONLY);
//	write(fd, "1", 1);
//	close(fd);
}


int main ( int argc, char *argv[] ){
	printf("---------------------\n");
//	printf("sizeof(size_t)= %d\n", sizeof(size_t));
	if ( argc != 3 ) /* argc should be 3 for correct execution */
	{
        	/* We print argv[0] assuming it is the program name */
        	printf( "correct usage: %s <path to datafile> <random I/O size in Bytes>\n NOTE: I/O size should be multiple of 512 bytes (one sector)\n", argv[0] );
		return -1;
	}
	size_t block_size = atoi(argv[2]);

	///---- setting files name ----///

	int myfd = lfs_open(argv[1], O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
	if (myfd == -1){
		printf("could not open/create the datafile! check the path and permissions. \n");
		return -1;
	}
	///---- starting workload ----///
	unsigned long long start;
	unsigned long long finish;
	unsigned long long seconds;
	struct timeval tv;
//	char * fill_file;
	long long myrand;
	clear_cache();
	//char c;
	int rett = 0;
	//FILE* my_io = fopen("/proc/self/io", "r");
	//if (my_io) {
	//	while ((c = getc(my_io)) != EOF)
	//		putchar(c);
	//	fclose(my_io);
	//}
//	pid_t my_pid = getpid();
//	system("cat /proc/%d/io", my_pid);
//	size_t seq_io = 800 * 1024 * 1024;
//	fill_file = (char *)malloc(seq_io);
//	memset(fill_file, 4, seq_io);
//	for(int iii = 0; iii < 40; iii++)
//		lfs_write(myfd, fill_file, seq_io, seq_io * iii);
//	printf("32 GB datafile created!\n");
//	clear_cache(); // clear the cache
//	lfs_close(myfd); // closing the file
//	myfd = lfs_open(argv[1], O_CREAT|O_RDWR, S_IRUSR|S_IWUSR); // re-openning the file
	gettimeofday(&tv, NULL);
        start = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
        srand(666);
	char * test_write;
//	free(fill_file);
	test_write = (char *)malloc(block_size);
	memset(test_write, 3, block_size);
	rett = lfs_write(myfd, test_write, block_size, (long long) 32 * 1024 * 1048576);
	if(rett <= 0)
		printf("something is not right %d\n", rett);
	int for_limit;
	if(block_size > 64 * 1024)
		for_limit = 1000;
	else
		for_limit = 10000;
	for(int i = 0; i < for_limit; i++)
	{
		//if(i % 50 == 0)
		myrand = (long long)(rand() % 8192) * 1048576 * 4;
		//	printf("random offset : %lld\n", myrand);
		rett = lfs_write(myfd, test_write, block_size, myrand);
		if(rett <= 0)
	                printf("some is not right %d\n", rett);
	}
        clear_cache(); // clear the cache
	free(test_write);
	lfs_close(myfd); // closing the file

	gettimeofday(&tv, NULL);
	finish = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
	seconds = finish - start;
	printf("writes took %llu milli-seconds\n", seconds);
	//system("cat /proc/%d/io", my_pid);
	//system("cat /proc/self/io");
	/*my_io = fopen("/proc/self/io", "r");
        if (my_io) {
                while ((c = getc(my_io)) != EOF)
                        putchar(c);
                fclose(my_io);
        }*/
	system("free -m | sed \"s/  */ /g\" | cut -d \" \" -f 7|tail -n 3");
	gettimeofday(&tv, NULL);
	start = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
	myfd = lfs_open(argv[1], O_CREAT|O_RDWR, S_IRUSR|S_IWUSR); // re-openning the file
	char * test_read;
	srand(start);
	system("free -m | sed \"s/  */ /g\" | cut -d \" \" -f 7|tail -n 3");
//	size_t read_bytes;
	test_read = (char *)malloc(8192 * 102400);
	for(int ii = 0; ii < 40; ii++){
		//myrand = (long long)(rand() % 8192) * 1048576 * 2;
		lfs_read(myfd, test_read, 8192 * 102400, (long long)8192 * 102400 * ii);
	}
	//free(test_read);
//	system("cat /proc/%d/io", my_pid);
	//system("cat /proc/self/io");
	/*my_io = fopen("/proc/self/io", "r");
        if (my_io) {
                while ((c = getc(my_io)) != EOF)
                        putchar(c);
                fclose(my_io);
        }*/
	gettimeofday(&tv, NULL);
	finish = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
	seconds = finish - start;
	system("free -m | sed \"s/  */ /g\" | cut -d \" \" -f 7|tail -n 3");
	lfs_close(myfd); // closing the file
	clear_cache(); // clear the cache
	printf("read took %llu milli-seconds\n", seconds);
	system("free -m | sed \"s/  */ /g\" | cut -d \" \" -f 7|tail -n 3");
//	system("cat /proc/%d/io", my_pid);
//	system("cat /proc/self/io");
	/*my_io = fopen("/proc/self/io", "r");
        if (my_io) {
                while ((c = getc(my_io)) != EOF)
                        putchar(c);
                fclose(my_io);
        }*/
	//int fd = open("read.result", O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
	//pwrite(fd, test_read, block_size * 10, 0);
	//close(fd);
	return 0;
}





