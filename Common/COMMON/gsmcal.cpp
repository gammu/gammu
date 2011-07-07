#include "stdafx.h"

#include <string.h>
#include <afxtempl.h>

#include "gsmcal.h"
#include "gsmpbk.h"
#include "coding.h"
#include "commfun.h"

extern void SF_SaveVCARDText(char *Buffer, int *Length, unsigned char *Text, char *Start,char *beforetext);

void GetwhichWeekDay(GSM_DateTime gsmtime, UINT &whichWeek, UINT &whichDay)
{
	whichWeek = whichDay = 0;
	if(gsmtime.Year == 0 || gsmtime.Month <1 ||  gsmtime.Month > 12 ||
		gsmtime.Day <1 ||  gsmtime.Day > 31 )
		return;
	COleDateTimeSpan dts;
	dts.SetDateTimeSpan(7,0,0,0);

	COleDateTime oleDateTime(gsmtime.Year,gsmtime.Month,gsmtime.Day,gsmtime.Hour,gsmtime.Minute,gsmtime.Second);

	whichDay = oleDateTime.GetDayOfWeek();

	COleDateTime dtTemp = oleDateTime;
	COleDateTime dtFirstDay;
	dtFirstDay.SetDate(oleDateTime.GetYear(),oleDateTime.GetMonth(),1);


	while(dtTemp.GetMonth() == oleDateTime.GetMonth())
	{
		dtTemp = dtTemp-dts;
		whichWeek++;
	}
	whichDay--;

}

time_t Fill_Time_T(GSM_DateTime DT, int TZ)
{
	struct tm 	tm_starttime;
	unsigned char 	buffer[30];

	dbgprintf("  StartTime  : %02i-%02i-%04i %02i:%02i:%02i\n",
		DT.Day,DT.Month,DT.Year,DT.Hour,DT.Minute,DT.Second);

	if (TZ != 0) {
#if defined(WIN32) || defined(__SVR4)
	    sprintf((char*)buffer,"TZ=PST+%i",TZ);
	    putenv((char*)buffer);
#else
	    sprintf((char*)buffer,"PST+%i",TZ);
	    setenv("TZ",buffer,1);
#endif
	}
	tzset();

	memset(&tm_starttime, 0, sizeof(tm_starttime));
	tm_starttime.tm_year 	= DT.Year - 1900;
	tm_starttime.tm_mon  	= DT.Month - 1;
	tm_starttime.tm_mday 	= DT.Day;
	tm_starttime.tm_hour 	= DT.Hour;
	tm_starttime.tm_min  	= DT.Minute;
	tm_starttime.tm_sec  	= DT.Second;
	tm_starttime.tm_isdst	= 0;
	
	return mktime(&tm_starttime);
}
void GetTimeDifference(unsigned long diff, GSM_DateTime *DT, bool Plus, int multi)
{
	time_t t_time;

	t_time = Fill_Time_T(*DT,8);

	if (Plus) {
		t_time 		+= diff*multi;
	} else {
		t_time 		-= diff*multi;
	}

	Fill_GSM_DateTime(DT, t_time);
	DT->Year = DT->Year + 1900;
	dbgprintf("  EndTime    : %02i-%02i-%04i %02i:%02i:%02i\n",
		DT->Day,DT->Month,DT->Year,DT->Hour,DT->Minute,DT->Second);
}
void SaveVCALDateTime(char *Buffer, int *Length, GSM_DateTime *Date, char *Start)
{
	if (Start != NULL) {
		*Length+=sprintf(Buffer+(*Length), "%s:",Start);
	}
	*Length+=sprintf(Buffer+(*Length), "%04d%02d%02dT%02d%02d%02dZ%c%c",
			Date->Year, Date->Month, Date->Day,
			Date->Hour, Date->Minute, Date->Second,13,10);
}

// 9@9u
void SF_SaveVCALDateTime(char *Buffer, int *Length, GSM_DateTime *Date, char *Start)
{
	if (Start != NULL) {
		*Length+=sprintf(Buffer+(*Length), "%s:",Start);
	}
	*Length+=sprintf(Buffer+(*Length), "%04d%02d%02dT%02d%02d%02dZ%c%c%c%c",
			Date->Year, Date->Month, Date->Day,
			Date->Hour, Date->Minute, Date->Second,0x2C,0x2C,0x2C,0x2C);
}


void SaveVCALDateTimeWithoutTimeZone(char *Buffer, int *Length, GSM_DateTime *Date, char *Start)
{
	if (Start != NULL) {
		*Length+=sprintf(Buffer+(*Length), "%s:",Start);
	}
	*Length+=sprintf(Buffer+(*Length), "%04d%02d%02dT%02d%02d%02d%c%c",
			Date->Year, Date->Month, Date->Day,
			Date->Hour, Date->Minute, Date->Second,13,10);
}
void ReadVCALDateTimeWithDayLight(char *Buffer, GSM_DateTime *dt,COleDateTime dtDaylightStart,COleDateTime dtDaylightEnd,int nTimeZone,int nDaylightZone)
{
	char year[5]="", month[3]="", day[3]="", hour[3]="", minute[3]="", second[3]="";

	memset(dt,0,sizeof(dt));

	strncpy(year, 	Buffer, 	4);
	strncpy(month, 	Buffer+4, 	2);
	strncpy(day, 	Buffer+6, 	2);
	strncpy(hour, 	Buffer+9,	2);
	strncpy(minute, Buffer+11,	2);
	strncpy(second, Buffer+13,	2);

	/* FIXME: Should check ranges... */
	dt->Year	= atoi(year);
	dt->Month	= atoi(month);
	dt->Day		= atoi(day);
	dt->Hour	= atoi(hour);
	dt->Minute	= atoi(minute);
	dt->Second	= atoi(second);
	
	COleDateTime dtTime;
	dtTime.SetDateTime(dt->Year,dt->Month,dt->Day,dt->Hour,dt->Minute,dt->Second);
	int nZone = nTimeZone;
	if(nDaylightZone !=0)
	{
		if(dtDaylightStart.m_dt<=dtTime.m_dt && dtTime.m_dt<=dtDaylightEnd.m_dt)
			nZone=nDaylightZone;
	}

	/* FIXME */
	if(nZone!=0  && strlen(Buffer)>=16 && (Buffer[15] =='Z' ||Buffer[15] =='z'))
	{
		if(nZone>0)
			GetTimeDifference(nZone*15, dt, true, 60);
		else 
			GetTimeDifference(-nZone*15, dt, false, 60);

	}

	dt->Timezone	= 0;
}

void ReadVCALDateTime(char *Buffer, GSM_DateTime *dt,int nTimeZone)
{
	char year[5]="", month[3]="", day[3]="", hour[3]="", minute[3]="", second[3]="";

	memset(dt,0,sizeof(dt));

	strncpy(year, 	Buffer, 	4);
	strncpy(month, 	Buffer+4, 	2);
	strncpy(day, 	Buffer+6, 	2);
	strncpy(hour, 	Buffer+9,	2);
	strncpy(minute, Buffer+11,	2);
	strncpy(second, Buffer+13,	2);



	/* FIXME: Should check ranges... */
	dt->Year	= atoi(year);
	dt->Month	= atoi(month);
	dt->Day		= atoi(day);
	dt->Hour	= atoi(hour);
	dt->Minute	= atoi(minute);
	dt->Second	= atoi(second);
	/* FIXME */
	if(nTimeZone!=0  && strlen(Buffer)>=16 && (Buffer[15] =='Z' ||Buffer[15] =='z'))
	{
		if(nTimeZone>0)
			GetTimeDifference(nTimeZone*15, dt, true, 60);
		else 
			GetTimeDifference(-nTimeZone*15, dt, false, 60);

	}

	dt->Timezone	= 0;
}

void WINAPI ReadVCALDateTime_GMT(char *Buffer, GSM_DateTime *dt)
{
	char year[5]="", month[3]="", day[3]="", hour[3]="", minute[3]="", second[3]="", gmt_h[2]="", gmt_m[2]="";
	int nGmt;

	memset(dt,0,sizeof(dt));

	strncpy(year, 	Buffer, 	4);
	strncpy(month, 	Buffer+4, 	2);
	strncpy(day, 	Buffer+6, 	2);
	strncpy(hour, 	Buffer+9,	2);
	strncpy(minute, Buffer+11,	2);
	strncpy(second, Buffer+13,	2);

	if(*(Buffer+15)=='+')	nGmt=1;
	else					nGmt=-1;

	strncpy(gmt_h,  Buffer+16,	2);
	strncpy(gmt_m,  Buffer+18,	2);

	/* FIXME: Should check ranges... */
	dt->Year	= atoi(year);
	dt->Month	= atoi(month);
	dt->Day		= atoi(day);
	dt->Hour	= atoi(hour);
	dt->Minute	= atoi(minute);
	dt->Second	= atoi(second);
	/* FIXME */
	dt->Timezone	= atoi(gmt_h)* nGmt;
}

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
void GSM_CalendarFindEntryIndex(GSM_CalendarEntry *entry, int *Text, int *Time, int *Alarm, int *Phone, int *Recurrance, int *RecurranceFreq, int *EndTime, int *Location ,int *RepeatEndDate,int *Description,int *DayofweekMask)
{
	int i;

	*Text		= -1;
	*Time		= -1;
	*Alarm		= -1;
	*Phone		= -1;
	*Recurrance	= -1;
	*EndTime	= -1;
	*Location	= -1;
	*RecurranceFreq	= -1;
	*RepeatEndDate = -1;
	*Description = *DayofweekMask = -1;
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
		case CAL_RECURRANCEFREQUENCY:
			if (*RecurranceFreq == -1) *RecurranceFreq = i;
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
		case CAL_REPEAT_STOPDATE:
			if (*RepeatEndDate == -1) *RepeatEndDate = i;
			break;
		case CAL_DESCRIPTION:
			if (*Description == -1) *Description = i;
			break;
		case CAL_REPEAT_DAYOFWEEK:
			if (*DayofweekMask == -1) *DayofweekMask = i;
			break;
		default:
			break;
		}
	}
}

// 9@9u
void SF_CalendarFindEntryIndex(GSM_CalendarEntry *entry, int *Text, int *Time, int *Alarm, int *Phone, int *Recurrance, int *RecurranceFreq, int *EndTime, int *Location ,int *RepeatEndDate,int *Description,int *DayofweekMask, int *Priority)
{
	int i;

	*Text		= -1;
	*Time		= -1;
	*Alarm		= -1;
	*Phone		= -1;
	*Recurrance	= -1;
	*EndTime	= -1;
	*Location	= -1;
	*RecurranceFreq	= -1;
	*RepeatEndDate = -1;
	*Description = *DayofweekMask = -1;
	*Priority = -1;

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
		case CAL_RECURRANCEFREQUENCY:
			if (*RecurranceFreq == -1) *RecurranceFreq = i;
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
		case CAL_REPEAT_STOPDATE:
			if (*RepeatEndDate == -1) *RepeatEndDate = i;
			break;
		case CAL_DESCRIPTION:
			if (*Description == -1) *Description = i;
			break;
		case CAL_REPEAT_DAYOFWEEK:
			if (*DayofweekMask == -1) *DayofweekMask = i;
			break;
		case CAL_PRIVATE:
			if (*Priority == -1) *Priority = i;
			break;
		default:
			break;
		}
	}
}




