#include "stdafx.h"
#include "Atgen.h"
#include "AtGenFundef.h"
#include "coding.h"

extern CATgenApp theApp;

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

GSM_Error ATGEN_GetReplyStatue(GSM_Protocol_Message	*msg,GSM_ATReplayInfo* pReplynfo)
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
	if (!strncmp(line,"+MMGL:",6)) pReplynfo->ReplyState = AT_Reply_Continue; //peggy
	if (!strncmp(line,"+CPBR:",6)) pReplynfo->ReplyState = AT_Reply_Continue; //peggy 0201 add
	if (!strncmp(line,"+MPBR:",6)) pReplynfo->ReplyState = AT_Reply_Continue; //peggy 0201 add
	if (!strncmp(line,"+MDBR:",6)) pReplynfo->ReplyState = AT_Reply_Continue; //peggy 0201 add
	if (!strncmp(line,"+SSHR:",6)) pReplynfo->ReplyState = AT_Reply_Continue; //bobby 8.31.05
	if (!strncmp(line,"+MPBRE:",7)) pReplynfo->ReplyState = AT_Reply_Continue; //bobby 1.24.06


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
GSM_Error ATGEN_ReplyGetSignalQuality(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	GSM_SignalQuality	*Signal = theApp.m_SignalQuality;
	int 			i;
	char 			*pos;

	Signal->SignalStrength 	= -1;
	Signal->SignalPercent 	= -1;
	Signal->BitErrorRate 	= -1;
    
	switch (Replynfo.ReplyState) {
        case AT_Reply_OK:
            smprintf(theApp.m_pDebuginfo, "Signal quality info received\n");
            i = atoi((char*)msg.Buffer+15);
            if (i != 99) {
                /* from GSM 07.07 section 8.5 */
                Signal->SignalStrength = 2 * i - 113;

                /* FIXME: this is wild guess and probably will be phone dependant */
                Signal->SignalPercent = 15 * i;
                if (Signal->SignalPercent > 100) Signal->SignalPercent = 100;
            }
            pos = strchr((char*)msg.Buffer + 15, ',');
            if (pos != NULL) {
                i = atoi(pos + 1);
                /* from GSM 05.08 section 8.2.4 */
                switch (i) {
                    case 0: Signal->BitErrorRate =  0; break; /* 0.14 */
                    case 1: Signal->BitErrorRate =  0; break; /* 0.28 */
                    case 2: Signal->BitErrorRate =  1; break; /* 0.57 */
                    case 3: Signal->BitErrorRate =  1; break; /* 1.13 */
                    case 4: Signal->BitErrorRate =  2; break; /* 2.26 */
                    case 5: Signal->BitErrorRate =  5; break; /* 4.53 */
                    case 6: Signal->BitErrorRate =  9; break; /* 9.05 */
                    case 7: Signal->BitErrorRate = 18; break; /* 18.10 */
                }
            }
            return ERR_NONE;
        case AT_Reply_CMSError:
            return ATGEN_HandleCMSError(Replynfo);
        default:
            break;
	}
	return ERR_UNKNOWNRESPONSE;
}
GSM_Error ATGEN_ReplyGetBatteryCharge(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);
    int 			i;
    
   theApp.m_BatteryCharge->BatteryPercent = -1;
   theApp.m_BatteryCharge->ChargeState 	=(GSM_ChargeState) 0;
        
    switch (Replynfo.ReplyState) {
        case AT_Reply_OK:
            smprintf(theApp.m_pDebuginfo, "Battery level received\n");
            theApp.m_BatteryCharge->BatteryPercent = atoi((char*)msg.Buffer+17);
            i = atoi((char*)msg.Buffer+14);
            if (i >= 0 && i <= 3) { 
                theApp.m_BatteryCharge->ChargeState =(GSM_ChargeState)( i + 1);
            }
            return ERR_NONE;
        case AT_Reply_Error:
            smprintf(theApp.m_pDebuginfo, "Can't get battery level\n");
            return ERR_UNKNOWN;
        case AT_Reply_CMSError:
            smprintf(theApp.m_pDebuginfo, "Can't get battery level\n");
            return ATGEN_HandleCMSError(Replynfo);
        default:
            break;
    }
    return ERR_UNKNOWNRESPONSE;
}
GSM_Error ATGEN_ReplyGetNetworkLAC_CID(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	GSM_NetworkInfo		*NetworkInfo = theApp.m_NetworkInfo;
	GSM_Lines		Lines;
	int			i=0;
	char			*answer;

	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		break;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(Replynfo);
	default:
		return ERR_UNKNOWNRESPONSE;
	}
	SplitLines((unsigned char *)GetLineString(msg.Buffer,theApp.m_Lines,2),
		strlen(GetLineString(msg.Buffer,theApp.m_Lines,2)),
		&Lines,(unsigned char *) ",", 1, true);

	/* Find number of lines */
	while (Lines.numbers[i*2+1] != 0) {
		/* FIXME: handle special chars correctly */
		smprintf(theApp.m_pDebuginfo, "%i \"%s\"\n",i+1,GetLineString((unsigned char *)GetLineString(msg.Buffer,theApp.m_Lines,2),Lines,i+1));
		i++;
	}

	smprintf(theApp.m_pDebuginfo, "Network LAC & CID & state received\n");
	answer = GetLineString((unsigned char *)GetLineString(msg.Buffer,theApp.m_Lines,2),Lines,2);
	while (*answer == 0x20) answer++;
	switch (answer[0]) {
		case '0': NetworkInfo->State = GSM_NoNetwork;		break;
		case '1': NetworkInfo->State = GSM_HomeNetwork; 	break;
		case '2': NetworkInfo->State = GSM_RequestingNetwork; 	break;
		case '3': NetworkInfo->State = GSM_RegistrationDenied;	break;
		case '4': NetworkInfo->State = GSM_NetworkStatusUnknown;break;
		case '5': NetworkInfo->State = GSM_RoamingNetwork; 	break;
		default : NetworkInfo->State = GSM_NetworkStatusUnknown;break;
	}
	if (NetworkInfo->State == GSM_HomeNetwork ||
	    NetworkInfo->State == GSM_RoamingNetwork) {
		memset(NetworkInfo->CID,0,4);
		memset(NetworkInfo->LAC,0,4);

		if (Lines.numbers[3*2+1]==0) return ERR_NONE;

		answer = GetLineString((unsigned char *)GetLineString(msg.Buffer,theApp.m_Lines,2),Lines,3);
		while (*answer == 0x20) answer++;
		sprintf((char *)NetworkInfo->CID,	"%c%c%c%c", answer[1], answer[2], answer[3], answer[4]);

		answer = GetLineString((unsigned char *)GetLineString(msg.Buffer,theApp.m_Lines,2),Lines,4);
		while (*answer == 0x20) answer++;
		sprintf((char *)NetworkInfo->LAC,	"%c%c%c%c", answer[1], answer[2], answer[3], answer[4]);

		smprintf(theApp.m_pDebuginfo, "CID   : %s\n",NetworkInfo->CID);
		smprintf(theApp.m_pDebuginfo, "LAC   : %s\n",NetworkInfo->LAC);
	}
	return ERR_NONE;
}

