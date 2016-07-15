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


#include <stdio.h>
#include <string.h>


memvol_file_t* g_file;

static void * memvol_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    puts("memvol_file_create() called!");

    g_file = (memvol_file_t *)calloc(1, sizeof(memvol_file_t));
    printf("filename: %s\n", name);

    int myvalue = 7;

    GHashTable * table;
    
    table = g_hash_table_new (g_str_hash, g_str_equal ); 
   
    g_hash_table_insert(table,"mykey", &myvalue);
    printf("Hash table size: %d\n", g_hash_table_size(table));
  
    g_hash_table_insert(table,"mykey2", &myvalue);
    printf("Hash table size: %d\n", g_hash_table_size(table));

    int* output1 = g_hash_table_lookup(table, "mykey");
    printf("lookup1: %d\n", *output1);

    g_file->name = (char*)malloc(strlen(name));
    g_file->root_group = (memvol_group_t*)malloc(sizeof(*g_file->root_group));
    strcpy(g_file->name, name);

    return (void *)g_file;
}

static void * memvol_file_open(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    memvol_file_t *f;
    f = (memvol_file_t *)calloc(1, sizeof(memvol_file_t));

    printf("memvol_file_open() called!\n");

    return (void *)f;
}

static herr_t memvol_file_get(void *file, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
    memvol_file_t *f = (memvol_file_t *)file;
    return 1;
}

static herr_t memvol_file_close(void *file, hid_t dxpl_id, void **req)
{
    memvol_file_t *f = (memvol_file_t *)file;
    free(f->name);
    free(f->root_group);
    free(f);

    f->name = NULL;
    f->root_group = NULL;
    f = NULL;

    return 1;
}
