/* Test for decoding Samsung replies */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "../libgammu/phone/at/atgen.h"
#include "../libgammu/phone/at/samsung.h"
#include "../libgammu/protocol/protocol.h" /* Needed for GSM_Protocol_Message */
#include "../libgammu/gsmstate.h" /* Needed for state machine internals */
#include "../libgammu/gsmphones.h" /* Phone data */

#define BUFFER_SIZE ((size_t)16384)

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Debug_Info *debug_info;
	GSM_Phone_ATGENData *Priv;
	GSM_Phone_Data *Data;
	GSM_StateMachine *s;
	GSM_Error error;
	GSM_CalendarEntry entry;

	/* Init locales to get proper encoding */
	GSM_InitLocales(NULL);

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
	Data->Cal = &entry;
	Priv = &s->Phone.Data.Priv.ATGEN;
	Priv->ReplyState = AT_Reply_OK;
	Priv->SMSMode = SMS_AT_PDU;
	Priv->Charset = AT_CHARSET_UTF8;
	Priv->Manufacturer = AT_Samsung;

	/* Perform real tests */
	error = SAMSUNG_ParseAniversary(s, "+ORGR: 67,2,,\"Laura Santiesteban Cabrera\",3,11,2009,9,0,,,,,,,1,3,0,4,,,,,");
	gammu_test_result(error, "Aniversary 1");

	error = SAMSUNG_ParseTask(s, "+ORGR: 205,3,,\"Cemento\",13,3,2009,10,35,13,3,2009,,,,1,3,0,0,1,0,,,");
	gammu_test_result(error, "Task 1");

	error = SAMSUNG_ParseAppointment(s, "+ORGR: 161,1,\"Comprar lagrimas artificiales\",\"Farmacia\",2,4,2009,9,0,2,4,2009,9,10,\"Farmacia\",1,1,0,3,,,29,1,2010");
	gammu_test_result(error, "Appointment 1");

	error = SAMSUNG_ParseAppointment(s, "+ORGR: 235,4,\"Curso\",\"Averiguar\",13,3,2009,9,50,13,3,2009,9,59,,1,1,0,,,,,,");
	gammu_test_result(error, "Appointment 2");

	/* Free state machine */
	GSM_FreeStateMachine(s);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
