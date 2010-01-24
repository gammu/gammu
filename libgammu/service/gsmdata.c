/* (c) 2002-2004 by Marcin Wiacek */

#include <string.h>
#include <stdio.h>

#include <gammu-debug.h>
#include <gammu-datetime.h>

#include "gsmdata.h"
#include "../misc/coding/coding.h"
#include "../debug.h"

/* http://forum.nokia.com: OTA MMS Settings 1.0, OTA Settings 7.0 */
static void AddWAPSMSParameterText(unsigned char *Buffer, size_t *Length, unsigned char ID, char *Text, int Len)
{
	int i;

	Buffer[(*Length)++] = 0x87; 			/* PARM with attributes */
	Buffer[(*Length)++] = ID;
	Buffer[(*Length)++] = 0x11; 			/* VALUE */
	Buffer[(*Length)++] = 0x03; 			/* Inline string */
	for (i=0;i<Len;i++) {
		Buffer[(*Length)++] = Text[i];		/* Text */
	}
	Buffer[(*Length)++] = 0x00; 			/* END Inline string */
	Buffer[(*Length)++] = 0x01; 			/* END PARMeter */
}

/* http://forum.nokia.com: OTA MMS Settings 1.0, OTA Settings 7.0 */
static void AddWAPSMSParameterInt(unsigned char *Buffer, size_t *Length, unsigned char ID, unsigned char Value)
{
	Buffer[(*Length)++] = 0x87; 			/* PARM with attributes */
	Buffer[(*Length)++] = ID;
	Buffer[(*Length)++] = Value;
	Buffer[(*Length)++] = 0x01; 			/* END PARMeter */
}

/* http://forum.nokia.com  : OTA MMS Settings 1.0, OTA Settings 7.0
 * http://www.wapforum.org : Wireless Datagram Protocol
 */
