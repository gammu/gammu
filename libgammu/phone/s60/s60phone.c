/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Copyright (c) 2011 Michal Cihar <michal@cihar.com>
 */


#include "../../gsmstate.h"
#include "../../protocol/s60/s60-ids.h"
#include "../../gsmcomon.h"
#include "../../misc/coding/coding.h"
#include "../../gsmphones.h"
#include "../../gsmstate.h"
#include "../../service/gsmmisc.h"
#include "../pfunc.h"
#include <string.h>

#if defined(GSM_ENABLE_S60)
GSM_Error S60_Initialise(GSM_StateMachine *s)
{
	GSM_Phone_S60Data *Priv = &s->Phone.Data.Priv.S60;
	GSM_Error error;
	size_t i;

	Priv->ContactLocations = NULL;
	Priv->ContactLocationsSize = 0;
	Priv->ContactLocationsPos = 0;

	Priv->CalendarLocations = NULL;
	Priv->CalendarLocationsSize = 0;
	Priv->CalendarLocationsPos = 0;

	Priv->ToDoLocations = NULL;
	Priv->ToDoLocationsSize = 0;
	Priv->ToDoLocationsPos = 0;

	s->Phone.Data.SignalQuality = NULL;
	s->Phone.Data.BatteryCharge = NULL;

	for (i = 0; i < sizeof(Priv->MessageParts) / sizeof(Priv->MessageParts[0]); i++) {
		Priv->MessageParts[i] = NULL;
	}

	error = GSM_WaitFor (s, NULL, 0, 0, S60_TIMEOUT, ID_Initialise);
	if (error != ERR_NONE) {
		return error;
	}

	if (Priv->MajorVersion != 1 || Priv->MinorVersion != 5) {
		smprintf(s, "Unsupported protocol version\n");
		return ERR_NOTSUPPORTED;
	}

	error = GSM_WaitFor(s, NULL, 0, NUM_HELLO_REQUEST, S60_TIMEOUT, ID_EnableEcho);
	if (error != ERR_NONE) {
		return error;
	}

	return error;

//	return ERR_NONE;
}

/**
 * Splits values from S60 reply.
 */
GSM_Error S60_SplitValues(GSM_Protocol_Message *msg, GSM_StateMachine *s)
{
	unsigned char * pos = msg->Buffer - 1;
	size_t i;
	GSM_Phone_S60Data *Priv = &s->Phone.Data.Priv.S60;

	for (i = 0; i < sizeof(Priv->MessageParts) / sizeof(Priv->MessageParts[0]); i++) {
		Priv->MessageParts[i] = NULL;
	}

	i = 0;

	while ((pos - msg->Buffer) < (ssize_t)msg->Length) {
		if (i >  sizeof(Priv->MessageParts) / sizeof(Priv->MessageParts[0])) {
			smprintf(s, "Too many reply parts!\n");
			return ERR_MOREMEMORY;
		}
		Priv->MessageParts[i++] = pos + 1;

		/* Find end of next field */
		pos = strchr(pos + 1, NUM_SEPERATOR);
		if (pos == NULL) {
			break;
		}

		/* Zero terminate string */
		*pos = 0;
	}
	return ERR_NONE;
}

GSM_Error S60_Terminate(GSM_StateMachine *s)
{
	GSM_Phone_S60Data *Priv = &s->Phone.Data.Priv.S60;

	free(Priv->ContactLocations);
	Priv->ContactLocations = NULL;
	Priv->ContactLocationsSize = 0;
	Priv->ContactLocationsPos = 0;

	free(Priv->CalendarLocations);
	Priv->CalendarLocations = NULL;
	Priv->CalendarLocationsSize = 0;
	Priv->CalendarLocationsPos = 0;

	free(Priv->ToDoLocations);
	Priv->ToDoLocations = NULL;
	Priv->ToDoLocationsSize = 0;
	Priv->ToDoLocationsPos = 0;

	return ERR_NONE;
}

static GSM_Error S60_Reply_Generic(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Type) {
		case NUM_SYSINFO_REPLY_START:
		case NUM_CONTACTS_REPLY_HASH_SINGLE_START:
		case NUM_CONTACTS_REPLY_CONTACT_START:
		case NUM_CALENDAR_REPLY_ENTRIES_START:
			return ERR_NEEDANOTHERANSWER;
		default:
			return ERR_NONE;
	}
}


