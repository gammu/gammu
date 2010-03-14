
#include "../../../gsmstate.h"

#ifdef GSM_ENABLE_NOKIA6110

#include <string.h>

#include "../../../gsmcomon.h"
#include "../../../misc/coding.h"
#include "../../../service/gsmsms.h"
#include "../../pfunc.h"
#include "../nfunc.h"
#include "n6110.h"
#include "dct3func.h"

static unsigned char N6110_MEMORY_TYPES[] = {
	GMT_ME, 0x02,
	GMT_SM, 0x03,
	GMT_ON, 0x05,
	GMT_DC, 0x07,
	GMT_RC, 0x08,
	GMT_MC, 0x09,
	GMT_VM, 0x0b,
	  0x00, 0x00
};

#ifdef GSM_ENABLE_6110_AUTHENTICATION

/* This function provides Nokia authentication protocol.
 * This code is written specially for Gnokii project by Odinokov Serge.
 * If you have some special requests for Serge just write him to
 * apskaita@post.omnitel.net or serge@takas.lt
 * Reimplemented in C by Pavel Janík ml.
 * Nokia authentication protocol is used in the communication between Nokia
 * mobile phones (e.g. Nokia 6110) and Nokia Cellular Data Suite software,
 * commercially sold by Nokia Corp.
 * The authentication scheme is based on the token send by the phone to the
 * software. The software does it's magic (see the function
 * N6110_GetNokiaAuthentication) and returns the result back to the phone.
 * If the result is correct the phone responds with the message "Accessory
 * connected!" displayed on the LCD. Otherwise it will display "Accessory not
 * supported" and some functions will not be available for use (?).
 * The specification of the protocol is not publicly available, no comment.
 */
static void N6110_GetNokiaAuthentication(unsigned char *Imei, unsigned char *MagicBytes, unsigned char *MagicResponse)
{
	int 		i, j, CRC=0;
	unsigned char 	Temp[16];	/* This is our temporary working area. */

	/* Here we put FAC (Final Assembly Code) and serial number into our area. */
	Temp[0]  = Imei[6];		Temp[1]  = Imei[7];
	Temp[2]  = Imei[8];		Temp[3]  = Imei[9];
	Temp[4]  = Imei[10];		Temp[5]  = Imei[11];
	Temp[6]  = Imei[12];		Temp[7]  = Imei[13];

	/* And now the TAC (Type Approval Code). */
	Temp[8]  = Imei[2];		Temp[9]  = Imei[3];
	Temp[10] = Imei[4];		Temp[11] = Imei[5];

	/* And now we pack magic bytes from the phone. */
	Temp[12] = MagicBytes[0];	Temp[13] = MagicBytes[1];
	Temp[14] = MagicBytes[2];	Temp[15] = MagicBytes[3];

	for (i=0; i<=11; i++) if (Temp[i + 1]& 1) Temp[i]<<=1;
	switch (Temp[15] & 0x03) {
		case 1:
		case 2: 	j = Temp[13] & 0x07;
				for (i=0; i<=3; i++) Temp[i+j] ^= Temp[i+12];
				break;
		default: 	j = Temp[14] & 0x07;
				for (i=0; i<=3; i++) Temp[i + j] |= Temp[i + 12];
	}
	for (i=0; i<=15; i++) CRC ^= Temp[i];
	for (i=0; i<=15; i++) {
		switch (Temp[15 - i] & 0x06) {
			case 0: j = Temp[i] | CRC; break;
			case 2:
			case 4: j = Temp[i] ^ CRC; break;
			case 6: j = Temp[i] & CRC; break;
		}
		if (j == CRC) j = 0x2c;
		if (Temp[i] == 0) j = 0;
		MagicResponse[i] = j;
	}
}

static GSM_Error N6110_ReplyGetMagicBytes(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	GSM_Phone_N6110Data *Priv = &Data->Priv.N6110;

	sprintf(Data->IMEICache, "%s", msg.Buffer+9);
	sprintf(Data->HardwareCache, "%s", msg.Buffer+39);
	sprintf(Data->ProductCodeCache, "%s", msg.Buffer+31);

	dprintf("Message: Mobile phone identification received:\n");
	dprintf("   IMEI            : %s\n", msg.Buffer+9);
	dprintf("   Model           : %s\n", msg.Buffer+25);
	dprintf("   Production Code : %s\n", msg.Buffer+31);
	dprintf("   HW              : %s\n", msg.Buffer+39);
	dprintf("   Firmware        : %s\n", msg.Buffer+44);

	/* These bytes are probably the source of the "Accessory not connected"
	 * messages on the phone when trying to emulate NCDS... I hope....
	 * UPDATE: of course, now we have the authentication algorithm.
	 */
	dprintf("   Magic bytes: %02x %02x %02x %02x\n", msg.Buffer[50], msg.Buffer[51], msg.Buffer[52], msg.Buffer[53]);

	Priv->MagicBytes[0]=msg.Buffer[50];
	Priv->MagicBytes[1]=msg.Buffer[51];
	Priv->MagicBytes[2]=msg.Buffer[52];
	Priv->MagicBytes[3]=msg.Buffer[53];

	return GE_NONE;
}

static GSM_Error N6110_MakeAuthentication(GSM_StateMachine *s)
{
	GSM_Phone_N6110Data 	*Priv = &s->Phone.Data.Priv.N6110;
	GSM_Error		error;
	unsigned char 		connect4[] = {N6110_FRAME_HEADER, 0x10};
	unsigned char 		magic_connect[] = {
		N6110_FRAME_HEADER, 0x12,
		/* The real magic goes here ... These bytes are filled in
		 * with the function N6110_GetNokiaAuthentication. */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		/* NOKIA&GNOKII Accessory */
		 'N', 'O', 'K', 'I', 'A', '&', 'N', 'O', 'K', 'I', 'A',
		 'a', 'c', 'c', 'e', 's', 's', 'o', 'r', 'y',
  		0x00, 0x00, 0x00, 0x00};

	dprintf("Getting magic bytes for authentication\n");
	error=GSM_WaitFor (s, connect4, 4, 0x64, 4, ID_MakeAuthentication);
	if (error!=GE_NONE) return error;

	N6110_GetNokiaAuthentication(s->Phone.Data.IMEICache, Priv->MagicBytes, magic_connect+4);
	dprintf("Sending authentication bytes\n");
	return s->Protocol.Functions->WriteMessage(s, magic_connect, 45, 0x64);
}

#endif

static GSM_Error N6110_Initialise (GSM_StateMachine *s)
{
	GSM_Error error=GE_NONE;

#ifdef GSM_ENABLE_6110_AUTHENTICATION
	if (IsPhoneFeatureAvailable(s->Phone.Data.Model, F_MAGICBYTES)) {
		if (s->connectiontype == GCT_FBUS2 ||
		    s->connectiontype == GCT_INFRARED) {
			error=N6110_MakeAuthentication(s);
			if (error!=GE_NONE) return error;
		}
	}
#endif
#ifdef DEBUG
	error=DCT3_SetIncomingCB(s,true);
	if (error!=GE_NONE) return error;
#endif

	return GE_NONE;
}

static GSM_Error N6110_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	return DCT3_GetDateTime(s, date_time, 0x11);
}

static GSM_Error N6110_GetAlarm(GSM_StateMachine *s, GSM_DateTime *alarm, int alarm_number)
{
	return DCT3_GetAlarm(s, alarm, alarm_number, 0x11);
}

static GSM_Error N6110_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	return DCT3_SetDateTime(s, date_time, 0x11);
}

static GSM_Error N6110_SetAlarm(GSM_StateMachine *s, GSM_DateTime *alarm, int alarm_number)
{
	return DCT3_SetAlarm(s, alarm, alarm_number, 0x11);
}

static GSM_Error N6110_ReplyGetMemory(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int count;

	dprintf("Phonebook entry received\n");
	switch (msg.Buffer[3]) {
	case 0x02:
		Data->Memory->EntriesNum = 0;
		count=5;
		/* If name is not empty */
		if (msg.Buffer[count]!=0x00) {
			if (msg.Buffer[count]>GSM_PHONEBOOK_TEXT_LENGTH) {
				dprintf("Too long text\n");
				return GE_UNKNOWNRESPONSE;
			}
			Data->Memory->Entries[Data->Memory->EntriesNum].EntryType=PBK_Name;
			if (IsPhoneFeatureAvailable(Data->Model, F_NOPBKUNICODE)) {
				EncodeUnicode(Data->Memory->Entries[Data->Memory->EntriesNum].Text,
					msg.Buffer+count+1,msg.Buffer[count]);
			} else {
				memcpy(Data->Memory->Entries[Data->Memory->EntriesNum].Text,
					msg.Buffer+count+1,msg.Buffer[count]);
				Data->Memory->Entries[Data->Memory->EntriesNum].Text[msg.Buffer[count]]=0x00;
				Data->Memory->Entries[Data->Memory->EntriesNum].Text[msg.Buffer[count]+1]=0x00;
			}
			dprintf("   Name \"%s\"\n",
				DecodeUnicodeString(Data->Memory->Entries[Data->Memory->EntriesNum].Text));
			Data->Memory->EntriesNum++;
		}
		count=count+msg.Buffer[count]+1;

		/* If number is empty */
		if (msg.Buffer[count]==0x00) return GE_NONE;

		if (msg.Buffer[count]>GSM_PHONEBOOK_TEXT_LENGTH) {
			dprintf("Too long text\n");
			return GE_UNKNOWNRESPONSE;
		}
		Data->Memory->Entries[Data->Memory->EntriesNum].EntryType = PBK_Number_General;
		Data->Memory->Entries[Data->Memory->EntriesNum].VoiceTag  = 0;
		EncodeUnicode(Data->Memory->Entries[Data->Memory->EntriesNum].Text,
			msg.Buffer+count+1,msg.Buffer[count]);
		dprintf("   Number \"%s\"\n",
			DecodeUnicodeString(Data->Memory->Entries[Data->Memory->EntriesNum].Text));
		Data->Memory->EntriesNum++;
		count=count+msg.Buffer[count]+1;

		if (msg.Buffer[count]>0 && msg.Buffer[count]<6) {
			Data->Memory->Entries[Data->Memory->EntriesNum].EntryType=PBK_Caller_Group;
			dprintf("Caller group \"%i\"\n",msg.Buffer[count]);
			Data->Memory->Entries[Data->Memory->EntriesNum].Number=msg.Buffer[count];
			Data->Memory->EntriesNum++;
		}
		count++;

		if (Data->Memory->MemoryType==GMT_DC ||
		    Data->Memory->MemoryType==GMT_RC ||
		    Data->Memory->MemoryType==GMT_MC)
		{
		        NOKIA_DecodeDateTime(msg.Buffer+count+1,&Data->Memory->Entries[Data->Memory->EntriesNum].Date);
			Data->Memory->Entries[Data->Memory->EntriesNum].EntryType=PBK_Date;

			/* These values are set, when date and time unavailable in phone.
			 * Values from 3310 - in other can be different */
			if (Data->Memory->Entries[2].Date.Day   !=20  ||
			    Data->Memory->Entries[2].Date.Month !=1   ||
			    Data->Memory->Entries[2].Date.Year  !=2118||
			    Data->Memory->Entries[2].Date.Hour  !=3	 ||
			    Data->Memory->Entries[2].Date.Minute!=14  ||
			    Data->Memory->Entries[2].Date.Second!=7)
				Data->Memory->EntriesNum++;
		}		

		return GE_NONE;
	default:
		switch (msg.Buffer[4]) {
			case 0x74:
				/* TODO: check if not too high */
				dprintf("ERROR: Empty ????\n");
				Data->Memory->EntriesNum = 0;
				return GE_NONE;
			case 0x7d:
				dprintf("ERROR: Invalid memory type\n");
				return GE_NOTSUPPORTED;
			case 0x8d:
				dprintf("ERROR: no PIN\n");
				return GE_SECURITYERROR;
			default:
				dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
		}
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_GetMemory (GSM_StateMachine *s, GSM_PhonebookEntry *entry)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x01,
		0x00,		/* memory type */
		0x00,		/* location */
		0x00};

	req[4] = NOKIA_GetMemoryType(entry->MemoryType,N6110_MEMORY_TYPES);
	if (req[4]==0xff) return GE_NOTSUPPORTED;

	req[5] = entry->Location;

	s->Phone.Data.Memory=entry;
	dprintf("Getting phonebook entry\n");
	return GSM_WaitFor (s, req, 7, 0x03, 4, ID_GetMemory);
}

static GSM_Error N6110_ReplyGetMemoryStatus(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Memory status received\n");
	switch (msg.Buffer[3]) {
	case 0x08:
		dprintf("   Memory type: %i\n",msg.Buffer[4]);
		dprintf("   Free       : %i\n",msg.Buffer[5]);
		Data->MemoryStatus->Free=msg.Buffer[5];
		dprintf("   Used       : %i\n",msg.Buffer[6]);
		Data->MemoryStatus->Used=msg.Buffer[6];
		return GE_NONE;
		break;
	case 0x09:
		switch (msg.Buffer[4]) {
		case 0x6f:
			dprintf("Phone is probably powered off.\n");
			return GE_TIMEOUT;
		case 0x7d:
			dprintf("Memory type not supported by phone model.\n");
			return GE_NOTSUPPORTED;
		case 0x8d:
			dprintf("Waiting for security code.\n");
			return GE_SECURITYERROR;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
		}
	default:
		return GE_UNKNOWNRESPONSE;
	}
}

