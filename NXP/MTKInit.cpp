#include "stdafx.h"
#include "MTK.h"
#include "commfun.h"
#include "MTKfundef.h"
extern CMTKApp theApp;
GSM_Reply_MsgType ReplymsgType;
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

GSM_Error MTK_GetReplyStatue(GSM_Protocol_Message	*msg,GSM_ATReplayInfo* pReplynfo)
{
	int 			i	= 0, j, k;
	char                    *err, *line;
	ATErrorCode		*ErrorCodes = NULL;

	SplitLines(msg->Buffer, msg->Length, &theApp.m_Lines,(unsigned char *) "\x0D\x0A", 2, true);

	/* Find number of lines */
	while (theApp.m_Lines.numbers[i*2+1] != 0) {
		/* FIXME: handle special chars correctly */
		smprintf(theApp.m_pDebuginfo, "%i \"%s\"\n",i+1,GetLineString(msg->Buffer,theApp.m_Lines,i+1));
		i++;
	}

	pReplynfo->ReplyState 	= AT_Reply_Unknown;
	pReplynfo->ErrorText[0]     	= '\0';
	pReplynfo->ErrorCode     	= 0;

	line = GetLineString(msg->Buffer,theApp.m_Lines,i);
	if (!strcmp(line,"OK"))		pReplynfo->ReplyState = AT_Reply_OK;
	if (!_strnicmp (line,"ATE",3))	pReplynfo->ReplyState = AT_Reply_OK;
	if (!strcmp(line,"> "))		pReplynfo->ReplyState = AT_Reply_SMSEdit;
	if (!strcmp(line,"CONNECT"))	pReplynfo->ReplyState = AT_Reply_Connect;
	if (!strcmp(line,"ERROR"  ))	pReplynfo->ReplyState = AT_Reply_Error;
	if (!strcmp(line,"ENTERED"  ))	pReplynfo->ReplyState = AT_Reply_Entered;

	if (!strncmp(line,"+CME ERROR:",11)) {
		pReplynfo->ReplyState = AT_Reply_CMEError;
		ErrorCodes = CMEErrorCodes;
	}
	if (!strncmp(line,"+CMS ERROR:",11)) {
		pReplynfo->ReplyState = AT_Reply_CMSError;
		ErrorCodes = CMSErrorCodes;
	}
	if (!strncmp(line,"+MMGL:",6)) pReplynfo->ReplyState = AT_Reply_Continue; 
	if (!strncmp(line,"+CPBR:",6)) pReplynfo->ReplyState = AT_Reply_Continue; 
	if (!strncmp(line,"+EFSL:",6)) pReplynfo->ReplyState = AT_Reply_Continue; 
	if (!strncmp(line,"+EFSR:",6)) pReplynfo->ReplyState = AT_Reply_Continue; 

	if (pReplynfo->ReplyState == AT_Reply_CMEError || pReplynfo->ReplyState == AT_Reply_CMSError) {
	        j = 0;
		/* One char behind +CM[SE] ERROR */
		err = line + 12;
		while (err[j] && !isalnum(err[j])) j++;
		if (isdigit(err[j])) {
			pReplynfo->ErrorCode = atoi(&(err[j]));
			k = 0;
			while (ErrorCodes[k].Number != -1) {
				if (ErrorCodes[k].Number == pReplynfo->ErrorCode) 
				{
					wsprintf(pReplynfo->ErrorText ,"%s",ErrorCodes[k].Text);
				//	Priv->ErrorText = (char *)&(ErrorCodes[k].Text);
					break;
				}
				k++;
			}
		} else if (isalpha(err[j])) {
			k = 0;
			while (ErrorCodes[k].Number != -1) {
				if (!strncmp(err + j, ErrorCodes[k].Text, strlen(ErrorCodes[k].Text))) {
					pReplynfo->ErrorCode = ErrorCodes[k].Number;
					wsprintf(pReplynfo->ErrorText ,"%s",ErrorCodes[k].Text);
			//		Priv->ErrorText = (char *)&(ErrorCodes[k].Text);
					break;
				}
				k++;
			}
		}
	}
	return ERR_NONE;
}

