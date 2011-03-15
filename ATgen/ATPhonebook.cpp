#include "stdafx.h"
#include "Atgen.h"
#include "AtGenFundef.h"
#include "coding.h"
#include "gsmpbk.h"

extern CATgenApp theApp;

extern GSM_Reply_MsgType ReplymsgType;
GSM_Error ATGEN_ReplyGetMutiMemory(GSM_Protocol_Message msg)
{
	int nIndex = 0;
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	GSM_MemoryEntry		*Memory = &theApp.m_PBKMemoryEntry;
	char			*pos;
	unsigned char		buffer[500],buffer2[500];
	int			len;
	FILE *	file;

    GSM_MemoryStatus  *MemoryStatus = theApp.m_MemoryStatus;
	int testlen = 0;


	switch (Replynfo.ReplyState) {
	case AT_Reply_Continue:
	case AT_Reply_OK:
 		smprintf(theApp.m_pDebuginfo, "Phonebook entry received\n");
 		Memory->EntriesNum = 0;

		pos = strstr((char*)msg.Buffer, "+CPBR:");
		if (pos == NULL)
		{
			if(Replynfo.ReplyState == AT_Reply_Continue)
				return ERR_UNKNOWN;
			else return ERR_NONE; 
		}
		/* Go after +CPBR: */
		pos += 6; 

		/* Location */
		while (*pos && !isdigit(*pos)) pos++;
	//	Memory->Location = atoi(pos) + 1 - theApp.m_MemoryInfo.FirstMemoryEntry;
 		nIndex = atoi(pos) + 1 - theApp.m_MemoryInfo.FirstMemoryEntry;
		wsprintf(Memory->szIndex,"%d",nIndex);
//		smprintf(theApp.m_pDebuginfo, "Location: %d\n", Memory->Location);
		smprintf(theApp.m_pDebuginfo, "Location: %s\n", Memory->szIndex);
			
		/* Number */
		while (*pos != '"') pos++;
		pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 		smprintf(theApp.m_pDebuginfo, "Number: %s\n",buffer);
 		Memory->EntriesNum++;
 		Memory->Entries[0].EntryType  = PBK_Number_General;
 		Memory->Entries[0].VoiceTag   = 0;
 		Memory->Entries[0].SMSList[0] = 0;

		len = strlen((char*)buffer + 1) - 1;
		if (theApp.m_PBKCharset == AT_PBK_HEX && (len > 10) && (len % 2 == 0) && (strchr((char*)buffer + 1, '+') == NULL)) {
			/* This is probably hex encoded number */
			DecodeHexBin(buffer2, buffer+1, len);
			DecodeDefault(Memory->Entries[0].Text ,buffer2, strlen((char*)buffer2), false, NULL);
		} else if (theApp.m_PBKCharset == AT_PBK_UCS2 && (len > 20) && (len % 4 == 0) && (strchr((char*)buffer + 1, '+') == NULL)) {
			/* This is probably unicode encoded number */
			DecodeHexUnicode(Memory->Entries[0].Text, buffer + 1,len);
		} else  {
	 		EncodeUnicode(Memory->Entries[0].Text, buffer + 1, len);
		}
		
		/* Number format */
		pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 		smprintf(theApp.m_pDebuginfo, "Number format: %s\n",buffer);

		/* International number */
		if (!strcmp((char*)buffer,"145")) {
			sprintf((char*)buffer+1,"%s",DecodeUnicodeString(Memory->Entries[0].Text));
			if (strlen((char*)buffer+1)!=0 && buffer[1] != '+') {
				/* Sony Ericsson issue */
				/* International number is without + */
				buffer[0] = '+';
				EncodeUnicode(Memory->Entries[0].Text,buffer,strlen((char*)buffer));
			}
		}

		/* Name */
		pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 		smprintf(theApp.m_pDebuginfo, "Name text: %s\n",buffer);

		// only for ME type
		if ( (theApp.m_ManufacturerID == AT_Ericsson) && ( MemoryStatus->MemoryType == 1 ) )
		{	 
			testlen = strlen((char*)buffer);

			// check last two byte , that was added vt SE mobile-self
			if( ( buffer[strlen((char*)buffer)-3] == '/') && (buffer[strlen((char*)buffer)-2] == 'H') )
			{
              Memory->Entries[0].EntryType = PBK_Number_Home;
              buffer[strlen((char*)buffer)-3]=0x22; // it is ( " ).
              buffer[strlen((char*)buffer)-2]=0x00;
			}

			else if( ( buffer[strlen((char*)buffer)-3] == '/') && (buffer[strlen((char*)buffer)-2] == 'W') )
			{
              Memory->Entries[0].EntryType = PBK_Number_Work; 
              buffer[strlen((char*)buffer)-3]=0x22; // it is ( " ).
              buffer[strlen((char*)buffer)-2]=0x00;
			}

			else if( ( buffer[strlen((char*)buffer)-3] == '/') && (buffer[strlen((char*)buffer)-2] == 'M') )
			{
              Memory->Entries[0].EntryType = PBK_Number_Mobile; 
              buffer[strlen((char*)buffer)-3]=0x22; // it is ( " ).
              buffer[strlen((char*)buffer)-2]=0x00;
			}
			else
			  Memory->Entries[0].EntryType = PBK_Number_General;
		}



 		Memory->EntriesNum++;
 		Memory->Entries[1].EntryType=PBK_Text_Name;
		switch (theApp.m_PBKCharset) {
		case AT_PBK_HEX:
			DecodeHexBin(buffer2,buffer+1,strlen((char*)buffer)-2);
 			DecodeDefault(Memory->Entries[1].Text,buffer2,strlen((char*)buffer2),false,NULL);
			break;
		case AT_PBK_GSM:
 			DecodeDefault(Memory->Entries[1].Text,buffer+1,strlen((char*)buffer)-2,false,NULL);
			break;			
		case AT_PBK_UTF8:

			DecodeUTF8ToUnicode(Memory->Entries[1].Text,buffer+1,strlen((char*)buffer)-2);	//for _UNICODE
			break;			
		case AT_PBK_UCS2:		
			if(buffer[0] = '"')
				DecodeHexUnicode(Memory->Entries[1].Text,buffer+1,strlen((char*)buffer+1) - 1);
			else
				DecodeHexUnicode(Memory->Entries[1].Text,buffer,strlen((char*)buffer));

			break;			
		case AT_PBK_PCCP437:
			/* FIXME: correctly decode PCCP437 */
 			DecodeDefault(Memory->Entries[1].Text,buffer+1,strlen((char*)buffer)-2,false,NULL);
			break;
		}

		/* Samsung number type */
		if (theApp.m_ManufacturerID == AT_Samsung)
		{
			int type;

			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
 			smprintf(theApp.m_pDebuginfo, "Number type: %s\n",buffer);
			type = strtoul((char*)buffer, NULL, 0);
			switch (type) {
			case 0:
 				Memory->Entries[0].EntryType = PBK_Number_Mobile;
				break;
			case 1:
 				Memory->Entries[0].EntryType = PBK_Number_Work;
				break;
			case 2:
 				Memory->Entries[0].EntryType = PBK_Number_Home;
				break;
			case 3:
 				Memory->Entries[0].EntryType = PBK_Text_Email;
				break;
			default:
 				Memory->Entries[0].EntryType = PBK_Number_General;
			}
		}




		theApp.m_PHKNum ++;
		file = fopen(theApp.m_szPHKTempFile,"a+b");
		if(file)
		{
			fwrite(&theApp.m_PBKMemoryEntry,1,sizeof(GSM_MemoryEntry),file);
			fclose(file);
		}

		ZeroMemory(&theApp.m_PBKMemoryEntry,sizeof(GSM_MemoryEntry));
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
GSM_Error ATGEN_ReplyGetPBKMemories(GSM_Protocol_Message msg)
{
	smprintf(theApp.m_pDebuginfo, "PBK memories received\n");
	if (strlen((char*)msg.Buffer) > AT_PBK_MAX_MEMORIES) {
		smprintf(theApp.m_pDebuginfo, "ERROR: Too long phonebook memories information received! (Recevided %d, AT_PBK_MAX_MEMORIES is %d\n", strlen((char*)msg.Buffer), AT_PBK_MAX_MEMORIES);
		return ERR_MOREMEMORY;
	}
	memcpy(theApp.m_PBKMemories,msg.Buffer,strlen((char*)msg.Buffer));
	return ERR_NONE;
}


GSM_Error ATGEN_ReplyGetCPBRMemoryInfo(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);
	char 			*pos;
 	unsigned char		buffer[500];

 	switch (Replynfo.ReplyState) 
	{
 	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "Memory info received\n");
		/* Parse +CPBR: (first-last),max_number_len,max_name_len */ 

		/* Parse first location */
		pos = strchr((char*)msg.Buffer, '(');
		if (!pos) 
		{
			pos = strstr((char*)msg.Buffer, "+CPBR:");
			if (pos == NULL)
				return ERR_UNKNOWN;
			/* Go after +CPBR: */
			pos += 6; 
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);

			CString strBuffer = buffer ;
			int nth = strBuffer.Find("-");
			if(nth == -1)return ERR_UNKNOWN;
			CString strTemp = strBuffer.Left(nth);
			theApp.m_MemoryInfo.FirstMemoryEntry = atoi(strTemp.GetBuffer(MAX_PATH));
			strTemp.ReleaseBuffer();
	
			strTemp = strBuffer.Right(strBuffer.GetLength()-nth -1);
			theApp.m_MemoryInfo.MemorySize = atoi(strTemp.GetBuffer(MAX_PATH)) + 1 - theApp.m_MemoryInfo.FirstMemoryEntry;
			strTemp.ReleaseBuffer();

			buffer[0] = 0;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
			if (strlen((char*)buffer) <=0) return ERR_UNKNOWN;
			theApp.m_MemoryInfo.NumberLength = atoi((char*)buffer);

			buffer[0] = 0;
			pos += ATGEN_ExtractOneParameter((unsigned char *)pos, buffer);
			if (strlen((char*)buffer) <=0) return ERR_UNKNOWN;
			theApp.m_MemoryInfo.TextLength = atoi((char*)buffer);

			return ERR_NONE;
		}
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
GSM_Error ATGEN_ReplySetMemory(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);
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
}GSM_Error SIEMENS_ReplyGetMemory(GSM_Protocol_Message msg)
{
	return ERR_NONE;
}
GSM_Error ATGEN_ReplyGetCPBRMemoryStatus(GSM_Protocol_Message msg)
{
	int			line=0;
	char			*str;
	int			cur;
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);
	GSM_MemoryStatus	*MemoryStatus =theApp.m_MemoryStatus;


	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "Memory entries received\n");
		/* Walk through lines with +CPBR: */
		while (theApp.m_Lines.numbers[line*2+1]!=0) {
			str = GetLineString(msg.Buffer,theApp.m_Lines,line+1);
			if (strncmp(str, "+CPBR: ", 7) == 0) {
				MemoryStatus->MemoryUsed++;
				if (sscanf(str, "+CPBR: %d,", &cur) == 1) {
					cur -= theApp.m_MemoryInfo.FirstMemoryEntry - 1;
					if (cur == theApp.m_NextMemoryEntry || theApp.m_NextMemoryEntry == 0)
						theApp.m_NextMemoryEntry = cur + 1;
				}
			}
			line++;
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

GSM_Error ATGEN_GetMemoryInfo(GSM_MemoryStatus *Status,GSM_MemoryInfo* pMemoryInfo, GSM_AT_NeededMemoryInfo NeededInfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	GSM_Error		error;
	char			req[20];
	int			start;
	int			end;
	int         max_no;

	smprintf(theApp.m_pDebuginfo, "Getting memory information\n");
	
	theApp.m_MemoryInfo.MemorySize		= 0;
	theApp.m_MemoryInfo.TextLength		= 0;
	theApp.m_MemoryInfo.NumberLength		= 0;
	theApp.m_MemoryInfo.FirstMemoryEntry = 0;


	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBR=?");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"AT+CPBR=?\r", 10, 0x00, 8, false,NULL,&ReplymsgType,ATGEN_ReplyGetCPBRMemoryInfo);

	memcpy(pMemoryInfo,&theApp.m_MemoryInfo,sizeof(GSM_MemoryInfo));

	if (error != ERR_NONE) return error;

	max_no = pMemoryInfo->MemorySize;
	if (NeededInfo == AT_Total || NeededInfo == AT_Sizes || NeededInfo == AT_First) return ERR_NONE;

	smprintf(theApp.m_pDebuginfo, "Getting memory status by reading values\n");
	
	theApp.m_MemoryStatus	= Status;
	Status->MemoryUsed		= 0;
	Status->MemoryFree		= 0;
	start				= pMemoryInfo->FirstMemoryEntry;
	theApp.m_NextMemoryEntry		= 0;
	pMemoryInfo->MemorySize += start-1;
	while (1) {
		end	= start + 20;
		if (end > pMemoryInfo->MemorySize) end = pMemoryInfo->MemorySize;
		sprintf(req, "AT+CPBR=%i,%i\r", start, end);

		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBR=");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		error = pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 8, false,NULL,&ReplymsgType,ATGEN_ReplyGetCPBRMemoryStatus);
		if(error == ERR_EMPTY)
		{
			Status->MemoryFree = max_no - Status->MemoryUsed;
			return ERR_NONE;
		}
		if (error != ERR_NONE) return error;

		if (NeededInfo == AT_NextEmpty && theApp.m_NextMemoryEntry != 0 && theApp.m_NextMemoryEntry != end + 1)
			return ERR_NONE;
		if (end == pMemoryInfo->MemorySize) {

			Status->MemoryFree = max_no - Status->MemoryUsed;
			return ERR_NONE;
		}
		start = end + 1;
	}
}

GSM_Error ATGEN_SetPBKMemoryEx(GSM_MemoryType MemType,bool bCheckOldType,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{

	char 			req[] = "AT+CPBS=\"XX\"\r";
	GSM_Error		error;


	if(bCheckOldType)
	{
		if (theApp.m_PBKMemory == MemType) return ERR_NONE;
	}

	/* Zero values that are for actual memory */
	theApp.m_MemoryInfo.MemorySize		= 0;
	theApp.m_MemoryInfo.FirstMemoryEntry		= 0;
	theApp.m_NextMemoryEntry		= 0;
	theApp.m_MemoryInfo.TextLength		= 0;
	theApp.m_MemoryInfo.NumberLength		= 0;

	if (theApp.m_PBKMemories[0] == 0) {

		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBS=?");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		error = pWriteCommandfn ((unsigned char *)"AT+CPBS=?\r", 10, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_ReplyGetPBKMemories);
		if (error != ERR_NONE) return error;
	}

	switch (MemType) {
		case MEM_SM:
			req[9] = 'S'; req[10] = 'M';
			break;
		case MEM_ME: 
		        if (strstr(theApp.m_PBKMemories,"ME")==NULL) return ERR_NOTSUPPORTED;
			req[9] = 'M'; req[10] = 'E';
			break;
		case MEM_RC:
		        if (strstr(theApp.m_PBKMemories,"RC")==NULL) return ERR_NOTSUPPORTED;
			req[9] = 'R'; req[10] = 'C';
			break;
		case MEM_MC:
		        if (strstr(theApp.m_PBKMemories,"MC")==NULL) return ERR_NOTSUPPORTED;
			req[9] = 'M'; req[10] = 'C';
			break;
		case MEM_ON:
		        if (strstr(theApp.m_PBKMemories,"ON")==NULL) return ERR_NOTSUPPORTED;
			req[9] = 'O'; req[10] = 'N';
			break;
		case MEM_FD:
		        if (strstr(theApp.m_PBKMemories,"FD")==NULL) return ERR_NOTSUPPORTED;
			req[9] = 'F'; req[10] = 'D';
			break;
		case MEM_DC:
			if (strstr(theApp.m_PBKMemories,"DC")!=NULL) {
				req[9] = 'D'; req[10] = 'C';
				break;
			}
			if (strstr(theApp.m_PBKMemories,"LD")!=NULL) {
				req[9] = 'L'; req[10] = 'D';
				break;
			}
			return ERR_NOTSUPPORTED;
			break;
		default:
			return ERR_NOTSUPPORTED;
	}

	smprintf(theApp.m_pDebuginfo, "Setting memory type\n");
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBS=");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		error = pWriteCommandfn ((unsigned char *)req, 13, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);

	if (error == ERR_NONE) theApp.m_PBKMemory = MemType;
	return error;
}
GSM_Error ATGEN_SetPBKMemory(GSM_MemoryType MemType,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	return ATGEN_SetPBKMemoryEx(MemType,true,pWriteCommandfn,pDebuginfo);

}

GSM_Error ATGEN_GetMemoryStatus(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error error;
	theApp.m_pDebuginfo = pDebuginfo;

	error = ATGEN_SetPBKMemory(Status->MemoryType,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;


	Status->MemoryUsed		= -1;
	Status->MemoryFree		= -1;
	error =  ATGEN_GetMemoryInfo(Status,&theApp.m_MemoryInfo, AT_Total,pWriteCommandfn,pDebuginfo);

	return error;

}
GSM_Error ATGEN_PrivGetMemory (GSM_MemoryEntry *entry, int endlocation,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error 		error;
	unsigned char		req[20];
	int nLocation;
	GSM_MemoryEntry* pMemoryEntry = NULL;
	FILE *	file;

	int nIndex = atoi(entry->szIndex);
	if(strlen(entry->szIndex) <=0 || nIndex==0) return ERR_INVALIDLOCATION;

	if (entry->MemoryType == MEM_ME)
	{
		if (theApp.m_PBKSBNR == 0) {
			sprintf((char*)req, "AT^SBNR=?\r");
			smprintf(theApp.m_pDebuginfo, "Checking availablity of SBNR\n");

			ReplymsgType.nCount = 1;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT^SBNR=?");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;
			error = pWriteCommandfn ((unsigned char *)req, strlen((char*)req), 0x00, 8, false,NULL,&ReplymsgType,ATGEN_GenericReply);
			switch (error) {
			case ERR_NONE:
				theApp.m_PBKSBNR = AT_SBNR_AVAILABLE;
				break;
			case ERR_UNKNOWN:
			case ERR_NOTSUPPORTED:
				theApp.m_PBKSBNR = AT_SBNR_NOTAVAILABLE;
				break;
			default:
				return error;
			}
		}
		if (theApp.m_PBKSBNR == AT_SBNR_AVAILABLE) {
			int nIndex = atoi(entry->szIndex);

			sprintf((char*)req, "AT^SBNR=vcf,%i\r",nIndex-1);
			theApp.m_Memory=entry;
			smprintf(theApp.m_pDebuginfo, "Getting phonebook entry\n");

			ReplymsgType.nCount = 1;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT^SBNR");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;
			return pWriteCommandfn ((unsigned char *)req, strlen((char*)req), 0x00, 8, false,NULL,&ReplymsgType,SIEMENS_ReplyGetMemory);
		}
	}

	if (endlocation == 0)
	{

		nLocation =atoi(entry->szIndex) -1;
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
	else 
	{
		return ERR_UNKNOWNRESPONSE;
	}
}
GSM_Error ATGEN_GetMemory (GSM_MemoryEntry *entry,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	return ATGEN_PrivGetMemory(entry, 0,pWriteCommandfn,pDebuginfo);
}
GSM_Error ATGEN_PrivSetMemory4Sharp(GSM_MemoryEntry *entry,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	/* REQUEST_SIZE should be big enough to handle all possibl cases
	 * correctly, especially with unicode entries */
#define REQUEST_SIZE	((4 * GSM_PHONEBOOK_TEXT_LENGTH) + 30)
#define NAME_TEXT_LEN	5*2
#define NAME_BUFF_LEN	NAME_TEXT_LEN+2

	int					Group, Name, Number,NumberType=0, len;
	GSM_Error 			error;
	unsigned char		req[REQUEST_SIZE + 1];
	unsigned char		name[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)], name_sim[NAME_BUFF_LEN];
	unsigned char		number[GSM_PHONEBOOK_TEXT_LENGTH + 1];
	int					reqlen, pos=0;
	bool				PreferUnicode = false, bMultiByte=false;
	GSM_Reply_MsgType	ReplymsgType;

	int nIndex = atoi(entry->szIndex);
	if(strlen(entry->szIndex) <=0 || nIndex==0) return ERR_INVALIDLOCATION;

	error = ATGEN_SetPBKMemory(entry->MemoryType,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;  // Put to Initialize

	GSM_PhonebookFindDefaultNameNumberGroup(entry, &Name, &Number, &Group);

	name[0] = 0;
	if (Name != -1) {
		PreferUnicode = true;
		error = ATGEN_SetPBKCharset(PreferUnicode,NULL,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;

		// recalculate the max length for TC,SC,...
		while(1){
			if(entry->Entries[Name].Text[pos]!=0x00 && entry->Entries[Name].Text[pos+1]!=0x00){
				bMultiByte=true; break; 
			}
			if(entry->Entries[Name].Text[pos]==0x00 && entry->Entries[Name].Text[pos+1]==0x00)
				break;
			pos+=2;
		}
		if(bMultiByte){
			memset(name_sim, 0, NAME_BUFF_LEN);
			memcpy(name_sim, entry->Entries[Name].Text, NAME_TEXT_LEN);
			CopyUnicodeString(entry->Entries[Name].Text, name_sim);
		}

		switch (theApp.m_PBKCharset) {
		case AT_PBK_HEX:
			EncodeHexBin(name, DecodeUnicodeString(entry->Entries[Name].Text), UnicodeLength(entry->Entries[Name].Text));
			len = strlen((char*)name);
			break;
		case AT_PBK_GSM:
			smprintf(theApp.m_pDebuginfo, "str: %s\n", DecodeUnicodeString(entry->Entries[Name].Text));
			len = UnicodeLength(entry->Entries[Name].Text);
			EncodeDefault(name, entry->Entries[Name].Text, &len, true, NULL);
			break;
		case AT_PBK_UTF8:

			len=EncodeUnicode2UTF8(entry->Entries[Name].Text,name);// for _UNICODE
			len = strlen((char*)name);
			break;
		case AT_PBK_UCS2:
			EncodeHexUnicode(name, entry->Entries[Name].Text, UnicodeLength(entry->Entries[Name].Text));
			len = strlen((char*)name);
			break;
		case AT_PBK_PCCP437:
			/* FIXME: correctly decode PCCP437 */
			smprintf(theApp.m_pDebuginfo, "str: %s\n", DecodeUnicodeString(entry->Entries[Name].Text));
			len = UnicodeLength(entry->Entries[Name].Text);
			EncodeDefault(name, entry->Entries[Name].Text, &len, true, NULL);
			break;
		}
	} else {
		smprintf(theApp.m_pDebuginfo, "WARNING: No usable name found!\n");
		len = 0;
	}

	if (Number != -1) {
		GSM_PackSemiOctetNumber(entry->Entries[Number].Text, number, false);
		NumberType = number[0];
		sprintf((char*)number,"%s",DecodeUnicodeString(entry->Entries[Number].Text));
	} else {
		smprintf(theApp.m_pDebuginfo, "WARNING: No usable number found!\n");
		number[0] = 0;
	}

	if (theApp.m_MemoryInfo.FirstMemoryEntry == 0) {
		error = ATGEN_GetMemoryInfo( NULL,&theApp.m_MemoryInfo, AT_First,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;
	}

	sprintf((char*)req, "AT+CPBW=%d,\"%s\",%i,\"", atoi(entry->szIndex) + theApp.m_MemoryInfo.FirstMemoryEntry - 1, number, NumberType);
	reqlen = strlen((char*)req);
	if (reqlen + len > REQUEST_SIZE - 2) {
		smprintf(theApp.m_pDebuginfo, "WARNING: Text truncated to fit in buffer!\n");
		len = REQUEST_SIZE - 2 - reqlen;
	}
	memcpy(req + reqlen, name, len);
	reqlen += len;
	memcpy(req + reqlen, "\"\r", 2);
	reqlen += 2;

	smprintf(theApp.m_pDebuginfo, "Writing phonebook entry\n");
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBW");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return pWriteCommandfn ((unsigned char *)req, reqlen, 0x00, 8, false,NULL,&ReplymsgType,ATGEN_ReplySetMemory);
#undef REQUEST_SIZE
}// ATGEN_PrivSetMemory4Sharp
GSM_Error ATGEN_PrivSetMemory(GSM_MemoryEntry *entry,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	return ATGEN_PrivSetMemoryEx(entry,true,true,pWriteCommandfn,pDebuginfo);
}
GSM_Error ATGEN_PrivSetMemoryEx(GSM_MemoryEntry *entry,bool bWithSpace,bool bWithIndex,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	/* REQUEST_SIZE should be big enough to handle all possibl cases
	 * correctly, especially with unicode entries */
	theApp.m_pDebuginfo = pDebuginfo;
#define REQUEST_SIZE	((4 * GSM_PHONEBOOK_TEXT_LENGTH) + 30)
	int			Group, Name, Number,NumberType=0, len;
	GSM_Error 		error;
	unsigned char		req[REQUEST_SIZE + 1];
	unsigned char		name[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];

	unsigned char		number[GSM_PHONEBOOK_TEXT_LENGTH + 1];
	int			reqlen;
	bool			PreferUnicode = false;


	int nIndex = atoi(entry->szIndex);
	if(strlen(entry->szIndex) <=0 || nIndex==0) return ERR_INVALIDLOCATION;

	error = ATGEN_SetPBKMemory(entry->MemoryType,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;  // Put to Initialize

	GSM_PhonebookFindDefaultNameNumberGroup(entry, &Name, &Number, &Group);

	name[0] = 0;
	if (Name != -1) {

		PreferUnicode = true;
		error = ATGEN_SetPBKCharset(PreferUnicode,NULL,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;

		switch (theApp.m_PBKCharset) {
		case AT_PBK_HEX:
			EncodeHexBin(name, DecodeUnicodeString(entry->Entries[Name].Text), UnicodeLength(entry->Entries[Name].Text));
			len = strlen((char*)name);
			break;
		case AT_PBK_GSM:
			smprintf(theApp.m_pDebuginfo, "str: %s\n", DecodeUnicodeString(entry->Entries[Name].Text));
			len = UnicodeLength(entry->Entries[Name].Text);
			EncodeDefault(name, entry->Entries[Name].Text, &len, true, NULL);
			break;
		case AT_PBK_UTF8:

			len=EncodeUnicode2UTF8(entry->Entries[Name].Text,name);// for _UNICODE
			len = strlen((char*)name);
			break;
		case AT_PBK_UCS2:
			EncodeHexUnicode(name, entry->Entries[Name].Text, UnicodeLength(entry->Entries[Name].Text));
			len = strlen((char*)name);
			break;
		case AT_PBK_PCCP437:
			/* FIXME: correctly decode PCCP437 */
			smprintf(theApp.m_pDebuginfo, "str: %s\n", DecodeUnicodeString(entry->Entries[Name].Text));
			len = UnicodeLength(entry->Entries[Name].Text);
			EncodeDefault(name, entry->Entries[Name].Text, &len, true, NULL);
			break;
		}
	} else {
		smprintf(theApp.m_pDebuginfo, "WARNING: No usable name found!\n");
		len = 0;
	}

	if (Number != -1) {
		GSM_PackSemiOctetNumber(entry->Entries[Number].Text, number, false);
		NumberType = number[0];
		sprintf((char*)number,"%s",DecodeUnicodeString(entry->Entries[Number].Text));
	} else {
		smprintf(theApp.m_pDebuginfo, "WARNING: No usable number found!\n");
		number[0] = 0;
	}

	if (theApp.m_MemoryInfo.FirstMemoryEntry == 0) {
		error = ATGEN_GetMemoryInfo( NULL,&theApp.m_MemoryInfo, AT_First,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;
	}

	/* We can't use here:
	 * sprintf(req, "AT+CPBW=%d, \"%s\", %i, \"%s\"\r",
	 *         entry->Location, number, NumberType, name);
	 * because name can contain 0 when using GSM alphabet.
	 */

	if(bWithSpace)
	{
		if(bWithIndex)
			sprintf((char*)req, "AT+CPBW=%d, \"%s\", %i, \"", atoi(entry->szIndex) + theApp.m_MemoryInfo.FirstMemoryEntry - 1, number, NumberType);
		else
			sprintf((char*)req, "AT+CPBW=, \"%s\", %i, \"", number, NumberType);
	}
	else
	{
		if(bWithIndex)
			sprintf((char*)req, "AT+CPBW=%d,\"%s\",%i,\"", atoi(entry->szIndex) + theApp.m_MemoryInfo.FirstMemoryEntry - 1, number, NumberType);
		else
			sprintf((char*)req, "AT+CPBW=,\"%s\",%i,\"",  number, NumberType);
	}
	reqlen = strlen((char*)req);
	if (reqlen + len > REQUEST_SIZE - 2) {
		smprintf(theApp.m_pDebuginfo, "WARNING: Text truncated to fit in buffer!\n");
		len = REQUEST_SIZE - 2 - reqlen;
	}
	memcpy(req + reqlen, name, len);
	reqlen += len;
	memcpy(req + reqlen, "\"\r", 2);
	reqlen += 2;

	smprintf(theApp.m_pDebuginfo, "Writing phonebook entry\n");

	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBW");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return pWriteCommandfn ((unsigned char *)req, reqlen, 0x00, 20, false,NULL,&ReplymsgType,ATGEN_ReplySetMemory);
#undef REQUEST_SIZE
}
GSM_Error ATGEN_SetMemory(GSM_MemoryEntry *entry,int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	return ATGEN_SetMemoryEx(entry,true,true,Control,pWriteCommandfn,pDebuginfo);
}
GSM_Error ATGEN_SetMemoryEx(GSM_MemoryEntry *entry,bool bWithSpace,bool bWithIndex,int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	if( Control == Type_End ) return ERR_NONE;
//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;
//	if (entry->Location == 0) return ERR_INVALIDLOCATION;
	int nIndex = atoi(entry->szIndex);
	if(strlen(entry->szIndex) <=0 || nIndex==0) return ERR_INVALIDLOCATION;

	if(theApp.m_ManufacturerID == AT_Sharp){
		return ATGEN_PrivSetMemory4Sharp(entry,pWriteCommandfn,pDebuginfo);
	}else{
		return ATGEN_PrivSetMemoryEx(entry,bWithSpace,bWithIndex,pWriteCommandfn,pDebuginfo);
	}
}

GSM_Error ATGEN_DeleteMemory(GSM_MemoryEntry *entry , int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error 		error;

	unsigned char		req[100];

//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;
	if( Control==Type_End) return ERR_NONE;
//	if (entry->Location < 1) return ERR_INVALIDLOCATION;
	if (atoi(entry->szIndex) < 1) return ERR_INVALIDLOCATION;

	error = ATGEN_SetPBKMemory(entry->MemoryType,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

	if (theApp.m_MemoryInfo.FirstMemoryEntry == 0) {
		error = ATGEN_GetMemoryInfo(NULL,&theApp.m_MemoryInfo, AT_First,pWriteCommandfn,pDebuginfo);;
		if (error != ERR_NONE) return error;
	}


	sprintf((char*)req, "AT+CPBW=%d\r",atoi(entry->szIndex) + theApp.m_MemoryInfo.FirstMemoryEntry - 1);

	smprintf(theApp.m_pDebuginfo, "Deleting phonebook entry\n");

	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBW");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return pWriteCommandfn (req, strlen((char*)req), 0x00, 20, false,NULL,&ReplymsgType,ATGEN_ReplySetMemory);
}
GSM_Error ATGEN_InitPBKGet(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error error;
	char			req[20];
	int			start;
	int         max_no;
	int end;

	error = ATGEN_SetPBKMemory(Status->MemoryType,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

	theApp.m_MemoryStatus=Status;

	theApp.m_MemoryInfo.MemorySize		= 0;
	theApp.m_MemoryInfo.TextLength		= 0;
	theApp.m_MemoryInfo.NumberLength		= 0;
 

	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBR=?");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"AT+CPBR=?\r", 10, 0x00, 8, false,NULL,&ReplymsgType,ATGEN_ReplyGetCPBRMemoryInfo);

	if (error != ERR_NONE) return error;

	max_no = theApp.m_MemoryInfo.MemorySize;

	theApp.m_MemoryStatus	= Status;
	Status->MemoryUsed		= 0;
	Status->MemoryFree		= 0;
	start				= theApp.m_MemoryInfo.FirstMemoryEntry;
	theApp.m_NextMemoryEntry		= 0;
	theApp.m_MemoryInfo.MemorySize += start-1;
	theApp.m_PHKNum = 0;
	remove(theApp.m_szPHKTempFile);

	ZeroMemory(&theApp.m_PBKMemoryEntry,sizeof(GSM_MemoryEntry));

	end =  max_no/2;
	sprintf(req, "AT+CPBR=%i,%i\r", start, start + end);

	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBR=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;

	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"+CPBR:");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;

	GSM_ATMultiAnwser ATMultiAnwser;
	wsprintf(ATMultiAnwser.Specialtext,"+CPBR:");
	ATMultiAnwser.Anwserlines = 1;

	error = pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,&ATMultiAnwser,&ReplymsgType,ATGEN_ReplyGetMutiMemory);

	if (error != ERR_NONE && error != ERR_EMPTY) return error;
	
	sprintf(req, "AT+CPBR=%i,%i\r", start + end+1, theApp.m_MemoryInfo.MemorySize);

	error = pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,&ATMultiAnwser,&ReplymsgType,ATGEN_ReplyGetMutiMemory);
	if (error != ERR_NONE && error != ERR_EMPTY) return error;


	Status->MemoryFree = max_no - theApp.m_PHKNum;
	Status->MemoryUsed = theApp.m_PHKNum;
	return ERR_NONE;
	
	
}
void ATGEN_DecodePHKString (unsigned char *dest, unsigned char *buffer)
{
	unsigned char		buffer2[500];
	switch (theApp.m_PBKCharset) 
	{
	case AT_PBK_HEX:
		DecodeHexBin(buffer2,buffer+1,strlen((char*)buffer)-2);
 		DecodeDefault(dest,buffer2,strlen((char*)buffer2),false,NULL);
		break;
	case AT_PBK_GSM:
 		DecodeDefault(dest,buffer+1,strlen((char*)buffer)-2,false,NULL);
		break;			
	case AT_PBK_UTF8:

		DecodeUTF8ToUnicode(dest,buffer+1,strlen((char*)buffer)-2); //for _UNICODE
		break;			
	case AT_PBK_UCS2:

		DecodeHexUnicode(dest,buffer,strlen((char*)buffer) - 1);

		break;			
	case AT_PBK_PCCP437:
		/* FIXME: correctly decode PCCP437 */
 		DecodeDefault(dest,buffer+1,strlen((char*)buffer)-2,false,NULL);
		break;
	}
}

unsigned char *ATGEN_EncodePHKString (const unsigned char *src,int* len)
{
 	static char szdest[1024];

	switch (theApp.m_PBKCharset) {
	case AT_PBK_HEX:
		EncodeHexBin((unsigned char *)szdest, DecodeUnicodeString(src), UnicodeLength(src));
		*len = strlen(szdest);
		break;
	case AT_PBK_GSM:
		smprintf(theApp.m_pDebuginfo, "str: %s\n", DecodeUnicodeString(src));
		*len = UnicodeLength(src);
		EncodeDefault((unsigned char *)szdest, src, len, true, NULL);
		break;
	case AT_PBK_UTF8:

		*len=EncodeUnicode2UTF8((unsigned char *)src,(unsigned char *)szdest);// for _UNICODE
		*len = strlen(szdest);
		break;
	case AT_PBK_UCS2:
		EncodeHexUnicode((unsigned char *)szdest, src, UnicodeLength(src));
		*len = strlen(szdest);
		break;
	case AT_PBK_PCCP437:
		/* FIXME: correctly decode PCCP437 */
		smprintf(theApp.m_pDebuginfo, "str: %s\n", DecodeUnicodeString(src));
		*len = UnicodeLength(src);
		EncodeDefault((unsigned char *)szdest, src, len, true, NULL);
		break;
	}
	return (unsigned char *)szdest;
}
GSM_Error ATGEN_InitPBKGetEx(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo,
							  int (*pGetStatusfn)(int nCur,int nTotal))
{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error error;
	char			req[20];
	int			start;
	int         max_no;
	int end;

	error = ATGEN_SetPBKMemory(Status->MemoryType,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

	theApp.m_MemoryStatus=Status;

	theApp.m_MemoryInfo.MemorySize		= 0;
	theApp.m_MemoryInfo.TextLength		= 0;
	theApp.m_MemoryInfo.NumberLength		= 0;
 

	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBR=?");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"AT+CPBR=?\r", 10, 0x00, 8, false,NULL,&ReplymsgType,ATGEN_ReplyGetCPBRMemoryInfo);

	if (error != ERR_NONE) return error;

	max_no = theApp.m_MemoryInfo.MemorySize;

	theApp.m_MemoryStatus	= Status;
	Status->MemoryUsed		= 0;
	Status->MemoryFree		= 0;
	start				= theApp.m_MemoryInfo.FirstMemoryEntry;
	theApp.m_NextMemoryEntry		= 0;
	theApp.m_MemoryInfo.MemorySize += start-1;
	theApp.m_PHKNum = 0;
	remove(theApp.m_szPHKTempFile);

	ZeroMemory(&theApp.m_PBKMemoryEntry,sizeof(GSM_MemoryEntry));

	int nFirstIndex = start;
	int nCotiuneGet = true;
	while (nCotiuneGet && start <= theApp.m_MemoryInfo.MemorySize) 
	{
		end	= start + 100;
		if (end > theApp.m_MemoryInfo.MemorySize) end = theApp.m_MemoryInfo.MemorySize;
		sprintf(req, "AT+CPBR=%i,%i\r", start, end);
		ReplymsgType.nCount = 2;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBR=");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;

		wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"+CPBR:");
		ReplymsgType.CheckInfo[1].subtypechar = 0;
		ReplymsgType.CheckInfo[1].subtype = 0x00;

		GSM_ATMultiAnwser ATMultiAnwser;
		wsprintf(ATMultiAnwser.Specialtext,"+CPBR:");
		ATMultiAnwser.Anwserlines = 1;

		error = pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,&ATMultiAnwser,&ReplymsgType,ATGEN_ReplyGetMutiMemory);
		if (error != ERR_NONE && error != ERR_EMPTY) return error;
		if(pGetStatusfn)
			nCotiuneGet = pGetStatusfn(end-nFirstIndex,max_no);

		start = end + 1;
	}
	if(!nCotiuneGet) return ERR_CANCELED;
	Status->MemoryFree = max_no - theApp.m_PHKNum;
	Status->MemoryUsed = theApp.m_PHKNum;
	return ERR_NONE;
	
}

GSM_Error ATGEN_InitPBKGetEx2(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo,
							  int (*pGetStatusfn)(int nCur,int nTotal), int nMax)
{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error error;
	char			req[20];
	int			start;
	int         max_no;
	int end;

	error = ATGEN_SetPBKMemory(Status->MemoryType,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

	theApp.m_MemoryStatus=Status;

	theApp.m_MemoryInfo.MemorySize		= 0;
	theApp.m_MemoryInfo.TextLength		= 0;
	theApp.m_MemoryInfo.NumberLength		= 0;
 
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBR=?");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"AT+CPBR=?\r", 10, 0x00, 8, false,NULL,&ReplymsgType,ATGEN_ReplyGetCPBRMemoryInfo);

	if (error != ERR_NONE) return error;
	max_no = theApp.m_MemoryInfo.MemorySize;

	theApp.m_MemoryStatus	= Status;
	Status->MemoryUsed		= 0;
	Status->MemoryFree		= 0;
	start				= theApp.m_MemoryInfo.FirstMemoryEntry;
	theApp.m_NextMemoryEntry		= 0;
	theApp.m_MemoryInfo.MemorySize += start-1;
	theApp.m_PHKNum = 0;
	remove(theApp.m_szPHKTempFile);

	ZeroMemory(&theApp.m_PBKMemoryEntry,sizeof(GSM_MemoryEntry));

	int nFirstIndex = start;
	int nCotiuneGet = true;
	while (nCotiuneGet && start <= theApp.m_MemoryInfo.MemorySize) 
	{
		if(nMax==1){
			sprintf(req, "AT+CPBR=%i\r", start++);
		}else{
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

		GSM_ATMultiAnwser ATMultiAnwser;
		wsprintf(ATMultiAnwser.Specialtext,"+CPBR:");
		ATMultiAnwser.Anwserlines = 1;

		error = pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,&ATMultiAnwser,&ReplymsgType,ATGEN_ReplyGetMutiMemory);
		if (error != ERR_NONE&& error != ERR_EMPTY) return error;
		if(pGetStatusfn)
			nCotiuneGet = pGetStatusfn(end-nFirstIndex,max_no);

		if(nMax!=1){
			start = end + 1;
		}
	}
	if(!nCotiuneGet) return ERR_CANCELED;
	Status->MemoryFree = max_no - theApp.m_PHKNum;
	Status->MemoryUsed = theApp.m_PHKNum;
	return ERR_NONE;
	
}
// parse AT+CPBS?...+CPBS:"SM",10,200...OK.
GSM_Error ATGEN_ReplyGetCPBSMemoryInfo(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);
	char 			*pos;
 
 	switch (Replynfo.ReplyState) 
	{
 	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "Memory info received\n");
		// parse memory used
		pos = strchr((char*) msg.Buffer, ',');
		if (!pos) return ERR_UNKNOWN;
		pos++;
		theApp.m_MemoryStatus->MemoryUsed=atoi(pos);

		// parse total memory
		pos=strchr(pos, ',');
		if (!pos) return ERR_UNKNOWN;
		pos++;
		theApp.m_MemoryStatus->MemoryFree=atoi(pos) - theApp.m_MemoryStatus->MemoryUsed;

		return ERR_NONE;
	case AT_Reply_Error:
		return ERR_UNKNOWN;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(Replynfo);
 	default:
		return ERR_UNKNOWNRESPONSE;
	}
}

GSM_Error ATGEN_GetMemoryStatus4Sharp(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error error;
	GSM_Reply_MsgType ReplymsgType;
	theApp.m_MemoryStatus=Status;

	theApp.m_MemoryInfo.FirstMemoryEntry= 1;
	theApp.m_MemoryInfo.MemorySize		= 0;
	theApp.m_MemoryInfo.TextLength		= 0;
	theApp.m_MemoryInfo.NumberLength	= 0;

	error = ATGEN_SetPBKMemory(Status->MemoryType,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBS?");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error= pWriteCommandfn ((unsigned char *)"AT+CPBS?\r", 9, 0x00, 8, false,NULL,&ReplymsgType,ATGEN_ReplyGetCPBSMemoryInfo);
	if(error!=ERR_NONE)	return error;

	theApp.m_MemoryInfo.MemorySize		= Status->MemoryUsed+Status->MemoryFree;

	return error;
}
GSM_Error ATGEN_InitPBKGet4Sharp(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error error;
	char			req[20];
	int			start;
	int         max_no;
	int end;

	theApp.m_MemoryStatus=Status;

	error = ATGEN_SetPBKMemory(Status->MemoryType,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

	Status->MemoryUsed		= 0;
	Status->MemoryFree		= 0;
	error=ATGEN_GetMemoryStatus4Sharp(Status, pWriteCommandfn, pDebuginfo);
	if (error != ERR_NONE) return error;

	start	= 1;
	max_no	= theApp.m_MemoryInfo.MemorySize;
	end		=  max_no/2;

	theApp.m_NextMemoryEntry		= 0;
	theApp.m_PHKNum = 0;
	remove(theApp.m_szPHKTempFile);
	ZeroMemory(&theApp.m_PBKMemoryEntry,sizeof(GSM_MemoryEntry));

	sprintf(req, "AT+CPBR=%i,%i\r", start, start + end);
	ReplymsgType.nCount = 2;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBR=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;

	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"+CPBR:");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;

	GSM_ATMultiAnwser ATMultiAnwser;
	wsprintf(ATMultiAnwser.Specialtext,"+CPBR:");
	ATMultiAnwser.Anwserlines = 1;

	error = pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,&ATMultiAnwser,&ReplymsgType,ATGEN_ReplyGetMutiMemory);
	if (error != ERR_NONE) return error;
	
	sprintf(req, "AT+CPBR=%i,%i\r", start + end+1, theApp.m_MemoryInfo.MemorySize);
	error = pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 40, false,&ATMultiAnwser,&ReplymsgType,ATGEN_ReplyGetMutiMemory);
	if (error != ERR_NONE) return error;
	Status->MemoryFree = max_no - theApp.m_PHKNum;
	Status->MemoryUsed = theApp.m_PHKNum;
	return ERR_NONE;	
}