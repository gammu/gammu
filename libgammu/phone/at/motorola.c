/* © 2007 - 2009 Michal Čihař */

/**
 * @file motorola.c
 * @author Michal Čihař
 */
/**
 * @ingroup Phone
 * @{
 */
/**
 * @addtogroup ATPhone
 * @{
 */

#include <gammu-config.h>

#ifdef GSM_ENABLE_ATGEN

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "../../service/gsmpbk.h"
#include "atgen.h"
#include <string.h>
//#include "../../misc/coding/coding.h"
#include "../../../helper/string.h"

typedef struct {
	char Command[20];
	int Mode;
} MOTOROLA_CommandInfo;

/**
 * AT Commands which can be issued only in one mode.
 *
 * List is based on c18 AT Commands document revision C.
 */
MOTOROLA_CommandInfo Commands[] = {
	{"+CGMI", 2},
	{"+CGMM", 2},
	{"+CGMR", 2},
	{"+CGSN", 2},
	{"+CSCS", 2},
	{"+CIMI", 2},
	{"+CNUM", 2},
	{"+CVHU", 2},
	{"+CHUP", 2},
	{"+CIND", 2},
	{"+CLCK", 2},
	{"D", 2},  /* We want voice call */
	{"H", 2},  /* We want voice call */
	{"A", 2},  /* We want voice call */
	{"+CRING", 2},
	{"+CLIP", 2},
	{"+CLIR", 2},
	{"+CCFC", 2},
	{"+CHLD", 2},
	{"+COLP", 2},
	{"+CCWA", 2},
	{"+CLCC", 2},
	{"+CPBS", 2},
	{"+CPBR", 2},
	{"+CPBF", 2},
	{"+CPBW", 2},
	{"+CCLK", 2},
	{"+CNMI", 2},
	{"+CMGD", 2},
	{"+CMSS", 2},
	{"+CSMS", 2},
	{"+CPMS", 2},
	{"+CMGF", 2},
	{"+CSDH", 2},
	{"+CMTI", 2},
	{"+CMGL", 2},
	{"+CMGR", 2},
	{"+CMGW", 2},
	{"+CSCA", 2},
	{"+COPS", 2},
	{"+CBC", 2},
	{"+CRTT", 2},
	{"+CMEE", 2},
	{"+CKPD", 2},

	{"+CHV", 0},
	{"+CDV", 0},
	{"+CPAS", 0},
	{"+CREG", 0},
	{"+CSQ", 0},
	{"+GCAP", 0},
	{"+CMUT", 0},
	{"+CIMSI", 0},

	{"", 0},
};

GSM_Error MOTOROLA_SetModeReply(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
		case AT_Reply_OK:
		case AT_Reply_Connect:
			/*
			 * The typo in next line (Unkown) is intentional,
			 * phone returns it this way.
			 */
			if (strstr(GetLineString(msg.Buffer, &Priv->Lines, 2), "Unkown mode value") != NULL) {
				return ERR_NOTSUPPORTED;
			}
			return ERR_NONE;
		case AT_Reply_Error:
			return ERR_UNKNOWN;
		case AT_Reply_CMSError:
			return ATGEN_HandleCMSError(s);
		case AT_Reply_CMEError:
			return ATGEN_HandleCMEError(s);
		default:
			break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error MOTOROLA_SetMode(GSM_StateMachine *s, const char *command)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	MOTOROLA_CommandInfo *cmd;
	const char *realcmd;
	char buffer[30]={0};
	GSM_Error error = ERR_NONE;

	/* Do we need any mode switching? */
	if (!Priv->Mode) {
		return ERR_NONE;
	}

	/* We don't care about non AT commands */
	if (strncasecmp(command, "AT", 2) != 0) {
		return ERR_NONE;
	}

	/* Skip AT prefix */
	realcmd = command + 2;

	/* Do we have it in our list? */
	for (cmd = Commands; cmd->Command[0] != 0; cmd++) {
		if (strncasecmp(realcmd, cmd->Command, strlen(cmd->Command)) == 0) {
			break;
		}
	}

	/* Not found? */
	if (cmd->Command[0] == 0) {
		smprintf(s, "Nothing known about %s command, using current mode\n", command);
		return ERR_NONE;
	}

	/* Compare current mode */
	if (cmd->Mode == Priv->CurrentMode) {
		smprintf(s, "Already in mode %d\n", cmd->Mode);
		return ERR_NONE;
	}

	/* Switch mode */
	smprintf(s, "Switching to mode %d\n", cmd->Mode);
	sprintf(buffer, "AT+MODE=%d\r", cmd->Mode);
	error = GSM_WaitFor(s, buffer, strlen(buffer), 0x00, 100, ID_ModeSwitch);

	/* On succes we remember it */
	if (error == ERR_NONE) {

		/* We might need to restore charset as phone resets it */
		if (cmd->Mode == 2) {
			smprintf(s, "Waiting for banner...\n");

			/* Wait for banner */
			error = GSM_WaitForOnce(s, NULL, 0x00, 0x00, 40);

			if (error != ERR_NONE) {
				return error;
			}

			/* Check for banner result */
			if (Priv->CurrentMode != 2) {
				smprintf(s, "Failed to set mode 2!\n");
				return ERR_BUG;
			}

			/* Now we can work with phone */
			error = ATGEN_SetCharset(s, AT_PREF_CHARSET_RESET);
		} else {
			Priv->CurrentMode = cmd->Mode;
		}
	}
	return error;
}

