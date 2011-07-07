#include "stdafx.h"
#include "MTK.h"
#include "commfun.h"

#include "MTKfundef.h"
#include "obexgenfun.h"		// main symbols
#include "AtGenFundef.h"		// main symbols
#include "coding.h"		// main symbols


extern CMTKApp theApp;
extern GSM_Reply_MsgType ReplymsgType;
 char Base64Alphabet1[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
unsigned char GetBase64Index1(char ch)
{
	unsigned char i, v_Return = -1;
	for(i = 0; i < 64; i++)
	{
		if(Base64Alphabet1[(int)i] == ch)
		{
			v_Return = i;
			break;
		}
	}
	return v_Return;
}
int Decode1(const char * szDecoding, char * szOutput)
{
	char			Ch;
	char			chCurrentIndex;
	unsigned int	i,nCurrentBits;
	unsigned char	chTargetChar;
	char			*pl_DeData;
	nCurrentBits = 0;
	chTargetChar = '\0';
	pl_DeData = szOutput;
	for(i = 0; i < strlen(szDecoding); i++)
	{
		Ch = szDecoding[i];
		if( Ch == '=')
		{
			nCurrentBits = 0;
	continue;
		}
		chCurrentIndex = GetBase64Index1(Ch);
		if(chCurrentIndex < 0) 	return 0;
		if((nCurrentBits+6) >= 8)
		{
			chTargetChar |= (chCurrentIndex >> (6-(8-nCurrentBits)));
			*pl_DeData = chTargetChar;
			pl_DeData++;
			nCurrentBits = nCurrentBits - 2;
			chTargetChar = chCurrentIndex << (8 - nCurrentBits);
		}
		else
		{
			chTargetChar |= (chCurrentIndex << (2-nCurrentBits));
			nCurrentBits += 6;
		}
	}
	return pl_DeData-szOutput;
}

int Encode1(const unsigned char *pp_Data, int v_DataLen,unsigned char *pp_EncodeData)
{
	unsigned char	v_FirstByte,v_SecondByte,v_ThirdByte, v_FourthByte;
	unsigned char	*pl_EnData;
	int 			i,v_Left;	
	pl_EnData = (unsigned char *)pp_EncodeData;
	for(i = 0; i < v_DataLen / 3; i++)
	{
		v_FirstByte	= pp_Data[0] >> 2;
		v_SecondByte	= ((pp_Data[0] & 0x03) << 4) | (pp_Data[1] >> 4) ;
		v_ThirdByte	= ((pp_Data[1] & 0x0F) << 2) | (pp_Data[2] >> 6);
		v_FourthByte	= pp_Data[2] & 0x3F;
		pl_EnData[0] = Base64Alphabet1[(int)v_FirstByte];
        pl_EnData[1] = Base64Alphabet1[(int)v_SecondByte];
		pl_EnData[2] = Base64Alphabet1[(int)v_ThirdByte];
		pl_EnData[3] = Base64Alphabet1[(int)v_FourthByte];

		pp_Data += 3;
		pl_EnData += 4;	 
	}
	v_Left = v_DataLen % 3;
	if(v_Left == 1)
	{
		v_FirstByte = pp_Data[0] >> 2;
		v_SecondByte = (pp_Data[0] & 0x03) << 4;
		pl_EnData[0] = Base64Alphabet1[(int)v_FirstByte];
		pl_EnData[1] = Base64Alphabet1[(int)v_SecondByte];
		pl_EnData[2] = '=';
		pl_EnData[3] = '=';
		pp_Data += 1;
		pl_EnData += 4;
	}
	else if(v_Left == 2)
	{
		v_FirstByte	= pp_Data[0] >> 2;
		v_SecondByte	= ((pp_Data[0] & 0x03) << 4) | ((pp_Data[1] & 0xF0) >> 4);
		v_ThirdByte	= (pp_Data[1] & 0x0F) << 2; 
		pl_EnData[0] = Base64Alphabet1[(int)v_FirstByte];
		pl_EnData[1] = Base64Alphabet1[(int)v_SecondByte];
		pl_EnData[2] = Base64Alphabet1[(int)v_ThirdByte];
		pl_EnData[3] = '=';
		pp_Data += 2;
		pl_EnData += 4;	 
	}
	
	return pl_EnData - pp_EncodeData ;
}
void bcd2ascii1( char *p_bcd,char *p_ascii,int ascii_len)
{
	unsigned char i,j,len_char,temp_number;
	memset(p_ascii,'\0',ascii_len);
	len_char=*p_bcd;
	j=1;
	if(len_char%2)
	{
		for(i=0;i<len_char-1;i=i+2)
		{
				temp_number=*(p_bcd+j);
				p_ascii[i]=((temp_number & 0xf0)>>4)+'0';
				p_ascii[i+1]=(temp_number & 0x0f)+'0';
				j++;
		}
		temp_number=*(p_bcd+j);
		p_ascii[len_char-1]=temp_number;
	}
	else
	{
		for(i=0;i<len_char;i=i+2)
		{
				temp_number=*(p_bcd+j);
				p_ascii[i]=((temp_number & 0xf0)>>4)+'0';
				p_ascii[i+1]=(temp_number & 0x0f)+'0';
				j++;
		}
	}
	p_ascii[len_char]='\0';
	len_char=strlen(p_ascii);
	for(i=0;i<len_char;i++)
	{
		if(p_ascii[i]==(10+'0'))
		{
			p_ascii[i]='p';
		}
		if(p_ascii[i]==(11+'0'))
		{
			p_ascii[i]='w';
		}
		if(p_ascii[i]==(12+'0'))
		{
			p_ascii[i]='+';
		}
		if(p_ascii[i]==(13+'0'))
		{
			p_ascii[i]='*';
		}
		if(p_ascii[i]==(14+'0'))
		{
			p_ascii[i]='#';
		}
	}
}
void  ascii2bcd1(char *p_ascii,char *p_bcdcode,int bcdcode_len)
{
	unsigned char i,j,len_char;
	memset(p_bcdcode,'\0',bcdcode_len);
	len_char=strlen(p_ascii);
	for(i=0;i<len_char;i++)
	{
		if((*(p_ascii+i))=='p')
		{
			*(p_ascii+i)=(10+'0');
		}
		else if((*(p_ascii+i))=='w')
		{
			*(p_ascii+i)=(11+'0');
		}
		else if((*(p_ascii+i))=='+')
		{
			*(p_ascii+i)=(12+'0');
		}
		else if((*(p_ascii+i))=='*')
		{
			*(p_ascii+i)=(13+'0');
		}
		else if((*(p_ascii+i))=='#')
		{
			*(p_ascii+i)=(14+'0');
		}
	}
	j=1;
	p_bcdcode[0]=len_char;

	if(len_char%2)
	{
		for(i=0;i<len_char-1;i=i+2)
		{
			p_bcdcode[j]=(*(p_ascii+i)-'0')*16+(*(p_ascii+i+1)-'0');
			j++;
		}
		p_bcdcode[j]=*(p_ascii+i);

	}
	else
	{
		for(i=0;i<len_char-1;i=i+2)
		{
			p_bcdcode[j]=(*(p_ascii+i)-'0')*16+(*(p_ascii+i+1)-'0');
			j++;
		}
	}
}
char CheckSum1(char* pSrc, int nLen)
{

       if(nLen == 0)return 0;
       BYTE ch = *pSrc;
       for(int i = 1; i < nLen; i++)
       {
              ch = ch ^ (*(pSrc+i));
       }
       return ch;
}

GSM_Error MTK_ReplyGetFilePart(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	MTK_GetReplyStatue(&msg,&Replynfo);

	char			*pos;
	unsigned char		buffer[2000],buffer2[2000];
	int nIndex ,nEof,nSize,old,tol;
	nIndex=nEof=nSize = 0;
	switch (Replynfo.ReplyState) {
	case AT_Reply_Continue: //peggy 0201 add
	case AT_Reply_OK:
		pos = strstr((char*)msg.Buffer, "+EFSR:");
		if (pos == NULL)//return ERR_UNKNOWN;//peggy
		{
			if(Replynfo.ReplyState == AT_Reply_Continue)
				return ERR_UNKNOWN;
			else return ERR_NONE; 
		}
		pos += 6; 
		pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
		nIndex = atoi((char*)buffer);
		pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
		nEof = atoi((char*)buffer);
		pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
		nSize = atoi((char*)buffer);
		while (*pos != '"') pos++;
		pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
		DecodeHexBin(buffer2, buffer+1, strlen((char*)buffer)-2);
	
		old = theApp.m_File->Used;
		tol = old +	nSize;
		theApp.m_File->Used = tol;

		theApp.m_File->Buffer = (unsigned char *)realloc(theApp.m_File->Buffer,tol);
		memcpy(theApp.m_File->Buffer+old,buffer2,nSize); 
		if(theApp.m_pGetFiletatusfn)
			theApp.m_pGetFiletatusfn(tol,theApp.m_File->FileSize);

		if(Replynfo.ReplyState == AT_Reply_Continue)
			return ERR_NEEDANOTHEDATA;
		else 
		{
			if(nEof)	return ERR_NONE; 
			return ERR_UNKNOWNRESPONSE;
		}
		break;
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(Replynfo);
	case AT_Reply_Error:
 		smprintf(theApp.m_pDebuginfo, "Error - too high location ?\n");
		return ERR_INVALIDLOCATION;
	case AT_Reply_CMSError:
 	        return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}
GSM_Error MTK_ReplyGetFreeSize(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	MTK_GetReplyStatue(&msg,&Replynfo);
	unsigned char		buffer[500];
	char			*pos;
	switch (Replynfo.ReplyState) 
	{
	case AT_Reply_Continue:
	case AT_Reply_OK:
		{
			pos = strstr((char*)msg.Buffer, "*FREESIZE:");
			if (pos == NULL)	return ERR_UNKNOWN;
			pos += 10; 
			while (*pos && !isdigit(*pos)) pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
			*theApp.m_nFreeSpace = atoi((char*)buffer);
		}
		return ERR_NONE; 
		break;
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(Replynfo);
	case AT_Reply_Error:
 		smprintf(theApp.m_pDebuginfo, "Error - too high location ?\n");
		return ERR_INVALIDLOCATION;
	case AT_Reply_CMSError:
 	        return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}
GSM_Error MTK_ReplyGetMutiFileInfo(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	MTK_GetReplyStatue(&msg,&Replynfo);
	char			*pos;
	unsigned char		buffer[500];
	unsigned char		uName[500];
	unsigned char		szName[500];
	unsigned char       fullpath[500];
	unsigned char       fullcomand[500];
	unsigned char temp[10];
	unsigned char fold[10];
	EncodeUnicode (temp, ( unsigned char *) "/", 1);
	DecodeUnicode(theApp.m_ChangeFolderPath.wFileFullPathName,fullpath);
	sprintf((char *)fullcomand,"*LIST:%s",(char *)fullpath);
	bool bIsContinue=false;
	switch (Replynfo.ReplyState) 
	{
	case AT_Reply_Continue:
	case AT_Reply_OK:
		{
			pos = strstr((char*)msg.Buffer, (char *)fullcomand);
			if (pos == NULL)
			{
				if(Replynfo.ReplyState == AT_Reply_Continue)
					return ERR_UNKNOWN;
				else return ERR_NONE; 
			}
			pos += strlen((char *)fullcomand);
			while (pos)
			{
				while (*pos != ',')
				{
					pos++;
					int nth = pos - (char*)msg.Buffer ;
					if(nth== msg.Length)
					{
						bIsContinue=true;
						break;


					}
				}
				if (bIsContinue)
				{
					break;
				}
			//	pos--;
			///	pos += ATGEN_ExtractOneParameter((unsigned char *)pos, fold);
				pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
				pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
				strcpy((char *)szName,(char *)buffer);
//				int nFold=0;
//				nFold=atoi((char *)fold);
				int nFileSize  = 0;
//				if (nFold==0)
//				{
				pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
				nFileSize=atoi((char *)buffer);
// 				}
// 				if (nFold==1)
// 				{
// 				theApp.m_Files[theApp.m_FilesLocationsUsed].Folder		= true;
// 
// 				}
// 				else
				theApp.m_Files[theApp.m_FilesLocationsUsed].Folder		= false;
				theApp.m_Files[theApp.m_FilesLocationsUsed].Protected   = false;
				theApp.m_Files[theApp.m_FilesLocationsUsed].ReadOnly    = false;
				theApp.m_Files[theApp.m_FilesLocationsUsed].Hidden = false;
				theApp.m_Files[theApp.m_FilesLocationsUsed].System = false;
				theApp.m_Files[theApp.m_FilesLocationsUsed].Level= theApp.m_ChangeFolderPath.Level+1;
				//EncodeUnicode(uName,szName,strlen((char *)szName));//080527libaoliu
				DecodeUTF8ToUnicode(uName,szName,strlen((char *)szName));

				CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].Name,uName);
				
				if(UnicodeLength(theApp.m_ChangeFolderPath.ID_FullName) > 0)
				{
					CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,theApp.m_ChangeFolderPath.ID_FullName);
					UnicodeCat(theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,temp);
					UnicodeCat(theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,uName); 
				}
				else
					CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,uName);
				
				if(UnicodeLength(theApp.m_ChangeFolderPath.wFileFullPathName)> 0)
				{
					CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].wFileFullPathName,theApp.m_ChangeFolderPath.wFileFullPathName);
					UnicodeCat(theApp.m_Files[theApp.m_FilesLocationsUsed].wFileFullPathName,temp);
					UnicodeCat(theApp.m_Files[theApp.m_FilesLocationsUsed].wFileFullPathName,uName); 
				}
				else
				{
					CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].wFileFullPathName,uName);
				}
				theApp.m_Files[theApp.m_FilesLocationsUsed].FileSize = nFileSize;
				
				theApp.m_FilesLocationsUsed++;
			}
			
			if(Replynfo.ReplyState == AT_Reply_Continue)
				return ERR_NEEDANOTHEDATA;
			else return ERR_NONE; 
		}
		break;
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(Replynfo);
	case AT_Reply_Error:
 		smprintf(theApp.m_pDebuginfo, "Error - too high location ?\n");
		return ERR_INVALIDLOCATION;
	case AT_Reply_CMSError:
 	        return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

