/**
 * Tests that SQL service rows are read in ascending column order.
 *
 * Some ODBC drivers require SQLGetData calls to follow column order.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <gammu.h>
#include <gammu-smsd.h>

#include "../smsd/core.h"
#include "../smsd/services/sql.h"
#include "common.h"

GSM_Error SMSD_SendSMS(GSM_SMSDConfig *config);

typedef enum {
	RESULT_NONE,
	RESULT_FIND_ID,
	RESULT_REFRESH,
	RESULT_OUTBOX_BODY,
	RESULT_OUTBOX_MULTIPART,
	RESULT_SENT_ITEM,
	RESULT_DELIVERY_SELECT,
	RESULT_DELIVERY_UPDATE
} ResultKind;

typedef struct {
	SQL_result *result;
	ResultKind kind;
	int row;
	int last_field;
	unsigned long long fields;
} ResultState;

static ResultState states[4];
static unsigned long long find_id_fields;
static unsigned long long outbox_body_fields;
static unsigned long long outbox_multipart_fields;
static unsigned long long sent_item_fields;
static unsigned long long delivery_fields;
static int multipart_queries;
static time_t delivery_time;
static time_t outbox_insert_time;
static gboolean delivery_update_seen;
static gboolean delivery_select_present;
static gboolean outbox_is_multipart;
static gboolean sent_item_present;
static gboolean sent_item_mismatch;
static GSM_Error sent_item_query_error;
static int outbox_relative_validity;
static int sent_item_relative_validity;
static int outbox_retries;
static const char *outbox_status;
static const char *sent_item_status;
static char query_find_id[] = "find-id";
static char query_refresh[] = "refresh";
static char query_outbox_body[] = "outbox-body";
static char query_outbox_multipart[] = "outbox-multipart";
static char query_sent_item[] = "sent-item";
static char query_delivery_select[] = "delivery-select";
static char query_delivery_update[] = "delivery-update";
static char query_delivery_update_other[] = "delivery-update-other";

static ResultState *find_state(SQL_result *result)
{
	int i;

	for (i = 0; i < 4; i++) {
		if (states[i].result == result) {
			return &states[i];
		}
	}
	return NULL;
}

static ResultState *set_state(SQL_result *result, ResultKind kind)
{
	ResultState *state;
	int i;

	state = find_state(result);
	if (state == NULL) {
		for (i = 0; i < 4; i++) {
			if (states[i].result == NULL) {
				state = &states[i];
				state->result = result;
				break;
			}
		}
	}
	test_result(state != NULL);

	state->kind = kind;
	state->row = 0;
	state->last_field = -1;
	state->fields = 0;
	return state;
}

static ResultState *record_field(SQL_result *result, unsigned int field)
{
	ResultState *state = find_state(result);

	test_result(state != NULL);
	test_result((int)field >= state->last_field);
	state->last_field = (int)field;
	state->fields |= 1ULL << field;
	return state;
}

static GSM_Error mock_connect(GSM_SMSDConfig *config UNUSED)
{
	return ERR_NONE;
}

static GSM_Error mock_query(GSM_SMSDConfig *config UNUSED, const char *query, SQL_result *result)
{
	ResultKind kind;

	if (strcmp(query, "find-id") == 0) {
		kind = RESULT_FIND_ID;
	} else if (strcmp(query, "refresh") == 0) {
		kind = RESULT_REFRESH;
	} else if (strcmp(query, "outbox-body") == 0) {
		kind = RESULT_OUTBOX_BODY;
	} else if (strcmp(query, "outbox-multipart") == 0) {
		kind = RESULT_OUTBOX_MULTIPART;
		multipart_queries++;
	} else if (strcmp(query, "sent-item") == 0) {
		if (sent_item_query_error != ERR_NONE) {
			return sent_item_query_error;
		}
		kind = RESULT_SENT_ITEM;
	} else if (strcmp(query, "delivery-select") == 0) {
		kind = RESULT_DELIVERY_SELECT;
	} else if (strcmp(query, "delivery-update") == 0 || strcmp(query, "delivery-update-other") == 0) {
		kind = RESULT_DELIVERY_UPDATE;
		delivery_update_seen = TRUE;
	} else {
		fprintf(stderr, "Unexpected query: %s\n", query);
		return ERR_BUG;
	}

	set_state(result, kind);
	return ERR_NONE;
}

static void mock_free(GSM_SMSDConfig *config UNUSED)
{
}

static void mock_free_result(GSM_SMSDConfig *config UNUSED, SQL_result *result)
{
	ResultState *state = find_state(result);

	test_result(state != NULL);
	switch (state->kind) {
		case RESULT_FIND_ID:
			find_id_fields = state->fields;
			break;
		case RESULT_OUTBOX_BODY:
			outbox_body_fields = state->fields;
			break;
		case RESULT_OUTBOX_MULTIPART:
			outbox_multipart_fields |= state->fields;
			break;
		case RESULT_SENT_ITEM:
			sent_item_fields = state->fields;
			break;
		case RESULT_DELIVERY_SELECT:
			delivery_fields = state->fields;
			break;
		default:
			break;
	}
	state->kind = RESULT_NONE;
}

static int mock_next_row(GSM_SMSDConfig *config UNUSED, SQL_result *result)
{
	ResultState *state = find_state(result);

	test_result(state != NULL);
	switch (state->kind) {
		case RESULT_FIND_ID:
		case RESULT_OUTBOX_BODY:
			return state->row++ == 0;
		case RESULT_DELIVERY_SELECT:
			return delivery_select_present && state->row++ == 0;
		case RESULT_OUTBOX_MULTIPART:
			return multipart_queries == 1 && state->row++ == 0;
		case RESULT_SENT_ITEM:
			return sent_item_present && state->row++ == 0;
		default:
			return 0;
	}
}

static unsigned long long mock_seq_id(GSM_SMSDConfig *config UNUSED, const char *id UNUSED)
{
	return 0;
}

static unsigned long mock_affected_rows(GSM_SMSDConfig *config UNUSED, SQL_result *result)
{
	ResultState *state = find_state(result);

	test_result(state != NULL);
	return state->kind == RESULT_REFRESH ? 1 : 0;
}

static const char *mock_get_string(GSM_SMSDConfig *config UNUSED, SQL_result *result, unsigned int field)
{
	ResultState *state = record_field(result, field);

	if (state->kind == RESULT_OUTBOX_BODY) {
		switch (field) {
			case 0:
			case 2:
				return NULL;
			case 1:
				return "Default_No_Compression";
			case 4:
				return "test message";
			case 6:
				return "+420123456";
			case 10:
				return "sql-read-order";
			case 12:
				return outbox_status;
			default:
				break;
		}
	} else if (state->kind == RESULT_OUTBOX_MULTIPART) {
		switch (field) {
			case 0:
			case 2:
				return NULL;
			case 1:
				return "Default_No_Compression";
			case 4:
				return "multipart message";
			case 7:
				return "SendingOK";
			default:
				break;
		}
	} else if (state->kind == RESULT_SENT_ITEM) {
		switch (field) {
			case 0:
				return "00740065007300740020006D006500730073006100670065";
			case 1:
				return "Default_No_Compression";
			case 2:
				return "";
			case 4:
				return "test message";
			case 5:
				return sent_item_mismatch ? "+420999999" : "+420123456";
			case 8:
				return "sql-read-order";
			case 9:
				return sent_item_status;
			default:
				break;
		}
	} else if (state->kind == RESULT_DELIVERY_SELECT) {
		if (field == 1) {
			return "SendingOK";
		}
		if (field == 4) {
			return "+420987654";
		}
	}

	fprintf(stderr, "Unexpected string field %u for result kind %d\n", field, state->kind);
	exit(2);
}

static long long mock_get_number(GSM_SMSDConfig *config UNUSED, SQL_result *result, unsigned int field)
{
	ResultState *state = record_field(result, field);

	if (state->kind == RESULT_FIND_ID && field == 0) {
		return 42;
	}
	if (state->kind == RESULT_OUTBOX_BODY) {
		switch (field) {
			case 3:
				return -1;
			case 5:
				return 42;
			case 8:
				return outbox_relative_validity;
			case 11:
				return outbox_retries;
			default:
				break;
		}
	}
	if (state->kind == RESULT_OUTBOX_MULTIPART) {
		switch (field) {
			case 3:
				return -1;
			case 5:
				return 42;
			default:
				break;
		}
	}
	if (state->kind == RESULT_SENT_ITEM) {
		switch (field) {
			case 3:
				return -1;
			case 7:
				return sent_item_relative_validity;
			default:
				break;
		}
	}
	if (state->kind == RESULT_DELIVERY_SELECT && field == 0) {
		return 123;
	}

	fprintf(stderr, "Unexpected numeric field %u for result kind %d\n", field, state->kind);
	exit(2);
}

static time_t mock_get_date(GSM_SMSDConfig *config UNUSED, SQL_result *result, unsigned int field)
{
	ResultState *state = record_field(result, field);

	if (state->kind == RESULT_FIND_ID && field == 1) {
		return outbox_insert_time;
	}
	if (state->kind == RESULT_SENT_ITEM && field == 6) {
		return outbox_insert_time;
	}
	if (state->kind == RESULT_DELIVERY_SELECT && field == 2) {
		return delivery_time;
	}

	fprintf(stderr, "Unexpected date field %u for result kind %d\n", field, state->kind);
	exit(2);
}

static gboolean mock_get_bool(GSM_SMSDConfig *config UNUSED, SQL_result *result, unsigned int field)
{
	ResultState *state = record_field(result, field);

	if (state->kind == RESULT_OUTBOX_BODY && field == 7) {
		return outbox_is_multipart;
	}
	if (state->kind == RESULT_OUTBOX_BODY && field == 9) {
		return TRUE;
	}

	fprintf(stderr, "Unexpected boolean field %u for result kind %d\n", field, state->kind);
	exit(2);
}

static char *mock_quote_string(GSM_SMSDConfig *config UNUSED, const char *value)
{
	char *result = malloc(strlen(value) + 1);

	test_result(result != NULL);
	strcpy(result, value);
	return result;
}

static struct GSM_SMSDdbobj mock_db = {
	mock_connect,
	mock_query,
	mock_free,
	mock_free_result,
	mock_next_row,
	mock_seq_id,
	mock_affected_rows,
	mock_get_string,
	mock_get_number,
	mock_get_date,
	mock_get_bool,
	mock_quote_string
};

static void reset_mock(void)
{
	memset(states, 0, sizeof(states));
	find_id_fields = 0;
	outbox_body_fields = 0;
	outbox_multipart_fields = 0;
	sent_item_fields = 0;
	delivery_fields = 0;
	multipart_queries = 0;
	delivery_update_seen = FALSE;
	delivery_select_present = TRUE;
	outbox_insert_time = 1700000000;
	outbox_is_multipart = TRUE;
	sent_item_present = FALSE;
	sent_item_mismatch = FALSE;
	sent_item_query_error = ERR_NONE;
	outbox_relative_validity = SMS_VALID_Max_Time;
	sent_item_relative_validity = SMS_VALID_Max_Time;
	outbox_retries = 0;
	outbox_status = "SendingOK";
	sent_item_status = "SendingOKNoReport";
}

static void setup_config(GSM_SMSDConfig *config)
{
	memset(config, 0, sizeof(*config));
	config->driver = "odbc";
	config->sql = "mysql";
	config->db = &mock_db;
	config->backend_retries = 1;
	config->skipsmscnumber = "";
	config->deliveryreportdelay = 3600;

	config->SMSDSQL_queries[SQL_QUERY_FIND_OUTBOX_SMS_ID] = query_find_id;
	config->SMSDSQL_queries[SQL_QUERY_REFRESH_SEND_STATUS] = query_refresh;
	config->SMSDSQL_queries[SQL_QUERY_FIND_OUTBOX_BODY] = query_outbox_body;
	config->SMSDSQL_queries[SQL_QUERY_FIND_OUTBOX_MULTIPART] = query_outbox_multipart;
	config->SMSDSQL_queries[SQL_QUERY_FIND_SENT_ITEM] = query_sent_item;
	config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_SELECT] = query_delivery_select;
	config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_UPDATE_DELIVERED] = query_delivery_update;
	config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_UPDATE] = query_delivery_update_other;
}

static void test_find_outbox_order(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;
	char id[32];

	reset_mock();
	setup_config(&config);
	memset(&sms, 0, sizeof(sms));

	error = SMSDSQL.FindOutboxSMS(&sms, &config, id);

	test_result(error == ERR_NONE);
	test_result(strcmp(id, "42") == 0);
	test_result(sms.Number == 2);
	test_result(config.SkipMessage[0] == TRUE);
	test_result(config.SkipMessage[1] == TRUE);
	test_result(find_id_fields == ((1ULL << 0) | (1ULL << 1)));
	test_result(outbox_body_fields == ((1ULL << 13) - 1));
	test_result(outbox_multipart_fields == (((1ULL << 6) - 1) | (1ULL << 7)));
}

static void test_find_outbox_without_sent_item(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;
	char id[32];

	reset_mock();
	setup_config(&config);
	memset(&sms, 0, sizeof(sms));
	outbox_is_multipart = FALSE;
	outbox_status = "Reserved";

	error = SMSDSQL.FindOutboxSMS(&sms, &config, id);

	test_result(error == ERR_NONE);
	test_result(sms.Number == 1);
	test_result(config.SkipMessage[0] == FALSE);
	test_result(sent_item_fields == 0);
}

static void test_matching_sent_item_is_skipped(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;
	char id[32];

	reset_mock();
	setup_config(&config);
	memset(&sms, 0, sizeof(sms));
	outbox_is_multipart = FALSE;
	outbox_status = "Reserved";
	sent_item_present = TRUE;

	error = SMSDSQL.FindOutboxSMS(&sms, &config, id);

	test_result(error == ERR_NONE);
	test_result(sms.Number == 1);
	test_result(config.SkipMessage[0] == TRUE);
	test_result(sent_item_fields == ((1ULL << 10) - 1));
}

static void test_inherited_sent_item_validity_is_skipped(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;
	char id[32];

	reset_mock();
	setup_config(&config);
	memset(&sms, 0, sizeof(sms));
	outbox_is_multipart = FALSE;
	outbox_status = "Reserved";
	outbox_relative_validity = -1;
	sent_item_present = TRUE;

	error = SMSDSQL.FindOutboxSMS(&sms, &config, id);

	test_result(error == ERR_NONE);
	test_result(sms.Number == 1);
	test_result(config.SkipMessage[0] == TRUE);
	test_result(sent_item_fields == ((1ULL << 10) - 1));
}

static void test_matching_sending_error_is_skipped(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;
	char id[32];

	reset_mock();
	setup_config(&config);
	memset(&sms, 0, sizeof(sms));
	outbox_is_multipart = FALSE;
	outbox_status = "Reserved";
	outbox_retries = 2;
	sent_item_present = TRUE;
	sent_item_status = "SendingError";

	error = SMSDSQL.FindOutboxSMS(&sms, &config, id);

	test_result(error == ERR_NONE);
	test_result(sms.Number == 1);
	test_result(config.retries == 2);
	test_result(config.SkipMessage[0] == TRUE);
	test_result(sent_item_fields == ((1ULL << 10) - 1));
}

static void test_reused_sent_item_id_is_rejected(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;
	char id[32];

	reset_mock();
	setup_config(&config);
	memset(&sms, 0, sizeof(sms));
	outbox_is_multipart = FALSE;
	outbox_status = "Reserved";
	sent_item_present = TRUE;
	sent_item_mismatch = TRUE;

	error = SMSDSQL.FindOutboxSMS(&sms, &config, id);

	test_result(error == ERR_FILEALREADYEXIST);
	test_result(sms.Number == 0);
	test_result(config.SkipMessage[0] == FALSE);
	test_result(sent_item_fields == ((1ULL << 10) - 1));
}

static void test_reconciliation_query_error_is_retryable(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;
	char id[32];

	reset_mock();
	setup_config(&config);
	memset(&sms, 0, sizeof(sms));
	outbox_is_multipart = FALSE;
	outbox_status = "Reserved";
	sent_item_query_error = ERR_SQL;

	error = SMSDSQL.FindOutboxSMS(&sms, &config, id);

	test_result(error == ERR_BUSY);
	test_result(sms.Number == 0);
	test_result(config.SkipMessage[0] == FALSE);
	test_result(sent_item_fields == 0);
}

static void test_delivery_report_order(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_StringArray sent_ids;
	GSM_Error error;

	reset_mock();
	setup_config(&config);
	memset(&sms, 0, sizeof(sms));
	GSM_StringArray_New(&sent_ids);
	sms.Number = 1;
	GSM_SetDefaultSMSData(&sms.SMS[0]);
	sms.SMS[0].PDU = SMS_Status_Report;
	EncodeUnicode(sms.SMS[0].Number, "+420123456", strlen("+420123456"));
	EncodeUnicode(sms.SMS[0].SMSC.Number, "+420987654", strlen("+420987654"));
	EncodeUnicode(sms.SMS[0].Text, "Delivered", strlen("Delivered"));
	delivery_time = Fill_Time_T(sms.SMS[0].DateTime);

	error = SMSDSQL.SaveInboxSMS(&sms, &config, NULL, &sent_ids);

	test_result(error == ERR_NONE);
	test_result(delivery_update_seen == TRUE);
	test_result(delivery_fields == ((1ULL << 0) | (1ULL << 1) | (1ULL << 2) | (1ULL << 4)));
	test_result(sent_ids.used == 1);
	test_result(strcmp(sent_ids.data[0], "123") == 0);
	GSM_StringArray_Free(&sent_ids);
}

static void test_unmatched_delivery_report_id(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_StringArray sent_ids;
	GSM_Error error;

	reset_mock();
	setup_config(&config);
	memset(&sms, 0, sizeof(sms));
	GSM_StringArray_New(&sent_ids);
	delivery_select_present = FALSE;
	sms.Number = 1;
	GSM_SetDefaultSMSData(&sms.SMS[0]);
	sms.SMS[0].PDU = SMS_Status_Report;
	EncodeUnicode(sms.SMS[0].Number, "+420123456", strlen("+420123456"));
	EncodeUnicode(sms.SMS[0].SMSC.Number, "+420987654", strlen("+420987654"));
	EncodeUnicode(sms.SMS[0].Text, "Delivered", strlen("Delivered"));

	error = SMSDSQL.SaveInboxSMS(&sms, &config, NULL, &sent_ids);

	test_result(error == ERR_NONE);
	test_result(delivery_update_seen == FALSE);
	test_result(sent_ids.used == 1);
	test_result(strcmp(sent_ids.data[0], "") == 0);
	GSM_StringArray_Free(&sent_ids);
}

static int collision_add_calls;
static int collision_move_calls;
static int collision_update_calls;
static int collision_update_status_code;
static int collision_update_part;
static int collision_find_number;
static unsigned int collision_find_retries;
static gboolean collision_find_skip;
static gboolean collision_move_sent;
static GSM_Error collision_find_error;

static GSM_Error collision_find_outbox(
	GSM_MultiSMSMessage *sms,
	GSM_SMSDConfig *config,
	char *id)
{
	strcpy(id, "42");
	sms->Number = collision_find_number;
	config->retries = collision_find_retries;
	if (collision_find_number > 0) {
		config->SkipMessage[0] = collision_find_skip;
	}
	return collision_find_error;
}

static GSM_Error collision_move(
	GSM_MultiSMSMessage *sms UNUSED,
	GSM_SMSDConfig *config UNUSED,
	char *id UNUSED,
	gboolean always_delete UNUSED,
	gboolean sent)
{
	collision_move_calls++;
	collision_move_sent = sent;
	return ERR_NONE;
}

static GSM_Error collision_add_sent(
	GSM_MultiSMSMessage *sms UNUSED,
	GSM_SMSDConfig *config UNUSED,
	char *id UNUSED,
	int part UNUSED,
	GSM_SMSDSendingError error UNUSED,
	int tpmr UNUSED)
{
	collision_add_calls++;
	return ERR_NONE;
}

static GSM_Error collision_update_retries(
	GSM_SMSDConfig *config,
	char *id UNUSED)
{
	collision_update_calls++;
	collision_update_status_code = config->StatusCode;
	collision_update_part = config->Part;
	return ERR_NONE;
}

static GSM_SMSDService collision_service = {
	NULL,
	NULL,
	NULL,
	NULL,
	collision_find_outbox,
	collision_move,
	NULL,
	collision_add_sent,
	NULL,
	collision_update_retries,
	NULL,
	NULL
};

static void check_send_error_is_left_queued(GSM_Error find_error)
{
	GSM_SMSDConfig config;
	GSM_Error error;

	memset(&config, 0, sizeof(config));
	config.Service = &collision_service;
	config.StatusCode = 535;
	config.Part = 2;
	collision_add_calls = 0;
	collision_move_calls = 0;
	collision_update_calls = 0;
	collision_update_status_code = 0;
	collision_update_part = 0;
	collision_find_number = 0;
	collision_find_retries = 0;
	collision_find_skip = FALSE;
	collision_move_sent = TRUE;
	collision_find_error = find_error;

	error = SMSD_SendSMS(&config);

	test_result(error == find_error);
	test_result(collision_add_calls == 0);
	test_result(collision_move_calls == 0);
	test_result(collision_update_calls == 1);
	test_result(collision_update_status_code == -1);
	test_result(collision_update_part == -1);
}

static void test_send_finalized_failure_is_removed(void)
{
	GSM_SMSDConfig config;
	GSM_Error error;

	memset(&config, 0, sizeof(config));
	config.Service = &collision_service;
	config.maxretries = 1;
	collision_add_calls = 0;
	collision_move_calls = 0;
	collision_update_calls = 0;
	collision_find_number = 1;
	collision_find_retries = 2;
	collision_find_skip = TRUE;
	collision_move_sent = TRUE;
	collision_find_error = ERR_NONE;

	error = SMSD_SendSMS(&config);

	test_result(error == ERR_UNKNOWN);
	test_result(collision_add_calls == 0);
	test_result(collision_move_calls == 1);
	test_result(collision_move_sent == FALSE);
	test_result(collision_update_calls == 0);
}

static void test_send_collision_is_left_queued(void)
{
	check_send_error_is_left_queued(ERR_FILEALREADYEXIST);
}

static void test_send_reconciliation_error_is_left_queued(void)
{
	check_send_error_is_left_queued(ERR_BUSY);
}

int main(void)
{
	test_find_outbox_order();
	test_find_outbox_without_sent_item();
	test_matching_sent_item_is_skipped();
	test_inherited_sent_item_validity_is_skipped();
	test_matching_sending_error_is_skipped();
	test_reused_sent_item_id_is_rejected();
	test_reconciliation_query_error_is_retryable();
	test_delivery_report_order();
	test_unmatched_delivery_report_id();
	test_send_finalized_failure_is_removed();
	test_send_collision_is_left_queued();
	test_send_reconciliation_error_is_left_queued();
	return 0;
}
