/*
 * =====================================================================================
 *
 *       Filename:  db_iface.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/19/2017 08:51:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */


#ifndef  db_iface_INC
#define  db_iface_INC

#include <hdf5.h>
#ifdef PGSQL
#include <libpq-fe.h>
#else
#include <sqlite3.h>
#endif

#include "debug.h"
#include "h5_sqlite_plugin.h"


/* Database */
void DB_connect(const char* db_fn, void** db);
void DB_disconnect(void* db);
int DB_entry_exists(SQO_t* obj, const char* table, const char* name);

/* FILE */
void DBF_create(SQF_t* file, unsigned flags, void* conn, hid_t fcpl_id, hid_t fapl_id);
void DBF_get_fapl(SQF_t* file, hid_t* obj);
void DBF_get_fcpl(SQF_t* file, hid_t* obj);

/* ATTRIBUTE  */
void DBA_create(SQA_t* attr, H5VL_loc_params_t loc_params, hid_t acpl_id, hid_t aapl_id, hid_t dxpl_id);
SQA_t* DBA_open(SQO_t* obj, H5VL_loc_params_t loc_params, const char* attr_name);
SQA_t* DBA_open_by_idx(SQO_t* obj, H5VL_loc_params_t loc_params, const unsigned int idx);
void DBA_get_acpl(SQA_t* attr, hid_t* ret_id);
void DBA_get_type(SQA_t* attr, hid_t* ret_id);
void DBA_get_space(SQA_t* attr, hid_t* ret_id);
void DBA_write( const char *location, const void *data, int size, void* conn);
void DBA_read( const char *location, void *buf, void* conn);
void DBA_get_info(SQO_t* obj, const char* attr_name, H5A_info_t* ainfo);
void DB_create_name_list(SQO_t* obj, H5VL_loc_params_t loc_params, const char* tab_name, char*** attr_list, size_t* attr_list_size);
void DB_destroy_name_list(char** list, size_t size);

/* GROUP */
void DBG_create(SQG_t* group, H5VL_loc_params_t loc_params, hid_t gcpl_id, hid_t gapl_id, hid_t gxpl_id);
SQG_t* DBG_open(SQO_t* parent, H5VL_loc_params_t loc_params, const char* group_name);
void DBG_get_gcpl(SQG_t* group, hid_t* plist);

/* DATASET */
void DBD_create(SQD_t* dset, H5VL_loc_params_t loc_params, hid_t gcpl_id, hid_t gapl_id, hid_t gxpl_id);
SQD_t* DBD_open(SQO_t* parent, H5VL_loc_params_t loc_params, const char *name);
//void DBD_get(const char* path, hid_t* type_id, hid_t* space_id, void* conn);
void DBD_get_type(SQD_t* dset, hid_t* type_id);
void DBD_get_space(SQD_t* dset, hid_t* space_id);
void DBD_get_dcpl(SQD_t* dset, hid_t* plist);
void DBD_get_dapl(SQD_t* dset, hid_t* plist);

/* OBJECT  */
hid_t DBO_open(SQO_t* obj, H5VL_loc_params_t loc_params); 

/* LINK */

#endif   /* ----- #ifndef db_iface_INC  ----- */
