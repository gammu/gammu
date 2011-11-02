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

	/* Free state machine */
	GSM_FreeStateMachine(s);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
