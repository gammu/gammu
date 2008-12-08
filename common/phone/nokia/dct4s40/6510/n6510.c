/* (c) 2002-2005 by Marcin Wiacek */
/* based on some Markus Plail, Pawel Kot work from Gnokii (www.gnokii.org)
 * (C) 1999-2000 Hugh Blemings & Pavel Janik ml. (C) 2001-2004 Pawel Kot
 * GNU GPL version 2 or later
 */
/* Due to a problem in the source code management, the names of some of
 * the authors have unfortunately been lost. We do not mean to belittle
 * their efforts and hope they will contact us to see their names
 * properly added to the Copyright notice above.
 * Having published their contributions under the terms of the GNU
 * General Public License (GPL) [version 2], the Copyright of these
 * authors will remain respected by adhering to the license they chose
 * to publish their code under.
 */

#include <gammu-config.h>

#ifdef GSM_ENABLE_NOKIA6510

#include <string.h>
#include <time.h>

#include <gammu-nokia.h>

#include "../../../../misc/coding/coding.h"
#include "../../../../misc/locales.h"
#include "../../../../gsmcomon.h"
#include "../../../../service/gsmlogo.h"
#include "../../nfunc.h"
#include "../../nfuncold.h"
#include "../../../pfunc.h"
#include "../dct4func.h"
#include "n6510.h"
#include "6510cal.h"
#include "6510file.h"

static GSM_Error N6510_Initialise (GSM_StateMachine *s)
{
	s->Phone.Data.Priv.N6510.CalendarIconsNum  = 0;
	s->Phone.Data.Priv.N6510.LastFreeMemoryLocation = 0;
	s->Phone.Data.Priv.N6510.LastFreeMemoryType = 0;

	/* Enables various things like incoming SMS, call info, etc. */
	return N71_65_EnableFunctions (s, "\x01\x02\x06\x0A\x14\x17\x39", 7);
}

static GSM_Error N6510_ReplyGetMemory(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Phonebook entry received\n");
	if (msg.Buffer[6] == 0x0f)
		return N71_65_ReplyGetMemoryError(msg.Buffer[10], s);

	if (msg.Length < 22) {
		return ERR_UNKNOWN;
	}
	return N71_65_DecodePhonebook(s,
			s->Phone.Data.Memory,
			s->Phone.Data.Bitmap,
			s->Phone.Data.SpeedDial,
			msg.Buffer + 22,
			msg.Length - 22,
			false);
}

static GSM_Error N6510_GetMemory (GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	GSM_Error error;
	unsigned char req[] = {N6110_FRAME_HEADER, 0x07, 0x01, 0x01, 0x00, 0x01,
			       0xfe, 0x10, 	/* memory type */
			       0x00, 0x00, 0x00, 0x00,
			       0x00, 0x01, 	/* location */
			       0x00, 0x00, 0x01};

	req[9] = NOKIA_GetMemoryType(s, entry->MemoryType,N71_65_MEMORY_TYPES);
	if (req[9]==0xff) return ERR_NOTSUPPORTED;

	if (entry->Location==0x00) return ERR_INVALIDLOCATION;

	req[14] = entry->Location / 256;
	req[15] = entry->Location % 256;

	s->Phone.Data.Memory=entry;
	smprintf(s, "Getting phonebook entry\n");
	error = GSM_WaitFor (s, req, 19, 0x03, 4, ID_GetMemory);
	if (error == ERR_WORKINPROGRESS) {
		sleep(2);
		smprintf(s, "Retrying to get phonebook entry\n");
		error = GSM_WaitFor (s, req, 19, 0x03, 4, ID_GetMemory);
	}
	if (entry->MemoryType == MEM_DC || entry->MemoryType == MEM_RC || entry->MemoryType == MEM_MC) {
		/* 6111 */
		if (error == ERR_NOTSUPPORTED) return ERR_EMPTY;
	}
	return error;
}

static GSM_Error N6510_ReplyGetMemoryStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Memory status received\n");
	/* Quess ;-)) */
	if (msg.Buffer[14]==0x10) {
		Data->MemoryStatus->MemoryFree = msg.Buffer[18]*256 + msg.Buffer[19];
	} else if (msg.Buffer[14]==0x02) {
		Data->MemoryStatus->MemoryFree = msg.Buffer[22];
	} else {
		Data->MemoryStatus->MemoryFree = msg.Buffer[17];
	}
	smprintf(s, "Size       : %i\n",Data->MemoryStatus->MemoryFree);
	Data->MemoryStatus->MemoryUsed = msg.Buffer[20]*256 + msg.Buffer[21];
	smprintf(s, "Used       : %i\n",Data->MemoryStatus->MemoryUsed);
	Data->MemoryStatus->MemoryFree -= Data->MemoryStatus->MemoryUsed;
	smprintf(s, "Free       : %i\n",Data->MemoryStatus->MemoryFree);
	return ERR_NONE;
}

static GSM_Error N6510_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x03, 0x02,
			       0x00,		/* memory type */
			       0x55, 0x55, 0x55, 0x00};

	req[5] = NOKIA_GetMemoryType(s, Status->MemoryType,N71_65_MEMORY_TYPES);
	if (req[5]==0xff) return ERR_NOTSUPPORTED;

	s->Phone.Data.MemoryStatus=Status;
	smprintf(s, "Getting memory status\n");
	return GSM_WaitFor (s, req, 10, 0x03, 4, ID_GetMemoryStatus);
}

static GSM_Error N6510_ReplyGetSMSC(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 			i, current, j;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	switch (msg.Buffer[4]) {
		case 0x00:
			smprintf(s, "SMSC received\n");
			break;
		case 0x02:
			smprintf(s, "SMSC empty\n");
			return ERR_INVALIDLOCATION;
		default:
			smprintf(s, "Unknown SMSC state: %02x\n",msg.Buffer[4]);
			return ERR_UNKNOWNRESPONSE;
	}
	memset(Data->SMSC,0,sizeof(GSM_SMSC));
	Data->SMSC->Location 	= msg.Buffer[8];
	Data->SMSC->Format 	= SMS_FORMAT_Text;
	switch (msg.Buffer[10]) {
		case 0x00: Data->SMSC->Format = SMS_FORMAT_Text; 	break;
		case 0x22: Data->SMSC->Format = SMS_FORMAT_Fax; 	break;
		case 0x26: Data->SMSC->Format = SMS_FORMAT_Pager;	break;
		case 0x32: Data->SMSC->Format = SMS_FORMAT_Email;	break;
	}
	Data->SMSC->Validity.Format	= SMS_Validity_RelativeFormat;
	Data->SMSC->Validity.Relative	= msg.Buffer[12];
	if (msg.Buffer[12] == 0x00) Data->SMSC->Validity.Relative = SMS_VALID_Max_Time;

	current = 14;
	for (i=0;i<msg.Buffer[13];i++) {
		switch (msg.Buffer[current]) {
		case 0x81:
			j=current+4;
			while (msg.Buffer[j]!=0) {j++;}
			j=j-33;
			if (j>GSM_MAX_SMSC_NAME_LENGTH) {
				smprintf(s, "Too long name\n");
				return ERR_UNKNOWNRESPONSE;
			}
			CopyUnicodeString(Data->SMSC->Name,msg.Buffer+current+4);
			smprintf(s, "   Name \"%s\"\n", DecodeUnicodeString(Data->SMSC->Name));
			break;
		case 0x82:
			switch (msg.Buffer[current+2]) {
			case 0x01:
				GSM_UnpackSemiOctetNumber(Data->SMSC->DefaultNumber,msg.Buffer+current+4,true);
				smprintf(s, "   Default number \"%s\"\n", DecodeUnicodeString(Data->SMSC->DefaultNumber));
				break;
			case 0x02:
				GSM_UnpackSemiOctetNumber(Data->SMSC->Number,msg.Buffer+current+4,false);
				smprintf(s, "   Number \"%s\"\n", DecodeUnicodeString(Data->SMSC->Number));
				break;
			default:
				smprintf(s, "Unknown SMSC number: %02x\n",msg.Buffer[current+2]);
				return ERR_UNKNOWNRESPONSE;
			}
			break;
		default:
			smprintf(s, "Unknown SMSC block: %02x\n",msg.Buffer[current]);
			return ERR_UNKNOWNRESPONSE;
		}
		current = current + msg.Buffer[current+1];
	}
	return ERR_NONE;
}

static GSM_Error N6510_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x14,
			       0x01,		/* location */
			       0x00};

	if (smsc->Location==0x00) return ERR_INVALIDLOCATION;

	req[4]=smsc->Location;

	s->Phone.Data.SMSC=smsc;
	smprintf(s, "Getting SMSC\n");
	return GSM_WaitFor (s, req, 6, 0x02, 4, ID_GetSMSC);
}

static GSM_Error N6510_ReplySetSMSC(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[4]) {
		case 0x00:
			smprintf(s, "SMSC set OK\n");
			return ERR_NONE;
		case 0x02:
			smprintf(s, "Invalid SMSC location\n");
			return ERR_INVALIDLOCATION;
		default:
			smprintf(s, "Unknown SMSC state: %02x\n",msg.Buffer[4]);
			return ERR_UNKNOWNRESPONSE;
	}
}

static GSM_Error N6510_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	int 		count = 13,i;
	unsigned char 	req[256] = {N6110_FRAME_HEADER,
		    		    0x12, 0x55, 0x01, 0x0B, 0x34,
				    0x05,		/* Location 	*/
				    0x00,
				    0x00,		/* Format 	*/
				    0x00,
				    0xFF};		/* Validity	*/

	req[8]  = smsc->Location;
	switch (smsc->Format) {
		case SMS_FORMAT_Text:	req[10] = 0x00;	break;
		case SMS_FORMAT_Fax:	req[10] = 0x22;	break;
		case SMS_FORMAT_Pager:	req[10] = 0x26;	break;
		case SMS_FORMAT_Email:	req[10] = 0x32;	break;
	}
	req[12]  = smsc->Validity.Relative;

	/* We have now blocks. Number of blocks = 3 */
	req[count++] 	 	 = 0x03;

	/* -------------- SMSC number ----------------- */
	/* Block type: number */
	req[count++] 		 = 0x82;
	/* Offset to next block */
	req[count++] 		 = 0x1A;
 	/* Type of number: SMSC number */
	req[count++] 		 = 0x02;
	req[count] = GSM_PackSemiOctetNumber(smsc->Number, req+count+2, false) + 1;
	if (req[count]>18) {
		smprintf(s, "Too long SMSC number in frame\n");
		return ERR_UNKNOWN;
	}
	req[count+1] = req[count] - 1;
	count += 23;

	/* --------------- Default number ------------- */
	/* Block type: number */
	req[count++] 		 = 0x82;
	/* Offset to next block */
	req[count++] 		 = 0x14;
 	/* Type of number: default number */
	req[count++] 		 = 0x01;
	req[count] = GSM_PackSemiOctetNumber(smsc->DefaultNumber, req+count+2, true) + 1;
	if (req[count]*2>12) {
		smprintf(s, "Too long SMSC number in frame\n");
		return ERR_UNKNOWN;
	}
	req[count+1] = req[count] - 1;
	count += 17;

	/* -------------- SMSC name ------------------- */
	req[count++] 		 = 0x81;
	req[count++] 		 = UnicodeLength(smsc->Name)*2 + 2 + 4;
	req[count++] 		 = UnicodeLength(smsc->Name)*2 + 2;
	req[count++] 		 = 0x00;
	/* Can't make CopyUnicodeString(req+count,sms->Name) !!!!
	 * with MSVC6 count is changed then
	 */
	i = count;
	CopyUnicodeString(req+i,smsc->Name);
	count += UnicodeLength(smsc->Name)*2 + 2;

	smprintf(s, "Setting SMSC\n");
	return GSM_WaitFor (s, req, count, 0x02, 4, ID_SetSMSC);
}

static GSM_Error N6510_ReplyGetNetworkInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int		current = msg.Buffer[7]+7, tmp;
	GSM_Phone_Data	*Data = &s->Phone.Data;
#ifdef DEBUG
	char		name[100];
	GSM_NetworkInfo NetInfo;

	smprintf(s, "Network status            : ");
	switch (msg.Buffer[8]) {
		case 0x00 : smprintf(s, "home network\n");		break;
		case 0x01 : smprintf(s, "roaming network\n");      	break;
		case 0x04 : smprintf(s, "not logged");		  	break;
		case 0x06 : smprintf(s, "SIM card rejected\n");		break;
		case 0x09 : smprintf(s, "not logged");		  	break;
		default	  : smprintf(s, "unknown %i!\n",msg.Buffer[8]); break;
	}
	if (msg.Buffer[8]==0x00 || msg.Buffer[8] == 0x01) {
		NOKIA_DecodeNetworkCode(msg.Buffer + (current + 7),NetInfo.NetworkCode);
		smprintf(s, "Network code              : %s\n", NetInfo.NetworkCode);
		smprintf(s, "Network name for Gammu    : %s ",
			DecodeUnicodeString(GSM_GetNetworkName(NetInfo.NetworkCode)));
		smprintf(s, "(%s)\n",DecodeUnicodeString(GSM_GetCountryName(NetInfo.NetworkCode)));

		sprintf(NetInfo.LAC, "%02X%02X", msg.Buffer[current+1], msg.Buffer[current+2]);
		smprintf(s, "LAC                       : %s\n", NetInfo.LAC);

		sprintf(NetInfo.CID, "%02X%02X", msg.Buffer[current+5], msg.Buffer[current+6]);
		smprintf(s, "CID                       : %s\n", NetInfo.CID);

		tmp = 10;
		NOKIA_GetUnicodeString(s, &tmp, msg.Buffer,name,true);
		smprintf(s, "Network name for phone    : %s\n",DecodeUnicodeString(name));
	}
#endif
	if (Data->RequestID==ID_GetNetworkInfo) {
		Data->NetworkInfo->NetworkName[0] = 0x00;
		Data->NetworkInfo->NetworkName[1] = 0x00;
		Data->NetworkInfo->State 	  = 0;
		switch (msg.Buffer[8]) {
			case 0x00: Data->NetworkInfo->State = GSM_HomeNetwork;		break;
			case 0x01: Data->NetworkInfo->State = GSM_RoamingNetwork;	break;
			case 0x04:
			case 0x06:
			case 0x09: Data->NetworkInfo->State = GSM_NoNetwork;		break;
		}
		if (Data->NetworkInfo->State == GSM_HomeNetwork || Data->NetworkInfo->State == GSM_RoamingNetwork) {
			tmp = 10;
			NOKIA_GetUnicodeString(s, &tmp, msg.Buffer,Data->NetworkInfo->NetworkName,true);
			sprintf(Data->NetworkInfo->LAC,	"%02X%02X", 	msg.Buffer[current+1], msg.Buffer[current+2]);
			sprintf(Data->NetworkInfo->CID, "%02X%02X", 	msg.Buffer[current+5], msg.Buffer[current+6]);
			NOKIA_DecodeNetworkCode(msg.Buffer + (current+7),Data->NetworkInfo->NetworkCode);
		}
	}
	return ERR_NONE;
}

static GSM_Error N6510_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x00, 0x00};

	s->Phone.Data.NetworkInfo=netinfo;
	smprintf(s, "Getting network info\n");
	return GSM_WaitFor (s, req, 5, 0x0a, 4, ID_GetNetworkInfo);
}

static GSM_Error N6510_EncodeSMSFrame(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char *req, GSM_SMSMessageLayout *Layout, int *length)
{
	int			count = 0, pos1, pos2, pos3, pos4, pos5;
	GSM_Error		error;

	memset(Layout,255,sizeof(GSM_SMSMessageLayout));

	req[count++]		 = 0x01;
	if (sms->PDU != SMS_Deliver) {
		req[count++] 	 = 0x02;
	} else {
		req[count++] 	 = 0x00;
	}

	pos1		  	 = count; count++;
	/* firstbyte set in SMS Layout */
	Layout->firstbyte 	 = count; count++;
	if (sms->PDU != SMS_Deliver) {
		Layout->TPMR 	 = count; count++;

		Layout->TPPID	 = count; count++;

		/* TP.DCS set in SMS layout */
		Layout->TPDCS 	 = count; count++;
                req[count++] 	 = 0x00;
	} else {
		Layout->TPPID 	 = count; count++;
		/* TP.DCS set in SMS layout */
		Layout->TPDCS 	 = count; count++;
		Layout->DateTime = count; count += 7;
		req[count++] 	 = 0x55;
		req[count++] 	 = 0x55;
		req[count++] 	 = 0x55;
	}

	/* We have now blocks. Number of blocks = 3 or 4 */
	if (sms->PDU != SMS_Deliver) {
		req[count++] 	 = 0x04;
	} else {
		req[count++] 	 = 0x03;
	}

	/* -------------- Phone number ------------- */
	/* Block type: number */
	req[count++] 		 = 0x82;
	/* Offset to next block */
	req[count++] 		 = 0x10;
 	/* Type of number: default number */
	req[count++] 		 = 0x01;
	pos4 			 = count; count++;
 	/* now coded Number in SMS Layout */
	Layout->Number 		 = count; count+= 12;

	/* -------------- SMSC number -------------- */
	/* Block type: number */
	req[count++] 		 = 0x82;
	/* Offset to next block */
	req[count++] 		 = 0x10;
 	/* Type of number: SMSC number */
	req[count++] 		 = 0x02;
	pos5 			 = count; count++;
	/* now coded SMSC number in SMS Layout */
	Layout->SMSCNumber 	 = count; count += 12;

	/* -------------- SMS validity ------------- */
	if (sms->PDU != SMS_Deliver) {
 		/* Block type: validity */
		req[count++] 	 = 0x08;
		req[count++] 	 = 0x04;
 		/* data length */
		req[count++] 	 = 0x01;
		Layout->TPVP 	 = count; count++;
	}

	/* --------------- SMS text ---------------- */
	/* Block type: SMS text */
	req[count++] 		 = 0x80;
	/* this the same as req[11] but starting from req[42] */
	pos2			 = count; count++;
	pos3			 = count; count++;
 	/* FIXME*/
	Layout->TPUDL 		 = count; count++;
	/* SMS text and UDH coded in SMS Layout */
	Layout->Text 		 = count;

	error = PHONE_EncodeSMSFrame(s,sms,req,*Layout,length,false);
	if (error != ERR_NONE) return error;

	req[pos1] 		 = *length - 1;
	req[pos2] 		 = *length - Layout->Text + 6;
	req[pos3] 		 = *length - Layout->Text;

	/* Convert number of semioctets to number of chars */
	req[pos4]		 = req[Layout->Number] + 4;
	if (req[pos4] % 2) req[pos4]++;
	req[pos4] /= 2;

	req[pos5]		 = req[Layout->SMSCNumber] + 1;

	if (req[pos4]>12 || req[pos5]>12) {
		smprintf(s, "Too long phone number in frame\n");
		return ERR_UNKNOWN;
	}

	return ERR_NONE;
}

static GSM_Error N6510_ReplyGetSMSFolders(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 			j, num = 0;
	size_t pos;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	switch (msg.Buffer[3]) {
	case 0x13:
		smprintf(s, "SMS folders names received\n");
		Data->SMSFolders->Number = msg.Buffer[5]+2;
		pos 			 = 6;
		for (j=0;j<msg.Buffer[5];j++) {
			while (true) {
				if (msg.Buffer[pos]   == msg.Buffer[6] &&
				    msg.Buffer[pos+1] == msg.Buffer[7]) break;
				if (pos+4 > msg.Length) return ERR_UNKNOWNRESPONSE;
				pos++;
			}
			pos+=4;
			smprintf(s, "Folder index: %02x",msg.Buffer[pos - 2]);
			if (msg.Buffer[pos - 1]>GSM_MAX_SMS_FOLDER_NAME_LEN) {
				smprintf(s, "Too long text\n");
				return ERR_UNKNOWNRESPONSE;
			}
			CopyUnicodeString(Data->SMSFolders->Folder[num].Name,msg.Buffer + pos);
			smprintf(s, ", folder name: \"%s\"\n",DecodeUnicodeString(Data->SMSFolders->Folder[num].Name));
			Data->SMSFolders->Folder[num].InboxFolder = false;
			Data->SMSFolders->Folder[num].Memory 	  = MEM_ME;
			if (num == 0x01) { /* OUTBOX SIM */
				Data->SMSFolders->Folder[0].Memory 	= MEM_SM;
				Data->SMSFolders->Folder[0].InboxFolder = true;
				Data->SMSFolders->Folder[0].OutboxFolder = false;

				Data->SMSFolders->Folder[1].Memory 	= MEM_SM;
				Data->SMSFolders->Folder[1].InboxFolder = false;
				Data->SMSFolders->Folder[1].OutboxFolder = true;

				CopyUnicodeString(Data->SMSFolders->Folder[2].Name,Data->SMSFolders->Folder[0].Name);
				Data->SMSFolders->Folder[2].Memory 	= MEM_ME;
				Data->SMSFolders->Folder[2].InboxFolder = true;
				Data->SMSFolders->Folder[2].OutboxFolder = false;

				CopyUnicodeString(Data->SMSFolders->Folder[3].Name,Data->SMSFolders->Folder[1].Name);
				Data->SMSFolders->Folder[3].Memory 	= MEM_ME;
				Data->SMSFolders->Folder[3].InboxFolder = false;
				Data->SMSFolders->Folder[3].OutboxFolder = true;

				num+=2;
			}
			num++;
		}
		return ERR_NONE;
	case 0xf0:
		smprintf(s, "HINT: Maybe phone needs F_SMS_FILES?\n");
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error N6510_GetSMSFoldersS40_30(GSM_StateMachine *s UNUSED, GSM_SMSFolders *folders)
{
	folders->Number=4;
	EncodeUnicode(folders->Folder[0].Name,_("SIM"),strlen(_("SIM")));
	EncodeUnicode(folders->Folder[1].Name,_("Inbox"),strlen(_("Inbox")));
	EncodeUnicode(folders->Folder[2].Name,_("Sent items"),strlen(_("Sent items")));
	EncodeUnicode(folders->Folder[3].Name,_("Saved items"),strlen(_("Saved items")));
	folders->Folder[0].InboxFolder = true;
	folders->Folder[1].InboxFolder = true;
	folders->Folder[2].InboxFolder = false;
	folders->Folder[3].InboxFolder = false;
	folders->Folder[0].OutboxFolder = true;
	folders->Folder[1].OutboxFolder = false;
	folders->Folder[2].OutboxFolder = true;
	folders->Folder[3].OutboxFolder = false;
	folders->Folder[0].Memory      = MEM_SM;
	folders->Folder[1].Memory      = MEM_ME;
	folders->Folder[2].Memory      = MEM_ME;
	folders->Folder[3].Memory      = MEM_ME;
	return ERR_NONE;
}

static GSM_Error N6510_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x12, 0x00, 0x00};

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMS_FILES)) return N6510_GetFilesystemSMSFolders(s,folders);
		return N6510_GetSMSFoldersS40_30(s,folders);
	}

	s->Phone.Data.SMSFolders=folders;
	smprintf(s, "Getting SMS folders\n");
	return GSM_WaitFor (s, req, 6, 0x14, 4, ID_GetSMSFolders);
}

static GSM_Error N6510_ReplyGetSMSFolderStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int			i;
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;

	smprintf(s, "SMS folder status received\n");
	Priv->LastSMSFolder.Number=msg.Buffer[6]*256+msg.Buffer[7];
	smprintf(s, "Number of Entries: %i\n",Priv->LastSMSFolder.Number);
	smprintf(s, "Locations: ");
	for (i=0;i<Priv->LastSMSFolder.Number;i++) {
		Priv->LastSMSFolder.Location[i]=msg.Buffer[8+(i*2)]*256+msg.Buffer[(i*2)+9];
		smprintf(s, "%i ",Priv->LastSMSFolder.Location[i]);
	}
	smprintf(s, "\n");
	NOKIA_SortSMSFolderStatus(s, &Priv->LastSMSFolder);
	return ERR_NONE;
}

