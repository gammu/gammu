/* (c) 2006 by Marcin Wiacek
 * (C) 2007 Matthias Lechner <matthias@lmme.de> */

#include <string.h>
#include <time.h>

#include <gammu-config.h>

#include "../../gsmcomon.h"
#include "../../gsmphones.h"
#include "../../misc/coding/coding.h"
#include "../../service/gsmmisc.h"
#include "../nokia/nfunc.h"
#include "../pfunc.h"

#ifdef GSM_ENABLE_GNAPGEN

unsigned char GNAPGEN_MEMORY_TYPES[] = {
	MEM_SM,		 0x01,
	MEM_ON,		 0x03,
	MEM_DC,		 0x05,
	MEM_RC,		 0x06,
	MEM_MC,		 0x07,
	  0x00,		 0x00
};

GSM_Error GNAPGEN_ReplyGetSMSFolderStatus(GSM_Protocol_Message msg, GSM_StateMachine *s) {
	GSM_Phone_GNAPGENData           *Priv = &s->Phone.Data.Priv.GNAPGEN;
	int i;
	int pos = 10;

	if( msg.Buffer[3] == 17){
		smprintf(s, "Invalid memory type");
		return ERR_UNKNOWN;
	}

	Priv->SMSCount = msg.Buffer[8] * 256 + msg.Buffer[9];
	smprintf(s, "SMS count: %d\n", Priv->SMSCount );

	for( i=0; i<Priv->SMSCount; i++ ) {
		smprintf(s, "Entry id %d: %d\n", i, msg.Buffer[pos+1] * 256 * 256 + msg.Buffer[pos+2]*256+msg.Buffer[pos+3]);
		Priv->SMSIDs[i].byte1 = msg.Buffer[pos];
		Priv->SMSIDs[i].byte2 = msg.Buffer[pos+1];
		Priv->SMSIDs[i].byte3 = msg.Buffer[pos+2];
		Priv->SMSIDs[i].byte4 = msg.Buffer[pos+3];
		pos += 4;
	}

	return ERR_NONE;
}

static GSM_Error GNAPGEN_PrivGetSMSFolderStatus(GSM_StateMachine *s, int folderid)
{
	unsigned char req[] = {0, 3,
		0,0x0c};		/* folderID c,d,e,f...*/

		req[3] = folderid;

		return GSM_WaitFor (s, req, 4, 6, 4, ID_GetSMSFolderStatus);
}

static GSM_Error GSM_DecodeSMSDateTime(GSM_DateTime *DT, unsigned char *req)
{
	DT->Year    = DecodeWithBCDAlphabet(req[0]);
	if (DT->Year<90) DT->Year=DT->Year+2000; else DT->Year=DT->Year+1990;
	DT->Month   = DecodeWithBCDAlphabet(req[1]);
	DT->Day     = DecodeWithBCDAlphabet(req[2]);
	DT->Hour    = DecodeWithBCDAlphabet(req[3]);
	DT->Minute  = DecodeWithBCDAlphabet(req[4]);
	DT->Second  = DecodeWithBCDAlphabet(req[5]);

	/* Base for timezone is GMT. It's in quarters */
	DT->Timezone=(10*(req[6]&0x07)+(req[6]>>4))*3600/4;

	if (req[6]&0x08) DT->Timezone = -DT->Timezone;

	dbgprintf("Decoding date & time: ");
	dbgprintf("%s %4d/%02d/%02d ", DayOfWeek(DT->Year, DT->Month, DT->Day),
		  DT->Year, DT->Month, DT->Day);
	dbgprintf("%02d:%02d:%02d%+03i%02i\n", DT->Hour, DT->Minute, DT->Second,
		DT->Timezone / 3600, abs((DT->Timezone % 3600) / 60));

	return ERR_NONE;
}

GSM_Error GNAPGEN_PrivSetSMSLayout( GSM_SMSMessage *sms, unsigned char *buffer, GSM_SMSMessageLayout *Layout ) {
	int position = 0;
	/*  setting sms layout */
	*Layout = PHONE_SMSDeliver;
	Layout->SMSCNumber = 0;

	/*  the pdu type is always behind the smsc number */
	if( (buffer[0] % 2) == 0 )
		position = (buffer[0] / 2) + 1;
	else
		position = ((buffer[0] +1 ) / 2) + 1;

	position++;
	Layout->firstbyte = position;

	/*  determine whether the sms was received or sent */
	switch( buffer[position] & 1 ) {
		/*  SMS-DELIVER (incoming message, received) */
		case 0:
			dbgprintf("Message type: SMS-DELIVER\n");
			sms->PDU = SMS_Deliver;

			position++;
			Layout->Number = position;

			if( (buffer[position] % 2) == 0 )
				position += (buffer[position] / 2) + 1;
			else
				position += ((buffer[position] + 1 ) / 2) + 1;

			position++;
			Layout->TPPID = position;

			position++;
			Layout->TPDCS = position;

			position++;
			Layout->DateTime = position;
			Layout->SMSCTime = position;

			position += 7;
			Layout->TPUDL = position;

			position++;
			Layout->Text = position;

			Layout->TPStatus = 255;
			Layout->TPVP = 255;
			Layout->TPMR = 255;

			break;

		/*  SMS-SUBMIT (outgoing message, to be sent) */
		case 1:
			dbgprintf("Message type: SMS-SUBMIT\n");
			sms->PDU = SMS_Submit;

			position++;
			Layout->TPMR = position;

			position++;
			Layout->Number = position;

			if( (buffer[position] % 2) == 0 )
				position += (buffer[position] / 2) + 1;
			else
				position += ((buffer[position] + 1 ) / 2) + 1;

			position++;
			Layout->TPPID = position;

			position++;
			Layout->TPDCS = position;

			position++;
			/* the validity period field length depends on the VP flag set in the pdu-type */
			if( buffer[position] & 0x16 ) {
				/*  VP is integer represented */
				Layout->TPVP = position;
			} else if ( buffer[position] & 0x08 ) {
				position += 6;
				Layout->TPVP = position;
				/*  VP is semi-octet represented */
			}

			position++;
			Layout->TPUDL = position;

			position++;
			Layout->Text = position;

			Layout->TPStatus = 255;
			Layout->DateTime = 255;
			Layout->SMSCTime = 255;

			break;

		default:
			dbgprintf("Unknown message type: (PDU) %d\n", buffer[Layout->firstbyte] );
			return ERR_UNKNOWN;
	}

	return ERR_NONE;
}

GSM_Error GNAPGEN_DecodeSMSFrame(GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout *Layout)
{
	GSM_DateTime	zerodt = {0,0,0,0,0,0,0};
#ifdef DEBUG
	if (Layout->firstbyte == 255) {
		dbgprintf("ERROR: firstbyte in SMS layout not set\n");
		return ERR_UNKNOWN;
	}

	if (Layout->TPDCS     != 255) dbgprintf("TPDCS     : %02x %i\n",buffer[Layout->TPDCS]    ,buffer[Layout->TPDCS]);
	if (Layout->TPMR      != 255) dbgprintf("TPMR      : %02x %i\n",buffer[Layout->TPMR]     ,buffer[Layout->TPMR]);
	if (Layout->TPPID     != 255) dbgprintf("TPPID     : %02x %i\n",buffer[Layout->TPPID]    ,buffer[Layout->TPPID]);
	if (Layout->TPUDL     != 255) dbgprintf("TPUDL     : %02x %i\n",buffer[Layout->TPUDL]    ,buffer[Layout->TPUDL]);
	if (Layout->firstbyte != 255) dbgprintf("FirstByte : %02x %i\n",buffer[Layout->firstbyte],buffer[Layout->firstbyte]);
	if (Layout->Text      != 255) dbgprintf("Text      : %02x %i\n",buffer[Layout->Text]     ,buffer[Layout->Text]);
#endif

	SMS->UDH.Type 			= UDH_NoUDH;
	SMS->Coding 			= SMS_Coding_8bit;
	SMS->Length			= 0;
	SMS->SMSC.Location		= 0;
	SMS->SMSC.DefaultNumber[0]	= 0;
	SMS->SMSC.DefaultNumber[1]	= 0;
	SMS->SMSC.Number[0]		= 0;
	SMS->SMSC.Number[1]		= 0;
	SMS->SMSC.Name[0]		= 0;
	SMS->SMSC.Name[1]		= 0;
	SMS->SMSC.Validity.Format	= SMS_Validity_NotAvailable;
	SMS->SMSC.Format		= SMS_FORMAT_Text;
	SMS->Number[0]			= 0;
	SMS->Number[1]			= 0;
	SMS->OtherNumbersNum		= 0;
	SMS->Name[0]			= 0;
	SMS->Name[1]			= 0;
	SMS->ReplyViaSameSMSC		= false;
	if (Layout->SMSCNumber!=255) {
		GSM_UnpackSemiOctetNumber(SMS->SMSC.Number,buffer+Layout->SMSCNumber,true);
		dbgprintf("SMS center number : \"%s\"\n",DecodeUnicodeString(SMS->SMSC.Number));
	}
	if ((buffer[Layout->firstbyte] & 0x80)!=0) SMS->ReplyViaSameSMSC=true;
#ifdef DEBUG
	if (SMS->ReplyViaSameSMSC) dbgprintf("SMS centre set for reply\n");
#endif
	if (Layout->Number!=255) {
		GSM_UnpackSemiOctetNumber(SMS->Number,buffer+Layout->Number,true);
		dbgprintf("Remote number : \"%s\"\n",DecodeUnicodeString(SMS->Number));
	}
	if (Layout->Text != 255 && Layout->TPDCS!=255 && Layout->TPUDL!=255) {
		SMS->Coding = GSM_GetMessageCoding(buffer[Layout->TPDCS]);
		GSM_DecodeSMSFrameText(SMS, buffer, *Layout);
	}
	if (Layout->DateTime != 255) {
		GSM_DecodeSMSDateTime(&SMS->DateTime,buffer+(Layout->DateTime));
	} else {
		SMS->DateTime = zerodt;
	}


	if (Layout->SMSCTime != 255 && Layout->TPStatus != 255) {
		/* GSM 03.40 section 9.2.3.11 (TP-Service-Centre-Time-Stamp) */
		dbgprintf("SMSC response date: ");
		GSM_DecodeSMSDateTime(&SMS->SMSCTime, buffer+(Layout->SMSCTime));
		GSM_DecodeSMSFrameStatusReportData(SMS,buffer,*Layout);
		GSM_DecodeSMSDateTime(&SMS->DateTime, buffer+(Layout->SMSCTime));
	} else {
		SMS->SMSCTime = zerodt;
	}
	SMS->Class = -1;
	if (Layout->TPDCS != 255) {
		/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme) and GSM 03.38 section 4 */
		if ((buffer[Layout->TPDCS] & 0xD0) == 0x10) {
			/* bits 7..4 set to 00x1 */
			if ((buffer[Layout->TPDCS] & 0xC) == 0xC) {
				dbgprintf("WARNING: reserved alphabet value in TPDCS\n");
			} else {
				SMS->Class = (buffer[Layout->TPDCS] & 3);
			}
		} else if ((buffer[Layout->TPDCS] & 0xF0) == 0xF0) {
			/* bits 7..4 set to 1111 */
			if ((buffer[Layout->TPDCS] & 8) == 8) {
				dbgprintf("WARNING: set reserved bit 3 in TPDCS\n");
			} else {
				SMS->Class = (buffer[Layout->TPDCS] & 3);
			}
		}
		dbgprintf("SMS class: %i\n",SMS->Class);
	}

	SMS->MessageReference = 0;
	if (Layout->TPMR != 255) SMS->MessageReference = buffer[Layout->TPMR];

	SMS->ReplaceMessage = 0;
	if (Layout->TPPID != 255) {
		if (buffer[Layout->TPPID] > 0x40 && buffer[Layout->TPPID] < 0x48) {
			SMS->ReplaceMessage = buffer[Layout->TPPID] - 0x40;
		}
	}
	SMS->RejectDuplicates = false;
	if ((buffer[Layout->firstbyte] & 0x04)==0x04) SMS->RejectDuplicates = true;

	return ERR_NONE;
}

