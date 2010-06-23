                                           
#include "../../gsmstate.h"

#ifdef GSM_ENABLE_ALCATEL

#include <string.h>
#include <time.h>

#include "../../gsmcomon.h"
#include "../../misc/coding.h"
#include "../../service/gsmsms.h"
#include "../pfunc.h"
#include "alcatel.h"

static GSM_Error ALCATEL_SetBinaryMode(GSM_StateMachine *s)
{
	/* send something to select binary mode */
	/* if (error != GE_NONE) return error; */
	s->Protocol.Functions = &ALCABUSFunctions;
	return GE_NONE;
}

static GSM_Reply_Function ALCATELReplyFunctions[] = {
{NULL,				"\x00"			,0x00,0x00,ID_None		 }
};                                                                                      

GSM_Phone_Functions ALCATELPhone = {
	"501|701",
	NAUTOReplyFunctions,
	NONEFUNCTION,		/*	Initialise		*/
	NONEFUNCTION,		/*	Terminate 		*/
	GSM_DispatchMessage,
	NOTIMPLEMENTED,		/*	GetModel		*/
	NOTIMPLEMENTED,		/*	GetFirmware		*/
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
	NOTIMPLEMENTED,		/*	SetSMSC			*/
	NOTIMPLEMENTED		/*	SetIncomingCall		*/
};

#endif


