/*
 * =====================================================================================
 *
 *       Filename:  db-sqlite.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/19/2017 08:46:14 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */


#include <unistd.h>
#include <sqlite3.h>
#include <string.h>
#include <hdf5.h>

#include "db_iface.h"
#include "base.h"

char *err_msg = 0;  /* pointer to an error string */



void DB_connect(const char* db_fn, void** db_ptr) {
	TRACEMSG("");
	sqlite3** db = (sqlite3**) db_ptr;
	int rc = sqlite3_open(db_fn, db);
	if (rc != SQLITE_OK) {
		while (rc != SQLITE_OK) {
			rc = sqlite3_open(db_fn, db);
			DEBUGMSG("Cannot open database: %s, errcode %d", sqlite3_errmsg(*db), rc);
			sleep(1);
			//			ERRORMSG("Cannot open database: %s", sqlite3_errmsg(db));
		}
		//			sqlite3_close(db);
	}
//	sqlite3_busy_timeout(*db, 10000);
}



void DB_disconnect(void* db_ptr) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) db_ptr;
	sqlite3_close(db);
	db = NULL;
}



//typedef struct {
//    hbool_t             corder_valid;   /* Indicate if creation order is valid */
//    H5O_msg_crt_idx_t   corder;         /* Creation order                 */
//    H5T_cset_t          cset;           /* Character set of attribute name */
//    hsize_t             data_size;      /* Size of raw data		  */
//} H5A_info_t;
void DBA_get_info (SQO_t* obj, const char* attr_name, H5A_info_t* ainfo) {
  ainfo->corder_valid = false;
  ainfo->corder = 0;
  ainfo->cset = H5T_CSET_ASCII; // possible: H5T_CSET_ASCII, H5T_CSET_UTF8
  ainfo->data_size = 100;
}



