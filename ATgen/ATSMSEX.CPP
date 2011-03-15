#include "stdafx.h"
#include "Atgen.h"
#include "AtGenFundef.h"
#include "coding.h"
extern CATgenApp theApp;

extern GSM_Reply_MsgType ReplymsgType;
GSM_SMSMessageLayout PHONE_SMSDeliverEX = {
	35 	/*	SMS Text	*/,	16  /*	Phone number	*/,
	0 	/*	SMSC Number 	*/,	14  /*	TPDCS		*/,
	28 	/*	SendingDateTime	*/,	255 /*	SMSCDateTime	*/,
	255	/*	TPStatus	*/,	15  /*	TPUDL		*/,
	255	/*	TPVP		*/,	12  /*	firstbyte	*/,
	255	/*	TPMR		*/,	13  /*  TPPID		*/};
GSM_SMSMessageLayout PHONE_SMSSubmitEX = {
	36 	/*	SMS Text	*/,	17  /*	Phone number	*/,
	0 	/*	SMSC Number 	*/,	15  /*	TPDCS		*/,
	255 	/*	SendingDateTime	*/,	255 /*	SMSCDateTime	*/,
	255	/*	TPStatus	*/,	16  /*	TPUDL		*/,
	29	/*	TPVP		*/,	12  /*	firstbyte	*/,
	13	/*	TPMR		*/,	14  /*  TPPID		*/};
GSM_SMSMessageLayout PHONE_SMSStatusReportEX = {
	255 	/*	SMS Text	*/,	15  /*	Phone number	*/,
	0 	/*	SMSC Number 	*/,	255 /*	TPDCS		*/,
	27 	/*	SendingDateTime	*/,	34  /*	SMSCDateTime	*/,
	14	/*	TPStatus	*/,	255 /*	TPUDL		*/,
	255	/*	TPVP		*/,	12  /*	firstbyte	*/,
	13	/*	TPMR		*/,	255 /*  TPPID??		*/};

