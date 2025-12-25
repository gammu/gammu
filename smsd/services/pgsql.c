/* (c) 2006 by Andrea Riciputi */
/* Copyright (c) 2009 - 2018 Michal Cihar <michal@cihar.com> */

#define _XOPEN_SOURCE
#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <time.h>

#include <gammu.h>

#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#ifdef WIN32
#include <windows.h>
#ifndef __GNUC__
#pragma comment(lib, "libpq.lib")
#endif
#endif

#include "../core.h"
#include "sql.h"

long long SMSDPgSQL_GetNumber(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	return atoi(PQgetvalue(res->pg.res, res->pg.iter, field));
}

time_t SMSDPgSQL_GetDate(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	const char *date;

	date = PQgetvalue(res->pg.res, res->pg.iter, field);
	return SMSDSQL_ParseDate(Config, date);
}

gboolean SMSDPgSQL_GetBool(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	const char *value;
	value = PQgetvalue(res->pg.res, res->pg.iter, field);
	return GSM_StringToBool(value);
}

const char *SMSDPgSQL_GetString(GSM_SMSDConfig * Config, SQL_result *res, unsigned int field)
{
	return PQgetvalue(res->pg.res, res->pg.iter, field);
}

static void SMSDPgSQL_LogError(GSM_SMSDConfig * Config, PGresult * Res)
{
	if (Res == NULL) {
		SMSD_Log(DEBUG_INFO, Config, "Error: %s", PQerrorMessage(Config->conn.pg));
	} else {
		SMSD_Log(DEBUG_INFO, Config, "Error: %s", PQresultErrorMessage(Res));
	}
}

/* Disconnects from a database */
void SMSDPgSQL_Free(GSM_SMSDConfig * Config)
{
	if (Config->conn.pg != NULL) {
		PQfinish(Config->conn.pg);
		Config->conn.pg = NULL;
	}
}

/* [Re]connects to database */
static GSM_Error SMSDPgSQL_Connect(GSM_SMSDConfig * Config)
{
	unsigned char buf[400];
	PGresult *rc;
	int Status;

	unsigned int port = 5432;
	char *pport;

	pport = strstr(Config->host, ":");
	if (pport) {
		*pport++ = '\0';
		port = atoi(pport);
	}

	sprintf(buf, "host = '%s' user = '%s' password = '%s' dbname = '%s' port = %d", Config->host, Config->user, Config->password, Config->database, port);

	SMSDPgSQL_Free(Config);
	Config->conn.pg = PQconnectdb(buf);
	if (PQstatus(Config->conn.pg) != CONNECTION_OK) {
		SMSD_Log(DEBUG_ERROR, Config, "Error connecting to database: %s", PQerrorMessage(Config->conn.pg));
		PQfinish(Config->conn.pg);
		return ERR_DB_CONNECT;
	}

	rc = PQexec(Config->conn.pg, "SET NAMES 'UTF8'");
	if ((rc == NULL) || ((Status = PQresultStatus(rc)) != PGRES_COMMAND_OK && (Status != PGRES_TUPLES_OK))) {
		SMSDPgSQL_LogError(Config, rc);
		return ERR_DB_CONNECT;
	}
	PQclear(rc);
	SMSD_Log(DEBUG_INFO, Config, "Connected to database: %s on %s. Server version: %d Protocol: %d",
		 PQdb(Config->conn.pg), PQhost(Config->conn.pg), PQserverVersion(Config->conn.pg), PQprotocolVersion(Config->conn.pg));

	return ERR_NONE;
}

void SMSDPgSQL_FreeResult(GSM_SMSDConfig * Config, SQL_result *res)
{
	PQclear(res->pg.res);
	res->pg.iter = -1;
}

int SMSDPgSQL_NextRow(GSM_SMSDConfig * Config, SQL_result *res)
{
	if(PQntuples(res->pg.res) > ++(res->pg.iter))
		return 1;
	else
		return 0;
}

static GSM_Error SMSDPgSQL_Query(GSM_SMSDConfig * Config, const char *query, SQL_result * Res)
{
	ExecStatusType Status = PGRES_COMMAND_OK;

	Res->pg.res = PQexec(Config->conn.pg, query);
	Res->pg.iter = -1;
	if ((Res->pg.res == NULL) || ((Status = PQresultStatus(Res->pg.res)) != PGRES_COMMAND_OK && (Status != PGRES_TUPLES_OK))) {
		SMSDPgSQL_LogError(Config, Res->pg.res);

		/* Check for reconnect */
		if ((Res->pg.res == NULL) || (Status == PGRES_FATAL_ERROR)) {
			/* Dirty hack */
			Res->pg.res = PQexec(Config->conn.pg, "SELECT 42");
			if (Res->pg.res != NULL)
				PQclear(Res->pg.res);
			if (PQstatus(Config->conn.pg) != CONNECTION_OK) {
				return ERR_DB_TIMEOUT;
			} else {
				return ERR_SQL;
			}
		}
	}
	return ERR_NONE;
}

/* Assume 2 * strlen(from) + 1 buffer in to */
char * SMSDPgSQL_QuoteString(GSM_SMSDConfig * Config, const char *from)
{
	char *to;
	int ret =0;
	to = malloc(strlen(from)*2+3);
	to[0] = '\'';
	to[1] = '\0';
#ifdef HAVE_PQESCAPESTRINGCONN
	PQescapeStringConn(Config->conn.pg, to+1, from, strlen(from), &ret);
#else
	PQescapeString(to+1, from, strlen(from));
#endif
	strcat(to, "'");
	return to;
}

unsigned long SMSDPgSQL_AffectedRows(GSM_SMSDConfig * Config, SQL_result *res)
{
	return atoi(PQcmdTuples(res->pg.res));
}

unsigned long long SMSDPgSQL_SeqID(GSM_SMSDConfig * Config, const char *seq_id)
{
	unsigned long id;
	char buff[100];
	PGresult *rc;
	int Status;

	snprintf(buff, sizeof(buff), "SELECT currval('\"%s\"')", seq_id);
	rc = PQexec(Config->conn.pg, buff);
	if ((rc == NULL) || ((Status = PQresultStatus(rc)) != PGRES_COMMAND_OK && (Status != PGRES_TUPLES_OK))) {
		SMSDPgSQL_LogError(Config, rc);
		return 0;
	}
	id = atoi(PQgetvalue(rc, 0, 0));
	PQclear(rc);
	return id;
}

struct GSM_SMSDdbobj SMSDPgSQL = {
	SMSDPgSQL_Connect,
	SMSDPgSQL_Query,
	SMSDPgSQL_Free,
	SMSDPgSQL_FreeResult,
	SMSDPgSQL_NextRow,
	SMSDPgSQL_SeqID,
	SMSDPgSQL_AffectedRows,
	SMSDPgSQL_GetString,
	SMSDPgSQL_GetNumber,
	SMSDPgSQL_GetDate,
	SMSDPgSQL_GetBool,
	SMSDPgSQL_QuoteString,
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
