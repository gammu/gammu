/**
 * libsql database service
 *
 * Part of Gammu project
 *
 * Copyright (C) 2009-2011 Michal Čihař
 *           (c) 2010 Miloslav Semler
 *
 * Licensed under GNU GPL version 2 or later
 */

#define _XOPEN_SOURCE
#define _BSD_SOURCE
#include <time.h>
#include <gammu.h>
#include "../../helper/strptime.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#ifdef WIN32
#include <windows.h>
#endif

#include "../core.h"
#include "../../helper/string.h"

/**
 * Returns name of the SQL dialect to use.
 */
const char *SMSDSQL_SQLName(GSM_SMSDConfig * Config)
{
	if (Config->sql != NULL) {
		return Config->sql;
	} else {
		return Config->driver;
	}
}

/* FIXME: I know this is broken, need to figure out better way */
const char now_plus_odbc[] = "{fn CURRENT_TIMESTAMP()} + INTERVAL %d SECOND";
const char now_plus_mysql[] = "(NOW() + INTERVAL %d SECOND) + 0";
const char now_plus_pgsql[] = "now() + interval '%d seconds'";
const char now_plus_sqlite[] = "datetime('now', '+%d seconds')";
const char now_plus_freetds[] = "DATEADD('second', %d, CURRENT_TIMESTAMP)";
const char now_plus_access[] = "now()+#00:00:%d#";
const char now_plus_fallback[] = "NOW() + INTERVAL %d SECOND";


static const char *SMSDSQL_NowPlus(GSM_SMSDConfig * Config, int seconds)
{
	const char *driver_name;
	static char result[100];

	driver_name = SMSDSQL_SQLName(Config);

	if (strcasecmp(driver_name, "mysql") == 0 || strcasecmp(driver_name, "native_mysql") == 0) {
		sprintf(result, now_plus_mysql, seconds);
	} else if (strcasecmp(driver_name, "pgsql") == 0 || strcasecmp(driver_name, "native_pgsql") == 0) {
		sprintf(result, now_plus_pgsql, seconds);
	} else if (strncasecmp(driver_name, "sqlite", 6) == 0) {
		sprintf(result, now_plus_sqlite, seconds);
	} else if (strcasecmp(driver_name, "freetds") == 0) {
		sprintf(result, now_plus_freetds, seconds);
	} else if (strcasecmp(driver_name, "access") == 0) {
		sprintf(result, now_plus_access, seconds);
	} else if (strcasecmp(driver_name, "odbc") == 0) {
		sprintf(result, now_plus_odbc, seconds);
	} else {
		sprintf(result, now_plus_fallback, seconds);
	}
	return result;
}

const char escape_char_odbc[] = "";
const char escape_char_mysql[] = "`";
const char escape_char_pgsql[] = "\"";
const char escape_char_sqlite[] = "";
const char escape_char_freetds[] = "";
const char escape_char_fallback[] = "";

static const char *SMSDSQL_EscapeChar(GSM_SMSDConfig * Config)
{
	const char *driver_name;

	driver_name = SMSDSQL_SQLName(Config);

	if (strcasecmp(driver_name, "mysql") == 0 || strcasecmp(driver_name, "native_mysql") == 0) {
		return escape_char_mysql;
	} else if (strcasecmp(driver_name, "pgsql") == 0 || strcasecmp(driver_name, "native_pgsql") == 0) {
		return escape_char_pgsql;
	} else if (strncasecmp(driver_name, "sqlite", 6) == 0) {
		return escape_char_sqlite;
	} else if (strcasecmp(driver_name, "freetds") == 0 || strcasecmp(driver_name, "mssql") == 0 || strcasecmp(driver_name, "sybase") == 0) {
		return escape_char_freetds;
	} else if (strcasecmp(Config->driver, "odbc") == 0) {
		return escape_char_odbc;
	} else {
		return escape_char_fallback;
	}
}

const char top_clause_access[] = "TOP";
const char top_clause_fallback[] = "";

static const char *SMSDSQL_TopClause(GSM_SMSDConfig * Config, const char *count)
{
	const char *driver_name;
	static char result[100];

	driver_name = SMSDSQL_SQLName(Config);

	if (strcasecmp(driver_name, "access") == 0) {
		strcpy(result, top_clause_access);
		strcat(result, " ");
		strcat(result, count);
		strcat(result, " ");
		return result;
	} else {
		return top_clause_fallback;
	}
}

const char limit_clause_access[] = "";
const char limit_clause_fallback[] = "LIMIT";

static const char *SMSDSQL_LimitClause(GSM_SMSDConfig * Config, const char *count)
{
	const char *driver_name;
	static char result[100];

	driver_name = SMSDSQL_SQLName(Config);

	if (strcasecmp(driver_name, "access") == 0) {
		return limit_clause_access;
	} else {
		strcpy(result, limit_clause_fallback);
		strcat(result, " ");
		strcat(result, count);
		return result;
	}
}

const char now_odbc[] = "{fn CURRENT_TIMESTAMP()}";
const char now_mysql[] = "NOW()";
const char now_pgsql[] = "now()";
const char now_sqlite[] = "datetime('now')";
const char now_freetds[] = "CURRENT_TIMESTAMP";
const char now_access[] = "now()";
const char now_fallback[] = "NOW()";

const char currtime_odbc[] = "{fn CURTIME()}";
const char currtime_mysql[] = "CURTIME()";
const char currtime_pgsql[] = "localtime";
const char currtime_sqlite[] = "time('now')";
const char currtime_freetds[] = "CURRENT_TIME";
const char currtime_fallback[] = "CURTIME()";