void NOKIA_EncodeWAPMMSSettingsSMSText(unsigned char *Buffer, size_t *Length, GSM_WAPSettings *settings, gboolean MMS)
{
	int 		i;
	unsigned char 	buffer[400];

	Buffer[(*Length)++] = 0x01; 			/* Push ID */
	Buffer[(*Length)++] = 0x06; 			/* PDU Type (push) */
	Buffer[(*Length)++] = 0x2C; 			/* Headers length (content type + headers) */
	strcpy(Buffer+(*Length),"\x1F\x2A");
	(*Length)=(*Length)+2;				/* Value length */
	strcpy(Buffer+(*Length),"application/x-wap-prov.browser-settings");
	(*Length)=(*Length)+39;				/* MIME-Type */
	Buffer[(*Length)++] = 0x00; 			/* end inline string */
	strcpy(Buffer+(*Length),"\x81\xEA");
	(*Length)=(*Length)+2;				/* charset UTF-8 short int. */
	strcpy(Buffer+(*Length),"\x01\x01");
	(*Length)=(*Length)+2;				/* version WBXML 1.1 */
	Buffer[(*Length)++] = 0x6A; 			/* charset UTF-8 */
	Buffer[(*Length)++] = 0x00; 			/* string table length */

	Buffer[(*Length)++] = 0x45; 			/* CHARACTERISTIC-LIST with content */
		Buffer[(*Length)++] = 0xC6; 		/* CHARACTERISTIC with content and attributes */
		Buffer[(*Length)++] = 0x06; 		/* TYPE=ADDRESS */
		Buffer[(*Length)++] = 0x01; 		/* END PARMeter */
			switch (settings->Bearer) {
			case WAPSETTINGS_BEARER_GPRS:
				/* Bearer */
				AddWAPSMSParameterInt(Buffer, Length, 0x12, 0x49);
				/* PPP_LOGINTYPE (manual login or not) */
				if (settings->ManualLogin) {
					AddWAPSMSParameterInt(Buffer, Length, 0x1D, 0x65);
				} else {
					AddWAPSMSParameterInt(Buffer, Length, 0x1D, 0x64);
				}
				/* PPP_AUTHTYPE*/
				if (settings->IsNormalAuthentication) {
					/* OTA_CSD_AUTHTYPE_PAP */
					AddWAPSMSParameterInt(Buffer, Length, 0x22, 0x70);
				} else {
					/* OTA_CSD_AUTHTYPE_CHAP */
					AddWAPSMSParameterInt(Buffer, Length, 0x22, 0x71);
				}
				/* GPRS_ACCESSPOINTNAME */
				AddWAPSMSParameterText(Buffer, Length, 0x1C, DecodeUnicodeString(settings->DialUp), UnicodeLength(settings->DialUp));
				/* PROXY */
				AddWAPSMSParameterText(Buffer, Length, 0x13, DecodeUnicodeString(settings->IPAddress), UnicodeLength(settings->IPAddress));
				/* PPP_AUTHNAME (user) */
				AddWAPSMSParameterText(Buffer, Length, 0x23, DecodeUnicodeString(settings->User), UnicodeLength(settings->User));
				/* PPP_AUTHSECRET (password) */
				AddWAPSMSParameterText(Buffer, Length, 0x24, DecodeUnicodeString(settings->Password), UnicodeLength(settings->Password));
				break;
			case WAPSETTINGS_BEARER_DATA:
				/* Bearer */
				AddWAPSMSParameterInt(Buffer, Length, 0x12, 0x45);
				/* CSD_DIALSTRING */
				AddWAPSMSParameterText(Buffer, Length, 0x21, DecodeUnicodeString(settings->DialUp), UnicodeLength(settings->DialUp));
				/* PROXY */
				AddWAPSMSParameterText(Buffer, Length, 0x13, DecodeUnicodeString(settings->IPAddress), UnicodeLength(settings->IPAddress));
				/* PPP_LOGINTYPE (manual login or not) */
				if (settings->ManualLogin) {
					AddWAPSMSParameterInt(Buffer, Length, 0x1D, 0x65);
				} else {
					AddWAPSMSParameterInt(Buffer, Length, 0x1D, 0x64);
				}
				/* PPP_AUTHTYPE*/
				if (settings->IsNormalAuthentication) {
					/* OTA_CSD_AUTHTYPE_PAP */
					AddWAPSMSParameterInt(Buffer, Length, 0x22, 0x70);
				} else {
					/* OTA_CSD_AUTHTYPE_CHAP */
					AddWAPSMSParameterInt(Buffer, Length, 0x22, 0x71);
				}
				/* CSD_CALLTYPE (type of call) */
				if (settings->IsISDNCall) {
					/* ISDN */
					AddWAPSMSParameterInt(Buffer, Length, 0x28, 0x73);
				} else {
					/* analogue */
					AddWAPSMSParameterInt(Buffer, Length, 0x28, 0x72);
				}
				/* CSD_CALLSPEED (speed of call) */
				switch (settings->Speed) {
				case WAPSETTINGS_SPEED_AUTO:
					AddWAPSMSParameterInt(Buffer, Length, 0x29, 0x6A);
					break;
				case WAPSETTINGS_SPEED_9600:
					AddWAPSMSParameterInt(Buffer, Length, 0x29, 0x6B);
					break;
				case WAPSETTINGS_SPEED_14400:
					AddWAPSMSParameterInt(Buffer, Length, 0x29, 0x6C);
				}
				/* PPP_AUTHNAME (user) */
				AddWAPSMSParameterText(Buffer, Length, 0x23, DecodeUnicodeString(settings->User), UnicodeLength(settings->User));
				/* PPP_AUTHSECRET (password) */
				AddWAPSMSParameterText(Buffer, Length, 0x24, DecodeUnicodeString(settings->Password), UnicodeLength(settings->Password));
				break;
#ifdef DEVELOP
			case WAPSETTINGS_BEARER_SMS:
				/* Bearer */
				AddWAPSMSParameterInt(Buffer, Length, 0x12, 0x41);
				/* PROXY */
				AddWAPSMSParameterText(Buffer, Length, 0x13, DecodeUnicodeString(settings->Server), UnicodeLength(settings->Server));
				/* SMS_SMSC_ADDRESS */
				/*  ..... */
				break;
			case WAPSETTINGS_BEARER_USSD:
				/* FIXME */
				/* Bearer */
				AddWAPSMSParameterInt(Buffer, Length, 0x12, 0x41);
				/* PROXY */
				AddWAPSMSParameterText(Buffer, Length, 0x13, DecodeUnicodeString(settings->Service), UnicodeLength(settings->Service));
				/* USSD_SERVICE_CODE */
				/* FIXME */
				AddWAPSMSParameterText(Buffer, Length, 0x13, DecodeUnicodeString(settings->Code), UnicodeLength(settings->Code));
#else
			case WAPSETTINGS_BEARER_SMS:
			case WAPSETTINGS_BEARER_USSD:
				break;
#endif
			}
			/* PORT */
			if (settings->IsSecurity) {
				if (settings->IsContinuous) {
					/* Port = 9203. Continuous */
					AddWAPSMSParameterInt(Buffer, Length, 0x14, 0x63);
				} else {
					/* Port = 9202. Temporary */
					AddWAPSMSParameterInt(Buffer, Length, 0x14, 0x62);
				}
			} else {
				if (settings->IsContinuous) {
					/* Port = 9201. Continuous */
					AddWAPSMSParameterInt(Buffer, Length, 0x14, 0x61);
				} else {
					/* Port = 9200. Temporary */
					AddWAPSMSParameterInt(Buffer, Length, 0x14, 0x60);
				}
			}
		Buffer[(*Length)++] = 0x01; 		/* END PARMeter */

		/* URL */
		Buffer[(*Length)++] = 0x86; 		/* CHARACTERISTIC-LIST with attributes */
		if (MMS) {
			Buffer[(*Length)++] = 0x7C; 	/* TYPE = MMSURL */
		} else {
			Buffer[(*Length)++] = 0x07; 	/* TYPE = URL */
		}
		Buffer[(*Length)++] = 0x11; 		/* VALUE */
		Buffer[(*Length)++] = 0x03; 		/* Inline string */
		sprintf(buffer,"%s",DecodeUnicodeString(settings->HomePage));
		for (i=0;i<(int)strlen(buffer);i++) {
			Buffer[(*Length)++] = buffer[i];/* Text */
		}
		Buffer[(*Length)++] = 0x00; 		/* END Inline string */
		Buffer[(*Length)++] = 0x01; 		/* END PARMeter */

		/* ISP_NAME (name) */
		Buffer[(*Length)++] = 0xC6; 		/* CHARACTERISTIC with content and attributes */
		Buffer[(*Length)++] = 0x08; 		/* TYPE=NAME */
		Buffer[(*Length)++] = 0x01; 		/* END PARMeter */
			/* Settings name */
			AddWAPSMSParameterText(Buffer, Length, 0x15, DecodeUnicodeString(settings->Title), UnicodeLength(settings->Title));
		Buffer[(*Length)++] = 0x01; 		/* END PARMeter */
	Buffer[(*Length)++] = 0x01;			/* END PARMeter */
}

