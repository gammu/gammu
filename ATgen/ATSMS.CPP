#include "stdafx.h"
#include "Atgen.h"
#include "AtGenFundef.h"
#include "coding.h"
#include <afxtempl.h>
extern CATgenApp theApp;
CList<unsigned short,unsigned short>IndexFileList;

extern GSM_Reply_MsgType ReplymsgType;
GSM_SMSMessageLayout PHONE_SMSDeliver = {
	35 	/*	SMS Text	*/,	16  /*	Phone number	*/,
	0 	/*	SMSC Number 	*/,	14  /*	TPDCS		*/,
	28 	/*	SendingDateTime	*/,	255 /*	SMSCDateTime	*/,
	255	/*	TPStatus	*/,	15  /*	TPUDL		*/,
	255	/*	TPVP		*/,	12  /*	firstbyte	*/,
	255	/*	TPMR		*/,	13  /*  TPPID		*/};
GSM_SMSMessageLayout PHONE_SMSSubmit = {
	36 	/*	SMS Text	*/,	17  /*	Phone number	*/,
	0 	/*	SMSC Number 	*/,	15  /*	TPDCS		*/,
	255 	/*	SendingDateTime	*/,	255 /*	SMSCDateTime	*/,
	255	/*	TPStatus	*/,	16  /*	TPUDL		*/,
	29	/*	TPVP		*/,	12  /*	firstbyte	*/,
	13	/*	TPMR		*/,	14  /*  TPPID		*/};
GSM_SMSMessageLayout PHONE_SMSStatusReport = {
	255 	/*	SMS Text	*/,	15  /*	Phone number	*/,
	0 	/*	SMSC Number 	*/,	255 /*	TPDCS		*/,
	27 	/*	SendingDateTime	*/,	34  /*	SMSCDateTime	*/,
	14	/*	TPStatus	*/,	255 /*	TPUDL		*/,
	255	/*	TPVP		*/,	12  /*	firstbyte	*/,
	13	/*	TPMR		*/,	255 /*  TPPID??		*/};
// typedef struct 
// {
//     unsigned long v_TimeInfo; /* 12b:4b:5b:5b:6b  ----  Year:Month:Date:Hour:Minute */
//     unsigned char v_MessageStatus;    /* 1 read; 3 unread; 5 sent; 7 unsent  */
//     unsigned char v_MediumType;       /*	1: SIM card  0: Flash */
//     unsigned short v_PhysicsId;            /* not useful for us*/
//     unsigned char v_IsExtendedMsg;   /*   related with EMS, not used   */
//     unsigned short	v_ConcatMsgRef; 
//     unsigned char	v_ConcatDetails;
//     unsigned char	v_MaxConcatMsg;
//     unsigned char	v_ConcatSequencePos;
//     unsigned char	v_IsFirstReceived;
//     unsigned char	v_EmsCompelete;
// } t_SMSMixListRecord;
// 
// typedef struct 
// {
//     t_SMSMixListRecord  MixListRecord[1000];   /* 1000 for ram and  N is the */         /* capacity of current SIM card  */
//     unsigned char RecordUsed[1000];  /* if RecordUsed[n] is 0xff, indicate that there is a SMS at index n, and if RecordUsed[n] is 0, there is nothing at that index.*/
// } t_SMSMixListTable;

	typedef struct {
		unsigned char 	v_AttachType;
		unsigned char	v_AttachDataPredefined;
		unsigned short 	v_Reserved;
	} t_EMS_AttachInfo;
	
	typedef struct{
		unsigned char  v_MsgId;  /* not useful for us*/
		unsigned char  v_PosInSim; /* not useful for us*/
		unsigned char  v_MessageStatus; /* 1 read; 3 unread; 5 sent; 7 unsent  */
		unsigned char  v_MsgIcon; /* emotion icon Id, not useful for us */
		unsigned long  v_time_info; /* 12b:4b:5b:5b:6b  ----  Year:Month:Date:Hour:Minute */
		char           a_PhoneNumber[42];  /* phone numer of the sender or recipient*/ 
		char           a_UserData[247];   /* the content of this short message */
		unsigned char  v_IsExtendedMsg; /* not used */
	} t_SmsRecord;
	typedef struct {
		t_SmsRecord		    v_UserInfo;
		unsigned char		v_ConcatInfo[8];		/* no use for pc tools */
		unsigned char		v_NumOfAttach;		/* no use for pc tools */
		unsigned char 		v_Alphabet;			/* no use for pc tools */
		unsigned short 		v_Reserved;			/* no use for pc tools */
		t_EMS_AttachInfo    v_EMS_AttachInfo[10];	/* no use for pc tools */
	} t_EMSRecord;

		typedef struct{
		unsigned char  v_MsgId;  /* not useful for us*/
		unsigned char  v_PosInSim; /* not useful for us*/
		unsigned char  v_MessageStatus; /* 1 read; 3 unread; 5 sent; 7 unsent  */
		unsigned char  v_MsgIcon; /* emotion icon Id, not useful for us */
		unsigned long  v_time_info; /* 12b:4b:5b:5b:6b  ----  Year:Month:Date:Hour:Minute */
		char           a_PhoneNumber[42];  /* phone numer of the sender or recipient*/ 
		char           a_ReplyCenter[42];
		char           a_UserData[247];   /* the content of this short message */
		unsigned char  v_IsExtendedMsg; /* not used */
	} t_NewSmsRecord;
	typedef struct {
		t_NewSmsRecord		    v_UserInfo;
		unsigned char		v_ConcatInfo[8];		/* no use for pc tools */
		unsigned char		v_NumOfAttach;		/* no use for pc tools */
		unsigned char 		v_Alphabet;			/* no use for pc tools */
		unsigned short 		v_Reserved;			/* no use for pc tools */
		t_EMS_AttachInfo    v_EMS_AttachInfo[10];	/* no use for pc tools */
	} t_NewEMSRecord;
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



GSM_Error ATGEN_ReplyGetSMSC(GSM_Protocol_Message msg)
{
	GSM_SMSC		*SMSC = theApp.m_SMSC;
	int			current;
	int			len;
	unsigned char 		buffer[100];
	int nshift = 0;

	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);
	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "SMSC info received\n");

		current = 0;
	//	while (msg.Buffer[current]!='"') current++;
		while (msg.Buffer[current]!=':' && current<msg.Length) current++;
		if(current>=msg.Length)
		{
			current = 0;
			while (msg.Buffer[current]!='"' && current<msg.Length) current++;
			current-=2;

		}
		if(msg.Buffer[current +2] == '"')
		{
			nshift = 1;
		}
		current +=2;


		/* SMSC number */
		/* FIXME: support for all formats */
		current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);

		/* 
		 * Some phones return this as unicode encoded when they are
		 * switched to UCS2 mode, so we try to solve this correctly.
		 */
		len 		= strlen((char*)buffer + nshift) - nshift;
		buffer[len + 1] = 0;
		if ((len > 20) && (len % 4 == 0) && (strchr((char*)buffer + nshift, '+') == NULL)) 
		{
			/* This is probably unicode encoded number */
			DecodeHexUnicode(SMSC->Number,buffer + nshift,len);
		} 
		else  
		{
			EncodeUnicode(SMSC->Number,buffer + nshift,len);
		}
		smprintf(theApp.m_pDebuginfo, "Number: \"%s\"\n",DecodeUnicodeString(SMSC->Number));

		/* Format of SMSC number */
		current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
		smprintf(theApp.m_pDebuginfo, "Format %s\n",buffer);
		/* International number */
		if (!strcmp((char*)buffer,"145")) 
		{
			sprintf((char*)buffer+1,"%s",DecodeUnicodeString(SMSC->Number));
			if (strlen((char*)buffer+1)!=0 && buffer[1] != '+') 
			{
				/* Sony Ericsson issue */
				/* International number is without + */
				buffer[0] = '+';
				EncodeUnicode(SMSC->Number,buffer,strlen((char*)buffer));
			}
		}
		
		SMSC->Format 		= SMS_FORMAT_Text;
		SMSC->Validity.Format = SMS_Validity_RelativeFormat;
		SMSC->Validity.Relative	= SMS_VALID_Max_Time;
		SMSC->Name[0]		= 0;
		SMSC->Name[1]		= 0;
		SMSC->DefaultNumber[0]	= 0;
		SMSC->DefaultNumber[1]	= 0;
		return ERR_NONE;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}
