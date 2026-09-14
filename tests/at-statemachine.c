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

static const char *nwtime_test = "^NWTIME: 18/03/09,12:36:21+4,00\r\nAT+CPMS=\"SM\",\"SM\"\r\r\n+CPMS: 0,20,0,20,0,20\r\n\r\nOK\r\n";

static void feed_reply(GSM_StateMachine *s, const char *reply)
{
	size_t i;
	GSM_Error error;

	for (i = 0; reply[i] != '\0'; i++) {
		error = AT_StateMachine(s, reply[i]);
		gammu_test_result(error, "AT_StateMachine");
	}
}

static void test_ecind(GSM_StateMachine *s)
{
	GSM_Phone_Data *data = &s->Phone.Data;
	GSM_SecurityCodeType security = SEC_Pin;
	GSM_SMSMemoryStatus status;
	size_t messages;
	size_t i;
	static const char *sms_replies[] = {
		"+ECIND: 2,0\r\nAT+CPMS=\"SM\",\"SM\"\r\r\n+CPMS: 1,10,1,10,1,10\r\n\r\nOK\r\n",
		"AT+CPMS=\"SM\",\"SM\"\r\r\n+ECIND: 2,0\r\n+CPMS: 1,10,1,10,1,10\r\n\r\nOK\r\n"
	};

	/* Issue #441: a notification must not swallow the following PIN reply. */
	data->SecurityStatus = &security;
	data->RequestID = ID_GetSecurityStatus;
	feed_reply(s, "+ECIND: 2,0\r\nAT+CPIN?\r\r\n+CPIN: READY\r\n\r\nOK\r\n");
	test_result(data->RequestID == ID_None);
	test_result(data->DispatchError == ERR_NONE);
	test_result(security == SEC_None);

	/* Notifications preceding or interleaved with SMS storage replies. */
	data->SMSStatus = &status;
	for (i = 0; i < sizeof(sms_replies) / sizeof(sms_replies[0]); i++) {
		memset(&status, 0, sizeof(status));
		data->RequestID = ID_GetSMSStatus;
		feed_reply(s, sms_replies[i]);
		test_result(data->RequestID == ID_None);
		test_result(data->DispatchError == ERR_NONE);
		test_result(status.SIMUsed == 1);
		test_result(status.SIMSize == 10);
	}

	/* Repeated notifications alone must not complete an active request. */
	security = SEC_Pin;
	data->RequestID = ID_GetSecurityStatus;
	messages = s->MessagesCount;
	feed_reply(s, "+ECIND: 2,0\r\n+ECIND: 2,1\r\n");
	test_result(data->RequestID == ID_GetSecurityStatus);
	test_result(data->DispatchError == ERR_NONE);
	test_result(security == SEC_Pin);
	test_result(s->MessagesCount == messages + 2);
	feed_reply(s, "AT+CPIN?\r\r\n+CPIN: READY\r\n\r\nOK\r\n");
	test_result(data->RequestID == ID_None);
	test_result(data->DispatchError == ERR_NONE);
	test_result(security == SEC_None);

	/* Idle notifications are consumed without changing reported state. */
	messages = s->MessagesCount;
	feed_reply(s, "+ECIND: 2,1\r\n+ECIND: 2,0\r\n");
	test_result(data->RequestID == ID_None);
	test_result(data->DispatchError == ERR_NONE);
	test_result(s->MessagesCount == messages + 2);
	test_result(status.SIMUsed == 1);
	test_result(status.SIMSize == 10);
	data->SecurityStatus = NULL;
	data->SMSStatus = NULL;
}

static void count_sms_status(GSM_StateMachine *s UNUSED, int status UNUSED,
		int reference UNUSED, void *user_data)
{
	(*(int *)user_data)++;
}

