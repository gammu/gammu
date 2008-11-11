/* (c) 2002-2005 by Marcin Wiacek */

#include <gammu-config.h>

#ifdef GSM_ENABLE_NOKIA6510

#include <string.h>
#include <time.h>

#include "../../../../misc/coding/coding.h"
#include "../../../../gsmcomon.h"
#include "../../../../service/gsmlogo.h"
#include "../../nfunc.h"
#include "../../nfuncold.h"
#include "../../../pfunc.h"
#include "../dct4func.h"
#include "n6510.h"

/* method 3 */
GSM_Error N6510_ReplyGetCalendarInfo3(GSM_Protocol_Message msg, GSM_StateMachine *s, GSM_NOKIACalToDoLocations *Last)
{
	size_t i=0,j=0;

	while (Last->Location[j] != 0x00) j++;
	if (j >= GSM_MAXCALENDARTODONOTES) {
		smprintf(s, "Increase GSM_MAXCALENDARTODONOTES\n");
		return ERR_MOREMEMORY;
	}
	if (j == 0) {
		Last->Number = msg.Buffer[8]*256 + msg.Buffer[9];
		smprintf(s, "Number of Entries: %i\n", Last->Number);
	}
	smprintf(s, "Locations: ");
	while (14+(i*4) <= msg.Length) {
		Last->Location[j] = msg.Buffer[12+i*4]*256 + msg.Buffer[13+i*4];
		smprintf(s, "%i ", Last->Location[j]);
		i++;
		j++;
	}
	smprintf(s, "\nNumber of Entries in frame: " SIZE_T_FORMAT "\n",i);
	Last->Location[j] = 0;
	smprintf(s, "\n");
	if (i == 1 && msg.Buffer[12+0*4]*256+msg.Buffer[13+0*4] == 0) return ERR_EMPTY;
	if (i == 0) return ERR_EMPTY;
	return ERR_NONE;
}

/* method 3 */
GSM_Error N6510_GetCalendarInfo3(GSM_StateMachine *s, GSM_NOKIACalToDoLocations *Last, char Type)
{
	GSM_Error 	error = ERR_UNKNOWN;
	int		i;
	unsigned char   req[] = {N6110_FRAME_HEADER, 0x9E, 0xFF, 0xFF, 0x00, 0x00,
			         0x00, 0x00,	/* First location */
			         0x00};		/* 0 = calendar, 1 = ToDo in 6610 style, 2 = Notes */

	Last->Location[0] = 0x00;
	Last->Number	  = 0;

	req[10] = Type;
	if (Type == 0) {
		smprintf(s, "Getting locations for calendar method 3\n");
		error = GSM_WaitFor (s, req, 11, 0x13, 4, ID_GetCalendarNotesInfo);
	} else if (Type == 1) {
		smprintf(s, "Getting locations for ToDo method 2\n");
		error = GSM_WaitFor (s, req, 11, 0x13, 4, ID_GetToDo);
	} else if (Type == 2) {
		smprintf(s, "Getting locations for Notes\n");
		error = GSM_WaitFor (s, req, 11, 0x13, 4, ID_GetNote);
	}
	if (error != ERR_NONE && error != ERR_EMPTY) return error;

	while (1) {
		i=0;
		while (Last->Location[i] != 0x00) i++;
		smprintf(s, "i = %i %i\n",i,Last->Number);
		if (i == Last->Number) break;
		if (i != Last->Number && error == ERR_EMPTY) {
			smprintf(s, "Phone doesn't support some notes with this method. Workaround\n");
			Last->Number = i;
			break;
		}
		req[8] = Last->Location[i-1] / 256;
		req[9] = Last->Location[i-1] % 256;
		if (Type == 0) {
			smprintf(s, "Getting locations for calendar method 3\n");
			error = GSM_WaitFor (s, req, 11, 0x13, 4, ID_GetCalendarNotesInfo);
		} else if (Type == 1) {
			smprintf(s, "Getting locations for todo method 2\n");
			error = GSM_WaitFor (s, req, 11, 0x13, 4, ID_GetToDo);
		} else if (Type == 2) {
			smprintf(s, "Getting locations for Notes\n");
			error = GSM_WaitFor (s, req, 11, 0x13, 4, ID_GetNote);
		}
		if (error != ERR_NONE && error != ERR_EMPTY) return error;
	}
	return ERR_NONE;
}

