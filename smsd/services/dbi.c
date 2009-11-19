/**
 * libdbi database backend
 *
 * Part of Gammu project
 *
 * Copyright (C) 2009 Michal Čihař
 *
 * Licensed under GNU GPL version 2 or later
 */

#define _XOPEN_SOURCE
#define _BSD_SOURCE
#include "../../helper/strptime.h"
#include <time.h>
#include <gammu.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#ifdef WIN32
#  include <windows.h>
#ifndef __GNUC__
#  pragma comment(lib, "libdbi.lib")
#endif
#endif

#include <dbi/dbi.h>

#include "../core.h"

const char now_plus_mysql[] = "(NOW() + INTERVAL %d SECOND) + 0";
const char now_plus_pgsql[] = "now() + interval '%d seconds'";
const char now_plus_sqlite[] = "datetime('now', '+%d seconds')";
const char now_plus_freetds[] = "DATEADD('second', %d, CURRENT_TIMESTAMP)";
const char now_plus_fallback[] = "NOW() + INTERVAL %d SECOND";

long long SMSDDBI_GetNumber(GSM_SMSDConfig * Config, dbi_result res, const char *field)
{
	unsigned int type;

	type = dbi_result_get_field_type(res, field);

	switch (type) {
		case DBI_TYPE_INTEGER:
			type = dbi_result_get_field_attribs(res, field);
			if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE1) {
				return dbi_result_get_int(res, field);
			} else if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE2) {
				return dbi_result_get_int(res, field);
			} else if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE3) {
				return dbi_result_get_int(res, field);
			} else if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE4) {
				return dbi_result_get_int(res, field);
			} else if ((type & DBI_INTEGER_SIZEMASK) == DBI_INTEGER_SIZE8) {
				return dbi_result_get_longlong(res, field);
			}
			SMSD_Log(DEBUG_ERROR, Config, "Wrong integer field subtype! (Field = %s, type = %d)", field, type);
			return -1;
		case DBI_TYPE_DECIMAL:
			type = dbi_result_get_field_attribs(res, field);
			if ((type & DBI_DECIMAL_SIZEMASK) == DBI_DECIMAL_SIZE4) {
				return dbi_result_get_int(res, field);
			} else if ((type & DBI_DECIMAL_SIZEMASK) == DBI_DECIMAL_SIZE8) {
				return dbi_result_get_longlong(res, field);
			}
			SMSD_Log(DEBUG_ERROR, Config, "Wrong decimal field subtype! (Field = %s, type = %d)", field, type);
			return -1;
		case DBI_TYPE_ERROR:
		default:
			SMSD_Log(DEBUG_ERROR, Config, "Wrong field type! (Field = %s, type = %d)", field, type);
			return -1;
	}
}

time_t SMSDDBI_GetDate(GSM_SMSDConfig * Config, dbi_result res, const char *field)
{
	unsigned int type;
	struct tm timestruct;
	char *parse_res;
	const char *date;

	type = dbi_result_get_field_type(res, field);

	switch (type) {
		case DBI_TYPE_INTEGER:
		case DBI_TYPE_DECIMAL:
			return SMSDDBI_GetNumber(Config, res, field);
		case DBI_TYPE_STRING:
			date = dbi_result_get_string(res, field);
			SMSD_Log(DEBUG_ERROR, Config, "Got date as %s (Field = %s, type = %d)", date, field, type);
			parse_res = strptime(date, "%Y-%m-%d %H:%M:%S", &timestruct);
			timestruct.tm_isdst = 0;
#ifdef HAVE_STRUCT_TM_TM_ZONE
			timestruct.tm_gmtoff = 0;
			timestruct.tm_zone = NULL;
#endif
			if (parse_res != NULL && *parse_res == 0) {
				return mktime(&timestruct);
			}
			SMSD_Log(DEBUG_ERROR, Config, "DBI error: 9999 Failed to process date %s (Field = %s, type = %d)", date, field, type);
			return -1;
		case DBI_TYPE_DATETIME:
			return dbi_result_get_datetime(res, field);
		case DBI_TYPE_ERROR:
		default:
			SMSD_Log(DEBUG_ERROR, Config, "Wrong date field type! (Field = %s, type = %d)", field, type);
			return -1;
	}
}

gboolean SMSDDBI_GetBool(GSM_SMSDConfig * Config, dbi_result res, const char *field)
{
	unsigned int type;
	const char *value;
	int num;

	type = dbi_result_get_field_type(res, field);

	switch (type) {
		case DBI_TYPE_INTEGER:
		case DBI_TYPE_DECIMAL:
			num = SMSDDBI_GetNumber(Config, res, field);
			SMSD_Log(DEBUG_ERROR, Config, "Got gboolean as %d (Field = %s, type = %d)", num, field, type);
			if (num == -1) {
				return -1;
			} else if (num == 0) {
				return FALSE;
			} else {
				return TRUE;
			}
		case DBI_TYPE_STRING:
			value= dbi_result_get_string(res, field);
			SMSD_Log(DEBUG_ERROR, Config, "Got gboolean as %s (Field = %s, type = %d)", value, field, type);
			if (strcasecmp(value, "yes") == 0 || strcasecmp(value, "TRUE") == 0 ||
					strcasecmp(value, "y") == 0 || strcasecmp(value, "t") == 0) {
				return TRUE;
			}
			if (strcasecmp(value, "no") == 0 || strcasecmp(value, "FALSE") == 0 ||
					strcasecmp(value, "n") == 0 || strcasecmp(value, "f") == 0) {
				return TRUE;
			}
			if (strcasecmp(value, "default") == 0) {
				return -1;
			}
			SMSD_Log(DEBUG_ERROR, Config, "DBI error: 9999 Failed to process gboolean %s (Field = %s, type = %d)", value, field, type);
			return -1;
		case DBI_TYPE_ERROR:
		default:
			SMSD_Log(DEBUG_ERROR, Config, "Wrong gboolean field type! (Field = %s, type = %d)", field, type);
			return -1;
	}
}

