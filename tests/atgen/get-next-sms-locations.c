#include <assert.h>
#include <gammu-message.h>

#include "test_helper.h"

#include "../../libgammu/gsmstate.h"

GSM_Error ATGEN_GetNextSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, gboolean start);

static const char *sms_pdu =
	"07918497483252F0040B918496445078F700007121320144744004D4F29C0E\r";

static void setup_sms_memory(GSM_Phone_ATGENData *Priv, GSM_MemoryType memory)
{
	Priv->SIMSMSMemory = memory == MEM_SM ? AT_AVAILABLE : AT_NOTAVAILABLE;
	Priv->PhoneSMSMemory = memory == MEM_ME ? AT_AVAILABLE : AT_NOTAVAILABLE;
	Priv->SRSMSMemory = AT_NOTAVAILABLE;
	Priv->SIMSaveSMS = AT_NOTAVAILABLE;
	Priv->PhoneSaveSMS = AT_NOTAVAILABLE;
	Priv->SMSMemory = memory;
	Priv->SMSMemoryWrite = FALSE;
	Priv->NumFolders = 1;
}

static void cleanup_sms_locations(GSM_StateMachine *s)
{
	cleanup_state_machine(s);
}

static void supported_range_stops_at_reported_count(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_PhoneModel model;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* AT+CPMS="SM" */
		"+CPMS: 1,5000\r",
		"OK\r\n",
		/* AT+CMGD=? */
		"+CMGD: (1-5000),(0-4)\r",
		"OK\r\n",
		/* AT+CMGR=1 */
		"+CMGR: 1,\"\",23\r",
		sms_pdu,
		"OK\r\n",
		/* Status refresh for the second GetNextSMS call. */
		"+CPMS: 1,5000\r",
		"OK\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	model = *s->Phone.Data.ModelInfo;
	s->Phone.Data.ModelInfo = &model;
	GSM_AddPhoneFeature(&model, F_DISABLE_CMGL);
	setup_sms_memory(Priv, MEM_SM);
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_NONE);
	test_result(sms.SMS[0].Location == 1);
	test_result(strcmp((const char *)last_command(), "AT+CMGR=1\r") == 0);

	error = ATGEN_GetNextSMS(s, &sms, FALSE);
	test_result(error == ERR_EMPTY);
	test_result(strcmp((const char *)last_command(), "AT+CPMS=\"SM\"\r") == 0);

	cleanup_sms_locations(s);
}

static void shrinking_live_count_does_not_stop_scan(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_PhoneModel model;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* Initial status before reading two messages. */
		"+CPMS: 2,30\r",
		"OK\r\n",
		/* AT+CMGD=? */
		"+CMGD: (1-30),(0-4)\r",
		"OK\r\n",
		/* AT+CMGR=1 */
		"+CMGR: 1,\"\",23\r",
		sms_pdu,
		"OK\r\n",
		/* The caller deleted location 1 before asking for the next SMS. */
		"+CPMS: 1,30\r",
		"OK\r\n",
		/* AT+CMGR=2 */
		"+CMGR: 1,\"\",23\r",
		sms_pdu,
		"OK\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	model = *s->Phone.Data.ModelInfo;
	s->Phone.Data.ModelInfo = &model;
	GSM_AddPhoneFeature(&model, F_DISABLE_CMGL);
	setup_sms_memory(Priv, MEM_SM);
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_NONE);
	test_result(sms.SMS[0].Location == 1);
	test_result(Priv->SMSLocationTarget == 2);

	error = ATGEN_GetNextSMS(s, &sms, FALSE);
	test_result(error == ERR_NONE);
	test_result(sms.SMS[0].Location == 2);
	test_result(strcmp((const char *)last_command(), "AT+CMGR=2\r") == 0);

	cleanup_sms_locations(s);
}

static void supported_range_bounds_overreported_count(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_PhoneModel model;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* AT+CPMS="SM" */
		"+CPMS: 2,3\r",
		"OK\r\n",
		/* AT+CMGD=? */
		"+CMGD: (1-3),(0-4)\r",
		"OK\r\n",
		/* AT+CMGR=1 */
		"OK\r\n",
		/* AT+CMGR=2 */
		"+CMS ERROR: 321\r\n",
		/* AT+CMGR=3 */
		"+CMGR: 1,\"\",23\r",
		sms_pdu,
		"OK\r\n",
		/* Status refresh for the second GetNextSMS call. */
		"+CPMS: 2,3\r",
		"OK\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	model = *s->Phone.Data.ModelInfo;
	s->Phone.Data.ModelInfo = &model;
	GSM_AddPhoneFeature(&model, F_DISABLE_CMGL);
	setup_sms_memory(Priv, MEM_SM);
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_NONE);
	test_result(sms.SMS[0].Location == 3);
	test_result(strcmp((const char *)last_command(), "AT+CMGR=3\r") == 0);

	error = ATGEN_GetNextSMS(s, &sms, FALSE);
	test_result(error == ERR_EMPTY);
	test_result(strcmp((const char *)last_command(), "AT+CPMS=\"SM\"\r") == 0);

	cleanup_sms_locations(s);
}

