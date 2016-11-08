#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <mpi.h>
#include <sys/time.h>

#include <lfs-mpi-internal.h>
#include <lfs-mpi.h>

int
lfs_mpi_open (lfs_mpi_file_p * fd_p, char *df, int flags, mode_t mode, MPI_Comm com)
{
  *fd_p = (lfs_mpi_file_p) malloc (sizeof (struct lfs_file));

  lfs_mpi_file_p fd = *fd_p;
  int ret, rank;                //, size;

  MPI_Comm_rank (com, &rank);

  char *lfsfilename;
  lfsfilename = malloc ((strlen (df) + 100) * sizeof (char));
  sprintf (lfsfilename, "%s%d.log", df, rank);
  fd->filename = strdup (lfsfilename);
  fd->filename[strlen (fd->filename) - 4] = 0;

  fd->mother_file = strdup (df);
  // TODO work for read-only, write/read workflows, too.
  ret = access (fd->filename, F_OK);

	assert(((flags & O_TRUNC) && (flags & O_WRONLY || flags & O_RDWR))  || (flags & O_RDWR) || (flags == O_RDONLY));
	//if (flags & O_WRONLY)

	fd->log_file = fopen(lfsfilename, "a+");     // XXX: I had to change it to a+ from w. because for readying I want to read it and with w I couldn't! is it has to be w then I'll open it in r for the read as temp variable and close it when reading ends.

  fd->file_position = 0;
  fd->proc_rank = rank;
  fd->current_epoch = 0;
  ret = MPI_Comm_dup (com, &fd->com);
  assert (ret == MPI_SUCCESS);
	free(lfsfilename);

	if (flags & O_TRUNC && rank == 0){
		fd->data_file = open (fd->filename, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);

		// TODO check if the process count is identical
		int mof = open(fd->mother_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR );
		int size;
		MPI_Comm_size (fd->com, & size);
		write(mof, & size, sizeof(size));
		write(mof, & fd->current_epoch, sizeof(fd->current_epoch));
		close(mof);
	}else{
		fd->data_file = open (fd->filename, O_RDWR, S_IRUSR | S_IWUSR);
	}

  return 0;
}

void
lfs_mpi_next_epoch (lfs_mpi_file_p fd)
{
  fd->current_epoch++;
  off_t offset_zero = 0;
  size_t size_zero = 0;

	// we have a special marker (offset, size) = (0,0) to indicate that the next epoch starts
  fwrite (&offset_zero, sizeof(offset_zero), 1, fd->log_file);
  fwrite (&size_zero, sizeof(size_zero), 1, fd->log_file);

  //struct timeval tv;
  //gettimeofday (&tv, NULL);
  //long long time_in_mill = tv.tv_sec * 1000 + tv.tv_usec / 1000;
  //printf ("I'm Proc %d and I've reached the barrier! %lld\n", fd->proc_size, time_in_mill);

	// Probably we do not need it for write only.
	// using mpi_allreduce to inform others (mainly proc 0) about next epoch status
  MPI_Barrier(fd->com);

	// now rank 0 updates the global visible epoch
	if (fd->proc_rank == 0){
		int mof = open(fd->mother_file, O_WRONLY);
		pwrite(mof, & fd->current_epoch, sizeof(fd->current_epoch), sizeof(int));
		close(mof);
	}

  return;
}

// this is the LFS write function
size_t
lfs_mpi_write (lfs_mpi_file_p fd, char *buf, size_t count, off_t offset)
{
  size_t count1 = count;
  off_t offset1 = fd->file_position;
  size_t ret;

  // perfoming the lfs write ---> appending the data into the END OF FILE
  while (count1 > 0) {
    ret = pwrite (fd->data_file, buf, count1, offset1);
    if (ret != count1) {
      if (ret == -1) {
        if (errno == EINTR) {
          continue;
        }
				// we revert the status, we either write all or nothing, thus we re not updating the file position
        return 0;
      }
    }
    buf += ret;
    count1 -= ret;
    offset1 += ret;
  }
	// now update the log file position
  fd->file_position = offset1;

  // writing the mapping info for the written data to know it's exact address later
  fwrite (&offset, sizeof (offset), 1, fd->log_file);
  fwrite (&count, sizeof (count), 1, fd->log_file);
  return ret;
}


