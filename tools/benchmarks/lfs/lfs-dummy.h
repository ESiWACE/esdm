#ifndef LFS_DUMMY_H
#define LFS_DUMMY_H

#warning Using LFS Dummy


#define lfs_close close
#define lfs_open open
#define lfs_read pread
#define lfs_write pwrite

#endif /* !LFS_DUMMY_H */