static GSM_Error N6110_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x07,
		0x00};		/* MemoryType */

	req[4] = NOKIA_GetMemoryType(Status->MemoryType,N6110_MEMORY_TYPES);
	if (req[4]==0xff) return GE_NOTSUPPORTED;

	s->Phone.Data.MemoryStatus=Status;
	dprintf("Getting memory status\n");
	return GSM_WaitFor (s, req, 5, 0x03, 4, ID_GetMemoryStatus);
}

static GSM_Error N6110_ReplyGetSMSStatus(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("SMS status received\n");
	switch (msg.Buffer[3]) {
	case 0x37:
		dprintf("SIM size               : %i\n",msg.Buffer[7]);
		dprintf("Used in SIM            : %i\n",msg.Buffer[10]);
		dprintf("Unread in SIM          : %i\n",msg.Buffer[11]);
		Data->SMSStatus->SIMUsed 	= msg.Buffer[10];
		Data->SMSStatus->SIMUnRead 	= msg.Buffer[11];
		Data->SMSStatus->SIMSize	= msg.Buffer[7];
		Data->SMSStatus->PhoneUsed	= 0;
		Data->SMSStatus->PhoneUnRead 	= 0;
		Data->SMSStatus->PhoneSize	= 0;
		Data->SMSStatus->TemplatesUsed	= 0;
		return GE_NONE;
	case 0x38:
		dprintf("Error. No PIN ?\n");
		return GE_SECURITYERROR;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_ReplyGetSMSMessage(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("SMS Message received\n");
	switch(msg.Buffer[3]) {
	case 0x08:
		Data->GetSMSMessage->Number 	 	= 1;
		Data->GetSMSMessage->SMS[0].Name[0] 	= 0;
		Data->GetSMSMessage->SMS[0].Name[1] 	= 0;
		NOKIA_DecodeSMSState(msg.Buffer[4], &Data->GetSMSMessage->SMS[0]);
		switch (msg.Buffer[7]) {
		case 0x00: case 0x01:   /* Report or SMS_Deliver */
			Data->GetSMSMessage->SMS[0].Folder	= 0x01;
			Data->GetSMSMessage->SMS[0].InboxFolder = true;
			break;
		case 0x02:		/* SMS_Submit */
			Data->GetSMSMessage->SMS[0].Folder	= 0x02;
			Data->GetSMSMessage->SMS[0].InboxFolder = false;
			break;
		default:
			return GE_UNKNOWNRESPONSE;
		}
		DCT3_DecodeSMSFrame(&Data->GetSMSMessage->SMS[0],msg.Buffer+8);
		return GE_NONE;
	case 0x09:
		switch (msg.Buffer[4]) {
		case 0x02:
			dprintf("Too high location ?\n");
			return GE_INVALIDLOCATION;
		case 0x07:
			dprintf("Empty\n");
			return GE_EMPTY;
		case 0x0c:
			dprintf("Access error. No PIN ?\n");
			return GE_SECURITYERROR;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
		}
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_GetSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x07, 0x02,
                0x00,		/* Location */
                0x01, 0x64};

	if (sms->SMS[0].Folder!=0x00) return GE_NOTSUPPORTED;

	req[5] = sms->SMS[0].Location;

	s->Phone.Data.GetSMSMessage=sms;
	dprintf("Getting sms\n");
	return GSM_WaitFor (s, req, 8, 0x02, 4, ID_GetSMSMessage);
}

static GSM_Error N6110_GetNextSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	GSM_Phone_N6110Data 	*Priv = &s->Phone.Data.Priv.N6110;
	GSM_Error 		error;

	if (start) {
		error=s->Phone.Functions->GetSMSStatus(s,&Priv->LastSMSStatus);
		if (error!=GE_NONE) return error;
		Priv->LastSMSRead=0;
		sms->SMS[0].Location=0;
	}
	while (true) {
		sms->SMS[0].Location++;
		if (Priv->LastSMSRead>=(Priv->LastSMSStatus.SIMUsed+Priv->LastSMSStatus.PhoneUsed+Priv->LastSMSStatus.TemplatesUsed)) return GE_EMPTY;
		error=s->Phone.Functions->GetSMSMessage(s, sms);
		if (error==GE_NONE) {
			Priv->LastSMSRead++;
			break;
		}
		if (error != GE_EMPTY) return error;
	}
	return error;
}

static GSM_Error N6110_ReplyGetStatus(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
#ifdef DEBUG
	dprintf("Phone status received :\n");
	dprintf("Mode                  : ");
	switch (msg.Buffer[4]) {
		case 0x01: dprintf("registered within the network\n");	break;
		case 0x02: dprintf("call in progress\n");		break; /* ringing or already answered call */
		case 0x03: dprintf("waiting for security code\n");	break;
		case 0x04: dprintf("powered off\n");			break;
		default  : dprintf("unknown\n");
	}
	dprintf("Power source          : ");
	switch (msg.Buffer[7]) {
		case 0x01: dprintf("AC/DC\n");		break;
		case 0x02: dprintf("battery\n");	break;
		default  : dprintf("unknown\n");
	}
	dprintf("Battery Level         : %d\n", msg.Buffer[8]);
	dprintf("Signal strength       : %d\n", msg.Buffer[5]);
#endif

	switch (Data->RequestID) {
	case ID_GetBatteryLevel:
		*Data->BatteryLevel=((int)msg.Buffer[8])*25;
		return GE_NONE;
	case ID_GetNetworkLevel:
		*Data->NetworkLevel=((int)msg.Buffer[5])*25;
		return GE_NONE;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_GetStatus(GSM_StateMachine *s, int ID)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x01};
	return GSM_WaitFor (s, req, 4, 0x04, 4, ID);
}

static GSM_Error N6110_GetNetworkLevel(GSM_StateMachine *s, int *level)
{
	char 		value[100];
	GSM_Error 	error;

	dprintf("Getting network level\n");
	if (IsPhoneFeatureAvailable(s->Model, F_POWER_BATT)) {
		error = DCT3_Netmonitor(s, 1, value);
		if (error!=GE_NONE) return error;
		*level = 100;
		if (value[4]!='-') {
			if (value[5]=='9' && value[6]>'4') *level = 25;
			if (value[5]=='9' && value[6]<'5') *level = 50;
			if (value[5]=='8' && value[6]>'4') *level = 75;      
		} else *level = 0;
		return GE_NONE;
	} else {
		s->Phone.Data.NetworkLevel=level;
		return N6110_GetStatus(s, ID_GetNetworkLevel);
	}
}

static GSM_Error N6110_GetBatteryLevel(GSM_StateMachine *s, int *level)
{
	char 		value[100];
	GSM_Error 	error;

	dprintf("Getting battery level\n");
	if (IsPhoneFeatureAvailable(s->Model, F_POWER_BATT)) {
		error = DCT3_Netmonitor(s, 23, value);
		if (error!=GE_NONE) return error;
		*level = 100;
		if (value[29]=='7') *level = 75;
		if (value[29]=='5') *level = 50;
		if (value[29]=='2') *level = 25;
		return GE_NONE;
	} else {
		s->Phone.Data.BatteryLevel=level;
		return N6110_GetStatus(s, ID_GetBatteryLevel);
	}
}

static GSM_Error N6110_ReplySaveSMSMessage(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("SMS message saving status\n");
	switch (msg.Buffer[3]) {
	case 0x05:
		dprintf("Saved at location %i\n",msg.Buffer[5]);
		Data->SaveSMSMessage->Location=msg.Buffer[5];
		return GE_NONE;
	case 0x06:
		switch (msg.Buffer[4]) {
		case 0x02:
			dprintf("All locations busy\n");
			return GE_FULL;
		case 0x03:
			dprintf("Too high ?\n");
			return GE_INVALIDLOCATION;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
		}
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_SaveSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	int			length;
	GSM_Error		error;
	unsigned char req[256] = {
		N6110_FRAME_HEADER, 0x04,
		0x00,		/* SMS State */
		0x02,
		0x00,		/* SMS Location */
		0x02};		/* SMS Type */

	req[6] = sms->Location;
	if (sms->Folder==1) {			/* Inbox */
		req[4] 		= 1;		/* SMS State - GSM_Read */
		req[7] 		= 0x00;		/* SMS Type */
		sms->PDU 	= SMS_Deliver;
		error=PHONE_EncodeSMSFrame(s,sms,req+8,PHONE_SMSDeliver,&length,true);
	} else {                                /* Outbox */
		req[4] 		= 5;		/* SMS State - GSM_Sent */
		req[7] 		= 0x02;		/* SMS Type */
		sms->PDU 	= SMS_Submit;
		error=PHONE_EncodeSMSFrame(s,sms,req+8,PHONE_SMSSubmit,&length,true);
	}
	if (error != GE_NONE) return error;

	/* SMS State - GSM_Read -> GSM_Unread and GSM_Sent -> GSM_UnSent */
  	if (sms->State == GSM_UnSent || sms->State == GSM_UnRead) req[4] |= 0x02;

	s->Phone.Data.SaveSMSMessage=sms;
	dprintf("Saving sms\n");
	return GSM_WaitFor (s, req, 8+length, 0x14, 4, ID_SaveSMSMessage);
}

static GSM_Error N6110_ReplySetRingtone(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x37:
		dprintf("Ringtone set OK\n");
		return GE_NONE;
		break;
	case 0x38:
		dprintf("Error setting ringtone\n");
		switch (msg.Buffer[4]) {
		case 0x7d:
			dprintf("Too high location ?\n");
			return GE_INVALIDLOCATION;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
		}
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_ReplySetBinRingtone(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[4]) {
	case 0x00:
		dprintf("Set at location %i\n",msg.Buffer[3]+1);
		return GE_NONE;
	default:
		dprintf("Invalid location. Too high ?\n");
		return GE_INVALIDLOCATION;
	}
}

static GSM_Error N6110_SetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, int *maxlength)
{
	GSM_NetworkInfo	NetInfo;
	GSM_Error	error;
	int		size=200,current=8;
	GSM_UDHHeader 	UDHHeader;
	unsigned char 	req[1000] = {
		N6110_FRAME_HEADER, 0x36,
		0x00,		/* Location */
		0x00,0x78};
	unsigned char 	reqBin[1000] = {0x00,0x01,0xa0,0x00,0x00,0x0c,0x01,0x2c};

	if (IsPhoneFeatureAvailable(s->Model, F_NORING)) return GE_NOTSUPPORTED;
	if (Ringtone->Location == 0) return GE_INVALIDLOCATION;

	switch (Ringtone->Format) {
	case RING_NOTETONE:
		if (Ringtone->Location==255)
		{
			/* Only 6110, 6130 and 6150 support it */
			if (strcmp(s->Model,"NSE-3") == 0 || strcmp(s->Model,"NSK-3") == 0 ||
			    strcmp(s->Model,"NSM-1") == 0)
			{
				req[0] = 0x0c;
				req[1] = 0x01;
				UDHHeader.Type = UDH_NokiaRingtone;
				GSM_EncodeUDHHeader(&UDHHeader);
				/* We copy UDH now */
				memcpy(req+2,UDHHeader.Text,UDHHeader.Length);
				*maxlength=GSM_EncodeNokiaRTTLRingtone(*Ringtone, req+2+UDHHeader.Length, &size);
				error = s->Protocol.Functions->WriteMessage(s, req, 2+UDHHeader.Length+size, 0x12);
				if (error!=GE_NONE) return error;
				mili_sleep(1000);
				/* We have to make something (not important, what) now */
				/* no answer from phone*/
				return DCT3_GetNetworkInfo(s,&NetInfo);
			} else {
				return GE_NOTSUPPORTED;
			}
		}
		*maxlength=GSM_EncodeNokiaRTTLRingtone(*Ringtone, req+7, &size);
		req[4] = Ringtone->Location - 1;
		dprintf("Setting ringtone\n");
		return GSM_WaitFor (s, req, 7 + size, 0x05, 4, ID_SetRingtone);
	case RING_NOKIABINARY:
               	error=DCT3_EnableSecurity (s, 0x01);
		if (error!=GE_NONE) return error;
		memcpy(reqBin+current,DecodeUnicodeString(Ringtone->Name),strlen(DecodeUnicodeString(Ringtone->Name)));
		current += strlen(DecodeUnicodeString(Ringtone->Name));
		reqBin[current++] = 0x00;
		reqBin[current++] = 0x00;
		reqBin[current++] = 0x00;/*xxx*/
		memcpy(reqBin+current,Ringtone->NokiaBinary.Frame,Ringtone->NokiaBinary.Length);
		current=current+Ringtone->NokiaBinary.Length;
		reqBin[3]=Ringtone->Location-1;
		if (!strcmp(GetModelData(NULL,s->Model,NULL)->model,"3210")) reqBin[5]=0x10;
		dprintf("Setting binary ringtone\n");
		return GSM_WaitFor (s, reqBin, current, 0x40, 4, ID_SetRingtone);
	}
	return GE_NOTSUPPORTED;
}

static GSM_Error N6110_ReplyGetOpLogo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int count=5;

	dprintf("Operator logo received\n");
	NOKIA_DecodeNetworkCode(msg.Buffer+count,Data->Bitmap->NetworkCode);
	count = count + 3;
	dprintf("   Network code              : %s\n", Data->Bitmap->NetworkCode);
	dprintf("   Network name for Gammu    : %s ",
			DecodeUnicodeString(GSM_GetNetworkName(Data->Bitmap->NetworkCode)));
	dprintf("(%s)\n",DecodeUnicodeString(GSM_GetCountryName(Data->Bitmap->NetworkCode)));
	count = count + 3; 	/* We ignore size */
	Data->Bitmap->Width	= msg.Buffer[count++];
	Data->Bitmap->Height	= msg.Buffer[count++];
	count++;
	PHONE_DecodeBitmap(GSM_NokiaOperatorLogo,msg.Buffer+count,Data->Bitmap);
	return GE_NONE;
}

