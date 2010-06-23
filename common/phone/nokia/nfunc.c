
#include <string.h> /* memcpy only */
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include "../../gsmstate.h"
#include "../../misc/coding.h"
#include "../../service/gsmsms.h"
#include "../pfunc.h"
#include "nfunc.h"

unsigned char N71_65_MEMORY_TYPES[] = {
	GMT_DC,		0x01,
	GMT_MC,		0x02,
	GMT_RC,		0x03,
	GMT_ME,		0x05,
	GMT_SM,		0x06,
	GMT_VM,		0x09,
	GMT7110_CG,	0x10,
	GMT7110_SP,	0x0e,
	  0x00,		0x00
};

int N71_65_PackPBKBlock(int id, int size, int no, unsigned char *buf, unsigned char *block)
{
	dprintf("Adding block id:%i,number:%i,length:%i\n",id,no+1,size+6);

	*(block++) = id;
	*(block++) = 0;
	*(block++) = 0;
	*(block++) = size + 6;
	*(block++) = no + 1;
	memcpy(block, buf, size);
	block += size;
	*(block++) = 0;

	return (size + 6);
}

int N71_65_EncodePhonebookFrame(unsigned char *req, GSM_PhonebookEntry entry, int *block2, bool URL)
{
	int		count=0, len, i, block=0;
	char		string[500];
	unsigned char	type=0;
 		
	for (i = 0; i < entry.EntriesNum; i++)
	{
		switch (entry.Entries[i].EntryType) {
		case PBK_Number_General:
		case PBK_Number_Mobile:
		case PBK_Number_Work:
		case PBK_Number_Fax:
		case PBK_Number_Home:
			switch (entry.Entries[i].EntryType) {
			case PBK_Number_General:
				string[0] = N7110_NUMBER_GENERAL;	break;
			case PBK_Number_Mobile:
				string[0] = N7110_NUMBER_MOBILE;	break;
			case PBK_Number_Work:
				string[0] = N7110_NUMBER_WORK; 		break;
			case PBK_Number_Fax:
				string[0] = N7110_NUMBER_FAX; 		break;
			case PBK_Number_Home:
				string[0] = N7110_NUMBER_HOME;		break;
			default:					break;
			}
			len = strlen(DecodeUnicodeString(entry.Entries[i].Text));
			string[1] = 0;
			/* DCT 3 */			
			if (!URL) {
				string[2] = entry.Entries[i].VoiceTag;
			}
			string[3] = 0;
			string[4] = len * 2 + 2;     	/* length (with Termination) */
			CopyUnicodeString(string+5,entry.Entries[i].Text);
			string[len * 2 + 5] = 0; 	/* Terminating 0		 */
			count += N71_65_PackPBKBlock(N7110_ENTRYTYPE_NUMBER, len * 2 + 6, block++, string, req + count);
			/* DCT 4 */
			if (URL) {
				block++;
				req[count++] = N6510_ENTRYTYPE_VOICETAG;
				req[count++] = 0;
				req[count++] = 0;
				req[count++] = 8;
				req[count++] = 0x00;
				req[count++] = i+1;
				req[count++] = 0x00;
				req[count++] = entry.Entries[i].VoiceTag;
			}
			break;
		case PBK_Name:
		case PBK_Text_Note:
		case PBK_Text_Postal:
		case PBK_Text_Email:
		case PBK_Text_URL:
			len = strlen(DecodeUnicodeString(entry.Entries[i].Text));
			string[0] = len * 2 + 2;	/* length (with Termination) */
			CopyUnicodeString(string+1,entry.Entries[i].Text);
			string[len * 2 + 1] = 0; 	/* Terminating 0		 */
			switch (entry.Entries[i].EntryType) {
				case PBK_Text_Note:
					type = N7110_ENTRYTYPE_NOTE;	break;
				case PBK_Text_Postal:
					type = N7110_ENTRYTYPE_POSTAL;	break;
				case PBK_Text_Email:
					type = N7110_ENTRYTYPE_EMAIL;	break;
				case PBK_Text_URL:
					type = N7110_ENTRYTYPE_NOTE;
					if (URL) type = N6510_ENTRYTYPE_URL;
					break;
				case PBK_Name:
					type = N7110_ENTRYTYPE_NAME;	break;
				default:				break;
			}
			count += N71_65_PackPBKBlock(type, len * 2 + 2, block++, string, req + count);
			break;
		case PBK_Caller_Group:
			string[0] = entry.Entries[i].Number;
			string[1] = 0;
			count += N71_65_PackPBKBlock(N7110_ENTRYTYPE_GROUP, 2, block++, string, req + count);
			break;
		case PBK_Date:
			break;
		default:
			break;
		}
	}

	*block2=block;

	return count;
}

