#ifndef ESDM_DEBUG_H
#define ESDM_DEBUG_H

#include <assert.h>
#include <stdint.h>

void esdm_log(uint32_t loglevel, const char *format, ...);

#define ESDM_LOG(fmt) ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, "%s", fmt)
#define ESDM_LOG_FMT(loglevel, fmt, ...) esdm_log(loglevel, "%-30s:%d (%s): " #fmt "\n", __FILE__, __LINE__, __func__, __VA_ARGS__)
#define ESDM_LOG_COM_FMT(loglevel, component, fmt, ...) esdm_log(loglevel, "[%s] %-30s:%d (%s): " #fmt "\n", component, __FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef NDEBUG
// remove debug messages in total
#  define ESDM_DEBUG(fmt)
#  define ESDM_DEBUG_FMT(fmt, ...)
#  define ESDM_DEBUG_COM_FMT(com, fmt, ...)
#else
#  define ESDM_DEBUG(fmt) ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, "%s", fmt)
#  define ESDM_DEBUG_FMT(fmt, ...) ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, fmt, __VA_ARGS__)
#  define ESDM_DEBUG_COM_FMT(component, fmt, ...) ESDM_LOG_COM_FMT(ESDM_LOGLEVEL_DEBUG, component, fmt, __VA_ARGS__)
#endif

#define ESDM_ERROR(fmt) ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, "%s", fmt)
#define ESDM_ERROR_FMT(fmt, ...)                         \
  do {                                                   \
    ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, fmt, __VA_ARGS__); \
    exit(1);                                             \
  } while (0)
#define ESDM_ERROR_COM_FMT(component, fmt, ...)                         \
  do {                                                                  \
    ESDM_LOG_COM_FMT(ESDM_LOGLEVEL_DEBUG, component, fmt, __VA_ARGS__); \
    exit(1);                                                            \
  } while (0)

#endif
