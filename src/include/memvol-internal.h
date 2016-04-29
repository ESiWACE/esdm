// This file is part of h5-memvol.
//
// SCIL is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCIL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with h5-memvol.  If not, see <http://www.gnu.org/licenses/>.

#ifndef H5_MEMVOL_INTERNAL_HEADER__
#define H5_MEMVOL_INTERNAL_HEADER__

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <glib.h>

#define DEBUG_INTERNALS // for now...

#ifdef DEBUG
  #define debug(...) fprintf(stderr, "[MEMVOL DEBUG] "__VA_ARGS__);
#else
  #define debug(...)
#endif

#ifdef DEBUG_INTERNALS
  #define debugI(...) fprintf(stderr, "[MEMVOL DEBUG I] "__VA_ARGS__);
#else
  #define debugI(...)
#endif

#define critical(...) { fprintf(stderr, "[MEMVOL CRITICAL] "__VA_ARGS__); exit(1); }
#define warn(...) fprintf(stderr, "[MEMVOL WARN] "__VA_ARGS__);

#define FUNC_START debug("CALL %s\n", __PRETTY_FUNCTION__);

typedef struct {
  GHashTable * childs_tbl;
  hid_t gcpl_id;
} memvol_group_t;

typedef struct {
  memvol_group_t root_grp; // it must start with the root group
} memvol_t;

static void memvol_group_init(memvol_group_t * group);

#endif
