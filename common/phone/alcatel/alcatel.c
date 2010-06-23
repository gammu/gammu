/*
 * High level functions for communication with Alcatel One Touch 501 and
 * compatible mobile phone.
 *
 * This code implements functions to communicate with Alcatel BE5 (One Touch
 * 500, 501, 701 and maybe others) phone. For some functions it uses normal AT
 * mode (not implemented here, look at ../at/atgen.[ch]) for others it
 * switches into binary mode and initialises underlaying protocol (see
 * ../../protocol/alcatel/alcabus.[ch]) and communicates over it. Don't ask
 * me why Alcatel uses such silly thing...
 *
 * Notes for future features:
 * - max phone number length is 61
 * - max name length is 50
 */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_ALCATEL

#include <string.h>
#include <time.h>

#include "../../gsmcomon.h"
#include "../../misc/coding.h"
#include "../../service/gsmsms.h"
#include "../pfunc.h"
#include "alcatel.h"

extern GSM_Reply_Function ALCATELReplyFunctions[];
extern GSM_Reply_Function ATGENReplyFunctions[];

extern GSM_Error ATGEN_Initialise		(GSM_StateMachine *s);
extern GSM_Error ATGEN_GetIMEI 			(GSM_StateMachine *s);
extern GSM_Error ATGEN_GetFirmware		(GSM_StateMachine *s);
extern GSM_Error ATGEN_GetModel			(GSM_StateMachine *s);
extern GSM_Error ATGEN_GetDateTime		(GSM_StateMachine *s, GSM_DateTime *date_time);
extern GSM_Error ATGEN_GetAlarm			(GSM_StateMachine *s, GSM_DateTime *alarm, int alarm_number);
extern GSM_Error ATGEN_GetMemory		(GSM_StateMachine *s, GSM_PhonebookEntry *entry);
extern GSM_Error ATGEN_SetMemory		(GSM_StateMachine *s, GSM_PhonebookEntry *entry);
extern GSM_Error ATGEN_GetMemoryStatus		(GSM_StateMachine *s, GSM_MemoryStatus *Status);
extern GSM_Error ATGEN_GetSMSC			(GSM_StateMachine *s, GSM_SMSC *smsc);
extern GSM_Error ATGEN_GetSMSMessage		(GSM_StateMachine *s, GSM_MultiSMSMessage *sms);
extern GSM_Error ATGEN_GetBatteryCharge		(GSM_StateMachine *s, GSM_BatteryCharge *bat);
extern GSM_Error ATGEN_GetSignalQuality		(GSM_StateMachine *s, GSM_SignalQuality *sig);
extern GSM_Error ATGEN_GetSMSFolders		(GSM_StateMachine *s, GSM_SMSFolders *folders);
extern GSM_Error ATGEN_GetNextSMSMessage	(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start);
extern GSM_Error ATGEN_GetSMSStatus		(GSM_StateMachine *s, GSM_SMSMemoryStatus *status);
extern GSM_Error ATGEN_DialVoice		(GSM_StateMachine *s, char *number);
extern GSM_Error ATGEN_AnswerCall		(GSM_StateMachine *s);
extern GSM_Error ATGEN_CancelCall		(GSM_StateMachine *s);
extern GSM_Error ATGEN_SaveSMSMessage		(GSM_StateMachine *s, GSM_SMSMessage *sms);
extern GSM_Error ATGEN_SendSMSMessage		(GSM_StateMachine *s, GSM_SMSMessage *sms);
extern GSM_Error ATGEN_SetDateTime		(GSM_StateMachine *s, GSM_DateTime *date_time);
extern GSM_Error ATGEN_DeleteSMSMessage		(GSM_StateMachine *s, GSM_SMSMessage *sms);
extern GSM_Error ATGEN_SetSMSC			(GSM_StateMachine *s, GSM_SMSC *smsc);
extern GSM_Error ATGEN_EnterSecurityCode	(GSM_StateMachine *s, GSM_SecurityCode Code);
extern GSM_Error ATGEN_GetSecurityStatus	(GSM_StateMachine *s, GSM_SecurityCodeType *Status);
extern GSM_Error ATGEN_ResetPhoneSettings	(GSM_StateMachine *s, GSM_ResetSettingsType Type);
extern GSM_Error ATGEN_SendDTMF			(GSM_StateMachine *s, char *sequence);
extern GSM_Error ATGEN_GetSIMIMSI		(GSM_StateMachine *s, char *IMSI);
extern GSM_Error ATGEN_HandleCMSError		(GSM_StateMachine *s);

extern GSM_Error ATGEN_DispatchMessage		(GSM_StateMachine *s);

/* Alcatel uses some 8-bit characters in contacts, calendar etc.. This table
 * attempts to decode it, it is probably not complete, here are just chars
 * that I found...
 */
unsigned char GSM_AlcatelAlphabet[] =
{
/*	in phone	unicode		description	*/
	0x89,		0x00,0xc1,	/* A acute	*/
	0xcf,		0x00,0x5c,	/* \		*/
	0xce,		0x00,0x7e,	/* ~		*/
	0xfc,		0x20,0xac,	/* Euro		*/
	0x84,		0x00,0xe7,	/* c cedilla	*/
	0x98,		0x00,0xe1,	/* a acute	*/
	0x00,		0x00,0x00
};

/* This is being called from atgen */
GSM_Error ALCATEL_ProtocolVersionReply	(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	char			*str, *str2;
/*
 * Reply received here looks like:
 * 1 "AT+CPROT=?"
 * 2 "+CPROT: 0,"V1.0",1"
 * 3 "+CPROT: 16,"V1.1",16"
 * 4 "OK"
 */
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
		case AT_Reply_OK:
			str = strstr(msg.Buffer, "\"V");
			if (str == NULL) return GE_UNKNOWNRESPONSE;
			str += 2;
			while((str2 = strstr(str, "\"V")) != NULL) str = str2 + 2;
			if (strncmp(str, "1.0", 3) == 0) {
				s->Phone.Data.Priv.ALCATEL.ProtocolVersion = V_1_0;
			} else if (strncmp(str, "1.1", 3) == 0) {
				s->Phone.Data.Priv.ALCATEL.ProtocolVersion = V_1_1;
			} else {
				smprintf(s, "Unknown protocol version. Please send debug log and phone info to author.\n");
				return GE_NOTIMPLEMENTED;
			}
			return GE_NONE;
		case AT_Reply_Error:
		case AT_Reply_CMSError:
			return ATGEN_HandleCMSError(s);
		default:
			return GE_UNKNOWNRESPONSE;
	}
}

static GSM_Error ALCATEL_SetBinaryMode(GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;

	if (Priv->Mode == ModeBinary) return GE_NONE;

	if (s->Speed != 19200) {
		smprintf(s, "Error: Alcatel binary mode works only at 19200 bps\n");
		return GE_NOTSUPPORTED;
	}

	dprintf ("Changing to binary mode\n");

	error=GSM_WaitFor (s, "AT+IFC=2,2\r", 11, 0x02, 4, ID_SetFlowControl);
	if (error != GE_NONE) return error;
	
	error=GSM_WaitFor (s, "AT+CPROT=?\r", 11, 0x02, 4, ID_AlcatelProtocol);
	if (error != GE_NONE) return error;

	if (Priv->ProtocolVersion == V_1_0) {
		error=GSM_WaitFor (s, "AT+CPROT=16,\"V1.0\",16\r", 22, 0x00, 4, ID_AlcatelConnect);
	} else {
		error=GSM_WaitFor (s, "AT+CPROT=16,\"V1.1\",16\r", 22, 0x00, 4, ID_AlcatelConnect);
	}
	if (error != GE_NONE) return error;

	dprintf ("Changing protocol to Alcabus\n");

	s->Protocol.Functions = &ALCABUSProtocol;
	error = s->Protocol.Functions->Initialise(s);
	if (error != GE_NONE) {
		s->Protocol.Functions = &ATProtocol;
		return error;
	}
	s->Phone.Functions->ReplyFunctions	= ALCATELReplyFunctions;
	Priv->Mode				= ModeBinary;
	Priv->BinaryItem			= 0;
	Priv->BinaryType			= 0;
	Priv->BinaryState			= StateAttached;
	return GE_NONE;
}

