/* (c) 2002-2004 by Marcin Wiacek */

#include <string.h>

#include "gsmdata.h"
#include "../misc/coding/coding.h"

/* SNIFFS, specs somewhere in http://www.wapforum.org */
void GSM_EncodeMMSIndicatorSMSText(unsigned char *Buffer, int *Length, GSM_MMSIndicator Indicator)
{
	unsigned char 	buffer[200];
	int		i;

	strcpy(Buffer+(*Length),"\xE6\x06\"");
	(*Length)=(*Length)+3;
	strcpy(Buffer+(*Length),"application/vnd.wap.mms-message");
	(*Length)=(*Length)+31;
	Buffer[(*Length)++] = 0x00;

	strcpy(Buffer+(*Length),"\xAF\x84\x8C\x82\x98");
	(*Length)=(*Length)+5;

	i = strlen(Indicator.Address);
	while (Indicator.Address[i] != '/' && i!=0) i--;
	strcpy(Buffer+(*Length),Indicator.Address+i+1);
	(*Length)=(*Length)+strlen(Indicator.Address+i+1);
	Buffer[(*Length)++] = 0x00;

	strcpy(Buffer+(*Length),"\x8D\x90\x89");
	(*Length)=(*Length)+3;

	sprintf(buffer,"%s/TYPE=PLMN",Indicator.Sender);
	Buffer[(*Length)++] = strlen(buffer);
	Buffer[(*Length)++] = 0x80;
	strcpy(Buffer+(*Length),buffer);
	(*Length)=(*Length)+strlen(buffer);
	Buffer[(*Length)++] = 0x00;

	Buffer[(*Length)++] = 0x96;
	strcpy(Buffer+(*Length),Indicator.Title);
	(*Length)=(*Length)+strlen(Indicator.Title);
	Buffer[(*Length)++] = 0x00;

	strcpy(Buffer+(*Length),"\x8A\x80\x8E\x02\x47\xBB\x88\x05\x81\x03\x02\xA3");
	(*Length)=(*Length)+12;
	Buffer[(*Length)++] = 0x00;

	Buffer[(*Length)++] = 0x83;
	strcpy(Buffer+(*Length),Indicator.Address);
	(*Length)=(*Length)+strlen(Indicator.Address);
	Buffer[(*Length)++] = 0x00;
}

/* http://forum.nokia.com: OTA MMS Settings 1.0, OTA Settings 7.0 */
static void AddWAPSMSParameterText(unsigned char *Buffer, int *Length, unsigned char ID, char *Text, int Len)
{
	int i;

	Buffer[(*Length)++] = 0x87; 			//PARM with attributes
	Buffer[(*Length)++] = ID;
	Buffer[(*Length)++] = 0x11; 			//VALUE
	Buffer[(*Length)++] = 0x03; 			//Inline string
	for (i=0;i<Len;i++) {
		Buffer[(*Length)++] = Text[i];		//Text
	}
	Buffer[(*Length)++] = 0x00; 			//END Inline string
	Buffer[(*Length)++] = 0x01; 			//END PARMeter
}

/* http://forum.nokia.com: OTA MMS Settings 1.0, OTA Settings 7.0 */
static void AddWAPSMSParameterInt(unsigned char *Buffer, int *Length, unsigned char ID, unsigned char Value)
{
	Buffer[(*Length)++] = 0x87; 			//PARM with attributes
	Buffer[(*Length)++] = ID;
	Buffer[(*Length)++] = Value;
	Buffer[(*Length)++] = 0x01; 			//END PARMeter
}

/* http://forum.nokia.com  : OTA MMS Settings 1.0, OTA Settings 7.0
 * http://www.wapforum.org : Wireless Datagram Protocol
 */
