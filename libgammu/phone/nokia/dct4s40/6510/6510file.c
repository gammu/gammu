/* (c) 2003-2006 by Marcin Wiacek */
/* function for making CRC for filesystem (c) 2003 by Michael Schroeder */

#include <gammu-config.h>

#ifdef GSM_ENABLE_NOKIA6510

#include <string.h>
#include <time.h>

#include "../../../../misc/coding/coding.h"
#include "../../../../gsmcomon.h"
#include "../../../../service/gsmlogo.h"
#include "../../nfunc.h"
#include "../../nfuncold.h"
#include "../../../pfunc.h"
#include "../dct4func.h"
#include "n6510.h"
#include "../../../../../helper/string.h"

/* shared */

static int N6510_FindFileCheckSum12(GSM_StateMachine *s, unsigned char *ptr, int len)
{
	int acc, i, accx;

	accx = 0;
	acc  = 0xffff;
	while (len--) {
		accx = (accx & 0xffff00ff) | (acc & 0xff00);
		acc  = (acc  & 0xffff00ff) | (*ptr++ << 8);
		for (i = 0; i < 8; i++) {
			acc <<= 1;
			if (acc & 0x10000)     acc ^= 0x1021;
			if (accx & 0x80000000) acc ^= 0x1021;
			accx <<= 1;
		}
	}
	smprintf(s, "Checksum from Gammu is %04X\n",(acc & 0xffff));
	return (acc & 0xffff);
}

GSM_Error N6510_ReplyGetFilePart12(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int old;

	smprintf(s,"File part received\n");
	old = s->Phone.Data.File->Used;
	s->Phone.Data.File->Used += msg.Buffer[6]*256*256*256+
				    msg.Buffer[7]*256*256+
				    msg.Buffer[8]*256+
				    msg.Buffer[9];
	smprintf(s,"Length of file part: %i\n",
			msg.Buffer[6]*256*256*256+
			msg.Buffer[7]*256*256+
			msg.Buffer[8]*256+
			msg.Buffer[9]);
	s->Phone.Data.File->Buffer = (unsigned char *)realloc(s->Phone.Data.File->Buffer,s->Phone.Data.File->Used);
	memcpy(s->Phone.Data.File->Buffer+old,msg.Buffer+10,s->Phone.Data.File->Used-old);
	return ERR_NONE;
}

GSM_Error N6510_ReplyGetFileCRC12(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_N6510Data *Priv = &s->Phone.Data.Priv.N6510;

	Priv->FileCheckSum = msg.Buffer[6] * 256 + msg.Buffer[7];
	smprintf(s,"File checksum from phone is %04X\n",Priv->FileCheckSum);
	return ERR_NONE;
}

/* filesystem 1 */

static GSM_Error N6510_GetFileCRC1(GSM_StateMachine *s, unsigned char *id)
{
	unsigned char	   	GetCRC[] = {
		N7110_FRAME_HEADER, 0x42, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x1E};    /* file ID */

	GetCRC[8] = atoi(DecodeUnicodeString(id)) / 256;
	GetCRC[9] = atoi(DecodeUnicodeString(id)) % 256;
	smprintf(s,"Getting CRC for file in filesystem\n");
	return GSM_WaitFor (s, GetCRC, 10, 0x6D, 4, ID_GetCRC);
}

