/**
 * @file
 * @brief Internal ESDM functionality, not to be used by backends and plugins.
 *
 */
#ifndef ESDM_INTERNAL_H
#define ESDM_INTERNAL_H


#include <esdm.h>



void esdm_log(uint32_t loglevel, const char* format, ...);

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

#define ESDM_DEBUG(msg) esdm_log(ESDM_LOGLEVEL_DEBUG, "[ESDM] %-30s %s:%d\n", msg, __FILE__, __LINE__)
#define ESDM_ERROR(msg) esdm_log(ESDM_LOGLEVEL_ERROR, "[ESDM] ERROR: %-30s %s:%d\n", msg, __FILE__, __LINE__); exit(1)
#define ESDM_LOG(loglevel, msg) esdm_log(loglevel, "[ESDM] %-30s %s:%d\n", msg, __FILE__, __LINE__)




// Fragment
esdm_fragment_t* esdm_fragment_create();
esdm_status_t esdm_fragment_commit(esdm_fragment_t *fragment);
esdm_status_t esdm_fragment_destroy(esdm_fragment_t *fragment);
esdm_status_t esdm_fragment_serialize(esdm_fragment_t *fragment, char **out);
esdm_fragment_t* esdm_fragment_deserialize(char *serialized_fragment);

// Datset
esdm_dataset_t* esdm_dataset_create();
esdm_status_t esdm_dataset_commit(esdm_dataset_t *dataset);
esdm_status_t esdm_dataset_destroy(esdm_dataset_t *dataset);

// Container
esdm_container_t* esdm_container_create();
esdm_status_t esdm_container_commit(esdm_container_t *container);
esdm_status_t esdm_container_destroy(esdm_container_t *container);

// Dataspace
esdm_dataspace_t* esdm_dataspace_create();
esdm_status_t esdm_dataspace_destroy(esdm_dataspace_t *dataspace);
esdm_status_t esdm_dataspace_serialize(esdm_dataspace_t *dataspace, char **out);
esdm_dataspace_t* esdm_dataspace_deserialize(char *serialized_dataspace);



// Metadata
esdm_status_t esdm_metadata_t_alloc();



// ESDM Core //////////////////////////////////////////////////////////////////

// Configuration
esdm_config_t* esdm_config_init(esdm_instance_t *esdm);
esdm_status_t esdm_config_finalize(esdm_instance_t *esdm);

esdm_config_backends_t* esdm_config_get_backends(esdm_instance_t *esdm);

// Modules
esdm_modules_t* esdm_modules_init(esdm_instance_t *esdm);
esdm_status_t esdm_modules_finalize();
esdm_status_t esdm_modules_register();

esdm_status_t esdm_modules_get_by_type(esdm_module_type_t type, esdm_module_type_array_t *array);

// I/O Scheduler
esdm_scheduler_t* esdm_scheduler_init(esdm_instance_t *esdm);
esdm_status_t esdm_scheduler_submit();

// Layout
esdm_layout_t* esdm_layout_init(esdm_instance_t *esdm);
esdm_status_t esdm_layout_finalize();
esdm_status_t esdm_layout_stat(char *desc);

// Performance Model
esdm_performance_t* esdm_performance_init(esdm_instance_t *esdm);
esdm_status_t esdm_performance_finalize();
esdm_status_t esdm_perf_model_split_io(esdm_pending_fragments_t *io, esdm_fragment_t *fragments);


// Backend (generic)
esdm_status_t esdm_backend_estimate_performance(esdm_backend_t *backend, int fragment);
esdm_status_t esdm_backend_io(esdm_backend_t *backend, esdm_fragment_t *fragment, esdm_metadata_t *metadata);




#endif