void GSM_CalendarFindDefaultTextTimeAlarmPhoneRecurrance(GSM_CalendarEntry *entry, int *Text, int *Time, int *Alarm, int *Phone, int *Recurrance, int *RecurranceFreq, int *EndTime, int *Location)
{
	int i;

	*Text		= -1;
	*Time		= -1;
	*Alarm		= -1;
	*Phone		= -1;
	*Recurrance	= -1;
	*EndTime	= -1;
	*Location	= -1;
	*RecurranceFreq	= -1;
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
		case CAL_RECURRANCEFREQUENCY:
			if (*RecurranceFreq == -1) *RecurranceFreq = i;
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
/*
GSM_Error GSM_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version)
{
 	int 	Text, Time, Alarm, Phone, Recurrance, RecurranceFreq, EndTime, Location;
	char 	buffer[2000];

	GSM_CalendarFindDefaultTextTimeAlarmPhoneRecurrance(note, &Text, &Time, &Alarm, &Phone, &Recurrance,&RecurranceFreq, &EndTime, &Location);

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
		 	if (Phone != -1) CopyUnicodeString((unsigned char *)buffer,note->Entries[Phone].Text);
			if (Text != -1)  {
				if (Phone != -1) EncodeUnicode((unsigned char *)buffer+UnicodeLength((unsigned char *)buffer)*2,(unsigned char *)" ",1);
				CopyUnicodeString((unsigned char *)buffer+UnicodeLength((unsigned char *)buffer)*2,note->Entries[Text].Text);
			}
			SaveVCALText(Buffer, Length, (unsigned char *)buffer, "SUMMARY");
		} else {
			SaveVCALText(Buffer, Length,note->Entries[Text].Text, "SUMMARY");
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
/*		if (Recurrance != -1 && note->Type != GSM_CAL_BIRTHDAY) {
			switch(note->Entries[Recurrance].Number) {
				case CAL_REPEAT_DAILY	 : 
					if(RecurranceFreq!= -1)
						*Length+=sprintf(Buffer+(*Length), "RRULE:D%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
					else
						*Length+=sprintf(Buffer+(*Length), "RRULE:D1 #0%c%c",13,10);
					break;
				case CAL_REPEAT_WEEKLY	 : 
					if(RecurranceFreq!= -1)
						*Length+=sprintf(Buffer+(*Length), "RRULE:W%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
					else
						*Length+=sprintf(Buffer+(*Length), "RRULE:W1 #0%c%c",13,10);
					break;
				case CAL_REPEAT_MONTHLY	 : 
					if(RecurranceFreq!= -1)
						*Length+=sprintf(Buffer+(*Length), "RRULE:MD%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
					else
						*Length+=sprintf(Buffer+(*Length), "RRULE:MD1 #0%c%c",13,10);	
					break;
				case CAL_REPEAT_YEARLY : 
					if(RecurranceFreq!= -1)
						*Length+=sprintf(Buffer+(*Length), "RRULE:YD%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
					else
						*Length+=sprintf(Buffer+(*Length), "RRULE:YD1 #0%c%c",13,10);
					break;
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
			switch(note->Entries[Recurrance].Number) {
				case CAL_REPEAT_DAILY	 : 
					if(RecurranceFreq!= -1)
						*Length+=sprintf(Buffer+(*Length), "RRULE:D%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
					else
						*Length+=sprintf(Buffer+(*Length), "RRULE:D1 #0%c%c",13,10);
					break;
				case CAL_REPEAT_WEEKLY	 : 
					if(RecurranceFreq!= -1)
						*Length+=sprintf(Buffer+(*Length), "RRULE:W%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
					else
						*Length+=sprintf(Buffer+(*Length), "RRULE:W1 #0%c%c",13,10);
					break;
				case CAL_REPEAT_MONTHLY	 : 
					if(RecurranceFreq!= -1)
						*Length+=sprintf(Buffer+(*Length), "RRULE:MD%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
					else
						*Length+=sprintf(Buffer+(*Length), "RRULE:MD1 #0%c%c",13,10);	
					break;
				case CAL_REPEAT_YEARLY : 
					if(RecurranceFreq!= -1)
						*Length+=sprintf(Buffer+(*Length), "RRULE:YD%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
					else
						*Length+=sprintf(Buffer+(*Length), "RRULE:YD1 #0%c%c",13,10);
					break;
			}
		}
	
		if (note->Type == GSM_CAL_CALL) {
			buffer[0] = 0;
			buffer[1] = 0;
		 	if (Phone != -1) CopyUnicodeString((unsigned char *)buffer,note->Entries[Phone].Text);
			if (Text != -1)  {
				if (Phone != -1) EncodeUnicode((unsigned char *)buffer+UnicodeLength((unsigned char *)buffer)*2,(unsigned char *)" ",1);
				CopyUnicodeString((unsigned char *)buffer+UnicodeLength((unsigned char *)buffer)*2,note->Entries[Text].Text);
			}
			SaveVCALText(Buffer, Length, (unsigned char *)buffer, "DESCRIPTION");
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
	
		SaveVCALText(Buffer, Length,note->Entries[Text].Text, "SUMMARY");

		if (Location != -1) {
			SaveVCALText(Buffer, Length,note->Entries[Location].Text, "LOCATION");
		}
	}
	
	*Length+=sprintf(Buffer+(*Length), "END:VEVENT%c%c",13,10);
	if (header) *Length+=sprintf(Buffer+(*Length), "END:VCALENDAR%c%c",13,10);

	return ERR_NONE;
}
*/
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
void GSM_ToDoFindFindEntryIndex(GSM_ToDoEntry *entry, int *Text, int *Alarm, int *Completed, int *EndTime, int *Phone
								,int *Recurrance,int *RecurranceFreq, int *RepeatEndDate,int *Description,int *DayofweekMask,int *CompletedDate)
{
	int i;

	*Text		= -1;
	*EndTime	= -1;
	*Alarm		= -1;
	*Completed	= -1;
	*Phone		= -1;
	*Recurrance = *RecurranceFreq = *RepeatEndDate =*Description =*DayofweekMask = -1;
	*CompletedDate = -1;
	for (i = 0; i < entry->EntriesNum; i++) {
		switch (entry->Entries[i].EntryType) {
		case TODO_END_DATETIME :
			if (*EndTime == -1) *EndTime = i;
			break;
		case TODO_COMPLETEDDATE :
			if (*CompletedDate == -1) *CompletedDate = i;
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
		case TODO_DESCRIPTION:
			if (*Description == -1) *Description = i;
			break;
		case TODO_RECURRANCE:
			if (*Recurrance == -1) *Recurrance = i;
			break;
		case TODO_RECURRANCEFREQUENCY:
			if (*RecurranceFreq == -1) *RecurranceFreq = i;
			break;
		case TODO_REPEAT_STOPDATE:
			if (*RepeatEndDate == -1) *RepeatEndDate = i;
			break;
		case TODO_REPEAT_DAYOFWEEK:
			if (*DayofweekMask == -1) *DayofweekMask = i;
			break;
		default:
			break;
		}
	}
}

void GSM_ToDoFindFindEntryIndexEx(GSM_ToDoEntry *entry, int *Text, int *Alarm, int *Completed, int *StartTime, int *EndTime, int *Phone
								,int *Recurrance,int *RecurranceFreq, int *RepeatEndDate,int *Description,int *DayofweekMask,int *CompletedDate)
{
	int i;
	
	*StartTime	= -1;
	*Text		= -1;
	*EndTime	= -1;
	*Alarm		= -1;
	*Completed	= -1;
	*Phone		= -1;
	*Recurrance = *RecurranceFreq = *RepeatEndDate =*Description =*DayofweekMask = -1;
	*CompletedDate = -1;
	for (i = 0; i < entry->EntriesNum; i++) {
		switch (entry->Entries[i].EntryType) {
		case TODO_START_DATETIME:
			if (*StartTime == -1) *StartTime = i;
			break;
		case TODO_END_DATETIME :
			if (*EndTime == -1) *EndTime = i;
			break;
		case TODO_COMPLETEDDATE :
			if (*CompletedDate == -1) *CompletedDate = i;
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
		case TODO_DESCRIPTION:
			if (*Description == -1) *Description = i;
			break;
		case TODO_RECURRANCE:
			if (*Recurrance == -1) *Recurrance = i;
			break;
		case TODO_RECURRANCEFREQUENCY:
			if (*RecurranceFreq == -1) *RecurranceFreq = i;
			break;
		case TODO_REPEAT_STOPDATE:
			if (*RepeatEndDate == -1) *RepeatEndDate = i;
			break;
		case TODO_REPEAT_DAYOFWEEK:
			if (*DayofweekMask == -1) *DayofweekMask = i;
			break;
		default:
			break;
		}
	}
}

GSM_Error GSM_EncodeVTODO(char *Buffer, int *Length, GSM_ToDoEntry *note, bool header, GSM_VToDoVersion Version,int Timezone)
{
 	int Text, Alarm, Completed, StartTime, EndTime, Phone;
	int Recurrance,RecurranceFreq ,RepeatEndDate,Description,DayofweekMask,CompletedDate;

	if (Version == Samsung_VToDo)
		GSM_ToDoFindFindEntryIndexEx(note, &Text, &Alarm, &Completed,&StartTime, &EndTime, &Phone,&Recurrance,&RecurranceFreq ,&RepeatEndDate,&Description,&DayofweekMask,&CompletedDate);
	else
		GSM_ToDoFindFindEntryIndex(note, &Text, &Alarm, &Completed, &EndTime, &Phone,&Recurrance,&RecurranceFreq ,&RepeatEndDate,&Description,&DayofweekMask,&CompletedDate);

	if (header) {
		*Length+=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);
		*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c",13,10);
	}

	*Length+=sprintf(Buffer+(*Length), "BEGIN:VTODO%c%c",13,10);

	if (Text == -1) return ERR_UNKNOWN;
	unsigned char utemp[(MAX_TODO_TEXT_LENGTH + 1)*2];
	CopyUnicodeString(utemp,note->Entries[Text].Text);
//	UnicodeReplace(utemp,";","\\;");

	if(Version == SonyEricsson_VToDo) //Encode with utf7
		SESaveVCALText(Buffer, Length, utemp, "SUMMARY",":");
	else
	{
		if (Version == Samsung_VToDo)
			UnicodeReplace(utemp,";","\\;");
		SESaveVCARDText(Buffer, Length, utemp, "SUMMARY",":");
	}
	if (Version == Samsung_VToDo)	//Summary = no use, Decription = title for samsung	//bobby 10.5.05
	{
		if(StartTime != -1)
		{
			GSM_DateTime gsmDateTime;
			memcpy(&gsmDateTime,&note->Entries[StartTime].Date,sizeof(GSM_DateTime));

			if(Timezone>0)
				GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
			else
				GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);
			if(Timezone == 0)
				SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &gsmDateTime, "DTSTART");
			else
				SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DTSTART");
		}
		else
		{	
			GSM_DateTime gsmDateTime;
			memcpy(&gsmDateTime,&note->Entries[EndTime].Date,sizeof(GSM_DateTime));
			if(Timezone>0)
				GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
			else
				GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);
			if(Timezone == 0)
				SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &gsmDateTime, "DTSTART");
			else
				SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DTSTART");
		}
	}

	if (EndTime != -1) 
	{
		if(Version == SonyEricsson_VToDo)
		{
			note->Entries[CompletedDate].Date.Hour = 23;
			note->Entries[CompletedDate].Date.Minute = 59;
			SaveVCALDateTime(Buffer, Length, &note->Entries[EndTime].Date, "DUE");
		}
		else
		{
			GSM_DateTime gsmDateTime;
			memcpy(&gsmDateTime,&note->Entries[EndTime].Date,sizeof(GSM_DateTime));

			if(Timezone>0)
				GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
			else
				GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);

			if(Timezone == 0)
				SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &gsmDateTime, "DUE");
			else
				SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DUE");
		}
	}
	if (CompletedDate != -1) 
	{
		GSM_DateTime gsmDateTime;
		memcpy(&gsmDateTime,&note->Entries[CompletedDate].Date,sizeof(GSM_DateTime));
		if(Timezone>0)
			GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
		else
			GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);
		if(Timezone == 0)
			SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &gsmDateTime, "COMPLETED");
		else
			SaveVCALDateTime(Buffer, Length, &gsmDateTime, "COMPLETED");
	}

	if(Version == SonyEricsson_VToDo)
	{
		switch (note->Type) {
		case GSM_CAL_MEETING:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "MISCELLANEOUS%c%c",13,10);
			break;
		case GSM_CAL_CALL:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "PHONECALL%c%c",13,10);
			break;
		case GSM_CAL_REMINDER:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "MESSAGE%c%c",13,10);
			break;
		case GSM_CAL_MEMO:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "TASK%c%c",13,10);
			break;
		case GSM_CAL_TRAVEL:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "PHONE CALL%c%c",13,10);
		//		*Length+=sprintf(Buffer+(*Length), "APPOINTMENT%c%c",13,10);
		break;
		}

	}
	else if(Version == Sagem_VToDo)
	{
		switch (note->Type) {
		case GSM_CAL_MEETING:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "UNKNOWN%c%c",13,10);
			break;
		case GSM_CAL_CALL:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "SHOPPING LIST%c%c",13,10);
			break;
		case GSM_CAL_REMINDER:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "BUSINESS%c%c",13,10);
			break;
		case GSM_CAL_MEMO:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "PERSONAL%c%c",13,10);
			break;
		case GSM_CAL_TRAVEL:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "PHONE CALL%c%c",13,10);
			break;
		case GSM_CAL_BUSINESS:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "GIFT%c%c",13,10);
			break;
		case GSM_CAL_VACATION:
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			*Length+=sprintf(Buffer+(*Length), "IDEA%c%c",13,10);
			break;
		}
	}
	*Length+=sprintf(Buffer+(*Length), "PRIORITY:%d%c%c",note->Priority,13,10);

	bool bCompleted = false;
	if (Completed != -1 ) 
		if(note->Entries[Completed].Number) bCompleted = true;

	if (bCompleted == false ) 
		*Length+=sprintf(Buffer+(*Length), "STATUS:NEEDS ACTION%c%c",13,10);
	else 
		*Length+=sprintf(Buffer+(*Length), "STATUS:COMPLETED%c%c",13,10);


	
	if (Alarm != -1) 
	{
		GSM_DateTime gsmDateTime;
		memcpy(&gsmDateTime,&note->Entries[Alarm].Date,sizeof(GSM_DateTime));
		if(Timezone>0)
			GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
		else
			GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);

		if(Timezone == 0)
			SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &gsmDateTime, "AALARM");
		else
			SaveVCALDateTime(Buffer, Length, &gsmDateTime, "AALARM");
	}


	if (Description != -1) 
	{
		CopyUnicodeString(utemp,note->Entries[Description].Text);
//		UnicodeReplace(utemp,";","\\;");

		if(Version == SonyEricsson_VToDo) //Encode with utf7
			SESaveVCALText(Buffer, Length, utemp, "DESCRIPTION",":");
		else
		{
			if (Version == Samsung_VToDo)
				UnicodeReplace(utemp,";","\\;");
			SESaveVCARDText(Buffer, Length, utemp, "DESCRIPTION",":");
		}
	}

	if (Recurrance != -1) 
	{
		int nFreq = 1;
		if(RecurranceFreq!= -1) nFreq = note->Entries[RecurranceFreq].Number;
		switch(note->Entries[Recurrance].Number)
		{
			case CAL_REPEAT_DAILY	 : 
				*Length+=sprintf(Buffer+(*Length), "RRULE:D%d #0",nFreq);
				break;
			case CAL_REPEAT_WEEKLY	 : 
				if(DayofweekMask == -1)
					*Length+=sprintf(Buffer+(*Length), "RRULE:W%d #0",nFreq);
				else 
				{
					char szDay[MAX_PATH];
					int nMask = note->Entries[DayofweekMask].Number;
					szDay[0] = '\0';
					if(nMask & 1) strcat(szDay ,"SU ");
					if(nMask & 2) strcat(szDay ,"MO ");
					if(nMask & 4) strcat(szDay ,"TU ");
					if(nMask & 8) strcat(szDay ,"WE ");
					if(nMask & 16) strcat(szDay ,"TH ");
					if(nMask & 32) strcat(szDay ,"FR ");
					if(nMask & 64) strcat(szDay ,"SA ");
					*Length+=sprintf(Buffer+(*Length), "RRULE:W%d %s#0",nFreq,szDay);

				}
				break;
			case CAL_REPEAT_MONTHLY	 : 
				*Length+=sprintf(Buffer+(*Length), "RRULE:MD%d #0",nFreq);
				break;
			case CAL_REPEAT_MONTH_WEEKLY	 : 
				{
					UINT whichWeek,  whichDay;
					GetwhichWeekDay(note->Entries[EndTime].Date,whichWeek,whichDay);
					char szday[MAX_PATH];
					switch(whichDay)
					{
					case 0:
						wsprintf(szday,"SU");
						break;
					case 1:
						wsprintf(szday,"MO");
						break;
					case 2:
						wsprintf(szday,"TU");
						break;
					case 3:
						wsprintf(szday,"WE");
						break;
					case 4:
						wsprintf(szday,"TH");
						break;
					case 5:
						wsprintf(szday,"FR");
						break;
					case 6:
						wsprintf(szday,"SA");
						break;
					}

					if(whichWeek == 5) 
						*Length+=sprintf(Buffer+(*Length), "RRULE:MP%d %d- %s #0",nFreq,1,szday);
					else
						*Length+=sprintf(Buffer+(*Length), "RRULE:MP%d %d+ %s #0",nFreq,whichWeek,szday);
				}
				break;
			case CAL_REPEAT_YEARLY : 
				*Length+=sprintf(Buffer+(*Length), "RRULE:YM%d #0",nFreq);
				break;
		}
		if(RepeatEndDate!=-1)
		{
			*Length+=sprintf(Buffer+(*Length), " %04d%02d%02dT%02d%02d%02d",
				note->Entries[RepeatEndDate].Date.Year,note->Entries[RepeatEndDate].Date.Month,note->Entries[RepeatEndDate].Date.Day,
				note->Entries[RepeatEndDate].Date.Hour,note->Entries[RepeatEndDate].Date.Minute,note->Entries[RepeatEndDate].Date.Second);
		}
		*Length+=sprintf(Buffer+(*Length), "%c%c",13,10);

		SaveVCALDateTime(Buffer, Length, &note->Entries[EndTime].Date, "DTSTART");

//DTEND
		if(RepeatEndDate!=-1)
			SaveVCALDateTime(Buffer, Length, &note->Entries[RepeatEndDate].Date, "DTEND");
		else
		{
			GSM_DateTime time;
			time.Year = 2100;
			time.Month = 12;
			time.Day = 31;
			time.Hour = time.Minute = time.Second = 0;
			SaveVCALDateTime(Buffer, Length, &time, "DTEND");
		}


	}	
	*Length+=sprintf(Buffer+(*Length), "END:VTODO%c%c",13,10);

	if (header) {
		*Length+=sprintf(Buffer+(*Length), "END:VCALENDAR%c%c",13,10);
	}
	return ERR_NONE;
}
GSM_Error GSM_DecodeVCALENDAR_VTODO(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, GSM_VCalendarVersion CalVer, GSM_VToDoVersion ToDoVer,int Timezone,bool bSupportDescription)
{
	unsigned char 	Line[2000],Buff[2000],	Line2[2000];
    int             Level = 0,num,npos;
	char szCalType[500];
	char  str[30];
//	time_t time;
	unsigned char	   	DescriptionText[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
	DescriptionText [0] =0;
	bool bSummary = false;
	bool bDescription = false;

	Calendar->EntriesNum 	= 0;
	ToDo->EntriesNum 	= 0;

	while (1) {
		MyGetLine(Buffer, Pos, Line, strlen((char *)Buffer));
		if (strlen((char *)Line) == 0) break;
		//Modify for SE K750i
		//=E6=88=91=E9=96=80=E6=98=A=..F=E4=B8=80=E5=AE=B6=E4=BA=BA..
		npos = *Pos;
		memcpy(Line2,Line,strlen((char*)Line)+1);
		while(Line2[strlen((char*)Line2)-1] == '=')
		{
			if(Buffer[npos+2] == 0x0d && Buffer[npos+3] == 0x0A)
			{
				if(Line[strlen((char*)Line)-1] == '=')
					Line[strlen((char*)Line)-1]= '\0';
				break;
			}
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
			if(Line2[0] == '=')
			{
				strcat((char *)Line,(char *)Line2+1);
			}
			else
			{
				Line[strlen((char*)Line)-1]= '\0';
				strcat((char *)Line,(char *)Line2);
			}
		}
		MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
		while(Line2[0] == 0x20 && Line2[1] == 0x20)
		{
			strcat((char *)Line,(char *)Line2);
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
		}

 /*		npos = *Pos;//peggy
        MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
  		//peggy +
		while(Line2[0] == '=')
		{
			strcat((char *)Line,(char *)Line2+1);

			if(Line2[strlen((char *)Line2) -1] == '=')
				MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
			else 
				break;
		}
		//peggy +*/
		switch (Level) {
		case 0:
			if (strstr((char *)Line,"BEGIN:VEVENT")) {
				Calendar->Type 	= GSM_CAL_MEMO;
				Level 		= 1;
			}
			if (strstr((char *)Line,"BEGIN:VTODO")) {
				ToDo->Priority 	= 2;//GSM_Priority_Low;
				Level 		= 2;
			}
			break;
		case 1: /* Calendar note */
			if (strstr((char *)Line,"END:VEVENT")) {
				if (Calendar->EntriesNum == 0) return ERR_EMPTY;
				if(bSupportDescription)
				{
					if(bDescription)
					{
						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_DESCRIPTION;
						CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,DescriptionText);
						UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
						Calendar->EntriesNum++;
					}
				}
				else
				{
					if(bSummary == false && bDescription)
					{
						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
						CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,DescriptionText);
						UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
						Calendar->EntriesNum++;
					}
				}
				return ERR_NONE;
			}
	/*		if (strstr(Line,"CATEGORIES:REMINDER")) 	Calendar->Type = GSM_CAL_REMINDER;
			if (strstr(Line,"CATEGORIES:DATE"))	 	Calendar->Type = GSM_CAL_REMINDER;//SE
			if (strstr(Line,"CATEGORIES:TRAVEL"))	 	Calendar->Type = GSM_CAL_TRAVEL;  //SE
			if (strstr(Line,"CATEGORIES:VACATION"))	 	Calendar->Type = GSM_CAL_VACATION;//SE
			if (strstr(Line,"CATEGORIES:MISCELLANEOUS")) 	Calendar->Type = GSM_CAL_MEMO;
			if (strstr(Line,"CATEGORIES:PHONE CALL")) 	Calendar->Type = GSM_CAL_CALL;
			if (strstr(Line,"CATEGORIES:SPECIAL OCCASION")) Calendar->Type = GSM_CAL_BIRTHDAY;
			if (strstr(Line,"CATEGORIES:ANNIVERSARY")) 	Calendar->Type = GSM_CAL_BIRTHDAY;
			if (strstr(Line,"CATEGORIES:MEETING")) 		Calendar->Type = GSM_CAL_MEETING;
			if (strstr(Line,"CATEGORIES:APPOINTMENT")) 	Calendar->Type = GSM_CAL_MEETING;
*/	////////// 
  			//peggy +
/*			if(Line2[0] == '=')
			{
				strcat((char *)Line,(char *)Line2+1);
			}*/
			//peggy +
			if (ReadVCALText(Line, "CATEGORIES", Buff) )
			{

				DecodeUnicode(Buff,(unsigned char *)szCalType);
				if (strstr(szCalType,"REMINDER")) 	Calendar->Type = GSM_CAL_REMINDER;
				if (strstr(szCalType,"DATE"))	 	Calendar->Type = GSM_CAL_REMINDER;//SE
				if (strstr(szCalType,"TRAVEL"))	 	Calendar->Type = GSM_CAL_TRAVEL;  //SE
				if (strstr(szCalType,"VACATION"))	 	Calendar->Type = GSM_CAL_VACATION;//SE
				if (strstr(szCalType,"MISCELLANEOUS")) 	Calendar->Type = GSM_CAL_MEMO;
				if (strstr(szCalType,"PHONE CALL")) 	Calendar->Type = GSM_CAL_CALL;
				if (strstr(szCalType,"SPECIAL OCCASION")) Calendar->Type = GSM_CAL_BIRTHDAY;
				if (strstr(szCalType,"ANNIVERSARY")) 	Calendar->Type = GSM_CAL_BIRTHDAY;
				if (strstr(szCalType,"MEETING")) 		Calendar->Type = GSM_CAL_MEETING;
				if (strstr(szCalType,"APPOINTMENT")) 	Calendar->Type = GSM_CAL_MEETING;

				if (strstr(szCalType,"Meeting")) 		Calendar->Type = GSM_CAL_MEETING;
				if (strstr(szCalType,"Memo")) 	Calendar->Type = GSM_CAL_MEMO;
				if (strstr(szCalType,"Birthday")) Calendar->Type = GSM_CAL_BIRTHDAY;
				if (strstr(szCalType,"Call")) Calendar->Type = GSM_CAL_REMINDER;
				if (strstr(szCalType,"Voice memo")) Calendar->Type =GSM_CAL_CALL ;
				if (strstr(szCalType,"Holiday")) Calendar->Type = GSM_CAL_VACATION;
				if (strstr(szCalType,"語音備忘")) 	Calendar->Type = GSM_CAL_MEMO;
				if (strstr(szCalType,"假期"))	 	Calendar->Type = GSM_CAL_VACATION;//SE
				if (strstr(szCalType,"文字提示")) 	Calendar->Type = GSM_CAL_REMINDER;
				if (strstr(szCalType,"撥叫")) 	Calendar->Type = GSM_CAL_CALL;
				if (strstr(szCalType,"生日")) Calendar->Type = GSM_CAL_BIRTHDAY;
				if (strstr(szCalType,"會議")) 		Calendar->Type = GSM_CAL_MEETING;
			}
			
            if (ReadVCALText(Line, "X-IRMC-LUID", Buff)) 
			{
				num=0;
				num=Searchstr(Line,(unsigned char *)"X-IRMC-LUID",11);
				if(num)
				{
					memcpy(str,&Line[num+11],30);
					if(CalVer == SonyEricsson_VCalendar)
					//	Calendar->Location=strtoul((char *)DecodeUnicodeString(Buff), NULL, 16);
						wsprintf(Calendar->szIndex,(char *)DecodeUnicodeString(Buff));
					else
					//	Calendar->Location=atoi(str);
						wsprintf(Calendar->szIndex,str);
				}
				
            }
	////////		
			if (strstr((char *)Line,"RRULE:D1")) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_DAILY ;//1*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1 ;
				Calendar->EntriesNum++;
			}
			if ((strstr((char *)Line,"RRULE:W1")) || (strstr((char *)Line,"RRULE:D7"))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY;//7*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1 ;
				Calendar->EntriesNum++;
			}
			if (strstr((char *)Line,"RRULE:W2")) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY;//14*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 2 ;
				Calendar->EntriesNum++;
			}
			if (strstr((char *)Line,"RRULE:MD1")) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_MONTHLY;//30*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1 ;
				Calendar->EntriesNum++;
			}
			if (strstr((char *)Line,"RRULE:YD1")) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_YEARLY;//365*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1 ;
				Calendar->EntriesNum++;
			}


		/*	if ((ReadVCALText(Line, "SUMMARY", Buff)) || (ReadVCALText(Line, "DESCRIPTION", Buff))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				Calendar->EntriesNum++;
				bSummary = true;
			}*/
			if ((ReadVCALText(Line, "SUMMARY", Buff))){// || (ReadVCALText(Line, "DESCRIPTION", Buff))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
				bSummary = true;
			}
			if ((ReadVCALText(Line, "DESCRIPTION", Buff))) 
			{
				CopyUnicodeString(DescriptionText,Buff);
				bDescription = true;
			}

			if (ReadVCALText(Line, "LOCATION", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LOCATION;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DTSTART", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_START_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);

				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DTEND", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_END_DATETIME;
				ReadVCALDateTime((char *)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
			
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DALARM", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
			
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "AALARM", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
			
				Calendar->EntriesNum++;
			}
			break;
		case 2: /* ToDo note */
			if (strstr((char *)Line,"END:VTODO")) {
				if (ToDo->EntriesNum == 0) return ERR_EMPTY;
				return ERR_NONE;
			}
			if (ReadVCALText(Line, "DUE", Buff)) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);

				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "COMPLETED", Buff)) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETEDDATE;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}

			if (ReadVCALText(Line, "DALARM", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_SILENT_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "AALARM", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "SUMMARY", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_TEXT;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				UnicodeReplace(ToDo->Entries[ToDo->EntriesNum].Text,"\\;",";");
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "PRIORITY", Buff)) {
			/*	if (ToDoVer == SonyEricsson_VToDo) {
					ToDo->Priority = GSM_Priority_Low;
					if (atoi((char *)DecodeUnicodeString(Buff))==2) ToDo->Priority = GSM_Priority_Medium;
					if (atoi((char *)DecodeUnicodeString(Buff))==1) ToDo->Priority = GSM_Priority_High;
					dbgprintf("atoi is %i %s\n",atoi((char *)DecodeUnicodeString(Buff)),DecodeUnicodeString(Buff));
				} else if (ToDoVer == Nokia_VToDo) {
					ToDo->Priority = GSM_Priority_Low;
					if (atoi((char *)DecodeUnicodeString(Buff))==2) ToDo->Priority = GSM_Priority_Medium;
					if (atoi((char *)DecodeUnicodeString(Buff))==3) ToDo->Priority = GSM_Priority_High;
				}*/
				 ToDo->Priority  = atoi((char *)DecodeUnicodeString(Buff));
			}
			if (strstr((char *)Line,"STATUS:COMPLETED")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 1;
				ToDo->EntriesNum++;
			}
			if (strstr((char *)Line,"STATUS:NEEDS ACTION")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 0;
				ToDo->EntriesNum++;
			}
   			if (ReadVCALText(Line, "CATEGORIES", Buff) )
			{
				DecodeUnicode(Buff,(unsigned char *)szCalType);
				if (strstr(szCalType,"MISCELLANEOUS")) 	ToDo->Type = GSM_CAL_MEETING;
				else if (strstr(szCalType,"PHONECALL")) 	ToDo->Type = GSM_CAL_CALL;
				else if (strstr(szCalType,"PHONE CALL")) 	ToDo->Type = GSM_CAL_TRAVEL;
				else if (strstr(szCalType,"MESSAGE")) 	ToDo->Type = GSM_CAL_REMINDER;
				else if (strstr(szCalType,"TASK")) 	ToDo->Type = GSM_CAL_MEMO;
				else ToDo->Type = GSM_CAL_MEETING;
			}
		     if (ReadVCALText(Line, "X-IRMC-LUID", Buff)) 
			{
				num=0;
				num=Searchstr(Line,(unsigned char *)"X-IRMC-LUID",11);
				if(num)
				{
					memcpy(str,&Line[num+11],30);
					wsprintf(ToDo->Location,str);
				/*	if(CalVer == SonyEricsson_VCalendar)
						ToDo->Location=strtoul((char *)DecodeUnicodeString(Buff), NULL, 16);
					else
						ToDo->Location=atoi(str);*/
				}
				
            }
			break;
		}
	}

	if (Calendar->EntriesNum == 0 && ToDo->EntriesNum == 0) return ERR_EMPTY;
	return ERR_NONE;
}

