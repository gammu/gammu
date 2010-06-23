/* (c) 2004 by Marcin Wiacek */

#include "../../cfg/config.h"

#ifdef HAVE_MYSQL_MYSQL_H

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#ifdef WIN32
#  include <windows.h>
#  pragma comment(lib, "libmysql.lib")
#endif

#include "../../common/misc/coding/coding.h"
#include "../../common/service/backup/gsmback.h"
#include "smsdcore.h"

/* Connects to database */
static GSM_Error SMSDMySQL_Init(GSM_SMSDConfig *Config)
{
	mysql_init(&Config->DB);
	if (!mysql_real_connect(&Config->DB,Config->PC,Config->user,Config->password,Config->database,0,NULL,0)) {
	    WriteSMSDLog("Error connecting to database: %s\n", mysql_error(&Config->DB));
	    return ERR_UNKNOWN;
	}
	return ERR_NONE;
}

/* Save SMS from phone (called Inbox sms - it's in phone Inbox) somewhere */
static GSM_Error SMSDMySQL_SaveInboxSMS(GSM_MultiSMSMessage sms, GSM_SMSDConfig *Config)
{
	unsigned char	buffer[10000],buffer2[200],buffer3[2];
	int 		i,j,z;

	for (i=0;i<sms.Number;i++) {
		if ((sms.SMS[i].PDU == SMS_Status_Report) && mystrncasecmp(Config->deliveryreport, "log", 3)) {
			strcpy(buffer, DecodeUnicodeString(sms.SMS[i].Number));
			WriteSMSDLog("Delivery report: %s to %s", DecodeUnicodeString(sms.SMS[i].Text), buffer);
			continue;
		}
		if (sms.SMS[i].PDU != SMS_Deliver) continue;
		buffer[0]=0;
		sprintf(buffer+strlen(buffer),"INSERT INTO `Inbox` \
			(`DateTime`,`Text`,`SenderNumber`,`Coding`,`SMSCNumber`,`UDH`, \
			`Class`,`TextDecoded`) VALUES ('%04d%02d%02d%02d%02d%02d','",
			sms.SMS[i].DateTime.Year,sms.SMS[i].DateTime.Month,sms.SMS[i].DateTime.Day,
			sms.SMS[i].DateTime.Hour,sms.SMS[i].DateTime.Minute,sms.SMS[i].DateTime.Second);
		switch (sms.SMS[i].Coding) {
		case SMS_Coding_Unicode:
	    	case SMS_Coding_Default:
			EncodeHexUnicode(buffer+strlen(buffer),sms.SMS[i].Text,UnicodeLength(sms.SMS[i].Text));
			break;
		case SMS_Coding_8bit:
			EncodeHexBin(buffer+strlen(buffer),sms.SMS[i].Text,sms.SMS[i].Length);
		}
		sprintf(buffer+strlen(buffer),"','%s','",DecodeUnicodeString(sms.SMS[i].Number));
		switch (sms.SMS[i].Coding) {
		case SMS_Coding_Unicode:
			sprintf(buffer+strlen(buffer),"Unicode");
			break;
	    	case SMS_Coding_Default:
			sprintf(buffer+strlen(buffer),"Default");
			break;
		case SMS_Coding_8bit:
			sprintf(buffer+strlen(buffer),"8bit");
			break;
		}
		sprintf(buffer+strlen(buffer),"','%s'",DecodeUnicodeString(sms.SMS[i].SMSC.Number));
		if (sms.SMS[i].UDH.Type == UDH_NoUDH) {
			sprintf(buffer+strlen(buffer),",''");
		} else {
			sprintf(buffer+strlen(buffer),",'");
			EncodeHexBin(buffer+strlen(buffer),sms.SMS[i].UDH.Text,sms.SMS[i].UDH.Length);
			sprintf(buffer+strlen(buffer),"'");
		}
		sprintf(buffer+strlen(buffer),",'%i','",sms.SMS[i].Class);
		switch (sms.SMS[i].Coding) {
		case SMS_Coding_Unicode:
	    	case SMS_Coding_Default:
			sprintf(buffer2,"%s",DecodeUnicodeString(sms.SMS[i].Text));
			for (j=0;j<(int)strlen(buffer2);j++) {
				sprintf(buffer3,"'");
				z = strlen(buffer);
				if (buffer2[j]==buffer3[0]) {				    
				    buffer[z+2]=0;
				    buffer[z+1]=buffer2[j];
				    buffer[z]  ='\\';
				} else {
				    buffer[z+1]=0;
				    buffer[z]  =buffer2[j];
				}
			}
			break;
		case SMS_Coding_8bit:
			break;
		}
		sprintf(buffer+strlen(buffer),"')");
		if (mysql_real_query(&Config->DB,buffer,strlen(buffer))) {
			WriteSMSDLog("Error writing to database: %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
			return ERR_UNKNOWN;
		}
	}

	return ERR_NONE;
}

/* Find one multi SMS to sending and return it (or return ERR_EMPTY)
 * There is also set ID for SMS
 */
static GSM_Error SMSDMySQL_FindOutboxSMS(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, unsigned char *ID)
{
	unsigned char 	buf[400];
	MYSQL_RES 	*Res;
	MYSQL_ROW 	Row;
	int 		i;

	sprintf(buf, "SELECT ID,DateTime FROM `Outbox` WHERE 1");
	if (mysql_real_query(&Config->DB,buf,strlen(buf))) {
		WriteSMSDLog("Error reading from database: %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	if (!(Res = mysql_store_result(&Config->DB))) {
		WriteSMSDLog("Error reading from database: %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	if (!(Row = mysql_fetch_row(Res))) {
		mysql_free_result(Res);
		return ERR_EMPTY;
	}
	sprintf(ID,"%s",Row[0]);
	sprintf(Config->DT,"%s",Row[1]);
	sms->Number = 0;
	for (i=1;i<10;i++) {
		GSM_SetDefaultSMSData(&sms->SMS[sms->Number]);
		if (i==1) {
			sprintf(buf, "SELECT Text,DestinationNumber,Coding,UDH,SMSCNumber,Class,TextDecoded,ID,MultiPart FROM `Outbox` WHERE ID='%s'",ID);
		} else {
			sprintf(buf, "SELECT Text,DestinationNumber,Coding,UDH,SMSCNumber,Class,TextDecoded,ID,SequencePosition FROM `Outbox_MultiPart` WHERE ID='%s' AND SequencePosition='%i'",ID,i);
		}
    		if (mysql_real_query(&Config->DB,buf,strlen(buf))) {
			WriteSMSDLog("Error reading from database: %s\n", mysql_error(&Config->DB));
			return ERR_UNKNOWN;
		}
		if (!(Res = mysql_store_result(&Config->DB))) {
			WriteSMSDLog("Error reading from database: %s\n", mysql_error(&Config->DB));
			return ERR_UNKNOWN;
		}
		if (!(Row = mysql_fetch_row(Res))) {
	    		mysql_free_result(Res);
			return ERR_NONE;
		}

		sms->SMS[sms->Number].Coding=SMS_Coding_Default;
		if (!strcmp(Row[2],"Unicode")) sms->SMS[sms->Number].Coding=SMS_Coding_Unicode;
		if (!strcmp(Row[2],"8bit"))    sms->SMS[sms->Number].Coding=SMS_Coding_8bit;

		if (strlen(Row[0])==0) {
			EncodeUnicode(sms->SMS[sms->Number].Text,Row[6],strlen(Row[6]));
		} else {
			switch (sms->SMS[sms->Number].Coding) {
			case SMS_Coding_Unicode:
			case SMS_Coding_Default:
				DecodeHexUnicode(sms->SMS[sms->Number].Text,Row[0],strlen(Row[0]));
				break;
			case SMS_Coding_8bit:
				DecodeHexBin(sms->SMS[sms->Number].Text,Row[0],strlen(Row[0]));
				sms->SMS[sms->Number].Length=strlen(Row[0])/2;
			}
		}	    
		EncodeUnicode(sms->SMS[sms->Number].Number,Row[1],strlen(Row[1]));
		sms->SMS[sms->Number].UDH.Type = UDH_NoUDH;
		if (strlen(Row[3])!=0) {
			sms->SMS[sms->Number].UDH.Type 		= UDH_UserUDH;
			sms->SMS[sms->Number].UDH.Length 	= strlen(Row[3])/2;
			DecodeHexBin(sms->SMS[sms->Number].UDH.Text,Row[3],strlen(Row[3]));
		}
		EncodeUnicode(sms->SMS[sms->Number].SMSC.Number,Row[4],strlen(Row[4]));
		sms->SMS[sms->Number].Class		= atoi(Row[5]);
		sms->SMS[sms->Number].PDU  		= SMS_Submit;
		sms->SMS[sms->Number].SMSC.Location = 0;
		sms->Number++;	    
		if (i == 1 && !strcmp(Row[8],"false")) break;
	}
  	return ERR_NONE;
}

/* After sending SMS is moved to Sent Items or Error Items. */
static GSM_Error SMSDMySQL_MoveSMS(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, unsigned char *ID, bool alwaysDelete, bool sent)
{
	char 		*status;
	unsigned char	buffer[10000],buffer2[200],buffer3[2];
	int 		i,j,z;
	
	if (sent) status = "OK"; else status = "Error";

	for (i=0;i<sms->Number;i++) {
		buffer[0]=0;
		sprintf(buffer+strlen(buffer),"UPDATE `SentItems` SET `Text`='");

		switch (sms->SMS[i].Coding) {
		case SMS_Coding_Unicode:
	    	case SMS_Coding_Default:
			EncodeHexUnicode(buffer+strlen(buffer),sms->SMS[i].Text,UnicodeLength(sms->SMS[i].Text));
			break;
		case SMS_Coding_8bit:
			EncodeHexBin(buffer+strlen(buffer),sms->SMS[i].Text,sms->SMS[i].Length);
		}

		sprintf(buffer+strlen(buffer),"',`DestinationNumber`='%s',`Coding`='",DecodeUnicodeString(sms->SMS[i].Number));

		switch (sms->SMS[i].Coding) {
		case SMS_Coding_Unicode:
			sprintf(buffer+strlen(buffer),"Unicode");
			break;
	    	case SMS_Coding_Default:
			sprintf(buffer+strlen(buffer),"Default");
			break;
		case SMS_Coding_8bit:
			sprintf(buffer+strlen(buffer),"8bit");
			break;
		}

		sprintf(buffer+strlen(buffer),"',`SMSCNumber`='%s',`UDH`='",DecodeUnicodeString(sms->SMS[i].SMSC.Number));
		if (sms->SMS[i].UDH.Type != UDH_NoUDH) {
			EncodeHexBin(buffer+strlen(buffer),sms->SMS[i].UDH.Text,sms->SMS[i].UDH.Length);
		}

		sprintf(buffer+strlen(buffer),"',`Class`='%i',`TextDecoded`='",sms->SMS[i].Class);
		switch (sms->SMS[i].Coding) {
		case SMS_Coding_Unicode:
	    	case SMS_Coding_Default:
			sprintf(buffer2,"%s",DecodeUnicodeString(sms->SMS[i].Text));
			for (j=0;j<(int)strlen(buffer2);j++) {
				sprintf(buffer3,"'");
				z = strlen(buffer);
				if (buffer2[j]==buffer3[0]) {				    
				    buffer[z+2]=0;
				    buffer[z+1]=buffer2[j];
				    buffer[z]  ='\\';
				} else {
				    buffer[z+1]=0;
				    buffer[z]  =buffer2[j];
				}
			}
			break;
		case SMS_Coding_8bit:
			break;
		}
		sprintf(buffer+strlen(buffer),"',`Status`='%s',`DateTime`='%s' ",status,Config->DT);
		sprintf(buffer+strlen(buffer),"WHERE `ID`='%s' AND `SequencePosition`='%i'",ID,i+1);
		if (mysql_real_query(&Config->DB,buffer,strlen(buffer))) {
			WriteSMSDLog("Error writing to database: %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
			return ERR_UNKNOWN;
		}
	}
	sprintf(buffer,"DELETE FROM `Outbox` WHERE `ID` = '%s'",ID);
	if (mysql_real_query(&Config->DB,buffer,strlen(buffer))) {
		WriteSMSDLog("Error writing to database: %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	sprintf(buffer,"DELETE FROM `Outbox_MultiPart` WHERE `ID` = '%s'",ID);
	if (mysql_real_query(&Config->DB,buffer,strlen(buffer))) {
		WriteSMSDLog("Error writing to database: %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
  	return ERR_NONE;
}

/* Adds SMS to Outbox */
static GSM_Error SMSDMySQL_CreateOutboxSMS(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config)
{
	unsigned char	buffer[10000],buffer2[200],buffer3[2],buffer4[10000];
	int 		i,j,z;
	GSM_DateTime	time;

	for (i=0;i<sms->Number;i++) {
		buffer[0]=0;
		if (i==0) {
			sprintf(buffer+strlen(buffer),"INSERT INTO `Outbox` (`MultiPart`,`DateTime");
		} else {
			sprintf(buffer+strlen(buffer),"INSERT INTO `Outbox_MultiPart` (`SequencePosition");
		}
		sprintf(buffer+strlen(buffer),"`,`Text`,`DestinationNumber`,`Coding`,`SMSCNumber`,`UDH`, \
			`Class`,`TextDecoded`,`ID`) VALUES ('");
		if (i==0) {
			if (sms->Number == 1) {
				sprintf(buffer+strlen(buffer),"false");
			} else {
				sprintf(buffer+strlen(buffer),"true");
			}
			sprintf(buffer+strlen(buffer),"',NOW()");
		} else {
			sprintf(buffer+strlen(buffer),"%i'",i+1);
		}
		sprintf(buffer+strlen(buffer),",'");

		switch (sms->SMS[i].Coding) {
		case SMS_Coding_Unicode:
	    	case SMS_Coding_Default:
			EncodeHexUnicode(buffer+strlen(buffer),sms->SMS[i].Text,UnicodeLength(sms->SMS[i].Text));
			break;
		case SMS_Coding_8bit:
			EncodeHexBin(buffer+strlen(buffer),sms->SMS[i].Text,sms->SMS[i].Length);
		}
		sprintf(buffer+strlen(buffer),"','%s','",DecodeUnicodeString(sms->SMS[i].Number));
		switch (sms->SMS[i].Coding) {
		case SMS_Coding_Unicode:
			sprintf(buffer+strlen(buffer),"Unicode");
			break;
	    	case SMS_Coding_Default:
			sprintf(buffer+strlen(buffer),"Default");
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
		case SMS_Coding_Unicode:
	    	case SMS_Coding_Default:
			sprintf(buffer2,"%s",DecodeUnicodeString(sms->SMS[i].Text));
			for (j=0;j<(int)strlen(buffer2);j++) {
				sprintf(buffer3,"'");
				z = strlen(buffer);
				if (buffer2[j]==buffer3[0]) {				    
				    buffer[z+2]=0;
				    buffer[z+1]=buffer2[j];
				    buffer[z]  ='\\';
				} else {
				    buffer[z+1]=0;
				    buffer[z]  =buffer2[j];
				}
			}
			break;
		case SMS_Coding_8bit:

			break;
		}
		sprintf(buffer+strlen(buffer),"','");
		if (i==0) {
			while (true) {
				GSM_GetCurrentDateTime(&time);
				buffer4[0] = 0;
				strcpy(buffer4,buffer);
				sprintf(buffer4+strlen(buffer4),"%i')",
					time.Hour*1000000+
					time.Minute*10000+
					time.Second*100+time.Day);
				if (mysql_real_query(&Config->DB,buffer4,strlen(buffer4))) {
					if (mysql_errno(&Config->DB) == ER_DUP_ENTRY) {
						WriteSMSDLog("Duplicated outgoing SMS ID\n");
						continue;
					}
					WriteSMSDLog("Error writing to database: %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
					return ERR_UNKNOWN;
				}
				break;
			}
		} else {
			strcpy(buffer4,buffer);
			sprintf(buffer4+strlen(buffer4),"%i')",
				time.Hour*1000000+
				time.Minute*10000+
				time.Second*100+time.Day);
			if (mysql_real_query(&Config->DB,buffer4,strlen(buffer4))) {
				WriteSMSDLog("Error writing to database: %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
				return ERR_UNKNOWN;
			}
		}
	}
  	return ERR_NONE;
}

static GSM_Error SMSDMySQL_AddSentSMSInfo(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, unsigned char *ID, int Part, bool OK)
{
	unsigned char buffer[10000];

	if (OK) WriteSMSDLog("Transmitted %s (%s: %i) to %s", Config->SMSID, (Part == sms->Number?"total":"part"),Part,DecodeUnicodeString(sms->SMS[0].Number));

	buffer[0] = 0;
	sprintf(buffer+strlen(buffer),"INSERT INTO `SentItems` \
		(`ID`,`SequencePosition`,`Status`,`SendingDateTime`) VALUES (");
	sprintf(buffer+strlen(buffer),"'%s','%i','Sending',NOW())",ID,Part);
	if (mysql_real_query(&Config->DB,buffer,strlen(buffer))) {
		WriteSMSDLog("Error writing to database: %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
  	return ERR_NONE;
}

GSM_SMSDService SMSDMySQL = {
	SMSDMySQL_Init,
	SMSDMySQL_SaveInboxSMS,
	SMSDMySQL_FindOutboxSMS,
	SMSDMySQL_MoveSMS,
	SMSDMySQL_CreateOutboxSMS,
	SMSDMySQL_AddSentSMSInfo
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