void NOKIA_EncodeWAPMMSSettingsSMSText(unsigned char *Buffer, int *Length, GSM_WAPSettings *settings, bool MMS)
{
	int 		i;
	unsigned char 	buffer[400];

	Buffer[(*Length)++] = 0x01; 			//Push ID
	Buffer[(*Length)++] = 0x06; 			//PDU Type (push)
	Buffer[(*Length)++] = 0x2C; 			//Headers length (content type + headers)
	strcpy(Buffer+(*Length),"\x1F\x2A");
	(*Length)=(*Length)+2;				//Value length
	strcpy(Buffer+(*Length),"application/x-wap-prov.browser-settings");
	(*Length)=(*Length)+39;				//MIME-Type
	Buffer[(*Length)++] = 0x00; 			//end inline string
	strcpy(Buffer+(*Length),"\x81\xEA");
	(*Length)=(*Length)+2;				//charset UTF-8 short int.
	strcpy(Buffer+(*Length),"\x01\x01");
	(*Length)=(*Length)+2;				//version WBXML 1.1
	Buffer[(*Length)++] = 0x6A; 			//charset UTF-8
	Buffer[(*Length)++] = 0x00; 			//string table length

	Buffer[(*Length)++] = 0x45; 			//CHARACTERISTIC-LIST with content
		Buffer[(*Length)++] = 0xC6; 		//CHARACTERISTIC with content and attributes
		Buffer[(*Length)++] = 0x06; 		//TYPE=ADDRESS
		Buffer[(*Length)++] = 0x01; 		//END PARMeter
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
				// .....
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
		Buffer[(*Length)++] = 0x01; 		//END PARMeter

		/* URL */
		Buffer[(*Length)++] = 0x86; 		//CHARACTERISTIC-LIST with attributes
		if (MMS) {
			Buffer[(*Length)++] = 0x7C; 	//TYPE = MMSURL
		} else {
			Buffer[(*Length)++] = 0x07; 	//TYPE = URL
		}
		Buffer[(*Length)++] = 0x11; 		//VALUE
		Buffer[(*Length)++] = 0x03; 		//Inline string
		sprintf(buffer,"%s",DecodeUnicodeString(settings->HomePage));
		for (i=0;i<(int)strlen(buffer);i++) {
			Buffer[(*Length)++] = buffer[i];//Text
		}
		Buffer[(*Length)++] = 0x00; 		//END Inline string
		Buffer[(*Length)++] = 0x01; 		//END PARMeter

		/* ISP_NAME (name) */
		Buffer[(*Length)++] = 0xC6; 		//CHARACTERISTIC with content and attributes
		Buffer[(*Length)++] = 0x08; 		//TYPE=NAME
		Buffer[(*Length)++] = 0x01; 		//END PARMeter
			/* Settings name */
			AddWAPSMSParameterText(Buffer, Length, 0x15, DecodeUnicodeString(settings->Title), UnicodeLength(settings->Title));
		Buffer[(*Length)++] = 0x01; 		//END PARMeter
	Buffer[(*Length)++] = 0x01;			//END PARMeter
}

