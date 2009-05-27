/* (c) 2001-2006 by Marcin Wiacek */
/* Based on some Pawel Kot and others work from Gnokii (www.gnokii.org)
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

#include <ctype.h>
#include <string.h>
#include <time.h>

#include <gammu-calendar.h>
#include <gammu-message.h>
#include <gammu-unicode.h>
#include <gammu-debug.h>

#include "../../misc/coding/coding.h"
#include "../../debug.h"

/* User data headers */
static GSM_UDHHeader UDHHeaders[] = {
	/* See GSM 03.40 section 9.2.3.24.1
	 * 1 byte 0x00
	 * 1 byte 0x03
	 * 1 byte 0x01: unique ID for message series
	 * 1 byte 0x00: how many SMS in sequence
	 * 1 byte 0x00: number of current SMS in sequence 		*/
	{ UDH_ConcatenatedMessages,     0x05, "\x00\x03\x01\x00\x00",2,-1,4,3},

	/* See GSM 03.40 section 9.2.3.24.2 for voice, fax and email messages */
	{ UDH_DisableVoice,             0x04, "\x01\x02\x00\x00",-1,-1,-1,-1},
	{ UDH_DisableFax,               0x04, "\x01\x02\x01\x00",-1,-1,-1,-1},
	{ UDH_DisableEmail,             0x04, "\x01\x02\x02\x00",-1,-1,-1,-1},
	{ UDH_EnableVoice,              0x04, "\x01\x02\x00\x01",-1,-1,-1,-1},
	{ UDH_EnableFax,                0x04, "\x01\x02\x01\x01",-1,-1,-1,-1},
	{ UDH_EnableEmail,              0x04, "\x01\x02\x02\x01",-1,-1,-1,-1},

	/* When send such SMS to some phones, they don't display anything,
	 * only beep and enable vibra/light
	 */
	{ UDH_VoidSMS,                  0x08, "\x01\x02\x02\x01\x01\x02\x02\x00",-1,-1,-1,-1},

	/* Nokia Smart Messaging (short version) UDH
	 * General format :
	 * 1 byte  0x05      : IEI application port addressing scheme, 16 bit address
	 * 1 byte  0x04      : IEI length
	 * 2 bytes           : destination address : high & low byte
	 * 2 bytes 0x00 0x00 : originator address  : high & low byte */
	{ UDH_NokiaRingtone,            0x06, "\x05\x04\x15\x81\x00\x00",-1,-1,-1,-1},
	{ UDH_NokiaOperatorLogo,        0x06, "\x05\x04\x15\x82\x00\x00",-1,-1,-1,-1},
	{ UDH_NokiaCallerLogo,          0x06, "\x05\x04\x15\x83\x00\x00",-1,-1,-1,-1},
	{ UDH_NokiaWAP,		        0x06, "\x05\x04\xc3\x4f\x00\x00",-1,-1,-1,-1},

	/* Nokia Smart Messaging (long version) UDH and other
	 * General format:
	 * 1 byte 0x05       : IEI application port addressing scheme, 16 bit address
	 * 1 byte 0x04       : IEI length
	 * 2 bytes 0x00 0x00 : destination address : high & low byte
	 * 2 bytes 0x00 0x00 : originator address  : high & low byte
	 * 1 byte 0x00       : SAR
	 * 1 byte 0x03       : SAR length
	 * 1 byte            : diagram reference number (unique ID for message series)
	 * 1 byte            : number of all SMS
	 * 1 byte            : number of current SMS */
	{ UDH_NokiaCalendarLong,        0x0b, "\x05\x04\x00\xe4\x00\x00\x00\x03\xc7\x00\x00",8,-1,10,9},
	{ UDH_MMSIndicatorLong,	        0x0b, "\x05\x04\x0b\x84\x23\xf0\x00\x03\xe5\x00\x00",8,-1,10,9},
	{ UDH_NokiaRingtoneLong,        0x0b, "\x05\x04\x15\x81\x00\x00\x00\x03\x01\x00\x00",8,-1,10,9},
	{ UDH_NokiaOperatorLogoLong,    0x0b, "\x05\x04\x15\x82\x00\x00\x00\x03\x02\x00\x00",8,-1,10,9},
	{ UDH_NokiaProfileLong,         0x0b, "\x05\x04\x15\x8a\x00\x00\x00\x03\xce\x00\x00",8,-1,10,9},
	{ UDH_NokiaPhonebookLong,       0x0b, "\x05\x04\x23\xf4\x00\x00\x00\x03\x01\x00\x00",8,-1,10,9},
	{ UDH_NokiaWAPLong,	        0x0b, "\x05\x04\xc3\x4f\x00\x00\x00\x03\x7f\x00\x00",8,-1,10,9},

	{ UDH_ConcatenatedMessages16bit,0x06, "\x08\x04\x00\x00\x00\x00",-1,2,5,4},

	{ UDH_NoUDH,                    0x00, "",-1,-1,-1,-1}
};

/* --------------------------- Unpacking SMS ------------------------------- */

/* See GSM 03.40 section 9.2.3.11 */
static GSM_Error GSM_DecodeSMSDateTime(GSM_Debug_Info *di, GSM_DateTime *DT, unsigned char *req)
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

	smfprintf(di, "Decoding date & time: %s\n", OSDateTime(*DT, true));

	return ERR_NONE;
}

