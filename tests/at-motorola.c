/* Test for decoding Motorola replies */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"
#include "../libgammu/phone/at/atgen.h"
#include "../libgammu/phone/at/motorola.h"
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
    char buff[2048];
    size_t pos;

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
	Priv->Manufacturer = AT_Motorola;

	/* Perform real tests */
	error = MOTOROLA_ParseCalendarSimple(s, "+MDBR: 0,\"Meeting\",1,0,\"17:00\",\"02-24-2006\",60,\"00:00\",\"00-00-2000\",0");
	gammu_test_result(error, "Calendar 1");

    pos = 0;
    error = GSM_EncodeVCALENDAR(buff, sizeof(buff), &pos, &entry, FALSE, SonyEricsson_VCalendar);
	gammu_test_result(error, "Encode Calendar 1");

    printf("%s\n", buff);

	error = MOTOROLA_ParseCalendarSimple(s, "+MDBR: 1,\"Breakfast\",1,1,\"10:00\",\"02-25-2006\",60,\"09:30\",\"02-25-2006\",2");
	gammu_test_result(error, "Calendar 2");

    pos = 0;
    error = GSM_EncodeVCALENDAR(buff, sizeof(buff), &pos, &entry, FALSE, SonyEricsson_VCalendar);
	gammu_test_result(error, "Encode Calendar 2");

    printf("%s\n", buff);

	/* Free state machine */
	GSM_FreeStateMachine(s);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