GSM_Error N6510_ReplyGetFileFolderInfo1(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_File		*File = s->Phone.Data.FileInfo;
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	int		     	i;
	unsigned		char buffer[500];

	switch (msg.Buffer[3]) {
	case 0x15:
		smprintf(s,"File or folder details received\n");
		CopyUnicodeString(File->Name,msg.Buffer+10);
		if (msg.Length == 14) {
			smprintf(s,"File not exist\n");
			return ERR_FILENOTEXIST;
		}
		if (!strncmp(DecodeUnicodeString(File->Name),"GMSTemp",7)) return ERR_EMPTY;
		if (File->Name[0] == 0x00 && File->Name[1] == 0x00) return ERR_UNKNOWN;

/* 		EncodeHexUnicode (buffer, File->Name, UnicodeLength(File->Name)); */
/* 		smprintf(s,"Name encoded: %s\n",buffer); */

		i = msg.Buffer[8]*256+msg.Buffer[9];
		smprintf(s, "%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			msg.Buffer[i-5],msg.Buffer[i-4],msg.Buffer[i-3],
			msg.Buffer[i-2],msg.Buffer[i-1],msg.Buffer[i],
			msg.Buffer[i+1],msg.Buffer[i+2],msg.Buffer[i+3]);

		File->Folder = false;
		if (msg.Buffer[i-5] == 0x00 && msg.Buffer[i-3]==0x02) File->Folder = true;

		File->ReadOnly  = false;
		File->Protected = false;
		File->System    = false;
		File->Hidden    = false;
		if (msg.Buffer[i+2] == 0x01) File->Protected    = true;
		if (msg.Buffer[i+4] == 0x01) File->ReadOnly     = true;
		if (msg.Buffer[i+5] == 0x01) File->Hidden       = true;
		if (msg.Buffer[i+6] == 0x01) File->System       = true;/* fixme */

		File->ModifiedEmpty = false;
		NOKIA_DecodeDateTime(s, msg.Buffer+i-22, &File->Modified, true, false);
		if (File->Modified.Year == 0x00) File->ModifiedEmpty = true;
		if (File->Modified.Year == 0xffff) File->ModifiedEmpty = true;
		smprintf(s, "%02x %02x %02x %02x\n",msg.Buffer[i-22],msg.Buffer[i-21],msg.Buffer[i-20],msg.Buffer[i-19]);

		Priv->FileToken = msg.Buffer[i-10]*256+msg.Buffer[i-9];
		Priv->ParentID  = msg.Buffer[i]*256+msg.Buffer[i+1];
		smprintf(s,"ParentID is %i\n",Priv->ParentID);

		File->Type = GSM_File_Other;
		if (msg.Length > 240){
			i = 227;
			if (msg.Buffer[i]==0x02 && msg.Buffer[i+2]==0x01)
				File->Type = GSM_File_Image_JPG;
			else if (msg.Buffer[i]==0x02 && msg.Buffer[i+2]==0x02)
				File->Type = GSM_File_Image_BMP;
			else if (msg.Buffer[i]==0x02 && msg.Buffer[i+2]==0x07)
				File->Type = GSM_File_Image_BMP;
			else if (msg.Buffer[i]==0x02 && msg.Buffer[i+2]==0x03)
				File->Type = GSM_File_Image_PNG;
			else if (msg.Buffer[i]==0x02 && msg.Buffer[i+2]==0x05)
				File->Type = GSM_File_Image_GIF;
			else if (msg.Buffer[i]==0x02 && msg.Buffer[i+2]==0x09)
				File->Type = GSM_File_Image_WBMP;
			else if (msg.Buffer[i]==0x04 && msg.Buffer[i+2]==0x01)
				File->Type = GSM_File_Sound_AMR;
			else if (msg.Buffer[i]==0x04 && msg.Buffer[i+2]==0x02)
				File->Type = GSM_File_Sound_MIDI;
			else if (msg.Buffer[i]==0x08 && msg.Buffer[i+2]==0x05)
				File->Type = GSM_File_Video_3GP;
			else if (msg.Buffer[i]==0x10 && msg.Buffer[i+2]==0x01)
				File->Type = GSM_File_Java_JAR;
#ifdef DEVELOP
			else if (msg.Buffer[i]==0x00 && msg.Buffer[i+2]==0x01)
				File->Type = GSM_File_MMS;
#endif
		}
		return ERR_NONE;
	case 0x2F:
		smprintf(s,"File or folder used bytes received\n");
		File->Used = msg.Buffer[6]*256*256*256+
			     msg.Buffer[7]*256*256+
			     msg.Buffer[8]*256+
			     msg.Buffer[9];
		return ERR_NONE;
	case 0x33:
		if (s->Phone.Data.RequestID == ID_GetFileInfo) {
			if (Priv->FilesLocationsUsed + (msg.Buffer[8]*256+msg.Buffer[9]) > 1000) return ERR_MOREMEMORY;
			i = Priv->FilesLocationsUsed-1;
			while (1) {
				if (i==-1) break;
				smprintf(s, "Copying %i to %i, max %i\n",
					i,i+(msg.Buffer[8]*256+msg.Buffer[9]),
					Priv->FilesLocationsUsed);
				memcpy(&Priv->Files[i+(msg.Buffer[8]*256+msg.Buffer[9])],&Priv->Files[i],sizeof(GSM_File));
				i--;
			}
			if (Priv->FilesLocationsUsed + (msg.Buffer[8]*256+msg.Buffer[9]) >= GSM_PHONE_MAXSMSINFOLDER) {
				smprintf(s,"Too small buffer for folder data\n");
				Priv->filesystem2error  = ERR_MOREMEMORY;
				Priv->FilesEnd 		= true;
				return ERR_MOREMEMORY;
			}
			Priv->FilesLocationsUsed += (msg.Buffer[8]*256+msg.Buffer[9]);
			for (i=0;i<(msg.Buffer[8]*256+msg.Buffer[9]);i++) {
				sprintf(buffer,"%i",msg.Buffer[13+i*4-1]*256 + msg.Buffer[13+i*4]);
				EncodeUnicode(Priv->Files[i].ID_FullName,buffer,strlen(buffer));
				Priv->Files[i].Level = File->Level+1;
				smprintf(s, "%s ",DecodeUnicodeString(Priv->Files[i].ID_FullName));
			}
			smprintf(s, "\n");
		}
		if ((msg.Buffer[8]*256+msg.Buffer[9]) != 0x00) File->Folder = true;
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetFileFolderInfo1(GSM_StateMachine *s, GSM_File *File, bool full)
{
	GSM_Error	       	error;
	unsigned char	   	req[10] = {
		N7110_FRAME_HEADER,
		0x14,	   	/* 0x14 - info, 0x22 - free/total, 0x2E - used, 0x32 - sublocations */
		0x01,	   	/* 0x00 for sublocations reverse sorting, 0x01 for free */
		0x00, 0x00, 0x01,
		0x00, 0x01};    /* Folder or file number */

	s->Phone.Data.FileInfo  = File;
	req[8]		  	= atoi(DecodeUnicodeString(File->ID_FullName)) / 256;
	req[9]		  	= atoi(DecodeUnicodeString(File->ID_FullName)) % 256;

	req[3] = 0x14;
	req[4] = 0x01;
	smprintf(s,"Getting info for file in filesystem\n");
	error=GSM_WaitFor (s, req, 10, 0x6D, 4, ID_GetFileInfo);
	if (error != ERR_NONE) return error;

	if (full) {
		req[3] = 0x32;
		req[4] = 0x00;
		smprintf(s,"Getting subfolders for filesystem\n");
		error=GSM_WaitFor (s, req, 10, 0x6D, 4, ID_GetFileInfo);
		if (error != ERR_NONE) return error;
	}

	if (!File->Folder) {
		req[3] = 0x2E;
		req[4] = 0x01;
		smprintf(s,"Getting used memory for file in filesystem\n");
		return GSM_WaitFor (s, req, 10, 0x6D, 4, ID_GetFileInfo);
	}

	return error;
}

static GSM_Error N6510_GetNextFileFolder1(GSM_StateMachine *s, GSM_File *File, bool start)
{
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	GSM_Error	       	error;
	int			i;
	unsigned char		buffer[5];

	if (start) {
		Priv->FilesLocationsUsed = 1;

		sprintf(buffer,"%i",0x01);
		EncodeUnicode(Priv->Files[0].ID_FullName,buffer,strlen(buffer));
		Priv->Files[0].Level = 1;
	}

	while (1) {
		if (Priv->FilesLocationsUsed == 0) return ERR_EMPTY;

		CopyUnicodeString(File->ID_FullName,Priv->Files[0].ID_FullName);
		File->Level = Priv->Files[0].Level;

		for (i=0;i<Priv->FilesLocationsUsed;i++) {
			memcpy(&Priv->Files[i],&Priv->Files[i+1],sizeof(GSM_File));
		}
		Priv->FilesLocationsUsed--;

		error = N6510_GetFileFolderInfo1(s, File, true);
		if (error == ERR_EMPTY) continue;
		return error;
	}
}

GSM_Error N6510_ReplyGetFileSystemStatus1(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
	case 0x23:
		if (!strcmp(s->Phone.Data.ModelInfo->model,"6310i")) {
			smprintf(s,"File or folder total bytes received\n");
			s->Phone.Data.FileSystemStatus->Free =
				3*256*256 + msg.Buffer[8]*256 + msg.Buffer[9] -
				s->Phone.Data.FileSystemStatus->Used;
		} else {
			smprintf(s,"File or folder free bytes received\n");
			s->Phone.Data.FileSystemStatus->Free =
					msg.Buffer[6]*256*256*256+
					msg.Buffer[7]*256*256+
					msg.Buffer[8]*256+
					msg.Buffer[9];
		}
		return ERR_NONE;
	case 0x2F:
		smprintf(s,"File or folder used bytes received\n");
		s->Phone.Data.FileSystemStatus->Used =
				msg.Buffer[6]*256*256*256+
				msg.Buffer[7]*256*256+
				msg.Buffer[8]*256+
				msg.Buffer[9];
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetFileSystemStatus1(GSM_StateMachine *s, GSM_FileSystemStatus *status)
{
	GSM_Error	       	error;
	unsigned char	   	req[10] = {
		N7110_FRAME_HEADER,
		0x22,	   	/* 0x14 - info, 0x22 - free/total, 0x2E - used, 0x32 - sublocations */
		0x01,	   	/* 0x00 for sublocations reverse sorting, 0x01 for free */
		0x00, 0x00, 0x01,
		0x00, 0x01};    /* Folder or file number */

	/* Used memory by types */
	status->UsedImages = 0;
	status->UsedSounds = 0;
	status->UsedThemes = 0;
	s->Phone.Data.FileSystemStatus = status;

	status->Free = 0;

	req[3] = 0x2E;
	req[4] = 0x01;
	smprintf(s, "Getting used/total memory in filesystem\n");
	error = GSM_WaitFor (s, req, 10, 0x6D, 4, ID_FileSystemStatus);
	if (error != ERR_NONE) return error;

	req[3] = 0x22;
	req[4] = 0x01;
	smprintf(s, "Getting free memory in filesystem\n");
	return GSM_WaitFor (s, req, 10, 0x6D, 4, ID_FileSystemStatus);
}

static GSM_Error N6510_GetFilePart1(GSM_StateMachine *s, GSM_File *File, int *Handle UNUSED, int *Size)
{
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	int		     	old;
	GSM_Error	       	error;
	unsigned char	   	req[] = {
		N7110_FRAME_HEADER, 0x0E, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x01,	     	/* Folder or file number */
		0x00, 0x00, 0x00, 0x00, /* Start from xxx byte */
		0x00, 0x00,
		0x03, 0xE8};	    	/* Read xxx bytes */

	if (File->Used == 0x00) {
		error = N6510_GetFileFolderInfo1(s, File, false);
		if (error != ERR_NONE) return error;

		if (File->Folder) return ERR_SHOULDBEFILE;

		(*Size) 	= File->Used;
		File->Used 	= 0;
	}

	old		 = File->Used;
	req[8]		 = atoi(DecodeUnicodeString(File->ID_FullName)) / 256;
	req[9]		 = atoi(DecodeUnicodeString(File->ID_FullName)) % 256;
	req[10]		 = old / (256*256*256);
	req[11]		 = old / (256*256);
	req[12]		 = old / 256;
	req[13]		 = old % 256;

	s->Phone.Data.File = File;
	smprintf(s, "Getting file part from filesystem\n");
	error=GSM_WaitFor (s, req, 18, 0x6D, 4, ID_GetFile);
	if (error != ERR_NONE) return error;
	if (File->Used - old != (0x03 * 256 + 0xE8)) {
		error = N6510_GetFileCRC1(s, File->ID_FullName);
		if (error != ERR_NONE) return error;

		if (N6510_FindFileCheckSum12(s, File->Buffer, File->Used) != Priv->FileCheckSum) {
			smprintf(s,"File2 checksum is %i, File checksum is %i\n",N6510_FindFileCheckSum12(s, File->Buffer, File->Used),Priv->FileCheckSum);
			return ERR_WRONGCRC;
		}
		return ERR_EMPTY;
	}
	return ERR_NONE;
}

static GSM_Error N6510_SetReadOnly1(GSM_StateMachine *s, unsigned char *ID, bool enable)
{
	unsigned char SetAttr[] = {
		N7110_FRAME_HEADER, 0x18,
		0x00, 		    /* state */
		0x00, 0x00, 0x01,
		0x00, 0x20};	    /* File ID */

	if (!enable) SetAttr[4] = 0x06;

	SetAttr[8] = atoi(DecodeUnicodeString(ID)) / 256;
	SetAttr[9] = atoi(DecodeUnicodeString(ID)) % 256;
	smprintf(s, "Setting readonly attribute\n");
	return GSM_WaitFor (s, SetAttr, 10, 0x6D, 4, ID_SetAttrib);
}

static GSM_Error N6510_SetFileAttributes1(GSM_StateMachine *s, GSM_File *File)
{
	GSM_Error error;
	GSM_File  file2;

	memset(&file2, 0, sizeof(file2));

	CopyUnicodeString(file2.ID_FullName,File->ID_FullName);
	error = N6510_GetFileFolderInfo1(s, &file2, false);
	if (error != ERR_NONE) return error;

	/*  setting folder attrib works, but we block it */
	if (file2.Folder) return ERR_SHOULDBEFILE;

	/*  todo */
	if (file2.System != File->System ||
	    file2.Hidden != File->Hidden ||
	    file2.Protected != File->Protected) {
		return ERR_NOTSUPPORTED;
	}

	return N6510_SetReadOnly1(s, File->ID_FullName, File->ReadOnly);
}

/* function checks if there is file/folder with searched name in folder with specified ID */
static GSM_Error N6510_SearchForFileName1(GSM_StateMachine *s, GSM_File *File)
{
	GSM_Error	       	error;
	GSM_File		Files[400], Files2[400];
	int		     	FilesLocationsUsed,FilesLocationsUsed2,i;
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;

	File->Folder = false;

	/* making backup */
	if (Priv->FilesLocationsUsed > 400) return ERR_MOREMEMORY;
	memcpy(Files, Priv->Files, sizeof(GSM_File)*400);
	FilesLocationsUsed = Priv->FilesLocationsUsed;

	/* putting own data */
	Priv->Files[0].Level    	= 1;
	Priv->FilesLocationsUsed 	= 1;
	CopyUnicodeString(Priv->Files[0].ID_FullName,File->ID_FullName);

	/* checking */
	error = N6510_GetFileFolderInfo1(s, &Priv->Files[0], true);

	/* backuping new data */
	if (Priv->FilesLocationsUsed > 400) return ERR_MOREMEMORY;
	memcpy(Files2, Priv->Files, sizeof(GSM_File)*400);
	FilesLocationsUsed2 = Priv->FilesLocationsUsed;

	/* restoring */
	memcpy(Priv->Files, Files, sizeof(GSM_File)*400);
	Priv->FilesLocationsUsed = FilesLocationsUsed;

	if (error != ERR_NONE) return error;

	for (i=0;i<FilesLocationsUsed2-1;i++) {
		smprintf(s, "ID is %s\n",DecodeUnicodeString(Files2[i].ID_FullName));
		error = N6510_GetFileFolderInfo1(s, &Files2[i], false);
		if (error == ERR_EMPTY) continue;
		if (error != ERR_NONE) return error;
		smprintf(s, "%s",DecodeUnicodeString(File->Name));
		smprintf(s, "%s \n",DecodeUnicodeString(Files2[i].Name));
		if (mywstrncasecmp(Files2[i].Name,File->Name,0)) {
			smprintf(s, "the same\n");
			File->Folder = Files2[i].Folder;
			return ERR_NONE;
		}
	}
	return ERR_EMPTY;
}

GSM_Error N6510_ReplyAddFileHeader1(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char buffer[5];

	switch (msg.Buffer[3]) {
	case 0x03:
		smprintf(s,"File header added\n");
		sprintf(buffer,"%i",msg.Buffer[8]*256+msg.Buffer[9]);
		EncodeUnicode(s->Phone.Data.File->ID_FullName,buffer,strlen(buffer));
		return ERR_NONE;
	case 0x13:
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error N6510_ReplyAddFilePart1(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s UNUSED)
{
	return ERR_NONE;
}

static GSM_Error N6510_AddFilePart1(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle UNUSED)
{
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	GSM_File		File2;
	GSM_Error	       	error;
	int		     	j;
	unsigned char	   	Header[20 + (2 * (GSM_MAX_FILENAME_ID_LENGTH + 1))] = {
		N7110_FRAME_HEADER, 0x02, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x0C,	     /* parent folder ID */
		0x00, 0x00, 0x00, 0xE8};
	unsigned char	   	Add[15000] = {
		N7110_FRAME_HEADER, 0x40, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x04,	     /* file ID */
		0x00, 0x00,
		0x01, 0x28};	     /* length */
	unsigned char 		end[30] = {
		N7110_FRAME_HEADER, 0x40, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x04,	     /* file ID */
		0x00, 0x00, 0x00, 0x00};

	memset(&File2, 0, sizeof(File2));

	s->Phone.Data.File = File;

	if (*Pos == 0) {
		error = N6510_SearchForFileName1(s,File);
		if (error == ERR_NONE) return ERR_FILEALREADYEXIST;
		if (error != ERR_EMPTY) return error;

		Header[8] = atoi(DecodeUnicodeString(File->ID_FullName)) / 256;
		Header[9] = atoi(DecodeUnicodeString(File->ID_FullName)) % 256;
		memset(Header+14, 0x00, 300);
		CopyUnicodeString(Header+14,File->Name);
		Header[222] = File->Used / (256*256*256);
		Header[223] = File->Used / (256*256);
		Header[224] = File->Used / 256;
		Header[225] = File->Used % 256;
		switch(File->Type) {
			case GSM_File_Image_JPG    : Header[231]=0x02; Header[233]=0x01; break;
			case GSM_File_Image_BMP    : Header[231]=0x02; Header[233]=0x02; break;
			case GSM_File_Image_PNG    : Header[231]=0x02; Header[233]=0x03; break;
			case GSM_File_Image_GIF    : Header[231]=0x02; Header[233]=0x05; break;
			case GSM_File_Image_WBMP   : Header[231]=0x02; Header[233]=0x09; break;
			case GSM_File_Sound_AMR    : Header[231]=0x04; Header[233]=0x01; break;
			case GSM_File_Sound_MIDI   : Header[231]=0x04; Header[233]=0x05; break; /* Header[238]=0x01; */
			case GSM_File_Sound_NRT    : Header[231]=0x04; Header[233]=0x06; break;
			case GSM_File_Video_3GP    : Header[231]=0x08; Header[233]=0x05; break;
			case GSM_File_Java_JAR     : Header[231]=0x10; Header[233]=0x01; break;
#ifdef DEVELOP
			case GSM_File_MMS:
				Header[214]=0x07;
				Header[215]=0xd3;
				Header[216]=0x06;
				Header[217]=0x01;
				Header[218]=0x12;
				Header[219]=0x13;
				Header[220]=0x29;
				Header[233]=0x01;
				break;
#endif
			default		    : Header[231]=0x01; Header[233]=0x05;
		}
		Header[235] = 0x01;
		Header[236] = atoi(DecodeUnicodeString(File->ID_FullName)) / 256;
		Header[237] = atoi(DecodeUnicodeString(File->ID_FullName)) % 256;
		if (File->Protected) Header[238] = 0x01; /* Nokia forward lock */
		if (File->Hidden)    Header[241] = 0x01;
		if (File->System)    Header[242] = 0x01; /* fixme */
		smprintf(s, "Adding file header\n");
		error=GSM_WaitFor (s, Header, 246, 0x6D, 4, ID_AddFile);
		if (error != ERR_NONE) return error;
	}

	j = 1000;
	if (File->Used - *Pos < 1000) j = File->Used - *Pos;
	Add[ 8] = atoi(DecodeUnicodeString(File->ID_FullName)) / 256;
	Add[ 9] = atoi(DecodeUnicodeString(File->ID_FullName)) % 256;
	Add[12] = j / 256;
	Add[13] = j % 256;
	memcpy(Add+14,File->Buffer+(*Pos),j);
	smprintf(s, "Adding file part %i %i\n",*Pos,j);
	error=GSM_WaitFor (s, Add, 14+j, 0x6D, 4, ID_AddFile);
	if (error != ERR_NONE) return error;
	*Pos = *Pos + j;

	if (j < 1000) {
		/* FIXME: This looks strange */
		end[8] = atoi(DecodeUnicodeString(File->ID_FullName)) / 256;
		end[9] = atoi(DecodeUnicodeString(File->ID_FullName)) % 256;
		smprintf(s, "Frame for ending adding file\n");
		error = GSM_WaitFor (s, end, 14, 0x6D, 4, ID_AddFile);
		if (error != ERR_NONE) return error;

		CopyUnicodeString(File2.ID_FullName,File->ID_FullName);
		error = N6510_GetFileFolderInfo1(s, &File2, false);
		if (error != ERR_NONE) return error;

		if (!File->ModifiedEmpty) {
			Header[3]   = 0x12;
			Header[4]   = 0x01;
			Header[12]  = 0x00;
			Header[13]  = 0xE8;
			Header[8]   = atoi(DecodeUnicodeString(File->ID_FullName)) / 256;
			Header[9]   = atoi(DecodeUnicodeString(File->ID_FullName)) % 256;
			memset(Header+14, 0x00, 300);
			CopyUnicodeString(Header+14,File->Name);
			NOKIA_EncodeDateTime(s,Header+214,&File->Modified);
			/* When you save too big file for phone and it changes
			 * size (some part is cut by firmware), you HAVE to write
			 * here correct file size. In other case filesystem
			 * will be damaged
			 */
			Header[224] = File2.Used / 256;
			Header[225] = File2.Used % 256;
			Header[226] = Priv->FileToken / 256;
			Header[227] = Priv->FileToken % 256;
			switch(File->Type) {
				case GSM_File_Image_JPG    : Header[231]=0x02; Header[233]=0x01; break;
				case GSM_File_Image_BMP    : Header[231]=0x02; Header[233]=0x02; break;
				case GSM_File_Image_PNG    : Header[231]=0x02; Header[233]=0x03; break;
				case GSM_File_Image_GIF    : Header[231]=0x02; Header[233]=0x05; break;
				case GSM_File_Image_WBMP   : Header[231]=0x02; Header[233]=0x09; break;
				case GSM_File_Sound_AMR    : Header[231]=0x04; Header[233]=0x01; break;
				case GSM_File_Sound_MIDI   : Header[231]=0x04; Header[233]=0x05; break; /* Header[238]=0x01; */
				case GSM_File_Sound_NRT    : Header[231]=0x04; Header[233]=0x06; break;
				case GSM_File_Video_3GP    : Header[231]=0x08; Header[233]=0x05; break;
				case GSM_File_Java_JAR     : Header[231]=0x10; Header[233]=0x01; break;
#ifdef DEVELOP
				case GSM_File_MMS:
					Header[214]=0x07;
					Header[215]=0xd3;
					Header[216]=0x06;
					Header[217]=0x01;
					Header[218]=0x12;
					Header[219]=0x13;
					Header[220]=0x29;
					Header[233]=0x01;
					break;
#endif
				default		    : Header[231]=0x01; Header[233]=0x05;
			}
			Header[235] = 0x01;
			Header[236] = Priv->ParentID / 256;
			Header[237] = Priv->ParentID % 256;
			smprintf(s, "Adding file header\n");
			error=GSM_WaitFor (s, Header, 246, 0x6D, 4, ID_AddFile);
			if (error != ERR_NONE) return error;
		}

		/* Can't delete from phone menu */
		if (File->ReadOnly) {
			error = N6510_SetReadOnly1(s, File->ID_FullName, true);
			if (error != ERR_NONE) return error;
		}

		error = N6510_GetFileCRC1(s, File->ID_FullName);
		if (error != ERR_NONE) return error;

		if (N6510_FindFileCheckSum12(s, File->Buffer, File->Used) != Priv->FileCheckSum) {
			smprintf(s,"File2 checksum is %i, File checksum is %i\n",N6510_FindFileCheckSum12(s, File->Buffer, File->Used),Priv->FileCheckSum);
			return ERR_WRONGCRC;
		}

		return ERR_EMPTY;
	}

	return ERR_NONE;
}

GSM_Error N6510_ReplyDeleteFileFolder1(GSM_Protocol_Message msg, GSM_StateMachine *s UNUSED)
{
	if (msg.Buffer[4] == 0x01) {
		return ERR_NONE;
	} else if (msg.Buffer[4] == 0x04) {
		return ERR_FILENOTEXIST;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_PrivDeleteFileFolder1(GSM_StateMachine *s, unsigned char *ID, bool file)
{
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	GSM_File		File;
	GSM_Error       error;
	unsigned char   Delete[40] = {
		N7110_FRAME_HEADER, 0x1E, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x35};	    /* File ID */

	memset(&File, 0, sizeof(File));

	Priv->FilesLocationsUsed = 0;
	CopyUnicodeString(File.ID_FullName,ID);
	error = N6510_GetFileFolderInfo1(s, &File, true);
	if (error != ERR_NONE) return error;
	if (file) {
		if (File.Folder) return ERR_SHOULDBEFILE;
	} else {
		if (!File.Folder) return ERR_SHOULDBEFOLDER;
		/* dont allow to delete non empty folder */
		if (Priv->FilesLocationsUsed != 0) return ERR_FOLDERNOTEMPTY;
	}

	error = N6510_SetReadOnly1(s, ID, false);
	if (error != ERR_NONE) return error;

	/* FIXME: This looks wrong */
	Delete[8] = atoi(DecodeUnicodeString(ID)) / 256;
	Delete[9] = atoi(DecodeUnicodeString(ID)) % 256;

	return GSM_WaitFor (s, Delete, 10, 0x6D, 4, ID_DeleteFile);
}

static GSM_Error N6510_DeleteFile1(GSM_StateMachine *s, unsigned char *ID)
{
	return N6510_PrivDeleteFileFolder1(s,ID,true);
}

static GSM_Error N6510_DeleteFolder1(GSM_StateMachine *s, unsigned char *ID)
{
	return N6510_PrivDeleteFileFolder1(s,ID,false);
}

GSM_Error N6510_ReplyAddFolder1(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char buffer[5];

	sprintf(buffer,"%i",msg.Buffer[8]*256+msg.Buffer[9]);
	EncodeUnicode(s->Phone.Data.File->ID_FullName,buffer,strlen(buffer));
	return ERR_NONE;
}

static GSM_Error N6510_AddFolder1(GSM_StateMachine *s, GSM_File *File)
{
	GSM_File	File2;
	GSM_Error       error;
	unsigned char   Header[20 + (2 * (GSM_MAX_FILENAME_ID_LENGTH + 1))] = {
		N7110_FRAME_HEADER, 0x04, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x0C,	     /* parent folder ID */
		0x00, 0x00, 0x00, 0xE8};

	memset(&File2, 0, sizeof(File2));

	CopyUnicodeString(File2.ID_FullName,File->ID_FullName);
	error = N6510_GetFileFolderInfo1(s, &File2, false);
	if (error != ERR_NONE) return error;
	if (!File2.Folder) return ERR_SHOULDBEFOLDER;

	Header[8] = atoi(DecodeUnicodeString(File->ID_FullName)) / 256;
	Header[9] = atoi(DecodeUnicodeString(File->ID_FullName)) % 256;
	memset(Header+14, 0x00, 300);
	CopyUnicodeString(Header+14,File->Name);
	Header[233] = 0x02;
	Header[235] = 0x01;
	Header[236] = atoi(DecodeUnicodeString(File->ID_FullName)) / 256;
	Header[237] = atoi(DecodeUnicodeString(File->ID_FullName)) % 256;

	s->Phone.Data.File = File;
	smprintf(s, "Adding folder\n");
	error = GSM_WaitFor (s, Header, 246, 0x6D, 4, ID_AddFolder);
	if (error != ERR_NONE) return error;

	if (!strcmp(DecodeUnicodeString(File->ID_FullName),"0")) return ERR_FILEALREADYEXIST;

	/* Can't delete from phone menu */
	if (File->ReadOnly) {
		error = N6510_SetReadOnly1(s, File->ID_FullName, true);
		if (error != ERR_NONE) return error;
	}

	return error;
}

static GSM_Error N6510_GetFolderListing1(GSM_StateMachine *s, GSM_File *File, bool start)
{
	GSM_Error	       	error;
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	int			i;

	if (start) {
		Priv->FilesLocationsUsed = 0;

		error = N6510_GetFileFolderInfo1(s, File, true);
		if (error != ERR_NONE) return error;

		if (!File->Folder) return ERR_SHOULDBEFOLDER;
	}

	while (true) {
		if (Priv->FilesLocationsUsed == 0) return ERR_EMPTY;

		memcpy(File,&Priv->Files[0],sizeof(GSM_File));
		error = N6510_GetFileFolderInfo1(s, File, false);

		for(i=1;i<Priv->FilesLocationsUsed;i++) {
			memcpy(&Priv->Files[i-1],&Priv->Files[i],sizeof(GSM_File));
		}
		Priv->FilesLocationsUsed--;

		/* 3510 for example */
		if (error == ERR_EMPTY) continue;

		break;
	}
	return error;
}

/* filesystem 2 */

GSM_Error N6510_ReplyOpenFile2(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	if (msg.Buffer[4]==0) {
		smprintf(s,"File opened and handle received\n");
		s->Phone.Data.FileHandle = msg.Buffer[6]*256*256*256+
				       msg.Buffer[7]*256*256+
				       msg.Buffer[8]*256+
				       msg.Buffer[9];
		smprintf(s,"File handle: %i\n",
				msg.Buffer[6]*256*256*256+
				msg.Buffer[7]*256*256+
				msg.Buffer[8]*256+
				msg.Buffer[9]);
		return ERR_NONE;
	} else if (msg.Buffer[4] == 0x03) {
		smprintf(s,"You can't open already existing folder\n");
		return ERR_FILEALREADYEXIST;
	} else if (msg.Buffer[4] == 0x06) {
		smprintf(s,"File not exist\n");
		return ERR_FILENOTEXIST;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_OpenFile2(GSM_StateMachine *s, char *Name, int *Handle, bool Create)
{
	unsigned char	 req[20 + (2 * (GSM_MAX_FILENAME_ID_LENGTH + 1))] =
		{N6110_FRAME_HEADER, 0x72,
				     0x00,	  /*  mode 0 - open read only, 0x11 - read write create */
				     0x02,
				     0xFF, 0xFF}; /*  name length */
	int		 Pos = 8;
	GSM_Error	 error;

	if (Create) req[4] = 0x11;
	req[6] = (UnicodeLength(Name)*2 + 2)/ 256 ;
	req[7] = (UnicodeLength(Name)*2 + 2)% 256 ;
	CopyUnicodeString(req+8,Name);
	if (req[9] == 'a' || req[9] == 'A') req[9] = 'b';
	if (req[9] == 'd' || req[9] == 'D') req[9] = 'a';
	Pos+=UnicodeLength(Name)*2;
	req[Pos++] = 0;
	req[Pos++] = 0;

	smprintf(s, "Opening file\n");
	error = GSM_WaitFor (s, req, Pos, 0x6D, 4, ID_OpenFile);
	if (error==ERR_NONE) (*Handle) = s->Phone.Data.FileHandle;
	return error;
}

static GSM_Error N6510_CloseFile2(GSM_StateMachine *s, int *Handle)
{
	unsigned char	 req[200] = {N6110_FRAME_HEADER, 0x74, 0x00, 0x00,
				     0x00, 0x00, 0x00, 0x00}; /* file handle */

	req[6]		 = (*Handle) / (256*256*256);
	req[7]		 = (*Handle) / (256*256);
	req[8]		 = (*Handle) / 256;
	req[9]		 = (*Handle) % 256;

	smprintf(s, "Closing file\n");
	return GSM_WaitFor (s, req, 10, 0x6D, 4, ID_CloseFile);
}

static GSM_Error N6510_GetFileCRC2(GSM_StateMachine *s, int *Handle)
{
	unsigned char req2[15000] = {
		N7110_FRAME_HEADER, 0x66, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00}; 	/*  handle */

	req2[6]	= (*Handle) / (256*256*256);
	req2[7]	= (*Handle) / (256*256);
	req2[8]	= (*Handle) / 256;
	req2[9]	= (*Handle) % 256;
	return GSM_WaitFor (s, req2, 10, 0x6D, 8, ID_GetCRC);
}

GSM_Error N6510_ReplyGetFileFolderInfo2(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	GSM_File		*FileInfo = s->Phone.Data.FileInfo;
	GSM_File		*File;
	int		     	i;

	switch (msg.Buffer[3]) {
	case 0x69:
	case 0x6D:
		switch (msg.Buffer[4]) {
		case 0x0C:
			smprintf(s,"Probably no MMC card\n");
			Priv->filesystem2error  = ERR_MEMORY;
			Priv->FilesEnd 		= true;
			return ERR_MEMORY;
		case 0x00:
		case 0x0D:
			switch (msg.Buffer[5]) {
			case 0x00:
				break;
			case 0x06:
				smprintf(s,"File not exist\n");
				return ERR_FILENOTEXIST;
			case 0x0C:
				smprintf(s,"Probably no MMC card\n");
				return ERR_MEMORY;
			default:
				smprintf(s,"unknown status code\n");
				return ERR_UNKNOWNRESPONSE;
			}
			smprintf(s,"File or folder details received\n");

			if (msg.Buffer[3] == 0x69) {
				if (Priv->FilesLocationsUsed + 1 >= GSM_PHONE_MAXSMSINFOLDER) {
					smprintf(s,"Too small buffer for folder data\n");
					Priv->filesystem2error  = ERR_MOREMEMORY;
					Priv->FilesEnd 		= true;
					return ERR_MOREMEMORY;
				}

				for (i=Priv->FilesLocationsUsed+1;i>0;i--) {
					memcpy(&Priv->Files[i],&Priv->Files[i-1],sizeof(GSM_File));
				}

				File = &Priv->Files[1];

				File->Level = Priv->Files[0].Level + 1;
				Priv->FilesLocationsUsed++;

				CopyUnicodeString(File->Name,msg.Buffer+32);
				smprintf(s,"\"%s\"\n",DecodeUnicodeString(File->Name));

				CopyUnicodeString(File->ID_FullName,FileInfo->ID_FullName);
				EncodeUnicode(File->ID_FullName+UnicodeLength(File->ID_FullName)*2,"/",1);
				CopyUnicodeString(File->ID_FullName+UnicodeLength(File->ID_FullName)*2,msg.Buffer+32);
				smprintf(s,"\"%s\"\n",DecodeUnicodeString(File->ID_FullName));
			} else {
				File = FileInfo;
			}

			if ((msg.Buffer[29] & 0x10) == 0x10) {
				File->Folder = true;
				smprintf(s,"Folder\n");
			} else {
				File->Folder = false;
				smprintf(s,"File\n");
				File->Used = msg.Buffer[10]*256*256*256+
					    msg.Buffer[11]*256*256+
					    msg.Buffer[12]*256+
					    msg.Buffer[13];
				smprintf(s,"Size %ld bytes\n", (long)File->Used);
			}
			File->ReadOnly = false;
			if ((msg.Buffer[29] & 1) == 1) {
				File->ReadOnly = true;
				smprintf(s,"Readonly\n");
			}
			File->Hidden = false;
			if ((msg.Buffer[29] & 2) == 2) {
				File->Hidden = true;
				smprintf(s,"Hidden\n");
			}
			File->System = false;
			if ((msg.Buffer[29] & 4) == 4) {
				File->System = true;
				smprintf(s,"System\n");
			}
			File->Protected = false;
			if ((msg.Buffer[29] & 0x40) == 0x40) {
				File->Protected = true;
				smprintf(s,"Protected\n");
			}

			File->ModifiedEmpty = false;
			NOKIA_DecodeDateTime(s, msg.Buffer+14, &File->Modified, true, false);
			if (File->Modified.Year == 0x00) File->ModifiedEmpty = true;
			if (File->Modified.Year == 0xffff) File->ModifiedEmpty = true;

			if (msg.Buffer[3] == 0x69 && msg.Buffer[4] == 0) Priv->FilesEnd = true;

			return ERR_NONE;
		case 0x06:
			smprintf(s,"File or folder details received - not available ?\n");
			Priv->filesystem2error  = ERR_FILENOTEXIST;
			Priv->FilesEnd 		= true;
			return ERR_FILENOTEXIST;
		case 0x0E:
			smprintf(s,"File or folder details received - empty\n");
			Priv->FilesEnd = true;
			return ERR_NONE;
		}
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetFileFolderInfo2(GSM_StateMachine *s, GSM_File *File)
{
	int		     	Pos=6;
	unsigned char	   	req[20 + (2 * (GSM_MAX_FILENAME_ID_LENGTH + 1))] = {
		N7110_FRAME_HEADER,0x6C,
		0xFF, 0xFF}; 			/*  name length */

	s->Phone.Data.FileInfo  = File;

	req[4] = (UnicodeLength(File->ID_FullName)*2 + 2)/256;
	req[5] = (UnicodeLength(File->ID_FullName)*2 + 2)%256;
	CopyUnicodeString(req+6,File->ID_FullName);
	if (req[7] == 'a' || req[7] == 'A') req[7] = 'b';
	if (req[7] == 'd' || req[7] == 'D') req[7] = 'a';
	Pos+=UnicodeLength(File->ID_FullName)*2;
	req[Pos++] = 0;
	req[Pos++] = 0;

	smprintf(s,"Getting info for file in filesystem\n");
	return GSM_WaitFor (s, req, Pos, 0x6D, 4, ID_GetFileInfo);
}

static GSM_Error N6510_PrivGetFolderListing2(GSM_StateMachine *s, GSM_File *File)
{
	GSM_Error		error;
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	unsigned char	   	req[20 + (2 * (GSM_MAX_FILENAME_ID_LENGTH + 1))] = {
		N6110_FRAME_HEADER, 0x68,
					    0xFF, 0xFF}; /*  name length */
	int			Pos = 6, i = 0;

	req[4] = (UnicodeLength(File->ID_FullName)*2 + 6)/ 256 ;
	req[5] = (UnicodeLength(File->ID_FullName)*2 + 6)% 256 ;
	CopyUnicodeString(req+6,File->ID_FullName);
	if (req[7] == 'a' || req[7] == 'A') req[7] = 'b';
	if (req[7] == 'd' || req[7] == 'D') req[7] = 'a';
	Pos+=UnicodeLength(File->ID_FullName)*2;
	req[Pos++] = 0;
	req[Pos++] = '/';
	req[Pos++] = 0;
	req[Pos++] = '*';
	req[Pos++] = 0;
	req[Pos++] = 0;

	smprintf(s, "Getting folder info %s\n",DecodeUnicodeString(File->ID_FullName));

	Priv->filesystem2error  = ERR_NONE;
	s->Phone.Data.FileInfo  = File;
	Priv->FilesEnd 		= false;
	error = s->Protocol.Functions->WriteMessage(s, req, Pos, 0x6D);
	if (error!=ERR_NONE) return error;

	while (!Priv->FilesEnd) {
		usleep(100000);
		if (GSM_ReadDevice(s,true)==0) i++; else i=0;
		if (i==3) {
			smprintf(s,"Connection broken or WELL KNOWN phone firmware problem (which makes, that not all files are reported)\n");
			Priv->filesystem2error  = ERR_FOLDERPART;
			return ERR_NONE;
		}
	}

	return ERR_NONE;
}

static GSM_Error N6510_GetNextFileFolder2(GSM_StateMachine *s, GSM_File *File, bool start)
{
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	GSM_Error	       	error;
	int		     	i;

	if (start) {
		Priv->FilesLocationsUsed = 2;

		Priv->Files[0].Level	= 1;
		Priv->Files[0].Folder	= true;
		Priv->Files[0].Level	= 1;
		EncodeUnicode(Priv->Files[0].ID_FullName,"d:",2);
		EncodeUnicode(Priv->Files[0].Name,"D (Permanent_memory 2)",22);

		Priv->Files[1].Level	= 1;
		Priv->Files[1].Folder	= true;
		Priv->Files[1].Level	= 1;
		EncodeUnicode(Priv->Files[1].ID_FullName,"a:",2);
		EncodeUnicode(Priv->Files[1].Name,"A (Memory card)",15);
	}

	smprintf(s, "entering %i\n",Priv->FilesLocationsUsed);
	if (Priv->FilesLocationsUsed == 0) return ERR_EMPTY;

	if (!Priv->Files[Priv->FilesLocationsUsed - 1].Folder) {
		memcpy(File,&Priv->Files[Priv->FilesLocationsUsed - 1],sizeof(GSM_File));
		Priv->FilesLocationsUsed--;
		return ERR_NONE;
	}

	error = N6510_PrivGetFolderListing2(s, &Priv->Files[0]);
	if (error != ERR_NONE) return error;

	memcpy(File,&Priv->Files[0],sizeof(GSM_File));
	for (i=0;i<Priv->FilesLocationsUsed-1;i++) {
		memcpy(&Priv->Files[i],&Priv->Files[i+1],sizeof(GSM_File));
	}
	Priv->FilesLocationsUsed--;

	if (Priv->filesystem2error == ERR_FOLDERPART) return ERR_FOLDERPART;

	return error;
}

static GSM_Error N6510_GetFilePart2(GSM_StateMachine *s, GSM_File *File, int *Handle, int *Size)
{
	int		    	old,j;
	GSM_Error	       	error;
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	unsigned char	   	req[] = {
		N7110_FRAME_HEADER, 0x5E, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x01,		/* file handle */
		0x00, 0x00, 0x00, 0x00, 	/* position */
		0x00, 0x00, 0x03, 0xE8, 	/* length */
		0x00, 0x00, 0x03, 0xE8};	/* buffer length */

	if (File->Used == 0x00) {
		error = N6510_GetFileFolderInfo2(s, File);
		if (error != ERR_NONE) return error;

		if (File->Folder) return ERR_SHOULDBEFILE;

		error = N6510_OpenFile2(s, File->ID_FullName, Handle, false);
		if (error != ERR_NONE) return error;

		for (j=UnicodeLength(File->ID_FullName)-1;j>0;j--) {
			if (File->ID_FullName[j*2+1] == '\\' || File->ID_FullName[j*2+1] == '/') break;
		}
		if (File->ID_FullName[j*2+1] == '\\' || File->ID_FullName[j*2+1] == '/') {
			CopyUnicodeString(File->Name,File->ID_FullName+j*2+2);
		} else {
			CopyUnicodeString(File->Name,File->ID_FullName);
		}

		(*Size) 	= File->Used;
		File->Used 	= 0;
	}

	req[6]		 = (*Handle) / (256*256*256);
	req[7]		 = (*Handle) / (256*256);
	req[8]		 = (*Handle) / 256;
	req[9]		 = (*Handle) % 256;

	old		 = File->Used;
	req[10]		 = old / (256*256*256);
	req[11]		 = old / (256*256);
	req[12]		 = old / 256;
	req[13]		 = old % 256;

	s->Phone.Data.File      = File;
	smprintf(s, "Getting file part from filesystem\n");
	error=GSM_WaitFor (s, req, 22, 0x6D, 4, ID_GetFile);
	if (error != ERR_NONE) return error;

	if (File->Used - old != (0x03 * 256 + 0xE8)) {
		error = N6510_GetFileCRC2(s, Handle);
		if (error != ERR_NONE) return error;

		error = N6510_CloseFile2(s, Handle);
		if (error != ERR_NONE) return error;

		if (N6510_FindFileCheckSum12(s, File->Buffer, File->Used) != Priv->FileCheckSum) {
			smprintf(s,"File2 checksum is %i, File checksum is %i\n",N6510_FindFileCheckSum12(s, File->Buffer, File->Used),Priv->FileCheckSum);
			return ERR_WRONGCRC;
		}

		return ERR_EMPTY;
	}
	return ERR_NONE;
}

GSM_Error N6510_ReplySetFileDate2(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s UNUSED)
{
	return ERR_NONE;
}

GSM_Error N6510_ReplySetAttrib2(GSM_Protocol_Message msg, GSM_StateMachine *s UNUSED)
{
	if (msg.Buffer[4] == 0x00) {
		return ERR_NONE;
	} else if (msg.Buffer[4] == 0x06) {
		return ERR_FILENOTEXIST;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_SetFileAttributes2(GSM_StateMachine *s, GSM_File *File)
{
	int		P = 10;
	GSM_Error	error;
	GSM_File	File2;
	unsigned char	Header2[20 + (2 * (GSM_MAX_FILENAME_ID_LENGTH + 1))] = {
		N7110_FRAME_HEADER, 0x6E,
		0x00, 0x0c};			/* name len */

	memcpy(&File2,File,sizeof(GSM_File));

	error = N6510_GetFileFolderInfo2(s, File);
	if (error != ERR_NONE) return error;

	/* haven't checked. */
	if (File->Folder) return ERR_SHOULDBEFILE;

	Header2[4] = (UnicodeLength(File2.ID_FullName) + 1)/ 256 ;
	Header2[5] = (UnicodeLength(File2.ID_FullName) + 1)% 256 ;
	Header2[6] = 0x00;
	Header2[7] = 0x00;
	Header2[8] = 0x00;
	Header2[9] = 0x00;
	if (File2.ReadOnly)  Header2[9] += 1;
	if (File2.Hidden)    Header2[9] += 2;
	if (File2.System)    Header2[9] += 4;
	if (File2.Protected) Header2[9] += 0x40;
	CopyUnicodeString(Header2+10,File2.ID_FullName);
	if (Header2[11] == 'a' || Header2[11] == 'A') Header2[11] = 'b';
	if (Header2[11] == 'd' || Header2[11] == 'D') Header2[11] = 'a';
	P+=UnicodeLength(File2.ID_FullName)*2;
	Header2[P++] = 0;
	Header2[P++] = 0;
	error = GSM_WaitFor (s, Header2, P, 0x6D, 4, ID_SetAttrib);
	if (error != ERR_NONE) return error;

	error = N6510_GetFileFolderInfo2(s, File);
	if (error != ERR_NONE) return error;

	/* mmc doesn't support protected */
	if (File2.System    != File->System     ||
	    File2.ReadOnly  != File->ReadOnly   ||
	    File2.Hidden    != File->Hidden     ) {
/* 	    File2.Protected != File->Protected) { */
		return ERR_NOTSUPPORTED;
	}

	return ERR_NONE;
}

static GSM_Error N6510_AddFilePart2(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle)
{
	GSM_Error	       	error;
	int		     	j,P;
/* 	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510; */
/* 	unsigned char		buffer[500]; */
	unsigned char	   	req[15000] = {
		N7110_FRAME_HEADER, 0x58, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 	/*  handle */
		0x00, 0x00, 0x04, 0x00};	/*  buffer len */
	unsigned char	   	Header[20 + (2 * (GSM_MAX_FILENAME_ID_LENGTH + 1))] = {
		N7110_FRAME_HEADER, 0x86,
		0x00, 0x0c};			/* name len */

	s->Phone.Data.File = File;

	if (*Pos == 0) {
		EncodeUnicode(File->ID_FullName+UnicodeLength(File->ID_FullName)*2,"/",1);
		CopyUnicodeString(File->ID_FullName+UnicodeLength(File->ID_FullName)*2,File->Name);

		error = N6510_GetFileFolderInfo2(s, File);
		switch (error) {
		case ERR_FILENOTEXIST:
			break;
		case ERR_NONE:
			return ERR_FILEALREADYEXIST;
		default:
			return error;
		}

		error = N6510_OpenFile2(s, File->ID_FullName, Handle, true);
		if (error != ERR_NONE) return error;
	}

	req[6]		 = (*Handle) / (256*256*256);
	req[7]		 = (*Handle) / (256*256);
	req[8]		 = (*Handle) / 256;
	req[9]		 = (*Handle) % 256;

	j = 2000;
	if (File->Used - *Pos < 2000) j = File->Used - *Pos;
	req[10]		 = j / (256*256*256);
	req[11]		 = j / (256*256);
	req[12]		 = j / 256;
	req[13]		 = j % 256;
	memcpy(req+14,File->Buffer+(*Pos),j);

	smprintf(s, "Adding file part %i %i\n",*Pos,j);
	error=GSM_WaitFor (s, req, 14+j, 0x6D, 4, ID_AddFile);
	if (error != ERR_NONE) return error;
	*Pos = *Pos + j;

	if (j < 2000) {
		error = N6510_CloseFile2(s, Handle);
		if (error != ERR_NONE) return error;

		P = 14;
		Header[4] = (UnicodeLength(File->ID_FullName) + 1)/ 256 ;
		Header[5] = (UnicodeLength(File->ID_FullName) + 1)% 256 ;
		Header[6] = File->Modified.Year / 256;
		Header[7] = File->Modified.Year % 256;
		Header[8] = File->Modified.Month;
		Header[9] = File->Modified.Day;
		Header[10] = 0x00;
		Header[11] = File->Modified.Hour;
		Header[12] = File->Modified.Minute;
		Header[13] = File->Modified.Second;
		CopyUnicodeString(Header+14,File->ID_FullName);
	 	if (Header[15] == 'a' || Header[15] == 'A') Header[15] = 'b';
		if (Header[15] == 'd' || Header[15] == 'D') Header[15] = 'a';
		P+=UnicodeLength(File->ID_FullName)*2;
		req[P++] = 0;
		req[P++] = 0;
		smprintf(s,"Setting file date\n");
		error = GSM_WaitFor (s, Header, P, 0x6D, 4, ID_AddFile);
		if (error != ERR_NONE) return error;

		error = N6510_SetFileAttributes2(s,File);
		if (error != ERR_NONE) return error;

/* 		error = N6510_OpenFile2(s, File->ID_FullName, Handle, false); */
/* 		if (error != ERR_NONE) return error; */
/* 		if ((*Handle) == 0) { */
/* 			error = N6510_OpenFile2(s, File->ID_FullName, Handle, false); */
/* 			if (error != ERR_NONE) return error; */
/* 		} */
/* 		error = N6510_GetFileCRC2(s, Handle); */
/* 		if (error != ERR_NONE) return error; */
/* 		error = N6510_CloseFile2(s, Handle); */
/* 		if (error != ERR_NONE) return error; */
/* 		if (N6510_FindFileCheckSum12(s, File->Buffer, File->Used) != Priv->FileCheckSum) { */
/* 			smprintf(s,"File2 checksum is %i, File checksum is %i\n",N6510_FindFileCheckSum12(s, File->Buffer, File->Used),Priv->FileCheckSum); */
/* 			return ERR_WRONGCRC; */
/* 		} */

		return ERR_EMPTY;
	}

	return ERR_NONE;
}

static GSM_Error N6510_GetFolderListing2(GSM_StateMachine *s, GSM_File *File, bool start)
{
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	GSM_Error 		error;
	int			i;

	if (start) {
		if (strcasecmp(DecodeUnicodeString(File->ID_FullName),"a:") == 0   ||
		    strcasecmp(DecodeUnicodeString(File->ID_FullName),"a:\\") == 0 ||
		    strcasecmp(DecodeUnicodeString(File->ID_FullName),"d:") == 0   ||
		    strcasecmp(DecodeUnicodeString(File->ID_FullName),"d:\\") == 0) {
		} else {
			/* we must check, if user gave folder name or not */
			error = N6510_GetFileFolderInfo2(s, File);
			if (error != ERR_NONE) return error;
			if (!File->Folder) return ERR_SHOULDBEFOLDER;
		}

		Priv->FilesLocationsUsed = 1;

		error = N6510_PrivGetFolderListing2(s, File);
		if (error != ERR_NONE) return error;

		memcpy(File,&Priv->Files[0],sizeof(GSM_File));
		for (i=0;i<Priv->FilesLocationsUsed-1;i++) {
			memcpy(&Priv->Files[i],&Priv->Files[i+1],sizeof(GSM_File));
		}
		Priv->FilesLocationsUsed--;
	}

	if (Priv->FilesLocationsUsed == 0) return ERR_EMPTY;

	memcpy(File,&Priv->Files[0],sizeof(GSM_File));
	for (i=0;i<Priv->FilesLocationsUsed-1;i++) {
		memcpy(&Priv->Files[i],&Priv->Files[i+1],sizeof(GSM_File));
	}
	Priv->FilesLocationsUsed--;
	if (start) {
		if (Priv->filesystem2error == ERR_FOLDERPART) return ERR_FOLDERPART;
	}
	return ERR_NONE;
}

GSM_Error N6510_ReplyDeleteFile2(GSM_Protocol_Message msg, GSM_StateMachine *s UNUSED)
{
	if (msg.Buffer[4] == 0x00) {
		return ERR_NONE;
	} else if (msg.Buffer[4] == 0x03) {
		/* trying to delete read only */
		return ERR_UNKNOWN;
	} else if (msg.Buffer[4] == 0x06) {
		return ERR_FILENOTEXIST;
	}

	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_DeleteFile2(GSM_StateMachine *s, unsigned char *ID)
{
	unsigned char   req[20 + (2 * (GSM_MAX_FILENAME_ID_LENGTH + 1))] = {
		N7110_FRAME_HEADER, 0x62};
	int		Pos = 6;
	GSM_File	file;
	GSM_Error	error;

	/* first remove readonly */
	file.ReadOnly  = false;
	file.Hidden    = false;
	file.System    = false;
	file.Protected = false;

	CopyUnicodeString(file.ID_FullName,ID);
	error = N6510_SetFileAttributes2(s,&file);
	if (error != ERR_NONE) return error;

	req[4] = (UnicodeLength(ID)*2 + 2)/ 256 ;
	req[5] = (UnicodeLength(ID)*2 + 2)% 256 ;
	CopyUnicodeString(req+6,ID);
	if (req[7] == 'a' || req[7] == 'A') req[7] = 'b';
	if (req[7] == 'd' || req[7] == 'D') req[7] = 'a';
	Pos+=UnicodeLength(ID)*2;
	req[Pos++] = 0;
	req[Pos++] = 0;

	smprintf(s,"Deleting file\n");
	return GSM_WaitFor (s, req, Pos, 0x6D, 4, ID_DeleteFile);
}

GSM_Error N6510_ReplyAddFolder2(GSM_Protocol_Message msg, GSM_StateMachine *s UNUSED)
{
	if (msg.Buffer[4] == 0x00) {
		return ERR_NONE;
	} if (msg.Buffer[4] == 0x04) {
		return ERR_FILEALREADYEXIST;
	} if (msg.Buffer[4] == 0x06) {
		return ERR_FILENOTEXIST;
	} if (msg.Buffer[4] == 0x0C) {
		return ERR_MEMORY;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_AddFolder2(GSM_StateMachine *s, GSM_File *File)
{
	GSM_Error	error;
	unsigned char   req[20 + (2 * (GSM_MAX_FILENAME_ID_LENGTH + 1))] = {
		N7110_FRAME_HEADER, 0x64};
	int		Pos = 6;
	int		Len = 0;

	Len = UnicodeLength(File->ID_FullName)*2 + 2;

	CopyUnicodeString(req+6,File->ID_FullName);
	Pos+=UnicodeLength(File->ID_FullName)*2;
	if (DecodeUnicodeString(File->ID_FullName)[UnicodeLength(File->ID_FullName)-1] != '\\' &&
	    DecodeUnicodeString(File->ID_FullName)[UnicodeLength(File->ID_FullName)-1] != '/') {
		req[Pos++] = 0;
		req[Pos++] = '/';
		Len += 2;
	}
	CopyUnicodeString(req+Pos,File->Name);
	if (req[Pos+1] == 'a' || req[Pos+1] == 'A') req[Pos+1] = 'b';
	if (req[Pos+1] == 'd' || req[Pos+1] == 'D') req[Pos+1] = 'a';
	Pos += UnicodeLength(File->Name)*2;
	Len += UnicodeLength(File->Name)*2;
	req[Pos++] = 0;
	req[Pos++] = 0;
	req[4] = Len / 256 ;
	req[5] = Len % 256 ;
	smprintf(s,"Adding folder\n");
	error=GSM_WaitFor (s, req, Pos, 0x6D, 4, ID_AddFolder);
	if (error == ERR_NONE) memcpy(File->ID_FullName,req+6,Pos);
	return error;
}

GSM_Error N6510_ReplyDeleteFolder2(GSM_Protocol_Message msg, GSM_StateMachine *s UNUSED)
{
	if (msg.Buffer[4] == 0x00) {
		return ERR_NONE;
	} if (msg.Buffer[4] == 0x03) {
		return ERR_SHOULDBEFOLDER;
	} if (msg.Buffer[4] == 0x06) {
		return ERR_FILENOTEXIST;
	} if (msg.Buffer[4] == 0x0C) {
		return ERR_MEMORY;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_DeleteFolder2(GSM_StateMachine *s, unsigned char *ID)
{
	GSM_File	File2;
	GSM_Error	error;
	unsigned char   req[20 + (2 * (GSM_MAX_FILENAME_ID_LENGTH + 1))] = {
		N7110_FRAME_HEADER, 0x6A};
	int		Pos = 6;

	/* we don't want to allow deleting non empty folders */
	CopyUnicodeString(File2.ID_FullName,ID);
	error = N6510_GetFolderListing2(s, &File2, true);
	switch (error) {
		case ERR_EMPTY:
			break;
		case ERR_NONE:
			return ERR_FOLDERNOTEMPTY;
		default:
			return error;
	}

	req[4] = (UnicodeLength(ID)*2 + 2)/ 256 ;
	req[5] = (UnicodeLength(ID)*2 + 2)% 256 ;
	CopyUnicodeString(req+6,ID);
	if (req[7] == 'a' || req[7] == 'A') req[7] = 'b';
	if (req[7] == 'd' || req[7] == 'D') req[7] = 'a';
	Pos+=UnicodeLength(ID)*2;
	req[Pos++] = 0;
	req[Pos++] = 0;

	smprintf(s,"Deleting folder\n");
	return GSM_WaitFor (s, req, Pos, 0x6D, 4, ID_DeleteFolder);
}

/* shared */

GSM_Error N6510_GetFolderListing(GSM_StateMachine *s, GSM_File *File, bool start)
{
	GSM_Error	error;
	GSM_File	File2;

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (DecodeUnicodeString(File->ID_FullName)[0] == 'c' ||
	    DecodeUnicodeString(File->ID_FullName)[0] == 'C') {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30) ||
		    GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILE1)) {
			return ERR_NOTSUPPORTED;
		}

		memcpy(&File2,File,sizeof(GSM_File));
		CopyUnicodeString(File2.ID_FullName,File->ID_FullName+3*2);
		error = N6510_GetFolderListing1(s,&File2,start);
		memcpy(File,&File2,sizeof(GSM_File));
		/* GetFolderListing changes ID */
		EncodeUnicode(File->ID_FullName,"c:/",3);
		CopyUnicodeString(File->ID_FullName+UnicodeLength(File->ID_FullName)*2,File2.ID_FullName);
		return error;
	} else {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
			return N6510_GetFolderListing2(s,File,start);
		} else {
			return ERR_NOTSUPPORTED;
		}
	}
}

GSM_Error N6510_GetNextFileFolder(GSM_StateMachine *s, GSM_File *File, bool start)
{
	GSM_Error	       	error;
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	char		    	buf[20 + (2 * (GSM_MAX_FILENAME_ID_LENGTH + 1))];

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (start) {
		Priv->Use2 = true;
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
			/* series 40 3.0 don't have filesystem 1 */
			Priv->Use2 = false;
		}
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILE1)) {
			Priv->Use2 = false;
		}
	}
	if (Priv->Use2) {
		error = N6510_GetNextFileFolder1(s,File,start);
		if (error == ERR_EMPTY) {
			if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
				return error;
			}
			Priv->Use2      = false;
			start	   	= true;
		} else {
			if (error == ERR_NONE) {
				sprintf(buf,"c:/%s",DecodeUnicodeString(File->ID_FullName));
				EncodeUnicode(File->ID_FullName,buf,strlen(buf));

				if (File->Level != 1) return error;

				buf[0] = 0;
				buf[1] = 0;
				CopyUnicodeString(buf,File->Name);
				EncodeUnicode(File->Name,"C (",3);
				CopyUnicodeString(File->Name+6,buf);
				EncodeUnicode(File->Name+UnicodeLength(File->Name)*2,")",1);
			}
			return error;
		}
	}
	return N6510_GetNextFileFolder2(s,File,start);
}

GSM_Error N6510_GetFilePart(GSM_StateMachine *s, GSM_File *File, int *Handle, int *Size)
{
	GSM_File	File2;
	char	    	buf[20 + (2 * (GSM_MAX_FILENAME_ID_LENGTH + 1))];
	GSM_Error       error;

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (DecodeUnicodeString(File->ID_FullName)[0] == 'c' ||
	    DecodeUnicodeString(File->ID_FullName)[0] == 'C') {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30) ||
		    GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILE1)) {
			return ERR_NOTSUPPORTED;
		}
		memcpy(&File2,File,sizeof(GSM_File));
		CopyUnicodeString(File2.ID_FullName,File->ID_FullName+3*2);
		error = N6510_GetFilePart1(s,&File2, Handle, Size);
		CopyUnicodeString(buf,File->ID_FullName);
		memcpy(File,&File2,sizeof(GSM_File));
		CopyUnicodeString(File->ID_FullName,buf);
		return error;
	} else {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
			return N6510_GetFilePart2(s,File, Handle, Size);
		} else {
			return ERR_NOTSUPPORTED;
		}
	}
}

GSM_Error N6510_AddFilePart(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle)
{
	GSM_File	File2;
	GSM_Error       error;

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (DecodeUnicodeString(File->ID_FullName)[0] == 'c' ||
	    DecodeUnicodeString(File->ID_FullName)[0] == 'C') {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30) ||
		    GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILE1)) {
			return ERR_NOTSUPPORTED;
		}

		memcpy(&File2,File,sizeof(GSM_File));
		CopyUnicodeString(File2.ID_FullName,File->ID_FullName+3*2);
		error = N6510_AddFilePart1(s,&File2,Pos,Handle);
		memcpy(File,&File2,sizeof(GSM_File));
		/* addfilepart returns new ID */
		EncodeUnicode(File->ID_FullName,"c:/",3);
		CopyUnicodeString(File->ID_FullName+UnicodeLength(File->ID_FullName)*2,File2.ID_FullName);
		return error;
	} else {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
			return N6510_AddFilePart2(s,File,Pos,Handle);
		} else {
			return ERR_NOTSUPPORTED;
		}
	}
}

GSM_Error N6510_DeleteFile(GSM_StateMachine *s, unsigned char *ID)
{
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (DecodeUnicodeString(ID)[0] == 'c' ||
	    DecodeUnicodeString(ID)[0] == 'C') {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30) ||
		    GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILE1)) {
			return ERR_NOTSUPPORTED;
		}

		return N6510_DeleteFile1(s,ID+6);
	} else {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
			return N6510_DeleteFile2(s,ID);
		} else {
			return ERR_NOTSUPPORTED;
		}
	}
}