/* method 3 */
GSM_Error N6510_ReplyGetCalendar3(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_CalendarEntry 		*entry = s->Phone.Data.Cal;
	GSM_DateTime			Date;
	unsigned long			diff;
	int				i;
	bool				found = false;
	GSM_Phone_N6510Data		*Priv = &s->Phone.Data.Priv.N6510;
	int len;

	smprintf(s, "Calendar note received method 3\n");

	smprintf(s,"Note type %02i: ",msg.Buffer[27]);
	switch(msg.Buffer[27]) {
		case 0x00: smprintf(s,"Reminder\n"); entry->Type = GSM_CAL_REMINDER; break;
		case 0x01: smprintf(s,"Meeting\n");  entry->Type = GSM_CAL_MEETING;  break;
		case 0x02: smprintf(s,"Call\n");     entry->Type = GSM_CAL_CALL;     break;
		case 0x04: smprintf(s,"Birthday\n"); entry->Type = GSM_CAL_BIRTHDAY; break;
		case 0x08: smprintf(s,"Memo\n");     entry->Type = GSM_CAL_MEMO;     break;
		default  : smprintf(s,"unknown\n");
	}

	smprintf(s,"StartTime: %04i-%02i-%02i %02i:%02i\n",
		msg.Buffer[28]*256+msg.Buffer[29],
		msg.Buffer[30],msg.Buffer[31],msg.Buffer[32],
		msg.Buffer[33]);
	Date.Year 	= msg.Buffer[28]*256+msg.Buffer[29];
	if (entry->Type == GSM_CAL_BIRTHDAY) {
		Date.Year = entry->Entries[0].Date.Year;
		smprintf(s,"%i\n",Date.Year);
	}
	Date.Month 	= msg.Buffer[30];
	Date.Day 	= msg.Buffer[31];
	Date.Hour 	= msg.Buffer[32];
	Date.Minute 	= msg.Buffer[33];
	/* Garbage seen with 3510i 3.51 */
	if (Date.Month == 0 && Date.Day == 0 && Date.Hour == 0 && Date.Minute == 0) return ERR_EMPTY;
	Date.Second	= 0;
	entry->Entries[0].EntryType = CAL_START_DATETIME;
	memcpy(&entry->Entries[0].Date,&Date,sizeof(GSM_DateTime));
	entry->EntriesNum++;

	GSM_GetCalendarRecurranceRepeat(msg.Buffer+40, msg.Buffer+46, entry);

	if (entry->Type != GSM_CAL_BIRTHDAY) {
		smprintf(s,"EndTime: %04i-%02i-%02i %02i:%02i\n",
			msg.Buffer[34]*256+msg.Buffer[35],
			msg.Buffer[36],msg.Buffer[37],msg.Buffer[38],
			msg.Buffer[39]);
		Date.Year 	= msg.Buffer[34]*256+msg.Buffer[35];
		Date.Month 	= msg.Buffer[36];
		Date.Day 	= msg.Buffer[37];
		Date.Hour 	= msg.Buffer[38];
		Date.Minute 	= msg.Buffer[39];
		Date.Second	= 0;
		entry->Entries[entry->EntriesNum].EntryType = CAL_END_DATETIME;
		memcpy(&entry->Entries[entry->EntriesNum].Date,&Date,sizeof(GSM_DateTime));
		entry->EntriesNum++;
	}

	smprintf(s, "Note icon: %02x\n",msg.Buffer[21]);
	for(i=0;i<Priv->CalendarIconsNum;i++) {
		if (Priv->CalendarIconsTypes[i] == entry->Type) {
			found = true;
		}
	}
	if (!found) {
		Priv->CalendarIconsTypes[Priv->CalendarIconsNum] = entry->Type;
		Priv->CalendarIcons	[Priv->CalendarIconsNum] = msg.Buffer[21];
		Priv->CalendarIconsNum++;
	}

	if (msg.Buffer[14] == 0xFF && msg.Buffer[15] == 0xFF && msg.Buffer[16] == 0xff && msg.Buffer[17] == 0xff) {
		smprintf(s, "No alarm\n");
	} else {
		diff  = ((unsigned int)msg.Buffer[14]) << 24;
		diff += ((unsigned int)msg.Buffer[15]) << 16;
		diff += ((unsigned int)msg.Buffer[16]) << 8;
		diff += msg.Buffer[17];

		memcpy(&entry->Entries[entry->EntriesNum].Date,&entry->Entries[0].Date,sizeof(GSM_DateTime));
		GetTimeDifference(diff, &entry->Entries[entry->EntriesNum].Date, false, 60);
		smprintf(s, "Alarm date   : %02i-%02i-%04i %02i:%02i:%02i\n",
			entry->Entries[entry->EntriesNum].Date.Day,   entry->Entries[entry->EntriesNum].Date.Month,
			entry->Entries[entry->EntriesNum].Date.Year,  entry->Entries[entry->EntriesNum].Date.Hour,
			entry->Entries[entry->EntriesNum].Date.Minute,entry->Entries[entry->EntriesNum].Date.Second);

		entry->Entries[entry->EntriesNum].EntryType = CAL_TONE_ALARM_DATETIME;
		if (msg.Buffer[22]==0x00 && msg.Buffer[23]==0x00 &&
		    msg.Buffer[24]==0x00 && msg.Buffer[25]==0x00) {
			entry->Entries[entry->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
			smprintf(s, "Alarm type   : Silent\n");
		}
		entry->EntriesNum++;
	}

	if (entry->Type == GSM_CAL_BIRTHDAY) {
		if (msg.Buffer[42] == 0xff && msg.Buffer[43] == 0xff) {
			entry->Entries[0].Date.Year = 0;
		} else {
			entry->Entries[0].Date.Year = msg.Buffer[42]*256+msg.Buffer[43];
		}
	}

	len = msg.Buffer[50] * 256 + msg.Buffer[51];
	if (len > GSM_MAX_CALENDAR_TEXT_LENGTH) {
		smprintf(s, "Calendar text too long (%d), truncating to %d\n", len, GSM_MAX_CALENDAR_TEXT_LENGTH);
		len = GSM_MAX_CALENDAR_TEXT_LENGTH;
	}
	memcpy(entry->Entries[entry->EntriesNum].Text,
		msg.Buffer + 54,
		len * 2);
	entry->Entries[entry->EntriesNum].Text[len * 2]  = 0;
	entry->Entries[entry->EntriesNum].Text[len * 2 + 1] = 0;
	entry->Entries[entry->EntriesNum].EntryType = CAL_TEXT;
	entry->EntriesNum++;
	smprintf(s, "Note text: \"%s\"\n",DecodeUnicodeString(entry->Entries[entry->EntriesNum-1].Text));

	if (entry->Type == GSM_CAL_CALL) {
		memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+(54+msg.Buffer[51]*2), msg.Buffer[52]*2);
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[52]*2]   = 0;
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[52]*2+1] = 0;
		entry->Entries[entry->EntriesNum].EntryType		   = CAL_PHONE;
		entry->EntriesNum++;
	}
	if (entry->Type == GSM_CAL_MEETING) {
		memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+(54+msg.Buffer[51]*2), msg.Buffer[52]*2);
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[52]*2]   = 0;
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[52]*2+1] = 0;
		entry->Entries[entry->EntriesNum].EntryType		   = CAL_LOCATION;
		entry->EntriesNum++;
	}

	return ERR_NONE;
}

static GSM_Error N6510_PrivGetGenericCalendar3(GSM_StateMachine *s, int Location, GSM_Phone_RequestID ID)
{
	unsigned char req[] = {N6110_FRAME_HEADER,0x7D,0x00,0x00,0x00,0x00,
			       0x00,0x99,	/* Location */
			       0xff,0xff,0xff,0xff};

	req[8] = Location / 256;
	req[9] = Location % 256;

	return GSM_WaitFor (s, req, 14, 0x13, 4, ID);
}

static GSM_Error N6510_PrivGetCalendar3(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start, int *LastCalendarYear)
{
	GSM_Error		error;
	GSM_DateTime		date_time;

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
	}

	Note->EntriesNum		= 0;
	Note->Entries[0].Date.Year 	= *LastCalendarYear;

	s->Phone.Data.Cal=Note;
	smprintf(s, "Getting calendar note method 3\n");
	return N6510_PrivGetGenericCalendar3(s, Note->Location, ID_GetCalendarNote);
}

/* method 3 */
static GSM_Error N6510_GetNextCalendar3(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start, GSM_NOKIACalToDoLocations *LastCalendar, int *LastCalendarYear, int *LastCalendarPos)
{
	GSM_Error error;
	bool	  start2;

	if (start) {
		error=N6510_GetCalendarInfo3(s,LastCalendar,0);
		if (error!=ERR_NONE) return error;
		if (LastCalendar->Number == 0) return ERR_EMPTY;

		*LastCalendarPos = 0;
	} else {
		(*LastCalendarPos)++;
	}

	error  = ERR_EMPTY;
	start2 = start;
	while (error == ERR_EMPTY) {
		if (*LastCalendarPos >= LastCalendar->Number) return ERR_EMPTY;

		Note->Location = LastCalendar->Location[*LastCalendarPos];
		error=N6510_PrivGetCalendar3(s, Note, start2, LastCalendarYear);
		if (error == ERR_EMPTY) (*LastCalendarPos)++;

		start2 = false;
	}
	return error;
}

GSM_Error N6510_ReplyGetCalendarInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
	case 0x3B:
		/* Old method 1 for accessing calendar */
		return N71_65_ReplyGetCalendarInfo1(msg, s, &s->Phone.Data.Priv.N6510.LastCalendar);
	case 0x9F:
		smprintf(s, "Info with calendar notes locations received method 3\n");
		return N6510_ReplyGetCalendarInfo3(msg, s, &s->Phone.Data.Priv.N6510.LastCalendar);
	}
	return ERR_UNKNOWNRESPONSE;
}

