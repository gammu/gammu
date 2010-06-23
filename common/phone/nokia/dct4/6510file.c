/* (c) 2003-2005 by Marcin Wiacek */
/* function for making CRC for filesystem (c) 2003 by Michael Schroeder */

#include "../../../gsmstate.h"

#ifdef GSM_ENABLE_NOKIA6510

#include <string.h>
#include <time.h>

#include "../../../misc/coding/coding.h"
#include "../../../gsmcomon.h"
#include "../../../service/gsmlogo.h"
#include "../nfunc.h"
#include "../nfuncold.h"
#include "../../pfunc.h"
#include "dct4func.h"
#include "n6510.h"

//filesystem 1

static int N6510_FindFileCheckSum1(unsigned char *ptr, int len)
{
	int acc, i, accx;
 
	accx = 0;
	acc  = 0xffff; 
	while (len--) {
		accx = (accx & 0xffff00ff) | (acc & 0xff00);
		acc  = (acc  & 0xffff00ff) | *ptr++ << 8;
		for (i = 0; i < 8; i++) {
			acc <<= 1;
			if (acc & 0x10000)     acc ^= 0x1021; 
			if (accx & 0x80000000) acc ^= 0x1021; 
			accx <<= 1;
		}
	}
	dbgprintf("Checksum from Gammu is %04X\n",(acc & 0xffff));
	return (acc & 0xffff);
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

		EncodeHexUnicode (buffer, File->Name, UnicodeLength(File->Name));
		smprintf(s,"Name encoded: %s\n",buffer);

		i = msg.Buffer[8]*256+msg.Buffer[9];
		dbgprintf("%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			msg.Buffer[i-5],msg.Buffer[i-4],msg.Buffer[i-3],
			msg.Buffer[i-2],msg.Buffer[i-1],msg.Buffer[i],
			msg.Buffer[i+1],msg.Buffer[i+2],msg.Buffer[i+3]);

		File->Folder = false;
		if (msg.Buffer[i-5] == 0x00 && msg.Buffer[i+3]!=0x00) File->Folder = true;

		File->ReadOnly  = false;
		File->Protected = false;
		File->System    = false;
		File->Hidden    = false;
		if (msg.Buffer[i+2] == 0x01) File->Protected    = true;
		if (msg.Buffer[i+4] == 0x01) File->ReadOnly     = true;
		if (msg.Buffer[i+5] == 0x01) File->Hidden       = true;
		if (msg.Buffer[i+6] == 0x01) File->System       = true;//fixme

		File->ModifiedEmpty = false;
		NOKIA_DecodeDateTime(s, msg.Buffer+i-22, &File->Modified);
		if (File->Modified.Year == 0x00) File->ModifiedEmpty = true;
		dbgprintf("%02x %02x %02x %02x\n",msg.Buffer[i-22],msg.Buffer[i-21],msg.Buffer[i-20],msg.Buffer[i-19]);

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
			i = Priv->FilesLocationsUsed-1;
			while (1) {
				if (i==Priv->FilesLocationsCurrent-1) break;
				dbgprintf("Copying %i to %i, max %i, current %i\n",
					i,i+msg.Buffer[9],
					Priv->FilesLocationsUsed,Priv->FilesLocationsCurrent);
				Priv->FilesLocations[i+msg.Buffer[9]]   = Priv->FilesLocations[i];
				Priv->FilesLevels[i+msg.Buffer[9]]      = Priv->FilesLevels[i];
				i--;
			}
			Priv->FilesLocationsUsed += msg.Buffer[9];
			for (i=0;i<msg.Buffer[9];i++) {
				Priv->FilesLocations[Priv->FilesLocationsCurrent+i]     = msg.Buffer[13+i*4-1]*256 + msg.Buffer[13+i*4];
				Priv->FilesLevels[Priv->FilesLocationsCurrent+i]	= File->Level+1;
				dbgprintf("%i ",Priv->FilesLocations[Priv->FilesLocationsCurrent+i]);
			}
			dbgprintf("\n");
		}
		if (msg.Buffer[9] != 0x00) File->Folder = true;
		return ERR_NONE;		
	case 0x43:
		Priv->FileCheckSum = msg.Buffer[6] * 256 + msg.Buffer[7];
		smprintf(s,"File checksum from phone is %04X\n",Priv->FileCheckSum);
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetFileFolderInfo1(GSM_StateMachine *s, GSM_File *File, GSM_Phone_RequestID Request)
{
	GSM_Error	       	error;
	unsigned char	   	req[10] = {
		N7110_FRAME_HEADER,
		0x14,	   	/* 0x14 - info, 0x22 - free/total, 0x2E - used, 0x32 - sublocations */
		0x01,	   	/* 0x00 for sublocations reverse sorting, 0x01 for free */
		0x00, 0x00, 0x01,
		0x00, 0x01};    /* Folder or file number */
	unsigned char	   	GetCRC[] = {
		N7110_FRAME_HEADER, 0x42, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x1E};    /* file ID */

	s->Phone.Data.FileInfo  = File;
	req[8]		  = atoi(File->ID_FullName) / 256;
	req[9]		  = atoi(File->ID_FullName) % 256;

	req[3] = 0x14;
	req[4] = 0x01;
	smprintf(s,"Getting info for file in filesystem\n");
	error=GSM_WaitFor (s, req, 10, 0x6D, 4, Request);
	if (error != ERR_NONE) return error;

	if (Request != ID_AddFile) {
		req[3] = 0x32;
		req[4] = 0x00;
		smprintf(s,"Getting subfolders for filesystem\n");
		error=GSM_WaitFor (s, req, 10, 0x6D, 4, Request);
		if (error != ERR_NONE) return error;

		if (!File->Folder) {
			req[3] = 0x2E;
			req[4] = 0x01;
			smprintf(s,"Getting used memory for file in filesystem\n");
			error=GSM_WaitFor (s, req, 10, 0x6D, 4, Request);
			if (error != ERR_NONE) return error;

			GetCRC[8] = atoi(File->ID_FullName) / 256;
			GetCRC[9] = atoi(File->ID_FullName) % 256;
			smprintf(s,"Getting CRC for file in filesystem\n");
			error=GSM_WaitFor (s, GetCRC, 10, 0x6D, 4, Request);
		}
	}
	return error;
}