static GSM_Error N6110_ReplyGetStartup(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int i, count = 5;

	dprintf("Startup logo & notes received\n");
	for (i=0;i<msg.Buffer[4];i++) {
	switch (msg.Buffer[count++]) {
	case 0x01:
		dprintf("Startup logo\n");
		if (Data->Bitmap->Type == GSM_StartupLogo) {
			Data->Bitmap->Height	= msg.Buffer[count++];
			Data->Bitmap->Width	= msg.Buffer[count++];
			PHONE_DecodeBitmap(GSM_NokiaStartupLogo, msg.Buffer + count, Data->Bitmap);
		} else {
			count = count + 2;
		}
		count = count + PHONE_GetBitmapSize(GSM_NokiaStartupLogo);
		break;
	case 0x02:
		dprintf("Welcome note\n");
		if (Data->Bitmap->Type == GSM_WelcomeNoteText) {
			EncodeUnicode(Data->Bitmap->Text,msg.Buffer+count, msg.Buffer[count]);
			dprintf("Text is \"%s\"\n",Data->Bitmap->Text);
		}
		count = count + msg.Buffer[count] + 1;
		break;
	case 0x03:
		dprintf("Dealer welcome note\n");
		if (Data->Bitmap->Type == GSM_DealerNoteText) {
			EncodeUnicode(Data->Bitmap->Text,msg.Buffer+count, msg.Buffer[count]);
			dprintf("Text is \"%s\"\n",Data->Bitmap->Text);
		}
		count = count + msg.Buffer[count] + 1;
		break;
	default:
		dprintf("Unknown block\n");
		return GE_UNKNOWNRESPONSE;
		break;		
	}
	}
	return GE_NONE;
}

static GSM_Error N6110_ReplyGetCallerLogo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int count;

	switch (msg.Buffer[3]) {
	case 0x11:
		dprintf("Caller group info received\n");
	        EncodeUnicode(Data->Bitmap->Text,msg.Buffer+6,msg.Buffer[5]);
		dprintf("Name : \"%s\"\n",DecodeUnicodeString(Data->Bitmap->Text));
		count = msg.Buffer[5] + 6;
		Data->Bitmap->Ringtone = msg.Buffer[count++];
		dprintf("Ringtone ID: %02x\n",Data->Bitmap->Ringtone);
		Data->Bitmap->Enabled=(msg.Buffer[count++]==1);
#ifdef DEBUG	
		dprintf("Caller group logo ");
		if (Data->Bitmap->Enabled)
			dprintf("enabled\n");
		else
			dprintf("disabled\n");
#endif /* DEBUG */			
		count = count + 3; 	/* We ignore size */
		Data->Bitmap->Width	= msg.Buffer[count++];
		Data->Bitmap->Height	= msg.Buffer[count++];
		count++;
		PHONE_DecodeBitmap(GSM_NokiaCallerLogo,msg.Buffer+count,Data->Bitmap);
		return GE_NONE;
	case 0x12:
		dprintf("Error getting caller group info\n");
		return GE_INVALIDLOCATION;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_ReplyGetSetPicture(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int count = 5, i;

	switch (msg.Buffer[3]) {
	case 0x02:
		dprintf("Picture Image received\n");
		if (msg.Buffer[count]!=0) {
			GSM_UnpackSemiOctetNumber(Data->Bitmap->Sender, msg.Buffer + 5, true);
			/* Convert number of semioctets to number of chars */
			i = msg.Buffer[5];
			if (i % 2) i++;
			i=i / 2 + 1;
			count = count + i + 1;
		} else {
			Data->Bitmap->Sender[0] = 0x00;
			Data->Bitmap->Sender[1] = 0x00;
			count+=3;
		}
		dprintf("Sender : \"%s\"\n",DecodeUnicodeString(Data->Bitmap->Sender));
		count++;
		EncodeUnicode(Data->Bitmap->Text,msg.Buffer+count+1,msg.Buffer[count]);
		dprintf("Text   : \"%s\"\n",DecodeUnicodeString(Data->Bitmap->Text));
		count += strlen(DecodeUnicodeString(Data->Bitmap->Text)) + 1;
		Data->Bitmap->Width	= msg.Buffer[count++];
		Data->Bitmap->Height	= msg.Buffer[count++];
		PHONE_DecodeBitmap(GSM_NokiaPictureImage, msg.Buffer + count + 2, Data->Bitmap);
#ifdef DEBUG
		if (di.dl == DL_TEXTALL) GSM_PrintBitmap(di.df,Data->Bitmap);
#endif
		return GE_NONE;
		break;
	case 0x04:
		dprintf("Picture Image set OK\n");
		return GE_NONE;
	case 0x05:
		dprintf("Can't set Picture Image - invalid location ?\n");
		return GE_INVALIDLOCATION;
		break;	
	case 0x06:
		dprintf("Can't get Picture Image - invalid location ?\n");
		return GE_INVALIDLOCATION;
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_GetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	GSM_Error error;
	unsigned char req[10] = { N6110_FRAME_HEADER };

	s->Phone.Data.Bitmap=Bitmap;	
	switch (Bitmap->Type) {
	case GSM_StartupLogo:
	case GSM_WelcomeNoteText:
	case GSM_DealerNoteText:
		if (Bitmap->Type == GSM_StartupLogo && IsPhoneFeatureAvailable(s->Model,F_NOSTARTUP)) return GE_NOTSUPPORTED;
		req[3] = 0x16;
		return GSM_WaitFor (s, req, 4, 0x05, 4, ID_GetBitmap);
	case GSM_CallerLogo:
		if (IsPhoneFeatureAvailable(s->Model,F_NOCALLER)) return GE_NOTSUPPORTED;
		req[3] = 0x10;
		req[4] = Bitmap->Location - 1;
		error = GSM_WaitFor (s, req, 5, 0x03, 4, ID_GetBitmap);
		if (error==GE_NONE) NOKIA_GetDefaultCallerGroupName(s,Bitmap);
		return error;	
	case GSM_OperatorLogo:
		req[3] = 0x33;
		req[4] = 0x01;
		return GSM_WaitFor (s, req, 5, 0x05, 4, ID_GetBitmap);
	case GSM_PictureImage:
		if (IsPhoneFeatureAvailable(s->Model,F_NOPICTURE)) return GE_NOTSUPPORTED;
		req[3] = 0x01;
		req[4] = Bitmap->Location - 1;
		return GSM_WaitFor (s, req, 5, 0x47, 4, ID_GetBitmap);
	default:
		break;
	}
	return GE_NOTSUPPORTED;
}

static GSM_Error N6110_ReplySetProfileFeature(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x11:
		dprintf("Feature of profile set\n");
		return GE_NONE;
	case 0x12:
		dprintf("Error setting profile feature\n");
		return GE_NOTSUPPORTED;
	} 
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_SetProfileFeature(GSM_StateMachine *s, unsigned char profile, unsigned char feature, unsigned char value)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x10, 0x01,
		0x00,		/* Profile */
		0x00,		/* Feature */
		0x00};		/* Value   */

	req[5]=profile;
	req[6]=feature;
	req[7]=value;
	dprintf("Setting profile feature\n");
	return GSM_WaitFor (s, req, 8, 0x05, 4, ID_SetProfile);
}

static GSM_Error N6110_ReplySetStartup(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Startup logo set OK\n");
	return GE_NONE;
}

static GSM_Error N6110_ReplySetCallerLogo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x14:
		dprintf("Caller group set OK\n");
		return GE_NONE;
	case 0x15:
		dprintf("Error setting caller group\n");
		return GE_INVALIDLOCATION;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_SetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	unsigned char	reqPreview[1000] = {0x0c,0x01};
	unsigned char	req[600] = { N6110_FRAME_HEADER };
	GSM_UDH		UDHType = UDH_NokiaOperatorLogo;
	GSM_UDHHeader 	UDHHeader;
	GSM_NetworkInfo	NetInfo;
	GSM_Error	error;
	int		count = 0;
	int		textlen;
	int		Width, Height;

	switch (Bitmap->Type) {
	case GSM_CallerLogo:
	case GSM_OperatorLogo:
		if (Bitmap->Location == 255) {
			/* Only 6110, 6130 and 6150 support it */
			if (strcmp(s->Model,"NSE-3") == 0 || strcmp(s->Model,"NSK-3") == 0 ||
			    strcmp(s->Model,"NSM-1") == 0)
			{
				if (Bitmap->Type==GSM_CallerLogo) UDHType = UDH_NokiaCallerLogo;
				UDHHeader.Type = UDHType;
				GSM_EncodeUDHHeader(&UDHHeader);
				/* We copy UDH now */
				memcpy(reqPreview+2,UDHHeader.Text,UDHHeader.Length);
				count = count + UDHHeader.Length;
				if (Bitmap->Type == GSM_OperatorLogo) {
					NOKIA_EncodeNetworkCode(reqPreview+count,Bitmap->NetworkCode);
					count = count + 3;
				}
				NOKIA_CopyBitmap(GSM_NokiaOperatorLogo,Bitmap,reqPreview, &count);
				reqPreview[count]=0x00;
				error = s->Protocol.Functions->WriteMessage(s, reqPreview, count + 1, 0x12);
				if (error!=GE_NONE) return error;
				mili_sleep(1000);
				/* We have to make something (not important, what) now */
				/* no answer from phone*/
				return DCT3_GetNetworkInfo(s,&NetInfo);
			} else {
				dprintf("%s\n",s->Model);
				return GE_NOTSUPPORTED;
			}
		}
		break;
	default:
		break;
	}

	count = 3;

	switch (Bitmap->Type) {
	case GSM_StartupLogo:
		if (IsPhoneFeatureAvailable(s->Model,F_NOSTARTUP)) return GE_NOTSUPPORTED;
		if (Bitmap->Location != 1) {
			if (IsPhoneFeatureAvailable(s->Model,F_NOSTARTANI)) return GE_NOTSUPPORTED;
		}
		if (!IsPhoneFeatureAvailable(s->Model,F_NOSTARTANI)) {
			if (!strcmp(GetModelData(NULL,s->Model,NULL)->model,"3210")) {
				error = N6110_SetProfileFeature(s,0,0x2e,((unsigned char)(Bitmap->Location-1)));
			} else {
				error = N6110_SetProfileFeature(s,0,0x29,((unsigned char)(Bitmap->Location-1)));
			}
			if (error != GE_NONE) return error;
			if (Bitmap->Location != 1) return GE_NONE;
		}
		req[count++] = 0x18;
		req[count++] = 0x01;	/* One block	*/
		req[count++] = 0x01;
		PHONE_GetBitmapWidthHeight(GSM_NokiaStartupLogo, &Width, &Height);
		req[count++] = Height;
		req[count++] = Width;
		PHONE_EncodeBitmap(GSM_NokiaStartupLogo, req + count, Bitmap);
		count = count + PHONE_GetBitmapSize(GSM_NokiaStartupLogo);
		return GSM_WaitFor (s, req, count, 0x05, 4, ID_SetBitmap);
	case GSM_WelcomeNoteText:
	case GSM_DealerNoteText:
		req[count++] = 0x18;
		req[count++] = 0x01;	/* One block	*/
		if (Bitmap->Type == GSM_WelcomeNoteText) {
			req[count++] = 0x02;
		} else {
			req[count++] = 0x03;
		}
		textlen	= strlen(DecodeUnicodeString(Bitmap->Text));
		req[count++] = textlen;
		memcpy(req + count,DecodeUnicodeString(Bitmap->Text),textlen);
		count += textlen;
		return GSM_WaitFor (s, req, count, 0x05, 4, ID_SetBitmap);
	case GSM_CallerLogo:
		if (IsPhoneFeatureAvailable(s->Model,F_NOCALLER)) return GE_NOTSUPPORTED;
		req[count++] = 0x13;
		req[count++] = Bitmap->Location - 1;
		textlen = strlen(DecodeUnicodeString(Bitmap->Text));
		req[count++] = textlen;
		memcpy(req+count,DecodeUnicodeString(Bitmap->Text),textlen);
		count += textlen;
		req[count++] = Bitmap->Ringtone;
		/* Setting for graphic:
		 * 0x00 - Off
		 * 0x01 - On
		 * 0x02 - View Graphics
		 * 0x03 - Send Graphics
		 * 0x04 - Send via IR
		 * You can even set it higher but Nokia phones (my
		 * 6110 at least) will not show you the name of this
		 * item in menu ;-)) Nokia is really joking here.
		 */
		if (Bitmap->Enabled) req[count++] = 0x01; else req[count++] = 0x00;
		req[count++] = (PHONE_GetBitmapSize(GSM_NokiaCallerLogo) + 4) >> 8;
		req[count++] = (PHONE_GetBitmapSize(GSM_NokiaCallerLogo) + 4) % 0xff;
		NOKIA_CopyBitmap(GSM_NokiaCallerLogo, Bitmap, req, &count);
		return GSM_WaitFor (s, req, count, 0x03, 4, ID_SetBitmap);
	case GSM_OperatorLogo:
		req[count++] = 0x30;
		req[count++] = 0x01;
		NOKIA_EncodeNetworkCode(req+count, Bitmap->NetworkCode);
		count = count + 3;
		req[count++] = (PHONE_GetBitmapSize(GSM_NokiaOperatorLogo) + 4) >> 8;
		req[count++] = (PHONE_GetBitmapSize(GSM_NokiaOperatorLogo) + 4) % 0xff;
		NOKIA_CopyBitmap(GSM_NokiaOperatorLogo, Bitmap, req, &count);
		return GSM_WaitFor (s, req, count, 0x05, 4, ID_SetBitmap);
	case GSM_PictureImage:
		if (IsPhoneFeatureAvailable(s->Model,F_NOPICTURE)) return GE_NOTSUPPORTED;
		req[count++] = 0x03;
		req[count++] = Bitmap->Location - 1;
		if (Bitmap->Sender[0]!=0 || Bitmap->Sender[1]!=0)
		{
			req[count]=GSM_PackSemiOctetNumber(Bitmap->Sender, req+count+1,true);
			/* Convert number of semioctets to number of chars and add count */
			textlen = req[count];
			if (textlen % 2) textlen++;
			count += textlen / 2 + 1;
			count++;
		} else {
			req[count++] = 0x00;
			req[count++] = 0x00;
		}
		req[count++] = 0x00;
		textlen = strlen(DecodeUnicodeString(Bitmap->Text));
		req[count++] = textlen;
		memcpy(req+count,DecodeUnicodeString(Bitmap->Text),textlen);
		count += textlen;
		NOKIA_CopyBitmap(GSM_NokiaPictureImage, Bitmap, req, &count);
		return GSM_WaitFor (s, req, count, 0x47, 4, ID_SetBitmap);
	default:
		break;
	}
	return GE_NOTSUPPORTED;
}

