/* This file is part of ESDM.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESDM.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @brief A metadata backend on top of a POSIX compatible filesystem.
 */


#define _GNU_SOURCE         /* See feature_test_macros(7) */


#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <jansson.h>

#include <esdm.h>
#include <esdm-debug.h>
#include "metadummy.h"


#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("METADUMMY", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("METADUMMY", fmt, __VA_ARGS__)


///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static int mkfs(esdm_md_backend_t* backend, int enforce_format)
{
	DEBUG_ENTER;

	struct stat sb;

	// use target directory from backend configuration
	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;

	// Enforce min target length?
	const char* tgt = options->target;
	if(strlen(tgt) < 6){
		printf("[mkfs] error, the target name is to short (< 6 characters)!\n");
		return ESDM_ERROR;
	}

	char containers[PATH_MAX];
	sprintf(containers, "%s/containers", tgt);
	if (enforce_format){
		printf("[mkfs] Removing %s\n", tgt);
		if (stat(containers, & sb) != 0){
			printf("[mkfs] error, this directory seems not to be created by ESDM!\n");
		}else{
			posix_recursive_remove(tgt);
		}
		if(enforce_format == 2) return ESDM_SUCCESS;
	}

	if (stat(tgt, &sb) == 0)
	{
		return ESDM_ERROR;
	}
	printf("[mkfs] Creating %s\n", tgt);

	mkdir(tgt, 0700);
	mkdir(containers, 0700);
	return ESDM_SUCCESS;
}


static int fsck()
{
	DEBUG_ENTER;


	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Internal Helpers  //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int entry_create(const char *path, esdm_metadata *data)
{
	DEBUG_ENTER;

	// int status;
	// struct stat sb;

	DEBUG("entry_create(%s - %s)\n", path, data != NULL ? data->json : NULL);

	// ENOENT => allow to create
	// write to non existing file
	int fd = open(path,	O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
	// everything ok? write and close
	if ( fd != -1 )
	{
		if(data != NULL && data->json != NULL){
			int ret = write_check(fd, data->json, data->size);
			assert( ret == 0 );
		}
		close(fd);
		return 0;
	}

	return 1;
}


static int entry_retrieve_tst(const char *path, esdm_dataset_t *dataset)
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

		read_check(fd, buf, sb.st_size);
		close(fd);
	}

	DEBUG("Entry content: %s\n", (char*)buf);

	// Save the metadata in the dataset structure

	dataset->metadata = (esdm_metadata *) malloc(sizeof(esdm_metadata));
	dataset->metadata->json = (char *) malloc(456*sizeof(char)); // randon number
	strcpy(dataset->metadata->json, buf);

	printf("\njson: %s %s\n", dataset->metadata->json, buf);

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
		write_check(fd, buf, len);
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


static int container_create(esdm_md_backend_t* backend, esdm_container *container)
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


	return 0;
}


static int container_retrieve(esdm_md_backend_t* backend, esdm_container *container)
{
	DEBUG_ENTER;

	char *path_metadata;
	char *path_container;

	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;


	asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
	asprintf(&path_container, "%s/containers/%s", tgt, container->name);

	// create metadata entry
	esdm_dataset_t *dataset = NULL;
	entry_retrieve_tst(path_metadata, dataset); // conflict


	free(path_metadata);
	free(path_container);


	return 0;
}


static int container_update(esdm_md_backend_t* backend, esdm_container *container)
{
	DEBUG_ENTER;

	char *path_metadata;
	char *path_container;

	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;

	asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
	asprintf(&path_container, "%s/containers/%s", tgt, container->name);

	// create metadata entry
	entry_update(path_metadata, "abc", 3);


	free(path_metadata);
	free(path_container);

	return 0;
}


