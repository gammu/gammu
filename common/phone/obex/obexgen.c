
/* www.irda.org OBEX specs 1.3 */

#include <string.h>
#include <time.h>

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "../../misc/coding.h"

#ifdef GSM_ENABLE_OBEXGEN

static GSM_Error OBEXGEN_ReplyConnect(GSM_Protocol_Message msg, GSM_StateMachine *s)
{                
	smprintf(s,"Init done\n");
	return GE_NONE;
}

GSM_Error OBEXGEN_Connect(GSM_StateMachine *s, char *buff, int length)
{
	unsigned char req[200] = {0x10,0x00,0x20,0x00};

	if (length != 0) memcpy(req+4,buff,length);

	smprintf(s, "Connecting\n");
	return GSM_WaitFor (s, req, 4+length, 0x80, 2, ID_GetIMEI);
}


GSM_Error OBEXGEN_Initialise(GSM_StateMachine *s)
{
	s->Phone.Data.VerNum 		= 0;
	s->Phone.Data.Version[0] 	= 0;
	s->Phone.Data.Manufacturer[0] 	= 0;
	strcpy(s->Phone.Data.Model,"obex");

	return GE_NONE;
}

static GSM_Error OBEXGEN_ReplyAddFilePart(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s,"Part of file added\n");
	return GE_NONE;
}

static GSM_Error OBEXGEN_AddFilePart(GSM_StateMachine *s, GSM_File *File, int *Pos)
{
	GSM_Error		error;
	int			j, Current = 0;
	unsigned char 		req[2000],req2[200];

	if (File->ID != 0) return GE_NOTSUPPORTED;

	s->Phone.Data.File = File;

	error = OBEXGEN_Connect(s,NULL,0);
	if (error != GE_NONE) return error;

	if (*Pos == 0) {
		/* Name block */
		OBEXAddBlock(req, &Current, 0x01, File->Name, UnicodeLength(File->Name)*2+2);

		/* File size block */
		req2[0] = File->Used / 256;
		req2[1] = File->Used % 256;
		OBEXAddBlock(req, &Current, 0xC3, req2, 2);
	}

	j = 1000;
	if (File->Used - *Pos < 1000) j = File->Used - *Pos;

	/* File body block */
	OBEXAddBlock(req, &Current, 0x48, File->Buffer+(*Pos), j);

	smprintf(s, "Adding file part %i %i\n",*Pos,j);
	error=GSM_WaitFor (s, req, Current, 0x02, 4, ID_AddFile);
	if (error != GE_NONE) return error;
	*Pos = *Pos + j;

	if (j < 1000) {
		/* End of file body block */
		OBEXAddBlock(req, &Current, 0x49, NULL, 0);

		smprintf(s, "Frame for ending adding file\n");
		error = GSM_WaitFor (s, req, 3, 0x82, 4, ID_AddFile);
		if (error != GE_NONE) return error;
		return GE_EMPTY;
	}

	return GE_NONE;
}

