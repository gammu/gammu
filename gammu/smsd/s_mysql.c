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
#include "../gammu.h"
#include "smsdcore.h"

/* Connects to database */
static GSM_Error SMSDMySQL_Init(GSM_SMSDConfig *Config)
{
	unsigned char		buf[400];
	MYSQL_RES 		*Res;
	MYSQL_ROW 		Row;

	mysql_init(&Config->DB);
	if (!mysql_real_connect(&Config->DB,Config->PC,Config->user,Config->password,Config->database,0,NULL,0)) {
	    	WriteSMSDLog("Error connecting to database: %s\n", mysql_error(&Config->DB));
	    	return ERR_UNKNOWN;
	}
	sprintf(buf, "SELECT ID FROM `outbox` WHERE 1");
	if (mysql_real_query(&Config->DB,buf,strlen(buf))) {
		WriteSMSDLog("No table for outbox sms: %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	if (!(Res = mysql_store_result(&Config->DB))) {
		WriteSMSDLog("No table for outbox sms: %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	mysql_free_result(Res);
	sprintf(buf, "SELECT ID FROM `outbox_multipart` WHERE 1");
	if (mysql_real_query(&Config->DB,buf,strlen(buf))) {
		WriteSMSDLog("No table for outbox sms: %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	if (!(Res = mysql_store_result(&Config->DB))) {
		WriteSMSDLog("No table for outbox sms: %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	mysql_free_result(Res);
	sprintf(buf, "SELECT ID FROM `sentitems` WHERE 1");
	if (mysql_real_query(&Config->DB,buf,strlen(buf))) {
		WriteSMSDLog("No table for sent sms: %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	if (!(Res = mysql_store_result(&Config->DB))) {
		WriteSMSDLog("No table for sent sms: %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	mysql_free_result(Res);
	sprintf(buf, "SELECT ID FROM `inbox` WHERE 1");
	if (mysql_real_query(&Config->DB,buf,strlen(buf))) {
		WriteSMSDLog("No table for inbox sms: %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	if (!(Res = mysql_store_result(&Config->DB))) {
		WriteSMSDLog("No table for inbox sms: %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	mysql_free_result(Res);
	sprintf(buf, "SELECT Version FROM `gammu` WHERE 1");
	if (mysql_real_query(&Config->DB,buf,strlen(buf))) {
		WriteSMSDLog("No Gammu table: %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	if (!(Res = mysql_store_result(&Config->DB))) {
		WriteSMSDLog("No Gammu table: %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	if (!(Row = mysql_fetch_row(Res))) {
		mysql_free_result(Res);
		WriteSMSDLog("No version info in Gammu table: %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	if (atoi(Row[0]) > 4) {
		mysql_free_result(Res);
		WriteSMSDLog("DataBase structures are from higher Gammu version");
		WriteSMSDLog("Please update this client application");
		return ERR_UNKNOWN;
	}
	if (atoi(Row[0]) < 4) {
		mysql_free_result(Res);
		WriteSMSDLog("DataBase structures are from older Gammu version");
		WriteSMSDLog("Please update DataBase, if you want to use this client application");
		return ERR_UNKNOWN;
	}
	mysql_free_result(Res);

	return ERR_NONE;
}

static GSM_Error SMSDMySQL_InitAfterConnect(GSM_SMSDConfig *Config)
{
	unsigned char buf[400],buf2[200];

	sprintf(buf,"DELETE FROM `phones` WHERE `IMEI` = '%s'",s.Phone.Data.IMEI);
#ifdef DEBUG
	fprintf(stdout,"%s\n",buf);
#endif
	if (mysql_real_query(&Config->DB,buf,strlen(buf))) {
		WriteSMSDLog("Error deleting from database (Init): %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
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

	sprintf(buf,"INSERT INTO `phones` (`IMEI`,`ID`,`Send`,`Receive`,`InsertIntoDB`,`TimeOut`,`Client`) VALUES ('%s','%s','yes','yes',NOW(),(NOW() + INTERVAL 10 SECOND)+0,'%s')",s.Phone.Data.IMEI,Config->PhoneID,buf2);
#ifdef DEBUG
	fprintf(stdout,"%s\n",buf);
#endif
	if (mysql_real_query(&Config->DB,buf,strlen(buf))) {
		WriteSMSDLog("Error deleting from database (Init): %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}

	return ERR_NONE;
}

/* Save SMS from phone (called Inbox sms - it's in phone Inbox) somewhere */
static GSM_Error SMSDMySQL_SaveInboxSMS(GSM_MultiSMSMessage sms, GSM_SMSDConfig *Config)
{
	MYSQL_RES 		*Res;
	MYSQL_ROW 		Row;
	unsigned char		buffer[10000],buffer2[200],buffer3[50];
	int 			i,j,z;
	GSM_DateTime		DT;
	time_t     		t_time1,t_time2;
	bool			found;
	long 			diff;

	for (i=0;i<sms.Number;i++) {
		if (sms.SMS[i].PDU == SMS_Status_Report) {
			strcpy(buffer2, DecodeUnicodeString(sms.SMS[i].Number));
			if (mystrncasecmp(Config->deliveryreport, "log", 3)) {
				WriteSMSDLog("Delivery report: %s to %s", DecodeUnicodeString(sms.SMS[i].Text), buffer2);
			}

			sprintf(buffer, "SELECT ID,Status,SendingDateTime,DeliveryDateTime,SMSCNumber FROM `sentitems` WHERE \
					DeliveryDateTime='00000000000000' AND \
					SenderID='%s' AND TPMR='%i' AND DestinationNumber='%s'",
					Config->PhoneID, sms.SMS[i].MessageReference, buffer2);
#ifdef DEBUG
			fprintf(stdout,"%s\n",buffer);
#endif
			if (mysql_real_query(&Config->DB,buffer,strlen(buffer))) {
				WriteSMSDLog("Error reading from database (SaveInbox): %s %s\n", buffer, mysql_error(&Config->DB));
				return ERR_UNKNOWN;
			}
			if (!(Res = mysql_store_result(&Config->DB))) {
				WriteSMSDLog("Error reading from database (SaveInbox): %s\n", mysql_error(&Config->DB));
				return ERR_UNKNOWN;
			}
			found = false;
			while ((Row = mysql_fetch_row(Res))) {
				if (strcmp(Row[4],DecodeUnicodeString(sms.SMS[i].SMSC.Number))) {
					if (Config->skipsmscnumber[0] == 0) continue;
					if (strcmp(Config->skipsmscnumber,Row[4])) continue;
				}
				if (!strcmp(Row[1],"SendingOK") || !strcmp(Row[1],"DeliveryPending")) {
					sprintf(buffer,"%c%c%c%c",Row[2][0],Row[2][1],Row[2][2],Row[2][3]);
					DT.Year = atoi(buffer);
					sprintf(buffer,"%c%c",Row[2][4],Row[2][5]);
					DT.Month = atoi(buffer);
					sprintf(buffer,"%c%c",Row[2][6],Row[2][7]);
					DT.Day = atoi(buffer);
					sprintf(buffer,"%c%c",Row[2][8],Row[2][9]);
					DT.Hour = atoi(buffer);
					sprintf(buffer,"%c%c",Row[2][10],Row[2][11]);
					DT.Minute = atoi(buffer);
					sprintf(buffer,"%c%c",Row[2][12],Row[2][13]);
					DT.Second = atoi(buffer);
					t_time1 = Fill_Time_T(DT,0);
					t_time2 = Fill_Time_T(sms.SMS[i].DateTime,0);
					diff = t_time2 - t_time1;
	//				fprintf(stderr,"diff is %i, %i-%i-%i-%i-%i and %i-%i-%i-%i-%i-%i\n",diff,
	//					DT.Year,DT.Month,DT.Day,DT.Hour,DT.Minute,DT.Second,
	//					sms.SMS[i].DateTime.Year,sms.SMS[i].DateTime.Month,sms.SMS[i].DateTime.Day,sms.SMS[i].DateTime.Hour,sms.SMS[i].DateTime.Minute,sms.SMS[i].DateTime.Second);
					if (diff > -10 && diff < 10) {
						found = true;
						break;
					}
				}	
			}
			if (found) {
				sprintf(buffer,"UPDATE `sentitems` SET `DeliveryDateTime`='%04i%02i%02i%02i%02i%02i', `Status`='",
					sms.SMS[i].SMSCTime.Year,sms.SMS[i].SMSCTime.Month,sms.SMS[i].SMSCTime.Day,
					sms.SMS[i].SMSCTime.Hour,sms.SMS[i].SMSCTime.Minute,sms.SMS[i].SMSCTime.Second);
				sprintf(buffer3,"%s",DecodeUnicodeString(sms.SMS[i].Text));
				if (!strcmp(buffer3,"Delivered")) {
					sprintf(buffer+strlen(buffer),"DeliveryOK");
				} else if (!strcmp(buffer3,"Failed")) {
					sprintf(buffer+strlen(buffer),"DeliveryFailed");
				} else if (!strcmp(buffer3,"Pending")) {
					sprintf(buffer+strlen(buffer),"DeliveryPending");
				} else if (!strcmp(buffer3,"Unknown")) {
					sprintf(buffer+strlen(buffer),"DeliveryUnknown");
				}
				sprintf(buffer+strlen(buffer),"', `StatusError` = '%i'",sms.SMS[i].DeliveryStatus);
				sprintf(buffer+strlen(buffer)," WHERE `ID` = '%s' AND `TPMR` = '%i'",Row[0],sms.SMS[i].MessageReference);
#ifdef DEBUG
				fprintf(stdout,"%s\n",buffer);
#endif
				if (mysql_real_query(&Config->DB,buffer,strlen(buffer))) {
					WriteSMSDLog("Error writing to database (SaveInboxSMS): %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
					return ERR_UNKNOWN;
				}
			}
			mysql_free_result(Res);
			continue;
		}
		if (sms.SMS[i].PDU != SMS_Deliver) continue;
		buffer[0]=0;
		sprintf(buffer+strlen(buffer),"INSERT INTO `inbox` \
			(`ReceivingDateTime`,`Text`,`SenderNumber`,`Coding`,`SMSCNumber`,`UDH`, \
			`Class`,`TextDecoded`,`RecipientID`) VALUES ('%04d%02d%02d%02d%02d%02d','",
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
		sprintf(buffer+strlen(buffer),"','%s')",Config->PhoneID);
#ifdef DEBUG
		fprintf(stdout,"%s\n",buffer);
#endif
		if (mysql_real_query(&Config->DB,buffer,strlen(buffer))) {
			WriteSMSDLog("Error writing to database (SaveInbox): %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
			return ERR_UNKNOWN;
		}
	}

	return ERR_NONE;
}

static GSM_Error SMSDMySQL_RefreshSendStatus(GSM_SMSDConfig *Config, unsigned char *ID)
{
	unsigned char buffer[10000];

	sprintf(buffer,"UPDATE `outbox` SET `SendingTimeOut`=(now() + INTERVAL 15 SECOND)+0 WHERE `ID` = '%s' AND `SendingTimeOut` < now()",ID);
	if (mysql_real_query(&Config->DB,buffer,strlen(buffer))) {
		WriteSMSDLog("Error writing to database (RefreshSendStatus): %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
#ifdef DEBUG
	fprintf(stdout,"%s\n",buffer);
#endif	
	if (mysql_affected_rows(&Config->DB) == 0) return ERR_UNKNOWN;
  	return ERR_NONE;
}

/* Find one multi SMS to sending and return it (or return ERR_EMPTY)
 * There is also set ID for SMS
 */
static GSM_Error SMSDMySQL_FindOutboxSMS(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, unsigned char *ID)
{
	unsigned char 		buf[400];
	MYSQL_RES 		*Res;
	MYSQL_ROW 		Row;
	int 			i;
	bool			found = false;

	sprintf(buf, "SELECT ID,InsertIntoDB,SendingDateTime,SenderID FROM `outbox` WHERE SendingDateTime < NOW() AND SendingTimeOut < NOW()");
	if (mysql_real_query(&Config->DB,buf,strlen(buf))) {
		WriteSMSDLog("Error reading from database (FindOutbox): %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	if (!(Res = mysql_store_result(&Config->DB))) {
		WriteSMSDLog("Error reading from database (FindOutbox): %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	while ((Row = mysql_fetch_row(Res))) {
		sprintf(ID,"%s",Row[0]);
		sprintf(Config->DT,"%s",Row[1]);
		if (strlen(Row[3]) == 0 || !strcmp(Row[3],Config->PhoneID)) {
			if (SMSDMySQL_RefreshSendStatus(Config, ID)==ERR_NONE) {
				found = true;
				break;
			}
		}
	}
	if (!found) {
		mysql_free_result(Res);
		return ERR_EMPTY;
	}
	mysql_free_result(Res);
	sms->Number = 0;
	for (i=0;i<MAX_MULTI_SMS;i++) {
		GSM_SetDefaultSMSData(&sms->SMS[i]);
		sms->SMS[i].SMSC.Number[0] = 0;
		sms->SMS[i].SMSC.Number[1] = 0;
	}
	for (i=1;i<MAX_MULTI_SMS+1;i++) {
		if (i==1) {
			sprintf(buf, "SELECT Text,Coding,UDH,Class,TextDecoded,ID,DestinationNumber,MultiPart,RelativeValidity,DeliveryReport FROM `outbox` WHERE ID='%s'",ID);
		} else {
			sprintf(buf, "SELECT Text,Coding,UDH,Class,TextDecoded,ID,SequencePosition FROM `outbox_multipart` WHERE ID='%s' AND SequencePosition='%i'",ID,i);
		}
#ifdef DEBUG
		fprintf(stdout,"%s\n",buf);
#endif
    		if (mysql_real_query(&Config->DB,buf,strlen(buf))) {
			WriteSMSDLog("Error reading from database (FindOutbox): %s\n", mysql_error(&Config->DB));
			return ERR_UNKNOWN;
		}
		if (!(Res = mysql_store_result(&Config->DB))) {
			WriteSMSDLog("Error reading from database (FindOutbox): %s\n", mysql_error(&Config->DB));
			return ERR_UNKNOWN;
		}
		if (!(Row = mysql_fetch_row(Res))) {
	    		mysql_free_result(Res);
			return ERR_NONE;
		}

		sms->SMS[sms->Number].Coding=SMS_Coding_Default;
		if (!strcmp(Row[1],"Unicode")) sms->SMS[sms->Number].Coding=SMS_Coding_Unicode;
		if (!strcmp(Row[1],"8bit"))    sms->SMS[sms->Number].Coding=SMS_Coding_8bit;

		if (strlen(Row[0])==0) {
			EncodeUnicode(sms->SMS[sms->Number].Text,Row[4],strlen(Row[4]));
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
		if (i == 1) {
			EncodeUnicode(sms->SMS[sms->Number].Number,Row[6],strlen(Row[6]));
		} else {
			CopyUnicodeString(sms->SMS[sms->Number].Number,sms->SMS[0].Number);
		}
		sms->SMS[sms->Number].UDH.Type = UDH_NoUDH;
		if (strlen(Row[2])!=0) {
			sms->SMS[sms->Number].UDH.Type 		= UDH_UserUDH;
			sms->SMS[sms->Number].UDH.Length 	= strlen(Row[2])/2;
			DecodeHexBin(sms->SMS[sms->Number].UDH.Text,Row[2],strlen(Row[2]));
		}
		sms->SMS[sms->Number].Class		= atoi(Row[3]);
		sms->SMS[sms->Number].PDU  		= SMS_Submit;
		sms->Number++;
		if (i==1) {
			Config->relativevalidity = atoi(Row[8]);

			Config->currdeliveryreport = -1;
			if (!strcmp(Row[9],"yes")) {
				Config->currdeliveryreport = 1;
			} else if (!strcmp(Row[9],"no")) {
				Config->currdeliveryreport = 0;
			}

			if (!strcmp(Row[7],"false")) break;

		}
	}
	mysql_free_result(Res);	
  	return ERR_NONE;
}

/* After sending SMS is moved to Sent Items or Error Items. */
static GSM_Error SMSDMySQL_MoveSMS(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, unsigned char *ID, bool alwaysDelete, bool sent)
{
	unsigned char buffer[10000];

	sprintf(buffer,"DELETE FROM `outbox` WHERE `ID` = '%s'",ID);
	if (mysql_real_query(&Config->DB,buffer,strlen(buffer))) {
		WriteSMSDLog("Error deleting from database (MoveSMS): %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	sprintf(buffer,"DELETE FROM `outbox_multipart` WHERE `ID` = '%s'",ID);
	if (mysql_real_query(&Config->DB,buffer,strlen(buffer))) {
		WriteSMSDLog("Error deleting from database (MoveSMS): %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
  	return ERR_NONE;
}

/* Adds SMS to Outbox */
static GSM_Error SMSDMySQL_CreateOutboxSMS(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config)
{
	unsigned char		buffer[10000],buffer2[200],buffer3[2],buffer4[10000];
	int 			i,j,z,ID;
	MYSQL_RES 		*Res;
	MYSQL_ROW 		Row;

	sprintf(buffer,"SELECT ID FROM outbox ORDER BY ID DESC LIMIT 1");
	if (mysql_real_query(&Config->DB,buffer,strlen(buffer))) {
		WriteSMSDLog("Error reading from database (CreateOutbox): %s\n", mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	if (!(Res = mysql_store_result(&Config->DB))) {
		WriteSMSDLog("Error reading from database (CreateOutbox): %s\n", mysql_error(&Config->DB));
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
			sprintf(buffer+strlen(buffer),"INSERT INTO `outbox` (`DeliveryReport`,`MultiPart`,`InsertIntoDB");
		} else {
			sprintf(buffer+strlen(buffer),"INSERT INTO `outbox_multipart` (`SequencePosition");
		}
		sprintf(buffer+strlen(buffer),"`,`Text`,");
		if (i==0) {
			sprintf(buffer+strlen(buffer),"`DestinationNumber`,`RelativeValidity`,");
		}
		sprintf(buffer+strlen(buffer),"`Coding`,`UDH`, \
			`Class`,`TextDecoded`,`ID`) VALUES ('");
		if (i==0) {
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
		sprintf(buffer+strlen(buffer),"','");
		if (sms->SMS[i].UDH.Type != UDH_NoUDH) {
			EncodeHexBin(buffer+strlen(buffer),sms->SMS[i].UDH.Text,sms->SMS[i].UDH.Length);
		}
		sprintf(buffer+strlen(buffer),"','%i','",sms->SMS[i].Class);
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
				ID++;
				sprintf(buffer4,"SELECT ID FROM sentitems WHERE ID='%i'",ID);
#ifdef DEBUG
				fprintf(stdout,"%s\n",buffer4);
#endif
				if (mysql_real_query(&Config->DB,buffer4,strlen(buffer4))) {
					WriteSMSDLog("Error reading from database (CreateOutbox): %s\n", mysql_error(&Config->DB));
					return ERR_UNKNOWN;
				}
				if (!(Res = mysql_store_result(&Config->DB))) {
					WriteSMSDLog("Error reading from database (CreateOutbox): %s\n", mysql_error(&Config->DB));
					return ERR_UNKNOWN;
				}
				if (!(Row = mysql_fetch_row(Res))) {
					buffer4[0] = 0;
					strcpy(buffer4,buffer);
					sprintf(buffer4+strlen(buffer4),"%i')",ID);
#ifdef DEBUG
					fprintf(stdout,"%s\n",buffer4);
#endif
					if (mysql_real_query(&Config->DB,buffer4,strlen(buffer4))) {
						if (mysql_errno(&Config->DB) == ER_DUP_ENTRY) {
							WriteSMSDLog("Duplicated outgoing SMS ID\n");
							continue;
						}
						WriteSMSDLog("Error writing to database (CreateOutbox): %d %s %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB),buffer4);
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
#ifdef DEBUG
			fprintf(stdout,"%s\n",buffer4);
#endif
			if (mysql_real_query(&Config->DB,buffer4,strlen(buffer4))) {
				WriteSMSDLog("Error writing to database (CreateOutbox): %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
				return ERR_UNKNOWN;
			}
		}
	}
  	return ERR_NONE;
}


static GSM_Error SMSDMySQL_AddSentSMSInfo(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, unsigned char *ID, int Part, GSM_SMSDSendingError err, int TPMR)
{
	unsigned char	buffer[10000],buffer2[200],buffer3[2],buff[50];
	int 		j,z;
	
	if (err == SMSD_SEND_OK) WriteSMSDLog("Transmitted %s (%s: %i) to %s", Config->SMSID, (Part == sms->Number?"total":"part"),Part,DecodeUnicodeString(sms->SMS[0].Number));

	buff[0] = 0;
	if (err == SMSD_SEND_OK) {
		if (sms->SMS[Part-1].PDU == SMS_Status_Report) {
			sprintf(buff,"SendingOK");
		} else {
			sprintf(buff,"SendingOKNoReport");
		}
	}
	if (err == SMSD_SEND_SENDING_ERROR) 	sprintf(buff,"SendingError");
	if (err == SMSD_SEND_ERROR) 		sprintf(buff,"Error");

	buffer[0] = 0;
	sprintf(buffer+strlen(buffer),"INSERT INTO `sentitems` \
		(`ID`,`SequencePosition`,`Status`,`SendingDateTime`, `SMSCNumber`, `TPMR`, \
		`SenderID`,`Text`,`DestinationNumber`,`Coding`,`UDH`,`Class`,`TextDecoded`,`InsertIntoDB`,`RelativeValidity`) VALUES (");
	sprintf(buffer+strlen(buffer),"'%s','%i','%s',NOW(),'%s','%i','%s','",ID,Part,buff,DecodeUnicodeString(sms->SMS[Part-1].SMSC.Number),TPMR,Config->PhoneID);
	switch (sms->SMS[Part-1].Coding) {
	case SMS_Coding_Unicode:
    	case SMS_Coding_Default:
		EncodeHexUnicode(buffer+strlen(buffer),sms->SMS[Part-1].Text,UnicodeLength(sms->SMS[Part-1].Text));
		break;
	case SMS_Coding_8bit:
		EncodeHexBin(buffer+strlen(buffer),sms->SMS[Part-1].Text,sms->SMS[Part-1].Length);
	}
	sprintf(buffer+strlen(buffer),"','%s','",DecodeUnicodeString(sms->SMS[Part-1].Number));
	switch (sms->SMS[Part-1].Coding) {
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
	sprintf(buffer+strlen(buffer),"','");
	if (sms->SMS[Part-1].UDH.Type != UDH_NoUDH) {
		EncodeHexBin(buffer+strlen(buffer),sms->SMS[Part-1].UDH.Text,sms->SMS[Part-1].UDH.Length);
	}
	sprintf(buffer+strlen(buffer),"','%i','",sms->SMS[Part-1].Class);
	switch (sms->SMS[Part-1].Coding) {
	case SMS_Coding_Unicode:
	case SMS_Coding_Default:
		sprintf(buffer2,"%s",DecodeUnicodeString(sms->SMS[Part-1].Text));
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
	sprintf(buffer+strlen(buffer),"','%s','",Config->DT);
	if (sms->SMS[Part-1].SMSC.Validity.Format == SMS_Validity_RelativeFormat) {
		sprintf(buffer+strlen(buffer),"%i')",sms->SMS[Part-1].SMSC.Validity.Relative);
	} else {
		sprintf(buffer+strlen(buffer),"-1')");
	}
#ifdef DEBUG
	fprintf(stdout,"%s\n",buffer);
#endif	
	if (mysql_real_query(&Config->DB,buffer,strlen(buffer))) {
		WriteSMSDLog("Error writing to database (AddSent): %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
  	return ERR_NONE;
}

static GSM_Error SMSDMySQL_RefreshPhoneStatus(GSM_SMSDConfig *Config)
{
	unsigned char buffer[500];

	sprintf(buffer,"UPDATE `phones` SET `TimeOut`= (NOW() + INTERVAL 10 SECOND)+0");
	sprintf(buffer+strlen(buffer)," WHERE `IMEI` = '%s'",s.Phone.Data.IMEI);
#ifdef DEBUG
	fprintf(stdout,"%s\n",buffer);
#endif
	if (mysql_real_query(&Config->DB,buffer,strlen(buffer))) {
		WriteSMSDLog("Error writing to database (SaveInboxSMS): %d %s\n", mysql_errno(&Config->DB), mysql_error(&Config->DB));
		return ERR_UNKNOWN;
	}
	return ERR_NONE;
}

GSM_SMSDService SMSDMySQL = {
	SMSDMySQL_Init,
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
