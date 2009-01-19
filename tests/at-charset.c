/* Test for decoding SMS on AT driver */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "../common/phone/at/atgen.h"
#include "../common/protocol/protocol.h" /* Needed for GSM_Protocol_Message */
#include "../common/gsmstate.h" /* Needed for state machine internals */
#include "../common/gsmphones.h" /* Phone data */

#define BUFFER_SIZE ((size_t)16384)

unsigned char latin1text[] = { 0x00, 0xed, 0x00, 0xed, 0x00, 0xed, 0x00, 0xe1, 0x00, 0xe1, 0x00, 0xe1, 0x00, 0xe1, 0x00, 0xe1, 0x00, 0xe9, 0x00, 0xe9, 0x00, 0xe9, 0x00, 0xe9, 0x00, 0xe9, 0x00, 0x00 };
char latin1utf8[] = "íííáááááééééé";

unsigned char latin2text[] = { 0x00, 0xed, 0x01, 0x1b, 0x00, 0xe1, 0x01, 'a' , 0x00, 0xfd, 0x01, 'Y' , 0x01, '\r', 0x00, 0xfd, 0x00, 0xed, 0x01, '\r', 0x01, 'Y' , 0x01, 0x1b, 0x00, 0x00 };
char latin2utf8[] = "íěášýřčýíčřě";

#ifdef ICONV_FOUND
char latin1cp437[] = { 0xa1, 0xa1, 0xa1, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0x82, 0x82, 0x82, 0x82, 0x82, 0x00 };
char latin1iso88591[] = { 0xed, 0xed, 0xed, 0xe1, 0xe1, 0xe1, 0xe1, 0xe1, 0xe9, 0xe9, 0xe9, 0xe9, 0xe9, 0x00 };
char latin2iso88592[] = { 0xed, 0xec, 0xe1, 0xb9, 0xfd, 0xf8, 0xe8, 0xfd, 0xed, 0xe8, 0xf8, 0xec, 0x00 };
#endif

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Debug_Info *debug_info;
	GSM_Phone_ATGENData *Priv;
	GSM_Phone_Data *Data;
	char buffer[BUFFER_SIZE];
	size_t result;
	GSM_StateMachine *s;
	GSM_Error error;

	/* Configure state machine */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, false, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	test_result(s != NULL);
	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(true, debug_info);

	/* Initialize AT engine */
	Data = &s->Phone.Data;
	Data->ModelInfo = GetModelData(NULL, NULL, "unknown", NULL);
	Priv = &s->Phone.Data.Priv.ATGEN;
	Priv->ReplyState = AT_Reply_OK;
	Priv->SMSMode = SMS_AT_PDU;
	Priv->Charset = AT_CHARSET_GSM;

	/* Perform real tests */
	Priv->Charset = AT_CHARSET_UTF8;
	error = ATGEN_EncodeText(s, latin2text, sizeof(latin2text) / 2, buffer, sizeof(buffer), &result);
	gammu_test_result(error, "Encode - 1");
	test_result(strcmp(latin2utf8, buffer) == 0);
	test_result(strncmp(latin2utf8, buffer, result) == 0);

	Priv->Charset = AT_CHARSET_UTF8;
	error = ATGEN_EncodeText(s, latin1text, sizeof(latin1text) / 2, buffer, sizeof(buffer), &result);
	gammu_test_result(error, "Encode - 2");
	test_result(strncmp(latin1utf8, buffer, result) == 0);
	test_result(strcmp(latin1utf8, buffer) == 0);

#ifdef ICONV_FOUND
	Priv->Charset = AT_CHARSET_PCCP437;
	error = ATGEN_EncodeText(s, latin1text, sizeof(latin1text) / 2, buffer, sizeof(buffer), &result);
	gammu_test_result(error, "Encode - 3");
	test_result(strncmp(latin1cp437, buffer, result) == 0);
	test_result(strcmp(latin1cp437, buffer) == 0);

	Priv->Charset = AT_CHARSET_ISO88591;
	error = ATGEN_EncodeText(s, latin1text, sizeof(latin1text) / 2, buffer, sizeof(buffer), &result);
	gammu_test_result(error, "Encode - 4");
	test_result(strncmp(latin1iso88591, buffer, result) == 0);
	test_result(strcmp(latin1iso88591, buffer) == 0);

	Priv->Charset = AT_CHARSET_ISO88592;
	error = ATGEN_EncodeText(s, latin2text, sizeof(latin2text) / 2, buffer, sizeof(buffer), &result);
	gammu_test_result(error, "Encode - 5");
	test_result(strncmp(latin2iso88592, buffer, result) == 0);
	test_result(strcmp(latin2iso88592, buffer) == 0);

#endif
	/* Free state machine */
	GSM_FreeStateMachine(s);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