static const char *SMSDSQL_CurrentTime(GSM_SMSDConfig * Config)
{
	const char *driver_name;

	driver_name = SMSDSQL_SQLName(Config);

	if (strcasecmp(driver_name, "mysql") == 0 || strcasecmp(driver_name, "native_mysql") == 0) {
		return currtime_mysql;
	} else if (strcasecmp(driver_name, "pgsql") == 0 || strcasecmp(driver_name, "native_pgsql") == 0) {
		return currtime_pgsql;
	} else if (strncasecmp(driver_name, "sqlite", 6) == 0) {
		return currtime_sqlite;
	} else if (strcasecmp(driver_name, "freetds") == 0 || strcasecmp(driver_name, "mssql") == 0 || strcasecmp(driver_name, "sybase") == 0) {
		return currtime_freetds;
	} else if (strcasecmp(Config->driver, "odbc") == 0) {
		return currtime_odbc;
	} else {
		return currtime_fallback;
	}
}
static const char *SMSDSQL_Now(GSM_SMSDConfig * Config)
{
	const char *driver_name;

	driver_name = SMSDSQL_SQLName(Config);

	if (strcasecmp(driver_name, "mysql") == 0 || strcasecmp(driver_name, "native_mysql") == 0) {
		return now_mysql;
	} else if (strcasecmp(driver_name, "pgsql") == 0 || strcasecmp(driver_name, "native_pgsql") == 0) {
		return now_pgsql;
	} else if (strncasecmp(driver_name, "sqlite", 6) == 0) {
		return now_sqlite;
	} else if (strcasecmp(driver_name, "freetds") == 0 || strcasecmp(driver_name, "mssql") == 0 || strcasecmp(driver_name, "sybase") == 0) {
		return now_freetds;
	} else if (strcasecmp(Config->driver, "access") == 0) {
		return now_access;
	} else if (strcasecmp(Config->driver, "odbc") == 0) {
		return now_odbc;
	} else {
		return now_fallback;
	}
}
static SQL_Error SMSDSQL_Query(GSM_SMSDConfig * Config, const char *query, SQL_result * res)
{
	SQL_Error error = SQL_TIMEOUT;
	int attempts = 1;
	struct GSM_SMSDdbobj *db = Config->db;

	for (attempts = 1; attempts <= Config->backend_retries; attempts++) {
		SMSD_Log(DEBUG_SQL, Config, "Execute SQL: %s", query);
		error = db->Query(Config, query, res);
		if (error == SQL_OK) {
			return error;
		}

		if (error != SQL_TIMEOUT){
			SMSD_Log(DEBUG_INFO, Config, "SQL failure: %d", error);
			return error;
		}

		SMSD_Log(DEBUG_INFO, Config, "SQL failed (timeout): %s", query);
		/* We will try to reconnect */
		SMSD_Log(DEBUG_INFO, Config, "reconnecting to database!");
		while (error != SQL_OK && attempts < Config->backend_retries) {
			SMSD_Log(DEBUG_INFO, Config, "Reconnecting after %d seconds...", attempts * attempts);
			sleep(attempts * attempts);
			db->Free(Config);
			error = db->Connect(Config);
			attempts++;
		}
	}
	return error;
}

void SMSDSQL_Time2String(GSM_SMSDConfig * Config, time_t timestamp, char *static_buff, size_t size)
{
	struct tm *timestruct;
	const char *driver_name;

	driver_name = SMSDSQL_SQLName(Config);

	if (timestamp == -2) {
		strcpy(static_buff, "0000-00-00 00:00:00");
	} else if (strcasecmp(driver_name, "pgsql") == 0 || strcasecmp(driver_name, "native_pgsql") == 0) {
		timestruct = gmtime(&timestamp);
		strftime(static_buff, size, "%Y-%m-%d %H:%M:%S GMT", timestruct);
	} else if (strcasecmp(driver_name, "access") == 0) {
		timestruct = gmtime(&timestamp);
		strftime(static_buff, size, "'%Y-%m-%d %H:%M:%S'", timestruct);
	} else if (strcasecmp(Config->driver, "odbc") == 0) {
		timestruct = gmtime(&timestamp);
		strftime(static_buff, size, "{ ts '%Y-%m-%d %H:%M:%S' }", timestruct);
	} else {
		timestruct = localtime(&timestamp);
		strftime(static_buff, size, "%Y-%m-%d %H:%M:%S", timestruct);
	}
}

static SQL_Error SMSDSQL_NamedQuery(GSM_SMSDConfig * Config, const char *sql_query, GSM_SMSMessage *sms,
	const SQL_Var *params, SQL_result * res)
{
	char buff[65536], *ptr, c, static_buff[8192];
	char *buffer2, *end;
	const char *to_print, *q = sql_query;
	int int_to_print;
	int numeric;
	int n, argc = 0;
	struct GSM_SMSDdbobj *db = Config->db;

	GSM_NetworkInfo NetInfo;
	GSM_Error error;
	char *NetCode, *NetName;
	if ( (error = GSM_GetNetworkInfo(Config->gsm,&NetInfo)) == ERR_NONE) {
		NetCode = NetInfo.NetworkCode;
		if (NetInfo.NetworkName[0] != 0x00 || NetInfo.NetworkName[1] != 0x00) {
			NetName = DecodeUnicodeConsole(NetInfo.NetworkName);
		}
	}

	if (params != NULL) {
		while (params[argc].type != SQL_TYPE_NONE) argc++;
	}

	ptr = buff;

	do {
		if (*q != '%') {
			*ptr++ = *q;
			continue;
		}
		c = *(++q);
		if( c >= '0' && c <= '9'){
			n = strtoul(q, &end, 10) - 1;
			if (n < argc && n >= 0) {
				switch(params[n].type){
					case SQL_TYPE_INT:
						ptr += sprintf(ptr, "%i", params[n].v.i);
						break;
					case SQL_TYPE_STRING:
						buffer2 = db->QuoteString(Config, params[n].v.s);
						memcpy(ptr, buffer2, strlen(buffer2));
						ptr += strlen(buffer2);
						free(buffer2);
						break;
					default:
						SMSD_Log(DEBUG_ERROR, Config, "SQL: unknown type: %i (application bug) in query: `%s`", params[n].type, sql_query);
						return SQL_BUG;
						break;
				}
			} else {
				SMSD_Log(DEBUG_ERROR, Config, "SQL: wrong number of parameter: %i (max %i) in query: `%s`", n+1, argc, sql_query);
				return SQL_BUG;
			}
			q = end - 1;
			continue;
		}
		numeric = 0;
		to_print = NULL;
		switch (c) {
			case 'I':
				to_print = Config->Status->IMEI;
				break;
			case 'P':
				to_print = Config->PhoneID;
				break;
			case 'O':
				to_print = NetCode;
				break;
			case 'M':
				to_print = NetName;
				break;
			case 'N':
				snprintf(static_buff, sizeof(static_buff), "Gammu %s, %s, %s", GAMMU_VERSION, GetOS(), GetCompiler());
				to_print = static_buff;
				break;
			case 'A':
				to_print = Config->CreatorID;
				break;
			default:
				if (sms != NULL) {
					switch (c) {
						case 'R':
							EncodeUTF8(static_buff, sms->Number);
							to_print = static_buff;
							break;
						case 'F':
							EncodeUTF8(static_buff, sms->SMSC.Number);
							to_print = static_buff;
							break;
						case 'u':
							if (sms->UDH.Type != UDH_NoUDH) {
								EncodeHexBin(static_buff, sms->UDH.Text, sms->UDH.Length);
								to_print = static_buff;
							}else{
								to_print = "";
							}
							break;
						case 'x':
							int_to_print =  sms->Class;
							numeric = 1;
							break;
						case 'c':
							to_print = GSM_SMSCodingToString(sms->Coding);
							break;
						case 't':
							int_to_print =  sms->MessageReference;
							numeric = 1;
							break;
						case 'E':
							switch (sms->Coding) {
								case SMS_Coding_Unicode_No_Compression:
								case SMS_Coding_Default_No_Compression:
									EncodeHexUnicode(static_buff, sms->Text, UnicodeLength(sms->Text));
									break;
								case SMS_Coding_8bit:
									EncodeHexBin(static_buff, sms->Text, sms->Length);
									break;
								default:
									*static_buff = '\0';
									break;
							}
							to_print = static_buff;
							break;
						case 'T':
							switch (sms->Coding) {
								case SMS_Coding_Unicode_No_Compression:
								case SMS_Coding_Default_No_Compression:
									EncodeUTF8(static_buff, sms->Text);
									to_print = static_buff;
									break;
								default:
									to_print = "";
									break;
							}
							break;
						case 'V':
							if (sms->SMSC.Validity.Format == SMS_Validity_RelativeFormat) {
								int_to_print = sms->SMSC.Validity.Relative;
							} else {
								int_to_print =  -1;
							}
							numeric = 1;
							break;
						case 'C':
							SMSDSQL_Time2String(Config, Fill_Time_T(sms->SMSCTime), static_buff, sizeof(static_buff));
							to_print = static_buff;
							break;
						case 'd':
							SMSDSQL_Time2String(Config, Fill_Time_T(sms->DateTime), static_buff, sizeof(static_buff));
							to_print = static_buff;
							break;
						case 'e':
							int_to_print = sms->DeliveryStatus;
							numeric = 1;
							break;
						default:
							SMSD_Log(DEBUG_ERROR, Config, "SQL: uexpected char '%c' in query: %s", c, sql_query);
							return SQL_BUG;

					} /* end of switch */
				} else {
					SMSD_Log(DEBUG_ERROR, Config, "Syntax error in query.. uexpected char '%c' in query: %s", c, sql_query);
					return SQL_BUG;
				}
				break;
		} /* end of switch */
		if (numeric) {
			ptr += sprintf(ptr, "%i", int_to_print);
		} else if (to_print != NULL) {
			buffer2 = db->QuoteString(Config, to_print);
			memcpy(ptr, buffer2, strlen(buffer2));
			ptr += strlen(buffer2);
			free(buffer2);
		} else {
			memcpy(ptr, "NULL", 4);
			ptr += 4;
		}
	} while (*(++q) != '\0');
	*ptr = '\0';
	return SMSDSQL_Query(Config, buff, res);

}

