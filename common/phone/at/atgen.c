                                           
#include "../../gsmstate.h"

#ifdef GSM_ENABLE_ATGEN

#include <string.h>
#include <time.h>
#include <ctype.h>

#include "../../gsmcomon.h"
#include "../../misc/coding.h"
#include "../../service/gsmsms.h"
#include "../pfunc.h"
#include "atgen.h"

#ifdef GSM_ENABLE_ALCATEL
extern GSM_Error ALCATEL_ProtocolVersionReply	(GSM_Protocol_Message msg, GSM_StateMachine *s);
#endif

extern GSM_Error ATGEN_CMS35ReplyGetBitmap	(GSM_Protocol_Message msg, GSM_StateMachine *s);
extern GSM_Error ATGEN_CMS35ReplySetBitmap	(GSM_Protocol_Message msg, GSM_StateMachine *s);
extern GSM_Error ATGEN_CMS35ReplyGetRingtone	(GSM_Protocol_Message msg, GSM_StateMachine *s);
extern GSM_Error ATGEN_CMS35ReplySetRingtone	(GSM_Protocol_Message msg, GSM_StateMachine *s);
extern GSM_Error ATGEN_CMS35ReplyGetNextCal	(GSM_Protocol_Message msg, GSM_StateMachine *s);
extern GSM_Error ATGEN_CMS35ReplySetCalendar	(GSM_Protocol_Message msg, GSM_StateMachine *s);
extern GSM_Error ATGEN_CMS35ReplyDeleteCalendar	(GSM_Protocol_Message msg, GSM_StateMachine *s);
extern GSM_Error ATGEN_SL45ReplyGetMemory	(GSM_Protocol_Message msg, GSM_StateMachine *s);

extern GSM_Error ATGEN_GetRingtone	(GSM_StateMachine *s, GSM_Ringtone *Ringtone, bool PhoneRingtone);
extern GSM_Error ATGEN_SetRingtone	(GSM_StateMachine *s, GSM_Ringtone *Ringtone, int *maxlength);
extern GSM_Error ATGEN_GetBitmap	(GSM_StateMachine *s, GSM_Bitmap *Bitmap);
extern GSM_Error ATGEN_SetBitmap	(GSM_StateMachine *s, GSM_Bitmap *Bitmap);
extern GSM_Error ATGEN_GetNextCalendar	(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start);
extern GSM_Error ATGEN_DelCalendarNote	(GSM_StateMachine *s, GSM_CalendarEntry *Note);
extern GSM_Error ATGEN_AddCalendarNote	(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool Past);

typedef struct {
    int     Number;
    char    Text[60];
} ATErrorCode;

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

GSM_Error ATGEN_HandleCMSError(GSM_StateMachine *s)
{
    GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

    if (Priv->ErrorCode == 0) {
        smprintf(s, "CMS Error occured, but it's type not detected\n");
    } else if (Priv->ErrorText == NULL) {
        smprintf(s, "CMS Error %i, no description available\n", Priv->ErrorCode);
    } else {
        smprintf(s, "CMS Error %i: \"%s\"\n", Priv->ErrorCode, Priv->ErrorText);
    }
    /* For error codes descriptions see table a bit above */
    switch (Priv->ErrorCode) {
        case 304:
        case 305:
            	return GE_BUG; 
        case 311:
        case 312:
        case 316:
        case 317:
        case 318:
            	return GE_SECURITYERROR;
        case 322:
            	return GE_FULL;
        case 321:
            	return GE_INVALIDLOCATION;
        default:
		return GE_UNKNOWN;
    }
}

/* FIXME: Function doesn't respect quoting of parameters and thus +FOO:
 *        "ab","cd,ef" will consider as three arguments: "ab" >> "cd >> ef"
 */
int ATGEN_ExtractOneParameter(unsigned char *input, unsigned char *output)
{
	int position=0;

	while (*input!=',' && *input!=0x0d && *input!=0x00) {
		*output=*input;
		input	++;
		output	++;
		position++;
	}
	*output=0;
	position++;
	return position;
}

void ATGEN_DecodeDateTime(GSM_DateTime *dt, unsigned char *input)
{
	dt->Year=2000+(*input-'0')*10;     input++;
	dt->Year=dt->Year+(*input-'0');    input++;

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
		input=input-2;
		if (*input=='-') dt->Timezone=-dt->Timezone;
	}
}

GSM_Error ATGEN_Initialise(GSM_StateMachine *s)
{
	GSM_Phone_ATGENData     *Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error               error;
    	char                    buff[2];

	Priv->SMSMode			= 0;
	Priv->Manufacturer		= 0;
	Priv->PhoneSMSMemory		= 0;
	Priv->SMSMemory			= 0;
	Priv->PBKMemory			= 0;
	Priv->PBKSBNR			= 0;
	Priv->PBKCharset		= 0;
 	Priv->UCS2CharsetFailed		= false;
 	Priv->NonUCS2CharsetFailed	= false;
	Priv->PBKMemories[0]		= 0;
	Priv->FirstCalendarPos		= 0;
	Priv->NextMemoryEntry		= 0;

	Priv->ErrorText			= NULL;

	if (s->ConnectionType != GCT_IRDAAT && s->ConnectionType != GCT_BLUEAT) {
		/* We try to escape AT+CMGS mode, at least Siemens M20
		 * then needs to get some rest
		 */
		smprintf(s, "Escaping SMS mode\n");
		error = s->Protocol.Functions->WriteMessage(s, "\x1B\r", 2, 0x00);
		if (error!=GE_NONE) return error;

	    	/* Grab any possible garbage */
	    	while (s->Device.Functions->ReadDevice(s, buff, 2) > 0) my_sleep(10);
	}

    	/* When some phones (Alcatel BE5) is first time connected, it needs extra
     	 * time to react, sending just AT wakes up the phone and it then can react
     	 * to ATE1. We don't need to check whether this fails as it is just to
     	 * wake up the phone and does nothing.
     	 */
    	smprintf(s, "Sending simple AT command to wake up some devices\n");
	GSM_WaitFor (s, "AT\r", 3, 0x00, 2, ID_IncomingFrame);

	smprintf(s, "Enabling echo\n");
	return GSM_WaitFor (s, "ATE1\r", 5, 0x00, 3, ID_EnableEcho);
}

GSM_Error ATGEN_DispatchMessage(GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv 	= &s->Phone.Data.Priv.ATGEN;
	GSM_Protocol_Message	*msg	= s->Phone.Data.RequestMsg;
	int 			i	= 0;
	int                     j,k;
	char                    *err;

	SplitLines(msg->Buffer, msg->Length, &Priv->Lines, "\x0D\x0A", 2, true);

	/* Find number of lines */
	while (1) {
		if (Priv->Lines.numbers[i*2+1]==0) break;
		/* FIXME: handle special chars correctly */
		smprintf(s, "%i \"%s\"\n",i+1,GetLineString(msg->Buffer,Priv->Lines,i+1));
		i++;
	}

	Priv->ReplyState 	= AT_Reply_Unknown;
	Priv->ErrorText     	= NULL;
	Priv->ErrorCode     	= 0;

	if (!strcmp(GetLineString(msg->Buffer,Priv->Lines,i),"OK"     )) Priv->ReplyState = AT_Reply_OK;
	if (!strcmp(GetLineString(msg->Buffer,Priv->Lines,i),"CONNECT")) Priv->ReplyState = AT_Reply_Connect;
	if (!strcmp(GetLineString(msg->Buffer,Priv->Lines,i),"ERROR"  )) Priv->ReplyState = AT_Reply_Error;
	if (!strncmp(GetLineString(msg->Buffer,Priv->Lines,i),"+CMS ERROR:",11)) {
	        j = 0;
		/* One char behind +CMS ERROR */
		err = GetLineString(msg->Buffer,Priv->Lines,i) + 12;
		while (err[j] && !isalnum(err[j])) j++;
		if (isdigit(err[j])) {
			Priv->ErrorCode = atoi(&(err[j]));
			k = 0;
			while (CMSErrorCodes[k].Number != -1) {
				if (CMSErrorCodes[k].Number == Priv->ErrorCode) {
					Priv->ErrorText = (char *)&(CMSErrorCodes[k].Text);
					break;
				}
				k++;
			}
		} else if (isalpha(err[j])) {
			k = 0;
			while (CMSErrorCodes[k].Number != -1) {
				if (!strncmp(err + j, CMSErrorCodes[k].Text, strlen(CMSErrorCodes[k].Text))) {
					Priv->ErrorCode = CMSErrorCodes[k].Number;
					Priv->ErrorText = (char *)&(CMSErrorCodes[k].Text);
					break;
				}
				k++;
			}
		}
		Priv->ReplyState = AT_Reply_CMSError;
	}
	if (!strncmp(GetLineString(msg->Buffer,Priv->Lines,i),"+CME ERROR:",11)) {
		Priv->ReplyState = AT_Reply_CMEError;
	}
	return GSM_DispatchMessage(s);
}

GSM_Error ATGEN_GenericReply(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
		case AT_Reply_OK:
		case AT_Reply_Connect:
			return GE_NONE;
		case AT_Reply_Error:
		case AT_Reply_CMSError:
			return ATGEN_HandleCMSError(s);
		case AT_Reply_CMEError:
	        	return GE_UNKNOWN;
		default:
			break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_ReplyGetModel(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	if (strlen(GetLineString(msg.Buffer, Priv->Lines, 2)) <= MAX_MODEL_LENGTH) {
		CopyLineString(Data->Model, msg.Buffer, Priv->Lines, 2);

		Data->ModelInfo = GetModelData(NULL,Data->Model,NULL);
		if (Data->ModelInfo->number[0] == 0) Data->ModelInfo = GetModelData(NULL,NULL,Data->Model);
		if (Data->ModelInfo->number[0] == 0) Data->ModelInfo = GetModelData(Data->Model,NULL,NULL);

		if (Data->ModelInfo->number[0] != 0) strcpy(Data->Model,Data->ModelInfo->number);

		if (strstr(msg.Buffer,"Nokia")) 	Priv->Manufacturer = AT_Nokia;
		else if (strstr(msg.Buffer,"M20")) 	Priv->Manufacturer = AT_Siemens;
		else if (strstr(msg.Buffer,"MC35")) 	Priv->Manufacturer = AT_Siemens;
		else if (strstr(msg.Buffer, "iPAQ")) 	Priv->Manufacturer = AT_HP;

		if (strstr(msg.Buffer,"M20")) 		strcpy(Data->Model,"M20");
		else if (strstr(msg.Buffer,"MC35")) 	strcpy(Data->Model,"MC35");
		else if (strstr(msg.Buffer, "iPAQ")) 	strcpy(Data->Model,"iPAQ");
	} else {
		smprintf(s, "WARNING: Model name too long, increase MAX_MODEL_LENGTH to at least %d\n", GetLineString(msg.Buffer, Priv->Lines, 2));
	}

	return GE_NONE;
}

GSM_Error ATGEN_GetModel(GSM_StateMachine *s)
{
	GSM_Error error;

	if (s->Phone.Data.Model[0] != 0) return GE_NONE;

	smprintf(s, "Getting model\n");
	error=GSM_WaitFor (s, "AT+CGMM\r", 8, 0x00, 3, ID_GetModel);
	if (error==GE_NONE) {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
			smprintf(s, "[Connected model  - \"%s\"]\n",s->Phone.Data.Model);
		}
	}
	return error;
}

