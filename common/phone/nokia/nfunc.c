
#include <string.h> /* memcpy only */
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include "../../gsmstate.h"
#include "../../misc/coding.h"
#include "../../service/gsmsms.h"
#include "../pfunc.h"
#include "nfunc.h"

#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)

/* --------------------- Some general Nokia functions ---------------------- */

void NOKIA_DecodeSMSState(unsigned char state, GSM_SMSMessage *sms)
{
	switch (state) {
		case 0x01 : sms->State = GSM_Read;   break;
		case 0x03 : sms->State = GSM_UnRead; break;
		case 0x05 : sms->State = GSM_Sent;   break;
		case 0x07 : sms->State = GSM_UnSent; break;
		default	  : dprintf("Unknown SMS state: %02x\n",state);
	}
}

GSM_Error NOKIA_ReplyGetPhoneString(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	strcpy(Data->PhoneString,msg.Buffer+Data->StartPhoneString);
	return GE_NONE;
}

/* Some strings are very easy. Some header, after it required string and 0x00.
 * We can get them using this function. We give frame to send (*string),
 * type of message (type), pointer for buffer for response (*value), request
 * type (request) and what is start byte in response for our string
 */
GSM_Error NOKIA_GetPhoneString(GSM_StateMachine *s, unsigned char *msgframe, int msglen, unsigned char msgtype, char *retvalue, GSM_Phone_RequestID request, int startresponse)
{
	retvalue[0] = 0;
	s->Phone.Data.StartPhoneString = startresponse;
	s->Phone.Data.PhoneString = retvalue;
	return GSM_WaitFor (s, msgframe, msglen,msgtype, 4, request);
}

GSM_Error NOKIA_GetManufacturer(GSM_StateMachine *s, char *manufacturer)
{
	EncodeUnicode(manufacturer,"Nokia",5);
	return GE_NONE;
}

/* Many functions contains such strings:
 * (1. length/256) - exist or not
 * 2. length%256
 * 3. string (unicode, no termination)
 * This function read string to output and increases counter
 */
void NOKIA_GetUnicodeString(int *current, unsigned char *input, unsigned char *output, bool FullLength)
{
	int length;

	if (FullLength) {
		length = (input[*current]*256+input[*current+1])*2;
		memcpy(output,input+(*current+2),length);
		*current = *current + 2 + length;
	} else {
		length = (input[*current])*2;
		memcpy(output,input+(*current+1),length);
		*current = *current + 1 + length;
	}

	output[length  ] = 0;
	output[length+1] = 0;
}

int NOKIA_SetUnicodeString(unsigned char *dest, unsigned char *string, bool FullLength)
{
	int length;
	
	length = strlen(DecodeUnicodeString(string));
	if (FullLength) {
		dest[0] = length / 256;
		dest[1] = length % 256;
		CopyUnicodeString(dest + 2, string);
		return 2+length*2;		
	} else {
		dest[0] = length % 256;
		CopyUnicodeString(dest + 1, string);
		return 1+length*2;
	}
}

/* Returns correct ID for concrete memory type */
GSM_MemoryType NOKIA_GetMemoryType(GSM_MemoryType memory_type, unsigned char *ID)
{
	int i=0;
	while (ID[i+1]!=0x00) {
		if (ID[i]==memory_type) return ID[i+1];
		i=i+2;
	}
	return 0xff;
}

void NOKIA_EncodeDateTime(unsigned char* buffer, GSM_DateTime *datetime)
{
	buffer[0] = datetime->Year / 256;
	buffer[1] = datetime->Year % 256;
	buffer[2] = datetime->Month;
	buffer[3] = datetime->Day;

	buffer[4] = datetime->Hour;
	buffer[5] = datetime->Minute;
}

void NOKIA_SortSMSFolderStatus(GSM_NOKIASMSFolder *Folder)
{
	int i,j;

	if (Folder->Number!=0) {
		/* Bouble sorting */
		i=0;
		while (i!=Folder->Number-1) {
			if (Folder->Location[i]>Folder->Location[i+1]) {
				j=Folder->Location[i];
				Folder->Location[i]=Folder->Location[i+1];
				Folder->Location[i+1]=j;
				i=0;
			} else {
				i++;
			}
		}
#ifdef DEBUG
		dprintf("Locations: ");
		for (i=0;i<Folder->Number;i++) {
			dprintf("%i ",Folder->Location[i]);
		}
		dprintf("\n");
#endif
	}
}

void NOKIA_GetDefaultProfileName(GSM_StateMachine *s, GSM_Profile *Profile)
{
	if (Profile->DefaultName) {
		switch(Profile->Location) {
			case 1:	EncodeUnicode(Profile->Name,GetMsg(s->msg,"General"),strlen(GetMsg(s->msg,"General")));
				break;
			case 2: EncodeUnicode(Profile->Name,GetMsg(s->msg,"Silent"),strlen(GetMsg(s->msg,"Silent")));
				break;
			case 3: EncodeUnicode(Profile->Name,GetMsg(s->msg,"Meeting"),strlen(GetMsg(s->msg,"Meeting")));
				break;
			case 4:	EncodeUnicode(Profile->Name,GetMsg(s->msg,"Outdoor"),strlen(GetMsg(s->msg,"Outdoor")));
				break;
			case 5:	EncodeUnicode(Profile->Name,GetMsg(s->msg,"Pager"),strlen(GetMsg(s->msg,"Pager")));
				break;
			case 6: EncodeUnicode(Profile->Name,GetMsg(s->msg,"Car"),strlen(GetMsg(s->msg,"Car")));
				break;
			case 7: EncodeUnicode(Profile->Name,GetMsg(s->msg,"Headset"),strlen(GetMsg(s->msg,"Headset")));
				break;
		}
	}
}

/* - Shared for DCT3 (n6110.c, n7110.c, n9110.c) and DCT4 (n6510.c) phones - */