static GSM_Error N6510_GetSMSFolderStatus(GSM_StateMachine *s, int folderid)
{
	unsigned char req[] = {N7110_FRAME_HEADER, 0x0C,
			       0x01,		/* 0x01=SIM, 0x02=ME 	*/
			       0x00,		/* Folder ID		*/
			       0x0f, 0x55, 0x55, 0x55};

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
		switch (folderid) {
			case 0x01: req[5] = 0x01; 			break; /* SIM 		*/
			default	 : req[5] = folderid; req[4] = 0x02; 	break; /* ME folders	*/
		}
	} else {
		switch (folderid) {
			case 0x01: req[5] = 0x02; 			 break; /* INBOX SIM 	*/
			case 0x02: req[5] = 0x03; 			 break; /* OUTBOX SIM 	*/
			default	 : req[5] = folderid - 1; req[4] = 0x02; break; /* ME folders	*/
		}
	}

	smprintf(s, "Getting SMS folder status\n");
	return GSM_WaitFor (s, req, 10, 0x14, 4, ID_GetSMSFolderStatus);
}

static void N6510_GetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char *folderid, unsigned int *location)
{
	int ifolderid;

	/* simulate flat SMS memory */
	if (sms->Folder==0x00) {
		ifolderid = sms->Location / GSM_PHONE_MAXSMSINFOLDER;
		*folderid = ifolderid + 0x01;
		*location = sms->Location - ifolderid * GSM_PHONE_MAXSMSINFOLDER;
		if (*folderid == 0x1B) (*folderid)=0x99; /* 0x1A is Outbox in 6230i */
	} else {
		*folderid = sms->Folder;
		*location = sms->Location;
		if (*folderid == 0x1A) (*folderid)=0x99; /* 0x1A is Outbox in 6230i */
	}
	smprintf(s, "SMS folder %i & location %i -> 6510 folder %i & location %i\n",
		sms->Folder,sms->Location,*folderid,*location);
}

static void N6510_SetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char folderid, int location)
{
	sms->Folder	= 0;
	sms->Location	= (folderid - 0x01) * GSM_PHONE_MAXSMSINFOLDER + location;
	smprintf(s, "6510 folder %i & location %i -> SMS folder %i & location %i\n",
		folderid,location,sms->Folder,sms->Location);
}

static GSM_Error N6510_DecodeSMSFrame(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char *buffer, size_t *current2)
{
	int 			i, current, blocks=0, SMSTemplateDateTime = 0;
	GSM_SMSMessageLayout 	Layout;
	GSM_Error		error;

	memset(&Layout,255,sizeof(GSM_SMSMessageLayout));
	Layout.firstbyte = 2;
	switch (buffer[0]) {
	case 0x00:
		smprintf(s, "SMS deliver\n");
		sms->PDU 	= SMS_Deliver;
		Layout.TPPID 	= 3;
		Layout.TPDCS 	= 4;
		Layout.DateTime = 5;
		blocks 		= 15;
		break;
	case 0x01:
		smprintf(s, "Delivery report\n");
		sms->PDU = SMS_Status_Report;
		Layout.TPMR	= 3;
		Layout.TPStatus	= 4;
		Layout.DateTime = 5;
		Layout.SMSCTime = 12;
		blocks 		= 19;
		break;
	case 0x02:
		smprintf(s, "SMS template\n");
		sms->PDU 	= SMS_Submit;
		Layout.TPMR	= 3;
		Layout.TPPID 	= 4;
		Layout.TPDCS 	= 5;
		blocks 		= 7;
		break;
	}
	current = blocks + 1;
	for (i=0;i<buffer[blocks];i++) {
		switch (buffer[current]) {
		case 0x08:
			smprintf(s, "SMSC timestamp (ignored)\n");
			break;
		case 0x80:
			smprintf(s, "SMS text\n");
			if (buffer[current + 2] > buffer[current + 3]) {
				Layout.TPUDL 	= current + 2;
			} else {
				Layout.TPUDL 	= current + 3;
			}
			Layout.Text 		= current + 4;
			break;
		case 0x82:
			switch (buffer[current+2]) {
			case 0x01:
				smprintf(s, "Phone number\n");
				Layout.Number = current + 4;
				break;
			case 0x02:
				smprintf(s, "SMSC number\n");
				Layout.SMSCNumber = current + 4;
				break;
			default:
				smprintf(s, "Unknown number\n");
				break;
			}
			break;
		case 0x84:
			smprintf(s, "Date and time of saving for SMS template\n");
			SMSTemplateDateTime = current + 2;
			break;
		default:
			smprintf(s, "Unknown block %02x\n",buffer[current]);
		}
		current = current + buffer[current + 1];
	}
	error = GSM_DecodeSMSFrame(sms,buffer,Layout);
	if (SMSTemplateDateTime != 0) {
		sms->PDU = SMS_Deliver;
		NOKIA_DecodeDateTime(s, buffer+SMSTemplateDateTime, &sms->DateTime);
		sms->DateTime.Timezone = 0;
	}
	(*current2) = current;
	return error;
}

GSM_Error N6510_ReplyGetSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	size_t			i,j;
	size_t			Width, Height;
	unsigned char		output[500]; /* output2[500]; */
	GSM_Phone_Data		*Data = &s->Phone.Data;
	GSM_Error		error;

	switch(msg.Buffer[3]) {
	case 0x03:
		smprintf(s, "SMS Message received\n");
		Data->GetSMSMessage->Number=1;
		NOKIA_DecodeSMSState(s, msg.Buffer[5], &(Data->GetSMSMessage->SMS[0]));
		switch (msg.Buffer[14]) {
		case 0x00:
		case 0x01:
		case 0x02:
			if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
				Data->GetSMSMessage->Number=0;
				i = 14;
				while (true) {
					error = N6510_DecodeSMSFrame(s, &Data->GetSMSMessage->SMS[Data->GetSMSMessage->Number],msg.Buffer+i,&j);
					if (error != ERR_NONE) return error;
					NOKIA_DecodeSMSState(s, msg.Buffer[5], &Data->GetSMSMessage->SMS[Data->GetSMSMessage->Number]);
					i+=j;
					Data->GetSMSMessage->Number++;
					if (i>=msg.Length) break;
				}
				return error;
			} else {
				return N6510_DecodeSMSFrame(s, &Data->GetSMSMessage->SMS[0],msg.Buffer+14,&j);
			}
		case 0xA0:
			smprintf(s, "Picture Image\n");
			Data->GetSMSMessage->Number = 0;
			i = 0;
			output[i++] = 0x30;	 /* Smart Messaging 3.0 */
			output[i++] = SM30_OTA;
			output[i++] = 0x01;	 /* Length */
			output[i++] = 0x00;	 /* Length */
			output[i++] = 0x00;
			PHONE_GetBitmapWidthHeight(GSM_NokiaPictureImage, &Width, &Height);
			output[i++] = Width;
			output[i++] = Height;
			output[i++] = 0x01;
			memcpy(output+i,msg.Buffer+30,PHONE_GetBitmapSize(GSM_NokiaPictureImage,0,0));
			i = i + PHONE_GetBitmapSize(GSM_NokiaPictureImage,0,0);
#if 0
 			if (msg.Length!=282) {
 				output[i++] = SM30_UNICODETEXT;
 				output[i++] = 0;
 				output[i++] = 0; /* Length - later changed */
 				GSM_UnpackEightBitsToSeven(0, msg.Length-282, msg.Length-304, msg.Buffer+282,output2);
 				DecodeDefault(output+i, output2, msg.Length - 282, true);
 				output[i - 1] = UnicodeLength(output+i) * 2;
 				i = i + output[i-1];
 			}
#endif
			GSM_MakeMultiPartSMS(Data->GetSMSMessage,output,i,UDH_NokiaProfileLong,SMS_Coding_8bit,1,0);
			for (i=0;i<3;i++) {
                		Data->GetSMSMessage->SMS[i].Number[0]=0;
                		Data->GetSMSMessage->SMS[i].Number[1]=0;
			}
			if (Data->Bitmap != NULL) {
				Data->Bitmap->Location	= 0;
				PHONE_GetBitmapWidthHeight(GSM_NokiaPictureImage, &Width, &Height);
				Data->Bitmap->BitmapWidth	= Width;
				Data->Bitmap->BitmapHeight	= Height;
				PHONE_DecodeBitmap(GSM_NokiaPictureImage, msg.Buffer + 30, Data->Bitmap);
				Data->Bitmap->Sender[0] = 0x00;
				Data->Bitmap->Sender[1] = 0x00;
				Data->Bitmap->Text[0] = 0;
				Data->Bitmap->Text[1] = 0;
			}
			return ERR_NONE;
		default:
			smprintf(s, "Unknown SMS type: %i\n",msg.Buffer[8]);
		}
		break;
	case 0x0f:
		smprintf(s, "SMS message info received\n");
		CopyUnicodeString(Data->GetSMSMessage->SMS[0].Name,msg.Buffer+52);
		smprintf(s, "Name: \"%s\"\n",DecodeUnicodeString(Data->GetSMSMessage->SMS[0].Name));
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_PrivGetSMSMessageBitmap(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, GSM_Bitmap *bitmap)
{
	GSM_Error		error;
	unsigned char		folderid,namebuffer[200];
	unsigned int		location;
	int			i;
	unsigned char req[] = {
		N6110_FRAME_HEADER,
		0x02,		/* msg type: 0x02 for getting sms, 0x0e for sms status */
		0x01,		/* 0x01=SIM, 0x02=ME 	*/
		0x00, 		/* FolderID */
		0x00, 0x02,	/* Location */
		0x01, 0x00};

	N6510_GetSMSLocation(s, &sms->SMS[0], &folderid, &location);

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
		switch (folderid) {
			case 0x01: req[5] = 0x01; 			 break; /* SIM 		*/
			default	 : req[5] = folderid; req[4] = 0x02; 	 break; /* ME folders	*/
		}
	} else {
		switch (folderid) {
			case 0x01: req[5] = 0x02; 			 break; /* INBOX SIM 	*/
			case 0x02: req[5] = 0x03; 			 break; /* OUTBOX SIM 	*/
			default	 : req[5] = folderid - 1; req[4] = 0x02; break; /* ME folders	*/
		}
	}

	req[6]=location / 256;
	req[7]=location % 256;

	s->Phone.Data.GetSMSMessage 	= sms;
	s->Phone.Data.Bitmap 		= bitmap;
	smprintf(s, "Getting sms message info\n");
	req[3] = 0x0e; req[8] = 0x55; req[9] = 0x55;
	error=GSM_WaitFor (s, req, 10, 0x14, 4, ID_GetSMSMessage);
	if (error!=ERR_NONE) return error;
	CopyUnicodeString(namebuffer,sms->SMS[0].Name);

	smprintf(s, "Getting sms\n");
	req[3] = 0x02; req[8] = 0x01; req[9] = 0x00;
	error=GSM_WaitFor (s, req, 10, 0x14, 4, ID_GetSMSMessage);
	if (error==ERR_NONE) {
		for (i=0;i<sms->Number;i++) {
			N6510_SetSMSLocation(s, &sms->SMS[i], folderid, location);
			sms->SMS[i].Folder 	= folderid;

			if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
				sms->SMS[i].InboxFolder = true;
				if (folderid > 2) sms->SMS[i].InboxFolder = false;
				sms->SMS[i].Memory = MEM_ME;
				if (folderid == 0x01) sms->SMS[i].Memory = MEM_SM;
			} else {
				sms->SMS[i].InboxFolder = true;
				if (folderid != 0x01 && folderid != 0x03) sms->SMS[i].InboxFolder = false;
				sms->SMS[i].Memory	= MEM_ME;
				if (folderid == 0x01 || folderid == 0x02) sms->SMS[i].Memory = MEM_SM;
			}

			CopyUnicodeString(sms->SMS[i].Name,namebuffer);
		}
	}
	return error;
}

static GSM_Error N6510_GetSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	GSM_Error		error;
	unsigned char		folderid;
	unsigned int		location;
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	int			i;
	bool			found = false;

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMS_FILES)) return ERR_NOTSUPPORTED;
	}

	/* Clear SMS structure of any possible junk */
	GSM_SetDefaultReceivedSMSData(&(sms->SMS[0]));

	N6510_GetSMSLocation(s, &(sms->SMS[0]), &folderid, &location);
	error=N6510_GetSMSFolderStatus(s, folderid);
	if (error!=ERR_NONE) return error;
	for (i=0;i<Priv->LastSMSFolder.Number;i++) {
		if (Priv->LastSMSFolder.Location[i]==location) {
			found = true;
			break;
		}
	}
	if (!found) return ERR_EMPTY;
	return N6510_PrivGetSMSMessageBitmap(s,sms,NULL);
}

static GSM_Error N6510_GetNextSMSMessageBitmap(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start, GSM_Bitmap *bitmap)
{
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	unsigned char		folderid;
	unsigned int		location;
	GSM_Error		error;
	int			i;
	bool			findnextfolder = false;

	if (start) {
		folderid	= 0x00;
		findnextfolder	= true;
		error=N6510_GetSMSFolders(s,&Priv->LastSMSFolders);
		if (error!=ERR_NONE) return error;
	} else {
		N6510_GetSMSLocation(s, &(sms->SMS[0]), &folderid, &location);
		for (i=0;i<Priv->LastSMSFolder.Number;i++) {
			if (Priv->LastSMSFolder.Location[i]==location) break;
		}
		/* Is this last location in this folder ? */
		if (i==Priv->LastSMSFolder.Number-1) {
			findnextfolder=true;
		} else {
			location=Priv->LastSMSFolder.Location[i+1];
		}
	}
	if (findnextfolder) {
		Priv->LastSMSFolder.Number=0;
		while (Priv->LastSMSFolder.Number==0) {
			folderid++;
			/* Too high folder number */
			if ((folderid-1)>=Priv->LastSMSFolders.Number) return ERR_EMPTY;
			/* Get next folder status */
			error=N6510_GetSMSFolderStatus(s, folderid);
			if (error!=ERR_NONE) return error;
			/* First location from this folder */
			location=Priv->LastSMSFolder.Location[0];
		}
	}
	N6510_SetSMSLocation(s, &sms->SMS[0], folderid, location);

	return N6510_PrivGetSMSMessageBitmap(s, sms, bitmap);
}

static GSM_Error N6510_GetNextSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30) &&
			GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMS_FILES)) {
		return N6510_GetNextFilesystemSMS(s,sms,start);
	}

	return N6510_GetNextSMSMessageBitmap(s, sms, start, NULL);
}

static GSM_Error N6510_ReplyStartupNoteLogo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	if (Data->RequestID == ID_GetBitmap) {
		switch (msg.Buffer[4]) {
		case 0x01:
			smprintf(s, "Welcome note text received\n");
			CopyUnicodeString(Data->Bitmap->Text,msg.Buffer+6);
			smprintf(s, "Text is \"%s\"\n",DecodeUnicodeString(Data->Bitmap->Text));
			return ERR_NONE;
		case 0x10:
			smprintf(s, "Dealer note text received\n");
			CopyUnicodeString(Data->Bitmap->Text,msg.Buffer+6);
			smprintf(s, "Text is \"%s\"\n",DecodeUnicodeString(Data->Bitmap->Text));
			return ERR_NONE;
		case 0x0f:
			smprintf(s, "Startup logo received\n");
			PHONE_DecodeBitmap(GSM_Nokia7110StartupLogo, msg.Buffer + 22, Data->Bitmap);
			return ERR_NONE;
		}
	}
	if (Data->RequestID == ID_SetBitmap) {
		switch (msg.Buffer[4]) {
			case 0x01:
			case 0x10:
			case 0x0f:
			case 0x25:
				return ERR_NONE;
		}
	}
	return ERR_UNKNOWN;
}

static GSM_Error N6510_GetPictureImage(GSM_StateMachine *s, GSM_Bitmap *Bitmap, int *location)
{
	GSM_MultiSMSMessage 	sms;
	int			Number;
	GSM_Bitmap		bitmap;
	GSM_Error		error;

	sms.SMS[0].Folder	= 0;
	Number			= 0;
	bitmap.Location		= 255;
	error=N6510_GetNextSMSMessageBitmap(s, &sms, true, &bitmap);
	while (error == ERR_NONE) {
		if (bitmap.Location != 255) {
			Number++;
			if (Number == Bitmap->Location) {
				bitmap.Location = Bitmap->Location;
				memcpy(Bitmap,&bitmap,sizeof(GSM_Bitmap));
				*location = sms.SMS[0].Location;
				return ERR_NONE;
			}
		}
		bitmap.Location		= 255;
		sms.SMS[0].Folder 	= 0;
		error=N6510_GetNextSMSMessageBitmap(s, &sms, false, &bitmap);
	}
	return ERR_INVALIDLOCATION;
}

static GSM_Error N6510_GetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	unsigned char reqOp	[] = {N6110_FRAME_HEADER, 0x23, 0x00, 0x00, 0x55, 0x55, 0x55};
	unsigned char reqStartup[] = {N6110_FRAME_HEADER, 0x02, 0x0f};
	unsigned char reqNote	[] = {N6110_FRAME_HEADER, 0x02, 0x01, 0x00};
	GSM_MemoryEntry	pbk;
	GSM_Error		error;
	int			Location;

	s->Phone.Data.Bitmap=Bitmap;
	switch (Bitmap->Type) {
	case GSM_StartupLogo:
		Bitmap->BitmapWidth  = 96;
		Bitmap->BitmapHeight = 65;
		GSM_ClearBitmap(Bitmap);
		smprintf(s, "Getting startup logo\n");
		return GSM_WaitFor (s, reqStartup, 5, 0x7A, 4, ID_GetBitmap);
	case GSM_DealerNote_Text:
		reqNote[4] = 0x10;
		smprintf(s, "Getting dealer note\n");
		return GSM_WaitFor (s, reqNote, 6, 0x7A, 4, ID_GetBitmap);
	case GSM_WelcomeNote_Text:
		smprintf(s, "Getting welcome note\n");
		return GSM_WaitFor (s, reqNote, 6, 0x7A, 4, ID_GetBitmap);
	case GSM_CallerGroupLogo:
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_PBK35)) return ERR_NOTSUPPORTED;
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_6230iCALLER)) {
			pbk.MemoryType	= MEM6510_CG2;
			pbk.Location	= Bitmap->Location;
			smprintf(s, "Getting caller group logo method 2\n");
			return N6510_GetMemory(s,&pbk);
		}
		Bitmap->BitmapWidth  	 = 72;
		Bitmap->BitmapHeight 	 = 14;
		GSM_ClearBitmap(Bitmap);
		pbk.MemoryType	= MEM7110_CG;
		pbk.Location	= Bitmap->Location;
		smprintf(s, "Getting caller group logo\n");
		error=N6510_GetMemory(s,&pbk);
		if (error==ERR_NONE) NOKIA_GetDefaultCallerGroupName(Bitmap);
		return error;
	case GSM_OperatorLogo:
		smprintf(s, "Getting operator logo\n");
		return GSM_WaitFor (s, reqOp, 9, 0x0A, 4, ID_GetBitmap);
	case GSM_PictureImage:
		return N6510_GetPictureImage(s, Bitmap, &Location);
	default:
		break;
	}
	return ERR_NOTSUPPORTED;
}

static GSM_Error N6510_ReplyGetIncSignalQuality(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Network level changed to: %i\n",msg.Buffer[4]);
	return ERR_NONE;
}

static GSM_Error N6510_ReplyGetSignalQuality(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Network level received: %i\n",msg.Buffer[8]);
    	Data->SignalQuality->SignalStrength 	= -1;
    	Data->SignalQuality->SignalPercent 	= ((int)msg.Buffer[8]);
    	Data->SignalQuality->BitErrorRate 	= -1;
	return ERR_NONE;
}

static GSM_Error N6510_GetSignalQuality(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x0B, 0x00, 0x02, 0x00, 0x00, 0x00};

	s->Phone.Data.SignalQuality = sig;
	smprintf(s, "Getting network level\n");
	return GSM_WaitFor (s, req, 9, 0x0a, 4, ID_GetSignalQuality);
}

static GSM_Error N6510_IncomingBatteryCharge(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Incoming battery level received???: %i\n",
			msg.Buffer[9]*100/7);
	return ERR_NONE;
}

static GSM_Error N6510_ReplyGetBatteryCharge(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

    	Data->BatteryCharge->BatteryPercent 	= ((int)(msg.Buffer[9]*100/7));
    	Data->BatteryCharge->ChargeState 	= 0;

	smprintf(s, "Battery level received: %i\n",
			Data->BatteryCharge->BatteryPercent);

	return ERR_NONE;
}

static GSM_Error N6510_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x0A, 0x02, 0x00};

	GSM_ClearBatteryCharge(bat);
	s->Phone.Data.BatteryCharge = bat;
	smprintf(s, "Getting battery level\n");
	return GSM_WaitFor (s, req, 6, 0x17, 4, ID_GetBatteryCharge);
}

static GSM_Error N6510_ReplyGetWAPBookmark(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return DCT3DCT4_ReplyGetWAPBookmark (msg, s, true);
}

static GSM_Error N6510_ReplyGetOperatorLogo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	if (msg.Buffer[3] == 0xf0) return ERR_NOTSUPPORTED;

	smprintf(s, "Operator logo received\n");
	if (msg.Length == 18) return ERR_EMPTY;
	NOKIA_DecodeNetworkCode(msg.Buffer+12,Data->Bitmap->NetworkCode);
	smprintf(s, "Network code %s\n",Data->Bitmap->NetworkCode);
	Data->Bitmap->BitmapWidth	= msg.Buffer[20];
	Data->Bitmap->BitmapHeight	= msg.Buffer[21];
	PHONE_DecodeBitmap(GSM_Nokia6510OperatorLogo,msg.Buffer+26,Data->Bitmap);
	return ERR_NONE;
}

GSM_Error N6510_ReplyDeleteMemory(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Phonebook entry deleted\n");
	return ERR_NONE;
}

GSM_Error N6510_DeleteMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	unsigned char req[] = {N7110_FRAME_HEADER, 0x0f, 0x55, 0x01,
			       0x04, 0x55, 0x00, 0x10, 0xFF, 0x02,
			       0x00, 0x01,		/* location	*/
			       0x00, 0x00, 0x00, 0x00,
			       0x05, 			/* memory type	*/
			       0x55, 0x55, 0x55};

	req[12] = entry->Location / 256;
	req[13] = entry->Location % 256;

	req[18] = NOKIA_GetMemoryType(s, entry->MemoryType,N71_65_MEMORY_TYPES);
	if (req[18]==0xff) return ERR_NOTSUPPORTED;

	smprintf(s, "Deleting phonebook entry\n");
	return GSM_WaitFor (s, req, 22, 0x03, 4, ID_SetMemory);
}

