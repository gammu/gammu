/* (c) 2001-2004 by Marcin Wiacek */
/* based on some Markus Plail work from Gnokii (www.gnokii.org)
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

#ifdef GSM_ENABLE_NOKIA7110

#include <string.h>
#include <time.h>

#include <gammu-nokia.h>

#include "../../../misc/coding/coding.h"
#include "../../../gsmcomon.h"
#include "../../../service/gsmlogo.h"
#include "../../pfunc.h"
#include "../nfunc.h"
#include "../nfuncold.h"
#include "n7110.h"
#include "dct3func.h"

static GSM_Error N7110_GetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm)
{
	return DCT3_GetAlarm(s, Alarm, 0x19);
}

static GSM_Error N7110_SetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm)
{
	return DCT3_SetAlarm(s, Alarm, 0x19);
}

static GSM_Error N7110_ReplyGetMemory(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Phonebook entry received\n");

	if (msg.Buffer[6] == 0x0f)
		return N71_65_ReplyGetMemoryError(msg.Buffer[10], s);

	return N71_65_DecodePhonebook(s, Data->Memory,Data->Bitmap,Data->SpeedDial,msg.Buffer+18,msg.Length-18,false);
}

static GSM_Error N7110_GetMemory (GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	unsigned char req[] = {N7110_FRAME_HEADER, 0x07, 0x01, 0x01, 0x00, 0x01, 0x02,
			       0x05,		/* memory type */
			       0x00, 0x00,	/* location */
			       0x00, 0x00};

	req[9] = NOKIA_GetMemoryType(s, entry->MemoryType,N71_65_MEMORY_TYPES);
	if (req[9]==0xff) return ERR_NOTSUPPORTED;

	if (entry->Location==0x00) return ERR_INVALIDLOCATION;

	req[10] = entry->Location / 256;
	req[11] = entry->Location % 256;

	s->Phone.Data.Memory=entry;
	smprintf(s, "Getting phonebook entry\n");
	return GSM_WaitFor (s, req, 14, 0x03, 4, ID_GetMemory);
}

static GSM_Error N7110_ReplyGetMemoryStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Memory status received\n");
	/* Quess ;-)) */
	if (msg.Buffer[10]==0x10) {
		Data->MemoryStatus->MemoryFree = msg.Buffer[14]*256 + msg.Buffer[15];
	} else {
		Data->MemoryStatus->MemoryFree = msg.Buffer[18];
	}
	smprintf(s, "   Size       : %i\n",Data->MemoryStatus->MemoryFree);
	Data->MemoryStatus->MemoryUsed = msg.Buffer[16]*256 + msg.Buffer[17];
	smprintf(s, "   Used       : %i\n",Data->MemoryStatus->MemoryUsed);
	Data->MemoryStatus->MemoryFree -= Data->MemoryStatus->MemoryUsed;
	smprintf(s, "   Free       : %i\n",Data->MemoryStatus->MemoryFree);
	return ERR_NONE;
}

static GSM_Error N7110_GetMemoryStatus(GSM_StateMachine *s, GSM_MemoryStatus *Status)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x03, 0x02,
			       0x05};		/* Memory type */

	req[5] = NOKIA_GetMemoryType(s, Status->MemoryType,N71_65_MEMORY_TYPES);
	if (req[5]==0xff) return ERR_NOTSUPPORTED;

	s->Phone.Data.MemoryStatus=Status;
	smprintf(s, "Getting memory status\n");
	return GSM_WaitFor (s, req, 6, 0x03, 4, ID_GetMemoryStatus);
}

static void N7110_GetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char *folderid, int *location)
{
	int ifolderid;

	/* simulate flat SMS memory */
	if (sms->Folder==0x00) {
		ifolderid = sms->Location / GSM_PHONE_MAXSMSINFOLDER;
		*folderid = (ifolderid + 1) * 0x08;
		*location = sms->Location - ifolderid * GSM_PHONE_MAXSMSINFOLDER;
	} else {
		*folderid = sms->Folder * 0x08;
		*location = sms->Location;
	}
	smprintf(s, "SMS folder %i & location %i -> 7110 folder %i & location %i\n",
		sms->Folder,sms->Location,*folderid,*location);
}

static void N7110_SetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms, unsigned char folderid, int location)
{
	sms->Folder	= 0;
	sms->Location	= (folderid / 0x08 - 1) * GSM_PHONE_MAXSMSINFOLDER + location;
	smprintf(s, "7110 folder %i & location %i -> SMS folder %i & location %i\n",
		folderid,location,sms->Folder,sms->Location);
}

static GSM_Error N7110_ReplyGetSMSFolders(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 		j,current=5;
	unsigned char 	buffer[200];
	GSM_Phone_Data	*Data = &s->Phone.Data;

	switch (msg.Buffer[3]) {
	case 0x7B:
		smprintf(s, "Names for SMS folders received\n");
		Data->SMSFolders->Number=msg.Buffer[4];
		for (j=0;j<msg.Buffer[4];j++) {
			smprintf(s, "Folder index: %02x",msg.Buffer[current]);
	    		current++;
			smprintf(s, ", folder name: \"");
			CopyUnicodeString(buffer,msg.Buffer+current);
			if ((UnicodeLength(buffer))>GSM_MAX_SMS_FOLDER_NAME_LEN) {
				smprintf(s, "Too long text\n");
				return ERR_UNKNOWNRESPONSE;
			}
			CopyUnicodeString(Data->SMSFolders->Folder[j].Name,buffer);
			smprintf(s, "%s\"\n",DecodeUnicodeString(buffer));
			current=current+2+UnicodeLength(buffer)*2;
			Data->SMSFolders->Folder[j].InboxFolder = false;
			if (j==0) Data->SMSFolders->Folder[j].InboxFolder = true;
			Data->SMSFolders->Folder[j].OutboxFolder = false;
			/**
			 * @todo We should detect outbox here.
			 */
			Data->SMSFolders->Folder[j].Memory = MEM_ME;
			if (j==0 || j==1) Data->SMSFolders->Folder[j].InboxFolder = MEM_MT;
		}
		return ERR_NONE;
	case 0x7C:
		smprintf(s, "Security error ? No PIN ?\n");
		return ERR_SECURITYERROR;
	case 0xCA:
		smprintf(s, "Wait a moment. Phone is during power on and busy now\n");
		return ERR_SECURITYERROR;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N7110_GetSMSFolders(GSM_StateMachine *s, GSM_SMSFolders *folders)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x7A, 0x00, 0x00};

	s->Phone.Data.SMSFolders=folders;
	smprintf(s, "Getting SMS folders\n");
	return GSM_WaitFor (s, req, 6, 0x14, 4, ID_GetSMSFolders);
}

static GSM_Error N7110_ReplyGetSMSFolderStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int			i;
	GSM_Phone_N7110Data	*Priv = &s->Phone.Data.Priv.N7110;

	smprintf(s, "SMS folder status received\n");
	Priv->LastSMSFolder.Number=msg.Buffer[4]*256+msg.Buffer[5];
	smprintf(s, "Number of Entries: %i\n",Priv->LastSMSFolder.Number);
	smprintf(s, "Locations: ");
	for (i=0;i<Priv->LastSMSFolder.Number;i++) {
		Priv->LastSMSFolder.Location[i]=msg.Buffer[6+(i*2)]*256+msg.Buffer[(i*2)+7];
		if (Priv->LastSMSFolder.Location[i] > GSM_PHONE_MAXSMSINFOLDER) {
			smprintf(s, "Increase GSM_PHONE_MAXSMSINFOLDER\n");
			return ERR_UNKNOWNRESPONSE;
		}
		smprintf(s, "%i ",Priv->LastSMSFolder.Location[i]);
	}
	smprintf(s, "\n");
	NOKIA_SortSMSFolderStatus(s, &Priv->LastSMSFolder);
	return ERR_NONE;
}

static GSM_Error N7110_PrivGetSMSFolderStatus(GSM_StateMachine *s, int folderid)
{
	unsigned char req[] = {N7110_FRAME_HEADER, 0x6b,
			       0x08,		/* folderID */
			       0x0F, 0x01};

        req[4] = folderid;

	smprintf(s, "Getting SMS folder status\n");
	return GSM_WaitFor (s, req, 7, 0x14, 4, ID_GetSMSFolderStatus);
}

static GSM_Error N7110_GetSMSFolderStatus(GSM_StateMachine *s, int folderid)
{
	GSM_Error		error;
	int			i;
	GSM_NOKIASMSFolder	folder;

	error = N7110_PrivGetSMSFolderStatus(s,folderid);
	/* 0x08 contais read Inbox, 0xf8 unread Inbox.
	 * we want all msg from Inbox, so read both 0x08 and 0xf8 */
	if (folderid==0x08 && error==ERR_NONE) {
		folder=s->Phone.Data.Priv.N7110.LastSMSFolder;
		error = N7110_PrivGetSMSFolderStatus(s,0xf8);
		if (error==ERR_NONE) {
			for (i=0;i<folder.Number;i++) {
				s->Phone.Data.Priv.N7110.LastSMSFolder.Location[s->Phone.Data.Priv.N7110.LastSMSFolder.Number++]=folder.Location[i];
			}
		}
	}
	return error;
}

static GSM_SMSMessageLayout N7110_SMSTemplate = {
	36 	/*	SMS Text	*/,	17  /*	Phone number	*/,
	255 	/*	SMSC Number 	*/,	15  /*	TPDCS		*/,
	255 	/*	SendingDateTime	*/,	255 /*	SMSCDateTime	*/,
	255	/*	TPStatus	*/,	16  /*	TPUDL		*/,
	255	/*	TPVP		*/,	12  /*	firstbyte	*/,
	13	/*	TPMR		*/,	255 /*  TPPID??		*/};

