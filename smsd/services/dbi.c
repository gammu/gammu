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
			SMSD_Log(-1, Config, "Wrong integer field subtype! (Field = %s, type = %d)", field, type);
			return -1;
		case DBI_TYPE_DECIMAL:
			type = dbi_result_get_field_attribs(res, field);
			if ((type & DBI_DECIMAL_SIZEMASK) == DBI_DECIMAL_SIZE4) {
				return dbi_result_get_int(res, field);
			} else if ((type & DBI_DECIMAL_SIZEMASK) == DBI_DECIMAL_SIZE8) {
				return dbi_result_get_longlong(res, field);
			}
			SMSD_Log(-1, Config, "Wrong decimal field subtype! (Field = %s, type = %d)", field, type);
			return -1;
		case DBI_TYPE_ERROR:
		default:
			SMSD_Log(-1, Config, "Wrong field type! (Field = %s, type = %d)", field, type);
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
			SMSD_Log(-1, Config, "Got date as %s (Field = %s, type = %d)", date, field, type);
			parse_res = strptime(date, "%Y-%m-%d %H:%M:%S", &timestruct);
			timestruct.tm_isdst = 0;
#ifdef HAVE_STRUCT_TM_TM_ZONE
			timestruct.tm_gmtoff = 0;
			timestruct.tm_zone = NULL;
#endif
			if (parse_res != NULL && *parse_res == 0) {
				return mktime(&timestruct);
			}
			SMSD_Log(-1, Config, "DBI error: 9999 Failed to process date %s (Field = %s, type = %d)", date, field, type);
			return -1;
		case DBI_TYPE_DATETIME:
			return dbi_result_get_datetime(res, field);
		case DBI_TYPE_ERROR:
		default:
			SMSD_Log(-1, Config, "Wrong date field type! (Field = %s, type = %d)", field, type);
			return -1;
	}
}

bool SMSDDBI_GetBool(GSM_SMSDConfig * Config, dbi_result res, const char *field)
{
	unsigned int type;
	const char *value;
	int num;

	type = dbi_result_get_field_type(res, field);

	switch (type) {
		case DBI_TYPE_INTEGER:
		case DBI_TYPE_DECIMAL:
			num = SMSDDBI_GetNumber(Config, res, field);
			SMSD_Log(-1, Config, "Got bool as %d (Field = %s, type = %d)", num, field, type);
			if (num == -1) {
				return -1;
			} else if (num == 0) {
				return false;
			} else {
				return true;
			}
		case DBI_TYPE_STRING:
			value= dbi_result_get_string(res, field);
			SMSD_Log(-1, Config, "Got bool as %s (Field = %s, type = %d)", value, field, type);
			if (strcasecmp(value, "yes") == 0 || strcasecmp(value, "true") == 0 ||
					strcasecmp(value, "y") == 0 || strcasecmp(value, "t") == 0) {
				return true;
			}
			if (strcasecmp(value, "no") == 0 || strcasecmp(value, "false") == 0 ||
					strcasecmp(value, "n") == 0 || strcasecmp(value, "f") == 0) {
				return true;
			}
			if (strcasecmp(value, "default") == 0) {
				return -1;
			}
			SMSD_Log(-1, Config, "DBI error: 9999 Failed to process bool %s (Field = %s, type = %d)", value, field, type);
			return -1;
		case DBI_TYPE_ERROR:
		default:
			SMSD_Log(-1, Config, "Wrong bool field type! (Field = %s, type = %d)", field, type);
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
		SMSD_Log(-1, Config, "Unknown DBI error!");
	} else {
		SMSD_Log(-1, Config, "DBI error %d: %s", rc, msg);
	}
}

void SMSDDBI_Callback(dbi_conn Conn, void *Config)
{
	SMSDDBI_LogError((GSM_SMSDConfig *)Config);
}

