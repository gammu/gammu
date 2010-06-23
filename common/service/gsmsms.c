
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "../gsmcomon.h"
#include "../misc/coding.h"
#include "gsmsms.h"
#include "gsmcal.h"
#include "gsmpbk.h"
#include "gsmlogo.h"
#include "gsmring.h"
#include "gsmwap.h"
#include "gsmnet.h"

/* User data headers */
static GSM_UDHHeader UDHHeaders[] = {
	/* See GSM 03.40 section 9.2.3.24.1
	 * 1 byte 0x00
	 * 1 byte 0x03
	 * 1 byte 0x01: unique ID for message series
	 * 1 byte 0x00: how many SMS in sequence
	 * 1 byte 0x00: number of current SMS in sequence 		*/
	{ UDH_ConcatenatedMessages, 0x05, "\x00\x03\x01\x00\x00",2,4,3},

	/* See GSM 03.40 section 9.2.3.24.2 for voice, fax and email messages */
	{ UDH_DisableVoice,         0x04, "\x01\x02\x00\x00",-1,-1,-1},
	{ UDH_DisableFax,           0x04, "\x01\x02\x01\x00",-1,-1,-1},
	{ UDH_DisableEmail,         0x04, "\x01\x02\x02\x00",-1,-1,-1},
	{ UDH_EnableVoice,          0x04, "\x01\x02\x00\x01",-1,-1,-1},
	{ UDH_EnableFax,            0x04, "\x01\x02\x01\x01",-1,-1,-1},
	{ UDH_EnableEmail,          0x04, "\x01\x02\x02\x01",-1,-1,-1},

	/* When send such SMS to some phones, they don't display anything,
	 * only beep and enable vibra/light
	 */
	{ UDH_VoidSMS,              0x08, "\x01\x02\x02\x01\x01\x02\x02\x00",-1,-1,-1},

	/* Nokia Smart Messaging (short version) UDH
	 * General format :
	 * 1 byte  0x05      : IEI application port addressing scheme, 16 bit address
	 * 1 byte  0x04      : IEI length
	 * 2 bytes           : destination address : high & low byte
	 * 2 bytes 0x00 0x00 : originator address  : high & low byte */
	{ UDH_NokiaRingtone,        0x06, "\x05\x04\x15\x81\x00\x00",-1,-1,-1},
	{ UDH_NokiaOperatorLogo,    0x06, "\x05\x04\x15\x82\x00\x00",-1,-1,-1},
	{ UDH_NokiaCallerLogo,      0x06, "\x05\x04\x15\x83\x00\x00",-1,-1,-1},
	{ UDH_NokiaWAPBookmark,     0x06, "\x05\x04\xc3\x4f\x00\x00",-1,-1,-1},

	/* Nokia Smart Messaging (long version) UDH
	 * General format:
	 * 1 byte 0x05       : IEI application port addressing scheme, 16 bit address
	 * 1 byte 0x04       : IEI length
	 * 2 bytes 0x00 0x00 : destination address : high & low byte
	 * 2 bytes 0x00 0x00 : originator address  : high & low byte
	 * 1 byte 0x00       : null byte for end first part ?
	 * 1 byte 0x03       : length for rest 
	 * 1 byte            : unique ID for message series
	 * 1 byte            : number of all SMS
	 * 1 byte            : number of current SMS */
	{ UDH_NokiaCalendarLong,    0x0b, "\x05\x04\x00\xe4\x00\x00\x00\x03\xc7\x00\x00",8,10,9},
	{ UDH_NokiaProfileLong,     0x0b, "\x05\x04\x15\x8a\x00\x00\x00\x03\xce\x00\x00",8,10,9},
	{ UDH_NokiaWAPBookmarkLong, 0x0b, "\x05\x04\xc3\x4f\x00\x00\x00\x03\x01\x00\x00",8,10,9},
	{ UDH_NokiaWAPSettingsLong, 0x0b, "\x05\x04\xc3\x4f\x00\x00\x00\x03\x7f\x00\x00",8,10,9},
	{ UDH_NokiaPhonebookLong,   0x0b, "\x05\x04\x23\xf4\x00\x00\x00\x03\x01\x00\x00",8,10,9},
	{ UDH_NokiaOperatorLogoLong,0x0b, "\x05\x04\x15\x82\x00\x00\x00\x03\x02\x00\x00",8,10,9},

	{ UDH_NoUDH,                0x00, "",-1,-1,-1}
};

/* --------------------------- Unpacking SMS ------------------------------- */

/* See GSM 03.40 section 9.2.3.11 */
static GSM_Error GSM_DecodeSMSDateTime(GSM_DateTime *DT, unsigned char *req)
{
	DT->Year    = DecodeWithBCDAlphabet(req[0]);
	if (DT->Year<90) DT->Year=DT->Year+2000; else DT->Year=DT->Year+1990;
	DT->Month   = DecodeWithBCDAlphabet(req[1]);
	DT->Day     = DecodeWithBCDAlphabet(req[2]);
	DT->Hour    = DecodeWithBCDAlphabet(req[3]);
	DT->Minute  = DecodeWithBCDAlphabet(req[4]);
	DT->Second  = DecodeWithBCDAlphabet(req[5]);

	DT->Timezone=(10*(req[6]&0x07)+(req[6]>>4))/4;
	if (req[6]&0x08) DT->Timezone = -DT->Timezone;

	dprintf("Decoding date & time: ");
	dprintf("%s %4d/%02d/%02d ", DayOfWeek(DT->Year, DT->Month, DT->Day),
		DT->Year, DT->Month, DT->Day);
	dprintf("%02d:%02d:%02d %04d\n", DT->Hour, DT->Minute, DT->Second, DT->Timezone);

	return GE_NONE;
}

void GSM_DecodeUDHHeader(GSM_UDHHeader *UDH)
{
	int	i, tmp, w;
	bool	UDHOK;

	UDH->Type 	= UDH_UserUDH;
	UDH->ID	  	= -1;
	UDH->PartNumber	= -1;
	UDH->AllParts	= -1;

	i=-1;
	while (true) {
		i++;
		if (UDHHeaders[i].Type==UDH_NoUDH) break;

		tmp=UDHHeaders[i].Length;
		/* if length is the same */
		if (tmp==UDH->Text[0]) { 

			if (tmp==0x05) tmp=tmp-3;/*three last bytes can be different for such UDH*/
			if (tmp==0x0b) tmp=tmp-3;/*three last bytes can be different for such UDH*/

			UDHOK=true;
			for (w=0;w<tmp;w++) {
				if (UDHHeaders[i].Text[w]!=UDH->Text[w+1]) {
					UDHOK=false;
					break;
				}
			}
			if (UDHOK) {
				UDH->Type=UDHHeaders[i].Type;

				if (UDHHeaders[i].ID		!=-1) UDH->ID 		= UDH->Text[UDHHeaders[i].ID+1];
				if (UDHHeaders[i].PartNumber	!=-1) UDH->PartNumber 	= UDH->Text[UDHHeaders[i].PartNumber+1];
				if (UDHHeaders[i].AllParts	!=-1) UDH->AllParts 	= UDH->Text[UDHHeaders[i].AllParts+1];
				break;
			}
		}
	}

#ifdef DEBUG
	dprintf("Type of UDH: ");
	switch (UDH->Type) {
	case UDH_ConcatenatedMessages:	dprintf("Concatenated (linked) message"); break;
	case UDH_DisableVoice:		dprintf("Disables voice indicator");	 break;
	case UDH_EnableVoice:		dprintf("Enables voice indicator");	 break;
	case UDH_DisableFax:		dprintf("Disables fax indicator");	 break;
	case UDH_EnableFax:		dprintf("Enables fax indicator");	 break;
	case UDH_DisableEmail:		dprintf("Disables email indicator");	 break;
	case UDH_EnableEmail:		dprintf("Enables email indicator");	 break;
	case UDH_VoidSMS:		dprintf("Void SMS");			 break;
	case UDH_NokiaWAPBookmark:	dprintf("Nokia WAP Bookmark");		 break;
	case UDH_NokiaOperatorLogoLong:	dprintf("Nokia operator logo");		 break;
	case UDH_NokiaWAPBookmarkLong:	dprintf("Nokia WAP Bookmark");		 break;
	case UDH_NokiaWAPSettingsLong:	dprintf("Nokia WAP Settings");		 break;
	case UDH_NokiaRingtone:		dprintf("Nokia ringtone");		 break;
	case UDH_NokiaOperatorLogo:	dprintf("Nokia GSM operator logo");	 break;
	case UDH_NokiaCallerLogo:	dprintf("Nokia caller logo");		 break;  	
	case UDH_NokiaProfileLong:	dprintf("Nokia profile");		 break;
	case UDH_NokiaCalendarLong:	dprintf("Nokia calendar note");		 break;
	case UDH_NokiaPhonebookLong:	dprintf("Nokia phonebook entry");	 break;
	case UDH_UserUDH:		dprintf("User UDH");			 break;
	case UDH_NoUDH:								 break;
	}
	if (UDH->ID != -1) dprintf(", ID %i",UDH->ID);
	if (UDH->PartNumber != -1 && UDH->AllParts != -1) {
		dprintf(", part %i of %i",UDH->PartNumber,UDH->AllParts);
	}
	dprintf("\n");
	if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) DumpMessage(di.df, UDH->Text, UDH->Length);
