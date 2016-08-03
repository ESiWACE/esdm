#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>


#include <lfs-internal.h>

char * filename;
char * lfsfilename;
size_t block_size;
//std::string temp = "datafile.df";
//filename = strdup(temp.c_str());
//temp = "metafile.mf";
//lfsfilename = strdup(temp.c_str());




int lfs_open(char* df, char* mf)
{
	filename = strdup(df);
	lfsfilename = strdup(mf);

	printf("filename: %s\n", filename);
	printf("lfsfilename: %s\n", lfsfilename);

	return 0;
}


int lfs_set_blocksize(size_t blocksize)
{
	block_size = blocksize;

	// potential error code when setting new blocksize fails ;)
	return 0;
}


void normal_write(size_t addr, char * data){
	int fd = open(filename, O_CREAT|O_APPEND|O_RDWR, S_IRUSR|S_IWUSR);
	size_t data_size;
  data_size = strlen(data);
	pwrite(fd, data, data_size * sizeof(char), addr);
	close(fd);
}


size_t normal_read(size_t addr, size_t size, char * res){
	int fd = open(filename, O_RDONLY);
	pread(fd, res, size, addr);
	return size;
}


// this is the LFS write function
void lfs_write(size_t addr, char * data){
  int fd = open(filename, O_CREAT|O_APPEND|O_RDWR, S_IRUSR|S_IWUSR);
  FILE * lfs = fopen(lfsfilename, "a");
  size_t ret = 0;

  size_t data_size;
  size_t block_num;
  data_size = strlen(data);
	// printf("give data size to lfs_write: %d\n", data_size);
  block_num /= block_size;  // we assume that data length is multiple of block_size !!!
  
	// determining the END OF FILE exact address
	struct stat stats;
  stat(filename, & stats);
  int end_of_file = stats.st_size;
  
	// perfoming the lfs write ---> appending the data into the END OF FILE
  ret = pwrite(fd, data, data_size * sizeof(char), end_of_file);
  
	// writing the mapping info for the written data to know it's exact address later
  fwrite(& addr, sizeof(addr), 1, lfs);
  fwrite(& data_size, sizeof(data_size), 1, lfs);

  close(fd);
  fclose(lfs);
}


// extracts the mapping dict from our metadata(log) file
lfs_record * read_record(){
  int ret = 0;

	// find the number of items in the array by using the size of the metadata file
  struct stat stats;
  ret = stat(lfsfilename, & stats);
  assert(ret == 0);
  int record_count = stats.st_size / sizeof(lfs_record_on_disk);

  lfs_record * records = (lfs_record *)malloc(sizeof(lfs_record) * record_count);
  
  size_t file_position = 0;

	// filling the created array with the values inside the metadata file
  FILE * lfs = fopen(lfsfilename, "r");
  for(int i=0; i < record_count; i++){
    ret = fread(& records[i], sizeof(lfs_record_on_disk), 1, lfs);
    records[i].pos = file_position;
    assert(ret == 1);
    file_position += records[i].size;
  }
  fclose(lfs);

  return records;
}


// finds the common are between two given tuples 
struct tup compare_tup(struct tup first, struct tup second){
	struct tup res;
	res.a = -1;
	res.b = -1;
	//printf("tups: (%d,%d) and (%d,%d)\n", first.a, first.b, second.a, second.b); 
	if (first.a <= second.a && first.b >= second.a){
		if (first.b <= second.b){
			res.a = second.a;
			res.b = first.b;
		} 
		else{
			res.a = second.a;
			res.b = second.b;
		}
	}
	if (first.a >= second.a && first.a <= second.b){
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
int lfs_find_chunks(size_t a, size_t b, int index, lfs_record * my_recs, std::vector<lfs_record>& chunks_stack){
	//printf("check it out: %d, %d, %d\n", a, b, index);
	
	// this IF is for ending the recursion
	if(a == b)
		return 0;
	struct tup res, rec, query;
	res.a = -1;
	res.b = -1;
	query.a = a;
	query.b = b;
	rec.a = my_recs[index].addr;
	rec.b = my_recs[index].addr + my_recs[index].size;
	// search through the logs until you find a record that overlaps with the given query area
	while(res.a == -1){
		res = compare_tup(query, rec);
		index--;
		//printf("i is: %d\n", index);
		if(index < 0){
//			printf("didn't find\n");
			return 0;
		}
		rec.a = my_recs[index].addr;
		rec.b = my_recs[index].addr + my_recs[index].size;
	}
	struct lfs_record found;
	//printf("result: %d, %d\n", res.a, res.b);
	found.addr = res.a;
	found.size = res.b - res.a;
	found.pos = my_recs[index + 1].pos + res.a - my_recs[index + 1].addr;
	chunks_stack.push_back(found);
	// call yourself for the remaing areas of the query that have not been covered with the found record
	lfs_find_chunks(a, res.a,index, my_recs, chunks_stack);
	lfs_find_chunks(res.b, b,index, my_recs, chunks_stack);
	return 0;
}


// main function for read query
size_t lfs_read(size_t addr, size_t size, char * res){
	int fd = open(filename, O_RDONLY);
	struct lfs_record temp;
	
	// get the latest updated logs
	struct lfs_record * my_recs = read_record();
	
	// create the vector that acts like a stack for our finding chunks recursive function
	std::vector<struct lfs_record> chunks_stack;
	struct stat stats;

	// find the length of the log array
  stat(lfsfilename, & stats);
  int record_count = stats.st_size / sizeof(lfs_record_on_disk);
	// call the recursive function to find which areas need to be read
	lfs_find_chunks(addr, addr + size, record_count - 1, my_recs, chunks_stack);
	int total_found_count = chunks_stack.size();

	// perform the read
	for(int i = 0; i < total_found_count; i++){
		temp = chunks_stack.at(i);
		//printf("chunk stack: (%d, %d, %d)\n",temp.addr,temp.size,temp.pos);
		pread(fd, &res[(temp.addr - addr)/sizeof(char)]/*&res + temp.addr - addr*/, temp.size, temp.pos);
	}
	// Optimization: Write down the read query for futer reads!
	if (1 == 0){
		lfs_write(addr, res);
	}
	return size;
}