static GSM_Error S60_Reply_Connect(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_S60Data *Priv = &s->Phone.Data.Priv.S60;
	char *pos;

	Priv->MajorVersion = atoi(msg.Buffer);
	pos = strchr(msg.Buffer, '.');
	if (pos == NULL) {
		return ERR_UNKNOWN;
	}
	Priv->MinorVersion = atoi(pos + 1);
	smprintf(s, "Connected to series60-remote version %d.%d\n", Priv->MajorVersion, Priv->MinorVersion);

	return ERR_NONE;
}

static GSM_Error S60_GetInfo(GSM_StateMachine *s)
{
	return GSM_WaitFor(s, "1", 1, NUM_SYSINFO_REQUEST, S60_TIMEOUT, ID_GetModel);
}

static GSM_Error S60_GetSignalQuality(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	GSM_Error error;

	sig->BitErrorRate = -1;
	sig->SignalStrength = -1;
	sig->SignalPercent = -1;

	s->Phone.Data.SignalQuality = sig;
	error = S60_GetInfo(s);
	s->Phone.Data.SignalQuality = NULL;
	return error;
}

static GSM_Error S60_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	GSM_Error error;

	GSM_ClearBatteryCharge(bat);
	s->Phone.Data.BatteryCharge = bat;
	error = S60_GetInfo(s);
	s->Phone.Data.BatteryCharge = NULL;
	return error;
}


static GSM_Error S60_Reply_GetInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_S60Data *Priv = &s->Phone.Data.Priv.S60;
	GSM_Error error;
	char *pos;
	GSM_SignalQuality *Signal = s->Phone.Data.SignalQuality;
	GSM_BatteryCharge *BatteryCharge = s->Phone.Data.BatteryCharge;

	error = S60_SplitValues(&msg, s);
	if (error != ERR_NONE) {
		return error;
	}
	if (Priv->MessageParts[0] == NULL || Priv->MessageParts[1] == NULL) {
		return ERR_UNKNOWN;
	}
	smprintf(s, "Received %s=%s\n", Priv->MessageParts[0], Priv->MessageParts[1]);
	if (strcmp(Priv->MessageParts[0], "imei") == 0) {
		strcpy(s->Phone.Data.IMEI, Priv->MessageParts[1]);
	} else if (strcmp(Priv->MessageParts[0], "model") == 0) {
		/* Parse manufacturer */
		pos = strstr(Priv->MessageParts[1], "(C)");
		if (pos != NULL) {
			strcpy(s->Phone.Data.Manufacturer, pos + 3);
		}
		/* Try to find model */
		pos = strchr(Priv->MessageParts[1], ' ');
		if (pos != NULL) {
			pos = strchr(pos + 1, ' ');
			if (pos != NULL) {
				strcpy(s->Phone.Data.Model, pos + 1);
				pos = strchr(s->Phone.Data.Model, ' ');
				if (pos != NULL) {
					*pos = 0;
				}
			} else {
				strcpy(s->Phone.Data.Model, Priv->MessageParts[1]);
			}
		} else {
			strcpy(s->Phone.Data.Model, Priv->MessageParts[1]);
		}
		s->Phone.Data.ModelInfo = GetModelData(s, NULL, s->Phone.Data.Model, NULL);

		if (s->Phone.Data.ModelInfo->number[0] == 0)
			s->Phone.Data.ModelInfo = GetModelData(s, NULL, NULL, s->Phone.Data.Model);

		if (s->Phone.Data.ModelInfo->number[0] == 0)
			s->Phone.Data.ModelInfo = GetModelData(s, s->Phone.Data.Model, NULL, NULL);

		if (s->Phone.Data.ModelInfo->number[0] == 0) {
			smprintf(s, "Unknown model, but it should still work\n");
		}
		smprintf(s, "[Model name: `%s']\n", s->Phone.Data.Model);
		smprintf(s, "[Model data: `%s']\n", s->Phone.Data.ModelInfo->number);
		smprintf(s, "[Model data: `%s']\n", s->Phone.Data.ModelInfo->model);
	} else if (strcmp(Priv->MessageParts[0], "s60_version") == 0) {
		strcpy(s->Phone.Data.Version, Priv->MessageParts[1]);
		strcat(s->Phone.Data.Version, ".");
		strcat(s->Phone.Data.Version, Priv->MessageParts[2]);
		GSM_CreateFirmwareNumber(s);
	} else if (Signal != NULL && strcmp(Priv->MessageParts[0], "signal_dbm") == 0) {
		Signal->SignalStrength = atoi(Priv->MessageParts[1]);
	} else if (Signal != NULL && strcmp(Priv->MessageParts[0], "signal_bars") == 0) {
		Signal->SignalPercent = 100 * 7 / atoi(Priv->MessageParts[1]);
	} else if (BatteryCharge != NULL && strcmp(Priv->MessageParts[0], "battery") == 0) {
		BatteryCharge->BatteryPercent = atoi(Priv->MessageParts[1]);
	}
	return ERR_NEEDANOTHERANSWER;
}

