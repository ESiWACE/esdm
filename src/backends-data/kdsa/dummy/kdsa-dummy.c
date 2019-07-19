// This is a dummy implementation of the KDSA API for debugging purposes.
// It stores the data simply in a file.

#include <assert.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <kdsa.h>

//#define DEBUG

#ifdef DEBUG
  #define debug(...) printf(__VA_ARGS__);
#else
  #define debug(...)
#endif

#define FUNC_START debug("CALL %s\n", __PRETTY_FUNCTION__);



int kdsa_write_unregistered(kdsa_vol_handle_t handle, kdsa_vol_offset_t off, void* buf, kdsa_size_t bytes){
  FUNC_START
  int f = * handle;
  size_t ret;
  ret = pwrite(f, buf, bytes, off);
  assert(ret == bytes);
  return 0;
}

int kdsa_read_unregistered(kdsa_vol_handle_t handle, kdsa_vol_offset_t off,  void* buf, kdsa_size_t bytes){
  FUNC_START

  int f = * handle;
  size_t ret;
  ret = pread(f, buf, bytes, off);
  if(ret == bytes){
    return 0;
  }else{
    return -1;
  }
}


int kdsa_connect(char* connection_string, uint32_t flags, kdsa_vol_handle_t *handle){
  FUNC_START

  int f = open(connection_string, O_RDWR);
  if (f == -1){
    f = open(connection_string, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR|S_IWUSR);
    if (f == -1){
      return -1;
    }
    size_t size = 0;
    int ret = pwrite(f, & size, sizeof(size), 0);
    assert(ret == sizeof(size));
  }
  int * fd = (int*) malloc(sizeof(int));
  *fd = f;

  *handle = (kdsa_vol_handle_t) fd;

  return 0;
}

int kdsa_disconnect(kdsa_vol_handle_t handle){
  FUNC_START
  int ret = close(*handle);
  free(handle);
  return ret;
}

int kdsa_set_read_buffer_size(kdsa_vol_handle_t handle, size_t new_read_buffer_size){
  return 0;
}

int kdsa_set_write_buffer_size(kdsa_vol_handle_t handle, size_t new_write_buffer_size){
  return 0;
}
