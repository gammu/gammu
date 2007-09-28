/* Code for printing SMSes used in tests */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../common/protocol/protocol.h" /* Needed for GSM_Protocol_Message */
#include "../common/gsmstate.h" /* Needed for state machine internals */
#include "../common/gsmphones.h" /* Phone data */

/* This is not part of API! */
extern GSM_Error ATGEN_ReplyGetSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s);

#define _(x) (x)
#define ngettext(x, y, z) (x)
#define LISTFORMAT "%-20s : "

void PrintSMSCNumber(unsigned char *number,GSM_Backup *Info);
void DisplaySingleSMSInfo(GSM_SMSMessage sms, bool displaytext, bool displayudh, GSM_Backup *Info);
void DisplayTestSMS(GSM_MultiSMSMessage sms);

