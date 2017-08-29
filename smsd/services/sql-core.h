/* drivers for sql service
 *
 * MySQL (C) 2004 by Marcin Wiacek
 * PostgreSQL (C) 2006 by Andrea Riciputi
 * DBI (C) 2009 by Michal Čihař
 *
 */

#ifndef __sql_core_h_
#define __sql_core_h_

#ifdef WIN32
#  include <winsock2.h>
#endif

#ifdef HAVE_MYSQL_MYSQL_H
#include <mysql.h>
#include <mysqld_error.h>
#endif

#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
#  include <libpq-fe.h>
#endif

#ifdef LIBDBI_FOUND
#include <dbi/dbi.h>
#endif

#ifdef HAVE_SHM
#include <sys/types.h>
#endif

#ifdef ODBC_FOUND
#include <sql.h>
#include <sqlext.h>
#endif

/* sql result structures */
typedef union {
#ifdef LIBDBI_FOUND
	dbi_result dbi;
#endif
#ifdef HAVE_MYSQL_MYSQL_H
	struct __mysql {
		MYSQL_RES *res;
		MYSQL_ROW row; /* keep in memory actual row */
		MYSQL * con;
	} my;
#endif
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	struct __pg {
		PGresult *res;
		int iter; /* libpq does not have nexrow .. */
	} pg;
#endif
#ifdef ODBC_FOUND
	SQLHSTMT odbc;      /* Statement being executed */
#endif
} SQL_result;

#define SMSD_ODBC_MAX_RETURN_STRINGS 30

/* sql connection structures */
typedef union __sql_conn {
#ifdef LIBDBI_FOUND
	dbi_conn dbi; /* dbi driver */
#endif
#ifdef HAVE_MYSQL_MYSQL_H
	MYSQL *my; /* mysql driver */
#endif
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	PGconn *pg; /* pgsql driver */
#endif
#ifdef ODBC_FOUND
	struct {
		SQLHENV env;        /* Environment */
		SQLHDBC dbc;        /* DBC */
		char * retstr[SMSD_ODBC_MAX_RETURN_STRINGS + 1];	    /* Return strings */
	} odbc;
#endif
} SQL_conn;

/* types passed to NamedQuery */
typedef enum {
	SQL_TYPE_NONE, /* used at end of array */
	SQL_TYPE_INT, /* argument is type int */
	SQL_TYPE_STRING /* argument is pointer to char */
} SQL_Type;

/* NamedQuery SQL parameter value as part of SQL_Var */
typedef union {
	const char *s;
	long long int i;
} SQL_Val;

/* NamedQuery SQL parameter passed by caller function */
typedef struct {
	SQL_Type type;
	SQL_Val v;
} SQL_Var;

/* configurable queries
 * NOTE: parameter sequence in select queries are mandatory !!!
 */
enum {
	SQL_QUERY_DELETE_PHONE, /* after-initialization phone deleting */
	SQL_QUERY_INSERT_PHONE, /* insert phone */
	SQL_QUERY_SAVE_INBOX_SMS_SELECT,
	SQL_QUERY_SAVE_INBOX_SMS_UPDATE_DELIVERED,
	SQL_QUERY_SAVE_INBOX_SMS_UPDATE,
	SQL_QUERY_SAVE_INBOX_SMS_INSERT,
	SQL_QUERY_UPDATE_RECEIVED,
	SQL_QUERY_REFRESH_SEND_STATUS,
	SQL_QUERY_UPDATE_RETRIES,
	SQL_QUERY_FIND_OUTBOX_SMS_ID,
	SQL_QUERY_FIND_OUTBOX_BODY,
	SQL_QUERY_FIND_OUTBOX_MULTIPART,
	SQL_QUERY_DELETE_OUTBOX,
	SQL_QUERY_DELETE_OUTBOX_MULTIPART,
	SQL_QUERY_CREATE_OUTBOX,
	SQL_QUERY_CREATE_OUTBOX_MULTIPART,
	SQL_QUERY_UPDATE_OUTBOX,
	SQL_QUERY_UPDATE_OUTBOX_MULTIPART,
	SQL_QUERY_UPDATE_OUTBOX_STATUSCODE,
	SQL_QUERY_UPDATE_OUTBOX_MULTIPART_STATUSCODE,
	SQL_QUERY_ADD_SENT_INFO,
	SQL_QUERY_UPDATE_SENT,
	SQL_QUERY_REFRESH_PHONE_STATUS,
	SQL_QUERY_LAST_NO
};

/* incomplete declaration - cyclic occurence of GSM_SMSDConfig */
struct GSM_SMSDConfig;


struct GSM_SMSDdbobj {
	GSM_Error (* Connect)(GSM_SMSDConfig *);
	GSM_Error (* Query)(GSM_SMSDConfig *, const char *, SQL_result *);
	void (* Free)(GSM_SMSDConfig *); /* = close() */
	void (* FreeResult)(GSM_SMSDConfig *, SQL_result *);
	int (* NextRow)(GSM_SMSDConfig *, SQL_result *);
	unsigned long long (* SeqID)(GSM_SMSDConfig *, const char *);
	unsigned long (* AffectedRows)(GSM_SMSDConfig *, SQL_result *);
	const char * (* GetString)(GSM_SMSDConfig *, SQL_result *, unsigned int);
	long long (* GetNumber)(GSM_SMSDConfig *, SQL_result *, unsigned int);
	time_t (* GetDate)(GSM_SMSDConfig *, SQL_result *, unsigned int);
	gboolean (* GetBool)(GSM_SMSDConfig *, SQL_result *, unsigned int);
	char * (* QuoteString)(GSM_SMSDConfig *, const char *);
};

/* database backends */
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
extern struct GSM_SMSDdbobj SMSDPgSQL;
#endif

#ifdef HAVE_MYSQL_MYSQL_H
extern struct GSM_SMSDdbobj SMSDMySQL;
#endif

#ifdef LIBDBI_FOUND
extern struct GSM_SMSDdbobj SMSDDBI;
#endif

#ifdef ODBC_FOUND
extern struct GSM_SMSDdbobj SMSDODBC;
#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