static void zero_based_locations_are_translated(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_PhoneModel model;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* AT+CPMS="ME" */
		"+CPMS: 1,10\r",
		"OK\r\n",
		/* AT+CMGD=? */
		"+CMGD: (0-9),(0-4)\r",
		"OK\r\n",
		/* AT+CMGR=0 */
		"+CMGR: 1,\"\",23\r",
		sms_pdu,
		"OK\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	model = *s->Phone.Data.ModelInfo;
	s->Phone.Data.ModelInfo = &model;
	GSM_AddPhoneFeature(&model, F_DISABLE_CMGL);
	GSM_AddPhoneFeature(&model, F_SMS_LOCATION_0);
	setup_sms_memory(Priv, MEM_ME);
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_NONE);
	test_result(sms.SMS[0].Location == 1);
	test_result(strcmp((const char *)last_command(), "AT+CMGR=0\r") == 0);

	cleanup_sms_locations(s);
}

static void inconsistent_cmgl_uses_location_bounds(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* Initial status used by ATGEN_GetSMSList. */
		"+CPMS: 1,30\r",
		"OK\r\n",
		/* AT+CMGL=4 returns no messages despite CPMS reporting one. */
		"OK\r\n",
		/* Status refresh before querying location bounds. */
		"+CPMS: 1,30\r",
		"OK\r\n",
		/* AT+CMGD=? */
		"+CMGD: (7),(0-4)\r",
		"OK\r\n",
		/* AT+CMGR=7 */
		"+CMGR: 1,\"\",23\r",
		sms_pdu,
		"OK\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	setup_sms_memory(Priv, MEM_SM);
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_NONE);
	test_result(sms.SMS[0].Location == 7);
	test_result(strcmp((const char *)last_command(), "AT+CMGR=7\r") == 0);

	cleanup_sms_locations(s);
}

static void cmgl_over_cpms_count_keeps_cache(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* Initial status used by ATGEN_GetSMSList. */
		"+CPMS: 0,30\r",
		"OK\r\n",
		/* AT+CMGL=4 returns a message despite CPMS reporting none. */
		"+CMGL: 7,0,\"\",23\r",
		sms_pdu,
		"OK\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	setup_sms_memory(Priv, MEM_SM);
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_NONE);
	test_result(sms.SMS[0].Location == 7);
	test_result(Priv->SMSCount == 1);
	test_result(strcmp((const char *)last_command(), "AT+CMGL=4\r") == 0);

	cleanup_sms_locations(s);
}

static void locations_switch_from_sim_to_phone(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_PhoneModel model;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* Status for both memories. */
		"+CPMS: 0,10\r",
		"OK\r\n",
		"+CPMS: 1,5000\r",
		"OK\r\n",
		/* Enumerate the supported ME range. */
		"+CMGD: (4-5),(0-4)\r",
		"OK\r\n",
		/* AT+CMGR=4 */
		"+CMS ERROR: 321\r\n",
		/* AT+CMGR=5 */
		"+CMGR: 1,\"\",23\r",
		sms_pdu,
		"OK\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	model = *s->Phone.Data.ModelInfo;
	s->Phone.Data.ModelInfo = &model;
	GSM_AddPhoneFeature(&model, F_DISABLE_CMGL);
	Priv->SIMSMSMemory = AT_AVAILABLE;
	Priv->PhoneSMSMemory = AT_AVAILABLE;
	Priv->SRSMSMemory = AT_NOTAVAILABLE;
	Priv->SIMSaveSMS = AT_NOTAVAILABLE;
	Priv->PhoneSaveSMS = AT_NOTAVAILABLE;
	Priv->SMSMemory = MEM_ME;
	Priv->SMSMemoryWrite = FALSE;
	Priv->NumFolders = 2;
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_NONE);
	test_result(sms.SMS[0].Location == GSM_PHONE_MAXSMSINFOLDER + 5);
	test_result(strcmp((const char *)last_command(), "AT+CMGR=5\r") == 0);

	cleanup_sms_locations(s);
}

