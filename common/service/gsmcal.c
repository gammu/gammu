/* (c) 2002-2004 by Marcin Wiacek, 2005 by Michal Cihar */

#include <string.h>

#include "gsmcal.h"
#include "gsmmisc.h"
#include "../misc/coding/coding.h"

void GSM_SetCalendarRecurranceRepeat(unsigned char *rec, unsigned char *endday, GSM_CalendarEntry *entry)
{
	int		i,start=-1,frequency=-1,dow=-1,day=-1,month=-1,end=-1,Recurrance = 0, Repeat=0;
	char 		Buf[20];
	GSM_DateTime	DT;
	time_t		t_time1,t_time2;

	rec[0] = 0;
	rec[1] = 0;

	for (i=0;i<entry->EntriesNum;i++) {
		if (entry->Entries[i].EntryType == CAL_START_DATETIME)   start 		= i;
		if (entry->Entries[i].EntryType == CAL_REPEAT_FREQUENCY) frequency 	= i;
		if (entry->Entries[i].EntryType == CAL_REPEAT_DAYOFWEEK) dow 		= i;
		if (entry->Entries[i].EntryType == CAL_REPEAT_DAY)       day 		= i;
		if (entry->Entries[i].EntryType == CAL_REPEAT_MONTH)     month 		= i;
		if (entry->Entries[i].EntryType == CAL_REPEAT_STOPDATE)  end 		= i;
	}
	if (start == -1) return;

	if (frequency != -1 && dow == -1 && day == -1 && month == -1) {
		if (entry->Entries[frequency].Number == 1) {
			//each day
			Recurrance = 24;
		}
	}

	sprintf(Buf,"%s",DayOfWeek(entry->Entries[start].Date.Year,
			entry->Entries[start].Date.Month,
			entry->Entries[start].Date.Day));
	if (!strcmp(Buf,"Mon")) i = 1;
	if (!strcmp(Buf,"Tue")) i = 2;
	if (!strcmp(Buf,"Wed")) i = 3;
	if (!strcmp(Buf,"Thu")) i = 4;
	if (!strcmp(Buf,"Fri")) i = 5;
	if (!strcmp(Buf,"Sat")) i = 6;
	if (!strcmp(Buf,"Sun")) i = 7;

	if (frequency != -1 && dow != -1 && day == -1 && month == -1) {
		if (entry->Entries[frequency].Number == 1 &&
		    entry->Entries[dow].Number == i) {
			//one week
			Recurrance = 24*7;
		}
	}
	if (frequency != -1 && dow != -1 && day == -1 && month == -1) {
		if (entry->Entries[frequency].Number == 2 &&
		    entry->Entries[dow].Number == i) {
			//two weeks
			Recurrance = 24*14;
		}
	}
	if (frequency != -1 && dow == -1 && day != -1 && month == -1) {
		if (entry->Entries[frequency].Number == 1 &&
		    entry->Entries[day].Number == entry->Entries[start].Date.Day) {
			//month
			Recurrance = 0xffff-1;
		}
	}
	if (frequency != -1 && dow == -1 && day != -1 && month != -1) {
		if (entry->Entries[frequency].Number == 1 &&
		    entry->Entries[day].Number == entry->Entries[start].Date.Day &&
		    entry->Entries[month].Number == entry->Entries[start].Date.Month) {
			//year
			Recurrance = 0xffff;
		}
	}

	rec[0] = Recurrance / 256;
	rec[1] = Recurrance % 256;

	if (endday == NULL) return;

	endday[0] = 0;
	endday[1] = 0;

	if (end == -1) return;

	t_time1 = Fill_Time_T(entry->Entries[start].Date);
	t_time2 = Fill_Time_T(entry->Entries[end].Date);
	if (t_time2 - t_time1 <= 0) return;

	switch (Recurrance) {
		case 24:
		case 24*7:
		case 24*14:
			Repeat = (t_time2 - t_time1) / (60*60*Recurrance) + 1;
			break;
		case 0xffff-1:
			memcpy(&DT,&entry->Entries[start].Date,sizeof(GSM_DateTime));
			while (1) {
				if ((DT.Year == entry->Entries[end].Date.Year && DT.Month > entry->Entries[end].Date.Month) ||
				    (DT.Year >  entry->Entries[end].Date.Year)) break;
				if (DT.Month == 12) {
					DT.Month = 1;
					DT.Year++;
				} else {
					DT.Month++;
				}
				Repeat++;
			}
			break;
		case 0xffff:
			Repeat = entry->Entries[end].Date.Year-entry->Entries[start].Date.Year+1;
			break;
	}

	endday[0] = Repeat/256;
	endday[1] = Repeat%256;

	dbgprintf("Repeat number: %i\n",Repeat);
}

void GSM_GetCalendarRecurranceRepeat(unsigned char *rec, unsigned char *endday, GSM_CalendarEntry *entry)
{
	int 	Recurrance,num=-1,i;
	char 	Buf[20];

	Recurrance = rec[0]*256 + rec[1];
	if (Recurrance == 0) return;
	/* dct3 and dct4: 65535 (0xffff) is 1 year */
	if (Recurrance == 0xffff) Recurrance=24*365;
	/* dct3: unavailable, dct4: 65534 (0xffff-1) is 30 days */
	if (Recurrance == 0xffff-1) Recurrance=24*30;
	dbgprintf("Recurrance   : %i hours\n",Recurrance);

	for (i=0;i<entry->EntriesNum;i++) {
		if (entry->Entries[i].EntryType == CAL_START_DATETIME) {
			num = i;
			break;
		}
	}
	if (num == -1) return;
	sprintf(Buf,"%s",DayOfWeek(entry->Entries[num].Date.Year,
			entry->Entries[num].Date.Month,
			entry->Entries[num].Date.Day));

	if (Recurrance == 24    || Recurrance == 24*7 ||
	    Recurrance == 24*30 || Recurrance == 24*365) {
		entry->Entries[entry->EntriesNum].EntryType	= CAL_REPEAT_FREQUENCY;
		entry->Entries[entry->EntriesNum].Number	= 1;
		entry->EntriesNum++;
	}
	if (Recurrance == 24*14) {
		entry->Entries[entry->EntriesNum].EntryType	= CAL_REPEAT_FREQUENCY;
		entry->Entries[entry->EntriesNum].Number	= 2;
		entry->EntriesNum++;
	}
	if (Recurrance == 24*7 || Recurrance == 24*14) {
		entry->Entries[entry->EntriesNum].EntryType	 = CAL_REPEAT_DAYOFWEEK;
		if (!strcmp(Buf,"Mon")) {
			entry->Entries[entry->EntriesNum].Number = 1;
		} else if (!strcmp(Buf,"Tue")) {
			entry->Entries[entry->EntriesNum].Number = 2;
		} else if (!strcmp(Buf,"Wed")) {
			entry->Entries[entry->EntriesNum].Number = 3;
		} else if (!strcmp(Buf,"Thu")) {
			entry->Entries[entry->EntriesNum].Number = 4;
		} else if (!strcmp(Buf,"Fri")) {
			entry->Entries[entry->EntriesNum].Number = 5;
		} else if (!strcmp(Buf,"Sat")) {
			entry->Entries[entry->EntriesNum].Number = 6;
		} else if (!strcmp(Buf,"Sun")) {
			entry->Entries[entry->EntriesNum].Number = 7;
		}
		entry->EntriesNum++;
	}
	if (Recurrance == 24*30) {
		entry->Entries[entry->EntriesNum].EntryType	= CAL_REPEAT_DAY;
		entry->Entries[entry->EntriesNum].Number	= entry->Entries[num].Date.Day;
		entry->EntriesNum++;
	}
	if (Recurrance == 24*365) {
		entry->Entries[entry->EntriesNum].EntryType	= CAL_REPEAT_DAY;
		entry->Entries[entry->EntriesNum].Number	= entry->Entries[num].Date.Day;
		entry->EntriesNum++;
		entry->Entries[entry->EntriesNum].EntryType	= CAL_REPEAT_MONTH;
		entry->Entries[entry->EntriesNum].Number	= entry->Entries[num].Date.Month;
		entry->EntriesNum++;
	}
	if (endday == NULL || endday[0]*256+endday[1] == 0) return;
	dbgprintf("Repeat   : %i times\n",endday[0]*256+endday[1]);
	memcpy(&entry->Entries[entry->EntriesNum].Date,&entry->Entries[num].Date,sizeof(GSM_DateTime));
	entry->Entries[entry->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
	switch (Recurrance) {
		case 24:
		case 24*7:
		case 24*14:
			GetTimeDifference(60*60*Recurrance*(endday[0]*256+endday[1]-1), &entry->Entries[entry->EntriesNum].Date, true, 1);
			entry->EntriesNum++;
			break;
		case 24*30:
			for (i=0;i<endday[0]*256+endday[1]-1;i++) {
				if (entry->Entries[entry->EntriesNum].Date.Month == 12) {
					entry->Entries[entry->EntriesNum].Date.Month = 1;
					entry->Entries[entry->EntriesNum].Date.Year++;
				} else {
					entry->Entries[entry->EntriesNum].Date.Month++;
				}
			}
			entry->EntriesNum++;
			break;
		case 24*365:
			entry->Entries[entry->EntriesNum].Date.Year += (endday[0]*256+endday[1] - 1);
			entry->EntriesNum++;
			break;
	}
	dbgprintf("End Repeat Time: %04i-%02i-%02i %02i:%02i:%02i\n",
		entry->Entries[entry->EntriesNum-1].Date.Year,
		entry->Entries[entry->EntriesNum-1].Date.Month,
		entry->Entries[entry->EntriesNum-1].Date.Day,
		entry->Entries[entry->EntriesNum-1].Date.Hour,
		entry->Entries[entry->EntriesNum-1].Date.Minute,
		entry->Entries[entry->EntriesNum-1].Date.Second);
}

bool IsCalendarNoteFromThePast(GSM_CalendarEntry *note)
{
	bool 		Past = true;
	int		i,End=-1;
	GSM_DateTime	DT;
	char		rec[20],endday[20];

	GSM_GetCurrentDateTime (&DT);
	for (i = 0; i < note->EntriesNum; i++) {
		switch (note->Entries[i].EntryType) {
		case CAL_START_DATETIME :
			if (note->Entries[i].Date.Year > DT.Year) Past = false;
			if (note->Entries[i].Date.Year == DT.Year &&
			    note->Entries[i].Date.Month > DT.Month) Past = false;
			if (note->Entries[i].Date.Year == DT.Year &&
			    note->Entries[i].Date.Month == DT.Month &&
			    note->Entries[i].Date.Day > DT.Day) Past = false;
			break;
		case CAL_REPEAT_STOPDATE:
			if (End == -1) End = i;
		default:
			break;
		}
		if (!Past) break;
	}
	if (note->Type == GSM_CAL_BIRTHDAY) Past = false;
	GSM_SetCalendarRecurranceRepeat(rec, endday, note);
	if (rec[0] != 0 || rec[1] != 0) {
		if (End == -1) {
			Past = false;
		} else {
			if (note->Entries[End].Date.Year > DT.Year) Past = false;
			if (note->Entries[End].Date.Year == DT.Year &&
			    note->Entries[End].Date.Month > DT.Month) Past = false;
			if (note->Entries[End].Date.Year == DT.Year &&
			    note->Entries[End].Date.Month == DT.Month &&
			    note->Entries[End].Date.Day > DT.Day) Past = false;
		}
	}
	return Past;
}

void GSM_CalendarFindDefaultTextTimeAlarmPhone(GSM_CalendarEntry *entry, int *Text, int *Time, int *Alarm, int *Phone, int *EndTime, int *Location)
{
	int i;

	*Text		= -1;
	*Time		= -1;
	*Alarm		= -1;
	*Phone		= -1;
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
 	int 	Text, Time, Alarm, Phone, EndTime, Location;
	char 	buffer[2000],rec[20],endday[20];

	GSM_CalendarFindDefaultTextTimeAlarmPhone(note, &Text, &Time, &Alarm, &Phone, &EndTime, &Location);

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
		if (note->Type != GSM_CAL_BIRTHDAY) {
			GSM_SetCalendarRecurranceRepeat(rec, endday, note);
			if (endday[0]*256+endday[1] == 0) {
				switch(rec[0]*256+rec[1]) {
					case 1*24	 : *Length+=sprintf(Buffer+(*Length), "RRULE:D1 #0%c%c",13,10);	 break;
					case 7*24	 : *Length+=sprintf(Buffer+(*Length), "RRULE:W1 #0%c%c",13,10);	 break;
					case 14*24	 : *Length+=sprintf(Buffer+(*Length), "RRULE:W2 #0%c%c",13,10);	 break;
					case 0xffff-1    : break;
					case 0xffff 	 : *Length+=sprintf(Buffer+(*Length), "RRULE:YD1 #0%c%c",13,10); break;
				}
			} else {
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

		GSM_SetCalendarRecurranceRepeat(rec, endday, note);
		if (endday[0]*256+endday[1] == 0) {
			switch(rec[0]*256+rec[1]) {
				case 1*24	 : *Length+=sprintf(Buffer+(*Length), "RRULE:D1%c%c",13,10);  break;
				case 7*24	 : *Length+=sprintf(Buffer+(*Length), "RRULE:D7%c%c",13,10);  break;
				case 0xffff-1    : *Length+=sprintf(Buffer+(*Length), "RRULE:MD1%c%c",13,10); break;
				case 0xffff 	 : *Length+=sprintf(Buffer+(*Length), "RRULE:YD1%c%c",13,10); break;
			}
		} else {
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
	unsigned char 	Line[2000],Buff[2000],bu[20];
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
				bu[0] = 0;
				bu[1] = 24;
				GSM_GetCalendarRecurranceRepeat(bu, NULL, Calendar);
			}
			if ((strstr(Line,"RRULE:W1")) || (strstr(Line,"RRULE:D7"))) {
				bu[0] = 0;
				bu[1] = 24*7;
				GSM_GetCalendarRecurranceRepeat(bu, NULL, Calendar);
			}
			if (strstr(Line,"RRULE:W2")) {
				bu[0] = (24*14)/256;
				bu[1] = (24*14)/256;
				GSM_GetCalendarRecurranceRepeat(bu, NULL, Calendar);
			}
			if (strstr(Line,"RRULE:MD1")) {
				bu[0] = (0xffff-1)/256;
				bu[1] = (0xffff-1)/256;
				GSM_GetCalendarRecurranceRepeat(bu, NULL, Calendar);
			}
			if (strstr(Line,"RRULE:YD1")) {
				bu[0] = (0xffff)/256;
				bu[1] = (0xffff)/256;
				GSM_GetCalendarRecurranceRepeat(bu, NULL, Calendar);
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
			if (ReadVCALText(Line, "DTSTART", Buff) && ReadVCALDateTime(DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_START_DATETIME;
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DTEND", Buff) && ReadVCALDateTime(DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_END_DATETIME;
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DALARM", Buff) && ReadVCALDateTime(DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "AALARM", Buff) && ReadVCALDateTime(DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_ALARM_DATETIME;
				Calendar->EntriesNum++;
			}
			break;
		case 2: /* ToDo note */
			if (strstr(Line,"END:VTODO")) {
				if (ToDo->EntriesNum == 0) return ERR_EMPTY;
				return ERR_NONE;
			}
			if (ReadVCALText(Line, "DUE", Buff) && ReadVCALDateTime(DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "DALARM", Buff) && ReadVCALDateTime(DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_SILENT_ALARM_DATETIME;
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "AALARM", Buff) && ReadVCALDateTime(DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_ALARM_DATETIME;
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
