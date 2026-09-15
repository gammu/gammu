/* Regression tests for vendor/family-specific CME and CMS decoding. */
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include "test_helper.h"
#include "../../libgammu/gsmstate.h"
#include "../../libgammu/protocol/at/at.h"
#include "../../libgammu/phone/at/at-error.h"

static int callbacks, callback_status;

static void sms_status(GSM_StateMachine *s UNUSED, int status, int reference,
		void *user_data UNUSED)
{
	callbacks++;
	callback_status = status;
	test_result(reference == -1);
}

static void feed(GSM_StateMachine *s, const char *reply)
{
	while (*reply != '\0') {
		test_result(AT_StateMachine(s, (unsigned char)*reply++) == ERR_NONE);
	}
}

static void check_reply(GSM_StateMachine *s, GSM_AT_Manufacturer vendor,
		gboolean cms, const char *value, int code, const char *text,
		GSM_Error expected, gboolean valid)
{
	GSM_Phone_ATGENData *priv = &s->Phone.Data.Priv.ATGEN;
	char reply[1024];
	int previous = callbacks;

	fprintf(stderr, "vendor=%d %s: %s\n", vendor, cms ? "CMS" : "CME", value);
	priv->Manufacturer = vendor;
	s->Phone.Data.RequestID = ID_IncomingFrame;
	/* Model the final reply to a submitted SMS; the command echo selects the handler. */
	s->Protocol.Data.AT.EditMode = TRUE;
	snprintf(reply, sizeof(reply), "AT+CMGS=33\r\r\n+CM%c ERROR: %s\r\n",
		cms ? 'S' : 'E', value);
	feed(s, reply);
	test_result(priv->ReplyState == (cms ? AT_Reply_CMSError : AT_Reply_CMEError));
	test_result(priv->ErrorCodeValid == valid);
	test_result(priv->ErrorCode == code);
	if (text != NULL) {
		test_result(priv->ErrorText != NULL);
		test_result(strcmp(priv->ErrorText, text) == 0);
	} else {
		test_result(priv->ErrorText == NULL);
	}
	test_result(s->Phone.Data.DispatchError == expected);
	test_result(s->Phone.Data.RequestID == ID_None);
	test_result(!s->Protocol.Data.AT.EditMode);
	test_result(callbacks == previous + 1);
	test_result(callback_status == code);
}