static void malformed_location_bounds_use_legacy_scan(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_PhoneModel model;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* AT+CPMS="SM" */
		"+CPMS: 1,30\r",
		"OK\r\n",
		/* Malformed AT+CMGD=? response. */
		"+CMGD: (4-1),(0-4)\r",
		"OK\r\n",
		/* Legacy AT+CMGR=1 */
		"+CMGR: 1,\"\",23\r",
		sms_pdu,
		"OK\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	model = *s->Phone.Data.ModelInfo;
	s->Phone.Data.ModelInfo = &model;
	GSM_AddPhoneFeature(&model, F_DISABLE_CMGL);
	setup_sms_memory(Priv, MEM_SM);
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_NONE);
	test_result(Priv->SMSLocationLegacy);
	test_result(sms.SMS[0].Location == 1);
	test_result(strcmp((const char *)last_command(), "AT+CMGR=1\r") == 0);

	cleanup_sms_locations(s);
}

static void empty_location_bounds_use_legacy_scan(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_PhoneModel model;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* AT+CPMS="SM" */
		"+CPMS: 1,30\r",
		"OK\r\n",
		/* Empty AT+CMGD=? capability range. */
		"+CMGD: (),(0-4)\r",
		"OK\r\n",
		/* Legacy AT+CMGR=1 */
		"+CMGR: 1,\"\",23\r",
		sms_pdu,
		"OK\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	model = *s->Phone.Data.ModelInfo;
	s->Phone.Data.ModelInfo = &model;
	GSM_AddPhoneFeature(&model, F_DISABLE_CMGL);
	setup_sms_memory(Priv, MEM_SM);
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_NONE);
	test_result(Priv->SMSLocationLegacy);
	test_result(sms.SMS[0].Location == 1);
	test_result(strcmp((const char *)last_command(), "AT+CMGR=1\r") == 0);

	cleanup_sms_locations(s);
}

static void unsupported_location_bounds_use_legacy_scan(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_PhoneModel model;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* AT+CPMS="SM" */
		"+CPMS: 1,30\r",
		"OK\r\n",
		/* Unsupported AT+CMGD=? command. */
		"ERROR\r\n",
		/* Legacy AT+CMGR=1 */
		"+CMGR: 1,\"\",23\r",
		sms_pdu,
		"OK\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	model = *s->Phone.Data.ModelInfo;
	s->Phone.Data.ModelInfo = &model;
	GSM_AddPhoneFeature(&model, F_DISABLE_CMGL);
	setup_sms_memory(Priv, MEM_SM);
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));
	/* Legacy scanning must not honor stale memory from a previous result. */
	sms.SMS[0].Memory = MEM_ME;

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_NONE);
	test_result(Priv->SMSLocationLegacy);
	test_result(sms.SMS[0].Location == 1);
	test_result(strcmp((const char *)last_command(), "AT+CMGR=1\r") == 0);

	cleanup_sms_locations(s);
}

static void cms_operation_not_allowed_uses_legacy_scan(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_PhoneModel model;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* AT+CPMS="SM" */
		"+CPMS: 1,30\r",
		"OK\r\n",
		/* AT+CMGD=? is not allowed. */
		"+CMS ERROR: 302\r\n",
		/* Legacy AT+CMGR=1 */
		"+CMGR: 1,\"\",23\r",
		sms_pdu,
		"OK\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	model = *s->Phone.Data.ModelInfo;
	s->Phone.Data.ModelInfo = &model;
	GSM_AddPhoneFeature(&model, F_DISABLE_CMGL);
	setup_sms_memory(Priv, MEM_SM);
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_NONE);
	test_result(Priv->SMSLocationLegacy);
	test_result(sms.SMS[0].Location == 1);
	test_result(strcmp((const char *)last_command(), "AT+CMGR=1\r") == 0);

	cleanup_sms_locations(s);
}

static void cme_operation_not_allowed_uses_legacy_scan(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_PhoneModel model;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* AT+CPMS="SM" */
		"+CPMS: 1,30\r",
		"OK\r\n",
		/* AT+CMGD=? is not allowed. */
		"+CME ERROR: 3\r\n",
		/* Legacy AT+CMGR=1 */
		"+CMGR: 1,\"\",23\r",
		sms_pdu,
		"OK\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	model = *s->Phone.Data.ModelInfo;
	s->Phone.Data.ModelInfo = &model;
	GSM_AddPhoneFeature(&model, F_DISABLE_CMGL);
	setup_sms_memory(Priv, MEM_SM);
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_NONE);
	test_result(Priv->SMSLocationLegacy);
	test_result(sms.SMS[0].Location == 1);
	test_result(strcmp((const char *)last_command(), "AT+CMGR=1\r") == 0);

	cleanup_sms_locations(s);
}

