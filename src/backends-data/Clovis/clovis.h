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
 *
 * Original author: Huang Hua <hua.huang@seagate.com>
 * Original creation date: 13-Oct-2017
 */
#ifndef CLOVIS_H
#define CLOVIS_H

#include "esdm.h"

/**
* Initializes the CLOVIS plugin. In particular this involves:
*
*    * Load configuration of this backend
*    * Load and potenitally calibrate performance model
*
*    * Connect with support services e.g. for technical metadata
*    * Setup directory structures used by this CLOVIS specific backend
*
*    * Poopulate esdm_backend_t struct and callbacks required for registration
*
* @return pointer to backend struct
*/

esdm_backend_t *clovis_backend_init (esdm_config_backend_t * config);

#endif
