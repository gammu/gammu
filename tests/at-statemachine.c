/* Test for decoding SMS on AT driver */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "../libgammu/protocol/at/at.h"
#include "../libgammu/protocol/protocol.h"	/* Needed for GSM_Protocol_Message */
#include "../libgammu/gsmstate.h"	/* Needed for state machine internals */
#include "../libgammu/gsmphones.h"	/* Phone data */

static const char *test_data = "+CUSD: 2,\"Maaf, permintaan Anda tidak dapat kami proses. Silakan coba beberapa saat lagi.\",0\r\nAT+CSCS=\"HEX\"\r\nOK\r\n";

static const char *second_test = "+CMTI: \"SM\",1\r\nAT+CPMS=\"SM\",\"SM\"\r\r\n+CPMS: 1,20,1,20,1,20\r\n\r\nOK\r\n";

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
	Data->RequestID = ID_SetMemoryCharset;
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

	/* Feed data */
	for (i = 0; i < strlen(test_data); i++) {
		error = AT_StateMachine(s, test_data[i]);
		gammu_test_result(error, "AT_StateMachine");
	}

	test_result(s->MessagesCount == 2);

	Data->RequestID = ID_SetMemoryType;

	/* Feed data */
	for (i = 0; i < strlen(second_test); i++) {
		error = AT_StateMachine(s, second_test[i]);
		gammu_test_result(error, "AT_StateMachine");
	}

	test_result(s->MessagesCount == 4);

	Data->RequestID = ID_None;

	/* Feed data */
	for (i = 0; i < strlen(second_test); i++) {
		error = AT_StateMachine(s, second_test[i]);
		gammu_test_result(error, "AT_StateMachine");
	}

	test_result(s->MessagesCount == 6);

	/* Free state machine */
	GSM_FreeStateMachine(s);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
