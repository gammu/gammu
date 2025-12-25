/**
 * libdbi database backend
 *
 * Part of Gammu project
 *
 * Copyright (c) 2009 - 2018 Michal Cihar <michal@cihar.com>
 *
 * Licensed under GNU GPL version 2 or later
 */

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
#include "sql.h"

long long SMSDDBI_GetNumber(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	unsigned int type;

	field++;
	type = dbi_result_get_field_type_idx(res->dbi, field);

	switch (type) {
		case DBI_TYPE_INTEGER:
			type = dbi_result_get_field_attribs_idx(res->dbi, field);
			if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE1) {
				return dbi_result_get_int_idx(res->dbi, field);
			} else if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE2) {
				return dbi_result_get_int_idx(res->dbi, field);
			} else if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE3) {
				return dbi_result_get_int_idx(res->dbi, field);
			} else if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE4) {
				return dbi_result_get_int_idx(res->dbi, field);
			} else if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE8) {
				return dbi_result_get_longlong_idx(res->dbi, field);
			}
			SMSD_Log(DEBUG_ERROR, Config, "Wrong integer field subtype from DBI: %d", type);
			return -1;
		case DBI_TYPE_DECIMAL:
			type = dbi_result_get_field_attribs_idx(res->dbi, field);
			if ((type & DBI_DECIMAL_SIZEMASK) == DBI_DECIMAL_SIZE4) {
				return dbi_result_get_int_idx(res->dbi, field);
			} else if ((type & DBI_DECIMAL_SIZEMASK) == DBI_DECIMAL_SIZE8) {
				return dbi_result_get_longlong_idx(res->dbi, field);
			}
			SMSD_Log(DEBUG_ERROR, Config, "Wrong decimal field subtype from DBI: %d", type);
			return -1;
#ifdef DBI_TYPE_XDECIMAL
		case DBI_TYPE_XDECIMAL:
			return dbi_result_get_as_longlong_idx(res->dbi, field);
#endif
		default:
			SMSD_Log(DEBUG_ERROR, Config, "Wrong field type for number (not INTEGER nor DECIMAL) from DBI: %d", type);
			return -1;
	}
}

time_t SMSDDBI_GetDate(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	unsigned int type;
	const char *date;

	field++;
	type = dbi_result_get_field_type_idx(res->dbi, field);

	switch (type) {
		case DBI_TYPE_INTEGER:
		case DBI_TYPE_DECIMAL:
#ifdef DBI_TYPE_XDECIMAL
		case DBI_TYPE_XDECIMAL:
#endif
			return SMSDDBI_GetNumber(Config, res, field);
		case DBI_TYPE_STRING:
			date = dbi_result_get_string_idx(res->dbi, field);
			return SMSDSQL_ParseDate(Config, date);
		case DBI_TYPE_DATETIME:
			return dbi_result_get_datetime_idx(res->dbi, field);
		case DBI_TYPE_ERROR:
		default:
			SMSD_Log(DEBUG_ERROR, Config, "Wrong field type for date from DBI: %d", type);
			return -1;
	}
}

gboolean SMSDDBI_GetBool(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	unsigned int type;
	const char *value;
	int num;

	field++;
	type = dbi_result_get_field_type_idx(res->dbi, field);

	switch (type) {
		case DBI_TYPE_INTEGER:
		case DBI_TYPE_DECIMAL:
#ifdef DBI_TYPE_XDECIMAL
		case DBI_TYPE_XDECIMAL:
#endif
			num = SMSDDBI_GetNumber(Config, res, field);
			if (num == -1) {
				return -1;
			} else if (num == 0) {
				return FALSE;
			} else {
				return TRUE;
			}
		case DBI_TYPE_STRING:
			value = dbi_result_get_string_idx(res->dbi, field);
			return GSM_StringToBool(value);
		case DBI_TYPE_ERROR:
		default:
			SMSD_Log(DEBUG_ERROR, Config, "Wrong field type for boolean from DBI: %d", type);
			return -1;
	}
}

const char *SMSDDBI_GetString(GSM_SMSDConfig * Config, SQL_result *res, unsigned int col)
{
	return dbi_result_get_string_idx(res->dbi, col+1);
}

static void SMSDDBI_LogError(GSM_SMSDConfig * Config)
{
	int rc;
	const char *msg;
	rc = dbi_conn_error(Config->conn.dbi, &msg);
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
	if (Config->conn.dbi != NULL) {
		dbi_conn_close(Config->conn.dbi);
		dbi_shutdown();
		Config->conn.dbi = NULL;
	}
}