static GSM_Error S60_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	GSM_Phone_S60Data *Priv = &s->Phone.Data.Priv.S60;
	GSM_Error error;

	if (Status->MemoryType != MEM_ME) {
		return ERR_NOTSUPPORTED;
	}

	s->Phone.Data.MemoryStatus = Status;
	Status->MemoryUsed = 0;
	Status->MemoryFree = 1000;
	Priv->ContactLocationsPos = 0;
	error = GSM_WaitFor(s, "", 0, NUM_CONTACTS_REQUEST_HASH_SINGLE, S60_TIMEOUT, ID_GetMemoryStatus);
	s->Phone.Data.MemoryStatus = NULL;
	return error;
}

static GSM_Error S60_StoreLocation(GSM_StateMachine *s, int **locations, size_t *size, size_t *pos, int location)
{
	if ((*pos) + 3 >= (*size)) {
		*locations = (int *)realloc(*locations, ((*size) + 20) * sizeof(int));
		if (*locations == NULL) {
			return ERR_MOREMEMORY;
		}
		*size += 20;
	}
	(*locations)[(*pos)] = location;
	(*locations)[(*pos) + 1] = 0;
	(*pos) += 1;
	return ERR_NONE;
}


static GSM_Error S60_Reply_ContactHash(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_S60Data *Priv = &s->Phone.Data.Priv.S60;
	GSM_Error error;

	error = S60_SplitValues(&msg, s);
	if (error != ERR_NONE) {
		return error;

	}

	if (Priv->MessageParts[0] == NULL) {
		return ERR_UNKNOWN;
	}

	error = S60_StoreLocation(s, &Priv->ContactLocations, &Priv->ContactLocationsSize, &Priv->ContactLocationsPos, atoi(Priv->MessageParts[0]));
	if (error != ERR_NONE) {
		return error;

	}

	if (s->Phone.Data.MemoryStatus != NULL) {
		s->Phone.Data.MemoryStatus->MemoryUsed++;
	}
	return ERR_NEEDANOTHERANSWER;
}

static GSM_Error S60_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *Status)
{
	GSM_Phone_S60Data *Priv = &s->Phone.Data.Priv.S60;
	GSM_Error error;

	s->Phone.Data.CalStatus = Status;
	Status->Used = 0;
	Status->Free = 1000;
	Priv->CalendarLocationsPos = 0;
	error = GSM_WaitFor(s, "", 0, NUM_CALENDAR_REQUEST_ENTRIES_ALL, S60_TIMEOUT, ID_GetCalendarNotesInfo);
	s->Phone.Data.CalStatus = NULL;
	return error;
}

static GSM_Error S60_Reply_CalendarCount(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_S60Data *Priv = &s->Phone.Data.Priv.S60;
	GSM_Error error;

	error = S60_SplitValues(&msg, s);
	if (error != ERR_NONE) {
		return error;

	}

	if (Priv->MessageParts[0] == NULL || Priv->MessageParts[1] == NULL) {
		return ERR_UNKNOWN;
	}

	if (strcmp(Priv->MessageParts[1], "appointment") != 0 &&
		strcmp(Priv->MessageParts[1], "event") != 0 &&
		strcmp(Priv->MessageParts[1], "annoversary") != 0) {
		return ERR_NEEDANOTHERANSWER;
	}

	error = S60_StoreLocation(s, &Priv->CalendarLocations, &Priv->CalendarLocationsSize, &Priv->CalendarLocationsPos, atoi(Priv->MessageParts[0]));
	if (error != ERR_NONE) {
		return error;

	}

	if (s->Phone.Data.CalStatus != NULL) {
		s->Phone.Data.CalStatus->Used++;
	}
	return ERR_NEEDANOTHERANSWER;
}