static GSM_Error ALCATEL_GoToBinaryState(GSM_StateMachine *s, GSM_Alcatel_BinaryState state, GSM_Alcatel_BinaryType type, int item) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;
	unsigned char		attach_buffer[] = {0x00, 0x00, 0x7C ,0x20};
	unsigned char		detach_buffer[] = {0x00, 0x01, 0x7C ,0x00};
	unsigned char		start_buffer[] =
					{0x00, 0x04, 0x7C, 0x80, /* 4 byte database id follows */
					 0x12, 0x34, 0x56, 0x78};
	unsigned char		end_buffer[] =
					{0x00, 0x04, 0x7C, 0x82,
					 0x00,			/* type */
					 0x00, 0x00, 0x00, 0x00}; /* TimeStamp */
	unsigned char		close_buffer[] =
					{0x00, 0x04,
					 0x00, 			 /*type */
					 0x23, 0x01};
	unsigned char		select1_buffer[] =
					{0x00, 0x00,
					 0x00, 			 /*type */
					 0x20};
	unsigned char		select2_buffer[] =
					{0x00, 0x04,
					 0x00, 			 /*type */
					 0x22, 0x01, 0x00};
	unsigned char		begin_buffer[] =
					{0x00, 0x04, 0x7C, 0x81,
					 0x00,			 /*type */
					 0x00, 0x85, 0x00};
	unsigned char		commit_buffer[] =
					{0x00, 0x04,
					 0x00, 			/*type */
					 0x20, 0x01};

	error = ALCATEL_SetBinaryMode(s);
	if (error != GE_NONE) return error;

	/* Do we need to do anything? */
	if ((state == Priv->BinaryState) && (type == Priv->BinaryType) && (item == Priv->BinaryItem)) return GE_NONE;

	/* We're editing, but the next state is not the same. so commit editing */
	if (Priv->BinaryState == StateEdit) {
		switch (Priv->BinaryType) {
			case TypeCalendar:
				commit_buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
				break;
			case TypeContacts:
				commit_buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
				break;
			case TypeToDo:
				commit_buffer[2] = ALCATEL_SYNC_TYPE_TODO;
				break;
		}
		dprintf ("Commiting edited record\n");
		error=GSM_WaitFor (s, commit_buffer, 5, 0x02, ALCATEL_TIMEOUT, ID_AlcatelCommit);
		if (error != GE_NONE) return error;
		error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelCommit2);
		if (error != GE_NONE) return error;
		Priv->BinaryState = StateSession;
		Priv->BinaryItem = 0;
	}

	/* Do we want to edit something of same type? */
	if ((state == StateEdit) && (type == Priv->BinaryType)) {
		/* Edit state doesn't need any switching, it is needed only for
		 * indication that e have to commit record before we switch to other
		 * mode.
		 */
		Priv->BinaryState = StateEdit;
		Priv->BinaryItem = item;
		return GE_NONE;
	}

	/* Now we can be only in Attached or Session state, so if states and types matches, just keep them as they are */
	if ((state == Priv->BinaryState) && (type == Priv->BinaryType)) {
		return GE_NONE;
	}

	/* Do we need to close session? */
	if (Priv->BinaryState == StateSession) {
		dprintf ("Ending session\n");
		switch (Priv->BinaryType) {
			case TypeCalendar:
				end_buffer[4] 	= ALCATEL_BEGIN_SYNC_CALENDAR;
				break;
			case TypeContacts:
				end_buffer[4] 	= ALCATEL_BEGIN_SYNC_CONTACTS;
				break;
			case TypeToDo:
				end_buffer[4] 	= ALCATEL_BEGIN_SYNC_TODO;
				break;
		}
		error=GSM_WaitFor (s, end_buffer, 9, 0x02, ALCATEL_TIMEOUT, ID_AlcatelEnd);
		if (error != GE_NONE) return error;
		
		switch (Priv->BinaryType) {
			case TypeCalendar:
				close_buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
				break;
			case TypeContacts:
				close_buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
				break;
			case TypeToDo:
				close_buffer[2] = ALCATEL_SYNC_TYPE_TODO;
				break;
		}
		dprintf ("Closing session\n");
		error=GSM_WaitFor (s, close_buffer, 5, 0x02, ALCATEL_TIMEOUT, ID_AlcatelClose);
		if (error != GE_NONE) return error;

		dprintf ("Detaching binary mode\n");
		GSM_WaitFor (s, detach_buffer, 4, 0x02, ALCATEL_TIMEOUT, ID_AlcatelDetach);

		Priv->BinaryState = StateAttached;
		Priv->BinaryType = 0;
	}

	/* Do we need to open session? */
	if (state == StateSession || state == StateEdit) {
		dprintf ("Starting session for %s\n",
				(type == TypeCalendar ? "Calendar" :
				(type == TypeToDo ? "Todo" :
				(type == TypeContacts ? "Contacts" :
				"Unknown!"))));
		/* Fill up buffers */
		switch (type) {
			case TypeCalendar:
				select1_buffer[2] 	= ALCATEL_SYNC_TYPE_CALENDAR;
				select2_buffer[2] 	= ALCATEL_SYNC_TYPE_CALENDAR;
				begin_buffer[4] 	= ALCATEL_BEGIN_SYNC_CALENDAR;
				break;
			case TypeContacts:
				select1_buffer[2] 	= ALCATEL_SYNC_TYPE_CONTACTS;
				select2_buffer[2] 	= ALCATEL_SYNC_TYPE_CONTACTS;
				begin_buffer[4] 	= ALCATEL_BEGIN_SYNC_CONTACTS;
				break;
			case TypeToDo:
				select1_buffer[2] 	= ALCATEL_SYNC_TYPE_TODO;
				select2_buffer[2] 	= ALCATEL_SYNC_TYPE_TODO;
				begin_buffer[4] 	= ALCATEL_BEGIN_SYNC_TODO;
				break;
		}
		dprintf ("Attaching in binary mode\n");

		/* Communicate */
		error=GSM_WaitFor (s, attach_buffer, 4, 0x02, ALCATEL_TIMEOUT, ID_AlcatelAttach);
		if (error != GE_NONE) return error;

		smprintf(s,"Start session\n");
		error=GSM_WaitFor (s, start_buffer, 8, 0x02, ALCATEL_TIMEOUT, ID_AlcatelStart);
		if (error != GE_NONE) return error;

		smprintf(s,"Select type\n");
		error=GSM_WaitFor (s, select1_buffer, 4, 0x02, ALCATEL_TIMEOUT, ID_AlcatelSelect1);
		if (error != GE_NONE) return error;
		error=GSM_WaitFor (s, select2_buffer, 6, 0x02, ALCATEL_TIMEOUT, ID_AlcatelSelect2);
		if (error != GE_NONE) return error;
		error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelSelect3);
		if (error != GE_NONE) return error;

		smprintf(s,"Begin transfer\n");
		error=GSM_WaitFor (s, begin_buffer, 8, 0x02, ALCATEL_TIMEOUT, ID_AlcatelBegin1);
		if (error != GE_NONE) return error;
		error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelBegin2);
		if (error != GE_NONE) return error;

		Priv->BinaryState = StateSession;
		Priv->BinaryType = type;
		/* Do we want to edit something of same type? */
		if ((state == StateEdit) && (type == Priv->BinaryType)) {
			Priv->BinaryState = StateEdit;
			Priv->BinaryItem = item;
			return GE_NONE;
		}
	}
	return GE_NONE;
}

static GSM_Error ALCATEL_SetATMode(GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;

	if (Priv->Mode == ModeAT) return GE_NONE;

	error = ALCATEL_GoToBinaryState(s, StateAttached, 0, 0);
	if (error != GE_NONE) return error;

	error = s->Protocol.Functions->Terminate(s);
	if (error != GE_NONE) return error;

	dprintf ("Changing protocol to AT\n");
	s->Protocol.Functions			= &ATProtocol;
	s->Phone.Functions->ReplyFunctions	= ATGENReplyFunctions;
	Priv->Mode				= ModeAT;

	my_sleep(100);

	/* In case we don't send AT command short after closing binary mode,
	 * phone takes VERY long to react next time. The error code in
	 * intetionally ignored.
	 */
	GSM_WaitFor (s, "AT\r", 3, 0x00, 0, ID_IncomingFrame);

	return GE_NONE;
}

static GSM_Error ALCATEL_Initialise(GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;

	Priv->Mode				= ModeAT;

	Priv->CalendarItems			= NULL;
	Priv->ContactsItems			= NULL;
	Priv->ToDoItems				= NULL;
	Priv->CalendarItemsCount		= 0;
	Priv->ToDoItemsCount			= 0;
	Priv->ContactsItemsCount		= 0;
	Priv->CurrentFields[0]			= 0;
	Priv->CurrentFieldsCount		= 0;
	Priv->CurrentFieldsItem			= 0;
	Priv->CurrentFieldsType			= 0;
	Priv->ProtocolVersion			= V_1_0;

	Priv->CurrentCategoriesCount		= 0;
	Priv->CurrentCategoriesType		= 0;

	s->Protocol.Functions			= &ATProtocol;
	s->Phone.Functions->ReplyFunctions	= ATGENReplyFunctions;

	if (s->Speed != 19200) {
		smprintf(s, "Warning: Alcatel binary mode works only at 19200 bps\n");
	}

	if (ATGEN_Initialise(s) != GE_NONE) {
		smprintf(s,"AT initialisation failed, trying to stop binary mode...\n");
		s->Protocol.Functions		= &ALCABUSProtocol;
		error = s->Protocol.Functions->Terminate(s);
		s->Protocol.Functions		= &ATProtocol;

		error = ATGEN_Initialise(s);
		if (error != GE_NONE) return error;
	}

	return GE_NONE;
}