static GSM_Error GNAPGEN_ReplyGetSMS(GSM_Protocol_Message msg, GSM_StateMachine *s) {
	GSM_SMSMessage *sms = 0;
	unsigned char buffer[800];
	int numberOfMessages = 0;
	int messageLen = 0;
	int state = 0;
	int i;
	int current=6;
	GSM_SMSMessageLayout layout;

	numberOfMessages = msg.Buffer[4] * 256 + msg.Buffer[5];
	s->Phone.Data.GetSMSMessage->Number = numberOfMessages;

	switch( msg.Buffer[msg.Length-1] ) {
		case 0x01:
			state = SMS_Read;
			break;
		case 0x03:
			state = SMS_UnRead;
			break;
		case 0x05:
			state = SMS_Sent;
			break;
		case 0x07:
			state = SMS_UnSent;
			break;
	}

	for( i=0; i<numberOfMessages; i++ ) {
		messageLen = msg.Buffer[current] * 256 + msg.Buffer[current+1];
		memset( buffer, 0, 800 );
		memcpy( buffer, msg.Buffer + current + 2, messageLen );
		current += messageLen + 2;
		sms = &s->Phone.Data.GetSMSMessage->SMS[i];

		sms->State = state;
		s->Phone.Data.GetSMSMessage->SMS[i].Name[0] 	= 0;
		s->Phone.Data.GetSMSMessage->SMS[i].Name[1]	= 0;

		GNAPGEN_PrivSetSMSLayout( sms, buffer, &layout );
		GNAPGEN_DecodeSMSFrame(sms,buffer,&layout);
	}

	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetNextSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	GSM_Phone_GNAPGENData           *Priv = &s->Phone.Data.Priv.GNAPGEN;
	GSM_Phone_Data			*Data = &s->Phone.Data;

	/* @todo remove the folder attribute from the request header since it's not used on the phone */

	unsigned char req [8] = {0,11,
				 0x00,0x0c, 		/* folder */
				 0x00,0x10,0x00,0x3F};	/* location */
	int i;
	bool skipfolder;
	GSM_Error error;

	if( start ) {
		Priv->CurrentFolderNumber = 0;
		Priv->CurrentSMSNumber = 0;
	}

	/*  retrieve the sms IDs for the given folder */
	if( Priv->CurrentSMSNumber == 0 )
		GNAPGEN_PrivGetSMSFolderStatus(s,Priv->SMSFolderID[Priv->CurrentFolderNumber]);

	for( i=0; i<5; i++ )
		GSM_SetDefaultSMSData(&sms->SMS[i]);

	/*  if there are no SMS in current folder, just skip it */
	do {
		if( Priv->SMSCount == 0 ) {
			if( (Data->SMSFolders->Number - 1) == Priv->CurrentFolderNumber )
				return ERR_EMPTY;
			else {
				Priv->CurrentSMSNumber = 0;
				Priv->CurrentFolderNumber++;
			}
			GNAPGEN_PrivGetSMSFolderStatus(s,Priv->SMSFolderID[Priv->CurrentFolderNumber]);
			skipfolder  = true;
		} else
			skipfolder = false;
	} while( skipfolder );

	/*  set location */
	req[4] = Priv->SMSIDs[Priv->CurrentSMSNumber].byte1;
	req[5] = Priv->SMSIDs[Priv->CurrentSMSNumber].byte2;
	req[6] = Priv->SMSIDs[Priv->CurrentSMSNumber].byte3;
	req[7] = Priv->SMSIDs[Priv->CurrentSMSNumber].byte4;

	sms->SMS[0].Folder = Priv->SMSFolderID[Priv->CurrentFolderNumber];
	if( Priv->CurrentFolderNumber == 0 )
		sms->SMS[0].InboxFolder = true;

	sms->SMS[0].Location = ( req[4] * (256*256*256) ) + req[5] * 256 * 256 + req[6] * 256 + req[7];

	sms->SMS[0].Memory = MEM_ME;

	s->Phone.Data.GetSMSMessage=sms;

	error = GSM_WaitFor (s, req, 8, 0x6, 500, ID_GetSMSMessage);
	if( error != ERR_NONE )
		return error;

	if( Priv->CurrentSMSNumber == ( Priv->SMSCount - 1 ) ) {
		/*  last sms in current folder reached, check if there are folders left to checkout */
		if( Priv->CurrentFolderNumber == (Data->SMSFolders->Number - 1) )
			return ERR_UNKNOWN;
		else {
			Priv->CurrentSMSNumber = 0;
			Priv->CurrentFolderNumber++;
		}
	} else
		Priv->CurrentSMSNumber++;

	return error;
}

static GSM_Error GNAPGEN_EncodeSMSFrame(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char *req, GSM_SMSMessageLayout *Layout, int *length)
{
	int			count = 0;
	GSM_Error		error;

	memset(Layout,255,sizeof(GSM_SMSMessageLayout));

	sms->Class = -1;

	*Layout = PHONE_SMSSubmit;

	/*  smsc number is semi-octet */
	Layout->SMSCNumber 	 = count;
	smprintf(s, "SMSCNumber: %d\n", count );
	if( UnicodeLength(sms->SMSC.Number) == 0 )
		count += (UnicodeLength(sms->SMSC.Number) / 2) + 1;
	else
		count += ((UnicodeLength(sms->SMSC.Number) + 1 ) / 2) + 1;

	/*  firstbyte set in SMS Layout */
	Layout->firstbyte 	 = count;
	smprintf(s, "firstbyte: %d\n", count);
	count++;

	if (sms->PDU != SMS_Deliver) {
		Layout->TPMR 	 = count;
		smprintf(s, "TPMR: %d\n", Layout->TPMR);
		count++;
	}

	/*  Phone number */
	Layout->Number 		 = count;
	smprintf(s, "Number: %d\n", count);

	if( UnicodeLength(sms->Number) == 0 )
		count += (UnicodeLength(sms->Number) / 2) + 1;
	else
		count += ((UnicodeLength(sms->Number) + 1 ) / 2) + 1;


	Layout->TPPID	 = count;
	smprintf(s, "TPPID: %d\n", count);
	count++;


	Layout->TPDCS 	 = count;
	smprintf(s, "TPDCS: %d\n", count);
	count++;

	if (sms->PDU == SMS_Deliver) {
		Layout->DateTime = count;
		smprintf(s, "DateTime: %d\n", count);
		count += 7;
	} else {
		Layout->TPVP 	 = count;
		smprintf(s, "TPVP: %d\n", count);
		count++;
	}

	Layout->TPUDL 		 = count;
	smprintf(s, "TPUDL: %d\n", count);
	count++;

	Layout->Text 		 = count;
	smprintf(s, "Text: %d\n", count);

	error = PHONE_EncodeSMSFrame(s,sms,req,*Layout,length,false);
	if (error != ERR_NONE) return error;




	req[0] = 0x0b;
	req[10] = 0x07;

	return ERR_NONE;
}

static GSM_Error GNAPGEN_SendSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	int			length = 11;
	GSM_Error		error;
	GSM_SMSMessageLayout 	Layout;
	unsigned char req [300] = {0,15};

	if (sms->PDU == SMS_Deliver) sms->PDU = SMS_Submit;
	memset(req+2,0x00,sizeof(req) - 2);
	error=GNAPGEN_EncodeSMSFrame(s, sms, req + 2, &Layout, &length);
	if (error != ERR_NONE) return error;
	DumpMessage(&s->di, req, length+1);
	/* return ERR_NONE; */
	smprintf(s, "Sending sms\n");
	return s->Protocol.Functions->WriteMessage(s, req, length + 2, 0x06);
}

GSM_Error GNAPGEN_ReplySetSMS (GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s) {
	/* @todo do something useful here ;) */
	smprintf(s, "SetSMS: got reply\n");
	return ERR_NONE;
}

