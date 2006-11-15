/* (c) 2006 by Marcin Wiacek */

#include <string.h>
#include <time.h>

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "../../misc/coding/coding.h"
#include "../../service/gsmmisc.h"
#include "../nokia/nfunc.h"
#include "../pfunc.h"

#ifdef GSM_ENABLE_GNAPGEN

unsigned char GNAPGEN_MEMORY_TYPES[] = {
	MEM_SM,		 0x01,
	MEM_ON,		 0x03,
	MEM_DC,		 0x05,
	MEM_RC,		 0x06,
	MEM_MC,		 0x07,
	  0x00,		 0x00
};

static GSM_Error GNAPGEN_PrivGetSMSFolderStatus(GSM_StateMachine *s, int folderid)
{
	unsigned char req[] = {0, 3,
			       0,0x0c};		/* folderID c,d,e,f...*/

	smprintf(s, "Getting SMS folder status\n");
	return GSM_WaitFor (s, req, 4, 6, 4, ID_GetSMSFolderStatus);
}

static GSM_Error GNAPGEN_GetNextSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	unsigned char req [8] = {0,11,
				 0x00,0x0c, 		//folder
				 0x00,0x10,0x00,0x3F};	//location

	GNAPGEN_PrivGetSMSFolderStatus(s,0x0c);

	return GSM_WaitFor (s, req, 8, 0x6, 500, ID_SaveSMSMessage);
}

static GSM_Error GNAPGEN_EncodeSMSFrame(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char *req, GSM_SMSMessageLayout *Layout, int *length)
{
	int			start, count = 0;
//	int l,i,pos4;
	GSM_Error		error;
//	unsigned char 		l0;
//	unsigned char		req0[256];

	memset(Layout,255,sizeof(GSM_SMSMessageLayout));

	start = *length;

	/* SMSC number */
//	pos5 			 = count; count++;
	Layout->SMSCNumber 	 = count; count += 8;

	/* firstbyte set in SMS Layout */
	Layout->firstbyte 	 = count; count++;

	if (sms->PDU != SMS_Deliver) {
		Layout->TPMR 	 = count; count++;
	}

	/* Phone number */
//	pos4 			 = count; count++;
	Layout->Number 		 = count; count+= 8;

	Layout->TPPID	 = count; count++;
	Layout->TPDCS 	 = count; count++;
	if (sms->PDU == SMS_Deliver) {
		Layout->DateTime = count; count += 7;
	} else {
		Layout->TPVP 	 = count; count++;
	}
	Layout->TPUDL 		 = count; count++;
	Layout->Text 		 = count;

	error = PHONE_EncodeSMSFrame(s,sms,req,*Layout,length,false);
	if (error != ERR_NONE) return error;


req[0] = 0x0b;
req[10] = 0x07;

	return ERR_NONE;
}

static GSM_Error GNAPGEN_SendSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	int			length = 11;
	GSM_Error		error;
	GSM_SMSMessageLayout 	Layout;
	unsigned char req [300] = {0,15};

	if (sms->PDU == SMS_Deliver) sms->PDU = SMS_Submit;
	memset(req+2,0x00,sizeof(req) - 2);
	error=GNAPGEN_EncodeSMSFrame(s, sms, req + 2, &Layout, &length);
	if (error != ERR_NONE) return error;
	DumpMessage(&s->di, req, length+1);
	//return ERR_NONE;
	smprintf(s, "Sending sms\n");
	return s->Protocol.Functions->WriteMessage(s, req, length + 2, 0x06);
}

static GSM_Error GNAPGEN_PrivSetSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	int			length = 0;
	GSM_SMSMessageLayout 	Layout;
	GSM_Error		error;

	unsigned char req [300] = {0,13,
				0x00,0x0c, //folder
				0x00,0x10,0x00,0x3F};//location

	if (sms->PDU == SMS_Deliver) sms->PDU = SMS_Submit;
	memset(req+8,0x00,sizeof(req) - 8);
	error=GNAPGEN_EncodeSMSFrame(s, sms, req + 8, &Layout, &length);

	s->Phone.Data.SaveSMSMessage=sms;
	smprintf(s, "Saving sms\n");
	return GSM_WaitFor (s, req, length+8, 0x6, 4, ID_SaveSMSMessage);
}

static GSM_Error GNAPGEN_AddSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
//	N6510_GetSMSLocation(s, sms, &folderid, &location);
//	location = 0;
//	N6510_SetSMSLocation(s, sms, folderid, location);
	return GNAPGEN_PrivSetSMSMessage(s, sms);
}

//-----------------------------------------------------------------------------