static GSM_Error N7110_ReplyGetSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int			i;
	size_t			Width, Height;
	unsigned char		output[500], output2[500];
	GSM_Phone_Data		*Data = &s->Phone.Data;

	switch(msg.Buffer[3]) {
	case 0x08:
		switch (msg.Buffer[8]) {
		case 0x00:
		case 0x01:
			smprintf(s, "SMS message\n");
			if (Data->RequestID == ID_GetSMSMessage) {
				Data->GetSMSMessage->Number=1;
				NOKIA_DecodeSMSState(s, msg.Buffer[4], &Data->GetSMSMessage->SMS[0]);
				DCT3_DecodeSMSFrame(s, &Data->GetSMSMessage->SMS[0],msg.Buffer+9);
				return ERR_NONE;
			}
		case 0x02:
			smprintf(s, "SMS template\n");
			if (Data->RequestID == ID_GetSMSMessage) {
				Data->GetSMSMessage->Number=1;
				NOKIA_DecodeSMSState(s, msg.Buffer[4], &Data->GetSMSMessage->SMS[0]);
				Data->GetSMSMessage->SMS[0].PDU=SMS_Submit;
				GSM_DecodeSMSFrame(&Data->GetSMSMessage->SMS[0],msg.Buffer+9,N7110_SMSTemplate);
				return ERR_NONE;
			}
		case 0x07:
			smprintf(s, "Picture Image\n");
			switch (Data->RequestID) {
			case ID_GetBitmap:
				PHONE_GetBitmapWidthHeight(GSM_NokiaPictureImage, &Width, &Height);
				Data->Bitmap->BitmapWidth	= Width;
				Data->Bitmap->BitmapHeight	= Height;
				PHONE_DecodeBitmap(GSM_NokiaPictureImage, msg.Buffer + 51, Data->Bitmap);
				GSM_UnpackSemiOctetNumber(Data->Bitmap->Sender,msg.Buffer+22,true);
#ifdef DEBUG
				GSM_UnpackSemiOctetNumber(output,msg.Buffer+9,true);
				smprintf(s, "SMSC : %s\n",DecodeUnicodeString(output));
#endif
				Data->Bitmap->Text[0] = 0;
				Data->Bitmap->Text[1] = 0;
				if (msg.Length!=304) {
					GSM_UnpackEightBitsToSeven(0, msg.Length-304, msg.Length-304, msg.Buffer+52+PHONE_GetBitmapSize(GSM_NokiaPictureImage,0,0),output);
					DecodeDefault(Data->Bitmap->Text, output, msg.Length - 304, true, NULL);
				}
				return ERR_NONE;
			case ID_GetSMSMessage:
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
				memcpy(output+i,msg.Buffer+51,PHONE_GetBitmapSize(GSM_NokiaPictureImage,0,0));
				i = i + PHONE_GetBitmapSize(GSM_NokiaPictureImage,0,0);
				if (msg.Length!=304) {
					output[i++] = SM30_UNICODETEXT;
					output[i++] = 0;
					output[i++] = 0; /* Length - later changed */
					GSM_UnpackEightBitsToSeven(0, msg.Length-304, msg.Length-304, msg.Buffer+52+PHONE_GetBitmapSize(GSM_NokiaPictureImage,0,0),output2);
					DecodeDefault(output+i, output2, msg.Length - 304, true, NULL);
					output[i - 1] = UnicodeLength(output+i) * 2;
					i = i + output[i-1];
				}
				GSM_MakeMultiPartSMS(Data->GetSMSMessage,output,i,UDH_NokiaProfileLong,SMS_Coding_8bit,1,0);
				for (i=0;i<3;i++) {
	                		Data->GetSMSMessage->SMS[i].Number[0]=0;
	                		Data->GetSMSMessage->SMS[i].Number[1]=0;
				}
				return ERR_NONE;
			default:
				smprintf(s, "Unknown SMS type: %i\n",msg.Buffer[8]);
				return ERR_UNKNOWNRESPONSE;
			}
		default:
			smprintf(s, "Unknown SMS type: %i\n",msg.Buffer[8]);
		}
		break;
	case 0x09:
		switch (msg.Buffer[4]) {
		case 0x02:
			smprintf(s, "Too high location ?\n");
			return ERR_INVALIDLOCATION;
		case 0x07:
			smprintf(s, "Empty\n");
			return ERR_EMPTY;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
		}
	case 0x6F:
		smprintf(s, "SMS message info received\n");
		if (msg.Length == 43) {
			Data->GetSMSMessage->SMS[0].Name[0] = 0;
			Data->GetSMSMessage->SMS[0].Name[1] = 0;
		} else {
			CopyUnicodeString(Data->GetSMSMessage->SMS[0].Name,msg.Buffer+43);
		}
		smprintf(s, "Name: \"%s\"\n",DecodeUnicodeString(Data->GetSMSMessage->SMS[0].Name));
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N7110_PrivGetSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	GSM_Error		error;
	unsigned char		folderid;
	int			location;
	int			i;
        unsigned char 		req[] = {N6110_FRAME_HEADER, 0x07,
					 0x08,			/* folder ID */
					 0x00, 0x05, 		/* location  */
					 0x01, 0x65, 0x01};
	unsigned char 		NameReq[] = {N6110_FRAME_HEADER, 0x6E,
					     0x08,		/* folder ID */
					     0x00, 0x05}; 	/* location  */

	N7110_GetSMSLocation(s, &sms->SMS[0], &folderid, &location);

	req[4]=folderid;
	req[5]=location / 256;
	req[6]=location % 256;

	s->Phone.Data.GetSMSMessage=sms;
	smprintf(s, "Getting sms\n");
	error=GSM_WaitFor (s, req, 10, 0x14, 4, ID_GetSMSMessage);
	if (error==ERR_NONE) {
		NameReq[4] = folderid;
		NameReq[5] = location / 256;
		NameReq[6] = location;
		smprintf(s, "Getting sms info\n");
		error=GSM_WaitFor (s, NameReq, 7, 0x14, 4, ID_GetSMSMessage);
		if (error != ERR_NONE) return error;
		for (i=0;i<sms->Number;i++) {
			N7110_SetSMSLocation(s, &sms->SMS[i], folderid, location);
			sms->SMS[i].Folder  	= folderid/0x08;
			sms->SMS[i].InboxFolder = true;
			if (folderid/0x08 != 0x01) sms->SMS[i].InboxFolder = false;
			CopyUnicodeString(sms->SMS[i].Name,sms->SMS[0].Name);
			sms->SMS[i].Memory = MEM_ME;
			if (folderid/0x08 == 0x01 || folderid/0x08 == 0x02) {
				sms->SMS[i].Memory = MEM_MT;
				if (folderid/0x08 == 0x01) { /* Inbox */
					if (sms->SMS[i].State == SMS_Sent)   sms->SMS[i].Memory = MEM_ME;
					if (sms->SMS[i].State == SMS_UnSent) sms->SMS[i].Memory = MEM_ME;
					if (sms->SMS[i].State == SMS_Read)   sms->SMS[i].Memory = MEM_SM;
					if (sms->SMS[i].State == SMS_UnRead) sms->SMS[i].Memory = MEM_SM;
				}
				if (folderid/0x08 == 0x02) { /* Outbox */
					if (sms->SMS[i].State == SMS_Sent)   sms->SMS[i].Memory = MEM_SM;
					if (sms->SMS[i].State == SMS_UnSent) sms->SMS[i].Memory = MEM_SM;
					if (sms->SMS[i].State == SMS_Read)   sms->SMS[i].Memory = MEM_ME;
					if (sms->SMS[i].State == SMS_UnRead) sms->SMS[i].Memory = MEM_ME;
				}
			}
		}
	}
	return error;
}

static GSM_Error N7110_GetSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms)
{
	GSM_Error		error;
	unsigned char		folderid;
	int		location;
	GSM_Phone_N7110Data	*Priv = &s->Phone.Data.Priv.N7110;
	int			i;
	bool			found = false;

	N7110_GetSMSLocation(s, &(sms->SMS[0]), &folderid, &location);
	error=N7110_GetSMSFolderStatus(s, folderid);
	if (error!=ERR_NONE) return error;
	for (i=0;i<Priv->LastSMSFolder.Number;i++) {
		if (Priv->LastSMSFolder.Location[i]==location) {
			found = true;
			break;
		}
	}
	if (!found) return ERR_EMPTY;
	return N7110_PrivGetSMSMessage(s,sms);
}

static GSM_Error N7110_GetNextSMSMessage(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start)
{
	GSM_Phone_N7110Data	*Priv = &s->Phone.Data.Priv.N7110;
	unsigned char		folderid;
	int		location;
	GSM_Error		error;
	int			i;
	bool			findnextfolder = false;

	if (start) {
		folderid=0x00;
		findnextfolder=true;
		error=N7110_GetSMSFolders(s,&Priv->LastSMSFolders);
		if (error!=ERR_NONE) return error;
	} else {
		N7110_GetSMSLocation(s, &(sms->SMS[0]), &folderid, &location);
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
			folderid=folderid+0x08;
			/* Too high folder number */
			if ((folderid/0x08)>Priv->LastSMSFolders.Number) return ERR_EMPTY;
			/* Get next folder status */
			error=N7110_GetSMSFolderStatus(s, folderid);
			if (error!=ERR_NONE) return error;
			/* First location from this folder */
			location=Priv->LastSMSFolder.Location[0];
		}
	}
	N7110_SetSMSLocation(s, &sms->SMS[0], folderid, location);

	return N7110_PrivGetSMSMessage(s, sms);
}

static int N7110_ReturnBinaryRingtoneLocation(char *model)
{
	if (strcmp(model,"NSE-5") == 0) return 0x72; /* first 0x72 - 7110 */
	if (strcmp(model,"NPE-3") == 0) return 0x89; /* first 0x89 - 6210 */
	if (strcmp(model,"NHM-3") == 0) return 0x89; /* quess for 6250 */
	return 0;
}