// extracts the mapping dict from our metadata(log) file
int read_record (struct lfs_record **rec, FILE * fd, int depth)
{
  int ret;
  // find the number of items in the array by using the size of the metadata file
  size_t fileLen;
  fseek (fd, 0, SEEK_END);
  fileLen = ftell (fd);
  fseek (fd, 0, SEEK_SET);
  int record_count = fileLen / sizeof (lfs_record_on_disk);
  lfs_record *records = (lfs_record *) malloc (sizeof (lfs_record) * record_count);
  size_t file_position = 0;
	// printf("here rec_count: %d\n", record_count);
  // filling the created array with the values inside the metadata file
  for (int i = 0; i < record_count; i++) {
    ret = fread (&records[i], sizeof (lfs_record_on_disk), 1, fd);
    records[i].pos = file_position;
    assert(ret == 1);
    file_position += records[i].size;
//              printf("this is record in read_rec_func: (%zu, %zu, %zu)\n", records[i].addr, records[i].size, records[i].pos);
  }                             // end of FOR
  // selecting the wanted EPOCH in the recod
  int begin = record_count - 1, end = record_count - 1;
  for (int i = record_count - 1; i >= 0; i--) {
    if (records[i].addr == 0 && records[i].size == 0) {
      depth--;
      end = begin;
      begin = i;
      if (depth < 0)
        break;
      // end of IF
    }                           // end of IF
    if (i == 0) {
      end = begin;
      begin = -1;
    }                           // end of IF
  }                             // end of FOR
//      printf("begin: %d, end: %d\n", begin, end);
  *rec = (lfs_record *) malloc (sizeof (lfs_record) * (end - begin));
  memcpy (*rec, &records[begin + 1], sizeof (lfs_record) * (end - begin)); // why memcpy here? why is there the need for begin & end?
	free(records);
  return end - begin;
}


// finds the common are between two given tuples
struct tup
compare_tup (struct tup first, struct tup second)
{
  struct tup res;
  res.a = -1;
  res.b = -1;
  //printf("tups: (%d,%d) and (%d,%d)\n", first.a, first.b, second.a, second.b);
  if (first.a <= second.a && first.b > second.a) {
    if (first.b <= second.b) {
      res.a = second.a;
      res.b = first.b;
    }
    else {
      res.a = second.a;
      res.b = second.b;
    }
  }
  if (first.a >= second.a && first.a < second.b) {
    if (first.b <= second.b) {
      res.a = first.a;
      res.b = first.b;
    }
    else {
      res.a = first.a;
      res.b = second.b;
    }
  }
  //printf("res in here: %d, %d    ", res.a, res.b);
  return res;
}


// recursive function that finds all of the areas that should be read to complete a read query
int
lfs_mpi_find_chunks (size_t a, size_t b, int index, struct lfs_record *my_recs, struct lfs_record **chunks_stack, int *ch_s, struct lfs_record **missing_chunks, int *m_ch_s)
{
//      printf("check it out: %zu %zu, %d\n", a, b, index);

  // this IF is for ending the recursion
  if (a == b)
    return 0;
  struct tup res, rec, query;
  res.a = -1;
  res.b = -1;
  query.a = a;
  query.b = b;
  // search through the logs until you find a record that overlaps with the given query area
  while (res.a == -1) {
    //if(a >= 69492736)
    //      printf("index is: %d\n",index);
    if (index < 0) {
      /*printf("check it out: %lu %lu, %d\n", a, b, index);
         //printf("didn't find\n");
         //return 0;
         struct lfs_record found;
         //printf("result: %lu, %lu\n", res.a, res.b);
         found.addr = a;
         found.size = b - a;
         found.pos = a;
         //chunks_stack.push_back(found);
         lfs_vec_add(chunks_stack, ch_s, found); */
      struct lfs_record miss;
      //printf("result: %lu, %lu\n", res.a, res.b);
      miss.addr = a;
      miss.size = b - a;
      miss.pos = a;
      lfs_vec_add (missing_chunks, m_ch_s, miss);
      return -1;

    }
    rec.a = my_recs[index].addr;
    rec.b = my_recs[index].addr + my_recs[index].size;
    res = compare_tup (query, rec);
    index--;
  }
  struct lfs_record found;
  //printf("result: %lu, %lu\n", res.a, res.b);
  found.addr = res.a;
  found.size = res.b - res.a;
  found.pos = my_recs[index + 1].pos + res.a - my_recs[index + 1].addr;
  //chunks_stack.push_back(found);
  lfs_vec_add (chunks_stack, ch_s, found);
  // call yourself for the remaing areas of the query that have not been covered with the found record
  lfs_mpi_find_chunks (a, res.a, index, my_recs, chunks_stack, ch_s, missing_chunks, m_ch_s);
  lfs_mpi_find_chunks (res.b, b, index, my_recs, chunks_stack, ch_s, missing_chunks, m_ch_s);
  return 0;
}

