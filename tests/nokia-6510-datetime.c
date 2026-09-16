/* Date/time commands must not disrupt phones with F_NODATETIME. */

#include <gammu.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "../libgammu/gsmstate.h"
#include "../libgammu/gsmphones.h"
#include "../libgammu/phone/nokia/dct4s40/6510/n6510.h"

static int writes;

static GSM_Error RecordWrite(GSM_StateMachine *s UNUSED,
		const unsigned char *buffer, size_t length, int type)
{
	test_result(type == 0x19);
	test_result((length == 6 && buffer[3] == 0x0A) ||
		    (length == 18 && buffer[3] == 0x01));
	writes++;
	return ERR_DEVICEWRITEERROR;
}

static void check_datetime(GSM_StateMachine *s, GSM_PhoneModel *model,
		gboolean disabled)
{
	GSM_DateTime date = {0}, original, previous = {0};
	GSM_Error error;
	GSM_Error expected = disabled ? ERR_NOTSUPPORTED : ERR_DEVICEWRITEERROR;

	date.Year = 2026;
	date.Month = 9;
	date.Day = 16;
	date.Hour = 12;
	date.Minute = 34;
	date.Second = 56;
	memcpy(&original, &date, sizeof(date));
	s->Phone.Data.ModelInfo = model;
	test_result(GSM_IsPhoneFeatureAvailable(model, F_NODATETIME) == disabled);

	writes = 0;
	s->Phone.Data.DateTime = &previous;
	s->Phone.Data.RequestID = ID_None;
	error = s->Phone.Functions->GetDateTime(s, &date);
	gammu_test_result_code(error, "Read date/time", expected);
	test_result(writes == (disabled ? 0 : 1));
	test_result(memcmp(&date, &original, sizeof(date)) == 0);
	if (disabled) {
		test_result(s->Phone.Data.DateTime == &previous);
		test_result(s->Phone.Data.RequestID == ID_None);
	}

	writes = 0;
	s->Phone.Data.RequestID = ID_None;
	error = s->Phone.Functions->SetDateTime(s, &date);
	gammu_test_result_code(error, "Set date/time", expected);
	test_result(writes == (disabled ? 0 : 1));
	test_result(memcmp(&date, &original, sizeof(date)) == 0);
	if (disabled) test_result(s->Phone.Data.RequestID == ID_None);
	s->Phone.Data.DateTime = NULL;
}

int main(void)
{
	const char *models[] = {"1200", "1208", "1209"};
	const char *affected[] = {"RH-99", "RH-105"};
	const char *unaffected[] = {"NPL-1", "RM-365", "RM-604", "unknown"};
	GSM_StateMachine *s;
	GSM_Protocol_Functions protocol = {0};
	GSM_PhoneModel synthetic = {0};
	size_t i;

	s = GSM_AllocStateMachine();
	test_result(s != NULL);
	s->CurrentConfig = GSM_GetConfig(s, 0);
	s->CurrentConfig->StartInfo = FALSE;
	s->ReplyNum = 1;
	s->Phone.Functions = &N6510Phone;
	protocol.WriteMessage = RecordWrite;
	s->Protocol.Functions = &protocol;

	for (i = 0; i < sizeof(models) / sizeof(models[0]); i++) {
		check_datetime(s, GetModelData(NULL, models[i], NULL, NULL), TRUE);
	}
	for (i = 0; i < sizeof(affected) / sizeof(affected[0]); i++) {
		check_datetime(s, GetModelData(NULL, NULL, affected[i], NULL), TRUE);
	}
	for (i = 0; i < sizeof(unaffected) / sizeof(unaffected[0]); i++) {
		check_datetime(s, GetModelData(NULL, NULL, unaffected[i], NULL), FALSE);
	}

	/* The flag controls the behavior independently of model identity. */
	synthetic.features[0] = F_NODATETIME;
	check_datetime(s, &synthetic, TRUE);
	synthetic.features[0] = 0;
	check_datetime(s, &synthetic, FALSE);

	GSM_FreeStateMachine(s);
	return 0;
}