static GSM_Error S60_GetToDoStatus(GSM_StateMachine *s, GSM_ToDoStatus *Status)
{
	GSM_Phone_S60Data *Priv = &s->Phone.Data.Priv.S60;
	GSM_Error error;

	s->Phone.Data.ToDoStatus = Status;
	Status->Used = 0;
	Status->Free = 1000;
	Priv->ToDoLocationsPos = 0;
	error = GSM_WaitFor(s, "", 0, NUM_CALENDAR_REQUEST_ENTRIES_ALL, S60_TIMEOUT, ID_GetToDoInfo);
	s->Phone.Data.ToDoStatus = NULL;
	return error;
}

static GSM_Error S60_Reply_ToDoCount(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_S60Data *Priv = &s->Phone.Data.Priv.S60;
	GSM_Error error;

	error = S60_SplitValues(&msg, s);
	if (error != ERR_NONE) {
		return error;

	}

	if (Priv->MessageParts[0] == NULL || Priv->MessageParts[1] == NULL) {
		return ERR_UNKNOWN;
	}

	if (strcmp(Priv->MessageParts[1], "todo") != 0) {
		return ERR_NEEDANOTHERANSWER;
	}

	error = S60_StoreLocation(s, &Priv->ToDoLocations, &Priv->ToDoLocationsSize, &Priv->ToDoLocationsPos, atoi(Priv->MessageParts[0]));
	if (error != ERR_NONE) {
		return error;

	}

	if (s->Phone.Data.ToDoStatus != NULL) {
		s->Phone.Data.ToDoStatus->Used++;
	}
	return ERR_NEEDANOTHERANSWER;
}


GSM_Reply_Function S60ReplyFunctions[] = {

	{S60_Reply_Connect,	"", 0x00, NUM_CONNECTED, ID_Initialise },
	{S60_Reply_Generic,	"", 0x00, NUM_HELLO_REPLY, ID_EnableEcho },

	{S60_Reply_Generic,	"", 0x00, NUM_SYSINFO_REPLY_START, ID_GetModel },
	{S60_Reply_GetInfo,	"", 0x00, NUM_SYSINFO_REPLY_LINE, ID_GetModel },
	{S60_Reply_Generic,	"", 0x00, NUM_SYSINFO_REPLY_END, ID_GetModel },

	{S60_Reply_Generic,	"", 0x00, NUM_CONTACTS_REPLY_HASH_SINGLE_START, ID_GetMemoryStatus },
	{S60_Reply_ContactHash, "", 0x00, NUM_CONTACTS_REPLY_HASH_SINGLE_LINE, ID_GetMemoryStatus },
	{S60_Reply_Generic,	"", 0x00, NUM_CONTACTS_REPLY_HASH_SINGLE_END, ID_GetMemoryStatus },

	{S60_Reply_Generic,	"", 0x00, NUM_CALENDAR_REPLY_ENTRIES_START, ID_GetCalendarNotesInfo },
	{S60_Reply_CalendarCount, "", 0x00, NUM_CALENDAR_REPLY_ENTRY, ID_GetCalendarNotesInfo },
	{S60_Reply_Generic,	"", 0x00, NUM_CALENDAR_REPLY_ENTRIES_END, ID_GetCalendarNotesInfo },

	{S60_Reply_Generic,	"", 0x00, NUM_CALENDAR_REPLY_ENTRIES_START, ID_GetToDoInfo },
	{S60_Reply_ToDoCount, "", 0x00, NUM_CALENDAR_REPLY_ENTRY, ID_GetToDoInfo },
	{S60_Reply_Generic,	"", 0x00, NUM_CALENDAR_REPLY_ENTRIES_END, ID_GetToDoInfo },

	{NULL,			"", 0x00, 0x00, ID_None }
};