GSM_Error N6510_AddFolder(GSM_StateMachine *s, GSM_File *File)
{
	GSM_File	File2;
	GSM_Error       error;

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (DecodeUnicodeString(File->ID_FullName)[0] == 'c' ||
	    DecodeUnicodeString(File->ID_FullName)[0] == 'C') {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30) ||
		    GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILE1)) {
			return ERR_NOTSUPPORTED;
		}
		memcpy(&File2,File,sizeof(GSM_File));
		CopyUnicodeString(File2.ID_FullName,File->ID_FullName+3*2);
		error = N6510_AddFolder1(s,&File2);
		memcpy(File,&File2,sizeof(GSM_File));
		/* addfolder returns new ID */
		EncodeUnicode(File->ID_FullName,"c:/",3);
		CopyUnicodeString(File->ID_FullName+UnicodeLength(File->ID_FullName)*2,File2.ID_FullName);
		return error;
	} else {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
			return N6510_AddFolder2(s,File);
		} else {
			return ERR_NOTSUPPORTED;
		}
	}
}

GSM_Error N6510_DeleteFolder(GSM_StateMachine *s, unsigned char *ID)
{
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (DecodeUnicodeString(ID)[0] == 'c' ||
	    DecodeUnicodeString(ID)[0] == 'C') {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30) ||
		    GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILE1)) {
			return ERR_NOTSUPPORTED;
		}
		return N6510_DeleteFolder1(s,ID+6);
	} else {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
			return N6510_DeleteFolder2(s,ID);
		} else {
			return ERR_NOTSUPPORTED;
		}
	}
}

