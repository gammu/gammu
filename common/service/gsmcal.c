/* (c) 2002-2003 by Marcin Wiacek */

#include <string.h>

#include "gsmcal.h"
#include "gsmmisc.h"
#include "../misc/coding/coding.h"

bool IsCalendarNoteFromThePast(GSM_CalendarEntry *note)
{
	bool 		Past = true;
	int		i;
	GSM_DateTime	DT;

	GSM_GetCurrentDateTime (&DT);
	for (i = 0; i < note->EntriesNum; i++) {
		switch (note->Entries[i].EntryType) {
		case CAL_RECURRANCE:
			Past = false;
			break;
		case CAL_START_DATETIME :
			if (note->Entries[i].Date.Year > DT.Year) Past = false;
			if (note->Entries[i].Date.Year == DT.Year &&
			    note->Entries[i].Date.Month > DT.Month) Past = false;
			if (note->Entries[i].Date.Year == DT.Year &&
			    note->Entries[i].Date.Month == DT.Month &&
			    note->Entries[i].Date.Day > DT.Day) Past = false;
			break;
		default:
			break;
		}
		if (!Past) break;
	}
	switch (note->Type) {
		case GSM_CAL_BIRTHDAY:
			Past = false;
			break;
		default:
			break;
	}
	return Past;
}

void GSM_CalendarFindDefaultTextTimeAlarmPhoneRecurrance(GSM_CalendarEntry *entry, int *Text, int *Time, int *Alarm, int *Phone, int *Recurrance, int *EndTime, int *Location)
{
	int i;

	*Text		= -1;
	*Time		= -1;
	*Alarm		= -1;
	*Phone		= -1;
	*Recurrance	= -1;
	*EndTime	= -1;
	*Location	= -1;
	for (i = 0; i < entry->EntriesNum; i++) {
		switch (entry->Entries[i].EntryType) {
		case CAL_START_DATETIME :
			if (*Time == -1) *Time = i;
			break;
		case CAL_END_DATETIME :
			if (*EndTime == -1) *EndTime = i;
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
		case CAL_LOCATION:
			if (*Location == -1) *Location = i;
			break;
		default:
			break;
		}
	}
}

