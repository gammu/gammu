
#include <string.h> /* memcpy only */
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include "../../gsmstate.h"
#include "../../misc/coding.h"
#include "../../service/gsmsms.h"
#include "../pfunc.h"
#include "nfunc.h"

#ifdef DEBUG
static void N71_65_GetCalendarAlarm(GSM_StateMachine *s, unsigned char *buffer, GSM_CalendarEntry *entry, int DT, GSM_Phone_Data *Data)
{
	unsigned long diff;

	if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0xff && buffer[3] == 0xff)
	{
		smprintf(s, "No alarm\n");
	} else {
		memcpy(&entry->Entries[entry->EntriesNum].Date,&entry->Entries[DT].Date,sizeof(GSM_DateTime));

		diff  = ((unsigned int)buffer[0]) << 24;
		diff += ((unsigned int)buffer[1]) << 16;
		diff += ((unsigned int)buffer[2]) << 8;
		diff += buffer[3];
		smprintf(s, "  Difference : %i seconds\n", diff);

		switch (entry->Type) {
		case GCN_MEETING:
			N71_65_GetTimeDiffence(s, diff, &entry->Entries[entry->EntriesNum].Date, false, 60);
			break;
		case GCN_REMINDER:
			if (!IsPhoneFeatureAvailable(Data->ModelInfo, F_CAL35)) {
				N71_65_GetTimeDiffence(s, diff, &entry->Entries[entry->EntriesNum].Date, false, 60);
				break;
			}
		case GCN_CALL:
			if (!IsPhoneFeatureAvailable(Data->ModelInfo, F_CAL35)) {
				N71_65_GetTimeDiffence(s, diff, &entry->Entries[entry->EntriesNum].Date, false, 60);
				break;
			}
		default:
			N71_65_GetTimeDiffence(s, diff, &entry->Entries[entry->EntriesNum].Date, false, 1);
		}
		smprintf(s, "Alarm date   : %02i-%02i-%04i %02i:%02i:%02i\n",
			entry->Entries[entry->EntriesNum].Date.Day,   entry->Entries[entry->EntriesNum].Date.Month,
			entry->Entries[entry->EntriesNum].Date.Year,  entry->Entries[entry->EntriesNum].Date.Hour,
			entry->Entries[entry->EntriesNum].Date.Minute,entry->Entries[entry->EntriesNum].Date.Second);

		entry->Entries[entry->EntriesNum].EntryType = CAL_ALARM_DATETIME;
		if (entry->Type == GCN_BIRTHDAY) {
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

	smprintf(s, "Calendar note received\n");

	if (msg.Length < 10) return GE_EMPTY;

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
	smprintf(s, "  Difference : %i seconds\n", diff);
	N71_65_GetTimeDiffence(s, diff, &Date, true, 1);
	Date.Year += 20;
	entry->Entries[0].EntryType = CAL_START_DATETIME;

	switch (msg.Buffer[21]) {
	case 0x01:
	case 0x08:
		if (msg.Buffer[21] == 0x01) {
			smprintf(s, "Meeting\n");
			entry->Type = GCN_MEETING;
		} else {
			smprintf(s, "Reminder\n");
			Data->Cal->Type = GCN_REMINDER;
		}

		memcpy(&entry->Entries[0].Date,&Date,sizeof(GSM_DateTime));
		entry->EntriesNum++;

		N71_65_GetCalendarAlarm(s, msg.Buffer+16, entry, 0, Data);
		N71_65_GetCalendarRecurrance(s, msg.Buffer+22, entry);

		memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+30, msg.Buffer[28]*2);
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[28]*2]   = 0;
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[28]*2+1] = 0;
		entry->Entries[entry->EntriesNum].EntryType		   = CAL_TEXT;
		break;
	case 0x02:
		smprintf(s, "Call\n");
		entry->Type = GCN_CALL;

		memcpy(&entry->Entries[0].Date,&Date,sizeof(GSM_DateTime));
		entry->EntriesNum++;

		N71_65_GetCalendarAlarm(s, msg.Buffer+16, entry, 0, Data);
		N71_65_GetCalendarRecurrance(s, msg.Buffer+22, entry);

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
		Data->Cal->Type = GCN_BIRTHDAY;

		/* Year was set earlier */
		entry->Entries[0].Date.Month	= Date.Month;
		entry->Entries[0].Date.Day	= Date.Day;
		entry->Entries[0].Date.Hour	= 23;
		entry->Entries[0].Date.Minute	= 59;
		entry->Entries[0].Date.Second	= 58;
		entry->EntriesNum++;

		N71_65_GetCalendarAlarm(s, msg.Buffer+16, entry, 0, Data);
		N71_65_GetCalendarRecurrance(s, msg.Buffer+22, entry);

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
		return GE_UNKNOWNRESPONSE;
	}
	smprintf(s, "Text         : \"%s\"\n", DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
	entry->EntriesNum++;
	return GE_NONE;
}

