/* This file is part of ESDM.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESDM.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @brief Debug adds functionality for logging and inspection of ESDM types
 *        during development.
 *
 */

#include <errno.h>
#include <esdm-datatypes.h>
#include <esdm-internal.h>
#include <fcntl.h>
#include <glib.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

bool ea_is_valid_dataset_name(char const*str) {
  // TODO allow names with a-a, A-Z,0-9,_-
  eassert(str != NULL);
  char last = 0;
  if(*str == 0){
    return 0;
  }
  for(char const * p = str; p[0] != 0 ; p++){
    char c = *p;
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c=='_' || c=='-' || c=='%'){
      last = c;
      continue;
    }
    if (c=='/'){
      if(last == 0){
        // cannot start the name with "/"
        return 0;
      }
      if(last == '/'){
        // cannot have two consecutive "/"
        return 0;
      }
      last = c;
      continue;
    }
    return 0;
  }
  if(last == '/'){
    // cannot end with "/"
    return 0;
  }
  return 1;
}

// directory handling /////////////////////////////////////////////////////////

int mkdir_recursive(const char *path) {
  char tmp[PATH_MAX];
  char *p = NULL;
  size_t len;

  // copy provided path, as we modify it
  snprintf(tmp, PATH_MAX, "%s", path);

  // check if last char of string is a /
  len = strlen(tmp);
  if (tmp[len - 1] == '/') tmp[len - 1] = 0;
  // if it is, set to 0

  // traverse string from start to end
  for (p = tmp + 1; *p; p++) {
    if (*p == '/' && (p - tmp) > 1) {
      // if current char is a /
      // temporaly set character at address p to 0
      // create dir from start of string
      // reset char at address back to /
      *p = 0;
      mkdir(tmp, S_IRWXU);
      *p = '/';
    }
    // continue with next position in string
  }
  return mkdir(tmp, S_IRWXU);
}

int posix_recursive_remove(const char *path) {
  ESDM_INFO_COM_FMT("AUX", "removing %s", path);
  struct stat sb = {0};
  int ret = stat(path, &sb);
  if(ret) return ESDM_ERROR;

  bool isDir = (sb.st_mode & S_IFMT) == S_IFDIR;
  if(isDir) {
    //Create a dummy file that will later be removed along with the other files in this directory.
    //
    //XXX: This is a really dirty workaround to reduce the impact of a caching problem:
    //     It appears that just unlinking a file/directory won't cause the kernel to cache some data associated with the containing directory.
    //     Adding a file, however, will cause this caching, making the subsequent stat()/unlink() operations more efficient.
    //     The effect is not huge, but still quite noticeable when we are dealing with more than 200k fragments.
    //
    //     Nevertheless, it seems we need to avoid creating hierarchies containing millions of files somehow.
    char child_path[PATH_MAX];
    sprintf(child_path, "%s/dummy", path);
    int dummyFile = creat(child_path, S_IRUSR | S_IWUSR); //force the directory to be cached
    if(dummyFile != -1) close(dummyFile);

    DIR *dir = opendir(path);
    if(!dir) return ESDM_ERROR;

    struct dirent *f;
    while(!ret && (f = readdir(dir))) {
      if (strcmp(f->d_name, ".") != 0 && strcmp(f->d_name, "..") != 0) {
        sprintf(child_path, "%s/%s", path, f->d_name);
        ret = posix_recursive_remove(child_path);
      }
    }
    ret |= closedir(dir);
  }

  if(!ret) ret = unlinkat(AT_FDCWD, path, isDir ? AT_REMOVEDIR : 0);

  return ret ? ESDM_ERROR : ESDM_SUCCESS;
}

// file I/O handling //////////////////////////////////////////////////////////
int ea_read_file(char *filepath, char **buf) {
  eassert(buf);

  int fd = open(filepath, O_RDONLY);
  if (fd < 0) {
    ESDM_WARN_COM_FMT("POSIX", "cannot open %s %s", filepath, strerror(errno));
    return 1;
  }

  off_t fsize = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);

  char *string = ea_checked_malloc(fsize + 1);
  int ret = ea_read_check(fd, string, fsize);
  close(fd);

  string[fsize] = 0;

  *buf = string;

  ESDM_DEBUG_COM_FMT("AUX", "read_file(): %s\n", string);
  return ret;
}

int ea_write_check(int fd, char *buf, size_t len) {
  while (len > 0) {
    ssize_t ret = write(fd, buf, len);
    if (ret != -1) {
      buf = buf + ret;
      len -= ret;
    } else {
      if (errno == EINTR) {
        continue;
      } else {
        ESDM_ERROR_COM_FMT("POSIX", "write %s", strerror(errno));
        return 1;
      }
    }
  }
  return 0;
}

int ea_read_check(int fd, char *buf, size_t len) {
  while (len > 0) {
    ssize_t ret = read(fd, buf, len);
    if (ret == 0) {
      return 1;
    } else if (ret != -1) {
      buf += ret;
      len -= ret;
    } else {
      if (errno == EINTR) {
        continue;
      } else {
        ESDM_ERROR_COM_FMT("POSIX", "read %s", strerror(errno));
        return 1;
      }
    }
  }
  return 0;
}