void DBF_create(SQF_t* file, unsigned flags, void* db_ptr, hid_t fcpl_id, hid_t fapl_id) {
	sqlite3* db = (sqlite3*) db_ptr;
	char *sql = "CREATE TABLE FILE("
	"path TEXT, "
	"name TEXT, "
	"flags INTEGER,"
	"fcpl BLOB, "
	"fapl BLOB, "
	"PRIMARY KEY(path, name ASC));";

	char *sql3 = "CREATE TABLE ATTRIBUTES ("
	"path TEXT,"
	"name TEXT, "
	"type BLOB, "
	"space BLOB, "
	"acpl BLOB, "
//	"aapl BLOB, "
//	"dxpl BLOB, "
//	"data BLOB, "
	"data_size INTEGER, "
	"PRIMARY KEY(path, name ASC));";

	char *sql4 = "CREATE TABLE GROUPS ("
	"path TEXT,"
	"name TEXT,"
	"gcpl BLOB, "
	"gapl BLOB, "
	"info BLOB, "
//	"dxpl BLOB, "
	"PRIMARY KEY(path, name ASC));";

	char *sql5 = "CREATE TABLE DATASETS ("
	"path TEXT,"
	"name TEXT,"
	"type BLOB, "
	"space BLOB, "
	"dcpl BLOB, "
	"dapl BLOB, "
	"info BLOB, "
	"offset INTEGER, "
	"data_size INTEGER, "
	"PRIMARY KEY(path, name ASC));";

	char *sql6 = "CREATE TABLE ATTR_DATA("
	"Path TEXT, "
	"Data BLOB, "
	"PRIMARY KEY(Path ASC));";

	char *sql2 =  "INSERT INTO FILE VALUES(?,?,?,?,?);";
	sqlite3_stmt *res;
	const char *pzTest;
	int rc;

	rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	if (rc != SQLITE_OK ) {
		ERRORMSG("SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);
	}
  rc = sqlite3_exec(db, sql3, 0, 0, &err_msg);
	if (rc != SQLITE_OK ) {
		ERRORMSG("SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);
	}
  rc = sqlite3_exec(db, sql4, 0, 0, &err_msg);
	if (rc != SQLITE_OK ) {
		ERRORMSG("SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);
	}
  rc = sqlite3_exec(db, sql5, 0, 0, &err_msg);
	if (rc != SQLITE_OK ) {
		ERRORMSG("SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);
	}
  rc = sqlite3_exec(db, sql6, 0, 0, &err_msg);
	if (rc != SQLITE_OK ) {
		ERRORMSG("SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(db);
	}
	rc = sqlite3_prepare(db, sql2, strlen(sql2), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	size_t fapl_size;
	H5Pencode(fapl_id, NULL, &fapl_size);
	char fapl_buf[fapl_size];
	H5Pencode(fapl_id, fapl_buf, &fapl_size);

	size_t fcpl_size;
	H5Pencode(fcpl_id, NULL, &fcpl_size);
	char fcpl_buf[fcpl_size];
	H5Pencode(fcpl_id, fcpl_buf, &fcpl_size);

	sqlite3_bind_text(res, 1, file->object.location, strlen(file->object.location), 0);
	sqlite3_bind_text(res, 2, file->object.name, strlen(file->object.name), 0);
	sqlite3_bind_int64(res, 3, flags);
	sqlite3_bind_blob(res, 4, fcpl_buf, fcpl_size, NULL);
	sqlite3_bind_blob(res, 5, fapl_buf, fapl_size, NULL);
	rc = sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}
	sqlite3_finalize(res);
	sqlite3_db_cacheflush(db);
}



void DBF_get_fapl(SQF_t* file, hid_t* plist) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) file->object.root->db;
	sqlite3_stmt *res;
	const char *pzTest;

	const char* sql = "SELECT fapl FROM FILE WHERE path = ? AND name = ?;";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	sqlite3_bind_text(res, 1, file->object.location, strlen(file->object.location), 0);
	sqlite3_bind_text(res, 2, file->object.name, strlen(file->object.name), 0);
	rc=sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	int data_size = sqlite3_column_bytes(res, 0);
	unsigned char* buf = malloc(data_size);
	const void * data =  sqlite3_column_blob(res, 0);
	memcpy(buf, data, data_size);
	*plist = H5Pdecode(buf);

	sqlite3_finalize(res);
}



void DBF_get_fcpl(SQF_t* file, hid_t* plist) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) file->object.root->db;
	sqlite3_stmt *res;
	const char *pzTest;

	const char* sql = "SELECT fcpl FROM FILE WHERE path = ? AND name = ?;";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	sqlite3_bind_text(res, 1, file->object.location, strlen(file->object.location), 0);
	sqlite3_bind_text(res, 2, file->object.name, strlen(file->object.name), 0);
	rc=sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	int data_size = sqlite3_column_bytes(res, 0);
	unsigned char* buf = malloc(data_size);
	const void * data =  sqlite3_column_blob(res, 0);
	memcpy(buf, data, data_size);
	*plist = H5Pdecode(buf);

	sqlite3_finalize(res);
}



int DB_entry_exists(SQO_t* obj, const char* table, const char* name) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) obj->root->db;
	sqlite3_stmt *res;
	int ret = 0;
	const char* sql_query_template =  "SELECT 1 FROM %s WHERE path = ? AND name = ?;";
	char* sql_query = malloc(strlen(table) + strlen(sql_query_template) + 1);
	sprintf(sql_query, sql_query_template, table);

	const char *pzTest;
	int rc;
	rc = sqlite3_prepare(db, sql_query, strlen(sql_query), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	} 

	char* path = create_path(obj);
	sqlite3_bind_text(res, 1, path, strlen(path), 0);
	sqlite3_bind_text(res, 2, name, strlen(name), 0);

	if (sqlite3_step(res) == SQLITE_ROW) {
		// record found
		rc = 1;
	}
	else{
		// no record found
		rc = 0;
	}

//	DEBUGMSG("%s -> result %d", sql_query, rc);
	free(sql_query);
	destroy_path(path);

	sqlite3_finalize(res);
	return rc;
}



