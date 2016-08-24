#ifndef LFS_DUMMY_H
#define LFS_DUMMY_H

#include <errno.h>

#warning Using LFS Dummy


#define lfs_close close
#define lfs_open open

static size_t lfs_read(int fd, char* buf, size_t count, off_t offset){
	size_t count1 = count;
	off_t offset1 = offset;
	size_t ret;
	while(count1 > 0){
                ret =  pread(fd, buf, count1, offset1);
		if (ret != count){
		  if (ret == -1){
		    if(errno == EINTR){
		      continue;
		    }
		    return count - count1;
		  }
                  if (ret == 0 && errno == 0){
			return count - count1;
		  }
                }
		buf += ret;
		count1 -= ret;
		offset1 += ret;
	}
	return count;
}


static size_t lfs_write(int fd, char* buf, size_t count, off_t offset){
	size_t count1 = count;
	off_t offset1 = offset;
	size_t ret;
	while(count1 > 0){
		ret = pwrite(fd, buf, count1, offset1);
		if (ret != count){
		  if (ret == -1){
		    if(errno == EINTR){
		      continue;
		    }
		    return count - count1;
		  }
                }
		buf += ret;
		count1 -= ret;
		offset1 += ret;
	}
	return count;
}

#endif /* !LFS_DUMMY_H */