static GSM_Error N6510_SetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	int 		count = 22;
	size_t blocks;
	unsigned char 	req[5000] = {
		N7110_FRAME_HEADER, 0x0b, 0x00, 0x01, 0x01, 0x00, 0x00, 0x10, 0x02,
		0x00,  			/* memory type */
		0x00, 0x00,  		/* location */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00};			/* Number of blocks */
	GSM_Error error;
	GSM_MemoryEntry tmp;
	GSM_MemoryStatus status;
	GSM_Phone_N6510Data *Priv = &s->Phone.Data.Priv.N6510;

	if (entry->Location == 0) {
		/*
		 * We want to remember last location so that we don't check the
		 * same place again and again.
		 */
		if (Priv->LastFreeMemoryType != entry->MemoryType) {
			Priv->LastFreeMemoryLocation = 0;
			Priv->LastFreeMemoryType = entry->MemoryType;
			status.MemoryType = entry->MemoryType;
			error = N6510_GetMemoryStatus(s, &status);
			if (error != ERR_NONE) return error;
			Priv->LastFreeMemorySize = status.MemoryUsed + status.MemoryFree;
		}

		/* Advance beyond last used location */
		tmp.MemoryType = entry->MemoryType;
		error = ERR_NONE;
		for (tmp.Location = Priv->LastFreeMemoryLocation + 1;
			tmp.Location < Priv->LastFreeMemorySize;
			tmp.Location++) {
			error = N6510_GetMemory(s, &tmp);
			if (error != ERR_NONE) break;
		}
		if (error == ERR_NONE) {
			/* Memory full */
			return ERR_FULL;
		} else if (error != ERR_EMPTY) {
			/* Other failure */
			return error;
		}
		/* We've got the location */
		entry->Location = tmp.Location;
		smprintf(s, "Found empty location: %d\n", entry->Location);
	}

	req[11] = NOKIA_GetMemoryType(s, entry->MemoryType,N71_65_MEMORY_TYPES);
	if (req[11]==0xff) return ERR_NOTSUPPORTED;

	req[12] = entry->Location / 256;
	req[13] = entry->Location % 256;

	count = count + N71_65_EncodePhonebookFrame(s, req+22, entry, &blocks, true, GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_VOICETAGS));
	req[21] = blocks;

	smprintf(s, "Writing phonebook entry\n");
	return GSM_WaitFor (s, req, count, 0x03, 4, ID_SetMemory);
}

static GSM_Error N6510_AddMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	entry->Location = 0;
	return N6510_SetMemory(s, entry);
}

static GSM_Error N6510_ReplySetOperatorLogo(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Operator logo set OK\n");
	return ERR_NONE;
}

static GSM_Error N6510_SetCallerLogo(GSM_StateMachine *s, GSM_Bitmap *bitmap)
{
	char		string[500];
	int		block=0, i;
	size_t Width, Height;
	unsigned int 	count = 22;
	unsigned char 	req[500] = {
		N6110_FRAME_HEADER, 0x0b, 0x00, 0x01, 0x01, 0x00, 0x00, 0x10,
		0xfe, 0x00,		/* memory type */
		0x00, 0x00,		/* location */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	/* Set memory type */
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_6230iCALLER)) {
		req[11] = MEM6510_CG2;
	} else {
		req[11] = MEM7110_CG;
	}

	req[13] = bitmap->Location;

	/* Enabling/disabling logo */
	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_6230iCALLER)) {
		string[0] = bitmap->BitmapEnabled?1:0;
		string[1] = 0;
		count += N71_65_PackPBKBlock(s, N7110_PBK_LOGOON, 2, block++, string, req + count);
	}

	/* Ringtone */
	if (!bitmap->DefaultRingtone) {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_6230iCALLER)) {
			string[0] = 0x00;
			string[1] = 0x00;
			string[2] = 0x00;
			string[3] = 0x10;
			string[4] = 0x00;
			string[5] = 0x00;
			string[6] = bitmap->RingtoneID;
			string[7] = 0x00;
			string[8] = 0x00;
			string[9] = 0x00;
			count += N71_65_PackPBKBlock(s, N6510_PBK_RINGTONEFILE_ID, 10, block++, string, req + count);
			req[count - 1] = 0x01;
		} else if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_PBKTONEGAL)) {
			/* do nothing ? */
		} else {
			string[0] = 0x00;
			string[1] = 0x00;
			string[2] = bitmap->RingtoneID;
			count += N71_65_PackPBKBlock(s, N7110_PBK_RINGTONE_ID, 3, block++, string, req + count);
			count --;
			req[count-5] = 8;
		}
	}

	/* Number of group */
	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_6230iCALLER)) {
		string[0] = bitmap->Location;
		string[1] = 0;
		count += N71_65_PackPBKBlock(s, N7110_PBK_GROUP, 2, block++, string, req + count);
	}

	/* Name */
	if (!bitmap->DefaultName) {
		i = UnicodeLength(bitmap->Text) * 2;
		string[0] = i + 2;
		memcpy(string + 1, bitmap->Text, i);
		string[i + 1] = 0;
		count += N71_65_PackPBKBlock(s, N7110_PBK_NAME, i + 2, block++, string, req + count);
	}

	/* Logo */
	if (!bitmap->DefaultBitmap) {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_6230iCALLER)) {
			/* write N6510_PBK_PICTURE_ID ? */
		} else {
			PHONE_GetBitmapWidthHeight(GSM_NokiaCallerLogo, &Width, &Height);
			string[0] = Width;
			string[1] = Height;
			string[2] = 0;
			string[3] = 0;
			string[4] = PHONE_GetBitmapSize(GSM_NokiaCallerLogo,0,0);
			PHONE_EncodeBitmap(GSM_NokiaCallerLogo, string + 5, bitmap);
			count += N71_65_PackPBKBlock(s, N7110_PBK_GROUPLOGO, PHONE_GetBitmapSize(GSM_NokiaCallerLogo,0,0) + 5, block++, string, req + count);
		}
	}

	req[21] = block;

	return GSM_WaitFor (s, req, count, 0x03, 4, ID_SetBitmap);
}

static GSM_Error N6510_ReplySetPicture(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Picture Image written OK, folder %i, location %i\n",msg.Buffer[4],msg.Buffer[5]*256+msg.Buffer[6]);
	return ERR_NONE;
}

static GSM_Error N6510_SetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	GSM_SMSMessage		sms;
	GSM_Phone_Bitmap_Types	Type;
	size_t			Width, Height, i, count;
	unsigned char		folderid;
	unsigned int		location;
	GSM_NetworkInfo 	NetInfo;
	GSM_Error		error;
	unsigned char reqStartup[1000] = {
		N7110_FRAME_HEADER, 0x04, 0x0F,
		0x00, 0x00, 0x00,
		0x04, 0xC0, 0x02, 0x00,
		0x41, 0xC0, 0x03, 0x00,
		0x60, 0xC0, 0x04};
	unsigned char reqColourWallPaper[200] = {
		N6110_FRAME_HEADER, 0x07, 0x00, 0x00, 0x00, 0xD5,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x01, 0x00,
		0x18};		/* Bitmap ID */
	unsigned char reqColourStartup[200] = {
		N6110_FRAME_HEADER, 0x04, 0x25, 0x00, 0x01, 0x00, 0x18};
	unsigned char reqOp[1000] = {
		N7110_FRAME_HEADER, 0x25, 0x01,
		0x55, 0x00, 0x00, 0x55,
		0x01,			/* 0x01 - not set, 0x02 - set */
		0x0C, 0x08,
		0x62, 0xF0, 0x10,	/* Network code */
		0x03, 0x55, 0x55};
	unsigned char reqColourOp[200] = {
		N6110_FRAME_HEADER,
		0x07, 0x00, 0x00, 0x00, 0xE7, 0x00, 0x00, 0x00, 0xF9, 0x00,
		0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00,
		0x18,			/* File ID */
		0x00,
		0x00, 0x00, 0x00};	/* Network code */
	unsigned char reqNote[200] = {N6110_FRAME_HEADER, 0x04, 0x01};
	unsigned char reqPicture[2000] = {
		N6110_FRAME_HEADER, 0x00,
		0x02, 0x05,		/* SMS folder 	*/
		0x00, 0x00,		/* location 	*/
		0x01, 0x01, 0xa0, 0x02, 0x01, 0x40, 0x00, 0x34,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x55, 0x55, 0x55, 0x03, 0x82, 0x10,
		0x01, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x10,
		0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x04,
		0x00, 0x00, 0xa1, 0x55, 0x01, 0x08, 0x00, 0x00,
		0x00, 0x01, 0x48, 0x1c, 0x00, 0xfc, 0x00};

	switch (Bitmap->Type) {
	case GSM_ColourWallPaper_ID:
		reqColourWallPaper[21] = Bitmap->ID;
		smprintf(s, "Setting colour wall paper\n");
		return GSM_WaitFor (s, reqColourWallPaper, 22, 0x43, 4, ID_SetBitmap);
	case GSM_StartupLogo:
		Type = GSM_Nokia7110StartupLogo;
		switch (Bitmap->Location) {
			case 1:	 PHONE_EncodeBitmap(Type, reqStartup + 22, Bitmap);
				 break;
			case 2:  memset(reqStartup+5,0x00,15);
				 PHONE_ClearBitmap(Type, reqStartup + 22,0,0);
				 break;
			default: return ERR_NOTSUPPORTED;
		}
		smprintf(s, "Setting startup logo\n");
		return GSM_WaitFor (s, reqStartup, 22+PHONE_GetBitmapSize(Type,0,0), 0x7A, 4, ID_SetBitmap);
	case GSM_DealerNote_Text:
		reqNote[4] = 0x10;
		CopyUnicodeString(reqNote + 5, Bitmap->Text);
		i = 6 + UnicodeLength(Bitmap->Text) * 2;
		reqNote[i++] 	= 0;
		reqNote[i] 	= 0;
		return GSM_WaitFor (s, reqNote, i, 0x7A, 4, ID_SetBitmap);
	case GSM_WelcomeNote_Text:
		CopyUnicodeString(reqNote + 5, Bitmap->Text);
		i = 6 + UnicodeLength(Bitmap->Text) * 2;
		reqNote[i++] 	= 0;
		reqNote[i] 	= 0;
		return GSM_WaitFor (s, reqNote, i, 0x7A, 4, ID_SetBitmap);
	case GSM_OperatorLogo:
		/* We want to set operator logo, not clear */
		if (strcmp(Bitmap->NetworkCode,"000 00")) {
			memset(reqOp + 19, 0, 281);
			NOKIA_EncodeNetworkCode(reqOp+12, Bitmap->NetworkCode);
			Type = GSM_Nokia6510OperatorLogo;
			reqOp[9]  = 0x02;	/* Logo enabled */
			reqOp[18] = 0x1a;	/* FIXME */
			reqOp[19] = PHONE_GetBitmapSize(Type,0,0) + 8 + 29 + 2;
			PHONE_GetBitmapWidthHeight(Type, &Width, &Height);
			reqOp[20] = Width;
			reqOp[21] = Height;
			reqOp[22] = 0x00;
			reqOp[23] = PHONE_GetBitmapSize(Type,0,0) + 29;
			reqOp[24] = 0x00;
			reqOp[25] = PHONE_GetBitmapSize(Type,0,0) + 29;
			PHONE_EncodeBitmap(Type, reqOp + 26, Bitmap);
			smprintf(s, "Setting operator logo\n");
			return GSM_WaitFor (s, reqOp, reqOp[19]+reqOp[11]+10, 0x0A, 4, ID_SetBitmap);
		} else {
			error=N6510_GetNetworkInfo(s,&NetInfo);
			if (error != ERR_NONE) return error;
			NOKIA_EncodeNetworkCode(reqOp+12, NetInfo.NetworkCode);
			smprintf(s, "Clearing operator logo\n");
			return GSM_WaitFor (s, reqOp, 18, 0x0A, 4, ID_SetBitmap);
		}
	case GSM_ColourOperatorLogo_ID:
		/* We want to set operator logo, not clear */
		if (strcmp(Bitmap->NetworkCode,"000 00")) {
			EncodeBCD(reqColourOp+23, Bitmap->NetworkCode, 6, false);
			reqColourOp[21] = Bitmap->ID;
		}
		smprintf(s, "Setting colour operator logo\n");
		return GSM_WaitFor (s, reqColourOp, 26, 0x43, 4, ID_SetBitmap);
	case GSM_ColourStartupLogo_ID:
		switch (Bitmap->Location) {
			case 0: reqColourStartup[6] = 0x00;
				reqColourStartup[8] = 0x00;
				smprintf(s, "Setting colour startup logo\n");
				return GSM_WaitFor (s, reqColourStartup, 9, 0x7A, 4, ID_SetBitmap);
			case 1: reqColourStartup[8] = Bitmap->ID;
				smprintf(s, "Setting colour startup logo\n");
				return GSM_WaitFor (s, reqColourStartup, 9, 0x7A, 4, ID_SetBitmap);
			default:return ERR_NOTSUPPORTED;
		}
	case GSM_CallerGroupLogo:
		return N6510_SetCallerLogo(s,Bitmap);
	case GSM_PictureImage:
		error = N6510_GetPictureImage(s, Bitmap, &sms.Location);
		if (error == ERR_NONE) {
			sms.Folder = 0;
			N6510_GetSMSLocation(s, &sms, &folderid, &location);
			switch (folderid) {
				case 0x01: reqPicture[5] = 0x02; 				break; /* INBOX SIM 	*/
				case 0x02: reqPicture[5] = 0x03; 				break; /* OUTBOX SIM 	*/
				default	 : reqPicture[5] = folderid - 1; reqPicture[4] = 0x02; 	break; /* ME folders	*/
			}
			reqPicture[6]=location / 256;
			reqPicture[7]=location;
		}
		Type = GSM_NokiaPictureImage;
		count = 78;
		PHONE_EncodeBitmap(Type, reqPicture + count, Bitmap);
		count += PHONE_GetBitmapSize(Type,0,0);
		smprintf(s, "Setting Picture Image\n");
		return GSM_WaitFor (s, reqPicture, count, 0x14, 4, ID_SetBitmap);
	default:
		break;
	}
	return ERR_NOTSUPPORTED;
}

static GSM_Error N6510_ReplyGetRingtoneID(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_N6510Data *Priv = &s->Phone.Data.Priv.N6510;

	smprintf(s, "Ringtone ID received\n");
	Priv->RingtoneID = msg.Buffer[15];
	return ERR_NONE;
}

static GSM_Error N6510_ReplySetBinRingtone(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Binary ringtone set\n");
	return ERR_NONE;
}

static GSM_Error N6510_SetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, int *maxlength)
{
	GSM_Error		error;
	GSM_Phone_N6510Data 	*Priv = &s->Phone.Data.Priv.N6510;
	GSM_NetworkInfo		NetInfo;
	size_t			size=200, current;
	unsigned char 		GetIDReq[] = {
		N7110_FRAME_HEADER, 0x01, 0x00, 0x00,
		0x00, 0xFF, 0x06, 0xE1, 0x00,
		0xFF, 0x06, 0xE1, 0x01, 0x42};
	unsigned char		SetPreviewReq[1000] = {
		0xAE,		/* Ringtone ID */
		0x01, 0x00, 0x0D, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00,
		0x00};  	/*Length*/
	unsigned char		AddBinaryReq[33000] = {
		N7110_FRAME_HEADER, 0x0E, 0x7F, 0xFF, 0xFE};

	if (Ringtone->Format == RING_NOTETONE && Ringtone->Location==255)
	{
		smprintf(s, "Getting ringtone ID\n");
		error=GSM_WaitFor (s, GetIDReq, 14, 0xDB, 4, ID_SetRingtone);
		if (error != ERR_NONE) return error;
		*maxlength=GSM_EncodeNokiaRTTLRingtone(*Ringtone, SetPreviewReq+11, &size);
		SetPreviewReq[0]  = Priv->RingtoneID;
		SetPreviewReq[10] = size;
		smprintf(s, "Setting ringtone\n");
		error = s->Protocol.Functions->WriteMessage(s, SetPreviewReq, size+11, 0x00);
		if (error!=ERR_NONE) return error;
		sleep(1);
		/* We have to make something (not important, what) now */
		/* no answer from phone*/
		return s->Phone.Functions->GetNetworkInfo(s,&NetInfo);
	}
	if (Ringtone->Format == RING_NOKIABINARY) {
		AddBinaryReq[7] = UnicodeLength(Ringtone->Name);
		CopyUnicodeString(AddBinaryReq+8,Ringtone->Name);
		current = 8 + UnicodeLength(Ringtone->Name)*2;
		AddBinaryReq[current++] = Ringtone->NokiaBinary.Length/256 + 1;
		AddBinaryReq[current++] = Ringtone->NokiaBinary.Length%256 + 1;
		AddBinaryReq[current++] = 0x00;
		memcpy(AddBinaryReq+current,Ringtone->NokiaBinary.Frame,Ringtone->NokiaBinary.Length);
		current += Ringtone->NokiaBinary.Length;
		smprintf(s, "Adding binary ringtone\n");
		return GSM_WaitFor (s, AddBinaryReq, current, 0x1F, 4, ID_SetRingtone);
	}
	if (Ringtone->Format == RING_MIDI) {
		AddBinaryReq[7] = UnicodeLength(Ringtone->Name);
		CopyUnicodeString(AddBinaryReq+8,Ringtone->Name);
		current = 8 + UnicodeLength(Ringtone->Name)*2;
		AddBinaryReq[current++] = Ringtone->NokiaBinary.Length/256;
		AddBinaryReq[current++] = Ringtone->NokiaBinary.Length%256;
		memcpy(AddBinaryReq+current,Ringtone->NokiaBinary.Frame,Ringtone->NokiaBinary.Length);
		current += Ringtone->NokiaBinary.Length;
		AddBinaryReq[current++] = 0x00;
		AddBinaryReq[current++] = 0x00;
		smprintf(s, "Adding binary or MIDI ringtone\n");
		return GSM_WaitFor (s, AddBinaryReq, current, 0x1F, 4, ID_SetRingtone);
	}
	return ERR_NOTSUPPORTED;
}

static GSM_Error N6510_ReplyDeleteRingtones(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Ringtones deleted\n");
	return ERR_NONE;
}

static GSM_Error N6510_DeleteUserRingtones(GSM_StateMachine *s)
{
	unsigned char DelAllRingtoneReq[] = {N7110_FRAME_HEADER, 0x10, 0x7F, 0xFE};

	smprintf(s, "Deleting all user ringtones\n");
	return GSM_WaitFor (s, DelAllRingtoneReq, 6, 0x1F, 4, ID_SetRingtone);
}

static GSM_Error N6510_PressKey(GSM_StateMachine *s, GSM_KeyCode Key, bool Press)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x11, 0x00, 0x01, 0x00, 0x00,
			       0x00,		/* Event */
			       0x01};		/* Number of presses */

	req[7] = Key;
	if (Press) {
		req[8] = NOKIA_PRESSPHONEKEY;
		s->Phone.Data.PressKey = true;
		smprintf(s, "Pressing key\n");
	} else {
		req[8] = NOKIA_RELEASEPHONEKEY;
		s->Phone.Data.PressKey = false;
		smprintf(s, "Releasing key\n");
	}
	return GSM_WaitFor (s, req, 10, 0x0c, 4, ID_PressKey);
}

static GSM_Error N6510_EnableConnectionFunctions(GSM_StateMachine *s, N6510_Connection_Settings Type)
{
	GSM_Error	error;
	unsigned char 	req2[] = {N6110_FRAME_HEADER, 0x00, 0x01};
	unsigned char 	req3[] = {N6110_FRAME_HEADER, 0x00, 0x03};
	unsigned char 	req4[] = {N6110_FRAME_HEADER, 0x00, 0x04};

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_6230iWAP)) return ERR_NOTSUPPORTED;

	if (Type == N6510_MMS_SETTINGS    && GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOMMS)) return ERR_NOTSUPPORTED;
	if (Type == N6510_CHAT_SETTINGS   && !GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_CHAT)) return ERR_NOTSUPPORTED;
	if (Type == N6510_SYNCML_SETTINGS && !GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SYNCML)) return ERR_NOTSUPPORTED;

	error=DCT3DCT4_DisableConnectionFunctions(s);
	if (error!=ERR_NONE) return error;

	switch (Type) {
	case N6510_WAP_SETTINGS:
		return DCT3DCT4_EnableWAPFunctions(s);
	case N6510_MMS_SETTINGS:
		dbgprintf("Enabling MMS\n");
		return GSM_WaitFor (s, req2, 5, 0x3f, 4, ID_EnableConnectFunc);
	case N6510_SYNCML_SETTINGS:
		dbgprintf("Enabling SyncML\n");
		return GSM_WaitFor (s, req3, 5, 0x3f, 5, ID_EnableConnectFunc);
	case N6510_CHAT_SETTINGS:
		dbgprintf("Enabling Chat\n");
		return GSM_WaitFor (s, req4, 5, 0x3f, 5, ID_EnableConnectFunc);
	default:
		return ERR_UNKNOWN;
	}
}