static GSM_Error ALCATEL_Terminate(GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	free(Priv->CalendarItems);
	free(Priv->ContactsItems);
	free(Priv->ToDoItems);
	return ALCATEL_SetATMode(s);
}

/* finds whether id is set in the phone */
static GSM_Error ALCATEL_IsIdAvailable(GSM_StateMachine *s, int id) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int			i;

	if (id > ALCATEL_MAX_LOCATION) return GE_INVALIDLOCATION;

	switch (Priv->BinaryType) {
		case TypeCalendar:
			Priv->CurrentList 	= &(Priv->CalendarItems);
			Priv->CurrentCount 	= &(Priv->CalendarItemsCount);
			break;
		case TypeContacts:
			Priv->CurrentList 	= &(Priv->ContactsItems);
			Priv->CurrentCount 	= &(Priv->ContactsItemsCount);
			break;
		case TypeToDo:
			Priv->CurrentList 	= &(Priv->ToDoItems);
			Priv->CurrentCount 	= &(Priv->ToDoItemsCount);
			break;
	}
	
	for (i=0; i<*Priv->CurrentCount; i++) {
		if ((*Priv->CurrentList)[i] == id) return GE_NONE;
	}
	
	return GE_EMPTY;
}

/* finds next id that is available in the phone */
static GSM_Error ALCATEL_GetNextId(GSM_StateMachine *s, int *id) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int 			i = 0;
	int			next = ALCATEL_MAX_LOCATION;

	switch (Priv->BinaryType) {
		case TypeCalendar:
			Priv->CurrentList 	= &(Priv->CalendarItems);
			Priv->CurrentCount 	= &(Priv->CalendarItemsCount);
			break;
		case TypeContacts:
			Priv->CurrentList 	= &(Priv->ContactsItems);
			Priv->CurrentCount 	= &(Priv->ContactsItemsCount);
			break;
		case TypeToDo:
			Priv->CurrentList 	= &(Priv->ToDoItems);
			Priv->CurrentCount 	= &(Priv->ToDoItemsCount);
			break;
	}

	for (i=0; i<*Priv->CurrentCount; i++) {
		if (((*Priv->CurrentList)[i] > *id) && ((*Priv->CurrentList)[i] < next )) {
			next = (*Priv->CurrentList)[i];
		}
	}
	if (next == ALCATEL_MAX_LOCATION) {
		return GE_EMPTY;
	} else {
		*id = next;
		return GE_NONE;
	}
}

static GSM_Error ALCATEL_ReplyGetIds(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int 			count,i,pos;

	count 		 = msg.Buffer[10];
	*Priv->CurrentCount += count;

	*Priv->CurrentList = (int *)realloc(*Priv->CurrentList, (*Priv->CurrentCount + 1)* sizeof(int));
	if (*Priv->CurrentList == NULL) return GE_MOREMEMORY;

	for (i = 0; i < count; i++) {
		pos = 11 + (4 * i);
		(*Priv->CurrentList)[*Priv->CurrentCount - count + i] = msg.Buffer[pos + 3] +
							(msg.Buffer[pos + 2] << 8) +
							(msg.Buffer[pos + 1] << 16) +
							(msg.Buffer[pos] << 24);
	}
	(*Priv->CurrentList)[*Priv->CurrentCount] = 0;

	/* If last byte is 0, then we transmitted all items */
	Priv->TransferCompleted = msg.Buffer[4 + msg.Buffer[4]] == 0;
	return GE_NONE;
}

static GSM_Error ALCATEL_GetAvailableIds(GSM_StateMachine *s, bool refresh)
{
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;
	int			i;
	unsigned char 		buffer[] =
					{0x00, 0x04,
					 0x00, 			/*type */
					 0x2F, 0x01};

	if (Priv->BinaryState != StateSession) return GE_UNKNOWN;

	switch (Priv->BinaryType) {
		case TypeCalendar:
			buffer[2]		= ALCATEL_SYNC_TYPE_CALENDAR;
			Priv->CurrentList	= &(Priv->CalendarItems);
			Priv->CurrentCount	= &(Priv->CalendarItemsCount);
			break;
		case TypeContacts:
			buffer[2]		= ALCATEL_SYNC_TYPE_CONTACTS;
			Priv->CurrentList	= &(Priv->ContactsItems);
			Priv->CurrentCount	= &(Priv->ContactsItemsCount);
			break;
		case TypeToDo:
			buffer[2]		= ALCATEL_SYNC_TYPE_TODO;
			Priv->CurrentList	= &(Priv->ToDoItems);
			Priv->CurrentCount	= &(Priv->ToDoItemsCount);
			break;
	}

	if (*Priv->CurrentList != NULL) {
		if (!refresh) return GE_NONE;
		free(*Priv->CurrentList);
		*Priv->CurrentList = NULL;
	}
	smprintf(s,"Reading items list\n");

	*Priv->CurrentCount = 0;
	Priv->TransferCompleted = false;

	error=GSM_WaitFor (s, buffer, 5, 0x02, ALCATEL_TIMEOUT, ID_AlcatelGetIds1);
	if (error != GE_NONE) return error;

	while (!Priv->TransferCompleted) {
		error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelGetIds2);
		if (error != GE_NONE) return error;
	}

	i = 0;
	smprintf(s,"Received %d ids: ", *Priv->CurrentCount);
	for (i=0; i < *Priv->CurrentCount; i++) {
		smprintf(s,"%i ", (*Priv->CurrentList)[i]);
	}
	smprintf(s,"\n");

	return GE_NONE;
}

static GSM_Error ALCATEL_ReplyGetFields(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int 			i;

	if (msg.Buffer[14] > GSM_PHONEBOOK_ENTRIES) {
		smprintf(s, "WARNING: Field list truncated, you should increase GSM_PHONEBOOK_ENTRIES to at least %d\n", msg.Buffer[14]);
		Priv->CurrentFieldsCount = GSM_PHONEBOOK_ENTRIES;
	} else {
		Priv->CurrentFieldsCount = msg.Buffer[14];
	}

	Priv->CurrentFields[Priv->CurrentFieldsCount] = 0;

	for (i = 0; i < Priv->CurrentFieldsCount; i++) {
		Priv->CurrentFields[i] = msg.Buffer[15 + i];
	}

	return GE_NONE;
}

static GSM_Error ALCATEL_GetFields(GSM_StateMachine *s, int id) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;
	int			i;
	unsigned char 		buffer[] =
					{0x00, 0x04,
					 0x00, 			/* type */
					 0x30, 0x01,
					 0x00, 0x00, 0x00, 0x00}; /* item */

	if (Priv->BinaryState != StateSession) return GE_UNKNOWN;
	if ((Priv->CurrentFieldsItem == id) && (Priv->CurrentFieldsType == Priv->BinaryType)) return GE_NONE;

	smprintf(s,"Reading item fields (%d)\n", id);

	buffer[5] = (id >> 24);
	buffer[6] = ((id >> 16) & 0xff);
	buffer[7] = ((id >> 8) & 0xff);
	buffer[8] = (id & 0xff);

	switch (Priv->BinaryType) {
		case TypeCalendar:
			buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
			break;
		case TypeContacts:
			buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
			break;
		case TypeToDo:
			buffer[2] = ALCATEL_SYNC_TYPE_TODO;
			break;
	}

	Priv->CurrentFieldsItem = id;
	Priv->CurrentFieldsType = Priv->BinaryType;

	error=GSM_WaitFor (s, buffer, 9, 0x02, ALCATEL_TIMEOUT, ID_AlcatelGetFields1);
	if (error != GE_NONE) return error;
	error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelGetFields2);
	if (error != GE_NONE) return error;

	i = 0;
	smprintf(s,"Received %d fields: ", Priv->CurrentFieldsCount);
	for (i=0; i < Priv->CurrentFieldsCount; i++) {
		smprintf(s,"%i ", Priv->CurrentFields[i]);
	}
	smprintf(s,"\n");

	return GE_NONE;
}