static GSM_Error GNAPGEN_ReplyGetSignalQuality(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Network level received: %i\n",msg.Buffer[4]);
    	Data->SignalQuality->SignalStrength 	= -1;
    	Data->SignalQuality->SignalPercent 	= ((int)msg.Buffer[4]);
    	Data->SignalQuality->BitErrorRate 	= -1;
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetSignalQuality(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	unsigned char req[] = {0x00,0x03};

	s->Phone.Data.SignalQuality = sig;
	smprintf(s, "Getting network level\n");
	return GSM_WaitFor (s, req, 2, 0x03, 4, ID_GetSignalQuality);
}

static GSM_Error GNAPGEN_ReplyGetBatteryCharge(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Battery level received: %i\n",msg.Buffer[4]);
    	Data->BatteryCharge->BatteryPercent 	= ((int)(msg.Buffer[4]));
    	Data->BatteryCharge->ChargeState 	= 0;
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	unsigned char req[] = {0x00, 0x01};

	s->Phone.Data.BatteryCharge = bat;
	smprintf(s, "Getting battery level\n");
	return GSM_WaitFor (s, req, 2, 0x04, 4, ID_GetBatteryCharge);
}

static GSM_Error GNAPGEN_ReplyGetNetworkInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data	*Data = &s->Phone.Data;
	char		buf[100];
#ifdef DEBUG
	GSM_NetworkInfo NetInfo;

	smprintf(s, "Network status            : ");
	switch (msg.Buffer[9]) {
		case 0x00 : smprintf(s, "home network ?\n");		break;
		default	  : smprintf(s, "unknown %i!\n",msg.Buffer[9]); break;
	}
	if (msg.Buffer[9]==0x00) {
		sprintf(NetInfo.CID, "%02X%02X", msg.Buffer[4], msg.Buffer[5]);
		smprintf(s, "CID                       : %s\n", NetInfo.CID);
		sprintf(NetInfo.LAC, "%02X%02X", msg.Buffer[6], msg.Buffer[7]);
		smprintf(s, "LAC                       : %s\n", NetInfo.LAC);

		memset(buf,0,sizeof(buf));
		memcpy(buf,msg.Buffer+11,msg.Buffer[10]*2);
		sprintf(NetInfo.NetworkCode,"%s",DecodeUnicodeString(buf));
		smprintf(s, "Network code              : %s\n", NetInfo.NetworkCode);
		smprintf(s, "Network name for Gammu    : %s ",
			DecodeUnicodeString(GSM_GetNetworkName(NetInfo.NetworkCode)));
		smprintf(s, "(%s)\n",DecodeUnicodeString(GSM_GetCountryName(NetInfo.NetworkCode)));
	}
#endif
	Data->NetworkInfo->NetworkName[0] = 0x00;
	Data->NetworkInfo->NetworkName[1] = 0x00;
	Data->NetworkInfo->State 	  = 0;
	switch (msg.Buffer[8]) {
		case 0x00: Data->NetworkInfo->State = GSM_HomeNetwork;		break;
	}
	if (Data->NetworkInfo->State == GSM_HomeNetwork) {
		sprintf(Data->NetworkInfo->CID, "%02X%02X", msg.Buffer[4], msg.Buffer[5]);
		sprintf(Data->NetworkInfo->LAC, "%02X%02X", msg.Buffer[6], msg.Buffer[7]);

		memset(buf,0,sizeof(buf));
		memcpy(buf,msg.Buffer+11,msg.Buffer[10]*2);
		sprintf(Data->NetworkInfo->NetworkCode,"%s",DecodeUnicodeString(buf));
	}
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo)
{
	unsigned char req[] = {0x00, 0x01};

	s->Phone.Data.NetworkInfo=netinfo;
	smprintf(s, "Getting network info\n");
	return GSM_WaitFor (s, req, 2, 0x03, 4, ID_GetNetworkInfo);
}

static GSM_Error GNAPGEN_ReplyGetMemoryStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Memory status received\n");
	if (msg.Length == 4) return ERR_EMPTY;

	Data->MemoryStatus->MemoryUsed = msg.Buffer[8]*256 + msg.Buffer[9];
	Data->MemoryStatus->MemoryFree = msg.Buffer[12]*256 + msg.Buffer[13];
	smprintf(s, "Free       : %i\n",Data->MemoryStatus->MemoryFree);
	smprintf(s, "Used       : %i\n",Data->MemoryStatus->MemoryUsed);

	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	unsigned char req[] = {0x00,0x07,0x00,
			       0x00};		/* memory type */

	if (Status->MemoryType != MEM_ME) {
		req[3] = NOKIA_GetMemoryType(s, Status->MemoryType,GNAPGEN_MEMORY_TYPES);
		if (req[3]==0xff) return ERR_NOTSUPPORTED;
	}

	s->Phone.Data.MemoryStatus=Status;
	smprintf(s, "Getting memory status\n");
	return GSM_WaitFor (s, req, 4, 0x02, 4, ID_GetMemoryStatus);
}

static GSM_Error GNAPGEN_ReplyGetMemory(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 			i,pos=6,type,subtype,len;
        GSM_MemoryEntry         *entry = s->Phone.Data.Memory;

	entry->EntriesNum=0;

	smprintf(s, "Phonebook entry received\n");
	for (i=0;i<msg.Buffer[5];i++) {
		type = msg.Buffer[pos]*256+msg.Buffer[pos+1];
		subtype = msg.Buffer[pos+2]*256+msg.Buffer[pos+3];
		pos+=4;
		switch (type) {
		case 0x07:
			len = msg.Buffer[pos]*256+msg.Buffer[pos+1];
			if (len!=0) {
				entry->Entries[entry->EntriesNum].EntryType=PBK_Text_Name;
				memcpy(entry->Entries[entry->EntriesNum].Text,msg.Buffer+pos+2,len*2);
				entry->Entries[entry->EntriesNum].Text[len*2]=0;
				entry->Entries[entry->EntriesNum].Text[len*2+1]=0;
				entry->EntriesNum++;
			}
			pos+=2+len*2;
			break;
		case 0x0B:
			len = msg.Buffer[pos]*256+msg.Buffer[pos+1];
			entry->Entries[entry->EntriesNum].EntryType=PBK_Number_General;
			memcpy(entry->Entries[entry->EntriesNum].Text,msg.Buffer+pos+2,len*2);
			entry->Entries[entry->EntriesNum].Text[len*2]=0;
			entry->Entries[entry->EntriesNum].Text[len*2+1]=0;
			entry->EntriesNum++;
			pos+=2+len*2;
			break;
		case 0x13:
			entry->Entries[entry->EntriesNum].EntryType=PBK_Date;
			NOKIA_DecodeDateTime(s, msg.Buffer+pos, &entry->Entries[entry->EntriesNum].Date);
			entry->EntriesNum++;
			pos+=2+7;
			break;
		default:
			smprintf(s, "unknown %i\n",type);
			return ERR_UNKNOWN;
		}
	}

	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetMemory (GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	unsigned char req[] = {0x00, 0x01,
			       0x00, 0x00, 		/* memory type */
			       0x00, 0x00, 0x00, 0x00}; /* location */

	if (entry->MemoryType != MEM_ME) {
		req[3] = NOKIA_GetMemoryType(s, entry->MemoryType,GNAPGEN_MEMORY_TYPES);
		if (req[3]==0xff) return ERR_NOTSUPPORTED;
	}

	if (entry->Location==0x00) return ERR_INVALIDLOCATION;

	req[6] = entry->Location / 256;
	req[7] = entry->Location % 256;

	s->Phone.Data.Memory=entry;
	smprintf(s, "Getting phonebook entry\n");
	return GSM_WaitFor (s, req, 8, 0x02, 6, ID_GetMemory);
}

GSM_Error GNAPGEN_ReplyGetToDo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_ToDoEntry 	*Last = s->Phone.Data.ToDo;
	int 		pos = 8;

	smprintf(s, "TODO received\n");

	memcpy(Last->Entries[0].Text,msg.Buffer+pos+2,(msg.Buffer[pos]*256+msg.Buffer[pos+1])*2);
	Last->Entries[0].Text[(msg.Buffer[pos]*256+msg.Buffer[pos+1])*2] = 0;
	Last->Entries[0].Text[(msg.Buffer[pos]*256+msg.Buffer[pos+1])*2+1] = 0;
	smprintf(s, "Text: \"%s\"\n",DecodeUnicodeString(Last->Entries[0].Text));
	pos+=(msg.Buffer[pos]*256+msg.Buffer[pos+1])*2+2;

	/**
	 * @todo There might be better type.
	 */
	Last->Type = GSM_CAL_MEMO;

	switch (msg.Buffer[pos]) {
		case 1  : Last->Priority = GSM_Priority_High; 	break;
		case 2  : Last->Priority = GSM_Priority_Medium; break;
		case 3  : Last->Priority = GSM_Priority_Low; 	break;
		default	: return ERR_UNKNOWN;
	}
	smprintf(s, "Priority: %i\n",msg.Buffer[4]);

 	Last->Entries[0].EntryType = TODO_TEXT;
	Last->EntriesNum	   = 1;

	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetNextToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool refresh)
{
	GSM_Error 		error;
	unsigned char           req[] = {0x00, 0x07,
                                	 0x00, 0x00, 0x00, 0x00};         /* Location */

	if (refresh) {
		ToDo->Location = 1;
	} else {
		ToDo->Location++;
	}

        req[4]          = ToDo->Location / 256;
        req[5]          = ToDo->Location % 256;

	s->Phone.Data.ToDo = ToDo;
	smprintf(s, "Getting todo\n");
	error = GSM_WaitFor (s, req, 6, 7, 4, ID_GetToDo);
	if (error == ERR_INVALIDLOCATION) error = ERR_EMPTY;
	return error;
}

