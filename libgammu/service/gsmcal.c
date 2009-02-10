/* (c) 2002-2004 by Marcin Wiacek, 2005-2007 by Michal Cihar */

/**
 * @file gsmcal.c
 * @author Michal Čihař <michal@cihar.com>
 * @author Marcin Wiacek
 * @date 2002-2007
 */
/**
 * \addtogroup Calendar
 * @{
 */
#define _GNU_SOURCE
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <gammu-calendar.h>
#include <gammu-unicode.h>

#include "../misc/misc.h"
#include "../debug.h"
#include "gsmmisc.h"
#include "gsmcal.h"
#include "../misc/coding/coding.h"

void GSM_SetCalendarRecurranceRepeat(GSM_Debug_Info *di, unsigned char *rec, unsigned char *endday, GSM_CalendarEntry *entry)
{
	int i;
	int start=-1,frequency=-1,dow=-1,day=-1,month=-1,end=-1,Recurrance = 0, Repeat=0, j;
	GSM_DateTime	DT;
	time_t		t_time1,t_time2;

	rec[0] = 0;
	rec[1] = 0;

	for (j = 0; j < entry->EntriesNum; j++) {
		if (entry->Entries[j].EntryType == CAL_START_DATETIME)   start 		= j;
		if (entry->Entries[j].EntryType == CAL_REPEAT_FREQUENCY) frequency 	= j;
		if (entry->Entries[j].EntryType == CAL_REPEAT_DAYOFWEEK) dow 		= j;
		if (entry->Entries[j].EntryType == CAL_REPEAT_DAY)       day 		= j;
		if (entry->Entries[j].EntryType == CAL_REPEAT_MONTH)     month 		= j;
		if (entry->Entries[j].EntryType == CAL_REPEAT_STOPDATE)  end 		= j;
	}
	if (start == -1) return;

	if (frequency != -1 && dow == -1 && day == -1 && month == -1) {
		if (entry->Entries[frequency].Number == 1) {
			/* each day */
			Recurrance = 24;
		}
	}

	i = GetDayOfWeek(entry->Entries[start].Date.Year,
			entry->Entries[start].Date.Month,
			entry->Entries[start].Date.Day);

	if (frequency != -1 && dow != -1 && day == -1 && month == -1) {
		if (entry->Entries[frequency].Number == 1 &&
		    entry->Entries[dow].Number == i) {
			/* one week */
			Recurrance = 24*7;
		}
	}
	if (frequency != -1 && dow != -1 && day == -1 && month == -1) {
		if (entry->Entries[frequency].Number == 2 &&
		    entry->Entries[dow].Number == i) {
			/* two weeks */
			Recurrance = 24*14;
		}
	}
	if (frequency != -1 && dow == -1 && day != -1 && month == -1) {
		if (entry->Entries[frequency].Number == 1 &&
		    entry->Entries[day].Number == entry->Entries[start].Date.Day) {
			/* month */
			Recurrance = 0xffff-1;
		}
	}
	if (frequency != -1 && dow == -1 && day != -1 && month != -1) {
		if (entry->Entries[frequency].Number == 1 &&
		    entry->Entries[day].Number == entry->Entries[start].Date.Day &&
		    entry->Entries[month].Number == entry->Entries[start].Date.Month) {
			/* year */
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

	smfprintf(di, "Repeat number: %i\n",Repeat);
}

void GSM_GetCalendarRecurranceRepeat(GSM_Debug_Info *di, unsigned char *rec, unsigned char *endday, GSM_CalendarEntry *entry)
{
	int 	Recurrance,num=-1,i;

	Recurrance = rec[0]*256 + rec[1];
	if (Recurrance == 0) return;
	/* dct3 and dct4: 65535 (0xffff) is 1 year */
	if (Recurrance == 0xffff) Recurrance=24*365;
	/* dct3: unavailable, dct4: 65534 (0xffff-1) is 30 days */
	if (Recurrance == 0xffff-1) Recurrance=24*30;
	smfprintf(di, "Recurrance   : %i hours\n",Recurrance);

	for (i=0;i<entry->EntriesNum;i++) {
		if (entry->Entries[i].EntryType == CAL_START_DATETIME) {
			num = i;
			break;
		}
	}
	if (num == -1) return;

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
		entry->Entries[entry->EntriesNum].Number = GetDayOfWeek(entry->Entries[num].Date.Year,
			entry->Entries[num].Date.Month,
			entry->Entries[num].Date.Day);
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
	smfprintf(di, "Repeat   : %i times\n",endday[0]*256+endday[1]);
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
	smfprintf(di, "End Repeat Time: %04i-%02i-%02i %02i:%02i:%02i\n",
		entry->Entries[entry->EntriesNum-1].Date.Year,
		entry->Entries[entry->EntriesNum-1].Date.Month,
		entry->Entries[entry->EntriesNum-1].Date.Day,
		entry->Entries[entry->EntriesNum-1].Date.Hour,
		entry->Entries[entry->EntriesNum-1].Date.Minute,
		entry->Entries[entry->EntriesNum-1].Date.Second);
}

bool GSM_IsCalendarNoteFromThePast(GSM_CalendarEntry *note)
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
	GSM_SetCalendarRecurranceRepeat(NULL, rec, endday, note);
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


/**
 * Function to compute time difference between alarm and event time.
 */
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


GSM_Error GSM_Translate_Category (GSM_CatTranslation direction, char *string, GSM_CalendarNoteType *Type)
{
	/* Mozilla has user defined categories. These must be converted to GSM_CAL_xxx types.
	   TODO: For now we use hardcoded conversions. Should be user configurable. */

	switch (direction) {
	case TRANSL_TO_GSM:
		if (strstr(string,"MEETING")) 			*Type = GSM_CAL_MEETING;
		else if (strstr(string,"REMINDER")) 		*Type = GSM_CAL_REMINDER;
		else if (strstr(string,"DATE"))	 		*Type = GSM_CAL_REMINDER; /* SE */
		else if (strstr(string,"TRAVEL"))	 	*Type = GSM_CAL_TRAVEL;   /* SE */
		else if (strstr(string,"VACATION"))	 	*Type = GSM_CAL_VACATION; /* SE */
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
			case GSM_CAL_CALL:
				strcpy(string, "PHONE CALL");
				break;
			case GSM_CAL_MEETING:
				strcpy(string, "MEETING");
				break;
			case GSM_CAL_REMINDER:
				strcpy(string, "DATE");
				break;
			case GSM_CAL_TRAVEL:
				strcpy(string, "TRAVEL");
				break;
			case GSM_CAL_VACATION:
				strcpy(string, "VACATION");
				break;
			case GSM_CAL_BIRTHDAY:
				strcpy(string, "ANNIVERSARY");
				break;
			case GSM_CAL_MEMO:
			default:
				strcpy(string, "MISCELLANEOUS");
				break;
		}
		break;
	}
	return 0;
}

/**
 * Grabs single value of type from calendar note starting with record
 * start.
 */
GSM_Error GSM_Calendar_GetValue(const GSM_CalendarEntry *note, int *start, const GSM_CalendarType type, int *number, GSM_DateTime *date)
{
	for (; *start < note->EntriesNum; (*start)++) {
		if (note->Entries[*start].EntryType == type) {
			if (number != NULL) {
				*number = note->Entries[*start].Number;
			}
			if (date != NULL) {
				*date = note->Entries[*start].Date;
			}
			(*start)++;
			return ERR_NONE;
		}
	}
	return ERR_EMPTY;
}

/**
 * Converts Gammu recurrence to vCal format. See GSM_DecodeVCAL_RRULE
 * for grammar description.
 */
GSM_Error GSM_EncodeVCAL_RRULE(char *Buffer, const size_t buff_len, size_t *Length, GSM_CalendarEntry *note, int TimePos UNUSED, GSM_VCalendarVersion Version)
{
	int i;
	int j;
	const char *DaysOfWeek[8] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA", "SU"};
	bool repeating = false;
	int repeat_dayofweek = -1;
	int repeat_day = -1;
	int repeat_dayofyear = -1;
	int repeat_weekofmonth = -1;
	int repeat_month = -1;
	int repeat_count = -1;
	int repeat_frequency = -1;
	bool header;
#if 0
	GSM_DateTime repeat_startdate = {0,0,0,0,0,0,0};
#endif
	GSM_DateTime repeat_stopdate = {0,0,0,0,0,0,0};
	GSM_Error error;

	/* First scan for entry, whether there is  recurrence at all */
	for (i = 0; i < note->EntriesNum; i++) {
		switch (note->Entries[i].EntryType) {
			/* We don't care about following here */
			case CAL_PRIVATE:
			case CAL_CONTACTID:
			case CAL_START_DATETIME:
			case CAL_END_DATETIME:
			case CAL_LAST_MODIFIED:
			case CAL_TONE_ALARM_DATETIME:
			case CAL_SILENT_ALARM_DATETIME:
			case CAL_TEXT:
			case CAL_DESCRIPTION:
			case CAL_PHONE:
			case CAL_LOCATION:
			case CAL_LUID:
				break;
			case CAL_REPEAT_DAYOFWEEK:
				repeat_dayofweek 	= note->Entries[i].Number;
				repeating 		= true;
				break;
			case CAL_REPEAT_DAY:
				repeat_day 		= note->Entries[i].Number;
				repeating 		= true;
				break;
			case CAL_REPEAT_DAYOFYEAR:
				repeat_dayofyear	= note->Entries[i].Number;
				repeating 		= true;
				break;
			case CAL_REPEAT_WEEKOFMONTH:
				repeat_weekofmonth 	= note->Entries[i].Number;
				repeating 		= true;
				break;
			case CAL_REPEAT_MONTH:
				repeat_month 		= note->Entries[i].Number;
				repeating 		= true;
				break;
			case CAL_REPEAT_FREQUENCY:
				repeat_frequency 	= note->Entries[i].Number;
				repeating 		= true;
				break;
			case CAL_REPEAT_COUNT:
				repeat_count	 	= note->Entries[i].Number;
				repeating 		= true;
				break;
			case CAL_REPEAT_STARTDATE:
#if 0
				repeat_startdate 	= note->Entries[i].Date;
				repeating 		= true;
#endif
				break;
			case CAL_REPEAT_STOPDATE:
				repeat_stopdate 	= note->Entries[i].Date;
				repeating 		= true;
				break;
		}
	}
	/* Did we found something? */
	if (repeating) {
		error = VC_Store(Buffer, buff_len, Length,  "RRULE:");
		if (error != ERR_NONE) return error;

		/* Safe fallback */
		if (repeat_frequency == -1) {
			repeat_frequency = 1;
		}

		if ((repeat_dayofyear != -1) || (Version == Siemens_VCalendar && repeat_day != -1 && repeat_month != -1)) {
			/* Yearly by day */
			if (Version == Mozilla_iCalendar) {
				error = VC_Store(Buffer, buff_len, Length,  "FREQ=YEARLY");
				if (error != ERR_NONE) return error;
			} else {
				error = VC_Store(Buffer, buff_len, Length,  "YD%d", repeat_frequency);
				if (error != ERR_NONE) return error;
			}

			/* Store month numbers */
			header = false;
			for (i = 0; i < note->EntriesNum; i++) {
				if (note->Entries[i].EntryType == CAL_REPEAT_DAYOFYEAR) {
					if (Version == Mozilla_iCalendar) {
						if (!header) {
							error = VC_Store(Buffer, buff_len, Length,  ";BYYEARDAY=%d", note->Entries[i].Number);
							if (error != ERR_NONE) return error;
							header = true;
						} else {
							error = VC_Store(Buffer, buff_len, Length,  ",%d", note->Entries[i].Number);
							if (error != ERR_NONE) return error;
						}
					} else {
						error = VC_Store(Buffer, buff_len, Length,  " %d", note->Entries[i].Number);
						if (error != ERR_NONE) return error;
					}
				}
			}
		} else if (repeat_day != -1 && repeat_month != -1) {
			/* Yearly by month and day */
			if (Version == Mozilla_iCalendar) {
				error = VC_Store(Buffer, buff_len, Length,  "FREQ=YEARLY");
				if (error != ERR_NONE) return error;
			} else {
				error = VC_Store(Buffer, buff_len, Length,  "YM%d", repeat_frequency);
				if (error != ERR_NONE) return error;
			}

			/* Store month numbers */
			header = false;
			for (i = 0; i < note->EntriesNum; i++) {
				if (note->Entries[i].EntryType == CAL_REPEAT_MONTH) {
					if (Version == Mozilla_iCalendar) {
						if (!header) {
							error = VC_Store(Buffer, buff_len, Length,  ";BYMONTH=%d", note->Entries[i].Number);
							if (error != ERR_NONE) return error;
							header = true;
						} else {
							error = VC_Store(Buffer, buff_len, Length,  ",%d", note->Entries[i].Number);
							if (error != ERR_NONE) return error;
						}
					} else {
						error = VC_Store(Buffer, buff_len, Length,  " %d", note->Entries[i].Number);
						if (error != ERR_NONE) return error;
					}
				}
			}
		} else if (repeat_day != -1) {
			/* Monthly by day */
			if (Version == Mozilla_iCalendar) {
				error = VC_Store(Buffer, buff_len, Length,  "FREQ=MONTHLY");
				if (error != ERR_NONE) return error;
			} else {
				error = VC_Store(Buffer, buff_len, Length,  "MD%d", repeat_frequency);
				if (error != ERR_NONE) return error;
			}

			/* Store day numbers */
			header = false;
			for (i = 0; i < note->EntriesNum; i++) {
				if (note->Entries[i].EntryType == CAL_REPEAT_DAY) {
					if (Version == Mozilla_iCalendar) {
						if (!header) {
							error = VC_Store(Buffer, buff_len, Length,  ";BYMONTHDAY=%d", note->Entries[i].Number);
							if (error != ERR_NONE) return error;
							header = true;
						} else {
							error = VC_Store(Buffer, buff_len, Length,  ",%d", note->Entries[i].Number);
							if (error != ERR_NONE) return error;
						}
					} else {
						error = VC_Store(Buffer, buff_len, Length,  " %d", note->Entries[i].Number);
						if (error != ERR_NONE) return error;
					}
				}
			}
		} else if (repeat_dayofweek != -1 && repeat_weekofmonth != -1) {
			/* Monthly by day and week */
			if (Version == Mozilla_iCalendar) {
				error = VC_Store(Buffer, buff_len, Length,  "FREQ=MONTHLY");
				if (error != ERR_NONE) return error;
			} else {
				error = VC_Store(Buffer, buff_len, Length,  "MP%d", repeat_frequency);
				if (error != ERR_NONE) return error;
			}

			/* Store week numbers and week days */
			if (Version != Mozilla_iCalendar) {
				for (i = 0; i < note->EntriesNum; i++) {
					if (note->Entries[i].EntryType == CAL_REPEAT_WEEKOFMONTH) {
						error = VC_Store(Buffer, buff_len, Length,  " %d+", note->Entries[i].Number);
						if (error != ERR_NONE) return error;
						for (j = 0; j < note->EntriesNum; j++) {
							if (note->Entries[j].EntryType == CAL_REPEAT_DAYOFWEEK) {
								error = VC_Store(Buffer, buff_len, Length,  " %s", DaysOfWeek[note->Entries[j].Number]);
								if (error != ERR_NONE) return error;
							}
						}
					}
				}
			} else {
				header = false;
				for (i = 0; i < note->EntriesNum; i++) {
					if (note->Entries[i].EntryType == CAL_REPEAT_WEEKOFMONTH) {
						if (!header) {
							error = VC_Store(Buffer, buff_len, Length,  ";BYSETPOS=%d", note->Entries[i].Number);
							if (error != ERR_NONE) return error;
							header = true;
						} else {
							error = VC_Store(Buffer, buff_len, Length,  ",%d", note->Entries[i].Number);
							if (error != ERR_NONE) return error;
						}
					}
				}
				header = false;
				for (j = 0; j < note->EntriesNum; j++) {
					if (note->Entries[j].EntryType == CAL_REPEAT_DAYOFWEEK) {
						if (!header) {
							error = VC_Store(Buffer, buff_len, Length,  ";BYDAY=%s", DaysOfWeek[note->Entries[j].Number]);
							if (error != ERR_NONE) return error;
							header = true;
						} else {
							error = VC_Store(Buffer, buff_len, Length,  ",%s", DaysOfWeek[note->Entries[j].Number]);
							if (error != ERR_NONE) return error;
						}
					}
				}
			}
		} else if (repeat_dayofweek != -1) {
			/* Weekly by day */
			if (Version == Mozilla_iCalendar) {
				error = VC_Store(Buffer, buff_len, Length,  "FREQ=WEEKLY");
				if (error != ERR_NONE) return error;
			} else {
				error = VC_Store(Buffer, buff_len, Length,  "W%d", repeat_frequency);
				if (error != ERR_NONE) return error;
			}

			/* Store week days */
			header = false;
			for (i = 0; i < note->EntriesNum; i++) {
				if (note->Entries[i].EntryType == CAL_REPEAT_DAYOFWEEK) {
					if (Version == Mozilla_iCalendar) {
						if (!header) {
							error = VC_Store(Buffer, buff_len, Length,  ";BYDAY=%s", DaysOfWeek[note->Entries[i].Number]);
							if (error != ERR_NONE) return error;
							header = true;
						} else {
							error = VC_Store(Buffer, buff_len, Length,  ",%s", DaysOfWeek[note->Entries[i].Number]);
							if (error != ERR_NONE) return error;
						}
					} else {
						error = VC_Store(Buffer, buff_len, Length,  " %s", DaysOfWeek[note->Entries[i].Number]);
						if (error != ERR_NONE) return error;
					}
				}
			}
		} else {
			/* Daily */
			if (Version == Mozilla_iCalendar) {
				error = VC_Store(Buffer, buff_len, Length,  "FREQ=DAILY");
				if (error != ERR_NONE) return error;
			} else {
				error = VC_Store(Buffer, buff_len, Length,  "D%d", repeat_frequency);
				if (error != ERR_NONE) return error;
			}
		}

		/* Store frequency */
		if (Version == Mozilla_iCalendar && repeat_frequency > 1) {
			error = VC_Store(Buffer, buff_len, Length,  ";INTERVAL=%d", repeat_frequency);
			if (error != ERR_NONE) return error;
		}

		/* Store number of repetitions if available */
		if (repeat_count != -1) {
			if (Version == Mozilla_iCalendar) {
				if (repeat_count > 0) {
					error = VC_Store(Buffer, buff_len, Length,  ";COUNT=%d", repeat_count);
					if (error != ERR_NONE) return error;
				}
			} else {
				error = VC_Store(Buffer, buff_len, Length,  " #%d", repeat_count);
				if (error != ERR_NONE) return error;
			}
		}

		/* Store end of repetition date if available */
		if (repeat_stopdate.Day != 0) {
			if (Version == Mozilla_iCalendar) {
				error = VC_Store(Buffer, buff_len, Length,  ";UNTIL=");
				if (error != ERR_NONE) return error;
			}
			error = VC_StoreDate(Buffer, buff_len, Length, &repeat_stopdate, NULL);
			if (error != ERR_NONE) return error;
		} else {
			/* Add EOL */
			error = VC_StoreLine(Buffer, buff_len, Length,  "");
			if (error != ERR_NONE) return error;
		}

		return ERR_NONE;
	}
	return ERR_EMPTY;
}

/**
 * Adjusts all datetime information in calendar entry according to delta.
 */
void GSM_Calendar_AdjustDate(GSM_CalendarEntry *note, GSM_DeltaTime *delta)
{
	int i;

	/* Loop over entries */
	for (i=0; i < note->EntriesNum; i++) {
		switch (note->Entries[i].EntryType) {
			case CAL_START_DATETIME :
			case CAL_END_DATETIME :
			case CAL_TONE_ALARM_DATETIME :
			case CAL_SILENT_ALARM_DATETIME:
			case CAL_LAST_MODIFIED:
			case CAL_REPEAT_STARTDATE:
			case CAL_REPEAT_STOPDATE:
				note->Entries[i].Date = GSM_AddTime(note->Entries[i].Date, *delta);
				break;
			case CAL_TEXT:
			case CAL_DESCRIPTION:
			case CAL_PHONE:
			case CAL_LOCATION:
			case CAL_LUID:
			case CAL_REPEAT_DAYOFWEEK:
			case CAL_REPEAT_DAY:
			case CAL_REPEAT_WEEKOFMONTH:
			case CAL_REPEAT_MONTH:
			case CAL_REPEAT_FREQUENCY:
			case CAL_REPEAT_DAYOFYEAR:
			case CAL_REPEAT_COUNT:
			case CAL_PRIVATE:
			case CAL_CONTACTID:
				/* No need to care */
				break;
		}
	}
}

void GSM_ToDo_AdjustDate(GSM_ToDoEntry *note, GSM_DeltaTime *delta)
{
	int i;

	/* Loop over entries */
	for (i=0; i < note->EntriesNum; i++) {
		switch (note->Entries[i].EntryType) {
			case TODO_END_DATETIME :
			case TODO_ALARM_DATETIME :
			case TODO_SILENT_ALARM_DATETIME:
			case TODO_LAST_MODIFIED:
			case TODO_START_DATETIME:
				note->Entries[i].Date = GSM_AddTime(note->Entries[i].Date, *delta);
				break;
			case TODO_TEXT:
			case TODO_DESCRIPTION:
			case TODO_PHONE:
			case TODO_LOCATION:
			case TODO_LUID:
			case TODO_PRIVATE:
			case TODO_COMPLETED:
			case TODO_CONTACTID:
			case TODO_CATEGORY:
				/* No need to care */
				break;
		}
	}
}

GSM_Error GSM_EncodeVCALENDAR(char *Buffer, const size_t buff_len, size_t *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version)
{
	GSM_DateTime 	deltatime;
	char 		dtstr[20];
	char		category[100];
	int		i, alarm_pos = -1, date_pos = -1;
	GSM_Error error;

	/* Write header */
	if (header) {
		error = VC_StoreLine(Buffer, buff_len, Length,  "BEGIN:VCALENDAR");
		if (error != ERR_NONE) return error;
		error = VC_StoreLine(Buffer, buff_len, Length,  "VERSION:%s", Version == Mozilla_iCalendar ? "2.0" : "1.0");
		if (error != ERR_NONE) return error;
	}
	error = VC_StoreLine(Buffer, buff_len, Length,  "BEGIN:VEVENT");
	if (error != ERR_NONE) return error;

	if (Version == Mozilla_iCalendar) {
		/* Mozilla Calendar needs UIDs. http://www.innerjoin.org/iCalendar/events-and-uids.html */
		error = VC_StoreLine(Buffer, buff_len, Length,  "UID:calendar-%i",note->Location);
		if (error != ERR_NONE) return error;
		error = VC_StoreLine(Buffer, buff_len, Length,  "STATUS:CONFIRMED");
		if (error != ERR_NONE) return error;
	}

	/* Store category */
	GSM_Translate_Category(TRANSL_TO_VCAL, category, &note->Type);
	error = VC_StoreLine(Buffer, buff_len, Length,  "CATEGORIES:%s", category);
	if (error != ERR_NONE) return error;

	/* Loop over entries */
	for (i=0; i < note->EntriesNum; i++) {
		switch (note->Entries[i].EntryType) {
			case CAL_START_DATETIME :
				date_pos = i;
				if (Version == Mozilla_iCalendar && (note->Type == GSM_CAL_MEMO || note->Type == GSM_CAL_BIRTHDAY)) {
					error = VC_StoreDate(Buffer, buff_len, Length, &note->Entries[i].Date, "DTSTART;VALUE=DATE");
					if (error != ERR_NONE) return error;
				} else {
					error = VC_StoreDateTime(Buffer, buff_len, Length, &note->Entries[i].Date, "DTSTART");
					if (error != ERR_NONE) return error;
				}
				break;
			case CAL_END_DATETIME :
				if (Version == Mozilla_iCalendar && (note->Type == GSM_CAL_MEMO || note->Type == GSM_CAL_BIRTHDAY)) {
					error = VC_StoreDate(Buffer, buff_len, Length, &note->Entries[i].Date, "DTEND;VALUE=DATE");
					if (error != ERR_NONE) return error;
				} else {
					error = VC_StoreDateTime(Buffer, buff_len, Length, &note->Entries[i].Date, "DTEND");
					if (error != ERR_NONE) return error;
				}
				break;
			case CAL_TONE_ALARM_DATETIME :
				alarm_pos = i;
				/* Disable alarm for birthday entries. Mozilla would generate an alarm before birth! */
				if (Version != Mozilla_iCalendar || note->Type != GSM_CAL_BIRTHDAY) {
					error = VC_StoreDateTime(Buffer, buff_len, Length, &note->Entries[i].Date, "AALARM");
					if (error != ERR_NONE) return error;
				}
				break;
			case CAL_SILENT_ALARM_DATETIME:
				alarm_pos = i;
				/* Disable alarm for birthday entries. Mozilla would generate an alarm before birth! */
				if (Version != Mozilla_iCalendar || note->Type != GSM_CAL_BIRTHDAY) {
					error = VC_StoreDateTime(Buffer, buff_len, Length, &note->Entries[i].Date, "DALARM");
					if (error != ERR_NONE) return error;
				}
				break;
			case CAL_LAST_MODIFIED:
				if (Version == Mozilla_iCalendar && (note->Type == GSM_CAL_MEMO || note->Type == GSM_CAL_BIRTHDAY)) {
					error = VC_StoreDate(Buffer, buff_len, Length, &note->Entries[i].Date, "LAST-MODIFIED;VALUE=DATE");
					if (error != ERR_NONE) return error;
				} else {
					error = VC_StoreDateTime(Buffer, buff_len, Length, &note->Entries[i].Date, "LAST-MODIFIED");
					if (error != ERR_NONE) return error;
				}
				break;
			case CAL_TEXT:
				error = VC_StoreText(Buffer, buff_len, Length, note->Entries[i].Text, "SUMMARY", Version == Mozilla_iCalendar);
				if (error != ERR_NONE) return error;
				break;
			case CAL_DESCRIPTION:
				error = VC_StoreText(Buffer, buff_len, Length, note->Entries[i].Text, "DESCRIPTION", Version == Mozilla_iCalendar);
				if (error != ERR_NONE) return error;
				break;
			case CAL_PHONE:
				/* There is no specific field for phone number, use description */
				error = VC_StoreText(Buffer, buff_len, Length, note->Entries[i].Text, "DESCRIPTION", Version == Mozilla_iCalendar);
				if (error != ERR_NONE) return error;
				break;
			case CAL_LOCATION:
				error = VC_StoreText(Buffer, buff_len, Length, note->Entries[i].Text, "LOCATION", Version == Mozilla_iCalendar);
				if (error != ERR_NONE) return error;
				break;
			case CAL_LUID:
				error = VC_StoreText(Buffer, buff_len, Length, note->Entries[i].Text, "X-IRMC-LUID", Version == Mozilla_iCalendar);
				if (error != ERR_NONE) return error;
				break;
			case CAL_REPEAT_DAYOFWEEK:
			case CAL_REPEAT_DAY:
			case CAL_REPEAT_WEEKOFMONTH:
			case CAL_REPEAT_MONTH:
			case CAL_REPEAT_FREQUENCY:
			case CAL_REPEAT_STARTDATE:
			case CAL_REPEAT_STOPDATE:
			case CAL_REPEAT_DAYOFYEAR:
			case CAL_REPEAT_COUNT:
				/* Handled later */
				break;
			case CAL_PRIVATE:
				if (note->Entries[i].Number == 0) {
					error = VC_StoreLine(Buffer, buff_len, Length,  "CLASS:PUBLIC");
					if (error != ERR_NONE) return error;
				} else {
					error = VC_StoreLine(Buffer, buff_len, Length,  "CLASS:PRIVATE");
					if (error != ERR_NONE) return error;
				}
				break;
			case CAL_CONTACTID:
				/* Not supported */
				break;
		}
	}

	/* Handle recurrance */
	if (note->Type == GSM_CAL_BIRTHDAY) {
		if (Version == Mozilla_iCalendar) {
			error = VC_StoreLine(Buffer, buff_len, Length,  "X-MOZILLA-RECUR-DEFAULT-UNITS:years");
			if (error != ERR_NONE) return error;
		} else if (Version == Siemens_VCalendar) {
			error = VC_StoreLine(Buffer, buff_len, Length,  "RRULE:YD1");
			if (error != ERR_NONE) return error;
		} else {
			error = VC_StoreLine(Buffer, buff_len, Length,  "RRULE:YM1");
			if (error != ERR_NONE) return error;
		}
	} else {
		error = GSM_EncodeVCAL_RRULE(Buffer, buff_len, Length, note, date_pos, Version);
		if (error != ERR_NONE && error != ERR_EMPTY) return error;
	}

	/* Include mozilla specific alarm encoding */
	if (Version == Mozilla_iCalendar && alarm_pos != -1 && date_pos != -1) {
		deltatime = VCALTimeDiff(&note->Entries[alarm_pos].Date, &note->Entries[date_pos].Date);

		dtstr[0]='\0';
		if (deltatime.Minute !=0) {
			error = VC_StoreLine(Buffer, buff_len, Length,  "X-MOZILLA-ALARM-DEFAULT-UNITS:minutes");
			if (error != ERR_NONE) return error;
			error = VC_StoreLine(Buffer, buff_len, Length,  "X-MOZILLA-ALARM-DEFAULT-LENGTH:%i", deltatime.Minute);
			if (error != ERR_NONE) return error;
			sprintf(dtstr,"-PT%iM",deltatime.Minute);
		} else if (deltatime.Hour !=0) {
			error = VC_StoreLine(Buffer, buff_len, Length,  "X-MOZILLA-ALARM-DEFAULT-UNITS:hours");
			if (error != ERR_NONE) return error;
			error = VC_StoreLine(Buffer, buff_len, Length,  "X-MOZILLA-ALARM-DEFAULT-LENGTH:%i", deltatime.Hour);
			if (error != ERR_NONE) return error;
			sprintf(dtstr,"-PT%iH",deltatime.Hour);
		} else if (deltatime.Day !=0) {
			error = VC_StoreLine(Buffer, buff_len, Length,  "X-MOZILLA-ALARM-DEFAULT-UNITS:days");
			if (error != ERR_NONE) return error;
			error = VC_StoreLine(Buffer, buff_len, Length,  "X-MOZILLA-ALARM-DEFAULT-LENGTH:%i", deltatime.Day);
			if (error != ERR_NONE) return error;
			sprintf(dtstr,"-P%iD",deltatime.Day);
		}
		if (dtstr[0] != '\0') {
			error = VC_StoreLine(Buffer, buff_len, Length,  "BEGIN:VALARM");
			if (error != ERR_NONE) return error;
			error = VC_StoreLine(Buffer, buff_len, Length,  "TRIGGER;VALUE=DURATION");
			if (error != ERR_NONE) return error;
			/**
			 * @todo this looks wrong!
			 */
			error = VC_StoreLine(Buffer, buff_len, Length,  " :%s",dtstr);
			if (error != ERR_NONE) return error;
			error = VC_StoreLine(Buffer, buff_len, Length,  "END:VALARM");
			if (error != ERR_NONE) return error;
		}
	}

	error = VC_StoreLine(Buffer, buff_len, Length,  "END:VEVENT");
	if (error != ERR_NONE) return error;
	if (header) {
		error = VC_StoreLine(Buffer, buff_len, Length,  "END:VCALENDAR");
		if (error != ERR_NONE) return error;
	}

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

GSM_Error GSM_EncodeVTODO(char *Buffer, const size_t buff_len, size_t *Length, GSM_ToDoEntry *note, bool header, GSM_VToDoVersion Version)
{
	char		category[100];
	int		i;
	GSM_Error error;

	/* Write header */
	if (header) {
		error = VC_StoreLine(Buffer, buff_len, Length,  "BEGIN:VCALENDAR");
		if (error != ERR_NONE) return error;
		error = VC_StoreLine(Buffer, buff_len, Length,  "VERSION:%s", Version == Mozilla_VToDo ? "2.0" : "1.0");
		if (error != ERR_NONE) return error;
	}
	error = VC_StoreLine(Buffer, buff_len, Length,  "BEGIN:VTODO");
	if (error != ERR_NONE) return error;

	if (Version == Mozilla_iCalendar) {
		/* Mozilla Calendar needs UIDs. http://www.innerjoin.org/iCalendar/events-and-uids.html */
		error = VC_StoreLine(Buffer, buff_len, Length,  "UID:calendar-%i",note->Location);
		if (error != ERR_NONE) return error;
		error = VC_StoreLine(Buffer, buff_len, Length,  "STATUS:CONFIRMED");
		if (error != ERR_NONE) return error;
	}

	if (Version == Mozilla_VToDo) {
		switch (note->Priority) {
			case GSM_Priority_None	:
				error = VC_StoreLine(Buffer, buff_len, Length,  "PRIORITY:0");
				if (error != ERR_NONE) return error;
				break;
			case GSM_Priority_Low	:
				error = VC_StoreLine(Buffer, buff_len, Length,  "PRIORITY:1");
				if (error != ERR_NONE) return error;
				break;
			case GSM_Priority_Medium:
				error = VC_StoreLine(Buffer, buff_len, Length,  "PRIORITY:5");
				if (error != ERR_NONE) return error;
				break;
			case GSM_Priority_High	:
				error = VC_StoreLine(Buffer, buff_len, Length,  "PRIORITY:9");
				if (error != ERR_NONE) return error;
				break;
		}
	} else {
		switch (note->Priority) {
			case GSM_Priority_None	:
				error = VC_StoreLine(Buffer, buff_len, Length,  "PRIORITY:0");
				if (error != ERR_NONE) return error;
				break;
			case GSM_Priority_Low	:
				error = VC_StoreLine(Buffer, buff_len, Length,  "PRIORITY:1");
				if (error != ERR_NONE) return error;
				break;
			case GSM_Priority_Medium:
				error = VC_StoreLine(Buffer, buff_len, Length,  "PRIORITY:2");
				if (error != ERR_NONE) return error;
				break;
			case GSM_Priority_High	:
				error = VC_StoreLine(Buffer, buff_len, Length,  "PRIORITY:3");
				if (error != ERR_NONE) return error;
				break;
		}
	}
	/* Store category */
	GSM_Translate_Category(TRANSL_TO_VCAL, category, &note->Type);
	error = VC_StoreLine(Buffer, buff_len, Length,  "CATEGORIES:%s", category);
	if (error != ERR_NONE) return error;

	/* Loop over entries */
	for (i=0; i < note->EntriesNum; i++) {
		switch (note->Entries[i].EntryType) {
			case TODO_END_DATETIME :
				if (note->Entries[i].Date.Year   != 2037	&&
				    note->Entries[i].Date.Month  != 12	&&
				    note->Entries[i].Date.Day    != 31	&&
				    note->Entries[i].Date.Hour   != 23	&&
				    note->Entries[i].Date.Minute != 59 ) {
					error = VC_StoreDateTime(Buffer, buff_len, Length, &note->Entries[i].Date, "DUE");
					if (error != ERR_NONE) return error;
				}
				break;
			case TODO_ALARM_DATETIME :
				/* Disable alarm for birthday entries. Mozilla would generate an alarm before birth! */
				if (Version != Mozilla_iCalendar || note->Type != GSM_CAL_BIRTHDAY) {
					error = VC_StoreDateTime(Buffer, buff_len, Length, &note->Entries[i].Date, "AALARM");
					if (error != ERR_NONE) return error;
				}
				break;
			case TODO_SILENT_ALARM_DATETIME:
				/* Disable alarm for birthday entries. Mozilla would generate an alarm before birth! */
				if (Version != Mozilla_iCalendar || note->Type != GSM_CAL_BIRTHDAY) {
					error = VC_StoreDateTime(Buffer, buff_len, Length, &note->Entries[i].Date, "DALARM");
					if (error != ERR_NONE) return error;
				}
				break;
			case TODO_START_DATETIME:
				error = VC_StoreDateTime(Buffer, buff_len, Length, &note->Entries[i].Date, "DTSTART");
				if (error != ERR_NONE) return error;
				break;
			case TODO_LAST_MODIFIED:
				error = VC_StoreDateTime(Buffer, buff_len, Length, &note->Entries[i].Date, "LAST-MODIFIED");
				if (error != ERR_NONE) return error;
				break;
			case TODO_TEXT:
				error = VC_StoreText(Buffer, buff_len, Length, note->Entries[i].Text, "SUMMARY", Version == Mozilla_iCalendar);
				if (error != ERR_NONE) return error;
				break;
			case TODO_DESCRIPTION:
				error = VC_StoreText(Buffer, buff_len, Length, note->Entries[i].Text, "DESCRIPTION", Version == Mozilla_iCalendar);
				if (error != ERR_NONE) return error;
				break;
			case TODO_PHONE:
				/* There is no specific field for phone number, use description */
				error = VC_StoreText(Buffer, buff_len, Length, note->Entries[i].Text, "DESCRIPTION", Version == Mozilla_iCalendar);
				if (error != ERR_NONE) return error;
				break;
			case TODO_LOCATION:
				error = VC_StoreText(Buffer, buff_len, Length, note->Entries[i].Text, "LOCATION", Version == Mozilla_iCalendar);
				if (error != ERR_NONE) return error;
				break;
			case TODO_LUID:
				error = VC_StoreText(Buffer, buff_len, Length, note->Entries[i].Text, "X-IRMC-LUID", Version == Mozilla_iCalendar);
				if (error != ERR_NONE) return error;
				break;
			case TODO_PRIVATE:
				if (note->Entries[i].Number == 0) {
					error = VC_StoreLine(Buffer, buff_len, Length,  "CLASS:PUBLIC");
					if (error != ERR_NONE) return error;
				} else {
					error = VC_StoreLine(Buffer, buff_len, Length,  "CLASS:PRIVATE");
					if (error != ERR_NONE) return error;
				}
				break;
			case TODO_COMPLETED:
				if (note->Entries[i].Number == 1) {
					error = VC_StoreLine(Buffer, buff_len, Length,  "STATUS:COMPLETED");
					if (error != ERR_NONE) return error;
					error = VC_StoreLine(Buffer, buff_len, Length,  "PERCENT-COMPLETE:100");
					if (error != ERR_NONE) return error;
				} else {
					error = VC_StoreLine(Buffer, buff_len, Length,  "STATUS:NEEDS ACTION");
					if (error != ERR_NONE) return error;
				}
				break;
			case TODO_CONTACTID:
			case TODO_CATEGORY:
				/* Not supported */
				break;
		}
	}

	error = VC_StoreLine(Buffer, buff_len, Length,  "END:VTODO");
	if (error != ERR_NONE) return error;
	if (header) {
		error = VC_StoreLine(Buffer, buff_len, Length,  "END:VCALENDAR");
		if (error != ERR_NONE) return error;
	}

	return ERR_NONE;
}

GSM_TimeUnit ReadVCALTimeUnits (unsigned char *Buffer)
{
	if (strcasestr(Buffer,"days"))	return GSM_TimeUnit_Days;
	if (strcasestr(Buffer,"hours"))	return GSM_TimeUnit_Hours;
	if (strcasestr(Buffer,"minutes")) return GSM_TimeUnit_Minutes;
	if (strcasestr(Buffer,"seconds")) return GSM_TimeUnit_Seconds;
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

/**
 * Prepare input buffer (notably line continuations).
 */
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

/**
 * Decode day of week to gammu enumeration (1 = Monday...7 = Sunday).
 */
GSM_Error GSM_DecodeVCAL_DOW(const char *Buffer, int *Output)
{
	if (toupper(Buffer[0])== 'M' && toupper(Buffer[1]) == 'O') {
		*Output = 1;
		return ERR_NONE;
	} else if (toupper(Buffer[0])== 'T' && toupper(Buffer[1]) == 'U') {
		*Output = 2;
		return ERR_NONE;
	} else if (toupper(Buffer[0])== 'W' && toupper(Buffer[1]) == 'E') {
		*Output = 3;
		return ERR_NONE;
	} else if (toupper(Buffer[0])== 'T' && toupper(Buffer[1]) == 'H') {
		*Output = 4;
		return ERR_NONE;
	} else if (toupper(Buffer[0])== 'F' && toupper(Buffer[1]) == 'R') {
		*Output = 5;
		return ERR_NONE;
	} else if (toupper(Buffer[0])== 'S' && toupper(Buffer[1]) == 'A') {
		*Output = 6;
		return ERR_NONE;
	} else if (toupper(Buffer[0])== 'S' && toupper(Buffer[1]) == 'U') {
		*Output = 7;
		return ERR_NONE;
	}
	return ERR_UNKNOWN;
}

/**
 * Decodes vCalendar RRULE recurrance format into calendar entry. It
 * should be implemented according to following grammar:
 *
 * @code
 *   {}         0 or more
 *
 *   []         0 or 1
 *
 *   start      ::= <daily> [<enddate>] |
 *
 *               <weekly> [<enddate>] |
 *
 *               <monthlybypos> [<enddate>] |
 *
 *               <monthlybyday> [<enddate>] |
 *
 *               <yearlybymonth> [<enddate>] |
 *
 *               <yearlybyday> [<enddate>]
 *
 *   digit ::= <0|1|2|3|4|5|6|7|8|9>
 *
 *   digits ::= <digit> {<digits>}
 *
 *   enddate    ::= ISO 8601_date_time value(e.g., 19940712T101530Z)
 *
 *   interval   ::= <digits>
 *
 *   duration   ::= #<digits>
 *
 *   lastday    ::= LD
 *
 *   plus               ::= +
 *
 *   minus              ::= -
 *
 *   daynumber          ::= <1-31> [<plus>|<minus>]| <lastday>
 *
 *   daynumberlist      ::= daynumber {<daynumberlist>}
 *
 *   month              ::= <1-12>
 *
 *   monthlist  ::= <month> {<monthlist>}
 *
 *   day                ::= <1-366>
 *
 *   daylist            ::= <day> {<daylist>}
 *
 *   occurrence ::= <1-5><plus> | <1-5><minus>
 *
 *   weekday    ::= <SU|MO|TU|WE|TH|FR|SA>
 *
 *   weekdaylist        ::= <weekday> {<weekdaylist>}
 *
 *   occurrenceweekday  ::= [<occurrence>] <weekday>
 *
 *   occurenceweekdaylist       ::= <occurenceweekday>
 *
 *      {<occurenceweekdaylist>}
 *
 *   daily              ::= D<interval> [<duration>]
 *
 *   weekly             ::= W<interval> [<weekdaylist>] [<duration>]
 *
 *   monthlybypos       ::= MP<interval> [<occurrenceweekdaylist>]
 *
 *      [<duration>]
 *
 *   monthlybyday       ::= MD<interval> [<daynumberlist>] [<duration>]
 *
 *   yearlybymonth      ::= YM<interval> [<monthlist>] [<duration>]
 *
 *   yearlybyday        ::= YD<interval> [<daylist>] [<duration>]
 *
 * @endcode
 *
 * @li @b enddate      Controls when a repeating event terminates. The enddate
 *              is the last time an event can occur.
 *
 * @li @b Interval     Defines the frequency in which a rule repeats.
 *
 * @li @b duration     Controls the number of events a rule generates.
 *
 * @li @b Lastday      Can be used as a replacement to daynumber to indicate
 * the last day of the month.
 *
 * @li @b daynumber    A number representing a day of the month.
 *
 * @li @b month                A number representing a month of the year.
 *
 * @li @b day          A number representing a day of the year.
 *
 * @li @b occurrence   Controls which week of the month a particular weekday
 * event occurs.
 *
 * @li @b weekday      A symbol representing a day of the week.
 *
 * @li @b daily                Defines a rule that repeats on a daily basis.
 *
 * @li @b weekly               Defines a rule that repeats on a weekly basis.
 *
 * @li @b monthlybypos Defines a rule that repeats on a monthly basis on a
 * relative day and week.
 *
 * @li @b monthlybyday Defines a rule that repeats on a monthly basis on an
 * absolute day.
 *
 * @li @b yearlybymonth        Defines a rule that repeats on specific months
 * of the year.
 *
 * @li @b yearlybyday  Defines a rule that repeats on specific days of the
 * year.
 *
 * @todo Negative week of month and day of month are not supported.
 */
GSM_Error GSM_DecodeVCAL_RRULE(GSM_Debug_Info *di, const char *Buffer, GSM_CalendarEntry *Calendar, int TimePos)
{
	const char *pos = Buffer;
	bool have_info;

/* Skip spaces */
#define NEXT_NOSPACE(terminate) \
	while (isspace(*pos) && *pos) pos++; \
	if (terminate && *pos == 0) return ERR_NONE;
/* Skip numbers */
#define NEXT_NONUMBER(terminate) \
	while (isdigit(*pos) && *pos) pos++; \
	if (terminate && *pos == 0) return ERR_NONE;
/* Go to next char */
#define NEXT_CHAR(terminate) \
	if (*pos) pos++; \
	if (terminate && *pos == 0) return ERR_UNKNOWN;
/* Go to next char */
#define NEXT_CHAR_NOERR(terminate) \
	if (*pos) pos++; \
	if (terminate && *pos == 0) return ERR_NONE;

#define GET_DOW(type, terminate) \
	Calendar->Entries[Calendar->EntriesNum].EntryType = type; \
	Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE; \
	if (GSM_DecodeVCAL_DOW(pos, &(Calendar->Entries[Calendar->EntriesNum].Number)) != ERR_NONE) return ERR_UNKNOWN; \
	Calendar->EntriesNum++; \
	NEXT_CHAR(1); \
	NEXT_CHAR_NOERR(terminate);

#define GET_NUMBER(type, terminate) \
	Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE; \
	Calendar->Entries[Calendar->EntriesNum].EntryType = type; \
	Calendar->Entries[Calendar->EntriesNum].Number = atoi(pos); \
	Calendar->EntriesNum++; \
	NEXT_NONUMBER(terminate);

#define GET_FREQUENCY(terminate) \
	GET_NUMBER(CAL_REPEAT_FREQUENCY, terminate);

	/* This should not happen */
	if (TimePos == -1) {
		return ERR_UNKNOWN;
	}

	NEXT_NOSPACE(1);

	/* Detect primary rule type */
	switch (*pos) {
		/* Daily */
		case 'D':
			NEXT_CHAR(1);
			GET_FREQUENCY(1);
			break;
		/* Weekly */
		case 'W':
			NEXT_CHAR(1);
			GET_FREQUENCY(0);
			NEXT_NOSPACE(0);
			/* There might be now list of months, if there is none, we use date */
			have_info = false;

			while (isalpha(*pos)) {
				have_info = true;
				GET_DOW(CAL_REPEAT_DAYOFWEEK, 1);
				NEXT_NOSPACE(0);
			}

			if (!have_info) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_DAYOFWEEK;
				Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
				Calendar->Entries[Calendar->EntriesNum].Number =
					GetDayOfWeek(
						Calendar->Entries[TimePos].Date.Year,
						Calendar->Entries[TimePos].Date.Month,
						Calendar->Entries[TimePos].Date.Day);
				if (Calendar->Entries[Calendar->EntriesNum].Number == 0) {
					Calendar->Entries[Calendar->EntriesNum].Number = 7;
				}
				Calendar->EntriesNum++;
			}
			break;
		/* Monthly */
		case 'M':
			NEXT_CHAR(1);
			switch (*pos) {
				/* Monthly by position */
				case 'P':
					NEXT_CHAR(1);
					GET_FREQUENCY(0);
					NEXT_NOSPACE(0);
					if (isdigit(*pos)) {
						GET_NUMBER(CAL_REPEAT_WEEKOFMONTH, 0);
						if (*pos == '+') {
							pos++;
						} else if (*pos == '-') {
							pos++;
							smfprintf(di, "WARNING: Negative week position not supported!");
						}
						NEXT_NOSPACE(0);

						while (isalpha(*pos)) {
							have_info = true;
							GET_DOW(CAL_REPEAT_DAYOFWEEK, 0);
							NEXT_NOSPACE(0);
						}
					} else {
						/* Need to fill in info from current date */
						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_WEEKOFMONTH;
						Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
						Calendar->Entries[Calendar->EntriesNum].Number =
							GetWeekOfMonth(
								Calendar->Entries[TimePos].Date.Year,
								Calendar->Entries[TimePos].Date.Month,
								Calendar->Entries[TimePos].Date.Day);
						Calendar->EntriesNum++;

						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_DAYOFWEEK;
						Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
						Calendar->Entries[Calendar->EntriesNum].Number =
							GetDayOfWeek(
								Calendar->Entries[TimePos].Date.Year,
								Calendar->Entries[TimePos].Date.Month,
								Calendar->Entries[TimePos].Date.Day);
						if (Calendar->Entries[Calendar->EntriesNum].Number == 0) {
							Calendar->Entries[Calendar->EntriesNum].Number = 7;
						}
						Calendar->EntriesNum++;
					}
					break;
				/* Monthly by day */
				case 'D':
					NEXT_CHAR(1);
					GET_FREQUENCY(0);
					NEXT_NOSPACE(0);
					if (isdigit(*pos)) {
						while (isdigit(*pos)) {
							GET_NUMBER(CAL_REPEAT_DAY, 0);
							if (*pos == '+') {
								pos++;
							} else if (*pos == '-') {
								pos++;
								smfprintf(di, "WARNING: Negative day position not supported!");
							}
							NEXT_NOSPACE(0);
						}
					} else {
						/* Need to fill in info from current date */
						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_DAY;
						Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
						Calendar->Entries[Calendar->EntriesNum].Number = Calendar->Entries[TimePos].Date.Day;
						Calendar->EntriesNum++;
					}

					break;
				default:
					smfprintf(di, "Could not decode recurrency: %s\n", pos);
					return ERR_UNKNOWN;
			}
			break;
		/* Yearly */
		case 'Y':
			NEXT_CHAR(1);
			switch (*pos) {
				/* Yearly by month */
				case 'M':
					NEXT_CHAR(1);
					GET_FREQUENCY(0);
					NEXT_NOSPACE(0);
					/* We need date of event */
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_DAY;
					Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
					Calendar->Entries[Calendar->EntriesNum].Number =
						Calendar->Entries[TimePos].Date.Day;
					Calendar->EntriesNum++;
					/* There might be now list of months, if there is none, we use date */
					have_info = false;

					while (isdigit(*pos)) {
						have_info = true;
						GET_NUMBER(CAL_REPEAT_MONTH, 0);
						NEXT_NOSPACE(0);
					}

					if (!have_info) {
						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_MONTH;
						Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
						Calendar->Entries[Calendar->EntriesNum].Number =
							Calendar->Entries[TimePos].Date.Month;
						Calendar->EntriesNum++;
					}
					break;
				/* Yearly by day */
				case 'D':
					NEXT_CHAR(1);
					GET_FREQUENCY(0);
					NEXT_NOSPACE(0);
					/* There might be now list of days, if there is none, we use date */
					have_info = false;

					while (isdigit(*pos)) {
						have_info = true;
						GET_NUMBER(CAL_REPEAT_DAYOFYEAR, 0);
						NEXT_NOSPACE(0);
					}

					if (!have_info) {
#if 0
						/*
						 * This seems to be according to specification,
						 * however several vendors (Siemens, some web based
						 * calendars use YD1 for simple year repeating. So
						 * we handle this as YM1 just to be compatbile with
						 * those.
						 */
						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_DAYOFYEAR;
						Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
						Calendar->Entries[Calendar->EntriesNum].Number =
							GetDayOfYear(
								Calendar->Entries[TimePos].Date.Year,
								Calendar->Entries[TimePos].Date.Month,
								Calendar->Entries[TimePos].Date.Day);
						Calendar->EntriesNum++;
#endif
						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_DAY;
						Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
						Calendar->Entries[Calendar->EntriesNum].Number =
							Calendar->Entries[TimePos].Date.Day;
						Calendar->EntriesNum++;

						Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_MONTH;
						Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
						Calendar->Entries[Calendar->EntriesNum].Number =
							Calendar->Entries[TimePos].Date.Month;
						Calendar->EntriesNum++;
					}
					break;
				default:
					smfprintf(di, "Could not decode recurrency: %s\n", pos);
					return ERR_UNKNOWN;
			}
			break;
		default:
			smfprintf(di, "Could not decode recurrency: %s\n", pos);
			return ERR_UNKNOWN;
	}

	/* Go to duration */
	NEXT_NOSPACE(1);

	/* Do we have duration encoded? */
	if (*pos == '#') {
		pos++;
		if (*pos == 0) return ERR_UNKNOWN;
		GET_NUMBER(CAL_REPEAT_COUNT, 0);
	}

	/* Go to end date */
	NEXT_NOSPACE(1);

	/* Do we have end date encoded? */
	if (ReadVCALDateTime(pos, &(Calendar->Entries[Calendar->EntriesNum].Date))) {
		Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_REPEAT_STOPDATE;
		Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
		Calendar->EntriesNum++;
	}

	return ERR_NONE;
}

GSM_Error GSM_DecodeVCALENDAR_VTODO(GSM_Debug_Info *di, char *Buffer, size_t *Pos, GSM_CalendarEntry *Calendar,
					GSM_ToDoEntry *ToDo, GSM_VCalendarVersion CalVer, GSM_VToDoVersion ToDoVer)
{
	unsigned char 	Line[2000],Buff[2000];
	int		Level = 0;
	GSM_DateTime	Date;
	GSM_DeltaTime	OneHour = {0, 0, 0, 1, 0, 0, 0};
	GSM_DeltaTime	trigger;
	GSM_Error	error;
	int		deltatime = 0;
	int		dstflag = 0;
	bool		is_date_only;
	bool		date_only = false;
	int		lBuffer;
 	int 		Time=-1, Alarm=-1;
	char		*rrule = NULL;

	if (!Buffer) return ERR_EMPTY;

	Calendar->EntriesNum 	= 0;
	ToDo->EntriesNum 	= 0;
	lBuffer = strlen(Buffer);
	trigger.Timezone = -999 * 3600;

	if (CalVer == Mozilla_iCalendar && *Pos ==0) {
		error = GSM_Make_VCAL_Lines (Buffer, &lBuffer);
		if (error != ERR_NONE) return error;
	}

	while (1) {
		error = MyGetLine(Buffer, Pos, Line, lBuffer, sizeof(Line), true);
		if (error != ERR_NONE) return error;
		if (strlen(Line) == 0) break;

		switch (Level) {
		case 0:
			if (strstr(Line,"BEGIN:VEVENT")) {
				Calendar->Type = 0;
				date_only = true;
				dstflag = 0;
				Time=-1; Alarm=-1;
				Level 		= 1;
			}
			if (strstr(Line,"BEGIN:VTODO")) {
				ToDo->Priority 	= GSM_Priority_None;
				ToDo->Type = GSM_CAL_MEMO;
				dstflag = 0;
				Time=-1; Alarm=-1;
				Level 		= 2;
			}
			break;
		case 1: /* Calendar note */
			if (strstr(Line,"END:VEVENT")) {
				if (Time == -1) {
					smfprintf(di, "vCalendar without date!\n");
					return ERR_UNKNOWN;
				}
				if (rrule != NULL) {
					if (CalVer == Mozilla_iCalendar) {
						/* @todo: We don't have parser for this right now */
						error = ERR_NONE;
					} else {
						error = GSM_DecodeVCAL_RRULE(di, rrule, Calendar, Time);
					}
					free(rrule);
					if (error != ERR_NONE) {
						return error;
					}
				}
				if (Calendar->EntriesNum == 0) return ERR_EMPTY;

				if (trigger.Timezone != -999 * 3600) {
					Alarm = Calendar->EntriesNum;
					Calendar->Entries[Alarm].Date = GSM_AddTime (Calendar->Entries[Time].Date, trigger);
					Calendar->Entries[Alarm].EntryType = CAL_TONE_ALARM_DATETIME;
					Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
					Calendar->EntriesNum++;
				}

				if (dstflag != 0) {
					/*
					 * Day saving time was active while entry was created,
					 * add one hour to adjust it.
					 */
					if (dstflag == 4) {
						GSM_Calendar_AdjustDate(Calendar, &OneHour);
						smfprintf(di, "Adjusting DST: %i\n", dstflag);
					} else {
						smfprintf(di, "Unknown DST flag: %i\n", dstflag);
					}
				}

				/* If event type is undefined choose appropriate type. Memos carry dates only, no times.
				   Use Meetings for events with full date+time settings. */
				if (Calendar->Type == 0) {
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
			if (strncmp(Line, "UID:", 4) == 0) {
				ReadVCALText(Line, "UID", Buff, CalVer == Mozilla_iCalendar);  /*  Any use for UIDs? */
				break;
			}
#if 0
			if (strstr(Line,"X-MOZILLA-ALARM-DEFAULT-UNITS:")) {
				if (ReadVCALText(Line, "X-MOZILLA-ALARM-DEFAULT-UNITS", Buff, CalVer == Mozilla_iCalendar)) {
					unit = ReadVCALTimeUnits(DecodeUnicodeString(Buff));
					break;
				}
			}
#endif
			if (strstr(Line,"X-MOZILLA-ALARM-DEFAULT-LENGTH:")) {
				if (ReadVCALInt(Line, "X-MOZILLA-ALARM-DEFAULT-LENGTH", &deltatime)) {
					break;
				}
			}

			if (strstr(Line,"BEGIN:VALARM")) {
				error = MyGetLine(Buffer, Pos, Line, lBuffer, sizeof(Line), true);
				if (error != ERR_NONE) return error;
				if (strlen(Line) == 0) break;
				if (ReadVCALText(Line, "TRIGGER;VALUE=DURATION", Buff, CalVer == Mozilla_iCalendar)) {
					trigger = ReadVCALTriggerTime(DecodeUnicodeString(Buff));
					break;
				}
			}

			/* Event type. Must be set correctly to let phone calendar work as expected. For example
			   without GSM_CAL_MEETING the time part of an event date/time will be dropped. */
			if (strstr(Line,"CATEGORIES:")) {
				GSM_Translate_Category(TRANSL_TO_GSM, Line + 11, &Calendar->Type);
				break;
			}

			if (strstr(Line,"RRULE:")) {
				if (rrule == NULL) {
					rrule = strdup(Line + 6);
				} else {
					smfprintf(di, "Ignoring second recurrence: %s\n", Line);
				}
				break;
			}

			if ((ReadVCALText(Line, "SUMMARY", Buff, CalVer == Mozilla_iCalendar))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TEXT;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,
					DecodeUnicodeSpecialChars(Buff));
				Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
				Calendar->EntriesNum++;
			}
			if ((ReadVCALText(Line, "DESCRIPTION", Buff, CalVer == Mozilla_iCalendar))) {
				CopyUnicodeString(Buff,DecodeUnicodeSpecialChars(Buff));
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_DESCRIPTION;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,
					DecodeUnicodeSpecialChars(Buff));
				Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
				Calendar->EntriesNum++;
			}
			if (ReadVCALText(Line, "LOCATION", Buff, CalVer == Mozilla_iCalendar)) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LOCATION;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,
					DecodeUnicodeSpecialChars(Buff));
				Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
				Calendar->EntriesNum++;
			}
			if ((ReadVCALText(Line, "X-IRMC-LUID", Buff, CalVer == Mozilla_iCalendar))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LUID;
				CopyUnicodeString(Calendar->Entries[Calendar->EntriesNum].Text,
					DecodeUnicodeSpecialChars(Buff));
				Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
				Calendar->EntriesNum++;
			}
			if ((ReadVCALText(Line, "CLASS", Buff, CalVer == Mozilla_iCalendar))) {
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_PRIVATE;
				if (mywstrncasecmp(Buff, "\0P\0U\0B\0L\0I\0C\0\0", 0)) {
					Calendar->Entries[Calendar->EntriesNum].Number = 0;
				} else {
					Calendar->Entries[Calendar->EntriesNum].Number = 1;
				}
				Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
				Calendar->EntriesNum++;
			}
			if (ReadVCALDate(Line, "DTSTART", &Date, &is_date_only)) {
				Calendar->Entries[Calendar->EntriesNum].Date = Date;
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_START_DATETIME;
				Time = Calendar->EntriesNum;
				Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
				Calendar->EntriesNum++;
				if (!is_date_only) date_only = false;
			}
			if (ReadVCALDate(Line, "DTEND", &Date, &is_date_only)) {
				Calendar->Entries[Calendar->EntriesNum].Date = Date;
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_END_DATETIME;
				Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
				Calendar->EntriesNum++;
				if (!is_date_only) date_only = false;
			}
			if (ReadVCALDate(Line, "DALARM", &Date, &is_date_only)) {
				Calendar->Entries[Calendar->EntriesNum].Date = Date;
				if (CalVer == Siemens_VCalendar) {
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TONE_ALARM_DATETIME;
				} else {
					Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
				}
				Alarm = Calendar->EntriesNum;
				Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
				Calendar->EntriesNum++;
			}
			if (ReadVCALDate(Line, "AALARM", &Date, &is_date_only)) {
				Calendar->Entries[Calendar->EntriesNum].Date = Date;
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_TONE_ALARM_DATETIME;
				Alarm = Calendar->EntriesNum;
				Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
				Calendar->EntriesNum++;
			}
			if (ReadVCALDate(Line, "LAST-MODIFIED", &Date, &is_date_only)) {
				Calendar->Entries[Calendar->EntriesNum].Date = Date;
				Calendar->Entries[Calendar->EntriesNum].EntryType = CAL_LAST_MODIFIED;
				Calendar->Entries[Calendar->EntriesNum].AddError = ERR_NONE;
				Calendar->EntriesNum++;
			}
			if (strstr(Line,"X-SONYERICSSON-DST:")) {
				if (ReadVCALInt(Line, "X-SONYERICSSON-DST", &dstflag)) {
					break;
				}
			}
			break;

		case 2: /* ToDo note */
			if (strstr(Line,"END:VTODO")) {
				if (ToDo->EntriesNum == 0) return ERR_EMPTY;

				if (dstflag != 0) {
					/*
					 * Day saving time was active while entry was created,
					 * add one hour to adjust it.
					 */
					if (dstflag == 4) {
						GSM_ToDo_AdjustDate(ToDo, &OneHour);
						smfprintf(di, "Adjusting DST: %i\n", dstflag);
					} else {
						smfprintf(di, "Unknown DST flag: %i\n", dstflag);
					}
				}

				return ERR_NONE;
			}

			if (strstr(Line,"CATEGORIES:")) {
				GSM_Translate_Category(TRANSL_TO_GSM, Line+11, &ToDo->Type);
			}

			if (strncmp(Line, "UID:", 4) == 0) {
				ReadVCALText(Line, "UID", Buff, ToDoVer == Mozilla_iCalendar);  /*  Any use for UIDs? */
				break;
			}
#if 0
			if (strstr(Line,"X-MOZILLA-ALARM-DEFAULT-UNITS:")) {
				if (ReadVCALText(Line, "X-MOZILLA-ALARM-DEFAULT-UNITS", Buff, ToDoVer == Mozilla_iCalendar)) {
					unit = ReadVCALTimeUnits(DecodeUnicodeString(Buff));
					break;
				}
			}
#endif
			if (strstr(Line,"X-MOZILLA-ALARM-DEFAULT-LENGTH:")) {
				if (ReadVCALInt(Line, "X-MOZILLA-ALARM-DEFAULT-LENGTH", &deltatime)) {
					break;
				}
			}
			if (strstr(Line,"X-SONYERICSSON-DST:")) {
				if (ReadVCALInt(Line, "X-SONYERICSSON-DST", &dstflag)) {
					break;
				}
			}

			if (ReadVCALDate(Line, "DUE", &Date, &is_date_only)) {
				if (ToDo->Entries[ToDo->EntriesNum].Date.Year   != 2037	&&
				    ToDo->Entries[ToDo->EntriesNum].Date.Month  != 12	&&
				    ToDo->Entries[ToDo->EntriesNum].Date.Day    != 31	&&
				    ToDo->Entries[ToDo->EntriesNum].Date.Hour   != 23	&&
				    ToDo->Entries[ToDo->EntriesNum].Date.Minute != 59 ) {
					ToDo->Entries[ToDo->EntriesNum].Date = Date;
					ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_END_DATETIME;
					ToDo->EntriesNum++;
				}
			}
			if (ReadVCALDate(Line, "DTSTART", &Date, &is_date_only)) {
				ToDo->Entries[ToDo->EntriesNum].Date = Date;
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_START_DATETIME;
				ToDo->EntriesNum++;
			}
			if (ReadVCALDate(Line, "DALARM", &Date, &is_date_only)) {
				ToDo->Entries[ToDo->EntriesNum].Date = Date;
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_SILENT_ALARM_DATETIME;
				Alarm = Calendar->EntriesNum;
				ToDo->EntriesNum++;
			}
			if (ReadVCALDate(Line, "LAST-MODIFIED", &Date, &is_date_only)) {
				ToDo->Entries[ToDo->EntriesNum].Date = Date;
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_LAST_MODIFIED;
				ToDo->EntriesNum++;
			}
			if (ReadVCALDate(Line, "AALARM", &Date, &is_date_only)) {
				ToDo->Entries[ToDo->EntriesNum].Date = Date;
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_ALARM_DATETIME;
				Alarm = Calendar->EntriesNum;
				ToDo->EntriesNum++;
			}

			if ((ReadVCALText(Line, "SUMMARY", Buff, ToDoVer == Mozilla_iCalendar))) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_TEXT;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,
					DecodeUnicodeSpecialChars(Buff));
				ToDo->EntriesNum++;
			}
			if ((ReadVCALText(Line, "DESCRIPTION", Buff, ToDoVer == Mozilla_iCalendar))) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_DESCRIPTION;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,
					DecodeUnicodeSpecialChars(Buff));
				ToDo->EntriesNum++;
			}
			if ((ReadVCALText(Line, "LOCATION", Buff, ToDoVer == Mozilla_iCalendar))) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_LOCATION;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,
					DecodeUnicodeSpecialChars(Buff));
				ToDo->EntriesNum++;
			}
			if (ReadVCALText(Line, "PRIORITY", Buff, ToDoVer == Mozilla_iCalendar)) {
				if (atoi(DecodeUnicodeString(Buff))==3) ToDo->Priority = GSM_Priority_Low;
				else if (atoi(DecodeUnicodeString(Buff))==2) ToDo->Priority = GSM_Priority_Medium;
				else if (atoi(DecodeUnicodeString(Buff))==1) ToDo->Priority = GSM_Priority_High;
				else ToDo->Priority = GSM_Priority_None;
			}
			if (strstr(Line,"STATUS:COMPLETED")) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_COMPLETED;
				ToDo->Entries[ToDo->EntriesNum].Number	  = 1;
				ToDo->EntriesNum++;
			}
			if ((ReadVCALText(Line, "X-IRMC-LUID", Buff, ToDoVer == Mozilla_iCalendar))) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_LUID;
				CopyUnicodeString(ToDo->Entries[ToDo->EntriesNum].Text,
					DecodeUnicodeSpecialChars(Buff));
				ToDo->EntriesNum++;
			}
			if ((ReadVCALText(Line, "CLASS", Buff, ToDoVer == Mozilla_iCalendar))) {
				ToDo->Entries[ToDo->EntriesNum].EntryType = TODO_PRIVATE;
				if (mywstrncasecmp(Buff, "\0P\0U\0B\0L\0I\0C\0", 0)) {
					ToDo->Entries[ToDo->EntriesNum].Number = 0;
				} else {
					ToDo->Entries[ToDo->EntriesNum].Number = 1;
				}
				ToDo->EntriesNum++;
			}
			break;
		}
	}

	if (Calendar->EntriesNum == 0 && ToDo->EntriesNum == 0) return ERR_EMPTY;
	return ERR_NONE;
}