static GSM_Error ALCATEL_ReplyGetFieldValue(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;
	unsigned char 		*buffer = &(msg.Buffer[16]);

	if (buffer[1] == 0x05 && buffer[2] == 0x67) {
		/* date */
		Priv->ReturnType		= Alcatel_date;
		Priv->ReturnDateTime.Day	= buffer[4];
		Priv->ReturnDateTime.Month	= buffer[5];
		Priv->ReturnDateTime.Year	= buffer[7] + (buffer[6] << 8);
		Priv->ReturnDateTime.Timezone	= 0; /* FIXME: how to acquire this? */

		Priv->ReturnDateTime.Hour	= 0;
		Priv->ReturnDateTime.Minute	= 0;
		Priv->ReturnDateTime.Second	= 0;
	} else if (buffer[1] == 0x06 && buffer[2] == 0x68) {
		/* time */
		Priv->ReturnType		= Alcatel_time;
		Priv->ReturnDateTime.Hour	= buffer[4];
		Priv->ReturnDateTime.Minute	= buffer[5];
		Priv->ReturnDateTime.Second	= buffer[6];
		
		Priv->ReturnDateTime.Day	= 0;
		Priv->ReturnDateTime.Month	= 0;
		Priv->ReturnDateTime.Year	= 0;
		Priv->ReturnDateTime.Timezone	= 0;
	} else if (buffer[1] == 0x08 && buffer[2] == 0x3C) {
		/* string */
		Priv->ReturnType = Alcatel_string;
		if (GSM_PHONEBOOK_TEXT_LENGTH < buffer[3])
			smprintf(s, "WARNING: Text truncated, you should increase GSM_PHONEBOOK_TEXT_LENGTH to at least %d\n", buffer[3] + 1);
		if (Priv->ProtocolVersion == V_1_0) {
			DecodeDefault( Priv->ReturnString, buffer + 4, MIN(GSM_PHONEBOOK_TEXT_LENGTH, buffer[3]), false, GSM_AlcatelAlphabet);
		} else if(Priv->ProtocolVersion == V_1_1 && (buffer[4] & 0x80)) {
			memcpy(Priv->ReturnString, buffer + 5, buffer[3]);
			Priv->ReturnString[buffer[3] + 1] = 0;
			Priv->ReturnString[buffer[3] + 2] = 0;
			ReverseUnicodeString(Priv->ReturnString);
		} else {
			DecodeDefault( Priv->ReturnString, buffer + 4, MIN(GSM_PHONEBOOK_TEXT_LENGTH, buffer[3]), false, GSM_AlcatelAlphabet);
		}
	} else if (buffer[1] == 0x07 && buffer[2] == 0x3C) {
		/* phone */
		Priv->ReturnType = Alcatel_phone;
		if (GSM_PHONEBOOK_TEXT_LENGTH < buffer[3])
			smprintf(s, "WARNING: Text truncated, you should increase GSM_PHONEBOOK_TEXT_LENGTH to at least %d\n", buffer[3] + 1);
		if (Priv->ProtocolVersion == V_1_0) {
			DecodeDefault( Priv->ReturnString, buffer + 4, MIN(GSM_PHONEBOOK_TEXT_LENGTH, buffer[3]), false, GSM_AlcatelAlphabet);
		} else if(Priv->ProtocolVersion == V_1_1 && (buffer[4] & 0x80)) {
			memcpy(Priv->ReturnString, buffer + 5, buffer[3]);
			Priv->ReturnString[buffer[3] + 1] = 0;
			Priv->ReturnString[buffer[3] + 2] = 0;
			ReverseUnicodeString(Priv->ReturnString);
		} else {
			DecodeDefault( Priv->ReturnString, buffer + 4, MIN(GSM_PHONEBOOK_TEXT_LENGTH, buffer[3]), false, GSM_AlcatelAlphabet);
		}
	} else if (buffer[1] == 0x03 && buffer[2] == 0x3B) {
		/* boolean */
		Priv->ReturnType = Alcatel_bool;
		Priv->ReturnInt = buffer[3];
	} else if (buffer[1] == 0x02 && buffer[2] == 0x3A) {
		/* integer */
		Priv->ReturnType = Alcatel_int;
		Priv->ReturnInt = buffer[6] + (buffer[5] << 8) + (buffer[4] << 16) + (buffer[3] << 24);
	} else if (buffer[1] == 0x04 && buffer[2] == 0x38) {
		/* enumeration */
		Priv->ReturnType 	= Alcatel_enum;
		Priv->ReturnInt 	= buffer[3];
	} else if (buffer[1] == 0x00 && buffer[2] == 0x38) {
		/* byte */
		Priv->ReturnType 	= Alcatel_byte;
		Priv->ReturnInt 	= buffer[3];
	} else {
		smprintf(s, "WARNING: Uknown data type received (%02X,%02X)\n", buffer[1], buffer[2]);
		return GE_UNKNOWNRESPONSE;
	}
	return GE_NONE;
}

static GSM_Error ALCATEL_GetFieldValue(GSM_StateMachine *s, int id, int field)
{
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;
	unsigned char 		buffer[] =
					{0x00, 0x04,
					 0x00, 				/* type */
					 0x1f, 0x01,
					 0x00, 0x00, 0x00, 0x00,	/* here follows 4byte id */
					 0x00};				/* field */

	smprintf(s,"Reading item value (%08x.%02x)\n", id, field);

	switch (Priv->BinaryType) {
		case TypeCalendar:
			buffer[2] = ALCATEL_SYNC_TYPE_CALENDAR;
			break;
		case TypeContacts:
			buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
			break;
		case TypeToDo:
			buffer[2] = ALCATEL_SYNC_TYPE_TODO;
			break;
	}

	buffer[5] = (id >> 24);
	buffer[6] = ((id >> 16) & 0xff);
	buffer[7] = ((id >> 8) & 0xff);
	buffer[8] = (id & 0xff);
	buffer[9] = (field & 0xff);

	error=GSM_WaitFor (s, buffer, 10, 0x02, ALCATEL_TIMEOUT, ID_AlcatelGetFieldValue1);
	if (error != GE_NONE) return error;
	error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelGetFieldValue2);
	if (error != GE_NONE) return error;

	return GE_NONE;
}

static GSM_Error ALCATEL_ReplyGetCategories(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int 			i;

	/* Did we get any category? */
	if (msg.Buffer[4] == 6) {
		Priv->CurrentCategoriesCount = 0;
		return GE_NONE;
	}
	if (msg.Buffer[12] > ALCATEL_MAX_CATEGORIES) {
		smprintf(s, "WARNING: Field list truncated, you should increase ALCATEL_MAX_CATEGORIES to at least %d\n", msg.Buffer[12]);
		Priv->CurrentCategoriesCount = ALCATEL_MAX_CATEGORIES;
	} else {
		Priv->CurrentCategoriesCount = msg.Buffer[12];
	}

	for (i = 0; i < Priv->CurrentCategoriesCount; i++) {
		Priv->CurrentCategories[i] = msg.Buffer[13 + i];
	}

	return GE_NONE;
}

static GSM_Error ALCATEL_GetAvailableCategoryIds(GSM_StateMachine *s) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;
	int		 	i;
	unsigned char 		buffer[] =
					{0x00, 0x04,
					 0x00 /*type */,
					 0x0b,
					 0x00 /* list */};

	if (Priv->BinaryState != StateSession) return GE_UNKNOWN;
	switch (Priv->BinaryType) {
		case TypeContacts:
			buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
			buffer[4] = ALCATEL_LIST_CONTACTS_CAT;
			break;
		case TypeToDo:
			buffer[2] = ALCATEL_SYNC_TYPE_TODO;
			buffer[4] = ALCATEL_LIST_TODO_CAT;
			break;
		default:
			return GE_NOTSUPPORTED;
	}

	if (Priv->CurrentCategoriesType == Priv->BinaryType) return GE_NONE;
	Priv->CurrentCategoriesType = Priv->BinaryType;

	smprintf(s,"Reading category list\n");

	error=GSM_WaitFor (s, buffer, 5, 0x02, ALCATEL_TIMEOUT, ID_AlcatelGetCategories1);
	if (error != GE_NONE) return error;

	error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelGetCategories2);
	if (error != GE_NONE) return error;

	i = 0;
	smprintf(s,"Received %d ids: ", Priv->CurrentCategoriesCount);
	for (i=0; i < Priv->CurrentCategoriesCount; i++) {
		smprintf(s,"%i ", Priv->CurrentCategories[i]);
	}
	smprintf(s,"\n");

	return GE_NONE;
}

static GSM_Error ALCATEL_IsCategoryIdAvailable(GSM_StateMachine *s, int id) {
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int 			i = 0;

	if (Priv->CurrentCategoriesType != Priv->BinaryType) return GE_UNKNOWN;

	for (i = 0; i< Priv->CurrentCategoriesCount; i++) {
		if (Priv->CurrentCategories[i] == id) return GE_NONE;
	}
	return GE_EMPTY;
}

