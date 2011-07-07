#include "stdafx.h"
#include "MTK.h"
#include "commfun.h"
#include "MTKfundef.h"
#include "coding.h"
#include "gsmpbk.h"
/*#include "encode.h"*/
#include <afxtempl.h>


extern CMTKApp theApp;
extern GSM_Reply_MsgType ReplymsgType;
CList <unsigned short,unsigned short>IndecFileList;



 char Base64Alphabet[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
unsigned char GetBase64Index(char ch)
{
	unsigned char i, v_Return = -1;
	for(i = 0; i < 64; i++)
	{
		if(Base64Alphabet[(int)i] == ch)
		{
			v_Return = i;
			break;
		}
	}
	return v_Return;
}
int Decode(const char * szDecoding, char * szOutput)
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
		chCurrentIndex = GetBase64Index(Ch);
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

int Encode(const unsigned char *pp_Data, int v_DataLen,unsigned char *pp_EncodeData)
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
		pl_EnData[0] = Base64Alphabet[(int)v_FirstByte];
        pl_EnData[1] = Base64Alphabet[(int)v_SecondByte];
		pl_EnData[2] = Base64Alphabet[(int)v_ThirdByte];
		pl_EnData[3] = Base64Alphabet[(int)v_FourthByte];

		pp_Data += 3;
		pl_EnData += 4;	 
	}
	v_Left = v_DataLen % 3;
	if(v_Left == 1)
	{
		v_FirstByte = pp_Data[0] >> 2;
		v_SecondByte = (pp_Data[0] & 0x03) << 4;
		pl_EnData[0] = Base64Alphabet[(int)v_FirstByte];
		pl_EnData[1] = Base64Alphabet[(int)v_SecondByte];
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
		pl_EnData[0] = Base64Alphabet[(int)v_FirstByte];
		pl_EnData[1] = Base64Alphabet[(int)v_SecondByte];
		pl_EnData[2] = Base64Alphabet[(int)v_ThirdByte];
		pl_EnData[3] = '=';
		pp_Data += 2;
		pl_EnData += 4;	 
	}
	
	return pl_EnData - pp_EncodeData ;
}
void bcd2ascii( char *p_bcd,char *p_ascii,int ascii_len)
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
void  ascii2bcd(char *p_ascii,char *p_bcdcode,int bcdcode_len)
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
char CheckSum(char* pSrc, int nLen)
{

       if(nLen == 0)return 0;
       BYTE ch = *pSrc;
       for(int i = 1; i < nLen; i++)
       {
              ch = ch ^ (*(pSrc+i));
       }
       return ch;
}
GSM_Error MTK_ReplySetMemory(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	MTK_GetReplyStatue(&msg,&Replynfo);
	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "Phonebook entry written OK\n");
		return ERR_NONE;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(Replynfo);
	case AT_Reply_CMEError:
	        return ATGEN_HandleCMEError(Replynfo);
	case AT_Reply_Error:
		return ERR_INVALIDDATA;
	default:
		return ERR_UNKNOWNRESPONSE;
	}
}
GSM_Error MTK_ReplyArimaGetMutiMemory(GSM_Protocol_Message msg)
{
	int nIndex = 0;
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

	
	GSM_MemoryEntry		*Memory = &theApp.m_PBKMemoryEntry;
	char			*pos;
	unsigned char		buffer[500],buffer2[500];
	int			len;
	FILE *	file;
	// v1.1.0.0 , added by mingfa
    GSM_MemoryStatus  *MemoryStatus = theApp.m_MemoryStatus;
	int testlen = 0;


	switch (Replynfo.ReplyState) {
	case AT_Reply_Continue: //peggy 0201 add
	case AT_Reply_OK:
		pos = (char*)msg.Buffer;
		while(pos)
		{
 			Memory->EntriesNum = 0;
			pos = strstr((char*)pos, "^CPFR:");
			if (pos == NULL)//return ERR_UNKNOWN;//peggy
			{
				if(Replynfo.ReplyState == AT_Reply_Continue)
					return ERR_UNKNOWN;
				else return ERR_NONE; 
			}
			pos += 6; 

			while (*pos && !isdigit(*pos)) pos++;
 			nIndex = atoi(pos) + 1 - theApp.m_MemoryInfo.FirstMemoryEntry;
			wsprintf(Memory->szIndex,"%d",nIndex);
			smprintf(theApp.m_pDebuginfo, "Location: %s\n", Memory->szIndex);
				
			while (*pos != '"') pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 			smprintf(theApp.m_pDebuginfo, "Number: %s\n",buffer);
 	//		Memory->EntriesNum++;
  			if(MemoryStatus->MemoryType == MEM_ME)
				Memory->Entries[Memory->EntriesNum].EntryType  = PBK_Number_Mobile;
			else
				Memory->Entries[Memory->EntriesNum].EntryType  = PBK_Number_General;
 			Memory->Entries[Memory->EntriesNum].VoiceTag   = 0;
 			Memory->Entries[Memory->EntriesNum].SMSList[0] = 0;

			len = strlen((char*)buffer + 1) - 1;
			if (theApp.m_PBKCharset == AT_PBK_HEX && (len > 10) && (len % 2 == 0) && (strchr((char*)buffer + 1, '+') == NULL)) {
				/* This is probably hex encoded number */
				DecodeHexBin(buffer2, buffer+1, len);
				DecodeDefault(Memory->Entries[Memory->EntriesNum].Text ,buffer2, strlen((char*)buffer2), false, NULL);
			} /*else if (theApp.m_PBKCharset == AT_PBK_UCS2 && (len > 20) && (len % 4 == 0) && (strchr((char*)buffer + 1, '+') == NULL)) {
				/* This is probably unicode encoded number */
			//	DecodeHexUnicode(Memory->Entries[0].Text, buffer + 1,len);
		//	}*/ 
			else  {
	 			EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, buffer + 1, len);
			}
			
			/* Number format */
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 			smprintf(theApp.m_pDebuginfo, "Number format: %s\n",buffer);

			/* International number */
			if (!strcmp((char*)buffer,"145")) 
			{
				sprintf((char*)buffer+1,"%s",DecodeUnicodeString(Memory->Entries[Memory->EntriesNum].Text));
				if (strlen((char*)buffer+1)!=0 && buffer[1] != '+') {
					/* Sony Ericsson issue */
					/* International number is without + */
					buffer[0] = '+';
					EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text,buffer,strlen((char*)buffer));
				}
			}
			if(UnicodeLength(Memory->Entries[Memory->EntriesNum].Text)>0)
			{
				Memory->EntriesNum++;
			}
			/* Name */
			while (*pos != '"') pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 			smprintf(theApp.m_pDebuginfo, "Name text: %s\n",buffer);

 		//	Memory->EntriesNum++;
 			Memory->Entries[Memory->EntriesNum].EntryType=PBK_Text_Name;
			switch (theApp.m_PBKCharset) {
			case AT_PBK_GSM:
 				DecodeDefault(Memory->Entries[Memory->EntriesNum].Text,buffer+1,strlen((char*)buffer)-2,false,NULL);
				break;			
			case AT_PBK_UCS2:	
				{
					unsigned char temp[MAX_PATH];
				DecodeHexUnicode(Memory->Entries[Memory->EntriesNum].Text,buffer+1,strlen((char*)buffer+1) - 1);
				DecodeUnicode(Memory->Entries[Memory->EntriesNum].Text,temp);
				}
				break;			
			}
			Memory->EntriesNum++;

//advance entry++
			//home number
			while (*pos != '"') pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
			Memory->Entries[Memory->EntriesNum].EntryType  = PBK_Number_Home;
 			Memory->Entries[Memory->EntriesNum].VoiceTag   = 0;
 			Memory->Entries[Memory->EntriesNum].SMSList[0] = 0;
			len = strlen((char*)buffer + 1) - 1;
			if (theApp.m_PBKCharset == AT_PBK_HEX && (len > 10) && (len % 2 == 0) && (strchr((char*)buffer + 1, '+') == NULL)) {
				DecodeHexBin(buffer2, buffer+1, len);
				DecodeDefault(Memory->Entries[Memory->EntriesNum].Text ,buffer2, strlen((char*)buffer2), false, NULL);
			} 
			else 
	 			EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, buffer + 1, len);
		   if(UnicodeLength(Memory->Entries[Memory->EntriesNum].Text)>0)
 				Memory->EntriesNum++;

		//Company Name
			while (*pos != '"') pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 			Memory->Entries[Memory->EntriesNum].EntryType=PBK_Text_Company;
			// 0702 for Arima by mingfa
			{
				unsigned char temp[MAX_PATH];
				DecodeHexUnicode(Memory->Entries[Memory->EntriesNum].Text,buffer+1,strlen((char*)buffer+1) - 1);
				DecodeUnicode(Memory->Entries[Memory->EntriesNum].Text,temp);
			}
			// 0702 marked by mingfa
		//	EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, buffer + 1, strlen((char*)buffer+1) - 1);
			if(UnicodeLength(Memory->Entries[Memory->EntriesNum].Text)>0)
  				Memory->EntriesNum++;
		//Email
			while (*pos != '"') pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 			Memory->Entries[Memory->EntriesNum].EntryType=PBK_Text_Email;
			EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, buffer + 1, strlen((char*)buffer+1) - 1);
 			if(UnicodeLength(Memory->Entries[Memory->EntriesNum].Text)>0)
				Memory->EntriesNum++;
		//office Number
			while (*pos != '"') pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
			Memory->Entries[Memory->EntriesNum].EntryType  = PBK_Number_Work;
 			Memory->Entries[Memory->EntriesNum].VoiceTag   = 0;
 			Memory->Entries[Memory->EntriesNum].SMSList[0] = 0;
			len = strlen((char*)buffer + 1) - 1;
			if (theApp.m_PBKCharset == AT_PBK_HEX && (len > 10) && (len % 2 == 0) && (strchr((char*)buffer + 1, '+') == NULL)) {
				DecodeHexBin(buffer2, buffer+1, len);
				DecodeDefault(Memory->Entries[Memory->EntriesNum].Text ,buffer2, strlen((char*)buffer2), false, NULL);
			} 
			else 
	 			EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, buffer + 1, len);
 			if(UnicodeLength(Memory->Entries[Memory->EntriesNum].Text)>0)
  				Memory->EntriesNum++;
		//fax Number
			while (*pos != '"') pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
			Memory->Entries[Memory->EntriesNum].EntryType  = PBK_Number_Fax;
 			Memory->Entries[Memory->EntriesNum].VoiceTag   = 0;
 			Memory->Entries[Memory->EntriesNum].SMSList[0] = 0;
			len = strlen((char*)buffer + 1) - 1;
			if (theApp.m_PBKCharset == AT_PBK_HEX && (len > 10) && (len % 2 == 0) && (strchr((char*)buffer + 1, '+') == NULL)) {
				DecodeHexBin(buffer2, buffer+1, len);
				DecodeDefault(Memory->Entries[Memory->EntriesNum].Text ,buffer2, strlen((char*)buffer2), false, NULL);
			} 
			else 
	 			EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, buffer + 1, len);
  			if(UnicodeLength(Memory->Entries[Memory->EntriesNum].Text)>0)
 				Memory->EntriesNum++;
	
			while (*pos && !isdigit(*pos)) pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 			int nYear = atoi((char*)buffer);
		
			while (*pos && !isdigit(*pos)) pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 			int nMonth = atoi((char*)buffer);
			
			while (*pos && !isdigit(*pos)) pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 			int nDay = atoi((char*)buffer);
			if(nYear >0 && (nMonth>=1 && nMonth<=12) && (nDay>=1 && nDay<=31))
			{
				Memory->Entries[Memory->EntriesNum].EntryType  = PBK_Date;
 				Memory->Entries[Memory->EntriesNum].VoiceTag   = 0;
 				Memory->Entries[Memory->EntriesNum].SMSList[0] = 0;
				Memory->Entries[Memory->EntriesNum].Date.Year = nYear;
				Memory->Entries[Memory->EntriesNum].Date.Month = nMonth;
				Memory->Entries[Memory->EntriesNum].Date.Day = nDay;

				Memory->EntriesNum++;
			}

