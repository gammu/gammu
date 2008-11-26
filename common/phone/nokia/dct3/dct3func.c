/* (c) 2001-2004 by Marcin Wiacek */
/* resetting DCT4 phones settings (c) by Walek */
/* based on some Markus Plail, Pavel Janik & others work from Gnokii (www.gnokii.org)
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

#include <string.h> /* memcpy only */
#include <stdio.h>
#include <ctype.h>

#include "../../../gsmstate.h"
#include "../../../misc/coding/coding.h"
#include "../../pfunc.h"
#include "../nfunc.h"
#include "dct3func.h"

#include <gammu-config.h>

#ifdef GSM_ENABLE_NOKIA_DCT3

GSM_Error DCT3_DeleteWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error error;

	/* We have to enable WAP frames in phone */
	error=DCT3DCT4_EnableWAPFunctions(s);
	if (error!=ERR_NONE) return error;

	return DCT3DCT4_DeleteWAPBookmarkPart(s,bookmark);
}

GSM_Error DCT3_GetWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error error;

	/* We have to enable WAP frames in phone */
	error=DCT3DCT4_EnableWAPFunctions(s);
	if (error!=ERR_NONE) return error;

	return DCT3DCT4_GetWAPBookmarkPart(s,bookmark);
}

GSM_Error DCT3_ReplyPressKey(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	switch (msg.Buffer[2]) {
	case 0x46:
		smprintf(s, "Pressing key OK\n");
		if (Data->PressKey) return ERR_NONE;
		break;
	case 0x47:
		smprintf(s, "Releasing key OK\n");
		if (!Data->PressKey) return ERR_NONE;
		break;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error DCT3_PressKey(GSM_StateMachine *s, GSM_KeyCode Key, bool Press)
{
	unsigned char PressReq[]   = {0x00, 0x01, 0x46, 0x00, 0x01,
				      0x0a};		/* Key code */
	unsigned char ReleaseReq[] = {0x00, 0x01, 0x47, 0x00, 0x01, 0x0c};

	if (Press) {
		PressReq[5] = Key;
		s->Phone.Data.PressKey = true;
		smprintf(s, "Pressing key\n");
		return GSM_WaitFor (s, PressReq, 6, 0xd1, 4, ID_PressKey);
	} else {
		s->Phone.Data.PressKey = false;
		smprintf(s, "Releasing key\n");
		return GSM_WaitFor (s, ReleaseReq, 6, 0xd1, 4, ID_PressKey);
	}
}

GSM_Error DCT3_ReplyPlayTone(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Tone played\n");
	return ERR_NONE;
}

GSM_Error DCT3_PlayTone(GSM_StateMachine *s, int Herz, unsigned char Volume, bool start)
{
	GSM_Error 	error;
	unsigned char 	req[] = {0x00,0x01,0x8f,
				 0x00,		/* Volume */
			 	 0x00,		/* HerzLo */
				 0x00};		/* HerzHi */

	if (start) {
		error=DCT3_EnableSecurity (s, 0x01);
		if (error!=ERR_NONE) return error;
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

GSM_Error DCT3_ReplyIncomingCB(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_CBMessage 	CB;
	int		i;
	char		Buffer[300];

	smprintf(s, "CB received\n");
	CB.Channel = msg.Buffer[7];
	i = GSM_UnpackEightBitsToSeven(0, msg.Buffer[9], msg.Buffer[9], msg.Buffer+10, Buffer);
	i = msg.Buffer[9] - 1;
	while (i!=0) {
		if (Buffer[i] == 13) i = i - 1; else break;
	}
	DecodeDefault(CB.Text, Buffer, i + 1, false, NULL);
	smprintf(s, "Channel %i, text \"%s\"\n",CB.Channel,DecodeUnicodeString(CB.Text));
	if (s->Phone.Data.EnableIncomingCB && s->User.IncomingCB!=NULL) {
		s->User.IncomingCB(s,CB);
	}
	return ERR_NONE;
}

GSM_Error DCT3_ReplySetIncomingCB(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
	case 0x21:
		smprintf(s, "CB set\n");
		return ERR_NONE;
	case 0x22:
		smprintf(s, "CB not set\n");
		return ERR_UNKNOWN;
	case 0xCA:
		smprintf(s, "No network and no CB\n");
		return ERR_SECURITYERROR;
	}
	return ERR_UNKNOWNRESPONSE;
}

#endif

GSM_Error DCT3_SetIncomingCB(GSM_StateMachine *s, bool enable)
{
#ifdef GSM_ENABLE_CELLBROADCAST
	unsigned char reqOn[]  = {N6110_FRAME_HEADER, 0x20, 0x01,
				  0x01, 0x00, 0x00, 0x01, 0x01};
	unsigned char reqOff[] = {N6110_FRAME_HEADER, 0x20, 0x00,
		 		  0x00, 0x00, 0x00, 0x00, 0x00};

	if (s->Phone.Data.EnableIncomingCB!=enable) {
		s->Phone.Data.EnableIncomingCB 	= enable;
		if (enable) {
			smprintf(s, "Enabling incoming CB\n");
			return GSM_WaitFor(s, reqOn, 10, 0x02, 4, ID_SetIncomingCB);
		} else {
			smprintf(s, "Disabling incoming CB\n");
			return GSM_WaitFor(s, reqOff, 10, 0x02, 4, ID_SetIncomingCB);
		}
	}
	return ERR_NONE;
#else
	return ERR_SOURCENOTAVAILABLE;
#endif
}

GSM_Error DCT3_ReplySetSMSC(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "SMSC set\n");
	return ERR_NONE;
}

GSM_Error DCT3_SetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	unsigned char req[100] = {N6110_FRAME_HEADER, 0x30, 0x64};

	memset(req+6,0,100-6);

	/* SMSC location */
	req[5] = smsc->Location;

	/* SMSC format */
	switch (smsc->Format) {
		case SMS_FORMAT_Text  : req[7] = 0x00; break;
		case SMS_FORMAT_Fax   : req[7] = 0x22; break;
		case SMS_FORMAT_Pager : req[7] = 0x26; break;
		case SMS_FORMAT_Email : req[7] = 0x32; break;
	}

	/* SMS validity */
	req[9] = smsc->Validity.Relative;

	/* Default number for SMS messages */
	req[10] = GSM_PackSemiOctetNumber(smsc->DefaultNumber, req+11, true);

	/* SMSC number */
	req[22] = GSM_PackSemiOctetNumber(smsc->Number, req+23, false);

	/* SMSC name */
	memcpy(req + 34, DecodeUnicodeString(smsc->Name),UnicodeLength(smsc->Name));

	smprintf(s, "Setting SMSC\n");
	return GSM_WaitFor (s, req, 35+UnicodeLength(smsc->Name), 0x02, 4, ID_SetSMSC);
}

GSM_Error DCT3_ReplyEnableSecurity(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "State of security commands set\n");
	return ERR_NONE;
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
	unsigned char req[] = {0x00, 0x01, 0x64,
			       0x01};	/* 0x00/0x01 - off/on,
					 * 0x03/0x04 - soft/hard reset,
					 * 0x06      - CONTACT SERVICE
					 */

	/* 0x06 MAKES CONTACT SERVICE! BE CAREFULL! */
	/* When use 0x03 and had during session changed time & date
	 * some phones (like 6150 or 6210) can ask for time & date after reset
	 * or disable clock on the screen
	 */
	if (status!=0x06) req[3] = status;
	smprintf(s, "Setting state of security commands\n");
	return GSM_WaitFor (s, req, 4, 0x40, 4, ID_EnableSecurity);
}