// POSIX other ////////////////////////////////////////////////////////////////

void print_stat(struct stat sb) {
  printf("\n");
  printf("File type:                ");
  switch (sb.st_mode & S_IFMT) {
    case S_IFBLK: printf("block device\n"); break;
    case S_IFCHR: printf("character device\n"); break;
    case S_IFDIR: printf("directory\n"); break;
    case S_IFIFO: printf("FIFO/pipe\n"); break;
    case S_IFLNK: printf("symlink\n"); break;
    case S_IFREG: printf("regular file\n"); break;
    case S_IFSOCK: printf("socket\n"); break;
    default: printf("unknown?\n"); break;
  }
  printf("I-node number:            %ld\n", (long)sb.st_ino);
  printf("Mode:                     %lo (octal)\n", (unsigned long)sb.st_mode);
  printf("Link count:               %ld\n", (long)sb.st_nlink);
  printf("Ownership:                UID=%ld   GID=%ld\n", (long)sb.st_uid, (long)sb.st_gid);
  printf("Preferred I/O block size: %ld bytes\n", (long)sb.st_blksize);
  printf("File size:                %lld bytes\n", (long long)sb.st_size);
  printf("Blocks allocated:         %lld\n", (long long)sb.st_blocks);
  printf("Last status change:       %s", ctime(&sb.st_ctime));
  printf("Last file access:         %s", ctime(&sb.st_atime));
  printf("Last file modification:   %s", ctime(&sb.st_mtime));
  printf("\n");
}

// use json_decref() to free it
json_t *load_json(const char *str) {
  json_error_t error;
  json_t *root = json_loads(str, 0, &error);
  if (!root) {
    ESDM_DEBUG_FMT("JSON error on line %d: %s\n", error.line, error.text);
    return (json_t *)NULL;
  }
  return root;
}


int ea_compute_hash_str(const char * str){
  int hash = 0;
  for(; *str != 0; str++){
    hash = (hash<<3) + *str;
  }
  return hash;
}

static void getRandom(uint8_t* bytes, size_t count) {
  static FILE* randomSource = NULL;
  if(!randomSource) {
    randomSource = fopen("/dev/urandom", "r");
    eassert(randomSource);
  }

  while(count) {
    size_t chunkSize = fread(bytes, sizeof(*bytes), count, randomSource);
    bytes += chunkSize;
    count -= chunkSize;
  }
}

static const char kCharset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";
#define kCharsetBits 6
_Static_assert(sizeof(kCharset) - 1 == 1 << kCharsetBits, "wrong number of characters in kCharset");

void ea_generate_id(char* str, size_t length) {
  size_t randomBitCount = length*kCharsetBits;
  eassert(randomBitCount >= 128 && "don't try to use less than 128 random bits to avoid possibility of collision");

  size_t randomByteCount = (randomBitCount + 7)/8;
  uint8_t randomBytes[randomByteCount];
  getRandom(randomBytes, randomByteCount);

  uint64_t bitCache = 0;
  size_t cachedBits = 0, usedRandomBytes = 0;
  for(size_t i = 0; i < length; i++) {
    //load enough bits into the cache
    while(cachedBits < kCharsetBits) {
      eassert(usedRandomBytes < randomByteCount);
      bitCache = (bitCache << 8) | randomBytes[usedRandomBytes++];
      cachedBits += 8;
    }

    //take kCharsetBits out of the cache
    size_t index = bitCache & ((1 << kCharsetBits) - 1);
    bitCache >>= kCharsetBits;
    cachedBits -= kCharsetBits;

    //use those random bits to output a character
    str[i] = kCharset[index];
  }
  str[length] = 0;  //termination
}

char* ea_make_id(size_t length) {
  char* result = ea_checked_malloc(length + 1);
  ea_generate_id(result, length);
  return result;
}

void* ea_checked_malloc(size_t size) {
  void* result = malloc(size);
  if(!result) {
    fprintf(stderr, "out-of-memory error: could not allocate a block of %zd bytes, aborting...\n", size);
    abort();
  }
  return result;
}

void* ea_checked_calloc(size_t nmemb, size_t size) {
  void* result = calloc(nmemb, size);
  if(!result) {
    fprintf(stderr, "out-of-memory error: could not allocate a block for %zd objects of %zd bytes, aborting...\n", nmemb, size);
    abort();
  }
  return result;
}

void* ea_checked_realloc(void* ptr, size_t size) {
  void* result = realloc(ptr, size);
  if(!result && size) {
    fprintf(stderr, "out-of-memory error: could not reallocate a block to a size of %zd bytes, aborting...\n", size);
    abort();
  }
  return result;
}

void* ea_memdup(void* data, size_t size) {
  void* result = ea_checked_malloc(size);
  memcpy(result, data, size);
  return result;
}

#ifdef ESM

