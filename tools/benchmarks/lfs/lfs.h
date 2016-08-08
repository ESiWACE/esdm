#ifndef LFS_H
#define LFS_H

int lfs_open(char *df, int flags, mode_t mode);
size_t lfs_read(int fd, char *buf, size_t count, off_t offset);
ssize_t lfs_write(int fd, void *buf, size_t count, off_t offset);


#endif /* !LFS_H */