void GSM_DecodeUDHHeader(GSM_Debug_Info *di, GSM_UDHHeader *UDH)
{
	int	i, tmp, w;
	bool	UDHOK;

	UDH->Type 	= UDH_UserUDH;
	UDH->ID8bit	= -1;
	UDH->ID16bit	= -1;
	UDH->PartNumber	= -1;
	UDH->AllParts	= -1;

	i=-1;
	while (UDHHeaders[++i].Type != UDH_NoUDH) {

		tmp=UDHHeaders[i].Length;
		/* if length is the same */
		if (tmp==UDH->Text[0]) {

			if (tmp==0x05) tmp=tmp-3;/*three last bytes can be different for such UDH*/
			if (tmp==0x0b) tmp=tmp-3;/*three last bytes can be different for such UDH*/
			if (tmp==0x06 && UDH->Text[1] == 0x08) tmp=tmp-4;

			UDHOK=true;
			for (w=0;w<tmp;w++) {
				if (UDHHeaders[i].Text[w]!=UDH->Text[w+1]) {
					UDHOK=false;
					break;
				}
			}
			if (UDHOK) {
				UDH->Type=UDHHeaders[i].Type;

				if (UDHHeaders[i].ID8bit	!=-1) UDH->ID8bit 	= UDH->Text[UDHHeaders[i].ID8bit+1];
				if (UDHHeaders[i].ID16bit	!=-1) UDH->ID16bit 	= UDH->Text[UDHHeaders[i].ID16bit+1]*256+UDH->Text[UDHHeaders[i].ID16bit+2];
				if (UDHHeaders[i].PartNumber	!=-1) UDH->PartNumber 	= UDH->Text[UDHHeaders[i].PartNumber+1];
				if (UDHHeaders[i].AllParts	!=-1) UDH->AllParts 	= UDH->Text[UDHHeaders[i].AllParts+1];
				break;
			}
		}
	}

#ifdef DEBUG
	smfprintf(di, "Type of UDH: ");
	switch (UDH->Type) {
	case UDH_ConcatenatedMessages      : smfprintf(di, "Concatenated (linked) message"); 		break;
	case UDH_ConcatenatedMessages16bit : smfprintf(di, "Concatenated (linked) message"); 		break;
	case UDH_DisableVoice		   : smfprintf(di, "Disables voice indicator");	 	break;
	case UDH_EnableVoice		   : smfprintf(di, "Enables voice indicator");	 	break;
	case UDH_DisableFax		   : smfprintf(di, "Disables fax indicator");	 		break;
	case UDH_EnableFax		   : smfprintf(di, "Enables fax indicator");	 		break;
	case UDH_DisableEmail		   : smfprintf(di, "Disables email indicator");	 	break;
	case UDH_EnableEmail		   : smfprintf(di, "Enables email indicator");	 	break;
	case UDH_VoidSMS		   : smfprintf(di, "Void SMS");			 	break;
	case UDH_NokiaWAP		   : smfprintf(di, "Nokia WAP Bookmark");		 	break;
	case UDH_NokiaOperatorLogoLong	   : smfprintf(di, "Nokia operator logo");		 	break;
	case UDH_NokiaWAPLong		   : smfprintf(di, "Nokia WAP Bookmark or WAP/MMS Settings");	break;
	case UDH_NokiaRingtone		   : smfprintf(di, "Nokia ringtone");		 		break;
	case UDH_NokiaRingtoneLong	   : smfprintf(di, "Nokia ringtone");		 		break;
	case UDH_NokiaOperatorLogo	   : smfprintf(di, "Nokia GSM operator logo");	 	break;
	case UDH_NokiaCallerLogo	   : smfprintf(di, "Nokia caller logo");		 	break;
	case UDH_NokiaProfileLong	   : smfprintf(di, "Nokia profile");		 		break;
	case UDH_NokiaCalendarLong	   : smfprintf(di, "Nokia calendar note");		 	break;
	case UDH_NokiaPhonebookLong	   : smfprintf(di, "Nokia phonebook entry");	 		break;
	case UDH_UserUDH		   : smfprintf(di, "User UDH");			 	break;
	case UDH_MMSIndicatorLong	   : smfprintf(di, "MMS indicator");		 		break;
	case UDH_NoUDH:								 		break;
	}
	if (UDH->ID8bit != -1) smfprintf(di, ", ID 8 bit %i",UDH->ID8bit);
	if (UDH->ID16bit != -1) smfprintf(di, ", ID 16 bit %i",UDH->ID16bit);
	if (UDH->PartNumber != -1 && UDH->AllParts != -1) {
		smfprintf(di, ", part %i of %i",UDH->PartNumber,UDH->AllParts);
	}
	smfprintf(di, "\n");
	DumpMessageText(di, UDH->Text, UDH->Length);
#endif
}

GSM_Coding_Type GSM_GetMessageCoding(GSM_Debug_Info *di, const char TPDCS) {

	/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme) and GSM 03.38 section 4 */
	if ((TPDCS & 0xC0) == 0) {
		/* bits 7..4 set to 00xx */
		if ((TPDCS & 0xC) == 0xC) {
			smfprintf(di, "WARNING: reserved alphabet value in TPDCS\n");
		} else {
			if (TPDCS == 0) 			return SMS_Coding_Default_No_Compression;
			if ((TPDCS & 0x2C) == 0x00) 	return SMS_Coding_Default_No_Compression;
			if ((TPDCS & 0x2C) == 0x20) 	return SMS_Coding_Default_Compression;
			if ((TPDCS & 0x2C) == 0x08) 	return SMS_Coding_Unicode_No_Compression;
			if ((TPDCS & 0x2C) == 0x28) 	return SMS_Coding_Unicode_Compression;
		}
	} else if ((TPDCS & 0xF0) >= 0x40 &&
		   (TPDCS & 0xF0) <= 0xB0) {
		/* bits 7..4 set to 0100 ... 1011 */
		smfprintf(di, "WARNING: reserved coding group in TPDCS\n");
	} else if (((TPDCS & 0xF0) == 0xC0) ||
	      	   ((TPDCS & 0xF0) == 0xD0)) {
		/* bits 7..4 set to 1100 or 1101 */
		if ((TPDCS & 4) == 4) {
			smfprintf(di, "WARNING: set reserved bit 2 in TPDCS\n");
		} else {
			return SMS_Coding_Default_No_Compression;
		}
	} else if ((TPDCS & 0xF0) == 0xE0) {
		/* bits 7..4 set to 1110 */
		if ((TPDCS & 4) == 4) {
			smfprintf(di, "WARNING: set reserved bit 2 in TPDCS\n");
		} else {
			return SMS_Coding_Unicode_No_Compression;
		}
	} else if ((TPDCS & 0xF0) == 0xF0) {
		/* bits 7..4 set to 1111 */
		if ((TPDCS & 8) == 8) {
			smfprintf(di, "WARNING: set reserved bit 3 in TPDCS\n");
		} else {
			if ((TPDCS & 4) == 0) return SMS_Coding_Default_No_Compression;
		}
	}
	return 0;
}

