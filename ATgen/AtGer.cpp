/* (c) 2002-2005 by Marcin Wiacek and Michal Cihar */


static ATErrorCode CMSErrorCodes[] = {
	/*
	 * Error codes not specified here were either undefined or reserved in my
	 * copy of specifications, if you have newer one, please fill in the gaps.
	 */
	/* 0...127 from GSM 04.11 Annex E-2 */
	{1,    "Unassigned (unallocated) number"},
	{8,    "Operator determined barring"},
	{10,   "Call barred"},
	{21,   "Short message transfer rejected"},
	{27,   "Destination out of service"},
	{28,   "Unidentified subscriber"},
	{29,   "Facility rejected"},
	{30,   "Unknown subscriber"},
	{38,   "Network out of order"},
	{41,   "Temporary failure"},
	{42,   "Congestion"},
	{47,   "Resources unavailable, unspecified"},
	{50,   "Requested facility not subscribed"},
	{69,   "Requested facility not implemented"},
	{81,   "Invalid short message transfer reference value"},
	{95,   "Invalid message, unspecified"},
	{96,   "Invalid mandatory information"},
	{97,   "Message type non-existent or not implemented"},
	{98,   "Message not compatible with short message protocol state"},
	{99,   "Information element non-existent or not implemented"},
	{111,  "Protocol error, unspecified"},
	{127,  "Interworking, unspecified"},
	/* 128...255 from GSM 03.40 subclause 9.2.3.22 */
	{0x80, "Telematic interworking not supported"},
	{0x81, "Short message Type 0 not supported"},
	{0x82, "Cannot replace short message"},
	{0x8F, "Unspecified TP-PID error"},
	{0x90, "Data coding scheme (alphabet) not supported"},
	{0x91, "Message class not supported"},
	{0x9F, "Unspecified TP-DCS error"},
	{0xA0, "Command cannot be actioned"},
	{0xA1, "Command unsupported"},
	{0xAF, "Unspecified TP-Command error"},
	{0xB0, "TPDU not supported"},
	{0xC0, "SC busy"},
	{0xC1, "No SC subscription"},
	{0xC2, "SC system failure"},
	{0xC3, "Invalid SME address"},
	{0xC4, "Destination SME barred"},
	{0xC5, "SM Rejected-Duplicate SM"},
	{0xC6, "TP-VPF not supported"},
	{0xC7, "TP-VP not supported"},
	{0xD0, "SIM SMS storage full"},
	{0xD1, "No SMS storage capability in SIM"},
	{0xD2, "Error in MS"},
	{0xD3, "Memory Capacity Exceede"},
	{0xD4, "SIM Application Toolkit Busy"},
	{0xFF, "Unspecified error cause"},
	/* 300...511 from GSM 07.05 subclause 3.2.5 */
	{300,  "ME failure"},
	{301,  "SMS service of ME reserved"},
	{302,  "operation not allowed"},
	{303,  "operation not supported"},
	{304,  "invalid PDU mode parameter"},
	{305,  "invalid text mode parameter"},
	{310,  "SIM not inserted"},
	{311,  "SIM PIN required"},
	{312,  "PH-SIM PIN required"},
	{313,  "SIM failure"},
	{314,  "SIM busy"},
	{315,  "SIM wrong"},
	{316,  "SIM PUK required"},
	{317,  "SIM PIN2 required"},
	{318,  "SIM PUK2 required"},
	{320,  "memory failure"},
	{321,  "invalid memory index"},
	{322,  "memory full"},
	{330,  "SMSC address unknown"},
	{331,  "no network service"},
	{332,  "network timeout"},
	{340,  "no CNMA acknowledgement expected"},
	{500,  "unknown error"},
	/* > 512 are manufacturer specific according to GSM 07.05 subclause 3.2.5 */
	{-1,   ""}
};
static ATErrorCode CMEErrorCodes[] = {
	/* CME Error codes from GSM 07.07 section 9.2 */
	{0,   "phone failure"},
	{1,   "no connection to phone"},
	{2,   "phone-adaptor link reserved"},
	{3,   "operation not allowed"},
	{4,   "operation not supported"},
	{5,   "PH-SIM PIN required"},
	{10,  "SIM not inserted"},
	{11,  "SIM PIN required"},
	{12,  "SIM PUK required"},
	{13,  "SIM failure"},
	{14,  "SIM busy"},
	{15,  "SIM wrong"},
	{16,  "incorrect password"},
	{17,  "SIM PIN2 required"},
	{18,  "SIM PUK2 required"},
	{20,  "memory full"},
	{21,  "invalid index"},
	{22,  "not found"},
	{23,  "memory failure"},
	{24,  "text string too long"},
	{25,  "invalid characters in text string"},
	{26,  "dial string too long"},
	{27,  "invalid characters in dial string"},
	{30,  "no network service"},
	{31,  "network timeout"},
	{100, "unknown"},
	{-1,   ""}
};
void ATGEN_DecodeDateTime(GSM_DateTime *dt, unsigned char *input)
{
	/* Samsung phones report year as %d instead of %02d */
	if (input[4] == '/') {
		dt->Year=(*input-'0')*1000;
		input++;
		dt->Year=dt->Year+(*input-'0')*100;
		input++;
		dt->Year=dt->Year+(*input-'0')*10;
		input++;
		dt->Year=dt->Year-2000;  
	}
	else if (input[2] == '/') {
		dt->Year=(*input-'0')*10;
		input++;
	} else {
		dt->Year=0;
	}

	dt->Year=dt->Year+(*input-'0');    input++;
	dt->Year+=2000;

	input++;
	dt->Month=(*input-'0')*10;         input++;
	dt->Month=dt->Month+(*input-'0');  input++;

	input++;
	dt->Day=(*input-'0')*10;           input++;
	dt->Day=dt->Day+(*input-'0');      input++;

	input++;
	dt->Hour=(*input-'0')*10;          input++;
	dt->Hour=dt->Hour+(*input-'0');    input++;

	input++;
	dt->Minute=(*input-'0')*10;        input++;
	dt->Minute=dt->Minute+(*input-'0');input++;

	input++;
	dt->Second=(*input-'0')*10;        input++;
	dt->Second=dt->Second+(*input-'0');input++;

	if (input!=NULL) {
		input++;
		dt->Timezone=(*input-'0')*10;          input++;
		dt->Timezone=dt->Timezone+(*input-'0');input++;
		input=input-3;
		if (*input=='-') dt->Timezone=-dt->Timezone;
	}
}


