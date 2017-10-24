/**
 * @file
 * @brief Internal ESDM functionality, not to be used by backends and plugins.
 *
 */
#ifndef ESDM_INTERNAL_H
#define ESDM_INTERNAL_H


#include <esdm.h>
#include <esdm-utils.h>



void esdm_log(uint32_t loglevel, const char* format, ...);

#define ESDM_DEBUG(loglevel, msg) esdm_log(loglevel, "[ESMD] %-30s %s:%d\n", msg, __FILE__, __LINE__)
#define ESDM_LOG(loglevel, msg) esdm_log(loglevel, "[ESDM] %-30s %s:%d\n", msg, __FILE__, __LINE__)





#endif