GSM_Error ATGEN_ReplyGetNetworkCode(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	GSM_NetworkInfo		*NetworkInfo = theApp.m_NetworkInfo;

	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "Network code received\n");
	/*	if (Priv->Manufacturer == AT_Falcom) {
			NetworkInfo->NetworkCode[0] = msg.Buffer[22];
			NetworkInfo->NetworkCode[1] = msg.Buffer[23];
			NetworkInfo->NetworkCode[2] = msg.Buffer[24];
			NetworkInfo->NetworkCode[3] = ' ';
			NetworkInfo->NetworkCode[4] = msg.Buffer[25];
			NetworkInfo->NetworkCode[5] = msg.Buffer[26];
		} else*/ {
			NetworkInfo->NetworkCode[0] = msg.Buffer[23];
			NetworkInfo->NetworkCode[1] = msg.Buffer[24];
			NetworkInfo->NetworkCode[2] = msg.Buffer[25];
			NetworkInfo->NetworkCode[3] = ' ';
			NetworkInfo->NetworkCode[4] = msg.Buffer[26];
			NetworkInfo->NetworkCode[5] = msg.Buffer[27];
		}
		NetworkInfo->NetworkCode[6] = 0;
		sprintf((char*)NetworkInfo->NetworkName,"%s",DecodeUnicodeString((unsigned char *)GSM_GetNetworkName(NetworkInfo->NetworkCode)));
		smprintf(theApp.m_pDebuginfo, "   Network code              : %s\n", NetworkInfo->NetworkCode);
		smprintf(theApp.m_pDebuginfo, "   Network name for MBdrv    : %s ",
			DecodeUnicodeString((unsigned char *)GSM_GetNetworkName(NetworkInfo->NetworkCode)));
		smprintf(theApp.m_pDebuginfo, "(%s)\n",DecodeUnicodeString((unsigned char *)GSM_GetCountryName(NetworkInfo->NetworkCode)));
		return ERR_NONE;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}
