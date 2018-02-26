/* This file is part of ESDM.                                              
 *                                                                              
 * This program is is free software: you can redistribute it and/or modify         
 * it under the terms of the GNU Lesser General Public License as published by  
 * the Free Software Foundation, either version 3 of the License, or            
 * (at your option) any later version.                                          
 *                                                                              
 * This program is is distributed in the hope that it will be useful,           
 * but WITHOUT ANY WARRANTY; without even the implied warranty of               
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                
 * GNU General Public License for more details.                                 
 *                                                                                 
 * You should have received a copy of the GNU Lesser General Public License        
 * along with ESDM.  If not, see <http://www.gnu.org/licenses/>.           
 */                                                                         

/**
 * @file
 * @brief A data backend to provide POSIX compatibility.
 */


#define _GNU_SOURCE         /* See feature_test_macros(7) */


#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include <jansson.h>

#include "metadummy.h"


void log(uint32_t loglevel, const char* format, ...)
{
	uint32_t active_loglevel = 99;

	if ( loglevel <= active_loglevel ) {
		va_list args;
		va_start(args,format);
		vprintf(format,args);
		va_end(args);
	}
}
#define DEBUG(loglevel, msg) log(loglevel, "[METADUMMY] %-30s %s:%d\n", msg, __FILE__, __LINE__)


///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int mkfs(esdm_backend_t* backend) 
{
	DEBUG(0, "metadummy setup");

	//const char* tgt = options->target;
	const char* tgt = "./_metadummy";

	struct stat st = {0};

	if (stat(tgt, &st) == -1)
	{
		char* root; 
		asprintf(&root, "%s", tgt);
		mkdir(root, 0700);
		free(root);
	}
}


/**
 * Similar to the command line counterpart fsck for ESDM plugins is responsible
 * to check and potentially repair the "filesystem".
 *
 */
static int fsck()
{


	return 0;
}





int print_stat(struct stat sb)
{
	printf("File type:                ");
	switch (sb.st_mode & S_IFMT) {
		case S_IFBLK:  printf("block device\n");      break;
		case S_IFCHR:  printf("character device\n");  break;
		case S_IFDIR:  printf("directory\n");         break;
		case S_IFIFO:  printf("FIFO/pipe\n");         break;
		case S_IFLNK:  printf("symlink\n");           break;
		case S_IFREG:  printf("regular file\n");      break;
		case S_IFSOCK: printf("socket\n");            break;
		default:       printf("unknown?\n");          break;
	}
	printf("I-node number:            %ld\n", (long) sb.st_ino);
	printf("Mode:                     %lo (octal)\n", (unsigned long) sb.st_mode);
	printf("Link count:               %ld\n", (long) sb.st_nlink);
	printf("Ownership:                UID=%ld   GID=%ld\n", (long) sb.st_uid, (long) sb.st_gid);
	printf("Preferred I/O block size: %ld bytes\n", (long) sb.st_blksize);
	printf("File size:                %lld bytes\n", (long long) sb.st_size);
	printf("Blocks allocated:         %lld\n", (long long) sb.st_blocks);
	printf("Last status change:       %s", ctime(&sb.st_ctime));
	printf("Last file access:         %s", ctime(&sb.st_atime));
	printf("Last file modification:   %s", ctime(&sb.st_mtime));
}




///////////////////////////////////////////////////////////////////////////////
// Internal Handlers //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// find_fragment


int entry_create(const char *name)
{
	int status;
	struct stat sb;
	const char* tgt = "./_metadummy";
	char *path;

	asprintf(&path, "%s/%s", tgt, name);
	printf("\nentry_create(%s)\n", path);

	// ENOENT => allow to create

	status = stat(path, &sb);
	if (status == -1) {
		perror("stat");

		print_stat(sb);

		// write to non existing file
		int fd = open(path,	O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);

		// everything ok? write and close
		if ( fd != -1 )
		{
			close(fd);
		}

		free(path);
		return 0;

	} else {
		// already exists
		return -1;
	}

}


int entry_receive(const char *name)
{
	int status;
	struct stat sb;
	const char* tgt = "./_metadummy";
	char *path;
	char *buf;

	asprintf(&path, "%s/%s", tgt, name);
	printf("\nentry_receive(%s)\n", path);

	status = stat(path, &sb);
	if (status == -1) {
		perror("stat");

		// does not exist
		return -1;
	}

	print_stat(sb);


	// write to non existing file
	int fd = open(path,	O_RDONLY | S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);

	// everything ok? write and close
	if ( fd != -1 )
	{
		// write some metadata
		buf = (char*) malloc(sb.st_size + 1);
		buf[sb.st_size] = 0;

		read(fd, buf, sb.st_size);
		close(fd);
	}


	printf("Entry content: %s\n", buf);



	
	free(path);

	return 0;
}


