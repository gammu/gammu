/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.
	
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "gsm-api.h"
#include "gsm-coding.h"

/* User data headers */
GSM_UDHHeader UDHHeaders[] = {
  { GSM_ConcatenatedMessages, 0x05, "\x00\x03\x01\x00\x00" },
      /* See GSM 03.40 section 9.2.3.24.1 */
  { GSM_DisableVoice,         0x04, "\x01\x02\x00\x00" },
  { GSM_DisableFax,           0x04, "\x01\x02\x01\x00" },
  { GSM_DisableEmail,         0x04, "\x01\x02\x02\x00" },
  { GSM_EnableVoice,          0x04, "\x01\x02\x00\x01" },
  { GSM_EnableFax,            0x04, "\x01\x02\x01\x01" },
  { GSM_EnableEmail,          0x04, "\x01\x02\x02\x01" },
      /* See GSM 03.40 section 9.2.3.24.2 for voice, fax and email messages */
  { GSM_VoidSMS,              0x08, "\x01\x02\x02\x01\x01\x02\x02\x00" },
      /* When send such SMS to some phones, they don't display anything,
         only beep and enable vibra/light */
  { GSM_BugSMS,               0x09, "\x01\x02\x02\x01\x01\x02\x02\x00\xc0" },

  /* Short Smart Messaging UDH */
  /* General format: */
  /* 1 byte  0x05     : IEI application port addressing scheme, 16 bit address */
  /* 1 byte  0x04     : IEI length */
  /* 2 bytes          : destination address: high & low byte */
  /* 2 bytes 0x00 0x00: originator address: high & low byte */
  { GSM_RingtoneUDH,          0x06, "\x05\x04\x15\x81\x00\x00" },
  { GSM_OpLogo,               0x06, "\x05\x04\x15\x82\x00\x00" },
  { GSM_CallerIDLogo,         0x06, "\x05\x04\x15\x83\x00\x00" },
  { GSM_WAPBookmarkUDH,       0x06, "\x05\x04\xc3\x4f\x00\x00" },

  /* Long Smart Messaging UDH */
  /* General format: */
  /* 1 byte 0x05      : IEI application port addressing scheme, 16 bit address */
  /* 1 byte 0x04      : IEI length */
  /* 2 bytes 0x00 0x00: destination address: high & low byte */
  /* 2 bytes 0x00 0x00: originator address: high & low byte */
  /* 1 byte 0x00      : null byte for end first part ? */
  /* 1 byte 0x03      : length for rest ? */
  /* 1 byte                               */
  /* 1 byte           : number of all SMS */
  /* 1 byte           : number of current SMS */
  { GSM_CalendarNoteUDH,      0x0b, "\x05\x04\x00\xe4\x00\x00\x00\x03\xc7\x00\x00" }, //from 3310 ?
  { GSM_CalendarNoteUDH2,     0x0b, "\x05\x04\x23\xf5\x00\x00\x00\x03\x01\x00\x00" }, //from 6210 or 9210 Note: last 0x01 changes according to note type
  { GSM_ProfileUDH,           0x0b, "\x05\x04\x15\x8a\x00\x00\x00\x03\xce\x00\x00" },
  { GSM_WAPBookmarkUDH,       0x0b, "\x05\x04\xc3\x4f\x00\x00\x00\x03\x01\x00\x00" },//note last 0x01 can change
  { GSM_WAPSettingsUDH,       0x0b, "\x05\x04\xc3\x4f\x00\x00\x00\x03\x7f\x00\x00" },
  { GSM_PhonebookUDH,         0x0b, "\x05\x04\x23\xf4\x00\x00\x00\x03\x01\x00\x00" },

  { GSM_NoUDH,                0x00, "" }
};

#define ByteMask ((1 << Bits) - 1)

int GSM_PackSevenBitsToEight(int offset, unsigned char *input, unsigned char *output)
{
        unsigned char *OUT = output; /* Current pointer to the output buffer */
        unsigned char *IN  = input;  /* Current pointer to the input buffer */
        int Bits;                    /* Number of bits directly copied to
                                        the output buffer */
        Bits = (7 + offset) % 8;

        /* If we don't begin with 0th bit, we will write only a part of the
           first octet */
        if (offset) {
                *OUT = 0x00;
                OUT++;
        }

        while ((IN - input) < strlen(input)) {
                unsigned char Byte = EncodeWithDefaultAlphabet(*IN);

                *OUT = Byte >> (7 - Bits);
                /* If we don't write at 0th bit of the octet, we should write
                   a second part of the previous octet */
                if (Bits != 7)
                        *(OUT-1) |= (Byte & ((1 << (7-Bits)) - 1)) << (Bits+1);

                Bits--;

                if (Bits == -1) Bits = 7;
                else OUT++;

                IN++;
        }
        return (OUT - output);
}