GSM_Error DCT3DCT4_ReplyGetWAPBookmark(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User, bool FullLength)
{
	int tmp;

	dprintf("WAP bookmark received\n");
	switch (msg.Buffer[3]) {
	case 0x07:
		tmp = 4;

		Data->WAPBookmark->Location = msg.Buffer[tmp] * 256 + msg.Buffer[tmp+1];
		dprintf("Location: %i\n",Data->WAPBookmark->Location);
		tmp = tmp + 2;

 		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPBookmark->Title, FullLength);
		dprintf("Title   : \"%s\"\n",DecodeUnicodeString(Data->WAPBookmark->Title));

 		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPBookmark->Address, FullLength);
		dprintf("Address : \"%s\"\n",DecodeUnicodeString(Data->WAPBookmark->Address));

		return GE_NONE;
	case 0x08:
		switch (msg.Buffer[4]) {
		case 0x01:
			dprintf("Security error. Inside WAP bookmarks menu\n");
			return GE_INSIDEPHONEMENU;
		case 0x02:
			dprintf("Invalid or empty\n");
			return GE_INVALIDLOCATION;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
			return GE_UNKNOWNRESPONSE;
		}
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error DCT3DCT4_ReplySetWAPBookmark(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x0A:
		dprintf("WAP bookmark set OK\n");
		return GE_NONE;
	case 0x0B:
		dprintf("WAP bookmark setting error\n");
		switch (msg.Buffer[4]) {
		case 0x01:
			dprintf("Security error. Inside WAP bookmarks menu\n");
			return GE_INSIDEPHONEMENU;
		case 0x02:
			dprintf("Can't write to empty location ?\n");
			return GE_EMPTY;
		case 0x04:
			dprintf("Full memory\n");
			return GE_FULL;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
			return GE_UNKNOWNRESPONSE;
		}
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error DCT3DCT4_ReplyEnableWAP(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("WAP functions enabled\n");
	return GE_NONE;
}

GSM_Error DCT3DCT4_EnableWAP(GSM_StateMachine *s)
{
	unsigned char req[] = { N6110_FRAME_HEADER, 0x00 };

	/* Check if have WAP in phone */
	if (IsPhoneFeatureAvailable(s->Model,F_NOWAP)) return GE_NOTSUPPORTED;

	dprintf("Enabling WAP\n");
	return GSM_WaitFor (s, req, 4, 0x3f, 4, ID_EnableWAP);
}

GSM_Error DCT3DCT4_ReplyDelWAPBookmark(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x0D:
		dprintf("WAP bookmark deleted OK\n");
		return GE_NONE;
	case 0x0E:
		dprintf("WAP bookmark deleting error\n");
		switch (msg.Buffer[4]) {
		case 0x01:
			dprintf("Security error. Inside WAP bookmarks menu\n");
			return GE_SECURITYERROR;
		case 0x02:
			dprintf("Invalid location\n");
			return GE_INVALIDLOCATION;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
			return GE_UNKNOWNRESPONSE;
		}
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error DCT3DCT4_DeleteWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error 	error;
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x0C,
		0x00, 0x00};		/* Location */

	/* We have to enable WAP frames in phone */
	error=DCT3DCT4_EnableWAP(s);
	if (error!=GE_NONE) return error;

	req[5] = bookmark->Location;

	dprintf("Deleting WAP bookmark\n");
	return GSM_WaitFor (s, req, 6, 0x3f, 4, ID_DeleteWAPBookmark);
}

GSM_Error DCT3DCT4_GetWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error error;
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x06,
		0x00, 0x00};		/* Location */

	/* We have to enable WAP frames in phone */
	error=DCT3DCT4_EnableWAP(s);
	if (error!=GE_NONE) return error;

	req[5]=bookmark->Location-1;

	s->Phone.Data.WAPBookmark=bookmark;
	dprintf("Getting WAP bookmark\n");
	return GSM_WaitFor (s, req, 6, 0x3f, 4, ID_GetWAPBookmark);
}

GSM_Error DCT3DCT4_ReplyGetModelFirmware(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	GSM_Lines lines;

	SplitLines(msg.Buffer, msg.Length, &lines, "\x20\x0A", 2);

	strcpy(Data->Model,GetLineString(msg.Buffer, lines, 4));
	dprintf("Received model %s\n",Data->Model);

	strcpy(Data->VersionDate,GetLineString(msg.Buffer, lines, 3));
	dprintf("Received firmware date %s\n",Data->VersionDate);

	strcpy(Data->Version,GetLineString(msg.Buffer, lines, 2));
	dprintf("Received firmware version %s\n",Data->Version);
	GSM_CreateFirmwareNumber(Data);

	return GE_NONE;
}

GSM_Error DCT3DCT4_GetModel (GSM_StateMachine *s)
{
	unsigned char 	req[5] = {N6110_FRAME_HEADER, 0x03, 0x00};
	GSM_Error 	error;

	if (strlen(s->Model)>0) return GE_NONE;

	dprintf("Getting model\n");
	error=GSM_WaitFor (s, req, 5, 0xd1, 3, ID_GetModel);
	if (error==GE_NONE) {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL) {
			smprintf(s, "[Connected model  - \"%s\"]\n",s->Phone.Data.Model);
			smprintf(s, "[Firmware version - \"%s\"]\n",s->Phone.Data.Version);
			smprintf(s, "[Firmware date    - \"%s\"]\n",s->Phone.Data.VersionDate);
		}
	}
	return error;
}

GSM_Error DCT3DCT4_GetFirmware (GSM_StateMachine *s)
{
	unsigned char req[5] = {N6110_FRAME_HEADER, 0x03, 0x00};  
	GSM_Error error;

	if (strlen(s->Ver)>0) return GE_NONE;
	
	dprintf("Getting firmware version\n");
	error=GSM_WaitFor (s, req, 5, 0xd1, 3, ID_GetFirmware);
	if (error==GE_NONE) {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL) {
			smprintf(s, "[Connected model  - \"%s\"]\n",s->Phone.Data.Model);
			smprintf(s, "[Firmware version - \"%s\"]\n",s->Phone.Data.Version);
			smprintf(s, "[Firmware date    - \"%s\"]\n",s->Phone.Data.VersionDate);
		}
	}
	return error;
}

/* ---------- Shared for n7110.c and n6510.c ------------------------------- */

GSM_Error N71_65_ReplyDeleteMemory(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Phonebook entry deleted\n");
	return GE_NONE;
}

GSM_Error N71_65_DeleteMemory(GSM_StateMachine *s, GSM_PhonebookEntry *entry, unsigned char *memory)
{
	unsigned char req[] = {
		N7110_FRAME_HEADER, 0x0f, 0x00, 0x01,
		0x04, 0x00, 0x00, 0x0c, 0x01, 0xff,
		0x00, 0x01,		/* location	*/
		0x05, 			/* memory type	*/
		0x00, 0x00, 0x00};

	req[12] = (entry->Location >> 8);
	req[13] = entry->Location & 0xff;

	req[14] = NOKIA_GetMemoryType(entry->MemoryType,memory);
	if (req[14]==0xff) return GE_NOTSUPPORTED;

	dprintf("Deleting phonebook entry\n");
	return GSM_WaitFor (s, req, 18, 0x03, 4, ID_SetMemory);
}

GSM_Error N71_65_ReplyGetMemoryError(unsigned char error, GSM_Phone_Data *Data)
{
	switch (error) {
	case 0x30:
		dprintf("Invalid memory type\n");
		if (Data->Memory->MemoryType == GMT_ME) return GE_EMPTY;
		return GE_NOTSUPPORTED;
	case 0x33:
		dprintf("Empty location\n");
		Data->Memory->EntriesNum = 0;
		return GE_NONE;
	case 0x34:
		dprintf("Too high location ?\n");
		return GE_INVALIDLOCATION;
	default:
		dprintf("ERROR: unknown %i\n",error);
		return GE_UNKNOWNRESPONSE;
	}
}

GSM_Error N71_65_ReplyWritePhonebook(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Phonebook entry written ");
	switch (msg.Buffer[6]) {
	case 0x0f:
		dprintf(" - error\n");
		switch (msg.Buffer[10]) {
		case 0x36:
			dprintf("Too long name\n");
			return GE_NOTSUPPORTED;
		case 0x3d:
			dprintf("Wrong entry type\n");
			return GE_NOTSUPPORTED;
		case 0x3e:
			dprintf("Too much entries\n");
			return GE_NOTSUPPORTED;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[10]);
			return GE_UNKNOWNRESPONSE;
		}
	default:
		dprintf(" - OK\n");
		return GE_NONE;
	}
}