static GSM_Error N7110_ReplyGetRingtone(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int			tmp;
	size_t i;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	smprintf(s, "Ringtone received\n");
	switch (msg.Buffer[3]) {
	case 0x23:
		tmp=0;i=4;
		while (msg.Buffer[i]!=0 || msg.Buffer[i+1]!=0) {
			tmp++;
			i=i+2;
			if (i>msg.Length) return ERR_EMPTY;
		}
		memcpy(Data->Ringtone->Name,msg.Buffer+6,tmp*2);
		smprintf(s, "Name \"%s\"\n",DecodeUnicodeString(Data->Ringtone->Name));
		/* Looking for end */
		i=37;
		while (true) {
			if (msg.Buffer[i]==0x07 && msg.Buffer[i+1]==0x0b) {
				i=i+2; break;
			}
			if (msg.Buffer[i]==0x0e && msg.Buffer[i+1]==0x0b) {
				i=i+2; break;
			}
			i++;
			if (i==msg.Length) return ERR_EMPTY;
		}
		/* Copying frame */
		memcpy(Data->Ringtone->NokiaBinary.Frame,msg.Buffer+37,i-37);
		Data->Ringtone->NokiaBinary.Length=i-37;
		return ERR_NONE;
	case 0x24:
		smprintf(s, "Invalid location. Too high ?\n");
		return ERR_INVALIDLOCATION;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N7110_GetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, bool PhoneRingtone)
{
	unsigned char req[] = {N7110_FRAME_HEADER, 0x22, 0x00, 0x00};

	if (PhoneRingtone) return ERR_NOTSUPPORTED;
	if (Ringtone->Format == 0x00) Ringtone->Format = RING_NOKIABINARY;

	switch (Ringtone->Format) {
	case RING_NOTETONE:
		/* In the future get binary and convert */
		return ERR_NOTSUPPORTED;
	case RING_NOKIABINARY:
		req[5]=N7110_ReturnBinaryRingtoneLocation(s->Phone.Data.Model)+Ringtone->Location;
		s->Phone.Data.Ringtone=Ringtone;
		smprintf(s, "Getting binary ringtone\n");
		return GSM_WaitFor (s, req, 6, 0x1f, 4, ID_GetRingtone);
	case RING_MIDI:
	case RING_MMF:
		return ERR_NOTSUPPORTED;
	}
	return ERR_NOTSUPPORTED;
}

static GSM_Error N7110_ReplyGetPictureImageInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int			i;
	GSM_Phone_N7110Data	*Priv = &s->Phone.Data.Priv.N7110;

	smprintf(s, "Received info for Picture Images\n");
	smprintf(s, "Number    : %i\n",msg.Buffer[4]*256+msg.Buffer[5]);
	smprintf(s, "Locations :");
	Priv->LastPictureImageFolder.Number=msg.Buffer[4]*256+msg.Buffer[5];
	for (i=0;i<Priv->LastPictureImageFolder.Number;i++) {
		Priv->LastPictureImageFolder.Location[i]=msg.Buffer[6+i*2]*256+msg.Buffer[7+i*2];
		smprintf(s, " %i",Priv->LastPictureImageFolder.Location[i]);
	}
	smprintf(s, "\n");
	return ERR_NONE;
}

static GSM_Error N7110_GetPictureImageLocation(GSM_StateMachine *s, GSM_Bitmap *Bitmap, unsigned char *folder, int *location)
{
	GSM_Phone_N7110Data	*Priv = &s->Phone.Data.Priv.N7110;
	GSM_SMSFolders	 	folders;
	GSM_Error		error;
	int			i, j = 0, count = 0;
	unsigned char 		req[] = {N6110_FRAME_HEADER, 0x96,
					 0x00,			/* Folder ID */
					 0x0f, 0x07};

	/* Just to have some defined values */
	*folder = 0;
	*location = 0;

	error=N7110_GetSMSFolders (s, &folders);
	if (error!=ERR_NONE) return error;

	for (i=0;i<folders.Number;i++) {
		req[4] = (i+1) * 0x08;	/* SMS folder ID */
		error = GSM_WaitFor (s, req, 7, 0x14, 4, ID_GetBitmap);
		if (error!=ERR_NONE) return error;
		for (j=0;j<Priv->LastPictureImageFolder.Number;j++) {
			count++;
			if (count==Bitmap->Location) break;
		}
		if (count==Bitmap->Location) break;
	}
	if (count!=Bitmap->Location) return ERR_INVALIDLOCATION;
	*folder		= (i+1) * 0x08;	/* SMS Folder ID */
	*location	= Priv->LastPictureImageFolder.Location[j];
	return ERR_NONE;
}

static GSM_Error N7110_GetPictureImage(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	unsigned char		folder;
	int			location;
	GSM_Error		error;
	unsigned char 		req[] = {N6110_FRAME_HEADER, 0x07,
					 0x00, 			/* Folder ID */
					 0x00, 0x00,		/* Location */
					 0x00, 0x64};

	error = N7110_GetPictureImageLocation(s, Bitmap, &folder, &location);
	switch (error) {
	case ERR_NONE:
		req[4] = folder;
		req[5] = location / 256;
		req[6] = location % 256;
		return GSM_WaitFor (s, req, 9, 0x14, 4, ID_GetBitmap);
	default:
		return error;
	}
}

static GSM_Error N7110_GetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	GSM_MemoryEntry		pbk;
	GSM_Error		error;
	unsigned char		OpReq[] = {N6110_FRAME_HEADER, 0x70};

	s->Phone.Data.Bitmap=Bitmap;
	switch (Bitmap->Type) {
	case GSM_StartupLogo:
		smprintf(s, "Getting startup logo\n");
		return N71_92_GetPhoneSetting(s, ID_GetBitmap, 0x15);
	case GSM_WelcomeNote_Text:
		smprintf(s, "Getting welcome note\n");
		return N71_92_GetPhoneSetting(s, ID_GetBitmap, 0x02);
	case GSM_DealerNote_Text:
		smprintf(s, "Getting dealer note\n");
		return N71_92_GetPhoneSetting(s, ID_GetBitmap, 0x17);
	case GSM_CallerGroupLogo:
		pbk.MemoryType = MEM7110_CG;
		pbk.Location   = Bitmap->Location;
		smprintf(s, "Getting caller group logo\n");
		error=N7110_GetMemory(s,&pbk);
		if (error==ERR_NONE) NOKIA_GetDefaultCallerGroupName(Bitmap);
		return error;
	case GSM_OperatorLogo:
		smprintf(s, "Getting operator logo\n");
		/* This is like DCT3_GetNetworkInfo */
		return GSM_WaitFor (s, OpReq, 4, 0x0a, 4, ID_GetBitmap);
	case GSM_PictureImage:
		/* 7110 doesn't support it */
		if (strcmp(s->Phone.Data.Model,"NSE-5") == 0) return ERR_NOTSUPPORTED;
		return N7110_GetPictureImage(s, Bitmap);
	default:
		break;
	}
	return ERR_NOTSUPPORTED;
}

