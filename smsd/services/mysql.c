/* (c) 2004 by Marcin Wiacek */

#include <gammu.h>

#ifdef HAVE_MYSQL_MYSQL_H

#define _XOPEN_SOURCE
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

#ifdef WIN32
#include <windows.h>
#ifndef __GNUC__
#pragma comment(lib, "libmysql.lib")
#endif
#endif

#include "../core.h"
//#include "sql.h"

long long SMSDMySQL_GetNumber(SQL_result rc, unsigned int field)
{
	return atoi(rc.my.row[field]);
}

const char *SMSDMySQL_GetString(SQL_result rc, unsigned int field)
{
	return rc.my.row[field];
}

time_t SMSDMySQL_GetDate(SQL_result rc, unsigned int field)
{
	const char *date;
	int ret;
	struct tm t;
	
	SMSDMySQL.error = NULL;
	date = rc.my.row[field];
	/* windows has not strptime() */
	ret = sscanf(date, "%u-%u-%u %u:%u:%u", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec);
	t.tm_isdst = -1;
	t.tm_year -= 1900;
	t.tm_mon--;
#ifdef HAVE_STRUCT_TM_TM_ZONE
	t.tm_gmtoff = 0;
	t.tm_zone = NULL;
#endif
	if (ret == 6) {
		return mktime(&t);
	}
	SMSDMySQL.error = "Failed to process date!";
	return -1;
}

gboolean SMSDMySQL_GetBool(SQL_result rc, unsigned int field)
{
	const char *value = rc.my.row[field];
	
	SMSDMySQL.error = NULL;
	if(atoi(value) > 0){
		return TRUE;
	}
	if (strcasecmp(value, "yes") == 0 || strcasecmp(value, "TRUE") == 0 || strcasecmp(value, "y") == 0 || strcasecmp(value, "t") == 0) {
		return TRUE;
	}
	if (strcasecmp(value, "no") == 0 || strcasecmp(value, "FALSE") == 0 || strcasecmp(value, "n") == 0 || strcasecmp(value, "f") == 0) {
		return FALSE;
	}
	return FALSE;
}

const char *SMSMySQL_GetString(SQL_result res, unsigned int col)
{
	SMSDMySQL.error = NULL;
	return res.my.row[col];
}

/* Disconnects from a database */
void SMSDMySQL_Free(SQL_conn *conn)
{
	mysql_close(conn->my);
	free(conn->my);
	conn->my = NULL;
}

static int SMSDMySQL_LogError(GSM_SMSDConfig * Config)
{
	int mysql_err;

	mysql_err = mysql_errno(Config->db->conn.my);

	SMSD_Log(DEBUG_ERROR, Config, "Error code: %d, Error: %s", mysql_err, mysql_error(Config->db->conn.my));

	return mysql_err;
}

static SQL_Error SMSDMySQL_Connect(GSM_SMSDConfig * Config)
{

	unsigned int port = 0;
	int error;
	char *pport;
	char *socketname = NULL;
	struct GSM_SMSDdbobj *db = Config->db;

	pport = strstr(Config->host, ":");
	if (pport) {
		*pport++ = '\0';
		/* Is it port or socket? */
		if (strchr("0123456798", *pport) != NULL) {
			port = atoi(pport);
		} else {
			socketname = pport;
		}
	}
	if (db->conn.my == NULL) {
		db->conn.my = malloc(sizeof(MYSQL));
	}
	if (db->conn.my == NULL) {
		SMSD_Log(DEBUG_ERROR, Config, "MySQL allocation failed!");
		return SQL_FAIL;
	}
	mysql_init(db->conn.my);
	if (!mysql_real_connect(db->conn.my, Config->host, Config->user, Config->password, Config->database, port, socketname, 0)) {
		SMSD_Log(DEBUG_ERROR, Config, "Error connecting to database!");
		SMSDMySQL_LogError(Config);
		error = mysql_errno(db->conn.my);
		if (error == 2003 || error == 2002) { /* cant connect through socket */
			return SQL_TIMEOUT;
		}
		return SQL_FAIL;
	}

	mysql_query(db->conn.my, "SET NAMES utf8;");
	SMSD_Log(DEBUG_INFO, Config, "Connected to Database: %s on %s", Config->database, Config->host);
	return SQL_OK;
}

static SQL_Error SMSDMySQL_Query(GSM_SMSDConfig * Config, const char *query, SQL_result *res)
{
	struct GSM_SMSDdbobj *db = Config->db;
	int error;

	if (mysql_query(db->conn.my, query) != 0) {
		SMSDMySQL_LogError(Config);
		error = mysql_errno(db->conn.my);
		if (error == 2006 || error == 2013 || error == 2012) { /* connection lost */
			return SQL_TIMEOUT;
		}
		return SQL_FAIL;	
	}
	
	res->my.res = mysql_store_result(db->conn.my);
	res->my.row = NULL; 
	res->my.con = db->conn.my; 

	return SQL_OK;
}

/* free mysql results */
void SMSDMySQL_FreeResult(SQL_result res)
{
	mysql_free_result(res.my.res);
}

/* set pointer to next row */
int SMSDMySQL_NextRow(SQL_result *res)
{
	MYSQL_ROW row;
	row = mysql_fetch_row(res->my.res);
	res->my.row = row;
	if(row != NULL){
		return 1;
	}
	return 0;
}

/* quote strings */
char * SMSDMySQL_QuoteString(SQL_conn *conn, const char *string)
{
	char *buff;
	int len = strlen(string);
	buff = malloc(len*2+3);
	
	buff[0] = '\'';
	buff[1] = '\0';
	mysql_real_escape_string(conn->my, buff+1, string, len);
	strcat(buff, "'");
	return buff;
}

/* LAST_INSERT_ID */
unsigned long long SMSDMySQL_SeqID(SQL_conn *conn, const char *dummy)
{
	return mysql_insert_id(conn->my);
}

unsigned long SMSDMySQL_AffectedRows(SQL_result Res)
{
	return mysql_affected_rows(Res.my.con);
}

unsigned long SMSDMySQL_NumRows(SQL_result Res)
{
	return mysql_num_rows(Res.my.res);
}

struct GSM_SMSDdbobj SMSDMySQL = {
	"mysql",
	SMSDMySQL_Connect,
	SMSDMySQL_Query,
	SMSDMySQL_Free,
	SMSDMySQL_FreeResult,
	SMSDMySQL_NextRow,
	SMSDMySQL_SeqID,
	SMSDMySQL_AffectedRows,
	SMSDMySQL_NumRows,
	SMSDMySQL_GetString,
	SMSDMySQL_GetNumber,
	SMSDMySQL_GetDate,
	SMSDMySQL_GetBool,
	SMSDMySQL_QuoteString,
	NULL,
	{.my = NULL}
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