GSM_Error N71_65_ReplyGetCalendarInfo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User, GSM_NOKIACalendarLocations *LastCalendar)
{
	int i;

	dprintf("Info with calendar notes locations received\n");
	LastCalendar->Number=msg.Buffer[4]*256+msg.Buffer[5];
	dprintf("Number of Entries: %i\n",LastCalendar->Number);
	dprintf("Locations: ");
	for (i=0;i<LastCalendar->Number;i++) {
		/* Some phones seems to return incorrect number
		 * of calendar entries. Please don't ask why.
		 * Here is workaround */
		if (((8+(i*2))>=msg.Length) || ((9+(i*2))>=msg.Length)) {
			LastCalendar->Number = i+1;
			dprintf("\nCorrect number of Entries: %i",LastCalendar->Number);
			break;
		}
		LastCalendar->Location[i]=msg.Buffer[8+(i*2)]*256+msg.Buffer[9+(i*2)];
		dprintf("%i ",LastCalendar->Location[i]);
	}
	dprintf("\n");
	return GE_NONE;
}

GSM_Error N71_65_GetCalendarInfo(GSM_StateMachine *s)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x3a, 0xFF, 0xFE};
	dprintf("Getting locations for calendar\n");
	return GSM_WaitFor (s, req, 6, 0x13, 4, ID_GetCalendarNotesInfo);
}

GSM_Error N71_65_GetCalendarNote(GSM_StateMachine *s, GSM_CalendarNote *Note, bool start, GSM_NOKIACalendarLocations *LastCalendar)
{
	GSM_Error		error;
	GSM_DateTime		date_time;
	unsigned char 		req[] = {
		N6110_FRAME_HEADER, 0x19, 
		0x00, 0x00};		/* Location */

	if (start) {
		error=N71_65_GetCalendarInfo(s);
		if (error!=GE_NONE) return error;
	}

	if (Note->Location==0x00) return GE_INVALIDLOCATION;
	if (Note->Location > LastCalendar->Number) return GE_INVALIDLOCATION;

	req[4] = LastCalendar->Location[Note->Location-1] >> 8;
	req[5] = LastCalendar->Location[Note->Location-1] & 0xff;

	/* We have to get current year. It's NOT written in frame for Birthday */
	error=s->Phone.Functions->GetDateTime(s,&date_time);
	switch (error) {
	case GE_EMPTY:
	case GE_NOTIMPLEMENTED:
		GSM_GetCurrentDateTime (&date_time);
		break;
	case GE_NONE:
		break;
	default:
		return error;
	}
	Note->Time.Year = date_time.Year;

	s->Phone.Data.Calendar=Note;
	dprintf("Getting calendar note\n");
	return GSM_WaitFor (s, req, 6, 0x13, 4, ID_GetCalendarNote);
}

static void N71_65_GetNoteAlarm(int alarmdiff, GSM_DateTime *time, GSM_DateTime *alarm, int alarm2)
{
	time_t     t_alarm;
	struct tm  tm_time;
	struct tm  *tm_alarm;

#ifdef DEBUG			
	if (alarmdiff == 0xffff) dprintf("No alarm\n");
       			    else dprintf("Alarm is %i seconds before date\n", alarmdiff*alarm2);
#endif
	if (alarmdiff != 0xffff) {	
		memset(&tm_time, 0, sizeof(tm_time));
		tm_time.tm_year = time->Year - 1900;
		tm_time.tm_mon  = time->Month - 1;
		tm_time.tm_mday = time->Day;
		tm_time.tm_hour = time->Hour;
		tm_time.tm_min  = time->Minute;
		tm_time.tm_sec  = time->Second;
		tzset();
		t_alarm  = mktime(&tm_time);
		t_alarm -= alarmdiff*alarm2;
		tm_alarm = localtime(&t_alarm);
		alarm->Year   = tm_alarm->tm_year + 1900;
		alarm->Month  = tm_alarm->tm_mon + 1;
		alarm->Day    = tm_alarm->tm_mday;
		alarm->Hour   = tm_alarm->tm_hour;
		alarm->Minute = tm_alarm->tm_min;
		alarm->Second = tm_alarm->tm_sec;
		dprintf("Alarm        : %02i-%02i-%04i %02i:%02i:%02i\n",
			alarm->Day,alarm->Month,alarm->Year,
			alarm->Hour,alarm->Minute,alarm->Second);
	}
}

static void N71_65_GetNoteTimes(unsigned char *block, GSM_CalendarNote *c)
{
	time_t		alarmdiff;

	c->Time.Year	= block[8]*256+block[9];
	c->Time.Month	= block[10];
	c->Time.Day	= block[11];
	if (c->Type != GCN_REMINDER) {
		c->Time.Hour	= block[12];
		c->Time.Minute	= block[13];
	} else {
		c->Time.Hour	= 0;
		c->Time.Minute	= 0;
	}
	c->Time.Second = 0;

	dprintf("Date: %02i-%02i-%04i %02i:%02i:%02i\n",
		c->Time.Day,c->Time.Month,c->Time.Year,
		c->Time.Hour,c->Time.Minute,c->Time.Second);

	if (c->Type != GCN_REMINDER) {
		alarmdiff = block[14]*256 + block[15];
		N71_65_GetNoteAlarm(alarmdiff, &(c->Time), &(c->Alarm), 60);
		c->Recurrance = block[16]*256 + block[17];
		memcpy(c->Text, block+20, block[18]*2);
		c->Text[block[18]*2]=0;
		c->Text[block[18]*2+1]=0;
	} else {
		c->Recurrance = block[12]*256 + block[13];
		memcpy(c->Text, block+16, block[14]*2);
		c->Text[block[14]*2]=0;
		c->Text[block[14]*2+1]=0;
	}

	/* 0xffff -> 1 Year (8760 hours) */
	if (c->Recurrance == 0xffff) c->Recurrance=8760;

	dprintf("Recurrance   : %i hours\nText         : \"%s\"\n", c->Recurrance,DecodeUnicodeString(c->Text));
}