GSM_Error SamsungZ_DecodeVCALENDAR_VTODO(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, GSM_VCalendarVersion CalVer, GSM_VToDoVersion ToDoVer,int Timezone)
{
	unsigned char 	Line[2000],Buff[2000],	Line2[2000];
    int             Level = 0,num,npos;
	char szCalType[500];
	char  str[30];
//	time_t time;
	unsigned char	   	DescriptionText[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
	char * pos;
	DescriptionText [0] =0;
	bool bSummary = false;
	bool bDescription = false;

	Calendar->EntriesNum 	= 0;
	ToDo->EntriesNum 	= 0;

	while (1) {
		MyGetLine(Buffer, Pos, Line, strlen((char *)Buffer));
		if (strlen((char *)Line) == 0) break;
		//Modify for SE K750i
		//=E6=88=91=E9=96=80=E6=98=A=..F=E4=B8=80=E5=AE=B6=E4=BA=BA..
		npos = *Pos;
		memcpy(Line2,Line,strlen((char*)Line)+1);
		while(Line2[strlen((char*)Line2)-1] == '=')
		{
			if(Buffer[npos+2] == 0x0d && Buffer[npos+3] == 0x0A)
			{
				if(Line[strlen((char*)Line)-1] == '=')
					Line[strlen((char*)Line)-1]= '\0';
				break;
			}
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
			if(Line2[0] == '=')
			{
				strcat((char *)Line,(char *)Line2+1);
			}
			else
			{
				Line[strlen((char*)Line)-1]= '\0';
				strcat((char *)Line,(char *)Line2);
			}
		}
		MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
		while(Line2[0] == 0x20 /*&& Line2[1] == 0x20*/)
		{
			int nShiftIndex = 1;
			if(Line2[1] == 0x20) nShiftIndex =  2;

			strcat((char *)Line,(char *)Line2+nShiftIndex);
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
		}
		switch (Level) {
		case 0:
			if(strstr((char *)Line, "<![CDATA["))
			{
				CString csRemove = Line;
				csRemove.Replace("<![CDATA[","");
				sprintf((char *)Line,"%s",csRemove);
			}
			//changed from to BEGIN:VEVENT -> VEVENT  because was getting <![CDATA[VEVENT only
			if(strlen((char *)Line) <=0) break;
			if (strstr((char *)Line,"VEVENT")) {
				Calendar->Type 	= GSM_CAL_MEETING;
				Level 		= 1;
			}
			if (strstr((char *)Line,"VTODO")) {
				ToDo->Priority 	= 2;//GSM_Priority_Low;
				Level 		= 2;
			}
			break;
		case 1: /* Calendar note */
			if(strstr((char *)Line, "<![CDATA["))
			{
				CString csRemove = Line;
				csRemove.Replace("<![CDATA[","");
				sprintf((char *)Line,"%s",csRemove);
			}
			if (strstr((char *)Line,"END:VEVENT")) 
			{
				if (Calendar->EntriesNum == 0) 	return ERR_EMPTY;
				return ERR_NONE;
			}
			
            if (ReadVCALText(Line, "X-IRMC-LUID", Buff)) 
			{
				num=0;
				num=Searchstr(Line,(unsigned char *)"X-IRMC-LUID",11);
				if(num)
				{
					memcpy(str,&Line[num+11],30);
					if(CalVer == SonyEricsson_VCalendar)
					//	Calendar->Location=strtoul((char *)DecodeUnicodeString(Buff), NULL, 16);
						wsprintf(Calendar->szIndex,(char *)DecodeUnicodeString(Buff));
					else
					//	Calendar->Location=atoi(str);
						wsprintf(Calendar->szIndex,str);
				}
				
            }
			if (strstr((char *)Line,"RRULE:D1")) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_DAILY ;//1*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1 ;
				Calendar->EntriesNum++;

				pos = strstr((char *)Line, "RRULE:D1 ");
				pos += 9;
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
				ReadVCALDateTime(pos, &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);

				Calendar->EntriesNum++;
			}
			if (strstr((char *)Line,"RRULE:W1"))
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY;//7*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1 ;
				Calendar->EntriesNum++;

				pos = strstr((char *)Line, "RRULE:W1 ");
				pos += 9;
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
				ReadVCALDateTime(pos, &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
				Calendar->EntriesNum++;
			}
			if (strstr((char *)Line,"RRULE:W2")) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY;//14*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 2 ;
				Calendar->EntriesNum++;

				pos = strstr((char *)Line, "RRULE:W2 ");
				pos += 9;
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
				ReadVCALDateTime(pos, &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
				Calendar->EntriesNum++;
			}
			if (strstr((char *)Line,"RRULE:MD1")) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_MONTHLY;//30*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1 ;
				Calendar->EntriesNum++;

				pos = strstr((char *)Line, "RRULE:MD1 ");
				pos += 10;
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
				ReadVCALDateTime(pos, &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
				Calendar->EntriesNum++;
			}
			if (strstr((char *)Line,"RRULE:YD1")) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_YEARLY;//365*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1 ;
				Calendar->EntriesNum++;

				pos = strstr((char *)Line, "RRULE:YD1 ");
				pos += 10;
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
				ReadVCALDateTime(pos, &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
				Calendar->EntriesNum++;
			}
			if ((ReadVCALText(Line, "SUMMARY", Buff))){
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
			}
			if ((ReadVCALText(Line, "DESCRIPTION", Buff))) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_DESCRIPTION;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
			}

			if (ReadVCALText(Line, "LOCATION", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LOCATION;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DTSTART", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_START_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DTEND", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_END_DATETIME;
				ReadVCALDateTime((char *)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
			
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DALARM", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
			
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "AALARM", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
				Calendar->EntriesNum++;
			}
			break;
		case 2: /* ToDo note */
			if(strstr((char *)Line, "<![CDATA["))
			{
				CString csRemove = Line;
				csRemove.Replace("<![CDATA[","");
				sprintf((char *)Line,"%s",csRemove);
			}
			if (strstr((char *)Line,"END:VTODO")) {
				if (ToDo->EntriesNum == 0) return ERR_EMPTY;
				return ERR_NONE;
			}
			if (ReadVCALText(Line, "DTSTART", Buff))
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_START_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "DUE", Buff)) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);

				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "COMPLETED", Buff)) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETEDDATE;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}

			if (ReadVCALText(Line, "DALARM", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_SILENT_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "AALARM", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "SUMMARY", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_TEXT;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				UnicodeReplace(ToDo->Entries[ToDo->EntriesNum].Text,"\\;",";");
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "PRIORITY", Buff)) {
				 ToDo->Priority  = atoi((char *)DecodeUnicodeString(Buff));
			}
			if (ReadVCALText(Line, "DESCRIPTION", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_DESCRIPTION;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				UnicodeReplace(ToDo->Entries[ToDo->EntriesNum].Text,"\\;",";");
				ToDo->EntriesNum++;
			}
	/*		if (ReadVCALText(Line, "LAST-MODIFIED", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = ??????? see if needed;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				ToDo->EntriesNum++;
			}*/
			if (strstr((char *)Line,"STATUS:COMPLETED")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 1;
				ToDo->EntriesNum++;
			}
			if (strstr((char *)Line,"STATUS:NEEDS ACTION")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 0;
				ToDo->EntriesNum++;
			}
   			if (ReadVCALText(Line, "CATEGORIES", Buff) )
			{
				DecodeUnicode(Buff,(unsigned char *)szCalType);
				if (strstr(szCalType,"MISCELLANEOUS")) 	ToDo->Type = GSM_CAL_MEETING;
				else if (strstr(szCalType,"PHONECALL")) 	ToDo->Type = GSM_CAL_CALL;
				else if (strstr(szCalType,"PHONE CALL")) 	ToDo->Type = GSM_CAL_TRAVEL;
				else if (strstr(szCalType,"MESSAGE")) 	ToDo->Type = GSM_CAL_REMINDER;
				else if (strstr(szCalType,"TASK")) 	ToDo->Type = GSM_CAL_MEMO;
				else ToDo->Type = GSM_CAL_MEETING;
			}
		    if (ReadVCALText(Line, "X-IRMC-LUID", Buff)) 
			{
				num=0;
				num=Searchstr(Line,(unsigned char *)"X-IRMC-LUID",11);
				if(num)
				{
					memcpy(str,&Line[num+11],30);
					wsprintf(ToDo->Location,str);
				/*	if(CalVer == SonyEricsson_VCalendar)
						ToDo->Location=strtoul((char *)DecodeUnicodeString(Buff), NULL, 16);
					else
						ToDo->Location=atoi(str);*/
				}
				
            }
			break;
		}
	}

	if (Calendar->EntriesNum == 0 && ToDo->EntriesNum == 0) return ERR_EMPTY;
	return ERR_NONE;
}

