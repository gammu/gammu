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

#define MAX_RETRIES 1

struct _GSM_SMSDConfig {
	const char	*Service;
	/* general options */
	INI_Entry       *IncludeNumbers, *ExcludeNumbers;
	unsigned int    commtimeout, 	 sendtimeout,   receivefrequency;
	int deliveryreportdelay;
	unsigned int	resetfrequency;
	const char   *deliveryreport, *logfilename,  *PINCode;
	const char	*PhoneID;
	const char   *RunOnReceive;
	bool checksecurity;

	/* options for FILES */
	const char   *inboxpath, 	 *outboxpath, 	*sentsmspath;
	const char   *errorsmspath, 	 *inboxformat,  *transmitformat;

	/* private variables required for work */
	int		relativevalidity;
	unsigned int 	retries;
	int		currdeliveryreport;
	unsigned char 	SMSID[200],	 prevSMSID[200];
	GSM_SMSC	SMSC;
	char		IMEI[GSM_MAX_IMEI_LENGTH];

#if defined(HAVE_MYSQL_MYSQL_H) || defined(HAVE_POSTGRESQL_LIBPQ_FE_H)
	/* options for SQL database */
	const char	*database,	 *user,		*password;
	const char	*PC,		 *skipsmscnumber;
        char 		DT[20];
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
	INI_Section 		*smsdcfgfile;
	volatile bool	shutdown;
	GSM_StateMachine *gsm;
	char *gammu_log_buffer;
	size_t gammu_log_buffer_size;
	/**
	 * Log messages to syslog?
	 */
	bool use_syslog;
	/**
	 * Log critical messages to stderr?
	 */
	bool use_stderr;
	/**
	 * Log with timestamps (not applicable for syslog).
	 */
	bool use_timestamps;
	int debug_level;
	FILE *log_file;
	volatile GSM_Error SendingSMSStatus;
	volatile int TPMR;
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
	GSM_Error	(*MoveSMS)  	      (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *ID, bool alwaysDelete, bool sent);
	GSM_Error	(*CreateOutboxSMS)    (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config);
	GSM_Error	(*AddSentSMSInfo)     (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *ID, int Part, GSM_SMSDSendingError err, int TPMR);
	GSM_Error	(*RefreshSendStatus)  (GSM_SMSDConfig *Config, char *ID);
	/**
	 * Updates phone status in service backend. Please note that
	 * this can not talk to the phone.
	 */
	GSM_Error	(*RefreshPhoneStatus) (GSM_SMSDConfig *Config, GSM_BatteryCharge *Battery, GSM_SignalQuality *Signal);
} GSM_SMSDService;

extern GSM_Error SMSD_NoneFunction		(void);
extern GSM_Error SMSD_NotImplementedFunction	(void);
extern GSM_Error SMSD_NotSupportedFunction	(void);

#define NONEFUNCTION 	(void *) SMSD_NoneFunction
#define NOTIMPLEMENTED 	(void *) SMSD_NotImplementedFunction
#define NOTSUPPORTED 	(void *) SMSD_NotSupportedFunction

/**
 * Logs a message to SMSD log.
 *
 * \param level Importance level, 0 is an important message,
 *		1,2,4,... are debug information which can be disabled in smsdrc.
 *		-1 is a critical error message
 * \param format printf like format string.
 */
PRINTF_STYLE(3, 4)
void SMSD_Log(int level, GSM_SMSDConfig *Config, const char *format, ...);

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
