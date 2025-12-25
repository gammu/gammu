/* (c) 2002-2004 by Marcin Wiacek and Joergen Thomsen */
/* Copyright (c) 2009 - 2018 Michal Cihar <michal@cihar.com> */

#ifndef __core_h_
#define __core_h_

#include <gammu.h>
#include <gammu-smsd.h>

#ifdef HAVE_SHM
#include <sys/types.h>
#endif
/* definition of dbobject */
#if defined(HAVE_MYSQL_MYSQL_H) || defined(HAVE_POSTGRESQL_LIBPQ_FE_H) || defined(LIBDBI_FOUND) || defined(ODBC_FOUND)
#include "services/sql-core.h"
#endif

#define SMSD_SHM_VERSION (2)
#define SMSD_SHM_KEY (0xfa << 16 || SMSD_SHM_VERSION)
#define SMSD_DB_VERSION (17)

#include "log.h"

#include "../libgammu/misc/array.h"

typedef enum {
	DEBUG_ERROR = -1,
	DEBUG_INFO = 0,
	DEBUG_NOTICE = 1,
	DEBUG_SQL = 2,
	DEBUG_GAMMU = 4,
} SMSD_DebugLevel;

typedef enum {
	SMSD_LOG_NONE,
	SMSD_LOG_FILE,
	SMSD_LOG_SYSLOG,
	SMSD_LOG_EVENTLOG
} SMSD_LogType;

typedef enum {
	SMSD_SEND_OK = 1,
	SMSD_SEND_SENDING_ERROR,
	SMSD_SEND_DELIVERY_PENDING,
	SMSD_SEND_DELIVERY_FAILED,
	SMSD_SEND_DELIVERY_OK,
	SMSD_SEND_DELIVERY_UNKNOWN,
	SMSD_SEND_ERROR
} GSM_SMSDSendingError;

typedef struct {
	GSM_Error	(*Init) 	      (GSM_SMSDConfig *Config);
	GSM_Error	(*Free) 	      (GSM_SMSDConfig *Config);
	GSM_Error	(*InitAfterConnect)   (GSM_SMSDConfig *Config);
	GSM_Error	(*SaveInboxSMS)       (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char **Locations);
	GSM_Error	(*FindOutboxSMS)      (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *ID);
	GSM_Error	(*MoveSMS)  	      (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *ID, gboolean alwaysDelete, gboolean sent);
	GSM_Error	(*CreateOutboxSMS)    (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *NewID);
	GSM_Error	(*AddSentSMSInfo)     (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *ID, int Part, GSM_SMSDSendingError err, int TPMR);
	GSM_Error	(*RefreshSendStatus)  (GSM_SMSDConfig *Config, char *ID);
	GSM_Error	(*UpdateRetries)  (GSM_SMSDConfig *Config, char *ID);
	/**
	 * Updates phone status in service backend. Please note that
	 * this can not talk to the phone.
	 */
	GSM_Error	(*RefreshPhoneStatus) (GSM_SMSDConfig *Config);
	/**
	 * Reads configuration specific for this backend.
	 */
	GSM_Error	(*ReadConfiguration) (GSM_SMSDConfig *Config);
} GSM_SMSDService;

struct _GSM_SMSDConfig {
	const char	*ServiceName;
	const char *program_name;
	/* general options */
	GSM_StringArray IncludeNumbersList, ExcludeNumbersList;
	GSM_StringArray IncludeSMSCList, ExcludeSMSCList;
	unsigned int    commtimeout, 	 sendtimeout,   receivefrequency, statusfrequency;
	unsigned int loopsleep;
	int deliveryreportdelay;
	unsigned int	resetfrequency;
	unsigned int	hardresetfrequency;
	unsigned int multiparttimeout;
	const char   *deliveryreport, *logfilename, *logfacility,  *PINCode, *NetworkCode, *PhoneCode;
	const char	*PhoneID;
	const char   *RunOnReceive;
	const char   *RunOnFailure; /* run this command on phone communication failure */
	const char   *RunOnSent; /* run this command when an SMS has been sent successfully */
	const char   *RunOnIncomingCall; /* run this command when a phone call has been canceled */
	gboolean checksecurity;
	gboolean hangupcalls;
	gboolean checkbattery;
	gboolean checksignal;
	gboolean checknetwork;
	gboolean enable_send;
	gboolean enable_receive;
	unsigned int maxretries;
	unsigned int retrytimeout;
	int backend_retries;