GSM_Error SAGEM_DecodeVCALENDAR_VTODO(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, int Timezone)
{
	unsigned char 	Line[2000],Buff[2000],	Line2[2000];
    int             Level = 0,num,npos;
	char szCalType[500];
	char  str[30];
	char szTime[MAX_PATH];
//	time_t time;
	unsigned char	   	DescriptionText[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
	DescriptionText [0] =0;
	bool bSummary = false;
	bool bDescription = false;

	Calendar->EntriesNum 	= 0;
	ToDo->EntriesNum 	= 0;

	while (1) {
		MyGetLine(Buffer, Pos, Line, strlen((char *)Buffer));
		if (strlen((char *)Line) == 0) break;
		//Modify for SE K750i
		//=E6=88=91=E9=96=80=E6=98=A=..F=E4=B8=80=E5=AE=B6=E4=BA=BA..
		npos = *Pos;
		memcpy(Line2,Line,strlen((char*)Line)+1);
		while(Line2[strlen((char*)Line2)-1] == '=')
		{
			if(Buffer[npos+2] == 0x0d && Buffer[npos+3] == 0x0A)
			{
				if(Line[strlen((char*)Line)-1] == '=')
					Line[strlen((char*)Line)-1]= '\0';
				break;
			}
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
			if(Line2[0] == '=')
			{
				strcat((char *)Line,(char *)Line2+1);
			}
			else
			{
				Line[strlen((char*)Line)-1]= '\0';
				strcat((char *)Line,(char *)Line2);
			}
		}
		MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
		while(Line2[0] == 0x20 && Line2[1] == 0x20)
		{
			strcat((char *)Line,(char *)Line2);
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
		}

 /*		npos = *Pos;//peggy
        MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
  		//peggy +
		while(Line2[0] == '=')
		{
			strcat((char *)Line,(char *)Line2+1);

			if(Line2[strlen((char *)Line2) -1] == '=')
				MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
			else 
				break;
		}
		//peggy +*/
		switch (Level) {
		case 0:
			if (strstr((char *)Line,"BEGIN:VEVENT")) {
				Calendar->Type 	= GSM_CAL_MEMO;
				Level 		= 1;
			}
			if (strstr((char *)Line,"BEGIN:VTODO")) {
				ToDo->Priority 	= 2;//GSM_Priority_Low;
				Level 		= 2;
			}
			break;
		case 1: /* Calendar note */
			if (strstr((char *)Line,"END:VEVENT")) {
				if (Calendar->EntriesNum == 0) return ERR_EMPTY;
				if(bSummary == false && bDescription)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
					CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,DescriptionText);
					UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
					Calendar->EntriesNum++;
				}
				return ERR_NONE;
			}
			if (SAGEM_ReadVCALText(Line, "CATEGORIES", Buff) )
			{
				DecodeUnicode(Buff,(unsigned char *)szCalType);
				if (strstr(szCalType,"EVENT")) 	Calendar->Type = GSM_CAL_MEMO;
				if (strstr(szCalType,"HOLIDAY"))	 	Calendar->Type = GSM_CAL_VACATION;//SE
				if (strstr(szCalType,"TRAVEL"))	 	Calendar->Type = GSM_CAL_TRAVEL;  //SE
				if (strstr(szCalType,"BIRTHDAY")) 	Calendar->Type = GSM_CAL_BIRTHDAY;
				if (strstr(szCalType,"MEETING")) 		Calendar->Type = GSM_CAL_MEETING;
				if (strstr(szCalType,"APPOINTMENT")) 	Calendar->Type = GSM_CAL_REMINDER;
			}
			
	////////		
			char *pTempFrequency;
			if ((pTempFrequency = strstr((char *)Line,"RRULE:D"))) 
			{
				bool bSupportedRepeat = true;
				int nFrequency = atoi(pTempFrequency+7);

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				if(nFrequency == 7)
				{
					Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY ;
					nFrequency = 1;
				}
				else
				{
					int DayOfWeekMask = 0;
					if(strstr((char *)Line,"SU"))
						DayOfWeekMask |=1;
					if(strstr((char *)Line,"MO"))
						DayOfWeekMask |=2;
					if(strstr((char *)Line,"TU"))
						DayOfWeekMask |=4;
					if(strstr((char *)Line,"WE"))
						DayOfWeekMask |=8;
					if(strstr((char *)Line,"TH"))
						DayOfWeekMask |=16;
					if(strstr((char *)Line,"FR"))
						DayOfWeekMask |=32;
					if(strstr((char *)Line,"SA"))
						DayOfWeekMask |=64;

					if(DayOfWeekMask == 62)
					{
						Calendar->Entries[Calendar->EntriesNum].Number = CAL_REPEAT_WEEKLY;
						Calendar->EntriesNum++;
						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_DAYOFWEEK;
						Calendar->Entries[Calendar->EntriesNum].Number    = DayOfWeekMask  ;
					}
					else if(DayOfWeekMask == 65)
					{
						Calendar->Entries[Calendar->EntriesNum].Number = CAL_REPEAT_WEEKLY;
						Calendar->EntriesNum++;
						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_DAYOFWEEK;
						Calendar->Entries[Calendar->EntriesNum].Number    = DayOfWeekMask  ;
					}
					else if(DayOfWeekMask == 0 || DayOfWeekMask == 127)
						Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_DAILY ;
					else bSupportedRepeat = false;
				}
				if(bSupportedRepeat)
				{
					Calendar->EntriesNum++;

					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
					Calendar->Entries[Calendar->EntriesNum].Number    = nFrequency ;

					Calendar->EntriesNum++;
				}

			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:W"))) 
			{
		//	if ((strstr((char *)Line,"RRULE:W1")) || (strstr((char *)Line,"RRULE:D7"))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY;//7*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+7) ;
				Calendar->EntriesNum++;

		/*		int DayOfWeekMask = 0;
				if(strstr((char *)Line,"SU"))
					DayOfWeekMask |=1;
				if(strstr((char *)Line,"MO"))
					DayOfWeekMask |=2;
				if(strstr((char *)Line,"TU"))
					DayOfWeekMask |=4;
				if(strstr((char *)Line,"WE"))
					DayOfWeekMask |=8;
				if(strstr((char *)Line,"TH"))
					DayOfWeekMask |=16;
				if(strstr((char *)Line,"FR"))
					DayOfWeekMask |=32;
				if(strstr((char *)Line,"SA"))
					DayOfWeekMask |=64;
				if(DayOfWeekMask>0)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_DAYOFWEEK;
					Calendar->Entries[Calendar->EntriesNum].Number    = DayOfWeekMask  ;
					Calendar->EntriesNum++;
				}*/

			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:MD"))) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_MONTHLY;//30*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+8)  ;
				Calendar->EntriesNum++;
			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:MP"))) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_MONTH_WEEKLY;//30*24;
				Calendar->EntriesNum++;


				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+8)  ;
				Calendar->EntriesNum++;

			}

			if ((pTempFrequency = strstr((char *)Line,"RRULE:YM"))) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_YEARLY;//365*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+8) ;
				Calendar->EntriesNum++;
			}



		/*	if ((ReadVCALText(Line, "SUMMARY", Buff)) || (ReadVCALText(Line, "DESCRIPTION", Buff))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				Calendar->EntriesNum++;
				bSummary = true;
			}*/
			if ((SAGEM_ReadVCALText(Line, "SUMMARY", Buff))){// || (SAGEM_ReadVCALText(Line, "DESCRIPTION", Buff))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
				bSummary = true;
			}
			if ((SAGEM_ReadVCALText(Line, "DESCRIPTION", Buff))) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LOCATION;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
			}

	//		if (SAGEM_ReadVCALText(Line, "LOCATION", Buff)) {
	//			Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LOCATION;
	//			CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
	//			Calendar->EntriesNum++;
	//		}
			if (SAGEM_ReadVCALText(Line, "DTSTART", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_START_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
				Calendar->EntriesNum++;
			}
			if (SAGEM_ReadVCALText(Line, "DTEND", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_END_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
			
				Calendar->EntriesNum++;
			}
			if (SAGEM_ReadVCALText(Line, "DALARM", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
				Calendar->EntriesNum++;
			}
			if (SAGEM_ReadVCALText(Line, "AALARM", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_ALARM_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
				Calendar->EntriesNum++;
			}
			break;
		case 2: /* ToDo note */
			if (strstr((char *)Line,"END:VTODO")) {
				if (ToDo->EntriesNum == 0) return ERR_EMPTY;
				ToDo->Priority = 0;
				return ERR_NONE;
			}
			if (SAGEM_ReadVCALText(Line, "DUE", Buff)) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}
			if (SAGEM_ReadVCALText(Line, "COMPLETED", Buff)) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETEDDATE;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);

				ToDo->EntriesNum++;
			}

			if (SAGEM_ReadVCALText(Line, "DALARM", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_SILENT_ALARM_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}
			if (SAGEM_ReadVCALText(Line, "AALARM", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_ALARM_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}
			if (SAGEM_ReadVCALText(Line, "SUMMARY", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_TEXT;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				UnicodeReplace(ToDo->Entries[ToDo->EntriesNum].Text,"\\;",";");
				ToDo->EntriesNum++;
			}
			if (SAGEM_ReadVCALText(Line, "DESCRIPTION", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_DESCRIPTION;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				UnicodeReplace(ToDo->Entries[ToDo->EntriesNum].Text,"\\;",";");
				ToDo->EntriesNum++;
			}
	//		if (SAGEM_ReadVCALText(Line, "PRIORITY", Buff)) {
	//			 ToDo->Priority  = atoi((char *)DecodeUnicodeString(Buff));
	//		}
			if (strstr((char *)Line,"STATUS:COMPLETED")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 1;
				ToDo->EntriesNum++;
			}
			if (strstr((char *)Line,"STATUS:NEEDS ACTION")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 0;
				ToDo->EntriesNum++;
			}
   			if (SAGEM_ReadVCALText(Line, "CATEGORIES", Buff) )
			{
				DecodeUnicode(Buff,(unsigned char *)szCalType);
				if (strstr(szCalType,"UNKNOWN")) 	ToDo->Type = GSM_CAL_MEETING;
				else if (strstr(szCalType,"SHOPPING LIST")) 	ToDo->Type = GSM_CAL_CALL;
				else if (strstr(szCalType,"BUSINESS")) 	ToDo->Type = GSM_CAL_REMINDER;
				else if (strstr(szCalType,"PHONE CALL")) 	ToDo->Type = GSM_CAL_TRAVEL;
				else if (strstr(szCalType,"PERSONAL")) 	ToDo->Type = GSM_CAL_MEMO;
				else if (strstr(szCalType,"IDEA")) 	ToDo->Type = GSM_CAL_VACATION;
				else if (strstr(szCalType,"GIFT")) 	ToDo->Type = GSM_CAL_BUSINESS;
				else ToDo->Type = GSM_CAL_MEETING;
			}
		     if (SAGEM_ReadVCALText(Line, "X-IRMC-LUID", Buff)) 
			{
				num=0;
				num=Searchstr(Line,(unsigned char *)"X-IRMC-LUID",11);
				if(num)
				{
					memcpy(str,&Line[num+11],30);
					wsprintf(ToDo->Location,str);
				/*	if(CalVer == SonyEricsson_VCalendar)
						ToDo->Location=strtoul((char *)DecodeUnicodeString(Buff), NULL, 16);
					else
						ToDo->Location=atoi(str);*/
				}
				
            }
			break;
		}
	}

	if (Calendar->EntriesNum == 0 && ToDo->EntriesNum == 0) return ERR_EMPTY;
	return ERR_NONE;
}

// 9@9u
GSM_Error SF_DecodeVCALENDAR_VTODO(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, int Timezone)
{
	unsigned char 	Line[2000],Buff[2000],	Line2[2000];
    int             Level = 0,num,npos;
	char szCalType[500];
	char  str[30];
	char szTime[MAX_PATH];
//	time_t time;
	unsigned char	   	DescriptionText[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
	DescriptionText [0] =0;
	bool bSummary = false;
	bool bDescription = false;
	char			*temppos;
	int  i;

	Calendar->EntriesNum 	= 0;
	ToDo->EntriesNum 	= 0;

	// 12/25 added for UID
	temppos = strstr((char*)Buffer, "UID:");
    if (temppos == NULL)
	{
     return ERR_NONE; // ???
	}
	temppos += 4; 

    while (*temppos && !isdigit(*temppos)) temppos++;
	for ( i = 0 ; *(temppos + i ) != 0x0D ; i++ )
        Calendar->szIndex[i] = *(temppos + i );

	Calendar->szIndex[i] ='\0' ;		 
			

	while (1) {
		MyGetLine(Buffer, Pos, Line, strlen((char *)Buffer));
		if (strlen((char *)Line) == 0) break;
		npos = *Pos;
		memcpy(Line2,Line,strlen((char*)Line)+1);
		while(Line2[strlen((char*)Line2)-1] == '=')
		{
			if(Buffer[npos+2] == 0x0d && Buffer[npos+3] == 0x0A)
			{
				if(Line[strlen((char*)Line)-1] == '=')
					Line[strlen((char*)Line)-1]= '\0';
				break;
			}
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
			if(Line2[0] == '=')
			{
				strcat((char *)Line,(char *)Line2+1);
			}
			else
			{
				Line[strlen((char*)Line)-1]= '\0';
				strcat((char *)Line,(char *)Line2);
			}
		}
		MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
		while(Line2[0] == 0x20 && Line2[1] == 0x20)
		{
			strcat((char *)Line,(char *)Line2);
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
		}

 /*		npos = *Pos;//peggy
        MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
  		//peggy +
		while(Line2[0] == '=')
		{
			strcat((char *)Line,(char *)Line2+1);

			if(Line2[strlen((char *)Line2) -1] == '=')
				MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
			else 
				break;
		}
		//peggy +*/



		switch (Level) {
		case 0:
			if (strstr((char *)Line,"BEGIN:VEVENT")) {
		//		Calendar->Type 	= GSM_CAL_MEMO;
				Level 		= 1;
			}
			if (strstr((char *)Line,"BEGIN:VTODO")) {
			//	ToDo->Priority 	= 2;//GSM_Priority_Low;
			//	Level 		= 2;
                Level 		= 1; // fro 9@9u
			}
			break;
		case 1: /* Calendar note */
			if (strstr((char *)Line,"END:VEVENT")) {
				if (Calendar->EntriesNum == 0) return ERR_EMPTY;
			/*??? 9@9u mingfa
				if(bSummary == false && bDescription)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
					CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,DescriptionText);
					UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
					Calendar->EntriesNum++;
				}
			*/
				return ERR_NONE;
			}

			if (strstr((char *)Line,"END:VTODO")) {
				if (Calendar->EntriesNum == 0) return ERR_EMPTY;
				return ERR_NONE;
			}

			// 12/25 added for UID
			/*
			if (SAGEM_ReadVCALText(Line, "UID", Buff)) {
	         sprintf(Calendar->szIndex,"%s",DecodeUnicodeString(Buff));			 
			}
            */

			if (SAGEM_ReadVCALText(Line, "CATEGORIES", Buff) )
			{
				DecodeUnicode(Buff,(unsigned char *)szCalType);
				if (strstr(szCalType,"Meeting")) 	Calendar->Type = GSM_CAL_MEETING;
				else if (strstr(szCalType,"Call")) 	Calendar->Type = GSM_CAL_CALL;
				else if (strstr(szCalType,"Reminder")) 	Calendar->Type = GSM_CAL_REMINDER;
				else if (strstr(szCalType,"Anniversary")) 	Calendar->Type = GSM_CAL_BIRTHDAY;
				else if (strstr(szCalType,"Course")) 	Calendar->Type = GSM_CAL_TRAVEL;
				else if (strstr(szCalType,"Date")) 	Calendar->Type = GSM_CAL_MEMO;
				else Calendar->Type = GSM_CAL_MEETING;			
			
			}
			
	////////		
			char *pTempFrequency;
			if ((pTempFrequency = strstr((char *)Line,"RRULE:D"))) // len =7
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_DAILY;//7*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1 ;
				Calendar->EntriesNum++;
                /* 12/29
				bool bSupportedRepeat = true;
				int nFrequency = atoi(pTempFrequency+7);

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				if(nFrequency == 7)
				{
					Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY ;
					nFrequency = 1;
				}
				else
				{
					int DayOfWeekMask = 0;
					if(strstr((char *)Line,"SU"))
						DayOfWeekMask |=1;
					if(strstr((char *)Line,"MO"))
						DayOfWeekMask |=2;
					if(strstr((char *)Line,"TU"))
						DayOfWeekMask |=4;
					if(strstr((char *)Line,"WE"))
						DayOfWeekMask |=8;
					if(strstr((char *)Line,"TH"))
						DayOfWeekMask |=16;
					if(strstr((char *)Line,"FR"))
						DayOfWeekMask |=32;
					if(strstr((char *)Line,"SA"))
						DayOfWeekMask |=64;


					if(DayOfWeekMask == 62)
					{
						Calendar->Entries[Calendar->EntriesNum].Number = CAL_REPEAT_WEEKLY;
						Calendar->EntriesNum++;
						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_DAYOFWEEK;
						Calendar->Entries[Calendar->EntriesNum].Number    = DayOfWeekMask  ;
					}
					else if(DayOfWeekMask == 65)
					{
						Calendar->Entries[Calendar->EntriesNum].Number = CAL_REPEAT_WEEKLY;
						Calendar->EntriesNum++;
						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_DAYOFWEEK;
						Calendar->Entries[Calendar->EntriesNum].Number    = DayOfWeekMask  ;
					}
					else if(DayOfWeekMask == 0 || DayOfWeekMask == 127)
						Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_DAILY ;
					else bSupportedRepeat = false;
				}
				if(bSupportedRepeat)
				{
					Calendar->EntriesNum++;

					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
					Calendar->Entries[Calendar->EntriesNum].Number    = nFrequency ;

					Calendar->EntriesNum++;
				}
               */
			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:W"))) // len = 7
			{
		//	if ((strstr((char *)Line,"RRULE:W1")) || (strstr((char *)Line,"RRULE:D7"))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY;//7*24;
				Calendar->EntriesNum++;

	//			Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
	//			Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+7) ;
	//			Calendar->EntriesNum++;

              // 12/29
				int freqnum = 0;
				int DayOfWeekMask = 0;
				if(strstr((char *)Line,"SU"))
				{
					DayOfWeekMask |=1;
					freqnum++;
				}
				if(strstr((char *)Line,"MO"))
				{
					DayOfWeekMask |=2;
                    freqnum++;
				}
				if(strstr((char *)Line,"TU"))
				{
					DayOfWeekMask |=4;
                    freqnum++;
				}
				if(strstr((char *)Line,"WE"))
				{
					DayOfWeekMask |=8;
					freqnum++;
				}
				if(strstr((char *)Line,"TH"))
				{
					DayOfWeekMask |=16;
					freqnum++;
				}
				if(strstr((char *)Line,"FR"))
				{
					DayOfWeekMask |=32;
					freqnum++;
				}
				if(strstr((char *)Line,"SA"))
				{
					DayOfWeekMask |=64;
					freqnum++;
				}
				if(DayOfWeekMask>0)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_DAYOFWEEK;
					Calendar->Entries[Calendar->EntriesNum].Number    = DayOfWeekMask  ;
					Calendar->EntriesNum++;
				}
				else
                  freqnum = 1;


				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1; //freqnum ;  //20080108
				Calendar->EntriesNum++;


			}
//			if ((pTempFrequency = strstr((char *)Line,"RRULE:MD"))) // len = 8
			if ((pTempFrequency = strstr((char *)Line,"RRULE:M"))) // len = 8 ; 0103
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_MONTHLY;//30*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
			//	Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+8)  ;
				Calendar->Entries[Calendar->EntriesNum].Number    = 1  ;
				Calendar->EntriesNum++;
			}
			/*
			if ((pTempFrequency = strstr((char *)Line,"RRULE:MP"))) // 9@9u no used
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_MONTH_WEEKLY;//30*24;
				Calendar->EntriesNum++;


				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+8)  ;
				Calendar->EntriesNum++;

			}
			*/
          
//			if ((pTempFrequency = strstr((char *)Line,"RRULE:YM"))) 
			if ((pTempFrequency = strstr((char *)Line,"RRULE:Y"))) //0103
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_YEARLY;//365*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
			//	Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+8) ;
                Calendar->Entries[Calendar->EntriesNum].Number    = 1;
				Calendar->EntriesNum++;
			}

          


			if ((SAGEM_ReadVCALText(Line, "SUMMARY", Buff))){// || (SAGEM_ReadVCALText(Line, "DESCRIPTION", Buff))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");// ???
				Calendar->EntriesNum++;
				bSummary = true;
			}
			// 9@9u
			if ((SAGEM_ReadVCALText(Line, "LOCATION", Buff))) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LOCATION;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
			}

	//		if (SAGEM_ReadVCALText(Line, "DESCRIPTION", Buff)) {
	//			Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LOCATION;
	//			CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
	//			Calendar->EntriesNum++;
	//		}
			if (SAGEM_ReadVCALText(Line, "DTSTART", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_START_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
				Calendar->EntriesNum++;
			}
			if (SAGEM_ReadVCALText(Line, "DTEND", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_END_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
			
				Calendar->EntriesNum++;
			}
			// 20080115 fro TODO Type
			if (SAGEM_ReadVCALText(Line, "DUE", Buff)) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_END_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
			
				Calendar->EntriesNum++;
			}

			if (SAGEM_ReadVCALText(Line, "DALARM", Buff)) {
				// 9@9u marked by mingfa , alarm base on AALARM
				/*
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
				Calendar->EntriesNum++;
				*/
			}
			if (SAGEM_ReadVCALText(Line, "AALARM", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_ALARM_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,Timezone);
				Calendar->EntriesNum++;
			}
			if (SAGEM_ReadVCALText(Line, "PRIORITY", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_PRIVATE;
				Calendar->Entries[Calendar->EntriesNum].Number    = atoi((char *)DecodeUnicodeString(Buff));
				// ToDo->Priority  = atoi((char *)DecodeUnicodeString(Buff));
				 Calendar->EntriesNum++;
			}

			break;
		case 2: /* ToDo note */
			if (strstr((char *)Line,"END:VTODO")) {
				if (ToDo->EntriesNum == 0) return ERR_EMPTY;
				ToDo->Priority = 0; //??? init 
				return ERR_NONE;
			}
			if (SAGEM_ReadVCALText(Line, "DUE", Buff)) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}
			/**??? 9@9u unused
			if (SAGEM_ReadVCALText(Line, "COMPLETED", Buff)) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETEDDATE;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);

				ToDo->EntriesNum++;
			}
			*/
            // ??? mingfa , why DALARM and AALARM are exist both
			if (SAGEM_ReadVCALText(Line, "DALARM", Buff)) {
				// 9@9u marked by mingfa , alarm base on AALARM
				/*
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_SILENT_ALARM_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
				*/
			}
			if (SAGEM_ReadVCALText(Line, "AALARM", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_ALARM_DATETIME;
				sprintf(szTime,(char*)DecodeUnicodeString(Buff));
				ReadVCALDateTime(szTime, &ToDo->Entries[ToDo->EntriesNum].Date,Timezone);
				ToDo->EntriesNum++;
			}
			if (SAGEM_ReadVCALText(Line, "SUMMARY", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_TEXT;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				UnicodeReplace(ToDo->Entries[ToDo->EntriesNum].Text,"\\;",";");// ???
				ToDo->EntriesNum++;
			}
			/**??? 9@9u unused
			if (SAGEM_ReadVCALText(Line, "DESCRIPTION", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_DESCRIPTION;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				UnicodeReplace(ToDo->Entries[ToDo->EntriesNum].Text,"\\;",";");
				ToDo->EntriesNum++;
			}
			**/

			if (SAGEM_ReadVCALText(Line, "PRIORITY", Buff)) {
				 ToDo->Priority  = atoi((char *)DecodeUnicodeString(Buff));
			}
            /**??? 9@9u unused
			if (strstr((char *)Line,"STATUS:COMPLETED")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 1;
				ToDo->EntriesNum++;
			}
			if (strstr((char *)Line,"STATUS:NEEDS ACTION")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 0;
				ToDo->EntriesNum++;
			}
			**/

   			if (SAGEM_ReadVCALText(Line, "CATEGORIES", Buff) )
			{
				DecodeUnicode(Buff,(unsigned char *)szCalType);
				if (strstr(szCalType,"Meeting")) 	ToDo->Type = GSM_CAL_MEETING;
				else if (strstr(szCalType,"Call")) 	ToDo->Type = GSM_CAL_CALL;
				else if (strstr(szCalType,"Reminder")) 	ToDo->Type = GSM_CAL_REMINDER;
				else if (strstr(szCalType,"Anniversary")) 	ToDo->Type = GSM_CAL_BIRTHDAY;
				else if (strstr(szCalType,"Course")) 	ToDo->Type = GSM_CAL_TRAVEL;
				else if (strstr(szCalType,"Date")) 	ToDo->Type = GSM_CAL_MEMO;
				else ToDo->Type = GSM_CAL_MEETING;
			}

			break;
		}
	}

	if (Calendar->EntriesNum == 0 && ToDo->EntriesNum == 0) return ERR_EMPTY;
	return ERR_NONE;
}


/*
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
/**			if (strstr(Line,"END:VEVENT")) {
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
/**			if (strstr(Line,"END:VTODO")) {
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
*/
GSM_Error GSM_EncodeVNTFile(unsigned char *Buffer, int *Length, GSM_NoteEntry *Note)
{
	*Length+=sprintf((char *)Buffer+(*Length), "BEGIN:VNOTE%c%c",13,10);
	*Length+=sprintf((char *)Buffer+(*Length), "VERSION:1.1%c%c",13,10);
	SaveVCALText((char*)Buffer, Length,(unsigned char *) Note->Text, "BODY");
	*Length+=sprintf((char *)Buffer+(*Length), "END:VNOTE%c%c",13,10);

	return ERR_NONE;
}
void SESaveVCALText(char *Buffer, int *Length, unsigned char *Text, char *Start,char *beforetext)
{
	unsigned char buffer[1000];

	if (UnicodeLength(Text) != 0) {
//		EncodeCString2UTF8(DecodeUnicodeString(Text),buffer);
		EncodeUnicode2UTF8(Text,buffer);// for _UNICODE
		if (UnicodeLength(Text)==strlen((char*)buffer)) {
			*Length+=sprintf(Buffer+(*Length), "%s%s%s%c%c",Start,beforetext,DecodeUnicodeString(Text),13,10);
		} else {
	//		*Length+=sprintf(Buffer+(*Length), "%s;CHARSET=UTF-8%s%s%c%c",Start,beforetext,buffer,13,10);
			EncodeUnicode2UTF7(Text,buffer);
			*Length+=sprintf(Buffer+(*Length), "%s;CHARSET=UTF-7%s%s%c%c",Start,beforetext,buffer,13,10);
		}
	}	    

}
GSM_Error SONYERIC_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version,int Timezone)
{
 	int 	Text, Time, Alarm, Phone, Recurrance, EndTime, Location,RecurranceFreq,RepeatEndDate,Description,DayofweekMask;
//	char 	buffer[2000];

//	GSM_CalendarFindDefaultTextTimeAlarmPhoneRecurrance(note, &Text, &Time, &Alarm, &Phone, &Recurrance,&RecurranceFreq, &EndTime, &Location);
   GSM_CalendarFindEntryIndex(note, &Text, &Time, &Alarm, &Phone, &Recurrance,&RecurranceFreq, &EndTime, &Location,&RepeatEndDate,&Description,&DayofweekMask);

	if (header) {
		*Length+=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);
		*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c",13,10);
	}
	*Length+=sprintf(Buffer+(*Length), "BEGIN:VEVENT%c%c",13,10);


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

	GSM_DateTime gsmDateTime;
	memcpy(&gsmDateTime,&note->Entries[Time].Date,sizeof(GSM_DateTime));

	if(Timezone>0)
		GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
	else
		GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);

	SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DTSTART");

	if (EndTime != -1) {
		memcpy(&gsmDateTime,&note->Entries[EndTime].Date,sizeof(GSM_DateTime));
		if(Timezone>0)
			GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
		else
			GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);

		SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DTEND");
	}

	if (Alarm != -1) {
		memcpy(&gsmDateTime,&note->Entries[Alarm].Date,sizeof(GSM_DateTime));
		if(Timezone>0)
			GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
		else
			GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);

		SaveVCALDateTime(Buffer, Length, &gsmDateTime, "AALARM");
	}
	if (Recurrance != -1 ) 
	{
		switch(note->Entries[Recurrance].Number) 
		{
			case CAL_REPEAT_DAILY	 : 
				if(RecurranceFreq!= -1)
					*Length+=sprintf(Buffer+(*Length), "RRULE:D%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
				else
					*Length+=sprintf(Buffer+(*Length), "RRULE:D1 #0%c%c",13,10);
				break;
			case CAL_REPEAT_WEEKLY	 : 
				if(RecurranceFreq!= -1)
					*Length+=sprintf(Buffer+(*Length), "RRULE:W%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
				else
					*Length+=sprintf(Buffer+(*Length), "RRULE:W1 #0%c%c",13,10);
				break;
			case CAL_REPEAT_MONTHLY	 : 
				if(RecurranceFreq!= -1)
					*Length+=sprintf(Buffer+(*Length), "RRULE:MD%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
				else
					*Length+=sprintf(Buffer+(*Length), "RRULE:MD1 #0%c%c",13,10);	
				break;
			case CAL_REPEAT_YEARLY : 
				if(RecurranceFreq!= -1)
					*Length+=sprintf(Buffer+(*Length), "RRULE:YD%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
				else
					*Length+=sprintf(Buffer+(*Length), "RRULE:YD1 #0%c%c",13,10);
				break;
		}
	}

//	SaveVCALText(Buffer, Length, note->Entries[Text].Text, "SUMMARY");
	if( Text!= -1 &&  UnicodeLength((unsigned char *)note->Entries[Text].Text) > 0)
	{
		unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
		CopyUnicodeString(utemp,note->Entries[Text].Text);
	//	UnicodeReplace(utemp,";","\\;");

		SESaveVCALText(Buffer, Length, utemp, "SUMMARY",":");
	}

	if (Location != -1)
	{
	//	SaveVCALText(Buffer, Length, note->Entries[Location].Text, "LOCATION");
		unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
		CopyUnicodeString(utemp,note->Entries[Location].Text);
//		UnicodeReplace(utemp,";","\\;");

		SESaveVCALText(Buffer, Length, utemp, "LOCATION",":");
	}
	if (Description != -1&&  UnicodeLength((unsigned char *)note->Entries[Description].Text) > 0) 
	{
		unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
		CopyUnicodeString(utemp,note->Entries[Description].Text);
		SESaveVCARDText(Buffer, Length, utemp, "DESCRIPTION",":");
	}
	*Length+=sprintf(Buffer+(*Length), "END:VEVENT%c%c",13,10);
	if (header) *Length+=sprintf(Buffer+(*Length), "END:VCALENDAR%c%c",13,10);

	return ERR_NONE;
}
GSM_Error S55_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version,int Timezone)
{
 	int 	Text, Time, Alarm, Phone, Recurrance, EndTime, Location,RecurranceFreq;
//	char 	buffer[2000];

	GSM_CalendarFindDefaultTextTimeAlarmPhoneRecurrance(note, &Text, &Time, &Alarm, &Phone, &Recurrance,&RecurranceFreq, &EndTime, &Location);

	if (header) {
		*Length+=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);
		*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c",13,10);
	}
	*Length+=sprintf(Buffer+(*Length), "BEGIN:VEVENT%c%c",13,10);

	//if (Version == Nokia_VCalendar) 
	{
		*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
		switch (note->Type) {
		case GSM_CAL_REMINDER:
			*Length+=sprintf(Buffer+(*Length), "Call%c%c",13,10);
			break;
		case GSM_CAL_MEMO:
			*Length+=sprintf(Buffer+(*Length), "Memo%c%c",13,10);
			break;
		case GSM_CAL_CALL:
			*Length+=sprintf(Buffer+(*Length), "Voice memo%c%c",13,10);
			break;
		case GSM_CAL_BIRTHDAY:
			*Length+=sprintf(Buffer+(*Length), "Birthday%c%c",13,10);
			break;
		case GSM_CAL_VACATION:
			*Length+=sprintf(Buffer+(*Length), "Holiday%c%c",13,10);
			break;
		case GSM_CAL_MEETING:
		default:
			*Length+=sprintf(Buffer+(*Length), "Meeting%c%c",13,10);
			break;
		}
		if( Text!= -1 &&  UnicodeLength((unsigned char *)note->Entries[Text].Text) > 0)
		{
			unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
			CopyUnicodeString(utemp,note->Entries[Text].Text);
//			UnicodeReplace(utemp,";","\\;");
			SaveVCALText(Buffer, Length, utemp, "SUMMARY");
		}
	
		if (Time == -1) return ERR_UNKNOWN;

		GSM_DateTime gsmDateTime;
		memcpy(&gsmDateTime,&note->Entries[Time].Date,sizeof(GSM_DateTime));

		if(Timezone>0)
			GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
		else
			GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);

		SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DTSTART");

		if (EndTime != -1) {
			memcpy(&gsmDateTime,&note->Entries[EndTime].Date,sizeof(GSM_DateTime));
			if(Timezone>0)
				GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
			else
				GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);
			SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DTEND");
		}

		if (Alarm != -1) {
			memcpy(&gsmDateTime,&note->Entries[Alarm].Date,sizeof(GSM_DateTime));
			if(Timezone>0)
				GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
			else
				GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);

			if (note->Entries[Alarm].EntryType == CAL_SILENT_ALARM_DATETIME) {
				SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DALARM");
			} else {
				SaveVCALDateTime(Buffer, Length, &gsmDateTime, "AALARM");
			}
		}

		/* Birthday is known to be recurranced */
		if (Recurrance != -1 ) 
		{
			switch(note->Entries[Recurrance].Number) 
			{
				case CAL_REPEAT_DAILY	 : 
					if(RecurranceFreq!= -1)
						*Length+=sprintf(Buffer+(*Length), "RRULE:D%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
					else
						*Length+=sprintf(Buffer+(*Length), "RRULE:D1 #0%c%c",13,10);
					break;
				case CAL_REPEAT_WEEKLY	 : 
					if(RecurranceFreq!= -1)
						*Length+=sprintf(Buffer+(*Length), "RRULE:W%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
					else
						*Length+=sprintf(Buffer+(*Length), "RRULE:W1 #0%c%c",13,10);
					break;
				case CAL_REPEAT_MONTHLY	 : 
					if(RecurranceFreq!= -1)
						*Length+=sprintf(Buffer+(*Length), "RRULE:MD%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
					else
						*Length+=sprintf(Buffer+(*Length), "RRULE:MD1 #0%c%c",13,10);	
					break;
				case CAL_REPEAT_YEARLY : 
					if(RecurranceFreq!= -1)
						*Length+=sprintf(Buffer+(*Length), "RRULE:YD%d #0%c%c",note->Entries[RecurranceFreq].Number,13,10);
					else
						*Length+=sprintf(Buffer+(*Length), "RRULE:YD1 #0%c%c",13,10);
					break;
			}
		}
	}	
	*Length+=sprintf(Buffer+(*Length), "END:VEVENT%c%c",13,10);
	if (header) *Length+=sprintf(Buffer+(*Length), "END:VCALENDAR%c%c",13,10);

	return ERR_NONE;
}
/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
GSM_Error NokiaS6_DecodeVCALENDAR(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, int* pTimezone)
{
	unsigned char 	Line[2000],Buff[2000],	Line2[2000];
    int             Level = 0,npos;
	char szCalType[500];
//	GSM_DateTime ToDoDueDate,ToDoCompletedDate;
//	ToDoDueDate.Year = ToDoCompletedDate.Year = 0;
	bool bToDoCompleted = false;
//	char  str[30];
//	time_t time;

	Calendar->EntriesNum 	= 0;
	ToDo->EntriesNum 	= 0;

	while (1) {
		MyGetLine(Buffer, Pos, Line, strlen((char *)Buffer));
		if (strlen((char *)Line) == 0) break;
 		//Modify for SE K750i
		//=E6=88=91=E9=96=80=E6=98=A=..F=E4=B8=80=E5=AE=B6=E4=BA=BA..
		npos = *Pos;
		memcpy(Line2,Line,strlen((char*)Line)+1);
		while(Line2[strlen((char*)Line2)-1] == '=')
		{
			if(Buffer[npos+2] == 0x0d && Buffer[npos+3] == 0x0A)
			{
				if(Line[strlen((char*)Line)-1] == '=')
					Line[strlen((char*)Line)-1]= '\0';
				break;
			}
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
			if(Line2[0] == '=')
			{
				strcat((char *)Line,(char *)Line2+1);
			}
			else
			{
				Line[strlen((char*)Line)-1]= '\0';
				strcat((char *)Line,(char *)Line2);
			}
		}
		MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
		while(Line2[0] == 0x20 && Line2[1] == 0x20)
		{
			strcat((char *)Line,(char *)Line2);
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
		}
/*		npos = *Pos;//peggy
        MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
  		//peggy +
		while(Line2[0] == '=')
		{
			strcat((char *)Line,(char *)Line2+1);

			if(Line2[strlen((char *)Line2) -1] == '=')
				MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
			else 
				break;
		}
		//peggy +*/
		switch (Level) {
		case 0:
			if (strnicmp((char *)Line,"TZ:",3) == 0)
			{
				char time[3];
				int nTimeZone = 0;
				time[3]='\0';
				memcpy(time,Line+4,2);
				nTimeZone = atoi(time);
				if(Line[3] == '-')
					*pTimezone = nTimeZone * (-4);
				else
					*pTimezone = nTimeZone*4;
			}

			if (strstr((char *)Line,"BEGIN:VEVENT")) {
				Calendar->Type 	= GSM_CAL_MEMO;
				Level 		= 1;
			}
			if (strstr((char *)Line,"BEGIN:VTODO")) {
				ToDo->Priority 	= 2;//GSM_Priority_Low;
				Level 		= 2;
			}
			break;
		case 1: /* Calendar note */
			if (strstr((char *)Line,"END:VEVENT"))
			{
				if (Calendar->EntriesNum == 0) return ERR_EMPTY;
				if(Calendar->Type == GSM_CAL_MEMO)
				{
					for(int i = 0; i<Calendar->EntriesNum;i++)
					{
						if(Calendar->Entries[i].EntryType == CAL_END_DATETIME)
						{
							if(Calendar->Entries[i].Date.Hour ==0 && Calendar->Entries[i].Date.Minute == 0 && Calendar->Entries[i].Date.Second ==0)
							{
								GetTimeDifference(1, &(Calendar->Entries[i].Date), false, 1);
							}
							break;
						}
					}
				}

				return ERR_NONE;
			}
			if (ReadVCALText(Line, "X-EPOCAGENDAENTRYTYPE", Buff) )
			{

				DecodeUnicode(Buff,(unsigned char *)szCalType);
				if (strstr(szCalType,"EVENT")) 	Calendar->Type = GSM_CAL_MEMO;
				if (strstr(szCalType,"ANNIVERSARY")) 	Calendar->Type = GSM_CAL_BIRTHDAY;
				if (strstr(szCalType,"APPOINTMENT")) 	Calendar->Type = GSM_CAL_MEETING;

			}
			
 	////////	
			char *pTempFrequency;
			if ((pTempFrequency = strstr((char *)Line,"RRULE:D"))) 
			{
				int nFrequency = atoi(pTempFrequency+7);

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				if(nFrequency == 7)
				{
					Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY ;
					nFrequency = 1;
				}
				else
					Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_DAILY ;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = nFrequency ;

				Calendar->EntriesNum++;
				if(strlen((char *)Line) >=24)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &Calendar->Entries[Calendar->EntriesNum].Date,*pTimezone);
					Calendar->EntriesNum++;

				}

			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:W"))) 
			{
		//	if ((strstr((char *)Line,"RRULE:W1")) || (strstr((char *)Line,"RRULE:D7"))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY;//7*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+7) ;
				Calendar->EntriesNum++;

				int DayOfWeekMask = 0;
				if(strstr((char *)Line,"SU"))
					DayOfWeekMask |=1;
				if(strstr((char *)Line,"MO"))
					DayOfWeekMask |=2;
				if(strstr((char *)Line,"TU"))
					DayOfWeekMask |=4;
				if(strstr((char *)Line,"WE"))
					DayOfWeekMask |=8;
				if(strstr((char *)Line,"TH"))
					DayOfWeekMask |=16;
				if(strstr((char *)Line,"FR"))
					DayOfWeekMask |=32;
				if(strstr((char *)Line,"SA"))
					DayOfWeekMask |=64;
				if(DayOfWeekMask>0)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_DAYOFWEEK;
					Calendar->Entries[Calendar->EntriesNum].Number    = DayOfWeekMask  ;
					Calendar->EntriesNum++;
				}

				if(strlen((char *)Line) >=24)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &Calendar->Entries[Calendar->EntriesNum].Date,*pTimezone);
					Calendar->EntriesNum++;

				}
			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:MD"))) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_MONTHLY;//30*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+8)  ;
				Calendar->EntriesNum++;
				if(strlen((char *)Line) >=24)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &Calendar->Entries[Calendar->EntriesNum].Date,*pTimezone);
					Calendar->EntriesNum++;

				}
			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:MP"))) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_MONTH_WEEKLY;//30*24;
				Calendar->EntriesNum++;


				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+8)  ;
				Calendar->EntriesNum++;


				if(strlen((char *)Line) >=24)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &Calendar->Entries[Calendar->EntriesNum].Date,*pTimezone);

					Calendar->EntriesNum++;

				}
			}

			if ((pTempFrequency = strstr((char *)Line,"RRULE:YM"))) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_YEARLY;//365*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+8) ;
				Calendar->EntriesNum++;
				if(strlen((char *)Line) >=24)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &Calendar->Entries[Calendar->EntriesNum].Date,*pTimezone);
					Calendar->EntriesNum++;

				}
			}


			if ((ReadVCALText(Line, "SUMMARY", Buff)) ) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
			}
			if ((ReadVCALText(Line, "DESCRIPTION", Buff))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_DESCRIPTION;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
			}

			if (ReadVCALText(Line, "LOCATION", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LOCATION;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DTSTART", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_START_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,*pTimezone);
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DTEND", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_END_DATETIME;
				ReadVCALDateTime((char *)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,*pTimezone);
	
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DALARM", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,*pTimezone);
			
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "AALARM", Buff) ||ReadVCALText(Line, "AALARM;TYPE=X-EPOCSOUND", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,*pTimezone);
			
				Calendar->EntriesNum++;
			}
			break;
		case 2: /* ToDo note */
			if (strstr((char *)Line,"END:VTODO")) 
			{
				if (ToDo->EntriesNum == 0) return ERR_EMPTY;
			/*	if(bToDoCompleted && ToDoCompletedDate.Year !=0)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
					ToDo->Entries[ToDo->EntriesNum].Date = ToDoCompletedDate;
					ToDo->EntriesNum++;
				}
				else if(bToDoCompleted == false && ToDoDueDate.Year !=0)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
					ToDo->Entries[ToDo->EntriesNum].Date = ToDoDueDate;
					ToDo->EntriesNum++;
				}*/

				return ERR_NONE;
			}
			if (ReadVCALText(Line, "DUE", Buff))
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,*pTimezone);
				ToDo->EntriesNum++;
		//		ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDoDueDate);
			}
			if (ReadVCALText(Line, "COMPLETED", Buff))
			{
	//			ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDoCompletedDate);
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETEDDATE;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,*pTimezone);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "DALARM", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_SILENT_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,*pTimezone);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "AALARM", Buff)||ReadVCALText(Line, "AALARM;TYPE=X-EPOCSOUND", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_ALARM_DATETIME;
				ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,*pTimezone);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "SUMMARY", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_TEXT;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				UnicodeReplace(ToDo->Entries[ToDo->EntriesNum].Text,"\\;",";");
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "PRIORITY", Buff)) {
			/*	if (ToDoVer == SonyEricsson_VToDo) {
					ToDo->Priority = GSM_Priority_Low;
					if (atoi((char *)DecodeUnicodeString(Buff))==2) ToDo->Priority = GSM_Priority_Medium;
					if (atoi((char *)DecodeUnicodeString(Buff))==1) ToDo->Priority = GSM_Priority_High;
					dbgprintf("atoi is %i %s\n",atoi((char *)DecodeUnicodeString(Buff)),DecodeUnicodeString(Buff));
				} else if (ToDoVer == Nokia_VToDo)*/ 
			/*	{
					ToDo->Priority = GSM_Priority_Medium;
					if (atoi((char *)DecodeUnicodeString(Buff))==3) ToDo->Priority = GSM_Priority_Low;
					if (atoi((char *)DecodeUnicodeString(Buff))==2) ToDo->Priority = GSM_Priority_Medium;
					if (atoi((char *)DecodeUnicodeString(Buff))==1) ToDo->Priority = GSM_Priority_High;
				}*/
				ToDo->Priority = atoi((char *)DecodeUnicodeString(Buff));
			}
			if (strstr((char *)Line,"STATUS:COMPLETED")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 1;
				ToDo->EntriesNum++;
				bToDoCompleted = true;
			}
			if (strstr((char *)Line,"STATUS:NEEDS ACTION")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 0;
				ToDo->EntriesNum++;
				bToDoCompleted = false;
			}
//Repeat  ++++++++++
			if ((pTempFrequency = strstr((char *)Line,"RRULE:D"))) 
			{
				int nFrequency = atoi(pTempFrequency+7);

				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCE;
				if(nFrequency == 7)
				{
					ToDo->Entries[ToDo->EntriesNum].Number    = CAL_REPEAT_WEEKLY ;
					nFrequency = 1;
				}
				else
					ToDo->Entries[ToDo->EntriesNum].Number    = CAL_REPEAT_DAILY ;
				ToDo->EntriesNum++;

				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCEFREQUENCY;
				ToDo->Entries[ToDo->EntriesNum].Number    = nFrequency ;

				ToDo->EntriesNum++;
				if(strlen((char *)Line) >=24)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &ToDo->Entries[ToDo->EntriesNum].Date,*pTimezone);

					if((*pTimezone)>0)
						GetTimeDifference((*pTimezone)*15, &ToDo->Entries[ToDo->EntriesNum].Date, true, 60);
					else
						GetTimeDifference(-(*pTimezone)*15, &ToDo->Entries[ToDo->EntriesNum].Date, false, 60);
					ToDo->EntriesNum++;

				}

			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:W"))) 
			{
		//	if ((strstr((char *)Line,"RRULE:W1")) || (strstr((char *)Line,"RRULE:D7"))) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCE;
				ToDo->Entries[ToDo->EntriesNum].Number    = CAL_REPEAT_WEEKLY;//7*24;
				ToDo->EntriesNum++;

				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCEFREQUENCY;
				ToDo->Entries[ToDo->EntriesNum].Number    = atoi(pTempFrequency+7) ;
				ToDo->EntriesNum++;

				int DayOfWeekMask = 0;
				if(strstr((char *)Line,"SU"))
					DayOfWeekMask |=1;
				if(strstr((char *)Line,"MO"))
					DayOfWeekMask |=2;
				if(strstr((char *)Line,"TU"))
					DayOfWeekMask |=4;
				if(strstr((char *)Line,"WE"))
					DayOfWeekMask |=8;
				if(strstr((char *)Line,"TH"))
					DayOfWeekMask |=16;
				if(strstr((char *)Line,"FR"))
					DayOfWeekMask |=32;
				if(strstr((char *)Line,"SA"))
					DayOfWeekMask |=64;
				if(DayOfWeekMask>0)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_REPEAT_DAYOFWEEK;
					ToDo->Entries[ToDo->EntriesNum].Number    = DayOfWeekMask  ;
					ToDo->EntriesNum++;
				}

				if(strlen((char *)Line) >=24)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &ToDo->Entries[ToDo->EntriesNum].Date,*pTimezone);

					ToDo->EntriesNum++;

				}
			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:MD"))) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCE;
				ToDo->Entries[ToDo->EntriesNum].Number    = CAL_REPEAT_MONTHLY;//30*24;
				ToDo->EntriesNum++;

				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCEFREQUENCY;
				ToDo->Entries[ToDo->EntriesNum].Number    = atoi(pTempFrequency+8)  ;
				ToDo->EntriesNum++;
				if(strlen((char *)Line) >=24)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &ToDo->Entries[ToDo->EntriesNum].Date,*pTimezone);
					ToDo->EntriesNum++;

				}
			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:MP"))) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCE;
				ToDo->Entries[ToDo->EntriesNum].Number    = CAL_REPEAT_MONTH_WEEKLY;//30*24;
				ToDo->EntriesNum++;


				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCEFREQUENCY;
				ToDo->Entries[ToDo->EntriesNum].Number    = atoi(pTempFrequency+8)  ;
				ToDo->EntriesNum++;


				if(strlen((char *)Line) >=24)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &ToDo->Entries[ToDo->EntriesNum].Date,*pTimezone);

					ToDo->EntriesNum++;

				}
			}

			if ((pTempFrequency = strstr((char *)Line,"RRULE:YM"))) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCE;
				ToDo->Entries[ToDo->EntriesNum].Number    = CAL_REPEAT_YEARLY;//365*24;
				ToDo->EntriesNum++;

				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCEFREQUENCY;
				ToDo->Entries[ToDo->EntriesNum].Number    = atoi(pTempFrequency+8) ;
				ToDo->EntriesNum++;
				if(strlen((char *)Line) >=24)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &ToDo->Entries[ToDo->EntriesNum].Date,*pTimezone);
					ToDo->EntriesNum++;

				}
			}

//Repeat  ---------
			if ((ReadVCALText(Line, "DESCRIPTION", Buff))) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_DESCRIPTION;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				UnicodeReplace(ToDo->Entries[ToDo->EntriesNum].Text,"\\;",";");
				ToDo->EntriesNum++;
			}


			break;
		}
	}

	if (Calendar->EntriesNum == 0 && ToDo->EntriesNum == 0) return ERR_EMPTY;


	return ERR_NONE;
}
GSM_Error NokiaS6_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version,int Timezone)
{
 	int 	Text, Time, Alarm, Phone, Recurrance, RecurranceFreq, EndTime, Location,RepeatEndDate,Description,DayofweekMask;
	char 	buffer[2000];

	GSM_CalendarFindEntryIndex(note, &Text, &Time, &Alarm, &Phone, &Recurrance,&RecurranceFreq, &EndTime, &Location,&RepeatEndDate,&Description,&DayofweekMask);

	if (header) {
		*Length+=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);
		*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c",13,10);
	}
	*Length+=sprintf(Buffer+(*Length), "BEGIN:VEVENT%c%c",13,10);

	if (Version == Nokia_VCalendar) {
		*Length+=sprintf(Buffer+(*Length), "X-EPOCAGENDAENTRYTYPE:");
		switch (note->Type) {
		case GSM_CAL_MEMO:
			*Length+=sprintf(Buffer+(*Length), "EVENT%c%c",13,10);
			break;
		case GSM_CAL_BIRTHDAY:
			*Length+=sprintf(Buffer+(*Length), "ANNIVERSARY%c%c",13,10);
			break;
		case GSM_CAL_MEETING:
		default:
			*Length+=sprintf(Buffer+(*Length), "APPOINTMENT%c%c",13,10);
			break;
		}
		if (note->Type == GSM_CAL_CALL) {
			buffer[0] = 0;
			buffer[1] = 0;
		 	if (Phone != -1) CopyUnicodeString((unsigned char *)buffer,note->Entries[Phone].Text);
			if (Text != -1)  {
				if (Phone != -1) EncodeUnicode((unsigned char *)buffer+UnicodeLength((unsigned char *)buffer)*2,(unsigned char *)" ",1);
				CopyUnicodeString((unsigned char *)buffer+UnicodeLength((unsigned char *)buffer)*2,note->Entries[Text].Text);
			}
			if(UnicodeLength((unsigned char *)buffer) > 0)
			{
			//	UnicodeReplace((unsigned char *)buffer,";","\\;");
				SESaveVCARDText(Buffer, Length, (unsigned char *)buffer, "SUMMARY",":");
			}
		} else {
			if( Text!= -1 &&  UnicodeLength((unsigned char *)note->Entries[Text].Text) > 0)
			{
				unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
				CopyUnicodeString(utemp,note->Entries[Text].Text);
//				UnicodeReplace(utemp,";","\\;");
				SESaveVCARDText(Buffer, Length,utemp, "SUMMARY",":");
			}
		}
		if (note->Type == GSM_CAL_MEETING && Location != -1) 
		{
			unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
			CopyUnicodeString(utemp,note->Entries[Location].Text);
//			UnicodeReplace(utemp,";","\\;");
			SESaveVCARDText(Buffer, Length, utemp, "LOCATION",":");
		}
		if (Description != -1) {
			unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
			CopyUnicodeString(utemp,note->Entries[Description].Text);
//			UnicodeReplace(utemp,";","\\;");
			SESaveVCARDText(Buffer, Length, utemp, "DESCRIPTION",":");
		}
	
		if (Time == -1) return ERR_UNKNOWN;
		if (note->Type == GSM_CAL_MEMO)
		{
			note->Entries[Time].Date.Hour = note->Entries[Time].Date.Minute = note->Entries[Time].Date.Second =0;
		}
	//	SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[Time].Date, "DTSTART");
		if(Timezone == 0)
			SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[Time].Date, "DTSTART");
		else
		{
			GSM_DateTime gsmDateTime;
			memcpy(&gsmDateTime,&note->Entries[Time].Date,sizeof(GSM_DateTime));

			if(Timezone>0)
				GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
			else 
				GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);
			SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DTSTART");
		}

		if (EndTime != -1)
		{
			if (note->Type == GSM_CAL_MEMO)
			{
				if(note->Entries[EndTime].Date.Hour!= 0 || note->Entries[EndTime].Date.Minute !=0 || note->Entries[EndTime].Date.Minute !=0 )
				{
					GetTimeDifference(24*60, &note->Entries[EndTime].Date, true, 60);
					note->Entries[EndTime].Date.Hour = note->Entries[EndTime].Date.Minute = note->Entries[EndTime].Date.Second =0;
				}

			}
	//		SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[EndTime].Date, "DTEND");
			if(Timezone == 0)
				SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[EndTime].Date, "DTEND");
			else
			{
				GSM_DateTime gsmDateTime;
				memcpy(&gsmDateTime,&note->Entries[EndTime].Date,sizeof(GSM_DateTime));

				if(Timezone>0)
					GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
				else 
					GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);
				SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DTEND");
			}

		}

		if (Alarm != -1) {
		//	if (note->Entries[Alarm].EntryType == CAL_SILENT_ALARM_DATETIME) {
		//		SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[Alarm].Date, "DALARM");
		//	} else {
			//	SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[Alarm].Date, "AALARM");
		//	}

			if(Timezone == 0)
				SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[Alarm].Date, "AALARM");
			else
			{
				GSM_DateTime gsmDateTime;
				memcpy(&gsmDateTime,&note->Entries[Alarm].Date,sizeof(GSM_DateTime));

				if(Timezone>0)
					GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
				else 
					GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);
				SaveVCALDateTime(Buffer, Length, &gsmDateTime, "AALARM");
			}

		}

		/* Birthday is known to be recurranced */
		if (Recurrance != -1 /*&& note->Type != GSM_CAL_BIRTHDAY*/) 
		{
			int nFreq = 1;
			if(RecurranceFreq!= -1) nFreq = note->Entries[RecurranceFreq].Number;
			switch(note->Entries[Recurrance].Number)
			{
				case CAL_REPEAT_DAILY	 : 
					*Length+=sprintf(Buffer+(*Length), "RRULE:D%d #0",nFreq);
					break;
				case CAL_REPEAT_WEEKLY	 : 
					if(DayofweekMask == -1)
						*Length+=sprintf(Buffer+(*Length), "RRULE:W%d #0",nFreq);
					else 
					{
						char szDay[MAX_PATH];
						int nMask = note->Entries[DayofweekMask].Number;
						szDay[0] = '\0';
						if(nMask & 1) strcat(szDay ,"SU ");
						if(nMask & 2) strcat(szDay ,"MO ");
						if(nMask & 4) strcat(szDay ,"TU ");
						if(nMask & 8) strcat(szDay ,"WE ");
						if(nMask & 16) strcat(szDay ,"TH ");
						if(nMask & 32) strcat(szDay ,"FR ");
						if(nMask & 64) strcat(szDay ,"SA ");
						*Length+=sprintf(Buffer+(*Length), "RRULE:W%d %s#0",nFreq,szDay);

					}
					break;
				case CAL_REPEAT_MONTHLY	 : 
					*Length+=sprintf(Buffer+(*Length), "RRULE:MD%d #0",nFreq);
					break;
				case CAL_REPEAT_MONTH_WEEKLY	 : 
					{
						UINT whichWeek,  whichDay;
						GetwhichWeekDay(note->Entries[Time].Date,whichWeek,whichDay);
						char szday[MAX_PATH];
						switch(whichDay)
						{
						case 0:
							wsprintf(szday,"SU");
							break;
						case 1:
							wsprintf(szday,"MO");
							break;
						case 2:
							wsprintf(szday,"TU");
							break;
						case 3:
							wsprintf(szday,"WE");
							break;
						case 4:
							wsprintf(szday,"TH");
							break;
						case 5:
							wsprintf(szday,"FR");
							break;
						case 6:
							wsprintf(szday,"SA");
							break;
						}

						if(whichWeek == 5) 
							*Length+=sprintf(Buffer+(*Length), "RRULE:MP%d %d- %s #0",nFreq,1,szday);
						else
							*Length+=sprintf(Buffer+(*Length), "RRULE:MP%d %d+ %s #0",nFreq,whichWeek,szday);
					}
					break;
				case CAL_REPEAT_YEARLY : 
					*Length+=sprintf(Buffer+(*Length), "RRULE:YM%d #0",nFreq);
					break;
			}
			if(RepeatEndDate!=-1)
			{
				*Length+=sprintf(Buffer+(*Length), " %04d%02d%02dT%02d%02d%02d",
					note->Entries[RepeatEndDate].Date.Year,note->Entries[RepeatEndDate].Date.Month,note->Entries[RepeatEndDate].Date.Day,
					note->Entries[RepeatEndDate].Date.Hour,note->Entries[RepeatEndDate].Date.Minute,note->Entries[RepeatEndDate].Date.Second);
			}
			*Length+=sprintf(Buffer+(*Length), "%c%c",13,10);

		}
	}	
	*Length+=sprintf(Buffer+(*Length), "END:VEVENT%c%c",13,10);
	if (header) *Length+=sprintf(Buffer+(*Length), "END:VCALENDAR%c%c",13,10);

	return ERR_NONE;
}

GSM_Error Samsung_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version)
{
 	int 	Text, Time, Alarm, Phone, Recurrance, RecurranceFreq, EndTime, Location,RepeatEndDate,Description,DayofweekMask;
	char 	buffer[2000];

	GSM_CalendarFindEntryIndex(note, &Text, &Time, &Alarm, &Phone, &Recurrance,&RecurranceFreq, &EndTime, &Location,&RepeatEndDate,&Description,&DayofweekMask);

	if (header) {
		*Length+=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);
		*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c",13,10);
	}
	*Length+=sprintf(Buffer+(*Length), "BEGIN:VEVENT%c%c",13,10);

	if (Version == Nokia_VCalendar) {
		*Length+=sprintf(Buffer+(*Length), "X-EPOCAGENDAENTRYTYPE:");
		switch (note->Type) {
		case GSM_CAL_MEMO:
			*Length+=sprintf(Buffer+(*Length), "EVENT%c%c",13,10);
			break;
		case GSM_CAL_BIRTHDAY:
			*Length+=sprintf(Buffer+(*Length), "ANNIVERSARY%c%c",13,10);
			break;
		case GSM_CAL_MEETING:
		default:
			*Length+=sprintf(Buffer+(*Length), "APPOINTMENT%c%c",13,10);
			break;
		}
		if (note->Type == GSM_CAL_CALL) {
			buffer[0] = 0;
			buffer[1] = 0;
		 	if (Phone != -1) CopyUnicodeString((unsigned char *)buffer,note->Entries[Phone].Text);
			if (Text != -1)  {
				if (Phone != -1) EncodeUnicode((unsigned char *)buffer+UnicodeLength((unsigned char *)buffer)*2,(unsigned char *)" ",1);
				CopyUnicodeString((unsigned char *)buffer+UnicodeLength((unsigned char *)buffer)*2,note->Entries[Text].Text);
			}
			if(UnicodeLength((unsigned char *)buffer) > 0)
			{
				UnicodeReplace((unsigned char *)buffer,";","\\;");
				SESaveVCARDText(Buffer, Length, (unsigned char *)buffer, "SUMMARY",":");
			}
		} else {
			if( Text!= -1 &&  UnicodeLength((unsigned char *)note->Entries[Text].Text) > 0)
			{
				unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
				CopyUnicodeString(utemp,note->Entries[Text].Text);
				UnicodeReplace(utemp,";","\\;");
				SESaveVCARDText(Buffer, Length,utemp, "SUMMARY",":");
			}
		}
		if (note->Type == GSM_CAL_MEETING && Location != -1) 
		{
			unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
			CopyUnicodeString(utemp,note->Entries[Location].Text);
			UnicodeReplace(utemp,";","\\;");
			SESaveVCARDText(Buffer, Length, utemp, "LOCATION",":");
		}
		if (Description != -1) {
			unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
			CopyUnicodeString(utemp,note->Entries[Description].Text);
			UnicodeReplace(utemp,";","\\;");
			SESaveVCARDText(Buffer, Length, utemp, "DESCRIPTION",":");
		}
	
		if (Time == -1) return ERR_UNKNOWN;
		if (note->Type == GSM_CAL_MEMO)
		{
			note->Entries[Time].Date.Hour = note->Entries[Time].Date.Minute = note->Entries[Time].Date.Second =0;
		}
		SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[Time].Date, "DTSTART");

		if (EndTime != -1)
		{
			if (note->Type == GSM_CAL_MEMO)
			{
				if(note->Entries[EndTime].Date.Hour!= 0 || note->Entries[EndTime].Date.Minute !=0 || note->Entries[EndTime].Date.Minute !=0 )
				{
					GetTimeDifference(24*60, &note->Entries[EndTime].Date, true, 60);
					note->Entries[EndTime].Date.Hour = note->Entries[EndTime].Date.Minute = note->Entries[EndTime].Date.Second =0;
				}

			}
			SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[EndTime].Date, "DTEND");
		}

		if (Alarm != -1) {
		//	if (note->Entries[Alarm].EntryType == CAL_SILENT_ALARM_DATETIME) {
		//		SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[Alarm].Date, "DALARM");
		//	} else {
				SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[Alarm].Date, "AALARM");
		//	}
		}

		/* Birthday is known to be recurranced */
		if (Recurrance != -1 /*&& note->Type != GSM_CAL_BIRTHDAY*/) 
		{
			int nFreq = 1;
			if(RecurranceFreq!= -1) nFreq = note->Entries[RecurranceFreq].Number;
			switch(note->Entries[Recurrance].Number)
			{
				case CAL_REPEAT_DAILY	 : 
					*Length+=sprintf(Buffer+(*Length), "RRULE:D1");
					break;
				case CAL_REPEAT_WEEKLY	 : 
					*Length+=sprintf(Buffer+(*Length), "RRULE:W1");
					break;
				case CAL_REPEAT_MONTHLY	 : 
					*Length+=sprintf(Buffer+(*Length), "RRULE:MD1");
					break;
			/*	case CAL_REPEAT_MONTH_WEEKLY	 : 
					{
						UINT whichWeek,  whichDay;
						GetwhichWeekDay(note->Entries[Time].Date,whichWeek,whichDay);
						char szday[MAX_PATH];
						switch(whichDay)
						{
						case 0:
							wsprintf(szday,"SU");
							break;
						case 1:
							wsprintf(szday,"MO");
							break;
						case 2:
							wsprintf(szday,"TU");
							break;
						case 3:
							wsprintf(szday,"WE");
							break;
						case 4:
							wsprintf(szday,"TH");
							break;
						case 5:
							wsprintf(szday,"FR");
							break;
						case 6:
							wsprintf(szday,"SA");
							break;
						}

						if(whichWeek == 5) 
							*Length+=sprintf(Buffer+(*Length), "RRULE:MP%d %d- %s #0",nFreq,1,szday);
						else
							*Length+=sprintf(Buffer+(*Length), "RRULE:MP%d %d+ %s #0",nFreq,whichWeek,szday);
					}
					break;
				case CAL_REPEAT_YEARLY : 
					*Length+=sprintf(Buffer+(*Length), "RRULE:YM%d #0",nFreq);
					break;*/
			}
			if(RepeatEndDate!=-1)
			{
				*Length+=sprintf(Buffer+(*Length), " %04d%02d%02dT%02d%02d%02d",
					note->Entries[RepeatEndDate].Date.Year,note->Entries[RepeatEndDate].Date.Month,note->Entries[RepeatEndDate].Date.Day,
					note->Entries[RepeatEndDate].Date.Hour,note->Entries[RepeatEndDate].Date.Minute,note->Entries[RepeatEndDate].Date.Second);
			}
			*Length+=sprintf(Buffer+(*Length), "%c%c",13,10);

		}
	}	
	*Length+=sprintf(Buffer+(*Length), "END:VEVENT%c%c",13,10);
	if (header) *Length+=sprintf(Buffer+(*Length), "END:VCALENDAR%c%c",13,10);

	return ERR_NONE;
}

GSM_Error SAGEM_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header)
{
 	int 	Text, Time, Alarm, Phone, Recurrance, RecurranceFreq, EndTime, Location,RepeatEndDate,Description,DayofweekMask;
//	char 	buffer[2000];

	GSM_CalendarFindEntryIndex(note, &Text, &Time, &Alarm, &Phone, &Recurrance,&RecurranceFreq, &EndTime, &Location,&RepeatEndDate,&Description,&DayofweekMask);

	if (header) {
		*Length+=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);
		*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c",13,10);
	}
	*Length+=sprintf(Buffer+(*Length), "BEGIN:VEVENT%c%c",13,10);

	{
		*Length+=sprintf(Buffer+(*Length), "CATEGORIES;CHARSET=US-ASCII:");
		switch (note->Type) {
		case GSM_CAL_REMINDER:
			*Length+=sprintf(Buffer+(*Length), "APPOINTMENT%c%c",13,10);
			break;
		case GSM_CAL_MEETING:
			*Length+=sprintf(Buffer+(*Length), "MEETING%c%c",13,10);
			break;
		case GSM_CAL_BIRTHDAY:
			*Length+=sprintf(Buffer+(*Length), "BIRTHDAY%c%c",13,10);
			break;
		case GSM_CAL_MEMO:
			*Length+=sprintf(Buffer+(*Length), "EVENT%c%c",13,10);
			break;
		case GSM_CAL_VACATION:
			*Length+=sprintf(Buffer+(*Length), "HOLIDAYS%c%c",13,10);
			break;
		case GSM_CAL_TRAVEL:
			*Length+=sprintf(Buffer+(*Length), "TRAVEL%c%c",13,10);
			break;
		default:
			*Length+=sprintf(Buffer+(*Length), "MEETING%c%c",13,10);
			break;
		}
		if (Text != -1) {
			unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
			CopyUnicodeString(utemp,note->Entries[Text].Text);
	//		UnicodeReplace(utemp,";","\\;");

			SESaveVCARDText(Buffer, Length, utemp, "SUMMARY",":");
		}
		if (Location != -1) {
			unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
			CopyUnicodeString(utemp,note->Entries[Location].Text);
		//	UnicodeReplace(utemp,";","\\;");
			SESaveVCARDText(Buffer, Length, utemp, "DESCRIPTION",":");
		}
		if (Text == -1 && Location == -1) 
			*Length+=sprintf(Buffer+(*Length), "SUMMARY:  %c%c",13,10);
	
		if (Time == -1) return ERR_UNKNOWN;
		SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[Time].Date, "DTSTART");

		if (EndTime != -1)
			SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[EndTime].Date, "DTEND");

		if (Alarm != -1) 
			SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[Alarm].Date, "AALARM");

		if (Recurrance != -1 /*&& note->Type != GSM_CAL_BIRTHDAY*/) 
		{
			int nFreq = 1;
			if(RecurranceFreq!= -1) nFreq = note->Entries[RecurranceFreq].Number;
			switch(note->Entries[Recurrance].Number)
			{
				case CAL_REPEAT_DAILY	 : 
					*Length+=sprintf(Buffer+(*Length), "RRULE:D%d SU MO TU WE TH FR SA #0",nFreq);
					break;
			/*	case CAL_REPEAT_YEAR_WEEKDAYS	 : 
					*Length+=sprintf(Buffer+(*Length), "RRULE:D%d MO TU WE TH FR #0",nFreq);
					break;
				case CAL_REPEAT_YEAR_WEEKEND	 : 
					*Length+=sprintf(Buffer+(*Length), "RRULE:D%d SU SA #0",nFreq);
					break;*/
				case CAL_REPEAT_WEEKLY	 : 
		//			*Length+=sprintf(Buffer+(*Length), "RRULE:W%d #0",nFreq);
					if(DayofweekMask == -1)
						*Length+=sprintf(Buffer+(*Length), "RRULE:W%d #0",nFreq);
					else 
					{
						char szDay[MAX_PATH];
						int nMask = note->Entries[DayofweekMask].Number;
						szDay[0] = '\0';
						if(nMask & 1) strcat(szDay ,"SU ");
						if(nMask & 2) strcat(szDay ,"MO ");
						if(nMask & 4) strcat(szDay ,"TU ");
						if(nMask & 8) strcat(szDay ,"WE ");
						if(nMask & 16) strcat(szDay ,"TH ");
						if(nMask & 32) strcat(szDay ,"FR ");
						if(nMask & 64) strcat(szDay ,"SA ");
						if(strlen(szDay) > 3)
							*Length+=sprintf(Buffer+(*Length), "RRULE:D%d %s#0",nFreq,szDay);
						else 
							*Length+=sprintf(Buffer+(*Length), "RRULE:W%d #0",nFreq);

					}

					break;
				case CAL_REPEAT_MONTHLY	 : 
					*Length+=sprintf(Buffer+(*Length), "RRULE:MD%d #0",nFreq);
					break;
				case CAL_REPEAT_MONTH_WEEKLY	 : 
					{
						UINT whichWeek,  whichDay;
						GetwhichWeekDay(note->Entries[Time].Date,whichWeek,whichDay);
						char szday[MAX_PATH];
						switch(whichDay)
						{
						case 0:
							wsprintf(szday,"SU");
							break;
						case 1:
							wsprintf(szday,"MO");
							break;
						case 2:
							wsprintf(szday,"TU");
							break;
						case 3:
							wsprintf(szday,"WE");
							break;
						case 4:
							wsprintf(szday,"TH");
							break;
						case 5:
							wsprintf(szday,"FR");
							break;
						case 6:
							wsprintf(szday,"SA");
							break;
						}

						if(whichWeek == 5) 
							*Length+=sprintf(Buffer+(*Length), "RRULE:MP%d %d- %s #0",nFreq,1,szday);
						else
							*Length+=sprintf(Buffer+(*Length), "RRULE:MP%d %d+ %s #0",nFreq,whichWeek,szday);
					}
					break;
				case CAL_REPEAT_YEARLY : 
					*Length+=sprintf(Buffer+(*Length), "RRULE:YM%d #0",nFreq);
					break;
			}
			if(RepeatEndDate!=-1)
			{
				*Length+=sprintf(Buffer+(*Length), " %04d%02d%02dT%02d%02d%02d",
					note->Entries[RepeatEndDate].Date.Year,note->Entries[RepeatEndDate].Date.Month,note->Entries[RepeatEndDate].Date.Day,
					note->Entries[RepeatEndDate].Date.Hour,note->Entries[RepeatEndDate].Date.Minute,note->Entries[RepeatEndDate].Date.Second);
			}
			*Length+=sprintf(Buffer+(*Length), "%c%c",13,10);

		}
	}	
	*Length+=sprintf(Buffer+(*Length), "END:VEVENT%c%c",13,10);
	if (header) *Length+=sprintf(Buffer+(*Length), "END:VCALENDAR%c%c",13,10);

	return ERR_NONE;
}


// 9@9u
GSM_Error SF_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, int Timezone)
{
 	int 	Text, Time, Alarm, Phone, Recurrance, RecurranceFreq, EndTime, Location,RepeatEndDate,Description,DayofweekMask, Priority;
//	char 	buffer[2000];
    int level =1 ; // VEVENT
//	int testentrynum = 0;

//testentrynum = note->EntriesNum;

	SF_CalendarFindEntryIndex(note, &Text, &Time, &Alarm, &Phone, &Recurrance,&RecurranceFreq, &EndTime, &Location,&RepeatEndDate,&Description,&DayofweekMask, &Priority);


//testentrynum = note->EntriesNum;

	if (header) {
	//	*Length+=sprintf(Buffer, "%c%cBEGIN:VCALENDAR%c%c",13,10,13,10);  // 0x0D , 0x0A
	//	*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c",13,10);
		*Length+=sprintf(Buffer, "BEGIN:VCALENDAR%c%c%c%c",0x2C,0x2C,0x2C,0x2C);  // ',' = 0x2C
		*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
	}
	if ( ( note->Type == GSM_CAL_MEETING ) || ( note->Type == GSM_CAL_REMINDER ) || ( note->Type == GSM_CAL_TRAVEL ) )
	 // *Length+=sprintf(Buffer+(*Length), "BEGIN:VEVENT%c%c",13,10);
	    *Length+=sprintf(Buffer+(*Length), "BEGIN:VEVENT%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
	else // GSM_CAL_BIRTHDAY, GSM_CAL_CALL , GSM_CAL_MEMO
	{
   //   *Length+=sprintf(Buffer+(*Length), "BEGIN:VTODO%c%c",13,10);
        *Length+=sprintf(Buffer+(*Length), "BEGIN:VTODO%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
      level = 2; // VTODO
	}


//testentrynum = note->EntriesNum;

	if (Time == -1) return ERR_UNKNOWN;
		  GSM_DateTime gsmDateTime;
		  memcpy(&gsmDateTime,&note->Entries[Time].Date,sizeof(GSM_DateTime));

		  if(Timezone>0)
			GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
		  else
			GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);

		  SF_SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DTSTART");

	if (EndTime != -1)
		{
			memcpy(&gsmDateTime,&note->Entries[EndTime].Date,sizeof(GSM_DateTime));
			if(Timezone>0)
				GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
			else
				GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);
			if ( level == 1 )  
			  SF_SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DTEND"); // VEVENT
			else   
			  SF_SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DUE");   //VTODO

		}

	if (Alarm != -1) {
			memcpy(&gsmDateTime,&note->Entries[Alarm].Date,sizeof(GSM_DateTime));
			if(Timezone>0)
				GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
			else
				GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);

			// 9@9u need both
			SF_SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DALARM");
			SF_SaveVCALDateTime(Buffer, Length, &gsmDateTime, "AALARM");

		}

	if (Text != -1) {
			unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
			CopyUnicodeString(utemp,note->Entries[Text].Text);
	//		UnicodeReplace(utemp,";","\\;");

			SF_SaveVCARDText(Buffer, Length, utemp, "SUMMARY",":");
		}