/* method 3 */
GSM_Error N6510_ReplyGetCalendarNotePos3(GSM_Protocol_Message msg, GSM_StateMachine *s,int *FirstCalendarPos)
{
	smprintf(s, "First calendar location: %i\n",msg.Buffer[8]*256+msg.Buffer[9]);
	*FirstCalendarPos = msg.Buffer[8]*256+msg.Buffer[9];
	return ERR_NONE;
}

/* method 3 */
static GSM_Error N6510_GetCalendarNotePos3(GSM_StateMachine *s)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x95, 0x00};

	smprintf(s, "Getting first free calendar note location\n");
	return GSM_WaitFor (s, req, 5, 0x13, 4, ID_GetCalendarNotePos);
}

GSM_Error N6510_ReplyGetCalendarNotePos(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
	case 0x32:
		/* Old method 1 for accessing calendar */
		return N71_65_ReplyGetCalendarNotePos1(msg, s,&s->Phone.Data.Priv.N6510.FirstCalendarPos);
	case 0x96:
		return N6510_ReplyGetCalendarNotePos3(msg, s,&s->Phone.Data.Priv.N6510.FirstCalendarPos);
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_FindCalendarIconID3(GSM_StateMachine *s, GSM_CalendarEntry *Entry, unsigned char *ID)
{
	int 				i,j,LastCalendarYear;
	GSM_Phone_N6510Data		*Priv = &s->Phone.Data.Priv.N6510;
	GSM_CalendarEntry 		Note;
	GSM_NOKIACalToDoLocations	LastCalendar1,LastCalendar2;
	GSM_Error			error;
	bool				found;

	for(i=0;i<Priv->CalendarIconsNum;i++) {
		if (Priv->CalendarIconsTypes[i] == Entry->Type) {
			*ID = Priv->CalendarIcons[i];
			return ERR_NONE;
		}
	}

	smprintf(s, "Starting finding note ID\n");

	error=N6510_GetCalendarInfo3(s, &Priv->LastCalendar,0);
	memcpy(&LastCalendar1,&Priv->LastCalendar,sizeof(GSM_NOKIACalToDoLocations));
	if (error != ERR_NONE) return error;

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_CAL35) ||
	    GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_CAL65) ||
	    GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_CAL62)) {
		error=N71_65_AddCalendar2(s,Entry);
	} else {
		/* First method 1 was used for meeting only
		 * but it made problems with 6230 RH-12 4.44
                 * (probably for other Series 40 2.0 phones too)
                 * For now meeting, call and memo uses method 1
                 * Please note, that method 1 is the oldest one and in some
                 * moment Nokia can remove it from firmware
                 */
/* 		if (Entry->Type == GSM_CAL_MEETING) { */
		if (Entry->Type == GSM_CAL_MEETING || Entry->Type == GSM_CAL_CALL || Entry->Type == GSM_CAL_MEMO) {
			error=N71_65_AddCalendar1(s, Entry, &s->Phone.Data.Priv.N6510.FirstCalendarPos);
		} else {
			error=N71_65_AddCalendar2(s,Entry);
		}
	}
	if (error != ERR_NONE) return error;

	error=N6510_GetCalendarInfo3(s, &Priv->LastCalendar,0);
	memcpy(&LastCalendar2,&Priv->LastCalendar,sizeof(GSM_NOKIACalToDoLocations));
	if (error != ERR_NONE) return error;

	smprintf(s,"Number of entries: %i %i\n",LastCalendar1.Number,LastCalendar2.Number);

	for(i=0;i<LastCalendar2.Number;i++) {
		found = true;
		for(j=0;j<LastCalendar1.Number;j++) {
			if (LastCalendar1.Location[j] == LastCalendar2.Location[i]) {
				found = false;
				break;
			}
		}
		if (found) {
			Note.Location = LastCalendar2.Location[i];
			error=N6510_PrivGetCalendar3(s, &Note, true, &LastCalendarYear);
			if (error != ERR_NONE) return error;

			error=N71_65_DelCalendar(s, &Note);
			if (error != ERR_NONE) return error;

			smprintf(s, "Ending finding note ID\n");

			for(j=0;j<Priv->CalendarIconsNum;j++) {
				if (Priv->CalendarIconsTypes[j] == Entry->Type) {
					*ID = Priv->CalendarIcons[j];
					return ERR_NONE;
				}
			}
			return ERR_UNKNOWN;
		}
	}

	return ERR_UNKNOWN;
}

/* method 3 */
GSM_Error N6510_ReplyAddCalendar3(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Calendar note added\n");
	return ERR_NONE;
}