int GSM_UnpackEightBitsToSeven(int offset, int in_length, int out_length,
                           unsigned char *input, unsigned char *output)
{
        unsigned char *OUT = output; /* Current pointer to the output buffer */
        unsigned char *IN  = input;  /* Current pointer to the input buffer */
        unsigned char Rest = 0x00;
        int Bits;

        Bits = offset ? offset : 7;

        while ((IN - input) < in_length) {

                *OUT = ((*IN & ByteMask) << (7 - Bits)) | Rest;
                Rest = *IN >> Bits;

                /* If we don't start from 0th bit, we shouldn't go to the
                   next char. Under *OUT we have now 0 and under Rest -
                   _first_ part of the char. */
                if ((IN != input) || (Bits == 7)) OUT++;
                IN++;

                if ((OUT - output) >= out_length) break;

                /* After reading 7 octets we have read 7 full characters but
                   we have 7 bits as well. This is the next character */
                if (Bits == 1) {
                        *OUT = Rest;
                        OUT++;
                        Bits = 7;
                        Rest = 0x00;
                } else {
                        Bits--;
                }
        }

        return OUT - output;
}

/* This function implements packing of numbers (SMS Center number and
   destination number) for SMS sending function. */
/* See GSM 03.40 9.1.1:
   1 byte  - length of number given in semioctets or bytes (when given in bytes,
             includes one byte for byte with number format.
             Returned by function (set semioctet to true, if want result
             in semioctets).
   1 byte  - format of number. See GSM_NumberType; in gsm-common.h. Returned
             in u8 *Output.
   n bytes - 2n or 2n-1 semioctets with number. For some types of numbers
             in the most significant bits of the last byte with 0x0f
             (1111 binary) are filled if the number is represented
             with odd number of digits. Returned in u8 *Output. */
/* 1 semioctet = 4 bits = half of byte */
int GSM_PackSemiOctetNumber(u8 *Number, u8 *Output, bool semioctet) {

  u8 *IN=Number;  /* Pointer to the input number */
  u8 *OUT=Output; /* Pointer to the output */
  int length=0,j;
  unsigned char format=GNT_UNKNOWN; // format of number used by us

  /* Checking for format number */
  while (*IN) {
    if (length==0 && *IN=='+')
      format=GNT_INTERNATIONAL;  // first byte is '+'. It can be international
    else {
      if (*IN>'9' || *IN<'0') { 
        format=GNT_ALPHANUMERIC; //char is not number. It must be alphanumeric
      }
    }
    IN++;length++;
  }

  /* Now we return to first byte */
  for (j=0;j<length;j++) IN--;

  /* The first byte in the Semi-octet representation of the address field is
     the Type-of-Address. This field is described in the official GSM
     specification 03.40 version 5.3.0, section 9.1.2.5, page 33.*/
  *OUT++=format;

  /* The next field is the number. See GSM 03.40 section 9.1.2 */
  switch (format) {
    case GNT_ALPHANUMERIC:
      length=GSM_PackSevenBitsToEight(0, IN, OUT)*2;
      break;

    case GNT_INTERNATIONAL:
      length--;
      EncodeBCD (OUT, IN+1, length, true);
      break;

    default:
      EncodeBCD (OUT, IN, length, true);
      break;
  }

  if (semioctet) {
    return length;
  } else {
    /* Convert number of semioctets to number of chars */
    if (length % 2) length++;
    return length / 2 + 1;
  }
}

char *GSM_UnpackSemiOctetNumber(u8 *Number, bool semioctet) {

  static char Buffer[20]="";  
  int length=Number[0];

  if (semioctet) {
    /* Convert number of semioctets to number of chars */
    if (length % 2) length++;
    length=length / 2 + 1;
  }
  
  length--; //without leading byte with format of number

  switch (Number[1]) {
    case GNT_ALPHANUMERIC:
      GSM_UnpackEightBitsToSeven(0, length, length, Number+2, Buffer);
      Buffer[length]=0;
      break;

    case GNT_INTERNATIONAL:
      Buffer[0]='+';
      DecodeBCD (Buffer+1, Number+2, length);
      break;

    default:
      DecodeBCD (Buffer, Number+2, length);
      break;
  }

  return Buffer;
}

/* See GSM 03.40 section 9.2.3.11 */
GSM_Error GSM_EncodeSMSDateTime(GSM_DateTime *DT, unsigned char *req)
{
#ifdef DEBUG
  fprintf(stdout,_("Date & time in saved SMS: %02i/%02i/%04i %02i:%02i:%02i\n"),
    DT->Day,DT->Month,DT->Year,DT->Hour,DT->Minute,DT->Second);
#endif
  
  req[0]=EncodeWithBCDAlphabet(DT->Year);
  req[1]=EncodeWithBCDAlphabet(DT->Month);
  req[2]=EncodeWithBCDAlphabet(DT->Day);
  req[3]=EncodeWithBCDAlphabet(DT->Hour);
  req[4]=EncodeWithBCDAlphabet(DT->Minute);
  req[5]=EncodeWithBCDAlphabet(DT->Second);

  /* FIXME: do it */
  req[6]=0; /* TimeZone = +-0 */

  return GE_NONE;
}