GSM_Error N71_65_ReplyGetCalendarNote(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int alarm;

	dprintf("Calendar note received\n");
	Data->Calendar->SilentAlarm	= false;
	Data->Calendar->Phone[0]	= 0;
	Data->Calendar->Phone[1]	= 0;
	memset(&Data->Calendar->Alarm, 0, sizeof(GSM_DateTime));

	switch (msg.Buffer[6]) {
	case 0x01:
		dprintf("Meeting\n");
		Data->Calendar->Type = GCN_MEETING;
		N71_65_GetNoteTimes(msg.Buffer, Data->Calendar);
		return GE_NONE;
	case 0x02:
		dprintf("Call\n");
		Data->Calendar->Type = GCN_CALL;
		N71_65_GetNoteTimes(msg.Buffer, Data->Calendar);
		memcpy(Data->Calendar->Phone, msg.Buffer+20+msg.Buffer[18]*2, msg.Buffer[19]*2);
		Data->Calendar->Phone[msg.Buffer[19]*2]=0;
		Data->Calendar->Phone[msg.Buffer[19]*2+1]=0;
		dprintf("Phone number: \"%s\"\n",DecodeUnicodeString(Data->Calendar->Phone));
		return GE_NONE;
	case 0x04:
		dprintf("Birthday\n");
		Data->Calendar->Type		= GCN_BIRTHDAY;
		Data->Calendar->Time.Month	= msg.Buffer[10];
		Data->Calendar->Time.Day	= msg.Buffer[11];
		Data->Calendar->Time.Hour	= 23;
		Data->Calendar->Time.Minute	= 59;
		Data->Calendar->Time.Second	= 58;
		alarm  = ((unsigned int)msg.Buffer[14]) << 24;
		alarm += ((unsigned int)msg.Buffer[15]) << 16;
		alarm += ((unsigned int)msg.Buffer[16]) << 8;
		alarm += msg.Buffer[17];
		N71_65_GetNoteAlarm(alarm, &(Data->Calendar->Time), &(Data->Calendar->Alarm) ,1);
		Data->Calendar->Time.Year = msg.Buffer[18]*256 + msg.Buffer[19];
		dprintf("Birthday date: %02i-%02i-%04i (age %d)\n",
			Data->Calendar->Time.Day,Data->Calendar->Time.Month,
			Data->Calendar->Time.Year,
			Data->Calendar->Alarm.Year - Data->Calendar->Time.Year);

		if (msg.Buffer[20]==0x00) Data->Calendar->SilentAlarm = true;
		dprintf("Alarm type   : %s\n", (msg.Buffer[20] ? "Tone  " : "Silent"));

		memcpy(Data->Calendar->Text,msg.Buffer+22,msg.Buffer[21]*2);
		Data->Calendar->Text[msg.Buffer[21]*2]=0;
		Data->Calendar->Text[msg.Buffer[21]*2+1]=0;
		dprintf("Text         : \"%s\"\n",DecodeUnicodeString(Data->Calendar->Text));

		Data->Calendar->Recurrance = 0;
		return GE_NONE;
	case 0x08:
		dprintf("Reminder\n");
		Data->Calendar->Type = GCN_REMINDER;
		N71_65_GetNoteTimes(msg.Buffer, Data->Calendar);
		return GE_NONE;
	default:
		dprintf("ERROR: unknown %i\n",msg.Buffer[6]);
		return GE_UNKNOWNRESPONSE;
	}
}

GSM_Error N71_65_ReplyGetCalendarNotePos(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User,int *FirstCalendarPos)
{
	dprintf("First calendar location: %i\n",msg.Buffer[4]*256+msg.Buffer[5]);
	*FirstCalendarPos = msg.Buffer[4]*256+msg.Buffer[5];
	return GE_NONE;
}

static GSM_Error N71_65_GetCalendarNotePos(GSM_StateMachine *s)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x31};

	dprintf("Getting first free calendar note location\n");
	return GSM_WaitFor (s, req, 4, 0x13, 4, ID_GetCalendarNotePos);
}

static long N71_65_GetNoteAlarmDiff(GSM_DateTime *time, GSM_DateTime *alarm)
{
	time_t     t_alarm;
	time_t     t_time;
	struct tm  tm_alarm;
	struct tm  tm_time;

	tzset();

	tm_alarm.tm_year	= alarm->Year - 1900;
	tm_alarm.tm_mon		= alarm->Month - 1;
	tm_alarm.tm_mday	= alarm->Day;
	tm_alarm.tm_hour	= alarm->Hour;
	tm_alarm.tm_min		= alarm->Minute;
	tm_alarm.tm_sec		= alarm->Second;
	tm_alarm.tm_isdst	= 0;
	t_alarm			= mktime(&tm_alarm);

	tm_time.tm_year		= time->Year - 1900;
	tm_time.tm_mon		= time->Month - 1;
	tm_time.tm_mday		= time->Day;
	tm_time.tm_hour		= time->Hour;
	tm_time.tm_min		= time->Minute;
	tm_time.tm_sec		= time->Second;
	tm_time.tm_isdst	= 0;
	t_time 			= mktime(&tm_time);

	dprintf("Alarm: %02i-%02i-%04i %02i:%02i:%02i\n",
			alarm->Day,alarm->Month,alarm->Year,
			alarm->Hour,alarm->Minute,alarm->Second);
	dprintf("Date: %02i-%02i-%04i %02i:%02i:%02i\n",
			time->Day,time->Month,time->Year,
			time->Hour,time->Minute,time->Second);
	dprintf("Difference in alarm time is %f\n",difftime(t_time, t_alarm)+3600);

	return ((long)(difftime(t_time,t_alarm)+3600));
}

GSM_Error N71_65_ReplySetCalendarNote(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
#ifdef DEBUG
	dprintf("Written Calendar Note type ");
	switch ((msg.Buffer[3]/2)-1) {
		case 0:	dprintf("Meeting");	break;
		case 1:	dprintf("Call");	break;
		case 2:	dprintf("Birthday");	break;
		case 3:	dprintf("Reminder");	break;
	}
	dprintf(" on location %d\n",msg.Buffer[4]*256+msg.Buffer[5]);
#endif
	return GE_NONE;
}

