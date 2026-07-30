/**
 * Tests that SQL service rows are read in ascending column order.
 *
 * Some ODBC drivers require SQLGetData calls to follow column order.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include <gammu.h>
#include <gammu-smsd.h>

#include "../smsd/core.h"
#include "../smsd/services/sql.h"
#include "common.h"

GSM_Error SMSD_SendSMS(GSM_SMSDConfig *config);
gboolean SMSD_CheckMultipart(
	GSM_SMSDConfig *config,
	GSM_MultiSMSMessage *multi_sms);

typedef enum {
	RESULT_NONE,
	RESULT_FIND_ID,
	RESULT_REFRESH,
	RESULT_OUTBOX_BODY,
	RESULT_OUTBOX_MULTIPART,
	RESULT_SENT_ITEM,
	RESULT_DELIVERY_SELECT,
	RESULT_DELIVERY_UPDATE,
	RESULT_INBOX_INSERT,
	RESULT_INBOX_METADATA,
	RESULT_RESTORE_INBOX_GROUPS,
	RESULT_UPDATE_RECEIVED
} ResultKind;

typedef struct {
	long long message_id;
	int sequence_position;
	int part_count;
	char processed[6];
} InboxInsert;

typedef struct {
	long long message_id;
	int sequence_position;
	int part_count;
	long long row_id;
	char processed[6];
} InboxMetadata;

typedef struct {
	long long message_id;
	const char *sender;
	const char *smsc;
	const char *udh;
	int sequence_position;
	int part_count;
	long long age;
} RestoreInboxGroup;

typedef struct {
	SQL_result *result;
	ResultKind kind;
	int row;
	int last_field;
	unsigned long long fields;
} ResultState;

static ResultState states[4];
static unsigned long long find_id_fields;
static int find_day_mask;
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
static unsigned long long next_inbox_id;
static int inbox_insert_count;
static int inbox_metadata_count;
static int update_received_count;
static GSM_Error update_received_error;
static int restore_inbox_group_count;
static InboxInsert inbox_insert[128];
static InboxMetadata inbox_metadata[128];
static RestoreInboxGroup restore_inbox_groups[8];
static char query_find_id[] = "find-id-%1-%2";
static char query_refresh[] = "refresh";
static char query_outbox_body[] = "outbox-body";
static char query_outbox_multipart[] = "outbox-multipart";
static char query_sent_item[] = "sent-item";
static char query_delivery_select[] = "delivery-select";
static char query_delivery_update[] = "delivery-update";
static char query_delivery_update_other[] = "delivery-update-other";
static char query_inbox_insert[] = "inbox-insert-%1-%2-%3-%4";
static char query_inbox_metadata[] = "inbox-metadata-%1-%2-%3-%4-%5";
static char query_restore_inbox_groups[] = "restore-inbox-groups";
static char query_update_received[] = "update-received";

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
	int find_limit;

	if (sscanf(query, "find-id-%d-%d", &find_limit, &find_day_mask) == 2) {
		test_result(find_limit == 1);
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
	} else if (sscanf(query, "inbox-insert-%lld-%d-%d-%5s",
			  &inbox_insert[inbox_insert_count].message_id,
			  &inbox_insert[inbox_insert_count].sequence_position,
			  &inbox_insert[inbox_insert_count].part_count,
			  inbox_insert[inbox_insert_count].processed) == 4) {
		kind = RESULT_INBOX_INSERT;
		inbox_insert_count++;
	} else if (sscanf(query, "inbox-metadata-%lld-%d-%d-%lld-%5s",
			  &inbox_metadata[inbox_metadata_count].message_id,
			  &inbox_metadata[inbox_metadata_count].sequence_position,
			  &inbox_metadata[inbox_metadata_count].part_count,
			  &inbox_metadata[inbox_metadata_count].row_id,
			  inbox_metadata[inbox_metadata_count].processed) == 5) {
		kind = RESULT_INBOX_METADATA;
		inbox_metadata_count++;
	} else if (strcmp(query, "restore-inbox-groups") == 0) {
		kind = RESULT_RESTORE_INBOX_GROUPS;
	} else if (strcmp(query, "update-received") == 0) {
		if (update_received_error != ERR_NONE) {
			return update_received_error;
		}
		kind = RESULT_UPDATE_RECEIVED;
		update_received_count++;
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
	state->result = NULL;
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
		case RESULT_RESTORE_INBOX_GROUPS:
			if (state->row < restore_inbox_group_count) {
				state->row++;
				state->last_field = -1;
				return 1;
			}
			return 0;
		default:
			return 0;
	}
}

static unsigned long long mock_seq_id(GSM_SMSDConfig *config UNUSED, const char *id UNUSED)
{
	return next_inbox_id++;
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
	} else if (state->kind == RESULT_RESTORE_INBOX_GROUPS) {
		RestoreInboxGroup *group = &restore_inbox_groups[state->row - 1];

		switch (field) {
			case 1:
				return group->sender;
			case 2:
				return group->smsc;
			case 3:
				return group->udh;
			default:
				break;
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
	if (state->kind == RESULT_RESTORE_INBOX_GROUPS) {
		RestoreInboxGroup *group = &restore_inbox_groups[state->row - 1];

		switch (field) {
			case 0:
				return group->message_id;
			case 4:
				return group->sequence_position;
			case 5:
				return group->part_count;
			case 6:
				return group->age;
			default:
				break;
		}
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
	find_day_mask = 0;
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
	next_inbox_id = 101;
	inbox_insert_count = 0;
	inbox_metadata_count = 0;
	update_received_count = 0;
	update_received_error = ERR_NONE;
	restore_inbox_group_count = 0;
	memset(inbox_insert, 0, sizeof(inbox_insert));
	memset(inbox_metadata, 0, sizeof(inbox_metadata));
	memset(restore_inbox_groups, 0, sizeof(restore_inbox_groups));
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
	config->multiparttimeout = 600;

	config->SMSDSQL_queries[SQL_QUERY_FIND_OUTBOX_SMS_ID] = query_find_id;
	config->SMSDSQL_queries[SQL_QUERY_REFRESH_SEND_STATUS] = query_refresh;
	config->SMSDSQL_queries[SQL_QUERY_FIND_OUTBOX_BODY] = query_outbox_body;
	config->SMSDSQL_queries[SQL_QUERY_FIND_OUTBOX_MULTIPART] = query_outbox_multipart;
	config->SMSDSQL_queries[SQL_QUERY_FIND_SENT_ITEM] = query_sent_item;
	config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_SELECT] = query_delivery_select;
	config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_UPDATE_DELIVERED] = query_delivery_update;
	config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_UPDATE] = query_delivery_update_other;
	config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_INSERT] = query_inbox_insert;
	config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_UPDATE_METADATA] = query_inbox_metadata;
	config->SMSDSQL_queries[SQL_QUERY_RESTORE_INBOX_GROUPS] = query_restore_inbox_groups;
	config->SMSDSQL_queries[SQL_QUERY_UPDATE_RECEIVED] = query_update_received;
}

static void test_phone_status_timeout(void)
{
	GSM_SMSDConfig config;

	setup_config(&config);

	test_result(SMSDSQL_PhoneStatusTimeout(0) == 10);
	test_result(SMSDSQL_PhoneStatusTimeout(60) == 70);
	test_result(SMSDSQL_PhoneStatusTimeout(UINT_MAX) == INT_MAX);

	config.sql = "mysql";
	test_result(strcmp(SMSDSQL_NowPlus(&config, 70),
			"(NOW() + INTERVAL 70 SECOND) + 0") == 0);
	config.sql = "pgsql";
	test_result(strcmp(SMSDSQL_NowPlus(&config, 70),
			"now() + interval '70 seconds'") == 0);
	config.sql = "sqlite3";
	test_result(strcmp(SMSDSQL_NowPlus(&config, 70),
			"datetime('now', '+70 seconds', 'localtime')") == 0);
	config.sql = "access";
	test_result(strcmp(SMSDSQL_NowPlus(&config, 70),
			"DateAdd('s', 70, Now())") == 0);
	config.sql = "mssql";
	test_result(strcmp(SMSDSQL_NowPlus(&config, 70),
			"DATEADD(second, 70, CURRENT_TIMESTAMP)") == 0);
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
	test_result(SMSDSQL_DayMask(1) == 1);
	test_result(SMSDSQL_DayMask(0) == 64);
	test_result(strcmp(SMSDSQL_DayMaskPredicate(&config),
			"(`SendDays` & %2) <> 0") == 0);
	test_result(strcmp(SMSDSQL_TimeDiff(&config, "InsertIntoDB"),
			"TIMESTAMPDIFF(SECOND, `InsertIntoDB`, NOW())") == 0);
	config.sql = "pgsql";
	test_result(strcmp(SMSDSQL_TimeDiff(&config, "InsertIntoDB"),
			"CAST(EXTRACT(EPOCH FROM (now() - \"InsertIntoDB\")) "
			"AS BIGINT)") == 0);
	config.sql = "sqlite3";
	test_result(strcmp(SMSDSQL_TimeDiff(&config, "InsertIntoDB"),
			"CAST((julianday('now', 'localtime') - "
			"julianday(InsertIntoDB)) * 86400 AS INTEGER)") == 0);
	config.sql = "oracle";
	test_result(strcmp(SMSDSQL_DayMaskPredicate(&config),
			"BITAND(SendDays, %2) <> 0") == 0);
	test_result(strcmp(SMSDSQL_TimeDiff(&config, "InsertIntoDB"),
			"ROUND((CAST(CURRENT_TIMESTAMP AS DATE) - "
			"CAST(InsertIntoDB AS DATE)) * 86400)") == 0);
	config.sql = "access";
	test_result(strcmp(SMSDSQL_DayMaskPredicate(&config),
			"(SendDays AND %2) <> 0") == 0);
	test_result(strcmp(SMSDSQL_TimeDiff(&config, "InsertIntoDB"),
			"DateDiff('s', InsertIntoDB, Now())") == 0);
	config.sql = "mssql";
	test_result(strcmp(SMSDSQL_TimeDiff(&config, "InsertIntoDB"),
			"DATEDIFF(second, \"InsertIntoDB\", CURRENT_TIMESTAMP)") == 0);
	config.sql = NULL;
	test_result(strcmp(SMSDSQL_TimeDiff(&config, "InsertIntoDB"),
			"{fn TIMESTAMPDIFF(SQL_TSI_SECOND, InsertIntoDB, "
			"CURRENT_TIMESTAMP)}") == 0);
	test_result(find_day_mask >= 1 && find_day_mask <= 64);
	test_result((find_day_mask & (find_day_mask - 1)) == 0);
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

static void prepare_multipart_inbox(GSM_MultiSMSMessage *sms, int part)
{
	memset(sms, 0, sizeof(*sms));
	sms->Number = 1;
	GSM_SetDefaultSMSData(&sms->SMS[0]);
	sms->SMS[0].PDU = SMS_Deliver;
	sms->SMS[0].UDH.Type = UDH_ConcatenatedMessages;
	sms->SMS[0].UDH.ID8bit = 0x70;
	sms->SMS[0].UDH.ID16bit = -1;
	sms->SMS[0].UDH.AllParts = 2;
	sms->SMS[0].UDH.PartNumber = part;
}

static void prepare_user_udh_multipart_inbox(
	GSM_MultiSMSMessage *sms,
	int part,
	gboolean reference_16bit)
{
	static const unsigned char udh_8bit[] = {
		0x0b, 0x00, 0x03, 0x70, 0x02, 0x00,
		0x05, 0x04, 0x23, 0xf0, 0x00, 0x00};
	static const unsigned char udh_16bit[] = {
		0x0c, 0x05, 0x04, 0x23, 0xf0, 0x00, 0x00,
		0x08, 0x04, 0x00, 0x70, 0x02, 0x00};
	const unsigned char *udh;
	size_t udh_length;
	size_t part_offset;

	memset(sms, 0, sizeof(*sms));
	sms->Number = 1;
	GSM_SetDefaultSMSData(&sms->SMS[0]);
	sms->SMS[0].PDU = SMS_Deliver;
	if (reference_16bit) {
		udh = udh_16bit;
		udh_length = sizeof(udh_16bit);
		part_offset = 12;
	} else {
		udh = udh_8bit;
		udh_length = sizeof(udh_8bit);
		part_offset = 5;
	}
	memcpy(sms->SMS[0].UDH.Text, udh, udh_length);
	sms->SMS[0].UDH.Text[part_offset] = part;
	sms->SMS[0].UDH.Length = udh_length;
	GSM_DecodeUDHHeader(NULL, &sms->SMS[0].UDH);
	test_result(sms->SMS[0].UDH.Type == UDH_UserUDH);
}

static GSM_Error save_inbox_part(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *config)
{
	GSM_StringArray locations;
	GSM_Error error;

	GSM_StringArray_New(&locations);
	error = SMSDSQL.SaveInboxSMS(sms, config, &locations, NULL);
	test_result(error != ERR_NONE || locations.used == 1);
	GSM_StringArray_Free(&locations);
	return error;
}

static void free_inbox_groups(GSM_SMSDConfig *config)
{
	SMSD_SQLInboxGroup *group, *next;

	group = config->inbox_groups;
	while (group != NULL) {
		next = group->next;
		free(group);
		group = next;
	}
	config->inbox_groups = NULL;
}

static int inbox_group_count(const GSM_SMSDConfig *config)
{
	const SMSD_SQLInboxGroup *group;
	int count = 0;

	for (group = config->inbox_groups; group != NULL; group = group->next) {
		count++;
	}
	return count;
}

static void test_inbox_message_metadata(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_StringArray locations;
	GSM_Error error;
	int group, part;

	reset_mock();
	setup_config(&config);

	for (group = 0; group < 2; group++) {
		for (part = 1; part <= 2; part++) {
			prepare_multipart_inbox(&sms, part);
			GSM_StringArray_New(&locations);

			error = SMSDSQL.SaveInboxSMS(&sms, &config, &locations, NULL);

			test_result(error == ERR_NONE);
			test_result(locations.used == 1);
			GSM_StringArray_Free(&locations);
		}
	}

	memset(&sms, 0, sizeof(sms));
	sms.Number = 1;
	GSM_SetDefaultSMSData(&sms.SMS[0]);
	sms.SMS[0].PDU = SMS_Deliver;
	GSM_StringArray_New(&locations);
	error = SMSDSQL.SaveInboxSMS(&sms, &config, &locations, NULL);
	test_result(error == ERR_NONE);
	test_result(locations.used == 1);
	GSM_StringArray_Free(&locations);

	memset(&sms, 0, sizeof(sms));
	sms.Number = 1;
	GSM_SetDefaultSMSData(&sms.SMS[0]);
	sms.SMS[0].PDU = SMS_Deliver;
	sms.SMS[0].UDH.Type = UDH_ConcatenatedMessages;
	sms.SMS[0].UDH.ID8bit = 0x70;
	sms.SMS[0].UDH.ID16bit = -1;
	sms.SMS[0].UDH.AllParts = 3;
	sms.SMS[0].UDH.PartNumber = 2;
	GSM_StringArray_New(&locations);
	error = SMSDSQL.SaveInboxSMS(&sms, &config, &locations, NULL);
	test_result(error == ERR_NONE);
	test_result(locations.used == 1);
	GSM_StringArray_Free(&locations);

	test_result(inbox_insert_count == 6);
	test_result(inbox_metadata_count == 6);
	test_result(update_received_count == 6);

	for (part = 0; part < inbox_insert_count; part++) {
		test_result(strcmp(inbox_insert[part].processed, "true") == 0);
		test_result(strcmp(inbox_metadata[part].processed, "false") == 0);
	}
	test_result(inbox_insert[0].message_id == 0);
	test_result(inbox_insert[0].sequence_position == 1);
	test_result(inbox_insert[0].part_count == 2);
	test_result(inbox_insert[1].message_id == 101);
	test_result(inbox_insert[1].sequence_position == 2);
	test_result(inbox_insert[1].part_count == 2);
	test_result(inbox_insert[2].message_id == 0);
	test_result(inbox_insert[3].message_id == 103);
	test_result(inbox_insert[4].message_id == 0);
	test_result(inbox_insert[5].message_id == 0);

	test_result(inbox_metadata[0].message_id == 101);
	test_result(inbox_metadata[0].sequence_position == 1);
	test_result(inbox_metadata[0].part_count == 2);
	test_result(inbox_metadata[0].row_id == 101);
	test_result(inbox_metadata[1].message_id == 101);
	test_result(inbox_metadata[1].sequence_position == 2);
	test_result(inbox_metadata[1].part_count == 2);
	test_result(inbox_metadata[1].row_id == 102);

	test_result(inbox_metadata[2].message_id == 103);
	test_result(inbox_metadata[2].sequence_position == 1);
	test_result(inbox_metadata[2].part_count == 2);
	test_result(inbox_metadata[2].row_id == 103);
	test_result(inbox_metadata[3].message_id == 103);
	test_result(inbox_metadata[3].sequence_position == 2);
	test_result(inbox_metadata[3].part_count == 2);
	test_result(inbox_metadata[3].row_id == 104);

	test_result(inbox_metadata[4].message_id == 105);
	test_result(inbox_metadata[4].sequence_position == 1);
	test_result(inbox_metadata[4].part_count == 1);
	test_result(inbox_metadata[4].row_id == 105);

	test_result(inbox_metadata[5].message_id == 106);
	test_result(inbox_metadata[5].sequence_position == 2);
	test_result(inbox_metadata[5].part_count == 3);
	test_result(inbox_metadata[5].row_id == 106);
	free_inbox_groups(&config);
}

static void test_inbox_group_zero_timeout(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;

	reset_mock();
	setup_config(&config);
	config.multiparttimeout = 0;

	prepare_multipart_inbox(&sms, 1);
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);

	prepare_multipart_inbox(&sms, 2);
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);

	test_result(inbox_metadata_count == 2);
	test_result(inbox_metadata[0].message_id == 101);
	test_result(inbox_metadata[1].message_id == 102);
	free_inbox_groups(&config);
}

static void test_inbox_group_fixed_timeout(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;

	reset_mock();
	setup_config(&config);

	prepare_multipart_inbox(&sms, 1);
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);
	test_result(config.inbox_groups != NULL);
	config.inbox_groups->created =
		time(NULL) - config.multiparttimeout;

	prepare_multipart_inbox(&sms, 2);
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);

	test_result(inbox_metadata_count == 2);
	test_result(inbox_metadata[0].message_id == 101);
	test_result(inbox_metadata[1].message_id == 102);
	free_inbox_groups(&config);
}

static void test_inbox_group_cleanup_on_single_message(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;

	reset_mock();
	setup_config(&config);

	prepare_multipart_inbox(&sms, 1);
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);
	test_result(config.inbox_groups != NULL);
	config.inbox_groups->created =
		time(NULL) - config.multiparttimeout;

	memset(&sms, 0, sizeof(sms));
	sms.Number = 1;
	GSM_SetDefaultSMSData(&sms.SMS[0]);
	sms.SMS[0].PDU = SMS_Deliver;
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);
	test_result(config.inbox_groups == NULL);
}

static void test_inbox_polling_timeout_is_not_cached(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;

	reset_mock();
	setup_config(&config);
	prepare_multipart_inbox(&sms, 1);
	config.IncompleteMessageID = sms.SMS[0].UDH.ID8bit;
	config.IncompleteMessageTime = time(NULL) - config.multiparttimeout;

	test_result(SMSD_CheckMultipart(&config, &sms) == TRUE);
	test_result(config.ProcessingIncompleteMessage == TRUE);
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);
	test_result(config.inbox_groups == NULL);
	test_result(inbox_metadata[0].message_id == 101);
}

static void test_inbox_nokia_multipart_groups(void)
{
	static const GSM_UDH types[] = {
		UDH_NokiaRingtoneLong,
		UDH_NokiaOperatorLogoLong,
		UDH_NokiaWAPLong,
		UDH_NokiaCalendarLong,
		UDH_NokiaProfileLong,
		UDH_NokiaPhonebookLong,
		UDH_MMSIndicatorLong
	};
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;
	size_t i;
	int part;

	reset_mock();
	setup_config(&config);

	for (i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
		for (part = 1; part <= 2; part++) {
			prepare_multipart_inbox(&sms, part);
			sms.SMS[0].UDH.Type = types[i];
			error = save_inbox_part(&sms, &config);
			test_result(error == ERR_NONE);
		}
		test_result(inbox_metadata[2 * i].message_id == 101 + 2 * i);
		test_result(inbox_metadata[2 * i + 1].message_id == 101 + 2 * i);
	}
	test_result(config.inbox_groups == NULL);
}

static void test_inbox_user_udh_multipart_groups(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;

	reset_mock();
	setup_config(&config);

	prepare_user_udh_multipart_inbox(&sms, 1, FALSE);
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);
	prepare_user_udh_multipart_inbox(&sms, 1, TRUE);
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);
	prepare_user_udh_multipart_inbox(&sms, 2, FALSE);
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);
	prepare_user_udh_multipart_inbox(&sms, 2, TRUE);
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);

	test_result(inbox_metadata[0].message_id == 101);
	test_result(inbox_metadata[0].sequence_position == 1);
	test_result(inbox_metadata[0].part_count == 2);
	test_result(inbox_metadata[1].message_id == 102);
	test_result(inbox_metadata[1].sequence_position == 1);
	test_result(inbox_metadata[1].part_count == 2);
	test_result(inbox_metadata[2].message_id == 101);
	test_result(inbox_metadata[2].sequence_position == 2);
	test_result(inbox_metadata[2].part_count == 2);
	test_result(inbox_metadata[3].message_id == 102);
	test_result(inbox_metadata[3].sequence_position == 2);
	test_result(inbox_metadata[3].part_count == 2);
	test_result(config.inbox_groups == NULL);

	prepare_user_udh_multipart_inbox(&sms, 1, FALSE);
	sms.SMS[0].UDH.Text[0] = 4;
	sms.SMS[0].UDH.Length = 5;
	GSM_DecodeUDHHeader(NULL, &sms.SMS[0].UDH);
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);
	test_result(inbox_metadata[4].message_id == 105);
	test_result(inbox_metadata[4].part_count == 1);
	test_result(config.inbox_groups == NULL);
}

static void test_restore_inbox_groups(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;

	reset_mock();
	setup_config(&config);
	restore_inbox_group_count = 1;
	restore_inbox_groups[0].message_id = 4294967295LL;
	restore_inbox_groups[0].sender = "+420123456";
	restore_inbox_groups[0].smsc = "";
	restore_inbox_groups[0].udh = "050003700201";
	restore_inbox_groups[0].sequence_position = 1;
	restore_inbox_groups[0].part_count = 2;
	restore_inbox_groups[0].age = 10;

	error = SMSDSQL_RestoreInboxGroups(&config);
	test_result(error == ERR_NONE);
	test_result(inbox_group_count(&config) == 1);

	prepare_multipart_inbox(&sms, 2);
	EncodeUnicode(
		sms.SMS[0].Number,
		"00420123456",
		strlen("00420123456"));
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);
	test_result(inbox_metadata[0].message_id == 4294967295LL);
	test_result(config.inbox_groups == NULL);

	reset_mock();
	setup_config(&config);
	restore_inbox_group_count = 3;
	restore_inbox_groups[0].message_id = 300;
	restore_inbox_groups[0].sender = "";
	restore_inbox_groups[0].smsc = "";
	restore_inbox_groups[0].udh = "050003700201";
	restore_inbox_groups[0].sequence_position = 1;
	restore_inbox_groups[0].part_count = 2;
	restore_inbox_groups[0].age = 10;
	restore_inbox_groups[1] = restore_inbox_groups[0];
	restore_inbox_groups[1].udh = "050003700202";
	restore_inbox_groups[1].sequence_position = 2;
	restore_inbox_groups[2] = restore_inbox_groups[1];

	error = SMSDSQL_RestoreInboxGroups(&config);
	test_result(error == ERR_NONE);
	test_result(config.inbox_groups == NULL);

	reset_mock();
	setup_config(&config);
	restore_inbox_group_count = 1;
	restore_inbox_groups[0].message_id = 400;
	restore_inbox_groups[0].sender = "";
	restore_inbox_groups[0].smsc = "";
	restore_inbox_groups[0].udh = "0B0003700201050423F00000";
	restore_inbox_groups[0].sequence_position = 1;
	restore_inbox_groups[0].part_count = 2;
	restore_inbox_groups[0].age = 10;

	error = SMSDSQL_RestoreInboxGroups(&config);
	test_result(error == ERR_NONE);
	test_result(config.inbox_groups != NULL);

	prepare_user_udh_multipart_inbox(&sms, 2, FALSE);
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);
	test_result(inbox_metadata[0].message_id == 400);
	test_result(config.inbox_groups == NULL);
}

static void test_inbox_group_survives_post_insert_failure(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;

	reset_mock();
	setup_config(&config);

	prepare_multipart_inbox(&sms, 1);
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);
	test_result(config.inbox_groups != NULL);

	update_received_error = ERR_SQL;
	prepare_multipart_inbox(&sms, 2);
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_SQL);
	test_result(config.inbox_groups != NULL);
	test_result(inbox_metadata[1].message_id == 101);

	update_received_error = ERR_NONE;
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);
	test_result(config.inbox_groups == NULL);
	test_result(inbox_metadata[2].message_id == 101);
}

static void test_inbox_group_capacity(void)
{
	GSM_SMSDConfig config;
	GSM_MultiSMSMessage sms;
	GSM_Error error;
	int i;

	reset_mock();
	setup_config(&config);

	for (i = 0; i < GSM_MAX_MULTI_SMS + 1; i++) {
		prepare_multipart_inbox(&sms, 1);
		sms.SMS[0].UDH.ID8bit = i;
		error = save_inbox_part(&sms, &config);
		test_result(error == ERR_NONE);
	}
	test_result(inbox_group_count(&config) == GSM_MAX_MULTI_SMS + 1);

	prepare_multipart_inbox(&sms, 2);
	sms.SMS[0].UDH.ID8bit = 0;
	error = save_inbox_part(&sms, &config);
	test_result(error == ERR_NONE);

	test_result(inbox_metadata_count == GSM_MAX_MULTI_SMS + 2);
	test_result(inbox_metadata[GSM_MAX_MULTI_SMS + 1].message_id == 101);
	test_result(inbox_group_count(&config) == GSM_MAX_MULTI_SMS);
	free_inbox_groups(&config);
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
	test_phone_status_timeout();
	test_find_outbox_order();
	test_find_outbox_without_sent_item();
	test_matching_sent_item_is_skipped();
	test_inherited_sent_item_validity_is_skipped();
	test_matching_sending_error_is_skipped();
	test_reused_sent_item_id_is_rejected();
	test_reconciliation_query_error_is_retryable();
	test_delivery_report_order();
	test_unmatched_delivery_report_id();
	test_inbox_message_metadata();
	test_inbox_group_zero_timeout();
	test_inbox_group_fixed_timeout();
	test_inbox_group_cleanup_on_single_message();
	test_inbox_polling_timeout_is_not_cached();
	test_inbox_nokia_multipart_groups();
	test_inbox_user_udh_multipart_groups();
	test_restore_inbox_groups();
	test_inbox_group_survives_post_insert_failure();
	test_inbox_group_capacity();
	test_send_finalized_failure_is_removed();
	test_send_collision_is_left_queued();
	test_send_reconciliation_error_is_left_queued();
	return 0;
}
