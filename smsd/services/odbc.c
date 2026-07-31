/**
 * ODBC database backend
 *
 * Part of Gammu project
 *
 * Copyright (C) 2011 - 2018 Michal Čihař
 *
 * Licensed under GNU GPL version 2 or later
 */

#include <gammu.h>

#include <ctype.h>
#include <errno.h>

#ifdef WIN32
#include <windows.h>
#ifndef __GNUC__
#pragma comment(lib, "odbc32.lib")
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>

#include "../../libgammu/misc/string.h"
#include "../core.h"
#include "sql.h"
#include "sql-core.h"

static void SMSDODBC_LogError(GSM_SMSDConfig * Config, SQLRETURN origret, SQLSMALLINT handle_type, SQLHANDLE handle, const char *message)
{
	SQLSMALLINT	 i = 0;
	SQLINTEGER	 native;
	SQLCHAR	 state[ 7 ];
	SQLCHAR	 text[256];
	SQLSMALLINT	 len;
	SQLRETURN	 ret;

	SMSD_Log(DEBUG_ERROR, Config, "%s, Code = %d, ODBC diagnostics:", message, (int)origret);

	do {
		ret = SQLGetDiagRec(handle_type, handle, ++i, state, &native, text, sizeof(text), &len );
		if (SQL_SUCCEEDED(ret)) {
			SMSD_Log(DEBUG_ERROR, Config, "%s:%ld:%ld:%s\n", state, (long)i, (long)native, text);
		}
	} while (ret == SQL_SUCCESS);
}

gboolean SMSDODBC_ParseSignedInteger(const char *text, long long *value)
{
	char *end;
	long long parsed;

	if (text == NULL || text[0] == '\0' || isspace((unsigned char)text[0]) || value == NULL) {
		return FALSE;
	}
	errno = 0;
	parsed = strtoll(text, &end, 10);
	if (errno == ERANGE || end == text || end[0] != '\0') {
		return FALSE;
	}
	*value = parsed;
	return TRUE;
}

gboolean SMSDODBC_ParseUnsignedInteger(const char *text, unsigned long long *value)
{
	char *end;
	unsigned long long parsed;

	if (text == NULL || text[0] == '\0' || isspace((unsigned char)text[0]) ||
		text[0] == '-' || value == NULL) {
		return FALSE;
	}
	errno = 0;
	parsed = strtoull(text, &end, 10);
	if (errno == ERANGE || end == text || end[0] != '\0') {
		return FALSE;
	}
	*value = parsed;
	return TRUE;
}

static gboolean SMSDODBC_GetIntegerText(GSM_SMSDConfig *Config, SQLHSTMT stmt,
	SQLUSMALLINT field, char *buffer, size_t buffer_size)
{
	SQLLEN length;
	SQLRETURN ret;

	/* Oracle ODBC does not support SQL_C_SBIGINT or SQL_C_UBIGINT. */
	buffer[0] = '\0';
	ret = SQLGetData(stmt, field, SQL_C_CHAR, buffer, (SQLLEN)buffer_size, &length);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, ret, SQL_HANDLE_STMT, stmt, "SQLGetData(integer) failed");
		return FALSE;
	}
	if (length == SQL_NULL_DATA) {
		SMSD_Log(DEBUG_ERROR, Config, "Can not convert NULL to integer");
		return FALSE;
	}
	if (length == SQL_NO_TOTAL || length < 0 || (size_t)length >= buffer_size) {
		SMSD_Log(DEBUG_ERROR, Config, "ODBC integer value was truncated");
		return FALSE;
	}
	return TRUE;
}

long long SMSDODBC_GetNumber(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	char text[32];
	long long value;

	if (!SMSDODBC_GetIntegerText(Config, res->odbc, field + 1, text, sizeof(text))) {
		return -1;
	}
	if (!SMSDODBC_ParseSignedInteger(text, &value)) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to parse ODBC integer value: %s", text);
		return -1;
	}
	return value;
}

time_t SMSDODBC_GetDate(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	SQL_TIMESTAMP_STRUCT sqltime;
	GSM_DateTime DT;
	SQLRETURN ret;

	ret = SQLGetData(res->odbc, field + 1, SQL_C_TYPE_TIMESTAMP, &sqltime, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, ret, SQL_HANDLE_STMT, res->odbc, "SQLGetData(timestamp) failed");
		return -1;
	}

	DT.Year = sqltime.year;
	DT.Month = sqltime.month;
	DT.Day = sqltime.day;
	DT.Hour = sqltime.hour;
	DT.Minute = sqltime.minute;
	DT.Second = sqltime.second;

	return Fill_Time_T(DT);
}

