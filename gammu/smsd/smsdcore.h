
#include "../../common/service/gsmsms.h"

void SMSDaemon(int argc, char *argv[]);

typedef struct {
	GSM_Error	(*Init) 	 	(void);
	GSM_Error	(*SaveInboxSMS)  	(GSM_MultiSMSMessage sms, char *inboxpath, char *inboxformat);
	GSM_Error	(*FindOutboxSMS) 	(GSM_MultiSMSMessage *sms, char *outboxpath, char *transmitformat, char *ID);
	GSM_Error	(*MoveSMS)	 	(char *sourcepath, char *destpath, char *ID, int alwaysDelete);
} GSM_SMSDaemonService;

void WriteSMSDLog(char *format, ...);

