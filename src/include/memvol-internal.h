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

typedef struct {
    GHashTable* children;
    char* name;
} memvol_group_t;

typedef struct {
    char* name;
} memvol_dataset_t;

typedef struct {
    memvol_group_t* root_group;
    char* name;
} memvol_file_t;

typedef enum {
    GROUP_T
} memvol_object_type;

typedef struct {
    memvol_object_type type;
    void* subclass; //entweder memvol_group_t* oder memvol_file_t* -> type member
} memvol_object_t;
#endif