GSM_Error N71_65_DecodePhonebook(GSM_PhonebookEntry 	*entry,
				 GSM_Bitmap 		*bitmap,
				 GSM_SpeedDial 		*speed,
				 unsigned char 		*MessageBuffer,
				 int 			MessageLength)
{
	unsigned char 	*Block;
	int		length = 0;
	bool		WasBitmap = false;
	            
	entry->EntriesNum = 0;

	if (entry->MemoryType==GMT7110_CG) {
		bitmap->Text[0] = 0x00;
		bitmap->Text[1] = 0x00;
	}

	Block = &MessageBuffer[0];
	while (length != MessageLength) {
#ifdef DEBUG
		dprintf("Phonebook entry block - length %i", Block[3]-6);
		if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) DumpMessage(di.df, Block+5, Block[3]-6);
#endif
		if (entry->EntriesNum==GSM_PHONEBOOK_ENTRIES) {
			dprintf("Too many entries\n");
			return GE_UNKNOWNRESPONSE;
		}

		switch (Block[0]) {
		case N7110_ENTRYTYPE_NAME:
		case N7110_ENTRYTYPE_EMAIL:
		case N7110_ENTRYTYPE_POSTAL:
		case N7110_ENTRYTYPE_NOTE:
		case N6510_ENTRYTYPE_URL:
			if (Block[5]/2>GSM_PHONEBOOK_TEXT_LENGTH) {
				dprintf("Too long text\n");
				return GE_UNKNOWNRESPONSE;
			}
			memcpy(entry->Entries[entry->EntriesNum].Text,Block+6,Block[5]);
			switch (Block[0]) {
			case N7110_ENTRYTYPE_NAME:
				entry->Entries[entry->EntriesNum].EntryType=PBK_Name;
				if (entry->MemoryType==GMT7110_CG) {
					memcpy(bitmap->Text,Block+6,Block[5]);
				}
				dprintf("   Name \"%s\"\n",DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
				break;
			case N7110_ENTRYTYPE_EMAIL:
				entry->Entries[entry->EntriesNum].EntryType=PBK_Text_Email;
				dprintf("   Email \"%s\"\n",DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
				break;
			case N7110_ENTRYTYPE_POSTAL:
				entry->Entries[entry->EntriesNum].EntryType=PBK_Text_Postal;
				dprintf("   Postal \"%s\"\n",DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
				break;
			case N7110_ENTRYTYPE_NOTE:
				entry->Entries[entry->EntriesNum].EntryType=PBK_Text_Note;
				dprintf("   Note \"%s\"\n",DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
				break;
			case N6510_ENTRYTYPE_URL:
				entry->Entries[entry->EntriesNum].EntryType=PBK_Text_URL;
				dprintf("   URL \"%s\"\n",DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
				break;
			}
			entry->EntriesNum ++;
			break;
		case N7110_ENTRYTYPE_DATE:
			entry->Entries[entry->EntriesNum].EntryType=PBK_Date;
			NOKIA_DecodeDateTime(Block+6, &entry->Entries[entry->EntriesNum].Date);
			entry->EntriesNum ++;
			break;
		case N7110_ENTRYTYPE_NUMBER:
			switch (Block[5]) {
			case 0x00:
			case 0x01:	/* Not assigned dialed number */
			case 0x0b:
			case N7110_NUMBER_GENERAL:
				dprintf("  General number");
				entry->Entries[entry->EntriesNum].EntryType=PBK_Number_General;
				break;
			case N7110_NUMBER_WORK:
				dprintf("  Work number");
				entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Work;
				break;
			case N7110_NUMBER_FAX:
				dprintf("  Fax number");
				entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Fax;
				break;
			case N7110_NUMBER_MOBILE:
				dprintf("  Mobile number");
				entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Mobile;
				break;
			case N7110_NUMBER_HOME:
				dprintf("  Home number");
				entry->Entries[entry->EntriesNum].EntryType=PBK_Number_Home;
				break;
			default:
				dprintf("Unknown number type %02x\n",Block[5]);
				return GE_UNKNOWNRESPONSE;
			}
			if (Block[9]/2>GSM_PHONEBOOK_TEXT_LENGTH) {
				dprintf("Too long text\n");
				return GE_UNKNOWNRESPONSE;
			}
			memcpy(entry->Entries[entry->EntriesNum].Text,Block+10,Block[9]);
			dprintf(" \"%s\"\n",DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
			/* DCT3 phones like 6210 */
			entry->Entries[entry->EntriesNum].VoiceTag = Block[7];
#ifdef DEBUG
			if (entry->Entries[entry->EntriesNum].VoiceTag != 0) dprintf("Voice tag %i assigned\n",Block[7]);
#endif
			entry->EntriesNum ++;
			break;
		case N7110_ENTRYTYPE_RINGTONE:
			if (entry->MemoryType==GMT7110_CG) {
				bitmap->Ringtone=Block[5];
				dprintf("Ringtone ID : %i\n",Block[5]);
			} else return GE_UNKNOWNRESPONSE;
			break;
		case N7110_ENTRYTYPE_LOGOON:
			if (entry->MemoryType==GMT7110_CG) {
				bitmap->Enabled=(Block[5]==0x00 ? false : true);
				dprintf("Logo : %s\n", bitmap->Enabled==true ? "enabled":"disabled");
			} else return GE_UNKNOWNRESPONSE;
			break;
		case N7110_ENTRYTYPE_GROUPLOGO:
			if (entry->MemoryType==GMT7110_CG) {
				dprintf("Caller logo\n");
				PHONE_DecodeBitmap(GSM_NokiaCallerLogo, Block+10, bitmap);
				WasBitmap = true;
			} else return GE_UNKNOWNRESPONSE;
			break;
		case N7110_ENTRYTYPE_GROUP:
			entry->Entries[entry->EntriesNum].EntryType=PBK_Caller_Group;
			dprintf("Caller group \"%i\"\n",Block[5]);
			entry->Entries[entry->EntriesNum].Number=Block[5];
			if (Block[5]!=0) entry->EntriesNum ++;
			break;
		case N6510_ENTRYTYPE_VOICETAG:
			dprintf("Entry %i has voice tag %i\n",Block[5]-1,Block[7]);
			entry->Entries[Block[5]-1].VoiceTag = Block[7];
			break;
		/* 6210 5.56, SIM speed dials or ME with 1 number */
		case N7110_ENTRYTYPE_SIM_SPEEDDIAL:
			if (entry->MemoryType==GMT7110_SP) {
#ifdef DEBUG
				dprintf("location %i\n",(Block[6]*256+Block[7]));
#endif			
				speed->MemoryType = GMT_ME;
				if (Block[8] == 0x06) speed->MemoryType = GMT_SM;
				speed->MemoryLocation 	= (Block[6]*256+Block[7]);
				speed->MemoryNumberID 	= 2;
			} else return GE_UNKNOWNRESPONSE;
			break;
		case N7110_ENTRYTYPE_SPEEDDIAL:
			if (entry->MemoryType==GMT7110_SP) {
#ifdef DEBUG
				switch (Block[12]) {
					case 0x05: dprintf("ME\n"); break;
					case 0x06: dprintf("SM\n"); break;
					default	 : dprintf("%02x\n",Block[12]);
				}
				dprintf("location %i, number %i in location\n",
					(Block[6]*256+Block[7])-1,Block[14]);
#endif			
				switch (Block[12]) {
					case 0x05: speed->MemoryType = GMT_ME; break;
					case 0x06: speed->MemoryType = GMT_SM; break;
				}
				speed->MemoryLocation = (Block[6]*256+Block[7])-1;
				speed->MemoryNumberID = Block[14];
			} else return GE_UNKNOWNRESPONSE;
			break;
		default:
			dprintf("ERROR: unknown pbk entry %i\n",Block[0]);
			return GE_UNKNOWNRESPONSE;
		}
		length=length + Block[3];
		Block = &Block[(int) Block[3]];
	}

	/* In DCT4 default caller logos are NOT return */
	if (!WasBitmap && entry->MemoryType==GMT7110_CG) return GE_SECURITYERROR;

	if (entry->EntriesNum == 0) return GE_EMPTY;

	return GE_NONE;
}

void NOKIA_GetDefaultCallerGroupName(GSM_StateMachine *s, GSM_Bitmap *Bitmap)
{
	Bitmap->DefaultName = false;
	if (Bitmap->Text[0]==0x00 && Bitmap->Text[1]==0x00) {
		Bitmap->DefaultName = true;
		switch(Bitmap->Location) {
		case 1:
			EncodeUnicode(Bitmap->Text,GetMsg(s->msg,"Family"),strlen(GetMsg(s->msg,"Family")));
			break;
		case 2:
			EncodeUnicode(Bitmap->Text,GetMsg(s->msg,"VIP"),strlen(GetMsg(s->msg,"VIP")));
			break;
		case 3:
			EncodeUnicode(Bitmap->Text,GetMsg(s->msg,"Friends"),strlen(GetMsg(s->msg,"Friends")));
			break;
		case 4:
			EncodeUnicode(Bitmap->Text,GetMsg(s->msg,"Colleagues"),strlen(GetMsg(s->msg,"Colleagues")));
			break;
		case 5:
			EncodeUnicode(Bitmap->Text,GetMsg(s->msg,"Other"),strlen(GetMsg(s->msg,"Other")));
			break;
		}
	}
}

void NOKIA_DecodeDateTime(unsigned char* buffer, GSM_DateTime *datetime)
{
	datetime->Year	= buffer[0] * 256 + buffer[1];
	datetime->Month	= buffer[2];
	datetime->Day	= buffer[3];

	datetime->Hour	 = buffer[4];
	datetime->Minute = buffer[5];
	datetime->Second = buffer[6];

	dprintf("Decoding date and time\n");
	dprintf("   Time: %02d:%02d:%02d\n",
		datetime->Hour, datetime->Minute, datetime->Second);
	dprintf("   Date: %4d/%02d/%02d\n",
		datetime->Year, datetime->Month, datetime->Day);
}

#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)

/* --------------------- Some general Nokia functions ---------------------- */

void NOKIA_DecodeSMSState(unsigned char state, GSM_SMSMessage *sms)
{
	switch (state) {
		case 0x01 : sms->State = GSM_Read;   break;
		case 0x03 : sms->State = GSM_UnRead; break;
		case 0x05 : sms->State = GSM_Sent;   break;
		case 0x07 : sms->State = GSM_UnSent; break;
		default	  : dprintf("Unknown SMS state: %02x\n",state);
	}
}

GSM_Error NOKIA_ReplyGetPhoneString(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	strcpy(Data->PhoneString,msg.Buffer+Data->StartPhoneString);
	return GE_NONE;
}

/* Some strings are very easy. Some header, after it required string and 0x00.
 * We can get them using this function. We give frame to send (*string),
 * type of message (type), pointer for buffer for response (*value), request
 * type (request) and what is start byte in response for our string
 */
GSM_Error NOKIA_GetPhoneString(GSM_StateMachine *s, unsigned char *msgframe, int msglen, unsigned char msgtype, char *retvalue, GSM_Phone_RequestID request, int startresponse)
{
	retvalue[0] = 0;
	s->Phone.Data.StartPhoneString = startresponse;
	s->Phone.Data.PhoneString = retvalue;
	return GSM_WaitFor (s, msgframe, msglen,msgtype, 4, request);
}

GSM_Error NOKIA_GetManufacturer(GSM_StateMachine *s, char *manufacturer)
{
	EncodeUnicode(manufacturer,"Nokia",5);
	return GE_NONE;
}

/* Many functions contains such strings:
 * (1. length/256) - exist or not
 * 2. length%256
 * 3. string (unicode, no termination)
 * This function read string to output and increases counter
 */
void NOKIA_GetUnicodeString(int *current, unsigned char *input, unsigned char *output, bool FullLength)
{
	int length;

	if (FullLength) {
		length = (input[*current]*256+input[*current+1])*2;
		memcpy(output,input+(*current+2),length);
		*current = *current + 2 + length;
	} else {
		length = (input[*current])*2;
		memcpy(output,input+(*current+1),length);
		*current = *current + 1 + length;
	}

	output[length  ] = 0;
	output[length+1] = 0;
}

int NOKIA_SetUnicodeString(unsigned char *dest, unsigned char *string, bool FullLength)
{
	int length;
	
	length = strlen(DecodeUnicodeString(string));
	if (FullLength) {
		dest[0] = length / 256;
		dest[1] = length % 256;
		CopyUnicodeString(dest + 2, string);
		return 2+length*2;		
	} else {
		dest[0] = length % 256;
		CopyUnicodeString(dest + 1, string);
		return 1+length*2;
	}
}

/* Returns correct ID for concrete memory type */
GSM_MemoryType NOKIA_GetMemoryType(GSM_MemoryType memory_type, unsigned char *ID)
{
	int i=0;

	while (ID[i+1]!=0x00) {
		if (ID[i]==memory_type) return ID[i+1];
		i=i+2;
	}
	return 0xff;
}

void NOKIA_EncodeDateTime(unsigned char* buffer, GSM_DateTime *datetime)
{
	buffer[0] = datetime->Year / 256;
	buffer[1] = datetime->Year % 256;
	buffer[2] = datetime->Month;
	buffer[3] = datetime->Day;

	buffer[4] = datetime->Hour;
	buffer[5] = datetime->Minute;
}

void NOKIA_SortSMSFolderStatus(GSM_NOKIASMSFolder *Folder)
{
	int i,j;

	if (Folder->Number!=0) {
		/* Bouble sorting */
		i=0;
		while (i!=Folder->Number-1) {
			if (Folder->Location[i]>Folder->Location[i+1]) {
				j=Folder->Location[i];
				Folder->Location[i]=Folder->Location[i+1];
				Folder->Location[i+1]=j;
				i=0;
			} else {
				i++;
			}
		}
#ifdef DEBUG
		dprintf("Locations: ");
		for (i=0;i<Folder->Number;i++) {
			dprintf("%i ",Folder->Location[i]);
		}
		dprintf("\n");
#endif
	}
}

void NOKIA_GetDefaultProfileName(GSM_StateMachine *s, GSM_Profile *Profile)
{
	if (Profile->DefaultName) {
		switch(Profile->Location) {
			case 1:	EncodeUnicode(Profile->Name,GetMsg(s->msg,"General"),strlen(GetMsg(s->msg,"General")));
				break;
			case 2: EncodeUnicode(Profile->Name,GetMsg(s->msg,"Silent"),strlen(GetMsg(s->msg,"Silent")));
				break;
			case 3: EncodeUnicode(Profile->Name,GetMsg(s->msg,"Meeting"),strlen(GetMsg(s->msg,"Meeting")));
				break;
			case 4:	EncodeUnicode(Profile->Name,GetMsg(s->msg,"Outdoor"),strlen(GetMsg(s->msg,"Outdoor")));
				break;
			case 5:	EncodeUnicode(Profile->Name,GetMsg(s->msg,"Pager"),strlen(GetMsg(s->msg,"Pager")));
				break;
			case 6: EncodeUnicode(Profile->Name,GetMsg(s->msg,"Car"),strlen(GetMsg(s->msg,"Car")));
				break;
			case 7: EncodeUnicode(Profile->Name,GetMsg(s->msg,"Headset"),strlen(GetMsg(s->msg,"Headset")));
				break;
		}
	}
}

/* - Shared for DCT3 (n6110.c, n7110.c, n9110.c) and DCT4 (n6510.c) phones - */

GSM_Error DCT3DCT4_ReplyGetWAPBookmark(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User, bool FullLength)
{
	int tmp;

	dprintf("WAP bookmark received\n");
	switch (msg.Buffer[3]) {
	case 0x07:
		tmp = 4;

		Data->WAPBookmark->Location = msg.Buffer[tmp] * 256 + msg.Buffer[tmp+1];
		dprintf("Location: %i\n",Data->WAPBookmark->Location);
		tmp = tmp + 2;

 		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPBookmark->Title, FullLength);
		dprintf("Title   : \"%s\"\n",DecodeUnicodeString(Data->WAPBookmark->Title));

 		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPBookmark->Address, FullLength);
		dprintf("Address : \"%s\"\n",DecodeUnicodeString(Data->WAPBookmark->Address));

		return GE_NONE;
	case 0x08:
		switch (msg.Buffer[4]) {
		case 0x01:
			dprintf("Security error. Inside WAP bookmarks menu\n");
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

GSM_Error DCT3DCT4_ReplySetWAPBookmark(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x0A:
		dprintf("WAP bookmark set OK\n");
		return GE_NONE;
	case 0x0B:
		dprintf("WAP bookmark setting error\n");
		switch (msg.Buffer[4]) {
		case 0x01:
			dprintf("Security error. Inside WAP bookmarks menu\n");
			return GE_INSIDEPHONEMENU;
		case 0x02:
			dprintf("Can't write to empty location ?\n");
			return GE_EMPTY;
		case 0x04:
			dprintf("Full memory\n");
			return GE_FULL;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
			return GE_UNKNOWNRESPONSE;
		}
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error DCT3DCT4_ReplyEnableWAP(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("WAP functions enabled\n");
	return GE_NONE;
}

GSM_Error DCT3DCT4_EnableWAP(GSM_StateMachine *s)
{
	unsigned char req[] = { N6110_FRAME_HEADER, 0x00 };

	/* Check if have WAP in phone */
	if (IsPhoneFeatureAvailable(s->Model,F_NOWAP)) return GE_NOTSUPPORTED;

	dprintf("Enabling WAP\n");
	return GSM_WaitFor (s, req, 4, 0x3f, 4, ID_EnableWAP);
}

GSM_Error DCT3DCT4_ReplyDelWAPBookmark(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x0D:
		dprintf("WAP bookmark deleted OK\n");
		return GE_NONE;
	case 0x0E:
		dprintf("WAP bookmark deleting error\n");
		switch (msg.Buffer[4]) {
		case 0x01:
			dprintf("Security error. Inside WAP bookmarks menu\n");
			return GE_SECURITYERROR;
		case 0x02:
			dprintf("Invalid location\n");
			return GE_INVALIDLOCATION;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
			return GE_UNKNOWNRESPONSE;
		}
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error DCT3DCT4_DeleteWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error 	error;
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x0C,
		0x00, 0x00};		/* Location */

	/* For now !!! */
	if (!strcmp(GetModelData(NULL,s->Model,NULL)->model,"3510")) {
		if (s->VerNum>3.37) return GE_NOTSUPPORTED;
	}

	/* We have to enable WAP frames in phone */
	error=DCT3DCT4_EnableWAP(s);
	if (error!=GE_NONE) return error;

	req[5] = bookmark->Location;

	dprintf("Deleting WAP bookmark\n");
	return GSM_WaitFor (s, req, 6, 0x3f, 4, ID_DeleteWAPBookmark);
}

GSM_Error DCT3DCT4_GetWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error error;
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x06,
		0x00, 0x00};		/* Location */

	/* We have to enable WAP frames in phone */
	error=DCT3DCT4_EnableWAP(s);
	if (error!=GE_NONE) return error;

	req[5]=bookmark->Location-1;

	s->Phone.Data.WAPBookmark=bookmark;
	dprintf("Getting WAP bookmark\n");
	return GSM_WaitFor (s, req, 6, 0x3f, 4, ID_GetWAPBookmark);
}

GSM_Error DCT3DCT4_ReplyGetModelFirmware(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	GSM_Lines lines;

	SplitLines(msg.Buffer, msg.Length, &lines, "\x20\x0A", 2, false);

	strcpy(Data->Model,GetLineString(msg.Buffer, lines, 4));
	dprintf("Received model %s\n",Data->Model);

	strcpy(Data->VersionDate,GetLineString(msg.Buffer, lines, 3));
	dprintf("Received firmware date %s\n",Data->VersionDate);

	strcpy(Data->Version,GetLineString(msg.Buffer, lines, 2));
	dprintf("Received firmware version %s\n",Data->Version);
	GSM_CreateFirmwareNumber(Data);

	return GE_NONE;
}

GSM_Error DCT3DCT4_GetModel (GSM_StateMachine *s)
{
	unsigned char 	req[5] = {N6110_FRAME_HEADER, 0x03, 0x00};
	GSM_Error 	error;

	if (strlen(s->Model)>0) return GE_NONE;

	dprintf("Getting model\n");
	error=GSM_WaitFor (s, req, 5, 0xd1, 3, ID_GetModel);
	if (error==GE_NONE) {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
			smprintf(s, "[Connected model  - \"%s\"]\n",s->Phone.Data.Model);
			smprintf(s, "[Firmware version - \"%s\"]\n",s->Phone.Data.Version);
			smprintf(s, "[Firmware date    - \"%s\"]\n",s->Phone.Data.VersionDate);
		}
	}
	return error;
}

GSM_Error DCT3DCT4_GetFirmware (GSM_StateMachine *s)
{
	unsigned char req[5] = {N6110_FRAME_HEADER, 0x03, 0x00};  
	GSM_Error error;

	if (strlen(s->Ver)>0) return GE_NONE;
	
	dprintf("Getting firmware version\n");
	error=GSM_WaitFor (s, req, 5, 0xd1, 3, ID_GetFirmware);
	if (error==GE_NONE) {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
			smprintf(s, "[Connected model  - \"%s\"]\n",s->Phone.Data.Model);
			smprintf(s, "[Firmware version - \"%s\"]\n",s->Phone.Data.Version);
			smprintf(s, "[Firmware date    - \"%s\"]\n",s->Phone.Data.VersionDate);
		}
	}
	return error;
}

/* ---------- Shared for n7110.c and n6510.c ------------------------------- */

GSM_Error N71_65_ReplyDeleteMemory(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Phonebook entry deleted\n");
	return GE_NONE;
}

GSM_Error N71_65_DeleteMemory(GSM_StateMachine *s, GSM_PhonebookEntry *entry, unsigned char *memory)
{
	unsigned char req[] = {
		N7110_FRAME_HEADER, 0x0f, 0x00, 0x01,
		0x04, 0x00, 0x00, 0x0c, 0x01, 0xff,
		0x00, 0x01,		/* location	*/
		0x05, 			/* memory type	*/
		0x00, 0x00, 0x00};

	req[12] = (entry->Location >> 8);
	req[13] = entry->Location & 0xff;

	req[14] = NOKIA_GetMemoryType(entry->MemoryType,memory);
	if (req[14]==0xff) return GE_NOTSUPPORTED;

	dprintf("Deleting phonebook entry\n");
	return GSM_WaitFor (s, req, 18, 0x03, 4, ID_SetMemory);
}

GSM_Error N71_65_ReplyGetMemoryError(unsigned char error, GSM_Phone_Data *Data)
{
	switch (error) {
	case 0x30:
		dprintf("Invalid memory type\n");
		if (Data->Memory->MemoryType == GMT_ME) return GE_EMPTY;
		return GE_NOTSUPPORTED;
	case 0x33:
		dprintf("Empty location\n");
		Data->Memory->EntriesNum = 0;
		return GE_NONE;
	case 0x34:
		dprintf("Too high location ?\n");
		return GE_INVALIDLOCATION;
	default:
		dprintf("ERROR: unknown %i\n",error);
		return GE_UNKNOWNRESPONSE;
	}
}

GSM_Error N71_65_ReplyWritePhonebook(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Phonebook entry written ");
	switch (msg.Buffer[6]) {
	case 0x0f:
		dprintf(" - error\n");
		switch (msg.Buffer[10]) {
		case 0x36:
			dprintf("Too long name\n");
			return GE_NOTSUPPORTED;
		case 0x3d:
			dprintf("Wrong entry type\n");
			return GE_NOTSUPPORTED;
		case 0x3e:
			dprintf("Too much entries\n");
			return GE_NOTSUPPORTED;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[10]);
			return GE_UNKNOWNRESPONSE;
		}
	default:
		dprintf(" - OK\n");
		return GE_NONE;
	}
}

bool NOKIA_FindPhoneFeatureValue(GSM_Profile_PhoneTableValue 	ProfileTable[],
				 GSM_Profile_Feat_ID		FeatureID,
				 GSM_Profile_Feat_Value		FeatureValue,
			    	 unsigned char 			*PhoneID,
			    	 unsigned char 			*PhoneValue) 
{
	int i=0;

	dprintf("Trying to find feature %i with value %i\n",FeatureID,FeatureValue);
	while (ProfileTable[i].ID != 0x00) {
		if (ProfileTable[i].ID == FeatureID &&
		    ProfileTable[i].Value == FeatureValue) {
			*PhoneID	= ProfileTable[i].PhoneID;
			*PhoneValue	= ProfileTable[i].PhoneValue;
			return true;
		}
		i++;
	}
	return false;
}

#define PROFILE_CALLERGROUPS_GROUP1      0x01
#define PROFILE_CALLERGROUPS_GROUP2      0x02
#define PROFILE_CALLERGROUPS_GROUP3      0x04
#define PROFILE_CALLERGROUPS_GROUP4      0x08
#define PROFILE_CALLERGROUPS_GROUP5      0x10

void NOKIA_FindFeatureValue(GSM_Profile_PhoneTableValue ProfileTable[],
			    unsigned char 		ID,
			    unsigned char 		Value,
			    GSM_Phone_Data 		*Data,
			    bool			CallerGroups) 
{
	int i;

	if (CallerGroups) {
		dprintf("Caller groups: %i\n", Value);
		Data->Profile->FeatureID [Data->Profile->FeaturesNumber] = Profile_CallerGroups;
		Data->Profile->FeaturesNumber++;
		for (i=0;i<5;i++) Data->Profile->CallerGroups[i] = false;
		if ((Value & PROFILE_CALLERGROUPS_GROUP1)==PROFILE_CALLERGROUPS_GROUP1) Data->Profile->CallerGroups[0] = true;
		if ((Value & PROFILE_CALLERGROUPS_GROUP2)==PROFILE_CALLERGROUPS_GROUP2) Data->Profile->CallerGroups[1] = true;
		if ((Value & PROFILE_CALLERGROUPS_GROUP3)==PROFILE_CALLERGROUPS_GROUP3) Data->Profile->CallerGroups[2] = true;
		if ((Value & PROFILE_CALLERGROUPS_GROUP4)==PROFILE_CALLERGROUPS_GROUP4) Data->Profile->CallerGroups[3] = true;
		if ((Value & PROFILE_CALLERGROUPS_GROUP5)==PROFILE_CALLERGROUPS_GROUP5) Data->Profile->CallerGroups[4] = true;
		return;
	}

	i = 0;
	while (ProfileTable[i].ID != 0x00) {
		if (ProfileTable[i].PhoneID == ID &&
		    ProfileTable[i].PhoneValue == Value) {
#ifdef DEBUG
			switch (ProfileTable[i].ID) {
			case Profile_KeypadTone		: dprintf("Keypad tones\n"); 	 	  break;
			case Profile_CallAlert		: dprintf("Call alert\n"); 		  break;
			case Profile_RingtoneVolume	: dprintf("Ringtone volume\n"); 	  break;
			case Profile_MessageTone	: dprintf("SMS message tones\n");  	  break;
			case Profile_Vibration		: dprintf("Vibration\n"); 		  break;
			case Profile_WarningTone	: dprintf("Warning (ang games) tones\n"); break;
			case Profile_AutoAnswer		: dprintf("Automatic answer\n"); 	  break;
			case Profile_Lights		: dprintf("Lights\n"); 			  break;
			case Profile_ScreenSaver	: dprintf("Screen Saver\n"); 		  break;
			case Profile_ScreenSaverTime	: dprintf("Screen Saver timeout\n");	  break;
			default				:					  break;
			}
#endif
			Data->Profile->FeatureID	[Data->Profile->FeaturesNumber] = ProfileTable[i].ID;
			Data->Profile->FeatureValue	[Data->Profile->FeaturesNumber] = ProfileTable[i].Value;
			Data->Profile->FeaturesNumber++;
			break;
		}
		i++;
	}
}

GSM_Profile_PhoneTableValue Profile71_65[] = {
	{Profile_KeypadTone,	 PROFILE_KEYPAD_OFF,		0x00,0x00},
	{Profile_KeypadTone,	 PROFILE_KEYPAD_LEVEL1,		0x00,0x01},
	{Profile_KeypadTone,	 PROFILE_KEYPAD_LEVEL2,		0x00,0x02},
	{Profile_KeypadTone,	 PROFILE_KEYPAD_LEVEL3,		0x00,0x03},
	/* Lights ? */
	{Profile_CallAlert,	 PROFILE_CALLALERT_RINGING,	0x02,0x00},
	{Profile_CallAlert,	 PROFILE_CALLALERT_ASCENDING,	0x02,0x01},
	{Profile_CallAlert,	 PROFILE_CALLALERT_RINGONCE,	0x02,0x02},
	{Profile_CallAlert,	 PROFILE_CALLALERT_BEEPONCE,	0x02,0x03},
	{Profile_CallAlert,	 PROFILE_CALLALERT_OFF,		0x02,0x05},
/*	{Profile_CallAlert,	 PROFILE_CALLALERT_CALLERGROUPS,0x02,0x07},	*/
	/* Ringtone ID */
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL1,		0x04,0x00},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL2,		0x04,0x01},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL3,		0x04,0x02},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL4,		0x04,0x03},
	{Profile_RingtoneVolume, PROFILE_VOLUME_LEVEL5,		0x04,0x04},
	{Profile_MessageTone,	 PROFILE_MESSAGE_NOTONE,	0x05,0x00},
	{Profile_MessageTone,	 PROFILE_MESSAGE_STANDARD,	0x05,0x01},
	{Profile_MessageTone,	 PROFILE_MESSAGE_SPECIAL,	0x05,0x02},
	{Profile_MessageTone,	 PROFILE_MESSAGE_BEEPONCE,	0x05,0x03},
	{Profile_MessageTone,	 PROFILE_MESSAGE_ASCENDING,	0x05,0x04},
	{Profile_Vibration,	 PROFILE_VIBRATION_OFF,		0x06,0x00},
	{Profile_Vibration,	 PROFILE_VIBRATION_ON,		0x06,0x01},
	{Profile_WarningTone,	 PROFILE_WARNING_OFF,		0x07,0x00},
	{Profile_WarningTone,	 PROFILE_WARNING_ON,		0x07,0x01},
	/* Caller groups */
	{Profile_AutoAnswer,	 PROFILE_AUTOANSWER_OFF,	0x09,0x00},
	{Profile_AutoAnswer,	 PROFILE_AUTOANSWER_ON,		0x09,0x01},
	{0x00,			 0x00,				0x00,0x00}
};