#endif
}

GSM_Error GSM_DecodeSMSFrameText(GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout)
{
	int		off=0;		/* off - length of the user data header */
	int 		w,i,tmp=0;
	unsigned char	output[161];

	SMS->UDH.Length = 0;
	/* UDH header available */
	if (buffer[Layout.firstbyte] & 64) {
		/* Length of UDH header */
		off = (buffer[Layout.Text] + 1);
		SMS->UDH.Length = off;
		dprintf("UDH header available (length %i)\n",off);    
    
		/* Copy UDH header into SMS->UDH */
		for (i = 0; i < off; i++) SMS->UDH.Text[i] = buffer[Layout.Text + i];

		GSM_DecodeUDHHeader(&SMS->UDH);
	}

	/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme) and GSM 03.38 section 4 */
	if ((buffer[Layout.TPDCS] & 0xf4) == 0xf4) SMS->Coding=GSM_Coding_8bit;
	if ((buffer[Layout.TPDCS] & 0x08) == 0x08) SMS->Coding=GSM_Coding_Unicode;

	switch (SMS->Coding) {
		case GSM_Coding_Default:
			i = 0;
			do {
				i+=7;
				w=(i-off)%i;
			} while (w<0);
			SMS->Length=buffer[Layout.TPUDL] - (off*8 + w) / 7;
			tmp=GSM_UnpackEightBitsToSeven(w, buffer[Layout.TPUDL]-off, SMS->Length, buffer+(Layout.Text+off), output);
			dprintf("7 bit SMS, length %i\n",SMS->Length);
			DecodeDefault (SMS->Text, output, SMS->Length, true, NULL);
			dprintf("%s\n",DecodeUnicodeString(SMS->Text));
			break;
		case GSM_Coding_8bit:
			SMS->Length=buffer[Layout.TPUDL] - off;
			memcpy(SMS->Text,buffer+(Layout.Text+off),SMS->Length);
#ifdef DEBUG
			dprintf("8 bit SMS, length %i\n",SMS->Length);
			if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) DumpMessage(di.df, SMS->Text, SMS->Length);
#endif
			break;
		case GSM_Coding_Unicode:
			SMS->Length=(buffer[Layout.TPUDL] - off) / 2;
			DecodeUnicodeSpecialNOKIAChars(SMS->Text,buffer+(Layout.Text+off), SMS->Length);
#ifdef DEBUG
			dprintf("Unicode SMS, length %i\n",SMS->Length);
			if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) DumpMessage(di.df,buffer+(Layout.Text+off), SMS->Length*2);
			dprintf("%s\n",DecodeUnicodeString(SMS->Text));
#endif
			break;
	}

	return GE_NONE;
}

GSM_Error GSM_DecodeSMSFrameStatusReportData(GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout)
{
	SMS->DeliveryStatus = buffer[Layout.TPStatus];
    
	if (buffer[Layout.TPStatus] < 0x03) {
		EncodeUnicode(SMS->Text,"Delivered",9);
		SMS->Length = 9;      
	} else if (buffer[Layout.TPStatus] & 0x40) {
		EncodeUnicode(SMS->Text,"Failed",6);
		SMS->Length = 6;
	} else if (buffer[Layout.TPStatus] & 0x20) {
		EncodeUnicode(SMS->Text,"Pending",7);
		SMS->Length = 7;
	} else {
		EncodeUnicode(SMS->Text,"Unknown",7);
		SMS->Length = 7;
	}

#ifdef DEBUG
	/* See GSM 03.40 section 9.2.3.15 (TP-Status) */
	if (buffer[Layout.TPStatus] & 0x40) {
		if (buffer[Layout.TPStatus] & 0x20) {
			/* 0x60, 0x61, ... */
			dprintf("Temporary error, SC is not making any more transfer attempts\n");
		} else {
			/* 0x40, 0x41, ... */
     			dprintf("Permanent error, SC is not making any more transfer attempts\n");
		}
    	} else if (buffer[Layout.TPStatus] & 0x20) {
		/* 0x20, 0x21, ... */
		dprintf("Temporary error, SC still trying to transfer SM\n");
	}
	switch (buffer[Layout.TPStatus]) {
	case 0x00: dprintf("SM received by the SME");					break;
	case 0x01: dprintf("SM forwarded by the SC to the SME but the SC is unable to confirm delivery");break;
	case 0x02: dprintf("SM replaced by the SC");					break;
	case 0x20: dprintf("Congestion");						break;
	case 0x21: dprintf("SME busy");							break;
	case 0x22: dprintf("No response from SME");					break;
	case 0x23: dprintf("Service rejected");						break;
	case 0x24: dprintf("Quality of service not aviable");				break;
	case 0x25: dprintf("Error in SME");						break;
        case 0x40: dprintf("Remote procedure error");					break;
        case 0x41: dprintf("Incompatibile destination");				break;
        case 0x42: dprintf("Connection rejected by SME");				break;
        case 0x43: dprintf("Not obtainable");						break;
        case 0x44: dprintf("Quality of service not available");				break;
        case 0x45: dprintf("No internetworking available");				break;
        case 0x46: dprintf("SM Validity Period Expired");				break;
        case 0x47: dprintf("SM deleted by originating SME");				break;
        case 0x48: dprintf("SM Deleted by SC Administration");				break;
        case 0x49: dprintf("SM does not exist");					break;
        case 0x60: dprintf("Congestion");						break;
        case 0x61: dprintf("SME busy");							break;
        case 0x62: dprintf("No response from SME");					break;
        case 0x63: dprintf("Service rejected");						break;
        case 0x64: dprintf("Quality of service not available");				break;
        case 0x65: dprintf("Error in SME");						break;
        default  : dprintf("Reserved/Specific to SC: %x",buffer[Layout.TPStatus]);	break;
	}          
	dprintf("\n");
#endif /* DEBUG */

	return GE_NONE;
}

GSM_Error GSM_DecodeSMSFrame(GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout)
{
#ifdef DEBUG
	if (Layout.firstbyte == 255) {
		dprintf("ERROR: firstbyte in SMS layout not set\n");
		return GE_UNKNOWN;
	}
	if (Layout.TPDCS     != 255) dprintf("TPDCS     : %02x %i\n",buffer[Layout.TPDCS]    ,buffer[Layout.TPDCS]);
	if (Layout.TPMR      != 255) dprintf("TPMR      : %02x %i\n",buffer[Layout.TPMR]     ,buffer[Layout.TPMR]);
	if (Layout.TPPID     != 255) dprintf("TPPID     : %02x %i\n",buffer[Layout.TPPID]    ,buffer[Layout.TPPID]);
	if (Layout.TPUDL     != 255) dprintf("TPUDL     : %02x %i\n",buffer[Layout.TPUDL]    ,buffer[Layout.TPUDL]);
	if (Layout.firstbyte != 255) dprintf("FirstByte : %02x %i\n",buffer[Layout.firstbyte],buffer[Layout.firstbyte]);
	if (Layout.Text      != 255) dprintf("Text      : %02x %i\n",buffer[Layout.Text]     ,buffer[Layout.Text]);
#endif

	SMS->UDH.Type 		= UDH_NoUDH;
	SMS->Coding 		= GSM_Coding_Default;
	SMS->Length		= 0;
	SMS->SMSC.Number[0] 	= 0;
	SMS->SMSC.Number[1] 	= 0;
	SMS->Number[0] 		= 0;
	SMS->Number[1] 		= 0;
	SMS->Name[0] 		= 0;
	SMS->Name[1] 		= 0;
	SMS->ReplyViaSameSMSC	= false;
	if (Layout.SMSCNumber!=255) {
		GSM_UnpackSemiOctetNumber(SMS->SMSC.Number,buffer+Layout.SMSCNumber,false);
		dprintf("SMS center number : \"%s\"\n",DecodeUnicodeString(SMS->SMSC.Number));
	}
	if ((buffer[Layout.firstbyte] & 0x80)!=0) SMS->ReplyViaSameSMSC=true;
#ifdef DEBUG
	if (SMS->ReplyViaSameSMSC) dprintf("SMS centre set for reply\n");
#endif
	if (Layout.Number!=255) {
		GSM_UnpackSemiOctetNumber(SMS->Number,buffer+Layout.Number,true);
		dprintf("Remote number : \"%s\"\n",DecodeUnicodeString(SMS->Number));
	}
	if (Layout.Text != 255 && Layout.TPDCS!=255 && Layout.TPUDL!=255) {
		GSM_DecodeSMSFrameText(SMS, buffer, Layout);
	}
	if (Layout.DateTime != 255) {
		GSM_DecodeSMSDateTime(&SMS->DateTime,buffer+(Layout.DateTime));
	}
	if (Layout.SMSCTime != 255 && Layout.TPStatus != 255) {
		/* See GSM 03.40 section 9.2.3.11 (TP-Service-Centre-Time-Stamp) */
		dprintf("SMSC response date: ");
		GSM_DecodeSMSDateTime(&SMS->SMSCTime, buffer+(Layout.SMSCTime));
		GSM_DecodeSMSFrameStatusReportData(SMS,buffer,Layout);
	}
	SMS->Class = -1;
	if (Layout.TPDCS != 255) {
		if ((buffer[Layout.TPDCS] & 0xF3)==0xF0) SMS->Class = 0;
		if ((buffer[Layout.TPDCS] & 0xF3)==0xF1) SMS->Class = 1;
		if ((buffer[Layout.TPDCS] & 0xF3)==0xF2) SMS->Class = 2;
		if ((buffer[Layout.TPDCS] & 0xF3)==0xF3) SMS->Class = 3;
	}
	dprintf("SMS class: %i\n",SMS->Class);
	SMS->MessageReference = 0;
	if (Layout.TPMR != 255) {
		SMS->MessageReference = buffer[Layout.TPMR];
	}
	SMS->ReplaceMessage = 0;
	if (Layout.TPPID != 255) {
		if (buffer[Layout.TPPID] > 0x40 && buffer[Layout.TPPID] < 0x48) {
			SMS->ReplaceMessage = buffer[Layout.TPPID] - 0x40;
		}
	}
	SMS->RejectDuplicates = false;
	if ((buffer[Layout.firstbyte] & 0x04)==0x04) SMS->RejectDuplicates = true;

	return GE_NONE;
}