// for 9@9u
GSM_Error SF_GetReplyStatue(GSM_Protocol_Message	*msg,GSM_ATReplayInfo* pReplynfo)
{
	int 			i	= 0, j, k;
	char                    *err, *line;
	ATErrorCode		*ErrorCodes = NULL;

	SplitLines(msg->Buffer, msg->Length, &theApp.m_Lines,(unsigned char *) "\x0D\x0A", 2, true);

	/* Find number of lines */
	while (theApp.m_Lines.numbers[i*2+1] != 0) {
		/* FIXME: handle special chars correctly */
		smprintf(theApp.m_pDebuginfo, "%i \"%s\"\n",i+1,GetLineString(msg->Buffer,theApp.m_Lines,i+1));
		i++;
	}

	pReplynfo->ReplyState 	= AT_Reply_Unknown;
	pReplynfo->ErrorText[0]     	= '\0';
	pReplynfo->ErrorCode     	= 0;

	line = GetLineString(msg->Buffer,theApp.m_Lines,i);
	if (!strcmp(line,"OK"))		pReplynfo->ReplyState = AT_Reply_OK;
	if (!_strnicmp (line,"ATE",3))	pReplynfo->ReplyState = AT_Reply_OK;
	if (!strcmp(line,"> "))		pReplynfo->ReplyState = AT_Reply_SMSEdit;
	if (!strcmp(line,"CONNECT"))	pReplynfo->ReplyState = AT_Reply_Connect;
	if (!strcmp(line,"ERROR"  ))	pReplynfo->ReplyState = AT_Reply_Error;
	if (!strcmp(line,"ENTERED"  ))	pReplynfo->ReplyState = AT_Reply_Entered;

	if (!strncmp(line,"+CME ERROR:",11)) {
		pReplynfo->ReplyState = AT_Reply_CMEError;
		ErrorCodes = CMEErrorCodes;
	}
	if (!strncmp(line,"+CMS ERROR:",11)) {
		pReplynfo->ReplyState = AT_Reply_CMSError;
		ErrorCodes = CMSErrorCodes;
	}
	if (!strncmp(line,"+MMGL:",6)) pReplynfo->ReplyState = AT_Reply_Continue; 
	if (!strncmp(line,"+CPBR:",6)) pReplynfo->ReplyState = AT_Reply_Continue; 
	if (!strncmp(line,"+EFSL:",6)) pReplynfo->ReplyState = AT_Reply_Continue; 
	if (!strncmp(line,"+EFSR:",6)) pReplynfo->ReplyState = AT_Reply_Continue; 

	if (pReplynfo->ReplyState == AT_Reply_CMEError || pReplynfo->ReplyState == AT_Reply_CMSError) {
	        j = 0;
		/* One char behind +CM[SE] ERROR */
		err = line + 12;
		while (err[j] && !isalnum(err[j])) j++;
		if (isdigit(err[j])) {
			pReplynfo->ErrorCode = atoi(&(err[j]));
			k = 0;
			while (ErrorCodes[k].Number != -1) {
				if (ErrorCodes[k].Number == pReplynfo->ErrorCode) 
				{
					wsprintf(pReplynfo->ErrorText ,"%s",ErrorCodes[k].Text);
				//	Priv->ErrorText = (char *)&(ErrorCodes[k].Text);
					break;
				}
				k++;
			}
		} else if (isalpha(err[j])) {
			k = 0;
			while (ErrorCodes[k].Number != -1) {
				if (!strncmp(err + j, ErrorCodes[k].Text, strlen(ErrorCodes[k].Text))) {
					pReplynfo->ErrorCode = ErrorCodes[k].Number;
					wsprintf(pReplynfo->ErrorText ,"%s",ErrorCodes[k].Text);
			//		Priv->ErrorText = (char *)&(ErrorCodes[k].Text);
					break;
				}
				k++;
			}
		}
	}
	return ERR_NONE;
}