static GSM_Error N6510_ReplyGetConnectionSettings(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	int 			tmp,num=0,i;
	GSM_Phone_Data		*Data = &s->Phone.Data;
	unsigned char		buff[2000];

	switch(msg.Buffer[3]) {
	case 0x16:
		smprintf(s, "Connection settings received OK\n");

		Data->WAPSettings->Number = Priv->BearerNumber;

		Data->WAPSettings->Proxy[0]   = 0x00;
		Data->WAPSettings->Proxy[1]   = 0x00;
		Data->WAPSettings->ProxyPort  = 8080;

		Data->WAPSettings->Proxy2[0]  = 0x00;
		Data->WAPSettings->Proxy2[1]  = 0x00;
		Data->WAPSettings->Proxy2Port = 8080;

		tmp = 4;

		NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[0].Title,true);
		CopyUnicodeString(Data->WAPSettings->Settings[1].Title,Data->WAPSettings->Settings[0].Title);
		smprintf(s, "Title: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].Title));

		NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[0].HomePage,true);
		CopyUnicodeString(Data->WAPSettings->Settings[1].HomePage,Data->WAPSettings->Settings[0].HomePage);
		smprintf(s, "Homepage: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].HomePage));

#ifdef DEBUG
		smprintf(s, "Connection type: ");
		switch (msg.Buffer[tmp]) {
			case 0x00: smprintf(s, "temporary\n");  break;
			case 0x01: smprintf(s, "continuous\n"); break;
			default:   smprintf(s, "unknown\n");
		}
		smprintf(s, "Connection security: ");
		switch (msg.Buffer[tmp+1]) {
			case 0x00: smprintf(s, "off\n");	break;
			case 0x01: smprintf(s, "on\n");		break;
			default:   smprintf(s, "unknown\n");
		}
		smprintf(s, "Bearer: ");
		switch (msg.Buffer[tmp+2]) {
			case 0x01: smprintf(s, "GSM data\n");	break;
			case 0x03: smprintf(s, "GPRS\n");	break;
			default:   smprintf(s, "unknown\n");
		}
		if (msg.Buffer[tmp+3] == 0x01) smprintf(s, "locked\n");
#endif
		Data->WAPSettings->Settings[0].IsContinuous = false;
		if (msg.Buffer[tmp] == 0x01) Data->WAPSettings->Settings[0].IsContinuous = true;
		Data->WAPSettings->Settings[1].IsContinuous = Data->WAPSettings->Settings[0].IsContinuous;

		Data->WAPSettings->Settings[0].IsSecurity = false;
		if (msg.Buffer[tmp+1] == 0x01) Data->WAPSettings->Settings[0].IsSecurity = true;
		Data->WAPSettings->Settings[1].IsSecurity = Data->WAPSettings->Settings[0].IsSecurity;

		Data->WAPSettings->ActiveBearer = WAPSETTINGS_BEARER_DATA;
		if (msg.Buffer[tmp+2] == 0x03) Data->WAPSettings->ActiveBearer = WAPSETTINGS_BEARER_GPRS;

		Data->WAPSettings->ReadOnly = false;
		if (msg.Buffer[tmp+3] == 0x01) Data->WAPSettings->ReadOnly = true;

		tmp+=3;

		if (Priv->BearerNumber == 2) {
			/* Here starts settings for data bearer */
			Data->WAPSettings->Settings[0].Bearer = WAPSETTINGS_BEARER_DATA;
			while ((msg.Buffer[tmp] != 0x01) || (msg.Buffer[tmp + 1] != 0x00)) tmp++;
			tmp += 4;

#ifdef DEBUG
			smprintf(s, "Authentication type: ");
			switch (msg.Buffer[tmp]) {
				case 0x00: smprintf(s, "normal\n");	break;
				case 0x01: smprintf(s, "secure\n");	break;
				default:   smprintf(s, "unknown\n");	break;
			}
			smprintf(s, "Data call type: ");
			switch (msg.Buffer[tmp+1]) {
				case 0x00: smprintf(s, "analogue\n");	break;
				case 0x01: smprintf(s, "ISDN\n");	break;
				default:   smprintf(s, "unknown\n");	break;
			}
			smprintf(s, "Data call speed: ");
			switch (msg.Buffer[tmp+2]) {
				case 0x00: smprintf(s, "automatic\n"); 	break;
				case 0x01: smprintf(s, "9600\n");	break;
				case 0x02: smprintf(s, "14400\n");	break;
				default:   smprintf(s, "unknown\n");	break;
			}
			smprintf(s, "Login Type: ");
			switch (msg.Buffer[tmp+4]) {
				case 0x00: smprintf(s, "manual\n");	break;
				case 0x01: smprintf(s, "automatic\n");	break;
				default:   smprintf(s, "unknown\n");	break;
			}
#endif
			Data->WAPSettings->Settings[0].IsNormalAuthentication=true;
			if (msg.Buffer[tmp]==0x01) Data->WAPSettings->Settings[0].IsNormalAuthentication=false;

			Data->WAPSettings->Settings[0].IsISDNCall=false;
			if (msg.Buffer[tmp+1]==0x01) Data->WAPSettings->Settings[0].IsISDNCall=true;

			switch (msg.Buffer[tmp+2]) {
				case 0x00: Data->WAPSettings->Settings[0].Speed=WAPSETTINGS_SPEED_AUTO;  break;
				case 0x01: Data->WAPSettings->Settings[0].Speed=WAPSETTINGS_SPEED_9600;	 break;
				case 0x02: Data->WAPSettings->Settings[0].Speed=WAPSETTINGS_SPEED_14400; break;
				default:
					smprintf(s, "Unknown speed settings: 0x%0x\n", msg.Buffer[tmp+2]);
					Data->WAPSettings->Settings[0].Speed=WAPSETTINGS_SPEED_AUTO;
					break;
			}

			Data->WAPSettings->Settings[0].ManualLogin=false;
			if (msg.Buffer[tmp+4]==0x00) Data->WAPSettings->Settings[0].ManualLogin = true;

			tmp+=5;

			NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[0].IPAddress,false);
			smprintf(s, "IP address: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].IPAddress));

			NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[0].DialUp,true);
			smprintf(s, "Dial-up number: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].DialUp));

			NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[0].User,true);
			smprintf(s, "User name: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].User));

			NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[0].Password,true);
			smprintf(s, "Password: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].Password));

			num = 1;
		} else {
			num = 0;
		}

		/* Here starts settings for gprs bearer */
		Data->WAPSettings->Settings[num].Bearer = WAPSETTINGS_BEARER_GPRS;
		while (msg.Buffer[tmp] != 0x03) tmp++;
		tmp += 4;

#ifdef DEBUG
		smprintf(s, "Authentication type: ");
		switch (msg.Buffer[tmp]) {
			case 0x00: smprintf(s, "normal\n");	break;
			case 0x01: smprintf(s, "secure\n");	break;
			default:   smprintf(s, "unknown\n");	break;
		}
		smprintf(s, "GPRS connection: ");
		switch (msg.Buffer[tmp+1]) {
			case 0x00: smprintf(s, "ALWAYS online\n"); break;
			case 0x01: smprintf(s, "when needed\n");   break;
			default:   smprintf(s, "unknown\n"); 	   break;
		}
		smprintf(s, "Login Type: ");
		switch (msg.Buffer[tmp+2]) {
			case 0x00: smprintf(s, "manual\n");	break;
			case 0x01: smprintf(s, "automatic\n");	break;
			default:   smprintf(s, "unknown\n");	break;
		}
#endif
		Data->WAPSettings->Settings[num].IsNormalAuthentication=true;
		if (msg.Buffer[tmp]==0x01) Data->WAPSettings->Settings[num].IsNormalAuthentication=false;

		Data->WAPSettings->Settings[num].IsISDNCall=false;
		Data->WAPSettings->Settings[num].Speed = WAPSETTINGS_SPEED_AUTO;

		Data->WAPSettings->Settings[num].IsContinuous = true;
		if (msg.Buffer[tmp+1] == 0x01) Data->WAPSettings->Settings[num].IsContinuous = false;

		Data->WAPSettings->Settings[num].ManualLogin=false;
		if (msg.Buffer[tmp+2]==0x00) Data->WAPSettings->Settings[num].ManualLogin = true;

		tmp+=3;

		NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[num].DialUp,false);
		smprintf(s, "Access point: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[num].DialUp));

		NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[num].IPAddress,true);
		smprintf(s, "IP address: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[num].IPAddress));

		NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[num].User,true);
		smprintf(s, "User name: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[num].User));

		NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[num].Password,true);
		smprintf(s, "Password: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[num].Password));

		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_WAPMMSPROXY)) {
			if (msg.Buffer[tmp] == 0x00 && msg.Buffer[tmp+1] == 0x00) tmp = tmp+2;

			memcpy(buff,msg.Buffer+tmp+10,msg.Buffer[tmp+4]);
			buff[msg.Buffer[tmp+4]] = 0x00;
			smprintf(s, "Proxy 1: \"%s\", port %i\n",buff,msg.Buffer[tmp+6]*256+msg.Buffer[tmp+7]);
			EncodeUnicode(Data->WAPSettings->Proxy,buff,strlen(buff));
			Data->WAPSettings->ProxyPort = msg.Buffer[tmp+6]*256+msg.Buffer[tmp+7];

			memcpy(buff,msg.Buffer+tmp+10+msg.Buffer[tmp+4],msg.Buffer[tmp+5]);
			buff[msg.Buffer[tmp+5]] = 0x00;
			smprintf(s, "Proxy 2: \"%s\", port %i\n",buff,msg.Buffer[tmp+8]*256+msg.Buffer[tmp+9]);
			EncodeUnicode(Data->WAPSettings->Proxy2,buff,strlen(buff));
			Data->WAPSettings->Proxy2Port = msg.Buffer[tmp+8]*256+msg.Buffer[tmp+9];

			tmp = tmp + msg.Buffer[tmp+3] + 19;

			for (i=0;i<4;i++) {
#ifdef DEBUG
				smprintf(s, "Proxy data %i\n",i+1);
				if (msg.Buffer[tmp+2]!=0) memcpy(buff,msg.Buffer+tmp+9,msg.Buffer[tmp+2]*2);
				buff[msg.Buffer[tmp+2]*2]  =0;
				buff[msg.Buffer[tmp+2]*2+1]=0;
				smprintf(s, "IP: \"%s\"",DecodeUnicodeString(buff));
				smprintf(s, ", port %i\n",msg.Buffer[tmp+3]*256+msg.Buffer[tmp+4]);
#endif
				tmp = tmp + msg.Buffer[tmp];
			}

#ifdef DEBUG
			smprintf(s, "%02x %02x\n",msg.Buffer[tmp],msg.Buffer[tmp+1]);
			smprintf(s, "Port %i\n",msg.Buffer[tmp+3]*256+msg.Buffer[tmp+4]);
			tmp = tmp + msg.Buffer[tmp];
#endif
		}

		return ERR_NONE;
	case 0xf0:
		/*
		 * Don't know exactly what 0x0f means, but the message is too short
		 * to contain information:
		 *
		 * 01 |58X|00 |F0 |01 |15 |00 |00 |00 |00
		 */
		smprintf(s, "Connection settings receiving error, assuming empty\n");
		return ERR_EMPTY;
	case 0x11:
	case 0x17:
		smprintf(s, "Connection settings receiving error\n");
		switch (msg.Buffer[4]) {
		case 0x01:
			smprintf(s, "Security error. Inside phone settings menu\n");
			return ERR_INSIDEPHONEMENU;
		case 0x02:
		case 0x03: /* Guess */
			smprintf(s, "Invalid or empty\n");
			return ERR_INVALIDLOCATION;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
			return ERR_UNKNOWNRESPONSE;
		}
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetConnectionSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings, N6510_Connection_Settings Type)
{
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	GSM_Error 		error;
	unsigned char 		req[] = {N6110_FRAME_HEADER, 0x15,
				 	 0x00};		/* Location */

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) return ERR_NOTSUPPORTED;
	if (!strcmp(s->Phone.Data.ModelInfo->model,"6020")) return ERR_NOTSUPPORTED;

	error = N6510_EnableConnectionFunctions(s, Type);
	if (error!=ERR_NONE) return error;

	req[4] 			  = settings->Location-1;
	s->Phone.Data.WAPSettings = settings;

	switch (Type) {
	case N6510_MMS_SETTINGS:
		smprintf(s, "Getting MMS settings\n");
		Priv->BearerNumber = 1;
		break;
	case N6510_WAP_SETTINGS:
		smprintf(s, "Getting WAP settings\n");
		Priv->BearerNumber = 2;
		break;
	case N6510_SYNCML_SETTINGS:
		smprintf(s, "Getting SyncML settings\n");
		Priv->BearerNumber = 2;
		break;
	case N6510_CHAT_SETTINGS:
		smprintf(s, "Getting Chat settings\n");
		Priv->BearerNumber = 1;
		break;
	}

	error=GSM_WaitFor (s, req, 5, 0x3f, 4, ID_GetConnectSet);
	if (error != ERR_NONE) {
		if (error == ERR_INVALIDLOCATION || error == ERR_INSIDEPHONEMENU) {
			DCT3DCT4_DisableConnectionFunctions(s);
		}
		return error;
	}

	error=DCT3DCT4_GetActiveConnectSet(s);
	if (error != ERR_NONE) return error;

	return DCT3DCT4_DisableConnectionFunctions(s);
}

static GSM_Error N6510_GetWAPSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
	return N6510_GetConnectionSettings(s, settings, N6510_WAP_SETTINGS);
}

static GSM_Error N6510_GetMMSSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
	return N6510_GetConnectionSettings(s, settings, N6510_MMS_SETTINGS);
}

static GSM_Error N6510_ReplyGetSyncMLSettings(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_SyncMLSettings *Sett = s->Phone.Data.SyncMLSettings;

	smprintf(s, "SyncML settings received OK\n");
	CopyUnicodeString(Sett->User,msg.Buffer+18);
	CopyUnicodeString(Sett->Password,msg.Buffer+86);
	CopyUnicodeString(Sett->PhonebookDataBase,msg.Buffer+130);
	CopyUnicodeString(Sett->CalendarDataBase,msg.Buffer+234);
	CopyUnicodeString(Sett->Server,msg.Buffer+338);

	Sett->SyncPhonebook = false;
	Sett->SyncCalendar  = false;
	if ((msg.Buffer[598] & 0x02)==0x02) Sett->SyncCalendar = true;
	if ((msg.Buffer[598] & 0x01)==0x01) Sett->SyncPhonebook = true;

	return ERR_NONE;
}

static GSM_Error N6510_ReplyGetSyncMLName(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_SyncMLSettings *Sett = s->Phone.Data.SyncMLSettings;

	smprintf(s, "SyncML names received OK\n");

	CopyUnicodeString(Sett->Name,msg.Buffer+18);

	return ERR_NONE;
}

static GSM_Error N6510_GetSyncMLSettings(GSM_StateMachine *s, GSM_SyncMLSettings *settings)
{
	GSM_Error 	error;
/* 	unsigned char 	NameReq[] = {N6110_FRAME_HEADER, 0x05, */
/* 				 0x00, 0x00, 0x00, 0x31, 0x00, */
/* 				 0x06, 0x00, 0x00, 0x00, 0xDE, 0x00, 0x00}; */
/* 	unsigned char 	GetActive[] = {N6110_FRAME_HEADER, 0x05, */
/* 				 0x00, 0x00, 0x00, 0x31, 0x00, */
/* 				 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00}; */
	unsigned char 	req[] = {N6110_FRAME_HEADER, 0x05,
				 0x00, 0x00, 0x00, 0x31, 0x00,
				 0x01, /* location */
				 0x00, 0x00, 0x02, 0x46, 0x00, 0x00};

	settings->Connection.Location = settings->Location;
	error = N6510_GetConnectionSettings(s, &settings->Connection, N6510_SYNCML_SETTINGS);
	if (error != ERR_NONE) return error;

	settings->Active = settings->Connection.Active;

	settings->Name[0] = 0;
	settings->Name[1] = 0;
	s->Phone.Data.SyncMLSettings  = settings;

/* 	smprintf(s, "Getting SyncML settings name\n"); */
/* 	error = GSM_WaitFor (s, NameReq, 16, 0x43, 4, ID_GetSyncMLName); */
/* 	if (error != ERR_NONE) return error; */

	req[9] = settings->Location - 1;
	smprintf(s, "Getting additional SyncML settings\n");
	return GSM_WaitFor (s, req, 16, 0x43, 4, ID_GetSyncMLSettings);
}

static GSM_Error N6510_ReplyGetChatSettings(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_ChatSettings 	*Sett = s->Phone.Data.ChatSettings;
	int			i;

	Sett->Name[0] 	  = 0;
	Sett->Name[1] 	  = 0;
	Sett->HomePage[0] = 0;
	Sett->HomePage[1] = 0;
	Sett->User[0] 	  = 0;
	Sett->User[1] 	  = 0;
	Sett->Password[0] = 0;
	Sett->Password[1] = 0;

	switch(msg.Buffer[3]) {
	case 0x3B:
		smprintf(s, "Chat settings received OK\n");
		memcpy(Sett->Name,msg.Buffer+20,msg.Buffer[12]*2);
		Sett->Name[msg.Buffer[12]*2] 	   = 0;
		Sett->Name[msg.Buffer[12]*2+1] 	   = 0;
		memcpy(Sett->HomePage,msg.Buffer+20+msg.Buffer[12]*2,msg.Buffer[15]*2);
		Sett->HomePage[msg.Buffer[15]*2]   = 0;
		Sett->HomePage[msg.Buffer[15]*2+1] = 0;
		i = msg.Buffer[12]*2 + msg.Buffer[15]*2 + 29;
		memcpy(Sett->User,msg.Buffer+i+3,msg.Buffer[i]*2);
		Sett->User[msg.Buffer[i]*2]   = 0;
		Sett->User[msg.Buffer[i]*2+1] = 0;
		memcpy(Sett->Password,msg.Buffer+i+3+msg.Buffer[i]*2,msg.Buffer[i+1]*2);
		Sett->Password[msg.Buffer[i+1]*2]   = 0;
		Sett->Password[msg.Buffer[i+1]*2+1] = 0;
		return ERR_NONE;
	case 0x3C:
		smprintf(s, "Empty chat settings received\n");
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetChatSettings(GSM_StateMachine *s, GSM_ChatSettings *settings)
{
	GSM_Error 	error;
	unsigned char 	req[] = {N6110_FRAME_HEADER, 0x3a,
				 0x09,			/*  location */
				 0x01, 0x0e};

	settings->Connection.Location = settings->Location;
	error = N6510_GetConnectionSettings(s, &settings->Connection, N6510_CHAT_SETTINGS);
	if (error != ERR_NONE) return error;

	settings->Active = settings->Connection.Active;

	s->Phone.Data.ChatSettings  = settings;
	req[4] 			    = settings->Location - 1;
	smprintf(s, "Getting additional Chat settings\n");
	return GSM_WaitFor (s, req, 7, 0x3f, 4, ID_GetChatSettings);
}

static GSM_Error N6510_ReplySetConnectionSettings(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
	case 0x19:
		smprintf(s, "Connection settings cleaned\n");
		return ERR_NONE;
	case 0x1a:
		smprintf(s, "Connection settings setting status\n");
		switch (msg.Buffer[4]) {
		case 0x01:
			smprintf(s, "Security error. Inside phone settings menu\n");
			return ERR_INSIDEPHONEMENU;
		case 0x03:
			smprintf(s, "Invalid location\n");
			return ERR_INVALIDLOCATION;
		case 0x05:
			smprintf(s, "Written OK\n");
			return ERR_NONE;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
			return ERR_UNKNOWNRESPONSE;
		}
	case 0x28:
	case 0x2B:
		smprintf(s, "Set OK\n");
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_SetConnectionSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings, N6510_Connection_Settings Type)
{
	GSM_Error 	error;
	int 		i, pad = 0, length, pos = 5, loc1=-1,loc2=-1,port;
	unsigned char	*Proxy;
	unsigned char 	req[2000] = {N6110_FRAME_HEADER, 0x18,
				     0x00};		/* Location */
	unsigned char 	Lock[5] = {N6110_FRAME_HEADER, 0x27,
				   0x00};		/* Location */
	unsigned char 	UnLock[5] = {N6110_FRAME_HEADER, 0x2A,
				   0x00};		/* Location */

	error = N6510_EnableConnectionFunctions(s, Type);
	if (error!=ERR_NONE) return error;

	memset(req + pos, 0, 1000 - pos);

	req[4] = settings->Location-1;

	for (i=0;i<settings->Number;i++) {
		if (settings->Settings[i].Bearer == WAPSETTINGS_BEARER_DATA) loc1=i;
		if (settings->Settings[i].Bearer == WAPSETTINGS_BEARER_GPRS) loc2=i;
	}

	if (loc1 != -1) {
		/* Name */
		length = UnicodeLength(settings->Settings[loc1].Title);
		if (!(length % 2)) pad = 1;
		pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc1].Title, false);

		/* Home */
		length = UnicodeLength(settings->Settings[loc1].HomePage);
		if (((length + pad) % 2)) pad = 2; else pad = 0;
		pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc1].HomePage, true);

		if (settings->Settings[loc1].IsContinuous) req[pos] = 0x01; pos++;
		if (settings->Settings[loc1].IsSecurity) req[pos] = 0x01; pos++;
	} else if (loc2 != -1) {
		/* Name */
		length = UnicodeLength(settings->Settings[loc2].Title);
		if (!(length % 2)) pad = 1;
		pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc2].Title, false);

		/* Home */
		length = UnicodeLength(settings->Settings[loc2].HomePage);
		if (((length + pad) % 2)) pad = 2; else pad = 0;
		pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc2].HomePage, true);

		if (settings->Settings[loc2].IsContinuous) req[pos] = 0x01; pos++;
		if (settings->Settings[loc2].IsSecurity) req[pos] = 0x01; pos++;
	} else {
		/* Name */
		length = 0;
		if (!(length % 2)) pad = 1;
		pos ++;

		/* Home */
		length = 0;
		if (((length + pad) % 2)) pad = 2; else pad = 0;
		pos += 2;

		pos += 2;
	}

	if (Type == N6510_MMS_SETTINGS || Type == N6510_CHAT_SETTINGS) {
		req[pos++] = 0x03; /* active bearer: GPRS */
	} else {
		if (settings->ActiveBearer == WAPSETTINGS_BEARER_GPRS && loc2 != -1) {
			req[pos++] = 0x03; /* active bearer: GPRS */
		} else {
			req[pos++] = 0x01; /* active bearer: data set */
		}
	}

	/* Number of sent bearers */
	if (Type == N6510_MMS_SETTINGS || Type == N6510_CHAT_SETTINGS) {
		req[pos] = 0x01;
	} else {
		req[pos] = 0x02;
	}
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_WAPMMSPROXY)) req[pos] += 2;
	pos++;
 	pos += pad;

	if (Type != N6510_MMS_SETTINGS && Type != N6510_CHAT_SETTINGS) {
		/* GSM data block */
		memcpy(req + pos, "\x01\x00", 2);	pos += 2;

		if (loc1 != -1) {
			length  = UnicodeLength(settings->Settings[loc1].IPAddress)*2+1;
			length += UnicodeLength(settings->Settings[loc1].DialUp)   *2+2;
			length += UnicodeLength(settings->Settings[loc1].User)     *2+2;
			length += UnicodeLength(settings->Settings[loc1].Password) *2+2;
		} else {
			length = 1 + 2 + 2 + 2;
		}
		length += 11;
		req[pos++] = length / 256;
		req[pos++] = length % 256;

		if (loc1 != -1) {
			if (!settings->Settings[loc1].IsNormalAuthentication) req[pos]=0x01; pos++;
			if (settings->Settings[loc1].IsISDNCall) req[pos]=0x01;	pos++;
			switch (settings->Settings[loc1].Speed) {
				case WAPSETTINGS_SPEED_AUTO	: 		 break;
				case WAPSETTINGS_SPEED_9600	: req[pos]=0x01; break;
				case WAPSETTINGS_SPEED_14400	: req[pos]=0x02; break;
			}
			pos++;
			req[pos++]=0x01;
			if (!settings->Settings[loc1].ManualLogin) req[pos] = 0x01; pos++;

			pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc1].IPAddress, false);
			pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc1].DialUp, true);
			pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc1].User, true);
			pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc1].Password, true);
		} else {
			pos += 3;
			req[pos++]=0x01;
			pos += 8;
		}

		/* Padding */
		pos+=2;
	}

	/* GPRS block */
	memcpy(req + pos, "\x03\x00", 2);	pos += 2;

	if (loc2 != -1) {
		length  = UnicodeLength(settings->Settings[loc2].DialUp)   *2+1;
		length += UnicodeLength(settings->Settings[loc2].IPAddress)*2+2;
		length += UnicodeLength(settings->Settings[loc2].User)     *2+2;
		length += UnicodeLength(settings->Settings[loc2].Password) *2+2;
	} else {
		length = 7;
	}
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_WAPMMSPROXY)) length+=2;
	length += 7;
	req[pos++] = length / 256;
	req[pos++] = length % 256;

	if (loc2 != -1) {
		if (!settings->Settings[loc2].IsNormalAuthentication) req[pos] = 0x01; pos++;
		if (!settings->Settings[loc2].IsContinuous) req[pos] = 0x01; pos++;
		if (!settings->Settings[loc2].ManualLogin) req[pos] = 0x01; pos++;

		pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc2].DialUp, false);
		pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc2].IPAddress, true);
		pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc2].User, true);
		pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc2].Password, true);
	} else {
		pos += 10;
	}

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_WAPMMSPROXY)) {
		req[pos++] = 0x00;
		req[pos++] = 0x00;

		/* Proxy block */
		req[pos++] = 0x06;
		req[pos++] = 0x01;
		if (UnicodeLength(settings->Proxy)!=0 ||
		    UnicodeLength(settings->Proxy2)!=0) {
			req[pos++] = (UnicodeLength(settings->Proxy)+UnicodeLength(settings->Proxy2)+13)/256;
			req[pos++] = (UnicodeLength(settings->Proxy)+UnicodeLength(settings->Proxy2)+13)%256;
		} else {
			req[pos++] = (UnicodeLength(settings->Proxy)+UnicodeLength(settings->Proxy2)+12)/256;
			req[pos++] = (UnicodeLength(settings->Proxy)+UnicodeLength(settings->Proxy2)+12)%256;
		}
		req[pos++] = UnicodeLength(settings->Proxy);
		req[pos++] = UnicodeLength(settings->Proxy2);
		req[pos++] = settings->ProxyPort/256;
		req[pos++] = settings->ProxyPort%256;
		req[pos++] = settings->Proxy2Port/256;
		req[pos++] = settings->Proxy2Port%256;
		if (UnicodeLength(settings->Proxy)!=0) {
			sprintf(req+pos,"%s",DecodeUnicodeString(settings->Proxy));
			pos+=UnicodeLength(settings->Proxy);
		}
		if (UnicodeLength(settings->Proxy2)!=0) {
			sprintf(req+pos,"%s",DecodeUnicodeString(settings->Proxy2));
			pos+=UnicodeLength(settings->Proxy2);
		}
		if (UnicodeLength(settings->Proxy)!=0 ||
		    UnicodeLength(settings->Proxy2)!=0) {
			req[pos++] = 0x00;
		}
		req[pos++] = 0x00; req[pos++] = 0x00;
		req[pos++] = 0x07; /* unknown */
		req[pos++] = 0x00; req[pos++] = 0x00;
		req[pos++] = 0x80; /* unknown */
		req[pos++] = 0x01; /* unknown */
		req[pos++] = 0x05; /* unknown */
		req[pos++] = 0x00; req[pos++] = 0x00;

		/* Proxy data blocks */
		for (i=0;i<4;i++) {
			port  = 8080;
			Proxy = NULL;
			if (i==0) {
				port  = settings->ProxyPort;
				Proxy = settings->Proxy;
			} else if (i==1) {
				port  = settings->Proxy2Port;
				Proxy = settings->Proxy2;
			}
			req[pos++] = 0x08; req[pos++] = 0x00;
			if (Proxy != NULL && UnicodeLength(Proxy)!=0) {
				if (UnicodeLength(Proxy)%2 != 0) {
					req[pos++] = (12 + (UnicodeLength(Proxy)+1)*2)/256;
					req[pos++] = (12 + (UnicodeLength(Proxy)+1)*2)%256;
				} else {
					req[pos++] = (12 + UnicodeLength(Proxy)*2)/256;
					req[pos++] = (12 + UnicodeLength(Proxy)*2)%256;
				}
			} else {
				req[pos++] = 12/256;
				req[pos++] = 12%256;
			}
			req[pos++] = i+1;
			if (Proxy != NULL) {
				req[pos++] = UnicodeLength(Proxy);
			} else {
				req[pos++] = 0;
			}
			req[pos++] = port/256;
			req[pos++] = port%256;
			req[pos++] = 0x00;

			req[pos++] = 0x00;
			req[pos++] = 0x01;

			req[pos++] = 0x00;
			if (Proxy != NULL && UnicodeLength(Proxy)!=0) {
				CopyUnicodeString(req+pos,Proxy);
				pos+=UnicodeLength(Proxy)*2;
				if (UnicodeLength(Proxy)%2 != 0) {
					req[pos++] = 0x00;
					req[pos++] = 0x00;
				}
			}
		}

		req[pos++] = 0x09; req[pos++] = 0x00; req[pos++] = 0x00;
		req[pos++] = 0x0C; req[pos++] = 0x02; req[pos++] = 0x00;
		req[pos++] = 0x00; req[pos++] = 0x02; req[pos++] = 0x00;
		req[pos++] = 0x00; req[pos++] = 0x00; req[pos++] = 0x00;
	} else {
		/* end of blocks ? */
		memcpy(req + pos, "\x80\x00\x00\x0c", 4);	pos += 4;
	}

	UnLock[4] = settings->Location-1;
	smprintf(s, "Making Connection settings read-write\n");
	error = GSM_WaitFor (s, UnLock, 5, 0x3f, 4, ID_SetConnectSet);
	if (error != ERR_NONE) return error;

	switch (Type) {
	case N6510_MMS_SETTINGS:
		smprintf(s, "Setting MMS settings\n");
		break;
	case N6510_CHAT_SETTINGS:
		smprintf(s, "Setting Chat settings\n");
		break;
	case N6510_WAP_SETTINGS:
		smprintf(s, "Setting WAP settings\n");
		break;
	case N6510_SYNCML_SETTINGS:
		smprintf(s, "Setting SyncML settings\n");
		break;
	}
	error = GSM_WaitFor (s, req, pos, 0x3f, 4, ID_SetConnectSet);
	if (error != ERR_NONE) {
		if (error == ERR_INSIDEPHONEMENU || error == ERR_INVALIDLOCATION) {
			DCT3DCT4_DisableConnectionFunctions(s);
		}
		return error;
	}

	if (settings->ReadOnly) {
		Lock[4] = settings->Location-1;
		smprintf(s, "Making Connection settings readonly\n");
		error = GSM_WaitFor (s, Lock, 5, 0x3f, 4, ID_SetConnectSet);
		if (error != ERR_NONE) return error;
	}

	error = DCT3DCT4_SetActiveConnectSet(s, settings);
	if (error != ERR_NONE) return error;

	return DCT3DCT4_DisableConnectionFunctions(s);
}