GSM_Error NOKIA_SetIncomingSMS(GSM_StateMachine *s, bool enable)
{
	s->Phone.Data.EnableIncomingSMS = enable;
#ifdef DEBUG
	if (enable) {
		dprintf("Enabling incoming SMS\n");
	} else {
		dprintf("Disabling incoming SMS\n");
	}
#endif
	return GE_NONE;
}

GSM_Error N71_65_ReplyCallInfo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	GSM_Call 	call;
	int			tmp;
#ifdef DEBUG
	unsigned char 	buffer[200];

	switch (msg.Buffer[3]) {
	case 0x02 : dprintf("Call established, remote phone is ringing.\n"); 	break;
	case 0x03 : dprintf("Call complete\n"); 				break;
	case 0x04 : dprintf("Call hangup!\n"); 					break;
	case 0x05 : dprintf("Incoming call\n"); 				break;
	case 0x07 : dprintf("Call answer initiated.\n"); 			break;
	case 0x09 : dprintf("Call released.\n"); 				break;
	case 0x0a : dprintf("Call is being released.\n"); 			break;
	case 0x0c : dprintf("Audio status\n"); 					break;
	case 0x53 : dprintf("Outgoing call\n");  				break;
	}

	if (msg.Buffer[3] != 0x0c) dprintf("Call ID   : %i\n",msg.Buffer[4]);

	switch (msg.Buffer[3]) {
	case 0x03:
	case 0x05:
	case 0x53:		
		dprintf("Call mode : %i\n",msg.Buffer[5]);
		tmp = 6;
		NOKIA_GetUnicodeString(&tmp, msg.Buffer,buffer,false);
		dprintf("Number    : \"%s\"\n",DecodeUnicodeString(buffer));
		break;
	case 0x04:
		dprintf("Cause Type : %i\n", msg.Buffer[5]);
		dprintf("Cause ID   : %i\n", msg.Buffer[6]);
		break;
	case 0x0c:
		if (msg.Buffer[4] == 0x01) dprintf("Audio enabled\n");
				      else dprintf("Audio disabled\n");
		break;
	default:
		break;
	}