static int container_destroy(esdm_md_backend_t* backend, esdm_container *container)
{
	DEBUG_ENTER;

	char *path_metadata;
	char *path_container;

	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;

	asprintf(&path_metadata, "%s/containers/%s.md", tgt, container->name);
	asprintf(&path_container, "%s/containers/%s", tgt, container->name);

	// create metadata entry
	entry_destroy(path_metadata);


	// TODO: also remove existing datasets?


	free(path_metadata);
	free(path_container);

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Dataset Helpers ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static int dataset_create(esdm_md_backend_t* backend, esdm_dataset_t *dataset)
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
	entry_create(path_metadata, dataset->metadata);
//		entry_create(path_metadata, &x);

	// create directory for datsets
	if (stat(path_dataset, &sb) == -1)
	{
		mkdir(path_dataset, 0700);
	}

	return 0;
}


static int dataset_retrieve(esdm_md_backend_t* backend, esdm_dataset_t *dataset)
{
	DEBUG_ENTER;

	char path_metadata[PATH_MAX];

	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;

	sprintf(path_metadata, "%s/containers/%s/%s.md", tgt, dataset->container->name, dataset->name);
	struct stat statbuf;
  int ret = stat(path_metadata, & statbuf);
	if (ret != 0) return ESDM_ERROR;
	off_t len = statbuf.st_size;

	int fd = open(path_metadata,	O_RDONLY | S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
	if( fd < 0 ) return ESDM_ERROR;
	dataset->metadata->json = (char *) malloc(len);
	dataset->metadata->size = len;
	dataset->metadata->buff_size = len;

	char *bbuf = dataset->metadata->json;
	while(len > 0){
		ssize_t ret = read(fd, bbuf, len);
		if (ret != -1){
			bbuf += ret;
			len -= ret;
		}else{
			if(errno == EINTR){
				continue;
			}else{
				ESDM_ERROR_COM_FMT("POSIX MD", "read %s", strerror(errno));
				return 1;
			}
		}
	}

	close(fd);

	return 0;
}


static int dataset_update(esdm_md_backend_t* backend, esdm_dataset_t *dataset)
{
	DEBUG_ENTER;

	return 0;
}


static int dataset_destroy(esdm_md_backend_t* backend, esdm_dataset_t *dataset)
{
	DEBUG_ENTER;

	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Fragment Helpers ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static int fragment_retrieve(esdm_md_backend_t* backend, esdm_fragment_t *fragment, json_t * metadata)
{
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
			read_check(fd, fragment->metadata->json, sb.st_size);
			close(fd);
		}

		return 0;
}


static esdm_fragment_t * create_fragment_from_metadata(int fd, esdm_dataset_t * dataset, esdm_dataspace_t * space)
{
	struct stat sb;

	fstat(fd, & sb);
	DEBUG("Fragment found size:%ld", sb.st_size);

	esdm_fragment_t * f;
	f = malloc(sizeof(esdm_fragment_t));
	f->metadata = malloc(sb.st_size + sizeof(esdm_metadata));
	f->metadata->json = (char*)(f->metadata) + sizeof(esdm_metadata);
	f->metadata->size = sb.st_size;
	read_check(fd, f->metadata->json, sb.st_size);

	uint64_t elements = esdm_dataspace_element_count(space);
	int64_t bytes = elements * esdm_sizeof(space->datatype);

	f->dataset = dataset;
	f->dataspace = space;
	f->buf = NULL;
	f->elements = elements;
	f->bytes = bytes;
	f->status = ESDM_STATUS_PERSISTENT;
	f->in_place = 0;
	//printf("%s \n", f->metadata->json);

	return f;
}

/*
 * Assumptions: there are no fragments created while reading back data!
 */
static int lookup(esdm_md_backend_t* backend, esdm_dataset_t * dataset, esdm_dataspace_t * space, int * out_frag_count, esdm_fragment_t *** out_fragments)
{
	DEBUG_ENTER;

	// set data, options and tgt for convienience
	metadummy_backend_options_t *options = (metadummy_backend_options_t*) backend->data;
	const char* tgt = options->target;
	// determine path
	char path[PATH_MAX];
	sprintf(path, "%s/containers/%s/%s/", tgt, dataset->container->name, dataset->name);

	// optimization: check if we find a fragment that matches the requested domain exactly
	{
		char fragment_name[PATH_MAX];
		esdm_dataspace_string_descriptor(fragment_name, space);
		char path_full[PATH_MAX];
		sprintf(path_full, "%s/%s", path, fragment_name);
		int fd = open(path_full, O_RDONLY);
		if(fd >= 0){
			// found a fragment
			*out_frag_count = 1;
			esdm_fragment_t ** frag = (esdm_fragment_t**) malloc(sizeof(esdm_fragment_t*));
			*out_fragments = frag;
			frag[0] = create_fragment_from_metadata(fd, dataset, space);
			frag[0]->in_place = 1;
			close(fd);
			return ESDM_SUCCESS;
		}
	}

	DIR * dir = opendir(path);
	if(dir == NULL){
		return ESDM_ERROR;
	}

	int frag_count = 0;
	struct dirent *e = readdir(dir);
	while(e != NULL){
		if(e->d_name[0] != '.'){
			DEBUG("checking:%s", e->d_name);
			if(esdm_dataspace_overlap_str(space, ',', e->d_name, NULL, NULL) == ESDM_SUCCESS){
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
			esdm_dataspace_t * subspace;
			if(esdm_dataspace_overlap_str(space, ',', e->d_name, NULL, & subspace) == ESDM_SUCCESS){
				assert(frag_no < frag_count);
				int fd = openat(dirfd, e->d_name, O_RDONLY);
				frag[frag_no] = create_fragment_from_metadata(fd, dataset, subspace);
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
static int fragment_update(esdm_md_backend_t* backend, esdm_fragment_t *fragment)
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


	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static int metadummy_backend_performance_estimate(esdm_md_backend_t* backend, esdm_fragment_t *fragment, float * out_time)
{
	DEBUG_ENTER;
	*out_time = 0;

	return 0;
}


static int metadummy_finalize(esdm_md_backend_t* b)
{
	DEBUG_ENTER;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static esdm_md_backend_t backend_template = {
	.name = "metadummy",
	.version = "0.0.1",
	.data = NULL,
	.callbacks = {
		// General for ESDM
		metadummy_finalize, // finalize
		metadummy_backend_performance_estimate, // performance_estimate

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
		NULL, // fragment destroy,
		mkfs,
	},
};


esdm_md_backend_t* metadummy_backend_init(esdm_config_backend_t *config)
{
	DEBUG_ENTER;

	esdm_md_backend_t* backend = (esdm_md_backend_t*) malloc(sizeof(esdm_md_backend_t));
	memcpy(backend, &backend_template, sizeof(esdm_md_backend_t));

	metadummy_backend_options_t* data = (metadummy_backend_options_t*) malloc(sizeof(metadummy_backend_options_t));

	data->target = config->target;
	backend->data = data;
	backend->config = config;
	//metadummy_test();


	mkfs(backend, 0);

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

	esdm_dataset_t *dataset = NULL;
	ret = entry_retrieve_tst(abc, dataset);
	assert(ret == 0);

	// double create
	ret = entry_create(def, NULL);
	assert(ret == 0);

	ret = entry_create(def, NULL);
	assert(ret == -1);

	// perform update and test
	ret = entry_update(abc, "huhuhuhuh", 5);

	ret = entry_retrieve_tst(abc, dataset);
	// delete entry and expect retrieve to fail
	ret = entry_destroy(abc);
	ret = entry_retrieve_tst(abc, dataset);
	assert(ret == -1);

	// clean up
	ret = entry_destroy(def);
	assert(ret == 0);

	ret = entry_destroy(def);
	assert(ret == -1);

}
