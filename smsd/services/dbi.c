/**
 * libdbi database backend
 *
 * Part of Gammu project
 *
 * Copyright (C) 2009 - 2010 Michal Čihař
 *
 * Licensed under GNU GPL version 2 or later
 */

#define _XOPEN_SOURCE
#define _BSD_SOURCE
#include "../../helper/strptime.h"
#include <time.h>
#include <gammu.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#ifdef WIN32
#include <windows.h>
#ifndef __GNUC__
#pragma comment(lib, "libdbi.lib")
#endif
#endif

#include "../core.h"

long long SMSDDBI_GetNumber(GSM_SMSDConfig * Config, SQL_result rc, unsigned int field)
{
	unsigned int type;
	dbi_result res = rc.dbi;

	field++;
	type = dbi_result_get_field_type_idx(res, field);
	SMSDDBI.error = NULL;

	switch (type) {
		case DBI_TYPE_INTEGER:
			type = dbi_result_get_field_attribs_idx(res, field);
			if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE1) {
				return dbi_result_get_int_idx(res, field);
			} else if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE2) {
				return dbi_result_get_int_idx(res, field);
			} else if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE3) {
				return dbi_result_get_int_idx(res, field);
			} else if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE4) {
				return dbi_result_get_int_idx(res, field);
			} else if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE8) {
				return dbi_result_get_longlong_idx(res, field);
			}
			SMSDDBI.error = "Wrong integer field subtype!";
			return -1;
		case DBI_TYPE_DECIMAL:
			type = dbi_result_get_field_attribs_idx(res, field);
			if ((type & DBI_DECIMAL_SIZEMASK) == DBI_DECIMAL_SIZE4) {
				return dbi_result_get_int_idx(res, field);
			} else if ((type & DBI_DECIMAL_SIZEMASK) == DBI_DECIMAL_SIZE8) {
				return dbi_result_get_longlong_idx(res, field);
			}
			SMSDDBI.error = "Wrong decimal field subtype!";
			return -1;
		default:
			SMSDDBI.error = "Wrong field type! (not INTEGER nor DECIMAL)";
			return -1;
	}
}

time_t SMSDDBI_GetDate(GSM_SMSDConfig * Config, SQL_result rc, unsigned int field)
{
	unsigned int type;
	struct tm timestruct;
	char *parse_res;
	const char *date;
	dbi_result res = rc.dbi;

	field++;
	type = dbi_result_get_field_type_idx(res, field);
	SMSDDBI.error = NULL;

	switch (type) {
		case DBI_TYPE_INTEGER:
		case DBI_TYPE_DECIMAL:
			return SMSDDBI_GetNumber(Config, rc, field);
		case DBI_TYPE_STRING:
			date = dbi_result_get_string_idx(res, field);
			parse_res = strptime(date, "%Y-%m-%d %H:%M:%S", &timestruct);
			timestruct.tm_isdst = 0;
#ifdef HAVE_STRUCT_TM_TM_ZONE
			timestruct.tm_gmtoff = 0;
			timestruct.tm_zone = NULL;
#endif
			if (parse_res != NULL && *parse_res == 0) {
				return mktime(&timestruct);
			}
			SMSD_Log(DEBUG_ERROR, Config, "Failed to parse date: %s", date);
			SMSDDBI.error = "Failed to process date";
			return -1;
		case DBI_TYPE_DATETIME:
			return dbi_result_get_datetime_idx(res, field);
		case DBI_TYPE_ERROR:
		default:
			SMSDDBI.error = "Wrong date field type!";
			return -1;
	}
}