/* method 3 */
static GSM_Error N6510_AddCalendar3(GSM_StateMachine *s, GSM_CalendarEntry *Note, int *FirstCalendarPos)
{
	GSM_CalendarNoteType	NoteType, OldNoteType;
	time_t     		t_time1,t_time2;
	long			diff;
 	GSM_Error		error;
	GSM_DateTime		DT,date_time;
 	int 			Text, Time, Alarm, Phone, EndTime, Location, count=54;
	unsigned char 		req[5000] = {
		N6110_FRAME_HEADER, 0x65,
		0x00,					/* 0 = calendar, 1 = todo 		*/
		0x00, 0x00, 0x00,
		0x00, 0x00,                             /* location 	    			*/
		0x00, 0x00, 0x00, 0x00,
		0xFF, 0xFF, 0xFF, 0xFF,			/* alarm 	    			*/
		0x80, 0x00, 0x00,
		0x01,					/* note icon	    			*/
		0xFF, 0xFF, 0xFF, 0xFF,			/* alarm type       			*/
		0x00,					/* 0x02 or 0x00     			*/
		0x01, 					/* note type	    			*/
		0x07, 0xD0, 0x01, 0x12, 0x0C, 0x00, 	/* start date/time  			*/
		0x07, 0xD0, 0x01, 0x12, 0x0C, 0x00, 	/* end date/time    			*/
		0x00, 0x00,				/* recurrance	    			*/
		0x00, 0x00,				/* birth year	    			*/
		0x20,					/* ToDo priority 			*/
		0x00,					/* ToDo completed ?			*/
		0x00, 0x00, 				/* How many times repeat cal note or 0 	*/
		0x00,
		0x00,					/* note text length 			*/
		0x00,					/* phone length/meeting place		*/
		0x00, 0x00, 0x00};

	error=N6510_GetCalendarNotePos3(s);
	if (error!=ERR_NONE) return error;
	Note->Location = *FirstCalendarPos;
	req[8] = *FirstCalendarPos/256;
	req[9] = *FirstCalendarPos%256;

 	NoteType = N71_65_FindCalendarType(Note->Type, s->Phone.Data.ModelInfo);

	switch(NoteType) {
		case GSM_CAL_REMINDER 	: req[27]=0x00; req[26]=0x02; break;
		case GSM_CAL_MEETING  	: req[27]=0x01; break;
		case GSM_CAL_CALL     	: req[27]=0x02; break;
		case GSM_CAL_BIRTHDAY 	: req[27]=0x04; break;
		case GSM_CAL_MEMO     	: req[27]=0x08; break;
		default		  	: return ERR_UNKNOWN;
	}

	OldNoteType = Note->Type;
	Note->Type  = NoteType;
	error=N6510_FindCalendarIconID3(s, Note, &req[21]);
	Note->Type  = OldNoteType;
	if (error!=ERR_NONE) return error;

	GSM_CalendarFindDefaultTextTimeAlarmPhone(Note, &Text, &Time, &Alarm, &Phone, &EndTime, &Location);

	if (Time == -1) {
		smprintf(s,"No time!\n");
		return ERR_UNKNOWN;
	}
	smprintf(s, "Time OK\n");
	memcpy(&DT,&Note->Entries[Time].Date,sizeof(GSM_DateTime));
	req[28]	= DT.Year / 256;
	req[29]	= DT.Year % 256;
	req[30]	= DT.Month;
	req[31]	= DT.Day;
	req[32]	= DT.Hour;
	req[33]	= DT.Minute;

	if (NoteType == GSM_CAL_BIRTHDAY) {
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
		req[28]	= date_time.Year / 256;
		req[29]	= date_time.Year % 256;
		if (DT.Year == 0) {
			req[42]	= 0xff;
			req[43]	= 0xff;
		} else {
			req[42]	= DT.Year / 256;
			req[43]	= DT.Year % 256;
		}
	}

	if (EndTime != -1) memcpy(&DT,&Note->Entries[EndTime].Date,sizeof(GSM_DateTime));

	req[34]	= DT.Year / 256;
	req[35]	= DT.Year % 256;
	req[36]	= DT.Month;
	req[37]	= DT.Day;
	req[38]	= DT.Hour;
	req[39]	= DT.Minute;
	if (NoteType == GSM_CAL_BIRTHDAY) {
		req[34]	= date_time.Year / 256;
		req[35]	= date_time.Year % 256;
	}

	GSM_SetCalendarRecurranceRepeat(req+40, req+52, Note);

	if (Alarm != -1) {
		memcpy(&DT,&Note->Entries[Time].Date,sizeof(GSM_DateTime));
		if (Note->Entries[Alarm].EntryType == CAL_SILENT_ALARM_DATETIME) {
			req[22] = 0x00; req[23] = 0x00; req[24] = 0x00; req[25] = 0x00;
		}
		if (NoteType == GSM_CAL_BIRTHDAY) DT.Year = date_time.Year;
		t_time2   = Fill_Time_T(DT);
		t_time1   = Fill_Time_T(Note->Entries[Alarm].Date);
		diff	  = (t_time1-t_time2)/60;

		smprintf(s, "  Difference : %li seconds or minutes\n", -diff);
		req[14] = (unsigned char)(-diff >> 24);
		req[15] = (unsigned char)(-diff >> 16);
		req[16] = (unsigned char)(-diff >> 8);
		req[17] = (unsigned char)(-diff);
	}

	if (Text != -1) {
		req[48] = UnicodeLength(Note->Entries[Text].Text) / 256;
		req[49] = UnicodeLength(Note->Entries[Text].Text) % 256;
		CopyUnicodeString(req + 54, Note->Entries[Text].Text);
		count += req[49] * 2 + (req[48] * 256) * 2;
	}

	if (Phone != -1 && NoteType == GSM_CAL_CALL) {
		req[50] = UnicodeLength(Note->Entries[Phone].Text);
		CopyUnicodeString(req+54+req[49]*2,Note->Entries[Phone].Text);
		count+= req[50]*2;
	}

	if (Location != -1 && NoteType == GSM_CAL_MEETING) {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_CAL62) ||
		    GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_CAL65) ||
		    GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_CAL35)) {
		} else {
			req[50] = UnicodeLength(Note->Entries[Location].Text);
			CopyUnicodeString(req+54+req[49]*2,Note->Entries[Location].Text);
			count+= req[50]*2;
		}
	}

	req[count++] = 0x00;

	smprintf(s, "Writing calendar note method 3\n");
	return GSM_WaitFor (s, req, count, 0x13, 4, ID_SetCalendarNote);
}

GSM_Error N6510_GetNextCalendar(GSM_StateMachine *s,  GSM_CalendarEntry *Note, bool start)
{
#ifdef GSM_FORCE_DCT4_CALENDAR_6210
    	/* Method 1. Some features missed. Not working with some notes in 3510 */
	return N71_65_GetNextCalendar1(s,Note,start,&s->Phone.Data.Priv.N6510.LastCalendar,&s->Phone.Data.Priv.N6510.LastCalendarYear,&s->Phone.Data.Priv.N6510.LastCalendarPos);
#endif

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_CAL62)) {
		/* Method 1. Some features missed. Not working with some notes in 3510 */
		return N71_65_GetNextCalendar1(s,Note,start,&s->Phone.Data.Priv.N6510.LastCalendar,&s->Phone.Data.Priv.N6510.LastCalendarYear,&s->Phone.Data.Priv.N6510.LastCalendarPos);

	    	/* Method 2. In known phones texts of notes cut to 50 chars. Some features missed */
/* 		return N71_65_GetNextCalendar2(s,Note,start,&s->Phone.Data.Priv.N6510.LastCalendarYear,&s->Phone.Data.Priv.N6510.LastCalendarPos); */
	} else {
	    	/* Method 3. All DCT4 features supported. Not supported by 8910 */
	    	return N6510_GetNextCalendar3(s,Note,start,&s->Phone.Data.Priv.N6510.LastCalendar,&s->Phone.Data.Priv.N6510.LastCalendarYear,&s->Phone.Data.Priv.N6510.LastCalendarPos);
	}
}

GSM_Error N6510_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *Status)
{
	GSM_Error error;

#ifdef GSM_FORCE_DCT4_CALENDAR_6210
        /* Method 1 */
	error=N71_65_GetCalendarInfo1(s, &s->Phone.Data.Priv.N6510.LastCalendar);
	if (error!=ERR_NONE) return error;
	Status->Used = s->Phone.Data.Priv.N6510.LastCalendar.Number;
	return ERR_NONE;
#endif

	/**
	 * @todo This should be acquired from phone
	 */
	Status->Free = 100;

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_CAL62)) {
	        /* Method 1 */
		error=N71_65_GetCalendarInfo1(s, &s->Phone.Data.Priv.N6510.LastCalendar);
		if (error!=ERR_NONE) return error;
		Status->Used = s->Phone.Data.Priv.N6510.LastCalendar.Number;
		return ERR_NONE;

	    	/* Method 2 */
/* 		return ERR_NOTSUPPORTED; */
	} else {
		/* Method 3 */
		error=N6510_GetCalendarInfo3(s,&s->Phone.Data.Priv.N6510.LastCalendar,0);
		if (error!=ERR_NONE) return error;
		Status->Used = s->Phone.Data.Priv.N6510.LastCalendar.Number;
		return ERR_NONE;
	}
}

