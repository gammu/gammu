/* Test for dispatching replies in AT driver */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "../libgammu/phone/at/atgen.h"
#include "../libgammu/protocol/protocol.h"	/* Needed for GSM_Protocol_Message */
#include "../libgammu/gsmstate.h"	/* Needed for state machine internals */
#include "../libgammu/gsmphones.h"	/* Phone data */

GSM_StateMachine *s;

void do_test(const char *reply, GSM_AT_Reply_State expected, GSM_Error expect_error)
{
	GSM_Protocol_Message msg;
	GSM_Error error;

	msg.Length = strlen(reply);
	msg.Buffer = (char *)reply;
	msg.Type = 0;

	s->Phone.Data.RequestMsg = &msg;

	error = ATGEN_DispatchMessage(s);
	gammu_test_result_code(error, "Dispatch", expect_error);

}

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Debug_Info *debug_info;
	GSM_Phone_ATGENData *Priv;
	GSM_Phone_Data *Data;
	GSM_SecurityCodeType Status;
	GSM_SignalQuality Signal;

	/* Init locales to get proper encoding */
	GSM_InitLocales(NULL);

	/* Configure state machine */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	test_result(s != NULL);
	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(TRUE, debug_info);

	/* Initialize AT engine */
	Data = &s->Phone.Data;
	Data->ModelInfo = GetModelData(NULL, NULL, "unknown", NULL);
	Priv = &s->Phone.Data.Priv.ATGEN;
	Priv->ReplyState = AT_Reply_OK;
	Priv->SMSMode = SMS_AT_PDU;
	Priv->Charset = AT_CHARSET_GSM;
	s->Phone.Data.SecurityStatus = &Status;
	s->Phone.Functions = &ATGENPhone;
	InitLines(&s->Phone.Data.Priv.ATGEN.Lines);

	/* Perform real tests */
	s->Phone.Data.RequestID = ID_GetSecurityStatus;
	s->Protocol.Data.AT.CPINNoOK = TRUE;
	do_test("AT+CPIN?\r\n+CPIN: READY\r\n", AT_Reply_OK, ERR_NONE);

	s->Phone.Data.RequestID = ID_GetSecurityStatus;
	s->Protocol.Data.AT.CPINNoOK = FALSE;
	do_test("AT+CPIN?\r\n+CPIN: READY\r\n", AT_Reply_Unknown, ERR_UNKNOWNRESPONSE);

	s->Phone.Data.RequestID = ID_GetSignalQuality;
	s->Phone.Data.SignalQuality = &Signal;
	do_test("AT+CSQ\r\nAT+CSQ\r\n+CME ERROR: 515", AT_Reply_CMEError, ERR_BUSY);

	s->Phone.Data.RequestID = ID_SetMemoryType;
	do_test("AT+CPMS=\"ME\"\rAT+CPMS=\"ME\"\r\r\n+CPMS: 2,300,2,300,2,300\r\n\r\n+CPMS: 2,300,2,300,2,300\r\n\r\nOK\r\n", AT_Reply_OK, ERR_NONE);

	s->Phone.Data.RequestID = ID_GetFirmware;
	do_test("AT+CGMR\r\nNokia N950 (RM-680 rev 1124)\r\nDFL61 HARMATTAN 2.2011.39-5 PR RM680\r\nLinux version 2.6.32.39-dfl61-20113701 #1 PREEMPT Mon Sep 12 11:29:43 EEST 2011 (armv7l)\r\nmatd version 0.4.5\r\nMCU Vp 92_11w21_v6 26-05-11 RM-680 (c) Nokia\r\nOK", AT_Reply_OK, ERR_NONE);

	/* Free state machine */
	GSM_FreeStateMachine(s);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
