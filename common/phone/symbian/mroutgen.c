/* (c) 2003 by Marcin Wiacek */
/* EXPERIMENTAL. NOT FUNCTIONAL */

#include <string.h>
#include <time.h>

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "../../misc/coding/coding.h"
#include "../../service/gsmmisc.h"

#ifdef GSM_ENABLE_MROUTERGEN

GSM_Error MROUTERGEN_StartModem(GSM_StateMachine *s)
{
	return ERR_NONE;
}

GSM_Error MROUTERGEN_Initialise(GSM_StateMachine *s)
{

unsigned char req2[]={0xFF,0x03,0x80,0xFD,0x04,0x01,0x00,0x09,0x11,
0x05,0x00,0x01,0x03,0x77,0xBC};

unsigned char req0[]={
0xFF,0x7D,0x23,0xC0,0x21,0x7D,0x21,0x7D,0x24,0x7D,
0x20,0x7D,0x2E,0x7D,0x22,0x7D,0x26,0x7D,0x20,0x7D,
0x20,0x7D,0x20,0x7D,0x20,0x7D,0x27,0x7D,0x22,0x7D,
0x28,0x7D,0x22,0x69,0x27,
0x7E,

0x7E,
0xFF,0x7D,0x23,0xC0,0x21,0x7D,0x22,0x7D,0x21,0x7D,
0x20,0x7D,0x34,0x7D,0x25,0x7D,0x26,0x87,0xB9,0x7D,
0x28,0xCF,0x7D,0x22,0x7D,0x26,0x7D,0x20,0x7D,0x20,
0x7D,0x20,0x7D,0x20,0x7D,0x27,0x7D,0x22,0x7D,0x28,
0x7D,0x22,0x82,0xB0,
0x7E,

0x7E,
0xFF,0x7D,0x23,0xC0,0x21,0x7D,0x21,0x7D,0x24,0x7D,
0x20,0x7D,0x2E,0x7D,0x22,0x7D,0x26,0x7D,0x20,0x7D,
0x20,0x7D,0x20,0x7D,0x20,0x7D,0x27,0x7D,0x22,0x7D,
0x28,0x7D,0x22,0x69,0x27,
0x7E,

0x7E,
0xFF,0x7D,0x23,0xC0,0x21,0x7D,0x22,0x7D,0x21,0x7D,
0x20,0x7D,0x34,0x7D,0x25,0x7D,0x26,0x87,0xB9,0x7D,
0x28,0xCF,0x7D,0x22,0x7D,0x26,0x7D,0x20,0x7D,0x20,
0x7D,0x20,0x7D,0x20,0x7D,0x27,0x7D,0x22,0x7D,0x28,
0x7D,0x22,0x82,0xB0};

unsigned char req[]={0x2F,0x45,0x00,0x00,0x34,0x00,0xC6,
			0x00,0x00,0x0A,0x0D,0x59,0xB4,0xA9,0xFE,0x01,
			0x44,0xA9,0xFE,0x01,0x0A,0x04,0x09,0x0B,0xB8,
			0x10,0x00,0x02,0x79,0x69,0x81,0x74,0x99,0x50,
			0x18,0x60,0x00,0x9D,0x0C,0x00,0x00,0x08,0x00,
			0x00,0x00,0x12,0x00,0x41,0x00,0x01,0x14,0x00,
			0x00,0xBA,0x4A};

unsigned char req3[]={0x2D,0x5C,0x0D,0xEC,0x4C,0x10,0x26,0x08,0x00,
0x00,0x00,0x12,0x00,0x15,0x00,0x01,0x08,0x00,0x00,
0xAD,0xDE};
smprintf(s,"writing\n");
	GSM_WaitFor (s, req0, 164, 0x00, 200, ID_Initialise);
smprintf(s,"writing\n");
	GSM_WaitFor (s, req3, 21, 0x00, 200, ID_Initialise);
	GSM_WaitFor (s, req, 55, 0x00, 200, ID_Initialise);

	GSM_WaitFor (s, req2, 15, 0x00, 200, ID_Initialise);


	while (1) {
		GSM_ReadDevice(s,false);
	}



	while (1) {
		GSM_ReadDevice(s,false);
	}


return ERR_UNKNOWN;
}

static GSM_Reply_Function MROUTERGENReplyFunctions[] = {
	{NULL,				"\x00",0x00,0x00,ID_None			}
};