// 0903 added by mingfa for arima
			//group id
			while (*pos && !isdigit(*pos)) pos++;
			Memory->Entries[Memory->EntriesNum].EntryType  = PBK_Caller_Group;
 			Memory->Entries[Memory->EntriesNum].VoiceTag   = 0;
 			Memory->Entries[Memory->EntriesNum].SMSList[0] = 0;
			Memory->Entries[Memory->EntriesNum].Number = atoi(pos) ;
			Memory->Entries[Memory->EntriesNum].Text[0] ='\0';
  			Memory->EntriesNum++;

			// VOIP URI ( To use PBK_Number_ISDN for VOIP URI )
			while (*pos != '"') pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 			Memory->Entries[Memory->EntriesNum].EntryType=PBK_Number_ISDN;
 			Memory->Entries[Memory->EntriesNum].VoiceTag   = 0;
 			Memory->Entries[Memory->EntriesNum].SMSList[0] = 0;
			EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, buffer + 1, strlen((char*)buffer+1) - 1);
 			if(UnicodeLength(Memory->Entries[Memory->EntriesNum].Text)>0)
				Memory->EntriesNum++;


//advance entry--

			theApp.m_PHKNum ++;
			file = fopen(theApp.m_szPHKTempFile,"a+b");
			if(file)
			{
				fwrite(&theApp.m_PBKMemoryEntry,1,sizeof(GSM_MemoryEntry),file);
				fclose(file);
			}

			ZeroMemory(&theApp.m_PBKMemoryEntry,sizeof(GSM_MemoryEntry));
		}
		if(Replynfo.ReplyState == AT_Reply_Continue)
			return ERR_NEEDANOTHEDATA;
		else return ERR_NONE; 
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
GSM_Error NXP_ReplyGetDataMemory(GSM_Protocol_Message msg)
{
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
			
			if (!theApp.isFirst)
			{
				_mbscpy(theApp.m_unDecodeBase64,buffer);
				theApp.isFirst=true;
			}
			else
				_mbscat(theApp.m_unDecodeBase64,buffer);
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
GSM_Error NXP_ReplyGetDataFileMemory(GSM_Protocol_Message msg)
{
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
			
			if (!theApp.isFirst)
			{
				_mbscpy(theApp.m_unDecodeBase64,buffer);
				theApp.isFirst=true;
			}
			else
				_mbscat(theApp.m_unDecodeBase64,buffer);
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
GSM_Error MTK_ReplyGetMutiMemory(GSM_Protocol_Message msg)
{
	int nIndex = 0;
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
	GSM_MemoryEntry		*Memory = &theApp.m_PBKMemoryEntry;
	char			*pos;
	char			*pos2;
	unsigned char		buffer[500],buffer2[500];
	int			len;
	FILE *	file;
    GSM_MemoryStatus  *MemoryStatus = theApp.m_MemoryStatus;
	int testlen = 0;

	switch (Replynfo.ReplyState) 
	{
	case AT_Reply_Continue: //peggy 0201 add
	case AT_Reply_OK:
 		smprintf(theApp.m_pDebuginfo, "Phonebook entry received\n");
		pos = (char*)msg.Buffer;
		while(pos)
		{
 			Memory->EntriesNum = 0;
			pos = strstr((char*)pos, "+CPBR:");
			if (pos == NULL)//return ERR_UNKNOWN;//peggy
			{
				if(Replynfo.ReplyState == AT_Reply_Continue)
					return ERR_UNKNOWN;
				else return ERR_NONE; 
			}
			pos += 6; 
			while (*pos && !isdigit(*pos)) pos++;
 			nIndex = atoi(pos) + 1 - theApp.m_MemoryInfo.FirstMemoryEntry;
			wsprintf(Memory->szIndex,"%d",nIndex);
			smprintf(theApp.m_pDebuginfo, "Location: %s\n", Memory->szIndex);
				
			while (*pos != '"') pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 			smprintf(theApp.m_pDebuginfo, "Number: %s\n",buffer);
 	//		Memory->EntriesNum++;
  			if(MemoryStatus->MemoryType == MEM_ME)
				Memory->Entries[Memory->EntriesNum].EntryType  = PBK_Number_Mobile;
			else
				Memory->Entries[Memory->EntriesNum].EntryType  = PBK_Number_General;
 			Memory->Entries[Memory->EntriesNum].VoiceTag   = 0;
 			Memory->Entries[Memory->EntriesNum].SMSList[0] = 0;

			len = strlen((char*)buffer + 1) - 1;
			if (theApp.m_PBKCharset == AT_PBK_HEX && (len > 10) && (len % 2 == 0) && (strchr((char*)buffer + 1, '+') == NULL)) {
				/* This is probably hex encoded number */
				DecodeHexBin(buffer2, buffer+1, len);
				DecodeDefault(Memory->Entries[Memory->EntriesNum].Text ,buffer2, strlen((char*)buffer2), false, NULL);
			} /*else if (theApp.m_PBKCharset == AT_PBK_UCS2 && (len > 20) && (len % 4 == 0) && (strchr((char*)buffer + 1, '+') == NULL)) {
				/* This is probably unicode encoded number */
			//	DecodeHexUnicode(Memory->Entries[0].Text, buffer + 1,len);
		//	}*/ 
			else  
			{
	 			EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, buffer + 1, len);
			}
			
			/* Number format */
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 			smprintf(theApp.m_pDebuginfo, "Number format: %s\n",buffer);

			/* International number */
			if (!strcmp((char*)buffer,"145")) {
				sprintf((char*)buffer+1,"%s",DecodeUnicodeString(Memory->Entries[Memory->EntriesNum].Text));
				if (strlen((char*)buffer+1)!=0 && buffer[1] != '+') {
					/* Sony Ericsson issue */
					/* International number is without + */
					buffer[0] = '+';
					EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text,buffer,strlen((char*)buffer));
				}
			}
			if(UnicodeLength(Memory->Entries[Memory->EntriesNum].Text)>0)
			{
				Memory->EntriesNum++;
			}
			/* Name */
		
			while (*pos != '"') pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 			smprintf(theApp.m_pDebuginfo, "Name text: %s\n",buffer);

 		//	Memory->EntriesNum++;
 			Memory->Entries[Memory->EntriesNum].EntryType=PBK_Text_Name;
			switch (theApp.m_PBKCharset) 
			{
			case AT_PBK_GSM:
 				DecodeDefault(Memory->Entries[Memory->EntriesNum].Text,buffer+1,strlen((char*)buffer)-2,false,NULL);
				break;			
			case AT_PBK_UCS2:	
				{
					unsigned char temp[MAX_PATH];
					
				DecodeHexUnicode(Memory->Entries[Memory->EntriesNum].Text,buffer+1,strlen((char*)buffer+1) - 1);
				DecodeUnicode(Memory->Entries[Memory->EntriesNum].Text,temp);
				}
				break;			
			}
			Memory->EntriesNum++;
//advance entry++
			pos2 = strstr((char*)pos, "+Option");
			if (pos2)
			{

				//home number
				pos =pos2+ 8; 
				while (*pos != '"') pos++;
				pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
				Memory->Entries[Memory->EntriesNum].EntryType  = PBK_Number_Home;
 				Memory->Entries[Memory->EntriesNum].VoiceTag   = 0;
 				Memory->Entries[Memory->EntriesNum].SMSList[0] = 0;
				len = strlen((char*)buffer + 1) - 1;
				if (theApp.m_PBKCharset == AT_PBK_HEX && (len > 10) && (len % 2 == 0) && (strchr((char*)buffer + 1, '+') == NULL)) {
					DecodeHexBin(buffer2, buffer+1, len);
					DecodeDefault(Memory->Entries[Memory->EntriesNum].Text ,buffer2, strlen((char*)buffer2), false, NULL);
				} 
				else 
	 				EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, buffer + 1, len);
			   if(UnicodeLength(Memory->Entries[Memory->EntriesNum].Text)>0)
 					Memory->EntriesNum++;

			//Company Name
				while (*pos != '"') pos++;
				pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 				Memory->Entries[Memory->EntriesNum].EntryType=PBK_Text_Company;
				{
					unsigned char temp[MAX_PATH];
					DecodeHexUnicode(Memory->Entries[Memory->EntriesNum].Text,buffer+1,strlen((char*)buffer+1) - 1);
					DecodeUnicode(Memory->Entries[Memory->EntriesNum].Text,temp);
				}
				if(UnicodeLength(Memory->Entries[Memory->EntriesNum].Text)>0)
  					Memory->EntriesNum++;
			//Email
				while (*pos != '"') pos++;
				pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 				Memory->Entries[Memory->EntriesNum].EntryType=PBK_Text_Email;
				EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, buffer + 1, strlen((char*)buffer+1) - 1);
 				if(UnicodeLength(Memory->Entries[Memory->EntriesNum].Text)>0)
					Memory->EntriesNum++;
			//office Number
				while (*pos != '"') pos++;
				pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
				Memory->Entries[Memory->EntriesNum].EntryType  = PBK_Number_Work;
 				Memory->Entries[Memory->EntriesNum].VoiceTag   = 0;
 				Memory->Entries[Memory->EntriesNum].SMSList[0] = 0;
				len = strlen((char*)buffer + 1) - 1;
				if (theApp.m_PBKCharset == AT_PBK_HEX && (len > 10) && (len % 2 == 0) && (strchr((char*)buffer + 1, '+') == NULL)) {
					DecodeHexBin(buffer2, buffer+1, len);
					DecodeDefault(Memory->Entries[Memory->EntriesNum].Text ,buffer2, strlen((char*)buffer2), false, NULL);
				} 
				else 
	 				EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, buffer + 1, len);
 				if(UnicodeLength(Memory->Entries[Memory->EntriesNum].Text)>0)
  					Memory->EntriesNum++;
			//fax Number
				while (*pos != '"') pos++;
				pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
				Memory->Entries[Memory->EntriesNum].EntryType  = PBK_Number_Fax;
 				Memory->Entries[Memory->EntriesNum].VoiceTag   = 0;
 				Memory->Entries[Memory->EntriesNum].SMSList[0] = 0;
				len = strlen((char*)buffer + 1) - 1;
				if (theApp.m_PBKCharset == AT_PBK_HEX && (len > 10) && (len % 2 == 0) && (strchr((char*)buffer + 1, '+') == NULL)) {
					DecodeHexBin(buffer2, buffer+1, len);
					DecodeDefault(Memory->Entries[Memory->EntriesNum].Text ,buffer2, strlen((char*)buffer2), false, NULL);
				} 
				else 
	 				EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, buffer + 1, len);
  				if(UnicodeLength(Memory->Entries[Memory->EntriesNum].Text)>0)
 					Memory->EntriesNum++;
			//picture tag
				pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
			//melody id
				pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
			//group id
				while (*pos && !isdigit(*pos)) pos++;
				Memory->Entries[Memory->EntriesNum].EntryType  = PBK_Caller_Group;
 				Memory->Entries[Memory->EntriesNum].VoiceTag   = 0;
 				Memory->Entries[Memory->EntriesNum].SMSList[0] = 0;
				Memory->Entries[Memory->EntriesNum].Number = atoi(pos) ;
				Memory->Entries[Memory->EntriesNum].Text[0] ='\0';
  				Memory->EntriesNum++;

			}
