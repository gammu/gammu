/* Test for handling corrupted AT responses with non-ASCII characters */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "../libgammu/protocol/at/at.h"
#include "../libgammu/protocol/protocol.h"	/* Needed for GSM_Protocol_Message */
#include "../libgammu/gsmstate.h"	/* Needed for state machine internals */
#include "../libgammu/gsmphones.h"	/* Phone data */

/* Simulates the corrupted CFUN response from issue #681 where modem
 * echoes back "AT\xFF\xE5\r\nOK\r\n" instead of "AT+CFUN=1\r\nOK\r\n"
 */
static const unsigned char corrupted_cfun_response[] = {
	'A', 'T', 0xFF, 0xE5, '\r', '\n', 'O', 'K', '\r', '\n', 0
};

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
	Data->RequestID = ID_Reset;  /* Simulating CFUN command */
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

	/* Feed corrupted data - non-ASCII chars should be filtered out */
	for (i = 0; corrupted_cfun_response[i] != 0; i++) {
		error = AT_StateMachine(s, corrupted_cfun_response[i]);
		gammu_test_result(error, "AT_StateMachine");
	}

	/* Should have received one message (OK response) */
	/* The corrupted echo "AT\xFF\xE5" should be filtered to just "AT" */
	test_result(s->MessagesCount >= 1);

	/* Verify the buffer doesn't contain the garbage bytes */
	test_result(d->Msg.Buffer == NULL || 
	            (memchr(d->Msg.Buffer, 0xFF, d->Msg.Length) == NULL &&
	             memchr(d->Msg.Buffer, 0xE5, d->Msg.Length) == NULL));

	/* Free state machine */
	GSM_FreeStateMachine(s);

	gammu_test_result(ERR_NONE, "at-cfun-corrupted");
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