GSM_Error ATGEN_ReplyGetSMSCEX(GSM_Protocol_Message msg)
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
		if ((len > 20) && (len % 4 == 0) && (strchr((char*)buffer + nshift, '+') == NULL)) {
			/* This is probably unicode encoded number */
			DecodeHexUnicode(SMSC->Number,buffer + nshift,len);
		} else  {
			EncodeUnicode(SMSC->Number,buffer + nshift,len);
		}
		smprintf(theApp.m_pDebuginfo, "Number: \"%s\"\n",DecodeUnicodeString(SMSC->Number));

		/* Format of SMSC number */
		current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
		smprintf(theApp.m_pDebuginfo, "Format %s\n",buffer);
		/* International number */
		if (!strcmp((char*)buffer,"145")) {
			sprintf((char*)buffer+1,"%s",DecodeUnicodeString(SMSC->Number));
			if (strlen((char*)buffer+1)!=0 && buffer[1] != '+') {
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


GSM_Error ATGEN_ReplyDeleteSMSMessageEX(GSM_Protocol_Message msg)
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

GSM_Error ATGEN_ReplyGetSMSMemoriesEX(GSM_Protocol_Message msg)
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
		if (strstr((char*)msg.Buffer, "), (") != NULL || strstr((char*)msg.Buffer, "),(") != NULL) {
			theApp.m_CanSaveSMS = true;
		}

		if (strstr((char*)msg.Buffer, "\"SM\"") != NULL) theApp.m_SIMSMSMemory = AT_AVAILABLE;
		else theApp.m_SIMSMSMemory = AT_NOTAVAILABLE;

		if (strstr((char*)msg.Buffer, "\"ME\"") != NULL) theApp.m_PhoneSMSMemory = AT_AVAILABLE;
		else theApp.m_PhoneSMSMemory = AT_NOTAVAILABLE;
	
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
GSM_Error ATGEN_ReplyAddSMSMessageEX(GSM_Protocol_Message msg)
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

GSM_Error ATGEN_ReplyGetSMSMessageEX(GSM_Protocol_Message msg)
{
//	if (theApp.m_ManufacturerID == AT_LG) 
//		return LG_ReplyGetSMSMessage(msg);

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
		switch (theApp.m_SMSMode) {
		case SMS_AT_PDU:
			CopyLineString(buffer, msg.Buffer, theApp.m_Lines, 2);
			switch (buffer[7]) {
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
					smsframe[PHONE_SMSStatusReportEX.TPMR]=buffer[current++];
					current2=((buffer[current])+1)/2+1;
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSStatusReportEX.Number+i]=buffer[current++];
					for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReportEX.DateTime+i]=buffer[current++];
					smsframe[0] = 0;
					for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReportEX.SMSCTime+i]=buffer[current++];
					smsframe[PHONE_SMSStatusReportEX.TPStatus]=buffer[current];
					GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSStatusReportEX);
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
						for(i=0;i<4;i++) smsframe[PHONE_SMSDeliverEX.Number+i]=buffer[current++];
						current+=6;
						for(i=0;i<current2-3;i++) smsframe[PHONE_SMSDeliverEX.Number+i+4]=buffer[current++];
					} else {
						for(i=0;i<current2+1;i++) smsframe[PHONE_SMSDeliverEX.Number+i]=buffer[current++];
					}
				} else {
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSDeliverEX.Number+i]=buffer[current++];
				}
				smsframe[PHONE_SMSDeliverEX.TPPID] = buffer[current++];
				smsframe[PHONE_SMSDeliverEX.TPDCS] = buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSDeliverEX.DateTime+i]=buffer[current++];
				smsframe[PHONE_SMSDeliverEX.TPUDL] = buffer[current++];
				for(i=0;i<smsframe[PHONE_SMSDeliverEX.TPUDL];i++) smsframe[i+PHONE_SMSDeliverEX.Text]=buffer[current++];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSDeliverEX);
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
				smsframe[PHONE_SMSSubmitEX.TPMR] = buffer[current++];
				current2=((buffer[current])+1)/2+1;
				if (IsPhoneFeatureAvailable(theApp.m_pMobileInfo, F_M20SMS)) {
					if (buffer[current+1]==NUMBER_ALPHANUMERIC_NUMBERING_PLAN_UNKNOWN) {
						smprintf(theApp.m_pDebuginfo, "Trying to read alphanumeric number\n");
						for(i=0;i<4;i++) smsframe[PHONE_SMSSubmitEX.Number+i]=buffer[current++];
						current+=6;
						for(i=0;i<current2-3;i++) smsframe[PHONE_SMSSubmitEX.Number+i+4]=buffer[current++];
					} else {
						for(i=0;i<current2+1;i++) smsframe[PHONE_SMSSubmitEX.Number+i]=buffer[current++];
					}
				} else {
					for(i=0;i<current2+1;i++) smsframe[PHONE_SMSSubmitEX.Number+i]=buffer[current++];
				}
				smsframe[PHONE_SMSSubmitEX.TPPID] = buffer[current++];
				smsframe[PHONE_SMSSubmitEX.TPDCS] = buffer[current++];
				/* See GSM 03.40 9.2.3.3 - TPVP can not exist in frame */
				if ((smsframe[12] & 0x18)!=0) current++; //TPVP is ignored now
				smsframe[PHONE_SMSSubmitEX.TPUDL] = buffer[current++];
				for(i=0;i<smsframe[PHONE_SMSSubmitEX.TPUDL];i++) smsframe[i+PHONE_SMSSubmitEX.Text]=buffer[current++];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSSubmitEX);
				return ERR_NONE;
			case 0x02:
				smprintf(theApp.m_pDebuginfo, "SMS type - status report\n");
				sms->PDU 	 = SMS_Status_Report;
				sms->Folder 	 = 1;	/*INBOX SIM*/
				sms->InboxFolder = true;
				smprintf(theApp.m_pDebuginfo, "TPMR is %02x\n",buffer[current]);
				smsframe[PHONE_SMSStatusReportEX.TPMR] = buffer[current++];
				current2=((buffer[current])+1)/2+1;
				for(i=0;i<current2+1;i++) smsframe[PHONE_SMSStatusReportEX.Number+i]=buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReportEX.DateTime+i]=buffer[current++];
				for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReportEX.SMSCTime+i]=buffer[current++];
				smsframe[PHONE_SMSStatusReportEX.TPStatus]=buffer[current];
				GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSStatusReportEX);
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
				buffer[PHONE_SMSStatusReportEX.TPStatus] = TPStatus;
				error=GSM_DecodeSMSFrameStatusReportData(sms, buffer, PHONE_SMSStatusReportEX);
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
					DecodeHexBin(buffer+PHONE_SMSDeliverEX.Text, msg.Buffer+current, TPUDL*2);
					buffer[PHONE_SMSDeliverEX.firstbyte] 	= firstbyte;
					buffer[PHONE_SMSDeliverEX.TPDCS] 		= TPDCS;
					buffer[PHONE_SMSDeliverEX.TPUDL] 		= TPUDL;
					return GSM_DecodeSMSFrameText(sms, buffer, PHONE_SMSDeliverEX);
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