/* See GSM 03.40 section 9.2.3.11 */
GSM_Error GSM_DecodeSMSDateTime(GSM_DateTime *DT, unsigned char *req)
{
  DT->Year    = DecodeWithBCDAlphabet(req[0]);
  DT->Month   = DecodeWithBCDAlphabet(req[1]);
  DT->Day     = DecodeWithBCDAlphabet(req[2]);
  DT->Hour    = DecodeWithBCDAlphabet(req[3]);
  DT->Minute  = DecodeWithBCDAlphabet(req[4]);
  DT->Second  = DecodeWithBCDAlphabet(req[5]);

  DT->Timezone=(10*(req[6]&0x07)+(req[6]>>4))/4;
  if (req[6]&0x08) DT->Timezone = -DT->Timezone;

#ifdef DEBUG
  fprintf(stdout, _("%d%d-"), req[0]&0xf, req[0]>>4);
  fprintf(stdout, _("%d%d-"), req[1]&0xf, req[1]>>4);
  fprintf(stdout, _("%d%d "), req[2]&0xf, req[2]>>4);
  fprintf(stdout, _("%d%d:"), req[3]&0xf, req[3]>>4);
  fprintf(stdout, _("%d%d:"), req[4]&0xf, req[4]>>4);
  fprintf(stdout, _("%d%d "), req[5]&0xf, req[5]>>4);

  if (req[6]) {
    if (req[6] & 0x08) fprintf(stdout, "-");
                  else fprintf(stdout, "+");

    fprintf(stdout, _("%02d00"), (10*(req[6]&0x07)+(req[6]>>4))/4);
  }

  fprintf(stdout, "\n");
#endif

  return GE_NONE;
}

int GSM_EncodeETSISMSSubmitData(GSM_SMSMessage *SMS, GSM_ETSISMSMessage *ETSI)
{
  int off,size=0,size2=0,w,i;

  /* off - length of the user data header */
  off = 0;

  if (SMS->UDHType) {
    /* GSM 03.40 section 9.2.3.23 (TP-User-Data-Header-Indicator) */
    ETSI->firstbyte |= 0x40;

    /* off - length of the user data header */
    off = 1 + SMS->UDH[0];

    /* we copy the udh */
    memcpy(ETSI->MessageText, SMS->UDH, off);

//  if (SMS->UDHType==GSM_HangSMS) ETSI->TPDCS=0x08; //Such SMS hangs phone (5110, 5.07),
                                                     //when saved to Outbox atry try to read it
					             /*from phone's menu*/
  }

  switch (SMS->Coding) {
    /* When save SMS to SIM and it's 8 bit SMS,
       "Edit" is not displayed in phone menu and
       "Message cannot be displayed here" instead of message text */
    case GSM_Coding_8bit:

      /* the mask for the 8-bit data */
      /* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme) and GSM 03.38 section 4 */
      ETSI->TPDCS |= 0xf4;
    
      memcpy(ETSI->MessageText + off, SMS->MessageText, SMS->Length);

      size2 = size = SMS->Length+off;

      break;

    case GSM_Coding_Default:

      w=(7-off)%7;
      if (w<0) w=(14-off)%14;

      size = GSM_PackSevenBitsToEight(w, SMS->MessageText, ETSI->MessageText + off);
      size += off;
      size2 = (off*8 + w) / 7 + strlen(SMS->MessageText);

      break;

    case GSM_Coding_Unicode:

      /* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme) and GSM 03.38 section 4 */
      ETSI->TPDCS |= 0x08;

#ifdef DEBUG
      fprintf(stdout,_("SMS Length is %i\n"),strlen(SMS->MessageText));
#endif

      EncodeUnicode (ETSI->MessageText+off,SMS->MessageText,strlen(SMS->MessageText));
      /* here we code "special" chars */
      for (i=0;i<strlen(SMS->MessageText);i++) {
	if (SMS->MessageText[i]=='~') ETSI->MessageText[off+1+i*2]=1; //enables/disables blinking
        if (SMS->MessageText[i]=='`') ETSI->MessageText[off+1+i*2]=0; //hides rest ot contents
      }

      size=size2=strlen(SMS->MessageText)*2+off;

      break;
  }

  /* FIXME: support for compression */
  /* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme) and GSM 03.38 section 4 */
  /* See also GSM 03.42 */
  /* if (SMS->Compression) ETSI->TPDCS |= 0x20; */

  /* SMS->Length is:
  	- integer representation of the number od octets within the user data when UD is coded using 8bit data
  	- the sum of the number of septets in UDH including any padding and number of septets in UD in other case */
  /* GSM 03.40 section 9.2.3.16 (TP-User-Data-Length) */
  ETSI->TPUDL = size2;

  return size;
}