GSM_Error DCT3_ReplyGetIMEI(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	memcpy(s->Phone.Data.IMEI,msg.Buffer + 4, 16);
	smprintf(s, "Received IMEI %s\n",s->Phone.Data.IMEI);
	return ERR_NONE;
}

GSM_Error DCT3_GetIMEI (GSM_StateMachine *s)
{
	unsigned char 	req[] = {0x00, 0x01, 0x66, 0x00};
	GSM_Error 	error;

	if (strlen(s->Phone.Data.IMEI)!=0) return ERR_NONE;

	error=DCT3_EnableSecurity (s, 0x01);
	if (error!=ERR_NONE) return error;

	smprintf(s, "Getting IMEI\n");
	return GSM_WaitFor (s, req, 4, 0x40, 2, ID_GetIMEI);
}

GSM_Error DCT3_ReplySIMLogin(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Login for SIM card\n");
	return ERR_NONE;
}

GSM_Error DCT3_ReplySIMLogout(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Logout for SIM card\n");
	return ERR_NONE;
}

GSM_Error DCT3_ReplyGetDateTime(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Date & time received\n");
	if (msg.Buffer[4]==0x01) {
		NOKIA_DecodeDateTime(s, msg.Buffer+8, s->Phone.Data.DateTime);
		return ERR_NONE;
	}
	smprintf(s, "Not set in phone\n");
	return ERR_EMPTY;
}

GSM_Error DCT3_GetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time, unsigned char msgtype)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x62};

	s->Phone.Data.DateTime=date_time;
	smprintf(s, "Getting date & time\n");
	return GSM_WaitFor (s, req, 4, msgtype, 4, ID_GetDateTime);
}

GSM_Error DCT3_ReplyGetAlarm(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data	*Data = &s->Phone.Data;

	smprintf(s, "Alarm: ");
	if (msg.Buffer[8]==0x02) {
		smprintf(s, "set to %02i:%02i\n", msg.Buffer[9], msg.Buffer[10]);
		Data->Alarm->Repeating 		= true;
		Data->Alarm->Text[0] 		= 0;
		Data->Alarm->Text[1] 		= 0;
		Data->Alarm->DateTime.Hour	= msg.Buffer[9];
		Data->Alarm->DateTime.Minute	= msg.Buffer[10];
		Data->Alarm->DateTime.Second	= 0;
		return ERR_NONE;
	}
	smprintf(s, "not set\n");
	return ERR_EMPTY;
}

GSM_Error DCT3_GetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm, unsigned char msgtype)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x6d};

	if (Alarm->Location!=1) return ERR_NOTSUPPORTED;

	s->Phone.Data.Alarm=Alarm;
	smprintf(s, "Getting alarm\n");
	return GSM_WaitFor (s, req, 4, msgtype, 4, ID_GetAlarm);
}

GSM_Error DCT3_ReplySetDateTime(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Date & time: ");
	if (msg.Buffer[4]==0x01) {
		smprintf(s, "set OK\n");
		return ERR_NONE;
	}
	smprintf(s, "error setting\n");
	return ERR_UNKNOWN;
}

GSM_Error DCT3_SetDateTime(GSM_StateMachine *s, GSM_DateTime *date_time, unsigned char msgtype)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x60, 0x01, 0x01, 0x07,
			       0x00, 0x00,	/* Year 		*/
			       0x00,		/* Month 		*/
			       0x00,		/* Day 			*/
			       0x00,            /* Hour 		*/
			       0x00,		/* Minute 		*/
			       0x00};		/* Unknown. Not seconds */

	NOKIA_EncodeDateTime(s, req+7, date_time);
	smprintf(s, "Setting date & time\n");
	return GSM_WaitFor (s, req, 14, msgtype, 4, ID_SetDateTime);
}

GSM_Error DCT3_ReplySetAlarm(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	smprintf(s, "Alarm: ");
	if (msg.Buffer[4]==0x01) {
		smprintf(s, "set OK\n");
		return ERR_NONE;
	}
	smprintf(s, "error setting\n");
	return ERR_UNKNOWN;
}

GSM_Error DCT3_SetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm, unsigned char msgtype)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x6b, 0x01, 0x20, 0x03,
			       0x02,      /* Unknown. Not for enabling/disabling */
			       0x00,      /* Hour 				 */
			       0x00,	  /* Minute 				 */
			       0x00};	  /* Unknown. Not seconds 		 */

	if (Alarm->Location != 1) return ERR_NOTSUPPORTED;

	req[8] = Alarm->DateTime.Hour;
	req[9] = Alarm->DateTime.Minute;

	smprintf(s, "Setting alarm\n");
	return GSM_WaitFor (s, req, 11, msgtype, 4, ID_SetAlarm);
}

GSM_Error DCT3_ReplyWaitSMSC(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Waiting for SMSC reply\n");
	return ERR_NONE;
}

GSM_Error DCT3_ReplyGetSMSC(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 		i;
	GSM_Phone_Data	*Data = &s->Phone.Data;

	switch (msg.Buffer[3]) {
	case 0x34:
		smprintf(s, "SMSC received\n");
		Data->SMSC->Format = SMS_FORMAT_Text;
		switch (msg.Buffer[6]) {
			case 0x00: Data->SMSC->Format = SMS_FORMAT_Text; 	break;
			case 0x22: Data->SMSC->Format = SMS_FORMAT_Fax; 	break;
			case 0x26: Data->SMSC->Format = SMS_FORMAT_Pager;	break;
			case 0x32: Data->SMSC->Format = SMS_FORMAT_Email;	break;
		}
		Data->SMSC->Validity.Format 	= SMS_Validity_RelativeFormat;
		Data->SMSC->Validity.Relative	= msg.Buffer[8];
		if (msg.Buffer[8] == 0x00) Data->SMSC->Validity.Relative = SMS_VALID_Max_Time;

		i=33;
		while (msg.Buffer[i]!=0) {i++;}
		i=i-33;
		if (i>GSM_MAX_SMSC_NAME_LENGTH) {
			smprintf(s, "Too long name\n");
			return ERR_UNKNOWNRESPONSE;
		}
		EncodeUnicode(Data->SMSC->Name,msg.Buffer+33,i);
		smprintf(s, "Name \"%s\"\n", DecodeUnicodeString(Data->SMSC->Name));

		GSM_UnpackSemiOctetNumber(Data->SMSC->DefaultNumber,msg.Buffer+9,true);
		smprintf(s, "Default number \"%s\"\n", DecodeUnicodeString(Data->SMSC->DefaultNumber));

		GSM_UnpackSemiOctetNumber(Data->SMSC->Number,msg.Buffer+21,false);
		smprintf(s, "Number \"%s\"\n", DecodeUnicodeString(Data->SMSC->Number));

		return ERR_NONE;
	case 0x35:
		smprintf(s, "Getting SMSC failed\n");
		return ERR_INVALIDLOCATION;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error DCT3_GetSMSC(GSM_StateMachine *s, GSM_SMSC *smsc)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x33, 0x64,
			       0x00};		/* Location */

	if (smsc->Location==0x00) return ERR_INVALIDLOCATION;

	req[5]=smsc->Location;

	s->Phone.Data.SMSC=smsc;
	smprintf(s, "Getting SMSC\n");
	return GSM_WaitFor (s, req, 6, 0x02, 4, ID_GetSMSC);
}

