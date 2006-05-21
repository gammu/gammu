/* (c) 2006 by Marcin Wiacek */

#include <string.h>
#include <time.h>

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "../../misc/coding/coding.h"
#include "../../service/gsmmisc.h"
#include "../nokia/nfunc.h"
#include "../pfunc.h"

#ifdef GSM_ENABLE_GNAPGEN

static GSM_Error GNAPGEN_ReplyGetSMSFolders(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 			j, pos;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	smprintf(s, "SMS folders names received\n");
	Data->SMSFolders->Number = msg.Buffer[5];

	pos = 6;
	for (j=0;j<msg.Buffer[5];j++) {
		if (msg.Buffer[pos+3]>GSM_MAX_SMS_FOLDER_NAME_LEN) {
			smprintf(s, "Too long text\n");
			return ERR_UNKNOWNRESPONSE;
		}
		memcpy(Data->SMSFolders->Folder[j].Name,msg.Buffer + pos+4,msg.Buffer[pos+3]*2);
		Data->SMSFolders->Folder[j].Name[msg.Buffer[pos+3]*2]=0;
		Data->SMSFolders->Folder[j].Name[msg.Buffer[pos+3]*2+1]=0;
		smprintf(s, ", folder name: \"%s\"\n",DecodeUnicodeString(Data->SMSFolders->Folder[j].Name));
		Data->SMSFolders->Folder[j].InboxFolder = false;
		if (j==0) Data->SMSFolders->Folder[j].InboxFolder = true;
		Data->SMSFolders->Folder[j].Memory 	  = MEM_ME;
		pos+=msg.Buffer[pos+3]*2+4;
	}
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	unsigned char req[] = {0x00,0x01};

	s->Phone.Data.SMSFolders=folders;
	smprintf(s, "Getting SMS folders\n");
	return GSM_WaitFor (s, req, 2, 0x06, 4, ID_GetSMSFolders);
}

static GSM_Error GNAPGEN_ReplyGetSMSC(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 			pos=7;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	if (msg.Buffer[7]*2>GSM_MAX_SMSC_NAME_LENGTH) {
		smprintf(s, "Too long name\n");
		return ERR_UNKNOWNRESPONSE;
	}
	memcpy(Data->SMSC->Name,msg.Buffer+8,msg.Buffer[7]*2);
	Data->SMSC->Name[msg.Buffer[7]*2] = 0;
	Data->SMSC->Name[msg.Buffer[7]*2+1] = 0;
	smprintf(s, "   Name \"%s\"\n", DecodeUnicodeString(Data->SMSC->Name));
	pos+=msg.Buffer[7]*2;

	pos+=4;

	Data->SMSC->Format 		= SMS_FORMAT_Text;
	Data->SMSC->Validity.Format	= SMS_Validity_RelativeFormat;
	Data->SMSC->Validity.Relative 	= SMS_VALID_Max_Time;

	Data->SMSC->DefaultNumber[0] = 0;
	Data->SMSC->DefaultNumber[1] = 0;

	memcpy(Data->SMSC->Number,msg.Buffer+pos+4,msg.Buffer[pos+3]*2);
	Data->SMSC->Number[msg.Buffer[pos+3]*2] = 0;
	Data->SMSC->Number[msg.Buffer[pos+3]*2+1] = 0;
	smprintf(s, "   Number \"%s\"\n", DecodeUnicodeString(Data->SMSC->Number));

	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	unsigned char req[] = {0x00,21,
			       0x00,0x01};		/* location */

	if (smsc->Location==0x00) return ERR_INVALIDLOCATION;

	req[3]=smsc->Location-1;

	s->Phone.Data.SMSC=smsc;
	smprintf(s, "Getting SMSC\n");
	return GSM_WaitFor (s, req, 4, 0x06, 4, ID_GetSMSC);
}

static GSM_Error GNAPGEN_ReplyGetDateTime(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Date & time received\n");
	NOKIA_DecodeDateTime(s, msg.Buffer+4, s->Phone.Data.DateTime);
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	unsigned char req[2] = {0x00,0x01};

	s->Phone.Data.DateTime=date_time;
	smprintf(s, "Getting date & time\n");
	return GSM_WaitFor (s, req, 2, 0x08, 4, ID_GetDateTime);
}