GSM_Error N6510_GetFileSystemStatus(GSM_StateMachine *s, GSM_FileSystemStatus *status)
{
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
		return ERR_NOTSUPPORTED;
	} else {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30) ||
		    GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILE1)) {
			return ERR_NOTSUPPORTED;
		}
		return N6510_GetFileSystemStatus1(s,status);
	}
}

GSM_Error N6510_SetFileAttributes(GSM_StateMachine *s, GSM_File *File)
{
	GSM_File	File2;
	char	    	buf[20 + (2 * (GSM_MAX_FILENAME_ID_LENGTH + 1))];
	GSM_Error       error;

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (DecodeUnicodeString(File->ID_FullName)[0] == 'c' ||
	    DecodeUnicodeString(File->ID_FullName)[0] == 'C') {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30) ||
		    GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILE1)) {
			return ERR_NOTSUPPORTED;
		}
		memcpy(&File2,File,sizeof(GSM_File));
		CopyUnicodeString(File2.ID_FullName,File->ID_FullName+3*2);
		error = N6510_SetFileAttributes1(s,&File2);
		CopyUnicodeString(buf,File->ID_FullName);
		memcpy(File,&File2,sizeof(GSM_File));
		CopyUnicodeString(File->ID_FullName,buf);
		return error;
	} else {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
			return N6510_SetFileAttributes2(s,File);
		} else {
			return ERR_NOTSUPPORTED;
		}
	}
}