#endif
	if (User->IncomingCall) {
		switch (msg.Buffer[3]) {
		case 0x02:
			call.Status = GN_CALL_CallStart;
			break;
		case 0x04:
			call.Status = GN_CALL_CallRemoteEnd;
			break;
		case 0x05:
			call.Status = GN_CALL_IncomingCall;
			tmp = 6;
			NOKIA_GetUnicodeString(&tmp, msg.Buffer,call.PhoneNumber,false);
			break;
		case 0x09:
			call.Status = GN_CALL_CallLocalEnd;
			break;
		case 0x53:
			call.Status = GN_CALL_OutgoingCall;
			tmp = 6;
			NOKIA_GetUnicodeString(&tmp, msg.Buffer,call.PhoneNumber,false);
			break;
		default:
			return GE_NONE;
		}
		User->IncomingCall(Data->Device, call);
	}

	return GE_NONE;
}

static time_t Fill_Time_T(GSM_DateTime DT)
{
	struct tm tm_starttime;

	dprintf("  StartTime  : %02i-%02i-%04i %02i:%02i:%02i\n",
		DT.Day,DT.Month,DT.Year,DT.Hour,DT.Minute,DT.Second);

#ifdef WIN32
	putenv("TZ=PST+8");
#else
	setenv("TZ","PST+8",1);
#endif
	tzset();

	memset(&tm_starttime, 0, sizeof(tm_starttime));
	tm_starttime.tm_year 	= DT.Year - 1900;
	tm_starttime.tm_mon  	= DT.Month - 1;
	tm_starttime.tm_mday 	= DT.Day;
	tm_starttime.tm_hour 	= DT.Hour;
	tm_starttime.tm_min  	= DT.Minute;
	tm_starttime.tm_sec  	= DT.Second;
	tm_starttime.tm_isdst	= 0;
	
	return mktime(&tm_starttime);
}