void DB_create_name_list(SQO_t* parent, H5VL_loc_params_t loc_params, const char* tab_name, char*** attr_list, size_t* attr_list_size) {
      
	sqlite3* db = (sqlite3*) parent->root->db;
	sqlite3_stmt *res;
	int ret = 0;
	const char *pzTest;
	int rc;

	char* sql_query_count_template =  "SELECT count(*) FROM %s WHERE path = ?;";
	char* sql_query_count = (char*) malloc(strlen(sql_query_count_template) + strlen(tab_name) + 1);
	sprintf(sql_query_count, sql_query_count_template, tab_name);

	rc = sqlite3_prepare(db, sql_query_count, strlen(sql_query_count), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	} 

	char* path = create_path(parent);
//	sqlite3_bind_text(res, 1, tab_name, strlen(tab_name), 0);
	sqlite3_bind_text(res, 1, path, strlen(path), 0);

	if (SQLITE_ROW == (rc = sqlite3_step(res))) {
		*attr_list_size = sqlite3_column_int64(res, 0);
	}
	else if (SQLITE_BUSY == rc) {
		ERRORMSG("Busy");
	}

	*attr_list = (char**) malloc(sizeof(**attr_list) * *attr_list_size); 
	sqlite3_finalize(res);

	sqlite3_stmt *res2;
	const char *pzTest2;
	int rc2;

	char* sql_query_template =  "SELECT name FROM %s WHERE path = ?;";
	char* sql_query = (char*) malloc(strlen(sql_query_template) + strlen(tab_name) + 1);
	sprintf(sql_query, sql_query_template, tab_name);

	rc2 = sqlite3_prepare(db, sql_query, strlen(sql_query), &res, &pzTest2);
	if (rc2 != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	} 

//	sqlite3_bind_text(res, 1, tab_name, strlen(tab_name), 0);
	sqlite3_bind_text(res, 1, path, strlen(path), 0);

	for (size_t i = 0; i < *attr_list_size; ++i) {
		rc = sqlite3_step(res);
		if (rc == SQLITE_BUSY) {
			ERRORMSG("Busy");
		}
		(*attr_list)[i] = strdup(sqlite3_column_text(res, 0));
	}
	sqlite3_finalize(res);
	destroy_path(path);
}



void DB_destroy_name_list(char** list, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		free(list[i]);
		list[i] = NULL;
	}
	free(list);
	list = NULL;
}



void DBA_create(SQA_t* attr, H5VL_loc_params_t loc_params, hid_t acpl_id, hid_t aapl_id, hid_t dxpl_id)
{
	TRACEMSG("");
	sqlite3* db = (sqlite3*) attr->object.root->db;

	char *sql2=  "INSERT INTO ATTRIBUTES VALUES(?,?,?,?,?,?);";

	sqlite3_stmt *res;
	const char *pzTest;
	int rc;

	if (H5P_DEFAULT == acpl_id) {
		acpl_id = H5Pcreate(H5P_ATTRIBUTE_CREATE);
	}

	hid_t type_id;
	H5Pget(acpl_id, "attr_type_id", &type_id);
	assert(-1 != type_id);
	size_t type_size;
	H5Tencode(type_id, NULL, &type_size);
	char* type_buf = (char*) malloc(type_size);
	H5Tencode(type_id, type_buf, &type_size);

	hid_t space_id;
	H5Pget(acpl_id, "attr_space_id", &space_id);
	assert(-1 != space_id);
	size_t space_size;
	H5Sencode(space_id, NULL, &space_size);
	unsigned char* space_buf = (unsigned char*) malloc(space_size);
	H5Sencode(space_id, space_buf, &space_size);

	size_t acpl_size;
	H5Pencode(acpl_id, NULL, &acpl_size);
	char* acpl_buf = (char*) malloc(acpl_size);
	assert(NULL != acpl_buf);
	H5Pencode(acpl_id, acpl_buf, &acpl_size);

//	size_t aapl_size;
//	H5Pencode(aapl_id, NULL, &aapl_size);
//	char* aapl_buf = (char*) malloc(aapl_size);
//	assert(NULL != aapl_buf);
//	H5Pencode(aapl_id, aapl_buf, &aapl_size);
//
//	size_t dxpl_size;
//	H5Pencode(dxpl_id, NULL, &dxpl_size);
//	char* dxpl_buf = (char*) malloc(dxpl_size);
//	assert(NULL != dxpl_buf);
//	H5Pencode(dxpl_id, dxpl_buf, &dxpl_size);

	rc = sqlite3_prepare(db, sql2, strlen(sql2), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s", sqlite3_errmsg(db));
	}

	sqlite3_bind_text(res, 1, attr->object.location, strlen(attr->object.location), 0);
	sqlite3_bind_text(res, 2, attr->object.name, strlen(attr->object.name), 0);
	sqlite3_bind_blob(res, 3, type_buf, type_size, NULL);
	sqlite3_bind_blob(res, 4, space_buf, space_size, NULL);
	sqlite3_bind_blob(res, 5, acpl_buf, acpl_size, NULL);
	sqlite3_bind_int64(res, 6, attr->data_size);
//	sqlite3_bind_blob(res, 7, aapl_buf, aapl_size, NULL);
//	sqlite3_bind_blob(res, 8, dxpl_buf, dxpl_size, NULL);

	rc = sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	sqlite3_finalize(res);
	free(space_buf);
	free(type_buf);
	free(acpl_buf);
	sqlite3_db_cacheflush(db);
}



