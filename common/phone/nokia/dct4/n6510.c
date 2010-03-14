
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
	unsigned char Info[] = {N6110_FRAME_HEADER,0x10,0x06,0x01,0x02,0x0a,0x14,0x17,0x39};

	/* Enables various things like incoming SMS, call info, etc. */
	return s->Protocol.Functions->WriteMessage(s, Info, 11, 0x10);
}

static GSM_Error N6510_ReplyGetIMEI(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	memcpy(Data->IMEI,msg.Buffer + 10, 16);
	dprintf("Received IMEI %s\n",Data->IMEI);
	return GE_NONE;
}

static GSM_Error N6510_GetIMEI (GSM_StateMachine *s, unsigned char *imei)
{
	unsigned char req[5] = {N6110_FRAME_HEADER, 0x00, 0x41};

	s->Phone.Data.IMEI=imei;
	dprintf("Getting IMEI\n");
	return GSM_WaitFor (s, req, 5, 0x1B, 2, ID_GetIMEI);
}

static GSM_Error N6510_ReplyGetMemory(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Phonebook entry received\n");
	switch (msg.Buffer[6]) {
	case 0x0f:
		return N71_65_ReplyGetMemoryError(msg.Buffer[10],Data);
	default:
		return N71_65_DecodePhonebook(Data->Memory,Data->Bitmap,Data->SpeedDial,msg.Buffer+22,msg.Length-22);
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

	req[9] = NOKIA_GetMemoryType(entry->MemoryType,N71_65_MEMORY_TYPES);
	if (req[9]==0xff) return GE_NOTSUPPORTED;

	if (entry->Location==0x00) return GE_INVALIDLOCATION;

	req[14] = (entry->Location>>8);
	req[15] = entry->Location & 0xff;

	s->Phone.Data.Memory=entry;
	dprintf("Getting phonebook entry\n");
	return GSM_WaitFor (s, req, 20, 0x03, 4, ID_GetMemory);
}

static GSM_Error N6510_ReplyStartupNoteLogo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[4]) {
	case 0x01:
		dprintf("Welcome note text received\n");
		CopyUnicodeString(Data->Bitmap->Text,msg.Buffer+6);
		dprintf("Text is \"%s\"\n",DecodeUnicodeString(Data->Bitmap->Text));
		return GE_NONE;
	case 0x0f:
		if (Data->RequestID == ID_GetBitmap) {
			dprintf("Startup logo received\n");
			PHONE_DecodeBitmap(GSM_Nokia7110StartupLogo, msg.Buffer + 22, Data->Bitmap);
		}
		if (Data->RequestID == ID_SetBitmap) {
			dprintf("Startup logo set\n");
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
		dprintf("Getting startup logo\n");
		return GSM_WaitFor (s, reqStartup, 5, 0x7A, 4, ID_GetBitmap);
	case GSM_WelcomeNoteText:
		dprintf("Getting welcome note\n");
		return GSM_WaitFor (s, reqNote, 6, 0x7A, 4, ID_GetBitmap);
	case GSM_CallerLogo:
		/* You can only get logos which have been altered, the standard */
		/* logos can't be read!! */
		pbk.MemoryType	= GMT7110_CG;
		pbk.Location	= Bitmap->Location;
		dprintf("Getting caller group logo\n");
		error=N6510_GetMemory(s,&pbk);
		if (error==GE_NONE) NOKIA_GetDefaultCallerGroupName(s, Bitmap);
		return error;
	case GSM_OperatorLogo:
		dprintf("Getting operator logo\n");
		return GSM_WaitFor (s, reqOp, 9, 0x0A, 4, ID_GetBitmap);
	case GSM_PictureImage:
		break;
	default:
		break;
	}
	return GE_NOTSUPPORTED;
}

static GSM_Error N6510_ReplyGetMemoryStatus(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Memory status received\n");
	/* Quess ;-)) */
	if (msg.Buffer[14]==0x10) {
		Data->MemoryStatus->Free = msg.Buffer[18]*256 + msg.Buffer[19];
	} else {
		Data->MemoryStatus->Free = msg.Buffer[17];
	}
	dprintf("   Size       : %i\n",Data->MemoryStatus->Free);
	Data->MemoryStatus->Used = msg.Buffer[20]*256 + msg.Buffer[21];
	dprintf("   Used       : %i\n",Data->MemoryStatus->Used);
	Data->MemoryStatus->Free -= Data->MemoryStatus->Used;
	dprintf("   Free       : %i\n",Data->MemoryStatus->Free);
	return GE_NONE;
}

static GSM_Error N6510_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x03, 0x02,
		0x00,		/* MemoryType */
		0x55, 0x55, 0x55, 0x00};

	req[5] = NOKIA_GetMemoryType(Status->MemoryType,N71_65_MEMORY_TYPES);
	if (req[5]==0xff) return GE_NOTSUPPORTED;

	s->Phone.Data.MemoryStatus=Status;
	dprintf("Getting memory status\n");
	return GSM_WaitFor (s, req, 10, 0x03, 4, ID_GetMemoryStatus);
}

static GSM_Error N6510_ReplyGetSMSC(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int i, current, j;

	switch (msg.Buffer[4]) {
		case 0x00:
			dprintf("SMSC received\n");
			break;
		case 0x02:
			dprintf("SMSC empty\n");
			return GE_INVALIDLOCATION;
		default:
			dprintf("Unknown SMSC state: %02x\n",msg.Buffer[4]);
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
				dprintf("Too long name\n");
				return GE_UNKNOWNRESPONSE;
			}
			CopyUnicodeString(Data->SMSC->Name,msg.Buffer+current+4);
			dprintf("   Name \"%s\"\n", DecodeUnicodeString(Data->SMSC->Name));
			break;
		case 0x82:
			switch (msg.Buffer[current+2]) {
			case 0x01:
				GSM_UnpackSemiOctetNumber(Data->SMSC->DefaultNumber,msg.Buffer+current+4,true);
				dprintf("   Default number \"%s\"\n", DecodeUnicodeString(Data->SMSC->DefaultNumber));
				break;
			case 0x02:
				GSM_UnpackSemiOctetNumber(Data->SMSC->Number,msg.Buffer+current+4,false);
				dprintf("   Number \"%s\"\n", DecodeUnicodeString(Data->SMSC->Number));
				break;
			default:
				dprintf("Unknown SMSC number: %02x\n",msg.Buffer[current+2]);
				return GE_UNKNOWNRESPONSE;
			}
			break;
		default:
			dprintf("Unknown SMSC block: %02x\n",msg.Buffer[current]);
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
	dprintf("Getting SMSC\n");
	return GSM_WaitFor (s, req, 6, 0x02, 4, ID_GetSMSC);
}

static GSM_Error N6510_ReplySetSMSC(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[4]) {
		case 0x00:
			dprintf("SMSC set OK\n");
			return GE_NONE;
		case 0x02:
			dprintf("Invalid SMSC location\n");
			return GE_INVALIDLOCATION;
		default:
			dprintf("Unknown SMSC state: %02x\n",msg.Buffer[4]);
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
		dprintf("Too long SMSC number in frame\n");
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
		dprintf("Too long SMSC number in frame\n");
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
	
	dprintf("Setting SMSC\n");
	return GSM_WaitFor (s, req, count, 0x02, 4, ID_SetSMSC);
}