/**
 * Catches +MBAN: reply.
 */
GSM_Error MOTOROLA_Banner(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	s->Phone.Data.Priv.ATGEN.CurrentMode = 2;
	return ERR_NONE;
}


GSM_Error MOTOROLA_ReplyGetMemoryInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
 	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;

	Priv->PBK_MPBR = AT_NOTAVAILABLE;

 	switch (Priv->ReplyState) {
 	case AT_Reply_OK:
		/* FIXME: does phone give also some useful infromation here? */
		Priv->PBK_MPBR = AT_AVAILABLE;

		return ERR_NONE;
	case AT_Reply_Error:
		return ERR_UNKNOWN;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
	        return ATGEN_HandleCMEError(s);
 	default:
		return ERR_UNKNOWNRESPONSE;
	}
}

GSM_Error MOTOROLA_ReplyGetMemory(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
 	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
 	GSM_MemoryEntry		*Memory = s->Phone.Data.Memory;
	GSM_Error error;
	const char *str;
	int number_type, entry_type;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
 		smprintf(s, "Phonebook entry received\n");
		Memory->EntriesNum = 2;
		Memory->Entries[0].AddError = ERR_NONE;
		Memory->Entries[0].VoiceTag = 0;
		Memory->Entries[0].SMSList[0] = 0;
		Memory->Entries[1].EntryType = PBK_Text_Name;
		Memory->Entries[1].AddError = ERR_NONE;
		Memory->Entries[1].VoiceTag = 0;
		Memory->Entries[1].SMSList[0] = 0;

		/* Get line from reply */
		str = GetLineString(msg.Buffer, &Priv->Lines, 2);

		/* Detect empty entry */
		if (strcmp(str, "OK") == 0) return ERR_EMPTY;

		/*
		 * Parse reply string
		 *
		 * +MPBR: 18,"user@domain.net",128,"Contact Name",6,0,255,0,0,1,255,255,0,"",0,0,"","","","","","","",""
		 */
		error = ATGEN_ParseReply(s, str,
					"+MPBR: @i, @p, @i, @s, @i, @0",
					&Memory->Location,
					Memory->Entries[0].Text, sizeof(Memory->Entries[0].Text),
					&number_type,
					Memory->Entries[1].Text, sizeof(Memory->Entries[1].Text),
					&entry_type);
		switch (entry_type) {
			case 0:
				Memory->Entries[0].EntryType = PBK_Number_Work;
				GSM_TweakInternationalNumber(Memory->Entries[0].Text, number_type);
				break;
			case 1:
				Memory->Entries[0].EntryType = PBK_Number_Home;
				GSM_TweakInternationalNumber(Memory->Entries[0].Text, number_type);
				break;
			case 2:
			case 10:
			case 11:
				Memory->Entries[0].EntryType = PBK_Number_General;
				GSM_TweakInternationalNumber(Memory->Entries[0].Text, number_type);
				break;
			case 3:
				Memory->Entries[0].EntryType = PBK_Number_Mobile;
				GSM_TweakInternationalNumber(Memory->Entries[0].Text, number_type);
				break;
			case 4:
				Memory->Entries[0].EntryType = PBK_Number_Fax;
				GSM_TweakInternationalNumber(Memory->Entries[0].Text, number_type);
				break;
			case 5:
				Memory->Entries[0].EntryType = PBK_Number_Pager;
				GSM_TweakInternationalNumber(Memory->Entries[0].Text, number_type);
				break;
			case 6:
				Memory->Entries[0].EntryType = PBK_Text_Email;
				break;
			case 7:
				Memory->Entries[0].EntryType = PBK_Text_Email; /* Mailing list */
				break;
			default:
				Memory->Entries[0].EntryType = PBK_Text_Note;
		}

		if (error != ERR_NONE) {
			return error;
		}
		return ERR_NONE;
	case AT_Reply_Error:
                return ERR_UNKNOWN;
	case AT_Reply_CMSError:
 	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
	        return ATGEN_HandleCMEError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error MOTOROLA_LockCalendar(GSM_StateMachine *s)
{
	GSM_Error error;

	ATGEN_WaitForAutoLen(s, "AT+MDBL=1\r", 0x00, 10, ID_SetCalendarNote);

	return error;
}

GSM_Error MOTOROLA_UnlockCalendar(GSM_StateMachine *s)
{
	GSM_Error error;

	ATGEN_WaitForAutoLen(s, "AT+MDBL=0\r", 0x00, 10, ID_SetCalendarNote);

	return error;
}

GSM_Error MOTOROLA_ReplyGetCalendarStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error error;
	int ignore;

	if (Priv->ReplyState != AT_Reply_OK) {
		switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
		case AT_Reply_Error:
			return ERR_NOTSUPPORTED;
		case AT_Reply_CMSError:
			return ATGEN_HandleCMSError(s);
		case AT_Reply_CMEError:
			return ATGEN_HandleCMEError(s);
		default:
			return ERR_UNKNOWNRESPONSE;
		}
	}

	/*
	 * Reply looks like:
	 * +MDBR: 500,1,64,8,2
	 * (max events that can be stored, number of stored events, ?, ?, ?)
	 */

	error = ATGEN_ParseReply(s,
		GetLineString(msg.Buffer, &Priv->Lines, 2),
		"+MDBR: @i, @i, @i, @i, @i",
		&s->Phone.Data.CalStatus->Free,
		&s->Phone.Data.CalStatus->Used,
		&ignore,
		&ignore,
		&ignore);
	if (error != ERR_NONE) return error;
	s->Phone.Data.CalStatus->Free += s->Phone.Data.CalStatus->Used;
	return ERR_NONE;
}

