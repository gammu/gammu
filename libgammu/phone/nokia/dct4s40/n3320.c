/* (c) 2004 by Marcin Wiacek */

#include <gammu-config.h>

#ifdef GSM_ENABLE_NOKIA3320

#include <string.h>
#include <time.h>

#include "../../../gsmcomon.h"
#include "../../../misc/coding/coding.h"
#include "../../../service/gsmlogo.h"
#include "../nfunc.h"
#include "../nfuncold.h"
#include "../../pfunc.h"
#include "n3320.h"

static GSM_Error N3320_ReplyGetMemory(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Phonebook entry received\n");
	if (msg.Buffer[6] == 0x0f)
		return N71_65_ReplyGetMemoryError(msg.Buffer[10], s);

	return N71_65_DecodePhonebook(s, s->Phone.Data.Memory, s->Phone.Data.Bitmap, s->Phone.Data.SpeedDial, msg.Buffer+22, msg.Length-22,TRUE);
}

static GSM_Error N3320_GetMemory (GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x07, 0x01, 0x01, 0x00, 0x01,
			       0xfe, 0x10, 	/* memory type */
			       0x00, 0x00, 0x00, 0x00,
			       0x00, 0x01, 	/* location */
			       0x00, 0x00, 0x01};

	req[9] = NOKIA_GetMemoryType(s, entry->MemoryType,N71_65_MEMORY_TYPES);
	if (entry->MemoryType == MEM_SM) return ERR_NOTSUPPORTED;
	if (req[9]==0xff) return ERR_NOTSUPPORTED;

	if (entry->Location==0x00) return ERR_INVALIDLOCATION;

	req[14] = entry->Location / 256;
	req[15] = entry->Location % 256;

	s->Phone.Data.Memory=entry;
	smprintf(s, "Getting phonebook entry\n");
	return GSM_WaitFor (s, req, 19, 0x03, 4, ID_GetMemory);
}

static GSM_Error N3320_ReplyGetMemoryStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Memory status received\n");
	/* Quess ;-)) */
	if (msg.Buffer[14]==0x10) {
		Data->MemoryStatus->MemoryFree = msg.Buffer[18]*256 + msg.Buffer[19];
	} else {
		Data->MemoryStatus->MemoryFree = msg.Buffer[17];
	}
	smprintf(s, "Size       : %i\n",Data->MemoryStatus->MemoryFree);
	Data->MemoryStatus->MemoryUsed = msg.Buffer[20]*256 + msg.Buffer[21];
	smprintf(s, "Used       : %i\n",Data->MemoryStatus->MemoryUsed);
	Data->MemoryStatus->MemoryFree -= Data->MemoryStatus->MemoryUsed;
	smprintf(s, "Free       : %i\n",Data->MemoryStatus->MemoryFree);
	return ERR_NONE;
}

static GSM_Error N3320_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x03, 0x02,
			       0x00,		/* memory type */
			       0x55, 0x55, 0x55, 0x00};

	req[5] = NOKIA_GetMemoryType(s, Status->MemoryType,N71_65_MEMORY_TYPES);
	if (Status->MemoryType == MEM_SM) return ERR_NOTSUPPORTED;
	if (req[5]==0xff) return ERR_NOTSUPPORTED;

	s->Phone.Data.MemoryStatus=Status;
	smprintf(s, "Getting memory status\n");
	return GSM_WaitFor (s, req, 10, 0x03, 4, ID_GetMemoryStatus);
}

static GSM_Error N3320_ReplyGetDateTime(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Date & time received\n");
	if (msg.Buffer[4]==0x01) {
		NOKIA_DecodeDateTime(s, msg.Buffer+10, s->Phone.Data.DateTime, TRUE, FALSE);
		return ERR_NONE;
	}
	smprintf(s, "Not set in phone\n");
	return ERR_EMPTY;
}

static GSM_Error N3320_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x0A, 0x00, 0x00};

	s->Phone.Data.DateTime=date_time;
	smprintf(s, "Getting date & time\n");
	return GSM_WaitFor (s, req, 6, 0x19, 4, ID_GetDateTime);
}

static GSM_Error N3320_GetNextCalendar(GSM_StateMachine *s,  GSM_CalendarEntry *Note, gboolean start)
{
	return N71_65_GetNextCalendar1(s,Note,start,&s->Phone.Data.Priv.N3320.LastCalendar,&s->Phone.Data.Priv.N3320.LastCalendarYear,&s->Phone.Data.Priv.N3320.LastCalendarPos);
}

static GSM_Error N3320_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *Status)
{
	GSM_Error error;

        /* Method 1 */
	error=N71_65_GetCalendarInfo1(s, &s->Phone.Data.Priv.N3320.LastCalendar);
	if (error!=ERR_NONE) return error;

	/**
	 * @todo This should be acquired from phone
	 */
	Status->Free = 100;
	Status->Used = s->Phone.Data.Priv.N3320.LastCalendar.Number;
	return ERR_NONE;
}