static GSM_Error N7110_SetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, int *maxlength)
{
	GSM_Ringtone	dest;
	GSM_Error	error;
	GSM_NetworkInfo	NetInfo;
	size_t		size=200;
	unsigned char	req[1000] = {0x7C, 0x01, 0x00, 0x0D, 0x00,
				     0x00, 0x00, 0x00, 0x00, 0x00,
				     0x00};	  	/*Length*/
	unsigned char	req2[4000] = {N7110_FRAME_HEADER, 0x1F, 0x00,
				      0x87,		/* Location */
				      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				      0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	switch (Ringtone->Format) {
	case RING_NOTETONE:
		if (Ringtone->Location==255) {
			/* 7110 doesn't support it */
			if (strcmp(s->Phone.Data.Model,"NSE-5") == 0) return ERR_NOTSUPPORTED;
			*maxlength=GSM_EncodeNokiaRTTLRingtone(*Ringtone, req+11, &size);
			req[10] = size;
			error = s->Protocol.Functions->WriteMessage(s, req, size+11, 0x00);
			if (error!=ERR_NONE) return error;
			sleep(1);
			/* We have to make something (not important, what) now */
			/* no answer from phone*/
			return DCT3_GetNetworkInfo(s,&NetInfo);
		}
		GSM_RingtoneConvert(&dest, Ringtone, RING_NOKIABINARY);
		break;
	case RING_NOKIABINARY:
		memcpy(&dest,Ringtone,sizeof(GSM_Ringtone));
		break;
	default:
		return ERR_NOTSUPPORTED;
	}
	req2[5]=N7110_ReturnBinaryRingtoneLocation(s->Phone.Data.Model)+Ringtone->Location;
	CopyUnicodeString(req2+6,Ringtone->Name);
	memcpy(req2+37,dest.NokiaBinary.Frame,dest.NokiaBinary.Length);
	error = s->Protocol.Functions->WriteMessage(s, req2, 37+dest.NokiaBinary.Length, 0x1F);
	if (error!=ERR_NONE) return error;
	sleep(1);
	/* We have to make something (not important, what) now */
	/* no answer from phone*/
	return DCT3_GetNetworkInfo(s,&NetInfo);
}

static GSM_Error N7110_ReplySaveSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	switch (msg.Buffer[3]) {
	case 0x05:
		smprintf(s, "SMS message saving status\n");
		smprintf(s, "Saved in folder %i at location %i\n",msg.Buffer[4], msg.Buffer[5]*256+msg.Buffer[6]);
		if (msg.Buffer[4] == 0xf8) {
			N7110_SetSMSLocation(s, Data->SaveSMSMessage,0x08,msg.Buffer[5]*256+msg.Buffer[6]);
			Data->SaveSMSMessage->Folder = 0x01;
		} else {
			N7110_SetSMSLocation(s, Data->SaveSMSMessage,msg.Buffer[4],msg.Buffer[5]*256+msg.Buffer[6]);
			Data->SaveSMSMessage->Folder = msg.Buffer[4] / 0x08;
		}
		return ERR_NONE;
	case 0x06:
		smprintf(s, "SMS message saving status\n");
		switch (msg.Buffer[4]) {
		case 0x03:
			smprintf(s, "Too high location ?\n");
			return ERR_INVALIDLOCATION;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
		}
		break;
	case 0x84:
		smprintf(s, "Name for SMS changed OK to \"%s\"\n",DecodeUnicodeString(msg.Buffer+7));
		smprintf(s, "Saved in folder %i at location %i\n",msg.Buffer[4], msg.Buffer[5]*256+msg.Buffer[6]);
		if (msg.Buffer[4] == 0xf8) {
			N7110_SetSMSLocation(s, Data->SaveSMSMessage,0x08,msg.Buffer[5]*256+msg.Buffer[6]);
			Data->SaveSMSMessage->Folder = 0x01;
		} else {
			N7110_SetSMSLocation(s, Data->SaveSMSMessage,msg.Buffer[4],msg.Buffer[5]*256+msg.Buffer[6]);
			Data->SaveSMSMessage->Folder = msg.Buffer[4] / 0x08;
		}
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N7110_PrivSetSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	int			length, location;
	unsigned char		folderid, folder;
	GSM_Error		error;
	unsigned char 		req[256] = {N6110_FRAME_HEADER, 0x04,
				  	    0x03,	/* sms status 	*/
				  	    0x10,	/* folder 	*/
				  	    0x00,0x00,	/* location 	*/
				  	    0x00};
	unsigned char 		NameReq[200] = {N6110_FRAME_HEADER, 0x83};

	switch (sms->State) {
		case SMS_Read	: req[4] = 0x01; break;
		case SMS_UnRead	: req[4] = 0x03; break;
		case SMS_Sent	: req[4] = 0x05; break;
		case SMS_UnSent	: req[4] = 0x07; break;
	}

	N7110_GetSMSLocation(s, sms, &folderid, &location);
	req[5] = folderid;
	req[6] = location / 256;
	req[7] = location % 256;

	sms->Memory = MEM_ME;
	/* Outbox */
	if (folderid == 0x10 && (sms->State == SMS_Sent || sms->State == SMS_UnSent)) {
		sms->PDU = SMS_Submit; /* We will use SIM Outbox */
		sms->Memory = MEM_SM;
	}
	/* Inbox */
	if (folderid == 0x08 && (sms->State == SMS_UnRead || sms->State == SMS_Read)) {
		if (sms->State == SMS_UnRead) req[5] = 0xf8; /* We will use SIM Inbox */
		sms->Memory = MEM_SM;
	}

	switch (sms->PDU) {
	case SMS_Deliver:
		error = PHONE_EncodeSMSFrame(s,sms,req+9,PHONE_SMSDeliver,&length,true);
		break;
	case SMS_Submit:
		smprintf(s, "Saving SMS template\n");
		error  = PHONE_EncodeSMSFrame(s,sms,req+9,N7110_SMSTemplate,&length,true);
		req[8] = 0x02;	/* SMS Template info */
		break;
	default:
		return ERR_UNKNOWN;
	}
	if (error != ERR_NONE) return error;

	s->Phone.Data.SaveSMSMessage=sms;
	smprintf(s, "Saving sms\n");
	error=GSM_WaitFor (s, req, 9+length, 0x14, 4, ID_SaveSMSMessage);
	if (error == ERR_NONE && UnicodeLength(sms->Name)!=0) {
		folder = sms->Folder;
		sms->Folder = 0;
		N7110_GetSMSLocation(s, sms, &folderid, &location);
		length = 4;
		NameReq[length++] = folderid;
		NameReq[length++] = location / 256;
		NameReq[length++] = location % 256;
		CopyUnicodeString(NameReq+length, sms->Name);
		length = length+UnicodeLength(sms->Name)*2;
		NameReq[length++] = 0;
		NameReq[length++] = 0;
		error=GSM_WaitFor (s, NameReq, length, 0x14, 4, ID_SaveSMSMessage);
		sms->Folder = folder;
	}
	return error;
}

static GSM_Error N7110_SetSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	int			location;
	unsigned char		folderid;

	N7110_GetSMSLocation(s, sms, &folderid, &location);
	if (location == 0) return ERR_INVALIDLOCATION;
	return N7110_PrivSetSMSMessage(s, sms);
}

static GSM_Error N7110_AddSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	int			location;
	unsigned char		folderid;

	N7110_GetSMSLocation(s, sms, &folderid, &location);
	location = 0;
	N7110_SetSMSLocation(s, sms, folderid, location);
	return N7110_PrivSetSMSMessage(s, sms);
}

static GSM_Error N7110_ReplyClearOperatorLogo(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Clearing operator logo.....\n");
	return ERR_NONE;
}

static GSM_Error N7110_ReplySetOperatorLogo(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Setting operator logo.....\n");
	return ERR_NONE;
}

static GSM_Error N7110_SetCallerLogo(GSM_StateMachine *s, GSM_Bitmap *bitmap)
{
	int		block=0, i;
	size_t Width, Height;
	unsigned int 	count = 18;
	char		string[500];
	unsigned char   req[500] = {N6110_FRAME_HEADER, 0x0b, 0x00,
				    0x01, 0x01, 0x00, 0x00, 0x0c,
				    0x00, 0x10,		/* memory type */
				    0x00, 0x00,		/* location */
				    0x00, 0x00, 0x00};

	req[13] = bitmap->Location;

	/* Enabling/disabling logo */
	if (bitmap->DefaultBitmap) {
		string[0] = 0; /* disabling */
	} else {
		string[0] = bitmap->BitmapEnabled?1:0;
	}
	string[1] = 0;
	count += N71_65_PackPBKBlock(s, N7110_PBK_LOGOON, 2, block++, string, req + count);

	/* Ringtone */
	if (!bitmap->DefaultRingtone) {
		string[0] = bitmap->RingtoneID;
		string[1] = 0;
		count += N71_65_PackPBKBlock(s, N7110_PBK_RINGTONE_ID, 2, block++, string, req + count);
	}

	/* Number of group */
	string[0] = bitmap->Location;
	string[1] = 0;
	count += N71_65_PackPBKBlock(s, N7110_PBK_GROUP, 2, block++, string, req + count);

	/* Name */
	if (!bitmap->DefaultName) {
		i = UnicodeLength(bitmap->Text) * 2;
		string[0] = i + 2;
		memcpy(string + 1, bitmap->Text, i);
		string[i + 1] = 0;
		count += N71_65_PackPBKBlock(s, N7110_PBK_NAME, i + 2, block++, string, req + count);
	}

	/* Logo */
	if (bitmap->DefaultBitmap) {
		bitmap->BitmapWidth  = 72;
		bitmap->BitmapHeight = 14;
		GSM_ClearBitmap(bitmap);
	}
	PHONE_GetBitmapWidthHeight(GSM_NokiaCallerLogo, &Width, &Height);
	string[0] = Width;
	string[1] = Height;
	string[2] = 0;
	string[3] = 0;
	string[4] = PHONE_GetBitmapSize(GSM_NokiaCallerLogo,0,0);
	PHONE_EncodeBitmap(GSM_NokiaCallerLogo, string + 5, bitmap);
	count += N71_65_PackPBKBlock(s, N7110_PBK_GROUPLOGO, PHONE_GetBitmapSize(GSM_NokiaCallerLogo,0,0) + 5, block++, string, req + count);

	req[17] = block;

	return GSM_WaitFor (s, req, count, 0x03, 4, ID_SetBitmap);
}

static GSM_Error N7110_ReplySetPicture(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Picture Image written OK, folder %i, location %i\n",msg.Buffer[4],msg.Buffer[5]*256+msg.Buffer[6]);
	return ERR_NONE;
}

static GSM_Error N7110_SetPictureImage(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	unsigned char		folder;
	GSM_Error		error;
	int			location, i, count;
	size_t Width, Height;
	GSM_Phone_Bitmap_Types	Type = GSM_NokiaPictureImage;
	unsigned char 		req[500] = {N6110_FRAME_HEADER, 0x50, 0x07,
					    0x00,	/* location	*/
					    0x00, 0x00,	/* index	*/
					    0x07};

	error=N7110_GetPictureImageLocation(s, Bitmap, &folder, &location);
	switch (error) {
	case ERR_NONE:
		req[5] = folder;
		req[6] = location / 256;
		req[7] = location % 256;
		break;
	case ERR_INVALIDLOCATION:
		req[5] = 0x21;	/* Save in Templates folder */
		req[6] = 0;
		req[7] = 0;
		break;
	default:
		return error;
	}

	/* Cleaning */
	for (i=0;i<36;i++) req[i+9]=0;

	count=8;
	if (UnicodeLength(Bitmap->Text)==0) {
		count+=2 ;req[count]=0x0c;
		count+=2 ;req[count]=0x0d;
		count+=2 ;req[count]=0x0e;
		count+=2 ;req[count]=0x0f;
		count+=2 ;req[count]=0x10;
		count+=2 ;req[count]=0x11;
		count+=23;req[count]=0x02;
		count++  ;
	} else {
		count+=2 ;req[count]=0x54;
		count++  ;req[count]=0xd4;
		count++  ;req[count]=0x0d;
		count+=2 ;req[count]=0x0e;
		count+=2 ;req[count]=0x0f;
		count+=2 ;req[count]=0x10;
		count+=2 ;req[count]=0x11;
		count+=21;req[count]=0x01;
		count+=3 ;
	}
	req[count] = 0x01;
	count+=2;
	req[count++] = 0x01;
	PHONE_GetBitmapWidthHeight(Type, &Width, &Height);
	req[count++] = Width;
	req[count++] = Height;
	req[count++] = PHONE_GetBitmapSize(Type,0,0) / 256;
	req[count++] = PHONE_GetBitmapSize(Type,0,0) % 256;
	PHONE_EncodeBitmap(Type, req + count, Bitmap);
	count += PHONE_GetBitmapSize(Type,0,0);
	if (UnicodeLength(Bitmap->Text)!=0) {
		req[count] = UnicodeLength(Bitmap->Text);
		GSM_PackSevenBitsToEight(0, Bitmap->Text, req+count+1,strlen(Bitmap->Text));
		count = count + req[count];
	} else {
		req[count++]=0x00;
	}
	req[count++]=0x00;
	smprintf(s, "Setting Picture Image\n");
	return GSM_WaitFor (s, req, count, 0x14, 4, ID_SetBitmap);
}