GSM_Error N6510_GetNextRootFolder(GSM_StateMachine *s, GSM_File *File)
{
	GSM_Error 		error;
	GSM_File  		File2;
	unsigned char		buffer[5];

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	memset(&File2, 0, sizeof(File2));

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30) ||
	    GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILE1)) {
		if (UnicodeLength(File->ID_FullName) == 0) {
			EncodeUnicode(File->ID_FullName,"d:",2);
			EncodeUnicode(File->Name,"D (Permanent_memory 2)",22);
		} else if (!strcmp(DecodeUnicodeString(File->ID_FullName),"d:")) {
			EncodeUnicode(File->ID_FullName,"a:",2);
			error = N6510_GetFolderListing2(s, File, true);
			if (error != ERR_NONE && error != ERR_EMPTY) return ERR_EMPTY;
			EncodeUnicode(File->Name,"A (Memory card)",15);
			EncodeUnicode(File->ID_FullName,"a:",2);
		} else {
			return ERR_EMPTY;
		}
		return ERR_NONE;
	}

	if (UnicodeLength(File->ID_FullName) == 0) {
		sprintf(buffer,"%i",0x01);
		EncodeUnicode(File2.ID_FullName,buffer,strlen(buffer));
		File2.Level = 1;

		error = N6510_GetFileFolderInfo1(s, &File2, false);
		if (error != ERR_NONE) return error;
	}

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
		if (UnicodeLength(File->ID_FullName) == 0) {
			memcpy(File,&File2,sizeof(GSM_File));
			EncodeUnicode(File->Name,"C (",3);
			CopyUnicodeString(File->Name+6,File2.Name);
			EncodeUnicode(File->Name+UnicodeLength(File->Name)*2,")",1);
			sprintf(buffer,"c:\\%i",0x01);
			EncodeUnicode(File->ID_FullName,buffer,strlen(buffer));
		} else if (!strcmp(DecodeUnicodeString(File->ID_FullName),"c:\\1")) {
			EncodeUnicode(File->ID_FullName,"d:",2);
			EncodeUnicode(File->Name,"D (Permanent_memory 2)",22);
		} else if (!strcmp(DecodeUnicodeString(File->ID_FullName),"d:")) {
			EncodeUnicode(File->ID_FullName,"a:",2);
			error = N6510_GetFolderListing2(s, File, true);
			if (error != ERR_NONE && error != ERR_EMPTY) return ERR_EMPTY;
			EncodeUnicode(File->Name,"A (Memory card)",15);
			EncodeUnicode(File->ID_FullName,"a:",2);
		} else {
			return ERR_EMPTY;
		}
		return ERR_NONE;
	}
	if (UnicodeLength(File->ID_FullName) == 0) {
		memcpy(File,&File2,sizeof(GSM_File));
		EncodeUnicode(File->Name,"C (",3);
		CopyUnicodeString(File->Name+6,File2.Name);
		EncodeUnicode(File->Name+UnicodeLength(File->Name)*2,")",1);
		sprintf(buffer,"c:\\%i",0x01);
		EncodeUnicode(File->ID_FullName,buffer,strlen(buffer));
	} else if (!strcmp(DecodeUnicodeString(File->ID_FullName),"c:\\1")) {
		return ERR_EMPTY;
	}
	return ERR_NONE;
}

