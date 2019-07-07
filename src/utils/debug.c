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

#include <glib.h>
#include <stdarg.h>
#include <stdio.h>
#include <execinfo.h>

#include <esdm-internal.h>

static esdm_loglevel_e global_loglevel = ESDM_LOGLEVEL_INFO;
static char logbuffer[4097];
static char * logpointer = logbuffer;
static int log_on_exit = 1;

void esdmI_log_dump(){
  void *array[15];
  size_t size;
  char **strings;
  size_t i;

  if(! log_on_exit){
    return;
  }
  log_on_exit = 0;

  printf("\nESDM has not been shutdown correctly. Stacktrace:\n");

  size = backtrace (array, 15);
  strings = backtrace_symbols(array, size);

  for (i = 3; i < size; i++){
     printf ("%zu: %s\n", i, strings[i]);
  }

  free (strings);

  printf("\nLast messages (circular ring buffer)\n[...]");
  printf("%s\n", logpointer + 1);
  if(logpointer != logbuffer){
    printf("%s", logbuffer);
  }
  printf("\n");
}

void esdm_loglevel(esdm_loglevel_e loglevel){
  global_loglevel = loglevel;
}

void esdm_log_on_exit(int on){
  log_on_exit = on;
}

void esdm_log(uint32_t loglevel, const char *format, ...) {
  if (loglevel <= global_loglevel) {
    va_list args;
    printf("[ESDM] ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
  }
  int len = 4096 - (int)(logpointer - logbuffer);
  va_list args;
  va_start(args, format);
  int count = vsnprintf(logpointer, len, format, args);
  va_end(args);
  if(count > len){
    *logpointer = 0;
    logpointer = logbuffer;
    len = 4096 - (int)(logpointer - logbuffer);
    va_start(args, format);
    int count = vsnprintf(logpointer, len, format, args);
    va_end(args);
    if(count > len){
      printf("ESDM logging error, logmessage is too big\n");
      return;
    }
  }
  logpointer += count;
}
