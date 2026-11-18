/* Regression tests for SIM ICCID reading (GSM_GetSIMICCID) on AT phones. */
#include "test_helper.h"
#include "../../libgammu/gsmstate.h"

/* Test ICCIDs: 89 (telecom), 001 (test network country code) */
#define ICCID19 "8900101234567890123"
#define ICCID20 "89001012345678901234"
/* EF_ICCID content (swapped BCD, F padded) of ICCID19 */
#define EF_ICCID19 "980001214365870921F3"

#define CCID "AT+CCID\r"
#define CRSM "AT+CRSM=176,12258,0,0,10\r"

/*
 * Run one dialog and check the result.
 *
 * replies are the modem answers to the commands, without the echo.
 * last is the last command expected to be sent.
 */
static void check_iccid(const char *name, const char **replies, size_t count,
		const char *last, GSM_Error expected_error, const char *expected_iccid)
{
	GSM_StateMachine *s = setup_state_machine();
	/* Room for the result plus a guard area that must stay untouched */
	char iccid[GSM_MAX_INFO_LENGTH + 64];
	GSM_Error error;
	size_t i;

	printf("%s\n", name);

	setup_at_engine(s);
	bind_response_handling(s);
	set_responses(replies, count);

	memset(iccid, 'X', sizeof(iccid));

	error = GSM_GetSIMICCID(s, iccid);

	test_result(strcmp((const char *)last_command(), last) == 0);
	test_result(error == expected_error);
	if (expected_iccid != NULL) {
		test_result(strcmp(iccid, expected_iccid) == 0);
	}
	/* Nothing may be written past GSM_MAX_INFO_LENGTH bytes */
	for (i = GSM_MAX_INFO_LENGTH; i < sizeof(iccid); i++) {
		test_result(iccid[i] == 'X');
	}

	cleanup_state_machine(s);
}

#define CHECK(name, replies, last, error, iccid) \
	check_iccid(name, replies, sizeof(replies) / sizeof(*(replies)), last, error, iccid)

int main(void)
{
	/* AT+CCID replies seen on various modules */
	const char *quectel[] = {"\r\n+CCID: " ICCID19 "F\r\n\r\nOK\r\n"};
	const char *quoted[] = {"\r\n+CCID: \"" ICCID20 "\"\r\n\r\nOK\r\n"};
	const char *bare[] = {"\r\n" ICCID20 "\r\n\r\nOK\r\n"};
	const char *iccid_prefix[] = {"\r\n+ICCID: " ICCID19 "\r\n\r\nOK\r\n"};
	const char *huawei[] = {"\r\n^ICCID: " ICCID19 "f\r\n\r\nOK\r\n"};

	/* AT+CCID not supported, fall back to reading EF_ICCID */
	const char *crsm_after_error[] = {
		"\r\nERROR\r\n",
		"\r\n+CRSM: 144,0,\"" EF_ICCID19 "\"\r\n\r\nOK\r\n"};
	const char *crsm_after_cme[] = {
		"\r\n+CME ERROR: 4\r\n",
		"\r\n+CRSM: 144,0," EF_ICCID19 "\r\n\r\nOK\r\n"};
	const char *crsm_after_garbage[] = {
		"\r\n+CCID: HELLO\r\n\r\nOK\r\n",
		"\r\n+CRSM: 145,10,\"" EF_ICCID19 "\"\r\n\r\nOK\r\n"};
	const char *crsm_file_not_found[] = {
		"\r\nERROR\r\n",
		"\r\n+CRSM: 106,130\r\n\r\nOK\r\n"};
	const char *crsm_error[] = {
		"\r\nERROR\r\n",
		"\r\nERROR\r\n"};
	const char *crsm_invalid[] = {
		"\r\nERROR\r\n",
		"\r\n+CRSM: 144,0,\"98000121436587092\"\r\n\r\nOK\r\n"};

	/* No SIM: reported as such, no fallback */
	const char *no_sim[] = {"\r\n+CME ERROR: 10\r\n"};

	/* A garbage line longer than the caller's buffer must be rejected */
	char long_reply[GSM_MAX_INFO_LENGTH + 64];
	const char *too_long[] = {long_reply};

	CHECK("quectel", quectel, CCID, ERR_NONE, ICCID19);
	CHECK("quoted", quoted, CCID, ERR_NONE, ICCID20);
	CHECK("bare", bare, CCID, ERR_NONE, ICCID20);
	CHECK("iccid-prefix", iccid_prefix, CCID, ERR_NONE, ICCID19);
	CHECK("huawei", huawei, CCID, ERR_NONE, ICCID19);

	CHECK("crsm-after-error", crsm_after_error, CRSM, ERR_NONE, ICCID19);
	CHECK("crsm-after-cme", crsm_after_cme, CRSM, ERR_NONE, ICCID19);
	CHECK("crsm-after-garbage", crsm_after_garbage, CRSM, ERR_NONE, ICCID19);
	CHECK("crsm-file-not-found", crsm_file_not_found, CRSM, ERR_NOTSUPPORTED, NULL);
	CHECK("crsm-error", crsm_error, CRSM, ERR_NOTSUPPORTED, NULL);
	CHECK("crsm-invalid", crsm_invalid, CRSM, ERR_UNKNOWNRESPONSE, NULL);

	CHECK("no-sim", no_sim, CCID, ERR_NOSIM, NULL);

	memset(long_reply, 0, sizeof(long_reply));
	strcpy(long_reply, "\r\n");
	memset(long_reply + 2, '9', GSM_MAX_INFO_LENGTH + 10);
	strcat(long_reply, "\r\n\r\nOK\r\n");
	CHECK("too-long", too_long, CCID, ERR_MOREMEMORY, NULL);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
