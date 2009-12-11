/* (c) 2002-2008 by Marcin Wiacek and Michal Cihar */

/**
 * @file atgen.c
 * @author Michal Čihař
 * @author Marcin Wiacek
 */
/**
 * @ingroup Phone
 * @{
 */
/**
 * @addtogroup ATPhone
 * @{
 */

#define _GNU_SOURCE
#include <gammu-config.h>

#ifdef GSM_ENABLE_ATGEN

#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>

#include "../../gsmcomon.h"
#include "../../gsmphones.h"
#include "../../misc/coding/coding.h"
#include "../../service/gsmpbk.h"
#include "../pfunc.h"

#include "atgen.h"
#include "atfunc.h"

#include "../../../helper/string.h"

GSM_Error ATGEN_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Error error;
	/*
	 * String value.
	 * It indicates the SMSC number.
	 * The numberr is composed '*', '#', '0'-'9'
	 * The number contains 20 characters at most.
	 */
	unsigned char smscCmdReq[GSM_MAX_NUMBER_LENGTH + 12]={'\0'};

	if (smsc->Location!=1) {
		return ERR_INVALIDLOCATION;
	}
	smprintf(s, "Setting SMSC\n");
	sprintf(smscCmdReq, "AT+CSCA=\"%s\"\r",DecodeUnicodeString(smsc->Number));
	ATGEN_WaitForAutoLen(s, smscCmdReq, 0x00, 4, ID_SetSMSC);
	return error;
}

GSM_Error ATGEN_ReplyGetSMSMemories(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	char *pos_start=NULL, *pos_end=NULL, *pos_tmp=NULL;
	const char *Line;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		/* Reply here is:
		 * (memories for reading)[, (memories for writing)[, (memories for storing received messages)]]
		 * each memory is in quotes,
		 * Example: ("SM"), ("SM"), ("SM")
		 *
		 * TODO: Reply can be also "SM", "SM", "SM"
		 *
		 * We need to get from this supported memories. For this case
		 * we assume, that just appearence of memory makes it
		 * available for everything. Then we need to find out whether
		 * phone supports writing to memory. This is done by searching
		 * for "), (", which will appear between lists.
		 *
		 * @todo: Add support for BM (broadcast messages) and SR (status reports).
		 */
		s->Phone.Data.Priv.ATGEN.PhoneSaveSMS = AT_NOTAVAILABLE;
		s->Phone.Data.Priv.ATGEN.SIMSaveSMS = AT_NOTAVAILABLE;

		Line = GetLineString(msg.Buffer, &Priv->Lines, 2);
		if (strcmp(Line, "+CPMS: ") == 0 && Priv->Manufacturer == AT_Samsung) {
			smprintf(s, "Assuming broken Samsung response, both memories available!\n");
			s->Phone.Data.Priv.ATGEN.PhoneSMSMemory = AT_AVAILABLE;
			s->Phone.Data.Priv.ATGEN.SIMSMSMemory = AT_AVAILABLE;
			s->Phone.Data.Priv.ATGEN.PhoneSaveSMS = AT_AVAILABLE;
			s->Phone.Data.Priv.ATGEN.SIMSaveSMS = AT_AVAILABLE;
			goto success;
		}

		if (strchr(msg.Buffer, '(') == NULL) {
			smprintf(s, "Assuming broken iWOW style response, no lists!\n");
			pos_start = strstr(msg.Buffer, "\", \"");

			if (pos_start == NULL) {
				pos_start = strstr(msg.Buffer, "\",\"");
			}
		} else {
			pos_start = strstr(msg.Buffer, "), (");

			if (pos_start == NULL) {
				pos_start = strstr(msg.Buffer, "),(");
			}
		}
		if (pos_start != NULL) {
			/* Detect which memories we can use for saving */
			pos_end = strchrnul(pos_start + 1, ')');
			pos_tmp = strstr(pos_start, "\"SM\"");

			if (pos_tmp != NULL && pos_tmp < pos_end) {
				s->Phone.Data.Priv.ATGEN.SIMSaveSMS = AT_AVAILABLE;
			}
			pos_tmp = strstr(pos_start, "\"ME\"");

			if (pos_tmp != NULL && pos_tmp < pos_end) {
				s->Phone.Data.Priv.ATGEN.PhoneSaveSMS = AT_AVAILABLE;
			}
		}
		if (strstr(msg.Buffer, "\"SM\"") != NULL) {
			s->Phone.Data.Priv.ATGEN.SIMSMSMemory = AT_AVAILABLE;
		} else {
			s->Phone.Data.Priv.ATGEN.SIMSMSMemory = AT_NOTAVAILABLE;
		}
		if (strstr(msg.Buffer, "\"ME\"") != NULL) {
			s->Phone.Data.Priv.ATGEN.PhoneSMSMemory = AT_AVAILABLE;
		} else {
			s->Phone.Data.Priv.ATGEN.PhoneSMSMemory = AT_NOTAVAILABLE;

			/* Check for Motorola style folders */
			if (strstr(msg.Buffer, "\"MT\"") != NULL && strstr(msg.Buffer, "\"OM\"") != NULL) {
				s->Phone.Data.Priv.ATGEN.PhoneSMSMemory = AT_AVAILABLE;
				s->Phone.Data.Priv.ATGEN.PhoneSaveSMS = AT_AVAILABLE;
				s->Phone.Data.Priv.ATGEN.MotorolaSMS = TRUE;
			}

		}
success:
		smprintf(s, "Available SMS memories received: read: ME : %s, SM : %s, save: ME : %s, SM = %s, Motorola = %s\n",
				s->Phone.Data.Priv.ATGEN.PhoneSMSMemory == AT_AVAILABLE ? "ok" : "N/A",
				s->Phone.Data.Priv.ATGEN.SIMSMSMemory == AT_AVAILABLE ? "ok" : "N/A",
				s->Phone.Data.Priv.ATGEN.PhoneSaveSMS == AT_AVAILABLE ? "ok" : "N/A",
				s->Phone.Data.Priv.ATGEN.SIMSaveSMS == AT_AVAILABLE ? "ok" : "N/A",
				s->Phone.Data.Priv.ATGEN.MotorolaSMS ? "yes" : "no"
				);
		return ERR_NONE;
	case AT_Reply_Error:
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	default:
		return ERR_UNKNOWNRESPONSE;
	}
}

GSM_Error ATGEN_GetSMSMemories(GSM_StateMachine *s)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	error = ATGEN_GetManufacturer(s);

	if (error != ERR_NONE) {
		return error;
	}

	smprintf(s, "Getting available SMS memories\n");
	ATGEN_WaitForAutoLen(s, "AT+CPMS=?\r", 0x00, 4, ID_GetSMSMemories);

	if (error!=ERR_NONE) {
		return error;
	}
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMS_SM)) {
		smprintf(s, "Forcing support for SM storage!\n");
		Priv->SIMSaveSMS = AT_AVAILABLE;
		Priv->SIMSMSMemory = AT_AVAILABLE;
	}
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMS_ME)) {
		smprintf(s, "Forcing support for ME storage!\n");
		Priv->PhoneSMSMemory = AT_AVAILABLE;
		Priv->PhoneSaveSMS = AT_AVAILABLE;
	}
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMS_NO_ME)) {
		smprintf(s, "Forcing to disable ME storage!\n");
		s->Phone.Data.Priv.ATGEN.PhoneSMSMemory = AT_NOTAVAILABLE;
		s->Phone.Data.Priv.ATGEN.PhoneSaveSMS = AT_NOTAVAILABLE;
	}
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMS_NO_SM)) {
		smprintf(s, "Forcing to disable SM storage!\n");
		s->Phone.Data.Priv.ATGEN.SIMSMSMemory = AT_NOTAVAILABLE;
		s->Phone.Data.Priv.ATGEN.SIMSaveSMS = AT_NOTAVAILABLE;
	}
	return ERR_NONE;
}

GSM_Error ATGEN_SetSMSMemory(GSM_StateMachine *s, gboolean SIM, gboolean for_write, gboolean outbox)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	/*
	 * Store message to memory.
	 */
	unsigned char cpmsCmdReq[] = "AT+CPMS=\"XX\",\"XX\"\r";
	size_t cpmsCmdReqLength = strlen(cpmsCmdReq);

	/* If phone encodes also values in command, we need normal charset */
	if (Priv->EncodedCommands) {
		error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);

		if (error != ERR_NONE) {
			return error;
		}
	}
	if ((SIM && Priv->SIMSMSMemory == 0) || (!SIM && Priv->PhoneSMSMemory == 0)) {
		/* We silently ignore error here, because when this fails, we can try to setmemory anyway */
		ATGEN_GetSMSMemories(s);
	}

	/* If phone can not save SMS, don't try to set memory for saving */
	if (for_write) {
		if (SIM && Priv->SIMSaveSMS == AT_NOTAVAILABLE) {
			smprintf(s, "Saving SMS not supported!\n");
			return ERR_NOTSUPPORTED;
		}
		if (!SIM && Priv->PhoneSaveSMS == AT_NOTAVAILABLE) {
			smprintf(s, "Saving SMS not supported!\n");
			return ERR_NOTSUPPORTED;
		}
	} else {
		/* No need to set memory for writing */
		cpmsCmdReq[12] = '\r';
		cpmsCmdReqLength = 13;
	}
	if (SIM) {
		if (Priv->SMSMemory == MEM_SM && (Priv->SMSMemoryWrite || !for_write)) {
			return ERR_NONE;
		}
		if (Priv->SIMSMSMemory == AT_NOTAVAILABLE) {
			return ERR_NOTSUPPORTED;
		}
		cpmsCmdReq[9] = 'S'; cpmsCmdReq[10] = 'M';
		cpmsCmdReq[14] = 'S'; cpmsCmdReq[15] = 'M';

		smprintf(s, "Setting SMS memory type to SM\n");
		ATGEN_WaitFor(s, cpmsCmdReq, cpmsCmdReqLength, 0x00, 20, ID_SetMemoryType);

		if (Priv->SIMSMSMemory == 0 && error != ERR_NONE) {
			Priv->SIMSMSMemory = AT_AVAILABLE;
		}
		if (error == ERR_NOTSUPPORTED) {
			smprintf(s, "Can't access SIM card?\n");
			return ERR_SECURITYERROR;
		}
		if (error != ERR_NONE) {
			return error;
		}
		Priv->SMSMemory = MEM_SM;
		Priv->SMSMemoryWrite = for_write;
	} else {
		if (Priv->SMSMemory == MEM_ME && (Priv->SMSMemoryWrite || !for_write)) {
			return ERR_NONE;
		}
		if (Priv->PhoneSMSMemory == AT_NOTAVAILABLE) {
			return ERR_NOTSUPPORTED;
		}
		if (Priv->MotorolaSMS) {
			cpmsCmdReq[9]  = 'M'; cpmsCmdReq[10] = 'T';

			if (outbox) {
				cpmsCmdReq[14] = 'O'; cpmsCmdReq[15] = 'M';
			} else {
				cpmsCmdReq[14] = 'I'; cpmsCmdReq[15] = 'M';
			}
		} else {
			cpmsCmdReq[9] = 'M'; cpmsCmdReq[10] = 'E';
			cpmsCmdReq[14] = 'M'; cpmsCmdReq[15] = 'E';
		}
		smprintf(s, "Setting SMS memory type to ME\n");
		ATGEN_WaitFor(s, cpmsCmdReq, cpmsCmdReqLength, 0x00, 20, ID_SetMemoryType);

		if (Priv->PhoneSMSMemory == 0 && error == ERR_NONE) {
			Priv->PhoneSMSMemory = AT_AVAILABLE;
		}
		if (error != ERR_NONE) {
			return error;
		}
		Priv->SMSMemory = MEM_ME;
		Priv->SMSMemoryWrite = for_write;
	}
	return error;
}