/* http://forum.nokia.com: OTA Settings 7.0 */
/* first it used default/ISO coding */
/* Joergen Thomsen changed to UTF8 */
void NOKIA_EncodeWAPBookmarkSMSText(unsigned char *Buffer, size_t *Length, GSM_WAPBookmark *bookmark)
{
	unsigned char	buffer[100];

	Buffer[(*Length)++] = 0x01; 			/* Push ID */
	Buffer[(*Length)++] = 0x06; 			/* PDU Type (push) */
	Buffer[(*Length)++] = 0x2D; 			/* Headers length (content type + headers) */
	strcpy(Buffer+(*Length),"\x1F\x2B");
	(*Length)=(*Length)+2;				/* Value length */
	strcpy(Buffer+(*Length),"application/x-wap-prov.browser-bookmarks");
	(*Length)=(*Length)+40;				/* MIME-Type */
	Buffer[(*Length)++] = 0x00; 			/* end inline string */
	strcpy(Buffer+(*Length),"\x81\xEA");
	(*Length)=(*Length)+2;				/* charset UTF-8 short int. */

	/* added by Joergen Thomsen */
	Buffer[(*Length)++] = 0x01;			/*  Version WBXML 1.1 */
	Buffer[(*Length)++] = 0x01;			/*  Unknown public identifier */
	Buffer[(*Length)++] = 0x6A;			/*  charset UTF-8 */
	Buffer[(*Length)++] = 0x00;			/*  string table length */

	Buffer[(*Length)++] = 0x45; 			/* CHARACTERISTIC-LIST with content */
		/* URL */
		Buffer[(*Length)++] = 0xC6; 		/* CHARACTERISTIC with content and attributes */
		Buffer[(*Length)++] = 0x7F;             /* TYPE = BOOKMARK */
		Buffer[(*Length)++] = 0x01; 		/* END PARMeter */

			/* TITLE */
			EncodeUTF8(buffer, bookmark->Title);
			AddWAPSMSParameterText(Buffer, Length, 0x15, buffer, strlen(buffer));
			/* URL */
			EncodeUTF8(buffer, bookmark->Address);
			AddWAPSMSParameterText(Buffer, Length, 0x17, buffer, strlen(buffer));

		Buffer[(*Length)++] = 0x01;		/* END (CHARACTERISTIC) */
	Buffer[(*Length)++] = 0x01;			/* END (CHARACTERISTIC-LIST) */
}

/**
 * Encodes WAP indicator message.
 *
 * Produced output is WBXML, as defined in WAP-167.
 */
void GSM_EncodeWAPIndicatorSMSText(unsigned char *Buffer, size_t *Length, char *Text, char *URL)
{
	size_t i, len;

	Buffer[(*Length)++] = 0x01; 			/* Push ID */
	Buffer[(*Length)++] = 0x06; 			/* PDU Type (push) */
	Buffer[(*Length)++] = 0x01; 			/* Headers length */
	Buffer[(*Length)++] = 0xAE; 			/* MIME-Type: application/vnd.wap.sic */

	Buffer[(*Length)++] = 0x02; 			/*  WBXML 1.2 */
	Buffer[(*Length)++] = 0x05; 			/*  SI 1.0 Public Identifier */
	Buffer[(*Length)++] = 0x6A;			/*  charset UTF-8 */
	Buffer[(*Length)++] = 0x00;			/*  string table length */
	Buffer[(*Length)++] = 0x45;			/*  SI with content */
		Buffer[(*Length)++] = 0xC6;		/*  indication with content and attributes */
			Buffer[(*Length)++] = 0x0B;	/*  address */
			Buffer[(*Length)++] = 0x03; 	/*  Inline string */
			len = strlen(URL);
			for (i = 0; i < len; i++) {
				Buffer[(*Length)++] = URL[i];/* Text */
			}
			Buffer[(*Length)++] = 0x00; 	/*  END Inline string */

#if 0
			Buffer[(*Length)++] = 0x0A;	/*  created... */
			Buffer[(*Length)++] = 0xC3;	/*  OPAQUE */
			Buffer[(*Length)++] = 0x07;	/*  length */
			Buffer[(*Length)++] = 0x20;	/*  year */
			Buffer[(*Length)++] = 0x08;	/*  year */
			Buffer[(*Length)++] = 0x12;	/*  month */
			Buffer[(*Length)++] = 0x10;	/*  day */
			Buffer[(*Length)++] = 0x00;	/*  hour */
			Buffer[(*Length)++] = 0x00;	/*  minute */
			Buffer[(*Length)++] = 0x00;	/*  second */

			Buffer[(*Length)++] = 0x10;	/*  expires */
			Buffer[(*Length)++] = 0xC3;	/*  OPAQUE */
			Buffer[(*Length)++] = 0x04;	/*  length */
			Buffer[(*Length)++] = 0x20;	/*  year */
			Buffer[(*Length)++] = 0x10;	/*  year */
			Buffer[(*Length)++] = 0x06;	/*  month */
			Buffer[(*Length)++] = 0x25;	/*  day */
#endif

		Buffer[(*Length)++] = 0x01;		/*  END (indication) */
		Buffer[(*Length)++] = 0x03; 		/*  Inline string */
		len = strlen(Text);
		for (i = 0; i < len; i++) {
			Buffer[(*Length)++] = Text[i];	/* Text */
		}
		Buffer[(*Length)++] = 0x00; 		/*  END Inline string */
		Buffer[(*Length)++] = 0x01;		/*  END (indication) */
	Buffer[(*Length)++] = 0x01;			/*  END (SI) */
}