GSM_Error N6510_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
#ifdef GSM_FORCE_DCT4_CALENDAR_6210
	return N71_65_AddCalendar2(s,Note);
#endif

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_CAL62)) {
		return N71_65_AddCalendar2(s,Note);
/* 		return N71_65_AddCalendar1(s, Note, &s->Phone.Data.Priv.N6510.FirstCalendarPos); */
	} else {
		/* Method 3. All DCT4 features supported. Not supported by 8910 */
		return N6510_AddCalendar3(s, Note, &s->Phone.Data.Priv.N6510.FirstCalendarPos);
	}
}

GSM_Error N6510_ReplyGetNoteInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return N6510_ReplyGetCalendarInfo3(msg, s, &s->Phone.Data.Priv.N6510.LastNote);
}

GSM_Error N6510_ReplyGetNote(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int len;

	smprintf(s, "Note received\n");
	len = msg.Buffer[50] * 256 + msg.Buffer[51];
	if (len > GSM_MAX_NOTE_TEXT_LENGTH) {
		smprintf(s, "Note too long (%d), truncating to %d\n", len, GSM_MAX_NOTE_TEXT_LENGTH);
		len = GSM_MAX_NOTE_TEXT_LENGTH;
	}
	memcpy(s->Phone.Data.Note->Text,
		msg.Buffer + 54,
		len * 2);
	s->Phone.Data.Note->Text[len * 2] = 0;
	s->Phone.Data.Note->Text[(len * 2) + 1] = 0;
	return ERR_NONE;
}

GSM_Error N6510_GetNextNote(GSM_StateMachine *s, GSM_NoteEntry *Note, bool start)
{
	GSM_Error 			error;
	GSM_NOKIACalToDoLocations	*LastNote = &s->Phone.Data.Priv.N6510.LastNote;

	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOTES)) return ERR_NOTSUPPORTED;

	if (start) {
		error=N6510_GetCalendarInfo3(s,LastNote,2);
		if (error!=ERR_NONE) return error;
		Note->Location = 1;
	} else {
		Note->Location++;
	}

	if (Note->Location > LastNote->Number) return ERR_EMPTY;

	s->Phone.Data.Note = Note;
	smprintf(s, "Getting note\n");
	return N6510_PrivGetGenericCalendar3(s, LastNote->Location[Note->Location-1], ID_GetNote);
}

GSM_Error N6510_DeleteNote(GSM_StateMachine *s, GSM_NoteEntry *Not)
{
	GSM_Error 			error;
	GSM_NOKIACalToDoLocations	*LastNote = &s->Phone.Data.Priv.N6510.LastNote;
	GSM_CalendarEntry		Note;

	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOTES)) return ERR_NOTSUPPORTED;

	error=N6510_GetCalendarInfo3(s,LastNote,2);
	if (error!=ERR_NONE) return error;

	smprintf(s, "Deleting Note\n");

	if (Not->Location > LastNote->Number || Not->Location == 0) return ERR_INVALIDLOCATION;

	Note.Location = LastNote->Location[Not->Location-1];
	return N71_65_DelCalendar(s,&Note);
}

GSM_Error N6510_ReplyGetNoteFirstLoc(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "First Note location: %i\n",msg.Buffer[8]*256+msg.Buffer[9]);
	s->Phone.Data.Note->Location = msg.Buffer[8]*256+msg.Buffer[9];
	return ERR_NONE;
}

GSM_Error N6510_ReplyAddNote(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Note added\n");
	return ERR_NONE;
}

GSM_Error N6510_AddNote(GSM_StateMachine *s, GSM_NoteEntry *Not)
{
	GSM_Error		error;
	int 			count=54;
	unsigned char 		reqLoc[] = {N6110_FRAME_HEADER, 0x95,
					    0x02};	/* 1 = todo, 2 = note */
	unsigned char 		req[GSM_MAX_NOTE_TEXT_LENGTH + 500] = {
		N6110_FRAME_HEADER, 0x65,
		0x02,					/* 0 = calendar, 1 = todo, 2 = note 	*/
		0x00, 0x00, 0x00,
		0x00, 0x00,                             /* location 	    			*/
		0x00, 0x00, 0x00, 0x00,
		0xFF, 0xFF, 0xFF, 0xFF,			/* alarm 	    			*/
		0x80, 0x00, 0x00,
		0xA9,					/* note icon	    			*/
		0x00, 0x00, 0x00, 0x00,			/* alarm type       			*/
		0x00,					/* 0x02 or 0x00     			*/
		0x80, 					/* note type	    			*/
		0x07, 0xD2, 0x01, 0x01, 0x00, 0x00, 	/* start date/time  			*/
		0x07, 0xD2, 0x01, 0x11, 0x00, 0x00, 	/* end date/time    			*/
		0x00, 0x00,				/* recurrance	    			*/
		0xFF, 0xFF,				/* birth year	    			*/
		0x00,					/* ToDo priority 			*/
		0x00,					/* ToDo completed ?			*/
		0x00, 0x00, 0x00,
		0x00,					/* note text length 			*/
		0x00,					/* phone length/meeting place		*/
		0x00, 0x00, 0x00};
	size_t length;

	s->Phone.Data.Note = Not;

	smprintf(s, "Getting first free Note location\n");
	error = GSM_WaitFor (s, reqLoc, 5, 0x13, 4, ID_SetNote);
	if (error!=ERR_NONE) return error;
	req[8] = Not->Location / 256;
	req[9] = Not->Location % 256;

	length = UnicodeLength(Not->Text);
	req[48] = length / 256;
	req[49] = length % 256;
	CopyUnicodeString(req + 54, Not->Text);
	count += length * 2;

	req[count++] = 0x00;

	smprintf(s, "Adding Note\n");
	return GSM_WaitFor (s, req, count, 0x13, 4, ID_SetNote);
}

GSM_Error N6510_GetNoteStatus(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	GSM_NOKIACalToDoLocations	*LastNote = &s->Phone.Data.Priv.N6510.LastNote;
	GSM_Error			error;

	error = N6510_GetCalendarInfo3(s,LastNote,2);
	if (error!=ERR_NONE) return error;

	status->Used = LastNote->Number;
	return ERR_NONE;
}

/* ToDo support - 6310 style */
GSM_Error N6510_ReplyGetToDoStatus1(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int				i;
	GSM_NOKIACalToDoLocations	*Last = &s->Phone.Data.Priv.N6510.LastToDo;

	smprintf(s, "TODO locations received\n");
	Last->Number=msg.Buffer[6]*256+msg.Buffer[7];
	smprintf(s, "Number of Entries: %i\n",Last->Number);
	smprintf(s, "Locations: ");
	for (i=0;i<Last->Number;i++) {
		Last->Location[i]=msg.Buffer[12+(i*4)]*256+msg.Buffer[(i*4)+13];
		smprintf(s, "%i ",Last->Location[i]);
	}
	smprintf(s, "\n");
	return ERR_NONE;
}