GSM_Phone_Functions MROUTERGENPhone = {
	"mrouter",
	MROUTERGENReplyFunctions,
	MROUTERGEN_Initialise,
	NONEFUNCTION,			/*	Terminate 		*/
	GSM_DispatchMessage,
	NOTSUPPORTED,			/* 	ShowStartInfo		*/
	NONEFUNCTION,			/*	GetManufacturer		*/
	NONEFUNCTION,			/*	GetModel		*/
	NONEFUNCTION,			/*	GetFirmware		*/
	NOTIMPLEMENTED,			/*	GetIMEI			*/
	NOTSUPPORTED,			/*	GetOriginalIMEI		*/
	NOTSUPPORTED,			/*	GetManufactureMonth	*/
	NOTSUPPORTED,			/*	GetProductCode		*/
	NOTSUPPORTED,			/*	GetHardware		*/
	NOTSUPPORTED,			/*	GetPPM			*/
	NOTSUPPORTED,			/*	GetSIMIMSI		*/
	NOTSUPPORTED,			/*	GetDateTime		*/
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
        NOTSUPPORTED,      		/*  	GetCategoryStatus 	*/	
	NOTSUPPORTED,			/*	GetMemoryStatus		*/
	NOTSUPPORTED,			/*	GetMemory		*/
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
	NOTSUPPORTED,			/*	SendSMSMessage		*/
	NOTSUPPORTED,			/*	SendSavedSMS		*/
	NOTSUPPORTED,			/*	SetFastSMSSending	*/
	NOTSUPPORTED,			/*	SetIncomingSMS		*/
	NOTSUPPORTED,			/* 	SetIncomingCB		*/
	NOTSUPPORTED,			/*	GetSMSFolders		*/
 	NOTSUPPORTED,			/* 	AddSMSFolder		*/
 	NOTSUPPORTED,			/* 	DeleteSMSFolder		*/
	NOTSUPPORTED,			/*	DialVoice		*/
	NOTSUPPORTED,			/*	AnswerCall		*/
	NOTSUPPORTED,			/*	CancelCall		*/
 	NOTSUPPORTED,			/* 	HoldCall 		*/
 	NOTSUPPORTED,			/* 	UnholdCall 		*/
 	NOTSUPPORTED,			/* 	ConferenceCall 		*/
 	NOTSUPPORTED,			/* 	SplitCall		*/
 	NOTSUPPORTED,			/* 	TransferCall		*/
 	NOTSUPPORTED,			/* 	SwitchCall		*/
 	NOTSUPPORTED,			/* 	GetCallDivert		*/
 	NOTSUPPORTED,			/* 	SetCallDivert		*/
 	NOTSUPPORTED,			/* 	CancelAllDiverts	*/
	NOTSUPPORTED,			/*	SetIncomingCall		*/
	NOTSUPPORTED,			/*  	SetIncomingUSSD		*/
	NOTSUPPORTED,			/*	SendDTMF		*/
	NOTSUPPORTED,			/*	GetRingtone		*/
	NOTSUPPORTED,			/*	SetRingtone		*/
	NOTSUPPORTED,			/*	GetRingtonesInfo	*/
	NOTSUPPORTED,			/* 	DeleteUserRingtones	*/
	NOTSUPPORTED,			/*	PlayTone		*/
	NOTSUPPORTED,			/*	GetWAPBookmark		*/
	NOTSUPPORTED,			/* 	SetWAPBookmark 		*/
	NOTSUPPORTED, 			/* 	DeleteWAPBookmark 	*/
	NOTSUPPORTED,			/* 	GetWAPSettings 		*/
	NOTSUPPORTED,			/* 	SetWAPSettings 		*/
	NOTSUPPORTED,			/* 	GetMMSSettings		*/
 	NOTSUPPORTED,			/* 	SetMMSSettings		*/
	NOTSUPPORTED,			/*	GetSyncMLSettings	*/
	NOTSUPPORTED,			/*	SetSyncMLSettings	*/
	NOTSUPPORTED,			/*	GetChatSettings		*/
	NOTSUPPORTED,			/*	SetChatSettings		*/
	NOTSUPPORTED,			/*	GetBitmap		*/
	NOTSUPPORTED,			/*	SetBitmap		*/
	NOTSUPPORTED,			/*	GetToDoStatus		*/
	NOTSUPPORTED,			/*	GetToDo			*/
	NOTSUPPORTED,			/*	GetNextToDo		*/
	NOTSUPPORTED,			/*	SetToDo			*/
	NOTSUPPORTED,			/*	AddToDo			*/
	NOTSUPPORTED,			/*	DeleteToDo		*/
	NOTSUPPORTED,			/*	DeleteAllToDo		*/
	NOTSUPPORTED,			/*	GetCalendarStatus	*/
	NOTSUPPORTED,			/*	GetCalendar		*/
    	NOTSUPPORTED,			/*  	GetNextCalendar		*/
	NOTSUPPORTED,			/*	SetCalendar		*/
	NOTSUPPORTED,			/*	AddCalendar		*/
	NOTSUPPORTED,			/*	DeleteCalendar		*/
	NOTSUPPORTED,			/*	DeleteAllCalendar	*/
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
	NOTSUPPORTED,			/* 	ClearFMStations		*/
	NOTSUPPORTED,			/* 	GetNextFileFolder	*/
	NOTSUPPORTED,			/* 	GetFilePart		*/
	NOTSUPPORTED,			/* 	AddFilePart		*/
	NOTSUPPORTED, 			/* 	GetFileSystemStatus	*/
	NOTSUPPORTED,			/* 	DeleteFile		*/
	NOTSUPPORTED,			/* 	AddFolder		*/
	NOTSUPPORTED,			/* 	GetGPRSAccessPoint	*/
	NOTSUPPORTED			/* 	SetGPRSAccessPoint	*/
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