GSM_Error ATGEN_ReplyGetManufacturer(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Manufacturer info received\n");
		Priv->Manufacturer = AT_Unknown;
		if (strlen(GetLineString(msg.Buffer, Priv->Lines, 2)) <= MAX_MANUFACTURER_LENGTH) {
			CopyLineString(s->Phone.Data.Manufacturer, msg.Buffer, Priv->Lines, 2);
		} else {
			smprintf(s, "WARNING: Manufacturer name too long, increase MAX_MANUFACTURER_LENGTH to at least %d\n", GetLineString(msg.Buffer, Priv->Lines, 2));
			s->Phone.Data.Manufacturer[0] = 0;
		}
		if (strstr(msg.Buffer,"Falcom")) {
			smprintf(s, "Falcom\n");
			strcpy(s->Phone.Data.Manufacturer,"Falcom");
			Priv->Manufacturer = AT_Falcom;
			if (strstr(msg.Buffer,"A2D")) {
				strcpy(s->Phone.Data.Model,"A2D");
				s->Phone.Data.ModelInfo = GetModelData(NULL,s->Phone.Data.Model,NULL);
				smprintf(s, "Model A2D\n");
			}
		}
		if (strstr(msg.Buffer,"Nokia")) {
			smprintf(s, "Nokia\n");
			strcpy(s->Phone.Data.Manufacturer,"Nokia");
			Priv->Manufacturer = AT_Nokia;
		}
		if (strstr(msg.Buffer,"SIEMENS")) {
			smprintf(s, "Siemens\n");
			strcpy(s->Phone.Data.Manufacturer,"Siemens");
			Priv->Manufacturer = AT_Siemens;
		}
		if (strstr(msg.Buffer,"ERICSSON")) {
			smprintf(s, "Ericsson\n");
			strcpy(s->Phone.Data.Manufacturer,"Ericsson");
			Priv->Manufacturer = AT_Ericsson;
		}
		if (strstr(msg.Buffer,"iPAQ")) {
			smprintf(s, "iPAQ\n");
			strcpy(s->Phone.Data.Manufacturer,"HP");
			Priv->Manufacturer = AT_HP;
		}
		if (strstr(msg.Buffer,"ALCATEL")) {
			smprintf(s, "Alcatel\n");
			strcpy(s->Phone.Data.Manufacturer,"Alcatel");
			Priv->Manufacturer = AT_Alcatel;
		}
		return GE_NONE;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetManufacturer(GSM_StateMachine *s)
{
	if (s->Phone.Data.Manufacturer[0] != 0) return GE_NONE;

	return GSM_WaitFor (s, "AT+CGMI\r", 8, 0x00, 4, ID_GetManufacturer);
}

GSM_Error ATGEN_ReplyGetFirmwareCGMR(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	unsigned int		i = 0;

	strcpy(s->Phone.Data.Version,"unknown");
	s->Phone.Data.VerNum = 0;
	if (Priv->ReplyState == AT_Reply_OK) {
		CopyLineString(s->Phone.Data.Version, msg.Buffer, Priv->Lines, 2);
	}
	if (Priv->Manufacturer == AT_Ericsson) {
		while (1) {
			if (s->Phone.Data.Version[i] == 0x20) {
				s->Phone.Data.Version[i] = 0x00;
				break;
			}
			if (i == strlen(s->Phone.Data.Version)) break;
			i++;
		}
	}
	smprintf(s, "Received firmware version: \"%s\"\n",s->Phone.Data.Version);
	GSM_CreateFirmwareNumber(s);
	return GE_NONE;
}

GSM_Error ATGEN_ReplyGetFirmwareATI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
//		strcpy(Data->Version,"0.00");
//		*Data->VersionNum=0;
//		if (Data->Priv.ATGEN.ReplyState==AT_Reply_OK) {
//			CopyLineString(Data->Version, msg.Buffer, Priv->Lines, 2);
//		}
//		smprintf(s, "Received firmware version: \"%s\"\n",Data->Version);
//		GSM_CreateFirmwareNumber(Data);
//		return GE_NONE;
	case AT_Reply_Error:
		return GE_NOTSUPPORTED;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetFirmware(GSM_StateMachine *s)
{
	GSM_Error error;

	if (s->Phone.Data.Version[0] != 0) return GE_NONE;

	error=ATGEN_GetManufacturer(s);
	if (error != GE_NONE) return error;

//	smprintf(s, "Getting firmware - method 1\n");
//	error=GSM_WaitFor (s, "ATI\r", 4, 0x00, 3, ID_GetFirmware);
//	if (error != GE_NONE) {
		smprintf(s, "Getting firmware - method 2\n");
		error=GSM_WaitFor (s, "AT+CGMR\r", 8, 0x00, 3, ID_GetFirmware);
//	}
	if (error==GE_NONE) {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
			smprintf(s, "[Firmware version - \"%s\"]\n",s->Phone.Data.Version);
		}
	}
	return error;
}

GSM_Error ATGEN_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	unsigned char req[50];

	if (smsc->Location!=1) return GE_NOTSUPPORTED;

	sprintf(req, "AT+CSCA=\"%s\"\r",DecodeUnicodeString(smsc->Number));

	smprintf(s, "Setting SMSC\n");
	return GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_SetSMSC);
}

GSM_Error ATGEN_ReplyGetSMSMemories(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		dprintf("Memory status received\n");
		if (strncmp(msg.Buffer, "ME", 2) == 0) s->Phone.Data.Priv.ATGEN.PhoneSMSMemory = AT_PHONE_SMS_AVAILABLE;
		return GE_NONE;
	case AT_Reply_Error:
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	default:
		return GE_UNKNOWNRESPONSE;
	}
}

GSM_Error ATGEN_GetSMSMemories(GSM_StateMachine *s)
{
	dprintf("Getting available SMS memories\n");
	return GSM_WaitFor (s, "AT+CPMS?\r", 9, 0x00, 4, ID_GetSMSMemories);
}

GSM_Error ATGEN_SetSMSMemory(GSM_StateMachine *s, bool SIM)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	char 			req[] = "AT+CPMS=\"XX\",\"XX\"\r";
	GSM_Error		error;

	if (SIM) {
		if (Priv->SMSMemory == GMT_SM) return GE_NONE;
		req[9]  = 'S'; req[10] = 'M';
		req[14] = 'S'; req[15] = 'M';
		smprintf(s, "Setting SMS memory type to SM\n");
		error=GSM_WaitFor (s, req, 18, 0x00, 3, ID_SetMemoryType);
		if (error == GE_NOTSUPPORTED) {
			smprintf(s, "Can't access SIM card?\n");
			return GE_SECURITYERROR;
		}
		if (error != GE_NONE) return error;
		Priv->SMSMemory = GMT_SM;
	} else {
		if (Priv->PhoneSMSMemory == AT_PHONE_SMS_NOTAVAILABLE) {
			return GE_NOTSUPPORTED;
		}
		req[9]  = 'M'; req[10] = 'E';
		req[14] = 'M'; req[15] = 'E';
		smprintf(s, "Setting SMS memory type to ME\n");
		error=GSM_WaitFor (s, req, 18, 0x00, 3, ID_SetMemoryType);
		if (Priv->PhoneSMSMemory == 0) {
			if (error != GE_NONE) {
				Priv->PhoneSMSMemory = AT_PHONE_SMS_NOTAVAILABLE;
				error = ATGEN_GetSMSMemories(s);
				if (error != GE_NONE) return GE_NOTSUPPORTED;
			} else {
				Priv->PhoneSMSMemory = AT_PHONE_SMS_AVAILABLE;
			}
		}
		if (error == GE_NONE) Priv->SMSMemory = GMT_ME;
	}
	return error;
}

GSM_Error ATGEN_GetSMSMode(GSM_StateMachine *s)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
  	GSM_Error 		error;

	if (Priv->SMSMode != 0) return GE_NONE;

	smprintf(s, "Trying SMS PDU mode\n");
	error=GSM_WaitFor (s, "AT+CMGF=0\r", 10, 0x00, 3, ID_GetSMSMode);
	if (error==GE_NONE) {
		Priv->SMSMode = SMS_AT_PDU;
		return GE_NONE;
	}

	smprintf(s, "Trying SMS text mode\n");
	error=GSM_WaitFor (s, "AT+CMGF=1\r", 10, 0x00, 3, ID_GetSMSMode);
	if (error==GE_NONE) {
		smprintf(s, "Enabling displaying all parameters in text mode\n");
		error=GSM_WaitFor (s, "AT+CSDH=1\r", 10, 0x00, 3, ID_GetSMSMode);
		if (error == GE_NONE) Priv->SMSMode = SMS_AT_TXT;
	}

	return error;
}

GSM_Error ATGEN_GetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char *folderid, int *location)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	int			ifolderid;

	if (Priv->PhoneSMSMemory == AT_PHONE_SMS_NOTAVAILABLE) {
		if (sms->Folder>0x01) return GE_NOTSUPPORTED;
	} else {
		if (sms->Folder>0x02) return GE_NOTSUPPORTED;
	}

	/* simulate flat SMS memory */
	if (sms->Folder==0x00) {
		ifolderid = sms->Location / PHONE_MAXSMSINFOLDER;
		*folderid = ifolderid + 1;
		*location = sms->Location - ifolderid * PHONE_MAXSMSINFOLDER;
	} else {
		*folderid = sms->Folder;
		*location = sms->Location;
	}
	smprintf(s, "SMS folder %i & location %i -> ATGEN folder %i & location %i\n",
		sms->Folder,sms->Location,*folderid,*location);

	if (*folderid==2) {
		return ATGEN_SetSMSMemory(s, false);
	} else {
		return ATGEN_SetSMSMemory(s, true);
	}
}

void ATGEN_SetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char folderid, int location)
{
	sms->Folder	= 0;
	sms->Location	= (folderid - 1) * PHONE_MAXSMSINFOLDER + location;
	smprintf(s, "ATGEN folder %i & location %i -> SMS folder %i & location %i\n",
		folderid,location,sms->Folder,sms->Location);
}