static GSM_Error ALCATEL_ReplyGetCategoryText(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int len;

	len = msg.Buffer[14];
	if (len > GSM_MAX_CATEGORY_NAME_LENGTH) {
		smprintf(s, "WARNING: Category name truncated, you should increase GSM_MAX_CATEGORY_NAME_LENGTH to at least %d\n", len);
	}
	if (Priv->ProtocolVersion == V_1_0) {
		DecodeDefault( Priv->ReturnString, msg.Buffer + 15, MIN(GSM_MAX_CATEGORY_NAME_LENGTH, len), false, GSM_AlcatelAlphabet);
	} else if(Priv->ProtocolVersion == V_1_1 && (msg.Buffer[15] & 0x80)) {
		memcpy(Priv->ReturnString, msg.Buffer + 16, len);
		Priv->ReturnString[len + 1] = 0;
		Priv->ReturnString[len + 2] = 0;
		ReverseUnicodeString(Priv->ReturnString);
	} else {
		DecodeDefault( Priv->ReturnString, msg.Buffer + 15, MIN(GSM_MAX_CATEGORY_NAME_LENGTH, len), false, GSM_AlcatelAlphabet);
	}
	return GE_NONE;
}

static GSM_Error ALCATEL_GetCategoryText(GSM_StateMachine *s, int id) {
	unsigned char		buffer[] = {0x00, 0x04, 0x00 /*type*/, 0x0c, 0x00 /*list*/, 0x0A, 0x01, 0x00 /*item*/ };
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error		error;

	switch (Priv->BinaryType) {
		case TypeContacts:
			buffer[2] = ALCATEL_SYNC_TYPE_CONTACTS;
			buffer[4] = ALCATEL_LIST_CONTACTS_CAT;
			break;
		case TypeToDo:
			buffer[2] = ALCATEL_SYNC_TYPE_TODO;
			buffer[4] = ALCATEL_LIST_TODO_CAT;
			break;
		default:
			return GE_NOTSUPPORTED;
	}

	buffer[7] = (id & 0xff);

	error=GSM_WaitFor (s, buffer, 8, 0x02, ALCATEL_TIMEOUT, ID_AlcatelGetCategoryText1);
	if (error != GE_NONE) return error;
	error=GSM_WaitFor (s, 0, 0, 0x00, ALCATEL_TIMEOUT, ID_AlcatelGetCategoryText2);
	if (error != GE_NONE) return error;

	return GE_NONE;
}


static GSM_Error ALCATEL_GetManufacturer(GSM_StateMachine *s)
{
	strcpy(s->Phone.Data.Manufacturer, "Alcatel");
	return GE_NONE;
}

static GSM_Error ALCATEL_GetIMEI (GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_GetIMEI(s);
}

static GSM_Error ALCATEL_GetFirmware(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_GetFirmware(s);
}

static GSM_Error ALCATEL_GetModel(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_GetModel(s);
}

static GSM_Error ALCATEL_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_GetDateTime(s, date_time);
}

static GSM_Error ALCATEL_GetAlarm(GSM_StateMachine *s, GSM_DateTime *alarm, int alarm_number)
{
	GSM_Error error;

	if (alarm_number != 1) return GE_NOTSUPPORTED;
	/* XXX: In fact phone reports 5 alarms, but setting more than one really
	 * confuses it */
	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	/* TODO: this maybe should be done in binary mode, because alarms over AT
	 * commands seems to be a bit unreliable */
	return ATGEN_GetAlarm(s, alarm, alarm_number);
}

static GSM_Error ALCATEL_GetMemory(GSM_StateMachine *s, GSM_PhonebookEntry *entry)
{
	GSM_Error 		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int 			i;
	int			j = 0;

	if (entry->MemoryType == GMT_ME) {
		if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= GE_NONE) return error;
		if ((error = ALCATEL_GetAvailableIds(s, false))!= GE_NONE) return error;
		if ((error = ALCATEL_IsIdAvailable(s, entry->Location))!= GE_NONE) {
			entry->EntriesNum = 0;
			return error;
		}
		if ((error = ALCATEL_GetFields(s, entry->Location))!= GE_NONE) return error;

		entry->EntriesNum = Priv->CurrentFieldsCount;
		entry->PreferUnicode = false;

		for (i=0; i<Priv->CurrentFieldsCount; i++) {
			if ((error = ALCATEL_GetFieldValue(s, entry->Location, Priv->CurrentFields[i]))!= GE_NONE) return error;
			entry->Entries[i].VoiceTag = 0;
			switch (Priv->CurrentFields[i]) {
				case 0:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 0, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_LastName;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 1:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 1, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_FirstName;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 2:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 2, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Company;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 3:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 3, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_JobTitle;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 4:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 4, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Note;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 5:
					if (Priv->ReturnType != Alcatel_byte) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 5, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Category;
					entry->Entries[i - j].Number = Priv->ReturnInt;
					break;
				case 6:
					if (Priv->ReturnType != Alcatel_bool) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 6, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Private;
					entry->Entries[i - j].Number = Priv->ReturnInt;
					break;
				case 7:
					if (Priv->ReturnType != Alcatel_phone) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 7, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Number_Work;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 8:
					if (Priv->ReturnType != Alcatel_phone) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 8, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Number_General;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 9:
					if (Priv->ReturnType != Alcatel_phone) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 9, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Number_Fax;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 10:
					if (Priv->ReturnType != Alcatel_phone) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 10, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Number_Other;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 11:
					if (Priv->ReturnType != Alcatel_phone) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 11, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Number_Pager;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 12:
					if (Priv->ReturnType != Alcatel_phone) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 12, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Number_Mobile;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 13:
					if (Priv->ReturnType != Alcatel_phone) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 13, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Number_Home;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 14:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 14, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Email;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 15:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 15, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Email;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 16:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 16, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_StreetAddress;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 17:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 17, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_City;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 18:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 18, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_State;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 19:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 19, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Zip;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 20:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 20, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Country;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 21:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 21, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Custom1;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 22:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 22, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Custom2;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 23:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 23, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Custom3;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 24:
					if (Priv->ReturnType != Alcatel_string) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 24, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_Text_Custom4;
					CopyUnicodeString(entry->Entries[i - j].Text, Priv->ReturnString);
					break;
				case 25:
					if (Priv->ReturnType != Alcatel_int) {
						smprintf(s,"WARNING: Received unexpected type %02X for field 25, ignoring\n", Priv->ReturnType);
						entry->EntriesNum--;
						j++;
						break;
					}
					entry->Entries[i - j].EntryType = PBK_PictureID;
					entry->Entries[i - j].Number = Priv->ReturnInt;
					break;
				default:
					entry->EntriesNum--;
					j++;
					smprintf(s,"WARNING: Received unknown field %02X, ignoring. Type = %02X. Value = ", Priv->CurrentFields[i], Priv->ReturnType);
					switch (Priv->ReturnType) {
						case Alcatel_date:
							smprintf(s, "%d.%d.%d", Priv->ReturnDateTime.Day, Priv->ReturnDateTime.Month, Priv->ReturnDateTime.Year);
							break;
						case Alcatel_time:
							smprintf(s, "%d:%d:%d", Priv->ReturnDateTime.Hour, Priv->ReturnDateTime.Minute, Priv->ReturnDateTime.Second);
							break;
						case Alcatel_string:
						case Alcatel_phone:
							smprintf(s, "\"%s\"",DecodeUnicodeString(Priv->ReturnString));
							break;
						case Alcatel_enum:
						case Alcatel_bool:
						case Alcatel_int:
						case Alcatel_byte:
							smprintf(s, "%d", Priv->ReturnInt);
							break;
					}
					smprintf(s,"\n");
			}
		}
		return GE_NONE;
	} else {
		if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
		return ATGEN_GetMemory(s, entry);
	}
}

static GSM_Error ALCATEL_SetMemory(GSM_StateMachine *s, GSM_PhonebookEntry *entry)
{
	GSM_Error error;

	if (entry->MemoryType == GMT_ME) {
		if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= GE_NONE) return error;
		if ((error = ALCATEL_GetAvailableIds(s, false))!= GE_NONE) return error;
		if ((error = ALCATEL_IsIdAvailable(s, entry->Location))!= GE_NONE) return error;
		if ((error = ALCATEL_GetFields(s, entry->Location))!= GE_NONE) return error;
		/* TODO: do the real setting */
		return GE_WORKINPROGRESS;
	} else {
		if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
		return ATGEN_SetMemory(s, entry);
	}
}


static GSM_Error ALCATEL_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	GSM_Phone_ALCATELData 	*Priv = &s->Phone.Data.Priv.ALCATEL;
	GSM_Error 		error;

	if (Status->MemoryType == GMT_ME) {
		if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeContacts, 0))!= GE_NONE) return error;
		if ((error = ALCATEL_GetAvailableIds(s, false))!= GE_NONE) return error;
		Status->Used = Priv->ContactsItemsCount;
		Status->Free = ALCATEL_FREE_MEMORY;
		return GE_NONE;
	} else {
		if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
		return ATGEN_GetMemoryStatus(s, Status);
	}
}

static GSM_Error ALCATEL_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_GetSMSC(s, smsc);
}

static GSM_Error ALCATEL_GetSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_GetSMSMessage(s, sms);
}

static GSM_Error ALCATEL_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_GetBatteryCharge(s, bat);
}