static GSM_Error N7110_SetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	GSM_Error		error;
	GSM_Phone_Bitmap_Types	Type;
	size_t			Width, Height, i;
	unsigned char reqStartup[1000] = {N7110_FRAME_HEADER, 0xec,
		0x15,			/* Startup Logo setting */
		0x00, 0x00, 0x00, 0x04, 0xc0, 0x02, 0x00,
		0x00,			/* Bitmap height */
		0xc0, 0x03, 0x00,
		0x00,			/* Bitmap width */
		0xc0, 0x04, 0x03, 0x00};
	unsigned char reqOp[1000] = {N7110_FRAME_HEADER, 0xa3, 0x01,
		0x00,			/* logo disabled */
		0x00, 0xf0, 0x00,	/* network code (000 00) */
		0x00 ,0x04,
		0x08,			/* length of rest */
		0x00, 0x00,		/* Bitmap width / height */
		0x00,
		0x00,			/* Bitmap size */
		0x00, 0x00};
	unsigned char reqClrOp[] = {0x00, 0x01, 0x00, 0xaf, 0x00};
	unsigned char reqStartupText[500] = {N7110_FRAME_HEADER, 0xec,
					     0x02}; /* Startup Text setting */

	switch (Bitmap->Type) {
	case GSM_StartupLogo:
		if (Bitmap->Location!=1) return ERR_NOTSUPPORTED;
		Type=GSM_Nokia6210StartupLogo;
		if (strcmp(s->Phone.Data.Model,"NSE-5") == 0) Type=GSM_Nokia7110StartupLogo;
		PHONE_GetBitmapWidthHeight(Type, &Width, &Height);
		reqStartup[12] = Height;
		reqStartup[16] = Width;
		PHONE_EncodeBitmap(Type, reqStartup + 21, Bitmap);
		smprintf(s, "Setting startup logo\n");
		return GSM_WaitFor (s, reqStartup, 21+PHONE_GetBitmapSize(Type,0,0), 0x7A, 4, ID_SetBitmap);
	case GSM_WelcomeNote_Text:
		CopyUnicodeString(reqStartupText + 5, Bitmap->Text);
		i = 6 + UnicodeLength(Bitmap->Text) * 2;
		reqStartupText[i++] = 0;
		reqStartupText[i++] = 0;
		return GSM_WaitFor (s, reqStartupText, i, 0x7A, 4, ID_SetBitmap);
	case GSM_DealerNote_Text:
		reqStartupText[4] = 0x17;
		CopyUnicodeString(reqStartupText + 5, Bitmap->Text);
		i = 6 + UnicodeLength(Bitmap->Text) * 2;
		reqStartupText[i++] = 0;
		reqStartupText[i++] = 0;
		return GSM_WaitFor (s, reqStartupText, i, 0x7A, 4, ID_SetBitmap);
	case GSM_OperatorLogo:
		/* We want to set operator logo, not clear */
		if (strcmp(Bitmap->NetworkCode,"000 00")) {
			reqOp[5] = 0x01;	/* Logo enabled */
			NOKIA_EncodeNetworkCode(reqOp+6, Bitmap->NetworkCode);
			Type = GSM_Nokia7110OperatorLogo;
			reqOp[11] = 8 + PHONE_GetBitmapSize(Type,0,0);
			PHONE_GetBitmapWidthHeight(Type, &Width, &Height);
			reqOp[12]=Width;
			reqOp[13]=Height;
			reqOp[15]=PHONE_GetBitmapSize(Type,0,0);
			PHONE_EncodeBitmap(Type, reqOp + 18, Bitmap);
			smprintf(s, "Setting operator logo\n");
			return GSM_WaitFor (s, reqOp, 18+PHONE_GetBitmapSize(Type,0,0), 0x0A, 4, ID_SetBitmap);
		} else {
			smprintf(s, "Clearing operator logo\n");
			for (i=0;i<5;i++) {
				reqClrOp[4]=i;
				error=GSM_WaitFor (s, reqClrOp, 5, 0x0A, 4, ID_SetBitmap);
				if (error!=ERR_NONE) return error;
			}
			return GSM_WaitFor (s, reqOp, 18, 0x0A, 4, ID_SetBitmap);
		}
	case GSM_CallerGroupLogo:
		return N7110_SetCallerLogo(s,Bitmap);
	case GSM_PictureImage:
		return N7110_SetPictureImage(s,Bitmap);
	default:
		break;
	}
	return ERR_NOTSUPPORTED;
}

static GSM_Error N7110_ReplyDeleteMemory(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Phonebook entry deleted\n");
	return ERR_NONE;
}

static GSM_Error N7110_DeleteMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	unsigned char req[] = {N7110_FRAME_HEADER, 0x0f, 0x00, 0x01,
			       0x04, 0x00, 0x00, 0x0c, 0x01, 0xff,
			       0x00, 0x01,		/* location	*/
			       0x05, 			/* memory type	*/
			       0x00, 0x00, 0x00};

	req[12] = entry->Location / 256;
	req[13] = entry->Location % 256;

	req[14] = NOKIA_GetMemoryType(s, entry->MemoryType,N71_65_MEMORY_TYPES);
	if (req[14]==0xff) return ERR_NOTSUPPORTED;

	smprintf(s, "Deleting phonebook entry\n");
	return GSM_WaitFor (s, req, 18, 0x03, 4, ID_SetMemory);
}

static GSM_Error N7110_SetMemory(GSM_StateMachine *s, GSM_MemoryEntry *entry)
{
	int 		count = 18;
	size_t blocks;
	unsigned char 	req[5000] = {N7110_FRAME_HEADER, 0x0b, 0x00,
				    0x01, 0x01, 0x00, 0x00, 0x0c,
				    0x00, 0x00,		/* memory type	*/
				    0x00, 0x00, 	/* location	*/
				    0x00, 0x00, 0x00};

	if (entry->Location == 0) return ERR_NOTSUPPORTED;

	req[11] = NOKIA_GetMemoryType(s, entry->MemoryType,N71_65_MEMORY_TYPES);
	if (req[11]==0xff) return ERR_NOTSUPPORTED;

	req[12] = entry->Location >> 8;
	req[13] = entry->Location & 0xff;

	count   = count + N71_65_EncodePhonebookFrame(s, req+18, entry, &blocks, false, GSM_IsPhoneFeatureAvailable(s->Phone.Data.ModelInfo, F_VOICETAGS));
	req[17] = blocks;

	smprintf(s, "Writing phonebook entry\n");
	return GSM_WaitFor (s, req, count, 0x03, 4, ID_SetMemory);
}

static GSM_Error N7110_DeleteSMS(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	unsigned char		folderid;
	int			location;
	unsigned char 		req[] = {N7110_FRAME_HEADER, 0x0a,
			       		 0x00, 		/* folder   */
			       		 0x00, 0x00, 	/* location */
			       		 0x01};

	N7110_GetSMSLocation(s, sms, &folderid, &location);
	req[4] = folderid;
	req[5] = location / 256;
	req[6] = location % 256;

	smprintf(s, "Deleting sms\n");
	return GSM_WaitFor (s, req, 8, 0x14, 4, ID_DeleteSMSMessage);
}

static GSM_Error N7110_ReplyGetSMSStatus(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "SMS status received\n");
	switch (msg.Buffer[3]) {
	case 0x37:
		smprintf(s, "SIM size               : %i\n",msg.Buffer[8]*256+msg.Buffer[9]);
		smprintf(s, "Used in phone memory   : %i\n",msg.Buffer[10]*256+msg.Buffer[11]);
		smprintf(s, "Unread in phone memory : %i\n",msg.Buffer[12]*256+msg.Buffer[13]);
		smprintf(s, "Used in SIM            : %i\n",msg.Buffer[14]*256+msg.Buffer[15]);
		smprintf(s, "Unread in SIM          : %i\n",msg.Buffer[16]*256+msg.Buffer[17]);
		Data->SMSStatus->SIMSize	= msg.Buffer[8]*256+msg.Buffer[9];
		Data->SMSStatus->PhoneUsed	= msg.Buffer[10]*256+msg.Buffer[11];
		Data->SMSStatus->PhoneUnRead 	= msg.Buffer[12]*256+msg.Buffer[13];
		Data->SMSStatus->PhoneSize	= 150;
		Data->SMSStatus->SIMUsed 	= msg.Buffer[14]*256+msg.Buffer[15];
		Data->SMSStatus->SIMUnRead 	= msg.Buffer[16]*256+msg.Buffer[17];
		return ERR_NONE;
	case 0x38:
		smprintf(s, "Error. No PIN ?\n");
		return ERR_SECURITYERROR;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N7110_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	GSM_Error 		error;
	GSM_Phone_N7110Data	*Priv = &s->Phone.Data.Priv.N7110;

	error = DCT3_GetSMSStatus(s,status);
	if (error != ERR_NONE) return error;

	/* 6210 family doesn't show in frame with SMS status info
         * about Templates. We get separately info about this SMS folder.
	 */
	error = N7110_GetSMSFolderStatus(s, 0x20);
	if (error != ERR_NONE) return error;
	status->TemplatesUsed = Priv->LastSMSFolder.Number;

	return ERR_NONE;
}