gboolean SMSDDBI_GetBool(GSM_SMSDConfig * Config, SQL_result rc, unsigned int field)
{
	unsigned int type;
	const char *value;
	int num;
	dbi_result res = rc.dbi;

	field++;
	type = dbi_result_get_field_type_idx(res, field);

	switch (type) {
		case DBI_TYPE_INTEGER:
		case DBI_TYPE_DECIMAL:
			num = SMSDDBI_GetNumber(Config, rc, field);
			if (num == -1) {
				return -1;
			} else if (num == 0) {
				return FALSE;
			} else {
				return TRUE;
			}
		case DBI_TYPE_STRING:
			value = dbi_result_get_string_idx(res, field);
			return GSM_StringToBool(value);
		case DBI_TYPE_ERROR:
		default:
			SMSDDBI.error = "Wrong gboolean field type!";
			return -1;
	}
}

const char *SMSDDBI_GetString(GSM_SMSDConfig * Config, SQL_result res, unsigned int col)
{
	return dbi_result_get_string_idx(res.dbi, col+1);
}

static void SMSDDBI_LogError(GSM_SMSDConfig * Config)
{
	int rc;
	const char *msg;
	rc = dbi_conn_error(((struct GSM_SMSDdbobj *)Config->db)->conn.dbi, &msg);
	if (rc == -1) {
		SMSD_Log(DEBUG_ERROR, Config, "Unknown DBI error!");
	} else {
		SMSD_Log(DEBUG_ERROR, Config, "DBI error %d: %s", rc, msg);
	}
}

void SMSDDBI_Callback(dbi_conn Conn, void *Config)
{
	SMSDDBI_LogError((GSM_SMSDConfig *) Config);
}

/* Disconnects from a database */
void SMSDDBI_Free(GSM_SMSDConfig * Config)
{
	if (Config->db->conn.dbi != NULL) {
		dbi_conn_close(Config->db->conn.dbi);
		dbi_shutdown();
		Config->db->conn.dbi = NULL;
	}
}

/* Connects to database */
static SQL_Error SMSDDBI_Connect(GSM_SMSDConfig * Config)
{
	int rc;
	struct GSM_SMSDdbobj *db = Config->db;

	rc = dbi_initialize(Config->driverspath);

	if (rc == 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI did not find any drivers, try using DriversPath option");
		dbi_shutdown();
		return SQL_FAIL;
	} else if (rc < 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to initialize!");
		return SQL_FAIL;
	}

	db->conn.dbi = dbi_conn_new(Config->driver);
	db->DriverName = dbi_driver_get_name(dbi_conn_get_driver(db->conn.dbi));
	if (db->conn.dbi == NULL) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to init %s driver!", Config->driver);
		dbi_shutdown();
		return SQL_FAIL;
	} else {
		SMSD_Log(DEBUG_SQL, Config, "Using DBI driver '%s'", db->DriverName);
	}

	dbi_conn_error_handler(db->conn.dbi, SMSDDBI_Callback, Config);

	if (dbi_conn_set_option(db->conn.dbi, "sqlite_dbdir", Config->dbdir) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set sqlite_dbdir!");
		SMSDDBI_Free(Config);
		return SQL_FAIL;
	}
	if (dbi_conn_set_option(db->conn.dbi, "sqlite3_dbdir", Config->dbdir) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set sqlite3_dbdir!");
		SMSDDBI_Free(Config);
		return SQL_FAIL;
	}
	if (dbi_conn_set_option(db->conn.dbi, "host", Config->host) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set host!");
		SMSDDBI_Free(Config);
		return SQL_FAIL;
	}
	if (dbi_conn_set_option(db->conn.dbi, "username", Config->user) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set username!");
		SMSDDBI_Free(Config);
		return SQL_FAIL;
	}
	if (dbi_conn_set_option(db->conn.dbi, "password", Config->password) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set password!");
		SMSDDBI_Free(Config);
		return SQL_FAIL;
	}
	if (dbi_conn_set_option(db->conn.dbi, "dbname", Config->database) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set dbname!");
		SMSDDBI_Free(Config);
		return SQL_FAIL;
	}
	if (dbi_conn_set_option(db->conn.dbi, "encoding", "UTF-8") != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set encoding!");
		SMSDDBI_Free(Config);
		return SQL_FAIL;
	}

	if (dbi_conn_connect(db->conn.dbi) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to connect!");
		SMSDDBI_Free(Config);
		return SQL_FAIL;
	}
	Config->db = db;
	return SQL_OK;
}