GSM_Error ATGEN_ReplyGetSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv 	= &s->Phone.Data.Priv.ATGEN;
	GSM_SMSMessage		*sms	= &s->Phone.Data.GetSMSMessage->SMS[0];
	int 			current = 0, current2, i;
	unsigned char 		buffer[300],smsframe[800];
	unsigned char		firstbyte, TPDCS, TPUDL, TPStatus;
	GSM_Error		error;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		if (Priv->Lines.numbers[4] == 0x00) return GE_EMPTY;
		s->Phone.Data.GetSMSMessage->Number 	 	= 1;
		s->Phone.Data.GetSMSMessage->SMS[0].Name[0] 	= 0;
		s->Phone.Data.GetSMSMessage->SMS[0].Name[1]	= 0;
		switch (Priv->SMSMode) {
		case SMS_AT_PDU:
			CopyLineString(buffer, msg.Buffer, Priv->Lines, 2);
			switch (buffer[7]) {
				case '0': sms->State = GSM_UnRead; 	break;
				case '1': sms->State = GSM_Read;	break;
				case '2': sms->State = GSM_UnSent;	break;
				default : sms->State = GSM_Sent;	break;//case '3'
			}
			DecodeHexBin (buffer, GetLineString(msg.Buffer,Priv->Lines,3), strlen(GetLineString(msg.Buffer,Priv->Lines,3)));
			/* Siemens MC35 (only ?) */
			if (strstr(msg.Buffer,"+CMGR: 0,,0")!=NULL) return GE_EMPTY;
			/* Siemens M20 */
			if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_M20SMS)) {
				if (buffer[1]!=GNT_UNKNOWN && buffer[1]!=GNT_INTERNATIONAL &&
				    buffer[1]!=GNT_ALPHANUMERIC) {
					/* Seems to be Delivery Report */
					smprintf(s, "SMS type - status report (M20 style)\n");
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
					return GE_NONE;					
				}
			}
			/* We use locations from SMS layouts like in ../phone2.c(h) */
			for(i=0;i<buffer[0]+1;i++) smsframe[i]=buffer[current++];
			smsframe[12]=buffer[current++];
			/* See GSM 03.40 section 9.2.3.1 */
			switch (smsframe[12] & 0x03) {
			case 0x00:
				smprintf(s, "SMS type - deliver\n");
				sms->PDU 	 = SMS_Deliver;
				if (Priv->SMSMemory == GMT_SM) {
					sms->Folder = 1; /*INBOX SIM*/ 
				} else {
					sms->Folder = 3; /*INBOX ME*/ 
				}
				sms->InboxFolder = true;
				current2=((buffer[current])+1)/2+1;
				if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_M20SMS)) {
					if (buffer[current+1]==GNT_ALPHANUMERIC) {
						smprintf(s, "Trying to read alphanumeric number\n");
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
				return GE_NONE;
			case 0x01:
				smprintf(s, "SMS type - submit\n");
				sms->PDU 	 = SMS_Submit;
				if (Priv->SMSMemory == GMT_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/ 
				} else {
					sms->Folder = 4; /*OUTBOX ME*/ 
				}
				sms->InboxFolder = false;
				smsframe[PHONE_SMSSubmit.TPMR] = buffer[current++];
				current2=((buffer[current])+1)/2+1;
				if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_M20SMS)) {
					if (buffer[current+1]==GNT_ALPHANUMERIC) {
						smprintf(s, "Trying to read alphanumeric number\n");
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
				return GE_NONE;
			case 0x02:
				smprintf(s, "SMS type - status report\n");
				sms->PDU 	 = SMS_Status_Report;
				sms->Folder 	 = 1;	/*INBOX SIM*/
				sms->InboxFolder = true;
				smsframe[PHONE_SMSStatusReport.TPMR] = buffer[current++];
				current2=((buffer[current])+1)/2+1;
				for(i=0;i<current2+1;i++) smsframe[PHONE_SMSStatusReport.Number+i]=buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.DateTime+i]=buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReport.SMSCTime+i]=buffer[current++];
				smsframe[PHONE_SMSStatusReport.TPStatus]=buffer[current];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSStatusReport);
				return GE_NONE;
			}
			break;
		case SMS_AT_TXT:
			current = 0;
			while (msg.Buffer[current]!='"') current++;
			current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
			if (!strcmp(buffer,"\"0\"") || !strcmp(buffer,"\"REC UNREAD\"")) {
				smprintf(s, "SMS type - deliver\n");
				sms->State 	 = GSM_UnRead;
				sms->PDU 	 = SMS_Deliver;
				if (Priv->SMSMemory == GMT_SM) {
					sms->Folder = 1; /*INBOX SIM*/ 
				} else {
					sms->Folder = 3; /*INBOX ME*/ 
				}
				sms->InboxFolder = true;
			} else if (!strcmp(buffer,"\"1\"") || !strcmp(buffer,"\"REC READ\"")) {
				smprintf(s, "SMS type - deliver\n");
				sms->State 	 = GSM_Read;
				sms->PDU 	 = SMS_Deliver;
				if (Priv->SMSMemory == GMT_SM) {
					sms->Folder = 1; /*INBOX SIM*/ 
				} else {
					sms->Folder = 3; /*INBOX ME*/ 
				}
				sms->InboxFolder = true;
			} else if (!strcmp(buffer,"\"2\"") || !strcmp(buffer,"\"STO UNSENT\"")) {
				smprintf(s, "SMS type - submit\n");
				sms->State 	 = GSM_UnSent;
				sms->PDU 	 = SMS_Submit;
				if (Priv->SMSMemory == GMT_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/ 
				} else {
					sms->Folder = 4; /*OUTBOX ME*/ 
				}
				sms->InboxFolder = false;
			} else if (!strcmp(buffer,"\"3\"") || !strcmp(buffer,"\"STO SENT\"")) {
				smprintf(s, "SMS type - submit\n");
				sms->State 	 = GSM_Sent;
				sms->PDU 	 = SMS_Submit;
				if (Priv->SMSMemory == GMT_SM) {
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
				EncodeUnicode(sms->Number,buffer+1,strlen(buffer)-2);
				smprintf(s, "Sender \"%s\"\n",DecodeUnicodeString(sms->Number));
				/* ??? */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* Sending datetime */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				i = strlen(buffer);
				buffer[i] = ',';
				i++;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
				smprintf(s, "\"%s\"\n",buffer);
				ATGEN_DecodeDateTime(&sms->DateTime, buffer+1);
				/* Date of SMSC response */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				i = strlen(buffer);
				buffer[i] = ',';
				i++;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
				smprintf(s, "\"%s\"\n",buffer);
				ATGEN_DecodeDateTime(&sms->SMSCTime, buffer+1);
				/* TPStatus */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPStatus=atoi(buffer);
				buffer[PHONE_SMSStatusReport.TPStatus] = TPStatus;
				error=GSM_DecodeSMSFrameStatusReportData(sms, buffer, PHONE_SMSStatusReport);
				if (error!=GE_NONE) return error;
				/* NO SMSC number */
				sms->SMSC.Number[0]=0;
				sms->SMSC.Number[1]=0;
				sms->PDU = SMS_Status_Report;
				sms->ReplyViaSameSMSC=false;
			} else {
				/* Sender number */
				/* FIXME: support for all formats */
				EncodeUnicode(sms->Number,buffer+1,strlen(buffer)-2);
				/* Sender number in alphanumeric format ? */
				current += ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				if (strlen(buffer)!=0) EncodeUnicode(sms->Number,buffer+1,strlen(buffer)-2);
				smprintf(s, "Sender \"%s\"\n",DecodeUnicodeString(sms->Number));
				/* Sending datetime */
				if (sms->Folder==1 || sms->Folder==3) {
					current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
					/* FIXME: ATGEN_ExtractOneParameter() is broken as it doesn't respect
					 * quoting of parameters and thus +FOO: "ab","cd,ef" will consider
					 * as three arguments: "ab" >> "cd >> ef"
					 */
					if (*buffer=='"') {
						i = strlen(buffer);
						buffer[i] = ',';
						i++;
						current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
					}
					smprintf(s, "\"%s\"\n",buffer);
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
				firstbyte=atoi(buffer);
				sms->ReplyViaSameSMSC=false;
				/* GSM 03.40 section 9.2.3.17 (TP-Reply-Path) */
				if ((firstbyte & 128)==128) sms->ReplyViaSameSMSC=true;
				/* TP PID */        
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				sms->ReplaceMessage = 0;
				if (atoi(buffer) > 0x40 && atoi(buffer) < 0x48) {
					sms->ReplaceMessage = atoi(buffer) - 0x40;
				}
				smprintf(s, "TPPID: %02x %i\n",atoi(buffer),atoi(buffer));
				/* TP DCS */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPDCS=atoi(buffer);
				if (sms->Folder==2 || sms->Folder==4) {
					/*TP VP */
					current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				}
				/* SMSC number */
				/* FIXME: support for all formats */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				EncodeUnicode(sms->SMSC.Number,buffer+1,strlen(buffer)-2);
				/* Format of SMSC number */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				/* TPUDL */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPUDL=atoi(buffer);
				current++;
				sms->Coding = GSM_Coding_Default;
				/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme)
				 * and GSM 03.38 section 4
				 */
				if ((TPDCS & 0xf4) == 0xf4) sms->Coding=GSM_Coding_8bit;
				if ((TPDCS & 0x08) == 0x08) sms->Coding=GSM_Coding_Unicode;
				sms->Class = -1;
				if ((TPDCS & 0xF3)==0xF0) sms->Class = 0;
				if ((TPDCS & 0xF3)==0xF1) sms->Class = 1;
				if ((TPDCS & 0xF3)==0xF2) sms->Class = 2;
				if ((TPDCS & 0xF3)==0xF3) sms->Class = 3;
				smprintf(s, "SMS class: %i\n",sms->Class);
				switch (sms->Coding) {          
				case GSM_Coding_Default:
					/* GSM 03.40 section 9.2.3.23 (TP-User-Data-Header-Indicator) */
					/* If not SMS with UDH, it's coded normal */
					/* If UDH available, treat it as Unicode or 8 bit */
					if ((firstbyte & 0x40)!=0x40) {
						sms->UDH.Type	= UDH_NoUDH;
						sms->Length	= TPUDL;
						EncodeUnicode(sms->Text,msg.Buffer+Priv->Lines.numbers[2*2],TPUDL);
						break;
					}
				case GSM_Coding_Unicode:
				case GSM_Coding_8bit:
					DecodeHexBin(buffer+PHONE_SMSDeliver.Text, msg.Buffer+current, TPUDL*2);
					buffer[PHONE_SMSDeliver.firstbyte] 	= firstbyte;
					buffer[PHONE_SMSDeliver.TPDCS] 		= TPDCS;
					buffer[PHONE_SMSDeliver.TPUDL] 		= TPUDL;
					return GSM_DecodeSMSFrameText(sms, buffer, PHONE_SMSDeliver);
				}
			}
			return GE_NONE;
		default:
			break;
		}
		break;
	case AT_Reply_CMSError:
		if (Priv->ErrorCode == 320 || Priv->ErrorCode == 500) {
			return GE_EMPTY;
		} else {
			return ATGEN_HandleCMSError(s);
		}
	case AT_Reply_CMEError:
	case AT_Reply_Error:
		/* A2D returns Error with empty location */
		return GE_EMPTY;
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	unsigned char	req[20], folderid;
	GSM_Error	error;
	int		location,smsfolder;

	error=ATGEN_GetModel(s);
	if (error!=GE_NONE) return error;

	error=ATGEN_GetSMSLocation(s,&sms->SMS[0], &folderid, &location);
	if (error!=GE_NONE) return error;
	sprintf(req, "AT+CMGR=%i\r",location);

	error=ATGEN_GetSMSMode(s);
	if (error != GE_NONE) return error;

	error=ATGEN_GetManufacturer(s);
	if (error != GE_NONE) return error;

	s->Phone.Data.GetSMSMessage=sms;
	smprintf(s, "Getting SMS\n");
	error=GSM_WaitFor (s, req, strlen(req), 0x00, 5, ID_GetSMSMessage);
	if (error==GE_NONE) {
		smsfolder = sms->SMS[0].Folder;
		ATGEN_SetSMSLocation(s, &sms->SMS[0], folderid, location);
		sms->SMS[0].Folder = smsfolder;
	}
	return error;
}

GSM_Error ATGEN_GetNextSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error 		error;

	if (start) {
		error=s->Phone.Functions->GetSMSStatus(s,&Priv->LastSMSStatus);
		if (error!=GE_NONE) return error;
		Priv->LastSMSRead		= 0;
		sms->SMS[0].Location 		= 0;
	}
	sms->SMS[0].Folder = 0;
	while (true) {
		sms->SMS[0].Location++;
		if (sms->SMS[0].Location<PHONE_MAXSMSINFOLDER) {
			if (Priv->LastSMSRead>=Priv->LastSMSStatus.SIMUsed) {
				if (Priv->LastSMSStatus.PhoneUsed==0) return GE_EMPTY;
				Priv->LastSMSRead	= 0;
				sms->SMS[0].Location 	= PHONE_MAXSMSINFOLDER + 1;
			}
		} else {
			if (Priv->LastSMSRead>=Priv->LastSMSStatus.PhoneUsed) return GE_EMPTY;
		}
		error=s->Phone.Functions->GetSMS(s, sms);
		if (error==GE_NONE) {
			Priv->LastSMSRead++;
			break;
		}
		if (error != GE_EMPTY && error != GE_INVALIDLOCATION) return error;
	}
	return error;
}

GSM_Error ATGEN_ReplyGetSMSStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_SMSMemoryStatus	*SMSStatus = s->Phone.Data.SMSStatus;
	char 			*start;
	int			current = 0;
	unsigned char		buffer[50];

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "SMS status received\n");
		start = strstr(msg.Buffer, "+CPMS: ") + 7;
		if (strstr(msg.Buffer,"ME")!=NULL) {
			SMSStatus->PhoneUsed 	= atoi(start);
			current+=ATGEN_ExtractOneParameter(start+current, buffer);
			current+=ATGEN_ExtractOneParameter(start+current, buffer);
			SMSStatus->PhoneSize	= atoi(buffer);
			smprintf(s, "Used : %i\n",SMSStatus->PhoneUsed);
			smprintf(s, "Size : %i\n",SMSStatus->PhoneSize);
		} else {
			SMSStatus->SIMUsed 	= atoi(start);
			current+=ATGEN_ExtractOneParameter(start+current, buffer);
			current+=ATGEN_ExtractOneParameter(start+current, buffer);
			SMSStatus->SIMSize	= atoi(buffer);
			smprintf(s, "Used : %i\n",SMSStatus->SIMUsed);
			smprintf(s, "Size : %i\n",SMSStatus->SIMSize);
			if (SMSStatus->SIMSize == 0) {
				smprintf(s, "Can't access SIM card\n");
				return GE_SECURITYERROR;
			}
		}
		return GE_NONE;
	case AT_Reply_Error:
		if (strstr(msg.Buffer,"SM")!=NULL) {
			smprintf(s, "Can't access SIM card\n");
			return GE_SECURITYERROR;
		}
		return GE_NOTSUPPORTED;
 	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error 		error,error2;

	status->SIMUnRead	= 0;

	s->Phone.Data.SMSStatus=status;
	smprintf(s, "Getting SIM SMS status\n");
	error=GSM_WaitFor (s, "AT+CPMS=\"SM\",\"SM\"\r", 18, 0x00, 4, ID_GetSMSStatus);
	if (error!=GE_NONE) return error;
	Priv->SMSMemory = GMT_SM;

	status->PhoneUsed	= 0;
	status->PhoneUnRead 	= 0;
	status->PhoneSize	= 0;
	status->TemplatesUsed	= 0;
	
	if (Priv->PhoneSMSMemory == 0) {
		error2 = ATGEN_SetSMSMemory(s,false);
		if (error2 != GE_NONE) return error2;
	}
	if (Priv->PhoneSMSMemory == AT_PHONE_SMS_AVAILABLE) {
		smprintf(s, "Getting phone SMS status\n");
		error2=GSM_WaitFor (s, "AT+CPMS=\"ME\",\"ME\"\r", 18, 0x00, 4, ID_GetSMSStatus);
		if (error2 == GE_NONE) Priv->SMSMemory = GMT_ME;
		if (error2 != GE_NOTSUPPORTED) error=error2;
	}

	return error;
}

GSM_Error ATGEN_ReplyGetIMEI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	CopyLineString(s->Phone.Data.IMEI, msg.Buffer, s->Phone.Data.Priv.ATGEN.Lines, 2);
	smprintf(s, "Received IMEI %s\n",s->Phone.Data.IMEI);
	return GE_NONE;
}

GSM_Error ATGEN_GetIMEI (GSM_StateMachine *s)
{
	if (s->Phone.Data.IMEI[0] != 0) return GE_NONE;
	smprintf(s, "Getting IMEI\n");
	return GSM_WaitFor (s, "AT+CGSN\r", 8, 0x00, 2, ID_GetIMEI);
}

