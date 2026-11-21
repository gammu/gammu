/* Regression tests for AT+CIMI (GSM_GetSIMIMSI) reply parsing. */
#include "test_helper.h"
#include "../../libgammu/gsmstate.h"

#define IMSI "001010123456789"

/*
 * Run one AT+CIMI dialog and check the result.
 *
 * reply is everything the modem sends after echoing "AT+CIMI\r".
 */
static void check_imsi(const char *name, const char *reply,
		GSM_Error expected_error, const char *expected_imsi)
{
	GSM_StateMachine *s = setup_state_machine();
	const char *responses[1];
	/* Room for the result plus a guard area that must stay untouched */
	char imsi[GSM_MAX_INFO_LENGTH + 64];
	const char imei[] = "490154203237518";
	GSM_Error error;
	size_t i;

	printf("%s\n", name);

	setup_at_engine(s);
	bind_response_handling(s);
	responses[0] = reply;
	set_responses(responses, 1);

	/* Pretend the IMEI was already read and cached, as SMSD does */
	strcpy(s->Phone.Data.IMEI, imei);

	memset(imsi, 'X', sizeof(imsi));

	error = GSM_GetSIMIMSI(s, imsi);

	test_result(strcmp((const char *)last_command(), "AT+CIMI\r") == 0);
	test_result(error == expected_error);
	if (expected_imsi != NULL) {
		test_result(strcmp(imsi, expected_imsi) == 0);
	}
	/* Nothing may be written past GSM_MAX_INFO_LENGTH bytes */
	for (i = GSM_MAX_INFO_LENGTH; i < sizeof(imsi); i++) {
		test_result(imsi[i] == 'X');
	}
	/* Parsing the IMSI must not touch the cached IMEI */
	test_result(strcmp(s->Phone.Data.IMEI, imei) == 0);

	cleanup_state_machine(s);
}

int main(void)
{
	char long_reply[GSM_MAX_INFO_LENGTH + 64];

	/* 3GPP TS 27.007: bare digits (e.g. Quectel, SIMCom, Huawei) */
	check_imsi("plain", "\r\n" IMSI "\r\n\r\nOK\r\n", ERR_NONE, IMSI);

	/* Motorola prefixes the reply with the command name */
	check_imsi("motorola", "\r\n+CIMI: " IMSI "\r\n\r\nOK\r\n", ERR_NONE, IMSI);
	check_imsi("motorola-nospace", "\r\n+CIMI:" IMSI "\r\n\r\nOK\r\n", ERR_NONE, IMSI);

	/* Alcatel (BE5) prefixes the reply with "<IMSI>:" */
	check_imsi("alcatel", "\r\n<IMSI>: " IMSI "\r\n\r\nOK\r\n", ERR_NONE, IMSI);
	check_imsi("alcatel-nospace", "\r\n<IMSI>:" IMSI "\r\n\r\nOK\r\n", ERR_NONE, IMSI);

	/* No SIM / SIM not ready */
	check_imsi("error", "\r\nERROR\r\n", ERR_SECURITYERROR, NULL);
	check_imsi("cme-error", "\r\n+CME ERROR: 10\r\n", ERR_NOSIM, NULL);

	/* A garbage line longer than the caller's buffer must be rejected */
	memset(long_reply, 0, sizeof(long_reply));
	strcpy(long_reply, "\r\n");
	memset(long_reply + 2, '9', GSM_MAX_INFO_LENGTH + 10);
	strcat(long_reply, "\r\n\r\nOK\r\n");
	check_imsi("too-long", long_reply, ERR_MOREMEMORY, NULL);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