GSM_Error N6510_PrivGet3220FilesystemMMSFolders(GSM_StateMachine *s, GSM_MMSFolders *folders)
{
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	bool 			Start = true;
	GSM_File	 	Files;
	GSM_Error		error;

	memset(&Files, 0, sizeof(Files));

	EncodeUnicode(Files.ID_FullName,"d:/predefmessages",17);

	folders->Number = 0;

	smprintf(s, "Getting MMS folders\n");
	while (1) {
		error = N6510_GetFolderListing(s,&Files,Start);
		if (error == ERR_EMPTY) return ERR_NONE;
		if (error != ERR_NONE) return error;

		Start = false;

		folders->Folder[folders->Number].InboxFolder = false;
		if (!strcmp(DecodeUnicodeString(Files.Name),"predefinbox")) {
			folders->Folder[folders->Number].InboxFolder = true;
		}

		CopyUnicodeString(Priv->MMSFoldersID2[folders->Number],Files.ID_FullName);

		if (!strcmp(DecodeUnicodeString(Files.Name),"predefinbox")) {
			EncodeUnicode(folders->Folder[folders->Number].Name,"Inbox",5);
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"predefsent")) {
			EncodeUnicode(folders->Folder[folders->Number].Name,"Sent items",10);
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"predefoutbox")) {
			EncodeUnicode(folders->Folder[folders->Number].Name,"Outbox",6);
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"predefdrafts")) {
			EncodeUnicode(folders->Folder[folders->Number].Name,"Templates",9);
		} else {
			continue;
		}

		folders->Number++;
	}
}

