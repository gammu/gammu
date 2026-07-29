#include <gammu.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "../libgammu/misc/array.h"
#include "../smsd/core.h"
#include "common.h"

static void check_environment(const char *name, const char *expected)
{
#ifdef WIN32
	char actual_buffer[100];
	DWORD length;
	const char *actual;

	actual_buffer[0] = 0;
	SetLastError(ERROR_SUCCESS);
	length = GetEnvironmentVariableA(name, actual_buffer, sizeof(actual_buffer));
	if (length >= sizeof(actual_buffer) ||
	    (length == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND)) {
		actual = NULL;
	} else {
		actual = actual_buffer;
	}
#else
	const char *actual = getenv(name);
#endif

	if (actual == NULL || strcmp(actual, expected) != 0) {
		fprintf(stderr, "Environment mismatch for %s:\nexpected: %s\nactual: %s\n",
			name, expected, actual == NULL ? "(unset)" : actual);
		exit(2);
	}
}

static void set_test_environment(const char *name, const char *value)
{
#ifdef WIN32
	if (!SetEnvironmentVariableA(name, value)) {
#else
	if (setenv(name, value, 1) != 0) {
#endif
		fprintf(stderr, "Failed to set environment variable %s\n", name);
		exit(2);
	}
}

int main(void)
{
	GSM_MultiSMSMessage sms;
	GSM_SMSDConfig config;
	GSM_StringArray sent_ids;

	memset(&sms, 0, sizeof(sms));
	memset(&config, 0, sizeof(config));
	GSM_StringArray_New(&sent_ids);

	config.PhoneID = "modem-a";
	config.gsm = GSM_AllocStateMachine();
	test_result(config.gsm != NULL);

	sms.Number = 2;
	GSM_SetDefaultSMSData(&sms.SMS[0]);
	GSM_SetDefaultSMSData(&sms.SMS[1]);
	sms.SMS[0].PDU = SMS_Status_Report;
	sms.SMS[0].DateTime.Year = 2024;
	sms.SMS[0].DateTime.Month = 1;
	sms.SMS[0].DateTime.Day = 2;
	sms.SMS[0].DateTime.Hour = 3;
	sms.SMS[0].DateTime.Minute = 4;
	sms.SMS[0].DateTime.Second = 5;
	sms.SMS[0].DateTime.Timezone = 90 * 60;
	sms.SMS[0].SMSCTime.Year = 2024;
	sms.SMS[0].SMSCTime.Month = 1;
	sms.SMS[0].SMSCTime.Day = 2;
	sms.SMS[0].SMSCTime.Hour = 4;
	sms.SMS[0].SMSCTime.Minute = 5;
	sms.SMS[0].SMSCTime.Second = 6;
	sms.SMS[0].SMSCTime.Timezone = -2 * 60 * 60;
	sms.SMS[1].PDU = SMS_Deliver;
	memset(&sms.SMS[1].DateTime, 0, sizeof(sms.SMS[1].DateTime));

	test_result(GSM_StringArray_Add(&sent_ids, "123"));
	test_result(GSM_StringArray_Add(&sent_ids, ""));

	SMSD_RunOnEnvironment(&sms, &config, &sent_ids, TRUE);

	check_environment("PHONE_ID", "modem-a");
	check_environment("SMS_MESSAGES", "2");
	check_environment("SMS_1_SENT_ID", "123");
	check_environment("SMS_2_SENT_ID", "");
	check_environment("SMS_1_TIMESTAMP", "1704159245");
	check_environment("SMS_1_DATETIME", "2024-01-02T03:04:05+01:30");
	check_environment("SMS_1_SMSC_TIMESTAMP", "1704175506");
	check_environment("SMS_1_SMSC_DATETIME", "2024-01-02T04:05:06-02:00");
	check_environment("SMS_2_TIMESTAMP", "");
	check_environment("SMS_2_DATETIME", "");
	check_environment("SMS_2_SMSC_TIMESTAMP", "");
	check_environment("SMS_2_SMSC_DATETIME", "");

	SMSD_RunOnEnvironment(&sms, &config, &sent_ids, FALSE);
	check_environment("SMS_1_SENT_ID", "");
	check_environment("SMS_1_TIMESTAMP", "1704159245");
	check_environment("SMS_1_SMSC_TIMESTAMP", "");
	check_environment("SMS_1_SMSC_DATETIME", "");

	set_test_environment("DECODED_PARTS", "1");
	set_test_environment("DECODED_1_TEXT", "stale");
	set_test_environment("DECODED_1_MMS_SENDER", "stale");
	config.PhoneID = "modem-b";
	SMSD_RunOnEnvironment(NULL, &config, NULL, FALSE);
	check_environment("PHONE_ID", "modem-b");
	check_environment("SMS_MESSAGES", "0");
	check_environment("DECODED_PARTS", "0");
	check_environment("SMS_1_CLASS", "");
	check_environment("SMS_1_SENT_ID", "");
	check_environment("SMS_1_TIMESTAMP", "");
	check_environment("SMS_1_DATETIME", "");
	check_environment("SMS_1_SMSC_TIMESTAMP", "");
	check_environment("SMS_1_SMSC_DATETIME", "");
	check_environment("DECODED_1_TEXT", "");
	check_environment("DECODED_1_MMS_SENDER", "");

	GSM_StringArray_Free(&sent_ids);
	GSM_FreeStateMachine(config.gsm);
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