GSM_Error ATGEN_GetSMSMode(GSM_StateMachine *s)
{
  	GSM_Error error=ERR_NONE;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	if (Priv->SMSMode != 0) {
		return ERR_NONE;
	}

	/* Prefer PDU mode for most phones */
	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_USE_SMSTEXTMODE)) {
		smprintf(s, "Trying SMS PDU mode\n");
		ATGEN_WaitForAutoLen(s, "AT+CMGF=0\r", 0x00, 9, ID_GetSMSMode);

		if (error==ERR_NONE) {
			Priv->SMSMode = SMS_AT_PDU;
			return ERR_NONE;
		}
	}
	smprintf(s, "Trying SMS text mode\n");
	ATGEN_WaitForAutoLen(s, "AT+CMGF=1\r", 0x00, 9, ID_GetSMSMode);

	if (error == ERR_NONE) {
		Priv->SMSMode = SMS_AT_TXT;
		smprintf(s, "Enabling displaying all parameters in text mode\n");
		ATGEN_WaitForAutoLen(s, "AT+CSDH=1\r", 0x00, 3, ID_GetSMSMode);

		if (error == ERR_NONE) {
			Priv->SMSTextDetails = TRUE;
		} else {
			error = ERR_NONE;
		}
	}
	return error;
}

GSM_Error ATGEN_GetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char *folderid, int *location, gboolean for_write)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	int ifolderid = 0, maxfolder = 0;

	if (Priv->PhoneSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, FALSE, for_write, (sms->Folder % 2) == 0);

		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			return error;
		}
	}
	if (Priv->SIMSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, TRUE, for_write, (sms->Folder % 2) == 0);

		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			return error;
		}
	}

	if (Priv->SIMSMSMemory != AT_AVAILABLE && Priv->PhoneSMSMemory != AT_AVAILABLE) {
		smprintf(s, "No SMS memory at all!\n");
		return ERR_NOTSUPPORTED;
	}
	if (Priv->SIMSMSMemory == AT_AVAILABLE && Priv->PhoneSMSMemory == AT_AVAILABLE) {
		/* Both available */
		maxfolder = 4;
	} else {
		/* One available */
		maxfolder = 2;
	}

	/* simulate flat SMS memory */
	if (sms->Folder == 0x00) {
		ifolderid = sms->Location / GSM_PHONE_MAXSMSINFOLDER;

		if (ifolderid + 1 > maxfolder) {
			smprintf(s, "Too high location for flat folder: %d (folder=%d, maxfolder=%d)\n",
					sms->Location,
					ifolderid + 1,
					maxfolder);
			return ERR_NOTSUPPORTED;
		}
		*folderid = ifolderid + 1;
		*location = sms->Location - ifolderid * GSM_PHONE_MAXSMSINFOLDER;
	} else {
		if (sms->Folder > 2 * maxfolder) {
			smprintf(s, "Too high folder: folder=%d, maxfolder=%d\n",
					sms->Folder,
					maxfolder);
			return ERR_NOTSUPPORTED;
		}
		*folderid = sms->Folder <= 2 ? 1 : 2;
		*location = sms->Location;
	}

	/* Some phones start locations from 0, handle them here */
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMS_LOCATION_0)) {
		(*location)--;
	}
	smprintf(s, "SMS folder %i & location %i -> ATGEN folder %i & location %i\n",
			sms->Folder,sms->Location,*folderid,*location);

	/* Set the needed memory type */
	if (Priv->SIMSMSMemory == AT_AVAILABLE &&
			*folderid == 1) {
		sms->Memory = MEM_SM;
		return ATGEN_SetSMSMemory(s, TRUE, for_write, (sms->Folder % 2) == 0);
	} else {
		sms->Memory = MEM_ME;
		return ATGEN_SetSMSMemory(s, FALSE, for_write, (sms->Folder % 2) == 0);
	}
}

/**
 * Converts location from AT internal to Gammu API. We need to ensure
 * locations in API are sequential over all folders.
 */
void ATGEN_SetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char folderid, int location)
{
	sms->Folder = 0; /* Flat memory */
	sms->Location = (folderid - 1) * GSM_PHONE_MAXSMSINFOLDER + location;

	/* Some phones start locations from 0, handle them here */
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMS_LOCATION_0)) {
		sms->Location++;
	}
	smprintf(s, "ATGEN folder %i & location %i -> SMS folder %i & location %i\n",
			folderid, location, sms->Folder, sms->Location);
}

GSM_Error ATGEN_DecodePDUMessage(GSM_StateMachine *s, const char *PDU, const int state)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_SMSMessage *sms = &s->Phone.Data.GetSMSMessage->SMS[0];
	unsigned char buffer[300] = {'\0'};
	size_t parse_len = 0, length = 0;

	length = strlen(PDU);

	if (!DecodeHexBin (
				buffer,
				PDU,
				length
				)) {
		smprintf(s, "Failed to decode hex string!\n");
		return ERR_CORRUPTED;
	}
	switch (state) {
		case 0:
			sms->State = SMS_UnRead;
			break;
		case 1:
			sms->State = SMS_Read;
			break;
		case 2:
			sms->State = SMS_UnSent;
			break;
		default:
			sms->State = SMS_Sent;
			break;
	}
	length /= 2; /* We decoded hex -> binary */
	error = GSM_DecodePDUFrame(&(s->di), sms,  buffer, length, &parse_len, TRUE);

	if (error != ERR_NONE) {
		return error;
	}
	if (parse_len != length) {
		smprintf(s, "Did not parse all PDU data (%u, %u)!\n", (unsigned int)parse_len, (unsigned int)length);

		if (buffer[parse_len] == 0xff) {
			smprintf(s, "Assuming broken phone which pads SMS data with FF\n");
		} else {
			return ERR_UNKNOWN;
		}
	}

	switch (sms->PDU) {
		case SMS_Deliver:
			/* @bug Broken when MEM_SM is not available */
			if (Priv->SMSMemory == MEM_SM) {
				sms->Folder = 1; /*INBOX SIM*/
			} else {
				sms->Folder = 3; /*INBOX ME*/
			}
			sms->InboxFolder = TRUE;
			break;
		case SMS_Submit:
			/* @bug Broken when MEM_SM is not available */
			if (Priv->SMSMemory == MEM_SM) {
				sms->Folder = 2; /*OUTBOX SIM*/
				smprintf(s, "Outbox SIM\n");
			} else {
				sms->Folder = 4; /*OUTBOX ME*/
			}
			sms->InboxFolder = FALSE;
			break;
		case SMS_Status_Report:
			sms->PDU 	 = SMS_Status_Report;
			sms->Folder 	 = 1;	/*INBOX SIM*/
			sms->InboxFolder = TRUE;
			break;
	}
	return ERR_NONE;
}