GSM_Error ATGEN_ReplySaveSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	char *start;

	if (s->Protocol.Data.AT.EditMode) {
		s->Protocol.Data.AT.EditMode = false;
		return GE_NONE;
	}

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "SMS saved OK\n");
		start = strstr(msg.Buffer, "+CMGW: ") + 7;
		s->Phone.Data.SaveSMSMessage->Location = atoi(start);
		smprintf(s, "Saved at location %i\n",s->Phone.Data.SaveSMSMessage->Location);
		return GE_NONE;
	case AT_Reply_Error:
		smprintf(s, "Error\n");
		return GE_NOTSUPPORTED;
	case AT_Reply_CMSError:
		/* This error occurs in case that phone couldn't save SMS */
		return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_MakeSMSFrame(GSM_StateMachine *s, GSM_SMSMessage *message, unsigned char *hexreq, int *current, int *length2)
{
	GSM_Error 		error;
	int			i, length;
	unsigned char		req[1000], buffer[1000];
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_SMSC	 	SMSC;

	error=ATGEN_GetSMSMode(s);
	if (error != GE_NONE) return error;

	length 	 = 0;
	*current = 0;
	switch (Priv->SMSMode) {
	case SMS_AT_PDU:
		if (message->PDU == SMS_Deliver) {
 			smprintf(s, "SMS Deliver\n");
			error=PHONE_EncodeSMSFrame(s,message,buffer,PHONE_SMSDeliver,&length,true);
			if (error != GE_NONE) return error;
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
		} else {
			smprintf(s, "SMS Submit\n");
			error=PHONE_EncodeSMSFrame(s,message,buffer,PHONE_SMSSubmit,&length,true);
			if (error != GE_NONE) return error;
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
		if (Priv->Manufacturer == 0) {
			error=ATGEN_GetManufacturer(s);
			if (error != GE_NONE) return error;
		}
		if (Priv->Manufacturer != AT_Nokia) {
			if (message->Coding != GSM_Coding_Default) return GE_NOTSUPPORTED;
		}
		error=PHONE_EncodeSMSFrame(s,message,req,PHONE_SMSDeliver,&i,true);
		if (error != GE_NONE) return error;
		CopyUnicodeString(SMSC.Number,message->SMSC.Number);
		SMSC.Location=1;
		error=ATGEN_SetSMSC(s,&SMSC);
		if (error!=GE_NONE) return error;
		sprintf(buffer, "AT+CSMP=%i,%i,%i,%i\r",
			req[PHONE_SMSDeliver.firstbyte],
			req[PHONE_SMSDeliver.TPVP],
			req[PHONE_SMSDeliver.TPPID],
			req[PHONE_SMSDeliver.TPDCS]);
		error=GSM_WaitFor (s, buffer, strlen(buffer), 0x00, 4, ID_SetSMSParameters);
		if (error==GE_NOTSUPPORTED) {
			/* At least Nokia Communicator 9000i doesn't support <vp> parameter */
			sprintf(buffer, "AT+CSMP=%i,,%i,%i\r",
				req[PHONE_SMSDeliver.firstbyte],
				req[PHONE_SMSDeliver.TPPID],
				req[PHONE_SMSDeliver.TPDCS]);
			error=GSM_WaitFor (s, buffer, strlen(buffer), 0x00, 4, ID_SetSMSParameters);
		}
		if (error!=GE_NONE) return error;
		switch (message->Coding) {
		case GSM_Coding_Default:
			/* If not SMS with UDH, it's as normal text */
			if (message->UDH.Type==UDH_NoUDH) {
				strcpy(hexreq,DecodeUnicodeString(message->Text));
				*length2 = UnicodeLength(message->Text);
				break;
			}
	        case GSM_Coding_Unicode:        
	        case GSM_Coding_8bit:
			error=PHONE_EncodeSMSFrame(s,message,buffer,PHONE_SMSDeliver,current,true);
			if (error != GE_NONE) return error;
			EncodeHexBin (hexreq, buffer+PHONE_SMSDeliver.Text, buffer[PHONE_SMSDeliver.TPUDL]);
			*length2 = buffer[PHONE_SMSDeliver.TPUDL] * 2;
			break;
		}
		break;
	}
	return GE_NONE;
}

GSM_Error ATGEN_SaveSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error 		error;
	int			state,Replies,reply, current, current2;
	unsigned char		buffer[1000], hexreq[1000];
	GSM_Phone_Data		*Phone = &s->Phone.Data;
	unsigned char		*statetxt;

	/* This phone supports only sent/unsent messages on SIM */
	if (strcmp(s->Phone.Data.Model,"ONE TOUCH 500")==0) {
		if (sms->Folder != 2) {
			smprintf(s, "This phone supports only folder = 2!\n");
			return GE_NOTSUPPORTED;
		}
	}
	
	sms->PDU = SMS_Submit;
	switch (sms->Folder) {
	case 1:  sms->PDU = SMS_Deliver;		/* Inbox SIM */
		 error=ATGEN_SetSMSMemory(s, true);
		 break;
	case 2:  error=ATGEN_SetSMSMemory(s, true);	/* Outbox SIM */
	 	 break;
	case 3:  sms->PDU = SMS_Deliver;
		 error=ATGEN_SetSMSMemory(s, false);	/* Inbox phone */
		 break;
	case 4:  error=ATGEN_SetSMSMemory(s, false);	/* Outbox phone */
		 break;
	default: return GE_NOTSUPPORTED;
	}
	if (error!=GE_NONE) return error;

	error=ATGEN_GetModel(s);
	if (error!=GE_NONE) return error;

	error = ATGEN_MakeSMSFrame(s, sms, hexreq, &current, &current2);
	if (error != GE_NONE) return error;

	switch (Phone->Priv.ATGEN.SMSMode) {
	case SMS_AT_PDU:
		if (sms->PDU == SMS_Deliver) {
			state = 0;
			if (sms->State == GSM_Read || sms->State == GSM_Sent) state = 1;
		} else {
			state = 2;
			if (sms->State == GSM_Read || sms->State == GSM_Sent) state = 3;
		}
		/* Siemens M20 */
		if (IsPhoneFeatureAvailable(Phone->ModelInfo, F_M20SMS)) {
			/* No (good and 100% working) support for alphanumeric numbers */
			if (sms->Number[1]!='+' && (sms->Number[1]<'0' || sms->Number[1]>'9')) {
				EncodeUnicode(sms->Number,"123",3);
				error = ATGEN_MakeSMSFrame(s, sms, hexreq, &current, &current2);
				if (error != GE_NONE) return error;
			}
		}
		sprintf(buffer, "AT+CMGW=%i,%i\r",current,state);
		break;
	case SMS_AT_TXT:
		if (sms->PDU == SMS_Deliver) {
			statetxt = "REC UNREAD";
			if (sms->State == GSM_Read || sms->State == GSM_Sent) statetxt = "REC READ";
		} else {
			statetxt = "STO UNSENT";
			if (sms->State == GSM_Read || sms->State == GSM_Sent) statetxt = "STO SENT";
		}
		/* Siemens M20 */
		if (IsPhoneFeatureAvailable(Phone->ModelInfo, F_M20SMS)) {
			/* No (good and 100% working) support for alphanumeric numbers */
			/* FIXME: Try to autodetect support for <stat> (statetxt) parameter although:
			 * Siemens M20 supports +CMGW <stat> specification but on my model it just
			 * reports ERROR (and <stat> is not respected).
			 * Fortunately it will write "+CMGW: <index>\n" before and the message gets written
			 */
			if (sms->Number[1]!='+' && (sms->Number[1]<'0' || sms->Number[1]>'9')) {
		        	sprintf(buffer, "AT+CMGW=\"123\",,\"%s\"\r",statetxt);
			} else {
		        	sprintf(buffer, "AT+CMGW=\"%s\",,\"%s\"\r",DecodeUnicodeString(sms->Number),statetxt);
			}
		} else {
			sprintf(buffer, "AT+CMGW=\"%s\",,\"%s\"\r",DecodeUnicodeString(sms->Number),statetxt);
		}
	}

	Phone->SaveSMSMessage = sms;
	
	for (reply=0;reply<s->ReplyNum;reply++) {
		if (reply!=0) {
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
			    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
			    smprintf(s, "[Retrying %i]\n", reply+1);
			}
		}
		s->Protocol.Data.AT.EditMode 	= true;
		Replies 			= s->ReplyNum;
		s->ReplyNum			= 1;
		smprintf(s,"Waiting for modem prompt\n");
		error=GSM_WaitFor (s, buffer, strlen(buffer), 0x00, 3, ID_SaveSMSMessage);
		s->ReplyNum			 = Replies;
		if (error == GE_NONE) {
			Phone->DispatchError 	= GE_TIMEOUT;
			Phone->RequestID 	= ID_SaveSMSMessage;
			smprintf(s, "Saving SMS\n");
			error = s->Protocol.Functions->WriteMessage(s, hexreq, current2, 0x00);
			if (error!=GE_NONE) return error;
			my_sleep(500);
			/* CTRL+Z ends entering */
			error = s->Protocol.Functions->WriteMessage(s, "\x1A", 1, 0x00);
			if (error!=GE_NONE) return error;
			error = GSM_WaitForOnce(s, NULL, 0x00, 0x00, 4);
			if (error != GE_TIMEOUT) return error;
		} else {
			smprintf(s, "Escaping SMS mode\n");
			error = s->Protocol.Functions->WriteMessage(s, "\x1B\r", 2, 0x00);
			if (error!=GE_NONE) return error;
		}
        }

	return Phone->DispatchError;
}

GSM_Error ATGEN_ReplySendSMS(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	if (s->Protocol.Data.AT.EditMode) {
		s->Protocol.Data.AT.EditMode = false;
		return GE_NONE;
	}

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
 		smprintf(s, "SMS sent OK\n");
 		if (s->User.SendSMSStatus!=NULL) s->User.SendSMSStatus(s->CurrentConfig->Device,0);
		return GE_NONE;
	case AT_Reply_CMSError:
 		smprintf(s, "Error %i\n",Priv->ErrorCode);
 		if (s->User.SendSMSStatus!=NULL) s->User.SendSMSStatus(s->CurrentConfig->Device,Priv->ErrorCode);
 		return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_SendSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error 		error,error2;
	int			current, current2, Replies;
	unsigned char		buffer[1000], hexreq[1000];
	GSM_Phone_Data		*Phone = &s->Phone.Data;

	if (sms->PDU == SMS_Deliver) sms->PDU = SMS_Submit;

	error = ATGEN_MakeSMSFrame(s, sms, hexreq, &current, &current2);
	if (error != GE_NONE) return error;

	switch (Phone->Priv.ATGEN.SMSMode) {
	case SMS_AT_PDU:
		sprintf(buffer, "AT+CMGS=%i\r",current);
		break;
	case SMS_AT_TXT:
		sprintf(buffer, "AT+CMGS=\"%s\"\r",DecodeUnicodeString(sms->Number));
	}

	s->Protocol.Data.AT.EditMode 	= true;
	Replies 			= s->ReplyNum;
	s->ReplyNum			= 1;
	smprintf(s,"Waiting for modem prompt\n");
	error=GSM_WaitFor (s, buffer, strlen(buffer), 0x00, 3, ID_IncomingFrame);
	s->ReplyNum			 = Replies;
	if (error == GE_NONE) {
		smprintf(s, "Sending SMS\n");
		error = s->Protocol.Functions->WriteMessage(s, hexreq, current2, 0x00);
		if (error!=GE_NONE) return error;
		my_sleep(500);
		/* CTRL+Z ends entering */
		error=s->Protocol.Functions->WriteMessage(s, "\x1A", 1, 0x00);
		my_sleep(100);
		return error;
	} else {
		smprintf(s, "Escaping SMS mode\n");
		error2=s->Protocol.Functions->WriteMessage(s, "\x1B\r", 2, 0x00);
		if (error2 != GE_NONE) return error2;
	}
	return error;
}

GSM_Error ATGEN_ReplyGetDateTime_Alarm(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 			current	= 19;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		if (msg.Buffer[current]==0x0d || msg.Buffer[current-1]==0x0d) {
			smprintf(s, "Not set in phone\n");
			return GE_EMPTY;
		} else {
			if (Data->RequestID == ID_GetDateTime) {
				ATGEN_DecodeDateTime(Data->DateTime, msg.Buffer+current);
			} else {
				ATGEN_DecodeDateTime(Data->Alarm, msg.Buffer+current);
			}
			return GE_NONE;
		}
	case AT_Reply_Error:
		return GE_NOTSUPPORTED;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	s->Phone.Data.DateTime=date_time;
	smprintf(s, "Getting date & time\n");
	return GSM_WaitFor (s, "AT+CCLK?\r", 9, 0x00, 4, ID_GetDateTime);
}

GSM_Error ATGEN_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	char req[128];

	sprintf(req, "AT+CCLK=\"%02i/%02i/%02i,%02i:%02i:%02i+00\"\r",
		     date_time->Year-2000,date_time->Month,date_time->Day,
		     date_time->Hour,date_time->Minute,date_time->Second);

	smprintf(s, "Setting date & time\n");
	return GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_SetDateTime);
}

GSM_Error ATGEN_GetAlarm(GSM_StateMachine *s, GSM_DateTime *alarm, int alarm_number)
{
	if (alarm_number!=1) return GE_NOTSUPPORTED;

	s->Phone.Data.Alarm=alarm;
	smprintf(s, "Getting alarm\n");
	return GSM_WaitFor (s, "AT+CALA?\r", 9, 0x00, 4, ID_GetAlarm);
}

GSM_Error ATGEN_ReplyGetSMSC(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_SMSC		*SMSC = s->Phone.Data.SMSC;
	int			current;
	int			len;
	unsigned char 		buffer[100];

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "SMSC info received\n");
		current = 0;
		while (msg.Buffer[current]!='"') current++;
		/* SMSC number */
		/* FIXME: support for all formats */
		current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);

		/* 
		 * Some phones return this as unicode encoded when they are
		 * switched to UCS2 mode, so we try to solve this correctly.
		 */
		len = strlen(buffer + 1) - 1;
		buffer[len + 1] = 0;
		if ((len > 20) && (len % 4 == 0) && (strchr(buffer + 1, '+') == NULL)) {
			/* This is probably unicode encoded number */
			DecodeHexUnicode(SMSC->Number,buffer + 1,len);
		} else  {
			EncodeUnicode(SMSC->Number,buffer + 1,len);
		}
			
		smprintf(s, "Number: \"%s\"\n",DecodeUnicodeString(SMSC->Number));
		/* Format of SMSC number */
		current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
		SMSC->Format = GSMF_Text;
		SMSC->Validity.VPF	= GSM_RelativeFormat;
		SMSC->Validity.Relative	= GSMV_Max_Time;
		SMSC->Name[0]		= 0;
		SMSC->Name[1]		= 0;
		SMSC->DefaultNumber[0]	= 0;
		SMSC->DefaultNumber[1]	= 0;
		return GE_NONE;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	if (smsc->Location==0x00 || smsc->Location!=0x01) return GE_INVALIDLOCATION;
	
	s->Phone.Data.SMSC=smsc;
	smprintf(s, "Getting SMSC\n");
	return GSM_WaitFor (s, "AT+CSCA?\r", 9, 0x00, 4, ID_GetSMSC);
}

GSM_Error ATGEN_ReplyIncomingLAC_CID(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Incoming LAC & CID info\n");
	return GE_NONE;
}