int entry_update(const char *name, char *buf, size_t len)
{
	int status;
	struct stat sb;
	const char* tgt = "./_metadummy";
	char *path;

	asprintf(&path, "%s/%s", tgt, name);
	printf("\nentry_update(%s)\n", path);

	status = stat(path, &sb);
	if (status == -1) {
		perror("stat");
		return -1;
	}

	print_stat(sb);

	// write to non existing file
	int fd = open(path,	O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);

	// everything ok? write and close
	if ( fd != -1 )
	{
		// write some metadata
		write(fd, buf, len);
		close(fd);
	}

	free(path);

	return 0;
}


int entry_destroy(const char *name) 
{
	int status;
	struct stat sb;
	const char* tgt = "./_metadummy";
	char *path;

	asprintf(&path, "%s/%s", tgt, name);
	printf("\nentry_destroy(%s)\n", path);

	status = stat(path, &sb);
	if (status == -1) {
		perror("stat");
		return -1;
	}

	print_stat(sb);

	status = unlink(path);
	if (status == -1) {
		perror("unlink");
		return -1;
	}

	free(path);

	return 0;
}



///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int metadummy_backend_performance_estimate(esdm_backend_t* backend) 
{
	DEBUG(0, "Calculating performance estimate.");


	return 0;
}


int metadummy_create() 
{
	DEBUG(0, "Create");


	// check if container already exists

	struct stat st = {0};
	if (stat("_esdm-fs", &st) == -1)
	{
		mkdir("_esdm-fs/containers", 0700);
	}


    //#include <unistd.h>
    //ssize_t pread(int fd, void *buf, size_t count, off_t offset);
    //ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);

	return 0;
}


/**
 *	
 *	handle
 *	mode
 *	owner?	
 *
 */
int metadummy_open() 
{
	DEBUG(0, "Open");
return 0;
}

int metadummy_write() 
{
	DEBUG(0, "Write");
	return 0;
}

int metadummy_read() 
{
	DEBUG(0, "Read");
	return 0;
}

int metadummy_close() 
{
	DEBUG(0, "Close");
	return 0;
}



int metadummy_allocate() 
{
	DEBUG(0, "Allocate");
	return 0;
}


int metadummy_update() 
{
	DEBUG(0, "Update");
	return 0;
}


int metadummy_lookup() 
{
	DEBUG(0, "Lookup");
	return 0;
}



///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static esdm_backend_t backend_template = {
///////////////////////////////////////////////////////////////////////////////
// WARNING: This serves as a template for the metadummy plugin and is memcpied!  //
///////////////////////////////////////////////////////////////////////////////
	.name = "metadummy",
	.type = ESDM_TYPE_METADATA,
	.version = "0.0.1",
	.data = NULL,
	.callbacks = {
		NULL, // finalize
		metadummy_backend_performance_estimate, // performance_estimate

		metadummy_create, // create
		metadummy_open, // open
		metadummy_write, // write
		metadummy_read, // read
		metadummy_close, // close

		NULL, // allocate
		NULL, // update
		NULL, // lookup
	},
};

/**
* Initializes the POSIX plugin. In particular this involves:
*
*	* Load configuration of this backend
*	* Load and potentially calibrate performance model
*
*	* Connect with support services e.g. for technical metadata
*	* Setup directory structures used by this POSIX specific backend
*
*	* Populate esdm_backend_t struct and callbacks required for registration
*
* @return pointer to backend struct
*/
esdm_backend_t* metadummy_backend_init(void* init_data) {
	
	DEBUG(0, "Initializing metadummy backend.");

	esdm_backend_t* backend = (esdm_backend_t*) malloc(sizeof(esdm_backend_t));
	memcpy(backend, &backend_template, sizeof(esdm_backend_t));

	backend->data = init_data;


	// todo check metadummy style persitency structure available?
	mkfs(backend);	


	int ret = -1;



	// create entry and test
	ret = entry_create("abc");
	assert(ret == 0);

	ret = entry_receive("abc");
	assert(ret == 0);


	// double create
	ret = entry_create("def");
	assert(ret == 0);

	ret = entry_create("def");
	assert(ret == -1);


	// perform update and test
	ret = entry_update("abc", "huhuhuhuh", 5);
	ret = entry_receive("abc");

	// delete entry and expect receive to fail
	ret = entry_destroy("abc");
	ret = entry_receive("abc");
	assert(ret == -1);

	// clean up
	ret = entry_destroy("def");
	assert(ret == 0);

	ret = entry_destroy("def");
	assert(ret == -1);
	


	return backend;

}

/**
* Initializes the POSIX plugin. In particular this involves:
*
*/
int metadummy_finalize()
{

	return 0;
}


