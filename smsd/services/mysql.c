/* (c) 2004 by Marcin Wiacek */

#include <gammu.h>

#ifdef HAVE_MYSQL_MYSQL_H

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

#ifdef WIN32
#  include <windows.h>
#ifndef __GNUC__
#  pragma comment(lib, "libmysql.lib")
#endif
#endif

#include "../core.h"

/* Disconnects from a database */
static GSM_Error SMSDMySQL_Free(GSM_SMSDConfig *Config)
{
	SMSD_Log(DEBUG_NOTICE, Config, "Disconnecting from MySQL");
	mysql_close(&Config->DBConnMySQL);
	return ERR_NONE;
}

static void SMSDMySQL_LogError(GSM_SMSDConfig *Config)
{
	SMSD_Log(DEBUG_ERROR, Config, "Error code: %d, Error: %s", mysql_errno(&Config->DBConnMySQL), mysql_error(&Config->DBConnMySQL));
}


struct _TableCheck
{
	const char *query;
	const char *msg;
} TableCheck;

static struct _TableCheck	tc[] =
{
	{ "SELECT ID FROM `outbox` WHERE 1", "No table for outbox sms" },
	{ "SELECT ID FROM `outbox_multipart` WHERE 1", "No table for outbox multipart sms" },
	{ "SELECT ID FROM `sentitems` WHERE 1", "No table for sent sms" },
	{ "SELECT ID FROM `inbox` WHERE 1", "No table for inbox sms" },
	{ NULL, NULL },
};