	/* options for FILES */
	const char   *inboxpath, 	 *outboxpath, 	*sentsmspath;
	const char   *errorsmspath, 	 *inboxformat,  *transmitformat, *outboxformat;

	/* private variables required for work */
	int		relativevalidity;
	unsigned int 	retries;
	int		currdeliveryreport;
	unsigned char 	SMSID[200],	 prevSMSID[200];
	GSM_SMSC	SMSC, SMSCCache;
	const char	*skipsmscnumber;
	int		IgnoredMessages;
	gboolean 	SkipMessage[GSM_MAX_MULTI_SMS];

#if defined(HAVE_MYSQL_MYSQL_H) || defined(HAVE_POSTGRESQL_LIBPQ_FE_H) || defined(LIBDBI_FOUND) || defined(ODBC_FOUND)
	/* options for SQL database */
	/**
	 * Database name.
	 */
	const char	*database;
	/**
	 * User name to connect to database.
	 */
	const char	*user;
	/**
	 * User password to connect to database.
	 */
	const char	*password;
	/**
	 * Database driver to use.
	 */
	const char	*driver;
	/**
	 * Path to database drivers.
	 */
	const char	*driverspath;
	/**
	 * SQL dialect to use.
	 */
	const char	*sql;
	/**
	 * Path to database directory.
	 */
	const char	*dbdir;
	/**
	 * Address of the database (eg. hostname).
	 */
	const char	*host;
        char 		DT[200];
	char		CreatorID[200];
	/* database data structure */
	struct GSM_SMSDdbobj *db;
	SQL_conn conn;
	/* configurable SQL queries */
	char * SMSDSQL_queries[SQL_QUERY_LAST_NO];

	const char *table_gammu;
	const char *table_inbox;
	const char *table_sentitems;
	const char *table_outbox;
	const char *table_outbox_multipart;
	const char *table_phones;
#endif

	INI_Section 		*smsdcfgfile;
	volatile gboolean	shutdown;
	/**
	 * Whether SMSD daemon itself is running.
	 */
	gboolean running;
	/**
	 * Whether we're connected to the database.
	 */
	gboolean connected;
	gboolean exit_on_failure;
	GSM_Error failure;
	GSM_StateMachine *gsm;
	char *gammu_log_buffer;
	size_t gammu_log_buffer_size;
	/**
	 * Log critical messages to stderr?
	 */
	gboolean use_stderr;
	/**
	 * Log with timestamps (not applicable for syslog).
	 */
	gboolean use_timestamps;
	int debug_level;
	/**
	 * Where to send log messages.
	 */
	SMSD_LogType log_type;
	void *log_handle;

	volatile GSM_Error SendingSMSStatus;
	/**
	 * Message reference set by callback from libGammu.
	 */
	volatile int TPMR;
	volatile int StatusCode;
	volatile int Part;

	/**
	 * Multipart messages processing.
	 */
	int IncompleteMessageID;
	time_t IncompleteMessageTime;

#ifdef HAVE_SHM
	key_t shm_key;
	int shm_handle;
#endif
#ifdef WIN32
	char map_key[MAX_PATH + 20];
	HANDLE map_handle;
#endif
	GSM_SMSDStatus *Status;
	GSM_SMSDService		*Service;
};

extern GSM_Error SMSD_NoneFunction		(void);
extern GSM_Error SMSD_EmptyFunction		(void);
extern GSM_Error SMSD_NotImplementedFunction	(void);
extern GSM_Error SMSD_NotSupportedFunction	(void);


#define NONEFUNCTION 	(void *) SMSD_NoneFunction
#define EMPTYFUNCTION 	(void *) SMSD_EmptyFunction
#define NOTIMPLEMENTED 	(void *) SMSD_NotImplementedFunction
#define NOTSUPPORTED 	(void *) SMSD_NotSupportedFunction

/**
 * Checks whether database version is up to date.
 */
GSM_Error SMSD_CheckDBVersion(GSM_SMSDConfig *Config, int version);

/**
 * Terminates SMSD with logging error messages to log. This does not
 * signal running SMSD to stop, it can be called from initialization of
 * SMSD wrapping program to terminate with logging.
 *
 * \param Config Pointer to SMSD configuration data.
 * \param msg Message to display.
 * \param error GSM error code, if applicable.
 * \param rc Program return code, will be passed to exit (if enabled).
 */
void SMSD_Terminate(GSM_SMSDConfig *Config, const char *msg, GSM_Error error, gboolean exitprogram, int rc);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