//testentrynum = note->EntriesNum;

	if (Recurrance != -1 ) 
		{
			int nFreq = 1;
			if(RecurranceFreq!= -1) nFreq = note->Entries[RecurranceFreq].Number;
			switch(note->Entries[Recurrance].Number)
			{
				case CAL_REPEAT_DAILY	 : 
 			        *Length+=sprintf(Buffer+(*Length), "RRULE:D%d #0",nFreq);
		//			*Length+=sprintf(Buffer+(*Length), "RRULE:D%d SU MO TU WE TH FR SA #0",nFreq);
        //        	*Length+=sprintf(Buffer+(*Length), "%c%c",13,10);
                    *Length+=sprintf(Buffer+(*Length), "%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
					break;

				case CAL_REPEAT_WEEKLY	 : 
		//			*Length+=sprintf(Buffer+(*Length), "RRULE:W%d #0",nFreq);
					if(DayofweekMask == -1)
					{
						*Length+=sprintf(Buffer+(*Length), "RRULE:W%d #0",nFreq);
					//	*Length+=sprintf(Buffer+(*Length), "%c%c",13,10);
                        *Length+=sprintf(Buffer+(*Length), "%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
					}
					else 
					{
						char szDay[MAX_PATH];
						int nMask = note->Entries[DayofweekMask].Number;
						szDay[0] = '\0';
						if(nMask & 1) strcat(szDay ,"SU ");
						if(nMask & 2) strcat(szDay ,"MO ");
						if(nMask & 4) strcat(szDay ,"TU ");
						if(nMask & 8) strcat(szDay ,"WE ");
						if(nMask & 16) strcat(szDay ,"TH ");
						if(nMask & 32) strcat(szDay ,"FR ");
						if(nMask & 64) strcat(szDay ,"SA ");
					//	if(strlen(szDay) > 3)
					//	{
							//12/19 for firmware bug ; 
                            nFreq = 1;
							*Length+=sprintf(Buffer+(*Length), "RRULE:W%d %s#0",nFreq,szDay);
						//	*Length+=sprintf(Buffer+(*Length), "%c%c",13,10);
							*Length+=sprintf(Buffer+(*Length), "%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
					//	}
					//	else 
					//	{
					//		*Length+=sprintf(Buffer+(*Length), "RRULE:W%d #0",nFreq);
                        //	*Length+=sprintf(Buffer+(*Length), "%c%c",13,10);
					//		*Length+=sprintf(Buffer+(*Length), "%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
					//	}

					}

					break;
				case CAL_REPEAT_MONTHLY	 : 
				//	*Length+=sprintf(Buffer+(*Length), "RRULE:MD%d #0",nFreq); // ????
					*Length+=sprintf(Buffer+(*Length), "RRULE:M%d #0",nFreq); // 0103
				//	*Length+=sprintf(Buffer+(*Length), "%c%c",13,10);
                    *Length+=sprintf(Buffer+(*Length), "%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
					break;

				case CAL_REPEAT_YEARLY : 
				//	*Length+=sprintf(Buffer+(*Length), "RRULE:YM%d #0",nFreq);
					*Length+=sprintf(Buffer+(*Length), "RRULE:Y%d #0",nFreq);
                    *Length+=sprintf(Buffer+(*Length), "%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
					break;


			}

		//	*Length+=sprintf(Buffer+(*Length), "%c%c",13,10);

		}

//testentrynum = note->EntriesNum;


//		*Length+=sprintf(Buffer+(*Length), "CATEGORIES;CHARSET=US-ASCII:");
		*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
		switch (note->Type) {
		case GSM_CAL_REMINDER:
			*Length+=sprintf(Buffer+(*Length), "Reminder%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
			break;
		case GSM_CAL_MEETING:
			*Length+=sprintf(Buffer+(*Length), "Meeting%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
			break;
		case GSM_CAL_BIRTHDAY:
			*Length+=sprintf(Buffer+(*Length), "Anniversary%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
			break;
		case GSM_CAL_MEMO:
			*Length+=sprintf(Buffer+(*Length), "Date%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
			break;
		case GSM_CAL_CALL:
			*Length+=sprintf(Buffer+(*Length), "Call%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
			break;
		case GSM_CAL_TRAVEL:
			*Length+=sprintf(Buffer+(*Length), "Course%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
			break;
		default:
			*Length+=sprintf(Buffer+(*Length), "Meeting%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
			break;
		}

	   if (Priority != -1) {
		// note->Entries[Priority].Number = 1; // for test 
	    *Length+=sprintf(Buffer+(*Length), "PRIORITY:%d%c%c%c%c",note->Entries[Priority].Number,0x2C,0x2C,0x2C,0x2C);
		}
	   else
	   {
//		 note->Entries[Priority].Number = 2; // for test 
	    *Length+=sprintf(Buffer+(*Length), "PRIORITY:%d%c%c%c%c",2,0x2C,0x2C,0x2C,0x2C);
	   }


		if (Location != -1) {
			unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
			CopyUnicodeString(utemp,note->Entries[Location].Text);
		//	UnicodeReplace(utemp,";","\\;");
			SF_SaveVCARDText(Buffer, Length, utemp, "LOCATION",":");
		}

	if ( level == 1 )
	  *Length+=sprintf(Buffer+(*Length), "END:VEVENT%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
	else
	  *Length+=sprintf(Buffer+(*Length), "END:VTODO%c%c%c%c",0x2C,0x2C,0x2C,0x2C);

//	if (header) *Length+=sprintf(Buffer+(*Length), "END:VCALENDAR%c%c%c%c",0x2C,0x2C,0x2C,0x2C);
	if (header) *Length+=sprintf(Buffer+(*Length), "END:VCALENDAR");

//testentrynum = note->EntriesNum;	
	
	return ERR_NONE;
}



bool NextMonthlyWeeklyDate(COleDateTime dtMonth, int whichWeek,int whichDay,COleDateTime &dtOutput)
{
	COleDateTime dt2;
	COleDateTimeSpan dts;
	dt2.SetDate(dtMonth.GetYear(),dtMonth.GetMonth(),1);
	COleDateTime dt3 = dt2; 
	int doW2 = dt2.GetDayOfWeek();
	doW2--;

	if (whichDay < doW2) {
		dts.SetDateTimeSpan(whichDay-doW2+7,0,0,0);
		whichWeek--;
	} else {
		dts.SetDateTimeSpan(whichDay-doW2,0,0,0);
		whichWeek--; //peggy 0614
	}


	dt2 += dts;  //First whichDay

	dts.SetDateTimeSpan(7,0,0,0);
	for (int i=0;i<whichWeek;i++) {
		dt2 += dts;
	}

	if (dt2.GetMonth() != dt3.GetMonth()) {
		dts.SetDateTimeSpan(7,0,0,0);
		dt2 -= dts;
		if (dt2.GetMonth() != dt3.GetMonth()) {
			dt2 -= dts;
		}
		dtOutput = dt2;
		return true;
	}
	dtOutput = dt2;
	return true;
}
bool GetRepeatDate_Weekly(COleDateTime dtStart,int interval,int nCount ,COleDateTime& dtRepeatEndDate,int DayofWeekMask)
{
	COleDateTimeSpan dts(7*interval,0,0,0) ;
	COleDateTime dtNext = dtStart;

	CList<COleDateTimeSpan , COleDateTimeSpan> DayMaskList;
	DayMaskList.RemoveAll();
	SYSTEMTIME st;
	int nWeekMask;
	for(int week = 1;week<7;week++)
	{
		COleDateTimeSpan dtspan(week,0,0,0) ;
		COleDateTime dtTemp = dtStart+dtspan;
		dtTemp.GetAsSystemTime(st);
		nWeekMask = 1<<st.wDayOfWeek;
		if(nWeekMask & DayofWeekMask)
			DayMaskList.AddTail(dtspan);
	}

	int nRepeatCount = 0;
	POSITION pos;

	nRepeatCount = 1;
	dtRepeatEndDate = dtNext;
	while((int)nRepeatCount < (int)nCount)
	{
		pos = DayMaskList.GetHeadPosition();
		while(pos && (int)nRepeatCount < (int)nCount)
		{
			COleDateTimeSpan span = DayMaskList.GetNext(pos);
			COleDateTime dtmaskweekday = dtNext+span;
			dtRepeatEndDate = dtmaskweekday;
			nRepeatCount++;
		}
		if((int)nRepeatCount < (int)nCount)
		{
			dtNext = dtNext + dts;
			dtRepeatEndDate = dtNext;
			nRepeatCount++;
		}
	}
	return true;
}
GSM_Error MOTOE2_DecodeVCALENDAR(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, int* pTimezone)
{
	unsigned char 	Line[2000],Buff[2000],	Line2[2000];
    int             Level = 0,npos;
	char szCalType[500];
//	GSM_DateTime ToDoDueDate,ToDoCompletedDate;
//	ToDoDueDate.Year = ToDoCompletedDate.Year = 0;
	bool bToDoCompleted = false;
//	char  str[30];
//	time_t time;
	int nTimeZone = *pTimezone;

	int nRepeatCount = 0;
	int nRepeatType=0; //1:Daily 2:Weekly 3:Monthly MD 4:MP 5:Yearly
	int interval = 0,	nDayofWeekMask=0;	
	Calendar->EntriesNum 	= 0;
	ToDo->EntriesNum 	= 0;
	COleDateTime dtDaylightStart,dtDaylightEnd;
	int nDayLightZone = 0;
	while (1) {
		MyGetLine(Buffer, Pos, Line, strlen((char *)Buffer));
		if (strlen((char *)Line) == 0) break;
 		//Modify for SE K750i
		//=E6=88=91=E9=96=80=E6=98=A=..F=E4=B8=80=E5=AE=B6=E4=BA=BA..
		npos = *Pos;
		memcpy(Line2,Line,strlen((char*)Line)+1);
		while(Line2[strlen((char*)Line2)-1] == '=')
		{
			if(Buffer[npos+2] == 0x0d && Buffer[npos+3] == 0x0A)
			{
				if(Line[strlen((char*)Line)-1] == '=')
					Line[strlen((char*)Line)-1]= '\0';
				break;
			}
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
			if(Line2[0] == '=')
			{
				strcat((char *)Line,(char *)Line2+1);
			}
			else
			{
				Line[strlen((char*)Line)-1]= '\0';
				strcat((char *)Line,(char *)Line2);
			}
		}
		MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
		while(Line2[0] == 0x20 && Line2[1] == 0x20)
		{
			strcat((char *)Line,(char *)Line2);
			MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
		}
/*		npos = *Pos;//peggy
        MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
  		//peggy +
		while(Line2[0] == '=')
		{
			strcat((char *)Line,(char *)Line2+1);

			if(Line2[strlen((char *)Line2) -1] == '=')
				MyGetLine(Buffer, &npos, Line2, strlen((char *)Buffer));//peggy
			else 
				break;
		}
		//peggy +*/
		switch (Level) {
		case 0:
			if (strnicmp((char *)Line,"TZ:",3) == 0)
			{
				char time[3];
				int TimeZone = 0;
				time[3]='\0';
				if(Line[3] == '-')
				{
					memcpy(time,Line+4,2);
					TimeZone = atoi(time);
					nTimeZone = TimeZone * (-4);
				}
				else
				{
					memcpy(time,Line+3,2);
					TimeZone = atoi(time);
					nTimeZone = TimeZone*4;
				}
			}
			if (strnicmp((char *)Line,"DAYLIGHT:TRUE;",14) == 0)
			{
				char time[3];
				int TimeZone = 0;
				time[3]='\0';
				if(Line[14] == '-')
				{
					memcpy(time,Line+15,2);
					TimeZone = atoi(time);
					nDayLightZone = TimeZone * (-4);
				}
				if(Line[14] == '+')
				{
					memcpy(time,Line+15,2);
					TimeZone = atoi(time);
					nDayLightZone = TimeZone * 4;
				}
				else
				{
					memcpy(time,Line+14,2);
					TimeZone = atoi(time);
					nDayLightZone = TimeZone*4;
				}

				char *pstr = strchr((char *)Line+14,';');
		
				if(pstr)
				{
					GSM_DateTime gsmTime;
					ZeroMemory(&gsmTime,sizeof(GSM_DateTime));
					ReadVCALDateTime(pstr+1, &gsmTime,nDayLightZone);
					dtDaylightStart.SetDateTime(gsmTime.Year,gsmTime.Month,gsmTime.Day,	gsmTime.Hour,gsmTime.Minute,gsmTime.Second);

					pstr = strchr((char *)pstr+1,';');
					if(pstr)
					{
						ZeroMemory(&gsmTime,sizeof(GSM_DateTime));
						ReadVCALDateTime(pstr+1, &gsmTime,nDayLightZone);
						dtDaylightEnd.SetDateTime(gsmTime.Year,gsmTime.Month,gsmTime.Day,	gsmTime.Hour,gsmTime.Minute,gsmTime.Second);
					}
					else
						nDayLightZone=0;
				}
				else nDayLightZone = 0;
			}

			if (strstr((char *)Line,"BEGIN:VEVENT")) {
				Calendar->Type 	= GSM_CAL_MEETING;
				Level 		= 1;
			}
			if (strstr((char *)Line,"BEGIN:VTODO")) {
				ToDo->Priority 	= 2;//GSM_Priority_Low;
				Level 		= 2;
			}
			break;
		case 1: /* Calendar note */
			if (strstr((char *)Line,"END:VEVENT"))
			{
				if (Calendar->EntriesNum == 0) return ERR_EMPTY;
				
				if(nRepeatCount !=0 && nRepeatType !=0)
				{
					for(int i = 0; i<Calendar->EntriesNum;i++)
					{
						if(Calendar->Entries[i].EntryType == CAL_START_DATETIME)
						{
							COleDateTime oleStartDate;
							oleStartDate.SetDateTime(Calendar->Entries[i].Date.Year,Calendar->Entries[i].Date.Month,Calendar->Entries[i].Date.Day,
								Calendar->Entries[i].Date.Hour,Calendar->Entries[i].Date.Minute,Calendar->Entries[i].Date.Second);
							switch(nRepeatType)
							{
							case 1://dayly
								{
									COleDateTimeSpan ts(nRepeatCount,0,0,0) ;
									COleDateTime dtEndDate = oleStartDate +ts;

									Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
									Calendar->Entries[Calendar->EntriesNum].Date.Year = dtEndDate.GetYear();
									Calendar->Entries[Calendar->EntriesNum].Date.Month = dtEndDate.GetMonth();
									Calendar->Entries[Calendar->EntriesNum].Date.Day = dtEndDate.GetDay();
									Calendar->Entries[Calendar->EntriesNum].Date.Hour = Calendar->Entries[Calendar->EntriesNum].Date.Minute =Calendar->Entries[Calendar->EntriesNum].Date.Second=0;
									Calendar->EntriesNum++;
								}
								break;
							case 2://weekly
								{
							/*		COleDateTimeSpan ts(nRepeatCount*7,0,0,0) ;*/
									COleDateTime dtEndDate ;
									GetRepeatDate_Weekly(oleStartDate,interval,nRepeatCount ,dtEndDate,nDayofWeekMask);

									Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
									Calendar->Entries[Calendar->EntriesNum].Date.Year = dtEndDate.GetYear();
									Calendar->Entries[Calendar->EntriesNum].Date.Month = dtEndDate.GetMonth();
									Calendar->Entries[Calendar->EntriesNum].Date.Day = dtEndDate.GetDay();
									Calendar->Entries[Calendar->EntriesNum].Date.Hour = Calendar->Entries[Calendar->EntriesNum].Date.Minute =Calendar->Entries[Calendar->EntriesNum].Date.Second=0;
									Calendar->EntriesNum++;
								}
								break;
							case 3://MD
								{
									int nMonthCount = oleStartDate.GetMonth() + nRepeatCount  ;
									int nMonth = nMonthCount%12;
									int nYear =nMonthCount /12;
									if(nMonth == 0)
									{
										nYear--;
										nMonth = 12;
									}
									Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
									Calendar->Entries[Calendar->EntriesNum].Date.Year = oleStartDate.GetYear() + nYear;
									Calendar->Entries[Calendar->EntriesNum].Date.Month = nMonth;
									Calendar->Entries[Calendar->EntriesNum].Date.Day = oleStartDate.GetDay();
									Calendar->Entries[Calendar->EntriesNum].Date.Hour = Calendar->Entries[Calendar->EntriesNum].Date.Minute =Calendar->Entries[Calendar->EntriesNum].Date.Second=0;
									Calendar->EntriesNum++;


								}
								break;
							case 4: // mp
								{
									int nMonthCount = oleStartDate.GetMonth() + nRepeatCount  ;
									int nMonth = nMonthCount%12;
									int nYear =nMonthCount /12;
									if(nMonth == 0)
									{
										nYear--;
										nMonth = 12;
									}
									
									COleDateTime dtEndDate(oleStartDate.GetYear() + nYear,nMonth,oleStartDate.GetDay(),oleStartDate.GetHour(),
													oleStartDate.GetMinute(),oleStartDate.GetSecond());
	
									unsigned int Week,Day;
									GetwhichWeekDay(Calendar->Entries[i].Date,Week,Day);

									COleDateTime dtRepeatEnd;
									NextMonthlyWeeklyDate(dtEndDate,Week,Day,dtRepeatEnd);

									Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
									Calendar->Entries[Calendar->EntriesNum].Date.Year = dtRepeatEnd.GetYear();
									Calendar->Entries[Calendar->EntriesNum].Date.Month = dtRepeatEnd.GetMonth();
									Calendar->Entries[Calendar->EntriesNum].Date.Day = dtRepeatEnd.GetDay();
									Calendar->Entries[Calendar->EntriesNum].Date.Hour = Calendar->Entries[Calendar->EntriesNum].Date.Minute =Calendar->Entries[Calendar->EntriesNum].Date.Second=0;
									Calendar->EntriesNum++;
								}
								break;
							case 5://yearly
								{
									Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
									Calendar->Entries[Calendar->EntriesNum].Date.Year = oleStartDate.GetYear() + nRepeatCount;
									Calendar->Entries[Calendar->EntriesNum].Date.Month = oleStartDate.GetMonth();
									Calendar->Entries[Calendar->EntriesNum].Date.Day = oleStartDate.GetDay();
									Calendar->Entries[Calendar->EntriesNum].Date.Hour = Calendar->Entries[Calendar->EntriesNum].Date.Minute =Calendar->Entries[Calendar->EntriesNum].Date.Second=0;
									Calendar->EntriesNum++;

								}
								break;
							}

							break;
						}
					}
				}

				return ERR_NONE;
			}
			if (ReadVCALText(Line, "CATEGORIES", Buff) )
			{

				DecodeUnicode(Buff,(unsigned char *)szCalType);
				if (strstr(szCalType,"none")) 	Calendar->Type = GSM_CAL_MEETING;
				if (strstr(szCalType,"meeting"))	 	Calendar->Type = GSM_CAL_REMINDER;//SE
				if (strstr(szCalType,"appointment"))	 	Calendar->Type = GSM_CAL_CALL;  //SE
				if (strstr(szCalType,"presentation"))	 	Calendar->Type = GSM_CAL_BIRTHDAY;//SE
				if (strstr(szCalType,"conference call")) 	Calendar->Type = GSM_CAL_MEMO;
				if (strstr(szCalType,"breakfast")) 	Calendar->Type = GSM_CAL_TRAVEL;
				if (strstr(szCalType,"lunch")) Calendar->Type = GSM_CAL_VACATION;
				if (strstr(szCalType,"dinner")) 	Calendar->Type = GSM_CAL_T_ATHL;
				if (strstr(szCalType,"anniversary")) 		Calendar->Type = GSM_CAL_T_CYCL;
				if (strstr(szCalType,"birthday")) 	Calendar->Type = GSM_CAL_T_BUDO;

				if (strstr(szCalType,"flight")) 		Calendar->Type = GSM_CAL_T_DANC;
				if (strstr(szCalType,"party")) 	Calendar->Type = GSM_CAL_T_EXTR;
				if (strstr(szCalType,"vacation")) Calendar->Type = GSM_CAL_T_FOOT;
			}
	
 	////////	
			char *pTempFrequency;
			if ((pTempFrequency = strstr((char *)Line,"RRULE:D"))) 
			{
				int nFrequency = atoi(pTempFrequency+7);

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				if(nFrequency == 7)
				{
					Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY ;
					nFrequency = 1;
				}
				else
					Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_DAILY ;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = nFrequency ;

				Calendar->EntriesNum++;
				char *pstr = strstr((char *)Line,"#");
				if(pstr)
				{
					int nCount = atoi(pstr+1);
					if(nCount>0)
					{
						nRepeatCount = (nCount-1)*nFrequency;
						nRepeatType =1;
					}
				}

			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:W"))) 
			{
		//	if ((strstr((char *)Line,"RRULE:W1")) || (strstr((char *)Line,"RRULE:D7"))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_WEEKLY;//7*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+7) ;
				Calendar->EntriesNum++;

				int DayOfWeekMask = 0;
				if(strstr((char *)Line,"SU"))
					DayOfWeekMask |=1;
				if(strstr((char *)Line,"MO"))
					DayOfWeekMask |=2;
				if(strstr((char *)Line,"TU"))
					DayOfWeekMask |=4;
				if(strstr((char *)Line,"WE"))
					DayOfWeekMask |=8;
				if(strstr((char *)Line,"TH"))
					DayOfWeekMask |=16;
				if(strstr((char *)Line,"FR"))
					DayOfWeekMask |=32;
				if(strstr((char *)Line,"SA"))
					DayOfWeekMask |=64;
				if(DayOfWeekMask>0)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_DAYOFWEEK;
					Calendar->Entries[Calendar->EntriesNum].Number    = DayOfWeekMask  ;
					Calendar->EntriesNum++;
				}

			/*	if(strlen((char *)Line) >=24)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &Calendar->Entries[Calendar->EntriesNum].Date,nTimeZone);
					Calendar->EntriesNum++;

				}*/
				char *pstr = strstr((char *)Line,"#");
				if(pstr)
				{
					int nCount = atoi(pstr+1);
					if(nCount>0)
					{
						nRepeatCount = nCount;
						interval = atoi(pTempFrequency+7);	
						nDayofWeekMask = DayOfWeekMask;
						nRepeatType =2;
					}
				}

			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:MD"))) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_MONTHLY;//30*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+8)  ;
				Calendar->EntriesNum++;
		/*		if(strlen((char *)Line) >=24)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &Calendar->Entries[Calendar->EntriesNum].Date,nTimeZone);
					Calendar->EntriesNum++;

				}*/
				char *pstr = strstr((char *)Line,"#");
				if(pstr)
				{
					int nCount = atoi(pstr+1);
					if(nCount>0)
					{
						nRepeatCount = (nCount-1)*(atoi(pTempFrequency+8));
						nRepeatType =3;
					}
				}
			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:MP"))) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_MONTH_WEEKLY;//30*24;
				Calendar->EntriesNum++;


				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+8)  ;
				Calendar->EntriesNum++;


		/*		if(strlen((char *)Line) >=24)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &Calendar->Entries[Calendar->EntriesNum].Date,nTimeZone);

					Calendar->EntriesNum++;

				}*/
				char *pstr = strstr((char *)Line,"#");
				if(pstr)
				{
					int nCount = atoi(pstr+1);
					if(nCount>0)
					{
						nRepeatCount = (nCount-1)*(atoi(pTempFrequency+8));
						nRepeatType =4;
					}
				}

			}

			if ((pTempFrequency = strstr((char *)Line,"RRULE:YM"))) 
			{
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCE;
				Calendar->Entries[Calendar->EntriesNum].Number    = CAL_REPEAT_YEARLY;//365*24;
				Calendar->EntriesNum++;

				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_RECURRANCEFREQUENCY;
				Calendar->Entries[Calendar->EntriesNum].Number    = atoi(pTempFrequency+8) ;
				Calendar->EntriesNum++;
			/*	if(strlen((char *)Line) >=24)
				{
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &Calendar->Entries[Calendar->EntriesNum].Date,nTimeZone);
					Calendar->EntriesNum++;

				}*/
				char *pstr = strstr((char *)Line,"#");
				if(pstr)
				{
					int nCount = atoi(pstr+1);
					if(nCount>0)
					{
						nRepeatCount = (nCount-1)*(atoi(pTempFrequency+8));
						nRepeatType =5;
					}
				}
			}


			if ((ReadVCALText(Line, "SUMMARY", Buff)) ) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
			}
			if ((ReadVCALText(Line, "DESCRIPTION", Buff))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_DESCRIPTION;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
			}

			if (ReadVCALText(Line, "LOCATION", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LOCATION;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,Buff);
				UnicodeReplace(Calendar->Entries[Calendar->EntriesNum].Text,"\\;",";");
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DTSTART", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_START_DATETIME;
			//	ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,nTimeZone);
				ReadVCALDateTimeWithDayLight((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,dtDaylightStart,dtDaylightEnd,nTimeZone,nDayLightZone);
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DTEND", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_END_DATETIME;
			//	ReadVCALDateTime((char *)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,nTimeZone);
				ReadVCALDateTimeWithDayLight((char *)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,dtDaylightStart,dtDaylightEnd,nTimeZone,nDayLightZone);
	
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "DALARM", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
			//	ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,nTimeZone);
				ReadVCALDateTimeWithDayLight((char *)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,dtDaylightStart,dtDaylightEnd,nTimeZone,nDayLightZone);
			
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "AALARM", Buff) ||ReadVCALText(Line, "AALARM;TYPE=X-EPOCSOUND", Buff)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_ALARM_DATETIME;
			//	ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,nTimeZone);
				ReadVCALDateTimeWithDayLight((char *)DecodeUnicodeString(Buff), &Calendar->Entries[Calendar->EntriesNum].Date,dtDaylightStart,dtDaylightEnd,nTimeZone,nDayLightZone);
			
				Calendar->EntriesNum++;
			}
			break;
		case 2: /* ToDo note */
			if (strstr((char *)Line,"END:VTODO")) 
			{
				if (ToDo->EntriesNum == 0) return ERR_EMPTY;
			/*	if(bToDoCompleted && ToDoCompletedDate.Year !=0)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
					ToDo->Entries[ToDo->EntriesNum].Date = ToDoCompletedDate;
					ToDo->EntriesNum++;
				}
				else if(bToDoCompleted == false && ToDoDueDate.Year !=0)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
					ToDo->Entries[ToDo->EntriesNum].Date = ToDoDueDate;
					ToDo->EntriesNum++;
				}*/

				return ERR_NONE;
			}
			if (ReadVCALText(Line, "DUE", Buff))
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
			//	ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,nTimeZone);
				ReadVCALDateTimeWithDayLight((char *)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,dtDaylightStart,dtDaylightEnd,nTimeZone,nDayLightZone);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "COMPLETED", Buff))
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETEDDATE;
			//	ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,nTimeZone);
				ReadVCALDateTimeWithDayLight((char *)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,dtDaylightStart,dtDaylightEnd,nTimeZone,nDayLightZone);

				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "DALARM", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_SILENT_ALARM_DATETIME;
		//		ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,nTimeZone);
				ReadVCALDateTimeWithDayLight((char *)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,dtDaylightStart,dtDaylightEnd,nTimeZone,nDayLightZone);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "AALARM", Buff)||ReadVCALText(Line, "AALARM;TYPE=X-EPOCSOUND", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_ALARM_DATETIME;
		//		ReadVCALDateTime((char*)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,nTimeZone);
				ReadVCALDateTimeWithDayLight((char *)DecodeUnicodeString(Buff), &ToDo->Entries[ToDo->EntriesNum].Date,dtDaylightStart,dtDaylightEnd,nTimeZone,nDayLightZone);
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "SUMMARY", Buff)) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_TEXT;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				UnicodeReplace(ToDo->Entries[ToDo->EntriesNum].Text,"\\;",";");
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "PRIORITY", Buff)) {
			/*	if (ToDoVer == SonyEricsson_VToDo) {
					ToDo->Priority = GSM_Priority_Low;
					if (atoi((char *)DecodeUnicodeString(Buff))==2) ToDo->Priority = GSM_Priority_Medium;
					if (atoi((char *)DecodeUnicodeString(Buff))==1) ToDo->Priority = GSM_Priority_High;
					dbgprintf("atoi is %i %s\n",atoi((char *)DecodeUnicodeString(Buff)),DecodeUnicodeString(Buff));
				} else if (ToDoVer == Nokia_VToDo)*/ 
			/*	{
					ToDo->Priority = GSM_Priority_Medium;
					if (atoi((char *)DecodeUnicodeString(Buff))==3) ToDo->Priority = GSM_Priority_Low;
					if (atoi((char *)DecodeUnicodeString(Buff))==2) ToDo->Priority = GSM_Priority_Medium;
					if (atoi((char *)DecodeUnicodeString(Buff))==1) ToDo->Priority = GSM_Priority_High;
				}*/
				ToDo->Priority = atoi((char *)DecodeUnicodeString(Buff));
			}
			if (strstr((char *)Line,"STATUS:COMPLETED")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 1;
				ToDo->EntriesNum++;
				bToDoCompleted = true;
			}
			if (strstr((char *)Line,"STATUS:NEEDS ACTION")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 0;
				ToDo->EntriesNum++;
				bToDoCompleted = false;
			}
//Repeat  ++++++++++
/*			if ((pTempFrequency = strstr((char *)Line,"RRULE:D"))) 
			{
				int nFrequency = atoi(pTempFrequency+7);

				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCE;
				if(nFrequency == 7)
				{
					ToDo->Entries[ToDo->EntriesNum].Number    = CAL_REPEAT_WEEKLY ;
					nFrequency = 1;
				}
				else
					ToDo->Entries[ToDo->EntriesNum].Number    = CAL_REPEAT_DAILY ;
				ToDo->EntriesNum++;

				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCEFREQUENCY;
				ToDo->Entries[ToDo->EntriesNum].Number    = nFrequency ;

				ToDo->EntriesNum++;
				if(strlen((char *)Line) >=24)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &ToDo->Entries[ToDo->EntriesNum].Date,nTimeZone);

					if((nTimeZone)>0)
						GetTimeDifference((nTimeZone)*15, &ToDo->Entries[ToDo->EntriesNum].Date, true, 60);
					else
						GetTimeDifference(-(nTimeZone)*15, &ToDo->Entries[ToDo->EntriesNum].Date, false, 60);
					ToDo->EntriesNum++;

				}

			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:W"))) 
			{
		//	if ((strstr((char *)Line,"RRULE:W1")) || (strstr((char *)Line,"RRULE:D7"))) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCE;
				ToDo->Entries[ToDo->EntriesNum].Number    = CAL_REPEAT_WEEKLY;//7*24;
				ToDo->EntriesNum++;

				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCEFREQUENCY;
				ToDo->Entries[ToDo->EntriesNum].Number    = atoi(pTempFrequency+7) ;
				ToDo->EntriesNum++;

				int DayOfWeekMask = 0;
				if(strstr((char *)Line,"SU"))
					DayOfWeekMask |=1;
				if(strstr((char *)Line,"MO"))
					DayOfWeekMask |=2;
				if(strstr((char *)Line,"TU"))
					DayOfWeekMask |=4;
				if(strstr((char *)Line,"WE"))
					DayOfWeekMask |=8;
				if(strstr((char *)Line,"TH"))
					DayOfWeekMask |=16;
				if(strstr((char *)Line,"FR"))
					DayOfWeekMask |=32;
				if(strstr((char *)Line,"SA"))
					DayOfWeekMask |=64;
				if(DayOfWeekMask>0)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_REPEAT_DAYOFWEEK;
					ToDo->Entries[ToDo->EntriesNum].Number    = DayOfWeekMask  ;
					ToDo->EntriesNum++;
				}

				if(strlen((char *)Line) >=24)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &ToDo->Entries[ToDo->EntriesNum].Date,nTimeZone);

					ToDo->EntriesNum++;

				}
			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:MD"))) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCE;
				ToDo->Entries[ToDo->EntriesNum].Number    = CAL_REPEAT_MONTHLY;//30*24;
				ToDo->EntriesNum++;

				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCEFREQUENCY;
				ToDo->Entries[ToDo->EntriesNum].Number    = atoi(pTempFrequency+8)  ;
				ToDo->EntriesNum++;
				if(strlen((char *)Line) >=24)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &ToDo->Entries[ToDo->EntriesNum].Date,nTimeZone);
					ToDo->EntriesNum++;

				}
			}
			if ((pTempFrequency = strstr((char *)Line,"RRULE:MP"))) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCE;
				ToDo->Entries[ToDo->EntriesNum].Number    = CAL_REPEAT_MONTH_WEEKLY;//30*24;
				ToDo->EntriesNum++;


				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCEFREQUENCY;
				ToDo->Entries[ToDo->EntriesNum].Number    = atoi(pTempFrequency+8)  ;
				ToDo->EntriesNum++;


				if(strlen((char *)Line) >=24)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &ToDo->Entries[ToDo->EntriesNum].Date,nTimeZone);

					ToDo->EntriesNum++;

				}
			}

			if ((pTempFrequency = strstr((char *)Line,"RRULE:YM"))) 
			{
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCE;
				ToDo->Entries[ToDo->EntriesNum].Number    = CAL_REPEAT_YEARLY;//365*24;
				ToDo->EntriesNum++;

				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_RECURRANCEFREQUENCY;
				ToDo->Entries[ToDo->EntriesNum].Number    = atoi(pTempFrequency+8) ;
				ToDo->EntriesNum++;
				if(strlen((char *)Line) >=24)
				{
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_REPEAT_STOPDATE;
					int nLen = strlen((char*)Line);
					ReadVCALDateTime((char*)Line+nLen-15, &ToDo->Entries[ToDo->EntriesNum].Date,nTimeZone);
					ToDo->EntriesNum++;

				}
			}
*/
//Repeat  ---------
			if ((ReadVCALText(Line, "DESCRIPTION", Buff))) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_DESCRIPTION;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,Buff);
				UnicodeReplace(ToDo->Entries[ToDo->EntriesNum].Text,"\\;",";");
				ToDo->EntriesNum++;
			}


			break;
		}
	}

	if (Calendar->EntriesNum == 0 && ToDo->EntriesNum == 0) return ERR_EMPTY;


	return ERR_NONE;
}
bool GetRepeatDatesCount_Daily(COleDateTime dtStart,int interval,int* nCount ,COleDateTime& dtRepeatEndDate)
{
	COleDateTimeSpan dts(1*interval,0,0,0) ;
	COleDateTime dtNext = dtStart;
	COleDateTime dtLast = dtStart;
	(*nCount)= 0;

	while((int)dtRepeatEndDate.m_dt >= (int)dtNext.m_dt)
	{
	//	pDataTimeList->AddTail(dtNext);
		dtLast = dtNext;
		(*nCount)++;
		dtNext = dtNext + dts;
	}
	dtRepeatEndDate = dtLast;
	return true;
}
bool GetRepeatDate_Daily(COleDateTime dtStart,int interval,int nCount ,COleDateTime& dtRepeatEndDate)
{
	COleDateTimeSpan dts(1*interval,0,0,0) ;
	COleDateTime dtNext = dtStart;
	COleDateTime dtLast = dtStart;

	while(nCount>0)
	{
		dtRepeatEndDate +=  dts;
		nCount--;
	}
	return true;
}
bool GetRepeatDatesCount_Weekly(COleDateTime dtStart,int interval,int* nCount ,COleDateTime& dtRepeatEndDate,int DayofWeekMask)
{
	COleDateTimeSpan dts(7*interval,0,0,0) ;
	COleDateTime dtNext = dtStart;
	COleDateTime dtLast = dtStart;

	CList<COleDateTimeSpan , COleDateTimeSpan> DayMaskList;
	DayMaskList.RemoveAll();
	SYSTEMTIME st;
	int nWeekMask;
	for(int week = 1;week<7;week++)
	{
		COleDateTimeSpan dtspan(week,0,0,0) ;
		COleDateTime dtTemp = dtStart+dtspan;
		dtTemp.GetAsSystemTime(st);
		nWeekMask = 1<<st.wDayOfWeek;
		if(nWeekMask & DayofWeekMask)
			DayMaskList.AddTail(dtspan);
	}
	(*nCount) = 0;
	POSITION pos;
	while((int)dtRepeatEndDate.m_dt >= (int)dtNext.m_dt)
	{
		(*nCount)++;
		dtLast = dtNext;
		pos = DayMaskList.GetHeadPosition();
		while(pos)
		{
			COleDateTimeSpan span = DayMaskList.GetNext(pos);
			COleDateTime dtmaskweekday = dtNext+span;
			if((int)dtRepeatEndDate.m_dt >= (int)dtmaskweekday.m_dt)
			{
				(*nCount)++;
				dtLast = dtmaskweekday;
			}

		}
		dtNext = dtNext + dts;
	}
	dtRepeatEndDate = dtLast;
	return true;
}
bool GetRepeatDatesCount_Monthly(COleDateTime dtStart,int interval,int* nCount ,COleDateTime& dtRepeatEndDate)
{
	int nY = dtStart.GetYear();
	int nM = dtStart.GetMonth();
	int nD = dtStart.GetDay();
	int nH = dtStart.GetHour();
	int nMin = dtStart.GetMinute();
	int nS = dtStart.GetSecond();
	
	int nMonthCount  ;
	int nMonth ;
	int nYear;

	COleDateTime dtNext = dtStart;
	COleDateTime dtLast = dtStart;

	(*nCount) = 0;
	int nIndex = 1;
	while((int)dtRepeatEndDate.m_dt >= (int)dtNext.m_dt)
	{
		if(dtNext.GetStatus() == COleDateTime::valid)
		{
			dtLast = dtNext;
			(*nCount)++;
		}
		nMonthCount = nM + (nIndex * interval)  ;
		nMonth = nMonthCount%12;
		nYear =nMonthCount /12;
		if(nMonth == 0)
		{
			nYear--;
			nMonth = 12;
		}
		dtNext.SetDateTime(nY+nYear,nMonth,nD,nH,nMin,nS);
		nIndex++;
	}
	dtRepeatEndDate = dtLast;
	return true;
}
bool GetRepeatDate_Monthly(COleDateTime dtStart,int interval,int nCount ,COleDateTime& dtRepeatEndDate)
{
	int nY = dtStart.GetYear();
	int nM = dtStart.GetMonth();
	int nD = dtStart.GetDay();
	int nH = dtStart.GetHour();
	int nMin = dtStart.GetMinute();
	int nS = dtStart.GetSecond();
	
	int nMonthCount  ;
	int nMonth ;
	int nYear;

	COleDateTime dtNext = dtStart;
	COleDateTime dtLast = dtStart;

	int nIndex = 1;
	while(nCount>0)
	{
		if(dtNext.GetStatus() == COleDateTime::valid)
		{
			dtLast = dtNext;
			nCount--;
		}
		nMonthCount = nM + (nIndex * interval)  ;
		nMonth = nMonthCount%12;
		nYear =nMonthCount /12;
		if(nMonth == 0)
		{
			nYear--;
			nMonth = 12;
		}
		dtNext.SetDateTime(nY+nYear,nMonth,nD,nH,nMin,nS);
		nIndex++;
	}
	dtRepeatEndDate = dtLast;
	return true;
}
bool GetRepeatDatesCount_MonthNth(COleDateTime dtStart,int interval,int* nCount ,COleDateTime& dtRepeatEndDate,int Week,int Day)
{
	COleDateTime dtNext = dtStart;
	COleDateTime dtLast = dtStart;

	COleDateTimeSpan dts;
	dts.SetDateTimeSpan(5,0,0,0);
	COleDateTime dtMonth;
	dtMonth.SetDate(dtStart.GetYear(),dtStart.GetMonth(),28);

	(*nCount) = 0;
	while((int)dtRepeatEndDate.m_dt >= (int)dtNext.m_dt)
	{
		if(dtNext.GetStatus() == COleDateTime::valid)
		{
			dtLast = dtNext;
			(*nCount)++;
		}
		for(int j =0; j<interval;j++)
		{
			dtMonth+=dts;
			dtMonth.SetDate(dtMonth.GetYear(),dtMonth.GetMonth(),28);
		}
		NextMonthlyWeeklyDate(dtMonth,Week,Day,dtNext);
	}
	dtRepeatEndDate = dtLast;

	return true;
}
bool GetRepeatDate_MonthNth(COleDateTime dtStart,int interval,int nCount ,COleDateTime& dtRepeatEndDate,int Week,int Day)
{
	COleDateTime dtNext = dtStart;
	COleDateTime dtLast = dtStart;

	COleDateTimeSpan dts;
	dts.SetDateTimeSpan(5,0,0,0);
	COleDateTime dtMonth;
	dtMonth.SetDate(dtStart.GetYear(),dtStart.GetMonth(),28);

	while(nCount>0)
	{
		if(dtNext.GetStatus() == COleDateTime::valid)
		{
			dtLast = dtNext;
			nCount--;
		}
		for(int j =0; j<interval;j++)
		{
			dtMonth+=dts;
			dtMonth.SetDate(dtMonth.GetYear(),dtMonth.GetMonth(),28);
		}
		NextMonthlyWeeklyDate(dtMonth,Week,Day,dtNext);
	}
	dtRepeatEndDate = dtLast;

	return true;
}
bool GetRepeatDatesCount_Yearly(COleDateTime dtStart,int interval,int* nCount ,COleDateTime& dtRepeatEndDate)
{
	int nY = dtStart.GetYear();
	int nM = dtStart.GetMonth();
	int nD = dtStart.GetDay();
	int nH = dtStart.GetHour();
	int nMin = dtStart.GetMinute();
	int nS = dtStart.GetSecond();
	COleDateTime dtNext = dtStart;
	COleDateTime dtLast = dtStart;
	int nIndex = 1;
	(*nCount) = 0;

	while((int)dtRepeatEndDate.m_dt >= (int)dtNext.m_dt)
	{
		dtLast = dtNext;
		(*nCount)++;
		dtNext.SetDateTime(nY+(nIndex*interval),nM,nD,nH,nMin,nS);
		nIndex++;
	}
	dtRepeatEndDate = dtLast;

	return true;
}
GSM_Error MOTOE2_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header,int Timezone)
{
 	int 	Text, Time, Alarm, Phone, Recurrance, RecurranceFreq, EndTime, Location,RepeatEndDate,Description,DayofweekMask;
//	char 	buffer[2000];

	GSM_CalendarFindEntryIndex(note, &Text, &Time, &Alarm, &Phone, &Recurrance,&RecurranceFreq, &EndTime, &Location,&RepeatEndDate,&Description,&DayofweekMask);

	if (header) {
		*Length+=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);
		*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c",13,10);
	//	*Length+=sprintf(Buffer+(*Length), "TZ:%d%c%c",Timezone,13,10);

	}
	*Length+=sprintf(Buffer+(*Length), "BEGIN:VEVENT%c%c",13,10);

//	if (Version == Nokia_VCalendar) 
	{
		if(note->Type!=GSM_CAL_MEETING)
		{
			*Length+=sprintf(Buffer+(*Length), "CATEGORIES:");
			switch (note->Type) {
			case GSM_CAL_MEETING:
				*Length+=sprintf(Buffer+(*Length), "none%c%c",13,10);
				break;
			case GSM_CAL_REMINDER:
				*Length+=sprintf(Buffer+(*Length), "meeting%c%c",13,10);
				break;
			case GSM_CAL_CALL:
				*Length+=sprintf(Buffer+(*Length), "appointment%c%c",13,10);
				break;
			case GSM_CAL_BIRTHDAY:
				*Length+=sprintf(Buffer+(*Length), "presentation%c%c",13,10);
				break;
			case GSM_CAL_MEMO:
				*Length+=sprintf(Buffer+(*Length), "conference call%c%c",13,10);
				break;
			case GSM_CAL_TRAVEL:
				*Length+=sprintf(Buffer+(*Length), "breakfast%c%c",13,10);
				break;
			case GSM_CAL_VACATION:
				*Length+=sprintf(Buffer+(*Length), "lunch%c%c",13,10);
				break;
			case GSM_CAL_T_ATHL:
				*Length+=sprintf(Buffer+(*Length), "dinner%c%c",13,10);
				break;
			case GSM_CAL_T_CYCL:
				*Length+=sprintf(Buffer+(*Length), "anniversary%c%c",13,10);
				break;
			case GSM_CAL_T_BUDO:
				*Length+=sprintf(Buffer+(*Length), "birthday%c%c",13,10);
				break;
			case GSM_CAL_T_DANC:
				*Length+=sprintf(Buffer+(*Length), "flight%c%c",13,10);
				break;
			case GSM_CAL_T_EXTR:
				*Length+=sprintf(Buffer+(*Length), "party%c%c",13,10);
				break;
			case GSM_CAL_T_FOOT:
				*Length+=sprintf(Buffer+(*Length), "vacation%c%c",13,10);
				break;
			default:
				*Length+=sprintf(Buffer+(*Length), "none%c%c",13,10);
				break;
			}
		}

		if( Text!= -1 &&  UnicodeLength((unsigned char *)note->Entries[Text].Text) > 0)
		{
			unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
			CopyUnicodeString(utemp,note->Entries[Text].Text);
			UnicodeReplace(utemp,";","\\;");

			SESaveVCARDText(Buffer, Length,utemp, "SUMMARY",":");
		}

		if ( Location != -1 &&  UnicodeLength((unsigned char *)note->Entries[Location].Text) > 0) 
		{
			unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
			CopyUnicodeString(utemp,note->Entries[Location].Text);
			UnicodeReplace(utemp,";","\\;");
			SESaveVCARDText(Buffer, Length, utemp, "LOCATION",":");
		}
		if (Description != -1&&  UnicodeLength((unsigned char *)note->Entries[Description].Text) > 0) 
		{
			unsigned char utemp[(MAX_CALENDAR_TEXT_LENGTH + 1)*2];
			CopyUnicodeString(utemp,note->Entries[Description].Text);
			UnicodeReplace(utemp,";","\\;");
			SESaveVCARDText(Buffer, Length, utemp, "DESCRIPTION",":");
		}
	
		if (Time == -1) return ERR_UNKNOWN;
		GSM_DateTime     start_Date = note->Entries[Time].Date;

		if(Timezone == 0)
			SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[Time].Date, "DTSTART");
		else
		{
			GSM_DateTime gsmDateTime;
			memcpy(&gsmDateTime,&note->Entries[Time].Date,sizeof(GSM_DateTime));

			if(Timezone>0)
				GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
			else 
				GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);
			SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DTSTART");
		}

		if (EndTime != -1)
		{
			if(Timezone == 0)
				SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[EndTime].Date, "DTEND");
			else
			{
				GSM_DateTime gsmDateTime;
				memcpy(&gsmDateTime,&note->Entries[EndTime].Date,sizeof(GSM_DateTime));
				if(Timezone>0)
					GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
				else 
					GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);
				SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DTEND");
			}

		}

		if (Alarm != -1) 
		{
			unsigned long diff;
			diff = GetDurationDifferenceTime(&note->Entries[Time].Date,  &note->Entries[Alarm].Date);
			if(diff>99)
			{
				if(diff!=1440 && diff!=10080)
				{
					if(diff>1440)
					{
						//1440+(10080-1440)/2 = 5760
						if(diff >=5760) diff = 10080;
						else diff = 1440;
					}
					else
					{
						//60+(1440-60)/2 = 750
						if(diff >=750) diff = 1440;
						else diff = 60;
					}
					GetTimeDifference(diff, &start_Date, false, 60);
					note->Entries[Alarm].Date = start_Date;

				}
			}
			else if(diff==0)
			{
				GetTimeDifference(1, &start_Date, false, 60);
				note->Entries[Alarm].Date = start_Date;
			}

			if(Timezone == 0)
				SaveVCALDateTimeWithoutTimeZone(Buffer, Length, &note->Entries[Alarm].Date, "DALARM");
			else
			{
				GSM_DateTime gsmDateTime;
				memcpy(&gsmDateTime,&note->Entries[Alarm].Date,sizeof(GSM_DateTime));
				if(Timezone>0)
					GetTimeDifference(Timezone*15, &gsmDateTime, false, 60);
				else 
					GetTimeDifference(-Timezone*15, &gsmDateTime, true, 60);
				SaveVCALDateTime(Buffer, Length, &gsmDateTime, "DALARM");
			}

		}

		/* Birthday is known to be recurranced */
		if (Recurrance != -1 /*&& note->Type != GSM_CAL_BIRTHDAY*/) 
		{
			int nFreq = 1;
			int nRepeatCount=0;
			if(RecurranceFreq!= -1) nFreq = note->Entries[RecurranceFreq].Number;
			COleDateTime dtStart,dtRepeatEndDate;
			dtStart.SetDateTime(note->Entries[Time].Date.Year,note->Entries[Time].Date.Month,note->Entries[Time].Date.Day,
				note->Entries[Time].Date.Hour,note->Entries[Time].Date.Minute,note->Entries[Time].Date.Second);
			if(RepeatEndDate!=-1)
			{
				if(note->Entries[RepeatEndDate].Date.Year>2037)
					note->Entries[RepeatEndDate].Date.Year = 2037;
				dtRepeatEndDate.SetDateTime(note->Entries[RepeatEndDate].Date.Year,note->Entries[RepeatEndDate].Date.Month,note->Entries[RepeatEndDate].Date.Day,
					note->Entries[RepeatEndDate].Date.Hour,note->Entries[RepeatEndDate].Date.Minute,note->Entries[RepeatEndDate].Date.Second);
			}

			switch(note->Entries[Recurrance].Number)
			{
				case CAL_REPEAT_DAILY	 : 
					if(RepeatEndDate!=-1)
					{
						GetRepeatDatesCount_Daily(dtStart,nFreq,&nRepeatCount ,dtRepeatEndDate);
						note->Entries[RepeatEndDate].Date.Year = dtRepeatEndDate.GetYear();
						note->Entries[RepeatEndDate].Date.Month = dtRepeatEndDate.GetMonth();
						note->Entries[RepeatEndDate].Date.Day = dtRepeatEndDate.GetDay();

					}
					*Length+=sprintf(Buffer+(*Length), "RRULE:D%d #%d",nFreq,nRepeatCount);
			//		*Length+=sprintf(Buffer+(*Length), "RRULE:D%d ",nFreq);
					break;
				case CAL_REPEAT_WEEKLY	 : 
					if(RepeatEndDate!=-1)
					{
						if(DayofweekMask == -1)
							GetRepeatDatesCount_Weekly(dtStart,nFreq,&nRepeatCount ,dtRepeatEndDate,0);
						else
							GetRepeatDatesCount_Weekly(dtStart,nFreq,&nRepeatCount ,dtRepeatEndDate,note->Entries[DayofweekMask].Number);
						note->Entries[RepeatEndDate].Date.Year = dtRepeatEndDate.GetYear();
						note->Entries[RepeatEndDate].Date.Month = dtRepeatEndDate.GetMonth();
						note->Entries[RepeatEndDate].Date.Day = dtRepeatEndDate.GetDay();

					}
					if(DayofweekMask == -1)
				//		*Length+=sprintf(Buffer+(*Length), "RRULE:W%d ",nFreq);
						*Length+=sprintf(Buffer+(*Length), "RRULE:W%d #%d",nFreq,nRepeatCount);
					else 
					{
						char szDay[MAX_PATH];
						int nMask = note->Entries[DayofweekMask].Number;
						szDay[0] = '\0';
						if(nMask & 1) strcat(szDay ,"SU ");
						if(nMask & 2) strcat(szDay ,"MO ");
						if(nMask & 4) strcat(szDay ,"TU ");
						if(nMask & 8) strcat(szDay ,"WE ");
						if(nMask & 16) strcat(szDay ,"TH ");
						if(nMask & 32) strcat(szDay ,"FR ");
						if(nMask & 64) strcat(szDay ,"SA ");
						*Length+=sprintf(Buffer+(*Length), "RRULE:W%d %s#%d",nFreq,szDay,nRepeatCount);
					//	*Length+=sprintf(Buffer+(*Length), "RRULE:W%d %s",nFreq,szDay);

					}
					break;
				case CAL_REPEAT_MONTHLY	 : 
					if(RepeatEndDate!=-1)
					{
						GetRepeatDatesCount_Monthly(dtStart,nFreq,&nRepeatCount ,dtRepeatEndDate);
						note->Entries[RepeatEndDate].Date.Year = dtRepeatEndDate.GetYear();
						note->Entries[RepeatEndDate].Date.Month = dtRepeatEndDate.GetMonth();
						note->Entries[RepeatEndDate].Date.Day = dtRepeatEndDate.GetDay();
					}
					*Length+=sprintf(Buffer+(*Length), "RRULE:MD%d #%d",nFreq,nRepeatCount);
				//	*Length+=sprintf(Buffer+(*Length), "RRULE:MD%d ",nFreq);
					break;
				case CAL_REPEAT_MONTH_WEEKLY	 : 
					{
						UINT whichWeek,  whichDay;
						GetwhichWeekDay(note->Entries[Time].Date,whichWeek,whichDay);
						if(RepeatEndDate!=-1)
						{
							GetRepeatDatesCount_MonthNth(dtStart,nFreq,&nRepeatCount ,dtRepeatEndDate,whichWeek,whichDay);
							note->Entries[RepeatEndDate].Date.Year = dtRepeatEndDate.GetYear();
							note->Entries[RepeatEndDate].Date.Month = dtRepeatEndDate.GetMonth();
							note->Entries[RepeatEndDate].Date.Day = dtRepeatEndDate.GetDay();
						}
						char szday[MAX_PATH];
						switch(whichDay)
						{
						case 0:
							wsprintf(szday,"SU");
							break;
						case 1:
							wsprintf(szday,"MO");
							break;
						case 2:
							wsprintf(szday,"TU");
							break;
						case 3:
							wsprintf(szday,"WE");
							break;
						case 4:
							wsprintf(szday,"TH");
							break;
						case 5:
							wsprintf(szday,"FR");
							break;
						case 6:
							wsprintf(szday,"SA");
							break;
						}

						if(whichWeek == 5) 
					//		*Length+=sprintf(Buffer+(*Length), "RRULE:MP%d %d- %s ",nFreq,1,szday);
							*Length+=sprintf(Buffer+(*Length), "RRULE:MP%d %d- %s #%d",nFreq,1,szday,nRepeatCount);
						else
					//		*Length+=sprintf(Buffer+(*Length), "RRULE:MP%d %d+ %s ",nFreq,whichWeek,szday);
							*Length+=sprintf(Buffer+(*Length), "RRULE:MP%d %d+ %s #%d",nFreq,whichWeek,szday,nRepeatCount);
					}
					break;
				case CAL_REPEAT_YEARLY : 
					nFreq = 1;
					if(RecurranceFreq!= -1) note->Entries[RecurranceFreq].Number =1;
					if(RepeatEndDate!=-1)
					{
						GetRepeatDatesCount_Yearly(dtStart,nFreq,&nRepeatCount ,dtRepeatEndDate);
						note->Entries[RepeatEndDate].Date.Year = dtRepeatEndDate.GetYear();
						note->Entries[RepeatEndDate].Date.Month = dtRepeatEndDate.GetMonth();
						note->Entries[RepeatEndDate].Date.Day = dtRepeatEndDate.GetDay();

					}
					*Length+=sprintf(Buffer+(*Length), "RRULE:YM%d #%d",nFreq,nRepeatCount);
				//	*Length+=sprintf(Buffer+(*Length), "RRULE:YM%d ",nFreq);
					break;
			}
	/*		if(RepeatEndDate!=-1)
			{
				*Length+=sprintf(Buffer+(*Length), "%04d%02d%02dT%02d%02d%02d",
					note->Entries[RepeatEndDate].Date.Year,note->Entries[RepeatEndDate].Date.Month,note->Entries[RepeatEndDate].Date.Day,
					note->Entries[RepeatEndDate].Date.Hour,note->Entries[RepeatEndDate].Date.Minute,note->Entries[RepeatEndDate].Date.Second);
			}*/
			*Length+=sprintf(Buffer+(*Length), "%c%c",13,10);

		}
	}	
	*Length+=sprintf(Buffer+(*Length), "END:VEVENT%c%c",13,10);
	if (header) *Length+=sprintf(Buffer+(*Length), "END:VCALENDAR%c%c",13,10);

	return ERR_NONE;
}