GSM_Error GSM_EncodeURLFile(unsigned char *Buffer, size_t *Length, GSM_WAPBookmark *bookmark)
{
	*Length+=sprintf(Buffer+(*Length), "BEGIN:VBKM%c%c",13,10);
	*Length+=sprintf(Buffer+(*Length), "VERSION:1.0%c%c",13,10);
	*Length+=sprintf(Buffer+(*Length), "TITLE:%s%c%c",DecodeUnicodeString(bookmark->Title),13,10);
	*Length+=sprintf(Buffer+(*Length), "URL:%s%c%c",DecodeUnicodeString(bookmark->Address),13,10);
	*Length+=sprintf(Buffer+(*Length), "BEGIN:ENV%c%c",13,10);
	*Length+=sprintf(Buffer+(*Length), "X-IRMC-URL;QUOTED-PRINTABLE:=%c%c",13,10);
	*Length+=sprintf(Buffer+(*Length), "[InternetShortcut] =%c%c",13,10);
	*Length+=sprintf(Buffer+(*Length), "URL=%s%c%c",DecodeUnicodeString(bookmark->Address),13,10);
	*Length+=sprintf(Buffer+(*Length), "END:ENV%c%c",13,10);
	*Length+=sprintf(Buffer+(*Length), "END:VBKM%c%c",13,10);

	return ERR_NONE;
}

/* -------------------------------- MMS ------------------------------------ */

/* SNIFFS, specs somewhere in http://www.wapforum.org */
void GSM_EncodeMMSIndicatorSMSText(unsigned char *Buffer, size_t *Length, GSM_MMSIndicator Indicator)
{
	unsigned char 	buffer[200];
	int		i;


	/*
	 * WSP header
	 */

	/* Transaction ID (maybe this should be random?) */
	Buffer[(*Length)++] = 0xe6;
	/* PDU type push */
	Buffer[(*Length)++] = 0x06;
	/* Header length */
	Buffer[(*Length)++] = 0x22;
	/* MIME type (header) */
	strcpy(Buffer+(*Length),"application/vnd.wap.mms-message");
	(*Length)=(*Length)+31;
	Buffer[(*Length)++] = 0x00;

	/* WAP push (x-wap-application.ua) */
	Buffer[(*Length)++] = 0xaf;
	Buffer[(*Length)++] = 0x84;

	/*
	 * MMS data
	 */

	/* Transaction type */
	Buffer[(*Length)++] = 0x8c;
	/* m-notification-ind */
	Buffer[(*Length)++] = 0x82;

	/* Transaction ID, usually last part of address */
	Buffer[(*Length)++] = 0x98;
	i = strlen(Indicator.Address);
	while (Indicator.Address[i] != '/' && i!=0) i--;
	strcpy(Buffer+(*Length),Indicator.Address+i+1);
	(*Length)=(*Length)+strlen(Indicator.Address+i+1);
	Buffer[(*Length)++] = 0x00;

	/* MMS version */
	Buffer[(*Length)++] = 0x8d;
	/* 1.2 (0x90 is 1.0) */
	Buffer[(*Length)++] = 0x92;

	if (Indicator.Personal) {
		/* Message class */
		Buffer[(*Length)++] = 0x8a;
		/* Personal (80 = personal, 81 = ad, 82=info, 83 = auto) */
		Buffer[(*Length)++] = 0x80;
	}

	if (Indicator.MessageSize > 0) {
		/* Message size */
		Buffer[(*Length)++] = 0x8e;
		/* Length of size */
		Buffer[(*Length)++] = 0x04;
		Buffer[(*Length)++] = ((long)Indicator.MessageSize >> 24) & 0xff;
		Buffer[(*Length)++] = ((long)Indicator.MessageSize >> 16) & 0xff;
		Buffer[(*Length)++] = ((long)Indicator.MessageSize >>  8) & 0xff;
		Buffer[(*Length)++] = ((long)Indicator.MessageSize >>  0) & 0xff;
	}

	/* Sender address */
	Buffer[(*Length)++] = 0x89;
	sprintf(buffer,"%s/TYPE=PLMN",Indicator.Sender);
	/* Field size */
	Buffer[(*Length)++] = strlen(buffer) + 2;
	/* Sender address is present */
	Buffer[(*Length)++] = 0x80;
	strcpy(Buffer+(*Length),buffer);
	(*Length)=(*Length)+strlen(buffer);
	/* Zero terminate */
	Buffer[(*Length)++] = 0x00;

	/* Subject */
	Buffer[(*Length)++] = 0x96;
	strcpy(Buffer+(*Length),Indicator.Title);
	(*Length)=(*Length)+strlen(Indicator.Title);
	Buffer[(*Length)++] = 0x00;

	strcpy(Buffer+(*Length),"\x8A\x80\x8E\x02\x47\xBB\x88\x05\x81\x03\x02\xA3");
	(*Length)=(*Length)+12;
	Buffer[(*Length)++] = 0x00;

	/* Content location */
	Buffer[(*Length)++] = 0x83;
	strcpy(Buffer+(*Length),Indicator.Address);
	(*Length)=(*Length)+strlen(Indicator.Address);
	Buffer[(*Length)++] = 0x00;
}

