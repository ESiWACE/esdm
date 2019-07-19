/*
 * This file is a primitive stub for the KDSA API to allow testing of the KDSA backend without having the proprietary XPD library.
 */
#ifndef __XPD_DUMMY_H
#define __XPD_DUMMY_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#define KDSA_FLAGS_HANDLE_USE_EVENT 1L<<0
#define KDSA_FLAGS_HANDLE_IO_NOSPIN 1L<<1

typedef uint64_t kdsa_vol_offset_t;
typedef uint64_t kdsa_size_t;
typedef int* kdsa_vol_handle_t;

int kdsa_write_unregistered(kdsa_vol_handle_t handle, kdsa_vol_offset_t off, void* buf, kdsa_size_t bytes);
int kdsa_read_unregistered(kdsa_vol_handle_t handle, kdsa_vol_offset_t off,  void* buf, kdsa_size_t bytes);
int kdsa_connect(char* connection_string, uint32_t flags, kdsa_vol_handle_t *handle);
int kdsa_disconnect(kdsa_vol_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif
