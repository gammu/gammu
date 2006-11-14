/* (c) 2002-2004 by Marcin Wiacek, 2005 by Michal Cihar */

#include <string.h>
#include <time.h>

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
			    note->Entries[i].Date.Day >= DT.Day) Past = false;
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
			    note->Entries[End].Date.Day >= DT.Day) Past = false;
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
		case CAL_TONE_ALARM_DATETIME :
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


/*  Function to compute time difference between alarm and event time */
GSM_DateTime	VCALTimeDiff ( GSM_DateTime *Alarm,  GSM_DateTime *Time)
{
	int dt;
	struct tm talarm, ttime;
	GSM_DateTime delta;

	talarm.tm_mday = Alarm->Day;
	talarm.tm_mon  = Alarm->Month-1;
	talarm.tm_year = Alarm->Year -1900;
	talarm.tm_hour = Alarm->Hour;
	talarm.tm_min  = Alarm->Minute;
	talarm.tm_sec  = Alarm->Second;
	talarm.tm_isdst = 0;

	ttime.tm_mday = Time->Day;
	ttime.tm_mon  = Time->Month-1;
	ttime.tm_year = Time->Year -1900;
	ttime.tm_hour = Time->Hour;
	ttime.tm_min  = Time->Minute;
	ttime.tm_sec  = Time->Second;
	ttime.tm_isdst = 0;

	dt = mktime(&ttime) - mktime(&talarm);

	if (dt <= 0) dt = 0;

	/* Mozilla Calendar only accepts relative times for alarm.
	   Maximum representation of time differences is in days.*/
	delta.Year	= 0;
	delta.Month  	= 0;
	delta.Day	= dt / 86400	  ; dt = dt - delta.Day * 86400;
	delta.Hour	= dt / 3600	  ; dt = dt - delta.Hour * 3600;
	delta.Minute 	= dt / 60	  ; dt = dt - delta.Minute * 60;
	delta.Second 	= dt;

	/* Use only one representation. If delta has minutes convert all to minutes etc.*/
	if (delta.Minute !=0) {
		delta.Minute = delta.Day * 24*60 + delta.Hour * 60 + delta.Minute;
		delta.Day=0; delta.Hour=0;
	} else if (delta.Hour !=0) {
		delta.Hour = delta.Day * 24 + delta.Hour;
		delta.Day=0;
	}

	delta.Timezone = 0;

	return delta;
}


GSM_Error GSM_Translate_Category (GSM_CatTranslation direction, unsigned char *string, GSM_CalendarNoteType *Type)
{
	/* Mozilla has user defined categories. These must be converted to GSM_CAL_xxx types.
	   TODO: For now we use hardcoded conversions. Should be user configurable. */

	switch (direction) {
	case TRANSL_TO_GSM:
		if (strstr(string,"MEETING")) 			*Type = GSM_CAL_MEETING;
		else if (strstr(string,"REMINDER")) 		*Type = GSM_CAL_REMINDER;
		else if (strstr(string,"DATE"))	 		*Type = GSM_CAL_REMINDER; //SE
		else if (strstr(string,"TRAVEL"))	 	*Type = GSM_CAL_TRAVEL;   //SE
		else if (strstr(string,"VACATION"))	 	*Type = GSM_CAL_VACATION; //SE
		else if (strstr(string,"MISCELLANEOUS"))	*Type = GSM_CAL_MEMO;
		else if (strstr(string,"PHONE CALL")) 		*Type = GSM_CAL_CALL;
		else if (strstr(string,"SPECIAL OCCASION")) 	*Type = GSM_CAL_BIRTHDAY;
		else if (strstr(string,"ANNIVERSARY")) 		*Type = GSM_CAL_BIRTHDAY;
		else if (strstr(string,"APPOINTMENT")) 		*Type = GSM_CAL_MEETING;
		/* These are the Nokia 6230i categories in german. */
		else if (strstr(string,"Erinnerung"))	 	*Type = GSM_CAL_REMINDER;
		else if (strstr(string,"Besprechung"))	 	*Type = GSM_CAL_MEETING;
		else if (strstr(string,"Anrufen"))	 	*Type = GSM_CAL_CALL;
		else if (strstr(string,"Geburtstag"))	 	*Type = GSM_CAL_BIRTHDAY;
		else if (strstr(string,"Notiz"))	 	*Type = GSM_CAL_MEMO;
		/* default */
		else *Type = GSM_CAL_MEETING;
		break;

	case TRANSL_TO_VCAL:
		switch (*Type) {
			case GSM_CAL_REMINDER:	{strcpy(string, "Erinnerung") ;		break; }
			case GSM_CAL_MEETING:	{strcpy(string, "Besprechung") ;	break; }
			case GSM_CAL_CALL:	{strcpy(string, "Anrufen") ;		break; }
			case GSM_CAL_BIRTHDAY:	{strcpy(string, "Geburtstag") ;		break; }
			case GSM_CAL_MEMO:	{strcpy(string, "Notiz") ;		break; }
			default: break;
		}
		break;
	}
	return 0;
}

