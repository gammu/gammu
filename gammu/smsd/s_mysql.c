/* (c) 2004 by Marcin Wiacek */

#include "../../cfg/config.h"

#ifdef HAVE_MYSQL_MYSQL_H

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <mysql/mysql.h>

#include "../../common/misc/coding/coding.h"
#include "../../common/service/backup/gsmback.h"
#include "smsdcore.h"

MYSQL DB;

/* Connects to database */
static GSM_Error SMSDMySQL_Init(GSM_SMSDConfig *Config)
{
	mysql_init(&DB);
	if (!mysql_real_connect(&DB,Config->PC,Config->user,Config->password,Config->database,0,NULL,0)) {
	    WriteSMSDLog("Error connecting to database: %s\n", mysql_error(&DB));
	    return ERR_UNKNOWN;
	}
	return ERR_NONE;
}

/* Save SMS from phone (called Inbox sms - it's in phone Inbox) somewhere */
static GSM_Error SMSDMySQL_SaveInboxSMS(GSM_MultiSMSMessage sms, GSM_SMSDConfig *Config)
{
	unsigned char	buffer[10000];
	int 		i;

	for (i=0;i<sms.Number;i++) {
		if ((sms.SMS[i].PDU == SMS_Status_Report) && mystrncasecmp(Config->deliveryreport, "log", 3)) {
			strcpy(buffer, DecodeUnicodeString(sms.SMS[i].Number));
			WriteSMSDLog("Delivery report: %s to %s", DecodeUnicodeString(sms.SMS[i].Text), buffer);
			continue;
		}
		if (sms.SMS[i].PDU != SMS_Deliver) continue;
		buffer[0]=0;
		sprintf(buffer+strlen(buffer),"INSERT INTO `Inbox` \
			(`Date_Time`,`Text`,`Sender_Number`,`Coding`,`SMSC_Number`,`UDH`, \
			`Class`) VALUES ('%04d-%02d-%02d %02d:%02d:%02d','",
			sms.SMS[i].DateTime.Year,sms.SMS[i].DateTime.Month,sms.SMS[i].DateTime.Day,
			sms.SMS[i].DateTime.Hour,sms.SMS[i].DateTime.Minute,sms.SMS[i].DateTime.Second);
		switch (sms.SMS[i].Coding) {
		case SMS_Coding_Unicode:
	    	case SMS_Coding_Default:
			EncodeHexUnicode(buffer+strlen(buffer),sms.SMS[i].Text,UnicodeLength(sms.SMS[i].Text));
			break;
		case SMS_Coding_8bit:
			EncodeHexUnicode(buffer+strlen(buffer),sms.SMS[i].Text,sms.SMS[i].Length);
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
		sprintf(buffer+strlen(buffer),",'%i')",sms.SMS[i].Class);
		if (mysql_real_query(&DB,buffer,strlen(buffer))) {
			WriteSMSDLog("Error writing to database: %s\n", mysql_error(&DB));
			return ERR_UNKNOWN;
		}
	}

	return ERR_NONE;
}

GSM_SMSDService SMSDMySQL = {
	SMSDMySQL_Init,
	SMSDMySQL_SaveInboxSMS,
	NOTSUPPORTED,
	NOTSUPPORTED
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