GSM_Error ATGEN_ReplyGetFirmwareGMR(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	unsigned int		i = 0;

	strcpy(theApp.m_pszTemp,"unknown");
//	s->Phone.Data.VerNum = 0;
	if (Replynfo.ReplyState == AT_Reply_OK) 
	{
		CopyLineString((unsigned char *)theApp.m_pszTemp, msg.Buffer, theApp.m_Lines, 2);
		/* Sometimes phone adds this before manufacturer (Sagem) */
		if (strncmp("+GMR: ", theApp.m_pszTemp, 6) == 0) {
			memmove(theApp.m_pszTemp, theApp.m_pszTemp + 6, strlen(theApp.m_pszTemp + 6) + 1);
		}
	}
	if (theApp.m_ManufacturerID == AT_Ericsson) {
		while (1) {
			if (theApp.m_pszTemp[i] == 0x20) {
				theApp.m_pszTemp[i] = 0x00;
				break;
			}
			if (i == strlen(theApp.m_pszTemp)) break;
			i++;
		}
	}
	smprintf(theApp.m_pDebuginfo, "Received firmware version: \"%s\"\n",theApp.m_pszTemp);
	return ERR_NONE;
}
GSM_Error ATGEN_ReplyGetFirmwareCGMR(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	unsigned int		i = 0;

	strcpy(theApp.m_pszTemp,"unknown");
//	s->Phone.Data.VerNum = 0;
	if (Replynfo.ReplyState == AT_Reply_OK) 
	{
		CopyLineString((unsigned char *)theApp.m_pszTemp, msg.Buffer, theApp.m_Lines, 2);
		/* Sometimes phone adds this before manufacturer (Sagem) */
		if (strncmp("+CGMR: ", theApp.m_pszTemp, 7) == 0) {
			memmove(theApp.m_pszTemp, theApp.m_pszTemp + 7, strlen(theApp.m_pszTemp + 7) + 1);
		}
	}
	if (theApp.m_ManufacturerID == AT_Ericsson) {
		while (1) {
			if (theApp.m_pszTemp[i] == 0x20) {
				theApp.m_pszTemp[i] = 0x00;
				break;
			}
			if (i == strlen(theApp.m_pszTemp)) break;
			i++;
		}
	}
	smprintf(theApp.m_pDebuginfo, "Received firmware version: \"%s\"\n",theApp.m_pszTemp);
//	GSM_CreateFirmwareNumber(s);
	return ERR_NONE;
}