SQA_t* DBA_open(SQO_t* parent, H5VL_loc_params_t loc_params, const char* attr_name) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) parent->root->db;
	sqlite3_stmt *res;

	const char *pzTest;
	int rc;
	char *sql=  "SELECT data_size FROM ATTRIBUTES WHERE path = ? AND name = ?;";

	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	SQA_t* attr = (SQA_t *) malloc(sizeof(*attr));
	attr->object.location = create_path(parent);
	attr->object.name = strdup(attr_name);
	attr->object.fapl = parent->fapl;
	attr->object.root = parent->root;

	sqlite3_bind_text(res, 1, attr->object.location, strlen(attr->object.location), 0);
	sqlite3_bind_text(res, 2, attr->object.name, strlen(attr->object.name), 0);
	rc = sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	size_t data_size = sqlite3_column_int64(res, 0);
	sqlite3_finalize(res);

	attr->data_size = data_size;
	return attr;
}



SQA_t* DBA_open_by_idx(SQO_t* obj, H5VL_loc_params_t loc_params, const unsigned int idx) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) obj->root->db;

	char** attr_list = NULL;
	size_t attr_list_size = 0;
	DB_create_name_list(obj, loc_params, "ATTRIBUTES", &attr_list, &attr_list_size);
	const unsigned int test =  attr_list_size;
	if ((idx + 1) > test) {
//		return NULL;
		for (size_t i = 0; i < attr_list_size; ++i) {
			printf("attribute found: %s + %s -> %s\n", obj->location, obj->name, attr_list[i]);
		}
		ERRORMSG("Found %zu attributes, but at least %zu were expected.", attr_list_size, idx + 1);
	}

	/* --- */

	sqlite3_stmt *res;
	const char *pzTest;

	char *sql=  "SELECT data_size FROM ATTRIBUTES WHERE path = ? AND name = ?;";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	char* path = create_path(obj);
	sqlite3_bind_text(res, 1, path, strlen(path), 0);
	sqlite3_bind_text(res, 2, attr_list[idx], strlen(attr_list[idx]), 0);
	rc = sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	size_t data_size = sqlite3_column_int64(res, 0);
	sqlite3_finalize(res);

	SQA_t* attr = NULL;
	if (attr_list_size > 0) {
		attr = (SQA_t *) malloc(sizeof(*attr));
		attr->object.location = create_path(obj);
		attr->object.name = strdup(attr_list[idx]);
		attr->data_size = data_size;
		attr->object.fapl = obj->fapl;
		attr->object.root = obj->root;
	}
	else {
		ERRORMSG("Found %zu attributes, but expected more than 0.", attr_list_size);
	}
	destroy_path(path);
	DB_destroy_name_list(attr_list, attr_list_size);
	return attr;
}



void DBA_get_acpl(SQA_t* attr, hid_t* acpl_id) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) attr->object.root->db;
	sqlite3_stmt *res;
	const char *pzTest;

	char *sql=  "SELECT acpl FROM ATTRIBUTES WHERE path = ? AND name = ?;";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	sqlite3_bind_text(res, 1, attr->object.location, strlen(attr->object.location), 0);
	sqlite3_bind_text(res, 2, attr->object.name, strlen(attr->object.name), 0);

	rc=sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	int data_size = sqlite3_column_bytes(res, 0);
	if (0 == data_size) {
		ERRORMSG("Couldn't read acpl.");
	}
	unsigned char* acpl_buf = malloc(data_size);
	const void * data =  sqlite3_column_blob(res, 0);
	memcpy(acpl_buf, data, data_size);
	*acpl_id = H5Pdecode(acpl_buf);
	assert(-1 != *acpl_id);

	sqlite3_finalize(res);
}