/* ----------------------------- Packing SMS ------------------------------- */

/* See GSM 03.40 section 9.2.3.11 */
static GSM_Error GSM_EncodeSMSDateTime(GSM_DateTime *DT, unsigned char *req)
{
	int Year;

	dprintf("Encoding SMS datetime: %02i/%02i/%04i %02i:%02i:%02i\n",
		DT->Day,DT->Month,DT->Year,DT->Hour,DT->Minute,DT->Second);

	/* We need to have only two last digits of year */
	if (DT->Year>1900)
	{
		if (DT->Year<2000) Year = DT->Year-1900;
			      else Year = DT->Year-2000;
	} else Year = DT->Year;

	req[0]=EncodeWithBCDAlphabet(Year);
	req[1]=EncodeWithBCDAlphabet(DT->Month);
	req[2]=EncodeWithBCDAlphabet(DT->Day);
	req[3]=EncodeWithBCDAlphabet(DT->Hour);
	req[4]=EncodeWithBCDAlphabet(DT->Minute);
	req[5]=EncodeWithBCDAlphabet(DT->Second);

	/* FIXME: do it */
	req[6]=0; /* TimeZone = +-0 */

	return GE_NONE;
}

static int GSM_EncodeSMSFrameText(GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout)
{
	int	off = 0;	/* off - length of the user data header */
	int	size = 0, size2 = 0, w,p;
	char	buff[200];

	if (SMS->UDH.Type!=UDH_NoUDH) {
		buffer[Layout.firstbyte] |= 0x40;			/* GSM 03.40 section 9.2.3.23 (TP-User-Data-Header-Indicator) */
		off = 1 + SMS->UDH.Text[0];				/* off - length of the user data header */
		memcpy(buffer+Layout.Text, SMS->UDH.Text, off);		/* we copy the udh */
#ifdef DEBUG
		dprintf("UDH, length %i\n",off);
		if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) DumpMessage(di.df, SMS->UDH.Text, off);
#endif
	}
	switch (SMS->Coding) {
		case GSM_Coding_8bit:
			/* the mask for the 8-bit data */
			/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme)
			 * and GSM 03.38 section 4 */
			buffer[Layout.TPDCS] |= 0xf4;    
			memcpy(buffer+(Layout.Text+off), SMS->Text, SMS->Length);
			size2 = size = SMS->Length+off;
#ifdef DEBUG
			dprintf("8 bit SMS, length %i\n",SMS->Length);
			if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) DumpMessage(di.df,SMS->Text,SMS->Length);
#endif
			break;
		case GSM_Coding_Default:
			p = 0;
			do {
				p+=7;
				w=(p-off)%p;
			} while (w<0);
			p = strlen(DecodeUnicodeString(SMS->Text));
			EncodeDefault(buff, SMS->Text, &p, true, NULL);
			size = GSM_PackSevenBitsToEight(w, buff, buffer+(Layout.Text+off), p);
			size += off;
			size2 = (off*8 + w) / 7 + p;
			dprintf("7 bit SMS, length %i, %i\n",size,size2);
			dprintf("%s\n",DecodeUnicodeString(SMS->Text));
			if (size > GSM_MAX_8BIT_SMS_LENGTH) {
				size = 0; size2 = 0;
			}
			break;
		case GSM_Coding_Unicode:
			/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme)
			 * and GSM 03.38 section 4 */
			buffer[Layout.TPDCS] |= 0x08;
			EncodeUnicodeSpecialNOKIAChars(buffer+(Layout.Text+off), SMS->Text, strlen(DecodeUnicodeString(SMS->Text)));
			size=size2=strlen(DecodeUnicodeString(buffer+(Layout.Text+off)))*2+off;
#ifdef DEBUG
			dprintf("Unicode SMS, length %i\n",(size2-off)/2);
			if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) DumpMessage(di.df,buffer+(Layout.Text+off), size2-off);
			dprintf("%s\n",DecodeUnicodeString(buffer+(Layout.Text+off)));
#endif
			break;
	}

	/* SMS->Length is:
	 - integer representation of the number od octets within the user data
	   when UD is coded using 8bit data
	 - the sum of the number of septets in UDH including any padding
	   and number of septets in UD in other case */
	/* GSM 03.40 section 9.2.3.16 (TP-User-Data-Length) */
	buffer[Layout.TPUDL] = size2;
	return size;
}

GSM_Error GSM_EncodeSMSFrame(GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout, int *length, bool clear)
{
	int i;

	if (clear) {
		/* Cleaning up to the SMS text */
		for (i=0;i<Layout.Text;i++) buffer[i] = 0;
	}

	/* GSM 03.40 section 9.2.3.1 (TP-Message-Type-Indicator) */
	switch (SMS->PDU) {
		case SMS_Submit:
			buffer[Layout.firstbyte] |= 0x01;
			break;
		/* SMS_Status_Report when Submit sms should have delivery report */
		/* We DON'T CREATE FRAME FOR REAL SMS_STATUS_REPORT		 */
		case SMS_Status_Report:
			buffer[Layout.firstbyte] |= 0x01;
			/* GSM 03.40 section 9.2.3.5 (TP-Status-Raport-Request) */
			/* Mask for request for delivery report from SMSC */
			buffer[Layout.firstbyte] |= 0x20;
			break;
		case SMS_Deliver:
			buffer[Layout.firstbyte] |= 0x00;
	}

	/* GSM 03.40 section 9.2.3.17 (TP-Reply-Path) */
	if (SMS->ReplyViaSameSMSC) buffer[Layout.firstbyte] |= 0x80;

	if (Layout.Number!=255) {
		buffer[Layout.Number] = GSM_PackSemiOctetNumber(SMS->Number,buffer+(Layout.Number+1),true);
		dprintf("Recipient number \"%s\"\n",DecodeUnicodeString(SMS->Number));
	}
	if (Layout.SMSCNumber!=255) {
		buffer[Layout.SMSCNumber]=GSM_PackSemiOctetNumber(SMS->SMSC.Number,buffer+(Layout.SMSCNumber+1), false);
		dprintf("SMSC number \"%s\"\n",DecodeUnicodeString(SMS->SMSC.Number));
	}
  
	/* Message Class*/
	/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme) and GSM 03.38 section 4 */
	if (Layout.TPDCS != 255) {
		if (SMS->Class>=0 && SMS->Class<5) buffer[Layout.TPDCS] |= (240+SMS->Class);
	}

	if (Layout.TPVP != 255) {
		/* GSM 03.40 section 9.2.3.3 (TP-Validity-Period-Format) */
		/* Bits 4 and 3: 10. TP-VP field present and integer represent (relative) */
		buffer[Layout.firstbyte] |= 0x10;
		buffer[Layout.TPVP]=((unsigned char)SMS->SMSC.Validity.Relative);
		dprintf("SMS validity %02x\n",SMS->SMSC.Validity.Relative);
	}

	if (Layout.DateTime != 255) {
		GSM_EncodeSMSDateTime(&SMS->DateTime, buffer+Layout.DateTime);
	}

	if (Layout.TPMR != 255) {
		buffer[Layout.TPMR] = SMS->MessageReference;
	}

	if (SMS->RejectDuplicates) {
		/* GSM 03.40 section 9.2.3.25 (TP Reject Duplicates) */
		buffer[Layout.firstbyte] |= 0x04;
	}

	if (Layout.TPPID != 255) {
		buffer[Layout.TPPID] = 0;
		if (SMS->ReplaceMessage > 0 && SMS->ReplaceMessage < 8) {
			buffer[Layout.TPPID] = 0x40 + SMS->ReplaceMessage;
		}
	}

	/* size is the length of the data in octets including udh */
	*length=GSM_EncodeSMSFrameText(SMS,buffer,Layout);
	if (*length == 0) return GE_UNKNOWN;
	*length += Layout.Text;	

	return GE_NONE;
}

