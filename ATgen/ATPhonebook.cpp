
/* (c) 2002-2005 by Marcin Wiacek and Michal Cihar */

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
			if (pos == NULL)//return ERR_UNKNOWN;//peggy
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

//	error = GSM_WaitFor (s, "AT+CPBR=?\r", 10, 0x00, 4, ID_GetMemoryStatus);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBR=?");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"AT+CPBR=?\r", 10, 0x00, 8, false,NULL,&ReplymsgType,ATGEN_ReplyGetCPBRMemoryInfo);

	memcpy(pMemoryInfo,&theApp.m_MemoryInfo,sizeof(GSM_MemoryInfo));
	// v1.1.0.1 for SE F500i "power on and run PB first " , added by mingfa
	/* it's useless to fix F500i bug
	if ( error!= ERR_NONE)
	{
	error=GSM_WaitFor (s, "AT+CGMM\r", 8, 0x00, 3, ID_GetModel);
	Sleep(1000);
	error = GSM_WaitFor (s, "AT+CPBR=?\r", 10, 0x00, 4, ID_GetMemoryStatus);
	}
	*/

//	if (Priv->Manufacturer == AT_Samsung)
//		error = GSM_WaitFor (s, "", 0, 0x00, 4, ID_GetMemoryStatus);
	if (error != ERR_NONE) return error;
//	max_no = xParseMaximunMemory(s->Phone.Data.RequestMsg->Buffer);
	max_no = pMemoryInfo->MemorySize;
	if (NeededInfo == AT_Total || NeededInfo == AT_Sizes || NeededInfo == AT_First) return ERR_NONE;

	smprintf(theApp.m_pDebuginfo, "Getting memory status by reading values\n");
	
	theApp.m_MemoryStatus	= Status;
	Status->MemoryUsed		= 0;
	Status->MemoryFree		= 0;
	start				= pMemoryInfo->FirstMemoryEntry;
	theApp.m_NextMemoryEntry		= 0;
	pMemoryInfo->MemorySize += start-1; //Jay add for Motorola comparion start and end
	while (1) {
		end	= start + 20;
		if (end > pMemoryInfo->MemorySize) end = pMemoryInfo->MemorySize;
		sprintf(req, "AT+CPBR=%i,%i\r", start, end);
	//	error	= GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_GetMemoryStatus);
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
	/*	if( strlen(s->Phone.Data.RequestMsg->Buffer ) == 0x14 )
		{
			Status->MemoryFree = max_no - Status->MemoryUsed;
			return ERR_NONE;
		}*/
		if (NeededInfo == AT_NextEmpty && theApp.m_NextMemoryEntry != 0 && theApp.m_NextMemoryEntry != end + 1)
			return ERR_NONE;
		if (end == pMemoryInfo->MemorySize) {
	//		Status->MemoryFree = Priv->MemorySize - Status->MemoryUsed;//peggy -
			Status->MemoryFree = max_no - Status->MemoryUsed; //peggy +
			return ERR_NONE;
		}
		start = end + 1;
	}
}



