#include "stdafx.h"
#include "MTK.h"
#include "commfun.h"
#include "coding.h"
#include "MTKfundef.h"
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

extern CMTKApp theApp;
extern GSM_Reply_MsgType ReplymsgType;

GSM_Error MTK_ReplyMMSFloderInfo(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	MTK_GetReplyStatue(&msg,&Replynfo);

	char			*pos;
	unsigned char		buffer[2000];
	int nRet = 0;
	switch (Replynfo.ReplyState) {
	case AT_Reply_Continue: //peggy 0201 add
	case AT_Reply_OK:
		pos = strstr((char*)msg.Buffer, "+EMMSFS:");
		if (pos == NULL)		return ERR_UNKNOWN;
		pos += 8; 
		pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
		nRet = atoi((char*)buffer);
		if(nRet != 0) 	return ERR_UNKNOWN;
	
		pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
		theApp.m_MMSFolderInfo.nMsgCount = atoi((char*)buffer);

		pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
		theApp.m_MMSFolderInfo.nUnread = atoi((char*)buffer);

		while (*pos != '"') pos++;
		pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
		DecodeHexUnicode(theApp.m_MMSFolderInfo.uHomedir,buffer+1,strlen((char*)buffer)-2);
	
		while (*pos != '"') pos++;
		pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
		DecodeHexUnicode(theApp.m_MMSFolderInfo.uFileinfo,buffer+1,strlen((char*)buffer)-2);
		return ERR_NONE;
		break;
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(Replynfo);
	case AT_Reply_Error:
		return ERR_INVALIDLOCATION;
	case AT_Reply_CMSError:
 	        return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

void RemoveAllChileItem(GSM_File *Folder)
{
	int nParentId = atoi((char*)Folder->ID_FullName);
	POSITION delpos;
	POSITION pos = theApp.m_MMSFileInfoList.GetHeadPosition();
	while(pos)
	{
		delpos = pos;
		GSM_File* fileinfo = (GSM_File* )theApp.m_MMSFileInfoList.GetNext(pos);
		if(fileinfo)
		{
			if(nParentId == fileinfo->Level)
			{
				delete fileinfo;
				fileinfo = NULL;
				theApp.m_MMSFileInfoList.RemoveAt(delpos);

			}
		}
	}
}
void AddFileToFileList(unsigned char* pBuf,unsigned char *pHomedir,int nParentIndex)
{
	char szHexId[200];
	char szFileName[200];
	unsigned char uFileName[200];
	MMSFileInfo fileinfo;
	int nsize = sizeof(MMSFileInfo);
	memcpy(&fileinfo,pBuf,sizeof(MMSFileInfo));

	int nIndex =fileinfo.nMsgId *(-1);
	char szName[1];
	szName[0] =fileinfo.szext[0] ;
	szName[1] ='\0' ;

	GSM_File *file = new GSM_File;
	ZeroMemory(file,sizeof(GSM_File));
	file->Folder		= false;
	file->Level		= nParentIndex;

	DecodeUTF8ToUnicode(file->Name,fileinfo.szName,strlen((char*)fileinfo.szName)	);

	itoa(nIndex,(char*)file->ID_FullName,10);
	itoa(fileinfo.nMsgId,szHexId,16);
	sprintf(szFileName,"\\%s.%s",szHexId,szName);
	EncodeUnicode(uFileName,(unsigned char*)szFileName,strlen(szFileName));
	CopyUnicodeString(file->wFileFullPathName,pHomedir);
	UnicodeCat(file->wFileFullPathName,uFileName);
	theApp.m_MMSFileInfoList.AddTail(file);

	theApp.m_FilesLocations[theApp.m_FilesLocationsUsed]		= nIndex;
	theApp.m_FilesLocationsUsed++;

/*	char szFileName[200];
	unsigned char uFileName[200];
	unsigned char szTemp[200];
	char szName[1];
	szName[0] =pBuf[12] ;
	szName[1] ='\0' ;
	int nIndex =(pBuf[0]) *(-1);

	GSM_File *file = new GSM_File;
	ZeroMemory(file,sizeof(GSM_File));
	file->Folder		= false;
	file->Level		= nParentIndex;
	sprintf((char*)szTemp,(char*)pBuf+53);
	EncodeUnicode(file->Name,szTemp,strlen((char*)szTemp));
	itoa(nIndex,(char*)file->ID_FullName,10);
	sprintf(szFileName,"\\%d.%s",pBuf[0],szName);
	EncodeUnicode(uFileName,(unsigned char*)szFileName,strlen(szFileName));
	CopyUnicodeString(file->wFileFullPathName,pHomedir);
	UnicodeCat(file->wFileFullPathName,uFileName);
	theApp.m_MMSFileInfoList.AddTail(file);

	theApp.m_FilesLocations[theApp.m_FilesLocationsUsed]		= nIndex;
	theApp.m_FilesLocationsUsed++;*/
}
GSM_Error GetFileListinFolder(GSM_File *Folder)
{
	GSM_Error		error = ERR_NONE;;
	unsigned char  szTemp[MAX_PATH];
	RemoveAllChileItem(Folder);
	if(stricmp((char*)Folder->ID_FullName,"1") ==0) //Root index :1
	{
		GSM_File *file = new GSM_File;
		ZeroMemory(file,sizeof(GSM_File));
		file->Folder		= true;
		file->Level		= 1;//parentIndex
		sprintf((char*)szTemp,"Messages");
		EncodeUnicode(file->Name,szTemp,strlen((char*)szTemp));
		itoa(10,(char*)file->ID_FullName,10);
		theApp.m_MMSFileInfoList.AddTail(file);

		theApp.m_FilesLocations[theApp.m_FilesLocationsUsed]		= 10;
		theApp.m_FilesLocationsUsed++;
	}
	else if(stricmp((char*)Folder->ID_FullName,"10") ==0) //Messages index :10
	{
/*{
			GSM_File *file = new GSM_File;
			ZeroMemory(file,sizeof(GSM_File));
			file->Folder		= true;
			file->Level		= 10;//parentIndex
			sprintf((char*)szTemp,"INBOX");
			EncodeUnicode(file->Name,szTemp,strlen((char*)szTemp));
			itoa(100,(char*)file->ID_FullName,10);
			theApp.m_MMSFileInfoList.AddTail(file);

			theApp.m_FilesLocations[theApp.m_FilesLocationsUsed]		= 100;
			theApp.m_FilesLocationsUsed++;
		}

		{
			GSM_File *file = new GSM_File;
			ZeroMemory(file,sizeof(GSM_File));
			file->Folder		= true;
			file->Level		= 10;//parentIndex
			sprintf((char*)szTemp,"OUTBOX");
			EncodeUnicode(file->Name,szTemp,strlen((char*)szTemp));
			itoa(102,(char*)file->ID_FullName,10);
			theApp.m_MMSFileInfoList.AddTail(file);

			theApp.m_FilesLocations[theApp.m_FilesLocationsUsed]		= 102;
			theApp.m_FilesLocationsUsed++;
		}
*/
		{
			GSM_File *file = new GSM_File;
			ZeroMemory(file,sizeof(GSM_File));
			file->Folder		= true;
			file->Level		= 10;//parentIndex
			sprintf((char*)szTemp,"DRAFTS");
			EncodeUnicode(file->Name,szTemp,strlen((char*)szTemp));
			itoa(101,(char*)file->ID_FullName,10);
			theApp.m_MMSFileInfoList.AddTail(file);

			theApp.m_FilesLocations[theApp.m_FilesLocationsUsed]		= 101;
			theApp.m_FilesLocationsUsed++;
		}
	/*{
			GSM_File *file = new GSM_File;
			ZeroMemory(file,sizeof(GSM_File));
			file->Folder		= true;
			file->Level		= 10;//parentIndex
			sprintf((char*)szTemp,"SEND");
			EncodeUnicode(file->Name,szTemp,strlen((char*)szTemp));
			itoa(103,(char*)file->ID_FullName,10);
			theApp.m_MMSFileInfoList.AddTail(file);

			theApp.m_FilesLocations[theApp.m_FilesLocationsUsed]		= 103;
			theApp.m_FilesLocationsUsed++;
		}

		{
			GSM_File *file = new GSM_File;
			ZeroMemory(file,sizeof(GSM_File));
			file->Folder		= true;
			file->Level		= 10;//parentIndex
			sprintf((char*)szTemp,"TEMPLATES");
			EncodeUnicode(file->Name,szTemp,strlen((char*)szTemp));
			itoa(104,(char*)file->ID_FullName,10);
			theApp.m_MMSFileInfoList.AddTail(file);

			theApp.m_FilesLocations[theApp.m_FilesLocationsUsed]		= 104;
			theApp.m_FilesLocationsUsed++;
		}*/

	}
	else
	{
		int nFolderIndex = 0;
		if(stricmp((char*)Folder->ID_FullName,"100") ==0) //inbox
		   nFolderIndex = 1;
		else if(stricmp((char*)Folder->ID_FullName,"101") ==0) //DRAFTS 
	 	   nFolderIndex = 8;
		else if(stricmp((char*)Folder->ID_FullName,"102") ==0) //OUTBOX
	 	   nFolderIndex = 2;
		else if(stricmp((char*)Folder->ID_FullName,"103") ==0) //SEND
	 	   nFolderIndex = 4;
		else if(stricmp((char*)Folder->ID_FullName,"104") ==0) //TEMPLATES
	 	   nFolderIndex = 64;
		if(nFolderIndex !=0)
		{
			unsigned char req[500];

			ReplymsgType.nCount = 1;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+EMMSFS=");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;
			sprintf((char*)req , "AT+EMMSFS=%d,2\r",nFolderIndex);
			error = theApp.m_pWriteCommandfn (req, strlen((char*)req), 0x00, 8, false,NULL,&ReplymsgType,MTK_ReplyMMSFloderInfo);
			if(error == ERR_NONE)
			{
				if(theApp.m_MMSFolderInfo.nMsgCount)
				{
					GSM_File file;
					ZeroMemory(&file,sizeof(GSM_File));
					CopyUnicodeString(file.wFileFullPathName,theApp.m_MMSFolderInfo.uFileinfo);
					error = OBEX_GetFilePart(&file,NULL);
					int nFileSize = file.Used;
					int nPos = 0, nLastPos =file.Used-96; 
					int nParentIndex = atoi((char*)Folder->ID_FullName);
					unsigned char Buffer[96];
					while(nPos >=0 && nPos<=nLastPos)
					{
						memcpy(Buffer,file.Buffer+nPos,96);
						AddFileToFileList(Buffer,theApp.m_MMSFolderInfo.uHomedir,nParentIndex);
						nPos+=96;

					}
				}
			}
			else
			{
				int a=1;
			}
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT\r");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;
			error = theApp.m_pWriteCommandfn ((unsigned char *)"AT\r", 3, 0x00, 4, false,NULL,&ReplymsgType,ATGEN_GenericReply);
			Sleep(100);
		}
	}
	return error;
}

GSM_File* FindMMSFileInfoByIndex(int nIndex)
{
	POSITION pos = theApp.m_MMSFileInfoList.GetHeadPosition();
	while(pos)
	{
		GSM_File* fileinfo = (GSM_File* )theApp.m_MMSFileInfoList.GetNext(pos);
		if(fileinfo)
		{
			int nfileindex = atoi((char*)fileinfo->ID_FullName);
			if(nfileindex == nIndex)
				return fileinfo;
		}
	}
	return NULL;
}
GSM_Error WINAPI GetNextFileFolder(GSM_File *File, bool start)
{
	GSM_Error		error = ERR_NONE;;
	unsigned char  szTemp[MAX_PATH];
	if (start)
	{
		GSM_File *pMMSFile = FindMMSFileInfoByIndex(1);
		if(pMMSFile == NULL)
		{
			GSM_File *file = new GSM_File;
			ZeroMemory(file,sizeof(GSM_File));
			file->Folder		= true;
			file->Level		= -1;
			sprintf((char*)szTemp,"ROOT");
			EncodeUnicode(file->Name,szTemp,strlen((char*)szTemp));
			itoa(1,(char*)file->ID_FullName,10);
			theApp.m_MMSFileInfoList.AddTail(file);
		}

		theApp.m_FilesLocationsUsed 	= 1;
		theApp.m_FilesLocationsCurrent 	= 0;
		theApp.m_FilesLocations[0]		= 0x01;
	}

	if (theApp.m_FilesLocationsCurrent == theApp.m_FilesLocationsUsed) return ERR_EMPTY;

	sprintf((char*)File->ID_FullName,"%i",theApp.m_FilesLocations[theApp.m_FilesLocationsCurrent]);
	GSM_File *pFile = FindMMSFileInfoByIndex(theApp.m_FilesLocations[theApp.m_FilesLocationsCurrent]);
	if(pFile == NULL) return ERR_EMPTY;
	memcpy(File,pFile,sizeof(GSM_File));

//	File->Level	= theApp.m_FilesLevels[theApp.m_FilesLocationsCurrent];
	theApp.m_FilesLocationsCurrent++;
//	if(File->Folder == true)
//		error = GetFileListinFolder(File);
	return error;

}
GSM_Error WINAPI GetFilePart(GSM_File *File)
{
	GSM_File *pGetfile = FindMMSFileInfoByIndex(atoi((char*)File->ID_FullName));
	GSM_Error err =   OBEX_GetFilePart(pGetfile,NULL);
	if(err == ERR_NONE)
	{
		memcpy(File,pGetfile,sizeof(GSM_File));
		return ERR_EMPTY;
	}
	return err;
}
GSM_Error WINAPI AddFilePart(GSM_File *File, int *Pos)
{
	GSM_Error err =ERR_NONE;
	GSM_File tempFile;
	ZeroMemory(&tempFile,sizeof(GSM_File));
	CopyUnicodeString(tempFile.wFileFullPathName,theApp.m_MMSFolderInfo.uHomedir);

	char szFileName[MAX_PATH];
	unsigned char uFileName[MAX_PATH];
	sprintf(szFileName,"\\temp.m");
	EncodeUnicode(uFileName,(unsigned char *)szFileName,strlen(szFileName));
	UnicodeCat(tempFile.wFileFullPathName,uFileName);

	GSM_File PathFile;
	ZeroMemory(&PathFile,sizeof(GSM_File));
	CopyUnicodeString(PathFile.wFileFullPathName,theApp.m_MMSFolderInfo.uHomedir);
	OBEX_SetFolderPath(&PathFile);

	OBEX_DeleteFile(&tempFile);

	tempFile.Used = File->Used;
	tempFile.Buffer =(unsigned char *)realloc(tempFile.Buffer,tempFile.Used);
	memcpy(tempFile.Buffer,File->Buffer,File->Used);
	err = OBEX_AddFilePart(&tempFile,Pos,NULL);
	free(tempFile.Buffer);
	*Pos = File->Used;
	if(err = ERR_NONE)
	{
		unsigned char req[500];

		int nUniLen = UnicodeLength(tempFile.wFileFullPathName);
		if(nUniLen <=120)
		{
			char szFileName[500];
			err = MTK_SetCharset(true);
			if(err !=ERR_NONE) return err;

			EncodeHexUnicode((unsigned char *)szFileName, tempFile.wFileFullPathName,nUniLen);
			sprintf((char*)req, "AT+EMMSEXE=1,\"%s\"\r",szFileName);
		}
		else
		{
			err = MTK_SetCharset(false);
			if(err !=ERR_NONE) return err;
			unsigned char szASCIIName[500];
			DecodeUnicode(tempFile.wFileFullPathName,szASCIIName);
			sprintf((char*)req, "AT+EMMSEXE=1,\"%s\"\r",(char*)szASCIIName);
		}

		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+EMMSEXE=");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		err = theApp.m_pWriteCommandfn (req, strlen((char*)req), 0x00, 8, false,NULL,&ReplymsgType,ATGEN_GenericReply);

	}
	return err;	
}
GSM_Error WINAPI GetFileSystemStatus(GSM_FileSystemStatus *status)
{
	GSM_Error error = ERR_NONE;
	status->Used = 0;
	status->Free = -1;
/*
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+EMMSFS=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	unsigned char req[500];
	sprintf((char*)req , "AT+EMMSFS=8,2\r");
	error = theApp.m_pWriteCommandfn (req, strlen((char*)req), 0x00, 8, false,NULL,&ReplymsgType,MTK_ReplyMMSFloderInfo);
	if(error == ERR_NONE)
	{
		status->Used = theApp.m_MMSFolderInfo.nMsgCount;
		status->Free = -1;

	}*/
	return error;

}
GSM_Error WINAPI DeleteFilePart(unsigned char *ID)
{
	return ERR_NOTSUPPORTED;
}
GSM_Error WINAPI AddFolder(GSM_File *File)
{
	return ERR_NOTSUPPORTED;
}

GSM_Error WINAPI SetFilesLocations(int Locations) // == SetFolderPath
{
	GSM_File *pMMSFile = FindMMSFileInfoByIndex(1);
	if(pMMSFile == NULL)
	{
		unsigned char szTemp[MAX_PATH];
		GSM_File *file = new GSM_File;
		ZeroMemory(file,sizeof(GSM_File));
		file->Folder		= true;
		file->Level		= -1;
		sprintf((char*)szTemp,"ROOT");
		EncodeUnicode(file->Name,szTemp,strlen((char*)szTemp));
		itoa(1,(char*)file->ID_FullName,10);
		theApp.m_MMSFileInfoList.AddTail(file);
	}
	theApp.m_FilesLocationsUsed 	= 1;
	theApp.m_FilesLocationsCurrent 	= 0;
	theApp.m_FilesLocations[0]		= Locations;

	return ERR_NONE;
}
int WINAPI GetObjectSubList(int argv[])
{
	int nLocation = theApp.m_FilesLocationsCurrent-1;
 
	if (nLocation == theApp.m_FilesLocationsUsed) return 0;

	GSM_File *pFile = FindMMSFileInfoByIndex(theApp.m_FilesLocations[nLocation]);
	if(pFile == NULL) return 0;
	if(pFile->Folder == true)
		 GetFileListinFolder(pFile);
	else
		return 0;
	
	int nfile =0;
	int i=0;
	if(theApp.m_FilesLocationsUsed)
	{
		nfile=theApp.m_FilesLocationsUsed -1;
		for(i=0 ;i<nfile ;i++)
		{
			argv[i]=theApp.m_FilesLocations[i+1];
		}

	}
	return nfile;
}

