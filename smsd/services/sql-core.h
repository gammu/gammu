/* drivers for sql service
 *
 * MySQL (C) 2004 by Marcin Wiacek 
 * PostgreSQL (C) 2006 by Andrea Riciputi
 * DBI (C) 2009 by Michal Čihař 
 *
 */

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
} SQL_result;

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
} SQL_conn;

/* SQL errors */
typedef enum {
	SQL_OK, /* all ok */
	SQL_TIMEOUT, /* query or connection timeout */
	SQL_FAIL, /* query failed */
	SQL_LOCKED /* locked table - currently unused */
} SQL_Error;

/* types passed to NamedQuery */
typedef enum {
	SQL_TYPE_NONE, /* used at end of array */
	SQL_TYPE_INT, /* argument is type int */
	SQL_TYPE_STRING /* argument is pointer to char */
} SQL_Type;

/* NamedQuery SQL parameter value as part of SQL_Var */
typedef union {
	const char *s;
	int i;
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
	SQL_QUERY_FIND_OUTBOX_SMS_ID,
	SQL_QUERY_FIND_OUTBOX_BODY,
	SQL_QUERY_FIND_OUTBOX_MULTIPART,
	SQL_QUERY_DELETE_OUTBOX,
	SQL_QUERY_DELETE_OUTBOX_MULTIPART,
	SQL_QUERY_CREATE_OUTBOX,
	SQL_QUERY_CREATE_OUTBOX_MULTIPART,
	SQL_QUERY_ADD_SENT_INFO,
	SQL_QUERY_UPDATE_SENT,
	SQL_QUERY_REFRESH_PHONE_STATUS,
	SQL_QUERY_LAST_NO
};

/* incomplete declaration - cyclic occurence of GSM_SMSDConfig */
struct GSM_SMSDConfig;


struct GSM_SMSDdbobj {
	const char *DriverName;
	SQL_Error (* Connect)(GSM_SMSDConfig *);
	SQL_Error (* Query)(GSM_SMSDConfig *, const char *, SQL_result *res);
	void (* Free)(SQL_conn *); /* = close() */
	void (* FreeResult)(SQL_result);
	int (* NextRow)(SQL_result *);
	unsigned long long (* SeqID)(SQL_conn *conn, const char *);
	unsigned long (* AffectedRows)(SQL_result);
	unsigned long (* NumRows)(SQL_result);
	const char * (* GetString)(SQL_result, unsigned int);
	long long (* GetNumber)(SQL_result, unsigned int);
	time_t (* GetDate)(SQL_result, unsigned int);
	gboolean (* GetBool)(SQL_result, unsigned int);
	char * (* QuoteString)(SQL_conn *conn, const char *);
	const char *error;
	SQL_conn conn;
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

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