GSM_Error GNAPGEN_DeleteCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	unsigned char                   req[] = {0x00, 0x05,
                                                 0x00, 0x00, 0x00, 0x00};         /* Location */

        req[4]          = Note->Location / 256;
        req[5]          = Note->Location % 256;

        smprintf(s, "Deleting calendar note\n");
	return GSM_WaitFor (s, req, 6, 7, 4, ID_DeleteCalendarNote);
}

GSM_Error GNAPGEN_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_DateTime		DT;
 	int 			Text, Time, Alarm, Phone, EndTime, Location, current=7;
	unsigned char 		req[5000] = {
		0x00, 0x03,0x00,0x00,
		0x00, 0x00, 0x00, 0x00, /* location ? 	*/
		0x00};			/* type */

	switch(Note->Type) {
		case GSM_CAL_MEETING 	: req[6] = 0x01; break;
		case GSM_CAL_REMINDER 	: req[6] = 0x04; break;
		case GSM_CAL_MEMO	:
		default 		: req[6] = 0x08; break;
	}

	GSM_CalendarFindDefaultTextTimeAlarmPhone(Note, &Text, &Time, &Alarm, &Phone, &EndTime, &Location);

	if (Time == -1) return ERR_UNKNOWN;
	memcpy(&DT,&Note->Entries[Time].Date,sizeof(GSM_DateTime));
	req[current++]	= DT.Year / 256;
	req[current++]	= DT.Year % 256;
	req[current++]	= DT.Month;
	req[current++]	= DT.Day;
	req[current++]	= DT.Hour;
	req[current++]	= DT.Minute;
	req[current++]	= DT.Second;

	if (EndTime == -1) {
		memset(&DT,0,sizeof(GSM_DateTime));
		DT.Month = 1;
		DT.Day = 1;

	} else {
		memcpy(&DT,&Note->Entries[EndTime].Date,sizeof(GSM_DateTime));
	}
	req[current++]	= DT.Year / 256;
	req[current++]	= DT.Year % 256;
	req[current++]	= DT.Month;
	req[current++]	= DT.Day;
	req[current++]	= DT.Hour;
	req[current++]	= DT.Minute;
	req[current++]	= DT.Second;

	if (Alarm == -1) {
		memset(&DT,0,sizeof(GSM_DateTime));
		DT.Month = 1;
		DT.Day = 1;
	} else {
		memcpy(&DT,&Note->Entries[Alarm].Date,sizeof(GSM_DateTime));
	}
	req[current++]	= DT.Year / 256;
	req[current++]	= DT.Year % 256;
	req[current++]	= DT.Month;
	req[current++]	= DT.Day;
	req[current++]	= DT.Hour;
	req[current++]	= DT.Minute;
	req[current++]	= DT.Second;

	if (Text == -1) return ERR_UNKNOWN;
	req[current++]	= UnicodeLength(Note->Entries[Text].Text) / 256;
	req[current++]	= UnicodeLength(Note->Entries[Text].Text) % 256;
	memcpy(req+current,Note->Entries[Text].Text,UnicodeLength(Note->Entries[Text].Text)*2);
	current+=UnicodeLength(Note->Entries[Text].Text)*2;

	req[current++]	= 0;
	req[current++]	= 0;

	if (Location==-1) {
		req[current++]	= 0;
		req[current++]	= 0;
	} else {
		req[current++]	= UnicodeLength(Note->Entries[Location].Text) / 256;
		req[current++]	= UnicodeLength(Note->Entries[Location].Text) % 256;
		memcpy(req+current,Note->Entries[Location].Text,UnicodeLength(Note->Entries[Location].Text)*2);
		current+=UnicodeLength(Note->Entries[Location].Text)*2;
	}

	if (Note->Type == GSM_CAL_MEETING) {
		GSM_GetCalendarRecurranceRepeat(req+current, NULL, Note);
		current+=2;
	} else {
		req[current++]	= 0xff;
		req[current++]	= 0xff;
	}

	smprintf(s, "Writing calendar note\n");
	return GSM_WaitFor (s, req, current, 7, 4, ID_SetCalendarNote);
}