static GSM_Error GNAPGEN_PrivSetSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	int			length = 0;
	GSM_SMSMessageLayout 	Layout;
	GSM_Error		error;


	unsigned char req [300] = {0,13,
				0x00,0x0c, /* folder */
				0x00,0x10,0x00,0x3F}; /* location */


	req[3] = sms->Folder;

	/* @todo allow existing messages to be edited (not implemented in gnapplet) */

	if (sms->PDU == SMS_Deliver)
		sms->PDU = SMS_Submit;

	memset(req+8,0x00,300-8);

	error = GNAPGEN_EncodeSMSFrame( s, sms, req + 10, &Layout, &length );
	req[9] = length;

	s->Phone.Data.SaveSMSMessage=sms;
	return GSM_WaitFor (s, req, length+10, 0x6, 4, ID_SaveSMSMessage);
}

static GSM_Error GNAPGEN_AddSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
/* 	N6510_GetSMSLocation(s, sms, &folderid, &location); */
/* 	location = 0; */
/* 	N6510_SetSMSLocation(s, sms, folderid, location); */
	return GNAPGEN_PrivSetSMSMessage(s, sms);
}

/* ----------------------------------------------------------------------------- */

static GSM_Error GNAPGEN_ReplyGetSignalQuality(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Network level received: %i\n",msg.Buffer[4]);
    	Data->SignalQuality->SignalStrength 	= -1;
    	Data->SignalQuality->SignalPercent 	= ((int)msg.Buffer[4]);
    	Data->SignalQuality->BitErrorRate 	= -1;
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetSignalQuality(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	unsigned char req[] = {0x00,0x03};

	s->Phone.Data.SignalQuality = sig;
	smprintf(s, "Getting network level\n");
	return GSM_WaitFor (s, req, 2, 0x03, 4, ID_GetSignalQuality);
}

static GSM_Error GNAPGEN_ReplyGetBatteryCharge(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Battery level received: %i\n",msg.Buffer[4]);
    	Data->BatteryCharge->BatteryPercent 	= ((int)(msg.Buffer[4]));
    	Data->BatteryCharge->ChargeState 	= 0;
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	unsigned char req[] = {0x00, 0x01};

	GSM_ClearBatteryCharge(bat);
	s->Phone.Data.BatteryCharge = bat;
	smprintf(s, "Getting battery level\n");
	return GSM_WaitFor (s, req, 2, 0x04, 4, ID_GetBatteryCharge);
}

static GSM_Error GNAPGEN_ReplyGetNetworkInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data	*Data = &s->Phone.Data;
	char		buf[100];
#ifdef DEBUG
	GSM_NetworkInfo NetInfo;

	smprintf(s, "Network status            : ");
	switch (msg.Buffer[9]) {
		case 0x00 : smprintf(s, "home network ?\n");		break;
		default	  : smprintf(s, "unknown %i!\n",msg.Buffer[9]); break;
	}
	if (msg.Buffer[9]==0x00) {
		sprintf(NetInfo.CID, "%02X%02X", msg.Buffer[4], msg.Buffer[5]);
		smprintf(s, "CID                       : %s\n", NetInfo.CID);
		sprintf(NetInfo.LAC, "%02X%02X", msg.Buffer[6], msg.Buffer[7]);
		smprintf(s, "LAC                       : %s\n", NetInfo.LAC);

		memset(buf,0,sizeof(buf));
		memcpy(buf,msg.Buffer+11,msg.Buffer[10]*2);
		sprintf(NetInfo.NetworkCode,"%s",DecodeUnicodeString(buf));
		smprintf(s, "Network code              : %s\n", NetInfo.NetworkCode);
		smprintf(s, "Network name for Gammu    : %s ",
			DecodeUnicodeString(GSM_GetNetworkName(NetInfo.NetworkCode)));
		smprintf(s, "(%s)\n",DecodeUnicodeString(GSM_GetCountryName(NetInfo.NetworkCode)));
	}
#endif
	Data->NetworkInfo->NetworkName[0] = 0x00;
	Data->NetworkInfo->NetworkName[1] = 0x00;
	Data->NetworkInfo->State 	  = 0;
	switch (msg.Buffer[8]) {
		case 0x00: Data->NetworkInfo->State = GSM_HomeNetwork;		break;
	}
	if (Data->NetworkInfo->State == GSM_HomeNetwork) {
		sprintf(Data->NetworkInfo->CID, "%02X%02X", msg.Buffer[4], msg.Buffer[5]);
		sprintf(Data->NetworkInfo->LAC, "%02X%02X", msg.Buffer[6], msg.Buffer[7]);

		memset(buf,0,sizeof(buf));
		memcpy(buf,msg.Buffer+11,msg.Buffer[10]*2);
		sprintf(Data->NetworkInfo->NetworkCode,"%s",DecodeUnicodeString(buf));
	}
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo)
{
	unsigned char req[] = {0x00, 0x01};

	s->Phone.Data.NetworkInfo=netinfo;
	smprintf(s, "Getting network info\n");
	return GSM_WaitFor (s, req, 2, 0x03, 4, ID_GetNetworkInfo);
}

static GSM_Error GNAPGEN_ReplyGetMemoryStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Memory status received\n");
	if (msg.Length == 4) return ERR_EMPTY;

	Data->MemoryStatus->MemoryUsed = msg.Buffer[8]*256 + msg.Buffer[9];
	Data->MemoryStatus->MemoryFree = msg.Buffer[12]*256 + msg.Buffer[13];
	smprintf(s, "Free       : %i\n",Data->MemoryStatus->MemoryFree);
	smprintf(s, "Used       : %i\n",Data->MemoryStatus->MemoryUsed);

	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	unsigned char req[] = {0x00,0x07,0x00,
			       0x00};		/* memory type */

	if (Status->MemoryType != MEM_ME) {
		req[3] = NOKIA_GetMemoryType(s, Status->MemoryType,GNAPGEN_MEMORY_TYPES);
		if (req[3]==0xff) return ERR_NOTSUPPORTED;
	}

	s->Phone.Data.MemoryStatus=Status;
	smprintf(s, "Getting memory status\n");
	return GSM_WaitFor (s, req, 4, 0x02, 4, ID_GetMemoryStatus);
}

GSM_Error GNAPGEN_ReplyGetNextMemory( GSM_Protocol_Message msg, GSM_StateMachine *s ) {
	int 			i,pos=8,type,subtype,len;
        GSM_MemoryEntry         *entry = s->Phone.Data.Memory;
	GSM_Phone_GNAPGENData	*Priv = &s->Phone.Data.Priv.GNAPGEN;

	/*  17 == GN_ERR_INVALIDMEMORYTYPE */
	if( msg.Buffer[3] == 17 ) {
		smprintf(s, "unknown memory type\n");
		return ERR_UNKNOWN;
	}

	entry->Location=msg.Buffer[5];
	entry->EntriesNum=0;

	for (i=0;i<msg.Buffer[7];i++) {
		type = msg.Buffer[pos]*256+msg.Buffer[pos+1];
		subtype = msg.Buffer[pos+2]*256+msg.Buffer[pos+3];
		pos+=4;
		switch (type) {
                /*  name */
		case 0x07:
			len = msg.Buffer[pos]*256+msg.Buffer[pos+1];
			if (len!=0) {
				entry->Entries[entry->EntriesNum].EntryType=PBK_Text_Name;
				memcpy(entry->Entries[entry->EntriesNum].Text,msg.Buffer+pos+2,len*2);
				entry->Entries[entry->EntriesNum].Text[len*2]=0;
				entry->Entries[entry->EntriesNum].Text[len*2+1]=0;
				entry->EntriesNum++;
			}
			pos+=2+len*2;
			break;
                /*  email */
                case 0x08:
			len = msg.Buffer[pos]*256+msg.Buffer[pos+1];
			entry->Entries[entry->EntriesNum].EntryType=PBK_Text_Email;
			memcpy(entry->Entries[entry->EntriesNum].Text,msg.Buffer+pos+2,len*2);
			entry->Entries[entry->EntriesNum].Text[len*2]=0;
			entry->Entries[entry->EntriesNum].Text[len*2+1]=0;
			entry->EntriesNum++;
			pos+=2+len*2;
			break;
                /*  0x0b is a general identifier for a number */
		case 0x0B:
			switch(subtype) {
				/*  fax */
				case 0x04:
					entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Fax;
					break;
				/*  work */
				case 0x06:
					entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Work;
					break;
				/*  mobile */
				case 0x03:
					entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Mobile;
					break;
				/*  home */
				case 0x02:
					entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Home;
					break;
				/*  general */
				case 0x0a: default:
					entry->Entries[entry->EntriesNum].EntryType=PBK_Number_General;
					break;
			}
			len = msg.Buffer[pos]*256+msg.Buffer[pos+1];
			memcpy(entry->Entries[entry->EntriesNum].Text,msg.Buffer+pos+2,len*2);
			entry->Entries[entry->EntriesNum].Text[len*2]=0;
			entry->Entries[entry->EntriesNum].Text[len*2+1]=0;
			entry->EntriesNum++;
			pos+=2+len*2;
			break;
                /*  date */
		case 0x13:
			entry->Entries[entry->EntriesNum].EntryType=PBK_Date;
			NOKIA_DecodeDateTime(s, msg.Buffer+pos, &entry->Entries[entry->EntriesNum].Date);
			entry->EntriesNum++;
			pos+=2+7;
			break;
		/*  note */
		case 0x0a:
			len = msg.Buffer[pos]*256+msg.Buffer[pos+1];
			entry->Entries[entry->EntriesNum].EntryType=PBK_Text_Note;
			memcpy(entry->Entries[entry->EntriesNum].Text,msg.Buffer+pos+2,len*2);
			entry->Entries[entry->EntriesNum].Text[len*2]=0;
			entry->Entries[entry->EntriesNum].Text[len*2+1]=0;
			entry->EntriesNum++;
			pos+=2+len*2;
			break;
		/*  url */
		case 0x2c:
			len = msg.Buffer[pos]*256+msg.Buffer[pos+1];
			entry->Entries[entry->EntriesNum].EntryType=PBK_Text_URL;
			memcpy(entry->Entries[entry->EntriesNum].Text,msg.Buffer+pos+2,len*2);
			entry->Entries[entry->EntriesNum].Text[len*2]=0;
			entry->Entries[entry->EntriesNum].Text[len*2+1]=0;
			entry->EntriesNum++;
			pos+=2+len*2;
			break;
		default:
			Priv->LastContactArrived = true;
			return ERR_UNKNOWN;
		}
	}

	return ERR_NONE;

}


