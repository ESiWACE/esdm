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
#include <esdm-debug.h>

// generate test patterns
void etest_gen_buffer(int dims, int64_t bounds[], uint64_t ** out_buff);
int etest_verify_buffer(int dims, int64_t bounds[], uint64_t * buff);
void etest_memset_buffer(int dims, int64_t bounds[], uint64_t * buff);

//A variant of eassert() that allows us to check that the call actually *crashes* the program.
//This is used to check for the presence of the appropriate eassert() calls to check function contracts.
//Checks for abnormal termination by some signal.
#ifdef NDEBUG
  //Must compile out `eassert_crash()` if `assert()` is compiled out as it is supposed to check for a crash produced by some other `assert()`.
  //If that other `assert()` were compiled out in a production built while the checking `eassert_crash()` is left in,
  //the result would be a production-built-only failure.
  #define eassert_crash(call)
#else
  #define eassert_crash(call) do { \
    printf("checking for expected crash...\n"); \
    pid_t child = fork(); \
    if(child) { \
      int status; \
      pid_t result = waitpid(child, &status, 0); \
      eassert(result == child); \
      eassert(WIFSIGNALED(status)); \
    } else { \
      call; \
      exit(0); \
    } \
    printf("... OK, child crashed successfully\n"); \
  } while(0)
#endif

//A variant of eassert() that allows us to check that the call triggers a fatal error bailout.
//This is used to check for the presence of the appropriate exit() or ERROR_ESDM*() calls to check function contracts.
//Checks for normal termination with a non-zero status.
#define eassert_bailout(call) do { \
  printf("checking for expected error...\n"); \
  pid_t child = fork(); \
  if(child) { \
    int status; \
    pid_t result = waitpid(child, &status, 0); \
    eassert(result == child); \
    eassert(WIFEXITED(status)); \
    eassert(WEXITSTATUS(status)); \
  } else { \
    call; \
    exit(0); \
  } \
  printf("... OK, child exited with an error\n"); \
} while(0)



#endif