void DBA_get_type(SQA_t* attr, hid_t* type_id) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) attr->object.root->db;
	sqlite3_stmt *res;
	const char *pzTest;

	char *sql=  "SELECT type FROM ATTRIBUTES WHERE path = ? AND name = ?;";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	sqlite3_bind_text(res, 1, attr->object.location, strlen(attr->object.location), 0);
	sqlite3_bind_text(res, 2, attr->object.name, strlen(attr->object.name), 0);

	rc=sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	int data_size = sqlite3_column_bytes(res, 0);
	if (0 == data_size) {
		ERRORMSG("Couldn't read type.");
	}
	unsigned char* type_buf = malloc(data_size);
	const void * data =  sqlite3_column_blob(res, 0);
	memcpy(type_buf, data, data_size);
	*type_id = H5Tdecode(type_buf);
	assert(-1 != *type_id);

	sqlite3_finalize(res);
}



void DBA_get_space(SQA_t* attr, hid_t* space_id) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) attr->object.root->db;
	sqlite3_stmt *res;
	const char *pzTest;

	char *sql=  "SELECT space FROM ATTRIBUTES WHERE path = ? AND name = ?;";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	sqlite3_bind_text(res, 1, attr->object.location, strlen(attr->object.location), 0);
	sqlite3_bind_text(res, 2, attr->object.name, strlen(attr->object.name), 0);

	rc=sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	int data_size = sqlite3_column_bytes(res, 0);
	if (0 == data_size) {
		ERRORMSG("Couldn't read space.");
	}
	unsigned char* space_buf = malloc(data_size);
	const void * data =  sqlite3_column_blob(res, 0);
	memcpy(space_buf, data, data_size);
	*space_id = H5Sdecode(space_buf);
	assert(-1 != *space_id);

	sqlite3_finalize(res);
}



void DBA_write( const char *location, const void *data, int size, void* db_ptr) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) db_ptr;

	char *sql2=  "INSERT INTO ATTR_DATA VALUES(?,?);";
	sqlite3_stmt *res;
	int rc;

	rc = sqlite3_prepare(db, sql2, -1, &res, 0);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
	}
	sqlite3_bind_text(res, 1, location, strlen(location), 0);
	sqlite3_bind_blob(res, 2, data, size, SQLITE_STATIC);
	rc = sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}
	sqlite3_finalize(res);
}



void DBA_read( const char *location, /*OUT*/void *buf, void* db_ptr) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) db_ptr;
	char *sql = "SELECT data FROM ATTR_DATA WHERE Path=?;";
	sqlite3_stmt *res;
	int rc;

	rc = sqlite3_prepare(db, sql, -1, &res, 0);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_bind_text(res, 1, location, strlen(location), 0);
	rc = sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}
	int data_size = sqlite3_column_bytes(res, 0);
	const void * data =  sqlite3_column_blob(res, 0);
	memcpy(buf, data, data_size);

	sqlite3_finalize(res);
}



void DBG_create(SQG_t* group, H5VL_loc_params_t loc_params, hid_t gcpl_id, hid_t gapl_id, hid_t gxpl_id) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) group->object.root->db;

	char *sql2=  "INSERT INTO GROUPS VALUES(?,?,?,?,?);";

	sqlite3_stmt *res;
	const char *pzTest;
	int rc;

	rc = sqlite3_prepare(db, sql2, strlen(sql2), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
	}

	size_t gcpl_size;
	H5Pencode(gcpl_id, NULL, &gcpl_size);
	char gcpl_buf[gcpl_size];
	H5Pencode(gcpl_id, gcpl_buf, &gcpl_size);

	size_t gapl_size;
	H5Pencode(gapl_id, NULL, &gapl_size);
	char gapl_buf[gapl_size];
	H5Pencode(gapl_id, gapl_buf, &gapl_size);

	sqlite3_bind_text(res, 1, group->object.location, strlen(group->object.location), 0);
	sqlite3_bind_text(res, 2, group->object.name, strlen(group->object.name), 0);