/* ToDo support - 6310 style */
static GSM_Error N6510_GetToDoStatus1(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	GSM_Error 			error;
	GSM_NOKIACalToDoLocations	*LastToDo = &s->Phone.Data.Priv.N6510.LastToDo;
	unsigned char reqLoc[] = {
			N6110_FRAME_HEADER,
			0x15, 0x01, 0x00, 0x00,
			0x00, 0x00, 0x00};

	smprintf(s, "Getting ToDo locations\n");
	error = GSM_WaitFor (s, reqLoc, 10, 0x55, 4, ID_GetToDo);
	if (error != ERR_NONE) return error;

	status->Used = LastToDo->Number;
	/**
	 * @todo This should be acquired from phone
	 */
	status->Free = 100;
	return ERR_NONE;
}

static GSM_Error N6510_GetToDoStatus2(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	GSM_NOKIACalToDoLocations	*LastToDo = &s->Phone.Data.Priv.N6510.LastToDo;
	GSM_Error			error;

	error = N6510_GetCalendarInfo3(s,LastToDo,1);
	if (error!=ERR_NONE) return error;

	status->Used = LastToDo->Number;
	/**
	 * @todo This should be acquired from phone
	 */
	status->Free = 100;
	return ERR_NONE;
}

GSM_Error N6510_GetToDoStatus(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	status->Used = 0;

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_TODO63)) {
		return N6510_GetToDoStatus1(s, status);
	} else if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_TODO66)) {
		return N6510_GetToDoStatus2(s, status);
	} else {
		return ERR_NOTSUPPORTED;
	}
}

/* ToDo support - 6310 style */
GSM_Error N6510_ReplyGetToDo1(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_ToDoEntry *Last = s->Phone.Data.ToDo;

	smprintf(s, "TODO received method 1\n");

	/**
	 * @todo There might be better type.
	 */
	Last->Type = GSM_CAL_MEMO;

	switch (msg.Buffer[4]) {
		case 1  : Last->Priority = GSM_Priority_High; 	break;
		case 2  : Last->Priority = GSM_Priority_Medium; break;
		case 3  : Last->Priority = GSM_Priority_Low; 	break;
		default	: return ERR_UNKNOWN;
	}
	smprintf(s, "Priority: %i\n",msg.Buffer[4]);

	CopyUnicodeString(Last->Entries[0].Text,msg.Buffer+14);
 	Last->Entries[0].EntryType = TODO_TEXT;
	Last->EntriesNum		 = 1;
	smprintf(s, "Text: \"%s\"\n",DecodeUnicodeString(Last->Entries[0].Text));

	return ERR_NONE;
}

/* ToDo support - 6310 style */
static GSM_Error N6510_GetNextToDo1(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool refresh)
{
	GSM_Error 			error;
	GSM_ToDoStatus 			status;
	GSM_NOKIACalToDoLocations	*LastToDo = &s->Phone.Data.Priv.N6510.LastToDo;
	unsigned char reqGet[] = {
			N6110_FRAME_HEADER,
			0x03, 0x00, 0x00, 0x80, 0x00,
			0x00, 0x17};		/* Location */

	if (refresh) {
		error = N6510_GetToDoStatus(s, &status);
		if (error != ERR_NONE) return error;
		ToDo->Location = 1;
	} else {
		ToDo->Location++;
	}
	if (ToDo->Location > LastToDo->Number) return ERR_EMPTY;

	reqGet[8] = LastToDo->Location[ToDo->Location-1] / 256;
	reqGet[9] = LastToDo->Location[ToDo->Location-1] % 256;
	s->Phone.Data.ToDo = ToDo;
	smprintf(s, "Getting ToDo\n");
	return GSM_WaitFor (s, reqGet, 10, 0x55, 4, ID_GetToDo);
}

GSM_Error N6510_ReplyGetToDoStatus2(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return N6510_ReplyGetCalendarInfo3(msg, s, &s->Phone.Data.Priv.N6510.LastToDo);
}

/* Similiar to getting calendar method 3 */
GSM_Error N6510_ReplyGetToDo2(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_ToDoEntry 		*Last = s->Phone.Data.ToDo;
	GSM_DateTime		Date;
	unsigned long		diff;
	int len;

	smprintf(s, "ToDo received method 2\n");

	/**
	 * @todo There might be better type.
	 */
	Last->Type = GSM_CAL_MEMO;

	switch (msg.Buffer[44]) {
		case 0x10: Last->Priority = GSM_Priority_Low; 		break;
		case 0x20: Last->Priority = GSM_Priority_Medium; 	break;
		case 0x30: Last->Priority = GSM_Priority_High; 		break;
		default	 : return ERR_UNKNOWN;
	}

	len = msg.Buffer[50] * 256 + msg.Buffer[51];
	if (len > GSM_MAX_TODO_TEXT_LENGTH) {
		smprintf(s, "Todo text too long (%d), truncating to %d\n", len, GSM_MAX_TODO_TEXT_LENGTH);
		len = GSM_MAX_TODO_TEXT_LENGTH;
	}
	memcpy(Last->Entries[0].Text,
		msg.Buffer + 54,
		len * 2);
	Last->Entries[0].Text[len * 2] = 0;
	Last->Entries[0].Text[(len * 2) + 1] = 0;
    	Last->Entries[0].EntryType = TODO_TEXT;
	smprintf(s, "Text: \"%s\"\n",DecodeUnicodeString(Last->Entries[0].Text));

	smprintf(s,"EndTime: %04i-%02i-%02i %02i:%02i\n",
		msg.Buffer[34]*256+msg.Buffer[35],
		msg.Buffer[36],msg.Buffer[37],msg.Buffer[38],
		msg.Buffer[39]);
	Date.Year 	= msg.Buffer[34]*256+msg.Buffer[35];
	Date.Month 	= msg.Buffer[36];
	Date.Day 	= msg.Buffer[37];
	Date.Hour 	= msg.Buffer[38];
	Date.Minute 	= msg.Buffer[39];
	Date.Second	= 0;
	Last->Entries[1].EntryType = TODO_END_DATETIME;
	memcpy(&Last->Entries[1].Date,&Date,sizeof(GSM_DateTime));

	smprintf(s,"StartTime: %04i-%02i-%02i %02i:%02i\n",
		msg.Buffer[28]*256+msg.Buffer[29],
		msg.Buffer[30],msg.Buffer[31],msg.Buffer[32],
		msg.Buffer[33]);
	Date.Year 	= msg.Buffer[28]*256+msg.Buffer[29];
	Date.Month 	= msg.Buffer[30];
	Date.Day 	= msg.Buffer[31];
	Date.Hour 	= msg.Buffer[32];
	Date.Minute 	= msg.Buffer[33];
	Date.Second	= 0;

	Last->EntriesNum = 2;

	if (msg.Buffer[45] == 0x01) {
		Last->Entries[2].Number		= msg.Buffer[45];
	    	Last->Entries[2].EntryType 	= TODO_COMPLETED;
		Last->EntriesNum++;
		smprintf(s,"Completed\n");
	}

	if (msg.Buffer[14] == 0xFF && msg.Buffer[15] == 0xFF && msg.Buffer[16] == 0xff && msg.Buffer[17] == 0xff) {
		smprintf(s, "No alarm\n");
	} else {
		diff  = ((unsigned int)msg.Buffer[14]) << 24;
		diff += ((unsigned int)msg.Buffer[15]) << 16;
		diff += ((unsigned int)msg.Buffer[16]) << 8;
		diff += msg.Buffer[17];

		memcpy(&Last->Entries[Last->EntriesNum].Date,&Date,sizeof(GSM_DateTime));
		GetTimeDifference(diff, &Last->Entries[Last->EntriesNum].Date, false, 60);
		smprintf(s, "Alarm date   : %02i-%02i-%04i %02i:%02i:%02i\n",
			Last->Entries[Last->EntriesNum].Date.Day,   Last->Entries[Last->EntriesNum].Date.Month,
			Last->Entries[Last->EntriesNum].Date.Year,  Last->Entries[Last->EntriesNum].Date.Hour,
			Last->Entries[Last->EntriesNum].Date.Minute,Last->Entries[Last->EntriesNum].Date.Second);

		Last->Entries[Last->EntriesNum].EntryType = TODO_ALARM_DATETIME;
		if (msg.Buffer[22]==0x00 && msg.Buffer[23]==0x00 &&
		    msg.Buffer[24]==0x00 && msg.Buffer[25]==0x00)
		{
			Last->Entries[Last->EntriesNum].EntryType = TODO_SILENT_ALARM_DATETIME;
			smprintf(s, "Alarm type   : Silent\n");
		}
		Last->EntriesNum++;
	}

	return ERR_NONE;
}