/* for example: "Euro_char" text */
static void Decode3310Subset3(int j, GSM_Protocol_Message msg, GSM_Phone_Data *Data)
{
	wchar_t wc;
	int	len = 0;
	int 	i;
	bool	charfound;

	i = j;
	while (i!=msg.Buffer[23]) {
		EncodeWithUnicodeAlphabet(msg.Buffer+24+i,&wc);
		charfound = false;
		if (i!=msg.Buffer[23]-2) {
			if (msg.Buffer[24+i]  ==0xe2 && msg.Buffer[24+i+1]==0x82 &&
			    msg.Buffer[24+i+2]==0xac) {
				wc = 0x20 * 256 + 0xac;
				i+=2;
				charfound = true;
			}
		}
		if (i!=msg.Buffer[23]-1 && !charfound) {
			if (msg.Buffer[24+i]>=0xc2) {
				wc = DecodeWithUTF8Alphabet(msg.Buffer[24+i],msg.Buffer[24+i+1]);
				i++;
			}
		}
		Data->Calendar->Text[len++] = (wc >> 8) & 0xff;
		Data->Calendar->Text[len++] = wc & 0xff;
		i++;
	}
	Data->Calendar->Text[len++] = 0;
	Data->Calendar->Text[len++] = 0;
}

/* For example: "a with : above" char */
static void Decode3310Subset2(int j, GSM_Protocol_Message msg, GSM_Phone_Data *Data)
{
	int	len = 0;
	int 	i;

	i = j;
	while (i!=msg.Buffer[23]) {
		Data->Calendar->Text[len++] = 0x00;
		Data->Calendar->Text[len++] = msg.Buffer[24+i];
		i++;
	}
	Data->Calendar->Text[len++] = 0;
	Data->Calendar->Text[len++] = 0;
}

static GSM_Error N6110_ReplyGetCalendarNote(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int	i = 0;
	bool	SpecialSubSet = false;

	switch (msg.Buffer[4]) {
	case 0x01:
		dprintf("Calendar note received\n");
		switch (msg.Buffer[8]) {
			case 0x01: Data->Calendar->Type = GCN_REMINDER;	break;
			case 0x02: Data->Calendar->Type = GCN_CALL;	break;
			case 0x03: Data->Calendar->Type = GCN_MEETING;	break;
			case 0x04: Data->Calendar->Type = GCN_BIRTHDAY;	break;
	                case 0x05: Data->Calendar->Type = GCN_T_ATHL;   break;
	                case 0x06: Data->Calendar->Type = GCN_T_BALL;   break;
	                case 0x07: Data->Calendar->Type = GCN_T_CYCL;   break;
	                case 0x08: Data->Calendar->Type = GCN_T_BUDO;   break;
	                case 0x09: Data->Calendar->Type = GCN_T_DANC;   break;
	                case 0x0a: Data->Calendar->Type = GCN_T_EXTR;   break;
	                case 0x0b: Data->Calendar->Type = GCN_T_FOOT;   break;
	                case 0x0c: Data->Calendar->Type = GCN_T_GOLF;   break;
	                case 0x0d: Data->Calendar->Type = GCN_T_GYM;    break;
	                case 0x0e: Data->Calendar->Type = GCN_T_HORS;   break;
	                case 0x0f: Data->Calendar->Type = GCN_T_HOCK;   break;
	                case 0x10: Data->Calendar->Type = GCN_T_RACE;   break;
	                case 0x11: Data->Calendar->Type = GCN_T_RUGB;   break;
	                case 0x12: Data->Calendar->Type = GCN_T_SAIL;   break;
	                case 0x13: Data->Calendar->Type = GCN_T_STRE;   break;
	                case 0x14: Data->Calendar->Type = GCN_T_SWIM;   break;
	                case 0x15: Data->Calendar->Type = GCN_T_TENN;   break;
	                case 0x16: Data->Calendar->Type = GCN_T_TRAV;   break;
	                case 0x17: Data->Calendar->Type = GCN_T_WINT;   break;
			default  :
				dprintf("Unknown note type %i\n",msg.Buffer[8]);
				return GE_UNKNOWNRESPONSE;
		}
#ifdef DEBUG
		switch (msg.Buffer[8]) {
			case 0x01: dprintf("Reminder\n");	break;
			case 0x02: dprintf("Call\n");		break;
			case 0x03: dprintf("Meeting\n");	break;
			case 0x04: dprintf("Birthday\n");	break;
		}
#endif
		NOKIA_DecodeDateTime(msg.Buffer+9, &Data->Calendar->Time);
		dprintf("Time        : %02i-%02i-%04i %02i:%02i:%02i\n",
			Data->Calendar->Time.Day,Data->Calendar->Time.Month,Data->Calendar->Time.Year,
			Data->Calendar->Time.Hour,Data->Calendar->Time.Minute,Data->Calendar->Time.Second);
		NOKIA_DecodeDateTime(msg.Buffer+16, &Data->Calendar->Alarm);
#ifdef DEBUG
		if (Data->Calendar->Alarm.Year!=0) {
			dprintf("Alarm       : %02i-%02i-%04i %02i:%02i:%02i\n",
				Data->Calendar->Alarm.Day,Data->Calendar->Alarm.Month,Data->Calendar->Alarm.Year,
				Data->Calendar->Alarm.Hour,Data->Calendar->Alarm.Minute,Data->Calendar->Alarm.Second);
		} else {
			dprintf("No alarm\n");
		}
#endif
		if (IsPhoneFeatureAvailable(Data->Model,F_CAL52) ||
		    IsPhoneFeatureAvailable(Data->Model,F_CAL82)) {
			memcpy(Data->Calendar->Text,msg.Buffer+24,msg.Buffer[23]);
			Data->Calendar->Text[msg.Buffer[23]  ]=0;
			Data->Calendar->Text[msg.Buffer[23]+1]=0;
			dprintf("Text \"%s\"\n",DecodeUnicodeString(Data->Calendar->Text));
		} else {
			if (IsPhoneFeatureAvailable(Data->Model,F_CAL33))
			{
				/* first char is subset for 33xx and reminders */
				if (Data->Calendar->Type == GCN_REMINDER) {
					i=1;
					dprintf("Subset %i in reminder note !\n",msg.Buffer[24]);
				}
				SpecialSubSet = true;
				switch (msg.Buffer[24]) {
					case 2  : Decode3310Subset2(i,msg,Data); break;
					case 3  : Decode3310Subset3(i,msg,Data); break;
					default : SpecialSubSet = false;	 break;
				}
			}
			if (!SpecialSubSet) {
				EncodeUnicode(Data->Calendar->Text,msg.Buffer+24+i,msg.Buffer[23]-i);
			}
			dprintf("Text       : \"%s\"\n",DecodeUnicodeString(Data->Calendar->Text));
		}
		if (Data->Calendar->Type == GCN_CALL)
		{
			EncodeUnicode(Data->Calendar->Phone,msg.Buffer+24+msg.Buffer[23]+1,msg.Buffer[24+msg.Buffer[23]]);
			dprintf("Phone       : \"%s\"\n",DecodeUnicodeString(Data->Calendar->Phone));
		}
		Data->Calendar->Recurrance	= 0;
		Data->Calendar->SilentAlarm	= false;
		return GE_NONE;
	case 0x93:
		dprintf("Can't get calendar note - too high location?\n");
		return GE_INVALIDLOCATION;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_GetCalendarNote(GSM_StateMachine *s, GSM_CalendarNote *Note, bool start)
{
	GSM_Error	error;
	GSM_DateTime	date_time;
	unsigned char 	req[] = {
		N6110_FRAME_HEADER, 0x66,
		0x00};		/* Location */

	if (IsPhoneFeatureAvailable(s->Model, F_NOCALENDAR)) return GE_NOTSUPPORTED;

	req[4]=Note->Location;

	s->Phone.Data.Calendar=Note;
	dprintf("Getting calendar note\n");
	error=GSM_WaitFor (s, req, 5, 0x13, 4, ID_GetCalendarNote);
	/* 2090 year is set for example in 3310 */
	if (error == GE_NONE && Note->Time.Year == 2090) {
		error=N6110_GetDateTime(s, &date_time);
		if (error == GE_NONE) Note->Time.Year = date_time.Year;
	}
	return error;
}

static GSM_Error N6110_ReplyCallInfo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
#ifdef DEBUG
	int tmp, count;
#endif
	switch (msg.Buffer[3]) {
	case 0x02:
		dprintf("Call going, sequence %d\n",msg.Buffer[4]);
		break;
	case 0x03:
		dprintf("Call in progress, sequence %d\n",msg.Buffer[4]);
		break;
	case 0x04:
		dprintf("Remote end hang up, sequence %d, error %i\n",msg.Buffer[4],msg.Buffer[6]);
		break;	
	case 0x05:
#ifdef DEBUG
		dprintf("Incoming call, sequence %d, number \"",msg.Buffer[4]);
		count=msg.Buffer[6];
		for (tmp=0; tmp <count; tmp++) dprintf("%c", msg.Buffer[7+tmp]);
		dprintf("\", name \"");
		for (tmp=0; tmp<msg.Buffer[7+count]; tmp++) dprintf("%c", msg.Buffer[8+count+tmp]);
		dprintf("\"\n");
#endif
		break;
	case 0x07:
		dprintf("Call answered, sequence %d\n",msg.Buffer[4]);
		break;
	case 0x09:
		dprintf("Call end from your phone, sequence %d\n",msg.Buffer[4]);
		break;
	case 0x0a:
		dprintf("Call info, meaning not known, sequence %d\n",msg.Buffer[4]);
		break;
	case 0x23:
		dprintf("Call held, meaning not known, sequence %d\n",msg.Buffer[4]);
		break;
	case 0x25:
		dprintf("Call resumed, meaning not known, sequence %d\n",msg.Buffer[4]);
		break;
	}
	return GE_NONE;
}

static GSM_Error N6110_DeleteSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x0a, 0x02,
		0x00};		/* Location */

	if (sms->Folder!=0x00) return GE_NOTSUPPORTED;

	req[5]=sms->Location;

	dprintf("Deleting sms\n");
	return GSM_WaitFor (s, req, 6, 0x14, 4, ID_DeleteSMSMessage);
}

