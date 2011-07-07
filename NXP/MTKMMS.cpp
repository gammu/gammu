#include "stdafx.h"
#include "MTK.h"
#include "commfun.h"
#include "coding.h"
#include "MTKfundef.h"

extern CMTKApp theApp;
extern GSM_Reply_MsgType ReplymsgType;
GSM_Error MTK_ReplyAddMMSFile(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	MTK_GetReplyStatue(&msg,&Replynfo);
	char szHexId[MAX_PATH];
	char szName[MAX_PATH];
	szName[0]=0x73;
	szName[1]='\0';


	char			*pos;
	unsigned char		buffer[2000];
	char		szFileName[MAX_PATH];
	unsigned char		uFileName[MAX_PATH];
	char szIndex[MAX_PATH];
	int nRet = 0,nIndex;
	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		pos = strstr((char*)msg.Buffer, "+EMMSEXE:");
		if (pos == NULL)		return ERR_UNKNOWN;
		pos += 9; 
		pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
		nRet = atoi((char*)buffer);
		if(nRet != 0) 	return ERR_UNKNOWN;
		pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
		nIndex = atoi((char*)buffer);

		theApp.m_File->Folder		= false;
		theApp.m_File->Level		= 2;
		sprintf(szIndex,"%d",nIndex);
		EncodeUnicode(theApp.m_File->ID_FullName,(unsigned char*)szIndex,strlen(szIndex)+1);

		itoa(nIndex,szHexId,16);
		sprintf(szFileName,"\\%s.%s",szHexId,szName);
		EncodeUnicode(uFileName,(unsigned char*)szFileName,strlen(szFileName));
		CopyUnicodeString(theApp.m_File->wFileFullPathName,theApp.m_MMSFolderInfo.uHomedir);
		UnicodeCat(theApp.m_File->wFileFullPathName,uFileName);

		return ERR_NONE;
		break;
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(Replynfo);
	case AT_Reply_Error:
		return ERR_UNKNOWNRESPONSE;
	case AT_Reply_CMSError:
 	        return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}
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
GSM_Error WINAPI MMSInit()
{
	return ERR_NONE;
}
GSM_Error WINAPI MMSTerminate()
{
	return ERR_NONE;
}

GSM_Error WINAPI DeleteMMSFile(GSM_File *file,int ControlType)
{
	if( ControlType==Type_End) return ERR_NONE;
	GSM_Error err =ERR_NONE;
	unsigned char req[500];

	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+EMMSEXE=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;

	unsigned char szIDFullName[800];
	DecodeUnicode(file->ID_FullName,szIDFullName);

	sprintf((char*)req, "AT+EMMSEXE=0,0,%d\r",atoi((char*)szIDFullName));
	err = theApp.m_pWriteCommandfn (req, strlen((char*)req), 0x00, 8, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	return err;
}
GSM_Error WINAPI AddMMSFilePart(GSM_File *ParentFile,GSM_File *File,int (*pGetStatusfn)(int nCur,int nTotal),int ControlType)
{
	if( ControlType==Type_End) return ERR_NONE;
	int nPos =0;
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
	err = OBEX_AddFilePart(&tempFile,&nPos,pGetStatusfn);
	free(tempFile.Buffer);
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
		theApp.m_File = File;
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+EMMSEXE=");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		err = theApp.m_pWriteCommandfn (req, strlen((char*)req), 0x00, 8, false,NULL,&ReplymsgType,MTK_ReplyAddMMSFile);

	}
	return err;	
}
GSM_Error WINAPI GetMMSFilePart(GSM_File *File,int (*pGetStatusfn)(int nCur,int nTotal),int Control)
{
	if( Control==Type_End) return ERR_NONE;
	GSM_Error err =   OBEX_GetFilePart(File,pGetStatusfn);
	return err;
}
void AddFileToFileList(unsigned char* pBuf,unsigned char *pHomedir)
{
	char szId[200];
	char szHexId[200];
	char szFileName[200];
	unsigned char uFileName[200];
	MMSFileInfo fileinfo;
	memcpy(&fileinfo,pBuf,sizeof(MMSFileInfo));

	int nIndex =fileinfo.nMsgId ;

	char szName[2];
	szName[0] =fileinfo.szext[0] ;
	szName[1] ='\0' ;

	ZeroMemory(&theApp.m_Files[theApp.m_FilesLocationsUsed],sizeof(GSM_File));
	theApp.m_Files[theApp.m_FilesLocationsUsed].Folder		= false;
	theApp.m_Files[theApp.m_FilesLocationsUsed].Level		= 2;

//	DecodeUTF2String((char*)theApp.m_Files[theApp.m_FilesLocationsUsed].Name,fileinfo.szName,strlen((char*)fileinfo.szName)	);
	DecodeUTF8ToUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].Name,fileinfo.szName,strlen((char*)fileinfo.szName)	);
	sprintf(szId,"%d",nIndex);
	EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,(unsigned char*)szId,strlen(szId)+1);

	itoa(fileinfo.nMsgId,szHexId,16);
	sprintf(szFileName,"\\%s.%s",szHexId,szName);
	EncodeUnicode(uFileName,(unsigned char*)szFileName,strlen(szFileName));
	CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].wFileFullPathName,pHomedir);
	UnicodeCat(theApp.m_Files[theApp.m_FilesLocationsUsed].wFileFullPathName,uFileName);
	theApp.m_FilesLocationsUsed++;
}