static GSM_Error SMSDSQL_CheckTable(GSM_SMSDConfig * Config, const char *table)
{
	SQL_result res;
	char buffer[200];
	SQL_Error error;
	struct GSM_SMSDdbobj *db = Config->db;
	const char *escape_char;

	escape_char = SMSDSQL_EscapeChar(Config);

	sprintf(buffer, "SELECT %s %sID%s FROM %s %s", SMSDSQL_TopClause(Config, "1"), escape_char, escape_char, table, SMSDSQL_LimitClause(Config, "1"));
	error = SMSDSQL_Query(Config, buffer, &res);
	if (error != SQL_OK) {
		SMSD_Log(DEBUG_ERROR, Config, "Table %s not found, disconnecting!", table);
		db->Free(Config);
		return ERR_UNKNOWN;
	}
	db->FreeResult(Config, &res);
	return ERR_NONE;
}

/* Disconnects from a database */
static GSM_Error SMSDSQL_Free(GSM_SMSDConfig * Config)
{
	int i;
	SMSD_Log(DEBUG_SQL, Config, "Disconnecting from SQL database.");
	Config->db->Free(Config);
	/* free configuration */
	for(i = 0; i < SQL_QUERY_LAST_NO; i++){
		free(Config->SMSDSQL_queries[i]);
		Config->SMSDSQL_queries[i] = NULL;
	}
	return ERR_NONE;
}

/* Connects to database */
static GSM_Error SMSDSQL_Init(GSM_SMSDConfig * Config)
{
	SQL_result res;
	int version;
	GSM_Error error;
	struct GSM_SMSDdbobj *db;
	const char *escape_char;
	char buffer[100];

#ifdef WIN32
	_tzset();
#else
	tzset();
#endif

	db = Config->db;

	if (db->Connect(Config) != SQL_OK)
		return ERR_UNKNOWN;

	error = SMSDSQL_CheckTable(Config, "outbox");
	if (error != ERR_NONE)
		return error;
	error = SMSDSQL_CheckTable(Config, "outbox_multipart");
	if (error != ERR_NONE)
		return error;
	error = SMSDSQL_CheckTable(Config, "sentitems");
	if (error != ERR_NONE)
		return error;
	error = SMSDSQL_CheckTable(Config, "inbox");
	if (error != ERR_NONE)
		return error;

	escape_char = SMSDSQL_EscapeChar(Config);

	sprintf(buffer, "SELECT %sVersion%s FROM gammu", escape_char, escape_char);
	if (SMSDSQL_Query(Config, buffer, &res) != SQL_OK) {
		db->Free(Config);
		return ERR_UNKNOWN;
	}
	if (db->NextRow(Config, &res) != 1) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to seek to first row!");
		db->FreeResult(Config, &res);
		db->Free(Config);
		return ERR_UNKNOWN;
	}
	version = db->GetNumber(Config, &res, 0);
	db->FreeResult(Config, &res);
	if (SMSD_CheckDBVersion(Config, version) != ERR_NONE) {
		db->Free(Config);
		return ERR_UNKNOWN;
	}

	SMSD_Log(DEBUG_INFO, Config, "Connected to Database %s: %s on %s", Config->driver, Config->database, Config->host);

	return ERR_NONE;
}