static GSM_Error N6510_ReplyGetNetworkInfo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int		current = msg.Buffer[7]+7, tmp;
#ifdef DEBUG
	char		name[100];
	GSM_NetworkInfo NetInfo;

	switch (msg.Buffer[8]) {
		case 0x00 : dprintf("   Logged into home network.\n");		break;
		case 0x01 : dprintf("   Logged into a roaming network.\n");	break;
		case 0x04 :
		case 0x09 : dprintf("   Not logged in any network!");		break;
		default	  : dprintf("   Unknown network status!\n");		break;
	}
	if (msg.Buffer[8]==0x00 || msg.Buffer[8] == 0x01) {
		tmp = 10;
		NOKIA_GetUnicodeString(&tmp, msg.Buffer,name,true);
		dprintf("   Network name for phone    : %s\n",DecodeUnicodeString(name));
		NOKIA_DecodeNetworkCode(msg.Buffer + (current + 7),NetInfo.NetworkCode);
		sprintf(NetInfo.CellID, "%02x%02x", msg.Buffer[current+5], msg.Buffer[current+6]);
		sprintf(NetInfo.LAC,	"%02x%02x", msg.Buffer[current+1], msg.Buffer[current+2]);
		dprintf("   CellID                    : %s\n", NetInfo.CellID);
		dprintf("   LAC                       : %s\n", NetInfo.LAC);
		dprintf("   Network code              : %s\n", NetInfo.NetworkCode);
		dprintf("   Network name for Gammu    : %s ",
			DecodeUnicodeString(GSM_GetNetworkName(NetInfo.NetworkCode)));
		dprintf("(%s)\n",DecodeUnicodeString(GSM_GetCountryName(NetInfo.NetworkCode)));
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
			NOKIA_GetUnicodeString(&tmp, msg.Buffer,Data->NetworkInfo->NetworkName,true);
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
	dprintf("Getting network info\n");
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
		dprintf("Too long phone number in frame\n");
		return GE_UNKNOWN;
	}

	return GE_NONE;
}

static GSM_Error N6510_ReplyGetSMSFolders(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int j,tmp, num = 0;

	switch (msg.Buffer[3]) {
	case 0x13:
		dprintf("SMS folders names received\n");
		Data->SMSFolders->Number=msg.Buffer[5]+2;		
		for (j=0;j<msg.Buffer[5];j++) {
			tmp = 10 + (j * 40);
			dprintf("Folder index: %02x",msg.Buffer[tmp - 2]);
			if (msg.Buffer[tmp - 1]>GSM_MAX_SMS_FOLDER_NAME_LEN) {
				dprintf("Too long text\n");
				return GE_UNKNOWNRESPONSE;
			}
			CopyUnicodeString(Data->SMSFolders->Folder[num].Name,msg.Buffer + tmp);
			dprintf(", folder name: \"%s\"\n",DecodeUnicodeString(Data->SMSFolders->Folder[num].Name));
			if (num == 0x00 || num == 0x02) {
				num++;
				CopyUnicodeString(Data->SMSFolders->Folder[num].Name,msg.Buffer + tmp);
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
	dprintf("Getting SMS folders\n");
	return GSM_WaitFor (s, req, 6, 0x14, 4, ID_GetSMSFolders);
}

static GSM_Error N6510_ReplyGetSMSFolderStatus(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int			i;
	GSM_Phone_N6510Data	*Priv = &Data->Priv.N6510;

	dprintf("SMS folder status received\n");
	Priv->LastSMSFolder.Number=msg.Buffer[6]*256+msg.Buffer[7];
	dprintf("Number of Entries: %i\n",Priv->LastSMSFolder.Number);
	dprintf("Locations: ");
	for (i=0;i<Priv->LastSMSFolder.Number;i++) {
		Priv->LastSMSFolder.Location[i]=msg.Buffer[8+(i*2)]*256+msg.Buffer[(i*2)+9];
		dprintf("%i ",Priv->LastSMSFolder.Location[i]);
	}
	dprintf("\n");
	NOKIA_SortSMSFolderStatus(&Priv->LastSMSFolder);
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

	dprintf("Getting SMS folder status\n");
	return GSM_WaitFor (s, req, 10, 0x14, 4, ID_GetSMSFolderStatus);
}

static void N6510_GetSMSLocation(GSM_SMSMessage *sms, unsigned char *folderid, int *location)
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
	dprintf("SMS folder %i & location %i -> 6510 folder %i & location %i\n",
		sms->Folder,sms->Location,*folderid,*location);
}

static void N6510_SetSMSLocation(GSM_SMSMessage *sms, unsigned char folderid, int location)
{
	sms->Folder	= 0;
	sms->Location	= (folderid - 0x01) * PHONE_MAXSMSINFOLDER + location;
	dprintf("6510 folder %i & location %i -> SMS folder %i & location %i\n",
		folderid,location,sms->Folder,sms->Location);
}

static GSM_Error N6510_DecodeSMSFrame(GSM_SMSMessage *sms, unsigned char *buffer)
{
	int 			i, current, blocks=0;
	GSM_SMSMessageLayout 	Layout;

	memset(&Layout,255,sizeof(GSM_SMSMessageLayout));
	Layout.firstbyte = 2;
	switch (buffer[0]) {
	case 0x00:
		dprintf("SMS deliver\n");
		sms->PDU = SMS_Deliver;
		Layout.TPPID 	= 3;
		Layout.TPDCS 	= 4;
		Layout.DateTime = 5;
		blocks 		= 15;
		break;
	case 0x01:
		dprintf("Delivery report\n");
		sms->PDU = SMS_Status_Report;
		Layout.TPStatus	= 4;
		Layout.DateTime = 5;
		Layout.SMSCTime = 12;
		blocks 		= 19;
		break;
	case 0x02:
		dprintf("SMS template\n");
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
				dprintf("User data\n");
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
						dprintf("Phone number\n");
						Layout.Number = current + 4;
						break;
					case 0x02:
						dprintf("SMSC number\n");
						Layout.SMSCNumber = current + 4;
						break;
					default:
						dprintf("Unknown number\n");
						break;
				}
				break;
			default:
				dprintf("Unknown block %02x\n",buffer[current]);
		}
		current = current + buffer[current + 1];
	}
	return GSM_DecodeSMSFrame(sms,buffer,Layout);
}

