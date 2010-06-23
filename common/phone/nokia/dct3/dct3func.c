
#include <string.h> /* memcpy only */
#include <stdio.h>
#include <ctype.h>

#include "../../../gsmstate.h"
#include "../../../misc/coding.h"
#include "../../../service/gsmsms.h"
#include "../../pfunc.h"
#include "../nfunc.h"
#include "dct3func.h"

#if defined(GSM_ENABLE_NOKIA_DCT3)

GSM_Error DCT3_ReplyPressKey(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[2]) {
	case 0x46:
		dprintf("Pressing key OK\n");
		if (Data->PressKey) return GE_NONE;
		break;
	case 0x47:
		dprintf("Releasing key OK\n");
		if (!Data->PressKey) return GE_NONE;
		break;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error DCT3_PressKey(GSM_StateMachine *s, GSM_KeyCode Key, bool Press)
{
	unsigned char PressReq[]   = {
				0x00, 0x01, 0x46, 0x00, 0x01,
				0x0a};		/* Key code */
	unsigned char ReleaseReq[] = {0x00, 0x01, 0x47, 0x00, 0x01, 0x0c};

	if (Press) {
		PressReq[5] = Key;
		s->Phone.Data.PressKey = true;
		dprintf("Pressing key\n");
		return GSM_WaitFor (s, PressReq, 6, 0xd1, 4, ID_PressKey);
	} else {
		s->Phone.Data.PressKey = false;
		dprintf("Releasing key\n");
		return GSM_WaitFor (s, ReleaseReq, 6, 0xd1, 4, ID_PressKey);
	}
}

GSM_Error DCT3_ReplyPlayTone(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Tone played\n");
	return GE_NONE;
}

GSM_Error DCT3_PlayTone(GSM_StateMachine *s, int Herz, unsigned char Volume, bool start)
{
	GSM_Error error;
	unsigned char req[] = {
		0x00,0x01,0x8f,
		0x00,	/* Volume */
		0x00,	/* HerzLo */
		0x00};	/* HerzHi */

	if (start) {
		error=DCT3_EnableSecurity (s, 0x01);
		if (error!=GE_NONE) return error;
	}

	/* For Herz==255*255 we have silent */  
	if (Herz!=255*255) {
		req[3]=Volume;
		req[5]=Herz%256;
		req[4]=Herz/256;
	} else {
		req[3]=0;
		req[5]=0;
		req[4]=0;
	}

	return GSM_WaitFor (s, req, 6, 0x40, 4, ID_PlayTone);
}

#ifdef GSM_ENABLE_CELLBROADCAST

GSM_Error DCT3_ReplyIncomingCB(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	GSM_CBMessage 	CB;
	int		i;
	char		Buffer[300];

	dprintf("CB received\n");
	CB.Channel = msg.Buffer[7];
	i=GSM_UnpackEightBitsToSeven(0, msg.Buffer[9], msg.Buffer[9], msg.Buffer+10, Buffer);
	i = msg.Buffer[9] - 1;
	while (i!=0) {
		if (Buffer[i] == 13) i = i - 1; else break;
	}
	DecodeDefault(CB.Text, Buffer, i + 1);
	dprintf("Channel %i, text \"%s\"\n",CB.Channel,DecodeUnicodeString(CB.Text));
	if (Data->EnableIncomingCB && User->IncomingCB!=NULL) {
		User->IncomingCB(Data->Device,CB);
	}
	return GE_NONE;
}

GSM_Error DCT3_ReplySetIncomingCB(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("CB set\n");
	return GE_NONE;
}

#endif

GSM_Error DCT3_SetIncomingCB(GSM_StateMachine *s, bool enable)
{
#ifdef GSM_ENABLE_CELLBROADCAST
	unsigned char reqOn[] = {
		N6110_FRAME_HEADER, 0x20, 0x01,
		0x01, 0x00, 0x00, 0x01, 0x01};
	unsigned char reqOff[] = {
		N6110_FRAME_HEADER, 0x20, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00};

	if (s->Phone.Data.EnableIncomingCB!=enable) {
		s->Phone.Data.EnableIncomingCB 	= enable;
		if (enable) {
			dprintf("Enabling incoming CB\n");
			return GSM_WaitFor(s, reqOn, 10, 0x02, 4, ID_SetIncomingCB);
		} else {
			dprintf("Disabling incoming CB\n");
			return GSM_WaitFor(s, reqOff, 10, 0x02, 4, ID_SetIncomingCB);
		}
	}
#endif
	return GE_NONE;
}

GSM_Error DCT3_ReplySetSMSC(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("SMSC set\n");
	return GE_NONE;
}

GSM_Error DCT3_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	unsigned char req[64] = {
		N6110_FRAME_HEADER, 0x30, 0x64,
		0x00,				/* Location			*/
		0x00,
		0x00,				/* SMS Message Format		*/
		0x00,
		0x00,				/* Validity			*/
		0,0,0,0,0,0,0,0,0,0,0,0, 	/* Default recipient number	*/
		0,0,0,0,0,0,0,0,0,0,0,0}; 	/* Message Center Number	*/

	req[5]  = smsc->Location;
	switch (smsc->Format) {
		case GSMF_Text:		req[7] = 0x00;	break;
		case GSMF_Fax:		req[7] = 0x22;	break;
		case GSMF_Pager:	req[7] = 0x26;	break;
		case GSMF_Email:	req[7] = 0x32;	break;
	}
	req[9]  = smsc->Validity.Relative;
	req[10] = GSM_PackSemiOctetNumber(smsc->DefaultNumber, req+11, true);
	req[22] = GSM_PackSemiOctetNumber(smsc->Number, req+23, false);
	memcpy (req + 34, DecodeUnicodeString(smsc->Name),strlen(DecodeUnicodeString(smsc->Name)));
	dprintf("Setting SMSC\n");
	return GSM_WaitFor (s, req, 35+strlen(DecodeUnicodeString(smsc->Name)), 0x02, 4, ID_SetSMSC);
}

GSM_Error DCT3_ReplyEnableSecurity(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("State of security commands set\n");
	return GE_NONE;
}

/* If you set make some things (for example, change Security Code from
 * phone's menu, disable and enable phone), it won't answer for 0x40 frames
 * and you won't be able to play tones, get netmonitor, etc.
 * This function do thing called "Enabling extended security commands"
 * and it enables 0x40 frame functions.
 * This frame can also some other things - see below
 */
GSM_Error DCT3_EnableSecurity (GSM_StateMachine *s, unsigned char status)
{
	unsigned char req[4] =
		{0x00, 0x01, 0x64,
		 0x01};			/* 0x01 - on, 0x00 - off,
					 * 0x03 & 0x04 - soft & hard reset,
					 * 0x06 - CONTACT SERVICE */

	/* 0x06 MAKES CONTACT SERVICE! BE CAREFULL! */
	/* When use 0x03 and had during session changed time & date
	 * some phones (like 6150 or 6210) can ask for time & date after reset
	 * or disable clock on the screen
	 */
	if (status!=0x06) req[3] = status;
	dprintf("Setting state of security commands\n");
	return GSM_WaitFor (s, req, 4, 0x40, 4, ID_EnableSecurity);
}

