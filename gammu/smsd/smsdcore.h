/* (c) 2002-2004 by Marcin Wiacek and Joergen Thomsen */

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
	unsigned int	resetfrequency;
	unsigned char   *deliveryreport, *logfilename,  *PINCode;
	unsigned char	*PhoneID;

	/* options for FILES */
	unsigned char   *inboxpath, 	 *outboxpath, 	*sentsmspath;
	unsigned char   *errorsmspath, 	 *inboxformat,  *transmitformat;

	/* options for MYSQL */
	unsigned char	*database,	 *user,		*password;
	unsigned char	*PC,		 *skipsmscnumber;

	/* private variables required for work */
	int		relativevalidity;
	unsigned int 	retries,	 currdeliveryreport;
	unsigned char 	SMSID[200],	 prevSMSID[200];
	GSM_SMSC	SMSC;
#ifdef HAVE_MYSQL_MYSQL_H
	MYSQL 		DB;		 char 		DT[20];
#endif
} GSM_SMSDConfig;

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
	GSM_Error	(*InitAfterConnect)   (GSM_SMSDConfig *Config);
	GSM_Error	(*SaveInboxSMS)       (GSM_MultiSMSMessage  sms, GSM_SMSDConfig *Config);
	GSM_Error	(*FindOutboxSMS)      (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, unsigned char *ID);
	GSM_Error	(*MoveSMS)  	      (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, unsigned char *ID, bool alwaysDelete, bool sent);
	GSM_Error	(*CreateOutboxSMS)    (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config);
	GSM_Error	(*AddSentSMSInfo)     (GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, unsigned char *ID, int Part, GSM_SMSDSendingError err, int TPMR);
	GSM_Error	(*RefreshSendStatus)  (GSM_SMSDConfig *Config, unsigned char *ID);
	GSM_Error	(*RefreshPhoneStatus) (GSM_SMSDConfig *Config);
} GSM_SMSDService;

#ifdef __GNUC__
__attribute__((format(printf, 1, 2)))
#endif
void WriteSMSDLog(char *format, ...);

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
