/* Test for decoding SMS on AT driver */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "../libgammu/phone/at/atgen.h"
#include "../libgammu/protocol/protocol.h" /* Needed for GSM_Protocol_Message */
#include "../libgammu/gsmstate.h" /* Needed for state machine internals */
#include "../libgammu/gsmphones.h" /* Phone data */

#define BUFFER_SIZE ((size_t)16384)

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
	gammu_test_result(error, "+CPBR: @i, @s, @p, @i, @s, @d");

	error = ATGEN_ParseReply(s,
			"+CPBR: 6,\"\",,\"005300740061006E006C006500790020005000610075006C\"",
			"+CPBR: @i, @p, @I, @s",
			&i, buffer, BUFFER_SIZE, &i, buffer, BUFFER_SIZE);
	gammu_test_result(error, "+CPBR: @i, @p, @I, @s");

	error = ATGEN_ParseReply(s,
			"+CPBR: 1,\"+60122256476\",145,\"\",\"08/07/04\",\"17:24:55\"",
			"+CPBR: @i, @p, @I, @s, @d",
			&i, buffer, BUFFER_SIZE, &i, buffer, BUFFER_SIZE, &dt);
	gammu_test_result(error, "+CPBR: @i, @p, @I, @s, @d");

	Priv->Charset = AT_CHARSET_UCS2;
	Priv->Manufacturer = AT_Motorola;
	error = ATGEN_ParseReply(s,
			"+CPBR: 419,\"030450566735\",129,56697263686F77204B6C696E696B756D2053742E20333100",
			"+CPBR: @i, @p, @I, @s",
			&i, buffer, BUFFER_SIZE, &i, buffer, BUFFER_SIZE);
	gammu_test_result(error, "+CPBR: @i, @p, @I, @s");

	Priv->Charset = AT_CHARSET_UCS2;
	Priv->Manufacturer = AT_Motorola;
	error = ATGEN_ParseReply(s,
			"+CPBR: 419,\"030450566735\",129,56697263686F77204B6C696E696B756D2053742E20333100",
			"+CPBR: @i, @p, @I, @e",
			&i, buffer, BUFFER_SIZE, &i, buffer, BUFFER_SIZE);
	gammu_test_result(error, "+CPBR: @i, @p, @I, @e");

	Priv->Charset = AT_CHARSET_UCS2;
	Priv->Manufacturer = AT_Motorola;
	error = ATGEN_ParseReply(s,
			"+CPBR: 212,\"mlaubner@web.de\",128,004D0061006E00660072006500640020004C006100750062006E00650072",
			"+CPBR: @i, @p, @I, @s",
			&i, buffer, BUFFER_SIZE, &i, buffer, BUFFER_SIZE);
	gammu_test_result(error, "+CPBR: @i, @p, @I, @s");

	Priv->Charset = AT_CHARSET_HEX;
	Priv->Manufacturer = AT_Motorola;
	error = ATGEN_ParseReply(s,
			"+CSCA: 002B003300380030003600330039003000310030003000300030,145",
			"+CSCA: @p, @i",
			buffer, BUFFER_SIZE, &i);
	gammu_test_result(error, "+CSCA: @p, @i");

	Priv->Charset = AT_CHARSET_GSM;
	Priv->Manufacturer = AT_Motorola;
	error = ATGEN_ParseReply(s,
					"+CPBR: (1-250),50,14,,,,40,20",
					"+CPBR: (@i-@i), @i, @i, @0",
			&i, &i, &i, &i);
	gammu_test_result(error, "+CPBR: (@i-@i), @i, @i, @0");

	error = ATGEN_ParseReply(s,
					"+CMGL:1,0,,26",
					"+CMGL: @i, @r, @p, @0",
					&i,
					buffer, BUFFER_SIZE,
					buffer, BUFFER_SIZE);
	gammu_test_result(error, "+CMGL: @i, @r, @p, @0");

	Priv->Charset = AT_CHARSET_GSM;
	Priv->Manufacturer = AT_Motorola;
	error = ATGEN_ParseReply(s,
					"+SPBR: 10,\"12345\",\"\",\"\",\"\",\"\",\"\",\"5,test1\",\"0,\",\"0,\",\"0,\"",
					"+SPBR: @i, @p, @p, @p, @p, @p, @s, @t, @t, @t, @t",
			&i,
			buffer, BUFFER_SIZE,
			buffer, BUFFER_SIZE,
			buffer, BUFFER_SIZE,
			buffer, BUFFER_SIZE,
			buffer, BUFFER_SIZE,
			buffer, BUFFER_SIZE,
			buffer, BUFFER_SIZE,
			buffer, BUFFER_SIZE,
			buffer, BUFFER_SIZE,
			buffer, BUFFER_SIZE
			);
	gammu_test_result(error, "+SPBR: @i, @p, @p, @p, @p, @p, @s, @t, @t, @t, @t");

	/* Free state machine */
	GSM_FreeStateMachine(s);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
