
#include <string.h>
#include <time.h>

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "nfunc.h"

#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)

static GSM_Reply_Function NAUTOReplyFunctions[] = {
	{DCT3DCT4_ReplyGetModelFirmware,"\xD2",0x02,0x00,ID_GetModel		},
	{DCT3DCT4_ReplyGetModelFirmware,"\xD2",0x02,0x00,ID_GetFirmware		},

	{NULL,				"\x00",0x00,0x00,ID_None			}
};

GSM_Phone_Functions NAUTOPhone = {
	"NAUTO",
	NAUTOReplyFunctions,
	NONEFUNCTION,			/*	Initialise		*/
	NONEFUNCTION,			/*	Terminate 		*/
	GSM_DispatchMessage,
	DCT3DCT4_GetModel,
	DCT3DCT4_GetFirmware,
	NOTIMPLEMENTED,			/*	GetIMEI			*/
	NOTIMPLEMENTED,			/*	GetDateTime		*/
	NOTIMPLEMENTED,			/*	GetAlarm		*/
	NOTIMPLEMENTED,			/*	GetMemory		*/
	NOTIMPLEMENTED,			/*	GetMemoryStatus		*/
	NOTIMPLEMENTED,			/*	GetSMSC			*/
	NOTIMPLEMENTED,			/*	GetSMSMessage		*/
	NOTIMPLEMENTED,			/*	GetSMSFolders		*/
	NOKIA_GetManufacturer,
	NOTIMPLEMENTED,			/*	GetNextSMSMessage	*/
	NOTIMPLEMENTED,			/*	GetSMSStatus		*/
	NOTIMPLEMENTED,			/*	SetIncomingSMS		*/
	NOTIMPLEMENTED,			/*	GetNetworkInfo		*/
	NOTIMPLEMENTED,			/*	Reset			*/
	NOTIMPLEMENTED,			/*	DialVoice		*/
	NOTIMPLEMENTED,			/*	AnswerCall		*/
	NOTIMPLEMENTED,			/*	CancelCall		*/
	NOTIMPLEMENTED,			/*	GetRingtone		*/
	NOTIMPLEMENTED,			/*	GetWAPBookmark		*/
	NOTIMPLEMENTED,			/*	GetBitmap		*/
	NOTIMPLEMENTED,			/*	SetRingtone		*/
	NOTIMPLEMENTED,			/*	SaveSMSMessage		*/
	NOTIMPLEMENTED,			/*	SendSMSMessage		*/
	NOTIMPLEMENTED,			/*	SetDateTime		*/
	NOTIMPLEMENTED,			/*	SetAlarm		*/
	NOTIMPLEMENTED,			/*	SetBitmap		*/
	NOTIMPLEMENTED,			/* 	SetMemory 		*/
	NOTIMPLEMENTED,			/* 	DeleteSMS 		*/
	NOTIMPLEMENTED,			/* 	SetWAPBookmark 		*/
	NOTIMPLEMENTED, 		/* 	DeleteWAPBookmark 	*/
	NOTIMPLEMENTED,			/* 	GetWAPSettings 		*/
	NOTIMPLEMENTED,			/* 	SetIncomingCB		*/
	NOTIMPLEMENTED,			/*	SetSMSC			*/
	NOTIMPLEMENTED,			/*	GetManufactureMonth	*/
	NOTIMPLEMENTED,			/*	GetProductCode		*/
	NOTIMPLEMENTED,			/*	GetOriginalIMEI		*/
	NOTIMPLEMENTED,			/*	GetHardware		*/
	NOTIMPLEMENTED,			/*	GetPPM			*/
	NOTIMPLEMENTED,			/*	PressKey		*/
	NOTIMPLEMENTED,			/*	GetToDo			*/
	NOTIMPLEMENTED,			/*	DeleteAllToDo		*/
	NOTIMPLEMENTED,			/*	SetToDo			*/
	NOTIMPLEMENTED,			/*	GetToDoStatus		*/
	NOTIMPLEMENTED,			/*	PlayTone		*/
	NOTIMPLEMENTED,			/*	EnterSecurityCode	*/
	NOTIMPLEMENTED,			/*	GetSecurityStatus	*/
	NOTIMPLEMENTED, 		/*	GetProfile		*/
	NOTIMPLEMENTED,			/*	GetRingtonesInfo	*/
	NOTIMPLEMENTED,			/* 	SetWAPSettings 		*/
	NOTIMPLEMENTED,			/*	GetSpeedDial		*/
	NOTIMPLEMENTED,			/*	SetSpeedDial		*/
	NOTIMPLEMENTED,			/*	ResetPhoneSettings	*/
	NOTIMPLEMENTED,			/*	SendDTMF		*/
	NOTIMPLEMENTED,			/*	GetDisplayStatus	*/
	NOTIMPLEMENTED,			/*	SetAutoNetworkLogin	*/
	NOTIMPLEMENTED, 		/*	SetProfile		*/
	NOTIMPLEMENTED,			/*	GetSIMIMSI		*/
	NOTIMPLEMENTED,			/*	SetIncomingCall		*/
    	NOTIMPLEMENTED,			/*	GetNextCalendar		*/
	NOTIMPLEMENTED,   		/*	DelCalendar		*/
	NOTIMPLEMENTED,       		/*	AddCalendar		*/
	NOTIMPLEMENTED,			/*	GetBatteryCharge	*/
	NOTIMPLEMENTED,			/*	GetSignalQuality	*/
	NOTIMPLEMENTED,     		/*  	GetCategory 		*/
        NOTIMPLEMENTED,      		/*  	GetCategoryStatus 	*/	
    	NOTIMPLEMENTED,			/*  	GetFMStation        	*/
    	NOTIMPLEMENTED,			/*  	SetFMStation        	*/
    	NOTIMPLEMENTED,			/*  	ClearFMStations       	*/
	NOTIMPLEMENTED,			/*  	SetIncomingUSSD		*/
	NOTIMPLEMENTED,			/* 	DeleteUserRingtones	*/
	NOTIMPLEMENTED,			/* 	ShowStartInfo		*/
	NOTIMPLEMENTED,			/* 	GetNextFileFolder	*/
	NOTIMPLEMENTED,			/*	GetFilePart		*/
	NOTIMPLEMENTED,			/* 	AddFile			*/
	NOTIMPLEMENTED, 		/* 	GetFreeFileMemory 	*/
	NOTIMPLEMENTED,			/*	DeleteFile		*/
	NOTIMPLEMENTED,			/*	AddFolder		*/
	NOTIMPLEMENTED,			/* 	GetMMSSettings		*/
 	NOTIMPLEMENTED,			/* 	SetMMSSettings		*/
 	NOTIMPLEMENTED,			/* 	HoldCall 		*/
 	NOTIMPLEMENTED,			/* 	UnholdCall 		*/
 	NOTIMPLEMENTED,			/* 	ConferenceCall 		*/
 	NOTIMPLEMENTED,			/* 	SplitCall		*/
 	NOTIMPLEMENTED,			/* 	TransferCall		*/
 	NOTIMPLEMENTED,			/* 	SwitchCall		*/
 	NOTIMPLEMENTED,			/* 	GetCallDivert		*/
 	NOTIMPLEMENTED,			/* 	SetCallDivert		*/
 	NOTIMPLEMENTED,			/* 	CancelAllDiverts	*/
 	NOTIMPLEMENTED,			/* 	AddSMSFolder		*/
 	NOTIMPLEMENTED,			/* 	DeleteSMSFolder		*/
	NOTIMPLEMENTED,			/* 	GetGPRSAccessPoint	*/
	NOTIMPLEMENTED,			/* 	SetGPRSAccessPoint	*/
	NOTSUPPORTED,			/* 	GetLocale		*/
	NOTSUPPORTED,			/* 	SetLocale		*/
	NOTSUPPORTED,			/* 	GetCalendarSettings	*/
	NOTSUPPORTED,			/* 	SetCalendarSettings	*/
	NOTSUPPORTED			/*	GetNote			*/
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