static void test_vendor_codes(GSM_StateMachine *s)
{
	static const struct {
		GSM_AT_Manufacturer vendor;
		gboolean cms;
		int code;
		const char *text;
		GSM_Error error;
	} cases[] = {
		{AT_Wavecom, TRUE, 512, "MM establishment failure (for SMS)", ERR_NETWORK_ERROR},
		{AT_Wavecom, TRUE, 514, "CP error (for SMS)", ERR_NETWORK_ERROR},
		{AT_Wavecom, TRUE, 515, "Please wait, init or command processing in progress", ERR_BUSY},
		{AT_Wavecom, FALSE, 515, "Please wait, init or command processing in progress", ERR_BUSY},
		{AT_Siemens, TRUE, 512, "User abort", ERR_CANCELED},
		{AT_Siemens, TRUE, 515, "invalid character in address string", ERR_INVALIDDATA},
		{AT_Siemens, TRUE, 516, "invalid length", ERR_INVALIDDATA},
		{AT_Siemens, FALSE, 257, "call barred", ERR_SECURITYERROR},
		{AT_Siemens, FALSE, 615, "network failure", ERR_NETWORK_ERROR},
		{AT_Siemens, FALSE, 764, "missing input value", ERR_INVALIDDATA},
		{AT_Motorola, TRUE, 512, "Network busy", ERR_NETWORK_ERROR},
		{AT_Motorola, TRUE, 516, "Invalid preferred memory storage", ERR_INVALIDLOCATION},
		{AT_Motorola, FALSE, 268, "Cancelled by user", ERR_CANCELED},
		{AT_Huawei, FALSE, 257, "network rejected request", ERR_NETWORK_ERROR},
		{AT_Huawei, FALSE, 65287, "another SPN query operation still not finished", ERR_BUSY},
		{AT_Falcom, FALSE, 258, "Phone is busy", ERR_BUSY},
		{AT_Falcom, FALSE, 588, "GPRS - feature not supported", ERR_NOTSUPPORTED},
		{AT_ITegno, FALSE, 513, "Lower Layer Failure (for SMS)", ERR_NETWORK_ERROR},
		{AT_ITegno, TRUE, 513, "ACM Reset Needed", ERR_UNKNOWN},
		{AT_Quectel, TRUE, 512, "SIM not ready", ERR_BUSY},
		{AT_Quectel, TRUE, 515, "ME storage failure", ERR_MEMORY},
		{AT_Quectel, FALSE, 904, "Audio device busy", ERR_BUSY},
		{AT_SIMCom, TRUE, 512, "SMS no error", ERR_UNKNOWN},
		{AT_SIMCom, TRUE, 532, "Doing SIM refresh", ERR_BUSY},
		{AT_SIMCom, FALSE, 810, "No Error", ERR_UNKNOWN},
		{AT_SIMCom, FALSE, 160, "DNS resolve failed", ERR_NETWORK_ERROR},
		{AT_Telit, TRUE, 512, "No SM resources", ERR_BUSY},
		{AT_Telit, TRUE, 515, "No response from network", ERR_NETWORK_ERROR},
		{AT_Telit, FALSE, 551, "wrong state", ERR_UNKNOWN},
		{AT_Telit, FALSE, 615, "FTP not connected", ERR_NETWORK_ERROR},
		{AT_Sierra, TRUE, 606, "ME Busy - CM server request already pending", ERR_BUSY},
		{AT_Sierra, FALSE, 904, "DNS error", ERR_NETWORK_ERROR},
		{AT_Sierra, FALSE, 910, "Bad session ID", ERR_INVALIDDATA},
		{AT_UBlox, FALSE, 2000, "Command timeout", ERR_TIMEOUT},
		{AT_UBlox, FALSE, 3000, "Command aborted", ERR_CANCELED},
		{AT_UBlox, TRUE, 512, "Relay Protocol Acknowledgement", ERR_UNKNOWN},
		{AT_UBlox, TRUE, 539, "MS pending MO SM transfer", ERR_BUSY},
		{AT_UBlox, TRUE, 612, "SIM Toolkit Request Is Rejected, Because Another SIM Toolkit Request Is Pending", ERR_BUSY},
		{AT_Samsung, FALSE, -1, "[Samsung] Empty location", ERR_EMPTY},
		{AT_Ericsson, FALSE, 601, "Operation not supported", ERR_NOTSUPPORTED}
	};
	size_t i, j;
	char value[512];

	for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		snprintf(value, sizeof(value), "%d", cases[i].code);
		check_reply(s, cases[i].vendor, cases[i].cms, value, cases[i].code,
			cases[i].text, cases[i].error, TRUE);
		/* Case and surrounding whitespace must not affect verbose decoding. */
		snprintf(value, sizeof(value), "  %s \t", cases[i].text);
		for (j = 0; value[j] != '\0'; j++) {
			value[j] = toupper((unsigned char)value[j]);
		}
		check_reply(s, cases[i].vendor, cases[i].cms, value, cases[i].code,
			cases[i].text, cases[i].error, TRUE);
	}
}

static void test_unknown_and_malformed(GSM_StateMachine *s)
{
	static const char *malformed[] = {"", " ", "+", "-", "512junk", "512 0",
		"999999999999999999999999", "-99999999999999999999999", "@512",
		"SIM not ready suffix", "SIM not", "User abort", "Unknown future error"};
	size_t i;
	char value[64];

	for (i = 0; i < sizeof(malformed) / sizeof(malformed[0]); i++) {
		check_reply(s, AT_Quectel, TRUE, malformed[i], -1, NULL, ERR_UNKNOWN, FALSE);
	}
	check_reply(s, AT_Unknown, TRUE, "512", 512, NULL, ERR_UNKNOWN, TRUE);
	check_reply(s, AT_Unknown, TRUE, "User abort", -1, NULL, ERR_UNKNOWN, FALSE);
	check_reply(s, AT_Nokia, TRUE, "512", 512, NULL, ERR_UNKNOWN, TRUE);
	check_reply(s, AT_Wavecom, FALSE, "512", 512, NULL, ERR_UNKNOWN, TRUE);
	check_reply(s, AT_Siemens, TRUE, "257", 257, NULL, ERR_UNKNOWN, TRUE);
	check_reply(s, AT_Siemens, TRUE, "615", 615, NULL, ERR_UNKNOWN, TRUE);
	check_reply(s, AT_Unknown, FALSE, "515", 515, NULL, ERR_UNKNOWN, TRUE);
	check_reply(s, AT_Unknown, FALSE, "601", 601, NULL, ERR_UNKNOWN, TRUE);
	/* Telit uses the same verbose description for 551 and 601. Numeric replies
	 * retain the actual code; verbose replies use the first documented match. */
	check_reply(s, AT_Telit, FALSE, "601", 601, "wrong state", ERR_UNKNOWN, TRUE);
	check_reply(s, AT_Unknown, FALSE, "-1", -1, NULL, ERR_UNKNOWN, TRUE);
	check_reply(s, AT_Samsung, TRUE, "-1", -1, NULL, ERR_UNKNOWN, TRUE);
	check_reply(s, AT_Samsung, FALSE, "-1x", -1, NULL, ERR_UNKNOWN, FALSE);
	check_reply(s, AT_Samsung, FALSE, " -1 \t", -1, "[Samsung] Empty location", ERR_EMPTY, TRUE);
	check_reply(s, AT_Wavecom, TRUE, " +512 \t", 512, "MM establishment failure (for SMS)", ERR_NETWORK_ERROR, TRUE);
	check_reply(s, AT_Unknown, FALSE, "0", 0, "phone failure", ERR_UNKNOWN, TRUE);
	snprintf(value, sizeof(value), "%d", INT_MAX);
	check_reply(s, AT_Unknown, TRUE, value, INT_MAX, NULL, ERR_UNKNOWN, TRUE);
	snprintf(value, sizeof(value), "%d", INT_MIN);
	check_reply(s, AT_Unknown, TRUE, value, INT_MIN, NULL, ERR_UNKNOWN, TRUE);
	snprintf(value, sizeof(value), "%llu", (unsigned long long)INT_MAX + 1);
	check_reply(s, AT_Unknown, TRUE, value, -1, NULL, ERR_UNKNOWN, FALSE);
	snprintf(value, sizeof(value), "-%llu", (unsigned long long)INT_MAX + 2);
	check_reply(s, AT_Unknown, TRUE, value, -1, NULL, ERR_UNKNOWN, FALSE);
	/* Reset state on a successful reply, too. */
	s->Phone.Data.RequestID = ID_IncomingFrame;
	feed(s, "AT\r\r\nOK\r\n");
	test_result(s->Phone.Data.Priv.ATGEN.ErrorText == NULL);
	test_result(!s->Phone.Data.Priv.ATGEN.ErrorCodeValid);
}

