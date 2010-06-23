
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
	NONEFUNCTION,		/*	Initialise		*/
	NONEFUNCTION,		/*	Terminate 		*/
	GSM_DispatchMessage,
	DCT3DCT4_GetModel,
	DCT3DCT4_GetFirmware,
	NOTIMPLEMENTED,		/*	GetIMEI			*/
	NOTIMPLEMENTED,		/*	GetDateTime		*/
	NOTIMPLEMENTED,		/*	GetAlarm		*/
	NOTIMPLEMENTED,		/*	GetMemory		*/
	NOTIMPLEMENTED,		/*	GetMemoryStatus		*/
	NOTIMPLEMENTED,		/*	GetSMSC			*/
	NOTIMPLEMENTED,		/*	GetSMSMessage		*/
	NOTIMPLEMENTED,		/*	GetBatteryLevel		*/
	NOTIMPLEMENTED,		/*	GetNetworkLevel		*/
	NOTIMPLEMENTED,		/*	GetSMSFolders		*/
	NOKIA_GetManufacturer,
	NOTIMPLEMENTED,		/*	GetNextSMSMessage	*/
	NOTIMPLEMENTED,		/*	GetSMSStatus		*/
	NOTIMPLEMENTED,		/*	SetIncomingSMS		*/
	NOTIMPLEMENTED,		/*	GetNetworkInfo		*/
	NOTIMPLEMENTED,		/*	Reset			*/
	NOTIMPLEMENTED,		/*	DialVoice		*/
	NOTIMPLEMENTED,		/*	AnswerCall		*/
	NOTIMPLEMENTED,		/*	CancelCall		*/
	NOTIMPLEMENTED,		/*	GetSecurityCode		*/
	NOTIMPLEMENTED,		/*	GetRingtone		*/
	NOTIMPLEMENTED,		/*	GetCalendarNote		*/
	NOTIMPLEMENTED,		/*	GetWAPBookmark		*/
	NOTIMPLEMENTED,		/*	GetBitmap		*/
	NOTIMPLEMENTED,		/*	SetRingtone		*/
	NOTIMPLEMENTED,		/*	SaveSMSMessage		*/
	NOTIMPLEMENTED,		/*	SendSMSMessage		*/
	NOTIMPLEMENTED,		/*	SetDateTime		*/
	NOTIMPLEMENTED,		/*	SetAlarm		*/
	NOTIMPLEMENTED,		/*	SetBitmap		*/
	NOTIMPLEMENTED,		/* 	SetMemory 		*/
	NOTIMPLEMENTED,		/* 	DeleteSMS 		*/
	NOTIMPLEMENTED,		/* 	DeleteCalendarNote 	*/
	NOTIMPLEMENTED, 	/* 	SetCalendarNote 	*/
	NOTIMPLEMENTED,		/* 	SetWAPBookmark 		*/
	NOTIMPLEMENTED, 	/* 	DeleteWAPBookmark 	*/
	NOTIMPLEMENTED,		/* 	GetWAPSettings 		*/
	NOTIMPLEMENTED, 	/* 	GetProfile 		*/
	NOTIMPLEMENTED,		/* 	SetIncomingCB		*/
	NOTIMPLEMENTED		/*	SetSMSC			*/
};

#endif
