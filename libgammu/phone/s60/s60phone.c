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
#include <string.h>

#if defined(GSM_ENABLE_S60)
GSM_Error S60_Initialise(GSM_StateMachine *s)
{
	GSM_Phone_S60Data *Priv = &s->Phone.Data.Priv.S60;
	GSM_Error error;

	Priv->foo = 0;

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

GSM_Error S60_Terminate(GSM_StateMachine *s)
{
	GSM_Phone_S60Data *Priv = &s->Phone.Data.Priv.S60;

	Priv->foo = 0;

	return ERR_NONE;
}

static GSM_Error S60_Reply_Generic(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Type) {
		case NUM_SYSINFO_REPLY_START:
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

static GSM_Error S60_Reply_GetInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return ERR_NEEDANOTHERANSWER;
}


GSM_Reply_Function S60ReplyFunctions[] = {

	{S60_Reply_Connect,	"", 0x00, NUM_CONNECTED, ID_Initialise },
	{S60_Reply_Generic,	"", 0x00, NUM_HELLO_REPLY, ID_EnableEcho },
	{S60_Reply_Generic,	"", 0x00, NUM_SYSINFO_REPLY_START, ID_GetModel },
	{S60_Reply_GetInfo,	"", 0x00, NUM_SYSINFO_REPLY_LINE, ID_GetModel },
	{S60_Reply_Generic,	"", 0x00, NUM_SYSINFO_REPLY_END, ID_GetModel },
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
	NOTIMPLEMENTED,			/*	GetBatteryCharge	*/
	NOTIMPLEMENTED,			/*	GetSignalQuality	*/
	NOTIMPLEMENTED,			/*	GetNetworkInfo		*/
	NOTIMPLEMENTED,     		/*  	GetCategory 		*/
 	NOTSUPPORTED,       		/*  	AddCategory 		*/
        NOTIMPLEMENTED,      		/*  	GetCategoryStatus 	*/
	NOTIMPLEMENTED,                 /*      GetMemoryStatus */
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
	NOTIMPLEMENTED,                 /*      GetTodoStatus */
	NOTIMPLEMENTED,                 /*      GetTodo */
	NOTIMPLEMENTED,                 /*      GetNextTodo */
	NOTIMPLEMENTED,                 /*      SetTodo */
	NOTIMPLEMENTED,                 /*      AddTodo */
	NOTIMPLEMENTED,                 /*      DeleteTodo */
	NOTIMPLEMENTED,                 /*      DeleteAllTodo */
	NOTIMPLEMENTED,                 /*      GetCalendarStatus */
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