GSM_Error DCT3_ReplyGetIMEI(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	memcpy(Data->IMEI,msg.Buffer + 4, 16);
	dprintf("Received IMEI %s\n",Data->IMEI);
	return GE_NONE;
}

GSM_Error DCT3_GetIMEI (GSM_StateMachine *s, unsigned char *imei)
{
	unsigned char 	req[4] = {0x00, 0x01, 0x66, 0x00};  
	GSM_Error 	error;

	if (strlen(s->Phone.Data.IMEICache)!=0) {
		strcpy(imei,s->Phone.Data.IMEICache);
		return GE_NONE;
	}

	error=DCT3_EnableSecurity (s, 0x01);
	if (error!=GE_NONE) return error;

	s->Phone.Data.IMEI=imei;
	dprintf("Getting IMEI\n");
	return GSM_WaitFor (s, req, 4, 0x40, 2, ID_GetIMEI);
}

GSM_Error DCT3_ReplySIMLogin(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("SIM card login\n");
	return GE_NONE;
}

GSM_Error DCT3_ReplySIMLogout(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("SIM card logout\n");
	return GE_NONE;
}

GSM_Error DCT3_ReplyGetDateTime(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Date & time received\n");
	if (msg.Buffer[4]==0x01) {
		NOKIA_DecodeDateTime(msg.Buffer+8, Data->DateTime);
		return GE_NONE;
	}
	dprintf("Not set in phone\n");
	return GE_EMPTY;
}

GSM_Error DCT3_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time, unsigned char msgtype)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x62};

	s->Phone.Data.DateTime=date_time;
	dprintf("Getting date & time\n");
	return GSM_WaitFor (s, req, 4, msgtype, 4, ID_GetDateTime);
}

GSM_Error DCT3_ReplyGetAlarm(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Alarm received\n");
	if (msg.Buffer[8]==0x02) {
		dprintf("   Alarm: %02d:%02d\n", msg.Buffer[9], msg.Buffer[10]);
		Data->Alarm->Hour	= msg.Buffer[9];
		Data->Alarm->Minute	= msg.Buffer[10];
		Data->Alarm->Second	= 0;
		return GE_NONE;
	}
	dprintf("   Not set in phone\n");
	return GE_EMPTY;
}

GSM_Error DCT3_GetAlarm(GSM_StateMachine *s, GSM_DateTime *alarm, int alarm_number, unsigned char msgtype)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x6d};

	if (alarm_number!=1) return GE_NOTSUPPORTED;

	s->Phone.Data.Alarm=alarm;
	dprintf("Getting alarm\n");
	return GSM_WaitFor (s, req, 4, msgtype, 4, ID_GetAlarm);
}

GSM_Error DCT3_ReplySetDateTime(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Date & time set\n");
	if (msg.Buffer[4]==0x01) {
		dprintf("Set OK\n");
		return GE_NONE;
	}
	dprintf("Error\n");
	return GE_UNKNOWN;
}

GSM_Error DCT3_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time, unsigned char msgtype)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x60, 0x01, 0x01, 0x07,
		0x00, 0x00,	/* Year (0x07cf = 1999) */
		0x00, 0x00,	/* Month & Day */
		0x00, 0x00,	/* Hours & Minutes */
		0x00 };		/* Unknown, but not seconds - try 59 and wait 1 sec. */

	NOKIA_EncodeDateTime(req+7, date_time);
	dprintf("Setting date & time\n");
	return GSM_WaitFor (s, req, 14, msgtype, 4, ID_SetDateTime);
}

GSM_Error DCT3_ReplySetAlarm(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Alarm set\n");
	if (msg.Buffer[4]==0x01) {
		dprintf("Set OK\n");
		return GE_NONE;
	}
	dprintf("Error\n");
	return GE_UNKNOWN;
}

GSM_Error DCT3_SetAlarm(GSM_StateMachine *s, GSM_DateTime *alarm, int alarm_number, unsigned char msgtype)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x6b, 0x01, 0x20, 0x03,
		0x02,      	/* should be alarm on/off, but it doesn't work */
		0x00, 0x00,	/* Hours Minutes */
		0x00 };		/* Unknown, but not seconds - try 59 and wait 1 sec. */

	if (alarm_number!=1) return GE_NOTSUPPORTED;

	req[8] = alarm->Hour;
	req[9] = alarm->Minute;

	dprintf("Setting alarm\n");
	return GSM_WaitFor (s, req, 11, msgtype, 4, ID_SetAlarm);
}

