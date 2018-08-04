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
#include <dirent.h>

#include <jansson.h>

#include <esdm.h>
#include <esdm-debug.h>
#include "metadummy.h"


#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("METADUMMY", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("METADUMMY", fmt, __VA_ARGS__)


// forward declarations
static void metadummy_test();



///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int mkfs(esdm_backend_t* backend)
{
	DEBUG_ENTER;

	struct stat sb;

	// use target directory from backend configuration
	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;

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
	DEBUG_ENTER;


	return 0;
}




///////////////////////////////////////////////////////////////////////////////
// Internal Helpers  //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int entry_create(const char *path, esdm_metadata_t * data)
{
	DEBUG_ENTER;

	int status;
	struct stat sb;

	DEBUG("entry_create(%s - %s)\n", path, data != NULL ? data->json : NULL);

	// ENOENT => allow to create

	status = stat(path, &sb);
	//if (status == -1) {
	//	perror("stat");

		// write to non existing file
		int fd = open(path,	O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);

		// everything ok? write and close
		if ( fd != -1 )
		{
			if(data != NULL && data->json != NULL){
				int size = strlen(data->json) + 1;
				int ret = write(fd, data->json, size);
				assert( ret == size );
			}
			close(fd);
			return 0;
		}
		return 1;
	//} else {
	//	// already exists
	//	return -1;
	//}

}

static int entry_retrieve_tst(const char *path)
{
	DEBUG_ENTER;

	int status;
	struct stat sb;
	char *buf;

	DEBUG("entry_retrieve_tst(%s)\n", path);

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


	DEBUG("Entry content: %s\n", (char*)buf);

	return 0;
}