static GSM_Error GNAPGEN_ReplyGetNextCalendar(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
        int                     pos;
        GSM_CalendarEntry       *Entry = s->Phone.Data.Cal;

        switch (msg.Buffer[3]) {
        case 0x00:
                smprintf(s, "Calendar note received\n");
                switch (msg.Buffer[8]) {
                        case 0x01: Entry->Type = GSM_CAL_MEETING;   break;
                        case 0x04: Entry->Type = GSM_CAL_REMINDER;  break;
                        case 0x08: Entry->Type = GSM_CAL_MEMO;      break;
                        default  :
                                smprintf(s, "Unknown note type %i\n",msg.Buffer[8]);
                                return ERR_UNKNOWNRESPONSE;
                }

		pos = 9;
                Entry->EntriesNum = 0;

                NOKIA_DecodeDateTime(s, msg.Buffer+pos, &Entry->Entries[Entry->EntriesNum].Date);
                smprintf(s, "Time        : %02i-%02i-%04i %02i:%02i:%02i\n",
                        Entry->Entries[Entry->EntriesNum].Date.Day,Entry->Entries[Entry->EntriesNum].Date.Month,Entry->Entries[Entry->EntriesNum].Date.Year,
                        Entry->Entries[Entry->EntriesNum].Date.Hour,Entry->Entries[Entry->EntriesNum].Date.Minute,Entry->Entries[Entry->EntriesNum].Date.Second);
                Entry->Entries[Entry->EntriesNum].EntryType = CAL_START_DATETIME;
                Entry->EntriesNum++;
		pos+=7;

                NOKIA_DecodeDateTime(s, msg.Buffer+pos, &Entry->Entries[Entry->EntriesNum].Date);
                smprintf(s, "Time        : %02i-%02i-%04i %02i:%02i:%02i\n",
                        Entry->Entries[Entry->EntriesNum].Date.Day,Entry->Entries[Entry->EntriesNum].Date.Month,Entry->Entries[Entry->EntriesNum].Date.Year,
                        Entry->Entries[Entry->EntriesNum].Date.Hour,Entry->Entries[Entry->EntriesNum].Date.Minute,Entry->Entries[Entry->EntriesNum].Date.Second);
                Entry->Entries[Entry->EntriesNum].EntryType = CAL_END_DATETIME;
                Entry->EntriesNum++;
		pos+=7;

                NOKIA_DecodeDateTime(s, msg.Buffer+pos, &Entry->Entries[Entry->EntriesNum].Date);
                if (Entry->Entries[Entry->EntriesNum].Date.Year!=0) {
	                smprintf(s, "Alarm       : %02i-%02i-%04i %02i:%02i:%02i\n",
	                        Entry->Entries[Entry->EntriesNum].Date.Day,Entry->Entries[Entry->EntriesNum].Date.Month,Entry->Entries[Entry->EntriesNum].Date.Year,
	                        Entry->Entries[Entry->EntriesNum].Date.Hour,Entry->Entries[Entry->EntriesNum].Date.Minute,Entry->Entries[Entry->EntriesNum].Date.Second);
	                Entry->Entries[Entry->EntriesNum].EntryType = CAL_TONE_ALARM_DATETIME;
	                Entry->EntriesNum++;
                } else {
                        smprintf(s, "No alarm\n");
                }
		pos+=7;

		memcpy(Entry->Entries[Entry->EntriesNum].Text,msg.Buffer+pos+2,msg.Buffer[pos+1]*2);
		Entry->Entries[Entry->EntriesNum].Text[msg.Buffer[pos+1]*2  ]=0;
		Entry->Entries[Entry->EntriesNum].Text[msg.Buffer[pos+1]*2+1]=0;
                smprintf(s, "Text \"%s\"\n",DecodeUnicodeString(Entry->Entries[Entry->EntriesNum].Text));
                if (msg.Buffer[pos+1] != 0x00) {
                        Entry->Entries[Entry->EntriesNum].EntryType = CAL_TEXT;
                        Entry->EntriesNum++;
                }
		pos+=msg.Buffer[pos+1]*2+4;

		memcpy(Entry->Entries[Entry->EntriesNum].Text,msg.Buffer+pos+2,msg.Buffer[pos+1]*2);
		Entry->Entries[Entry->EntriesNum].Text[msg.Buffer[pos+1]*2  ]=0;
		Entry->Entries[Entry->EntriesNum].Text[msg.Buffer[pos+1]*2+1]=0;
                smprintf(s, "Text \"%s\"\n",DecodeUnicodeString(Entry->Entries[Entry->EntriesNum].Text));
                if (msg.Buffer[pos+1] != 0x00) {
                        Entry->Entries[Entry->EntriesNum].EntryType = CAL_LOCATION;
                        Entry->EntriesNum++;
                }
		pos+=msg.Buffer[pos+1]*2+2;

		if (Entry->Type == GSM_CAL_MEETING) {
			GSM_GetCalendarRecurranceRepeat(msg.Buffer+pos, NULL, Entry);
		}

                return ERR_NONE;
        case 0x10:
                smprintf(s, "Can't get calendar note - too high location?\n");
                return ERR_INVALIDLOCATION;
        }
        return ERR_UNKNOWNRESPONSE;
}

