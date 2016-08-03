#ifndef LFS_INTERNAL_H_
#define LFS_INTERNAL_H_

#include <sys/types.h>
#include <vector>

#include <lfs.h>

struct tup{
  size_t a;
  size_t b;
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
lfs_record * read_record();
struct tup compare_tup(struct tup first, struct tup second); // C++ my friend is not the goal :-)
int lfs_find_chunks(size_t a, size_t b, int index, lfs_record * my_recs, std::vector<lfs_record>& chunks_stack);


#endif
