#ifndef LFS_MPI_H
#define LFS_MPI_H

#include <errno.h>
#include <mpi.h>

#warning Using LFS Dummy

typedef struct lfs_file* lfs_mpi_file_p;

void lfs_mpi_next_epoch(lfs_mpi_file_p fd);
int lfs_mpi_close(lfs_mpi_file_p fd);
int lfs_mpi_open(lfs_mpi_file_p * fd, char *df, int flags, mode_t mode, MPI_Comm com);
size_t lfs_mpi_read(lfs_mpi_file_p fd, char *buf, size_t count, off_t offset);
size_t lfs_mpi_write(lfs_mpi_file_p fd, void *buf, size_t count, off_t offset);

/*#define lfs_close mpi_close
#define lfs_open mpi_open
#define lfs_read mpi_read
#define lfs_write mpi_write*/

#endif /* !LFS_DUMMY_H */