//	sqlite3_bind_text(res, 2, obj_name, strlen(obj_name), 0);
//	sqlite3_bind_text(res, 3, obj_type, strlen(obj_type), 0);
	sqlite3_bind_blob(res, 3, gcpl_buf, gcpl_size, NULL);
	sqlite3_bind_blob(res, 4, gapl_buf, gapl_size, NULL);
	sqlite3_bind_blob(res, 5, &group->object.info, sizeof(group->object.info), NULL);

	rc = sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}
	sqlite3_finalize(res);
	sqlite3_db_cacheflush(db);
}



SQG_t* DBG_open(SQO_t* parent, H5VL_loc_params_t loc_params, const char* group_name) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) parent->root->db;
	sqlite3_stmt *res;

	const char *pzTest;
	int rc;
	char *sql=  "SELECT info FROM GROUPS WHERE path = ? AND name = ?;";

	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	SQG_t* group = (SQG_t *) malloc(sizeof(*group));
	group->object.location = create_path(parent);
	group->object.name = strdup(group_name);
	group->object.fapl = parent->fapl;
	group->object.root = parent->root;

	sqlite3_bind_text(res, 1, group->object.location, strlen(group->object.location), 0);
	sqlite3_bind_text(res, 2, group->object.name, strlen(group->object.name), 0);
	rc = sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	int data_size = sqlite3_column_bytes(res, 0);
	if (0 == data_size) {
		ERRORMSG("Couldn't read info.");
	}
	assert(data_size == sizeof(group->object.info));
	const void * data =  sqlite3_column_blob(res, 0);
	memcpy(&group->object.info, data, data_size);

	sqlite3_finalize(res);

	return group;
}



void DBG_get_gcpl(SQG_t* group, hid_t* plist) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) group->object.root->db;
	sqlite3_stmt *res;
	const char *pzTest;

	const char* sql = "SELECT gcpl FROM GROUPS WHERE path = ? AND name = ?;";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	sqlite3_bind_text(res, 1, group->object.location, strlen(group->object.location), 0);
	sqlite3_bind_text(res, 2, group->object.name, strlen(group->object.name), 0);
	rc=sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	int data_size = sqlite3_column_bytes(res, 0);
	unsigned char* buf = malloc(data_size);
	const void * data =  sqlite3_column_blob(res, 0);
	memcpy(buf, data, data_size);
	*plist = H5Pdecode(buf);

	sqlite3_finalize(res);
}



static int busy_handler (void* parm, int n) {
	DEBUGMSG("Handler activated");
	return 0;
}



void DBD_create(SQD_t* dset, H5VL_loc_params_t loc_params, hid_t dcpl_id, hid_t dapl_id, hid_t dxpl_id) {
	TRACEMSG("");
	herr_t err;
	sqlite3* db = (sqlite3*) dset->object.root->db;

	char *sql2=  "INSERT INTO DATASETS VALUES(?,?,?,?,?,?,?,?,?);";

	sqlite3_stmt *res;
	const char *pzTest;
	int rc;

	rc = sqlite3_prepare(db, sql2, strlen(sql2), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s", sqlite3_errmsg(db));
	}

	hid_t type_id;
	if (-1 == (err = H5Pget(dcpl_id, "dataset_type_id", &type_id))) {
		ERRORMSG("Couldn't get type from dcpl.");
	}
	assert(-1 != type_id);
	size_t type_size;
	H5Tencode(type_id, NULL, &type_size);
	char* type_buf = (char*) malloc(type_size);
	H5Tencode(type_id, type_buf, &type_size);

	hid_t space_id;
	if (-1 == (err = H5Pget(dcpl_id, "dataset_space_id", &space_id))) {
		ERRORMSG("Couldn't get type from dcpl.");
	}
	assert(-1 != space_id);
	size_t space_size;
	H5Sencode(space_id, NULL, &space_size);
	unsigned char* space_buf = (unsigned char*) malloc(space_size);
	H5Sencode(space_id, space_buf, &space_size);

	size_t dcpl_size;
	H5Pencode(dcpl_id, NULL, &dcpl_size);
	char* dcpl_buf = (char*) malloc(dcpl_size);
	assert(NULL != dcpl_buf);
	H5Pencode(dcpl_id, dcpl_buf, &dcpl_size);

	size_t dapl_size;
	H5Pencode(dapl_id, NULL, &dapl_size);
	char* dapl_buf = (char*) malloc(dapl_size);
	assert(NULL != dapl_buf);
	H5Pencode(dapl_id, dapl_buf, &dapl_size);

	sqlite3_bind_text(res, 1, dset->object.location, strlen(dset->object.location), 0);
	sqlite3_bind_text(res, 2, dset->object.name, strlen(dset->object.name), 0);
	sqlite3_bind_blob(res, 3, type_buf, type_size, NULL);
	sqlite3_bind_blob(res, 4, space_buf, space_size, NULL);
	sqlite3_bind_blob(res, 5, dcpl_buf, dcpl_size, NULL);
	sqlite3_bind_blob(res, 6, dapl_buf, dapl_size, NULL);
	sqlite3_bind_blob(res, 7, &dset->object.info, sizeof(dset->object.info), NULL);
	sqlite3_bind_int64(res, 8, dset->offset);
	sqlite3_bind_int64(res, 9, dset->data_size);

	rc = sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	sqlite3_finalize(res);
	sqlite3_db_cacheflush(db);
}