GSM_Error N71_65_SetCalendarNote(GSM_StateMachine *s, GSM_CalendarNote *Note, int *FirstCalendarPos)
{
	int			count=0;
	long			seconds, minutes;
 	GSM_Error		error;
	unsigned char 		req[200] = {
		N6110_FRAME_HEADER,
		0x01,				/* note type */
		0x00, 0x00,			/* location ? */
		0x00,				/* entry type */
		0x00,
		0x00, 0x00, 0x00, 0x00,		/* Year(2bytes), Month, Day */
		/* here starts block ... depends on note type */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00};                          

	if (Note->Location==0x00) return GE_INVALIDLOCATION;
	error=s->Phone.Functions->DeleteCalendarNote(s,Note);
	if (error!=GE_NONE && error!=GE_INVALIDLOCATION) return error;
	/*
	 * 6210/7110 needs to seek the first free pos to inhabit with next note
	 */
	error=N71_65_GetCalendarNotePos(s);
	if (error!=GE_NONE) return error;
	if (FirstCalendarPos != NULL) {
		req[4] = *FirstCalendarPos/256;
		req[5] = *FirstCalendarPos%256;
	}

	switch(Note->Type) {
		case GCN_CALL    : req[3]=0x03; req[6]=0x02; break;
		case GCN_BIRTHDAY: req[3]=0x05; req[6]=0x04; break;
		case GCN_REMINDER: req[3]=0x07; req[6]=0x08; break;
		case GCN_MEETING :
		default		 : req[3]=0x01; req[6]=0x01; break;
	}

	req[8]	= Note->Time.Year >> 8;
	req[9]	= Note->Time.Year & 0xff;
	req[10]	= Note->Time.Month;
	req[11]	= Note->Time.Day;

	count=12;
	switch(Note->Type)
	{
	case GCN_BIRTHDAY:
		req[count++] = 0x00;		/* 12 */
		req[count++] = 0x00;		/* 13 */
		/* Alarm */
		req[count++] = 0x00;		/* 14 */
		req[count++] = 0x00;		/* 15 */
		req[count++] = 0xff;		/* 16 */
		req[count++] = 0xff;		/* 17 */
		if(Note->Alarm.Year!=0) {
			/* I try with Time.Year = Alarm.Year. If negative, I increase 1 year,
			 * but only once ! This thing, because I may have Alarm period across
			 * a year. (eg. Birthday on 2001-01-10 and Alarm on 2000-12-27)
			 */
			Note->Time.Year = Note->Alarm.Year;
			if((seconds = N71_65_GetNoteAlarmDiff(&Note->Time,&Note->Alarm))<0L)
			{
				Note->Time.Year++;
				seconds = N71_65_GetNoteAlarmDiff(&Note->Time,&Note->Alarm);
			}
			if(seconds>=0L)
			{
				count -= 4;
				req[count++] = ((unsigned char)seconds>>24);		/* 14 */
				req[count++] = (seconds>>16) & 0xff;	/* 15 */
				req[count++] = (seconds>>8) & 0xff;	/* 16 */
				req[count++] = seconds & 0xff;		/* 17 */
			}
		}
		if (Note->SilentAlarm) req[count++] = 0x00; else req[count++] = 0x01;
		req[count++] = strlen(DecodeUnicodeString(Note->Text));	/* 19 */
		CopyUnicodeString(req+count,Note->Text);
		count=count+2*strlen(DecodeUnicodeString(Note->Text));	/* 22->N */
		break;
	case GCN_REMINDER:
		/* Recurrance - setting 1 Year repeat */
		if(Note->Recurrance >= 8760) Note->Recurrance = 0xffff;
		req[count++] = Note->Recurrance>>8;	/* 12 */
		req[count++] = Note->Recurrance&0xff;	/* 13 */
		/* len of text */
		req[count++] = strlen(DecodeUnicodeString(Note->Text));	/* 14 */
		req[count++]=0x00;					/* 15 */
		CopyUnicodeString(req+count,Note->Text);
		count=count+2*strlen(DecodeUnicodeString(Note->Text));	/* 16->N */
		break;
	    case GCN_MEETING:
	    case GCN_CALL:
	    default:
		req[count++] = Note->Time.Hour;   /* 12 */
		req[count++] = Note->Time.Minute; /* 13 */
		/* Alarm */
		req[count++] = 0xff;		  /* 14 */
		req[count++] = 0xff;		  /* 15 */
		if(Note->Alarm.Year!=0)
		{
			seconds=N71_65_GetNoteAlarmDiff(&Note->Time,&Note->Alarm);
			if(seconds>=0L)
			{
				minutes=seconds/60L;
				count -= 2;
				req[count++] = ((unsigned char)minutes>>8);
				req[count++] = minutes&0xff;
			}
		}
		/* Recurrance - setting 1 Year repeat */
		if(Note->Recurrance >= 8760) Note->Recurrance = 0xffff;
		req[count++] = Note->Recurrance >> 8;	/* 16 */
		req[count++] = Note->Recurrance & 0xff;	/* 17 */

		req[count++] = strlen(DecodeUnicodeString(Note->Text));	/* 18 */
		if (Note->Type==GCN_CALL) {
			req[count++] = strlen(DecodeUnicodeString(Note->Phone)); /* 19 */
		} else {
			req[count++] = 0x00;					 /* 19 */
		}
		CopyUnicodeString(req+count,Note->Text);
		count=count+2*strlen(DecodeUnicodeString(Note->Text));		 /* 20->N */
		if (Note->Type==GCN_CALL) {
			CopyUnicodeString(req+count,Note->Phone);
			count=count+2*strlen(DecodeUnicodeString(Note->Phone));  /* (N+1)->n */
		}
		break;
	}
	req[count] = 0x00;
	dprintf("Writing calendar note\n");
	return GSM_WaitFor (s, req, count, 0x13, 4, ID_SetCalendarNote);
}

GSM_Error N71_65_ReplyDeleteCalendarNote(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Deleted calendar note on location %d\n",msg.Buffer[4]*256+msg.Buffer[5]);
	return GE_NONE;
}

GSM_Error N71_65_DelCalendarNote(GSM_StateMachine *s, GSM_CalendarNote *Note, GSM_NOKIACalendarLocations *LastCalendar)
{
	GSM_Error 	error;
	unsigned char 	req[] = {
		N6110_FRAME_HEADER, 0x0b,
		0x00, 0x00};			/* location */

	if (Note->Location==0x00) return GE_INVALIDLOCATION;
	error=N71_65_GetCalendarInfo(s);
	if (error!=GE_NONE) return error;
	if (Note->Location > LastCalendar->Number) return GE_INVALIDLOCATION;

	req[4] = LastCalendar->Location[Note->Location-1] >> 8;
	req[5] = LastCalendar->Location[Note->Location-1] & 0xff;

	dprintf("Deleting calendar note\n");
	return GSM_WaitFor (s, req, 6, 0x13, 4, ID_DeleteCalendarNote);
}


bool NOKIA_FindPhoneFeatureValue(GSM_Profile_PhoneTableValue 	ProfileTable[],
				 GSM_Profile_Feat_ID		FeatureID,
				 GSM_Profile_Feat_Value		FeatureValue,
			    	 unsigned char 			*PhoneID,
			    	 unsigned char 			*PhoneValue) 
{
	int i=0;

	dprintf("Trying to find feature %i with value %i\n",FeatureID,FeatureValue);
	while (ProfileTable[i].ID != 0x00) {
		if (ProfileTable[i].ID == FeatureID &&
		    ProfileTable[i].Value == FeatureValue) {
			*PhoneID	= ProfileTable[i].PhoneID;
			*PhoneValue	= ProfileTable[i].PhoneValue;
			return true;
		}
		i++;
	}
	return false;
}

#define PROFILE_CALLERGROUPS_GROUP1      0x01
#define PROFILE_CALLERGROUPS_GROUP2      0x02
#define PROFILE_CALLERGROUPS_GROUP3      0x04
#define PROFILE_CALLERGROUPS_GROUP4      0x08
#define PROFILE_CALLERGROUPS_GROUP5      0x10

