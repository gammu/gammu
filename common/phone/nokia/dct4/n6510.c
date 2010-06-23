
#include "../../../gsmstate.h"

#ifdef GSM_ENABLE_NOKIA6510

#include <string.h>
#include <time.h>

#include "../../../gsmcomon.h"
#include "../../../misc/coding.h"
#include "../../../service/gsmlogo.h"
#include "../nfunc.h"
#include "../../pfunc.h"
#include "n6510.h"

static GSM_Error N6510_Initialise (GSM_StateMachine *s)
{
	/* Enables various things like incoming SMS, call info, etc. */
	return N71_65_EnableFunctions (s, "\x01\x02\x06\x0A\x14\x17\x39", 7);
}

static GSM_Error N6510_ReplyGetIMEI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	memcpy(s->Phone.Data.IMEI,msg.Buffer + 10, 16);
	smprintf(s, "Received IMEI %s\n",s->Phone.Data.IMEI);
	return GE_NONE;
}

static GSM_Error N6510_GetIMEI (GSM_StateMachine *s)
{
	unsigned char req[5] = {N6110_FRAME_HEADER, 0x00, 0x41};

	smprintf(s, "Getting IMEI\n");
	return GSM_WaitFor (s, req, 5, 0x1B, 2, ID_GetIMEI);
}

static GSM_Error N6510_ReplyGetMemory(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Phonebook entry received\n");
	switch (msg.Buffer[6]) {
	case 0x0f:
		return N71_65_ReplyGetMemoryError(msg.Buffer[10], s);
	default:
		return N71_65_DecodePhonebook(s, s->Phone.Data.Memory, s->Phone.Data.Bitmap, s->Phone.Data.SpeedDial, msg.Buffer+22, msg.Length-22);
	}
	return GE_UNKNOWN;
}

static GSM_Error N6510_GetMemory (GSM_StateMachine *s, GSM_PhonebookEntry *entry)
{
	unsigned char req[] = {
		N7110_FRAME_HEADER, 0x07, 0x04, 0x01, 0x00, 0x01,
		0x02, 		/* memory type */
		0x05,
		0x00, 0x00, 0x00, 0x00, 
		0x00, 0x01,	/*location */
		0x00, 0x00};

	req[9] = NOKIA_GetMemoryType(s, entry->MemoryType,N71_65_MEMORY_TYPES);
	if (req[9]==0xff) return GE_NOTSUPPORTED;

	if (entry->Location==0x00) return GE_INVALIDLOCATION;

	req[14] = (entry->Location>>8);
	req[15] = entry->Location & 0xff;

	s->Phone.Data.Memory=entry;
	smprintf(s, "Getting phonebook entry\n");
	return GSM_WaitFor (s, req, 20, 0x03, 4, ID_GetMemory);
}

static GSM_Error N6510_ReplyStartupNoteLogo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data		*Data = &s->Phone.Data;
	switch (msg.Buffer[4]) {
	case 0x01:
		smprintf(s, "Welcome note text received\n");
		CopyUnicodeString(Data->Bitmap->Text,msg.Buffer+6);
		smprintf(s, "Text is \"%s\"\n",DecodeUnicodeString(Data->Bitmap->Text));
		return GE_NONE;
	case 0x0f:
		if (Data->RequestID == ID_GetBitmap) {
			smprintf(s, "Startup logo received\n");
			PHONE_DecodeBitmap(GSM_Nokia7110StartupLogo, msg.Buffer + 22, Data->Bitmap);
		}
		if (Data->RequestID == ID_SetBitmap) {
			smprintf(s, "Startup logo set\n");
		}
		return GE_NONE;
	}
	return GE_UNKNOWN;
}

static GSM_Error N6510_GetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	unsigned char reqOp	[] = {N6110_FRAME_HEADER, 0x23, 0x00, 0x00, 0x55, 0x55, 0x55};
	unsigned char reqStartup[] = {N6110_FRAME_HEADER, 0x02, 0x0f};
	unsigned char reqNote	[] = {N6110_FRAME_HEADER, 0x02, 0x01, 0x00};
	GSM_PhonebookEntry	pbk;
	GSM_Error		error;

	s->Phone.Data.Bitmap=Bitmap;	
	switch (Bitmap->Type) {
	case GSM_StartupLogo:
		smprintf(s, "Getting startup logo\n");
		return GSM_WaitFor (s, reqStartup, 5, 0x7A, 4, ID_GetBitmap);
	case GSM_WelcomeNoteText:
		smprintf(s, "Getting welcome note\n");
		return GSM_WaitFor (s, reqNote, 6, 0x7A, 4, ID_GetBitmap);
	case GSM_CallerLogo:
		/* You can only get logos which have been altered, the standard */
		/* logos can't be read!! */
		pbk.MemoryType	= GMT7110_CG;
		pbk.Location	= Bitmap->Location;
		smprintf(s, "Getting caller group logo\n");
		error=N6510_GetMemory(s,&pbk);
		if (error==GE_NONE) NOKIA_GetDefaultCallerGroupName(s, Bitmap);
		return error;
	case GSM_OperatorLogo:
		smprintf(s, "Getting operator logo\n");
		return GSM_WaitFor (s, reqOp, 9, 0x0A, 4, ID_GetBitmap);
	case GSM_PictureImage:
		break;
	default:
		break;
	}
	return GE_NOTSUPPORTED;
}

static GSM_Error N6510_ReplyGetMemoryStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data		*Data = &s->Phone.Data;
	smprintf(s, "Memory status received\n");
	/* Quess ;-)) */
	if (msg.Buffer[14]==0x10) {
		Data->MemoryStatus->Free = msg.Buffer[18]*256 + msg.Buffer[19];
	} else {
		Data->MemoryStatus->Free = msg.Buffer[17];
	}
	smprintf(s, "   Size       : %i\n",Data->MemoryStatus->Free);
	Data->MemoryStatus->Used = msg.Buffer[20]*256 + msg.Buffer[21];
	smprintf(s, "   Used       : %i\n",Data->MemoryStatus->Used);
	Data->MemoryStatus->Free -= Data->MemoryStatus->Used;
	smprintf(s, "   Free       : %i\n",Data->MemoryStatus->Free);
	return GE_NONE;
}

static GSM_Error N6510_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x03, 0x02,
		0x00,		/* MemoryType */
		0x55, 0x55, 0x55, 0x00};

	req[5] = NOKIA_GetMemoryType(s, Status->MemoryType,N71_65_MEMORY_TYPES);
	if (req[5]==0xff) return GE_NOTSUPPORTED;

	s->Phone.Data.MemoryStatus=Status;
	smprintf(s, "Getting memory status\n");
	return GSM_WaitFor (s, req, 10, 0x03, 4, ID_GetMemoryStatus);
}

static GSM_Error N6510_ReplyGetSMSC(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int i, current, j;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	switch (msg.Buffer[4]) {
		case 0x00:
			smprintf(s, "SMSC received\n");
			break;
		case 0x02:
			smprintf(s, "SMSC empty\n");
			return GE_INVALIDLOCATION;
		default:
			smprintf(s, "Unknown SMSC state: %02x\n",msg.Buffer[4]);
			return GE_UNKNOWNRESPONSE;
	}
	memset(Data->SMSC,0,sizeof(GSM_SMSC));
	Data->SMSC->Location 	= msg.Buffer[8];
	Data->SMSC->Format 	= GSMF_Text;
	switch (msg.Buffer[10]) {
		case 0x00: Data->SMSC->Format = GSMF_Text; 	break;
		case 0x22: Data->SMSC->Format = GSMF_Fax; 	break;
		case 0x26: Data->SMSC->Format = GSMF_Pager;	break;
		case 0x32: Data->SMSC->Format = GSMF_Email;	break;
	}
	Data->SMSC->Validity.VPF	= GSM_RelativeFormat;
	Data->SMSC->Validity.Relative	= msg.Buffer[12];
	current = 14;
	for (i=0;i<msg.Buffer[13];i++) {
		switch (msg.Buffer[current]) {
		case 0x81:
			j=current+4;
			while (msg.Buffer[j]!=0) {j++;}
			j=j-33;
			if (j>GSM_MAX_SMSC_NAME_LENGTH) {
				smprintf(s, "Too long name\n");
				return GE_UNKNOWNRESPONSE;
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
				return GE_UNKNOWNRESPONSE;
			}
			break;
		default:
			smprintf(s, "Unknown SMSC block: %02x\n",msg.Buffer[current]);
			return GE_UNKNOWNRESPONSE;
		}
		current = current + msg.Buffer[current+1];
	}
	return GE_NONE;
}