GSM_Error GSM_DecodeETSISMSSubmitData(GSM_SMSMessage *SMS, GSM_ETSISMSMessage *ETSI)
{
  int off,w,i,tmp=0;
  unsigned char output[161];
  bool UDHOK;

  /* off - length of the user data header */
  off = 0;
  
  SMS->UDHType = GSM_NoUDH;

  if (ETSI->firstbyte & 64) { /* UDH header available */

    off = (ETSI->MessageText[0] + 1); /* Length of UDH header */
    
    /* Copy UDH header into SMS->UDH */
    for (i = 0; i < off; i++) SMS->UDH[i] = ETSI->MessageText[i];

#ifdef DEBUG
    fprintf(stdout, "   UDH header available (length %i",off);
#endif
    
    SMS->UDHType = GSM_UnknownUDH;

    i=-1;
    while (true) {
      i++;
      if (UDHHeaders[i].UDHType==GSM_NoUDH) break;
      tmp=UDHHeaders[i].Length;
      if (tmp==SMS->UDH[0]) { //if length is the same

        if (tmp==0x05) tmp=tmp-2;/*two last bytes can be different for such UDH*/
        if (tmp==0x0b) tmp=tmp-3;/*three last bytes can be different for such UDH*/

        UDHOK=true;
        for (w=0;w<tmp;w++) {
          if (UDHHeaders[i].Text[w]!=SMS->UDH[w+1]) {
            UDHOK=false;
            break;
          }
        }
        if (UDHOK) {
          SMS->UDHType=UDHHeaders[i].UDHType;
          break;
        }
      }
    }

#ifdef DEBUG
    switch (SMS->UDHType) {
      case GSM_ConcatenatedMessages:
        fprintf(stdout,_(", concatenated (linked) message %d/%d"),SMS->UDH[5],SMS->UDH[4]);break;
      case GSM_DisableVoice:
        fprintf(stdout,_(", disables voice indicator"));break;
      case GSM_EnableVoice:
        fprintf(stdout,_(", enables voice indicator"));break;
      case GSM_DisableFax:
        fprintf(stdout,_(", disables fax indicator"));break;
      case GSM_EnableFax:
        fprintf(stdout,_(", enables fax indicator"));break;
      case GSM_DisableEmail:
        fprintf(stdout,_(", disables email indicator"));break;
      case GSM_EnableEmail:
        fprintf(stdout,_(", enables email indicator"));break;
      case GSM_VoidSMS:
        fprintf(stdout,_(", void SMS"));break;
      case GSM_WAPBookmarkUDH:
        fprintf(stdout,_(", WAP Bookmark"));break;
      case GSM_WAPBookmarkUDHLong:
        fprintf(stdout,_(", WAP Bookmark, part %i/%i"),SMS->UDH[11],SMS->UDH[10]);break;
      case GSM_WAPSettingsUDH:
        fprintf(stdout,_(", WAP Settings, part %i/%i"),SMS->UDH[11],SMS->UDH[10]);break;
      case GSM_RingtoneUDH:
        fprintf(stdout,_(", ringtone"));break;
      case GSM_OpLogo:
        fprintf(stdout,_(", GSM Operator Logo"));break;
      case GSM_CallerIDLogo:
        fprintf(stdout,_(", Caller Logo"));break;
      case GSM_ProfileUDH:
        fprintf(stdout,_(", Profile SMS, part %i/%i"),SMS->UDH[11],SMS->UDH[10]);break;
      case GSM_CalendarNoteUDH:
        fprintf(stdout,_(", Calendar note SMS, part %i/%i"),SMS->UDH[11],SMS->UDH[10]);break;
      case GSM_CalendarNoteUDH2:
        fprintf(stdout,_(", Calendar note SMS, part %i/%i"),SMS->UDH[11],SMS->UDH[10]);break;
      case GSM_PhonebookUDH:
        fprintf(stdout,_(", Phonebook Entry, part %i/%i"),SMS->UDH[11],SMS->UDH[10]);break;
      default:
        fprintf(stdout,_(", UNKNOWN"));break;
    }
               
    fprintf(stdout, ")\n");

    hexdump(off,SMS->UDH);
#endif
  }

  SMS->Coding = GSM_Coding_Default;

  /* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme) and GSM 03.38 section 4 */
  if ((ETSI->TPDCS & 0xf4) == 0xf4) SMS->Coding=GSM_Coding_8bit;
  if ((ETSI->TPDCS & 0x08) == 0x08) SMS->Coding=GSM_Coding_Unicode;

  switch (SMS->Coding) {
    case GSM_Coding_Default:
      w=(7-off)%7;
      if (w<0) w=(14-off)%14;
  
      SMS->Length=ETSI->TPUDL - (off*8 + w) / 7;

      tmp=GSM_UnpackEightBitsToSeven(w,ETSI->TPUDL-off, SMS->Length, ETSI->MessageText+off, output);

#ifdef DEBUG
      fprintf(stdout, "   7 bit SMS, body is (length %i): ",SMS->Length);
#endif /* DEBUG */

      DecodeDefault (SMS->MessageText, output, SMS->Length);

#ifdef DEBUG
      fprintf(stdout, "%s\n",SMS->MessageText);	  
#endif

      break;
    case GSM_Coding_8bit:
      SMS->Length=ETSI->TPUDL - off;

      memcpy(SMS->MessageText,ETSI->MessageText+off,SMS->Length);

#ifdef DEBUG
      fprintf(stdout, "   8 bit SMS, body is (length %i)\n",SMS->Length);
      hexdump(SMS->Length,SMS->MessageText);
#endif /* DEBUG */

      break;
    case GSM_Coding_Unicode:
      SMS->Length=(ETSI->TPUDL - off) / 2;

#ifdef DEBUG
      fprintf(stdout, "   7 bit SMS, body is (length %i), Unicode coding: ",SMS->Length);
      for (i=0; i<SMS->Length;i++) {
        fprintf(stdout, "[%02x %02x]", ETSI->MessageText[off+i*2] , ETSI->MessageText[off+i*2+1]);
      }
      fprintf(stdout, "\n");	          
#endif /* DEBUG */

      /* here we decode "special" chars */
      for (i=0; i<SMS->Length;i++) {
        if (ETSI->MessageText[off+i*2]  ==0x00 &&
	    ETSI->MessageText[off+i*2+1]==0x01)
	  ETSI->MessageText[off+i*2+1]='~'; //enables/disables blinking
        if (ETSI->MessageText[off+i*2]  ==0x00 &&
            ETSI->MessageText[off+i*2+1]==0x00)
	  ETSI->MessageText[off+i*2+1]='`'; //hides rest ot contents
      }

      DecodeUnicode (SMS->MessageText, ETSI->MessageText+off, SMS->Length);

      break;
  }

  return GE_NONE;
}