GSM_Error DCT3_ReplyGetNetworkInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int		count;
	GSM_Phone_Data	*Data = &s->Phone.Data;
#ifdef DEBUG
	GSM_NetworkInfo NetInfo;
	char		name[100];

	smprintf(s, "Network info received\n");
	smprintf(s, "Status                 : ");
	switch (msg.Buffer[8]) {
		case 0x01: smprintf(s, "home network");				break;
		case 0x02: smprintf(s, "roaming network");			break;
		case 0x03: smprintf(s, "requesting network");			break;
		case 0x04: smprintf(s, "not registered in the network");	break;
		default  : smprintf(s, "unknown");
	}
	smprintf(s, "\n");
	smprintf(s, "Network selection      : %s\n", msg.Buffer[9]==1?"manual":"automatic");
	if (msg.Buffer[8]<0x03) {
		sprintf(NetInfo.CID, "%02X%02X", msg.Buffer[10], msg.Buffer[11]);
		smprintf(s, "CID                    : %s\n", NetInfo.CID);

		sprintf(NetInfo.LAC, "%02X%02X", msg.Buffer[12], msg.Buffer[13]);
		smprintf(s, "LAC                    : %s\n", NetInfo.LAC);

		NOKIA_DecodeNetworkCode(msg.Buffer+14,NetInfo.NetworkCode);
		smprintf(s, "Network code           : %s\n", NetInfo.NetworkCode);
		smprintf(s, "Network name for Gammu : %s ",
				DecodeUnicodeString(GSM_GetNetworkName(NetInfo.NetworkCode)));
		smprintf(s, "(%s)\n",DecodeUnicodeString(GSM_GetCountryName(NetInfo.NetworkCode)));

		if (msg.Length>18) {
			if (msg.Buffer[18]==0x00) {
				/* In 6210 name is in "normal" Unicode */
				memcpy(name,msg.Buffer+18,msg.Buffer[17]*2);
				name[msg.Buffer[17]*2]	=0x00;
				name[msg.Buffer[17]*2+1]=0x00;
				smprintf(s, "Network name for phone : %s\n",DecodeUnicodeString(name));
			} else {
				/* In 9210 first 0x00 is cut from Unicode string */
				name[0] = 0;
				memcpy(name+1,msg.Buffer+18,msg.Buffer[17]*2);
				name[msg.Buffer[17]*2+1]=0x00;
				name[msg.Buffer[17]*2+2]=0x00;
				smprintf(s, "Network name for phone : %s\n",DecodeUnicodeString(name));
			}
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
			sprintf(Data->NetworkInfo->CID, "%02X%02X", msg.Buffer[10], msg.Buffer[11]);
			sprintf(Data->NetworkInfo->LAC, "%02X%02X", msg.Buffer[12], msg.Buffer[13]);
		}
	}
	/* 6210/6250/7110 */
	if (Data->RequestID==ID_GetBitmap) {
		if (msg.Buffer[4]==0x02) {
			smprintf(s, "Operator logo available\n");
			count = 7;
			/* skip network info */
			count += msg.Buffer[count];
			count ++;
			Data->Bitmap->BitmapWidth	= msg.Buffer[count++];
			Data->Bitmap->BitmapHeight	= msg.Buffer[count++];
        		count+=4;
			PHONE_DecodeBitmap(GSM_Nokia7110OperatorLogo,msg.Buffer+count,Data->Bitmap);
			NOKIA_DecodeNetworkCode(msg.Buffer+14,Data->Bitmap->NetworkCode);
		} else {
			Data->Bitmap->BitmapWidth	= 78;
			Data->Bitmap->BitmapHeight	= 21;
			GSM_ClearBitmap(Data->Bitmap);
			strcpy(Data->Bitmap->NetworkCode,"000 00");
		}
	}
	return ERR_NONE;
}

GSM_Error DCT3_GetNetworkInfo(GSM_StateMachine *s, GSM_NetworkInfo *netinfo)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x70};

	s->Phone.Data.NetworkInfo=netinfo;
	smprintf(s, "Getting network info\n");
	return GSM_WaitFor (s, req, 4, 0x0a, 4, ID_GetNetworkInfo);
}

GSM_Error DCT3_ReplyDialCommand(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Answer for call commands\n");
	return ERR_NONE;
}

GSM_Error DCT3_DialVoice(GSM_StateMachine *s, char *number, GSM_CallShowNumber ShowNumber)
{
	unsigned int	i = 0;
	GSM_Error	error;
	unsigned char 	req[100] = {0x00, 0x01, 0x7c,
		                    0x01}; 		/* call command */

	if (ShowNumber != GSM_CALL_DefaultNumberPresence) return ERR_NOTSUPPORTED;

	error=DCT3_EnableSecurity (s, 0x01);
	if (error!=ERR_NONE) return error;

	for (i=0; i < strlen(number); i++) req[4+i]=number[i];
	req[4+i+1]=0;

	smprintf(s, "Making voice call\n");
	return GSM_WaitFor (s, req, 4+strlen(number)+1, 0x40, 4, ID_DialVoice);
}

static GSM_Error DCT3_CancelAllCalls(GSM_StateMachine *s)
{
	GSM_Error	error;
	unsigned char 	req[] = {0x00, 0x01, 0x7c,
		                 0x03}; 		/* call command */

	error=DCT3_EnableSecurity (s, 0x01);
	if (error!=ERR_NONE) return error;

	smprintf(s, "Canceling calls\n");
	return GSM_WaitFor (s, req, 4, 0x40, 4, ID_CancelCall);
}

GSM_Error DCT3_CancelCall(GSM_StateMachine *s, int ID, bool all)
{
	if (!all) return DCT3DCT4_CancelCall(s,ID);
	return DCT3_CancelAllCalls(s);
}

