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
    puts("------------ memvol_file_create() called -------------");

    //speicher allocieren
    memvol_file_t* file = (memvol_file_t *)malloc(sizeof(memvol_file_t));
    memvol_object_t* object = (memvol_object_t *)malloc(sizeof(memvol_object_t));

    file->root_group = (memvol_group_t *)malloc(sizeof(memvol_group_t));
    file->name = (char *)malloc(strlen(name) + 1);
    file->root_group->name = (char *)malloc(strlen("/") + 1);

    object->type = (memvol_object_type)malloc(sizeof(memvol_object_type));
    object->subclass = (memvol_group_t *)malloc(sizeof(memvol_group_t));

    //werte initialisieren
    strcpy(file->name, name);

    file->root_group->children = g_hash_table_new(g_str_hash, g_str_equal);
    strcpy(file->root_group->name, "/");

    object->type = GROUP_T;
    object->subclass = (memvol_group_t *)file->root_group;

    //g_hash_table_insert(file->root_group->children, strdup("/"), object);

    if (file_table == NULL) {
        file_table = g_hash_table_new(g_str_hash, g_str_equal);
    }

    g_hash_table_insert(file_table, strdup(name), object);

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

    memvol_object_t* ret = g_hash_table_lookup(file_table, name);

    if (ret == NULL) {
        puts("File existiert nicht!");

    } else {
        memvol_file_t* f = (memvol_file_t *)ret->subclass;
        printf("Datei %s (%p) geoeffnet.\n", name, (void*)f);

    }

    puts("------------------------------------------------------");
    puts("");

    return (void *)ret;
}

static herr_t memvol_file_get(void *file, H5VL_file_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
    puts("------------ memvol_file_get() called ----------------");

    memvol_file_t *f = (memvol_file_t *)((memvol_object_t *)file)->subclass;
    herr_t ret = 0;

    //get_type => H5VL_file_get_t -> vol/src/H5VLpublic.h
    switch (get_type) {
        case H5VL_FILE_GET_FAPL: {
                printf("H5Fget_access_plist %p\n", va_arg(arguments, void*));
                ret = 0;
                break;
            }
        case H5VL_FILE_GET_FCPL: {
                printf("H5Fget_create_plist %p\n", va_arg(arguments, void*));
                ret = 0;
                break;
            }
        case H5VL_FILE_GET_INTENT: {
                printf("H5Fget_intent %p\n", va_arg(arguments, void*));
                ret = 0;
                break;
            }
        case H5VL_FILE_GET_NAME: {
                printf("H5Fget_name %p\n", va_arg(arguments, void*));
                ret = 0;
                break;
            }
        case H5VL_FILE_GET_OBJ_COUNT: {
                printf("H5Fget_obj_count %p\n", va_arg(arguments, void*));
                ret = 0;
                break;
            }
        case H5VL_FILE_GET_OBJ_IDS: {
                printf("H5Fget_obj_ids %p\n", va_arg(arguments, void*));
                ret = 0;
                break;
            }
        case H5VL_OBJECT_GET_FILE: {
                printf("H5VL_OBJECT_GET_FILE %p\n", va_arg(arguments, void*));
                ret = 0;
                break;
            }
        default: {
                puts("ERROR");
                ret = -1;
            }
    }

    puts("------------------------------------------------------");
    puts("");

    return ret;
}

static herr_t memvol_file_close(void *obj, hid_t dxpl_id, void **req)
{
    puts("------------ memvol_file_close() called --------------");

    memvol_object_t* object = (memvol_object_t *)obj;

    if (object->type == GROUP_T) {
        memvol_group_t* f = (memvol_group_t *)object->subclass;

        printf("Root-Group-Pointer %p\n", (void*)f);

        free(f->name);
        //g_hash_table_destroy(f->children);
        free(f);

        f->name = NULL;
        f->children = NULL;
        f = NULL;

        puts("------------------------------------------------------");
        puts("");
        
        return 1;
        
    } else {
        puts("ERROR: Kein Root-Group Pointer!");

        puts("------------------------------------------------------");
        puts("");

        return -1;

    }
}