GSM_Error ATGEN_ReplyGetSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Error error = ERR_UNKNOWN;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_SMSMessage *sms = &s->Phone.Data.GetSMSMessage->SMS[0];
	unsigned char buffer[300] = {'\0'}, firstbyte = 0, TPDCS = 0, TPUDL = 0, TPStatus = 0;
	const char *line;
	size_t length = 0;
	int current = 0, i = 0;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		if (Priv->Lines.numbers[4] == 0x00) return ERR_EMPTY;
		s->Phone.Data.GetSMSMessage->Number 	 	= 1;
		s->Phone.Data.GetSMSMessage->SMS[0].Name[0] 	= 0;
		s->Phone.Data.GetSMSMessage->SMS[0].Name[1]	= 0;

		switch (Priv->SMSMode) {
		case SMS_AT_PDU:
			CopyLineString(buffer, msg.Buffer, &Priv->Lines, 2);

			/* Siemens MC35 (only ?) */
			if (strstr(msg.Buffer,"+CMGR: 0,,0")!=NULL) {
				return ERR_EMPTY;
			}
			/* @bug Hardcoded position is wrong! */
			return ATGEN_DecodePDUMessage(s,
					GetLineString(msg.Buffer,&Priv->Lines,3),
					atoi(buffer + 7));
		case SMS_AT_TXT:
			GSM_SetDefaultReceivedSMSData(sms);
			error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, &Priv->Lines, 2),
					"+CMGR: @r, @p @d",
					buffer, sizeof(buffer), sms->Number, sizeof(sms->Number),
					&sms->DateTime, sizeof(sms->DateTime));

			if (error != ERR_NONE) {
				return error;
			}
			if (!strcmp(buffer,"0") || !strcmp(buffer,"REC UNREAD")) {
				smprintf(s, "SMS type - deliver\n");
				sms->State 	 = SMS_UnRead;
				sms->PDU 	 = SMS_Deliver;

				if (Priv->SMSMemory == MEM_SM) {
					sms->Folder = 1; /*INBOX SIM*/
				} else {
					sms->Folder = 3; /*INBOX ME*/
				}
				sms->InboxFolder = TRUE;
			} else if (!strcmp(buffer,"1") || !strcmp(buffer,"REC READ")) {
				smprintf(s, "SMS type - deliver\n");
				sms->State 	 = SMS_Read;
				sms->PDU 	 = SMS_Deliver;

				if (Priv->SMSMemory == MEM_SM) {
					sms->Folder = 1; /*INBOX SIM*/
				} else {
					sms->Folder = 3; /*INBOX ME*/
				}
				sms->InboxFolder = TRUE;
			} else if (!strcmp(buffer,"2") || !strcmp(buffer,"STO UNSENT")) {
				smprintf(s, "SMS type - submit\n");
				sms->State 	 = SMS_UnSent;
				sms->PDU 	 = SMS_Submit;

				if (Priv->SMSMemory == MEM_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/
				} else {
					sms->Folder = 4; /*OUTBOX ME*/
				}
				sms->InboxFolder = FALSE;
			} else if (!strcmp(buffer,"3") || !strcmp(buffer,"STO SENT")) {
				smprintf(s, "SMS type - submit\n");
				sms->State 	 = SMS_Sent;
				sms->PDU 	 = SMS_Submit;

				if (Priv->SMSMemory == MEM_SM) {
					sms->Folder = 2; /*OUTBOX SIM*/
				} else {
					sms->Folder = 4; /*OUTBOX ME*/
				}
				sms->InboxFolder = FALSE;
			}
			if (Priv->SMSTextDetails == FALSE) {
				sms->Class = 1;
				sms->Coding = SMS_Coding_Default_No_Compression;
				sms->UDH.Type	= UDH_NoUDH;
				sms->Length	= 0;
				sms->SMSC.Number[0]=0;
				sms->SMSC.Number[1]=0;
				sms->ReplyViaSameSMSC=FALSE;

				/* Go trough all lines till OK */
				for (i = 3; strcmp(line = GetLineString(msg.Buffer, &Priv->Lines, i), "OK") != 0; i++) {
					if (i > 3) {
						/* Include new line */
						sms->Text[(2 * sms->Length) + 0] = 0;
						sms->Text[(2 * sms->Length) + 1] = '\n';
						sms->Text[(2 * sms->Length) + 2] = 0;
						sms->Text[(2 * sms->Length) + 3] = 0;
						sms->Length++;
					}
					length = GetLineLength(msg.Buffer, &Priv->Lines, i);
					error = ATGEN_DecodeText(s, line, length,
						sms->Text + (2 * sms->Length),
						sizeof(sms->Text) - (2 * sms->Length),
						TRUE, FALSE);
					if (error != ERR_NONE)
						return error;
					sms->Length += length;
				}
				return ERR_NONE;
			}

			/*
			 * This is just a hack until proper parsing of text mode is done.
			 * It uses old style of manual parsing, to skip entries parsed above.
			 */
			current = 0;

			while (msg.Buffer[current]!='"') {
				current++;
			}
			current += ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
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
				error = ATGEN_DecodeDateTime(s, &sms->DateTime, buffer);

				if (error!=ERR_NONE) {
					return error;
				}
				/* Date of SMSC response */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				i = strlen(buffer);
				buffer[i] = ',';
				i++;
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer+i);
				smprintf(s, "\"%s\"\n",buffer);
				error = ATGEN_DecodeDateTime(s, &sms->SMSCTime, buffer);

				if (error!=ERR_NONE) {
					return error;
				}
				/* TPStatus */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				TPStatus=atoi(buffer);
				buffer[PHONE_SMSStatusReport.TPStatus] = TPStatus;
				error=GSM_DecodeSMSFrameStatusReportData(&(s->di), sms, buffer, PHONE_SMSStatusReport);

				if (error!=ERR_NONE) {
					return error;
				}
				/* NO SMSC number */
				sms->SMSC.Number[0]=0;
				sms->SMSC.Number[1]=0;
				sms->PDU = SMS_Status_Report;
				sms->ReplyViaSameSMSC=FALSE;
			} else {
				/* Sender number */
				/* FIXME: support for all formats */
				EncodeUnicode(sms->Number,buffer+1,strlen(buffer)-2);

				/* Sender number in alphanumeric format ? */
				current += ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);

				if (strlen(buffer)!=0) {
					EncodeUnicode(sms->Number,buffer+1,strlen(buffer)-2);
				}
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
						error = ATGEN_DecodeDateTime(s, &sms->DateTime, buffer);
						if (error!=ERR_NONE) return error;
					else {
						/* FIXME: What is the proper undefined GSM_DateTime ? */
						memset(&sms->DateTime, 0, sizeof(sms->DateTime));
					}
					error = ATGEN_DecodeDateTime(s, &sms->DateTime, buffer);

					if (error!=ERR_NONE) {
						return error;
					}
				}
				/* Sender number format */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);

				/* First byte */
				current+=ATGEN_ExtractOneParameter(msg.Buffer+current, buffer);
				firstbyte=atoi(buffer);
				sms->ReplyViaSameSMSC=FALSE;

				/* GSM 03.40 section 9.2.3.17 (TP-Reply-Path) */
				if ((firstbyte & 128)==128) {
					sms->ReplyViaSameSMSC=TRUE;
				}
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
				sms->Coding = GSM_GetMessageCoding(&(s->di), TPDCS);
				sms->Class = -1;

				if ((TPDCS & 0xD0) == 0x10) {
					/* bits 7..4 set to 00x1 */
					if ((TPDCS & 0xC) == 0xC) {
						smprintf(s, "WARNING: reserved alphabet value in TPDCS\n");
					} else {
						sms->Class = (TPDCS & 3);
					}
				} else if ((TPDCS & 0xF0) == 0xF0) {
					/* bits 7..4 set to 1111 */
					if ((TPDCS & 8) == 8) {
						smprintf(s, "WARNING: set reserved bit 3 in TPDCS\n");
					} else {
						sms->Class = (TPDCS & 3);
					}
				}
				smprintf(s, "SMS class: %i\n",sms->Class);

				switch (sms->Coding) {
				case SMS_Coding_Default_No_Compression:
					/* GSM 03.40 section 9.2.3.23 (TP-User-Data-Header-Indicator) */
					/* If not SMS with UDH, it's coded normal */
					/* If UDH available, treat it as Unicode or 8 bit */
					if ((firstbyte & 0x40)!=0x40) {
						sms->UDH.Type	= UDH_NoUDH;
						sms->Length	= TPUDL;
						EncodeUnicode(sms->Text,msg.Buffer+Priv->Lines.numbers[2*2],TPUDL);
						break;
					}
				case SMS_Coding_Unicode_No_Compression:
				case SMS_Coding_8bit:
					DecodeHexBin(buffer+PHONE_SMSDeliver.Text, msg.Buffer+current, TPUDL*2);
					buffer[PHONE_SMSDeliver.firstbyte] 	= firstbyte;
					buffer[PHONE_SMSDeliver.TPDCS] 		= TPDCS;
					buffer[PHONE_SMSDeliver.TPUDL] 		= TPUDL;
					return GSM_DecodeSMSFrameText(&(s->di), sms, buffer, PHONE_SMSDeliver);
				default:
					break;
				}
			}
			return ERR_NONE;
		default:
			break;
		}
		break;
	case AT_Reply_CMSError:
		if (Priv->ErrorCode == 320 || Priv->ErrorCode == 500) {
			return ERR_EMPTY;
		} else {
			return ATGEN_HandleCMSError(s);
		}
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	case AT_Reply_Error:
		/* A2D returns Error with empty location */
		return ERR_EMPTY;
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	unsigned char req[20] = {'\0'}, folderid = 0;
	int location = 0, getfolder = 0, add = 0, length = 0;

	/* Set mode of SMS */
	error = ATGEN_GetSMSMode(s);

	if (error != ERR_NONE) {
		return error;
	}
	/* Clear SMS structure of any possible junk */
	GSM_SetDefaultReceivedSMSData(&sms->SMS[0]);
	error = ATGEN_GetSMSLocation(s, &sms->SMS[0], &folderid, &location, FALSE);

	if (error != ERR_NONE) {
		return error;
	}
	if (Priv->SMSMemory == MEM_ME && GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMSME900)) {
		add = 899;
	}
	length=sprintf(req, "AT+CMGR=%i\r", location + add);

	/* There is possibility that date will be encoded in text mode */
	if (Priv->SMSMode == SMS_AT_TXT) {
		error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);

		if (error != ERR_NONE) {
			return error;
		}
	}
	error = ATGEN_GetManufacturer(s);

	if (error != ERR_NONE) {
		return error;
	}
	s->Phone.Data.GetSMSMessage = sms;
	smprintf(s, "Getting SMS\n");
	ATGEN_WaitFor(s, req, length, 0x00, 20, ID_GetSMSMessage);

	if (error == ERR_NONE || error == ERR_CORRUPTED) {
		getfolder = sms->SMS[0].Folder;
/* 		if (getfolder != 0 && getfolder != sms->SMS[0].Folder) return ERR_EMPTY; */
		ATGEN_SetSMSLocation(s, &sms->SMS[0], folderid, location);
		sms->SMS[0].Folder = getfolder;
		sms->SMS[0].Memory = MEM_SM;
		if (getfolder > 2) sms->SMS[0].Memory = MEM_ME;
	}
	return error;
}

GSM_Error ATGEN_ReplyGetMessageList(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_SMSMessage sms;
	int line = 1, cur = 0, allocsize = 0;
	char *tmp = NULL;
	const char *str;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		break;
	case AT_Reply_Error:
		return ERR_NOTSUPPORTED;
 	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	default:
		return ERR_UNKNOWNRESPONSE;
	}
	smprintf(s, "SMS listing received\n");
	Priv->SMSCount = 0;
	Priv->SMSCache = NULL;

	/* Walk through lines with +CMGL: */
	/* First line is our command so we can skip it */
	for (line = 2; strcmp("OK", str = GetLineString(msg.Buffer, &Priv->Lines, line)) != 0; line++) {
		/*
		 * Find +CMGL, it should be on beginning, but it does not have to (see
		 * corruption mentioned at the end of loop.
		 */
		str = strstr(str, "+CMGL:");

		if (str == NULL) {
			/*
			 * Sometimes an SMS message will contain a line break. In SMS text
                         * mode we skip to the next line and try again to find +CMGL.
			 * FIXME: Can we do the same for SMS PDU mode?
			 */
			if (Priv->SMSMode == SMS_AT_PDU) {
				smprintf(s, "Can not find +CMGL:!\n");
				return ERR_UNKNOWN;
			}
			continue;
		}

		/* Parse reply */
		error = ATGEN_ParseReply(s, str, "+CMGL: @i, @0", &cur);

		if (error != ERR_NONE) {
			return error;
		}
		Priv->SMSCount++;

		/* Reallocate buffer if needed */
		if (allocsize <= Priv->SMSCount) {
			allocsize += 20;
			Priv->SMSCache = (GSM_AT_SMS_Cache *)realloc(Priv->SMSCache, allocsize * sizeof(GSM_AT_SMS_Cache));

			if (Priv->SMSCache == NULL) {
				return ERR_MOREMEMORY;
			}
		}

		/* Should we use index instead of location? Samsung P900 needs this hack. */
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_BROKEN_CMGL)) {
			ATGEN_SetSMSLocation(s, &sms, Priv->SMSReadFolder, Priv->SMSCount);
		} else {
			ATGEN_SetSMSLocation(s, &sms, Priv->SMSReadFolder, cur);
		}
		Priv->SMSCache[Priv->SMSCount - 1].Location = sms.Location;
		Priv->SMSCache[Priv->SMSCount - 1].State = -1;

		/* Go to PDU/Text data */
		line++;

		/* Fill in cache of PDU data */
		if (Priv->SMSMode == SMS_AT_PDU) {
			error = ATGEN_ParseReply(s, str, "+CMGL: @i, @i, @0",
					&cur,
					&Priv->SMSCache[Priv->SMSCount - 1].State);

			if (error != ERR_NONE) {
				smprintf(s, "Failed to parse reply, not using cache!\n");
				Priv->SMSCache[Priv->SMSCount - 1].State = -1;
			}
			/* Get next line (PDU data) */
			str = GetLineString(msg.Buffer, &Priv->Lines, line);

			if (strlen(str) >= GSM_AT_MAXPDULEN) {
				smprintf(s, "PDU (%s) too long for cache, skipping!\n", str);
				Priv->SMSCache[Priv->SMSCount - 1].State = -1;
			} else {
				strcpy(Priv->SMSCache[Priv->SMSCount - 1].PDU, str);

				/* Some phones corrupt output and do not put new line before +CMGL occassionally */
				tmp = strstr(Priv->SMSCache[Priv->SMSCount - 1].PDU, "+CMGL:");

				if (tmp != NULL) {
					smprintf(s, "WARNING: Line should contain PDU data, but contains +CMGL, stripping it!\n");
					*tmp = 0;

					/* Go line back, because we have to process this line again */
					line--;
				}
			}
		}

	}
	smprintf(s, "Read %d SMS locations\n", Priv->SMSCount);
	return ERR_NONE;
}