GSM_Error ATGEN_ReplyGetSMSStatusEX(GSM_Protocol_Message msg)
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
			if (SMSStatus->SIMSize == 0) {
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

GSM_Error ATGEN_GetSMSMemoriesEX(GSM_SMSMemoryInfo* pSMSmemoryinfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
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
	error = pWriteCommandfn ((unsigned char *)"AT+CPMS=?\r", 10, 0x00, 8,false,NULL,&ReplymsgType,ATGEN_ReplyGetSMSMemoriesEX);
	if(pSMSmemoryinfo)
	{
		pSMSmemoryinfo->m_CanSaveSMS = theApp.m_CanSaveSMS;
		pSMSmemoryinfo->m_PhoneSMSMemory = theApp.m_PhoneSMSMemory;
		pSMSmemoryinfo->m_SIMSMSMemory = theApp.m_SIMSMSMemory;
	}
	return error;
}

GSM_Error ATGEN_SetSMSMemoryEX(bool SIM,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
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
		ATGEN_GetSMSMemoriesEX(NULL,pWriteCommandfn,pDebuginfo);
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
		error= pWriteCommandfn ((unsigned char *)req, reqlen, 0x00, 15,false,NULL,&ReplymsgType,ATGEN_GenericReply);
	
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
		error= pWriteCommandfn ((unsigned char *)req, reqlen, 0x00, 15,false,NULL,&ReplymsgType,ATGEN_GenericReply);
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




GSM_Error ATGEN_GetSMSModeEX(GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	theApp.m_pDebuginfo = pDebuginfo;
//	GSM_Phone_ATGENData	*Priv = &s->Phone.Data.Priv.ATGEN;
  	GSM_Error 		error;

	if (theApp.m_SMSMode != SMS_AT_ERROR) return ERR_NONE;
//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;

	smprintf(theApp.m_pDebuginfo, "Trying SMS PDU mode\n");
//	error=GSM_WaitFor (s, "AT+CMGF=0\r", 10, 0x00, 3, ID_GetSMSMode);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CMGF");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	error= pWriteCommandfn ((unsigned char *)"AT+CMGF=0\r", 10, 0x00, 6,false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error==ERR_NONE) {
		theApp.m_SMSMode = SMS_AT_PDU;
		return ERR_NONE;
	}

	smprintf(theApp.m_pDebuginfo, "Trying SMS text mode\n");
//	error=GSM_WaitFor (s, "AT+CMGF=1\r", 10, 0x00, 3, ID_GetSMSMode);
	error= pWriteCommandfn ((unsigned char *)"AT+CMGF=1\r", 10, 0x00, 6,false,NULL,&ReplymsgType,ATGEN_GenericReply);
	if (error==ERR_NONE) {
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

GSM_Error ATGEN_GetSMSEX(OnePhoneModel *pMobileInfo, GSM_MultiSMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
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
	
//	error=ATGEN_GetSMSLocationEX(&sms->SMS[0], &folderid, &location,pWriteCommandfn,pDebuginfo);
//	if (error!=ERR_NONE) return error;
//	if(sms->SMS[0].Location > theApp.m_LastSMSStatus.SIMUsed)	
//		sms->SMS[0].Location -= theApp.m_LastSMSStatus.SIMUsed;
	location = sms->SMS[0].Location;

	if (theApp.m_SMSMemory == MEM_ME && IsPhoneFeatureAvailable(pMobileInfo, F_SMSME900)) add = 899;
	sprintf((char*)req, "AT+CMGR=%i\r", location + add);

	error=ATGEN_GetSMSMode(pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

	theApp.m_GetSMSMessage=sms;
	smprintf(theApp.m_pDebuginfo, "Getting SMS\n");

	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CMGR");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	theApp.m_pMobileInfo = pMobileInfo;
	error= pWriteCommandfn (req, strlen((char*)req), 0x00, 10,false,NULL,&ReplymsgType,ATGEN_ReplyGetSMSMessageEX);

	if (error==ERR_NONE) {
		getfolder = sms->SMS[0].Folder;

		sms->SMS[0].Folder = getfolder;
		sms->SMS[0].Memory = MEM_SM;
		if (getfolder > 2) sms->SMS[0].Memory = MEM_ME;
	}
	return error;
}
GSM_Error ATGEN_GetNextSMSEX(OnePhoneModel *pMobileInfo, GSM_MultiSMSMessage *sms, bool start, int startIndex,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
//	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Error 		error;
	int			usedsms;
//	if(s->Phone.Data.Priv.ATGEN.OBEX) return ERR_OBEXMODE;

	if (theApp.m_PhoneSMSMemory == 0) {
		error = ATGEN_SetSMSMemoryEX(false,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (theApp.m_SIMSMSMemory == 0) {
		error = ATGEN_SetSMSMemoryEX(true,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) return error;
	}
	if (theApp.m_SIMSMSMemory == AT_NOTAVAILABLE && theApp.m_PhoneSMSMemory == AT_NOTAVAILABLE) return ERR_NOTSUPPORTED;

	if (start) {
		error=ATGEN_GetSMSStatus(&theApp.m_LastSMSStatus,pWriteCommandfn,pDebuginfo);
		if (error!=ERR_NONE) return error;
		theApp.m_SMSMemFlag			= false;
		theApp.m_LastSMSRead		= 0;
		sms->SMS[0].Location 		= startIndex -1;
		error = ATGEN_SetSMSMemoryEX(true,pWriteCommandfn,pDebuginfo);
	}
	while (true) {
		sms->SMS[0].Location++;
		if (theApp.m_SMSMemFlag == false) {
			if (theApp.m_SIMSMSMemory == AT_AVAILABLE) {
				usedsms = theApp.m_LastSMSStatus.SIMUsed;
			} else {
				usedsms = theApp.m_LastSMSStatus.PhoneUsed;
			}
					
			if (theApp.m_LastSMSRead >= usedsms) {
				if (theApp.m_PhoneSMSMemory == AT_NOTAVAILABLE || theApp.m_LastSMSStatus.PhoneUsed==0) return ERR_EMPTY;
				theApp.m_LastSMSRead	= 0;
				sms->SMS[0].Location 	= startIndex ;
				theApp.m_SMSMemFlag		= true;
				error = ATGEN_SetSMSMemoryEX(false,pWriteCommandfn,pDebuginfo);
				if(error != ERR_NONE)	return error;
			}
		} else {
			if (theApp.m_PhoneSMSMemory == AT_NOTAVAILABLE) return ERR_EMPTY;
			if (theApp.m_LastSMSRead>=theApp.m_LastSMSStatus.PhoneUsed) return ERR_EMPTY;
		}
		sms->SMS[0].Folder = 0;
		error=ATGEN_GetSMSEX(pMobileInfo,sms,pWriteCommandfn,pDebuginfo);
		if (error==ERR_NONE) {
			theApp.m_LastSMSRead++;
			break;
		}
		if (error != ERR_EMPTY && error != ERR_INVALIDLOCATION) return error;
	}
	return error;
}

GSM_Error ATGEN_GetSMSCEX(GSM_SMSC *smsc,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
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
	return pWriteCommandfn ((unsigned char *)"AT+CSCA?\r", 9, 0x00, 8,false,NULL,&ReplymsgType,ATGEN_ReplyGetSMSCEX);
}

GSM_Error PHONE_EncodeSMSFrameEX(GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout, int *length, bool clear
							   ,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	GSM_Error error;

	if (SMS->SMSC.Location!=0) {
		error = ATGEN_GetSMSCEX(&SMS->SMSC,pWriteCommandfn,pDebuginfo);
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
	return GSM_EncodeSMSFrame(SMS, buffer, Layout, length, clear);
}

GSM_Error ATGEN_PrivAddSMSEX(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,bool bSetSMSMode,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
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
		 error=ATGEN_SetSMSMemoryEX(true,pWriteCommandfn,pDebuginfo);
		 break;
	case 2:  error=ATGEN_SetSMSMemoryEX(true,pWriteCommandfn,pDebuginfo);	/* Outbox SIM */
	 	 break;
	case 3:  sms->PDU = SMS_Deliver;
		 error=ATGEN_SetSMSMemoryEX(false,pWriteCommandfn,pDebuginfo);	/* Inbox phone */
		 break;
	case 4:  error=ATGEN_SetSMSMemoryEX(false,pWriteCommandfn,pDebuginfo);	/* Outbox phone */
		 break;
	default: return ERR_NOTSUPPORTED;
	}
	if (error!=ERR_NONE) return error;

	if(bSetSMSMode == false)
	{
		error=ATGEN_GetSMSModeEX(pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;
	}
	else
	{
		error=ATGEN_SetSMSMode(true,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE)
		{
			error=ATGEN_SetSMSMode(false,pWriteCommandfn,pDebuginfo);
			if (error != ERR_NONE) return error;
		}
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
				error=ATGEN_GetSMSModeEX(pWriteCommandfn,pDebuginfo);
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
		error = pWriteCommandfn ((unsigned char *)buffer, strlen((char*)buffer), 0x00, 16,false,NULL,&ReplymsgType,ATGEN_ReplyAddSMSMessageEX);
	
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

			error = pWriteCommandfn ((unsigned char *)"\x1A", 1, 0x00, 20,false,NULL,&ReplymsgType,ATGEN_ReplyAddSMSMessageEX);
		

			if (error != ERR_TIMEOUT)
			{
				if(bSetSMSMode == false)
				{
					if(sms->Folder == 1 || sms->Folder ==2)
						folderid =1 ;
					else
						folderid =2 ;
					if (theApp.m_SMSMemory == MEM_ME && IsPhoneFeatureAvailable(pMobileInfo, F_SMSME900)) sms->Location -= 899;

				}
				else
				{
					if(theApp.m_SMSMemory == MEM_SM)
						sms->Location =( -1)*sms->Location;
				}
				if(theApp.m_ManufacturerID == AT_Motorola && error == ERR_NONE)
				{
					wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT\r");
					ReplymsgType.CheckInfo[0].subtypechar = 0;
					ReplymsgType.CheckInfo[0].subtype = 0x00;
					pWriteCommandfn ((unsigned char *)"AT\r", 3, 0x00, 4, false,NULL,&ReplymsgType,ATGEN_GenericReply);
				}
				return error;
			}
		} else {
			smprintf(theApp.m_pDebuginfo, "Escaping SMS mode\n");

			error2 = pWriteCommandfn ((unsigned char *)"\x1B\r", 2, 0x00, 0,false,NULL,NULL,NULL);
			if (error2 != ERR_NONE) return error2;
			
			if(bSetSMSMode == false)
			{
				if(sms->Folder == 1 || sms->Folder ==2)
					folderid ='1';
				else
					folderid ='2';

				if (theApp.m_SMSMemory == MEM_ME && IsPhoneFeatureAvailable(pMobileInfo, F_SMSME900)) sms->Location -= 899;

			}
			else
			{
				if(theApp.m_SMSMemory == MEM_SM)
					sms->Location =( -1)*sms->Location;
			}

			if(theApp.m_ManufacturerID == AT_Motorola && error == ERR_NONE)
			{
				wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT\r");
				ReplymsgType.CheckInfo[0].subtypechar = 0;
				ReplymsgType.CheckInfo[0].subtype = 0x00;
				pWriteCommandfn ((unsigned char *)"AT\r", 3, 0x00, 4, false,NULL,&ReplymsgType,ATGEN_GenericReply);
			}
			return error;
		}
    }

	return error;
}
GSM_Error ATGEN_AddSMSForTextModeEX(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,bool bSetSMSMode,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	GSM_Error 		error, error2;
	int			state,reply, current, current2;
	unsigned char		buffer[1000], hexreq[1000];
//	GSM_Phone_Data		*Phone = &s->Phone.Data;
//	unsigned char		*statetxt;
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
		 error=ATGEN_SetSMSMemoryEX(true,pWriteCommandfn,pDebuginfo);
		 break;
	case 2:  error=ATGEN_SetSMSMemoryEX(true,pWriteCommandfn,pDebuginfo);	/* Outbox SIM */
	 	 break;
	case 3:  sms->PDU = SMS_Deliver;
		 error=ATGEN_SetSMSMemoryEX(false,pWriteCommandfn,pDebuginfo);	/* Inbox phone */
		 break;
	case 4:  error=ATGEN_SetSMSMemoryEX(false,pWriteCommandfn,pDebuginfo);	/* Outbox phone */
		 break;
	default: return ERR_NOTSUPPORTED;
	}
	if (error!=ERR_NONE) return error;

	if(bSetSMSMode == false)
	{
		error=ATGEN_GetSMSModeEX(pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;
	}
	else
	{
		error=ATGEN_SetSMSMode(true,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE)
		{
			error=ATGEN_SetSMSMode(false,pWriteCommandfn,pDebuginfo);
			if (error != ERR_NONE) return error;
		}
	}
	if(theApp.m_SMSMode!=SMS_AT_TXT)
	{
		error = ATGEN_MakeSMSFrame(sms, hexreq, &current, &current2,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;
	}
	else
	{
/*		switch (sms->Coding)
		{
		case SMS_Coding_Default:
			/* If not SMS with UDH, it's as normal text */
/*			if (sms->UDH.Type==UDH_NoUDH) {
				strcpy((char*)hexreq,(char*)DecodeUnicodeString(sms->Text));
				current2 = UnicodeLength(sms->Text);
				break;
			}
			case SMS_Coding_Unicode:        
			case SMS_Coding_8bit:
	//		error=PHONE_EncodeSMSFrameEX(message,buffer,PHONE_SMSDeliverEX,current,true,pWriteCommandfn,pDebuginfo);
	//		if (error != ERR_NONE) return error;
	//		EncodeHexBin (hexreq, buffer+PHONE_SMSDeliverEX.Text, buffer[PHONE_SMSDeliverEX.TPUDL]);
			EncodeHexBin ((unsigned char *)hexreq, DecodeUnicodeString(sms->Text), UnicodeLength(sms->Text));
			current2= strlen((char*)hexreq);
			break;
		}*/
		switch (theApp.m_PBKCharset) {
		case AT_PBK_HEX:
			EncodeHexBin((unsigned char *)hexreq, DecodeUnicodeString(sms->Text), UnicodeLength(sms->Text));
			current2 = strlen((char*)hexreq);
			break;
		case AT_PBK_GSM:
			current2 = UnicodeLength(sms->Text);
			EncodeDefault((unsigned char *)hexreq, sms->Text, &current2, true, NULL);
			break;
		case AT_PBK_UTF8:
//			len=EncodeCString2UTF8(DecodeUnicodeString(entry->Entries[Name].Text),name);
			current2=EncodeUnicode2UTF8(sms->Text,(unsigned char *)hexreq);// for _UNICODE
			current2 = strlen((char*)hexreq);
			break;
		case AT_PBK_UCS2:
			EncodeHexUnicode((unsigned char *)hexreq, sms->Text, UnicodeLength(sms->Text));
			current2 = strlen((char*)hexreq);
			break;
		case AT_PBK_PCCP437:
			/* FIXME: correctly decode PCCP437 */
			current2 = UnicodeLength(sms->Text);
			EncodeDefault((unsigned char *)hexreq, sms->Text, &current2, true, NULL);
			break;
		}
	}
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
				error=ATGEN_GetSMSModeEX(pWriteCommandfn,pDebuginfo);
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
/*		if (sms->PDU == SMS_Deliver) {
			statetxt =(unsigned char *) "REC UNREAD";
			if (sms->State == SMS_Read || sms->State == SMS_Sent) statetxt =(unsigned char *) "REC READ";
		} else {
			statetxt =(unsigned char *) "STO UNSENT";
			if (sms->State == SMS_Read || sms->State == SMS_Sent) statetxt = (unsigned char *)"STO SENT";
		}*/
		/* Siemens M20 */
	//	if (IsPhoneFeatureAvailable(pMobileInfo, F_M20SMS)) {
			/* No (good and 100% working) support for alphanumeric numbers */
			/* FIXME: Try to autodetect support for <stat> (statetxt) parameter although:
			 * Siemens M20 supports +CMGW <stat> specification but on my model it just
			 * reports ERROR (and <stat> is not respected).
			 * Fortunately it will write "+CMGW: <index>\n" before and the message gets written
			 */
	/*		if (sms->Number[1]!='+' && (sms->Number[1]<'0' || sms->Number[1]>'9')) {
		        	sprintf((char*)buffer, "AT+CMGW=\"123\",,\"%s\"\r",statetxt);
			} else {
		        	sprintf((char*)buffer, "AT+CMGW=\"%s\",,\"%s\"\r",DecodeUnicodeString(sms->Number),statetxt);
			}
		} else {
			sprintf((char*)buffer, "AT+CMGW=\"%s\",,\"%s\"\r",DecodeUnicodeString(sms->Number),statetxt);
		}*/
		sprintf((char*)buffer, "AT+CMGW=\"%s\"\r",DecodeUnicodeString(sms->Number));

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
		error = pWriteCommandfn ((unsigned char *)buffer, strlen((char*)buffer), 0x00, 16,false,NULL,&ReplymsgType,ATGEN_ReplyAddSMSMessageEX);
	
		if (error == ERR_NONE) 
		{

			error = pWriteCommandfn ((unsigned char *)hexreq, current2, 0x00, 0,false,NULL,NULL,NULL);
			if (error!=ERR_NONE) return error;
			my_sleep(500);
			/* CTRL+Z ends entering */
			if(theApp.m_SMSMode!=SMS_AT_TXT)
				error = pWriteCommandfn ((unsigned char *)"\x1A", 1, 0x00, 20,false,NULL,&ReplymsgType,ATGEN_ReplyAddSMSMessageEX);
			else
				error = pWriteCommandfn ((unsigned char *)"\x1A\r", 2, 0x00, 20,false,NULL,&ReplymsgType,ATGEN_ReplyAddSMSMessageEX);
		

			if (error != ERR_TIMEOUT)
			{
				if(bSetSMSMode == false)
				{
					if(sms->Folder == 1 || sms->Folder ==2)
						folderid =1 ;
					else
						folderid =2 ;
					if (theApp.m_SMSMemory == MEM_ME && IsPhoneFeatureAvailable(pMobileInfo, F_SMSME900)) sms->Location -= 899;

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

			error2 = pWriteCommandfn ((unsigned char *)"\x1B\r", 2, 0x00, 0,false,NULL,NULL,NULL);
			if (error2 != ERR_NONE) return error2;
			
			if(bSetSMSMode == false)
			{
				if(sms->Folder == 1 || sms->Folder ==2)
					folderid ='1';
				else
					folderid ='2';

				if (theApp.m_SMSMemory == MEM_ME && IsPhoneFeatureAvailable(pMobileInfo, F_SMSME900)) sms->Location -= 899;

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
GSM_Error ATGEN_AddSMSEX(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error err =ATGEN_PrivAddSMSEX(pMobileInfo,sms,false,pSetATProtocolData,pWriteCommandfn,pDebuginfo);
	if(IsPhoneFeatureAvailable(pMobileInfo, F_SMS_SHIFTMEINDEX))
	{
		if(sms->Memory == MEM_ME)
			sms->Location -=theApp.m_SIMSize;
	}
	return err;
}
GSM_Error ATGEN_DeleteSMSEX(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	theApp.m_pDebuginfo = pDebuginfo;
	unsigned char		req[20], folderid;
	GSM_Error		error;
	int			location;
	int add = 0;
	GSM_MultiSMSMessage	msms;

	msms.Number = 0;
	msms.SMS[0] = *sms;

	/* By reading SMS we check if it is really inbox/outbox */
	if(sms->Memory == MEM_SM)
		ATGEN_SetSMSMemoryEX(true,pWriteCommandfn,pDebuginfo);
	else
		ATGEN_SetSMSMemoryEX(false,pWriteCommandfn,pDebuginfo);
	error = ATGEN_GetSMSEX(pMobileInfo,&msms,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;
	
	location = sms->Location;
//	error = ATGEN_GetSMSLocationEX(sms, &folderid, &location,pWriteCommandfn,pDebuginfo);
//	if (error != ERR_NONE) return error;

	if (theApp.m_SMSMemory == MEM_ME && IsPhoneFeatureAvailable(pMobileInfo, F_SMSME900)) add = 899;
	sprintf((char*)req, "AT+CMGD=%i\r",location+ add);

	smprintf(theApp.m_pDebuginfo, "Deleting SMS\n");
//	return GSM_WaitFor (s, req, strlen(req), 0x00, 5, ID_DeleteSMSMessage);
	ReplymsgType.nCount = 1;
	wsprintf((char*)ReplymsgType.CheckInfo[0].msgtype,"AT+CMGD");
	ReplymsgType.CheckInfo[0].subtypechar = 0;
	ReplymsgType.CheckInfo[0].subtype = 0x00;
	return  pWriteCommandfn ( (unsigned char *)req, strlen((char*)req), 0x00, 10,false,NULL,&ReplymsgType,ATGEN_ReplyDeleteSMSMessageEX);
}

//#ifdef SENDBYTEXTMODE

void GSM_SetDefaultSMSDataEX(GSM_SMSMessage *SMS)
{
	SMS->Class			= -1;
	SMS->SMSC.Location		= 1;
	SMS->SMSC.Name[0]		= 0;
	SMS->SMSC.Name[1]		= 0;
	SMS->SMSC.Number[0]		= 0;
	SMS->SMSC.Number[1]		= 0;
	SMS->SMSC.DefaultNumber[0]	= 0;
	SMS->SMSC.DefaultNumber[1]	= 0;
	SMS->SMSC.Format		= SMS_FORMAT_Text;
	SMS->SMSC.Validity.Format	= SMS_Validity_RelativeFormat;
	SMS->SMSC.Validity.Relative	= SMS_VALID_Max_Time;
	SMS->ReplyViaSameSMSC		= false;
	SMS->UDH.Type			= UDH_NoUDH;
	SMS->UDH.Length			= 0;
	SMS->UDH.Text[0] 		= 0;
	SMS->UDH.ID8bit			= 0;
	SMS->UDH.ID16bit		= 0;
	SMS->UDH.PartNumber		= 0;
	SMS->UDH.AllParts		= 0;
	SMS->Coding			= SMS_Coding_8bit;
	SMS->Text[0] 			= 0;
	SMS->Text[1] 			= 0;
	SMS->PDU			= SMS_Submit;
	SMS->RejectDuplicates		= false;
	SMS->MessageReference		= 0;
	SMS->ReplaceMessage		= 0;
	SMS->Length			= 0;
	SMS->DeliveryStatus		= 0;
	SMS->ReplyViaSameSMSC		= 0;
	SMS->Class			= 0;
	SMS->MessageReference		= 0;

	/* This part is required to save SMS */
	SMS->State			= SMS_UnSent;
	SMS->Location			= 0;
//	SMS->Memory			= 0;
	SMS->Folder			= 0x02;	/*Outbox*/
	SMS->InboxFolder		= false;
	GSM_GetCurrentDateTime (&SMS->DateTime);
	GSM_GetCurrentDateTime (&SMS->SMSCTime);
	SMS->Name[0]			= 0;
	SMS->Name[1]			= 0;
}
GSM_Error ATGEN_DecodeNokiaSMSFileEX(GSM_File *pfile,GSM_MultiSMSMessage *pSMS)
{
	GSM_SetDefaultSMSDataEX(&pSMS->SMS[0]);
	int i = 175;
	int 			current = 0,current2;
	unsigned char 	smsframe[800];
//	GSM_Error		error;
	ZeroMemory(smsframe,800);
	unsigned char *buffer = pfile->Buffer+i;
	GSM_SMSMessage		*sms	= &pSMS->SMS[0];
	pSMS->Number 	 	= 1;
	/* We use locations from SMS layouts like in ../phone2.c(h) */
//	if(buffer[0]>0)
	{
		for(i=0;i<buffer[0]+1;i++) smsframe[i]=buffer[current++];
	}
	smsframe[12]=buffer[current++];
	if(smsframe[12]== 0x8C) return ERR_UNKNOWNFRAME;
	/* See GSM 03.40 section 9.2.3.1 */
	switch (smsframe[12] & 0x03)
	{
	case 0x00:
		smprintf(theApp.m_pDebuginfo, "SMS type - deliver\n");
		sms->PDU 	 = SMS_Deliver;
		current2=((buffer[current])+1)/2+1;
		for(i=0;i<current2+1;i++) smsframe[PHONE_SMSDeliverEX.Number+i]=buffer[current++];

		smsframe[PHONE_SMSDeliverEX.TPPID] = buffer[current++];
		smsframe[PHONE_SMSDeliverEX.TPDCS] = buffer[current++];
		for(i=0;i<7;i++) smsframe[PHONE_SMSDeliverEX.DateTime+i]=buffer[current++];
		smsframe[PHONE_SMSDeliverEX.TPUDL] = buffer[current++];
		for(i=0;i<smsframe[PHONE_SMSDeliverEX.TPUDL];i++) smsframe[i+PHONE_SMSDeliverEX.Text]=buffer[current++];
		GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSDeliverEX);
		return ERR_NONE;
	case 0x01:
		smprintf(theApp.m_pDebuginfo, "SMS type - submit\n");
		sms->PDU 	 = SMS_Submit;
		smsframe[PHONE_SMSSubmitEX.TPMR] = buffer[current++];
		current2=((buffer[current])+1)/2+1;
		for(i=0;i<current2+1;i++) smsframe[PHONE_SMSSubmitEX.Number+i]=buffer[current++];
		smsframe[PHONE_SMSSubmitEX.TPPID] = buffer[current++];
		smsframe[PHONE_SMSSubmitEX.TPDCS] = buffer[current++];
		/* See GSM 03.40 9.2.3.3 - TPVP can not exist in frame */
		if ((smsframe[12] & 0x18)!=0) current++; //TPVP is ignored now
		smsframe[PHONE_SMSSubmitEX.TPUDL] = buffer[current++];
		for(i=0;i<smsframe[PHONE_SMSSubmitEX.TPUDL];i++) smsframe[i+PHONE_SMSSubmitEX.Text]=buffer[current++];
		GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSSubmitEX);
		{
			int nBlockLen = buffer[current]*256+buffer[current+1]; 
			current+=2;
			int nPosBlock = 0;
			unsigned char *pBlock = buffer+current;
			while(nPosBlock<nBlockLen && pBlock[nPosBlock++]!=0x04 )
			{
				nPosBlock+=pBlock[nPosBlock]*256+pBlock[nPosBlock+1];
				nPosBlock+=2;
			}

			if(nPosBlock<nBlockLen && pBlock[nPosBlock-1]==0x04)
			{
				int nLen = pBlock[nPosBlock]*256+pBlock[nPosBlock+1];
				nPosBlock+=2;
				if(nLen>0 && (nPosBlock+nLen)<=nBlockLen)
				{
					memcpy(sms->Number,pBlock+nPosBlock,nLen);
					sms->Number[nLen] = 0;
				}
			}
		}
		return ERR_NONE;
	case 0x02:
		smprintf(theApp.m_pDebuginfo, "SMS type - status report\n");
		sms->PDU 	 = SMS_Status_Report;
		sms->Folder 	 = 1;	/*INBOX SIM*/
		sms->InboxFolder = true;
		smprintf(theApp.m_pDebuginfo, "TPMR is %02x\n",buffer[current]);
		smsframe[PHONE_SMSStatusReportEX.TPMR] = buffer[current++];
		current2=((buffer[current])+1)/2+1;
		for(i=0;i<current2+1;i++) smsframe[PHONE_SMSStatusReportEX.Number+i]=buffer[current++];
		for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReportEX.DateTime+i]=buffer[current++];
		for(i=0;i<7;i++) smsframe[PHONE_SMSStatusReportEX.SMSCTime+i]=buffer[current++];
		smsframe[PHONE_SMSStatusReportEX.TPStatus]=buffer[current];
		GSM_DecodeSMSFrame(sms,smsframe,PHONE_SMSStatusReportEX);
		return ERR_NONE;

	break;
	}
	return ERR_NONE;
}