void MTK_FindNextDir_UnicodePath(unsigned char *Path, unsigned int *Pos, unsigned char *Return)
{
	unsigned char temp1[10];
	EncodeUnicode(temp1,(unsigned char*)"\\",2);

	Return[0] = 0;
	Return[1] = 0;
	while(1) 
	{
		if (Path[(*Pos)*2] == 0x00 && Path[(*Pos)*2+1] == 0x00) break;
		if ((Path[(*Pos)*2] == temp1[0] && Path[(*Pos)*2+1] == temp1[1] ))
		{
			(*Pos)++;
			break;
		}
		int nLen = UnicodeLength(Return);
		Return[(nLen+1)*2] = 0;
		Return[(nLen+1)*2+1] = 0;
		nLen = UnicodeLength(Return);
		Return[nLen*2] =  Path[(*Pos)*2];
		Return[nLen*2+1] =  Path[(*Pos)*2+1];
		(*Pos)++;
	}
}

GSM_Error WINAPI MTK_SetPath(unsigned char *UnicodePath)
{
	GSM_Error error = ERR_NONE;
	unsigned char req[500];
	unsigned char szdest[2000];

	unsigned char temp1[10];
	EncodeUnicode(temp1,(unsigned char*)"\\",2);
	int nUnicodeLength = UnicodeLength(UnicodePath);
	if(UnicodePath[(nUnicodeLength-1)*2] == temp1[0] && UnicodePath[(nUnicodeLength-1)*2+1] == temp1[1])
	{
		UnicodePath[(nUnicodeLength-1)*2] = 0;
		UnicodePath[(nUnicodeLength-1)*2+1] = 0;
		nUnicodeLength = UnicodeLength(UnicodePath);
	}

	if(nUnicodeLength<=120)
	{
		error = MTK_SetCharset(true);
		if(error !=ERR_NONE) return error;
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+EFSF");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		
		EncodeHexUnicode((unsigned char *)szdest, UnicodePath, UnicodeLength(UnicodePath));

		sprintf((char*)req , "AT+EFSF=2,\"%s\"\r",szdest);
		error = theApp.m_pWriteCommandfn (req, strlen((char*)req), 0x00, 8, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	}
	else //available after 04.22
	{
		unsigned char 		Name[500];
		int Pos = 0;
		do {
			MTK_FindNextDir_UnicodePath(UnicodePath,(unsigned int *) &Pos, Name);
			if(UnicodeLength((unsigned char*)Name) == 0)
			{
				//setRootPath
				ReplymsgType.nCount = 1;
				wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+EFSF");
				ReplymsgType.CheckInfo[0].subtypechar = 0;
				ReplymsgType.CheckInfo[0].subtype = 0x00;
				sprintf((char*)req , "AT+EFSF=3\r");
				error = theApp.m_pWriteCommandfn (req, strlen((char*)req), 0x00, 8, false,NULL,&ReplymsgType,ATGEN_GenericReply);
			}
			else 
			{
				if(UnicodeLength((unsigned char*)Name )<=120)
				{
					error = MTK_SetCharset(true);
					if(error !=ERR_NONE) return error;
					ReplymsgType.nCount = 1;
					wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+EFSF");
					ReplymsgType.CheckInfo[0].subtypechar = 0;
					ReplymsgType.CheckInfo[0].subtype = 0x00;
					
					EncodeHexUnicode((unsigned char *)szdest, UnicodePath, UnicodeLength(UnicodePath));

					sprintf((char*)req , "AT+EFSF=2,\"%s\"\r",szdest);
					error = theApp.m_pWriteCommandfn (req, strlen((char*)req), 0x00, 8, false,NULL,&ReplymsgType,ATGEN_GenericReply);
				}
				else
				{	
					unsigned char szASCIIName[500];
					DecodeUnicode(Name,szASCIIName);
					int nLen  = strlen((char*)szASCIIName);
					if(nLen == nUnicodeLength)
					{
						error = MTK_SetCharset(false);
						if(error !=ERR_NONE) return error;

						ReplymsgType.nCount = 1;
						wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+EFSF");
						ReplymsgType.CheckInfo[0].subtypechar = 0;
						ReplymsgType.CheckInfo[0].subtype = 0x00;
						CString strPath ;
						strPath.Format("%s",szASCIIName);
						strPath.Replace("\\","\\5C");
						
						sprintf((char*)req , "AT+EFSF=2,\"%s\"\r",strPath);
						error = theApp.m_pWriteCommandfn (req, strlen((char*)req), 0x00, 8, false,NULL,&ReplymsgType,ATGEN_GenericReply);
					}
					else
						error =  ERR_UNKNOWN;
				}

			} //else
			if (error != ERR_NONE) return error;
			if (Pos == UnicodeLength(UnicodePath)) break;
		} while (1);
	}//else
	return error;
}
GSM_Error WINAPI OBEX_SetMode(bool bObexMode,OBEX_Service service)
{
	return ERR_NONE;
}

GSM_Error WINAPI OBEX_DeleteFile(GSM_File *ID)
{
	GSM_Error error = ERR_NONE;
	
	char			        req[2000];
	/*	char			        szFileName[2000];*/
	unsigned char			unFileName[2000];
	unsigned char           unFilePath[2000];
	unsigned char           szFileName[2000];
	unsigned char           szFilePath[2000];
	
	int nLen = UnicodeLength(ID->wFileFullPathName);
	if(nLen<=0) 
		CopyUnicodeString(unFileName,ID->ID_FullName);
	else
		GetFileName_Unicode(ID->wFileFullPathName,unFileName);
	GetFilePath_Unicode(ID->wFileFullPathName,unFilePath);
	DecodeUnicode(unFilePath,szFilePath);
	//DecodeUnicode(unFileName,szFileName);//080527libaoliu
	EncodeUnicode2UTF8(unFileName,szFileName);

	
	int Length=strlen((char *)szFilePath)-1;
	szFilePath[Length]=0;
	sprintf(req, "AT*REMOVE=%s/%s\r",szFilePath,szFileName);
	
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*REMOVE");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	return error;

}
GSM_Error MTK_ReplyAddFileEx(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	MTK_GetReplyStatue(&msg,&Replynfo);

//	char			*start;

	if (theApp.m_EditMode) 
	{
		if (Replynfo.ReplyState != AT_Reply_SMSEdit)
		{
			//	return ERR_UNKNOWN;
		}
		theApp.m_EditMode = false;
		theApp.m_pSetATProtocolDatafn(false,true,0x01);
	//	s->Protocol.Data.AT.EditMode = false;
		return ERR_NONE;
	}

	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
 		return ERR_NONE;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(Replynfo);
	case AT_Reply_Error:
		return ERR_UNKNOWN;
	default:
		return ERR_UNKNOWNRESPONSE;
	}
}
GSM_Error WINAPI MTK_GetFreeSpace(unsigned char *UnicodePath,DWORD& dwsize)
{
	GSM_Error error = ERR_NONE;
	dwsize = -1;
	//08512libaoliu
// 	int nLen = UnicodeLength(UnicodePath);
// 	if(nLen<=0) 
// 		return ERR_UNKNOWN;
// 
// 	unsigned char			uPath[2000];
// 	char			szPath[100];
// 	CopyUnicodeString(uPath,UnicodePath);
// 	uPath[6] = uPath[7] = 0;
// 	DecodeUnicode(uPath,(unsigned char*)szPath);
	unsigned char req[500];

// 	if(_stricmp(szPath,"C:\\") == 0)
// 		sprintf((char*)req,"AT+EFS=67\r");
// 	else if(_stricmp(szPath,"D:\\") == 0)
// 		sprintf((char*)req,"AT+EFS=68\r");
// 	else if(_stricmp(szPath,"E:\\") == 0)
// 		sprintf((char*)req,"AT+EFS=69\r");
// 	else if(_stricmp(szPath,"F:\\") == 0)
// 		sprintf((char*)req,"AT+EFS=70\r");
// 	else if(_stricmp(szPath,"G:\\") == 0)
	sprintf((char*)req,"AT*FREESIZE\r");
/*	else 		return ERR_UNKNOWN;*/
	theApp.m_nFreeSpace = &dwsize;
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*FREESIZE");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;

	error = theApp.m_pWriteCommandfn (req, strlen((char*)req), 0x00, 12, false,NULL,&ReplymsgType,MTK_ReplyGetFreeSize);
	return error;
}
GSM_Error WINAPI OBEX_AddFilePart(GSM_File *File, int *Pos,int (*pGetStatusfn)(int nCur,int nTotal))
{
	GSM_Error error = ERR_NONE;
	DWORD dwfree =-1;
	error = MTK_GetFreeSpace(theApp.m_ChangeFolderPath.wFileFullPathName,dwfree);
	if (error!=ERR_NONE)
	{
		return error;
	}
	if(error == ERR_NONE)
	{
		if(File->Used > dwfree)
			return ERR_FULL;
	}
// 	error = MTK_SetPath(theApp.m_ChangeFolderPath.wFileFullPathName);
// 	if(error !=ERR_NONE) 
// 		return error;//080512libaoliu

	char			        req[5000];
	unsigned char 		wPathName[1000];
	unsigned char 		wFileName[1000];
	GetFilePath_Unicode(File->wFileFullPathName,wPathName);
	GetFileName_Unicode(File->wFileFullPathName,wFileName); // file name

// 	char			        szFileName[2000];
// 	unsigned char			unFileName[2000];
// 	unsigned char			szDelFileName[2000];
// 	unsigned char			Data[2000];
	unsigned char szFullPath[1000];
	unsigned char szFileName[1000];
	//CopyUnicodeString(szFullName,File->wFileFullPathName);
    DecodeUnicode(wPathName,szFullPath);
	//DecodeUnicode(wFileName,szFileName);//080527//libaoliu
	//EncodeUnicode2UTF8(wPathName,szFullPath);
	EncodeUnicode2UTF8(wFileName,szFileName);
	

	int len=strlen((char *)szFullPath);
	szFullPath[len-1]=0;
	sprintf(req,"AT*STARTDL=%s/%s,%d\r",szFullPath,szFileName,File->Used);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*STARTDL");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 12, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if(error !=ERR_NONE)
	{ 
		return error;
	}
	int nWrited = 0 ;
	BOOL bGetConti = TRUE;
	int nWriteDataSize = theApp.m_nPacketSize;
	theApp.m_File = File;
	unsigned char			Data[2000];
	unsigned char           EncodeDataBuffer[2000];
	unsigned char           checkSum[10];
	int i=0;
	while((nWrited < theApp.m_File->Used )&& bGetConti)
	{
		if(nWrited+ nWriteDataSize > theApp.m_File->Used)
			nWriteDataSize = theApp.m_File->Used-nWrited;
		
		memset(Data,0,sizeof(Data));
		memset(EncodeDataBuffer,0,sizeof(EncodeDataBuffer));
		memset(checkSum,0,sizeof(checkSum));
		memcpy(Data,theApp.m_File->Buffer +nWrited,nWriteDataSize);
		char check=CheckSum1((char *)Data,nWriteDataSize);
		Encode1((unsigned char*)&check,1,checkSum);
		Encode1(Data,nWriteDataSize,EncodeDataBuffer);
		sprintf(req,"AT*PUT=%d,%d,%s,%s\r",i,nWriteDataSize,EncodeDataBuffer,checkSum);
		ReplymsgType.nCount = 2;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*PUT=");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*PUT");
		ReplymsgType.CheckInfo[1].subtypechar = 0;
		ReplymsgType.CheckInfo[1].subtype = 0x00;
		error=theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
		if (error != ERR_NONE&& error != ERR_EMPTY) 
		{
		OBEX_DeleteFile(theApp.m_File);
		return ERR_CANCELED;
		 return error;
		}
		i++;
		nWrited +=nWriteDataSize; 
		if(pGetStatusfn)
			bGetConti = pGetStatusfn(nWrited,theApp.m_File->Used);

	}
	sprintf(req,"at*end\r");
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"at*end");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*end:");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error != ERR_NONE&& error != ERR_EMPTY) 
	{
		OBEX_DeleteFile(theApp.m_File);
		return ERR_CANCELED;
		return error;
	}

	if(bGetConti == false && nWrited < theApp.m_File->Used)
	{
		OBEX_DeleteFile(theApp.m_File);
		return ERR_CANCELED;
	}
	File->Modified.Year = 0;
	File->Modified.Month= 0;
	File->Modified.Day= 0;
	File->Modified.Hour= 0;
	File->Modified.Minute= 0;
	File->Modified.Second= 0;
	
	return error;
}
GSM_Error NXP_ReplyGetFileInfo(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);
	char 			*pos;
	long Length=0;
 	switch (Replynfo.ReplyState) 
	{
 	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "Memory info received\n");
		pos =(char*)msg.Buffer;
		if (!pos) return ERR_UNKNOWN;
		pos = strstr((char*)pos, "*STARTUL:");
		pos=pos+9;
		Length=atol(pos);
		Length=Length*2;
		if (theApp.m_unDecodeBase64)
		{
			delete theApp.m_unDecodeBase64;
			theApp.m_unDecodeBase64=NULL;
		}
		else
		{
			theApp.m_unDecodeBase64=new unsigned char[Length];
			theApp.position=(char *)theApp.m_unDecodeBase64;
		}

		return ERR_NONE;
	case AT_Reply_Error:
		return ERR_UNKNOWN;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(Replynfo);
 	default:
		return ERR_UNKNOWNRESPONSE;
	}


	
}
GSM_Error NXP_ReplyGetData(GSM_Protocol_Message msg)
{
	int old ,tol;
	GSM_ATReplayInfo Replynfo;
	GSM_Protocol_Message tempmsg;
	tempmsg.Buffer = (unsigned char *)malloc(101);
	if(msg.Length < 100)
	{
		memcpy(tempmsg.Buffer,msg.Buffer,msg.Length);
		tempmsg.Length = msg.Length;
		tempmsg.BufferUsed = msg.Length+1;
	}
	else
	{
		memcpy(tempmsg.Buffer,msg.Buffer+msg.Length -100,100);
		if(tempmsg.Buffer[99] == 0x00)
		{
			tempmsg.Length = 99;
			tempmsg.BufferUsed =100;
		}
		else
		{
			tempmsg.Length = 100;
			tempmsg.BufferUsed =101;
		}
	}
	MTK_GetReplyStatue(&tempmsg,&Replynfo);
	free(tempmsg.Buffer);
	char			*pos;
	char			*pos2;
	unsigned char		buffer[2000]/*,buffer2[2000]*/;
	long zero=0;
	switch (Replynfo.ReplyState) 
	{
	case AT_Reply_Continue: 
	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "Phonebook entry received\n");
		pos = (char*)msg.Buffer;
		pos=strstr((char*)pos,"*GET:");
		pos=pos+5;
		zero=atol(pos);
		if (zero)
		{
			pos2 = strstr((char*)pos, ",");
			pos=pos2+1;
			ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
			int Length=strlen((char*)buffer);
			unsigned char *encodebuffer=new unsigned char[Length];
		    int l=Decode1((char *)buffer,(char *)encodebuffer);
			memcpy(theApp.position,encodebuffer,l);
			theApp.position=theApp.position+l;
			//int lk=strlen((char *)encodebuffer);
			
			//if (!theApp.isFirst)
			{
			//_mbscpy(theApp.m_unDecodeBase64,encodebuffer);
			//theApp.isFirst=true;
			}
// 			else
// 			{
// 				_mbscat(theApp.m_unDecodeBase64,encodebuffer);
// 			}
		delete []encodebuffer;
		old = theApp.m_File->Used;
		tol = old +zero;
		theApp.m_File->Used = tol;
		if(theApp.m_pGetFiletatusfn)
		{
		theApp.isCancel=theApp.m_pGetFiletatusfn(tol,theApp.m_File->FileSize);
		}
		}
		else
		{
			theApp.isGetZero=true;
		}

		return ERR_NONE;
		break;
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(Replynfo);
	case AT_Reply_Error:
		smprintf(theApp.m_pDebuginfo, "Error - too high location ?\n");
		return ERR_INVALIDLOCATION;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;

}
GSM_Error WINAPI OBEX_GetFilePart(GSM_File *File,int (*pGetStatusfn)(int nCur,int nTotal))
{
	GSM_Error error = ERR_NONE;
	char			req[2000];
//	char			szFileName[2000];
//	unsigned char			unFileName[2000];


	unsigned char 		wPathName[2000];
	unsigned char 		wFileName[2000];
	GetFilePath_Unicode(File->wFileFullPathName,wPathName);
	GetFileName_Unicode(File->wFileFullPathName,wFileName);
	theApp.m_File = File;
	int nLen = UnicodeLength(wFileName);
	if(nLen<=0) 
	{
		CopyUnicodeString(wFileName,File->ID_FullName);
	}
	//	EncodeUnicode(wFileName,File->ID_FullName,strlen((char *)File->ID_FullName));
	    unsigned char szASCIIName[500];
		unsigned char szASCIIPath[100];
		DecodeUnicode(wPathName,szASCIIPath);
	//	DecodeUnicode(wFileName,szASCIIName);//080527libaoliu
		EncodeUnicode2UTF8(wFileName,szASCIIName);

	    theApp.isCancel=true;
	 
		int Length=strlen((char *)szASCIIPath)-1;
		szASCIIPath[Length]=0;
		sprintf(req, "AT*STARTUL=%s/%s\r",szASCIIPath,szASCIIName);
		ReplymsgType.nCount = 2;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*STARTUL=");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*STARTUL");
		ReplymsgType.CheckInfo[1].subtypechar = 0;
		ReplymsgType.CheckInfo[1].subtype = 0x00;
 		error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,NXP_ReplyGetFileInfo);
 	    if (error != ERR_NONE&& error != ERR_EMPTY) return error;
		theApp.m_pGetFiletatusfn = pGetStatusfn;
		theApp.isFirst=false;
	    theApp.isGetZero=false;
		sprintf(req,"at*get\r");
		ReplymsgType.nCount = 2;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"at*get");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*get:");
		ReplymsgType.CheckInfo[1].subtypechar = 0;
		ReplymsgType.CheckInfo[1].subtype = 0x00;
		while (theApp.isCancel)
		{
			if (!theApp.isGetZero)
			{
				error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,NXP_ReplyGetData);
				if (error != ERR_NONE&& error != ERR_EMPTY) 
				{
					if (theApp.m_unDecodeBase64)//081027libaoliu
					{
						delete theApp.m_unDecodeBase64;
						theApp.m_unDecodeBase64=NULL;
					}
					theApp.m_pGetFiletatusfn = NULL;
					return ERR_CANCELED;
					return error;
				}
			}
			else
				break;
	
		}
		if (theApp.isCancel)
		{
		
			if (theApp.isGetZero)
			{
				sprintf(req,"at*end\r");
				ReplymsgType.nCount = 2;
				wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"at*end");
				ReplymsgType.CheckInfo[0].subtypechar = 0;
				ReplymsgType.CheckInfo[0].subtype = 0x00;
				wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*end:");
				ReplymsgType.CheckInfo[1].subtypechar = 0;
				ReplymsgType.CheckInfo[1].subtype = 0x00;
				error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
				if (error != ERR_NONE&& error != ERR_EMPTY) 
				{
					if (theApp.m_unDecodeBase64)//081027libaoliu
					{
						delete theApp.m_unDecodeBase64;
						theApp.m_unDecodeBase64=NULL;
					}
					theApp.m_pGetFiletatusfn = NULL;
					return ERR_CANCELED;
					return error;
				}
			}
			
			theApp.m_File->Buffer = (unsigned char *)realloc(theApp.m_File->Buffer,theApp.m_File->FileSize);
			memcpy(theApp.m_File->Buffer,theApp.m_unDecodeBase64,theApp.m_File->FileSize);
		}
		else
		{
			if (theApp.m_unDecodeBase64)
			{
				delete theApp.m_unDecodeBase64;
				theApp.m_unDecodeBase64=NULL;
			}
			theApp.m_pGetFiletatusfn = NULL;
			return ERR_CANCELED;
			
		}

	if (theApp.m_unDecodeBase64)
	{
		delete theApp.m_unDecodeBase64;
		theApp.m_unDecodeBase64=NULL;
	}
	theApp.m_pGetFiletatusfn = NULL;
	return error;
}
GSM_Error WINAPI OBEX_GetNextFileFolder(GSM_File *File, bool start)
{
	return ERR_NOTSUPPORTED;
}
//////////////////////////////////////////////////////////////////////////080512LIBAOLIU
GSM_Error NXP_ReplyGetRootDirInfo( void )
{


//	char			    *pos;
//	unsigned char		buffer[500];
//	unsigned char		uName[500];
//	unsigned char		szName[500];
//	unsigned char       temp[10];
//	unsigned char tempApplication[50];
	unsigned char tempPicture[50];
	unsigned char tempSound[50];
//	unsigned char tempOthers[50];
	unsigned char tempPicturefullname[50];
	unsigned char tempsoundfullname[50];
//	unsigned char tempOthersfullname[50];
	int i=0;

	EncodeUnicode (tempPicture, ( unsigned char *) "picture", 7);
	EncodeUnicode (tempSound, ( unsigned char *) "melody", 6);
//	EncodeUnicode(tempOthers,( unsigned char *) "Others",6);
	EncodeUnicode(tempPicturefullname,( unsigned char *)"/app/picture",12);
	EncodeUnicode(tempsoundfullname,( unsigned char *)"/app/melody",11);
//	EncodeUnicode(tempOthersfullname,( unsigned char *)"/app3/user/Others",17);

	for (i=1 ; i < 3 ; i++ ) 
	{
        theApp.m_Files[theApp.m_FilesLocationsUsed].Folder		= true;
		theApp.m_Files[theApp.m_FilesLocationsUsed].ReadOnly = false;
		theApp.m_Files[theApp.m_FilesLocationsUsed].Hidden = false;
		theApp.m_Files[theApp.m_FilesLocationsUsed].System = false;
		theApp.m_Files[theApp.m_FilesLocationsUsed].Protected = false;
		theApp.m_Files[theApp.m_FilesLocationsUsed].Level = 1;
		switch( i )
		{
		case 1 :
		  CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].Name,tempPicture);
		  CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,tempPicturefullname);
		  CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].wFileFullPathName,tempPicturefullname);
		 break;
		case 2 :
		  CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].Name,tempSound);
		  CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,tempsoundfullname);
		  CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].wFileFullPathName,tempsoundfullname);
		 break;
