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
    printf("function memvol_group_create called!\n");

    memvol_group_t* grp;
    grp = (memvol_group_t *)calloc(1, sizeof(memvol_group_t));//

    grp->name = (char*)malloc(strlen(name));//
    grp->children = (GHashTable*)malloc(sizeof(GHashTable*));//
    strcpy(grp->name, name);//
    GHashTable* table = g_hash_table_new(g_str_hash, g_str_equal);//
    size_t size = sizeof(GHashTable*);
    memcpy(grp->children, table, size);//
    
    printf("%s\n", grp->name);//
    printf("GHashTable size: %d\n", g_hash_table_size(grp->children));
    // TODO
    // Objekte richtig einfuegen (g_hash_table_lookup_node Error ?)
    // Erstellen und suchen/oeffnen von groups ueber ihren Namen
    //char* testing = "testing";
    //g_hash_table_insert(grp->children, "dataset1", &testing);
    //printf("GHashTable size: %d\n", g_hash_table_size(grp->children));
    //printf("GHashTable content: \'%s\'\n", g_hash_table_lookup(grp->children, testing));

    return (void*)grp;//
}

static herr_t memvol_group_close(void* grp, hid_t dxpl_id, void** req) {

    printf("function memvol_group_close called!\n");

    memvol_group_t *g = (memvol_group_t*)grp;//
    free(g->name);//
    free(g->children);//
    free(g);//

    g->name = NULL;//
    g->children = NULL;//
    g = NULL;//

    //printf("CALL: %s\n", __PRETTY_FUNCTION__);
    return 1;
}

static void* memvol_group_get(void *group, H5VL_group_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
    memvol_group_t *g = (memvol_group_t *)group;
    return (void*)g->children;
}