GSM_Error GNAPGEN_ReplyGetManufacturer(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 	buff[200];
	int 		pos=8,len;

	len=msg.Buffer[pos]*256+msg.Buffer[pos+1];
	memset(buff,0,sizeof(buff));
	memcpy(buff,msg.Buffer+pos+2,len*2);

	strcpy(s->Phone.Data.Manufacturer,DecodeUnicodeString(buff));

	return ERR_NONE;
}

GSM_Error GNAPGEN_GetManufacturer(GSM_StateMachine *s)
{
	unsigned char req[2] = {0x00,0x01};

	smprintf(s, "Getting manufacturer\n");
	return GSM_WaitFor (s, req, 2, 0x01, 2, ID_GetManufacturer);
}

GSM_Error GNAPGEN_ReplyGetIMEI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 	buff[200];
	int 		pos=8,len,i;

	for (i=0;i<3;i++) {
		len=msg.Buffer[pos]*256+msg.Buffer[pos+1];
		memset(buff,0,sizeof(buff));
		memcpy(buff,msg.Buffer+pos+2,len*2);
		pos+=2+len*2;
	}

	strcpy(s->Phone.Data.IMEI,DecodeUnicodeString(buff));
	smprintf(s, "Received IMEI %s\n",s->Phone.Data.IMEI);

	return ERR_NONE;
}

GSM_Error GNAPGEN_GetIMEI(GSM_StateMachine *s)
{
	unsigned char req[2] = {0x00,0x01};

	smprintf(s, "Getting IMEI\n");
	return GSM_WaitFor (s, req, 2, 0x01, 2, ID_GetIMEI);
}

GSM_Error GNAPGEN_ReplyGetID(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return ERR_NONE;
}

GSM_Error GNAPGEN_ReplyGetModelFirmware(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Lines	lines;
	GSM_Phone_Data	*Data = &s->Phone.Data;

	if (Data->RequestID!=ID_GetManufacturer && Data->RequestID!=ID_GetModel) return ERR_NONE;

	SplitLines(DecodeUnicodeString(msg.Buffer+6), msg.Length-6, &lines, "\x0A", 1, false);

	strcpy(Data->Model,GetLineString(DecodeUnicodeString(msg.Buffer+6), lines, 4));
	smprintf(s, "Received model %s\n",Data->Model);
	Data->ModelInfo = GetModelData(NULL,Data->Model,NULL);

	strcpy(Data->VerDate,GetLineString(DecodeUnicodeString(msg.Buffer+6), lines, 3));
	smprintf(s, "Received firmware date %s\n",Data->VerDate);

	strcpy(Data->Version,GetLineString(DecodeUnicodeString(msg.Buffer+6), lines, 2));
	smprintf(s, "Received firmware version %s\n",Data->Version);
	GSM_CreateFirmwareNumber(s);

	return ERR_NONE;
}

GSM_Error GNAPGEN_GetModel (GSM_StateMachine *s)
{
	unsigned char req[2] = {0x00,0x01};
	GSM_Error 	error;

	if (strlen(s->Phone.Data.Model)>0) return ERR_NONE;

	smprintf(s, "Getting model\n");
	return GSM_WaitFor (s, req, 2, 0x01, 2, ID_GetModel);
	if (error==ERR_NONE) {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
			smprintf(s, "[Connected model  - \"%s\"]\n",s->Phone.Data.Model);
			smprintf(s, "[Firmware version - \"%s\"]\n",s->Phone.Data.Version);
			smprintf(s, "[Firmware date    - \"%s\"]\n",s->Phone.Data.VerDate);
		}
	}
	return error;
}

GSM_Error GNAPGEN_GetFirmware (GSM_StateMachine *s)
{
	unsigned char req[2] = {0x00,0x01};
	GSM_Error error;

	if (strlen(s->Phone.Data.Version)>0) return ERR_NONE;

	smprintf(s, "Getting firmware version\n");
	return GSM_WaitFor (s, req, 2, 0x01, 2, ID_GetFirmware);
	if (error==ERR_NONE) {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
			smprintf(s, "[Connected model  - \"%s\"]\n",s->Phone.Data.Model);
			smprintf(s, "[Firmware version - \"%s\"]\n",s->Phone.Data.Version);
			smprintf(s, "[Firmware date    - \"%s\"]\n",s->Phone.Data.VerDate);
		}
	}
	return error;
}