GSM_Error ATGEN_HandleCMSError(GSM_ATReplayInfo Replynfo)
{

	if (Replynfo.ErrorCode == 0) {
		smprintf(theApp.m_pDebuginfo, "CMS Error occured, but it's type not detected\n");
	} else if (strlen(Replynfo.ErrorText )== 0) {
		smprintf(theApp.m_pDebuginfo, "CMS Error %i, no description available\n", Replynfo.ErrorCode);
	} else {
		smprintf(theApp.m_pDebuginfo, "CMS Error %i: \"%s\"\n", Replynfo.ErrorCode, Replynfo.ErrorText);
	}
	/* For error codes descriptions see table a bit above */
	switch (Replynfo.ErrorCode) {
	case 304:
            	return ERR_NOTSUPPORTED; 
        case 305:
            	return ERR_BUG; 
        case 311:
        case 312:
        case 316:
        case 317:
        case 318:
            	return ERR_SECURITYERROR;
        case 322:
            	return ERR_FULL;
        case 321:
            	return ERR_INVALIDLOCATION;
        default:
		return ERR_UNKNOWN;
	}
}

GSM_Error ATGEN_HandleCMEError(GSM_ATReplayInfo Replynfo)
{

	if (Replynfo.ErrorCode == 0) {
		smprintf(theApp.m_pDebuginfo, "CME Error occured, but it's type not detected\n");
	} else if (strlen(Replynfo.ErrorText )== 0) {
		smprintf(theApp.m_pDebuginfo, "CME Error %i, no description available\n", Replynfo.ErrorCode);
	} else {
		smprintf(theApp.m_pDebuginfo, "CME Error %i: \"%s\"\n", Replynfo.ErrorCode, Replynfo.ErrorText);
	}
	/* For error codes descriptions see table a bit above */
	switch (Replynfo.ErrorCode) {
		case -1:
			return ERR_EMPTY;
		case 3:
			return ERR_PERMISSION;
		case 4:
			return ERR_NOTSUPPORTED;
		case 5:
		case 11:
		case 12:
		case 16:
		case 17:
		case 18:
			return ERR_SECURITYERROR;
		case 20:
			return ERR_FULL;
		case 21:
			return ERR_INVALIDLOCATION;
		case 22:
			return ERR_EMPTY;
		case 23:
			return ERR_MEMORY;
		case 24:
		case 25:
		case 26:
		case 27:
			return ERR_INVALIDDATA;
		default:
			return ERR_UNKNOWN;
	}
}
GSM_Error ATGEN_GenericReply(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);
	switch (Replynfo.ReplyState) 
	{
		case AT_Reply_OK:
		case AT_Reply_Connect:
		case AT_Reply_Entered:
			return ERR_NONE;
		case AT_Reply_Error:
			return ERR_UNKNOWN;
		case AT_Reply_CMSError:
			return ATGEN_HandleCMSError(Replynfo);
		case AT_Reply_CMEError:
			return ATGEN_HandleCMEError(Replynfo);
		default:
			break;
	}
	return ERR_UNKNOWNRESPONSE;
}



GSM_Error ATGEN_ReplyGetModel(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	if (Replynfo.ReplyState != AT_Reply_OK) return ERR_NOTSUPPORTED;

	if (strlen(GetLineString(msg.Buffer, theApp.m_Lines, 2)) <= MAX_MODEL_LENGTH)
	{
		CopyLineString((unsigned char *)theApp.m_pszTemp, msg.Buffer, theApp.m_Lines, 2);

		/* Sometimes phone adds this before manufacturer (Sagem) */
		if (strncmp("+CGMM: ", theApp.m_pszTemp, 7) == 0) {
			memmove(theApp.m_pszTemp, theApp.m_pszTemp + 7, strlen(theApp.m_pszTemp + 7) + 1);
		}
/*
		Data->ModelInfo = GetModelData(NULL,theApp.m_pszTemp,NULL);
		if (Data->ModelInfo->number[0] == 0) Data->ModelInfo = GetModelData(NULL,NULL,theApp.m_pszTemp);
		if (Data->ModelInfo->number[0] == 0) Data->ModelInfo = GetModelData(theApp.m_pszTemp,NULL,NULL);

		if (Data->ModelInfo->number[0] != 0) strcpy(theApp.m_pszTemp,Data->ModelInfo->number);

		if (strstr(msg.Buffer,"Nokia")) 
		{
		  theApp.m_ManufacturerID = AT_Nokia;
		}
		else if (strstr(msg.Buffer,"M20")) 
		{
	 	    theApp.m_ManufacturerID = AT_Siemens;
		    strcpy(theApp.m_pszTemp,"M20");
			strcpy(Data->ModelInfo->model,"M20");// mingfa
		}
		else if (strstr(msg.Buffer,"MC35"))
		{
			theApp.m_ManufacturerID = AT_Siemens;
			strcpy(theApp.m_pszTemp,"MC35");
			strcpy(Data->ModelInfo->model,"MC35");// mingfa
		}
		else if (strstr(msg.Buffer,"SL5C"))
		{
		    theApp.m_ManufacturerID = AT_Siemens;
		    strcpy(theApp.m_pszTemp,"SL5C");
			strcpy(Data->ModelInfo->model,"SL5C");// mingfa
		}
		else if (strstr(msg.Buffer,"SL6C"))
		{
			theApp.m_ManufacturerID = AT_Siemens;
			strcpy(theApp.m_pszTemp,"SL6C");
			strcpy(Data->ModelInfo->model,"SL6C");// mingfa
		}
		/* v1.1.0.1 ; don't support
		else if (strstr(msg.Buffer, "iPAQ"))
		{
			theApp.m_ManufacturerID = AT_HP;
			strcpy(theApp.m_pszTemp,"iPAQ");
			strcpy(Data->ModelInfo->model,"iPAQ");// mingfa
		}
		*/
	/**	else if (strstr(msg.Buffer,"1130102-CNT39mc"))
		{
			theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"T39");
			strcpy(Data->ModelInfo->model,"T39");// mingfa
		}
		else if (strstr(msg.Buffer,"1130201-BV"))
		{
		    theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"T68");
			strcpy(Data->ModelInfo->model,"T68");// mingfa
		}
		else if (strstr(msg.Buffer,"1021011-BV"))
		{
		    theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"T610");
			strcpy(Data->ModelInfo->model,"T610");// mingfa
		}
//		else if (strstr(msg.Buffer,"1021021-BV"))//mingfa;  1021021 is error
		else if (strstr(msg.Buffer,"1021012-BV"))
		{
		    theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"T630");
			strcpy(Data->ModelInfo->model,"T630");// mingfa
		}
		else if (strstr(msg.Buffer,"1021041-BV"))
		{
		    theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"K700i");
			strcpy(Data->ModelInfo->model,"K700i");// mingfa
		}
		else if (strstr(msg.Buffer,"1021043-BV"))
		{
			theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"K500i");
			strcpy(Data->ModelInfo->model,"K500i");// mingfa
		}
		else if (strstr(msg.Buffer,"1021051-CN"))
		{
			theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"S700i");
			strcpy(Data->ModelInfo->model,"S700i");// mingfa
		}
		else if (strstr(msg.Buffer,"1021031-BV"))
		{
		    theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"Z600");
			strcpy(Data->ModelInfo->model,"Z600");// mingfa
		}
		else if (strstr(msg.Buffer,"AAD-3011011-BV")) 
		{
			theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"Z1010");
			strcpy(Data->ModelInfo->model,"Z1010");// mingfa
		}
		else if (strstr(msg.Buffer,"1021011-CNP900")) 
		{
			theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"P900");
			strcpy(Data->ModelInfo->model,"P900");// mingfa
		}
		else if (strstr(msg.Buffer,"1021012-CNP910i")) 
		{
		    theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"P910i");
			strcpy(Data->ModelInfo->model,"P910i");// mingfa
		}
		else if (strstr(msg.Buffer,"AAD-3021011-BV"))
		{
			theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"V800");
			strcpy(Data->ModelInfo->model,"V800");// mingfa
		}
		else if (strstr(msg.Buffer,"1130601-BVT300"))//v1.1.0.1 mingfa
		{
			theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"T300");
			strcpy(Data->ModelInfo->model,"T300");
		}
		else if (strstr(msg.Buffer,"1001012-CNT230"))//v1.1.0.1 mingfa
		{
			theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"T230");
			strcpy(Data->ModelInfo->model,"T230");
		}
		else if (strstr(msg.Buffer,"AAB-1021042-BV")) //v1.1.0.1 mingfa
		{
			theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"F500i");
			strcpy(Data->ModelInfo->model,"F500i");
        }
		else if (strstr(msg.Buffer,"1001013-BVT290")) //v1.1.0.1 mingfa
		{
			theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"T290i");
			strcpy(Data->ModelInfo->model,"T290i");
        }
		/*don't support
		else if (strstr(msg.Buffer,"1130602-BVT310")) //??? v1.1.0.1 mingfa
		{
			theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"T310");
			strcpy(Data->ModelInfo->model,"T310");
        }
		*/
/**		else if (strstr(msg.Buffer,"AAB-1021071-BV")) // v1.1.0.1 mingfa
		{
			theApp.m_ManufacturerID = AT_Ericsson;
			strcpy(theApp.m_pszTemp,"K300i");
			strcpy(Data->ModelInfo->model,"K300i");
        }
		else if (strstr(msg.Buffer, "V501"))
		{
			theApp.m_ManufacturerID = AT_Motorola; //peggy
			strcpy(theApp.m_pszTemp,"V501");//peggy
            strcpy(Data->ModelInfo->model,"V501");// mingfa
		}
		else if (strstr(msg.Buffer, "V600i")) 
		{
			theApp.m_ManufacturerID = AT_Motorola; //peggy
			strcpy(theApp.m_pszTemp,"V600i"); //peggy
			strcpy(Data->ModelInfo->model,"V600i");// mingfa
		}
		else if (strstr(msg.Buffer, "V600")) 
		{
			theApp.m_ManufacturerID = AT_Motorola; //peggy
			strcpy(theApp.m_pszTemp,"V600"); //peggy
			strcpy(Data->ModelInfo->model,"V600");// mingfa
		}
		else if (strstr(msg.Buffer, "V80"))
		{
			theApp.m_ManufacturerID = AT_Motorola; //peggy
			strcpy(theApp.m_pszTemp,"V80"); //peggy
			strcpy(Data->ModelInfo->model,"V80");// mingfa
		}
		else if (strstr(msg.Buffer, "E398")) 
		{
			theApp.m_ManufacturerID = AT_Motorola; //peggy
			strcpy(theApp.m_pszTemp,"E398"); //peggy
			strcpy(Data->ModelInfo->model,"E398");// mingfa
		}
		else if (strstr(msg.Buffer, "V620")) 
		{
			theApp.m_ManufacturerID = AT_Motorola; //peggy
			strcpy(theApp.m_pszTemp,"V620"); //peggy
			strcpy(Data->ModelInfo->model,"V620");// mingfa
		}
		else if (strstr(msg.Buffer, "E550")) 
		{
			theApp.m_ManufacturerID = AT_Motorola; //peggy
			strcpy(theApp.m_pszTemp,"E550"); //peggy
			strcpy(Data->ModelInfo->model,"E550");// mingfa
		}
		else if (strstr(msg.Buffer, "V635")) 
		{
			theApp.m_ManufacturerID = AT_Motorola; //peggy
			strcpy(theApp.m_pszTemp,"V635"); //peggy
			strcpy(Data->ModelInfo->model,"V635");// mingfa
		}
		else if (strstr(msg.Buffer, "V500")) 
		{
		    theApp.m_ManufacturerID = AT_Motorola; //peggy
			strcpy(theApp.m_pszTemp,"V500"); //peggy
			strcpy(Data->ModelInfo->model,"V500");// mingfa
		}
		else if (strstr(msg.Buffer, "V550"))
		{
			theApp.m_ManufacturerID = AT_Motorola; //peggy
			strcpy(theApp.m_pszTemp,"V550"); //peggy
			strcpy(Data->ModelInfo->model,"V550");// mingfa
		}
		else if (strstr(msg.Buffer, "V525"))
		{
			theApp.m_ManufacturerID = AT_Motorola; //peggy
			strcpy(theApp.m_pszTemp,"V525"); //peggy
			strcpy(Data->ModelInfo->model,"V525");// mingfa
		}
		else if (strstr(msg.Buffer, "E1000"))
		{
			theApp.m_ManufacturerID = AT_Motorola; //peggy
			strcpy(theApp.m_pszTemp,"E1000"); //peggy
			strcpy(Data->ModelInfo->model,"E1000");// mingfa
		}
		else if (strstr(msg.Buffer, "V303"))
		{
			theApp.m_ManufacturerID = AT_Motorola; //mingfa
			strcpy(theApp.m_pszTemp,"V303"); //mingfa
			strcpy(Data->ModelInfo->model,"V303");// mingfa
		}
		else if (strstr(msg.Buffer, "V300")) // NOTE : V300 should be before V3, if not , model name will be "V3"
		{
			theApp.m_ManufacturerID = AT_Motorola; //mingfa
			strcpy(theApp.m_pszTemp,"V300"); //mingfa
			strcpy(Data->ModelInfo->model,"V300");// mingfa
		}
		else if (strstr(msg.Buffer, "V3"))
		{
			theApp.m_ManufacturerID = AT_Motorola; //peggy
			strcpy(theApp.m_pszTemp,"V3"); //peggy
			strcpy(Data->ModelInfo->model,"V3");// mingfa
		}
*/

	} else {
		smprintf(theApp.m_pDebuginfo, "WARNING: Model name too long, increase MAX_MODEL_LENGTH to at least %zd\n", strlen(GetLineString(msg.Buffer, theApp.m_Lines, 2)));
	}
	return ERR_NONE;
}
GSM_Error ATGEN_ReplyGetIMEI(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);
	CopyLineString((unsigned char *)theApp.m_pszTemp, msg.Buffer, theApp.m_Lines, 2);
	smprintf(theApp.m_pDebuginfo, "Received IMEI %s\n",theApp.m_pszTemp);
	return ERR_NONE;
}
GSM_Error ATGEN_ReplyGetDateTime_Alarm(GSM_Protocol_Message msg)
{
	int 			current	= 19;
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		if (msg.Buffer[current]==0x0d || msg.Buffer[current-1]==0x0d) {
			smprintf(theApp.m_pDebuginfo, "Not set in phone\n");
			return ERR_EMPTY;
		} else {
			ATGEN_DecodeDateTime(theApp.m_pdate_time, msg.Buffer+current);
			return ERR_NONE;
		}
	case AT_Reply_Error:
		return ERR_NOTSUPPORTED;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}
