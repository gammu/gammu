
#include "../../../gsmstate.h"

#include <string.h>
#include <time.h>

#include "../../../gsmcomon.h"
#include "../../../misc/coding.h"
#include "../../../service/gsmlogo.h"
#include "../nfunc.h"
#include "../nfuncold.h"
#include "../../pfunc.h"
#include "dct4func.h"

#ifdef GSM_ENABLE_NOKIA_DCT4

GSM_Error DCT4_ReplyGetPhoneMode(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	s->Phone.Data.PhoneString[0] = msg.Buffer[4];
	return GE_NONE;
}

GSM_Error DCT4_GetPhoneMode(GSM_StateMachine *s)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x02, 0x00, 0x00};

	smprintf(s,"Getting phone mode\n");

	return GSM_WaitFor (s, req, 6, 0x15, 4, ID_Reset);
}

GSM_Error DCT4_ReplySetPhoneMode(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return GE_NONE;
}

GSM_Error DCT4_SetPhoneMode(GSM_StateMachine *s, DCT4_PHONE_MODE mode)
{
	unsigned char PhoneMode[10];
	int	      i;
	GSM_Error     error;
	unsigned char req[] = {N6110_FRAME_HEADER, 0x01,
			       0x04,		/* phone mode */
			       0x00};

	if (s->ConnectionType != GCT_FBUS2) return GE_OTHERCONNECTIONREQUIRED;

	s->Phone.Data.PhoneString 	= PhoneMode;
	req[4] 				= mode;

	while (1) {
		smprintf(s,"Going to phone mode %i\n",mode);
		error = GSM_WaitFor (s, req, 6, 0x15, 4, ID_Reset);
		if (error != GE_NONE) return error;
		for (i=0;i<20;i++) {
			error=DCT4_GetPhoneMode(s);
			if (error != GE_NONE) return error;
			if (PhoneMode[0] == mode) return GE_NONE;
			my_sleep(500);
		}
	}
	return GE_NONE;
}

GSM_Error DCT4_ReplyGetIMEI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	memcpy(s->Phone.Data.IMEI,msg.Buffer + 10, 16);
	smprintf(s, "Received IMEI %s\n",s->Phone.Data.IMEI);
	return GE_NONE;
}

GSM_Error DCT4_GetIMEI (GSM_StateMachine *s)
{
	unsigned char req[5] = {N6110_FRAME_HEADER, 0x00, 0x41};

	smprintf(s, "Getting IMEI\n");
	return GSM_WaitFor (s, req, 5, 0x1B, 2, ID_GetIMEI);
}

GSM_Error DCT4_GetHardware(GSM_StateMachine *s, char *value)
{
	return NOKIA_GetPhoneString(s,"\x00\x03\x02\x07\x00\x02",6,0x1b,value,ID_GetHardware,10);
}

GSM_Error DCT4_GetProductCode(GSM_StateMachine *s, char *value)
{
	return NOKIA_GetPhoneString(s,"\x00\x03\x04\x0b\x00\x02",6,0x1b,value,ID_GetProductCode,10);
}

GSM_Error DCT4_Reset(GSM_StateMachine *s, bool hard)
{
	GSM_Error error;

	if (hard) return GE_NOTSUPPORTED;

	error = DCT4_SetPhoneMode(s, DCT4_MODE_TEST);
	if (error != GE_NONE) return error;

	error = DCT4_SetPhoneMode(s, DCT4_MODE_NORMAL);
	if (error != GE_NONE) return error;

	s->Phone.Data.EnableIncomingSMS = false;
	s->Phone.Data.EnableIncomingCB  = false;

	return GE_NONE;
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