void ea_start_timer(timer *t1) {
  *t1 = clock64();
}

double ea_stop_timer(timer t1) {
  timer end;
  ea_start_timer(&end);
  return (end - t1) / 1000.0 / 1000.0;
}

double ea_timer_subtract(timer number, timer subtract) {
  return (number - subtract) / 1000.0 / 1000.0;
}

#else // POSIX COMPLAINT

void ea_start_timer(timer *t1) {
  clock_gettime(CLOCK_MONOTONIC, t1);
}

static timer time_diff(struct timespec end, struct timespec start) {
  struct timespec diff;
  if (end.tv_nsec < start.tv_nsec) {
    diff.tv_sec = end.tv_sec - start.tv_sec - 1;
    diff.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  } else {
    diff.tv_sec = end.tv_sec - start.tv_sec;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return diff;
}

static double time_to_double(struct timespec t) {
  double d = (double)t.tv_nsec;
  d /= 1000000000.0;
  d += (double)t.tv_sec;
  return d;
}

double ea_timer_subtract(timer number, timer subtract) {
  return time_to_double(time_diff(number, subtract));
}

double ea_stop_timer(timer t1) {
  timer end;
  ea_start_timer(&end);
  return time_to_double(time_diff(end, t1));
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// data conversion /////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//define the converter functions

#define defineConverter(destType, sourceType) \
  void* convert_from_##sourceType##_to_##destType(void* vdest, const void* vsource, size_t sourceBytes) { \
    sourceType const* source = vsource; \
    destType* dest = vdest; \
    size_t elementCount = sourceBytes/sizeof*source; \
    eassert(elementCount*sizeof(sourceType) == sourceBytes); \
    for(size_t i = 0; i < elementCount; i++) dest[i] = (destType)source[i]; \
    return dest; \
  }

#define defineConvertersForSourceType(sourceType) \
  defineConverter(int8_t, sourceType) \
  defineConverter(int16_t, sourceType) \
  defineConverter(int32_t, sourceType) \
  defineConverter(int64_t, sourceType) \
  defineConverter(uint8_t, sourceType) \
  defineConverter(uint16_t, sourceType) \
  defineConverter(uint32_t, sourceType) \
  defineConverter(uint64_t, sourceType) \
  defineConverter(float, sourceType) \
  defineConverter(double, sourceType)

defineConvertersForSourceType(int8_t)
defineConvertersForSourceType(int16_t)
defineConvertersForSourceType(int32_t)
defineConvertersForSourceType(int64_t)
defineConvertersForSourceType(uint8_t)
defineConvertersForSourceType(uint16_t)
defineConvertersForSourceType(uint32_t)
defineConvertersForSourceType(uint64_t)
defineConvertersForSourceType(float)
defineConvertersForSourceType(double)

//define the selector function
ea_datatype_converter ea_converter_for_types(esdm_type_t requestDestType, esdm_type_t requestSourceType) {
  if(requestDestType == requestSourceType) return memcpy; //fast path for all noop conversions

  #define selectConverterForDest(destType, esdmDestType, sourceType) \
    if(esdmDestType == requestDestType) return convert_from_##sourceType##_to_##destType;

  #define selectConvertersForSource(sourceType, esdmSourceType) \
    if(esdmSourceType == requestSourceType) { \
      selectConverterForDest(int8_t, SMD_DTYPE_INT8, sourceType) \
      selectConverterForDest(int16_t, SMD_DTYPE_INT16, sourceType) \
      selectConverterForDest(int32_t, SMD_DTYPE_INT32, sourceType) \
      selectConverterForDest(int64_t, SMD_DTYPE_INT64, sourceType) \
      selectConverterForDest(uint8_t, SMD_DTYPE_UINT8, sourceType) \
      selectConverterForDest(uint16_t, SMD_DTYPE_UINT16, sourceType) \
      selectConverterForDest(uint32_t, SMD_DTYPE_UINT32, sourceType) \
      selectConverterForDest(uint64_t, SMD_DTYPE_UINT64, sourceType) \
      selectConverterForDest(float, SMD_DTYPE_FLOAT, sourceType) \
      selectConverterForDest(double, SMD_DTYPE_DOUBLE, sourceType) \
      return NULL; \
    }

  selectConvertersForSource(int8_t, SMD_DTYPE_INT8)
  selectConvertersForSource(int16_t, SMD_DTYPE_INT16)
  selectConvertersForSource(int32_t, SMD_DTYPE_INT32)
  selectConvertersForSource(int64_t, SMD_DTYPE_INT64)
  selectConvertersForSource(uint8_t, SMD_DTYPE_UINT8)
  selectConvertersForSource(uint16_t, SMD_DTYPE_UINT16)
  selectConvertersForSource(uint32_t, SMD_DTYPE_UINT32)
  selectConvertersForSource(uint64_t, SMD_DTYPE_UINT64)
  selectConvertersForSource(float, SMD_DTYPE_FLOAT)
  selectConvertersForSource(double, SMD_DTYPE_DOUBLE)
  return NULL;
}