static GSM_Error N6510_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x14,
		0x01,		/* SMS Center Number. */
		0x00};

	if (smsc->Location==0x00) return GE_INVALIDLOCATION;
	
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
			return GE_NONE;
		case 0x02:
			smprintf(s, "Invalid SMSC location\n");
			return GE_INVALIDLOCATION;
		default:
			smprintf(s, "Unknown SMSC state: %02x\n",msg.Buffer[4]);
			return GE_UNKNOWNRESPONSE;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6510_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	int count = 13,i;
	unsigned char req[64] = {
		N6110_FRAME_HEADER,
		0x12, 0x55, 0x01, 0x0B, 0x34,
		0x05,		/* Location 	*/
		0x00,
		0x00,		/* Format 	*/
		0x00,
		0xFF};		/* Validity	*/

	req[8]  = smsc->Location;
	switch (smsc->Format) {
		case GSMF_Text:		req[10] = 0x00;	break;
		case GSMF_Fax:		req[10] = 0x22;	break;
		case GSMF_Pager:	req[10] = 0x26;	break;
		case GSMF_Email:	req[10] = 0x32;	break;
	}
	req[12]  = smsc->Validity.Relative;

	/* Magic. Nokia new ideas: coding data in the sequent blocks */
	req[count++] 	 	 = 0x03; 		/* Number of blocks */

	/* Block 2. SMSC Number */
	req[count++] 		 = 0x82; 		/* type: number */
	req[count++] 		 = 0x10;		/* offset to next block starting from start of block */
	req[count++] 		 = 0x02; 		/* first number field => SMSC number */
	req[count] = GSM_PackSemiOctetNumber(smsc->Number, req+count+2, false) + 1;
	if (req[count]>8) {
		smprintf(s, "Too long SMSC number in frame\n");
		return GE_UNKNOWN;
	}
	req[count+1] = req[count] - 1;
	count += 13;

	/* Block 1. Default Number */
	req[count++] 		 = 0x82; 		/* type: number */
	req[count++] 		 = 0x14;		/* offset to next block starting from start of block */
	req[count++] 		 = 0x01; 		/* first number field => default number */
	req[count] = GSM_PackSemiOctetNumber(smsc->DefaultNumber, req+count+2, true) + 1;
	if (req[count]*2>12) {
		smprintf(s, "Too long SMSC number in frame\n");
		return GE_UNKNOWN;
	}
	req[count+1] = req[count] - 1;
	count += 17;

	/* Block 3. SMSC name */
	req[count++] 		 = 0x81;
	req[count++] 		 = strlen(DecodeUnicodeString(smsc->Name))*2 + 2 + 4;
	req[count++] 		 = strlen(DecodeUnicodeString(smsc->Name))*2 + 2;
	req[count++] 		 = 0x00;
	/* Can't make CopyUnicodeString(req+count,sms->Name) !!!!
	 * with MSVC6 count is changed then
	 */
	i = count;
	CopyUnicodeString(req+i,smsc->Name);
	count += strlen(DecodeUnicodeString(smsc->Name))*2 + 2;
	
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

	switch (msg.Buffer[8]) {
		case 0x00 : smprintf(s, "   Logged into home network.\n");		break;
		case 0x01 : smprintf(s, "   Logged into a roaming network.\n");		break;
		case 0x04 :
		case 0x09 : smprintf(s, "   Not logged in any network!");		break;
		default	  : smprintf(s, "   Unknown network status!\n");		break;
	}
	if (msg.Buffer[8]==0x00 || msg.Buffer[8] == 0x01) {
		tmp = 10;
		NOKIA_GetUnicodeString(s, &tmp, msg.Buffer,name,true);
		smprintf(s, "   Network name for phone    : %s\n",DecodeUnicodeString(name));
		NOKIA_DecodeNetworkCode(msg.Buffer + (current + 7),NetInfo.NetworkCode);
		sprintf(NetInfo.CellID, "%02x%02x", msg.Buffer[current+5], msg.Buffer[current+6]);
		sprintf(NetInfo.LAC,	"%02x%02x", msg.Buffer[current+1], msg.Buffer[current+2]);
		smprintf(s, "   CellID                    : %s\n", NetInfo.CellID);
		smprintf(s, "   LAC                       : %s\n", NetInfo.LAC);
		smprintf(s, "   Network code              : %s\n", NetInfo.NetworkCode);
		smprintf(s, "   Network name for Gammu    : %s ",
			DecodeUnicodeString(GSM_GetNetworkName(NetInfo.NetworkCode)));
		smprintf(s, "(%s)\n",DecodeUnicodeString(GSM_GetCountryName(NetInfo.NetworkCode)));
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
			case 0x09: Data->NetworkInfo->State = GSM_NoNetwork;		break;
		}
		if (Data->NetworkInfo->State == GSM_HomeNetwork || Data->NetworkInfo->State == GSM_RoamingNetwork) {
			tmp = 10;
			NOKIA_GetUnicodeString(s, &tmp, msg.Buffer,Data->NetworkInfo->NetworkName,true);
			sprintf(Data->NetworkInfo->CellID, "%02x%02x", 	msg.Buffer[current+5], msg.Buffer[current+6]);
			sprintf(Data->NetworkInfo->LAC,	"%02x%02x", 	msg.Buffer[current+1], msg.Buffer[current+2]);
			NOKIA_DecodeNetworkCode(msg.Buffer + (current+7),Data->NetworkInfo->NetworkCode);
		}
	}
	return GE_NONE;
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
	int			start, count = 0, pos1, pos2, pos3, pos4, pos5;
	GSM_Error		error;

	memset(Layout,255,sizeof(GSM_SMSMessageLayout));

	start			 = *length;

	req[count++]		 = 0x01;		/* one big block ? */
	if (sms->PDU != SMS_Deliver) {
		req[count++] 	 = 0x02;
	} else {
		req[count++] 	 = 0x00;
	}

	pos1		  	 = count; count++;
	Layout->firstbyte 	 = count; count++;	/* firstbyte set in SMS Layout */
	if (sms->PDU != SMS_Deliver) {
		Layout->TPMR 	 = count; count++;	/* ??? */
		Layout->TPPID	 = count; count++;
		Layout->TPDCS 	 = count; count++;	/* TP.DCS set in SMS layout */
                req[count++] 	 = 0x00;
	} else {
		Layout->TPPID 	 = count; count++;
		Layout->TPDCS 	 = count; count++;	/* TP.DCS set in SMS layout */
		Layout->DateTime = count; count += 7;
		req[count++] 	 = 0x55;
		req[count++] 	 = 0x55;
		req[count++] 	 = 0x55;
	}

	/* Magic. Nokia new ideas: coding SMS in the sequent blocks */
	if (sms->PDU != SMS_Deliver) {
		req[count++] 	 = 0x04; 		/* Number of blocks */
	} else {
		req[count++] 	 = 0x03; 		/* Number of blocks */
	}

	/* Block 1. Remote Number */
	req[count++] 		 = 0x82; 		/* type: number */
	req[count++] 		 = 0x10;		/* offset to next block starting from start of block (req[18]) */
	req[count++] 		 = 0x01; 		/* first number field => phone number */
	pos4 			 = count; count++;
	Layout->Number 		 = count; count+= 12; 	/* now coded Number in SMS Layout */

	/* Block 2. SMSC Number */
	req[count++] 		 = 0x82; 		/* type: number */
	req[count++] 		 = 0x10;		/* offset to next block starting from start of block (req[18]) */
	req[count++] 		 = 0x02; 		/* first number field => SMSC number */
	pos5 			 = count; count++;
	Layout->SMSCNumber 	 = count; count += 12; 	/* now coded SMSC number in SMS Layout */

	/* Block 3. Validity Period */
	if (sms->PDU != SMS_Deliver) {
		req[count++] 	 = 0x08; 		/* type: validity */
		req[count++] 	 = 0x04;
		req[count++] 	 = 0x01; 		/* data length */
		Layout->TPVP 	 = count; count++;
	}

	/* Block 4. User Data */
	req[count++] 		 = 0x80; 		/* type: User Data */
	pos2			 = count; count++; 				/* same as req[11] but starting from req[42] */
	pos3			 = count; count++;
	Layout->TPUDL 		 = count; count++; 	/* FIXME*/
	Layout->Text 		 = count;		/* SMS text and UDH coded in SMS Layout */

	error = PHONE_EncodeSMSFrame(s,sms,req,*Layout,length,false);
	if (error != GE_NONE) return error;

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
		return GE_UNKNOWN;
	}

	return GE_NONE;
}

static GSM_Error N6510_ReplyGetSMSFolders(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int j, num = 0, pos;
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
				if (pos+4 > msg.Length) return GE_UNKNOWNRESPONSE;
				pos++;
			}
			pos+=4;
			smprintf(s, "Folder index: %02x",msg.Buffer[pos - 2]);
			if (msg.Buffer[pos - 1]>GSM_MAX_SMS_FOLDER_NAME_LEN) {
				smprintf(s, "Too long text\n");
				return GE_UNKNOWNRESPONSE;
			}
			CopyUnicodeString(Data->SMSFolders->Folder[num].Name,msg.Buffer + pos);
			smprintf(s, ", folder name: \"%s\"\n",DecodeUnicodeString(Data->SMSFolders->Folder[num].Name));
			if (num == 0x00 || num == 0x02) {
				num++;
				CopyUnicodeString(Data->SMSFolders->Folder[num].Name,msg.Buffer + pos);
			}
			num++;
		}
		return GE_NONE;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x12, 0x00, 0x00};

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
	return GE_NONE;
}

static GSM_Error N6510_GetSMSFolderStatus(GSM_StateMachine *s, int folderid)
{
	unsigned char req[] = {
		N7110_FRAME_HEADER, 0x0C, 
		0x01,		/* 0x01 SIM, 0x02 ME 	*/
		0x00,		/* Folder ID		*/
		0x0f, 0x55, 0x55, 0x55};

	switch (folderid) {
		case 0x01: req[5] = 0x02; 			 break; /* INBOX SIM 	*/
		case 0x02: req[5] = 0x03; 			 break; /* OUTBOX SIM 	*/
		default	 : req[5] = folderid - 1; req[4] = 0x02; break; /* ME folders	*/
	}

	smprintf(s, "Getting SMS folder status\n");
	return GSM_WaitFor (s, req, 10, 0x14, 4, ID_GetSMSFolderStatus);
}

static void N6510_GetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char *folderid, int *location)
{
	int ifolderid;

	/* simulate flat SMS memory */
	if (sms->Folder==0x00) {
		ifolderid = sms->Location / PHONE_MAXSMSINFOLDER;
		*folderid = ifolderid + 0x01;
		*location = sms->Location - ifolderid * PHONE_MAXSMSINFOLDER;
	} else {
		*folderid = sms->Folder;
		*location = sms->Location;
	}
	smprintf(s, "SMS folder %i & location %i -> 6510 folder %i & location %i\n",
		sms->Folder,sms->Location,*folderid,*location);
}

static void N6510_SetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char folderid, int location)
{
	sms->Folder	= 0;
	sms->Location	= (folderid - 0x01) * PHONE_MAXSMSINFOLDER + location;
	smprintf(s, "6510 folder %i & location %i -> SMS folder %i & location %i\n",
		folderid,location,sms->Folder,sms->Location);
}

static GSM_Error N6510_DecodeSMSFrame(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char *buffer)
{
	int 			i, current, blocks=0;
	GSM_SMSMessageLayout 	Layout;

	memset(&Layout,255,sizeof(GSM_SMSMessageLayout));
	Layout.firstbyte = 2;
	switch (buffer[0]) {
	case 0x00:
		smprintf(s, "SMS deliver\n");
		sms->PDU = SMS_Deliver;
		Layout.TPPID 	= 3;
		Layout.TPDCS 	= 4;
		Layout.DateTime = 5;
		blocks 		= 15;
		break;
	case 0x01:
		smprintf(s, "Delivery report\n");
		sms->PDU = SMS_Status_Report;
		Layout.TPStatus	= 4;
		Layout.DateTime = 5;
		Layout.SMSCTime = 12;
		blocks 		= 19;
		break;
	case 0x02:
		smprintf(s, "SMS template\n");
		sms->PDU = SMS_Submit;
		Layout.TPMR	= 3;
		Layout.TPPID 	= 4;
		Layout.TPDCS 	= 5;
		blocks 		= 7;
		break;
	}
	current = blocks + 1;
	for (i=0;i<buffer[blocks];i++) {
		switch (buffer[current]) {
			case 0x80:
				smprintf(s, "User data\n");
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
			default:
				smprintf(s, "Unknown block %02x\n",buffer[current]);
		}
		current = current + buffer[current + 1];
	}
	return GSM_DecodeSMSFrame(sms,buffer,Layout);
}

static GSM_Error N6510_ReplyGetSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int			i;
	int			Width, Height;
	unsigned char		output[500]; //output2[500];
	GSM_Phone_Data		*Data = &s->Phone.Data;

	switch(msg.Buffer[3]) {
	case 0x03:
		smprintf(s, "SMS Message received\n");
		Data->GetSMSMessage->Number=1;
		NOKIA_DecodeSMSState(s, msg.Buffer[5], &Data->GetSMSMessage->SMS[0]);
		switch (msg.Buffer[14]) {
		case 0x00:
		case 0x01:
		case 0x02:
			return N6510_DecodeSMSFrame(s, &Data->GetSMSMessage->SMS[0],msg.Buffer+14);
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
//			if (msg.Length!=282) {
//				output[i++] = SM30_UNICODETEXT;
//				output[i++] = 0;
//				output[i++] = 0; /* Length - later changed */
//				GSM_UnpackEightBitsToSeven(0, msg.Length-282, msg.Length-304, msg.Buffer+282,output2);
//				DecodeDefault(output+i, output2, msg.Length - 282, true);
//				output[i - 1] = strlen(DecodeUnicodeString(output+i)) * 2;
//				i = i + output[i-1];
//			}
			GSM_MakeMultiPartSMS(Data->GetSMSMessage,output,i,UDH_NokiaProfileLong,GSM_Coding_8bit,1,0);
			for (i=0;i<3;i++) {
                		Data->GetSMSMessage->SMS[i].Number[0]=0;
                		Data->GetSMSMessage->SMS[i].Number[1]=0;
			}
			return GE_NONE;
		default:
			smprintf(s, "Unknown SMS type: %i\n",msg.Buffer[8]);
		}
		break;
	case 0x0f:
		smprintf(s, "SMS message info received\n");
		CopyUnicodeString(Data->GetSMSMessage->SMS[0].Name,msg.Buffer+52);
		smprintf(s, "Name: \"%s\"\n",DecodeUnicodeString(Data->GetSMSMessage->SMS[0].Name));
		return GE_NONE;		
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6510_PrivGetSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	GSM_Error		error;
	unsigned char		folderid,namebuffer[200];
	int			location;
	int			i;
	unsigned char req[] = {
		N6110_FRAME_HEADER,
		0x02,		/* msg type: 0x02 for getting sms, 0x0e for sms status */
		0x01,		/* 0x01 SIM, 0x02 ME 	*/
		0x00, 		/* FolderID */
		0x00, 0x02,	/* Location */
		0x01, 0x00};

	N6510_GetSMSLocation(s, &sms->SMS[0], &folderid, &location);

	switch (folderid) {
		case 0x01: req[5] = 0x02; 			 break; /* INBOX SIM 	*/
		case 0x02: req[5] = 0x03; 			 break; /* OUTBOX SIM 	*/
		default	 : req[5] = folderid - 1; req[4] = 0x02; break; /* ME folders	*/
	}
	req[6]=location / 256;
	req[7]=location;

	s->Phone.Data.GetSMSMessage=sms;
	smprintf(s, "Getting sms message info\n");
	req[3] = 0x0e; req[8] = 0x55; req[9] = 0x55;
	error=GSM_WaitFor (s, req, 10, 0x14, 4, ID_GetSMSMessage);
	if (error!=GE_NONE) return error;
	CopyUnicodeString(namebuffer,sms->SMS[0].Name);

	smprintf(s, "Getting sms\n");
	req[3] = 0x02; req[8] = 0x01; req[9] = 0x00;
	error=GSM_WaitFor (s, req, 10, 0x14, 4, ID_GetSMSMessage);
	if (error==GE_NONE) {
		for (i=0;i<sms->Number;i++) {
			N6510_SetSMSLocation(s, &sms->SMS[i], folderid, location);
			sms->SMS[i].Folder 	= folderid;
			sms->SMS[i].InboxFolder = true;
			if (folderid != 0x01 && folderid != 0x02) sms->SMS[i].InboxFolder = false;
			CopyUnicodeString(sms->SMS[i].Name,namebuffer);
		}
	}
	return error;
}