const char *SMSDODBC_GetString(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	SQLLEN size;
	SQLRETURN ret;
	char shortbuffer[1];

	if (field > SMSD_ODBC_MAX_RETURN_STRINGS) {
		SMSD_Log(DEBUG_ERROR, Config, "Field %d returning NULL, too many fields!", field);
		return NULL;
	}

	/* Figure out string length */
	ret = SQLGetData(res->odbc, field + 1, SQL_C_CHAR, shortbuffer, 0, &size);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, ret, SQL_HANDLE_STMT, res->odbc, "SQLGetData(string,0) failed");
		return NULL;
	}

	/* Did not we get NULL? */
	if (size == SQL_NULL_DATA) {
		SMSD_Log(DEBUG_SQL, Config, "Field %d returning NULL", field);
		return NULL;
	}

	/* Allocate string */
	Config->conn.odbc.retstr[field] = realloc(Config->conn.odbc.retstr[field], size + 1);
	if (Config->conn.odbc.retstr[field] == NULL) {
		SMSD_Log(DEBUG_ERROR, Config, "Field %d returning NULL, failed to allocate %ld bytes of memory", field, (long)(size + 1));
		return NULL;
	}

	/* Actually grab result from database */
	ret = SQLGetData(res->odbc, field + 1, SQL_C_CHAR, Config->conn.odbc.retstr[field], size + 1, &size);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, ret, SQL_HANDLE_STMT, res->odbc, "SQLGetData(string) failed");
		return NULL;
	}

	SMSD_Log(DEBUG_SQL, Config, "Field %d returning string \"%s\"", field, Config->conn.odbc.retstr[field]);

	return Config->conn.odbc.retstr[field];
}

gboolean SMSDODBC_GetBool(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	SQLLEN size;
	SQLRETURN ret;
	char value[6];

	/*
	 * Fetch booleans as text so both native bit columns ("0"/"1") and
	 * textual SQL enums ("false"/"true", "no"/"yes") are interpreted
	 * consistently. Some ODBC drivers successfully convert textual values
	 * to SQL_C_BIT as zero, making a type-conversion fallback unreliable.
	 *
	 * Do not use SMSDODBC_GetString here. Its zero-length size probe can
	 * fail with 22003 when a numeric or bit value is converted to text.
	 */
	ret = SQLGetData(res->odbc, field + 1, SQL_C_CHAR, value, sizeof(value), &size);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, ret, SQL_HANDLE_STMT, res->odbc, "SQLGetData(boolean) failed");
		return FALSE;
	}
	if (size == SQL_NULL_DATA) {
		SMSD_Log(DEBUG_SQL, Config, "Field %d returning NULL", field);
		return FALSE;
	}

	SMSD_Log(DEBUG_SQL, Config, "Field %d returning boolean \"%s\"", field, value);
	return GSM_StringToBool(value);
}

/* Disconnects from a database */
void SMSDODBC_Free(GSM_SMSDConfig * Config)
{
	int field;

	SQLDisconnect(Config->conn.odbc.dbc);
	SQLFreeHandle(SQL_HANDLE_ENV, Config->conn.odbc.env);

	for (field = 0; field < SMSD_ODBC_MAX_RETURN_STRINGS; field++) {
		if (Config->conn.odbc.retstr[field] != NULL) {
			free(Config->conn.odbc.retstr[field]);
			Config->conn.odbc.retstr[field] = NULL;
		}
	}
}

/* Connects to database */
static GSM_Error SMSDODBC_Connect(GSM_SMSDConfig * Config)
{
	SQLRETURN ret;
	int field;
	char driver_name[1000];
	SQLSMALLINT len;

	for (field = 0; field < SMSD_ODBC_MAX_RETURN_STRINGS; field++) {
		Config->conn.odbc.retstr[field] = NULL;
	}

	ret = SQLAllocHandle (SQL_HANDLE_ENV, SQL_NULL_HANDLE, &Config->conn.odbc.env);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, ret, SQL_HANDLE_ENV, Config->conn.odbc.env, "SQLAllocHandle(ENV) failed");
		return ERR_DB_DRIVER;
	}

	ret = SQLSetEnvAttr (Config->conn.odbc.env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, ret, SQL_HANDLE_ENV, Config->conn.odbc.env, "SQLSetEnvAttr failed");
		return ERR_DB_CONFIG;
	}

	ret = SQLAllocHandle (SQL_HANDLE_DBC, Config->conn.odbc.env, &Config->conn.odbc.dbc);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, ret, SQL_HANDLE_ENV, Config->conn.odbc.env, "SQLAllocHandle(DBC) failed");
		return ERR_DB_CONFIG;
	}

	ret = SQLConnect(Config->conn.odbc.dbc,
			  (SQLCHAR*)Config->host, SQL_NTS,
			  (SQLCHAR*)Config->user, SQL_NTS,
			  (SQLCHAR*)Config->password, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, ret, SQL_HANDLE_DBC, Config->conn.odbc.dbc, "SQLConnect failed");
		return ERR_DB_CONNECT;
	}

	ret = SQLGetInfo(Config->conn.odbc.dbc, SQL_DRIVER_NAME, driver_name, sizeof(driver_name), &len);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, ret, SQL_HANDLE_DBC, Config->conn.odbc.dbc, "SQLGetInfo failed");
		return ERR_DB_CONNECT;
	} else{
		SMSD_Log(DEBUG_NOTICE, Config, "Connected to driver %s", driver_name);
	}


	return ERR_NONE;
}

