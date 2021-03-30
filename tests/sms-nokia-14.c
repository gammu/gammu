/* Test for decoding SMS on Nokia 6510 driver */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

#include "../libgammu/protocol/protocol.h"	/* Needed for GSM_Protocol_Message */
#include "../libgammu/gsmstate.h"	/* Needed for state machine internals */
#include "../libgammu/gsmphones.h"	/* Phone data */

#include "../helper/message-display.h"

unsigned char data[] = {
    0x01, 0x5D, 0x00, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const char text[] = "";
char decoded_text[200];

/* This is not part of API! */
extern GSM_Error N6510_ReplyGetSMSMessage(GSM_Protocol_Message *msg, GSM_StateMachine * s);

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Debug_Info *debug_info;
	GSM_StateMachine *s;
	GSM_Protocol_Message msg;
	GSM_Error error;
	GSM_MultiSMSMessage sms;

	/* Init locales for proper output */
	GSM_InitLocales(NULL);

	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	test_result(s != NULL);

	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(TRUE, debug_info);
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	s->Phone.Data.ModelInfo = GetModelData(NULL, NULL, "unknown", NULL);

	/* Init message */
	msg.Type = 0x14;
	msg.Length = sizeof(data);
	msg.Buffer = data;

	s->Phone.Data.GetSMSMessage = &sms;

	/* Parse it */
	error = N6510_ReplyGetSMSMessage(&msg, s);

	/* Display message */
	DisplayMultiSMSInfo(&sms, FALSE, TRUE, NULL, NULL);
	DisplayMultiSMSInfo(&sms, TRUE, TRUE, NULL, NULL);

	/* Free state machine */
	GSM_FreeStateMachine(s);

	/* Check expected text */
	/* We do not compare full text due to locales problem */
	EncodeUTF8(decoded_text, sms.SMS[0].Text);
	test_result(strcmp(text, decoded_text) == 0);

	gammu_test_result(error, "N6510_ReplyGetSMSMessage");

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
