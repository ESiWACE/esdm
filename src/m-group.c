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


static void* memvol_group_create(void* obj, H5VL_loc_params_t loc_params, const char* name, hid_t gcpl_id, hid_t gapl_id, hid_t dxpl_id, void** req)
{
    puts("------------ memvol_group_create() called ------------");

    //speicher allocieren
    memvol_group_t* group = (memvol_group_t *)malloc(sizeof(memvol_group_t));
    memvol_object_t* object = (memvol_object_t *)malloc(sizeof(memvol_object_t));

    group->name = (char *)malloc(strlen(name));
    group->children = (GHashTable*)malloc(sizeof(GHashTable*));

    object->type = (memvol_object_type)malloc(sizeof(memvol_object_type));
    object->subclass = (memvol_group_t *)malloc(sizeof(memvol_group_t));

    //werte initialisieren
    memvol_group_t* parent_group;
    memvol_object_t* o = (memvol_object_t *)obj;
    if (o->type == GROUP_T) {
        parent_group = (memvol_group_t *)o->subclass;

    } else {
        return (void *)0;

    }

    object->type = GROUP_T;
    object->subclass = group;

    strcpy(group->name, name);
    group->children = g_hash_table_new(g_str_hash, g_str_equal);

    g_hash_table_insert(parent_group->children, strdup(name), object);

    //debug ausgaben
    printf("Gruppe %s (%p) erstellt!\n",group->name, (void *)group);
    printf("Parent-Group: %s\n", parent_group->name);

    GList* children_keys = g_hash_table_get_values(parent_group->children);

    printf("ls ../ : ");
    while (children_keys != NULL) {
        memvol_object_t* obj = (memvol_object_t *)children_keys->data;
        if (obj->type == GROUP_T) {
            printf("%s  ", ((memvol_group_t *)obj->subclass)->name);

        }
        children_keys = children_keys->next;
    }
    g_list_free(children_keys);
    printf("\n");

    puts("------------------------------------------------------");
    puts("");

    return (void *)object;
}

static void * memvol_group_open(void* object, H5VL_loc_params_t loc_params, const char* name, hid_t gapl_id, hid_t dxpl_id, void **req)
{
    puts("------------ memvol_group_open() called --------------");

    memvol_object_t* obj = (memvol_object_t *)object;
    memvol_group_t* parent = (memvol_group_t *)obj->subclass;

    memvol_object_t* ret = g_hash_table_lookup(parent->children, name);

    //debug asugaben
    if (ret == NULL) {
        puts("Gruppe nicht im angegebenen Parent gefunden!");

    } else {
        memvol_group_t* found = (memvol_group_t *)ret->subclass;
        printf("Group %s (%p) im Parent %s (%p) geoeffnet\n", (char*)found->name, (void*)found, parent->name, (void*) parent);

    }

    puts("------------------------------------------------------");
    puts("");

    return (void *)ret;
}

static herr_t memvol_group_close(void* grp, hid_t dxpl_id, void** req) {

    puts("memvol_group_close() called!");

    memvol_group_t* g = (memvol_group_t *)grp;
    free(g->name);
    //g_free(g->children); /* SEG FAULT */
    free(g);

    g->name = NULL;
    g->children = NULL;
    g = NULL;

    return 1;
}

static herr_t memvol_group_get(void *group, H5VL_group_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
    puts("------------ memvol_group_get() called ---------------");

    memvol_group_t *g = (memvol_group_t *)((memvol_object_t *)group)->subclass;
    herr_t ret = 0;

    //get_type entweder 'group creation property list' oder 'group info'
    //--> vol/src/H5VLpublic.h
    if (get_type == H5VL_GROUP_GET_GCPL) {
        printf("H5Gget_create_plist %p\n", va_arg(arguments, void*));
        ret = 0;

    } else if (get_type == H5VL_GROUP_GET_INFO) {
        printf("H5Gget_info %p\n", va_arg(arguments, void*));
        ret = 0;

    } else {
        puts("ERROR");
        ret =  -1;
    }

    puts("------------------------------------------------------");
    puts("");

    return ret;
}