/* ToDo support - 6610 style */
static GSM_Error N6510_GetNextToDo2(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool refresh)
{
	GSM_Error 			error;
	GSM_NOKIACalToDoLocations	*LastToDo = &s->Phone.Data.Priv.N6510.LastToDo;

	if (refresh) {
		error=N6510_GetCalendarInfo3(s,LastToDo,1);
		if (error!=ERR_NONE) return error;
		ToDo->Location = 1;
	} else {
		ToDo->Location++;
	}

	if (ToDo->Location > LastToDo->Number) return ERR_EMPTY;

	s->Phone.Data.ToDo = ToDo;
	smprintf(s, "Getting todo method 2\n");
	return N6510_PrivGetGenericCalendar3(s, LastToDo->Location[ToDo->Location-1], ID_GetToDo);
}

GSM_Error N6510_GetNextToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool refresh)
{
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_TODO63)) {
		return N6510_GetNextToDo1(s, ToDo, refresh);
	} else if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_TODO66)) {
		return N6510_GetNextToDo2(s, ToDo, refresh);
	} else {
		return ERR_NOTSUPPORTED;
	}
}

/* ToDo support - 6310 style */
GSM_Error N6510_ReplyDeleteAllToDo1(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "All TODO deleted\n");
	return ERR_NONE;
}

/* ToDo support - 6310 style */
GSM_Error N6510_DeleteAllToDo1(GSM_StateMachine *s)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x11};

	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_TODO63)) {
		return ERR_NOTSUPPORTED;
	}

	smprintf(s, "Deleting all ToDo method 1\n");
	return GSM_WaitFor (s, req, 4, 0x55, 4, ID_DeleteAllToDo);
}

GSM_Error N6510_DeleteToDo2(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error 			error;
	GSM_NOKIACalToDoLocations	*LastToDo = &s->Phone.Data.Priv.N6510.LastToDo;
	GSM_CalendarEntry		Note;

	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_TODO66)) {
		return ERR_NOTSUPPORTED;
	}

	error=N6510_GetCalendarInfo3(s,LastToDo,1);
	if (error!=ERR_NONE) return error;

	smprintf(s, "Deleting ToDo method 2\n");

	if (ToDo->Location > LastToDo->Number || ToDo->Location == 0) return ERR_INVALIDLOCATION;

	Note.Location = LastToDo->Location[ToDo->Location-1];
	return N71_65_DelCalendar(s,&Note);
}

/* ToDo support - 6310 style */
GSM_Error N6510_ReplyGetToDoFirstLoc1(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "TODO first location received method 1: %02x\n",msg.Buffer[9]);
	s->Phone.Data.ToDo->Location = msg.Buffer[9];
	return ERR_NONE;
}

/* ToDo support - 6310 style */
GSM_Error N6510_ReplyAddToDo1(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "TODO set OK\n");
	return ERR_NONE;
}

/* ToDo support - 6310 style */
static GSM_Error N6510_AddToDo1(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
 	int 			Text, Alarm, EndTime, Completed, ulen, Phone;
	GSM_Error		error;
	unsigned char 		reqLoc[] 	= {N6110_FRAME_HEADER, 0x0F};
	unsigned char 		reqSet[500] 	= {
		N6110_FRAME_HEADER, 0x01,
		0x03,		/* Priority */
		0x00,		/* Length of text */
		0x80,0x00,0x00,
		0x18};		/* Location */

	s->Phone.Data.ToDo = ToDo;

	smprintf(s, "Getting first ToDo location\n");
	error = GSM_WaitFor (s, reqLoc, 4, 0x55, 4, ID_SetToDo);
	if (error != ERR_NONE) return error;
	reqSet[9] = ToDo->Location;

	switch (ToDo->Priority) {
		case GSM_Priority_Low	: reqSet[4] = 3; break;
		case GSM_Priority_Medium: reqSet[4] = 2; break;
		case GSM_Priority_High	: reqSet[4] = 1; break;
		default: break;
	}

	GSM_ToDoFindDefaultTextTimeAlarmCompleted(ToDo, &Text, &Alarm, &Completed, &EndTime, &Phone);

    	if (Text == -1) return ERR_NOTSUPPORTED; /* XXX: shouldn't this be handled different way? */
    	ulen = UnicodeLength(ToDo->Entries[Text].Text);
	reqSet[5] = ulen+1;
	CopyUnicodeString(reqSet+10,ToDo->Entries[Text].Text);
	reqSet[10+ulen*2] 	= 0x00;
	reqSet[10+ulen*2+1] 	= 0x00;
	smprintf(s, "Adding ToDo method 1\n");
	return GSM_WaitFor (s, reqSet, 12+ulen*2, 0x55, 4, ID_SetToDo);
}

GSM_Error N6510_ReplyAddToDo2(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "ToDo added method 2\n");
	return ERR_NONE;
}

GSM_Error N6510_ReplyGetToDoFirstLoc2(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "First ToDo location method 2: %i\n",msg.Buffer[8]*256+msg.Buffer[9]);
	s->Phone.Data.ToDo->Location = msg.Buffer[8]*256+msg.Buffer[9];
	return ERR_NONE;
}