static GSM_Error N7110_ReplyGetProfileFeature(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	switch (msg.Buffer[3]) {
	case 0x02:
		smprintf(s, "Profile feature %02x with value %02x\n",msg.Buffer[6],msg.Buffer[10]);
		switch (msg.Buffer[6]) {
		case 0x03:
			smprintf(s, "Ringtone ID\n");
			Data->Profile->FeatureID	[Data->Profile->FeaturesNumber] = Profile_RingtoneID;
			Data->Profile->FeatureValue	[Data->Profile->FeaturesNumber] = msg.Buffer[10];
			Data->Profile->FeaturesNumber++;
			break;
		case 0x08:	/* Caller groups */
			NOKIA_FindFeatureValue(s, Profile71_65,msg.Buffer[6],msg.Buffer[10],Data,true);
			break;
		case 0x09:	/* Autoanswer */
			if (Data->Profile->CarKitProfile || Data->Profile->HeadSetProfile) {
				NOKIA_FindFeatureValue(s, Profile71_65,msg.Buffer[6],msg.Buffer[10],Data,false);
			}
			break;
		case 0xff :
			CopyUnicodeString(Data->Profile->Name, msg.Buffer+10);
			smprintf(s, "profile Name: \"%s\"\n", DecodeUnicodeString(Data->Profile->Name));
			Data->Profile->DefaultName = false;
			break;
		default:
			NOKIA_FindFeatureValue(s, Profile71_65,msg.Buffer[6],msg.Buffer[10],Data,false);
		}
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

static GSM_Error N7110_GetProfile(GSM_StateMachine *s, GSM_Profile *Profile)
{
	GSM_Error 	error;
	int		i;
	unsigned char 	Features[12] = {0x00,0x02,0x03,0x04,0x05,0x06,
					0x07,0x08,0x09,0xff,
					0x0a,0x22};
	unsigned char 	req[] = {N6110_FRAME_HEADER, 0x01, 0x01, 0x01, 0x01,
				 0x00,  	/* Profile Location	*/
				 0xff};		/* Feature number	*/

	if (Profile->Location > 7) return ERR_INVALIDLOCATION;

	Profile->CarKitProfile		= false;
	Profile->HeadSetProfile		= false;
	if (Profile->Location == 6) Profile->CarKitProfile  = true;
	if (Profile->Location == 7) Profile->HeadSetProfile = true;

	Profile->FeaturesNumber = 0;

	s->Phone.Data.Profile=Profile;
	for (i = 0; i < 10; i++) {
		req[7] = Profile->Location;
		req[8] = Features[i];
		smprintf(s, "Getting profile feature\n");
		error = GSM_WaitFor (s, req, 9, 0x39, 4, ID_GetProfile);
		if (error!=ERR_NONE) return error;
	}
	NOKIA_GetDefaultProfileName(Profile);
	Profile->Active = false;
	return error;
}

static GSM_Error N7110_ReplySetProfileFeature(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Profile feature set\n");
	return ERR_NONE;
}

static GSM_Error N7110_SetProfile(GSM_StateMachine *s, GSM_Profile *Profile)
{
	int 		i;
	bool		found;
	GSM_Error	error;
	unsigned char	ID,Value;
	unsigned char 	req[] = {N6110_FRAME_HEADER, 0x03, 0x01, 0x01, 0x03,
				 0x02,   /* feature number	*/
				 0x01,	 /* Profile Location 	*/
				 0x01,
				 0xff};	 /* Value 		*/

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
			req[7]  = ID;
			req[8]  = Profile->Location;
			req[10] = Value;
			smprintf(s, "Setting profile feature\n");
			error = GSM_WaitFor (s, req, 11, 0x39, 4, ID_SetProfile);
			if (error!=ERR_NONE) return error;
		}
	}
	return ERR_NONE;
}

static GSM_Error N7110_GetSpeedDial(GSM_StateMachine *s, GSM_SpeedDial *SpeedDial)
{
	GSM_MemoryEntry 	pbk;
	GSM_Error		error;

	pbk.MemoryType			= MEM7110_SP;
	pbk.Location			= SpeedDial->Location;
	SpeedDial->MemoryLocation 	= 0;
	s->Phone.Data.SpeedDial		= SpeedDial;

	smprintf(s, "Getting speed dial\n");
	error=N7110_GetMemory(s,&pbk);
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

static GSM_Error N7110_ReplyIncomingSMS(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_SMSMessage		sms;
	GSM_Phone_Data		*Data = &s->Phone.Data;

#ifdef DEBUG
	smprintf(s, "SMS message received\n");
	sms.State 	= SMS_UnRead;
	sms.InboxFolder = true;
	DCT3_DecodeSMSFrame(s, &sms,msg.Buffer+8);
#endif
	if (Data->EnableIncomingSMS && s->User.IncomingSMS!=NULL) {
		sms.State 	= SMS_UnRead;
		sms.InboxFolder = true;
		DCT3_DecodeSMSFrame(s, &sms,msg.Buffer+8);

		s->User.IncomingSMS(s,sms);
	}
	return ERR_NONE;
}

static GSM_Error N7110_Initialise (GSM_StateMachine *s)
{
#ifdef DEBUG
	DCT3_SetIncomingCB(s,true);
#endif
#ifdef GSM_ENABLE_N71_92INCOMINGINFO
	/* Enables various things like incoming SMS, call info, etc. */
	return N71_65_EnableFunctions (s, "\x01\x02\x06\x0A\x14\x17", 6);
#endif
	return ERR_NONE;
}

static GSM_Error N7110_ReplyGetCalendarInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	/* Old method 1 for accessing calendar */
	return N71_65_ReplyGetCalendarInfo1(msg, s, &s->Phone.Data.Priv.N7110.LastCalendar);
}

#ifdef DEBUG
static GSM_Error N7110_ReplyGetCalendarNotePos(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	/* Old method 1 for accessing calendar */
	return N71_65_ReplyGetCalendarNotePos1(msg, s, &s->Phone.Data.Priv.N7110.FirstCalendarPos);
}
#endif

static GSM_Error N7110_GetNextCalendar(GSM_StateMachine *s,  GSM_CalendarEntry *Note, bool start)
{
	return N71_65_GetNextCalendar1(s,Note,start,&s->Phone.Data.Priv.N7110.LastCalendar,&s->Phone.Data.Priv.N7110.LastCalendarYear,&s->Phone.Data.Priv.N7110.LastCalendarPos);
/* 	return N71_65_GetNextCalendar2(s,Note,start,&s->Phone.Data.Priv.N7110.LastCalendarYear,&s->Phone.Data.Priv.N7110.LastCalendarPos); */
}

static GSM_Error N7110_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *Status)
{
	GSM_Error error;

        /* Method 1 */
	error=N71_65_GetCalendarInfo1(s, &s->Phone.Data.Priv.N7110.LastCalendar);
	if (error!=ERR_NONE) return error;
	/**
	 * @todo This should be acquired from phone
	 */
	Status->Free = 100;
	Status->Used = s->Phone.Data.Priv.N6510.LastCalendar.Number;
	return ERR_NONE;

    	/* Method 2 */
/* 	return GE_NOTSUPPORTED; */
}

static GSM_Error N7110_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
/* 	return N71_65_AddCalendar1(s, Note, NULL); */
	return N71_65_AddCalendar2(s,Note);
}

static GSM_Error N7110_ReplyGetNetworkInfoError(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Probably means no PIN\n");
	return ERR_SECURITYERROR;
}

static GSM_Error N7110_SetIncomingCall(GSM_StateMachine *s, bool enable)
{
#ifndef GSM_ENABLE_N71_92INCOMINGINFO
	return ERR_SOURCENOTAVAILABLE;
#else
	return NOKIA_SetIncomingCall(s,enable);
#endif
}

static GSM_Error N7110_SetIncomingUSSD(GSM_StateMachine *s, bool enable)
{
#ifndef GSM_ENABLE_N71_92INCOMINGINFO
	return ERR_SOURCENOTAVAILABLE;
#else
	return NOKIA_SetIncomingUSSD(s,enable);
#endif
}

static GSM_Error N7110_SetIncomingSMS(GSM_StateMachine *s, bool enable)
{
#ifndef GSM_ENABLE_N71_92INCOMINGINFO
	return ERR_SOURCENOTAVAILABLE;
#else
	return NOKIA_SetIncomingSMS(s,enable);
#endif
}

GSM_Error N7110_AnswerCall(GSM_StateMachine *s, int ID, bool all)
{
	if (!all) return DCT3DCT4_AnswerCall(s,ID);
	return DCT3_AnswerAllCalls(s);
}

GSM_Error N7110_SetCallDivert(GSM_StateMachine *s, GSM_MultiCallDivert *divert)
{
	GSM_Error	error;
	int		i;

	/* No answer from phone side */
	i = s->ReplyNum;
	s->ReplyNum = 1;
	error = DCT3DCT4_SetCallDivert(s,divert);
	s->ReplyNum = i;
	return error;
}

GSM_Error N7110_CancelAllDiverts(GSM_StateMachine *s)
{
	GSM_Error	error;
	int		i;

	/* No answer from phone side */
	i = s->ReplyNum;
	s->ReplyNum = 1;
	error = DCT3DCT4_CancelAllDiverts(s);
	s->ReplyNum = i;
	return error;
}

static GSM_Reply_Function N7110ReplyFunctions[] = {
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
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x0C,ID_IncomingFrame	  },
	{N71_65_ReplySendDTMF,		  "\x01",0x03,0x51,ID_SendDTMF		  },
	{N71_65_ReplyCallInfo,		  "\x01",0x03,0x53,ID_IncomingFrame	  },
	{N71_65_ReplySendDTMF,		  "\x01",0x03,0x59,ID_SendDTMF		  },
	{N71_65_ReplySendDTMF,		  "\x01",0x03,0x5E,ID_SendDTMF		  },

	{DCT3_ReplySendSMSMessage,	  "\x02",0x03,0x02,ID_IncomingFrame  	  },
	{DCT3_ReplySendSMSMessage,	  "\x02",0x03,0x03,ID_IncomingFrame	  },
	{N7110_ReplyIncomingSMS,	  "\x02",0x03,0x10,ID_IncomingFrame	  },
#ifdef GSM_ENABLE_CELLBROADCAST
	{DCT3_ReplySetIncomingCB,	  "\x02",0x03,0x21,ID_SetIncomingCB	  },
	{DCT3_ReplySetIncomingCB,	  "\x02",0x03,0x22,ID_SetIncomingCB	  },
	{DCT3_ReplyIncomingCB,		  "\x02",0x03,0x23,ID_IncomingFrame	  },