GSM_Error ATGEN_ReplyReset(GSM_Protocol_Message msg)
{
	smprintf(theApp.m_pDebuginfo, "Reset done\n");
	return ERR_NONE;
}
GSM_Error ATGEN_SetPBKUCS2Charset(ATCharsetInfo *pCharsetInfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error		error;
//	error=GSM_WaitFor (s, "AT+CSCS=\"UCS2\"\r", 15, 0x00, 3, ID_SetMemoryCharset);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CSCS=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"AT+CSCS=\"UCS2\"\r", 15, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);

	if (error == ERR_NONE) 
		theApp.m_PBKCharset = AT_PBK_UCS2;
	else
		theApp.m_UCS2CharsetFailed = true;
	if(pCharsetInfo)
	{
		pCharsetInfo->m_NonUCS2CharsetFailed = theApp.m_NonUCS2CharsetFailed;
		pCharsetInfo->m_PBKCharset = theApp.m_PBKCharset;
		pCharsetInfo->m_UCS2CharsetFailed = theApp.m_UCS2CharsetFailed;
	}
	return error;
}

GSM_Error ATGEN_SetPBKCharset(bool PreferUnicode,ATCharsetInfo *pCharsetInfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error		error;
	bool bSharpUTF8=false;

	/* Have we already selected something? */
	if (theApp.m_PBKCharset!=AT_PBK_ERROR) {
		/* If we want unicode charset and we have it already or setting of it
		 * failed, we have nothing to do. */
		if (PreferUnicode  && (theApp.m_PBKCharset==AT_PBK_UCS2 || theApp.m_UCS2CharsetFailed)) 
		{
			if(pCharsetInfo)
			{
				pCharsetInfo->m_NonUCS2CharsetFailed = theApp.m_NonUCS2CharsetFailed;
				pCharsetInfo->m_PBKCharset = theApp.m_PBKCharset;
				pCharsetInfo->m_UCS2CharsetFailed = theApp.m_UCS2CharsetFailed;
			}
			return ERR_NONE;
		}

		/* If we don't need unicode charset and we have some (or have unicode
		 * charset when other failed), we have nothing to do. */
		if (!PreferUnicode && (theApp.m_PBKCharset!=AT_PBK_UCS2 || theApp.m_NonUCS2CharsetFailed)) 
		{
			if(pCharsetInfo)
			{
				pCharsetInfo->m_NonUCS2CharsetFailed = theApp.m_NonUCS2CharsetFailed;
				pCharsetInfo->m_PBKCharset = theApp.m_PBKCharset;
				pCharsetInfo->m_UCS2CharsetFailed = theApp.m_UCS2CharsetFailed;
			}
			return ERR_NONE;
		}
	}
	if (theApp.m_ManufacturerID == AT_Unknown)
	{
		char szGetManufacturer[MAX_PATH];
		error=ATGEN_GetManufacturer(szGetManufacturer,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE)
		{
			if(pCharsetInfo)
			{
				pCharsetInfo->m_NonUCS2CharsetFailed = theApp.m_NonUCS2CharsetFailed;
				pCharsetInfo->m_PBKCharset = theApp.m_PBKCharset;
				pCharsetInfo->m_UCS2CharsetFailed = theApp.m_UCS2CharsetFailed;
			}
			return error;
		}
	}


	if (theApp.m_ManufacturerID == AT_Samsung) {
		// Bomber, 2005.07.27
		//theApp.m_PBKCharset = AT_PBK_PCCP437;
		theApp.m_PBKCharset = AT_PBK_UTF8;
		if(pCharsetInfo)
		{
			pCharsetInfo->m_NonUCS2CharsetFailed = theApp.m_NonUCS2CharsetFailed;
			pCharsetInfo->m_PBKCharset = theApp.m_PBKCharset;
			pCharsetInfo->m_UCS2CharsetFailed = theApp.m_UCS2CharsetFailed;
		}
		return ERR_NONE;
	}
	// Sharp: 902SH/SX813
	if(theApp.m_ManufacturerID == AT_Sharp){
		char szModelName[MAX_PATH];
		error = ATGEN_GetModel(szModelName,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE)
		{
			if(pCharsetInfo)
			{
				pCharsetInfo->m_NonUCS2CharsetFailed = theApp.m_NonUCS2CharsetFailed;
				pCharsetInfo->m_PBKCharset = theApp.m_PBKCharset;
				pCharsetInfo->m_UCS2CharsetFailed = theApp.m_UCS2CharsetFailed;
			}
			return error;
		}
		if(strncmp(szModelName, "SHARP/SX813", 11)==0 ||
			strncmp(szModelName, "SHARP/902SH", 11)==0 )
		{
			bSharpUTF8=true;
		}
	}
	/* SonyErricson  phones use only PCCP437? */
	if (theApp.m_ManufacturerID == AT_Ericsson || bSharpUTF8) {
		smprintf(theApp.m_pDebuginfo, "Setting charset to UTF8\n");
		if( theApp.m_PBKCharset == AT_PBK_UTF8)
		{
			if(pCharsetInfo)
			{
				pCharsetInfo->m_NonUCS2CharsetFailed = theApp.m_NonUCS2CharsetFailed;
				pCharsetInfo->m_PBKCharset = theApp.m_PBKCharset;
				pCharsetInfo->m_UCS2CharsetFailed = theApp.m_UCS2CharsetFailed;
			}
			return ERR_NONE;
		}
//		error=GSM_WaitFor (s, "AT+CSCS=\"UTF-8\"\r", 16, 0x00, 3, ID_SetMemoryCharset);
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CSCS=");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		error = pWriteCommandfn ((unsigned char *)"AT+CSCS=\"UTF-8\"\r", 16, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);
		if (error == ERR_NONE ) {
			theApp.m_PBKCharset = AT_PBK_UTF8;
			if(pCharsetInfo)
			{
				pCharsetInfo->m_NonUCS2CharsetFailed = theApp.m_NonUCS2CharsetFailed;
				pCharsetInfo->m_PBKCharset = theApp.m_PBKCharset;
				pCharsetInfo->m_UCS2CharsetFailed = theApp.m_UCS2CharsetFailed;
			}
			return ERR_NONE;
		}
	}

	if (PreferUnicode && !theApp.m_UCS2CharsetFailed) {
		smprintf(theApp.m_pDebuginfo, "Setting charset to UCS2\n");
	//	error=GSM_WaitFor (s, "AT+CSCS=\"UCS2\"\r", 15, 0x00, 3, ID_SetMemoryCharset);
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CSCS=");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		error = pWriteCommandfn ((unsigned char *)"AT+CSCS=\"UCS2\"\r", 15, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);
		if (error == ERR_NONE) {
			theApp.m_PBKCharset = AT_PBK_UCS2;
			if(pCharsetInfo)
			{
				pCharsetInfo->m_NonUCS2CharsetFailed = theApp.m_NonUCS2CharsetFailed;
				pCharsetInfo->m_PBKCharset = theApp.m_PBKCharset;
				pCharsetInfo->m_UCS2CharsetFailed = theApp.m_UCS2CharsetFailed;
			}
			return ERR_NONE;
		} else {
			theApp.m_UCS2CharsetFailed = true;
		}
	}
	
	smprintf(theApp.m_pDebuginfo, "Setting charset to HEX\n");