static GSM_Error N3320_ReplyGetCalendarInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return N71_65_ReplyGetCalendarInfo1(msg, s, &s->Phone.Data.Priv.N3320.LastCalendar);
}

static GSM_Reply_Function N3320ReplyFunctions[] = {
	{N3320_ReplyGetMemoryStatus,	  "\x03",0x03,0x04,ID_GetMemoryStatus	  },
	{N3320_ReplyGetMemory,		  "\x03",0x03,0x08,ID_GetMemory		  },

	{N71_65_ReplyGetNextCalendar1,	  "\x13",0x03,0x1A,ID_GetCalendarNote	  },/*method 1*/
	{N3320_ReplyGetCalendarInfo,	  "\x13",0x03,0x3B,ID_GetCalendarNotesInfo},/*method 1*/

	{N3320_ReplyGetDateTime,	  "\x19",0x03,0x0B,ID_GetDateTime	  },

	{DCT3DCT4_ReplyGetModelFirmware,  "\xD2",0x02,0x00,ID_GetModel		  },
	{DCT3DCT4_ReplyGetModelFirmware,  "\xD2",0x02,0x00,ID_GetFirmware	  },

	{NULL,				  "\x00",0x00,0x00,ID_None		  }
};

GSM_Phone_Functions N3320Phone = {
	"3320",
	N3320ReplyFunctions,
	NONEFUNCTION,			/*	Initialise		*/
	NONEFUNCTION,			/*	Terminate		*/
	GSM_DispatchMessage,
	NOTSUPPORTED,			/* 	ShowStartInfo		*/
	NOKIA_GetManufacturer,
	DCT3DCT4_GetModel,
	DCT3DCT4_GetFirmware,
	NOTSUPPORTED,			/*	GetIMEI			*/
	NOTSUPPORTED,			/*	GetOriginalIMEI		*/
	NOTSUPPORTED,			/*	GetManufactureMonth	*/
	NOTSUPPORTED,			/*	GetProductCode		*/
	NOTSUPPORTED,			/*	GetHardware		*/
	NOTSUPPORTED,			/*	GetPPM			*/
	NOTSUPPORTED,			/*	GetSIMIMSI		*/
	N3320_GetDateTime,
	NOTSUPPORTED,			/*	SetDateTime		*/
	NOTSUPPORTED,			/*	GetAlarm		*/
	NOTSUPPORTED,			/*	SetAlarm		*/
	NOTSUPPORTED,			/* 	GetLocale		*/
	NOTSUPPORTED,			/* 	SetLocale		*/
	NOTSUPPORTED,			/*	PressKey		*/
	NOTSUPPORTED,			/*	Reset			*/
	NOTSUPPORTED,			/*	ResetPhoneSettings	*/
	NOTSUPPORTED,			/*	EnterSecurityCode	*/
	NOTSUPPORTED,			/*	GetSecurityStatus	*/
	NOTSUPPORTED,			/*	GetDisplayStatus	*/
	NOTSUPPORTED,			/*	SetAutoNetworkLogin	*/
	NOTSUPPORTED,			/*	GetBatteryCharge	*/
	NOTSUPPORTED,			/*	GetSignalQuality	*/
	NOTSUPPORTED,			/*	GetNetworkInfo		*/
	NOTSUPPORTED,     		/*  	GetCategory 		*/
 	NOTSUPPORTED,       		/*  	AddCategory 		*/
        NOTSUPPORTED,      		/*  	GetCategoryStatus 	*/
	N3320_GetMemoryStatus,
	N3320_GetMemory,
	NOTSUPPORTED,			/*	GetNextMemory		*/
	NOTSUPPORTED,			/*	SetMemory		*/
	NOTSUPPORTED,			/*	AddMemory		*/
	NOTSUPPORTED,			/*	DeleteMemory		*/
	NOTIMPLEMENTED,			/*	DeleteAllMemory		*/
	NOTSUPPORTED,			/*	GetSpeedDial		*/
	NOTSUPPORTED,			/*	SetSpeedDial		*/
	NOTSUPPORTED,			/*	GetSMSC			*/
	NOTSUPPORTED,			/*	SetSMSC			*/
	NOTSUPPORTED,			/*	GetSMSStatus		*/
	NOTSUPPORTED,			/*	GetSMS			*/
	NOTSUPPORTED,			/*	GetNextSMS		*/
	NOTSUPPORTED,			/*	SetSMS			*/
	NOTSUPPORTED,			/*	AddSMS			*/
	NOTSUPPORTED,			/* 	DeleteSMS 		*/
	NOTSUPPORTED,			/*	SendSMS			*/
	NOTSUPPORTED,			/*	SendSavedSMS		*/
	NOTSUPPORTED,			/*	SetFastSMSSending	*/
	NOTSUPPORTED,			/*	SetIncomingSMS		*/
	NOTSUPPORTED,			/* 	SetIncomingCB		*/
	NOTSUPPORTED,			/*	GetSMSFolders		*/
 	NOTSUPPORTED,			/* 	AddSMSFolder		*/
 	NOTSUPPORTED,			/* 	DeleteSMSFolder		*/
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
 	NOTSUPPORTED,			/* 	GetCallDivert		*/
 	NOTSUPPORTED,			/* 	SetCallDivert		*/
 	NOTSUPPORTED,			/* 	CancelAllDiverts	*/
	NOTIMPLEMENTED,			/*	SetIncomingCall		*/
	NOTIMPLEMENTED,			/*  	SetIncomingUSSD		*/
	NOTSUPPORTED,			/*	SendDTMF		*/
	NOTSUPPORTED,			/*	GetRingtone		*/
	NOTSUPPORTED,			/*	SetRingtone		*/
	NOTSUPPORTED,			/*	GetRingtonesInfo	*/
	NOTIMPLEMENTED,			/* 	DeleteUserRingtones	*/
	NOTSUPPORTED,			/*	PlayTone		*/
	NOTSUPPORTED,			/*	GetWAPBookmark		*/
	NOTSUPPORTED,			/* 	SetWAPBookmark 		*/
	NOTSUPPORTED, 			/* 	DeleteWAPBookmark 	*/
	NOTSUPPORTED,			/* 	GetWAPSettings 		*/
	NOTSUPPORTED,			/* 	SetWAPSettings 		*/
	NOTSUPPORTED,			/*	GetSyncMLSettings	*/
	NOTSUPPORTED,			/*	SetSyncMLSettings	*/
	NOTSUPPORTED,			/*	GetChatSettings		*/
	NOTSUPPORTED,			/*	SetChatSettings		*/
	NOTSUPPORTED,			/* 	GetMMSSettings		*/
	NOTSUPPORTED,			/* 	SetMMSSettings		*/
	NOTSUPPORTED,			/*	GetMMSFolders		*/
	NOTSUPPORTED,			/*	GetNextMMSFile		*/
	NOTSUPPORTED,			/*	GetBitmap		*/
	NOTSUPPORTED,			/*	SetBitmap		*/
	NOTSUPPORTED,			/*	GetToDoStatus		*/
	NOTSUPPORTED,			/*	GetToDo			*/
	NOTSUPPORTED,			/*	GetNextToDo		*/
	NOTSUPPORTED,			/*	SetToDo			*/
	NOTSUPPORTED,			/*	AddToDo			*/
	NOTSUPPORTED,			/*	DeleteToDo		*/
	NOTSUPPORTED,			/*	DeleteAllToDo		*/
	N3320_GetCalendarStatus,
	NOTIMPLEMENTED,			/*	GetCalendar		*/
    	N3320_GetNextCalendar,
	NOTIMPLEMENTED,			/*	SetCalendar		*/
	NOTSUPPORTED,			/*	AddCalendar		*/
	NOTSUPPORTED,			/*	DeleteCalendar		*/
	NOTIMPLEMENTED,			/*	DeleteAllCalendar	*/
	NOTSUPPORTED,			/* 	GetCalendarSettings	*/
	NOTSUPPORTED,			/* 	SetCalendarSettings	*/
	NOTSUPPORTED,			/*	GetNoteStatus		*/
	NOTSUPPORTED,			/*	GetNote			*/
	NOTSUPPORTED,			/*	GetNextNote		*/
	NOTSUPPORTED,			/*	SetNote			*/
	NOTSUPPORTED,			/*	AddNote			*/
	NOTSUPPORTED,			/* 	DeleteNote		*/
	NOTSUPPORTED,			/*	DeleteAllNotes		*/
	NOTSUPPORTED, 			/*	GetProfile		*/
	NOTSUPPORTED, 			/*	SetProfile		*/
    	NOTSUPPORTED,			/*  	GetFMStation        	*/
    	NOTSUPPORTED,			/*  	SetFMStation        	*/
    	NOTSUPPORTED,			/*  	ClearFMStations       	*/
	NOTSUPPORTED,			/*	GetNextFileFolder	*/
	NOTSUPPORTED,			/*	GetFolderListing	*/
	NOTSUPPORTED,			/*	GetNextRootFolder	*/
	NOTSUPPORTED,			/*	SetFileAttributes	*/
	NOTSUPPORTED,			/*	GetFilePart		*/
	NOTIMPLEMENTED,			/*	AddFilePart		*/
	NOTSUPPORTED,			/* 	SendFilePart		*/
	NOTSUPPORTED,	 		/* 	GetFileSystemStatus	*/
	NOTIMPLEMENTED,			/*	DeleteFile		*/
	NOTIMPLEMENTED,			/*	AddFolder		*/
	NOTSUPPORTED,			/* 	DeleteFolder		*/
	NOTSUPPORTED,			/* 	GetGPRSAccessPoint	*/
	NOTSUPPORTED			/* 	SetGPRSAccessPoint	*/
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