size_t
lfs_mpi_internal_read (int fd, char *buf, struct lfs_record ** query, int *q_index, struct lfs_record * rec, int record_count, struct lfs_record ** missing_chunks, int *m_ch_s,
                       off_t main_addr)
{
  struct lfs_record *chunks_stack;
  size_t count1;                //= count;
  off_t offset1;                //= offset;
  size_t ret;
  struct lfs_record temp;
  chunks_stack = (lfs_record *) malloc (sizeof (lfs_record) * 1001);
  int ch_s = 1;
  *missing_chunks = (lfs_record *) malloc (sizeof (lfs_record) * 1001);
//      printf("internal_read: allocations are done, q_index is %d\n", *q_index - 1);
  for (int i = 0; i < (*q_index - 1); i++) {
    temp = (*query)[i];
//              printf("internal_read: query is: %zu %zu %zu\n", temp.addr, temp.size ,temp.pos);
    lfs_mpi_find_chunks (temp.addr, temp.addr + temp.size, record_count - 1, rec, &chunks_stack, &ch_s, missing_chunks, m_ch_s);
  }                             // end of FOR
//      printf("internal_read: total found chunks = %d\n", ch_s - 1);
  int total_found_count = ch_s - 1;
  // perform the read
  for (int i = 0; i < total_found_count; i++) {
    temp = chunks_stack[i];
//                printf("chunk stack: (%zu, %zu, %zu)\n", temp.addr, temp.size, temp.pos);

    //buf = &buf[(temp.addr - main_addr)/sizeof(char)];
    count1 = temp.size;
    offset1 = temp.pos;
    while (count1 > 0) {
      ret = pread (fd, &buf[(temp.addr - main_addr) / sizeof (char)], count1, offset1);
      if (ret != count1) {

        if (ret == -1 ) {
          if (errno == EINTR) {
            continue;
          }                     // end of IF
          return temp.size - count1;
        }                       // end of IF
        if (ret == 0 ) {
          return temp.size - count1;
        }                       // end of IF
      }                         // end of IF
      temp.addr += ret * sizeof (char);
      count1 -= ret;
      offset1 += ret;
    }                           // end of WHILE
  }                             // end of FOR
//      printf("internal_read: freeing\n");
  free (chunks_stack);
  return 0;
}