static GSM_Error GNAPGEN_GetNextMemory( GSM_StateMachine *s, GSM_MemoryEntry *entry, bool start ) {
	GSM_Phone_GNAPGENData           *Priv = &s->Phone.Data.Priv.GNAPGEN;

	unsigned char req[] = {0x00, 11,
			       0x00, 0x00,	/* memory type */
			       0x00, 0x00};	/* start (bool) */

	if( start ) {
		Priv->LastContactArrived = false;
		req[5] = 0x01;
	} else
		req[5] = 0x00;

	if( Priv->LastContactArrived )
		return ERR_EMPTY;

	if (entry->MemoryType != MEM_ME) {
		req[3] = NOKIA_GetMemoryType(s, entry->MemoryType,GNAPGEN_MEMORY_TYPES);
		if (req[3]==0xff) return ERR_NOTSUPPORTED;
	}

	s->Phone.Data.Memory=entry;
	return GSM_WaitFor (s, req, 6, 0x02, 6, ID_GetMemory);
}

static GSM_Error GNAPGEN_ReplyGetMemory(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 			i,pos=6,type,subtype,len;
        GSM_MemoryEntry         *entry = s->Phone.Data.Memory;

	entry->EntriesNum=0;

	smprintf(s, "Phonebook entry received\n");
	for (i=0;i<msg.Buffer[5];i++) {
		type = msg.Buffer[pos]*256+msg.Buffer[pos+1];
		subtype = msg.Buffer[pos+2]*256+msg.Buffer[pos+3];
		pos+=4;
		switch (type) {
                /*  name */
		case 0x07:
			len = msg.Buffer[pos]*256+msg.Buffer[pos+1];
			if (len!=0) {
				entry->Entries[entry->EntriesNum].EntryType=PBK_Text_Name;
				memcpy(entry->Entries[entry->EntriesNum].Text,msg.Buffer+pos+2,len*2);
				entry->Entries[entry->EntriesNum].Text[len*2]=0;
				entry->Entries[entry->EntriesNum].Text[len*2+1]=0;
				entry->EntriesNum++;
			}
			pos+=2+len*2;
			break;
                /*  email */
                case 0x08:
			len = msg.Buffer[pos]*256+msg.Buffer[pos+1];
			entry->Entries[entry->EntriesNum].EntryType=PBK_Text_Email;
			memcpy(entry->Entries[entry->EntriesNum].Text,msg.Buffer+pos+2,len*2);
			entry->Entries[entry->EntriesNum].Text[len*2]=0;
			entry->Entries[entry->EntriesNum].Text[len*2+1]=0;
			entry->EntriesNum++;
			pos+=2+len*2;
			break;
                /*  0x0b is a general identifier for a number */
		case 0x0B:
			switch(subtype) {
				/*  fax */
				case 0x04:
					entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Fax;
					break;
				/*  work */
				case 0x06:
					entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Work;
					break;
				/*  mobile */
				case 0x03:
					entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Mobile;
					break;
				/*  home */
				case 0x02:
					entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Home;
					break;
				/*  general */
				case 0x0a: default:
					entry->Entries[entry->EntriesNum].EntryType=PBK_Number_General;
					break;
			}
			len = msg.Buffer[pos]*256+msg.Buffer[pos+1];
			memcpy(entry->Entries[entry->EntriesNum].Text,msg.Buffer+pos+2,len*2);
			entry->Entries[entry->EntriesNum].Text[len*2]=0;
			entry->Entries[entry->EntriesNum].Text[len*2+1]=0;
			entry->EntriesNum++;
			pos+=2+len*2;
			break;
                /*  date */
		case 0x13:
			entry->Entries[entry->EntriesNum].EntryType=PBK_Date;
			NOKIA_DecodeDateTime(s, msg.Buffer+pos, &entry->Entries[entry->EntriesNum].Date);
			entry->EntriesNum++;
			pos+=2+7;
			break;
		/*  note */
		case 0x0a:
			len = msg.Buffer[pos]*256+msg.Buffer[pos+1];
			entry->Entries[entry->EntriesNum].EntryType=PBK_Text_Note;
			memcpy(entry->Entries[entry->EntriesNum].Text,msg.Buffer+pos+2,len*2);
			entry->Entries[entry->EntriesNum].Text[len*2]=0;
			entry->Entries[entry->EntriesNum].Text[len*2+1]=0;
			entry->EntriesNum++;
			pos+=2+len*2;
			break;
		/*  url */
		case 0x2c:
			len = msg.Buffer[pos]*256+msg.Buffer[pos+1];
			entry->Entries[entry->EntriesNum].EntryType=PBK_Text_URL;
			memcpy(entry->Entries[entry->EntriesNum].Text,msg.Buffer+pos+2,len*2);
			entry->Entries[entry->EntriesNum].Text[len*2]=0;
			entry->Entries[entry->EntriesNum].Text[len*2+1]=0;
			entry->EntriesNum++;
			pos+=2+len*2;
			break;
		default:
			smprintf(s, "unknown %i\n",type);
			return ERR_UNKNOWN;
		}
	}

	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetMemory (GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	unsigned char req[] = {0x00, 0x01,
			       0x00, 0x00, 		/* memory type */
			       0x00, 0x00, 0x00, 0x00}; /* location */

	if (entry->MemoryType != MEM_ME) {
		req[3] = NOKIA_GetMemoryType(s, entry->MemoryType,GNAPGEN_MEMORY_TYPES);
		if (req[3]==0xff) return ERR_NOTSUPPORTED;
	}

	if (entry->Location==0x00) return ERR_INVALIDLOCATION;

	req[6] = entry->Location / 256;
	req[7] = entry->Location % 256;

	s->Phone.Data.Memory=entry;
	smprintf(s, "Getting phonebook entry\n");
	return GSM_WaitFor (s, req, 8, 0x02, 6, ID_GetMemory);
}

static GSM_Error GNAPGEN_ReplySetMemory(GSM_Protocol_Message msg, GSM_StateMachine *s) {
	smprintf(s, "Got reply: SetMemory()\n");
	if( msg.Buffer[4] == 0 )
		return ERR_NONE;
	else
		return ERR_UNKNOWN;
}

static GSM_Error GNAPGEN_SetMemory (GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	unsigned char req[1000] = {0x00, 0x03,
				   0x00, 0x00, 		/* memory type */
				   0x00, 0x00, 0x00, 0x00, /* location */
				   0x00, 0x00 };	/* number of entries to read */
	/*  for each entry, 2 bytes are reserved for the entry type, */
	/*  2 bytes for the entry sub-type if available and 64 byte for the actual entry */
	int currentByte = 10;
	int i = 0;
	int entryCount = 0;
	GSM_SubMemoryEntry *subMemoryEntry;

	memset( req + 3,0x00,sizeof(req) - 3 );

	if (entry->MemoryType != MEM_ME) {
		req[3] = NOKIA_GetMemoryType(s, entry->MemoryType,GNAPGEN_MEMORY_TYPES);
		if (req[3]==0xff) return ERR_NOTSUPPORTED;
	}

	if (entry->Location==0x00) return ERR_INVALIDLOCATION;

	req[6] = entry->Location / 256;
	req[7] = entry->Location % 256;

	for( i=0; i< entry->EntriesNum; i++ ) {
		subMemoryEntry = &entry->Entries[i];
		switch( subMemoryEntry->EntryType ) {
			case PBK_Number_Home:
				req[currentByte++] = 0x00;
				req[currentByte++] = 0x0b;
				req[currentByte++] = 0x00;
				req[currentByte++] = 0x02;
				break;

			case PBK_Number_General:
			case PBK_Number_Mobile:
				req[currentByte++] = 0x00;
				req[currentByte++] = 0x0b;
				req[currentByte++] = 0x00;
				req[currentByte++] = 0x03;
				break;

			case PBK_Number_Work:
				req[currentByte++] = 0x00;
				req[currentByte++] = 0x0b;
				req[currentByte++] = 0x00;
				req[currentByte++] = 0x06;
				break;

			case PBK_Number_Fax:
				req[currentByte++] = 0x00;
				req[currentByte++] = 0x0b;
				req[currentByte++] = 0x00;
				req[currentByte++] = 0x04;
				break;

			case PBK_Text_Email:
				req[currentByte++] = 0x00;
				req[currentByte++] = 0x08;
				req[currentByte++] = 0x00;
				req[currentByte++] = 0x00;
				break;

			case PBK_Text_Name:
				req[currentByte++] = 0x00;
				req[currentByte++] = 0x07;
				req[currentByte++] = 0x00;
				req[currentByte++] = 0x00;
				break;
			default:
				continue;
		}

		entryCount++;

		req[currentByte++] = 0x00;
		req[currentByte++] = UnicodeLength( subMemoryEntry->Text );

		memcpy( req + currentByte, subMemoryEntry->Text, UnicodeLength( subMemoryEntry->Text ) * 2 );
		currentByte += UnicodeLength( subMemoryEntry->Text ) * 2;
	}

	req[9] = entryCount;

	return GSM_WaitFor(s, req, currentByte, 0x02, 4, ID_SetMemory);
}

GSM_Error GNAPGEN_ReplyDeleteMemory ( GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s ) {
	/* @todo implement error handling */
	smprintf(s, "Deleted\n");
	return ERR_NONE;
}

static GSM_Error GNAPGEN_DeleteMemory ( GSM_StateMachine *s, GSM_MemoryEntry *entry ) {
	unsigned char req[] = {0x00, 0x05,
			       0x00, 0x00,	/* memory type */
			       0x00, 0x00, 0x00, 0x00 }; /* location */

	if (entry->MemoryType != MEM_ME) {
		req[3] = NOKIA_GetMemoryType(s, entry->MemoryType,GNAPGEN_MEMORY_TYPES);
		if (req[3]==0xff) return ERR_NOTSUPPORTED;
	}

	if (entry->Location==0x00) return ERR_INVALIDLOCATION;

	req[6] = entry->Location / 256;
	req[7] = entry->Location % 256;

	return GSM_WaitFor (s, req, 8, 0x02, 6, ID_DeleteMemory);
}

static GSM_Error GNAPGEN_AddMemory ( GSM_StateMachine *s, GSM_MemoryEntry *entry ) {
	entry->Location = -1;
	return GNAPGEN_SetMemory( s, entry );
}

static GSM_Error GNAPGEN_ReplyDeleteSMSMessage ( GSM_Protocol_Message msg, GSM_StateMachine *s ) {
	/*  17 == GN_ERR_INVALIDMEMORYTYPE */
	switch( msg.Buffer[3] ) {
		case 16:
			smprintf(s, "invalid location\n");
			return ERR_UNKNOWN;
			break;
		case 17:
			smprintf(s, "unknown memory type\n");
			return ERR_UNKNOWN;
			break;
		case 0:
			smprintf(s, "deleted");
	}


	return ERR_NONE;
}

static GSM_Error GNAPGEN_DeleteSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms) {
	unsigned char req[] = {0x00, 17,
			       0x00, 0x00,	/* folder */
			       0x00, 0x00, 0x00, 0x00 }; /* location */

	req[3] = sms->Folder;

	req[4] = (sms->Location / (256*256*256)) % 256;
	req[5] = (sms->Location / (256*256)) % 256;
	req[6] = (sms->Location / 256) % 256 ;
	req[7] = sms->Location % 256;

	return GSM_WaitFor (s, req, 8, 0x06, 6, ID_DeleteSMSMessage );
}