static GSM_Error N6510_GetNextFileFolder1(GSM_StateMachine *s, GSM_File *File, bool start)
{
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	GSM_Error	       	error;

	if (start) {
		Priv->FilesLocationsUsed	= 1;
		Priv->FilesLocationsCurrent     = 0;
		Priv->FilesLocations[0]	 	= 0x01;
		Priv->FilesLevels[0]	    	= 1;
	}

	while (1) {
		if (Priv->FilesLocationsCurrent == Priv->FilesLocationsUsed) return ERR_EMPTY;

		sprintf(File->ID_FullName,"%i",Priv->FilesLocations[Priv->FilesLocationsCurrent]);
		File->Level     = Priv->FilesLevels[Priv->FilesLocationsCurrent];
		Priv->FilesLocationsCurrent++;

		error = N6510_GetFileFolderInfo1(s, File, ID_GetFileInfo);
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

	s->Phone.Data.FileSystemStatus = status;

	status->Free = 0;

	req[3] = 0x2E;
	req[4] = 0x01;
	smprintf(s, "Getting used/total memory in filesystem\n");
	error = GSM_WaitFor (s, req, 10, 0x6D, 4, ID_FileSystemStatus);

	req[3] = 0x22;
	req[4] = 0x01;
	smprintf(s, "Getting free memory in filesystem\n");
	return GSM_WaitFor (s, req, 10, 0x6D, 4, ID_FileSystemStatus);
}

static GSM_Error N6510_SearchForFileName1(GSM_StateMachine *s, GSM_File *File)
{
	GSM_File		File2;
	GSM_Error	       	error;
	int		     	FilesLocations[1000],FilesLocations2[1000];
	int		    	FilesLevels[1000];
	int		     	FilesLocationsUsed, FilesLocationsCurrent;
	int		     	FilesLocationsUsed2, FilesLocationsCurrent2;
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;

	memcpy(FilesLocations,  Priv->FilesLocations,   sizeof(FilesLocations));
	memcpy(FilesLevels,     Priv->FilesLevels,      sizeof(FilesLevels));
	FilesLocationsUsed      = Priv->FilesLocationsUsed;
	FilesLocationsCurrent   = Priv->FilesLocationsCurrent;

	Priv->FilesLocationsUsed	= 1;
	Priv->FilesLocationsCurrent     = 1;
	Priv->FilesLocations[0]	 	= atoi(File->ID_FullName);
	Priv->FilesLevels[0]	    	= 1;

	strcpy(File2.ID_FullName,File->ID_FullName);
	error = N6510_GetFileFolderInfo1(s, &File2, ID_GetFileInfo);
	memcpy(FilesLocations2,	Priv->FilesLocations, sizeof(FilesLocations2));
	FilesLocationsUsed2	     	= Priv->FilesLocationsUsed;
	FilesLocationsCurrent2	  	= Priv->FilesLocationsCurrent;

	memcpy(Priv->FilesLocations, FilesLocations, sizeof(FilesLocations));
	memcpy(Priv->FilesLevels, FilesLevels, sizeof(FilesLevels));
	Priv->FilesLocationsUsed	= FilesLocationsUsed;
	Priv->FilesLocationsCurrent     = FilesLocationsCurrent;
	if (error != ERR_NONE) return error;

	while (1) {
		if (FilesLocationsCurrent2 == FilesLocationsUsed2) return ERR_EMPTY;

		sprintf(File2.ID_FullName,"%i",FilesLocations2[FilesLocationsCurrent2]);
		dbgprintf("Current is %i\n",FilesLocations2[FilesLocationsCurrent2]);
		FilesLocationsCurrent2++;

		error = N6510_GetFileFolderInfo1(s, &File2, ID_AddFile);
		if (error == ERR_EMPTY) continue;
		if (error != ERR_NONE) return error;
		dbgprintf("%s %s\n",DecodeUnicodeString(File->Name),DecodeUnicodeString(File2.Name));
		if (mywstrncasecmp(File2.Name,File->Name,0)) return ERR_NONE;
	}
	return ERR_EMPTY;
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

static GSM_Error N6510_GetFilePart1(GSM_StateMachine *s, GSM_File *File, int *Handle, int *Size)
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
		error = N6510_GetFileFolderInfo1(s, File, ID_GetFile);
		if (error != ERR_NONE) return error;

		(*Size) 	= File->Used;
		File->Used 	= 0;
	}

	old		 = File->Used;
	req[8]		 = atoi(File->ID_FullName) / 256;
	req[9]		 = atoi(File->ID_FullName) % 256;
	req[10]		 = old / (256*256*256);
	req[11]		 = old / (256*256);
	req[12]		 = old / 256;
	req[13]		 = old % 256;

	s->Phone.Data.File = File;
	smprintf(s, "Getting file part from filesystem\n");
	error=GSM_WaitFor (s, req, 18, 0x6D, 4, ID_GetFile);
	if (error != ERR_NONE) return error;
	if (File->Used - old != (0x03 * 256 + 0xE8)) {
		if (N6510_FindFileCheckSum1(File->Buffer, File->Used) != Priv->FileCheckSum) {
			smprintf(s,"File2 checksum is %i, File checksum is %i\n",N6510_FindFileCheckSum1(File->Buffer, File->Used),Priv->FileCheckSum);
			return ERR_WRONGCRC;
		}
		return ERR_EMPTY;
	}
	return ERR_NONE;
}