GSM_Error WINAPI GMMSFileList(GSM_File *ParentFile, GSM_File *childFile,bool bStart)
{
	GSM_Error error = ERR_NONE;
	unsigned char  szTemp[MAX_PATH];
	if(ParentFile->Folder == FALSE)
		return ERR_BUG;
	if(bStart)
	{
		theApp.m_FilesLocationsUsed = 0;
		theApp.m_FilesLocationsCurrent 	= 0;
		if(UnicodeLength(ParentFile->wFileFullPathName) == 0) //root path
		{
			ZeroMemory(&theApp.m_Files[theApp.m_FilesLocationsUsed],sizeof(GSM_File));
			theApp.m_Files[theApp.m_FilesLocationsUsed].Folder		= true;
			theApp.m_Files[theApp.m_FilesLocationsUsed].Level		= 1;
		//	sprintf((char*)theApp.m_Files[theApp.m_FilesLocationsUsed].Name,"Inbox");
		//	sprintf((char*)theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,"-1");
			sprintf((char*)szTemp,"Inbox");
		
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].Name,szTemp,strlen((char*)szTemp));
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,(unsigned char*)"-1",2);
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].wFileFullPathName,szTemp,strlen((char*)szTemp));
			theApp.m_Files[theApp.m_FilesLocationsUsed].ReadOnly = true;
			theApp.m_FilesLocationsUsed++;

			ZeroMemory(&theApp.m_Files[theApp.m_FilesLocationsUsed],sizeof(GSM_File));
			theApp.m_Files[theApp.m_FilesLocationsUsed].Folder		= true;
			theApp.m_Files[theApp.m_FilesLocationsUsed].Level		= 1;
		//	sprintf((char*)theApp.m_Files[theApp.m_FilesLocationsUsed].Name,"Outbox");
		//	sprintf((char*)theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,"-2");
			sprintf((char*)szTemp,"Outbox");
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].Name,szTemp,strlen((char*)szTemp));
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,(unsigned char*)"-2",2);
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].wFileFullPathName,szTemp,strlen((char*)szTemp));
			theApp.m_Files[theApp.m_FilesLocationsUsed].ReadOnly = true;
			theApp.m_FilesLocationsUsed++;

			ZeroMemory(&theApp.m_Files[theApp.m_FilesLocationsUsed],sizeof(GSM_File));
			theApp.m_Files[theApp.m_FilesLocationsUsed].Folder		= true;
			theApp.m_Files[theApp.m_FilesLocationsUsed].Level		= 1;
		//	sprintf((char*)theApp.m_Files[theApp.m_FilesLocationsUsed].Name,"Send");
	//		sprintf((char*)theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,"-3");
			sprintf((char*)szTemp,"Send");
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].Name,szTemp,strlen((char*)szTemp));
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,(unsigned char*)"-3",2);
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].wFileFullPathName,szTemp,strlen((char*)szTemp));
			theApp.m_Files[theApp.m_FilesLocationsUsed].ReadOnly = true;
			theApp.m_FilesLocationsUsed++;

			ZeroMemory(&theApp.m_Files[theApp.m_FilesLocationsUsed],sizeof(GSM_File));
			theApp.m_Files[theApp.m_FilesLocationsUsed].Folder		= true;
			theApp.m_Files[theApp.m_FilesLocationsUsed].Level		= 1;
	//		sprintf((char*)theApp.m_Files[theApp.m_FilesLocationsUsed].Name,"Drafts");
	//		sprintf((char*)theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,"-4");
			sprintf((char*)szTemp,"Drafts");
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].Name,szTemp,strlen((char*)szTemp));
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,(unsigned char*)"-4",2);
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].wFileFullPathName,szTemp,strlen((char*)szTemp));
			theApp.m_Files[theApp.m_FilesLocationsUsed].ReadOnly = false;
			theApp.m_FilesLocationsUsed++;

			ZeroMemory(&theApp.m_Files[theApp.m_FilesLocationsUsed],sizeof(GSM_File));
			theApp.m_Files[theApp.m_FilesLocationsUsed].Folder		= true;
			theApp.m_Files[theApp.m_FilesLocationsUsed].Level		= 1;
	//		sprintf((char*)theApp.m_Files[theApp.m_FilesLocationsUsed].Name,"Templates");
	//		sprintf((char*)theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,"-5");
			sprintf((char*)szTemp,"Templates");
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].Name,szTemp,strlen((char*)szTemp));
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,(unsigned char*)"-5",2);
			EncodeUnicode(theApp.m_Files[theApp.m_FilesLocationsUsed].wFileFullPathName,szTemp,strlen((char*)szTemp));
			theApp.m_Files[theApp.m_FilesLocationsUsed].ReadOnly = true;
			theApp.m_Files[theApp.m_FilesLocationsUsed].System = true;// Can not delete
			theApp.m_FilesLocationsUsed++;
		}
		else
		{
			int nFolderIndex = 0;
			char szIDFullName[MAX_PATH];
			DecodeUnicode(ParentFile->ID_FullName,(unsigned char*)szIDFullName);
			if(stricmp(szIDFullName,"-1") ==0) //inbox
			   nFolderIndex = 1;
			else if(stricmp(szIDFullName,"-4") ==0) //DRAFTS 
	 		   nFolderIndex = 8;
			else if(stricmp(szIDFullName,"-2") ==0) //OUTBOX
	 		   nFolderIndex = 2;
			else if(stricmp(szIDFullName,"-3") ==0) //SEND
	 		   nFolderIndex = 4;
			else if(stricmp(szIDFullName,"-5") ==0) //TEMPLATES
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
						unsigned char Buffer[96];
						while(nPos >=0 && nPos<=nLastPos)
						{
							memcpy(Buffer,file.Buffer+nPos,96);
							AddFileToFileList(Buffer,theApp.m_MMSFolderInfo.uHomedir);
							nPos+=96;

						}
						if(file.Buffer )
							free(file.Buffer );
					}
				}
				wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT\r");
				ReplymsgType.CheckInfo[0].subtypechar = 0;
				ReplymsgType.CheckInfo[0].subtype = 0x00;
				theApp.m_pWriteCommandfn ((unsigned char *)"AT\r", 3, 0x00, 4, false,NULL,&ReplymsgType,ATGEN_GenericReply);
				Sleep(100);
			}//if(nFolderIndex !=0)
		}//not root path
	}//bStart
	if(theApp.m_FilesLocationsCurrent>=theApp.m_FilesLocationsUsed) return ERR_EMPTY;
	memcpy(childFile,&theApp.m_Files[theApp.m_FilesLocationsCurrent],sizeof(GSM_File));
	theApp.m_FilesLocationsCurrent++;
	return error;
}

