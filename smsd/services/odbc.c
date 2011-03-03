/**
 * ODBC database backend
 *
 * Part of Gammu project
 *
 * Copyright (C) 2011 Michal Čihař
 *
 * Licensed under GNU GPL version 2 or later
 */

#include <gammu.h>

#ifdef WIN32
#include <windows.h>
#ifndef __GNUC__
#pragma comment(lib, "libodbc32.lib")
#endif
#endif

#include <stdio.h>
#include <sql.h>
#include <sqlext.h>

#include "../core.h"
#include "sql.h"
#include "sql-core.h"

long long SMSDODBC_GetNumber(GSM_SMSDConfig * Config, SQL_result rc, unsigned int field)
{
	SQLRETURN ret;
	SQLINTEGER value;

	ret = SQLGetData(rc.odbc, field + 1, SQL_C_SLONG, &value, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) {
		return -1;
	}
	return value;
}

time_t SMSDODBC_GetDate(GSM_SMSDConfig * Config, SQL_result rc, unsigned int field)
{
	struct tm timestruct;
	SQL_TIMESTAMP_STRUCT sqltime;
	SQLRETURN ret;

	ret = SQLGetData(rc.odbc, field + 1, SQL_C_TYPE_TIMESTAMP, &sqltime, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) {
		return -1;
	}

	tzset();

#ifdef HAVE_DAYLIGHT
	timestruct.tm_isdst	= daylight;
#else
	timestruct.tm_isdst	= -1;
#endif
#ifdef HAVE_STRUCT_TM_TM_ZONE
	/* No sqltime zone information */
	timestruct.tm_gmtoff = timezone;
	timestruct.tm_zone = *tzname;
#endif

	timestruct.tm_year = sqltime.year;
	timestruct.tm_mon = sqltime.month;
	timestruct.tm_mday = sqltime.day;
	timestruct.tm_hour = sqltime.hour;
	timestruct.tm_min = sqltime.minute;
	timestruct.tm_sec = sqltime.second;

	return mktime(&timestruct);
}

const char *SMSDODBC_GetString(GSM_SMSDConfig * Config, SQL_result rc, unsigned int field)
{
	SQLLEN size;
	SQLRETURN ret;

	ret = SQLGetData(rc.odbc, field + 1, SQL_C_CHAR, NULL, 0, &size);
	if (!SQL_SUCCEEDED(ret)) {
		return NULL;
	}

	Config->conn.odbc.retstr = realloc(Config->conn.odbc.retstr, size + 1);

	ret = SQLGetData(rc.odbc, field + 1, SQL_C_CHAR, Config->conn.odbc.retstr, size + 1, &size);
	if (!SQL_SUCCEEDED(ret)) {
		return NULL;
	}

	return Config->conn.odbc.retstr;
}

gboolean SMSDODBC_GetBool(GSM_SMSDConfig * Config, SQL_result rc, unsigned int field)
{
	long long intval;
	const char * charval;

	intval = SMSDODBC_GetNumber(Config, rc, field);
	if (intval == -1) {
		charval = SMSDODBC_GetString(Config, rc, field);
		return GSM_StringToBool(charval);
	}
	return intval ? TRUE : FALSE;
}

static void SMSDODBC_LogError(GSM_SMSDConfig * Config, SQLSMALLINT handle_type, SQLHANDLE handle, const char *message)
{
	SQLINTEGER	 i = 0;
	SQLINTEGER	 native;
	SQLCHAR	 state[ 7 ];
	SQLCHAR	 text[256];
	SQLSMALLINT	 len;
	SQLRETURN	 ret;

	SMSD_Log(DEBUG_ERROR, Config, "%s, ODBC diagnostics:", message);

	do {
		ret = SQLGetDiagRec(handle_type, handle, ++i, state, &native, text, sizeof(text), &len );
		if (SQL_SUCCEEDED(ret)) {
			SMSD_Log(DEBUG_ERROR, Config, "%s:%ld:%ld:%s\n", state, (long)i, (long)native, text);
		}
	} while (ret == SQL_SUCCESS);
}

/* Disconnects from a database */
void SMSDODBC_Free(GSM_SMSDConfig * Config)
{
	SQLDisconnect(Config->conn.odbc.dbc);
	SQLFreeHandle(SQL_HANDLE_ENV, Config->conn.odbc.env);
	if (Config->conn.odbc.retstr != NULL) {
		free(Config->conn.odbc.retstr);
		Config->conn.odbc.retstr = NULL;
	}
}

