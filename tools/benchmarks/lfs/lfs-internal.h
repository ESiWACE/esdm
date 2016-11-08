#ifndef LFS_INTERNAL_H_
#define LFS_INTERNAL_H_

#include <sys/types.h>

#include <lfs.h>

struct tup
{
  size_t a;
  size_t b;
};

struct lfs_files
{
  FILE *log_file;
  int data_file;
  int proc_num;
};

typedef struct
{
  off_t addr;
  size_t size;
} lfs_record_on_disk;

typedef struct lfs_record
{
  off_t addr;
  size_t size;
  size_t pos;
} lfs_record;

// internal only?
lfs_record *read_record (int fd);
struct tup compare_tup (struct tup first, struct tup second);   // C++ my friend is not the goal :-)
int lfs_find_chunks (size_t a, size_t b, int index, struct lfs_record *my_recs, struct lfs_record **chunks_stack, int *ch_s);
void lfs_vec_add (struct lfs_record **chunks_stack, int *size, struct lfs_record chunk);

#endif