//	error=GSM_WaitFor (s, "AT+CSCS=\"HEX\"\r", 14, 0x00, 3, ID_SetMemoryCharset);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CSCS=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"AT+CSCS=\"HEX\"\r", 14, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	/* Falcom replies OK for HEX mode and send everything
	 * in normal format */
	if (error == ERR_NONE && theApp.m_ManufacturerID != AT_Falcom) {
		theApp.m_PBKCharset = AT_PBK_HEX;
		if(pCharsetInfo)
		{
			pCharsetInfo->m_NonUCS2CharsetFailed = theApp.m_NonUCS2CharsetFailed;
			pCharsetInfo->m_PBKCharset = theApp.m_PBKCharset;
			pCharsetInfo->m_UCS2CharsetFailed = theApp.m_UCS2CharsetFailed;
		}
		return ERR_NONE;
	}

	smprintf(theApp.m_pDebuginfo, "Setting charset to GSM\n");
//	error=GSM_WaitFor (s, "AT+CSCS=\"GSM\"\r", 14, 0x00, 3, ID_SetMemoryCharset);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CSCS=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"AT+CSCS=\"GSM\"\r", 14, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error == ERR_NONE) {
		theApp.m_PBKCharset = AT_PBK_GSM;
		if(pCharsetInfo)
		{
			pCharsetInfo->m_NonUCS2CharsetFailed = theApp.m_NonUCS2CharsetFailed;
			pCharsetInfo->m_PBKCharset = theApp.m_PBKCharset;
			pCharsetInfo->m_UCS2CharsetFailed = theApp.m_UCS2CharsetFailed;
		}
		return ERR_NONE;
	}

	if (!theApp.m_UCS2CharsetFailed) {
		theApp.m_NonUCS2CharsetFailed = true;
		smprintf(theApp.m_pDebuginfo, "Setting charset to UCS2\n");
	//	error=GSM_WaitFor (s, "AT+CSCS=\"UCS2\"\r", 15, 0x00, 3, ID_SetMemoryCharset);
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CSCS=");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		error = pWriteCommandfn ((unsigned char *) "AT+CSCS=\"UCS2\"\r", 15, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);
		if (error == ERR_NONE) {
			theApp.m_PBKCharset = AT_PBK_UCS2;
			if(pCharsetInfo)
			{
				pCharsetInfo->m_NonUCS2CharsetFailed = theApp.m_NonUCS2CharsetFailed;
				pCharsetInfo->m_PBKCharset = theApp.m_PBKCharset;
				pCharsetInfo->m_UCS2CharsetFailed = theApp.m_UCS2CharsetFailed;
			}
			return ERR_NONE;
		} else {
			theApp.m_UCS2CharsetFailed = true;
		}
	}
	if(pCharsetInfo)
	{
		pCharsetInfo->m_NonUCS2CharsetFailed = theApp.m_NonUCS2CharsetFailed;
		pCharsetInfo->m_PBKCharset = theApp.m_PBKCharset;
		pCharsetInfo->m_UCS2CharsetFailed = theApp.m_UCS2CharsetFailed;
	}

	return error;	
}