/* ----------------- Some help functions ----------------------------------- */

void GSM_SetDefaultSMSData(GSM_SMSMessage *SMS)
{
	SMS->Class			= -1;
	SMS->SMSC.Location		= 1;
	SMS->SMSC.Validity.VPF		= GSM_RelativeFormat;
	SMS->SMSC.Validity.Relative	= GSMV_Max_Time;
	SMS->ReplyViaSameSMSC		= false;
	SMS->UDH.Type			= UDH_NoUDH;
	SMS->UDH.Length			= 0;
	SMS->UDH.Text[0] 		= 0;
	SMS->UDH.ID			= 0;
	SMS->UDH.PartNumber		= 0;
	SMS->UDH.AllParts		= 0;
	SMS->Coding			= GSM_Coding_Default;
	SMS->Text[0] 			= 0;
	SMS->Text[1] 			= 0;
	SMS->PDU			= SMS_Submit;
	SMS->RejectDuplicates		= false;
	SMS->MessageReference		= 0;
	SMS->ReplaceMessage		= 0;

	/* This part is required to save SMS */    
	SMS->State			= GSM_UnSent;
	SMS->Location			= 0;
	SMS->Folder			= 0x02;	/*Outbox*/
	GSM_GetCurrentDateTime (&SMS->DateTime);
	SMS->Name[0]			= 0;
	SMS->Name[1]			= 0;
}

/* This function encodes the UserDataHeader as described in:
 * - GSM 03.40 version 6.1.0 Release 1997, section 9.2.3.24
 * - Smart Messaging Specification, Revision 1.0.0, September 15, 1997
 */
void GSM_EncodeUDHHeader(GSM_UDHHeader *UDH)
{
	int i=0;

	switch (UDH->Type) {
		case UDH_NoUDH:
			UDH->Length = 0;
			break;
		case UDH_UserUDH:
			UDH->Length = UDH->Text[0] + 1;
			break;
		default:
			while (true) {
				if (UDHHeaders[i].Type==UDH_NoUDH)
				{
					dprintf("Not supported UDH type\n");
					break;
				}
				if (UDHHeaders[i].Type==UDH->Type)
				{
					/* UDH Length */
					UDH->Text[0] = UDHHeaders[i].Length;
					memcpy(UDH->Text+1, UDHHeaders[i].Text, UDHHeaders[i].Length);
					UDH->Length 	= UDH->Text[0] + 1;

					if (UDHHeaders[i].ID 		!= -1) UDH->Text[UDHHeaders[i].ID+1]		= UDH->ID;
					if (UDHHeaders[i].PartNumber 	!= -1) UDH->Text[UDHHeaders[i].PartNumber+1]	= UDH->PartNumber;
					if (UDHHeaders[i].AllParts 	!= -1) UDH->Text[UDHHeaders[i].AllParts+1]	= UDH->AllParts;
					break;
				}
				i++;
			}
	}
}

/* Calculates number of SMS and number of left chars in SMS */
void GSM_SMSCounter(int 		MessageLength,
		    unsigned char 	*MessageBuffer,
		    GSM_UDHHeader 	UDH,
		    GSM_Coding_Type 	Coding,
		    int 		*SMSNum,
		    int 		*CharsLeft)
{
	int pos=0,maxlen,smslen;

	*SMSNum = 0;
	maxlen  = 0;
	smslen  = 0;
	if (MessageLength == 0) *SMSNum = 1;
	while (true) {
		if (pos==MessageLength) break;

		switch (Coding) {
			case GSM_Coding_8bit:
				/*max=140*/
				maxlen=(GSM_MAX_8BIT_SMS_LENGTH-UDH.Length);
				break;
			case GSM_Coding_Default:
				/*max=160*/
				FindDefaultAlphabetLen(MessageBuffer+pos*2,&maxlen,&smslen,(GSM_MAX_8BIT_SMS_LENGTH-UDH.Length)*8/7);
				break;
			case GSM_Coding_Unicode:
				/*max=70*/
				maxlen=(GSM_MAX_8BIT_SMS_LENGTH-UDH.Length)/2;
				break;
		}
		if ((MessageLength-pos)<maxlen) maxlen=MessageLength-pos;
		pos=pos+maxlen;
		(*SMSNum)++;
	}
	switch (Coding) {
		case GSM_Coding_8bit:
			/*max=140*/
			(*CharsLeft)=GSM_MAX_8BIT_SMS_LENGTH-UDH.Length-maxlen;
			break;
		case GSM_Coding_Default:
			/*max=160*/
			(*CharsLeft)=(GSM_MAX_8BIT_SMS_LENGTH-UDH.Length)*8/7-smslen;
			break;
		case GSM_Coding_Unicode:
			/*max=70*/
			(*CharsLeft)=(GSM_MAX_8BIT_SMS_LENGTH-UDH.Length)/2-maxlen;
			break;
	}	
}

/* ----------------- Splitting SMS into parts ------------------------------ */

static unsigned char GSM_MakeSMSIDFromTime()
{
	GSM_DateTime 	Date;
	unsigned char	retval;

	GSM_GetCurrentDateTime (&Date);
	retval = Date.Second;
	switch (Date.Minute/10) {
	case 2: case 7: 	retval = retval +  60; break;
	case 4: case 8: 	retval = retval + 120; break;
	case 9: case 5: case 0: retval = retval + 180; break;
	}
	retval += Date.Minute/10;
	return retval;
}

void GSM_MakeMultiPartSMS(GSM_MultiSMSMessage	*SMS,
			  unsigned char		*MessageBuffer,
			  int			MessageLength,
			  GSM_UDH		UDHType,
			  GSM_Coding_Type	Coding,
			  int			Class,
			  unsigned char		ReplaceMessage)
{
	int 		j;
	int 		maxlen,pos=0,smslen;
	GSM_SMSMessage  *MySMS;
	unsigned char 	UDHID;

	while (true) {
		if (pos==MessageLength) break;
		MySMS = &SMS->SMS[SMS->Number];
		GSM_SetDefaultSMSData(MySMS);
		MySMS->UDH.Type = UDHType;
		GSM_EncodeUDHHeader(&MySMS->UDH);
		MySMS->Class    = Class;
		MySMS->Coding   = Coding;
		maxlen		= 0;
		switch (Coding) {
			case GSM_Coding_8bit:
				/*max=140*/
				maxlen=(GSM_MAX_8BIT_SMS_LENGTH-MySMS->UDH.Length);
				break;
			case GSM_Coding_Default:
				/*max=160*/
				FindDefaultAlphabetLen(MessageBuffer+pos*2,&maxlen,&smslen,(GSM_MAX_8BIT_SMS_LENGTH-MySMS->UDH.Length)*8/7);
				break;
			case GSM_Coding_Unicode:
				/*max=70*/
				maxlen=(GSM_MAX_8BIT_SMS_LENGTH-MySMS->UDH.Length)/2;
				break;
		}
		if ((MessageLength-pos)<maxlen) maxlen=MessageLength-pos;
		switch (Coding) {
			case GSM_Coding_8bit:
				memcpy(MySMS->Text,MessageBuffer+pos,maxlen);
				break;
			case GSM_Coding_Unicode:
			case GSM_Coding_Default:
				memcpy(MySMS->Text,MessageBuffer+pos*2,maxlen*2);
				MySMS->Text[maxlen*2]  = 0;
				MySMS->Text[maxlen*2+1]= 0;
				break;
		}
		MySMS->Length 	= maxlen;
		pos 		= pos+maxlen;
		SMS->Number++;
		if (SMS->Number==MAX_MULTI_SMS) break;
	}
	UDHID = GSM_MakeSMSIDFromTime();
	for (j=0;j<SMS->Number;j++)
	{
		SMS->SMS[j].MessageReference 	= 0;
		SMS->SMS[j].RejectDuplicates 	= false;
		SMS->SMS[j].ReplaceMessage 	= 0;
		SMS->SMS[j].UDH.ID 		= UDHID;
		SMS->SMS[j].UDH.PartNumber 	= j+1;
		SMS->SMS[j].UDH.AllParts 	= SMS->Number;
		GSM_EncodeUDHHeader(&SMS->SMS[j].UDH);
	}
	if (SMS->Number == 1) SMS->SMS[0].ReplaceMessage = ReplaceMessage;
}