GSM_Error GSM_DecodeETSISMSStatusReportData(GSM_SMSMessage *SMS, GSM_ETSISMSMessage *ETSI)
{
  /* See GSM 03.40 section 9.2.3.11 (TP-Service-Centre-Time-Stamp) */
#ifdef DEBUG
  fprintf(stdout, _("   SMSC response date: "));
#endif
  GSM_DecodeSMSDateTime(&SMS->SMSCTime, ETSI->SMSCDateTime);
    
  if (ETSI->TPStatus < 0x03) {
    strcpy(SMS->MessageText,_("Delivered"));

#ifdef DEBUG
    /* more detailed reason only for debug */
    /* See GSM 03.40 section 9.2.3.15 (TP-Status) */
    switch (ETSI->TPStatus) {
      case 0x00: fprintf(stdout, _("   SM received by the SME"));break;
      case 0x01: fprintf(stdout, _("   SM forwarded by the SC to the SME but the SC is unable to confirm delivery"));break;
      case 0x02: fprintf(stdout, _("   SM replaced by the SC"));break;
    }
#endif /* DEBUG */

    SMS->Length = 10;
      
  } else if (ETSI->TPStatus & 0x40) {

    strcpy(SMS->MessageText,_("Failed"));

#ifdef DEBUG
    /* more detailed reason only for debug */
    if (ETSI->TPStatus & 0x20) {

      /* See GSM 03.40 section 9.2.3.15 (TP-Status) */
      fprintf(stdout, _("   Temporary error, SC is not making any more transfer attempts\n"));
      switch (ETSI->TPStatus) {
        case 0x60: fprintf(stdout, _("   Congestion"));break;
        case 0x61: fprintf(stdout, _("   SME busy"));break;
        case 0x62: fprintf(stdout, _("   No response from SME"));break;
        case 0x63: fprintf(stdout, _("   Service rejected"));break;
        case 0x64: fprintf(stdout, _("   Quality of service not available"));break;
        case 0x65: fprintf(stdout, _("   Error in SME"));break;
        default  : fprintf(stdout, _("   Reserved/Specific to SC: %x"),ETSI->TPStatus);break;
      }

    } else {

      /* See GSM 03.40 section 9.2.3.15 (TP-Status) */
      fprintf(stdout, _("   Permanent error, SC is not making any more transfer attempts\n"));
      switch (ETSI->TPStatus) {
        case 0x40: fprintf(stdout, _("   Remote procedure error"));break;
        case 0x41: fprintf(stdout, _("   Incompatibile destination"));break;
        case 0x42: fprintf(stdout, _("   Connection rejected by SME"));break;
        case 0x43: fprintf(stdout, _("   Not obtainable"));break;
        case 0x44: fprintf(stdout, _("   Quality of service not available"));break;
        case 0x45: fprintf(stdout, _("   No internetworking available"));break;
        case 0x46: fprintf(stdout, _("   SM Validity Period Expired"));break;
        case 0x47: fprintf(stdout, _("   SM deleted by originating SME"));break;
        case 0x48: fprintf(stdout, _("   SM Deleted by SC Administration"));break;
        case 0x49: fprintf(stdout, _("   SM does not exist"));break;
        default  : fprintf(stdout, _("   Reserved/Specific to SC: %x"),ETSI->TPStatus);break;
      }
    }
#endif /* DEBUG */
      
      SMS->Length = 6;
  } else if (ETSI->TPStatus & 0x20) {
    strcpy(SMS->MessageText,_("Pending"));

#ifdef DEBUG
    /* more detailed reason only for debug */
    /* See GSM 03.40 section 9.2.3.15 (TP-Status) */
    fprintf(stdout, _("   Temporary error, SC still trying to transfer SM\n"));
    switch (ETSI->TPStatus) {
      case 0x20: fprintf(stdout, _("   Congestion"));break;
      case 0x21: fprintf(stdout, _("   SME busy"));break;
      case 0x22: fprintf(stdout, _("   No response from SME"));break;
      case 0x23: fprintf(stdout, _("   Service rejected"));break;
      case 0x24: fprintf(stdout, _("   Quality of service not aviable"));break;
      case 0x25: fprintf(stdout, _("   Error in SME"));break;
      default  : fprintf(stdout, _("   Reserved/Specific to SC: %x"),ETSI->TPStatus);break;
    }
#endif /* DEBUG */
    SMS->Length = 7;
  } else {
    strcpy(SMS->MessageText,_("Unknown"));

#ifdef DEBUG
    /* more detailed reason only for debug */
    fprintf(stdout, _("   Reserved/Specific to SC: %x"),ETSI->TPStatus);
#endif /* DEBUG */
    SMS->Length = 8;
  }

#ifdef DEBUG
  fprintf(stdout, _("\n"));
#endif /* DEBUG */

  return GE_NONE;
}

