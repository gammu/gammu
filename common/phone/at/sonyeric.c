                                           
#include "../../gsmstate.h"

#ifdef GSM_ENABLE_ATGEN

#include <string.h>
#include <time.h>
#include <ctype.h>

#include "../../gsmcomon.h"
#include "../../misc/coding/coding.h"

extern GSM_Reply_Function 		ATGENReplyFunctions[];
extern GSM_Error ATGEN_DispatchMessage	(GSM_StateMachine *s);

extern GSM_Reply_Function 		OBEXGENReplyFunctions[];
extern GSM_Error OBEXGEN_GetFilePart	(GSM_StateMachine *s, GSM_File *File);
extern GSM_Error OBEXGEN_AddFilePart	(GSM_StateMachine *s, GSM_File *File, int *Pos);
extern GSM_Error OBEXGEN_Disconnect	(GSM_StateMachine *s);

static GSM_Error SONYERIC_SetOBEXMode(GSM_StateMachine *s)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;

	if (Priv->OBEX) return GE_NONE;

	dprintf ("Changing to OBEX\n");

	error=GSM_WaitFor (s, "AT*EOBEX\r", 9, 0x00, 4, ID_SetOBEX);
	if (error != GE_NONE) return error;

	s->Protocol.Functions = &OBEXProtocol;
	error = s->Protocol.Functions->Initialise(s);
	if (error != GE_NONE) {
		s->Protocol.Functions = &ATProtocol;
		return error;
	}
	strcpy(s->CurrentConfig->Model,"seobex");
	s->Phone.Data.Priv.OBEXGEN.Service 	= 0;
	s->Phone.Functions->DispatchMessage	= GSM_DispatchMessage;
	s->Phone.Functions->ReplyFunctions	= OBEXGENReplyFunctions;
	Priv->OBEX				= true;
	return GE_NONE;
}

static GSM_Error SONYERIC_SetATMode(GSM_StateMachine *s)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;

	if (!Priv->OBEX) return GE_NONE;

	dprintf ("Changing to AT\n");

	error = OBEXGEN_Disconnect(s);
	if (error != GE_NONE) return error;

	s->Protocol.Functions = &ATProtocol;
	error = s->Protocol.Functions->Initialise(s);
	if (error != GE_NONE) {
		s->Protocol.Functions = &OBEXProtocol;
		return error;
	}
	strcpy(s->CurrentConfig->Model,"at");
	s->Phone.Functions->DispatchMessage	= ATGEN_DispatchMessage;
	s->Phone.Functions->ReplyFunctions	= ATGENReplyFunctions;
	Priv->OBEX				= false;
	return GE_NONE;
}

GSM_Error SONYERIC_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start)
{
	GSM_Error		error;
	GSM_ToDoEntry		ToDo;
	int			Pos, num, Loc;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (start) {
		error = SONYERIC_SetOBEXMode(s);
		if (error != GE_NONE) return error;

		strcpy(Priv->file.ID_FullName,"telecom/cal.vcs");
		free(Priv->file.Buffer);
		Priv->file.Used 	= 0;
		Priv->file.Buffer 	= NULL;

		error = GE_NONE;
		while (error == GE_NONE) error = OBEXGEN_GetFilePart(s,&Priv->file);
		if (error != GE_EMPTY) return error;

		error = SONYERIC_SetATMode(s);
		if (error != GE_NONE) return error;

		Loc = 1;
	} else {
		Loc = Note->Location+1;
	}

	Pos = 0;
	num = 0;
	while (1) {
		error = GSM_DecodeVCALENDAR_VTODO(Priv->file.Buffer, &Pos, Note, &ToDo, Nokia_VCalendar, Nokia_VToDo);
		if (error == GE_EMPTY) break;
		if (error != GE_NONE) return error;
		if (Note->EntriesNum != 0) {			
			num++;
			if (num == Loc) {
				Note->Location = Loc;
				return GE_NONE;
			}
		}
	}
	
	Note->Location = Loc;
	return GE_EMPTY;
}

