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

memvol_group_t* g_group;//

static void* memvol_group_create(void* obj, H5VL_loc_params_t loc_params, const char* name, hid_t gcpl_id, hid_t gapl_id, hid_t dxpl_id, void** req)
{
    g_group = (memvol_group_t *)calloc(1, sizeof(memvol_group_t));//

    g_group->name = (char*)malloc(strlen(name));//
    g_group->children = (int*)malloc(sizeof(int));//
    strcpy(g_group->name, name);//
    
    printf("%s\n", name);//

    //printf("CALL: %s\n", __PRETTY_FUNCTION__);
    //void* ret = malloc(10);
    return (void*)g_group;//
}

static herr_t memvol_group_close(void* grp, hid_t dxpl_id, void** req) {

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

static herr_t memvol_group_get(void *group, H5VL_group_get_t get_type, hid_t dxpl_id, void **req, va_list arguments)
{
    memvol_group_t *g = (memvol_group_t *)group;
    return (herr_t)g->children;
}