static GSM_Error N6510_GetSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	GSM_Error		error;
	unsigned char		folderid;
	int			location;
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	int			i;
	bool			found = false;

	N6510_GetSMSLocation(s, &sms->SMS[0], &folderid, &location);
	error=N6510_GetSMSFolderStatus(s, folderid);
	if (error!=GE_NONE) return error;
	for (i=0;i<Priv->LastSMSFolder.Number;i++) {
		if (Priv->LastSMSFolder.Location[i]==location) {
			found = true;
			break;
		}
	}
	if (!found) return GE_EMPTY;
	return N6510_PrivGetSMSMessage(s,sms);
}

static GSM_Error N6510_GetNextSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	unsigned char		folderid;
	int			location;
	GSM_Error		error;
	int			i;
	bool			findnextfolder = false;

	if (start) {
		folderid=0x00;
		findnextfolder=true;
		error=N6510_GetSMSFolders(s,&Priv->LastSMSFolders);
		if (error!=GE_NONE) return error;
	} else {
		N6510_GetSMSLocation(s, &sms->SMS[0], &folderid, &location);
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
			if ((folderid-1)>Priv->LastSMSFolders.Number) return GE_EMPTY;
			/* Get next folder status */
			error=N6510_GetSMSFolderStatus(s, folderid);
			if (error!=GE_NONE) return error;
			/* First location from this folder */
			location=Priv->LastSMSFolder.Location[0];
		}
	}
	N6510_SetSMSLocation(s, &sms->SMS[0], folderid, location);

	return N6510_PrivGetSMSMessage(s, sms);
}

static GSM_Error N6510_ReplyGetIncSignalQuality(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Network level changed to: %i\n",msg.Buffer[4]);
	return GE_NONE;
}

static GSM_Error N6510_ReplyGetSignalQuality(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data		*Data = &s->Phone.Data;
	smprintf(s, "Network level received: %i\n",msg.Buffer[8]);
    	Data->SignalQuality->SignalStrength 	= -1;
    	Data->SignalQuality->SignalPercent 	= ((int)msg.Buffer[8]);
    	Data->SignalQuality->BitErrorRate 	= -1;
	return GE_NONE;
}

static GSM_Error N6510_GetSignalQuality(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x0B, 0x00, 0x02, 0x00, 0x00, 0x00};

	s->Phone.Data.SignalQuality = sig;
	smprintf(s, "Getting network level\n");
	return GSM_WaitFor (s, req, 9, 0x0a, 4, ID_GetSignalQuality);
}

static GSM_Error N6510_ReplyGetBatteryCharge(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data		*Data = &s->Phone.Data;
	smprintf(s, "Battery level received: %i\n",msg.Buffer[9]*100/7);
    	Data->BatteryCharge->BatteryPercent 	= ((int)(msg.Buffer[9]*100/7));
    	Data->BatteryCharge->ChargeState 	= 0;
	return GE_NONE;
}

static GSM_Error N6510_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x0A, 0x02, 0x00};

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
	GSM_Phone_Data		*Data = &s->Phone.Data;
	smprintf(s, "Operator logo received\n");
	NOKIA_DecodeNetworkCode(msg.Buffer+12,Data->Bitmap->NetworkCode);
	smprintf(s, "Network code %s\n",Data->Bitmap->NetworkCode);
	Data->Bitmap->Width	= msg.Buffer[20];
	Data->Bitmap->Height	= msg.Buffer[21];
	if (msg.Length == 18) return GE_EMPTY;
	PHONE_DecodeBitmap(GSM_Nokia6510OperatorLogo,msg.Buffer+26,Data->Bitmap);
	return GE_NONE;
}

static GSM_Error N6510_SetMemory(GSM_StateMachine *s, GSM_PhonebookEntry *entry)
{
	int count = 22, blocks;
	unsigned char req[500] = {
		N7110_FRAME_HEADER, 0x0b, 0x00, 0x01, 0x01, 0x00, 0x00, 0x10,
		0x02, 0x00,  /* memory type */
		0x00, 0x00,  /* location */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	if (entry->EntriesNum!=0) {
		req[11] = NOKIA_GetMemoryType(s, entry->MemoryType,N71_65_MEMORY_TYPES);
		if (req[11]==0xff) return GE_NOTSUPPORTED;

		req[12] = (entry->Location >> 8);
		req[13] = entry->Location & 0xff;

		count = count + N71_65_EncodePhonebookFrame(s, req+22, *entry, &blocks, true);
		req[21] = blocks;

		smprintf(s, "Writing phonebook entry\n");
		return GSM_WaitFor (s, req, count, 0x03, 4, ID_SetMemory);
	} else {
		return N71_65_DeleteMemory(s, entry, N71_65_MEMORY_TYPES);
	}  
}

static GSM_Error N6510_ReplySetOperatorLogo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Operator logo set OK\n");
	return GE_NONE;
}

static GSM_Error N6510_SetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	GSM_Phone_Bitmap_Types	Type;
	int			Width, Height;
	GSM_NetworkInfo 	NetInfo;
	GSM_Error		error;
	unsigned char reqStartup[1000] = {
		N7110_FRAME_HEADER, 0x04, 0x0F,
		0x00, 0x00, 0x00,
		0x04, 0xC0, 0x02, 0x00,
		0x41, 0xC0, 0x03, 0x00,
		0x60, 0xC0, 0x04};
	unsigned char reqOp[1000] = {
		N7110_FRAME_HEADER, 0x25, 0x01,
		0x55, 0x00, 0x00, 0x55,
		0x01,			/* 0x01 - not set, 0x02 - set */
		0x0C, 0x08,
		0x62, 0xF0, 0x10,	/* Network code */
		0x03, 0x55, 0x55};

	switch (Bitmap->Type) {
	case GSM_StartupLogo:
		Type = GSM_Nokia7110StartupLogo;
		switch (Bitmap->Location) {
			case 1:
				PHONE_EncodeBitmap(Type, reqStartup + 22, Bitmap);
				break;
			case 2:
				memset(reqStartup+5,0x00,15);
				PHONE_ClearBitmap(Type, reqStartup + 22,0,0);
				break;
			default:
				return GE_NOTSUPPORTED;
		}
		smprintf(s, "Setting startup logo\n");
		return GSM_WaitFor (s, reqStartup, 22+PHONE_GetBitmapSize(Type,0,0), 0x7A, 4, ID_SetBitmap);
	case GSM_WelcomeNoteText:	
		break;
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
			if (error != GE_NONE) return error;
			NOKIA_EncodeNetworkCode(reqOp+12, NetInfo.NetworkCode);
			smprintf(s, "Clearing operator logo\n");
			return GSM_WaitFor (s, reqOp, 18, 0x0A, 4, ID_SetBitmap);
		}
	case GSM_CallerLogo:
		break;
	case GSM_PictureImage:
		break;
	default:
		break;
	}
	return GE_NOTSUPPORTED;
}

static GSM_Error N6510_ReplyGetRingtoneID(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_N6510Data *Priv = &s->Phone.Data.Priv.N6510;		

	smprintf(s, "Ringtone ID received\n");
	Priv->RingtoneID = msg.Buffer[15];
	return GE_NONE;
}

static GSM_Error N6510_ReplySetBinRingtone(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Binary ringtone set\n");
	return GE_NONE;
}

static GSM_Error N6510_SetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, int *maxlength)
{
	GSM_Error		error;
	GSM_Phone_N6510Data 	*Priv = &s->Phone.Data.Priv.N6510;
	GSM_NetworkInfo		NetInfo;
	int			size=200, current;
	unsigned char 		GetIDReq[] = {
		N7110_FRAME_HEADER, 0x01, 0x00, 0x00,
		0x00, 0xFF, 0x06, 0xE1, 0x00,
		0xFF, 0x06, 0xE1, 0x01, 0x42};
	unsigned char		SetPreviewReq[1000] = {
		0xAE,		/* Ringtone ID */
		0x01, 0x00, 0x0D, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00,
		0x00};  	/*Length*/                               
	unsigned char		SetBinaryReq[1000] = {
		N7110_FRAME_HEADER, 0x0E, 0x7F, 0xFF, 0xFE};
//	unsigned char		DelAllRingtoneReq[] = {
//		N7110_FRAME_HEADER, 0x10, 0x7F, 0xFE};

	if (Ringtone->Format == RING_NOTETONE && Ringtone->Location==255)
	{
		smprintf(s, "Getting ringtone ID\n");
		error=GSM_WaitFor (s, GetIDReq, 14, 0xDB, 4, ID_SetRingtone);
		if (error != GE_NONE) return error;
		*maxlength=GSM_EncodeNokiaRTTLRingtone(*Ringtone, SetPreviewReq+11, &size);
		SetPreviewReq[0]  = Priv->RingtoneID;
		SetPreviewReq[10] = size;
		smprintf(s, "Setting ringtone\n");
		error = s->Protocol.Functions->WriteMessage(s, SetPreviewReq, size+11, 0x00);
		if (error!=GE_NONE) return error;
		my_sleep(1000);
		/* We have to make something (not important, what) now */
		/* no answer from phone*/
		return s->Phone.Functions->GetNetworkInfo(s,&NetInfo);
	}
	if (Ringtone->Format == RING_NOKIABINARY) {
//		smprintf(s, "Deleting all user ringtones\n");
//		GSM_WaitFor (s, DelAllRingtoneReq, 6, 0x1F, 4, ID_SetRingtone);

		SetBinaryReq[7] = strlen(DecodeUnicodeString(Ringtone->Name));
		CopyUnicodeString(SetBinaryReq+8,Ringtone->Name);
		current = 8 + strlen(DecodeUnicodeString(Ringtone->Name))*2;
		SetBinaryReq[current++] = Ringtone->NokiaBinary.Length/256 + 1;
		SetBinaryReq[current++] = Ringtone->NokiaBinary.Length%256 + 1;
		SetBinaryReq[current++] = 0x00;
		memcpy(SetBinaryReq+current,Ringtone->NokiaBinary.Frame,Ringtone->NokiaBinary.Length);
		current += Ringtone->NokiaBinary.Length;
		smprintf(s, "Setting binary ringtone\n");
		return GSM_WaitFor (s, SetBinaryReq, current, 0x1F, 4, ID_SetRingtone);
	}
	return GE_NOTSUPPORTED;
}

static GSM_Error N6510_GetProductCode(GSM_StateMachine *s, char *value)
{
	return NOKIA_GetPhoneString(s,"\x00\x03\x04\x0b\x00\x02",6,0x1b,value,ID_GetProductCode,10);
}

static GSM_Error N6510_GetHardware(GSM_StateMachine *s, char *value)
{
	return NOKIA_GetPhoneString(s,"\x00\x03\x02\x07\x00\x02",6,0x1b,value,ID_GetHardware,10);
}

static GSM_Error N6510_Reset(GSM_StateMachine *s, bool hard)
{
	GSM_Error 	error;
	GSM_DateTime	Date;
	unsigned int	i,j;
	unsigned char 	req[] = {0x00, 0x06, 0x00, 0x01, 0x04, 0x00};

	if (s->ConnectionType == GCT_DLR3AT) return GE_NOTSUPPORTED;
	if (hard) return GE_NOTSUPPORTED;

	/* Going to test mode */
	error=GSM_WaitFor (s, req, 6, 0x15, 4, ID_Reset);
	if (error != GE_NONE) return error;
	for (i=0;i<6;i++) {
		GSM_GetCurrentDateTime (&Date);
		j=Date.Second;
		while (j==Date.Second) {
			my_sleep(10);
			GSM_GetCurrentDateTime(&Date);
		}
	}

	GSM_TerminateConnection(s);

	while (!false) {
		error=GSM_InitConnection(s,s->ReplyNum);
		if (error==GE_NONE) break;
		GSM_TerminateConnection(s);
	}

	/* Going to normal mode */
	req[4] = 0x01;
	error=GSM_WaitFor (s, req, 6, 0x15, 4, ID_Reset);

	if (error == GE_NONE) {
		s->Phone.Data.EnableIncomingSMS = false;
		s->Phone.Data.EnableIncomingCB  = false;
	}
	return error;
}

static GSM_Error N6510_ReplyGetToDo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "TODO received\n");
	if (msg.Buffer[4] >= 1 && msg.Buffer[4] <= 3) {
		Data->ToDo->Priority = msg.Buffer[4];
	}
	smprintf(s, "Priority: %i\n",msg.Buffer[4]);

	CopyUnicodeString(Data->ToDo->Entries[0].Text,msg.Buffer+14);
    	Data->ToDo->Entries[0].EntryType = TODO_TEXT;
	Data->ToDo->EntriesNum		 = 1;
	smprintf(s, "Text: \"%s\"\n",DecodeUnicodeString(Data->ToDo->Entries[0].Text));
	return GE_NONE;
}

