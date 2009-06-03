/* (c) 2002-2003 by Marcin Wiacek */

#include <string.h>
#include <time.h>

#include <gammu-config.h>

#include "../../../misc/coding/coding.h"
#include "../../../service/gsmlogo.h"
#include "../nfunc.h"
#include "../nfuncold.h"
#include "../../pfunc.h"
#include "dct4func.h"

#ifdef GSM_ENABLE_NOKIA_DCT4

GSM_Error DCT4_ReplyGetPhoneMode(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	s->Phone.Data.PhoneString[0] = msg.Buffer[4];
	return ERR_NONE;
}

GSM_Error DCT4_GetPhoneMode(GSM_StateMachine *s)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x02, 0x00, 0x00};

	smprintf(s,"Getting phone mode\n");

	return GSM_WaitFor (s, req, 6, 0x15, 4, ID_Reset);
}

GSM_Error DCT4_ReplySetPhoneMode(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s UNUSED)
{
	return ERR_NONE;
}

GSM_Error DCT4_SetPhoneMode(GSM_StateMachine *s, DCT4_PHONE_MODE mode)
{
	unsigned char PhoneMode[10];
	int	      i;
	GSM_Error     error;
	unsigned char req[] = {N6110_FRAME_HEADER, 0x01,
			       0x04,		/* phone mode */
			       0x00};

	if (s->ConnectionType != GCT_FBUS2) return ERR_OTHERCONNECTIONREQUIRED;

	s->Phone.Data.PhoneString 	= PhoneMode;
	req[4] 				= mode;

	smprintf(s,"Going to phone mode %i\n",mode);
	error = GSM_WaitFor (s, req, 6, 0x15, 4, ID_Reset);
	if (error != ERR_NONE) return error;
	for (i=0;i<20;i++) {
		error=DCT4_GetPhoneMode(s);
		if (error != ERR_NONE) return error;
		if (PhoneMode[0] == mode) break;
		usleep(500000);
	}

	return ERR_NONE;
}

GSM_Error DCT4_ReplyGetIMEI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	memcpy(s->Phone.Data.IMEI,msg.Buffer + 10, 16);
	smprintf(s, "Received IMEI %s\n",s->Phone.Data.IMEI);
	return ERR_NONE;
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

GSM_Error DCT4_Reset(GSM_StateMachine *s, gboolean hard)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x05,
			       0x80,		/* 0x80 - reset, 0x00 - off */
			       0x00};
/* 	unsigned char TimeReq[] = {N6110_FRAME_HEADER, 0x0E, 0x00, 0x00}; */

	if (hard) return ERR_NOTSUPPORTED;

/* 	error = DCT4_SetPhoneMode(s, DCT4_MODE_TEST); */
/* 	if (error != ERR_NONE) return error; */
/* 	error = DCT4_SetPhoneMode(s, DCT4_MODE_NORMAL); */
/* 	if (error != ERR_NONE) return error; */

	s->Phone.Data.EnableIncomingSMS = FALSE;
	s->Phone.Data.EnableIncomingCB  = FALSE;

	return GSM_WaitFor (s, req, 6, 0x15, 2, ID_Reset);
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
