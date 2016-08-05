#ifndef LFS_H
#define LFS_H

int lfs_set_blocksize(size_t blocksize);

#ifndef LFS_DUMMY_OPERATION

int lfs_open(char * df, char * mf);
size_t lfs_read(int fd, char *buf, size_t count, off_t offset);
ssize_t lfs_write(int fd, void *buf, size_t count, off_t offset);


#else


//void normal_write(size_t addr, char * data);
//size_t normal_read(size_t addr, size_t size, char * res);
int lfs_open(char * df, char * mf);

//#define lfs_open open
#define lfs_read pread
#define lfs_write pwrite

#endif
#endif /* !LFS_H */
