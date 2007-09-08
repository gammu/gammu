/* Test for decoding SMS on Nokia 6110 driver */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../common/protocol/protocol.h" /* Needed for GSM_Protocol_Message */
#include "../common/gsmstate.h" /* Needed for state machine internals */
#include "../common/gsmphones.h" /* Phone data */

GSM_StateMachine *s;
GSM_Protocol_Message msg;
#if 0
unsigned char data[] = "AT+CMGR=1\x0D\x0A"
	"+CMGR: 1,,78\x0D\x0A"
	"0791246030500200110481643000007030427132500047D6B0BEECCE83F4E17558EF4EAFEB2CD09C5DD78BC3207519447E8FC3737719D44EB7DF20B8FC6D7FEB5D2062795D5797DB65903E0C82BFC7E837BCEC4EBB00\x0D\x0A"
	"OK\x0D\x0A";
#else
unsigned char data[] = "AT+CMGR=1\x0D\x0A"
	"+CMGR: 3,,29\x0d\x0a"
	"0791361907001003B17A0C913619397750320000AD11CD701E340FB3C3F23CC81D0689C3BF\x0d\x0a"
	"OK\x0d\x0a";
#endif
GSM_Error error;
GSM_MultiSMSMessage sms;
GSM_MultiPartSMSInfo	SMSInfo;

/* This is not part of API! */
extern GSM_Error ATGEN_ReplyGetSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s);

int main(int argc, char **argv)
{
	GSM_Debug_Info *di;
	GSM_Phone_ATGENData *Priv;
	GSM_Phone_Data *Data;

	di = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, di);
	GSM_SetDebugLevel("textall", di);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL) {
		printf("Could not allocate state machine!\n");
		return 1;
	}

	/* Init message */
	Data = &s->Phone.Data;
	Data->ModelInfo = GetModelData(NULL, "unknown", NULL);
	Priv = &s->Phone.Data.Priv.ATGEN;
	Priv->ReplyState = AT_Reply_OK;
	Priv->SMSMode = SMS_AT_PDU;
	msg.Type = 0;
	msg.Length = strlen(data);
	msg.Buffer = data;
	SplitLines(msg.Buffer, msg.Length, &Priv->Lines, "\x0D\x0A", 2, true);

	s->Phone.Data.GetSMSMessage = &sms;

	/* Parse it */
	error = ATGEN_ReplyGetSMSMessage(msg, s);

	/* Free state machine */
	GSM_FreeStateMachine(s);

	printf("%s\n", GSM_ErrorString(error));

	return (error == ERR_NONE) ? 0 : 1;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