static GSM_Error ALCATEL_GetSignalStrength(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_GetSignalQuality(s, sig);
}

static GSM_Error ALCATEL_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_GetSMSFolders(s, folders);
}

static GSM_Error ALCATEL_GetNextSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_GetNextSMSMessage(s, sms, start);
}

static GSM_Error ALCATEL_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_GetSMSStatus(s, status);
}

static GSM_Error ALCATEL_DialVoice(GSM_StateMachine *s, char *number)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_DialVoice(s, number);
}

static GSM_Error ALCATEL_AnswerCall(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_AnswerCall(s);
}

static GSM_Error ALCATEL_CancelCall(GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_CancelCall(s);
}

static GSM_Error ALCATEL_SaveSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_SaveSMSMessage(s, sms);
}

static GSM_Error ALCATEL_SendSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_SendSMSMessage(s, sms);
}

static GSM_Error ALCATEL_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_SetDateTime(s, date_time);
}

static GSM_Error ALCATEL_DeleteSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_DeleteSMSMessage(s, sms);
}

static GSM_Error ALCATEL_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_SetSMSC(s, smsc);
}

static GSM_Error ALCATEL_EnterSecurityCode(GSM_StateMachine *s, GSM_SecurityCode Code)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_EnterSecurityCode(s, Code);
}

static GSM_Error ALCATEL_GetSecurityStatus(GSM_StateMachine *s, GSM_SecurityCodeType *Status)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_GetSecurityStatus(s, Status);
}

static GSM_Error ALCATEL_ResetPhoneSettings(GSM_StateMachine *s, GSM_ResetSettingsType Type)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_ResetPhoneSettings(s, Type);
}

static GSM_Error ALCATEL_SendDTMF(GSM_StateMachine *s, char *sequence)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_SendDTMF(s, sequence);
}

static GSM_Error ALCATEL_GetSIMIMSI(GSM_StateMachine *s, char *IMSI)
{
	GSM_Error error;

	if ((error = ALCATEL_SetATMode(s))!= GE_NONE) return error;
	return ATGEN_GetSIMIMSI(s, IMSI);
}

static GSM_Error ALCATEL_SetAlarm (GSM_StateMachine *s, GSM_DateTime *alarm, int alarm_number)
{
	GSM_Error error;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= GE_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, false))!= GE_NONE) return error;
	/* TODO: do the real setting */
	return GE_WORKINPROGRESS;
}

static GSM_Error ALCATEL_GetNextCalendarNote(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start)
{
	GSM_Error		error;
	GSM_DateTime		*dt = NULL;
	GSM_DateTime		evdate;
	bool			evdateused = true;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int			i;
	int			j=0;

	if (start) Note->Location = -1;
	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= GE_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, start))!= GE_NONE) return error;
	if ((error = ALCATEL_GetNextId(s, &Note->Location))!= GE_NONE) {
		Note->EntriesNum = 0;
		return error;
	}
	if ((error = ALCATEL_GetFields(s, Note->Location))!= GE_NONE) return error;

	Note->EntriesNum = Priv->CurrentFieldsCount;

	for (i=0; i < Priv->CurrentFieldsCount; i++) {
		if ((error = ALCATEL_GetFieldValue(s, Note->Location, Priv->CurrentFields[i]))!= GE_NONE) return error;
		switch (Priv->CurrentFields[i]) {
			case 0:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				j++;
				Note->EntriesNum--;
				evdate = Priv->ReturnDateTime;
				evdateused = false;
				break;
			case 1:
				if (Priv->ReturnType != Alcatel_time) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_START_DATETIME;
				Note->Entries[i-j].Date = Priv->ReturnDateTime;
				Note->Entries[i-j].Date.Day = evdate.Day;
				Note->Entries[i-j].Date.Month = evdate.Month;
				Note->Entries[i-j].Date.Year = evdate.Year;
				Note->Entries[i-j].Date.Timezone = evdate.Timezone;
				evdateused = true;
				break;
			case 2:
				if (Priv->ReturnType != Alcatel_time) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_STOP_DATETIME;
				Note->Entries[i-j].Date = Priv->ReturnDateTime;
				Note->Entries[i-j].Date.Day = evdate.Day;
				Note->Entries[i-j].Date.Month = evdate.Month;
				Note->Entries[i-j].Date.Year = evdate.Year;
				Note->Entries[i-j].Date.Timezone = evdate.Timezone;
				evdateused = true;
				break;
			case 3:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				if (dt == NULL) {
					Note->Entries[i-j].EntryType = CAL_ALARM_DATETIME;
					Note->Entries[i-j].Date = Priv->ReturnDateTime;
					dt = &(Note->Entries[i-j].Date);
				} else {
					j++;
					Note->EntriesNum--;
					dt->Day = Priv->ReturnDateTime.Day;
					dt->Month = Priv->ReturnDateTime.Month;
					dt->Year = Priv->ReturnDateTime.Year;
					dt->Timezone = Priv->ReturnDateTime.Timezone;
					dt = NULL;
				}
				break;
			case 4:
				if (Priv->ReturnType != Alcatel_time) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				if (dt == NULL) {
					Note->Entries[i-j].EntryType = CAL_ALARM_DATETIME;
					Note->Entries[i-j].Date = Priv->ReturnDateTime;
					dt = &(Note->Entries[i-j].Date);
				} else {
					j++;
					Note->EntriesNum--;
					dt->Hour = Priv->ReturnDateTime.Hour;
					dt->Minute = Priv->ReturnDateTime.Minute;
					dt->Second = Priv->ReturnDateTime.Second;
					dt = NULL;
				}
				break;
			case 5:
				if (Priv->ReturnType != Alcatel_string) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_TEXT;
				CopyUnicodeString(Note->Entries[i-j].Text, Priv->ReturnString);
				break;
			case 6:
				if (Priv->ReturnType != Alcatel_bool) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_PRIVATE;
				Note->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 7:
				if (Priv->ReturnType != Alcatel_enum) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				switch (Priv->ReturnInt) {
					case 0:
						Note->Type = GCN_MEETING;
						break;
					case 2:
						Note->Type = GCN_BIRTHDAY;
						break;
					case 3:
						Note->Type = GCN_CALL;
						break;
					case 4:
						Note->Type = GCN_ALARM;
						break;
					case 5:
						Note->Type = GCN_DAILY_ALARM;
						break;
					case 9:
						/* I'd call this repeating event, but it makes no sense creating one more type ... */
						Note->Type = GCN_MEETING;
						break;
				}
				j++;
				Note->EntriesNum--;
				break;
			case 8:
				if (Priv->ReturnType != Alcatel_int) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				/* 0xffffffff indicates that there is phone (BF5), 0 means none (BF5, BE5)*/
				if (Priv->ReturnInt == 0xffffffff || Priv->ReturnInt == 0) {
					j++;
					Note->EntriesNum--;
				} else {
					Note->Entries[i-j].EntryType = CAL_CONTACTID;
					Note->Entries[i-j].Number = Priv->ReturnInt;
				}
				break;
			case 9:
				if (Priv->ReturnType != Alcatel_phone) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_PHONE;
				CopyUnicodeString(Note->Entries[i-j].Text, Priv->ReturnString);
				break;
			case 10:
				if (Priv->ReturnType != Alcatel_byte) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_REPEAT_DAYOFWEEK;
				Note->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 11:
				if (Priv->ReturnType != Alcatel_byte) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_REPEAT_DAY;
				Note->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 12:
				if (Priv->ReturnType != Alcatel_byte) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_REPEAT_WEEKOFMONTH;
				Note->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 13:
				if (Priv->ReturnType != Alcatel_byte) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_REPEAT_MONTH;
				Note->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 17:
				if (Priv->ReturnType != Alcatel_byte) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				/* In BF5 birthday has frequency = 1 */
				if (Note->Type == GCN_BIRTHDAY) {
					Note->EntriesNum--;
					j++;
				} else {
					Note->Entries[i-j].EntryType = CAL_REPEAT_FREQUENCY;
					Note->Entries[i-j].Number = Priv->ReturnInt;
				}
				break;
			case 18:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_REPEAT_STARTDATE;
				Note->Entries[i-j].Date = Priv->ReturnDateTime;
				break;
			case 19:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				Note->Entries[i-j].EntryType = CAL_REPEAT_STOPDATE;
				Note->Entries[i-j].Date = Priv->ReturnDateTime;
				break;
			case 20:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				/* This entry had always same value as the 3rd (alarm date) */
				j++;
				Note->EntriesNum--;
				break;
			case 21:
				if (Priv->ReturnType != Alcatel_time) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					Note->EntriesNum--;
					j++;
					break;
				}
				/* This entry had always same value as the 4th (alarm time) */
				j++;
				Note->EntriesNum--;
				break;
			default:
				Note->EntriesNum--;
				j++;
				smprintf(s,"WARNING: Received unknown field %02X, ignoring. Type = %02X. Value = ", Priv->CurrentFields[i], Priv->ReturnType);
				switch (Priv->ReturnType) {
					case Alcatel_date:
						smprintf(s, "%d.%d.%d", Priv->ReturnDateTime.Day, Priv->ReturnDateTime.Month, Priv->ReturnDateTime.Year);
						break;
					case Alcatel_time:
						smprintf(s, "%d:%d:%d", Priv->ReturnDateTime.Hour, Priv->ReturnDateTime.Minute, Priv->ReturnDateTime.Second);
						break;
					case Alcatel_string:
					case Alcatel_phone:
						smprintf(s, "\"%s\"",DecodeUnicodeString(Priv->ReturnString));
						break;
					case Alcatel_enum:
					case Alcatel_bool:
					case Alcatel_int:
					case Alcatel_byte:
						smprintf(s, "%d", Priv->ReturnInt);
						break;
				}
				smprintf(s,"\n");
		}
	}
	/* The event didn't have start/stop time -> we need only date */
	if (!evdateused) {
		j--;
		Note->EntriesNum++;
		Note->Entries[i-j].EntryType = CAL_START_DATETIME;
		Note->Entries[i-j].Date = evdate;
	}
	return GE_NONE;
}