GSM_Phone_Functions S60Phone = {
	"s60",
	S60ReplyFunctions,
	S60_Initialise,
	S60_Terminate,
	GSM_DispatchMessage,
	NOTIMPLEMENTED,			/* 	ShowStartInfo		*/
	S60_GetInfo,                 /*      GetManufacturer */
	S60_GetInfo,                 /*      GetModel */
	S60_GetInfo,                 /*      GetFirmware */
	S60_GetInfo,                 /*      GetIMEI */
	NOTIMPLEMENTED,			/*	GetOriginalIMEI		*/
	NOTIMPLEMENTED,			/*	GetManufactureMonth	*/
	NOTIMPLEMENTED,			/*	GetProductCode		*/
	NOTIMPLEMENTED,			/*	GetHardware		*/
	NOTIMPLEMENTED,			/*	GetPPM			*/
	NOTIMPLEMENTED,			/*	GetSIMIMSI		*/
	NOTIMPLEMENTED,			/*	GetDateTime		*/
	NOTIMPLEMENTED,			/*	SetDateTime		*/
	NOTIMPLEMENTED,			/*	GetAlarm		*/
	NOTIMPLEMENTED,			/*	SetAlarm		*/
	NOTSUPPORTED,			/* 	GetLocale		*/
	NOTSUPPORTED,			/* 	SetLocale		*/
	NOTIMPLEMENTED,			/*	PressKey		*/
	NOTIMPLEMENTED,			/*	Reset			*/
	NOTIMPLEMENTED,			/*	ResetPhoneSettings	*/
	NOTIMPLEMENTED,			/*	EnterSecurityCode	*/
	NOTIMPLEMENTED,			/*	GetSecurityStatus	*/
	NOTIMPLEMENTED,			/*	GetDisplayStatus	*/
	NOTIMPLEMENTED,			/*	SetAutoNetworkLogin	*/
	S60_GetBatteryCharge,
	S60_GetSignalQuality,
	NOTIMPLEMENTED,			/*	GetNetworkInfo		*/
	NOTIMPLEMENTED,     		/*  	GetCategory 		*/
 	NOTSUPPORTED,       		/*  	AddCategory 		*/
        NOTIMPLEMENTED,      		/*  	GetCategoryStatus 	*/
	S60_GetMemoryStatus,
	NOTIMPLEMENTED,                 /*      GetMemory */
	NOTIMPLEMENTED,                 /*      GetNextMemory */
	NOTIMPLEMENTED,                 /*      SetMemory */
	NOTIMPLEMENTED,                 /*      AddMemory */
	NOTIMPLEMENTED,                 /*      DeleteMemory */
	NOTIMPLEMENTED,                 /*      DeleteAllMemory */
	NOTIMPLEMENTED,			/*	GetSpeedDial		*/
	NOTIMPLEMENTED,			/*	SetSpeedDial		*/
	NOTIMPLEMENTED,			/*	GetSMSC			*/
	NOTIMPLEMENTED,			/*	SetSMSC			*/
	NOTIMPLEMENTED,			/*	GetSMSStatus		*/
	NOTIMPLEMENTED,			/*	GetSMS			*/
	NOTIMPLEMENTED,			/*	GetNextSMS		*/
	NOTIMPLEMENTED,			/*	SetSMS			*/
	NOTIMPLEMENTED,			/*	AddSMS			*/
	NOTIMPLEMENTED,			/* 	DeleteSMS 		*/
	NOTIMPLEMENTED,			/*	SendSMSMessage		*/
	NOTSUPPORTED,			/*	SendSavedSMS		*/
	NOTSUPPORTED,			/*	SetFastSMSSending	*/
	NOTIMPLEMENTED,			/*	SetIncomingSMS		*/
	NOTIMPLEMENTED,			/* 	SetIncomingCB		*/
	NOTIMPLEMENTED,			/*	GetSMSFolders		*/
 	NOTIMPLEMENTED,			/* 	AddSMSFolder		*/
 	NOTIMPLEMENTED,			/* 	DeleteSMSFolder		*/
	NOTIMPLEMENTED,			/*	DialVoice		*/
        NOTIMPLEMENTED,			/*	DialService		*/
	NOTIMPLEMENTED,			/*	AnswerCall		*/
	NOTIMPLEMENTED,			/*	CancelCall		*/
 	NOTIMPLEMENTED,			/* 	HoldCall 		*/
 	NOTIMPLEMENTED,			/* 	UnholdCall 		*/
 	NOTIMPLEMENTED,			/* 	ConferenceCall 		*/
 	NOTIMPLEMENTED,			/* 	SplitCall		*/
 	NOTIMPLEMENTED,			/* 	TransferCall		*/
 	NOTIMPLEMENTED,			/* 	SwitchCall		*/
 	NOTIMPLEMENTED,			/* 	GetCallDivert		*/
 	NOTIMPLEMENTED,			/* 	SetCallDivert		*/
 	NOTIMPLEMENTED,			/* 	CancelAllDiverts	*/
	NOTIMPLEMENTED,			/*	SetIncomingCall		*/
	NOTIMPLEMENTED,			/*  	SetIncomingUSSD		*/
	NOTIMPLEMENTED,			/*	SendDTMF		*/
	NOTIMPLEMENTED,			/*	GetRingtone		*/
	NOTIMPLEMENTED,			/*	SetRingtone		*/
	NOTIMPLEMENTED,			/*	GetRingtonesInfo	*/
	NOTIMPLEMENTED,			/* 	DeleteUserRingtones	*/
	NOTIMPLEMENTED,			/*	PlayTone		*/
	NOTIMPLEMENTED,			/*	GetWAPBookmark		*/
	NOTIMPLEMENTED,			/* 	SetWAPBookmark 		*/
	NOTIMPLEMENTED, 		/* 	DeleteWAPBookmark 	*/
	NOTIMPLEMENTED,			/* 	GetWAPSettings 		*/
	NOTIMPLEMENTED,			/* 	SetWAPSettings 		*/
	NOTSUPPORTED,			/*	GetSyncMLSettings	*/
	NOTSUPPORTED,			/*	SetSyncMLSettings	*/
	NOTSUPPORTED,			/*	GetChatSettings		*/
	NOTSUPPORTED,			/*	SetChatSettings		*/
	NOTSUPPORTED,			/* 	GetMMSSettings		*/
	NOTSUPPORTED,			/* 	SetMMSSettings		*/
	NOTSUPPORTED,			/*	GetMMSFolders		*/
	NOTSUPPORTED,			/*	GetNextMMSFileInfo	*/
	NOTIMPLEMENTED,			/*	GetBitmap		*/
	NOTIMPLEMENTED,			/*	SetBitmap		*/
	S60_GetToDoStatus,
	NOTIMPLEMENTED,                 /*      GetTodo */
	NOTIMPLEMENTED,                 /*      GetNextTodo */
	NOTIMPLEMENTED,                 /*      SetTodo */
	NOTIMPLEMENTED,                 /*      AddTodo */
	NOTIMPLEMENTED,                 /*      DeleteTodo */
	NOTIMPLEMENTED,                 /*      DeleteAllTodo */
	S60_GetCalendarStatus,
	NOTIMPLEMENTED,                 /*      GetCalendar */
    	NOTIMPLEMENTED,                 /*      GetNextCalendar */
	NOTIMPLEMENTED,                 /*      SetCalendar */
	NOTIMPLEMENTED,                 /*      AddCalendar */
	NOTIMPLEMENTED,                 /*      DeleteCalendar */
	NOTIMPLEMENTED,                 /*      DeleteAllCalendar */
	NOTSUPPORTED,			/* 	GetCalendarSettings	*/
	NOTSUPPORTED,			/* 	SetCalendarSettings	*/
	NOTIMPLEMENTED,                 /*      GetNoteStatus */
	NOTIMPLEMENTED,                 /*      GetNote */
	NOTIMPLEMENTED,                 /*      GetNextNote */
	NOTIMPLEMENTED,                 /*      SetNote */
	NOTIMPLEMENTED,                 /*      AddNote */
	NOTIMPLEMENTED,                 /*      DeleteNote */
	NOTIMPLEMENTED,                 /*      DeleteAllNotes */
	NOTIMPLEMENTED, 		/*	GetProfile		*/
	NOTIMPLEMENTED, 		/*	SetProfile		*/
    	NOTIMPLEMENTED,			/*  	GetFMStation        	*/
    	NOTIMPLEMENTED,			/*  	SetFMStation        	*/
    	NOTIMPLEMENTED,			/*  	ClearFMStations       	*/
	NOTIMPLEMENTED,                 /*      GetNextFileFolder */
	NOTIMPLEMENTED,			/*	GetFolderListing	*/
	NOTSUPPORTED,			/*	GetNextRootFolder	*/
	NOTSUPPORTED,			/*	SetFileAttributes	*/
	NOTIMPLEMENTED,                 /*      GetFilePart */
	NOTIMPLEMENTED,                 /*      AddFilePart */
	NOTIMPLEMENTED,                 /*      SendFilePart */
	NOTIMPLEMENTED, 		/* 	GetFileSystemStatus	*/
	NOTIMPLEMENTED,                 /*      DeleteFile */
	NOTIMPLEMENTED,                 /*      AddFolder */
	NOTIMPLEMENTED,                 /*      DeleteFile */		/* 	DeleteFolder		*/
	NOTSUPPORTED,			/* 	GetGPRSAccessPoint	*/
	NOTSUPPORTED			/* 	SetGPRSAccessPoint	*/
};
#endif


/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