GSM_Error ATGEN_ReplyGetNetworkLAC_CID(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_NetworkInfo	*NetworkInfo = s->Phone.Data.NetworkInfo;

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		break;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	default:
		return GE_UNKNOWNRESPONSE;
	}

	smprintf(s, "Network LAC & CID & state received\n");
#ifdef DEBUG
	switch (msg.Buffer[20]) {
		case '0': smprintf(s, "Not registered into any network. Not searching for network\n"); 	  break;
		case '1': smprintf(s, "Home network\n"); 						  break;
		case '2': smprintf(s, "Not registered into any network. Searching for network\n"); 	  break;
		case '3': smprintf(s, "Registration denied\n"); 					  break;
		case '4': smprintf(s, "Unknown\n"); 							  break;
		case '5': smprintf(s, "Registered in roaming network\n"); 				  break;
		default : smprintf(s, "Unknown\n");
	}
#endif
	switch (msg.Buffer[20]) {
		case '0': NetworkInfo->State = GSM_NoNetwork;		break;
		case '1': NetworkInfo->State = GSM_HomeNetwork; 	break;
		case '2': NetworkInfo->State = GSM_RequestingNetwork; 	break;
		case '3': NetworkInfo->State = GSM_NoNetwork; 		break;
		case '4': NetworkInfo->State = GSM_NoNetwork; 		break;
		case '5': NetworkInfo->State = GSM_RoamingNetwork; 	break;
		default : NetworkInfo->State = GSM_NoNetwork; 		break;
	}
	if (NetworkInfo->State == GSM_HomeNetwork ||
	    NetworkInfo->State == GSM_RoamingNetwork) {
		sprintf(NetworkInfo->CellID,	"%c%c%c%c", msg.Buffer[23], msg.Buffer[24], msg.Buffer[25], msg.Buffer[26]);
		sprintf(NetworkInfo->LAC,	"%c%c%c%c", msg.Buffer[30], msg.Buffer[31], msg.Buffer[32], msg.Buffer[33]);
		smprintf(s, "CellID: %s\n",NetworkInfo->CellID);
		smprintf(s, "LAC   : %s\n",NetworkInfo->LAC);
	}
	return GE_NONE;
}

GSM_Error ATGEN_ReplyGetNetworkCode(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_NetworkInfo		*NetworkInfo = s->Phone.Data.NetworkInfo;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Network code received\n");
		if (Priv->Manufacturer == AT_Falcom) {
			NetworkInfo->NetworkCode[0] = msg.Buffer[22];
			NetworkInfo->NetworkCode[1] = msg.Buffer[23];
			NetworkInfo->NetworkCode[2] = msg.Buffer[24];
			NetworkInfo->NetworkCode[3] = ' ';
			NetworkInfo->NetworkCode[4] = msg.Buffer[25];
			NetworkInfo->NetworkCode[5] = msg.Buffer[26];
		} else {
			NetworkInfo->NetworkCode[0] = msg.Buffer[23];
			NetworkInfo->NetworkCode[1] = msg.Buffer[24];
			NetworkInfo->NetworkCode[2] = msg.Buffer[25];
			NetworkInfo->NetworkCode[3] = ' ';
			NetworkInfo->NetworkCode[4] = msg.Buffer[26];
			NetworkInfo->NetworkCode[5] = msg.Buffer[27];
		}
		NetworkInfo->NetworkCode[6] = 0;
		smprintf(s, "   Network code              : %s\n", NetworkInfo->NetworkCode);
		smprintf(s, "   Network name for Gammu    : %s ",
			DecodeUnicodeString(GSM_GetNetworkName(NetworkInfo->NetworkCode)));
		smprintf(s, "(%s)\n",DecodeUnicodeString(GSM_GetCountryName(NetworkInfo->NetworkCode)));
		return GE_NONE;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo)
{
	GSM_Error error;

	s->Phone.Data.NetworkInfo=netinfo;

	netinfo->NetworkName[0] = 0;
	netinfo->NetworkName[1] = 0;
	netinfo->NetworkCode[0] = 0;

	smprintf(s, "Enable full network info\n");
	error=GSM_WaitFor(s, "AT+CREG=2\r", 10, 0x00, 4, ID_GetNetworkInfo);
	if ((error != GE_NONE) && (s->Phone.Data.Priv.ATGEN.Manufacturer!=AT_Siemens)) return error;

	smprintf(s, "Getting network LAC and CID and state\n");
	error=GSM_WaitFor(s, "AT+CREG?\r", 9, 0x00, 4, ID_GetNetworkInfo);
	if (error != GE_NONE) return error;

	if (netinfo->State == GSM_HomeNetwork || netinfo->State == GSM_RoamingNetwork) {
		smprintf(s, "Setting short network name format\n");
		error=GSM_WaitFor(s, "AT+COPS=3,2\r", 12, 0x00, 4, ID_GetNetworkInfo);

		error=ATGEN_GetManufacturer(s);
		if (error != GE_NONE) return error;

		smprintf(s, "Getting network code\n");
		error=GSM_WaitFor(s, "AT+COPS?\r", 9, 0x00, 4, ID_GetNetworkInfo);
	}
	return error;
}

GSM_Error ATGEN_ReplyGetPBKMemories(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "PBK memories received\n");
	if (strlen(msg.Buffer) > AT_PBK_MAX_MEMORIES) {
		smprintf(s, "ERROR: Too long phonebook memories information received! (Recevided %d, AT_PBK_MAX_MEMORIES is %d\n", strlen(msg.Buffer), AT_PBK_MAX_MEMORIES);
		return GE_MOREMEMORY;
	}
	memcpy(s->Phone.Data.Priv.ATGEN.PBKMemories,msg.Buffer,strlen(msg.Buffer));
	return GE_NONE;
}

GSM_Error ATGEN_SetPBKMemory(GSM_StateMachine *s, GSM_MemoryType MemType)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	char 			req[] = "AT+CPBS=\"XX\"\r";
	GSM_Error		error;

	if (Priv->PBKMemory == MemType) return GE_NONE;

	if (Priv->PBKMemories[0] == 0) {
		error=GSM_WaitFor (s, "AT+CPBS=?\r", 10, 0x00, 3, ID_SetMemoryType);
		if (error != GE_NONE) return error;
	}

	switch (MemType) {
		case GMT_SM:
			req[9] = 'S'; req[10] = 'M';
			break;
		case GMT_ME: 
		        if (strstr(Priv->PBKMemories,"ME")==NULL) return GE_NOTSUPPORTED;
			req[9] = 'M'; req[10] = 'E';
			break;
		case GMT_RC:
		        if (strstr(Priv->PBKMemories,"RC")==NULL) return GE_NOTSUPPORTED;
			req[9] = 'R'; req[10] = 'C';
			break;
		case GMT_MC:
		        if (strstr(Priv->PBKMemories,"MC")==NULL) return GE_NOTSUPPORTED;
			req[9] = 'M'; req[10] = 'C';
			break;
		case GMT_ON:
		        if (strstr(Priv->PBKMemories,"ON")==NULL) return GE_NOTSUPPORTED;
			req[9] = 'O'; req[10] = 'N';
			break;
		case GMT_FD:
		        if (strstr(Priv->PBKMemories,"FD")==NULL) return GE_NOTSUPPORTED;
			req[9] = 'F'; req[10] = 'D';
			break;
		case GMT_DC:
			if (strstr(Priv->PBKMemories,"DC")!=NULL) {
				req[9] = 'D'; req[10] = 'C';
				break;
			}
			if (strstr(Priv->PBKMemories,"LD")!=NULL) {
				req[9] = 'L'; req[10] = 'D';
				break;
			}
			return GE_NOTSUPPORTED;
			break;
		default:
			return GE_NOTSUPPORTED;
	}

	smprintf(s, "Setting memory type\n");
	error=GSM_WaitFor (s, req, 13, 0x00, 3, ID_SetMemoryType);
	if (error == GE_NONE) Priv->PBKMemory = MemType;
	return error;
}

GSM_Error ATGEN_ReplyGetCPBSMemoryStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_MemoryStatus	*MemoryStatus = s->Phone.Data.MemoryStatus;
	char 			*start;

	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Memory status received\n");
  		MemoryStatus->Used = 0;
		MemoryStatus->Free = 0;
		start = strchr(msg.Buffer, ',');
		if (start) {
			start++;
			MemoryStatus->Used = atoi(start);
			start = strchr(start, ',');
			if (start) {
				start++;
				MemoryStatus->Free = atoi(start) - MemoryStatus->Used;
				return GE_NONE;
			} else return GE_UNKNOWN;
		} else return GE_UNKNOWN;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_ReplyGetCPBRMemoryInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
 	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
 	GSM_MemoryStatus	*MemoryStatus = s->Phone.Data.MemoryStatus;
	char 			*pos;
	int			first;
 
 	switch (Priv->ReplyState) {
 	case AT_Reply_OK:
		smprintf(s, "Memory info received\n");
   		MemoryStatus->Used = 0;
 		MemoryStatus->Free = 0;
		/* Parse +CPBR: (first-last),max_number_len,max_name_len */ 
		pos = strchr(msg.Buffer, '(');
		if (pos) {
			pos++;
			first = atoi(pos);
			
			pos = strchr(pos, '-');
			if (pos) {
				pos++;
				/* We assign Size to Used, because MemoryStatus don't have Size member */
				MemoryStatus->Used = atoi(pos);
				if (first != 1) return GE_UNKNOWN; else return GE_NONE;
			} else return GE_UNKNOWN;
		} else return GE_UNKNOWN;
	case AT_Reply_Error:
		return GE_UNKNOWN;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
 	default:
		return GE_UNKNOWNRESPONSE;
	}
}

GSM_Error ATGEN_ReplyGetCPBRMemoryStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_MemoryStatus	*MemoryStatus = s->Phone.Data.MemoryStatus;
	int			line=0;
	char			*str;
	int			cur;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Memory status received\n");
		/* Walk through lines with +CPBR: */
		while (Priv->Lines.numbers[line*2+1]!=0) {
			str = GetLineString(msg.Buffer,Priv->Lines,line+1);
			if (strncmp(str, "+CPBR: ", 7) == 0) {
				MemoryStatus->Used++;
				if (sscanf(str, "+CPBR: %d,", &cur) == 1) {
					if (cur == Priv->NextMemoryEntry || Priv->NextMemoryEntry == 0)
						Priv->NextMemoryEntry = cur + 1;
				}
			}
			line++;
		}
		return GE_NONE;
	case AT_Reply_Error:
		return GE_UNKNOWN;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	default:
		return GE_UNKNOWNRESPONSE;
	}
}

GSM_Error ATGEN_GetMemoryInfo(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	GSM_Error	error;
	char		req[20];
	int		start,end,Size;

	smprintf(s, "Getting memory information\n");
	
	s->Phone.Data.MemoryStatus=Status;

	error = GSM_WaitFor (s, "AT+CPBR=?\r", 10, 0x00, 4, ID_GetMemoryStatus);
	if (error != GE_NONE) return GE_NONE;

	Size		= Status->Used;
	Status->Used 	= 0;
	start 		= 1;
	s->Phone.Data.Priv.ATGEN.NextMemoryEntry = 0;
	while (1) {
		end	= start + 20;
		if (end > Size) end = Size;
		sprintf(req, "AT+CPBR=%i,%i\r", start, end);
		error	= GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_GetMemoryStatus);
		if (error != GE_NONE) return GE_NONE;
		if (end == Size) {
			Status->Free = Size - Status->Used;
			return GE_NONE;
		}
		start	= end + 1;
	}
}

GSM_Error ATGEN_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	GSM_Error error;

	error = ATGEN_SetPBKMemory(s, Status->MemoryType);
	if (error != GE_NONE) return error;

	s->Phone.Data.MemoryStatus=Status;

	/* in some phones doesn't work or doesn't return memory status inside */
	/* Some workaround for buggy mobile, that hangs after "AT+CPBS?" for other
	 * memory than SM.
	 */
	if (!IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_BROKENCPBS) || (Status->MemoryType == GMT_SM)) {
		smprintf(s, "Getting memory status\n");
		error=GSM_WaitFor (s, "AT+CPBS?\r", 9, 0x00, 4, ID_GetMemoryStatus);
		if (error == GE_NONE) return error;
	}

	return ATGEN_GetMemoryInfo(s, Status);
}

GSM_Error ATGEN_SetPBKCharset(GSM_StateMachine *s, bool PreferUnicode)
{
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error		error;

	/* Have we already selected something? */
	if (Priv->PBKCharset!=0) {
		/* If we want unicode charset and we have it already or setting of it
		 * failed, we have nothing to do. */
		if (PreferUnicode  && (Priv->PBKCharset==AT_PBK_UCS2 || Priv->UCS2CharsetFailed)) return GE_NONE;

		/* If we don't need unicode charset and we have some (or have unicode
		 * charset when other failed), we have nothing to do. */
		if (!PreferUnicode && (Priv->PBKCharset!=AT_PBK_UCS2 || Priv->NonUCS2CharsetFailed)) return GE_NONE;
	}

	error=ATGEN_GetManufacturer(s);
	if (error != GE_NONE) return error;

	if (PreferUnicode && !Priv->UCS2CharsetFailed) {
		smprintf(s, "Setting charset to UCS2\n");
		error=GSM_WaitFor (s, "AT+CSCS=\"UCS2\"\r", 15, 0x00, 3, ID_SetMemoryCharset);
		if (error == GE_NONE) {
			Priv->PBKCharset = AT_PBK_UCS2;
			return GE_NONE;
		} else {
			Priv->UCS2CharsetFailed = true;
		}
	}
	
	smprintf(s, "Setting charset to HEX\n");
	error=GSM_WaitFor (s, "AT+CSCS=\"HEX\"\r", 14, 0x00, 3, ID_SetMemoryCharset);
	/* Falcom replies OK for HEX mode and send everything
	 * in normal format */
	if (error == GE_NONE && Priv->Manufacturer != AT_Falcom) {
		Priv->PBKCharset = AT_PBK_HEX;
		return GE_NONE;
	}

	smprintf(s, "Setting charset to GSM\n");
	error=GSM_WaitFor (s, "AT+CSCS=\"GSM\"\r", 14, 0x00, 3, ID_SetMemoryCharset);
	if (error == GE_NONE) {
		Priv->PBKCharset = AT_PBK_GSM;
		return GE_NONE;
	}

	if (!Priv->UCS2CharsetFailed) {
		Priv->NonUCS2CharsetFailed = true;
		smprintf(s, "Setting charset to UCS2\n");
		error=GSM_WaitFor (s, "AT+CSCS=\"UCS2\"\r", 15, 0x00, 3, ID_SetMemoryCharset);
		if (error == GE_NONE) {
			Priv->PBKCharset = AT_PBK_UCS2;
			return GE_NONE;
		} else {
			Priv->UCS2CharsetFailed = true;
		}
	}

	return error;	
}

