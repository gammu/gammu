/* Test for decoding SMS on AT driver */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../common/phone/at/atgen.h"
#include "../common/protocol/protocol.h" /* Needed for GSM_Protocol_Message */
#include "../common/gsmstate.h" /* Needed for state machine internals */
#include "../common/gsmphones.h" /* Phone data */

#define BUFFER_SIZE 16384

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Debug_Info *debug_info;
	GSM_Phone_ATGENData *Priv;
	GSM_Phone_Data *Data;
	GSM_DateTime dt;
	unsigned char buffer[BUFFER_SIZE];
	int i;
	GSM_StateMachine *s;
	GSM_Error error;

	/* Configure state machine */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL) {
		printf("Could not allocate state machine!\n");
		return 1;
	}
	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(true, debug_info);
	GSM_SetDebugFileDescriptor(stderr, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Initialize AT engine */
	Data = &s->Phone.Data;
	Data->ModelInfo = GetModelData(NULL, NULL, "unknown", NULL);
	Priv = &s->Phone.Data.Priv.ATGEN;
	Priv->ReplyState = AT_Reply_OK;
	Priv->SMSMode = SMS_AT_PDU;
	Priv->Charset = AT_CHARSET_GSM;

	/* Perform real tests */
	error = ATGEN_ParseReply(s,
			"+CPBR: 1,,\"+31234657899\",145,\"Mama GSM\",\"2007/11/02,09:27\"",
			"+CPBR: @i, @s, @p, @i, @s, @d",
			&i, buffer, BUFFER_SIZE, buffer, BUFFER_SIZE, &i, buffer, BUFFER_SIZE, &dt
			);
	if (error != ERR_NONE) {
		printf("%s\n", GSM_ErrorString(error));
		return 1;
	}
	error = ATGEN_ParseReply(s,
			"+CPBR: 6,\"\",,\"005300740061006E006C006500790020005000610075006C\"",
			"+CPBR: @i, @p, @I, @s",
			&i, buffer, BUFFER_SIZE, &i, buffer, BUFFER_SIZE);
	if (error != ERR_NONE) {
		printf("%s\n", GSM_ErrorString(error));
		return 1;
	}

	error = ATGEN_ParseReply(s,
			"+CPBR: 1,\"+60122256476\",145,\"\",\"08/07/04\",\"17:24:55\"",
			"+CPBR: @i, @p, @I, @s, @d",
			&i, buffer, BUFFER_SIZE, &i, buffer, BUFFER_SIZE);
	if (error != ERR_NONE) {
		printf("%s\n", GSM_ErrorString(error));
		return 1;
	}

	/* Free state machine */
	GSM_FreeStateMachine(s);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