static void GSM_EncodeSMS30MultiPartSMS(GSM_EncodeMultiPartSMSInfo *Info,
					char *Buffer, int *Length)
{
	int len;

	/*SM version. Here 3.0*/
	Buffer[(*Length)++] = 0x30;

	if (Info->Entries[0].ID == SMS_NokiaProfileLong) {
		if (Info->Entries[0].Buffer[0]!=0x00 || Info->Entries[0].Buffer[1]!=0x00) {
			Buffer[(*Length)++] = SM30_PROFILENAME;
			Buffer[(*Length)++] = 0x00;
			Buffer[(*Length)++] = 2*strlen(DecodeUnicodeString(Info->Entries[0].Buffer));
			CopyUnicodeString(Buffer+(*Length),Info->Entries[0].Buffer);
			*Length = *Length + 2*strlen(DecodeUnicodeString(Info->Entries[0].Buffer));
		}
	}
	if (Info->Entries[0].ID == SMS_NokiaProfileLong || Info->Entries[0].ID == SMS_NokiaRingtoneLong) {
		Buffer[(*Length)++] = SM30_RINGTONE;
		/* Length for this part later will be changed */
		Buffer[(*Length)++] = 0x01;
		Buffer[(*Length)++] = 0x00;
		/* Smart Messaging 3.0 says: 16*9=144 bytes,
		 * but on 3310 4.02 it was possible to save about 196 chars
		 * (without cutting) */
		len = 196;
		Info->Entries[0].RingtoneNotes=GSM_EncodeNokiaRTTLRingtone(*Info->Entries[0].Ringtone,Buffer+(*Length),&len);
		Buffer[(*Length)-2] = len / 256;
		Buffer[(*Length)-1] = len % 256;
		*Length = *Length + len;
	}
	if (Info->Entries[0].ID != SMS_NokiaRingtoneLong) {
		if (Info->Entries[0].ID == SMS_NokiaPictureImageLong) {
			Buffer[(*Length)++] = SM30_OTA;
		} else {
			Buffer[(*Length)++] = SM30_SCREENSAVER;
		}
		Buffer[(*Length)++] = 0x01;
		Buffer[(*Length)++] = 0x00;
		NOKIA_CopyBitmap(GSM_NokiaPictureImage, &Info->Entries[0].Bitmap->Bitmap[0], Buffer, Length);
		if (Info->Entries[0].Bitmap->Bitmap[0].Text[0]!=0 || Info->Entries[0].Bitmap->Bitmap[0].Text[1]!=0) {
			if (Info->UnicodeCoding) {
				Buffer[(*Length)++] = SM30_UNICODETEXT;
				/* Length for text part */
				Buffer[(*Length)++] = 0x00;
				Buffer[(*Length)++] = strlen(DecodeUnicodeString(Info->Entries[0].Bitmap->Bitmap[0].Text))*2;
				memcpy(Buffer+(*Length),Info->Entries[0].Bitmap->Bitmap[0].Text,strlen(DecodeUnicodeString(Info->Entries[0].Bitmap->Bitmap[0].Text))*2);
				*Length = *Length + strlen(DecodeUnicodeString(Info->Entries[0].Bitmap->Bitmap[0].Text))*2;
			} else {
				/*ID for ISO-8859-1 text*/
				Buffer[(*Length)++] = SM30_ISOTEXT;
				Buffer[(*Length)++] = 0x00;
				Buffer[(*Length)++] = strlen(DecodeUnicodeString(Info->Entries[0].Bitmap->Bitmap[0].Text));
				memcpy(Buffer+(*Length),DecodeUnicodeString(Info->Entries[0].Bitmap->Bitmap[0].Text),strlen(DecodeUnicodeString(Info->Entries[0].Bitmap->Bitmap[0].Text)));
				*Length = *Length +strlen(DecodeUnicodeString(Info->Entries[0].Bitmap->Bitmap[0].Text));
			}
		}
	}
}

static GSM_Error GSM_AddSMS_Text_UDH(GSM_MultiSMSMessage 	*SMS,
				     GSM_Coding_Type		Coding,
				     char 			*Buffer,
				     int			BufferLen,
				     bool 			UDH,
				     int 			*UsedText)
{
	int Pos,Free,Copy;

	switch (Coding) {
	case GSM_Coding_Default:
		FindDefaultAlphabetLen(SMS->SMS[SMS->Number].Text,&Pos,UsedText,500);
		break;
	case GSM_Coding_Unicode:
		*UsedText = strlen(DecodeUnicodeString(SMS->SMS[SMS->Number].Text))*2;
		break;
	case GSM_Coding_8bit:
		*UsedText = SMS->SMS[SMS->Number].Length;
		break;
	}
	Free = GSM_MAX_8BIT_SMS_LENGTH - SMS->SMS[SMS->Number].UDH.Length - 1 - *UsedText;
	dprintf("Free, Used %i %i\n",Free,*UsedText);
	if (UDH) {
		dprintf("Adding UDH\n");
		if (Free - BufferLen <= 0) {
			dprintf("Going to the new SMS\n");
			SMS->Number++;
			*UsedText = 0;
		}
		if (SMS->SMS[SMS->Number].UDH.Length == 0) {
			SMS->SMS[SMS->Number].UDH.Length  = 1;
			SMS->SMS[SMS->Number].UDH.Text[0] = 0x00;
		}
		memcpy(SMS->SMS[SMS->Number].UDH.Text+SMS->SMS[SMS->Number].UDH.Length,Buffer,BufferLen);
		SMS->SMS[SMS->Number].UDH.Length  	+= BufferLen;
		SMS->SMS[SMS->Number].UDH.Text[0] 	+= BufferLen;
		SMS->SMS[SMS->Number].UDH.Type 		=  UDH_UserUDH;
		dprintf("UDH added\n");
	} else {
		dprintf("Adding text\n");
		if (Free == 0) {
			dprintf("Going to the new SMS\n");
			SMS->Number++;
			*UsedText = 0;
			Free = GSM_MAX_8BIT_SMS_LENGTH - SMS->SMS[SMS->Number].UDH.Length;
		}

		Pos = 0;
		while (Pos != BufferLen) {
			Copy = Free;
			dprintf("Copying %i bytes\n",Copy);
			if (BufferLen - Pos < Free) Copy = BufferLen - Pos;
			dprintf("Copying %i bytes\n",Copy);
			switch (Coding) {
			case GSM_Coding_Default:
			case GSM_Coding_Unicode:
				SMS->SMS[SMS->Number].Text[strlen(DecodeUnicodeString(SMS->SMS[SMS->Number].Text))*2+Copy*2]   = 0;
				SMS->SMS[SMS->Number].Text[strlen(DecodeUnicodeString(SMS->SMS[SMS->Number].Text))*2+Copy*2+1] = 0;
				memcpy(SMS->SMS[SMS->Number].Text+strlen(DecodeUnicodeString(SMS->SMS[SMS->Number].Text))*2,Buffer+Pos*2,Copy*2);				
				break;
			case GSM_Coding_8bit:
				break;
			}
			Pos += Copy;
			if (Pos != BufferLen) {
				dprintf("Going to the new SMS\n");
				SMS->Number++;
				*UsedText = 0;
				Free = GSM_MAX_8BIT_SMS_LENGTH - SMS->SMS[SMS->Number].UDH.Length;
			}
		}
		dprintf("Text added\n");
	}
	dprintf("Position %i\n",*UsedText);
	return GE_NONE;
}
					
