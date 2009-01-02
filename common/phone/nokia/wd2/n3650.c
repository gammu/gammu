/* (c) 2003 by Marcin Wiacek */

#include "../../../gsmstate.h"

#ifdef GSM_ENABLE_NOKIA3650

#include <string.h>
#include <time.h>

#include "../../../gsmcomon.h"
#include "../../../misc/coding/coding.h"
#include "../../../service/gsmlogo.h"
#include "../nfunc.h"
#include "../nfuncold.h"
#include "../../pfunc.h"
#include "../dct4s40/dct4func.h"
#include "n3650.h"

static GSM_Error N3650_ReplyGetFilePart(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int old;

	smprintf(s,"File part received\n");
	old = s->Phone.Data.File->Used;

	if (msg.Length < 10) {
		if (old == 0) return ERR_UNKNOWN;
		return ERR_EMPTY;
	}

	s->Phone.Data.File->Used += msg.Buffer[10]*256*256*256+
			    	    msg.Buffer[11]*256*256+
			    	    msg.Buffer[12]*256+
			    	    msg.Buffer[13];
	smprintf(s,"Length: %i\n",
			msg.Buffer[10]*256*256*256+
			msg.Buffer[11]*256*256+
			msg.Buffer[12]*256+
			msg.Buffer[13]);
	s->Phone.Data.File->Buffer = (unsigned char *)realloc(s->Phone.Data.File->Buffer,s->Phone.Data.File->Used);
	memcpy(s->Phone.Data.File->Buffer+old,msg.Buffer+18,s->Phone.Data.File->Used-old);
	if (s->Phone.Data.File->Used-old < 0x03 * 256 + 0xD4) return ERR_EMPTY;
	return ERR_NONE;
}

static GSM_Error N3650_GetFilePart(GSM_StateMachine *s, GSM_File *File, int *Handle UNUSED, int *Size)
{
	unsigned int 		len=10,i;
	GSM_Error		error;
	unsigned char 		StartReq[500] = {
		N7110_FRAME_HEADER, 0x0D, 0x10, 0x01, 0x07,
		0x24,		/* len1 */
		0x12,		/* len2 */
		0x0E,		/* len3 */
		0x00};		/* File name */
	unsigned char		ContinueReq[] = {
		N7110_FRAME_HEADER, 0x0D, 0x20, 0x01, 0xF0,
		0x08, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00};

	if (File->Used == 0) {
		(*Size) = 0;

		sprintf(StartReq+10,"%s",File->ID_FullName);
		len+=strlen(File->ID_FullName)-1;
		StartReq[7] = strlen(File->ID_FullName) + 3;

		StartReq[8] = strlen(File->ID_FullName);
		StartReq[9] = 0;
		while (File->ID_FullName[StartReq[8]] != '\\') {
			StartReq[8]--;
			StartReq[9]++;
		}
		for (i=StartReq[8];i<strlen(File->ID_FullName);i++) {
			StartReq[i+10] = StartReq[i+1+10];
		}
		StartReq[9]--;

		EncodeUnicode(File->Name,File->ID_FullName+StartReq[8]+1,StartReq[9]);
		File->Folder = false;

		error = DCT4_SetPhoneMode(s, DCT4_MODE_TEST);
		if (error != ERR_NONE) return error;

		s->Phone.Data.File = File;
		return GSM_WaitFor (s, StartReq, len, 0x58, 4, ID_GetFile);
	}

	s->Phone.Data.File = File;
	error = GSM_WaitFor (s, ContinueReq, 14, 0x58, 4, ID_GetFile);

#if 0
	if (error == GE_EMPTY) {
		error = DCT4_SetPhoneMode(s, DCT4_MODE_NORMAL);
		if (error != ERR_NONE) return error;
		return GE_EMPTY;
	}
#endif

	return error;
}

