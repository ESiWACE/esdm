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

static GHashTable* file_table;

static void * memvol_file_create(const char *name, unsigned flags, hid_t fcpl_id, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    puts("------------ memvol_file_create() called ------------");

    //speicher allocieren
    memvol_file_t* file = (memvol_file_t *)malloc(sizeof(memvol_file_t));
    memvol_object_t* object = (memvol_object_t *)malloc(sizeof(memvol_object_t));

    file->root_group = (memvol_group_t *)malloc(sizeof(memvol_group_t));
    file->name = (char *)malloc(strlen(name));
    file->root_group->name = (char *)malloc(strlen("/"));

    object->type = (memvol_object_type)malloc(sizeof(memvol_object_type));
    object->subclass = (memvol_group_t *)malloc(sizeof(memvol_group_t));

    //werte initialisieren
    strcpy(file->name, name);

    file->root_group->children = g_hash_table_new(g_str_hash, g_str_equal);
    strcpy(file->root_group->name, "/");

    object->type = GROUP_T;
    object->subclass = (memvol_group_t *)file->root_group;

    g_hash_table_insert(file->root_group->children, strdup("/"), object);

    if (file_table == NULL) {
        file_table = g_hash_table_new(g_str_hash, g_str_equal);
    }

    g_hash_table_insert(file_table, strdup(name), file);

    //debug ausgaben
    printf("Datei %s (%p) erstellt!\n", file->name, (void*)file);
    
    if (file->root_group != NULL) {
        printf("Root-Group '/' (%p)\n", (void*)file->root_group);

    } else {
        puts("Keine Root Gruppe erstellt!");

    }

    puts("------------------------------------------------------");
    puts("");

    return (void *)object;
}

static void * memvol_file_open(const char *name, unsigned flags, hid_t fapl_id, hid_t dxpl_id, void **req)
{
    puts("------------ memvol_file_open() called ---------------");

    memvol_file_t* ret = g_hash_table_lookup(file_table, name);

    if (ret == NULL) {
        puts("File existiert nicht!");
    } else {
        printf("Datei %s (%p) geoeffnet.\n", ret->name, (void*)ret);
    }

    puts("------------------------------------------------------");
    puts("");

    return (void *)ret;
}

static herr_t memvol_file_get(void *file, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
    memvol_file_t *f = (memvol_file_t *)file;
    return 1;
}

static herr_t memvol_file_close(void *obj, hid_t dxpl_id, void **req)
{
    puts("memvol_file_close() called!");

    memvol_group_t* f = (memvol_group_t *)((memvol_object_t *)obj)->subclass;
    printf("Root-Group-Pointer %p\n", (void*)f);
    
    //free(f->name);
    //g_free(f->children);
    //free(f);
    //f->name = NULL;
    //f->children = NULL;
    //f = NULL;

    return 1;
}
