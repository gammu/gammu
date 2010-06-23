
#include <string.h>

#include "gsmcal.h"
#include "../misc/coding.h"

void GSM_CalendarFindDefaultTextTimeAlarmPhoneRecurrance(GSM_CalendarEntry entry, int *Text, int *Time, int *Alarm, int *Phone, int *Recurrance)
{
	int i;

	*Text		= -1;
	*Time		= -1;
	*Alarm		= -1;
	*Phone		= -1;
	*Recurrance	= -1;
	for (i = 0; i < entry.EntriesNum; i++)
	{
		switch (entry.Entries[i].EntryType) {
		case CAL_START_DATETIME :
			if (*Time == -1) *Time = i;
			break;
		case CAL_ALARM_DATETIME :
		case CAL_SILENT_ALARM_DATETIME:
			if (*Alarm == -1) *Alarm = i;
			break;
		case CAL_RECURRANCE:
			if (*Recurrance == -1) *Recurrance = i;
			break;
		case CAL_TEXT:
			if (*Text == -1) *Text = i;
			break;
		case CAL_PHONE:
			if (*Phone == -1) *Phone = i;
			break;
		default:
			break;
		}
	}
}

GSM_Error NOKIA_EncodeVCALENDAR10SMSText(char *Buffer, int *Length, GSM_CalendarEntry note)
{
	char 	buffer[1000];
 	int 	Text, Time, Alarm, Phone, Recurrance;

	GSM_CalendarFindDefaultTextTimeAlarmPhoneRecurrance(note, &Text, &Time, &Alarm, &Phone, &Recurrance);

	*Length+=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);
	*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c",13,10);
	*Length+=sprintf(Buffer+(*Length), "BEGIN:VEVENT%c%c",13,10);
	*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
	switch (note.Type) {
		case GCN_REMINDER:
			*Length+=sprintf(Buffer+(*Length), "MISCELLANEOUS%c%c",13,10);
			break;
		case GCN_CALL:
			*Length+=sprintf(Buffer+(*Length), "PHONE CALL%c%c",13,10);
			break;
		case GCN_BIRTHDAY:
			*Length+=sprintf(Buffer+(*Length), "SPECIAL OCCASION%c%c",13,10);
			break;
		case GCN_MEETING:
		default		:
			*Length+=sprintf(Buffer+(*Length), "MEETING%c%c",13,10);
			break;
	}
	if (note.Type == GCN_CALL && Phone != -1)
	{
		EncodeUTF8(buffer,note.Entries[Phone].Text);
		if (strlen(DecodeUnicodeString(note.Entries[Phone].Text))==strlen(buffer)) {
			*Length+=sprintf(Buffer+(*Length), "SUMMARY:%s%c%c",DecodeUnicodeString(note.Entries[Phone].Text),13,10);
		} else {
			*Length+=sprintf(Buffer+(*Length), "SUMMARY;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c",buffer,13,10);
		}	    
	} else {
		if (Text == -1) return GE_UNKNOWN;
		EncodeUTF8(buffer,note.Entries[Text].Text);
		if (strlen(DecodeUnicodeString(note.Entries[Text].Text))==strlen(buffer)) {
			*Length+=sprintf(Buffer+(*Length), "SUMMARY:%s%c%c",DecodeUnicodeString(note.Entries[Text].Text),13,10);
		} else {
			*Length+=sprintf(Buffer+(*Length), "SUMMARY;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c",buffer,13,10);
		}	    
	}

	if (Time == -1) return GE_UNKNOWN;
	*Length+=sprintf(Buffer+(*Length), "DTSTART:%04d%02d%02dT%02d%02d%02d%c%c",
			note.Entries[Time].Date.Year, note.Entries[Time].Date.Month, note.Entries[Time].Date.Day,
			note.Entries[Time].Date.Hour, note.Entries[Time].Date.Minute, note.Entries[Time].Date.Second,13,10);

	if (Alarm != -1) {
		*Length+=sprintf(Buffer+(*Length), "DALARM:%04d%02d%02dT%02d%02d%02d%c%c",
				note.Entries[Alarm].Date.Year, note.Entries[Alarm].Date.Month, note.Entries[Alarm].Date.Day,
				note.Entries[Alarm].Date.Hour, note.Entries[Alarm].Date.Minute, note.Entries[Alarm].Date.Second,13,10);
	}

	/* Birthday is known to be recurranced */
	if (Recurrance != -1 && note.Type != GCN_BIRTHDAY) {
		switch(note.Entries[Recurrance].Number/24) {
			case 1	 : *Length+=sprintf(Buffer+(*Length), "RRULE:D1 #0%c%c",13,10);	 break;
			case 7	 : *Length+=sprintf(Buffer+(*Length), "RRULE:W1 #0%c%c",13,10);	 break;
			case 14	 : *Length+=sprintf(Buffer+(*Length), "RRULE:W2 #0%c%c",13,10);	 break;
			case 365 : *Length+=sprintf(Buffer+(*Length), "RRULE:YD1 #0%c%c",13,10); break;
		}
	}

	*Length+=sprintf(Buffer+(*Length), "END:VEVENT%c%c",13,10);
	*Length+=sprintf(Buffer+(*Length), "END:VCALENDAR%c%c",13,10);

	return GE_NONE;
}

bool IsNoteFromThePast(GSM_CalendarEntry note)
{
	bool 		Past = true;
	int		i;
	GSM_DateTime	DT;

	GSM_GetCurrentDateTime (&DT);
	for (i = 0; i < note.EntriesNum; i++)
	{
		switch (note.Entries[i].EntryType) {
		case CAL_RECURRANCE:
			Past = false;
			break;
		case CAL_START_DATETIME :
			if (note.Entries[i].Date.Year > DT.Year) Past = false;
			if (note.Entries[i].Date.Year == DT.Year &&
			    note.Entries[i].Date.Month > DT.Month) Past = false;
			if (note.Entries[i].Date.Year == DT.Year &&
			    note.Entries[i].Date.Month == DT.Month &&
			    note.Entries[i].Date.Day > DT.Day) Past = false;
			break;
		default:
			break;
		}
		if (!Past) break;
	}
	switch (note.Type) {
		case GCN_BIRTHDAY:
			Past = false;
			break;
		default:
			break;
	}
	return Past;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