static void test_standard_codes(GSM_StateMachine *s)
{
	GSM_AT_Manufacturer vendor;
	for (vendor = AT_Nokia; vendor <= AT_ITegno; vendor++) {
		/* CMS zero retains the legacy internal-error result. CME zero has
		 * a separate mapping; neither is a missing or malformed code. */
		check_reply(s, vendor, TRUE, "0", 0, "Unspecified SMS failure", ERR_PHONE_INTERNAL, TRUE);
		check_reply(s, vendor, FALSE, "0", 0, "phone failure", ERR_UNKNOWN, TRUE);
		check_reply(s, vendor, TRUE, "331", 331, "no network service", ERR_NETWORK_ERROR, TRUE);
		check_reply(s, vendor, FALSE, "22", 22, "not found", ERR_EMPTY, TRUE);
		check_reply(s, vendor, TRUE, "304", 304, "invalid PDU mode parameter", ERR_NOTSUPPORTED, TRUE);
		check_reply(s, vendor, FALSE, "23", 23, "memory failure", ERR_MEMORY, TRUE);
		check_reply(s, vendor, TRUE, "NO NETWORK SERVICE", 331,
			"no network service", ERR_NETWORK_ERROR, TRUE);
	}
}

static void test_identification(GSM_StateMachine *s)
{
	static const struct {
		const char *name;
		GSM_AT_Manufacturer vendor;
	} cases[] = {
		{"WAVECOM", AT_Wavecom}, {"SIEMENS", AT_Siemens},
		{"MOTOROLA", AT_Motorola}, {"HUAWEI", AT_Huawei},
		{"FALCOM", AT_Falcom}, {"iTegno", AT_ITegno}, {"iWOW", AT_ITegno},
		{"Quectel", AT_Quectel}, {"SIMCOM_Ltd", AT_SIMCom},
		{"Sierra Wireless, Incorporated", AT_Sierra}, {"u-blox", AT_UBlox},
		{"Telit", AT_Telit}, {"Samsung", AT_Samsung}, {"Sony Ericsson", AT_Ericsson},
		{"Unrecognized vendor", AT_Unknown}
	};
	size_t i;
	char reply[256];
	for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		s->Phone.Data.RequestID = ID_GetManufacturer;
		snprintf(reply, sizeof(reply), "AT+CGMI\r\r\n%s\r\nOK\r\n", cases[i].name);
		feed(s, reply);
		test_result(s->Phone.Data.Priv.ATGEN.Manufacturer == cases[i].vendor);
		test_result(s->Phone.Data.DispatchError == ERR_NONE);
	}
}

int main(void)
{
	GSM_StateMachine *s = setup_state_machine();
	setup_at_engine(s);
	GSM_SetSendSMSStatusCallback(s, sms_status, NULL);
	test_vendor_codes(s);
	test_unknown_and_malformed(s);
	test_standard_codes(s);
	test_identification(s);
	cleanup_state_machine(s);
	return 0;
}