GSM_Error ATGEN_ReplyGetMemory(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
 	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
 	GSM_PhonebookEntry	*Memory = s->Phone.Data.Memory;
	int			current;
	unsigned char		buffer[500],buffer2[500];

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
 		smprintf(s, "Phonebook entry received\n");
 		Memory->EntriesNum = 0;
		if (Priv->Lines.numbers[4]==0) return GE_EMPTY;
		current = 0;
		while (msg.Buffer[current]!='"') current++;
		/* Number */
		current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
 		smprintf(s, "Number: %s\n",buffer);
 		Memory->EntriesNum++;
 		Memory->Entries[0].EntryType=PBK_Number_General;
 		EncodeUnicode(Memory->Entries[0].Text,buffer+1,strlen(buffer)-2);
		/* Number format */
		current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
 		smprintf(s, "Number format: %s\n",buffer);
		/* Name */
		current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
 		smprintf(s, "Name text: %s\n",buffer);
 		Memory->EntriesNum++;
 		Memory->Entries[1].EntryType=PBK_Name;
		switch (Priv->PBKCharset) {
		case AT_PBK_HEX:
			DecodeHexBin(buffer2,buffer+1,strlen(buffer)-2);
 			DecodeDefault(Memory->Entries[1].Text,buffer2,strlen(buffer2),false,NULL);
			Memory->PreferUnicode = false;
			break;
		case AT_PBK_GSM:
 			DecodeDefault(Memory->Entries[1].Text,buffer+1,strlen(buffer)-2,false,NULL);
			Memory->PreferUnicode = false;
			break;			
		case AT_PBK_UCS2:
			DecodeHexUnicode(Memory->Entries[1].Text,buffer+1,strlen(buffer+1) - 1);
			/* If entry is longer than maximum that can be stored
			 * in UCS2, we won't want unicode. */
			/* FIXME: The number should be probably read by AT+CPBR=? */
			Memory->PreferUnicode = (strlen(buffer+1) - 1 < (4 * (14 /* here should be real max entry size*/ / 2)));
			break;			
		}
		return GE_NONE;
	case AT_Reply_Error:
 		smprintf(s, "Error - too high location ?\n");
		return GE_INVALIDLOCATION;
	case AT_Reply_CMSError:
 	        return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetMemory (GSM_StateMachine *s, GSM_PhonebookEntry *entry)
{
	GSM_Error 		error;
	unsigned char		req[20];
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;

	if (entry->Location==0x00) return GE_INVALIDLOCATION;

	if (entry->MemoryType == GMT_ME) {
		if (Priv->PBKSBNR == 0) {
			sprintf(req, "AT^SBNR=?\r");
			smprintf(s, "Checking availablity of SBNR\n");
			error=GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_GetMemory);
			switch (error) {
			case GE_NONE:
				Priv->PBKSBNR = AT_SBNR_AVAILABLE;
				break;
			case GE_UNKNOWN:
			case GE_NOTSUPPORTED:
				Priv->PBKSBNR = AT_SBNR_NOTAVAILABLE;
				break;
			default:
				return error;
			}
		}
		if (Priv->PBKSBNR == AT_SBNR_AVAILABLE) {
			sprintf(req, "AT^SBNR=vcf,%i\r",entry->Location-1);
			s->Phone.Data.Memory=entry;
			smprintf(s, "Getting phonebook entry\n");
			return GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_GetMemory);
		}
	}

	error=ATGEN_GetManufacturer(s);
	if (error != GE_NONE) return error;

	error=ATGEN_SetPBKMemory(s, entry->MemoryType);
	if (error != GE_NONE) return error;

	error=ATGEN_SetPBKCharset(s, true); /* For reading we prefer unicode */
	if (error != GE_NONE) return error;

	sprintf(req, "AT+CPBR=%i\r",entry->Location);

	s->Phone.Data.Memory=entry;
	smprintf(s, "Getting phonebook entry\n");
	return GSM_WaitFor (s, req, strlen(req), 0x00, 4, ID_GetMemory);
}

GSM_Error ATGEN_ReplyDialVoice(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Dial voice OK\n");
		return GE_NONE;
	case AT_Reply_Error:
		smprintf(s, "Dial voice error\n");
		return GE_UNKNOWN;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_DialVoice(GSM_StateMachine *s, char *number, GSM_CallShowNumber ShowNumber)
{
	char req[39] = "ATDT";  

	if (ShowNumber != GN_CALL_Default) return GE_NOTSUPPORTED;
	if (strlen(number) > 32) return (GE_UNKNOWN);

	strcat(req, number);
	strcat(req, ";\r");

	smprintf(s, "Making voice call\n");
	return GSM_WaitFor (s, req, 4+2+strlen(number), 0x00, 5, ID_DialVoice);
}

GSM_Error ATGEN_ReplyEnterSecurityCode(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Security code was OK\n");
		return GE_NONE;
	case AT_Reply_Error:
		smprintf(s, "Incorrect security code\n");
		return GE_SECURITYERROR;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_EnterSecurityCode(GSM_StateMachine *s, GSM_SecurityCode Code)
{
	unsigned char req[50];

	switch (Code.Type) {
	case GSCT_Pin   : sprintf(req, "AT+CPIN=\"%s\"\r" , Code.Code); break;
	case GSCT_Pin2  : sprintf(req, "AT+CPIN2=\"%s\"\r", Code.Code); break;
	default		: return GE_NOTIMPLEMENTED;
	}

	smprintf(s, "Entering security code\n");
	return GSM_WaitFor (s, req, strlen(req), 0x00, 6, ID_EnterSecurityCode);
}

GSM_Error ATGEN_ReplyGetSecurityStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_SecurityCodeType *Status = s->Phone.Data.SecurityStatus;

	smprintf(s, "Security status received - ");
	if (strstr(msg.Buffer,"READY")) {
		*Status = GSCT_None;
		smprintf(s, "nothing to enter\n");
		return GE_NONE;
	}
	if (strstr(msg.Buffer,"PH_SIM PIN")) {
		smprintf(s, "no SIM inside or other error\n");
		return GE_UNKNOWN;
	}
	if (strstr(msg.Buffer,"SIM PIN")) {
		*Status = GSCT_Pin;
		smprintf(s, "waiting for PIN\n");
		return GE_NONE;
	}
	if (strstr(msg.Buffer,"SIM PUK")) {
		*Status = GSCT_Puk;
		smprintf(s, "waiting for PUK\n");
		return GE_NONE;
	}
	if (strstr(msg.Buffer,"SIM PIN2")) {
		*Status = GSCT_Pin2;
		smprintf(s, "waiting for PIN2\n");
		return GE_NONE;
	}
	if (strstr(msg.Buffer,"SIM PUK2")) {
		*Status = GSCT_Puk2;
		smprintf(s, "waiting for PUK2\n");
		return GE_NONE;
	}
	smprintf(s, "unknown\n");
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetSecurityStatus(GSM_StateMachine *s, GSM_SecurityCodeType *Status)
{
	s->Phone.Data.SecurityStatus=Status;

	smprintf(s, "Getting security code status\n");
	/* Please note, that A2D doesn't return OK on the end.
 	 * Because of it ReplyGetSecurityStatus is called after receiving line
	 * with +CPIN:
	 */
	return GSM_WaitFor (s, "AT+CPIN?\r", 9, 0x00, 4, ID_GetSecurityStatus);
}

GSM_Error ATGEN_AnswerCall(GSM_StateMachine *s, int ID, bool all)
{
	if (all) {
		smprintf(s, "Answering all calls\n");
		return GSM_WaitFor (s, "ATA\r", 4, 0x00, 4, ID_AnswerCall);
	}
	return GE_NOTSUPPORTED;
}

GSM_Error ATGEN_ReplyCancelCall(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Call call;

	switch(s->Phone.Data.Priv.ATGEN.ReplyState) {
        case AT_Reply_OK:
     	    smprintf(s, "Calls canceled\n");
            call.CallIDAvailable = false;
            call.Status 	 = GN_CALL_CallLocalEnd;
            if (s->User.IncomingCall) s->User.IncomingCall(s->CurrentConfig->Device, call);
 
            return GE_NONE;
    	case AT_Reply_CMSError:
            return ATGEN_HandleCMSError(s);
        default:
    	    return GE_UNKNOWN;
	}
}

GSM_Error ATGEN_CancelCall(GSM_StateMachine *s, int ID, bool all)
{
	GSM_Error error;
	
	if (all) {
		smprintf(s, "Dropping all calls\n");
		error = GSM_WaitFor (s, "ATH\r", 4, 0x00, 4, ID_CancelCall);
		if (error == GE_UNKNOWN) {
		    return GSM_WaitFor (s, "AT+CHUP\r", 8, 0x00, 4, ID_CancelCall);
		}
		return error;
	}
	return GE_NOTSUPPORTED;
}

GSM_Error ATGEN_ReplyReset(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Reset done\n");
	return GE_NONE;
}

GSM_Error ATGEN_Reset(GSM_StateMachine *s, bool hard)
{
	GSM_Error error;

	if (!hard) return GE_NOTSUPPORTED;

	smprintf(s, "Resetting device\n");
	/* Siemens 35 */
	error=GSM_WaitFor (s, "AT+CFUN=1,1\r", 12, 0x00, 8, ID_Reset);
	if (error != GE_NONE) {
		/* Siemens M20 */
		error=GSM_WaitFor (s, "AT^SRESET\r", 10, 0x00, 8, ID_Reset);
	}
	return error;
}

GSM_Error ATGEN_ReplyResetPhoneSettings(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Reset done\n");
	return GE_NONE;
}

GSM_Error ATGEN_ResetPhoneSettings(GSM_StateMachine *s, GSM_ResetSettingsType Type)
{
	smprintf(s, "Resetting settings to default\n");
	return GSM_WaitFor (s, "AT&F\r", 5, 0x00, 4, ID_ResetPhoneSettings);
}

GSM_Error ATGEN_SetAutoNetworkLogin(GSM_StateMachine *s)
{
	smprintf(s, "Enabling automatic network login\n");
	return GSM_WaitFor (s, "AT+COPS=0\r", 10, 0x00, 4, ID_SetAutoNetworkLogin);
}

GSM_Error ATGEN_SendDTMF(GSM_StateMachine *s, char *sequence)
{
	unsigned char 	req[80] = "AT+VTS=";
	int 		n;

	for (n = 0; n < 32; n++) {
		if (sequence[n] == '\0') break;
		if (n != 0) req[6 + 2 * n] = ',';
		req[7 + 2 * n] = sequence[n];
	}

	strcat(req, ";\r");

	smprintf(s, "Sending DTMF\n");
	return GSM_WaitFor (s, req, 7+2+2*strlen(sequence), 0x00, 4, ID_SendDTMF);
}

GSM_Error ATGEN_ReplyDeleteSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "SMS deleted OK\n");
		return GE_NONE;
	case AT_Reply_Error:
		smprintf(s, "Invalid location\n");
		return GE_INVALIDLOCATION;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_DeleteSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	unsigned char	req[20], folderid;
	GSM_Error	error;
	int		location;

	error=ATGEN_GetSMSLocation(s,sms, &folderid, &location);
	if (error!=GE_NONE) return error;
	sprintf(req, "AT+CMGD=%i\r",location);

	smprintf(s, "Deleting SMS\n");
	return GSM_WaitFor (s, req, strlen(req), 0x00, 5, ID_DeleteSMSMessage);
}

GSM_Error ATGEN_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error 		error;

	PHONE_GetSMSFolders(s,folders);
	switch (Priv->PhoneSMSMemory) {
		case AT_PHONE_SMS_NOTAVAILABLE:
			return GE_NONE;
		case 0:
			error=ATGEN_SetSMSMemory(s, false);
			/* Not supported */
			if (error == GE_NOTSUPPORTED) return GE_NONE;
			if (error != GE_NONE) return error;
		default:
			break;
	}
	if (Priv->PhoneSMSMemory == AT_PHONE_SMS_AVAILABLE) {
		folders->Number=4;
		CopyUnicodeString(folders->Folder[2].Name,folders->Folder[0].Name);
		CopyUnicodeString(folders->Folder[3].Name,folders->Folder[1].Name);
	}
	return GE_NONE;
}