/* Connects to database */
static GSM_Error SMSDDBI_Connect(GSM_SMSDConfig * Config)
{
	int rc;
	struct GSM_SMSDdbobj *db = Config->db;

	rc = dbi_initialize(Config->driverspath);

	if (rc == 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI did not find any drivers, try using DriversPath option");
		dbi_shutdown();
		return ERR_DB_DRIVER;
	} else if (rc < 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to initialize!");
		return ERR_DB_DRIVER;
	}

	Config->conn.dbi = dbi_conn_new(Config->driver);
	if (Config->conn.dbi == NULL) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to init %s driver!", Config->driver);
		dbi_shutdown();
		return ERR_DB_DRIVER;
	} else {
		SMSD_Log(DEBUG_SQL, Config, "Using DBI driver '%s'", dbi_driver_get_name(dbi_conn_get_driver(Config->conn.dbi)));
	}

	dbi_conn_error_handler(Config->conn.dbi, SMSDDBI_Callback, Config);

	if (dbi_conn_set_option(Config->conn.dbi, "sqlite_dbdir", Config->dbdir) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set sqlite_dbdir!");
		SMSDDBI_Free(Config);
		return ERR_DB_CONFIG;
	}
	if (dbi_conn_set_option(Config->conn.dbi, "sqlite3_dbdir", Config->dbdir) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set sqlite3_dbdir!");
		SMSDDBI_Free(Config);
		return ERR_DB_CONFIG;
	}
	if (dbi_conn_set_option(Config->conn.dbi, "host", Config->host) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set host!");
		SMSDDBI_Free(Config);
		return ERR_DB_CONFIG;
	}
	if (dbi_conn_set_option(Config->conn.dbi, "username", Config->user) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set username!");
		SMSDDBI_Free(Config);
		return ERR_DB_CONFIG;
	}
	if (dbi_conn_set_option(Config->conn.dbi, "password", Config->password) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set password!");
		SMSDDBI_Free(Config);
		return ERR_DB_CONFIG;
	}
	if (dbi_conn_set_option(Config->conn.dbi, "dbname", Config->database) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set dbname!");
		SMSDDBI_Free(Config);
		return ERR_DB_CONFIG;
	}
	if (dbi_conn_set_option(Config->conn.dbi, "encoding", "UTF-8") != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set encoding!");
		SMSDDBI_Free(Config);
		return ERR_DB_CONFIG;
	}

	if (dbi_conn_connect(Config->conn.dbi) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to connect!");
		SMSDDBI_Free(Config);
		return ERR_DB_CONNECT;
	}
	Config->db = db;
	return ERR_NONE;
}

static GSM_Error SMSDDBI_Query(GSM_SMSDConfig * Config, const char *query, SQL_result * res)
{
	const char *msg;
	int rc;

	res->dbi = NULL;

	res->dbi = dbi_conn_query(Config->conn.dbi, query);
	if (res->dbi != NULL)
		return ERR_NONE;

	SMSD_Log(DEBUG_INFO, Config, "SQL failed: %s", query);
	/* Black magic to decide whether we should bail out or attempt to retry */
	rc = dbi_conn_error(Config->conn.dbi, &msg);
	if (rc != -1) {
		SMSD_Log(DEBUG_INFO, Config, "SQL failure: %s", msg);
		if (strstr(msg, "syntax") != NULL) {
			return ERR_SQL;
		}
		if (strstr(msg, "violation") != NULL) {
			return ERR_SQL;
		}
		if (strstr(msg, "violates") != NULL) {
			return ERR_SQL;
		}
		if (strstr(msg, "SQL error") != NULL) {
			return ERR_SQL;
		}
		if (strstr(msg, "duplicate") != NULL) {
			return ERR_SQL;
		}
		if (strstr(msg, "unique") != NULL) {
			return ERR_SQL;
		}
		if (strstr(msg, "need to rewrite") != NULL) {
			return ERR_SQL;
		}
		if (strstr(msg, "locked") != NULL) {
			return ERR_DB_TIMEOUT;
		}
	}
	return ERR_DB_TIMEOUT;
}

/* free sql results */
void SMSDDBI_FreeResult(GSM_SMSDConfig * Config, SQL_result *res)
{
	dbi_result_free(res->dbi);
}

/* set pointer to next row */
int SMSDDBI_NextRow(GSM_SMSDConfig * Config, SQL_result *res)
{
	return dbi_result_next_row(res->dbi);
}
/* quote strings */
char * SMSDDBI_QuoteString(GSM_SMSDConfig * Config, const char *string)
{
	char *encoded_text = NULL;
	dbi_conn_quote_string_copy(Config->conn.dbi, string, &encoded_text);
	return encoded_text;
}
/* LAST_INSERT_ID */
unsigned long long SMSDDBI_SeqID(GSM_SMSDConfig * Config, const char *id)
{
	unsigned long long new_id;
	char buffer[100];

	new_id = dbi_conn_sequence_last(Config->conn.dbi, NULL);
	if (new_id == 0) {
		new_id = dbi_conn_sequence_last(Config->conn.dbi, id);
		/* Need to escape for PostgreSQL */
		if (new_id == 0) {
			sprintf(buffer, "\"%s\"", id);
			new_id = dbi_conn_sequence_last(Config->conn.dbi, buffer);
		}
	}
	return new_id;
}

unsigned long SMSDDBI_AffectedRows(GSM_SMSDConfig * Config, SQL_result *res)
{
	return dbi_result_get_numrows_affected(res->dbi);
}

struct GSM_SMSDdbobj SMSDDBI = {
	SMSDDBI_Connect,
	SMSDDBI_Query,
	SMSDDBI_Free,
	SMSDDBI_FreeResult,
	SMSDDBI_NextRow,
	SMSDDBI_SeqID,
	SMSDDBI_AffectedRows,
	SMSDDBI_GetString,
	SMSDDBI_GetNumber,
	SMSDDBI_GetDate,
	SMSDDBI_GetBool,
	SMSDDBI_QuoteString,
};

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