GSM_Error ATGEN_GetSMSList(GSM_StateMachine *s, gboolean first)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	int used = 0;

	/* Set mode of SMS */
	error = ATGEN_GetSMSMode(s);

	if (error != ERR_NONE) {
		return error;
	}

	/* Get number of messages */
	error = ATGEN_GetSMSStatus(s,&Priv->LastSMSStatus);

	if (error!=ERR_NONE) {
		return error;
	}
	if (first) {
		Priv->SMSReadFolder = 1;

		if (Priv->SIMSMSMemory == AT_AVAILABLE) {
			error = ATGEN_SetSMSMemory(s, TRUE, FALSE, FALSE);

			if (error != ERR_NONE) {
				return error;
			}
			used = Priv->LastSMSStatus.SIMUsed;
		} else if (Priv->PhoneSMSMemory == AT_AVAILABLE) {
			error = ATGEN_SetSMSMemory(s, FALSE, FALSE, FALSE);

			if (error != ERR_NONE) {
				return error;
			}
			used = Priv->LastSMSStatus.PhoneUsed;
		} else {
			return ERR_NOTSUPPORTED;
		}
	} else {
		Priv->SMSReadFolder = 2;

		if (Priv->PhoneSMSMemory == AT_AVAILABLE) {
			error = ATGEN_SetSMSMemory(s, FALSE, FALSE, FALSE);

			if (error != ERR_NONE) {
				return error;
			}
			used = Priv->LastSMSStatus.PhoneUsed;
		} else {
			return ERR_NOTSUPPORTED;
		}
	}
	Priv->LastSMSRead = 0;
	Priv->SMSCount = 0;

	if (Priv->SMSCache != NULL) {
		free(Priv->SMSCache);
		Priv->SMSCache = NULL;
	}
	smprintf(s, "Getting SMS locations\n");

	if (Priv->SMSMode == SMS_AT_TXT) {
		ATGEN_WaitForAutoLen(s, "AT+CMGL=\"ALL\"\r", 0x00, 500, ID_GetSMSMessage);
	} else {
		ATGEN_WaitForAutoLen(s, "AT+CMGL=4\r", 0x00, 20, ID_GetSMSMessage);
	}
	if (error == ERR_NOTSUPPORTED) {
		ATGEN_WaitForAutoLen(s, "AT+CMGL\r", 0x00, 5, ID_GetSMSMessage);
	}
	/*
	 * We did not read anything, but it is correct, indicate that
	 * cache should be used (even if it is empty).
	 */
	if (error == ERR_NONE && Priv->SMSCache == NULL) {
		Priv->SMSCache = (GSM_AT_SMS_Cache *)realloc(Priv->SMSCache, sizeof(GSM_AT_SMS_Cache));
	}
	if (used != Priv->SMSCount && error == ERR_NONE) {
		smprintf(s, "WARNING: Used messages according to CPMS %d, but CMGL returned %d. Expect problems!\n", used, Priv->SMSCount);
		smprintf(s, "HINT: Your might want to use F_USE_SMSTEXTMODE flag\n");
	}
	return error;
}

GSM_Error ATGEN_GetNextSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, gboolean start)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	int usedsms = 0, i = 0, found = -1, tmpfound = -1;

	if (Priv->PhoneSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, FALSE, FALSE, FALSE);

		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			return error;
		}
	}
	if (Priv->SIMSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, TRUE, FALSE, FALSE);

		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			return error;
		}
	}
	if (Priv->SIMSMSMemory == AT_NOTAVAILABLE && Priv->PhoneSMSMemory == AT_NOTAVAILABLE) return ERR_NOTSUPPORTED;

	/* On start we need to init everything */
	if (start) {
		/* Start from beginning */
		sms->SMS[0].Location = 0;

		/* Get list of messages */
		error = ATGEN_GetSMSList(s, TRUE);
	}

	/* Use listed locations if we have them */
	if (Priv->SMSCache != NULL) {
		if (start) {
			found = 0;
		} else {
			for (i = 0; i < Priv->SMSCount; i++) {
				if (Priv->SMSCache[i].Location == sms->SMS[0].Location) {
					found = i + 1;
					break;
				}
				if ((Priv->SMSCache[i].Location < sms->SMS[0].Location)
					&& ((tmpfound == -1) ||
						(sms->SMS[0].Location - Priv->SMSCache[i].Location <
						sms->SMS[0].Location - Priv->SMSCache[tmpfound - 1].Location))
					) {
					tmpfound = i + 1;
				}
			}
		}

		if (found == -1) {
			smprintf(s, "Invalid location passed to %s!\n", __FUNCTION__);

			if (tmpfound == -1) {
				return ERR_INVALIDLOCATION;
			} else {
				smprintf(s, "Attempting to skip to next location!\n");
				found = tmpfound;
			}
		}
		smprintf(s, "Cache status: Found: %d, count: %d\n", found, Priv->SMSCount);

		if (found >= Priv->SMSCount) {
			/* Did we already read second folder? */
			if (Priv->SMSReadFolder == 2) {
				return ERR_EMPTY;
			}

			/* Get list of messages */
			error = ATGEN_GetSMSList(s, FALSE);

			/* Not supported folder? We're done then. */
			if (error == ERR_NOTSUPPORTED) {
				return ERR_EMPTY;
			}
			if (error != ERR_NONE) {
				return error;
			}

			/* Did we read anything? */
			if (Priv->SMSCache != NULL && Priv->SMSCount == 0) {
				return ERR_EMPTY;
			}

			/* Start again */
			found = 0;
		}

		/* We might get no messages in listing above */
		if (Priv->SMSCache != NULL) {
			sms->SMS[0].Folder = 0;
			sms->Number = 1;
			sms->SMS[0].Memory = Priv->SMSMemory;
			sms->SMS[0].Location = Priv->SMSCache[found].Location;

			if (Priv->SMSCache[found].State != -1) {
				/* Get message from cache */
				GSM_SetDefaultReceivedSMSData(&sms->SMS[0]);
				s->Phone.Data.GetSMSMessage = sms;
				smprintf(s, "Getting message from cache\n");
				smprintf(s, "%s\n", Priv->SMSCache[found].PDU);
				error = ATGEN_DecodePDUMessage(s,
						Priv->SMSCache[found].PDU,
						Priv->SMSCache[found].State);

				/* Is the entry corrupted? */
				if (error != ERR_CORRUPTED) {
					return error;
				}
				/* Mark it as invalid */
				Priv->SMSCache[found].State = -1;
				/* And fall back to normal reading */
			}

			/* Finally read the message */
			smprintf(s, "Reading next message on location %d\n", sms->SMS[0].Location);
			return ATGEN_GetSMS(s, sms);
		}
	}

	/* Use brute force if listing does not work */
	while (TRUE) {
		sms->SMS[0].Location++;

		if (sms->SMS[0].Location < GSM_PHONE_MAXSMSINFOLDER) {
			if (Priv->SIMSMSMemory == AT_AVAILABLE) {
				usedsms = Priv->LastSMSStatus.SIMUsed;
			} else {
				usedsms = Priv->LastSMSStatus.PhoneUsed;
			}

			if (Priv->LastSMSRead >= usedsms) {
				if (Priv->PhoneSMSMemory == AT_NOTAVAILABLE || Priv->LastSMSStatus.PhoneUsed == 0) {
					smprintf(s, "No more messages to read\n");
					return ERR_EMPTY;
				}
				Priv->LastSMSRead	= 0;

				/* Start on next folder */
				sms->SMS[0].Location 	= GSM_PHONE_MAXSMSINFOLDER + 1;
			}
		} else {
			if (Priv->PhoneSMSMemory == AT_NOTAVAILABLE) return ERR_EMPTY;
			if (Priv->LastSMSRead >= Priv->LastSMSStatus.PhoneUsed) return ERR_EMPTY;
		}
		sms->SMS[0].Folder = 0;
		error = ATGEN_GetSMS(s, sms);

		if (error == ERR_NONE) {
			Priv->LastSMSRead++;
			break;
		}
		if (error != ERR_EMPTY && error != ERR_INVALIDLOCATION) return error;
	}
	return error;
}