static GSM_Error GSM_EncodeEMSMultiPartSMS(GSM_EncodeMultiPartSMSInfo 	*Info,
				           GSM_MultiSMSMessage 		*SMS,
					   bool 			linked)
{
	unsigned char		Buffer[GSM_MAX_SMS_LENGTH*2*MAX_MULTI_SMS];
	int 			i,UsedText,j,Length,Width,Height;
	unsigned char		UDHID;
	GSM_Phone_Bitmap_Types	BitmapType;
	EncodeMultiPartSMSEntry *Entry;

	/* Cleaning on the start */
	for (i=0;i<MAX_MULTI_SMS;i++)
	{
		GSM_SetDefaultSMSData(&SMS->SMS[i]);

		SMS->SMS[i].UDH.Type = UDH_NoUDH;
		if (linked) SMS->SMS[i].UDH.Type = UDH_ConcatenatedMessages;
		GSM_EncodeUDHHeader(&SMS->SMS[i].UDH);
	}

	/* Packing */
	for (i=0;i<Info->EntriesNum;i++) {
		Entry = &Info->Entries[i];

		switch (Entry->ID) {
		case SMS_EMSPredefinedSound:
		case SMS_EMSPredefinedAnimation:
			if (Entry->ID == SMS_EMSPredefinedSound) {
				Buffer[0] = 0x0B;	/* ID for def.sound	*/
			} else {
				Buffer[0] = 0x0D; 	/* ID for def.animation	*/
			}
			Buffer[1] = 0x02; 		/* Length of rest 	*/
			Buffer[2] = 0x00; 		/* Position in EMS msg	*/
			Buffer[3] = Entry->Number; 	/* Number of anim.	*/
			GSM_AddSMS_Text_UDH(SMS,GSM_Coding_Default,Buffer,4,true,&UsedText);
			SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-2] = UsedText;
			break;
		case SMS_EMSSound10:
		case SMS_EMSSound12:
			Length = 128; /* 128 bytes is maximal length from specs */
			if (Entry->ID == SMS_EMSSound10) {
				Entry->RingtoneNotes = GSM_EncodeEMSSound(*Entry->Ringtone, Buffer+3, &Length, 1.0);
			} else {
				Entry->RingtoneNotes = GSM_EncodeEMSSound(*Entry->Ringtone, Buffer+3, &Length, 1.2);
			}

			Buffer[0] = 0x0C;	/* ID for EMS sound 	*/
			Buffer[1] = Length+1;	/* Length of rest 	*/
			Buffer[2] = 0x00; 	/* Position in EMS msg 	*/
			GSM_AddSMS_Text_UDH(SMS,GSM_Coding_Default,Buffer,Length+3,true,&UsedText);
			SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-Length-1] = UsedText;
			break;
		case SMS_EMSAnimation:
			if (Entry->Bitmap->Bitmap[0].Width > 8 || Entry->Bitmap->Bitmap[0].Height > 8) {
				BitmapType = GSM_EMSMediumPicture;	/* Bitmap 16x16 */
				Buffer[0]  = 0x0E;			/* ID for 16x16 animation */
			} else {		
				BitmapType = GSM_EMSSmallPicture;	/* Bitmap 8x8 */
				Buffer[0]  = 0x0F;			/* ID for 8x8 animation */
			}
			Length = PHONE_GetBitmapSize(BitmapType);
	
			Buffer[1] = Length*Entry->Bitmap->Number + 1;	/* Length of rest 	 */
			Buffer[2] = 0x00; 	 		 	/* Position in EMS msg	 */
			for (j=0;j<Entry->Bitmap->Number;j++) {
				PHONE_EncodeBitmap(BitmapType, Buffer+3+j*Length, &Entry->Bitmap->Bitmap[j]);
			}
			GSM_AddSMS_Text_UDH(SMS,GSM_Coding_Default,Buffer,3+Length*Entry->Bitmap->Number,true,&UsedText);
			SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-1-Length*Entry->Bitmap->Number] = UsedText;
			break;
		case SMS_EMSFixedBitmap:
			if (Entry->Bitmap->Bitmap[0].Width > 16 || Entry->Bitmap->Bitmap[0].Height > 16) {
				BitmapType = GSM_EMSBigPicture;		/* Bitmap 32x32 */
			} else if (Entry->Bitmap->Bitmap[0].Width > 8 || Entry->Bitmap->Bitmap[0].Height > 8) {
				BitmapType = GSM_EMSMediumPicture;	/* Bitmap 16x16 */
			} else {		
				BitmapType = GSM_EMSSmallPicture;	/* Bitmap 8x8 */
			}
			Length = PHONE_GetBitmapSize(BitmapType);
			PHONE_GetBitmapWidthHeight(BitmapType, &Width, &Height);
	
			Buffer[0] = 0x12;		/* ID for EMS bitmap	*/
			Buffer[1] = Length + 3;		/* Length of rest 	*/
			Buffer[2] = 0x00; 		/* Position in EMS msg	*/
			Buffer[3] = Width/8;		/* Bitmap width/8 	*/
			Buffer[4] = Height;		/* Bitmap height  	*/
			PHONE_EncodeBitmap(BitmapType,Buffer+5, &Entry->Bitmap->Bitmap[0]);
			GSM_AddSMS_Text_UDH(SMS,GSM_Coding_Default,Buffer,5+Length,true,&UsedText);
			SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-3-Length] = UsedText;
			break;
		case SMS_ConcatenatedTextLong:
			GSM_AddSMS_Text_UDH(SMS,GSM_Coding_Default,Entry->Buffer,strlen(DecodeUnicodeString(Entry->Buffer)),false,&UsedText);
		default:
			break;
		}
	}

	SMS->Number++;

	if (linked) {
		UDHID = GSM_MakeSMSIDFromTime();
		for (i=0;i<SMS->Number;i++)
		{
			SMS->SMS[i].UDH.Text[2+1] = UDHID;
			SMS->SMS[i].UDH.Text[3+1] = SMS->Number;
			SMS->SMS[i].UDH.Text[4+1] = i+1;
		}
	}

#ifdef DEBUG
	dprintf("SMS number is %i\n",SMS->Number);
	for (i=0;i<SMS->Number;i++) {
		dprintf("UDH length %i\n",SMS->SMS[i].UDH.Length);
		DumpMessage(di.df, SMS->SMS[i].UDH.Text, SMS->SMS[i].UDH.Length);
		dprintf("SMS length %i\n",strlen(DecodeUnicodeString(SMS->SMS[i].Text))*2);
		DumpMessage(di.df, SMS->SMS[i].Text, strlen(DecodeUnicodeString(SMS->SMS[i].Text))*2);
	}
#endif
	return GE_NONE;
}

