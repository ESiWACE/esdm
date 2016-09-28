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

int lfs_mpi_open(lfs_mpi_file_p * fd_p, char *df, int flags, mode_t mode, MPI_Comm com)
{
	*fd_p = (lfs_mpi_file_p) malloc(sizeof(struct lfs_file));

	lfs_mpi_file_p fd = *fd_p;
	int ret, rank, size;

	MPI_Comm_rank(MPI_COMM_WORLD, & rank);

	char * lfsfilename;
	lfsfilename = malloc((strlen(df) + 100) * sizeof(char));
	sprintf(lfsfilename, "%s%d.log", df, rank);
	fd->filename = strdup(lfsfilename);
	fd->filename[strlen(fd->filename) - 4] = 0;

	fd->mother_file = strdup(df);

	// TODO work for read-only, write/read workflows, too.
	ret = access(fd->filename, F_OK ) == -1;
	MPI_Allreduce(MPI_IN_PLACE, &ret, 1, MPI_INT, MPI_MIN, com);

	if( ! ret ) { // no file does exist so far
		return -1;
	}

	if(rank == 0) {
		MPI_Comm_size(MPI_COMM_WORLD, & size);

		FILE * meta_file = fopen (df, "w");
		fprintf(meta_file, "%d", size);
		fclose(meta_file);
	}

	fd->log_file = fopen(lfsfilename, "w"); // this is not working concurrently my friend
	fd->data_file = open(fd->filename, O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR);
	fd->file_position = 0;
	fd->proc_rank = rank;
	fd->current_epoch = 0;
	ret = MPI_Comm_dup(com, & fd->com);
	assert(ret == MPI_SUCCESS);
	return 0;
}

void lfs_mpi_next_epoch(lfs_mpi_file_p fd){
	fd->current_epoch++;
	off_t offset_zero = 0;
	size_t size_zero = 0;

	fwrite(& offset_zero, sizeof(offset_zero), 1, fd->log_file);
	fwrite(& size_zero, sizeof(size_zero), 1, fd->log_file);

	struct timeval  tv;
	gettimeofday(&tv, NULL);
	long long time_in_mill = tv.tv_sec*1000 + tv.tv_usec/1000;
	printf("I'm Proc %d and I've reached the barrier! %lld\n", fd->proc_rank, time_in_mill);

	// Probably we do not need it for write only.
	MPI_Barrier(MPI_COMM_WORLD);

	gettimeofday(&tv, NULL);
	time_in_mill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	printf("Proc %d released! %lld\n", fd->proc_rank, time_in_mill);
}

// this is the LFS write function
size_t lfs_mpi_write(lfs_mpi_file_p fd, void *buf, size_t count, off_t offset){
	int ret = 0;
	size_t data_size;
	data_size = count;

	// perfoming the lfs write ---> appending the data into the END OF FILE
	// TODO PROPER POSIX WRITE CYCLE LOOP, SEE MY CHANGES IN LFS.c
	ret = pwrite(fd->data_file, buf, data_size, fd->file_position);
	fd->file_position += ret;
	// writing the mapping info for the written data to know it's exact address later
	fwrite(& offset, sizeof(offset), 1, fd->log_file);
	fwrite(& data_size, sizeof(data_size), 1, fd->log_file);
	return ret;
}


// extracts the mapping dict from our metadata(log) file
int read_record(struct lfs_record ** rec, FILE* fd, int depth){
	int ret;
	// find the number of items in the array by using the size of the metadata file
  	size_t fileLen;
	fseek(fd, 0, SEEK_END);
	fileLen = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	int record_count = fileLen / sizeof(lfs_record_on_disk);
	lfs_record * records = (lfs_record *)malloc(sizeof(lfs_record) * record_count);
	size_t file_position = 0;
//	printf("here rec_count: %d\n", record_count);
	// filling the created array with the values inside the metadata file
	for(int i=0; i < record_count; i++){
		ret = fread(&records[i], sizeof(lfs_record_on_disk), 1, fd);
		records[i].pos = file_position;
		assert(ret == 1);
		file_position += records[i].size;
//		printf("this is record in read_rec_func: (%zu, %zu, %zu)\n", records[i].addr, records[i].size, records[i].pos);
	} // end of FOR
	// selecting the wanted EPOCH in the recod
	int begin = record_count - 1, end = record_count - 1;
	for(int i = record_count - 1; i >= 0; i--){
		if(records[i].addr == 0 && records[i].size == 0){
			depth--;
			end = begin;
			begin = i;
			if(depth < 0)
				break;
			// end of IF
		} // end of IF
		if(i == 0){
			end = begin;
			begin = -1;
		} // end of IF
	} // end of FOR
//	printf("begin: %d, end: %d\n", begin, end);
	*rec = (lfs_record *)malloc(sizeof(lfs_record) * (end - begin));
	memcpy(*rec, &records[begin + 1], sizeof(lfs_record) * (end - begin));
	return end - begin;
}