static GSM_Error N6510_SetWAPSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
	return N6510_SetConnectionSettings(s, settings, N6510_WAP_SETTINGS);
}

static GSM_Error N6510_SetMMSSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
	return N6510_SetConnectionSettings(s, settings, N6510_MMS_SETTINGS);
}

static GSM_Error N6510_ReplyGetOriginalIMEI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	if (msg.Buffer[7] == 0x00) {
		smprintf(s, "No SIM card\n");
		return ERR_NOSIM;
	} else {
		return NOKIA_ReplyGetPhoneString(msg, s);
	}
}

static GSM_Error N6510_GetOriginalIMEI(GSM_StateMachine *s, char *value)
{
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) return ERR_NOTSUPPORTED;

	return NOKIA_GetPhoneString(s,"\x00\x07\x02\x01\x00\x01",6,0x42,value,ID_GetOriginalIMEI,14);
}

static GSM_Error N6510_ReplyGetSMSStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	switch (msg.Buffer[3]) {
	case 0x09:
		switch (msg.Buffer[4]) {
		case 0x00:
			smprintf(s, "Max. in phone memory   : %i\n",msg.Buffer[10]*256+msg.Buffer[11]);
			smprintf(s, "Used in phone memory   : %i\n",msg.Buffer[12]*256+msg.Buffer[13]);
			smprintf(s, "Unread in phone memory : %i\n",msg.Buffer[14]*256+msg.Buffer[15]);
			smprintf(s, "Max. in SIM            : %i\n",msg.Buffer[22]*256+msg.Buffer[23]);
			smprintf(s, "Used in SIM            : %i\n",msg.Buffer[24]*256+msg.Buffer[25]);
			smprintf(s, "Unread in SIM          : %i\n",msg.Buffer[26]*256+msg.Buffer[27]);
			Data->SMSStatus->PhoneSize	= msg.Buffer[10]*256+msg.Buffer[11];
			Data->SMSStatus->PhoneUsed	= msg.Buffer[12]*256+msg.Buffer[13];
			Data->SMSStatus->PhoneUnRead 	= msg.Buffer[14]*256+msg.Buffer[15];
			Data->SMSStatus->SIMSize	= msg.Buffer[22]*256+msg.Buffer[23];
			Data->SMSStatus->SIMUsed 	= msg.Buffer[24]*256+msg.Buffer[25];
			Data->SMSStatus->SIMUnRead 	= msg.Buffer[26]*256+msg.Buffer[27];
			return ERR_NONE;
		case 0x0f:
			smprintf(s, "No PIN\n");
			return ERR_SECURITYERROR;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
			return ERR_UNKNOWNRESPONSE;
		}
	case 0x1a:
		smprintf(s, "Wait a moment. Phone is during power on and busy now\n");
		return ERR_SECURITYERROR;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	GSM_Error 		error;
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	unsigned char req[] = {N6110_FRAME_HEADER, 0x08, 0x00, 0x00};

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) return ERR_NOTSUPPORTED;

	s->Phone.Data.SMSStatus=status;
	smprintf(s, "Getting SMS status\n");
	error = GSM_WaitFor (s, req, 6, 0x14, 2, ID_GetSMSStatus);
	if (error != ERR_NONE) return error;

	/* DCT4 family doesn't show in frame with SMS status info
         * about Templates. We get separately info about this SMS folder.
	 */
	error = N6510_GetSMSFolderStatus(s, 0x06);
	if (error != ERR_NONE) return error;
	status->TemplatesUsed = Priv->LastSMSFolder.Number;

	return error;
}

static GSM_Error N6510_ReplyDeleteSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
		case 0x05:
			smprintf(s, "SMS deleted OK\n");
			return ERR_NONE;
		case 0x06:
			switch (msg.Buffer[4]) {
				case 0x02:
					smprintf(s, "Invalid location\n");
					return ERR_INVALIDLOCATION;
				default:
					smprintf(s, "Unknown error: %02x\n",msg.Buffer[4]);
					return ERR_UNKNOWNRESPONSE;
			}
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_DeleteSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	unsigned char		folderid;
	unsigned int		location;
	unsigned char 		req[] = {N6110_FRAME_HEADER, 0x04,
					 0x01, 		/* 0x01=SM, 0x02=ME */
					 0x00, 		/* FolderID */
					 0x00, 0x02, 	/* Location */
					 0x0F, 0x55};

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMS_FILES)) return ERR_NOTSUPPORTED;
	}

	N6510_GetSMSLocation(s, sms, &folderid, &location);

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
		switch (folderid) {
			case 0x01: req[5] = 0x01; 			 break; /* SIM 		*/
			default	 : req[5] = folderid; req[4] = 0x02; 	 break; /* ME folders	*/
		}
	} else {
		switch (folderid) {
			case 0x01: req[5] = 0x02; 			 break; /* INBOX SIM 	*/
			case 0x02: req[5] = 0x03; 			 break; /* OUTBOX SIM 	*/
			default	 : req[5] = folderid - 1; req[4] = 0x02; break; /* ME folders	*/
		}
	}
	req[6]=location / 256;
	req[7]=location % 256;

	smprintf(s, "Deleting sms\n");
	return GSM_WaitFor (s, req, 10, 0x14, 4, ID_DeleteSMSMessage);
}

static GSM_Error N6510_ReplySendSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[8]) {
	case 0x00:
		smprintf(s, "SMS sent OK, TPMR for sent sms is %d\n",msg.Buffer[10]);
		if (s->User.SendSMSStatus!=NULL) s->User.SendSMSStatus(s,0,msg.Buffer[10]);
		return ERR_NONE;
	default:
		smprintf(s, "SMS not sent OK, error code probably %i\n",msg.Buffer[8]);
		if (s->User.SendSMSStatus!=NULL) s->User.SendSMSStatus(s,msg.Buffer[8],msg.Buffer[10]);
		return ERR_NONE;
	}
}

static GSM_Error N6510_SendSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	int			length = 11;
	GSM_Error		error;
	GSM_SMSMessageLayout 	Layout;
	unsigned char req [300] = {
		N6110_FRAME_HEADER, 0x02, 0x00, 0x00, 0x00, 0x55, 0x55};

	if (sms->PDU == SMS_Deliver) sms->PDU = SMS_Submit;
	memset(req+9,0x00,sizeof(req) - 9);
	error=N6510_EncodeSMSFrame(s, sms, req + 9, &Layout, &length);
	if (error != ERR_NONE) return error;

	smprintf(s, "Sending sms\n");
	return s->Protocol.Functions->WriteMessage(s, req, length + 9, 0x02);
}

static GSM_Error N6510_ReplyGetSecurityStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Security Code status received: ");
	switch (msg.Buffer[4]) {
	case 0x01 : smprintf(s, "waiting for Security Code.\n"); *Data->SecurityStatus = SEC_SecurityCode;	break;
	case 0x07 :
	case 0x02 : smprintf(s, "waiting for PIN.\n");		 *Data->SecurityStatus = SEC_Pin;		break;
	case 0x03 : smprintf(s, "waiting for PUK.\n");		 *Data->SecurityStatus = SEC_Puk;		break;
	case 0x05 : smprintf(s, "PIN ok, SIM ok\n");		 *Data->SecurityStatus = SEC_None;		break;
	case 0x06 : smprintf(s, "No input status\n"); 		 *Data->SecurityStatus = SEC_None;		break;
	case 0x16 : smprintf(s, "No SIM card\n");		 return ERR_NOSIM;
	case 0x1A : smprintf(s, "SIM card rejected!\n");	 *Data->SecurityStatus = SEC_None;		break;
	default   : smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
		    return ERR_UNKNOWNRESPONSE;
	}
	return ERR_NONE;
}

static GSM_Error N6510_GetSecurityStatus(GSM_StateMachine *s, GSM_SecurityCodeType *Status)
{
	unsigned char req[5] = {N6110_FRAME_HEADER, 0x11, 0x00};

	s->Phone.Data.SecurityStatus=Status;
	smprintf(s, "Getting security code status\n");
	return GSM_WaitFor (s, req, 5, 0x08, 2, ID_GetSecurityStatus);
}

static GSM_Error N6510_ReplyEnterSecurityCode(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
	case 0x08:
		smprintf(s, "Security code OK\n");
		return ERR_NONE;
	case 0x09:
		switch (msg.Buffer[4]) {
		case 0x06:
			smprintf(s, "Wrong PIN\n");
			return ERR_SECURITYERROR;
		case 0x09:
			smprintf(s, "Wrong PUK\n");
			return ERR_SECURITYERROR;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
		}
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_EnterSecurityCode(GSM_StateMachine *s, GSM_SecurityCode Code)
{
	int 		len = 0;
	unsigned char 	req[15] = {N6110_FRAME_HEADER, 0x07,
				   0x00};	/* Code type */

	switch (Code.Type) {
		case SEC_Pin	: req[4] = 0x02; break;
		case SEC_Puk	: req[4] = 0x03; break;/* FIXME */
		default		: return ERR_NOTSUPPORTED;
	}

	len = strlen(Code.Code);
	memcpy(req+5,Code.Code,len);
	req[5+len]=0x00;

	smprintf(s, "Entering security code\n");
	return GSM_WaitFor (s, req, 6+len, 0x08, 4, ID_EnterSecurityCode);
}

static GSM_Error N6510_ReplySaveSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 		folder;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	switch (msg.Buffer[3]) {
	case 0x01:
		switch (msg.Buffer[4]) {
		case 0x00:
			smprintf(s, "Done OK\n");
			break;
		case 0x02:
			printf("Incorrect location\n");
			return ERR_INVALIDLOCATION;
		case 0x03:
			printf("Memory full (for example no empty space in SIM)\n");
			return ERR_FULL;
		case 0x05:
			printf("Incorrect folder\n");
			return ERR_INVALIDLOCATION;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
			return ERR_UNKNOWNRESPONSE;
		}

		smprintf(s, "Folder info: %i %i\n",msg.Buffer[5],msg.Buffer[8]);
		Data->SaveSMSMessage->Memory = MEM_ME;
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
			folder = msg.Buffer[8];
		} else {
			folder = msg.Buffer[8] + 1;
			/* inbox,outbox */
			if (msg.Buffer[8] == 0x02 || msg.Buffer[8] == 0x03) {
				if (msg.Buffer[5] == 0x01) {
					folder = msg.Buffer[8] - 1;
					Data->SaveSMSMessage->Memory = MEM_SM;
				}
			}
		}
		N6510_SetSMSLocation(s, Data->SaveSMSMessage,folder,msg.Buffer[6]*256+msg.Buffer[7]);
		smprintf(s, "Saved in folder %i at location %i\n",folder, msg.Buffer[6]*256+msg.Buffer[7]);
		Data->SaveSMSMessage->Folder = folder;
		return ERR_NONE;
	case 0x17:
		smprintf(s, "SMS name changed\n");
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_PrivSetSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	int			length = 11;
	unsigned int		location;
	unsigned char		folderid, folder;
	GSM_SMSMessageLayout 	Layout;
	GSM_Error		error;
	unsigned char req [300] = {
		N6110_FRAME_HEADER, 0x00,
		0x01,			/* 1 = SIM, 2 = ME 	*/
		0x02,			/* Folder   		*/
		0x00, 0x01,		/* Location 		*/
		0x01};			/* SMS state 		*/
	unsigned char NameReq[200] = {
		N6110_FRAME_HEADER, 0x16,
		0x01,			/* 1 = SIM, 2 = ME 	*/
		0x02,			/* Folder   		*/
		0x00, 0x01};		/* Location 		*/

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
		if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SMS_FILES)) return ERR_NOTSUPPORTED;
	}

	N6510_GetSMSLocation(s, sms, &folderid, &location);
	if (folderid == 0x99) return ERR_INVALIDLOCATION;
	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
		switch (folderid) {
			case 0x02: req[4] = 0x02; req[5] = 0x02; break; /* inbox */
			/* sms saved to sent items make problems later during reading */
			/* case 0x03: req[4] = 0x02; req[5] = 0x03; break; //sent items */
			default	 : return ERR_NOTSUPPORTED; /* at least 6111 doesn't support saving to other */
		}
	} else {
		switch (folderid) {
			case 0x01: req[5] = 0x02; 			 break; /* INBOX SIM 	*/
			case 0x02: req[5] = 0x03; 			 break; /* OUTBOX SIM 	*/
			default	 : req[5] = folderid - 1; req[4] = 0x02; break; /* ME folders	*/
		}
	}
	req[6]=location / 256;
	req[7]=location % 256;

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
		switch (sms->PDU) {
		case SMS_Status_Report: /* this is SMS submit with delivery report request */
		case SMS_Submit:
			break;
		case SMS_Deliver:
			/* Sent items */
			if (folderid == 0x03) sms->PDU = SMS_Submit;
			break;
		default:
			return ERR_UNKNOWN;
		}
	} else {
		switch (sms->PDU) {
		case SMS_Status_Report: /* this is SMS submit with delivery report request */
		case SMS_Submit:
			/* Inbox */
			if (folderid == 0x01 || folderid == 0x03) sms->PDU = SMS_Deliver;
			break;
		case SMS_Deliver:
			/* SIM Outbox */
			if (folderid == 0x02) sms->PDU = SMS_Submit;
			break;
		default:
			return ERR_UNKNOWN;
		}
		if (sms->PDU == SMS_Deliver) {
			switch (sms->State) {
				case SMS_Sent	: /* We use GSM_Read, because phone return error */
				case SMS_Read	: req[8] = 0x01; break;
				case SMS_UnSent	: /* We use GSM_UnRead, because phone return error */
				case SMS_UnRead	: req[8] = 0x03; break;
			}
		} else {
			switch (sms->State) {
				case SMS_Sent	: /* We use GSM_Sent, because phone change folder */
				case SMS_Read	: req[8] = 0x05; break;
				case SMS_UnSent	: /* We use GSM_UnSent, because phone change folder */
				case SMS_UnRead	: req[8] = 0x07; break;
			}
		}
	}
	memset(req+9,0x00,sizeof(req) - 9);
	error=N6510_EncodeSMSFrame(s, sms, req + 9, &Layout, &length);
	if (error != ERR_NONE) return error;

	s->Phone.Data.SaveSMSMessage=sms;
	smprintf(s, "Saving sms\n");
	error=GSM_WaitFor (s, req, length+9, 0x14, 4, ID_SaveSMSMessage);
	if (error != ERR_NONE) return error;

	/* no adding to SIM SMS */
	if (UnicodeLength(sms->Name)==0 || sms->Folder < 3) return ERR_NONE;

	folder = sms->Folder;
	sms->Folder = 0;
	N6510_GetSMSLocation(s, sms, &folderid, &location);

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) {
		switch (folderid) {
			case 0x03: NameReq[4] = 0x02; NameReq[5] = 0x02; break; /* sent items */
			case 0x02: NameReq[4] = 0x02; NameReq[5] = 0x03; break; /* inbox */
			default	 : return ERR_NOTSUPPORTED; /* at least 6111 doesn't support saving to other */
		}
	} else {
		switch (folderid) {
			case 0x01: NameReq[5] = 0x02; 				 break; /* INBOX SIM 	*/
			case 0x02: NameReq[5] = 0x03; 			 	 break; /* OUTBOX SIM 	*/
			default	 : NameReq[5] = folderid - 1; NameReq[4] = 0x02; break; /* ME folders	*/
		}
	}
	NameReq[6]=location / 256;
	NameReq[7]=location % 256;
	length = 8;
	CopyUnicodeString(NameReq+length, sms->Name);
	length = length+UnicodeLength(sms->Name)*2;
	NameReq[length++] = 0;
	NameReq[length++] = 0;
	error=GSM_WaitFor (s, NameReq, length, 0x14, 4, ID_SaveSMSMessage);
	sms->Folder = folder;
	return error;
}

static GSM_Error N6510_SetSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	unsigned int		location;
	unsigned char		folderid;

	N6510_GetSMSLocation(s, sms, &folderid, &location);
	if (location == 0) return ERR_INVALIDLOCATION;
	return N6510_PrivSetSMSMessage(s, sms);
}

static GSM_Error N6510_AddSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	unsigned int		location;
	unsigned char		folderid;

	N6510_GetSMSLocation(s, sms, &folderid, &location);
	location = 0;
	N6510_SetSMSLocation(s, sms, folderid, location);
	return N6510_PrivSetSMSMessage(s, sms);
}

static GSM_Error N6510_ReplyGetDateTime(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Date & time received\n");
	if (msg.Buffer[4]==0x01) {
		NOKIA_DecodeDateTime(s, msg.Buffer+10, s->Phone.Data.DateTime);
		return ERR_NONE;
	}
	smprintf(s, "Not set in phone\n");
	return ERR_EMPTY;
}

static GSM_Error N6510_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x0A, 0x00, 0x00};

	s->Phone.Data.DateTime=date_time;
	smprintf(s, "Getting date & time\n");
	return GSM_WaitFor (s, req, 6, 0x19, 4, ID_GetDateTime);
}

static GSM_Error N6510_ReplySetDateTime(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Date & time set\n");
	return ERR_NONE;
}

static GSM_Error N6510_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	unsigned char req[]  = {N6110_FRAME_HEADER,
				0x01, 0x00, 0x01, 0x01, 0x0c, 0x01, 0x03,
				0x00, 0x00,	/* Year */
				0x08, 0x01,     /* Month & Day */
				0x15, 0x1f,	/* Hours & Minutes */
				0x2b,		/* Second ? */
				0x00};

	NOKIA_EncodeDateTime(s, req+10, date_time);
	req[16] = date_time->Second;
	smprintf(s, "Setting date & time\n");
	return GSM_WaitFor (s, req, 18, 0x19, 4, ID_SetDateTime);
}

static GSM_Error N6510_ReplyGetManufactureMonth(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	if (msg.Buffer[7] == 0x00) {
		smprintf(s, "No SIM card\n");
		return ERR_NOSIM;
	} else {
		sprintf(s->Phone.Data.PhoneString,"%02i/%04i",msg.Buffer[13],msg.Buffer[14]*256+msg.Buffer[15]);
	        return ERR_NONE;
	}
}

static GSM_Error N6510_GetManufactureMonth(GSM_StateMachine *s, char *value)
{
	unsigned char req[6] = {0x00, 0x05, 0x02, 0x01, 0x00, 0x02};
/* 	unsigned char req[6] = {0x00, 0x03, 0x04, 0x0B, 0x01, 0x00}; */

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) return ERR_NOTSUPPORTED;

	s->Phone.Data.PhoneString=value;
	smprintf(s, "Getting manufacture month\n");
	return GSM_WaitFor (s, req, 6, 0x42, 2, ID_GetManufactureMonth);
/* 	return GSM_WaitFor (s, req, 6, 0x1B, 2, ID_GetManufactureMonth); */
}

static GSM_Error N6510_ReplyGetAlarm(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	switch(msg.Buffer[3]) {
	case 0x1A:
		smprintf(s, "   Alarm: %02d:%02d\n", msg.Buffer[14], msg.Buffer[15]);
		Data->Alarm->Repeating 		= true;
		Data->Alarm->Text[0] 		= 0;
		Data->Alarm->Text[1] 		= 0;
		Data->Alarm->DateTime.Hour	= msg.Buffer[14];
		Data->Alarm->DateTime.Minute	= msg.Buffer[15];
		Data->Alarm->DateTime.Second	= 0;
		return ERR_NONE;
	case 0x20:
		smprintf(s, "Alarm state received\n");
		if (msg.Buffer[37] == 0x01) {
			smprintf(s, "   Not set in phone\n");
			return ERR_EMPTY;
		}
		smprintf(s, "Enabled\n");
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm)
{
	unsigned char   StateReq[] = {N6110_FRAME_HEADER, 0x1f, 0x01, 0x00};
	unsigned char   GetReq  [] = {N6110_FRAME_HEADER, 0x19, 0x00, 0x02};
	GSM_Error	error;

	if (Alarm->Location != 1) return ERR_NOTSUPPORTED;

	s->Phone.Data.Alarm=Alarm;
	smprintf(s, "Getting alarm state\n");
	error = GSM_WaitFor (s, StateReq, 6, 0x19, 4, ID_GetAlarm);
	if (error != ERR_NONE) return error;

	smprintf(s, "Getting alarm\n");
	return GSM_WaitFor (s, GetReq, 6, 0x19, 4, ID_GetAlarm);
}

static GSM_Error N6510_ReplySetAlarm(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Alarm set\n");
	return ERR_NONE;
}

static GSM_Error N6510_SetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm)
{
	unsigned char req[]  = {N6110_FRAME_HEADER,
				0x11, 0x00, 0x01, 0x01, 0x0c, 0x02,
				0x01, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00,		/* Hours, Minutes */
				0x00, 0x00, 0x00 };

	if (Alarm->Location != 1) return ERR_NOTSUPPORTED;

	req[14] = Alarm->DateTime.Hour;
	req[15] = Alarm->DateTime.Minute;

	smprintf(s, "Setting alarm\n");
	return GSM_WaitFor (s, req, 19, 0x19, 4, ID_SetAlarm);
}