SQD_t* DBD_open(SQO_t* parent, H5VL_loc_params_t loc_params, const char *name) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) parent->root->db;
	sqlite3_stmt *res;
	const char *pzTest;
	int rc;

	SQD_t* dset = (SQD_t*) malloc(sizeof(*dset));
	dset->object.root = parent->root;
	dset->object.fapl = parent->fapl;
	dset->object.location = create_path(parent);
	dset->object.name = strdup(name);

	char *sql=  "SELECT offset, data_size, info FROM DATASETS WHERE path = ? and name = ?;";

	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	sqlite3_bind_text(res, 1, dset->object.location, strlen(dset->object.location), 0);
	sqlite3_bind_text(res, 2, dset->object.name, strlen(dset->object.name), 0);

	rc = sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	dset->offset = sqlite3_column_int64(res, 0);
	dset->data_size = sqlite3_column_int64(res, 1);

	int data_size = sqlite3_column_bytes(res, 2);
	if (0 == data_size) {
		ERRORMSG("Couldn't read info.");
	}
	assert(data_size == sizeof(dset->object.info));
	const void * data =  sqlite3_column_blob(res, 2);
	memcpy(&dset->object.info, data, data_size);

	sqlite3_finalize(res);
	return dset;
}



void DBD_get_dcpl(SQD_t* dset, hid_t* plist) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) dset->object.root->db;
	sqlite3_stmt *res;
	const char *pzTest;

	const char* sql = "SELECT dcpl FROM DATASETS WHERE path = ? AND name = ?;";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	sqlite3_bind_text(res, 1, dset->object.location, strlen(dset->object.location), 0);
	sqlite3_bind_text(res, 2, dset->object.name, strlen(dset->object.name), 0);
	rc=sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	int data_size = sqlite3_column_bytes(res, 0);
	if (0 == data_size) {
		ERRORMSG("Couldn't read dcpl.");
	}
	unsigned char* buf = malloc(data_size);
	const void * data =  sqlite3_column_blob(res, 0);
	memcpy(buf, data, data_size);
	*plist = H5Pdecode(buf);
	if (-1 == *plist) {
		ERRORMSG("Couldn't read dcpl");
	}

	sqlite3_finalize(res);
}



void DBD_get_dapl(SQD_t* dset, hid_t* plist) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) dset->object.root->db;
	sqlite3_stmt *res;
	const char *pzTest;

	const char* sql = "SELECT dapl FROM DATASETS WHERE path = ? AND name = ?;";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	sqlite3_bind_text(res, 1, dset->object.location, strlen(dset->object.location), 0);
	sqlite3_bind_text(res, 2, dset->object.name, strlen(dset->object.name), 0);
	rc=sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	int data_size = sqlite3_column_bytes(res, 0);
	if (0 == data_size) {
		ERRORMSG("Couldn't read dapl.");
	}
	unsigned char* buf = malloc(data_size);
	const void * data =  sqlite3_column_blob(res, 0);
	memcpy(buf, data, data_size);
	*plist = H5Pdecode(buf);
	if (-1 == *plist) {
		ERRORMSG("Couldn't read dapl");
	}

	sqlite3_finalize(res);
}