/* Connects to database */
static GSM_Error SMSDDBI_Connect(GSM_SMSDConfig * Config)
{
	int rc;

	rc = dbi_initialize(Config->driverspath);

	if (rc == 0) {
		SMSD_Log(-1, Config, "DBI did not find any drivers, try using DriversPath option");
		dbi_shutdown();
		return ERR_UNKNOWN;
	} else if (rc < 0) {
		SMSD_Log(-1, Config, "DBI failed to initialize!");
		return ERR_UNKNOWN;
	}

	Config->DBConnDBI = dbi_conn_new(Config->driver);
	if (Config->DBConnDBI == NULL) {
		SMSD_Log(-1, Config, "DBI failed to init %s driver!", Config->driver);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}

	dbi_conn_error_handler(Config->DBConnDBI, SMSDDBI_Callback, Config);

	if (dbi_conn_set_option(Config->DBConnDBI, "sqlite_dbdir", Config->dbdir) != 0) {
		SMSD_Log(-1, Config, "DBI failed to set sqlite_dbdir!");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "sqlite3_dbdir", Config->dbdir) != 0) {
		SMSD_Log(-1, Config, "DBI failed to set sqlite3_dbdir!");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "host", Config->PC) != 0) {
		SMSD_Log(-1, Config, "DBI failed to set host!");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "username", Config->user) != 0) {
		SMSD_Log(-1, Config, "DBI failed to set username!");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "password", Config->password) != 0) {
		SMSD_Log(-1, Config, "DBI failed to set password!");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "dbname", Config->database) != 0) {
		SMSD_Log(-1, Config, "DBI failed to set dbname!");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "encoding", "UTF-8") != 0) {
		SMSD_Log(-1, Config, "DBI failed to set encoding!");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}

	if (dbi_conn_connect(Config->DBConnDBI) != 0) {
		SMSD_Log(-1, Config, "DBI failed to connect!");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	return ERR_NONE;
}

/* Disconnects from a database */
static GSM_Error SMSDDBI_Free(GSM_SMSDConfig *Config)
{
	if (Config->DBConnDBI != NULL) {
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		Config->DBConnDBI = NULL;
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
		SMSD_Log(2, Config, "Execute SQL: %s", query);
		*res = dbi_conn_query(Config->DBConnDBI, query);
		if (*res != NULL) return ERR_NONE;

		SMSD_Log(0, Config, "SQL failed: %s", query);
		if (attempts >= SMSD_SQL_RETRIES) {
			return ERR_TIMEOUT;
		}
		/* Black magic to decide whether we should bail out or attempt to retry */
		rc = dbi_conn_error(Config->DBConnDBI, &msg);
		if (rc != -1) {
			SMSD_Log(0, Config, "SQL failure: %s", msg);
			if (strstr(msg, "syntax") != NULL) {
				return ERR_BUG;
			}
			if (strstr(msg, "violation") != NULL) {
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
				SMSD_Log(0, Config, "Retrying after %d seconds...", 5 * attempts);
				sleep(5 * attempts);
				continue;
			}
		}
		/* We will try to reconnect */
		SMSD_Log(0, Config, "Failed to detect problem cause, reconnecting to database!");
		error = ERR_UNKNOWN;
		while (error != ERR_NONE) {
			SMSD_Log(0, Config, "Reconnecting after %d seconds...", 5 * attempts);
			sleep(5 * attempts);
			SMSDDBI_Free(Config);
			error = SMSDDBI_Connect(Config);
			if (error == ERR_NONE) {
				break;
			}
			attempts++;
		}
	}
	return ERR_TIMEOUT;
}

