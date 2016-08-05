#ifndef LFS_DUMMY_H
#define LFS_DUMMY_H

int lfs_open(char * df, char * mf);

#define lfs_read pread
#define lfs_write pwrite

#endif /* !LFS_DUMMY_H */