GSM_Error GSM_ClearMMSMultiPart(GSM_EncodedMultiPartMMSInfo *info)
{
	int i;

	for (i=0;i<GSM_MAX_MULTI_MMS;i++) {
		if (info->Entries[i].File.Buffer != NULL) {
			free(info->Entries[i].File.Buffer);
			info->Entries[i].File.Buffer=NULL;
		}
	}

	memset(info,0,sizeof(GSM_EncodedMultiPartMMSInfo));

	for (i=0;i<GSM_MAX_MULTI_MMS;i++) {
		info->Entries[i].File.Buffer = NULL;
	}
	info->DateTimeAvailable = FALSE;

	return ERR_NONE;
}

void GSM_AddWAPMIMEType(int type, char *buffer)
{
	char tmpbuf[100];
	switch (type) {
		case  3:
			strcat(buffer, "text/plain");
			break;
		case  6:
			strcat(buffer, "text/x-vCalendar");
			break;
		case  7:
			strcat(buffer, "text/x-vCard");
			break;
		case 29:
			strcat(buffer, "image/gif");
			break;
		case 30:
			strcat(buffer, "image/jpeg");
			break;
		case 35:
			strcat(buffer, "application/vnd.wap.multipart.mixed");
			break;
		case 51:
			strcat(buffer, "application/vnd.wap.multipart.related");
			break;
		default:
			sprintf(tmpbuf, "application/x-%d", type);
			strcat(buffer, tmpbuf);
			break;
	}
}