static GSM_Error N6510_ReplyGetToDoLocations(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int			i;
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;

	smprintf(s, "TODO locations received\n");
	Priv->LastToDo.Number=msg.Buffer[6]*256+msg.Buffer[7];
	smprintf(s, "Number of Entries: %i\n",Priv->LastToDo.Number);
	smprintf(s, "Locations: ");
	for (i=0;i<Priv->LastToDo.Number;i++) {
		Priv->LastToDo.Location[i]=msg.Buffer[12+(i*4)]*256+msg.Buffer[(i*4)+13];
		smprintf(s, "%i ",Priv->LastToDo.Location[i]);
	}
	smprintf(s, "\n");
	return GE_NONE;
}

static GSM_Error N6510_GetToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool refresh)
{
	GSM_Error 		error;
	GSM_NOKIAToDoLocations	*LastToDo = &s->Phone.Data.Priv.N6510.LastToDo;
	unsigned char reqLoc[] = {
			N6110_FRAME_HEADER,
			0x15, 0x01, 0x00, 0x00,
			0x00, 0x00, 0x00};
	unsigned char reqGet[] = {
			N6110_FRAME_HEADER,
			0x03, 0x00, 0x00, 0x80, 0x00,
			0x00, 0x17};		/* Location */

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOTODO)) return GE_NOTSUPPORTED;

	if (refresh) {
		smprintf(s, "Getting ToDo locations\n");
		error = GSM_WaitFor (s, reqLoc, 10, 0x55, 4, ID_GetToDo);
		if (error != GE_NONE) return error;
	}
	if (ToDo->Location > LastToDo->Number) return GE_INVALIDLOCATION;
	reqGet[8] = LastToDo->Location[ToDo->Location-1] / 256;
	reqGet[9] = LastToDo->Location[ToDo->Location-1] % 256;
	s->Phone.Data.ToDo = ToDo;	
	smprintf(s, "Getting ToDo\n");
	return GSM_WaitFor (s, reqGet, 10, 0x55, 4, ID_GetToDo);
}

static GSM_Error N6510_PressKey(GSM_StateMachine *s, GSM_KeyCode Key, bool Press)
{
#ifdef DEVELOP
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x11, 0x00, 0x01, 0x00, 0x00,
		0x00,	/* Event */
		0x01};	/* Number of presses */

//	req[7] = Key;
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
#else
	return GE_NOTSUPPORTED;
#endif
}

static GSM_Error N6510_ReplyGetWAPSettings(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int tmp;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	switch(msg.Buffer[3]) {
	case 0x16:
		Data->WAPSettings->Number = 2;

		smprintf(s, "WAP settings received OK\n");

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
#endif
		Data->WAPSettings->Settings[0].IsContinuous = false;
		if (msg.Buffer[tmp] == 0x01) Data->WAPSettings->Settings[0].IsContinuous = true;
		Data->WAPSettings->Settings[1].IsContinuous = Data->WAPSettings->Settings[0].IsContinuous;

		Data->WAPSettings->Settings[0].IsSecurity = false;
		if (msg.Buffer[tmp+1] == 0x01) Data->WAPSettings->Settings[0].IsSecurity = true;
		Data->WAPSettings->Settings[1].IsSecurity = Data->WAPSettings->Settings[0].IsSecurity;

		tmp+=3;

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

		/* Here starts settings for gprs bearer */
		Data->WAPSettings->Settings[1].Bearer = WAPSETTINGS_BEARER_GPRS;
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
		Data->WAPSettings->Settings[1].IsNormalAuthentication=true;
		if (msg.Buffer[tmp]==0x01) Data->WAPSettings->Settings[1].IsNormalAuthentication=false;

		Data->WAPSettings->Settings[1].IsContinuous = true;
		if (msg.Buffer[tmp+1] == 0x01) Data->WAPSettings->Settings[1].IsContinuous = false;

		Data->WAPSettings->Settings[1].ManualLogin=false;
		if (msg.Buffer[tmp+2]==0x00) Data->WAPSettings->Settings[1].ManualLogin = true;

		tmp+=3;

		NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[1].DialUp,false);
		smprintf(s, "Access point: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[1].DialUp));

		NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[1].IPAddress,true);
		smprintf(s, "IP address: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[1].IPAddress));

		NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[1].User,true);
		smprintf(s, "User name: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[1].User));

		NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[1].Password,true);
		smprintf(s, "Password: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[1].Password));

		return GE_NONE;
	case 0x17:
		smprintf(s, "WAP settings receiving error\n");
		switch (msg.Buffer[4]) {
		case 0x01:
			smprintf(s, "Security error. Inside WAP settings menu\n");
			return GE_INSIDEPHONEMENU;
		case 0x02:
			smprintf(s, "Invalid or empty\n");
			return GE_INVALIDLOCATION;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
			return GE_UNKNOWNRESPONSE;
		}
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetWAPSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
	GSM_Error 	error;
	unsigned char 	req[] = {
		N6110_FRAME_HEADER, 0x15,
		0x00};		/* Location */

	/* We have to enable WAP frames in phone */
	error=DCT3DCT4_EnableWAP(s);
	if (error!=GE_NONE) return error;

	req[4] = settings->Location-1;
	s->Phone.Data.WAPSettings = settings;
	smprintf(s, "Getting WAP settins\n");
	return GSM_WaitFor (s, req, 6, 0x3f, 4, ID_GetWAPSettings);
}

static GSM_Error N6510_ReplySetWAPSettings(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
	case 0x19:
		smprintf(s, "WAP settings cleaned\n");
		return GE_NONE;
	case 0x1a:
		smprintf(s, "WAP settings setting status\n");
		switch (msg.Buffer[4]) {
		case 0x01:
			smprintf(s, "Security error. Inside WAP settings menu\n");
			return GE_INSIDEPHONEMENU;
		case 0x03:
			smprintf(s, "Invalid location\n");
			return GE_INVALIDLOCATION;
		case 0x05:
			smprintf(s, "Written OK\n");
			return GE_NONE;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
			return GE_UNKNOWNRESPONSE;
		}
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6510_SetWAPSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
	GSM_Error 	error;
	int 		i, pad = 0, length, pos = 5, loc1=-1,loc2=-1;
	unsigned char 	req[1000] = {
		N6110_FRAME_HEADER, 0x18,
		0x00};		/* Location */

	/* For now !!! */
	if (!strcmp(s->Phone.Data.ModelInfo->model,"3510")) {
		if (s->Phone.Data.VerNum>3.37) return GE_NOTSUPPORTED;
	}

	/* We have to enable WAP frames in phone */
	error=DCT3DCT4_EnableWAP(s);
	if (error!=GE_NONE) return error;

	memset(req + pos, 0, 1000 - pos);

	req[4] = settings->Location-1;

	for (i=0;i<settings->Number;i++) {
		if (settings->Settings[i].Bearer == WAPSETTINGS_BEARER_DATA) loc1=i;
		if (settings->Settings[i].Bearer == WAPSETTINGS_BEARER_GPRS) loc2=i;
	}

	if (loc1 != -1) {
		/* Name */
		length = strlen(DecodeUnicodeString(settings->Settings[loc1].Title));
		if (!(length % 2)) pad = 1;
		pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc1].Title, false);

		/* Home */
		length = strlen(DecodeUnicodeString(settings->Settings[loc1].HomePage));
		if (((length + pad) % 2)) pad = 2; else pad = 0;
		pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc1].HomePage, true);

		if (settings->Settings[loc1].IsContinuous) req[pos] = 0x01; pos++;
		if (settings->Settings[loc1].IsSecurity) req[pos] = 0x01; pos++;

		req[pos++] = 0x01; //data set
	} else if (loc2 != -1) {
		/* Name */
		length = strlen(DecodeUnicodeString(settings->Settings[loc2].Title));
		if (!(length % 2)) pad = 1;
		pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc2].Title, false);

		/* Home */
		length = strlen(DecodeUnicodeString(settings->Settings[loc2].HomePage));
		if (((length + pad) % 2)) pad = 2; else pad = 0;
		pos += NOKIA_SetUnicodeString(s, req + pos, settings->Settings[loc2].HomePage, true);

		if (settings->Settings[loc2].IsContinuous) req[pos] = 0x01; pos++;
		if (settings->Settings[loc2].IsSecurity) req[pos] = 0x01; pos++;

		req[pos++] = 0x03; //GPRS
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

		req[pos++] = 0x01; //data
	}

	/* How many parts do we send? */
	req[pos++] = 0x02; 			pos += pad;

	/* GSM data */
	memcpy(req + pos, "\x01\x00", 2);	pos += 2;

	if (loc1 != -1) {
		length = strlen(DecodeUnicodeString(settings->Settings[loc1].IPAddress))*2+1;
		length += strlen(DecodeUnicodeString(settings->Settings[loc1].DialUp))*2+2;
		length += strlen(DecodeUnicodeString(settings->Settings[loc1].User))*2+2;
		length += strlen(DecodeUnicodeString(settings->Settings[loc1].Password))*2+2;
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
//	if (length % 2) pos++;

	/* GPRS block */
	memcpy(req + pos, "\x03\x00", 2);	pos += 2;

	if (loc2 != -1) {
		length = strlen(DecodeUnicodeString(settings->Settings[loc2].DialUp))*2+1;
		length += strlen(DecodeUnicodeString(settings->Settings[loc2].IPAddress))*2+2;
		length += strlen(DecodeUnicodeString(settings->Settings[loc2].User))*2+2;
		length += strlen(DecodeUnicodeString(settings->Settings[loc2].Password))*2+2;
	} else {
		length = 7;
	}
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

	/* end of blocks ? */
	memcpy(req + pos, "\x80\x00\x00\x0c", 4);	pos += 4;

	pos += 8;

	smprintf(s, "Setting WAP settins\n");
	return GSM_WaitFor (s, req, pos, 0x3f, 4, ID_SetWAPSettings);
}

static GSM_Error N6510_ReplyGetOriginalIMEI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	if (msg.Buffer[7] == 0x00) {
		smprintf(s, "No SIM card\n");
		return GE_SECURITYERROR;
	} else {
		return NOKIA_ReplyGetPhoneString(msg, s);
	}
}

static GSM_Error N6510_GetOriginalIMEI(GSM_StateMachine *s, char *value)
{
	return NOKIA_GetPhoneString(s,"\x00\x07\x02\x01\x00\x01",6,0x42,value,ID_GetOriginalIMEI,14);
}

static GSM_Error N6510_ReplyDeleteAllToDo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "All TODO deleted\n");
	return GE_NONE;
}

static GSM_Error N6510_DeleteAllToDo(GSM_StateMachine *s)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x11};

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOTODO)) return GE_NOTSUPPORTED;

	smprintf(s, "Deleting all ToDo\n");
	return GSM_WaitFor (s, req, 4, 0x55, 4, ID_DeleteAllToDo);
}

static GSM_Error N6510_ReplyGetToDoFirstLoc(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "TODO first location received: %02x\n",msg.Buffer[9]);
	s->Phone.Data.ToDo->Location = msg.Buffer[9];
	return GE_NONE;
}

static GSM_Error N6510_ReplySetToDo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "TODO set OK\n");
	return GE_NONE;
}

static GSM_Error N6510_SetToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo)
{
	GSM_Error		error;
    	int 			ulen,i;
    	char 			*txt=NULL;
	unsigned char 		reqLoc[] 	= {N6110_FRAME_HEADER, 0x0F};
	unsigned char 		reqSet[500] 	= {
		N6110_FRAME_HEADER, 0x01,
		0x03,		/* Priority */
		0x00,		/* Length of text */
		0x80,0x00,0x00,
		0x18};		/* Location */

	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_NOTODO)) return GE_NOTSUPPORTED;

	s->Phone.Data.ToDo = ToDo;

	if (ToDo->Location == 0) {
		smprintf(s, "Getting first ToDo location\n");
		error = GSM_WaitFor (s, reqLoc, 4, 0x55, 4, ID_SetToDo);
		if (error != GE_NONE) return error;
		reqSet[9] = ToDo->Location;
	} else {
		return GE_NOTSUPPORTED;
	}
	reqSet[4] = ToDo->Priority;
    	for (i=0; i<ToDo->EntriesNum; i++) {
        	if (ToDo->Entries[i].EntryType == TODO_TEXT) {
            		txt = ToDo->Entries[i].Text;
            		break;
        	}
    	}
    	if (txt == NULL) return GE_NOTSUPPORTED; /* XXX: shouldn't this be handled different way? */
    	ulen = strlen(DecodeUnicodeString(txt));
	reqSet[5] = +1;
	CopyUnicodeString(reqSet+10,txt);
	reqSet[10+ulen*2] 	= 0x00;
	reqSet[10+ulen*2+1] 	= 0x00;
	smprintf(s, "Setting ToDo\n");
	return GSM_WaitFor (s, reqSet, 12+ulen*2, 0x55, 4, ID_SetToDo);
}