static GSM_Error SMSDODBC_Query(GSM_SMSDConfig * Config, const char *query, SQL_result * res)
{
	SQLRETURN ret;

	ret = SQLAllocHandle(SQL_HANDLE_STMT, Config->conn.odbc.dbc, &res->odbc);
	if (!SQL_SUCCEEDED(ret)) {
		return ERR_SQL;
	}

	ret = SQLExecDirect (res->odbc, (SQLCHAR*)query, SQL_NTS);
	/*
	 * If SQLExecDirect executes a searched update, insert, or delete
	 * statement that does not affect any rows at the data source, the call
	 * to SQLExecDirect returns SQL_NO_DATA.
	 */
	if (SQL_SUCCEEDED(ret) || ret == SQL_NO_DATA) {
		return ERR_NONE;
	}

	SMSDODBC_LogError(Config, ret, SQL_HANDLE_STMT, res->odbc, "SQLExecDirect failed");
	return ERR_SQL;
}

/* free sql results */
void SMSDODBC_FreeResult(GSM_SMSDConfig * Config, SQL_result *res)
{
	SQLFreeHandle (SQL_HANDLE_STMT, res->odbc);
}

/* set pointer to next row */
int SMSDODBC_NextRow(GSM_SMSDConfig * Config, SQL_result *res)
{
	SQLRETURN ret;

	ret = SQLFetch(res->odbc);

	if (!SQL_SUCCEEDED(ret)) {
		if (ret != SQL_NO_DATA) {
			SMSDODBC_LogError(Config, ret, SQL_HANDLE_STMT, res->odbc, "SQLFetch failed");
		}
		return 0;
	}
	return 1;
}

static size_t SMSDODBC_OracleEscapedSize(const char *string)
{
	size_t escaped_size = 0;

	while (*string != '\0') {
		escaped_size += (*string == '\'') ? 2 : 1;
		string++;
	}
	return escaped_size;
}

static size_t SMSDODBC_UTF8CharacterLength(const char *string, size_t remaining)
{
	const unsigned char first = (unsigned char)string[0];
	size_t length;

	if ((first & 0x80) == 0) {
		length = 1;
	} else if ((first & 0xe0) == 0xc0) {
		length = 2;
	} else if ((first & 0xf0) == 0xe0) {
		length = 3;
	} else if ((first & 0xf8) == 0xf0) {
		length = 4;
	} else {
		length = 1;
	}
	return length <= remaining ? length : 1;
}

static char *SMSDODBC_QuoteOracleCLOB(const char *string)
{
	const size_t chunk_limit = 3900;
	const size_t length = strlen(string);
	const size_t escaped_size = SMSDODBC_OracleEscapedSize(string);
	const size_t max_chunks = (escaped_size / chunk_limit) + 1;
	char *quoted, *output;
	size_t position = 0;

	quoted = malloc((length * 2) + (max_chunks * 24) + 4);
	if (quoted == NULL) {
		return NULL;
	}
	output = quoted;
	memcpy(output, "(TO_CLOB('", 10);
	output += 10;

	while (position < length) {
		size_t chunk_size = 0;

		while (position < length) {
			size_t character_length = SMSDODBC_UTF8CharacterLength(
				string + position, length - position);
			size_t character_size = character_length;

			if (character_length == 1 && string[position] == '\'') {
				character_size++;
			}
			if (chunk_size != 0 && chunk_size + character_size > chunk_limit) {
				break;
			}
			if (character_length == 1 && string[position] == '\'') {
				*output++ = '\'';
			}
			memcpy(output, string + position, character_length);
			output += character_length;
			position += character_length;
			chunk_size += character_size;
		}

		memcpy(output, "')", 2);
		output += 2;
		if (position < length) {
			memcpy(output, " || TO_CLOB('", 13);
			output += 13;
		}
	}
	*output++ = ')';
	*output = '\0';
	return quoted;
}

