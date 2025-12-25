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
    0x01, 0x6B, 0x00, 0x03, 0x00, 0x01, 0x02, 0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x80,
    0x00, 0x00, 0x08, 0x66, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x82, 0x0C,
    0x01, 0x08, 0x0B, 0x91, 0x97, 0x60, 0x60, 0x82, 0x87, 0xF0, 0x82, 0x08, 0x02, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x80, 0x5C, 0x56, 0x2B, 0x04, 0x23, 0x00, 0x20, 0x04, 0x3C, 0x04, 0x35, 0x04, 0x3D,
    0x04, 0x4F, 0x00, 0x20, 0x04, 0x38, 0x04, 0x41, 0x04, 0x42, 0x04, 0x35, 0x04, 0x40, 0x04, 0x38,
    0x04, 0x3A, 0x04, 0x30, 0x00, 0x2E, 0x04, 0x1E, 0x04, 0x42, 0x04, 0x3F, 0x04, 0x38, 0x04, 0x48,
    0x04, 0x38, 0x04, 0x42, 0x04, 0x35, 0x04, 0x41, 0x04, 0x4C, 0x00, 0x2C, 0x04, 0x3A, 0x04, 0x30,
    0x04, 0x3A, 0x00, 0x20, 0x04, 0x34, 0x04, 0x3E, 0x04, 0x3B, 0x04, 0x35, 0x04, 0x42, 0x04, 0x38,
    0x04, 0x42, 0x04, 0x35, 0x00, 0x2C, 0x04, 0x3E, 0x04, 0x3A, 0x00, 0x3F, 0x00, 0x00
};

const char text[] = "У меня истерика.Отпишитесь,как долетите,ок?";
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

    test_result(sms.SMS[0].DateTime.Year == 0);

	gammu_test_result(error, "N6510_ReplyGetSMSMessage");

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
