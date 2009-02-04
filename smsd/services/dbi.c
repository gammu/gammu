/**
 * libdbi database backend
 *
 * Part of Gammu project
 *
 * Copyright (C) 2009 Michal Čihař
 *
 * Licensed under GNU GPL version 2 or later
 */

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
		SMSD_Log(-1, Config, "Unknown DBI error!\n");
	} else {
		SMSD_Log(-1, Config, "DBI error %d: %s\n", rc, msg);
	}
}

static GSM_Error SMSDDBI_CheckTable(GSM_SMSDConfig * Config, const char *table)
{
	dbi_result res;

	res = dbi_conn_queryf(Config->DBConnDBI, "SELECT id FROM %s", table);
	if (res == NULL) {
		SMSD_Log(-1, Config, "Table %s not found!", table);
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	dbi_result_free(res);
	return ERR_NONE;
}

void SMSDDBI_Callback(dbi_conn Conn, void *Config)
{
	SMSDDBI_LogError((GSM_SMSDConfig *)Config);
}

/* Connects to database */
static GSM_Error SMSDDBI_Init(GSM_SMSDConfig * Config)
{
	int rc;
	dbi_result res;
	int version;
	GSM_Error error;

	rc = dbi_initialize(Config->driverspath);

	if (rc == 0) {
		SMSD_Log(-1, Config, "DBI did not find any drivers, try using DriversPath option\n");
		dbi_shutdown();
		return ERR_UNKNOWN;
	} else if (rc < 0) {
		SMSD_Log(-1, Config, "DBI failed to initialize!\n");
		return ERR_UNKNOWN;
	}

	Config->DBConnDBI = dbi_conn_new(Config->driver);
	if (Config->DBConnDBI == NULL) {
		SMSD_Log(-1, Config, "DBI failed to init %s driver!\n", Config->driver);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}

	dbi_conn_error_handler(Config->DBConnDBI, SMSDDBI_Callback, Config);

	if (dbi_conn_set_option(Config->DBConnDBI, "sqlite_dbdir", Config->dbdir) != 0) {
		SMSD_Log(-1, Config, "DBI failed to set sqlite_dbdir!\n");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "sqlite3_dbdir", Config->dbdir) != 0) {
		SMSD_Log(-1, Config, "DBI failed to set sqlite3_dbdir!\n");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "host", Config->PC) != 0) {
		SMSD_Log(-1, Config, "DBI failed to set host!\n");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "username", Config->user) != 0) {
		SMSD_Log(-1, Config, "DBI failed to set username!\n");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "password", Config->password) != 0) {
		SMSD_Log(-1, Config, "DBI failed to set password!\n");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "dbname", Config->database) != 0) {
		SMSD_Log(-1, Config, "DBI failed to set dbname!\n");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	if (dbi_conn_set_option(Config->DBConnDBI, "encoding", "UTF-8") != 0) {
		SMSD_Log(-1, Config, "DBI failed to set encoding!\n");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}

	if (dbi_conn_connect(Config->DBConnDBI) != 0) {
		SMSD_Log(-1, Config, "DBI failed to connect!\n");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}

	error = SMSDDBI_CheckTable(Config, "outbox");
	if (error != ERR_NONE) return error;
	error = SMSDDBI_CheckTable(Config, "outbox_multipart");
	if (error != ERR_NONE) return error;
	error = SMSDDBI_CheckTable(Config, "sentitems");
	if (error != ERR_NONE) return error;
	error = SMSDDBI_CheckTable(Config, "inbox");
	if (error != ERR_NONE) return error;

	res = dbi_conn_query(Config->DBConnDBI, "SELECT Version FROM gammu");
	if (res == NULL) {
		SMSD_Log(-1, Config, "Table gammu not found!");
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	if (dbi_result_get_numrows(res) != 1) {
		SMSD_Log(-1, Config, "No Version information in table gammu!");
		dbi_result_free(res);
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	if (dbi_result_first_row(res) != 1) {
		SMSD_Log(-1, Config, "Failed to seek to first row!");
		dbi_result_free(res);
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}
	version = dbi_result_get_longlong_idx(res, 1);
	dbi_result_free(res);
	if (SMSD_CheckDBVersion(Config, version) != ERR_NONE) {
		dbi_conn_close(Config->DBConnDBI);
		dbi_shutdown();
		return ERR_UNKNOWN;
	}

	SMSD_Log(0, Config, "Connected to Database %s: %s on %s", Config->driver, Config->database, Config->PC);

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
	GSM_Error error;
	int attempts = 1;

	*res = NULL;

	for (attempts = 1; attempts < SMSD_SQL_RETRIES; attempts++) {
		SMSD_Log(2, Config, "Execute SQL: %s\n", query);
		*res = dbi_conn_query(Config->DBConnDBI, query);
		if (*res != NULL) return ERR_NONE;

		SMSD_Log(0, Config, "SQL failed: %s\n", query);
		if (attempts >= SMSD_SQL_RETRIES) {
			return ERR_TIMEOUT;
		}
		/* Black magic to decide whether we should bail out or attempt to retry */
		if (dbi_conn_error(Config->DBConnDBI, &msg) == 0) {
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
			if (strstr(msg, "locked") != NULL) {
				SMSD_Log(0, Config, "Retrying after %d seconds...\n", 5 * attempts);
				sleep(5 * attempts);
				continue;
			}
		}
		/* We will try to reconnect */
		SMSD_Log(0, Config, "Failed to detect problem cause, reconnecting to database!\n");
		error = ERR_UNKNOWN;
		while (error != ERR_NONE) {
			SMSD_Log(0, Config, "Reconnecting fter %d seconds...\n", 5 * attempts);
			sleep(5 * attempts);
			SMSDDBI_Free(Config);
			error = SMSDDBI_Init(Config);
			if (error == ERR_NONE) {
				break;
			}
			attempts++;
		}
	}
	return ERR_TIMEOUT;
}

static GSM_Error SMSDDBI_InitAfterConnect(GSM_SMSDConfig * Config)
{
	unsigned char buf[400], buf2[200];

	dbi_result Res;

	sprintf(buf, "DELETE FROM phones WHERE IMEI = '%s'", Config->Status->IMEI);

	if (SMSDDBI_Query(Config, buf, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error deleting from database (%s)\n", __FUNCTION__);
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
		SMSD_Log(0, Config, "Error inserting into database (%s)\n", __FUNCTION__);
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
				"SELECT ID, Status, EXTRACT(EPOCH FROM SendingDateTime), DeliveryDateTime, SMSCNumber "
                                        "FROM sentitems WHERE "
					"DeliveryDateTime = 'epoch' AND "
					"SenderID = '%s' AND TPMR = '%i' AND DestinationNumber = '%s'",
				Config->PhoneID, sms->SMS[i].MessageReference, buffer2);
			if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
				SMSD_Log(0, Config, "Error reading from database (%s)\n", __FUNCTION__);
				return ERR_UNKNOWN;
			}

			found = false;
			while (dbi_result_next_row(Res)) {

				if (strcmp
				    (dbi_result_get_string_idx(Res, 5),
				     DecodeUnicodeString(sms->SMS[i].SMSC.
							 Number))) {
					if (Config->skipsmscnumber[0] == 0)
						continue;
					if (strcmp
					    (Config->skipsmscnumber,
					     dbi_result_get_string_idx(Res, 5)))
						continue;
				}

				if (!strcmp(dbi_result_get_string_idx(Res, 2), "SendingOK")
				    || !strcmp(dbi_result_get_string_idx(Res, 2),
					       "DeliveryPending")) {
					t_time1 = dbi_result_get_int_idx(Res, 3);
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
					" WHERE ID = '%s' AND `TPMR` = '%i'",
					 dbi_result_get_string_idx(Res, 1),
					sms->SMS[i].MessageReference);
				if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
					SMSD_Log(0, Config, "Error writing to database (%s)\n", __FUNCTION__);
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
			SMSD_Log(0, Config, "Error writing to database (%s)\n", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		dbi_result_free(Res);

		new_id = dbi_conn_sequence_last(Config->DBConnDBI, NULL);
		SMSD_Log(1, Config, "Inserted message id %llu\n", new_id);

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
			SMSD_Log(0, Config, "Error updating number of received messages (%s)\n", __FUNCTION__);
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
		SMSD_Log(0, Config, "Error writing to database (%s)\n", __FUNCTION__);
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

	sprintf(buf,
		"SELECT ID, InsertIntoDB, SendingDateTime, SenderID FROM outbox "
                      "WHERE SendingDateTime < %s AND SendingTimeOut < %s",
		SMSDDBI_Now(Config),
		SMSDDBI_Now(Config));

	if (SMSDDBI_Query(Config, buf, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error reading from database (%s)\n", __FUNCTION__);
		return ERR_UNKNOWN;
	}

	while (dbi_result_next_row(Res)) {
		sprintf(ID, "%lld", dbi_result_get_longlong_idx(Res,1));
		sprintf(Config->DT, "%s", dbi_result_get_string_idx(Res, 2));

		if (dbi_result_get_string_idx(Res, 4) == NULL
		    || strlen(dbi_result_get_string_idx(Res, 4)) == 0
		    || !strcmp(dbi_result_get_string_idx(Res, 4), Config->PhoneID)) {

			if (SMSDDBI_RefreshSendStatus(Config, ID) == ERR_NONE) {
				found = true;
				break;
			}
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
				"RelativeValidity, DeliveryReport, CreatorID FROM outbox WHERE ID='%s'",
				ID);
		} else {
			sprintf(buf,
				"SELECT Text, Coding, UDH, Class, TextDecoded, ID, SequencePosition "
				"FROM outbox_multipart WHERE ID='%s' AND SequencePosition='%i'",
				ID, i);
		}
		if (SMSDDBI_Query(Config, buf, &Res) != ERR_NONE) {
			SMSD_Log(0, Config, "Error reading from database (%s)\n", __FUNCTION__);
			return ERR_UNKNOWN;
		}

		if (dbi_result_get_numrows(Res) == 0) {
			dbi_result_free(Res);
			return ERR_NONE;
		}
		dbi_result_first_row(Res);

		sms->SMS[sms->Number].Coding = SMS_Coding_8bit;
		if (!strcmp(dbi_result_get_string_idx(Res, 2), "Unicode_No_Compression")) {
			sms->SMS[sms->Number].Coding =
			    SMS_Coding_Unicode_No_Compression;
		}
		if (!strcmp(dbi_result_get_string_idx(Res, 2), "Default_No_Compression")) {
			sms->SMS[sms->Number].Coding =
			    SMS_Coding_Default_No_Compression;
		}

		if (dbi_result_get_string_idx(Res, 1) == NULL
		    || strlen(dbi_result_get_string_idx(Res, 1)) == 0) {
			SMSD_Log(1, Config, "Message: %s\n", dbi_result_get_string_idx(Res, 5));
			DecodeUTF8(sms->SMS[sms->Number].Text,
				   dbi_result_get_string_idx(Res, 5),
				   strlen(dbi_result_get_string_idx(Res, 5)));
		} else {
			switch (sms->SMS[sms->Number].Coding) {
				case SMS_Coding_Unicode_No_Compression:

				case SMS_Coding_Default_No_Compression:
					DecodeHexUnicode(sms->SMS[sms->Number].Text,
						dbi_result_get_string_idx(Res, 1),
						strlen(dbi_result_get_string_idx(Res, 1)));
					break;

				case SMS_Coding_8bit:
					DecodeHexBin(sms->SMS[sms->Number].Text,
						dbi_result_get_string_idx(Res, 1),
						strlen(dbi_result_get_string_idx(Res, 1)));
					sms->SMS[sms->Number].Length =
						 strlen(dbi_result_get_string_idx(Res, 1)) / 2;

				default:
					break;
			}
		}

		if (i == 1) {
			EncodeUnicode(sms->SMS[sms->Number].Number,
				      dbi_result_get_string_idx(Res, 7),
				      strlen(dbi_result_get_string_idx(Res, 7)));
		} else {
			CopyUnicodeString(sms->SMS[sms->Number].Number,
					  sms->SMS[0].Number);
		}

		sms->SMS[sms->Number].UDH.Type = UDH_NoUDH;
		if (dbi_result_get_string_idx(Res, 3) != NULL
		    && strlen(dbi_result_get_string_idx(Res, 3)) != 0) {
			sms->SMS[sms->Number].UDH.Type = UDH_UserUDH;
			sms->SMS[sms->Number].UDH.Length =
			    strlen(dbi_result_get_string_idx(Res, 3)) / 2;
			DecodeHexBin(sms->SMS[sms->Number].UDH.Text,
				     dbi_result_get_string_idx(Res, 3),
				     strlen(dbi_result_get_string_idx(Res, 3)));
		}

		sms->SMS[sms->Number].Class = dbi_result_get_int_idx(Res, 4);
		sms->SMS[sms->Number].PDU = SMS_Submit;
		sms->Number++;

		if (i == 1) {
			sprintf(Config->CreatorID, "%s",
				dbi_result_get_string_idx(Res, 11));

			Config->relativevalidity = dbi_result_get_int_idx(Res, 9);

			Config->currdeliveryreport = -1;
			if (!strcmp(dbi_result_get_string_idx(Res, 10), "yes")) {
				Config->currdeliveryreport = 1;
			} else if (!strcmp(dbi_result_get_string_idx(Res, 10), "no")) {
				Config->currdeliveryreport = 0;
			}

			if (!strcmp(dbi_result_get_string_idx(Res, 8), "f"))
				break;

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
		SMSD_Log(0, Config, "Error deleting from database (%s)\n", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	dbi_result_free(Res);

	sprintf(buffer, "DELETE FROM outbox_multipart WHERE ID = '%s'", ID);
	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error deleting from database (%s)\n", __FUNCTION__);
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
	int i, ID;
	int numb_rows, numb_tuples;
	dbi_result Res;
	char *encoded_text;

	sprintf(buffer, "SELECT ID FROM outbox ORDER BY ID DESC LIMIT 1");
	if (SMSDDBI_Query(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error reading from database (%s)\n", __FUNCTION__);
		return ERR_UNKNOWN;
	}

	if (dbi_result_get_numrows(Res) != 0) {
		dbi_result_first_row(Res);
		sprintf(buffer, "%lld", dbi_result_get_longlong_idx(Res, 1));
		ID = atoi(buffer);
	} else {
		ID = 0;
	}
	dbi_result_free(Res);

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
		if (i == 0) {
			while (true) {
				ID++;
				sprintf(buffer4,
					"SELECT ID FROM sentitems WHERE ID='%i'",
					ID);
				if (SMSDDBI_Query(Config, buffer4, &Res) != ERR_NONE) {
					SMSD_Log(0, Config, "Error reading from database (%s)\n", __FUNCTION__);
					return ERR_UNKNOWN;
				}

				numb_rows = dbi_result_get_numrows(Res);
				dbi_result_free(Res);
				if (numb_rows == 0) {
					sprintf(buffer4,
						"SELECT ID FROM outbox WHERE ID='%i'",
						ID);
					if (SMSDDBI_Query(Config, buffer4, &Res) != ERR_NONE) {
						SMSD_Log(0, Config, "Error writing to database (%s)\n", __FUNCTION__);
						return ERR_UNKNOWN;
					}

					numb_tuples = dbi_result_get_numrows(Res);
					dbi_result_free(Res);

					if (numb_tuples > 0) {
						SMSD_Log(0, Config, "Duplicated outgoing SMS ID\n");
						continue;
					} else {
						buffer4[0] = 0;
						strcpy(buffer4, buffer);
						sprintf(buffer4 +
							strlen(buffer4), "%i')",
							ID);

						if (SMSDDBI_Query(Config, buffer4, &Res) != ERR_NONE) {
							SMSD_Log(0, Config, "Error reading from database (%s)\n", __FUNCTION__);
							return ERR_UNKNOWN;
						}
						dbi_result_free(Res);
						break;
					}
				}
			}
		} else {
			strcpy(buffer4, buffer);
			sprintf(buffer4 + strlen(buffer4), "%i')", ID);
			if (SMSDDBI_Query(Config, buffer4, &Res) != ERR_NONE) {
				SMSD_Log(0, Config, "Error writing to database (%s)\n", __FUNCTION__);
				return ERR_UNKNOWN;
			}
			dbi_result_free(Res);
		}
	}
	SMSD_Log(0, Config, "Written message with ID %i\n", ID);
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
		SMSD_Log(1, Config, "Transmitted %s (%s: %i) to %s", Config->SMSID,
			     (Part == sms->Number ? "total" : "part"), Part,
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
		Config->CreatorID, ID, Part,
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
		SMSD_Log(0, Config, "Error writing to database (%s)\n", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	dbi_result_free(Res);

	if (SMSDDBI_Query(Config, "UPDATE phones SET Sent = Sent + 1", &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error updating number of sent messages (%s)\n", __FUNCTION__);
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
		SMSD_Log(0, Config, "Error writing to database (%s)\n", __FUNCTION__);
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