static GSM_Reply_Function GNAPGENReplyFunctions[] = {
	//informations
	{GNAPGEN_ReplyGetIMEI,		"\x01",0x01,0x02,ID_GetIMEI			},
	{GNAPGEN_ReplyGetManufacturer,	"\x01",0x01,0x02,ID_GetManufacturer		},
	{GNAPGEN_ReplyGetID,		"\x01",0x01,0x02,ID_GetModel			},
	{GNAPGEN_ReplyGetID,		"\x01",0x01,0x02,ID_GetFirmware			},

	//type 5 is DEBUG
	{GNAPGEN_ReplyGetModelFirmware,	"\x05",0x01,0x02,ID_IncomingFrame		},

	//type 6 is SMS
	{GNAPGEN_ReplyGetSMSFolders,	"\x06",0x01,0x02,ID_GetSMSFolders		},
	{GNAPGEN_ReplyGetSMSC,		"\x06",0x01,0x16,ID_GetSMSC			},

	//time
	{GNAPGEN_ReplyGetDateTime,	"\x08",0x01,0x02,ID_GetDateTime			},

	{NULL,				"\x00",0x00,0x00,ID_None			}
};

GSM_Phone_Functions GNAPGENPhone = {
	"gnap",
	GNAPGENReplyFunctions,
	NONEFUNCTION,			/*	Initialise		*/
	NONEFUNCTION,			/*	Terminate 		*/
	GSM_DispatchMessage,
	NOTSUPPORTED,			/* 	ShowStartInfo		*/
	GNAPGEN_GetManufacturer,
	GNAPGEN_GetModel,
	GNAPGEN_GetFirmware,
	GNAPGEN_GetIMEI,
	NOTSUPPORTED,			/*	GetOriginalIMEI		*/
	NOTSUPPORTED,			/*	GetManufactureMonth	*/
	NOTSUPPORTED,			/*	GetProductCode		*/
	NOTSUPPORTED,			/*	GetHardware		*/
	NOTSUPPORTED,			/*	GetPPM			*/
	NOTSUPPORTED,			/*	GetSIMIMSI		*/
	GNAPGEN_GetDateTime,
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
	NOTSUPPORTED,			/*	GetMemoryStatus		*/
	NOTSUPPORTED,			/*	GetMemory		*/
	NOTSUPPORTED,			/*	GetNextMemory		*/
	NOTSUPPORTED,			/*	SetMemory		*/
	NOTSUPPORTED,			/*	AddMemory		*/
	NOTSUPPORTED,			/*	DeleteMemory		*/
	NOTIMPLEMENTED,			/*	DeleteAllMemory		*/
	NOTSUPPORTED,			/*	GetSpeedDial		*/
	NOTSUPPORTED,			/*	SetSpeedDial		*/
	GNAPGEN_GetSMSC,
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
	GNAPGEN_GetSMSFolders,
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
	NOTSUPPORTED,			/*	GetSyncMLSettings	*/
	NOTSUPPORTED,			/*	SetSyncMLSettings	*/
	NOTSUPPORTED,			/*	GetChatSettings		*/
	NOTSUPPORTED,			/*	SetChatSettings		*/
	NOTSUPPORTED,			/* 	GetMMSSettings		*/
	NOTSUPPORTED,			/* 	SetMMSSettings		*/
	NOTSUPPORTED,			/*	GetMMSFolders		*/
	NOTSUPPORTED,			/*	GetNextMMSFileInfo	*/
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
	NOTSUPPORTED,			/*	GetFolderListing	*/
	NOTSUPPORTED,			/*	GetNextRootFolder	*/
	NOTSUPPORTED,			/*	SetFileAttributes	*/
	NOTSUPPORTED,			/* 	GetFilePart		*/
	NOTSUPPORTED,			/* 	AddFilePart		*/
	NOTSUPPORTED, 			/* 	GetFileSystemStatus	*/
	NOTSUPPORTED,			/* 	DeleteFile		*/
	NOTSUPPORTED,			/* 	AddFolder		*/
	NOTSUPPORTED,			/* 	DeleteFolder		*/
	NOTSUPPORTED,			/* 	GetGPRSAccessPoint	*/
	NOTSUPPORTED			/* 	SetGPRSAccessPoint	*/
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
