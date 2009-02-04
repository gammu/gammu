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
	if (Config->connected) {
		mysql_close(&Config->DBConnMySQL);
		Config->connected = false;
	}
	return ERR_NONE;
}

/* Connects to database */
static GSM_Error SMSDMySQL_Init(GSM_SMSDConfig *Config)
{
	unsigned char		buf[400];
	MYSQL_RES 		*Res;
	MYSQL_ROW 		Row;

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
	    	SMSD_Log(-1, Config, "Error connecting to database: %s\n", mysql_error(&Config->DBConnMySQL));
	    	return ERR_UNKNOWN;
	}
	sprintf(buf, "SELECT ID FROM `outbox` WHERE 1");
	if (mysql_real_query(&Config->DBConnMySQL,buf,strlen(buf))) {
		SMSD_Log(-1, Config, "No table for outbox sms: %s\n", mysql_error(&Config->DBConnMySQL));
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	if (!(Res = mysql_store_result(&Config->DBConnMySQL))) {
		SMSD_Log(-1, Config, "No table for outbox sms: %s\n", mysql_error(&Config->DBConnMySQL));
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	mysql_free_result(Res);
	sprintf(buf, "SELECT ID FROM `outbox_multipart` WHERE 1");
	if (mysql_real_query(&Config->DBConnMySQL,buf,strlen(buf))) {
		SMSD_Log(-1, Config, "No table for outbox sms: %s\n", mysql_error(&Config->DBConnMySQL));
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	if (!(Res = mysql_store_result(&Config->DBConnMySQL))) {
		SMSD_Log(-1, Config, "No table for outbox sms: %s\n", mysql_error(&Config->DBConnMySQL));
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	mysql_free_result(Res);
	sprintf(buf, "SELECT ID FROM `sentitems` WHERE 1");
	if (mysql_real_query(&Config->DBConnMySQL,buf,strlen(buf))) {
		SMSD_Log(-1, Config, "No table for sent sms: %s\n", mysql_error(&Config->DBConnMySQL));
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	if (!(Res = mysql_store_result(&Config->DBConnMySQL))) {
		SMSD_Log(-1, Config, "No table for sent sms: %s\n", mysql_error(&Config->DBConnMySQL));
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	mysql_free_result(Res);
	sprintf(buf, "SELECT ID FROM `inbox` WHERE 1");
	if (mysql_real_query(&Config->DBConnMySQL,buf,strlen(buf))) {
		SMSD_Log(-1, Config, "No table for inbox sms: %s\n", mysql_error(&Config->DBConnMySQL));
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	if (!(Res = mysql_store_result(&Config->DBConnMySQL))) {
		SMSD_Log(-1, Config, "No table for inbox sms: %s\n", mysql_error(&Config->DBConnMySQL));
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	mysql_free_result(Res);
	sprintf(buf, "SELECT Version FROM `gammu` WHERE 1");
	if (mysql_real_query(&Config->DBConnMySQL,buf,strlen(buf))) {
		SMSD_Log(-1, Config, "No Gammu table: %s\n", mysql_error(&Config->DBConnMySQL));
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	if (!(Res = mysql_store_result(&Config->DBConnMySQL))) {
		SMSD_Log(-1, Config, "No Gammu table: %s\n", mysql_error(&Config->DBConnMySQL));
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	if (!(Row = mysql_fetch_row(Res))) {
		mysql_free_result(Res);
		SMSD_Log(-1, Config, "No version info in Gammu table: %s\n", mysql_error(&Config->DBConnMySQL));
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	if (SMSD_CheckDBVersion(Config, atoi(Row[0])) != ERR_NONE) {
		mysql_free_result(Res);
		mysql_close(&Config->DBConnMySQL);
		return ERR_UNKNOWN;
	}
	mysql_free_result(Res);
	mysql_query(&Config->DBConnMySQL,"SET NAMES utf8;");
	SMSD_Log(0, Config, "Connected to Database: %s on %s", Config->database, Config->PC);
	Config->connected = true;
	return ERR_NONE;
}

static void SMSDMySQL_LogError(GSM_SMSDConfig *Config)
{
	SMSD_Log(0, Config, "Error code: %d, Error: %s\n", mysql_errno(&Config->DBConnMySQL), mysql_error(&Config->DBConnMySQL));
}

static GSM_Error SMSDMySQL_Query_Real(GSM_SMSDConfig *Config, const char *query, bool retry)
{
	SMSD_Log(2, Config, "Execute SQL: %s\n", query);

	if (mysql_query(&Config->DBConnMySQL, query) != 0) {
		SMSD_Log(0, Config, "SQL failed: %s\n", query);
		SMSDMySQL_LogError(Config);
		/* MySQL server has gone away */
		if (retry && mysql_errno(&Config->DBConnMySQL) == 2006) {
			SMSD_Log(0, Config, "Trying to reconnect to the Database...");
			SMSDMySQL_Free(Config);
			sleep(30);
			SMSDMySQL_Init(Config);
			SMSD_Log(0, Config, "Retrying query...");
			return SMSDMySQL_Query_Real(Config, query, false);
		}

	}
	return ERR_NONE;
}

static GSM_Error SMSDMySQL_Query(GSM_SMSDConfig *Config, const char *query)
{
	return SMSDMySQL_Query_Real(Config, query, true);
}

static GSM_Error SMSDMySQL_Store_Real(GSM_SMSDConfig *Config, const char *query, MYSQL_RES **result, bool retry)
{
	GSM_Error error;

	error = SMSDMySQL_Query_Real(Config, query, true);
	if (error != ERR_NONE) return error;

	*result = mysql_store_result(&Config->DBConnMySQL);

	if (*result == NULL) {
		SMSD_Log(0, Config, "Store result failed: %s\n", query);
		SMSDMySQL_LogError(Config);
		/* MySQL server has gone away */
		if (retry && mysql_errno(&Config->DBConnMySQL) == 2006) {
			SMSD_Log(0, Config, "Trying to reconnect to the Database...");
			SMSDMySQL_Free(Config);
			sleep(30);
			SMSDMySQL_Init(Config);
			SMSD_Log(0, Config, "Retrying query...");
			return SMSDMySQL_Store_Real(Config, query, result, false);
		}
	}

	return ERR_NONE;
}

static GSM_Error SMSDMySQL_Store(GSM_SMSDConfig *Config, const char *query, MYSQL_RES **result)
{
	return SMSDMySQL_Store_Real(Config, query, result, true);
}

static GSM_Error SMSDMySQL_InitAfterConnect(GSM_SMSDConfig *Config)
{
	unsigned char buf[400],buf2[200];

	sprintf(buf,"DELETE FROM `phones` WHERE `IMEI` = '%s'", Config->Status->IMEI);
	if (SMSDMySQL_Query(Config, buf) != ERR_NONE) {
		SMSD_Log(0, Config, "Error deleting from database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
		return ERR_UNKNOWN;
	}

	sprintf(buf2,"Gammu %s",VERSION);
	if (strlen(GetOS()) != 0) {
		strcat(buf2+strlen(buf2),", ");
		strcat(buf2+strlen(buf2),GetOS());
	}
	if (strlen(GetCompiler()) != 0) {
		strcat(buf2+strlen(buf2),", ");
		strcat(buf2+strlen(buf2),GetCompiler());
	}

	SMSD_Log(0, Config, "Communication established");
	sprintf(buf,"INSERT INTO `phones` (`IMEI`,`ID`,`Send`,`Receive`,`InsertIntoDB`,`TimeOut`,`Client`, `Battery`, `Signal`) VALUES ('%s','%s','yes','yes',NOW(),(NOW() + INTERVAL 10 SECOND)+0,'%s', -1, -1)",
		Config->Status->IMEI, Config->PhoneID, buf2);
	if (SMSDMySQL_Query(Config, buf) != ERR_NONE) {
		SMSD_Log(0, Config, "Error inserting into database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
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
	bool			found;
	long 			diff;
	my_ulonglong		new_id;
	size_t			locations_size = 0, locations_pos = 0;

	*Locations = NULL;

	for (i=0;i<sms->Number;i++) {
		if (sms->SMS[i].PDU == SMS_Status_Report) {
			strcpy(buffer2, DecodeUnicodeString(sms->SMS[i].Number));
			if (strncasecmp(Config->deliveryreport, "log", 3) == 0) {
				SMSD_Log(0, Config, "Delivery report: %s to %s", DecodeUnicodeString(sms->SMS[i].Text), buffer2);
			}

			sprintf(buffer, "SELECT ID,Status,UNIX_TIMESTAMP(SendingDateTime),DeliveryDateTime,SMSCNumber FROM `sentitems` WHERE "
					"DeliveryDateTime='0000-00-00 00:00:00' AND "
					"SenderID='%s' AND TPMR='%i' AND DestinationNumber='%s'",
					Config->PhoneID, sms->SMS[i].MessageReference, buffer2);
			if (SMSDMySQL_Store(Config, buffer, &Res) != ERR_NONE) {
				SMSD_Log(0, Config, "Error reading from database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
				return ERR_UNKNOWN;
			}
			found = false;
			while ((Row = mysql_fetch_row(Res))) {
				if (strcmp(Row[4],DecodeUnicodeString(sms->SMS[i].SMSC.Number))) {
					if (Config->skipsmscnumber[0] == 0) continue;
					if (strcmp(Config->skipsmscnumber,Row[4])) continue;
				}
				if (!strcmp(Row[1],"SendingOK") || !strcmp(Row[1],"DeliveryPending")) {
					t_time1 = atoi(Row[2]);
					t_time2 = Fill_Time_T(sms->SMS[i].DateTime);
					diff = t_time2 - t_time1;

					if (diff > -Config->deliveryreportdelay && diff < Config->deliveryreportdelay) {
						found = true;
						break;
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
					SMSD_Log(0, Config, "Error writing to database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
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
			SMSD_Log(0, Config, "Error writing to database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
			return ERR_UNKNOWN;
		}
		new_id = mysql_insert_id(&Config->DBConnMySQL);
		if (new_id == 0) {
			SMSD_Log(0, Config, "Error writing to database (%s): %s, new id is zero!\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
			return ERR_UNKNOWN;
		}
		SMSD_Log(1, Config, "Inserted message id %llu\n", new_id);

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

	return ERR_NONE;
}

static GSM_Error SMSDMySQL_RefreshSendStatus(GSM_SMSDConfig *Config, char *ID)
{
	unsigned char buffer[10000];

	sprintf(buffer,"UPDATE `outbox` SET `SendingTimeOut`=(now() + INTERVAL 15 SECOND)+0 WHERE `ID` = '%s' AND `SendingTimeOut` < now()",ID);
	if (SMSDMySQL_Query(Config, buffer) != ERR_NONE) {
		SMSD_Log(0, Config, "Error writing to database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
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
	bool			found = false;

	sprintf(buf, "SELECT ID,InsertIntoDB,SendingDateTime,SenderID FROM `outbox` WHERE SendingDateTime < NOW() AND SendingTimeOut < NOW()");
	if (SMSDMySQL_Store(Config, buf, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error reading from database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
		return ERR_UNKNOWN;
	}
	while ((Row = mysql_fetch_row(Res))) {
		sprintf(ID,"%s",Row[0]);
		sprintf(Config->DT,"%s",Row[1]);
		if (Row[3] == NULL || strlen(Row[3]) == 0 || !strcmp(Row[3],Config->PhoneID)) {
			if (SMSDMySQL_RefreshSendStatus(Config, ID)==ERR_NONE) {
				found = true;
				break;
			}
		}
	}
	mysql_free_result(Res);

	if (!found) {
		return ERR_EMPTY;
	}

	sms->Number = 0;
	for (i=0;i<GSM_MAX_MULTI_SMS;i++) {
		GSM_SetDefaultSMSData(&sms->SMS[i]);
		sms->SMS[i].SMSC.Number[0] = 0;
		sms->SMS[i].SMSC.Number[1] = 0;
	}
	for (i=1;i<GSM_MAX_MULTI_SMS+1;i++) {
		if (i==1) {
			sprintf(buf, "SELECT Text,Coding,UDH,Class,TextDecoded,ID,DestinationNumber,MultiPart,RelativeValidity,DeliveryReport,CreatorID FROM `outbox` WHERE ID='%s'",ID);
		} else {
			sprintf(buf, "SELECT Text,Coding,UDH,Class,TextDecoded,ID,SequencePosition FROM `outbox_multipart` WHERE ID='%s' AND SequencePosition='%i'",ID,i);
		}
		if (SMSDMySQL_Store(Config, buf, &Res) != ERR_NONE) {
			SMSD_Log(0, Config, "Error reading from database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
			return ERR_UNKNOWN;
		}
		if (!(Row = mysql_fetch_row(Res))) {
	    		mysql_free_result(Res);
			return ERR_NONE;
		}

		sms->SMS[sms->Number].Coding=SMS_Coding_8bit;
		if (!strcmp(Row[1],"Unicode_No_Compression")) sms->SMS[sms->Number].Coding=SMS_Coding_Unicode_No_Compression;
		if (!strcmp(Row[1],"Default_No_Compression")) sms->SMS[sms->Number].Coding=SMS_Coding_Default_No_Compression;

		if (Row[0] == NULL || strlen(Row[0])==0) {
			SMSD_Log(1, Config, "Message: %s\n", Row[4]);
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

			if (!strcmp(Row[7],"false")) break;

		}
		if (SMSDMySQL_Query(Config, "UPDATE `phones` SET `Received`= `Received` + 1") != ERR_NONE) {
			SMSD_Log(0, Config, "Error updating number of received messages (%s)\n", __FUNCTION__);
			return ERR_UNKNOWN;
		}
		mysql_free_result(Res);
	}
  	return ERR_NONE;
}

/* After sending SMS is moved to Sent Items or Error Items. */
static GSM_Error SMSDMySQL_MoveSMS(GSM_MultiSMSMessage *sms UNUSED,
		GSM_SMSDConfig *Config, char *ID,
		bool alwaysDelete UNUSED, bool sent UNUSED)
{
	unsigned char buffer[10000];

	sprintf(buffer,"DELETE FROM `outbox` WHERE `ID` = '%s'",ID);
	if (SMSDMySQL_Query(Config, buffer) != ERR_NONE) {
		SMSD_Log(0, Config, "Error deleting from database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
		return ERR_UNKNOWN;
	}
	sprintf(buffer,"DELETE FROM `outbox_multipart` WHERE `ID` = '%s'",ID);
	if (SMSDMySQL_Query(Config, buffer) != ERR_NONE) {
		SMSD_Log(0, Config, "Error deleting from database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
		return ERR_UNKNOWN;
	}
  	return ERR_NONE;
}

/* Adds SMS to Outbox */
static GSM_Error SMSDMySQL_CreateOutboxSMS(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config)
{
	char			buffer[10000],buffer2[400],buffer4[10000],buffer5[400];
	int 			i,ID;
	MYSQL_RES 		*Res;
	MYSQL_ROW 		Row;

	sprintf(buffer,"SELECT ID FROM outbox ORDER BY ID DESC LIMIT 1");
	if (SMSDMySQL_Store(Config, buffer, &Res) != ERR_NONE) {
		SMSD_Log(0, Config, "Error reading from database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
		return ERR_UNKNOWN;
	}
	if ((Row = mysql_fetch_row(Res))) {
		sprintf(buffer,"%s",Row[0]);
		ID = atoi(buffer);
	} else {
		ID = 0;
	}
	mysql_free_result(Res);

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
		sprintf(buffer+strlen(buffer),"`Coding`,`UDH`, `Class`,`TextDecoded`,`ID`) VALUES (");
		if (i==0) {
			sprintf(buffer+strlen(buffer),"'Gammu %s',",VERSION);
			sprintf(buffer+strlen(buffer),"'%s','", Config->PhoneID);
			if (sms->SMS[i].PDU == SMS_Status_Report) {
				sprintf(buffer+strlen(buffer),"yes','");
			} else {
				sprintf(buffer+strlen(buffer),"default','");
			}
			if (sms->Number == 1) {
				sprintf(buffer+strlen(buffer),"false");
			} else {
				sprintf(buffer+strlen(buffer),"true");
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
		sprintf(buffer+strlen(buffer),"','");
		if (i==0) {
			while (true) {
				ID++;
				sprintf(buffer4,"SELECT ID FROM sentitems WHERE ID='%i'",ID);
				if (SMSDMySQL_Store(Config, buffer4, &Res) != ERR_NONE) {
					SMSD_Log(0, Config, "Error reading from database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
					return ERR_UNKNOWN;
				}
				if (!(Row = mysql_fetch_row(Res))) {
					buffer4[0] = 0;
					strcpy(buffer4,buffer);
					sprintf(buffer4+strlen(buffer4),"%i')",ID);
					if (SMSDMySQL_Query(Config, buffer4) != ERR_NONE) {
						if (mysql_errno(&Config->DBConnMySQL) == ER_DUP_ENTRY) {
							SMSD_Log(-1, Config, "Duplicated outgoing SMS ID\n");
							continue;
						}
						SMSD_Log(0, Config, "Error writing to database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
						return ERR_UNKNOWN;
					}
					mysql_free_result(Res);
					break;
				}
				mysql_free_result(Res);
			}
		} else {
			strcpy(buffer4,buffer);
			sprintf(buffer4+strlen(buffer4),"%i')",ID);
			if (SMSDMySQL_Query(Config, buffer4) != ERR_NONE) {
				SMSD_Log(0, Config, "Error writing to database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
				return ERR_UNKNOWN;
			}
		}
	}
	SMSD_Log(0, Config, "Written message with ID %i\n", ID);
  	return ERR_NONE;
}


static GSM_Error SMSDMySQL_AddSentSMSInfo(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *ID, int Part, GSM_SMSDSendingError err, int TPMR)
{
	unsigned char	buffer[10000],buffer2[400],buff[50],buffer5[400];

	if (err == SMSD_SEND_OK) {
		SMSD_Log(1, Config, "Transmitted %s (%s: %i) to %s", Config->SMSID,
			     (Part == sms->Number ? "total" : "part"), Part,
			     DecodeUnicodeString(sms->SMS[0].Number));
	}

	buff[0] = 0;
	if (err == SMSD_SEND_OK) {
		if (sms->SMS[Part-1].PDU == SMS_Status_Report) {
			sprintf(buff,"SendingOK");
		} else {
			sprintf(buff,"SendingOKNoReport");
		}
	}
	if (err == SMSD_SEND_SENDING_ERROR)
		sprintf(buff,"SendingError");
	if (err == SMSD_SEND_ERROR)
		sprintf(buff,"Error");

	buffer[0] = 0;
	sprintf(buffer+strlen(buffer),"INSERT INTO `sentitems` "
		"(`CreatorID`,`ID`,`SequencePosition`,`Status`,`SendingDateTime`, `SMSCNumber`, `TPMR`, "
		"`SenderID`,`Text`,`DestinationNumber`,`Coding`,`UDH`,`Class`,`TextDecoded`,`InsertIntoDB`,`RelativeValidity`) VALUES (");
	sprintf(buffer+strlen(buffer),"'%s','%s','%i','%s',NOW(),'%s','%i','%s','",Config->CreatorID,ID,Part,buff,DecodeUnicodeString(sms->SMS[Part-1].SMSC.Number),TPMR,Config->PhoneID);
	switch (sms->SMS[Part-1].Coding) {
	case SMS_Coding_Unicode_No_Compression:
    	case SMS_Coding_Default_No_Compression:
		EncodeHexUnicode(buffer+strlen(buffer),sms->SMS[Part-1].Text,UnicodeLength(sms->SMS[Part-1].Text));
		break;
	case SMS_Coding_8bit:
		EncodeHexBin(buffer+strlen(buffer),sms->SMS[Part-1].Text,sms->SMS[Part-1].Length);
	default:
		break;
	}
	sprintf(buffer+strlen(buffer),"','%s','",DecodeUnicodeString(sms->SMS[Part-1].Number));
	switch (sms->SMS[Part-1].Coding) {
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
	if (sms->SMS[Part-1].UDH.Type != UDH_NoUDH) {
		EncodeHexBin(buffer+strlen(buffer),sms->SMS[Part-1].UDH.Text,sms->SMS[Part-1].UDH.Length);
	}
	sprintf(buffer+strlen(buffer),"','%i','",sms->SMS[Part-1].Class);
	switch (sms->SMS[Part-1].Coding) {
	case SMS_Coding_Unicode_No_Compression:
	case SMS_Coding_Default_No_Compression:
		EncodeUTF8(buffer2, sms->SMS[Part-1].Text);
		mysql_real_escape_string(&Config->DBConnMySQL,buffer5,buffer2,strlen(buffer2));
		memcpy(buffer+strlen(buffer),buffer5,strlen(buffer5)+1);
		break;
	default:
		break;
	}
	sprintf(buffer+strlen(buffer),"','%s','",Config->DT);
	if (sms->SMS[Part-1].SMSC.Validity.Format == SMS_Validity_RelativeFormat) {
		sprintf(buffer+strlen(buffer),"%i')",sms->SMS[Part-1].SMSC.Validity.Relative);
	} else {
		sprintf(buffer+strlen(buffer),"-1')");
	}
	if (SMSDMySQL_Query(Config, buffer) != ERR_NONE) {
		SMSD_Log(0, Config, "Error writing to database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
		return ERR_UNKNOWN;
	}
	if (SMSDMySQL_Query(Config, "UPDATE `phones` SET `Sent`= `Sent` + 1") != ERR_NONE) {
		SMSD_Log(0, Config, "Error updating number of sent messages (%s)\n", __FUNCTION__);
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
		SMSD_Log(0, Config, "Error writing to database (%s): %s\n", __FUNCTION__, mysql_error(&Config->DBConnMySQL));
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