// finds the common are between two given tuples
struct tup compare_tup(struct tup first, struct tup second){
	struct tup res;
	res.a = -1;
	res.b = -1;
	//printf("tups: (%d,%d) and (%d,%d)\n", first.a, first.b, second.a, second.b);
	if (first.a <= second.a && first.b > second.a){
		if (first.b <= second.b){
			res.a = second.a;
			res.b = first.b;
		}
		else{
			res.a = second.a;
			res.b = second.b;
		}
	}
	if (first.a >= second.a && first.a < second.b){
		if (first.b <= second.b){
			res.a = first.a;
			res.b = first.b;
		}
		else{
			res.a = first.a;
			res.b = second.b;
		}
	}
	//printf("res in here: %d, %d    ", res.a, res.b);
	return res;
}


// recursive function that finds all of the areas that should be read to complete a read query
int lfs_mpi_find_chunks(size_t a, size_t b, int index, struct lfs_record * my_recs, struct lfs_record ** chunks_stack, int* ch_s, struct lfs_record ** missing_chunks, int* m_ch_s){
//	printf("check it out: %zu %zu, %d\n", a, b, index);

	// this IF is for ending the recursion
	if(a == b)
		return 0;
	struct tup res, rec, query;
	res.a = -1;
	res.b = -1;
	query.a = a;
	query.b = b;
	// search through the logs until you find a record that overlaps with the given query area
	while(res.a == -1){
		//if(a >= 69492736)
		//	printf("index is: %d\n",index);
		if(index < 0){
			/*printf("check it out: %lu %lu, %d\n", a, b, index);
			//printf("didn't find\n");
			//return 0;
			struct lfs_record found;
        		//printf("result: %lu, %lu\n", res.a, res.b);
		        found.addr = a;
		        found.size = b - a;
	        	found.pos = a;
        		//chunks_stack.push_back(found);
	        	lfs_vec_add(chunks_stack, ch_s, found);*/
			struct lfs_record miss;
		        //printf("result: %lu, %lu\n", res.a, res.b);
		        miss.addr = a;
        		miss.size = b - a;
	        	miss.pos = a;
			lfs_vec_add(missing_chunks, m_ch_s, miss);
       			return -1;

		}
		rec.a = my_recs[index].addr;
		rec.b = my_recs[index].addr + my_recs[index].size;
		res = compare_tup(query, rec);
		index--;
	}
	struct lfs_record found;
	//printf("result: %lu, %lu\n", res.a, res.b);
	found.addr = res.a;
	found.size = res.b - res.a;
	found.pos = my_recs[index + 1].pos + res.a - my_recs[index + 1].addr;
	//chunks_stack.push_back(found);
	lfs_vec_add(chunks_stack, ch_s, found);
	// call yourself for the remaing areas of the query that have not been covered with the found record
	lfs_mpi_find_chunks(a, res.a,index, my_recs, chunks_stack, ch_s, missing_chunks, m_ch_s);
	lfs_mpi_find_chunks(res.b, b,index, my_recs, chunks_stack, ch_s, missing_chunks, m_ch_s);
	return 0;
}