static void test_quectel_sms_done(GSM_StateMachine *s)
{
	GSM_Phone_Data *data = &s->Phone.Data;
	GSM_Protocol_ATData *protocol = &s->Protocol.Data.AT;
	GSM_SecurityCodeType security = SEC_Pin;
	int callbacks = 0;
	size_t messages;
	size_t i;
	static const struct {
		const char *before_prompt;
		const char *prompt;
	} replies[] = {
		{"", "+QIND: SMS DONE\r\nAT+CMGS=40\r\r\n> "},
		{"+QIND: SMS DONE\r\n", "AT+CMGS=40\r\r\n> "},
		{"AT+CMGS=40\r\r\n+QIND: SMS DONE\r\n", "> "},
		{"+QIND: SMS DONE\r\n+QIND: SMS DONE\r\n", "AT+CMGS=40\r\r\n> "}
	};

	GSM_SetSendSMSStatusCallback(s, count_sms_status, &callbacks);

	/* Issue #468: the notification must not hide or replace the prompt. */
	for (i = 0; i < sizeof(replies) / sizeof(replies[0]); i++) {
		data->RequestID = ID_IncomingFrame;
		protocol->EditMode = TRUE;
		feed_reply(s, replies[i].before_prompt);
		test_result(data->RequestID == ID_IncomingFrame);
		test_result(protocol->EditMode);
		test_result(callbacks == 0);
		feed_reply(s, replies[i].prompt);
		test_result(data->RequestID == ID_None);
		test_result(data->DispatchError == ERR_NONE);
		test_result(!protocol->EditMode);
		test_result(callbacks == 0);

		/* Only the subsequent send result should report SMS completion. */
		feed_reply(s, "\r\n+CMGS: 1\r\nOK\r\n");
		test_result(data->DispatchError == ERR_NONE);
		test_result(callbacks == 1);
		callbacks = 0;
	}

	/* Idle notifications are consumed without reporting an SMS result. */
	messages = s->MessagesCount;
	feed_reply(s, "+QIND: SMS DONE\r\n+QIND: SMS DONE\r\n");
	test_result(data->RequestID == ID_None);
	test_result(data->DispatchError == ERR_NONE);
	test_result(!protocol->EditMode);
	test_result(s->MessagesCount == messages + 2);
	test_result(callbacks == 0);

	/* An unrelated request must still wait for its own response. */
	data->SecurityStatus = &security;
	data->RequestID = ID_GetSecurityStatus;
	feed_reply(s, "+QIND: SMS DONE\r\n");
	test_result(data->RequestID == ID_GetSecurityStatus);
	test_result(data->DispatchError == ERR_NONE);
	test_result(security == SEC_Pin);
	feed_reply(s, "AT+CPIN?\r\r\n+CPIN: READY\r\n\r\nOK\r\n");
	test_result(data->RequestID == ID_None);
	test_result(data->DispatchError == ERR_NONE);
	test_result(security == SEC_None);
	test_result(callbacks == 0);
	data->SecurityStatus = NULL;
	GSM_SetSendSMSStatusCallback(s, NULL, NULL);
}

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Debug_Info *debug_info;
	GSM_Phone_ATGENData *Priv;
	GSM_Phone_Data *Data;
	GSM_SMSMemoryStatus SMSStatus;
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

	memset(&SMSStatus, 0, sizeof(SMSStatus));
	Data->SMSStatus = &SMSStatus;
	Data->RequestID = ID_GetSMSStatus;

	/* Unsolicited Huawei network time must not contaminate a command reply. */
	for (i = 0; i < strlen(nwtime_test); i++) {
		error = AT_StateMachine(s, nwtime_test[i]);
		gammu_test_result(error, "AT_StateMachine");
	}

	test_result(Data->RequestID == ID_None);
	test_result(Data->DispatchError == ERR_NONE);
	test_result(SMSStatus.SIMUsed == 0);
	test_result(SMSStatus.SIMSize == 20);
	test_result(s->MessagesCount == 8);

	test_ecind(s);
	test_quectel_sms_done(s);

	/* Free state machine */
	GSM_FreeStateMachine(s);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