static GSM_Error GNAPGEN_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start)
{
	GSM_Error 			error;
	GSM_Phone_GNAPGENData           *Priv = &s->Phone.Data.Priv.GNAPGEN;
	unsigned char                   req[] = {0x00, 0x01,
                                                 0x00, 0x00, 0x00, 0x00};         /* Location */

        if (start) {
                Priv->LastCalendarPos = 1;
        } else {
                Priv->LastCalendarPos++;
        }

        Note->Location  = Priv->LastCalendarPos;
        req[4]          = Priv->LastCalendarPos / 256;
        req[5]          = Priv->LastCalendarPos % 256;

        s->Phone.Data.Cal=Note;
        smprintf(s, "Getting calendar note\n");
        error = GSM_WaitFor (s, req, 6, 7, 4, ID_GetCalendarNote);
        if (error == ERR_INVALIDLOCATION) error = ERR_EMPTY;
        return error;
}

static GSM_Error GNAPGEN_ReplyGetSMSStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Used in phone memory   : %i\n",msg.Buffer[6]*256+msg.Buffer[7]);
	smprintf(s, "Unread in phone memory : %i\n",msg.Buffer[10]*256+msg.Buffer[11]);
	Data->SMSStatus->PhoneSize	= 0xff*256+0xff;
	Data->SMSStatus->PhoneUsed	= msg.Buffer[6]*256+msg.Buffer[7];
	Data->SMSStatus->PhoneUnRead 	= msg.Buffer[10]*256+msg.Buffer[11];
	Data->SMSStatus->TemplatesUsed = 0;
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	unsigned char req[] = {0x00, 0x09};

	s->Phone.Data.SMSStatus=status;
	smprintf(s, "Getting SMS status\n");
	return GSM_WaitFor (s, req, 2, 0x6, 2, ID_GetSMSStatus);
}

static GSM_Error GNAPGEN_ReplyGetSMSFolders(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 			j, pos;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	smprintf(s, "SMS folders names received\n");
	Data->SMSFolders->Number = msg.Buffer[5];

	pos = 6;
	for (j=0;j<msg.Buffer[5];j++) {
		if (msg.Buffer[pos+3]>GSM_MAX_SMS_FOLDER_NAME_LEN) {
			smprintf(s, "Too long text\n");
			return ERR_UNKNOWNRESPONSE;
		}
		memcpy(Data->SMSFolders->Folder[j].Name,msg.Buffer + pos+4,msg.Buffer[pos+3]*2);
		Data->SMSFolders->Folder[j].Name[msg.Buffer[pos+3]*2]=0;
		Data->SMSFolders->Folder[j].Name[msg.Buffer[pos+3]*2+1]=0;
		smprintf(s, ", folder name: \"%s\"\n",DecodeUnicodeString(Data->SMSFolders->Folder[j].Name));
		Data->SMSFolders->Folder[j].InboxFolder = false;
		if (j==0) Data->SMSFolders->Folder[j].InboxFolder = true;
		Data->SMSFolders->Folder[j].Memory 	  = MEM_ME;
		pos+=msg.Buffer[pos+3]*2+4;
	}
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	unsigned char req[] = {0x00,0x01};

	s->Phone.Data.SMSFolders=folders;
	smprintf(s, "Getting SMS folders\n");
	return GSM_WaitFor (s, req, 2, 0x06, 4, ID_GetSMSFolders);
}

static GSM_Error GNAPGEN_ReplyGetSMSC(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 			pos=7;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	if (msg.Buffer[7]*2>GSM_MAX_SMSC_NAME_LENGTH) {
		smprintf(s, "Too long name\n");
		return ERR_UNKNOWNRESPONSE;
	}
	memcpy(Data->SMSC->Name,msg.Buffer+8,msg.Buffer[7]*2);
	Data->SMSC->Name[msg.Buffer[7]*2] = 0;
	Data->SMSC->Name[msg.Buffer[7]*2+1] = 0;
	smprintf(s, "   Name \"%s\"\n", DecodeUnicodeString(Data->SMSC->Name));
	pos+=msg.Buffer[7]*2;

	pos+=4;

	Data->SMSC->Format 		= SMS_FORMAT_Text;
	Data->SMSC->Validity.Format	= SMS_Validity_RelativeFormat;
	Data->SMSC->Validity.Relative 	= SMS_VALID_Max_Time;

	Data->SMSC->DefaultNumber[0] = 0;
	Data->SMSC->DefaultNumber[1] = 0;

	memcpy(Data->SMSC->Number,msg.Buffer+pos+4,msg.Buffer[pos+3]*2);
	Data->SMSC->Number[msg.Buffer[pos+3]*2] = 0;
	Data->SMSC->Number[msg.Buffer[pos+3]*2+1] = 0;
	smprintf(s, "   Number \"%s\"\n", DecodeUnicodeString(Data->SMSC->Number));

	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	unsigned char req[] = {0x00,21,
			       0x00,0x01};		/* location */

	if (smsc->Location==0x00) return ERR_INVALIDLOCATION;

	req[3]=smsc->Location-1;

	s->Phone.Data.SMSC=smsc;
	smprintf(s, "Getting SMSC\n");
	return GSM_WaitFor (s, req, 4, 0x06, 4, ID_GetSMSC);
}

