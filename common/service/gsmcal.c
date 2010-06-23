
#include <string.h>

#include "gsmcal.h"
#include "../misc/coding.h"

void NOKIA_EncodeVCALENDAR10SMSText(char *Buffer, int *Length, GSM_CalendarNote note)
{
	char buffer[1000];

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
	if (note.Type == GCN_CALL && (note.Phone[0]!=0 || note.Phone[1]!=0))
	{
		EncodeUTF8(buffer,note.Phone);
		if (strlen(DecodeUnicodeString(note.Phone))==strlen(buffer)) {
			*Length+=sprintf(Buffer+(*Length), "SUMMARY:%s%c%c",DecodeUnicodeString(note.Phone),13,10);
		} else {
			*Length+=sprintf(Buffer+(*Length), "SUMMARY;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c",buffer,13,10);
		}	    
	} else {
		EncodeUTF8(buffer,note.Text);
		if (strlen(DecodeUnicodeString(note.Text))==strlen(buffer)) {
			*Length+=sprintf(Buffer+(*Length), "SUMMARY:%s%c%c",DecodeUnicodeString(note.Text),13,10);
		} else {
			*Length+=sprintf(Buffer+(*Length), "SUMMARY;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c",buffer,13,10);
		}	    
	}

	*Length+=sprintf(Buffer+(*Length), "DTSTART:%04d%02d%02dT%02d%02d%02d%c%c",
			note.Time.Year, note.Time.Month, note.Time.Day,
			note.Time.Hour, note.Time.Minute, note.Time.Second,13,10);

	if (note.Alarm.Year!=0) {
		*Length+=sprintf(Buffer+(*Length), "DALARM:%04d%02d%02dT%02d%02d%02d%c%c",
				note.Alarm.Year, note.Alarm.Month, note.Alarm.Day,
				note.Alarm.Hour, note.Alarm.Minute, note.Alarm.Second,13,10);
	}

	switch(note.Recurrance/24) {
		case 1	 : *Length+=sprintf(Buffer+(*Length), "RRULE:D1 #0%c%c",13,10);	 break;
		case 7	 : *Length+=sprintf(Buffer+(*Length), "RRULE:W1 #0%c%c",13,10);	 break;
		case 14	 : *Length+=sprintf(Buffer+(*Length), "RRULE:W2 #0%c%c",13,10);	 break;
		case 365 : *Length+=sprintf(Buffer+(*Length), "RRULE:YD1 #0%c%c",13,10); break;
	}

	*Length+=sprintf(Buffer+(*Length), "END:VEVENT%c%c",13,10);
	*Length+=sprintf(Buffer+(*Length), "END:VCALENDAR%c%c",13,10);
}
