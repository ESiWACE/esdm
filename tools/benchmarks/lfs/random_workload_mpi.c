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
#include <sys/time.h>
/*
#ifdef LFS_DUMMY_OPERATION
#include <lfs-dummy.h>
char *bench_type = "dummy";
#else
#include <lfs.h>
char *bench_type = "LFS";
#endif
*/
#include <lfs-mpi.h>


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
//	if ( argc != 5 ) /* argc should be 3 for correct execution */
//	{
//        	/* We print argv[0] assuming it is the program name */
//        	printf( "correct usage: %s <path to datafile> <random I/O size in Bytes> <file size in Bytes> <iterations>\n NOTE: I/O size should be multiple of 512 bytes (one sector)\n", argv[0] );
//		return -1;
//	}
//	size_t block_size = atoi(argv[2]);
//	long long file_size = (long long) atoll(argv[3]);
//	int iterations = atoi(argv[4]);

//	assert(block_size < (2^30));

	///---- setting files name ----///

	int ierr, num_procs, my_id;
	MPI_Init(&argc, &argv);

	/* find out MY process ID, and how many processes were started. */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	printf("Hello world! I'm process %i out of %i processes\n", my_id, num_procs);

	int myfd = lfs_mpi_open("MP.data", O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, MPI_COMM_WORLD);

	size_t seq_io = 800;
	if(my_id == 0 ){
		lfs_mpi_next_epoch();
	        char * fill_file = (char *)malloc(seq_io);
		memset(fill_file, 1, seq_io);
		lfs_mpi_write(myfd, fill_file, seq_io, 0);
	}
	if(my_id == 1 ){
                char * fill_file = (char *)malloc(seq_io);
                memset(fill_file, 2, seq_io);
                lfs_mpi_write(myfd, fill_file, seq_io, 800);
		lfs_mpi_next_epoch();
        }
	if(my_id == 2 ){
                sleep(3);
		char * fill_file = (char *)malloc(seq_io);
                memset(fill_file, 3, seq_io);
                lfs_mpi_write(myfd, fill_file, seq_io, 400);
		lfs_mpi_next_epoch();
		char * test_read = (char *)malloc(seq_io * 2);
		lfs_mpi_read(myfd, test_read, seq_io * 2, 0);
		FILE * res = fopen("mpi-result", "a+");
		fwrite(test_read, sizeof(char), (seq_io * 2) / sizeof(char), res);
		fclose(res);
        }

	lfs_mpi_close(myfd);
	ierr = MPI_Finalize();
	return 0;
}
