/* (c) 2002-2003 by Marcin Wiacek */

#include <gammu-config.h>

#ifdef GSM_ENABLE_NOKIA9210

#include <string.h>
#include <time.h>

#include "../../../misc/coding/coding.h"
#include "../../../gsmcomon.h"
#include "../../../service/gsmlogo.h"
#include "../../pfunc.h"
#include "../nfunc.h"
#include "n9210.h"
#include "dct3func.h"

static GSM_Error N9210_GetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	unsigned char OpReq[] = {N6110_FRAME_HEADER, 0x70};

	s->Phone.Data.Bitmap=Bitmap;
	switch (Bitmap->Type) {
	case GSM_OperatorLogo:
		smprintf(s, "Getting operator logo\n");
		/* This is like DCT3_GetNetworkInfo */
		return GSM_WaitFor (s, OpReq, 4, 0x0a, 4, ID_GetBitmap);
	case GSM_StartupLogo:
		smprintf(s, "Getting startup logo\n");
		return N71_92_GetPhoneSetting(s, ID_GetBitmap, 0x15);
	case GSM_WelcomeNote_Text:
		smprintf(s, "Getting welcome note\n");
		return N71_92_GetPhoneSetting(s, ID_GetBitmap, 0x02);
	default:
		break;
	}
	return ERR_NOTSUPPORTED;
}

static GSM_Error N9210_ReplySetOpLogo(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Operator logo clear/set\n");
	return ERR_NONE;
}

static GSM_Error N9210_SetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	GSM_Error		error;
	GSM_Phone_Bitmap_Types	Type;
	size_t			Width, Height, i,count=3;
	unsigned char		req[600] = { N7110_FRAME_HEADER };
	unsigned char 		reqStartup[1000] = {
		N6110_FRAME_HEADER, 0xec,
		0x15,			/* Startup Logo setting */
		0x04, 0x00, 0x00, 0x00, 0x30, 0x00,
		0x02, 0xc0, 0x54, 0x00, 0x03, 0xc0,
		0xf8, 0xf8, 0x01, 0x04};
	unsigned char 		reqStartupText[500] = {
		N7110_FRAME_HEADER, 0xec,
		0x02};			/* Startup Text setting */
	unsigned char 		reqClrOp[] = {
		N7110_FRAME_HEADER, 0xAF,
		0x02};			/* Number of logo = 0 - 0x04 */

	switch (Bitmap->Type) {
	case GSM_StartupLogo:
		if (Bitmap->Location!=1) return ERR_NOTSUPPORTED;
		Type=GSM_NokiaStartupLogo;
		PHONE_GetBitmapWidthHeight(Type, &Width, &Height);
		PHONE_EncodeBitmap(Type, reqStartup + 21, Bitmap);
		smprintf(s, "Setting startup logo\n");
		return GSM_WaitFor (s, reqStartup, 21+PHONE_GetBitmapSize(Type,0,0), 0x7A, 4, ID_SetBitmap);
	case GSM_WelcomeNote_Text:
		/* Nokia bug: Unicode text is moved one char to left */
		CopyUnicodeString(reqStartupText + 4, Bitmap->Text);
		reqStartupText[4] = 0x02;
		i = 5 + UnicodeLength(Bitmap->Text) * 2;
		reqStartupText[i++] = 0;
		reqStartupText[i++] = 0;
		return GSM_WaitFor (s, reqStartupText, i, 0x7A, 4, ID_SetBitmap);
	case GSM_OperatorLogo:
		/* First part for clearing logo */
		if (!strcmp(Bitmap->NetworkCode,"000 00")) {
			for (i=0;i<5;i++) {
				reqClrOp[4] = i;
				error=GSM_WaitFor (s, reqClrOp, 5, 0x0A, 4, ID_SetBitmap);
				if (error != ERR_NONE) return error;
			}
		}
		Type=GSM_NokiaOperatorLogo;
		req[count++] = 0xA3;
		req[count++] = 0x01;
		req[count++] = 0x00; /* Logo removed */
		NOKIA_EncodeNetworkCode(req+count, "000 00");
		count = count + 3;
		req[count++] = 0x00;
		req[count++] = 0x04;
		req[count++] = 0x08; /* Length of rest + 2 */
		memcpy(req+count, "\x00\x00\x00\x00\x00\x00", 6);
		count += 6;
		error=GSM_WaitFor (s, req, count, 0x0A, 4, ID_SetBitmap);
		if (error != ERR_NONE) return error;
		/* We wanted only clear - now exit */
		if (!strcmp(Bitmap->NetworkCode,"000 00")) return error;

		/* Now setting logo */
		count=3;
		req[count++] = 0xA3;
		req[count++] = 0x01;
		req[count++] = 0x01; /* Logo set */
		NOKIA_EncodeNetworkCode(req+count, Bitmap->NetworkCode);
		count = count + 3;
		req[count++] = 0x00;
		req[count++] = 0x04;
		req[count++] = PHONE_GetBitmapSize(Type,0,0)+8;
		PHONE_GetBitmapWidthHeight(Type, &Width, &Height);
		req[count++] = Width;
		req[count++] = Height;
		req[count++] = PHONE_GetBitmapSize(Type,0,0);
		req[count++] = 0x00;
		req[count++] = 0x00;
		req[count++] = 0x00;
		PHONE_EncodeBitmap(Type, req+count, Bitmap);
		return GSM_WaitFor (s, req, count+PHONE_GetBitmapSize(Type,0,0), 0x0A, 4, ID_SetBitmap);
	default:
		break;
	}
	return ERR_NOTSUPPORTED;
}

