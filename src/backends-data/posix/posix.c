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
#include <errno.h>

#include <esdm.h>
#include <esdm-debug.h>

#include "posix.h"


#define DEBUG_ENTER ESDM_DEBUG_COM_FMT("POSIX", "", "")
#define DEBUG(fmt, ...) ESDM_DEBUG_COM_FMT("POSIX", fmt, __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
// Helper and utility /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static int mkfs(esdm_backend* backend, int enforce_format)
{
	posix_backend_data_t* data = (posix_backend_data_t*)backend->data;

	DEBUG("mkfs: backend->(void*)data->target = %s\n", data->target);

	const char* tgt = data->target;
	if(strlen(tgt) < 6){
		return ESDM_ERROR;
	}
	char sdat[PATH_MAX];
	sprintf(sdat, "%s/shared-datasets", tgt);

	struct stat st = {0};
	if (enforce_format){
		printf("[mkfs] Removing %s\n", tgt);
		if (stat(sdat, & st) != 0){
			printf("[mkfs] error, this directory seems not to be created by ESDM!\n");
		}else{
			posix_recursive_remove(tgt);
		}
		if(enforce_format == 2) return ESDM_SUCCESS;
	}

	if (stat(tgt, &st) != -1)
	{
		return ESDM_ERROR;
	}
	printf("[mkfs] Creating %s\n", tgt);
	char root[PATH_MAX];
	char cont[PATH_MAX];
	char sfra[PATH_MAX];

	sprintf(root, "%s", tgt);
	sprintf(cont, "%s/containers", tgt);
	sprintf(sfra, "%s/shared-fragments", tgt);

	mkdir(root, 0700);
	mkdir(cont, 0700);
	mkdir(sdat, 0700);
	mkdir(sfra, 0700);
	return ESDM_SUCCESS;
}

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
		// write to non existing file
		int fd = open(path,	O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
		// everything ok? write and close
		if ( fd != -1 )
		{
			close(fd);
			return 0;
		}
	}
	// already exists
	return -1;
}


static int entry_retrieve(const char *path, void *buf)
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

	// write to non existing file
	int fd = open(path,	O_RDONLY | S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
	// everything ok? read and close
	if ( fd != -1 )
	{
		size_t len = sb.st_size;
		char *bbuf = (char*) buf;
		while(len > 0){
			ssize_t ret = read(fd, bbuf, len);
			if (ret != -1){
				bbuf += ret;
				len -= ret;
			}else{
				if(errno == EINTR){
					continue;
				}else{
					ESDM_ERROR_COM_FMT("POSIX", "read %s", strerror(errno));
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


static int fragment_retrieve(esdm_backend* backend, esdm_fragment_t *fragment, json_t * metadata)
{
	DEBUG_ENTER;

	// set data, options and tgt for convienience
	posix_backend_data_t* data = (posix_backend_data_t*)backend->data;
	const char* tgt = data->target;

	// serialization of subspace for fragment
	char fragment_name[PATH_MAX];
	esdm_dataspace_string_descriptor(fragment_name, fragment->dataspace);

	// determine path
	char path[PATH_MAX];
	sprintf(path, "%s/containers/%s/%s/", tgt, fragment->dataset->container->name, fragment->dataset->name);

	// determine path to fragment
	char path_fragment[PATH_MAX];
	sprintf(path_fragment, "%s/containers/%s/%s/%s", tgt, fragment->dataset->container->name, fragment->dataset->name, fragment_name);

	DEBUG("path: %s", path);
	DEBUG("path_fragment: %s", path_fragment);

	//entry_update()

	entry_retrieve(path_fragment, fragment->buf);
	//DEBUG("buf=%s", fragment->buf);
	return 0;
}


static int fragment_update(esdm_backend* backend, esdm_fragment_t *fragment)
{
	DEBUG_ENTER;

	// set data, options and tgt for convienience
	posix_backend_data_t* data = (posix_backend_data_t*)backend->data;
	const char* tgt = data->target;

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
	entry_create(path_fragment);

	fragment->metadata->size += sprintf(& fragment->metadata->json[fragment->metadata->size], "{\"path\" : \"%s\"}", path_fragment);

	entry_update(path_fragment, fragment->buf, fragment->bytes);
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
// ESDM Callbacks /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


static int posix_backend_performance_estimate(esdm_backend* backend, esdm_fragment_t *fragment, float * out_time)
{
	DEBUG_ENTER;

	if (!backend || !fragment || !out_time)
		return 1;

	posix_backend_data_t* data = (posix_backend_data_t*) backend->data;
	return esdm_backend_perf_model_long_lat_perf_estimate(& data->perf_model, fragment, out_time);
}


int posix_finalize(esdm_backend* backend)
{
	DEBUG_ENTER;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// ESDM Module Registration ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static esdm_backend backend_template = {
///////////////////////////////////////////////////////////////////////////////
// NOTE: This serves as a template for the posix plugin and is memcopied!    //
///////////////////////////////////////////////////////////////////////////////
	.name = "POSIX",
	.type = SMD_DTYPE_DATA,
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
		mkfs,
	},
};

// Two versions of this function!!!
//
// datadummy.c

esdm_backend* posix_backend_init(esdm_config_backend_t *config)
{
	DEBUG_ENTER;

	if (!config || !config->type || strcasecmp(config->type, "POSIX") || !config->target) {
		DEBUG("Wrong configuration%s\n", "");
		return NULL;
	}

	esdm_backend* backend = (esdm_backend*) malloc(sizeof(esdm_backend));
	memcpy(backend, &backend_template, sizeof(esdm_backend));

	// allocate memory for backend instance
	backend->data = (void*) malloc(sizeof(posix_backend_data_t));
	posix_backend_data_t* data = (posix_backend_data_t*) backend->data;

	if (data && config->performance_model)
		esdm_backend_parse_perf_model_lat_thp(config->performance_model, & data->perf_model);
	else
		esdm_backend_reset_perf_model_lat_thp(& data->perf_model);

	// configure backend instance
	data->config = config;
	data->target = json_string_value(json_path_get(config->backend, "$.target"));


	DEBUG("Backend config: target=%s\n", data->target);

	return backend;
}