/* Connects to database */
static SQL_Error SMSDODBC_Connect(GSM_SMSDConfig * Config)
{
	SQLRETURN ret;

	Config->conn.odbc.retstr = NULL;

	ret = SQLAllocHandle (SQL_HANDLE_ENV, SQL_NULL_HANDLE, &Config->conn.odbc.env);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, SQL_HANDLE_ENV, Config->conn.odbc.env, "SQLAllocHandle failed");
		return SQL_FAIL;
	}

	ret = SQLSetEnvAttr (Config->conn.odbc.env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, SQL_HANDLE_ENV, Config->conn.odbc.env, "SQLSetEnvAttr failed");
		return SQL_FAIL;
	}

	ret = SQLAllocHandle (SQL_HANDLE_DBC, Config->conn.odbc.env, &Config->conn.odbc.dbc);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, SQL_HANDLE_ENV, Config->conn.odbc.env, "SQLAllocHandle failed");
		return SQL_FAIL;
	}

	ret = SQLConnect(Config->conn.odbc.dbc,
			  (SQLCHAR*)Config->host, SQL_NTS,
			  (SQLCHAR*)Config->user, SQL_NTS,
			  (SQLCHAR*)Config->password, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, SQL_HANDLE_DBC, Config->conn.odbc.dbc, "SQLAllocHandle failed");
		return SQL_FAIL;
	}

	return SQL_OK;
}

static SQL_Error SMSDODBC_Query(GSM_SMSDConfig * Config, const char *query, SQL_result * res)
{
	SQLRETURN ret;

	ret = SQLAllocHandle(SQL_HANDLE_STMT, Config->conn.odbc.dbc, &res->odbc);
	if (!SQL_SUCCEEDED(ret)) {
		return SQL_FAIL;
	}

	ret = SQLExecDirect (res->odbc, (SQLCHAR*)query, SQL_NTS);
	if (SQL_SUCCEEDED(ret)) {
		return SQL_OK;
	}

	return SQL_FAIL;
}

/* free sql results */
void SMSDODBC_FreeResult(GSM_SMSDConfig * Config, SQL_result res)
{
	SQLFreeHandle (SQL_HANDLE_STMT, res.odbc);
}

/* set pointer to next row */
int SMSDODBC_NextRow(GSM_SMSDConfig * Config, SQL_result *res)
{
	SQLRETURN ret;

	ret = SQLFetch(res->odbc);

	return SQL_SUCCEEDED(ret);
}

/* quote strings */
char * SMSDODBC_QuoteString(GSM_SMSDConfig * Config, SQL_conn *conn, const char *string)
{
	char *encoded_text = NULL;
	/* TODO: implement escaping */
	encoded_text = strdup(string);
	return encoded_text;
}

/* LAST_INSERT_ID */
unsigned long long SMSDODBC_SeqID(GSM_SMSDConfig * Config, const char *id)
{
	/* TODO */
	return 0;
}

unsigned long SMSDODBC_AffectedRows(GSM_SMSDConfig * Config, SQL_result res)
{
	SQLRETURN ret;
	SQLLEN count;

	ret = SQLRowCount (res.odbc, &count);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, SQL_HANDLE_DBC, Config->conn.odbc.dbc, "SQLRowCount failed");
		return 0;
	}
	return count;
}

unsigned long SMSDODBC_NumRows(GSM_SMSDConfig * Config, SQL_result Res)
{
	SQLRETURN ret;

	/* FIXME: This is hack relying on sql.c not using anything else than 0 and 1 as values */
	ret = SQLMoreResults(Res.odbc);
	if (SQL_SUCCEEDED(ret)) {
		return 1;
	}
	return 0;
}

struct GSM_SMSDdbobj SMSDODBC = {
	"",
	SMSDODBC_Connect,
	SMSDODBC_Query,
	SMSDODBC_Free,
	SMSDODBC_FreeResult,
	SMSDODBC_NextRow,
	SMSDODBC_SeqID,
	SMSDODBC_AffectedRows,
	SMSDODBC_NumRows,
	SMSDODBC_GetString,
	SMSDODBC_GetNumber,
	SMSDODBC_GetDate,
	SMSDODBC_GetBool,
	SMSDODBC_QuoteString,
};

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
