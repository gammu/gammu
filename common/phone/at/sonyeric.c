/* (c) 2003 by Marcin Wiacek */
                                           
#include "../../gsmstate.h"

#ifdef GSM_ENABLE_ATGEN

#include <string.h>
#include <time.h>
#include <ctype.h>

#include "../../gsmcomon.h"
#include "../../misc/coding/coding.h"

#include "atgen.h"
#include "sonyeric.h"

#if defined(GSM_ENABLE_BLUEOBEX) || defined(GSM_ENABLE_IRDAOBEX)

#include "../obex/obexgen.h"

extern GSM_Protocol_Functions 	OBEXProtocol;
extern GSM_Reply_Function	OBEXGENReplyFunctions[];
extern GSM_Reply_Function	ATGENReplyFunctions[];

static GSM_Error SONYERIC_SetOBEXMode(GSM_StateMachine *s)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;

	if (Priv->OBEX) return ERR_NONE;

	dbgprintf ("Changing to OBEX\n");

	error=GSM_WaitFor (s, "AT*EOBEX\r", 9, 0x00, 4, ID_SetOBEX);
	if (error != ERR_NONE) return error;

	error = s->Protocol.Functions->Terminate(s);
	if (error != ERR_NONE) return error;

	s->Protocol.Functions = &OBEXProtocol;
	error = s->Protocol.Functions->Initialise(s);
	if (error != ERR_NONE) {
		s->Protocol.Functions = &ATProtocol;
		return error;
	}
	strcpy(s->CurrentConfig->Model,"seobex");
	s->Phone.Data.Priv.OBEXGEN.Service 	= 0;
	s->Phone.Functions->DispatchMessage	= GSM_DispatchMessage;
	s->Phone.Functions->ReplyFunctions	= OBEXGENReplyFunctions;
	Priv->OBEX				= true;
	return ERR_NONE;
}

static GSM_Error SONYERIC_SetATMode(GSM_StateMachine *s)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;

	if (!Priv->OBEX) return ERR_NONE;

	dbgprintf ("Changing to AT\n");

	error = OBEXGEN_Disconnect(s);
	if (error != ERR_NONE) return error;

	error = s->Protocol.Functions->Terminate(s);
	if (error != ERR_NONE) return error;

	s->Protocol.Functions = &ATProtocol;
	error = s->Protocol.Functions->Initialise(s);
	if (error != ERR_NONE) {
		s->Protocol.Functions = &OBEXProtocol;
		return error;
	}
	strcpy(s->CurrentConfig->Model,"at");
	s->Phone.Functions->DispatchMessage	= ATGEN_DispatchMessage;
	s->Phone.Functions->ReplyFunctions	= ATGENReplyFunctions;
	Priv->OBEX				= false;
	return ERR_NONE;
}

static GSM_Error SONYERIC_GetFile(GSM_StateMachine *s, GSM_File *File, unsigned char *FileName)
{
	GSM_Error error;

	strcpy(File->ID_FullName,FileName);
	File->Used 	= 0;
	if (File->Buffer != NULL) free(File->Buffer);
	File->Buffer 	= NULL;

	error = SONYERIC_SetOBEXMode(s);
	if (error != ERR_NONE) return error;

	error = ERR_NONE;
	while (error == ERR_NONE) error = OBEXGEN_GetFilePart(s,File);
	if (error != ERR_EMPTY) return error;

	return SONYERIC_SetATMode(s);
}

static GSM_Error SONYERIC_SetFile(GSM_StateMachine *s, unsigned char *FileName, unsigned char *Buffer, int Length)
{
	GSM_Error	error;
	GSM_File 	File;
	int		Pos = 0;

	error = SONYERIC_SetOBEXMode(s);
	if (error != ERR_NONE) return error;

	strcpy(File.ID_FullName,FileName);
	EncodeUnicode(File.Name,FileName,strlen(FileName));
	File.Used 	= Length;
	File.Buffer 	= malloc(Length);
	memcpy(File.Buffer,Buffer,Length);

	error = ERR_NONE;
	while (error == ERR_NONE) error = OBEXGEN_AddFilePart(s,&File,&Pos);
	free(File.Buffer);
	if (error != ERR_EMPTY) return error;

	return SONYERIC_SetATMode(s);
}

#endif