GSM_Error GSM_DecodeVNOTE(char *Buffer, size_t *Pos, GSM_NoteEntry *Note)
{
	unsigned char   Line[2000],Buff[2000];
	int	     Level = 0;
	GSM_Error	error;

	Note->Text[0] = 0;
	Note->Text[1] = 0;

	while (1) {
		error = MyGetLine(Buffer, Pos, Line, strlen(Buffer), sizeof(Line), true);
		if (error != ERR_NONE) return error;
		if (strlen(Line) == 0) break;
		switch (Level) {
		case 0:
			if (strstr(Line,"BEGIN:VNOTE")) Level = 1;
			break;
		case 1:
			if (strstr(Line,"END:VNOTE")) {
				if (UnicodeLength(Note->Text) == 0) return ERR_EMPTY;
				return ERR_NONE;
			}
			if (ReadVCALText(Line, "BODY",	      Buff, false)) {
				CopyUnicodeString(Note->Text, Buff);
			}
			break;
		}
	}

	return ERR_BUG;
}

GSM_Error GSM_EncodeVNTFile(char *Buffer, const size_t buff_len, size_t *Length, GSM_NoteEntry *Note)
{
	GSM_Error error;

	error = VC_StoreLine(Buffer, buff_len, Length,  "BEGIN:VNOTE");
	if (error != ERR_NONE) return error;
	error = VC_StoreLine(Buffer, buff_len, Length,  "VERSION:1.1");
	if (error != ERR_NONE) return error;
	error = VC_StoreText(Buffer, buff_len, Length, Note->Text, "BODY", false);
	if (error != ERR_NONE) return error;
	error = VC_StoreLine(Buffer, buff_len, Length,  "END:VNOTE");
	if (error != ERR_NONE) return error;

	return ERR_NONE;
}
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
