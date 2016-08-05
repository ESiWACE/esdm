#ifndef LFS_INTERNAL_H_
#define LFS_INTERNAL_H_

#include <sys/types.h>

#ifdef LFS_DUMMY_OPERATION
#include <lfs-dummy.h>
#else
#include <lfs.h>
#endif

struct tup{
  size_t a;
  size_t b;
};

struct lfs_files{
	FILE* log_file;
	int data_file;
};

struct lfs_record_on_disk{
  size_t addr;
  size_t size;
};

struct lfs_record{
  size_t addr;
  size_t size;
  size_t pos;
};

// internal only?
lfs_record * read_record(int fd);
struct tup compare_tup(struct tup first, struct tup second); // C++ my friend is not the goal :-)
int lfs_find_chunks(size_t a, size_t b, int index, struct lfs_record * my_recs, struct lfs_record * chunks_stack, int* ch_s);
void lfs_vec_add(struct lfs_record* chunks_stack, int * size, struct lfs_record chunk);

#endif
