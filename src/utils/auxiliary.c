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
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c=='_' || c=='-'){
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

int read_file(char *filepath, char **buf) {
  eassert(buf);

  int fd = open(filepath, O_RDONLY);
  if (fd < 0) {
    ESDM_ERROR_COM_FMT("POSIX", "cannot open %s %s", filepath, strerror(errno));
    return 1;
  }

  off_t fsize = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);

  char *string = malloc(fsize + 1);
  int ret = read_check(fd, string, fsize);
  close(fd);

  string[fsize] = 0;

  *buf = string;

  ESDM_DEBUG_COM_FMT("AUX", "read_file(): %s\n", string);
  return ret;
}

int write_check(int fd, char *buf, size_t len) {
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

int read_check(int fd, char *buf, size_t len) {
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

void* ea_memdup(void* data, size_t size) {
  void* result = malloc(size);
  memcpy(result, data, size);
  return result;
}

#ifdef ESM

void start_timer(timer *t1) {
  *t1 = clock64();
}

double stop_timer(timer t1) {
  timer end;
  start_timer(&end);
  return (end - t1) / 1000.0 / 1000.0;
}

double timer_subtract(timer number, timer subtract) {
  return (number - subtract) / 1000.0 / 1000.0;
}

#else // POSIX COMPLAINT

void start_timer(timer *t1) {
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

double timer_subtract(timer number, timer subtract) {
  return time_to_double(time_diff(number, subtract));
}

double stop_timer(timer t1) {
  timer end;
  start_timer(&end);
  return time_to_double(time_diff(end, t1));
}

#endif