GSM_Error SONYERIC_GetToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool refresh)
{
	GSM_Error		error;
	GSM_CalendarEntry	Calendar;
	int			Pos, num, Loc;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer!=AT_Ericsson) return GE_NOTSUPPORTED;

	if (refresh) {
		error = SONYERIC_SetOBEXMode(s);
		if (error != GE_NONE) return error;

		strcpy(Priv->file.ID_FullName,"telecom/cal.vcs");
		free(Priv->file.Buffer);
		Priv->file.Used 	= 0;
		Priv->file.Buffer 	= NULL;

		error = GE_NONE;
		while (error == GE_NONE) error = OBEXGEN_GetFilePart(s,&Priv->file);
		if (error != GE_EMPTY) return error;

		error = SONYERIC_SetATMode(s);
		if (error != GE_NONE) return error;
	}

	Loc = ToDo->Location;
	Pos = 0;
	num = 0;
	while (1) {
		error = GSM_DecodeVCALENDAR_VTODO(Priv->file.Buffer, &Pos, &Calendar, ToDo, Nokia_VCalendar, SonyEricsson_VToDo);
		if (error == GE_EMPTY) break;
		if (error != GE_NONE) return error;
		if (ToDo->EntriesNum != 0) {			
			num++;
			if (num == Loc) return GE_NONE;
		}
	}
	return GE_INVALIDLOCATION;
}

GSM_Error SONYERIC_GetToDoStatus(GSM_StateMachine *s, GSM_ToDoStatus *status)
{
	GSM_Error		error;
	GSM_ToDoEntry		ToDo;
	GSM_CalendarEntry 	Calendar;
	int			Pos;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->Manufacturer!=AT_Ericsson) return GE_NOTSUPPORTED;

	error = SONYERIC_SetOBEXMode(s);
	if (error != GE_NONE) return error;

	strcpy(Priv->file.ID_FullName,"telecom/cal.vcs");
	free(Priv->file.Buffer);
	Priv->file.Used 	= 0;
	Priv->file.Buffer 	= NULL;

	error = GE_NONE;
	while (error == GE_NONE) error = OBEXGEN_GetFilePart(s,&Priv->file);
	if (error != GE_EMPTY) return error;

	error = SONYERIC_SetATMode(s);
	if (error != GE_NONE) return error;

	status->Used 	= 0;
	Pos 		= 0;
	while (1) {
		error = GSM_DecodeVCALENDAR_VTODO(Priv->file.Buffer, &Pos, &Calendar, &ToDo, Nokia_VCalendar, Nokia_VToDo);
		if (error == GE_EMPTY) break;
		if (error != GE_NONE) return error;
		if (ToDo.EntriesNum != 0) status->Used++;
	}
	
	return GE_NONE;
}

GSM_Error SONYERIC_AddCalendarNote(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool Past)
{
	GSM_Error		error;
	unsigned char 		req[500];
	int			size=0,Pos=0;
	GSM_File		File;

//	if (Note->Location==0x00) return GE_INVALIDLOCATION;	

	if (!Past && IsCalendarNoteFromThePast(Note)) return GE_NONE;

	error=GSM_EncodeVCALENDAR(req,&size,Note,true,SonyEricsson_VCalendar);

	error = SONYERIC_SetOBEXMode(s);
	if (error != GE_NONE) return error;

	strcpy(File.ID_FullName,"telecom/cal/luid/.vcs");
	EncodeUnicode(File.Name,"telecom/cal/luid/.vcs",21);
	File.Used 	= size;
	File.Buffer 	= malloc(size);
	memcpy(File.Buffer,req,size);
	dprintf("adding file %i\n",size);

	error = GE_NONE;
	while (error == GE_NONE) error = OBEXGEN_AddFilePart(s,&File,&Pos);
	if (error != GE_EMPTY) return error;

	return SONYERIC_SetATMode(s);
}

GSM_Error SONYERIC_SetToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;
	unsigned char 		req[500];
	int			size=0,Pos=0;
	GSM_File		File;

	if (ToDo->Location!=0x00) return GE_NOTSUPPORTED;	
	if (Priv->Manufacturer!=AT_Ericsson) return GE_NOTSUPPORTED;

	error=GSM_EncodeVTODO(req,&size,ToDo,true,SonyEricsson_VToDo);

	error = SONYERIC_SetOBEXMode(s);
	if (error != GE_NONE) return error;

	strcpy(File.ID_FullName,"telecom/cal/luid/.vcs");
	EncodeUnicode(File.Name,"telecom/cal/luid/.vcs",21);
	File.Used 	= size;
	File.Buffer 	= malloc(size);
	memcpy(File.Buffer,req,size);
	dprintf("adding file %i\n",size);

	error = GE_NONE;
	while (error == GE_NONE) error = OBEXGEN_AddFilePart(s,&File,&Pos);
	if (error != GE_EMPTY) return error;

	return SONYERIC_SetATMode(s);
}