/* http://forum.nokia.com: OTA Settings 7.0 */
/* first it used default/ISO coding */
/* Joergen Thomsen changed to UTF8 */
void NOKIA_EncodeWAPBookmarkSMSText(unsigned char *Buffer, int *Length, GSM_WAPBookmark *bookmark)
{
	unsigned char	buffer[100];

//	bool		UnicodeCoding = false;
//	EncodeUTF8QuotedPrintable(buffer,bookmark->Title);
//	if (UnicodeLength(bookmark->Title)!=strlen(buffer)) UnicodeCoding = true;

	Buffer[(*Length)++] = 0x01; 			//Push ID
	Buffer[(*Length)++] = 0x06; 			//PDU Type (push)
	Buffer[(*Length)++] = 0x2D; 			//Headers length (content type + headers)
	strcpy(Buffer+(*Length),"\x1F\x2B");
	(*Length)=(*Length)+2;				//Value length
	strcpy(Buffer+(*Length),"application/x-wap-prov.browser-bookmarks");
	(*Length)=(*Length)+40;				//MIME-Type
	Buffer[(*Length)++] = 0x00; 			//end inline string
	strcpy(Buffer+(*Length),"\x81\xEA");
	(*Length)=(*Length)+2;				//charset UTF-8 short int.

	/* removed by Joergen Thomsen */
	/* Block from sniffs. UNKNOWN */
//	if (!UnicodeCoding) {
//		Buffer[(*Length)++] = 0x00;
//		Buffer[(*Length)++] = 0x01;
//	} else {
//		strcpy(Buffer+(*Length),"\x01\x01\x87\x68");
//		(*Length)=(*Length)+4;
//	}
//	Buffer[(*Length)++] = 0x00;

	/* added by Joergen Thomsen */
	Buffer[(*Length)++] = 0x01;			// Version WBXML 1.1
	Buffer[(*Length)++] = 0x01;			// Unknown public identifier
	Buffer[(*Length)++] = 0x6A;			// charset UTF-8
	Buffer[(*Length)++] = 0x00;			// string table length

	Buffer[(*Length)++] = 0x45; 			//CHARACTERISTIC-LIST with content
		/* URL */
		Buffer[(*Length)++] = 0xC6; 		//CHARACTERISTIC with content and attributes
		Buffer[(*Length)++] = 0x7F;             //TYPE = BOOKMARK
		Buffer[(*Length)++] = 0x01; 		//END PARMeter

			/* removed by Joergen Thomsen */
//			if (!UnicodeCoding) {
//				/* TITLE */
//				AddWAPSMSParameterText(Buffer, Length, 0x15, DecodeUnicodeString(bookmark->Title), UnicodeLength(bookmark->Title));
//				/* URL */
//				AddWAPSMSParameterText(Buffer, Length, 0x17, DecodeUnicodeString(bookmark->Address), UnicodeLength(bookmark->Address));
//			} else {
//				/* TITLE */
//				AddWAPSMSParameterText(Buffer, Length, 0x15, bookmark->Title, UnicodeLength(bookmark->Title)*2+1);
//				/* URL */
//				AddWAPSMSParameterText(Buffer, Length, 0x17, bookmark->Address, UnicodeLength(bookmark->Address)*2+1);
//			}

			/* added by Joergen Thomsen */
			/* TITLE */
			EncodeUTF8(buffer, bookmark->Title);
			AddWAPSMSParameterText(Buffer, Length, 0x15, buffer, strlen(buffer));
			/* URL */
			EncodeUTF8(buffer, bookmark->Address);
			AddWAPSMSParameterText(Buffer, Length, 0x17, buffer, strlen(buffer));

		Buffer[(*Length)++] = 0x01;		//END (CHARACTERISTIC)
	Buffer[(*Length)++] = 0x01;			//END (CHARACTERISTIC-LIST)
}

