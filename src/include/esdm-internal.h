/**
 * @file
 * @brief Internal ESDM functionality, not to be used by backends and plugins.
 *
 */
#ifndef ESDM_INTERNAL_H
#define ESDM_INTERNAL_H


#include <esdm.h>



void esdm_log(uint32_t loglevel, const char* format, ...);

#define ESDM_DEBUG(loglevel, msg) esdm_log(loglevel, "[ESDM] %-30s %s:%d\n", msg, __FILE__, __LINE__)
#define ESDM_LOG(loglevel, msg) esdm_log(loglevel, "[ESDM] %-30s %s:%d\n", msg, __FILE__, __LINE__)






// Data


// Metadata
esdm_status_t esdm_metadata_t_alloc();



// I/O Scheduler
esdm_status_t esdm_scheduler_submit();


// Layout



// Performance Model
esdm_status_t esdm_perf_model_split_io(esdm_pending_fragments_t* io, esdm_fragment_t* fragments);


// Backend (generic)
esdm_status_t esdm_backend_estimate_performance(esdm_backend_t* backend, int fragment);
esdm_status_t esdm_backend_io(esdm_backend_t* backend, esdm_fragment_t, int metadata);




#endif