/* method 2 */
/* Note: in known phones texts of notes cut to 50 chars */
GSM_Error N71_65_GetNextCalendar2(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start, int *LastCalendarYear, int *LastCalendarPos)
{
	GSM_Error		error;
	GSM_DateTime		date_time;
	unsigned char 		req[] = {
		N6110_FRAME_HEADER, 0x3e, 
		0xFF, 0xFE};		/* Location */

	if (start) {
		/* We have to get current year. It's NOT written in frame for
		 * Birthday
		 */
		error=s->Phone.Functions->GetDateTime(s,&date_time);
		switch (error) {
			case GE_EMPTY:
			case GE_NOTIMPLEMENTED:
				GSM_GetCurrentDateTime(&date_time);
				break;
			case GE_NONE:
				break;
			default:
				return error;
		}
		*LastCalendarYear = date_time.Year;

		/* First location at all */
		req[4] = 0xFF;
		req[5] = 0xFE;
	} else {
		req[4] = *LastCalendarPos >> 8;
		req[5] = *LastCalendarPos & 0xff;
	}
	Note->EntriesNum		= 0;
	Note->Entries[0].Date.Year 	= *LastCalendarYear;

	s->Phone.Data.Cal = Note;
	smprintf(s, "Getting calendar note\n");
	error=GSM_WaitFor (s, req, 6, 0x13, 4, ID_GetCalendarNote);
	*LastCalendarPos = Note->Location;
	return error;
}

/* method 1*/
GSM_Error N71_65_ReplyGetCalendarNotePos1(GSM_Protocol_Message msg, GSM_StateMachine *s,int *FirstCalendarPos)
{
	smprintf(s, "First calendar location: %i\n",msg.Buffer[4]*256+msg.Buffer[5]);
	*FirstCalendarPos = msg.Buffer[4]*256+msg.Buffer[5];
	return GE_NONE;
}

/* method 1*/
static GSM_Error N71_65_GetCalendarNotePos1(GSM_StateMachine *s)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x31};

	smprintf(s, "Getting first free calendar note location\n");
	return GSM_WaitFor (s, req, 4, 0x13, 4, ID_GetCalendarNotePos);
}

/* method 1 */
GSM_Error N71_65_ReplyAddCalendar1(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
#ifdef DEBUG
	smprintf(s, "Written Calendar Note type ");
	switch ((msg.Buffer[3]/2)-1) {
		case 0:	smprintf(s, "Meeting");	break;
		case 1:	smprintf(s, "Call");	break;
		case 2:	smprintf(s, "Birthday");break;
		case 3:	smprintf(s, "Reminder");break;
	}
	smprintf(s, " on location %d\n",msg.Buffer[4]*256+msg.Buffer[5]);
#endif
	return GE_NONE;
}