// 		case 3 :
// 		  CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].Name,tempOthers);
// 		  CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].ID_FullName,tempOthersfullname);
// 		  CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsUsed].wFileFullPathName,tempOthersfullname);
// 		  break;


		}
		theApp.m_Files[theApp.m_FilesLocationsUsed].FileSize = 0;
		theApp.m_FilesLocationsUsed++;
	}
    return ERR_NONE;


		
}
GSM_Error WINAPI OBEX_GetObexFileList(GSM_File *File)
{
	GSM_Error error = ERR_NONE;
	//SetPath first
	theApp.m_FilesLocationsUsed 	= 0;
	theApp.m_FilesLocationsCurrent 	= 0;

	error = MTK_SetCharset(true);
	char			req[2000];
//	unsigned char			szdest[2000];
	if(UnicodeLength(theApp.m_ChangeFolderPath.wFileFullPathName) == 0)
	{
		error=NXP_ReplyGetRootDirInfo();//080512libaoliu
	    return error;
		/*sprintf(req, "AT*LIST=/app\r");*/
	}
	else
	{
		unsigned char 		wPathName[800];
		unsigned char 		wFileName[800];
		GetFilePath_Unicode(theApp.m_ChangeFolderPath.wFileFullPathName,wPathName);
		GetFileName_Unicode(theApp.m_ChangeFolderPath.wFileFullPathName,wFileName);
		{
			unsigned char szASCIIName[500];
			unsigned char szASCIIPath[500];
			DecodeUnicode(wFileName,szASCIIName);
			DecodeUnicode(wPathName,szASCIIPath);
			sprintf(req, "AT*LIST=%s%s\r",szASCIIPath,szASCIIName);
		}
	}
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*LIST");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;

	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*LIST:");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;

// 	GSM_ATMultiAnwser ATMultiAnwser;
// 	wsprintf(ATMultiAnwser.Specialtext,"+EFSL:");
// 	ATMultiAnwser.Anwserlines = 1;

	error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,MTK_ReplyGetMutiFileInfo);
	if (error == ERR_EMPTY) error  = ERR_NONE;
	if(theApp.m_FilesLocationsUsed  > 0) 
		error = ERR_NONE;

	return error;
}
GSM_Error WINAPI OBEX_SetFolderPath(GSM_File *File)
{
	memcpy(&theApp.m_ChangeFolderPath,File,sizeof(GSM_File));
	theApp.m_bSetPath = true;
	return ERR_NONE;
}
int WINAPI OBEX_GetOBEXSubList(GSM_File *filelist[])
{
    int nfile =0;
	int i=0;

	if(theApp.m_FilesLocationsUsed)
	{
		nfile=theApp.m_FilesLocationsUsed ;
		for(i=0 ;i<nfile ;i++)
		{
			filelist[i]=&theApp.m_Files[i];
		}

	}
	return nfile;
}
GSM_Error WINAPI OBEX_GetOBEXFolderListCount(int  *nFileNo)
{
	*nFileNo = 0;

	if(theApp.m_FilesLocationsUsed)
	{
		*nFileNo=theApp.m_FilesLocationsUsed ;
	}
	return ERR_NONE;
}