static GSM_Error N6510_ReplyGetSMSMessage(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int			i;
	int			Width, Height;
	unsigned char		output[500]; //output2[500];

	switch(msg.Buffer[3]) {
	case 0x03:
		dprintf("SMS Message received\n");
		Data->GetSMSMessage->Number=1;
		NOKIA_DecodeSMSState(msg.Buffer[5], &Data->GetSMSMessage->SMS[0]);
		switch (msg.Buffer[14]) {
		case 0x00:
		case 0x01:
		case 0x02:
			return N6510_DecodeSMSFrame(&Data->GetSMSMessage->SMS[0],msg.Buffer+14);
		case 0xA0:
			dprintf("Picture Image\n");
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
			memcpy(output+i,msg.Buffer+30,PHONE_GetBitmapSize(GSM_NokiaPictureImage));
			i = i + PHONE_GetBitmapSize(GSM_NokiaPictureImage);
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
			dprintf("Unknown SMS type: %i\n",msg.Buffer[8]);
		}
		break;
	case 0x0f:
		dprintf("SMS message info received\n");
		CopyUnicodeString(Data->GetSMSMessage->SMS[0].Name,msg.Buffer+52);
		dprintf("Name: \"%s\"\n",DecodeUnicodeString(Data->GetSMSMessage->SMS[0].Name));
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

	N6510_GetSMSLocation(&sms->SMS[0], &folderid, &location);

	switch (folderid) {
		case 0x01: req[5] = 0x02; 			 break; /* INBOX SIM 	*/
		case 0x02: req[5] = 0x03; 			 break; /* OUTBOX SIM 	*/
		default	 : req[5] = folderid - 1; req[4] = 0x02; break; /* ME folders	*/
	}
	req[6]=location / 256;
	req[7]=location;

	s->Phone.Data.GetSMSMessage=sms;
	dprintf("Getting sms message info\n");
	req[3] = 0x0e; req[8] = 0x55; req[9] = 0x55;
	error=GSM_WaitFor (s, req, 10, 0x14, 4, ID_GetSMSMessage);
	if (error!=GE_NONE) return error;
	CopyUnicodeString(namebuffer,sms->SMS[0].Name);

	dprintf("Getting sms\n");
	req[3] = 0x02; req[8] = 0x01; req[9] = 0x00;
	error=GSM_WaitFor (s, req, 10, 0x14, 4, ID_GetSMSMessage);
	if (error==GE_NONE) {
		for (i=0;i<sms->Number;i++) {
			N6510_SetSMSLocation(&sms->SMS[i], folderid, location);
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

	N6510_GetSMSLocation(&sms->SMS[0], &folderid, &location);
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
		N6510_GetSMSLocation(&sms->SMS[0], &folderid, &location);
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
	N6510_SetSMSLocation(&sms->SMS[0], folderid, location);

	return N6510_PrivGetSMSMessage(s, sms);
}

static GSM_Error N6510_ReplyGetCalendarInfo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	return N71_65_ReplyGetCalendarInfo(msg, Data, User, &Data->Priv.N6510.LastCalendar);
}

static GSM_Error N6510_GetCalendarNote(GSM_StateMachine *s, GSM_CalendarNote *Note, bool start)
{
	return N71_65_GetCalendarNote(s,Note,start,&s->Phone.Data.Priv.N6510.LastCalendar);
}

static GSM_Error N6510_DelCalendarNote(GSM_StateMachine *s, GSM_CalendarNote *Note)
{
	return N71_65_DelCalendarNote(s, Note,&s->Phone.Data.Priv.N6510.LastCalendar);
}

static GSM_Error N6510_ReplyGetCalendarNotePos(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	return N71_65_ReplyGetCalendarNotePos(msg, Data, User,&Data->Priv.N6510.FirstCalendarPos);
}

static GSM_Error N6510_SetCalendarNote(GSM_StateMachine *s, GSM_CalendarNote *Note)
{
	return N71_65_SetCalendarNote(s, Note, &s->Phone.Data.Priv.N6510.FirstCalendarPos);
}

static GSM_Error N6510_ReplyGetIncomingNetLevel(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Network level changed to: %i\n",msg.Buffer[4]);
	return GE_NONE;
}

static GSM_Error N6510_ReplyGetNetworkLevel(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Network level received: %i\n",msg.Buffer[8]);
	*Data->NetworkLevel=((int)msg.Buffer[8]);
	return GE_NONE;
}

static GSM_Error N6510_GetNetworkLevel(GSM_StateMachine *s, int *level)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x0B, 0x00, 0x02, 0x00, 0x00, 0x00};

	s->Phone.Data.NetworkLevel=level;
	dprintf("Getting network level\n");
	return GSM_WaitFor (s, req, 9, 0x0a, 4, ID_GetNetworkLevel);
}

static GSM_Error N6510_ReplyGetBatteryLevel(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Battery level received: %i\n",msg.Buffer[9]*100/7);
	*Data->BatteryLevel=((int)(msg.Buffer[9]*100/7));
	return GE_NONE;
}

static GSM_Error N6510_GetBatteryLevel(GSM_StateMachine *s, int *level)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x0A, 0x02, 0x00};

	s->Phone.Data.BatteryLevel=level;
	dprintf("Getting battery level\n");
	return GSM_WaitFor (s, req, 6, 0x17, 4, ID_GetBatteryLevel);
}

static GSM_Error N6510_ReplyGetWAPBookmark(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	return DCT3DCT4_ReplyGetWAPBookmark (msg,Data,User,true);
}

static GSM_Error N6510_ReplyGetOperatorLogo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Operator logo received\n");
	NOKIA_DecodeNetworkCode(msg.Buffer+12,Data->Bitmap->NetworkCode);
	dprintf("Network code %s\n",Data->Bitmap->NetworkCode);
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
		req[11] = NOKIA_GetMemoryType(entry->MemoryType,N71_65_MEMORY_TYPES);
		if (req[11]==0xff) return GE_NOTSUPPORTED;

		req[12] = (entry->Location >> 8);
		req[13] = entry->Location & 0xff;

		count = count + N71_65_EncodePhonebookFrame(req+22, *entry, &blocks, true);
		req[21] = blocks;

		dprintf("Writing phonebook entry\n");
		return GSM_WaitFor (s, req, count, 0x03, 4, ID_SetMemory);
	} else {
		return N71_65_DeleteMemory(s, entry, N71_65_MEMORY_TYPES);
	}  
}