GSM_Error DCT3_ReplyGetSMSC(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int i;

	dprintf("SMSC received\n");
	switch (msg.Buffer[3]) {
	case 0x34:
		Data->SMSC->Format = GSMF_Text;
		switch (msg.Buffer[6]) {
			case 0x00: Data->SMSC->Format = GSMF_Text; 	break;
			case 0x22: Data->SMSC->Format = GSMF_Fax; 	break;
			case 0x26: Data->SMSC->Format = GSMF_Pager;	break;
			case 0x32: Data->SMSC->Format = GSMF_Email;	break;
		}
		Data->SMSC->Validity.VPF	= GSM_RelativeFormat;
		Data->SMSC->Validity.Relative	= msg.Buffer[8];
	
		i=33;
		while (msg.Buffer[i]!=0) {i++;}
		i=i-33;
		if (i>GSM_MAX_SMSC_NAME_LENGTH) {
			dprintf("Too long name\n");
			return GE_UNKNOWNRESPONSE;
		}
		EncodeUnicode(Data->SMSC->Name,msg.Buffer+33,i);
		dprintf("   Name \"%s\"\n", DecodeUnicodeString(Data->SMSC->Name));
	
		GSM_UnpackSemiOctetNumber(Data->SMSC->DefaultNumber,msg.Buffer+9,true);
		dprintf("   Default number \"%s\"\n", DecodeUnicodeString(Data->SMSC->DefaultNumber));
		GSM_UnpackSemiOctetNumber(Data->SMSC->Number,msg.Buffer+21,false);
		dprintf("   Number \"%s\"\n", DecodeUnicodeString(Data->SMSC->Number));
	
		return GE_NONE;
	case 0x35:
		dprintf("Getting SMSC failed\n");
		return GE_INVALIDLOCATION;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error DCT3_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	unsigned char req[] = {
		N6110_FRAME_HEADER, 0x33, 0x64,
		0x00};		/* SMS Center Number. */

	if (smsc->Location==0x00) return GE_INVALIDLOCATION;
	
	req[5]=smsc->Location;

	s->Phone.Data.SMSC=smsc;
	dprintf("Getting SMSC\n");
	return GSM_WaitFor (s, req, 6, 0x02, 4, ID_GetSMSC);
}

GSM_Error DCT3_ReplyGetNetworkInfo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int		count;
#ifdef DEBUG
	GSM_NetworkInfo NetInfo;
	char		name[100];

	dprintf("Network info received\n");
	dprintf("   Status                    : ");
	switch (msg.Buffer[8]) {
		case 0x01: dprintf("home network selected");		break;
		case 0x02: dprintf("roaming network");			break;
		case 0x03: dprintf("requesting network");		break;
		case 0x04: dprintf("not registered in the network");	break;
		default  : dprintf("unknown");
	}
	dprintf("\n");
	dprintf("   Network selection         : %s\n", msg.Buffer[9]==1?"manual":"automatic");
	if (msg.Buffer[8]<0x03) {
		NOKIA_DecodeNetworkCode(msg.Buffer+14,NetInfo.NetworkCode);
		sprintf(NetInfo.CellID, "%02x%02x", msg.Buffer[10], msg.Buffer[11]);
		sprintf(NetInfo.LAC,	"%02x%02x", msg.Buffer[12], msg.Buffer[13]);
		dprintf("   CellID                    : %s\n", NetInfo.CellID);
		dprintf("   LAC                       : %s\n", NetInfo.LAC);
		dprintf("   Network code              : %s\n", NetInfo.NetworkCode);
		dprintf("   Network name for Gammu    : %s ",
				DecodeUnicodeString(GSM_GetNetworkName(NetInfo.NetworkCode)));
		dprintf("(%s)\n",DecodeUnicodeString(GSM_GetCountryName(NetInfo.NetworkCode)));
		if (msg.Buffer[18]==0x00) {
			/* In 6210 name is in "normal" Unicode */
			memcpy(name,msg.Buffer+18,msg.Buffer[17]*2);
			name[msg.Buffer[17]*2]	=0x00;
			name[msg.Buffer[17]*2+1]=0x00;
			dprintf("   Network name for phone    : %s\n",DecodeUnicodeString(name));
		} else {
			/* In 9210 first 0x00 is cut from Unicode string */
			name[0] = 0;
			memcpy(name+1,msg.Buffer+18,msg.Buffer[17]*2);
			name[msg.Buffer[17]*2+1]=0x00;
			name[msg.Buffer[17]*2+2]=0x00;
			dprintf("   Network name for phone    : %s\n",DecodeUnicodeString(name));
		}
	}
#endif
	if (Data->RequestID==ID_GetNetworkInfo) {
		Data->NetworkInfo->NetworkName[0] = 0x00;
		Data->NetworkInfo->NetworkName[1] = 0x00;
		Data->NetworkInfo->State 	  = 0;
		switch (msg.Buffer[8]) {
			case 0x01: Data->NetworkInfo->State = GSM_HomeNetwork;		break;
			case 0x02: Data->NetworkInfo->State = GSM_RoamingNetwork;	break;
			case 0x03: Data->NetworkInfo->State = GSM_RequestingNetwork;	break;
			case 0x04: Data->NetworkInfo->State = GSM_NoNetwork;		break;
		}
		if (Data->NetworkInfo->State == GSM_HomeNetwork || Data->NetworkInfo->State == GSM_RoamingNetwork) {
			if (msg.Buffer[18]==0x00) {
				/* In 6210 name is in "normal" Unicode */
				memcpy(Data->NetworkInfo->NetworkName,msg.Buffer+18,msg.Buffer[17]*2);
				Data->NetworkInfo->NetworkName[msg.Buffer[17]*2]   = 0x00;
				Data->NetworkInfo->NetworkName[msg.Buffer[17]*2+1] = 0x00;
			} else {
				/* In 9210 first 0x00 is cut from Unicode string */
				Data->NetworkInfo->NetworkName[0] = 0;
				memcpy(Data->NetworkInfo->NetworkName+1,msg.Buffer+18,msg.Buffer[17]*2);
				Data->NetworkInfo->NetworkName[msg.Buffer[17]*2+1]=0x00;
				Data->NetworkInfo->NetworkName[msg.Buffer[17]*2+2]=0x00;
			}
			NOKIA_DecodeNetworkCode(msg.Buffer+14,Data->NetworkInfo->NetworkCode);
			sprintf(Data->NetworkInfo->CellID,	"%02x%02x", msg.Buffer[10], msg.Buffer[11]);
			sprintf(Data->NetworkInfo->LAC,		"%02x%02x", msg.Buffer[12], msg.Buffer[13]);
		}
	}
	/* 6210/6250/7110 */
	if (Data->RequestID==ID_GetBitmap) {
		if (msg.Buffer[4]==0x02) {
			dprintf("Operator logo available\n");
			count = 7;
			/* skip network info */
			count += msg.Buffer[count];
			count ++;
			Data->Bitmap->Width	= msg.Buffer[count++];
			Data->Bitmap->Height	= msg.Buffer[count++];
        		count+=4;
			PHONE_DecodeBitmap(GSM_Nokia7110OperatorLogo,msg.Buffer+count,Data->Bitmap);
			NOKIA_DecodeNetworkCode(msg.Buffer+14,Data->Bitmap->NetworkCode);
		} else {
			Data->Bitmap->Width	= 78;
			Data->Bitmap->Height	= 21;
			GSM_ClearBitmap(Data->Bitmap);
			strcpy(Data->Bitmap->NetworkCode,"000 00");
		}
	}
	return GE_NONE;
}

GSM_Error DCT3_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x70};

	s->Phone.Data.NetworkInfo=netinfo;
	dprintf("Getting network info\n");
	return GSM_WaitFor (s, req, 4, 0x0a, 4, ID_GetNetworkInfo);
}

GSM_Error DCT3_ReplyDialCommand(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Answer for call commands\n");
	return GE_NONE;
}

GSM_Error DCT3_DialVoice(GSM_StateMachine *s, char *number)
{
	unsigned int	i = 0;
	GSM_Error	error;
	unsigned char req[64] = {
		0x00, 0x01, 0x7c,
                0x01}; 			/* call command */

	error=DCT3_EnableSecurity (s, 0x01);
	if (error!=GE_NONE) return error;

	for (i=0; i < strlen(number) ; i++) req[4+i]=number[i];  
	req[4+i+1]=0;

	dprintf("Making voice call\n");
	return GSM_WaitFor (s, req, 4+strlen(number)+1, 0x40, 4, ID_DialVoice);
}

