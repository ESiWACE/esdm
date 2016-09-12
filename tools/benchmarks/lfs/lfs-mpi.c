#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <mpi.h>

#include <lfs-mpi-internal.h>

char * filename;
char * lfsfilename;
//std::string temp = "datafile.df";
//filename = strdup(temp.c_str());
//temp = "metafile.mf";
//lfsfilename = strdup(temp.c_str());


struct lfs_files lfsfiles[20];
int current_index = 1;
char * mother_file;

int lfs_mpi_open(int proc_num, char *df, int flags, mode_t mode)
{
	int length_num = snprintf(NULL, 0, "%d", proc_num);
	printf("length: %d\n", length_num);
	char * proc_name = malloc(length_num + 1);
	snprintf(proc_name, length_num + 1, "%d", proc_num);
        printf("proc_name: %s !!!\n", proc_name);
	filename = malloc((strlen(df) + strlen(proc_name)) * sizeof(char));
	strcpy(filename, df);
        strcat(filename, proc_name);
	lfsfilename = (char *) malloc((strlen(filename) + 4) * sizeof(char));
	strcpy(lfsfilename, filename);
	strcat(lfsfilename, ".log");
	printf("filename: %s\n", filename);
	printf("lfsfilename: %s\n", lfsfilename);
	mother_file = strdup(df);
	if(access(filename, F_OK ) == -1) {
		FILE * meta_file = fopen (df, "a+");
		fwrite(proc_name, sizeof(char) * length_num, 1, meta_file);
		fwrite("\n", sizeof(char), 1, meta_file);
		fclose(meta_file);
	}
	lfsfiles[current_index].log_file = fopen(lfsfilename, "a+");
	lfsfiles[current_index].data_file = open(filename, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
	lfsfiles[current_index].proc_num = proc_num;
	current_index++;
	free(filename);
	free(lfsfilename);
	free(proc_name);
	return current_index - 1;
}

// this is the LFS write function
ssize_t lfs_mpi_write(int fd, void *buf, size_t count, off_t offset){
	int ret = 0;
	size_t data_size;
	data_size = count;
	// determining the END OF FILE exact address
	struct stat stats;
	//stat(fdp->data_file, & stats);
	fstat(lfsfiles[fd].data_file, &stats);
	size_t  end_of_file = stats.st_size;
	// perfoming the lfs write ---> appending the data into the END OF FILE
	ret = pwrite(lfsfiles[fd].data_file, buf, data_size, end_of_file);
	// writing the mapping info for the written data to know it's exact address later
	fwrite(& offset, sizeof(offset), 1, lfsfiles[fd].log_file);
	fwrite(& data_size, sizeof(data_size), 1, lfsfiles[fd].log_file);
	return ret;
}


// extracts the mapping dict from our metadata(log) file
lfs_record * read_record(int fd){
	int ret;
	// find the number of items in the array by using the size of the metadata file
  	//struct stat stats;
	//ret = fstat(myfd, & stats);
	//printf("this is FD %d\n",fd);
	size_t fileLen;
	fseek(lfsfiles[fd].log_file, 0, SEEK_END);
	fileLen = ftell(lfsfiles[fd].log_file);
	fseek(lfsfiles[fd].log_file, 0, SEEK_SET);
	//assert(ret == 0);
	int record_count = fileLen / sizeof(lfs_record_on_disk);
	//printf("this is size %d\n",record_count);
	lfs_record * records = (lfs_record *)malloc(sizeof(lfs_record) * record_count);
	size_t file_position = 0;

	// filling the created array with the values inside the metadata file
	//  FILE * lfs = fopen(lfsfilename, "r");
	for(int i=0; i < record_count; i++){
		ret = fread(& records[i], sizeof(lfs_record_on_disk), 1, lfsfiles[fd].log_file);
		records[i].pos = file_position;
		assert(ret == 1);
		file_position += records[i].size;
	}
	//fclose(lfs);
	return records;
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
	//printf("check it out: %zu %zu, %d\n", a, b, index);

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


// main function for read query
size_t lfs_mpi_read(int fd, char *buf, size_t count, off_t offset){
//size_t lfs_read(size_t addr, size_t size, char * res){
	//int fd = open(filename, O_RDONLY);
	struct lfs_record temp;
	//int rtrd;
	// get the latest updated logs
	struct lfs_record * my_recs = read_record(fd);

	// create the vector that acts like a stack for our finding chunks recursive function
	struct lfs_record * chunks_stack;
	chunks_stack = (lfs_record *)malloc(sizeof(lfs_record) * 1001);
	int ch_s = 1;
	struct lfs_record * missing_chunks;
        missing_chunks = (lfs_record *)malloc(sizeof(lfs_record) * 1001);
        int m_ch_s = 1;
	// find the length of the log array
	//struct stat stats;
	// int	myfd;
	//myfd = fileno(lfsfiles[fd].log_file);
	//	fstat(myfd, & stats);
	size_t fileLen;
	fseek(lfsfiles[fd].log_file, 0, SEEK_END);
	fileLen = ftell(lfsfiles[fd].log_file);
	fseek(lfsfiles[fd].log_file, 0, SEEK_SET);
	int record_count = fileLen / sizeof(lfs_record_on_disk);
//	for(int mm = 0; mm < record_count; mm++)
//		printf("rec: %lu, %lu, %lu\n", my_recs[mm].addr, my_recs[mm].size, my_recs[mm].pos);
	// call the recursive function to find which areas need to be read
	lfs_mpi_find_chunks(offset, offset + count, record_count - 1, my_recs, &chunks_stack, &ch_s, &missing_chunks, &m_ch_s);
	int total_miss_count = m_ch_s - 1;
	if(total_miss_count > 0){
		printf("miss count > 0 \n");
		FILE * meta_file = fopen (mother_file, "r");
		int proc_rank, length_num;
		char * proc_name;
		char * lfsfilename2;
		char * filename2;
		int found_index;
		struct lfs_record temp2;
                struct lfs_record * my_recs2;
                struct lfs_record * chunks_stack2;
                int ch_s2 = 1;
                struct lfs_record * missing_chunks2;
                int m_ch_s2 = 1;
                size_t fileLen2;
                int record_count2;
                int total_found_count2;
		fscanf(meta_file, "%d", &proc_rank);
		while(!feof(meta_file)) {
			printf("this is rank: %d !!!\n", proc_rank);
			for (int j = current_index - 1; j >= 1; j--)
				if(lfsfiles[j].proc_num == proc_rank){
					if(fcntl(lfsfiles[j].data_file, F_GETFL) != -1){
						found_index = j;
						printf("found_open\n");
						break;
					} else {
						printf("found_not_open\n");
						length_num = snprintf(NULL, 0, "%d", proc_rank);
						proc_name = malloc(length_num + 1);
						snprintf(proc_name, length_num + 1, "%d", proc_rank);
						printf("proc_name: %s !!!\n", proc_name);
					        lfsfilename2 = malloc((strlen(mother_file) + strlen(proc_name) + 4) * sizeof(char));
					        strcpy(lfsfilename2, mother_file);
					        strcat(lfsfilename2, proc_name);
		        			strcat(lfsfilename2, ".log");
						filename2 = malloc((strlen(mother_file) + strlen(proc_name)) * sizeof(char));
                        	                strcpy(filename2, mother_file);
                               	        	strcat(filename2, proc_name);
						printf("filename: %s\n", filename2);
                                                printf("lfsfilename: %s\n", lfsfilename2);
						lfsfiles[current_index].log_file = fopen(lfsfilename2, "a+");
						lfsfiles[current_index].data_file = open(filename2, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
						lfsfiles[current_index].proc_num = proc_rank;
						current_index++;
						found_index = current_index - 1;
						break;
					} //end of IF/ELSE
				} // end of IF
			// end of FOR
			my_recs2 = read_record(found_index);
		        chunks_stack2 = (lfs_record *)malloc(sizeof(lfs_record) * 1001);
		        ch_s2 = 1;
		        missing_chunks2 = (lfs_record *)malloc(sizeof(lfs_record) * 1001);
		        m_ch_s2 = 1;
			fseek(lfsfiles[found_index].log_file, 0, SEEK_END);
		        fileLen2 = ftell(lfsfiles[found_index].log_file);
		        fseek(lfsfiles[found_index].log_file, 0, SEEK_SET);
		        record_count2 = fileLen2 / sizeof(lfs_record_on_disk);
			for(int jj = 0; jj <= total_miss_count; jj++){
				temp2 = missing_chunks[jj];
				lfs_mpi_find_chunks(temp.addr, temp2.addr + temp2.size, record_count2 - 1, my_recs2, &chunks_stack2, &ch_s2, &missing_chunks2, &m_ch_s2);
				total_found_count2 = ch_s2 - 1;
				for(int p = 0; p < total_found_count2; p++){
			                temp2 = chunks_stack2[p];
			                printf("chunk stack: (%zu, %zu, %zu)\n",temp2.addr,temp2.size,temp2.pos);
					printf("found proc num: %d !!!\n", lfsfiles[found_index].proc_num);
			                pread(lfsfiles[found_index].data_file, &buf[(temp2.addr - offset)/sizeof(char)], temp2.size, temp2.pos);
			                //printf("rtrd is: %d\n",rtrd);
        			} // end of FOR
			} // end of FOR
			free(chunks_stack2);
			free(missing_chunks2);
			free(my_recs2);
		fscanf(meta_file, "%d", &proc_rank);
		} // end of WHILE
	} //end of IF
	int total_found_count = ch_s - 1;
	//printf("recursive func finished\n");
	// perform the read
	for(int i = 0; i < total_found_count; i++){
		temp = chunks_stack[i];
		//printf("chunk stack: (%zu, %zu, %zu)\n",temp.addr,temp.size,temp.pos);
		pread(lfsfiles[fd].data_file, &buf[(temp.addr - offset)/sizeof(char)], temp.size, temp.pos);
		//printf("rtrd is: %d\n",rtrd);
	} // end of FOR
	//free(chunks_stack);
	free(my_recs);
	// Optimization: Write down the read query for future reads!
	if (total_found_count == -3 /* THRESHOLD */){
		lfs_mpi_write(fd, buf, count, offset);
	} // end of IF
	free(chunks_stack);
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
	//printf("five\n");
	//printf("added chunk: %d, %d, %d\n", chunk.addr, chunk.size, chunk.pos);
	//printf("added chunk: %d, %d, %d\n", chunks_stack[*size - 2].addr, chunks_stack[*size - 2].size, chunks_stack[*size - 2].pos);
}

int  lfs_mpi_close(int  handle){
	fclose(lfsfiles[handle].log_file);
	close(lfsfiles[handle].data_file);
	return 0;
}