static GSM_Error N6510_ReplySetOperatorLogo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Operator logo set OK\n");
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
				PHONE_ClearBitmap(Type, reqStartup + 22);
				break;
			default:
				return GE_NOTSUPPORTED;
		}
		dprintf("Setting startup logo\n");
		return GSM_WaitFor (s, reqStartup, 22+PHONE_GetBitmapSize(Type), 0x7A, 4, ID_SetBitmap);
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
			reqOp[19] = PHONE_GetBitmapSize(Type) + 8 + 29 + 2;
			PHONE_GetBitmapWidthHeight(Type, &Width, &Height);
			reqOp[20] = Width;
			reqOp[21] = Height;
			reqOp[22] = 0x00;
			reqOp[23] = PHONE_GetBitmapSize(Type) + 29;
			reqOp[24] = 0x00;
			reqOp[25] = PHONE_GetBitmapSize(Type) + 29;
			PHONE_EncodeBitmap(Type, reqOp + 26, Bitmap);
			dprintf("Setting operator logo\n");
			return GSM_WaitFor (s, reqOp, reqOp[19]+reqOp[11]+10, 0x0A, 4, ID_SetBitmap);
		} else {
			error=N6510_GetNetworkInfo(s,&NetInfo);
			if (error != GE_NONE) return error;
			NOKIA_EncodeNetworkCode(reqOp+12, NetInfo.NetworkCode);
			dprintf("Clearing operator logo\n");
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

static GSM_Error N6510_ReplyGetRingtoneID(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	GSM_Phone_N6510Data *Priv = &Data->Priv.N6510;		

	dprintf("Ringtone ID received\n");
	Priv->RingtoneID = msg.Buffer[15];
	return GE_NONE;
}

static GSM_Error N6510_ReplySetBinRingtone(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Binary ringtone set\n");
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
		dprintf("Getting ringtone ID\n");
		error=GSM_WaitFor (s, GetIDReq, 14, 0xDB, 4, ID_SetRingtone);
		if (error != GE_NONE) return error;
		*maxlength=GSM_EncodeNokiaRTTLRingtone(*Ringtone, SetPreviewReq+11, &size);
		SetPreviewReq[0]  = Priv->RingtoneID;
		SetPreviewReq[10] = size;
		dprintf("Setting ringtone\n");
		error = s->Protocol.Functions->WriteMessage(s, SetPreviewReq, size+11, 0x00);
		if (error!=GE_NONE) return error;
		mili_sleep(1000);
		/* We have to make something (not important, what) now */
		/* no answer from phone*/
		return s->Phone.Functions->GetNetworkInfo(s,&NetInfo);
	}
	if (Ringtone->Format == RING_NOKIABINARY) {
//		dprintf("Deleting all user ringtones\n");
//		GSM_WaitFor (s, DelAllRingtoneReq, 6, 0x1F, 4, ID_SetRingtone);

		SetBinaryReq[7] = strlen(DecodeUnicodeString(Ringtone->Name));
		CopyUnicodeString(SetBinaryReq+8,Ringtone->Name);
		current = 8 + strlen(DecodeUnicodeString(Ringtone->Name))*2;
		SetBinaryReq[current++] = Ringtone->NokiaBinary.Length/256 + 1;
		SetBinaryReq[current++] = Ringtone->NokiaBinary.Length%256 + 1;
		SetBinaryReq[current++] = 0x00;
		memcpy(SetBinaryReq+current,Ringtone->NokiaBinary.Frame,Ringtone->NokiaBinary.Length);
		current += Ringtone->NokiaBinary.Length;
		dprintf("Setting binary ringtone\n");
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

	if (s->connectiontype == GCT_DLR3AT) return GE_NOTSUPPORTED;
	if (hard) return GE_NOTSUPPORTED;

	/* Going to test mode */
	error=GSM_WaitFor (s, req, 6, 0x15, 4, ID_Reset);
	if (error != GE_NONE) return error;
	for (i=0;i<6;i++) {
		GSM_GetCurrentDateTime (&Date);
		j=Date.Second;
		while (j==Date.Second) {
			mili_sleep(10);
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

static GSM_Error N6510_ReplyGetToDo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("TODO received\n");
	if (msg.Buffer[4] >= 1 && msg.Buffer[4] <= 3) {
		Data->ToDo->Priority = msg.Buffer[4];
	}
	dprintf("Priority: %i\n",msg.Buffer[4]);
	CopyUnicodeString(Data->ToDo->Text,msg.Buffer+14);
	dprintf("Text: \"%s\"\n",DecodeUnicodeString(Data->ToDo->Text));
	return GE_NONE;
}

static GSM_Error N6510_ReplyGetToDoLocations(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int			i;
	GSM_Phone_N6510Data	*Priv = &Data->Priv.N6510;

	dprintf("TODO locations received\n");
	Priv->LastToDo.Number=msg.Buffer[6]*256+msg.Buffer[7];
	dprintf("Number of Entries: %i\n",Priv->LastToDo.Number);
	dprintf("Locations: ");
	for (i=0;i<Priv->LastToDo.Number;i++) {
		Priv->LastToDo.Location[i]=msg.Buffer[12+(i*4)]*256+msg.Buffer[(i*4)+13];
		dprintf("%i ",Priv->LastToDo.Location[i]);
	}
	dprintf("\n");
	return GE_NONE;
}

static GSM_Error N6510_GetToDo(GSM_StateMachine *s, GSM_TODO *ToDo, bool refresh)
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

	if (refresh) {
		dprintf("Getting ToDo locations\n");
		error = GSM_WaitFor (s, reqLoc, 10, 0x55, 4, ID_GetToDo);
		if (error != GE_NONE) return error;
	}
	if (ToDo->Location > LastToDo->Number) return GE_INVALIDLOCATION;
	reqGet[8] = LastToDo->Location[ToDo->Location-1] / 256;
	reqGet[9] = LastToDo->Location[ToDo->Location-1] % 256;
	s->Phone.Data.ToDo = ToDo;	
	dprintf("Getting ToDo\n");
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
		dprintf("Pressing key\n");
	} else {
		req[8] = NOKIA_RELEASEPHONEKEY;
		s->Phone.Data.PressKey = false;
		dprintf("Releasing key\n");
	}
	return GSM_WaitFor (s, req, 10, 0x0c, 4, ID_PressKey);
#else
	return GE_NOTSUPPORTED;
#endif
}

static GSM_Error N6510_ReplyGetWAPSettings(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int tmp;

	switch(msg.Buffer[3]) {
	case 0x16:
		Data->WAPSettings->Number = 2;

		dprintf("WAP settings received OK\n");

		tmp = 4;

		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[0].Title,true);
		CopyUnicodeString(Data->WAPSettings->Settings[1].Title,Data->WAPSettings->Settings[0].Title);
		dprintf("Title: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].Title));

		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[0].HomePage,true);
		CopyUnicodeString(Data->WAPSettings->Settings[1].HomePage,Data->WAPSettings->Settings[0].HomePage);
		dprintf("Homepage: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].HomePage));

#ifdef DEBUG
		dprintf("Connection type: ");      
		switch (msg.Buffer[tmp]) {
			case 0x00: dprintf("temporary\n");  	break;
			case 0x01: dprintf("continuous\n"); 	break;
			default:   dprintf("unknown\n");
		}
		dprintf("Connection security: ");
		switch (msg.Buffer[tmp+1]) {
			case 0x00: dprintf("off\n");		break;
			case 0x01: dprintf("on\n");		break;
			default:   dprintf("unknown\n");
		}
		dprintf("Bearer: ");
		switch (msg.Buffer[tmp+2]) {
			case 0x01: dprintf("GSM data\n");	break;
			case 0x03: dprintf("GPRS\n");		break;
			default:   dprintf("unknown\n");
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
		dprintf("Authentication type: ");
		switch (msg.Buffer[tmp]) {
			case 0x00: dprintf("normal\n");		break;
			case 0x01: dprintf("secure\n");		break;
			default:   dprintf("unknown\n");	break;
		}
		dprintf("Data call type: ");
		switch (msg.Buffer[tmp+1]) {
			case 0x00: dprintf("analogue\n");	break;
			case 0x01: dprintf("ISDN\n");		break;
			default:   dprintf("unknown\n");	break;
		}
		dprintf("Data call speed: ");
		switch (msg.Buffer[tmp+2]) {
			case 0x00: dprintf("automatic\n"); 	break;
			case 0x01: dprintf("9600\n");	  	break;
			case 0x02: dprintf("14400\n");	  	break;
			default:   dprintf("unknown\n");	break;
		}
		dprintf("Login Type: ");
		switch (msg.Buffer[tmp+4]) {
			case 0x00: dprintf("manual\n");		break;
			case 0x01: dprintf("automatic\n");	break;
			default:   dprintf("unknown\n");	break;
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

		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[0].IPAddress,false);
		dprintf("IP address: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].IPAddress));

		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[0].DialUp,true);
		dprintf("Dial-up number: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].DialUp));

		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[0].User,true);
		dprintf("User name: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].User));

		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[0].Password,true);		
		dprintf("Password: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].Password));

		/* Here starts settings for gprs bearer */
		Data->WAPSettings->Settings[1].Bearer = WAPSETTINGS_BEARER_GPRS;
		while (msg.Buffer[tmp] != 0x03) tmp++;
		tmp += 4;

#ifdef DEBUG
		dprintf("Authentication type: ");
		switch (msg.Buffer[tmp]) {
			case 0x00: dprintf("normal\n");		break;
			case 0x01: dprintf("secure\n");		break;
			default:   dprintf("unknown\n");	break;
		}
		dprintf("GPRS connection: ");
		switch (msg.Buffer[tmp+1]) {
			case 0x00: dprintf("ALWAYS online\n");	break;
			case 0x01: dprintf("when needed\n");	break;
			default:   dprintf("unknown\n"); 	break;
		}
		dprintf("Login Type: ");
		switch (msg.Buffer[tmp+2]) {
			case 0x00: dprintf("manual\n");		break;
			case 0x01: dprintf("automatic\n");	break;
			default:   dprintf("unknown\n");	break;
		}
#endif
		Data->WAPSettings->Settings[1].IsNormalAuthentication=true;
		if (msg.Buffer[tmp]==0x01) Data->WAPSettings->Settings[1].IsNormalAuthentication=false;

		Data->WAPSettings->Settings[1].IsContinuous = true;
		if (msg.Buffer[tmp+1] == 0x01) Data->WAPSettings->Settings[1].IsContinuous = false;

		Data->WAPSettings->Settings[1].ManualLogin=false;
		if (msg.Buffer[tmp+2]==0x00) Data->WAPSettings->Settings[1].ManualLogin = true;

		tmp+=3;

		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[1].DialUp,false);
		dprintf("Access point: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[1].DialUp));

		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[1].IPAddress,true);
		dprintf("IP address: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[1].IPAddress));

		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[1].User,true);
		dprintf("User name: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[1].User));

		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[1].Password,true);
		dprintf("Password: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[1].Password));

		return GE_NONE;
	case 0x17:
		dprintf("WAP settings receiving error\n");
		switch (msg.Buffer[4]) {
		case 0x01:
			dprintf("Security error. Inside WAP settings menu\n");
			return GE_INSIDEPHONEMENU;
		case 0x02:
			dprintf("Invalid or empty\n");
			return GE_INVALIDLOCATION;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
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
	dprintf("Getting WAP settins\n");
	return GSM_WaitFor (s, req, 6, 0x3f, 4, ID_GetWAPSettings);
}

static GSM_Error N6510_ReplySetWAPSettings(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x19:
		dprintf("WAP settings cleaned\n");
		return GE_NONE;
	case 0x1a:
		dprintf("WAP settings setting status\n");
		switch (msg.Buffer[4]) {
		case 0x01:
			dprintf("Security error. Inside WAP settings menu\n");
			return GE_INSIDEPHONEMENU;
		case 0x03:
			dprintf("Invalid location\n");
			return GE_INVALIDLOCATION;
		case 0x05:
			dprintf("Written OK\n");
			return GE_NONE;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
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
		pos += NOKIA_SetUnicodeString(req + pos, settings->Settings[loc1].Title, false);

		/* Home */
		length = strlen(DecodeUnicodeString(settings->Settings[loc1].HomePage));
		if (((length + pad) % 2)) pad = 2; else pad = 0;
		pos += NOKIA_SetUnicodeString(req + pos, settings->Settings[loc1].HomePage, true);

		if (settings->Settings[loc1].IsContinuous) req[pos] = 0x01; pos++;
		if (settings->Settings[loc1].IsSecurity) req[pos] = 0x01; pos++;

		req[pos++] = 0x01; //data set
	} else if (loc2 != -1) {
		/* Name */
		length = strlen(DecodeUnicodeString(settings->Settings[loc2].Title));
		if (!(length % 2)) pad = 1;
		pos += NOKIA_SetUnicodeString(req + pos, settings->Settings[loc2].Title, false);

		/* Home */
		length = strlen(DecodeUnicodeString(settings->Settings[loc2].HomePage));
		if (((length + pad) % 2)) pad = 2; else pad = 0;
		pos += NOKIA_SetUnicodeString(req + pos, settings->Settings[loc2].HomePage, true);

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

		pos += NOKIA_SetUnicodeString(req + pos, settings->Settings[loc1].IPAddress, false);
		pos += NOKIA_SetUnicodeString(req + pos, settings->Settings[loc1].DialUp, true);
		pos += NOKIA_SetUnicodeString(req + pos, settings->Settings[loc1].User, true);
		pos += NOKIA_SetUnicodeString(req + pos, settings->Settings[loc1].Password, true);
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

		pos += NOKIA_SetUnicodeString(req + pos, settings->Settings[loc2].DialUp, false);
		pos += NOKIA_SetUnicodeString(req + pos, settings->Settings[loc2].IPAddress, true);
		pos += NOKIA_SetUnicodeString(req + pos, settings->Settings[loc2].User, true);
		pos += NOKIA_SetUnicodeString(req + pos, settings->Settings[loc2].Password, true);
	} else {
		pos += 10;
	}

	/* end of blocks ? */
	memcpy(req + pos, "\x80\x00\x00\x0c", 4);	pos += 4;

	pos += 8;

	dprintf("Setting WAP settins\n");
	return GSM_WaitFor (s, req, pos, 0x3f, 4, ID_SetWAPSettings);
}

static GSM_Error N6510_ReplyGetOriginalIMEI(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	if (msg.Buffer[7] == 0x00) {
		dprintf("No SIM card\n");
		return GE_SECURITYERROR;
	} else {
		return NOKIA_ReplyGetPhoneString(msg, Data, User);
	}
}

static GSM_Error N6510_GetOriginalIMEI(GSM_StateMachine *s, char *value)
{
	return NOKIA_GetPhoneString(s,"\x00\x07\x02\x01\x00\x01",6,0x42,value,ID_GetOriginalIMEI,14);
}

static GSM_Error N6510_ReplyDeleteAllToDo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("All TODO deleted\n");
	return GE_NONE;
}

static GSM_Error N6510_DeleteAllToDo(GSM_StateMachine *s)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x11};

	dprintf("Deleting all ToDo\n");
	return GSM_WaitFor (s, req, 4, 0x55, 4, ID_DeleteAllToDo);
}

static GSM_Error N6510_ReplyGetToDoFirstLoc(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("TODO first location received: %02x\n",msg.Buffer[9]);
	Data->ToDo->Location = msg.Buffer[9];
	return GE_NONE;
}

static GSM_Error N6510_ReplySetToDo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("TODO set OK\n");
	return GE_NONE;
}

static GSM_Error N6510_SetToDo(GSM_StateMachine *s, GSM_TODO *ToDo)
{
	GSM_Error		error;
	unsigned char 		reqLoc[] 	= {N6110_FRAME_HEADER, 0x0F};
	unsigned char 		reqSet[500] 	= {
		N6110_FRAME_HEADER, 0x01,
		0x03,		/* Priority */
		0x00,		/* Length of text */
		0x80,0x00,0x00,
		0x18};		/* Location */

	s->Phone.Data.ToDo = ToDo;

	if (ToDo->Location == 0) {
		dprintf("Getting first ToDo location\n");
		error = GSM_WaitFor (s, reqLoc, 4, 0x55, 4, ID_SetToDo);
		if (error != GE_NONE) return error;
		reqSet[9] = ToDo->Location;
	} else {
		return GE_NOTSUPPORTED;
	}
	reqSet[4] = ToDo->Priority;
	reqSet[5] = strlen(DecodeUnicodeString(ToDo->Text))+1;
	CopyUnicodeString(reqSet+10,ToDo->Text);
	reqSet[10+strlen(DecodeUnicodeString(ToDo->Text))*2] 	= 0x00;
	reqSet[10+strlen(DecodeUnicodeString(ToDo->Text))*2+1] 	= 0x00;
	dprintf("Setting ToDo\n");
	return GSM_WaitFor (s, reqSet, 12+strlen(DecodeUnicodeString(ToDo->Text))*2, 0x55, 4, ID_SetToDo);
}

static GSM_Error N6510_SetWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error 	error;
	int 		count;
	int		location;
	unsigned char req[600] = { N6110_FRAME_HEADER, 0x09 };

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

	dprintf("Setting WAP bookmark\n");
	return GSM_WaitFor (s, req, count, 0x3f, 4, ID_SetWAPBookmark);
}