GSM_Error DCT3_CancelCall(GSM_StateMachine *s)
{
	GSM_Error	error;
	unsigned char req[64] = {
		0x00, 0x01, 0x7c,
                0x03}; 			/* call command */

	error=DCT3_EnableSecurity (s, 0x01);
	if (error!=GE_NONE) return error;

	dprintf("Canceling calls\n");
	return GSM_WaitFor (s, req, 4, 0x40, 4, ID_CancelCall);
}

GSM_Error DCT3_AnswerCall(GSM_StateMachine *s)
{
	GSM_Error	error;
	unsigned char 	req[64] = {
		0x00, 0x01, 0x7c,
                0x02}; 			/* call command */

	error=DCT3_EnableSecurity (s, 0x01);
	if (error!=GE_NONE) return error;

	dprintf("Answering calls\n");
	return GSM_WaitFor (s, req, 4, 0x40, 4, ID_AnswerCall);
}

GSM_Error DCT3_Reset(GSM_StateMachine *s, bool hard)
{
	GSM_Error error;

	if (hard) {
		error=DCT3_EnableSecurity(s, 0x04);
	} else {
		error=DCT3_EnableSecurity(s, 0x03);
	}
	if (error == GE_NONE) {
		s->Phone.Data.EnableIncomingSMS = false;
		s->Phone.Data.EnableIncomingCB  = false;
	}
	return error;
}

GSM_Error DCT3_ReplyGetWAPBookmark(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	return DCT3DCT4_ReplyGetWAPBookmark (msg,Data,User,false);
}

GSM_Error DCT3_SetWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
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

	req[count++] = strlen(DecodeUnicodeString(bookmark->Title));
	CopyUnicodeString(req+count,bookmark->Title);
	count = count + 2*strlen(DecodeUnicodeString(bookmark->Title));

	req[count++] = strlen(DecodeUnicodeString(bookmark->Address));
	CopyUnicodeString(req+count,bookmark->Address);
	count = count + 2*strlen(DecodeUnicodeString(bookmark->Address));

	/* ??? */
	req[count++] = 0x01; req[count++] = 0x80; req[count++] = 0x00;
	req[count++] = 0x00; req[count++] = 0x00; req[count++] = 0x00;
	req[count++] = 0x00; req[count++] = 0x00; req[count++] = 0x00;

	dprintf("Setting WAP bookmark\n");
	return GSM_WaitFor (s, req, count, 0x3f, 4, ID_SetWAPBookmark);
}