void NOKIA_FindFeatureValue(GSM_Profile_PhoneTableValue ProfileTable[],
			    unsigned char 		ID,
			    unsigned char 		Value,
			    GSM_Phone_Data 		*Data,
			    bool			CallerGroups) 
{
	int i;

	if (CallerGroups) {
		dprintf("Caller groups: %i\n", Value);
		Data->Profile->FeatureID [Data->Profile->FeaturesNumber] = Profile_CallerGroups;
		Data->Profile->FeaturesNumber++;
		for (i=0;i<5;i++) Data->Profile->CallerGroups[i] = false;
		if ((Value & PROFILE_CALLERGROUPS_GROUP1)==PROFILE_CALLERGROUPS_GROUP1) Data->Profile->CallerGroups[0] = true;
		if ((Value & PROFILE_CALLERGROUPS_GROUP2)==PROFILE_CALLERGROUPS_GROUP2) Data->Profile->CallerGroups[1] = true;
		if ((Value & PROFILE_CALLERGROUPS_GROUP3)==PROFILE_CALLERGROUPS_GROUP3) Data->Profile->CallerGroups[2] = true;
		if ((Value & PROFILE_CALLERGROUPS_GROUP4)==PROFILE_CALLERGROUPS_GROUP4) Data->Profile->CallerGroups[3] = true;
		if ((Value & PROFILE_CALLERGROUPS_GROUP5)==PROFILE_CALLERGROUPS_GROUP5) Data->Profile->CallerGroups[4] = true;
		return;
	}

	i = 0;
	while (ProfileTable[i].ID != 0x00) {
		if (ProfileTable[i].PhoneID == ID &&
		    ProfileTable[i].PhoneValue == Value) {
#ifdef DEBUG
			switch (ProfileTable[i].ID) {
			case Profile_KeypadTone		: dprintf("Keypad tones\n"); 	 	  break;
			case Profile_CallAlert		: dprintf("Call alert\n"); 		  break;
			case Profile_RingtoneVolume	: dprintf("Ringtone volume\n"); 	  break;
			case Profile_MessageTone	: dprintf("SMS message tones\n");  	  break;
			case Profile_Vibration		: dprintf("Vibration\n"); 		  break;
			case Profile_WarningTone	: dprintf("Warning (ang games) tones\n"); break;
			case Profile_AutoAnswer		: dprintf("Automatic answer\n"); 	  break;
			case Profile_Lights		: dprintf("Lights\n"); 			  break;
			case Profile_ScreenSaver	: dprintf("Screen Saver\n"); 		  break;
			case Profile_ScreenSaverTime	: dprintf("Screen Saver timeout\n");	  break;
			default				:					  break;
			}
#endif
			Data->Profile->FeatureID	[Data->Profile->FeaturesNumber] = ProfileTable[i].ID;
			Data->Profile->FeatureValue	[Data->Profile->FeaturesNumber] = ProfileTable[i].Value;
			Data->Profile->FeaturesNumber++;
			break;
		}
		i++;
	}
}

GSM_Profile_PhoneTableValue Profile71_65[] = {
	{Profile_KeypadTone,	 PROFILE_KEYPAD_OFF,		0x00,0x00},
	{Profile_KeypadTone,	 PROFILE_KEYPAD_LEVEL1,		0x00,0x01},
	{Profile_KeypadTone,	 PROFILE_KEYPAD_LEVEL2,		0x00,0x02},
	{Profile_KeypadTone,	 PROFILE_KEYPAD_LEVEL3,		0x00,0x03},
	/* Lights ? */
	{Profile_CallAlert,	 PROFILE_CALLALERT_RINGING,	0x02,0x00},
	{Profile_CallAlert,	 PROFILE_CALLALERT_ASCENDING,	0x02,0x01},
	{Profile_CallAlert,	 PROFILE_CALLALERT_RINGONCE,	0x02,0x02},
	{Profile_CallAlert,	 PROFILE_CALLALERT_BEEPONCE,	0x02,0x03},
	{Profile_CallAlert,	 PROFILE_CALLALERT_OFF,		0x02,0x05},
/*	{Profile_CallAlert,	 PROFILE_CALLALERT_CALLERGROUPS,0x02,0x07},	*/
	/* Ringtone ID */
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL1,		0x04,0x00},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL2,		0x04,0x01},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL3,		0x04,0x02},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL4,		0x04,0x03},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL5,		0x04,0x04},
	{Profile_MessageTone,	 PROFILE_MESSAGE_NOTONE,	0x05,0x00},
	{Profile_MessageTone,	 PROFILE_MESSAGE_STANDARD,	0x05,0x01},
	{Profile_MessageTone,	 PROFILE_MESSAGE_SPECIAL,	0x05,0x02},
	{Profile_MessageTone,	 PROFILE_MESSAGE_BEEPONCE,	0x05,0x03},
	{Profile_MessageTone,	 PROFILE_MESSAGE_ASCENDING,	0x05,0x04},
	{Profile_Vibration,	 PROFILE_VIBRATION_OFF,		0x06,0x00},
	{Profile_Vibration,	 PROFILE_VIBRATION_ON,		0x06,0x01},
	{Profile_WarningTone,	 PROFILE_WARNING_OFF,		0x07,0x00},
	{Profile_WarningTone,	 PROFILE_WARNING_ON,		0x07,0x01},
	/* Caller groups */
	{Profile_AutoAnswer,	 PROFILE_AUTOANSWER_OFF,	0x09,0x00},
	{Profile_AutoAnswer,	 PROFILE_AUTOANSWER_ON,		0x09,0x01},
	{0x00,			 0x00,				0x00,0x00}
};

GSM_Error NOKIA_SetIncomingSMS(GSM_StateMachine *s, bool enable)
{
	s->Phone.Data.EnableIncomingSMS = enable;
#ifdef DEBUG
	if (enable) {
		dprintf("Enabling incoming SMS\n");
	} else {
		dprintf("Disabling incoming SMS\n");
	}
#endif
	return GE_NONE;
}

GSM_Error N71_65_ReplyCallInfo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	GSM_Call 	call;
	int			tmp;
#ifdef DEBUG
	unsigned char 	buffer[200];

	switch (msg.Buffer[3]) {
	case 0x02 : dprintf("Call established, remote phone is ringing.\n"); 	break;
	case 0x03 : dprintf("Call complete\n"); 				break;
	case 0x04 : dprintf("Call hangup!\n"); 					break;
	case 0x05 : dprintf("Incoming call\n"); 				break;
	case 0x07 : dprintf("Call answer initiated.\n"); 			break;
	case 0x09 : dprintf("Call released.\n"); 				break;
	case 0x0a : dprintf("Call is being released.\n"); 			break;
	case 0x0c : dprintf("Audio status\n"); 					break;
	case 0x53 : dprintf("Outgoing call\n");  				break;
	}

	if (msg.Buffer[3] != 0x0c) dprintf("Call ID   : %i\n",msg.Buffer[4]);

	switch (msg.Buffer[3]) {
	case 0x03:
	case 0x05:
	case 0x53:		
		dprintf("Call mode : %i\n",msg.Buffer[5]);
		tmp = 6;
		NOKIA_GetUnicodeString(&tmp, msg.Buffer,buffer,false);
		dprintf("Number    : \"%s\"\n",DecodeUnicodeString(buffer));
		break;
	case 0x04:
		dprintf("Cause Type : %i\n", msg.Buffer[5]);
		dprintf("Cause ID   : %i\n", msg.Buffer[6]);
		break;
	case 0x0c:
		if (msg.Buffer[4] == 0x01) dprintf("Audio enabled\n");
				      else dprintf("Audio disabled\n");
		break;
	default:
		break;
	}
#endif
	if (User->IncomingCall) {
		switch (msg.Buffer[3]) {
		case 0x02:
			call.Status = GN_CALL_CallStart;
			break;
		case 0x04:
			call.Status = GN_CALL_CallRemoteEnd;
			break;
		case 0x05:
			call.Status = GN_CALL_IncomingCall;
			tmp = 6;
			NOKIA_GetUnicodeString(&tmp, msg.Buffer,call.PhoneNumber,false);
			break;
		case 0x09:
			call.Status = GN_CALL_CallLocalEnd;
			break;
		case 0x53:
			call.Status = GN_CALL_OutgoingCall;
			tmp = 6;
			NOKIA_GetUnicodeString(&tmp, msg.Buffer,call.PhoneNumber,false);
			break;
		default:
			return GE_NONE;
		}
		User->IncomingCall(Data->Device, call);
	}

	return GE_NONE;
}

