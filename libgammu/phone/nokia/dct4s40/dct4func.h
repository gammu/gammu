/* (c) 2002-2003 by Marcin Wiacek */

#ifndef dct4func_h
#define dct4func_h

#include "../ncommon.h"

typedef enum {
	DCT4_MODE_NORMAL = 0x01,
	DCT4_MODE_TEST   = 0x04,
	DCT4_MODE_LOCAL  = 0x05
} DCT4_PHONE_MODE;

GSM_Error DCT4_ReplyGetPhoneMode (GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT4_ReplySetPhoneMode (GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error DCT4_ReplyGetIMEI	 (GSM_Protocol_Message msg, GSM_StateMachine *s);

GSM_Error DCT4_GetPhoneMode	 (GSM_StateMachine *s);
GSM_Error DCT4_SetPhoneMode	 (GSM_StateMachine *s, DCT4_PHONE_MODE mode);
GSM_Error DCT4_GetIMEI		 (GSM_StateMachine *s);
GSM_Error DCT4_GetHardware	 (GSM_StateMachine *s, char *value);
GSM_Error DCT4_GetProductCode	 (GSM_StateMachine *s, char *value);
GSM_Error DCT4_Reset		 (GSM_StateMachine *s, bool hard);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
