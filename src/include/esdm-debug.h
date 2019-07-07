#ifndef ESDM_DEBUG_H
#define ESDM_DEBUG_H

#include <assert.h>
#include <stdint.h>

void esdm_log(uint32_t loglevel, const char *format, ...);

#define ESDM_LOG(fmt) ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, "%s", fmt)
#define ESDM_LOG_FMT(loglevel, fmt, ...) esdm_log(loglevel, "%s:%d %s(): " fmt "\n", __FILENAME__, __LINE__, __func__, __VA_ARGS__)
#define ESDM_LOG_COM_FMT(loglevel, component, fmt, ...) esdm_log(loglevel, "[%s] %s:%d %s(): " fmt "\n", component, __FILENAME__, __LINE__, __func__, __VA_ARGS__)

#ifdef DEBUG_OFF
// remove debug messages in total
#  define ESDM_DEBUG(fmt)
#  define ESDM_DEBUG_FMT(fmt, ...)
#  define ESDM_DEBUG_COM_FMT(com, fmt, ...)
#else
#  define ESDM_DEBUG(fmt) ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, "%s", fmt)
#  define ESDM_DEBUG_FMT(fmt, ...) ESDM_LOG_FMT(ESDM_LOGLEVEL_DEBUG, fmt, __VA_ARGS__)
#  define ESDM_DEBUG_COM_FMT(component, fmt, ...) ESDM_LOG_COM_FMT(ESDM_LOGLEVEL_DEBUG, component, fmt, __VA_ARGS__)
#endif

//These macros terminate the process.
//If you only want to log an error, use the ESDM_LOG*() macros instead.
#define ESDM_ERROR(fmt) \
  do { \
    ESDM_LOG_FMT(ESDM_LOGLEVEL_ERROR, "ERROR %s", fmt); \
    exit(1); \
  } while (0)
#define ESDM_ERROR_FMT(fmt, ...)                         \
  do {                                                   \
    ESDM_LOG_FMT(ESDM_LOGLEVEL_ERROR, "ERROR " fmt, __VA_ARGS__); \
    exit(1);                                             \
  } while (0)
#define ESDM_ERROR_COM_FMT(component, fmt, ...)                         \
  do {                                                                  \
    ESDM_LOG_COM_FMT(ESDM_LOGLEVEL_ERROR, component, "ERROR " fmt, __VA_ARGS__); \
    exit(1);                                                            \
  } while (0)

#define ESDM_INFO(fmt) \
  do { \
    ESDM_LOG_FMT(ESDM_LOGLEVEL_INFO, "INFO %s", fmt); \
  } while (0)
#define ESDM_INFO_FMT(fmt, ...)                         \
  do {                                                   \
    ESDM_LOG_FMT(ESDM_LOGLEVEL_INFO, "INFO " fmt, __VA_ARGS__); \
  } while (0)
#define ESDM_INFO_COM_FMT(component, fmt, ...)                         \
  do {                                                                  \
    ESDM_LOG_COM_FMT(ESDM_LOGLEVEL_INFO, component, "INFO " fmt, __VA_ARGS__); \
  } while (0)

#define ESDM_WARN(fmt) \
  do { \
    ESDM_LOG_FMT(ESDM_LOGLEVEL_WARNING, "WARN %s", fmt); \
  } while (0)
#define ESDM_WARN_FMT(fmt, ...)                         \
  do {                                                   \
    ESDM_LOG_FMT(ESDM_LOGLEVEL_WARNING, "WARN " fmt, __VA_ARGS__); \
  } while (0)
#define ESDM_WARN_COM_FMT(component, fmt, ...)                         \
  do {                                                                  \
    ESDM_LOG_COM_FMT(ESDM_LOGLEVEL_WARNING, component, "WARN " fmt, __VA_ARGS__); \
  } while (0)


#endif