GSM_Error ATGEN_ReplyGetSMSLocation(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);
	int nFlag = 1;
	if(theApp.m_SMSMemory == MEM_SM) 
		nFlag = -1;;
	char			*pos;
	switch (Replynfo.ReplyState) {
	case AT_Reply_Continue:
		pos = strstr((char*)msg.Buffer, "+MMGL:");
		if (pos == NULL) return ERR_UNKNOWN;
		pos += 6; 
		/* Location */

		while (*pos && !isdigit(*pos)) pos++;

		theApp.m_SMSUsedLocation.nIndex[theApp.m_SMSUsedLocation.Number] = atoi(pos)*nFlag;
		theApp.m_SMSUsedLocation.Number += 1;
		do
		{
			pos = strstr(pos, "+MMGL:");
			if(pos != NULL)
			{
				pos += 6; 
				while (*pos && !isdigit(*pos)) pos++;
				theApp.m_SMSUsedLocation.nIndex[theApp.m_SMSUsedLocation.Number] = atoi(pos)*nFlag;
				theApp.m_SMSUsedLocation.Number += 1;

			}
		}while(pos!=NULL);
		return ERR_NEEDANOTHEDATA;
	case AT_Reply_OK:
		pos = strstr((char*)msg.Buffer, "+MMGL:");
		if (pos == NULL) return ERR_NONE;
		pos += 6; 
		/* Location */

		while (*pos && !isdigit(*pos)) pos++;

		theApp.m_SMSUsedLocation.nIndex[theApp.m_SMSUsedLocation.Number] = atoi(pos)*nFlag;
		theApp.m_SMSUsedLocation.Number += 1;
		do
		{
			pos = strstr(pos, "+MMGL:");
			if(pos != NULL)
			{
				pos += 6; 
				while (*pos && !isdigit(*pos)) pos++;
				theApp.m_SMSUsedLocation.nIndex[theApp.m_SMSUsedLocation.Number] = atoi(pos)*nFlag;
				theApp.m_SMSUsedLocation.Number += 1;

			}
		}while(pos!=NULL);


		return ERR_NONE;
	case AT_Reply_Error:
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(Replynfo);
	default:
		return ERR_UNKNOWNRESPONSE;
	}
}
GSM_Error ATGEN_ReplySendSMS(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	char			*start;

	if (theApp.m_EditMode) 
	{
		if (Replynfo.ReplyState != AT_Reply_SMSEdit)
		{
				return ERR_UNKNOWN;
		}
		theApp.m_EditMode = false;
		theApp.pSetATProtocolDatafn(false,true,0x01);
	//	s->Protocol.Data.AT.EditMode = false;
		return ERR_NONE;
	}

	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
 		smprintf(theApp.m_pDebuginfo, "SMS sent OK\n");
 	/*	if (s->User.SendSMSStatus!=NULL) {
			start = strstr(msg.Buffer, "+CMGW: ");
			if (start != NULL) {
				s->User.SendSMSStatus(s->CurrentConfig->Device,0,atoi(start+7));
			} else {
				s->User.SendSMSStatus(s->CurrentConfig->Device,0,-1);
			}
		}*/
		if(theApp.m_bGetSendSMSResult == false)
		{
			start = strstr((char*)msg.Buffer, "+CMGS: ");
			if (start != NULL) 
			{
				smprintf(theApp.m_pDebuginfo, "SMS sent OK, TPMR for sent sms is %02x\n",msg.Buffer[10]);
				theApp.m_SendSMSStatus = ERR_NONE;
				theApp.m_bGetSendSMSResult = true;
			}
		}
		return ERR_NONE;
	case AT_Reply_CMSError:
 		smprintf(theApp.m_pDebuginfo, "Error %i\n",Replynfo.ErrorCode);
 	//	if (s->User.SendSMSStatus!=NULL) s->User.SendSMSStatus(s->CurrentConfig->Device,theApp.m_ErrorCode,-1);
 		if(theApp.m_bGetSendSMSResult == false)
		{
			if( Replynfo.ErrorCode == 0)
				theApp.m_SendSMSStatus = ERR_NONE;
			else 
				theApp.m_SendSMSStatus = ERR_UNKNOWN;

			theApp.m_bGetSendSMSResult = true;
		}
		return ATGEN_HandleCMSError(Replynfo);
	case AT_Reply_Error:
		return ERR_UNKNOWN;
	default:
		return ERR_UNKNOWNRESPONSE;
	}
}
GSM_Error ATGEN_ReplyDeleteSMSMessage(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);
	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "SMS deleted OK\n");
		return ERR_NONE;
	case AT_Reply_Error:
		smprintf(theApp.m_pDebuginfo, "Invalid location\n");
		return ERR_INVALIDLOCATION;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_ReplyGetSMSMemories(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);
	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		/* Reply here is:
		 * (memories for reading)[, (memories for writing)[, (memories for storing received messages)]]
		 * each memory is in quotes, 
		 * Example: ("SM"), ("SM"), ("SM")
		 *
		 * We need to get from this supported memories. For this case
		 * we assume, that just appearence of memory makes it
		 * available for everything. Then we need to find out whether
		 * phone supports writing to memory. This is done by searching
		 * for "), (", which will appear between lists.
		 */
		theApp.m_CanSaveSMS = false;
		if (strstr((char*)msg.Buffer, "), (") != NULL || strstr((char*)msg.Buffer, "),(") != NULL) 
		{
			theApp.m_CanSaveSMS = true;
		}

		if (strstr((char*)msg.Buffer, "\"SM\"") != NULL) theApp.m_SIMSMSMemory = AT_AVAILABLE;
		else theApp.m_SIMSMSMemory = AT_NOTAVAILABLE;

		if (strstr((char*)msg.Buffer, "\"ME\"") != NULL) theApp.m_PhoneSMSMemory = AT_AVAILABLE;
		else theApp.m_PhoneSMSMemory =AT_NOTAVAILABLE;
	
		if (strstr((char*)msg.Buffer, "\"MT\"") != NULL &&
			theApp.m_SIMSMSMemory == AT_NOTAVAILABLE && 
			theApp.m_PhoneSMSMemory == AT_NOTAVAILABLE )
		{
			theApp.m_PhoneSMSMemory = AT_AVAILABLE;
		}

		smprintf(theApp.m_pDebuginfo, "Available SMS memories received, ME = %d, SM = %d, cansavesms =", theApp.m_PhoneSMSMemory, theApp.m_SIMSMSMemory);
		if (theApp.m_CanSaveSMS) smprintf(theApp.m_pDebuginfo, "true");
		smprintf(theApp.m_pDebuginfo, "\n");
		return ERR_NONE;
	case AT_Reply_Error:
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(Replynfo);
	default:
		return ERR_UNKNOWNRESPONSE;
	}
}
GSM_Error ATGEN_ReplyAddSMSMessage(GSM_Protocol_Message msg)
{
	char 	*start;
	int	i;
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	if (theApp.m_EditMode) 
	{
		if (Replynfo.ReplyState != AT_Reply_SMSEdit)
		{
			return ATGEN_HandleCMSError(Replynfo);
		}
		theApp.m_EditMode = false;
		theApp.pSetATProtocolDatafn(false,true,0x01);
	//	s->Protocol.Data.AT.EditMode = false;
		return ERR_NONE;
	}

	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "SMS saved OK\n");
		for(i=0;i<msg.Length;i++) {
			if (msg.Buffer[i] == 0x00) msg.Buffer[i] = 0x20;
		}
		start = strstr((char*)msg.Buffer, "+CMGW: ");
		if (start == NULL) return ERR_UNKNOWN;
		theApp.m_SaveSMSMessage->Location = atoi(start+7);
		smprintf(theApp.m_pDebuginfo, "Saved at location %i\n",theApp.m_SaveSMSMessage->Location);
		return ERR_NONE;
	case AT_Reply_Error:
		smprintf(theApp.m_pDebuginfo, "Error\n");
		return ERR_NOTSUPPORTED;
	case AT_Reply_CMSError:
		{
			return ATGEN_HandleCMSError(Replynfo);
		/* This error occurs in case that phone couldn't save SMS */
		}
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}
GSM_Error MOTO_ReplyGetSMSMessage(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	GSM_SMSMessage		*sms	= &theApp.m_GetSMSMessage->SMS[0];
	int 			current = 0, current2, i,msglength;
	unsigned char 		buffer[8000],smsframe[800];
	unsigned char		firstbyte, TPDCS, TPUDL, TPStatus;
	GSM_Error		error;
	ZeroMemory(&smsframe,800);
	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		if (theApp.m_Lines.numbers[4] == 0x00) return ERR_EMPTY;
		theApp.m_GetSMSMessage->Number 	 	= 1;
		theApp.m_GetSMSMessage->SMS[0].Name[0] 	= 0;
		theApp.m_GetSMSMessage->SMS[0].Name[1]	= 0;
		switch (theApp.m_SMSMode) {
		case SMS_AT_PDU:
			CopyLineString(buffer, msg.Buffer, theApp.m_Lines, 2);
			switch (buffer[7]) {
				case '0': sms->State = SMS_UnRead; 	break;
				case '1': sms->State = SMS_Read;	break;
				case '2': sms->State = SMS_UnSent;	break;
				default : sms->State = SMS_Sent;	break;//case '3'
			}
			DecodeHexBin (buffer, (unsigned char *)GetLineString(msg.Buffer,theApp.m_Lines,3), strlen(GetLineString(msg.Buffer,theApp.m_Lines,3)));
			/* Siemens MC35 (only ?) */
			if (strstr((char *)msg.Buffer,"+CMGR: 0,,0")!=NULL) return ERR_EMPTY;
			/* Siemens M20 */
			if (IsPhoneFeatureAvailable(theApp.m_pMobileInfo, F_M20SMS)) {
				/* we check for the most often visible */
				if (buffer[1]!=NUMBER_UNKNOWN_NUMBERING_PLAN_ISDN && buffer[1]!=NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN &&
				    buffer[1]!=NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN) {
					/* Seems to be Delivery Report */
					smprintf(theApp.m_pDebuginfo, "SMS type - status report (M20 style)\n");
					sms->PDU 	 = SMS_Status_Report;
					sms->Folder 	 = 1;	/*INBOX SIM*/
					sms->InboxFolder = true;

					smsframe[12]=buffer[current++];
					smsframe[PHONE_SMSStatusReport.TPMR]=buffer[current++];
					current2=((buffer[current])+1)/2+1;
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSStatusReport.Number+i]=buffer[current++];
					for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.DateTime+i]=buffer[current++];
					smsframe[0] = 0;
					for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.SMSCTime+i]=buffer[current++];
					smsframe[PHONE_SMSStatusReport.TPStatus]=buffer[current];
					GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSStatusReport);
					return ERR_NONE;					
				}
			}
			/* We use locations from SMS layouts like in ../phone2.c(h) */
			for(i=0;i<buffer[0]+1;i++) smsframe[i]=buffer[current++];
			smsframe[12]=buffer[current++];
			/* See GSM 03.40 section 9.2.3.1 */
			switch (smsframe[12] & 0x03) {
			case 0x00:
				smprintf(theApp.m_pDebuginfo, "SMS type - deliver\n");
				sms->PDU 	 = SMS_Deliver;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 1; /*INBOX SIM*/ 
				} else {
					sms->Folder = 3; /*INBOX ME*/ 
				}
				sms->InboxFolder = true;
				current2=((buffer[current])+1)/2+1;
				if (IsPhoneFeatureAvailable(theApp.m_pMobileInfo, F_M20SMS)) {
					if (buffer[current+1]==NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN) {
						smprintf(theApp.m_pDebuginfo, "Trying to read alphanumeric number\n");
						for(i=0;i<4;i++) smsframe[PHONE_SMSDeliver.Number+i]=buffer[current++];
						current+=6;
						for(i=0;i<current2-3;i++) smsframe[PHONE_SMSDeliver.Number+i+4]=buffer[current++];
					} else {
						for(i=0;i<current2+1;i++) smsframe[PHONE_SMSDeliver.Number+i]=buffer[current++];
					}
				} else {
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSDeliver.Number+i]=buffer[current++];
				}
				smsframe[PHONE_SMSDeliver.TPPID] = buffer[current++];
				smsframe[PHONE_SMSDeliver.TPDCS] = buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSDeliver.DateTime+i]=buffer[current++];
				smsframe[PHONE_SMSDeliver.TPUDL] = buffer[current++];
				for(i=0;i<smsframe[PHONE_SMSDeliver.TPUDL];i++) smsframe[i+PHONE_SMSDeliver.Text]=buffer[current++];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSDeliver);
				return ERR_NONE;
			case 0x01:
				smprintf(theApp.m_pDebuginfo, "SMS type - submit\n");
				sms->PDU 	 = SMS_Submit;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/ 
					smprintf(theApp.m_pDebuginfo, "Outbox SIM\n");
				} else {
					sms->Folder = 4; /*OUTBOX ME*/ 
				}
				sms->InboxFolder = false;
				smsframe[PHONE_SMSSubmit.TPMR] = buffer[current++];
				current2=((buffer[current])+1)/2+1;
				if (IsPhoneFeatureAvailable(theApp.m_pMobileInfo, F_M20SMS)) {
					if (buffer[current+1]==NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN) {
						smprintf(theApp.m_pDebuginfo, "Trying to read alphanumeric number\n");
						for(i=0;i<4;i++) smsframe[PHONE_SMSSubmit.Number+i]=buffer[current++];
						current+=6;
						for(i=0;i<current2-3;i++) smsframe[PHONE_SMSSubmit.Number+i+4]=buffer[current++];
					} else {
						for(i=0;i<current2+1;i++) smsframe[PHONE_SMSSubmit.Number+i]=buffer[current++];
					}
				} else {
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSSubmit.Number+i]=buffer[current++];
				}
				smsframe[PHONE_SMSSubmit.TPPID] = buffer[current++];
				smsframe[PHONE_SMSSubmit.TPDCS] = buffer[current++];
				/* See GSM 03.40 9.2.3.3 - TPVP can not exist in frame */
				if ((smsframe[12] & 0x18)!=0) current++; //TPVP is ignored now
				smsframe[PHONE_SMSSubmit.TPUDL] = buffer[current++];
				for(i=0;i<smsframe[PHONE_SMSSubmit.TPUDL];i++) smsframe[i+PHONE_SMSSubmit.Text]=buffer[current++];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSSubmit);
				return ERR_NONE;
			case 0x02:
				smprintf(theApp.m_pDebuginfo, "SMS type - status report\n");
				sms->PDU 	 = SMS_Status_Report;
				sms->Folder 	 = 1;	/*INBOX SIM*/
				sms->InboxFolder = true;
				smprintf(theApp.m_pDebuginfo, "TPMR is %02x\n",buffer[current]);
				smsframe[PHONE_SMSStatusReport.TPMR] = buffer[current++];
				current2=((buffer[current])+1)/2+1;
				for(i=0;i<current2+1;i++) smsframe[PHONE_SMSStatusReport.Number+i]=buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.DateTime+i]=buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.SMSCTime+i]=buffer[current++];
				smsframe[PHONE_SMSStatusReport.TPStatus]=buffer[current];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSStatusReport);
				return ERR_NONE;
			}
			break;
		case SMS_AT_TXT:
			current = 0;
			msglength = strlen((char *)msg.Buffer)-6;
			while (msg.Buffer[current]!='"') current++;
			current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
			if (!strcmp((char *)buffer,"\"0\"") || !strcmp((char *)buffer,"\"REC UNREAD\"")) {
				smprintf(theApp.m_pDebuginfo, "SMS type - deliver\n");
				sms->State 	 = SMS_UnRead;
				sms->PDU 	 = SMS_Deliver;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 1; /*INBOX SIM*/ 
				} else {
					sms->Folder = 3; /*INBOX ME*/ 
				}
				sms->InboxFolder = true;
			} else if (!strcmp((char *)buffer,"\"1\"") || !strcmp((char *)buffer,"\"REC READ\"")) {
				smprintf(theApp.m_pDebuginfo, "SMS type - deliver\n");
				sms->State 	 = SMS_Read;
				sms->PDU 	 = SMS_Deliver;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 1; /*INBOX SIM*/ 
				} else {
					sms->Folder = 3; /*INBOX ME*/ 
				}
				sms->InboxFolder = true;
			} else if (!strcmp((char *)buffer,"\"2\"") || !strcmp((char *)buffer,"\"STO UNSENT\"")) {
				smprintf(theApp.m_pDebuginfo, "SMS type - submit\n");
				sms->State 	 = SMS_UnSent;
				sms->PDU 	 = SMS_Submit;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/ 
				} else {
					sms->Folder = 4; /*OUTBOX ME*/ 
				}
				sms->InboxFolder = false;
			} else if (!strcmp((char *)buffer,"\"3\"") || !strcmp((char *)buffer,"\"STO SENT\"")) {
				smprintf(theApp.m_pDebuginfo, "SMS type - submit\n");
				sms->State 	 = SMS_Sent;
				sms->PDU 	 = SMS_Submit;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/ 
				} else {
					sms->Folder = 4; /*OUTBOX ME*/ 
				}
				sms->InboxFolder = false;
			}
			current += ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
			/* It's delivery report according to Nokia AT standards */
			if (sms->Folder==1 && buffer[0]!=0 && buffer[0]!='"'&& buffer[1]!='"') {
				/* ??? */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* format of sender number */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* Sender number */
				/* FIXME: support for all formats */
				//peggy add +
				if(theApp.m_PBKCharset ==AT_PBK_UCS2)
				{
					DecodeHexUnicode(sms->Number, buffer+1,strlen((char *)buffer)-2);

				}
				else
				//peggy add -
					EncodeUnicode(sms->Number,buffer+1,strlen((char *)buffer)-2);
				smprintf(theApp.m_pDebuginfo, "Sender \"%s\"\n",DecodeUnicodeString(sms->Number));
				/* ??? */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* Sending datetime */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				i = strlen((char *)buffer);
				buffer[i] = ',';
				i++;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
				smprintf(theApp.m_pDebuginfo, "\"%s\"\n",buffer);
				ATGEN_DecodeDateTime(&sms->DateTime, buffer+1);
				/* Date of SMSC response */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				i = strlen((char *)buffer);
				buffer[i] = ',';
				i++;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
				smprintf(theApp.m_pDebuginfo, "\"%s\"\n",buffer);
				ATGEN_DecodeDateTime(&sms->SMSCTime, buffer+1);
				/* TPStatus */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPStatus=atoi((char *)buffer);
				buffer[PHONE_SMSStatusReport.TPStatus] = TPStatus;
				error=GSM_DecodeSMSFrameStatusReportData(sms, buffer, PHONE_SMSStatusReport);
				if (error!=ERR_NONE) return error;
				/* NO SMSC number */
				sms->SMSC.Number[0]=0;
				sms->SMSC.Number[1]=0;
				sms->PDU = SMS_Status_Report;
				sms->ReplyViaSameSMSC=false;
			} else {
				/* Sender number */
				/* FIXME: support for all formats */
				if(theApp.m_PBKCharset ==AT_PBK_UCS2 && *(buffer+1)!='"'&& *(buffer)!='"')
				{
					DecodeHexUnicode(sms->Number, buffer+1,strlen((char *)buffer+1)-2);
				}
				else
				{
					if(*(buffer+1)=='"')
						EncodeUnicode(sms->Number,buffer+2,strlen((char *)buffer+1)-3);
					else
						EncodeUnicode(sms->Number,buffer+1,strlen((char *)buffer+1)-2);
				}
				/* Sender number in alphanumeric format ? */
				if(current >= msglength) return ERR_NONE;
				sms->Coding = SMS_Coding_Default;;
				sms->UDH.Type	= UDH_NoUDH;
				sms->Length	= TPUDL;
				if(theApp.m_PBKCharset ==AT_PBK_UCS2 && *(msg.Buffer+theApp.m_Lines.numbers[2*2])!='"')
				{
				//	DecodeHexUnicode(sms->Text, buffer+1,strlen((char *)buffer)-1);
					DecodeHexUnicode(sms->Text,msg.Buffer+theApp.m_Lines.numbers[2*2],TPUDL);

				}
				else
				//peggy add -
					EncodeUnicode(sms->Text,msg.Buffer+theApp.m_Lines.numbers[2*2],TPUDL);
			//		EncodeUnicode(sms->Text,buffer+1,strlen((char *)buffer)-1);
		/*		if (strlen((char *)buffer)!=0) 
				{
					//peggy add +
				}*/
			//	current += ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				smprintf(theApp.m_pDebuginfo, "Sender \"%s\"\n",DecodeUnicodeString(sms->Number));
				/* Sending datetime */
				if (sms->Folder==1 || sms->Folder==3) {
					current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
					/* FIXME: ATGEN_ExtractOneParameter() is broken as it doesn't respect
					 * quoting of parameters and thus +FOO: "ab","cd,ef" will consider
					 * as three arguments: "ab" >> "cd >> ef"
					 */
					if (*buffer=='"' || (*buffer==' ' && *(buffer+1)=='"')) {
						i = strlen((char *)buffer);
						buffer[i] = ',';
						i++;
						current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
					}
					smprintf(theApp.m_pDebuginfo, "\"%s\"\n",buffer);
					if (*buffer)
					{
						if(*(buffer+1) == '"')
							ATGEN_DecodeDateTime(&sms->DateTime, buffer+2);
						else
							ATGEN_DecodeDateTime(&sms->DateTime, buffer+1);
					}
					else {
						/* FIXME: What is the proper undefined GSM_DateTime ? */
						memset(&sms->DateTime, 0, sizeof(sms->DateTime));
					}
				//	ATGEN_DecodeDateTime(&sms->DateTime, buffer+1);
				}
				/* Sender number format */
				if(current >= msglength) return ERR_NONE;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* First byte */
				if(current >= msglength) return ERR_NONE;
				else
				{
					while (current <msglength)
					{
						current+=ATGEN_ExtractOneParameter(msg.Buffer+current+1 , buffer+strlen((char*)buffer));
						current++;
					}
				//	sms->Length = strlen((char*)buffer+1)/4;
					DecodeHexUnicode(sms->Text,buffer+1,320);
					//DecodeHexUnicode(sms->Text,buffer+1,sms->Length);
					//sprintf((char*)sms->Text, (char*)buffer);
					return ERR_NONE;
				}
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				firstbyte=atoi((char *)buffer);
				sms->ReplyViaSameSMSC=false;
				/* GSM 03.40 section 9.2.3.17 (TP-Reply-Path) */
				if ((firstbyte & 128)==128) sms->ReplyViaSameSMSC=true;
				/* TP PID */   
				if(current >= msglength) return ERR_NONE;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				sms->ReplaceMessage = 0;
				if (atoi((char *)buffer) > 0x40 && atoi((char *)buffer) < 0x48) {
					sms->ReplaceMessage = atoi((char *)buffer) - 0x40;
				}
				smprintf(theApp.m_pDebuginfo, "TPPID: %02x %i\n",atoi((char *)buffer),atoi((char *)buffer));
				/* TP DCS */
				if(current >= msglength) return ERR_NONE;

				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPDCS=atoi((char *)buffer);
				if (sms->Folder==2 || sms->Folder==4) {
					/*TP VP */
					current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				}
				/* SMSC number */
				/* FIXME: support for all formats */
				if(current >= msglength) return ERR_NONE;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				//peggy add +
				if(theApp.m_PBKCharset ==AT_PBK_UCS2)
				{
					DecodeHexUnicode(sms->SMSC.Number, buffer+1,strlen((char *)buffer)-2);

				}
				else
				//peggy add -
					EncodeUnicode(sms->SMSC.Number,buffer+1,strlen((char *)buffer)-2);
				/* Format of SMSC number */
				if(current >= msglength) return ERR_NONE;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* TPUDL */
				if(current >= msglength) return ERR_NONE;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPUDL=atoi((char *)buffer);
				current++;
				sms->Coding = SMS_Coding_Default;
				/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme)
				 * and GSM 03.38 section 4
				 */
				if ((TPDCS & 0x04) == 0x04) sms->Coding=SMS_Coding_8bit;
				if ((TPDCS & 0x08) == 0x08) sms->Coding=SMS_Coding_Unicode;
				sms->Class = -1;
				if ((TPDCS & 0xF3)==0xF0) sms->Class = 0;
				if ((TPDCS & 0xF3)==0xF1) sms->Class = 1;
				if ((TPDCS & 0xF3)==0xF2) sms->Class = 2;
				if ((TPDCS & 0xF3)==0xF3) sms->Class = 3;
				smprintf(theApp.m_pDebuginfo, "SMS class: %i\n",sms->Class);
				switch (sms->Coding) {          
				case SMS_Coding_Default:
					/* GSM 03.40 section 9.2.3.23 (TP-User-Data-Header-Indicator) */
					/* If not SMS with UDH, it's coded normal */
					/* If UDH available, treat it as Unicode or 8 bit */
					if ((firstbyte & 0x40)!=0x40) {
						sms->UDH.Type	= UDH_NoUDH;
						sms->Length	= TPUDL;
						EncodeUnicode(sms->Text,msg.Buffer+theApp.m_Lines.numbers[2*2],TPUDL);
						break;
					}
				case SMS_Coding_Unicode:
				case SMS_Coding_8bit:
					DecodeHexBin(buffer+PHONE_SMSDeliver.Text, msg.Buffer+current, TPUDL*2);
					buffer[PHONE_SMSDeliver.firstbyte] 	= firstbyte;
					buffer[PHONE_SMSDeliver.TPDCS] 		= TPDCS;
					buffer[PHONE_SMSDeliver.TPUDL] 		= TPUDL;
					return GSM_DecodeSMSFrameText(sms, buffer, PHONE_SMSDeliver);
				}
			}
			return ERR_NONE;
		default:
			break;
		}
		break;
	case AT_Reply_CMSError:
		if (Replynfo.ErrorCode == 320 || Replynfo.ErrorCode == 500) {
			return ERR_EMPTY;
		} else {
			return ATGEN_HandleCMSError(Replynfo);
		}
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(Replynfo);
	case AT_Reply_Error:
		/* A2D returns Error with empty location */
		return ERR_EMPTY;
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}
GSM_Error LG_ReplyGetSMSMessage(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

//	GSM_Phone_ATGENData 	*Priv 	= &s->Phone.Data.Priv.ATGEN;
	GSM_SMSMessage		*sms	= &theApp.m_GetSMSMessage->SMS[0];
	int 			current = 0, current2, i;
	unsigned char 		buffer[800],smsframe[800],buffer2[800];
	unsigned char		firstbyte, TPDCS, TPUDL, TPStatus;
	GSM_Error		error;
	ZeroMemory(&smsframe,800);
	char			*pos;

	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		if (theApp.m_Lines.numbers[4] == 0x00) return ERR_EMPTY;
		theApp.m_GetSMSMessage->Number 	 	= 1;
		theApp.m_GetSMSMessage->SMS[0].Name[0] 	= 0;
		theApp.m_GetSMSMessage->SMS[0].Name[1]	= 0;
		switch (theApp.m_SMSMode) {
		case SMS_AT_PDU:
				pos = strstr((char*)msg.Buffer, "+CMGR:");
				if (pos == NULL)
					return ERR_UNKNOWN;
				pos += 6; 
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer2);
		//	CopyLineString(buffer, msg.Buffer, theApp.m_Lines, 2);
			switch (atoi((char*)buffer2)) {
				case 0: sms->State = SMS_UnRead; 	break;
				case 1: sms->State = SMS_Read;	break;
				case 2: sms->State = SMS_UnSent;	break;
				default : sms->State = SMS_Sent;	break;//case '3'
			}
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer2);
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer2);
			if (*pos == 0x0A ) pos++;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer2);

		//	DecodeHexBin (buffer,(unsigned char *) GetLineString(msg.Buffer,theApp.m_Lines,3), strlen(GetLineString(msg.Buffer,theApp.m_Lines,3)));
			DecodeHexBin (buffer,(unsigned char *)buffer2, strlen((char*)buffer2));
			/* Siemens MC35 (only ?) */
			if (strstr((char*)msg.Buffer,"+CMGR: 0,,0")!=NULL) return ERR_EMPTY;
			/* Siemens M20 */
			if (IsPhoneFeatureAvailable(theApp.m_pMobileInfo, F_M20SMS)) {
				/* we check for the most often visible */
				if (buffer[1]!=NUMBER_UNKNOWN_NUMBERING_PLAN_ISDN && buffer[1]!=NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN &&
				    buffer[1]!=NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN) {
					/* Seems to be Delivery Report */
					smprintf(theApp.m_pDebuginfo, "SMS type - status report (M20 style)\n");
					sms->PDU 	 = SMS_Status_Report;
					sms->Folder 	 = 1;	/*INBOX SIM*/
					sms->InboxFolder = true;

					smsframe[12]=buffer[current++];
					smsframe[PHONE_SMSStatusReport.TPMR]=buffer[current++];
					current2=((buffer[current])+1)/2+1;
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSStatusReport.Number+i]=buffer[current++];
					for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.DateTime+i]=buffer[current++];
					smsframe[0] = 0;
					for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.SMSCTime+i]=buffer[current++];
					smsframe[PHONE_SMSStatusReport.TPStatus]=buffer[current];
					GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSStatusReport);
					return ERR_NONE;					
				}
			}
			/* We use locations from SMS layouts like in ../phone2.c(h) */
			for(i=0;i<buffer[0]+1;i++) smsframe[i]=buffer[current++];
			smsframe[12]=buffer[current++];
			/* See GSM 03.40 section 9.2.3.1 */
			switch (smsframe[12] & 0x03) {
			case 0x00:
				smprintf(theApp.m_pDebuginfo, "SMS type - deliver\n");
				sms->PDU 	 = SMS_Deliver;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 1; /*INBOX SIM*/ 
				} else {
					sms->Folder = 3; /*INBOX ME*/ 
				}
				sms->InboxFolder = true;
				current2=((buffer[current])+1)/2+1;
				if (IsPhoneFeatureAvailable(theApp.m_pMobileInfo, F_M20SMS)) {
					if (buffer[current+1]==NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN) {
						smprintf(theApp.m_pDebuginfo, "Trying to read alphanumeric number\n");
						for(i=0;i<4;i++) smsframe[PHONE_SMSDeliver.Number+i]=buffer[current++];
						current+=6;
						for(i=0;i<current2-3;i++) smsframe[PHONE_SMSDeliver.Number+i+4]=buffer[current++];
					} else {
						for(i=0;i<current2+1;i++) smsframe[PHONE_SMSDeliver.Number+i]=buffer[current++];
					}
				} else {
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSDeliver.Number+i]=buffer[current++];
				}
				smsframe[PHONE_SMSDeliver.TPPID] = buffer[current++];
				smsframe[PHONE_SMSDeliver.TPDCS] = buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSDeliver.DateTime+i]=buffer[current++];
				smsframe[PHONE_SMSDeliver.TPUDL] = buffer[current++];
				for(i=0;i<smsframe[PHONE_SMSDeliver.TPUDL];i++) smsframe[i+PHONE_SMSDeliver.Text]=buffer[current++];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSDeliver);
				return ERR_NONE;
			case 0x01:
				smprintf(theApp.m_pDebuginfo, "SMS type - submit\n");
				sms->PDU 	 = SMS_Submit;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/ 
					smprintf(theApp.m_pDebuginfo, "Outbox SIM\n");
				} else {
					sms->Folder = 4; /*OUTBOX ME*/ 
				}
				sms->InboxFolder = false;
				smsframe[PHONE_SMSSubmit.TPMR] = buffer[current++];
				current2=((buffer[current])+1)/2+1;
				if (IsPhoneFeatureAvailable(theApp.m_pMobileInfo, F_M20SMS)) {
					if (buffer[current+1]==NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN) {
						smprintf(theApp.m_pDebuginfo, "Trying to read alphanumeric number\n");
						for(i=0;i<4;i++) smsframe[PHONE_SMSSubmit.Number+i]=buffer[current++];
						current+=6;
						for(i=0;i<current2-3;i++) smsframe[PHONE_SMSSubmit.Number+i+4]=buffer[current++];
					} else {
						for(i=0;i<current2+1;i++) smsframe[PHONE_SMSSubmit.Number+i]=buffer[current++];
					}
				} else {
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSSubmit.Number+i]=buffer[current++];
				}
				smsframe[PHONE_SMSSubmit.TPPID] = buffer[current++];
				smsframe[PHONE_SMSSubmit.TPDCS] = buffer[current++];
				/* See GSM 03.40 9.2.3.3 - TPVP can not exist in frame */
				if ((smsframe[12] & 0x18)!=0) current++; //TPVP is ignored now
				smsframe[PHONE_SMSSubmit.TPUDL] = buffer[current++];
				for(i=0;i<smsframe[PHONE_SMSSubmit.TPUDL];i++) smsframe[i+PHONE_SMSSubmit.Text]=buffer[current++];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSSubmit);
				return ERR_NONE;
			case 0x02:
				smprintf(theApp.m_pDebuginfo, "SMS type - status report\n");
				sms->PDU 	 = SMS_Status_Report;
				sms->Folder 	 = 1;	/*INBOX SIM*/
				sms->InboxFolder = true;
				smprintf(theApp.m_pDebuginfo, "TPMR is %02x\n",buffer[current]);
				smsframe[PHONE_SMSStatusReport.TPMR] = buffer[current++];
				current2=((buffer[current])+1)/2+1;
				for(i=0;i<current2+1;i++) smsframe[PHONE_SMSStatusReport.Number+i]=buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.DateTime+i]=buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.SMSCTime+i]=buffer[current++];
				smsframe[PHONE_SMSStatusReport.TPStatus]=buffer[current];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSStatusReport);
				return ERR_NONE;
			}
			break;
		case SMS_AT_TXT:
			current = 0;
			while (msg.Buffer[current]!='"') current++;
			current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
			if (!strcmp((char*)buffer,"\"0\"") || !strcmp((char*)buffer,"\"REC UNREAD\"")) {
				smprintf(theApp.m_pDebuginfo, "SMS type - deliver\n");
				sms->State 	 = SMS_UnRead;
				sms->PDU 	 = SMS_Deliver;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 1; /*INBOX SIM*/ 
				} else {
					sms->Folder = 3; /*INBOX ME*/ 
				}
				sms->InboxFolder = true;
			} else if (!strcmp((char*)buffer,"\"1\"") || !strcmp((char*)buffer,"\"REC READ\"")) {
				smprintf(theApp.m_pDebuginfo, "SMS type - deliver\n");
				sms->State 	 = SMS_Read;
				sms->PDU 	 = SMS_Deliver;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 1; /*INBOX SIM*/ 
				} else {
					sms->Folder = 3; /*INBOX ME*/ 
				}
				sms->InboxFolder = true;
			} else if (!strcmp((char*)buffer,"\"2\"") || !strcmp((char*)buffer,"\"STO UNSENT\"")) {
				smprintf(theApp.m_pDebuginfo, "SMS type - submit\n");
				sms->State 	 = SMS_UnSent;
				sms->PDU 	 = SMS_Submit;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/ 
				} else {
					sms->Folder = 4; /*OUTBOX ME*/ 
				}
				sms->InboxFolder = false;
			} else if (!strcmp((char*)buffer,"\"3\"") || !strcmp((char*)buffer,"\"STO SENT\"")) {
				smprintf(theApp.m_pDebuginfo, "SMS type - submit\n");
				sms->State 	 = SMS_Sent;
				sms->PDU 	 = SMS_Submit;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/ 
				} else {
					sms->Folder = 4; /*OUTBOX ME*/ 
				}
				sms->InboxFolder = false;
			}
			current += ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
			/* It's delivery report according to Nokia AT standards */
			if (sms->Folder==1 && buffer[0]!=0 && buffer[0]!='"') {
				/* ??? */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* format of sender number */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* Sender number */
				/* FIXME: support for all formats */
				EncodeUnicode(sms->Number,buffer+1,strlen((char*)buffer)-2);
				smprintf(theApp.m_pDebuginfo, "Sender \"%s\"\n",DecodeUnicodeString(sms->Number));
				/* ??? */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* Sending datetime */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				i = strlen((char*)buffer);
				buffer[i] = ',';
				i++;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
				smprintf(theApp.m_pDebuginfo, "\"%s\"\n",buffer);
				ATGEN_DecodeDateTime(&sms->DateTime, buffer+1);
				/* Date of SMSC response */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				i = strlen((char*)buffer);
				buffer[i] = ',';
				i++;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
				smprintf(theApp.m_pDebuginfo, "\"%s\"\n",buffer);
				ATGEN_DecodeDateTime(&sms->SMSCTime, buffer+1);
				/* TPStatus */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPStatus=atoi((char*)buffer);
				buffer[PHONE_SMSStatusReport.TPStatus] = TPStatus;
				error=GSM_DecodeSMSFrameStatusReportData(sms, buffer, PHONE_SMSStatusReport);
				if (error!=ERR_NONE) return error;
				/* NO SMSC number */
				sms->SMSC.Number[0]=0;
				sms->SMSC.Number[1]=0;
				sms->PDU = SMS_Status_Report;
				sms->ReplyViaSameSMSC=false;
			} else {
				/* Sender number */
				/* FIXME: support for all formats */
				EncodeUnicode(sms->Number,buffer+1,strlen((char*)buffer)-2);
				/* Sender number in alphanumeric format ? */
				current += ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				if (strlen((char*)buffer)!=0) EncodeUnicode(sms->Number,buffer+1,strlen((char*)buffer)-2);
				smprintf(theApp.m_pDebuginfo, "Sender \"%s\"\n",DecodeUnicodeString(sms->Number));
				/* Sending datetime */
				if (sms->Folder==1 || sms->Folder==3) {
					current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
					/* FIXME: ATGEN_ExtractOneParameter() is broken as it doesn't respect
					 * quoting of parameters and thus +FOO: "ab","cd,ef" will consider
					 * as three arguments: "ab" >> "cd >> ef"
					 */
					if (*buffer=='"') {
						i = strlen((char*)buffer);
						buffer[i] = ',';
						i++;
						current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
					}
					smprintf(theApp.m_pDebuginfo, "\"%s\"\n",buffer);
					if (*buffer)
						ATGEN_DecodeDateTime(&sms->DateTime, buffer+1);
					else {
						/* FIXME: What is the proper undefined GSM_DateTime ? */
						memset(&sms->DateTime, 0, sizeof(sms->DateTime));
					}
					ATGEN_DecodeDateTime(&sms->DateTime, buffer+1);
				}
				/* Sender number format */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* First byte */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				firstbyte=atoi((char*)buffer);
				sms->ReplyViaSameSMSC=false;
				/* GSM 03.40 section 9.2.3.17 (TP-Reply-Path) */
				if ((firstbyte & 128)==128) sms->ReplyViaSameSMSC=true;
				/* TP PID */        
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				sms->ReplaceMessage = 0;
				if (atoi((char*)buffer) > 0x40 && atoi((char*)buffer) < 0x48) {
					sms->ReplaceMessage = atoi((char*)buffer) - 0x40;
				}
				smprintf(theApp.m_pDebuginfo, "TPPID: %02x %i\n",atoi((char*)buffer),atoi((char*)buffer));
				/* TP DCS */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPDCS=atoi((char*)buffer);
				if (sms->Folder==2 || sms->Folder==4) {
					/*TP VP */
					current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				}
				/* SMSC number */
				/* FIXME: support for all formats */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				EncodeUnicode(sms->SMSC.Number,buffer+1,strlen((char*)buffer)-2);
				/* Format of SMSC number */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* TPUDL */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPUDL=atoi((char*)buffer);
				current++;
				sms->Coding = SMS_Coding_Default;
				/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme)
				 * and GSM 03.38 section 4
				 */
				if ((TPDCS & 0x04) == 0x04) sms->Coding=SMS_Coding_8bit;
				if ((TPDCS & 0x08) == 0x08) sms->Coding=SMS_Coding_Unicode;
				sms->Class = -1;
				if ((TPDCS & 0xF3)==0xF0) sms->Class = 0;
				if ((TPDCS & 0xF3)==0xF1) sms->Class = 1;
				if ((TPDCS & 0xF3)==0xF2) sms->Class = 2;
				if ((TPDCS & 0xF3)==0xF3) sms->Class = 3;
				smprintf(theApp.m_pDebuginfo, "SMS class: %i\n",sms->Class);
				switch (sms->Coding) {          
				case SMS_Coding_Default:
					/* GSM 03.40 section 9.2.3.23 (TP-User-Data-Header-Indicator) */
					/* If not SMS with UDH, it's coded normal */
					/* If UDH available, treat it as Unicode or 8 bit */
					if ((firstbyte & 0x40)!=0x40) {
						sms->UDH.Type	= UDH_NoUDH;
						sms->Length	= TPUDL;
						EncodeUnicode(sms->Text,msg.Buffer+theApp.m_Lines.numbers[2*2],TPUDL);
						break;
					}
				case SMS_Coding_Unicode:
				case SMS_Coding_8bit:
					DecodeHexBin(buffer+PHONE_SMSDeliver.Text, msg.Buffer+current, TPUDL*2);
					buffer[PHONE_SMSDeliver.firstbyte] 	= firstbyte;
					buffer[PHONE_SMSDeliver.TPDCS] 		= TPDCS;
					buffer[PHONE_SMSDeliver.TPUDL] 		= TPUDL;
					return GSM_DecodeSMSFrameText(sms, buffer, PHONE_SMSDeliver);
				}
			}
			return ERR_NONE;
		default:
			break;
		}
		break;
	case AT_Reply_CMSError:
		if (Replynfo.ErrorCode == 320 || Replynfo.ErrorCode == 500) {
			return ERR_EMPTY;
		} else {
			return ATGEN_HandleCMSError(Replynfo);
		}
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(Replynfo);
	case AT_Reply_Error:
		/* A2D returns Error with empty location */
		return ERR_EMPTY;
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_ReplyGetSMSMessage(GSM_Protocol_Message msg)
{
	if (theApp.m_ManufacturerID == AT_LG) 
		return LG_ReplyGetSMSMessage(msg);

	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

//	GSM_Phone_ATGENData 	*Priv 	= &s->Phone.Data.Priv.ATGEN;
	GSM_SMSMessage		*sms	= &theApp.m_GetSMSMessage->SMS[0];
	char*	str ;
	int 			current = 0, current2, i;
	unsigned char 		buffer[300],smsframe[800];
	unsigned char		firstbyte, TPDCS, TPUDL, TPStatus;
	GSM_Error		error;
	ZeroMemory(smsframe,800);
//	char retval[8000];

	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		if (theApp.m_Lines.numbers[4] == 0x00) return ERR_EMPTY;
		theApp.m_GetSMSMessage->Number 	 	= 1;
		theApp.m_GetSMSMessage->SMS[0].Name[0] 	= 0;
		theApp.m_GetSMSMessage->SMS[0].Name[1]	= 0;
		switch (theApp.m_SMSMode)
		{
		case SMS_AT_PDU:
			CopyLineString(buffer, msg.Buffer, theApp.m_Lines, 2);
			switch (buffer[7]) 
			{
				case '0': sms->State = SMS_UnRead; 	break;
				case '1': sms->State = SMS_Read;	break;
				case '2': sms->State = SMS_UnSent;	break;
				default : sms->State = SMS_Sent;	break;//case '3'
			}
			str = GetLineString(msg.Buffer,theApp.m_Lines,3);
			if(strlen(str)<=2 && (theApp.m_Lines.numbers[5]-theApp.m_Lines.numbers[4] >2))
			{
				char retval[8000];
				int n = strlen(str);
				memcpy(retval,str,n);
				retval[n]=0x30;
				retval[n+1]=0x30;
				char *p = str+strlen(str) +2;
				memcpy(retval+n+2,p,strlen(p)+2);
				DecodeHexBin (buffer,(unsigned char *)retval, strlen(retval));
			}
			else
				DecodeHexBin (buffer,(unsigned char *)str, strlen(str));
			/* Siemens MC35 (only ?) */
			if (strstr((char*)msg.Buffer,"+CMGR: 0,,0")!=NULL) return ERR_EMPTY;
			/* Siemens M20 */
			if (IsPhoneFeatureAvailable(theApp.m_pMobileInfo, F_M20SMS)) {
				/* we check for the most often visible */
				if (buffer[1]!=NUMBER_UNKNOWN_NUMBERING_PLAN_ISDN && buffer[1]!=NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN &&
				    buffer[1]!=NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN) {
					/* Seems to be Delivery Report */
					smprintf(theApp.m_pDebuginfo, "SMS type - status report (M20 style)\n");
					sms->PDU 	 = SMS_Status_Report;
					sms->Folder 	 = 1;	/*INBOX SIM*/
					sms->InboxFolder = true;

					smsframe[12]=buffer[current++];
					smsframe[PHONE_SMSStatusReport.TPMR]=buffer[current++];
					current2=((buffer[current])+1)/2+1;
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSStatusReport.Number+i]=buffer[current++];
					for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.DateTime+i]=buffer[current++];
					smsframe[0] = 0;
					for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.SMSCTime+i]=buffer[current++];
					smsframe[PHONE_SMSStatusReport.TPStatus]=buffer[current];
					GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSStatusReport);
					return ERR_NONE;					
				}
			}
			/* We use locations from SMS layouts like in ../phone2.c(h) */
			for(i=0;i<buffer[0]+1;i++) smsframe[i]=buffer[current++];
			smsframe[12]=buffer[current++];
			/* See GSM 03.40 section 9.2.3.1 */
			switch (smsframe[12] & 0x03) {
			case 0x00:
				smprintf(theApp.m_pDebuginfo, "SMS type - deliver\n");
				sms->PDU 	 = SMS_Deliver;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 1; /*INBOX SIM*/ 
				} else {
					sms->Folder = 3; /*INBOX ME*/ 
				}
				sms->InboxFolder = true;
				current2=((buffer[current])+1)/2+1;
				if (IsPhoneFeatureAvailable(theApp.m_pMobileInfo, F_M20SMS)) {
					if (buffer[current+1]==NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN) {
						smprintf(theApp.m_pDebuginfo, "Trying to read alphanumeric number\n");
						for(i=0;i<4;i++) smsframe[PHONE_SMSDeliver.Number+i]=buffer[current++];
						current+=6;
						for(i=0;i<current2-3;i++) smsframe[PHONE_SMSDeliver.Number+i+4]=buffer[current++];
					} else {
						for(i=0;i<current2+1;i++) smsframe[PHONE_SMSDeliver.Number+i]=buffer[current++];
					}
				} else {
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSDeliver.Number+i]=buffer[current++];
				}
				smsframe[PHONE_SMSDeliver.TPPID] = buffer[current++];
				smsframe[PHONE_SMSDeliver.TPDCS] = buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSDeliver.DateTime+i]=buffer[current++];
				smsframe[PHONE_SMSDeliver.TPUDL] = buffer[current++];
				for(i=0;i<smsframe[PHONE_SMSDeliver.TPUDL];i++) smsframe[i+PHONE_SMSDeliver.Text]=buffer[current++];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSDeliver);
				return ERR_NONE;
			case 0x01:
				smprintf(theApp.m_pDebuginfo, "SMS type - submit\n");
				sms->PDU 	 = SMS_Submit;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/ 
					smprintf(theApp.m_pDebuginfo, "Outbox SIM\n");
				} else {
					sms->Folder = 4; /*OUTBOX ME*/ 
				}
				sms->InboxFolder = false;
				smsframe[PHONE_SMSSubmit.TPMR] = buffer[current++];
				current2=((buffer[current])+1)/2+1;
				if (IsPhoneFeatureAvailable(theApp.m_pMobileInfo, F_M20SMS)) {
					if (buffer[current+1]==NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN) {
						smprintf(theApp.m_pDebuginfo, "Trying to read alphanumeric number\n");
						for(i=0;i<4;i++) smsframe[PHONE_SMSSubmit.Number+i]=buffer[current++];
						current+=6;
						for(i=0;i<current2-3;i++) smsframe[PHONE_SMSSubmit.Number+i+4]=buffer[current++];
					} else {
						for(i=0;i<current2+1;i++) smsframe[PHONE_SMSSubmit.Number+i]=buffer[current++];
					}
				} else {
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSSubmit.Number+i]=buffer[current++];
				}
				smsframe[PHONE_SMSSubmit.TPPID] = buffer[current++];
				smsframe[PHONE_SMSSubmit.TPDCS] = buffer[current++];
				/* See GSM 03.40 9.2.3.3 - TPVP can not exist in frame */
				//if ((smsframe[12] & 0x18)!=0) current++; //TPVP is ignored now
				if ((smsframe[12] & 0x18)==0x18) current += 7; //TPVP is ignored now
				else if ((smsframe[12] & 0x10)==0x10) current++; //TPVP is ignored now//090715libaoliu
				smsframe[PHONE_SMSSubmit.TPUDL] = buffer[current++];
				for(i=0;i<smsframe[PHONE_SMSSubmit.TPUDL];i++) smsframe[i+PHONE_SMSSubmit.Text]=buffer[current++];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSSubmit);
				return ERR_NONE;
			case 0x02:
				smprintf(theApp.m_pDebuginfo, "SMS type - status report\n");
				sms->PDU 	 = SMS_Status_Report;
				sms->Folder 	 = 1;	/*INBOX SIM*/
				sms->InboxFolder = true;
				smprintf(theApp.m_pDebuginfo, "TPMR is %02x\n",buffer[current]);
				smsframe[PHONE_SMSStatusReport.TPMR] = buffer[current++];
				current2=((buffer[current])+1)/2+1;
				for(i=0;i<current2+1;i++) smsframe[PHONE_SMSStatusReport.Number+i]=buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.DateTime+i]=buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.SMSCTime+i]=buffer[current++];
				smsframe[PHONE_SMSStatusReport.TPStatus]=buffer[current];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSStatusReport);
				return ERR_NONE;
			}
			break;
		case SMS_AT_TXT:
			current = 0;
			while (msg.Buffer[current]!='"') current++;
			current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
			if (!strcmp((char*)buffer,"\"0\"") || !strcmp((char*)buffer,"\"REC UNREAD\"")) {
				smprintf(theApp.m_pDebuginfo, "SMS type - deliver\n");
				sms->State 	 = SMS_UnRead;
				sms->PDU 	 = SMS_Deliver;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 1; /*INBOX SIM*/ 
				} else {
					sms->Folder = 3; /*INBOX ME*/ 
				}
				sms->InboxFolder = true;
			} else if (!strcmp((char*)buffer,"\"1\"") || !strcmp((char*)buffer,"\"REC READ\"")) {
				smprintf(theApp.m_pDebuginfo, "SMS type - deliver\n");
				sms->State 	 = SMS_Read;
				sms->PDU 	 = SMS_Deliver;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 1; /*INBOX SIM*/ 
				} else {
					sms->Folder = 3; /*INBOX ME*/ 
				}
				sms->InboxFolder = true;
			} else if (!strcmp((char*)buffer,"\"2\"") || !strcmp((char*)buffer,"\"STO UNSENT\"")) {
				smprintf(theApp.m_pDebuginfo, "SMS type - submit\n");
				sms->State 	 = SMS_UnSent;
				sms->PDU 	 = SMS_Submit;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/ 
				} else {
					sms->Folder = 4; /*OUTBOX ME*/ 
				}
				sms->InboxFolder = false;
			} else if (!strcmp((char*)buffer,"\"3\"") || !strcmp((char*)buffer,"\"STO SENT\"")) {
				smprintf(theApp.m_pDebuginfo, "SMS type - submit\n");
				sms->State 	 = SMS_Sent;
				sms->PDU 	 = SMS_Submit;
				if (theApp.m_SMSMemory == MEM_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/ 
				} else {
					sms->Folder = 4; /*OUTBOX ME*/ 
				}
				sms->InboxFolder = false;
			}
			current += ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
			/* It's delivery report according to Nokia AT standards */
			if (sms->Folder==1 && buffer[0]!=0 && buffer[0]!='"') {
				/* ??? */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* format of sender number */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* Sender number */
				/* FIXME: support for all formats */
				EncodeUnicode(sms->Number,buffer+1,strlen((char*)buffer)-2);
				smprintf(theApp.m_pDebuginfo, "Sender \"%s\"\n",DecodeUnicodeString(sms->Number));
				/* ??? */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* Sending datetime */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				i = strlen((char*)buffer);
				buffer[i] = ',';
				i++;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
				smprintf(theApp.m_pDebuginfo, "\"%s\"\n",buffer);
				ATGEN_DecodeDateTime(&sms->DateTime, buffer+1);
				/* Date of SMSC response */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				i = strlen((char*)buffer);
				buffer[i] = ',';
				i++;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
				smprintf(theApp.m_pDebuginfo, "\"%s\"\n",buffer);
				ATGEN_DecodeDateTime(&sms->SMSCTime, buffer+1);
				/* TPStatus */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPStatus=atoi((char*)buffer);
				buffer[PHONE_SMSStatusReport.TPStatus] = TPStatus;
				error=GSM_DecodeSMSFrameStatusReportData(sms, buffer, PHONE_SMSStatusReport);
				if (error!=ERR_NONE) return error;
				/* NO SMSC number */
				sms->SMSC.Number[0]=0;
				sms->SMSC.Number[1]=0;
				sms->PDU = SMS_Status_Report;
				sms->ReplyViaSameSMSC=false;
			} else {
				/* Sender number */
				/* FIXME: support for all formats */
				EncodeUnicode(sms->Number,buffer+1,strlen((char*)buffer)-2);
				/* Sender number in alphanumeric format ? */
				current += ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				if (strlen((char*)buffer)!=0) EncodeUnicode(sms->Number,buffer+1,strlen((char*)buffer)-2);
				smprintf(theApp.m_pDebuginfo, "Sender \"%s\"\n",DecodeUnicodeString(sms->Number));
				/* Sending datetime */
				if (sms->Folder==1 || sms->Folder==3) {
					current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
					/* FIXME: ATGEN_ExtractOneParameter() is broken as it doesn't respect
					 * quoting of parameters and thus +FOO: "ab","cd,ef" will consider
					 * as three arguments: "ab" >> "cd >> ef"
					 */
					if (*buffer=='"') {
						i = strlen((char*)buffer);
						buffer[i] = ',';
						i++;
						current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
					}
					smprintf(theApp.m_pDebuginfo, "\"%s\"\n",buffer);
					if (*buffer)
						ATGEN_DecodeDateTime(&sms->DateTime, buffer+1);
					else {
						/* FIXME: What is the proper undefined GSM_DateTime ? */
						memset(&sms->DateTime, 0, sizeof(sms->DateTime));
					}
					ATGEN_DecodeDateTime(&sms->DateTime, buffer+1);
				}
				/* Sender number format */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* First byte */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				firstbyte=atoi((char*)buffer);
				sms->ReplyViaSameSMSC=false;
				/* GSM 03.40 section 9.2.3.17 (TP-Reply-Path) */
				if ((firstbyte & 128)==128) sms->ReplyViaSameSMSC=true;
				/* TP PID */        
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				sms->ReplaceMessage = 0;
				if (atoi((char*)buffer) > 0x40 && atoi((char*)buffer) < 0x48) {
					sms->ReplaceMessage = atoi((char*)buffer) - 0x40;
				}
				smprintf(theApp.m_pDebuginfo, "TPPID: %02x %i\n",atoi((char*)buffer),atoi((char*)buffer));
				/* TP DCS */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPDCS=atoi((char*)buffer);
				if (sms->Folder==2 || sms->Folder==4) 
				{
					/*TP VP */
					current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				}
				/* SMSC number */
				/* FIXME: support for all formats */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				EncodeUnicode(sms->SMSC.Number,buffer+1,strlen((char*)buffer)-2);
				/* Format of SMSC number */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* TPUDL */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPUDL=atoi((char*)buffer);
				current++;
				sms->Coding = SMS_Coding_Default;
				/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme)
				 * and GSM 03.38 section 4
				 */
				if ((TPDCS & 0x04) == 0x04) sms->Coding=SMS_Coding_8bit;
				if ((TPDCS & 0x08) == 0x08) sms->Coding=SMS_Coding_Unicode;
				sms->Class = -1;
				if ((TPDCS & 0xF3)==0xF0) sms->Class = 0;
				if ((TPDCS & 0xF3)==0xF1) sms->Class = 1;
				if ((TPDCS & 0xF3)==0xF2) sms->Class = 2;
				if ((TPDCS & 0xF3)==0xF3) sms->Class = 3;
				smprintf(theApp.m_pDebuginfo, "SMS class: %i\n",sms->Class);
				switch (sms->Coding) {          
				case SMS_Coding_Default:
					/* GSM 03.40 section 9.2.3.23 (TP-User-Data-Header-Indicator) */
					/* If not SMS with UDH, it's coded normal */
					/* If UDH available, treat it as Unicode or 8 bit */
					if ((firstbyte & 0x40)!=0x40) {
						sms->UDH.Type	= UDH_NoUDH;
						sms->Length	= TPUDL;
						EncodeUnicode(sms->Text,msg.Buffer+theApp.m_Lines.numbers[2*2],TPUDL);
						break;
					}
				case SMS_Coding_Unicode:
				case SMS_Coding_8bit:
					DecodeHexBin(buffer+PHONE_SMSDeliver.Text, msg.Buffer+current, TPUDL*2);
					buffer[PHONE_SMSDeliver.firstbyte] 	= firstbyte;
					buffer[PHONE_SMSDeliver.TPDCS] 		= TPDCS;
					buffer[PHONE_SMSDeliver.TPUDL] 		= TPUDL;
					return GSM_DecodeSMSFrameText(sms, buffer, PHONE_SMSDeliver);
				}
			}
			return ERR_NONE;
		default:
			break;
		}
		break;
	case AT_Reply_CMSError:
		if (Replynfo.ErrorCode == 320 || Replynfo.ErrorCode == 500) {
			return ERR_EMPTY;
		} else {
			return ATGEN_HandleCMSError(Replynfo);
		}
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(Replynfo);
	case AT_Reply_Error:
		/* A2D returns Error with empty location */
		return ERR_EMPTY;
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}