GSM_Error DCT3_AnswerAllCalls(GSM_StateMachine *s)
{
	GSM_Error	error;
	unsigned char 	req[] = {0x00, 0x01, 0x7c,
                		 0x02}; 		/* call command */

	error=DCT3_EnableSecurity (s, 0x01);
	if (error!=ERR_NONE) return error;

	smprintf(s, "Answering calls\n");
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
	if (error == ERR_NONE) {
		s->Phone.Data.EnableIncomingSMS = false;
		s->Phone.Data.EnableIncomingCB  = false;
	}
	return error;
}

GSM_Error DCT3_ReplyGetWAPBookmark(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	return DCT3DCT4_ReplyGetWAPBookmark (msg,s,false);
}

GSM_Error DCT3_SetWAPBookmark(GSM_StateMachine *s, GSM_WAPBookmark *bookmark)
{
	GSM_Error 	error;
	int 		count = 4, location;
	unsigned char 	req[600] = {N6110_FRAME_HEADER, 0x09};

	/* We have to enable WAP frames in phone */
	error=DCT3DCT4_EnableWAPFunctions(s);
	if (error!=ERR_NONE) return error;

	location = bookmark->Location - 1;
	if (bookmark->Location == 0) location = 0xffff;
	req[count++] = (location & 0xff00) >> 8;
	req[count++] = location & 0x00ff;

	count += NOKIA_SetUnicodeString(s, req+count, bookmark->Title,   false);
	count += NOKIA_SetUnicodeString(s, req+count, bookmark->Address, false);

	/* unknown */
	req[count++] = 0x01; req[count++] = 0x80; req[count++] = 0x00;
	req[count++] = 0x00; req[count++] = 0x00; req[count++] = 0x00;
	req[count++] = 0x00; req[count++] = 0x00; req[count++] = 0x00;

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

GSM_Error DCT3_ReplyGetWAPSettings(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int 			tmp,Number;
/* 	int			tmp2; */
	GSM_Phone_Data		*Data = &s->Phone.Data;
#ifdef GSM_ENABLE_NOKIA6110
	GSM_Phone_N6110Data 	*Priv6110 = &s->Phone.Data.Priv.N6110;
#endif
#ifdef GSM_ENABLE_NOKIA7110
	GSM_Phone_N7110Data 	*Priv7110 = &s->Phone.Data.Priv.N7110;
#endif

	switch(msg.Buffer[3]) {
	case 0x16:
		smprintf(s, "WAP settings part 1 received OK\n");

		tmp = 4;

		NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[0].Title,false);
		smprintf(s, "Title: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].Title));

		NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[0].HomePage,false);
		smprintf(s, "Homepage: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[0].HomePage));
#ifdef DEBUG
		smprintf(s, "Connection type: ");
		switch (msg.Buffer[tmp]) {
			case 0x00: smprintf(s, "temporary\n"); 	break;
			case 0x01: smprintf(s, "continuous\n"); break;
			default:   smprintf(s, "unknown\n");
		}
		smprintf(s, "Connection security: ");
		switch (msg.Buffer[tmp+13]) {
			case 0x00: smprintf(s, "off\n");	break;
			case 0x01: smprintf(s, "on\n");		break;
			default:   smprintf(s, "unknown\n");
		}
#endif
		Data->WAPSettings->Settings[0].IsContinuous = false;
		if (msg.Buffer[tmp] == 0x01) Data->WAPSettings->Settings[0].IsContinuous = true;
		Data->WAPSettings->Settings[0].IsSecurity = false;
		if (msg.Buffer[tmp+13] == 0x01) Data->WAPSettings->Settings[0].IsSecurity = true;

		/* I'm not sure here. Experimental values from 6210 5.56 */
/* 		tmp2 = DecodeUnicodeLength(Data->WAPSettings->Settings[0].Title); */
/* 		if (tmp2 != 0) tmp2 --; */
/* 		tmp2 += tmp; */
		if (!(UnicodeLength(Data->WAPSettings->Settings[0].Title)) % 2) tmp++;
		if (UnicodeLength(Data->WAPSettings->Settings[0].HomePage)!=0) tmp++;

		smprintf(s, "ID for writing %i\n",msg.Buffer[tmp+5]);

		smprintf(s, "Current set location in phone %i\n",msg.Buffer[tmp+6]);

		smprintf(s, "1 location %i\n",msg.Buffer[tmp+8]);
		smprintf(s, "2 location %i\n",msg.Buffer[tmp+9]);
		smprintf(s, "3 location %i\n",msg.Buffer[tmp+10]);
		smprintf(s, "4 location %i\n",msg.Buffer[tmp+11]);
#ifdef GSM_ENABLE_NOKIA7110
		if (strstr(N7110Phone.models, Data->ModelInfo->model) != NULL) {
			Priv7110->WAPLocations.ID 		= msg.Buffer[tmp+5];
			Priv7110->WAPLocations.CurrentLocation	= msg.Buffer[tmp+6];
			Priv7110->WAPLocations.Locations[0] 	= msg.Buffer[tmp+8];
			Priv7110->WAPLocations.Locations[1] 	= msg.Buffer[tmp+9];
			Priv7110->WAPLocations.Locations[2] 	= msg.Buffer[tmp+10];
			Priv7110->WAPLocations.Locations[3] 	= msg.Buffer[tmp+11];

/* 			Priv7110->WAPLocations.CurrentLocation	= msg.Buffer[tmp2+1]; */
/* 			Priv7110->WAPLocations.Locations[0] 	= msg.Buffer[tmp2+3]; */
/* 			Priv7110->WAPLocations.Locations[1] 	= msg.Buffer[tmp2+4]; */
/* 			Priv7110->WAPLocations.Locations[2] 	= msg.Buffer[tmp2+5]; */
/* 			Priv7110->WAPLocations.Locations[3] 	= msg.Buffer[tmp2+6]; */
		}
#endif
#ifdef GSM_ENABLE_NOKIA6110
		if (strstr(N6110Phone.models, Data->ModelInfo->model) != NULL) {
			Priv6110->WAPLocations.ID 		= msg.Buffer[tmp+5];
			Priv6110->WAPLocations.CurrentLocation	= msg.Buffer[tmp+6];
			Priv6110->WAPLocations.Locations[0] 	= msg.Buffer[tmp+8];
			Priv6110->WAPLocations.Locations[1] 	= msg.Buffer[tmp+9];
			Priv6110->WAPLocations.Locations[2] 	= msg.Buffer[tmp+10];
			Priv6110->WAPLocations.Locations[3] 	= msg.Buffer[tmp+11];
		}
#endif
		return ERR_NONE;
	case 0x17:
		smprintf(s, "WAP settings part 1 receiving error\n");
		switch (msg.Buffer[4]) {
		case 0x01:
			smprintf(s, "Security error. Inside WAP settings menu\n");
			return ERR_INSIDEPHONEMENU;
		case 0x02:
			smprintf(s, "Invalid or empty\n");
			return ERR_INVALIDLOCATION;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
			return ERR_UNKNOWNRESPONSE;
		}
	case 0x1c:
		smprintf(s, "WAP settings part 2 received OK\n");
		Number = Data->WAPSettings->Number;
		switch (msg.Buffer[5]) {
		case 0x00:
			Data->WAPSettings->Settings[Number].Bearer = WAPSETTINGS_BEARER_SMS;
			smprintf(s, "Settings for SMS bearer:\n");
			tmp = 6;

			NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[Number].Service,false);
			smprintf(s, "Service number: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].Service));

			NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[Number].Server,false);
			smprintf(s, "Server number: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].Server));
			break;
		case 0x01:
			Data->WAPSettings->Settings[Number].Bearer = WAPSETTINGS_BEARER_DATA;
			smprintf(s, "Settings for data bearer:\n");
			Data->WAPSettings->Settings[Number].ManualLogin = false;
			tmp = 10;

			NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[Number].IPAddress,false);
			smprintf(s, "IP address: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].IPAddress));

			NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[Number].DialUp,false);
			smprintf(s, "Dial-up number: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].DialUp));

			NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[Number].User,false);
			smprintf(s, "User name: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].User));

			NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[Number].Password,false);
			smprintf(s, "Password: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].Password));
#ifdef DEBUG
			smprintf(s, "Authentication type: ");
			switch (msg.Buffer[6]) {
				case 0x00: smprintf(s, "normal\n");	 break;
				case 0x01: smprintf(s, "secure\n");	 break;
				default:   smprintf(s, "unknown\n"); 	 break;
			}
			smprintf(s, "Data call type: ");
			switch (msg.Buffer[7]) {
				case 0x00: smprintf(s, "analogue\n");	 break;
				case 0x01: smprintf(s, "ISDN\n");	 break;
				default:   smprintf(s, "unknown\n"); 	 break;
			}
			smprintf(s, "Data call speed: ");
			switch (msg.Buffer[9]) {
				case 0x01: smprintf(s, "9600\n");	 break;
				case 0x02: smprintf(s, "14400\n");	 break;
				default:   smprintf(s, "unknown\n"); 	 break;
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
			smprintf(s, "Settings for USSD bearer:\n");
			tmp = 7;
			NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[Number].Service,false);
#ifdef DEBUG
			if (msg.Buffer[6]==0x01)
				smprintf(s, "Service number: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].Service));
			else
				smprintf(s, "IP address: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].Service));
#endif
			Data->WAPSettings->Settings[Number].IsIP=true;
			if (msg.Buffer[6]==0x01) Data->WAPSettings->Settings[Number].IsIP=false;
			NOKIA_GetUnicodeString(s, &tmp, msg.Buffer, Data->WAPSettings->Settings[Number].Code,false);
			smprintf(s, "Service code: \"%s\"\n",DecodeUnicodeString(Data->WAPSettings->Settings[Number].Code));
		}
		Data->WAPSettings->Number++;
		return ERR_NONE;
	case 0x1d:
		smprintf(s, "Incorrect WAP settings location\n");
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
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
	unsigned char 		req[]  = {N6110_FRAME_HEADER,0x15,
					  0x00};		/* Location */
	unsigned char 		req2[] = {N6110_FRAME_HEADER,0x1b,
					  0x00};		/* Location */

	/* We have to enable WAP frames in phone */
	error=DCT3DCT4_EnableWAPFunctions(s);
	if (error!=ERR_NONE) return error;

	s->Phone.Data.WAPSettings = settings;
	settings->Number   = 0;
	settings->ReadOnly = false;

	req[4] = settings->Location-1;
	smprintf(s, "Getting WAP settings part 1\n");
	error = GSM_WaitFor (s, req, 5, 0x3f, 4, ID_GetConnectSet);
	if (error != ERR_NONE) return error;

#ifdef GSM_ENABLE_NOKIA7110
	if (strstr(N7110Phone.models, s->Phone.Data.ModelInfo->model) != NULL) {
		for (i=0;i<4;i++) {
			req2[4] = Priv7110->WAPLocations.Locations[i];
			smprintf(s, "Getting WAP settings part 2\n");
			error=GSM_WaitFor (s, req2, 5, 0x3f, 4, ID_GetConnectSet);
			if (error != ERR_NONE) return error;
			if (Priv7110->WAPLocations.Locations[i] == Priv7110->WAPLocations.CurrentLocation) {
				settings->ActiveBearer = settings->Settings[settings->Number-1].Bearer;
			}
		}
	}
#endif
#ifdef GSM_ENABLE_NOKIA6110
	if (strstr(N6110Phone.models, s->Phone.Data.ModelInfo->model) != NULL) {
		for (i=0;i<4;i++) {
			req2[4] = Priv6110->WAPLocations.Locations[i];
			smprintf(s, "Getting WAP settings part 2\n");
			error=GSM_WaitFor (s, req2, 5, 0x3f, 4, ID_GetConnectSet);
			if (error != ERR_NONE) return error;
			if (Priv6110->WAPLocations.Locations[i] == Priv6110->WAPLocations.CurrentLocation) {
				settings->ActiveBearer = settings->Settings[settings->Number-1].Bearer;
			}
		}
	}
#endif
	if (error == ERR_NONE) {
		for (i=1;i<3;i++) {
			CopyUnicodeString(settings->Settings[i].Title,settings->Settings[0].Title);
			CopyUnicodeString(settings->Settings[i].HomePage,settings->Settings[0].HomePage);
			settings->Settings[i].IsContinuous = settings->Settings[0].IsContinuous;
			settings->Settings[i].IsSecurity   = settings->Settings[0].IsSecurity;

			settings->Settings[i].IsContinuous = settings->Settings[0].IsContinuous;
			settings->Settings[i].IsSecurity   = settings->Settings[0].IsSecurity;
		}
		error = DCT3DCT4_GetActiveConnectSet(s);
	}
	if (error != ERR_NONE) return error;

	settings->Proxy[0]   = 0x00;
	settings->Proxy[1]   = 0x00;
	settings->ProxyPort  = 8080;

	settings->Proxy2[0]  = 0x00;
	settings->Proxy2[1]  = 0x00;
	settings->Proxy2Port = 8080;

	return DCT3DCT4_DisableConnectionFunctions(s);
}

GSM_Error DCT3_ReplySetWAPSettings(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch(msg.Buffer[3]) {
	case 0x19:
		smprintf(s, "WAP settings part 1 set OK\n");
		return ERR_NONE;
	case 0x1a:
		smprintf(s, "WAP settings part 1 setting error\n");
		switch (msg.Buffer[4]) {
		case 0x01:
			smprintf(s, "Security error. Inside WAP settings menu\n");
			return ERR_INSIDEPHONEMENU;
		case 0x02:
			smprintf(s, "Incorrect data\n");
			return ERR_UNKNOWN;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
			return ERR_UNKNOWNRESPONSE;
		}
	case 0x1F:
		smprintf(s, "WAP settings part 2 set OK\n");
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
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
	unsigned char 		req[]  = {N6110_FRAME_HEADER,0x15,
					  0x00};	/* Location */
	unsigned char 		req2[] = {N6110_FRAME_HEADER,0x1b,
					  0x00};	/* Location */
	unsigned char SetReq[200]  = {N7110_FRAME_HEADER, 0x18,
				      0x00}; 		/* Location */
	unsigned char SetReq2[200] = {N7110_FRAME_HEADER, 0x1e,
				      0x00}; 		/* Location */

	/* We have to enable WAP frames in phone */
	error=DCT3DCT4_EnableWAPFunctions(s);
	if (error!=ERR_NONE) return error;

	s->Phone.Data.WAPSettings = &settings2;
	settings2.Number 	  = 0;

	req[4] = settings->Location-1;
	smprintf(s, "Getting WAP settings part 1\n");
	error = GSM_WaitFor (s, req, 6, 0x3f, 4, ID_GetConnectSet);
	if (error != ERR_NONE) return error;

#ifdef GSM_ENABLE_NOKIA6110
	if (strstr(N6110Phone.models, s->Phone.Data.ModelInfo->model) != NULL) {
		for (i=0;i<4;i++) locations[i] = Priv6110->WAPLocations.Locations[i];
		ID = Priv6110->WAPLocations.ID;
	}
#endif
#ifdef GSM_ENABLE_NOKIA7110
	if (strstr(N7110Phone.models, s->Phone.Data.ModelInfo->model) != NULL) {
		for (i=0;i<4;i++) locations[i] = Priv7110->WAPLocations.Locations[i];
		ID = Priv7110->WAPLocations.ID;
	}
#endif

	/* Now we get info about supported types by phone and their locations */
	for (i=0;i<4;i++) {
		settings2.Number = 0;
		settings2.Settings[0].Bearer = 0;
		req2[4] = locations[i];
		smprintf(s, "Getting WAP settings part 2\n");
		error=GSM_WaitFor (s, req2, 6, 0x3f, 4, ID_GetConnectSet);
		if (error != ERR_NONE) return error;
		switch (settings2.Settings[0].Bearer) {
			case WAPSETTINGS_BEARER_DATA: phone1 = locations[i]; break;
			case WAPSETTINGS_BEARER_SMS : phone2 = locations[i]; break;
			case WAPSETTINGS_BEARER_USSD: phone3 = locations[i]; break;
			default			    : break;
		}
		if (error != ERR_NONE) return error;
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
		pos += NOKIA_SetUnicodeString(s, SetReq + pos, settings->Settings[loc1].Title, false);
		/* HomePage */
		pos += NOKIA_SetUnicodeString(s, SetReq + pos, settings->Settings[loc1].HomePage, false);
		if (settings->Settings[loc1].IsContinuous) SetReq[pos] = 0x01;
		pos++;
		SetReq[pos++] = ID;

		SetReq[pos]   = phone1; /* bearer */
		switch (settings->ActiveBearer) {
			case WAPSETTINGS_BEARER_DATA:
				if (loc1 != -1) SetReq[pos] = phone1;
				break;
			case WAPSETTINGS_BEARER_SMS:
				if (loc2 != -1) SetReq[pos] = phone2;
				break;
			case WAPSETTINGS_BEARER_USSD:
				if (loc3 != -1) SetReq[pos] = phone3;
				break;
			default: break;
		}
		pos++;

		if (settings->Settings[loc1].IsSecurity) SetReq[pos] = 0x01;
		pos++;
	} else if (loc2 != -1) {
		/* Name */
		pos += NOKIA_SetUnicodeString(s, SetReq + pos, settings->Settings[loc2].Title, false);
		/* HomePage */
		pos += NOKIA_SetUnicodeString(s, SetReq + pos, settings->Settings[loc2].HomePage, false);
		if (settings->Settings[loc2].IsContinuous) SetReq[pos] = 0x01;
		pos++;
		SetReq[pos++] = ID;

		SetReq[pos]   = phone2; /* bearer */
		switch (settings->ActiveBearer) {
			case WAPSETTINGS_BEARER_DATA:
				if (loc1 != -1) SetReq[pos] = phone1;
				break;
			case WAPSETTINGS_BEARER_SMS:
				if (loc2 != -1) SetReq[pos] = phone2;
				break;
			case WAPSETTINGS_BEARER_USSD:
				if (loc3 != -1) SetReq[pos] = phone3;
				break;
			default: break;
		}
		pos++;

		if (settings->Settings[loc2].IsSecurity) SetReq[pos] = 0x01;
		pos++;
	} else if (loc3 != -1) {
		/* Name */
		pos += NOKIA_SetUnicodeString(s, SetReq + pos, settings->Settings[loc3].Title, false);
		/* HomePage */
		pos += NOKIA_SetUnicodeString(s, SetReq + pos, settings->Settings[loc3].HomePage, false);
		if (settings->Settings[loc3].IsContinuous) SetReq[pos] = 0x01;
		pos++;
		SetReq[pos++] = ID;

		SetReq[pos]   = phone3; /* bearer */
		switch (settings->ActiveBearer) {
			case WAPSETTINGS_BEARER_DATA:
				if (loc1 != -1) SetReq[pos] = phone1;
				break;
			case WAPSETTINGS_BEARER_SMS:
				if (loc2 != -1) SetReq[pos] = phone2;
				break;
			case WAPSETTINGS_BEARER_USSD:
				if (loc3 != -1) SetReq[pos] = phone3;
				break;
			default: break;
		}
		pos++;

		if (settings->Settings[loc3].IsSecurity) SetReq[pos] = 0x01;
		pos++;
	} else {
		return ERR_UNKNOWN;	/* We have to have write something known */
	}
	memcpy(SetReq + pos, "\x01\x80\x00\x00\x00\x00\x00\x00\x00", 9);
	pos += 9;

	smprintf(s, "Writing WAP settings part 1\n");
	error=GSM_WaitFor (s, SetReq, pos, 0x3f, 4, ID_SetConnectSet);
	if (error != ERR_NONE) return error;

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
			pos += NOKIA_SetUnicodeString(s, SetReq2 + pos, settings->Settings[loc1].IPAddress, false);
			/* Number */
			pos += NOKIA_SetUnicodeString(s, SetReq2 + pos, settings->Settings[loc1].DialUp, false);
			/* Username  */
			pos += NOKIA_SetUnicodeString(s, SetReq2 + pos, settings->Settings[loc1].User, false);
			/* Password */
			pos += NOKIA_SetUnicodeString(s, SetReq2 + pos, settings->Settings[loc1].Password, false);
		} else pos+=5;
		memcpy(SetReq2 + pos, "\x80\x00\x00\x00\x00\x00\x00\x00", 8);
		pos += 8;
		smprintf(s, "Writing WAP settings part 2 (Data bearer)\n");
		error=GSM_WaitFor (s, SetReq2, pos, 0x3f, 4, ID_SetConnectSet);
		if (error != ERR_NONE) return error;
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
			pos += NOKIA_SetUnicodeString(s, SetReq2 + pos, settings->Settings[loc2].Service, false);
			/* Server number */
			pos += NOKIA_SetUnicodeString(s, SetReq2 + pos, settings->Settings[loc2].Server, false);
		} else pos += 2;
		memcpy(SetReq2 + pos, "\x80\x00\x00\x00\x00\x00\x00\x00", 8);
		pos += 8;
		smprintf(s, "Writing WAP settings part 2 (SMS bearer)\n");
		error=GSM_WaitFor (s, SetReq2, pos, 0x3f, 4, ID_SetConnectSet);
		if (error != ERR_NONE) return error;
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
			pos += NOKIA_SetUnicodeString(s, SetReq2 + pos, settings->Settings[loc3].Service, false);
			/* Code number */
			pos += NOKIA_SetUnicodeString(s, SetReq2 + pos, settings->Settings[loc3].Code, false);
		} else pos+=2;
		memcpy(SetReq2 + pos, "\x80\x00\x00\x00\x00\x00\x00\x00", 8);
		pos += 8;
		smprintf(s, "Writing WAP settings part 2 (USSD bearer)\n");
		error=GSM_WaitFor (s, SetReq2, pos, 0x3f, 4, ID_SetConnectSet);
		if (error != ERR_NONE) return error;
	}
	error = DCT3DCT4_SetActiveConnectSet(s, settings);
	if (error != ERR_NONE) return error;

	return DCT3DCT4_DisableConnectionFunctions(s);
}

GSM_Error DCT3_ReplySendSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
	case 0x02:
		smprintf(s, "SMS sent OK\n");
		if (s->User.SendSMSStatus!=NULL) s->User.SendSMSStatus(s,0,msg.Buffer[5]);
		return ERR_NONE;
	case 0x03:
		smprintf(s, "Error %i\n",msg.Buffer[6]);
		if (s->User.SendSMSStatus!=NULL) s->User.SendSMSStatus(s,msg.Buffer[6],-1);
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error DCT3_SendSMSMessage(GSM_StateMachine *s, GSM_SMSMessage *sms)
{
	int			length;
	GSM_Error		error;
	unsigned char 		req[256] = {N6110_FRAME_HEADER, 0x01, 0x02, 0x00};

	error=PHONE_EncodeSMSFrame(s,sms,req+6,PHONE_SMSSubmit,&length, true);
	if (error != ERR_NONE) return error;

	smprintf(s, "Sending sms\n");
	return s->Protocol.Functions->WriteMessage(s, req, 6+length, 0x02);
}

GSM_Error DCT3_ReplyNetmonitor(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
	case 0x00:
		smprintf(s, "Netmonitor correctly set\n");
		break;
	default:
		smprintf(s, "Menu %i\n",msg.Buffer[3]);
		smprintf(s, "%s\n",msg.Buffer+4);
		strcpy(s->Phone.Data.Netmonitor,msg.Buffer+4);
		break;
	}
	return ERR_NONE;
}