/* Connects to database */
static GSM_Error SMSDMySQL_Init(GSM_SMSDConfig *Config)
{
	MYSQL_RES 		*Res;
	MYSQL_ROW 		Row;
	struct _TableCheck *T;

	unsigned int port = 0;
	char * pport;
	char * socketname = NULL;

	pport = strstr( Config->PC, ":" );
	if (pport) {
		*pport ++ = '\0';
		/* Is it port or socket? */
		if (strchr("0123456798", *pport) != NULL) {
			port = atoi( pport );
		} else {
			socketname = pport;
		}
	}

	mysql_init(&Config->DBConnMySQL);
	if (!mysql_real_connect(
				&Config->DBConnMySQL,
				Config->PC,
				Config->user,
				Config->password,
				Config->database,
				port,
				socketname,
				0)) {
	    	SMSD_Log(DEBUG_ERROR, Config, "Error connecting to database!");
		SMSDMySQL_LogError(Config);
	    	return ERR_UNKNOWN;
	}

	for (T = tc; T->query; T++) {
		if (mysql_query(&Config->DBConnMySQL, T->query)) {
			SMSD_Log(DEBUG_ERROR, Config, "%s", T->msg);
			SMSDMySQL_LogError(Config);
			mysql_close(&Config->DBConnMySQL);
			return ERR_UNKNOWN;
		}
		if (!(Res = mysql_store_result(&Config->DBConnMySQL))) {
			SMSD_Log(DEBUG_ERROR, Config, "%s", T->msg);
			SMSDMySQL_LogError(Config);
			mysql_close(&Config->DBConnMySQL);
			return ERR_UNKNOWN;
		}
		mysql_free_result(Res);
	}

	if (mysql_query(&Config->DBConnMySQL, "SELECT Version FROM `gammu` WHERE 1")) {
		SMSD_Log(DEBUG_ERROR, Config, "No Gammu table");
		SMSDMySQL_LogError(Config);
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	if (!(Res = mysql_store_result(&Config->DBConnMySQL))) {
		SMSD_Log(DEBUG_ERROR, Config, "No Gammu table");
		SMSDMySQL_LogError(Config);
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	if (!(Row = mysql_fetch_row(Res))) {
		mysql_free_result(Res);
		SMSD_Log(DEBUG_ERROR, Config, "No version info in Gammu table");
		SMSDMySQL_LogError(Config);
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	if (SMSD_CheckDBVersion(Config, atoi(Row[0])) != ERR_NONE) {
		mysql_free_result(Res);
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	mysql_free_result(Res);
	mysql_query(&Config->DBConnMySQL, "SET NAMES utf8;");
	SMSD_Log(DEBUG_INFO, Config, "Connected to Database: %s on %s", Config->database, Config->PC);
	return ERR_NONE;
}

static GSM_Error SMSDMySQL_Query_Real(GSM_SMSDConfig *Config, const char *query, gboolean retry)
{
	SMSD_Log(DEBUG_SQL, Config, "Execute SQL: %s", query);

	if (mysql_query(&Config->DBConnMySQL, query) != 0) {
		SMSD_Log(DEBUG_INFO, Config, "SQL failed: %s", query);
		SMSDMySQL_LogError(Config);
		/* MySQL server has gone away */
		if (retry && mysql_errno(&Config->DBConnMySQL) == 2006) {
			SMSD_Log(DEBUG_INFO, Config, "Trying to reconnect to the Database...");
			SMSDMySQL_Free(Config);
			sleep(30);
			SMSDMySQL_Init(Config);
			SMSD_Log(DEBUG_INFO, Config, "Retrying query...");
			return SMSDMySQL_Query_Real(Config, query, FALSE);
		}

	}
	return ERR_NONE;
}

static GSM_Error SMSDMySQL_Query(GSM_SMSDConfig *Config, const char *query)
{
	return SMSDMySQL_Query_Real(Config, query, TRUE);
}

static GSM_Error SMSDMySQL_Store_Real(GSM_SMSDConfig *Config, const char *query, MYSQL_RES **result, gboolean retry)
{
	GSM_Error error;

	error = SMSDMySQL_Query_Real(Config, query, TRUE);
	if (error != ERR_NONE) return error;

	*result = mysql_store_result(&Config->DBConnMySQL);

	if (*result == NULL) {
		SMSD_Log(DEBUG_INFO, Config, "Store result failed: %s", query);
		SMSDMySQL_LogError(Config);
		/* MySQL server has gone away */
		if (retry && mysql_errno(&Config->DBConnMySQL) == 2006) {
			SMSD_Log(DEBUG_INFO, Config, "Trying to reconnect to the Database...");
			SMSDMySQL_Free(Config);
			sleep(30);
			SMSDMySQL_Init(Config);
			SMSD_Log(DEBUG_INFO, Config, "Retrying query...");
			return SMSDMySQL_Store_Real(Config, query, result, FALSE);
		}
	}

	return ERR_NONE;
}

static GSM_Error SMSDMySQL_Store(GSM_SMSDConfig *Config, const char *query, MYSQL_RES **result)
{
	return SMSDMySQL_Store_Real(Config, query, result, TRUE);
}

static GSM_Error SMSDMySQL_InitAfterConnect(GSM_SMSDConfig *Config)
{
	char query[400];
	char client_name[200];

	snprintf(query, sizeof(query), "DELETE FROM `phones` WHERE `IMEI` = '%s'", Config->Status->IMEI);
	if (SMSDMySQL_Query(Config, query) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error deleting phone info from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}

	snprintf(client_name, sizeof(client_name), "Gammu %s, %s, %s", VERSION, GetOS(), GetCompiler());

	SMSD_Log(DEBUG_INFO, Config, "Communication established");
	snprintf(query, sizeof(query),
		"INSERT INTO `phones` (`IMEI`,`ID`,`Send`,`Receive`,`InsertIntoDB`,`TimeOut`,`Client`, `Battery`, `Signal`) VALUES ('%s','%s','yes','yes',NOW(),(NOW() + INTERVAL 10 SECOND)+0,'%s', -1, -1)",
		Config->Status->IMEI, Config->PhoneID, client_name);
	if (SMSDMySQL_Query(Config, query) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error inserting phone info into database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}

	return ERR_NONE;
}

/* Save SMS from phone (called Inbox sms - it's in phone Inbox) somewhere */
static GSM_Error SMSDMySQL_SaveInboxSMS(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char **Locations)
{
	MYSQL_RES 		*Res;
	MYSQL_ROW 		Row;
	unsigned char		buffer[10000],buffer2[400],buffer3[50],buffer4[800];
	int 			i;
	time_t     		t_time1,t_time2;
	gboolean			found;
	long 			diff;
	my_ulonglong		new_id;
	size_t			locations_size = 0, locations_pos = 0;
	char			buf[400];

	*Locations = NULL;

	for (i=0;i<sms->Number;i++) {
		if (sms->SMS[i].PDU == SMS_Status_Report) {
			strcpy(buffer2, DecodeUnicodeString(sms->SMS[i].Number));

			SMSD_Log(DEBUG_INFO, Config, "Delivery report: %s to %s", DecodeUnicodeString(sms->SMS[i].Text), buffer2);

			sprintf(buffer, "SELECT ID,Status,UNIX_TIMESTAMP(SendingDateTime),DeliveryDateTime,SMSCNumber FROM `sentitems` WHERE "
					"DeliveryDateTime IS NULL AND "
					"SenderID='%s' AND TPMR='%i' AND DestinationNumber='%s'",
					Config->PhoneID, sms->SMS[i].MessageReference, buffer2);
			if (SMSDMySQL_Store(Config, buffer, &Res) != ERR_NONE) {
				SMSD_Log(DEBUG_INFO, Config, "Error reading sent items from database (%s)", __FUNCTION__);
				return ERR_UNKNOWN;
			}
			found = FALSE;
			while ((Row = mysql_fetch_row(Res))) {
				SMSD_Log(DEBUG_NOTICE, Config, "Checking for delivery report, SMSC=%s, state=%s", Row[4], Row[1]);
				if (strcmp(Row[4],DecodeUnicodeString(sms->SMS[i].SMSC.Number))) {
					if (Config->skipsmscnumber[0] == 0) continue;
					if (strcmp(Config->skipsmscnumber,Row[4])) continue;
				}
				if (!strcmp(Row[1],"SendingOK") || !strcmp(Row[1],"DeliveryPending")) {
					t_time1 = atoi(Row[2]);
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
				sprintf(buffer,"UPDATE `sentitems` SET `DeliveryDateTime`='%04i-%02i-%02i %02i:%02i:%02i', `Status`='",
					sms->SMS[i].SMSCTime.Year,sms->SMS[i].SMSCTime.Month,sms->SMS[i].SMSCTime.Day,
					sms->SMS[i].SMSCTime.Hour,sms->SMS[i].SMSCTime.Minute,sms->SMS[i].SMSCTime.Second);
				sprintf(buffer3,"%s",DecodeUnicodeString(sms->SMS[i].Text));
				if (!strcmp(buffer3,"Delivered")) {
					sprintf(buffer+strlen(buffer),"DeliveryOK");
				} else if (!strcmp(buffer3,"Failed")) {
					sprintf(buffer+strlen(buffer),"DeliveryFailed");
				} else if (!strcmp(buffer3,"Pending")) {
					sprintf(buffer+strlen(buffer),"DeliveryPending");
				} else if (!strcmp(buffer3,"Unknown")) {
					sprintf(buffer+strlen(buffer),"DeliveryUnknown");
				}
				sprintf(buffer+strlen(buffer),"', `StatusError` = '%i'",sms->SMS[i].DeliveryStatus);
				sprintf(buffer+strlen(buffer)," WHERE `ID` = '%s' AND `TPMR` = '%i'",Row[0],sms->SMS[i].MessageReference);
				if (SMSDMySQL_Query(Config, buffer) != ERR_NONE) {
					SMSD_Log(DEBUG_INFO, Config, "Error updating sent item i ndatabase (%s)", __FUNCTION__);
					return ERR_UNKNOWN;
				}
			}
			mysql_free_result(Res);
			continue;
		}
		if (sms->SMS[i].PDU != SMS_Deliver) continue;
		buffer[0]=0;
		sprintf(buffer+strlen(buffer),"INSERT INTO `inbox` "
			"(`ReceivingDateTime`,`Text`,`SenderNumber`,`Coding`,`SMSCNumber`,`UDH`, "
			"`Class`,`TextDecoded`,`RecipientID`) VALUES ('%04d%02d%02d%02d%02d%02d','",
			sms->SMS[i].DateTime.Year,sms->SMS[i].DateTime.Month,sms->SMS[i].DateTime.Day,
			sms->SMS[i].DateTime.Hour,sms->SMS[i].DateTime.Minute,sms->SMS[i].DateTime.Second);
		switch (sms->SMS[i].Coding) {
		case SMS_Coding_Unicode_No_Compression:
	    	case SMS_Coding_Default_No_Compression:
			EncodeHexUnicode(buffer+strlen(buffer),sms->SMS[i].Text,UnicodeLength(sms->SMS[i].Text));
			break;
		case SMS_Coding_8bit:
			EncodeHexBin(buffer+strlen(buffer),sms->SMS[i].Text,sms->SMS[i].Length);
		default:
			break;
		}
		sprintf(buffer+strlen(buffer),"','%s','",DecodeUnicodeString(sms->SMS[i].Number));
		switch (sms->SMS[i].Coding) {
		case SMS_Coding_Unicode_No_Compression:
			sprintf(buffer+strlen(buffer),"Unicode_No_Compression");
			break;
		case SMS_Coding_Unicode_Compression:
			sprintf(buffer+strlen(buffer),"Unicode_Compression");
			break;
	    	case SMS_Coding_Default_No_Compression:
			sprintf(buffer+strlen(buffer),"Default_No_Compression");
			break;
	    	case SMS_Coding_Default_Compression:
			sprintf(buffer+strlen(buffer),"Default_Compression");
			break;
		case SMS_Coding_8bit:
			sprintf(buffer+strlen(buffer),"8bit");
			break;
		}
		sprintf(buffer+strlen(buffer),"','%s'",DecodeUnicodeString(sms->SMS[i].SMSC.Number));
		if (sms->SMS[i].UDH.Type == UDH_NoUDH) {
			sprintf(buffer+strlen(buffer),",''");
		} else {
			sprintf(buffer+strlen(buffer),",'");
			EncodeHexBin(buffer+strlen(buffer),sms->SMS[i].UDH.Text,sms->SMS[i].UDH.Length);
			sprintf(buffer+strlen(buffer),"'");
		}
		sprintf(buffer+strlen(buffer),",'%i','",sms->SMS[i].Class);
		switch (sms->SMS[i].Coding) {
		case SMS_Coding_Unicode_No_Compression:
	    	case SMS_Coding_Default_No_Compression:
			EncodeUTF8(buffer2, sms->SMS[i].Text);
			mysql_real_escape_string(&Config->DBConnMySQL,buffer4,buffer2,strlen(buffer2));
			memcpy(buffer+strlen(buffer),buffer4,strlen(buffer4)+1);
			break;
		case SMS_Coding_8bit:
			break;
		default:
			break;
		}
		sprintf(buffer+strlen(buffer),"','%s')",Config->PhoneID);
		if (SMSDMySQL_Query(Config, buffer) != ERR_NONE) {
			SMSD_Log(DEBUG_INFO, Config, "Error writing inbox message to database (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		new_id = mysql_insert_id(&Config->DBConnMySQL);
		if (new_id == 0) {
			SMSD_Log(DEBUG_INFO, Config, "Error writing inbox message to database (%s), new id is zero!", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		SMSD_Log(DEBUG_NOTICE, Config, "Inserted message id %lu", (long)new_id);

		if (locations_pos + 10 >= locations_size) {
			locations_size += 40;
			*Locations = (char *)realloc(*Locations, locations_size);
			assert (*Locations != NULL);
			if (locations_pos == 0) {
				*Locations[0] = 0;
			}
		}
		locations_pos += sprintf((*Locations) + locations_pos, "%lu ", (long)new_id);

		sprintf(buf, "UPDATE phones SET Received = Received + 1 WHERE IMEI = '%s'", Config->Status->IMEI);
		if (SMSDMySQL_Query(Config, buf) != ERR_NONE) {
			SMSD_Log(DEBUG_INFO, Config, "Error updating number of received messages (%s)", __FUNCTION__);
	    		mysql_free_result(Res);
			return ERR_UNKNOWN;
		}
	}

	return ERR_NONE;
}

static GSM_Error SMSDMySQL_RefreshSendStatus(GSM_SMSDConfig *Config, char *ID)
{
	unsigned char buffer[10000];

	sprintf(buffer,"UPDATE `outbox` SET `SendingTimeOut`=(now() + INTERVAL 15 SECOND)+0 WHERE `ID` = '%s' AND (`SendingTimeOut` < now() OR SendingTimeOut IS NULL)",ID);
	if (SMSDMySQL_Query(Config, buffer) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error updating send timestamp in database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	if (mysql_affected_rows(&Config->DBConnMySQL) == 0) return ERR_UNKNOWN;
  	return ERR_NONE;
}

/* Find one multi SMS to sending and return it (or return ERR_EMPTY)
 * There is also set ID for SMS
 */
static GSM_Error SMSDMySQL_FindOutboxSMS(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *ID)
{
	unsigned char 		buf[400];
	MYSQL_RES 		*Res;
	MYSQL_ROW 		Row;
	int 			i;
	gboolean			found = FALSE;

	if (SMSDMySQL_Store(Config, "SELECT ID,InsertIntoDB,SendingDateTime,SenderID FROM `outbox` WHERE SendingDateTime < NOW() AND SendingTimeOut < NOW()", &Res) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error reading message to send from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	while ((Row = mysql_fetch_row(Res))) {
		sprintf(ID,"%s",Row[0]);
		sprintf(Config->DT,"%s",Row[1]);
		if (Row[3] == NULL || strlen(Row[3]) == 0 || !strcmp(Row[3],Config->PhoneID)) {
			if (SMSDMySQL_RefreshSendStatus(Config, ID)==ERR_NONE) {
				found = TRUE;
				break;
			}
		}
	}
	mysql_free_result(Res);

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
			sprintf(buf, "SELECT Text,Coding,UDH,Class,TextDecoded,ID,DestinationNumber,MultiPart,RelativeValidity,DeliveryReport,CreatorID FROM `outbox` WHERE ID='%s'",ID);
		} else {
			sprintf(buf, "SELECT Text,Coding,UDH,Class,TextDecoded,ID,SequencePosition FROM `outbox_multipart` WHERE ID='%s' AND SequencePosition='%i'",ID,i);
		}
		if (SMSDMySQL_Store(Config, buf, &Res) != ERR_NONE) {
			SMSD_Log(DEBUG_INFO, Config, "Error reading outbox message from database (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		if (!(Row = mysql_fetch_row(Res))) {
	    		mysql_free_result(Res);
			return ERR_NONE;
		}

		sms->SMS[sms->Number].Coding = SMS_Coding_8bit;
		if (!strcmp(Row[1], "Unicode_No_Compression")) {
			sms->SMS[sms->Number].Coding = SMS_Coding_Unicode_No_Compression;
		}
		if (!strcmp(Row[1], "Default_No_Compression")) {
			sms->SMS[sms->Number].Coding = SMS_Coding_Default_No_Compression;
		}

		if (Row[0] == NULL || strlen(Row[0])==0) {
			SMSD_Log(DEBUG_NOTICE, Config, "Message: %s", Row[4]);
			DecodeUTF8(sms->SMS[sms->Number].Text,Row[4],strlen(Row[4]));
		} else {
			switch (sms->SMS[sms->Number].Coding) {
			case SMS_Coding_Unicode_No_Compression:
			case SMS_Coding_Default_No_Compression:
				DecodeHexUnicode(sms->SMS[sms->Number].Text,Row[0],strlen(Row[0]));
				break;
			case SMS_Coding_8bit:
				DecodeHexBin(sms->SMS[sms->Number].Text,Row[0],strlen(Row[0]));
				sms->SMS[sms->Number].Length=strlen(Row[0])/2;
			default:
				break;
			}
		}
		if (i == 1) {
			EncodeUnicode(sms->SMS[sms->Number].Number,Row[6],strlen(Row[6]));
		} else {
			CopyUnicodeString(sms->SMS[sms->Number].Number,sms->SMS[0].Number);
		}
		sms->SMS[sms->Number].UDH.Type = UDH_NoUDH;
		if (Row[2]!=NULL && strlen(Row[2])!=0) {
			sms->SMS[sms->Number].UDH.Type 		= UDH_UserUDH;
			sms->SMS[sms->Number].UDH.Length 	= strlen(Row[2])/2;
			DecodeHexBin(sms->SMS[sms->Number].UDH.Text,Row[2],strlen(Row[2]));
		}
		sms->SMS[sms->Number].Class		= atoi(Row[3]);
		sms->SMS[sms->Number].PDU  		= SMS_Submit;
		sms->Number++;
		if (i==1) {
			sprintf(Config->CreatorID,"%s",Row[10]);

			Config->relativevalidity = atoi(Row[8]);

			Config->currdeliveryreport = -1;
			if (!strcmp(Row[9],"yes")) {
				Config->currdeliveryreport = 1;
			} else if (!strcmp(Row[9],"no")) {
				Config->currdeliveryreport = 0;
			}

			if (!strcmp(Row[7],"FALSE")) {
				mysql_free_result(Res);
				break;
			}

		}
		mysql_free_result(Res);
	}
  	return ERR_NONE;
}

/* After sending SMS is moved to Sent Items or Error Items. */
static GSM_Error SMSDMySQL_MoveSMS(GSM_MultiSMSMessage *sms UNUSED,
		GSM_SMSDConfig *Config, char *ID,
		gboolean alwaysDelete UNUSED, gboolean sent UNUSED)
{
	char buffer[100];

	sprintf(buffer,"DELETE FROM `outbox` WHERE `ID` = '%s'",ID);
	if (SMSDMySQL_Query(Config, buffer) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error deleting outbox message from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	sprintf(buffer,"DELETE FROM `outbox_multipart` WHERE `ID` = '%s'",ID);
	if (SMSDMySQL_Query(Config, buffer) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error deleting multipart outbox message from database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
  	return ERR_NONE;
}

/* Adds SMS to Outbox */
static GSM_Error SMSDMySQL_CreateOutboxSMS(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *NewID)
{
	char			buffer[10000],buffer2[400],buffer5[400];
	int 			i,ID = 0;


	for (i=0;i<sms->Number;i++) {
		buffer[0]=0;
		if (i==0) {
			sprintf(buffer+strlen(buffer),"INSERT INTO `outbox` (`CreatorID`,`SenderID`,`DeliveryReport`,`MultiPart`,`InsertIntoDB");
		} else {
			sprintf(buffer+strlen(buffer),"INSERT INTO `outbox_multipart` (`SequencePosition");
		}
		sprintf(buffer+strlen(buffer),"`,`Text`,");
		if (i==0) {
			sprintf(buffer+strlen(buffer),"`DestinationNumber`,`RelativeValidity`,");
		}

		strcat(buffer, "`Coding`, `UDH`, `Class`, `TextDecoded`");
		if (i != 0) {
			strcat(buffer, ", `ID`");
		}
		strcat(buffer, ") VALUES (");

		if (i==0) {
			sprintf(buffer+strlen(buffer),"'Gammu %s',",VERSION);
			sprintf(buffer+strlen(buffer),"'%s','", Config->PhoneID);
			if (sms->SMS[i].PDU == SMS_Status_Report) {
				sprintf(buffer+strlen(buffer),"yes','");
			} else {
				sprintf(buffer+strlen(buffer),"default','");
			}
			if (sms->Number == 1) {
				sprintf(buffer+strlen(buffer),"FALSE");
			} else {
				sprintf(buffer+strlen(buffer),"TRUE");
			}
			sprintf(buffer+strlen(buffer),"',NOW()");
		} else {
			sprintf(buffer+strlen(buffer),"'%i'",i+1);
		}
		sprintf(buffer+strlen(buffer),",'");

		switch (sms->SMS[i].Coding) {
		case SMS_Coding_Unicode_No_Compression:
	    	case SMS_Coding_Default_No_Compression:
			EncodeHexUnicode(buffer+strlen(buffer),sms->SMS[i].Text,UnicodeLength(sms->SMS[i].Text));
			break;
		case SMS_Coding_8bit:
			EncodeHexBin(buffer+strlen(buffer),sms->SMS[i].Text,sms->SMS[i].Length);
		default:
			break;
		}
		sprintf(buffer+strlen(buffer),"',");
		if (i==0) {
			sprintf(buffer+strlen(buffer),"'%s',",DecodeUnicodeString(sms->SMS[i].Number));
			if (sms->SMS[i].SMSC.Validity.Format == SMS_Validity_RelativeFormat) {
				sprintf(buffer+strlen(buffer),"'%i',",sms->SMS[i].SMSC.Validity.Relative);
			} else {
				sprintf(buffer+strlen(buffer),"'-1',");
			}
		}
		sprintf(buffer+strlen(buffer),"'");
		switch (sms->SMS[i].Coding) {
		case SMS_Coding_Unicode_No_Compression:
			sprintf(buffer+strlen(buffer),"Unicode_No_Compression");
			break;
		case SMS_Coding_Unicode_Compression:
			sprintf(buffer+strlen(buffer),"Unicode_Compression");
			break;
	    	case SMS_Coding_Default_No_Compression:
			sprintf(buffer+strlen(buffer),"Default_No_Compression");
			break;
	    	case SMS_Coding_Default_Compression:
			sprintf(buffer+strlen(buffer),"Default_Compression");
			break;
		case SMS_Coding_8bit:
			sprintf(buffer+strlen(buffer),"8bit");
			break;
		}
		sprintf(buffer+strlen(buffer),"','");
		if (sms->SMS[i].UDH.Type != UDH_NoUDH) {
			EncodeHexBin(buffer+strlen(buffer),sms->SMS[i].UDH.Text,sms->SMS[i].UDH.Length);
		}
		sprintf(buffer+strlen(buffer),"','%i','",sms->SMS[i].Class);
		switch (sms->SMS[i].Coding) {
		case SMS_Coding_Unicode_No_Compression:
	    	case SMS_Coding_Default_No_Compression:
			EncodeUTF8(buffer2, sms->SMS[i].Text);
			mysql_real_escape_string(&Config->DBConnMySQL,buffer5,buffer2,strlen(buffer2));
			memcpy(buffer+strlen(buffer),buffer5,strlen(buffer5)+1);
			break;
		default:
			break;
		}
		sprintf(buffer+strlen(buffer),"'");
		if (i != 0) {
			sprintf(buffer + strlen(buffer), ", %u", ID);
		}
		strcat(buffer, ")");
		if (SMSDMySQL_Query(Config, buffer) != ERR_NONE) {
			SMSD_Log(DEBUG_INFO, Config, "Error writing outbox record to database (%s)", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		if (i == 0) {
			ID = mysql_insert_id(&Config->DBConnMySQL);
			if (ID == 0) {
				SMSD_Log(DEBUG_INFO, Config, "Failed to get inserted row ID (%s)", __FUNCTION__);
				return ERR_UNKNOWN;
			}
		}
	}
	SMSD_Log(DEBUG_INFO, Config, "Written message with ID %i", ID);
	if (NewID != NULL) sprintf(NewID, "%d", ID);
  	return ERR_NONE;
}


static GSM_Error SMSDMySQL_AddSentSMSInfo(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *ID, int Part, GSM_SMSDSendingError err, int TPMR)
{
	char send_state[100];
	char query[10000];
	char utf_text[2 * GSM_MAX_SMS_LENGTH], escaped_text[2 * GSM_MAX_SMS_LENGTH];
	int query_pos;

	if (err == SMSD_SEND_OK) {
		SMSD_Log(DEBUG_NOTICE, Config, "Transmitted %s (%s: %i) to %s", Config->SMSID,
			     (Part == sms->Number ? "total" : "part"), Part,
			     DecodeUnicodeString(sms->SMS[0].Number));
	}

	send_state[0] = 0;
	if (err == SMSD_SEND_OK) {
		if (sms->SMS[Part-1].PDU == SMS_Status_Report) {
			strcpy(send_state, "SendingOK");
		} else {
			strcpy(send_state, "SendingOKNoReport");
		}
	}
	if (err == SMSD_SEND_SENDING_ERROR)
		strcpy(send_state, "SendingError");
	if (err == SMSD_SEND_ERROR)
		strcpy(send_state, "Error");

	query[0] = 0;
	query_pos = 0;
	query_pos += snprintf(query + query_pos, sizeof(query) - query_pos,
		"INSERT INTO `sentitems` "
		"(`CreatorID`,`ID`,`SequencePosition`,`Status`,`SendingDateTime`, `SMSCNumber`, `TPMR`, "
		"`SenderID`,`Text`,`DestinationNumber`,`Coding`,`UDH`,`Class`,`TextDecoded`,`InsertIntoDB`,`RelativeValidity`) VALUES (");
	query_pos += snprintf(query + query_pos, sizeof(query) - query_pos,
		"'%s','%s','%i','%s',NOW(),'%s','%i','%s','",
		Config->CreatorID, ID, Part, send_state, DecodeUnicodeString(sms->SMS[Part-1].SMSC.Number), TPMR, Config->PhoneID);
	switch (sms->SMS[Part-1].Coding) {
		case SMS_Coding_Unicode_No_Compression:
		case SMS_Coding_Default_No_Compression:
			EncodeHexUnicode(query + query_pos,
				sms->SMS[Part-1].Text,
				UnicodeLength(sms->SMS[Part-1].Text));
			query_pos = strlen(query);
			break;
		case SMS_Coding_8bit:
			EncodeHexBin(query + query_pos,
				sms->SMS[Part-1].Text,
				sms->SMS[Part-1].Length);
			query_pos = strlen(query);
		default:
			break;
	}
	query_pos += snprintf(query + query_pos, sizeof(query) - query_pos,
		"','%s','",
		DecodeUnicodeString(sms->SMS[Part-1].Number));
	switch (sms->SMS[Part-1].Coding) {
		case SMS_Coding_Unicode_No_Compression:
			query_pos += snprintf(query + query_pos, sizeof(query) - query_pos,
				"Unicode_No_Compression");
			break;
		case SMS_Coding_Unicode_Compression:
			query_pos += snprintf(query + query_pos, sizeof(query) - query_pos,
				"Unicode_Compression");
			break;
		case SMS_Coding_Default_No_Compression:
			query_pos += snprintf(query + query_pos, sizeof(query) - query_pos,
				"Default_No_Compression");
			break;
		case SMS_Coding_Default_Compression:
			query_pos += snprintf(query + query_pos, sizeof(query) - query_pos,
				"Default_Compression");
			break;
		case SMS_Coding_8bit:
			query_pos += snprintf(query + query_pos, sizeof(query) - query_pos,
				"8bit");
			break;
	}
	query_pos += snprintf(query + query_pos, sizeof(query) - query_pos,
		"','");
	if (sms->SMS[Part-1].UDH.Type != UDH_NoUDH) {
		EncodeHexBin(query + query_pos,
			sms->SMS[Part-1].UDH.Text,
			sms->SMS[Part-1].UDH.Length);
		query_pos = strlen(query);
	}
	query_pos += snprintf(query + query_pos, sizeof(query) - query_pos,
		"','%i','",
		sms->SMS[Part-1].Class);
	switch (sms->SMS[Part-1].Coding) {
		case SMS_Coding_Unicode_No_Compression:
		case SMS_Coding_Default_No_Compression:
			EncodeUTF8(utf_text, sms->SMS[Part-1].Text);
			mysql_real_escape_string(&Config->DBConnMySQL, escaped_text, utf_text, strlen(utf_text));
			query_pos += snprintf(query + query_pos, sizeof(query) - query_pos,
				"%s",
				escaped_text);
			break;
		default:
			break;
	}
	query_pos += snprintf(query + query_pos, sizeof(query) - query_pos,
		"','%s','",
		Config->DT);
	if (sms->SMS[Part-1].SMSC.Validity.Format == SMS_Validity_RelativeFormat) {
		query_pos += snprintf(query + query_pos, sizeof(query) - query_pos,
			"%i')",
			sms->SMS[Part-1].SMSC.Validity.Relative);
	} else {
		query_pos += snprintf(query + query_pos, sizeof(query) - query_pos,
			"-1')");
	}
	if (SMSDMySQL_Query(Config, query) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error writing sent item to database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	snprintf(query, sizeof(query), "UPDATE `phones` SET `Sent`= `Sent` + 1 WHERE IMEI = '%s'", Config->Status->IMEI);
	if (SMSDMySQL_Query(Config, query) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error updating number of sent messages (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
  	return ERR_NONE;
}

static GSM_Error SMSDMySQL_RefreshPhoneStatus(GSM_SMSDConfig *Config)
{
	unsigned char buffer[500];

	sprintf(buffer,
		"UPDATE `phones` SET `TimeOut`= (NOW() + INTERVAL 10 SECOND)+0, `Battery`= %d, `Signal`= %d WHERE `IMEI` = '%s'",
		Config->Status->Charge.BatteryPercent, Config->Status->Network.SignalPercent, Config->Status->IMEI);
	if (SMSDMySQL_Query(Config, buffer) != ERR_NONE) {
		SMSD_Log(DEBUG_INFO, Config, "Error writing phone information to database (%s)", __FUNCTION__);
		return ERR_UNKNOWN;
	}
	return ERR_NONE;
}

GSM_SMSDService SMSDMySQL = {
	SMSDMySQL_Init,
	SMSDMySQL_Free,
	SMSDMySQL_InitAfterConnect,
	SMSDMySQL_SaveInboxSMS,
	SMSDMySQL_FindOutboxSMS,
	SMSDMySQL_MoveSMS,
	SMSDMySQL_CreateOutboxSMS,
	SMSDMySQL_AddSentSMSInfo,
	SMSDMySQL_RefreshSendStatus,
	SMSDMySQL_RefreshPhoneStatus
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