GSM_Error GSM_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version)
{
 	int 		Text, Time, Alarm, Phone, EndTime, Location;
	char 		buffer[2000],rec[20],endday[20];
	GSM_DateTime 	deltatime;
	char 		dtstr[20];

	GSM_CalendarFindDefaultTextTimeAlarmPhone(note, &Text, &Time, &Alarm, &Phone, &EndTime, &Location);

	if (header) {
		*Length+=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);
		*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c",13,10);
	}
	*Length+=sprintf(Buffer+(*Length), "BEGIN:VEVENT%c%c",13,10);

	/* Mozilla Calendar needs UIDs. http://www.innerjoin.org/iCalendar/events-and-uids.html */
	if (Version == Mozilla_VCalendar) {
		*Length+=sprintf(Buffer+(*Length), "UID:calendar-%i%c%c",note->Location,13,10);
	}

	if (Version == Nokia_VCalendar || Version == Mozilla_VCalendar) {
		unsigned char buf[50];

		GSM_Translate_Category (TRANSL_TO_VCAL, buf, &note->Type);
		*Length+=sprintf(Buffer+(*Length), "CATEGORIES:%s%c%c",buf,13,10);

		if (Text != -1)  {
			CopyUnicodeString(note->Entries[Text].Text,EncodeUnicodeSpecialChars(note->Entries[Text].Text));
		}

		if (note->Type == GSM_CAL_CALL) {
			buffer[0] = 0; buffer[1] = 0;
		 	if (Phone != -1) CopyUnicodeString(buffer,note->Entries[Phone].Text);
			if (Text != -1)  {
				if (Phone != -1) EncodeUnicode(buffer+UnicodeLength(buffer)*2," ",1);
				CopyUnicodeString(buffer+UnicodeLength(buffer)*2,note->Entries[Text].Text);
			}
			SaveVCALTextUTF8(Buffer, Length, buffer, "SUMMARY");

		} else {
			if (Text == -1) return ERR_UNKNOWN;
			/* Split text field into summary and description */
			{
				char delim[4] = {0x00,'/',0x00,0x00};
				char null[2] = {0x00,0x00};
				char *loc;
				int ofs=0,	len;

				loc=mywstrstr(note->Entries[Text].Text, delim);
				if (loc == NULL)
					len=2*UnicodeLength(note->Entries[Text].Text);
				else
					len=(unsigned int) loc- (unsigned int) note->Entries[Text].Text;
				memcpy(buffer, note->Entries[Text].Text, len);
				memcpy(buffer+len, null, sizeof(null));
				SaveVCALTextUTF8(Buffer, Length, buffer, "SUMMARY");

				if (loc != NULL) {
					ofs=len+sizeof(delim)-2;
					loc=mywstrstr(note->Entries[Text].Text+ofs, delim);
					if (loc == NULL)
						len=2*UnicodeLength(note->Entries[Text].Text+ofs);
					else
						len=(unsigned int) loc- ((unsigned int) note->Entries[Text].Text+ofs);
					memcpy(buffer, note->Entries[Text].Text+ofs, len);
					memcpy(buffer+len, null, sizeof(null));
					SaveVCALTextUTF8(Buffer, Length, buffer, "DESCRIPTION");
		}
		}
		}

		if (note->Type == GSM_CAL_MEETING && Location != -1) {
			SaveVCALTextUTF8(Buffer, Length, EncodeUnicodeSpecialChars(note->Entries[Location].Text), "LOCATION");
		}

		if (Version == Mozilla_VCalendar) {
			*Length+=sprintf(Buffer+(*Length), "STATUS:CONFIRMED%c%c",13,10);
		}

		/* Start Time */
		if (Time == -1) return ERR_UNKNOWN;
		switch (Version) {
		case Nokia_VCalendar:
			SaveVCALDateTime(Buffer, Length, &note->Entries[Time].Date, "DTSTART");
			break;
		case Mozilla_VCalendar:
			switch (note->Type) {
			case GSM_CAL_MEMO:
			case GSM_CAL_BIRTHDAY:
				SaveVCALDate(Buffer, Length, &note->Entries[Time].Date, "DTSTART;VALUE=DATE");
				/* whole-day entry: set start time to midnight so difference to alarm time will be correct*/
				note->Entries[Time].Date.Hour=0 ; note->Entries[Time].Date.Minute=0 ; note->Entries[Time].Date.Second=0;
				break;
			default:
				SaveVCALDateTime(Buffer, Length, &note->Entries[Time].Date, "DTSTART");
				break;
			}
			break;
		default:
			break;
		}

		/* End time */
		switch (Version) {
		case Nokia_VCalendar:
			if (EndTime != -1) SaveVCALDateTime(Buffer, Length, &note->Entries[EndTime].Date, "DTEND");
			break;
		case Mozilla_VCalendar:
			switch (note->Type) {
			case GSM_CAL_MEMO:
				SaveVCALDate(Buffer, Length, &note->Entries[EndTime].Date, "DTEND;VALUE=DATE");
				break;
			case GSM_CAL_BIRTHDAY: {
				SaveVCALDate(Buffer, Length, &note->Entries[Time].Date, "DTEND;VALUE=DATE");
				break; }
			default:
				if (EndTime != -1) SaveVCALDateTime(Buffer, Length, &note->Entries[EndTime].Date, "DTEND");
				break;
			}
		default:
			break;
		}

		/* Disable alarm for birthday entries. Mozilla would generate an alarm before birth! */
		if (Version == Mozilla_VCalendar && note->Type == GSM_CAL_BIRTHDAY)	Alarm=-1;
		if (Alarm != -1) {
			switch (Version) {
			case Nokia_VCalendar:
				if (note->Entries[Alarm].EntryType == CAL_SILENT_ALARM_DATETIME) {
					SaveVCALDateTime(Buffer, Length, &note->Entries[Alarm].Date, "DALARM");
				} else {
					SaveVCALDateTime(Buffer, Length, &note->Entries[Alarm].Date, "AALARM");
				}
				break;
			case Mozilla_VCalendar:
				deltatime = VCALTimeDiff(&note->Entries[Alarm].Date, &note->Entries[Time].Date);

				dtstr[0]='\0';
				if (deltatime.Minute !=0) {
					*Length+=sprintf(Buffer+(*Length), "X-MOZILLA-ALARM-DEFAULT-UNITS:minutes%c%c",13,10);
					*Length+=sprintf(Buffer+(*Length), "X-MOZILLA-ALARM-DEFAULT-LENGTH:%i%c%c",
						deltatime.Minute,13,10);
					sprintf(dtstr,"-PT%iM",deltatime.Minute);
				} else if (deltatime.Hour !=0) {
					*Length+=sprintf(Buffer+(*Length), "X-MOZILLA-ALARM-DEFAULT-UNITS:hours%c%c",13,10);
					*Length+=sprintf(Buffer+(*Length), "X-MOZILLA-ALARM-DEFAULT-LENGTH:%i%c%c",
						deltatime.Hour,13,10);
					sprintf(dtstr,"-PT%iH",deltatime.Hour);
				} else if (deltatime.Day !=0) {
					*Length+=sprintf(Buffer+(*Length), "X-MOZILLA-ALARM-DEFAULT-UNITS:days%c%c",13,10);
					*Length+=sprintf(Buffer+(*Length), "X-MOZILLA-ALARM-DEFAULT-LENGTH:%i%c%c",
						deltatime.Day,13,10);
					sprintf(dtstr,"-P%iD",deltatime.Day);
				}
				if (dtstr[0] != '\0') {
					*Length+=sprintf(Buffer+(*Length), "BEGIN:VALARM%c%c",13,10);
					*Length+=sprintf(Buffer+(*Length), "TRIGGER;VALUE=DURATION%c%c",13,10);
					*Length+=sprintf(Buffer+(*Length), " :%s%c%c",dtstr,13,10);
					*Length+=sprintf(Buffer+(*Length), "END:VALARM%c%c",13,10);
				}
				break;
			default:
				break;
			}
		}

		/* Birthday is known to be recurranced */
		if (note->Type == GSM_CAL_BIRTHDAY) {
			if (Version == Mozilla_VCalendar) {
				*Length+=sprintf(Buffer+(*Length), "X-MOZILLA-RECUR-DEFAULT-UNITS:years%c%c",13,10);
				*Length+=sprintf(Buffer+(*Length), "RRULE:FREQ=YEARLY;INTERVAL=1;BYMONTH=6%c%c",13,10);
			}
		} else {
			GSM_SetCalendarRecurranceRepeat(rec, endday, note);
			if (endday[0]*256+endday[1] == 0) {
				switch(rec[0]*256+rec[1]) {
					case 1*24	 : *Length+=sprintf(Buffer+(*Length), "RRULE:D1 #0%c%c",13,10);	 break;
					case 7*24	 : *Length+=sprintf(Buffer+(*Length), "RRULE:W1 #0%c%c",13,10);	 break;
					case 14*24	 : *Length+=sprintf(Buffer+(*Length), "RRULE:W2 #0%c%c",13,10);	 break;
					case 0xffff-1    : break;
					case 0xffff 	 : *Length+=sprintf(Buffer+(*Length), "RRULE:YD1 #0%c%c",13,10); break;
				}
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
	char buffer[2000];
	int code;

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
			default: break;
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
	} else if (Version == Mozilla_VToDo) {
		/* Mozilla Calendar needs UIDs. http://www.innerjoin.org/iCalendar/events-and-uids.html */
		*Length+=sprintf(Buffer+(*Length), "UID:todo-%i%c%c",note->Location,13,10);

		if (Text == -1) return ERR_UNKNOWN;

		/* Split text field into summary, description, location */
		{
			char delim[4] = {0x00,'/',0x00,0x00};
			char null[2] = {0x00,0x00};
			char *loc;
			int ofs=0,	len;

			CopyUnicodeString(note->Entries[Text].Text,EncodeUnicodeSpecialChars(note->Entries[Text].Text));

			loc=mywstrstr(note->Entries[Text].Text, delim);
			if (loc == NULL)
				len=2*UnicodeLength(note->Entries[Text].Text);
			else
				len=(unsigned int) loc- (unsigned int) note->Entries[Text].Text;
			memcpy(buffer, note->Entries[Text].Text, len);
			memcpy(buffer+len, null, sizeof(null));
			SaveVCALTextUTF8(Buffer, Length, buffer, "SUMMARY");

			if (loc != NULL) {
				ofs=len+sizeof(delim)-2;
				loc=mywstrstr(note->Entries[Text].Text+ofs, delim);
				if (loc == NULL)
					len=2*UnicodeLength(note->Entries[Text].Text+ofs);
				else
					len=(unsigned int) loc- (unsigned int) (note->Entries[Text].Text+ofs);
				memcpy(buffer, note->Entries[Text].Text+ofs, len);
				memcpy(buffer+len, null, sizeof(null));
				SaveVCALTextUTF8(Buffer, Length, buffer, "DESCRIPTION");

				if (loc != NULL) {
					ofs=ofs+len+sizeof(delim)-2;
					loc=mywstrstr(note->Entries[Text].Text+ofs, delim);
					if (loc == NULL)
						len=2*UnicodeLength(note->Entries[Text].Text+ofs);
					else
						len=(unsigned int) loc- ((unsigned int) note->Entries[Text].Text+ofs);
					memcpy(buffer, note->Entries[Text].Text+ofs, len);
					memcpy(buffer+len, null, sizeof(null));
					SaveVCALTextUTF8(Buffer, Length, buffer, "LOCATION");
				}
			}
		}


		if (Completed == -1) {
			*Length+=sprintf(Buffer+(*Length), "STATUS:NEEDS ACTION%c%c",13,10);
		} else {
			GSM_DateTime	DT;
			*Length+=sprintf(Buffer+(*Length), "STATUS:COMPLETED%c%c",13,10);
			GSM_GetCurrentDateTime (&DT);
			SaveVCALDateTime(Buffer, Length, &DT, "COMPLETED");
			*Length+=sprintf(Buffer+(*Length), "PERCENT-COMPLETE:100%c%c",13,10);
		}

		code = 0;
		switch (note->Priority) {
			case GSM_Priority_None	: code = 0 ; break;
			case GSM_Priority_Low	: code = 9 ; break;
			case GSM_Priority_Medium: code = 5 ; break;
			case GSM_Priority_High	: code = 1 ; break;
		}
		if (code !=0) *Length+=sprintf(Buffer+(*Length), "PRIORITY:%i%c%c",code,13,10);

		if (EndTime != -1) {
        	/* 20371231T2359xx is a flag for 'date not set' */
        	if (note->Entries[EndTime].Date.Year   != 2037	&&
	            note->Entries[EndTime].Date.Month  != 12	&&
	            note->Entries[EndTime].Date.Day    != 31	&&
	            note->Entries[EndTime].Date.Hour   != 23	&&
	            note->Entries[EndTime].Date.Minute != 59 )
			SaveVCALDateTime(Buffer, Length, &note->Entries[EndTime].Date, "DUE");
		}


		if (Alarm != -1) {
			GSM_DateTime 	deltatime;
			char 		dtstr[20];

			deltatime = VCALTimeDiff(&note->Entries[Alarm].Date, &note->Entries[EndTime].Date);
			dtstr[0]='\0';
			if (deltatime.Minute !=0) {
				*Length+=sprintf(Buffer+(*Length), "X-MOZILLA-ALARM-DEFAULT-UNITS:minutes%c%c",13,10);
				*Length+=sprintf(Buffer+(*Length), "X-MOZILLA-ALARM-DEFAULT-LENGTH:%i%c%c",
					deltatime.Minute,13,10);
				sprintf(dtstr,"-PT%iM",deltatime.Minute);
			} else if (deltatime.Hour !=0) {
				*Length+=sprintf(Buffer+(*Length), "X-MOZILLA-ALARM-DEFAULT-UNITS:hours%c%c",13,10);
				*Length+=sprintf(Buffer+(*Length), "X-MOZILLA-ALARM-DEFAULT-LENGTH:%i%c%c",
					deltatime.Hour,13,10);
				sprintf(dtstr,"-PT%iH",deltatime.Hour);
			} else if (deltatime.Day !=0) {
				*Length+=sprintf(Buffer+(*Length), "X-MOZILLA-ALARM-DEFAULT-UNITS:days%c%c",13,10);
				*Length+=sprintf(Buffer+(*Length), "X-MOZILLA-ALARM-DEFAULT-LENGTH:%i%c%c",
					deltatime.Day,13,10);
				sprintf(dtstr,"-P%iD",deltatime.Day);
			}
			if (dtstr[0] != '\0') {
				*Length+=sprintf(Buffer+(*Length), "BEGIN:VALARM%c%c",13,10);
				*Length+=sprintf(Buffer+(*Length), "TRIGGER;VALUE=DURATION;RELATED=END%c%c",13,10);
				*Length+=sprintf(Buffer+(*Length), " :%s%c%c",dtstr,13,10);
				*Length+=sprintf(Buffer+(*Length), "END:VALARM%c%c",13,10);
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
			default: break;
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

GSM_TimeUnit ReadVCALTimeUnits (unsigned char *Buffer)
{
	if (mystrcasestr(Buffer,"days"))	return GSM_TimeUnit_Days;
	if (mystrcasestr(Buffer,"hours"))	return GSM_TimeUnit_Hours;
	if (mystrcasestr(Buffer,"minutes")) return GSM_TimeUnit_Minutes;
	if (mystrcasestr(Buffer,"seconds")) return GSM_TimeUnit_Seconds;
	return GSM_TimeUnit_Unknown;
}

GSM_DeltaTime ReadVCALTriggerTime (unsigned char *Buffer)
{
	GSM_DeltaTime 	dt;
	int 		sign = 1;
	int 		pos = 0;
	int 		val;
	char 		unit;

	dt.Timezone = 0;
	dt.Year = 0 ; dt.Day = 0; dt.Month = 0; dt.Hour = 0; dt.Minute = 0; dt.Second = 0;

	if (Buffer[pos] == '+') {
		sign = 1; pos++;
	} else if (Buffer[pos] == '-') {
		sign = -1; pos++;
	}
	if (Buffer[pos] == 'P') pos++;
	if (Buffer[pos] == 'T') pos++;

	if ( !sscanf(Buffer+pos,"%i%c",&val,&unit) ) return dt;

	switch (unit) {
		case 'D': dt.Day    = sign * val ; break;
		case 'H': dt.Hour   = sign * val ; break;
		case 'M': dt.Minute = sign * val ; break;
		case 'S': dt.Second = sign * val ; break;
	}

	return dt;
}

/* Prepare input buffer (notably line continuations) */
int GSM_Make_VCAL_Lines (unsigned char *Buffer, int *lBuffer)
{
	int src=0;
	int dst=0;

	for (src=0; src <= *lBuffer; src++) {
		if (Buffer[src] == '\r') src++;
		if (Buffer[src] == '\n') {
			if (Buffer[src+1] == ' ' && Buffer[src+2] == ':' ) src = src + 2;
			if (Buffer[src+1] == ' ' && Buffer[src+2] == ';' ) src = src + 2;
		}
		if (dst > src) return ERR_UNKNOWN;
		Buffer[dst] = Buffer[src];
		dst++;
	}
	*lBuffer = dst-1;
	return ERR_NONE;
}

GSM_Error GSM_DecodeVCALENDAR_VTODO(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar,
					GSM_ToDoEntry *ToDo, GSM_VCalendarVersion CalVer, GSM_VToDoVersion ToDoVer)
{
	unsigned char 	Line[2000],Buff[2000],bu[20];
	int		Level = 0;
	GSM_DateTime	Date;
	GSM_TimeUnit	unit = GSM_TimeUnit_Unknown;
	GSM_DeltaTime	trigger;
	int		deltatime = 0;
	bool		is_date_only;
	bool		date_only = false;
	int		lBuffer;
 	int 		Text=-1, Time=-1, Alarm=-1, EndTime=-1, Location=-1;

	if (!Buffer) return ERR_EMPTY;

	Calendar->EntriesNum 	= 0;
	ToDo->EntriesNum 	= 0;
	lBuffer = strlen(Buffer);
	trigger.Timezone = -999;

	if (CalVer == Mozilla_VCalendar && *Pos ==0) {
		int error;
		error = GSM_Make_VCAL_Lines (Buffer, &lBuffer);
		if (error != ERR_NONE) return error;
	}

	while (1) {
		MyGetLine(Buffer, Pos, Line, lBuffer, true);
		if (strlen(Line) == 0) break;

		switch (Level) {
		case 0:
			if (strstr(Line,"BEGIN:VEVENT")) {
				Calendar->Type = -1;
				date_only = true;
				Text=-1; Time=-1; Alarm=-1; EndTime=-1; Location=-1;
				Level 		= 1;
			}
			if (strstr(Line,"BEGIN:VTODO")) {
				ToDo->Priority 	= GSM_Priority_None;
				Text=-1; Time=-1; Alarm=-1; EndTime=-1; Location=-1;
				Level 		= 2;
			}
			break;
		case 1: /* Calendar note */
			if (strstr(Line,"END:VEVENT")) {
				if (Time == -1) return ERR_UNKNOWN;
				if (Calendar->EntriesNum == 0) return ERR_EMPTY;

				if (trigger.Timezone != -999) {
					Alarm = Calendar->EntriesNum;
					Calendar->Entries[Alarm].Date = GSM_AddTime (Calendar->Entries[Time].Date, trigger);
					Calendar->Entries[Alarm].EntryType = CAL_TONE_ALARM_DATETIME;
					Calendar->EntriesNum++;
				}

				/* If event type is undefined choose approbiate type. Memos carry dates only, no times.
				   Use Meetings for events with full date+time settings. */
				if (Calendar->Type == -1) {
					if (date_only)
						Calendar->Type = GSM_CAL_MEMO;
					else
						Calendar->Type = GSM_CAL_MEETING;
				}

				return ERR_NONE;
			}

			/* Read Mozilla calendar entries. Some of them will not be used here. Notably alarm time
			   can defined in several ways. We will use the trigger value only since this is the value
			   Mozilla calendar uses when importing ics-files. */
			if (strstr(Line,"UID:")) {
				ReadVCALText(Line, "UID", Buff);  // Any use for UIDs?
				break;
			}
			if (strstr(Line,"X-MOZILLA-ALARM-DEFAULT-UNITS:")) {
				if (ReadVCALText(Line, "X-MOZILLA-ALARM-DEFAULT-UNITS", Buff)) {
					unit = ReadVCALTimeUnits(DecodeUnicodeString(Buff));
					break;
				}
			}
			if (strstr(Line,"X-MOZILLA-ALARM-DEFAULT-LENGTH:")) {
				if (ReadVCALInt(Line, "X-MOZILLA-ALARM-DEFAULT-LENGTH", &deltatime)) {
					break;
				}
			}

			if (strstr(Line,"BEGIN:VALARM")) {
				MyGetLine(Buffer, Pos, Line, lBuffer, true);
				if (strlen(Line) == 0) break;
				if (ReadVCALText(Line, "TRIGGER;VALUE=DURATION", Buff)) {
					trigger = ReadVCALTriggerTime(DecodeUnicodeString(Buff));
					break;
				}
			}

			/* Event type. Must be set correctly to let phone calendar work as expected. For example
			   without GSM_CAL_MEETING the time part of an event date/time will be dropped. */
			if (strstr(Line,"CATEGORIES:")) {
				GSM_Translate_Category(TRANSL_TO_GSM, Line+11, &Calendar->Type);
			}

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

			switch (CalVer) {
			case Mozilla_VCalendar:
				/* Calendar-entries in phones only have a text and a location field. So we concatenate summary
				   and description to one text field with '/' as delimiter.
				   Note: Mozilla calendar writes UTF8 but does not denote it */
				if ((ReadVCALTextUTF8(Line, "SUMMARY", Buff))) {
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
					CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,
						DecodeUnicodeSpecialChars(Buff));
					Text = Calendar->EntriesNum;
					Calendar->EntriesNum++;
				}
				/* Append description to text field. */
				if ((ReadVCALTextUTF8(Line, "DESCRIPTION", Buff))) {
					CopyUnicodeString(Buff,DecodeUnicodeSpecialChars(Buff));
					if (Text == -1) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
					Text = Calendar->EntriesNum;
					Calendar->EntriesNum++;
					} else {
						char del[10];
						size_t len= UnicodeLength(Calendar->Entries[Text].Text);
						DecodeUTF8(del,"/",1);
						CopyUnicodeString(Calendar->Entries[Text].Text+len*2,del);
						CopyUnicodeString(Calendar->Entries[Text].Text+len*2+2,Buff);
					}
				}
				if (ReadVCALTextUTF8(Line, "LOCATION", Buff)) {
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LOCATION;
					CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,
						DecodeUnicodeSpecialChars(Buff));
					Location = Calendar->EntriesNum;
					Calendar->EntriesNum++;
				}
				break;

			default:
				if ((ReadVCALText(Line, "SUMMARY", Buff))) {
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
					CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,
						DecodeUnicodeSpecialChars(Buff));
					Text = Calendar->EntriesNum;
					Calendar->EntriesNum++;
				}
				if ((ReadVCALText(Line, "DESCRIPTION", Buff))) {
					CopyUnicodeString(Buff,DecodeUnicodeSpecialChars(Buff));
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_DESCRIPTION;
					CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,
						DecodeUnicodeSpecialChars(Buff));
					Calendar->EntriesNum++;
				}
				if (ReadVCALText(Line, "LOCATION", Buff)) {
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LOCATION;
					CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,
						DecodeUnicodeSpecialChars(Buff));
					Location = Calendar->EntriesNum;
					Calendar->EntriesNum++;
				}
			}

			if ((ReadVCALText(Line, "X-IRMC-LUID", Buff))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LUID;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,
					DecodeUnicodeSpecialChars(Buff));
				Calendar->EntriesNum++;
			}
			if ((ReadVCALText(Line, "CLASS", Buff))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_PRIVATE;
				if (mywstrncasecmp(Buff, "\0P\0U\0B\0L\0I\0C\0", 0)) {
					Calendar->Entries[Calendar->EntriesNum].Number = 0;
				} else {
					Calendar->Entries[Calendar->EntriesNum].Number = 1;
				}
				Calendar->EntriesNum++;
			}
			if (ReadVCALDate(Line, "DTSTART", &Date, &is_date_only)) {
				Calendar->Entries[Calendar->EntriesNum].Date = Date;
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_START_DATETIME;
				Time = Calendar->EntriesNum;
				Calendar->EntriesNum++;
				if (!is_date_only) date_only = false;
			}
			if (ReadVCALDate(Line, "DTEND", &Date, &is_date_only)) {
				Calendar->Entries[Calendar->EntriesNum].Date = Date;
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_END_DATETIME;
				EndTime = Calendar->EntriesNum;
				Calendar->EntriesNum++;
				if (!is_date_only) date_only = false;
			}
			if (ReadVCALDate(Line, "DALARM", &Date, &is_date_only)) {
				Calendar->Entries[Calendar->EntriesNum].Date = Date;
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
				Alarm = Calendar->EntriesNum;
				Calendar->EntriesNum++;
			}
			if (ReadVCALDate(Line, "AALARM", &Date, &is_date_only)) {
				Calendar->Entries[Calendar->EntriesNum].Date = Date;
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TONE_ALARM_DATETIME;
				Alarm = Calendar->EntriesNum;
				Calendar->EntriesNum++;
			}
			break;

		case 2: /* ToDo note */
			if (strstr(Line,"END:VTODO")) {
				if (ToDo->EntriesNum == 0) return ERR_EMPTY;
                /* Substitute very late date if Mozilla hasn't one */
                if (EndTime == -1) {
					memset (&Date, 0, sizeof(GSM_DateTime));
					Date.Year = 2037 ; Date.Month = 12 ; Date.Day = 31 ;
					Date.Hour = 23 ; Date.Minute = 59 ; Date.Second = 59;
					ToDo->Entries[ToDo->EntriesNum].Date = Date;
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
					ToDo->EntriesNum++;
                }
				return ERR_NONE;
			}

			if (strstr(Line,"UID:")) {
				ReadVCALText(Line, "UID", Buff);  // Any use for UIDs?
				break;
			}
			if (strstr(Line,"X-MOZILLA-ALARM-DEFAULT-UNITS:")) {
				if (ReadVCALText(Line, "X-MOZILLA-ALARM-DEFAULT-UNITS", Buff)) {
					unit = ReadVCALTimeUnits(DecodeUnicodeString(Buff));
					break;
				}
			}
			if (strstr(Line,"X-MOZILLA-ALARM-DEFAULT-LENGTH:")) {
				if (ReadVCALInt(Line, "X-MOZILLA-ALARM-DEFAULT-LENGTH", &deltatime)) {
					break;
				}
			}

			if (ReadVCALDate(Line, "DUE", &Date, &is_date_only)) {
				ToDo->Entries[ToDo->EntriesNum].Date = Date;
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
				EndTime = Calendar->EntriesNum;
				ToDo->EntriesNum++;
			}
			if (ReadVCALDate(Line, "DALARM", &Date, &is_date_only)) {
				ToDo->Entries[ToDo->EntriesNum].Date = Date;
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_SILENT_ALARM_DATETIME;
				Alarm = Calendar->EntriesNum;
				ToDo->EntriesNum++;
			}
			if (ReadVCALDate(Line, "AALARM", &Date, &is_date_only)) {
				ToDo->Entries[ToDo->EntriesNum].Date = Date;
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_ALARM_DATETIME;
				Alarm = Calendar->EntriesNum;
				ToDo->EntriesNum++;
			}

			switch (ToDoVer) {
			case Mozilla_VToDo:
				/* ToDo-Notes in phones only have a single text field. So we concatenate summary,
				   description and location to one text field with '/' as delimiter.
				   Note: Mozilla calendar writes UTF8 but does not denote it */
				if ((ReadVCALTextUTF8(Line, "SUMMARY", Buff))) {
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_TEXT;
					CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,
						DecodeUnicodeSpecialChars(Buff));
					Text = ToDo->EntriesNum;
					ToDo->EntriesNum++;
				}
				/* Append description to text field. */
				if ((ReadVCALTextUTF8(Line, "DESCRIPTION", Buff))) {
					CopyUnicodeString(Buff,DecodeUnicodeSpecialChars(Buff));
					if (Text == -1) {
						ToDo->Entries[ToDo->EntriesNum].EntryType = CAL_TEXT;
					CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
					Text = ToDo->EntriesNum;
					ToDo->EntriesNum++;
					} else {
						char del[10];
						size_t len= UnicodeLength(ToDo->Entries[Text].Text);
						DecodeUTF8(del,"/",1);
						CopyUnicodeString(ToDo->Entries[Text].Text+len*2,del);
						CopyUnicodeString(ToDo->Entries[Text].Text+len*2+2,Buff);
				}
				}
				/* Append location to text field. */
				if (ReadVCALTextUTF8(Line, "LOCATION", Buff)) {
					CopyUnicodeString(Buff,DecodeUnicodeSpecialChars(Buff));
					if (Text == -1) {
						ToDo->Entries[ToDo->EntriesNum].EntryType = CAL_TEXT;
						CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
						Text = ToDo->EntriesNum;
						ToDo->EntriesNum++;
			} else {
						char del[10];
						size_t len= UnicodeLength(ToDo->Entries[Text].Text);
						DecodeUTF8(del,"/",1);
						CopyUnicodeString(ToDo->Entries[Text].Text+len*2,del);
						CopyUnicodeString(ToDo->Entries[Text].Text+len*2+2,Buff);
					}
				}
				break;

			default:
				if ((ReadVCALText(Line, "SUMMARY", Buff))) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_TEXT;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				ToDo->EntriesNum++;
			}
				/* Append description to text field. */
				if ((ReadVCALText(Line, "DESCRIPTION", Buff))) {
					CopyUnicodeString(Buff,DecodeUnicodeSpecialChars(Buff));
					if (Text == -1) {
						ToDo->Entries[ToDo->EntriesNum].EntryType = CAL_TEXT;
						CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
						Text = ToDo->EntriesNum;
						ToDo->EntriesNum++;
					} else {
						char del[10];
						size_t len= UnicodeLength(ToDo->Entries[Text].Text);
						DecodeUTF8(del,"/",1);
						CopyUnicodeString(ToDo->Entries[Text].Text+len*2,del);
						CopyUnicodeString(ToDo->Entries[Text].Text+len*2+2,Buff);
					}
				}
				/* Append location to text field. */
				if (ReadVCALText(Line, "LOCATION", Buff)) {
					if (Text == -1) {
						ToDo->Entries[ToDo->EntriesNum].EntryType = CAL_TEXT;
					CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
					Text = ToDo->EntriesNum;
					ToDo->EntriesNum++;
					} else {
						char del[10];
						size_t len= UnicodeLength(ToDo->Entries[Text].Text);
						DecodeUTF8(del,"/",1);
						CopyUnicodeString(ToDo->Entries[Text].Text+len*2,del);
						CopyUnicodeString(ToDo->Entries[Text].Text+len*2+2,Buff);
					}
				}
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