static GSM_Error GNAPGEN_ReplyGetAlarm(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Alarm received\n");
	if (msg.Buffer[4] == 0x00) return ERR_EMPTY;
	Data->Alarm->Repeating 		= false;
	Data->Alarm->Text[0] 		= 0;
	Data->Alarm->Text[1] 		= 0;
	NOKIA_DecodeDateTime(s, msg.Buffer+5, &Data->Alarm->DateTime);
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetAlarm(GSM_StateMachine *s, GSM_Alarm *alarm)
{
	unsigned char req[] = {0x00, 0x05};

	if (alarm->Location != 1) return ERR_NOTSUPPORTED;

	s->Phone.Data.Alarm=alarm;
	smprintf(s, "Getting alarm\n");
	return GSM_WaitFor (s, req, 2, 0x8, 4, ID_GetAlarm);
}

static GSM_Error GNAPGEN_ReplyGetDateTime(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Date & time received\n");
	NOKIA_DecodeDateTime(s, msg.Buffer+4, s->Phone.Data.DateTime);
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	unsigned char req[2] = {0x00,0x01};

	s->Phone.Data.DateTime=date_time;
	smprintf(s, "Getting date & time\n");
	return GSM_WaitFor (s, req, 2, 0x08, 4, ID_GetDateTime);
}

GSM_Error GNAPGEN_ReplyGetHW(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 	buff[200];
	int 		pos=8,len,i;

	for (i=0;i<5;i++) {
		len=msg.Buffer[pos]*256+msg.Buffer[pos+1];
		memset(buff,0,sizeof(buff));
		memcpy(buff,msg.Buffer+pos+2,len*2);
		pos+=2+len*2;
	}

	strcpy(s->Phone.Data.HardwareCache,DecodeUnicodeString(buff));
	smprintf(s, "Received HW %s\n",s->Phone.Data.HardwareCache);

	return ERR_NONE;
}

GSM_Error GNAPGEN_GetHW(GSM_StateMachine *s, char *value)
{
	GSM_Error	error;
	unsigned char req[2] = {0x00,0x01};

	if (strlen(s->Phone.Data.HardwareCache)!=0) {
		strcpy(value,s->Phone.Data.HardwareCache);
		return ERR_NONE;
	}

	smprintf(s, "Getting HW\n");
	error = GSM_WaitFor (s, req, 2, 0x01, 2, ID_GetHardware);
	if (error == ERR_NONE) strcpy(value,s->Phone.Data.HardwareCache);
	return error;
}

GSM_Error GNAPGEN_ReplyGetManufacturer(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 			buff[200];
	int 				pos=8,len;
	GSM_Phone_GNAPGENData           *Priv = &s->Phone.Data.Priv.GNAPGEN;

	smprintf(s, "gnapplet %i. %i\n",msg.Buffer[4]*256+msg.Buffer[5],msg.Buffer[6]*256+msg.Buffer[7]);
	Priv->GNAPPLETVer = msg.Buffer[4]*256+msg.Buffer[5] + 0.01*(msg.Buffer[6]*256+msg.Buffer[7]);

	len=msg.Buffer[pos]*256+msg.Buffer[pos+1];
	memset(buff,0,sizeof(buff));
	memcpy(buff,msg.Buffer+pos+2,len*2);

	strcpy(s->Phone.Data.Manufacturer,DecodeUnicodeString(buff));

	return ERR_NONE;
}

GSM_Error GNAPGEN_GetManufacturer(GSM_StateMachine *s)
{
	unsigned char req[2] = {0x00,0x01};

	smprintf(s, "Getting manufacturer\n");
	return GSM_WaitFor (s, req, 2, 0x01, 2, ID_GetManufacturer);
}

GSM_Error GNAPGEN_ReplyGetIMEI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 	buff[200];
	int 		pos=8,len,i;

	for (i=0;i<3;i++) {
		len=msg.Buffer[pos]*256+msg.Buffer[pos+1];
		memset(buff,0,sizeof(buff));
		memcpy(buff,msg.Buffer+pos+2,len*2);
		pos+=2+len*2;
	}

	strcpy(s->Phone.Data.IMEI,DecodeUnicodeString(buff));
	smprintf(s, "Received IMEI %s\n",s->Phone.Data.IMEI);

	return ERR_NONE;
}

GSM_Error GNAPGEN_GetIMEI(GSM_StateMachine *s)
{
	unsigned char req[2] = {0x00,0x01};

	smprintf(s, "Getting IMEI\n");
	return GSM_WaitFor (s, req, 2, 0x01, 2, ID_GetIMEI);
}

GSM_Error GNAPGEN_ReplyGetID(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return ERR_NONE;
}

GSM_Error GNAPGEN_ReplyGetModelFirmware(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Lines	lines;
	GSM_Phone_Data	*Data = &s->Phone.Data;

	if (Data->RequestID!=ID_GetManufacturer && Data->RequestID!=ID_GetModel) return ERR_NONE;

	SplitLines(DecodeUnicodeString(msg.Buffer+6), msg.Length-6, &lines, "\x0A", 1, false);

	strcpy(Data->Model,GetLineString(DecodeUnicodeString(msg.Buffer+6), lines, 4));
	smprintf(s, "Received model %s\n",Data->Model);
	Data->ModelInfo = GetModelData(NULL,Data->Model,NULL);

	strcpy(Data->VerDate,GetLineString(DecodeUnicodeString(msg.Buffer+6), lines, 3));
	smprintf(s, "Received firmware date %s\n",Data->VerDate);

	strcpy(Data->Version,GetLineString(DecodeUnicodeString(msg.Buffer+6), lines, 2));
	smprintf(s, "Received firmware version %s\n",Data->Version);
	GSM_CreateFirmwareNumber(s);

	return ERR_NONE;
}

GSM_Error GNAPGEN_GetModel (GSM_StateMachine *s)
{
	unsigned char req[2] = {0x00,0x01};
	GSM_Error 	error;

	if (strlen(s->Phone.Data.Model)>0) return ERR_NONE;

	smprintf(s, "Getting model\n");
	return GSM_WaitFor (s, req, 2, 0x01, 2, ID_GetModel);
	if (error==ERR_NONE) {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
			smprintf(s, "[Connected model  - \"%s\"]\n",s->Phone.Data.Model);
			smprintf(s, "[Firmware version - \"%s\"]\n",s->Phone.Data.Version);
			smprintf(s, "[Firmware date    - \"%s\"]\n",s->Phone.Data.VerDate);
		}
	}
	return error;
}

GSM_Error GNAPGEN_GetFirmware (GSM_StateMachine *s)
{
	unsigned char req[2] = {0x00,0x01};
	GSM_Error error;

	if (strlen(s->Phone.Data.Version)>0) return ERR_NONE;

	smprintf(s, "Getting firmware version\n");
	return GSM_WaitFor (s, req, 2, 0x01, 2, ID_GetFirmware);
	if (error==ERR_NONE) {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
			smprintf(s, "[Connected model  - \"%s\"]\n",s->Phone.Data.Model);
			smprintf(s, "[Firmware version - \"%s\"]\n",s->Phone.Data.Version);
			smprintf(s, "[Firmware date    - \"%s\"]\n",s->Phone.Data.VerDate);
		}
	}
	return error;
}