/* method 1 */
GSM_Error N71_65_AddCalendar1(GSM_StateMachine *s, GSM_CalendarEntry *Note, int *FirstCalendarPos, bool Past)
{
	long			seconds;
 	GSM_Error		error;
	GSM_DateTime		DT;
 	int 			Text, Time, Alarm, Phone, Recurrance, EndTime, count=12;
	unsigned char 		req[5000] = {
		N6110_FRAME_HEADER,
		0x01,				/* note type */
		0x00, 0x00,			/* location ? */
		0x00,				/* entry type */
		0x00,
		0x00, 0x00, 0x00, 0x00,		/* Year(2bytes), Month, Day */
		/* here starts block ... depends on note type */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00};                          

	if (!Past && IsNoteFromThePast(*Note)) return GE_NONE;

	error=N71_65_GetCalendarNotePos1(s);
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

	GSM_CalendarFindDefaultTextTimeAlarmPhoneRecurrance(*Note, &Text, &Time, &Alarm, &Phone, &Recurrance, &EndTime);

	if (Time == -1) return GE_UNKNOWN;
	memcpy(&DT,&Note->Entries[Time].Date,sizeof(GSM_DateTime));
	req[8]	= DT.Year >> 8;
	req[9]	= DT.Year & 0xff;
	req[10]	= DT.Month;
	req[11]	= DT.Day;

	switch(Note->Type) {
	case GCN_BIRTHDAY:
		req[count++] = 0x00;		/* 12 */
		req[count++] = 0x00;		/* 13 */

		/* Alarm */
		req[count++] = 0x00;		/* 14 */
		req[count++] = 0x00;		/* 15 */
		req[count++] = 0xff;		/* 16 */
		req[count++] = 0xff;		/* 17 */
		if (Alarm != -1) {
			/* I try with Time.Year = Alarm.Year. If negative, I increase 1 year,
			 * but only once ! This thing, because I may have Alarm period across
			 * a year. (eg. Birthday on 2001-01-10 and Alarm on 2000-12-27)
			 */
			DT.Year = Note->Entries[Alarm].Date.Year;
			if((seconds = Fill_Time_T(DT,8)-Fill_Time_T(Note->Entries[Alarm].Date,8))<0L)
			{
				DT.Year++;
				seconds = Fill_Time_T(DT,8)-Fill_Time_T(Note->Entries[Alarm].Date,8);
			}
			if(seconds>=0L)
			{
				count -= 4;
				req[count++] = (unsigned char)(seconds>>24); 			/* 14 */
				req[count++] = (unsigned char)((seconds>>16) & 0xff);		/* 15 */
				req[count++] = (unsigned char)((seconds>>8) & 0xff);		/* 16 */
				req[count++] = (unsigned char)(seconds & 0xff);			/* 17 */
			}
			if (Note->Entries[Alarm].EntryType == CAL_SILENT_ALARM_DATETIME) req[count++] = 0x01; else req[count++] = 0x00;
		}

		if (Text != -1) {
			req[count++] = strlen(DecodeUnicodeString(Note->Entries[Text].Text));	/* 19 */
			CopyUnicodeString(req+count,Note->Entries[Text].Text);
			count=count+2*strlen(DecodeUnicodeString(Note->Entries[Text].Text));	/* 22->N */
		} else {
			req[count++] = 0x00;
		}
		break;
	case GCN_REMINDER:
		if (Recurrance != -1) {
			/* 0xffff -> 1 Year (8760 hours) */
			if (Note->Entries[Recurrance].Number >= 8760) {
				req[count++] = 0xff;
				req[count++] = 0xff;
			} else {
				req[count++] = Note->Entries[Recurrance].Number >> 8;   /* 12 */
				req[count++] = Note->Entries[Recurrance].Number & 0xff; /* 13 */
			}
		} else {
			req[count++] = 0x00; /* 12 */
			req[count++] = 0x00; /* 13 */
		}

		if (Text != -1) {
			req[count++] = strlen(DecodeUnicodeString(Note->Entries[Text].Text));	/* 14 */
			req[count++] = 0x00;							/* 15 */
			CopyUnicodeString(req+count,Note->Entries[Text].Text);
			count=count+2*strlen(DecodeUnicodeString(Note->Entries[Text].Text));	/* 16->N */
		} else {
			req[count++] = 0x00;
			req[count++] = 0x00;
		}
		break;
	case GCN_MEETING:
	case GCN_CALL:
	default:
		req[count++] = DT.Hour;   /* 12 */
		req[count++] = DT.Minute; /* 13 */

		/* Alarm */
		req[count++] = 0xff;	  /* 14 */
		req[count++] = 0xff;	  /* 15 */
		if (Alarm != -1) {
			seconds=Fill_Time_T(DT,8)-Fill_Time_T(Note->Entries[Alarm].Date,8);
			if(seconds>=0L)
			{
				count -= 2;
				req[count++] = ((unsigned char)(seconds/60L)>>8);
				req[count++] = (unsigned char)((seconds/60L)&0xff);
			}
		}

		if (Recurrance != -1) {
			/* 0xffff -> 1 Year (8760 hours) */
			if (Note->Entries[Recurrance].Number >= 8760) {
				req[count++] = 0xff;
				req[count++] = 0xff;
			} else {
				req[count++] = Note->Entries[Recurrance].Number >> 8;   /* 12 */
				req[count++] = Note->Entries[Recurrance].Number & 0xff; /* 13 */
			}
		} else {
			req[count++] = 0x00; /* 16 */
			req[count++] = 0x00; /* 17 */
		}

		if (Text != -1) {
			req[count++] = strlen(DecodeUnicodeString(Note->Entries[Text].Text));
		} else {
			req[count++] = 0x00; /* 18 */
		}
		if (Note->Type == GCN_CALL && Phone != -1) {
			req[count++] = strlen(DecodeUnicodeString(Note->Entries[Phone].Text));
		} else {
			req[count++] = 0x00; /* 19 */
		}
		if (Text != -1) {
			CopyUnicodeString(req+count,Note->Entries[Text].Text);
			count=count+2*strlen(DecodeUnicodeString(Note->Entries[Text].Text));/* 20->N */
		}
		if (Note->Type == GCN_CALL && Phone != -1) {
			CopyUnicodeString(req+count,Note->Entries[Phone].Text);
			count=count+2*strlen(DecodeUnicodeString(Note->Entries[Phone].Text));/* (N+1)->n */
		}
		break;
	}
	req[count] = 0x00;
	smprintf(s, "Writing calendar note\n");
	return GSM_WaitFor (s, req, count, 0x13, 4, ID_SetCalendarNote);
}
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