GSM_Error GNAPGEN_ReplyGetToDo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_ToDoEntry 	*Last = s->Phone.Data.ToDo;
	int 		pos = 8;

	smprintf(s, "TODO received\n");

	memcpy(Last->Entries[0].Text,msg.Buffer+pos+2,(msg.Buffer[pos]*256+msg.Buffer[pos+1])*2);
	Last->Entries[0].Text[(msg.Buffer[pos]*256+msg.Buffer[pos+1])*2] = 0;
	Last->Entries[0].Text[(msg.Buffer[pos]*256+msg.Buffer[pos+1])*2+1] = 0;
	smprintf(s, "Text: \"%s\"\n",DecodeUnicodeString(Last->Entries[0].Text));
	pos+=(msg.Buffer[pos]*256+msg.Buffer[pos+1])*2+2;

	/**
	 * @todo There might be better type.
	 */
	Last->Type = GSM_CAL_MEMO;

	switch (msg.Buffer[pos]) {
		case 1  : Last->Priority = GSM_Priority_High; 	break;
		case 2  : Last->Priority = GSM_Priority_Medium; break;
		case 3  : Last->Priority = GSM_Priority_Low; 	break;
		default	: return ERR_UNKNOWN;
	}
	smprintf(s, "Priority: %i\n",msg.Buffer[4]);

 	Last->Entries[0].EntryType = TODO_TEXT;
	Last->EntriesNum	   = 1;

	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetNextToDo(GSM_StateMachine *s, GSM_ToDoEntry *ToDo, bool refresh)
{
	GSM_Error 		error;
	unsigned char           req[] = {0x00, 0x07,
                                	 0x00, 0x00, 0x00, 0x00};         /* Location */

	if (refresh) {
		ToDo->Location = 1;
	} else {
		ToDo->Location++;
	}

        req[4]          = ToDo->Location / 256;
        req[5]          = ToDo->Location % 256;

	s->Phone.Data.ToDo = ToDo;
	smprintf(s, "Getting todo\n");
	error = GSM_WaitFor (s, req, 6, 7, 4, ID_GetToDo);
	if (error == ERR_INVALIDLOCATION) error = ERR_EMPTY;
	return error;
}

GSM_Error GNAPGEN_DeleteCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	unsigned char                   req[] = {0x00, 0x05,
                                                 0x00, 0x00, 0x00, 0x00};         /* Location */

        req[4]          = Note->Location / 256;
        req[5]          = Note->Location % 256;

        smprintf(s, "Deleting calendar note\n");
	return GSM_WaitFor (s, req, 6, 7, 4, ID_DeleteCalendarNote);
}

GSM_Error GNAPGEN_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	GSM_DateTime		DT;
 	int 			Text, Time, Alarm, Phone, EndTime, Location, current=7;
	unsigned char 		req[5000] = {
		0x00, 0x03,0x00,0x00,
		0x00, 0x00, 0x00, 0x00, /* location ? 	*/
		0x00};			/* type */

	switch(Note->Type) {
		case GSM_CAL_MEETING 	: req[6] = 0x01; break;
		case GSM_CAL_REMINDER 	: req[6] = 0x04; break;
		case GSM_CAL_MEMO	:
		default 		: req[6] = 0x08; break;
	}

	GSM_CalendarFindDefaultTextTimeAlarmPhone(Note, &Text, &Time, &Alarm, &Phone, &EndTime, &Location);

	if (Time == -1) return ERR_UNKNOWN;
	memcpy(&DT,&Note->Entries[Time].Date,sizeof(GSM_DateTime));
	req[current++]	= DT.Year / 256;
	req[current++]	= DT.Year % 256;
	req[current++]	= DT.Month;
	req[current++]	= DT.Day;
	req[current++]	= DT.Hour;
	req[current++]	= DT.Minute;
	req[current++]	= DT.Second;

	if (EndTime == -1) {
		memset(&DT,0,sizeof(GSM_DateTime));
		DT.Month = 1;
		DT.Day = 1;

	} else {
		memcpy(&DT,&Note->Entries[EndTime].Date,sizeof(GSM_DateTime));
	}
	req[current++]	= DT.Year / 256;
	req[current++]	= DT.Year % 256;
	req[current++]	= DT.Month;
	req[current++]	= DT.Day;
	req[current++]	= DT.Hour;
	req[current++]	= DT.Minute;
	req[current++]	= DT.Second;

	if (Alarm == -1) {
		memset(&DT,0,sizeof(GSM_DateTime));
		DT.Month = 1;
		DT.Day = 1;
	} else {
		memcpy(&DT,&Note->Entries[Alarm].Date,sizeof(GSM_DateTime));
	}
	req[current++]	= DT.Year / 256;
	req[current++]	= DT.Year % 256;
	req[current++]	= DT.Month;
	req[current++]	= DT.Day;
	req[current++]	= DT.Hour;
	req[current++]	= DT.Minute;
	req[current++]	= DT.Second;

	if (Text == -1) return ERR_UNKNOWN;
	req[current++]	= UnicodeLength(Note->Entries[Text].Text) / 256;
	req[current++]	= UnicodeLength(Note->Entries[Text].Text) % 256;
	memcpy(req+current,Note->Entries[Text].Text,UnicodeLength(Note->Entries[Text].Text)*2);
	current+=UnicodeLength(Note->Entries[Text].Text)*2;

	req[current++]	= 0;
	req[current++]	= 0;

	if (Location==-1) {
		req[current++]	= 0;
		req[current++]	= 0;
	} else {
		req[current++]	= UnicodeLength(Note->Entries[Location].Text) / 256;
		req[current++]	= UnicodeLength(Note->Entries[Location].Text) % 256;
		memcpy(req+current,Note->Entries[Location].Text,UnicodeLength(Note->Entries[Location].Text)*2);
		current+=UnicodeLength(Note->Entries[Location].Text)*2;
	}

	if (Note->Type == GSM_CAL_MEETING) {
		GSM_GetCalendarRecurranceRepeat(req+current, NULL, Note);
		current+=2;
	} else {
		req[current++]	= 0xff;
		req[current++]	= 0xff;
	}

	smprintf(s, "Writing calendar note\n");
	return GSM_WaitFor (s, req, current, 7, 4, ID_SetCalendarNote);
}