/* quote strings */
char * SMSDODBC_QuoteString(GSM_SMSDConfig * Config, const char *string)
{
	char *encoded_text = NULL;
	size_t i, len, pos = 0;
	char quote = '"';
	gboolean sql_standard_escaping = FALSE;

	const char *driver_name;

	if (Config->sql != NULL) {
		driver_name = Config->sql;
	} else {
		driver_name = Config->driver;
	}

	if (strcasecmp(driver_name, "mssql") == 0 ||
			strcasecmp(driver_name, "oracle") == 0) {
		quote = '\'';
		sql_standard_escaping = TRUE;
	} else if (strcasecmp(driver_name, "mysql") == 0 ||
			strcasecmp(driver_name, "native_mysql") == 0 ||
			strcasecmp(driver_name, "pgsql") == 0 ||
			strcasecmp(driver_name, "native_pgsql") == 0 ||
			strncasecmp(driver_name, "sqlite", 6) == 0 ||
			strncasecmp(driver_name, "freetds", 6) == 0 ||
			strcasecmp(Config->driver, "access") == 0) {
		quote = '\'';
	}
	if (strcasecmp(driver_name, "oracle") == 0 &&
			SMSDODBC_OracleEscapedSize(string) > 4000) {
		return SMSDODBC_QuoteOracleCLOB(string);
	}

	len = strlen(string);

	encoded_text = (char *)malloc((len * 2) + 3);
	encoded_text[pos++] = quote;
	for (i = 0; i < len; i++) {
		if (string[i] == quote && sql_standard_escaping) {
			encoded_text[pos++] = quote;
		} else if (!sql_standard_escaping && (string[i] == quote || string[i] == '\\')) {
			encoded_text[pos++] = '\\';
		}
		encoded_text[pos++] = string[i];
	}
	encoded_text[pos++] = quote;
	encoded_text[pos] = '\0';
	return encoded_text;
}

const char *SMSDODBC_SeqIDQuery(GSM_SMSDConfig * Config, const char *id)
{
	const char *driver_name;
	const unsigned char *position;
	static char query[256];

	driver_name = Config->sql == NULL ? Config->driver : Config->sql;
	if (strcasecmp(driver_name, "oracle") != 0) {
		return "SELECT @@IDENTITY";
	}

	if (id == NULL || id[0] == '\0') {
		return NULL;
	}
	for (position = (const unsigned char *)id; *position != '\0'; position++) {
		if (!isalnum(*position) && *position != '_') {
			return NULL;
		}
	}

	snprintf(query, sizeof(query), "SELECT %s.CURRVAL FROM dual", id);
	return query;
}

/* LAST_INSERT_ID */
unsigned long long SMSDODBC_SeqID(GSM_SMSDConfig * Config, const char *id)
{
	SQLRETURN ret;
	SQLHSTMT stmt;
	char text[32];
	unsigned long long value;
	const char *query;

	query = SMSDODBC_SeqIDQuery(Config, id);
	if (query == NULL) {
		return 0;
	}

	ret = SQLAllocHandle(SQL_HANDLE_STMT, Config->conn.odbc.dbc, &stmt);
	if (!SQL_SUCCEEDED(ret)) {
		return 0;
	}

	ret = SQLExecDirect (stmt, (SQLCHAR*)query, SQL_NTS);
	if (!SQL_SUCCEEDED(ret)) {
		SQLFreeHandle (SQL_HANDLE_STMT, stmt);
		return 0;
	}

	ret = SQLFetch(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		SQLFreeHandle (SQL_HANDLE_STMT, stmt);
		return 0;
	}

	if (!SMSDODBC_GetIntegerText(Config, stmt, 1, text, sizeof(text))) {
		SQLFreeHandle (SQL_HANDLE_STMT, stmt);
		return 0;
	}
	if (!SMSDODBC_ParseUnsignedInteger(text, &value)) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to parse ODBC sequence value: %s", text);
		SQLFreeHandle (SQL_HANDLE_STMT, stmt);
		return 0;
	}
	SQLFreeHandle (SQL_HANDLE_STMT, stmt);

	return value;
}

unsigned long SMSDODBC_AffectedRows(GSM_SMSDConfig * Config, SQL_result *res)
{
	SQLRETURN ret;
	SQLLEN count;

	ret = SQLRowCount (res->odbc, &count);
	if (!SQL_SUCCEEDED(ret)) {
		SMSDODBC_LogError(Config, ret, SQL_HANDLE_DBC, Config->conn.odbc.dbc, "SQLRowCount failed");
		return 0;
	}
	return (unsigned long)count;
}

struct GSM_SMSDdbobj SMSDODBC = {
	SMSDODBC_Connect,
	SMSDODBC_Query,
	SMSDODBC_Free,
	SMSDODBC_FreeResult,
	SMSDODBC_NextRow,
	SMSDODBC_SeqID,
	SMSDODBC_AffectedRows,
	SMSDODBC_GetString,
	SMSDODBC_GetNumber,
	SMSDODBC_GetDate,
	SMSDODBC_GetBool,
	SMSDODBC_QuoteString,
};

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
