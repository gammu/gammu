/* (c) 2002-2004 by Marcin Wiacek */
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

static int N6510_FindFileCheckSum(unsigned char *ptr, int len)
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

GSM_Error N6510_ReplyGetFileFolderInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_File	 	*File = s->Phone.Data.FileInfo;
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	int			i;

	switch (msg.Buffer[3]) {
	case 0x15:
		smprintf(s,"File or folder details received\n");
		CopyUnicodeString(File->Name,msg.Buffer+10);
		if (!strncmp(DecodeUnicodeString(File->Name),"GMSTemp",7)) return ERR_EMPTY;
		if (File->Name[0] == 0x00 && File->Name[1] == 0x00) return ERR_UNKNOWN;

		i = msg.Buffer[8]*256+msg.Buffer[9];
		dbgprintf("%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			msg.Buffer[i-5],msg.Buffer[i-4],msg.Buffer[i-3],
			msg.Buffer[i-2],msg.Buffer[i-1],msg.Buffer[i],
			msg.Buffer[i+1],msg.Buffer[i+2],msg.Buffer[i+3]);

		File->Folder 	= false;
		if (msg.Buffer[i-5] == 0x00) File->Folder 	= true;

		File->ReadOnly 	= false;
		File->Protected = false;
		File->System	= false;
		File->Hidden	= false;
		if (msg.Buffer[i+2] == 0x01) File->Protected 	= true;
		if (msg.Buffer[i+4] == 0x01) File->ReadOnly 	= true;
		if (msg.Buffer[i+5] == 0x01) File->Hidden	= true;
		if (msg.Buffer[i+6] == 0x01) File->System	= true;//fixme

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
				Priv->FilesLocations[i+msg.Buffer[9]] 	= Priv->FilesLocations[i];
				Priv->FilesLevels[i+msg.Buffer[9]]	= Priv->FilesLevels[i];
				i--;
			}
			Priv->FilesLocationsUsed += msg.Buffer[9];
			for (i=0;i<msg.Buffer[9];i++) {
				Priv->FilesLocations[Priv->FilesLocationsCurrent+i] 	= msg.Buffer[13+i*4-1]*256 + msg.Buffer[13+i*4];
				Priv->FilesLevels[Priv->FilesLocationsCurrent+i] 	= File->Level+1;
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

static GSM_Error N6510_GetFileFolderInfo(GSM_StateMachine *s, GSM_File *File, GSM_Phone_RequestID Request)
{
	GSM_Error		error;
	unsigned char 		req[10] = {
		N7110_FRAME_HEADER,
		0x14,           /* 0x14 - info, 0x22 - free/total, 0x2E - used, 0x32 - sublocations */
		0x01,		/* 0x00 for sublocations reverse sorting, 0x01 for free */
		0x00, 0x00, 0x01,
		0x00, 0x01};	/* Folder or file number */
	unsigned char 		GetCRC[] = {
		N7110_FRAME_HEADER, 0x42, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x1E}; 	/* file ID */

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

 	s->Phone.Data.FileInfo 	= File;
	req[8]			= atoi(File->ID_FullName) / 256;
	req[9] 			= atoi(File->ID_FullName) % 256;

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

GSM_Error N6510_GetNextFileFolder(GSM_StateMachine *s, GSM_File *File, bool start)
{
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	GSM_Error		error;

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (start) {
		Priv->FilesLocationsUsed 	= 1;
		Priv->FilesLocationsCurrent 	= 0;
		Priv->FilesLocations[0]		= 0x01;
		Priv->FilesLevels[0]		= 1;
	}

	while (1) {
		if (Priv->FilesLocationsCurrent == Priv->FilesLocationsUsed) return ERR_EMPTY;

		sprintf(File->ID_FullName,"%i",Priv->FilesLocations[Priv->FilesLocationsCurrent]);
		File->Level	= Priv->FilesLevels[Priv->FilesLocationsCurrent];
		Priv->FilesLocationsCurrent++;

		error = N6510_GetFileFolderInfo(s, File, ID_GetFileInfo);
		if (error == ERR_EMPTY) continue;
		return error;
	}
}

GSM_Error N6510_ReplyGetFileSystemStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
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

GSM_Error N6510_GetFileSystemStatus(GSM_StateMachine *s, GSM_FileSystemStatus *status)
{
	GSM_Error		error;
	unsigned char 		req[10] = {
		N7110_FRAME_HEADER,
		0x22,           /* 0x14 - info, 0x22 - free/total, 0x2E - used, 0x32 - sublocations */
		0x01,		/* 0x00 for sublocations reverse sorting, 0x01 for free */
		0x00, 0x00, 0x01,
		0x00, 0x01};	/* Folder or file number */

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

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

static GSM_Error N6510_SearchForFileName(GSM_StateMachine *s, GSM_File *File)
{
	GSM_File		File2;
	GSM_Error		error;
	int 			FilesLocations[1000],FilesLocations2[1000];
	int 			FilesLevels[1000];
	int 			FilesLocationsUsed, FilesLocationsCurrent;
	int 			FilesLocationsUsed2, FilesLocationsCurrent2;
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;

	memcpy(FilesLocations,	Priv->FilesLocations,	sizeof(FilesLocations));
	memcpy(FilesLevels,	Priv->FilesLevels,	sizeof(FilesLevels));
	FilesLocationsUsed 	= Priv->FilesLocationsUsed;
	FilesLocationsCurrent 	= Priv->FilesLocationsCurrent;

	Priv->FilesLocationsUsed 	= 1;
	Priv->FilesLocationsCurrent 	= 1;
	Priv->FilesLocations[0]		= atoi(File->ID_FullName);
	Priv->FilesLevels[0]		= 1;

	strcpy(File2.ID_FullName,File->ID_FullName);
	error = N6510_GetFileFolderInfo(s, &File2, ID_GetFileInfo);
	memcpy(FilesLocations2,		Priv->FilesLocations,	sizeof(FilesLocations2));
	FilesLocationsUsed2 		= Priv->FilesLocationsUsed;
	FilesLocationsCurrent2 		= Priv->FilesLocationsCurrent;

	memcpy(Priv->FilesLocations,	FilesLocations,		sizeof(FilesLocations));
	memcpy(Priv->FilesLevels,	FilesLevels,		sizeof(FilesLevels));
	Priv->FilesLocationsUsed 	= FilesLocationsUsed;
	Priv->FilesLocationsCurrent 	= FilesLocationsCurrent;
	if (error != ERR_NONE) return error;

	while (1) {
		if (FilesLocationsCurrent2 == FilesLocationsUsed2) return ERR_EMPTY;

		sprintf(File2.ID_FullName,"%i",FilesLocations2[FilesLocationsCurrent2]);
		dbgprintf("Current is %i\n",FilesLocations2[FilesLocationsCurrent2]);
		FilesLocationsCurrent2++;

		error = N6510_GetFileFolderInfo(s, &File2, ID_AddFile);
		if (error == ERR_EMPTY) continue;
		if (error != ERR_NONE) return error;
		dbgprintf("%s %s\n",DecodeUnicodeString(File->Name),DecodeUnicodeString(File2.Name));
		if (mywstrncasecmp(File2.Name,File->Name,0)) return ERR_NONE;
	}
	return ERR_EMPTY;
}

GSM_Error N6510_ReplyGetFilePart(GSM_Protocol_Message msg, GSM_StateMachine *s)
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

GSM_Error N6510_GetFilePart(GSM_StateMachine *s, GSM_File *File)
{
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	int 			old;
	GSM_Error		error;
	unsigned char 		req[] = {
		N7110_FRAME_HEADER, 0x0E, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x01,		/* Folder or file number */
		0x00, 0x00, 0x00, 0x00,	/* Start from xxx byte */
		0x00, 0x00,
		0x03, 0xE8};		/* Read xxx bytes */

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	if (File->Used == 0x00) {
		error = N6510_GetFileFolderInfo(s, File, ID_GetFile);
		if (error != ERR_NONE) return error;
		File->Used = 0;
	}

	old			= File->Used;
	req[8] 			= atoi(File->ID_FullName) / 256;
	req[9] 			= atoi(File->ID_FullName) % 256;
        req[10]                 = old / (256*256*256);
        req[11]                 = old / (256*256);
        req[12]                 = old / 256;
        req[13]                 = old % 256;

	s->Phone.Data.File 	= File;
	smprintf(s, "Getting file part from filesystem\n");
	error=GSM_WaitFor (s, req, 18, 0x6D, 4, ID_GetFile);
	if (error != ERR_NONE) return error;
	if (File->Used - old != (0x03 * 256 + 0xE8)) {
		if (N6510_FindFileCheckSum(File->Buffer, File->Used) != Priv->FileCheckSum) {
			smprintf(s,"File2 checksum is %i, File checksum is %i\n",N6510_FindFileCheckSum(File->Buffer, File->Used),Priv->FileCheckSum);
			return ERR_WRONGCRC;
		}
		return ERR_EMPTY;
	}
	return ERR_NONE;
}

static GSM_Error N6510_SetReadOnly(GSM_StateMachine *s, unsigned char *ID, bool enable)
{
	unsigned char SetAttr[] = {
		N7110_FRAME_HEADER, 0x18, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x20};		/* File ID */

	if (!enable) SetAttr[4] = 0x06;

	SetAttr[8] = atoi(ID) / 256;
	SetAttr[9] = atoi(ID) % 256;
	smprintf(s, "Setting readonly attribute\n");
	return GSM_WaitFor (s, SetAttr, 10, 0x6D, 4, ID_DeleteFile);
}

GSM_Error N6510_ReplyAddFileHeader(GSM_Protocol_Message msg, GSM_StateMachine *s)
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

GSM_Error N6510_ReplyAddFilePart(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return ERR_NONE;
}

GSM_Error N6510_AddFilePart(GSM_StateMachine *s, GSM_File *File, int *Pos)
{
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	GSM_File		File2;
	GSM_Error		error;
	int			j;
	unsigned char 		Header[400] = {
		N7110_FRAME_HEADER, 0x02, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x0C, 		/* parent folder ID */
 		0x00, 0x00, 0x00, 0xE8};
	unsigned char		Add[15000] = {
		N7110_FRAME_HEADER, 0x40, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x04, 		/* file ID */
		0x00, 0x00, 
		0x01, 0x28}; 		/* length */
	unsigned char end[30] = {
		N7110_FRAME_HEADER, 0x40, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x04, 		/* file ID */
		0x00, 0x00, 0x00, 0x00};

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	s->Phone.Data.File = File;

	if (*Pos == 0) {
		error = N6510_SearchForFileName(s,File);
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
			default                    : Header[231]=0x01; Header[233]=0x05;
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
		error = N6510_GetFileFolderInfo(s, &File2, ID_GetFileInfo);
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
				default                    : Header[231]=0x01; Header[233]=0x05;
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
			error = N6510_SetReadOnly(s, File->ID_FullName, true);
			if (error != ERR_NONE) return error;
		}

		if (N6510_FindFileCheckSum(File->Buffer, File->Used) != Priv->FileCheckSum) {
			smprintf(s,"File2 checksum is %i, File checksum is %i\n",N6510_FindFileCheckSum(File->Buffer, File->Used),Priv->FileCheckSum);
			return ERR_WRONGCRC;
		}

		return ERR_EMPTY;
	}

	return ERR_NONE;
}

GSM_Error N6510_ReplyDeleteFile(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return ERR_NONE;
}

GSM_Error N6510_DeleteFile(GSM_StateMachine *s, unsigned char *ID)
{
	GSM_Error	error;
	unsigned char 	Delete[40] = {
		N7110_FRAME_HEADER, 0x1E, 0x00, 0x00, 0x00, 0x01, 
		0x00, 0x35};		/* File ID */

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	error = N6510_SetReadOnly(s, ID, false);
	if (error != ERR_NONE) return error;

	Delete[8] = atoi(ID) / 256;
	Delete[9] = atoi(ID) % 256;
	return GSM_WaitFor (s, Delete, 10, 0x6D, 4, ID_DeleteFile);
}

GSM_Error N6510_ReplyAddFolder(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	sprintf(s->Phone.Data.File->ID_FullName,"%i",msg.Buffer[9]);
	return ERR_NONE;
}

GSM_Error N6510_AddFolder(GSM_StateMachine *s, GSM_File *File)
{
	GSM_Error	error;
	unsigned char Header[400] = {
		N7110_FRAME_HEADER, 0x04, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x0C, 		/* parent folder ID */
 		0x00, 0x00, 0x00, 0xE8};

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOFILESYSTEM)) return ERR_NOTSUPPORTED;

	error = N6510_SearchForFileName(s,File);
	if (error == ERR_NONE) return ERR_INVALIDLOCATION;
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
		error = N6510_SetReadOnly(s, File->ID_FullName, true);
		if (error != ERR_NONE) return error;
	}

	return error;
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