static void N71_65_GetTimeDiffence(unsigned long diff, GSM_DateTime *DT, bool Plus, int multi)
{
	time_t     	t_time;
	struct tm  	*tm_endtime;

	t_time = Fill_Time_T(*DT);

	if (Plus) {
		t_time 		+= diff*multi;
	} else {
		t_time 		-= diff*multi;
	}

	tm_endtime 		= localtime(&t_time);
	DT->Year   		= tm_endtime->tm_year + 1900;
	DT->Month  		= tm_endtime->tm_mon + 1;
	DT->Day    		= tm_endtime->tm_mday;
	DT->Hour   		= tm_endtime->tm_hour;
	DT->Minute 		= tm_endtime->tm_min;
	DT->Second 		= tm_endtime->tm_sec;
	dprintf("  EndTime    : %02i-%02i-%04i %02i:%02i:%02i\n",
		DT->Day,DT->Month,DT->Year,DT->Hour,DT->Minute,DT->Second);
}

static void N71_65_GetCalendarRecurrance(unsigned char *buffer, GSM_CalendarEntry *entry)
{
	int Recurrance;

	Recurrance = buffer[0]*256 + buffer[1];
	if (Recurrance == 0xffff) Recurrance=8760; /* 0xffff -> 1 Year (8760 hours) */
	if (Recurrance != 0) {
		dprintf("Recurrance   : %i hours\n",Recurrance);
		entry->Entries[entry->EntriesNum].EntryType	= CAL_RECURRANCE;
		entry->Entries[entry->EntriesNum].Number	= Recurrance;
		entry->EntriesNum++;
	}
}

static void N71_65_GetCalendarAlarm(unsigned char *buffer, GSM_CalendarEntry *entry, int DT, GSM_Phone_Data *Data)
{
	unsigned long diff;

	if (buffer[0] == 0x00 && buffer[1] == 0x00 && buffer[2] == 0xff && buffer[3] == 0xff)
	{
		dprintf("No alarm\n");
	} else {
		memcpy(&entry->Entries[entry->EntriesNum].Date,&entry->Entries[DT].Date,sizeof(GSM_DateTime));

		diff  = ((unsigned int)buffer[0]) << 24;
		diff += ((unsigned int)buffer[1]) << 16;
		diff += ((unsigned int)buffer[2]) << 8;
		diff += buffer[3];
		dprintf("  Difference : %i seconds\n", diff);

		switch (entry->Type) {
		case GCN_MEETING:
			N71_65_GetTimeDiffence(diff, &entry->Entries[entry->EntriesNum].Date, false, 60);
			break;
		case GCN_REMINDER:
			if (!IsPhoneFeatureAvailable(Data->Model, F_CAL35)) {
				N71_65_GetTimeDiffence(diff, &entry->Entries[entry->EntriesNum].Date, false, 60);
				break;
			}
		case GCN_CALL:
			if (!IsPhoneFeatureAvailable(Data->Model, F_CAL35)) {
				N71_65_GetTimeDiffence(diff, &entry->Entries[entry->EntriesNum].Date, false, 60);
				break;
			}
		default:
			N71_65_GetTimeDiffence(diff, &entry->Entries[entry->EntriesNum].Date, false, 1);
		}
		dprintf("Alarm date   : %02i-%02i-%04i %02i:%02i:%02i\n",
			entry->Entries[entry->EntriesNum].Date.Day,   entry->Entries[entry->EntriesNum].Date.Month,
			entry->Entries[entry->EntriesNum].Date.Year,  entry->Entries[entry->EntriesNum].Date.Hour,
			entry->Entries[entry->EntriesNum].Date.Minute,entry->Entries[entry->EntriesNum].Date.Second);

		entry->Entries[entry->EntriesNum].EntryType = CAL_ALARM_DATETIME;
		if (entry->Type == GCN_BIRTHDAY) {
			if (buffer[14]!=0x00) entry->Entries[entry->EntriesNum].EntryType = CAL_SILENT_ALARM_DATETIME;
			dprintf("Alarm type   : Silent\n");
		}

		entry->EntriesNum++;
	}
}