#endif
	{DCT3_ReplySetSMSC,		  "\x02",0x03,0x31,ID_SetSMSC		  },
	{DCT3_ReplyGetSMSC,		  "\x02",0x03,0x34,ID_GetSMSC		  },
	{DCT3_ReplyGetSMSC,		  "\x02",0x03,0x35,ID_GetSMSC		  },
#ifdef GSM_ENABLE_CELLBROADCAST
	{DCT3_ReplySetIncomingCB,	  "\x02",0x03,0xCA,ID_SetIncomingCB	  },
#endif

	{N7110_ReplyGetMemoryStatus,	  "\x03",0x03,0x04,ID_GetMemoryStatus	  },
	{N7110_ReplyGetMemory,		  "\x03",0x03,0x08,ID_GetMemory		  },
	{N71_65_ReplyWritePhonebook,	  "\x03",0x03,0x0C,ID_SetBitmap		  },
	{N71_65_ReplyWritePhonebook,	  "\x03",0x03,0x0C,ID_SetMemory		  },
	{N7110_ReplyDeleteMemory,	  "\x03",0x03,0x10,ID_SetMemory		  },

	{N71_65_ReplyUSSDInfo,		  "\x06",0x03,0x03,ID_IncomingFrame	  },
	{NoneReply,			  "\x06",0x03,0x06,ID_IncomingFrame	  },

	{DCT3_ReplySIMLogin,		  "\x09",0x03,0x80,ID_IncomingFrame	  },
	{DCT3_ReplySIMLogout,		  "\x09",0x03,0x81,ID_IncomingFrame	  },

	{DCT3_ReplyGetNetworkInfo,	  "\x0A",0x03,0x71,ID_GetNetworkInfo	  },
	{DCT3_ReplyGetNetworkInfo,	  "\x0A",0x03,0x71,ID_GetBitmap		  },
	{DCT3_ReplyGetNetworkInfo,	  "\x0A",0x03,0x71,ID_IncomingFrame	  },
	{N7110_ReplyGetNetworkInfoError,  "\x0A",0x03,0x72,ID_GetNetworkInfo	  },
	{DCT3_ReplyGetNetworkInfo,	  "\x0A",0x03,0x73,ID_IncomingFrame	  },
	{N71_92_ReplyGetSignalQuality,	  "\x0A",0x03,0x82,ID_GetSignalQuality	  },
	{N7110_ReplySetOperatorLogo,	  "\x0A",0x03,0xA4,ID_SetBitmap		  },
	{N7110_ReplyClearOperatorLogo,	  "\x0A",0x03,0xB0,ID_SetBitmap		  },
	{NoneReply,			  "\x0A",0x03,0xB5,ID_IncomingFrame	  },

#ifdef DEBUG
	{N71_65_ReplyAddCalendar1,	  "\x13",0x03,0x02,ID_SetCalendarNote	  },/*method 1*/
	{N71_65_ReplyAddCalendar1,	  "\x13",0x03,0x04,ID_SetCalendarNote	  },/*method 1*/
	{N71_65_ReplyAddCalendar1,	  "\x13",0x03,0x06,ID_SetCalendarNote	  },/*method 1*/
	{N71_65_ReplyAddCalendar1,	  "\x13",0x03,0x08,ID_SetCalendarNote	  },/*method 1*/
#endif
	{N71_65_ReplyDelCalendar,	  "\x13",0x03,0x0C,ID_DeleteCalendarNote  },
	{N71_65_ReplyGetNextCalendar1,	  "\x13",0x03,0x1A,ID_GetCalendarNote	  },/*method 1*/
#ifdef DEBUG
	{N7110_ReplyGetCalendarNotePos,	  "\x13",0x03,0x32,ID_GetCalendarNotePos  },/*method 1*/
#endif
	{N7110_ReplyGetCalendarInfo,	  "\x13",0x03,0x3B,ID_GetCalendarNotesInfo},/*method 1*/
#ifdef DEBUG
	{N71_65_ReplyGetNextCalendar2,	  "\x13",0x03,0x3F,ID_GetCalendarNote	  },/*method 2*/
#endif
	{N71_65_ReplyAddCalendar2,	  "\x13",0x03,0x41,ID_SetCalendarNote	  },/*method 2*/

	{N7110_ReplySaveSMSMessage,	  "\x14",0x03,0x05,ID_SaveSMSMessage	  },
	{N7110_ReplySaveSMSMessage,	  "\x14",0x03,0x06,ID_SaveSMSMessage	  },
	{N7110_ReplyGetSMSMessage,	  "\x14",0x03,0x08,ID_GetSMSMessage	  },
	{N7110_ReplyGetSMSMessage,	  "\x14",0x03,0x08,ID_GetBitmap		  },
	{N7110_ReplyGetSMSMessage,	  "\x14",0x03,0x09,ID_GetSMSMessage	  },
	{DCT3_ReplyDeleteSMSMessage,	  "\x14",0x03,0x0B,ID_DeleteSMSMessage	  },
	{DCT3_ReplyDeleteSMSMessage,	  "\x14",0x03,0x0C,ID_DeleteSMSMessage	  },
	{N7110_ReplyGetSMSStatus,	  "\x14",0x03,0x37,ID_GetSMSStatus	  },
	{N7110_ReplyGetSMSStatus,	  "\x14",0x03,0x38,ID_GetSMSStatus	  },
	{N7110_ReplySetPicture,		  "\x14",0x03,0x51,ID_SetBitmap		  },
	{N7110_ReplyGetSMSFolderStatus,	  "\x14",0x03,0x6C,ID_GetSMSFolderStatus  },
	{N7110_ReplyGetSMSMessage,	  "\x14",0x03,0x6F,ID_GetSMSMessage	  },
	{N7110_ReplyGetSMSFolders,	  "\x14",0x03,0x7B,ID_GetSMSFolders	  },
	{N7110_ReplyGetSMSFolders,	  "\x14",0x03,0x7C,ID_GetSMSFolders	  },
	{N7110_ReplySaveSMSMessage,	  "\x14",0x03,0x84,ID_SaveSMSMessage	  },
	{N7110_ReplyGetPictureImageInfo,  "\x14",0x03,0x97,ID_GetBitmap		  },
	{N7110_ReplyGetSMSFolders,	  "\x14",0x03,0xCA,ID_GetSMSFolders	  },

	{N71_92_ReplyGetBatteryCharge,	  "\x17",0x03,0x03,ID_GetBatteryCharge	  },

	{DCT3_ReplySetDateTime,		  "\x19",0x03,0x61,ID_SetDateTime	  },
	{DCT3_ReplyGetDateTime,		  "\x19",0x03,0x63,ID_GetDateTime	  },
	{DCT3_ReplySetAlarm,		  "\x19",0x03,0x6C,ID_SetAlarm		  },
	{DCT3_ReplyGetAlarm,		  "\x19",0x03,0x6E,ID_GetAlarm		  },

	{N7110_ReplyGetRingtone,	  "\x1f",0x03,0x23,ID_GetRingtone	  },
	{N7110_ReplyGetRingtone,	  "\x1f",0x03,0x24,ID_GetRingtone	  },

	{DCT3DCT4_ReplyEnableConnectFunc, "\x3f",0x03,0x01,ID_EnableConnectFunc   },
	{DCT3DCT4_ReplyEnableConnectFunc, "\x3f",0x03,0x02,ID_EnableConnectFunc	  },
	{DCT3DCT4_ReplyDisableConnectFunc,"\x3f",0x03,0x04,ID_DisableConnectFunc  },
	{DCT3DCT4_ReplyDisableConnectFunc,"\x3f",0x03,0x05,ID_DisableConnectFunc  },
	{DCT3_ReplyGetWAPBookmark,	  "\x3f",0x03,0x07,ID_GetWAPBookmark	  },
	{DCT3_ReplyGetWAPBookmark,	  "\x3f",0x03,0x08,ID_GetWAPBookmark	  },
	{DCT3DCT4_ReplySetWAPBookmark,	  "\x3f",0x03,0x0A,ID_SetWAPBookmark	  },
	{DCT3DCT4_ReplySetWAPBookmark,	  "\x3f",0x03,0x0B,ID_SetWAPBookmark	  },
	{DCT3DCT4_ReplyDelWAPBookmark,	  "\x3f",0x03,0x0D,ID_DeleteWAPBookmark	  },
	{DCT3DCT4_ReplyDelWAPBookmark,	  "\x3f",0x03,0x0E,ID_DeleteWAPBookmark	  },
	{DCT3DCT4_ReplyGetActiveConnectSet,"\x3f",0x03,0x10,ID_GetConnectSet	  },
	{DCT3DCT4_ReplySetActiveConnectSet,"\x3f",0x03,0x13,ID_SetConnectSet	  },
	{DCT3_ReplyGetWAPSettings,	  "\x3f",0x03,0x16,ID_GetConnectSet	  },
	{DCT3_ReplyGetWAPSettings,	  "\x3f",0x03,0x17,ID_GetConnectSet	  },
	{DCT3_ReplySetWAPSettings,	  "\x3f",0x03,0x19,ID_SetConnectSet	  },
	{DCT3_ReplySetWAPSettings,	  "\x3f",0x03,0x1A,ID_SetConnectSet	  },
	{DCT3_ReplyGetWAPSettings,	  "\x3f",0x03,0x1C,ID_GetConnectSet	  },
	{DCT3_ReplyGetWAPSettings,	  "\x3f",0x03,0x1D,ID_GetConnectSet	  },
	{DCT3_ReplySetWAPSettings,	  "\x3f",0x03,0x1F,ID_SetConnectSet	  },

	{N7110_ReplyGetProfileFeature,	  "\x39",0x03,0x02,ID_GetProfile	  },
	{N7110_ReplySetProfileFeature,	  "\x39",0x03,0x04,ID_SetProfile	  },

	{DCT3_ReplyEnableSecurity,	  "\x40",0x02,0x64,ID_EnableSecurity	  },
	{N61_71_ReplyResetPhoneSettings,  "\x40",0x02,0x65,ID_ResetPhoneSettings  },
	{DCT3_ReplyGetIMEI,		  "\x40",0x02,0x66,ID_GetIMEI		  },
	{DCT3_ReplyDialCommand,		  "\x40",0x02,0x7C,ID_DialVoice		  },
	{DCT3_ReplyDialCommand,		  "\x40",0x02,0x7C,ID_CancelCall	  },
	{DCT3_ReplyDialCommand,		  "\x40",0x02,0x7C,ID_AnswerCall	  },
	{DCT3_ReplyNetmonitor,		  "\x40",0x02,0x7E,ID_Netmonitor	  },
	{DCT3_ReplyPlayTone,		  "\x40",0x02,0x8F,ID_PlayTone		  },
	{NOKIA_ReplyGetPhoneString,	  "\x40",0x02,0xC8,ID_GetHardware	  },
	{NOKIA_ReplyGetPhoneString,	  "\x40",0x02,0xC8,ID_GetPPM		  },
	{NOKIA_ReplyGetPhoneString,	  "\x40",0x02,0xCA,ID_GetProductCode	  },
	{NOKIA_ReplyGetPhoneString,	  "\x40",0x02,0xCC,ID_GetManufactureMonth },
	{NOKIA_ReplyGetPhoneString,	  "\x40",0x02,0xCC,ID_GetOriginalIMEI	  },
	{NoneReply,			  "\x40",0x02,0xFF,ID_IncomingFrame	  },

	{N71_92_ReplyPhoneSetting,	  "\x7a",0x04,0x02,ID_GetBitmap		  },
	{N71_92_ReplyPhoneSetting,	  "\x7a",0x04,0x02,ID_SetBitmap		  },
	{N71_92_ReplyPhoneSetting,	  "\x7a",0x04,0x15,ID_GetBitmap		  },
	{N71_92_ReplyPhoneSetting,	  "\x7a",0x04,0x15,ID_SetBitmap		  },
	{N71_92_ReplyPhoneSetting,	  "\x7a",0x04,0x17,ID_GetBitmap		  },
	{N71_92_ReplyPhoneSetting,	  "\x7a",0x04,0x17,ID_SetBitmap		  },

	{DCT3DCT4_ReplyGetModelFirmware,  "\xD2",0x02,0x00,ID_GetModel		  },
	{DCT3DCT4_ReplyGetModelFirmware,  "\xD2",0x02,0x00,ID_GetFirmware	  },
	{DCT3_ReplyPressKey,		  "\xD2",0x02,0x46,ID_PressKey		  },
	{DCT3_ReplyPressKey,		  "\xD2",0x02,0x47,ID_PressKey		  },

	{NULL,				  "\x00",0x00,0x00,ID_None		  }
};