GSM_Error ATGEN_Initialise(OnePhoneModel *pMobileInfo,GSM_Error (*pSetATProtocolDatafn)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error               error;

	theApp.m_pDebuginfo = pDebuginfo;
	theApp.m_ManufacturerID		= AT_Unknown;
	theApp.m_PBKCharset		= AT_PBK_ERROR;
 	theApp.m_UCS2CharsetFailed		= false;
 	theApp.m_NonUCS2CharsetFailed	= false;
	theApp.m_PBKSBNR			= AT_SBNR_ERROR;
	theApp.m_MemoryInfo.FirstMemoryEntry		= 0;
	theApp.m_NextMemoryEntry		= 0;
	theApp.m_MemoryInfo.TextLength		= 0;
	theApp.m_MemoryInfo.NumberLength		= 0;
	theApp.m_MemoryInfo.MemorySize		= 0;
	theApp.m_PhoneSMSMemory		= AT_SMSMemory_ERROR;
	theApp.m_SIMSMSMemory		= AT_SMSMemory_ERROR;
	theApp.m_SMSMemory			= MEM_ERROR;
	theApp.m_CanSaveSMS		= false;
	theApp.m_PBKMemory			= MEM_ERROR;
	theApp.m_SMSMode			= SMS_AT_ERROR;
	theApp.m_pMobileInfo = pMobileInfo;
/** peggy temp
	theApp.m_PBKMemory			= 0;
	theApp.m_PBKMemories[0]		= 0;
	theApp.m_FirstCalendarPos		= 0;
	theApp.m_file.Used 		= 0;
	theApp.m_file.Buffer 		= NULL;
	theApp.m_OBEX			= false;

	theApp.m_ErrorText			= NULL;
	theApp.m_dtCurrentTime.Day = theApp.m_dtCurrentTime.Hour =theApp.m_dtCurrentTime.Minute =
	theApp.m_dtCurrentTime.Month = theApp.m_dtCurrentTime.Second =theApp.m_dtCurrentTime.Timezone =theApp.m_dtCurrentTime.Year =0;
*/
    smprintf(theApp.m_pDebuginfo, "Sending simple AT command to wake up some devices\n");
    // v1.1.0.1 ;marked by mingfa
	//error = GSM_WaitFor (s, "AT\r", 3, 0x00, 2, ID_IncomingFrame);
	//if (error != ERR_NONE) return error;

	//v1.1.0.1  
    // verify , refer to PP S/w
//	error = GSM_WaitFor (s, "ATZ\r", 4, 0x00, 3, ID_IncomingFrame);
	// modify for GX18
	/*
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"ATZ\r");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"ATZ\r", 4, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);
    */
    // added for GX18
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"ATE1");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"ATE1\r", 5, 0x00, 4, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error != ERR_NONE)
	{
	//	error = GSM_WaitFor (s, "ATE1\r", 5, 0x00, 3, ID_EnableEcho);
		Sleep( 200 );
		error = pWriteCommandfn ((unsigned char *)"ATE1\r", 5, 0x00, 4, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	}
	Sleep( 200 ); // for GX18
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"ATZ\r");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"ATZ\r", 4, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);

	if (error != ERR_NONE)
	{
	//  error = GSM_WaitFor (s, "AT\r", 3, 0x00, 2, ID_IncomingFrame);
		Sleep(100 );
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT\r");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		error = pWriteCommandfn ((unsigned char *)"AT\r", 3, 0x00, 4, false,NULL,&ReplymsgType,ATGEN_GenericReply);
		if (error != ERR_NONE) return error;
	}
    

	smprintf(theApp.m_pDebuginfo, "Enabling echo\n");