static GSM_Error ALCATEL_DelCalendarNote(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_Error error;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= GE_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, false))!= GE_NONE) return error;
	/* TODO: do the real deleting */
	return GE_WORKINPROGRESS;
}

static GSM_Error ALCATEL_AddCalendarNote(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool Past)
{
	GSM_Error error;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeCalendar, 0))!= GE_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, false))!= GE_NONE) return error;
	/* TODO: do the real setting */
	return GE_WORKINPROGRESS;
}

static GSM_Error ALCATEL_GetToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool refresh)
{
	GSM_Error		error;
	GSM_DateTime		*dt = NULL;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;
	int			i;
	int			j=0;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeToDo, 0))!= GE_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, refresh))!= GE_NONE) return error;
	if ((error = ALCATEL_IsIdAvailable(s, ToDo->Location))!= GE_NONE) {
		ToDo->EntriesNum = 0;
		return error;
	}
	if ((error = ALCATEL_GetFields(s, ToDo->Location))!= GE_NONE) return error;

	ToDo->EntriesNum = Priv->CurrentFieldsCount;

	for (i=0; i < Priv->CurrentFieldsCount; i++) {
		if ((error = ALCATEL_GetFieldValue(s, ToDo->Location, Priv->CurrentFields[i]))!= GE_NONE) return error;
		switch (Priv->CurrentFields[i]) {
			case 0:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				ToDo->Entries[i-j].EntryType = TODO_DUEDATE;
				ToDo->Entries[i-j].Date = Priv->ReturnDateTime;
				break;
			case 1:
				if (Priv->ReturnType != Alcatel_bool) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				ToDo->Entries[i-j].EntryType = TODO_COMPLETED;
				ToDo->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 2:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				if (dt == NULL) {
					ToDo->Entries[i-j].EntryType = TODO_ALARM_DATETIME;
					ToDo->Entries[i-j].Date = Priv->ReturnDateTime;
					dt = &(ToDo->Entries[i-j].Date);
				} else {
					j++;
					ToDo->EntriesNum--;
					dt->Day = Priv->ReturnDateTime.Day;
					dt->Month = Priv->ReturnDateTime.Month;
					dt->Year = Priv->ReturnDateTime.Year;
					dt->Timezone = Priv->ReturnDateTime.Timezone;
					dt = NULL;
				}
				break;
			case 3:
				if (Priv->ReturnType != Alcatel_time) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				if (dt == NULL) {
					ToDo->Entries[i-j].EntryType = TODO_ALARM_DATETIME;
					ToDo->Entries[i-j].Date = Priv->ReturnDateTime;
					dt = &(ToDo->Entries[i-j].Date);
				} else {
					j++;
					ToDo->EntriesNum--;
					dt->Hour = Priv->ReturnDateTime.Hour;
					dt->Minute = Priv->ReturnDateTime.Minute;
					dt->Second = Priv->ReturnDateTime.Second;
					dt = NULL;
				}
				break;
			case 4:
				if (Priv->ReturnType != Alcatel_string) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				ToDo->Entries[i-j].EntryType = TODO_TEXT;
				CopyUnicodeString(ToDo->Entries[i-j].Text, Priv->ReturnString);
				break;
			case 5:
				if (Priv->ReturnType != Alcatel_bool) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				ToDo->Entries[i-j].EntryType = TODO_PRIVATE;
				ToDo->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 6:
				if (Priv->ReturnType != Alcatel_byte) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				ToDo->Entries[i-j].EntryType = TODO_CATEGORY;
				ToDo->Entries[i-j].Number = Priv->ReturnInt;
				break;
			case 7:
				/* This one seems to be byte for BF5 and enum for BE5 */
				if (Priv->ReturnType != Alcatel_enum && Priv->ReturnType != Alcatel_byte) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				switch (Priv->ReturnInt) {
					case 0:
						ToDo->Priority = GSM_Priority_High;
						break;
					case 1:
						ToDo->Priority = GSM_Priority_Medium;
						break;
					case 2:
						ToDo->Priority = GSM_Priority_Low;
						break;
					default:
						ToDo->Priority = GSM_Priority_Medium;
						smprintf(s,"WARNING: Received unexpected priority %02X, ignoring\n", Priv->ReturnInt);
				}
				j++;
				ToDo->EntriesNum--;
				break;
			case 8:
				if (Priv->ReturnType != Alcatel_int) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				/* 0xffffffff indicates that there is phone, 0 means none */
				if (Priv->ReturnInt == 0xffffffff || Priv->ReturnInt == 0) {
					j++;
					ToDo->EntriesNum--;
				} else {
					ToDo->Entries[i-j].EntryType = TODO_CONTACTID;
					ToDo->Entries[i-j].Number = Priv->ReturnInt;
				}
				break;
			case 9:
				if (Priv->ReturnType != Alcatel_phone) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				ToDo->Entries[i-j].EntryType = TODO_PHONE;
				CopyUnicodeString(ToDo->Entries[i-j].Text, Priv->ReturnString);
				break;
			case 10:
				if (Priv->ReturnType != Alcatel_date) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				/* This entry had always same value as the 2nd (alarm date) */
				j++;
				ToDo->EntriesNum--;
				break;
			case 11:
				if (Priv->ReturnType != Alcatel_time) {
					smprintf(s,"WARNING: Received unexpected type %02X, ignoring\n", Priv->ReturnType);
					ToDo->EntriesNum--;
					j++;
					break;
				}
				/* This entry had always same value as the 3rd (alarm time) */
				j++;
				ToDo->EntriesNum--;
				break;
			default:
				ToDo->EntriesNum--;
				j++;
				smprintf(s,"WARNING: Received unknown field %02X, ignoring. Type = %02X. Value = ", Priv->CurrentFields[i], Priv->ReturnType);
				switch (Priv->ReturnType) {
					case Alcatel_date:
						smprintf(s, "%d.%d.%d", Priv->ReturnDateTime.Day, Priv->ReturnDateTime.Month, Priv->ReturnDateTime.Year);
						break;
					case Alcatel_time:
						smprintf(s, "%d:%d:%d", Priv->ReturnDateTime.Hour, Priv->ReturnDateTime.Minute, Priv->ReturnDateTime.Second);
						break;
					case Alcatel_string:
					case Alcatel_phone:
						smprintf(s, "\"%s\"",DecodeUnicodeString(Priv->ReturnString));
						break;
					case Alcatel_enum:
					case Alcatel_bool:
					case Alcatel_int:
					case Alcatel_byte:
						smprintf(s, "%d", Priv->ReturnInt);
						break;
				}
				smprintf(s,"\n");
		}
	}
	return GE_NONE;
}

static GSM_Error ALCATEL_DeleteAllToDo (GSM_StateMachine *s)
{
	GSM_Error error;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeToDo, 0))!= GE_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, false))!= GE_NONE) return error;
	/* TODO: do the real deleting */
	return GE_WORKINPROGRESS;
}

static GSM_Error ALCATEL_SetToDo (GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error error;

	if ((error = ALCATEL_GoToBinaryState(s, StateSession, TypeToDo, 0))!= GE_NONE) return error;
	if ((error = ALCATEL_GetAvailableIds(s, false))!= GE_NONE) return error;
	/* TODO: do the real setting */
	return GE_WORKINPROGRESS;
}

static GSM_Error ALCATEL_GetCategoryStatus(GSM_StateMachine *s, GSM_CategoryStatus *Status)
{
	GSM_Alcatel_BinaryType	type;
	GSM_Error		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	switch (Status->Type) {
		case Category_ToDo: type = TypeToDo; break;
		case Category_Phonebook: type = TypeContacts; break;
		default: return GE_NOTSUPPORTED;
	}
	if ((error = ALCATEL_GoToBinaryState(s, StateSession, type, 0))!= GE_NONE) return error;
	if ((error = ALCATEL_GetAvailableCategoryIds(s))!= GE_NONE) return error;

	Status->Used = Priv->CurrentCategoriesCount;

	return GE_NONE;
}

