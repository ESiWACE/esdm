/*
 * =====================================================================================
 *
 *       Filename:  db_postgresql.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/19/2017 08:47:58 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "db_iface.h"
#include <libpq-fe.h>


static void
exit_nicely(PGconn *conn) {
//    PQfinish(conn);
	assert(false);
	exit(1);
}



// TODO: Fix
void connect(const char* db_fn, void* db) {
	PGconn     *conn;
	conn = PQconnectdb("hostaddr=136.172.14.20 dbname=somedb user=joobog port=5432");
	if (PQstatus(conn) != CONNECTION_OK) {
		ERRORMSG("Connection to database failed: %s", PQerrorMessage(conn));
		exit_nicely(conn);
	}
	file->db = conn;
}



void file_create_database(const char *name, unsigned flags, PGconn* conn) {
	DEBUGMSG("");
	PGresult* res;

	res = PQexec(conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		ERRORMSG("BEGIN command failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

	res = PQexec(conn, "DROP TABLE IF EXISTS DATASET;");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		DEBUGMSG("DROP failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

	res = PQexec(conn, "DROP TABLE IF EXISTS DATA;");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		DEBUGMSG("DROP failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

	res = PQexec(conn, "DROP TABLE IF EXISTS ATTRIBUTE;");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		DEBUGMSG("DROP failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

	res = PQexec(conn, "DROP TABLE IF EXISTS GROUPS;");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		DEBUGMSG("DROP failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

	char *sql = "CREATE TABLE IF NOT EXISTS FILE(File_Name TEXT, Flags INTEGER, PRIMARY KEY(File_Name));";
	//	DROP TABLE file_tb;

	res = PQexec(conn, sql);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		DEBUGMSG("CREATE failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

	res = PQexec(conn, "END");
	PQclear(res);
}




void attr_create_database(const char *attr_name, const char *obj_name, const char *obj_type, const char *location, hid_t type_id, hid_t space_id, hid_t data_size, PGconn* conn)
{
	DEBUGMSG("");
	PGresult* res;

// begin
	res = PQexec(conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		ERRORMSG("BEGIN command failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

// create
	char *sql = "CREATE TABLE IF NOT EXISTS ATTRIBUTE(Attribut_Name TEXT,"
		"Primary_Data_Object_Name TEXT, "
		"Primary_Data_Object_Type TEXT, "
		"Path TEXT, "
		"type BYTEA, "
		"space BYTEA, "
		"data_size BYTEA, "
		"PRIMARY KEY(Path));";

	res = PQexec(conn, sql);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		DEBUGMSG("CREATE TABLE failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

// insert
	size_t type_size;
	H5Tencode(type_id, NULL, &type_size);
	char type_buf[type_size];
	H5Tencode(type_id, type_buf, &type_size);

	size_t space_size;
	H5Sencode(space_id, NULL, &space_size);
	unsigned char* space_buf = (unsigned char*) malloc(space_size);
	H5Sencode(space_id, space_buf, &space_size);

	const char *paramValues[7] = {attr_name, obj_name, obj_type, location, type_buf, space_buf, (char*) &data_size};
	int         paramLengths[7] = {strlen(attr_name), strlen(obj_name), strlen(obj_type), strlen(location), type_size, space_size, sizeof(data_size)};
	int         paramFormats[7] = {0, 0, 0, 0, 1, 1, 1};
	res = PQexecParams(conn, "INSERT INTO ATTRIBUTE VALUES($1, $2, $3, $4, $5, $6, $7);", 7, NULL, paramValues, paramLengths, paramFormats, 1);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		DEBUGMSG("INSERT failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

// end
	res = PQexec(conn, "END");
	PQclear(res);

	free(space_buf);
}



static void attr_open_database(char* path, size_t* data_size, PGconn* conn) {
	DEBUGMSG("");
	PGresult* res;

// begin
	res = PQexec(conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		ERRORMSG("BEGIN command failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

// select
	char sql[200];
	sprintf(sql, "SELECT data_size FROM ATTRIBUTE WHERE path = '%s';", path);

	res = PQexec(conn, sql);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		DEBUGMSG("SELECT failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}

	assert(1 == PQntuples(res));
	for (int i = 0; i < PQntuples(res); i++) {
		size_t type_size;
		char* data_size_buf = PQunescapeBytea(PQgetvalue(res, i, 0), &type_size);
		*data_size = (size_t) *data_size_buf;
		PQfreemem(data_size_buf);
	}

	PQclear(res);

// end
	res = PQexec(conn, "END");
	PQclear(res);
}




static void attr_get_database(const char* path, /*OUT*/ hid_t* type_id, /*OUT*/ hid_t* space_id, PGconn* conn) {
	DEBUGMSG("");
	PGresult* res;

// begin
	res = PQexec(conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		ERRORMSG("BEGIN command failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

// select
	char sql[200];
	sprintf(sql, "SELECT type, space FROM DATASET WHERE path='%s';", path);

	res = PQexec(conn, sql);

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		ERRORMSG("SELECT failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}

	assert(1 == PQntuples(res));
	assert(2 == PQnfields(res));

	for (int i = 0; i < PQntuples(res); i++) {
		size_t type_size;
		size_t space_size;

		char* type_buf = PQunescapeBytea(PQgetvalue(res, i, 0), &type_size);
		char* space_buf = PQunescapeBytea(PQgetvalue(res, i, 1), &space_size);

		*type_id = H5Tdecode(type_buf);
		assert(-1 != H5Tget_class(*type_id));
		*space_id = H5Sdecode(space_buf);
	}
	PQclear(res);

// end
	res = PQexec(conn, "END");
	PQclear(res);
}



void attr_write_database( const char *location, const void *data, int size, PGconn* conn) {
	DEBUGMSG("");
	PGresult* res;

// begin
	res = PQexec(conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		ERRORMSG("BEGIN command failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

// create
	char *sql = "CREATE TABLE IF NOT EXISTS DATA("
	"Path TEXT, "
	"Data BYTEA, "
	"PRIMARY KEY(Path));";


	res = PQexec(conn, sql);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		DEBUGMSG("CREATE TABLE failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

	const char *paramValues[2] = {location, (char*) data};
	int         paramLengths[2] = {strlen(location), size};
	int         paramFormats[2] = {0, 1};
	res = PQexecParams(conn, "INSERT INTO DATA VALUES($1, $2);", 2, NULL, paramValues, paramLengths, paramFormats, 1);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		DEBUGMSG("INSERT failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

// end
	res = PQexec(conn, "END");
	PQclear(res);
}


void attr_read_database( const char *location, /*OUT*/void *buf, PGconn* conn) {
	DEBUGMSG("");
	PGresult* res;

// begin
	res = PQexec(conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		ERRORMSG("BEGIN command failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

// select
	char sql[200];
	sprintf(sql, "SELECT data FROM DATA WHERE path='%s';", location);
	res = PQexec(conn, sql);

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		ERRORMSG("SELECT failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}

	assert(1 == PQntuples(res));
	for (int i = 0; i < PQntuples(res); i++) {
		size_t data_size;
		char* data_size_buf = PQunescapeBytea(PQgetvalue(res, i, 0), &data_size);
		buf = malloc(data_size);
		memcpy(buf, data_size_buf, data_size);
		PQfreemem(data_size_buf);
	}
	PQclear(res);


void group_create_database(const char *group_name, const char *obj_name, const char *obj_type, const char *location, PGconn* conn) {
	DEBUGMSG("");
	PGresult* res;

// begin
	res = PQexec(conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		ERRORMSG("BEGIN command failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

// create
	char *sql = "CREATE TABLE IF NOT EXISTS GROUPS("
	"Group_Name TEXT, Primary_Data_Object_Name TEXT,"
	"Primary_Data_Object_Type TEXT, Path TEXT,"
	"PRIMARY KEY(Path));";

	res = PQexec(conn, sql);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		DEBUGMSG("CREATE TABLE failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

	// insert
//	if (0 == ginfo->mpi_rank)  {
		const char *paramValues[4] = {group_name, obj_name, obj_type, location};
		int         paramLengths[4] = {strlen(group_name), strlen(obj_name), strlen(obj_type), strlen(location)};
		int         paramFormats[4] = {0, 0, 0, 0};
		res = PQexecParams(conn, "INSERT INTO GROUPS VALUES($1,$2,$3,$4);", 4, NULL, paramValues, paramLengths, paramFormats, 1);

		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			DEBUGMSG("INSERT failed: %s", PQerrorMessage(conn));
			PQclear(res);
			exit_nicely(conn);
		}
		PQclear(res);

		// end
		res = PQexec(conn, "END");
		PQclear(res);
//	}
//MPI_Barrier(MPI_COMM_WORLD);
}




static void dataset_create_database(const char *dataset_name, const char *obj_name, const char *obj_type, const char *location, hid_t type_id, hid_t space_id, hid_t data_size, PGconn* conn) {
	DEBUGMSG("");
	PGresult* res;

// begin
	res = PQexec(conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		ERRORMSG("BEGIN command failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

// create
	char *sql = "CREATE TABLE IF NOT EXISTS DATASET(Dataset_Name TEXT,"
		"Primary_Data_Object_Name TEXT,"
		"Primary_Data_Object_Type TEXT,"
		"Path TEXT,"
		"type BYTEA, "
		"space BYTEA, "
		"data_size BYTEA, "
		"PRIMARY KEY(Path));";

	res = PQexec(conn, sql);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		ERRORMSG("CREATE TABLE failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

// insert
	size_t type_size;
	H5Tencode(type_id, NULL, &type_size);
	char type_buf[type_size];
	H5Tencode(type_id, type_buf, &type_size);

	size_t space_size;
	H5Sencode(space_id, NULL, &space_size);
	unsigned char* space_buf = (unsigned char*) malloc(space_size);
	H5Sencode(space_id, space_buf, &space_size);

	const char *paramValues[7] = {
		dataset_name, obj_name, obj_type, location,
		type_buf, space_buf, (char*) &data_size};

	int         paramLengths[7] = {
		strlen(dataset_name), strlen(obj_name), strlen(obj_type), strlen(location),
		type_size, space_size, sizeof(data_size)};

	int         paramFormats[7] = {0, 0, 0, 0, 1, 1, 1};
	res = PQexecParams(conn, "INSERT INTO DATASET VALUES($1,$2,$3,$4,$5,$6,$7);", 7, NULL, paramValues, paramLengths, paramFormats, 1);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		DEBUGMSG("INSERT failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

// end
	res = PQexec(conn, "END");
	PQclear(res);

	free(space_buf);
}



static void dataset_open_database(const char* path, size_t* data_size, PGconn* conn) {
	DEBUGMSG("");
	PGresult* res;

// begin
	res = PQexec(conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		ERRORMSG("BEGIN command failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

// select
	char sql[200];
	sprintf(sql, "SELECT data_size FROM DATASET WHERE path='%s';", path);
	res = PQexec(conn, sql);

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		ERRORMSG("SELECT failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}

	assert(1 == PQntuples(res));
	for (int i = 0; i < PQntuples(res); i++) {
		size_t field_size;
		char* source_buf = PQunescapeBytea(PQgetvalue(res, i, 0), &field_size);
		char* target_buf = malloc(field_size);
		memcpy(target_buf, source_buf, field_size);
		PQfreemem(source_buf);
	}
	PQclear(res);

// end
	res = PQexec(conn, "END");
	PQclear(res);
}


static void dataset_get_database(const char* path, /*OUT*/ hid_t* type_id, /*OUT*/ hid_t* space_id, PGconn* conn) {
	DEBUGMSG("");
	PGresult* res;

// begin
	res = PQexec(conn, "BEGIN");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		ERRORMSG("BEGIN command failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

// select
	char sql[200];
	sprintf(sql, "SELECT type, space FROM DATASET WHERE path='%s';", path);

	res = PQexec(conn, sql);

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		ERRORMSG("SELECT failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}

//	PQprintOpt options = {0};
//	options.header    = 1;    /*  Ask for column headers            */
//	options.align     = 1;    /*  Pad short columns for alignment   */
//	options.fieldSep  = "|";  /*  Use a pipe as the field separator */
//
//	PQprint(stdout, res, &options);

	assert(1 == PQntuples(res));
	assert(2 == PQnfields(res));

	for (int i = 0; i < PQntuples(res); i++) {
//		int type_pq_size = PQgetlength(res, i, 0);
//		int space_pq_size = PQgetlength(res, i, 1);

		size_t type_size;
		size_t space_size;

		char* type_buf = PQunescapeBytea(PQgetvalue(res, i, 0), &type_size);
		char* space_buf = PQunescapeBytea(PQgetvalue(res, i, 1), &space_size);

		*type_id = H5Tdecode(type_buf);
		assert(-1 != H5Tget_class(*type_id));
		*space_id = H5Sdecode(space_buf);
	}


	PQclear(res);

// end
	res = PQexec(conn, "END");
	PQclear(res);

}