GSM_Error ATGEN_ReplyCDMAGetManufacturer(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "Manufacturer info received\n");
		theApp.m_ManufacturerID = AT_Unknown;
		if (strlen(GetLineString(msg.Buffer, theApp.m_Lines, 2)) <= MAX_MANUFACTURER_LENGTH) {
			CopyLineString((unsigned char *)theApp.m_pszTemp, msg.Buffer, theApp.m_Lines, 2);
		} else {
			smprintf(theApp.m_pDebuginfo, "WARNING: Manufacturer name too long, increase MAX_MANUFACTURER_LENGTH to at least %zd\n", strlen(GetLineString(msg.Buffer, theApp.m_Lines, 2)));
			theApp.m_pszTemp[0] = 0;
		}
		/* Sometimes phone adds this before manufacturer (Sagem) */
		if (strncmp("+GMI: ", theApp.m_pszTemp, 6) == 0) {
			memmove(theApp.m_pszTemp, theApp.m_pszTemp + 6, strlen(theApp.m_pszTemp + 6) + 1);
		}
	/*	if (strstr(msg.Buffer,"Falcom")) {
			smprintf(theApp.m_pDebuginfo, "Falcom\n");
			strcpy(theApp.m_pszTemp,"Falcom");
			theApp.m_ManufacturerID = AT_Falcom;
			if (strstr(msg.Buffer,"A2D")) {
				strcpy(s->Phone.Data.Model,"A2D");
				s->Phone.Data.ModelInfo = GetModelData(NULL,s->Phone.Data.Model,NULL);
				smprintf(theApp.m_pDebuginfo, "Model A2D\n");
			}
		}*/
		if (strstr((char*)msg.Buffer,"Nokia")) {
			smprintf(theApp.m_pDebuginfo, "Nokia\n");
			strcpy(theApp.m_pszTemp,"Nokia");
			theApp.m_ManufacturerID = AT_Nokia;
		}
		if (strstr((char*)msg.Buffer,"SIEMENS")) {
			smprintf(theApp.m_pDebuginfo, "Siemens\n");
			strcpy(theApp.m_pszTemp,"Siemens");
			theApp.m_ManufacturerID = AT_Siemens;
		}
		if (strstr((char*)msg.Buffer,"ERICSSON")) {
			smprintf(theApp.m_pDebuginfo, "Ericsson\n");
			strcpy(theApp.m_pszTemp,"Ericsson");
			theApp.m_ManufacturerID = AT_Ericsson;
		}
		if (strstr((char*)msg.Buffer,"Sony Ericsson")) {
			smprintf(theApp.m_pDebuginfo, "Ericsson\n");
			strcpy(theApp.m_pszTemp,"Ericsson");
			theApp.m_ManufacturerID = AT_Ericsson;
		}
		if (strstr((char*)msg.Buffer,"iPAQ")) {
			smprintf(theApp.m_pDebuginfo, "iPAQ\n");
			strcpy(theApp.m_pszTemp,"HP");
			theApp.m_ManufacturerID = AT_HP;
		}
		if (strstr((char*)msg.Buffer,"ALCATEL")) {
			smprintf(theApp.m_pDebuginfo, "Alcatel\n");
			strcpy(theApp.m_pszTemp,"Alcatel");
			theApp.m_ManufacturerID = AT_Alcatel;
		}
		if (strstr((char*)msg.Buffer,"SAGEM")) {
			smprintf(theApp.m_pDebuginfo, "Sagem\n");
			strcpy(theApp.m_pszTemp,"Sagem");
			theApp.m_ManufacturerID = AT_Sagem;
		}
		// Bomber, 2005.07.27
		//if (strstr((char*)msg.Buffer,"Samsung")) {
		if ((strstr((char*)msg.Buffer,"SAMSUNG"))||(strstr((char*)msg.Buffer,"Samsung"))) {
			smprintf(theApp.m_pDebuginfo, "Samsung\n");
			strcpy(theApp.m_pszTemp,"Samsung");
			theApp.m_ManufacturerID = AT_Samsung;
		}
		if (strstr((char*)msg.Buffer,"Motorola")) {
			smprintf(theApp.m_pDebuginfo, "Motorola\n");
			strcpy(theApp.m_pszTemp,"Motorola");
			theApp.m_ManufacturerID = AT_Motorola;
		}
		if ((strstr((char*)msg.Buffer,"SHARP"))||(strstr((char*)msg.Buffer,"Sharp"))) {
			smprintf(theApp.m_pDebuginfo, "Sharp\n");
			strcpy(theApp.m_pszTemp,"Sharp");
			theApp.m_ManufacturerID = AT_Sharp;
		}
		if ((strstr((char*)msg.Buffer,"PANASONIC"))||(strstr((char*)msg.Buffer,"Panasonic"))) {
			smprintf(theApp.m_pDebuginfo, "Panasonic\n");
			strcpy(theApp.m_pszTemp,"Panasonic");
			theApp.m_ManufacturerID = AT_Panasonic;
		}
		if ((strstr((char*)msg.Buffer,"LG"))||(strstr((char*)msg.Buffer,"lg"))) {
			smprintf(theApp.m_pDebuginfo, "LG\n");
			strcpy(theApp.m_pszTemp,"LG");
			theApp.m_ManufacturerID = AT_LG;
		}
		if ((strstr((char*)msg.Buffer,"TOSHIBA"))||(strstr((char*)msg.Buffer,"Toshiba"))) {
			smprintf(theApp.m_pDebuginfo, "Toshiba\n");
			strcpy(theApp.m_pszTemp,"Toshiba");
			theApp.m_ManufacturerID = AT_Toshiba;
		}
		if ((strstr((char*)msg.Buffer,"MTK"))||(strstr((char*)msg.Buffer,"mtk"))) {
			smprintf(theApp.m_pDebuginfo, "MTK\n");
			strcpy(theApp.m_pszTemp,"MTK");
			theApp.m_ManufacturerID = AT_MTK;
		}
		//0810 added by Mingfa for Sangfei , SMS bug 
        theApp.m_ManufacturerID = AT_MTK;

		return ERR_NONE;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_ReplyGetManufacturer(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	switch (Replynfo.ReplyState) {
	case AT_Reply_OK:
		smprintf(theApp.m_pDebuginfo, "Manufacturer info received\n");
		theApp.m_ManufacturerID = AT_Unknown;
		if (strlen(GetLineString(msg.Buffer, theApp.m_Lines, 2)) <= MAX_MANUFACTURER_LENGTH) {
			CopyLineString((unsigned char *)theApp.m_pszTemp, msg.Buffer, theApp.m_Lines, 2);
		} else {
			smprintf(theApp.m_pDebuginfo, "WARNING: Manufacturer name too long, increase MAX_MANUFACTURER_LENGTH to at least %zd\n", strlen(GetLineString(msg.Buffer, theApp.m_Lines, 2)));
			theApp.m_pszTemp[0] = 0;
		}
		/* Sometimes phone adds this before manufacturer (Sagem) */
		if (strncmp("+CGMI: ", theApp.m_pszTemp, 7) == 0) {
			memmove(theApp.m_pszTemp, theApp.m_pszTemp + 7, strlen(theApp.m_pszTemp + 7) + 1);
		}
	/*	if (strstr(msg.Buffer,"Falcom")) {
			smprintf(theApp.m_pDebuginfo, "Falcom\n");
			strcpy(theApp.m_pszTemp,"Falcom");
			theApp.m_ManufacturerID = AT_Falcom;
			if (strstr(msg.Buffer,"A2D")) {
				strcpy(s->Phone.Data.Model,"A2D");
				s->Phone.Data.ModelInfo = GetModelData(NULL,s->Phone.Data.Model,NULL);
				smprintf(theApp.m_pDebuginfo, "Model A2D\n");
			}
		}*/
		if (strstr((char*)msg.Buffer,"Nokia")) {
			smprintf(theApp.m_pDebuginfo, "Nokia\n");
			strcpy(theApp.m_pszTemp,"Nokia");
			theApp.m_ManufacturerID = AT_Nokia;
		}
		if (strstr((char*)msg.Buffer,"SIEMENS")) {
			smprintf(theApp.m_pDebuginfo, "Siemens\n");
			strcpy(theApp.m_pszTemp,"Siemens");
			theApp.m_ManufacturerID = AT_Siemens;
		}
		if (strstr((char*)msg.Buffer,"ERICSSON")) {
			smprintf(theApp.m_pDebuginfo, "Ericsson\n");
			strcpy(theApp.m_pszTemp,"Ericsson");
			theApp.m_ManufacturerID = AT_Ericsson;
		}
		if (strstr((char*)msg.Buffer,"Sony Ericsson")) {
			smprintf(theApp.m_pDebuginfo, "Ericsson\n");
			strcpy(theApp.m_pszTemp,"Ericsson");
			theApp.m_ManufacturerID = AT_Ericsson;
		}
		if (strstr((char*)msg.Buffer,"iPAQ")) {
			smprintf(theApp.m_pDebuginfo, "iPAQ\n");
			strcpy(theApp.m_pszTemp,"HP");
			theApp.m_ManufacturerID = AT_HP;
		}
		if (strstr((char*)msg.Buffer,"ALCATEL")) {
			smprintf(theApp.m_pDebuginfo, "Alcatel\n");
			strcpy(theApp.m_pszTemp,"Alcatel");
			theApp.m_ManufacturerID = AT_Alcatel;
		}
		if (strstr((char*)msg.Buffer,"SAGEM")) {
			smprintf(theApp.m_pDebuginfo, "Sagem\n");
			strcpy(theApp.m_pszTemp,"Sagem");
			theApp.m_ManufacturerID = AT_Sagem;
		}
		// Bomber, 2005.07.27
		//if (strstr((char*)msg.Buffer,"Samsung")) {
		if ((strstr((char*)msg.Buffer,"SAMSUNG"))||(strstr((char*)msg.Buffer,"Samsung"))) {
			smprintf(theApp.m_pDebuginfo, "Samsung\n");
			strcpy(theApp.m_pszTemp,"Samsung");
			theApp.m_ManufacturerID = AT_Samsung;
		}
		if (strstr((char*)msg.Buffer,"Motorola")) {
			smprintf(theApp.m_pDebuginfo, "Motorola\n");
			strcpy(theApp.m_pszTemp,"Motorola");
			theApp.m_ManufacturerID = AT_Motorola;
		}
		if ((strstr((char*)msg.Buffer,"SHARP"))||(strstr((char*)msg.Buffer,"Sharp"))) {
			smprintf(theApp.m_pDebuginfo, "Sharp\n");
			strcpy(theApp.m_pszTemp,"Sharp");
			theApp.m_ManufacturerID = AT_Sharp;
		}
		if ((strstr((char*)msg.Buffer,"PANASONIC"))||(strstr((char*)msg.Buffer,"Panasonic"))) {
			smprintf(theApp.m_pDebuginfo, "Panasonic\n");
			strcpy(theApp.m_pszTemp,"Panasonic");
			theApp.m_ManufacturerID = AT_Panasonic;
		}
		if ((strstr((char*)msg.Buffer,"LG"))||(strstr((char*)msg.Buffer,"lg"))) {
			smprintf(theApp.m_pDebuginfo, "LG\n");
			strcpy(theApp.m_pszTemp,"LG");
			theApp.m_ManufacturerID = AT_LG;
		}
		if ((strstr((char*)msg.Buffer,"TOSHIBA"))||(strstr((char*)msg.Buffer,"Toshiba"))) {
			smprintf(theApp.m_pDebuginfo, "Toshiba\n");
			strcpy(theApp.m_pszTemp,"Toshiba");
			theApp.m_ManufacturerID = AT_Toshiba;
		}
		if ((strstr((char*)msg.Buffer,"MTK"))||(strstr((char*)msg.Buffer,"mtk"))) {
			smprintf(theApp.m_pDebuginfo, "MTK\n");
			strcpy(theApp.m_pszTemp,"MTK");
			theApp.m_ManufacturerID = AT_MTK;
		}
		//08/10 added by Mingfa for Sangfei SMS bug 
        theApp.m_ManufacturerID = AT_MTK;

		return ERR_NONE;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(Replynfo);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_ReplyCDMAGetModel(GSM_Protocol_Message msg)
{
	GSM_ATReplayInfo Replynfo;
	ATGEN_GetReplyStatue(&msg,&Replynfo);

	if (Replynfo.ReplyState != AT_Reply_OK) return ERR_NOTSUPPORTED;

	if (strlen(GetLineString(msg.Buffer, theApp.m_Lines, 2)) <= MAX_MODEL_LENGTH)
	{
		CopyLineString((unsigned char *)theApp.m_pszTemp, msg.Buffer, theApp.m_Lines, 2);

		/* Sometimes phone adds this before manufacturer (Sagem) */
		if (strncmp("+GMM: ", theApp.m_pszTemp, 6) == 0) {
			memmove(theApp.m_pszTemp, theApp.m_pszTemp + 6, strlen(theApp.m_pszTemp + 6) + 1);
		}
	} else {
		smprintf(theApp.m_pDebuginfo, "WARNING: Model name too long, increase MAX_MODEL_LENGTH to at least %zd\n", strlen(GetLineString(msg.Buffer, theApp.m_Lines, 2)));
	}
	return ERR_NONE;
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
GSM_Error ATGEN_InitParameter(OnePhoneModel *pMobileInfo,Debug_Info	*pDebuginfo)
{
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
	return ERR_NONE;
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
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"ATZ\r");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"ATZ\r", 4, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_GenericReply);

	if (error != ERR_NONE)
	{
	//  error = GSM_WaitFor (s, "AT\r", 3, 0x00, 2, ID_IncomingFrame);
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
GSM_Error ATGEN_GetNetworkInfo(GSM_NetworkInfo *netinfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	GSM_Error error;
	theApp.m_pDebuginfo = pDebuginfo;

	theApp.m_NetworkInfo=netinfo;

	netinfo->NetworkName[0] = 0;
	netinfo->NetworkName[1] = 0;
	netinfo->NetworkCode[0] = 0;

	smprintf(theApp.m_pDebuginfo, "Enable full network info\n");

	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CREG=2");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error=pWriteCommandfn ((unsigned char *)"AT+CREG=2\r", 10, 0x00, 8, false,NULL,&ReplymsgType,ATGEN_GenericReply);
//	error=GSM_WaitFor(s, "AT+CREG=2\r", 10, 0x00, 4, ID_GetNetworkInfo);
	if ((error != ERR_NONE) &&   (theApp.m_ManufacturerID!=AT_Siemens) &&(theApp.m_ManufacturerID!=AT_Ericsson)) 
		return error;

	smprintf(theApp.m_pDebuginfo, "Getting network LAC and CID and state\n");

	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CREG?");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error=pWriteCommandfn ((unsigned char *)"AT+CREG?\r", 9, 0x00, 8, false,NULL,&ReplymsgType,ATGEN_ReplyGetNetworkLAC_CID);
//	error=GSM_WaitFor(s, "AT+CREG?\r", 9, 0x00, 4, ID_GetNetworkInfo);
	if (error != ERR_NONE) return error;

	if (netinfo->State == GSM_HomeNetwork || netinfo->State == GSM_RoamingNetwork) {
		smprintf(theApp.m_pDebuginfo, "Setting short network name format\n");
	
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+COPS=");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		error=pWriteCommandfn ((unsigned char *)"AT+COPS=3,2\r", 12, 0x00, 8, false,NULL,&ReplymsgType,ATGEN_GenericReply);
	//	error=GSM_WaitFor(s, "AT+COPS=3,2\r", 12, 0x00, 4, ID_GetNetworkInfo);
		char szManufacturer[MAX_PATH];
		error=ATGEN_GetManufacturer(szManufacturer,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;

		smprintf(theApp.m_pDebuginfo, "Getting network code\n");
		ReplymsgType.nCount = 1;
		wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+COPS");
		ReplymsgType.CheckInfo[0].subtypechar = 0;
		ReplymsgType.CheckInfo[0].subtype = 0x00;
		error=pWriteCommandfn ((unsigned char *)"AT+COPS?\r", 9, 0x00, 8, false,NULL,&ReplymsgType,ATGEN_ReplyGetNetworkCode);
//		error=GSM_WaitFor(s, "AT+COPS?\r", 9, 0x00, 4, ID_GetNetworkInfo);
	}
	return error;
}



GSM_Error ATGEN_GetSignalQuality(GSM_SignalQuality *sig,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	theApp.m_SignalQuality = sig;
	//return GSM_WaitFor (s, "AT+CSQ\r", 7, 0x00, 4, ATGEN_ReplyGetSignalQuality);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CSQ");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return pWriteCommandfn ((unsigned char *)"AT+CSQ\r", 7, 0x00, 15, false,NULL,&ReplymsgType,ATGEN_ReplyGetSignalQuality);
}


GSM_Error ATGEN_GetBatteryCharge(GSM_BatteryCharge *bat,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_BatteryCharge = bat;
//	return GSM_WaitFor (s, "AT+CBC\r", 7, 0x00, 4, ID_GetBatteryCharge);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CBC");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return pWriteCommandfn ((unsigned char *)"AT+CBC\r", 7, 0x00, 15, false,NULL,&ReplymsgType,ATGEN_ReplyGetBatteryCharge);
}

GSM_Error ATGEN_GetModel2(char* pszModel,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error error;

	smprintf(theApp.m_pDebuginfo, "Getting model\n");
	theApp.m_pszTemp = pszModel;
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"ATI");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error=pWriteCommandfn ((unsigned char *)"ATI\r", 4, 0x00, 6, false,NULL,&ReplymsgType,ATGEN_ReplyGetModel);

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
GSM_Error ATGEN_CDMA_GetModel(char* pszModel,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error error;
	theApp.m_pszTemp = pszModel;
	ReplymsgType.nCount = 1;
	sprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+GMM");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error=pWriteCommandfn ((unsigned char *)"AT+GMM\r", 7, 0x00, 10, false,NULL,&ReplymsgType,ATGEN_ReplyCDMAGetModel);
	return error;
}
GSM_Error ATGEN_CDMA_GetIMEI (char* pszIMEI,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	theApp.m_pDebuginfo = pDebuginfo;
	theApp.m_pszTemp = pszIMEI;
	ReplymsgType.nCount = 1;
	sprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+GSN");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return pWriteCommandfn ((unsigned char *)"AT+GSN\r", 7, 0x00, 10, false,NULL,&ReplymsgType,ATGEN_ReplyGetIMEI);
}

GSM_Error ATGEN_CDMA_GetManufacturer(char* pszManufacturer,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	theApp.m_pszTemp = pszManufacturer;
	ReplymsgType.nCount = 1;
	sprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+GMI");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return pWriteCommandfn ((unsigned char *)"AT+GMI\r", 7, 0x00, 12, false,NULL,&ReplymsgType,ATGEN_ReplyCDMAGetManufacturer);
}
GSM_Error ATGEN_CDMA_GetFirmware(char* pszFirmwareVer,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error error;
	theApp.m_pDebuginfo = pDebuginfo;

	smprintf(theApp.m_pDebuginfo, "Getting firmware - method 2\n");

	theApp.m_pszTemp = pszFirmwareVer;
	ReplymsgType.nCount = 1;
	sprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+GMR");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error = pWriteCommandfn ((unsigned char *)"AT+GMR\r", 7, 0x00, 8, false,NULL,&ReplymsgType,ATGEN_ReplyGetFirmwareGMR);
	return error;
}
