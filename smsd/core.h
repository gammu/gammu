/* (c) 2002-2004 by Marcin Wiacek and Joergen Thomsen */

#include <gammu.h>
#include <gammu-smsd.h>

#ifdef HAVE_MYSQL_MYSQL_H
#ifdef WIN32
#  include <winsock2.h>
#endif
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

#define SMSD_SHM_KEY (0xface)
#define SMSD_SHM_VERSION (1)
#define SMSD_DB_VERSION (11)
#define SMSD_SQL_RETRIES (10)

#include "log.h"

#include "../helper/array.h"

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

struct _GSM_SMSDConfig {
	const char	*Service;
	const char *program_name;
	/* general options */
	GSM_StringArray IncludeNumbersList, ExcludeNumbersList;
	GSM_StringArray IncludeSMSCList, ExcludeSMSCList;
	unsigned int    commtimeout, 	 sendtimeout,   receivefrequency, statusfrequency;
	unsigned int loopsleep;
	int deliveryreportdelay;
	unsigned int	resetfrequency;
	unsigned int multiparttimeout;
	const char   *deliveryreport, *logfilename,  *PINCode, *NetworkCode, *PhoneCode;
	const char	*PhoneID;
	const char   *RunOnReceive;
	gboolean checksecurity;
	gboolean checkbattery;
	gboolean checksignal;
	unsigned int maxretries;

	/* options for FILES */
	const char   *inboxpath, 	 *outboxpath, 	*sentsmspath;
	const char   *errorsmspath, 	 *inboxformat,  *transmitformat, *outboxformat;

	/* private variables required for work */
	int		relativevalidity;
	unsigned int 	retries;
	int		currdeliveryreport;
	unsigned char 	SMSID[200],	 prevSMSID[200];
	GSM_SMSC	SMSC;
	const char	*skipsmscnumber;

#if defined(HAVE_MYSQL_MYSQL_H) || defined(HAVE_POSTGRESQL_LIBPQ_FE_H) || defined(LIBDBI_FOUND)
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
	 * Path to database directory.
	 */
	const char	*dbdir;
	/**
	 * Address of the database (eg. hostname).
	 */
	const char	*PC;
        char 		DT[25];
	char		CreatorID[200];
#endif

#ifdef HAVE_MYSQL_MYSQL_H
       /* MySQL db connection */
       MYSQL DBConnMySQL;
#endif

#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
       /* PostgreSQL db connection */
       PGconn *DBConnPgSQL;
#endif

#ifdef LIBDBI_FOUND
	dbi_conn DBConnDBI;
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

	/**
	 * Multipart messages processing.
	 */
	unsigned char IncompleteMessageID;
	time_t IncompleteMessageTime;

#ifdef HAVE_SHM
	key_t shm_key;
	int shm_handle;
#endif
#ifdef WIN32
	char map_key[MAX_PATH];
	HANDLE map_handle;
#endif
	GSM_SMSDStatus *Status;
};

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
	/**
	 * Updates phone status in service backend. Please note that
	 * this can not talk to the phone.
	 */
	GSM_Error	(*RefreshPhoneStatus) (GSM_SMSDConfig *Config);
} GSM_SMSDService;

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

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