/* Series 40 3.0 */
GSM_Error N6510_PrivGetFilesystemMMSFolders(GSM_StateMachine *s, GSM_MMSFolders *folders)
{
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	bool 			Start = true;
	GSM_File	 	Files;
	GSM_Error		error;

	memset(&Files, 0, sizeof(Files));

	EncodeUnicode(Files.ID_FullName,"d:/predefmessages",17);

	folders->Number = 0;

	smprintf(s, "Getting MMS folders\n");
	while (1) {
		error = N6510_GetFolderListing(s,&Files,Start);
		if (error == ERR_EMPTY) return ERR_NONE;
		if (error != ERR_NONE) return error;

		Start = false;

		if (!strcmp(DecodeUnicodeString(Files.Name),"exchange")) {
			continue;
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"predefdrafts")) {
			continue;
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"predefsent")) {
			continue;
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"predefoutbox")) {
			continue;
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"predefinbox")) {
			continue;
		}

		folders->Folder[folders->Number].InboxFolder = false;
		if (!strcmp(DecodeUnicodeString(Files.Name),"1")) {
			folders->Folder[folders->Number].InboxFolder = true;
		}

		CopyUnicodeString(Priv->MMSFoldersID2[folders->Number],Files.ID_FullName);

		if (!strcmp(DecodeUnicodeString(Files.Name),"1")) {
			EncodeUnicode(folders->Folder[folders->Number].Name,"Inbox",5);
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"3")) {
			EncodeUnicode(folders->Folder[folders->Number].Name,"Sent items",10);
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"4")) {
			EncodeUnicode(folders->Folder[folders->Number].Name,"Saved messages",14);
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"5")) {
			EncodeUnicode(folders->Folder[folders->Number].Name,"Drafts",6);
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"6")) {
			EncodeUnicode(folders->Folder[folders->Number].Name,"Templates",9);
		} else {
			CopyUnicodeString(folders->Folder[folders->Number].Name,Files.Name);
		}

		folders->Number++;
	}
}

GSM_Error N6510_GetMMSFolders(GSM_StateMachine *s, GSM_MMSFolders *folders)
{
	GSM_Error 		error;
	GSM_File  		Files;
	bool			Start = true;
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	int			i;


	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	memset(&Files, 0, sizeof(Files));

	for (i=0;i<10;i++) {
		Priv->MMSFoldersID2[i][0] = 0;
		Priv->MMSFoldersID2[i][1] = 0;
	}

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_3220_MMS)) {
		return N6510_PrivGet3220FilesystemMMSFolders(s,folders);
	}

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
		return N6510_PrivGetFilesystemMMSFolders(s,folders);
	}

	EncodeUnicode(Files.ID_FullName,"c:/1",4);
	while (1) {
		error = N6510_GetFolderListing(s,&Files,Start);
		if (error == ERR_EMPTY) break;
		if (error != ERR_NONE) return error;
		Start = false;
		if (!Files.Folder || strcmp(DecodeUnicodeConsole(Files.Name),"Messages")) {
			continue;
		}
		Start 		= true;
		folders->Number = 0;

		while (1) {
			error = N6510_GetFolderListing(s,&Files,Start);
			if (error == ERR_EMPTY) return ERR_NONE;
			if (error != ERR_NONE) return error;
			Start = false;
			if (!Files.Folder) continue;
			CopyUnicodeString(folders->Folder[folders->Number].Name,Files.Name);
			CopyUnicodeString(Priv->MMSFoldersID2[folders->Number],Files.ID_FullName);
			folders->Folder[folders->Number].InboxFolder = false;
			if (!strcmp(DecodeUnicodeString(Files.Name),"Inbox")) {
				folders->Folder[folders->Number].InboxFolder = true;
			}
			folders->Number++;
		}
	}

	/* 6230i */
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
		EncodeUnicode(Files.ID_FullName,"d:/predefmessages",17);
		folders->Number = 0;
		Start 		= true;
		while (1) {
			error = N6510_GetFolderListing(s,&Files,Start);
			if (error == ERR_EMPTY) break;
			if (error != ERR_NONE) return error;
			Start = false;
			if (!Files.Folder) continue;
			folders->Folder[folders->Number].InboxFolder = false;
			if (!strcmp(DecodeUnicodeString(Files.Name),"predefinbox")) {
				EncodeUnicode(folders->Folder[folders->Number].Name,"Inbox",5);
				folders->Folder[folders->Number].InboxFolder = true;
			} else if (!strcmp(DecodeUnicodeString(Files.Name),"predefoutbox")) {
				EncodeUnicode(folders->Folder[folders->Number].Name,"Outbox",6);
			} else if (!strcmp(DecodeUnicodeString(Files.Name),"predefsent")) {
				EncodeUnicode(folders->Folder[folders->Number].Name,"Sent items",10);
			} else if (!strcmp(DecodeUnicodeString(Files.Name),"predefdrafts")) {
				EncodeUnicode(folders->Folder[folders->Number].Name,"Drafts",6);
			} else {
				CopyUnicodeString(folders->Folder[folders->Number].Name,Files.Name);
			}
			CopyUnicodeString(Priv->MMSFoldersID2[folders->Number],Files.ID_FullName);
			folders->Number++;
		}

		return ERR_NONE;
	}

	return ERR_NOTSUPPORTED;
}

GSM_Error N6510_GetNextMMSFileInfo(GSM_StateMachine *s, unsigned char *FileID, int *MMSFolder, bool start)
{
	GSM_MMSFolders 		folders;
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	GSM_Error		error;
	GSM_File		file;
	int			Handle,Size;

	if (start) {
		error = N6510_GetMMSFolders(s, &folders);
		if (error != ERR_NONE)
			return error;

		Priv->MMSFolderNum 	= 0;
		Priv->MMSFolderError 	= ERR_EMPTY;
	}

	while(true) {
		if (Priv->MMSFolderError == ERR_NONE) {
			Priv->MMSFolderError = N6510_GetFolderListing(s,&Priv->MMSFile,false);
			if (Priv->MMSFolderError != ERR_EMPTY && Priv->MMSFolderError != ERR_NONE)
				return Priv->MMSFolderError;
		}

		if (Priv->MMSFolderError == ERR_EMPTY) {
			while (1) {
				if (UnicodeLength(Priv->MMSFoldersID2[Priv->MMSFolderNum]) == 0)
					return ERR_EMPTY;

				CopyUnicodeString(Priv->MMSFile.ID_FullName,Priv->MMSFoldersID2[Priv->MMSFolderNum]);
				Priv->MMSFolderNum++;

				Priv->MMSFolderError = N6510_GetFolderListing(s,&Priv->MMSFile,true);
				if (Priv->MMSFolderError == ERR_EMPTY)
					continue;
				if (Priv->MMSFolderError != ERR_NONE)
					return Priv->MMSFolderError;
				break;
			}
		}
		(*MMSFolder) = Priv->MMSFolderNum;
		CopyUnicodeString(FileID,Priv->MMSFile.ID_FullName);

		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
			CopyUnicodeString(file.ID_FullName,FileID);
			file.Used   = 0;
			file.Buffer = NULL;
			error = N6510_GetFilePart2(s, &file, &Handle, &Size);
			if (error == ERR_NONE) {
				error = N6510_CloseFile2(s, &Handle);
				if (error != ERR_NONE)
					return error;
			} else if (error != ERR_EMPTY) {
				return error;
			}

			/* 0x00 = SMS, 0x01,0x03 = MMS */
			if (file.Buffer[6] != 0x00) {
				free(file.Buffer);
				file.Buffer = NULL;
				break;
			}
			free(file.Buffer);
			file.Buffer = NULL;
		} else {
			break;
		}
	}

	return ERR_NONE;
}