GSM_Error SONYERIC_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start)
{
#if defined(GSM_ENABLE_BLUEOBEX) || defined(GSM_ENABLE_IRDAOBEX)
	GSM_Error		error;
	GSM_ToDoEntry		ToDo;
	int			Pos, num, Loc;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (start) {
		error = SONYERIC_GetFile(s, &Priv->file, "telecom/cal.vcs");
		if (error != ERR_NONE) return error;

		Note->Location = 1;
	} else {
		Note->Location++;
	}
	smprintf(s, "Getting calendar note %i\n",Note->Location);

	Loc = Note->Location;
	Pos = 0;
	num = 0;
	while (1) {
		error = GSM_DecodeVCALENDAR_VTODO(Priv->file.Buffer, &Pos, Note, &ToDo, SonyEricsson_VCalendar, SonyEricsson_VToDo);
		if (error == ERR_EMPTY) break;
		if (error != ERR_NONE) return error;
		if (Note->EntriesNum != 0) {			
			num++;
			if (num == Loc) return ERR_NONE;
		}
	}
	return ERR_EMPTY;
#else
	return ERR_SOURCENOTAVAILABLE;
#endif
}

GSM_Error SONYERIC_GetNextToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool start)
{
#if defined(GSM_ENABLE_BLUEOBEX) || defined(GSM_ENABLE_IRDAOBEX)
	GSM_Error		error;
	GSM_CalendarEntry	Calendar;
	int			Pos, num, Loc;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer!=AT_Ericsson) return ERR_NOTSUPPORTED;

	if (start) {
		error = SONYERIC_GetFile(s, &Priv->file, "telecom/cal.vcs");
		if (error != ERR_NONE) return error;

		ToDo->Location = 1;
	} else {
		ToDo->Location++;
	}
	smprintf(s,"Getting ToDo %i\n",ToDo->Location);

	Loc = ToDo->Location;	
	Pos = 0;
	num = 0;
	while (1) {
		error = GSM_DecodeVCALENDAR_VTODO(Priv->file.Buffer, &Pos, &Calendar, ToDo, SonyEricsson_VCalendar, SonyEricsson_VToDo);
		if (error == ERR_EMPTY) break;
		if (error != ERR_NONE) return error;
		if (ToDo->EntriesNum != 0) {			
			num++;
			if (num == Loc) return ERR_NONE;
		}
	}

	return ERR_EMPTY;
#else
	return ERR_SOURCENOTAVAILABLE;
#endif
}

GSM_Error SONYERIC_GetToDoStatus(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
#if defined(GSM_ENABLE_BLUEOBEX) || defined(GSM_ENABLE_IRDAOBEX)
	GSM_Error		error;
	GSM_ToDoEntry		ToDo;
	GSM_CalendarEntry 	Calendar;
	int			Pos;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer!=AT_Ericsson) return ERR_NOTSUPPORTED;

	smprintf(s,"Getting ToDo status\n");

	error = SONYERIC_GetFile(s, &Priv->file, "telecom/cal.vcs");
	if (error != ERR_NONE) return error;

	status->Used 	= 0;
	Pos 		= 0;
	while (1) {
		error = GSM_DecodeVCALENDAR_VTODO(Priv->file.Buffer, &Pos, &Calendar, &ToDo, SonyEricsson_VCalendar, SonyEricsson_VToDo);
		if (error == ERR_EMPTY) break;
		if (error != ERR_NONE) return error;
		if (ToDo.EntriesNum != 0) status->Used++;
	}
	
	return ERR_NONE;
#else
	return ERR_SOURCENOTAVAILABLE;
#endif
}

GSM_Error SONYERIC_AddCalendarNote(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
#if defined(GSM_ENABLE_BLUEOBEX) || defined(GSM_ENABLE_IRDAOBEX)
	unsigned char 		req[5000];
	int			size=0;

	smprintf(s,"Adding calendar note\n");

	GSM_EncodeVCALENDAR(req,&size,Note,true,SonyEricsson_VCalendar);

	return SONYERIC_SetFile(s, "telecom/cal/luid/.vcs", req, size);
#else
	return ERR_SOURCENOTAVAILABLE;
#endif
}

GSM_Error SONYERIC_AddToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
#if defined(GSM_ENABLE_BLUEOBEX) || defined(GSM_ENABLE_IRDAOBEX)
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	unsigned char 		req[5000];
	int			size=0;

	if (Priv->Manufacturer!=AT_Ericsson) return ERR_NOTSUPPORTED;

	smprintf(s,"Adding ToDo\n");

	GSM_EncodeVTODO(req,&size,ToDo,true,SonyEricsson_VToDo);

	return SONYERIC_SetFile(s, "telecom/cal/luid/.vcs", req, size);
#else
	return ERR_SOURCENOTAVAILABLE;
#endif
}