GSM_Error GSM_EncodeMultiPartSMS(GSM_EncodeMultiPartSMSInfo	*Info,
			    	 GSM_MultiSMSMessage		*SMS)
{
	unsigned char	Buffer[GSM_MAX_SMS_LENGTH*2*MAX_MULTI_SMS];
	unsigned char	Buffer2[GSM_MAX_SMS_LENGTH*2*MAX_MULTI_SMS];
	int		Length = 0,smslen,i;
	GSM_Error	error;
	GSM_Coding_Type Coding 	= GSM_Coding_8bit;
	int		Class	= -1;
	GSM_UDH		UDH	= UDH_NoUDH;
	GSM_UDHHeader 	UDHHeader;
	bool		EMS	= false;

	SMS->Number = 0;

	for (i=0;i<Info->EntriesNum;i++) {
		switch (Info->Entries[i].ID) {
			case SMS_EMSPredefinedAnimation:
			case SMS_EMSPredefinedSound:
			case SMS_EMSSound10:
			case SMS_EMSSound12:
			case SMS_EMSFixedBitmap:
			case SMS_EMSAnimation:
				EMS = true;
			default:
				break;
		}
		if (EMS) break;
	}
	if (EMS) {
		error=GSM_EncodeEMSMultiPartSMS(Info,SMS,false);
		if (error != GE_NONE) return error;
		if (SMS->Number != 1) {
			SMS->Number = 0;
			error=GSM_EncodeEMSMultiPartSMS(Info,SMS,true);
		}
		return error;
	}
	if (Info->EntriesNum != 1) return GE_UNKNOWN;

	switch (Info->Entries[0].ID) {
	case SMS_NokiaRingtoneLong:
	case SMS_NokiaRingtone:
		UDH	= UDH_NokiaRingtone;
		Class	= 1;
		/* 6 = length of UDH_NokiaRingtone UDH header */
		Length = GSM_MAX_8BIT_SMS_LENGTH-6;
		Info->Entries[0].RingtoneNotes = GSM_EncodeNokiaRTTLRingtone(*Info->Entries[0].Ringtone,Buffer,&Length);
		if (Info->Entries[0].ID == SMS_NokiaRingtone) break;
		if (Info->Entries[0].RingtoneNotes != Info->Entries[0].Ringtone->NoteTone.NrCommands) {
			UDH = UDH_NokiaProfileLong;
			Length = 0;
			GSM_EncodeSMS30MultiPartSMS(Info,Buffer,&Length);
		}
		break;
	case SMS_NokiaOperatorLogoLong:
		if (Info->Entries[0].Bitmap->Bitmap[0].Width > 72 || Info->Entries[0].Bitmap->Bitmap[0].Height > 14) {
			UDH	= UDH_NokiaOperatorLogoLong;
			Class 	= 1;
			NOKIA_EncodeNetworkCode(Buffer, Info->Entries[0].Bitmap->Bitmap[0].NetworkCode);
			Length = Length + 3;
			NOKIA_CopyBitmap(GSM_Nokia7110OperatorLogo, &Info->Entries[0].Bitmap->Bitmap[0], Buffer, &Length);
			break;
		}
	case SMS_NokiaOperatorLogo:
		UDH	= UDH_NokiaOperatorLogo;
		Class 	= 1;
		NOKIA_EncodeNetworkCode(Buffer, Info->Entries[0].Bitmap->Bitmap[0].NetworkCode);
		Length = Length + 3;
		NOKIA_CopyBitmap(GSM_NokiaOperatorLogo, &Info->Entries[0].Bitmap->Bitmap[0], Buffer, &Length);
		break;
	case SMS_NokiaCallerLogo:
		UDH	= UDH_NokiaCallerLogo;
		Class 	= 1;
		NOKIA_CopyBitmap(GSM_NokiaCallerLogo, &Info->Entries[0].Bitmap->Bitmap[0], Buffer, &Length);
		break;
	case SMS_NokiaProfileLong:
	case SMS_NokiaPictureImageLong:
	case SMS_NokiaScreenSaverLong:
		Class   = 1;
		UDH	= UDH_NokiaProfileLong;
		GSM_EncodeSMS30MultiPartSMS(Info,Buffer,&Length);
		break;
	case SMS_NokiaWAPBookmarkLong:
		Class	= 1;
		NOKIA_EncodeWAPBookmarkSMSText(Buffer,&Length,*Info->Entries[0].Bookmark);
		/* 6 = length of UDH_NokiaWAPBookmark UDH header */
		if (Length>(GSM_MAX_8BIT_SMS_LENGTH-6)) {
			UDH=UDH_NokiaWAPBookmarkLong;
		} else {
			UDH=UDH_NokiaWAPBookmark;
		}
		break;
	case SMS_NokiaWAPSettingsLong:
		Class	= 1;
		UDH	= UDH_NokiaWAPSettingsLong;
		NOKIA_EncodeWAPSettingsSMSText(Buffer,&Length,*Info->Entries[0].Settings);
		break;
	case SMS_NokiaVCARD10Long:
		NOKIA_EncodeVCARD10SMSText(Buffer,&Length,*Info->Entries[0].Phonebook);
		/* is 1 SMS ? 8 = length of ..SCKE2 */
		if (Length<=GSM_MAX_SMS_LENGTH-8) {
			sprintf(Buffer,"//SCKE2 ");
			Length = 8;
			NOKIA_EncodeVCARD10SMSText(Buffer,&Length,*Info->Entries[0].Phonebook);
		} else {
			/* FIXME: It wasn't checked */
			UDH = UDH_NokiaPhonebookLong;
		}
		Coding = GSM_Coding_Default;
		memcpy(Buffer2,Buffer,Length);
		EncodeUnicode(Buffer,Buffer2,Length);
		break;
	case SMS_NokiaVCARD21Long:
		NOKIA_EncodeVCARD21SMSText(Buffer,&Length,*Info->Entries[0].Phonebook);
		/* Is 1 SMS ? 12 = length of ..SCKL23F4 */
		if (Length<=GSM_MAX_SMS_LENGTH-12) {
			sprintf(Buffer,"//SCKL23F4%c%c",13,10);
			Length = 12;
			NOKIA_EncodeVCARD21SMSText(Buffer,&Length,*Info->Entries[0].Phonebook);
		} else {
			UDH = UDH_NokiaPhonebookLong;
			/* Here can be also 8 bit coding */
		}
		Coding = GSM_Coding_Default;
		memcpy(Buffer2,Buffer,Length);
		EncodeUnicode(Buffer,Buffer2,Length);
		break;
	case SMS_NokiaVCALENDAR10Long:
		error=NOKIA_EncodeVCALENDAR10SMSText(Buffer,&Length,*Info->Entries[0].Calendar);
		if (error != GE_NONE) return error;
		/* Is 1 SMS ? 8 = length of ..SCKE4 */
		if (Length<=GSM_MAX_SMS_LENGTH-8) {
			sprintf(Buffer,"//SCKE4 ");  
			Length = 8;
			NOKIA_EncodeVCALENDAR10SMSText(Buffer,&Length,*Info->Entries[0].Calendar);
		} else {
			UDH = UDH_NokiaCalendarLong;
			/* can be here 8 bit coding ? */
		}
		Coding = GSM_Coding_Default;
		memcpy(Buffer2,Buffer,Length);
		EncodeUnicode(Buffer,Buffer2,Length);
		break;
	case SMS_DisableVoice:
	case SMS_DisableFax:
	case SMS_DisableEmail:
	case SMS_EnableVoice:
	case SMS_EnableFax:
	case SMS_EnableEmail:
	case SMS_VoidSMS:
	case SMS_Text:
		Class = Info->Class;
		switch (Info->Entries[0].ID) {
			case SMS_DisableVoice	: UDH = UDH_DisableVoice; break;
			case SMS_DisableFax	: UDH = UDH_DisableFax;	  break;
			case SMS_DisableEmail	: UDH = UDH_DisableEmail; break;
			case SMS_EnableVoice	: UDH = UDH_EnableVoice;  break;
			case SMS_EnableFax	: UDH = UDH_EnableFax; 	  break;
			case SMS_EnableEmail	: UDH = UDH_EnableEmail;  break;
			case SMS_VoidSMS	: UDH = UDH_VoidSMS;	  break;
			case SMS_Text		: UDH = UDH_NoUDH;	  break;
			default			:			  break;
		}
		UDHHeader.Type = UDH;
		GSM_EncodeUDHHeader(&UDHHeader);
		memcpy(Buffer,Info->Entries[0].Buffer,strlen(DecodeUnicodeString(Info->Entries[0].Buffer))*2);
		if (Info->UnicodeCoding) {
			Coding = GSM_Coding_Unicode;
			Length = strlen(DecodeUnicodeString(Info->Entries[0].Buffer));
			if (Length>70-UDHHeader.Length) Length = 70-UDHHeader.Length;
		} else {
			Coding = GSM_Coding_Default;
			FindDefaultAlphabetLen(Info->Entries[0].Buffer,&Length,&smslen,(GSM_MAX_8BIT_SMS_LENGTH-UDHHeader.Length)*8/7);
		}
		break;
	case SMS_ConcatenatedAutoTextLong:
		smslen = strlen(DecodeUnicodeString(Info->Entries[0].Buffer));
		memcpy(Buffer,Info->Entries[0].Buffer,smslen*2);
		EncodeDefault(Buffer2, Buffer, &smslen, true, NULL);
		DecodeDefault(Buffer,  Buffer2, smslen, true, NULL);
#ifdef DEBUG
		if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) {
			dprintf("Info->Entries[0].Buffer:\n");
			DumpMessage(di.df, Info->Entries[0].Buffer, strlen(DecodeUnicodeString(Info->Entries[0].Buffer))*2);
			dprintf("Buffer:\n");
			DumpMessage(di.df, Buffer, strlen(DecodeUnicodeString(Buffer))*2);
		}
#endif
		Info->UnicodeCoding = false;
		for (smslen=0;smslen<(int)(strlen(DecodeUnicodeString(Info->Entries[0].Buffer))*2);smslen++) {
			if (Info->Entries[0].Buffer[smslen] != Buffer[smslen]) {
				Info->UnicodeCoding = true;
				dprintf("Setting to Unicode %i\n",smslen);
				break;
			}
		}
		/* No break here - we go to the SMS_ConcatenatedTextLong */
	case SMS_ConcatenatedTextLong:
		Class = Info->Class;
		memcpy(Buffer,Info->Entries[0].Buffer,strlen(DecodeUnicodeString(Info->Entries[0].Buffer))*2);
		UDH = UDH_NoUDH;
		if (Info->UnicodeCoding) {
			Coding = GSM_Coding_Unicode;
			Length = strlen(DecodeUnicodeString(Info->Entries[0].Buffer));
			if (Length>70) UDH=UDH_ConcatenatedMessages;
		} else {
			Coding = GSM_Coding_Default;
			FindDefaultAlphabetLen(Info->Entries[0].Buffer,&Length,&smslen,5000);
			if (smslen>GSM_MAX_SMS_LENGTH) UDH=UDH_ConcatenatedMessages;
		}
	default:
		break;
	}
	GSM_MakeMultiPartSMS(SMS,Buffer,Length,UDH,Coding,Class,Info->ReplaceMessage);
	return GE_NONE;
}

void GSM_ClearMultiPartSMSInfo(GSM_EncodeMultiPartSMSInfo *Info)
{
	int i;

	for (i=0;i<MAX_MULTI_SMS;i++) {
		Info->Entries[i].Buffer		= NULL;
		Info->Entries[i].Ringtone	= NULL;
		Info->Entries[i].Bitmap		= NULL;
		Info->Entries[i].Phonebook	= NULL;
		Info->Entries[i].Bookmark	= NULL;
		Info->Entries[i].Settings	= NULL;
		Info->Entries[i].Calendar	= NULL;
	}
}

/* ----------------- Joining SMS from parts -------------------------------- */

bool GSM_DecodeMultiPartSMS(GSM_EncodeMultiPartSMSInfo	*Info,
			    GSM_MultiSMSMessage		*SMS)
{
	int 	i;
	char	Buffer[GSM_MAX_SMS_LENGTH*2*MAX_MULTI_SMS];
	int	Length = 0;
	bool 	RetVal = false, Bitmap = false, Ringtone = false, TextBuffer = false;

	if (SMS->SMS[0].UDH.Type == UDH_NokiaRingtone && SMS->Number == 1) {
		if (GSM_DecodeNokiaRTTLRingtone(Info->Entries[0].Ringtone, SMS->SMS[0].Text, SMS->SMS[0].Length)==GE_NONE)
		{
			RetVal 	 = true;
			Ringtone = true;
		}
	}
	if (SMS->SMS[0].UDH.Type == UDH_NokiaCallerLogo && SMS->Number == 1) {
		PHONE_DecodeBitmap(GSM_NokiaCallerLogo, SMS->SMS[0].Text+4, &Info->Entries[0].Bitmap->Bitmap[0]);
#ifdef DEBUG
		if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) GSM_PrintBitmap(di.df,&Info->Entries[0].Bitmap->Bitmap[0]);
#endif
		Bitmap = true;
		RetVal = true;
	}
	if (SMS->SMS[0].UDH.Type == UDH_NokiaOperatorLogo && SMS->Number == 1) {
		PHONE_DecodeBitmap(GSM_NokiaOperatorLogo, SMS->SMS[0].Text+7, &Info->Entries[0].Bitmap->Bitmap[0]);
		NOKIA_DecodeNetworkCode(SMS->SMS[0].Text, Info->Entries[0].Bitmap->Bitmap[0].NetworkCode);
#ifdef DEBUG
		if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) GSM_PrintBitmap(di.df,&Info->Entries[0].Bitmap->Bitmap[0]);