GSM_Error GSM_EncodeETSISMSSubmitHeader(GSM_SMSMessage *SMS,GSM_ETSISMSMessage *ETSI)
{
  GSM_Error error;

  /* First of all we should get SMSC number */
  if (SMS->MessageCenter.No) {
    error = GSM->GetSMSCenter(&SMS->MessageCenter);
    if (error != GE_NONE) return error;
    SMS->MessageCenter.No = 0;
  }

#ifdef DEBUG
  fprintf(stdout, _("Packing SMS to \"%s\" via message center \"%s\"\n"), SMS->Destination, SMS->MessageCenter.Number);
#endif /* DEBUG */

  ETSI->SMSCNumber[0]=GSM_PackSemiOctetNumber(SMS->MessageCenter.Number, ETSI->SMSCNumber+1, false);

  /* GSM 03.40 section 9.2.3.17 (TP-Reply-Path) */
  if (SMS->ReplyViaSameSMSC) ETSI->firstbyte |= 128;
  
  /* When save to Outbox with SMS Class, "Edit" is not displayed in phone menu
     and can forward it to another phone with set class (for example, 0=Flash) */
  /* Message Class*/
  /* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme) and GSM 03.38 section 4 */
  if (SMS->Class>=0 && SMS->Class<5) ETSI->TPDCS |= (240+SMS->Class);  

  /* When is not set for SMS saved for Inbox, phone displays "Message" instead
     of number and doesn't display "Details" info */
  ETSI->Number[0] = GSM_PackSemiOctetNumber(SMS->Destination, ETSI->Number+1, true);

  return GE_NONE;
}

GSM_Error GSM_DecodeETSISMSHeader(GSM_SMSMessage *SMS, GSM_ETSISMSMessage *ETSI)
{
#ifdef DEBUG
  fprintf(stdout, _("   SMS center number: %s"), GSM_UnpackSemiOctetNumber(ETSI->SMSCNumber,false));
  if (SMS->folder==0 && (ETSI->firstbyte & 128)!=0) //GST_INBOX
    fprintf(stdout, _(" (centre set for reply)"));
#endif

  strcpy(SMS->MessageCenter.Number, GSM_UnpackSemiOctetNumber(ETSI->SMSCNumber,false));

  SMS->ReplyViaSameSMSC=false;
  if ((ETSI->firstbyte & 128)!=0) SMS->ReplyViaSameSMSC=true;

#ifdef DEBUG      
  fprintf(stdout, _("\n   Remote number (recipient or sender): %s\n"), GSM_UnpackSemiOctetNumber(ETSI->Number,true));
#endif

  strcpy(SMS->Sender, GSM_UnpackSemiOctetNumber(ETSI->Number,true));

  return GE_NONE;
}

/* FIXME: we should allow for all validity formats */
GSM_Error GSM_EncodeETSISMSSubmitValidity(GSM_SMSMessage *SMS,GSM_ETSISMSMessage *ETSI)
{
  /* GSM 03.40 section 9.2.3.3 (TP-Validity-Period-Format) */
  /* Bits 4 and 3: 10. TP-VP field present and integer represent (relative) */
  ETSI->firstbyte |= 0x10;

  /* GSM 03.40 section 9.2.3.12 (TP-Validity Period) */
  /* FIXME: error-checking for correct Validity - it should not be bigger then
     63 weeks and smaller then 5minutes. We should also test intervals because
     the SMS->Validity to TP-VP is not continuos. I think that the simplest
     solution will be an array of correct values. We should parse it and if we
     find the closest TP-VP value we should use it. Or is it good to take
     closest smaller TP-VP as we do now? I think it is :-) */

  /* 5 minutes intervals up to 12 hours = 720 minutes */
  if (SMS->Validity <= 720)
    ETSI->TPVP = (unsigned char) (SMS->Validity/5)-1;

  /* 30 minutes intervals up to 1 day */
  else if ((SMS->Validity > 720) && (SMS->Validity <= 1440))
    ETSI->TPVP = (unsigned char) ((SMS->Validity-720)/30)+143;

  /* 1 day intervals up to 30 days */
  else if ((SMS->Validity > 1440) && (SMS->Validity <= 43200))
    ETSI->TPVP = (unsigned char) (SMS->Validity/1440)+166;

  /* 1 week intervals up to 63 weeks */
  else if ((SMS->Validity > 43200) && (SMS->Validity <= 635040))
    ETSI->TPVP = (unsigned char) (SMS->Validity/10080)+192;

  return GE_NONE;
}