//advance entry--

			theApp.m_PHKNum ++;
			file = fopen(theApp.m_szPHKTempFile,"a+b");
			if(file)
			{
				fwrite(&theApp.m_PBKMemoryEntry,1,sizeof(GSM_MemoryEntry),file);
				fclose(file);
			}

			ZeroMemory(&theApp.m_PBKMemoryEntry,sizeof(GSM_MemoryEntry));
		}
		if(Replynfo.ReplyState == AT_Reply_Continue)
			return ERR_NEEDANOTHEDATA;
		else return ERR_NONE; 
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
GSM_Error NXP_ReplyGetIndexFileInfo(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	MTK_GetReplyStatue(&msg,&Replynfo);
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
			theApp.m_unDecodeBase64=new unsigned char[Length];

		return ERR_NONE;
	case AT_Reply_Error:
		return ERR_UNKNOWN;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(Replynfo);
 	default:
		return ERR_UNKNOWNRESPONSE;
	}

}
// GSM_Error NXP_ReplyGetDataFileInfo(GSM_Protocol_Message msg)
// {
// 	GSM_ATReplayInfo Replynfo;
// 	MTK_GetReplyStatue(&msg,&Replynfo);
// 	char 			*pos;
// 	int Length=0;
//  	switch (Replynfo.ReplyState) 
// 	{
//  	case AT_Reply_OK:
// 		smprintf(theApp.m_pDebuginfo, "Memory info received\n");
// 		pos =(char*)msg.Buffer;
// 		if (!pos) return ERR_UNKNOWN;
// 		pos = strstr((char*)pos, "*STARTUL:");
// 		pos=pos+9;
// 		Length=atoi(pos);
// 		if (theApp.m_unDecodeBase64)
// 		{
// 			delete theApp.m_unDecodeBase64;
// 			theApp.m_unDecodeBase64=NULL;
// 		}
// 		else
// 			theApp.m_unDecodeBase64=new unsigned char[Length];
// 		return ERR_NONE;
// 	case AT_Reply_Error:
// 		return ERR_UNKNOWN;
// 	case AT_Reply_CMSError:
// 	        return ATGEN_HandleCMSError(Replynfo);
//  	default:
// 		return ERR_UNKNOWNRESPONSE;
// 	}
// 
// }

GSM_Error MTK_ReplyGetCPBRMemoryInfo(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	MTK_GetReplyStatue(&msg,&Replynfo);
	char 			*pos;
 	switch (Replynfo.ReplyState) 
	{
 	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "Memory info received\n");
		pos = strchr((char*)msg.Buffer, '(');
		if (!pos) return ERR_UNKNOWN;
		pos++;
		theApp.m_MemoryInfo.FirstMemoryEntry = atoi(pos);

		/* Parse last location*/
		pos = strchr(pos, '-');
		if (!pos) return ERR_UNKNOWN;
		pos++;
		theApp.m_MemoryInfo.MemorySize = atoi(pos) + 1 - theApp.m_MemoryInfo.FirstMemoryEntry;

		/* Parse number length*/
		pos = strchr(pos, ',');
		if (!pos) return ERR_UNKNOWN;
		pos++;
		theApp.m_MemoryInfo.NumberLength = atoi(pos);

		/* Parse text length*/
		pos = strchr(pos, ',');
		if (!pos) return ERR_UNKNOWN;
		pos++;
		theApp.m_MemoryInfo.TextLength = atoi(pos);

		return ERR_NONE;
	case AT_Reply_Error:
		return ERR_UNKNOWN;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(Replynfo);
 	default:
		return ERR_UNKNOWNRESPONSE;
	}
}
GSM_Error MTK_GetMemoryInfo()

{
	GSM_Error		error;

	smprintf(theApp.m_pDebuginfo, "Getting memory information\n");
	
	theApp.m_MemoryInfo.MemorySize		= 0;
	theApp.m_MemoryInfo.TextLength		= 0;
	theApp.m_MemoryInfo.NumberLength		= 0;
	theApp.m_MemoryInfo.FirstMemoryEntry = 0;

	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBR=?");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = theApp.m_pWriteCommandfn ((unsigned char *)"AT+CPBR=?\r", 10, 0x00, 8, false,NULL,&ReplymsgType,MTK_ReplyGetCPBRMemoryInfo);

	return error;
}
GSM_Error WINAPI GetMemoryStatus(GSM_MemoryStatus *Status)
{
	Status->MemoryUsed		= -1;
	Status->MemoryFree		= -1;
	return ERR_NONE;
}