static GSM_Error N3650_ReplyGetFolderInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_File	 	*File = s->Phone.Data.FileInfo;
	GSM_Phone_N3650Data	*Priv = &s->Phone.Data.Priv.N3650;
	int			i,pos = 6;

	i = Priv->FilesLocationsUsed-1;
	while (1) {
		if (i==Priv->FilesLocationsCurrent-1) break;
		smprintf(s, "Copying %i to %i, max %i, current %i\n",
			i,i+msg.Buffer[5],
			Priv->FilesLocationsUsed,Priv->FilesLocationsCurrent);
		memcpy(Priv->Files[i+msg.Buffer[5]],Priv->Files[i],sizeof(GSM_File));
		i--;
	}
	Priv->FileEntries	  = msg.Buffer[5];
	Priv->FilesLocationsUsed += msg.Buffer[5];
	for (i=0;i<msg.Buffer[5];i++) {
		Priv->Files[Priv->FilesLocationsCurrent+i]->Folder = true;
		if (msg.Buffer[pos+2] == 0x01) {
			Priv->Files[Priv->FilesLocationsCurrent+i]->Folder = false;
			smprintf(s,"File ");
		}
		EncodeUnicode(Priv->Files[Priv->FilesLocationsCurrent+i]->Name,msg.Buffer+pos+9,msg.Buffer[pos+8]);
		smprintf(s,"%s\n",DecodeUnicodeString(Priv->Files[Priv->FilesLocationsCurrent+i]->Name));
		Priv->Files[Priv->FilesLocationsCurrent+i]->Level  = File->Level+1;
		sprintf(Priv->Files[Priv->FilesLocationsCurrent+i]->ID_FullName,"%s\\%s",File->ID_FullName,msg.Buffer+pos+9);
		pos+=msg.Buffer[pos+1];
	}
	smprintf(s, "\n");
	return ERR_NONE;
}

static GSM_Error N3650_GetFolderInfo(GSM_StateMachine *s, GSM_File *File)
{
	int 			len=10;
	unsigned char 		req[500] = {
		N7110_FRAME_HEADER, 0x0B, 0x00, 0x01, 0x07,
		0x18,		/* folder name length + 6 	*/
		0x12,		/* folder name length 		*/
		0x00,
		0x00};		/* folder name 			*/

	/* FIXME: I doubt this works */
	sprintf(req+10,"%s", File->ID_FullName);
	len		+=strlen(File->ID_FullName);
	req[7] 		= strlen(File->ID_FullName) + 6;
	req[8] 		= strlen(File->ID_FullName);
	req[len++] 	= 0x00;
	req[len++] 	= 0x00;

	s->Phone.Data.FileInfo = File;
	return GSM_WaitFor (s, req, len, 0x58, 4, ID_GetFile);
}

static GSM_Error N3650_GetNextFileFolder(GSM_StateMachine *s, GSM_File *File, bool start)
{
	GSM_Error		error;
	GSM_Phone_N3650Data	*Priv = &s->Phone.Data.Priv.N3650;

	if (start) {
		error = DCT4_SetPhoneMode(s, DCT4_MODE_LOCAL);
		if (error != ERR_NONE) return error;

		Priv->Files[0]->Folder		= true;
		Priv->Files[0]->Level		= 1;
		Priv->Files[0]->Name[0]		= 0;
		Priv->Files[0]->Name[1]		= 0;
		Priv->Files[0]->ID_FullName[0]	= 'Z';
		Priv->Files[0]->ID_FullName[1]	= ':';
		Priv->Files[0]->ID_FullName[2]	= 0;

		Priv->Files[1]->Folder		= true;
		Priv->Files[1]->Level		= 1;
		Priv->Files[1]->Name[0]		= 0;
		Priv->Files[1]->Name[1]		= 0;
		Priv->Files[1]->ID_FullName[0]	= 'E';
		Priv->Files[1]->ID_FullName[1]	= ':';
		Priv->Files[1]->ID_FullName[2]	= 0;

		Priv->Files[2]->Folder		= true;
		Priv->Files[2]->Level		= 1;
		Priv->Files[2]->Name[0]		= 0;
		Priv->Files[2]->Name[1]		= 0;
		Priv->Files[2]->ID_FullName[0]	= 'C';
		Priv->Files[2]->ID_FullName[1]	= ':';
		Priv->Files[2]->ID_FullName[2]	= 0;

		Priv->FilesLocationsUsed 	= 3;
		Priv->FilesLocationsCurrent 	= 0;
		Priv->FileLev			= 1;
	}

	if (Priv->FilesLocationsCurrent == Priv->FilesLocationsUsed) {
#if 0
		error = DCT4_SetPhoneMode(s, DCT4_MODE_NORMAL);
		if (error != ERR_NONE) return error;
#endif

		return ERR_EMPTY;
	}

	strcpy(File->ID_FullName,Priv->Files[Priv->FilesLocationsCurrent]->ID_FullName);
	File->Level	= Priv->Files[Priv->FilesLocationsCurrent]->Level;
	File->Folder	= Priv->Files[Priv->FilesLocationsCurrent]->Folder;
	CopyUnicodeString(File->Name,Priv->Files[Priv->FilesLocationsCurrent]->Name);
	Priv->FilesLocationsCurrent++;

	if (!File->Folder) return ERR_NONE;

	if (Priv->FilesLocationsCurrent > 1) {
		if (File->ID_FullName[0]!=Priv->Files[Priv->FilesLocationsCurrent-2]->ID_FullName[0]) {
			if (File->ID_FullName[0] == 'E') {
				error = DCT4_SetPhoneMode(s, DCT4_MODE_TEST);
				error = DCT4_SetPhoneMode(s, DCT4_MODE_TEST);
			}
			if (File->ID_FullName[0] == 'C') {
				error = DCT4_SetPhoneMode(s, DCT4_MODE_LOCAL);
				error = DCT4_SetPhoneMode(s, DCT4_MODE_LOCAL);
			}
#if 0
		if (error != ERR_NONE) return error;
#endif
		}
	}

	File->ReadOnly  = false;
	File->System    = false;
	File->Protected = false;
	File->Hidden    = false;

	return N3650_GetFolderInfo(s, File);
}