GSM_Error ATGEN_ReplySetMemory(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "Phonebook entry written OK\n");
		return GE_NONE;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_SetMemory(GSM_StateMachine *s, GSM_PhonebookEntry *entry)
{
#define REQUEST_SIZE	200
	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
	int			Group, Name, Number,NumberType=0, len;
	GSM_Error 		error;
	unsigned char		req[REQUEST_SIZE + 1],name[200],number[200];
	int			length;
	GSM_MemoryStatus	Status;

	error=ATGEN_SetPBKMemory(s, entry->MemoryType);
	if (error != GE_NONE) return error;

	error=ATGEN_SetPBKCharset(s, entry->PreferUnicode);
	if (error != GE_NONE) return error;

	if (entry->Location == 0) {
		error = ATGEN_GetMemoryInfo(s, &Status);
		if (error != GE_NONE) return error;
		if (Priv->NextMemoryEntry == 0) return GE_FULL;
		entry->Location = Priv->NextMemoryEntry;
	}


	GSM_PhonebookFindDefaultNameNumberGroup(entry, &Name, &Number, &Group);

	name[0] = 0;
	if (Name != -1) {
		switch (Priv->PBKCharset) {
		case AT_PBK_HEX:
			EncodeHexBin(name, DecodeUnicodeString(entry->Entries[Name].Text), UnicodeLength(entry->Entries[Name].Text));
			len = strlen(name);
			break;
		case AT_PBK_GSM:
			smprintf(s, "str: %s\n", DecodeUnicodeString(entry->Entries[Name].Text));
			len = UnicodeLength(entry->Entries[Name].Text);
			EncodeDefault(name, entry->Entries[Name].Text, &len, true, NULL);
			break;
		case AT_PBK_UCS2:
			EncodeHexUnicode(name, entry->Entries[Name].Text, UnicodeLength(entry->Entries[Name].Text));
			len = strlen(name);
			break;
		}
	}

	number[0] = 0;
	if (Number != -1) {
		GSM_PackSemiOctetNumber(entry->Entries[Number].Text, number, false);
		NumberType = number[0];
		sprintf(number,"%s",DecodeUnicodeString(entry->Entries[Number].Text));
	}

	if (Number == -1 && Name == -1) {
		sprintf(req, "AT+CPBW=%d\r",entry->Location);
		length = strlen(req);
	} else {
		/* We can't use here:
		 * sprintf(req, "AT+CPBW=%d, \"%s\", %i, \"%s\"\r",
		 *         entry->Location, number, NumberType, name);
		 * because name can contain 0 when using GSM alphabet.
		 */
		sprintf(req, "AT+CPBW=%d, \"%s\", %i, \"",
		        entry->Location, number, NumberType);
		length = strlen(req);
		if (length + len > REQUEST_SIZE - 2) {
			smprintf(s, "WARNING: Text truncated to fit in buffer!\n");
		}
		memcpy(req + length, name, len);
		length += len;
		memcpy(req + length, "\"\r", 2);
		length += 2;
	}

	smprintf(s, "Writing phonebook entry\n");
	return GSM_WaitFor (s, req, length, 0x00, 4, ID_SetMemory);
#undef REQUEST_SIZE
}

/* Use ATGEN_ExtractOneParameter ?? */
void Extract_CLIP_number(char *dest, char *buf)
{
	char 	*start, *stop;
	int 	i = 0;

	stop = strstr(buf, ",");
        if (stop != NULL) {
        	start = strstr(buf, ":");
	        if (start != NULL) { 
			for (start = start + 2; start + i < stop;  i++)
			dest[i] = start[i];
       		}
	}
	dest[i] = 0; /* end the number */
	
	return;
}

GSM_Error ATGEN_ReplyIncomingCallInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	char 			num[128];
	GSM_Call 		call;

	call.CallIDAvailable 	= false;
	num[0] 			= 0;
	smprintf(s, "Incoming call info\n");
	if (strstr(msg.Buffer, "RING")) {
		call.Status = GN_CALL_IncomingCall;
		Extract_CLIP_number(num, msg.Buffer);
	} else if (strstr(msg.Buffer, "NO CARRIER")) {
		call.Status = GN_CALL_CallEnd;
	} else if (strstr(msg.Buffer, "COLP:")) {
		call.Status = GN_CALL_CallStart;
		Extract_CLIP_number(num, msg.Buffer);
	} else {
		smprintf(s, "CLIP: error\n");
		return GE_NONE;
	}
	EncodeUnicode(call.PhoneNumber, num, strlen(num));

	if (s->Phone.Data.EnableIncomingCall && s->User.IncomingCall!=NULL) s->User.IncomingCall(s->CurrentConfig->Device, call);

	return GE_NONE;
}

GSM_Error ATGEN_IncomingSMS(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
/*	GSM_SMSMessage sms; */

	smprintf(s, "Incoming SMS\n");
/*	if (User->IncomingSMS) User->IncomingSMS(Data->Device, sms); */

	return GE_NONE;
}

GSM_Error ATGEN_IncomingGPRS(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "GPRS change\n");
	/* "+CGREG: 1,1" */

	return GE_NONE;
}

GSM_Error ATGEN_IncomingBattery(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 	level = 0;
	char 	*p;
	
	/* "_OBS: 92,1" */
	p = strstr(msg.Buffer, "_OBS:");
	if (p) level = atoi(p + 5);
	smprintf(s, "Battery level changed to %d\n", level);

/*	if (User->BattChange) User->BattChange(level); */

	return GE_NONE;
}

GSM_Error ATGEN_IncomingNetworkLevel(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 	level = 0;
	char 	*p;

	/* "_OSIGQ: 12,0" */
	p = strstr(msg.Buffer, "_OSIGQ: ");
	if (p) level = atoi(p + 7);
	smprintf(s, "Network level changed to %d\n", level);

/*	if (User->SignalChange) User->SignalChange(level); */

	return GE_NONE;
}

GSM_Error ATGEN_ReplyGetSIMIMSI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Phone_Data		*Data = &s->Phone.Data;
	char 			*c;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		CopyLineString(Data->PhoneString, msg.Buffer, Priv->Lines, 2);

        	/* Read just IMSI also on phones that prepend it by "<IMSI>:" (Alcatel BE5) */
		c = strstr(Data->PhoneString, "<IMSI>:");
		if (c != NULL) {
			c += 7;
			memmove(Data->PhoneString, c, strlen(c) + 1);
		}

		smprintf(s, "Received IMSI %s\n",Data->PhoneString);
		return GE_NONE;
	case AT_Reply_Error:
	case AT_Reply_CMEError:
		smprintf(s, "No access to SIM card or not supported by device\n");
		return GE_SECURITYERROR;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	default:
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetSIMIMSI(GSM_StateMachine *s, char *IMSI)
{
	s->Phone.Data.PhoneString = IMSI;
	smprintf(s, "Getting SIM IMSI\n");
	return GSM_WaitFor (s, "AT+CIMI\r", 8, 0x00, 4, ID_GetSIMIMSI);
}

GSM_Error ATGEN_GetDisplayStatus(GSM_StateMachine *s, GSM_DisplayFeatures *features)
{
	return GE_NOTSUPPORTED;

	s->Phone.Data.DisplayFeatures = features;
	smprintf(s, "Getting display status\n");
	return GSM_WaitFor (s, "AT+CIND?\r",9, 0x00, 4, ID_GetDisplayStatus);
}

GSM_Error ATGEN_PressKey(GSM_StateMachine *s, GSM_KeyCode Key, bool Press)
{
	return GE_NOTSUPPORTED;

	s->Phone.Data.PressKey = true;
	smprintf(s, "Pressing key\n");
	return GSM_WaitFor (s, "AT+CKPD=\"1\"\r", 12, 0x00, 4, ID_PressKey);
}

GSM_Error ATGEN_IncomingSMSCInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return GE_NONE;
}

GSM_Error ATGEN_ReplyGetBatteryCharge(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
    GSM_Phone_Data		*Data = &s->Phone.Data;
    int 			i;
    
    Data->BatteryCharge->BatteryPercent = -1;
    Data->BatteryCharge->ChargeState 	= 0;
        
    switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
        case AT_Reply_OK:
            smprintf(s, "Battery level received\n");
            Data->BatteryCharge->BatteryPercent = atoi(msg.Buffer+17);
            i = atoi(msg.Buffer+14);
            if (i >= 0 && i <= 3) { 
                Data->BatteryCharge->ChargeState = i + 1;
            }
            return GE_NONE;
        case AT_Reply_Error:
            smprintf(s, "Can't get battery level\n");
            return GE_UNKNOWN;
        case AT_Reply_CMSError:
            smprintf(s, "Can't get battery level\n");
            return ATGEN_HandleCMSError(s);
        default:
            break;
    }
    return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	s->Phone.Data.BatteryCharge = bat;
	smprintf(s, "Getting battery charge\n");
	return GSM_WaitFor (s, "AT+CBC\r", 7, 0x00, 4, ID_GetBatteryCharge);
}