GSM_Error SONYERIC_DeleteAllToDo(GSM_StateMachine *s)
{
#if defined(GSM_ENABLE_BLUEOBEX) || defined(GSM_ENABLE_IRDAOBEX)
	GSM_Error		error;
	int			Pos,Level = 0,Used;
	unsigned char		*Buf;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	unsigned char 		Line[2000];

	if (Priv->Manufacturer!=AT_Ericsson) return ERR_NOTSUPPORTED;

	smprintf(s,"Deleting all ToDo\n");

	error = SONYERIC_GetFile(s, &Priv->file, "telecom/cal.vcs");
	if (error != ERR_NONE) return error;

	Pos  = 0;
	Buf  = NULL;
	Used = 0;
	while (1) {
		MyGetLine(Priv->file.Buffer, &Pos, Line, Priv->file.Used);
		if (strlen(Line) == 0) break;
		dbgprintf("Line is %s,%i,%i\n",Line,Priv->file.Used,Pos);
		switch (Level) {
		case 0:
			if (strstr(Line,"BEGIN:VTODO")) {
				Level = 2;
				break;
			}
			Buf=(unsigned char *)realloc(Buf,Used+strlen(Line)+3);
			strcpy(Buf+Used,Line);
			Used=Used+strlen(Line)+3;
			Buf[Used-3] = 13;
			Buf[Used-2] = 10;
			Buf[Used-1] = 0x00;
			break;
		case 2: /* ToDo note */
			if (strstr(Line,"END:VTODO")) {
				Level = 0;
			}
			break;
		}
	}

	error = SONYERIC_SetFile(s, "telecom/cal.vcs", Buf, Used);
//	if (Buf != NULL) free(Buf);
	return error;
#else
	return ERR_SOURCENOTAVAILABLE;
#endif
}

GSM_Error SONYERIC_DelCalendarNote(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
#if defined(GSM_ENABLE_BLUEOBEX) || defined(GSM_ENABLE_IRDAOBEX)
	GSM_Error		error;
	int			Pos,Level = 0,Loc=0,Used;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	unsigned char 		Line[2000];
	unsigned char		*Buf;

	smprintf(s, "Deleting calendar note %i\n",Note->Location);

	error = SONYERIC_GetFile(s, &Priv->file, "telecom/cal.vcs");
	if (error != ERR_NONE) return error;

	Pos  = 0;
	Buf  = NULL;
	Used = 0;
	while (1) {
		MyGetLine(Priv->file.Buffer, &Pos, Line, Priv->file.Used);
		if (strlen(Line) == 0) break;
		dbgprintf("Line is %s,%i,%i\n",Line,Priv->file.Used,Pos);
		switch (Level) {
		case 0:
			if (strstr(Line,"BEGIN:VEVENT")) {
				Loc++;
				if (Loc == Note->Location) {
					Level = 1;
					break;
				}
			}
			Buf=(unsigned char *)realloc(Buf,Used+strlen(Line)+3);
			strcpy(Buf+Used,Line);
			Used=Used+strlen(Line)+3;
			Buf[Used-3] = 13;
			Buf[Used-2] = 10;
			Buf[Used-1] = 0x00;
			break;
		case 1: /* Calendar note */
			if (strstr(Line,"END:VEVENT")) {
				Level = 0;
			}
			break;
		}
	}

	DumpMessage(s->di.df, s->di.dl, Buf, Used);

	error = SONYERIC_SetFile(s, "telecom/cal.vcs", Buf, Used);
	if (Buf != NULL) free(Buf);
	return error;
#else
	return ERR_SOURCENOTAVAILABLE;
#endif
}

GSM_Error SONYERIC_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *Status)
{
#if defined(GSM_ENABLE_BLUEOBEX) || defined(GSM_ENABLE_IRDAOBEX)
	GSM_Error		error;
	GSM_ToDoEntry		ToDo;
	GSM_CalendarEntry 	Calendar;
	int			Pos;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer!=AT_Ericsson) return ERR_NOTSUPPORTED;

	smprintf(s, "Getting calendar status\n");

	error = SONYERIC_GetFile(s, &Priv->file, "telecom/cal.vcs");
	if (error != ERR_NONE) return error;

	Status->Used 	= 0;
	Pos  		= 0;
	while (1) {
		error = GSM_DecodeVCALENDAR_VTODO(Priv->file.Buffer, &Pos, &Calendar, &ToDo, SonyEricsson_VCalendar, SonyEricsson_VToDo);
		if (error == ERR_EMPTY) break;
		if (error != ERR_NONE) return error;
		if (Calendar.EntriesNum != 0) Status->Used++;
	}
	
	return ERR_NONE;
#else
	return ERR_SOURCENOTAVAILABLE;
#endif
}