/* FIXME: do we need more than SMS_Submit and SMS_Deliver ? */
GSM_Error GSM_EncodeETSISMS(GSM_SMSMessage *SMS, GSM_ETSISMSMessage *ETSI, SMS_MessageType PDU, int *length)
{
  int size=0;
  GSM_Error error;

  ETSI->firstbyte=0;
  ETSI->TPPID=0;
  ETSI->TPDCS=0;
  ETSI->TPUDL=0;
  ETSI->TPStatus=0;
  ETSI->TPVP=0;

  switch (PDU) {
    case SMS_Submit:

      /* GSM 03.40 section 9.2.3.1 (TP-Message-Type-Indicator) */
      /* Bits 1 and 0: 01. SMS-Submit */
      ETSI->firstbyte |= 0x01;

      /* GSM 03.40 section 9.2.3.5 (TP-Status-Raport-Request) */
      /* Mask for request for delivery report from SMSC */
      if (SMS->Type == GST_DR) ETSI->firstbyte |= 32;

      error=GSM_EncodeETSISMSSubmitHeader(SMS, ETSI);
      if (error!=GE_NONE) return error;
      error=GSM_EncodeETSISMSSubmitValidity(SMS, ETSI);
      if (error!=GE_NONE) return error;
      size=GSM_EncodeETSISMSSubmitData(SMS, ETSI);

      break;
    case SMS_Deliver:

      /* GSM 03.40 section 9.2.3.1 (TP-Message-Type-Indicator) */
      /* Bits 1 and 0: 00. SMS-Deliver */
      ETSI->firstbyte |= 0x00;

      error=GSM_EncodeETSISMSSubmitHeader(SMS, ETSI);
      if (error!=GE_NONE) return error;
      GSM_EncodeSMSDateTime(&SMS->Time, ETSI->DeliveryDateTime);
      size=GSM_EncodeETSISMSSubmitData(SMS, ETSI);

      break;
    default:
      break;
  }

  /* size is the length of the data in octets including udh */
  *length=size;

  return GE_NONE;
}

/* This function decodes parts of SMS coded according to GSM 03.40 
   (given in GSM_ETSISMSMessage) to GSM_SMSMessage */
GSM_Error GSM_DecodeETSISMS(GSM_SMSMessage *SMS, GSM_ETSISMSMessage *ETSI)
{
  SMS_MessageType PDU=SMS_Deliver;

  /* See GSM 03.40 section 9.2.3.1 */
  if ((ETSI->firstbyte & 0x03) == 0x01) PDU=SMS_Submit;
  if ((ETSI->firstbyte & 0x03) == 0x02) PDU=SMS_Status_Report;

  GSM_DecodeETSISMSHeader(SMS, ETSI);

  switch (PDU) {
    case SMS_Submit:
#ifdef DEBUG
      fprintf(stdout, _("   SMS submit "));
#endif
      SMS->SMSData=false; 
      GSM_DecodeETSISMSSubmitData(SMS,ETSI);
      break;
    case SMS_Deliver:
#ifdef DEBUG
      fprintf(stdout, _("   SMS deliver "));
      fprintf(stdout, _("   Date: "));
#endif
      SMS->SMSData=true; 
      GSM_DecodeSMSDateTime(&SMS->Time, ETSI->DeliveryDateTime);
      GSM_DecodeETSISMSSubmitData(SMS,ETSI);
      break;
    case SMS_Status_Report:
#ifdef DEBUG
      fprintf(stdout, _("   SMS status report "));
      fprintf(stdout, _("   Date: "));
#endif
      SMS->SMSData=true; 
      GSM_DecodeSMSDateTime(&SMS->Time, ETSI->DeliveryDateTime);
      GSM_DecodeETSISMSStatusReportData(SMS,ETSI);
      break;
    default:
      break;
  }

  SMS->MessageText[SMS->Length]=0;

  return GE_NONE;
}

void GSM_SetDefaultSMSData (GSM_SMSMessage *SMS)
{
  struct tm *now;
  time_t nowh;
  GSM_DateTime Date;

  /* Default settings for SMS message:
  - no delivery report
  - Class Message 1
  - no compression
  - SMSC no. 1
  - validity 3 days */

  SMS->folder = GST_OUTBOX;
  SMS->Type = GST_SMS;
  SMS->Class = -1;
  SMS->Compression = false;
  SMS->MessageCenter.No = 1;
  SMS->Validity = 4320; /* 4320 minutes == 72 hours */
  SMS->ReplyViaSameSMSC = false;
  SMS->UDHType = GSM_NoUDH;
  SMS->Coding=GSM_Coding_Default;
  strcpy(SMS->Destination,"");

  /* This part is required to save SMS */    

  SMS->Status = GSS_NOTSENTREAD;
  SMS->Location = 0;

  nowh=time(NULL);
  now=localtime(&nowh);

  Date.Year = now->tm_year;
  Date.Month = now->tm_mon+1;
  Date.Day = now->tm_mday;
  Date.Hour = now->tm_hour;
  Date.Minute = now->tm_min;
  Date.Second = now->tm_sec;

  /* I have 100 (for 2000) Year now :-) */
  if (Date.Year>99 && Date.Year<1900) {
    Date.Year=Date.Year+1900;
  }

  /* We need to have only two last digits of year */
  if (Date.Year>1900)
  {
    if (Date.Year<2000) Date.Year = Date.Year-1900;
                   else Date.Year = Date.Year-2000;
  }

  SMS->Time.Year=Date.Year;
  SMS->Time.Month=Date.Month;
  SMS->Time.Day=Date.Day;
  SMS->Time.Hour=Date.Hour;
  SMS->Time.Minute=Date.Minute;
  SMS->Time.Second=Date.Second;

  SMS->Name[0]=0;
}