static SQL_Error SMSDDBI_Query(GSM_SMSDConfig * Config, const char *query, SQL_result * res)
{
	const char *msg;
	int rc;
	struct GSM_SMSDdbobj *db = Config->db;

	res->dbi = NULL;

	SMSD_Log(DEBUG_SQL, Config, "Execute SQL: %s", query);
	res->dbi = dbi_conn_query(db->conn.dbi, query);
	if (res->dbi != NULL)
		return SQL_OK;

	SMSD_Log(DEBUG_INFO, Config, "SQL failed: %s", query);
	/* Black magic to decide whether we should bail out or attempt to retry */
	rc = dbi_conn_error(db->conn.dbi, &msg);
	if (rc != -1) {
		SMSD_Log(DEBUG_INFO, Config, "SQL failure: %s", msg);
		if (strstr(msg, "syntax") != NULL) {
			return SQL_FAIL;
		}
		if (strstr(msg, "violation") != NULL) {
			return SQL_FAIL;
		}
		if (strstr(msg, "violates") != NULL) {
			return SQL_FAIL;
		}
		if (strstr(msg, "SQL error") != NULL) {
			return SQL_FAIL;
		}
		if (strstr(msg, "duplicate") != NULL) {
			return SQL_FAIL;
		}
		if (strstr(msg, "unique") != NULL) {
			return SQL_FAIL;
		}
		if (strstr(msg, "need to rewrite") != NULL) {
			return SQL_FAIL;
		}
		if (strstr(msg, "locked") != NULL) {
			return SQL_TIMEOUT;
		}
	}
	return ERR_TIMEOUT;
}

/* free sql results */
void SMSDDBI_FreeResult(GSM_SMSDConfig * Config, SQL_result res)
{
	dbi_result_free(res.dbi);
}

/* set pointer to next row */
int SMSDDBI_NextRow(GSM_SMSDConfig * Config, SQL_result *res)
{
	return dbi_result_next_row(res->dbi);
}
/* quote strings */
char * SMSDDBI_QuoteString(GSM_SMSDConfig * Config, SQL_conn *conn, const char *string)
{
	char *encoded_text = NULL;
	dbi_conn_quote_string_copy(conn->dbi, string, &encoded_text);
	return encoded_text;
}
/* LAST_INSERT_ID */
unsigned long long SMSDDBI_SeqID(GSM_SMSDConfig * Config, const char *id)
{
	unsigned long long new_id;
	new_id = dbi_conn_sequence_last(Config->db->conn.dbi, NULL);
	if (new_id == 0) {
		new_id = dbi_conn_sequence_last(Config->db->conn.dbi, id);
	}
	return new_id;
}

unsigned long SMSDDBI_AffectedRows(GSM_SMSDConfig * Config, SQL_result Res)
{
	return dbi_result_get_numrows_affected(Res.dbi);
}

unsigned long SMSDDBI_NumRows(GSM_SMSDConfig * Config, SQL_result Res)
{
	return dbi_result_get_numrows(Res.dbi);
}

struct GSM_SMSDdbobj SMSDDBI = {
	"",
	SMSDDBI_Connect,
	SMSDDBI_Query,
	SMSDDBI_Free,
	SMSDDBI_FreeResult,
	SMSDDBI_NextRow,
	SMSDDBI_SeqID,
	SMSDDBI_AffectedRows,
	SMSDDBI_NumRows,
	SMSDDBI_GetString,
	SMSDDBI_GetNumber,
	SMSDDBI_GetDate,
	SMSDDBI_GetBool,
	SMSDDBI_QuoteString,
	NULL,
	{.dbi = NULL}
};

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