/* method 2 */
GSM_Error N71_65_ReplyGetNextCalendar2(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	GSM_DateTime 		Date;
	GSM_CalendarEntry	*entry = Data->Cal;
	int			i;
	unsigned long		diff;

	dprintf("Calendar note received\n");

	if (msg.Length < 10) return GE_EMPTY;

	entry->Location = msg.Buffer[4]*256 + msg.Buffer[5];
	dprintf("Location: %i\n",entry->Location);

	/* Not birthday */
	if (msg.Buffer[21] != 0x04) {
		Date.Year 	= 2030;	Date.Month 	= 01; Date.Day    = 01;
		Date.Hour 	= 00;	Date.Minute 	= 00; Date.Second = 00;
	} else {
		Date.Year 	= 2029; Date.Month 	= 12; Date.Day 	  = 31;
		Date.Hour 	= 22;   Date.Minute 	= 59; Date.Second = 58;
	}
	diff  = ((unsigned int)msg.Buffer[12]) << 24;
	diff += ((unsigned int)msg.Buffer[13]) << 16;
	diff += ((unsigned int)msg.Buffer[14]) << 8;
	diff += msg.Buffer[15];
	dprintf("  Difference : %i seconds\n", diff);
	N71_65_GetTimeDiffence(diff, &Date, true, 1);
	Date.Year += 20;
	entry->Entries[0].EntryType = CAL_START_DATETIME;

	switch (msg.Buffer[21]) {
	case 0x01:
	case 0x08:
		if (msg.Buffer[21] == 0x01) {
			dprintf("Meeting\n");
			entry->Type = GCN_MEETING;
		} else {
			dprintf("Reminder\n");
			Data->Cal->Type = GCN_REMINDER;
		}

		memcpy(&entry->Entries[0].Date,&Date,sizeof(GSM_DateTime));
		entry->EntriesNum++;

		N71_65_GetCalendarAlarm(msg.Buffer+16, entry, 0, Data);
		N71_65_GetCalendarRecurrance(msg.Buffer+22, entry);

		memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+30, msg.Buffer[28]*2);
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[28]*2]   = 0;
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[28]*2+1] = 0;
		entry->Entries[entry->EntriesNum].EntryType		   = CAL_TEXT;
		break;
	case 0x02:
		dprintf("Call\n");
		entry->Type = GCN_CALL;

		memcpy(&entry->Entries[0].Date,&Date,sizeof(GSM_DateTime));
		entry->EntriesNum++;

		N71_65_GetCalendarAlarm(msg.Buffer+16, entry, 0, Data);
		N71_65_GetCalendarRecurrance(msg.Buffer+22, entry);

		i = msg.Buffer[28] * 2;
		if (i!=0) {
			memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+30, i);
			entry->Entries[entry->EntriesNum].Text[i]   	= 0;
			entry->Entries[entry->EntriesNum].Text[i+1] 	= 0;
			entry->Entries[entry->EntriesNum].EntryType	= CAL_PHONE;
			dprintf("Phone        : \"%s\"\n", DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
			entry->EntriesNum++;
		}

		memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+30+i, msg.Buffer[29]*2);
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[29]*2]   = 0;
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[29]*2+1] = 0;
		entry->Entries[entry->EntriesNum].EntryType		   = CAL_TEXT;
		break;
	case 0x04:
		dprintf("Birthday\n");
		Data->Cal->Type = GCN_BIRTHDAY;

		/* Year was set earlier */
		entry->Entries[0].Date.Month	= Date.Month;
		entry->Entries[0].Date.Day	= Date.Day;
		entry->Entries[0].Date.Hour	= 23;
		entry->Entries[0].Date.Minute	= 59;
		entry->Entries[0].Date.Second	= 58;
		entry->EntriesNum++;

		N71_65_GetCalendarAlarm(msg.Buffer+16, entry, 0, Data);
		N71_65_GetCalendarRecurrance(msg.Buffer+22, entry);

		/* Birthday year */
		entry->Entries[0].Date.Year = msg.Buffer[28]*256 + msg.Buffer[29];
		if (msg.Buffer[28] == 0xff && msg.Buffer[29] == 0xff) entry->Entries[0].Date.Year = 0;
		dprintf("Birthday date: %02i-%02i-%04i\n",
			entry->Entries[0].Date.Day,entry->Entries[0].Date.Month,
			entry->Entries[0].Date.Year);

		memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+32, msg.Buffer[31]*2);
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[31]*2]   = 0;
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[31]*2+1] = 0;
		entry->Entries[entry->EntriesNum].EntryType		   = CAL_TEXT;
		break;
	default:
		dprintf("ERROR: unknown %i\n",msg.Buffer[6]);
		return GE_UNKNOWNRESPONSE;
	}
	dprintf("Text         : \"%s\"\n", DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
	entry->EntriesNum++;
	return GE_NONE;
}

/* method 2 */
GSM_Error N71_65_GetNextCalendar2(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start, int *LastCalendarYear, int *LastCalendarPos)
{
	GSM_Error		error;
	GSM_DateTime		date_time;
	unsigned char 		req[] = {
		N6110_FRAME_HEADER, 0x3e, 
		0xFF, 0xFE};		/* Location */

	if (start) {
		/* We have to get current year. It's NOT written in frame for
		 * Birthday
		 */
		error=s->Phone.Functions->GetDateTime(s,&date_time);
		switch (error) {
			case GE_EMPTY:
			case GE_NOTIMPLEMENTED:
				GSM_GetCurrentDateTime(&date_time);
				break;
			case GE_NONE:
				break;
			default:
				return error;
		}
		*LastCalendarYear = date_time.Year;

		/* First location at all */
		req[4] = 0xFF;
		req[5] = 0xFE;
	} else {
		req[4] = *LastCalendarPos >> 8;
		req[5] = *LastCalendarPos & 0xff;
	}
	Note->EntriesNum		= 0;
	Note->Entries[0].Date.Year 	= *LastCalendarYear;

	s->Phone.Data.Cal = Note;
	dprintf("Getting calendar note\n");
	error=GSM_WaitFor (s, req, 6, 0x13, 4, ID_GetCalendarNote);
	*LastCalendarPos = Note->Location;
	return error;
}

/* method 2 */
GSM_Error N71_65_ReplyAddCalendar2(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Calendar note added\n");
	return GE_NONE;
}

/* method 2 */
GSM_Error N71_65_AddCalendar2(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	time_t     		t_time1,t_time2;
	GSM_DateTime		Date,date_time;
 	GSM_Error		error;
	long			diff;
 	int 			Text, Time, Alarm, Phone, Recurrance, length=25;
	unsigned char 		req[200] = {
		N6110_FRAME_HEADER,
		0x40,
		0x00,				/* frame length - 7 */
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,		/* start time saved as difference */
		0x00,0x00,0xff,0xff,		/* alarm saved as difference */
		0x00,				/* frame length - 7 */
		0x00,				/* note type */
		0x00,0x00,			/* recurrance */
		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00};		/* rest depends on note type */

	switch(Note->Type) {
		case GCN_REMINDER: req[18] = 0x08; length = 25; break;
		case GCN_BIRTHDAY: req[18] = 0x04; length = 28; break;
		case GCN_CALL    : req[18] = 0x02; length = 27; break;
		case GCN_MEETING :
		default		 : req[18] = 0x01; length = 25; break;
	}

	GSM_CalendarFindDefaultTextTimeAlarmPhoneRecurrance(*Note, &Text, &Time, &Alarm, &Phone, &Recurrance);

	if (Time == -1) return GE_UNKNOWN;
	if (Note->Type != GCN_BIRTHDAY) {
		Date.Year 	= 2030;	Date.Month 	= 01; Date.Day    = 01;
		Date.Hour 	= 00;	Date.Minute 	= 00; Date.Second = 00;
	} else {
		Date.Year 	= 2029; Date.Month 	= 12; Date.Day 	  = 31;
		Date.Hour 	= 22;   Date.Minute 	= 59; Date.Second = 58;
	}
	t_time1 = Fill_Time_T(Date);
	memcpy(&Date,&Note->Entries[Time].Date,sizeof(GSM_DateTime));
	if (Note->Type != GCN_BIRTHDAY) {
		Date.Year -= 20;
	} else {
		Date.Year = 1980;
		Date.Hour = 22; Date.Minute = 58; Date.Second = 58;
	}
	t_time2 = Fill_Time_T(Date);
	diff	= t_time1-t_time2;
	dprintf("  Difference : %i seconds\n", -diff);
	req[9]  = (unsigned char)(-diff >> 24);
	req[10] = (unsigned char)(-diff >> 16);
	req[11] = (unsigned char)(-diff >> 8);
	req[12] = (unsigned char)(-diff);
	if (Note->Type == GCN_BIRTHDAY) {
		req[25] = Note->Entries[Time].Date.Year / 256;
		req[26] = Note->Entries[Time].Date.Year % 256;
		/* Recurrance = 1 year */
		req[19] = 0xff;
		req[20] = 0xff;
	}

	if (Note->Type == GCN_CALL && Phone != -1) {
		req[25] = strlen(DecodeUnicodeString(Note->Entries[Phone].Text));
		CopyUnicodeString(req+length,Note->Entries[Phone].Text);
		length += strlen(DecodeUnicodeString(Note->Entries[Phone].Text))*2;
	}
 
	if (Alarm != -1) {
		if (Note->Type == GCN_BIRTHDAY) {
			if (Note->Entries[Alarm].EntryType == CAL_SILENT_ALARM_DATETIME) req[27] = 0x01;
			error=s->Phone.Functions->GetDateTime(s,&date_time);
			switch (error) {
				case GE_EMPTY:
				case GE_NOTIMPLEMENTED:
					GSM_GetCurrentDateTime(&date_time);
					break;
				case GE_NONE:
					break;
				default:
					return error;
			}
			Date.Year	= date_time.Year;
			Date.Hour   	= 23;
			Date.Minute 	= 59;
		} else {
			Date.Year += 20;
		}
		t_time2   = Fill_Time_T(Date);
		t_time1   = Fill_Time_T(Note->Entries[Alarm].Date);
		diff	  = t_time1-t_time2;

		/* Sometimes we have difference in minutes */
		if (Note->Type == GCN_MEETING) diff = diff / 60;
		if (!IsPhoneFeatureAvailable(s->Phone.Data.Model, F_CAL35)) {
			if (Note->Type == GCN_REMINDER || Note->Type == GCN_CALL) {
				diff = diff / 60;
			}
		}

		dprintf("  Difference : %i seconds or minutes\n", -diff);
		req[13] = (unsigned char)(-diff >> 24);
		req[14] = (unsigned char)(-diff >> 16);
		req[15] = (unsigned char)(-diff >> 8);
		req[16] = (unsigned char)(-diff);
	}

	if (Recurrance != -1) {
		/* 0xffff -> 1 Year (8760 hours) */
		if (Note->Entries[Recurrance].Number >= 8760) {
			req[19] = 0xff;
			req[20] = 0xff;
		} else {
			req[19] = Note->Entries[Recurrance].Number / 256;
			req[20] = Note->Entries[Recurrance].Number % 256;
		}
	}

	if (Text != -1) {
		switch (Note->Type) {
		case GCN_CALL:
			req[26] = strlen(DecodeUnicodeString(Note->Entries[Text].Text));
			break;
		default:
			req[length++] = strlen(DecodeUnicodeString(Note->Entries[Text].Text));
			if (Note->Type == GCN_REMINDER || Note->Type == GCN_MEETING) req[length++] = 0x00;
		}
		CopyUnicodeString(req+length,Note->Entries[Text].Text);
		length += strlen(DecodeUnicodeString(Note->Entries[Text].Text))*2;
	}

	req[length++] = 0x00;
	req[length++] = 0x00;

	req[4] = req[17] = length-7;

	dprintf("Writing calendar note\n");
	return GSM_WaitFor (s, req, length, 0x13, 4, ID_SetCalendarNote);
}

