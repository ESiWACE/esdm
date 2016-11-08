#ifndef LFS_INTERNAL_H_
#define LFS_INTERNAL_H_

#include <sys/types.h>

struct tup
{
  size_t a;
  size_t b;
};

struct lfs_file
{
  FILE *log_file;
  MPI_Comm com;
  int data_file;
  int proc_rank;
  char *mother_file;
  int current_epoch;
  off_t file_position;
  char *filename;
  char is_new;
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

int read_record (struct lfs_record **rec, FILE * fd, int depth);
struct tup compare_tup (struct tup first, struct tup second);
size_t lfs_mpi_internal_read (int fd, char *buf, struct lfs_record **query, int *q_index, struct lfs_record *rec, int record_count, struct lfs_record **missing_chunks, int *m_ch_s,
                              off_t main_addr);
int lfs_mpi_find_chunks (size_t a, size_t b, int index, struct lfs_record *my_recs, struct lfs_record **chunks_stack, int *ch_s, struct lfs_record **missing_chunks, int *m_ch_s);
void lfs_vec_add (struct lfs_record **chunks_stack, int *size, struct lfs_record chunk);

#endif
