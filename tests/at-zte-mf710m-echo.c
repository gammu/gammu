/* Test for ZTE MF710M echo issue fix */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "../libgammu/protocol/at/at.h"
#include "../libgammu/protocol/protocol.h"	/* Needed for GSM_Protocol_Message */
#include "../libgammu/gsmstate.h"	/* Needed for state machine internals */
#include "../libgammu/gsmphones.h"	/* Phone data */

/* This simulates the exact response from ZTE MF710M as reported in the issue:
 * The modem sends "ATE0+CPMS?" as a malformed echo (concatenation of two commands),
 * followed by the actual +CPMS response.
 *
 * Without the fix, Gammu would report "UNKNOWN frame" for the malformed echo line.
 * With the fix, Gammu strips the malformed echo and processes the response correctly.
 *
 * Note: The response "+CPMS:" might still generate "UNKNOWN frame" if the RequestID
 * doesn't match any handler, but that's acceptable - the key improvement is that the
 * malformed echo is removed and doesn't interfere with processing.
 */
static const char *zte_echo_response = "ATE0+CPMS?\r\n+CPMS: \"ME\", 0, 50, \"ME\", 0, 50, \"ME\", 0, 50\r\n\r\nOK\r\n";

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Debug_Info *debug_info;
	GSM_Phone_ATGENData *Priv;
	GSM_Phone_Data *Data;
	GSM_StateMachine *s;
	GSM_Protocol_ATData *d;
	size_t i;
	GSM_Error error;

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
	Data->RequestID = ID_GetSMSStatus;  /* Expecting CPMS response */
	Priv = &s->Phone.Data.Priv.ATGEN;
	Priv->ReplyState = AT_Reply_OK;
	Priv->SMSMode = SMS_AT_PDU;
	Priv->Charset = AT_CHARSET_GSM;
	s->Phone.Functions = &ATGENPhone;

	d = &s->Protocol.Data.AT;
	d->Msg.Buffer 		= NULL;
	d->Msg.BufferUsed	= 0;
	d->Msg.Length		= 0;
	d->Msg.Type		= 0;
	d->SpecialAnswerLines	= 0;
	d->LineStart		= -1;
	d->LineEnd		= -1;
	d->wascrlf 		= FALSE;
	d->EditMode		= FALSE;
	d->FastWrite		= FALSE;
	d->CPINNoOK		= FALSE;

	/* Feed data with malformed echo */
	for (i = 0; i < strlen(zte_echo_response); i++) {
		error = AT_StateMachine(s, zte_echo_response[i]);
		gammu_test_result(error, "AT_StateMachine");
	}

	/* Should have processed 1 message (the CPMS response, echo should be stripped) */
	test_result(s->MessagesCount == 1);

	/* Free state machine */
	GSM_FreeStateMachine(s);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