GSM_Error GSM_DecodeSMSFrameText(GSM_Debug_Info *di, GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout)
{
	int		off=0;	 	/* length of the User Data Header */
	int 		w,i;
	unsigned char	output[161];

	SMS->UDH.Length = 0;
	/* UDH header available */
	if (buffer[Layout.firstbyte] & 64) {
		/* Length of UDH header */
		off = (buffer[Layout.Text] + 1);
		SMS->UDH.Length = off;
		smfprintf(di, "UDH header available (length %i)\n",off);

		/* Copy UDH header into SMS->UDH */
		for (i = 0; i < off; i++) SMS->UDH.Text[i] = buffer[Layout.Text + i];

		GSM_DecodeUDHHeader(di, &SMS->UDH);
	}

	switch (SMS->Coding) {
		case SMS_Coding_Default_No_Compression:
			i = 0;
			do {
				i+=7;
				w=(i-off)%i;
			} while (w<0);
			SMS->Length=buffer[Layout.TPUDL] - (off*8 + w) / 7;
			if (SMS->Length < 0) {
				smfprintf(di, "No SMS text!\n");
				SMS->Length = 0;
				break;
			}
			GSM_UnpackEightBitsToSeven(w, buffer[Layout.TPUDL]-off, SMS->Length, buffer+(Layout.Text+off), output);
			smfprintf(di, "7 bit SMS, length %i\n",SMS->Length);
			DecodeDefault (SMS->Text, output, SMS->Length, true, NULL);
			smfprintf(di, "%s\n",DecodeUnicodeString(SMS->Text));
			break;
		case SMS_Coding_8bit:
			SMS->Length=buffer[Layout.TPUDL] - off;
			memcpy(SMS->Text,buffer+(Layout.Text+off),SMS->Length);
#ifdef DEBUG
			smfprintf(di, "8 bit SMS, length %i\n",SMS->Length);
			DumpMessageText(di, SMS->Text, SMS->Length);
#endif
			break;
		case SMS_Coding_Unicode_No_Compression:
			SMS->Length=(buffer[Layout.TPUDL] - off) / 2;
			DecodeUnicodeSpecialNOKIAChars(SMS->Text,buffer+(Layout.Text+off), SMS->Length);
#ifdef DEBUG
			smfprintf(di, "Unicode SMS, length %i\n",SMS->Length);
			DumpMessageText(di, buffer+(Layout.Text+off), SMS->Length*2);
			smfprintf(di, "%s\n",DecodeUnicodeString(SMS->Text));
#endif
			break;
		default:
			SMS->Length=0;
			break;
	}

	return ERR_NONE;
}

GSM_Error GSM_DecodeSMSStatusReportData(GSM_Debug_Info *di, GSM_SMSMessage *SMS, int TP_ST)
{
	SMS->DeliveryStatus 	= TP_ST;
	SMS->Coding 		= SMS_Coding_Unicode_No_Compression;

	if (TP_ST < 0x03) {
		EncodeUnicode(SMS->Text,"Delivered",9);
		SMS->Length = 9;
	} else if (TP_ST & 0x40) {
		EncodeUnicode(SMS->Text,"Failed",6);
		SMS->Length = 6;
	} else if (TP_ST & 0x20) {
		EncodeUnicode(SMS->Text,"Pending",7);
		SMS->Length = 7;
	} else {
		EncodeUnicode(SMS->Text,"Unknown",7);
		SMS->Length = 7;
	}

#ifdef DEBUG
	/* See GSM 03.40 section 9.2.3.15 (TP-Status) */
	if (TP_ST & 0x40) {
		if (TP_ST & 0x20) {
			/* 0x60, 0x61, ... */
			smfprintf(di, "Temporary error, SC is not making any more transfer attempts\n");
		} else {
			/* 0x40, 0x41, ... */
     			smfprintf(di, "Permanent error, SC is not making any more transfer attempts\n");
		}
    	} else if (TP_ST & 0x20) {
		/* 0x20, 0x21, ... */
		smfprintf(di, "Temporary error, SC still trying to transfer SM\n");
	}
	switch (TP_ST) {
	case 0x00: smfprintf(di, "SM received by the SME");					break;
	case 0x01: smfprintf(di, "SM forwarded by the SC to the SME but the SC is unable to confirm delivery");break;
	case 0x02: smfprintf(di, "SM replaced by the SC");					break;
	case 0x20: smfprintf(di, "Congestion");						break;
	case 0x21: smfprintf(di, "SME busy");						break;
	case 0x22: smfprintf(di, "No response from SME");					break;
	case 0x23: smfprintf(di, "Service rejected");					break;
	case 0x24: smfprintf(di, "Quality of service not available");			break;
	case 0x25: smfprintf(di, "Error in SME");						break;
        case 0x40: smfprintf(di, "Remote procedure error");					break;
        case 0x41: smfprintf(di, "Incompatibile destination");				break;
        case 0x42: smfprintf(di, "Connection rejected by SME");				break;
        case 0x43: smfprintf(di, "Not obtainable");						break;
        case 0x44: smfprintf(di, "Quality of service not available");			break;
        case 0x45: smfprintf(di, "No internetworking available");				break;
        case 0x46: smfprintf(di, "SM Validity Period Expired");				break;
        case 0x47: smfprintf(di, "SM deleted by originating SME");				break;
        case 0x48: smfprintf(di, "SM Deleted by SC Administration");			break;
        case 0x49: smfprintf(di, "SM does not exist");					break;
        case 0x60: smfprintf(di, "Congestion");						break;
        case 0x61: smfprintf(di, "SME busy");						break;
        case 0x62: smfprintf(di, "No response from SME");					break;
        case 0x63: smfprintf(di, "Service rejected");					break;
        case 0x64: smfprintf(di, "Quality of service not available");			break;
        case 0x65: smfprintf(di, "Error in SME");						break;
        default  : smfprintf(di, "Reserved/Specific to SC: %x",TP_ST);	break;
	}
	smfprintf(di, "\n");
#endif

	return ERR_NONE;
}