GSM_Error GSM_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version)
{
 	int 	Text, Time, Alarm, Phone, Recurrance, EndTime, Location;
	char 	buffer[2000];

	GSM_CalendarFindDefaultTextTimeAlarmPhoneRecurrance(note, &Text, &Time, &Alarm, &Phone, &Recurrance, &EndTime, &Location);

	if (header) {
		*Length+=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);
		*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c",13,10);
	}
	*Length+=sprintf(Buffer+(*Length), "BEGIN:VEVENT%c%c",13,10);

	if (Version == Nokia_VCalendar) {
		*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
		switch (note->Type) {
		case GSM_CAL_REMINDER:
			*Length+=sprintf(Buffer+(*Length), "REMINDER%c%c",13,10);
			break;
		case GSM_CAL_MEMO:
			*Length+=sprintf(Buffer+(*Length), "MISCELLANEOUS%c%c",13,10);
			break;
		case GSM_CAL_CALL:
			*Length+=sprintf(Buffer+(*Length), "PHONE CALL%c%c",13,10);
			break;
		case GSM_CAL_BIRTHDAY:
			*Length+=sprintf(Buffer+(*Length), "SPECIAL OCCASION%c%c",13,10);
			break;
		case GSM_CAL_MEETING:
		default:
			*Length+=sprintf(Buffer+(*Length), "MEETING%c%c",13,10);
			break;
		}
		if (note->Type == GSM_CAL_CALL) {
			buffer[0] = 0;
			buffer[1] = 0;
		 	if (Phone != -1) CopyUnicodeString(buffer,note->Entries[Phone].Text);
			if (Text != -1)  {
				if (Phone != -1) EncodeUnicode(buffer+UnicodeLength(buffer)*2," ",1);
				CopyUnicodeString(buffer+UnicodeLength(buffer)*2,note->Entries[Text].Text);
			}
			SaveVCALText(Buffer, Length, buffer, "SUMMARY");
		} else {
			SaveVCALText(Buffer, Length, note->Entries[Text].Text, "SUMMARY");
		}
		if (note->Type == GSM_CAL_MEETING && Location != -1) {
			SaveVCALText(Buffer, Length, note->Entries[Location].Text, "LOCATION");
		}
	
		if (Time == -1) return ERR_UNKNOWN;
		SaveVCALDateTime(Buffer, Length, &note->Entries[Time].Date, "DTSTART");

		if (EndTime != -1) {
			SaveVCALDateTime(Buffer, Length, &note->Entries[EndTime].Date, "DTEND");
		}

		if (Alarm != -1) {
			if (note->Entries[Alarm].EntryType == CAL_SILENT_ALARM_DATETIME) {
				SaveVCALDateTime(Buffer, Length, &note->Entries[Alarm].Date, "DALARM");
			} else {
				SaveVCALDateTime(Buffer, Length, &note->Entries[Alarm].Date, "AALARM");
			}
		}

		/* Birthday is known to be recurranced */
		if (Recurrance != -1 && note->Type != GSM_CAL_BIRTHDAY) {
			switch(note->Entries[Recurrance].Number/24) {
				case 1	 : *Length+=sprintf(Buffer+(*Length), "RRULE:D1 #0%c%c",13,10);	 break;
				case 7	 : *Length+=sprintf(Buffer+(*Length), "RRULE:W1 #0%c%c",13,10);	 break;
				case 14	 : *Length+=sprintf(Buffer+(*Length), "RRULE:W2 #0%c%c",13,10);	 break;
				case 365 : *Length+=sprintf(Buffer+(*Length), "RRULE:YD1 #0%c%c",13,10); break;
			}
		}
	} else if (Version == Siemens_VCalendar) {
		*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
		switch (note->Type) {
		case GSM_CAL_MEETING:
			*Length+=sprintf(Buffer+(*Length), "MEETING%c%c",13,10);
			break;
		case GSM_CAL_CALL:
			*Length+=sprintf(Buffer+(*Length), "PHONE CALL%c%c",13,10);
			break;
		case GSM_CAL_BIRTHDAY:
			*Length+=sprintf(Buffer+(*Length), "ANNIVERSARY%c%c",13,10);
			break;
		case GSM_CAL_MEMO:
		default:
			*Length+=sprintf(Buffer+(*Length), "MISCELLANEOUS%c%c",13,10);
			break;
		}
		
		if (Time == -1) return ERR_UNKNOWN;
		SaveVCALDateTime(Buffer, Length, &note->Entries[Time].Date, "DTSTART");

		if (Alarm != -1) {
			SaveVCALDateTime(Buffer, Length, &note->Entries[Alarm].Date, "DALARM");
		}

		if (Recurrance != -1) {
			switch(note->Entries[Recurrance].Number/24) {
				case 1	 : *Length+=sprintf(Buffer+(*Length), "RRULE:D1%c%c",13,10);	break;
				case 7	 : *Length+=sprintf(Buffer+(*Length), "RRULE:D7%c%c",13,10);	break;
				case 30	 : *Length+=sprintf(Buffer+(*Length), "RRULE:MD1%c%c",13,10);	break;
				case 365 : *Length+=sprintf(Buffer+(*Length), "RRULE:YD1%c%c",13,10);	break;
			}
		}
	
		if (note->Type == GSM_CAL_CALL) {
			buffer[0] = 0;
			buffer[1] = 0;
		 	if (Phone != -1) CopyUnicodeString(buffer,note->Entries[Phone].Text);
			if (Text != -1)  {
				if (Phone != -1) EncodeUnicode(buffer+UnicodeLength(buffer)*2," ",1);
				CopyUnicodeString(buffer+UnicodeLength(buffer)*2,note->Entries[Text].Text);
			}
			SaveVCALText(Buffer, Length, buffer, "DESCRIPTION");
		} else {
			SaveVCALText(Buffer, Length, note->Entries[Text].Text, "DESCRIPTION");
		}
	} else if (Version == SonyEricsson_VCalendar) {
		*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
		switch (note->Type) {
		case GSM_CAL_MEETING:
			*Length+=sprintf(Buffer+(*Length), "MEETING%c%c",13,10);
			break;
		case GSM_CAL_REMINDER:
			*Length+=sprintf(Buffer+(*Length), "DATE%c%c",13,10);
			break;
		case GSM_CAL_TRAVEL:
			*Length+=sprintf(Buffer+(*Length), "TRAVEL%c%c",13,10);
			break;
		case GSM_CAL_VACATION:
			*Length+=sprintf(Buffer+(*Length), "VACATION%c%c",13,10);
			break;
		case GSM_CAL_BIRTHDAY:
			*Length+=sprintf(Buffer+(*Length), "ANNIVERSARY%c%c",13,10);
			break;
		case GSM_CAL_MEMO:
		default:
			*Length+=sprintf(Buffer+(*Length), "MISCELLANEOUS%c%c",13,10);
			break;
		}
		
		if (Time == -1) return ERR_UNKNOWN;
		SaveVCALDateTime(Buffer, Length, &note->Entries[Time].Date, "DTSTART");

		if (EndTime != -1) {
			SaveVCALDateTime(Buffer, Length, &note->Entries[EndTime].Date, "DTEND");
		}

		if (Alarm != -1) {
			SaveVCALDateTime(Buffer, Length, &note->Entries[Alarm].Date, "AALARM");
		}
	
		SaveVCALText(Buffer, Length, note->Entries[Text].Text, "SUMMARY");

		if (Location != -1) {
			SaveVCALText(Buffer, Length, note->Entries[Location].Text, "LOCATION");
		}
	}
	
	*Length+=sprintf(Buffer+(*Length), "END:VEVENT%c%c",13,10);
	if (header) *Length+=sprintf(Buffer+(*Length), "END:VCALENDAR%c%c",13,10);

	return ERR_NONE;
}

