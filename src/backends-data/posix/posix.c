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
#include <errno.h>

#include <esdm.h>
#include <esdm-debug.h>

#include "posix.h"


#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("POSIX", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("POSIX", fmt, __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int mkfs(esdm_backend_t* backend)
{

	posix_backend_data_t* data = (posix_backend_data_t*)backend->data;

	DEBUG("mkfs: backend->(void*)data->target = %s\n", data->target);

	const char* tgt = data->target;

	struct stat st = {0};

	if (stat(tgt, &st) == -1)
	{
		char* root;
		char* cont;
		char* sdat;
		char* sfra;

		asprintf(&root, "%s", tgt);
		asprintf(&cont, "%s/containers", tgt);
		asprintf(&sdat, "%s/shared-datasets", tgt);
		asprintf(&sfra, "%s/shared-fragments", tgt);

		mkdir(root, 0700);
		mkdir(cont, 0700);
		mkdir(sdat, 0700);
		mkdir(sfra, 0700);

		free(root);
		free(cont);
		free(sdat);
		free(sfra);
	}
	return 0;
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
// Internal Helpers ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int entry_create(const char *path)
{
	int status;
	struct stat sb;

	DEBUG("entry_create(%s)\n", path);

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


static int entry_retrieve(const char *path, void **buf, size_t **count)
{
	int status;
	struct stat sb;

	DEBUG("entry_retrieve(%s)\n", path);

	status = stat(path, &sb);
	if (status == -1) {
		perror("stat");
		// does not exist
		return -1;
	}

	//print_stat(sb);


	// write to non existing file
	int fd = open(path,	O_RDONLY | S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);


	*count = malloc(sizeof(size_t));
	**count = sb.st_size;

	// everything ok? read and close
	if ( fd != -1 )
	{
		// write some metadata
		*buf = (void*) malloc(sb.st_size);
		assert(buf != NULL);
		//char* cbuf = (char*)*buf;
		//cbuf[sb.st_size] = 0;
		size_t len = sb.st_size;
		while(len > 0){
			ssize_t ret = read(fd, buf, len);
			if (ret != -1){
				buf = (void*) ((char*) buf + ret);
				len -= ret;
			}else{
				if(errno == EINTR){
					continue;
				}else{
					ESDM_ERROR_COM_FMT("POSIX", "read %s", strerror(errno));
					**count = len;
					return 1;
				}
			}
		}
		close(fd);
	}

	//printf("Entry content: %s\n", (char *) *buf);

	/*
	uint64_t *buf64 = (uint64_t*) buf;
	for (int i = 0; i < sb.st_size/sizeof(uint64_t); i++)
	{
		printf("idx %d: %d\n", i, buf64[i]);
	}
	*/


	return 0;
}


static int entry_update(const char *path, void *buf, size_t len)
{
	DEBUG_ENTER;

	int status;
	struct stat sb;

	DEBUG("entry_update(%s: %ld)\n", path, len);

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
		while(len > 0){
			ssize_t ret = write(fd, buf, len);
			if (ret != -1){
				buf = (void*) ((char*) buf + ret);
				len -= ret;
			}else{
				if(errno == EINTR){
					continue;
				}else{
					ESDM_ERROR_COM_FMT("POSIX", "write %s", strerror(errno));
					return 1;
				}
			}
		}

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
// Fragment Handlers //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int fragment_retrieve(esdm_backend_t* backend, esdm_fragment_t *fragment, json_t * metadata)
{
	DEBUG_ENTER;

	// set data, options and tgt for convienience
	posix_backend_data_t* data = (posix_backend_data_t*)backend->data;
	const char* tgt = data->target;

	// serialization of subspace for fragment
	char *fragment_name = esdm_dataspace_string_descriptor(fragment->dataspace);

	// determine path
	char *path;
	asprintf(&path, "%s/containers/%s/%s/", tgt, fragment->dataset->container->name, fragment->dataset->name);

	// determine path to fragment
	char *path_fragment;
	asprintf(&path_fragment, "%s/containers/%s/%s/%s", tgt, fragment->dataset->container->name, fragment->dataset->name, fragment_name);

	DEBUG("path: %s\n", path);
	DEBUG("path_fragment: %s\n", path_fragment);


	//entry_update()

	size_t *count = NULL;
	void *buf = NULL;

	entry_retrieve(path_fragment, &buf, &count);


	DEBUG("count = %d,  buf=%s\n", *count, buf);

	fragment->buf = buf;




	free(path);
	free(path_fragment);
	return 0;
}


static int fragment_update(esdm_backend_t* backend, esdm_fragment_t *fragment)
{
	DEBUG_ENTER;

	// set data, options and tgt for convienience
	posix_backend_data_t* data = (posix_backend_data_t*)backend->data;
	const char* tgt = data->target;

	// serialization of subspace for fragment
	char *fragment_name = esdm_dataspace_string_descriptor(fragment->dataspace);

	// determine path
	char *path;
	asprintf(&path, "%s/containers/%s/%s/", tgt, fragment->dataset->container->name, fragment->dataset->name);

	// determine path to fragment
	char *path_fragment;
	asprintf(&path_fragment, "%s/containers/%s/%s/%s", tgt, fragment->dataset->container->name, fragment->dataset->name, fragment_name);

	DEBUG("path: %s\n", path);
	DEBUG("path_fragment: %s\n", path_fragment);

	// create metadata entry
	mkdir_recursive(path);
	entry_create(path_fragment);

	fragment->metadata->size += sprintf(& fragment->metadata->json[fragment->metadata->size], "{\"path\" : \"%s\"}", path_fragment);

	entry_update(path_fragment, fragment->buf, fragment->bytes);
	//entry_update()

	size_t *count = NULL;
	void *buf = NULL;
	entry_retrieve(path_fragment, &buf, &count);
	free(path);
	free(path_fragment);
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


/**
 * Callback implementation when beeing queried for a performance estimate.
 *
 */
static int posix_backend_performance_estimate(esdm_backend_t* backend, esdm_fragment_t *fragment, float * out_time)
{
	DEBUG_ENTER;

	posix_backend_data_t* data = (posix_backend_data_t*) backend->data;
	return esdm_backend_perf_model_long_lat_perf_estimate(& data->perf_model, fragment, out_time);
}

/**
* Finalize callback implementation called on ESDM shutdown.
*
* This is the last chance for a backend to make outstanding changes persistent.
* This routine is also expected to clean up memory that is used by the backend.
*/
int posix_finalize(esdm_backend_t* backend)
{
	DEBUG_ENTER;

	return 0;
}



///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static esdm_backend_t backend_template = {
///////////////////////////////////////////////////////////////////////////////
// NOTE: This serves as a template for the posix plugin and is memcopied!    //
///////////////////////////////////////////////////////////////////////////////
	.name = "POSIX",
	.type = ESDM_TYPE_DATA,
	.version = "0.0.1",
	.data = NULL,
	.callbacks = {
		NULL, // finalize
		posix_backend_performance_estimate, // performance_estimate

		NULL, // create
		NULL, // open
		NULL, // write
		NULL, // read
		NULL, // close

		// Metadata Callbacks
		NULL, // lookup

		// ESDM Data Model Specific
		NULL, // container create
		NULL, // container retrieve
		NULL, // container update
		NULL, // container delete

		NULL, // dataset create
		NULL, // dataset retrieve
		NULL, // dataset update
		NULL, // dataset delete

		NULL, // fragment create
		fragment_retrieve, // fragment retrieve
		fragment_update, // fragment update
		NULL, // fragment delete
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
esdm_backend_t* posix_backend_init(esdm_config_backend_t *config)
{
	DEBUG_ENTER;

	esdm_backend_t* backend = (esdm_backend_t*) malloc(sizeof(esdm_backend_t));
	memcpy(backend, &backend_template, sizeof(esdm_backend_t));

	// allocate memory for backend instance
	backend->data = (void*) malloc(sizeof(posix_backend_data_t));
	posix_backend_data_t* data = (posix_backend_data_t*) backend->data;
	esdm_backend_parse_perf_model_lat_thp(config->performance_model, & data->perf_model);

	// configure backend instance
	data->config = config;
	data->target = json_string_value(json_path_get(config->backend, "$.target"));


	DEBUG("Backend config: target=%s\n", data->target);


	// todo check posix style persitency structure available?
	mkfs(backend);

	return backend;
}