GSM_Error DCT3_Netmonitor(GSM_StateMachine *s, int testnumber, char *value)
{
	GSM_Error 	error;
	unsigned char 	req[] = {0x00, 0x01, 0x7e,
				 0x00};		/* Test number */

	value[0] = 0;

	error=DCT3_EnableSecurity (s, 0x01);
	if (error != ERR_NONE) return error;

	req[3] = testnumber;

	smprintf(s, "Getting netmonitor test\n");
	s->Phone.Data.Netmonitor = value;
	return GSM_WaitFor (s, req, 4, 0x40, 4, ID_Netmonitor);
}

GSM_Error DCT3_GetManufactureMonth(GSM_StateMachine *s, char *value)
{
	GSM_Error error;

	error=DCT3_EnableSecurity (s, 0x01);
	if (error != ERR_NONE) return error;
	return NOKIA_GetPhoneString(s,"\x00\x01\xCC\x02",4,0x40,value,ID_GetManufactureMonth,5);
}

GSM_Error DCT3_GetProductCode(GSM_StateMachine *s, char *value)
{
	GSM_Error error;

	if (strlen(s->Phone.Data.ProductCodeCache)!=0) {
		strcpy(value,s->Phone.Data.ProductCodeCache);
		return ERR_NONE;
	}

	error=DCT3_EnableSecurity (s, 0x01);
	if (error != ERR_NONE) return error;
	return NOKIA_GetPhoneString(s,"\x00\x01\xCA\x01",4,0x40,value,ID_GetProductCode,5);
}

