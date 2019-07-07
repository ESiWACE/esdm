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

#include <esdm-internal.h>
#include <glib.h>
#include <stdarg.h>
#include <stdio.h>

static esdm_loglevel_e global_loglevel = ESDM_LOGLEVEL_INFO;

void esdm_loglevel(esdm_loglevel_e loglevel){
  global_loglevel = loglevel;
}

void esdm_log(uint32_t loglevel, const char *format, ...) {

  if (loglevel <= global_loglevel) {
    va_list args;
    printf("[ESDM] ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
  }
}