static GSM_Error SMSDSQL_InitAfterConnect(GSM_SMSDConfig * Config)
{
	SQL_result res;
	struct GSM_SMSDdbobj *db = Config->db;
	SQL_Var vars[3] = {{SQL_TYPE_STRING, {NULL}}, {SQL_TYPE_STRING, {NULL}}, {SQL_TYPE_NONE, {NULL}}};

	if (SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_DELETE_PHONE], NULL, NULL, &res) != SQL_OK) {
		SMSD_Log(DEBUG_INFO, Config, "Error deleting from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	db->FreeResult(Config, &res);

	SMSD_Log(DEBUG_INFO, Config, "Inserting phone info");
	vars[0].v.s = Config->enable_send ? "yes" : "no";
	vars[1].v.s = Config->enable_receive ? "yes" : "no";

	if (SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_INSERT_PHONE], NULL, vars, &res) != SQL_OK) {
		SMSD_Log(DEBUG_INFO, Config, "Error inserting into database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	db->FreeResult(Config, &res);

	return ERR_NONE;
}

/* Save SMS from phone (called Inbox sms - it's in phone Inbox) somewhere */
static GSM_Error SMSDSQL_SaveInboxSMS(GSM_MultiSMSMessage * sms, GSM_SMSDConfig * Config, char **Locations)
{
	SQL_result res, res2;
	SQL_Var vars[3];
	struct GSM_SMSDdbobj *db = Config->db;
	const char *q, *status;

	char smstext[3 * GSM_MAX_SMS_LENGTH + 1];
	char destinationnumber[3 * GSM_MAX_NUMBER_LENGTH + 1];
	char smsc_message[3 * GSM_MAX_NUMBER_LENGTH + 1];
	int i;
	time_t t_time1, t_time2;
	gboolean found;
	long diff;
	unsigned long long new_id;
	size_t locations_size = 0, locations_pos = 0;
	const char *state, *smsc;

	*Locations = NULL;

	for (i = 0; i < sms->Number; i++) {
		EncodeUTF8(destinationnumber, sms->SMS[i].Number);
		EncodeUTF8(smsc_message, sms->SMS[i].SMSC.Number);
		if (sms->SMS[i].PDU == SMS_Status_Report) {
			EncodeUTF8(smstext, sms->SMS[i].Text);
			SMSD_Log(DEBUG_INFO, Config, "Delivery report: %s to %s", smstext, destinationnumber);

			if (SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_SELECT], &sms->SMS[i], NULL, &res) != SQL_OK) {
				SMSD_Log(DEBUG_INFO, Config, "Error reading from database (%s)", __FUNCTION__);
				return ERR_UNKNOWN;
			}

			found = FALSE;
			while (db->NextRow(Config, &res)) {
				smsc = db->GetString(Config, &res, 4);
				state = db->GetString(Config, &res, 1);
				SMSD_Log(DEBUG_NOTICE, Config, "Checking for delivery report, SMSC=%s, state=%s", smsc, state);

				if (strcmp(smsc, smsc_message) != 0) {
					if (Config->skipsmscnumber[0] == 0 || strcmp(Config->skipsmscnumber, smsc)) {
						continue;
					}
				}

				if (strcmp(state, "SendingOK") == 0 || strcmp(state, "DeliveryPending") == 0) {
					t_time1 = db->GetDate(Config, &res, 2);
					if (t_time1 < 0) {
						SMSD_Log(DEBUG_ERROR, Config, "Invalid SendingDateTime -1 for SMS TPMR=%i", sms->SMS[i].MessageReference);
						return ERR_UNKNOWN;
					}
					t_time2 = Fill_Time_T(sms->SMS[i].DateTime);
					diff = t_time2 - t_time1;

					if (diff > -Config->deliveryreportdelay && diff < Config->deliveryreportdelay) {
						found = TRUE;
						break;
					} else {
						SMSD_Log(DEBUG_NOTICE, Config,
							 "Delivery report would match, but time delta is too big (%ld), consider increasing DeliveryReportDelay", diff);
					}
				}
			}

			if (found) {
				if (!strcmp(smstext, "Delivered")) {
					q = Config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_UPDATE_DELIVERED];
				} else {
					q = Config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_UPDATE];
				}

				if (!strcmp(smstext, "Delivered")) {
					status = "DeliveryOK";
				} else if (!strcmp(smstext, "Failed")) {
					status = "DeliveryFailed";
				} else if (!strcmp(smstext, "Pending")) {
					status = "DeliveryPending";
				} else if (!strcmp(smstext, "Unknown")) {
					status = "DeliveryUnknown";
				} else {
					status = "";
				}

				vars[0].type = SQL_TYPE_STRING;
				vars[0].v.s = status;			/* Status */
				vars[1].type = SQL_TYPE_INT;
				vars[1].v.i = (long)db->GetNumber(Config, &res, 0); /* ID */
				vars[2].type = SQL_TYPE_NONE;

				if (SMSDSQL_NamedQuery(Config, q, &sms->SMS[i], vars, &res2) != SQL_OK) {
					SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
					return ERR_UNKNOWN;
				}
				db->FreeResult(Config, &res2);
			}
			db->FreeResult(Config, &res);
			continue;
		}

		if (sms->SMS[i].PDU != SMS_Deliver)
			continue;

		if (SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_SAVE_INBOX_SMS_INSERT], &sms->SMS[i], NULL, &res) != SQL_OK) {
			SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}

		new_id = db->SeqID(Config, "inbox_ID_seq");
		if (new_id == 0) {
			SMSD_Log(DEBUG_INFO, Config, "Failed to get inserted row ID (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		SMSD_Log(DEBUG_NOTICE, Config, "Inserted message id %lu", (long)new_id);

		db->FreeResult(Config, &res);

		if (new_id != 0) {
			if (locations_pos + 10 >= locations_size) {
				locations_size += 40;
				*Locations = (char *)realloc(*Locations, locations_size);
				assert(*Locations != NULL);
				if (locations_pos == 0) {
					*Locations[0] = 0;
				}
			}
			locations_pos += sprintf((*Locations) + locations_pos, "%lu ", (long)new_id);
		}

		if (SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_UPDATE_RECEIVED], &sms->SMS[i], NULL, &res2) != SQL_OK) {
			SMSD_Log(DEBUG_INFO, Config, "Error updating number of received messages (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		db->FreeResult(Config, &res2);

	}

	return ERR_NONE;
}

static GSM_Error SMSDSQL_RefreshSendStatus(GSM_SMSDConfig * Config, char *ID)
{
	SQL_result res;
	struct GSM_SMSDdbobj *db = Config->db;
	SQL_Var vars[2] = {
		{SQL_TYPE_STRING, {ID}},
		{SQL_TYPE_NONE, {NULL}}};

	if (SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_REFRESH_SEND_STATUS], NULL, vars, &res) != SQL_OK) {
		SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}

	if (db->AffectedRows(Config, &res) == 0) {
		db->FreeResult(Config, &res);
		return ERR_UNKNOWN;
	}

	db->FreeResult(Config, &res);
	return ERR_NONE;
}

/* Find one multi SMS to sending and return it (or return ERR_EMPTY)
 * There is also set ID for SMS
 */
