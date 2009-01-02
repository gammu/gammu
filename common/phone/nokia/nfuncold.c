/* (c) 2003 by Marcin Wiacek */

#include <string.h> /* memcpy only */
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include "../../misc/coding/coding.h"
#include "../../gsmstate.h"
#include "../pfunc.h"
#include "nfunc.h"

#ifdef DEBUG
static void N71_65_GetCalendarAlarm(GSM_StateMachine *s, unsigned char *buffer, GSM_CalendarEntry *entry, int DT, GSM_Phone_Data *Data)
{
	unsigned long diff;

	if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0xff && buffer[3] == 0xff) {
		smprintf(s, "No alarm\n");
	} else {
		memcpy(&entry->Entries[entry->EntriesNum].Date,&entry->Entries[DT].Date,sizeof(GSM_DateTime));

		diff  = ((unsigned int)buffer[0]) << 24;
		diff += ((unsigned int)buffer[1]) << 16;
		diff += ((unsigned int)buffer[2]) << 8;
		diff += buffer[3];
		smprintf(s, "  Difference : %li seconds\n", diff);

		switch (entry->Type) {
		case GSM_CAL_MEETING:
			GetTimeDifference(diff, &entry->Entries[entry->EntriesNum].Date, false, 60);
			break;
		case GSM_CAL_MEMO:
			if (!GSM_IsPhoneFeatureAvailable(Data->ModelInfo, F_CAL35)) {
				GetTimeDifference(diff, &entry->Entries[entry->EntriesNum].Date, false, 60);
				break;
			}
		case GSM_CAL_CALL:
			if (!GSM_IsPhoneFeatureAvailable(Data->ModelInfo, F_CAL35)) {
				GetTimeDifference(diff, &entry->Entries[entry->EntriesNum].Date, false, 60);
				break;
			}
		default:
			GetTimeDifference(diff, &entry->Entries[entry->EntriesNum].Date, false, 1);
		}
		smprintf(s, "Alarm date   : %02i-%02i-%04i %02i:%02i:%02i\n",
			entry->Entries[entry->EntriesNum].Date.Day,   entry->Entries[entry->EntriesNum].Date.Month,
			entry->Entries[entry->EntriesNum].Date.Year,  entry->Entries[entry->EntriesNum].Date.Hour,
			entry->Entries[entry->EntriesNum].Date.Minute,entry->Entries[entry->EntriesNum].Date.Second);

		entry->Entries[entry->EntriesNum].EntryType = CAL_TONE_ALARM_DATETIME;
		if (entry->Type == GSM_CAL_BIRTHDAY) {
			if (buffer[14]!=0x00) entry->Entries[entry->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
			smprintf(s, "Alarm type   : Silent\n");
		}

		entry->EntriesNum++;
	}
}