static GSM_Error GNAPGEN_Initialise (GSM_StateMachine *s)
{
	GSM_Phone_GNAPGENData           *Priv = &s->Phone.Data.Priv.GNAPGEN;
	GSM_Error			error;

	error = GNAPGEN_GetManufacturer(s);
	if (error != ERR_NONE) return error;

	if (Priv->GNAPPLETVer==0.18) return ERR_NONE;
	return ERR_GNAPPLETWRONG;
}

static GSM_Reply_Function GNAPGENReplyFunctions[] = {
	//informations
	{GNAPGEN_ReplyGetIMEI,		"\x01",0x01,0x02,ID_GetIMEI			},
	{GNAPGEN_ReplyGetHW,		"\x01",0x01,0x02,ID_GetHardware			},
	{GNAPGEN_ReplyGetManufacturer,	"\x01",0x01,0x02,ID_GetManufacturer		},
	{GNAPGEN_ReplyGetID,		"\x01",0x01,0x02,ID_GetModel			},
	{GNAPGEN_ReplyGetID,		"\x01",0x01,0x02,ID_GetFirmware			},

	{GNAPGEN_ReplyGetMemory,	"\x02",0x01,0x02,ID_GetMemory			},
	{GNAPGEN_ReplyGetMemoryStatus,	"\x02",0x01,0x08,ID_GetMemoryStatus		},

	{GNAPGEN_ReplyGetNetworkInfo,	"\x03",0x01,0x02,ID_GetNetworkInfo		},
	{GNAPGEN_ReplyGetSignalQuality,	"\x03",0x01,0x04,ID_GetSignalQuality		},

	{GNAPGEN_ReplyGetBatteryCharge,	"\x04",0x01,0x02,ID_GetBatteryCharge		},

	//type 5 is DEBUG
	{GNAPGEN_ReplyGetModelFirmware,	"\x05",0x01,0x02,ID_IncomingFrame		},

	//type 6 is SMS
	{GNAPGEN_ReplyGetSMSFolders,	"\x06",0x01,0x02,ID_GetSMSFolders		},
	{GNAPGEN_ReplyGetSMSStatus,	"\x06",0x01,0x0A,ID_GetSMSStatus		},
	{GNAPGEN_ReplyGetSMSC,		"\x06",0x01,0x16,ID_GetSMSC			},

	//calendar
	{GNAPGEN_ReplyGetNextCalendar,	"\x07",0x01,0x02,ID_GetCalendarNote		},
	{NONEFUNCTION,			"\x07",0x01,0x06,ID_DeleteCalendarNote		},
	{GNAPGEN_ReplyGetToDo,		"\x07",0x01,0x08,ID_GetToDo			},

	//time
	{GNAPGEN_ReplyGetDateTime,	"\x08",0x01,0x02,ID_GetDateTime			},
	{GNAPGEN_ReplyGetAlarm,		"\x08",0x01,0x06,ID_GetAlarm			},

	{NULL,				"\x00",0x00,0x00,ID_None			}
};