static GSM_Error N6510_ReplyGetSMSStatus(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Max. in phone memory   : %i\n",msg.Buffer[10]*256+msg.Buffer[11]);
	dprintf("Used in phone memory   : %i\n",msg.Buffer[12]*256+msg.Buffer[13]);
	dprintf("Unread in phone memory : %i\n",msg.Buffer[14]*256+msg.Buffer[15]);
	dprintf("Max. in SIM            : %i\n",msg.Buffer[22]*256+msg.Buffer[23]);
	dprintf("Used in SIM            : %i\n",msg.Buffer[24]*256+msg.Buffer[25]);
	dprintf("Unread in SIM          : %i\n",msg.Buffer[26]*256+msg.Buffer[27]);
	Data->SMSStatus->PhoneSize	= msg.Buffer[10]*256+msg.Buffer[11];
	Data->SMSStatus->PhoneUsed	= msg.Buffer[12]*256+msg.Buffer[13];
	Data->SMSStatus->PhoneUnRead 	= msg.Buffer[14]*256+msg.Buffer[15];
	Data->SMSStatus->SIMSize	= msg.Buffer[22]*256+msg.Buffer[23];
	Data->SMSStatus->SIMUsed 	= msg.Buffer[24]*256+msg.Buffer[25];
	Data->SMSStatus->SIMUnRead 	= msg.Buffer[26]*256+msg.Buffer[27];
	return GE_NONE;
}

static GSM_Error N6510_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	GSM_Error 		error;
	GSM_Phone_N6510Data	*Priv = &s->Phone.Data.Priv.N6510;
	unsigned char req[] = {N6110_FRAME_HEADER, 0x08, 0x00, 0x00};

	s->Phone.Data.SMSStatus=status;
	dprintf("Getting SMS status\n");
	error = GSM_WaitFor (s, req, 6, 0x14, 3, ID_GetSMSStatus);
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

