/* (c) 2004 by Marcin Wiacek */
/* Copyright (c) 2009 - 2018 Michal Cihar <michal@cihar.com> */

#include <gammu.h>

#ifdef HAVE_MYSQL_MYSQL_H
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

#ifdef WIN32
#include <windows.h>
#ifndef __GNUC__
#pragma comment(lib, "mysqlclient.lib")
#endif
#endif

#include "../core.h"
#include "sql.h"

long long SMSDMySQL_GetNumber(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	return atoi(res->my.row[field]);
}

const char *SMSDMySQL_GetString(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	return res->my.row[field];
}

time_t SMSDMySQL_GetDate(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	const char *date;

	date = res->my.row[field];
	return SMSDSQL_ParseDate(Config, date);
}

gboolean SMSDMySQL_GetBool(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	const char *value = res->my.row[field];

	if(atoi(value) > 0){
		return TRUE;
	}
	return GSM_StringToBool(value);
}

const char *SMSMySQL_GetString(GSM_SMSDConfig * Config, SQL_result *res, unsigned int col)
{
	return res->my.row[col];
}

/* Disconnects from a database */
void SMSDMySQL_Free(GSM_SMSDConfig * Config)
{
	if (Config->conn.my != NULL) {
		mysql_close(Config->conn.my);
		free(Config->conn.my);
		Config->conn.my = NULL;
	}
}

static int SMSDMySQL_LogError(GSM_SMSDConfig * Config)
{
	int mysql_err;

	mysql_err = mysql_errno(Config->conn.my);

	SMSD_Log(DEBUG_ERROR, Config, "Error code: %d, Error: %s", mysql_err, mysql_error(Config->conn.my));

	return mysql_err;
}

static GSM_Error SMSDMySQL_Connect(GSM_SMSDConfig * Config)
{

	unsigned int port = 0;
	int error;
	char *pport;
	char *socketname = NULL;

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
	if (Config->conn.my == NULL) {
		Config->conn.my = malloc(sizeof(MYSQL));
		mysql_init(Config->conn.my);
	}
	if (Config->conn.my == NULL) {
		SMSD_Log(DEBUG_ERROR, Config, "MySQL allocation failed!");
		return ERR_DB_DRIVER;
	}
	if (!mysql_real_connect(Config->conn.my, Config->host, Config->user, Config->password, Config->database, port, socketname, 0)) {
		SMSD_Log(DEBUG_ERROR, Config, "Error connecting to database!");
		SMSDMySQL_LogError(Config);
		error = mysql_errno(Config->conn.my);
		if (error == 2006 || error == 2003 || error == 2002) { /* cant connect through socket */
			return ERR_DB_TIMEOUT;
		}
		return ERR_DB_CONNECT;
	}

	/* Try using utf8mb4 if MySQL server supports it */
	if (mysql_query(Config->conn.my, "SET NAMES utf8mb4;") != 0) {
		mysql_query(Config->conn.my, "SET NAMES utf8;");
	}
	SMSD_Log(DEBUG_INFO, Config, "Connected to Database: %s on %s", Config->database, Config->host);
	return ERR_NONE;
}

static GSM_Error SMSDMySQL_Query(GSM_SMSDConfig * Config, const char *query, SQL_result *res)
{
	int error;

	if (mysql_query(Config->conn.my, query) != 0) {
		SMSDMySQL_LogError(Config);
		error = mysql_errno(Config->conn.my);
		if (error == 2006 || error == 2013 || error == 2012) { /* connection lost */
			return ERR_DB_TIMEOUT;
		}
		return ERR_SQL;
	}

	res->my.res = mysql_store_result(Config->conn.my);
	res->my.row = NULL;
	res->my.con = Config->conn.my;

	return ERR_NONE;
}

/* free mysql results */
void SMSDMySQL_FreeResult(GSM_SMSDConfig * Config, SQL_result *res)
{
	mysql_free_result(res->my.res);
}

/* set pointer to next row */
int SMSDMySQL_NextRow(GSM_SMSDConfig * Config, SQL_result *res)
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
char * SMSDMySQL_QuoteString(GSM_SMSDConfig * Config, const char *string)
{
	char *buff;
	int len = strlen(string);
	buff = malloc(len*2+3);

	if (buff == NULL) {
		SMSD_Log(DEBUG_ERROR, Config, "String allocation for escaping failed!");
		return NULL;
	}

	buff[0] = '\'';
	buff[1] = '\0';
	mysql_real_escape_string(Config->conn.my, buff+1, string, len);
	strcat(buff, "'");
	return buff;
}

/* LAST_INSERT_ID */
unsigned long long SMSDMySQL_SeqID(GSM_SMSDConfig * Config, const char *dummy)
{
	return mysql_insert_id(Config->conn.my);
}

unsigned long SMSDMySQL_AffectedRows(GSM_SMSDConfig * Config, SQL_result *res)
{
	return mysql_affected_rows(res->my.con);
}

struct GSM_SMSDdbobj SMSDMySQL = {
	SMSDMySQL_Connect,
	SMSDMySQL_Query,
	SMSDMySQL_Free,
	SMSDMySQL_FreeResult,
	SMSDMySQL_NextRow,
	SMSDMySQL_SeqID,
	SMSDMySQL_AffectedRows,
	SMSDMySQL_GetString,
	SMSDMySQL_GetNumber,
	SMSDMySQL_GetDate,
	SMSDMySQL_GetBool,
	SMSDMySQL_QuoteString,
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