/* Series 40 3.0 */
GSM_Error N6510_PrivGetFilesystemSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders, bool real)
{
	bool 			Start = true;
	GSM_File	 	Files;
	GSM_Error		error;

	EncodeUnicode(Files.ID_FullName,"d:/predefmessages",17);

	folders->Number = 0;

	smprintf(s, "Getting SMS folders\n");
	while (1) {
		error = N6510_GetFolderListing(s,&Files,Start);
		if (error == ERR_EMPTY) return ERR_NONE;
		if (error != ERR_NONE) return error;

		Start = false;

		if (!strcmp(DecodeUnicodeString(Files.Name),"exchange")) {
			continue;
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"predefdrafts")) {
			continue;
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"predefsent")) {
			continue;
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"predefoutbox")) {
			continue;
		} else if (!strcmp(DecodeUnicodeString(Files.Name),"predefinbox")) {
			continue;
		}

		folders->Folder[folders->Number].InboxFolder = false;
		if (!strcmp(DecodeUnicodeString(Files.Name),"1")) {
			folders->Folder[folders->Number].InboxFolder = true;
		}
		folders->Folder[folders->Number].OutboxFolder = false;
		if (!strcmp(DecodeUnicodeString(Files.Name),"2")) {
			folders->Folder[folders->Number].OutboxFolder = true;
		}
		if (real) {
			CopyUnicodeString(folders->Folder[folders->Number].Name,Files.Name);
		} else {
			if (!strcmp(DecodeUnicodeString(Files.Name),"1")) {
				EncodeUnicode(folders->Folder[folders->Number].Name,"Inbox",5);
			} else if (!strcmp(DecodeUnicodeString(Files.Name),"2")) {
				EncodeUnicode(folders->Folder[folders->Number].Name,"Outbox",6);
			} else if (!strcmp(DecodeUnicodeString(Files.Name),"3")) {
				EncodeUnicode(folders->Folder[folders->Number].Name,"Sent items",10);
			} else if (!strcmp(DecodeUnicodeString(Files.Name),"4")) {
				EncodeUnicode(folders->Folder[folders->Number].Name,"Saved messages",14);
			} else if (!strcmp(DecodeUnicodeString(Files.Name),"5")) {
				EncodeUnicode(folders->Folder[folders->Number].Name,"Drafts",6);
			} else if (!strcmp(DecodeUnicodeString(Files.Name),"6")) {
				EncodeUnicode(folders->Folder[folders->Number].Name,"Templates",9);
			} else {
				EncodeUnicode(folders->Folder[folders->Number].Name,"User folder ",12);
				CopyUnicodeString(folders->Folder[folders->Number].Name + 24, Files.Name);
			}
		}
		folders->Folder[folders->Number].Memory      = MEM_ME;
		folders->Number++;
	}
}

/* Series 40 3.0 */
GSM_Error N6510_GetFilesystemSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	return N6510_PrivGetFilesystemSMSFolders(s, folders, false);
}

/* Series 40 3.0 */
static void N26510_GetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char *folderid, int *location)
{
	int ifolderid;

	/* simulate flat SMS memory */
	if (sms->Folder==0x00) {
		ifolderid = sms->Location / GSM_PHONE_MAXSMSINFOLDER;
		*folderid = ifolderid + 0x01;
		*location = sms->Location - ifolderid * GSM_PHONE_MAXSMSINFOLDER;
	} else {
		*folderid = sms->Folder;
		*location = sms->Location;
	}
	smprintf(s, "SMS folder %i & location %i -> 6510 folder %i & location %i\n",
		sms->Folder,sms->Location,*folderid,*location);
}

/* Series 40 3.0 */
static void N26510_SetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char folderid, int location)
{
	sms->Folder	= 0;
	sms->Location	= (folderid - 0x01) * GSM_PHONE_MAXSMSINFOLDER + location;
	smprintf(s, "6510 folder %i & location %i -> SMS folder %i & location %i\n",
		folderid,location,sms->Folder,sms->Location);
}

/* Series 40 3.0 */
GSM_Error N6510_DecodeFilesystemSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, GSM_File *FFF, int location)
{
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	size_t parse_len, pos;
	int loc;
	GSM_Error error;
	bool unknown, has_number;

	sms->Number = 1;
	sms->SMS[0].OtherNumbersNum = 0;

	loc = sms->SMS[0].Location;
	/* Parse PDU data */
	error = GSM_DecodePDUFrame(&(s->di), &(sms->SMS[0]),  FFF->Buffer + 176, FFF->Used - 176, &parse_len, false);
	if (error != ERR_NONE) return error;

	/* Copy recipient/sender number */
	/* Data we get from PDU seem to be bogus */
	/* This might be later overwriten using tags at the end of file */
	CopyUnicodeString(sms->SMS[0].Number, FFF->Buffer + 94);
	smprintf(s, "SMS number: %s\n", DecodeUnicodeString(sms->SMS[0].Number));
	has_number = false;

	sms->SMS[0].Location = loc;

	switch (sms->SMS[0].PDU) {
		case SMS_Deliver:
			sms->SMS[0].State = SMS_Read; /* @bug FIXME: this is wrong */
			break;
		case SMS_Submit:
			sms->SMS[0].State = SMS_Sent; /* @bug FIXME: this is wrong */
			break;
		case SMS_Status_Report:
			sms->SMS[0].State = SMS_Read; /* @bug FIXME: this is wrong */
			break;
	}

	/* Process structured data */
	pos = 176 + parse_len;

	if (pos >= FFF->Used) return ERR_NONE;

	/* First master block - 0x01 0x00 <LENGTH> */
	if (FFF->Buffer[pos] != 0x01) {
		smprintf(s, "Unknown block in SMS data after PDU: 0x%02x\n", FFF->Buffer[pos]);
		DumpMessage(&(s->di), FFF->Buffer + pos, FFF->Used - pos);
		return ERR_UNKNOWN;
	}
	pos += 3;

	while (pos < FFF->Used) {
		unknown = false;
		switch (FFF->Buffer[pos]) {
			case 0x02: /* SMSC number, ASCII */
				if (FFF->Buffer[pos + 2] <= 1) break;
				if (FFF->Buffer[pos + 2] - 1 > GSM_MAX_NUMBER_LENGTH) {
					smprintf(s, "WARNING: Too long SMS number, ignoring!\n");
				} else {
					EncodeUnicode(sms->SMS[0].SMSC.Number, FFF->Buffer + pos + 3, FFF->Buffer[pos + 2]);
				}
				break;
			case 0x03: /* Name, unicode */
				if (FFF->Buffer[pos + 2] <= 1) break;
				if (FFF->Buffer[pos + 2]/2 - 1 > GSM_MAX_SMS_NAME_LENGTH) {
					smprintf(s, "WARNING: Too long SMS name, ignoring!\n");
				} else {
					CopyUnicodeString(sms->SMS[0].Name, FFF->Buffer + pos + 3);
				}
				break;
			case 0x04: /* Sender, unicode */
			case 0x05: /* Recipient, unicode */
			case 0x2b: /* some text (Sender?), unicode */
				if (FFF->Buffer[pos + 2] <= 1) break;
				if (FFF->Buffer[pos + 2]/2 - 1 > GSM_MAX_NUMBER_LENGTH) {
					smprintf(s, "WARNING: Too long SMS number, ignoring!\n");
				} else {
					if (!has_number) {
						CopyUnicodeString(sms->SMS[0].Number, FFF->Buffer + pos + 3);
						has_number = true;
					} else {
						CopyUnicodeString(sms->SMS[0].OtherNumbers[sms->SMS[0].OtherNumbersNum++], FFF->Buffer + pos + 3);
					}
				}
				break;

			case 0x01:
				/* This is probably 0 = received, 1 = sent */
				if (FFF->Buffer[pos + 2] != 1 ||
					(FFF->Buffer[pos + 3] != 0x00 && FFF->Buffer[pos + 3] != 0x01)) {
					unknown = true;
				}
				break;
			case 0x0c:
				/* This seems to be message ID (per number) */
				break;
			case 0x07:
			case 0x0b:
			case 0x0e:
			case 0x22:
			case 0x24:
			case 0x26:
			case 0x27:
			case 0x2a:
			case 0x08:
				if (FFF->Buffer[pos + 2] != 1 || FFF->Buffer[pos + 3] != 0x00) {
					unknown = true;
				}
				break;
			case 0x06:
			case 0x09:
			case 0x12:
			case 0x23:
				if (FFF->Buffer[pos + 2] != 4 ||
					FFF->Buffer[pos + 3] != 0x00 ||
					FFF->Buffer[pos + 4] != 0x00 ||
					FFF->Buffer[pos + 5] != 0x00 ||
					FFF->Buffer[pos + 6] != 0x00
					) {
					unknown = true;
				}
				break;
			case 0x0f:
				if (FFF->Buffer[pos + 2] != 2 ||
					FFF->Buffer[pos + 3] != 0x00 ||
					FFF->Buffer[pos + 4] != 0x00
					) {
					unknown = true;
				}
				break;
			default:
				unknown = true;
				break;
		}
		if (unknown) {
			smprintf(s, "WARNING: Unknown block, see <http://cihar.com/gammu/report> how to report\n");
			DumpMessage(&(s->di), FFF->Buffer + pos, 3 + (FFF->Buffer[pos + 1] << 8) + FFF->Buffer[pos + 2]);
		}
		pos += 3 + (FFF->Buffer[pos + 1] << 8) + FFF->Buffer[pos + 2];
	}

	sms->SMS[0].DateTime = FFF->Modified;
	sms->SMS[0].DateTime.Timezone = 0;

	free(FFF->Buffer);
	FFF->Buffer = NULL;

	N26510_SetSMSLocation(s, &sms->SMS[0], 0, location);

	sms->SMS[0].Folder = Priv->SMSFileFolder;
	sms->SMS[0].Location = 0; /* fixme */

	return ERR_NONE;
}

GSM_Error N6510_GetNextFilesystemSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	unsigned char		folderid;
	int			location,Size,Handle;
	GSM_Error		error;
	GSM_File		FFF;
	bool			start2=start;

	GSM_SetDefaultReceivedSMSData(&sms->SMS[0]);

	while (true) {
		if (start2) {
			Priv->SMSFileError = ERR_EMPTY;
			Priv->SMSFileFolder = 0;
			location = 1;
			error=N6510_PrivGetFilesystemSMSFolders(s,&Priv->LastSMSFolders,true);
			if (error!=ERR_NONE) return error;
		} else {
			sms->SMS[0].Folder = 0;
			N26510_GetSMSLocation(s, &sms->SMS[0], &folderid, &location);
			location++;
			if (Priv->SMSFileError != ERR_EMPTY) {
				Priv->SMSFileError = N6510_GetFolderListing(s,&Priv->SMSFile,false);
			}
		}
		start2 = false;
		while (Priv->SMSFileError == ERR_EMPTY) {
			Priv->SMSFileFolder++;
			/* Too high folder number */
			if ((Priv->SMSFileFolder-1)>=Priv->LastSMSFolders.Number) return ERR_EMPTY;

			EncodeUnicode(Priv->SMSFile.ID_FullName,"d:/predefmessages/",18);
			CopyUnicodeString(Priv->SMSFile.ID_FullName+36,Priv->LastSMSFolders.Folder[Priv->SMSFileFolder-1].Name);
			smprintf(s,"folder name is %s\n",DecodeUnicodeString(Priv->SMSFile.ID_FullName));

			Priv->SMSFileError = N6510_GetFolderListing(s,&Priv->SMSFile,true);
		}

		/* readfile */
		FFF.Buffer= NULL;
		FFF.Used = 0;
		FFF.ID_FullName[0] = 0;
		FFF.ID_FullName[1] = 0;
		CopyUnicodeString(FFF.ID_FullName,Priv->SMSFile.ID_FullName);
		smprintf(s,"sms file name is %s\n",DecodeUnicodeString(FFF.ID_FullName));
		error = ERR_NONE;
		while (error == ERR_NONE) {
			error = N6510_GetFilePart(s,&FFF,&Handle,&Size);
			/* if mms, don't read all */
			if (error==ERR_NONE && FFF.Used>5 && FFF.Buffer[6] != 0x00) {
				error = N6510_CloseFile2(s, &Handle);
				if (error != ERR_NONE) return error;
				break;
			}
		}
		if (FFF.Buffer != NULL)  {
			DumpMessage(&s->di, FFF.Buffer, FFF.Used);

			/* 0x00 = SMS, 0x01,0x03 = MMS */
			if (FFF.Buffer[6] == 0x00) break;

			smprintf(s,"mms file");
			free(FFF.Buffer);
			FFF.Buffer = NULL;
		}
	}

	return N6510_DecodeFilesystemSMS(s, sms, &FFF, location);
}
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
