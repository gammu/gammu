
#include "../../common/service/gsmsms.h"

void SMSDeaemon(int argc, char *argv[]);

typedef struct {
	GSM_Error	(*Init) 	 	(void);
	GSM_Error	(*SaveInboxSMS)  	(GSM_MultiSMSMessage sms);
	GSM_Error	(*FindOutboxSMS) 	(GSM_MultiSMSMessage *sms, char *ID);
	GSM_Error	(*CopyOutboxSMS2Sent) 	(char *ID);
} GSM_SMSDeaemonService;