GSM_Error ATGEN_GetMemoryStatus(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error error;
	theApp.m_pDebuginfo = pDebuginfo;

//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;
	error = ATGEN_SetPBKMemory(Status->MemoryType,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

//	s->Phone.Data.MemoryStatus=Status;

	/* in some phones doesn't work or doesn't return memory status inside */
	/* Some workaround for buggy mobile, that hangs after "AT+CPBS?" for other
	 * memory than SM.
	 */
/*	if (!IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_BROKENCPBS) || (Status->MemoryType == MEM_SM)) {
		smprintf(theApp.m_pDebuginfo, "Getting memory status\n");
		error=GSM_WaitFor (s, "AT+CPBS?\r", 9, 0x00, 4, ID_GetMemoryStatus);
		if (error == ERR_NONE) return ERR_NONE;
	}*/

//	return ATGEN_GetMemoryInfo(s, Status, AT_Status); //peggy -
	Status->MemoryUsed		= -1;//peggy +
	Status->MemoryFree		= -1;//peggy +
	error =  ATGEN_GetMemoryInfo(Status,&theApp.m_MemoryInfo, AT_Total,pWriteCommandfn,pDebuginfo);//peggy +

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

//	if (entry->Location==0x00) return ERR_INVALIDLOCATION;
	int nIndex = atoi(entry->szIndex);
	if(strlen(entry->szIndex) <=0 || nIndex==0) return ERR_INVALIDLOCATION;

	if (entry->MemoryType == MEM_ME)
	{
		if (theApp.m_PBKSBNR == 0) {
			sprintf((char*)req, "AT^SBNR=?\r");
			smprintf(theApp.m_pDebuginfo, "Checking availablity of SBNR\n");
	//		error=GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_GetMemory);
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
		//	sprintf((char*)req, "AT^SBNR=vcf,%i\r",entry->Location-1);
			sprintf((char*)req, "AT^SBNR=vcf,%i\r",nIndex-1);
			theApp.m_Memory=entry;
			smprintf(theApp.m_pDebuginfo, "Getting phonebook entry\n");
		//	return GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_GetMemory);
			ReplymsgType.nCount = 1;
			wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT^SBNR");
			ReplymsgType.CheckInfo[0].subtypechar = 0;
			ReplymsgType.CheckInfo[0].subtype = 0x00;
			return pWriteCommandfn ((unsigned char *)req, strlen((char*)req), 0x00, 8, false,NULL,&ReplymsgType,SIEMENS_ReplyGetMemory);
		}
	}
/*
	error=ATGEN_GetManufacturer(s);
	if (error != ERR_NONE) return error;

	error=ATGEN_SetPBKMemory(s, entry->MemoryType);
	if (error != ERR_NONE) return error;

	if (Priv->FirstMemoryEntry == 0) {
		error = ATGEN_GetMemoryInfo(s, NULL, AT_First);
		if (error != ERR_NONE) return error;
	}

	//Remove to Initial
	//error=ATGEN_SetPBKCharset(s, true); /* For reading we prefer unicode */
	//if (error != ERR_NONE) return error;
	//----------------------
/*	if (endlocation == 0) {
		sprintf(req, "AT+CPBR=%i\r", entry->Location + Priv->FirstMemoryEntry - 1);
	} else {
		sprintf(req, "AT+CPBR=%i,%i\r", entry->Location + Priv->FirstMemoryEntry - 1, endlocation + Priv->FirstMemoryEntry - 1);
	}

	s->Phone.Data.Memory=entry;
	smprintf(theApp.m_pDebuginfo, "Getting phonebook entry\n");
	return GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_GetMemory);
	*/
	if (endlocation == 0)
	{
//		nLocation = entry->Location + Priv->FirstMemoryEntry - 1;
//		nLocation = entry->Location -1;
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
//	unsigned char		uname[2*(GSM_PHONEBOOK_TEXT_LENGTH + 1)];
	unsigned char		number[GSM_PHONEBOOK_TEXT_LENGTH + 1];
	int			reqlen;
	bool			PreferUnicode = false;

//	if (entry->Location == 0) return ERR_INVALIDLOCATION;
	int nIndex = atoi(entry->szIndex);
	if(strlen(entry->szIndex) <=0 || nIndex==0) return ERR_INVALIDLOCATION;

	error = ATGEN_SetPBKMemory(entry->MemoryType,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;  // Put to Initialize

	GSM_PhonebookFindDefaultNameNumberGroup(entry, &Name, &Number, &Group);

	name[0] = 0;
	if (Name != -1) {
/* peggy +
		len = UnicodeLength(entry->Entries[Name].Text);

		/* Compare if we would loose some information when not using
		 * unicode */
/* peggy +
		EncodeDefault(name, entry->Entries[Name].Text, &len, true, NULL);
		DecodeDefault(uname, name, len, true, NULL);
		if (!mywstrncmp(uname, entry->Entries[Name].Text, len)) {
			/* Get maximal text length */
/* peggy +

			if (Priv->TextLength == 0) {
				ATGEN_GetMemoryInfo(s, NULL, AT_Sizes);
			}
			
			/* I char stored in GSM alphabet takes 7 bits, one
			 * unicode 16, if storing in unicode would truncate
			 * text, do not use it, otherwise we will use it */
/* peggy +
			if ((Priv->TextLength != 0) && ((Priv->TextLength * 7 / 16) <= len)) {
				PreferUnicode = false;
			} else {
				PreferUnicode = true;
			}
		}
*/ //peggy +
		PreferUnicode = true;//peggy +
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
			//DecodeUcs2();
		//	len=EncodeCString2UTF8(DecodeUnicodeString(entry->Entries[Name].Text),name);
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
//	sprintf((char*)req, "AT+CPBW=%d, \"%s\", %i, \"", entry->Location + theApp.m_MemoryInfo.FirstMemoryEntry - 1, number, NumberType);
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
//	return GSM_WaitFor (s, req, reqlen, 0x00, 4, ID_SetMemory);
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
//	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
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

//	sprintf((char*)req, "AT+CPBW=%d\r",entry->Location + theApp.m_MemoryInfo.FirstMemoryEntry - 1);
	sprintf((char*)req, "AT+CPBW=%d\r",atoi(entry->szIndex) + theApp.m_MemoryInfo.FirstMemoryEntry - 1);

	smprintf(theApp.m_pDebuginfo, "Deleting phonebook entry\n");
//	return GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_SetMemory);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CPBW");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return pWriteCommandfn (req, strlen((char*)req), 0x00, 20, false,NULL,&ReplymsgType,ATGEN_ReplySetMemory);
}