static GSM_Error N6510_SetWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error 	error;
	int 		count;
	int		location;
	unsigned char req[600] = { N6110_FRAME_HEADER, 0x09 };

	/* For now !!! */
	if (!strcmp(s->Phone.Data.ModelInfo->model,"3510")) {
		if (s->Phone.Data.VerNum>3.37) return GE_NOTSUPPORTED;
	}

	/* We have to enable WAP frames in phone */
	error=DCT3DCT4_EnableWAP(s);
	if (error!=GE_NONE) return error;

	if (bookmark->Location == 0) {
		location = 0xffff;
	} else {
		location = bookmark->Location - 1;
	}
	count = 4;
	req[count++] = (location & 0xff00) >> 8;
	req[count++] = (location & 0x00ff);

	req[count++] = 0x00;
	req[count++] = strlen(DecodeUnicodeString(bookmark->Title));
	CopyUnicodeString(req+count,bookmark->Title);
	count = count + 2*strlen(DecodeUnicodeString(bookmark->Title));

	req[count++] = 0x00;
	req[count++] = strlen(DecodeUnicodeString(bookmark->Address));
	CopyUnicodeString(req+count,bookmark->Address);
	count = count + 2*strlen(DecodeUnicodeString(bookmark->Address));

	req[count++] = 0x00;
	req[count++] = 0x00;

	smprintf(s, "Setting WAP bookmark\n");
	return GSM_WaitFor (s, req, count, 0x3f, 4, ID_SetWAPBookmark);
}

static GSM_Error N6510_ReplyGetSMSStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data		*Data = &s->Phone.Data;
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
			return GE_NONE;
		case 0x0f:
			smprintf(s, "No PIN\n");
			return GE_SECURITYERROR;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
			return GE_UNKNOWNRESPONSE;
		}
	case 0x1a:
		smprintf(s, "Wait a moment. Phone is during power on and busy now\n");
		return GE_SECURITYERROR;		
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	GSM_Error 		error;
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	unsigned char req[] = {N6110_FRAME_HEADER, 0x08, 0x00, 0x00};

	s->Phone.Data.SMSStatus=status;
	smprintf(s, "Getting SMS status\n");
	error = GSM_WaitFor (s, req, 6, 0x14, 2, ID_GetSMSStatus);
	if (error != GE_NONE) return error;

	/* Nokia 6310 and family does not show not "fixed" messages from the
	 * Templates folder, ie. when you save a message to the Templates folder,
	 * SMSStatus does not change! Workaround: get Templates folder status, which
	 * does show these messages.
	 */
	error = N6510_GetSMSFolderStatus(s, 0x06);
	if (error != GE_NONE) return error;
	status->TemplatesUsed = Priv->LastSMSFolder.Number;

	return error;
}

static GSM_Error N6510_ReplyDeleteSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
		case 0x05:
			smprintf(s, "SMS deleted OK\n");
			return GE_NONE;
		case 0x06:
			switch (msg.Buffer[4]) {
				case 0x02:
					smprintf(s, "Invalid location\n");
					return GE_INVALIDLOCATION;
				default:
					smprintf(s, "Unknown error: %02x\n",msg.Buffer[4]);
					return GE_UNKNOWNRESPONSE;
			}
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6510_DeleteSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	unsigned char		folderid;
	int			location;
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x04,
		0x01, 		/* 0x01 for SM, 0x02 for ME */
		0x00, 		/* FolderID */
		0x00, 0x02, 	/* Location */
		0x0F, 0x55};

	N6510_GetSMSLocation(s, sms, &folderid, &location);

	switch (folderid) {
		case 0x01: req[5] = 0x02; 			 break; /* INBOX SIM 	*/
		case 0x02: req[5] = 0x03; 			 break; /* OUTBOX SIM 	*/
		default	 : req[5] = folderid - 1; req[4] = 0x02; break; /* ME folders	*/
	}
	req[6]=location / 256;
	req[7]=location;

	smprintf(s, "Deleting sms\n");
	return GSM_WaitFor (s, req, 10, 0x14, 4, ID_DeleteSMSMessage);
}

static GSM_Error N6510_ReplySendSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[8]) {
		case 0x00:
			smprintf(s, "SMS sent OK\n");
			if (s->User.SendSMSStatus!=NULL) s->User.SendSMSStatus(s->Config.Device,0);
			return GE_NONE;
		default:
			smprintf(s, "SMS not sent OK, error code probably %i\n",msg.Buffer[8]);
			if (s->User.SendSMSStatus!=NULL) s->User.SendSMSStatus(s->Config.Device,msg.Buffer[8]);
			return GE_NONE;
	}
}

static GSM_Error N6510_SendSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	int			length = 11;
	GSM_Error		error;
	GSM_SMSMessageLayout 	Layout;
	unsigned char req [256] = {
		N6110_FRAME_HEADER, 0x02, 0x00, 0x00, 0x00, 0x55, 0x55};

	if (sms->PDU == SMS_Deliver) sms->PDU = SMS_Submit;
	memset(req+9,0x00,sizeof(req) - 9);
	error=N6510_EncodeSMSFrame(s, sms, req + 9, &Layout, &length);
	if (error != GE_NONE) return error;

	smprintf(s, "Sending sms\n");
	return s->Protocol.Functions->WriteMessage(s, req, length + 9, 0x02);
}

static GSM_Error N6510_ReplyGetSecurityStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data	*Data = &s->Phone.Data;

	smprintf(s, "Security Code status received: ");
	switch (msg.Buffer[4]) {
	case 0x01 : smprintf(s, "waiting for Security Code.\n"); *Data->SecurityStatus = GSCT_SecurityCode;	break;
	case 0x07 :
	case 0x02 : smprintf(s, "waiting for PIN.\n");		 *Data->SecurityStatus = GSCT_Pin;		break;
	case 0x03 : smprintf(s, "waiting for PUK.\n");		 *Data->SecurityStatus = GSCT_Puk;		break;
	case 0x05 : smprintf(s, "PIN ok, SIM ok\n");		 *Data->SecurityStatus = GSCT_None;		break;
	case 0x06 : smprintf(s, "No input status\n"); 		 *Data->SecurityStatus = GSCT_None;		break;
	case 0x16 : smprintf(s, "No SIM!\n");			 *Data->SecurityStatus = GSCT_None;		break;
	case 0x1A : smprintf(s, "SIM rejected!\n");		 *Data->SecurityStatus = GSCT_None;		break;
	default   : smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
		    return GE_UNKNOWNRESPONSE;
	}
	return GE_NONE;
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
		return GE_NONE;
	case 0x09:
		switch (msg.Buffer[4]) {
		case 0x06:
			smprintf(s, "Wrong PIN\n");
			return GE_SECURITYERROR;
		case 0x09:
			smprintf(s, "Wrong PUK\n");
			return GE_SECURITYERROR;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
		}
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6510_EnterSecurityCode(GSM_StateMachine *s, GSM_SecurityCode Code)
{
	int 		len = 0;
	unsigned char 	req[15] = {
		N6110_FRAME_HEADER, 0x07,
		0x00};		/* Type of the entered code: 0x02 PIN, 0x03 PUK */

	switch (Code.Type) {
		case GSCT_Pin	: req[4] = 0x02; break;
		default		: return GE_NOTSUPPORTED;
	}                            

	len = strlen(Code.Code);
	memcpy(req+5,Code.Code,len);
	req[5+len]=0x00;

	smprintf(s, "Entering security code\n");
	return GSM_WaitFor (s, req, 6+len, 0x08, 4, ID_EnterSecurityCode);
}

static GSM_Error N6510_ReplySaveSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char folder;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	switch (msg.Buffer[3]) {
	case 0x01:
		switch (msg.Buffer[4]) {
			case 0x00:
				smprintf(s, "Done OK\n");
				smprintf(s, "Folder info: %i %i\n",msg.Buffer[8],msg.Buffer[5]);
				switch (msg.Buffer[8]) {
				case 0x02 : if (msg.Buffer[5] == 0x02) {
						 folder = 0x02; /* INBOX ME */
					    } else {
						 folder = 0x01; /* INBOX SIM */
					    }
					    break;
				case 0x03 : if (msg.Buffer[5] == 0x02) {
						 folder = 0x04; /* OUTBOX ME */
					    } else {
						 folder = 0x03; /* OUTBOX SIM */
					    }
					    break;
				default	  : folder = msg.Buffer[8] + 1;
				}
				N6510_SetSMSLocation(s, Data->SaveSMSMessage,folder,msg.Buffer[6]*256+msg.Buffer[7]);
				smprintf(s, "Saved in folder %i at location %i\n",folder, msg.Buffer[6]*256+msg.Buffer[7]);
				Data->SaveSMSMessage->Folder = folder;
				return GE_NONE;
			case 0x02:
				printf("Incorrect location\n");
				return GE_INVALIDLOCATION;
			case 0x05:
				printf("Incorrect folder\n");
				return GE_INVALIDLOCATION;
			default:
				smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
				return GE_UNKNOWNRESPONSE;
		}
	case 0x17:
		smprintf(s, "SMS name changed\n");
		return GE_NONE;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6510_SaveSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	int			location, length = 11;
	unsigned char		folderid, folder;
	GSM_SMSMessageLayout 	Layout;
	GSM_Error		error;
	unsigned char req [256] = {
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

	N6510_GetSMSLocation(s, sms, &folderid, &location);
	switch (folderid) {
		case 0x01: req[5] = 0x02; 			 break; /* INBOX SIM 	*/
		case 0x02: req[5] = 0x03; 			 break; /* OUTBOX SIM 	*/
		default	 : req[5] = folderid - 1; req[4] = 0x02; break; /* ME folders	*/
	}
	req[6]=location / 256;
	req[7]=location;

	switch (sms->PDU) {
	case SMS_Submit:
		/* Inbox */
		if (folderid == 0x01 || folderid == 0x03) sms->PDU = SMS_Deliver;
		break;
	case SMS_Deliver:
		break;
	default:
		return GE_UNKNOWN;
	}
	if (sms->PDU == SMS_Deliver) {
		switch (sms->State) {
			case GSM_Sent	: /* We use GSM_Read, because phone return error */
			case GSM_Read	: req[8] = 0x01; break;
			case GSM_UnSent	: /* We use GSM_UnRead, because phone return error */
			case GSM_UnRead	: req[8] = 0x03; break;
		}
	} else {
		switch (sms->State) {
			case GSM_Sent	: /* We use GSM_Sent, because phone change folder */
			case GSM_Read	: req[8] = 0x05; break;
			case GSM_UnSent	: /* We use GSM_UnSent, because phone change folder */
			case GSM_UnRead	: req[8] = 0x07; break;
		}
	}
	memset(req+9,0x00,sizeof(req) - 9);
	error=N6510_EncodeSMSFrame(s, sms, req + 9, &Layout, &length);
	if (error != GE_NONE) return error;

	s->Phone.Data.SaveSMSMessage=sms;
	smprintf(s, "Saving sms\n");
	error=GSM_WaitFor (s, req, length+9, 0x14, 4, ID_SaveSMSMessage);
	if (error == GE_NONE && strlen(DecodeUnicodeString(sms->Name))!=0) {
		folder = sms->Folder;
		sms->Folder = 0;
		N6510_GetSMSLocation(s, sms, &folderid, &location);
		switch (folderid) {
			case 0x01: NameReq[5] = 0x02; 				 break; /* INBOX SIM 	*/
			case 0x02: NameReq[5] = 0x03; 			 	 break; /* OUTBOX SIM 	*/
			default	 : NameReq[5] = folderid - 1; NameReq[4] = 0x02; break; /* ME folders	*/
		}
		NameReq[6]=location / 256;
		NameReq[7]=location;
		length = 8;
		CopyUnicodeString(NameReq+length, sms->Name);
		length = length+strlen(DecodeUnicodeString(sms->Name))*2;
		NameReq[length++] = 0;
		NameReq[length++] = 0;
		error=GSM_WaitFor (s, NameReq, length, 0x14, 4, ID_SaveSMSMessage);
		sms->Folder = folder;
	}
	return error;
}

static GSM_Error N6510_ReplyGetDateTime(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Date & time received\n");
	if (msg.Buffer[4]==0x01) {
		NOKIA_DecodeDateTime(s, msg.Buffer+10, s->Phone.Data.DateTime);
		return GE_NONE;
	}
	smprintf(s, "Not set in phone\n");
	return GE_EMPTY;
}

static GSM_Error N6510_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x0A, 0x00, 0x00};

	s->Phone.Data.DateTime=date_time;
	smprintf(s, "Getting date & time\n");
	return GSM_WaitFor (s, req, 6, 0x19, 4, ID_GetDateTime);
}