static void sim_security_error_does_not_use_legacy_scan(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_PhoneModel model;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* AT+CPMS="SM" */
		"+CPMS: 1,30\r",
		"OK\r\n",
		/* SIM PIN required. */
		"+CMS ERROR: 311\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	model = *s->Phone.Data.ModelInfo;
	s->Phone.Data.ModelInfo = &model;
	GSM_AddPhoneFeature(&model, F_DISABLE_CMGL);
	setup_sms_memory(Priv, MEM_SM);
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_SECURITYERROR);
	test_result(!Priv->SMSLocationLegacy);
	test_result(strcmp((const char *)last_command(), "AT+CMGD=?\r") == 0);

	cleanup_sms_locations(s);
}

static void smsme900_phone_bounds_are_normalized(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_PhoneModel model;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* Status for both memories. */
		"+CPMS: 0,30\r",
		"OK\r\n",
		"+CPMS: 1,100\r",
		"OK\r\n",
		/* ME reports its physical 900-based range. */
		"+CMGD: (900-999),(0-4)\r",
		"OK\r\n",
		/* The logical first ME location must be read as physical index 900. */
		"+CMGR: 1,\"\",23\r",
		sms_pdu,
		"OK\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	model = *s->Phone.Data.ModelInfo;
	s->Phone.Data.ModelInfo = &model;
	GSM_AddPhoneFeature(&model, F_DISABLE_CMGL);
	GSM_AddPhoneFeature(&model, F_SMSME900);
	Priv->SIMSMSMemory = AT_AVAILABLE;
	Priv->PhoneSMSMemory = AT_AVAILABLE;
	Priv->SRSMSMemory = AT_NOTAVAILABLE;
	Priv->SIMSaveSMS = AT_NOTAVAILABLE;
	Priv->PhoneSaveSMS = AT_NOTAVAILABLE;
	Priv->SMSMemory = MEM_ME;
	Priv->SMSMemoryWrite = FALSE;
	Priv->NumFolders = 2;
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_NONE);
	test_result(sms.SMS[0].Location == GSM_PHONE_MAXSMSINFOLDER + 1);
	test_result(strcmp((const char *)last_command(), "AT+CMGR=900\r") == 0);

	cleanup_sms_locations(s);
}

static void motorola_phone_bounds_use_mt(void)
{
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_PhoneModel model;
	GSM_StateMachine *s = setup_state_machine();
	GSM_Phone_ATGENData *Priv = setup_at_engine(s);
	const char *responses[] = {
		/* Status for both memories. */
		"+CPMS: 1,10\r",
		"OK\r\n",
		"+CPMS: 1,10\r",
		"OK\r\n",
		/* Select SM, then exhaust its supported range. */
		"OK\r\n",
		"+CMGD: (1),(0-4)\r",
		"OK\r\n",
		"+CMS ERROR: 321\r\n",
		/* Reject phone selection so its MT alias remains the last command. */
		"+CMS ERROR: 311\r\n",

		"ERROR\r\n"
	};

	puts(__func__);
	model = *s->Phone.Data.ModelInfo;
	s->Phone.Data.ModelInfo = &model;
	GSM_AddPhoneFeature(&model, F_DISABLE_CMGL);
	Priv->SIMSMSMemory = AT_AVAILABLE;
	Priv->PhoneSMSMemory = AT_AVAILABLE;
	Priv->SRSMSMemory = AT_NOTAVAILABLE;
	Priv->SIMSaveSMS = AT_NOTAVAILABLE;
	Priv->PhoneSaveSMS = AT_NOTAVAILABLE;
	Priv->SMSMemory = MEM_ME;
	Priv->SMSMemoryWrite = FALSE;
	Priv->MotorolaSMS = TRUE;
	Priv->NumFolders = 2;
	SET_RESPONSES(responses);
	bind_response_handling(s);
	memset(&sms, 0, sizeof(sms));

	error = ATGEN_GetNextSMS(s, &sms, TRUE);
	test_result(error == ERR_SECURITYERROR);
	test_result(!Priv->SMSLocationLegacy);
	test_result(strcmp((const char *)last_command(), "AT+CPMS=\"MT\"\r") == 0);

	cleanup_sms_locations(s);
}

int main(void)
{
	supported_range_stops_at_reported_count();
	shrinking_live_count_does_not_stop_scan();
	supported_range_bounds_overreported_count();
	zero_based_locations_are_translated();
	inconsistent_cmgl_uses_location_bounds();
	cmgl_over_cpms_count_keeps_cache();
	locations_switch_from_sim_to_phone();
	malformed_location_bounds_use_legacy_scan();
	empty_location_bounds_use_legacy_scan();
	unsupported_location_bounds_use_legacy_scan();
	cms_operation_not_allowed_uses_legacy_scan();
	cme_operation_not_allowed_uses_legacy_scan();
	sim_security_error_does_not_use_legacy_scan();
	smsme900_phone_bounds_are_normalized();
	motorola_phone_bounds_use_mt();
}
