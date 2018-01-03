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
#define ESDM_ERROR(msg) esdm_log(ESDM_LOGLEVEL_ERROR, "[ESDM] %-30s %s:%d\n", msg, __FILE__, __LINE__)
#define ESDM_LOG(loglevel, msg) esdm_log(loglevel, "[ESDM] %-30s %s:%d\n", msg, __FILE__, __LINE__)






// Datatypes
esdm_status_t esdm_fragment_create();
esdm_status_t esdm_fragment_destroy();

esdm_status_t esdm_dataset_create();
esdm_status_t esdm_dataset_destroy();



// Metadata
esdm_status_t esdm_metadata_t_alloc();



// I/O Scheduler
esdm_status_t esdm_scheduler_init();
esdm_status_t esdm_scheduler_submit();


// Layout
esdm_status_t esdm_layout_stat(char * desc);


// Performance Model
esdm_status_t esdm_perf_model_split_io(esdm_pending_fragments_t* io, esdm_fragment_t* fragments);


// Backend (generic)
esdm_status_t esdm_backend_estimate_performance(esdm_backend_t* backend, int fragment);
esdm_status_t esdm_backend_io(esdm_backend_t* backend, esdm_fragment_t, int metadata);




#endif