static const char * SMSDDBI_NowPlus(GSM_SMSDConfig * Config, int seconds)
{
	const char *driver_name;
	static char result[100];

	driver_name = dbi_driver_get_name(dbi_conn_get_driver(Config->DBConnDBI));

	if (strcmp(driver_name, "mysql") == 0) {
		sprintf(result, now_plus_mysql, seconds);
	} else if (strcmp(driver_name, "pgsql") == 0) {
		sprintf(result, now_plus_pgsql, seconds);
	} else if (strncmp(driver_name, "sqlite", 6) == 0) {
		sprintf(result, now_plus_sqlite, seconds);
	} else if (strcmp(driver_name, "freetds") == 0) {
		sprintf(result, now_plus_freetds, seconds);
	} else {
		sprintf(result, now_plus_fallback, seconds);
	}
	return result;
}

const char now_mysql[] = "NOW()";
const char now_pgsql[] = "now()";
const char now_sqlite[] = "datetime('now')";
const char now_freetds[] = "CURRENT_TIMESTAMP";
const char now_fallback[] = "NOW()";

static const char * SMSDDBI_Now(GSM_SMSDConfig * Config)
{
	const char *driver_name;

	driver_name = dbi_driver_get_name(dbi_conn_get_driver(Config->DBConnDBI));

	if (strcmp(driver_name, "mysql") == 0) {
		return now_mysql;
	} else if (strcmp(driver_name, "pgsql") == 0) {
		return now_pgsql;
	} else if (strncmp(driver_name, "sqlite", 6) == 0) {
		return now_sqlite;
	} else if (strcmp(driver_name, "freetds") == 0) {
		return now_freetds;
	} else {
		return now_fallback;
	}
}

static void SMSDDBI_LogError(GSM_SMSDConfig * Config)
{
	int rc;
	const char *msg;
	rc = dbi_conn_error(Config->DBConnDBI, &msg);
	if (rc == -1) {
		SMSD_Log(DEBUG_ERROR, Config, "Unknown DBI error!");
	} else {
		SMSD_Log(DEBUG_ERROR, Config, "DBI error %d: %s", rc, msg);
	}
}

void SMSDDBI_Callback(dbi_conn Conn, void *Config)
{
	SMSDDBI_LogError((GSM_SMSDConfig *)Config);
}

/* Disconnects from a database */
static GSM_Error SMSDDBI_Free(GSM_SMSDConfig *Config)
{
	if (Config->DBConnDBI != NULL) {
		SMSD_Log(DEBUG_NOTICE, Config, "Disconnecting from DBI");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		Config->DBConnDBI = NULL;
	}
	return ERR_NONE;
}

/* Connects to database */
static GSM_Error SMSDDBI_Connect(GSM_SMSDConfig * Config)
{
	int rc;

	rc = dbi_initialize(Config->driverspath);

	if (rc == 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI did not find any drivers, try using DriversPath option");
		dbi_shutdown();
		return ERR_UNKNOWN;
	} else if (rc < 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to initialize!");
		return ERR_UNKNOWN;
	}

	Config->DBConnDBI = dbi_conn_new(Config->driver);
	if (Config->DBConnDBI == NULL) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to init %s driver!", Config->driver);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}

	dbi_conn_error_handler(Config->DBConnDBI, SMSDDBI_Callback, Config);

	if (dbi_conn_set_option(Config->DBConnDBI, "sqlite_dbdir", Config->dbdir) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set sqlite_dbdir!");
		SMSDDBI_Free(Config);
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "sqlite3_dbdir", Config->dbdir) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set sqlite3_dbdir!");
		SMSDDBI_Free(Config);
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "host", Config->PC) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set host!");
		SMSDDBI_Free(Config);
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "username", Config->user) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set username!");
		SMSDDBI_Free(Config);
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "password", Config->password) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set password!");
		SMSDDBI_Free(Config);
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "dbname", Config->database) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set dbname!");
		SMSDDBI_Free(Config);
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "encoding", "UTF-8") != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to set encoding!");
		SMSDDBI_Free(Config);
		return ERR_UNKNOWN;
	}

	if (dbi_conn_connect(Config->DBConnDBI) != 0) {
		SMSD_Log(DEBUG_ERROR, Config, "DBI failed to connect!");
		SMSDDBI_Free(Config);
		return ERR_UNKNOWN;
	}
	return ERR_NONE;
}