static GSM_Error SMSDDBI_CheckTable(GSM_SMSDConfig * Config, const char *table)
{
	dbi_result res;
	char buffer[200];
	GSM_Error error;

	sprintf(buffer, "SELECT id FROM %s", table);
	error = SMSDDBI_Query(Config, buffer, &res);
	if (error != ERR_NONE) {
		SMSD_Log(-1, Config, "Table %s not found!", table);
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
		SMSD_Log(-1, Config, "No Version information in table gammu!");
		dbi_result_free(res);
		SMSDDBI_Free(Config);
		return ERR_UNKNOWN;
	}
	if (dbi_result_first_row(res) != 1) {
		SMSD_Log(-1, Config, "Failed to seek to first row!");
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

	SMSD_Log(0, Config, "Connected to Database %s: %s on %s", Config->driver, Config->database, Config->PC);

	return ERR_NONE;
}


static GSM_Error SMSDDBI_InitAfterConnect(GSM_SMSDConfig * Config)
{
	unsigned char buf[400], buf2[200];

	dbi_result Res;

	sprintf(buf, "DELETE FROM phones WHERE IMEI = '%s'", Config->Status->IMEI);

	if (SMSDDBI_Query(Config, buf, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error deleting from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	dbi_result_free(Res);

	sprintf(buf2, "Gammu %s", VERSION);
	if (strlen(GetOS()) != 0) {
		strcat(buf2 + strlen(buf2), ", ");
		strcat(buf2 + strlen(buf2), GetOS());
	}
	if (strlen(GetCompiler()) != 0) {
		strcat(buf2 + strlen(buf2), ", ");
		strcat(buf2 + strlen(buf2), GetCompiler());
	}

	sprintf(buf,
		"INSERT INTO phones (IMEI, ID, Send, Receive, InsertIntoDB, TimeOut, Client, Battery, Signal) VALUES ('%s', '%s', 'yes', 'yes', %s, %s, '%s', -1, -1)",
		Config->Status->IMEI, Config->PhoneID,
		SMSDDBI_Now(Config),
		SMSDDBI_NowPlus(Config, 10), buf2);

	if (SMSDDBI_Query(Config, buf, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error inserting into database (%s)", __FUNCTION__);
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

	unsigned char buffer[10000], buffer2[400], buffer3[50];
	int i;
	time_t t_time1, t_time2;
	bool found;
	long diff;
	unsigned long long	new_id;
	size_t			locations_size = 0, locations_pos = 0;
	const char *state, *smsc;

	*Locations = NULL;

	for (i = 0; i < sms->Number; i++) {
		if (sms->SMS[i].PDU == SMS_Status_Report) {
			strcpy(buffer2, DecodeUnicodeString(sms->SMS[i].Number));
			if (strncasecmp(Config->deliveryreport, "log", 3) == 0) {
				SMSD_Log(0, Config, "Delivery report: %s to %s",
					     DecodeUnicodeString(sms->SMS[i].
								 Text),
					     buffer2);
			}

			sprintf(buffer,
				"SELECT ID, Status, SendingDateTime, DeliveryDateTime, SMSCNumber "
                                        "FROM sentitems WHERE "
					"DeliveryDateTime = 'epoch' AND "
					"SenderID = '%s' AND TPMR = '%i' AND DestinationNumber = '%s'",
				Config->PhoneID, sms->SMS[i].MessageReference, buffer2);
			if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
				SMSD_Log(0, Config, "Error reading from database (%s)", __FUNCTION__);
				return ERR_UNKNOWN;
			}

			found = false;
			while (dbi_result_next_row(Res)) {
				smsc = dbi_result_get_string(Res, "SMSCNumber");
				state = dbi_result_get_string(Res, "Status");

				if (strcmp(smsc, DecodeUnicodeString(sms->SMS[i].SMSC.Number)) != 0) {
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
						found = true;
						break;
					}
				}
			}

			if (found) {
				sprintf(buffer, "UPDATE sentitems SET DeliveryDateTime = '%04i%02i%02i%02i%02i%02i', "
                                          "Status = '",
					sms->SMS[i].SMSCTime.Year, sms->SMS[i].SMSCTime.Month, sms->SMS[i].SMSCTime.Day, sms->SMS[i].SMSCTime.Hour, sms->SMS[i].SMSCTime.Minute,
					sms->SMS[i].SMSCTime.Second);

				sprintf(buffer3, "%s",
					DecodeUnicodeString(sms->SMS[i].Text));
				if (!strcmp(buffer3, "Delivered")) {
					sprintf(buffer + strlen(buffer),
						"DeliveryOK");
				} else if (!strcmp(buffer3, "Failed")) {
					sprintf(buffer + strlen(buffer),
						"DeliveryFailed");
				} else if (!strcmp(buffer3, "Pending")) {
					sprintf(buffer + strlen(buffer),
						"DeliveryPending");
				} else if (!strcmp(buffer3, "Unknown")) {
					sprintf(buffer + strlen(buffer),
						"DeliveryUnknown");
				}

				sprintf(buffer + strlen(buffer),
					"', StatusError = '%i'",
					sms->SMS[i].DeliveryStatus);
				sprintf(buffer + strlen(buffer),
					" WHERE ID = %lld AND `TPMR` = '%i'",
					SMSDDBI_GetNumber(Config, Res, "ID"),
					sms->SMS[i].MessageReference);
				if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
					SMSD_Log(0, Config, "Error writing to database (%s)", __FUNCTION__);
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

		sprintf(buffer + strlen(buffer), "','%s','",
			DecodeUnicodeString(sms->SMS[i].Number));

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

		sprintf(buffer + strlen(buffer), "','%s'",
			DecodeUnicodeString(sms->SMS[i].SMSC.Number));

		if (sms->SMS[i].UDH.Type == UDH_NoUDH) {
			sprintf(buffer + strlen(buffer), ",''");
		} else {
			sprintf(buffer + strlen(buffer), ",'");
			EncodeHexBin(buffer + strlen(buffer),
				     sms->SMS[i].UDH.Text,
				     sms->SMS[i].UDH.Length);
			sprintf(buffer + strlen(buffer), "'");
		}

		sprintf(buffer + strlen(buffer), ",'%i',", sms->SMS[i].Class);

		switch (sms->SMS[i].Coding) {

			case SMS_Coding_Unicode_No_Compression:

			case SMS_Coding_Default_No_Compression:
				EncodeUTF8(buffer2, sms->SMS[i].Text);
				dbi_conn_quote_string_copy(Config->DBConnDBI, buffer2, &encoded_text);
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
			SMSD_Log(0, Config, "Error writing to database (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		dbi_result_free(Res);

		new_id = dbi_conn_sequence_last(Config->DBConnDBI, NULL);
		SMSD_Log(1, Config, "Inserted message id %llu", new_id);

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

		if (SMSDDBI_Query(Config, "UPDATE phones SET Received = Received + 1", &Res) != ERR_NONE) {
			SMSD_Log(0, Config, "Error updating number of received messages (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		dbi_result_free(Res);

	}

	return ERR_NONE;
}

static GSM_Error SMSDDBI_RefreshSendStatus(GSM_SMSDConfig * Config,
					     char *ID)
{
	unsigned char buffer[10000];
	dbi_result Res;

	sprintf(buffer, "UPDATE outbox SET SendingTimeOut = %s "
                        "WHERE ID = '%s' AND SendingTimeOut < %s",
		SMSDDBI_NowPlus(Config, 15),
		ID,
		SMSDDBI_Now(Config));
	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error writing to database (%s)", __FUNCTION__);
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
	unsigned char buf[400];
	dbi_result Res;
	int i;
	bool found = false;
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
		SMSD_Log(0, Config, "Error reading from database (%s)", __FUNCTION__);
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
			found = true;
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
				"FROM outbox_multipart WHERE ID='%s' AND SequencePosition='%i'",
				ID, i);
		}
		if (SMSDDBI_Query(Config, buf, &Res) != ERR_NONE) {
			SMSD_Log(0, Config, "Error reading from database (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}

		if (dbi_result_get_numrows(Res) == 0) {
			dbi_result_free(Res);
			return ERR_NONE;
		}
		dbi_result_first_row(Res);

		coding = dbi_result_get_string(Res, "Coding");
		text = dbi_result_get_string(Res, "Text");
		text_len = strlen(text);
		text_decoded = dbi_result_get_string(Res, "TextDecoded");
		udh = dbi_result_get_string(Res, "UDH");
		udh_len = strlen(udh);

		sms->SMS[sms->Number].Coding = SMS_Coding_8bit;
		if (strcmp(coding, "Unicode_No_Compression") == 0) {
			sms->SMS[sms->Number].Coding = SMS_Coding_Unicode_No_Compression;
		} else if (strcmp(coding, "Default_No_Compression") == 0) {
			sms->SMS[sms->Number].Coding = SMS_Coding_Default_No_Compression;
		}

		if (text == NULL || text_len == 0) {
			SMSD_Log(1, Config, "Message: %s", text_decoded);
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
			EncodeUnicode(sms->SMS[sms->Number].Number, destination, strlen(destination));
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
				   bool alwaysDelete UNUSED, bool sent UNUSED)
{
	unsigned char buffer[10000];
	dbi_result Res;

	sprintf(buffer, "DELETE FROM outbox WHERE ID = '%s'", ID);
	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error deleting from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	dbi_result_free(Res);

	sprintf(buffer, "DELETE FROM outbox_multipart WHERE ID = '%s'", ID);
	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error deleting from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	dbi_result_free(Res);

	return ERR_NONE;
}

/* Adds SMS to Outbox */
static GSM_Error SMSDDBI_CreateOutboxSMS(GSM_MultiSMSMessage * sms,
					   GSM_SMSDConfig * Config)
{
	unsigned char buffer[10000], buffer2[400], buffer4[10000];
	int i;
	unsigned int ID;
	dbi_result Res;
	char *encoded_text;

	sprintf(buffer, "SELECT ID FROM outbox ORDER BY ID DESC LIMIT 1");
	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error reading from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}

	if (dbi_result_get_numrows(Res) != 0) {
		dbi_result_first_row(Res);
		ID = SMSDDBI_GetNumber(Config, Res, "ID");
	} else {
		ID = 0;
	}
	dbi_result_free(Res);

	sprintf(buffer, "SELECT ID FROM sentitems ORDER BY ID DESC LIMIT 1");
	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error reading from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}

	if (dbi_result_get_numrows(Res) != 0) {
		dbi_result_first_row(Res);
		ID = MAX(SMSDDBI_GetNumber(Config, Res, "ID"), ID);
	}
	dbi_result_free(Res);

	/* Get next ID */
	ID++;

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

		sprintf(buffer + strlen(buffer), "Coding, UDH,  "
			  "Class, TextDecoded, ID) VALUES (");

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
				sprintf(buffer + strlen(buffer), "false");
			} else {
				sprintf(buffer + strlen(buffer), "true");
			}

			sprintf(buffer + strlen(buffer), "', %s",
				SMSDDBI_Now(Config));
		} else {
			sprintf(buffer + strlen(buffer), "'%i'", i + 1);
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
			sprintf(buffer + strlen(buffer), "'%s', ",
				DecodeUnicodeString(sms->SMS[i].Number));

			if (sms->SMS[i].SMSC.Validity.Format ==
			    SMS_Validity_RelativeFormat) {
				sprintf(buffer + strlen(buffer), "'%i', ",
					sms->SMS[i].SMSC.Validity.Relative);
			} else {
				sprintf(buffer + strlen(buffer), "'-1', ");
			}
		}

		sprintf(buffer + strlen(buffer), "'");
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

		sprintf(buffer + strlen(buffer), "', '");
		if (sms->SMS[i].UDH.Type != UDH_NoUDH) {
			EncodeHexBin(buffer + strlen(buffer),
				     sms->SMS[i].UDH.Text,
				     sms->SMS[i].UDH.Length);
		}

		sprintf(buffer + strlen(buffer), "', '%i', ",
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

		sprintf(buffer + strlen(buffer), ", '");
		strcpy(buffer4, buffer);
		sprintf(buffer4 + strlen(buffer4), "%u')", ID);
		if (SMSDDBI_Query(Config, buffer4, &Res) != ERR_NONE) {
			SMSD_Log(0, Config, "Error writing to database (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		dbi_result_free(Res);
	}
	SMSD_Log(0, Config, "Written message with ID %u", ID);
	return ERR_NONE;
}

static GSM_Error SMSDDBI_AddSentSMSInfo(GSM_MultiSMSMessage * sms,
					  GSM_SMSDConfig * Config,
					  char *ID, int Part,
					  GSM_SMSDSendingError err, int TPMR)
{
	dbi_result Res;

	unsigned char buffer[10000], buffer2[400], buff[50];
	char *encoded_text;

	if (err == SMSD_SEND_OK) {
		SMSD_Log(1, Config, "Transmitted ID %s (%d/%d) to %s",
			Config->SMSID,
			Part, sms->Number,
			DecodeUnicodeString(sms->SMS[0].Number));
	}

	buff[0] = 0;
	if (err == SMSD_SEND_OK) {
		if (sms->SMS[Part - 1].PDU == SMS_Status_Report) {
			sprintf(buff, "SendingOK");
		} else {
			sprintf(buff, "SendingOKNoReport");
		}
	}

	if (err == SMSD_SEND_SENDING_ERROR)
		sprintf(buff, "SendingError");
	if (err == SMSD_SEND_ERROR)
		sprintf(buff, "Error");

	buffer[0] = 0;
	sprintf(buffer + strlen(buffer), "INSERT INTO sentitems "
		"(CreatorID, ID, SequencePosition, Status, SendingDateTime,  SMSCNumber,  TPMR, "
		"SenderID, Text, DestinationNumber, Coding, UDH, Class, TextDecoded, InsertIntoDB, "
                "RelativeValidity) VALUES (");
	sprintf(buffer + strlen(buffer),
		"'%s', '%s', '%i', '%s', %s, '%s', '%i', '%s', '",
		Config->PhoneID, ID, Part,
		buff,
		SMSDDBI_Now(Config),
		DecodeUnicodeString(sms->SMS[Part - 1].SMSC.Number),
		TPMR,
		Config->PhoneID);

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

	sprintf(buffer + strlen(buffer), "', '%s', '",
		DecodeUnicodeString(sms->SMS[Part - 1].Number));

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

	sprintf(buffer + strlen(buffer), "', '%i', ",
		sms->SMS[Part - 1].Class);

	switch (sms->SMS[Part - 1].Coding) {
		case SMS_Coding_Unicode_No_Compression:

		case SMS_Coding_Default_No_Compression:
			EncodeUTF8(buffer2, sms->SMS[Part - 1].Text);
			dbi_conn_quote_string_copy(Config->DBConnDBI, buffer2, &encoded_text);
			strcat(buffer, encoded_text);
			free(encoded_text);
			break;

		default:
			break;
	}

	sprintf(buffer + strlen(buffer), ", '%s', '", Config->DT);

	if (sms->SMS[Part - 1].SMSC.Validity.Format ==
	    SMS_Validity_RelativeFormat) {
		sprintf(buffer + strlen(buffer), "%i')",
			sms->SMS[Part - 1].SMSC.Validity.Relative);
	} else {
		sprintf(buffer + strlen(buffer), "-1')");
	}

	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error writing to database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	dbi_result_free(Res);

	if (SMSDDBI_Query(Config, "UPDATE phones SET Sent = Sent + 1", &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error updating number of sent messages (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	dbi_result_free(Res);

	return ERR_NONE;
}

static GSM_Error SMSDDBI_RefreshPhoneStatus(GSM_SMSDConfig * Config)
{
	dbi_result Res;

	unsigned char buffer[500];

	sprintf(buffer,
		"UPDATE phones SET TimeOut= %s, Battery = %d, Signal = %d WHERE IMEI = '%s'",
		SMSDDBI_NowPlus(Config, 10),
		Config->Status->Charge.BatteryPercent, Config->Status->Network.SignalPercent, Config->Status->IMEI);
	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error writing to database (%s)", __FUNCTION__);
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