static GSM_Error OBEXGEN_ReplyGetFilePart(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int old;

	switch (msg.Type) {
	case 0xA0:
	case 0x90:
		if (msg.Length < 6) return GE_NONE;
		smprintf(s,"File part received\n");
		old = s->Phone.Data.File->Used;
		s->Phone.Data.File->Used += msg.Buffer[1]*256+msg.Buffer[2]-3;
		smprintf(s,"Length of file part: %i\n",
				msg.Buffer[1]*256+msg.Buffer[2]-3);

		s->Phone.Data.File->Buffer = (unsigned char *)realloc(s->Phone.Data.File->Buffer,s->Phone.Data.File->Used);
		memcpy(s->Phone.Data.File->Buffer+old,msg.Buffer+3,s->Phone.Data.File->Used-old);
		return GE_NONE;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error OBEXGEN_GetFilePart(GSM_StateMachine *s, GSM_File *File)
{
	int 			Current = 0;
	GSM_Error		error;
	unsigned char 		req[2000], req2[200];

	s->Phone.Data.File = File;

	if (File->Used == 0x00) {
		switch (File->ID) {
		case 0x00:
			error = OBEXGEN_Connect(s,NULL,0);
			if (error != GE_NONE) return error;

			EncodeUnicode(File->Name,"one",3);

			/* Type block */
			strcpy(req2,"x-obex/capability");
			OBEXAddBlock(req, &Current, 0x42, req2, strlen(req2)+1);
			break;
		case 0xFF:

			Current = 0;

			/* Type block */
			strcpy(req2,"x-obex/folder-listing");
			OBEXAddBlock(req, &Current, 0x42, req2, strlen(req2)+1);

			/* Name block */
			CopyUnicodeString(req2,File->Name);
			OBEXAddBlock(req, &Current, 0x01, req2, UnicodeLength(req2)*2+2);

			break;
		default:
			return GE_NOTSUPPORTED;
		}
		File->Folder = false;
	}

	if (File->ID == 0xFF) {
		req[Current++] = 0xCB; req[Current++] = 0x00;
		req[Current++] = 0x00; req[Current++] = 0x00;
		req[Current++] = 0x01;
	}

	smprintf(s, "Getting file part from filesystem\n");
	error=GSM_WaitFor (s, req, Current, 0x83, 4, ID_GetFileInfo);

	Current = 0;
	if (File->ID == 0xFF) {
		req[Current++] = 0xCB; req[Current++] = 0x00;
		req[Current++] = 0x00; req[Current++] = 0x00;
		req[Current++] = 0x01;
	}

	smprintf(s, "Getting file part from filesystem\n");
//	error=GSM_WaitFor (s, req, Current, 0x83, 4, ID_GetFile);
	if (error != GE_NONE) return error;

	Current = 0;
	if (File->ID == 0xFF) {
		req[Current++] = 0xCB; req[Current++] = 0x00;
		req[Current++] = 0x00; req[Current++] = 0x00;
		req[Current++] = 0x01;
	}
	smprintf(s, "Getting file part from filesystem\n");
	error=GSM_WaitFor (s, req, Current, 0x83, 4, ID_GetFile);
	if (error != GE_NONE) return error;

#ifdef xxxx
	Current = 0;
	if (File->ID == 0xFF) {
		req[Current++] = 0xCB; req[Current++] = 0x00;
		req[Current++] = 0x00; req[Current++] = 0x00;
		req[Current++] = 0x01;
	}
//	error=GSM_WaitFor (s, req, Current, 0x81, 4, ID_GetFile);
	if (error != GE_NONE) return error;
#endif

	return GE_EMPTY;
}

static GSM_Error OBEXGEN_GetNextFileFolder(GSM_StateMachine *s, GSM_File *File, bool start)
{
	GSM_Phone_OBEXGENData	*Priv = &s->Phone.Data.Priv.OBEXGEN;
	GSM_Error		error;
	unsigned char		Line[500],*name,req[200],req2[200];
	int			Pos,i,j,num,pos2,Current,z;

	if (start) {
		Priv->FilesFolder[0]		= true;
		Priv->FilesLocationsUsed 	= 1;
		Priv->FilesLocationsCurrent 	= 0;
		Priv->FilesParents[0]		= -1;
		Priv->FilesLocations[0]		= 0x01;
		Priv->FilesLevels[0]		= 1;
		Priv->FileLev			= 1;
		Priv->FilesNames[0][0]		= 0;
		Priv->FilesNames[0][1]		= 0;

		/* Server ID */
		req2[0] = 0xF9; req2[1] = 0xEC; req2[2] = 0x7B;
		req2[3] = 0xC4; req2[4] = 0x95; req2[5] = 0x3C;
		req2[6] = 0x11; req2[7] = 0xD2; req2[8] = 0x98;
		req2[9] = 0x4E; req2[10]= 0x52; req2[11]= 0x54;
		req2[12]= 0x00; req2[13]= 0xDC; req2[14]= 0x9E;
		req2[15]= 0x09;
		Current = 0;
		/* Target block */
		OBEXAddBlock(req, &Current, 0x46, req2, 16);

		error = OBEXGEN_Connect(s,req,Current);
		if (error != GE_NONE) return error;

		Current = 0;
	}

	while (1) {
		if (Priv->FilesLocationsCurrent == Priv->FilesLocationsUsed) return GE_EMPTY;

		File->ID 	= 0xFF;
		File->ParentID 	= 0;
		File->Level	= Priv->FilesLevels[Priv->FilesLocationsCurrent];
		File->Folder	= Priv->FilesFolder[Priv->FilesLocationsCurrent];
		CopyUnicodeString(File->Name,Priv->FilesNames[Priv->FilesLocationsCurrent]);
		Priv->FilesLocationsCurrent++;

		if (File->Folder) {
			if (File->Level < Priv->FileLev) {
				for (i=0;i<File->Level;i++) {
					smprintf(s,"Changing path up\n");
					/* Name block */
					req[0] = 255;
					req[1] = 0x00;
					Current = 2;
					req[Current++] = 0xCB; req[Current++] = 0x00;
					req[Current++] = 0x00; req[Current++] = 0x00;
					req[Current++] = 0x01;
					error=GSM_WaitFor (s, req, Current, 0x85, 4, ID_GetFile);
				}
			}
	
			smprintf(s,"Level %i %i\n",File->Level,Priv->FileLev);
	
			File->Buffer 	= NULL;		
			File->Used 	= 0;
			OBEXGEN_GetFilePart(s, File);

			num = 0;
			Pos = 0;
			while (1) {
				MyGetLine(File->Buffer, &Pos, Line);
				if (strlen(Line) == 0) break;
				name = strstr(Line,"folder name=\"");
				if (name != NULL) {
					name += 13;
					j = 0;
					while(1) {
						if (name[j] == '"') break;
						j++;
					}
					name[j] = 0;
	
					if (strcmp(name,".")) num++;
				}
				name = strstr(Line,"file name=\"");
				if (name != NULL) num++;
			}
			if (num != 0) {
				i = Priv->FilesLocationsUsed-1;
				while (1) {
					if (i==Priv->FilesLocationsCurrent-1) break;
					Priv->FilesLocations[i+num] 	= Priv->FilesLocations[i];
					Priv->FilesParents[i+num]	= Priv->FilesParents[i];
					Priv->FilesLevels[i+num]	= Priv->FilesLevels[i];
					Priv->FilesFolder[i+num]	= Priv->FilesFolder[i];
					Priv->FilesNames[i+num][0] 	= 0;
					Priv->FilesNames[i+num][1] 	= 0;
					CopyUnicodeString(Priv->FilesNames[i+num],Priv->FilesNames[i]);
					i--;
				}
			}
			Pos 	= 0;
			pos2 	= 0;
			while (1) {
				MyGetLine(File->Buffer, &Pos, Line);
				if (strlen(Line) == 0) break;
				name = strstr(Line,"folder name=\"");
				if (name != NULL) {
					name += 13;
					j = 0;
					while(1) {
						if (name[j] == '"') break;
						j++;
					}
					name[j] = 0;
					if (strcmp(name,".")) {
						dprintf("copying folder %s to %i\n",name,Priv->FilesLocationsCurrent+pos2);
						Priv->FilesLevels[Priv->FilesLocationsCurrent+pos2] = File->Level+1;
						Priv->FilesFolder[Priv->FilesLocationsCurrent+pos2] = true;
						EncodeUnicode(Priv->FilesNames[Priv->FilesLocationsCurrent+pos2],name,strlen(name));
						Priv->FilesLocationsUsed++;
						pos2++;
					}
				}
				name = strstr(Line,"file name=\"");
				if (name != NULL) {
					name += 11;
					j = 0;
					while(1) {
						if (name[j] == '"') break;
						j++;
					}
					name[j] = 0;
					dprintf("copying file %s to %i\n",name,Priv->FilesLocationsCurrent+pos2);
					Priv->FilesLevels[Priv->FilesLocationsCurrent+pos2] = File->Level+1;
					Priv->FilesFolder[Priv->FilesLocationsCurrent+pos2] = false;
					EncodeUnicode(Priv->FilesNames[Priv->FilesLocationsCurrent+pos2],name,strlen(name));
					Priv->FilesLocationsUsed++;
					pos2++;
				}
			}

			z = Priv->FilesLocationsCurrent;
			while (1) {
				if (Priv->FilesFolder[z]) {
					if (Priv->FilesLevels[z] > File->Level) {
						smprintf(s,"Changing path down\n");
						req[0] = 64;
						req[1] = 0x00;
						/* Name block */
						Current = 2;
						OBEXAddBlock(req, &Current, 0x01, File->Name, UnicodeLength(File->Name)*2+2);
						req[Current++] = 0xCB; req[Current++] = 0x00;
						req[Current++] = 0x00; req[Current++] = 0x00;
						req[Current++] = 0x01;
						error=GSM_WaitFor (s, req, Current, 0x85, 4, ID_GetFile);
					}
					break;
				} 
				z++;
			}

			Priv->FileLev = File->Level;
			free(File->Buffer);
		} else {
			File->Used = Priv->FilesSizes[Priv->FilesLocationsCurrent-1];
		}
		return GE_NONE;
	}
}

static GSM_Reply_Function OBEXGENReplyFunctions[] = {
	{NONEFUNCTION,			"\x83",0x00,0x00,ID_GetFileInfo			},

	{OBEXGEN_ReplyAddFilePart,	"\x90",0x00,0x00,ID_AddFile			},
	{OBEXGEN_ReplyGetFilePart,	"\x90",0x00,0x00,ID_GetFile			},
	{NONEFUNCTION,			"\x90",0x00,0x00,ID_GetFileInfo			},

	{OBEXGEN_ReplyConnect,		"\xA0",0x00,0x00,ID_GetIMEI			},
	{OBEXGEN_ReplyAddFilePart,	"\xA0",0x00,0x00,ID_AddFile			},
	{OBEXGEN_ReplyGetFilePart,	"\xA0",0x00,0x00,ID_GetFile			},

	{NONEFUNCTION,			"\xA3",0x00,0x00,ID_GetFile			},

	{NONEFUNCTION,			"\xC0",0x00,0x00,ID_GetFile			},

	{NULL,				"\x00",0x00,0x00,ID_None			}
};

GSM_Phone_Functions OBEXGENPhone = {
	"obex",
	OBEXGENReplyFunctions,
	OBEXGEN_Initialise,
	NONEFUNCTION,		/*	Terminate 		*/
	GSM_DispatchMessage,
	NONEFUNCTION,
	NONEFUNCTION,
	NOTIMPLEMENTED,		/*	GetIMEI			*/
	NOTIMPLEMENTED,		/*	GetDateTime		*/
	NOTIMPLEMENTED,		/*	GetAlarm		*/
	NOTIMPLEMENTED,		/*	GetMemory		*/
	NOTIMPLEMENTED,		/*	GetMemoryStatus		*/
	NOTIMPLEMENTED,		/*	GetSMSC			*/
	NOTIMPLEMENTED,		/*	GetSMSMessage		*/
	NOTIMPLEMENTED,		/*	GetSMSFolders		*/
	NONEFUNCTION,
	NOTIMPLEMENTED,		/*	GetNextSMSMessage	*/
	NOTIMPLEMENTED,		/*	GetSMSStatus		*/
	NOTIMPLEMENTED,		/*	SetIncomingSMS		*/
	NOTIMPLEMENTED,		/*	GetNetworkInfo		*/
	NOTIMPLEMENTED,		/*	Reset			*/
	NOTIMPLEMENTED,		/*	DialVoice		*/
	NOTIMPLEMENTED,		/*	AnswerCall		*/
	NOTIMPLEMENTED,		/*	CancelCall		*/
	NOTIMPLEMENTED,		/*	GetRingtone		*/
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
	NOTIMPLEMENTED,		/* 	SetWAPBookmark 		*/
	NOTIMPLEMENTED, 	/* 	DeleteWAPBookmark 	*/
	NOTIMPLEMENTED,		/* 	GetWAPSettings 		*/
	NOTIMPLEMENTED,		/* 	SetIncomingCB		*/
	NOTIMPLEMENTED,		/*	SetSMSC			*/
	NOTIMPLEMENTED,		/*	GetManufactureMonth	*/
	NOTIMPLEMENTED,		/*	GetProductCode		*/
	NOTIMPLEMENTED,		/*	GetOriginalIMEI		*/
	NOTIMPLEMENTED,		/*	GetHardware		*/
	NOTIMPLEMENTED,		/*	GetPPM			*/
	NOTIMPLEMENTED,		/*	PressKey		*/
	NOTIMPLEMENTED,		/*	GetToDo			*/
	NOTIMPLEMENTED,		/*	DeleteAllToDo		*/
	NOTIMPLEMENTED,		/*	SetToDo			*/
	NOTIMPLEMENTED,		/*	GetToDoStatus		*/
	NOTIMPLEMENTED,		/*	PlayTone		*/
	NOTIMPLEMENTED,		/*	EnterSecurityCode	*/
	NOTIMPLEMENTED,		/*	GetSecurityStatus	*/
	NOTIMPLEMENTED, 	/*	GetProfile		*/
	NOTIMPLEMENTED,		/*	GetRingtonesInfo	*/
	NOTIMPLEMENTED,		/* 	SetWAPSettings 		*/
	NOTIMPLEMENTED,		/*	GetSpeedDial		*/
	NOTIMPLEMENTED,		/*	SetSpeedDial		*/
	NOTIMPLEMENTED,		/*	ResetPhoneSettings	*/
	NOTIMPLEMENTED,		/*	SendDTMF		*/
	NOTIMPLEMENTED,		/*	GetDisplayStatus	*/
	NOTIMPLEMENTED,		/*	SetAutoNetworkLogin	*/
	NOTIMPLEMENTED, 	/*	SetProfile		*/
	NOTIMPLEMENTED,		/*	GetSIMIMSI		*/
	NOTIMPLEMENTED,		/*	SetIncomingCall		*/
    	NOTIMPLEMENTED,		/*	GetNextCalendar		*/
	NOTIMPLEMENTED,   	/*	DelCalendar		*/
	NOTIMPLEMENTED,       	/*	AddCalendar		*/
	NOTIMPLEMENTED,		/*	GetBatteryCharge	*/
	NOTIMPLEMENTED,		/*	GetSignalQuality	*/
	NOTIMPLEMENTED,     	/*  	GetCategory 		*/
        NOTIMPLEMENTED,      	/*  	GetCategoryStatus 	*/	
    	NOTIMPLEMENTED,		/*  	GetFMStation        	*/
    	NOTIMPLEMENTED,		/*  	SetFMStation        	*/
    	NOTIMPLEMENTED,		/*  	ClearFMStations       	*/
	NOTIMPLEMENTED,		/*  	SetIncomingUSSD		*/
	NOTIMPLEMENTED,		/* 	DeleteUserRingtones	*/
	NOTIMPLEMENTED,		/* 	ShowStartInfo		*/
	OBEXGEN_GetNextFileFolder,
	OBEXGEN_GetFilePart,
	OBEXGEN_AddFilePart,
	NOTIMPLEMENTED, 	/* 	GetFreeFileMemory 	*/
	NOTIMPLEMENTED,		/*	DeleteFile		*/
	NOTIMPLEMENTED,		/*	AddFolder		*/
	NOTIMPLEMENTED,		/* 	GetMMSSettings		*/
	NOTIMPLEMENTED,		/* 	GetGPRSAccessPoint	*/
	NOTIMPLEMENTED		/* 	SetGPRSAccessPoint	*/
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