static GSM_Error N6510_SetReadOnly1(GSM_StateMachine *s, unsigned char *ID, bool enable)
{
	unsigned char SetAttr[] = {
		N7110_FRAME_HEADER, 0x18, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x20};	    /* File ID */

	if (!enable) SetAttr[4] = 0x06;

	SetAttr[8] = atoi(ID) / 256;
	SetAttr[9] = atoi(ID) % 256;
	smprintf(s, "Setting readonly attribute\n");
	return GSM_WaitFor (s, SetAttr, 10, 0x6D, 4, ID_DeleteFile);
}

GSM_Error N6510_ReplyAddFileHeader1(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
	case 0x03:
		smprintf(s,"File header added\n");
		sprintf(s->Phone.Data.File->ID_FullName,"%i",msg.Buffer[9]);
		return ERR_NONE;
	case 0x13:
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error N6510_ReplyAddFilePart1(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return ERR_NONE;
}

static GSM_Error N6510_AddFilePart1(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle)
{
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	GSM_File		File2;
	GSM_Error	       	error;
	int		     	j;
	unsigned char	   	Header[400] = {
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

	s->Phone.Data.File = File;

	if (*Pos == 0) {
		error = N6510_SearchForFileName1(s,File);
		if (error == ERR_NONE) return ERR_FILEALREADYEXIST;
		if (error != ERR_EMPTY) return error;

		Header[8] = atoi(File->ID_FullName) / 256;
		Header[9] = atoi(File->ID_FullName) % 256;
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
			case GSM_File_Sound_MIDI   : Header[231]=0x04; Header[233]=0x05; break; //Header[238]=0x01; 
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
		Header[236] = atoi(File->ID_FullName) / 256;
		Header[237] = atoi(File->ID_FullName) % 256;
		if (File->Protected) Header[238] = 0x01; //Nokia forward lock
		if (File->Hidden)    Header[241] = 0x01;
		if (File->System)    Header[242] = 0x01; //fixme
		smprintf(s, "Adding file header\n");
		error=GSM_WaitFor (s, Header, 246, 0x6D, 4, ID_AddFile);
		if (error != ERR_NONE) return error;
	}

	j = 1000;
	if (File->Used - *Pos < 1000) j = File->Used - *Pos;
	Add[ 8] = atoi(File->ID_FullName) / 256;
	Add[ 9] = atoi(File->ID_FullName) % 256;
	Add[12] = j / 256;
	Add[13] = j % 256;
	memcpy(Add+14,File->Buffer+(*Pos),j);
	smprintf(s, "Adding file part %i %i\n",*Pos,j);
	error=GSM_WaitFor (s, Add, 14+j, 0x6D, 4, ID_AddFile);
	if (error != ERR_NONE) return error;
	*Pos = *Pos + j;

	if (j < 1000) {
		end[8] = atoi(File->ID_FullName) / 256;
		end[9] = atoi(File->ID_FullName) % 256;
		smprintf(s, "Frame for ending adding file\n");
		error = GSM_WaitFor (s, end, 14, 0x6D, 4, ID_AddFile);
		if (error != ERR_NONE) return error;

		strcpy(File2.ID_FullName,File->ID_FullName);
		error = N6510_GetFileFolderInfo1(s, &File2, ID_GetFileInfo);
		if (error != ERR_NONE) return error;

		if (!File->ModifiedEmpty) {
			Header[3]   = 0x12;
			Header[4]   = 0x01;
			Header[12]  = 0x00;
			Header[13]  = 0xE8;
			Header[8]   = atoi(File->ID_FullName) / 256;
			Header[9]   = atoi(File->ID_FullName) % 256;
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
				case GSM_File_Sound_MIDI   : Header[231]=0x04; Header[233]=0x05; break; //Header[238]=0x01; 
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

		if (N6510_FindFileCheckSum1(File->Buffer, File->Used) != Priv->FileCheckSum) {
			smprintf(s,"File2 checksum is %i, File checksum is %i\n",N6510_FindFileCheckSum1(File->Buffer, File->Used),Priv->FileCheckSum);
			return ERR_WRONGCRC;
		}

		return ERR_EMPTY;
	}

	return ERR_NONE;
}

GSM_Error N6510_ReplyDeleteFile1(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return ERR_NONE;
}

static GSM_Error N6510_DeleteFile1(GSM_StateMachine *s, unsigned char *ID)
{
	GSM_Error       error;
	unsigned char   Delete[40] = {
		N7110_FRAME_HEADER, 0x1E, 0x00, 0x00, 0x00, 0x01, 
		0x00, 0x35};	    /* File ID */

	error = N6510_SetReadOnly1(s, ID, false);
	if (error != ERR_NONE) return error;

	Delete[8] = atoi(ID) / 256;
	Delete[9] = atoi(ID) % 256;
	return GSM_WaitFor (s, Delete, 10, 0x6D, 4, ID_DeleteFile);
}

GSM_Error N6510_ReplyAddFolder1(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	sprintf(s->Phone.Data.File->ID_FullName,"%i",msg.Buffer[9]);
	return ERR_NONE;
}

static GSM_Error N6510_AddFolder1(GSM_StateMachine *s, GSM_File *File)
{
	GSM_Error       error;
	unsigned char   Header[400] = {
		N7110_FRAME_HEADER, 0x04, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x0C,	     /* parent folder ID */
		0x00, 0x00, 0x00, 0xE8};

	error = N6510_SearchForFileName1(s,File);
	if (error == ERR_NONE) return ERR_FILEALREADYEXIST;
	if (error != ERR_EMPTY) return error;

	Header[8] = atoi(File->ID_FullName) / 256;
	Header[9] = atoi(File->ID_FullName) % 256;
	memset(Header+14, 0x00, 300);
	CopyUnicodeString(Header+14,File->Name);
	Header[233] = 0x02;
	Header[235] = 0x01;
	Header[236] = atoi(File->ID_FullName) / 256;
	Header[237] = atoi(File->ID_FullName) % 256;
	
	s->Phone.Data.File = File;
	smprintf(s, "Adding folder\n");
	error = GSM_WaitFor (s, Header, 246, 0x6D, 4, ID_AddFolder);
	if (error != ERR_NONE) return error;

	/* Can't delete from phone menu */
	if (File->ReadOnly) {
		error = N6510_SetReadOnly1(s, File->ID_FullName, true);
		if (error != ERR_NONE) return error;
	}

	return error;
}

//filesystem 2

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
	} else if (msg.Buffer[4] == 0x06) {
		smprintf(s,"File not exist\n");
		return ERR_FILENOTEXIST;		
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_OpenFile2(GSM_StateMachine *s, char *Name, int *Handle, bool Create)
{
	unsigned char	 req[200] = {N6110_FRAME_HEADER, 0x72,
				     0x00,	  // mode 0 - open read only, 0x11 - read write create
				     0x02, 
				     0xFF, 0xFF}; // name length
	int		 Pos = 8;
	GSM_Error	 error;

	if (Create) req[4] = 0x11;
	req[6] = (strlen(Name)*2 + 2)/ 256 ;
	req[7] = (strlen(Name)*2 + 2)% 256 ;
	EncodeUnicode(req+8,Name,strlen(Name));
	Pos+=strlen(Name)*2;
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
				     0x00, 0x00, 0x00, 0x00}; //file handle

	req[6]		 = (*Handle) / (256*256*256);
	req[7]		 = (*Handle) / (256*256);
	req[8]		 = (*Handle) / 256;
	req[9]		 = (*Handle) % 256;

	smprintf(s, "Closing file\n");
	return GSM_WaitFor (s, req, 10, 0x6D, 4, ID_CloseFile);
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
			Priv->FilesEnd = true;
			return ERR_NONE;			
		case 0x00:
		case 0x0D:
			switch (msg.Buffer[5]) {
			case 0x06:
				smprintf(s,"File not exist\n");
				return ERR_FILENOTEXIST;
			case 0x00:
				break;
			default:
				smprintf(s,"unknown status code\n");
				return ERR_UNKNOWNRESPONSE;
			}
			smprintf(s,"File or folder details received\n");

			if (msg.Buffer[3] == 0x69) {
				for (i=Priv->FilesLocationsUsed+1;i>Priv->FilesLocationsCurrent;i--) {
					memcpy(&Priv->Files[i],&Priv->Files[i-1],sizeof(GSM_File));
				}

				File = &Priv->Files[Priv->FilesLocationsCurrent+1];

				File->Level = Priv->Files[Priv->FilesLocationsCurrent].Level + 1;
				Priv->FilesLocationsUsed++;

				CopyUnicodeString(File->Name,msg.Buffer+32);
				smprintf(s,"\"%s\"\n",DecodeUnicodeString(File->Name));
				strcpy(File->ID_FullName,FileInfo->ID_FullName);
				sprintf(File->ID_FullName+strlen(File->ID_FullName),"/%s",DecodeUnicodeString(msg.Buffer+32));
				smprintf(s,"\"%s\"\n",File->ID_FullName);
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
				smprintf(s,"Size %i bytes\n",File->Used);
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
			NOKIA_DecodeDateTime(s, msg.Buffer+14, &File->Modified);

			if (msg.Buffer[3] == 0x69 && msg.Buffer[4] == 0) Priv->FilesEnd = true;

			return ERR_NONE;
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
	unsigned char	   	req[200] = {
		N7110_FRAME_HEADER,0x6C,
		0xFF, 0xFF}; // name length

	s->Phone.Data.FileInfo  = File;

	req[4] = (strlen(File->ID_FullName)*2 + 2)/ 256 ;
	req[5] = (strlen(File->ID_FullName)*2 + 2)% 256 ;
	EncodeUnicode(req+6,File->ID_FullName,strlen(File->ID_FullName));
	Pos+=strlen(File->ID_FullName)*2;
	req[Pos++] = 0;
	req[Pos++] = 0;

	smprintf(s,"Getting info for file in filesystem\n");
	return GSM_WaitFor (s, req, Pos, 0x6D, 4, ID_GetFileInfo);
}

static GSM_Error N6510_GetNextFileFolder2(GSM_StateMachine *s, GSM_File *File, bool start)
{
	GSM_Error	       	error;
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	unsigned char	   	req[200] = {N6110_FRAME_HEADER, 0x68, 
					    0xFF, 0xFF}; // name length
	int		     	Pos = 6, i = 0, old;

	if (start) {
		Priv->FilesLocationsUsed	= 2;
		Priv->FilesLocationsCurrent     = 0;

		Priv->FilesLocations[0]	 	= 0x01;
		Priv->FilesLevels[0]	    	= 1;
		Priv->Files[0].Folder	   	= true;
		Priv->Files[0].Level	    	= 1;
		sprintf(Priv->Files[0].ID_FullName,"a:");
		EncodeUnicode(Priv->Files[0].Name,"Phone",5);

		Priv->FilesLocations[1]	 	= 0x01;
		Priv->FilesLevels[1]	    	= 1;
		Priv->Files[1].Folder	   	= true;
		Priv->Files[1].Level	    	= 1;
		sprintf(Priv->Files[1].ID_FullName,"b:");
		EncodeUnicode(Priv->Files[1].Name,"MMC card",8);
	}

	dbgprintf("entering %i %i\n",Priv->FilesLocationsCurrent,Priv->FilesLocationsUsed);
	if (Priv->FilesLocationsCurrent == Priv->FilesLocationsUsed) return ERR_EMPTY;

	if (!Priv->Files[Priv->FilesLocationsCurrent].Folder) {
		memcpy(File,&Priv->Files[Priv->FilesLocationsCurrent],sizeof(GSM_File));
		Priv->FilesLocationsCurrent++;
		return ERR_NONE;
	}

	s->Phone.Data.FileInfo = File;

	req[4] = (strlen(Priv->Files[Priv->FilesLocationsCurrent].ID_FullName)*2 + 6)/ 256 ;
	req[5] = (strlen(Priv->Files[Priv->FilesLocationsCurrent].ID_FullName)*2 + 6)% 256 ;
	EncodeUnicode(req+6,Priv->Files[Priv->FilesLocationsCurrent].ID_FullName,strlen(Priv->Files[Priv->FilesLocationsCurrent].ID_FullName));
	Pos+=strlen(Priv->Files[Priv->FilesLocationsCurrent].ID_FullName)*2;
	req[Pos++] = 0;
	req[Pos++] = '/';
	req[Pos++] = 0;
	req[Pos++] = '*';
	req[Pos++] = 0;
	req[Pos++] = 0;

	sprintf(File->ID_FullName,"%s",Priv->Files[Priv->FilesLocationsCurrent].ID_FullName);
	smprintf(s, "Getting folder info\n");

	old = Priv->FilesLocationsUsed;

	Priv->FilesEnd = false;
	error = s->Protocol.Functions->WriteMessage(s, req, Pos, 0x6D);
	if (error!=ERR_NONE) return error;

	while (!Priv->FilesEnd) {
		if (GSM_ReadDevice(s,true)==0) i++;
		if (i==2) break;
	}

	if (!strcmp(File->ID_FullName,"b:") && Priv->FilesLocationsUsed==old) {
		Priv->FilesLocationsUsed--;
		return ERR_EMPTY;
	}

	memcpy(File,&Priv->Files[Priv->FilesLocationsCurrent],sizeof(GSM_File));
	Priv->FilesLocationsCurrent++;

	return error;
}

static GSM_Error N6510_GetFilePart2(GSM_StateMachine *s, GSM_File *File, int *Handle, int *Size)
{
	int		    	old;
	GSM_Error	       	error;
	unsigned char	   	req[] = {
		N7110_FRAME_HEADER, 0x5E, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x01,		//file handle
		0x00, 0x00, 0x00, 0x00, 	//position
		0x00, 0x00, 0x03, 0xE8, 	//length
		0x00, 0x00, 0x03, 0xE8};	//buffer length

	if (File->Used == 0x00) {
		error = N6510_GetFileFolderInfo2(s, File);
		if (error != ERR_NONE) return error;

		error = N6510_OpenFile2(s, File->ID_FullName, Handle, false);
		if (error != ERR_NONE) return error;

		EncodeUnicode(File->Name,File->ID_FullName,strlen(File->ID_FullName));
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
		return ERR_EMPTY;
	}
	return ERR_NONE;
}

static GSM_Error N6510_AddFilePart2(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle)
{
	GSM_Error	       	error;
	int		     	j;
	unsigned char		buffer[500];
	unsigned char	   	req[15000] = {
		N7110_FRAME_HEADER, 0x58, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 	// handle
		0x00, 0x00, 0x04, 0x00};	// buffer len

	s->Phone.Data.File = File;

	if (*Pos == 0) {
		sprintf(buffer,"%s/%s",File->ID_FullName,DecodeUnicodeString(File->Name));
		strcpy(File->ID_FullName,buffer);

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

	j = 1000;
	if (File->Used - *Pos < 1000) j = File->Used - *Pos;
	req[10]		 = j / (256*256*256);
	req[11]		 = j / (256*256);
	req[12]		 = j / 256;
	req[13]		 = j % 256;
	memcpy(req+14,File->Buffer+(*Pos),j);

	smprintf(s, "Adding file part %i %i\n",*Pos,j);
	error=GSM_WaitFor (s, req, 14+j, 0x6D, 4, ID_AddFile);
	if (error != ERR_NONE) return error;
	*Pos = *Pos + j;

	if (j < 1000) {
		error = N6510_CloseFile2(s, Handle);
		if (error != ERR_NONE) return error;

		return ERR_EMPTY;
	}

	return ERR_NONE;
}

//shared

GSM_Error N6510_GetNextFileFolder(GSM_StateMachine *s, GSM_File *File, bool start)
{
	GSM_Error	       	error;
	GSM_Phone_N6510Data     *Priv = &s->Phone.Data.Priv.N6510;
	char		    	buf[200];

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
		if (start) Priv->Use2 = true;
		if (Priv->Use2) {
			error = N6510_GetNextFileFolder2(s,File,start);
			if (error == ERR_EMPTY) {
				Priv->Use2      = false;
				start	   	= true;
			} else {
				return error;
			}
		}
		error = N6510_GetNextFileFolder1(s,File,start);
		if (error == ERR_NONE) {
			sprintf(buf,"%s",File->ID_FullName);
			sprintf(File->ID_FullName,"c:/%s",buf);
		}
		return error;
	} else {
		return N6510_GetNextFileFolder1(s,File,start);
	}
}

GSM_Error N6510_GetFilePart(GSM_StateMachine *s, GSM_File *File, int *Handle, int *Size)
{
	GSM_File	File2;
	char	    	buf[200];
	GSM_Error       error;

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
		if (File->ID_FullName[0] == 'c') {
			memcpy(&File2,File,sizeof(GSM_File));
			strcpy(buf,File2.ID_FullName+3);
			sprintf(File2.ID_FullName,"%s",buf);
			error = N6510_GetFilePart1(s,&File2, Handle, Size);
			if (error!=ERR_NONE) return error;
			memcpy(File,&File2,sizeof(GSM_File));
			strcpy(buf,File->ID_FullName);
			sprintf(File->ID_FullName,"c:\\%s",buf);
			return error;
		} else {
			return N6510_GetFilePart2(s,File, Handle, Size);
		}
	} else {
		return N6510_GetFilePart1(s,File, Handle, Size);
	}
}

GSM_Error N6510_AddFilePart(GSM_StateMachine *s, GSM_File *File, int *Pos, int *Handle)
{
	GSM_File	File2;
	char	    	buf[200];
	GSM_Error       error;

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
		if (File->ID_FullName[0] == 'c') {
			memcpy(&File2,File,sizeof(GSM_File));
			strcpy(buf,File2.ID_FullName+3);
			sprintf(File2.ID_FullName,"%s",buf);
			error = N6510_AddFilePart1(s,&File2,Pos,Handle);
			memcpy(File,&File2,sizeof(GSM_File));
			strcpy(buf,File->ID_FullName);
			sprintf(File->ID_FullName,"c:/%s",buf);
			return error;
		} else {
			return N6510_AddFilePart2(s,File,Pos,Handle);
		}
	} else {
		return N6510_AddFilePart1(s,File,Pos,Handle);
	}
}

GSM_Error N6510_DeleteFile(GSM_StateMachine *s, unsigned char *ID)
{
	char buf[200];

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
		if (ID[0] == 'c') {
			strcpy(buf,ID+3);
			return N6510_DeleteFile1(s,buf);
		} else {
			return ERR_NOTSUPPORTED;
		}
	} else {
		return N6510_DeleteFile1(s,ID);
	}
}

GSM_Error N6510_AddFolder(GSM_StateMachine *s, GSM_File *File)
{
	GSM_File	File2;
	char	    	buf[200];
	GSM_Error       error;

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
		if (File->ID_FullName[0] == 'c') {
			memcpy(&File2,File,sizeof(GSM_File));
			strcpy(buf,File2.ID_FullName+3);
			sprintf(File2.ID_FullName,"%s",buf);
			error = N6510_AddFolder1(s,&File2);
			memcpy(File,&File2,sizeof(GSM_File));
			strcpy(buf,File->ID_FullName);
			sprintf(File->ID_FullName,"c:\\%s",buf);
			return error;
		} else {
			return ERR_NOTSUPPORTED;
		}
	} else {
		return N6510_AddFolder1(s,File);
	}
}

GSM_Error N6510_GetFileSystemStatus(GSM_StateMachine *s, GSM_FileSystemStatus *status)
{
	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_FILES2)) {
		return ERR_NOTSUPPORTED;
//	      return N6510_GetFileSystemStatus2(s,status);
	} else {
		return N6510_GetFileSystemStatus1(s,status);
	}
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