GSM_Error ATGEN_ReplyGetSMSStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_SMSMemoryStatus *SMSStatus = s->Phone.Data.SMSStatus;
	unsigned char buffer[50] = {'\0'};
	int used = 0, size = 0;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "SMS status received\n");

		/* Check for +CPMS: 0,30,0,30,8,330, this is according to ETSI */
		error = ATGEN_ParseReply(s,
				GetLineString(msg.Buffer, &Priv->Lines, 2),
				"+CPMS: @i, @i, @0",
				&used, &size);
		if (error != ERR_NONE) {
			/*
			 * Samsung formats this different way, sample response:
			 * 1 "AT+CPMS="SM","SM""
			 * 2 "+CPMS:"SM",3,30,"SM",3,30,"SM",3,30"
			 * 3 "OK"
			 */
			error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, &Priv->Lines, 2),
					"+CPMS: @s, @i, @i, @0",
					&buffer, sizeof(buffer), &used, &size);
		}
		if (error != ERR_NONE) {
			/* For phones with single memory */
			error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, &Priv->Lines, 2),
					"+CPMS: @i, @i",
					&used, &size);
		}
		if (error == ERR_NONE) {
			smprintf(s, "Used : %i\n", used);
			smprintf(s, "Size : %i\n", size);
			if ((strstr(msg.Buffer, "CPMS=\"ME") != NULL) ||
				(Priv->MotorolaSMS && strstr(msg.Buffer, "CPMS=\"MT") != NULL)) {
				SMSStatus->PhoneUsed = used;
				SMSStatus->PhoneSize = size;
			} else {
				SMSStatus->SIMUsed = used;
				SMSStatus->SIMSize = size;
			}
		}
		return error;
	case AT_Reply_Error:
		if (strstr(msg.Buffer,"SM")!=NULL) {
			smprintf(s, "Can't access SIM card\n");
			return ERR_SECURITYERROR;
		}
		return ERR_NOTSUPPORTED;
 	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;

	/* No templates at all */
	status->TemplatesUsed = 0;
	status->SIMUsed	= 0;
	status->SIMUnRead = 0;
	status->SIMSize	= 0;
	s->Phone.Data.SMSStatus=status;

	if ((Priv->SIMSMSMemory == 0) || (Priv->PhoneSMSMemory == 0)) {
		/* We silently ignore error here, because when this fails, we can try to setmemory anyway */
		ATGEN_GetSMSMemories(s);
	}
	if (Priv->PhoneSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, FALSE, FALSE, FALSE);

		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			return error;
		}
	}
	if (Priv->SIMSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, TRUE, FALSE, FALSE);

		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			return error;
		}
	}
	if (Priv->SIMSMSMemory == AT_AVAILABLE) {
		smprintf(s, "Getting SIM SMS status\n");

		if (Priv->SIMSaveSMS == AT_AVAILABLE) {
			ATGEN_WaitForAutoLen(s, "AT+CPMS=\"SM\",\"SM\"\r", 0x00, 20, ID_GetSMSStatus);
			Priv->SMSMemoryWrite = TRUE;
		} else {
			ATGEN_WaitForAutoLen(s, "AT+CPMS=\"SM\"\r", 0x00, 20, ID_GetSMSStatus);
			Priv->SMSMemoryWrite = FALSE;
		}
		if (error!=ERR_NONE) {
			return error;
		}
		Priv->SMSMemory = MEM_SM;
	}
	status->PhoneUsed = 0;
	status->PhoneUnRead = 0;
	status->PhoneSize = 0;

	if (Priv->PhoneSMSMemory == AT_AVAILABLE) {
		smprintf(s, "Getting phone SMS status\n");

		if (Priv->PhoneSaveSMS == AT_AVAILABLE) {
			if (Priv->MotorolaSMS) {
				ATGEN_WaitForAutoLen(s, "AT+CPMS=\"MT\"\r", 0x00, 20, ID_GetSMSStatus);
				Priv->SMSMemoryWrite = FALSE;
			} else {
				ATGEN_WaitForAutoLen(s, "AT+CPMS=\"ME\",\"ME\"\r", 0x00, 20, ID_GetSMSStatus);
				Priv->SMSMemoryWrite = TRUE;
			}
		} else {
			ATGEN_WaitForAutoLen(s, "AT+CPMS=\"ME\"\r", 0x00, 20, ID_GetSMSStatus);
			Priv->SMSMemoryWrite = FALSE;
		}
		if (error!=ERR_NONE) {
			return error;
		}
		Priv->SMSMemory = MEM_ME;
	}
	return ERR_NONE;
}

GSM_Error ATGEN_ReplyAddSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	size_t i = 0;
	int folder = 0;

	if (s->Protocol.Data.AT.EditMode) {
		if (s->Phone.Data.Priv.ATGEN.ReplyState != AT_Reply_SMSEdit) {
			return ATGEN_HandleCMSError(s);
		}
		s->Protocol.Data.AT.EditMode = FALSE;
		return ERR_NONE;
	}
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "SMS saved OK\n");

		/* Number of lines */
		i = 0;

		while (Priv->Lines.numbers[i*2+1] != 0) {
			i++;
		}
		error = ATGEN_ParseReply(s,
				GetLineString(msg.Buffer, &Priv->Lines, i - 1),
				"+CMGW: @i",
				&s->Phone.Data.SaveSMSMessage->Location);

		if (error != ERR_NONE) {
			return error;
		}
		smprintf(s, "Saved at AT location %i\n",
				s->Phone.Data.SaveSMSMessage->Location);
		/* Adjust location */
		folder = s->Phone.Data.SaveSMSMessage->Folder;
		ATGEN_SetSMSLocation(s, s->Phone.Data.SaveSMSMessage,
				(folder / 2), /* We care only about SIM/Phone */
				s->Phone.Data.SaveSMSMessage->Location);
		s->Phone.Data.SaveSMSMessage->Folder = folder;
		return ERR_NONE;
	case AT_Reply_Error:
		smprintf(s, "Error\n");
		return ERR_NOTSUPPORTED;
	case AT_Reply_CMSError:
		/* This error occurs in case that phone couldn't save SMS */
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_MakeSMSFrame(GSM_StateMachine *s, GSM_SMSMessage *message, unsigned char *hexreq, int *current, int *length2)
{
	GSM_Error 		error;
	GSM_Phone_ATGENData 	*Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_SMSC	 	SMSC;
	unsigned char		req[1000] = {'\0'}, buffer[1000] = {'\0'};
	int			i = 0, length = 0;

	/* Set mode of SMS */
	error = ATGEN_GetSMSMode(s);

	if (error != ERR_NONE) {
		return error;
	}
	length = 0;
	*current = 0;

	switch (Priv->SMSMode) {
	case SMS_AT_PDU:
		if (message->PDU == SMS_Deliver) {
 			smprintf(s, "SMS Deliver\n");
			error=PHONE_EncodeSMSFrame(s,message,buffer,PHONE_SMSDeliver,&length,TRUE);

			if (error != ERR_NONE) {
				return error;
			}
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

			for(i=0;i<7;i++) {
				req[(*current)++]=buffer[PHONE_SMSDeliver.DateTime+i];
			}
			req[(*current)++]=buffer[PHONE_SMSDeliver.TPUDL];

			for(i=0;i<length;i++) {
				req[(*current)++]=buffer[PHONE_SMSDeliver.Text+i];
			}
			EncodeHexBin(hexreq, req, *current);
			*length2 = *current * 2;
			*current = *current - (req[PHONE_SMSDeliver.SMSCNumber]+1);
		} else {
			smprintf(s, "SMS Submit\n");
			error=PHONE_EncodeSMSFrame(s,message,buffer,PHONE_SMSSubmit,&length,TRUE);

			if (error != ERR_NONE) {
				return error;
			}
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

			for(i=0;i<length;i++) {
				req[(*current)++]=buffer[PHONE_SMSSubmit.Text+i];
			}
			req[(*current)+1]='\0';
			EncodeHexBin(hexreq, req, *current);
			*length2 = *current * 2;
			*current = *current - (req[PHONE_SMSSubmit.SMSCNumber]+1);
		}
		break;
	case SMS_AT_TXT:
		error = ATGEN_GetManufacturer(s);

		if (error != ERR_NONE) {
			return error;
		}

		if (Priv->Manufacturer != AT_Nokia) {
			if (message->Coding != SMS_Coding_Default_No_Compression) {
				return ERR_NOTSUPPORTED;
			}
		}
		error=PHONE_EncodeSMSFrame(s,message,req,PHONE_SMSDeliver,&i,TRUE);

		if (error != ERR_NONE) {
			return error;
		}
		CopyUnicodeString(SMSC.Number,message->SMSC.Number);
		SMSC.Location=1;
		error = ATGEN_SetSMSC(s,&SMSC);

		if (error != ERR_NONE) {
			return error;
		}
		sprintf(buffer, "AT+CSMP=%i,%i,%i,%i\r",
			req[PHONE_SMSDeliver.firstbyte],
			req[PHONE_SMSDeliver.TPVP],
			req[PHONE_SMSDeliver.TPPID],
			req[PHONE_SMSDeliver.TPDCS]);
		ATGEN_WaitForAutoLen(s, buffer, 0x00, 4, ID_SetSMSParameters);

		if (error == ERR_NOTSUPPORTED) {
			/* Nokia Communicator 9000i doesn't support <vp> parameter */
			sprintf(buffer, "AT+CSMP=%i,,%i,%i\r",
				req[PHONE_SMSDeliver.firstbyte],
				req[PHONE_SMSDeliver.TPPID],
				req[PHONE_SMSDeliver.TPDCS]);
			ATGEN_WaitForAutoLen(s, buffer, 0x00, 4, ID_SetSMSParameters);
		}
		if (error != ERR_NONE) {
			smprintf(s, "WARNING: Failed to set message parameters, continuing without them!\n");
		}
		switch (message->Coding) {
		case SMS_Coding_Default_No_Compression:
			/* If not SMS with UDH, it's as normal text */
			if (message->UDH.Type==UDH_NoUDH) {
				strcpy(hexreq,DecodeUnicodeString(message->Text));
				*length2 = UnicodeLength(message->Text);
				break;
			}
	        case SMS_Coding_Unicode_No_Compression:
	        case SMS_Coding_8bit:
			error=PHONE_EncodeSMSFrame(s,message,buffer,PHONE_SMSDeliver,current,TRUE);

			if (error != ERR_NONE) {
				return error;
			}
			EncodeHexBin (hexreq, buffer+PHONE_SMSDeliver.Text, buffer[PHONE_SMSDeliver.TPUDL]);
			*length2 = buffer[PHONE_SMSDeliver.TPUDL] * 2;
			break;
		default:
			break;
		}
		break;
	}
	return ERR_NONE;
}