/* This function encodes the UserDataHeader as described in:
   - GSM 03.40 version 6.1.0 Release 1997, section 9.2.3.24
   - Smart Messaging Specification, Revision 1.0.0, September 15, 1997
*/
GSM_Error EncodeUDHHeader(char *text, GSM_UDH UDHType)
{
	int i=0;

	if (UDHType!=GSM_NoUDH) {
          while (true) {
            if (UDHHeaders[i].UDHType==GSM_NoUDH) {
#ifdef DEBUG
		fprintf(stderr,_("Not supported User Data Header type\n"));
#endif
		break;
            }
            if (UDHHeaders[i].UDHType==UDHType) {
		text[0] = UDHHeaders[i].Length; // UDH Length
		memcpy(text+1, UDHHeaders[i].Text, UDHHeaders[i].Length);
		break;
            }
            i++;
          }
        }
	return GE_NONE;
}

int GSM_MakeSinglePartSMS2(GSM_SMSMessage *SMS,
    unsigned char *MessageBuffer,int cur, GSM_UDH UDHType, GSM_Coding_Type Coding){

  int j;
  int current,smsudhlength;

  GSM_SetDefaultSMSData(SMS);

  EncodeUDHHeader(SMS->UDH, UDHType);
  SMS->UDHType=UDHType;

  switch (UDHType) {
    case GSM_EnableVoice:
    case GSM_DisableVoice:
    case GSM_EnableEmail:
    case GSM_DisableEmail:
    case GSM_EnableFax:
    case GSM_DisableFax:
      SMS->Class=1;
      SMS->Coding=Coding;
      break;
    case GSM_NoUDH:
    case GSM_ConcatenatedMessages:
    case GSM_VoidSMS:
    case GSM_HangSMS:
    case GSM_BugSMS:
    case GSM_PhonebookUDH:
    case GSM_CalendarNoteUDH: //class=1?
      SMS->Class=-1;
      SMS->Coding=Coding;
      break;
    case GSM_OpLogo:
    case GSM_CallerIDLogo:
    case GSM_RingtoneUDH:
    case GSM_WAPBookmarkUDH:
    case GSM_WAPBookmarkUDHLong:
    case GSM_WAPSettingsUDH:
    case GSM_ProfileUDH:
      SMS->Class=1;
      SMS->Coding=GSM_Coding_8bit;
      break;
    default:
      fprintf(stderr,_("Error in makesinglepartsms !\n\n\n"));
  }

  current=cur;

  smsudhlength=0;
  if (UDHType!=GSM_NoUDH)
    smsudhlength=SMS->UDH[0]+1;

  j=0;
  switch (SMS->Coding) {
    case GSM_Coding_8bit:
      j=(GSM_MAX_SMS_8_BIT_LENGTH-smsudhlength);     //max=140
      break;
    case GSM_Coding_Default:
      j=(GSM_MAX_SMS_8_BIT_LENGTH-smsudhlength)*8/7; //max=160
      break;
    case GSM_Coding_Unicode:
      j=(GSM_MAX_SMS_8_BIT_LENGTH-smsudhlength)/2;   //max=70
      break;
  }
  if (current>j) current=j;

  memcpy(SMS->MessageText,MessageBuffer,current);    
  SMS->MessageText[current]=0;
  SMS->Length=current;

  return current;
}

void GSM_MakeMultiPartSMS2(GSM_MultiSMSMessage *SMS,
    unsigned char *MessageBuffer,int MessageLength, GSM_UDH UDHType, GSM_Coding_Type Coding){

  int i=0,j,pos=0,current=0;

  for (i=0;i<4;i++) {  
    if (pos==MessageLength) break;

    current=MessageLength-pos;

    pos=pos+GSM_MakeSinglePartSMS2(&SMS->SMS[i],MessageBuffer+pos,current,UDHType,Coding);
  }

  for (j=0;j<i;j++)
  {
    switch (UDHType) {
      case GSM_ProfileUDH:
      case GSM_WAPBookmarkUDHLong:
      case GSM_WAPSettingsUDH:
      case GSM_CalendarNoteUDH:
      case GSM_CalendarNoteUDH2:
      case GSM_PhonebookUDH:
        SMS->SMS[j].UDH[10]=i;
        SMS->SMS[j].UDH[11]=j+1;
        break;
      case GSM_ConcatenatedMessages:
        SMS->SMS[j].UDH[4]=i;
        SMS->SMS[j].UDH[5]=j+1;
        break;
    default:
      break;
    }
  }

  SMS->number=i;
}