GSM_Error WINAPI GetMemory (GSM_MemoryEntry *entry)
{
// 	GSM_MemoryEntry* pMemoryEntry = NULL;
// 	FILE *	file;
// 	int nIndex = atoi(entry->szIndex);
// 	if(strlen(entry->szIndex) <=0 || nIndex==0) return ERR_INVALIDLOCATION;
// 
// 	int nLocation =atoi(entry->szIndex);
// //	if(nLocation < theApp.m_PHKNum+1 )
// 	
// 		file = fopen(theApp.m_szPHKTempFile,"r+b");
// 		if(file)
// 		{
// 			int i=0;
// 			while (true)
// 			{
// 				memset(entry,0,sizeof(GSM_MemoryEntry));
// 				fseek(file,sizeof(GSM_MemoryEntry)*i,SEEK_SET);
// 				fread(entry,1,sizeof(GSM_MemoryEntry),file);
// 				if (nLocation==atoi(entry->szIndex))
// 				{
// 					break;
// 				}
// 				else if(i==IndecFileList.GetCount()-1)
// 				{
// 					fclose(file);
// 					entry->EntriesNum = 0;
// 					return ERR_EMPTY;
// 
// 				}
// 				i++;
// 					
// 			}
// // 			fseek(file,sizeof(GSM_MemoryEntry)*nLocation,SEEK_SET);
// // 			fread(entry,1,sizeof(GSM_MemoryEntry),file);
// 			fclose(file);
// 			return ERR_NONE;
// 		}
// 		else
// 		{
// 			entry->EntriesNum = 0;
// 	        return ERR_EMPTY;
// 
// 		}
// 
// // 	entry->EntriesNum = 0;
// // 	return ERR_EMPTY;
	GSM_MemoryEntry* pMemoryEntry = NULL;
	FILE *	file;
	int nIndex = atoi(entry->szIndex);
	if(strlen(entry->szIndex) <=0 || nIndex==0) return ERR_INVALIDLOCATION;

	int nLocation =atoi(entry->szIndex) -1;
	if(nLocation < theApp.m_PHKNum )
	{
		file = fopen(theApp.m_szPHKTempFile,"r+b");
		if(file)
		{
			fseek(file,sizeof(GSM_MemoryEntry)*nLocation,SEEK_SET);
			fread(entry,1,sizeof(GSM_MemoryEntry),file);
			fclose(file);
			return ERR_NONE;
		}
	}
	entry->EntriesNum = 0;
	return ERR_EMPTY;

}
GSM_Error MTK_FindEntryDateIndex(GSM_MemoryEntry *entry, int *Mobile,int *Name,int *Home,int *Work,int *Fax,int *Email,int *Group,int *Company, int *pager,int *postcode,int *note)
{
	int i;

	*Name = -1;
	*Mobile = -1;
	*Home = -1;
	*Work = -1;
	*Fax = -1;
	*Email = -1;
	*Group = -1;
	*Company = -1;
	*pager=-1;
	*postcode=-1;
	*note=-1;
	for (i = 0; i < entry->EntriesNum; i++) 
	{
		switch (entry->Entries[i].EntryType) 
		{
		case PBK_Number_General : if (*Mobile   == -1) *Mobile  = i; break;
		case PBK_Number_Mobile	: if (*Mobile   == -1) *Mobile  = i;break;
		case PBK_Text_Name      : if (*Name== -1) *Name    = i; break;
		case PBK_Number_Home    : if (*Home		== -1) *Home    = i; break;
		case PBK_Number_Work    : if (*Work     == -1) *Work    = i; break;
		case PBK_Number_Fax		: if (*Fax	    == -1) *Fax		= i; break;
		case PBK_Text_Email		: if (*Email	== -1) *Email	= i; break;
		case PBK_Caller_Group	: if (*Group	== -1) *Group	= i; break;
		case PBK_Text_Company	: if (*Company		== -1) *Company		= i; break;
		case PBK_Text_Postal    : if( *postcode ==-1)    *postcode=i;       break;
		case PBK_Number_Pager   : if( *pager==-1)*pager=i; break;
		case PBK_Text_Note      : if(*note==-1)*note=i;break;
		default                 :                                    break;
		}
	}
	return ERR_NONE;
}
// 0903 modify by mingfa for arima's group and voip field
GSM_Error MTK_FindEntryDateIndexEx(GSM_MemoryEntry *entry, int *Mobile,int *Name,int *Home,int *Work,int *Fax,int *Email,int *Group,int *Company,int *Date, int *Voip)
{
	int i;

	*Name = -1;
	*Mobile = -1;
	*Home = -1;
	*Work = -1;
	*Fax = -1;
	*Email = -1;
	*Group = -1;
	*Company = -1;
	*Date = -1;
	*Voip = -1;
	for (i = 0; i < entry->EntriesNum; i++) 
	{
		switch (entry->Entries[i].EntryType) 
		{
		case PBK_Number_General : if (*Mobile   == -1) *Mobile  = i; break;
		case PBK_Number_Mobile	: if (*Mobile   == -1) *Mobile  = i; break;
		case PBK_Text_Name      : if (*Name     == -1) *Name    = i; break;
		case PBK_Number_Home    : if (*Home		== -1) *Home    = i; break;
		case PBK_Number_Work    : if (*Work     == -1) *Work    = i; break;
		case PBK_Number_Fax		: if (*Fax	    == -1) *Fax		= i; break;
		case PBK_Text_Email		: if (*Email	== -1) *Email	= i; break;
		case PBK_Caller_Group	: if (*Group	== -1) *Group	= i; break;
		case PBK_Text_Company	: if (*Company	== -1) *Company	= i; break;
		case PBK_Date			: if (*Date		== -1) *Date	= i; break;
		case PBK_Number_ISDN	: if (*Voip		== -1) *Voip	= i; break;
		default                 :                                    break;
		}
	}
	return ERR_NONE;
}
int MTK_PackSemiOctetNumber(unsigned char *Number, unsigned char *Output, bool semioctet)
{
	unsigned char	format;
	int		length, i;
	unsigned char    *buffer;

	length=UnicodeLength(Number);
	buffer = (unsigned char*)malloc(length+2);
	memset( buffer, 0, length+2);
	memcpy(buffer,DecodeUnicodeString(Number),length+1);

	/* Checking for format number */
	format = NUMBER_UNKNOWN_NUMBERING_PLAN_ISDN;
	for (i=0;i<length;i++) 
	{
		/* first byte is '+'. Number can be international */
		if (i==0 && *(buffer+i)=='+') 
		{
			format=NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN;  
		}/* else {
			/*char is not number. It must be alphanumeric*/
		/*	if (!isdigit(*(buffer+i))) format=NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN;
		}*/
	}

	/**
	 * First byte is used for saving type of number. See GSM 03.40
	 * section 9.1.2.5
	 */
	Output[0]=format;

	/* After number type we will have number. GSM 03.40 section 9.1.2 */
	switch (format) 
	{
	case NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN:
		length=GSM_PackSevenBitsToEight(0, buffer, Output+1, strlen((char*)buffer))*2;
		if (strlen((char *)buffer)==7) length--;
		break;
	case NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN:
		length--;
		EncodeBCD (Output+1, buffer+1, length, true);
		break;
	default:
		EncodeBCD (Output+1, buffer, length, true);
		break;
	}
	free(buffer);
	if (semioctet) return length;

	/* Convert number of semioctets to number of chars */
	if (length % 2) length++;
	return length / 2 + 1;
}
// char Base64Alphabet[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
// unsigned char GetBase64Index(char ch)
// {
// 	unsigned char i, v_Return = -1;
// 	for(i = 0; i < 64; i++)
// 	{
// 		if(Base64Alphabet[(int)i] == ch)
// 		{
// 			v_Return = i;
// 			break;
// 		}
// 	}
// 	return v_Return;
// }
// int Decode(const char * szDecoding, char * szOutput)
// {
// 	char			Ch;
// 	char			chCurrentIndex;
// 	unsigned int	i,nCurrentBits;
// 	unsigned char	chTargetChar;
// 	char			*pl_DeData;
// 	nCurrentBits = 0;
// 	chTargetChar = '\0';
// 	pl_DeData = szOutput;
// 	for(i = 0; i < strlen(szDecoding); i++)
// 	{
// 		Ch = szDecoding[i];
// 		if( Ch == '=')
// 		{
// 			nCurrentBits = 0;
// 	continue;
// 		}
// 		chCurrentIndex = GetBase64Index(Ch);
// 		if(chCurrentIndex < 0) 	return 0;
// 		if((nCurrentBits+6) >= 8)
// 		{
// 			chTargetChar |= (chCurrentIndex >> (6-(8-nCurrentBits)));
// 			*pl_DeData = chTargetChar;
// 			pl_DeData++;
// 			nCurrentBits = nCurrentBits - 2;
// 			chTargetChar = chCurrentIndex << (8 - nCurrentBits);
// 		}
// 		else
// 		{
// 			chTargetChar |= (chCurrentIndex << (2-nCurrentBits));
// 			nCurrentBits += 6;
// 		}
// 	}
// 	return pl_DeData-szOutput;
// }
// 
// int Encode(const unsigned char *pp_Data, int v_DataLen,unsigned char *pp_EncodeData)
// {
// 	unsigned char	v_FirstByte,v_SecondByte,v_ThirdByte, v_FourthByte;
// 	unsigned char	*pl_EnData;
// 	int 			i,v_Left;	
// 	pl_EnData = (unsigned char *)pp_EncodeData;
// 	for(i = 0; i < v_DataLen / 3; i++)
// 	{
// 		v_FirstByte	= pp_Data[0] >> 2;
// 		v_SecondByte	= ((pp_Data[0] & 0x03) << 4) | (pp_Data[1] >> 4) ;
// 		v_ThirdByte	= ((pp_Data[1] & 0x0F) << 2) | (pp_Data[2] >> 6);
// 		v_FourthByte	= pp_Data[2] & 0x3F;
// 		pl_EnData[0] = Base64Alphabet[(int)v_FirstByte];
//         pl_EnData[1] = Base64Alphabet[(int)v_SecondByte];
// 		pl_EnData[2] = Base64Alphabet[(int)v_ThirdByte];
// 		pl_EnData[3] = Base64Alphabet[(int)v_FourthByte];
// 
// 		pp_Data += 3;
// 		pl_EnData += 4;	 
// 	}
// 	v_Left = v_DataLen % 3;
// 	if(v_Left == 1)
// 	{
// 		v_FirstByte = pp_Data[0] >> 2;
// 		v_SecondByte = (pp_Data[0] & 0x03) << 4;
// 		pl_EnData[0] = Base64Alphabet[(int)v_FirstByte];
// 		pl_EnData[1] = Base64Alphabet[(int)v_SecondByte];
// 		pl_EnData[2] = '=';
// 		pl_EnData[3] = '=';
// 		pp_Data += 1;
// 		pl_EnData += 4;
// 	}
// 	else if(v_Left == 2)
// 	{
// 		v_FirstByte	= pp_Data[0] >> 2;
// 		v_SecondByte	= ((pp_Data[0] & 0x03) << 4) | ((pp_Data[1] & 0xF0) >> 4);
// 		v_ThirdByte	= (pp_Data[1] & 0x0F) << 2; 
// 		pl_EnData[0] = Base64Alphabet[(int)v_FirstByte];
// 		pl_EnData[1] = Base64Alphabet[(int)v_SecondByte];
// 		pl_EnData[2] = Base64Alphabet[(int)v_ThirdByte];
// 		pl_EnData[3] = '=';
// 		pp_Data += 2;
// 		pl_EnData += 4;	 
// 	}
// 	
// 	return pl_EnData - pp_EncodeData ;
// }
// void bcd2ascii( char *p_bcd,char *p_ascii,int ascii_len)
// {
// 	unsigned char i,j,len_char,temp_number;
// 	memset(p_ascii,'\0',ascii_len);
// 	len_char=*p_bcd;
// 	j=1;
// 	if(len_char%2)
// 	{
// 		for(i=0;i<len_char-1;i=i+2)
// 		{
// 				temp_number=*(p_bcd+j);
// 				p_ascii[i]=((temp_number & 0xf0)>>4)+'0';
// 				p_ascii[i+1]=(temp_number & 0x0f)+'0';
// 				j++;
// 		}
// 		temp_number=*(p_bcd+j);
// 		p_ascii[len_char-1]=temp_number;
// 	}
// 	else
// 	{
// 		for(i=0;i<len_char;i=i+2)
// 		{
// 				temp_number=*(p_bcd+j);
// 				p_ascii[i]=((temp_number & 0xf0)>>4)+'0';
// 				p_ascii[i+1]=(temp_number & 0x0f)+'0';
// 				j++;
// 		}
// 	}
// 	p_ascii[len_char]='\0';
// 	len_char=strlen(p_ascii);
// 	for(i=0;i<len_char;i++)
// 	{
// 		if(p_ascii[i]==(10+'0'))
// 		{
// 			p_ascii[i]='p';
// 		}
// 		if(p_ascii[i]==(11+'0'))
// 		{
// 			p_ascii[i]='w';
// 		}
// 		if(p_ascii[i]==(12+'0'))
// 		{
// 			p_ascii[i]='+';
// 		}
// 		if(p_ascii[i]==(13+'0'))
// 		{
// 			p_ascii[i]='*';
// 		}
// 		if(p_ascii[i]==(14+'0'))
// 		{
// 			p_ascii[i]='#';
// 		}
// 	}
// }
// void  ascii2bcd(char *p_ascii,char *p_bcdcode,int bcdcode_len)
// {
// 	unsigned char i,j,len_char;
// 	memset(p_bcdcode,'\0',bcdcode_len);
// 	len_char=strlen(p_ascii);
// 	for(i=0;i<len_char;i++)
// 	{
// 		if((*(p_ascii+i))=='p')
// 		{
// 			*(p_ascii+i)=(10+'0');
// 		}
// 		else if((*(p_ascii+i))=='w')
// 		{
// 			*(p_ascii+i)=(11+'0');
// 		}
// 		else if((*(p_ascii+i))=='+')
// 		{
// 			*(p_ascii+i)=(12+'0');
// 		}
// 		else if((*(p_ascii+i))=='*')
// 		{
// 			*(p_ascii+i)=(13+'0');
// 		}
// 		else if((*(p_ascii+i))=='#')
// 		{
// 			*(p_ascii+i)=(14+'0');
// 		}
// 	}
// 	j=1;
// 	p_bcdcode[0]=len_char;
// 
// 	if(len_char%2)
// 	{
// 		for(i=0;i<len_char-1;i=i+2)
// 		{
// 			p_bcdcode[j]=(*(p_ascii+i)-'0')*16+(*(p_ascii+i+1)-'0');
// 			j++;
// 		}
// 		p_bcdcode[j]=*(p_ascii+i);
// 
// 	}
// 	else
// 	{
// 		for(i=0;i<len_char-1;i=i+2)
// 		{
// 			p_bcdcode[j]=(*(p_ascii+i)-'0')*16+(*(p_ascii+i+1)-'0');
// 			j++;
// 		}
// 	}
// }
// char CheckSum(char* pSrc, int nLen)
// 
// {
// 
//        if(nLen == 0)return 0;
// 
//        BYTE ch = *pSrc;
// 
//        for(int i = 1; i < nLen; i++)
// 
//        {
// 
//               ch = ch ^ (*(pSrc+i));
// 
//        }
// 
//        return ch;
// 
// }
GSM_Error NXP_PrivSetMemoryEx(GSM_MemoryEntry *entry,bool bWithSpace)
{
#define REQUEST_SIZE	((4 * GSM_PHONEBOOK_TEXT_LENGTH) + 30)
//	unsigned char		name[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
	unsigned char		number[GSM_PHONEBOOK_TEXT_LENGTH + 1];
	char		        req[REQUEST_SIZE + 1];
	GSM_Error 		error;
	t_smartRecord AddSmartRecord;
	ZeroMemory(&AddSmartRecord,sizeof(t_smartRecord));
	int Mobile,Name,Home,Work,Fax,Email,Group,Company,pager,postcode,note;
	MTK_FindEntryDateIndex(entry, &Mobile,&Name,&Home,&Work,&Fax,&Email,&Group,&Company,&pager,&postcode,&note);
	if (Name != -1)
	{
		//sprintf(AddSmartRecord.name,"%s",DecodeUnicodeString(entry->Entries[Name].Text));
		EncodeUnicode2UTF8(entry->Entries[Name].Text,(unsigned char *)AddSmartRecord.name);//080527
	}
		
	if (Mobile != -1)
	{
		
		DecodeUnicode(entry->Entries[Mobile].Text,number);
		//EncodeBCD((unsigned char *)AddSmartRecord.phoneNumberSet[0].BCDPhoneNb,(unsigned char *)number,strlen((char *)number),true);
		ascii2bcd((char *)number,AddSmartRecord.phoneNumberSet[0].BCDPhoneNb,strlen((char *)number));
		AddSmartRecord.phoneNumberSet[0].numberType=SMPB_TYPE_MOBILE;
// 		bcd2ascii((char *)tempBuffer,(char *)hello,strlen((char *)tempBuffer));
// 		int leng=strlen((char *)tempBuffer);
// 		//memcpy(AddSmartRecord.phoneNumberSet[0].BCDPhoneNb,tempBuffer,strlen((char *)tempBuffer));
// 		sprintf((char *)AddSmartRecord.phoneNumberSet[0].BCDPhoneNb,"%s",(char *)tempBuffer);
		//bcd2ascii(AddSmartRecord.phoneNumberSet[0].BCDPhoneNb,(char *)hello,strlen(AddSmartRecord.phoneNumberSet[0].BCDPhoneNb));

  		AddSmartRecord.defaultFieldType=1;
  		AddSmartRecord.defaultNumberIndex=1;
	}
	if (Home != -1)
	{
		DecodeUnicode(entry->Entries[Home].Text,number);
		
		ascii2bcd((char *)number,AddSmartRecord.phoneNumberSet[2].BCDPhoneNb,strlen((char *)number));

		AddSmartRecord.phoneNumberSet[2].numberType=SMPB_TYPE_HOME;
	     AddSmartRecord.defaultFieldType=3;
  		AddSmartRecord.defaultNumberIndex=3;
	
		//sprintf(AddSmartRecord.phoneNumberSet[2].BCDPhoneNb,"%s",tempBuffer);
	}
	if (Work != -1)
	{
		DecodeUnicode(entry->Entries[Work].Text,number);
		//unsigned char tempBuffer[50];
		ascii2bcd((char *)number,AddSmartRecord.phoneNumberSet[1].BCDPhoneNb,strlen((char *)number));
		AddSmartRecord.phoneNumberSet[1].numberType=SMPB_TYPE_OFFICE;
	
		//sprintf(AddSmartRecord.phoneNumberSet[1].BCDPhoneNb,"%s",tempBuffer);
	}
	if (Fax != -1)
	{
		DecodeUnicode(entry->Entries[Fax].Text,number);
		//unsigned char tempBuffer[50];
		ascii2bcd((char *)number,AddSmartRecord.phoneNumberSet[3].BCDPhoneNb,strlen((char *)number));
		AddSmartRecord.phoneNumberSet[3].numberType=SMPB_TYPE_FAX;
		
		//sprintf(AddSmartRecord.phoneNumberSet[3].BCDPhoneNb,"%s",tempBuffer);
	}
	if (pager !=-1)
	{
		DecodeUnicode(entry->Entries[pager].Text,number);
		//unsigned char tempBuffer[50];
		ascii2bcd((char *)number,AddSmartRecord.phoneNumberSet[4].BCDPhoneNb,strlen((char *)number));
		AddSmartRecord.phoneNumberSet[4].numberType=SMPB_TYPE_PAGER;
	
		//sprintf(AddSmartRecord.phoneNumberSet[4].BCDPhoneNb,"%s",tempBuffer);

	}
	
	if (postcode !=-1)
	{
	sprintf(AddSmartRecord.postcode,"%s",DecodeUnicodeString(entry->Entries[postcode].Text));

	}
	if (note !=-1)
	{
	sprintf(AddSmartRecord.note,"%s",DecodeUnicodeString(entry->Entries[note].Text));

	}
	if (Email !=-1)
	{
		sprintf(AddSmartRecord.emailAddress,"%s",DecodeUnicodeString(entry->Entries[Email].Text));
		
	}
	if (Group !=-1)
	{
// 		sprintf((char *)&AddSmartRecord.groupType,"%d",entry->Entries[Group].Number);
		AddSmartRecord.groupType=entry->Entries[Group].Number;
	}

	if (Company !=-1)
	{
		sprintf(AddSmartRecord.address,"%s",DecodeUnicodeString(entry->Entries[Company].Text));
	}
	
// 	unsigned char DecodeBuffer[2000];
//	int DecodeLength=Encode((unsigned char*)&AddSmartRecord,sizeof(t_smartRecord),DecodeBuffer);
	remove(theApp.m_szPHKAddDataFileTempFile);
	FILE *	file;
	file = fopen(theApp.m_szPHKAddDataFileTempFile,"a+b");
	if(file)
	{
		int len = fwrite(&AddSmartRecord,1,403,file);
		fclose(file);
	}
	sprintf(req, "AT*STARTDL=/app/dir/tmp.dat,%d\r", 403);
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*STARTDL=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*STARTDL");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error=theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,MTK_ReplySetMemory);
	if (error != ERR_NONE&& error != ERR_EMPTY) return error;
	unsigned char DataBuffer[200];
	unsigned char EncodeDataBuffer[200];
	unsigned char checkSum[10];
	file = fopen(theApp.m_szPHKAddDataFileTempFile,"r+b");
    for (int i=0;i<4;i++)
    {
		memset(DataBuffer,0,sizeof(DataBuffer));
		memset(EncodeDataBuffer,0,sizeof(EncodeDataBuffer));
		memset(checkSum,0,sizeof(checkSum));
		if(file)
		{
			fseek(file,/*DecodeLength/4*/108*i,SEEK_SET);
			if (i==3)
			{
			fread(DataBuffer,1,/*DecodeLength/4*/79,file);
			}
			else
			{
			fread(DataBuffer,1,/*DecodeLength/4*/108,file);
			}

		}
		

		if (i==3)
		{
			char check=CheckSum((char *)DataBuffer,79);
			Encode((unsigned char*)&check,1,checkSum);
			Encode(DataBuffer,79,EncodeDataBuffer);
			sprintf(req,"AT*PUT=%d,%d,%s,%s\r",i,79,EncodeDataBuffer,checkSum);
		}
		else
		{
			char check=CheckSum((char *)DataBuffer,108);
			Encode((unsigned char*)&check,1,checkSum);
			Encode(DataBuffer,108,EncodeDataBuffer);
			
			sprintf(req,"AT*PUT=%d,%d,%s,%s\r",i,108,EncodeDataBuffer,checkSum);
			
		}
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*PUT=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*PUT");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error=theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,MTK_ReplySetMemory);
	if (error != ERR_NONE&& error != ERR_EMPTY) 
	{
		fclose(file);//081027libaoliu
		return error;
	}
		
    }
	fclose(file);
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
		return error;
	int Index=atoi(entry->szIndex);
// 	POSITION pos=IndecFileList.Find(Index);
// 	if (pos)
// 	{
// 	sprintf(req,"AT*SYPH=1,1,%d,/app/dir/tmp.dat\r",Index);
// 	}
// 	else
	{
	sprintf(req,"AT*SYPH=0,1,%d,/app/dir/tmp.dat\r",Index);
	IndecFileList.AddTail(Index);
	}


	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*SYPH=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*SYPH:");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error != ERR_NONE&& error != ERR_EMPTY) return error;
	return error;
}
GSM_Error MTK_PrivSetMemoryEx(GSM_MemoryEntry *entry,bool bWithSpace)
{
	/* REQUEST_SIZE should be big enough to handle all possibl cases
	 * correctly, especially with unicode entries */
#define REQUEST_SIZE	((4 * GSM_PHONEBOOK_TEXT_LENGTH) + 30)
	int		/*	Group, Name,*/ /*Number*/NumberType=0, len;
	GSM_Error 		error;
	unsigned char		req[REQUEST_SIZE + 1];
	unsigned char		szASCIIName[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
	unsigned char		name[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
	unsigned char		number[GSM_PHONEBOOK_TEXT_LENGTH + 1];
	int			reqlen;
	bool			PreferUnicode = false;
	// added by mingfa for Russ
	int	 pos=0;
    bool bMultiByte=false;


	int nIndex = atoi(entry->szIndex);
	if(strlen(entry->szIndex) <=0 || nIndex==0) return ERR_INVALIDLOCATION;

	error = ATGEN_SetPBKMemory(entry->MemoryType,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo);
	if (error != ERR_NONE) return error;  // Put to Initialize

//	GSM_PhonebookFindDefaultNameNumberGroup(entry, &Name, &Number, &Group);
	 int Mobile,Name,Home,Work,Fax,Email,Group,Company,pager,postcode,note;
	 MTK_FindEntryDateIndex(entry, &Mobile,&Name,&Home,&Work,&Fax,&Email,&Group,&Company,&pager,&postcode,&note);
	name[0] = 0;
	if (Name != -1) {
		PreferUnicode = true;//peggy +
		len = UnicodeLength(entry->Entries[Name].Text);
		DecodeUnicode(entry->Entries[Name].Text,szASCIIName);
		int nMax = 0;

		// v3.0.0.5 Added by mingfa ; recalculate the max length for RUSS...
		while(1){
			if(entry->Entries[Name].Text[pos]!=0x00 && entry->Entries[Name].Text[pos+1]!=0x00){
				bMultiByte=true; break; 
			}
			if(entry->Entries[Name].Text[pos]==0x00 && entry->Entries[Name].Text[pos+1]==0x00)
				break;
			pos+=2;
		}

        //v3.0.0.5 modify by mingfa for Russ
		//if(strlen((char*)szASCIIName) == len)
		if( (strlen((char*)szASCIIName) == len) && (bMultiByte == false ) )
		{
			PreferUnicode = false;
			if(entry->MemoryType == MEM_ME)
				nMax = 30;
			else nMax = 12; 
			if(len >nMax)
			{
				entry->Entries[Name].Text[nMax*2] = 0;
				entry->Entries[Name].Text[nMax*2+1] = 0;
			}
		}
		else
		{
			PreferUnicode = true;
			if(entry->MemoryType == MEM_ME)
				nMax = 14;
			else nMax = 5; 
			if(len >nMax)
			{
				entry->Entries[Name].Text[nMax*2] = 0;
				entry->Entries[Name].Text[nMax*2+1] = 0;
			}
		}

/*		if(Company!=-1)
		{
			unsigned char		szASCIICompany[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
			len = UnicodeLength(entry->Entries[Company].Text);
			DecodeUnicode(entry->Entries[Company].Text,szASCIICompany);
			if(strlen((char*)szASCIICompany) != len)
			{
				if(PreferUnicode == false)
					PreferUnicode = true; 
			}
		}*/
		error = MTK_SetCharset(PreferUnicode);
		if (error != ERR_NONE) return error;

		switch (theApp.m_PBKCharset) 
		{
		case AT_PBK_GSM:
			smprintf(theApp.m_pDebuginfo, "str: %s\n", DecodeUnicodeString(entry->Entries[Name].Text));
			len = UnicodeLength(entry->Entries[Name].Text);
			EncodeDefault(name, entry->Entries[Name].Text, &len, true, NULL);
			break;
		case AT_PBK_UCS2:
			EncodeHexUnicode(name, entry->Entries[Name].Text, UnicodeLength(entry->Entries[Name].Text));
			len = strlen((char*)name);
			break;
		}
	} else {
		smprintf(theApp.m_pDebuginfo, "WARNING: No usable name found!\n");
		len = 0;
	}

	if (Mobile != -1) 
	{
		MTK_PackSemiOctetNumber(entry->Entries[Mobile].Text, number, false);
		NumberType = number[0];
		sprintf((char*)number,"%s",DecodeUnicodeString(entry->Entries[Mobile].Text));
	} 
	else 
	{
		smprintf(theApp.m_pDebuginfo, "WARNING: No usable number found!\n");
		number[0] = 0;
		NumberType=129;
	}

	if (theApp.m_MemoryInfo.FirstMemoryEntry == 0) {
		error = MTK_GetMemoryInfo();
		if (error != ERR_NONE) return error;
	}

	if(bWithSpace)
		sprintf((char*)req, "AT+CPBW=%d, \"%s\", %i, \"", atoi(entry->szIndex) + theApp.m_MemoryInfo.FirstMemoryEntry - 1, number, NumberType);
	else
		sprintf((char*)req, "AT+CPBW=%d,\"%s\",%i,\"", atoi(entry->szIndex) + theApp.m_MemoryInfo.FirstMemoryEntry - 1, number, NumberType);
	reqlen = strlen((char*)req);
	if (reqlen + len > REQUEST_SIZE - 2) {
		smprintf(theApp.m_pDebuginfo, "WARNING: Text truncated to fit in buffer!\n");
		len = REQUEST_SIZE - 2 - reqlen;
	}
	memcpy(req + reqlen, name, len);
	reqlen += len;
//	memcpy(req + reqlen, "\"\r", 2);
//	reqlen += 2;
	memcpy(req + reqlen, "\"", 1);
	reqlen += 1;
#ifdef _PG1900

//	if(Home!=-1 || Work!=-1|| Fax!=-1|| Email!=-1|| Group!=-1|| Company!=-1)
	{
		unsigned char		req2[REQUEST_SIZE + 1];
		unsigned char		homeNumber[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
		unsigned char		FaxNumber[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
		unsigned char		OfficeNumber[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
		unsigned char		CompanyText[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
		unsigned char		szEmail[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
		int nGroup = 9;
		homeNumber[0] = '\0';
		FaxNumber[0] = '\0';
		OfficeNumber[0] = '\0';
		CompanyText[0] = '\0';
		szEmail[0] = '\0';
		//home
		if(Home!=-1)
			sprintf((char*)homeNumber,"%s",DecodeUnicodeString(entry->Entries[Home].Text));
		//company
		if(Company!=-1)
			EncodeHexUnicode(CompanyText, entry->Entries[Company].Text, UnicodeLength(entry->Entries[Company].Text));
		//email
		if(Email!=-1)
			sprintf((char*)szEmail,"%s",DecodeUnicodeString(entry->Entries[Email].Text));
		//office
		if(Work!=-1)
			sprintf((char*)OfficeNumber,"%s",DecodeUnicodeString(entry->Entries[Work].Text));
		//fax
		if(Fax!=-1)
			sprintf((char*)FaxNumber,"%s",DecodeUnicodeString(entry->Entries[Fax].Text));
		//picture
		//melody
		//group
		if(Group!=-1)
			nGroup = entry->Entries[Group].Number;
		//<home number>,<company name>,<email>,<office number>,<fax number>,<picture tag>,<melody id>,<group id>
//		sprintf((char*)req2, ",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",15400,0,%d",
		sprintf((char*)req2, ",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",0,0,%d",
			homeNumber,CompanyText,szEmail,OfficeNumber,FaxNumber,nGroup);
		memcpy(req + reqlen, req2, strlen((char*)req2));
		reqlen += strlen((char*)req2);

	}
#endif
	memcpy(req + reqlen, "\r", 1);
	reqlen += 1;

	smprintf(theApp.m_pDebuginfo, "Writing phonebook entry\n");
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBW");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
    return theApp.m_pWriteCommandfn ((unsigned char *)req, reqlen, 0x00, 20, false,NULL,&ReplymsgType,MTK_ReplySetMemory);

// 	sprintf((char*)req, "AT*SYPH=1,0,0\r");
// 	smprintf(theApp.m_pDebuginfo, "Deleting phonebook entry\n");
// 	ReplymsgType.nCount = 1;
// 	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*SYPH=");
// 	ReplymsgType.CheckInfo[0].subtypechar = 0;
// 	ReplymsgType.CheckInfo[0].subtype = 0x00;
// 	return theApp.m_pWriteCommandfn (req, strlen((char*)req), 0x00, 20, false,NULL,&ReplymsgType,MTK_ReplySetMemory);

#undef REQUEST_SIZE
}
GSM_Error MTK_ArimaSetMemoryEx(GSM_MemoryEntry *entry,bool bWithSpace)
{
	/* REQUEST_SIZE should be big enough to handle all possibl cases
	 * correctly, especially with unicode entries */
#define REQUEST_SIZE	((4 * GSM_PHONEBOOK_TEXT_LENGTH) + 30)
	int		/*	Group, Name,*/ /*Number*/NumberType=0, len;
	GSM_Error 		error;
	unsigned char		req[REQUEST_SIZE + 1];
	unsigned char		szASCIIName[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
	unsigned char		name[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
	unsigned char		number[GSM_PHONEBOOK_TEXT_LENGTH + 1];
	int			reqlen;
	bool			PreferUnicode = false;

	int nIndex = atoi(entry->szIndex);
	if(strlen(entry->szIndex) <=0 || nIndex==0) return ERR_INVALIDLOCATION;

	error = ATGEN_SetPBKMemory(entry->MemoryType,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo);
	if (error != ERR_NONE) return error;  // Put to Initialize

//	GSM_PhonebookFindDefaultNameNumberGroup(entry, &Name, &Number, &Group);

// 0903 modify by mingfa for arima's group and voip field
	 int Mobile,Name,Home,Work,Fax,Email,Group,Company,Date;
	 int Voip;
//	 MTK_FindEntryDateIndexEx(entry, &Mobile,&Name,&Home,&Work,&Fax,&Email,&Group,&Company,&Date);
	 MTK_FindEntryDateIndexEx(entry, &Mobile,&Name,&Home,&Work,&Fax,&Email,&Group,&Company,&Date, &Voip);

	name[0] = 0;
	if (Name != -1) {
		PreferUnicode = true;//peggy +
		len = UnicodeLength(entry->Entries[Name].Text);
		DecodeUnicode(entry->Entries[Name].Text,szASCIIName);
		int nMax = 0;	
		if(strlen((char*)szASCIIName) == len)
		{
		// modified by mingfa for Arima UCS2
		//	PreferUnicode = false;
			PreferUnicode = true;
			if(entry->MemoryType == MEM_ME)
				nMax = 30;
            //    nMax = 15; //0628 modify by mingfa for UCS2
			else nMax = 12; 
			if(len >nMax)
			{
				entry->Entries[Name].Text[nMax*2] = 0;
				entry->Entries[Name].Text[nMax*2+1] = 0;
			}
		}
		else
		{
			PreferUnicode = true;
			if(entry->MemoryType == MEM_ME)
				nMax = 14;
			else nMax = 5; 
			if(len >nMax)
			{
				entry->Entries[Name].Text[nMax*2] = 0;
				entry->Entries[Name].Text[nMax*2+1] = 0;
			}
		}
		



/*		if(Company!=-1)
		{
			unsigned char		szASCIICompany[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
			len = UnicodeLength(entry->Entries[Company].Text);
			DecodeUnicode(entry->Entries[Company].Text,szASCIICompany);
			if(strlen((char*)szASCIICompany) != len)
			{
				if(PreferUnicode == false)
					PreferUnicode = true; 
			}
		}*/
		error = MTK_SetCharset(PreferUnicode);
		if (error != ERR_NONE) return error;

		switch (theApp.m_PBKCharset) {
		case AT_PBK_GSM:
			smprintf(theApp.m_pDebuginfo, "str: %s\n", DecodeUnicodeString(entry->Entries[Name].Text));
			len = UnicodeLength(entry->Entries[Name].Text);
			EncodeDefault(name, entry->Entries[Name].Text, &len, true, NULL);
			break;
		case AT_PBK_UCS2:
			EncodeHexUnicode(name, entry->Entries[Name].Text, UnicodeLength(entry->Entries[Name].Text));
			len = strlen((char*)name);
			break;
		}
	} else {
		smprintf(theApp.m_pDebuginfo, "WARNING: No usable name found!\n");
		len = 0;
	}

	if (Mobile != -1) {
		MTK_PackSemiOctetNumber(entry->Entries[Mobile].Text, number, false);
		NumberType = number[0];
		sprintf((char*)number,"%s",DecodeUnicodeString(entry->Entries[Mobile].Text));
	} else {
		smprintf(theApp.m_pDebuginfo, "WARNING: No usable number found!\n");
		number[0] = 0;
		NumberType=129;
	}

	if (theApp.m_MemoryInfo.FirstMemoryEntry == 0) {
		error = MTK_GetMemoryInfo();
		if (error != ERR_NONE) return error;
	}

	if(bWithSpace)
		sprintf((char*)req, "AT^CPFW=%d, \"%s\", %i, \"", atoi(entry->szIndex) + theApp.m_MemoryInfo.FirstMemoryEntry - 1, number, NumberType);
	else
		sprintf((char*)req, "AT^CPFW=%d,\"%s\",%i,\"", atoi(entry->szIndex) + theApp.m_MemoryInfo.FirstMemoryEntry - 1, number, NumberType);
	reqlen = strlen((char*)req);
	if (reqlen + len > REQUEST_SIZE - 2) {
		smprintf(theApp.m_pDebuginfo, "WARNING: Text truncated to fit in buffer!\n");
		len = REQUEST_SIZE - 2 - reqlen;
	}
	memcpy(req + reqlen, name, len);
	reqlen += len;
//	memcpy(req + reqlen, "\"\r", 2);
//	reqlen += 2;
	memcpy(req + reqlen, "\"", 1);
	reqlen += 1;

//	if(Home!=-1 || Work!=-1|| Fax!=-1|| Email!=-1|| Group!=-1|| Company!=-1)
	{
		unsigned char		req2[REQUEST_SIZE + 1];
		unsigned char		homeNumber[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
		unsigned char		FaxNumber[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
		unsigned char		OfficeNumber[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
		unsigned char		CompanyText[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
		unsigned char		szEmail[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
		unsigned char		szVoip[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)]; // added by mingfa
		char		szBirthday[100];
		int nGroup = 0; // modify by mingfa , default is 0 => "NONE"
		homeNumber[0] = '\0';
		FaxNumber[0] = '\0';
		OfficeNumber[0] = '\0';
		CompanyText[0] = '\0';
		szEmail[0] = '\0';
		sprintf(szBirthday,"0,0,0");
        szVoip[0] = '\0'; // added by mingfa

		//home
		if(Home!=-1)
			sprintf((char*)homeNumber,"%s",DecodeUnicodeString(entry->Entries[Home].Text));
		//company
		if(Company!=-1)
		// modified by mingfa for Arima UCS2
//			sprintf((char*)CompanyText,"%s",DecodeUnicodeString(entry->Entries[Company].Text));
			EncodeHexUnicode(CompanyText, entry->Entries[Company].Text, UnicodeLength(entry->Entries[Company].Text));


			//email
		if(Email!=-1)
			sprintf((char*)szEmail,"%s",DecodeUnicodeString(entry->Entries[Email].Text));
		//office
		if(Work!=-1)
			sprintf((char*)OfficeNumber,"%s",DecodeUnicodeString(entry->Entries[Work].Text));
		//fax
		if(Fax!=-1)
			sprintf((char*)FaxNumber,"%s",DecodeUnicodeString(entry->Entries[Fax].Text));
		if(Date!=-1)
			sprintf((char*)szBirthday,"%d,%d,%d",entry->Entries[Date].Date.Year,entry->Entries[Date].Date.Month,entry->Entries[Date].Date.Day);
 
		// 0903 added by mingfa for arima group and Voip field
		// Group
		if(Group!=-1)
			nGroup = entry->Entries[Group].Number;
		//Voip
		if(Voip!=-1)
			sprintf((char*)szVoip,"%s",DecodeUnicodeString(entry->Entries[Voip].Text));		


		//<home number>,<company name>,<email>,<office number>,<fax number>,Birthday
//		sprintf((char*)req2, ",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",15400,0,%d",
		//0903 modify by Mingfa
//		sprintf((char*)req2, ",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%s",
//			homeNumber,CompanyText,szEmail,OfficeNumber,FaxNumber,szBirthday);
		sprintf((char*)req2, ",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%s,%d,\"%s\"",
			homeNumber,CompanyText,szEmail,OfficeNumber,FaxNumber,szBirthday,nGroup,szVoip );

		memcpy(req + reqlen, req2, strlen((char*)req2));
		reqlen += strlen((char*)req2);

	}
	memcpy(req + reqlen, "\r", 1);
	reqlen += 1;

	smprintf(theApp.m_pDebuginfo, "Writing phonebook entry\n");
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT^CPFW");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return theApp.m_pWriteCommandfn ((unsigned char *)req, reqlen, 0x00, 20, false,NULL,&ReplymsgType,MTK_ReplySetMemory);
#undef REQUEST_SIZE
}

GSM_Error WINAPI SetMemory (GSM_MemoryEntry *entry,int Control)
{
	if( Control == Type_End ) return ERR_NONE;
	int nIndex = atoi(entry->szIndex);
	if(strlen(entry->szIndex) <=0 || nIndex==0) return ERR_INVALIDLOCATION;
	if(entry->MemoryType == MEM_ME)
	  return NXP_PrivSetMemoryEx(entry,false);
	else
      return MTK_PrivSetMemoryEx(entry,false);
// #endif
// 	return MTK_PrivSetMemoryEx(entry,false);
}
GSM_Error WINAPI DeleteMemory (GSM_MemoryEntry *entry,int Control)
{
	if (entry->MemoryType==MEM_SM)
	{
	
	GSM_Error error;
	unsigned char		req[100];

	if( Control==Type_End) return ERR_NONE;
	if (atoi(entry->szIndex) < 1) return ERR_INVALIDLOCATION;

	error = ATGEN_SetPBKMemory(entry->MemoryType,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo);
	if (error != ERR_NONE) return error;

	if (theApp.m_MemoryInfo.FirstMemoryEntry == 0)
	{
		error = MTK_GetMemoryInfo();;
		if (error != ERR_NONE) return error;
	}

	sprintf((char*)req, "AT+CPBW=%d\r",atoi(entry->szIndex) + theApp.m_MemoryInfo.FirstMemoryEntry - 1);

	smprintf(theApp.m_pDebuginfo, "Deleting phonebook entry\n");
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBW");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;

	return theApp.m_pWriteCommandfn (req, strlen((char*)req), 0x00, 20, false,NULL,&ReplymsgType,MTK_ReplySetMemory);
// 	if (error != ERR_NONE) return error;
// 
// 	sprintf((char*)req, "AT*SYPH=2,0,%d\r",atoi(entry->szIndex) + theApp.m_MemoryInfo.FirstMemoryEntry - 1);
// 	smprintf(theApp.m_pDebuginfo, "Deleting phonebook entry\n");
// 	ReplymsgType.nCount = 1;
// 	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*SYPH=");
// 	ReplymsgType.CheckInfo[0].subtypechar = 0;
// 	ReplymsgType.CheckInfo[0].subtype = 0x00;
// 	return theApp.m_pWriteCommandfn (req, strlen((char*)req), 0x00, 20, false,NULL,&ReplymsgType,MTK_ReplySetMemory);
	}
 	else
 	{
	unsigned char		req[100];
	if( Control==Type_End) return ERR_NONE;
	if (atoi(entry->szIndex) < 1) return ERR_INVALIDLOCATION;
	sprintf((char *)req,"AT*SYPH=2,1,%d\r",atoi(entry->szIndex));
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*SYPH=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return theApp.m_pWriteCommandfn (req, strlen((char*)req), 0x00, 20, false,NULL,&ReplymsgType,MTK_ReplySetMemory);
 	}
}
GSM_Error WINAPI CheckSIMCard(GSM_MemoryStatus *Status)
{
	return ERR_NOTIMPLEMENTED;
}
void InsertSort(int sort[],int n)
{
	int x;
	int i,j;
	for (i=1;i<n;i++)
	{
		x=sort[i];
		for (j=i-1;j>=0;j--)
		if (x<sort[j])
			sort[j+1]=sort[j];
		else
			break;
		sort[j+1]=x;
		
	}
}
GSM_Error NXP_InitPBKGetEx(GSM_MemoryStatus *Status,  int (*pGetStatusfn)(int nCur,int nTotal), int nMax)
{
	GSM_Error error;
	char			req[50];
	theApp.m_MemoryStatus=Status;
	sprintf(req, "AT*STARTUL=%s\r", "/app/dir/smpbtab");
	theApp.isFirst=false;
	theApp.isGetZero=false;
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*STARTUL=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*STARTUL");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,NXP_ReplyGetIndexFileInfo);
	if (error != ERR_NONE&& error != ERR_EMPTY) return error;
	sprintf(req,"at*get\r");
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"at*get");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*get:");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	while (1)
	{
		if (!theApp.isGetZero)
		{
			error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,NXP_ReplyGetDataMemory);
		}
		else
			break;
	}
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
		return error;
	}
	int Length=strlen((char *)theApp.m_unDecodeBase64);
	unsigned char *encodebuffer=new unsigned char[Length*2];
	int encodeLength=Decode((char *)theApp.m_unDecodeBase64,(char *)encodebuffer);

	remove(theApp.m_szPHKIndexFileTempFile);
	FILE *	file;
	file = fopen(theApp.m_szPHKIndexFileTempFile,"a+b");
	if(file)
	{
		int len = fwrite(encodebuffer,1,encodeLength,file);
		fclose(file);
	}
	if (encodebuffer)
	{
	delete encodebuffer;
	}
	file = fopen(theApp.m_szPHKIndexFileTempFile,"r+b");
	if(file)
	{
		int tempgroup[500];
		ZeroMemory(tempgroup,sizeof(tempgroup));
		IndecFileList.RemoveAll();
        t_smpbRamInfo smpbRamInfo;
		for (int i=0;i<500;i++)
		{
		fseek(file,sizeof(t_smpbRamInfo)*i,SEEK_SET);
		fread(&smpbRamInfo,1,sizeof(t_smpbRamInfo),file); 
		if (smpbRamInfo.fileID<=500)
		{
		IndecFileList.AddTail(smpbRamInfo.fileID);
		tempgroup[i]=smpbRamInfo.fileID;
		}
		}
		fclose(file);
		int tempcount=IndecFileList.GetCount();
		InsertSort(tempgroup,tempcount);
		IndecFileList.RemoveAll();
		for (int listcount=0;listcount<tempcount;listcount++)
		{
			IndecFileList.AddTail(tempgroup[listcount]);

		}

	
	}

	int Datacount=0;
 	Datacount=IndecFileList.GetCount();
	if (theApp.m_unDecodeBase64)
	{
		delete theApp.m_unDecodeBase64;
		theApp.m_unDecodeBase64=NULL;
	}
	theApp.m_PHKNum = 0;
	if (Datacount==0)
	{
	Status->MemoryFree = 500 - theApp.m_PHKNum;
	Status->MemoryUsed = theApp.m_PHKNum;
	 return ERR_NONE;
	}
	bool bIsEmpty=false;
	remove(theApp.m_szPHKTempFile);
	for (int j=0;j<25;j++)
	{
		
		sprintf(req, "AT*STARTUL=%s%d\r", "/app/dir/smpb",j);
		ReplymsgType.nCount = 2;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*STARTUL=");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*STARTUL");
		ReplymsgType.CheckInfo[1].subtypechar = 0;
		ReplymsgType.CheckInfo[1].subtype = 0x00;
		error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,NXP_ReplyGetIndexFileInfo);
		if (error != ERR_NONE&& error != ERR_EMPTY)
		{
			bIsEmpty=TRUE;
			break;/*return error;*/
		}
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
		while (1)
		{
			if (!theApp.isGetZero)
			{
				error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,NXP_ReplyGetDataFileMemory);
			}
			else
				break;
		}
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
				return error;
		}
		int DataLength=strlen((char *)theApp.m_unDecodeBase64);
		unsigned char *encodeDatabuffer=new unsigned char[DataLength*2];
		int encodeDataLength=Decode((char *)theApp.m_unDecodeBase64,(char *)encodeDatabuffer);
		remove(theApp.m_szPHKDataFileTempFile);
		file = fopen(theApp.m_szPHKDataFileTempFile,"a+b");
		if(file)
		{
			int len = fwrite(encodeDatabuffer,1,encodeDataLength,file);
			fclose(file);
		}
		file = fopen(theApp.m_szPHKDataFileTempFile,"r+b");
		if(file)
		{
		/*	remove(theApp.m_szPHKTempFile);*/
			ZeroMemory(&theApp.m_PBKMemoryEntry,sizeof(GSM_MemoryEntry));
		/*	theApp.m_PHKNum = 0;*/
			t_smartRecord smartRecord;
			ZeroMemory(&smartRecord,sizeof( t_smartRecord));
			int d=sizeof(t_smartRecord);
			POSITION pos=IndecFileList.GetHeadPosition();
			for (int i=0;i<Datacount;i++)
			{
				int CurrentIndex=IndecFileList.GetNext(pos)-1;//080704libaoliu
				if (CurrentIndex/20==j/*CurrentIndex>=(j*20)&&CurrentIndex<(j+1)*20*/)
				{
				fseek(file,sizeof(t_smartRecord)*((CurrentIndex%20)),SEEK_SET);
				fread(&smartRecord,1,sizeof(t_smartRecord),file);
				GSM_MemoryEntry		*Memory = &theApp.m_PBKMemoryEntry;
				wsprintf(Memory->szIndex,"%d",IndecFileList.GetAt(IndecFileList.FindIndex(i)));
				Memory->Entries[Memory->EntriesNum].EntryType=PBK_Text_Name;
				//EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, (unsigned char *)smartRecord.name, strlen(smartRecord.name));
				DecodeUTF8ToUnicode(Memory->Entries[Memory->EntriesNum].Text, (unsigned char *)smartRecord.name, strlen(smartRecord.name));//080527libaoliu
				Memory->EntriesNum++;
				for (int phonenumber=0 ; phonenumber<5; phonenumber++)
				{
					if (smartRecord.phoneNumberSet[phonenumber].numberType!=SMPB_TYPE_NONE)
					{
						int bcdcount=0;
						switch(smartRecord.phoneNumberSet[phonenumber].numberType)
						{
							unsigned char TempBuffer[100];
						case SMPB_TYPE_MOBILE:
							Memory->Entries[Memory->EntriesNum].EntryType=PBK_Number_Mobile;
							bcd2ascii(smartRecord.phoneNumberSet[phonenumber].BCDPhoneNb,(char *)TempBuffer,100);
							EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text,TempBuffer,strlen((char*)TempBuffer));
							Memory->EntriesNum++;
							break;
						case SMPB_TYPE_HOME:
							Memory->Entries[Memory->EntriesNum].EntryType=PBK_Number_Home;
							bcd2ascii(smartRecord.phoneNumberSet[phonenumber].BCDPhoneNb,(char *)TempBuffer,100);
							EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text,TempBuffer,strlen((char*)TempBuffer));
							Memory->EntriesNum++;
							
							break;
						case SMPB_TYPE_OFFICE:
							Memory->Entries[Memory->EntriesNum].EntryType=PBK_Number_Work;
							bcd2ascii(smartRecord.phoneNumberSet[phonenumber].BCDPhoneNb,(char *)TempBuffer,100);
							EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text,TempBuffer,strlen((char*)TempBuffer));
							Memory->EntriesNum++;
			                 break;
						case SMPB_TYPE_FAX:
							Memory->Entries[Memory->EntriesNum].EntryType=PBK_Number_Fax;
							bcd2ascii(smartRecord.phoneNumberSet[phonenumber].BCDPhoneNb,(char *)TempBuffer,100);
							EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text,TempBuffer,strlen((char*)TempBuffer));
							Memory->EntriesNum++;
							break;
						case SMPB_TYPE_PAGER:
							Memory->Entries[Memory->EntriesNum].EntryType=PBK_Number_Pager;
							bcd2ascii(smartRecord.phoneNumberSet[phonenumber].BCDPhoneNb,(char *)TempBuffer,100);
							EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text,TempBuffer,strlen((char*)TempBuffer));
							Memory->EntriesNum++;
							break;
						}
						
					}
					
				}
// 				int defaultFieldType=smartRecord.defaultFieldType;
//  			int  defaultNumberIndex=smartRecord.defaultNumberIndex;
				Memory->Entries[Memory->EntriesNum].EntryType=PBK_Text_Email;
				EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, (unsigned char *)smartRecord.emailAddress,strlen(smartRecord.emailAddress));
				Memory->EntriesNum++;
				Memory->Entries[Memory->EntriesNum].EntryType=PBK_Text_Postal;
				EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, (unsigned char *)smartRecord.postcode,strlen(smartRecord.postcode));
				Memory->EntriesNum++;
				Memory->Entries[Memory->EntriesNum].EntryType=PBK_Text_Note;
				EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, (unsigned char *)smartRecord.note,strlen(smartRecord.note));
				Memory->EntriesNum++;


				Memory->Entries[Memory->EntriesNum].EntryType=PBK_Text_Company;
				EncodeUnicode(Memory->Entries[Memory->EntriesNum].Text, (unsigned char *)smartRecord.address/*(unsigned char *)pos*/,strlen(smartRecord.address));
				Memory->EntriesNum++;
				Memory->Entries[Memory->EntriesNum].EntryType=PBK_Caller_Group;
				Memory->Entries[Memory->EntriesNum].Number =smartRecord.groupType;
				Memory->Entries[Memory->EntriesNum].Text[0] ='\0';
				Memory->EntriesNum++;
				FILE * Datafile = fopen(theApp.m_szPHKTempFile,"a+b");
				if(Datafile)
				{
					theApp.m_PHKNum++;
					fwrite(&theApp.m_PBKMemoryEntry,1,sizeof(GSM_MemoryEntry),Datafile);
					fclose(Datafile);
				}
				ZeroMemory(&theApp.m_PBKMemoryEntry,sizeof(GSM_MemoryEntry));
				}
				
			}
			fclose(file);
			
		}
		if (encodeDatabuffer)
		{
			delete encodeDatabuffer;
			
		}
		if (theApp.m_unDecodeBase64)
		{
			delete theApp.m_unDecodeBase64;
			theApp.m_unDecodeBase64=NULL;
		}
	}
	if (theApp.m_PHKNum>Datacount)
	{
		theApp.m_PHKNum=Datacount;
	}
	Status->MemoryFree = 500 - theApp.m_PHKNum;
	Status->MemoryUsed = theApp.m_PHKNum;
	if (bIsEmpty)
	{
		return ERR_NONE;
	}