GSM_Error GSM_DecodeSMSFrameStatusReportData(GSM_Debug_Info *di, GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout)
{
	return GSM_DecodeSMSStatusReportData(di, SMS, buffer[Layout.TPStatus]);
}

GSM_Error GSM_DecodePDUFrame(GSM_Debug_Info *di, GSM_SMSMessage *SMS, unsigned char *buffer, size_t length, size_t *final_pos, bool SMSC)
{
	size_t pos = 0;
	int type;
	int vpf = 0;
	int rp = 0;
	int udh = 0;
	int i,w;
	unsigned char	output[161];
	int			datalength;

	/* Set some sane data */
	GSM_SetDefaultReceivedSMSData(SMS);

	/* Parse SMSC if it is included */
	if (SMSC) {
		pos += GSM_UnpackSemiOctetNumber(di, SMS->SMSC.Number, buffer + pos, false);
		smfprintf(di, "SMS center number : \"%s\"\n",DecodeUnicodeString(SMS->SMSC.Number));
	}

	/* Message type */
	type = buffer[pos];
	pos++;
	switch (type & 0x3) {
		case 0:
			smfprintf(di, "SMS type: Deliver");
			SMS->PDU = SMS_Deliver;
			break;
		case 1:
			smfprintf(di, "SMS type: Submit");
			SMS->PDU = SMS_Submit;
			break;
		case 2:
			smfprintf(di, "SMS type: Status report");
			SMS->PDU = SMS_Status_Report;
			break;
		case 3:
			smfprintf(di, "SMS type: Reserverd, aborting!\n");
			return ERR_UNKNOWN;;
	}

	if (SMS->PDU == SMS_Submit || SMS->PDU == SMS_Deliver) {
		if (type & (1 << 7)) {
			smfprintf(di, ", Reply path set");
			rp = 1;
		}
		if (type & (1 << 6)) {
			smfprintf(di, ", UDH included");
			udh = 1;
		}
	}
	if (SMS->PDU == SMS_Submit) {
		vpf = (type & (0x3 << 3)) >> 3;
		switch (vpf) {
			case 0:
				smfprintf(di, ", No VP");
				break;
			case 1:
				smfprintf(di, ", Reserved VP!\n");
				return ERR_UNKNOWN;
			case 2:
				smfprintf(di, ", Relative VP");
				break;
			case 3:
				smfprintf(di, ", Absolute VP");
				break;
		}
	}
	smfprintf(di, "\n");

	/* Message reference */
	if (SMS->PDU == SMS_Submit || SMS->PDU == SMS_Status_Report) {
		SMS->MessageReference = buffer[pos];
		smfprintf(di, "SMS MR: 0x%02X\n", SMS->MessageReference);
		pos++;
		if (pos >= length) {
			smfprintf(di, "Ran out of buffer when parsing PDU!\n");
			return ERR_CORRUPTED;
		}
	}

	/* Remote number */
	pos += GSM_UnpackSemiOctetNumber(di, SMS->Number, buffer + pos, true);
	smfprintf(di, "Remote number : \"%s\"\n",DecodeUnicodeString(SMS->Number));
	if (pos >= length) {
		smfprintf(di, "Ran out of buffer when parsing PDU!\n");
		return ERR_CORRUPTED;
	}


	if (SMS->PDU == SMS_Submit || SMS->PDU == SMS_Deliver) {
		/* Protocol identifier */
		smfprintf(di, "SMS PID: 0x%02X\n", buffer[pos]);
		if (buffer[pos] > 0x40 && buffer[pos] < 0x48) {
			SMS->ReplaceMessage = buffer[pos];
		}
		pos++;
		if (pos >= length) {
			smfprintf(di, "Ran out of buffer when parsing PDU!\n");
			return ERR_CORRUPTED;
		}

		/* Data coding scheme */
		smfprintf(di, "SMS DCS: 0x%02X\n", buffer[pos]);
		SMS->Coding = GSM_GetMessageCoding(di, buffer[pos]);

		/* Message class */
		SMS->Class = -1;
		/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme) and GSM 03.38 section 4 */
		if ((buffer[pos] & 0xD0) == 0x10 || (buffer[pos] & 0xF0) == 0xF0) {
			SMS->Class = buffer[pos] & 3;
		}
		smfprintf(di, "SMS class: %i\n",SMS->Class);

		pos++;
		if (pos >= length) {
			smfprintf(di, "Ran out of buffer when parsing PDU!\n");
			return ERR_CORRUPTED;
		}
	}

	/* SMSC time stamp */
	if (SMS->PDU == SMS_Status_Report || SMS->PDU == SMS_Deliver) {
		GSM_DecodeSMSDateTime(di, &SMS->DateTime, buffer + pos);
		pos += 7;
		if (pos >= length) {
			smfprintf(di, "Ran out of buffer when parsing PDU!\n");
			return ERR_CORRUPTED;
		}
	}

	if (SMS->PDU == SMS_Status_Report) {
		/* Discharge Time */
		GSM_DecodeSMSDateTime(di, &SMS->SMSCTime, buffer + pos);
		pos += 7;
		if (pos >= length) {
			smfprintf(di, "Ran out of buffer when parsing PDU!\n");
			return ERR_CORRUPTED;
		}

		/* Status */
		GSM_DecodeSMSStatusReportData(di, SMS, buffer[pos]);
		pos++;
		if (pos >= length) {
			smfprintf(di, "Ran out of buffer when parsing PDU!\n");
			return ERR_CORRUPTED;
		}
	}

	/* Validity period */
	if (SMS->PDU == SMS_Submit) {
		if (vpf == 2) {
			SMS->SMSC.Validity.Format = SMS_Validity_RelativeFormat;
			SMS->SMSC.Validity.Relative = buffer[pos];
			smfprintf(di, "Relative validity: 0x%02X\n", buffer[pos]);
			pos++;
			if (pos >= length) {
				smfprintf(di, "Ran out of buffer when parsing PDU!\n");
				return ERR_CORRUPTED;
			}
		} else if (vpf == 3) {
			/* @todo TODO: handle absolute validity */
			smfprintf(di, "Absolute validity not handled!\n");
			pos += 7;
			if (pos >= length) {
				smfprintf(di, "Ran out of buffer when parsing PDU!\n");
				return ERR_CORRUPTED;
			}
		}
	}

	/* Data */
	if (SMS->PDU == SMS_Submit || SMS->PDU == SMS_Deliver) {
		datalength = buffer[pos];
		if (SMS->Coding == SMS_Coding_Default_No_Compression) {
			datalength = (datalength * 7) / 8;
			if ((buffer[pos] * 7) % 8 != 0) {
				datalength++;
			}
		}
		if (pos + datalength >= length) {
			smfprintf(di, "Ran out of buffer when parsing PDU!\n");
			return ERR_CORRUPTED;
		}
		if (final_pos != NULL) {
			*final_pos = pos + datalength + 1;
		}
		SMS->UDH.Length = 0;
		/* UDH header available */
		if (udh) {
			/* Length of UDH header */
			SMS->UDH.Length = (buffer[pos + 1] + 1);
			smfprintf(di, "UDH header available (length %i)\n",SMS->UDH.Length);

			/* Copy UDH header into SMS->UDH */
			for (i = 0; i < SMS->UDH.Length; i++) SMS->UDH.Text[i] = buffer[pos + 1 + i];

			GSM_DecodeUDHHeader(di, &SMS->UDH);
		}

		switch (SMS->Coding) {
			case SMS_Coding_Default_No_Compression:
				i = 0;
				do {
					i+=7;
					w=(i-SMS->UDH.Length)%i;
				} while (w<0);
				SMS->Length=buffer[pos] - (SMS->UDH.Length*8 + w) / 7;
				if (SMS->Length < 0) {
					smfprintf(di, "No SMS text!\n");
					SMS->Length = 0;
					break;
				}
				GSM_UnpackEightBitsToSeven(w, buffer[pos]-SMS->UDH.Length, SMS->Length, buffer+(pos + 1+SMS->UDH.Length), output);
				smfprintf(di, "7 bit SMS, length %i\n",SMS->Length);
				DecodeDefault (SMS->Text, output, SMS->Length, true, NULL);
				smfprintf(di, "%s\n",DecodeUnicodeString(SMS->Text));
				break;
			case SMS_Coding_8bit:
				SMS->Length=buffer[pos] - SMS->UDH.Length;
				memcpy(SMS->Text,buffer+(pos + 1+SMS->UDH.Length),SMS->Length);
#ifdef DEBUG
				smfprintf(di, "8 bit SMS, length %i\n",SMS->Length);
				DumpMessageText(di, SMS->Text, SMS->Length);
#endif
				break;
			case SMS_Coding_Unicode_No_Compression:
				SMS->Length=(buffer[pos] - SMS->UDH.Length) / 2;
				DecodeUnicodeSpecialNOKIAChars(SMS->Text,buffer+(pos + 1+SMS->UDH.Length), SMS->Length);
#ifdef DEBUG
				smfprintf(di, "Unicode SMS, length %i\n",SMS->Length);
				DumpMessageText(di, buffer+(pos + 1+SMS->UDH.Length), SMS->Length*2);
				smfprintf(di, "%s\n",DecodeUnicodeString(SMS->Text));
#endif
				break;
			default:
				SMS->Length=0;
				break;
		}
	} else {
		if (final_pos != NULL) {
			*final_pos = pos;
		}
	}

	return ERR_NONE;
}