static GSM_Error N6110_ReplySetMemory(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Reply for writing memory\n");
	switch (msg.Buffer[3]) {
	case 0x05:
		dprintf("Done OK\n");
		return GE_NONE;
	case 0x06:
		dprintf("Error\n");
		switch (msg.Buffer[4]) {
		case 0x7d:
			dprintf("Too high location ?\n");
			return GE_INVALIDLOCATION;
		case 0x90:
			dprintf("Too long name...or other error\n");
			return GE_NOTSUPPORTED;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
		}
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_SetMemory(GSM_StateMachine *s, GSM_PhonebookEntry *entry)
{
	int		current, Group, Name, Number;
	unsigned char 	req[128] = {
		N6110_FRAME_HEADER, 0x04,
		0x00,			/* memory type	*/
		0x00};			/* location	*/

	GSM_PhonebookFindDefaultNameNumberGroup(*entry, &Name, &Number, &Group);

	req[4] = NOKIA_GetMemoryType(entry->MemoryType,N6110_MEMORY_TYPES);
	req[5] = entry->Location;

	current = 7;

	if (IsPhoneFeatureAvailable(s->Model, F_NOPBKUNICODE)) {
		if (Name != -1) {
			req[6] = strlen(DecodeUnicodeString(entry->Entries[Name].Text));
			memcpy(req+current,DecodeUnicodeString(entry->Entries[Name].Text),strlen(DecodeUnicodeString(entry->Entries[Name].Text)));
			current += strlen(DecodeUnicodeString(entry->Entries[Name].Text));
		} else req[6] = 0;
	} else {
		if (Name != -1) {
			req[6] = strlen(DecodeUnicodeString(entry->Entries[Name].Text))*2+2;
			memcpy(req+current,entry->Entries[Name].Text,strlen(DecodeUnicodeString(entry->Entries[Name].Text))*2);
			current += strlen(DecodeUnicodeString(entry->Entries[Name].Text))*2;
		} else req[6] = 0;
		req[current++]=0x00;
		req[current++]=0x00;
	}

	if (Number != -1) {
		req[current++]=strlen(DecodeUnicodeString(entry->Entries[Number].Text));
		memcpy(req+current,DecodeUnicodeString(entry->Entries[Number].Text),strlen(DecodeUnicodeString(entry->Entries[Number].Text)));
		current += strlen(DecodeUnicodeString(entry->Entries[Number].Text));
	} else req[current++] = 0;

	/* This allow to save 14 characters name into SIM memory, when
	 * no caller group is selected. */
	if (Group == -1) {
		req[current++] = 0xff;
	} else {
		req[current++] = entry->Entries[Group].Number;
	}

	dprintf("Writing phonebook entry\n");
	return GSM_WaitFor (s, req, current, 0x03, 4, ID_SetMemory);
}

static GSM_Error N6110_ReplySetCalendarNote(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Written Calendar Note ");
	switch (msg.Buffer[4]) {
		case 0x01:
			dprintf("- OK\n");
			return GE_NONE;
		case 0x73:
		case 0x7d:
			dprintf("- error\n");
			return GE_UNKNOWN;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_SetCalendarNote(GSM_StateMachine *s, GSM_CalendarNote *Note)
{
	bool		Reminder3310 = false;
	unsigned char 	mychar1,mychar2;
	int		i, current;
	unsigned char req[200] = {
		N6110_FRAME_HEADER, 0x64, 0x01, 0x10,
		0x00,		/* Length of the rest of the frame. */
		0x00,		/* The type of calendar note */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x01, 0x00, 0x66, 0x01};

	if (IsPhoneFeatureAvailable(s->Model, F_NOCALENDAR)) return GE_NOTSUPPORTED;

	if (Note->Location==0x00) return GE_INVALIDLOCATION;

	if (IsPhoneFeatureAvailable(s->Model,F_CAL52)) {
		switch(Note->Type) {
    			case GCN_REMINDER: req[7]=0x01; break;
			case GCN_CALL    : req[7]=0x02; break;
	    		case GCN_MEETING : req[7]=0x03; break;
			case GCN_BIRTHDAY: req[7]=0x04; break;
    			case GCN_T_ATHL  : req[7]=0x05; break;
    			case GCN_T_BALL  : req[7]=0x06; break;
    			case GCN_T_CYCL  : req[7]=0x07; break;
    			case GCN_T_BUDO  : req[7]=0x08; break;
		        case GCN_T_DANC  : req[7]=0x09; break;
		        case GCN_T_EXTR  : req[7]=0x0a; break;
		        case GCN_T_FOOT  : req[7]=0x0b; break;
		        case GCN_T_GOLF  : req[7]=0x0c; break;
		        case GCN_T_GYM   : req[7]=0x0d; break;
		        case GCN_T_HORS  : req[7]=0x0e; break;
		        case GCN_T_HOCK  : req[7]=0x0f; break;
		        case GCN_T_RACE  : req[7]=0x10; break;
		        case GCN_T_RUGB  : req[7]=0x11; break;
		        case GCN_T_SAIL  : req[7]=0x12; break;
		        case GCN_T_STRE  : req[7]=0x13; break;
		        case GCN_T_SWIM  : req[7]=0x14; break;
		        case GCN_T_TENN  : req[7]=0x15; break;
		        case GCN_T_TRAV  : req[7]=0x16; break;
		        case GCN_T_WINT  : req[7]=0x17; break;	
		}
	} else {
		switch(Note->Type) {
			case GCN_CALL    : req[7]=0x02; break;
	    		case GCN_MEETING : req[7]=0x03; break;
			case GCN_BIRTHDAY: req[7]=0x04; break;
    			case GCN_REMINDER:
			default		 : req[7]=0x01; break;
		}
	}

	NOKIA_EncodeDateTime(req+8, &Note->Time);
	req[14] = Note->Time.Second;

	if (Note->Alarm.Year != 0) {
		NOKIA_EncodeDateTime(req+15, &Note->Alarm);
		req[21] = Note->Alarm.Second;
	}

	current = 23;

	if (IsPhoneFeatureAvailable(s->Model,F_CAL52) ||
	    IsPhoneFeatureAvailable(s->Model,F_CAL82)) {
		req[22] = strlen(DecodeUnicodeString(Note->Text))*2;
		memcpy(req+current,Note->Text,strlen(DecodeUnicodeString(Note->Text))*2);
		current += strlen(DecodeUnicodeString(Note->Text))*2;

		/*TEST2 - 8250*/
//		req[22] += 2;
//		req[current++] = 0;
//		req[current++] = 0;
	} else {
		req[22] = strlen(DecodeUnicodeString(Note->Text));
		if (IsPhoneFeatureAvailable(s->Model,F_CAL33))
		{
			Reminder3310 = true;
			if (!strcmp(GetModelData(NULL,s->Model,NULL)->model,"3310") && s->VerNum<5.11)
			{
				if (Note->Type!=GCN_REMINDER) Reminder3310 = false;
			}
			if (!strcmp(GetModelData(NULL,s->Model,NULL)->model,"3330") && s->VerNum<=4.50)
			{
				if (Note->Type!=GCN_REMINDER) Reminder3310 = false;			
			}
			if (Reminder3310) {
				req[22]++;		/* one additional char */
				req[current++] = 0x01;	/* we use now subset 1 */
				for (i=0;i<((int)strlen(DecodeUnicodeString(Note->Text)));i++)
				{
					/* Euro char */
					if (Note->Text[i*2]==0x20 && Note->Text[i*2+1]==0xAC) {
						req[current++] 	= 0xe2;
						req[current++] 	= 0x82;
						req[current++] 	= 0xac;
						req[23]		= 0x03; /* use subset 3 	*/
						req[22]+=2;		/* two additional chars	*/
					} else if (EncodeWithUTF8Alphabet(Note->Text[i*2],Note->Text[i*2+1],&mychar1,&mychar2))
					{
						req[current++] 	= mychar1;
						req[current++] 	= mychar2;
						req[23]		= 0x03; /* use subset 3 	*/
						req[22]++;		/* one additional char 	*/
					} else {
						current+=DecodeWithUnicodeAlphabet(((wchar_t)(Note->Text[i*2]*256+Note->Text[i*2+1])),req+current);
					}
				}
			}
		}
		if (!Reminder3310) {
			memcpy(req+current,DecodeUnicodeString(Note->Text),strlen(DecodeUnicodeString(Note->Text)));
			current += strlen(DecodeUnicodeString(Note->Text));
		}
	}

	if (Note->Type == GCN_CALL) {
		req[current++] = strlen(DecodeUnicodeString(Note->Phone));
		memcpy(req+current,DecodeUnicodeString(Note->Phone),strlen(DecodeUnicodeString(Note->Phone)));
		current += strlen(DecodeUnicodeString(Note->Phone));
	}

	req[6] = current - 8;	/*TEST1 - 8250*/

	dprintf("Writing calendar note\n");
	return GSM_WaitFor (s, req, current, 0x13, 4, ID_SetCalendarNote);
}

static GSM_Error N6110_ReplyDeleteCalendarNote(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Calendar note deleted\n");
	switch (msg.Buffer[4]) {
	case 0x01:
		dprintf("Done OK\n");
		return GE_NONE;
	case 0x93:
		dprintf("Can't be done - too high location ?\n");
		return GE_INVALIDLOCATION;
	default:
		dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
		return GE_UNKNOWNRESPONSE;
	}
}

static GSM_Error N6110_DeleteCalendar(GSM_StateMachine *s, GSM_CalendarNote *Note)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x68,
		0x00};		/* Location */

	if (IsPhoneFeatureAvailable(s->Model, F_NOCALENDAR)) return GE_NOTSUPPORTED;

	req[4] = Note->Location;

	dprintf("Deleting calendar note\n");
	return GSM_WaitFor (s, req, 6, 0x13, 5, ID_DeleteCalendarNote);
}

static GSM_Error N6110_ReplyGetRingtone(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	char 		buffer[2000];
	GSM_Error 	error;
	int 		i,end,start;

	dprintf("Ringtone received\n");
	switch (msg.Buffer[4]) {
	case 0x00:
		switch (Data->Ringtone->Format) {
		case RING_NOTETONE:
			memcpy(buffer,msg.Buffer,msg.Length);
			i=7;
			if (buffer[9]==0x4a && buffer[10]==0x3a) i=8;
			buffer[i]=0x02;	
			error=GSM_DecodeNokiaRTTLRingtone(Data->Ringtone, buffer+i, msg.Length-i);
			if (error!=GE_NONE) return GE_EMPTY;
			return GE_NONE;
		case RING_NOKIABINARY:
			i=8;
			while (msg.Buffer[i]!=0) {
				i++;
				if (i>msg.Length) return GE_EMPTY;
			}
			EncodeUnicode(Data->Ringtone->Name,msg.Buffer+8,i-8);
			dprintf("Name \"%s\"\n",DecodeUnicodeString(Data->Ringtone->Name));
			/* Looking for start && end */
			end=0;start=0;i=0;
			while (true) {
				if (start!=0) {
					if (msg.Buffer[i]==0x07 && msg.Buffer[i+1]==0x0b) {
						end=i+2; break;
					}
					if (msg.Buffer[i]==0x0e && msg.Buffer[i+1]==0x0b) {
						end=i+2; break;
					}
				} else {
					if (msg.Buffer[i]==0x02 && msg.Buffer[i+1]==0xfc && msg.Buffer[i+2]==0x09) {
						start = i;
					}
				}
				i++;
				if (i==msg.Length-3) return GE_EMPTY;
			}	  
			/* Copying frame */
			memcpy(Data->Ringtone->NokiaBinary.Frame,msg.Buffer+start,end-start);
			Data->Ringtone->NokiaBinary.Length=end-start;
#ifdef DEBUG
			if (di.dl == DL_TEXTALL) DumpMessage(di.df, Data->Ringtone->NokiaBinary.Frame, Data->Ringtone->NokiaBinary.Length);
#endif
			return GE_NONE;
		}
		dprintf("Ringtone format is %i\n",Data->Ringtone->Format);		
		break;
	default:
		dprintf("Invalid location. Too high ?\n");
		return GE_INVALIDLOCATION;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_GetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, bool PhoneRingtone)
{
	GSM_Error	error;
	unsigned char req[] = {
		0x00, 0x01, 0x9e,
		0x00 };		/* location */

	if (PhoneRingtone) return GE_NOTSUPPORTED;
	if (IsPhoneFeatureAvailable(s->Model,F_NORING)) return GE_NOTSUPPORTED;
	if (Ringtone->Location == 0) return GE_INVALIDLOCATION;

	if (Ringtone->Format == 0x00) {
		if (IsPhoneFeatureAvailable(s->Model,F_RING_SM)) {
			Ringtone->Format = RING_NOTETONE;
		} else {
			Ringtone->Format = RING_NOKIABINARY;
		}
	}

	switch (Ringtone->Format) {
	case RING_NOTETONE:
		if (!IsPhoneFeatureAvailable(s->Model,F_RING_SM)) return GE_NOTSUPPORTED;
		break;
	case RING_NOKIABINARY:
		if (IsPhoneFeatureAvailable(s->Model,F_RING_SM)) return GE_NOTSUPPORTED;
		break;
	}

	error=DCT3_EnableSecurity (s, 0x01);
	if (error!=GE_NONE) return error;

	req[3]=Ringtone->Location-1;
	s->Phone.Data.Ringtone=Ringtone;
	dprintf("Getting (binary) ringtone\n");
	return GSM_WaitFor (s, req, 4, 0x40, 4, ID_GetRingtone);
}

static GSM_Error N6110_ReplyGetSecurityStatus(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	*Data->SecurityStatus = msg.Buffer[4];

#ifdef DEBUG
	dprintf("Security code status\n");
	switch(msg.Buffer[4]) {
		case GSCT_SecurityCode: dprintf("waiting for Security Code.\n"); break;
		case GSCT_Pin         : dprintf("waiting for PIN.\n"); 		 break;
		case GSCT_Pin2        : dprintf("waiting for PIN2.\n"); 	 break;
		case GSCT_Puk         : dprintf("waiting for PUK.\n");		 break;
		case GSCT_Puk2        : dprintf("waiting for PUK2.\n"); 	 break;
		case GSCT_None        : dprintf("nothing to enter.\n"); 	 break;
		default		      : dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
					return GE_UNKNOWNRESPONSE;
	}      
#endif
	return GE_NONE;
}

static GSM_Error N6110_GetSecurityStatus(GSM_StateMachine *s, GSM_SecurityCodeType *Status)
{
	unsigned char req[4] = {N6110_FRAME_HEADER, 0x07};

	s->Phone.Data.SecurityStatus=Status;
	dprintf("Getting security code status\n");
	return GSM_WaitFor (s, req, 4, 0x08, 2, ID_GetSecurityStatus);
}

static GSM_Error N6110_ReplyEnterSecurityCode(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x0b:
		dprintf("Security code OK\n");
		return GE_NONE;
	case 0x0c:
		switch (msg.Buffer[4]) {
		case 0x88:
			dprintf("Wrong code\n");
			return GE_SECURITYERROR;
		case 0x8b:
			dprintf("Not required\n");
			return GE_NONE;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
		}
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_EnterSecurityCode(GSM_StateMachine *s, GSM_SecurityCode Code)
{
	int 		len = 0;
	unsigned char 	req[15] = {
		N6110_FRAME_HEADER, 0x0a,
		0x00};		/* Type of the entered code. */
                            
	req[4]=Code.Type;

	len = strlen(Code.Code);
	memcpy(req+5,Code.Code,len);
	req[5+len]=0x00;
	req[6+len]=0x00;

	dprintf("Entering security code\n");
	return GSM_WaitFor (s, req, 7+len, 0x08, 4, ID_EnterSecurityCode);
}

static GSM_Error N6110_ReplyGetSpeedDial(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x17:
		dprintf("Speed dial received\n");
		switch (msg.Buffer[4]) {
		case 0x02:
			Data->SpeedDial->MemoryType = GMT_ME;
			dprintf("ME ");
			break;
		case 0x03:
			Data->SpeedDial->MemoryType = GMT_SM;
			dprintf("SIM ");
			break;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
			return GE_UNKNOWNRESPONSE;
		}
		Data->SpeedDial->MemoryLocation = msg.Buffer[5];
		if (msg.Buffer[5] == 0x00) Data->SpeedDial->MemoryLocation = Data->SpeedDial->Location;
		Data->SpeedDial->MemoryNumberID = 2;
		dprintf("location %i\n",Data->SpeedDial->MemoryLocation);
		return GE_NONE;
	case 0x18:
		dprintf("Error getting speed dial. Invalid location\n");
		return GE_INVALIDLOCATION;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_GetSpeedDial(GSM_StateMachine *s, GSM_SpeedDial *SpeedDial)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x16,
		0x01};		/* Speed dial number */

	req[4] = SpeedDial->Location;

	s->Phone.Data.SpeedDial=SpeedDial;
	dprintf("Getting speed dial\n");
	return GSM_WaitFor (s, req, 5, 0x03, 4, ID_GetSpeedDial);
}

static GSM_Error N6110_ReplySendDTMF(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x40:
		dprintf("During sending DTMF\n");
		return GE_NONE;
	case 0x51:
		dprintf("DTMF sent OK\n");
		return GE_NONE;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_SendDTMF(GSM_StateMachine *s, char *sequence)
{
	unsigned char length = strlen(sequence);
	unsigned char req[64] = {
		N6110_FRAME_HEADER, 0x50,
		0x00}; 		/* Length of DTMF string. */

	if (!IsPhoneFeatureAvailable(s->Phone.Data.Model, F_DTMF)) return GE_NOTSUPPORTED;
                          
	if (length>59) length=59;
  	req[4] = length;
  
	memcpy(req+5,sequence,length);

	dprintf("Sending DTMF\n");
	return GSM_WaitFor (s, req, 5+length, 0x01, 4, ID_SendDTMF);
}

static GSM_Error N6110_ReplyGetDisplayStatus(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int i;

	dprintf("Display status received\n");
	if (Data->RequestID == ID_GetDisplayStatus) Data->DisplayFeatures->Number=0;
	for (i=0;i<msg.Buffer[4];i++) {
		if (msg.Buffer[2*i+6] == 0x02) {
#ifdef DEBUG
			switch (msg.Buffer[2*i+5]) {
				case 0x01: dprintf("Call in progress\n"); break;
				case 0x02: dprintf("Unknown\n");	  break;
				case 0x03: dprintf("Unread SMS\n");	  break;
				case 0x04: dprintf("Voice call\n");	  break;
				case 0x05: dprintf("Fax call active\n");  break;
				case 0x06: dprintf("Data call active\n"); break;
				case 0x07: dprintf("Keyboard lock\n");	  break;
				case 0x08: dprintf("SMS storage full\n"); break;
			}
#endif
			if (Data->RequestID == ID_GetDisplayStatus) {
				switch (msg.Buffer[2*i+5]) {
					case 0x01: Data->DisplayFeatures->Feature[Data->DisplayFeatures->Number] = GSM_CallActive;
						   break;
					case 0x03: Data->DisplayFeatures->Feature[Data->DisplayFeatures->Number] = GSM_UnreadSMS;
						   break;
					case 0x04: Data->DisplayFeatures->Feature[Data->DisplayFeatures->Number] = GSM_VoiceCall;
						   break;
					case 0x05: Data->DisplayFeatures->Feature[Data->DisplayFeatures->Number] = GSM_FaxCall;
						   break;
					case 0x06: Data->DisplayFeatures->Feature[Data->DisplayFeatures->Number] = GSM_DataCall;
						   break;
					case 0x07: Data->DisplayFeatures->Feature[Data->DisplayFeatures->Number] = GSM_KeypadLocked;
						   break;
					case 0x08: Data->DisplayFeatures->Feature[Data->DisplayFeatures->Number] = GSM_SMSMemoryFull;
						   break;
				}
				if (msg.Buffer[2*i+5]!=0x02) Data->DisplayFeatures->Number++;
			}
		}
	}
	return GE_NONE;
}

static GSM_Error N6110_GetDisplayStatus(GSM_StateMachine *s, GSM_DisplayFeatures *features)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x51};

	if (!IsPhoneFeatureAvailable(s->Phone.Data.Model, F_DISPSTATUS)) return GE_NOTSUPPORTED;

	s->Phone.Data.DisplayFeatures = features;
	dprintf("Getting display status\n");
	return GSM_WaitFor (s, req, 4, 0x0d, 4, ID_GetDisplayStatus);
}

static GSM_Profile_PhoneTableValue Profile6110[] = {
	{Profile_KeypadTone,	 PROFILE_KEYPAD_LEVEL1,		0x00,0x00},
	{Profile_KeypadTone,	 PROFILE_KEYPAD_LEVEL2,		0x00,0x01},
	{Profile_KeypadTone,	 PROFILE_KEYPAD_LEVEL3,		0x00,0x02},
	{Profile_KeypadTone,	 PROFILE_KEYPAD_OFF,		0x00,0xff},
	{Profile_Lights,	 PROFILE_LIGHTS_OFF,		0x01,0x00},
	{Profile_Lights,	 PROFILE_LIGHTS_AUTO,		0x01,0x01},
	{Profile_CallAlert,	 PROFILE_CALLALERT_RINGING,	0x02,0x01},
	{Profile_CallAlert,	 PROFILE_CALLALERT_BEEPONCE,	0x02,0x02},
	{Profile_CallAlert,	 PROFILE_CALLALERT_OFF,		0x02,0x04},
	{Profile_CallAlert,	 PROFILE_CALLALERT_RINGONCE,	0x02,0x05},
	{Profile_CallAlert,	 PROFILE_CALLALERT_ASCENDING,	0x02,0x06},
	{Profile_CallAlert,	 PROFILE_CALLALERT_CALLERGROUPS,0x02,0x07},
	/* Ringtone ID */
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL1,		0x04,0x06},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL2,		0x04,0x07},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL3,		0x04,0x08},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL4,		0x04,0x09},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL5,		0x04,0x0a},
	{Profile_MessageTone,	 PROFILE_MESSAGE_NOTONE,	0x05,0x00},
	{Profile_MessageTone,	 PROFILE_MESSAGE_STANDARD,	0x05,0x01},
	{Profile_MessageTone,	 PROFILE_MESSAGE_SPECIAL,	0x05,0x02},
	{Profile_MessageTone,	 PROFILE_MESSAGE_BEEPONCE,	0x05,0x03},
	{Profile_MessageTone,	 PROFILE_MESSAGE_ASCENDING,	0x05,0x04},
	{Profile_Vibration,	 PROFILE_VIBRATION_OFF,		0x06,0x00},
	{Profile_Vibration,	 PROFILE_VIBRATION_ON,		0x06,0x01},
	{Profile_WarningTone,	 PROFILE_WARNING_OFF,		0x07,0xff},
	{Profile_WarningTone,	 PROFILE_WARNING_ON,		0x07,0x04},
	/* Caller groups */
	{Profile_AutoAnswer,	 PROFILE_AUTOANSWER_OFF,	0x09,0x00},
	{Profile_AutoAnswer,	 PROFILE_AUTOANSWER_ON,		0x09,0x01},
	{0x00,			 0x00,				0x00,0x00}
};