GSM_Error DCT3_ReplyGetWAPSettings(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int 			tmp,Number;
#ifdef GSM_ENABLE_NOKIA6110
	GSM_Phone_N6110Data 	*Priv6110 = &Data->Priv.N6110;
#endif
#ifdef GSM_ENABLE_NOKIA7110
	GSM_Phone_N7110Data 	*Priv7110 = &Data->Priv.N7110;
#endif

	switch(msg.Buffer[3]) {
	case 0x16:
		dprintf("WAP settings part 1 received OK\n");

		tmp = 4;

		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[0].Title,false);
		dprintf("Title: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].Title));

		NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[0].HomePage,false);
		dprintf("Homepage: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].HomePage));
#ifdef DEBUG
		dprintf("Connection type: ");      
		switch (msg.Buffer[tmp]) {
			case 0x00: dprintf("temporary\n"); 	break;
			case 0x01: dprintf("continuous\n"); 	break;
			default:   dprintf("unknown\n");
		}
		dprintf("Connection security: ");
		switch (msg.Buffer[tmp+13]) {
			case 0x00: dprintf("off\n");		break;
			case 0x01: dprintf("on\n");		break;
			default:   dprintf("unknown\n");
		}
#endif
		Data->WAPSettings->Settings[0].IsContinuous = false;
		if (msg.Buffer[tmp] == 0x01) Data->WAPSettings->Settings[0].IsContinuous = true;
		Data->WAPSettings->Settings[0].IsSecurity = false;
		if (msg.Buffer[tmp+13] == 0x01) Data->WAPSettings->Settings[0].IsSecurity = true;

		/* I'm not sure here. Experimental values from 6210 5.56 */
		if (!(strlen(DecodeUnicodeString(Data->WAPSettings->Settings[0].Title))) % 2) tmp++;
		if (strlen(DecodeUnicodeString(Data->WAPSettings->Settings[0].HomePage))!=0) tmp++;

		dprintf("ID for writing %i\n",msg.Buffer[tmp+5]);

		dprintf("Current set location in phone %i\n",msg.Buffer[tmp+6]);

		dprintf("1 location %i\n",msg.Buffer[tmp+8]);
		dprintf("2 location %i\n",msg.Buffer[tmp+9]);
		dprintf("3 location %i\n",msg.Buffer[tmp+10]);
		dprintf("4 location %i\n",msg.Buffer[tmp+11]);
#ifdef GSM_ENABLE_NOKIA7110
		if (strstr(N7110Phone.models, GetModelData(NULL,Data->Model,NULL)->model) != NULL) {
			Priv7110->WAPLocations.ID 		= msg.Buffer[tmp+5];
			Priv7110->WAPLocations.CurrentLocation	= msg.Buffer[tmp+6];
			Priv7110->WAPLocations.Locations[0] 	= msg.Buffer[tmp+8];
			Priv7110->WAPLocations.Locations[1] 	= msg.Buffer[tmp+9];
			Priv7110->WAPLocations.Locations[2] 	= msg.Buffer[tmp+10];
			Priv7110->WAPLocations.Locations[3] 	= msg.Buffer[tmp+11];
		}
#endif
#ifdef GSM_ENABLE_NOKIA6110
		if (strstr(N6110Phone.models, GetModelData(NULL,Data->Model,NULL)->model) != NULL) {
			Priv6110->WAPLocations.ID 		= msg.Buffer[tmp+5];
			Priv6110->WAPLocations.CurrentLocation	= msg.Buffer[tmp+6];
			Priv6110->WAPLocations.Locations[0] 	= msg.Buffer[tmp+8];
			Priv6110->WAPLocations.Locations[1] 	= msg.Buffer[tmp+9];
			Priv6110->WAPLocations.Locations[2] 	= msg.Buffer[tmp+10];
			Priv6110->WAPLocations.Locations[3] 	= msg.Buffer[tmp+11];
		}
#endif
		return GE_NONE;
	case 0x17:
		dprintf("WAP settings part 1 receiving error\n");
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
	case 0x1c:
		dprintf("WAP settings part 2 received OK\n");
		Number = Data->WAPSettings->Number;
		switch (msg.Buffer[5]) {
		case 0x00:
			Data->WAPSettings->Settings[Number].Bearer = WAPSETTINGS_BEARER_SMS;
			dprintf("Settings for SMS bearer:\n");
			tmp = 6;

			NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[Number].Service,false);
			dprintf("Service number: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].Service));

			NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[Number].Server,false);
			dprintf("Server number: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].Server));
			break;
		case 0x01:
			Data->WAPSettings->Settings[Number].Bearer = WAPSETTINGS_BEARER_DATA;
			dprintf("Settings for data bearer:\n");
			Data->WAPSettings->Settings[Number].ManualLogin = false;
			tmp = 10;

			NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[Number].IPAddress,false);
			dprintf("IP address: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].IPAddress));

			NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[Number].DialUp,false);
			dprintf("Dial-up number: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].DialUp));

			NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[Number].User,false);
			dprintf("User name: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].User));

			NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[Number].Password,false);
			dprintf("Password: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].Password));
#ifdef DEBUG
			dprintf("Authentication type: ");
			switch (msg.Buffer[6]) {
				case 0x00: dprintf("normal\n");	 break;
				case 0x01: dprintf("secure\n");	 break;
				default:   dprintf("unknown\n"); break;
			}
			dprintf("Data call type: ");
			switch (msg.Buffer[7]) {
				case 0x00: dprintf("analogue\n");break;
				case 0x01: dprintf("ISDN\n");	 break;
				default:   dprintf("unknown\n"); break;
			}
			dprintf("Data call speed: ");
			switch (msg.Buffer[9]) {
				case 0x01: dprintf("9600\n");	 break;
				case 0x02: dprintf("14400\n");	 break;
				default:   dprintf("unknown\n"); break;
			}
#endif
			Data->WAPSettings->Settings[Number].IsNormalAuthentication=true;
			if (msg.Buffer[6]==0x01) Data->WAPSettings->Settings[Number].IsNormalAuthentication=false;
			Data->WAPSettings->Settings[Number].IsISDNCall=false;
			if (msg.Buffer[7]==0x01) Data->WAPSettings->Settings[Number].IsISDNCall=true;
			Data->WAPSettings->Settings[Number].Speed = WAPSETTINGS_SPEED_9600;
			if (msg.Buffer[9]==0x02) Data->WAPSettings->Settings[Number].Speed = WAPSETTINGS_SPEED_14400;
			break;	
		case 0x02:
			Data->WAPSettings->Settings[Number].Bearer=WAPSETTINGS_BEARER_USSD;
			dprintf("Settings for USSD bearer:\n");
			tmp = 7;
			NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[Number].Service,false);
#ifdef DEBUG
			if (msg.Buffer[6]==0x01) 
				dprintf("Service number: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].Service));
			else 
				dprintf("IP address: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].Service));
#endif
			Data->WAPSettings->Settings[Number].IsIP=true;
			if (msg.Buffer[6]==0x01) Data->WAPSettings->Settings[Number].IsIP=false;
			NOKIA_GetUnicodeString(&tmp, msg.Buffer, Data->WAPSettings->Settings[Number].Code,false);
			dprintf("Service code: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].Code));
		}
		Data->WAPSettings->Number++;
		return GE_NONE;
	case 0x1d:
		dprintf("Incorrect WAP settings location\n");
		return GE_NONE;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error DCT3_GetWAPSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
#ifdef GSM_ENABLE_NOKIA6110
	GSM_Phone_N6110Data 	*Priv6110 = &s->Phone.Data.Priv.N6110;
#endif
#ifdef GSM_ENABLE_NOKIA7110
	GSM_Phone_N7110Data 	*Priv7110 = &s->Phone.Data.Priv.N7110;
#endif
	GSM_Error 		error;
	int			i;
	unsigned char 		req[] = {
		N6110_FRAME_HEADER,0x15,
		0x00};		/* Location */
	unsigned char 		req2[] = {
		N6110_FRAME_HEADER,0x1b,
		0x00};		/* Location */

	/* We have to enable WAP frames in phone */
	error=DCT3DCT4_EnableWAP(s);
	if (error!=GE_NONE) return error;

	s->Phone.Data.WAPSettings = settings;
	settings->Number = 0;

	req[4] = settings->Location-1;
	dprintf("Getting WAP settins part 1\n");
	error = GSM_WaitFor (s, req, 6, 0x3f, 4, ID_GetWAPSettings);
	if (error != GE_NONE) return error;

#ifdef GSM_ENABLE_NOKIA7110
	if (strstr(N7110Phone.models, GetModelData(NULL,s->Model,NULL)->model) != NULL) {
		for (i=0;i<4;i++) {
			req2[4] = Priv7110->WAPLocations.Locations[i];
			dprintf("Getting WAP settins part 2\n");
			error=GSM_WaitFor (s, req2, 6, 0x3f, 4, ID_GetWAPSettings);
			if (error != GE_NONE) return error;
		}
	}
#endif
#ifdef GSM_ENABLE_NOKIA6110
	if (strstr(N6110Phone.models, GetModelData(NULL,s->Model,NULL)->model) != NULL) {
		for (i=0;i<4;i++) {
			req2[4] = Priv6110->WAPLocations.Locations[i];
			dprintf("Getting WAP settins part 2\n");
			error=GSM_WaitFor (s, req2, 6, 0x3f, 4, ID_GetWAPSettings);
			if (error != GE_NONE) return error;
		}
	}
#endif
	if (error == GE_NONE) {
		for (i=1;i<3;i++) {
			CopyUnicodeString(settings->Settings[i].Title,settings->Settings[0].Title);
			CopyUnicodeString(settings->Settings[i].HomePage,settings->Settings[0].HomePage);
			settings->Settings[i].IsContinuous = settings->Settings[0].IsContinuous;
			settings->Settings[i].IsSecurity   = settings->Settings[0].IsSecurity;
		}
	}
	return error;
}

GSM_Error DCT3_ReplySetWAPSettings(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch(msg.Buffer[3]) {
	case 0x19:
		dprintf("WAP settings part 1 set OK\n");
		return GE_NONE;
	case 0x1a:
		dprintf("WAP settings part 1 setting error\n");
		switch (msg.Buffer[4]) {
		case 0x01:
			dprintf("Security error. Inside WAP settings menu\n");
			return GE_INSIDEPHONEMENU;
		case 0x02:
			dprintf("Incorrect data\n");
			return GE_UNKNOWN;
		default:
			dprintf("ERROR: unknown %i\n",msg.Buffer[4]);
			return GE_UNKNOWNRESPONSE;
		}
	case 0x1F:
		dprintf("WAP settings part 2 set OK\n");
		return GE_NONE;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error DCT3_SetWAPSettings(GSM_StateMachine *s, GSM_MultiWAPSettings *settings)
{
#ifdef GSM_ENABLE_NOKIA6110
	GSM_Phone_N6110Data 	*Priv6110 = &s->Phone.Data.Priv.N6110;
#endif
#ifdef GSM_ENABLE_NOKIA7110
	GSM_Phone_N7110Data 	*Priv7110 = &s->Phone.Data.Priv.N7110;
#endif
	GSM_Error 		error;
	GSM_MultiWAPSettings	settings2;
	int			i,pos,phone1=-1,phone2=-1,phone3=-1;
	int			ID=0,locations[4],loc1=-1,loc2=-1,loc3=-1;
	unsigned char 		req[] = {
		N6110_FRAME_HEADER,0x15,
		0x00};		/* Location */
	unsigned char 		req2[] = {
		N6110_FRAME_HEADER,0x1b,
		0x00};		/* Location */
	unsigned char SetReq[200] = {
		N7110_FRAME_HEADER, 0x18,
		0x00}; 		/* Location */
	unsigned char SetReq2[200] = {
		N7110_FRAME_HEADER, 0x1e,
		0x00}; 		/* Location */

	/* We have to enable WAP frames in phone */
	error=DCT3DCT4_EnableWAP(s);
	if (error!=GE_NONE) return error;

	s->Phone.Data.WAPSettings = &settings2;
	settings2.Number 	  = 0;

	req[4] = settings->Location-1;
	dprintf("Getting WAP settings part 1\n");
	error = GSM_WaitFor (s, req, 6, 0x3f, 4, ID_GetWAPSettings);
	if (error != GE_NONE) return error;

#ifdef GSM_ENABLE_NOKIA6110
	if (strstr(N6110Phone.models, GetModelData(NULL,s->Model,NULL)->model) != NULL) {
		for (i=0;i<4;i++) locations[i] = Priv6110->WAPLocations.Locations[i];
		ID = Priv6110->WAPLocations.ID;
	}
#endif
#ifdef GSM_ENABLE_NOKIA7110
	if (strstr(N7110Phone.models, GetModelData(NULL,s->Model,NULL)->model) != NULL) {
		for (i=0;i<4;i++) locations[i] = Priv7110->WAPLocations.Locations[i];
		ID = Priv7110->WAPLocations.ID;
	}
#endif

	/* Now we get info about supported types by phone and their locations */
	for (i=0;i<4;i++) {
		settings2.Number = 0;
		settings2.Settings[0].Bearer = 0;
		req2[4] = locations[i];
		dprintf("Getting WAP settins part 2\n");
		error=GSM_WaitFor (s, req2, 6, 0x3f, 4, ID_GetWAPSettings);
		if (error != GE_NONE) return error;
		switch (settings2.Settings[0].Bearer) {
			case WAPSETTINGS_BEARER_DATA: phone1 = locations[i]; break;
			case WAPSETTINGS_BEARER_SMS : phone2 = locations[i]; break;
			case WAPSETTINGS_BEARER_USSD: phone3 = locations[i]; break;
			default			    : break;
		}
		if (error != GE_NONE) return error;
	}

	/* We have some phone locations and some data to set. We try to
 	 * find info about locations in phone used to write concrete bearers
	 */
	for (i=0;i<settings->Number;i++) {
		if (settings->Settings[i].Bearer == WAPSETTINGS_BEARER_DATA) {
			if (phone1 != -1) loc1=i;
		}
		if (settings->Settings[i].Bearer == WAPSETTINGS_BEARER_SMS) {
			if (phone2 != -1) loc2=i;
		}
		if (settings->Settings[i].Bearer == WAPSETTINGS_BEARER_USSD) {
			if (phone3 != -1) loc3=i;
		}
	}

	pos = 5;
	memset(SetReq + pos, 0, 200 - pos);
	SetReq[4] = settings->Location - 1;
	if (loc1 != -1) {
		/* Name */
		pos += NOKIA_SetUnicodeString(SetReq + pos, settings->Settings[loc1].Title, false);
		/* HomePage */
		pos += NOKIA_SetUnicodeString(SetReq + pos, settings->Settings[loc1].HomePage, false);
		if (settings->Settings[loc1].IsContinuous) SetReq[pos] = 0x01;
		pos++;
		SetReq[pos++] = ID;
		SetReq[pos++] = phone1; /* bearer */
		if (settings->Settings[loc1].IsSecurity) SetReq[pos] = 0x01;
		pos++;
	} else if (loc2 != -1) {
		/* Name */
		pos += NOKIA_SetUnicodeString(SetReq + pos, settings->Settings[loc2].Title, false);
		/* HomePage */
		pos += NOKIA_SetUnicodeString(SetReq + pos, settings->Settings[loc2].HomePage, false);
		if (settings->Settings[loc2].IsContinuous) SetReq[pos] = 0x01;
		pos++;
		SetReq[pos++] = ID;
		SetReq[pos++] = phone2; /* bearer */
		if (settings->Settings[loc2].IsSecurity) SetReq[pos] = 0x01;
		pos++;
	} else if (loc3 != -1) {
		/* Name */
		pos += NOKIA_SetUnicodeString(SetReq + pos, settings->Settings[loc3].Title, false);
		/* HomePage */
		pos += NOKIA_SetUnicodeString(SetReq + pos, settings->Settings[loc3].HomePage, false);
		if (settings->Settings[loc3].IsContinuous) SetReq[pos] = 0x01;
		pos++;
		SetReq[pos++] = ID;
		SetReq[pos++] = phone3; /* bearer */
		if (settings->Settings[loc3].IsSecurity) SetReq[pos] = 0x01;
		pos++;
	} else {
		return GE_UNKNOWN;	/* We have to have write something known */
	}
	memcpy(SetReq + pos, "\x01\x80\x00\x00\x00\x00\x00\x00\x00", 9);
	pos += 9;

	dprintf("Writing WAP settings part 1\n");
	error=GSM_WaitFor (s, SetReq, pos, 0x3f, 4, ID_SetWAPSettings);
	if (error != GE_NONE) return error;

	/* Data */
	if (phone1 != -1) {
		pos = 4;
		memset(SetReq2 + pos, 0, 200 - pos);
		SetReq2[pos++] = phone1;
		SetReq2[pos++] = 0x02; 
		SetReq2[pos++] = 0x01; /* GSMdata */
		if (loc1 != -1) {
			if (!settings->Settings[loc1].IsNormalAuthentication) SetReq2[pos] = 0x01;
		}
		pos++;
		if (loc1 != -1) {
			if (settings->Settings[loc1].IsISDNCall) SetReq2[pos] = 0x01;
		}
		pos++;
		if (loc1 != -1) {
			switch (settings->Settings[loc1].Speed) {
				case WAPSETTINGS_SPEED_9600  : SetReq2[pos++] = 0x01; break;
				case WAPSETTINGS_SPEED_14400 : SetReq2[pos++] = 0x02; break;
				default			     : SetReq2[pos++] = 0x02; break;
			}
			switch (settings->Settings[loc1].Speed) {
				case WAPSETTINGS_SPEED_9600  : SetReq2[pos++] = 0x01; break;
				case WAPSETTINGS_SPEED_14400 : SetReq2[pos++] = 0x02; break;
				default			     : SetReq2[pos++] = 0x02; break;
			}
		} else pos+=2;
		if (loc1 != -1) {
			/* IP */
			pos += NOKIA_SetUnicodeString(SetReq2 + pos, settings->Settings[loc1].IPAddress, false);
			/* Number */
			pos += NOKIA_SetUnicodeString(SetReq2 + pos, settings->Settings[loc1].DialUp, false);
			/* Username  */
			pos += NOKIA_SetUnicodeString(SetReq2 + pos, settings->Settings[loc1].User, false);
			/* Password */
			pos += NOKIA_SetUnicodeString(SetReq2 + pos, settings->Settings[loc1].Password, false);		
		} else pos+=5;
		memcpy(SetReq2 + pos, "\x80\x00\x00\x00\x00\x00\x00\x00", 8);
		pos += 8;
		dprintf("Writing WAP settings part 2 (Data bearer)\n");
		error=GSM_WaitFor (s, SetReq2, pos, 0x3f, 4, ID_SetWAPSettings);
		if (error != GE_NONE) return error;
	}
	/* SMS */
	if (phone2 != -1) {
		pos = 4;
		memset(SetReq2 + pos, 0, 200 - pos);
		SetReq2[pos++] = phone2;
		SetReq2[pos++] = 0x02; 
		SetReq2[pos++] = 0x00; /* SMS */
		if (loc2 != -1) {
			/* Service number */
			pos += NOKIA_SetUnicodeString(SetReq2 + pos, settings->Settings[loc2].Service, false);
			/* Server number */
			pos += NOKIA_SetUnicodeString(SetReq2 + pos, settings->Settings[loc2].Server, false);
		} else pos += 2;
		memcpy(SetReq2 + pos, "\x80\x00\x00\x00\x00\x00\x00\x00", 8);
		pos += 8;
		dprintf("Writing WAP settings part 2 (SMS bearer)\n");
		error=GSM_WaitFor (s, SetReq2, pos, 0x3f, 4, ID_SetWAPSettings);
		if (error != GE_NONE) return error;
	}
	/* USSD */
	if (phone3 != -1) {
		pos = 4;
		memset(SetReq2 + pos, 0, 200 - pos);
		SetReq2[pos++] = phone3;
		SetReq2[pos++] = 0x02; 
		SetReq2[pos++] = 0x02; /* USSD */
		if (loc3 != -1) {
			if (!settings->Settings[loc3].IsIP) SetReq2[pos] = 0x01;
		}
		pos++;
		if (loc3 != -1) {
			/* Service number or IP address */
			pos += NOKIA_SetUnicodeString(SetReq2 + pos, settings->Settings[loc3].Service, false);
			/* Code number */
			pos += NOKIA_SetUnicodeString(SetReq2 + pos, settings->Settings[loc3].Code, false);
		} else pos+=2;
		memcpy(SetReq2 + pos, "\x80\x00\x00\x00\x00\x00\x00\x00", 8);
		pos += 8;
		dprintf("Writing WAP settings part 2 (USSD bearer)\n");
		error=GSM_WaitFor (s, SetReq2, pos, 0x3f, 4, ID_SetWAPSettings);
		if (error != GE_NONE) return error;
	}
 	return error;
}

GSM_Error DCT3_ReplySendSMSMessage(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x02:
		dprintf("SMS sent OK\n");
		if (User->SendSMSStatus!=NULL) User->SendSMSStatus(Data->Device,0);
		return GE_NONE;
	case 0x03:
		dprintf("Error %i\n",msg.Buffer[6]);
		if (User->SendSMSStatus!=NULL) User->SendSMSStatus(Data->Device,msg.Buffer[6]);
		return GE_NONE;
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error DCT3_SendSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	int			length;
	GSM_Error		error;
	unsigned char req[256] = { N6110_FRAME_HEADER, 0x01, 0x02, 0x00 };

	error=PHONE_EncodeSMSFrame(s,sms,req+6,PHONE_SMSSubmit,&length, true);
	if (error != GE_NONE) return error;

	dprintf("Sending sms\n");
	return s->Protocol.Functions->WriteMessage(s, req, 6+length, 0x02);
}

GSM_Error DCT3_ReplyNetmonitor(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x00:
		dprintf("Netmonitor correctly set\n");
		break;
	default:
		dprintf("Menu %i\n",msg.Buffer[3]);
		dprintf("%s\n",msg.Buffer+4);
		strcpy(Data->Netmonitor,msg.Buffer+4);
		break;		
	}
	return GE_NONE;
}

GSM_Error DCT3_Netmonitor(GSM_StateMachine *s, int testnumber, char *value)
{
	GSM_Error 	error;
	unsigned char 	req[] = {
		0x00, 0x01, 0x7e,
		0x00};		/* Test number */

	value[0] = 0;

	error=DCT3_EnableSecurity (s, 0x01);
	if (error != GE_NONE) return error;

	req[3] = testnumber;

	dprintf("Getting netmonitor test\n");
	s->Phone.Data.Netmonitor = value;
	return GSM_WaitFor (s, req, 4, 0x40, 4, ID_Netmonitor);
}

GSM_Error DCT3_GetManufactureMonth(GSM_StateMachine *s, char *value)
{
	GSM_Error error;	

	error=DCT3_EnableSecurity (s, 0x01);
	if (error != GE_NONE) return error;
	return NOKIA_GetPhoneString(s,"\x00\x01\xCC\x02",4,0x40,value,ID_GetManufactureMonth,5);
}

GSM_Error DCT3_GetProductCode(GSM_StateMachine *s, char *value)
{
	GSM_Error error;	

	if (strlen(s->Phone.Data.ProductCodeCache)!=0) {
		strcpy(value,s->Phone.Data.ProductCodeCache);
		return GE_NONE;
	}

	error=DCT3_EnableSecurity (s, 0x01);
	if (error != GE_NONE) return error;
	return NOKIA_GetPhoneString(s,"\x00\x01\xCA\x01",4,0x40,value,ID_GetProductCode,5);
}

GSM_Error DCT3_GetOriginalIMEI(GSM_StateMachine *s, char *value)
{
	GSM_Error error;	

	error=DCT3_EnableSecurity (s, 0x01);
	if (error != GE_NONE) return error;
	return NOKIA_GetPhoneString(s,"\x00\x01\xCC\x01",4,0x40,value,ID_GetOriginalIMEI,5);
}

GSM_Error DCT3_GetHardware(GSM_StateMachine *s, char *value)
{
	GSM_Error error;	

	if (strlen(s->Phone.Data.HardwareCache)!=0) {
		strcpy(value,s->Phone.Data.HardwareCache);
		return GE_NONE;
	}

	error=DCT3_EnableSecurity (s, 0x01);
	if (error != GE_NONE) return error;
	return NOKIA_GetPhoneString(s,"\x00\x01\xC8\x05",4,0x40,value,ID_GetHardware,5);
}

GSM_Error DCT3_GetPPM(GSM_StateMachine *s, char *value)
{
	GSM_Error error;	

	error=DCT3_EnableSecurity (s, 0x01);
	if (error != GE_NONE) return error;
	return NOKIA_GetPhoneString(s,"\x00\x01\xC8\x12",4,0x40,value,ID_GetPPM,5);
}

GSM_Error DCT3_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x36, 0x64};

	s->Phone.Data.SMSStatus=status;
	dprintf("Getting SMS status\n");
	return GSM_WaitFor (s, req, 5, 0x14, 3, ID_GetSMSStatus);

	/* Nokia 6210 and family does not show not "fixed" messages from the
	 * Templates folder, ie. when you save a message to the Templates folder,
	 * SMSStatus does not change! Workaround: get Templates folder status, which
	 * does show these messages.
	 */
}

GSM_Error DCT3_ReplyDeleteSMSMessage(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch(msg.Buffer[3]) {
	case 0x0b:
		dprintf("SMS deleted\n");
		return GE_NONE;
	case 0x0c:
		dprintf("Error deleting SMS\n");
		switch (msg.Buffer[4]) {
			case 0x02:
				/* Not tested on 6210 */
				dprintf("Invalid location\n");
				return GE_INVALIDLOCATION;
			case 0x06:
				/* Not tested on 6210 */
				dprintf("Phone is OFF\n");
				return GE_PHONEOFF;
			default:
				dprintf("Unknown error: %02x\n",msg.Buffer[4]);
				return GE_UNKNOWNRESPONSE;
		}
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error N71_92_ReplyGetNetworkLevel(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Network level received: %i\n",msg.Buffer[4]);
	*Data->NetworkLevel=((int)msg.Buffer[4]);
	return GE_NONE;
}

GSM_Error N71_92_GetNetworkLevel(GSM_StateMachine *s, int *level)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x81};

	s->Phone.Data.NetworkLevel=level;
	dprintf("Getting network level\n");
	return GSM_WaitFor (s, req, 4, 0x0a, 4, ID_GetNetworkLevel);
}

GSM_Error N71_92_ReplyGetBatteryLevel(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Battery level received: %i\n",msg.Buffer[5]);
	*Data->BatteryLevel=((int)msg.Buffer[5]);
	return GE_NONE;
}

GSM_Error N71_92_GetBatteryLevel(GSM_StateMachine *s, int *level)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x02};

	s->Phone.Data.BatteryLevel=level;
	dprintf("Getting battery level\n");
	return GSM_WaitFor (s, req, 4, 0x17, 4, ID_GetBatteryLevel);
}