GSM_Phone_Functions GNAPGENPhone = {
	"gnap",
	GNAPGENReplyFunctions,
	GNAPGEN_Initialise,
	NONEFUNCTION,			/*	Terminate 		*/
	GSM_DispatchMessage,
	NOTSUPPORTED,			/* 	ShowStartInfo		*/
	GNAPGEN_GetManufacturer,
	GNAPGEN_GetModel,
	GNAPGEN_GetFirmware,
	GNAPGEN_GetIMEI,
	NOTSUPPORTED,			/*	GetOriginalIMEI		*/
	NOTSUPPORTED,			/*	GetManufactureMonth	*/
	NOTSUPPORTED,			/*	GetProductCode		*/
	GNAPGEN_GetHW,
	NOTSUPPORTED,			/*	GetPPM			*/
	NOTSUPPORTED,			/*	GetSIMIMSI		*/
	GNAPGEN_GetDateTime,
	NOTSUPPORTED,			/*	SetDateTime		*/
	GNAPGEN_GetAlarm,
	NOTSUPPORTED,			/*	SetAlarm		*/
	NOTSUPPORTED,			/* 	GetLocale		*/
	NOTSUPPORTED,			/* 	SetLocale		*/
	NOTSUPPORTED,			/*	PressKey		*/
	NOTSUPPORTED,			/*	Reset			*/
	NOTSUPPORTED,			/*	ResetPhoneSettings	*/
	NOTSUPPORTED,			/*	EnterSecurityCode	*/
	NOTSUPPORTED,			/*	GetSecurityStatus	*/
	NOTSUPPORTED,			/*	GetDisplayStatus	*/
	NOTSUPPORTED,			/*	SetAutoNetworkLogin	*/
	GNAPGEN_GetBatteryCharge,
	GNAPGEN_GetSignalQuality,
	GNAPGEN_GetNetworkInfo,
	NOTSUPPORTED,     		/*  	GetCategory 		*/
 	NOTSUPPORTED,       		/*  	AddCategory 		*/
        NOTSUPPORTED,      		/*  	GetCategoryStatus 	*/
	GNAPGEN_GetMemoryStatus,
	GNAPGEN_GetMemory,
	NOTSUPPORTED,			/*	GetNextMemory		*/
	NOTSUPPORTED,			/*	SetMemory		*/
	NOTSUPPORTED,			/*	AddMemory		*/
	NOTSUPPORTED,			/*	DeleteMemory		*/
	NOTIMPLEMENTED,			/*	DeleteAllMemory		*/
	NOTSUPPORTED,			/*	GetSpeedDial		*/
	NOTSUPPORTED,			/*	SetSpeedDial		*/
	GNAPGEN_GetSMSC,
	NOTSUPPORTED,			/*	SetSMSC			*/
	GNAPGEN_GetSMSStatus,
	NOTSUPPORTED,			/*	GetSMS			*/
	GNAPGEN_GetNextSMS,
	NOTSUPPORTED,			/*	SetSMS			*/
	GNAPGEN_AddSMS,
	NOTSUPPORTED,			/* 	DeleteSMS 		*/
	GNAPGEN_SendSMSMessage,
	NOTSUPPORTED,			/*	SendSavedSMS		*/
	NOTSUPPORTED,			/*	SetFastSMSSending	*/
	NOTSUPPORTED,			/*	SetIncomingSMS		*/
	NOTSUPPORTED,			/* 	SetIncomingCB		*/
	GNAPGEN_GetSMSFolders,
 	NOTSUPPORTED,			/* 	AddSMSFolder		*/
 	NOTSUPPORTED,			/* 	DeleteSMSFolder		*/
	NOTSUPPORTED,			/*	DialVoice		*/
	NOTSUPPORTED,			/*	AnswerCall		*/
	NOTSUPPORTED,			/*	CancelCall		*/
 	NOTSUPPORTED,			/* 	HoldCall 		*/
 	NOTSUPPORTED,			/* 	UnholdCall 		*/
 	NOTSUPPORTED,			/* 	ConferenceCall 		*/
 	NOTSUPPORTED,			/* 	SplitCall		*/
 	NOTSUPPORTED,			/* 	TransferCall		*/
 	NOTSUPPORTED,			/* 	SwitchCall		*/
 	NOTSUPPORTED,			/* 	GetCallDivert		*/
 	NOTSUPPORTED,			/* 	SetCallDivert		*/
 	NOTSUPPORTED,			/* 	CancelAllDiverts	*/
	NOTSUPPORTED,			/*	SetIncomingCall		*/
	NOTSUPPORTED,			/*  	SetIncomingUSSD		*/
	NOTSUPPORTED,			/*	SendDTMF		*/
	NOTSUPPORTED,			/*	GetRingtone		*/
	NOTSUPPORTED,			/*	SetRingtone		*/
	NOTSUPPORTED,			/*	GetRingtonesInfo	*/
	NOTSUPPORTED,			/* 	DeleteUserRingtones	*/
	NOTSUPPORTED,			/*	PlayTone		*/
	NOTSUPPORTED,			/*	GetWAPBookmark		*/
	NOTSUPPORTED,			/* 	SetWAPBookmark 		*/
	NOTSUPPORTED, 			/* 	DeleteWAPBookmark 	*/
	NOTSUPPORTED,			/* 	GetWAPSettings 		*/
	NOTSUPPORTED,			/* 	SetWAPSettings 		*/
	NOTSUPPORTED,			/*	GetSyncMLSettings	*/
	NOTSUPPORTED,			/*	SetSyncMLSettings	*/
	NOTSUPPORTED,			/*	GetChatSettings		*/
	NOTSUPPORTED,			/*	SetChatSettings		*/
	NOTSUPPORTED,			/* 	GetMMSSettings		*/
	NOTSUPPORTED,			/* 	SetMMSSettings		*/
	NOTSUPPORTED,			/*	GetMMSFolders		*/
	NOTSUPPORTED,			/*	GetNextMMSFileInfo	*/
	NOTSUPPORTED,			/*	GetBitmap		*/
	NOTSUPPORTED,			/*	SetBitmap		*/
	NOTSUPPORTED,			/*	GetToDoStatus		*/
	NOTSUPPORTED,			/*	GetToDo			*/
	GNAPGEN_GetNextToDo,
	NOTSUPPORTED,			/*	SetToDo			*/
	NOTSUPPORTED,			/*	AddToDo			*/
	NOTSUPPORTED,			/*	DeleteToDo		*/
	NOTSUPPORTED,			/*	DeleteAllToDo		*/
	NOTSUPPORTED,			/*	GetCalendarStatus	*/
	NOTSUPPORTED,			/*	GetCalendar		*/
    	GNAPGEN_GetNextCalendar,
	NOTSUPPORTED,			/*	SetCalendar		*/
	GNAPGEN_AddCalendar,
	GNAPGEN_DeleteCalendar,
	NOTSUPPORTED,			/*	DeleteAllCalendar	*/
	NOTSUPPORTED,			/* 	GetCalendarSettings	*/
	NOTSUPPORTED,			/* 	SetCalendarSettings	*/
	NOTSUPPORTED,			/*	GetNoteStatus		*/
	NOTSUPPORTED,			/*	GetNote			*/
	NOTSUPPORTED,			/*	GetNextNote		*/
	NOTSUPPORTED,			/*	SetNote			*/
	NOTSUPPORTED,			/*	AddNote			*/
	NOTSUPPORTED,			/* 	DeleteNote		*/
	NOTSUPPORTED,			/*	DeleteAllNotes		*/
	NOTSUPPORTED, 			/*	GetProfile		*/
	NOTSUPPORTED, 			/*	SetProfile		*/
    	NOTSUPPORTED,			/*  	GetFMStation        	*/
    	NOTSUPPORTED,			/*  	SetFMStation        	*/
	NOTSUPPORTED,			/* 	ClearFMStations		*/
	NOTSUPPORTED,			/* 	GetNextFileFolder	*/
	NOTSUPPORTED,			/*	GetFolderListing	*/
	NOTSUPPORTED,			/*	GetNextRootFolder	*/
	NOTSUPPORTED,			/*	SetFileAttributes	*/
	NOTSUPPORTED,			/* 	GetFilePart		*/
	NOTSUPPORTED,			/* 	AddFilePart		*/
	NOTSUPPORTED, 			/* 	GetFileSystemStatus	*/
	NOTSUPPORTED,			/* 	DeleteFile		*/
	NOTSUPPORTED,			/* 	AddFolder		*/
	NOTSUPPORTED,			/* 	DeleteFolder		*/
	NOTSUPPORTED,			/* 	GetGPRSAccessPoint	*/
	NOTSUPPORTED			/* 	SetGPRSAccessPoint	*/
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
