#ifndef ESDM_DEBUG_H
#define ESDM_DEBUG_H

#include <assert.h>

#define ESDM_DEBUG(msg) esdm_log(ESDM_LOGLEVEL_DEBUG, "[ESDM] %-30s %s:%d\n", msg, __FILE__, __LINE__)
#define ESDM_ERROR(msg) esdm_log(ESDM_LOGLEVEL_ERROR, "[ESDM] ERROR: %-30s %s:%d\n", msg, __FILE__, __LINE__); exit(1)
#define ESDM_LOG(loglevel, msg) esdm_log(loglevel, "[ESDM] %-30s %s:%d\n", msg, __FILE__, __LINE__)


#endif
