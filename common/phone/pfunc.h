/* (c) 2002-2003 by Marcin Wiacek */

#ifndef phone_common2_h
#define phone_common2_h

#include "../service/sms/gsmsms.h"

extern GSM_SMSMessageLayout PHONE_SMSSubmit;
extern GSM_SMSMessageLayout PHONE_SMSDeliver;
extern GSM_SMSMessageLayout PHONE_SMSStatusReport;

GSM_Error PHONE_GetSMSFolders		(GSM_StateMachine *s, GSM_SMSFolders *folders);

void 	  GSM_CreateFirmwareNumber	(GSM_StateMachine *s);

GSM_Error PHONE_EncodeSMSFrame		(GSM_StateMachine *s, GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout, int *length, bool clear);

GSM_Error PHONE_Terminate		(GSM_StateMachine *s);

GSM_Error PHONE_RTTLPlayOneNote		(GSM_StateMachine *s, GSM_RingNote note, bool first);

GSM_Error PHONE_Beep			(GSM_StateMachine *s);

GSM_Error NoneReply(GSM_Protocol_Message msg, GSM_StateMachine *s);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