GSM_Error GSM_DecodeSMSFrame(GSM_Debug_Info *di, GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout)
{
	GSM_DateTime	zerodt = {0,0,0,0,0,0,0};
#ifdef DEBUG
	if (Layout.firstbyte == 255) {
		smfprintf(di, "ERROR: firstbyte in SMS layout not set\n");
		return ERR_UNKNOWN;
	}
	if (Layout.TPDCS     != 255) smfprintf(di, "TPDCS     : %02x %i\n",buffer[Layout.TPDCS]    ,buffer[Layout.TPDCS]);
	if (Layout.TPMR      != 255) smfprintf(di, "TPMR      : %02x %i\n",buffer[Layout.TPMR]     ,buffer[Layout.TPMR]);
	if (Layout.TPPID     != 255) smfprintf(di, "TPPID     : %02x %i\n",buffer[Layout.TPPID]    ,buffer[Layout.TPPID]);
	if (Layout.TPUDL     != 255) smfprintf(di, "TPUDL     : %02x %i\n",buffer[Layout.TPUDL]    ,buffer[Layout.TPUDL]);
	if (Layout.firstbyte != 255) smfprintf(di, "FirstByte : %02x %i\n",buffer[Layout.firstbyte],buffer[Layout.firstbyte]);
	if (Layout.Text      != 255 && Layout.TPUDL     != 255 && buffer[Layout.TPUDL] > 0) {
		smfprintf(di, "Text      : %02x %i\n",buffer[Layout.Text]     ,buffer[Layout.Text]);
	}
#endif

	GSM_SetDefaultReceivedSMSData(SMS);

	if (Layout.SMSCNumber!=255) {
		GSM_UnpackSemiOctetNumber(di, SMS->SMSC.Number,buffer+Layout.SMSCNumber,false);
		smfprintf(di, "SMS center number : \"%s\"\n",DecodeUnicodeString(SMS->SMSC.Number));
	}
	if ((buffer[Layout.firstbyte] & 0x80)!=0) SMS->ReplyViaSameSMSC=true;
#ifdef DEBUG
	if (SMS->ReplyViaSameSMSC) smfprintf(di, "SMS centre set for reply\n");
#endif
	if (Layout.Number!=255) {
		GSM_UnpackSemiOctetNumber(di, SMS->Number,buffer+Layout.Number,true);
		smfprintf(di, "Remote number : \"%s\"\n",DecodeUnicodeString(SMS->Number));
	}
	if (Layout.TPDCS != 255) {
		/* Get message coding */
		SMS->Coding = GSM_GetMessageCoding(di, buffer[Layout.TPDCS]);
	}
	if (Layout.Text != 255 && Layout.TPDCS!=255 && Layout.TPUDL!=255 && buffer[Layout.TPUDL] > 0) {
		GSM_DecodeSMSFrameText(di, SMS, buffer, Layout);
	}
	if (Layout.DateTime != 255) {
		GSM_DecodeSMSDateTime(di, &SMS->DateTime,buffer+(Layout.DateTime));
	} else {
		SMS->DateTime = zerodt;
	}
	if (Layout.SMSCTime != 255 && Layout.TPStatus != 255) {
		/* GSM 03.40 section 9.2.3.11 (TP-Service-Centre-Time-Stamp) */
		smfprintf(di, "SMSC response date: ");
		GSM_DecodeSMSDateTime(di, &SMS->SMSCTime, buffer+(Layout.SMSCTime));
		GSM_DecodeSMSFrameStatusReportData(di, SMS,buffer,Layout);
	} else {
		SMS->SMSCTime = zerodt;
	}
	SMS->Class = -1;
	if (Layout.TPDCS != 255) {
		/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme) and GSM 03.38 section 4 */
		if ((buffer[Layout.TPDCS] & 0xD0) == 0x10) {
			/* bits 7..4 set to 00x1 */
			if ((buffer[Layout.TPDCS] & 0xC) == 0xC) {
				smfprintf(di, "WARNING: reserved alphabet value in TPDCS\n");
			} else {
				SMS->Class = (buffer[Layout.TPDCS] & 3);
			}
		} else if ((buffer[Layout.TPDCS] & 0xF0) == 0xF0) {
			/* bits 7..4 set to 1111 */
			if ((buffer[Layout.TPDCS] & 8) == 8) {
				smfprintf(di, "WARNING: set reserved bit 3 in TPDCS\n");
			} else {
				SMS->Class = (buffer[Layout.TPDCS] & 3);
			}
		}
	}
	smfprintf(di, "SMS class: %i\n",SMS->Class);

	SMS->MessageReference = 0;
	if (Layout.TPMR != 255) SMS->MessageReference = buffer[Layout.TPMR];

	SMS->ReplaceMessage = 0;
	if (Layout.TPPID != 255) {
		if (buffer[Layout.TPPID] > 0x40 && buffer[Layout.TPPID] < 0x48) {
			SMS->ReplaceMessage = buffer[Layout.TPPID] - 0x40;
		}
	}
	SMS->RejectDuplicates = false;
	if ((buffer[Layout.firstbyte] & 0x04)==0x04) SMS->RejectDuplicates = true;

	return ERR_NONE;
}