GSM_Error MTK_SetCharset(bool PreferUnicode)
{
	GSM_Error		error;
	if(theApp.m_PBKCharset == AT_PBK_UCS2 && PreferUnicode) 
		return ERR_NONE;
	if(theApp.m_PBKCharset == AT_PBK_GSM && PreferUnicode == false) 
		return ERR_NONE;

	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CSCS=");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	if(PreferUnicode)
		error = theApp.m_pWriteCommandfn ((unsigned char *)"AT+CSCS=\"UCS2\"\r", 15, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	else
		error = theApp.m_pWriteCommandfn ((unsigned char *)"AT+CSCS=\"IRA\"\r", 14, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);

	if (error == ERR_NONE) 
	{
		if(PreferUnicode)
			theApp.m_PBKCharset = AT_PBK_UCS2;
		else
			theApp.m_PBKCharset = AT_PBK_GSM;
	}
	return error;
}


GSM_Error WINAPI SetConnectparameter(GSM_ConnectionType *ConnectionType,OnePhoneModel *pMobileInfo,	GSM_Error (*pSetATProtocolDatafn)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	theApp.m_ConnectionType = ConnectionType;
	theApp.m_pMobileInfo = pMobileInfo;
	theApp.m_pWriteCommandfn = pWriteCommandfn;
	theApp.m_pSetATProtocolDatafn = pSetATProtocolDatafn;
	return ERR_NONE;
}
GSM_Error WINAPI Initialise()
{
	if (!IsPhoneFeatureAvailable(theApp.m_pMobileInfo, F_SLOWWRITE))
	{
		theApp.m_pSetATProtocolDatafn(false,true,0x02);
//		s->Protocol.Data.AT.FastWrite = true;
	}
#ifdef _PG1900
	ReplymsgType.nCount = 3;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+ESLP");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[1].msgtype,"OK");
	ReplymsgType.CheckInfo[1].subtypechar = 0;
	ReplymsgType.CheckInfo[1].subtype = 0x00;
	wsprintf((char*)ReplymsgType.CheckInfo[2].msgtype,"ERROR");
	ReplymsgType.CheckInfo[2].subtypechar = 0;
	ReplymsgType.CheckInfo[2].subtype = 0x00;
	theApp.m_pWriteCommandfn ((unsigned char *)"AT+ESLP=0\r", 10, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);
#endif

	GSM_Error error = ATGEN_Initialise(theApp.m_pMobileInfo,theApp.m_pSetATProtocolDatafn,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo);
	theApp.m_PBKCharset = AT_PBK_UCS2;
	theApp.m_MemoryInfo.FirstMemoryEntry		= 0;
	theApp.m_MemoryInfo.TextLength		= 0;
	theApp.m_MemoryInfo.NumberLength		= 0;
	theApp.m_MemoryInfo.MemorySize		= 0;
	theApp.m_MMSFileInfoList.RemoveAll();

	char szFirmware[MAX_PATH];
	GSM_Error error2 = GetFirmware(szFirmware);
	if(error2=ERR_NONE)
	{
		if(*theApp.m_ConnectionType == GCT_CABLEAT)
		{
			if(_stricmp(szFirmware,"EV500_V202_CC886_GPLUS_200511") == 0 ||
			_stricmp(szFirmware,"J202-ASUS-TW-R103-0-V4.3.0") == 0 ||
			_stricmp(szFirmware,"MAUI.05C.W06.12.MP.V24") == 0 ||
			_stricmp(szFirmware,"0604.G638D300.MP.FITEL") == 0 ||
			strstr(szFirmware,"F168_V1.13")!= NULL)
				theApp.m_nPacketSize = 200;
		}
	}
#ifdef _PG1900
	theApp.m_nPacketSize = 800;
#endif
	return error;
}

GSM_Error WINAPI Terminate ()
{
	POSITION pos = theApp.m_MMSFileInfoList.GetHeadPosition();
	while(pos)
	{
		GSM_File *f = (GSM_File*)theApp.m_MMSFileInfoList.GetNext(pos);
		if(f)
			delete f;
	}
	theApp.m_MMSFileInfoList.RemoveAll();

	return ATGEN_Terminate(theApp.m_pWriteCommandfn,theApp.m_pDebuginfo);
}
GSM_Error WINAPI GetManufacturer(char* pszManufacturer)
{
	return ATGEN_GetManufacturer(pszManufacturer,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo);
}
GSM_Error WINAPI GetModel(char* pszModel)
{
	return ATGEN_GetModel(pszModel,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
}
GSM_Error WINAPI GetFirmware(char* pszFirmwareVer)
{
	return ATGEN_GetFirmware(pszFirmwareVer,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
}
GSM_Error WINAPI GetIMEI(char* pszIMEI)
{
	GSM_Error error = ERR_NONE;
	char szIMEI[MAX_PATH];
	char szTemp[MAX_PATH];
	error =  ATGEN_GetIMEI(szIMEI,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
	char* psz = strstr(szIMEI,"\n");
	if(psz)
		sprintf(szTemp,psz+1);
	else
		sprintf(szTemp,szIMEI);

	psz = strstr(szTemp,"\r");
	if(psz)
		sprintf(pszIMEI,psz+1);
	else
		sprintf(pszIMEI,szTemp);
	return error;
}
GSM_Error WINAPI GetPhoneDateTime(GSM_DateTime *date_time)
{
	return ATGEN_GetDateTime(date_time,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
}
GSM_Error WINAPI SetPhoneDateTime(GSM_DateTime *date_time)
{
	return ATGEN_SetDateTime(date_time,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
}
GSM_Error WINAPI PhoneReset(bool hard)
{
	return ATGEN_Reset(hard,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
}
GSM_Error WINAPI CheckConnectStatus()
{
	static ss = 1;
	GSM_Error err = ERR_NONE;
	char szTemp[MAX_PATH];
	if(ss == 1)
	{
		ss = 0;
		err = GetModel(szTemp);
	}
	else
	{
		ss = 1;
		err = GetIMEI(szTemp);
	}
	return err;
}
GSM_Error WINAPI GetNetworkInfo(GSM_NetworkInfo *netinfo)
{
	if(theApp.m_bObexMode) return ERR_OBEXMODE;
	return ATGEN_GetNetworkInfo(netinfo,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
}
GSM_Error WINAPI GetSignalQuality(GSM_SignalQuality *sig)
{
	if(theApp.m_bObexMode) return ERR_OBEXMODE;
	return ATGEN_GetSignalQuality(sig,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
}
GSM_Error WINAPI GetBatteryCharge(GSM_BatteryCharge *bat)
{
	if(theApp.m_bObexMode) return ERR_OBEXMODE;
	return ATGEN_GetBatteryCharge(bat,theApp.m_pWriteCommandfn,theApp.m_pDebuginfo );
}