GSM_Error N71_92_ReplyPhoneSetting(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	GSM_Phone_Bitmap_Types BmpType;

	switch (msg.Buffer[4]) {
	case 0x02:
		if (Data->RequestID == ID_GetBitmap || Data->RequestID == ID_EachFrame) {
			dprintf("Welcome note text received\n");
			CopyUnicodeString(Data->Bitmap->Text,msg.Buffer+5);
			dprintf("Text is \"%s\"\n",DecodeUnicodeString(Data->Bitmap->Text));
			return GE_NONE;
		}
		if (Data->RequestID == ID_SetBitmap || Data->RequestID == ID_EachFrame) {
			dprintf("Startup text set\n");
			return GE_NONE;
		}
	case 0x15:
		if (Data->RequestID == ID_GetBitmap || Data->RequestID == ID_EachFrame) {
			dprintf("Startup logo received\n");
			BmpType=GSM_Nokia7110StartupLogo;
			if (msg.Buffer[17]==0x60) BmpType=GSM_Nokia6210StartupLogo;
			if (msg.Buffer[17]==0xc0) BmpType=GSM_NokiaStartupLogo;
			PHONE_DecodeBitmap(BmpType, msg.Buffer+22, Data->Bitmap);
			return GE_NONE;
		}
		if (Data->RequestID == ID_SetBitmap || Data->RequestID == ID_EachFrame) {
			dprintf("Startup logo set\n");
			return GE_NONE;
		}
	}
	return GE_UNKNOWNRESPONSE;
}