GSM_Error N71_65_ReplyDelCalendar(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Deleted calendar note on location %d\n",msg.Buffer[4]*256+msg.Buffer[5]);
	return GE_NONE;
}

GSM_Error N71_65_DelCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x0b,
		0x00, 0x00};			/* location */

	req[4] = Note->Location >> 8;
	req[5] = Note->Location & 0xff;

	dprintf("Deleting calendar note\n");
	return GSM_WaitFor (s, req, 6, 0x13, 4, ID_DeleteCalendarNote);
}

/* method 1 */
GSM_Error N71_65_ReplyGetCalendarInfo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User, GSM_NOKIACalendarLocations *LastCalendar)
{
	int i;

	dprintf("Info with calendar notes locations received\n");
	LastCalendar->Number=msg.Buffer[4]*256+msg.Buffer[5];
	dprintf("Number of Entries: %i\n",LastCalendar->Number);
	dprintf("Locations: ");
	for (i=0;i<LastCalendar->Number;i++) {
		/* Some phones seems to return incorrect number
		 * of calendar entries. Please don't ask why.
		 * Here is workaround */
		if (((8+(i*2))>=msg.Length) || ((9+(i*2))>=msg.Length)) {
			LastCalendar->Number = i+1;
			dprintf("\nCorrect number of Entries: %i",LastCalendar->Number);
			break;
		}
		LastCalendar->Location[i]=msg.Buffer[8+(i*2)]*256+msg.Buffer[9+(i*2)];
		dprintf("%i ",LastCalendar->Location[i]);
	}
	dprintf("\n");
	return GE_NONE;
}

/* method 1 */
GSM_Error N71_65_GetCalendarInfo(GSM_StateMachine *s)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x3a, 0xFF, 0xFE};
	dprintf("Getting locations for calendar\n");
	return GSM_WaitFor (s, req, 6, 0x13, 4, ID_GetCalendarNotesInfo);
}

/* method 1 */
GSM_Error N71_65_ReplyGetNextCalendar1(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int 			alarm,i;
	GSM_CalendarEntry	*entry = Data->Cal;

	dprintf("Calendar note received\n");

	/* Later these values can change */
	if (msg.Buffer[6]!=0x04) { /* Here not birthday */
		entry->Entries[0].Date.Year = msg.Buffer[8]*256+msg.Buffer[9];
	}
	entry->Entries[0].Date.Month 	= msg.Buffer[10];
	entry->Entries[0].Date.Day	= msg.Buffer[11];
	entry->Entries[0].Date.Hour	= msg.Buffer[12];
	entry->Entries[0].Date.Minute	= msg.Buffer[13];
	entry->Entries[0].Date.Second 	= 0;
	entry->Entries[0].EntryType 	= CAL_START_DATETIME;
	entry->EntriesNum++;

	switch (msg.Buffer[6]) {
	case 0x01:
		dprintf("Meeting\n");
		entry->Type = GCN_MEETING;

		alarm=msg.Buffer[14]*256+msg.Buffer[15];
		if (alarm != 0xffff) {
			dprintf("  Difference : %i seconds\n", alarm);
			memcpy(&entry->Entries[1].Date,&entry->Entries[0].Date,sizeof(GSM_DateTime));
			N71_65_GetTimeDiffence(alarm, &entry->Entries[1].Date, false, 60);
			entry->Entries[1].EntryType = CAL_ALARM_DATETIME;
			entry->EntriesNum++;
		}
		N71_65_GetCalendarRecurrance(msg.Buffer + 16, entry);

		memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+20, msg.Buffer[18]*2);
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[18]*2]   = 0;
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[18]*2+1] = 0;
		entry->Entries[entry->EntriesNum].EntryType		   = CAL_TEXT;
		dprintf("Text         : \"%s\"\n", DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
		entry->EntriesNum++;
		return GE_NONE;
	case 0x02:
		dprintf("Call\n");
		entry->Type = GCN_CALL;

		alarm=msg.Buffer[14]*256+msg.Buffer[15];
		if (alarm != 0xffff) {
			dprintf("  Difference : %i seconds\n", alarm);
			memcpy(&entry->Entries[1].Date,&entry->Entries[0].Date,sizeof(GSM_DateTime));
			N71_65_GetTimeDiffence(alarm, &entry->Entries[1].Date, false, 60);
			entry->Entries[1].EntryType = CAL_ALARM_DATETIME;
			entry->EntriesNum++;
		}
		N71_65_GetCalendarRecurrance(msg.Buffer + 16, entry);

		i = msg.Buffer[18] * 2;
		if (i!=0) {
			memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+20, i);
			entry->Entries[entry->EntriesNum].Text[i]   	= 0;
			entry->Entries[entry->EntriesNum].Text[i+1] 	= 0;
			entry->Entries[entry->EntriesNum].EntryType	= CAL_TEXT;
			dprintf("Text         : \"%s\"\n", DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
			entry->EntriesNum++;
		}

		memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+20+i, msg.Buffer[19]*2);
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[19]*2]   = 0;
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[19]*2+1] = 0;
		entry->Entries[entry->EntriesNum].EntryType		   = CAL_PHONE;
		dprintf("Phone        : \"%s\"\n", DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
		entry->EntriesNum++;
		return GE_NONE;
	case 0x04:
		dprintf("Birthday\n");
		entry->Type = GCN_BIRTHDAY;

		entry->Entries[0].Date.Hour	= 23;
		entry->Entries[0].Date.Minute	= 59;
		entry->Entries[0].Date.Second	= 58;

		alarm  = ((unsigned int)msg.Buffer[14]) << 24;
		alarm += ((unsigned int)msg.Buffer[15]) << 16;
		alarm += ((unsigned int)msg.Buffer[16]) << 8;
		alarm += msg.Buffer[17];
		if (alarm != 0xffff) {
			dprintf("  Difference : %i seconds\n", alarm);
			memcpy(&entry->Entries[1].Date,&entry->Entries[0].Date,sizeof(GSM_DateTime));
			N71_65_GetTimeDiffence(alarm, &entry->Entries[1].Date, false, 1);
			entry->Entries[1].EntryType = CAL_ALARM_DATETIME;
			if (msg.Buffer[20]!=0x00) {
				entry->Entries[1].EntryType = CAL_SILENT_ALARM_DATETIME;
				dprintf("Alarm type   : Silent\n");
			}
			entry->EntriesNum++;
		}

		entry->Entries[0].Date.Year = msg.Buffer[18]*256 + msg.Buffer[19];

		memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+22, msg.Buffer[21]*2);
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[21]*2]   = 0;
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[21]*2+1] = 0;
		entry->Entries[entry->EntriesNum].EntryType		   = CAL_TEXT;
		dprintf("Text         : \"%s\"\n", DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
		entry->EntriesNum++;
		return GE_NONE;
	case 0x08:
		dprintf("Reminder\n");
		entry->Type = GCN_REMINDER;

		entry->Entries[0].Date.Hour	= 0;
		entry->Entries[0].Date.Minute	= 0;

		N71_65_GetCalendarRecurrance(msg.Buffer + 12, entry);

		memcpy(entry->Entries[entry->EntriesNum].Text, msg.Buffer+16, msg.Buffer[14]*2);
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[14]*2]   = 0;
		entry->Entries[entry->EntriesNum].Text[msg.Buffer[14]*2+1] = 0;
		entry->Entries[entry->EntriesNum].EntryType		   = CAL_TEXT;
		dprintf("Text         : \"%s\"\n", DecodeUnicodeString(entry->Entries[entry->EntriesNum].Text));
		entry->EntriesNum++;
		return GE_NONE;
	default:
		dprintf("ERROR: unknown %i\n",msg.Buffer[6]);
		return GE_UNKNOWNRESPONSE;
	}
}

