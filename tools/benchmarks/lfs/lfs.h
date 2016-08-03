#ifndef LFS_H
#define LFS_H

int lfs_set_blocksize(size_t blocksize);

#ifndef LFS_DUMMY_OPERATION

int lfs_open(char * df, char * mf);
size_t lfs_read(size_t addr, size_t size, char * res);
void lfs_write(size_t addr, char * data);


#else


void normal_write(size_t addr, char * data);
size_t normal_read(size_t addr, size_t size, char * res);
int normal_open(char * df, char * mf);

#define lfs_open normal_open
#define lfs_read normal_read
#define lfs_write normal_write

#endif
#endif /* !LFS_H */