GSM_Error N71_92_GetPhoneSetting(GSM_StateMachine *s, int Request, int Setting)
{
	unsigned char req[] = {
		N7110_FRAME_HEADER, 0xee,
		0x1c};			/* Setting */

	req[4]=Setting;
	return GSM_WaitFor (s, req, 5, 0x7a, 4, Request);
}

GSM_Error N71_92_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	return DCT3_GetDateTime(s, date_time, 0x19);
}

GSM_Error N71_92_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time)
{
	return DCT3_SetDateTime(s, date_time, 0x19);
}

GSM_Error DCT3_DecodeSMSFrame(GSM_SMSMessage *SMS, unsigned char *buffer)
{
	switch (buffer[12] & 0x03) {
	case 0x00:
		dprintf("SMS type - deliver\n");
		SMS->PDU = SMS_Deliver;
		return GSM_DecodeSMSFrame(SMS,buffer,PHONE_SMSDeliver);
	case 0x01:
		dprintf("SMS type - submit\n");
		SMS->PDU = SMS_Submit;
		return GSM_DecodeSMSFrame(SMS,buffer,PHONE_SMSSubmit);
	case 0x02:
		dprintf("SMS type - delivery report\n");
		SMS->PDU = SMS_Status_Report;
		return GSM_DecodeSMSFrame(SMS,buffer,PHONE_SMSStatusReport);
	}
	return GE_UNKNOWN;
}