static GSM_Error GNAPGEN_ReplyGetNextCalendar(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
        int                     pos;
        GSM_CalendarEntry       *Entry = s->Phone.Data.Cal;

        switch (msg.Buffer[3]) {
        case 0x00:
                smprintf(s, "Calendar note received\n");
                switch (msg.Buffer[8]) {
                        case 0x01: Entry->Type = GSM_CAL_MEETING;   break;
                        case 0x04: Entry->Type = GSM_CAL_REMINDER;  break;
                        case 0x08: Entry->Type = GSM_CAL_MEMO;      break;
                        default  :
                                smprintf(s, "Unknown note type %i\n",msg.Buffer[8]);
                                return ERR_UNKNOWNRESPONSE;
                }

		pos = 9;
                Entry->EntriesNum = 0;

                NOKIA_DecodeDateTime(s, msg.Buffer+pos, &Entry->Entries[Entry->EntriesNum].Date);
                smprintf(s, "Time        : %02i-%02i-%04i %02i:%02i:%02i\n",
                        Entry->Entries[Entry->EntriesNum].Date.Day,Entry->Entries[Entry->EntriesNum].Date.Month,Entry->Entries[Entry->EntriesNum].Date.Year,
                        Entry->Entries[Entry->EntriesNum].Date.Hour,Entry->Entries[Entry->EntriesNum].Date.Minute,Entry->Entries[Entry->EntriesNum].Date.Second);
                Entry->Entries[Entry->EntriesNum].EntryType = CAL_START_DATETIME;
                Entry->EntriesNum++;
		pos+=7;

                NOKIA_DecodeDateTime(s, msg.Buffer+pos, &Entry->Entries[Entry->EntriesNum].Date);
                smprintf(s, "Time        : %02i-%02i-%04i %02i:%02i:%02i\n",
                        Entry->Entries[Entry->EntriesNum].Date.Day,Entry->Entries[Entry->EntriesNum].Date.Month,Entry->Entries[Entry->EntriesNum].Date.Year,
                        Entry->Entries[Entry->EntriesNum].Date.Hour,Entry->Entries[Entry->EntriesNum].Date.Minute,Entry->Entries[Entry->EntriesNum].Date.Second);
                Entry->Entries[Entry->EntriesNum].EntryType = CAL_END_DATETIME;
                Entry->EntriesNum++;
		pos+=7;

                NOKIA_DecodeDateTime(s, msg.Buffer+pos, &Entry->Entries[Entry->EntriesNum].Date);
                if (Entry->Entries[Entry->EntriesNum].Date.Year!=0) {
	                smprintf(s, "Alarm       : %02i-%02i-%04i %02i:%02i:%02i\n",
	                        Entry->Entries[Entry->EntriesNum].Date.Day,Entry->Entries[Entry->EntriesNum].Date.Month,Entry->Entries[Entry->EntriesNum].Date.Year,
	                        Entry->Entries[Entry->EntriesNum].Date.Hour,Entry->Entries[Entry->EntriesNum].Date.Minute,Entry->Entries[Entry->EntriesNum].Date.Second);
	                Entry->Entries[Entry->EntriesNum].EntryType = CAL_TONE_ALARM_DATETIME;
	                Entry->EntriesNum++;
                } else {
                        smprintf(s, "No alarm\n");
                }
		pos+=7;

		memcpy(Entry->Entries[Entry->EntriesNum].Text,msg.Buffer+pos+2,msg.Buffer[pos+1]*2);
		Entry->Entries[Entry->EntriesNum].Text[msg.Buffer[pos+1]*2  ]=0;
		Entry->Entries[Entry->EntriesNum].Text[msg.Buffer[pos+1]*2+1]=0;
                smprintf(s, "Text \"%s\"\n",DecodeUnicodeString(Entry->Entries[Entry->EntriesNum].Text));
                if (msg.Buffer[pos+1] != 0x00) {
                        Entry->Entries[Entry->EntriesNum].EntryType = CAL_TEXT;
                        Entry->EntriesNum++;
                }
		pos+=msg.Buffer[pos+1]*2+4;

		memcpy(Entry->Entries[Entry->EntriesNum].Text,msg.Buffer+pos+2,msg.Buffer[pos+1]*2);
		Entry->Entries[Entry->EntriesNum].Text[msg.Buffer[pos+1]*2  ]=0;
		Entry->Entries[Entry->EntriesNum].Text[msg.Buffer[pos+1]*2+1]=0;
                smprintf(s, "Text \"%s\"\n",DecodeUnicodeString(Entry->Entries[Entry->EntriesNum].Text));
                if (msg.Buffer[pos+1] != 0x00) {
                        Entry->Entries[Entry->EntriesNum].EntryType = CAL_LOCATION;
                        Entry->EntriesNum++;
                }
		pos+=msg.Buffer[pos+1]*2+2;

		if (Entry->Type == GSM_CAL_MEETING) {
			GSM_GetCalendarRecurranceRepeat(msg.Buffer+pos, NULL, Entry);
		}

                return ERR_NONE;
        case 0x10:
                smprintf(s, "Can't get calendar note - too high location?\n");
                return ERR_INVALIDLOCATION;
        }
        return ERR_UNKNOWNRESPONSE;
}

static GSM_Error GNAPGEN_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start)
{
	GSM_Error 			error;
	GSM_Phone_GNAPGENData           *Priv = &s->Phone.Data.Priv.GNAPGEN;
	unsigned char                   req[] = {0x00, 0x01,
                                                 0x00, 0x00, 0x00, 0x00};         /* Location */

        if (start) {
                Priv->LastCalendarPos = 1;
        } else {
                Priv->LastCalendarPos++;
        }

        Note->Location  = Priv->LastCalendarPos;
        req[4]          = Priv->LastCalendarPos / 256;
        req[5]          = Priv->LastCalendarPos % 256;

        s->Phone.Data.Cal=Note;
        smprintf(s, "Getting calendar note\n");
        error = GSM_WaitFor (s, req, 6, 7, 4, ID_GetCalendarNote);
        if (error == ERR_INVALIDLOCATION) error = ERR_EMPTY;
        return error;
}

static GSM_Error GNAPGEN_ReplyGetSMSStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Used in phone memory   : %i\n",msg.Buffer[6]*256+msg.Buffer[7]);
	smprintf(s, "Unread in phone memory : %i\n",msg.Buffer[10]*256+msg.Buffer[11]);
	Data->SMSStatus->PhoneSize	= 0xff*256+0xff;
	Data->SMSStatus->PhoneUsed	= msg.Buffer[6]*256+msg.Buffer[7];
	Data->SMSStatus->PhoneUnRead 	= msg.Buffer[10]*256+msg.Buffer[11];
	Data->SMSStatus->TemplatesUsed = 0;
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	unsigned char req[] = {0x00, 0x09};

	s->Phone.Data.SMSStatus=status;
	smprintf(s, "Getting SMS status\n");
	return GSM_WaitFor (s, req, 2, 0x6, 2, ID_GetSMSStatus);
}

static GSM_Error GNAPGEN_ReplyGetSMSFolders(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 			j, pos;
	GSM_Phone_Data		*Data = &s->Phone.Data;
	GSM_Phone_GNAPGENData   *Priv = &s->Phone.Data.Priv.GNAPGEN;

	smprintf(s, "SMS folders names received\n");
	Data->SMSFolders->Number = msg.Buffer[5];

	pos = 6;
	for (j=0;j<msg.Buffer[5];j++) {
		if (msg.Buffer[pos+3]>GSM_MAX_SMS_FOLDER_NAME_LEN) {
			smprintf(s, "Too long text\n");
			return ERR_UNKNOWNRESPONSE;
		}
		Priv->SMSFolderID[j] = msg.Buffer[pos+1];

		memcpy(Data->SMSFolders->Folder[j].Name,msg.Buffer + pos+4,msg.Buffer[pos+3]*2);
		Data->SMSFolders->Folder[j].Name[msg.Buffer[pos+3]*2]=0;
		Data->SMSFolders->Folder[j].Name[msg.Buffer[pos+3]*2+1]=0;
		smprintf(s, "id: %d, folder name: \"%s\"\n",msg.Buffer[pos+1], DecodeUnicodeString(Data->SMSFolders->Folder[j].Name));

		if( msg.Buffer[pos+1] == 12 )
			Data->SMSFolders->Folder[j].InboxFolder = true;
		else
			Data->SMSFolders->Folder[j].InboxFolder = false;
		Data->SMSFolders->Folder[j].OutboxFolder = false;
		/**
		 * @todo Need to detect outbox folder somehow.
		 */
		Data->SMSFolders->Folder[j].Memory 	  = MEM_ME;
		pos+=msg.Buffer[pos+3]*2+4;
	}
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	unsigned char req[] = {0x00,0x01};

	s->Phone.Data.SMSFolders=folders;
	smprintf(s, "Getting SMS folders\n");
	return GSM_WaitFor (s, req, 2, 0x06, 4, ID_GetSMSFolders);
}

static GSM_Error GNAPGEN_ReplyGetSMSC(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 			pos=7;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	if (msg.Buffer[7]*2>GSM_MAX_SMSC_NAME_LENGTH) {
		smprintf(s, "Too long name\n");
		return ERR_UNKNOWNRESPONSE;
	}
	memcpy(Data->SMSC->Name,msg.Buffer+8,msg.Buffer[7]*2);
	Data->SMSC->Name[msg.Buffer[7]*2] = 0;
	Data->SMSC->Name[msg.Buffer[7]*2+1] = 0;
	smprintf(s, "   Name \"%s\"\n", DecodeUnicodeString(Data->SMSC->Name));
	pos+=msg.Buffer[7]*2;

	pos+=4;

	Data->SMSC->Format 		= SMS_FORMAT_Text;
	Data->SMSC->Validity.Format	= SMS_Validity_RelativeFormat;
	Data->SMSC->Validity.Relative 	= SMS_VALID_Max_Time;

	Data->SMSC->DefaultNumber[0] = 0;
	Data->SMSC->DefaultNumber[1] = 0;

	memcpy(Data->SMSC->Number,msg.Buffer+pos+4,msg.Buffer[pos+3]*2);
	Data->SMSC->Number[msg.Buffer[pos+3]*2] = 0;
	Data->SMSC->Number[msg.Buffer[pos+3]*2+1] = 0;
	smprintf(s, "   Number \"%s\"\n", DecodeUnicodeString(Data->SMSC->Number));

	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	unsigned char req[] = {0x00,21,
			       0x00,0x01};		/* location */

	if (smsc->Location==0x00) return ERR_INVALIDLOCATION;

	req[3]=smsc->Location-1;

	s->Phone.Data.SMSC=smsc;
	smprintf(s, "Getting SMSC\n");
	return GSM_WaitFor (s, req, 4, 0x06, 4, ID_GetSMSC);
}

static GSM_Error GNAPGEN_ReplyGetAlarm(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Alarm received\n");
	if (msg.Buffer[4] == 0x00) return ERR_EMPTY;
	Data->Alarm->Repeating 		= false;
	Data->Alarm->Text[0] 		= 0;
	Data->Alarm->Text[1] 		= 0;
	NOKIA_DecodeDateTime(s, msg.Buffer+5, &Data->Alarm->DateTime);
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetAlarm(GSM_StateMachine *s, GSM_Alarm *timedelta)
{
	unsigned char req[] = {0x00, 0x05};

	if (timedelta->Location != 1) return ERR_NOTSUPPORTED;

	s->Phone.Data.Alarm=timedelta;
	smprintf(s, "Getting alarm\n");
	return GSM_WaitFor (s, req, 2, 0x8, 4, ID_GetAlarm);
}

static GSM_Error GNAPGEN_ReplyGetDateTime(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Date & time received\n");
	NOKIA_DecodeDateTime(s, msg.Buffer+4, s->Phone.Data.DateTime);
	return ERR_NONE;
}

static GSM_Error GNAPGEN_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	unsigned char req[2] = {0x00,0x01};

	s->Phone.Data.DateTime=date_time;
	smprintf(s, "Getting date & time\n");
	return GSM_WaitFor (s, req, 2, 0x08, 4, ID_GetDateTime);
}