static GSM_Error N3650_Initialise (GSM_StateMachine *s)
{
	GSM_Phone_N3650Data 	*Priv = &s->Phone.Data.Priv.N3650;
	int			i;

	for (i=0;i<10000;i++) {
		Priv->Files[i] = malloc(sizeof(GSM_File));
	        if (Priv->Files[i] == NULL) return ERR_MOREMEMORY;
	}
	return ERR_NONE;
}

static GSM_Error N3650_Terminate(GSM_StateMachine *s)
{
	GSM_Phone_N3650Data 	*Priv = &s->Phone.Data.Priv.N3650;
	int			i;

	for (i=0;i<10000;i++) free(Priv->Files[i]);
	return ERR_NONE;
}

static GSM_Reply_Function N3650ReplyFunctions[] = {
	{DCT4_ReplySetPhoneMode,	  "\x15",0x03,0x64,ID_Reset		  },
	{DCT4_ReplyGetPhoneMode,	  "\x15",0x03,0x65,ID_Reset		  },
	{NoneReply,		  	  "\x15",0x03,0x68,ID_Reset		  },

	{DCT4_ReplyGetIMEI,		  "\x1B",0x03,0x01,ID_GetIMEI		  },
	{NOKIA_ReplyGetPhoneString,	  "\x1B",0x03,0x08,ID_GetHardware	  },
	{NOKIA_ReplyGetPhoneString,	  "\x1B",0x03,0x0C,ID_GetProductCode	  },

	{N3650_ReplyGetFolderInfo,	  "\x58",0x03,0x0C,ID_GetFile		  },
	{N3650_ReplyGetFilePart,	  "\x58",0x03,0x0E,ID_GetFile		  },

	{NULL,				  "\x00",0x00,0x00,ID_None		  }
};

GSM_Phone_Functions N3650Phone = {
	"3650|NGAGE",
	N3650ReplyFunctions,
	N3650_Initialise,
	N3650_Terminate,
	GSM_DispatchMessage,
	NOTSUPPORTED,			/* 	ShowStartInfo		*/
	NOKIA_GetManufacturer,
	DCT3DCT4_GetModel,
	DCT3DCT4_GetFirmware,
	DCT4_GetIMEI,
	NOTSUPPORTED,			/*	GetOriginalIMEI		*/
	NOTSUPPORTED,			/*	GetManufactureMonth	*/
	DCT4_GetProductCode,
	DCT4_GetHardware,
	NOTSUPPORTED,			/*	GetPPM			*/
	NOTSUPPORTED,			/*	GetSIMIMSI		*/
	NOTSUPPORTED,			/*	GetDateTime		*/
	NOTSUPPORTED,			/*	SetDateTime		*/
	NOTSUPPORTED,			/*	GetAlarm		*/
	NOTSUPPORTED,			/*	SetAlarm		*/
	NOTSUPPORTED,			/* 	GetLocale		*/
	NOTSUPPORTED,			/* 	SetLocale		*/
	NOTSUPPORTED,			/*	PressKey		*/
	DCT4_Reset,
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
	NOTIMPLEMENTED,			/*	GetCalendarStatus	*/
	NOTIMPLEMENTED,			/*	GetCalendar		*/
    	NOTSUPPORTED,			/*  	GetNextCalendar		*/
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
	N3650_GetNextFileFolder,
	NOTSUPPORTED,			/*	GetFolderListing	*/
	NOTSUPPORTED,			/*	GetNextRootFolder	*/
	NOTSUPPORTED,			/*	SetFileAttributes	*/
	N3650_GetFilePart,
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