static GSM_Error N6510_ReplyGetRingtonesInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 			tmp,i;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	smprintf(s, "Ringtones info received\n");
	memset(Data->RingtonesInfo,0,sizeof(GSM_AllRingtonesInfo));
	if (msg.Buffer[4] * 256 + msg.Buffer[5] == 0x00) return ERR_EMPTY;
	Data->RingtonesInfo->Number = msg.Buffer[4] * 256 + msg.Buffer[5];
	/*  allocate array of ringtones based on number */
	Data->RingtonesInfo->Ringtone = calloc(Data->RingtonesInfo->Number, sizeof(GSM_RingtoneInfo));
	tmp = 6;
	for (i=0;i<Data->RingtonesInfo->Number;i++) {
		Data->RingtonesInfo->Ringtone[i].Group = msg.Buffer[tmp+4];
		Data->RingtonesInfo->Ringtone[i].ID    = msg.Buffer[tmp+2] * 256 + msg.Buffer[tmp+3];
		memcpy(Data->RingtonesInfo->Ringtone[i].Name,msg.Buffer+tmp+8,(msg.Buffer[tmp+6]*256+msg.Buffer[tmp+7])*2);
		smprintf(s, "%5i (%5i). \"%s\"\n",
			Data->RingtonesInfo->Ringtone[i].ID,
			Data->RingtonesInfo->Ringtone[i].Group,
			DecodeUnicodeString(Data->RingtonesInfo->Ringtone[i].Name));
		tmp = tmp + (msg.Buffer[tmp]*256+msg.Buffer[tmp+1]);
	}
	return ERR_NONE;
}

static GSM_Error N6510_PrivGetRingtonesInfo(GSM_StateMachine *s, GSM_AllRingtonesInfo *Info, bool AllRingtones)
{
	GSM_Error	error;
	unsigned char 	UserReq[8] = {N7110_FRAME_HEADER, 0x07, 0x00, 0x00, 0x00, 0x02};
/* 	unsigned char 	All_Req[9] = {N7110_FRAME_HEADER, 0x07, 0x00, 0x00, 0xFE, 0x00, 0x7D}; */
	unsigned char 	All_Req[8] = {N7110_FRAME_HEADER, 0x07, 0x00, 0x00, 0x00, 0x00};

	s->Phone.Data.RingtonesInfo=Info;
	smprintf(s, "Getting binary ringtones ID\n");
	if (AllRingtones) {
/* 		error = GSM_WaitFor (s, All_Req, 9, 0x1f, 4, ID_GetRingtonesInfo); */
		error = GSM_WaitFor (s, All_Req, 8, 0x1f, 4, ID_GetRingtonesInfo);
		if (error == ERR_EMPTY && Info->Number == 0) return ERR_NOTSUPPORTED;
		return error;
	} else {
		error = GSM_WaitFor (s, UserReq, 8, 0x1f, 4, ID_GetRingtonesInfo);
		if (error == ERR_EMPTY && Info->Number == 0) return ERR_NOTSUPPORTED;
		return error;
	}
}

static GSM_Error N6510_GetRingtonesInfo(GSM_StateMachine *s, GSM_AllRingtonesInfo *Info)
{
	return N6510_PrivGetRingtonesInfo(s, Info, true);
}

static GSM_Error N6510_ReplyGetRingtone(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	size_t 			tmp,i;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	smprintf(s, "Ringtone received\n");
	memcpy(Data->Ringtone->Name,msg.Buffer+8,msg.Buffer[7]*2);
	Data->Ringtone->Name[msg.Buffer[7]*2]=0;
	Data->Ringtone->Name[msg.Buffer[7]*2+1]=0;
	smprintf(s, "Name \"%s\"\n",DecodeUnicodeString(Data->Ringtone->Name));
 	if (msg.Buffer[msg.Buffer[7]*2+10] == 'M' &&
 	    msg.Buffer[msg.Buffer[7]*2+11] == 'T' &&
 	    msg.Buffer[msg.Buffer[7]*2+12] == 'h' &&
 	    msg.Buffer[msg.Buffer[7]*2+13] == 'd') {
 		smprintf(s,"MIDI\n");
 		tmp 	= msg.Buffer[7]*2+10;
 		i 	= msg.Length - 2; /* ?????? */
 		Data->Ringtone->Format = RING_MIDI;
 	} else {
 		/* Looking for end */
 		i=8+msg.Buffer[7]*2+3;
 		tmp = i;
 		while (true) {
 			if (msg.Buffer[i]==0x07 && msg.Buffer[i+1]==0x0b) {
 				i=i+2; break;
 			}
 			i++;
 			if (i==msg.Length) return ERR_EMPTY;
 		}
	}
	/* Copying frame */
	memcpy(Data->Ringtone->NokiaBinary.Frame,msg.Buffer+tmp,i-tmp);
	Data->Ringtone->NokiaBinary.Length=i-tmp;
	return ERR_NONE;
}

static GSM_Error N6510_GetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, bool PhoneRingtone)
{
 	GSM_AllRingtonesInfo 	Info = {0, NULL};
	GSM_Error		error;
	unsigned char 		req2[6] = {N7110_FRAME_HEADER, 0x12,
					   0x00, 0xe7}; 	/* Location */

	if (Ringtone->Format == 0x00) Ringtone->Format = RING_NOKIABINARY;

	switch (Ringtone->Format) {
	case RING_NOTETONE:
		/* In the future get binary and convert */
		return ERR_NOTSUPPORTED;
	case RING_NOKIABINARY:
		s->Phone.Data.Ringtone	= Ringtone;
		Info.Number 		= 0;
		error=N6510_PrivGetRingtonesInfo(s, &Info, PhoneRingtone);
		if (error != ERR_NONE) return error;
		if (Ringtone->Location > Info.Number) return ERR_INVALIDLOCATION;
		req2[4] = Info.Ringtone[Ringtone->Location-1].ID / 256;
		req2[5] = Info.Ringtone[Ringtone->Location-1].ID % 256;
		smprintf(s, "Getting binary ringtone\n");
 		error = GSM_WaitFor (s, req2, 6, 0x1f, 4, ID_GetRingtone);
 		if (Info.Ringtone) free(Info.Ringtone);
 		return error;
	case RING_MIDI:
	case RING_MMF:
		return ERR_NOTSUPPORTED;
	}
	return ERR_NOTSUPPORTED;
}

static GSM_Error N6510_PlayTone(GSM_StateMachine *s, int Herz, unsigned char Volume, bool start)
{
	GSM_Error 	error;
	unsigned char 	reqStart[] = {
		0x00,0x06,0x01,0x00,0x07,0x00 };
	unsigned char 	reqPlay[] = {
		0x00,0x06,0x01,0x14,0x05,0x04,
		0x00,0x00,0x00,0x03,0x03,0x08,
		0x00,0x00,0x00,0x01,0x00,0x00,
		0x03,0x08,0x01,0x00,
		0x07,0xd0,	/*Frequency */
		0x00,0x00,0x03,0x08,0x02,0x00,0x00,
		0x05,		/*Volume */
		0x00,0x00};
	unsigned char 	reqOff[] = {
		0x00,0x06,0x01,0x14,0x05,0x05,
		0x00,0x00,0x00,0x01,0x03,0x08,
		0x05,0x00,0x00,0x08,0x00,0x00};
/* 	unsigned char 	reqOff2[] = { */
/* 		0x00,0x06,0x01,0x14,0x05,0x04, */
/* 		0x00,0x00,0x00,0x01,0x03,0x08, */
/* 		0x00,0x00,0x00,0x00,0x00,0x00}; */

	if (start) {
		smprintf(s, "Enabling sound - part 1\n");
		error=GSM_WaitFor (s, reqStart, 6, 0x0b, 4, ID_PlayTone);
		if (error!=ERR_NONE) return error;
		smprintf(s, "Enabling sound - part 2 (disabling sound command)\n");
		error=GSM_WaitFor (s, reqOff, 18, 0x0b, 4, ID_PlayTone);
		if (error!=ERR_NONE) return error;
	}

	/* For Herz==255*255 we have silent */
	if (Herz!=255*255) {
		reqPlay[23] = Herz%256;
		reqPlay[22] = Herz/256;
		reqPlay[31] = Volume;
		smprintf(s, "Playing sound\n");
		return GSM_WaitFor (s, reqPlay, 34, 0x0b, 4, ID_PlayTone);
	} else {
		reqPlay[23] = 0;
		reqPlay[22] = 0;
		reqPlay[31] = 0;
		smprintf(s, "Playing silent sound\n");
		return GSM_WaitFor (s, reqPlay, 34, 0x0b, 4, ID_PlayTone);

/* 		smprintf(s, "Disabling sound - part 1\n"); */
/* 		error=GSM_WaitFor (s, reqOff, 18, 0x0b, 4, ID_PlayTone); */
/* 		if (error!=ERR_NONE) return error; */
/* 		smprintf(s, "Disabling sound - part 2\n"); */
/* 		return GSM_WaitFor (s, reqOff2, 18, 0x0b, 4, ID_PlayTone); */
	}
}

static GSM_Error N6510_ReplyGetPPM(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data 	*Data = &s->Phone.Data;
	size_t		pos = 6,len;

	smprintf(s, "Received phone info\n");

	while(pos < msg.Length) {
		if (msg.Buffer[pos] == 0x55 && msg.Buffer[pos+1] == 0x55) {
			while(1) {
				if (msg.Buffer[pos] != 0x55) break;
				pos++;
			}
		}
		len = pos;
		while(len < msg.Length) {
			if (msg.Buffer[len] == 0x00 && msg.Buffer[len+1] == 0x00) break;
			len++;
		}
		while(len < msg.Length) {
			if (msg.Buffer[len] != 0x00) break;
			len++;
		}
		len = len-pos;
		smprintf(s, "Block with ID %02x",msg.Buffer[pos]);
#ifdef DEBUG
		if (s->di.dl == DL_TEXTALL || s->di.dl == DL_TEXTALLDATE) DumpMessage(&s->di, msg.Buffer+pos, len);
#endif
		switch (msg.Buffer[pos]) {
		case 0x49:
			smprintf(s, "hardware version\n");
			break;
		case 0x58:
			pos += 3;
			while (msg.Buffer[pos] != 0x00) pos++;
			Data->PhoneString[0] = msg.Buffer[pos - 1];
			Data->PhoneString[1] = 0x00;
			smprintf(s, "PPM %s\n",Data->PhoneString);
			return ERR_NONE;
		default:
			break;
		}
		pos += len;
	}
	return ERR_NOTSUPPORTED;
}

static GSM_Error N6510_GetPPM(GSM_StateMachine *s,char *value)
{
/* 	unsigned char req[6] = {N6110_FRAME_HEADER, 0x07, 0x01, 0xff}; */
	unsigned char req[6] = {N6110_FRAME_HEADER, 0x07, 0x01, 0x00};

	s->Phone.Data.PhoneString=value;
	smprintf(s, "Getting PPM\n");
	return GSM_WaitFor (s, req, 6, 0x1b, 3, ID_GetPPM);
}

static GSM_Error N6510_GetSpeedDial(GSM_StateMachine *s, GSM_SpeedDial *SpeedDial)
{
	GSM_MemoryEntry 	pbk;
	GSM_Error		error;

	pbk.MemoryType			= MEM7110_SP;
	pbk.Location			= SpeedDial->Location;
	SpeedDial->MemoryLocation 	= 0;
	s->Phone.Data.SpeedDial		= SpeedDial;

	smprintf(s, "Getting speed dial\n");
	error=N6510_GetMemory(s,&pbk);
	switch (error) {
	case ERR_NOTSUPPORTED:
		smprintf(s, "No speed dials set in phone\n");
		return ERR_EMPTY;
	case ERR_NONE:
		if (SpeedDial->MemoryLocation == 0) {
			smprintf(s, "Speed dial not assigned or error in firmware\n");
			return ERR_EMPTY;
		}
		return ERR_NONE;
	default:
		return error;
	}
}

static GSM_Error N6510_ReplyGetProfile(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 	*blockstart;
	int 		i,j;
	GSM_Phone_Data	*Data = &s->Phone.Data;

	switch (msg.Buffer[3]) {
	case 0x02:
	blockstart = msg.Buffer + 7;
	for (i = 0; i < 11; i++) {
		smprintf(s, "Profile feature %02x ",blockstart[1]);
#ifdef DEBUG
		if (s->di.dl == DL_TEXTALL || s->di.dl == DL_TEXTALLDATE) DumpMessage(&s->di, blockstart, blockstart[0]);
#endif

		switch (blockstart[1]) {
		case 0x03:
			smprintf(s, "Ringtone ID\n");
			Data->Profile->FeatureID	[Data->Profile->FeaturesNumber] = Profile_RingtoneID;
			Data->Profile->FeatureValue	[Data->Profile->FeaturesNumber] = blockstart[7];
			if (blockstart[7] == 0x00) {
				Data->Profile->FeatureValue[Data->Profile->FeaturesNumber] = blockstart[10];
			}
			Data->Profile->FeaturesNumber++;
			break;
		case 0x05:	/* SMS tone */
			j = Data->Profile->FeaturesNumber;
			NOKIA_FindFeatureValue(s, Profile71_65,blockstart[1],blockstart[7],Data,false);
			if (j == Data->Profile->FeaturesNumber) {
				Data->Profile->FeatureID	[Data->Profile->FeaturesNumber] = Profile_MessageTone;
				Data->Profile->FeatureValue	[Data->Profile->FeaturesNumber] = PROFILE_MESSAGE_PERSONAL;
				Data->Profile->FeaturesNumber++;
				Data->Profile->FeatureID	[Data->Profile->FeaturesNumber] = Profile_MessageToneID;
				Data->Profile->FeatureValue	[Data->Profile->FeaturesNumber] = blockstart[7];
				Data->Profile->FeaturesNumber++;
			}
			break;
		case 0x08:	/* Caller groups */
			NOKIA_FindFeatureValue(s, Profile71_65,blockstart[1],blockstart[7],Data,true);
			break;
		case 0x0c :
			CopyUnicodeString(Data->Profile->Name,blockstart + 7);
			smprintf(s, "profile Name: \"%s\"\n", DecodeUnicodeString(Data->Profile->Name));
			Data->Profile->DefaultName = false;
			break;
		default:
			NOKIA_FindFeatureValue(s, Profile71_65,blockstart[1],blockstart[7],Data,false);
		}
		blockstart = blockstart + blockstart[0];
	}
	return ERR_NONE;
	case 0x06:
		Data->Profile->Active = false;
		if (Data->Profile->Location == msg.Buffer[5]) Data->Profile->Active = true;
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetProfile(GSM_StateMachine *s, GSM_Profile *Profile)
{
	unsigned char 	req[150]    = {N6110_FRAME_HEADER, 0x01, 0x01, 0x0C, 0x01};
	unsigned char	reqActive[] = {N6110_FRAME_HEADER, 0x05};
	int 		i, length = 7;
	GSM_Error	error;

	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_PROFILES)) return ERR_NOTSUPPORTED;

/* 	if (!strcmp(s->Phone.Data.ModelInfo->model,"3510")) { */
/* 		if (s->Phone.Data.VerNum>3.37) return ERR_NOTSUPPORTED; */
/* 	if (!strcmp(s->Phone.Data.ModelInfo->model,"6230")) { */
/* 	if (!strcmp(s->Phone.Data.ModelInfo->model,"6220")) { */
/* 	if (!strcmp(s->Phone.Data.ModelInfo->model,"5140")) { */
/* 	if (!strcmp(s->Phone.Data.ModelInfo->model,"6230i")) { */
/* 	if (!strcmp(s->Phone.Data.ModelInfo->model,"6020")) { */

	if (Profile->Location>5) return ERR_INVALIDLOCATION;

	for (i = 0; i < 0x0a; i++) {
		req[length++] = 0x04;
		req[length++] = Profile->Location;
		req[length++] = i;
		req[length++] = 0x01;
	}

	req[length++] = 0x04;
	req[length++] = Profile->Location;
	req[length++] = 0x0c;
	req[length++] = 0x01;

	req[length++] = 0x04;

	Profile->CarKitProfile	= false;
	Profile->HeadSetProfile	= false;

	Profile->FeaturesNumber = 0;

	s->Phone.Data.Profile=Profile;
	smprintf(s, "Getting profile\n");
	error = GSM_WaitFor (s, req, length, 0x39, 4, ID_GetProfile);
	if (error != ERR_NONE) return error;

	smprintf(s, "Checking, which profile is active\n");
	return GSM_WaitFor (s, reqActive, 4, 0x39, 4, ID_GetProfile);
}

static GSM_Error N6510_ReplySetProfile(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 	*blockstart;
	int 		i;

	smprintf(s, "Response to profile writing received!\n");

	blockstart = msg.Buffer + 6;
	for (i = 0; i < msg.Buffer[5]; i++) {
		switch (blockstart[2]) {
			case 0x00: smprintf(s, "keypad tone level"); 	break;
			case 0x02: smprintf(s, "call alert"); 		break;
			case 0x03: smprintf(s, "ringtone"); 		break;
			case 0x04: smprintf(s, "ringtone volume"); 	break;
			case 0x05: smprintf(s, "SMS tone"); 		break;
			case 0x06: smprintf(s, "vibration"); 		break;
			case 0x07: smprintf(s, "warning tone level"); 	break;
			case 0x08: smprintf(s, "caller groups"); 	break;
			case 0x09: smprintf(s, "automatic answer"); 	break;
			case 0x0c: smprintf(s, "name"); 		break;
			default:
				smprintf(s, "Unknown block type %02x", blockstart[2]);
				break;
		}
		if (msg.Buffer[4] == 0x00) {
			smprintf(s, ": set OK\n");
		} else {
			smprintf(s, ": setting error %i\n", msg.Buffer[4]);
		}
		blockstart = blockstart + blockstart[1];
	}
	return ERR_NONE;
}

static GSM_Error N6510_SetProfile(GSM_StateMachine *s, GSM_Profile *Profile)
{
	int 		i, length = 7, blocks = 0;
	bool		found;
	unsigned char	ID,Value;
	unsigned char 	req[150] = {N6110_FRAME_HEADER, 0x03, 0x01,
				    0x06,		/* Number of blocks */
				    0x03};

	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_PROFILES)) return ERR_NOTSUPPORTED;

	if (Profile->Location>5) return ERR_INVALIDLOCATION;

	for (i=0;i<Profile->FeaturesNumber;i++) {
		found = false;
		switch (Profile->FeatureID[i]) {
			case Profile_RingtoneID:
				ID 	= 0x03;
				Value 	= Profile->FeatureValue[i];
				found 	= true;
				break;
			default:
				found=NOKIA_FindPhoneFeatureValue(
					s,
					Profile71_65,
					Profile->FeatureID[i],Profile->FeatureValue[i],
					&ID,&Value);
		}
		if (found) {
			req[length] 	= 0x09;
			req[length + 1] = ID;
			req[length + 2] = Profile->Location;
			memcpy(req + length + 4, "\x00\x00\x01", 3);
			req[length + 8] = 0x03;
			req[length + 3] = req[length + 7] = Value;
			blocks++;
			length += 9;
		}
	}

	smprintf(s, "Setting profile\n");
	return GSM_WaitFor (s, req, length, 0x39, 4, ID_SetProfile);
}

static GSM_Error N6510_ReplyIncomingCB(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
		case 0x21:
			smprintf(s, "Ignoring cell broadcast ok event\n");
			break;
		case 0x22:
			smprintf(s, "Ignoring cell broadcast fail event\n");
			break;
		case 0x23:
			smprintf(s, "Ignoring cell broadcast read event\n");
			break;
		default:
			smprintf(s, "Ignoring cell broadcast event 0x%02x\n", msg.Buffer[3]);
			break;
	}
	return ERR_NONE;
}
static GSM_Error N6510_ReplyIncomingSMS(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_SMSMessage 	sms;
	size_t 		i;

#ifdef DEBUG
	smprintf(s, "SMS message received\n");
	N6510_DecodeSMSFrame(s, &sms, msg.Buffer+10,&i);
#endif

	if (s->Phone.Data.EnableIncomingSMS && s->User.IncomingSMS!=NULL) {
		GSM_SetDefaultSMSData(&sms);

		sms.State 	 = SMS_UnRead;
		sms.InboxFolder  = true;

		N6510_DecodeSMSFrame(s, &sms, msg.Buffer+10,&i);

		s->User.IncomingSMS(s,sms);
	}
	return ERR_NONE;
}

static GSM_Error N6510_DialVoice(GSM_StateMachine *s, char *number, GSM_CallShowNumber ShowNumber)
{
	unsigned int 	pos2 = 15;
	unsigned int	pos = 4;
	unsigned char 	req2[100] = {N6110_FRAME_HEADER,0x01,
				    0x00,0x02,0x07,0x04,
				    0x01,  /*  1 - voice, 2 - data */
				    0x00,0x03,
				    0x18,  /*  length of rest + 1 */
				    0x00,0x00,0x00};
	unsigned char 	req[100] = {N6110_FRAME_HEADER,0x01,
				    0x0c};	/* Number length */
	GSM_Error	error;

	/* USSD not supported */
	if (number[0] == '*') return ERR_NOTSUPPORTED;
	if (number[0] == '#') return ERR_NOTSUPPORTED;

	req[pos++] = strlen(number);
	EncodeUnicode(req+pos,number,strlen(number));
	pos += strlen(number)*2;
	req[pos++] = 0x05; /* call type: voice - 0x05, data - 0x01 */
	req[pos++] = 0x01;
	req[pos++] = 0x05;
	req[pos++] = 0x00;
	req[pos++] = 0x02;
	req[pos++] = 0x00;
	req[pos++] = 0x00;
	switch (ShowNumber) {
	case GSM_CALL_HideNumber:
		req[pos++] = 0x02;
		break;
	case GSM_CALL_ShowNumber:
		req[pos++] = 0x03;
		break;
	case GSM_CALL_DefaultNumberPresence:
		req[pos++] = 0x01;
		break;
	}
	smprintf(s, "Making voice call\n");
	error = GSM_WaitFor (s, req, pos, 0x01, 4, ID_DialVoice);
	if (error != ERR_NOTSUPPORTED) return error;

	if (ShowNumber != GSM_CALL_DefaultNumberPresence) return ERR_NOTSUPPORTED;

	req2[11] = strlen(number)*2+6;
	req2[pos2++] = strlen(number);
	EncodeUnicode(req2+pos2,number,strlen(number));
	pos2 += strlen(number)*2;

	smprintf(s, "Making voice call\n");
	error = GSM_WaitFor (s, req2, pos2, 0x01, 4, ID_DialVoice);
	if (error == ERR_NOTSUPPORTED) return ERR_NONE;
	return error;
}

static GSM_Error N6510_ReplyLogIntoNetwork(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Probably phone says: I log into network\n");
	return ERR_NONE;
}

void N6510_EncodeFMFrequency(double freq, unsigned char *buff)
{
	unsigned int		freq2;

 	freq2 = (unsigned int)(freq * 100);
 	freq2	= freq2 - 0xffff;
 	buff[0] = freq2 / 0x100;
 	buff[1] = freq2 % 0x100;
}

void N6510_DecodeFMFrequency(double *freq, unsigned char *buff)
{
	*freq = (double)(0xffff + buff[0] * 0x100 + buff[1])/1000.0;
}