static GSM_Error ALCATEL_GetCategory(GSM_StateMachine *s, GSM_Category *Category)
{
	GSM_Alcatel_BinaryType	type;
	GSM_Error		error;
	GSM_Phone_ALCATELData	*Priv = &s->Phone.Data.Priv.ALCATEL;

	switch (Category->Type) {
		case Category_ToDo: type = TypeToDo; break;
		case Category_Phonebook: type = TypeContacts; break;
		default: return GE_NOTSUPPORTED;
	}
	if ((error = ALCATEL_GoToBinaryState(s, StateSession, type, 0))!= GE_NONE) return error;
	if ((error = ALCATEL_GetAvailableCategoryIds(s))!= GE_NONE) return error;
	if ((error = ALCATEL_IsCategoryIdAvailable(s, Category->Location))!= GE_NONE) return error;
	if ((error = ALCATEL_GetCategoryText(s, Category->Location))!= GE_NONE) return error;

	CopyUnicodeString(Category->Name, Priv->ReturnString);

	return GE_NONE;
}

static GSM_Error ALCATEL_DispatchMessage(GSM_StateMachine *s)
{
	if (s->Phone.Data.Priv.ALCATEL.Mode == ModeBinary) {
		return GSM_DispatchMessage(s);
	} else {
		return ATGEN_DispatchMessage(s);
	}
}

static GSM_Error ALCATEL_ReplyGeneric(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	/* All error values are just VERY wild guesses, but these seems to work
	 * almost as expected ...
	 */
	switch (msg.Buffer[8]) {
		case 0x00: /* no error */
			return GE_NONE;
		case 0x10: /* same thing opened in phone menus */
			return GE_INSIDEPHONEMENU;
		case 0x13:
			/* This appears in more cases:
			 *	- phone needs PIN code
			 *	- we want to close not opened session
			 * For normal users the second case shouldn't occur...
			 */
			return GE_SECURITYERROR;
		case 0x14: /* Bad data */
		case 0x2f: /* Closing session when not opened */
		case 0x1f: /* Bad in/out counter in packet/ack */
		case 0x0e: /* Openning session when not closed */
		case 0x0C: /* Bad id (item/database) */
		case 0x11: /* Bad list id */
		case 0x2A: /* Nonexistant field/item id */
			return GE_BUG;
		case 0x23: /* Session opened */
		case 0x80: /* Transfer started */
			return GE_NONE;
		case 0x82: /* Transfer canceled */
			return GE_CANCELED;
		default:
			smprintf(s, "WARNING: Packet seems to indicate some status by %02X, ignoring!\n", msg.Buffer[8]);
			return GE_NONE;
	}
}

static GSM_Error ALCATEL_ReplyCommit(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	/* TODO: read received id */
	return GE_NONE;
}

static GSM_Reply_Function ALCATELReplyFunctions[] = {
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelAttach		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelDetach		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelCommit		},
{ALCATEL_ReplyCommit,		"\x02",0x00,0x00, ID_AlcatelCommit2		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelEnd			},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelClose		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelStart		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelSelect1		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelSelect2		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelSelect3		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelBegin1		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelBegin2		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelGetIds1		},
{ALCATEL_ReplyGetIds,		"\x02",0x00,0x00, ID_AlcatelGetIds2		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelGetCategories1	},
{ALCATEL_ReplyGetCategories,	"\x02",0x00,0x00, ID_AlcatelGetCategories2	},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelGetCategoryText1	},
{ALCATEL_ReplyGetCategoryText,	"\x02",0x00,0x00, ID_AlcatelGetCategoryText2	},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelGetFields1		},
{ALCATEL_ReplyGetFields,	"\x02",0x00,0x00, ID_AlcatelGetFields2		},
{ALCATEL_ReplyGeneric,		"\x02",0x00,0x00, ID_AlcatelGetFieldValue1	},
{ALCATEL_ReplyGetFieldValue,	"\x02",0x00,0x00, ID_AlcatelGetFieldValue2	},
{NULL,				"\x00",0x00,0x00, ID_None			}
};

GSM_Phone_Functions ALCATELPhone = {
	/* AFAIK, any 50[0123] phone should work, but I'm not sure whether all
	 * they were ever really released, if yes add them here also.
	 */
	"alcatel|OT501|OT701|OT715|BE5|BF5",
	ALCATELReplyFunctions,
	ALCATEL_Initialise,
	ALCATEL_Terminate,
	ALCATEL_DispatchMessage,
	ALCATEL_GetModel,
	ALCATEL_GetFirmware,
	ALCATEL_GetIMEI,
	ALCATEL_GetDateTime,
	ALCATEL_GetAlarm,
	ALCATEL_GetMemory,
	ALCATEL_GetMemoryStatus,
	ALCATEL_GetSMSC,
	ALCATEL_GetSMSMessage,
	ALCATEL_GetSMSFolders,
	ALCATEL_GetManufacturer,
	ALCATEL_GetNextSMSMessage,
	ALCATEL_GetSMSStatus,
	NOTIMPLEMENTED,			/* SetIncomingSMS	*/
	NOTSUPPORTED,			/* GetNetworkInfo	*/
	NOTSUPPORTED,			/* Reset		*/
	ALCATEL_DialVoice,
	ALCATEL_AnswerCall,
	ALCATEL_CancelCall,
	NOTSUPPORTED,			/* GetRingtone		*/
	NOTSUPPORTED,			/* GetWAPBookmark	*/
	NOTSUPPORTED,			/* GetBitmap		*/
	NOTSUPPORTED,			/* SetRingtone		*/
	ALCATEL_SaveSMSMessage,
	ALCATEL_SendSMSMessage,
	ALCATEL_SetDateTime,
	ALCATEL_SetAlarm,
	NOTSUPPORTED,			/* SetBitmap		*/
	ALCATEL_SetMemory,
	ALCATEL_DeleteSMSMessage,
	NOTSUPPORTED,			/* SetWAPBookmark	*/
	NOTSUPPORTED,			/* DeleteWAPBookmark	*/
	NOTSUPPORTED,			/* GetWAPSettings	*/
	NOTIMPLEMENTED,			/* SetIncomingCB	*/
	ALCATEL_SetSMSC,
	NOTSUPPORTED,			/* GetManufactureMonth	*/
	NOTSUPPORTED,			/* GetProductCode	*/
	NOTSUPPORTED,			/* GetOriginalIMEI	*/
	NOTSUPPORTED,			/* GetHardware		*/
	NOTSUPPORTED,			/* GetPPM		*/
	NOTSUPPORTED,			/* PressKey		*/
	ALCATEL_GetToDo,
	ALCATEL_DeleteAllToDo,
	ALCATEL_SetToDo,
	NOTSUPPORTED,			/* PlayTone		*/
	ALCATEL_EnterSecurityCode,
	ALCATEL_GetSecurityStatus,
	NOTSUPPORTED,			/* GetProfile		*/
	NOTSUPPORTED,			/* GetRingtonesInfo	*/
	NOTSUPPORTED,			/* SetWAPSettings	*/
	NOTSUPPORTED,			/* GetSpeedDial		*/
	NOTSUPPORTED,			/* SetSpeedDial		*/
	ALCATEL_ResetPhoneSettings,
	ALCATEL_SendDTMF,
	NOTSUPPORTED,			/* GetDisplayStatus	*/
	NOTSUPPORTED,			/* SetAutoNetworkLogin	*/
	NOTSUPPORTED,			/* SetProfile		*/
	ALCATEL_GetSIMIMSI,
	NONEFUNCTION,			/* SetIncomingCall	*/
	ALCATEL_GetNextCalendarNote,
	ALCATEL_DelCalendarNote,
	ALCATEL_AddCalendarNote,
	ALCATEL_GetBatteryCharge,
	ALCATEL_GetSignalStrength,
	ALCATEL_GetCategory,
	ALCATEL_GetCategoryStatus,
	NOTSUPPORTED,			/* GetFMStation		*/
	NOTSUPPORTED,			/* SetFMStation		*/
	NOTSUPPORTED,			/* ClearFMStations	*/
	NOTSUPPORTED,			/* SetIncomingUSSD	*/
	NOTSUPPORTED,			/* DeleteUserRingtones	*/
	NOTSUPPORTED,			/* ShowStartInfo	*/
	NOTSUPPORTED,			/* GetNextFileFolder	*/
	NOTSUPPORTED,			/* GetFile		*/
	NOTSUPPORTED,			/* AddFile		*/
	NOTSUPPORTED 			/* GetFreeFileMemory 	*/
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