size_t lfs_mpi_internal_read(lfs_mpi_file_p fd, char *buf, struct lfs_record ** query, int* q_index, struct lfs_record * rec, int record_count, struct lfs_record ** missing_chunks, int* m_ch_s, off_t main_addr){
	struct lfs_record * chunks_stack;
	struct lfs_record temp;
        chunks_stack = (lfs_record *)malloc(sizeof(lfs_record) * 1001);
	int ch_s = 1;
	*missing_chunks = (lfs_record *)malloc(sizeof(lfs_record) * 1001);
//	printf("internal_read: allocations are done, q_index is %d\n", *q_index - 1);
	for(int i = 0; i < (*q_index - 1); i++){
		temp = (*query)[i];
//		printf("internal_read: query is: %zu %zu %zu\n", temp.addr, temp.size ,temp.pos);
		lfs_mpi_find_chunks(temp.addr, temp.addr + temp.size, record_count - 1, rec, &chunks_stack, &ch_s, missing_chunks, m_ch_s);
	} // end of FOR
//	printf("internal_read: total found chunks = %d\n", ch_s - 1);
	int total_found_count = ch_s - 1;
        // perform the read
        for(int i = 0; i < total_found_count; i++){
                temp = chunks_stack[i];
//                printf("chunk stack: (%zu, %zu, %zu)\n", temp.addr, temp.size, temp.pos);
                pread(fd, &buf[(temp.addr - main_addr)/sizeof(char)], temp.size, temp.pos);
                } // end of FOR
//	printf("internal_read: freeing\n");
        free(chunks_stack);
        return 0;
}

// main function for read query
size_t lfs_mpi_read(lfs_mpi_file_p fd, char *buf, size_t count, off_t offset){
//	printf("entering mpi read\n");
	struct lfs_record temp;
        struct lfs_record * my_recs;
	int my_recs_size;
	int length_num;
	char * proc_name;
	struct lfs_record * query;
	struct lfs_record * swap_help;
	struct lfs_record * missing_chunks;
        int m_ch_s = 1;
	int proc_rank;
	int query_index = 1;
	int missing_count = 1;
	FILE * temp_log_file;
        int temp_data_file;
	FILE * meta_file;
	char * filename2;
	char * lfsfilename2;
	temp.addr = offset;
	temp.size = count;
	temp.pos = 0;
	// filling the query stack with the main query information.
	query = (lfs_record *)malloc(sizeof(lfs_record) * 1001);
	lfs_vec_add(&query, &query_index, temp);
	for(int i = 0; i <= current_epoch && missing_count > 0; i++){
		// get the records log with the epoch depth of i for the main file.
//		printf("get the records log with the epoch depth of %d for the main file\n", i);
		my_recs_size = read_record(&my_recs, lfsfiles[fd].log_file, i);
//		printf("this is record in read_func: (%zu, %zu, %zu)\n", my_recs[0].addr, my_recs[0].size, my_recs[0].pos);
		// read from the main file, with epoch depth of i.
//		printf("read from the main file, with epoch depth of %d\n", i);
		lfs_mpi_internal_read(lfsfiles[fd].data_file, buf, &query, &query_index, my_recs, my_recs_size, &missing_chunks, &m_ch_s, offset);
		// we no longer need the log so we free it.
//		printf("free 1\n");
		//my_recs[0];
//		printf("SDFSDFSDF\n");
		free(my_recs);
		missing_count = m_ch_s - 1;
//		printf("original missing_count is %d \n", missing_count);
//		printf("before if \n");
		if(missing_count > 0){
			// so we have some missing chunks, we have to find them in other files, WITH the SAME epoch depth.
//			printf("we have some missing chunks %d\n", missing_count);
			// now we free the query stack and replace it with the missing stack (swap missing and query).
//			printf("swap missing and query\n");
	                free(query);
                        swap_help = query;
                        query = missing_chunks;
                        query_index = m_ch_s;
                        missing_chunks = swap_help;
                        m_ch_s = 1;
			// checking all of the files and opening them to process the read.
			meta_file = fopen (mother_file, "r");
			fscanf(meta_file, "%d", &proc_rank);
                	while(!feof(meta_file)) {
				// check if it's not the same main file that belongs to this process.
                        	if(lfsfiles[fd].proc_rank != proc_rank){
//					printf("prepare the name of the data and log files for %d.\n", proc_rank);
					// prepare the name of the data and log files for the selected proc_rank.
	                                length_num = snprintf(NULL, 0, "%d", proc_rank);
        	                        proc_name = (char *)malloc(length_num + 1);
                	                snprintf(proc_name, length_num + 1, "%d", proc_rank);
                        	        lfsfilename2 = (char *)malloc((strlen(mother_file) + strlen(proc_name) + 4) * sizeof(char));
                        	        strcpy(lfsfilename2, mother_file);
                        	        strcat(lfsfilename2, proc_name);
                        	        strcat(lfsfilename2, ".log");
                        	        filename2 = (char *)malloc((strlen(mother_file) + strlen(proc_name)) * sizeof(char));
                        	        strcpy(filename2, mother_file);
                        	        strcat(filename2, proc_name);
					free(proc_name);
					// open both data and log files for the selected proc_rank.
                        	        temp_log_file = fopen(lfsfilename2, "r");
                        	        temp_data_file = open(filename2, O_RDONLY);
					// now the file is open, so we start reading the query stack (previously missing stack) from the file.
					// first we need to get the records log with the epoch depth of i for the selected file.
					my_recs_size = read_record(&my_recs, temp_log_file, i);
//					printf("rec size is: %d now perform the read\n", my_recs_size);
					// perform the read.
					lfs_mpi_internal_read(temp_data_file, buf, &query, &query_index, my_recs, my_recs_size, &missing_chunks, &m_ch_s, offset);
					// now we free the file names and close the related files.
					fclose(temp_log_file);
					close(temp_data_file);
					free(filename2);
					free(lfsfilename2);
					free(query);
					// we no longer need the log so we free it.
//					printf("free 2\n");
					free(my_recs);
					//printf("HIIIIIII\n");
					// now again, we free the query stack and replace it with the missing stack (swap missing and query).
					missing_count = m_ch_s - 1;
 //                                       printf("HIIIIIII44444, missing_count = %d\n", missing_count);
                                        if(missing_count == 0)
                                                break;
					swap_help = query;
                                        query = missing_chunks;
                                        query_index = m_ch_s;
                                        missing_chunks = swap_help;
					//printf("HIIIIIII22222\n");
                                        m_ch_s = 1;
//					printf("HIIIIIII333333\n");
				} // end of IF
				fscanf(meta_file, "%d", &proc_rank);
			} // end of WHILE
			fclose(meta_file);
		}// end of IF
	} // end of FOR
        // Optimization: Write down the read query for future reads!
        /*if (total_found_count == -3){
                lfs_write(fd, buf, count, offset);
        }*/
//	printf("salamaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
        return count;
}