// 	if (encodeDatabuffer)
// 	{
// 	delete encodeDatabuffer;
// 
// 	}
// 	if (theApp.m_unDecodeBase64)
// 	{
// 		delete theApp.m_unDecodeBase64;
// 		theApp.m_unDecodeBase64=NULL;
// 	}
	return error;
}
GSM_Error MTK_InitPBKGetEx(GSM_MemoryStatus *Status,  int (*pGetStatusfn)(int nCur,int nTotal), int nMax)
{
	GSM_Error error;
	char			req[20];
	int			start;
	int         max_no;
	int end;
	error = MTK_SetCharset(true);
	if (error != ERR_NONE) return error;

	error = ATGEN_SetPBKMemory(Status->MemoryType,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo);
	if (error != ERR_NONE) return error;

	theApp.m_MemoryStatus=Status;

	error = MTK_GetMemoryInfo();
	if (error != ERR_NONE) return error;
	max_no = theApp.m_MemoryInfo.MemorySize;

	theApp.m_MemoryStatus	= Status;
	Status->MemoryUsed		= 0;
	Status->MemoryFree		= 0;
	start				= theApp.m_MemoryInfo.FirstMemoryEntry;
//	theApp.m_NextMemoryEntry		= 0;
	theApp.m_MemoryInfo.MemorySize += start-1; //Jay add for Motorola comparion start and end
	theApp.m_PHKNum = 0;
	remove(theApp.m_szPHKTempFile);

	ZeroMemory(&theApp.m_PBKMemoryEntry,sizeof(GSM_MemoryEntry));

	int nFirstIndex = start;
	int nCotiuneGet = true;


	while (nCotiuneGet && start <= theApp.m_MemoryInfo.MemorySize) 
	{
		if(nMax==1)
		{
			sprintf(req, "AT+CPBR=%i\r", start++);
		}
		else
		{
			end	= start + nMax;
			if (end > theApp.m_MemoryInfo.MemorySize) end = theApp.m_MemoryInfo.MemorySize;
			sprintf(req, "AT+CPBR=%i,%i\r", start, end);
		}
		ReplymsgType.nCount = 2;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBR=");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;

		wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"+CPBR:");
		ReplymsgType.CheckInfo[1].subtypechar = 0;
		ReplymsgType.CheckInfo[1].subtype = 0x00;

		error = theApp.m_pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,MTK_ReplyGetMutiMemory);

		if (error != ERR_NONE&& error != ERR_EMPTY) return error;
		if(pGetStatusfn)
			nCotiuneGet = pGetStatusfn(end-nFirstIndex,max_no);

		if(nMax!=1)
		{
			start = end + 1;
		}
	}
	if(!nCotiuneGet) return ERR_CANCELED;
	Status->MemoryFree = max_no - theApp.m_PHKNum;
	Status->MemoryUsed = theApp.m_PHKNum;
	return ERR_NONE;
	
}
GSM_Error WINAPI InitPHKGet(GSM_MemoryStatus *Status)
{
	GSM_Error error;
	if (Status->MemoryType==MEM_SM)
	error =  MTK_InitPBKGetEx(Status,NULL,10);
 	else
 	 error=NXP_InitPBKGetEx(Status,NULL,1);


	return error;
}
GSM_Error WINAPI GetPhoneBitmap(GSM_Bitmap *Bitmap)
{
	return ERR_NOTSUPPORTED;
}
GSM_Error WINAPI InitPHKGetEx(GSM_MemoryStatus *Status,int (*GetStatus)(int nCur,int nTotal))
{
	GSM_Error error;
	if (Status->MemoryType==MEM_SM)
	{
	 error =MTK_InitPBKGetEx(Status,GetStatus,10);
	}
 	else
	 error=NXP_InitPBKGetEx(Status,GetStatus,1);

	return error;
}