/* ----------------------------- Packing SMS ------------------------------- */

/* See GSM 03.40 section 9.2.3.11 */
static GSM_Error GSM_EncodeSMSDateTime(GSM_Debug_Info *di, GSM_DateTime *DT, unsigned char *req)
{
	int Year;

	smfprintf(di, "Encoding SMS datetime: %s\n", OSDate(*DT));

	/* We need to have only two last digits of year */
	if (DT->Year>1900) {
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

	return ERR_NONE;
}

static int GSM_EncodeSMSFrameText(GSM_Debug_Info *di, GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout)
{
	int	off = 0;	/*  length of the User Data Header */
	int	size = 0, size2 = 0, w;
	size_t p;
	char	buff[200];

	if (SMS->UDH.Type!=UDH_NoUDH) {
		buffer[Layout.firstbyte] |= 0x40;			/* GSM 03.40 section 9.2.3.23 (TP-User-Data-Header-Indicator) */
		off = 1 + SMS->UDH.Text[0];				/* off - length of the User Data Header */
		memcpy(buffer+Layout.Text, SMS->UDH.Text, off);		/* we copy the udh */
#ifdef DEBUG
		smfprintf(di, "UDH, length %i\n",off);
		DumpMessageText(di, SMS->UDH.Text, off);
#endif
	}
	switch (SMS->Coding) {
		case SMS_Coding_8bit:
			/* the mask for the 8-bit data */
			/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme)
			 * and GSM 03.38 section 4 */
			buffer[Layout.TPDCS] |= (1 << 2);
			memcpy(buffer+(Layout.Text+off), SMS->Text, SMS->Length);
			size2 = size = SMS->Length+off;
#ifdef DEBUG
			smfprintf(di, "8 bit SMS, length %i\n",SMS->Length);
			DumpMessageText(di, SMS->Text, SMS->Length);
#endif
			break;
		case SMS_Coding_Default_No_Compression:
			p = 0;
			do {
				p+=7;
				w=(p-off)%p;
			} while (w<0);
			p = UnicodeLength(SMS->Text);
			EncodeDefault(buff, SMS->Text, &p, true, NULL);
			size = GSM_PackSevenBitsToEight(w, buff, buffer+(Layout.Text+off), p);
			size += off;
			size2 = (off*8 + w) / 7 + p;
			smfprintf(di, "7 bit SMS, length %i, %i\n",size,size2);
			smfprintf(di, "%s\n",DecodeUnicodeString(SMS->Text));
			if (size > GSM_MAX_8BIT_SMS_LENGTH) {
				size = 0; size2 = 0;
			}
			break;
		case SMS_Coding_Unicode_No_Compression:
			/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme)
			 * and GSM 03.38 section 4 */
			buffer[Layout.TPDCS] |= (1 << 3);
			EncodeUnicodeSpecialNOKIAChars(buffer+(Layout.Text+off), SMS->Text, UnicodeLength(SMS->Text));
			size=size2=UnicodeLength(buffer+(Layout.Text+off))*2+off;
#ifdef DEBUG
			smfprintf(di, "Unicode SMS, length %i\n",(size2-off)/2);
			DumpMessageText(di, buffer+(Layout.Text+off), size2-off);
			smfprintf(di, "%s\n",DecodeUnicodeString(buffer+(Layout.Text+off)));
#endif
			break;
		default:
			break;
	}

	/* GSM 03.40 section 9.2.3.16 (TP-User-Data-Length)
	 * SMS->Length is:
	   - integer representation of the number od octets within the
             user data when TP-User-Data is coded using 8 bit data
  	   - the sum of the number of septets in UDH including any padding
	     and the number of septets in TP-User-Data in other case
	 */
	buffer[Layout.TPUDL] = size2;
	return size;
}