GSM_Phone_Functions N7110Phone = {
	"6210|6250|7110|7190",
	N7110ReplyFunctions,
	N7110_Initialise,
	PHONE_Terminate,
	GSM_DispatchMessage,
	NOTSUPPORTED,			/* 	ShowStartInfo		*/
	NOKIA_GetManufacturer,
	DCT3DCT4_GetModel,
	DCT3DCT4_GetFirmware,
	DCT3_GetIMEI,
	DCT3_GetOriginalIMEI,
	DCT3_GetManufactureMonth,
	DCT3_GetProductCode,
	DCT3_GetHardware,
	DCT3_GetPPM,
	NOTSUPPORTED,			/*	GetSIMIMSI		*/
	N71_92_GetDateTime,
	N71_92_SetDateTime,
	N7110_GetAlarm,
	N7110_SetAlarm,
	NOTSUPPORTED,			/* 	GetLocale		*/
	NOTSUPPORTED,			/* 	SetLocale		*/
	DCT3_PressKey,
	DCT3_Reset,
	N61_71_ResetPhoneSettings,
	NOTSUPPORTED,			/*	EnterSecurityCode	*/
	NOTSUPPORTED,			/*	GetSecurityStatus	*/
	NOTSUPPORTED,			/*	GetDisplayStatus	*/
	NOTIMPLEMENTED,			/*	SetAutoNetworkLogin	*/
	N71_92_GetBatteryCharge,
	N71_92_GetSignalQuality,
	DCT3_GetNetworkInfo,
	NOTSUPPORTED,       		/*  	GetCategory 		*/
 	NOTSUPPORTED,       		/*  	AddCategory 		*/
        NOTSUPPORTED,        		/*  	GetCategoryStatus 	*/
	N7110_GetMemoryStatus,
	N7110_GetMemory,
	NOTIMPLEMENTED,			/*	GetNextMemory		*/
	N7110_SetMemory,
	NOTIMPLEMENTED,			/*	AddMemory		*/
	N7110_DeleteMemory,
	NOTIMPLEMENTED,			/*	DeleteAllMemory		*/
	N7110_GetSpeedDial,
	NOTIMPLEMENTED,			/*	SetSpeedDial		*/
	DCT3_GetSMSC,
	DCT3_SetSMSC,
	N7110_GetSMSStatus,
	N7110_GetSMSMessage,
	N7110_GetNextSMSMessage,
	N7110_SetSMS,
	N7110_AddSMS,
	N7110_DeleteSMS,
	DCT3_SendSMSMessage,
	NOTSUPPORTED,			/*	SendSavedSMS		*/
	NOTSUPPORTED,			/*	SetFastSMSSending	*/
	N7110_SetIncomingSMS,
	DCT3_SetIncomingCB,
	N7110_GetSMSFolders,
 	NOTIMPLEMENTED,			/* 	AddSMSFolder		*/
 	NOTIMPLEMENTED,			/* 	DeleteSMSFolder		*/
	DCT3_DialVoice,
        NOTIMPLEMENTED,			/*	DialService		*/
	N7110_AnswerCall,
	DCT3_CancelCall,
 	NOTIMPLEMENTED,			/* 	HoldCall 		*/
 	NOTIMPLEMENTED,			/* 	UnholdCall 		*/
 	NOTIMPLEMENTED,			/* 	ConferenceCall 		*/
 	NOTIMPLEMENTED,			/* 	SplitCall		*/
 	NOTIMPLEMENTED,			/* 	TransferCall		*/
 	NOTIMPLEMENTED,			/* 	SwitchCall		*/
 	NOTSUPPORTED,			/*	GetCallDivert		*/
 	N7110_SetCallDivert,
 	N7110_CancelAllDiverts,
	N7110_SetIncomingCall,
	N7110_SetIncomingUSSD,
	DCT3DCT4_SendDTMF,
	N7110_GetRingtone,
	N7110_SetRingtone,
	NOTSUPPORTED,			/*	GetRingtonesInfo	*/
	NOTSUPPORTED,			/* 	DeleteUserRingtones	*/
	DCT3_PlayTone,
	DCT3_GetWAPBookmark,
	DCT3_SetWAPBookmark,
	DCT3_DeleteWAPBookmark,
	DCT3_GetWAPSettings,
	DCT3_SetWAPSettings,
	NOTSUPPORTED,			/*	GetSyncMLSettings	*/
	NOTSUPPORTED,			/*	SetSyncMLSettings	*/
	NOTSUPPORTED,			/*	GetChatSettings		*/
	NOTSUPPORTED,			/*	SetChatSettings		*/
	NOTSUPPORTED,			/* 	GetMMSSettings		*/
	NOTSUPPORTED,			/* 	SetMMSSettings		*/
	NOTSUPPORTED,			/*	GetMMSFolders		*/
	NOTSUPPORTED,			/*	GetNextMMSFileInfo	*/
	N7110_GetBitmap,
	N7110_SetBitmap,
	NOTSUPPORTED,			/*	GetToDoStatus		*/
	NOTSUPPORTED,			/*	GetToDo			*/
	NOTSUPPORTED,			/*	GetNextToDo		*/
	NOTSUPPORTED,			/*	SetToDo			*/
	NOTSUPPORTED,			/*	AddToDo			*/
	NOTSUPPORTED,			/*	DeleteToDo		*/
	NOTSUPPORTED,			/*	DeleteAllToDo		*/
	N7110_GetCalendarStatus,
	NOTIMPLEMENTED,			/*	GetCalendar		*/
    	N7110_GetNextCalendar,
	NOTIMPLEMENTED,			/*	SetCalendar		*/
	N7110_AddCalendar,
	N71_65_DelCalendar,
	NOTIMPLEMENTED,			/*	DeleteAllCalendar	*/
	NOTSUPPORTED,			/* 	GetCalendarSettings	*/
	NOTSUPPORTED,			/* 	SetCalendarSettings	*/
	NOTSUPPORTED,			/*	GetNoteStatus		*/
	NOTSUPPORTED,			/*	GetNote			*/
	NOTSUPPORTED,			/*	GetNextNote		*/
	NOTSUPPORTED,			/*	SetNote			*/
	NOTSUPPORTED,			/*	AddNote			*/
	NOTSUPPORTED,			/* 	DeleteNote		*/
	NOTSUPPORTED,			/*	DeleteAllNotes		*/
	N7110_GetProfile,
	N7110_SetProfile,
    	NOTSUPPORTED,			/*  	GetFMStation        	*/
    	NOTSUPPORTED,			/*  	SetFMStation        	*/
	NOTSUPPORTED,			/*  	ClearFMStations       	*/
	NOTSUPPORTED,			/* 	GetNextFileFolder	*/
	NOTSUPPORTED,			/*	GetFolderListing	*/
	NOTSUPPORTED,			/*	GetNextRootFolder	*/
	NOTSUPPORTED,			/*	SetFileAttributes	*/
	NOTSUPPORTED,			/*	GetFilePart		*/
	NOTSUPPORTED,			/* 	AddFile			*/
	NOTSUPPORTED,			/* 	SendFilePart		*/
	NOTSUPPORTED, 			/* 	GetFileSystemStatus	*/
	NOTSUPPORTED,			/*	DeleteFile		*/
	NOTSUPPORTED,			/*	AddFolder		*/
	NOTSUPPORTED,			/* 	DeleteFolder		*/
	NOTSUPPORTED,			/* 	GetGPRSAccessPoint	*/
	NOTSUPPORTED			/* 	SetGPRSAccessPoint	*/
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