GSM_Error MOTOROLA_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *Status)
{
	GSM_Error error;

	s->Phone.Data.CalStatus = Status;

	ATGEN_WaitForAutoLen(s, "AT+MDBR=?\r", 0x00, 10, ID_GetCalendarNotesInfo);

	return error;
}

GSM_Error MOTOROLA_ParseCalendarSimple(GSM_StateMachine *s, const char *line)
{
	GSM_Error error;
	int ignore;
	GSM_CalendarEntry *Note = s->Phone.Data.Cal;
	int duration, repeat, has_time, has_alarm;
	/*
	 * Parse following format:
	 *
	 * +MDBR: 0,"Meeting",1,0,"17:00","02-24-2006",60,"00:00","00-00-2000",0
	 * +MDBR: 1,"Breakfast",1,1,"10:00","02-25-2006",60,"09:30","02-25-2006",2
	 * event num, description, time flag (if 0, start time is meaningless), alarm enabl
	 * ed flag (if 0, alarm time is meaningless), time, date, duration (mins), alarm ti
	 * me, alarm date, repeat type
	 *
	 * repeat type:
	 * 1 = daily
	 * 2 = weekly
	 * 3 = monthly on date
	 * 4 = monthly on day
	 * 5 = yearly
	 */
	Note->Type = GSM_CAL_MEMO;
	Note->Entries[0].EntryType = CAL_TEXT;
	Note->Entries[1].EntryType = CAL_START_DATETIME;
	Note->Entries[1].Date.Timezone = 0;
	Note->Entries[1].Date.Second = 0;
	Note->Entries[2].EntryType = CAL_TONE_ALARM_DATETIME;
	Note->Entries[2].Date.Timezone = 0;
	Note->Entries[2].Date.Second = 0;
	Note->EntriesNum = 3;
	error = ATGEN_ParseReply(s,
		line,
		"+MDBR: @i, @s, @i, @i, @d, @i, @d, @i",
		&ignore,
		Note->Entries[0].Text, sizeof(Note->Entries[0].Text),
		&has_time,
		&has_alarm,
		&(Note->Entries[1].Date),
		&duration,
		&(Note->Entries[2].Date),
		&repeat);

	if (!has_time && !has_alarm) {
		Note->EntriesNum = 1;
	} else if (!has_alarm) {
		Note->EntriesNum = 2;
	} else if (!has_time) {
		Note->EntriesNum = 2;
		Note->Entries[1].EntryType = Note->Entries[2].EntryType;
		Note->Entries[1].Date = Note->Entries[2].Date;
	}
	switch (repeat) {
		case 1:
			Note->Entries[Note->EntriesNum].EntryType = CAL_REPEAT_FREQUENCY;
			Note->Entries[Note->EntriesNum].Number = 1;
			Note->EntriesNum++;
			break;
		case 2:
			Note->Entries[Note->EntriesNum].EntryType = CAL_REPEAT_FREQUENCY;
			Note->Entries[Note->EntriesNum].Number = 7;
			Note->EntriesNum++;
			break;
		case 3:
			Note->Entries[Note->EntriesNum].EntryType = CAL_REPEAT_FREQUENCY;
			Note->Entries[Note->EntriesNum].Number = 1;
			Note->EntriesNum++;
			Note->Entries[Note->EntriesNum].EntryType = CAL_REPEAT_DAY;
			Note->Entries[Note->EntriesNum].Number = Note->Entries[1].Date.Day;
			Note->EntriesNum++;
			break;
		case 4:
			Note->Entries[Note->EntriesNum].EntryType = CAL_REPEAT_FREQUENCY;
			Note->Entries[Note->EntriesNum].Number = 1;
			Note->EntriesNum++;
			Note->Entries[Note->EntriesNum].EntryType = CAL_REPEAT_DAY;
			Note->Entries[Note->EntriesNum].Number = Note->Entries[1].Date.Day;
			Note->EntriesNum++;
			break;
		case 5:
			Note->Entries[Note->EntriesNum].EntryType = CAL_REPEAT_FREQUENCY;
			Note->Entries[Note->EntriesNum].Number = 365;
			Note->EntriesNum++;
			break;
	}
	return error;
}