GSM_Error ATGEN_AddSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error 		error, error2;
	GSM_Phone_Data		*Phone = &s->Phone.Data;
	unsigned char		buffer[1000] = {'\0'}, hexreq[1000] = {'\0'},folderid = 0;
	const char		*statetxt;
	int			state = 0, Replies = 0, reply = 0, current = 0, length = 0, location = 0;

	/* This phone supports only sent/unsent messages on SIM */
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMSONLYSENT)) {
		if (sms->Folder != 2) {
			smprintf(s, "This phone supports only folder = 2!\n");
			return ERR_NOTSUPPORTED;
		}
	}

	/* Check the lower bound (this is static, we do not support flat memory here */
	if (sms->Folder <= 0) {
		smprintf(s, "Flat memory not supported for adding!\n");
		return ERR_WRONGFOLDER;
	}

	/* We don't actually need this, but let's initialise it. */
	sms->Location = 0;

	/* Set correct memory type */
	error = ATGEN_GetSMSLocation(s, sms, &folderid, &location, TRUE);

	if (error != ERR_NONE) {
		return error;
	}

	/* Set message type based on folder */
	if ((sms->Folder % 2) == 1) {
		/* Inbox folder */
		sms->PDU = SMS_Deliver;
	} else {
		/* Outbox folder */
		sms->PDU = SMS_Submit;

		if (sms->Memory == MEM_ME &&
			GSM_IsPhoneFeatureAvailable(Phone->ModelInfo, F_SUBMIT_SIM_ONLY)) {
			smprintf(s, "This phone probably does not support saving submit messages to ME location!\n");
			smprintf(s, "But trying anyway...\n");
		}
	}

	/* Format SMS frame */
	error = ATGEN_MakeSMSFrame(s, sms, hexreq, &current, &length);

	if (error != ERR_NONE) {
		return error;
	}

	switch (Phone->Priv.ATGEN.SMSMode) {
	case SMS_AT_PDU:
		if (sms->PDU == SMS_Deliver) {
			state = 0;
			if (sms->State == SMS_Read || sms->State == SMS_Sent) state = 1;
		} else {
			state = 2;
			if (sms->State == SMS_Read || sms->State == SMS_Sent) state = 3;
		}
		/* Siemens M20 */
		if (GSM_IsPhoneFeatureAvailable(Phone->ModelInfo, F_M20SMS)) {
			/* No (good and 100% working) support for alphanumeric numbers */
			if (sms->Number[1]!='+' && (sms->Number[1]<'0' || sms->Number[1]>'9')) {
				EncodeUnicode(sms->Number,"123",3);
				error = ATGEN_MakeSMSFrame(s, sms, hexreq, &current, &length);
				if (error != ERR_NONE) return error;
			}
		}
		sprintf(buffer, "AT+CMGW=%i,%i\r",current,state);
		break;
	case SMS_AT_TXT:
		if (sms->PDU == SMS_Deliver) {
			statetxt = "REC UNREAD";
			if (sms->State == SMS_Read || sms->State == SMS_Sent) statetxt = "REC READ";
		} else {
			statetxt = "STO UNSENT";
			if (sms->State == SMS_Read || sms->State == SMS_Sent) statetxt = "STO SENT";
		}
		/* Siemens M20 */
		if (GSM_IsPhoneFeatureAvailable(Phone->ModelInfo, F_M20SMS)) {
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
			smprintf_level(s, D_ERROR, "[Retrying %i]\n", reply+1);
		}
		s->Protocol.Data.AT.EditMode 	= TRUE;
		Replies 			= s->ReplyNum;
		s->ReplyNum			= 1;
		smprintf(s,"Waiting for modem prompt\n");
		ATGEN_WaitForAutoLen(s, buffer, 0x00, 20, ID_SaveSMSMessage);
		s->ReplyNum			 = Replies;

		if (error == ERR_NONE) {
			Phone->DispatchError 	= ERR_TIMEOUT;
			Phone->RequestID 	= ID_SaveSMSMessage;
			usleep(100000);
			smprintf(s, "Saving SMS\n");
			error = s->Protocol.Functions->WriteMessage(s, hexreq, length, 0x00);

			if (error!=ERR_NONE) {
				return error;
			}
			usleep(500000);

			/* CTRL+Z ends entering */
			error = s->Protocol.Functions->WriteMessage(s, "\x1A", 1, 0x00);

			if (error != ERR_NONE) {
				return error;
			}
			usleep(100000);
			error = GSM_WaitForOnce(s, NULL, 0x00, 0x00, 40);

			if (error != ERR_TIMEOUT) {
				return error;
			}
		} else {
			smprintf(s, "Escaping SMS mode\n");
			error2 = s->Protocol.Functions->WriteMessage(s, "\x1B\r", 2, 0x00);

			if (error2 != ERR_NONE) {
				return error2;
			}
			return error;
		}
        }

	return Phone->DispatchError;
}

GSM_Error ATGEN_ReplySendSMS(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	int i = 0,reference = 0;

	if (s->Protocol.Data.AT.EditMode) {
		if (s->Phone.Data.Priv.ATGEN.ReplyState != AT_Reply_SMSEdit) {
			return ERR_UNKNOWN;
		}
		s->Protocol.Data.AT.EditMode = FALSE;
		return ERR_NONE;
	}

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
 		smprintf(s, "SMS sent OK\n");

		/* Number of lines */
		i = 0;
		while (Priv->Lines.numbers[i*2+1] != 0) {
			i++;
		}
		error = ATGEN_ParseReply(s,
				GetLineString(msg.Buffer, &Priv->Lines, i - 1),
				"+CMGS: @i",
				&reference);

		if (error != ERR_NONE) {
			reference = -1;
		}
		s->User.SendSMSStatus(s, 0, reference, s->User.SendSMSStatusUserData);
		return ERR_NONE;
	case AT_Reply_CMSError:
 		smprintf(s, "Error %i\n",Priv->ErrorCode);

 		if (s->User.SendSMSStatus != NULL) {
			s->User.SendSMSStatus(s, Priv->ErrorCode, -1, s->User.SendSMSStatusUserData);
		}
 		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
 		smprintf(s, "Error %i\n",Priv->ErrorCode);

 		if (s->User.SendSMSStatus != NULL) {
			s->User.SendSMSStatus(s, Priv->ErrorCode, -1, s->User.SendSMSStatusUserData);
		}
		return ATGEN_HandleCMEError(s);
	case AT_Reply_Error:
 		if (s->User.SendSMSStatus != NULL) {
			s->User.SendSMSStatus(s, -1, -1, s->User.SendSMSStatusUserData);
		}
		return ERR_UNKNOWN;
	default:
 		if (s->User.SendSMSStatus != NULL) {
			s->User.SendSMSStatus(s, -1, -1, s->User.SendSMSStatusUserData);
		}
		return ERR_UNKNOWNRESPONSE;
	}
}

GSM_Error ATGEN_SendSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error error, error2;
	GSM_Phone_Data *Phone = &s->Phone.Data;
	unsigned char buffer[1000] = {'\0'}, hexreq[1000] = {'\0'};
	int current = 0, length = 0, Replies = 0, retries = 0;

	if (sms->PDU == SMS_Deliver) {
		sms->PDU = SMS_Submit;
	}
	error = ATGEN_MakeSMSFrame(s, sms, hexreq, &current, &length);

	if (error != ERR_NONE) {
		return error;
	}
	switch (Phone->Priv.ATGEN.SMSMode) {
	case SMS_AT_PDU:
		sprintf(buffer, "AT+CMGS=%i\r",current);
		break;
	case SMS_AT_TXT:
		sprintf(buffer, "AT+CMGS=\"%s\"\r",DecodeUnicodeString(sms->Number));
		break;
	}

	/* We will be SMS edit mode */
	s->Protocol.Data.AT.EditMode = TRUE;
	/*
	 * We handle retries on our own, because we need to escape after
	 * failure to avoid sending message with AT commands.
	 */
	Replies = s->ReplyNum;
	s->ReplyNum = 1;

	while (retries < s->ReplyNum) {
		smprintf(s,"Waiting for modem prompt\n");
		ATGEN_WaitForAutoLen(s, buffer, 0x00, 30, ID_IncomingFrame);

		/* Restore original value */
		s->ReplyNum = Replies;

		if (error == ERR_NONE) {
			usleep(100000);
			smprintf(s, "Sending SMS\n");
			error = s->Protocol.Functions->WriteMessage(s, hexreq, length, 0x00);

			if (error!=ERR_NONE) {
				return error;
			}
			usleep(500000);
			/* CTRL+Z ends entering */
			error = s->Protocol.Functions->WriteMessage(s, "\x1A", 1, 0x00);
			usleep(100000);
			return error;
		}
		smprintf(s, "Escaping SMS mode\n");
		error2 = s->Protocol.Functions->WriteMessage(s, "\x1B\r", 2, 0x00);
		if (error2 != ERR_NONE) {
			return error2;
		}
		retries++;
	}
	return error;
}

GSM_Error ATGEN_SendSavedSMS(GSM_StateMachine *s, int Folder, int Location)
{
	GSM_Error error;
	GSM_MultiSMSMessage msms;
	unsigned char req[100] = {'\0'}, smsfolder = 0;
	int location = 0;

	msms.Number = 0;
	msms.SMS[0].Folder 	= Folder;
	msms.SMS[0].Location 	= Location;
	msms.SMS[0].Memory	= 0;

	/* By reading SMS we check if it is really inbox/outbox */
	error = ATGEN_GetSMS(s, &msms);

	if (error != ERR_NONE) {
		return error;
	}

	/* Can not send from other folder that outbox */
	if (msms.SMS[0].Folder != 2 && msms.SMS[0].Folder != 4) {
		return ERR_NOTSUPPORTED;
	}

	/* Set back original position as it was probably adjusted when
	 * reading message from phone (eg. folder was filled in). */
	msms.SMS[0].Folder 	= Folder;
	msms.SMS[0].Location 	= Location;
	msms.SMS[0].Memory	= 0;

	/* Adjust location to real ones */
	error=ATGEN_GetSMSLocation(s, &msms.SMS[0], &smsfolder, &location, FALSE);

	if (error != ERR_NONE) {
		return error;
	}
	sprintf(req, "AT+CMSS=%i\r",location);
	error=s->Protocol.Functions->WriteMessage(s, req, strlen(req), 0x00);
	usleep(strlen(req)*1000);
	return error;
}