static GSM_Error N6510_ReplyDeleteSMSMessage(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
		case 0x05:
			dprintf("SMS deleted OK\n");
			return GE_NONE;
		case 0x06:
			switch (msg.Buffer[4]) {
				case 0x02:
					dprintf("Invalid location\n");
					return GE_INVALIDLOCATION;
				default:
					dprintf("Unknown error: %02x\n",msg.Buffer[4]);
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

	N6510_GetSMSLocation(sms, &folderid, &location);

	switch (folderid) {
		case 0x01: req[5] = 0x02; 			 break; /* INBOX SIM 	*/
		case 0x02: req[5] = 0x03; 			 break; /* OUTBOX SIM 	*/
		default	 : req[5] = folderid - 1; req[4] = 0x02; break; /* ME folders	*/
	}
	req[6]=location / 256;
	req[7]=location;

	dprintf("Deleting sms\n");
	return GSM_WaitFor (s, req, 10, 0x14, 4, ID_DeleteSMSMessage);
}

static GSM_Error N6510_ReplySendSMSMessage(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[8]) {
		case 0x00:
			dprintf("SMS sent OK\n");
			if (User->SendSMSStatus!=NULL) User->SendSMSStatus(Data->Device,0);
			return GE_NONE;
		default:
			dprintf("SMS not sent OK, error code probably %i\n",msg.Buffer[8]);
			if (User->SendSMSStatus!=NULL) User->SendSMSStatus(Data->Device,msg.Buffer[8]);
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

	dprintf("Sending sms\n");
	return s->Protocol.Functions->WriteMessage(s, req, length + 9, 0x02);
}

static GSM_Error N6510_ReplyGetSecurityStatus(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Security Code status received: ");
	switch (msg.Buffer[4]) {
	case 0x01 : dprintf("waiting for Security Code.\n");	*Data->SecurityStatus = GSCT_SecurityCode;	break;
	case 0x07 :
	case 0x02 : dprintf("waiting for PIN.\n");		*Data->SecurityStatus = GSCT_Pin;		break;
	case 0x03 : dprintf("waiting for PUK.\n");		*Data->SecurityStatus = GSCT_Puk;		break;
	case 0x05 : dprintf("PIN ok, SIM ok\n");		*Data->SecurityStatus = GSCT_None;		break;
	case 0x06 : dprintf("No input status\n"); 		*Data->SecurityStatus = GSCT_None;		break;
	case 0x16 : dprintf("No SIM!\n");			*Data->SecurityStatus = GSCT_None;		break;
	case 0x1A : dprintf("SIM rejected!\n");			*Data->SecurityStatus = GSCT_None;		break;
	default   : dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
		    return GE_UNKNOWNRESPONSE;
	}
	return GE_NONE;
}

static GSM_Error N6510_GetSecurityStatus(GSM_StateMachine *s, GSM_SecurityCodeType *Status)
{
	unsigned char req[5] = {N6110_FRAME_HEADER, 0x11, 0x00};

	s->Phone.Data.SecurityStatus=Status;
	dprintf("Getting security code status\n");
	return GSM_WaitFor (s, req, 5, 0x08, 2, ID_GetSecurityStatus);
}

static GSM_Error N6510_ReplyEnterSecurityCode(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x08:
		dprintf("Security code OK\n");
		return GE_NONE;
	case 0x09:
		switch (msg.Buffer[4]) {
		case 0x06:
			dprintf("Wrong PIN\n");
			return GE_SECURITYERROR;
		case 0x09:
			dprintf("Wrong PUK\n");
			return GE_SECURITYERROR;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
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

	dprintf("Entering security code\n");
	return GSM_WaitFor (s, req, 6+len, 0x08, 4, ID_EnterSecurityCode);
}

static GSM_Error N6510_ReplySaveSMSMessage(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	unsigned char folder;

	switch (msg.Buffer[3]) {
	case 0x01:
		switch (msg.Buffer[4]) {
			case 0x00:
				dprintf("Done OK\n");
				dprintf("Folder info: %i %i\n",msg.Buffer[8],msg.Buffer[5]);
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
				N6510_SetSMSLocation(Data->SaveSMSMessage,folder,msg.Buffer[6]*256+msg.Buffer[7]);
				dprintf("Saved in folder %i at location %i\n",folder, msg.Buffer[6]*256+msg.Buffer[7]);
				Data->SaveSMSMessage->Folder = folder;
				return GE_NONE;
			case 0x02:
				printf("Incorrect location\n");
				return GE_INVALIDLOCATION;
			case 0x05:
				printf("Incorrect folder\n");
				return GE_INVALIDLOCATION;
			default:
				dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
				return GE_UNKNOWNRESPONSE;
		}
	case 0x17:
		dprintf("SMS name changed\n");
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

	N6510_GetSMSLocation(sms, &folderid, &location);
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
	dprintf("Saving sms\n");
	error=GSM_WaitFor (s, req, length+9, 0x14, 4, ID_SaveSMSMessage);
	if (error == GE_NONE && strlen(DecodeUnicodeString(sms->Name))!=0) {
		folder = sms->Folder;
		sms->Folder = 0;
		N6510_GetSMSLocation(sms, &folderid, &location);
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

static GSM_Error N6510_ReplyGetDateTime(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Date & time received\n");
	if (msg.Buffer[4]==0x01) {
		NOKIA_DecodeDateTime(msg.Buffer+10, Data->DateTime);
		return GE_NONE;
	}
	dprintf("Not set in phone\n");
	return GE_EMPTY;
}

static GSM_Error N6510_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x0A, 0x00, 0x00};

	s->Phone.Data.DateTime=date_time;
	dprintf("Getting date & time\n");
	return GSM_WaitFor (s, req, 6, 0x19, 4, ID_GetDateTime);
}

static GSM_Error N6510_ReplySetDateTime(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Date & time set\n");
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

	NOKIA_EncodeDateTime(req+10, date_time);
	req[16] = date_time->Second;
	dprintf("Setting date & time\n");
	return GSM_WaitFor (s, req, 18, 0x19, 4, ID_SetDateTime);
}

static GSM_Error N6510_ReplyGetManufactureMonth(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	if (msg.Buffer[7] == 0x00) {
		dprintf("No SIM card\n");
		return GE_SECURITYERROR;
	} else {
		sprintf(Data->PhoneString,"%02i/%04i",msg.Buffer[13],msg.Buffer[14]*256+msg.Buffer[15]);
	        return GE_NONE;
	}
}

static GSM_Error N6510_GetManufactureMonth(GSM_StateMachine *s, char *value)
{
	unsigned char req[6] = {0x00, 0x05, 0x02, 0x01, 0x00, 0x02};

	s->Phone.Data.PhoneString=value;
	dprintf("Getting manufacture month\n");
	return GSM_WaitFor (s, req, 6, 0x42, 2, ID_GetManufactureMonth);
}                                       

static GSM_Error N6510_ReplyGetAlarm(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch(msg.Buffer[3]) {
	case 0x1A:
		dprintf("   Alarm: %02d:%02d\n", msg.Buffer[14], msg.Buffer[15]);
		Data->Alarm->Hour	= msg.Buffer[14];
		Data->Alarm->Minute	= msg.Buffer[15];
		Data->Alarm->Second	= 0;
		return GE_NONE;
	case 0x20:
		dprintf("Alarm state received\n");
		if (msg.Buffer[37] == 0x01) {
			dprintf("   Not set in phone\n");
			return GE_EMPTY;
		}
		dprintf("Enabled\n");
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
	dprintf("Getting alarm state\n");
	error = GSM_WaitFor (s, StateReq, 6, 0x19, 4, ID_GetAlarm);
	if (error != GE_NONE) return error;

	dprintf("Getting alarm\n");
	return GSM_WaitFor (s, GetReq, 6, 0x19, 4, ID_GetAlarm);
}

static GSM_Error N6510_ReplySetAlarm(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Alarm set\n");
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

	dprintf("Setting alarm\n");
	return GSM_WaitFor (s, req, 19, 0x19, 4, ID_SetAlarm);
}

static GSM_Error N6510_ReplyGetRingtonesInfo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int tmp,i;

	dprintf("Ringtones info received\n");
	memset(Data->RingtonesInfo,0,sizeof(GSM_AllRingtonesInfo));
	Data->RingtonesInfo->Number = msg.Buffer[4] * 256 + msg.Buffer[5];
	tmp = 6;
	for (i=0;i<Data->RingtonesInfo->Number;i++) {			
		Data->RingtonesInfo->Ringtone[i].ID = msg.Buffer[tmp+2] * 256 + msg.Buffer[tmp+3];
		memcpy(Data->RingtonesInfo->Ringtone[i].Name,msg.Buffer+tmp+8,(msg.Buffer[tmp+6]*256+msg.Buffer[tmp+7])*2);
		dprintf("%i. \"%s\"\n",Data->RingtonesInfo->Ringtone[i].ID,DecodeUnicodeString(Data->RingtonesInfo->Ringtone[i].Name));
		tmp = tmp + (msg.Buffer[tmp]*256+msg.Buffer[tmp+1]);
	}
	return GE_NONE;
}

static GSM_Error N6510_PrivGetRingtonesInfo(GSM_StateMachine *s, GSM_AllRingtonesInfo *Info, bool AllRingtones)
{
	unsigned char UserReq[8] = {N7110_FRAME_HEADER, 0x07, 0x00, 0x00, 0x00, 0x02};
	unsigned char All_Req[9] = {N7110_FRAME_HEADER, 0x07, 0x00, 0x00, 0xFE, 0x00, 0x7D};

	s->Phone.Data.RingtonesInfo=Info;
	dprintf("Getting binary ringtones ID\n");
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

static GSM_Error N6510_ReplyGetRingtone(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int tmp,i;

	dprintf("Ringtone received\n");
	memcpy(Data->Ringtone->Name,msg.Buffer+8,msg.Buffer[7]*2);
	Data->Ringtone->Name[msg.Buffer[7]*2]=0;
	Data->Ringtone->Name[msg.Buffer[7]*2+1]=0;
	dprintf("Name \"%s\"\n",DecodeUnicodeString(Data->Ringtone->Name));
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
		dprintf("Getting binary ringtone\n");
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
		dprintf("Enabling sound - part 1\n");
		error=GSM_WaitFor (s, reqStart, 6, 0x0b, 4, ID_PlayTone);
		if (error!=GE_NONE) return error;
		dprintf("Enabling sound - part 2 (disabling sound command)\n");
		error=GSM_WaitFor (s, reqOff, 18, 0x0b, 4, ID_PlayTone);
		if (error!=GE_NONE) return error;
	}

	/* For Herz==255*255 we have silent */  
	if (Herz!=255*255) {
		reqPlay[23] = Herz%256;
		reqPlay[22] = Herz/256;
		reqPlay[31] = Volume;
		dprintf("Playing sound\n");
		return GSM_WaitFor (s, reqPlay, 34, 0x0b, 4, ID_PlayTone);
	} else {
		reqPlay[23] = 0;
		reqPlay[22] = 0;
		reqPlay[31] = 0;
		dprintf("Playing silent sound\n");
		return GSM_WaitFor (s, reqPlay, 34, 0x0b, 4, ID_PlayTone);

//		dprintf("Disabling sound - part 1\n");
//		error=GSM_WaitFor (s, reqOff, 18, 0x0b, 4, ID_PlayTone);
//		if (error!=GE_NONE) return error;		
//		dprintf("Disabling sound - part 2\n");
//		return GSM_WaitFor (s, reqOff2, 18, 0x0b, 4, ID_PlayTone);
	}
}

static GSM_Error N6510_ReplyGetPPM(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	Data->PhoneString[0] = msg.Buffer[50];
	Data->PhoneString[1] = 0x00;	
	dprintf("Received PPM %s\n",Data->PhoneString);
	return GE_NONE;
}

static GSM_Error N6510_GetPPM(GSM_StateMachine *s,char *value)
{
	unsigned char req[6] = {N6110_FRAME_HEADER, 0x07, 0x01, 0xff};

	s->Phone.Data.PhoneString=value;
	dprintf("Getting PPM\n");
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

	dprintf("Getting speed dial\n");
	error=N6510_GetMemory(s,&pbk);
	switch (error) {
	case GE_NOTSUPPORTED:
		dprintf("No speed dials set in phone\n");
		return GE_EMPTY;
	case GE_NONE:
		if (SpeedDial->MemoryLocation == 0) {
			dprintf("Speed dial not assigned or error in firmware\n");
			return GE_EMPTY;
		}
		return GE_NONE;
	default:
		return error;
	}
}

static GSM_Error N6510_ReplyGetProfile(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	unsigned char 	*blockstart;
	int 		i,j;
	                 
	blockstart = msg.Buffer + 7;
	for (i = 0; i < 11; i++) {
		dprintf("Profile feature %02x ",blockstart[1]);
#ifdef DEBUG
		if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) DumpMessage(di.df, blockstart, blockstart[0]);
#endif

		switch (blockstart[1]) {
		case 0x03:
			dprintf("Ringtone ID\n");
			Data->Profile->FeatureID	[Data->Profile->FeaturesNumber] = Profile_RingtoneID;
			Data->Profile->FeatureValue	[Data->Profile->FeaturesNumber] = blockstart[7];
			Data->Profile->FeaturesNumber++;
			break;
		case 0x05:	/* SMS tone */
			j = Data->Profile->FeaturesNumber;
			NOKIA_FindFeatureValue(Profile71_65,blockstart[1],blockstart[7],Data,false);
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
			NOKIA_FindFeatureValue(Profile71_65,blockstart[1],blockstart[7],Data,true);
			break;
		case 0x0c :
			CopyUnicodeString(Data->Profile->Name,blockstart + 7);
			dprintf("profile Name: \"%s\"\n", DecodeUnicodeString(Data->Profile->Name));
			Data->Profile->DefaultName = false;
			break;
		default:
			NOKIA_FindFeatureValue(Profile71_65,blockstart[1],blockstart[7],Data,false);
		}
		blockstart = blockstart + blockstart[0];
	}
	return GE_NONE;
}

static GSM_Error N6510_GetProfile(GSM_StateMachine *s, GSM_Profile *Profile)
{
	unsigned char 	req[150] = {N6110_FRAME_HEADER, 0x01, 0x01, 0x0C, 0x01};
	int 		i, length = 7;

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
	dprintf("Getting profile\n");
	return GSM_WaitFor (s, req, length, 0x39, 4, ID_GetProfile);
}

static GSM_Error N6510_ReplySetProfile(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	unsigned char 	*blockstart;
	int 		i;

	dprintf("Response to profile writing received!\n");

	blockstart = msg.Buffer + 6;
	for (i = 0; i < msg.Buffer[5]; i++) {
		switch (blockstart[2]) {
		case 0x00:
			if (msg.Buffer[4] == 0x00) 
				dprintf("keypad tone level successfully set!\n");
			else
				dprintf("failed to set keypad tone level! error: %i\n", msg.Buffer[4]);
			break;
		case 0x02:
			if (msg.Buffer[4] == 0x00) 
				dprintf("call alert successfully set!\n");
			else
				dprintf("failed to set call alert! error: %i\n", msg.Buffer[4]);
			break;
		case 0x03:
			if (msg.Buffer[4] == 0x00) 
				dprintf("ringtone successfully set!\n");
			else
			dprintf("failed to set ringtone! error: %i\n", msg.Buffer[4]);
			break;
		case 0x04:
			if (msg.Buffer[4] == 0x00) 
				dprintf("ringtone volume successfully set!\n");
			else
				dprintf("failed to set ringtone volume! error: %i\n", msg.Buffer[4]);
			break;
		case 0x05:
			if (msg.Buffer[4] == 0x00) 
				dprintf("msg.Buffer tone successfully set!\n");
			else
				dprintf("failed to set msg.Buffer tone! error: %i\n", msg.Buffer[4]);
			break;
		case 0x06:
			if (msg.Buffer[4] == 0x00) 
				dprintf("vibration successfully set!\n");
			else
				dprintf("failed to set vibration! error: %i\n", msg.Buffer[4]);
			break;
		case 0x07:
			if (msg.Buffer[4] == 0x00) 
				dprintf("warning tone level successfully set!\n");
			else
				dprintf("failed to set warning tone level! error: %i\n", msg.Buffer[4]);
			break;
		case 0x08:
			if (msg.Buffer[4] == 0x00) 
				dprintf("caller groups successfully set!\n");
			else
				dprintf("failed to set caller groups! error: %i\n", msg.Buffer[4]);
			break;
		case 0x0c:
			if (msg.Buffer[4] == 0x00) 
				dprintf("name successfully set!\n");
			else
				dprintf("failed to set name! error: %i\n", msg.Buffer[4]);
			break;
		default:
			dprintf("Unknown profile subblock type %02x!\n", blockstart[1]);
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

	dprintf("Setting profile\n");
	return GSM_WaitFor (s, req, length, 0x39, 4, ID_SetProfile);
}

static GSM_Error N6510_ReplyIncomingSMS(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	GSM_SMSMessage sms;

#ifdef DEBUG
	dprintf("SMS message received\n");
	N6510_DecodeSMSFrame(&sms, msg.Buffer+10);
#endif

	if (Data->EnableIncomingSMS && User->IncomingSMS!=NULL) {
		sms.State 	 = GSM_UnRead;
		sms.InboxFolder  = true;

		N6510_DecodeSMSFrame(&sms, msg.Buffer+10);

		User->IncomingSMS(Data->Device,sms);
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

	dprintf("Making voice call\n");
	return GSM_WaitFor (s, req, pos, 0x01, 4, ID_DialVoice);
}

static GSM_Error N6510_GetNextCalendarNote(GSM_StateMachine *s, GSM_CalendarNote *Note, bool start)
{
	return N71_65_GetNextCalendarNote(s,Note,start,&s->Phone.Data.Priv.N6510.LastCalendarYear,&s->Phone.Data.Priv.N6510.LastCalendarPos);
}

static GSM_Reply_Function N6510ReplyFunctions[] = {
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x02,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x03,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x04,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x05,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x07,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x09,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x0A,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x0C,ID_DialVoice		},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x0C,ID_IncomingFrame	},
	{N71_65_ReplyCallInfo,		"\x01",0x03,0x53,ID_IncomingFrame	},

	{N6510_ReplySendSMSMessage,	"\x02",0x03,0x03,ID_IncomingFrame	},
	{N6510_ReplyIncomingSMS,	"\x02",0x03,0x04,ID_IncomingFrame	},
	{N6510_ReplySetSMSC,		"\x02",0x03,0x13,ID_SetSMSC		},
	{N6510_ReplyGetSMSC,		"\x02",0x03,0x15,ID_GetSMSC		},

	{N6510_ReplyGetMemoryStatus,	"\x03",0x03,0x04,ID_GetMemoryStatus	},
	{N6510_ReplyGetMemory,		"\x03",0x03,0x08,ID_GetMemory		},
	{N71_65_ReplyDeleteMemory,	"\x03",0x03,0x10,ID_SetMemory		},
	{N71_65_ReplyWritePhonebook,	"\x03",0x03,0x0C,ID_SetBitmap		},
	{N71_65_ReplyWritePhonebook,	"\x03",0x03,0x0C,ID_SetMemory		},

	{N6510_ReplyEnterSecurityCode,	"\x08",0x03,0x08,ID_EnterSecurityCode	},
	{N6510_ReplyEnterSecurityCode,	"\x08",0x03,0x09,ID_EnterSecurityCode	},
	{N6510_ReplyGetSecurityStatus,	"\x08",0x03,0x12,ID_GetSecurityStatus	},

	{N6510_ReplyGetNetworkInfo,	"\x0A",0x03,0x01,ID_GetNetworkInfo	},
	{N6510_ReplyGetNetworkInfo,	"\x0A",0x03,0x01,ID_IncomingFrame	},
	{N6510_ReplyGetNetworkLevel,	"\x0A",0x03,0x0C,ID_GetNetworkLevel	},
	{N6510_ReplyGetIncomingNetLevel,"\x0A",0x03,0x1E,ID_IncomingFrame	},
	{N6510_ReplyGetOperatorLogo,	"\x0A",0x03,0x24,ID_GetBitmap		},
	{N6510_ReplySetOperatorLogo,	"\x0A",0x03,0x26,ID_SetBitmap		},

	{NONEFUNCTION,			"\x0B",0x03,0x01,ID_PlayTone		},
	{NONEFUNCTION,			"\x0B",0x03,0x15,ID_PlayTone		},
	{NONEFUNCTION,			"\x0B",0x03,0x16,ID_PlayTone		},

	{N71_65_ReplySetCalendarNote,	"\x13",0x03,0x02,ID_SetCalendarNote	},
	{N71_65_ReplySetCalendarNote,	"\x13",0x03,0x04,ID_SetCalendarNote	},
	{N71_65_ReplySetCalendarNote,	"\x13",0x03,0x06,ID_SetCalendarNote	},
	{N71_65_ReplySetCalendarNote,	"\x13",0x03,0x08,ID_SetCalendarNote	},
	{N71_65_ReplyDeleteCalendarNote,"\x13",0x03,0x0C,ID_DeleteCalendarNote	},
	{N71_65_ReplyGetCalendarNote,	"\x13",0x03,0x1A,ID_GetCalendarNote	},
	{N6510_ReplyGetCalendarNotePos,	"\x13",0x03,0x32,ID_GetCalendarNotePos	},
	{N6510_ReplyGetCalendarInfo,	"\x13",0x03,0x3B,ID_GetCalendarNotesInfo},
	{N71_65_ReplyGetNextNote,	"\x13",0x03,0x3F,ID_GetCalendarNote	},

	{N6510_ReplySaveSMSMessage,	"\x14",0x03,0x01,ID_SaveSMSMessage	},
	{N6510_ReplyGetSMSMessage,	"\x14",0x03,0x03,ID_GetSMSMessage	},
	{N6510_ReplyDeleteSMSMessage,	"\x14",0x03,0x05,ID_DeleteSMSMessage	},
	{N6510_ReplyDeleteSMSMessage,	"\x14",0x03,0x06,ID_DeleteSMSMessage	},
	{N6510_ReplyGetSMSStatus,	"\x14",0x03,0x09,ID_GetSMSStatus	},
	{N6510_ReplyGetSMSFolderStatus,	"\x14",0x03,0x0d,ID_GetSMSFolderStatus	},
	{N6510_ReplyGetSMSMessage,	"\x14",0x03,0x0f,ID_GetSMSMessage	},
	{N6510_ReplyGetSMSFolders,	"\x14",0x03,0x13,ID_GetSMSFolders	},
	{N6510_ReplySaveSMSMessage,	"\x14",0x03,0x17,ID_SaveSMSMessage	},

	{NONEFUNCTION,			"\x15",0x01,0x31,ID_Reset		},

	{N6510_ReplyGetBatteryLevel,	"\x17",0x03,0x0B,ID_GetBatteryLevel	},

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
	"3510|6310|6310i|6510|8310|8910",
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
	N6510_GetBatteryLevel,
	N6510_GetNetworkLevel,
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
	N6510_GetCalendarNote,
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
	N6510_DelCalendarNote,
	N6510_SetCalendarNote,
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
	NOTSUPPORTED,		/*	SendDTMF		*/
	NOTSUPPORTED,		/*	GetDisplayStatus	*/
	NOTIMPLEMENTED,		/*	SetAutoNetworkLogin	*/
	N6510_SetProfile,
	NOTSUPPORTED,		/*	GetSIMIMSI		*/
	NONEFUNCTION,		/*	SetIncomingCall		*/
    	N6510_GetNextCalendarNote
};

#endif