static GSM_Profile_PhoneTableValue Profile3310[] = {
	{Profile_KeypadTone,	 PROFILE_KEYPAD_LEVEL1,		0x00,0x00},
	{Profile_KeypadTone,	 PROFILE_KEYPAD_LEVEL2,		0x00,0x01},
	{Profile_KeypadTone,	 PROFILE_KEYPAD_LEVEL3,		0x00,0x02},
	{Profile_KeypadTone,	 PROFILE_KEYPAD_OFF,		0x00,0xff},
	{Profile_CallAlert,	 PROFILE_CALLALERT_RINGING,	0x01,0x01},
	{Profile_CallAlert,	 PROFILE_CALLALERT_BEEPONCE,	0x01,0x02},
	{Profile_CallAlert,	 PROFILE_CALLALERT_OFF,		0x01,0x04},
	{Profile_CallAlert,	 PROFILE_CALLALERT_RINGONCE,	0x01,0x05},
	{Profile_CallAlert,	 PROFILE_CALLALERT_ASCENDING,	0x01,0x06},
	/* Ringtone ID */
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL1,		0x03,0x06},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL2,		0x03,0x07},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL3,		0x03,0x08},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL4,		0x03,0x09},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL5,		0x03,0x0a},
	{Profile_MessageTone,	 PROFILE_MESSAGE_NOTONE,	0x04,0x00},
	{Profile_MessageTone,	 PROFILE_MESSAGE_STANDARD,	0x04,0x01},
	{Profile_MessageTone,	 PROFILE_MESSAGE_SPECIAL,	0x04,0x02},
	{Profile_MessageTone,	 PROFILE_MESSAGE_BEEPONCE,	0x04,0x03},
	{Profile_MessageTone,	 PROFILE_MESSAGE_ASCENDING,	0x04,0x04},
	{Profile_MessageTone,	 PROFILE_MESSAGE_PERSONAL,	0x04,0x05},
	{Profile_Vibration,	 PROFILE_VIBRATION_OFF,		0x05,0x00},
	{Profile_Vibration,	 PROFILE_VIBRATION_ON,		0x05,0x01},
	{Profile_Vibration,	 PROFILE_VIBRATION_FIRST,	0x05,0x02},
	{Profile_WarningTone,	 PROFILE_WARNING_OFF,		0x06,0xff},
	{Profile_WarningTone,	 PROFILE_WARNING_ON,		0x06,0x04},
	{Profile_ScreenSaver,	 PROFILE_SAVER_OFF,		0x07,0x00},
	{Profile_ScreenSaver,	 PROFILE_SAVER_ON,		0x07,0x01},
	{Profile_ScreenSaverTime,PROFILE_SAVER_TIMEOUT_5SEC,	0x08,0x00},
	{Profile_ScreenSaverTime,PROFILE_SAVER_TIMEOUT_20SEC,	0x08,0x01},
	{Profile_ScreenSaverTime,PROFILE_SAVER_TIMEOUT_1MIN,	0x08,0x02},
	{Profile_ScreenSaverTime,PROFILE_SAVER_TIMEOUT_2MIN,	0x08,0x03},
	{Profile_ScreenSaverTime,PROFILE_SAVER_TIMEOUT_5MIN,	0x08,0x04},
	{Profile_ScreenSaverTime,PROFILE_SAVER_TIMEOUT_10MIN,	0x08,0x05},
	{0x00,			 0x00,				0x00,0x00}
};