GSM_Error GSM_DecodeMMSFileToMultiPart(GSM_Debug_Info *di, GSM_File *file, GSM_EncodedMultiPartMMSInfo *info)
{
	size_t pos = 0;
	int type=0,parts,j;
	int		i,len2,len3,value2;
	long 		value;
	time_t 		timet;
	GSM_DateTime 	Date;
	char		buff[200],buff2[200];

	/* header */
	while(1) {
		if (pos > file->Used) break;
		if (!(file->Buffer[pos] & 0x80)) break;
		switch (file->Buffer[pos++] & 0x7F) {
		case 0x01:
			smfprintf(di, "  BCC               : not done yet\n");
			return ERR_FILENOTSUPPORTED;
		case 0x02:
			smfprintf(di, "  CC                : ");
			i = 0;
			while (file->Buffer[pos]!=0x00) {
				buff[i++] = file->Buffer[pos++];
			}
			buff[i] = 0;
			pos++;
			if (strstr(buff,"/TYPE=PLMN")!=NULL) {
				buff[strlen(buff)-10] = 0;
				info->CCType = MMSADDRESS_PHONE;
				smfprintf(di, "phone %s\n",buff);
			} else {
				info->CCType = MMSADDRESS_UNKNOWN;
				smfprintf(di, "%s\n",buff);
			}
			EncodeUnicode(info->CC,buff,strlen(buff));
			break;
		case 0x03:
			smfprintf(di, "  Content location  : not done yet\n");
			return ERR_FILENOTSUPPORTED;
		case 0x04:
			smfprintf(di, "  Content type      : ");
			buff[0] = 0;
			if (file->Buffer[pos] <= 0x1E) {
				len2 = file->Buffer[pos++];
				type = file->Buffer[pos++] & 0x7f;
				GSM_AddWAPMIMEType(type, buff);
				i=0;
				while (i<len2) {
					switch (file->Buffer[pos+i]) {
					case 0x89:
						strcat(buff, "; type=");
						i++;
						while (file->Buffer[pos+i]!=0x00) {
							buff[strlen(buff)+1] = 0;
							buff[strlen(buff)]   = file->Buffer[pos+i];
							i++;
						}
						i++;
						break;
					case 0x8A:
						strcat(buff, "; start=");
						i++;
						while (file->Buffer[pos+i]!=0x00) {
							buff[strlen(buff)+1] = 0;
							buff[strlen(buff)]   = file->Buffer[pos+i];
							i++;
						}
						i++;
						break;
					default:
						i++;
						break;
					}
				}
				pos+=len2-1;
			} else if (file->Buffer[pos] == 0x1F) {
				/* hack from coded files */
				len2 = file->Buffer[pos++];
				type = file->Buffer[pos++] & 0x7f;
				type +=2;
				GSM_AddWAPMIMEType(type, buff);
				i=0;
				while (i<len2) {
					switch (file->Buffer[pos+i]) {
					case 0x89:
						strcat(buff, "; type=");
						i++;
						while (file->Buffer[pos+i]!=0x00) {
							buff[strlen(buff)+1] = 0;
							buff[strlen(buff)]   = file->Buffer[pos+i];
							i++;
						}
						i++;
						break;
					case 0x8A:
						strcat(buff, "; start=");
						i++;
						while (file->Buffer[pos+i]!=0x00) {
							buff[strlen(buff)+1] = 0;
							buff[strlen(buff)]   = file->Buffer[pos+i];
							i++;
						}
						i++;
						break;
					default:
						i++;
						break;
					}
				}
				pos+=len2+2;
			} else if (file->Buffer[pos] >= 0x20 && file->Buffer[pos] <= 0x7F) {
				smfprintf(di, "not done yet 2\n");
				return ERR_FILENOTSUPPORTED;
			} else if (file->Buffer[pos] >= 0x80 && file->Buffer[pos] < 0xFF) {
				type = file->Buffer[pos++] & 0x7f;
				GSM_AddWAPMIMEType(type, buff);
			}
			smfprintf(di, "%s\n",buff);
			EncodeUnicode(info->ContentType,buff,strlen(buff));
			break;
		case 0x05:
			smfprintf(di, "  Date              : ");
			value=0;
			len2 = file->Buffer[pos++];
			for (i=0;i<len2;i++) {
				value=value<<8;
				value |= file->Buffer[pos++];
			}
			timet = value;
			Fill_GSM_DateTime(&Date, timet);
			smfprintf(di, "%s\n",OSDateTime(Date,0));
			info->DateTimeAvailable = TRUE;
			memcpy(&info->DateTime,&Date,sizeof(GSM_DateTime));
			break;
		case 0x06:
			smfprintf(di, "  Delivery report   : ");
			info->MMSReportAvailable = TRUE;
			switch(file->Buffer[pos++]) {
				case 0x80:
					smfprintf(di, "yes\n");
					info->MMSReport = TRUE;
					break;
				case 0x81:
					smfprintf(di, "no\n");
					info->MMSReport = FALSE;
					break;
				default:
					smfprintf(di, "unknown\n");
					return ERR_FILENOTSUPPORTED;
			}
			break;
		case 0x08:
			smfprintf(di, "  Expiry            : ");
			pos++; /* length? */
			switch (file->Buffer[pos]) {
				case 0x80: smfprintf(di, "date - ignored\n");	 	 break;
				case 0x81: smfprintf(di, "seconds - ignored\n");	 break;
				default  : smfprintf(di, "unknown %02x\n",file->Buffer[pos]);	 break;
			}
			pos++;
			pos++; /* expiry */
			pos++; /* expiry */
			pos++; /* expiry */
			pos++; /* expiry */
			break;
		case 0x09:
			pos++;
			pos++;
			if (file->Buffer[pos-1] == 128) {
				smfprintf(di, "  From              : ");
				len2=file->Buffer[pos-2]-1;
				for (i=0;i<len2;i++) {
					buff[i] = file->Buffer[pos++];
				}
				buff[i] = 0;
				if (strstr(buff,"/TYPE=PLMN")!=NULL) {
					buff[strlen(buff)-10] = 0;
					info->SourceType = MMSADDRESS_PHONE;
					smfprintf(di, "phone %s\n",buff);
				} else {
					info->SourceType = MMSADDRESS_UNKNOWN;
					smfprintf(di, "%s\n",buff);
				}
				EncodeUnicode(info->Source,buff,strlen(buff));
			}
			break;
		case 0x0A:
			smfprintf(di, "  Message class     : ");
			switch (file->Buffer[pos++]) {
				case 0x80: smfprintf(di, "personal\n");	 break;
				case 0x81: smfprintf(di, "advertisment\n");	 break;
				case 0x82: smfprintf(di, "informational\n"); break;
				case 0x83: smfprintf(di, "auto\n");		 break;
				default  : smfprintf(di, "unknown\n");	 break;
			}
			break;
		case 0x0B:
			smfprintf(di, "  Message ID        : ");
			while (file->Buffer[pos]!=0x00) {
				smfprintf(di, "%c",file->Buffer[pos]);
				pos++;
			}
			smfprintf(di, "\n");
			pos++;
			break;
		case 0x0C:
			smfprintf(di, "  Message type      : ");
			switch (file->Buffer[pos++]) {
				case 0x80: sprintf(info->MSGType,"m-send-req");  	   	break;
				case 0x81: sprintf(info->MSGType,"m-send-conf"); 	   	break;
				case 0x82: sprintf(info->MSGType,"m-notification-ind"); 	break;
				case 0x83: sprintf(info->MSGType,"m-notifyresp-ind");   	break;
				case 0x84: sprintf(info->MSGType,"m-retrieve-conf");		break;
				case 0x85: sprintf(info->MSGType,"m-acknowledge-ind");  	break;
				case 0x86: sprintf(info->MSGType,"m-delivery-ind");		break;
				default  : smfprintf(di, "unknown\n"); 	   			return ERR_FILENOTSUPPORTED;
			}
			smfprintf(di, "%s\n",info->MSGType);
			break;
		case 0x0D:
			value2 = file->Buffer[pos] & 0x7F;
			smfprintf(di, "  MMS version       : %i.%i\n", (value2 & 0x70) >> 4, value2 & 0x0f);
			pos++;
			break;
		case 0x0E:
			smfprintf(di, "  Message size      : ignored for now\n");
			pos += 2;
			break;
		case 0x0F:
			smfprintf(di, "  Priority          : ");
			switch (file->Buffer[pos++]) {
				case 0x80: smfprintf(di, "low\n");		break;
				case 0x81: smfprintf(di, "normal\n");	break;
				case 0x82: smfprintf(di, "high\n");		break;
				default  : smfprintf(di, "unknown\n");	break;
			}
			break;
		case 0x10:
			smfprintf(di, "  Read reply        : ");
			switch(file->Buffer[pos++]) {
				case 0x80: smfprintf(di, "yes\n"); 		break;
				case 0x81: smfprintf(di, "no\n");  		break;
				default  : smfprintf(di, "unknown\n");
			}
			break;
		case 0x11:
			smfprintf(di, "  Report allowed    : not done yet\n");
			return ERR_FILENOTSUPPORTED;
		case 0x12:
			smfprintf(di, "  Response status   : not done yet\n");
			return ERR_FILENOTSUPPORTED;
		case 0x13:
			smfprintf(di, "  Response text     : not done yet\n");
			return ERR_FILENOTSUPPORTED;
		case 0x14:
			smfprintf(di, "  Sender visibility : not done yet\n");
			return ERR_FILENOTSUPPORTED;
		case 0x15:
			smfprintf(di, "  Status            : ");
			switch (file->Buffer[pos++]) {
				case 0x80: smfprintf(di, "expired\n");	break;
				case 0x81: smfprintf(di, "retrieved\n");	break;
				case 0x82: smfprintf(di, "rejected\n");	break;
				case 0x83: smfprintf(di, "deferred\n");	break;
				case 0x84: smfprintf(di, "unrecognized\n");	break;
				default  : smfprintf(di, "unknown\n");
			}
			pos++;
			pos++;
			break;
		case 0x16:
			smfprintf(di, "  Subject           : ");
			if (file->Buffer[pos+1]==0xEA) {
				pos+=2;
			}
			i = 0;
			while (file->Buffer[pos]!=0x00) {
				buff[i++] = file->Buffer[pos++];
			}
			buff[i] = 0;
			smfprintf(di, "%s\n",buff);
			EncodeUnicode(info->Subject,buff,strlen(buff));
			pos++;
			break;
		case 0x17:
			smfprintf(di, "  To                : ");
			i = 0;
			while (file->Buffer[pos]!=0x00) {
				buff[i++] = file->Buffer[pos++];
			}
			buff[i] = 0;
			if (strstr(buff,"/TYPE=PLMN")!=NULL) {
				buff[strlen(buff)-10] = 0;
				info->DestinationType = MMSADDRESS_PHONE;
				smfprintf(di, "phone %s\n",buff);
			} else {
				info->DestinationType = MMSADDRESS_UNKNOWN;
				smfprintf(di, "%s\n",buff);
			}
			EncodeUnicode(info->Destination,buff,strlen(buff));
			pos++;
			break;
		case 0x18:
			smfprintf(di, "  Transaction ID    : ");
			while (file->Buffer[pos]!=0x00) {
				smfprintf(di, "%c",file->Buffer[pos]);
				pos++;
			}
			smfprintf(di, "\n");
			pos++;
			break;
		default:
			smfprintf(di, "  unknown1\n");
			break;
		}
	}

	/* if we don't have any parts, we exit */
	if (type != 35 && type != 51) return ERR_NONE;

	value = 0;
	while (TRUE) {
		value = value << 7;
		value |= file->Buffer[pos] & 0x7F;
		pos++;
		if (!(file->Buffer[pos-1] & 0x80)) break;
	}
	value2 = value;
	smfprintf(di, "  Parts             : %i\n",value2);
	parts = value;

	for (j=0;j<parts;j++) {
		value = 0;
		while (TRUE) {
			value = value << 7;
			value |= file->Buffer[pos] & 0x7F;
			pos++;
			if (!(file->Buffer[pos-1] & 0x80)) break;
		}
		smfprintf(di, "    Header len: %li",value);
		len2 = value;

		value = 0;
		while (TRUE) {
			value = value << 7;
			value |= file->Buffer[pos] & 0x7F;
			pos++;
			if (!(file->Buffer[pos-1] & 0x80)) break;
		}
		smfprintf(di, ", data len: %li\n",value);
		len3 = value;

		/* content type */
		i 	= 0;
		buff[0] = 0;
		smfprintf(di, "    Content type    : ");
		if (file->Buffer[pos] >= 0x80) {
			type = file->Buffer[pos] & 0x7f;
			GSM_AddWAPMIMEType(type, buff);
		} else if (file->Buffer[pos+i] == 0x1F) {
			i++;
			buff[0] = 0;
			/* len4 	= file->Buffer[pos+i]; */
			i++;
			if (!(file->Buffer[pos+i] & 0x80)) {
				while (file->Buffer[pos+i]!=0x00) {
					buff[strlen(buff)+1] = 0;
					buff[strlen(buff)]   = file->Buffer[pos+i];
					i++;
				}
				i++;
			} else {
				value = file->Buffer[pos+i] & 0x7F;
				GSM_AddWAPMIMEType(value, buff);
				i++;
			}
		} else if (file->Buffer[pos+i] < 0x1F) {
			i++;
			if (file->Buffer[pos+i] & 0x80) {
				type = file->Buffer[pos+i] & 0x7f;
				GSM_AddWAPMIMEType(type, buff);
				i++;
			} else {
				while (file->Buffer[pos+i]!=0x00) {
					buff[strlen(buff)+1] = 0;
					buff[strlen(buff)]   = file->Buffer[pos+i];
					i++;
				}
				i++;
			}
		} else {
			while (file->Buffer[pos+i]!=0x00) {
				buff[strlen(buff)+1] = 0;
				buff[strlen(buff)]   = file->Buffer[pos+i];
				i++;
			}
		}
		smfprintf(di, "%s\n",buff);
		EncodeUnicode(info->Entries[info->EntriesNum].ContentType,buff,strlen(buff));

		pos+=i;
		len2-=i;

		i=0;
		while (i<len2) {
			switch (file->Buffer[pos+i]) {
			case 0x81:
				i++;
				break;
			case 0x83:
				break;
			case 0x85:
				/* mms 1.0 file from GSM operator */
				buff2[0] = 0;
				i++;
				while (file->Buffer[pos+i]!=0x00) {
					buff2[strlen(buff2)+1] = 0;
					buff2[strlen(buff2)]   = file->Buffer[pos+i];
					i++;
				}
				EncodeUnicode(info->Entries[info->EntriesNum].File.Name,buff2,strlen(buff2));
				break;
			case 0x86:
				while (file->Buffer[pos+i]!=0x00) i++;
				break;
			case 0x89:
				strcpy(buff, "; type=");
				i++;
				while (file->Buffer[pos+i]!=0x00) {
					buff[strlen(buff)+1] = 0;
					buff[strlen(buff)]   = file->Buffer[pos+i];
					i++;
				}
				i++;
				break;
			case 0x8A:
				strcpy(buff, "; type=");
				i++;
				while (file->Buffer[pos+i]!=0x00) {
					buff[strlen(buff)+1] = 0;
					buff[strlen(buff)]   = file->Buffer[pos+i];
					i++;
				}
				i++;
				break;
			case 0x8E:
				i++;
				buff[0] = 0;
				smfprintf(di, "      Name          : ");
				while (file->Buffer[pos+i]!=0x00) {
					buff[strlen(buff)+1] = 0;
					buff[strlen(buff)]   = file->Buffer[pos+i];
					i++;
				}
				smfprintf(di, "%s\n",buff);
				EncodeUnicode(info->Entries[info->EntriesNum].File.Name,buff,strlen(buff));
				break;
			case 0xAE:
				while (file->Buffer[pos+i]!=0x00) i++;
				break;
			case 0xC0:
				i++;
				i++;
				buff[0] = 0;
				smfprintf(di, "      SMIL CID      : ");
				while (file->Buffer[pos+i]!=0x00) {
					buff[strlen(buff)+1] = 0;
					buff[strlen(buff)]   = file->Buffer[pos+i];
					i++;
				}
				smfprintf(di, "%s\n",buff);
				EncodeUnicode(info->Entries[info->EntriesNum].SMIL,buff,strlen(buff));
				break;
			default:
				smfprintf(di, "unknown3 %02x\n",file->Buffer[pos+i]);
			}
			i++;
		}
		pos+=i;

		/* data */
		info->Entries[info->EntriesNum].File.Buffer = (unsigned char *)realloc(info->Entries[info->EntriesNum].File.Buffer,len3);
		info->Entries[info->EntriesNum].File.Used   = len3;
		memcpy(info->Entries[info->EntriesNum].File.Buffer,file->Buffer+pos,len3);

		info->EntriesNum++;
		pos+=len3;
	}
	return ERR_NONE;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