#endif
		Bitmap = true;
		RetVal = true;
	}

	if (SMS->SMS[0].UDH.Type == UDH_ConcatenatedMessages) {
		for (i=0;i<SMS->Number;i++) {
			switch (SMS->SMS[i].Coding) {
			case GSM_Coding_8bit:
				memcpy(Info->Entries[0].Buffer+Length,SMS->SMS[i].Text,SMS->SMS[i].Length);
				Length=Length+SMS->SMS[i].Length;
				break;
			case GSM_Coding_Unicode:
			case GSM_Coding_Default:
				memcpy(Info->Entries[0].Buffer+Length,SMS->SMS[i].Text,strlen(DecodeUnicodeString(SMS->SMS[i].Text))*2);
				Length=Length+strlen(DecodeUnicodeString(SMS->SMS[i].Text))*2;
				break;
			}
		}
		Info->Entries[0].Buffer[Length]		= 0;
		Info->Entries[0].Buffer[Length+1]	= 0;
		TextBuffer 	= true;
		RetVal 		= true;
	}

	/* Generally Smart Messaging 3.0 decoding */
	if (SMS->SMS[0].UDH.Type == UDH_NokiaProfileLong) {
		RetVal = true;
		for (i=0;i<SMS->Number;i++) {
			if (SMS->SMS[i].UDH.Type != UDH_NokiaProfileLong ||
			    SMS->SMS[i].UDH.Text[11] != i+1		 ||
			    SMS->SMS[i].UDH.Text[10] != SMS->Number)
			{
				return false;
			}
			memcpy(Buffer+Length,SMS->SMS[i].Text,SMS->SMS[i].Length);
			Length = Length + SMS->SMS[i].Length;
		}
		Info->Entries[0].Bitmap->Bitmap[0].Text[0] = 0;
		Info->Entries[0].Bitmap->Bitmap[0].Text[1] = 0;
		i=1;
		while (i!=Length) {
			switch (Buffer[i]) {
				case SM30_ISOTEXT:
					dprintf("ISO 8859-2 text\n");
					break;
				case SM30_UNICODETEXT:
					dprintf("Unicode text\n");
					memcpy(Info->Entries[0].Bitmap->Bitmap[0].Text,Buffer+i+3,Buffer[i+1]*256+Buffer[i+2]);
					Info->Entries[0].Bitmap->Bitmap[0].Text[Buffer[i+1]*256 + Buffer[i+2]] 	= 0;
					Info->Entries[0].Bitmap->Bitmap[0].Text[Buffer[i+1]*256 + Buffer[i+2]+ 1] 	= 0;
					dprintf("Unicode Text \"%s\"\n",DecodeUnicodeString(Info->Entries[0].Bitmap->Bitmap[0].Text));
					break;
				case SM30_OTA:
					dprintf("OTA bitmap as Picture Image\n");
					PHONE_DecodeBitmap(GSM_NokiaPictureImage, Buffer + i + 7, &Info->Entries[0].Bitmap->Bitmap[0]);
#ifdef DEBUG
					if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) GSM_PrintBitmap(di.df,&Info->Entries[0].Bitmap->Bitmap[0]);
#endif
					Bitmap = true;
					break;
				case SM30_RINGTONE:
					dprintf("RTTL ringtone\n");
					break;
				case SM30_PROFILENAME:
					dprintf("Profile Name\n");
					break;
				case SM30_SCREENSAVER:
					dprintf("OTA bitmap as Screen Saver\n");
					PHONE_DecodeBitmap(GSM_NokiaPictureImage, Buffer + i + 7, &Info->Entries[0].Bitmap->Bitmap[0]);
#ifdef DEBUG
					if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) GSM_PrintBitmap(di.df,&Info->Entries[0].Bitmap->Bitmap[0]);
#endif
					Bitmap = true;
					break;
			}
			i = i + Buffer[i+1]*256 + Buffer[i+2] + 3;
			dprintf("%i %i\n",i,Length);
		}
	}
	if (!Bitmap) 		Info->Entries[0].Bitmap 	= NULL;
	if (!TextBuffer) 	Info->Entries[0].Buffer 	= NULL;
	if (!Ringtone) 		Info->Entries[0].Ringtone 	= NULL;
	return RetVal;
}

GSM_Error GSM_SortSMS(GSM_MultiSMSMessage *INPUT[150], GSM_MultiSMSMessage *OUTPUT[150])
{
	bool			INPUTSorted[150],copyit;
	int			i,OUTPUTNum,j,z,loop;

	for (i=0;i<150;i++) INPUTSorted[i] = false;

	OUTPUTNum = 0;
	OUTPUT[0] = NULL;

	i=0;
	while (INPUT[i]!=NULL) {
		loop = true;
		/* If this one SMS was sorted earlier, do not touch */
		if (INPUTSorted[i]) {
			i++;
			loop=false;
		}
		if (loop) {
			/* If we have:
			 * - linked sms returned by phone driver
			 * - sms without linking
			 * - unknown sms
			 * we copy it to OUTPUT
			 */
			if (INPUT[i]->Number 			!= 1 	       ||
			    INPUT[i]->SMS[0].UDH.Type 		== UDH_NoUDH   ||
			    INPUT[i]->SMS[0].UDH.Type 		== UDH_UserUDH ||
			    INPUT[i]->SMS[0].UDH.PartNumber 	== -1) {
				OUTPUT[OUTPUTNum] = malloc(sizeof(GSM_MultiSMSMessage));
			        if (OUTPUT[OUTPUTNum] == NULL) return GE_MOREMEMORY;
				OUTPUT[OUTPUTNum+1] = NULL;

				memcpy(OUTPUT[OUTPUTNum],INPUT[i],sizeof(GSM_MultiSMSMessage));
				INPUTSorted[i]=true;
				OUTPUTNum++;
				i=0;
				loop = false;
			}
		}
		if (loop) {
			/* We have 1'st part of linked sms. It's single.
			 * We will try to find other parts
			 */
			if (INPUT[i]->SMS[0].UDH.PartNumber == 1) {
				OUTPUT[OUTPUTNum] = malloc(sizeof(GSM_MultiSMSMessage));
			        if (OUTPUT[OUTPUTNum] == NULL) return GE_MOREMEMORY;
				OUTPUT[OUTPUTNum+1] = NULL;

				memcpy(&OUTPUT[OUTPUTNum]->SMS[0],&INPUT[i]->SMS[0],sizeof(GSM_SMSMessage));
				OUTPUT[OUTPUTNum]->Number = 1;
				INPUTSorted[i]=true;
				j=1;
				while (j!=INPUT[i]->SMS[0].UDH.AllParts) {
					z=0;
					while(INPUT[z]!=NULL) {
						copyit=false;
						/* Look for
						 * - non sorted earlier
						 * - single
						 * - with the same UDH (with the same ID, etc.) to the first sms
						 */
						if (!INPUTSorted[z] 						   	&&
						    INPUT[z]->Number 		    == 1 			   	&&
						    INPUT[z]->SMS[0].UDH.Type 	    == INPUT[i]->SMS[0].UDH.Type 	&&
						    INPUT[z]->SMS[0].UDH.ID 	    == INPUT[i]->SMS[0].UDH.ID		&&
						    INPUT[z]->SMS[0].UDH.AllParts   == INPUT[i]->SMS[0].UDH.AllParts 	&&
						    INPUT[z]->SMS[0].UDH.PartNumber == j+1) {
							/* For SMS_Deliver compare also SMSC and Sender number */
							if (INPUT[z]->SMS[0].PDU == SMS_Deliver) {
								if (!strcmp(DecodeUnicodeString(INPUT[z]->SMS[0].SMSC.Number),DecodeUnicodeString(INPUT[i]->SMS[0].SMSC.Number)) &&
								    !strcmp(DecodeUnicodeString(INPUT[z]->SMS[0].Number),DecodeUnicodeString(INPUT[i]->SMS[0].Number))) {
									copyit=true;
								}
							} else copyit=true;
						}
						/* We found correct sms. Copy it */
						if (copyit) {
							memcpy(&OUTPUT[OUTPUTNum]->SMS[j],&INPUT[z]->SMS[0],sizeof(GSM_SMSMessage));
							OUTPUT[OUTPUTNum]->Number++;
							INPUTSorted[z]=true;
							break;
						}
						z++;
					}
					/* Incomplete sequence */
					if (OUTPUT[OUTPUTNum]->Number==j) break;
					j++;
				}
				OUTPUTNum++;
				i=0;
			}
		}
	}
	return GE_NONE;
}