static GSM_Error N6110_ReplyGetProfileFeature(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x14:
		dprintf("Profile feature %02x with value %02x\n",msg.Buffer[6],msg.Buffer[8]);
		if (IsPhoneFeatureAvailable(Data->Model, F_PROFILES33)) {
			switch (msg.Buffer[6]) {
			case 0x02:
				dprintf("Ringtone ID\n");
				Data->Profile->FeatureID	[Data->Profile->FeaturesNumber] = Profile_RingtoneID;
				Data->Profile->FeatureValue	[Data->Profile->FeaturesNumber] = msg.Buffer[8];
				Data->Profile->FeaturesNumber++;
				break;
			case 0x09 :
				dprintf("screen saver number\n");
				Data->Profile->FeatureID	[Data->Profile->FeaturesNumber] = Profile_ScreenSaverNumber;
				Data->Profile->FeatureValue	[Data->Profile->FeaturesNumber] = msg.Buffer[8] + 1;
				Data->Profile->FeaturesNumber++;
				break;
			default:
				NOKIA_FindFeatureValue(Profile3310,msg.Buffer[6],msg.Buffer[8],Data,false);
			}
			return GE_NONE;
		}
		switch (msg.Buffer[6]) {
		case 0x01:	/* Lights */
			if (Data->Profile->CarKitProfile) {
				NOKIA_FindFeatureValue(Profile6110,msg.Buffer[6],msg.Buffer[8],Data,false);
			}
			break;
		case 0x03:
			dprintf("Ringtone ID\n");
			Data->Profile->FeatureID	[Data->Profile->FeaturesNumber] = Profile_RingtoneID;
			Data->Profile->FeatureValue	[Data->Profile->FeaturesNumber] = msg.Buffer[8];
			Data->Profile->FeaturesNumber++;
			break;
		case 0x08:      /* Caller groups */
			if (!IsPhoneFeatureAvailable(Data->Model, F_PROFILES51)) {
				NOKIA_FindFeatureValue(Profile6110,msg.Buffer[6],msg.Buffer[8],Data,true);
			}
			break;
		case 0x09:	/* Autoanswer */
			if (Data->Profile->CarKitProfile || Data->Profile->HeadSetProfile) {
				NOKIA_FindFeatureValue(Profile6110,msg.Buffer[6],msg.Buffer[8],Data,false);
			}
			break;
		default:
			NOKIA_FindFeatureValue(Profile6110,msg.Buffer[6],msg.Buffer[8],Data,false);
		}
		return GE_NONE;
	case 0x15:
		dprintf("Invalid profile location\n");
		return GE_INVALIDLOCATION;
	case 0x1b:
		if (IsPhoneFeatureAvailable(Data->Model, F_PROFILES33)) {
			EncodeUnicode(Data->Profile->Name,msg.Buffer+10,msg.Buffer[9]);
		} else {
			CopyUnicodeString(Data->Profile->Name,msg.Buffer+10);
		}
		dprintf("Profile name: \"%s\"\n",Data->Profile->Name);
		Data->Profile->DefaultName = false;
		if (msg.Buffer[9]==0x00) Data->Profile->DefaultName = true;
		return GE_NONE;
	}
	return GE_UNKNOWNRESPONSE;
}

static GSM_Error N6110_GetProfile(GSM_StateMachine *s, GSM_Profile *Profile)
{
	GSM_Error 	error;
	int		i,j;
	unsigned char 	name_req[] = {N6110_FRAME_HEADER, 0x1a, 0x00};
	unsigned char 	feat_req[] = {
		N6110_FRAME_HEADER, 0x13, 0x01,
		0x00,		/* Profile location 	*/
		0x00};		/* Feature number	*/

	s->Phone.Data.Profile=Profile;
	dprintf("Getting profile name\n");
	error = GSM_WaitFor (s, name_req, 5, 0x05, 4, ID_GetProfile);
	if (error!=GE_NONE) return error;
	if (Profile->DefaultName) {
		NOKIA_GetDefaultProfileName(s, Profile);
		if (IsPhoneFeatureAvailable(s->Phone.Data.Model, F_PROFILES51)) {
			switch(Profile->Location) {
			case 1:	EncodeUnicode(Profile->Name,GetMsg(s->msg,"Personal"),strlen(GetMsg(s->msg,"Personal")));
				break;
			case 2: EncodeUnicode(Profile->Name,GetMsg(s->msg,"Car"),strlen(GetMsg(s->msg,"Car")));
				break;
			case 3: EncodeUnicode(Profile->Name,GetMsg(s->msg,"Headset"),strlen(GetMsg(s->msg,"Headset")));
				break;
			}
		}
		if (IsPhoneFeatureAvailable(s->Phone.Data.Model, F_PROFILES33)) {
			switch(Profile->Location) {
			case 1:	EncodeUnicode(Profile->Name,GetMsg(s->msg,"General"),strlen(GetMsg(s->msg,"General")));
				break;
			case 2: EncodeUnicode(Profile->Name,GetMsg(s->msg,"Silent"),strlen(GetMsg(s->msg,"Silent")));
				break;
			case 3: EncodeUnicode(Profile->Name,GetMsg(s->msg,"Discreet"),strlen(GetMsg(s->msg,"Discreet")));
				break;
			case 4:	EncodeUnicode(Profile->Name,GetMsg(s->msg,"Loud"),strlen(GetMsg(s->msg,"Loud")));
				break;
			case 5: EncodeUnicode(Profile->Name,GetMsg(s->msg,"My style"),strlen(GetMsg(s->msg,"My style")));
				break;
			case 6: Profile->Name[0] = 0; Profile->Name[1] = 0;
				break;
			}
		}
	}

	Profile->FeaturesNumber = 0;

	Profile->CarKitProfile	= false;
	Profile->HeadSetProfile	= false;
	if (Profile->Location == 6) Profile->CarKitProfile  = true;
	if (Profile->Location == 7) Profile->HeadSetProfile = true;
	if (IsPhoneFeatureAvailable(s->Phone.Data.Model, F_PROFILES51)) {
		if (Profile->Location == 2) Profile->CarKitProfile  = true;
		if (Profile->Location == 3) Profile->HeadSetProfile = true;
	}
	if (IsPhoneFeatureAvailable(s->Phone.Data.Model, F_PROFILES33)) {
		Profile->HeadSetProfile		= false; //fixme
		Profile->CarKitProfile		= false;
	}

	for (i = 0x00; i <= 0x09; i++) {
		feat_req[5] = Profile->Location - 1; 
		feat_req[6] = i;
		dprintf("Getting profile feature\n");
		error = GSM_WaitFor (s, feat_req, 7, 0x05, 4, ID_GetProfile);
		if (error!=GE_NONE) return error;
	}

	for (i=0;i<Profile->FeaturesNumber;i++) {
		if (Profile->FeatureID[i] == Profile_CallAlert &&
		    Profile->FeatureValue[i] != PROFILE_CALLALERT_CALLERGROUPS) {
			for (j=0;j<5;j++) Profile->CallerGroups[j] = true;
		}
	}

	return error;
}

static GSM_Error N6110_SetProfile(GSM_StateMachine *s, GSM_Profile *Profile)
{
	int 				i;
	bool				found;
	unsigned char			ID,Value;
	GSM_Error			error;
	GSM_Profile_PhoneTableValue 	*ProfilePhone = Profile6110;
	
	if (IsPhoneFeatureAvailable(s->Phone.Data.Model, F_PROFILES33)) ProfilePhone = Profile3310;
	
	for (i=0;i<Profile->FeaturesNumber;i++) {
		found = false;
		if (ProfilePhone == Profile3310) {
			switch (Profile->FeatureID[i]) {
			case Profile_RingtoneID:
				ID 	= 0x02;
				Value 	= Profile->FeatureValue[i];
				found 	= true;
				break;
			case Profile_ScreenSaverNumber:
				ID 	= 0x09;
				Value 	= Profile->FeatureValue[i];
				found 	= true;
				break;
			default:
				found=NOKIA_FindPhoneFeatureValue(
					ProfilePhone,
					Profile->FeatureID[i],Profile->FeatureValue[i],
					&ID,&Value);
			}
		}
		if (ProfilePhone == Profile6110) {
			switch (Profile->FeatureID[i]) {
			case Profile_RingtoneID:
				ID 	= 0x03;
				Value 	= Profile->FeatureValue[i];
				found 	= true;
				break;
			default:
				found=NOKIA_FindPhoneFeatureValue(
					ProfilePhone,
					Profile->FeatureID[i],Profile->FeatureValue[i],
					&ID,&Value);
			}
		}
		if (found) {
			error=N6110_SetProfileFeature (s,((unsigned char)(Profile->Location-1)),ID,Value);
			if (error!=GE_NONE) return error;
		}
	}
	return GE_NONE;
}

static GSM_Error N6110_ReplyIncomingSMS(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	GSM_SMSMessage sms;

#ifdef DEBUG
	dprintf("SMS message received\n");
	sms.State 	= GSM_UnRead;
	sms.InboxFolder = true;
	DCT3_DecodeSMSFrame(&sms,msg.Buffer+7);
#endif
	if (Data->EnableIncomingSMS && User->IncomingSMS!=NULL) {
		sms.State 	= GSM_UnRead;
		sms.InboxFolder = true;
		DCT3_DecodeSMSFrame(&sms,msg.Buffer+7);

		User->IncomingSMS(Data->Device,sms);
	}
	return GE_NONE;
}

