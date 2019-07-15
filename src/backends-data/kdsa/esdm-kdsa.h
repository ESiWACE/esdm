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
#ifndef ESDM_BACKENDS_KDSA_H
#define ESDM_BACKENDS_KDSA_H

#include <esdm-internal.h>

#include <backends-data/generic-perf-model/lat-thr.h>

/*
A module specification in the configuration file:
{

        "type": "KDSA",
        "id": "p1",
        "target": "This is the XPD connection string",
        "max-threads-per-node" : 0,
        "max-fragment-size" : 1048,
        "accessibility" : "global"
}
As the KDSA plugin works asynchronously, no threads are needed inside ESDM.


Key design questions:
 * How to keep track of free space for fragments using a block bitmap?
 * How to update the block bitmap concurrently?
   * Compare and swap function
 * How to utilize the asynchronous interface from the XPD
*/

esdm_backend_t *kdsa_backend_init(esdm_config_backend_t *config);

#endif