static GSM_Error SMSDDBI_Query(GSM_SMSDConfig * Config, const char *query, dbi_result *res)
{
	const char *msg;
	int rc;
	GSM_Error error;
	int attempts = 1;

	*res = NULL;

	for (attempts = 1; attempts < SMSD_SQL_RETRIES; attempts++) {
		SMSD_Log(DEBUG_SQL, Config, "Execute SQL: %s", query);
		*res = dbi_conn_query(Config->DBConnDBI, query);
		if (*res != NULL) return ERR_NONE;

		SMSD_Log(DEBUG_INFO, Config, "SQL failed: %s", query);
		if (attempts >= SMSD_SQL_RETRIES) {
			return ERR_TIMEOUT;
		}
		/* Black magic to decide whether we should bail out or attempt to retry */
		rc = dbi_conn_error(Config->DBConnDBI, &msg);
		if (rc != -1) {
			SMSD_Log(DEBUG_INFO, Config, "SQL failure: %s", msg);
			if (strstr(msg, "syntax") != NULL) {
				return ERR_BUG;
			}
			if (strstr(msg, "violation") != NULL) {
				return ERR_BUG;
			}
			if (strstr(msg, "violates") != NULL) {
				return ERR_BUG;
			}
			if (strstr(msg, "SQL error") != NULL) {
				return ERR_BUG;
			}
			if (strstr(msg, "duplicate") != NULL) {
				return ERR_BUG;
			}
			if (strstr(msg, "unique") != NULL) {
				return ERR_BUG;
			}
			if (strstr(msg, "need to rewrite") != NULL) {
				return ERR_BUG;
			}
			if (strstr(msg, "locked") != NULL) {
				SMSD_Log(DEBUG_INFO, Config, "Retrying after %d seconds...", attempts * attempts);
				sleep(attempts * attempts);
				continue;
			}
		}
		/* We will try to reconnect */
		SMSD_Log(DEBUG_INFO, Config, "Failed to detect problem cause, reconnecting to database!");
		error = ERR_UNKNOWN;
		while (error != ERR_NONE) {
			SMSD_Log(DEBUG_INFO, Config, "Reconnecting after %d seconds...", attempts * attempts);
			sleep(attempts * attempts);
			SMSDDBI_Free(Config);
			error = SMSDDBI_Connect(Config);
			if (error == ERR_NONE) {
				break;
			}
			attempts++;
			if (attempts >= SMSD_SQL_RETRIES) {
				return ERR_TIMEOUT;
			}
		}
	}
	return ERR_TIMEOUT;
}

static GSM_Error SMSDDBI_CheckTable(GSM_SMSDConfig * Config, const char *table)
{
	dbi_result res;
	char buffer[200];
	GSM_Error error;

	sprintf(buffer, "SELECT id FROM %s LIMIT 1", table);
	error = SMSDDBI_Query(Config, buffer, &res);
	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_ERROR, Config, "Table %s not found!", table);
		SMSDDBI_Free(Config);
		return ERR_UNKNOWN;
	}
	dbi_result_free(res);
	return ERR_NONE;
}

/* Connects to database */
static GSM_Error SMSDDBI_Init(GSM_SMSDConfig * Config)
{
	dbi_result res;
	int version;
	GSM_Error error;

	error = SMSDDBI_Connect(Config);
	if (error != ERR_NONE) return error;

	error = SMSDDBI_CheckTable(Config, "outbox");
	if (error != ERR_NONE) return error;
	error = SMSDDBI_CheckTable(Config, "outbox_multipart");
	if (error != ERR_NONE) return error;
	error = SMSDDBI_CheckTable(Config, "sentitems");
	if (error != ERR_NONE) return error;
	error = SMSDDBI_CheckTable(Config, "inbox");
	if (error != ERR_NONE) return error;

	if (SMSDDBI_Query(Config, "SELECT Version FROM gammu", &res) != ERR_NONE) {
		SMSDDBI_Free(Config);
		return ERR_UNKNOWN;
	}
	if (dbi_result_get_numrows(res) != 1) {
		SMSD_Log(DEBUG_ERROR, Config, "No Version information in table gammu!");
		dbi_result_free(res);
		SMSDDBI_Free(Config);
		return ERR_UNKNOWN;
	}
	if (dbi_result_first_row(res) != 1) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to seek to first row!");
		dbi_result_free(res);
		SMSDDBI_Free(Config);
		return ERR_UNKNOWN;
	}
	version = SMSDDBI_GetNumber(Config, res, "Version");
	dbi_result_free(res);
	if (SMSD_CheckDBVersion(Config, version) != ERR_NONE) {
		SMSDDBI_Free(Config);
		return ERR_UNKNOWN;
	}

	SMSD_Log(DEBUG_INFO, Config, "Connected to Database %s: %s on %s", Config->driver, Config->database, Config->PC);

	return ERR_NONE;
}