static GSM_Reply_Function N6110ReplyFunctions[] = {
	{N6110_ReplyCallInfo,		"\x01",0x03,0x02,ID_IncomingFrame	},
	{N6110_ReplyCallInfo,		"\x01",0x03,0x03,ID_IncomingFrame	},
	{N6110_ReplyCallInfo,		"\x01",0x03,0x04,ID_IncomingFrame	},
	{N6110_ReplyCallInfo,		"\x01",0x03,0x05,ID_IncomingFrame	},
	{N6110_ReplyCallInfo,		"\x01",0x03,0x07,ID_IncomingFrame	},
	{N6110_ReplyCallInfo,		"\x01",0x03,0x09,ID_IncomingFrame	},
	{N6110_ReplyCallInfo,		"\x01",0x03,0x0A,ID_IncomingFrame	},
	{N6110_ReplyCallInfo,		"\x01",0x03,0x23,ID_IncomingFrame	},
	{N6110_ReplyCallInfo,		"\x01",0x03,0x25,ID_IncomingFrame	},
	{N6110_ReplySendDTMF,		"\x01",0x03,0x40,ID_SendDTMF		},
	{N6110_ReplySendDTMF,		"\x01",0x03,0x51,ID_SendDTMF		},

	{DCT3_ReplySendSMSMessage,	"\x02",0x03,0x02,ID_IncomingFrame	},
	{DCT3_ReplySendSMSMessage,	"\x02",0x03,0x03,ID_IncomingFrame	},
	{N6110_ReplyIncomingSMS,	"\x02",0x03,0x10,ID_IncomingFrame	},
#ifdef GSM_ENABLE_CELLBROADCAST
	{DCT3_ReplySetIncomingCB,	"\x02",0x03,0x21,ID_SetIncomingCB	},
	{DCT3_ReplySetIncomingCB,	"\x02",0x03,0x22,ID_SetIncomingCB	},
	{DCT3_ReplyIncomingCB,		"\x02",0x03,0x23,ID_IncomingFrame	},
#endif
	{DCT3_ReplySetSMSC,		"\x02",0x03,0x31,ID_SetSMSC		},
	{DCT3_ReplyGetSMSC,		"\x02",0x03,0x34,ID_GetSMSC		},
	{DCT3_ReplyGetSMSC,		"\x02",0x03,0x35,ID_GetSMSC		},

	{N6110_ReplyGetMemory,		"\x03",0x03,0x02,ID_GetMemory		},
	{N6110_ReplyGetMemory,		"\x03",0x03,0x03,ID_GetMemory		},
	{N6110_ReplySetMemory,		"\x03",0x03,0x05,ID_SetMemory		},
	{N6110_ReplySetMemory,		"\x03",0x03,0x06,ID_SetMemory		},
	{N6110_ReplyGetMemoryStatus,	"\x03",0x03,0x08,ID_GetMemoryStatus	},
	{N6110_ReplyGetMemoryStatus,	"\x03",0x03,0x09,ID_GetMemoryStatus	},
	{N6110_ReplyGetCallerLogo,	"\x03",0x03,0x11,ID_GetBitmap		},
	{N6110_ReplyGetCallerLogo,	"\x03",0x03,0x12,ID_GetBitmap		},
	{N6110_ReplySetCallerLogo,	"\x03",0x03,0x14,ID_SetBitmap		},
	{N6110_ReplySetCallerLogo,	"\x03",0x03,0x15,ID_SetBitmap		},
	{N6110_ReplyGetSpeedDial,	"\x03",0x03,0x17,ID_GetSpeedDial	},
	{N6110_ReplyGetSpeedDial,	"\x03",0x03,0x18,ID_GetSpeedDial	},
	/* 0x1A, 0x1B - reply set speed dial */

	{N6110_ReplyGetStatus,		"\x04",0x03,0x02,ID_GetNetworkLevel	},
	{N6110_ReplyGetStatus,		"\x04",0x03,0x02,ID_GetBatteryLevel	},

	{N6110_ReplySetProfileFeature,	"\x05",0x03,0x11,ID_SetProfile		},
	{N6110_ReplySetProfileFeature,	"\x05",0x03,0x12,ID_SetProfile		},
	{N6110_ReplyGetProfileFeature,	"\x05",0x03,0x14,ID_GetProfile		},
	{N6110_ReplyGetProfileFeature,	"\x05",0x03,0x15,ID_GetProfile		},
	{N6110_ReplyGetStartup,		"\x05",0x03,0x17,ID_GetBitmap		},
	{N6110_ReplySetStartup,		"\x05",0x03,0x19,ID_SetBitmap		},
	{N6110_ReplyGetProfileFeature,	"\x05",0x03,0x1b,ID_GetProfile		},
	{N61_91_ReplySetOpLogo,		"\x05",0x03,0x31,ID_SetBitmap		},
	{N61_91_ReplySetOpLogo,		"\x05",0x03,0x32,ID_SetBitmap		},
	{N6110_ReplyGetOpLogo,		"\x05",0x03,0x34,ID_GetBitmap		},
	{N6110_ReplySetRingtone,	"\x05",0x03,0x37,ID_SetRingtone		},
	{N6110_ReplySetRingtone,	"\x05",0x03,0x38,ID_SetRingtone		},

	/* msg type 0x06 - call divert */

	{N6110_ReplyGetSecurityStatus,	"\x08",0x03,0x08,ID_GetSecurityStatus	},
	{N6110_ReplyEnterSecurityCode,	"\x08",0x03,0x0b,ID_EnterSecurityCode	},
	{N6110_ReplyEnterSecurityCode,	"\x08",0x03,0x0c,ID_EnterSecurityCode	},

	{DCT3_ReplySIMLogin,		"\x09",0x03,0x80,ID_IncomingFrame	},
	{DCT3_ReplySIMLogout,		"\x09",0x03,0x81,ID_IncomingFrame	},

	{DCT3_ReplyGetNetworkInfo,	"\x0A",0x03,0x71,ID_GetNetworkInfo	},
	{DCT3_ReplyGetNetworkInfo,	"\x0A",0x03,0x71,ID_IncomingFrame	},

	{N6110_ReplyGetDisplayStatus,	"\x0D",0x03,0x52,ID_GetDisplayStatus	},
	{N6110_ReplyGetDisplayStatus,	"\x0D",0x03,0x52,ID_IncomingFrame	},

	{DCT3_ReplySetDateTime,		"\x11",0x03,0x61,ID_SetDateTime		},
	{DCT3_ReplyGetDateTime,		"\x11",0x03,0x63,ID_GetDateTime		},
	{DCT3_ReplySetAlarm,		"\x11",0x03,0x6C,ID_SetAlarm		},
	{DCT3_ReplyGetAlarm,		"\x11",0x03,0x6E,ID_GetAlarm		},

	{N6110_ReplySetCalendarNote,	"\x13",0x03,0x65,ID_SetCalendarNote	},
	{N6110_ReplySetCalendarNote,	"\x13",0x03,0x65,ID_IncomingFrame	},
	{N6110_ReplyGetCalendarNote,	"\x13",0x03,0x67,ID_GetCalendarNote	},
	{N6110_ReplyDeleteCalendarNote,	"\x13",0x03,0x69,ID_DeleteCalendarNote	},
	{N6110_ReplyDeleteCalendarNote,	"\x13",0x03,0x69,ID_IncomingFrame	},

	{N6110_ReplySaveSMSMessage,	"\x14",0x03,0x05,ID_SaveSMSMessage	},
	{N6110_ReplySaveSMSMessage,	"\x14",0x03,0x06,ID_SaveSMSMessage	},
	{N6110_ReplyGetSMSMessage,	"\x14",0x03,0x08,ID_GetSMSMessage	},
	{N6110_ReplyGetSMSMessage,	"\x14",0x03,0x09,ID_GetSMSMessage	},
	{DCT3_ReplyDeleteSMSMessage,	"\x14",0x03,0x0B,ID_DeleteSMSMessage	},
	{N6110_ReplyGetSMSStatus,	"\x14",0x03,0x37,ID_GetSMSStatus	},
	{N6110_ReplyGetSMSStatus,	"\x14",0x03,0x38,ID_GetSMSStatus	},

	{DCT3DCT4_ReplyEnableWAP,	"\x3f",0x03,0x01,ID_EnableWAP		},
	{DCT3DCT4_ReplyEnableWAP,	"\x3f",0x03,0x02,ID_EnableWAP		},
	{DCT3_ReplyGetWAPBookmark,	"\x3f",0x03,0x07,ID_GetWAPBookmark	},
	{DCT3_ReplyGetWAPBookmark,	"\x3f",0x03,0x08,ID_GetWAPBookmark	},
	{DCT3DCT4_ReplySetWAPBookmark,	"\x3f",0x03,0x0A,ID_SetWAPBookmark	},
	{DCT3DCT4_ReplySetWAPBookmark,	"\x3f",0x03,0x0B,ID_SetWAPBookmark	},
	{DCT3DCT4_ReplyDelWAPBookmark,	"\x3f",0x03,0x0D,ID_DeleteWAPBookmark	},
	{DCT3DCT4_ReplyDelWAPBookmark,	"\x3f",0x03,0x0E,ID_DeleteWAPBookmark	},
	{DCT3_ReplyGetWAPSettings,	"\x3f",0x03,0x16,ID_GetWAPSettings	},
	{DCT3_ReplyGetWAPSettings,	"\x3f",0x03,0x17,ID_GetWAPSettings	},
	{DCT3_ReplySetWAPSettings,	"\x3f",0x03,0x19,ID_SetWAPSettings	},
	{DCT3_ReplySetWAPSettings,	"\x3f",0x03,0x1A,ID_SetWAPSettings	},
	{DCT3_ReplyGetWAPSettings,	"\x3f",0x03,0x1C,ID_GetWAPSettings	},
	{DCT3_ReplyGetWAPSettings,	"\x3f",0x03,0x1D,ID_GetWAPSettings	},
	{DCT3_ReplySetWAPSettings,	"\x3f",0x03,0x1F,ID_SetWAPSettings	},

	{DCT3_ReplyEnableSecurity,	"\x40",0x02,0x64,ID_EnableSecurity	},
	{N61_71_ReplyResetPhoneSettings,"\x40",0x02,0x65,ID_ResetPhoneSettings	},
	{DCT3_ReplyGetIMEI,		"\x40",0x02,0x66,ID_GetIMEI		},
	{DCT3_ReplyDialCommand,		"\x40",0x02,0x7C,ID_DialVoice		},
	{DCT3_ReplyDialCommand,		"\x40",0x02,0x7C,ID_CancelCall		},
	{DCT3_ReplyDialCommand,		"\x40",0x02,0x7C,ID_AnswerCall		},
	{DCT3_ReplyNetmonitor,		"\x40",0x02,0x7E,ID_Netmonitor		},
	{DCT3_ReplyPlayTone,		"\x40",0x02,0x8F,ID_PlayTone		},
	{N6110_ReplyGetRingtone,	"\x40",0x02,0x9E,ID_GetRingtone		},
	{N6110_ReplySetBinRingtone,	"\x40",0x02,0xA0,ID_SetRingtone		},
	{NOKIA_ReplyGetPhoneString,	"\x40",0x02,0xC8,ID_GetHardware		},
	{NOKIA_ReplyGetPhoneString,	"\x40",0x02,0xC8,ID_GetPPM		},
	{NOKIA_ReplyGetPhoneString,	"\x40",0x02,0xCA,ID_GetProductCode	},
	{NOKIA_ReplyGetPhoneString,	"\x40",0x02,0xCC,ID_GetManufactureMonth	},
	{NOKIA_ReplyGetPhoneString,	"\x40",0x02,0xCC,ID_GetOriginalIMEI	},

	{N6110_ReplyGetSetPicture,	"\x47",0x03,0x02,ID_GetBitmap		},
	{N6110_ReplyGetSetPicture,	"\x47",0x03,0x04,ID_SetBitmap		},
	{N6110_ReplyGetSetPicture,	"\x47",0x03,0x05,ID_SetBitmap		},
	{N6110_ReplyGetSetPicture,	"\x47",0x03,0x06,ID_GetBitmap		},

#ifdef GSM_ENABLE_6110_AUTHENTICATION
	{N6110_ReplyGetMagicBytes,	"\x64",0x00,0x00,ID_MakeAuthentication	},
#endif

	{DCT3DCT4_ReplyGetModelFirmware,"\xD2",0x02,0x00,ID_GetModel		},
	{DCT3DCT4_ReplyGetModelFirmware,"\xD2",0x02,0x00,ID_GetFirmware		},
	{DCT3_ReplyPressKey,		"\xD2",0x02,0x46,ID_PressKey		},
	{DCT3_ReplyPressKey,		"\xD2",0x02,0x47,ID_PressKey		},

	{NULL,				"\x00",0x00,0x00,ID_None		}
};

GSM_Phone_Functions N6110Phone = {
	"3210|3310|3330|3390|3410|5110|5110i|5130|5190|5210|5510|6110|6130|6150|6190|8210|8250|8290|8850|8855|8890",
	N6110ReplyFunctions,
	N6110_Initialise,
	PHONE_Terminate,
	GSM_DispatchMessage,
	DCT3DCT4_GetModel,
	DCT3DCT4_GetFirmware,
	DCT3_GetIMEI,
	N6110_GetDateTime,
	N6110_GetAlarm,
	N6110_GetMemory,
	N6110_GetMemoryStatus,
	DCT3_GetSMSC,
	N6110_GetSMSMessage,
	N6110_GetBatteryLevel,
	N6110_GetNetworkLevel,
	PHONE_GetSMSFolders,
	NOKIA_GetManufacturer,
	N6110_GetNextSMSMessage,
	DCT3_GetSMSStatus,
	NOKIA_SetIncomingSMS,
	DCT3_GetNetworkInfo,
	DCT3_Reset,
	DCT3_DialVoice,
	DCT3_AnswerCall,
	DCT3_CancelCall,
	N6110_GetRingtone,
	N6110_GetCalendarNote,
	DCT3DCT4_GetWAPBookmark,
	N6110_GetBitmap,
	N6110_SetRingtone,
	N6110_SaveSMSMessage,
	DCT3_SendSMSMessage,
	N6110_SetDateTime,
	N6110_SetAlarm,
	N6110_SetBitmap,
	N6110_SetMemory,
	N6110_DeleteSMSMessage,
	N6110_DeleteCalendar,
	N6110_SetCalendarNote,
	DCT3_SetWAPBookmark,
	DCT3DCT4_DeleteWAPBookmark,
	DCT3_GetWAPSettings,
	DCT3_SetIncomingCB,
	DCT3_SetSMSC,
	DCT3_GetManufactureMonth,
	DCT3_GetProductCode,
	DCT3_GetOriginalIMEI,
	DCT3_GetHardware,
	DCT3_GetPPM,
	DCT3_PressKey,
	NOTSUPPORTED,		/*	GetToDo			*/
	NOTSUPPORTED,		/*	DeleteAllToDo		*/
	NOTSUPPORTED,		/*	SetToDo			*/
	DCT3_PlayTone,
	N6110_EnterSecurityCode,
	N6110_GetSecurityStatus,
	N6110_GetProfile,
	NOTSUPPORTED,		/*	GetRingtonesInfo	*/
	DCT3_SetWAPSettings,
	N6110_GetSpeedDial,
	NOTIMPLEMENTED,		/*	SetSpeedDial		*/
	N61_71_ResetPhoneSettings,
	N6110_SendDTMF,
	N6110_GetDisplayStatus,
	NOTIMPLEMENTED,		/*	SetAutoNetworkLogin	*/
	N6110_SetProfile,
	NOTSUPPORTED,		/*	GetSIMIMSI		*/
	NONEFUNCTION		/*	SetIncomingCall		*/
};

#endif