void GSM_ToDoFindDefaultTextTimeAlarmCompleted(GSM_ToDoEntry *entry, int *Text, int *Alarm, int *Completed, int *EndTime, int *Phone)
{
	int i;

	*Text		= -1;
	*EndTime	= -1;
	*Alarm		= -1;
	*Completed	= -1;
	*Phone		= -1;
	for (i = 0; i < entry->EntriesNum; i++) {
		switch (entry->Entries[i].EntryType) {
		case TODO_END_DATETIME :
			if (*EndTime == -1) *EndTime = i;
			break;
		case TODO_ALARM_DATETIME :
		case TODO_SILENT_ALARM_DATETIME:
			if (*Alarm == -1) *Alarm = i;
			break;
		case TODO_TEXT:
			if (*Text == -1) *Text = i;
			break;
		case TODO_COMPLETED:
			if (*Completed == -1) *Completed = i;
			break;
		case TODO_PHONE:
			if (*Phone == -1) *Phone = i;
			break;
		default:
			break;
		}
	}
}

GSM_Error GSM_EncodeVTODO(char *Buffer, int *Length, GSM_ToDoEntry *note, bool header, GSM_VToDoVersion Version)
{
 	int Text, Alarm, Completed, EndTime, Phone;

	GSM_ToDoFindDefaultTextTimeAlarmCompleted(note, &Text, &Alarm, &Completed, &EndTime, &Phone);

	if (header) {
		*Length+=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);
		*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c",13,10);
	}

	*Length+=sprintf(Buffer+(*Length), "BEGIN:VTODO%c%c",13,10);

	if (Version == Nokia_VToDo) {
		if (Text == -1) return ERR_UNKNOWN;
		SaveVCALText(Buffer, Length, note->Entries[Text].Text, "SUMMARY");

		if (Completed == -1) {
			*Length+=sprintf(Buffer+(*Length), "STATUS:NEEDS ACTION%c%c",13,10);
		} else {
			*Length+=sprintf(Buffer+(*Length), "STATUS:COMPLETED%c%c",13,10);
		}
	
		switch (note->Priority) {
			case GSM_Priority_Low	: *Length+=sprintf(Buffer+(*Length), "PRIORITY:1%c%c",13,10); break;
			case GSM_Priority_Medium: *Length+=sprintf(Buffer+(*Length), "PRIORITY:2%c%c",13,10); break;
			case GSM_Priority_High	: *Length+=sprintf(Buffer+(*Length), "PRIORITY:3%c%c",13,10); break;
		}

		if (EndTime != -1) {
			SaveVCALDateTime(Buffer, Length, &note->Entries[EndTime].Date, "DUE");
		}

		if (Alarm != -1) {
			if (note->Entries[Alarm].EntryType == CAL_SILENT_ALARM_DATETIME) {
				SaveVCALDateTime(Buffer, Length, &note->Entries[Alarm].Date, "DALARM");
			} else {
				SaveVCALDateTime(Buffer, Length, &note->Entries[Alarm].Date, "AALARM");
			}
		}
	} else if (Version == SonyEricsson_VToDo) {
		if (Text == -1) return ERR_UNKNOWN;
		SaveVCALText(Buffer, Length, note->Entries[Text].Text, "SUMMARY");

		if (Completed == -1) {
			*Length+=sprintf(Buffer+(*Length), "STATUS:NEEDS ACTION%c%c",13,10);
		} else {
			*Length+=sprintf(Buffer+(*Length), "STATUS:COMPLETED%c%c",13,10);
		}
	
		switch (note->Priority) {
			case GSM_Priority_Low	: *Length+=sprintf(Buffer+(*Length), "PRIORITY:3%c%c",13,10); break;
			case GSM_Priority_Medium: *Length+=sprintf(Buffer+(*Length), "PRIORITY:2%c%c",13,10); break;
			case GSM_Priority_High	: *Length+=sprintf(Buffer+(*Length), "PRIORITY:1%c%c",13,10); break;
		}

		if (Alarm != -1) {
			SaveVCALDateTime(Buffer, Length, &note->Entries[Alarm].Date, "AALARM");
		}		
	}

	*Length+=sprintf(Buffer+(*Length), "END:VTODO%c%c",13,10);

	if (header) {
		*Length+=sprintf(Buffer+(*Length), "END:VCALENDAR%c%c",13,10);
	}
	return ERR_NONE;
}