static GSM_Error N6510_ReplyGetFMStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
  	smprintf(s, "getting FM status OK\n");
	memcpy(s->Phone.Data.Priv.N6510.FMStatus,msg.Buffer,msg.Length);
	s->Phone.Data.Priv.N6510.FMStatusLength = msg.Length;
	return ERR_NONE;
}

static GSM_Error N6510_GetFMStatus(GSM_StateMachine *s)
{
 	unsigned char req[7] = {N6110_FRAME_HEADER, 0x0d, 0x00, 0x00, 0x01};

 	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_RADIO)) return ERR_NOTSUPPORTED;
 	return GSM_WaitFor (s, req, 7, 0x3E, 2, ID_GetFMStation);
}

static GSM_Error N6510_ReplyGetFMStation(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
  	unsigned char 	name[GSM_MAX_FMSTATION_LENGTH*2+2];
  	int		length;
  	GSM_Phone_Data	*Data = &s->Phone.Data;

	switch (msg.Buffer[3]) {
	case 0x06:
	  	smprintf(s, "Received FM station\n");
 		length = msg.Buffer[8];
		if (length > GSM_MAX_FMSTATION_LENGTH) {
			smprintf(s, "FM station name too long (%d), truncating!\n", length);
			length = GSM_MAX_FMSTATION_LENGTH;
		}
 		memcpy(name,msg.Buffer+18,length*2);
 		name[length*2]	 = 0x00;
 		name[length*2+1] = 0x00;
 		CopyUnicodeString(Data->FMStation->StationName,name);
		smprintf(s,"Station name: \"%s\"\n",DecodeUnicodeString(Data->FMStation->StationName));
		N6510_DecodeFMFrequency(&Data->FMStation->Frequency, msg.Buffer+16);
		return ERR_NONE;
	case 0x16:
	  	smprintf(s, "Received FM station. Empty ?\n");
		return ERR_EMPTY;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetFMStation (GSM_StateMachine *s, GSM_FMStation *FMStation)
{
 	GSM_Error 		error;
 	int			location;
  	unsigned char 		req[7] = {N6110_FRAME_HEADER, 0x05,
 					  0x00, 		/*  location */
	    				  0x00,0x01};

  	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_RADIO)) return ERR_NOTSUPPORTED;
   	if (FMStation->Location > GSM_MAX_FM_STATION) return ERR_INVALIDLOCATION;

  	s->Phone.Data.FMStation = FMStation;

 	error = N6510_GetFMStatus(s);
 	if (error != ERR_NONE) return error;

 	location = FMStation->Location-1;
  	if (s->Phone.Data.Priv.N6510.FMStatus[14+location] == 0xFF) return ERR_EMPTY;
 	req[4]   = s->Phone.Data.Priv.N6510.FMStatus[14+location];

 	smprintf(s, "Getting FM Station %i\n",FMStation->Location);
 	return GSM_WaitFor (s, req, 7, 0x3E, 2, ID_GetFMStation);
}

static GSM_Error N6510_ReplySetFMStation(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
#ifdef DEBUG
 	switch (msg.Buffer[4]){
 		case 0x03: smprintf(s, "FM stations cleaned\n");	  break;
 		case 0x11: smprintf(s, "Setting FM station status OK\n"); break;
 		case 0x12: smprintf(s, "Setting FM station OK\n");	  break;
 	}
#endif
 	return ERR_NONE;
}

static GSM_Error N6510_ClearFMStations (GSM_StateMachine *s)
{
	unsigned char req[7] = {N6110_FRAME_HEADER, 0x03,0x0f,0x00,0x01};

	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_RADIO)) return ERR_NOTSUPPORTED;

	smprintf(s, "Cleaning FM Stations\n");
	return GSM_WaitFor (s, req, 7, 0x3E, 2, ID_SetFMStation);
}

static GSM_Error N6510_SetFMStation (GSM_StateMachine *s, GSM_FMStation *FMStation)
{
	unsigned int 		len, location;
 	GSM_Error 		error;
 	unsigned char setstatus[36] = {N6110_FRAME_HEADER,0x11,0x00,0x01,0x01,
 	    			0x00,0x00,0x1c,0x00,0x14,0x00,0x00,
 				0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 				0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 				0xff,0xff,0xff,0xff,0xff,0x01};
 	unsigned char req[64] = {N6110_FRAME_HEADER, 0x12,0x00,0x01,0x00,
 				0x00, 			/*  0x0e + (strlen(name) * 2) */
 				0x00,			/*  strlen(name) */
 				0x14,0x09,0x00,
 				0x00, 			/*  location */
 				0x00,0x00,0x01,
 				0x00, 			/*  freqHi */
 				0x00, 			/*  freqLo */
 				0x01};

 	if (!GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_RADIO)) return ERR_NOTSUPPORTED;

 	s->Phone.Data.FMStation = FMStation;
 	location = FMStation->Location-1;

 	error = N6510_GetFMStatus(s);
 	if (error != ERR_NONE) return error;

 	memcpy(setstatus+14,s->Phone.Data.Priv.N6510.FMStatus+14,20);
 	setstatus [14+location] = location;

 	smprintf(s, "Setting FM status %i\n",FMStation->Location);
 	error = GSM_WaitFor (s, setstatus, 36 , 0x3E, 2, ID_SetFMStation);
 	if (error != ERR_NONE) return error;

 	req[12] = location;

	/* Name */
 	len 	= UnicodeLength(FMStation->StationName);
 	req[8] 	= len;
 	req[7] 	= 0x0e + len * 2;
 	memcpy (req+18,FMStation->StationName,len*2);

	/* Frequency */
	N6510_EncodeFMFrequency(FMStation->Frequency, req+16);

 	smprintf(s, "Setting FM Station %i\n",FMStation->Location);
 	return GSM_WaitFor (s, req, 0x13+len*2, 0x3E, 2, ID_SetFMStation);
}

static GSM_Error N6510_ReplySetLight(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Light set\n");
	return ERR_NONE;
}

GSM_Error N6510_SetLight(GSM_StateMachine *s, N6510_PHONE_LIGHTS light, bool enable)
{
	unsigned char req[14] = {
		N6110_FRAME_HEADER, 0x05,
		0x01,		/* 0x01 = Display, 0x03 = keypad */
		0x01,		/* 0x01 = Enable, 0x02 = disable */
		0x00, 0x00, 0x00, 0x01,
		0x05, 0x04, 0x02, 0x00};

	req[4] = light;
	if (!enable) req[5] = 0x02;
 	smprintf(s, "Setting light\n");
	return GSM_WaitFor (s, req, 14, 0x3A, 4, ID_SetLight);
}

static GSM_Error N6510_ShowStartInfo(GSM_StateMachine *s, bool enable)
{
	GSM_Error error;

	if (enable) {
		error=N6510_SetLight(s,N6510_LIGHT_DISPLAY,true);
		if (error != ERR_NONE) return error;

		error=N6510_SetLight(s,N6510_LIGHT_TORCH,true);
		if (error != ERR_NONE) return error;

		return N6510_SetLight(s,N6510_LIGHT_KEYPAD,true);
	} else {
		error=N6510_SetLight(s,N6510_LIGHT_DISPLAY,false);
		if (error != ERR_NONE) return error;

		error=N6510_SetLight(s,N6510_LIGHT_TORCH,false);
		if (error != ERR_NONE) return error;

		return N6510_SetLight(s,N6510_LIGHT_KEYPAD,false);
	}
}

#ifdef DEVELOP

static GSM_Error N6510_ReplyEnableGPRSAccessPoint(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	if (msg.Buffer[13] == 0x02) return ERR_NONE;
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_EnableGPRSAccessPoint(GSM_StateMachine *s)
{
	GSM_Error	error;
	int	 	i;
	unsigned char 	req[] = {
		N7110_FRAME_HEADER, 0x05, 0x00, 0x00, 0x00, 0x2C, 0x00,
		0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00};

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOGPRSPOINT)) return ERR_NOTSUPPORTED;

	for (i=0;i<3;i++) {
		smprintf(s, "Activating full GPRS access point support\n");
		error = GSM_WaitFor (s, req, 16, 0x43, 4, ID_EnableGPRSPoint);
		if (error != ERR_NONE) return error;
	}
	return error;
}

#endif

static GSM_Error N6510_ReplyGetGPRSAccessPoint(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_GPRSAccessPoint *point = s->Phone.Data.GPRSPoint;

	switch (msg.Buffer[13]) {
	case 0x01:
		smprintf(s,"Active GPRS point received\n");
		point->Active = false;
		if (point->Location == msg.Buffer[18]) point->Active = true;
		return ERR_NONE;
	case 0xD2:
		smprintf(s,"Names for GPRS points received\n");
		CopyUnicodeString(point->Name,msg.Buffer+18+(point->Location-1)*42);
		smprintf(s,"\"%s\"\n",DecodeUnicodeString(point->Name));
		return ERR_NONE;
	case 0xF2:
		smprintf(s,"URL for GPRS points received\n");
		CopyUnicodeString(point->URL,msg.Buffer+18+(point->Location-1)*202);
		smprintf(s,"\"%s\"\n",DecodeUnicodeString(point->URL));
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetGPRSAccessPoint(GSM_StateMachine *s, GSM_GPRSAccessPoint *point)
{
	GSM_Error	error;
	unsigned char 	URL[] = {
		N7110_FRAME_HEADER, 0x05, 0x00, 0x00, 0x00, 0x2C, 0x00,
		0x00, 0x00, 0x00, 0x03, 0xF2, 0x00, 0x00};
	unsigned char 	Name[] = {
		N7110_FRAME_HEADER, 0x05, 0x00, 0x00, 0x00, 0x2C, 0x00,
		0x01, 0x00, 0x00, 0x00, 0xD2, 0x00, 0x00};
	unsigned char 	Active[] = {
		N7110_FRAME_HEADER, 0x05, 0x00, 0x00, 0x00, 0x2C, 0x00,
		0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOGPRSPOINT)) return ERR_NOTSUPPORTED;
	if (point->Location < 1) return ERR_UNKNOWN;
	if (point->Location > 5) return ERR_INVALIDLOCATION;

	s->Phone.Data.GPRSPoint = point;

#ifdef DEVELOP
	error = N6510_EnableGPRSAccessPoint(s);
	if (error != ERR_NONE) return error;
#endif

	smprintf(s, "Getting GPRS access point name\n");
	error=GSM_WaitFor (s, Name, 16, 0x43, 4, ID_GetGPRSPoint);
	if (error != ERR_NONE) return error;

	smprintf(s, "Getting GPRS access point URL\n");
	error=GSM_WaitFor (s, URL, 16, 0x43, 4, ID_GetGPRSPoint);
	if (error != ERR_NONE) return error;

	smprintf(s, "Getting number of active GPRS access point\n");
	error=GSM_WaitFor (s, Active, 16, 0x43, 4, ID_GetGPRSPoint);
	if (error != ERR_NONE) return error;

	if (UnicodeLength(point->URL)==0 && UnicodeLength(point->Name)==0) return ERR_EMPTY;
	return error;
}

static GSM_Error N6510_ReplySetGPRSAccessPoint1(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[13]) {
	case 0x01:
	case 0xD2:
	case 0xF2:
		memcpy(s->Phone.Data.Priv.N6510.GPRSPoints,msg.Buffer,msg.Length);
		s->Phone.Data.Priv.N6510.GPRSPointsLength = msg.Length;
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_SetGPRSAccessPoint(GSM_StateMachine *s, GSM_GPRSAccessPoint *point)
{
	unsigned char	*buff = s->Phone.Data.Priv.N6510.GPRSPoints;
	GSM_Error	error;
	unsigned char 	URL[] = {
		N7110_FRAME_HEADER, 0x05, 0x00, 0x00, 0x00, 0x2C, 0x00,
		0x00, 0x00, 0x00, 0x03, 0xF2, 0x00, 0x00};
	unsigned char 	Name[] = {
		N7110_FRAME_HEADER, 0x05, 0x00, 0x00, 0x00, 0x2C, 0x00,
		0x01, 0x00, 0x00, 0x00, 0xD2, 0x00, 0x00};
	unsigned char 	Active[] = {
		N7110_FRAME_HEADER, 0x05, 0x00, 0x00, 0x00, 0x2C, 0x00,
		0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOGPRSPOINT)) return ERR_NOTSUPPORTED;
	if (point->Location < 1) return ERR_UNKNOWN;
	if (point->Location > 5) return ERR_INVALIDLOCATION;

	s->Phone.Data.GPRSPoint = point;

#ifdef DEVELOP
	error = N6510_EnableGPRSAccessPoint(s);
	if (error != ERR_NONE) return error;
#endif

	smprintf(s, "Getting GPRS access point name\n");
	error=GSM_WaitFor (s, Name, 16, 0x43, 4, ID_SetGPRSPoint);
	if (error != ERR_NONE) return error;
	CopyUnicodeString(buff+18+(point->Location-1)*42,point->Name);
	buff[0] = 0x00;
	buff[1] = 0x01;
	buff[2] = 0x01;
	buff[3] = 0x07;
	smprintf(s, "Setting GPRS access point name\n");
	error=GSM_WaitFor (s, buff, s->Phone.Data.Priv.N6510.GPRSPointsLength, 0x43, 4, ID_SetGPRSPoint);
	if (error != ERR_NONE) return error;

	smprintf(s, "Getting GPRS access point URL\n");
	error=GSM_WaitFor (s, URL, 16, 0x43, 4, ID_SetGPRSPoint);
	if (error != ERR_NONE) return error;
	CopyUnicodeString(buff+18+(point->Location-1)*42,point->URL);
	buff[0] = 0x00;
	buff[1] = 0x01;
	buff[2] = 0x01;
	buff[3] = 0x07;
	smprintf(s, "Setting GPRS access point URL\n");
	error=GSM_WaitFor (s, buff, s->Phone.Data.Priv.N6510.GPRSPointsLength, 0x43, 4, ID_SetGPRSPoint);
	if (error != ERR_NONE) return error;

	if (point->Active) {
		smprintf(s, "Getting number of active GPRS access point\n");
		error=GSM_WaitFor (s, Active, 16, 0x43, 4, ID_SetGPRSPoint);
		if (error != ERR_NONE) return error;
		buff[0] = 0x00;
		buff[1] = 0x01;
		buff[2] = 0x01;
		buff[3] = 0x07;
		buff[18]= point->Location;
		smprintf(s, "Setting number of active GPRS access point\n");
		error=GSM_WaitFor (s, buff, s->Phone.Data.Priv.N6510.GPRSPointsLength, 0x43, 4, ID_SetGPRSPoint);
		if (error != ERR_NONE) return error;
	}

	return error;
}

static GSM_Error N6510_ReplyGetLocale(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Locale *locale = s->Phone.Data.Locale;

	switch (msg.Buffer[3]) {
	case 0x8A:
		smprintf(s, "Date settings received\n");
		switch (msg.Buffer[4]) {
		case 0x00:
			locale->DateFormat 	= GSM_Date_DDMMYYYY;
			locale->DateSeparator 	= '.';
			break;
		case 0x01:
			locale->DateFormat 	= GSM_Date_MMDDYYYY;
			locale->DateSeparator 	= '.';
			break;
		case 0x02:
			locale->DateFormat 	= GSM_Date_YYYYMMDD;
			locale->DateSeparator 	= '.';
			break;
		case 0x04:
			locale->DateFormat 	= GSM_Date_DDMMYYYY;
			locale->DateSeparator 	= '/';
			break;
		case 0x05:
			locale->DateFormat 	= GSM_Date_MMDDYYYY;
			locale->DateSeparator 	= '/';
			break;
		case 0x06:
			locale->DateFormat 	= GSM_Date_YYYYMMDD;
			locale->DateSeparator 	= '/';
			break;
		case 0x08:
			locale->DateFormat 	= GSM_Date_DDMMYYYY;
			locale->DateSeparator 	= '-';
			break;
		case 0x09:
			locale->DateFormat 	= GSM_Date_MMDDYYYY;
			locale->DateSeparator 	= '-';
			break;
		case 0x0A:
			locale->DateFormat 	= GSM_Date_YYYYMMDD;
			locale->DateSeparator 	= '-';
			break;
		default:/* FIXME */
			locale->DateFormat 	= GSM_Date_DDMMYYYY;
			locale->DateSeparator 	= '/';
			break;
		}
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetLocale(GSM_StateMachine *s, GSM_Locale *locale)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x89};

	s->Phone.Data.Locale = locale;

	smprintf(s, "Getting date format\n");
	return GSM_WaitFor (s, req, 4, 0x13, 4, ID_GetLocale);
}

GSM_Error N6510_CancelCall(GSM_StateMachine *s, int ID, bool all)
{
	if (all) return ERR_NOTSUPPORTED;
	return DCT3DCT4_CancelCall(s,ID);
}

GSM_Error N6510_AnswerCall(GSM_StateMachine *s, int ID, bool all)
{
	if (all) return ERR_NOTSUPPORTED;
	return DCT3DCT4_AnswerCall(s,ID);
}

static GSM_Error N6510_ReplyAddSMSFolder(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s,"SMS folder \"%s\" has been added\n",DecodeUnicodeString(msg.Buffer+10));
	return ERR_NONE;
}

GSM_Error N6510_AddSMSFolder(GSM_StateMachine *s, unsigned char *name)
{
	unsigned char req[200] = {N6110_FRAME_HEADER, 0x10, 0x01, 0x00, 0x01,
			          0x00,     		/* Length */
				  0x00, 0x00};

	CopyUnicodeString(req+10,name);
	req[7] = UnicodeLength(name)*2 + 6;

	smprintf(s, "Adding SMS folder\n");
	return GSM_WaitFor (s, req, req[7] + 6, 0x14, 4, ID_AddSMSFolder);
}

static GSM_Error N6510_SetWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error 	error;
	int 		count=4, location;
	unsigned char 	req[600] = {N6110_FRAME_HEADER, 0x09};

	/* We have to enable WAP frames in phone */
	error=N6510_EnableConnectionFunctions(s,N6510_WAP_SETTINGS);
	if (error!=ERR_NONE) return error;

	location = bookmark->Location - 1;
	if (bookmark->Location == 0) location = 0xffff;
	req[count++] = (location & 0xff00) >> 8;
	req[count++] = location & 0x00ff;

	count += NOKIA_SetUnicodeString(s, req+count, bookmark->Title,   true);
	count += NOKIA_SetUnicodeString(s, req+count, bookmark->Address, true);

	req[count++] = 0x00;
	req[count++] = 0x00;
	req[count++] = 0x00;
	req[count++] = 0x00;

	smprintf(s, "Setting WAP bookmark\n");
	error = GSM_WaitFor (s, req, count, 0x3f, 4, ID_SetWAPBookmark);
	if (error != ERR_NONE) {
		if (error == ERR_INSIDEPHONEMENU || error == ERR_EMPTY || error == ERR_FULL) {
			DCT3DCT4_DisableConnectionFunctions(s);
		}
		return error;
	}

	return DCT3DCT4_DisableConnectionFunctions(s);
}

GSM_Error N6510_DeleteWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error error;

	/* We have to enable WAP frames in phone */
	error=N6510_EnableConnectionFunctions(s,N6510_WAP_SETTINGS);
	if (error!=ERR_NONE) return error;

	return DCT3DCT4_DeleteWAPBookmarkPart(s,bookmark);
}

GSM_Error N6510_GetWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error error;

	if (GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_SERIES40_30)) return ERR_NOTSUPPORTED;

	/* We have to enable WAP frames in phone */
	error=N6510_EnableConnectionFunctions(s,N6510_WAP_SETTINGS);
	if (error!=ERR_NONE) return error;

	return DCT3DCT4_GetWAPBookmarkPart(s,bookmark);
}

static GSM_Reply_Function N6510ReplyFunctions[] = {
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x02,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x03,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x04,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x05,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x07,ID_AnswerCall	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x07,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x09,ID_CancelCall	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x09,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x0A,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x0B,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x0C,ID_DialVoice		  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x0C,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x0F,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x10,ID_DialVoice		  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x10,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x23,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x25,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x27,ID_IncomingFrame	  },
	{N71_65_ReplySendDTMF,		  "\x01",0x03,0x51,ID_SendDTMF		  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x53,ID_IncomingFrame	  },
	{N71_65_ReplySendDTMF,		  "\x01",0x03,0x59,ID_SendDTMF		  },
	{N71_65_ReplySendDTMF,		  "\x01",0x03,0x5E,ID_SendDTMF		  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0xA6,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0xD2,ID_IncomingFrame	  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0xD3,ID_IncomingFrame	  },

	{N6510_ReplySendSMSMessage,	  "\x02",0x03,0x03,ID_IncomingFrame	  },
	{N6510_ReplyIncomingSMS,	  "\x02",0x03,0x04,ID_IncomingFrame	  },
	{N6510_ReplySetSMSC,		  "\x02",0x03,0x13,ID_SetSMSC		  },
	{N6510_ReplyGetSMSC,		  "\x02",0x03,0x15,ID_GetSMSC		  },
	{N6510_ReplyIncomingCB,		  "\x02",0x03,0x21,ID_IncomingFrame	  },
	{N6510_ReplyIncomingCB,		  "\x02",0x03,0x22,ID_IncomingFrame	  },
	{N6510_ReplyIncomingCB,		  "\x02",0x03,0x23,ID_IncomingFrame	  },

	{N6510_ReplyGetMemoryStatus,	  "\x03",0x03,0x04,ID_GetMemoryStatus	  },
	{N6510_ReplyGetMemory,		  "\x03",0x03,0x08,ID_GetMemory		  },
	{N71_65_ReplyWritePhonebook,	  "\x03",0x03,0x0C,ID_SetBitmap		  },
	{N71_65_ReplyWritePhonebook,	  "\x03",0x03,0x0C,ID_SetMemory		  },
	{N6510_ReplyDeleteMemory,	  "\x03",0x03,0x10,ID_SetMemory		  },

	{DCT3DCT4_ReplyCallDivert,	  "\x06",0x03,0x02,ID_Divert		  },
	{N71_65_ReplyUSSDInfo,		  "\x06",0x03,0x03,ID_IncomingFrame	  },
	{NoneReply,			  "\x06",0x03,0x06,ID_IncomingFrame	  },
	{NoneReply,			  "\x06",0x03,0x09,ID_IncomingFrame	  },

	{N6510_ReplyEnterSecurityCode,	  "\x08",0x03,0x08,ID_EnterSecurityCode	  },
	{N6510_ReplyEnterSecurityCode,	  "\x08",0x03,0x09,ID_EnterSecurityCode	  },
	{N6510_ReplyGetSecurityStatus,	  "\x08",0x03,0x12,ID_GetSecurityStatus	  },

	{N6510_ReplyGetNetworkInfo,	  "\x0A",0x03,0x01,ID_GetNetworkInfo	  },
	{N6510_ReplyGetNetworkInfo,	  "\x0A",0x03,0x01,ID_IncomingFrame	  },
	{N6510_ReplyLogIntoNetwork,	  "\x0A",0x03,0x02,ID_IncomingFrame	  },
	{N6510_ReplyGetSignalQuality,	  "\x0A",0x03,0x0C,ID_GetSignalQuality	  },
	{NoneReply,			  "\x0A",0x03,0x16,ID_IncomingFrame	  },
	{N6510_ReplyGetIncSignalQuality,  "\x0A",0x03,0x1E,ID_IncomingFrame	  },
	{NoneReply,			  "\x0A",0x03,0x20,ID_IncomingFrame	  },
	{N6510_ReplyGetOperatorLogo,	  "\x0A",0x03,0x24,ID_GetBitmap		  },
	{N6510_ReplyGetOperatorLogo,	  "\x0A",0x03,0xf0,ID_GetBitmap		  },
	{N6510_ReplySetOperatorLogo,	  "\x0A",0x03,0x26,ID_SetBitmap		  },

	{NoneReply,			  "\x0B",0x03,0x01,ID_PlayTone		  },
	{NoneReply,			  "\x0B",0x03,0x15,ID_PlayTone		  },
	{NoneReply,			  "\x0B",0x03,0x16,ID_PlayTone		  },

	{N71_65_ReplyAddCalendar1,	  "\x13",0x03,0x02,ID_SetCalendarNote	  },
	{N71_65_ReplyAddCalendar1,	  "\x13",0x03,0x04,ID_SetCalendarNote	  },
	{N71_65_ReplyAddCalendar1,	  "\x13",0x03,0x06,ID_SetCalendarNote	  },
	{N71_65_ReplyAddCalendar1,	  "\x13",0x03,0x08,ID_SetCalendarNote	  },
	{N71_65_ReplyDelCalendar,	  "\x13",0x03,0x0C,ID_DeleteCalendarNote  },
	{N71_65_ReplyDelCalendar,	  "\x13",0x03,0xF0,ID_DeleteCalendarNote  },
	{N71_65_ReplyGetNextCalendar1,	  "\x13",0x03,0x1A,ID_GetCalendarNote	  },/*method 1*/
	{N6510_ReplyGetCalendarNotePos,	  "\x13",0x03,0x32,ID_GetCalendarNotePos  },/*method 1*/
	{N6510_ReplyGetCalendarInfo,	  "\x13",0x03,0x3B,ID_GetCalendarNotesInfo},/*method 1*/