GSM_Error N61_91_ReplySetOpLogo(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x31:
		dprintf("Operator logo set OK\n");
		return GE_NONE;
	case 0x32:
		dprintf("Error setting operator logo\n");
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

GSM_Error N61_71_ReplyResetPhoneSettings(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	dprintf("Phone settings cleaned OK\n");
	return GE_NONE;
}

GSM_Error N61_71_ResetPhoneSettings(GSM_StateMachine *s, GSM_ResetSettingsType Type)
{
	GSM_Error	error;
	unsigned char 	req[] = {
		0x00, 0x01, 0x65,
		0x01};			/* Reset type */

	switch (Type) {
	case GSM_RESET_PHONESETTINGS			: req[3] = 0x01; break;
	case GSM_RESET_DEVICE				: req[3] = 0x02; break;
	case GSM_RESET_USERINTERFACE			: req[3] = 0x08; break;
	case GSM_RESET_USERINTERFACE_PHONESETTINGS	: req[3] = 0x38; break;
	case GSM_RESET_FULLFACTORY			: req[3] = 0xff; break;
	}

	error=DCT3_EnableSecurity (s, 0x01);
	if (error != GE_NONE) return error;

	return GSM_WaitFor (s, req, 4, 0x40, 4, ID_ResetPhoneSettings);
}

#endif