/* method 1 */
GSM_Error N71_65_GetNextCalendar1(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start, GSM_NOKIACalendarLocations *LastCalendar, int *LastCalendarYear, int *LastCalendarPos)
{
	GSM_Error		error;
	GSM_DateTime		date_time;
	unsigned char 		req[] = {
		N6110_FRAME_HEADER, 0x19, 
		0x00, 0x00};		/* Location */

	if (start) {
		error=N71_65_GetCalendarInfo(s);
		if (error!=GE_NONE) return error;

		/* We have to get current year. It's NOT written in frame for
		 * Birthday
		 */
		error=s->Phone.Functions->GetDateTime(s,&date_time);
		switch (error) {
			case GE_EMPTY:
			case GE_NOTIMPLEMENTED:
				GSM_GetCurrentDateTime(&date_time);
				break;
			case GE_NONE:
				break;
			default:
				return error;
		}
		*LastCalendarYear 	= date_time.Year;
		*LastCalendarPos 	= 0;
	} else {
		(*LastCalendarPos)++;
	}

	if (*LastCalendarPos >= LastCalendar->Number) return GE_EMPTY;

	req[4] = LastCalendar->Location[*LastCalendarPos] >> 8;
	req[5] = LastCalendar->Location[*LastCalendarPos] & 0xff;

	Note->EntriesNum		= 0;
	Note->Entries[0].Date.Year 	= *LastCalendarYear;
	Note->Location			= LastCalendar->Location[*LastCalendarPos];

	s->Phone.Data.Cal=Note;
	dprintf("Getting calendar note\n");
	return GSM_WaitFor (s, req, 6, 0x13, 4, ID_GetCalendarNote);
}

/* method 1*/
GSM_Error N71_65_ReplyGetCalendarNotePos(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User,int *FirstCalendarPos)
{
	dprintf("First calendar location: %i\n",msg.Buffer[4]*256+msg.Buffer[5]);
	*FirstCalendarPos = msg.Buffer[4]*256+msg.Buffer[5];
	return GE_NONE;
}

/* method 1*/
static GSM_Error N71_65_GetCalendarNotePos(GSM_StateMachine *s)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x31};

	dprintf("Getting first free calendar note location\n");
	return GSM_WaitFor (s, req, 4, 0x13, 4, ID_GetCalendarNotePos);
}

/* method 1 */
GSM_Error N71_65_ReplyAddCalendar1(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
#ifdef DEBUG
	dprintf("Written Calendar Note type ");
	switch ((msg.Buffer[3]/2)-1) {
		case 0:	dprintf("Meeting");	break;
		case 1:	dprintf("Call");	break;
		case 2:	dprintf("Birthday");	break;
		case 3:	dprintf("Reminder");	break;
	}
	dprintf(" on location %d\n",msg.Buffer[4]*256+msg.Buffer[5]);
#endif
	return GE_NONE;
}

/* method 1 */
GSM_Error N71_65_AddCalendar1(GSM_StateMachine *s, GSM_CalendarEntry *Note, int *FirstCalendarPos)
{
	long			seconds;
 	GSM_Error		error;
	GSM_DateTime		DT;
 	int 			Text, Time, Alarm, Phone, Recurrance, count=12;
	unsigned char 		req[200] = {
		N6110_FRAME_HEADER,
		0x01,				/* note type */
		0x00, 0x00,			/* location ? */
		0x00,				/* entry type */
		0x00,
		0x00, 0x00, 0x00, 0x00,		/* Year(2bytes), Month, Day */
		/* here starts block ... depends on note type */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00};                          

	error=N71_65_GetCalendarNotePos(s);
	if (error!=GE_NONE) return error;
	if (FirstCalendarPos != NULL) {
		req[4] = *FirstCalendarPos/256;
		req[5] = *FirstCalendarPos%256;
	}

	switch(Note->Type) {
		case GCN_CALL    : req[3]=0x03; req[6]=0x02; break;
		case GCN_BIRTHDAY: req[3]=0x05; req[6]=0x04; break;
		case GCN_REMINDER: req[3]=0x07; req[6]=0x08; break;
		case GCN_MEETING :
		default		 : req[3]=0x01; req[6]=0x01; break;
	}

	GSM_CalendarFindDefaultTextTimeAlarmPhoneRecurrance(*Note, &Text, &Time, &Alarm, &Phone, &Recurrance);

	if (Time == -1) return GE_UNKNOWN;
	memcpy(&DT,&Note->Entries[Time].Date,sizeof(GSM_DateTime));
	req[8]	= DT.Year >> 8;
	req[9]	= DT.Year & 0xff;
	req[10]	= DT.Month;
	req[11]	= DT.Day;

	switch(Note->Type) {
	case GCN_BIRTHDAY:
		req[count++] = 0x00;		/* 12 */
		req[count++] = 0x00;		/* 13 */

		/* Alarm */
		req[count++] = 0x00;		/* 14 */
		req[count++] = 0x00;		/* 15 */
		req[count++] = 0xff;		/* 16 */
		req[count++] = 0xff;		/* 17 */
		if (Alarm != -1) {
			/* I try with Time.Year = Alarm.Year. If negative, I increase 1 year,
			 * but only once ! This thing, because I may have Alarm period across
			 * a year. (eg. Birthday on 2001-01-10 and Alarm on 2000-12-27)
			 */
			DT.Year = Note->Entries[Alarm].Date.Year;
			if((seconds = Fill_Time_T(DT)-Fill_Time_T(Note->Entries[Alarm].Date))<0L)
			{
				DT.Year++;
				seconds = Fill_Time_T(DT)-Fill_Time_T(Note->Entries[Alarm].Date);
			}
			if(seconds>=0L)
			{
				count -= 4;
				req[count++] = ((unsigned char)seconds>>24); 	/* 14 */
				req[count++] = (seconds>>16) & 0xff;		/* 15 */
				req[count++] = (seconds>>8) & 0xff;		/* 16 */
				req[count++] = seconds & 0xff;			/* 17 */
			}
			if (Note->Entries[Alarm].EntryType == CAL_SILENT_ALARM_DATETIME) req[count++] = 0x01; else req[count++] = 0x00;
		}

		if (Text != -1) {
			req[count++] = strlen(DecodeUnicodeString(Note->Entries[Text].Text));	/* 19 */
			CopyUnicodeString(req+count,Note->Entries[Text].Text);
			count=count+2*strlen(DecodeUnicodeString(Note->Entries[Text].Text));	/* 22->N */
		} else {
			req[count++] = 0x00;
		}
		break;
	case GCN_REMINDER:
		if (Recurrance != -1) {
			/* 0xffff -> 1 Year (8760 hours) */
			if (Note->Entries[Recurrance].Number >= 8760) {
				req[count++] = 0xff;
				req[count++] = 0xff;
			} else {
				req[count++] = Note->Entries[Recurrance].Number >> 8;   /* 12 */
				req[count++] = Note->Entries[Recurrance].Number & 0xff; /* 13 */
			}
		} else {
			req[count++] = 0x00; /* 12 */
			req[count++] = 0x00; /* 13 */
		}

		if (Text != -1) {
			req[count++] = strlen(DecodeUnicodeString(Note->Entries[Text].Text));	/* 14 */
			req[count++] = 0x00;							/* 15 */
			CopyUnicodeString(req+count,Note->Entries[Text].Text);
			count=count+2*strlen(DecodeUnicodeString(Note->Entries[Text].Text));	/* 16->N */
		} else {
			req[count++] = 0x00;
			req[count++] = 0x00;
		}
		break;
	case GCN_MEETING:
	case GCN_CALL:
	default:
		req[count++] = DT.Hour;   /* 12 */
		req[count++] = DT.Minute; /* 13 */

		/* Alarm */
		req[count++] = 0xff;	  /* 14 */
		req[count++] = 0xff;	  /* 15 */
		if (Alarm != -1) {
			seconds=Fill_Time_T(DT)-Fill_Time_T(Note->Entries[Alarm].Date);
			if(seconds>=0L)
			{
				count -= 2;
				req[count++] = ((unsigned char)(seconds/60L)>>8);
				req[count++] = (seconds/60L)&0xff;
			}
		}

		if (Recurrance != -1) {
			/* 0xffff -> 1 Year (8760 hours) */
			if (Note->Entries[Recurrance].Number >= 8760) {
				req[count++] = 0xff;
				req[count++] = 0xff;
			} else {
				req[count++] = Note->Entries[Recurrance].Number >> 8;   /* 12 */
				req[count++] = Note->Entries[Recurrance].Number & 0xff; /* 13 */
			}
		} else {
			req[count++] = 0x00; /* 16 */
			req[count++] = 0x00; /* 17 */
		}

		if (Text != -1) {
			req[count++] = strlen(DecodeUnicodeString(Note->Entries[Text].Text));
		} else {
			req[count++] = 0x00; /* 18 */
		}
		if (Note->Type == GCN_CALL && Phone != -1) {
			req[count++] = strlen(DecodeUnicodeString(Note->Entries[Phone].Text));
		} else {
			req[count++] = 0x00; /* 19 */
		}
		if (Text != -1) {
			CopyUnicodeString(req+count,Note->Entries[Text].Text);
			count=count+2*strlen(DecodeUnicodeString(Note->Entries[Text].Text));/* 20->N */
		}
		if (Note->Type == GCN_CALL && Phone != -1) {
			CopyUnicodeString(req+count,Note->Entries[Phone].Text);
			count=count+2*strlen(DecodeUnicodeString(Note->Entries[Phone].Text));/* (N+1)->n */
		}
		break;
	}
	req[count] = 0x00;
	dprintf("Writing calendar note\n");
	return GSM_WaitFor (s, req, count, 0x13, 4, ID_SetCalendarNote);
}

#endif
