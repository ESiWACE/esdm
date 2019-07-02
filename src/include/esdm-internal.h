/**
 * @file
 * @brief Internal ESDM functionality, not to be used by backends and plugins.
 *
 */

#ifndef ESDM_INTERNAL_H
#define ESDM_INTERNAL_H

#include <jansson.h>
#include <glib.h>

#include <esdm-datatypes-internal.h>
#include <esdm-debug.h>
#include <esdm.h>

// ESDM Core //////////////////////////////////////////////////////////////////

// Configuration

/**
 * Initializes the site configuration module.
 *
 * @param	[in] esdm   Pointer to esdm instance.
 * @return	Pointer to newly created configuration instance.
 */

esdm_config_t *esdm_config_init();

esdm_config_t *esdm_config_init_from_str(const char *str);

esdm_status esdm_config_finalize(esdm_instance_t *esdm);

void esdm_dataset_init(esdm_container_t *container, const char *name, esdm_dataspace_t *dataspace, esdm_dataset_t **out_dataset);


/**
 * Gathers ESDM configuration settings from multiple locations to build one configuration string.
 *
 */

char *esdm_config_gather();

/**
 *	Fetches backends
 *
 *
 */

esdm_config_backends_t *esdm_config_get_backends(esdm_instance_t *esdm);

esdm_config_backend_t *esdm_config_get_metadata_coordinator(esdm_instance_t *esdm);

// Datatypes

// Modules
esdm_modules_t *esdm_modules_init(esdm_instance_t *esdm);

esdm_status esdm_modules_finalize();

esdm_status esdm_modules_register();

esdm_status esdm_modules_get_by_type(esdm_module_type_t type, esdm_module_type_array_t **array);

// I/O Scheduler

/**
 * Initialize scheduler component:
 *     * setup a thread pool
 *     * allow global and local limits
 *
 *     use globale limit only if ESDM_ACCESSIBILITY_GLOBAL is set    (data_accessibility_t enum)
 *
 *
 */

esdm_scheduler_t *esdm_scheduler_init(esdm_instance_t *esdm);

esdm_status esdm_scheduler_finalize(esdm_instance_t *esdm);

esdm_status esdm_scheduler_status_init(io_request_status_t *status);

esdm_status esdm_scheduler_status_finalize(io_request_status_t *status);

/**
 * Calls to reads have to be completed before they can return to the application and are therefor blocking.
 * Use esdm_scheduler_process_blocking from functions in the application facing API to process blocking scheduling.
 *
 * Note: write is also blocking right now.
 */

esdm_status esdm_scheduler_process_blocking(esdm_instance_t *esdm, io_operation_t type, esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *subspace);

esdm_status esdm_scheduler_enqueue(esdm_instance_t *esdm, io_request_status_t *status, io_operation_t type, esdm_dataset_t *dataset, void *buf, esdm_dataspace_t *subspace);

esdm_status esdm_scheduler_wait(io_request_status_t *status);

// Layout

/**
 * Initializes the init component by allocating and populating the esdm_layout
 * structure.
 *
 * @return Status
 */

esdm_layout_t *esdm_layout_init(esdm_instance_t *esdm);

/**
* Shutdown ESDM:
*  - finalize submodules
*  - free data structures
*
* @return Status
*/

esdm_status esdm_layout_finalize(esdm_instance_t *esdm);

/**
 * The layout reconstructor finds a reconstruction for subspace of a dataset.
 *
 * The reconstruction should take performance considerations into account.
 *
 * @return Status
 */

esdm_fragment_t *esdm_layout_reconstruction(esdm_dataset_t *dataset, esdm_dataspace_t *subspace);

/**
 * Splits pending requests into one or more requests based on performance
 * estimates obtained from available backends.
 *
 * @return Status
 */

esdm_status esdm_layout_recommendation(esdm_instance_t *esdm, esdm_fragment_t *in, esdm_fragment_t *out);


/**
 * Queries backend for performance estimate for the given fragment.
 */

void fetch_performance_from_backend(gpointer key, gpointer value, gpointer user_data);

// Performance Model

esdm_performance_t *esdm_performance_init(esdm_instance_t *esdm);

/**
 * Splits pending requests into one or more requests based on performance
 * estimates obtained from available backends.
 *
 */

esdm_status esdm_performance_recommendation(esdm_instance_t *esdm, esdm_fragment_t *in, esdm_fragment_t *out);

esdm_status esdm_performance_finalize();

// Backend (generic)

esdm_status esdm_backend_t_estimate_performance(esdm_backend_t *backend, int fragment);

///////////////////////////////////////////////////////////////////////////////
// UTILS //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// auxiliary.c ////////////////////////////////////////////////////////////////

/**
 * Print a detailed summary for the stat system call.
 */

void print_stat(struct stat sb);

int mkdir_recursive(const char *path);

void posix_recursive_remove(const char *path);

int read_file(char *filepath, char **buf);

/**
 * Read while ensuring and retrying until len is read or error occured.
 */

int read_check(int fd, char *buf, size_t len);

/**
 * Write while ensuring and retrying until len is written or error occured.
 */

int write_check(int fd, char *buf, size_t len);

esdm_status esdm_metadata_t_init_(esdm_metadata_t **output_metadata);


json_t *load_json(const char *str);

esdm_status esdm_dataset_retrieve_md_load(esdm_dataset_t *dset, char ** out_md, int * out_size);
esdm_status esdm_dataset_retrieve_md_parse(esdm_dataset_t *d, char * md, int size);


int ea_compute_hash_str(const char * str);
int ea_is_valid_name(const char *str);

#endif