static GSM_Error SMSDSQL_FindOutboxSMS(GSM_MultiSMSMessage * sms, GSM_SMSDConfig * Config, char *ID)
{
	SQL_result res;
	struct GSM_SMSDdbobj *db = Config->db;
	int i;
	time_t timestamp;
	const char *coding;
	const char *text;
	size_t text_len;
	const char *text_decoded;
	const char *destination;
	const char *udh;
	const char *q;
	size_t udh_len;
	SQL_Var vars[3];

	vars[0].type = SQL_TYPE_INT;
	vars[0].v.i = 1;
	vars[1].type = SQL_TYPE_NONE;

	while (TRUE) {
		if (SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_FIND_OUTBOX_SMS_ID], NULL, vars, &res) != SQL_OK) {
			SMSD_Log(DEBUG_INFO, Config, "Error reading from database (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}

		if (db->NextRow(Config, &res) != 1) {
			db->FreeResult(Config, &res);
			return ERR_EMPTY;
		}

		sprintf(ID, "%ld", (long)db->GetNumber(Config, &res, 0));
		timestamp = db->GetDate(Config, &res, 1);

		db->FreeResult(Config, &res);

		if (timestamp == -1) {
			SMSD_Log(DEBUG_INFO, Config, "Invalid date for InsertIntoDB.");
			return ERR_UNKNOWN;
		}

		SMSDSQL_Time2String(Config, timestamp, Config->DT, sizeof(Config->DT));
		if (SMSDSQL_RefreshSendStatus(Config, ID) == ERR_NONE) {
			break;
		}
	}

	sms->Number = 0;
	for (i = 0; i < GSM_MAX_MULTI_SMS; i++) {
		GSM_SetDefaultSMSData(&sms->SMS[i]);
		sms->SMS[i].SMSC.Number[0] = 0;
		sms->SMS[i].SMSC.Number[1] = 0;
	}

	for (i = 1; i < GSM_MAX_MULTI_SMS + 1; i++) {
		vars[0].type = SQL_TYPE_STRING;
		vars[0].v.s = ID;
		vars[1].type = SQL_TYPE_INT;
		vars[1].v.i = i;
		vars[2].type = SQL_TYPE_NONE;
		if (i == 1) {
			q = Config->SMSDSQL_queries[SQL_QUERY_FIND_OUTBOX_BODY];
		} else {
			q = Config->SMSDSQL_queries[SQL_QUERY_FIND_OUTBOX_MULTIPART];
		}
		if (SMSDSQL_NamedQuery(Config, q, NULL, vars, &res) != SQL_OK) {
			SMSD_Log(DEBUG_ERROR, Config, "Error reading from database (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}

		if (db->NextRow(Config, &res) != 1) {
			db->FreeResult(Config, &res);
			return ERR_NONE;
		}

		coding = db->GetString(Config, &res, 1);
		text = db->GetString(Config, &res, 0);
		if (text == NULL) {
			text_len = 0;
		} else {
			text_len = strlen(text);
		}
		text_decoded = db->GetString(Config, &res, 4);
		udh = db->GetString(Config, &res, 2);
		if (udh == NULL) {
			udh_len = 0;
		} else {
			udh_len = strlen(udh);
		}

		sms->SMS[sms->Number].Coding = GSM_StringToSMSCoding(coding);
		if (sms->SMS[sms->Number].Coding == 0) {
			if (text == NULL || text_len == 0) {
				SMSD_Log(DEBUG_NOTICE, Config, "Assuming default coding for text message");
				sms->SMS[sms->Number].Coding = SMS_Coding_Default_No_Compression;
			} else {
				SMSD_Log(DEBUG_NOTICE, Config, "Assuming 8bit coding for binary message");
				sms->SMS[sms->Number].Coding = SMS_Coding_8bit;
			}
		}

		if (text == NULL || text_len == 0) {
			if (text_decoded == NULL) {
				SMSD_Log(DEBUG_ERROR, Config, "Message without text!");
				return ERR_UNKNOWN;
			} else {
				SMSD_Log(DEBUG_NOTICE, Config, "Message: %s", text_decoded);
				DecodeUTF8(sms->SMS[sms->Number].Text, text_decoded, strlen(text_decoded));
			}
		} else {
			switch (sms->SMS[sms->Number].Coding) {
				case SMS_Coding_Unicode_No_Compression:

				case SMS_Coding_Default_No_Compression:
					DecodeHexUnicode(sms->SMS[sms->Number].Text, text, text_len);
					break;

				case SMS_Coding_8bit:
					DecodeHexBin(sms->SMS[sms->Number].Text, text, text_len);
					sms->SMS[sms->Number].Length = text_len / 2;
					break;

				default:
					break;
			}
		}

		if (i == 1) {
			destination = db->GetString(Config, &res, 6);
			if (destination == NULL) {
				SMSD_Log(DEBUG_ERROR, Config, "Message without recipient!");
				return ERR_UNKNOWN;
			}
			DecodeUTF8(sms->SMS[sms->Number].Number, destination, strlen(destination));
		} else {
			CopyUnicodeString(sms->SMS[sms->Number].Number, sms->SMS[0].Number);
		}

		sms->SMS[sms->Number].UDH.Type = UDH_NoUDH;
		if (udh != NULL && udh_len != 0) {
			sms->SMS[sms->Number].UDH.Type = UDH_UserUDH;
			sms->SMS[sms->Number].UDH.Length = udh_len / 2;
			DecodeHexBin(sms->SMS[sms->Number].UDH.Text, udh, udh_len);
		}

		sms->SMS[sms->Number].Class = db->GetNumber(Config, &res, 3);
		sms->SMS[sms->Number].PDU = SMS_Submit;
		sms->Number++;

		if (i == 1) {
			strcpy(Config->CreatorID, db->GetString(Config, &res, 10));
			Config->relativevalidity = db->GetNumber(Config, &res, 8);

			Config->currdeliveryreport = db->GetBool(Config, &res, 9);

			/* Is this a multipart message? */
			if (!db->GetBool(Config, &res, 7)) {
				db->FreeResult(Config, &res);
				break;
			}

		}
		db->FreeResult(Config, &res);
	}

	return ERR_NONE;
}

/* After sending SMS is moved to Sent Items or Error Items. */
static GSM_Error SMSDSQL_MoveSMS(GSM_MultiSMSMessage * sms UNUSED, GSM_SMSDConfig * Config, char *ID, gboolean alwaysDelete UNUSED, gboolean sent UNUSED)
{
	SQL_result res;
	SQL_Var vars[2];
	struct GSM_SMSDdbobj *db = Config->db;

	vars[0].type = SQL_TYPE_STRING;
	vars[0].v.s = ID;
	vars[1].type = SQL_TYPE_NONE;

	if (SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_DELETE_OUTBOX], NULL, vars, &res) != SQL_OK) {
		SMSD_Log(DEBUG_INFO, Config, "Error deleting from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	db->FreeResult(Config, &res);

	if (SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_DELETE_OUTBOX_MULTIPART], NULL, vars, &res) != SQL_OK) {
		SMSD_Log(DEBUG_INFO, Config, "Error deleting from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	db->FreeResult(Config, &res);

	return ERR_NONE;
}

/* Adds SMS to Outbox */
static GSM_Error SMSDSQL_CreateOutboxSMS(GSM_MultiSMSMessage * sms, GSM_SMSDConfig * Config, char *NewID)
{
	char creator[200];
	int i;
	unsigned int ID = 0;
	SQL_result res;
	SQL_Var vars[6];
	struct GSM_SMSDdbobj *db = Config->db;
	const char *report, *multipart, *q;

	sprintf(creator, "Gammu %s",GAMMU_VERSION); /* %1 */
	multipart = (sms->Number == 1) ? "FALSE" : "TRUE"; /* %3 */

	for (i = 0; i < sms->Number; i++) {
		report = (sms->SMS[i].PDU == SMS_Status_Report) ? "yes": "default"; /* %2 */
		if (i == 0) {
			q = Config->SMSDSQL_queries[SQL_QUERY_CREATE_OUTBOX];
		} else {
			q = Config->SMSDSQL_queries[SQL_QUERY_CREATE_OUTBOX_MULTIPART];
		}

		vars[0].type = SQL_TYPE_STRING;
		vars[0].v.s = creator;
		vars[1].type = SQL_TYPE_STRING;
		vars[1].v.s = report;
		vars[2].type = SQL_TYPE_STRING;
		vars[2].v.s = multipart;
		vars[3].type = SQL_TYPE_INT;
		vars[3].v.i = i+1;
		vars[4].type = SQL_TYPE_INT;
		vars[4].v.i = ID;
		vars[5].type = SQL_TYPE_NONE;

		if (SMSDSQL_NamedQuery(Config, q, &sms->SMS[i], vars, &res) != SQL_OK) {
			SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		if (i == 0) {
			ID = db->SeqID(Config, "outbox_ID_seq");
			if (ID == 0) {
				SMSD_Log(DEBUG_INFO, Config, "Failed to get inserted row ID (%s)", __FUNCTION__);
				return ERR_UNKNOWN;
			}
		}
		db->FreeResult(Config, &res);
	}
	SMSD_Log(DEBUG_INFO, Config, "Written message with ID %u", ID);
	if (NewID != NULL)
		sprintf(NewID, "%d", ID);
	return ERR_NONE;
}

static GSM_Error SMSDSQL_AddSentSMSInfo(GSM_MultiSMSMessage * sms, GSM_SMSDConfig * Config, char *ID, int Part, GSM_SMSDSendingError err, int TPMR)
{
	SQL_result res;
	struct GSM_SMSDdbobj *db = Config->db;

	const char *message_state;
	SQL_Var vars[6];
	char smsc[GSM_MAX_NUMBER_LENGTH + 1];
	char destination[GSM_MAX_NUMBER_LENGTH + 1];

	EncodeUTF8(smsc, sms->SMS[Part - 1].SMSC.Number);
	EncodeUTF8(destination, sms->SMS[Part - 1].Number);

	if (err == SMSD_SEND_OK) {
		SMSD_Log(DEBUG_NOTICE, Config, "Transmitted %s (%s: %i) to %s", Config->SMSID,
			 (Part == sms->Number ? "total" : "part"), Part, DecodeUnicodeString(sms->SMS[0].Number));
	}

	if (err == SMSD_SEND_OK) {
		if (sms->SMS[Part - 1].PDU == SMS_Status_Report) {
			message_state = "SendingOK";
		} else {
			message_state = "SendingOKNoReport";
		}
	} else if (err == SMSD_SEND_SENDING_ERROR) {
		message_state = "SendingError";
	} else if (err == SMSD_SEND_ERROR) {
		message_state = "Error";
	} else {
		SMSD_Log(DEBUG_INFO, Config, "Unknown SMS state: %d, assuming Error", err);
		message_state = "Error";
	}

	/* 1 = ID, 2 = SequencePosition, 3 = Status, 4 = TPMR, 5 = insertintodb */
	vars[0].type = SQL_TYPE_STRING;
	vars[0].v.s = ID;
	vars[1].type = SQL_TYPE_INT;
	vars[1].v.i = Part;
	vars[2].type = SQL_TYPE_STRING;
	vars[2].v.s = message_state;
	vars[3].type = SQL_TYPE_INT;
	vars[3].v.i = TPMR;
	vars[4].type = SQL_TYPE_STRING;
	vars[4].v.s = Config->DT;
	vars[5].type = SQL_TYPE_NONE;

	if (SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_ADD_SENT_INFO], &sms->SMS[Part - 1], vars, &res) != SQL_OK) {
		SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	db->FreeResult(Config, &res);

	if (SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_UPDATE_SENT], &sms->SMS[Part - 1], NULL, &res) != SQL_OK) {
		SMSD_Log(DEBUG_INFO, Config, "Error updating number of sent messages (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	db->FreeResult(Config, &res);

	return ERR_NONE;
}

static GSM_Error SMSDSQL_RefreshPhoneStatus(GSM_SMSDConfig * Config)
{
	SQL_result res;
	SQL_Var vars[3] = {
		{SQL_TYPE_INT, {NULL}},
		{SQL_TYPE_INT, {NULL}},
		{SQL_TYPE_NONE, {NULL}}};
	struct GSM_SMSDdbobj *db = Config->db;
	vars[0].v.i = Config->Status->Charge.BatteryPercent;
	vars[1].v.i = Config->Status->Network.SignalPercent;

	if (SMSDSQL_NamedQuery(Config, Config->SMSDSQL_queries[SQL_QUERY_REFRESH_PHONE_STATUS], NULL, vars, &res) != SQL_OK) {
		SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	db->FreeResult(Config, &res);

	return ERR_NONE;
}

/*
 * better strcat... shows where is the bug
 */
#define STRCAT_MAX 80
GSM_Error SMSDSQL_option(GSM_SMSDConfig *Config, int optint, const char *option, ...)
{
	size_t len[STRCAT_MAX], to_alloc = 0;
	int i, j;
	va_list ap;
	const char *arg;
	const char *args[STRCAT_MAX];
	char *buffer, *ptr;

	/* read from config */
	buffer = INI_GetValue(Config->smsdcfgfile, "sql", option, FALSE);
	/* found? */
	if (buffer != NULL){
		Config->SMSDSQL_queries[optint] = strdup(buffer); /* avoid to double free */
		return ERR_NONE;
	}

	/* not found.. we use default query */
	va_start(ap, option);
	for(i = 0; i < STRCAT_MAX; i++){
		arg = va_arg(ap, const char *);
		if (arg == NULL)
			break;
		len[i] = strlen(arg);
		args[i] = arg;
		to_alloc += len[i];
	}
	va_end(ap);

	if (i == STRCAT_MAX) {
		SMSD_Log(DEBUG_ERROR, Config, "STRCAT_MAX too small.. consider increase this value for option %s", option);
		return ERR_UNKNOWN;
	}

	buffer = malloc(to_alloc+1);
	if (buffer == NULL){
		SMSD_Log(DEBUG_ERROR, Config, "Insufficient memory problem for option %s", option);
		return ERR_UNKNOWN;
	}
	ptr = buffer;
	for (j = 0; j < i; j++) {
		memcpy(ptr, args[j], len[j]);
		ptr += len[j];
	}
	*ptr = '\0';
	Config->SMSDSQL_queries[optint] = buffer;
	return ERR_NONE;
}


/**
 * Reads common options for database backends.
 */
GSM_Error SMSDSQL_ReadConfiguration(GSM_SMSDConfig *Config)
{
	int locktime;
	const char *escape_char;

	Config->user = INI_GetValue(Config->smsdcfgfile, "smsd", "user", FALSE);
	if (Config->user == NULL) {
		Config->user="root";
	}

	Config->password = INI_GetValue(Config->smsdcfgfile, "smsd", "password", FALSE);
	if (Config->password == NULL) {
		Config->password="";
	}

	Config->host = INI_GetValue(Config->smsdcfgfile, "smsd", "host", FALSE);
	if (Config->host == NULL) {
		/* Backward compatibility */
		Config->host = INI_GetValue(Config->smsdcfgfile, "smsd", "pc", FALSE);
	}
	if (Config->host == NULL) {
		Config->host="localhost";
	}

	Config->database = INI_GetValue(Config->smsdcfgfile, "smsd", "database", FALSE);
	if (Config->database == NULL) {
		Config->database="sms";
	}

	Config->driverspath = INI_GetValue(Config->smsdcfgfile, "smsd", "driverspath", FALSE);

	Config->sql = INI_GetValue(Config->smsdcfgfile, "smsd", "sql", FALSE);

	Config->dbdir = INI_GetValue(Config->smsdcfgfile, "smsd", "dbdir", FALSE);

	if (Config->driver == NULL) {
		SMSD_Log(DEBUG_ERROR, Config, "No database driver selected. Must be native_mysql, native_pgsql, ODBC or DBI one.");
		return ERR_UNKNOWN;
	}

	Config->db = NULL;
#ifdef HAVE_MYSQL_MYSQL_H
	if (!strcasecmp(Config->driver, "native_mysql")) {
		Config->db = &SMSDMySQL;
	}
#endif
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	if (!strcasecmp(Config->driver, "native_pgsql")) {
		Config->db = &SMSDPgSQL;
	}
#endif
#ifdef ODBC_FOUND
	if (!strcasecmp(Config->driver, "odbc")) {
		Config->db = &SMSDODBC;
		if (Config->sql == NULL) {
			SMSD_Log(DEBUG_INFO, Config, "Using generic SQL for ODBC, this might fail. In such case please set SQL configuration option.");
		}
	}
#endif
	if (Config->db == NULL) {
#ifdef LIBDBI_FOUND
		Config->db = &SMSDDBI;
#else
		SMSD_Log(DEBUG_ERROR, Config, "Unknown DB driver");
		return ERR_UNKNOWN;
#endif
	}

	escape_char = SMSDSQL_EscapeChar(Config);
#define ESCAPE_FIELD(x) escape_char, x, escape_char

	locktime = Config->loopsleep * 8; /* reserve 8 sec per message */
	locktime = locktime < 60 ? 60 : locktime; /* Minimum time reserve is 60 sec */

	if (SMSDSQL_option(Config, SQL_QUERY_DELETE_PHONE, "delete_phone",
		"DELETE FROM phones WHERE ", ESCAPE_FIELD("IMEI"), " = %I", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_INSERT_PHONE, "insert_phone",
		"INSERT INTO phones (",
			ESCAPE_FIELD("IMEI"),
			", ", ESCAPE_FIELD("ID"),
			", ", ESCAPE_FIELD("NetCode"),
			", ", ESCAPE_FIELD("NetName"),
			", ", ESCAPE_FIELD("Send"),
			", ", ESCAPE_FIELD("Receive"),
			", ", ESCAPE_FIELD("InsertIntoDB"),
			", ", ESCAPE_FIELD("TimeOut"),
			", ", ESCAPE_FIELD("Client"),
			", ", ESCAPE_FIELD("Battery"),
			", ", ESCAPE_FIELD("Signal"),
			") VALUES (%I, %P, %O, %M, %1, %2, ",
			SMSDSQL_Now(Config),
			", ",
			SMSDSQL_NowPlus(Config, 10),
			", %N, -1, -1)", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_SAVE_INBOX_SMS_SELECT, "save_inbox_sms_select",
		"SELECT ",
			ESCAPE_FIELD("ID"),
			", ", ESCAPE_FIELD("Status"),
			", ", ESCAPE_FIELD("SendingDateTime"),
			", ", ESCAPE_FIELD("DeliveryDateTime"),
			", ", ESCAPE_FIELD("SMSCNumber"), " "
			"FROM sentitems WHERE ",
			ESCAPE_FIELD("DeliveryDateTime"), " IS NULL AND ",
			ESCAPE_FIELD("SenderID"), " = %P AND ",
			ESCAPE_FIELD("TPMR"), " = %t AND ",
			ESCAPE_FIELD("DestinationNumber"), " = %R", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_SAVE_INBOX_SMS_UPDATE_DELIVERED, "save_inbox_sms_update_delivered",
		"UPDATE sentitems "
			"SET ", ESCAPE_FIELD("DeliveryDateTime"), " = %C"
			", ", ESCAPE_FIELD("Status"), " = %1"
			", ", ESCAPE_FIELD("StatusError"), " = %e"
			" WHERE ", ESCAPE_FIELD("ID"), " = %2"
			" AND ", ESCAPE_FIELD("TPMR"), " = %t", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_SAVE_INBOX_SMS_UPDATE, "save_inbox_sms_update",
		"UPDATE sentitems "
			"SET ",	ESCAPE_FIELD("Status"), " = %1"
			", ", ESCAPE_FIELD("StatusError"), " = %e"
			" WHERE ", ESCAPE_FIELD("ID"), " = %2"
			" AND ", ESCAPE_FIELD("TPMR"), " = %t", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_SAVE_INBOX_SMS_INSERT, "save_inbox_sms_insert",
		"INSERT INTO inbox "
			"(", ESCAPE_FIELD("ReceivingDateTime"),
			", ", ESCAPE_FIELD("Text"),
			", ", ESCAPE_FIELD("SenderNumber"),
			", ", ESCAPE_FIELD("Coding"),
			", ", ESCAPE_FIELD("SMSCNumber"),
			", ", ESCAPE_FIELD("UDH"),
			", ", ESCAPE_FIELD("Class"),
			", ", ESCAPE_FIELD("TextDecoded"),
			", ", ESCAPE_FIELD("RecipientID"), ")"
			" VALUES (%d, %E, %R, %c, %F, %u, %x, %T, %P)", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_UPDATE_RECEIVED, "update_received",
		"UPDATE phones SET ",
			ESCAPE_FIELD("Received"), " = ", ESCAPE_FIELD("Received"), " + 1"
			" WHERE ", ESCAPE_FIELD("IMEI"), " = %I", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_REFRESH_SEND_STATUS, "refresh_send_status",
		"UPDATE outbox SET ",
			ESCAPE_FIELD("SendingTimeOut"), " = ", SMSDSQL_NowPlus(Config, locktime),
			" WHERE ", ESCAPE_FIELD("ID"), " = %1"
			" AND (", ESCAPE_FIELD("SendingTimeOut"), " < ", SMSDSQL_Now(Config),
			" OR ", ESCAPE_FIELD("SendingTimeOut"), " IS NULL)", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_FIND_OUTBOX_SMS_ID, "find_outbox_sms_id",
		"SELECT ", SMSDSQL_TopClause(Config, "%1"),
			ESCAPE_FIELD("ID"),
			", ", ESCAPE_FIELD("InsertIntoDB"),
			", ", ESCAPE_FIELD("SendingDateTime"),
			", ", ESCAPE_FIELD("SenderID"),
			" FROM outbox WHERE ",
			ESCAPE_FIELD("SendingDateTime"), " < ", SMSDSQL_Now(Config),
			" AND ", ESCAPE_FIELD("SendingTimeOut"), " < ", SMSDSQL_Now(Config),
			" AND ", ESCAPE_FIELD("SendBefore"), " >= ", SMSDSQL_CurrentTime(Config),
			" AND ", ESCAPE_FIELD("SendAfter"), " <= ", SMSDSQL_CurrentTime(Config),
			" AND ( ", ESCAPE_FIELD("SenderID"), " is NULL OR ", ESCAPE_FIELD("SenderID"), " = '' OR ", ESCAPE_FIELD("SenderID"), " = %P )"
			" ORDER BY ", ESCAPE_FIELD("InsertIntoDB"), " ASC ", SMSDSQL_LimitClause(Config, "%1"), NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_FIND_OUTBOX_BODY, "find_outbox_body",
		"SELECT ",
			ESCAPE_FIELD("Text"),
			", ", ESCAPE_FIELD("Coding"),
			", ", ESCAPE_FIELD("UDH"),
			", ", ESCAPE_FIELD("Class"),
			", ", ESCAPE_FIELD("TextDecoded"),
			", ", ESCAPE_FIELD("ID"),
			", ", ESCAPE_FIELD("DestinationNumber"),
			", ", ESCAPE_FIELD("MultiPart"),
			", ", ESCAPE_FIELD("RelativeValidity"),
			", ", ESCAPE_FIELD("DeliveryReport"),
			", ", ESCAPE_FIELD("CreatorID"),
			" FROM outbox WHERE ",
			ESCAPE_FIELD("ID"), "=%1", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_FIND_OUTBOX_MULTIPART, "find_outbox_multipart",
		"SELECT ",
			ESCAPE_FIELD("Text"),
			", ", ESCAPE_FIELD("Coding"),
			", ", ESCAPE_FIELD("UDH"),
			", ", ESCAPE_FIELD("Class"),
			", ", ESCAPE_FIELD("TextDecoded"),
			", ", ESCAPE_FIELD("ID"),
			", ", ESCAPE_FIELD("SequencePosition"),
			" FROM outbox_multipart WHERE ",
			ESCAPE_FIELD("ID"), "=%1 AND ",
			ESCAPE_FIELD("SequencePosition"), "=%2", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_DELETE_OUTBOX, "delete_outbox",
		"DELETE FROM outbox WHERE ", ESCAPE_FIELD("ID"), "=%1", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_DELETE_OUTBOX_MULTIPART, "delete_outbox_multipart",
		"DELETE FROM outbox_multipart WHERE ", ESCAPE_FIELD("ID"), "=%1", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_CREATE_OUTBOX, "create_outbox",
		"INSERT INTO outbox "
			"(", ESCAPE_FIELD("CreatorID"),
			", ", ESCAPE_FIELD("SenderID"),
			", ", ESCAPE_FIELD("DeliveryReport"),
			", ", ESCAPE_FIELD("MultiPart"),
			", ", ESCAPE_FIELD("InsertIntoDB"),
			", ", ESCAPE_FIELD("Text"),
			", ", ESCAPE_FIELD("DestinationNumber"),
			", ", ESCAPE_FIELD("RelativeValidity"),
			", ", ESCAPE_FIELD("Coding"),
			", ", ESCAPE_FIELD("UDH"),
			", ", ESCAPE_FIELD("Class"),
			", ", ESCAPE_FIELD("TextDecoded"), ") VALUES "
			"(%1, %P, %2, %3, ", SMSDSQL_Now(Config),
			", %E, %R, %V, %c, %u, %x, %T)", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_CREATE_OUTBOX_MULTIPART, "create_outbox_multipart",
		"INSERT INTO outbox_multipart "
			"(", ESCAPE_FIELD("SequencePosition"),
			", ", ESCAPE_FIELD("Text"),
			", ", ESCAPE_FIELD("Coding"),
			", ", ESCAPE_FIELD("UDH"),
			", ", ESCAPE_FIELD("Class"),
			", ", ESCAPE_FIELD("TextDecoded"),
			", ", ESCAPE_FIELD("ID"), ") VALUES (%4, %E, %c, %u, %x, %T, %5)", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_ADD_SENT_INFO, "add_sent_info",
		"INSERT INTO sentitems "
			"(", ESCAPE_FIELD("CreatorID"),
			", ", ESCAPE_FIELD("ID"),
			", ", ESCAPE_FIELD("SequencePosition"),
			", ", ESCAPE_FIELD("Status"),
			", ", ESCAPE_FIELD("SendingDateTime"),
			", ", ESCAPE_FIELD("SMSCNumber"),
			", ", ESCAPE_FIELD("TPMR"),
			", ", ESCAPE_FIELD("SenderID"),
			", ", ESCAPE_FIELD("Text"),
			", ", ESCAPE_FIELD("DestinationNumber"),
			", ", ESCAPE_FIELD("Coding"),
			", ", ESCAPE_FIELD("UDH"),
			", ", ESCAPE_FIELD("Class"),
			", ", ESCAPE_FIELD("TextDecoded"),
			", ", ESCAPE_FIELD("InsertIntoDB"),
			", ", ESCAPE_FIELD("RelativeValidity"),
			") "
			" VALUES (%A, %1, %2, %3, ",
			SMSDSQL_Now(Config),
			", %F, %4, %P, %E, %R, %c, %u, %x, %T, %5, %V)", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_UPDATE_SENT, "update_sent",
		"UPDATE phones SET ",
			ESCAPE_FIELD("Sent"), "= ", ESCAPE_FIELD("Sent"), " + 1"
			" WHERE ", ESCAPE_FIELD("IMEI"), " = %I", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}

	if (SMSDSQL_option(Config, SQL_QUERY_REFRESH_PHONE_STATUS, "refresh_phone_status",
		"UPDATE phones SET ",
			ESCAPE_FIELD("TimeOut"), "= ", SMSDSQL_NowPlus(Config, 10),
			", ", ESCAPE_FIELD("Battery"), " = %1"
			", ", ESCAPE_FIELD("Signal"), " = %2"
			", ", ESCAPE_FIELD("NetCode"), " = %O"
			", ", ESCAPE_FIELD("NetName"), " = %M"
			" WHERE ", ESCAPE_FIELD("IMEI"), " = %I", NULL) != ERR_NONE) {
		return ERR_UNKNOWN;
	}
#undef ESCAPE_FIELD

	return ERR_NONE;
}

time_t SMSDSQL_ParseDate(GSM_SMSDConfig * Config, const char *date)
{
	char *parse_res;
	struct tm timestruct;
	GSM_DateTime DT;

	if (strcmp(date, "0000-00-00 00:00:00") == 0) {
		return -2;
	}

	parse_res = strptime(date, "%Y-%m-%d %H:%M:%S", &timestruct);

	if (parse_res != NULL && *parse_res == 0) {
		DT.Year = timestruct.tm_year + 1900;
		DT.Month = timestruct.tm_mon + 1;
		DT.Day = timestruct.tm_mday;
		DT.Hour = timestruct.tm_hour;
		DT.Minute = timestruct.tm_min;
		DT.Second = timestruct.tm_sec;

		return Fill_Time_T(DT);
	}
	/* Used during testing */
	if (Config != NULL) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to parse date: %s", date);
	}
	return -1;
}

GSM_SMSDService SMSDSQL = {
	SMSDSQL_Init,
	SMSDSQL_Free,
	SMSDSQL_InitAfterConnect,
	SMSDSQL_SaveInboxSMS,
	SMSDSQL_FindOutboxSMS,
	SMSDSQL_MoveSMS,
	SMSDSQL_CreateOutboxSMS,
	SMSDSQL_AddSentSMSInfo,
	SMSDSQL_RefreshSendStatus,
	SMSDSQL_RefreshPhoneStatus,
	SMSDSQL_ReadConfiguration
};

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