GSM_Error ATGEN_ReplyGetSMSStatus(GSM_Protocol_Message msg)
{
//	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_SMSMemoryStatus	*SMSStatus = theApp.m_SMSStatus;
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	char 			*start;
	int			current = 0;
	unsigned char		buffer[50];
	int num;

	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "SMS status received\n");

//		start = strstr((char*)msg.Buffer, "+CPMS: ") + 7;
		num=Searchstr(msg.Buffer,(unsigned char *)"+CPMS: ",7);
		if(num)
			start=(char*)msg.Buffer + num+6;
		else
		{
			num=Searchstr(msg.Buffer,(unsigned char *)"+CPMS:",6);
			start=(char*)msg.Buffer + num+5;
		}
		if (strstr((char*)msg.Buffer,"ME")!=NULL) {
			SMSStatus->PhoneUsed 	= atoi(start);
			current+=ATGEN_ExtractOneParameter((unsigned char *)start+current, buffer);
			current+=ATGEN_ExtractOneParameter((unsigned char *)start+current, buffer);
			SMSStatus->PhoneSize	= atoi((char*)buffer);
			smprintf(theApp.m_pDebuginfo, "Used : %i\n",SMSStatus->PhoneUsed);
			smprintf(theApp.m_pDebuginfo, "Size : %i\n",SMSStatus->PhoneSize);
		}
		else if (strstr((char*)msg.Buffer,"MT")!=NULL) 
		{
			SMSStatus->PhoneUsed 	= atoi(start);
			current+=ATGEN_ExtractOneParameter((unsigned char *)start+current, buffer);
			current+=ATGEN_ExtractOneParameter((unsigned char *)start+current, buffer);
			SMSStatus->PhoneSize	= atoi((char*)buffer);
			smprintf(theApp.m_pDebuginfo, "Used : %i\n",SMSStatus->PhoneUsed);
			smprintf(theApp.m_pDebuginfo, "Size : %i\n",SMSStatus->PhoneSize);
		} 
		else 
		{
			SMSStatus->SIMUsed 	= atoi(start);
			current+=ATGEN_ExtractOneParameter((unsigned char *)start+current, buffer);
			current+=ATGEN_ExtractOneParameter((unsigned char *)start+current, buffer);
			SMSStatus->SIMSize	= atoi((char*)buffer);
			smprintf(theApp.m_pDebuginfo, "Used : %i\n",SMSStatus->SIMUsed);
			smprintf(theApp.m_pDebuginfo, "Size : %i\n",SMSStatus->SIMSize);
			if (SMSStatus->SIMSize == 0) 
			{
				smprintf(theApp.m_pDebuginfo, "Can't access SIM card\n");
				return ERR_SECURITYERROR;
			}
		}
		return ERR_NONE;
	case AT_Reply_Error:
		if (strstr((char*)msg.Buffer,"SM")!=NULL) {
			smprintf(theApp.m_pDebuginfo, "Can't access SIM card\n");
			return ERR_SECURITYERROR;
		}
		return ERR_NOTSUPPORTED;
 	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetSMSMemories(GSM_SMSMemoryInfo* pSMSmemoryinfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	GSM_Error		error;
//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;
	smprintf(theApp.m_pDebuginfo, "Getting available SMS memories\n");
//	return GSM_WaitFor (s, "AT+CPMS=?\r", 10, 0x00, 4, ID_GetSMSMemories);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPMS=?");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"AT+CPMS=?\r", 10, 0x00, 8,false,NULL,&ReplymsgType,ATGEN_ReplyGetSMSMemories);
	if(pSMSmemoryinfo)
	{
		pSMSmemoryinfo->m_CanSaveSMS = theApp.m_CanSaveSMS;
		pSMSmemoryinfo->m_PhoneSMSMemory = theApp.m_PhoneSMSMemory;
		pSMSmemoryinfo->m_SIMSMSMemory = theApp.m_SIMSMSMemory;
	}
	return error;
}