void DBD_get_type(SQD_t* dset, hid_t* type_id) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) dset->object.root->db;
	sqlite3_stmt *res;
	const char *pzTest;

	char *sql=  "SELECT type FROM DATASETS WHERE path = ? AND name = ?;";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	sqlite3_bind_text(res, 1, dset->object.location, strlen(dset->object.location), 0);
	sqlite3_bind_text(res, 2, dset->object.name, strlen(dset->object.name), 0);

	rc=sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	int data_size = sqlite3_column_bytes(res, 0);
	if (0 == data_size) {
		ERRORMSG("Couldn't read type.");
	}
	unsigned char* type_buf = malloc(data_size);
	const void * data =  sqlite3_column_blob(res, 0);
	memcpy(type_buf, data, data_size);
	*type_id = H5Tdecode(type_buf);
	assert(-1 != *type_id);

	sqlite3_finalize(res);
}


void DBD_get_space(SQD_t* dset, hid_t* space_id) {
	TRACEMSG("");
	sqlite3* db = (sqlite3*) dset->object.root->db;
	sqlite3_stmt *res;
	const char *pzTest;

	char *sql=  "SELECT space FROM DATASETS WHERE path = ? AND name = ?;";
	int rc;

	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
	if (rc != SQLITE_OK) {
		ERRORMSG("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}

	sqlite3_bind_text(res, 1, dset->object.location, strlen(dset->object.location), 0);
	sqlite3_bind_text(res, 2, dset->object.name, strlen(dset->object.name), 0);

	rc = sqlite3_step(res);
	if (rc == SQLITE_BUSY) {
		ERRORMSG("Busy");
	}

	int data_size = sqlite3_column_bytes(res, 0);
	if (0 == data_size) {
		ERRORMSG("Couldn't read space.");
	}
	unsigned char* space_buf = malloc(data_size);
	const void * data =  sqlite3_column_blob(res, 0);
	memcpy(space_buf, data, data_size);
	*space_id = H5Sdecode(space_buf);
	assert(-1 != *space_id);

	sqlite3_finalize(res);
}



//
//void DBD_get(const char* path, /*OUT*/ hid_t* type_id, /*OUT*/ hid_t* space_id, void* db_ptr) {
//	TRACEMSG("");
//	sqlite3* db = (sqlite3*) db_ptr;
//	sqlite3_stmt *res;
//	const char *pzTest;
//	int rc;
//	char *sql=  "SELECT type, space FROM DATASETS WHERE Path = ?;";
//
//	rc = sqlite3_prepare(db, sql, strlen(sql), &res, &pzTest);
//	if (rc != SQLITE_OK) {
//		ERRORMSG("Cannot prepare statement: %s", sqlite3_errmsg(db));
//		sqlite3_close(db);
//	}
//
//	sqlite3_bind_text(res, 1, path, strlen(path), 0);
//
//	rc=sqlite3_step(res);
//
//	char* type_buf = (char*) sqlite3_column_blob(res, 0);
//	*type_id = H5Tdecode(type_buf);
//	assert(-1 != H5Tget_class(*type_id));
//
//	int data_size = sqlite3_column_bytes(res, 1);
//	unsigned char* space_buf = malloc(data_size);
//	const void * data =  sqlite3_column_blob(res, 1);
//	memcpy(space_buf, data, data_size);
//
//	*space_id = H5Sdecode(space_buf);
//	sqlite3_finalize(res);
//}



hid_t DBO_open(SQO_t* obj, H5VL_loc_params_t loc_params) {
	hid_t res = -1;
	switch (loc_params.obj_type) {
		case H5I_DATASET:
			ERRORMSG("Not supported");
			break;
		case H5I_GROUP:
			ERRORMSG("Not supported");
			break;
		default:
			ERRORMSG("Not supported");
	}
	return res;
}