void GSM_EncodeWAPIndicatorSMSText(unsigned char *Buffer, int *Length, char *Text, char *URL)
{
	int i;

	Buffer[(*Length)++] = 0x01; 			//Push ID
	Buffer[(*Length)++] = 0x06; 			//PDU Type (push)
	Buffer[(*Length)++] = 28; 			//Headers length (content type + headers)
	strcpy(Buffer+(*Length),"\x1F\x23");
	(*Length)=(*Length)+2;				//Value length
	strcpy(Buffer+(*Length),"application/vnd.wap.sic");
	(*Length)=(*Length)+23;				//MIME-Type
	Buffer[(*Length)++] = 0x00; 			//end inline string
	strcpy(Buffer+(*Length),"\x81\xEA");
	(*Length)=(*Length)+2;				//charset UTF-8 short int.

	Buffer[(*Length)++] = 0x02; 			// WBXML 1.2
	Buffer[(*Length)++] = 0x05; 			// SI 1.0 Public Identifier
	Buffer[(*Length)++] = 0x6A;			// charset UTF-8
	Buffer[(*Length)++] = 0x00;			// string table length
	Buffer[(*Length)++] = 0x45;			// SI with content
		Buffer[(*Length)++] = 0xC6;		// indication with content and attributes
			Buffer[(*Length)++] = 0x0B;	// address
			Buffer[(*Length)++] = 0x03; 	// Inline string
			for (i=0;i<(int)strlen(URL);i++) {
				Buffer[(*Length)++] = URL[i];//Text
			}
			Buffer[(*Length)++] = 0x00; 	// END Inline string

#ifdef XXX
			Buffer[(*Length)++] = 0x0A;	// created...
			Buffer[(*Length)++] = 0xC3;	// OPAQUE
			Buffer[(*Length)++] = 0x07;	// length
			Buffer[(*Length)++] = 0x19;	// year
			Buffer[(*Length)++] = 0x80;	// year
			Buffer[(*Length)++] = 0x21;	// month
			Buffer[(*Length)++] = 0x12;	// ..
			Buffer[(*Length)++] = 0x00;	// ..
			Buffer[(*Length)++] = 0x00;	// ..
			Buffer[(*Length)++] = 0x00;	// ..
			Buffer[(*Length)++] = 0x10;	// expires
			Buffer[(*Length)++] = 0xC3;	// OPAQUE
			Buffer[(*Length)++] = 0x04;	// length
			Buffer[(*Length)++] = 0x20;	// year
			Buffer[(*Length)++] = 0x10;	// year
			Buffer[(*Length)++] = 0x06;	// month
			Buffer[(*Length)++] = 0x25;	// day
#endif

		Buffer[(*Length)++] = 0x01;		// END (indication)		
		Buffer[(*Length)++] = 0x03; 		// Inline string
		for (i=0;i<(int)strlen(Text);i++) {
			Buffer[(*Length)++] = Text[i];	//Text
		}
		Buffer[(*Length)++] = 0x00; 		// END Inline string
		Buffer[(*Length)++] = 0x01;		// END (indication)		
	Buffer[(*Length)++] = 0x01;			// END (SI)
}

void GSM_EncodeMMSFile(GSM_EncodeMultiPartMMSInfo *Info, unsigned char *Buffer, int *Length)
{
	int i;

	strcpy(Buffer+(*Length),"\x8C\x80\x98\x4F");
	(*Length)=(*Length)+4;

	/* Unique MMS ID ? */
	strcpy(Buffer+(*Length),"123456789");
	(*Length)=(*Length)+9;
	Buffer[(*Length)++] = 0x00;

	strcpy(Buffer+(*Length),"\x8D\x90\x89");
	(*Length)=(*Length)+3;

	strcpy(Buffer+(*Length),"\x01\x81\x86\x81\x96");
	(*Length)=(*Length)+5;
	
	if (UnicodeLength(Info->Subject) != 0) {
		sprintf(Buffer+(*Length),"%s",DecodeUnicodeString(Info->Subject));
		(*Length)=(*Length)+UnicodeLength(Info->Subject);
		Buffer[(*Length)++] = 0x00;
	}
	
	for (i=0;i<Info->EntriesNum;i++) {
	switch(Info->Entries[i].ID) {
	case MMS_Text:
		strcpy(Buffer+(*Length),"\x84\xA3\x01\x04\x04\x03\x83\x81\xEA");
		(*Length)=(*Length)+9;

		sprintf(Buffer+(*Length),"%s",DecodeUnicodeString(Info->Entries[i].Buffer));
		(*Length)=(*Length)+UnicodeLength(Info->Entries[i].Buffer);		
		break;
	default:
		break;
	}
	}	
}

void GSM_ClearMultiPartMMSInfo(GSM_EncodeMultiPartMMSInfo *Info)
{
	Info->EntriesNum	= 0;
	Info->Subject[0]	= 0x00;
	Info->Subject[1]	= 0x00;
	Info->Source[0] 	= 0x00;
	Info->Source[1]		= 0x00;
	Info->Destination[0] 	= 0x00;
	Info->Destination[1] 	= 0x00;
}

GSM_Error GSM_EncodeURLFile(unsigned char *Buffer, int *Length, GSM_WAPBookmark *bookmark)
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

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
