/* Test for decoding SMS on Nokia 6110 driver */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include "../common/protocol/protocol.h" /* Needed for GSM_Protocol_Message */
#include "../common/gsmstate.h" /* Needed for state machine internals */

#include "sms-printing.h"

unsigned char data[] = {
	0x01, 0x08, 0x00, 0x08, 0x01, 0x02, 0x01, 0x00, 0x05, 0x91, 0x26, 0x18, 0x16, 0x42, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x08, 0x0D, 0x91, 0x26, 0x58, 0x26, 0x84, 0x88, 0x65,
	0xF8, 0x00, 0x00, 0x00, 0x70, 0x90, 0x30, 0x10, 0x61, 0x63, 0x82, 0xD4, 0xF2, 0x9C, 0x0E, 0x9A,
	0xB7, 0xE7
	};

/* This is not part of API! */
extern GSM_Error N6110_ReplyGetSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s);

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
	if (s == NULL) {
		printf("Could not allocate state machine!\n");
		return 1;
	}

	/* Init message */
	msg.Type = 0x14;
	msg.Length = sizeof(data);
	msg.Buffer = data;

	s->Phone.Data.GetSMSMessage = &sms;

	/* Parse it */
	error = N6110_ReplyGetSMSMessage(msg, s);

	/* Display message */
	DisplayTestSMS(sms);

	/* Free state machine */
	GSM_FreeStateMachine(s);

	printf("%s\n", GSM_ErrorString(error));

	return (error == ERR_NONE) ? 0 : 1;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