GSM_Error ATGEN_ReplyGetSignalQuality(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_SignalQuality	*Signal = s->Phone.Data.SignalQuality;
	int 			i;
	char 			*pos;

	Signal->SignalStrength 	= -1;
	Signal->SignalPercent 	= -1;
	Signal->BitErrorRate 	= -1;
    
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
        case AT_Reply_OK:
            smprintf(s, "Signal quality info received\n");
            i = atoi(msg.Buffer+15);
            if (i != 99) {
                /* from GSM 07.07 section 8.5 */
                Signal->SignalStrength = 2 * i - 113;

                /* FIXME: this is wild guess and probably will be phone dependant */
                Signal->SignalPercent = 15 * i;
                if (Signal->SignalPercent > 100) Signal->SignalPercent = 100;
            }
            pos = strchr(msg.Buffer + 15, ',');
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
            return GE_NONE;
        case AT_Reply_CMSError:
            return ATGEN_HandleCMSError(s);
        default:
            break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetSignalQuality(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	s->Phone.Data.SignalQuality = sig;
	smprintf(s, "Getting signal quality info\n");
	return GSM_WaitFor (s, "AT+CSQ\r", 7, 0x00, 4, ID_GetSignalQuality);
}

/* When use AT+CPIN?, A2D returns it without OK and because of it Gammu
   parses answer without it.
   MC35 and other return OK after answer for AT+CPIN?. Here we handle it.
   Any better idea ?
 */
GSM_Error ATGEN_ReplyOK(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return GE_NONE;
}

GSM_Reply_Function ATGENReplyFunctions[] = {
{ATGEN_ReplyOK,			"OK"			,0x00,0x00,ID_IncomingFrame	 },

{ATGEN_ReplyGetBatteryCharge,	"AT+CBC"		,0x00,0x00,ID_GetBatteryCharge	 },
{ATGEN_ReplyGetSignalQuality,	"AT+CSQ"		,0x00,0x00,ID_GetSignalQuality	 },
{ATGEN_ReplyGetModel,		"AT+CGMM"		,0x00,0x00,ID_GetModel           },
{ATGEN_GenericReply,		"AT+CMGF"		,0x00,0x00,ID_GetSMSMode	 },
{ATGEN_GenericReply,		"AT+CSDH"		,0x00,0x00,ID_GetSMSMode	 },
{ATGEN_ReplyGetSMSMessage,	"AT+CMGR"		,0x00,0x00,ID_GetSMSMessage	 },
{ATGEN_ReplyGetSMSStatus,	"AT+CPMS"		,0x00,0x00,ID_GetSMSStatus	 },
{ATGEN_ReplyGetSMSMemories,	"AT+CPMS?"		,0x00,0x00,ID_GetSMSMemories	 },
{ATGEN_GenericReply,		"AT+CPMS"		,0x00,0x00,ID_SetMemoryType	 },
{ATGEN_ReplyGetManufacturer,	"AT+CGMI"		,0x00,0x00,ID_GetManufacturer	 },
{ATGEN_ReplyGetFirmwareCGMR,	"AT+CGMR"		,0x00,0x00,ID_GetFirmware	 },
{ATGEN_ReplyGetFirmwareATI,	"ATI"			,0x00,0x00,ID_GetFirmware	 },
{ATGEN_ReplyGetIMEI,		"AT+CGSN"		,0x00,0x00,ID_GetIMEI	 	 },
{ATGEN_ReplySaveSMSMessage,	"AT+CMGW"		,0x00,0x00,ID_SaveSMSMessage	 },
{ATGEN_GenericReply,		"AT+CSMP"		,0x00,0x00,ID_SetSMSParameters	 },
{ATGEN_ReplySendSMS,		"AT+CMGS"		,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_GenericReply,		"AT+CSCA"		,0x00,0x00,ID_SetSMSC		 },
{ATGEN_ReplyGetDateTime_Alarm,	"AT+CCLK?"		,0x00,0x00,ID_GetDateTime	 },
{ATGEN_ReplyGetDateTime_Alarm,	"AT+CALA?"		,0x00,0x00,ID_GetAlarm		 },
{ATGEN_GenericReply,		"AT+CCLK="		,0x00,0x00,ID_SetDateTime	 },
{ATGEN_ReplyGetSMSC,		"AT+CSCA?"		,0x00,0x00,ID_GetSMSC		 },
{ATGEN_GenericReply,		"AT+CREG=2"		,0x00,0x00,ID_GetNetworkInfo	 },
{ATGEN_ReplyIncomingLAC_CID,	"\x0D\x0A+CREG?"	,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_ReplyGetNetworkLAC_CID,	"AT+CREG?"		,0x00,0x00,ID_GetNetworkInfo	 },
{ATGEN_GenericReply,		"AT+COPS="		,0x00,0x00,ID_GetNetworkInfo	 },
{ATGEN_GenericReply,		"AT+COPS="		,0x00,0x00,ID_SetAutoNetworkLogin},
{ATGEN_ReplyGetNetworkCode,	"AT+COPS"		,0x00,0x00,ID_GetNetworkInfo	 },
{ATGEN_ReplyGetPBKMemories,	"AT+CPBS=?"		,0x00,0x00,ID_SetMemoryType	 },
{ATGEN_GenericReply,		"AT+CPBS="		,0x00,0x00,ID_SetMemoryType	 },
{ATGEN_ReplyGetCPBSMemoryStatus,"AT+CPBS?"		,0x00,0x00,ID_GetMemoryStatus	 },
{ATGEN_ReplyGetCPBRMemoryInfo,	"AT+CPBR=?"		,0x00,0x00,ID_GetMemoryStatus	 },
{ATGEN_ReplyGetCPBRMemoryStatus,"AT+CPBR="		,0x00,0x00,ID_GetMemoryStatus	 },
{ATGEN_GenericReply,		"AT+CSCS="		,0x00,0x00,ID_SetMemoryCharset	 },
{ATGEN_GenericReply,		"ATE1"			,0x00,0x00,ID_SetSMSParameters	 },
{ATGEN_GenericReply,		"\x1b\x0D"		,0x00,0x00,ID_SetSMSParameters	 },
{ATGEN_ReplyGetMemory,		"AT+CPBR="		,0x00,0x00,ID_GetMemory		 },
{ATGEN_GenericReply,		"AT^SBNR=?"		,0x00,0x00,ID_GetMemory		 },
{ATGEN_SL45ReplyGetMemory,	"AT^SBNR"		,0x00,0x00,ID_GetMemory		 },
{ATGEN_CMS35ReplyGetBitmap,	"AT^SBNR=\"bmp\""	,0x00,0x00,ID_GetBitmap	 	 },
{ATGEN_CMS35ReplySetBitmap,	"AT^SBNW=\"bmp\""	,0x00,0x00,ID_SetBitmap	 	 },
{ATGEN_CMS35ReplyGetRingtone,	"AT^SBNR=\"mid\""	,0x00,0x00,ID_GetRingtone	 },
{ATGEN_CMS35ReplySetRingtone,	"AT^SBNW=\"mid\""	,0x00,0x00,ID_SetRingtone	 },
{ATGEN_CMS35ReplyGetNextCal,	"AT^SBNR=\"vcs\""	,0x00,0x00,ID_GetCalendarNote	 },
{ATGEN_CMS35ReplySetCalendar,	"AT^SBNW=\"vcs\""	,0x00,0x00,ID_SetCalendarNote	 },
{ATGEN_CMS35ReplyDeleteCalendar,"AT^SBNW=\"vcs\""	,0x00,0x00,ID_DeleteCalendarNote },
{ATGEN_ReplyEnterSecurityCode,	"AT+CPIN="		,0x00,0x00,ID_EnterSecurityCode	 },
{ATGEN_ReplyEnterSecurityCode,	"AT+CPIN2="		,0x00,0x00,ID_EnterSecurityCode	 },
{ATGEN_ReplyGetSecurityStatus,	"AT+CPIN?"		,0x00,0x00,ID_GetSecurityStatus	 },
{ATGEN_ReplyDialVoice,		"ATDT"			,0x00,0x00,ID_DialVoice		 },
{ATGEN_ReplyCancelCall,		"AT+CHUP"		,0x00,0x00,ID_CancelCall	 },
{ATGEN_ReplyCancelCall,		"ATH"			,0x00,0x00,ID_CancelCall	 },
{ATGEN_ReplyReset,		"AT^SRESET"		,0x00,0x00,ID_Reset		 },
{ATGEN_ReplyReset,		"AT+CFUN=1,1"		,0x00,0x00,ID_Reset		 },
{ATGEN_ReplyResetPhoneSettings, "AT&F"			,0x00,0x00,ID_ResetPhoneSettings },
{ATGEN_GenericReply, 		"AT+VTS"		,0x00,0x00,ID_SendDTMF		 },
{ATGEN_ReplyDeleteSMSMessage,	"AT+CMGD"		,0x00,0x00,ID_DeleteSMSMessage	 },
{ATGEN_ReplySetMemory,		"AT+CPBW"		,0x00,0x00,ID_SetMemory		 },
{ATGEN_GenericReply,            "AT+CLIP=1"      	,0x00,0x00,ID_IncomingFrame      },
{ATGEN_ReplyGetSIMIMSI,		"AT+CIMI" 	 	,0x00,0x00,ID_GetSIMIMSI	 },
{ATGEN_GenericReply,		"AT\r"			,0x00,0x00,ID_IncomingFrame	 },

#ifdef GSM_ENABLE_ALCATEL
/*  Why do I give Alcatel specific things here? It's simple, Alcatel needs
 *  some AT commands to start it's binary mode, so this needs to be in AT
 *  related stuff. 
 *
 *  XXX: AT+IFC could later move outside this ifdef, because it is not Alcatel
 *  specific and it's part of ETSI specifications 
 */
{ATGEN_GenericReply,		"AT+IFC" 	 	,0x00,0x00,   ID_SetFlowControl  },
{ALCATEL_ProtocolVersionReply,	"AT+CPROT=?" 	 	,0x00,0x00,   ID_AlcatelProtocol },
{ATGEN_GenericReply,		"AT+CPROT" 	 	,0x00,0x00,   ID_AlcatelConnect	 },
#endif

{ATGEN_GenericReply,		"ATE1" 	 		,0x00,0x00,   ID_EnableEcho	 },

{ATGEN_ReplyIncomingCallInfo,	"+CLIP"			,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_ReplyIncomingCallInfo,	"\x0D\x0ARING"		,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_ReplyIncomingCallInfo,	"\x0D\x0A+COLP:"    	,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_ReplyIncomingCallInfo,	"\x0D\x0ANO CARRIER"	,0x00,0x00,ID_IncomingFrame	 },

{ATGEN_IncomingNetworkLevel,	"\x0D\x0A_OSIGQ:"	,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_IncomingNetworkLevel,	"_OSIGQ:"	 	,0x00,0x00,ID_IncomingFrame	 },

{ATGEN_IncomingGPRS,		"\x0D\x0A+CGREG:"	,0x00,0x00,ID_IncomingFrame      },
{ATGEN_IncomingGPRS,		"+CGREG:"	 	,0x00,0x00,ID_IncomingFrame      },

{ATGEN_IncomingBattery,		"\x0D\x0A_OBS:"  	,0x00,0x00,ID_IncomingFrame      },
{ATGEN_IncomingBattery,		"_OBS:"		 	,0x00,0x00,ID_IncomingFrame      },

{ATGEN_IncomingSMS,		"\x0D\x0A+CMTI:" 	,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_IncomingSMS,		"+CMTI:" 	 	,0x00,0x00,ID_IncomingFrame	 },

{ATGEN_IncomingSMSCInfo,	"\x0D\x0A^SCN:"		,0x00,0x00,ID_IncomingFrame	 },
{ATGEN_IncomingSMSCInfo,	"^SCN:"			,0x00,0x00,ID_IncomingFrame	 },

{NULL,				"\x00"			,0x00,0x00,ID_None		 }
};                                                                                      

GSM_Phone_Functions ATGENPhone = {
	"A2D|iPAQ|at|M20|MC35|C35i|5110|5130|5190|5210|6110|6130|6150|6190|6210|6250|6310|6310i|6510|7110|8210|8250|8290|8310|8390|8850|8855|8890|8910|9110|9210",
	ATGENReplyFunctions,
	ATGEN_Initialise,
	NONEFUNCTION,			/*	Terminate		*/
	ATGEN_DispatchMessage,
	ATGEN_GetModel,
	ATGEN_GetFirmware,
	ATGEN_GetIMEI,
	ATGEN_GetDateTime,
	ATGEN_GetAlarm,
	ATGEN_GetMemory,
	ATGEN_GetMemoryStatus,
	ATGEN_GetSMSC,
	ATGEN_GetSMSMessage,
	ATGEN_GetSMSFolders,
	ATGEN_GetManufacturer,
	ATGEN_GetNextSMSMessage,
	ATGEN_GetSMSStatus,
	NOTIMPLEMENTED,			/* 	SetIncomingSMS		*/
	ATGEN_GetNetworkInfo,
	ATGEN_Reset,
	ATGEN_DialVoice,
	ATGEN_AnswerCall,
	ATGEN_CancelCall,
	ATGEN_GetRingtone,
	NOTSUPPORTED,			/* 	GetWAPBookmark		*/
	ATGEN_GetBitmap,		/* 	GetBitmap		*/
	ATGEN_SetRingtone,
	ATGEN_SaveSMSMessage,
	ATGEN_SendSMSMessage,
	ATGEN_SetDateTime,
	NOTIMPLEMENTED,			/*	SetAlarm		*/
	ATGEN_SetBitmap,		/*	SetBitmap		*/
	ATGEN_SetMemory,
	ATGEN_DeleteSMSMessage,
	NOTSUPPORTED,			/* 	SetWAPBookmark 		*/
	NOTSUPPORTED,	 		/* 	DeleteWAPBookmark 	*/
	NOTSUPPORTED,			/* 	GetWAPSettings 		*/
	NOTIMPLEMENTED,			/*	SetIncomingCB		*/
	ATGEN_SetSMSC,
	NOTSUPPORTED,			/*	GetManufactureMonth	*/
	NOTSUPPORTED,			/*	GetProductCode		*/
	NOTSUPPORTED,			/*	GetOriginalIMEI		*/
	NOTSUPPORTED,			/*	GetHardware		*/
	NOTSUPPORTED,			/*	GetPPM			*/
	ATGEN_PressKey,
	NOTSUPPORTED,			/*	GetToDo			*/
	NOTSUPPORTED,			/*	DeleteAllToDo		*/
	NOTSUPPORTED,			/*	SetToDo			*/
	NOTSUPPORTED,			/*	GetToDoStatus		*/
	NOTSUPPORTED,			/*	PlayTone		*/
	ATGEN_EnterSecurityCode,
	ATGEN_GetSecurityStatus,
	NOTSUPPORTED, 			/*	GetProfile		*/
	NOTSUPPORTED,			/*	GetRingtonesInfo	*/
	NOTSUPPORTED,			/* 	SetWAPSettings 		*/
	NOTSUPPORTED,			/*	GetSpeedDial		*/
	NOTSUPPORTED,			/*	SetSpeedDial		*/
	ATGEN_ResetPhoneSettings,
	ATGEN_SendDTMF,
	ATGEN_GetDisplayStatus,
	ATGEN_SetAutoNetworkLogin,
	NOTSUPPORTED, 			/*	SetProfile		*/
	ATGEN_GetSIMIMSI,
	NONEFUNCTION,			/*	SetIncomingCall		*/
    	ATGEN_GetNextCalendar,
	ATGEN_DelCalendarNote,
	ATGEN_AddCalendarNote,
	ATGEN_GetBatteryCharge,
	ATGEN_GetSignalQuality,
 	NOTSUPPORTED,       		/*  	GetCategory 		*/
 	NOTSUPPORTED,        		/*  	GetCategoryStatus 	*/
    	NOTSUPPORTED,			/*  	GetFMStation        	*/
    	NOTSUPPORTED,			/* 	SetFMStation        	*/
    	NOTSUPPORTED,			/* 	ClearFMStations       	*/
	NOTSUPPORTED,			/* 	SetIncomingUSSD		*/
	NOTSUPPORTED,			/* 	DeleteUserRingtones	*/
	NOTSUPPORTED,			/* 	ShowStartInfo		*/
	NOTSUPPORTED,			/* 	GetNextFileFolder	*/
	NOTSUPPORTED,			/* 	GetFilePart		*/
	NOTSUPPORTED,			/* 	AddFile			*/
	NOTSUPPORTED, 			/* 	GetFileSystemStatus	*/
	NOTSUPPORTED,			/* 	DeleteFile		*/
	NOTSUPPORTED,			/* 	AddFolder		*/
	NOTSUPPORTED,			/* 	GetMMSSettings		*/
	NOTSUPPORTED,			/* 	SetMMSSettings		*/
 	NOTSUPPORTED,			/* 	HoldCall 		*/
 	NOTSUPPORTED,			/* 	UnholdCall 		*/
 	NOTSUPPORTED,			/* 	ConferenceCall 		*/
 	NOTSUPPORTED,			/* 	SplitCall		*/
 	NOTSUPPORTED,			/* 	TransferCall		*/
 	NOTSUPPORTED,			/* 	SwitchCall		*/
 	NOTSUPPORTED,			/* 	GetCallDivert		*/
 	NOTSUPPORTED,			/* 	SetCallDivert		*/
 	NOTSUPPORTED,			/* 	CancelAllDiverts	*/
 	NOTSUPPORTED,			/* 	AddSMSFolder		*/
 	NOTSUPPORTED,			/* 	DeleteSMSFolder		*/
	NOTSUPPORTED,			/* 	GetGPRSAccessPoint	*/
	NOTSUPPORTED,			/* 	SetGPRSAccessPoint	*/
	NOTSUPPORTED,			/* 	GetLocale		*/
	NOTSUPPORTED,			/* 	SetLocale		*/
	NOTSUPPORTED,			/* 	GetCalendarSettings	*/
	NOTSUPPORTED			/* 	SetCalendarSettings	*/
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
