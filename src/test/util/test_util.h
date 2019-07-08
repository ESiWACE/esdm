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

#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stdint.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// timer functions
#ifdef ESM
typedef clock64_t timer;
#else
typedef struct timespec timer;
#endif

void start_timer(timer *t1);

double stop_timer(timer t1);

double timer_subtract(timer number, timer subtract);

//A variant of assert() that allows us to check that the call actually *crashes* the program.
//This is used to check for the presence of the appropriate assert() calls to check function contracts.
//Checks for abnormal termination by some signal.
#define assert_crash(call) do { \
  pid_t child = fork(); \
  if(child) { \
    int status; \
    pid_t result = waitpid(child, &status, 0); \
    assert(result == child); \
    assert(WIFSIGNALED(status)); \
  } else { \
    call; \
    exit(0); \
  } \
} while(0)

//A variant of assert() that allows us to check that the call triggers a fatal error bailout.
//This is used to check for the presence of the appropriate exit() or ERROR_ESDM*() calls to check function contracts.
//Checks for normal termination with a non-zero status.
#define assert_bailout(call) do { \
  pid_t child = fork(); \
  if(child) { \
    int status; \
    pid_t result = waitpid(child, &status, 0); \
    assert(result == child); \
    assert(WIFEXITED(status)); \
    assert(WEXITSTATUS(status)); \
  } else { \
    call; \
    exit(0); \
  } \
} while(0)

#endif