/* method 2 */
GSM_Error N71_65_ReplyGetNextCalendar2(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_DateTime 		Date;
	GSM_CalendarEntry	*entry = s->Phone.Data.Cal;
	GSM_Phone_Data		*Data = &s->Phone.Data;
	int			i;
	unsigned long		diff;

	smprintf(s, "Calendar note received method 2\n");

	if (msg.Length < 10) return ERR_EMPTY;

	entry->Location = msg.Buffer[4]*256 + msg.Buffer[5];
	smprintf(s, "Location: %i\n",entry->Location);

	/* Not birthday */
	if (msg.Buffer[21] != 0x04) {
		Date.Year 	= 2030;	Date.Month 	= 01; Date.Day    = 01;
		Date.Hour 	= 00;	Date.Minute 	= 00; Date.Second = 00;
	} else {
		Date.Year 	= 2029; Date.Month 	= 12; Date.Day 	  = 31;
		Date.Hour 	= 22;   Date.Minute 	= 59; Date.Second = 58;
	}
	diff  = ((unsigned int)msg.Buffer[12]) << 24;
	diff += ((unsigned int)msg.Buffer[13]) << 16;
	diff += ((unsigned int)msg.Buffer[14]) << 8;
	diff += msg.Buffer[15];
	smprintf(s, "  Difference : %li seconds\n", diff);
	GetTimeDifference(diff, &Date, true, 1);
	Date.Year += 20;
	entry->Entries[0].EntryType = CAL_START_DATETIME;

	smprintf(s, "Note type %02x: ",msg.Buffer[21]);
	switch (msg.Buffer[21]) {
	case 0x01:
	case 0x08:
		if (msg.Buffer[21] == 0x01) {
			smprintf(s, "Meeting or Reminder\n");
			entry->Type = GSM_CAL_MEETING;
		} else {
			smprintf(s, "Memo\n");
			Data->Cal->Type = GSM_CAL_MEMO;
		}

		memcpy(&entry->Entries[0].Date,&Date,sizeof(GSM_DateTime));
		entry->EntriesNum++;

		N71_65_GetCalendarAlarm(s, msg.Buffer+16, entry, 0, Data);
		GSM_GetCalendarRecurranceRepeat(&(s->di), msg.Buffer+22, NULL, entry);

		memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+30, msg.Buffer[28]*2);
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[28]*2]   = 0;
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[28]*2+1] = 0;
		entry->Entries[entry->EntriesNum].EntryType		   = CAL_TEXT;
		break;
	case 0x02:
		smprintf(s, "Call\n");
		entry->Type = GSM_CAL_CALL;

		memcpy(&entry->Entries[0].Date,&Date,sizeof(GSM_DateTime));
		entry->EntriesNum++;

		N71_65_GetCalendarAlarm(s, msg.Buffer+16, entry, 0, Data);
		GSM_GetCalendarRecurranceRepeat(&(s->di), msg.Buffer+22, NULL, entry);

		i = msg.Buffer[28] * 2;
		if (i!=0) {
			memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+30, i);
			entry->Entries[entry->EntriesNum].Text[i]   	= 0;
			entry->Entries[entry->EntriesNum].Text[i+1] 	= 0;
			entry->Entries[entry->EntriesNum].EntryType	= CAL_PHONE;
			smprintf(s, "Phone        : \"%s\"\n", DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
			entry->EntriesNum++;
		}

		memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+30+i, msg.Buffer[29]*2);
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[29]*2]   = 0;
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[29]*2+1] = 0;
		entry->Entries[entry->EntriesNum].EntryType		   = CAL_TEXT;
		break;
	case 0x04:
		smprintf(s, "Birthday\n");
		Data->Cal->Type = GSM_CAL_BIRTHDAY;

		/* Year was set earlier */
		entry->Entries[0].Date.Month	= Date.Month;
		entry->Entries[0].Date.Day	= Date.Day;
		entry->Entries[0].Date.Hour	= 23;
		entry->Entries[0].Date.Minute	= 59;
		entry->Entries[0].Date.Second	= 58;
		entry->EntriesNum++;

		N71_65_GetCalendarAlarm(s, msg.Buffer+16, entry, 0, Data);
		GSM_GetCalendarRecurranceRepeat(&(s->di), msg.Buffer+22, NULL, entry);

		/* Birthday year */
		entry->Entries[0].Date.Year = msg.Buffer[28]*256 + msg.Buffer[29];
		if (msg.Buffer[28] == 0xff && msg.Buffer[29] == 0xff) entry->Entries[0].Date.Year = 0;
		smprintf(s, "Birthday date: %02i-%02i-%04i\n",
			entry->Entries[0].Date.Day,entry->Entries[0].Date.Month,
			entry->Entries[0].Date.Year);

		memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+32, msg.Buffer[31]*2);
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[31]*2]   = 0;
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[31]*2+1] = 0;
		entry->Entries[entry->EntriesNum].EntryType		   = CAL_TEXT;
		break;
	default:
		smprintf(s, "ERROR: unknown %i\n",msg.Buffer[6]);
		return ERR_UNKNOWNRESPONSE;
	}
	smprintf(s, "Text         : \"%s\"\n", DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
	entry->EntriesNum++;
	return ERR_NONE;
}

/* method 2 */
/* Note: in known phones texts of notes cut to 50 chars */
GSM_Error N71_65_GetNextCalendar2(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start, int *LastCalendarYear, int *LastCalendarPos)
{
	GSM_Error		error;
	GSM_DateTime		date_time;
	unsigned char 		req[] = {N6110_FRAME_HEADER, 0x3e,
					 0xFF, 0xFE};		/* Location */

	if (start) {
		/* We have to get current year. It's NOT written in frame for
		 * Birthday
		 */
		error=s->Phone.Functions->GetDateTime(s,&date_time);
		switch (error) {
			case ERR_EMPTY:
			case ERR_NOTIMPLEMENTED:
				GSM_GetCurrentDateTime(&date_time);
				break;
			case ERR_NONE:
				break;
			default:
				return error;
		}
		*LastCalendarYear = date_time.Year;

		/* First location at all */
		req[4] = 0xFF;
		req[5] = 0xFE;
	} else {
		req[4] = *LastCalendarPos / 256;
		req[5] = *LastCalendarPos % 256;
	}
	Note->EntriesNum		= 0;
	Note->Entries[0].Date.Year 	= *LastCalendarYear;

	s->Phone.Data.Cal = Note;
	smprintf(s, "Getting calendar note method 2\n");
	error=GSM_WaitFor (s, req, 6, 0x13, 4, ID_GetCalendarNote);
	*LastCalendarPos = Note->Location;
	return error;
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