GSM_Error ATGEN_SetSMSMemory(bool SIM,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
//	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	char 			req[] = "AT+CPMS=\"XX\",\"XX\"\r";
	int			reqlen = 18;
	GSM_Error		error;
	theApp.m_pDebuginfo = pDebuginfo;
	
//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;
	if ((SIM && theApp.m_SIMSMSMemory == 0) || (!SIM && theApp.m_PhoneSMSMemory == 0)) {
		/* We silently ignore error here, because when this fails, we can try to setmemory anyway */
		ATGEN_GetSMSMemories(NULL,pWriteCommandfn,pDebuginfo);
	}
	
	/* If phone can not save SMS, don't try to set memory for saving */
	if (!theApp.m_CanSaveSMS) {
		req[12] = '\r';
		reqlen = 13;
	}
	
	if (SIM) {
		if (theApp.m_SMSMemory == MEM_SM) return ERR_NONE;
		if (theApp.m_SIMSMSMemory == AT_NOTAVAILABLE) return ERR_NOTSUPPORTED;

		req[9]  = 'S'; req[10] = 'M';
		req[14] = 'S'; req[15] = 'M';
		
		smprintf(theApp.m_pDebuginfo, "Setting SMS memory type to SM\n");
	//	error=GSM_WaitFor (s, req, reqlen, 0x00, 3, ID_SetMemoryType);
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPMS");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		error= pWriteCommandfn ((unsigned char *)req, reqlen, 0x00, 6,false,NULL,&ReplymsgType,ATGEN_GenericReply);
	
		if (theApp.m_SIMSMSMemory == 0 && error == ERR_NONE) {
			theApp.m_SIMSMSMemory = AT_AVAILABLE;
		}
		if (error == ERR_NOTSUPPORTED) {
			smprintf(theApp.m_pDebuginfo, "Can't access SIM card?\n");
			return ERR_SECURITYERROR;
		}
		if (error != ERR_NONE) return error;
		theApp.m_SMSMemory = MEM_SM;
	} else {
		if (theApp.m_SMSMemory == MEM_ME) return ERR_NONE;
		if (theApp.m_PhoneSMSMemory == AT_NOTAVAILABLE) return ERR_NOTSUPPORTED;
		
		req[9]  = 'M'; req[10] = 'E';
		req[14] = 'M'; req[15] = 'E';

		smprintf(theApp.m_pDebuginfo, "Setting SMS memory type to ME\n");
	//	error=GSM_WaitFor (s, req, reqlen, 0x00, 3, ID_SetMemoryType);
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPMS");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		error= pWriteCommandfn ((unsigned char *)req, reqlen, 0x00, 6,false,NULL,&ReplymsgType,ATGEN_GenericReply);
		if(error != ERR_NONE)
		{
		//	error = GSM_WaitFor (s, "AT+CPMS=\"MT\"\r", 13, 0x00, 4, ID_SetMemoryType);
			ReplymsgType.nCount = 1;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPMS");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;
			error= pWriteCommandfn ((unsigned char *)"AT+CPMS=\"MT\"\r", 13, 0x00, 8,false,NULL,&ReplymsgType,ATGEN_GenericReply);
		}

		if (theApp.m_PhoneSMSMemory == 0 && error == ERR_NONE) {
			theApp.m_PhoneSMSMemory = AT_AVAILABLE;
		}
		if (error == ERR_NONE) theApp.m_SMSMemory = MEM_ME;
	}
	return error;
}