static GSM_Error N6510_ReplySetDateTime(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Date & time set\n");
	return GE_NONE;
}

static GSM_Error N6510_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER,
		0x01, 0x00, 0x01, 0x01, 0x0c, 0x01, 0x03,
		0x07, 0xd2,	/* Year */
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
		return GE_SECURITYERROR;
	} else {
		sprintf(s->Phone.Data.PhoneString,"%02i/%04i",msg.Buffer[13],msg.Buffer[14]*256+msg.Buffer[15]);
	        return GE_NONE;
	}
}

static GSM_Error N6510_GetManufactureMonth(GSM_StateMachine *s, char *value)
{
	unsigned char req[6] = {0x00, 0x05, 0x02, 0x01, 0x00, 0x02};

	s->Phone.Data.PhoneString=value;
	smprintf(s, "Getting manufacture month\n");
	return GSM_WaitFor (s, req, 6, 0x42, 2, ID_GetManufactureMonth);
}                                       

static GSM_Error N6510_ReplyGetAlarm(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data		*Data = &s->Phone.Data;
	switch(msg.Buffer[3]) {
	case 0x1A:
		smprintf(s, "   Alarm: %02d:%02d\n", msg.Buffer[14], msg.Buffer[15]);
		Data->Alarm->Hour	= msg.Buffer[14];
		Data->Alarm->Minute	= msg.Buffer[15];
		Data->Alarm->Second	= 0;
		return GE_NONE;
	case 0x20:
		smprintf(s, "Alarm state received\n");
		if (msg.Buffer[37] == 0x01) {
			smprintf(s, "   Not set in phone\n");
			return GE_EMPTY;
		}
		smprintf(s, "Enabled\n");
		return GE_NONE;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6510_GetAlarm(GSM_StateMachine *s, GSM_DateTime *alarm, int alarm_number)
{
	unsigned char   StateReq[] = {N6110_FRAME_HEADER, 0x1f, 0x01, 0x00};
	unsigned char   GetReq  [] = {N6110_FRAME_HEADER, 0x19, 0x00, 0x02};
	GSM_Error	error;

	if (alarm_number!=1) return GE_NOTSUPPORTED;

	s->Phone.Data.Alarm=alarm;
	smprintf(s, "Getting alarm state\n");
	error = GSM_WaitFor (s, StateReq, 6, 0x19, 4, ID_GetAlarm);
	if (error != GE_NONE) return error;

	smprintf(s, "Getting alarm\n");
	return GSM_WaitFor (s, GetReq, 6, 0x19, 4, ID_GetAlarm);
}

static GSM_Error N6510_ReplySetAlarm(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Alarm set\n");
	return GE_NONE;
}

static GSM_Error N6510_SetAlarm(GSM_StateMachine *s, GSM_DateTime *alarm, int alarm_number)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER,
		0x11, 0x00, 0x01, 0x01, 0x0c, 0x02,
		0x01, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00,		/* Hours, Minutes */
		0x00, 0x00, 0x00 };

	if (alarm_number!=1) return GE_NOTSUPPORTED;

	req[14] = alarm->Hour;
	req[15] = alarm->Minute;

	smprintf(s, "Setting alarm\n");
	return GSM_WaitFor (s, req, 19, 0x19, 4, ID_SetAlarm);
}

static GSM_Error N6510_ReplyGetRingtonesInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int tmp,i;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	smprintf(s, "Ringtones info received\n");
	memset(Data->RingtonesInfo,0,sizeof(GSM_AllRingtonesInfo));
	Data->RingtonesInfo->Number = msg.Buffer[4] * 256 + msg.Buffer[5];
	tmp = 6;
	for (i=0;i<Data->RingtonesInfo->Number;i++) {			
		Data->RingtonesInfo->Ringtone[i].ID = msg.Buffer[tmp+2] * 256 + msg.Buffer[tmp+3];
		memcpy(Data->RingtonesInfo->Ringtone[i].Name,msg.Buffer+tmp+8,(msg.Buffer[tmp+6]*256+msg.Buffer[tmp+7])*2);
		smprintf(s, "%i. \"%s\"\n",Data->RingtonesInfo->Ringtone[i].ID,DecodeUnicodeString(Data->RingtonesInfo->Ringtone[i].Name));
		tmp = tmp + (msg.Buffer[tmp]*256+msg.Buffer[tmp+1]);
	}
	return GE_NONE;
}

static GSM_Error N6510_PrivGetRingtonesInfo(GSM_StateMachine *s, GSM_AllRingtonesInfo *Info, bool AllRingtones)
{
	unsigned char UserReq[8] = {N7110_FRAME_HEADER, 0x07, 0x00, 0x00, 0x00, 0x02};
	unsigned char All_Req[9] = {N7110_FRAME_HEADER, 0x07, 0x00, 0x00, 0xFE, 0x00, 0x7D};

	s->Phone.Data.RingtonesInfo=Info;
	smprintf(s, "Getting binary ringtones ID\n");
	if (AllRingtones) {
		return GSM_WaitFor (s, All_Req, 9, 0x1f, 4, ID_GetRingtonesInfo);
	} else {
		return GSM_WaitFor (s, UserReq, 8, 0x1f, 4, ID_GetRingtonesInfo);
	}
}

static GSM_Error N6510_GetRingtonesInfo(GSM_StateMachine *s, GSM_AllRingtonesInfo *Info)
{
	return N6510_PrivGetRingtonesInfo(s, Info, true);
}

static GSM_Error N6510_ReplyGetRingtone(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int tmp,i;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	smprintf(s, "Ringtone received\n");
	memcpy(Data->Ringtone->Name,msg.Buffer+8,msg.Buffer[7]*2);
	Data->Ringtone->Name[msg.Buffer[7]*2]=0;
	Data->Ringtone->Name[msg.Buffer[7]*2+1]=0;
	smprintf(s, "Name \"%s\"\n",DecodeUnicodeString(Data->Ringtone->Name));
	/* Looking for end */
	i=8+msg.Buffer[7]*2+3;
	tmp = i;
	while (true) {
		if (msg.Buffer[i]==0x07 && msg.Buffer[i+1]==0x0b) {
			i=i+2; break;
		}
		i++;
		if (i==msg.Length) return GE_EMPTY;
	}	  
	/* Copying frame */
	memcpy(Data->Ringtone->NokiaBinary.Frame,msg.Buffer+tmp,i-tmp);
	Data->Ringtone->NokiaBinary.Length=i-tmp;
	return GE_NONE;
}

static GSM_Error N6510_GetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, bool PhoneRingtone)
{
	GSM_AllRingtonesInfo 	Info;
	GSM_Error		error;
	unsigned char 		req2[6] = {
		N7110_FRAME_HEADER, 0x12,
		0x00, 0xe7}; 	/* Location */

	if (Ringtone->Format == 0x00) Ringtone->Format = RING_NOKIABINARY;

	switch (Ringtone->Format) {
	case RING_NOTETONE:
		/* In the future get binary and convert */
		return GE_NOTSUPPORTED;
	case RING_NOKIABINARY:
		s->Phone.Data.Ringtone=Ringtone;
		error=N6510_PrivGetRingtonesInfo(s, &Info, PhoneRingtone);
		if (error != GE_NONE) return error;
		if (Ringtone->Location > Info.Number) return GE_INVALIDLOCATION;
		req2[4] = Info.Ringtone[Ringtone->Location-1].ID / 256;
		req2[5] = Info.Ringtone[Ringtone->Location-1].ID % 256;
		smprintf(s, "Getting binary ringtone\n");
		return GSM_WaitFor (s, req2, 6, 0x1f, 4, ID_GetRingtone);
	case RING_MIDI:
		return GE_NOTSUPPORTED;
	}
	return GE_NOTSUPPORTED;
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
//	unsigned char 	reqOff2[] = {
//		0x00,0x06,0x01,0x14,0x05,0x04,
//		0x00,0x00,0x00,0x01,0x03,0x08,
//		0x00,0x00,0x00,0x00,0x00,0x00};

	if (start) {
		smprintf(s, "Enabling sound - part 1\n");
		error=GSM_WaitFor (s, reqStart, 6, 0x0b, 4, ID_PlayTone);
		if (error!=GE_NONE) return error;
		smprintf(s, "Enabling sound - part 2 (disabling sound command)\n");
		error=GSM_WaitFor (s, reqOff, 18, 0x0b, 4, ID_PlayTone);
		if (error!=GE_NONE) return error;
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

//		smprintf(s, "Disabling sound - part 1\n");
//		error=GSM_WaitFor (s, reqOff, 18, 0x0b, 4, ID_PlayTone);
//		if (error!=GE_NONE) return error;		
//		smprintf(s, "Disabling sound - part 2\n");
//		return GSM_WaitFor (s, reqOff2, 18, 0x0b, 4, ID_PlayTone);
	}
}

static GSM_Error N6510_ReplyGetPPM(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data		*Data = &s->Phone.Data;
	Data->PhoneString[0] = msg.Buffer[50];
	Data->PhoneString[1] = 0x00;	
	smprintf(s, "Received PPM %s\n",Data->PhoneString);
	return GE_NONE;
}

static GSM_Error N6510_GetPPM(GSM_StateMachine *s,char *value)
{
	unsigned char req[6] = {N6110_FRAME_HEADER, 0x07, 0x01, 0xff};

	s->Phone.Data.PhoneString=value;
	smprintf(s, "Getting PPM\n");
	return GSM_WaitFor (s, req, 6, 0x1b, 3, ID_GetPPM);
}

static GSM_Error N6510_GetSpeedDial(GSM_StateMachine *s, GSM_SpeedDial *SpeedDial)
{
	GSM_PhonebookEntry 	pbk;
	GSM_Error		error;

	pbk.MemoryType			= GMT7110_SP;
	pbk.Location			= SpeedDial->Location;
	SpeedDial->MemoryLocation 	= 0;
	s->Phone.Data.SpeedDial		= SpeedDial;

	smprintf(s, "Getting speed dial\n");
	error=N6510_GetMemory(s,&pbk);
	switch (error) {
	case GE_NOTSUPPORTED:
		smprintf(s, "No speed dials set in phone\n");
		return GE_EMPTY;
	case GE_NONE:
		if (SpeedDial->MemoryLocation == 0) {
			smprintf(s, "Speed dial not assigned or error in firmware\n");
			return GE_EMPTY;
		}
		return GE_NONE;
	default:
		return error;
	}
}