static int entry_update(const char *path, void *buf, size_t len)
{
	DEBUG_ENTER;

	int status;
	struct stat sb;

	DEBUG("entry_update(%s)\n", path);

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


static int entry_destroy(const char *path)
{
	DEBUG_ENTER;

	int status;
	struct stat sb;

	DEBUG("entry_destroy(%s)\n", path);

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

static int container_create(esdm_backend_t* backend, esdm_container_t *container)
{
	DEBUG_ENTER;

	char *path_metadata;
	char *path_container;
	struct stat sb;

	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;

	DEBUG("tgt: %p\n", tgt);

	asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
	asprintf(&path_container, "%s/containers/%s", tgt, container->name);

	// create metadata entry
	entry_create(path_metadata, NULL);

	// create directory for datsets
	if (stat(path_container, &sb) == -1)
	{
		mkdir(path_container, 0700);
	}

	free(path_metadata);
	free(path_container);
}


static int container_retrieve(esdm_backend_t* backend, esdm_container_t *container)
{
	DEBUG_ENTER;

	char *path_metadata;
	char *path_container;
	struct stat sb;

	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;


	asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
	asprintf(&path_container, "%s/containers/%s", tgt, container->name);

	// create metadata entry
	entry_retrieve_tst(path_metadata);


	free(path_metadata);
	free(path_container);
}


static int container_update(esdm_backend_t* backend, esdm_container_t *container)
{
	DEBUG_ENTER;

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


static int container_destroy(esdm_backend_t* backend, esdm_container_t *container)
{
	DEBUG_ENTER;

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

static int dataset_create(esdm_backend_t* backend, esdm_dataset_t *dataset)
{
	DEBUG_ENTER;

	char path_metadata[PATH_MAX];
	char path_dataset[PATH_MAX];
	struct stat sb;


	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;

	DEBUG("tgt: %p\n", tgt);

	sprintf(path_metadata, "%s/containers/%s/%s.md", tgt, dataset->container->name, dataset->name);
	sprintf(path_dataset, "%s/containers/%s/%s", tgt, dataset->container->name, dataset->name);

	// create metadata entry
	entry_create(path_metadata, NULL);

	// create directory for datsets
	if (stat(path_dataset, &sb) == -1)
	{
		mkdir(path_dataset, 0700);
	}
}


static int dataset_retrieve(esdm_backend_t* backend, esdm_dataset_t *dataset)
{
	DEBUG_ENTER;
}


static int dataset_update(esdm_backend_t* backend, esdm_dataset_t *dataset)
{
	DEBUG_ENTER;
}


static int dataset_destroy(esdm_backend_t* backend, esdm_dataset_t *dataset)
{
	DEBUG_ENTER;
}



///////////////////////////////////////////////////////////////////////////////
// Fragment Helpers ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int fragment_retrieve(esdm_backend_t* backend, esdm_fragment_t *fragment, json_t * metadata){
		// set data, options and tgt for convienience
		metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
		const char* tgt = options->target;

		// serialization of subspace for fragment
		char fragment_name[PATH_MAX];
		esdm_dataspace_string_descriptor(fragment_name, fragment->dataspace);

		// determine path
		char path[PATH_MAX];
		sprintf(path, "%s/containers/%s/%s/", tgt, fragment->dataset->container->name, fragment->dataset->name);

		// determine path to fragment
		char path_fragment[PATH_MAX];
		sprintf(path_fragment, "%s/containers/%s/%s/%s", tgt, fragment->dataset->container->name, fragment->dataset->name, fragment_name);


		int status;
		struct stat sb;
		char *buf;

		status = stat(path_fragment, &sb);
		if (status == -1) {
			perror("stat");
			// does not exist
			return -1;
		}

		int fd = open(path_fragment,	O_RDONLY | S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);

		// everything ok? write and close
		if ( fd != -1 )
		{
			// write some metadata
			read(fd, fragment->metadata->json, sb.st_size);
			close(fd);
		}

		return 0;
}

int esdm_dataspace_overlap_str(esdm_dataspace_t *a, char * str){
	//printf("str: %s\n", str);

	char * save = NULL;
	const char * delim = ",";
	char * cur = strtok_r(str, delim, & save);
	if (cur == NULL) return 0;

	int64_t off[a->dimensions];
	int64_t size[a->dimensions];

	for(int d=0; d < a->dimensions; d++){
		if (cur == NULL) return 0;
		off[d] = atol(cur);
		if(off[d] < 0) return 0;
		//printf("o%ld,", off[d]);
		save[-1] = ','; // reset the overwritten text
		cur = strtok_r(NULL, delim, & save);
	}
	//printf("\n");

	for(int d=0; d < a->dimensions; d++){
		if (cur == NULL) return 0;
		size[d] = atol(cur);
		if(size[d] < 0) return 0;
		//printf("s%ld,", size[d]);
		if(save[0] != 0) save[-1] = ',';
		cur = strtok_r(NULL, delim, & save);
	}
	//printf("\n");
	if( cur != NULL){
		return 0;
	}

	// dimensions match, now check for overlap
	for(int d=0; d < a->dimensions; d++){
		int o1 = a->offset[d];
		int s1 = a->size[d];

		int o2 = off[d];
		int s2 = size[d];
		if ( o1 + s1 <= o2 ) return 0;
		if ( o2 + s2 <= o1 ) return 0;
	}
	return 1;
}

static esdm_fragment_t * create_fragment_from_metadata(int fd){
	struct stat sb;
	int ret = fstat(fd, & sb);
	DEBUG("Fragment found size:%ld", sb.st_size);

	esdm_fragment_t * frag;
	frag = malloc(sizeof(esdm_fragment_t));


	return frag;
}

/*
 * Assumptions: there are no fragments created while reading back data!
 */
static int lookup(esdm_backend_t* backend, esdm_dataset_t * dataset, esdm_dataspace_t * space, int * out_frag_count, esdm_fragment_t *** out_fragments){
	DEBUG_ENTER;

	// set data, options and tgt for convienience
	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;
	// determine path
	char path[PATH_MAX];
	sprintf(path, "%s/containers/%s/%s/", tgt, dataset->container->name, dataset->name);

	DIR * dir = opendir(path);
	if(dir == NULL){
		return ESDM_ERROR;
	}

	int frag_count = 0;
	struct dirent *e = readdir(dir);
	while(e != NULL){
		if(e->d_name[0] != '.'){
			DEBUG("checking:%s", e->d_name);
			if(esdm_dataspace_overlap_str(space, e->d_name)){
				DEBUG("Overlaps!", "");
				frag_count++;
			}
		}
		e = readdir(dir);
	}

	// read fragments!
	esdm_fragment_t ** frag = (esdm_fragment_t**) malloc(sizeof(esdm_fragment_t*) * frag_count);
	*out_fragments = frag;

	rewinddir(dir);
	int frag_no = 0;
	int dirfd = open(path, O_RDONLY);
	assert(dirfd >= 0);
	e = readdir(dir);
	while(e != NULL){
		if(e->d_name[0] != '.'){
			if(esdm_dataspace_overlap_str(space, e->d_name)){
				assert(frag_no < frag_count);
				int fd = openat(dirfd, e->d_name, O_RDONLY);
				frag[frag_no] = create_fragment_from_metadata(fd);
				close(fd);
				frag_no++;
			}
		}
		e = readdir(dir);
	}

	closedir(dir);
	close(dirfd);

	assert(frag_no == frag_count);

	*out_frag_count = frag_count;

	return ESDM_SUCCESS;
}

/*
 * How to: concurrent access by multiple processes
 */
static int fragment_update(esdm_backend_t* backend, esdm_fragment_t *fragment)
{
	DEBUG_ENTER;

	// set data, options and tgt for convienience
	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;

	// serialization of subspace for fragment
	char fragment_name[PATH_MAX];
	esdm_dataspace_string_descriptor(fragment_name, fragment->dataspace);

	// determine path
	char path[PATH_MAX];
	sprintf(path, "%s/containers/%s/%s/", tgt, fragment->dataset->container->name, fragment->dataset->name);

	// determine path to fragment
	char path_fragment[PATH_MAX];
	sprintf(path_fragment, "%s/containers/%s/%s/%s", tgt, fragment->dataset->container->name, fragment->dataset->name, fragment_name);

	DEBUG("path: %s\n", path);
	DEBUG("path_fragment: %s\n", path_fragment);

	// create metadata entry
	mkdir_recursive(path);
	entry_create(path_fragment, fragment->metadata);
}




///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int metadummy_backend_performance_estimate(esdm_backend_t* backend, esdm_fragment_t *fragment, float * out_time)
{
	DEBUG_ENTER;
	*out_time = 0;

	return 0;
}



/**
* Finalize callback implementation called on ESDM shutdown.
*
* This is the last chance for a backend to make outstanding changes persistent.
* This routine is also expected to clean up memory that is used by the backend.
*/
static int metadummy_finalize(esdm_backend_t* b)
{
	DEBUG_ENTER;

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
		metadummy_finalize, // finalize
		metadummy_backend_performance_estimate, // performance_estimate

		// Data Callbacks (POSIX like)
		NULL, // create
		NULL, // open
		NULL, // write
		NULL, // read
		NULL, // close

		// Metadata Callbacks
		lookup, // lookup

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
		fragment_retrieve, // fragment retrieve
		fragment_update, // fragment update
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
esdm_backend_t* metadummy_backend_init(esdm_config_backend_t *config)
{
	DEBUG_ENTER;

	esdm_backend_t* backend = (esdm_backend_t*) malloc(sizeof(esdm_backend_t));
	memcpy(backend, &backend_template, sizeof(esdm_backend_t));

	metadummy_backend_options_t* data = (metadummy_backend_options_t*) malloc(sizeof(metadummy_backend_options_t));

	data->target = config->target;
	backend->data = data;
	backend->config = config;

	// todo check metadummy style persitency structure available?
	mkfs(backend);

	//metadummy_test();

	return backend;

}








static void metadummy_test()
{
	int ret = -1;


	char abc[PATH_MAX];
	char def[PATH_MAX];

	const char* tgt = "./_metadummy";
	sprintf(abc, "%s/%s", tgt, "abc");
	sprintf(def, "%s/%s", tgt, "def");


	// create entry and test
	ret = entry_create(abc, NULL);
	assert(ret == 0);

	ret = entry_retrieve_tst(abc);
	assert(ret == 0);


	// double create
	ret = entry_create(def, NULL);
	assert(ret == 0);

	ret = entry_create(def, NULL);
	assert(ret == -1);


	// perform update and test
	ret = entry_update(abc, "huhuhuhuh", 5);
	ret = entry_retrieve_tst(abc);

	// delete entry and expect retrieve to fail
	ret = entry_destroy(abc);
	ret = entry_retrieve_tst(abc);
	assert(ret == -1);


	// clean up
	ret = entry_destroy(def);
	assert(ret == 0);

	ret = entry_destroy(def);
	assert(ret == -1);

}