GSM_Error DCT3_GetOriginalIMEI(GSM_StateMachine *s, char *value)
{
	GSM_Error error;

	error=DCT3_EnableSecurity (s, 0x01);
	if (error != ERR_NONE) return error;
	return NOKIA_GetPhoneString(s,"\x00\x01\xCC\x01",4,0x40,value,ID_GetOriginalIMEI,5);
}

GSM_Error DCT3_GetHardware(GSM_StateMachine *s, char *value)
{
	GSM_Error error;

	if (strlen(s->Phone.Data.HardwareCache)!=0) {
		strcpy(value,s->Phone.Data.HardwareCache);
		return ERR_NONE;
	}

	error=DCT3_EnableSecurity (s, 0x01);
	if (error != ERR_NONE) return error;
	return NOKIA_GetPhoneString(s,"\x00\x01\xC8\x05",4,0x40,value,ID_GetHardware,5);
}

GSM_Error DCT3_GetPPM(GSM_StateMachine *s, char *value)
{
	GSM_Error error;

	error=DCT3_EnableSecurity (s, 0x01);
	if (error != ERR_NONE) return error;
	return NOKIA_GetPhoneString(s,"\x00\x01\xC8\x12",4,0x40,value,ID_GetPPM,5);
}

GSM_Error DCT3_GetSMSStatus(GSM_StateMachine *s, GSM_SMSMemoryStatus *status)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x36, 0x64};

	s->Phone.Data.SMSStatus=status;
	smprintf(s, "Getting SMS status\n");
	return GSM_WaitFor (s, req, 5, 0x14, 4, ID_GetSMSStatus);

	/* 6210 family doesn't show in frame with SMS status info
         * about Templates. We get separately info about this SMS folder.
	 */
}