#ifdef DEBUG
	{N71_65_ReplyGetNextCalendar2,	  "\x13",0x03,0x3F,ID_GetCalendarNote	  },
#endif
	{N71_65_ReplyAddCalendar2,	  "\x13",0x03,0x41,ID_SetCalendarNote	  },/*method 2*/
	{N6510_ReplyAddCalendar3,	  "\x13",0x03,0x66,ID_SetCalendarNote	  },/*method 3*/
	{N6510_ReplyAddToDo2,		  "\x13",0x03,0x66,ID_SetToDo		  },
	{N6510_ReplyAddNote,		  "\x13",0x03,0x66,ID_SetNote		  },
	{N6510_ReplyGetCalendar3,	  "\x13",0x03,0x7E,ID_GetCalendarNote	  },/*method 3*/
	{N6510_ReplyGetToDo2,		  "\x13",0x03,0x7E,ID_GetToDo		  },
	{N6510_ReplyGetNote,		  "\x13",0x03,0x7E,ID_GetNote		  },
	{N6510_ReplyGetCalendarSettings,  "\x13",0x03,0x86,ID_GetCalendarSettings },
	{N6510_ReplyGetLocale,		  "\x13",0x03,0x8A,ID_GetLocale		  },
	{N6510_ReplyGetCalendarSettings,  "\x13",0x03,0x8E,ID_GetCalendarSettings },
	{N6510_ReplyGetCalendarNotePos,	  "\x13",0x03,0x96,ID_GetCalendarNotePos  },/*method 3*/
	{N6510_ReplyGetToDoFirstLoc2,	  "\x13",0x03,0x96,ID_SetToDo		  },
	{N6510_ReplyGetNoteFirstLoc,	  "\x13",0x03,0x96,ID_SetNote		  },
	{N6510_ReplyGetCalendarInfo,	  "\x13",0x03,0x9F,ID_GetCalendarNotesInfo},/*method 3*/
	{N6510_ReplyGetToDoStatus2,	  "\x13",0x03,0x9F,ID_GetToDo		  },
	{N6510_ReplyGetNoteInfo,	  "\x13",0x03,0x9F,ID_GetNote		  },

	{N6510_ReplySaveSMSMessage,	  "\x14",0x03,0x01,ID_SaveSMSMessage	  },
	{N6510_ReplySetPicture,		  "\x14",0x03,0x01,ID_SetBitmap		  },
	{N6510_ReplyGetSMSMessage,	  "\x14",0x03,0x03,ID_GetSMSMessage	  },
	{N6510_ReplyDeleteSMSMessage,	  "\x14",0x03,0x05,ID_DeleteSMSMessage	  },
	{N6510_ReplyDeleteSMSMessage,	  "\x14",0x03,0x06,ID_DeleteSMSMessage	  },
	{N6510_ReplyGetSMSStatus,	  "\x14",0x03,0x09,ID_GetSMSStatus	  },
	{N6510_ReplyGetSMSFolderStatus,	  "\x14",0x03,0x0d,ID_GetSMSFolderStatus  },
	{N6510_ReplyGetSMSMessage,	  "\x14",0x03,0x0f,ID_GetSMSMessage	  },
	{N6510_ReplyAddSMSFolder,	  "\x14",0x03,0x11,ID_AddSMSFolder	  },
	{N6510_ReplyGetSMSFolders,	  "\x14",0x03,0x13,ID_GetSMSFolders	  },
	{N6510_ReplySaveSMSMessage,	  "\x14",0x03,0x17,ID_SaveSMSMessage	  },
	{N6510_ReplyGetSMSStatus,	  "\x14",0x03,0x1a,ID_GetSMSStatus	  },

	{DCT4_ReplySetPhoneMode,	  "\x15",0x03,0x64,ID_Reset		  },
	{DCT4_ReplyGetPhoneMode,	  "\x15",0x03,0x65,ID_Reset		  },
	{NoneReply,		  	  "\x15",0x03,0x68,ID_Reset		  },

	{N6510_ReplyGetBatteryCharge,	  "\x17",0x03,0x0B,ID_GetBatteryCharge	  },
	{N6510_IncomingBatteryCharge,	  "\x17",0x03,0x2c,ID_IncomingFrame	  },

	{N6510_ReplySetDateTime,	  "\x19",0x03,0x02,ID_SetDateTime	  },
	{N6510_ReplyGetDateTime,	  "\x19",0x03,0x0B,ID_GetDateTime	  },
	{N6510_ReplySetAlarm,		  "\x19",0x03,0x12,ID_SetAlarm		  },
	{N6510_ReplyGetAlarm,		  "\x19",0x03,0x1A,ID_GetAlarm		  },
	{N6510_ReplyGetAlarm,		  "\x19",0x03,0x20,ID_GetAlarm		  },

	{DCT4_ReplyGetIMEI,		  "\x1B",0x03,0x01,ID_GetIMEI		  },
	{NOKIA_ReplyGetPhoneString,	  "\x1B",0x03,0x08,ID_GetHardware	  },
	{N6510_ReplyGetPPM,		  "\x1B",0x03,0x08,ID_GetPPM		  },
	{NOKIA_ReplyGetPhoneString,	  "\x1B",0x03,0x0C,ID_GetProductCode	  },

	/* 0x1C - vibra */

	{N6510_ReplyGetRingtonesInfo,	  "\x1f",0x03,0x08,ID_GetRingtonesInfo	  },
	{N6510_ReplyDeleteRingtones,	  "\x1f",0x03,0x11,ID_SetRingtone	  },
	{N6510_ReplyGetRingtone,	  "\x1f",0x03,0x13,ID_GetRingtone	  },
	{N6510_ReplySetBinRingtone,	  "\x1f",0x03,0x0F,ID_SetRingtone	  },

	/* 0x23 - voice records */

	{N6510_ReplyGetProfile,		  "\x39",0x03,0x02,ID_GetProfile	  },
	{N6510_ReplySetProfile,		  "\x39",0x03,0x04,ID_SetProfile	  },
	{N6510_ReplyGetProfile,		  "\x39",0x03,0x06,ID_GetProfile	  },

	{N6510_ReplySetLight,		  "\x3A",0x03,0x06,ID_SetLight		  },

 	{N6510_ReplyGetFMStation,	  "\x3E",0x03,0x06,ID_GetFMStation	  },
 	{N6510_ReplyGetFMStatus,	  "\x3E",0x03,0x0E,ID_GetFMStation	  },
 	{N6510_ReplySetFMStation,	  "\x3E",0x03,0x15,ID_SetFMStation	  },
 	{N6510_ReplyGetFMStation,	  "\x3E",0x03,0x16,ID_GetFMStation	  },

	{DCT3DCT4_ReplyEnableConnectFunc, "\x3f",0x03,0x01,ID_EnableConnectFunc	  },
	{DCT3DCT4_ReplyEnableConnectFunc, "\x3f",0x03,0x02,ID_EnableConnectFunc	  },
	{DCT3DCT4_ReplyDisableConnectFunc,"\x3f",0x03,0x04,ID_DisableConnectFunc  },
	{DCT3DCT4_ReplyDisableConnectFunc,"\x3f",0x03,0x05,ID_DisableConnectFunc  },
	{N6510_ReplyGetWAPBookmark,	  "\x3f",0x03,0x07,ID_GetWAPBookmark	  },
	{N6510_ReplyGetWAPBookmark,	  "\x3f",0x03,0x08,ID_GetWAPBookmark	  },
	{DCT3DCT4_ReplySetWAPBookmark,	  "\x3f",0x03,0x0A,ID_SetWAPBookmark	  },
	{DCT3DCT4_ReplySetWAPBookmark,	  "\x3f",0x03,0x0B,ID_SetWAPBookmark	  },
	{DCT3DCT4_ReplyDelWAPBookmark,	  "\x3f",0x03,0x0D,ID_DeleteWAPBookmark	  },
	{DCT3DCT4_ReplyDelWAPBookmark,	  "\x3f",0x03,0x0E,ID_DeleteWAPBookmark	  },
	{DCT3DCT4_ReplyGetActiveConnectSet,"\x3f",0x03,0x10,ID_GetConnectSet	  },
	{DCT3DCT4_ReplySetActiveConnectSet,"\x3f",0x03,0x13,ID_SetConnectSet	  },
	{N6510_ReplyGetConnectionSettings,"\x3f",0x03,0x11,ID_GetConnectSet	  },
	{N6510_ReplyGetConnectionSettings,"\x3f",0x03,0x16,ID_GetConnectSet	  },
	{N6510_ReplyGetConnectionSettings,"\x3f",0x03,0x17,ID_GetConnectSet	  },
	{N6510_ReplySetConnectionSettings,"\x3f",0x03,0x19,ID_SetConnectSet	  },
	{N6510_ReplySetConnectionSettings,"\x3f",0x03,0x1A,ID_SetConnectSet	  },
	{N6510_ReplySetConnectionSettings,"\x3f",0x03,0x28,ID_SetConnectSet	  },
	{N6510_ReplySetConnectionSettings,"\x3f",0x03,0x2B,ID_SetConnectSet	  },
	{N6510_ReplyGetChatSettings,	  "\x3f",0x03,0x3B,ID_GetChatSettings	  },
	{N6510_ReplyGetChatSettings,	  "\x3f",0x03,0x3C,ID_GetChatSettings	  },
	{N6510_ReplyGetConnectionSettings,"\x3f",0x03,0xf0,ID_GetConnectSet	  },

	{N6510_ReplyGetOriginalIMEI,	  "\x42",0x07,0x00,ID_GetOriginalIMEI	  },
	{N6510_ReplyGetManufactureMonth,  "\x42",0x07,0x00,ID_GetManufactureMonth },
	{N6510_ReplyGetOriginalIMEI,	  "\x42",0x07,0x01,ID_GetOriginalIMEI	  },
	{N6510_ReplyGetManufactureMonth,  "\x42",0x07,0x02,ID_GetManufactureMonth },

	{N6510_ReplySetOperatorLogo,	  "\x43",0x03,0x08,ID_SetBitmap		  },
	{N6510_ReplyGetGPRSAccessPoint,	  "\x43",0x03,0x06,ID_GetGPRSPoint	  },
	{N6510_ReplySetGPRSAccessPoint1,  "\x43",0x03,0x06,ID_SetGPRSPoint	  },
#ifdef DEVELOP
	{N6510_ReplyEnableGPRSAccessPoint,"\x43",0x03,0x06,ID_EnableGPRSPoint	  },
#endif
	{N6510_ReplyGetSyncMLSettings,	  "\x43",0x03,0x06,ID_GetSyncMLSettings	  },
	{N6510_ReplyGetSyncMLName,	  "\x43",0x03,0x06,ID_GetSyncMLName	  },
	{NoneReply,			  "\x43",0x03,0x08,ID_SetGPRSPoint	  },

	/* 0x4A - voice records */

	/* 0x53 - simlock */

	{N6510_ReplyAddToDo1,		  "\x55",0x03,0x02,ID_SetToDo		  },
	{N6510_ReplyGetToDo1,		  "\x55",0x03,0x04,ID_GetToDo		  },
	{N6510_ReplyGetToDoFirstLoc1,	  "\x55",0x03,0x10,ID_SetToDo		  },
	{N6510_ReplyDeleteAllToDo1,	  "\x55",0x03,0x12,ID_DeleteAllToDo	  },
	{N6510_ReplyGetToDoStatus1,	  "\x55",0x03,0x16,ID_GetToDo		  },

	{N6510_ReplyAddFileHeader1,	  "\x6D",0x03,0x03,ID_AddFile		  },
	{N6510_ReplyAddFolder1,		  "\x6D",0x03,0x05,ID_AddFolder		  },
	{N6510_ReplyGetFilePart12,	  "\x6D",0x03,0x0F,ID_GetFile		  },/* filesystem1&2 */
	{N6510_ReplyAddFileHeader1,	  "\x6D",0x03,0x13,ID_AddFile		  },
	{N6510_ReplyGetFileFolderInfo1,	  "\x6D",0x03,0x15,ID_GetFileInfo	  },
	{NoneReply,			  "\x6D",0x03,0x19,ID_SetAttrib		  },
	{N6510_ReplyDeleteFileFolder1,	  "\x6D",0x03,0x1F,ID_DeleteFile	  },
	{N6510_ReplyGetFileSystemStatus1, "\x6D",0x03,0x23,ID_FileSystemStatus	  },
	{N6510_ReplyGetFileFolderInfo1,	  "\x6D",0x03,0x2F,ID_GetFileInfo	  },
	{N6510_ReplyGetFileSystemStatus1, "\x6D",0x03,0x2F,ID_FileSystemStatus	  },
	{N6510_ReplyGetFileFolderInfo1,	  "\x6D",0x03,0x33,ID_GetFileInfo	  },
	{N6510_ReplyAddFilePart1,	  "\x6D",0x03,0x41,ID_AddFile		  },
	{N6510_ReplyGetFileCRC12,	  "\x6D",0x03,0x43,ID_GetCRC		  },
	{NoneReply,			  "\x6D",0x03,0x59,ID_AddFile		  },/* filesystem2 */
	{N6510_ReplyGetFilePart12,	  "\x6D",0x03,0x5F,ID_GetFile		  },/* filesystem2 */
	{N6510_ReplyDeleteFile2,	  "\x6D",0x03,0x63,ID_DeleteFile	  },/* filesystem2 */
	{N6510_ReplyAddFolder2,		  "\x6D",0x03,0x65,ID_AddFolder		  },/* filesystem2 */
	{N6510_ReplyGetFileCRC12,   	  "\x6D",0x03,0x67,ID_GetCRC		  },/* filesystem2 */
	{N6510_ReplyGetFileFolderInfo2,   "\x6D",0x03,0x69,ID_IncomingFrame	  },/* filesystem2 */
	{N6510_ReplyDeleteFolder2,	  "\x6D",0x03,0x6B,ID_DeleteFolder	  },/* filesystem2 */
	{N6510_ReplyGetFileFolderInfo2,   "\x6D",0x03,0x6D,ID_GetFileInfo	  },/* filesystem2 */
	{N6510_ReplySetAttrib2,		  "\x6D",0x03,0x6F,ID_SetAttrib		  },/* filesystem2 */
	{N6510_ReplyOpenFile2,		  "\x6D",0x03,0x73,ID_OpenFile		  },/* filesystem2 */
	{NoneReply,			  "\x6D",0x03,0x75,ID_CloseFile		  },/* filesystem2 */
	{N6510_ReplySetFileDate2,	  "\x6D",0x03,0x87,ID_AddFile		  },/* filesystem2 */

	{N6510_ReplyStartupNoteLogo,	  "\x7A",0x04,0x01,ID_GetBitmap		  },
	{N6510_ReplyStartupNoteLogo,	  "\x7A",0x04,0x01,ID_SetBitmap		  },
	{N6510_ReplyStartupNoteLogo,	  "\x7A",0x04,0x0F,ID_GetBitmap		  },
	{N6510_ReplyStartupNoteLogo,	  "\x7A",0x04,0x0F,ID_SetBitmap		  },
	{N6510_ReplyStartupNoteLogo,	  "\x7A",0x04,0x10,ID_GetBitmap		  },
	{N6510_ReplyStartupNoteLogo,	  "\x7A",0x04,0x10,ID_SetBitmap		  },
	{N6510_ReplyStartupNoteLogo,	  "\x7A",0x04,0x25,ID_SetBitmap		  },

	{DCT3DCT4_ReplyGetModelFirmware,  "\xD2",0x02,0x00,ID_GetModel		  },
	{DCT3DCT4_ReplyGetModelFirmware,  "\xD2",0x02,0x00,ID_GetFirmware	  },

	/* 0xD7 - Bluetooth */

	{N6510_ReplyGetRingtoneID,	  "\xDB",0x03,0x02,ID_SetRingtone	  },

	{NULL,				  "\x00",0x00,0x00,ID_None		  }
};

GSM_Phone_Functions N6510Phone = {
	"1100|1100a|1100b|1200|2600|2610|2630|2650|2760|3100|3100b|3105|3108|3109c|3200|3200a|3205|3220|3300|3510|3510i|3530|3589i|3590|3595|5000|5100|5140|5140i|5200|5300|5310|6020|6021|6030|6060|6070|6085|6086|6100|6101|6103|6111|6125|6131|6151|6170|6200|6220|6230|6230i|6233|6234|6270|6280|6300|6310|6310i|6385|6500c|6500s|6510|6610|6610i|6800|6810|6820|6822|7200|7210|7250|7250i|7260|7270|7360|7370|7600|7900|8310|8390|8800|8910|8910i",
	N6510ReplyFunctions,
	N6510_Initialise,
	NONEFUNCTION,			/*	Terminate 		*/
	GSM_DispatchMessage,
	N6510_ShowStartInfo,
	NOKIA_GetManufacturer,
	DCT3DCT4_GetModel,
	DCT3DCT4_GetFirmware,
	DCT4_GetIMEI,
	N6510_GetOriginalIMEI,
	N6510_GetManufactureMonth,
	DCT4_GetProductCode,
	DCT4_GetHardware,
	N6510_GetPPM,
	NOTSUPPORTED,			/*	GetSIMIMSI		*/
	N6510_GetDateTime,
	N6510_SetDateTime,
	N6510_GetAlarm,
	N6510_SetAlarm,
	N6510_GetLocale,
	NOTSUPPORTED,			/* 	SetLocale		*/
	N6510_PressKey,
	DCT4_Reset,
	NOTIMPLEMENTED,			/*	ResetPhoneSettings	*/
	N6510_EnterSecurityCode,
	N6510_GetSecurityStatus,
	NOTSUPPORTED,			/*	GetDisplayStatus	*/
	NOTIMPLEMENTED,			/*	SetAutoNetworkLogin	*/
	N6510_GetBatteryCharge,
	N6510_GetSignalQuality,
	N6510_GetNetworkInfo,
	NOTSUPPORTED,       		/*  	GetCategory 		*/
 	NOTSUPPORTED,       		/*  	AddCategory 		*/
	NOTSUPPORTED,        		/*  	GetCategoryStatus 	*/
	N6510_GetMemoryStatus,
	N6510_GetMemory,
	NOTIMPLEMENTED,			/*	GetNextMemory		*/
	N6510_SetMemory,
	N6510_AddMemory,
	N6510_DeleteMemory,
	NOTIMPLEMENTED,			/*	DeleteAllMemory		*/
	N6510_GetSpeedDial,
	NOTIMPLEMENTED,			/*	SetSpeedDial		*/
	N6510_GetSMSC,
	N6510_SetSMSC,
	N6510_GetSMSStatus,
	N6510_GetSMSMessage,
	N6510_GetNextSMSMessage,
	N6510_SetSMS,
	N6510_AddSMS,
	N6510_DeleteSMSMessage,
	N6510_SendSMSMessage,
	NOTSUPPORTED,			/*	SendSavedSMS		*/
	NOTSUPPORTED,			/*	SetFastSMSSending	*/
	NOKIA_SetIncomingSMS,
	NOTIMPLEMENTED,			/* 	SetIncomingCB		*/
	N6510_GetSMSFolders,
 	N6510_AddSMSFolder,
 	NOTIMPLEMENTED,			/* 	DeleteSMSFolder		*/
	N6510_DialVoice,
        NOTIMPLEMENTED,			/*	DialService		*/
 	N6510_AnswerCall,
 	N6510_CancelCall,
 	NOTIMPLEMENTED,			/* 	HoldCall 		*/
 	NOTIMPLEMENTED,			/* 	UnholdCall 		*/
 	NOTIMPLEMENTED,			/* 	ConferenceCall 		*/
 	NOTIMPLEMENTED,			/* 	SplitCall		*/
 	NOTIMPLEMENTED,			/* 	TransferCall		*/
 	NOTIMPLEMENTED,			/* 	SwitchCall		*/
 	DCT3DCT4_GetCallDivert,
 	DCT3DCT4_SetCallDivert,
 	DCT3DCT4_CancelAllDiverts,
	NOKIA_SetIncomingCall,
	NOKIA_SetIncomingUSSD,
	DCT3DCT4_SendDTMF,
	N6510_GetRingtone,
	N6510_SetRingtone,
	N6510_GetRingtonesInfo,
	N6510_DeleteUserRingtones,
	N6510_PlayTone,
	N6510_GetWAPBookmark,
	N6510_SetWAPBookmark,
	N6510_DeleteWAPBookmark,
	N6510_GetWAPSettings,
	N6510_SetWAPSettings,
	N6510_GetSyncMLSettings,
	NOTSUPPORTED,			/*	SetSyncMLSettings	*/
	N6510_GetChatSettings,
	NOTSUPPORTED,			/*	SetChatSettings		*/
	N6510_GetMMSSettings,
	N6510_SetMMSSettings,
	N6510_GetMMSFolders,
	N6510_GetNextMMSFileInfo,
	N6510_GetBitmap,
	N6510_SetBitmap,
	N6510_GetToDoStatus,
	NOTIMPLEMENTED,			/*	GetToDo			*/
	N6510_GetNextToDo,
	NOTIMPLEMENTED,			/*	SetToDo			*/
	N6510_AddToDo,
	N6510_DeleteToDo2,
	N6510_DeleteAllToDo1,
	N6510_GetCalendarStatus,
	NOTIMPLEMENTED,			/*	GetCalendar		*/
    	N6510_GetNextCalendar,
	NOTIMPLEMENTED,			/*	SetCalendar		*/
	N6510_AddCalendar,
	N71_65_DelCalendar,
	NOTIMPLEMENTED,			/*	DeleteAllCalendar	*/
	N6510_GetCalendarSettings,
	NOTSUPPORTED,			/* 	SetCalendarSettings	*/
	N6510_GetNoteStatus,
	NOTIMPLEMENTED,			/*	GetNote			*/
	N6510_GetNextNote,
	NOTIMPLEMENTED,			/*	SetNote			*/
	N6510_AddNote,
	N6510_DeleteNote,
	NOTSUPPORTED,			/*	DeleteAllNotes		*/
	N6510_GetProfile,
	N6510_SetProfile,
    	N6510_GetFMStation,
     	N6510_SetFMStation,
 	N6510_ClearFMStations,
	N6510_GetNextFileFolder,
	N6510_GetFolderListing,
	N6510_GetNextRootFolder,
	N6510_SetFileAttributes,
	N6510_GetFilePart,
	N6510_AddFilePart,
	NOTSUPPORTED,			/* 	SendFilePart		*/
	N6510_GetFileSystemStatus,
	N6510_DeleteFile,
	N6510_AddFolder,
	N6510_DeleteFolder,
	N6510_GetGPRSAccessPoint,
	N6510_SetGPRSAccessPoint
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
