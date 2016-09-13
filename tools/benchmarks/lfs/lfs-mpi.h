#ifndef LFS_DUMMY_H
#define LFS_DUMMY_H

#include <errno.h>

#warning Using LFS Dummy

int  lfs_mpi_close(int  handle);
int lfs_mpi_open(char *df, int flags, mode_t mode, int proc_num);
size_t lfs_mpi_read(int fd, char *buf, size_t count, off_t offset);
ssize_t lfs_mpi_write(int fd, void *buf, size_t count, off_t offset);

/*#define lfs_close mpi_close
#define lfs_open mpi_open
#define lfs_read mpi_read
#define lfs_write mpi_write*/

#endif /* !LFS_DUMMY_H */