GSM_Error DCT3_ReplyDeleteSMSMessage(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch(msg.Buffer[3]) {
	case 0x0b:
		smprintf(s, "SMS deleted\n");
		return ERR_NONE;
	case 0x0c:
		smprintf(s, "Error deleting SMS\n");
		switch (msg.Buffer[4]) {
		case 0x00:
			/* Not tested on 6210 */
			smprintf(s, "Unknown meaning, SMS seems to be deleted\n");
			return ERR_NONE;
		case 0x02:
			/* Not tested on 6210 */
			smprintf(s, "Invalid location\n");
			return ERR_INVALIDLOCATION;
		case 0x06:
			/* Not tested on 6210 */
			smprintf(s, "Phone is OFF\n");
			return ERR_PHONEOFF;
		default:
			smprintf(s, "Unknown error: %02x\n",msg.Buffer[4]);
			return ERR_UNKNOWNRESPONSE;
		}
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error N71_92_ReplyGetSignalQuality(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Network level received: %i\n",msg.Buffer[4]);
   	Data->SignalQuality->SignalStrength 	= -1;
    	Data->SignalQuality->SignalPercent 	= ((int)msg.Buffer[4]);
    	Data->SignalQuality->BitErrorRate 	= -1;
	return ERR_NONE;
}

GSM_Error N71_92_GetSignalQuality(GSM_StateMachine *s, GSM_SignalQuality *sig)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x81};

	s->Phone.Data.SignalQuality = sig;
	smprintf(s, "Getting network level\n");
	return GSM_WaitFor (s, req, 4, 0x0a, 4, ID_GetSignalQuality);
}