static GSM_Error N6510_AddToDo2(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_CalendarEntry	Note;
	time_t     		t_time1,t_time2;
	long			diff;
 	GSM_Error		error;
	GSM_DateTime		DT;
 	int 			Text, Alarm, EndTime, Completed, count=54, Phone;
	unsigned char 		reqLoc[] = {N6110_FRAME_HEADER, 0x95,
					    0x01};	/* 1 = todo, 2 = note */
	unsigned char 		req[GSM_MAX_TODO_TEXT_LENGTH + 500] = {
		N6110_FRAME_HEADER, 0x65,
		0x01,					/* 0 = calendar, 1 = todo 		*/
		0x00, 0x00, 0x00,
		0x00, 0x00,                             /* location 	    			*/
		0x00, 0x00, 0x00, 0x00,
		0xFF, 0xFF, 0xFF, 0xFF,			/* alarm 	    			*/
		0x80, 0x00, 0x00,
		0x01,					/* note icon	    			*/
		0xFF, 0xFF, 0xFF, 0xFF,			/* alarm type       			*/
		0x00,					/* 0x02 or 0x00     			*/
		0x01, 					/* note type	    			*/
		0x07, 0xD0, 0x01, 0x12, 0x0C, 0x00, 	/* start date/time  			*/
		0x07, 0xD0, 0x01, 0x12, 0x0C, 0x00, 	/* end date/time    			*/
		0x00, 0x00,				/* recurrance	    			*/
		0x00, 0x00,				/* birth year	    			*/
		0x20,					/* ToDo priority 			*/
		0x00,					/* ToDo completed ?			*/
		0x00, 0x00, 				/* How many times repeat cal note or 0 	*/
		0x00,
		0x00,					/* note text length 			*/
		0x00,					/* phone length/meeting place		*/
		0x00, 0x00, 0x00};

	s->Phone.Data.ToDo = ToDo;

	smprintf(s, "Getting first free ToDo location method 2\n");
	error = GSM_WaitFor (s, reqLoc, 5, 0x13, 4, ID_SetToDo);
	if (error!=ERR_NONE) return error;
	req[8] = ToDo->Location/256;
	req[9] = ToDo->Location%256;

	Note.Type = GSM_CAL_MEETING;
	DT.Year = 2004; DT.Month  = 1; 	DT.Day 	  = 1;
	DT.Hour = 12; 	DT.Minute = 12; DT.Second = 0;
	memcpy(&Note.Entries[0].Date,&DT,sizeof(GSM_DateTime));
	Note.Entries[0].EntryType 	= CAL_START_DATETIME;
	memcpy(&Note.Entries[1].Date,&DT,sizeof(GSM_DateTime));
	Note.Entries[1].EntryType 	= CAL_END_DATETIME;
	EncodeUnicode(Note.Entries[2].Text,"ala",3);
	Note.Entries[2].EntryType 	= CAL_TEXT;
	Note.EntriesNum 		= 3;
	error=N6510_FindCalendarIconID3(s, &Note, &req[21]);
	if (error!=ERR_NONE) return error;

	switch (ToDo->Priority) {
		case GSM_Priority_Low	: req[44] = 0x10; break;
		case GSM_Priority_Medium: req[44] = 0x20; break;
		case GSM_Priority_High	: req[44] = 0x30; break;
		default: break;
	}

	GSM_ToDoFindDefaultTextTimeAlarmCompleted(ToDo, &Text, &Alarm, &Completed, &EndTime, &Phone);

	if (Completed != -1) req[45] = 0x01;

	if (EndTime == -1) {
		GSM_GetCurrentDateTime(&DT);
	} else {
		memcpy(&DT,&ToDo->Entries[EndTime].Date,sizeof(GSM_DateTime));
	}
	/*Start time*/
	req[28]	= DT.Year / 256;
	req[29]	= DT.Year % 256;
	req[30]	= DT.Month;
	req[31]	= DT.Day;
	req[32]	= DT.Hour;
	req[33]	= DT.Minute;
	/*End time*/
	req[34]	= DT.Year / 256;
	req[35]	= DT.Year % 256;
	req[36]	= DT.Month;
	req[37]	= DT.Day;
	req[38]	= DT.Hour;
	req[39]	= DT.Minute;

	if (Alarm != -1) {
		if (ToDo->Entries[Alarm].EntryType == CAL_SILENT_ALARM_DATETIME)
		{
			req[22] = 0x00; req[23] = 0x00; req[24] = 0x00; req[25] = 0x00;
		}
		t_time2   = Fill_Time_T(DT);
		t_time1   = Fill_Time_T(ToDo->Entries[Alarm].Date);
		diff	  = (t_time1-t_time2)/60;

		smprintf(s, "  Difference : %li seconds or minutes\n", -diff);
		req[14] = (unsigned char)(-diff >> 24);
		req[15] = (unsigned char)(-diff >> 16);
		req[16] = (unsigned char)(-diff >> 8);
		req[17] = (unsigned char)(-diff);
	}

	if (Text != -1) {
		req[48] = UnicodeLength(ToDo->Entries[Text].Text) / 256;
		req[49] = UnicodeLength(ToDo->Entries[Text].Text) % 256;
		CopyUnicodeString(req + 54, ToDo->Entries[Text].Text);
		count += req[49] * 2 + (req[48] * 256) * 2;
	}

	req[count++] = 0x00;

	smprintf(s, "Adding ToDo method 2\n");
	return GSM_WaitFor (s, req, count, 0x13, 4, ID_SetToDo);
}

GSM_Error N6510_AddToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_TODO63)) {
		return N6510_AddToDo1(s, ToDo);
	} else if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_TODO66)) {
		return N6510_AddToDo2(s, ToDo);
	} else {
		return ERR_NOTSUPPORTED;
	}
}

GSM_Error N6510_ReplyGetCalendarSettings(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_CalendarSettings *sett = s->Phone.Data.CalendarSettings;

	switch (msg.Buffer[3]) {
	case 0x86:
		smprintf(s, "Auto deleting setting received\n");
		sett->AutoDelete = msg.Buffer[4];
		return ERR_NONE;
	case 0x8E:
		smprintf(s, "Start day for calendar received\n");
		switch(msg.Buffer[4]) {
		case 0x04:
			sett->StartDay = 1;
			return ERR_NONE;
		case 0x03:
			sett->StartDay = 6;
			return ERR_NONE;
		case 0x02:
			sett->StartDay = 7;
			return ERR_NONE;
		case 0x01:
			sett->StartDay = 1;
			return ERR_NONE;
		}
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error N6510_GetCalendarSettings(GSM_StateMachine *s, GSM_CalendarSettings *settings)
{
	GSM_Error	error;
	unsigned char 	req1[] = {N6110_FRAME_HEADER, 0x85};
	unsigned char 	req2[] = {N6110_FRAME_HEADER, 0x8D};

	s->Phone.Data.CalendarSettings = settings;

	smprintf(s, "Getting auto delete\n");
	error = GSM_WaitFor (s, req1, 4, 0x13, 4, ID_GetCalendarSettings);
	if (error != ERR_NONE) return error;

	smprintf(s, "Getting start day for week\n");
	return GSM_WaitFor (s, req2, 4, 0x13, 4, ID_GetCalendarSettings);
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