GSM_Error NXP_ReplyGetIndexFileInfo(GSM_Protocol_Message msg)
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
	ATGEN_GetReplyStatue(&tempmsg,&Replynfo);
	//MTK_GetReplyStatue(&tempmsg,&Replynfo);
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

	//MTK_GetReplyStatue(&tempmsg,&Replynfo);
	ATGEN_GetReplyStatue(&tempmsg,&Replynfo);
	
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
GSM_Error NXP_ReplyGetVer(GSM_Protocol_Message msg)
{
	char			*pos;
	unsigned char		buffer[30];
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);
	switch (Replynfo.ReplyState) 
	{
		case AT_Reply_OK:
		pos = (char*)msg.Buffer;
	
			pos = strstr((char*)pos, "*GETVER:");
			if (pos == NULL)//return ERR_UNKNOWN;//peggy
			{
				if(Replynfo.ReplyState == AT_Reply_Continue)
				{
					theApp.isbNewXP=false;
					return ERR_UNKNOWN;
				}
				else 
				{
					theApp.isbNewXP=false;
					return ERR_NONE; 
				}
			}
			pos=pos+8;
		    pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
			char *pdest=NULL;
			pdest=strrchr((char *)buffer,'-');//091001linaoliu
			int i=atoi(pdest+1);
			if (i>=20/*!strcmp((char *)buffer,"XP3-P1.0-DV1715-R02-GE-20")*/)
			{
				theApp.isbNewXP=true;
			}
			else
				theApp.isbNewXP=false;
	
		return ERR_NONE;			
		break;
	}

}
GSM_Error ATGEN_GetSMSStatus( GSM_SMSMemoryStatus *status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error 		error;

	/* No templates at all */
	status->TemplatesUsed	= 0;

	status->SIMUsed		= 0;
	status->SIMUnRead 	= 0;
	status->SIMSize		= 0;

	theApp.m_SMSStatus=status;
	
	if ((theApp.m_SIMSMSMemory == 0) || (theApp.m_PhoneSMSMemory == 0))
	{
		/* We silently ignore error here, because when this fails, we can try to setmemory anyway */
		ATGEN_GetSMSMemories(NULL,pWriteCommandfn,pDebuginfo);
	}
	
	if (theApp.m_PhoneSMSMemory == 0) 
	{
		error = ATGEN_SetSMSMemory(false,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (theApp.m_SIMSMSMemory == 0) 
	{
		error = ATGEN_SetSMSMemory(true,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}

	if (theApp.m_SIMSMSMemory == AT_AVAILABLE) 
	{
		smprintf(theApp.m_pDebuginfo, "Getting SIM SMS status\n");
		if (theApp.m_CanSaveSMS) 
		{
//			error=GSM_WaitFor (s, "AT+CPMS=\"SM\",\"SM\"\r", 18, 0x00, 4, ID_GetSMSStatus);
			ReplymsgType.nCount = 1;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPMS");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;
			error= pWriteCommandfn ((unsigned char *)"AT+CPMS=\"SM\",\"SM\"\r", 18, 0x00, 8,false,NULL,&ReplymsgType,ATGEN_ReplyGetSMSStatus);
		} 
		else 
		{
		//	error=GSM_WaitFor (s, "AT+CPMS=\"SM\"\r", 13, 0x00, 4, ID_GetSMSStatus);
			ReplymsgType.nCount = 1;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPMS");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;
			error= pWriteCommandfn ((unsigned char *)"AT+CPMS=\"SM\"\r", 13, 0x00, 8,false,NULL,&ReplymsgType,ATGEN_ReplyGetSMSStatus);
		}
		if (error!=ERR_NONE) return error;
		theApp.m_SMSMemory = MEM_SM;
	}

	status->PhoneUsed	= 0;
	status->PhoneUnRead 	= 0;
	status->PhoneSize	= 0;
	
	if (theApp.m_PhoneSMSMemory == AT_AVAILABLE) 
	{
		smprintf(theApp.m_pDebuginfo, "Getting phone SMS status\n");
		if (theApp.m_CanSaveSMS) 
		{
//			error = GSM_WaitFor (s, "AT+CPMS=\"ME\",\"ME\"\r", 18, 0x00, 4, ID_GetSMSStatus);
			ReplymsgType.nCount = 1;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPMS");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;
			error= pWriteCommandfn ((unsigned char *)"AT+CPMS=\"ME\",\"ME\"\r", 18, 0x00, 8,false,NULL,&ReplymsgType,ATGEN_ReplyGetSMSStatus);
		}
		else 
		{
		//	error = GSM_WaitFor (s, "AT+CPMS=\"ME\"\r", 13, 0x00, 4, ID_GetSMSStatus);
			ReplymsgType.nCount = 1;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPMS");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;
			error= pWriteCommandfn ((unsigned char *)"AT+CPMS=\"ME\"\r", 13, 0x00, 8,false,NULL,&ReplymsgType,ATGEN_ReplyGetSMSStatus);
		}
		if (error!=ERR_NONE)
		{
			ReplymsgType.nCount = 1;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPMS");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;
			error= pWriteCommandfn ((unsigned char *)"AT+CPMS=\"MT\"\r", 13, 0x00, 8,false,NULL,&ReplymsgType,ATGEN_ReplyGetSMSStatus);
		}

		if (error!=ERR_NONE) return error;
		theApp.m_SMSMemory = MEM_ME;
	}
	theApp.m_SIMSize = status->SIMSize;
	//0805006libaoliu
	/*GSM_Error error;*/
	//090714libaoliu
	
	char			req[50];
	sprintf(req, "AT*GETVER\r");
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*GETVER");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*GETVER:");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error =pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,NXP_ReplyGetVer);
	if (error != ERR_NONE) return error;



	//theApp.m_MemoryStatus=Status;
	sprintf(req, "AT*STARTUL=%s\r", "/app/sms/smstab");
	theApp.isFirst=false;
	theApp.isGetZero=false;
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*STARTUL=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*STARTUL");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,NXP_ReplyGetIndexFileInfo);
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
			error =pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,NXP_ReplyGetDataMemory);
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
	error = pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
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
	file = fopen(theApp.m_szPHKIndexFileTempFile,"r+b");
	if(file)
	{
			
		IndexFileList.RemoveAll();
		unsigned char RecordUsed[1000];
	   	fseek(file,22000,SEEK_SET);
		fread(RecordUsed,1,1000,file);
		for (int i=0;i<1000;i++)
		{
		if(RecordUsed[i]==0xff)
		{
		IndexFileList.AddTail(i);
		}
		}
		fclose(file);
	
	}
	if (encodebuffer)
	{
	delete encodebuffer;
	}
	if (theApp.m_unDecodeBase64)
	{
		delete theApp.m_unDecodeBase64;
		theApp.m_unDecodeBase64=NULL;
	}
	bool bIsEmpty=false;
	remove(theApp.m_szSMSDataFileTempFile);
	for (int j=0;j<50;j++)
	{
		if (j<10)
		{
			sprintf(req, "AT*STARTUL=%s%d\r", "/app/sms/dt/sms000",j);
			
		}
		else
		{
			sprintf(req, "AT*STARTUL=%s%d\r", "/app/sms/dt/sms00",j);
		}
		ReplymsgType.nCount = 2;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*STARTUL=");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"*STARTUL");
		ReplymsgType.CheckInfo[1].subtypechar = 0;
		ReplymsgType.CheckInfo[1].subtype = 0x00;
		error =pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,NXP_ReplyGetIndexFileInfo);
		if (error != ERR_NONE&& error != ERR_EMPTY)
		{
			
			bIsEmpty=TRUE;
			break;/*return error;*/
			
			/*break;*/
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
				error =pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,NXP_ReplyGetDataFileMemory);
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
			error =pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,NULL,&ReplymsgType,ATGEN_GenericReply);
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
		if (file)
		{

			if (theApp.isbNewXP)
			{
			t_NewEMSRecord SmsRecord;
			ZeroMemory(&SmsRecord,sizeof(t_NewEMSRecord));
			POSITION pos=IndexFileList.GetHeadPosition();
			for (int i=0;i<IndexFileList.GetCount();i++)
			{
				int CurrentIndex=IndexFileList.GetNext(pos);
				if(CurrentIndex/20==j)
				{
				fseek(file,sizeof(t_NewEMSRecord)*((CurrentIndex%20)),SEEK_SET);
				fread(&SmsRecord,1,sizeof(t_NewEMSRecord),file);
				FILE * Datafile = fopen(theApp.m_szSMSDataFileTempFile,"a+b");
				if(Datafile)
				{
				fwrite(&SmsRecord,1,sizeof(t_NewEMSRecord),Datafile);
				fclose(Datafile);
				}
				}//090926libaoliu
			}
			}
			else
			{
			t_EMSRecord SmsRecord;
			ZeroMemory(&SmsRecord,sizeof(t_EMSRecord));
			
			POSITION pos=IndexFileList.GetHeadPosition();
			for (int i=0;i<IndexFileList.GetCount();i++)
			{
				int CurrentIndex=IndexFileList.GetNext(pos);
				if(CurrentIndex/20==j)
				{
				fseek(file,sizeof(t_EMSRecord)*((CurrentIndex%20)),SEEK_SET);
				fread(&SmsRecord,1,sizeof(t_EMSRecord),file);
				FILE * Datafile = fopen(theApp.m_szSMSDataFileTempFile,"a+b");
				if(Datafile)
				{
				fwrite(&SmsRecord,1,sizeof(t_EMSRecord),Datafile);
				fclose(Datafile);
				}
				}//090926libaoliu
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
     status->PhoneSize=1000;
	 status->PhoneUsed=IndexFileList.GetCount();

	if(theApp.m_pMobileInfo)
	{
		if(IsPhoneFeatureAvailable(theApp.m_pMobileInfo, F_SMS_SETMAXMEINDEX))
		{
			theApp.m_MAXSMSINFOLDER = status->PhoneSize;
		}
	}
	theApp.m_PhoneSize= status->PhoneSize;
	return ERR_NONE;
}
GSM_Error ATGEN_GetSMSLocation(GSM_SMSMessage *sms, unsigned char *folderid, int *location,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
//	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	int			ifolderid, maxfolder;
	GSM_Error		error;

//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;
	if (theApp.m_PhoneSMSMemory == 0) 
	{
		error = ATGEN_SetSMSMemory(false,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (theApp.m_SIMSMSMemory == 0) 
	{
		error = ATGEN_SetSMSMemory(true,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	
	if (theApp.m_SIMSMSMemory != AT_AVAILABLE && theApp.m_PhoneSMSMemory != AT_AVAILABLE)
	{
		/* No SMS memory at all */
		return ERR_NOTSUPPORTED;
	}
	if (theApp.m_SIMSMSMemory == AT_AVAILABLE && theApp.m_PhoneSMSMemory == AT_AVAILABLE) 
	{
		/* Both available */
		maxfolder = 2;
	} 
	else 
	{
		/* One available */
		maxfolder = 1;
	}

	/* simulate flat SMS memory */
	if (sms->Folder == 0x00) 
	{
		ifolderid = (sms->Location-1) / theApp.m_MAXSMSINFOLDER ;//PHONE_MAXSMSINFOLDER;
		if (ifolderid + 1 > maxfolder) return ERR_NOTSUPPORTED;
		*folderid = ifolderid + 1;
		*location = sms->Location - ifolderid * theApp.m_MAXSMSINFOLDER ;//PHONE_MAXSMSINFOLDER;
	} 
	else
	{
		if (sms->Folder > 2 * maxfolder) return ERR_NOTSUPPORTED;
		*folderid = sms->Folder <= 2 ? 1 : 2;
		*location = sms->Location;
	}

	  if (( sms->Folder == 1 ) && ( sms->Location > theApp.m_MAXSMSINFOLDER /*PHONE_MAXSMSINFOLDER*/ ) )
	  {
		  ifolderid = (sms->Location-1) / theApp.m_MAXSMSINFOLDER /*PHONE_MAXSMSINFOLDER*/;
		  *folderid = ifolderid + 0x01;
		  *location = sms->Location - ifolderid * theApp.m_MAXSMSINFOLDER/*PHONE_MAXSMSINFOLDER*/;
	  }

	
	smprintf(theApp.m_pDebuginfo, "SMS folder %i & location %i -> ATGEN folder %i & location %i\n",
		sms->Folder,sms->Location,*folderid,*location);

	if (theApp.m_SIMSMSMemory == AT_AVAILABLE && *folderid == 1) 
	{
		return ATGEN_SetSMSMemory(true,pWriteCommandfn,pDebuginfo);
	} 
	else 
	{
		return ATGEN_SetSMSMemory(false,pWriteCommandfn,pDebuginfo);
	}
}

GSM_Error ATGEN_GetSMSMode(GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	theApp.m_pDebuginfo = pDebuginfo;
//	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
  	GSM_Error 		error;

	// added by mingfa for test
//	MessageBox( NULL, "ATGEN_GetSMSMode AT+CMGF=0", "ATGEN_GetSMSMode", MB_OK);// for testing 

	if (theApp.m_SMSMode != SMS_AT_ERROR) return ERR_NONE;
//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;

	smprintf(theApp.m_pDebuginfo, "Trying SMS PDU mode\n");
//	error=GSM_WaitFor (s, "AT+CMGF=0\r", 10, 0x00, 3, ID_GetSMSMode);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CMGF");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error= pWriteCommandfn ((unsigned char *)"AT+CMGF=0\r", 10, 0x00, 6,false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error==ERR_NONE) 
	{
		theApp.m_SMSMode = SMS_AT_PDU;
		return ERR_NONE;
	}
    // 0810 added by mingfa for SF only
    //theApp.m_SMSMode = SMS_AT_PDU;
    //return ERR_NONE;
	// end mingfa

	smprintf(theApp.m_pDebuginfo, "Trying SMS text mode\n");
//	error=GSM_WaitFor (s, "AT+CMGF=1\r", 10, 0x00, 3, ID_GetSMSMode);
	error= pWriteCommandfn ((unsigned char *)"AT+CMGF=1\r", 10, 0x00, 6,false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error==ERR_NONE) 
	{
		smprintf(theApp.m_pDebuginfo, "Enabling displaying all parameters in text mode\n");
	//	error=GSM_WaitFor (s, "AT+CSDH=1\r", 10, 0x00, 3, ID_GetSMSMode);
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CSDH");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		error= pWriteCommandfn ((unsigned char *)"AT+CSDH=1\r", 10, 0x00, 6,false,NULL,&ReplymsgType,ATGEN_GenericReply);
		if (error == ERR_NONE) theApp.m_SMSMode = SMS_AT_TXT;
	}

	return error;
}
void ATGEN_SetSMSLocation(GSM_SMSMessage *sms, unsigned char folderid, int location)
{
	sms->Folder	= 0;
	sms->Location	= (folderid - 1) * theApp.m_MAXSMSINFOLDER /*PHONE_MAXSMSINFOLDER */+ location;
	smprintf(theApp.m_pDebuginfo, "ATGEN folder %i & location %i -> SMS folder %i & location %i\n",
		folderid,location,sms->Folder,sms->Location);
}

GSM_Error ATGEN_GetSMS(OnePhoneModel *pMobileInfo, GSM_MultiSMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	unsigned char	req[20], folderid;
	GSM_Error	error;
	int		location, getfolder, add = 0;
	if(IsPhoneFeatureAvailable(pMobileInfo, F_SMS_SETMAXMEINDEX))
		theApp.m_MAXSMSINFOLDER = theApp.m_PhoneSize;

//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;
	if (theApp.m_ManufacturerID == AT_Unknown)
	{
		char szGetManufacturer[MAX_PATH];
		error=ATGEN_GetManufacturer(szGetManufacturer,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;
	}

	error=ATGEN_GetSMSLocation(&sms->SMS[0], &folderid, &location,pWriteCommandfn,pDebuginfo);
	if (error!=ERR_NONE) return error;
	if (theApp.m_SMSMemory == MEM_ME && IsPhoneFeatureAvailable(pMobileInfo, F_SMSME900)) add = 899;
	sprintf((char*)req, "AT+CMGR=%i\r", location + add);

	error=ATGEN_GetSMSMode(pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

//	error=ATGEN_GetManufacturer(s);
//	char szGetManufacturer[MAX_PATH];
//	error=ATGEN_GetManufacturer(szGetManufacturer,pWriteCommandfn,pDebuginfo);
//	if (error != ERR_NONE) return error;

	theApp.m_GetSMSMessage=sms;
	smprintf(theApp.m_pDebuginfo, "Getting SMS\n");
//	error=GSM_WaitFor (s, req, strlen(req), 0x00, 5, ID_GetSMSMessage);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CMGR");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	theApp.m_pMobileInfo = pMobileInfo;
	error= pWriteCommandfn (req, strlen((char*)req), 0x00, 10,false,NULL,&ReplymsgType,ATGEN_ReplyGetSMSMessage);

	if (error==ERR_NONE) 
	{
		getfolder = sms->SMS[0].Folder;
//		if (getfolder != 0 && getfolder != sms->SMS[0].Folder) return ERR_EMPTY;
		ATGEN_SetSMSLocation(&sms->SMS[0], folderid, location);
		sms->SMS[0].Folder = getfolder;
		sms->SMS[0].Memory = MEM_SM;
		if (getfolder > 2) sms->SMS[0].Memory = MEM_ME;
	}
	return error;
}
GSM_Error ATGEN_GetNextSMS(OnePhoneModel *pMobileInfo, GSM_MultiSMSMessage *sms, bool start,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
//	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error 		error;
	int			usedsms;
//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;

	if (theApp.m_PhoneSMSMemory == 0) 
	{
		error = ATGEN_SetSMSMemory(false,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (theApp.m_SIMSMSMemory == 0) 
	{
		error = ATGEN_SetSMSMemory(true,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (theApp.m_SIMSMSMemory == AT_NOTAVAILABLE && theApp.m_PhoneSMSMemory == AT_NOTAVAILABLE) 
		return ERR_NOTSUPPORTED;

	if (start) 
	{
		error=ATGEN_GetSMSStatus(&theApp.m_LastSMSStatus,pWriteCommandfn,pDebuginfo);
		if (error!=ERR_NONE) return error;
		theApp.m_LastSMSRead		= 0;
		sms->SMS[0].Location 		= 0;
		theApp.isMEsms =false;
	}
	while (true) 
	{
		sms->SMS[0].Location++;
		if (sms->SMS[0].Location < theApp.m_MAXSMSINFOLDER /*PHONE_MAXSMSINFOLDER*/) 
		{
			if (theApp.m_SIMSMSMemory == AT_AVAILABLE) 
			{
				usedsms = theApp.m_LastSMSStatus.SIMUsed;
			} 
			else 
			{
				usedsms = theApp.m_LastSMSStatus.PhoneUsed;
			}
					
			if (theApp.m_LastSMSRead >= usedsms) 
			{
				if (/*theApp.m_PhoneSMSMemory == AT_NOTAVAILABLE ||*/ theApp.m_LastSMSStatus.PhoneUsed==0) return ERR_EMPTY;
				if (!theApp.isMEsms)
				{
					theApp.m_LastSMSRead	= 0;
				}
				sms->SMS[0].Location 	= theApp.m_MAXSMSINFOLDER/*PHONE_MAXSMSINFOLDER*/ + 1;
				theApp.isMEsms =true;
			}
		}
		else 
		{
		/*	if (theApp.m_PhoneSMSMemory == AT_NOTAVAILABLE) return ERR_EMPTY;*/
			if (theApp.m_LastSMSRead>=theApp.m_LastSMSStatus.PhoneUsed) return ERR_EMPTY;
		}
		if (/*theApp.m_LastSMSRead==0&&*/theApp.isMEsms)
		{
			if (theApp.isbNewXP)
			{
			FILE * file;
			file = fopen(theApp.m_szSMSDataFileTempFile,"r+b");
			t_NewEMSRecord SmsRecord;
			ZeroMemory(&SmsRecord,sizeof( t_NewEMSRecord));
			if(file)
			{
				int d=sizeof( t_NewEMSRecord);
				fseek(file,sizeof( t_NewEMSRecord)*(theApp.m_LastSMSRead),SEEK_SET);
				fread(&SmsRecord,1,sizeof( t_NewEMSRecord),file);
				fclose(file);
			}
			sms->Number=1;
			sms->SMS[0].Location=IndexFileList.GetAt(IndexFileList.FindIndex(theApp.m_LastSMSRead));
			if ((SmsRecord.v_UserInfo.v_MessageStatus==1)||(SmsRecord.v_UserInfo.v_MessageStatus==3))
			{
			sms->SMS[0].Folder=1;
			sms->SMS[0].InboxFolder=true;//080926libaoliu
			}
			else
			{
			sms->SMS[0].Folder=0;
			sms->SMS[0].InboxFolder=FALSE;
			}
			//sms->SMS[0].InboxFolder=true;
			sms->SMS[0].Memory=MEM_ME;
			sms->SMS[0].State=SMS_Read;
			sms->SMS[0].Coding=SMS_Coding_Unicode;
			sms->SMS[0].DateTime.Year=(SmsRecord.v_UserInfo.v_time_info & 0XFFF00000)>>20;
			sms->SMS[0].DateTime.Month=(SmsRecord.v_UserInfo.v_time_info & 0X000F0000)>>16;
			sms->SMS[0].DateTime.Day=(SmsRecord.v_UserInfo.v_time_info & 0X0000FE00)>>11;
			sms->SMS[0].DateTime.Hour=(SmsRecord.v_UserInfo.v_time_info & 0X000007F0)>>6;
			sms->SMS[0].DateTime.Minute=SmsRecord.v_UserInfo.v_time_info & 0X0000003F;
			sms->SMS[0].DateTime.Second=0;
		//	EncodeUnicode(sms->SMS[0].Text,(unsigned char*)SmsRecord.v_UserInfo.a_UserData,strlen(SmsRecord.v_UserInfo.a_UserData));
			DecodeUTF8ToUnicode(sms->SMS[0].Text,(unsigned char*)SmsRecord.v_UserInfo.a_UserData,strlen(SmsRecord.v_UserInfo.a_UserData));//080527libaoliu
			EncodeUnicode(sms->SMS[0].Number,(unsigned char*)SmsRecord.v_UserInfo.a_PhoneNumber,strlen(SmsRecord.v_UserInfo.a_PhoneNumber));
			theApp.m_LastSMSRead++;

			error=ERR_NONE;
			break;
			}
			else
			{
			FILE * file;
			file = fopen(theApp.m_szSMSDataFileTempFile,"r+b");
			t_EMSRecord SmsRecord;
			ZeroMemory(&SmsRecord,sizeof( t_EMSRecord));
			if(file)
			{
				int d=sizeof( t_EMSRecord);
				fseek(file,sizeof( t_EMSRecord)*(theApp.m_LastSMSRead),SEEK_SET);
				fread(&SmsRecord,1,sizeof( t_EMSRecord),file);
				fclose(file);
			}
			sms->Number=1;
			sms->SMS[0].Location=IndexFileList.GetAt(IndexFileList.FindIndex(theApp.m_LastSMSRead));
			if ((SmsRecord.v_UserInfo.v_MessageStatus==1)||(SmsRecord.v_UserInfo.v_MessageStatus==3))
			{
			sms->SMS[0].Folder=1;
			sms->SMS[0].InboxFolder=true;//080926libaoliu
			}
			else
			{
			sms->SMS[0].Folder=0;
			sms->SMS[0].InboxFolder=FALSE;
			}
			//sms->SMS[0].InboxFolder=true;
			sms->SMS[0].Memory=MEM_ME;
			sms->SMS[0].State=SMS_Read;
			sms->SMS[0].Coding=SMS_Coding_Unicode;
			sms->SMS[0].DateTime.Year=(SmsRecord.v_UserInfo.v_time_info & 0XFFF00000)>>20;
			sms->SMS[0].DateTime.Month=(SmsRecord.v_UserInfo.v_time_info & 0X000F0000)>>16;
			sms->SMS[0].DateTime.Day=(SmsRecord.v_UserInfo.v_time_info & 0X0000FE00)>>11;
			sms->SMS[0].DateTime.Hour=(SmsRecord.v_UserInfo.v_time_info & 0X000007F0)>>6;
			sms->SMS[0].DateTime.Minute=SmsRecord.v_UserInfo.v_time_info & 0X0000003F;
			sms->SMS[0].DateTime.Second=0;
		//	EncodeUnicode(sms->SMS[0].Text,(unsigned char*)SmsRecord.v_UserInfo.a_UserData,strlen(SmsRecord.v_UserInfo.a_UserData));
			DecodeUTF8ToUnicode(sms->SMS[0].Text,(unsigned char*)SmsRecord.v_UserInfo.a_UserData,strlen(SmsRecord.v_UserInfo.a_UserData));//080527libaoliu
			EncodeUnicode(sms->SMS[0].Number,(unsigned char*)SmsRecord.v_UserInfo.a_PhoneNumber,strlen(SmsRecord.v_UserInfo.a_PhoneNumber));
			theApp.m_LastSMSRead++;

			error=ERR_NONE;
			break;
			}

		}
		else
		{
		sms->SMS[0].Folder = 0;
		error=ATGEN_GetSMS(pMobileInfo,sms,pWriteCommandfn,pDebuginfo);
		if (error==ERR_NONE) 
		{
			theApp.m_LastSMSRead++;
			break;
		}
		if (error != ERR_EMPTY && error != ERR_INVALIDLOCATION) return error;
		}

		
	}

	return error;
}

GSM_Error ATGEN_SetSMSC(GSM_SMSC *smsc,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	unsigned char req[50];

	if (smsc->Location!=1) return ERR_NOTSUPPORTED;
//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;

	sprintf((char*)req, "AT+CSCA=\"%s\"\r",DecodeUnicodeString(smsc->Number));

	smprintf(theApp.m_pDebuginfo, "Setting SMSC\n");
//	return GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_SetSMSC);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CSCA");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return pWriteCommandfn (req, strlen((char*)req), 0x00, 8,false,NULL,&ReplymsgType,ATGEN_GenericReply);
}
GSM_Error ATGEN_GetSMSC(GSM_SMSC *smsc,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;
	if (smsc->Location==0x00 || smsc->Location!=0x01) return ERR_INVALIDLOCATION;
	
	theApp.m_SMSC=smsc;
	smprintf(theApp.m_pDebuginfo, "Getting SMSC\n");
//	return GSM_WaitFor (s, "AT+CSCA?\r", 9, 0x00, 4, ID_GetSMSC);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CSCA?");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return pWriteCommandfn ((unsigned char *)"AT+CSCA?\r", 9, 0x00, 8,false,NULL,&ReplymsgType,ATGEN_ReplyGetSMSC);
}

GSM_Error PHONE_EncodeSMSFrame(GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout, int *length, bool clear
							   ,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	GSM_Error error;

	if (SMS->SMSC.Location!=0) {
		error = ATGEN_GetSMSC(&SMS->SMSC,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;
		SMS->SMSC.Location = 0;
	}
	if (SMS->PDU == SMS_Deliver) {
		if (SMS->SMSC.Number[0] == 0x00 && SMS->SMSC.Number[1] == 0x00) {
			return ERR_EMPTYSMSC;
		}
	}
	if(theApp.m_ManufacturerID == AT_MTK || theApp.m_ManufacturerID == AT_LG)
	{
		Layout.SMSCNumber = 255;
	}
	//0810 added by mingfa for Sangfei SMS bug, m_ManufacturerID error
	Layout.SMSCNumber = 255;

	return GSM_EncodeSMSFrame(SMS, buffer, Layout, length, clear);
}
GSM_Error ATGEN_MakeSMSFrame(GSM_SMSMessage *message, unsigned char *hexreq, int *current, int *length2,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	GSM_Error 		error;
	int			i, length;
	unsigned char		req[1000], buffer[1000];
//	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_SMSC	 	SMSC;

//	error=ATGEN_GetSMSMode(pWriteCommandfn,pDebuginfo);
//	if (error != ERR_NONE) return error;

	length 	 = 0;
	*current = 0;
	switch (theApp.m_SMSMode) 
	{
	case SMS_AT_PDU:
		if (message->PDU == SMS_Deliver)
		{
//		MessageBox( NULL, "message->PDU == SMS_Deliver", "SMS_Deliver", MB_OK);// for testing 


 			smprintf(theApp.m_pDebuginfo, "SMS Deliver\n");
			error=PHONE_EncodeSMSFrame(message,buffer,PHONE_SMSDeliver,&length,true,pWriteCommandfn,pDebuginfo);
			if (error != ERR_NONE) return error;
			length = length - PHONE_SMSDeliver.Text;
			for (i=0;i<buffer[PHONE_SMSDeliver.SMSCNumber]+1;i++) {
				req[(*current)++]=buffer[PHONE_SMSDeliver.SMSCNumber+i];
			}
			req[(*current)++]=buffer[PHONE_SMSDeliver.firstbyte];
			for (i=0;i<((buffer[PHONE_SMSDeliver.Number]+1)/2+1)+1;i++) {
				req[(*current)++]=buffer[PHONE_SMSDeliver.Number+i];
			}
			req[(*current)++]=buffer[PHONE_SMSDeliver.TPPID];
			req[(*current)++]=buffer[PHONE_SMSDeliver.TPDCS];
			for(i=0;i<7;i++) req[(*current)++]=buffer[PHONE_SMSDeliver.DateTime+i];
			req[(*current)++]=buffer[PHONE_SMSDeliver.TPUDL];
			for(i=0;i<length;i++) req[(*current)++]=buffer[PHONE_SMSDeliver.Text+i];
			EncodeHexBin(hexreq, req, *current);
			*length2 = *current * 2;
			*current = *current - (req[PHONE_SMSDeliver.SMSCNumber]+1);
		} 
		else 
		{
//		MessageBox( NULL, "message->PDU != SMS_Deliver", "Not SMS_Deliver", MB_OK);// for testing 


			smprintf(theApp.m_pDebuginfo, "SMS Submit\n");
			error=PHONE_EncodeSMSFrame(message,buffer,PHONE_SMSSubmit,&length,true,pWriteCommandfn,pDebuginfo);
			if (error != ERR_NONE) return error;
			length = length - PHONE_SMSSubmit.Text;
			for (i=0;i<buffer[PHONE_SMSSubmit.SMSCNumber]+1;i++) {
				req[(*current)++]=buffer[PHONE_SMSSubmit.SMSCNumber+i];
			}
			req[(*current)++]=buffer[PHONE_SMSSubmit.firstbyte];
			req[(*current)++]=buffer[PHONE_SMSSubmit.TPMR];
			for (i=0;i<((buffer[PHONE_SMSSubmit.Number]+1)/2+1)+1;i++) {
				req[(*current)++]=buffer[PHONE_SMSSubmit.Number+i];
			}
			req[(*current)++]=buffer[PHONE_SMSSubmit.TPPID];
			req[(*current)++]=buffer[PHONE_SMSSubmit.TPDCS];
			req[(*current)++]=buffer[PHONE_SMSSubmit.TPVP];
			req[(*current)++]=buffer[PHONE_SMSSubmit.TPUDL];
			for(i=0;i<length;i++) req[(*current)++]=buffer[PHONE_SMSSubmit.Text+i];
			EncodeHexBin(hexreq, req, *current);
			*length2 = *current * 2;
			*current = *current - (req[PHONE_SMSSubmit.SMSCNumber]+1);
		}
		break;
	case SMS_AT_TXT:
		if (theApp.m_ManufacturerID == AT_Unknown)
		{
			char szGetManufacturer[MAX_PATH];
			error=ATGEN_GetManufacturer(szGetManufacturer,pWriteCommandfn,pDebuginfo);
			if (error != ERR_NONE) return error;
		}
		if (theApp.m_ManufacturerID != AT_Nokia) {
			if (message->Coding != SMS_Coding_Default) return ERR_NOTSUPPORTED;
		}
		error=PHONE_EncodeSMSFrame(message,req,PHONE_SMSDeliver,&i,true,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;
		CopyUnicodeString(SMSC.Number,message->SMSC.Number);
		SMSC.Location=1;
		error=ATGEN_SetSMSC(&SMSC,pWriteCommandfn,pDebuginfo);
		if (error!=ERR_NONE) return error;
		sprintf((char*)buffer, "AT+CSMP=%i,%i,%i,%i\r",
			req[PHONE_SMSDeliver.firstbyte],
			req[PHONE_SMSDeliver.TPVP],
			req[PHONE_SMSDeliver.TPPID],
			req[PHONE_SMSDeliver.TPDCS]);
	//	error=GSM_WaitFor (s, buffer, strlen(buffer), 0x00, 4, ID_SetSMSParameters);
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CSMP");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		error = pWriteCommandfn (buffer, strlen((char*)buffer), 0x00, 8,false,NULL,&ReplymsgType,ATGEN_GenericReply);
		if (error==ERR_NOTSUPPORTED) 
		{
			/* Nokia Communicator 9000i doesn't support <vp> parameter */
			sprintf((char*)buffer, "AT+CSMP=%i,,%i,%i\r",
				req[PHONE_SMSDeliver.firstbyte],
				req[PHONE_SMSDeliver.TPPID],
				req[PHONE_SMSDeliver.TPDCS]);
		//	error=GSM_WaitFor (s, buffer, strlen(buffer), 0x00, 4, ID_SetSMSParameters);
			ReplymsgType.nCount = 1;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CSMP");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;
			error = pWriteCommandfn ((unsigned char *)buffer, strlen((char*)buffer), 0x00, 8,false,NULL,&ReplymsgType,ATGEN_GenericReply);
		}
		if (error!=ERR_NONE) return error;
		switch (message->Coding)
		{
		case SMS_Coding_Default:
			/* If not SMS with UDH, it's as normal text */
			if (message->UDH.Type==UDH_NoUDH) {
				strcpy((char*)hexreq,(char*)DecodeUnicodeString(message->Text));
				*length2 = UnicodeLength(message->Text);
				break;
			}
			case SMS_Coding_Unicode:        
			case SMS_Coding_8bit:
			error=PHONE_EncodeSMSFrame(message,buffer,PHONE_SMSDeliver,current,true,pWriteCommandfn,pDebuginfo);
			if (error != ERR_NONE) return error;
			EncodeHexBin (hexreq, buffer+PHONE_SMSDeliver.Text, buffer[PHONE_SMSDeliver.TPUDL]);
			*length2 = buffer[PHONE_SMSDeliver.TPUDL] * 2;
			break;
		}
		break;
	}
	return ERR_NONE;
}
GSM_Error ATGEN_PrivAddSMS(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,bool bSetSMSMode,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error 		error, error2;
	int			state,reply, current, current2;
	unsigned char		buffer[1000], hexreq[1000];
//	GSM_Phone_Data		*Phone = &s->Phone.Data;
	unsigned char		*statetxt;
	unsigned char folderid;
	theApp.pSetATProtocolDatafn = pSetATProtocolData;
//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;
	/* This phone supports only sent/unsent messages on SIM */
	if (IsPhoneFeatureAvailable(pMobileInfo, F_SMSONLYSENT)) {
		if (sms->Folder != 2) {
			smprintf(theApp.m_pDebuginfo, "This phone supports only folder = 2!\n");
			return ERR_NOTSUPPORTED;
		}
	}
	
	sms->PDU = SMS_Submit;
	switch (sms->Folder) {
	case 1:  sms->PDU = SMS_Deliver;		/* Inbox SIM */
		 error=ATGEN_SetSMSMemory(true,pWriteCommandfn,pDebuginfo);
		 break;
	case 2:  error=ATGEN_SetSMSMemory(true,pWriteCommandfn,pDebuginfo);	/* Outbox SIM */
	 	 break;
	case 3:  sms->PDU = SMS_Deliver;
		 error=ATGEN_SetSMSMemory(false,pWriteCommandfn,pDebuginfo);	/* Inbox phone */
		 break;
	case 4:  error=ATGEN_SetSMSMemory(false,pWriteCommandfn,pDebuginfo);	/* Outbox phone */
		 break;
	default: return ERR_NOTSUPPORTED;
	}
	if (error!=ERR_NONE) return error;

	if(bSetSMSMode == false)
	{
		error=ATGEN_GetSMSMode(pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;
	}
	else
	{
		error=ATGEN_SetSMSMode(true,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;
	}
	error = ATGEN_MakeSMSFrame(sms, hexreq, &current, &current2,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

	switch (theApp.m_SMSMode) {
	case SMS_AT_PDU:
		if (sms->PDU == SMS_Deliver) {
			state = 0;
			if (sms->State == SMS_Read || sms->State == SMS_Sent) state = 1;
		} else {
			state = 2;
			if (sms->State == SMS_Read || sms->State == SMS_Sent) state = 3;
		}
		/* Siemens M20 */
		if (IsPhoneFeatureAvailable(pMobileInfo, F_M20SMS)) {
			/* No (good and 100% working) support for alphanumeric numbers */
			if (sms->Number[1]!='+' && (sms->Number[1]<'0' || sms->Number[1]>'9')) {
				EncodeUnicode(sms->Number,(unsigned char *)"123",3);
				error=ATGEN_GetSMSMode(pWriteCommandfn,pDebuginfo);
				if (error != ERR_NONE) return error;
				error = ATGEN_MakeSMSFrame(sms, hexreq, &current, &current2,pWriteCommandfn,pDebuginfo);
				if (error != ERR_NONE) return error;
			}
		}
		if(theApp.m_ManufacturerID == AT_Motorola)
			sprintf((char*)buffer, "AT+CMGW=%i\r",current);
		else
			sprintf((char*)buffer, "AT+CMGW=%i,%i\r",current,state);

		break;
	case SMS_AT_TXT:
		if (sms->PDU == SMS_Deliver) {
			statetxt =(unsigned char *) "REC UNREAD";
			if (sms->State == SMS_Read || sms->State == SMS_Sent) statetxt =(unsigned char *) "REC READ";
		} else {
			statetxt =(unsigned char *) "STO UNSENT";
			if (sms->State == SMS_Read || sms->State == SMS_Sent) statetxt = (unsigned char *)"STO SENT";
		}
		/* Siemens M20 */
		if (IsPhoneFeatureAvailable(pMobileInfo, F_M20SMS)) {
			/* No (good and 100% working) support for alphanumeric numbers */
			/* FIXME: Try to autodetect support for <stat> (statetxt) parameter although:
			 * Siemens M20 supports +CMGW <stat> specification but on my model it just
			 * reports ERROR (and <stat> is not respected).
			 * Fortunately it will write "+CMGW: <index>\n" before and the message gets written
			 */
			if (sms->Number[1]!='+' && (sms->Number[1]<'0' || sms->Number[1]>'9')) {
		        	sprintf((char*)buffer, "AT+CMGW=\"123\",,\"%s\"\r",statetxt);
			} else {
		        	sprintf((char*)buffer, "AT+CMGW=\"%s\",,\"%s\"\r",DecodeUnicodeString(sms->Number),statetxt);
			}
		} else {
			sprintf((char*)buffer, "AT+CMGW=\"%s\",,\"%s\"\r",DecodeUnicodeString(sms->Number),statetxt);
		}
	}

	theApp.m_SaveSMSMessage = sms;
	
	for (reply=0;reply<2;reply++)
	{
		theApp.m_EditMode 	= true;
		theApp.pSetATProtocolDatafn(true,true,0x01);

	//	Replies 			= s->ReplyNum;
//		s->ReplyNum			= 1;
		smprintf(theApp.m_pDebuginfo,"Waiting for modem prompt\n");

//		error=GSM_WaitFor (s, buffer, strlen(buffer), 0x00, 3, ID_SaveSMSMessage);
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CMGW");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		error = pWriteCommandfn ((unsigned char *)buffer, strlen((char*)buffer), 0x00, 16,false,NULL,&ReplymsgType,ATGEN_ReplyAddSMSMessage);
	
	//	s->ReplyNum			 = Replies;
		if (error == ERR_NONE) 
		{
	//		Phone->DispatchError 	= ERR_TIMEOUT;
	//		Phone->RequestID 	= ID_SaveSMSMessage;
	//		smprintf(theApp.m_pDebuginfo, "Saving SMS\n");
		//	error = s->Protocol.Functions->WriteMessage(s, hexreq, current2, 0x00);
			error = pWriteCommandfn ((unsigned char *)hexreq, current2, 0x00, 0,false,NULL,NULL,NULL);
			if (error!=ERR_NONE) return error;
			my_sleep(500);
			/* CTRL+Z ends entering */
	//		error = s->Protocol.Functions->WriteMessage(s, "\x1A", 1, 0x00);
			error = pWriteCommandfn ((unsigned char *)"\x1A", 1, 0x00, 20,false,NULL,&ReplymsgType,ATGEN_ReplyAddSMSMessage);
		
	//		if (error!=ERR_NONE) return error;
	//		error = GSM_WaitForOnce(s, NULL, 0x00, 0x00, 4);
			if (error != ERR_TIMEOUT)
			{
				if(bSetSMSMode == false)
				{
					if(sms->Folder == 1 || sms->Folder ==2)
						folderid =1 ;
					else
						folderid =2 ;
					if (theApp.m_SMSMemory == MEM_ME && IsPhoneFeatureAvailable(pMobileInfo, F_SMSME900)) sms->Location -= 899;
					ATGEN_SetSMSLocation(sms, folderid, sms->Location);
				}
				else
				{
					if(theApp.m_SMSMemory == MEM_SM)
						sms->Location =( -1)*sms->Location;
				}
				return error;
			}
		} else {
			smprintf(theApp.m_pDebuginfo, "Escaping SMS mode\n");
		//	error2 = s->Protocol.Functions->WriteMessage(s, "\x1B\r", 2, 0x00);
			error2 = pWriteCommandfn ((unsigned char *)"\x1B\r", 2, 0x00, 0,false,NULL,NULL,NULL);
			if (error2 != ERR_NONE) return error2;
			
			if(bSetSMSMode == false)
			{
				if(sms->Folder == 1 || sms->Folder ==2)
					folderid ='1';
				else
					folderid ='2';

				if (theApp.m_SMSMemory == MEM_ME && IsPhoneFeatureAvailable(pMobileInfo, F_SMSME900)) sms->Location -= 899;
				ATGEN_SetSMSLocation(sms, folderid, sms->Location);
			}
			else
			{
				if(theApp.m_SMSMemory == MEM_SM)
					sms->Location =( -1)*sms->Location;
			}

			return error;
		}
    }

	return error;
}
GSM_Error ATGEN_AddSMS(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error err =ATGEN_PrivAddSMS(pMobileInfo,sms,false,pSetATProtocolData,pWriteCommandfn,pDebuginfo);
	if(IsPhoneFeatureAvailable(pMobileInfo, F_SMS_SHIFTMEINDEX))
	{
		if(sms->Memory == MEM_ME)
			sms->Location -=theApp.m_SIMSize;
	}
	return err;
}
GSM_Error ATGEN_DeleteSMS(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	unsigned char		req[20], folderid;
	GSM_Error		error;
	int			location;
	int add = 0;
	GSM_MultiSMSMessage	msms;
	if (sms->Memory==MEM_SM)
	{


	msms.Number = 0;
	msms.SMS[0] = *sms;

	/* By reading SMS we check if it is really inbox/outbox */
	error = ATGEN_GetSMS(pMobileInfo,&msms,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

	error = ATGEN_GetSMSLocation(sms, &folderid, &location,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

	if (theApp.m_SMSMemory == MEM_ME && IsPhoneFeatureAvailable(pMobileInfo, F_SMSME900)) add = 899;
	sprintf((char*)req, "AT+CMGD=%i\r",location+ add);

	smprintf(theApp.m_pDebuginfo, "Deleting SMS\n");
//	return GSM_WaitFor (s, req, strlen(req), 0x00, 5, ID_DeleteSMSMessage);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CMGD");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return  pWriteCommandfn ( (unsigned char *)req, strlen((char*)req), 0x00, 10,false,NULL,&ReplymsgType,ATGEN_ReplyDeleteSMSMessage);
	}
	else
	{
		sprintf((char*)req, "AT*SYSM=6,1,%d\r",sms->Location);
		smprintf(theApp.m_pDebuginfo, "Deleting SMS\n");
		//	return GSM_WaitFor (s, req, strlen(req), 0x00, 5, ID_DeleteSMSMessage);
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT*SYSM");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		return  pWriteCommandfn ( (unsigned char *)req, strlen((char*)req), 0x00, 10,false,NULL,&ReplymsgType,ATGEN_ReplyDeleteSMSMessage);


	}

}
GSM_Error ATGEN_PrivSendSMS( GSM_SMSMessage *sms,bool bSetSMSMode,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error 		error,error2;
	int			current, current2;
	unsigned char		buffer[1000], hexreq[1000];
//	GSM_Phone_Data		*Phone = &s->Phone.Data;
	theApp.pSetATProtocolDatafn = pSetATProtocolData;

//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;
	if (sms->PDU == SMS_Deliver) sms->PDU = SMS_Submit;
	if(bSetSMSMode == false)
	{
		error=ATGEN_GetSMSMode(pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;
	}
	else
	{
		error=ATGEN_SetSMSMode(true,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;
	}

	error = ATGEN_MakeSMSFrame(sms, hexreq, &current, &current2,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;
	error=ATGEN_SetSMSMode(true,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;//080512libaoliu
	switch (theApp.m_SMSMode) 
	{
	case SMS_AT_PDU:
		if (current<100)
		{
		sprintf((char*)buffer, "AT+CMGS=0%i\r",current);
		}
		else
		{
		sprintf((char*)buffer, "AT+CMGS=%i\r",current);
		}
		// added by fung for test
	//	MessageBox( NULL, "AT+CMGS=SMS_AT_PDU", "SMS_AT_PDU", MB_OK);// for testing 
		break;
	case SMS_AT_TXT:
		sprintf((char*)buffer, "AT+CMGS=\"%s\"\r",DecodeUnicodeString(sms->Number));
		// added by fung for test
	//	MessageBox( NULL, "AT+CMGS=SMS_AT_TXT", "SMS_AT_TXT", MB_OK);// for testing 
		break;
	}
	theApp.m_EditMode 	= true;
	pSetATProtocolData(true,true,0x01);

//	Replies 			= s->ReplyNum;
//	s->ReplyNum			= 1;
	smprintf(theApp.m_pDebuginfo,"Waiting for modem prompt\n");
//	error=GSM_WaitFor (s, buffer, strlen(buffer), 0x00, 3, ID_IncomingFrame);
	theApp.m_bGetSendSMSResult = false;
	theApp.m_SendSMSStatus = ERR_TIMEOUT;
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CMGS");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"AT+CMGS");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)buffer, strlen((char*)buffer), 0x00, 3,false,NULL,&ReplymsgType,ATGEN_ReplySendSMS);
//	s->ReplyNum			 = Replies;
	if (error == ERR_NONE) 
	{
		smprintf(theApp.m_pDebuginfo, "Sending SMS\n");
//		error = s->Protocol.Functions->WriteMessage(s, hexreq, current2, 0x00);
		error = pWriteCommandfn ((unsigned char *)hexreq, current2, 0x00, 0,false,NULL,NULL,NULL);
		if (error!=ERR_NONE) return error;
		my_sleep(500);
		/* CTRL+Z ends entering */
	//	error=s->Protocol.Functions->WriteMessage(s, "\x1A", 1, 0x00);
		error = pWriteCommandfn ((unsigned char *)"\x1A", 1, 0x00, 90,false,NULL,&ReplymsgType,ATGEN_ReplySendSMS);
		if(error == ERR_NONE ||  theApp.m_SendSMSStatus == ERR_NONE)
			error = ERR_NONE;
		my_sleep(100);
		return error;
	} 
	else 
	{
		smprintf(theApp.m_pDebuginfo, "Escaping SMS mode\n");
//		error2=s->Protocol.Functions->WriteMessage(s, "\x1B\r", 2, 0x00);
		error2 = pWriteCommandfn ((unsigned char *)"\x1B\r", 2, 0x00, 0,false,NULL,NULL,NULL);
		if (error2 != ERR_NONE) return error2;
	}
	return error;
}
GSM_Error ATGEN_SendSMS( GSM_SMSMessage *sms,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	return ATGEN_PrivSendSMS(sms,false,pSetATProtocolData,pWriteCommandfn,pDebuginfo);
}
GSM_Error ATGEN_SetSMSMode(bool bPDUMode,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
  	GSM_Error 		error = ERR_NONE;
	theApp.m_pDebuginfo = pDebuginfo;

	// added by mingfa for test
//	MessageBox( NULL, "ATGEN_SetSMSMode AT+CMGF=0", "ATGEN_SetSMSMode", MB_OK);// for testing 

	if(bPDUMode)
	{
		if(theApp.m_SMSMode == SMS_AT_PDU)
		{
	//		error=GSM_WaitFor (s, "AT+CMGF=0\r", 10, 0x00, 3, ID_GetSMSMode);
			ReplymsgType.nCount = 1;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CMGF");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;
			error= pWriteCommandfn ((unsigned char *)"AT+CMGF?\r", 9, 0x00, 6,false,NULL,&ReplymsgType,ATGEN_GenericReply);

			if (error==ERR_NONE) {
				theApp.m_SMSMode = SMS_AT_PDU;
				return ERR_NONE;
			}
		}
	}
	else 
	{
		if(theApp.m_SMSMode != SMS_AT_TXT)
		{
			ReplymsgType.nCount = 1;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CMGF");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;
			error= pWriteCommandfn ((unsigned char *)"AT+CMGF=1\r", 10, 0x00, 6,false,NULL,&ReplymsgType,ATGEN_GenericReply);
			if (error==ERR_NONE) 
			{
				theApp.m_SMSMode = SMS_AT_TXT;
			}
		}
	}

	return error;
}
GSM_Error ATGEN_GetSMSLocationMMGL(GSM_SMSMessage *sms, unsigned char *folderid, int *location,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	GSM_Error		error = ERR_NONE;
	theApp.m_pDebuginfo = pDebuginfo;

	if (theApp.m_PhoneSMSMemory == 0)
	{
		error = ATGEN_SetSMSMemory(false,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (theApp.m_SIMSMSMemory == 0) 
	{
		error = ATGEN_SetSMSMemory(true,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	
	if (theApp.m_SIMSMSMemory != AT_AVAILABLE && theApp.m_PhoneSMSMemory != AT_AVAILABLE) {
		return ERR_NOTSUPPORTED;
	}
	if(theApp.m_SMSUsedLocation.Number == 0)
	{
		if(theApp.m_PhoneSMSMemory == AT_AVAILABLE)
		{
			error = ATGEN_SetSMSMemory(false,pWriteCommandfn,pDebuginfo);
			error=ATGEN_SetSMSMode(false,pWriteCommandfn,pDebuginfo);
		//	error = GSM_WaitFor (s, "AT+MMGL=\"HEADER ONLY\"\r", 22, 0x00, 4, ID_GetSMSStatus);
			ReplymsgType.nCount = 2;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+MMGL");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;

			wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"+MMGL:");
			ReplymsgType.CheckInfo[1].subtypechar = 0;
			ReplymsgType.CheckInfo[1].subtype = 0x00;

			GSM_ATMultiAnwser ATMultiAnwser;
			wsprintf(ATMultiAnwser.Specialtext,"+MMGL:");
			ATMultiAnwser.Anwserlines = 1;
			error= pWriteCommandfn ((unsigned char *)"AT+MMGL=\"HEADER ONLY\"\r", 22, 0x00, 20,false,&ATMultiAnwser,&ReplymsgType,ATGEN_ReplyGetSMSLocation);
			if(error == ERR_TIMEOUT)
				error= pWriteCommandfn ((unsigned char *)"AT+MMGL=\"HEADER ONLY\"\r", 22, 0x00, 20,false,&ATMultiAnwser,&ReplymsgType,ATGEN_ReplyGetSMSLocation);
		}
		if(theApp.m_SIMSMSMemory == AT_AVAILABLE)
		{
			error = ATGEN_SetSMSMemory(true,pWriteCommandfn,pDebuginfo);
			error=ATGEN_SetSMSMode(false,pWriteCommandfn,pDebuginfo);
		//	error = GSM_WaitFor (s, "AT+MMGL=\"HEADER ONLY\"\r", 22, 0x00, 4, ID_GetSMSStatus);
			ReplymsgType.nCount = 2;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+MMGL");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;

			wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"+MMGL:");
			ReplymsgType.CheckInfo[1].subtypechar = 0;
			ReplymsgType.CheckInfo[1].subtype = 0x00;

			GSM_ATMultiAnwser ATMultiAnwser;
			wsprintf(ATMultiAnwser.Specialtext,"+MMGL:");
			ATMultiAnwser.Anwserlines = 1;
			error= pWriteCommandfn ((unsigned char *)"AT+MMGL=\"HEADER ONLY\"\r", 22, 0x00, 15,false,&ATMultiAnwser,&ReplymsgType,ATGEN_ReplyGetSMSLocation);
		}

	}
	if(theApp.m_LastSMSRead < theApp.m_SMSUsedLocation.Number)
	{
		*location = theApp.m_SMSUsedLocation.nIndex[theApp.m_LastSMSRead];
		*folderid = 1;
		theApp.m_LastSMSRead ++ ;
	}
	else 
		 return ERR_EMPTY;

	return error;

}
GSM_Error ATGEN_GetSMSMMGL(OnePhoneModel *pMobileInfo,GSM_MultiSMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	theApp.m_pDebuginfo = pDebuginfo;
	unsigned char	req[20], folderid;
	GSM_Error	error;
	int		 getfolder, add = 0;
	int		location = sms->SMS[0].Location;
	if(IsPhoneFeatureAvailable(pMobileInfo, F_SMS_SETMAXMEINDEX))
		theApp.m_MAXSMSINFOLDER = theApp.m_PhoneSize;
	ZeroMemory(&sms->SMS[0] ,sizeof(sms->SMS[0]));
	if(location == 0)
	{
		error=ATGEN_GetSMSLocationMMGL(&sms->SMS[0], &folderid, &location,pWriteCommandfn,pDebuginfo);
		if (error!=ERR_NONE) return error;
	}
	if(location < 0)
	{
		error = ATGEN_SetSMSMemory(true,pWriteCommandfn,pDebuginfo);
		location =(-1)*location;
	}
	else
		error = ATGEN_SetSMSMemory(false,pWriteCommandfn,pDebuginfo);



	if (theApp.m_SMSMemory == MEM_ME && IsPhoneFeatureAvailable(pMobileInfo, F_SMSME900)) add = 899;
	sprintf((char*)req, "AT+CMGR=%i\r", location + add);

	error=ATGEN_SetSMSMode(true,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

//	error=ATGEN_GetManufacturer(s);
///	char szGetManufacturer[MAX_PATH];
//	error=ATGEN_GetManufacturer(szGetManufacturer,pWriteCommandfn,pDebuginfo);
//	if (error != ERR_NONE) return error;
///
	theApp.m_GetSMSMessage=sms;
	smprintf(theApp.m_pDebuginfo, "Getting SMS\n");
//	error=GSM_WaitFor (s, req, strlen(req), 0x00, 5, ID_GetSMSMessage);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CMGR");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	theApp.m_pMobileInfo = pMobileInfo;
	error= pWriteCommandfn (req, strlen((char*)req), 0x00, 10,false,NULL,&ReplymsgType,MOTO_ReplyGetSMSMessage);
///
	if(theApp.m_SMSMemory == MEM_SM) 
		location = (-1)*location;
	if (error==ERR_NONE) 
	{
		getfolder = sms->SMS[0].Folder;
//		if (getfolder != 0 && getfolder != sms->SMS[0].Folder) return ERR_EMPTY;
//		MOTO_SetSMSLocation(s, &sms->SMS[0], folderid, location);
		sms->SMS[0].Location = location;
		sms->SMS[0].Folder = getfolder;
		sms->SMS[0].Memory = MEM_SM;
		if (getfolder > 2) sms->SMS[0].Memory = MEM_ME;
	}
	else
	{
		error=ATGEN_SetSMSMode(false,pWriteCommandfn,pDebuginfo);
		theApp.m_GetSMSMessage=sms;
		smprintf(theApp.m_pDebuginfo, "Getting SMS\n");
	//	error=GSM_WaitFor (s, req, strlen(req), 0x00, 5, ID_GetSMSMessage);
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CMGR");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		theApp.m_pMobileInfo = pMobileInfo;
		error= pWriteCommandfn (req, strlen((char*)req), 0x00, 10,false,NULL,&ReplymsgType,MOTO_ReplyGetSMSMessage);
		if (error==ERR_NONE) {
			getfolder = sms->SMS[0].Folder;
	//		if (getfolder != 0 && getfolder != sms->SMS[0].Folder) return ERR_EMPTY;
		//	MOTO_SetSMSLocation(s, &sms->SMS[0], folderid, location);
			sms->SMS[0].Location = location;
			sms->SMS[0].Folder = getfolder;
			sms->SMS[0].Memory = MEM_SM;
			if (getfolder > 2) sms->SMS[0].Memory = MEM_ME;
		}
	}
	return error;
}
GSM_Error ATGEN_GetNextSMSMMGL(OnePhoneModel *pMobileInfo,GSM_MultiSMSMessage *sms, bool start,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	GSM_Error 		error;
//	int			usedsms;

	if (theApp.m_PhoneSMSMemory == 0)
	{
		error = ATGEN_SetSMSMemory(false,pWriteCommandfn,pDebuginfo);
	/*	if (error != ERR_NONE && error != ERR_NOTSUPPORTED)
		{
			error = GSM_WaitFor (s, "AT+CPMS=\"MT\"\r", 13, 0x00, 4, ID_GetSMSStatus);
		}*/

		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (theApp.m_SIMSMSMemory == 0)
	{
		error = ATGEN_SetSMSMemory(true,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (theApp.m_SIMSMSMemory == AT_NOTAVAILABLE && theApp.m_PhoneSMSMemory == AT_NOTAVAILABLE) return ERR_NOTSUPPORTED;

	if (start) 
	{
	//	error=s->Phone.Functions->GetSMSStatus(&theApp.m_LastSMSStatus);
		error=ATGEN_GetSMSStatus(&theApp.m_LastSMSStatus,pWriteCommandfn,pDebuginfo);
		if (error!=ERR_NONE) return error;
		theApp.m_LastSMSRead		= 0;
		sms->SMS[0].Location 		= 0;
		theApp.m_SMSUsedLocation.Number = 0;
	}
	while (true)
	{
		sms->SMS[0].Location 		= 0;
		error=ATGEN_GetSMSMMGL( pMobileInfo,sms,pWriteCommandfn,pDebuginfo);
		if (error==ERR_NONE)
			break;
		if (error != ERR_EMPTY && error != ERR_INVALIDLOCATION) 
			return error;
	}
	return error;
}
GSM_Error ATGEN_SendSMSMMGL(GSM_SMSMessage *sms,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	theApp.m_pDebuginfo = pDebuginfo;
	return ATGEN_PrivSendSMS(sms,true,pSetATProtocolData,pWriteCommandfn,pDebuginfo);
}
GSM_Error ATGEN_AddSMSMMGL(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	return ATGEN_PrivAddSMS(pMobileInfo,sms,true,pSetATProtocolData,pWriteCommandfn,pDebuginfo);
}
GSM_Error ATGEN_DeleteSMSMMGL(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	unsigned char		req[20];//, folderid;
	GSM_Error		error;
//	int			location;
	GSM_MultiSMSMessage	msms;

	msms.Number = 0;
	msms.SMS[0] = *sms;


	/* By reading SMS we check if it is really inbox/outbox */
	error = ATGEN_GetSMSMMGL(pMobileInfo,&msms,pWriteCommandfn,pDebuginfo);
//	if (error != ERR_NONE) return error;
	if(msms.SMS[0].Location < 0)
		msms.SMS[0].Location = (-1)*msms.SMS[0].Location;

	sprintf((char*)req, "AT+CMGD=%i\r",msms.SMS[0].Location);

	smprintf(theApp.m_pDebuginfo, "Deleting SMS\n");
//	return GSM_WaitFor (s, req, strlen(req), 0x00, 5, ID_DeleteSMSMessage);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CMGD");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return  pWriteCommandfn ( (unsigned char *)req, strlen((char*)req), 0x00, 10,false,NULL,&ReplymsgType,ATGEN_ReplyDeleteSMSMessage);
}