static GSM_Error N6510_ReplyGetProfile(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 	*blockstart;
	int 		i,j;
	GSM_Phone_Data	*Data = &s->Phone.Data;
	                 
	blockstart = msg.Buffer + 7;
	for (i = 0; i < 11; i++) {
		smprintf(s, "Profile feature %02x ",blockstart[1]);
#ifdef DEBUG
		if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) DumpMessage(di.df, blockstart, blockstart[0]);
#endif

		switch (blockstart[1]) {
		case 0x03:
			smprintf(s, "Ringtone ID\n");
			Data->Profile->FeatureID	[Data->Profile->FeaturesNumber] = Profile_RingtoneID;
			Data->Profile->FeatureValue	[Data->Profile->FeaturesNumber] = blockstart[7];
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
	return GE_NONE;
}

static GSM_Error N6510_GetProfile(GSM_StateMachine *s, GSM_Profile *Profile)
{
	unsigned char 	req[150] = {N6110_FRAME_HEADER, 0x01, 0x01, 0x0C, 0x01};
	int 		i, length = 7;

	/* For now !!! */
	if (!strcmp(s->Phone.Data.ModelInfo->model,"3510")) {
		if (s->Phone.Data.VerNum>3.37) return GE_NOTSUPPORTED;
	}

	if (Profile->Location>5) return GE_INVALIDLOCATION;

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

	Profile->CarKitProfile		= false;
	Profile->HeadSetProfile		= false;

	Profile->FeaturesNumber = 0;

	s->Phone.Data.Profile=Profile;
	smprintf(s, "Getting profile\n");
	return GSM_WaitFor (s, req, length, 0x39, 4, ID_GetProfile);
}

static GSM_Error N6510_ReplySetProfile(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 	*blockstart;
	int 		i;

	smprintf(s, "Response to profile writing received!\n");

	blockstart = msg.Buffer + 6;
	for (i = 0; i < msg.Buffer[5]; i++) {
		switch (blockstart[2]) {
		case 0x00:
			if (msg.Buffer[4] == 0x00) 
				smprintf(s, "keypad tone level successfully set!\n");
			else
				smprintf(s, "failed to set keypad tone level! error: %i\n", msg.Buffer[4]);
			break;
		case 0x02:
			if (msg.Buffer[4] == 0x00) 
				smprintf(s, "call alert successfully set!\n");
			else
				smprintf(s, "failed to set call alert! error: %i\n", msg.Buffer[4]);
			break;
		case 0x03:
			if (msg.Buffer[4] == 0x00) 
				smprintf(s, "ringtone successfully set!\n");
			else
			smprintf(s, "failed to set ringtone! error: %i\n", msg.Buffer[4]);
			break;
		case 0x04:
			if (msg.Buffer[4] == 0x00) 
				smprintf(s, "ringtone volume successfully set!\n");
			else
				smprintf(s, "failed to set ringtone volume! error: %i\n", msg.Buffer[4]);
			break;
		case 0x05:
			if (msg.Buffer[4] == 0x00) 
				smprintf(s, "msg.Buffer tone successfully set!\n");
			else
				smprintf(s, "failed to set msg.Buffer tone! error: %i\n", msg.Buffer[4]);
			break;
		case 0x06:
			if (msg.Buffer[4] == 0x00) 
				smprintf(s, "vibration successfully set!\n");
			else
				smprintf(s, "failed to set vibration! error: %i\n", msg.Buffer[4]);
			break;
		case 0x07:
			if (msg.Buffer[4] == 0x00) 
				smprintf(s, "warning tone level successfully set!\n");
			else
				smprintf(s, "failed to set warning tone level! error: %i\n", msg.Buffer[4]);
			break;
		case 0x08:
			if (msg.Buffer[4] == 0x00) 
				smprintf(s, "caller groups successfully set!\n");
			else
				smprintf(s, "failed to set caller groups! error: %i\n", msg.Buffer[4]);
			break;
		case 0x0c:
			if (msg.Buffer[4] == 0x00) 
				smprintf(s, "name successfully set!\n");
			else
				smprintf(s, "failed to set name! error: %i\n", msg.Buffer[4]);
			break;
		default:
			smprintf(s, "Unknown profile subblock type %02x!\n", blockstart[1]);
			break;
		}
		blockstart = blockstart + blockstart[1];
	}
	return GE_NONE;
}

static GSM_Error N6510_SetProfile(GSM_StateMachine *s, GSM_Profile *Profile)
{
	int 		i, length = 7, blocks = 0;
	bool		found;
	unsigned char	ID,Value;
	unsigned char 	req[150] = {
		N6110_FRAME_HEADER, 0x03, 0x01,
		0x06,		/* Number of blocks */
		0x03};

	if (Profile->Location>5) return GE_INVALIDLOCATION;

	for (i=0;i<Profile->FeaturesNumber;i++) {
		found = false;
		switch (Profile->FeatureValue[i]) {
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

static GSM_Error N6510_ReplyIncomingSMS(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_SMSMessage sms;

#ifdef DEBUG
	smprintf(s, "SMS message received\n");
	N6510_DecodeSMSFrame(s, &sms, msg.Buffer+10);
#endif

	if (s->Phone.Data.EnableIncomingSMS && s->User.IncomingSMS!=NULL) {
		sms.State 	 = GSM_UnRead;
		sms.InboxFolder  = true;

		N6510_DecodeSMSFrame(s, &sms, msg.Buffer+10);

		s->User.IncomingSMS(s->Config.Device,sms);
	}
	return GE_NONE;
}

static GSM_Error N6510_DialVoice(GSM_StateMachine *s, char *number)
{
	unsigned int	pos = 4;
	unsigned char 	req[100] = {N6110_FRAME_HEADER,0x01,
		0x0c};			/* Number length in chars */

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
	req[pos++] = 0x01;

	smprintf(s, "Making voice call\n");
	return GSM_WaitFor (s, req, pos, 0x01, 4, ID_DialVoice);
}

/* Old method 1 for accessing calendar */
static GSM_Error N6510_ReplyGetCalendarInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return N71_65_ReplyGetCalendarInfo(msg, s, &s->Phone.Data.Priv.N6510.LastCalendar);
}

/* Old method 1 for accessing calendar */
static GSM_Error N6510_ReplyGetCalendarNotePos(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return N71_65_ReplyGetCalendarNotePos(msg, s,&s->Phone.Data.Priv.N6510.FirstCalendarPos);
}

static GSM_Error N6510_GetNextCalendar(GSM_StateMachine *s,  GSM_CalendarEntry *Note, bool start)
{
	if (IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_CALENDAR35)) {
		/* Note: in known phones texts of notes cut to 50 chars */
		return N71_65_GetNextCalendar2(s,Note,start,&s->Phone.Data.Priv.N6510.LastCalendarYear,&s->Phone.Data.Priv.N6510.LastCalendarPos);
	} else {
		return N71_65_GetNextCalendar1(s,Note,start,&s->Phone.Data.Priv.N6510.LastCalendar,&s->Phone.Data.Priv.N6510.LastCalendarYear,&s->Phone.Data.Priv.N6510.LastCalendarPos);
	}
}

static GSM_Error N6510_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool Past)
{
//	return N71_65_AddCalendar1(s, Note, &s->Phone.Data.Priv.N6510.FirstCalendarPos, Past);
	return N71_65_AddCalendar2(s,Note,Past);
}

static GSM_Error N6510_ReplyLogIntoNetwork(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Probably phone says: I log into network\n");
	return GE_NONE;
}

static GSM_Error N6510_SetFM_Reply(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
#ifdef DEBUG
 	switch (msg.Buffer[4]){
 		case 0x03: smprintf(s, "FM stations cleaned\n");	  break;		
 		case 0x11: smprintf(s, "Setting FM station status OK\n"); break;		
 		case 0x12: smprintf(s, "Setting FM station OK\n");	  break;
 	}
#endif
 	return GE_NONE;
}
 
static GSM_Error N6510_GetFM_Reply(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
  	unsigned char 	name[GSM_MAX_FMSTATION_LENGTH*2+1];
  	int		length;
  	GSM_Phone_Data	*Data = &s->Phone.Data;
  
  	smprintf(s, "Received FM station\n");
 	if (msg.Buffer[3]==0x16) return GE_UNKNOWN;
 	if (msg.Buffer[3]==0x06) {
 		length = msg.Buffer[8];
 		memcpy(name,msg.Buffer+18,length*2);
 		name[length*2]	 = 0x00;
 		name[length*2+1] = 0x00;
 		CopyUnicodeString(Data->FMStation->StationName,name);
 		Data->FMStation->Frequency = 0xffff + msg.Buffer[16] * 0x100 + msg.Buffer[17];
 	}
 	if (msg.Buffer[3]==0x0e) smprintf(s, "getting FM status OK\n");
	return GE_NONE;
}
  
static GSM_Error N6510_GetFMStatus (GSM_StateMachine *s)
{
 	unsigned char req[7] = {N6110_FRAME_HEADER, 0x0d, 0x00, 0x00,0x01};
 	
 	if (!IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_RADIO)) return GE_NOTSUPPORTED;
 	return GSM_WaitFor (s, req, 7, 0x3E, 2, ID_GetFMStation);
}
 
static GSM_Error N6510_GetFMStation (GSM_StateMachine *s, GSM_FMStation *FMStation)
{
 	GSM_Error 		error;
 	GSM_Protocol_Message	msg;
 	int			location;
  	unsigned char req[7] = {N6110_FRAME_HEADER, 0x05,
 				0x00, 		// location
  				0x00,0x01};
 
  	if (!IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_RADIO)) return GE_NOTSUPPORTED;
   	if (FMStation->Location > GSM_MAX_FM_STATION) return GE_INVALIDLOCATION;

  	s->Phone.Data.FMStation=FMStation;
 	error = N6510_GetFMStatus(s);
 	if (error != GE_NONE) return error;	
 
 	location = FMStation->Location-1;
 	msg = *(s->Phone.Data.RequestMsg);
  	if (msg.Buffer[14+location] != location) return GE_EMPTY;
 	req[4] = location;

 	smprintf(s, "Getting FM Station %i\n",FMStation->Location);
 	return GSM_WaitFor (s, req, 7, 0x3E, 2, ID_GetFMStation);
}
  
static GSM_Error N6510_ClearFMStations (GSM_StateMachine *s)
{
	unsigned char req[7] = {N6110_FRAME_HEADER, 0x03,0x0f,0x00,0x01};
 
	if (!IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_RADIO)) return GE_NOTSUPPORTED;

	smprintf(s, "Cleaning FM Stations\n");
	return GSM_WaitFor (s, req, 7, 0x3E, 2, ID_GetFMStation);
}
 
static GSM_Error N6510_SetFMStation (GSM_StateMachine *s, GSM_FMStation *FMStation)
{
 	int 			len, freq, location;	
 	GSM_Error 		error;
 	GSM_Protocol_Message	msg;    
 	unsigned char setstatus[36] = {N6110_FRAME_HEADER,0x11,0x00,0x01,0x01,
 	    			0x00,0x00,0x1c,0x00,0x14,0x00,0x00,
 				0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 				0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 				0xff,0xff,0xff,0xff,0xff,0x01};
 	unsigned char req[64] = {N6110_FRAME_HEADER, 0x12,0x00,0x01,0x00,
 				0x00, 			// 0x0e + (strlen(name) * 2)
 				0x00,			// strlen(name)
 				0x14,0x09,0x00,
 				0x00, 			// location
 				0x00,0x00,0x01,
 				0x00, 			// freqHi
 				0x00, 			// freqLo
 				0x01};
 	if (!IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_RADIO)) return GE_NOTSUPPORTED;
 
 	s->Phone.Data.FMStation=FMStation;
 	location = FMStation->Location-1;

 	error = N6510_GetFMStatus(s);
 	if (error != GE_NONE) return error;

 	msg = *(s->Phone.Data.RequestMsg);
 	memcpy(setstatus+14,msg.Buffer+14,20);
 	setstatus [14+location] = location;

 	error = GSM_WaitFor (s, setstatus, 36 , 0x3E, 2, ID_GetFMStation);
 	if (error != GE_NONE) return error;	

 	req[12] = location;
 	len 	= strlen (DecodeUnicodeString(FMStation->StationName));
 	req[8] 	= len;
 	req[7] 	= 0x0e + len * 2;
 	freq	= FMStation->Frequency-0xffff;
 	req[16] = freq / 0x100;
 	req[17] = freq % 0x100;
 	memcpy (req+18,FMStation->StationName,len*2);

 	smprintf(s, "Setting FM Station %i\n",FMStation->Location);
 	return GSM_WaitFor (s, req, 0x13+len*2, 0x3E, 2, ID_GetFMStation);
}