static GSM_Error N9210_ReplyIncomingSMS(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_SMSMessage		sms;
	GSM_Phone_Data		*Data = &s->Phone.Data;

#ifdef DEBUG
	smprintf(s, "SMS message received\n");
	sms.State 	= SMS_UnRead;
	sms.InboxFolder = true;
	DCT3_DecodeSMSFrame(s, &sms,msg.Buffer+5);
#endif
	if (Data->EnableIncomingSMS && s->User.IncomingSMS!=NULL) {
		sms.State 	= SMS_UnRead;
		sms.InboxFolder = true;
		DCT3_DecodeSMSFrame(s, &sms,msg.Buffer+5);

		s->User.IncomingSMS(s,sms, s->User.IncomingSMSUserData);
	}
	return ERR_NONE;
}

#ifdef GSM_ENABLE_N71_92INCOMINGINFO
static GSM_Error N9210_ReplySetIncomingSMS(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
	case 0x0e:
		s->Phone.Data.EnableIncomingSMS = true;
		smprintf(s, "Incoming SMS enabled\n");
		return ERR_NONE;
	case 0x0f:
		smprintf(s, "Error enabling incoming SMS\n");
		switch (msg.Buffer[4]) {
		case 0x0c:
			smprintf(s, "No PIN ?\n");
			return ERR_SECURITYERROR;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
		}
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N9210_SetIncomingSMS(GSM_StateMachine *s, bool enable)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x0d, 0x00, 0x00, 0x02};

	if (enable!=s->Phone.Data.EnableIncomingSMS) {
		if (enable) {
			smprintf(s, "Enabling incoming SMS\n");
			return GSM_WaitFor (s, req, 7, 0x02, 4, ID_SetIncomingSMS);
		} else {
			s->Phone.Data.EnableIncomingSMS = false;
			smprintf(s, "Disabling incoming SMS\n");
		}
	}
	return ERR_NONE;
}
#else
static GSM_Error N9210_SetIncomingSMS(GSM_StateMachine *s UNUSED, bool enable UNUSED)
{
	return ERR_SOURCENOTAVAILABLE;
}
#endif

