/* (c) 2002-2003 by Marcin Wiacek and Joergen Thomsen */

#include "../../cfg/config.h"

#ifdef HAVE_MYSQL_MYSQL_H
#ifdef WIN32
#  include <mysql.h>
#  include <mysqld_error.h>
#else
#  include <mysql/mysql.h>
#  include <mysql/mysqld_error.h>
#endif
#endif

#include "../../common/service/sms/gsmsms.h"
#include "../../common/service/sms/gsmmulti.h"

#define MAX_RETRIES 1

void      SMSDaemon		(int argc, char *argv[]);
GSM_Error SMSDaemonSendSMS	(char *service, char *filename, GSM_MultiSMSMessage *sms);

typedef struct {
	/* general options */
	INI_Entry       *IncludeNumbers, *ExcludeNumbers;
	unsigned int    commtimeout, 	 sendtimeout,   receivefrequency;
	unsigned char   *deliveryreport, *logfilename,  *PINCode;

	/* options for FILES */
	unsigned char   *inboxpath, 	 *outboxpath, 	*sentsmspath;
	unsigned char   *errorsmspath, 	 *inboxformat,  *transmitformat;

	/* options for MYSQL */
	unsigned char	*database,	 *user,		*password;
	unsigned char	*PC;

	/* private variables required for work */
	unsigned int 	retries;
	unsigned char 	SMSID[200],	 prevSMSID[200];
#ifdef HAVE_MYSQL_MYSQL_H
	MYSQL 		DB;		 char 		DT[20];
#endif
} GSM_SMSDConfig;

typedef struct {
	GSM_Error	(*Init) 	   (GSM_SMSDConfig *Config);
	GSM_Error	(*SaveInboxSMS)    (GSM_MultiSMSMessage  sms, GSM_SMSDConfig *Config);
	GSM_Error	(*FindOutboxSMS)   (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, unsigned char *ID);
	GSM_Error	(*MoveSMS)  	   (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, unsigned char *ID, bool alwaysDelete, bool sent);
	GSM_Error	(*CreateOutboxSMS) (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config);
	GSM_Error	(*AddSentSMSInfo)  (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, unsigned char *ID, int Part, bool OK);
} GSM_SMSDService;

#ifdef __GNUC__
__attribute__((format(printf, 1, 2)))
#endif
void WriteSMSDLog(char *format, ...);

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