static GSM_Reply_Function N6510ReplyFunctions[] = {
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x02,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x03,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x04,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x05,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x07,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x09,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x0A,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x0B,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x0C,ID_DialVoice		},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x0C,ID_IncomingFrame	},
	{N71_65_ReplySendDTMF,		"\x01",0x03,0x51,ID_SendDTMF		},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x53,ID_IncomingFrame	},
	{N71_65_ReplySendDTMF,		"\x01",0x03,0x59,ID_SendDTMF		},
	{N71_65_ReplySendDTMF,		"\x01",0x03,0x5E,ID_SendDTMF		},

	{N6510_ReplySendSMSMessage,	"\x02",0x03,0x03,ID_IncomingFrame	},
	{N6510_ReplyIncomingSMS,	"\x02",0x03,0x04,ID_IncomingFrame	},
	{N6510_ReplySetSMSC,		"\x02",0x03,0x13,ID_SetSMSC		},
	{N6510_ReplyGetSMSC,		"\x02",0x03,0x15,ID_GetSMSC		},

	{N6510_ReplyGetMemoryStatus,	"\x03",0x03,0x04,ID_GetMemoryStatus	},
	{N6510_ReplyGetMemory,		"\x03",0x03,0x08,ID_GetMemory		},
	{N71_65_ReplyDeleteMemory,	"\x03",0x03,0x10,ID_SetMemory		},
	{N71_65_ReplyWritePhonebook,	"\x03",0x03,0x0C,ID_SetBitmap		},
	{N71_65_ReplyWritePhonebook,	"\x03",0x03,0x0C,ID_SetMemory		},

	{N71_65_ReplyUSSDInfo,		"\x06",0x03,0x03,ID_IncomingFrame	},
	{NONEFUNCTION,			"\x06",0x03,0x06,ID_IncomingFrame	},

	{N6510_ReplyEnterSecurityCode,	"\x08",0x03,0x08,ID_EnterSecurityCode	},
	{N6510_ReplyEnterSecurityCode,	"\x08",0x03,0x09,ID_EnterSecurityCode	},
	{N6510_ReplyGetSecurityStatus,	"\x08",0x03,0x12,ID_GetSecurityStatus	},

	{N6510_ReplyGetNetworkInfo,	"\x0A",0x03,0x01,ID_GetNetworkInfo	},
	{N6510_ReplyGetNetworkInfo,	"\x0A",0x03,0x01,ID_IncomingFrame	},
	{N6510_ReplyLogIntoNetwork,	"\x0A",0x03,0x02,ID_IncomingFrame	},
	{N6510_ReplyGetSignalQuality,	"\x0A",0x03,0x0C,ID_GetSignalQuality	},
	{N6510_ReplyGetIncSignalQuality,"\x0A",0x03,0x1E,ID_IncomingFrame	},
	{N6510_ReplyGetOperatorLogo,	"\x0A",0x03,0x24,ID_GetBitmap		},
	{N6510_ReplySetOperatorLogo,	"\x0A",0x03,0x26,ID_SetBitmap		},

	{NONEFUNCTION,			"\x0B",0x03,0x01,ID_PlayTone		},
	{NONEFUNCTION,			"\x0B",0x03,0x15,ID_PlayTone		},
	{NONEFUNCTION,			"\x0B",0x03,0x16,ID_PlayTone		},

	{N71_65_ReplyAddCalendar1,	"\x13",0x03,0x02,ID_SetCalendarNote	},/*method 1*/
	{N71_65_ReplyAddCalendar1,	"\x13",0x03,0x04,ID_SetCalendarNote	},/*method 1*/
	{N71_65_ReplyAddCalendar1,	"\x13",0x03,0x06,ID_SetCalendarNote	},/*method 1*/
	{N71_65_ReplyAddCalendar1,	"\x13",0x03,0x08,ID_SetCalendarNote	},/*method 1*/
	{N71_65_ReplyDelCalendar,	"\x13",0x03,0x0C,ID_DeleteCalendarNote	},
	{N71_65_ReplyGetNextCalendar1,	"\x13",0x03,0x1A,ID_GetCalendarNote	},/*method 1*/
	{N6510_ReplyGetCalendarNotePos,	"\x13",0x03,0x32,ID_GetCalendarNotePos	},/*method 1*/
	{N6510_ReplyGetCalendarInfo,	"\x13",0x03,0x3B,ID_GetCalendarNotesInfo},/*method 1*/
	{N71_65_ReplyGetNextCalendar2,	"\x13",0x03,0x3F,ID_GetCalendarNote	},/*method 2*/
	{N71_65_ReplyAddCalendar2,	"\x13",0x03,0x41,ID_SetCalendarNote	},/*method 2*/

	{N6510_ReplySaveSMSMessage,	"\x14",0x03,0x01,ID_SaveSMSMessage	},
	{N6510_ReplyGetSMSMessage,	"\x14",0x03,0x03,ID_GetSMSMessage	},
	{N6510_ReplyDeleteSMSMessage,	"\x14",0x03,0x05,ID_DeleteSMSMessage	},
	{N6510_ReplyDeleteSMSMessage,	"\x14",0x03,0x06,ID_DeleteSMSMessage	},
	{N6510_ReplyGetSMSStatus,	"\x14",0x03,0x09,ID_GetSMSStatus	},
	{N6510_ReplyGetSMSFolderStatus,	"\x14",0x03,0x0d,ID_GetSMSFolderStatus	},
	{N6510_ReplyGetSMSMessage,	"\x14",0x03,0x0f,ID_GetSMSMessage	},
	{N6510_ReplyGetSMSFolders,	"\x14",0x03,0x13,ID_GetSMSFolders	},
	{N6510_ReplySaveSMSMessage,	"\x14",0x03,0x17,ID_SaveSMSMessage	},
	{N6510_ReplyGetSMSStatus,	"\x14",0x03,0x1a,ID_GetSMSStatus	},

	{NONEFUNCTION,			"\x15",0x01,0x31,ID_Reset		},

	{N6510_ReplyGetBatteryCharge,	"\x17",0x03,0x0B,ID_GetBatteryCharge	},

	{N6510_ReplySetDateTime,	"\x19",0x03,0x02,ID_SetDateTime		},
	{N6510_ReplyGetDateTime,	"\x19",0x03,0x0B,ID_GetDateTime		},
	{N6510_ReplySetAlarm,		"\x19",0x03,0x12,ID_SetAlarm		},
	{N6510_ReplyGetAlarm,		"\x19",0x03,0x1A,ID_GetAlarm		},
	{N6510_ReplyGetAlarm,		"\x19",0x03,0x20,ID_GetAlarm		},

	{N6510_ReplyGetIMEI,		"\x1B",0x03,0x01,ID_GetIMEI		},
	{NOKIA_ReplyGetPhoneString,	"\x1B",0x03,0x08,ID_GetHardware		},
	{N6510_ReplyGetPPM,		"\x1B",0x03,0x08,ID_GetPPM		},
	{NOKIA_ReplyGetPhoneString,	"\x1B",0x03,0x0C,ID_GetProductCode	},

	{N6510_ReplyGetRingtonesInfo,	"\x1f",0x03,0x08,ID_GetRingtonesInfo	},
	{N6510_ReplyGetRingtone,	"\x1f",0x03,0x13,ID_GetRingtone		},
	{N6510_ReplySetBinRingtone,	"\x1f",0x03,0x0F,ID_SetRingtone		},

	{N6510_ReplyGetProfile,		"\x39",0x03,0x02,ID_GetProfile		},
	{N6510_ReplySetProfile,		"\x39",0x03,0x04,ID_SetProfile		},

 	{N6510_SetFM_Reply,		"\x3E",0x03,0x15,ID_GetFMStation	},
 	{N6510_GetFM_Reply,		"\x3E",0x02,0x00,ID_GetFMStation	},

	{DCT3DCT4_ReplyEnableWAP,	"\x3f",0x03,0x01,ID_EnableWAP		},
	{DCT3DCT4_ReplyEnableWAP,	"\x3f",0x03,0x02,ID_EnableWAP		},
	{N6510_ReplyGetWAPBookmark,	"\x3f",0x03,0x07,ID_GetWAPBookmark	},
	{N6510_ReplyGetWAPBookmark,	"\x3f",0x03,0x08,ID_GetWAPBookmark	},
	{DCT3DCT4_ReplySetWAPBookmark,	"\x3f",0x03,0x0A,ID_SetWAPBookmark	},
	{DCT3DCT4_ReplySetWAPBookmark,	"\x3f",0x03,0x0B,ID_SetWAPBookmark	},
	{DCT3DCT4_ReplyDelWAPBookmark,	"\x3f",0x03,0x0D,ID_DeleteWAPBookmark	},
	{DCT3DCT4_ReplyDelWAPBookmark,	"\x3f",0x03,0x0E,ID_DeleteWAPBookmark	},
	{N6510_ReplySetWAPSettings,	"\x3f",0x03,0x19,ID_SetWAPSettings	},
	{N6510_ReplySetWAPSettings,	"\x3f",0x03,0x1A,ID_SetWAPSettings	},
	{N6510_ReplyGetWAPSettings,	"\x3f",0x03,0x16,ID_GetWAPSettings	},
	{N6510_ReplyGetWAPSettings,	"\x3f",0x03,0x17,ID_GetWAPSettings	},

	{N6510_ReplyGetOriginalIMEI,	"\x42",0x07,0x00,ID_GetOriginalIMEI	},
	{N6510_ReplyGetManufactureMonth,"\x42",0x07,0x00,ID_GetManufactureMonth	},
	{N6510_ReplyGetOriginalIMEI,	"\x42",0x07,0x01,ID_GetOriginalIMEI	},
	{N6510_ReplyGetManufactureMonth,"\x42",0x07,0x02,ID_GetManufactureMonth	},

	{N6510_ReplySetToDo,		"\x55",0x03,0x02,ID_SetToDo		},
	{N6510_ReplyGetToDo,		"\x55",0x03,0x04,ID_GetToDo		},
	{N6510_ReplyGetToDoFirstLoc,	"\x55",0x03,0x10,ID_SetToDo		},
	{N6510_ReplyDeleteAllToDo,	"\x55",0x03,0x12,ID_DeleteAllToDo	},
	{N6510_ReplyGetToDoLocations,	"\x55",0x03,0x16,ID_GetToDo		},

	{N6510_ReplyStartupNoteLogo,	"\x7A",0x04,0x01,ID_GetBitmap		},
	{N6510_ReplyStartupNoteLogo,	"\x7A",0x04,0x0F,ID_GetBitmap		},
	{N6510_ReplyStartupNoteLogo,	"\x7A",0x04,0x0F,ID_SetBitmap		},

	{DCT3DCT4_ReplyGetModelFirmware,"\xD2",0x02,0x00,ID_GetModel		},
	{DCT3DCT4_ReplyGetModelFirmware,"\xD2",0x02,0x00,ID_GetFirmware		},

	{N6510_ReplyGetRingtoneID,	"\xDB",0x03,0x02,ID_SetRingtone		},

	{NULL,				"\x00",0x00,0x00,ID_None		}
};

GSM_Phone_Functions N6510Phone = {
	"3510|3510i|3530|5100|6100|6310|6310i|6510|6610|7210|8310|8910",
	N6510ReplyFunctions,
	N6510_Initialise,
	NONEFUNCTION,		/*	Terminate 		*/
	GSM_DispatchMessage,
	DCT3DCT4_GetModel,
	DCT3DCT4_GetFirmware,
	N6510_GetIMEI,
	N6510_GetDateTime,
	N6510_GetAlarm,
	N6510_GetMemory,
	N6510_GetMemoryStatus,
	N6510_GetSMSC,
	N6510_GetSMSMessage,
	N6510_GetSMSFolders,
	NOKIA_GetManufacturer,
	N6510_GetNextSMSMessage,
	N6510_GetSMSStatus,
	NOKIA_SetIncomingSMS,
	N6510_GetNetworkInfo,
	N6510_Reset,
	N6510_DialVoice,
	NOTIMPLEMENTED,		/*	AnswerCall		*/
	NOTIMPLEMENTED,		/*	CancelCall		*/
	N6510_GetRingtone,
	DCT3DCT4_GetWAPBookmark,
	N6510_GetBitmap,
	N6510_SetRingtone,
	N6510_SaveSMSMessage,
	N6510_SendSMSMessage,
	N6510_SetDateTime,
	N6510_SetAlarm,
	N6510_SetBitmap,
	N6510_SetMemory,
	N6510_DeleteSMSMessage,
	N6510_SetWAPBookmark,
	DCT3DCT4_DeleteWAPBookmark,
	N6510_GetWAPSettings,
	NOTIMPLEMENTED,		/* 	SetIncomingCB		*/
	N6510_SetSMSC,
	N6510_GetManufactureMonth,
	N6510_GetProductCode,
	N6510_GetOriginalIMEI,
	N6510_GetHardware,
	N6510_GetPPM,
	N6510_PressKey,
	N6510_GetToDo,
	N6510_DeleteAllToDo,
	N6510_SetToDo,
	N6510_PlayTone,
	N6510_EnterSecurityCode,
	N6510_GetSecurityStatus,
	N6510_GetProfile,
	N6510_GetRingtonesInfo,
	N6510_SetWAPSettings,
	N6510_GetSpeedDial,
	NOTIMPLEMENTED,		/*	SetSpeedDial		*/
	NOTIMPLEMENTED,		/*	ResetPhoneSettings	*/
	DCT3DCT4_SendDTMF,
	NOTSUPPORTED,		/*	GetDisplayStatus	*/
	NOTIMPLEMENTED,		/*	SetAutoNetworkLogin	*/
	N6510_SetProfile,
	NOTSUPPORTED,		/*	GetSIMIMSI		*/
	NOKIA_SetIncomingCall,
    	N6510_GetNextCalendar,
	N71_65_DelCalendar,
	N6510_AddCalendar,
	N6510_GetBatteryCharge,
	N6510_GetSignalQuality,
	NOTSUPPORTED,       	/*  	GetCategory 		*/
	NOTSUPPORTED,        	/*  	GetCategoryStatus 	*/
    	N6510_GetFMStation,
     	N6510_SetFMStation,
 	N6510_ClearFMStations,
	NOKIA_SetIncomingUSSD
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
