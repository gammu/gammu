/* (c) 2006 by Andrea Riciputi */

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

long long SMSDPgSQL_GetNumber(SQL_result rc, unsigned int field)
{
	return atoi(PQgetvalue(rc.pg.res, rc.pg.iter, field));
}

time_t SMSDPgSQL_GetDate(SQL_result rc, unsigned int field)
{
	const char *date;
	struct tm t;
	int ret;
	
	date = PQgetvalue(rc.pg.res, rc.pg.iter, field);
	/* windows has not strptime() */
	ret = sscanf(date, "%u-%u-%u %u:%u:%u", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec);
	t.tm_isdst = -1;
#ifdef HAVE_STRUCT_TM_TM_ZONE
	t.tm_gmtoff = 0;
	t.tm_zone = NULL;
#endif
	if (ret == 6) {
		return mktime(&t);
	}
	SMSDPgSQL.error = "Failed to process date!";
	return -1;	
}

gboolean SMSDPgSQL_GetBool(SQL_result rc, unsigned int field)
{
	const char *value;
	value = PQgetvalue(rc.pg.res, rc.pg.iter, field);
	if (strcasecmp(value, "yes") == 0 || strcasecmp(value, "TRUE") == 0 || strcasecmp(value, "y") == 0 || strcasecmp(value, "t") == 0) {
		return TRUE;
	}
	if (strcasecmp(value, "no") == 0 || strcasecmp(value, "FALSE") == 0 || strcasecmp(value, "n") == 0 || strcasecmp(value, "f") == 0) {
		return FALSE;
	}
	return -1;
}

const char *SMSDPgSQL_GetString(SQL_result rc, unsigned int field)
{
	return PQgetvalue(rc.pg.res, rc.pg.iter, field);
}

/* Disconnects from a database */
void SMSDPgSQL_Free(SQL_conn *conn)
{
	if (conn->pg != NULL) {
		PQfinish(conn->pg);
		conn->pg = NULL;
	}
}

/* [Re]connects to database */
static SQL_Error SMSDPgSQL_Connect(GSM_SMSDConfig * Config)
{
	unsigned char buf[400];
	PGresult *Res;
	struct GSM_SMSDdbobj *db = (struct GSM_SMSDdbobj *)Config->db;

	unsigned int port = 5432;
	char *pport;

	pport = strstr(Config->host, ":");
	if (pport) {
		*pport++ = '\0';
		port = atoi(pport);
	}

	sprintf(buf, "host = '%s' user = '%s' password = '%s' dbname = '%s' port = %d", Config->host, Config->user, Config->password, Config->database, port);

	SMSDPgSQL_Free(&db->conn);
	db->conn.pg = PQconnectdb(buf);
	if (PQstatus(db->conn.pg) != CONNECTION_OK) {
		SMSD_Log(DEBUG_ERROR, Config, "Error connecting to database: %s", PQerrorMessage(db->conn.pg));
		PQfinish(db->conn.pg);
		return SQL_FAIL;
	}

	Res = PQexec(db->conn.pg, "SET NAMES UTF8");
	PQclear(Res);
	SMSD_Log(DEBUG_INFO, Config, "Connected to database: %s on %s. Server version: %d Protocol: %d",
		 PQdb(db->conn.pg), PQhost(db->conn.pg), PQserverVersion(db->conn.pg), PQprotocolVersion(db->conn.pg));

	return SQL_OK;
}

void SMSDPgSQL_FreeResult(SQL_result res)
{
	PQclear(res.pg.res);
	res.pg.iter = -1;
}

int SMSDPgSQL_NextRow(SQL_result *res)
{
	if(PQntuples(res->pg.res) < ++res->pg.iter)
		return 1;
	else
		return 0;
}

static void SMSDPgSQL_LogError(GSM_SMSDConfig * Config, PGresult * Res)
{
	struct GSM_SMSDdbobj *db = Config->db;
	if (Res == NULL) {
		SMSD_Log(DEBUG_INFO, Config, "Error: %s", PQerrorMessage(db->conn.pg));
	} else {
		SMSD_Log(DEBUG_INFO, Config, "Error: %s", PQresultErrorMessage(Res));
	}
}

static SQL_Error SMSDPgSQL_Query(GSM_SMSDConfig * Config, const char *query, SQL_result * Res)
{
	struct GSM_SMSDdbobj *db = Config->db;
	ExecStatusType Status = PGRES_COMMAND_OK;

	Res->pg.res = PQexec(db->conn.pg, query);
	Res->pg.iter = -1;
	if ((Res->pg.res == NULL) || ((Status = PQresultStatus(Res->pg.res)) != PGRES_COMMAND_OK && (Status != PGRES_TUPLES_OK))) {
		SMSDPgSQL_LogError(Config, Res->pg.res);

		/* Check for reconnect */
		if ((Res->pg.res == NULL) || (Status == PGRES_FATAL_ERROR)) {
			/* Dirty hack */
			Res->pg.res = PQexec(db->conn.pg, "SELECT 42");
			if (Res->pg.res != NULL)
				PQclear(Res->pg.res);
			if (PQstatus(db->conn.pg) != CONNECTION_OK) {
				return SQL_TIMEOUT;
			} else {
				return SQL_FAIL;
			}
		}
	} else {
		return SQL_OK;
	}
	return SQL_OK;
}

/* Assume 2 * strlen(from) + 1 buffer in to */
char * SMSDPgSQL_QuoteString(SQL_conn *conn, const char *from)
{
	char *to;
	int ret =0;
	to = malloc(strlen(from)*2+3);
	to[0] = '\'';
	to[1] = '\0';
#ifdef HAVE_PQESCAPESTRINGCONN
	PQescapeStringConn(conn->pg, to+1, from, strlen(from), &ret);
#else
	PQescapeString(to+1, from, strlen(from));
#endif
	strcat(to, "'");
	return to;
}

unsigned long SMSDPgSQL_NumRows(SQL_result Res)
{
	return PQntuples(Res.pg.res); 
}


unsigned long SMSDPgSQL_AffectedRows(SQL_result Res)
{
	return atoi(PQcmdTuples(Res.pg.res));
}

unsigned long long SMSDPgSQL_SeqID(SQL_conn *conn, const char *seq_id)
{
	unsigned long id;
	char buff[100];
	PGresult *rc; 
	int Status;

	snprintf(buff, sizeof(buff), "SELECT currval('%s')", seq_id);
	rc = PQexec(conn->pg, buff);
	if ((rc == NULL) || ((Status = PQresultStatus(rc)) != PGRES_COMMAND_OK && (Status != PGRES_TUPLES_OK))) {
		return 0;
	}
	id = atoi(PQgetvalue(rc, 0, 0));
	PQclear(rc);
	return id;
}

struct GSM_SMSDdbobj SMSDPgSQL = {
	"pgsql",
	SMSDPgSQL_Connect,
	SMSDPgSQL_Query,
	SMSDPgSQL_Free,
	SMSDPgSQL_FreeResult,
	SMSDPgSQL_NextRow,
	SMSDPgSQL_SeqID,
	SMSDPgSQL_AffectedRows,
	SMSDPgSQL_NumRows,
	SMSDPgSQL_GetString,
	SMSDPgSQL_GetNumber,
	SMSDPgSQL_GetDate,
	SMSDPgSQL_GetBool,
	SMSDPgSQL_QuoteString,
	NULL
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
