// This is a dummy implementation of the KDSA API for debugging purposes.
// It stores the data simply in a file.

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <esdm-debug.h>
#include <esdm-internal.h>
#include <kdsa.h>

//#define DEBUG
//#define KDSA_DUMMY_USE_FILE_PER_OFF

#ifdef DEBUG
  #define debug(...) printf(__VA_ARGS__);
#else
  #define debug(...)
#endif

#define FUNC_START debug("CALL %s\n", __func__);


int kdsa_compare_and_swap(kdsa_vol_handle_t handle, kdsa_vol_offset_t off, uint64_t compare, uint64_t swap, uint64_t *out_res){
  int fd = *(int*) handle;

  struct flock fl;
  memset(&fl, 0, sizeof(fl));

  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start = off;
  fl.l_len = sizeof(uint64_t);
  if (fcntl(fd, F_SETLK, &fl) != 0) {
    return -1;
  }

  debug("C&S: %lu %lu %lu\n", off, compare, swap);

  uint64_t cur;
  int ret;
  ret = pread(fd, & cur, sizeof(uint64_t), off);
  eassert(ret == sizeof(uint64_t));
  if(cur == compare){
    ret = pwrite(fd, & swap, sizeof(uint64_t), off);
    eassert(ret == sizeof(uint64_t));
  }
  *out_res = cur;

  fl.l_type = F_UNLCK;
  if (fcntl(fd, F_SETLK, &fl) != 0) {
    return -1;
  }

  return 0;
}

int kdsa_get_volume_size(kdsa_vol_handle_t handle, kdsa_size_t *out_size){
  *out_size = 10llu*1024*1024*1024;
  return 0;
}


int kdsa_write_unregistered(kdsa_vol_handle_t handle, kdsa_vol_offset_t off, void* buf, kdsa_size_t bytes){
  FUNC_START
  debug("W: %lu (%lu)\n", off, bytes);
  size_t ret;

// detailed debugging
#ifdef KDSA_DUMMY_USE_FILE_PER_OFF
  char name[256];
  sprintf(name, "kdsa-%ld.bin", off);
  int fd = open(name, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR|S_IWUSR);
  ret = write(fd, buf, bytes);
  close(fd);
  assert(ret == bytes);
  return 0;
#endif

  int f = *(int*) handle;
  ret = pwrite(f, buf, bytes, off);
  assert(ret == bytes);
  //fdatasync(f);
  return 0;
}

int kdsa_read_unregistered(kdsa_vol_handle_t handle, kdsa_vol_offset_t off,  void* buf, kdsa_size_t bytes){
  FUNC_START
  debug("R: %lu (%lu)\n", off, bytes);
  size_t ret;

#ifdef KDSA_DUMMY_USE_FILE_PER_OFF
  char name[256];
  sprintf(name, "kdsa-%ld.bin", off);
  int fd = open(name, O_RDONLY);
  ret = pread(fd, buf, bytes, 0);
  close(fd);
  return 0;
#endif

  int f = *(int*) handle;
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
  }
  int * fd = ea_checked_malloc(sizeof(int));
  *fd = f;

  *handle = (kdsa_vol_handle_t) fd;

  kdsa_size_t size;
  kdsa_get_volume_size(fd, & size);
  pwrite(f, & size, 1, size);

  return 0;
}

int kdsa_disconnect(kdsa_vol_handle_t handle){
  FUNC_START
  int ret = close(*(int*)handle);
  free(handle);
  return ret;
}

int kdsa_set_read_buffer_size(kdsa_vol_handle_t handle, size_t new_read_buffer_size){
  return 0;
}

int kdsa_set_write_buffer_size(kdsa_vol_handle_t handle, size_t new_write_buffer_size){
  return 0;
}