GSM_Error GNAPGEN_ReplyDialVoice( GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s ) {
	smprintf(s, "Dialed voice number\n");
	return ERR_NONE;
}

static GSM_Error GNAPGEN_DialVoice ( GSM_StateMachine *s, char *Number, GSM_CallShowNumber ShowNumber UNUSED) {
	/* @todo implement ShowNumber */
	unsigned char req[100] = {0x00,0x09};

	int currentByte = 2;

	unsigned char unicodeNumber[200];

	memset( req + 2,0x00,sizeof(req) - 2 );

	EncodeUnicode( unicodeNumber, Number, strlen(Number) );

	req[currentByte++] = 0x00;
	req[currentByte++] = UnicodeLength( unicodeNumber );

	memcpy( req + currentByte, unicodeNumber, UnicodeLength( unicodeNumber ) * 2 );
	currentByte += UnicodeLength( unicodeNumber ) * 2;

	return GSM_WaitFor (s, req, currentByte, 0x02, 8, ID_DialVoice);
}

GSM_Error GNAPGEN_ReplyGetHW(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 	buff[200];
	int 		pos=8,len,i;

	for (i=0;i<5;i++) {
		len=msg.Buffer[pos]*256+msg.Buffer[pos+1];
		memset(buff,0,sizeof(buff));
		memcpy(buff,msg.Buffer+pos+2,len*2);
		pos+=2+len*2;
	}

	strcpy(s->Phone.Data.HardwareCache,DecodeUnicodeString(buff));
	smprintf(s, "Received HW %s\n",s->Phone.Data.HardwareCache);

	return ERR_NONE;
}

GSM_Error GNAPGEN_GetHW(GSM_StateMachine *s, char *value)
{
	GSM_Error	error;
	unsigned char req[2] = {0x00,0x01};

	if (strlen(s->Phone.Data.HardwareCache)!=0) {
		strcpy(value,s->Phone.Data.HardwareCache);
		return ERR_NONE;
	}

	smprintf(s, "Getting HW\n");
	error = GSM_WaitFor (s, req, 2, 0x01, 2, ID_GetHardware);
	if (error == ERR_NONE) strcpy(value,s->Phone.Data.HardwareCache);
	return error;
}

GSM_Error GNAPGEN_ReplyGetManufacturer(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 			buff[200];
	int 				pos=8,len;
	GSM_Phone_GNAPGENData           *Priv = &s->Phone.Data.Priv.GNAPGEN;

	smprintf(s, "gnapplet %i. %i\n",msg.Buffer[4]*256+msg.Buffer[5],msg.Buffer[6]*256+msg.Buffer[7]);
	Priv->GNAPPLETVer = msg.Buffer[4]*256+msg.Buffer[5] * 100 + msg.Buffer[6]*256+msg.Buffer[7];

	len=msg.Buffer[pos]*256+msg.Buffer[pos+1];
	memset(buff,0,sizeof(buff));
	memcpy(buff,msg.Buffer+pos+2,len*2);

	strcpy(s->Phone.Data.Manufacturer,DecodeUnicodeString(buff));

	return ERR_NONE;
}

GSM_Error GNAPGEN_GetManufacturer(GSM_StateMachine *s)
{
	unsigned char req[2] = {0x00,0x01};

	smprintf(s, "Getting manufacturer\n");
	return GSM_WaitFor (s, req, 2, 0x01, 2, ID_GetManufacturer);
}

GSM_Error GNAPGEN_ReplyGetIMEI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char 	buff[200];
	int 		pos=8,len,i;

	for (i=0;i<3;i++) {
		len=msg.Buffer[pos]*256+msg.Buffer[pos+1];
		memset(buff,0,sizeof(buff));
		memcpy(buff,msg.Buffer+pos+2,len*2);
		pos+=2+len*2;
	}

	strcpy(s->Phone.Data.IMEI,DecodeUnicodeString(buff));
	smprintf(s, "Received IMEI %s\n",s->Phone.Data.IMEI);

	return ERR_NONE;
}

GSM_Error GNAPGEN_GetIMEI(GSM_StateMachine *s)
{
	unsigned char req[2] = {0x00,0x01};

	smprintf(s, "Getting IMEI\n");
	return GSM_WaitFor (s, req, 2, 0x01, 2, ID_GetIMEI);
}

GSM_Error GNAPGEN_ReplyGetID(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s UNUSED)
{
	return ERR_NONE;
}

GSM_Error GNAPGEN_ReplyGetModelFirmware(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_CutLines	lines;
	GSM_Phone_Data	*Data = &s->Phone.Data;

	if (Data->RequestID!=ID_GetManufacturer && Data->RequestID!=ID_GetModel) return ERR_NONE;

	InitLines(&lines);
	SplitLines(DecodeUnicodeString(msg.Buffer+6), msg.Length-6, &lines, "\x0A", 1, false);

	strcpy(Data->Model,GetLineString(DecodeUnicodeString(msg.Buffer+6), &lines, 4));
	smprintf(s, "Received model %s\n",Data->Model);
	Data->ModelInfo = GetModelData(s, NULL, Data->Model, NULL);

	strcpy(Data->VerDate,GetLineString(DecodeUnicodeString(msg.Buffer+6), &lines, 3));
	smprintf(s, "Received firmware date %s\n",Data->VerDate);

	strcpy(Data->Version,GetLineString(DecodeUnicodeString(msg.Buffer+6), &lines, 2));
	smprintf(s, "Received firmware version %s\n",Data->Version);
	GSM_CreateFirmwareNumber(s);

	FreeLines(&lines);

	return ERR_NONE;
}

GSM_Error GNAPGEN_GetModel (GSM_StateMachine *s)
{
	unsigned char req[2] = {0x00,0x01};
	GSM_Error 	error;

	if (strlen(s->Phone.Data.Model)>0) return ERR_NONE;

	smprintf(s, "Getting model\n");
	error = GSM_WaitFor (s, req, 2, 0x01, 2, ID_GetModel);
	if (error == ERR_NONE) {
		smprintf_level(s, D_TEXT, "[Connected model  - \"%s\"]\n",
				s->Phone.Data.Model);
	}
	return error;
}

GSM_Error GNAPGEN_GetFirmware (GSM_StateMachine *s)
{
	unsigned char req[2] = {0x00,0x01};
	GSM_Error error;

	if (strlen(s->Phone.Data.Version)>0) return ERR_NONE;

	smprintf(s, "Getting firmware version\n");
	error = GSM_WaitFor (s, req, 2, 0x01, 2, ID_GetFirmware);
	if (error==ERR_NONE) {
		smprintf_level(s, D_TEXT, "[Firmware version - \"%s\"]\n",
				s->Phone.Data.Version);
		smprintf_level(s, D_TEXT, "[Firmware date    - \"%s\"]\n",
				s->Phone.Data.VerDate);
	}
	return error;
}

static GSM_Error GNAPGEN_Initialise (GSM_StateMachine *s)
{
	GSM_Phone_GNAPGENData           *Priv = &s->Phone.Data.Priv.GNAPGEN;
	GSM_Error			error;

	error = GNAPGEN_GetManufacturer(s);
	if (error != ERR_NONE) return error;

	if (Priv->GNAPPLETVer == 18) return ERR_NONE;
	return ERR_GNAPPLETWRONG;
}

static GSM_Reply_Function GNAPGENReplyFunctions[] = {
	/* informations */
	{GNAPGEN_ReplyGetIMEI,		"\x01",0x01,0x02,ID_GetIMEI			},
	{GNAPGEN_ReplyGetHW,		"\x01",0x01,0x02,ID_GetHardware			},
	{GNAPGEN_ReplyGetManufacturer,	"\x01",0x01,0x02,ID_GetManufacturer		},
	{GNAPGEN_ReplyGetID,		"\x01",0x01,0x02,ID_GetModel			},
	{GNAPGEN_ReplyGetID,		"\x01",0x01,0x02,ID_GetFirmware			},

	{GNAPGEN_ReplyGetMemory,	"\x02",0x01,0x02,ID_GetMemory			},
	{GNAPGEN_ReplyGetMemoryStatus,	"\x02",0x01,0x08,ID_GetMemoryStatus		},
	{GNAPGEN_ReplyDeleteMemory,	"\x02",0x01,0x06,ID_DeleteMemory		},
	{GNAPGEN_ReplySetMemory,	"\x02",0x01,0x04,ID_SetMemory			},
	{GNAPGEN_ReplyDialVoice,	"\x02",0x01,10  ,ID_DialVoice			},
	{GNAPGEN_ReplyGetNextMemory,	"\x02",0x01,12  ,ID_GetMemory			},

	{GNAPGEN_ReplyGetNetworkInfo,	"\x03",0x01,0x02,ID_GetNetworkInfo		},
	{GNAPGEN_ReplyGetSignalQuality,	"\x03",0x01,0x04,ID_GetSignalQuality		},

	{GNAPGEN_ReplyGetBatteryCharge,	"\x04",0x01,0x02,ID_GetBatteryCharge		},

	/* type 5 is DEBUG */
	{GNAPGEN_ReplyGetModelFirmware,	"\x05",0x01,0x02,ID_IncomingFrame		},

	/* type 6 is SMS */
	{GNAPGEN_ReplyGetSMSFolders,	"\x06",0x01,0x02,ID_GetSMSFolders		},
	{GNAPGEN_ReplyGetSMSStatus,	"\x06",0x01,0x0A,ID_GetSMSStatus		},
	{GNAPGEN_ReplyGetSMSC,		"\x06",0x01,0x16,ID_GetSMSC			},
	{GNAPGEN_ReplyGetSMSFolderStatus,"\x06",0x01,0x04,ID_GetSMSFolderStatus		},
	{GNAPGEN_ReplyDeleteSMSMessage,	"\x06",0x01,18,ID_DeleteSMSMessage		},
	{GNAPGEN_ReplyGetSMS,		"\x06",0x01,12,ID_GetSMSMessage			},
	{GNAPGEN_ReplySetSMS,		"\x06",0x01,14,ID_SaveSMSMessage		},

	/* calendar */
	{GNAPGEN_ReplyGetNextCalendar,	"\x07",0x01,0x02,ID_GetCalendarNote		},
	{NONEFUNCTION,			"\x07",0x01,0x06,ID_DeleteCalendarNote		},
	{GNAPGEN_ReplyGetToDo,		"\x07",0x01,0x08,ID_GetToDo			},

	/* time */
	{GNAPGEN_ReplyGetDateTime,	"\x08",0x01,0x02,ID_GetDateTime			},
	{GNAPGEN_ReplyGetAlarm,		"\x08",0x01,0x06,ID_GetAlarm			},

	{NULL,				"\x00",0x00,0x00,ID_None			}
};