GSM_Error ERICSSON_ReplyGetDateLocale(GSM_Protocol_Message msg, GSM_StateMachine *s)
{ /*	Author: Peter Ondraska, based on code by Marcin Wiacek and Michal Cihar
	License: Whatever the current maintainer of gammulib chooses, as long as there
	is an easy way to obtain the source under GPL, otherwise the author's parts
	of this function are GPL 2.0.
  */
	GSM_Locale	*locale = s->Phone.Data.Locale;
	char		format;

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Date settings received\n");
		format=atoi(msg.Buffer);
		switch (format) {
			case 0: locale->DateFormat 	= GSM_Date_OFF;
				locale->DateSeparator 	= 0;
				break;
			case 1: locale->DateFormat 	= GSM_Date_DDMMMYY;
				locale->DateSeparator 	= '-';
				break;
			case 2: locale->DateFormat 	= GSM_Date_DDMMYY;
				locale->DateSeparator 	= '-';
				break;
			case 3: locale->DateFormat 	= GSM_Date_MMDDYY;
				locale->DateSeparator 	= '/';
				break;
			case 4: locale->DateFormat 	= GSM_Date_DDMMYY;
				locale->DateSeparator 	= '/';
				break;
			case 5: locale->DateFormat 	= GSM_Date_DDMMYY;
				locale->DateSeparator 	= '.';
				break;
			case 6: locale->DateFormat 	= GSM_Date_YYMMDD;
				locale->DateSeparator 	= 0;
				break;
			case 7: locale->DateFormat 	= GSM_Date_YYMMDD;
				locale->DateSeparator 	= '-';
				break;
			default:return ERR_UNKNOWNRESPONSE;
		}
	default: 
		return ERR_NOTSUPPORTED;
	}
}

GSM_Error ERICSSON_ReplyGetTimeLocale(GSM_Protocol_Message msg, GSM_StateMachine *s)
{ /*	Author: Peter Ondraska
	License: Whatever the current maintainer of gammulib chooses, as long as there
	is an easy way to obtain the source under GPL, otherwise the author's parts
	of this function are GPL 2.0.
  */
	char format;

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Time settings received\n");
		format=atoi(msg.Buffer);
		switch (format) {
		case 1:
		case 2: s->Phone.Data.Locale->AMPMTime=(format==2);
			return ERR_NONE;
		default:return ERR_UNKNOWNRESPONSE;
		}
	default: return ERR_NOTSUPPORTED;
	}
}

GSM_Error ERICSSON_GetLocale(GSM_StateMachine *s, GSM_Locale *locale)
{
        GSM_Error error;

	s->Phone.Data.Locale = locale;

	smprintf(s, "Getting date format\n");
	error=GSM_WaitFor (s, "AT+ESDF?\r", 9, 0x00, 3, ID_GetLocale);
	if (error!=ERR_NONE) return error;

	smprintf(s, "Getting time format\n");
	return GSM_WaitFor (s, "AT+ESTF?\r", 9, 0x00, 3, ID_GetLocale);
}


GSM_Error ERICSSON_SetLocale(GSM_StateMachine *s, GSM_Locale *locale)
{ /*	Author: Peter Ondraska
	License: Whatever the current maintainer of gammulib chooses, as long as there
	is an easy way to obtain the source under GPL, otherwise the author's parts
	of this function are GPL 2.0.
  */
	/* this is not yet supported by gammu.c */
	int	format=0;
	char	req[12];

	if (locale->DateFormat==GSM_Date_OFF) { format=0; } else
	if ((locale->DateFormat==GSM_Date_DDMMMYY)&&(locale->DateSeparator=='-')) { format=1; } else
	if ((locale->DateFormat==GSM_Date_DDMMYY)&&(locale->DateSeparator=='-')) { format=2; } else
	if ((locale->DateFormat==GSM_Date_MMDDYY)&&(locale->DateSeparator=='/')) { format=3; } else
	if ((locale->DateFormat==GSM_Date_DDMMYY)&&(locale->DateSeparator=='/')) { format=4; } else
	if ((locale->DateFormat==GSM_Date_DDMMYY)&&(locale->DateSeparator=='.')) { format=5; } else
	if ((locale->DateFormat==GSM_Date_YYMMDD)&&(locale->DateSeparator==0)) { format=6; } else
	if ((locale->DateFormat==GSM_Date_YYMMDD)&&(locale->DateSeparator=='-')) { format=7; }
	else { return ERR_NOTSUPPORTED; } /* ERR_WRONGINPUT */

	sprintf(req,"AT+ESDF=%i\r",format);
	smprintf(s, "Setting date format\n");
	return GSM_WaitFor (s, req, strlen(req), 0x00, 3, ID_SetLocale);

	if (locale->AMPMTime) { format=2; } else { format=1; }
	sprintf(req,"AT+ESTF=%i\r",format);
	smprintf(s, "Setting time format\n");
	return GSM_WaitFor (s, req, strlen(req), 0x00, 3, ID_SetLocale);
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