GSM_Error ATGEN_ReplyGetSMSC(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_SMSC *SMSC = s->Phone.Data.SMSC;
	int number_type = 0;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "SMSC info received\n");

		/* Parse reply */
		error = ATGEN_ParseReply(s,
					GetLineString(msg.Buffer, &Priv->Lines, 2),
					"+CSCA: @p, @i",
					SMSC->Number, sizeof(SMSC->Number),
					&number_type);
		if (error != ERR_NONE) {
			error = ATGEN_ParseReply(s,
						GetLineString(msg.Buffer, &Priv->Lines, 2),
						"+CSCA: @p, @0",
						SMSC->Number, sizeof(SMSC->Number));
		}
		if (error != ERR_NONE) {
			error = ATGEN_ParseReply(s,
						GetLineString(msg.Buffer, &Priv->Lines, 2),
						"+CSCA: @p",
						SMSC->Number, sizeof(SMSC->Number));
			number_type = NUMBER_INTERNATIONAL_NUMBERING_PLAN_ISDN;
		}
		if (error != ERR_NONE) {
			return error;
		}
		if (UnicodeLength(SMSC->Number) == 0) return ERR_EMPTY;

		/* International number */
		GSM_TweakInternationalNumber(SMSC->Number, number_type);

		/* Some things we can not find out */
		SMSC->Format 		= SMS_FORMAT_Text;
		SMSC->Validity.Format = SMS_Validity_RelativeFormat;
		SMSC->Validity.Relative	= SMS_VALID_Max_Time;
		SMSC->Name[0]		= 0;
		SMSC->Name[1]		= 0;
		SMSC->DefaultNumber[0]	= 0;
		SMSC->DefaultNumber[1]	= 0;
		return ERR_NONE;
	case AT_Reply_CMSError:
		return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
		return ATGEN_HandleCMEError(s);
	case AT_Reply_Error:
		return ERR_NOTSUPPORTED;
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	GSM_Error error;

	/*
	* Get SMSC command
	*/
	const char req[] = "AT+CSCA?\r";

	/* Only one location supported */
	if (smsc->Location != 1) {
		return ERR_INVALIDLOCATION;
	}

	/* We prefer normal charset */
	error = ATGEN_SetCharset(s, AT_PREF_CHARSET_NORMAL);

	if (error != ERR_NONE) {
		return error;
	}

	/* Issue command */
	s->Phone.Data.SMSC = smsc;
	smprintf(s, "Getting SMSC\n");
	ATGEN_WaitForAutoLen(s, req, 0x00, 4, ID_GetSMSC);
	return error;
}

GSM_Error ATGEN_ReplyDeleteSMSMessage(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	switch (s->Phone.Data.Priv.ATGEN.ReplyState) {
	case AT_Reply_OK:
		smprintf(s, "SMS deleted OK\n");
		return ERR_NONE;
	case AT_Reply_Error:
		smprintf(s, "Invalid location\n");
		return ERR_INVALIDLOCATION;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
	        return ATGEN_HandleCMEError(s);
	default:
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error ATGEN_DeleteSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	GSM_Error error;
	GSM_MultiSMSMessage msms;
	unsigned char req[20] = {'\0'}, folderid = 0;
	int location = 0, length = 0;

	msms.Number = 0;
	msms.SMS[0] = *sms;

	/* By reading SMS we check if it is really inbox/outbox */
	error = ATGEN_GetSMS(s, &msms);

	if (error != ERR_NONE) {
		return error;
	}
	error = ATGEN_GetSMSLocation(s, sms, &folderid, &location, TRUE);

	if (error != ERR_NONE) {
		return error;
	}
	smprintf(s, "Deleting SMS\n");
	length=sprintf(req, "AT+CMGD=%i\r",location);
	ATGEN_WaitFor(s, req, length, 0x00, 5, ID_DeleteSMSMessage);
	return error;
}

GSM_Error ATGEN_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	GSM_Error error;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	int used = 0;

	if (Priv->PhoneSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, FALSE, FALSE, FALSE);

		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			return error;
		}
	}
	if (Priv->SIMSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, TRUE, FALSE, FALSE);

		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			return error;
		}
	}
	folders->Number = 0;

	if (Priv->PhoneSMSMemory == AT_NOTAVAILABLE && Priv->SIMSMSMemory == AT_NOTAVAILABLE) {
		return ERR_NONE;
	}
	PHONE_GetSMSFolders(s,folders);

	if (Priv->SIMSMSMemory == AT_AVAILABLE) {
		used = 2;
	}
	if (Priv->PhoneSMSMemory == AT_AVAILABLE) {
		if (used != 0) {
			CopyUnicodeString(folders->Folder[used    ].Name,folders->Folder[0].Name);
			CopyUnicodeString(folders->Folder[used + 1].Name,folders->Folder[1].Name);
			folders->Folder[used    ].InboxFolder 	= folders->Folder[0].InboxFolder;
			folders->Folder[used + 1].InboxFolder 	= folders->Folder[1].InboxFolder;
			folders->Folder[used    ].OutboxFolder 	= folders->Folder[0].OutboxFolder;
			folders->Folder[used + 1].OutboxFolder 	= folders->Folder[1].OutboxFolder;
		}
		folders->Folder[used    ].Memory = MEM_ME;
		folders->Folder[used + 1].Memory = MEM_ME;
		folders->Number += 2;
		used += 2;
	}
	return ERR_NONE;
}

GSM_Error ATGEN_IncomingSMSCInfo(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s UNUSED)
{
	return ERR_NONE;
}

GSM_Error ATGEN_SetFastSMSSending(GSM_StateMachine *s, gboolean enable)
{
	GSM_Error error;

	if (enable) {
		smprintf(s, "Enabling fast SMS sending\n");
		ATGEN_WaitForAutoLen(s, "AT+CMMS=2\r", 0x00, 4, ID_SetFastSMSSending);
	} else {
		smprintf(s, "Disabling fast SMS sending\n");
		ATGEN_WaitForAutoLen(s, "AT+CMMS=0\r", 0x00, 4, ID_SetFastSMSSending);
	}
	return error;
}

GSM_Error ATGEN_IncomingSMSInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_Phone_Data *Data = &s->Phone.Data;
	GSM_SMSMessage sms;

	/* We get here: +CMTI: SM, 19 */
	char *buffer = NULL;

	memset(&sms, 0, sizeof(sms));
	smprintf(s, "Incoming SMS\n");

	if (Data->EnableIncomingSMS && s->User.IncomingSMS!=NULL) {
		sms.State 	 = 0;
		sms.InboxFolder  = TRUE;
		sms.PDU 	 = 0;

		buffer = strchr(msg.Buffer, ':');

		if (buffer == NULL) {
			return ERR_UNKNOWNRESPONSE;
		}
		buffer++;

		while (isspace((int)*buffer)) {
			buffer++;
		}
		if (strncmp(buffer, "ME", 2) == 0 || strncmp(buffer, "\"ME\"", 4) == 0) {
			if (Priv->SIMSMSMemory == AT_AVAILABLE) {
				sms.Folder = 3;
			} else {
				sms.Folder = 1;
			}
		} else if (strncmp(buffer, "SM", 2) == 0 || strncmp(buffer, "\"SM\"", 4) == 0) {
			sms.Folder = 1;
		} else {
			return ERR_UNKNOWNRESPONSE;
		}
		buffer = strchr(msg.Buffer, ',');

		if (buffer == NULL) {
			return ERR_UNKNOWNRESPONSE;
		}
		buffer++;

		while (isspace((int)*buffer)) {
			buffer++;
		}
		sms.Location = atoi(buffer);
		s->User.IncomingSMS(s, sms, s->User.IncomingSMSUserData);
	}
	return ERR_NONE;
}

GSM_Error ATGEN_IncomingSMSDeliver(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;
	GSM_SMSMessage sms;
	unsigned char buffer[300] = {'\0'}, smsframe[800] = {'\0'};
	int current = 0, length, i=0;

	smprintf(s, "Incoming SMS received (Deliver)\n");

	if (Data->EnableIncomingSMS && s->User.IncomingSMS!=NULL) {
		sms.State 	 = SMS_UnRead;
		sms.InboxFolder  = TRUE;
		sms.PDU 	 = SMS_Deliver;

		/* T310 with larger SMS goes crazy and mix this incoming
                 * frame with normal answers. PDU is always last frame
		 * We find its' number and parse it */
		while (Data->Priv.ATGEN.Lines.numbers[i*2+1] != 0) {
			/* FIXME: handle special chars correctly */
			i++;
		}
		DecodeHexBin (buffer,
			GetLineString(msg.Buffer,&Data->Priv.ATGEN.Lines,i),
			GetLineLength(msg.Buffer,&Data->Priv.ATGEN.Lines,i));

		/* We use locations from SMS layouts like in ../phone2.c(h) */
		for(i=0;i<buffer[0]+1;i++) {
			smsframe[i]=buffer[current++];
		}
		smsframe[12]=buffer[current++];
		length=((buffer[current])+1)/2+1;

		for(i=0;i<length+1;i++) {
			smsframe[PHONE_SMSDeliver.Number+i]=buffer[current++];
		}
		smsframe[PHONE_SMSDeliver.TPPID] = buffer[current++];
		smsframe[PHONE_SMSDeliver.TPDCS] = buffer[current++];

		for(i=0;i<7;i++) {
			smsframe[PHONE_SMSDeliver.DateTime+i]=buffer[current++];
		}
		smsframe[PHONE_SMSDeliver.TPUDL] = buffer[current++];

		for(i=0;i<smsframe[PHONE_SMSDeliver.TPUDL];i++) {
			smsframe[i+PHONE_SMSDeliver.Text]=buffer[current++];
		}
		GSM_DecodeSMSFrame(&(s->di), &sms,smsframe,PHONE_SMSDeliver);
		s->User.IncomingSMS(s,sms, s->User.IncomingSMSUserData);
	}
	return ERR_NONE;
}

/* I don't have phone able to do it and can't fill it */
GSM_Error ATGEN_IncomingSMSReport(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Incoming SMS received (Report)\n");
	return ERR_NONE;
}

gboolean InRange(int *range, int i) {
	while (*range != -1) {
		if (*range == i) {
			return TRUE;
		}
		range++;
	}
	return FALSE;
}

int *GetRange(GSM_StateMachine *s, const char *buffer)
{
	int *result = NULL;
	size_t	allocated = 0, pos = 0;
	const char *chr = buffer;
	char *endptr = NULL;
	gboolean in_range = FALSE;
	int current, diff, i;

	smprintf(s, "Parsing range: %s\n", chr);

	if (*chr != '(') {
		return NULL;
	}
	chr++;

	while (*chr != ')' && *chr != 0) {
		/* Read current number */
		current = strtol(chr, &endptr, 10);

		/* Detect how much numbers we have to store */
		if (in_range) {
			diff = current - result[pos - 1];
		} else {
			diff = 1;
		}

		/* Did we parse anything? */
		if (endptr == chr) {
			smprintf(s, "Failed to find number in range!\n");
			return NULL;
		}
		/* Allocate more memory if needed */
		if (allocated < pos + diff + 1) {
			result = (int *)realloc(result, sizeof(int) * (pos + diff + 10));
			if (result == NULL) {
				smprintf(s, "Not enough memory to parse range!\n");
				return NULL;
			}
			allocated = pos + 10 + diff;
		}

		/* Store number is memory */
		if (!in_range) {
			result[pos++] = current;
		} else {
			for (i = result[pos - 1] + 1; i <= current; i++) {
				result[pos++] = i;
			}
			in_range = FALSE;
		}
		/* Skip to next char after number */
		chr = endptr;

		/* Check for character after number */
		if (*chr == '-') {
			in_range = TRUE;
			chr++;
		} else if (*chr == ',') {
			chr++;
		} else if (*chr == ')') {
			result[pos++] = -1;
			break;
		} else if (*chr != ',') {
			smprintf(s, "Bad character in range: %c\n", *chr);
			return NULL;
		}
	}
	smprintf(s, "Returning range: ");

	for (i = 0; result[i] != -1; i++) {
		smprintf(s, "%d, ", result[i]);
	}
	smprintf(s, "-1\n");
	return result;
}