//	error = GSM_WaitFor (s, "ATE1\r", 5, 0x00, 3, ID_EnableEcho);
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"ATE1");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"ATE1\r", 5, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error != ERR_NONE)
	{
	//	error = GSM_WaitFor (s, "ATE1\r", 5, 0x00, 3, ID_EnableEcho);
		error = pWriteCommandfn ((unsigned char *)"ATE1\r", 5, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	}
	
	smprintf(theApp.m_pDebuginfo, "Enabling CME errors\n");
	/* Try numeric errors */
//	error = GSM_WaitFor (s, "AT+CMEE=1\r", 10, 0x00, 3, ID_EnableErrorInfo);
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CMEE=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"AT+CMEE=1\r", 10, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error != ERR_NONE) {
		/* Try textual errors */
	//	error = GSM_WaitFor (s, "AT+CMEE=2\r", 10, 0x00, 3, ID_EnableErrorInfo);
		error = pWriteCommandfn ((unsigned char *)"AT+CMEE=2\r", 10, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);
		if (error != ERR_NONE) {
			smprintf(theApp.m_pDebuginfo, "CME errors could not be enabled, some error types won't be detected.\n");
		}
	}

	char szModelName[MAX_PATH];
	error = ATGEN_GetModel(szModelName,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

	if (!IsPhoneFeatureAvailable(pMobileInfo, F_SLOWWRITE))
	{
		pSetATProtocolDatafn(false,true,0x02);
	//	s->Protocol.Data.AT.FastWrite = true;
	}
	error=ATGEN_SetPBKCharset(true,NULL,pWriteCommandfn,pDebuginfo); /* For reading we prefer unicode */
	if (error != ERR_NONE) return error;

	return error;
}
GSM_Error ATGEN_Terminate(GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
//	if(s->Phone.Data.Priv.OBEXGEN.Service != 0)
//		OBEXGEN_Disconnect(s);
//	free(Priv->file.Buffer);
	return ERR_NONE;
}

