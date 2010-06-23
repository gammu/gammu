                                           
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

		error = SONYERIC_SetATMode(s);
		if (error != GE_NONE) return error;
	}

	Loc = ToDo->Location;
	Pos = 0;
	num = 0;
	while (1) {
		error = GSM_DecodeVCALENDAR_VTODO(Priv->file.Buffer, &Pos, &Calendar, ToDo, Nokia_VCalendar, Nokia_VToDo);
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

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