GSM_Error ATGEN_ReplyGetCNMIMode(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	const char *buffer;
	int *range = NULL;

	switch (Priv->ReplyState) {
	case AT_Reply_OK:
		break;
	case AT_Reply_Error:
		return ERR_NOTSUPPORTED;
	case AT_Reply_CMSError:
	        return ATGEN_HandleCMSError(s);
	case AT_Reply_CMEError:
	        return ATGEN_HandleCMEError(s);
	default:
		return ERR_UNKNOWNRESPONSE;
	}

	/* Sample resposne we get here:
	AT+CNMI=?
	+CNMI: (0-2),(0,1,3),(0),(0,1),(0,1)

	Or:
	+CNMI:(0-3),(0-3),(0-3),(0,1),(0,1)

	Or:
	+CNMI: (2),(0-1,3),(0,2),(0-1),(0)"
	*/
	Priv->CNMIMode			= 0;
	Priv->CNMIProcedure		= 0;
	Priv->CNMIDeliverProcedure	= 0;
#ifdef GSM_ENABLE_CELLBROADCAST
	Priv->CNMIBroadcastProcedure	= 0;
#endif

	buffer = GetLineString(msg.Buffer, &Priv->Lines, 2);

	if (buffer == NULL) {
		return  ERR_UNKNOWNRESPONSE;
	}
	while (isspace((int)*buffer)) {
		buffer++;
	}
	if (strncmp(buffer, "+CNMI:", 6) != 0) {
		return ERR_UNKNOWNRESPONSE;
	}
	buffer += 7;
	buffer = strchr(buffer, '(');

	if (buffer == NULL) {
		return  ERR_UNKNOWNRESPONSE;
	}
	range = GetRange(s, buffer);

	if (range == NULL) {
		return  ERR_UNKNOWNRESPONSE;
	}
	if (InRange(range, 2)) {
		Priv->CNMIMode = 2; 	/* 2 = buffer messages and send them when link is free */
	}
	else if (InRange(range, 3)) {
		Priv->CNMIMode = 3; /* 3 = send messages directly */
	}
	else {
		return ERR_NONE; /* we don't want: 1 = ignore new messages, 0 = store messages and no indication */
	}
	free(range);
	range=NULL;

	buffer++;
	buffer = strchr(buffer, '(');

	if (buffer == NULL) {
		return  ERR_UNKNOWNRESPONSE;
	}
	range = GetRange(s, buffer);
	if (range == NULL) {
		return  ERR_UNKNOWNRESPONSE;
	}
	if (InRange(range, 1)) {
		Priv->CNMIProcedure = 1; 	/* 1 = store message and send where it is stored */
	}
	else if (InRange(range, 2)) {
		Priv->CNMIProcedure = 2; 	/* 2 = route message to TE */
	}
	else if (InRange(range, 3)) {
		Priv->CNMIProcedure = 3; 	/* 3 = 1 + route class 3 to TE */
	}
	/* we don't want: 0 = just store to memory */
	free(range);
	range=NULL;

	buffer++;
	buffer = strchr(buffer, '(');
#ifdef GSM_ENABLE_CELLBROADCAST
	if (buffer == NULL) {
		return  ERR_UNKNOWNRESPONSE;
	}
	range = GetRange(s, buffer);

	if (range == NULL) {
		return  ERR_UNKNOWNRESPONSE;
	}
	if (InRange(range, 2)) {
		Priv->CNMIBroadcastProcedure = 2; /* 2 = route message to TE */
	}
	else if (InRange(range, 1)) {
		Priv->CNMIBroadcastProcedure = 1; /* 1 = store message and send where it is stored */
	}
	else if (InRange(range, 3)) {
		Priv->CNMIBroadcastProcedure = 3; /* 3 = 1 + route class 3 to TE */
	}
	/* we don't want: 0 = just store to memory */
	free(range);
	range=NULL;
#endif

	buffer++;
	buffer = strchr(buffer, '(');

	if (buffer == NULL) {
		return  ERR_UNKNOWNRESPONSE;
	}
	range = GetRange(s, buffer);

	if (range == NULL) {
		return  ERR_UNKNOWNRESPONSE;
	}
	if (InRange(range, 1)) {
		Priv->CNMIDeliverProcedure = 1; /* 1 = route message to TE */
	}
	else if (InRange(range, 2)) {
		Priv->CNMIDeliverProcedure = 2; /* 1 = store message and send where it is stored */
	}
	/* we don't want: 0 = no routing */
	free(range);
	range=NULL;
	return ERR_NONE;
}

GSM_Error ATGEN_GetCNMIMode(GSM_StateMachine *s)
{
	GSM_Error error;

	ATGEN_WaitForAutoLen(s, "AT+CNMI=?\r", 0x00, 4, ID_GetCNMIMode);
	return error;
}

GSM_Error ATGEN_SetIncomingCB(GSM_StateMachine *s, gboolean enable)
{
#ifdef GSM_ENABLE_CELLBROADCAST
	GSM_Error error = ERR_NONE;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	char buffer[100] = {'\0'};
	int length = 0;

	if (Priv->CNMIMode == -1) {
		error = ATGEN_GetCNMIMode(s);

		if (error != ERR_NONE) {
			return error;
		}
	}

	if (Priv->CNMIMode == 0) {
		return ERR_NOTSUPPORTED;
	}
	if (Priv->CNMIBroadcastProcedure == 0) {
		return ERR_NOTSUPPORTED;
	}
	if (s->Phone.Data.EnableIncomingCB!=enable) {
		s->Phone.Data.EnableIncomingCB 	= enable;
		if (enable) {
			smprintf(s, "Enabling incoming CB\n");
			length=sprintf(buffer, "AT+CNMI=%d,,%d\r", Priv->CNMIMode, Priv->CNMIBroadcastProcedure);
			ATGEN_WaitFor(s, buffer, length, 0x00, 4, ID_SetIncomingCB);
		} else {
			smprintf(s, "Disabling incoming CB\n");
			length=sprintf(buffer, "AT+CNMI=%d,,%d\r", Priv->CNMIMode, 0);
			ATGEN_WaitFor(s, buffer, length, 0x00, 4, ID_SetIncomingCB);
		}
		return error;
	}
	return ERR_NONE;
#else
	return ERR_SOURCENOTAVAILABLE;
#endif
}


GSM_Error ATGEN_SetIncomingSMS(GSM_StateMachine *s, gboolean enable)
{
	GSM_Error error = ERR_NONE;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	char buffer[100] = {'\0'};
	int length=0;

	/* We will need this when incoming message, but we can not invoke AT commands there: */
	if (Priv->PhoneSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, FALSE, FALSE, FALSE);
		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			return error;
		}
	}
	if (Priv->SIMSMSMemory == 0) {
		error = ATGEN_SetSMSMemory(s, TRUE, FALSE, FALSE);

		if (error != ERR_NONE && error != ERR_NOTSUPPORTED) {
			return error;
		}
	}
	if (Priv->CNMIMode == -1) {
		error = ATGEN_GetCNMIMode(s);

		if (error != ERR_NONE) {
			return error;
		}
	}
	if (Priv->CNMIMode == 0) {
		return ERR_NOTSUPPORTED;
	}
	if (Priv->CNMIProcedure == 0 && Priv->CNMIDeliverProcedure == 0) {
		return ERR_NOTSUPPORTED;
	}
	if (s->Phone.Data.EnableIncomingSMS!=enable) {
		s->Phone.Data.EnableIncomingSMS = enable;

		if (enable) {
			smprintf(s, "Enabling incoming SMS\n");

			/* Delivery reports */
			if (Priv->CNMIDeliverProcedure != 0) {
				length=sprintf(buffer, "AT+CNMI=%d,,,%d\r", Priv->CNMIMode, Priv->CNMIDeliverProcedure);
				ATGEN_WaitFor(s, buffer, length, 0x00, 4, ID_SetIncomingSMS);

				if (error != ERR_NONE) {
					return error;
				}
			}

			/* Normal messages */
			if (Priv->CNMIProcedure != 0) {
				length=sprintf(buffer, "AT+CNMI=%d,%d\r", Priv->CNMIMode, Priv->CNMIProcedure);
				ATGEN_WaitFor(s, buffer, length, 0x00, 4, ID_SetIncomingSMS);

				if (error != ERR_NONE) {
					return error;
				}
			}
		} else {
			smprintf(s, "Disabling incoming SMS\n");

			/* Delivery reports */
			length=sprintf(buffer,"AT+CNMI=%d,,,%d\r", Priv->CNMIMode, 0);
			ATGEN_WaitFor(s, buffer, length, 0x00, 4, ID_SetIncomingSMS);

			if (error != ERR_NONE) {
				return error;
			}

			/* Normal messages */
			length=sprintf(buffer, "AT+CNMI=%d,%d\r", Priv->CNMIMode, 0);
			ATGEN_WaitFor(s, buffer, length, 0x00, 4, ID_SetIncomingSMS);

			if (error != ERR_NONE) {
				return error;
			}
		}
	}
	return ERR_NONE;
}

#ifdef GSM_ENABLE_CELLBROADCAST

GSM_Error ATGEN_ReplyIncomingCB(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
#if 0
	GSM_CBMessage 	CB;
	char Buffer[300] = {'\0'},Buffer2[300] = {'\0'};
	int i = 0;
	size_t j = 0;

	DecodeHexBin (Buffer,msg.Buffer+6,msg.Length-6);
	DumpMessage(&di ,Buffer,msg.Length-6);

	CB.Channel = Buffer[4];

	for (j=0;j<msg.Length;j++) {
		smprintf(s, "j=" SIZE_T_FORMAT "\n",j);
		i=GSM_UnpackEightBitsToSeven(0, msg.Buffer[6], msg.Buffer[6], msg.Buffer+j, Buffer2);
		i = msg.Buffer[6] - 1;

		while (i!=0) {
			if (Buffer[i] == 13) i = i - 1; else break;
		}
		DecodeDefault(CB.Text, Buffer2, msg.Buffer[6], FALSE, NULL);
		smprintf(s, "Channel %i, text \"%s\"\n",CB.Channel,DecodeUnicodeString(CB.Text));
	}
	if (s->Phone.Data.EnableIncomingCB && s->User.IncomingCB!=NULL) {
		s->User.IncomingCB(s,CB);
	}
	return ERR_NONE;
#else
	smprintf(s, "CB received\n");
	return ERR_NONE;
#endif
}

#endif


#endif
/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