#endif

unsigned char N71_65_MEMORY_TYPES[] = {
	GMT_DC,		0x01,
	GMT_MC,		0x02,
	GMT_RC,		0x03,
	GMT_ME,		0x05,
	GMT_SM,		0x06,
	GMT_VM,		0x09,
	GMT7110_CG,	0x10,
	GMT7110_SP,	0x0e,
	  0x00,		0x00
};

int N71_65_PackPBKBlock(int id, int size, int no, unsigned char *buf, unsigned char *block)
{
	dprintf("Adding block id:%i,number:%i,length:%i\n",id,no+1,size+6);

	*(block++) = id;
	*(block++) = 0;
	*(block++) = 0;
	*(block++) = size + 6;
	*(block++) = no + 1;
	memcpy(block, buf, size);
	block += size;
	*(block++) = 0;

	return (size + 6);
}

int N71_65_EncodePhonebookFrame(unsigned char *req, GSM_PhonebookEntry entry, int *block2, bool URL)
{
	int		count=0, len, i, block=0;
	char		string[500];
	unsigned char	type=0;
 		
	for (i = 0; i < entry.EntriesNum; i++)
	{
		switch (entry.Entries[i].EntryType) {
		case PBK_Number_General:
		case PBK_Number_Mobile:
		case PBK_Number_Work:
		case PBK_Number_Fax:
		case PBK_Number_Home:
			switch (entry.Entries[i].EntryType) {
			case PBK_Number_General:
				string[0] = N7110_NUMBER_GENERAL;	break;
			case PBK_Number_Mobile:
				string[0] = N7110_NUMBER_MOBILE;	break;
			case PBK_Number_Work:
				string[0] = N7110_NUMBER_WORK; 		break;
			case PBK_Number_Fax:
				string[0] = N7110_NUMBER_FAX; 		break;
			case PBK_Number_Home:
				string[0] = N7110_NUMBER_HOME;		break;
			default:					break;
			}
			len = strlen(DecodeUnicodeString(entry.Entries[i].Text));
			string[1] = 0;
			/* DCT 3 */			
			if (!URL) {
				string[2] = entry.Entries[i].VoiceTag;
			}
			string[3] = 0;
			string[4] = len * 2 + 2;     	/* length (with Termination) */
			CopyUnicodeString(string+5,entry.Entries[i].Text);
			string[len * 2 + 5] = 0; 	/* Terminating 0		 */
			count += N71_65_PackPBKBlock(N7110_ENTRYTYPE_NUMBER, len * 2 + 6, block++, string, req + count);
			/* DCT 4 */
			if (URL) {
				block++;
				req[count++] = N6510_ENTRYTYPE_VOICETAG;
				req[count++] = 0;
				req[count++] = 0;
				req[count++] = 8;
				req[count++] = 0x00;
				req[count++] = i+1;
				req[count++] = 0x00;
				req[count++] = entry.Entries[i].VoiceTag;
			}
			break;
		case PBK_Name:
		case PBK_Text_Note:
		case PBK_Text_Postal:
		case PBK_Text_Email:
		case PBK_Text_URL:
			len = strlen(DecodeUnicodeString(entry.Entries[i].Text));
			string[0] = len * 2 + 2;	/* length (with Termination) */
			CopyUnicodeString(string+1,entry.Entries[i].Text);
			string[len * 2 + 1] = 0; 	/* Terminating 0		 */
			switch (entry.Entries[i].EntryType) {
				case PBK_Text_Note:
					type = N7110_ENTRYTYPE_NOTE;	break;
				case PBK_Text_Postal:
					type = N7110_ENTRYTYPE_POSTAL;	break;
				case PBK_Text_Email:
					type = N7110_ENTRYTYPE_EMAIL;	break;
				case PBK_Text_URL:
					type = N7110_ENTRYTYPE_NOTE;
					if (URL) type = N6510_ENTRYTYPE_URL;
					break;
				case PBK_Name:
					type = N7110_ENTRYTYPE_NAME;	break;
				default:				break;
			}
			count += N71_65_PackPBKBlock(type, len * 2 + 2, block++, string, req + count);
			break;
		case PBK_Caller_Group:
			string[0] = entry.Entries[i].Number;
			string[1] = 0;
			count += N71_65_PackPBKBlock(N7110_ENTRYTYPE_GROUP, 2, block++, string, req + count);
			break;
		case PBK_Date:
			break;
		}
	}

	*block2=block;

	return count;
}