GSM_Error ATGEN_GetFirmware(char* pszFirmwareVer,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error error;
	theApp.m_pDebuginfo = pDebuginfo;

//	if (s->Phone.Data.Version[0] != 0) return ERR_NONE;
//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;

//	error=ATGEN_GetManufacturer(s);
//	if (error != ERR_NONE) return error;

	smprintf(theApp.m_pDebuginfo, "Getting firmware - method 2\n");

	//error=GSM_WaitFor (s, "AT+CGMR\r", 8, 0x00, 4, ID_GetFirmware);
	theApp.m_pszTemp = pszFirmwareVer;
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CGMR");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"AT+CGMR\r", 8, 0x00, 8, false,NULL,&ReplymsgType,ATGEN_ReplyGetFirmwareCGMR);
	return error;
}
GSM_Error ATGEN_GetManufacturer(char* pszManufacturer,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	theApp.m_pszTemp = pszManufacturer;
//	return GSM_WaitFor (s, "AT+CGMI\r", 8, 0x00, 6, ID_GetManufacturer);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CGMI");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return pWriteCommandfn ((unsigned char *)"AT+CGMI\r", 8, 0x00, 12, false,NULL,&ReplymsgType,ATGEN_ReplyGetManufacturer);
}
GSM_Error ATGEN_GetModel(char* pszModel,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error error;


// v1.1.0.0 ; marked by mingfa ; loopproc() will update modelinfo value
//	if (s->Phone.Data.Model[0] != 0) return ERR_NONE;

//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;

	smprintf(theApp.m_pDebuginfo, "Getting model\n");
//	error=GSM_WaitFor (s, "AT+CGMM\r", 8, 0x00, 3, ID_GetModel);
	theApp.m_pszTemp = pszModel;
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CGMM");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error=pWriteCommandfn ((unsigned char *)"AT+CGMM\r", 8, 0x00, 20, false,NULL,&ReplymsgType,ATGEN_ReplyGetModel);

	if (error==ERR_NONE) 
	{
		if(pDebuginfo)
		{
			if (pDebuginfo->dl==DL_TEXT || pDebuginfo->dl==DL_TEXTALL ||
				pDebuginfo->dl==DL_TEXTDATE || pDebuginfo->dl==DL_TEXTALLDATE) {
				smprintf(theApp.m_pDebuginfo, "[Connected model  - \"%s\"]\n",theApp.m_pszTemp);
			}
		}
	}
	return error;
}
GSM_Error ATGEN_GetIMEI (char* pszIMEI,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	theApp.m_pDebuginfo = pDebuginfo;
//	if (s->Phone.Data.IMEI[0] != 0) return ERR_NONE;
//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;
	smprintf(theApp.m_pDebuginfo, "Getting IMEI\n");
//	return GSM_WaitFor (s, "AT+CGSN\r", 8, 0x00, 3, ID_GetIMEI);
	theApp.m_pszTemp = pszIMEI;
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CGSN");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return pWriteCommandfn ((unsigned char *)"AT+CGSN\r", 8, 0x00, 20, false,NULL,&ReplymsgType,ATGEN_ReplyGetIMEI);
}

GSM_Error ATGEN_GetDateTime( GSM_DateTime *date_time,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;


//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;
//	s->Phone.Data.DateTime=date_time;
	theApp.m_pdate_time = date_time;
	if(theApp.m_ManufacturerID == AT_Sharp){
		theApp.m_pdate_time->Year=1900;
		return ERR_NONE;
	}
	smprintf(theApp.m_pDebuginfo, "Getting date & time\n");
//	return GSM_WaitFor (s, "AT+CCLK?\r", 9, 0x00, 4, ID_GetDateTime);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CCLK?");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return pWriteCommandfn ((unsigned char *)"AT+CCLK?\r", 9, 0x00, 20, false,NULL,&ReplymsgType,ATGEN_ReplyGetDateTime_Alarm);
}

GSM_Error ATGEN_SetDateTime(GSM_DateTime *date_time,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	char req[128];
	theApp.m_pDebuginfo = pDebuginfo;

//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;
	sprintf(req, "AT+CCLK=\"%02i/%02i/%02i,%02i:%02i:%02i+00\"\r",
		     date_time->Year-2000,date_time->Month,date_time->Day,
		     date_time->Hour,date_time->Minute,date_time->Second);

	smprintf(theApp.m_pDebuginfo, "Setting date & time\n");
//	return GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_SetDateTime);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CCLK=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return pWriteCommandfn ((unsigned char *)req, strlen(req), 0x00, 8, false,NULL,&ReplymsgType,ATGEN_GenericReply);
}
GSM_Error ATGEN_Reset( bool hard,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error error;

	theApp.m_pDebuginfo = pDebuginfo;
//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;
	if (!hard) return ERR_NOTSUPPORTED;

	smprintf(theApp.m_pDebuginfo, "Resetting device\n");
	/* Siemens 35 */
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CFUN=1,1");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error=pWriteCommandfn ((unsigned char *)"AT+CFUN=1,1\r", 12, 0x00, 16, false,NULL,&ReplymsgType,ATGEN_ReplyReset);
//	error=GSM_WaitFor (s, "AT+CFUN=1,1\r", 12, 0x00, 8, ID_Reset);
	if (error != ERR_NONE) {
		/* Siemens M20 */
	//	error=GSM_WaitFor (s, "AT^SRESET\r", 10, 0x00, 8, ID_Reset);
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT^SRESET");
		error=pWriteCommandfn ((unsigned char *)"AT^SRESET\r", 10, 0x00, 16, false,NULL,&ReplymsgType,ATGEN_ReplyReset);
	}
	return error;
}