void lfs_vec_add(struct lfs_record** chunks_stack, int * size, struct lfs_record chunk){
	//printf("size is:%d\n", *size - 1);
//	printf("one\n");
	if(*size % 1000 == 0){
//		printf("two\n");
		struct lfs_record* new_stack;
		//struct lfs_record* temp;
		int rtd = *size + 1000;
		//printf("allocating %d size\n", rtd);
		new_stack = (lfs_record *)realloc(*chunks_stack, sizeof(lfs_record) * rtd);
		if(new_stack == NULL)
			printf("MEMORY ALLOCATION ERROR IN LFS STACK!!!\n");
		/*for(int i=0; i < *size - 1; i++){
			new_stack[i].addr = chunks_stack[i].addr;
			new_stack[i].size = chunks_stack[i].size;
			new_stack[i].pos = chunks_stack[i].pos;
		}*/
//		printf("three\n");
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
//	printf("five\n");
	//printf("added chunk: %d, %d, %d\n", chunk.addr, chunk.size, chunk.pos);
	//printf("added chunk: %d, %d, %d\n", chunks_stack[*size - 2].addr, chunks_stack[*size - 2].size, chunks_stack[*size - 2].pos);
}

int  lfs_mpi_close(lfs_mpi_file_p fd){
//	printf("IN THE LFS_MPI_CLOSE\n");
	fclose(lfsfiles[handle].log_file);
//printf("IN middle of THE LFS_MPI_CLOSE\n");
	close(lfsfiles[handle].data_file);
//printf("IN THE end of the LFS_MPI_CLOSE\n");
  MPI_Comm_free(& lfsfiles[current_index].com);
	return 0;
}