GSM_Error N71_65_DecodePhonebook(GSM_PhonebookEntry 	*entry,
				 GSM_Bitmap 		*bitmap,
				 GSM_SpeedDial 		*speed,
				 unsigned char 		*MessageBuffer,
				 int 			MessageLength)
{
	unsigned char 	*Block;
	int		length = 0;
	bool		WasBitmap = false;
	            
	entry->EntriesNum = 0;

	if (entry->MemoryType==GMT7110_CG) {
		bitmap->Text[0] = 0x00;
		bitmap->Text[1] = 0x00;
	}

	Block = &MessageBuffer[0];
	while (length != MessageLength) {
#ifdef DEBUG
		dprintf("Phonebook entry block - length %i", Block[3]-6);
		if (di.dl == DL_TEXTALL) DumpMessage(di.df, Block+5, Block[3]-6);
#endif
		if (entry->EntriesNum==GSM_PHONEBOOK_ENTRIES) {
			dprintf("Too many entries\n");
			return GE_UNKNOWNRESPONSE;
		}

		switch (Block[0]) {
		case N7110_ENTRYTYPE_NAME:
		case N7110_ENTRYTYPE_EMAIL:
		case N7110_ENTRYTYPE_POSTAL:
		case N7110_ENTRYTYPE_NOTE:
		case N6510_ENTRYTYPE_URL:
			if (Block[5]/2>GSM_PHONEBOOK_TEXT_LENGTH) {
				dprintf("Too long text\n");
				return GE_UNKNOWNRESPONSE;
			}
			memcpy(entry->Entries[entry->EntriesNum].Text,Block+6,Block[5]);
			switch (Block[0]) {
			case N7110_ENTRYTYPE_NAME:
				entry->Entries[entry->EntriesNum].EntryType=PBK_Name;
				if (entry->MemoryType==GMT7110_CG) {
					memcpy(bitmap->Text,Block+6,Block[5]);
				}
				dprintf("   Name \"%s\"\n",DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
				break;
			case N7110_ENTRYTYPE_EMAIL:
				entry->Entries[entry->EntriesNum].EntryType=PBK_Text_Email;
				dprintf("   Email \"%s\"\n",DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
				break;
			case N7110_ENTRYTYPE_POSTAL:
				entry->Entries[entry->EntriesNum].EntryType=PBK_Text_Postal;
				dprintf("   Postal \"%s\"\n",DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
				break;
			case N7110_ENTRYTYPE_NOTE:
				entry->Entries[entry->EntriesNum].EntryType=PBK_Text_Note;
				dprintf("   Note \"%s\"\n",DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
				break;
			case N6510_ENTRYTYPE_URL:
				entry->Entries[entry->EntriesNum].EntryType=PBK_Text_URL;
				dprintf("   URL \"%s\"\n",DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
				break;
			}
			entry->EntriesNum ++;
			break;
		case N7110_ENTRYTYPE_DATE:
			entry->Entries[entry->EntriesNum].EntryType=PBK_Date;
			NOKIA_DecodeDateTime(Block+6, &entry->Entries[entry->EntriesNum].Date);
			entry->EntriesNum ++;
			break;
		case N7110_ENTRYTYPE_NUMBER:
			switch (Block[5]) {
			case 0x00:
			case 0x01:	/* Not assigned dialed number */
			case 0x0b:
			case N7110_NUMBER_GENERAL:
				dprintf("  General number");
				entry->Entries[entry->EntriesNum].EntryType=PBK_Number_General;
				break;
			case N7110_NUMBER_WORK:
				dprintf("  Work number");
				entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Work;
				break;
			case N7110_NUMBER_FAX:
				dprintf("  Fax number");
				entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Fax;
				break;
			case N7110_NUMBER_MOBILE:
				dprintf("  Mobile number");
				entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Mobile;
				break;
			case N7110_NUMBER_HOME:
				dprintf("  Home number");
				entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Home;
				break;
			default:
				dprintf("Unknown number type %02x\n",Block[5]);
				return GE_UNKNOWNRESPONSE;
			}
			if (Block[9]/2>GSM_PHONEBOOK_TEXT_LENGTH) {
				dprintf("Too long text\n");
				return GE_UNKNOWNRESPONSE;
			}
			memcpy(entry->Entries[entry->EntriesNum].Text,Block+10,Block[9]);
			dprintf(" \"%s\"\n",DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
			/* DCT3 phones like 6210 */
			entry->Entries[entry->EntriesNum].VoiceTag = Block[7];
#ifdef DEBUG
			if (entry->Entries[entry->EntriesNum].VoiceTag != 0) dprintf("Voice tag %i assigned\n",Block[7]);
#endif
			entry->EntriesNum ++;
			break;
		case N7110_ENTRYTYPE_RINGTONE:
			if (entry->MemoryType==GMT7110_CG) {
				bitmap->Ringtone=Block[5];
				dprintf("Ringtone ID : %i\n",Block[5]);
			} else return GE_UNKNOWNRESPONSE;
			break;
		case N7110_ENTRYTYPE_LOGOON:
			if (entry->MemoryType==GMT7110_CG) {
				bitmap->Enabled=(Block[5]==0x00 ? false : true);
				dprintf("Logo : %s\n", bitmap->Enabled==true ? "enabled":"disabled");
			} else return GE_UNKNOWNRESPONSE;
			break;
		case N7110_ENTRYTYPE_GROUPLOGO:
			if (entry->MemoryType==GMT7110_CG) {
				dprintf("Caller logo\n");
				PHONE_DecodeBitmap(GSM_NokiaCallerLogo, Block+10, bitmap);
				WasBitmap = true;
			} else return GE_UNKNOWNRESPONSE;
			break;
		case N7110_ENTRYTYPE_GROUP:
			entry->Entries[entry->EntriesNum].EntryType=PBK_Caller_Group;
			dprintf("Caller group \"%i\"\n",Block[5]);
			entry->Entries[entry->EntriesNum].Number=Block[5];
			if (Block[5]!=0) entry->EntriesNum ++;
			break;
		case N6510_ENTRYTYPE_VOICETAG:
			dprintf("Entry %i has voice tag %i\n",Block[5]-1,Block[7]);
			entry->Entries[Block[5]-1].VoiceTag = Block[7];
			break;
		/* 6210 5.56, SIM speed dials or ME with 1 number */
		case N7110_ENTRYTYPE_SIM_SPEEDDIAL:
			if (entry->MemoryType==GMT7110_SP) {
#ifdef DEBUG
				dprintf("location %i\n",(Block[6]*256+Block[7]));
#endif			
				speed->MemoryType = GMT_ME;
				if (Block[8] == 0x06) speed->MemoryType = GMT_SM;
				speed->MemoryLocation 	= (Block[6]*256+Block[7]);
				speed->MemoryNumberID 	= 2;
			} else return GE_UNKNOWNRESPONSE;
			break;
		case N7110_ENTRYTYPE_SPEEDDIAL:
			if (entry->MemoryType==GMT7110_SP) {
#ifdef DEBUG
				switch (Block[12]) {
					case 0x05: dprintf("ME\n"); break;
					case 0x06: dprintf("SM\n"); break;
					default	 : dprintf("%02x\n",Block[12]);
				}
				dprintf("location %i, number %i in location\n",
					(Block[6]*256+Block[7])-1,Block[14]);
#endif			
				switch (Block[12]) {
					case 0x05: speed->MemoryType = GMT_ME; break;
					case 0x06: speed->MemoryType = GMT_SM; break;
				}
				speed->MemoryLocation = (Block[6]*256+Block[7])-1;
				speed->MemoryNumberID = Block[14];
			} else return GE_UNKNOWNRESPONSE;
			break;
		default:
			dprintf("ERROR: unknown pbk entry %i\n",Block[0]);
			return GE_UNKNOWNRESPONSE;
		}
		length=length + Block[3];
		Block = &Block[(int) Block[3]];
	}

	/* In DCT4 default caller logos are NOT return */
	if (!WasBitmap && entry->MemoryType==GMT7110_CG) return GE_SECURITYERROR;

	return GE_NONE;
}

void NOKIA_GetDefaultCallerGroupName(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	Bitmap->DefaultName = false;
	if (Bitmap->Text[0]==0x00 && Bitmap->Text[1]==0x00) {
		Bitmap->DefaultName = true;
		switch(Bitmap->Location) {
		case 1:
			EncodeUnicode(Bitmap->Text,GetMsg(s->msg,"Family"),strlen(GetMsg(s->msg,"Family")));
			break;
		case 2:
			EncodeUnicode(Bitmap->Text,GetMsg(s->msg,"VIP"),strlen(GetMsg(s->msg,"VIP")));
			break;
		case 3:
			EncodeUnicode(Bitmap->Text,GetMsg(s->msg,"Friends"),strlen(GetMsg(s->msg,"Friends")));
			break;
		case 4:
			EncodeUnicode(Bitmap->Text,GetMsg(s->msg,"Colleagues"),strlen(GetMsg(s->msg,"Colleagues")));
			break;
		case 5:
			EncodeUnicode(Bitmap->Text,GetMsg(s->msg,"Other"),strlen(GetMsg(s->msg,"Other")));
			break;
		}
	}
}

void NOKIA_DecodeDateTime(unsigned char* buffer, GSM_DateTime *datetime)
{
	datetime->Year	= buffer[0] * 256 + buffer[1];
	datetime->Month	= buffer[2];
	datetime->Day	= buffer[3];

	datetime->Hour	 = buffer[4];
	datetime->Minute = buffer[5];
	datetime->Second = buffer[6];

	dprintf("Decoding date and time\n");
	dprintf("   Time: %02d:%02d:%02d\n",
		datetime->Hour, datetime->Minute, datetime->Second);
	dprintf("   Date: %4d/%02d/%02d\n",
		datetime->Year, datetime->Month, datetime->Day);
}