static GSM_Error N9210_Initialise (GSM_StateMachine *s)
{
#ifdef DEBUG
	DCT3_SetIncomingCB(s,true);

#ifdef GSM_ENABLE_N71_92INCOMINGINFO
	N9210_SetIncomingSMS(s,true);
#endif

#endif
	return ERR_NONE;
}

GSM_Error N9210_AnswerCall(GSM_StateMachine *s, int ID, bool all)
{
	if (!all) return DCT3DCT4_AnswerCall(s,ID);
	return DCT3_AnswerAllCalls(s);
}

static GSM_Reply_Function N9210ReplyFunctions[] = {
	{DCT3_ReplySendSMSMessage,	"\x02",0x03,0x02,ID_IncomingFrame	},
	{DCT3_ReplySendSMSMessage,	"\x02",0x03,0x03,ID_IncomingFrame	},
#ifdef GSM_ENABLE_N71_92INCOMINGINFO
	{N9210_ReplySetIncomingSMS,	"\x02",0x03,0x0E,ID_SetIncomingSMS	},
	{N9210_ReplySetIncomingSMS,	"\x02",0x03,0x0F,ID_SetIncomingSMS	},
#endif
	{N9210_ReplyIncomingSMS,	"\x02",0x03,0x11,ID_IncomingFrame	},
#ifdef GSM_ENABLE_CELLBROADCAST
	{DCT3_ReplySetIncomingCB,	"\x02",0x03,0x21,ID_SetIncomingCB	},
	{DCT3_ReplySetIncomingCB,	"\x02",0x03,0x22,ID_SetIncomingCB	},
	{DCT3_ReplyIncomingCB,		"\x02",0x03,0x23,ID_IncomingFrame	},
#endif
	{DCT3_ReplySetSMSC,		"\x02",0x03,0x31,ID_SetSMSC		},
	{DCT3_ReplyGetSMSC,		"\x02",0x03,0x34,ID_GetSMSC		},
	{DCT3_ReplyGetSMSC,		"\x02",0x03,0x35,ID_GetSMSC		},

	{N61_91_ReplySetOpLogo,		"\x05",0x03,0x31,ID_SetBitmap		},
	{N61_91_ReplySetOpLogo,		"\x05",0x03,0x32,ID_SetBitmap		},

	{DCT3_ReplyGetNetworkInfo,	"\x0A",0x03,0x71,ID_GetNetworkInfo	},
	{DCT3_ReplyGetNetworkInfo,	"\x0A",0x03,0x71,ID_IncomingFrame	},
	{N71_92_ReplyGetSignalQuality,	"\x0A",0x03,0x82,ID_GetSignalQuality	},
	{N9210_ReplySetOpLogo,		"\x0A",0x03,0xA4,ID_SetBitmap		},
	{N9210_ReplySetOpLogo,		"\x0A",0x03,0xB0,ID_SetBitmap		},

	{N71_92_ReplyGetBatteryCharge,	"\x17",0x03,0x03,ID_GetBatteryCharge	},

	{DCT3_ReplySetDateTime,		"\x19",0x03,0x61,ID_SetDateTime		},
	{DCT3_ReplyGetDateTime,		"\x19",0x03,0x63,ID_GetDateTime		},

	{DCT3_ReplyEnableSecurity,	"\x40",0x02,0x64,ID_EnableSecurity	},
	{DCT3_ReplyGetIMEI,		"\x40",0x02,0x66,ID_GetIMEI		},
	{DCT3_ReplyDialCommand,		"\x40",0x02,0x7C,ID_DialVoice		},
	{DCT3_ReplyDialCommand,		"\x40",0x02,0x7C,ID_CancelCall		},
	{DCT3_ReplyDialCommand,		"\x40",0x02,0x7C,ID_AnswerCall		},
	{DCT3_ReplyNetmonitor,		"\x40",0x02,0x7E,ID_Netmonitor		},
	{NOKIA_ReplyGetPhoneString,	"\x40",0x02,0xC8,ID_GetHardware		},
	{NOKIA_ReplyGetPhoneString,	"\x40",0x02,0xC8,ID_GetPPM		},
	{NOKIA_ReplyGetPhoneString,	"\x40",0x02,0xCA,ID_GetProductCode	},
	{NOKIA_ReplyGetPhoneString,	"\x40",0x02,0xCC,ID_GetManufactureMonth	},
	{NOKIA_ReplyGetPhoneString,	"\x40",0x02,0xCC,ID_GetOriginalIMEI	},

	{N71_92_ReplyPhoneSetting,	"\x7a",0x04,0x02,ID_GetBitmap		},
	{N71_92_ReplyPhoneSetting,	"\x7a",0x04,0x02,ID_SetBitmap		},
	{N71_92_ReplyPhoneSetting,	"\x7a",0x04,0x15,ID_GetBitmap		},
	{N71_92_ReplyPhoneSetting,	"\x7a",0x04,0x15,ID_SetBitmap		},

	{DCT3DCT4_ReplyGetModelFirmware,"\xD2",0x02,0x00,ID_GetModel		},
	{DCT3DCT4_ReplyGetModelFirmware,"\xD2",0x02,0x00,ID_GetFirmware		},

	{NULL,				"\x00",0x00,0x00,ID_None		}
};