GSM_Error GSM_EncodeSMSFrame(GSM_Debug_Info *di, GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout, int *length, bool clear)
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
			/* Set when want delivery report from SMSC */
			buffer[Layout.firstbyte] |= 0x20;
			break;
		case SMS_Deliver:
			buffer[Layout.firstbyte] |= 0x00;
	}

	/* GSM 03.40 section 9.2.3.17 (TP-Reply-Path) */
	if (SMS->ReplyViaSameSMSC) buffer[Layout.firstbyte] |= 0x80;

	if (Layout.Number!=255) {
		buffer[Layout.Number] = GSM_PackSemiOctetNumber(SMS->Number,buffer+(Layout.Number+1),true);
		smfprintf(di, "Recipient number \"%s\"\n",DecodeUnicodeString(SMS->Number));
	}
	if (Layout.SMSCNumber!=255) {
		buffer[Layout.SMSCNumber]=GSM_PackSemiOctetNumber(SMS->SMSC.Number,buffer+(Layout.SMSCNumber+1), false);
		smfprintf(di, "SMSC number \"%s\"\n",DecodeUnicodeString(SMS->SMSC.Number));
	}

	/* Message Class*/
	/* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme) and GSM 03.38 section 4 */
	if (Layout.TPDCS != 255) {
		if (SMS->Class >= 0 && SMS->Class <= 3) buffer[Layout.TPDCS] |= SMS->Class | (1 << 4);
		smfprintf(di, "SMS class %i\n",SMS->Class);
	}

	if (Layout.TPVP != 255) {
		/* GSM 03.40 section 9.2.3.3 (TP-Validity-Period-Format) */
		/* Bits 4 and 3: 10. TP-VP field present and integer represent (relative) */
		buffer[Layout.firstbyte] |= 0x10;
		buffer[Layout.TPVP]=((unsigned char)SMS->SMSC.Validity.Relative);
		smfprintf(di, "SMS validity %02x\n",SMS->SMSC.Validity.Relative);
	}

	if (Layout.DateTime != 255) {
		GSM_EncodeSMSDateTime(di, &SMS->DateTime, buffer+Layout.DateTime);
	}

	if (Layout.TPMR != 255) {
		smfprintf(di, "TPMR: %02x %i\n",SMS->MessageReference,SMS->MessageReference);
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

	/* size is the length of the data in octets including UDH */
	*length=GSM_EncodeSMSFrameText(di, SMS,buffer,Layout);
/* 	if (*length == 0) return GE_UNKNOWN; */
	*length += Layout.Text;

	return ERR_NONE;
}

/* ----------------- Some help functions ----------------------------------- */

