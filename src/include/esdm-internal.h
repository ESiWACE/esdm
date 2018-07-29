/**
 * @file
 * @brief Internal ESDM functionality, not to be used by backends and plugins.
 *
 */
#ifndef ESDM_INTERNAL_H
#define ESDM_INTERNAL_H

#include <esdm-debug.h>
#include <esdm.h>

/* see esdm-datatypes.h for definition
typedef enum {
	ESDM_LOGLEVEL_CRITICAL,
	ESDM_LOGLEVEL_ERROR,
	ESDM_LOGLEVEL_WARNING,
	ESDM_LOGLEVEL_INFO,
	ESDM_LOGLEVEL_DEBUG,
	ESDM_LOGLEVEL_NOTSET
} esdm_loglevel_t;
*/




// Metadata
// TODO: really necessary?


// ESDM Core //////////////////////////////////////////////////////////////////

// Configuration
esdm_config_t* esdm_config_init();
esdm_config_t* esdm_config_init_from_str(const char * str);
esdm_status_t esdm_config_finalize(esdm_instance_t *esdm);

esdm_config_backends_t* esdm_config_get_backends(esdm_instance_t *esdm);
esdm_config_backend_t* esdm_config_get_metadata_coordinator(esdm_instance_t *esdm);

// Datatypes


// Modules
esdm_modules_t* esdm_modules_init(esdm_instance_t *esdm);
esdm_status_t esdm_modules_finalize();
esdm_status_t esdm_modules_register();

esdm_status_t esdm_modules_get_by_type(esdm_module_type_t type, esdm_module_type_array_t *array);

// I/O Scheduler
esdm_scheduler_t* esdm_scheduler_init(esdm_instance_t *esdm);
esdm_status_t esdm_scheduler_finalize(esdm_instance_t *esdm);

esdm_status_t esdm_scheduler_status_init(io_request_status_t * status);
esdm_status_t esdm_scheduler_status_finalize(io_request_status_t * status);
esdm_status_t esdm_scheduler_process_blocking(esdm_instance_t *esdm, io_operation_t type, esdm_dataset_t *dataset,  void *buf, esdm_dataspace_t* subspace);
esdm_status_t esdm_scheduler_enqueue(esdm_instance_t *esdm, io_request_status_t * status, io_operation_t type, esdm_dataset_t *dataset,  void *buf, esdm_dataspace_t* subspace);
esdm_status_t esdm_scheduler_wait(io_request_status_t * status);



// Layout
esdm_layout_t* esdm_layout_init(esdm_instance_t *esdm);
esdm_status_t esdm_layout_finalize();
esdm_fragment_t* esdm_layout_reconstruction(esdm_dataset_t *dataset, esdm_dataspace_t *subspace);
esdm_status_t esdm_layout_recommendation(esdm_instance_t *esdm, esdm_fragment_t* in, esdm_fragment_t* out);
esdm_status_t esdm_layout_stat(char *desc);

// Performance Model
esdm_performance_t* esdm_performance_init(esdm_instance_t *esdm);
esdm_status_t esdm_performance_recommendation(esdm_instance_t *esdm, esdm_fragment_t* in, esdm_fragment_t* out);
esdm_status_t esdm_performance_finalize();


// Backend (generic)
esdm_status_t esdm_backend_estimate_performance(esdm_backend_t *backend, int fragment);
esdm_status_t esdm_backend_io(esdm_backend_t *backend, esdm_fragment_t *fragment, esdm_metadata_t *metadata);




// Auxiliary
void esdm_print_hashtable (GHashTable * tbl);





#endif