GSM_Error N71_92_ReplyGetBatteryCharge(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Data *Data = &s->Phone.Data;

	smprintf(s, "Battery level received: %i\n",msg.Buffer[5]);
    	Data->BatteryCharge->BatteryPercent 	= ((int)msg.Buffer[5]);
    	Data->BatteryCharge->ChargeState 	= 0;
	return ERR_NONE;
}

GSM_Error N71_92_GetBatteryCharge(GSM_StateMachine *s, GSM_BatteryCharge *bat)
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x02};

	s->Phone.Data.BatteryCharge = bat;
	smprintf(s, "Getting battery level\n");
	return GSM_WaitFor (s, req, 4, 0x17, 4, ID_GetBatteryCharge);
}

GSM_Error N71_92_ReplyPhoneSetting(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	GSM_Phone_Bitmap_Types	BmpType;
	GSM_Phone_Data		*Data = &s->Phone.Data;

	switch (msg.Buffer[4]) {
	case 0x02:
		if (Data->RequestID == ID_GetBitmap || Data->RequestID == ID_EachFrame) {
			smprintf(s, "Welcome note text received\n");
			CopyUnicodeString(Data->Bitmap->Text,msg.Buffer+6);
			smprintf(s, "Text is \"%s\"\n",DecodeUnicodeString(Data->Bitmap->Text));
			return ERR_NONE;
		}
		if (Data->RequestID == ID_SetBitmap || Data->RequestID == ID_EachFrame) {
			smprintf(s, "Startup text set\n");
			return ERR_NONE;
		}
	case 0x15:
		if (Data->RequestID == ID_GetBitmap || Data->RequestID == ID_EachFrame) {
			smprintf(s, "Startup logo received\n");
			BmpType=GSM_Nokia7110StartupLogo;
			if (msg.Buffer[17]==0x60) BmpType=GSM_Nokia6210StartupLogo;
			if (msg.Buffer[17]==0xc0) BmpType=GSM_NokiaStartupLogo;
			PHONE_DecodeBitmap(BmpType, msg.Buffer+22, Data->Bitmap);
			return ERR_NONE;
		}
		if (Data->RequestID == ID_SetBitmap || Data->RequestID == ID_EachFrame) {
			smprintf(s, "Startup logo set\n");
			return ERR_NONE;
		}
	case 0x17:
		if (Data->RequestID == ID_GetBitmap || Data->RequestID == ID_EachFrame) {
			smprintf(s, "Dealer note text received\n");
			CopyUnicodeString(Data->Bitmap->Text,msg.Buffer+6);
			smprintf(s, "Text is \"%s\"\n",DecodeUnicodeString(Data->Bitmap->Text));
			return ERR_NONE;
		}
		if (Data->RequestID == ID_SetBitmap || Data->RequestID == ID_EachFrame) {
			smprintf(s, "Dealer text set\n");
			return ERR_NONE;
		}
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error N71_92_GetPhoneSetting(GSM_StateMachine *s, int Request, int Setting)
{
	unsigned char req[] = {N7110_FRAME_HEADER, 0xee,
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

GSM_Error DCT3_DecodeSMSFrame(GSM_StateMachine *s, GSM_SMSMessage *SMS, unsigned char *buffer)
{
	switch (buffer[12] & 0x03) {
	case 0x00:
		smprintf(s, "SMS type - deliver\n");
		SMS->PDU = SMS_Deliver;
		return GSM_DecodeSMSFrame(SMS,buffer,PHONE_SMSDeliver);
	case 0x01:
		smprintf(s, "SMS type - submit\n");
		SMS->PDU = SMS_Submit;
		return GSM_DecodeSMSFrame(SMS,buffer,PHONE_SMSSubmit);
	case 0x02:
		smprintf(s, "SMS type - delivery report\n");
		SMS->PDU = SMS_Status_Report;
		return GSM_DecodeSMSFrame(SMS,buffer,PHONE_SMSStatusReport);
	}
	return ERR_UNKNOWN;
}

GSM_Error N61_91_ReplySetOpLogo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
	case 0x31:
		smprintf(s, "Operator logo set OK\n");
		return ERR_NONE;
	case 0x32:
		smprintf(s, "Error setting operator logo\n");
		switch (msg.Buffer[4]) {
		case 0x7d:
			smprintf(s, "Too high location ?\n");
			return ERR_INVALIDLOCATION;
		default:
			smprintf(s, "ERROR: unknown %i\n",msg.Buffer[4]);
		}
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error N61_71_ReplyResetPhoneSettings(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s)
{
	smprintf(s, "Phone settings cleaned OK\n");
	return ERR_NONE;
}

GSM_Error N61_71_ResetPhoneSettings(GSM_StateMachine *s, GSM_ResetSettingsType Type)
{
	GSM_Error	error;
	unsigned char 	req[] = {0x00, 0x01, 0x65,
				 0x01};			/* Reset type */

	switch (Type) {
	case GSM_RESET_PHONESETTINGS			: req[3] = 0x01; break;
	case GSM_RESET_DEVICE				: req[3] = 0x02; break;
	case GSM_RESET_USERINTERFACE			: req[3] = 0x08; break;
	case GSM_RESET_USERINTERFACE_PHONESETTINGS	: req[3] = 0x38; break;
	case GSM_RESET_FULLFACTORY			: req[3] = 0xff; break;
	}

	error=DCT3_EnableSecurity (s, 0x01);
	if (error != ERR_NONE) return error;

	return GSM_WaitFor (s, req, 4, 0x40, 4, ID_ResetPhoneSettings);
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
