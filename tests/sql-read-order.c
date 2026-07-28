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

typedef enum {
	RESULT_NONE,
	RESULT_FIND_ID,
	RESULT_REFRESH,
	RESULT_OUTBOX_BODY,
	RESULT_OUTBOX_MULTIPART,
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
static unsigned long long delivery_fields;
static int multipart_queries;
static time_t delivery_time;
static gboolean delivery_update_seen;
static char query_find_id[] = "find-id";
static char query_refresh[] = "refresh";
static char query_outbox_body[] = "outbox-body";
static char query_outbox_multipart[] = "outbox-multipart";
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
		case RESULT_DELIVERY_SELECT:
			return state->row++ == 0;
		case RESULT_OUTBOX_MULTIPART:
			return multipart_queries == 1 && state->row++ == 0;
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
				return "SendingOK";
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
				return SMS_VALID_Max_Time;
			case 11:
				return 0;
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
		return time(NULL);
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
		return TRUE;
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
	delivery_fields = 0;
	multipart_queries = 0;
	delivery_update_seen = FALSE;
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

static void test_delivery_report_order(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;

	reset_mock();
	setup_config(&config);
	memset(&sms, 0, sizeof(sms));
	sms.Number = 1;
	GSM_SetDefaultSMSData(&sms.SMS[0]);
	sms.SMS[0].PDU = SMS_Status_Report;
	EncodeUnicode(sms.SMS[0].Number, "+420123456", strlen("+420123456"));
	EncodeUnicode(sms.SMS[0].SMSC.Number, "+420987654", strlen("+420987654"));
	EncodeUnicode(sms.SMS[0].Text, "Delivered", strlen("Delivered"));
	delivery_time = Fill_Time_T(sms.SMS[0].DateTime);

	error = SMSDSQL.SaveInboxSMS(&sms, &config, NULL);

	test_result(error == ERR_NONE);
	test_result(delivery_update_seen == TRUE);
	test_result(delivery_fields == ((1ULL << 0) | (1ULL << 1) | (1ULL << 2) | (1ULL << 4)));
}

int main(void)
{
	test_find_outbox_order();
	test_delivery_report_order();
	return 0;
}