static GSM_Error SMSDDBI_InitAfterConnect(GSM_SMSDConfig * Config)
{
	char buf[600];

	dbi_result Res;

	sprintf(buf, "DELETE FROM phones WHERE IMEI = '%s'", Config->Status->IMEI);

	if (SMSDDBI_Query(Config, buf, &Res) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error deleting from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	dbi_result_free(Res);

	sprintf(buf,
		"INSERT INTO phones (IMEI, ID, Send, Receive, InsertIntoDB, TimeOut, Client, Battery, Signal) VALUES ('%s', '%s', 'yes', 'yes', %s, %s, '%s', -1, -1)",
		Config->Status->IMEI, Config->PhoneID,
		SMSDDBI_Now(Config),
		SMSDDBI_NowPlus(Config, 10),
		Config->Status->Client);

	if (SMSDDBI_Query(Config, buf, &Res) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error inserting into database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	dbi_result_free(Res);

	return ERR_NONE;
}

/* Save SMS from phone (called Inbox sms - it's in phone Inbox) somewhere */
static GSM_Error SMSDDBI_SaveInboxSMS(GSM_MultiSMSMessage *sms,
					GSM_SMSDConfig * Config,
					char **Locations
					)
{
	dbi_result Res = NULL;
	char *encoded_text;

	char buffer[10000];
	char destinationnumber[GSM_MAX_NUMBER_LENGTH + 1];
	char smstext[3 *  GSM_MAX_SMS_LENGTH + 1];
	int i;
	time_t t_time1, t_time2;
	gboolean found;
	long diff;
	unsigned long long	new_id;
	size_t			locations_size = 0, locations_pos = 0;
	const char *state, *smsc;
	char smsc_message[GSM_MAX_NUMBER_LENGTH + 1];
	time_t timestamp;
	struct tm *timestruct;

	*Locations = NULL;

	for (i = 0; i < sms->Number; i++) {
		EncodeUTF8(smstext, sms->SMS[i].Text);
		EncodeUTF8(smsc_message, sms->SMS[i].SMSC.Number);
		EncodeUTF8(destinationnumber, sms->SMS[i].Number);
		if (sms->SMS[i].PDU == SMS_Status_Report) {
			SMSD_Log(DEBUG_INFO, Config, "Delivery report: %s to %s", smstext, destinationnumber);

			sprintf(buffer,
				"SELECT ID, Status, SendingDateTime, DeliveryDateTime, SMSCNumber "
                                        "FROM sentitems WHERE "
					"DeliveryDateTime IS NULL AND "
					"SenderID = '%s' AND TPMR = %i AND DestinationNumber = '%s'",
				Config->PhoneID, sms->SMS[i].MessageReference, destinationnumber);
			if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
				SMSD_Log(DEBUG_INFO, Config, "Error reading from database (%s)", __FUNCTION__);
				return ERR_UNKNOWN;
			}

			found = FALSE;
			while (dbi_result_next_row(Res)) {
				smsc = dbi_result_get_string(Res, "SMSCNumber");
				state = dbi_result_get_string(Res, "Status");
				SMSD_Log(DEBUG_NOTICE, Config, "Checking for delivery report, SMSC=%s, state=%s", smsc, state);

				if (strcmp(smsc, smsc_message) != 0) {
					if (Config->skipsmscnumber[0] == 0 ||
							strcmp(Config->skipsmscnumber, smsc)) {
						continue;
					}
				}

				if (strcmp(state, "SendingOK") == 0 || strcmp(state, "DeliveryPending") == 0) {
					t_time1 = SMSDDBI_GetDate(Config, Res, "SendingDateTime");
					t_time2 = Fill_Time_T(sms->SMS[i].DateTime);
					diff = t_time2 - t_time1;

					if (diff > -Config->deliveryreportdelay && diff < Config->deliveryreportdelay) {
						found = TRUE;
						break;
					} else {
						SMSD_Log(DEBUG_NOTICE, Config, "Delivery report would match, but time delta is too big (%ld), consider increasing DeliveryReportDelay", diff);
					}
				}
			}

			if (found) {
				strcpy(buffer, "UPDATE sentitems SET DeliveryDateTime = '");
				timestamp = Fill_Time_T(sms->SMS[i].SMSCTime);
				if (strcmp(dbi_driver_get_name(dbi_conn_get_driver(Config->DBConnDBI)), "pgsql") == 0) {
					timestruct = gmtime(&timestamp);
					strftime(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), "%Y-%m-%d %H:%M:%S GMT", timestruct);
				} else {
					sprintf(buffer + strlen(buffer), "%ld", (long)timestamp);
				}
				strcat(buffer, "', Status = '");

				if (!strcmp(smstext, "Delivered")) {
					strcat(buffer, "DeliveryOK");
				} else if (!strcmp(smstext, "Failed")) {
					strcat(buffer, "DeliveryFailed");
				} else if (!strcmp(smstext, "Pending")) {
					strcat(buffer, "DeliveryPending");
				} else if (!strcmp(smstext, "Unknown")) {
					strcat(buffer, "DeliveryUnknown");
				}

				sprintf(buffer + strlen(buffer),
					"', StatusError = %i WHERE ID = %lld AND `TPMR` = %i",
					sms->SMS[i].DeliveryStatus,
					SMSDDBI_GetNumber(Config, Res, "ID"),
					sms->SMS[i].MessageReference);

				if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
					SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
					return ERR_UNKNOWN;
				}
			}
			dbi_result_free(Res);
			continue;
		}

		if (sms->SMS[i].PDU != SMS_Deliver)
			continue;
		buffer[0] = 0;
		sprintf(buffer + strlen(buffer), "INSERT INTO inbox "
			"(ReceivingDateTime, Text, SenderNumber, Coding, SMSCNumber, UDH, "
			"Class, TextDecoded, RecipientID) VALUES ('%04d-%02d-%02d %02d:%02d:%02d', '", sms->SMS[i].DateTime.Year, sms->SMS[i].DateTime.Month, sms->SMS[i].DateTime.Day, sms->SMS[i].DateTime.Hour, sms->SMS[i].DateTime.Minute, sms->SMS[i].DateTime.Second);

		switch (sms->SMS[i].Coding) {
			case SMS_Coding_Unicode_No_Compression:

			case SMS_Coding_Default_No_Compression:
				EncodeHexUnicode(buffer + strlen(buffer),
						 sms->SMS[i].Text,
						 UnicodeLength(sms->SMS[i].
							       Text));
				break;

			case SMS_Coding_8bit:
				EncodeHexBin(buffer + strlen(buffer),
					     sms->SMS[i].Text,
					     sms->SMS[i].Length);

			default:
				break;
		}

		sprintf(buffer + strlen(buffer), "','%s','", destinationnumber);

		switch (sms->SMS[i].Coding) {
			case SMS_Coding_Unicode_No_Compression:
				strcat(buffer, "Unicode_No_Compression");
				break;

			case SMS_Coding_Unicode_Compression:
				strcat(buffer, "Unicode_Compression");
				break;

			case SMS_Coding_Default_No_Compression:
				strcat(buffer, "Default_No_Compression");
				break;

			case SMS_Coding_Default_Compression:
				strcat(buffer, "Default_Compression");
				break;

			case SMS_Coding_8bit:
				strcat(buffer, "8bit");
				break;
		}

		sprintf(buffer + strlen(buffer), "','%s'", smsc_message);

		strcat(buffer, ",'");
		if (sms->SMS[i].UDH.Type != UDH_NoUDH) {
			EncodeHexBin(buffer + strlen(buffer),
				     sms->SMS[i].UDH.Text,
				     sms->SMS[i].UDH.Length);
		}
		strcat(buffer, "'");

		sprintf(buffer + strlen(buffer), ",%i,", sms->SMS[i].Class);

		switch (sms->SMS[i].Coding) {

			case SMS_Coding_Unicode_No_Compression:

			case SMS_Coding_Default_No_Compression:
				dbi_conn_quote_string_copy(Config->DBConnDBI, smstext, &encoded_text);
				strcat(buffer, encoded_text);
				free(encoded_text);
				break;

			case SMS_Coding_8bit:
				break;

			default:
				break;
		}

		sprintf(buffer + strlen(buffer), ",'%s')", Config->PhoneID);
		if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
			SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}

		new_id = dbi_conn_sequence_last(Config->DBConnDBI, NULL);
		if (new_id == 0) {
			new_id = dbi_conn_sequence_last(Config->DBConnDBI, "inbox_id_seq");
		}
		if (new_id == 0) {
			SMSD_Log(DEBUG_INFO, Config, "Failed to get inserted row ID (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		SMSD_Log(DEBUG_NOTICE, Config, "Inserted message id %llu", new_id);

		dbi_result_free(Res);

		if (new_id != 0) {
			if (locations_pos + 10 >= locations_size) {
				locations_size += 40;
				*Locations = (char *)realloc(*Locations, locations_size);
				assert (*Locations != NULL);
				if (locations_pos == 0) {
					*Locations[0] = 0;
				}
			}
			locations_pos += sprintf((*Locations) + locations_pos, "%llu ", new_id);
		}

		sprintf(buffer, "UPDATE phones SET Received = Received + 1 WHERE IMEI = '%s'", Config->Status->IMEI);
		if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
			SMSD_Log(DEBUG_INFO, Config, "Error updating number of received messages (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		dbi_result_free(Res);

	}

	return ERR_NONE;
}

static GSM_Error SMSDDBI_RefreshSendStatus(GSM_SMSDConfig * Config,
					     char *ID)
{
	char buffer[10000];
	dbi_result Res;

	sprintf(buffer, "UPDATE outbox SET SendingTimeOut = %s "
                        "WHERE ID = '%s' AND (SendingTimeOut < %s OR SendingTimeOut IS NULL)",
		SMSDDBI_NowPlus(Config, 15),
		ID,
		SMSDDBI_Now(Config));
	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}

	if (dbi_result_get_numrows_affected(Res) == 0) {
		dbi_result_free(Res);
		return ERR_UNKNOWN;
	}

	dbi_result_free(Res);
	return ERR_NONE;
}

