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
#ifndef METADUMMY_H
#define METADUMMY_H

#include <esdm.h>

// Internal functions used by this backend.
typedef struct {
  const char *type;
  const char *name;
  const char *target;
} metadummy_backend_options_t;

// Internal functions used by this backend.
typedef struct {
  metadummy_backend_options_t *options;
  int other;
} metadummy_backend_data_t;

int mkfs(esdm_backend_t *backend);

/**
 * Similar to the command line counterpart fsck for ESDM plugins is responsible
 * to check and potentially repair the "filesystem".
 *
 */
int fsck();

int posix_backend_performance_estimate(esdm_backend_t *backend) {
  int posix_create();

  int posix_open();

  int posix_write();

  int posix_read();

  int posix_close();

  int posix_allocate();

  int posix_update();

  int posix_lookup();

  /**
  * Initializes the POSIX plugin. In particular this involves:
  *
  *	* Load configuration of this backend
  *	* Load and potentially calibrate performance model
  *
  *	* Connect with support services e.g. for technical metadata
  *	* Setup directory structures used by this POSIX specific backend
  *
  *	* Populate esdm_backend_t struct and callbacks required for registration
  *
  * @return pointer to backend struct
  */
  esdm_backend_t *posix_backend_init(void *init_data) {
    int posix_finalize();

    // Those fundtions are not in datadummy.c file

    // int metadummy_backend_performance_estimate(esdm_backend_t *backend);
    //
    // esdm_backend_t *metadummy_backend_init(void *data);

#endif
