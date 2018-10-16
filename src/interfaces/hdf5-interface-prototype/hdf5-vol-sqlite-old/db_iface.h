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

//#define REPEAT(...) \
//{ \
//	while (0 != __VA_ARGS__) { \
//		DEBUGMSG("Waiting"); \
//		sleep(1); \
//	} \
//} 


//#define REPEAT_UNTIL_OK(...) \
//__VA_ARGS__;
//{ \
//	int _rc; \
//	if (SQLITE_OK != (_rc = __VA_ARGS__)) { \
//		DEBUGMSG("Waiting: %s; error code %d\n", sqlite3_errmsg(db), _rc); \
//		sleep(1); \
//	} \
//} 

#endif


#include "debug.h"
#include "h5_sqlite_plugin.h"


/* Database */
void DB_connect(const char* db_fn, void** db);
void DB_disconnect(void* db);
int DB_entry_exists(SQO_t* obj, const char* table, const char* name, int* exists);

/* FILE */
int DBF_create(SQF_t* file, unsigned flags, void* conn, hid_t fcpl_id, hid_t fapl_id);
int DBF_get_fapl(SQF_t* file, hid_t* obj);
int DBF_get_fcpl(SQF_t* file, hid_t* obj);

/* ATTRIBUTE  */
int DBA_create(SQA_t* attr, H5VL_loc_params_t loc_params, hid_t acpl_id, hid_t aapl_id, hid_t dxpl_id);
int DBA_open(SQO_t* obj, H5VL_loc_params_t loc_params, const char* attr_name, SQA_t* ret_obj);
int DBA_open_by_idx(SQO_t* obj, H5VL_loc_params_t loc_params, const unsigned int idx, SQA_t* ret_obj);
int DBA_get_acpl(SQA_t* attr, hid_t* ret_id);
int DBA_get_type(SQA_t* attr, hid_t* ret_id);
int DBA_get_space(SQA_t* attr, hid_t* ret_id);
int DBA_write(SQA_t* attr, const void *buf);
int DBA_read(SQA_t* attr,  void *buf);
int DBA_get_info(SQO_t* obj, const char* attr_name, H5A_info_t* ainfo);
int DB_create_name_list(SQO_t* obj, H5VL_loc_params_t loc_params, const char* tab_name, char*** attr_list, size_t* attr_list_size);
int DB_destroy_name_list(char** list, size_t size);

/* GROUP */
int DBG_create(SQG_t* group, H5VL_loc_params_t loc_params, hid_t gcpl_id, hid_t gapl_id, hid_t gxpl_id);
int DBG_open(SQO_t* parent, H5VL_loc_params_t loc_params, const char* group_name, SQG_t* ret_obj);
int DBG_get_gcpl(SQG_t* group, hid_t* plist);

/* DATASET */
int DBD_create(SQD_t* dset, H5VL_loc_params_t loc_params, hid_t gcpl_id, hid_t gapl_id, hid_t gxpl_id);
int DBD_open(SQO_t* parent, H5VL_loc_params_t loc_params, const char *name, SQD_t* ret_obj);
//void DBD_get(const char* path, hid_t* type_id, hid_t* space_id, void* conn);
int DBD_get_type(SQD_t* dset, hid_t* type_id);
int DBD_get_space(SQD_t* dset, hid_t* space_id);
int DBD_get_dcpl(SQD_t* dset, hid_t* plist);
int DBD_get_dapl(SQD_t* dset, hid_t* plist);

/* OBJECT  */
hid_t DBO_open(SQO_t* obj, H5VL_loc_params_t loc_params); 

/* LINK */

#endif   /* ----- #ifndef db_iface_INC  ----- */
