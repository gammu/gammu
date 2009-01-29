/* Test for decoding SMS on Nokia 6510 driver */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include "../common/protocol/protocol.h" /* Needed for GSM_Protocol_Message */
#include "../common/gsmstate.h" /* Needed for state machine internals */
#include "../common/gsmphones.h" /* Phone data */

#include "../helper/message-display.h"

#include "common.h"

unsigned char data[] = {
	0x01, 0x6A, 0x00, 0x03, 0x00, 0x07, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x02, 0x31,
	0x51, 0x00, 0x00, 0xF0, 0x00, 0x04, 0x82, 0x0B, 0x01, 0x07, 0x0A, 0x81, 0x70, 0x60, 0x50, 0x75,
	0x85, 0x82, 0x0C, 0x02, 0x08, 0x07, 0x91, 0x64, 0x07, 0x05, 0x80, 0x99, 0xF9, 0x80, 0x0D, 0x09,
	0x0A, 0x08, 0x0D, 0x02, 0x00, 0x07, 0x0B, 0x02, 0x00, 0x00, 0x08, 0x05, 0x01, 0xFF, 0x00,
	};

/* This is not part of API! */
extern GSM_Error N6510_ReplyGetSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s);

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Debug_Info *debug_info;
	GSM_StateMachine *s;
	GSM_Protocol_Message msg;
	GSM_Error error;
	GSM_MultiSMSMessage sms;

	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, false, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	test_result (s != NULL);

	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(true, debug_info);
	GSM_SetDebugFileDescriptor(stderr, false, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	s->Phone.Data.ModelInfo = GetModelData(NULL, NULL, "unknown", NULL);

	/* Init message */
	msg.Type = 0x14;
	msg.Length = sizeof(data);
	msg.Buffer = data;

	s->Phone.Data.GetSMSMessage = &sms;
	sms.SMS[0].Memory	= MEM_ME;

	/* Parse it */
	error = N6510_ReplyGetSMSMessage(msg, s);

	/* Display message */
	DisplayMultiSMSInfo(&sms, false, true, NULL, NULL);
	DisplayMultiSMSInfo(&sms, true, true, NULL, NULL);

	/* Free state machine */
	GSM_FreeStateMachine(s);

	gammu_test_result(error, "N6510_ReplyGetSMSMessage");

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