GSM_Phone_Functions N9210Phone = {
	"9210|9210i",
	N9210ReplyFunctions,
	N9210_Initialise,
	PHONE_Terminate,
	GSM_DispatchMessage,
	NOTSUPPORTED,			/* 	ShowStartInfo		*/
	NOKIA_GetManufacturer,
	DCT3DCT4_GetModel,
	DCT3DCT4_GetFirmware,
	DCT3_GetIMEI,
	DCT3_GetOriginalIMEI,
	DCT3_GetManufactureMonth,
	DCT3_GetProductCode,
	DCT3_GetHardware,
	DCT3_GetPPM,
	NOTSUPPORTED,			/*	GetSIMIMSI		*/
	N71_92_GetDateTime,
	N71_92_SetDateTime,
	NOTIMPLEMENTED,			/*	GetAlarm		*/
	NOTIMPLEMENTED,			/*	SetAlarm		*/
	NOTSUPPORTED,			/* 	GetLocale		*/
	NOTSUPPORTED,			/* 	SetLocale		*/
	NOTIMPLEMENTED,			/*	PressKey		*/
	NOTIMPLEMENTED,			/*	Reset			*/
	NOTIMPLEMENTED,			/*	ResetPhoneSettings	*/
	NOTSUPPORTED,			/*	EnterSecurityCode	*/
	NOTSUPPORTED,			/*	GetSecurityStatus	*/
	NOTSUPPORTED,			/*	GetDisplayStatus	*/
	NOTIMPLEMENTED,			/*	SetAutoNetworkLogin	*/
	N71_92_GetBatteryCharge,
	N71_92_GetSignalQuality,
	DCT3_GetNetworkInfo,
	NOTSUPPORTED,       		/*  	GetCategory 		*/
 	NOTSUPPORTED,       		/*  	AddCategory 		*/
        NOTSUPPORTED,        		/*  	GetCategoryStatus 	*/
	NOTIMPLEMENTED,			/*	GetMemoryStatus		*/
	NOTIMPLEMENTED,			/*	GetMemory		*/
	NOTIMPLEMENTED,			/*	GetNextMemory		*/
	NOTIMPLEMENTED,			/*	SetMemory		*/
	NOTIMPLEMENTED,			/*	AddMemory		*/
	NOTIMPLEMENTED,			/*	DeleteMemory		*/
	NOTIMPLEMENTED,			/*	DeleteAllMemory		*/
	NOTIMPLEMENTED,			/*	GetSpeedDial		*/
	NOTIMPLEMENTED,			/*	SetSpeedDial		*/
	DCT3_GetSMSC,
	DCT3_SetSMSC,			/* 	FIXME: test it		*/
	NOTIMPLEMENTED,			/*	GetSMSStatus		*/
	NOTIMPLEMENTED,			/*	GetSMS			*/
	NOTIMPLEMENTED,			/*	GetNextSMS		*/
	NOTIMPLEMENTED,			/*	SetSMS			*/
	NOTIMPLEMENTED,			/*	AddSMS			*/
	NOTIMPLEMENTED,			/* 	DeleteSMS 		*/
	DCT3_SendSMSMessage,
	NOTSUPPORTED,			/*	SendSavedSMS		*/
	NOTSUPPORTED,			/*	SetFastSMSSending	*/
	N9210_SetIncomingSMS,
	DCT3_SetIncomingCB,
	NOTIMPLEMENTED,			/*	GetSMSFolders		*/
 	NOTSUPPORTED,			/* 	AddSMSFolder		*/
 	NOTSUPPORTED,			/* 	DeleteSMSFolder		*/
	DCT3_DialVoice,
        NOTIMPLEMENTED,			/*	DialService		*/
	N9210_AnswerCall,
	DCT3_CancelCall,
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
        NOTIMPLEMENTED,         	/*      SetIncomingUSSD         */
	NOTSUPPORTED,			/*	SendDTMF		*/
	NOTIMPLEMENTED,			/*	GetRingtone		*/
	NOTIMPLEMENTED,			/*	SetRingtone		*/
	NOTSUPPORTED,			/*	GetRingtonesInfo	*/
	NOTSUPPORTED,			/* 	DeleteUserRingtones	*/
	NOTSUPPORTED,			/*	PlayTone		*/
	NOTIMPLEMENTED,			/*	GetWAPBookmark		*/
	NOTIMPLEMENTED,			/* 	SetWAPBookmark 		*/
	NOTIMPLEMENTED, 		/* 	DeleteWAPBookmark 	*/
	NOTIMPLEMENTED,			/* 	GetWAPSettings 		*/
	NOTSUPPORTED,			/* 	SetWAPSettings 		*/
	NOTSUPPORTED,			/*	GetSyncMLSettings	*/
	NOTSUPPORTED,			/*	SetSyncMLSettings	*/
	NOTSUPPORTED,			/*	GetChatSettings		*/
	NOTSUPPORTED,			/*	SetChatSettings		*/
	NOTSUPPORTED,			/* 	GetMMSSettings		*/
	NOTSUPPORTED,			/* 	SetMMSSettings		*/
	NOTSUPPORTED,			/*	GetMMSFolders		*/
	NOTSUPPORTED,			/*	GetNextMMSFileInfo	*/
	N9210_GetBitmap,
	N9210_SetBitmap,
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
	NOTIMPLEMENTED, 		/*	GetProfile		*/
	NOTSUPPORTED, 			/*	SetProfile		*/
    	NOTSUPPORTED,			/*  	GetFMStation        	*/
    	NOTSUPPORTED,			/*  	SetFMStation        	*/
        NOTSUPPORTED,           	/*      ClearFMStations         */
	NOTSUPPORTED,			/* 	GetNextFileFolder	*/
	NOTSUPPORTED,			/*	GetFolderListing	*/
	NOTSUPPORTED,			/*	GetNextRootFolder	*/
	NOTSUPPORTED,			/*	SetFileAttributes	*/
	NOTSUPPORTED,			/*	GetFilePart		*/
	NOTSUPPORTED,			/* 	AddFile			*/
	NOTSUPPORTED,			/* 	SendFilePart		*/
	NOTSUPPORTED, 			/* 	GetFileSystemStatus	*/
	NOTSUPPORTED,			/*	DeleteFile		*/
	NOTSUPPORTED,			/*	AddFolder		*/
	NOTSUPPORTED,			/* 	DeleteFolder		*/
	NOTSUPPORTED,			/* 	GetGPRSAccessPoint	*/
	NOTSUPPORTED			/* 	SetGPRSAccessPoint	*/
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