GSM_Error MOTOROLA_ReplyGetCalendar(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error error;
	const char *line;

	if (Priv->ReplyState != AT_Reply_OK) {
		switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
		case AT_Reply_Error:
			return ERR_NOTSUPPORTED;
		case AT_Reply_CMSError:
			return ATGEN_HandleCMSError(s);
		case AT_Reply_CMEError:
			return ATGEN_HandleCMEError(s);
		default:
			return ERR_UNKNOWNRESPONSE;
		}
	}

	line = GetLineString(msg.Buffer, &Priv->Lines, 2);

	if (strcmp("OK", line) == 0) {
		return ERR_EMPTY;
	}

	error = MOTOROLA_ParseCalendarSimple(s, line);
	if (error != ERR_NONE) {
		/* Fallback to parse complex later */
	}
	return error;
}

GSM_Error MOTOROLA_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, gboolean start)
{
	GSM_Error error;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	if (start) {
		/* One bellow actual first position */
		Note->Location = 0;
		error = MOTOROLA_GetCalendarStatus(s, &Priv->CalendarStatus);
		if (error != ERR_NONE) {
			return error;
		}
		Priv->CalendarRead = 0;
	}
	s->Phone.Data.Cal 	= Note;
	Note->EntriesNum 	= 0;
	smprintf(s, "Getting calendar entry\n");
	error = ERR_EMPTY;
	while (error == ERR_EMPTY) {
		Note->Location++;
		if (Note->Location >= Priv->CalendarStatus.Used + Priv->CalendarStatus.Free) {
			/* We're at the end */
			return ERR_EMPTY;
		}
		if (Priv->CalendarRead >= Priv->CalendarStatus.Used) {
			/* We've read all entries */
			return ERR_EMPTY;
		}
		error = MOTOROLA_GetCalendar(s, Note);
		if (error == ERR_NONE) {
			Priv->CalendarRead++;
		}
	}
	return error;
}

GSM_Error MOTOROLA_GetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	char req[50];
	GSM_Error error;

	error = MOTOROLA_LockCalendar(s);
	if (error != ERR_NONE) return ERR_NONE;

	s->Phone.Data.Cal = Note;

	sprintf(req, "AT+MDBR=%d\r", Note->Location - 1);

	ATGEN_WaitForAutoLen(s, req, 0x00, 10, ID_GetCalendarNote);
	MOTOROLA_UnlockCalendar(s);
	return error;
}

GSM_Error MOTOROLA_ReplySetCalendar(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error MOTOROLA_DelCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	char req[50];
	GSM_Error error;

	error = MOTOROLA_LockCalendar(s);
	if (error != ERR_NONE) return ERR_NONE;

	sprintf(req, "AT+MDBWE=%d,0,0\r", Note->Location);

	ATGEN_WaitForAutoLen(s, req, 0x00, 10, ID_DeleteCalendarNote);
	MOTOROLA_UnlockCalendar(s);
	return error;
}

GSM_Error MOTOROLA_SetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error error;

	error = MOTOROLA_LockCalendar(s);
	if (error != ERR_NONE) return ERR_NONE;

	MOTOROLA_UnlockCalendar(s);
	return ERR_NOTIMPLEMENTED;
}

GSM_Error MOTOROLA_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error error;

	error = MOTOROLA_LockCalendar(s);
	if (error != ERR_NONE) return ERR_NONE;

	MOTOROLA_UnlockCalendar(s);
	return ERR_NOTIMPLEMENTED;
}

#endif

/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
