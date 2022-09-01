 
#include <fcntl.h>
#include <lfs-mpi.h>
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
/*
#ifdef LFS_DUMMY_OPERATION
#include <lfs-dummy.h>
char *bench_type = "dummy";
#else
#include <lfs.h>
char *bench_type = "LFS";
#endif
*/

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
  //      if ( argc != 5 ) /* argc should be 3 for correct execution */
  //      {
  //              /* We print argv[0] assuming it is the program name */
  //              printf( "correct usage: %s <path to datafile> <random I/O size in Bytes> <file size in Bytes> <iterations>\n NOTE: I/O size should be multiple of 512 bytes (one sector)\n", argv[0] );
  //              return -1;
  //      }
  //      size_t block_size = atoi(argv[2]);
  //      long long file_size = (long long) atoll(argv[3]);
  //      int iterations = atoi(argv[4]);

  //      eassert(block_size < (2^30));

  ///---- setting files name ----///

  int ierr, num_procs, my_id;
  MPI_Init(&argc, &argv);

  /* find out MY process ID, and how many processes were started. */
  MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  printf("Hello world! I'm process %i out of %i processes\n", my_id, num_procs);

  lfs_mpi_file_p myfd;
  int ret = lfs_mpi_open(&myfd, "MP.data", O_TRUNC | O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, MPI_COMM_WORLD);

  size_t seq_io = 800;
  if (my_id == 0) {
    lfs_mpi_next_epoch(myfd);
    char *fill_file = ea_checked_malloc(seq_io);
    memset(fill_file, 1, seq_io);
    lfs_mpi_write(myfd, fill_file, seq_io, 0);
  }
  if (my_id == 1) {
    char *fill_file = ea_checked_malloc(seq_io);
    memset(fill_file, 2, seq_io);
    lfs_mpi_write(myfd, fill_file, seq_io, 800);
    lfs_mpi_next_epoch(myfd);
  }
  if (my_id == 2) {
    sleep(1);
    char *fill_file = ea_checked_malloc(seq_io);
    memset(fill_file, 3, seq_io);
    lfs_mpi_write(myfd, fill_file, seq_io, 400);
    lfs_mpi_next_epoch(myfd);
  }
  lfs_mpi_close(myfd);

  ret = lfs_mpi_open(&myfd, "MP.data", O_RDONLY, S_IRUSR | S_IWUSR, MPI_COMM_WORLD);
  if (my_id == 2) {
    char *test_read = ea_checked_malloc(seq_io * 2);
    lfs_mpi_read(myfd, test_read, seq_io * 2, 0);
    FILE *res = fopen("mpi-result", "a+");
    fwrite(test_read, sizeof(char), (seq_io * 2) / sizeof(char), res);
    fclose(res);
  }
  ierr = MPI_Finalize();
  return 0;
}
