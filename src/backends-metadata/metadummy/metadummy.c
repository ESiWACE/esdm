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

#include <esdm.h>
#include "metadummy.h"


void log(const char* format, ...)
{
	va_list args;
	va_start(args,format);
	vprintf(format,args);
	va_end(args);
}
#define DEBUG(msg) log("[METADUMMY] %-30s %s:%d\n", msg, __FILE__, __LINE__)


// forward declarations
void metadummy_test();



///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int mkfs(esdm_backend_t* backend) 
{
	DEBUG("metadummy setup");

	struct stat sb;

	// use target directory from backend configuration
	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;
	//const char* tgt = "./_metadummy";


	if (stat(tgt, &sb) == -1)
	{
		char* root; 
		char* containers; 
		
		asprintf(&root, "%s", tgt);
		mkdir(root, 0700);
	
		asprintf(&containers, "%s/containers", tgt);
		mkdir(containers, 0700);

		free(root);
		free(containers);
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




///////////////////////////////////////////////////////////////////////////////
// Internal Helpers  //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int entry_create(const char *path)
{
	int status;
	struct stat sb;
	
	printf("entry_create(%s)\n", path);

	// ENOENT => allow to create

	status = stat(path, &sb);
	if (status == -1) {
		perror("stat");

		// write to non existing file
		int fd = open(path,	O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);

		// everything ok? write and close
		if ( fd != -1 )
		{
			close(fd);
		}

		return 0;

	} else {
		// already exists
		return -1;
	}

}


int entry_retrieve(const char *path)
{
	int status;
	struct stat sb;
	char* buf;

	printf("entry_retrieve(%s)\n", path);

	status = stat(path, &sb);
	if (status == -1) {
		perror("stat");
		// does not exist
		return -1;
	}

	//print_stat(sb);


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

	return 0;
}


int entry_update(const char *path, char *buf, size_t len)
{
	int status;
	struct stat sb;

	printf("entry_update(%s)\n", path);

	status = stat(path, &sb);
	if (status == -1) {
		perror("stat");
		return -1;
	}

	//print_stat(sb);

	// write to non existing file
	int fd = open(path,	O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);

	// everything ok? write and close
	if ( fd != -1 )
	{
		// write some metadata
		write(fd, buf, len);
		close(fd);
	}

	return 0;
}


int entry_destroy(const char *path) 
{
	int status;
	struct stat sb;

	printf("entry_destroy(%s)\n", path);

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

	return 0;
}











///////////////////////////////////////////////////////////////////////////////
// Container Helpers //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int container_create(esdm_backend_t* backend, esdm_container_t *container)
{
	char *path_metadata;
	char *path_container;
	struct stat sb;

	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;

	asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
	asprintf(&path_container, "%s/containers/%s", tgt, container->name);

	// create metadata entry
	entry_create(path_metadata);

	// create directory for datsets
	if (stat(path_container, &sb) == -1)
	{
		mkdir(path_container, 0700);
	}

	free(path_metadata);
	free(path_container);
}


int container_retrieve(esdm_backend_t* backend, esdm_container_t *container)
{
	char *path_metadata;
	char *path_container;
	struct stat sb;

	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;


	asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
	asprintf(&path_container, "%s/containers/%s", tgt, container->name);

	// create metadata entry
	entry_retrieve(path_metadata);


	free(path_metadata);
	free(path_container);
}


int container_update(esdm_backend_t* backend, esdm_container_t *container)
{
	char *path_metadata;
	char *path_container;
	struct stat sb;

	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;

	asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
	asprintf(&path_container, "%s/containers/%s", tgt, container->name);

	// create metadata entry
	entry_update(path_metadata, "abc", 3);


	free(path_metadata);
	free(path_container);
}


int container_destroy(esdm_backend_t* backend, esdm_container_t *container) 
{
	char *path_metadata;
	char *path_container;
	struct stat sb;

	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;

	asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
	asprintf(&path_container, "%s/containers/%s", tgt, container->name);

	// create metadata entry
	entry_destroy(path_metadata);


	// TODO: also remove existing datasets?


	free(path_metadata);
	free(path_container);
}



///////////////////////////////////////////////////////////////////////////////
// Dataset Helpers ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int dataset_create(esdm_backend_t* backend, esdm_dataset_t *dataset)
{
	char *path_metadata;
	char *path_dataset;
	struct stat sb;


	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;

	asprintf(&path_metadata, "%s/containers/%s/%s.md", tgt, dataset->container->name, dataset->name);
	asprintf(&path_dataset, "%s/containers/%s/%s", tgt, dataset->container->name, dataset->name);

	// create metadata entry
	entry_create(path_metadata);

	// create directory for datsets
	if (stat(path_dataset, &sb) == -1)
	{
		mkdir(path_dataset, 0700);
	}

	free(path_metadata);
	free(path_dataset);
}


int dataset_retrieve(esdm_backend_t* backend, esdm_dataset_t *dataset)
{
}


int dataset_update(esdm_backend_t* backend, esdm_dataset_t *dataset)
{
}


int dataset_destroy(esdm_backend_t* backend, esdm_dataset_t *dataset) 
{
}





///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int metadummy_backend_performance_estimate(esdm_backend_t* backend) 
{
	DEBUG("Calculating performance estimate.");

	return 0;
}


int metadummy_create(esdm_backend_t* backend, char* name) 
{
	DEBUG("Create");

	// TODO; Sanitize name, and reject forbidden names


	//container_create(backend, name);



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
int metadummy_open(esdm_backend_t* backend) 
{
	DEBUG("Open");
	return 0;
}

int metadummy_write(esdm_backend_t* backend) 
{
	DEBUG("Write");
	return 0;
}

int metadummy_read(esdm_backend_t* backend) 
{
	DEBUG("Read");
	return 0;
}

int metadummy_close(esdm_backend_t* backend) 
{
	DEBUG("Close");
	return 0;
}



int metadummy_allocate(esdm_backend_t* backend) 
{
	DEBUG("Allocate");
	return 0;
}


int metadummy_update(esdm_backend_t* backend) 
{
	DEBUG("Update");
	return 0;
}


int metadummy_lookup(esdm_backend_t* backend) 
{
	DEBUG("Lookup");
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
		// General for ESDM
		NULL, // finalize
		metadummy_backend_performance_estimate, // performance_estimate

		// Data Callbacks (POSIX like)
		metadummy_create, // create
		metadummy_open, // open
		metadummy_write, // write
		metadummy_read, // read
		metadummy_close, // close

		// Metadata Callbacks
		NULL, // lookup

		// ESDM Data Model Specific
		container_create, // container create
		container_retrieve, // container retrieve
		container_update, // container update
		container_destroy, // container destroy

		dataset_create, // dataset create
		dataset_retrieve, // dataset retrieve
		dataset_update, // dataset update
		dataset_destroy, // dataset destroy

		NULL, // fragment create
		NULL, // fragment retrieve
		NULL, // fragment update
		NULL, // fragment destroy
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
	
	DEBUG("Initializing metadummy backend.");

	esdm_backend_t* backend = (esdm_backend_t*) malloc(sizeof(esdm_backend_t));
	memcpy(backend, &backend_template, sizeof(esdm_backend_t));

	metadummy_backend_options_t* data = (metadummy_backend_options_t*) malloc(sizeof(metadummy_backend_options_t));

	char *tgt;
	asprintf(&tgt, "./_metadummy");
	data->target = tgt;

	//backend->data = init_data;
	backend->data = data;


	// todo check metadummy style persitency structure available?
	mkfs(backend);	

	//metadummy_test();

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








void metadummy_test() 
{
	int ret = -1;


	char* abc;
	char* def;

	const char* tgt = "./_metadummy";
	asprintf(&abc, "%s/%s", tgt, "abc");
	asprintf(&def, "%s/%s", tgt, "def");


	// create entry and test
	ret = entry_create(abc);
	assert(ret == 0);

	ret = entry_retrieve(abc);
	assert(ret == 0);


	// double create
	ret = entry_create(def);
	assert(ret == 0);

	ret = entry_create(def);
	assert(ret == -1);


	// perform update and test
	ret = entry_update(abc, "huhuhuhuh", 5);
	ret = entry_retrieve(abc);

	// delete entry and expect retrieve to fail
	ret = entry_destroy(abc);
	ret = entry_retrieve(abc);
	assert(ret == -1);


	// clean up
	ret = entry_destroy(def);
	assert(ret == 0);

	ret = entry_destroy(def);
	assert(ret == -1);
	
}