GSM_Phone_Functions GNAPGENPhone = {
	"gnap",
	GNAPGENReplyFunctions,
	GNAPGEN_Initialise,
	NONEFUNCTION,			/*	Terminate 		*/
	GSM_DispatchMessage,
	NOTSUPPORTED,			/* 	ShowStartInfo		*/
	GNAPGEN_GetManufacturer,
	GNAPGEN_GetModel,
	GNAPGEN_GetFirmware,
	GNAPGEN_GetIMEI,
	NOTSUPPORTED,			/*	GetOriginalIMEI		*/
	NOTSUPPORTED,			/*	GetManufactureMonth	*/
	NOTSUPPORTED,			/*	GetProductCode		*/
	GNAPGEN_GetHW,
	NOTSUPPORTED,			/*	GetPPM			*/
	NOTSUPPORTED,			/*	GetSIMIMSI		*/
	GNAPGEN_GetDateTime,
	NOTSUPPORTED,			/*	SetDateTime		*/
	GNAPGEN_GetAlarm,
	NOTSUPPORTED,			/*	SetAlarm		*/
	NOTSUPPORTED,			/* 	GetLocale		*/
	NOTSUPPORTED,			/* 	SetLocale		*/
	NOTSUPPORTED,			/*	PressKey		*/
	NOTSUPPORTED,			/*	Reset			*/
	NOTSUPPORTED,			/*	ResetPhoneSettings	*/
	NOTSUPPORTED,			/*	EnterSecurityCode	*/
	NOTSUPPORTED,			/*	GetSecurityStatus	*/
	NOTSUPPORTED,			/*	GetDisplayStatus	*/
	NOTSUPPORTED,			/*	SetAutoNetworkLogin	*/
	GNAPGEN_GetBatteryCharge,
	GNAPGEN_GetSignalQuality,
	GNAPGEN_GetNetworkInfo,
	NOTSUPPORTED,     		/*  	GetCategory 		*/
 	NOTSUPPORTED,       		/*  	AddCategory 		*/
        NOTSUPPORTED,      		/*  	GetCategoryStatus 	*/
	GNAPGEN_GetMemoryStatus,
	GNAPGEN_GetMemory,
	GNAPGEN_GetNextMemory,
	GNAPGEN_SetMemory,
	GNAPGEN_AddMemory,
	GNAPGEN_DeleteMemory,
	NOTIMPLEMENTED,			/*	DeleteAllMemory		*/
	NOTSUPPORTED,			/*	GetSpeedDial		*/
	NOTSUPPORTED,			/*	SetSpeedDial		*/
	GNAPGEN_GetSMSC,
	NOTSUPPORTED,			/*	SetSMSC			*/
	GNAPGEN_GetSMSStatus,
	NOTSUPPORTED,			/*	GetSMS			*/
	GNAPGEN_GetNextSMS,
	NOTSUPPORTED,			/*	SetSMS			*/
	GNAPGEN_AddSMS,
	GNAPGEN_DeleteSMSMessage,
	GNAPGEN_SendSMSMessage,
	NOTSUPPORTED,			/*	SendSavedSMS		*/
	NOTSUPPORTED,			/*	SetFastSMSSending	*/
	NOTSUPPORTED,			/*	SetIncomingSMS		*/
	NOTSUPPORTED,			/* 	SetIncomingCB		*/
	GNAPGEN_GetSMSFolders,
 	NOTSUPPORTED,			/* 	AddSMSFolder		*/
 	NOTSUPPORTED,			/* 	DeleteSMSFolder		*/
	GNAPGEN_DialVoice,		/*	DialVoice		*/
        NOTIMPLEMENTED,			/*	DialService		*/
	NOTSUPPORTED,			/*	AnswerCall		*/
	NOTSUPPORTED,			/*	CancelCall		*/
 	NOTSUPPORTED,			/* 	HoldCall 		*/
 	NOTSUPPORTED,			/* 	UnholdCall 		*/
 	NOTSUPPORTED,			/* 	ConferenceCall 		*/
 	NOTSUPPORTED,			/* 	SplitCall		*/
 	NOTSUPPORTED,			/* 	TransferCall		*/
 	NOTSUPPORTED,			/* 	SwitchCall		*/
 	NOTSUPPORTED,			/* 	GetCallDivert		*/
 	NOTSUPPORTED,			/* 	SetCallDivert		*/
 	NOTSUPPORTED,			/* 	CancelAllDiverts	*/
	NOTSUPPORTED,			/*	SetIncomingCall		*/
	NOTSUPPORTED,			/*  	SetIncomingUSSD		*/
	NOTSUPPORTED,			/*	SendDTMF		*/
	NOTSUPPORTED,			/*	GetRingtone		*/
	NOTSUPPORTED,			/*	SetRingtone		*/
	NOTSUPPORTED,			/*	GetRingtonesInfo	*/
	NOTSUPPORTED,			/* 	DeleteUserRingtones	*/
	NOTSUPPORTED,			/*	PlayTone		*/
	NOTSUPPORTED,			/*	GetWAPBookmark		*/
	NOTSUPPORTED,			/* 	SetWAPBookmark 		*/
	NOTSUPPORTED, 			/* 	DeleteWAPBookmark 	*/
	NOTSUPPORTED,			/* 	GetWAPSettings 		*/
	NOTSUPPORTED,			/* 	SetWAPSettings 		*/
	NOTSUPPORTED,			/*	GetSyncMLSettings	*/
	NOTSUPPORTED,			/*	SetSyncMLSettings	*/
	NOTSUPPORTED,			/*	GetChatSettings		*/
	NOTSUPPORTED,			/*	SetChatSettings		*/
	NOTSUPPORTED,			/* 	GetMMSSettings		*/
	NOTSUPPORTED,			/* 	SetMMSSettings		*/
	NOTSUPPORTED,			/*	GetMMSFolders		*/
	NOTSUPPORTED,			/*	GetNextMMSFileInfo	*/
	NOTSUPPORTED,			/*	GetBitmap		*/
	NOTSUPPORTED,			/*	SetBitmap		*/
	NOTSUPPORTED,			/*	GetToDoStatus		*/
	NOTSUPPORTED,			/*	GetToDo			*/
	GNAPGEN_GetNextToDo,
	NOTSUPPORTED,			/*	SetToDo			*/
	NOTSUPPORTED,			/*	AddToDo			*/
	NOTSUPPORTED,			/*	DeleteToDo		*/
	NOTSUPPORTED,			/*	DeleteAllToDo		*/
	NOTSUPPORTED,			/*	GetCalendarStatus	*/
	NOTSUPPORTED,			/*	GetCalendar		*/
    	GNAPGEN_GetNextCalendar,
	NOTSUPPORTED,			/*	SetCalendar		*/
	GNAPGEN_AddCalendar,
	GNAPGEN_DeleteCalendar,
	NOTSUPPORTED,			/*	DeleteAllCalendar	*/
	NOTSUPPORTED,			/* 	GetCalendarSettings	*/
	NOTSUPPORTED,			/* 	SetCalendarSettings	*/
	NOTSUPPORTED,			/*	GetNoteStatus		*/
	NOTSUPPORTED,			/*	GetNote			*/
	NOTSUPPORTED,			/*	GetNextNote		*/
	NOTSUPPORTED,			/*	SetNote			*/
	NOTSUPPORTED,			/*	AddNote			*/
	NOTSUPPORTED,			/* 	DeleteNote		*/
	NOTSUPPORTED,			/*	DeleteAllNotes		*/
	NOTSUPPORTED, 			/*	GetProfile		*/
	NOTSUPPORTED, 			/*	SetProfile		*/
    	NOTSUPPORTED,			/*  	GetFMStation        	*/
    	NOTSUPPORTED,			/*  	SetFMStation        	*/
	NOTSUPPORTED,			/* 	ClearFMStations		*/
	NOTSUPPORTED,			/* 	GetNextFileFolder	*/
	NOTSUPPORTED,			/*	GetFolderListing	*/
	NOTSUPPORTED,			/*	GetNextRootFolder	*/
	NOTSUPPORTED,			/*	SetFileAttributes	*/
	NOTSUPPORTED,			/* 	GetFilePart		*/
	NOTSUPPORTED,			/* 	AddFilePart		*/
	NOTSUPPORTED,			/* 	SendFilePart		*/
	NOTSUPPORTED, 			/* 	GetFileSystemStatus	*/
	NOTSUPPORTED,			/* 	DeleteFile		*/
	NOTSUPPORTED,			/* 	AddFolder		*/
	NOTSUPPORTED,			/* 	DeleteFolder		*/
	NOTSUPPORTED,			/* 	GetGPRSAccessPoint	*/
	NOTSUPPORTED			/* 	SetGPRSAccessPoint	*/
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