// main function for read query
size_t
lfs_mpi_read (lfs_mpi_file_p fd, char *buf, size_t count, off_t offset)
{
  struct lfs_record temp;
  struct lfs_record *my_recs;
  int my_recs_size;
  int length_num;
  char *proc_name;
  struct lfs_record *query;
  struct lfs_record *swap_help;
  struct lfs_record *missing_chunks;
  int m_ch_s = 1;
  int proc_size;
  int query_index = 1;
  int missing_count = 1;
  FILE *temp_log_file;
  int temp_data_file;
  char *filename2;
  char *lfsfilename2;
  temp.addr = offset;
  temp.size = count;
  temp.pos = 0;
  // filling the query stack with the main query information.
  query = (lfs_record *) malloc (sizeof (lfs_record) * 1001);
  lfs_vec_add (&query, &query_index, temp);
  for (int i = 0; i <= fd->current_epoch && missing_count > 0; i++) {
    // get the records log with the epoch depth of i for the main file.
//              printf("get the records log with the epoch depth of %d for the main file\n", i);
    my_recs_size = read_record (&my_recs, fd->log_file, i);
//              printf("this is record in read_func: (%zu, %zu, %zu)\n", my_recs[0].addr, my_recs[0].size, my_recs[0].pos);
    // read from the main file, with epoch depth of i.
//              printf("read from the main file, with epoch depth of %d\n", i);
    lfs_mpi_internal_read (fd->data_file, buf, &query, &query_index, my_recs, my_recs_size, &missing_chunks, &m_ch_s, offset);
    // we no longer need the log so we free it.
//              printf("free 1\n");
    //my_recs[0];
//              printf("SDFSDFSDF\n");
    free (my_recs);
    missing_count = m_ch_s - 1;
//              printf("original missing_count is %d \n", missing_count);
//              printf("before if \n");
    if (missing_count == 0) {
			break;
		}
    // so we have some missing chunks, we have to find them in other files, WITH the SAME epoch depth.
//                      printf("we have some missing chunks %d\n", missing_count);
    // now we free the query stack and replace it with the missing stack (swap missing and query).
//                      printf("swap missing and query\n");
    free (query);
    swap_help = query;
    query = missing_chunks;
    query_index = m_ch_s;
    missing_chunks = swap_help;
    m_ch_s = 1;
    // checking all of the files and opening them to process the read.
		int epoch = 0;

		int mof = open(fd->mother_file, O_RDONLY);
		read(mof, & proc_size, sizeof(proc_size));
		read(mof, & epoch, sizeof(epoch));
		close(mof);
		for(int i=0; i < proc_size; i++){
      // check if it's not the same main file that belongs to this process.
      if (fd->proc_rank != i) {
        // prepare the name of the data and log files for the selected proc_size.
			  lfsfilename2 = malloc ((strlen(fd->mother_file) + 100) * sizeof (char));
			  sprintf (lfsfilename2, "%s%d.log", fd->mother_file, fd->proc_rank);

				filename2 = malloc ((strlen(fd->mother_file) + 10) * sizeof (char));
			  sprintf (filename2, "%s%d", fd->mother_file, fd->proc_rank);

				printf("%s %s\n", filename2, lfsfilename2);

        // open both data and log files for the selected proc_size.
        temp_log_file = fopen (lfsfilename2, "r");
				if (temp_log_file == NULL){
					printf("Error: %s\n", strerror(errno));
					assert(0);
				}
        temp_data_file = open (filename2, O_RDONLY);
				if (temp_data_file < 0){
					printf("Error: %s\n", strerror(errno));
					assert(0);
				}
        // now the file is open, so we start reading the query stack (previously missing stack) from the file.
        // first we need to get the records log with the epoch depth of i for the selected file.
        my_recs_size = read_record (&my_recs, temp_log_file, i);
        // perform the read.
        lfs_mpi_internal_read (temp_data_file, buf, &query, &query_index, my_recs, my_recs_size, &missing_chunks, &m_ch_s, offset);
        // now we free the file names and close the related files.
        fclose (temp_log_file);
        close (temp_data_file);
        free (filename2);
        free (lfsfilename2);
        free (query);
        // we no longer need the log so we free it.
        free (my_recs);
        // now again, we free the query stack and replace it with the missing stack (swap missing and query).
        missing_count = m_ch_s - 1;
        if (missing_count == 0){
          break;
				}
        swap_help = query;
        query = missing_chunks;
        query_index = m_ch_s;
        missing_chunks = swap_help;
        m_ch_s = 1;
      }                       // end of IF
		}
  }                             // end of FOR
  // Optimization: Write down the read query for future reads!
  /*if (total_found_count == -3){
     lfs_write(fd, buf, count, offset);
     } */
//      printf("salamaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
  return count;
}

void
lfs_vec_add (struct lfs_record **chunks_stack, int *size, struct lfs_record chunk)
{
  //printf("size is:%d\n", *size - 1);
//      printf("one\n");
  if (*size % 1000 == 0) {
//              printf("two\n");
    struct lfs_record *new_stack;
    //struct lfs_record* temp;
    int rtd = *size + 1000;
    //printf("allocating %d size\n", rtd);
    new_stack = (lfs_record *) realloc (*chunks_stack, sizeof (lfs_record) * rtd);
    if (new_stack == NULL){
      printf ("MEMORY ALLOCATION ERROR IN LFS STACK!!!\n");
		}
    /*for(int i=0; i < *size - 1; i++){
       new_stack[i].addr = chunks_stack[i].addr;
       new_stack[i].size = chunks_stack[i].size;
       new_stack[i].pos = chunks_stack[i].pos;
       } */
//              printf("three\n");
    //free(chunks_stack);
    *chunks_stack = new_stack;
    //printf("address of new chunk stack: %p\n", *chunks_stack);
    //printf("sozeof = %lu\n",sizeof(*new_stack));
    //free(temp);
  }
  //printf("address of current chunk stack: %p\n", *chunks_stack);

  (*chunks_stack)[*size - 1] = chunk;
  *size += 1;
  //printf("%d\n",*size);
//      printf("five\n");
  //printf("added chunk: %d, %d, %d\n", chunk.addr, chunk.size, chunk.pos);
  //printf("added chunk: %d, %d, %d\n", chunks_stack[*size - 2].addr, chunks_stack[*size - 2].size, chunks_stack[*size - 2].pos);
}

int
lfs_mpi_close (lfs_mpi_file_p fd)
{
  //printf("IN THE LFS_MPI_CLOSE\n");
  fclose (fd->log_file);
  //printf("IN middle of THE LFS_MPI_CLOSE\n");
  close (fd->data_file);
  //printf("IN THE end of the LFS_MPI_CLOSE\n");
  MPI_Comm_free (&fd->com);

	free(fd->filename);
  free(fd->mother_file);

	fd->filename = NULL;

  return 0;
}