/* Find one multi SMS to sending and return it (or return ERR_EMPTY)
 * There is also set ID for SMS
 */
static GSM_Error SMSDDBI_FindOutboxSMS(GSM_MultiSMSMessage * sms,
					 GSM_SMSDConfig * Config,
					 char *ID)
{
	char buf[400];
	dbi_result Res;
	int i;
	gboolean found = FALSE;
	time_t timestamp;
	const char *coding;
	const char *text;
	size_t text_len;
	const char *text_decoded;
	const char *destination;
	const char *udh;
	size_t udh_len;

	char *encoded_text;

	struct tm *timestruct;

	dbi_conn_quote_string_copy(Config->DBConnDBI, Config->PhoneID, &encoded_text);

	sprintf(buf, "SELECT ID, InsertIntoDB FROM outbox "
                      "WHERE SendingDateTime < %s AND SendingTimeOut < %s"
		      " AND ( SenderID is NULL OR SenderID = '' OR SenderID = %s )",
		SMSDDBI_Now(Config),
		SMSDDBI_Now(Config),
		encoded_text
		);
	free(encoded_text);

	if (SMSDDBI_Query(Config, buf, &Res) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error reading from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}

	while (dbi_result_next_row(Res)) {
		timestamp = SMSDDBI_GetDate(Config, Res, "InsertIntoDB");
		sprintf(ID, "%lld", SMSDDBI_GetNumber(Config, Res, "ID"));
		if (strcmp(dbi_driver_get_name(dbi_conn_get_driver(Config->DBConnDBI)), "pgsql") == 0) {
			timestruct = gmtime(&timestamp);
			strftime(Config->DT, sizeof(Config->DT), "%Y-%m-%d %H:%M:%S GMT", timestruct);
		} else {
			sprintf(Config->DT, "%lld", (long long)timestamp);
		}

		if (SMSDDBI_RefreshSendStatus(Config, ID) == ERR_NONE) {
			found = TRUE;
			break;
		}
	}

	dbi_result_free(Res);

	if (!found) {
		return ERR_EMPTY;
	}

	sms->Number = 0;
	for (i = 0; i < GSM_MAX_MULTI_SMS; i++) {
		GSM_SetDefaultSMSData(&sms->SMS[i]);
		sms->SMS[i].SMSC.Number[0] = 0;
		sms->SMS[i].SMSC.Number[1] = 0;
	}

	for (i = 1; i < GSM_MAX_MULTI_SMS + 1; i++) {
		if (i == 1) {
			sprintf(buf,
				"SELECT Text, Coding, UDH, Class, TextDecoded, ID, DestinationNumber, MultiPart, "
				"RelativeValidity, DeliveryReport FROM outbox WHERE ID='%s'",
				ID);
		} else {
			sprintf(buf,
				"SELECT Text, Coding, UDH, Class, TextDecoded, ID, SequencePosition "
				"FROM outbox_multipart WHERE ID='%s' AND SequencePosition=%i",
				ID, i);
		}
		if (SMSDDBI_Query(Config, buf, &Res) != ERR_NONE) {
			SMSD_Log(DEBUG_INFO, Config, "Error reading from database (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}

		if (dbi_result_get_numrows(Res) == 0) {
			dbi_result_free(Res);
			return ERR_NONE;
		}
		dbi_result_first_row(Res);

		coding = dbi_result_get_string(Res, "Coding");
		text = dbi_result_get_string(Res, "Text");
		if (text == NULL) {
			text_len = 0;
		} else {
			text_len = strlen(text);
		}
		text_decoded = dbi_result_get_string(Res, "TextDecoded");
		udh = dbi_result_get_string(Res, "UDH");
		if (udh == NULL) {
			udh_len = 0;
		} else {
			udh_len = strlen(udh);
		}

		sms->SMS[sms->Number].Coding = SMS_Coding_8bit;
		if (strcmp(coding, "Unicode_No_Compression") == 0) {
			sms->SMS[sms->Number].Coding = SMS_Coding_Unicode_No_Compression;
		} else if (strcmp(coding, "Default_No_Compression") == 0) {
			sms->SMS[sms->Number].Coding = SMS_Coding_Default_No_Compression;
		}

		if (text == NULL || text_len == 0) {
			SMSD_Log(DEBUG_NOTICE, Config, "Message: %s", text_decoded);
			DecodeUTF8(sms->SMS[sms->Number].Text, text_decoded, strlen(text_decoded));
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
			destination = dbi_result_get_string(Res, "DestinationNumber");
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

		sms->SMS[sms->Number].Class = SMSDDBI_GetNumber(Config, Res, "Class");
		sms->SMS[sms->Number].PDU = SMS_Submit;
		sms->Number++;

		if (i == 1) {
			Config->relativevalidity = SMSDDBI_GetNumber(Config, Res, "RelativeValidity");

			Config->currdeliveryreport = SMSDDBI_GetBool(Config, Res, "DeliveryReport");

			if (!SMSDDBI_GetBool(Config, Res, "MultiPart")) {
				dbi_result_free(Res);
				break;
			}

		}
		dbi_result_free(Res);
	}

	return ERR_NONE;
}

/* After sending SMS is moved to Sent Items or Error Items. */
static GSM_Error SMSDDBI_MoveSMS(GSM_MultiSMSMessage * sms UNUSED,
				   GSM_SMSDConfig * Config, char *ID,
				   gboolean alwaysDelete UNUSED, gboolean sent UNUSED)
{
	char buffer[10000];
	dbi_result Res;

	sprintf(buffer, "DELETE FROM outbox WHERE ID = '%s'", ID);
	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error deleting from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	dbi_result_free(Res);

	sprintf(buffer, "DELETE FROM outbox_multipart WHERE ID = '%s'", ID);
	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error deleting from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	dbi_result_free(Res);

	return ERR_NONE;
}

/* Adds SMS to Outbox */
static GSM_Error SMSDDBI_CreateOutboxSMS(GSM_MultiSMSMessage * sms,
					   GSM_SMSDConfig * Config, char *NewID)
{
	char buffer[10000], buffer2[400];
	int i;
	unsigned int ID = 0;
	dbi_result Res;
	char *encoded_text;

	for (i = 0; i < sms->Number; i++) {
		buffer[0] = 0;
		if (i == 0) {
			sprintf(buffer + strlen(buffer),
				"INSERT INTO outbox (CreatorID, SenderID, DeliveryReport, MultiPart, InsertIntoDB");
		} else {
			sprintf(buffer + strlen(buffer),
				"INSERT INTO outbox_multipart (SequencePosition");
		}

		sprintf(buffer + strlen(buffer), ", Text, ");

		if (i == 0) {
			sprintf(buffer + strlen(buffer),
				"DestinationNumber, RelativeValidity, ");
		}

		strcat(buffer, "Coding, UDH, Class, TextDecoded");
		if (i != 0) {
			strcat(buffer, ", ID");
		}
		strcat(buffer, ") VALUES (");

		if (i == 0) {
			sprintf(buffer + strlen(buffer), "'Gammu %s', ",
				VERSION);
			sprintf(buffer + strlen(buffer), "'%s', '",
				Config->PhoneID);

			if (sms->SMS[i].PDU == SMS_Status_Report) {
				sprintf(buffer + strlen(buffer), "yes', '");
			} else {
				sprintf(buffer + strlen(buffer), "default', '");
			}

			if (sms->Number == 1) {
				sprintf(buffer + strlen(buffer), "FALSE");
			} else {
				sprintf(buffer + strlen(buffer), "TRUE");
			}

			sprintf(buffer + strlen(buffer), "', %s",
				SMSDDBI_Now(Config));
		} else {
			sprintf(buffer + strlen(buffer), "%i", i + 1);
		}
		sprintf(buffer + strlen(buffer), ", '");

		switch (sms->SMS[i].Coding) {
			case SMS_Coding_Unicode_No_Compression:

			case SMS_Coding_Default_No_Compression:
				EncodeHexUnicode(buffer + strlen(buffer),
						 sms->SMS[i].Text,
						 UnicodeLength(sms->SMS[i].
							       Text));
				break;

			case SMS_Coding_8bit:
				EncodeHexBin(buffer + strlen(buffer),
					     sms->SMS[i].Text,
					     sms->SMS[i].Length);

			default:
				break;
		}

		sprintf(buffer + strlen(buffer), "', ");
		if (i == 0) {
			strcat(buffer, "'");
			EncodeUTF8(buffer + strlen(buffer), sms->SMS[i].Number);
			strcat(buffer, "', ");

			if (sms->SMS[i].SMSC.Validity.Format ==
			    SMS_Validity_RelativeFormat) {
				sprintf(buffer + strlen(buffer), "%i, ",
					sms->SMS[i].SMSC.Validity.Relative);
			} else {
				strcat(buffer, "-1, ");
			}
		}

		strcat(buffer, "'");
		switch (sms->SMS[i].Coding) {
			case SMS_Coding_Unicode_No_Compression:
				sprintf(buffer + strlen(buffer),
					"Unicode_No_Compression");
				break;

			case SMS_Coding_Unicode_Compression:
				sprintf(buffer + strlen(buffer),
					"Unicode_Compression");
				break;

			case SMS_Coding_Default_No_Compression:
				sprintf(buffer + strlen(buffer),
					"Default_No_Compression");
				break;

			case SMS_Coding_Default_Compression:
				sprintf(buffer + strlen(buffer),
					"Default_Compression");
				break;

			case SMS_Coding_8bit:
				sprintf(buffer + strlen(buffer), "8bit");
				break;
		}

		strcat(buffer, "', '");

		if (sms->SMS[i].UDH.Type != UDH_NoUDH) {
			EncodeHexBin(buffer + strlen(buffer),
				     sms->SMS[i].UDH.Text,
				     sms->SMS[i].UDH.Length);
		}

		sprintf(buffer + strlen(buffer), "', %i, ",
			sms->SMS[i].Class);
		switch (sms->SMS[i].Coding) {
			case SMS_Coding_Unicode_No_Compression:

			case SMS_Coding_Default_No_Compression:
				EncodeUTF8(buffer2, sms->SMS[i].Text);
				dbi_conn_quote_string_copy(Config->DBConnDBI, buffer2, &encoded_text);
				strcat(buffer, encoded_text);
				free(encoded_text);
				break;

			default:
				break;
		}

		if (i != 0) {
			sprintf(buffer + strlen(buffer), ", %u", ID);
		}
		strcat(buffer, ")");
		if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
			SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		if (i == 0) {
			ID = dbi_conn_sequence_last(Config->DBConnDBI, NULL);
			if (ID == 0) {
				ID = dbi_conn_sequence_last(Config->DBConnDBI, "outbox_id_seq");
			}
			if (ID == 0) {
				SMSD_Log(DEBUG_INFO, Config, "Failed to get inserted row ID (%s)", __FUNCTION__);
				return ERR_UNKNOWN;
			}
		}
		dbi_result_free(Res);
	}
	SMSD_Log(DEBUG_INFO, Config, "Written message with ID %u", ID);
	if (NewID != NULL) sprintf(NewID, "%d", ID);
	return ERR_NONE;
}

static GSM_Error SMSDDBI_AddSentSMSInfo(GSM_MultiSMSMessage * sms,
					  GSM_SMSDConfig * Config,
					  char *ID, int Part,
					  GSM_SMSDSendingError err, int TPMR)
{
	dbi_result Res;

	char buffer[10000], buffer2[400];
	char *encoded_text;
	char message_state[50];
	char smsc[GSM_MAX_NUMBER_LENGTH + 1];
	char destination[GSM_MAX_NUMBER_LENGTH + 1];

	EncodeUTF8(smsc, sms->SMS[Part - 1].SMSC.Number);
	EncodeUTF8(destination, sms->SMS[Part - 1].Number);

	if (err == SMSD_SEND_OK) {
		SMSD_Log(DEBUG_NOTICE, Config, "Transmitted ID %s (%d/%d) to %s",
			Config->SMSID, Part, sms->Number, destination);
	}

	if (err == SMSD_SEND_OK) {
		if (sms->SMS[Part - 1].PDU == SMS_Status_Report) {
			strcpy(message_state, "SendingOK");
		} else {
			strcpy(message_state, "SendingOKNoReport");
		}
	} else if (err == SMSD_SEND_SENDING_ERROR) {
		strcpy(message_state, "SendingError");
	} else if (err == SMSD_SEND_ERROR) {
		strcpy(message_state, "Error");
	} else {
		SMSD_Log(DEBUG_INFO, Config, "Unknown SMS state: %d, assuming Error", err);
		strcpy(message_state, "Error");
	}

	strcpy(buffer, "INSERT INTO sentitems "
		"(CreatorID, ID, SequencePosition, Status, SendingDateTime,  SMSCNumber,  TPMR, "
		"SenderID, Text, DestinationNumber, Coding, UDH, Class, TextDecoded, InsertIntoDB, "
                "RelativeValidity) VALUES (");
	sprintf(buffer + strlen(buffer),
		"'%s', '%s', %i, '%s', %s, '%s', %i, '%s', '",
		Config->PhoneID, ID, Part, message_state,
		SMSDDBI_Now(Config),
		smsc, TPMR, Config->PhoneID);

	switch (sms->SMS[Part - 1].Coding) {
		case SMS_Coding_Unicode_No_Compression:

		case SMS_Coding_Default_No_Compression:
			EncodeHexUnicode(buffer + strlen(buffer),
					 sms->SMS[Part - 1].Text,
					 UnicodeLength(sms->SMS[Part - 1].
						       Text));
			break;

		case SMS_Coding_8bit:
			EncodeHexBin(buffer + strlen(buffer),
				     sms->SMS[Part - 1].Text,
				     sms->SMS[Part - 1].Length);

		default:
			break;
	}

	sprintf(buffer + strlen(buffer), "', '%s', '", destination);

	switch (sms->SMS[Part - 1].Coding) {
		case SMS_Coding_Unicode_No_Compression:
			sprintf(buffer + strlen(buffer),
				"Unicode_No_Compression");
			break;

		case SMS_Coding_Unicode_Compression:
			sprintf(buffer + strlen(buffer), "Unicode_Compression");
			break;

		case SMS_Coding_Default_No_Compression:
			sprintf(buffer + strlen(buffer),
				"Default_No_Compression");
			break;

		case SMS_Coding_Default_Compression:
			sprintf(buffer + strlen(buffer), "Default_Compression");
			break;

		case SMS_Coding_8bit:
			sprintf(buffer + strlen(buffer), "8bit");
			break;
	}

	sprintf(buffer + strlen(buffer), "', '");

	if (sms->SMS[Part - 1].UDH.Type != UDH_NoUDH) {
		EncodeHexBin(buffer + strlen(buffer),
			     sms->SMS[Part - 1].UDH.Text,
			     sms->SMS[Part - 1].UDH.Length);
	}

	sprintf(buffer + strlen(buffer), "', %i, ",
		sms->SMS[Part - 1].Class);

	switch (sms->SMS[Part - 1].Coding) {
		case SMS_Coding_Unicode_No_Compression:

		case SMS_Coding_Default_No_Compression:
			EncodeUTF8(buffer2, sms->SMS[Part - 1].Text);
			dbi_conn_quote_string_copy(Config->DBConnDBI, buffer2, &encoded_text);
			strcat(buffer, encoded_text);
			free(encoded_text);
			break;

		case SMS_Coding_8bit:
			/* FIXME */
			strcat(buffer, "''");
			break;

		default:
			strcat(buffer, "''");
			break;
	}

	sprintf(buffer + strlen(buffer), ", '%s', ", Config->DT);

	if (sms->SMS[Part - 1].SMSC.Validity.Format ==
	    SMS_Validity_RelativeFormat) {
		sprintf(buffer + strlen(buffer), "%i)",
			sms->SMS[Part - 1].SMSC.Validity.Relative);
	} else {
		sprintf(buffer + strlen(buffer), "-1)");
	}

	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	dbi_result_free(Res);

	sprintf(buffer, "UPDATE phones SET Sent= Sent + 1 WHERE IMEI = '%s'", Config->Status->IMEI);
	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error updating number of sent messages (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	dbi_result_free(Res);

	return ERR_NONE;
}

static GSM_Error SMSDDBI_RefreshPhoneStatus(GSM_SMSDConfig * Config)
{
	dbi_result Res;

	char buffer[500];

	sprintf(buffer,
		"UPDATE phones SET TimeOut= %s, Battery = %d, Signal = %d WHERE IMEI = '%s'",
		SMSDDBI_NowPlus(Config, 10),
		Config->Status->Charge.BatteryPercent, Config->Status->Network.SignalPercent, Config->Status->IMEI);
	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error writing to database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	dbi_result_free(Res);

	return ERR_NONE;
}

GSM_SMSDService SMSDDBI = {
	SMSDDBI_Init,
	SMSDDBI_Free,
	SMSDDBI_InitAfterConnect,
	SMSDDBI_SaveInboxSMS,
	SMSDDBI_FindOutboxSMS,
	SMSDDBI_MoveSMS,
	SMSDDBI_CreateOutboxSMS,
	SMSDDBI_AddSentSMSInfo,
	SMSDDBI_RefreshSendStatus,
	SMSDDBI_RefreshPhoneStatus
};

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
