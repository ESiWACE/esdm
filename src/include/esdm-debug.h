#ifndef ESDM_DEBUG_H
#define ESDM_DEBUG_H

#include <assert.h>

#define ESDM_LOG(fmt) ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, "%s", fmt)
#define ESDM_LOG_FMT(loglevel, fmt, ...) esdm_log(loglevel, "%-30s:%d (%s): "#fmt"\n", __FILE__, __LINE__, __func__, __VA_ARGS__)


#ifdef NDEBUG
  // remove debug messages in total
  #define ESDM_DEBUG(fmt)
  #define ESDM_DEBUG_FMT(fmt, ...) 
#else
  #define ESDM_DEBUG(fmt) ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, "%s", fmt)
  #define ESDM_DEBUG_FMT(fmt, ...) ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, fmt, __VA_ARGS__)
#endif

#define ESDM_ERROR(fmt) ESDM_ERROR_FMT(ESDM_LOGLEVEL_DEBUG, "%s", fmt)
#define ESDM_ERROR_FMT(fmt, ...) do { ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, fmt, __VA_ARGS__); exit(1); } while(0)

#endif