GSM_Error GSM_DecodeVCALENDAR_VTODO(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, GSM_VCalendarVersion CalVer, GSM_VToDoVersion ToDoVer)
{
	unsigned char 	Line[2000],Buff[2000];
	int		Level = 0;

	Calendar->EntriesNum 	= 0;
	ToDo->EntriesNum 	= 0;

	while (1) {
		MyGetLine(Buffer, Pos, Line, strlen(Buffer));
		if (strlen(Line) == 0) break;
		switch (Level) {
		case 0:
			if (strstr(Line,"BEGIN:VEVENT")) {
				Calendar->Type 	= GSM_CAL_MEMO;
				Level 		= 1;
			}
			if (strstr(Line,"BEGIN:VTODO")) {
				ToDo->Priority 	= GSM_Priority_Low;
				Level 		= 2;
			}
			break;
		case 1: /* Calendar note */
			if (strstr(Line,"END:VEVENT")) {
				if (Calendar->EntriesNum == 0) return ERR_EMPTY;
				return ERR_NONE;
			}
			if (strstr(Line,"CATEGORIES:REMINDER")) 	Calendar->Type = GSM_CAL_REMINDER;
			if (strstr(Line,"CATEGORIES:DATE"))	 	Calendar->Type = GSM_CAL_REMINDER;//SE
			if (strstr(Line,"CATEGORIES:TRAVEL"))	 	Calendar->Type = GSM_CAL_TRAVEL;  //SE
			if (strstr(Line,"CATEGORIES:VACATION"))	 	Calendar->Type = GSM_CAL_VACATION;//SE
			if (strstr(Line,"CATEGORIES:MISCELLANEOUS")) 	Calendar->Type = GSM_CAL_MEMO;
			if (strstr(Line,"CATEGORIES:PHONE CALL")) 	Calendar->Type = GSM_CAL_CALL;
			if (strstr(Line,"CATEGORIES:SPECIAL OCCASION")) Calendar->Type = GSM_CAL_BIRTHDAY;
			if (strstr(Line,"CATEGORIES:ANNIVERSARY")) 	Calendar->Type = GSM_CAL_BIRTHDAY;
			if (strstr(Line,"CATEGORIES:MEETING")) 		Calendar->Type = GSM_CAL_MEETING;
			if (strstr(Line,"CATEGORIES:APPOINTMENT")) 	Calendar->Type = GSM_CAL_MEETING;
			if (strstr(Line,"RRULE:D1")) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1*24;
				Calendar->EntriesNum++;
			}
			if ((strstr(Line,"RRULE:W1")) || (strstr(Line,"RRULE:D7"))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = 7*24;
				Calendar->EntriesNum++;
			}
			if (strstr(Line,"RRULE:W2")) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = 14*24;
				Calendar->EntriesNum++;
			}
			if (strstr(Line,"RRULE:MD1")) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = 30*24;
				Calendar->EntriesNum++;
			}
			if (strstr(Line,"RRULE:YD1")) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = 365*24;
				Calendar->EntriesNum++;
			}
			if ((ReadVCALText(Line, "SUMMARY", Buff)) || (ReadVCALText(Line, "DESCRIPTION", Buff))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "LOCATION", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LOCATION;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DTSTART", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_START_DATETIME;
				ReadVCALDateTime(DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date);
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DTEND", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_END_DATETIME;
				ReadVCALDateTime(DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date);
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DALARM", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
				ReadVCALDateTime(DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date);
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "AALARM", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_ALARM_DATETIME;
				ReadVCALDateTime(DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date);
				Calendar->EntriesNum++;
			}
			break;
		case 2: /* ToDo note */
			if (strstr(Line,"END:VTODO")) {
				if (ToDo->EntriesNum == 0) return ERR_EMPTY;
				return ERR_NONE;
			}
			if (ReadVCALText(Line, "DUE", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
				ReadVCALDateTime(DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "DALARM", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_SILENT_ALARM_DATETIME;
				ReadVCALDateTime(DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "AALARM", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_ALARM_DATETIME;
				ReadVCALDateTime(DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "SUMMARY", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_TEXT;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "PRIORITY", Buff)) {
				if (ToDoVer == SonyEricsson_VToDo) {
					ToDo->Priority = GSM_Priority_Low;
					if (atoi(DecodeUnicodeString(Buff))==2) ToDo->Priority = GSM_Priority_Medium;
					if (atoi(DecodeUnicodeString(Buff))==1) ToDo->Priority = GSM_Priority_High;
					dbgprintf("atoi is %i %s\n",atoi(DecodeUnicodeString(Buff)),DecodeUnicodeString(Buff));
				} else if (ToDoVer == Nokia_VToDo) {
					ToDo->Priority = GSM_Priority_Low;
					if (atoi(DecodeUnicodeString(Buff))==2) ToDo->Priority = GSM_Priority_Medium;
					if (atoi(DecodeUnicodeString(Buff))==3) ToDo->Priority = GSM_Priority_High;
				}
			}
			if (strstr(Line,"STATUS:COMPLETED")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 1;
				ToDo->EntriesNum++;
			}
			break;
		}
	}

	if (Calendar->EntriesNum == 0 && ToDo->EntriesNum == 0) return ERR_EMPTY;
	return ERR_NONE;
}

GSM_Error GSM_EncodeVNTFile(unsigned char *Buffer, int *Length, GSM_NoteEntry *Note)
{
	*Length+=sprintf(Buffer+(*Length), "BEGIN:VNOTE%c%c",13,10);
	*Length+=sprintf(Buffer+(*Length), "VERSION:1.1%c%c",13,10);
	SaveVCALText(Buffer, Length, Note->Text, "BODY");
	*Length+=sprintf(Buffer+(*Length), "END:VNOTE%c%c",13,10);

	return ERR_NONE;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