void GSM_SetDefaultReceivedSMSData(GSM_SMSMessage *SMS)
{
	SMS->UDH.Type 			= UDH_NoUDH;
	SMS->UDH.Length			= 0;
	SMS->UDH.Text[0] 		= 0;
	SMS->UDH.ID8bit			= 0;
	SMS->UDH.ID16bit		= 0;
	SMS->UDH.PartNumber		= 0;
	SMS->UDH.AllParts		= 0;
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
	SMS->SMSC.Validity.Relative	= SMS_VALID_Max_Time;
	SMS->SMSC.Format		= SMS_FORMAT_Text;
	SMS->Number[0]			= 0;
	SMS->Number[1]			= 0;
	SMS->OtherNumbersNum		= 0;
	SMS->Name[0]			= 0;
	SMS->Name[1]			= 0;
	SMS->ReplyViaSameSMSC		= false;
	SMS->Class			= 0;
	SMS->Text[0] 			= 0;
	SMS->Text[1] 			= 0;
	SMS->RejectDuplicates		= false;
	SMS->MessageReference		= 0;
	SMS->ReplaceMessage		= 0;
	SMS->DeliveryStatus		= 0;
	SMS->DateTime.Year = 0;
}

void GSM_SetDefaultSMSData(GSM_SMSMessage *SMS)
{
	GSM_SetDefaultReceivedSMSData(SMS);

	SMS->SMSC.Validity.Format	= SMS_Validity_RelativeFormat;
	SMS->SMSC.Validity.Relative	= SMS_VALID_Max_Time;
	SMS->SMSC.Location		= 1;

	/* This part is required to save SMS */
	SMS->State			= SMS_UnSent;
	SMS->PDU			= SMS_Submit;
	SMS->Location			= 0;
	SMS->Memory			= 0;
	SMS->Folder			= 0x02;	/*Outbox*/
	SMS->InboxFolder		= false;
	GSM_GetCurrentDateTime (&SMS->DateTime);
	GSM_GetCurrentDateTime (&SMS->SMSCTime);
}

/**
 * GSM 03.40 section 9.2.3.24
 */
void GSM_EncodeUDHHeader(GSM_Debug_Info *di, GSM_UDHHeader *UDH)
{
	int i=0;

	if (UDH->Type == UDH_NoUDH) {
		UDH->Length = 0;
		return;
	}
	if (UDH->Type == UDH_UserUDH) {
		UDH->Length = UDH->Text[0] + 1;
		return;
	}
	while (true) {
		if (UDHHeaders[i].Type==UDH_NoUDH) {
			smfprintf(di, "Not supported UDH type\n");
			break;
		}
		if (UDHHeaders[i].Type!=UDH->Type) {
			i++;
			continue;
		}
		/* UDH Length */
		UDH->Text[0] = UDHHeaders[i].Length;
		memcpy(UDH->Text+1, UDHHeaders[i].Text, UDHHeaders[i].Length);
		UDH->Length 	= UDH->Text[0] + 1;

		if (UDHHeaders[i].ID8bit != -1) {
			UDH->Text[UDHHeaders[i].ID8bit+1] = UDH->ID8bit % 256;
		} else {
			UDH->ID8bit = -1;
		}
		if (UDHHeaders[i].ID16bit != -1) {
			UDH->Text[UDHHeaders[i].ID16bit+1] = UDH->ID16bit / 256;
			UDH->Text[UDHHeaders[i].ID16bit+2] = UDH->ID16bit % 256;
		} else {
			UDH->ID16bit = -1;
		}
		if (UDHHeaders[i].PartNumber != -1) {
			UDH->Text[UDHHeaders[i].PartNumber+1] = UDH->PartNumber;
		} else {
			UDH->PartNumber = -1;
		}
		if (UDHHeaders[i].AllParts != -1) {
			UDH->Text[UDHHeaders[i].AllParts+1] = UDH->AllParts;
		} else {
			UDH->AllParts = -1;
		}
		break;
	}
}

bool GSM_DecodeSiemensOTASMS(GSM_Debug_Info *di, GSM_SiemensOTASMSInfo	*Info,
			     GSM_SMSMessage 		*SMS)
{
	int current;

	if (SMS->PDU != SMS_Deliver) 		return false;
	if (SMS->Coding != SMS_Coding_8bit) 	return false;
	if (SMS->Class != 1) 			return false;
	if (SMS->UDH.Type != UDH_NoUDH) 	return false;
	if (SMS->Length < 22) 			return false;

	if (strncmp(SMS->Text,"//SEO",5)!=0) return false; /* Siemens Exchange Object */
	if (SMS->Text[5]!=1) return false; /* version 1 */
	Info->DataLen = SMS->Text[6] + SMS->Text[7]*256;
	Info->SequenceID = SMS->Text[8] + SMS->Text[9]*256 +
			 SMS->Text[10]*256*256 + SMS->Text[11]*256*256*256;
	Info->PacketNum = SMS->Text[12] + SMS->Text[13]*256;
	Info->PacketsNum = SMS->Text[14] + SMS->Text[15]*256;
	smfprintf(di, "Packet %i/%i\n",Info->PacketNum,Info->PacketsNum);
	Info->AllDataLen = SMS->Text[16] + SMS->Text[17]*256 +
			 SMS->Text[18]*256*256 + SMS->Text[19]*256*256*256;
	smfprintf(di, "DataLen %i/%lu\n",Info->DataLen,Info->AllDataLen);

	if (SMS->Text[20] > 9) return false;
	memcpy(Info->DataType,SMS->Text+21,SMS->Text[20]);
	Info->DataType[SMS->Text[20]] = 0;
	smfprintf(di, "DataType '%s'\n",Info->DataType);

	current = 21+SMS->Text[20];
	if (SMS->Text[current] > 39) return false;
	memcpy(Info->DataName,SMS->Text+current+1,SMS->Text[current]);
	Info->DataName[SMS->Text[current]] = 0;
	smfprintf(di, "DataName '%s'\n",Info->DataName);

	current += SMS->Text[current]+1;
	memcpy(Info->Data,SMS->Text+current,Info->DataLen);

	return true;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