GSM_Error SONYERIC_DeleteAllToDo(GSM_StateMachine *s)
{
	GSM_Error		error;
	int			Pos,Level = 0;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	unsigned char 		Line[2000];
	GSM_File		File2;

	if (Priv->Manufacturer!=AT_Ericsson) return GE_NOTSUPPORTED;

	error = SONYERIC_SetOBEXMode(s);
	if (error != GE_NONE) return error;

	strcpy(Priv->file.ID_FullName,"telecom/cal.vcs");
	free(Priv->file.Buffer);
	Priv->file.Used 	= 0;
	Priv->file.Buffer 	= NULL;

	error = GE_NONE;
	while (error == GE_NONE) error = OBEXGEN_GetFilePart(s,&Priv->file);
	if (error != GE_EMPTY) return error;

	File2.Used 	= 0;
	File2.Buffer 	= NULL;

	Pos = 0;
	while (1) {
		MyGetLine(Priv->file.Buffer, &Pos, Line, Priv->file.Used);
		if (strlen(Line) == 0) break;
		dprintf("Line is %s,%i,%i\n",Line,Priv->file.Used,Pos);
		switch (Level) {
		case 0:
			if (strstr(Line,"BEGIN:VTODO")) {
				Level = 2;
				break;
			}
			File2.Buffer=(unsigned char *)realloc(File2.Buffer,File2.Used+strlen(Line)+2);
			strcpy(File2.Buffer+File2.Used,Line);
			File2.Used=File2.Used+strlen(Line)+2;
			File2.Buffer[File2.Used] = 0x00;
			File2.Buffer[File2.Used-2] = 13;
			File2.Buffer[File2.Used-1] = 10;
			break;
		case 2: /* ToDo note */
			if (strstr(Line,"END:VTODO")) {
				Level = 0;
			}
			break;
		}
	}

//	DumpMessage(stdout, File2.Buffer, File2.Used);

	strcpy(File2.ID_FullName,"telecom/cal.vcs");
	EncodeUnicode(File2.Name,"telecom/cal.vcs",15);
	Pos   = 0;
	error = GE_NONE;
	while (error == GE_NONE) error = OBEXGEN_AddFilePart(s,&File2,&Pos);
	if (error != GE_EMPTY) return error;

	error = SONYERIC_SetATMode(s);
	if (error != GE_NONE) return error;

	return GE_NONE;
}

GSM_Error SONYERIC_DelCalendarNote(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error		error;
	int			Pos,Level = 0,Loc=0;
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	unsigned char 		Line[2000];
	GSM_File		File2;

	error = SONYERIC_SetOBEXMode(s);
	if (error != GE_NONE) return error;

	strcpy(Priv->file.ID_FullName,"telecom/cal.vcs");
	free(Priv->file.Buffer);
	Priv->file.Used 	= 0;
	Priv->file.Buffer 	= NULL;

	error = GE_NONE;
	while (error == GE_NONE) error = OBEXGEN_GetFilePart(s,&Priv->file);
	if (error != GE_EMPTY) return error;

	File2.Used 	= 0;
	File2.Buffer 	= NULL;

	Pos = 0;
	while (1) {
		MyGetLine(Priv->file.Buffer, &Pos, Line, Priv->file.Used);
		if (strlen(Line) == 0) break;
		dprintf("Line is %s,%i,%i\n",Line,Priv->file.Used,Pos);
		switch (Level) {
		case 0:
			if (strstr(Line,"BEGIN:VEVENT")) {
				Loc++;
				if (Loc == Note->Location) {
					Level = 1;
					break;
				}
			}
			File2.Buffer=(unsigned char *)realloc(File2.Buffer,File2.Used+strlen(Line)+2);
			strcpy(File2.Buffer+File2.Used,Line);
			File2.Used=File2.Used+strlen(Line)+2;
			File2.Buffer[File2.Used] = 0x00;
			File2.Buffer[File2.Used-2] = 13;
			File2.Buffer[File2.Used-1] = 10;
			break;
		case 1: /* Calendar note */
			if (strstr(Line,"END:VEVENT")) {
				Level = 0;
			}
			break;
		}
	}

//	DumpMessage(stdout, File2.Buffer, File2.Used);

	strcpy(File2.ID_FullName,"telecom/cal.vcs");
	EncodeUnicode(File2.Name,"telecom/cal.vcs",15);
	Pos   = 0;
	error = GE_NONE;
	while (error == GE_NONE) error = OBEXGEN_AddFilePart(s,&File2,&Pos);
	if (error != GE_EMPTY) return error;

	error = SONYERIC_SetATMode(s);
	if (error != GE_NONE) return error;

	return GE_NONE;
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
