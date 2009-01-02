/* (c) 2002-2006 by Marcin Wiacek */

#include <ctype.h>
#include <string.h>
#include <time.h>

#include <gammu-unicode.h>
#include <gammu-debug.h>

#include "gsmmulti.h"
#include "../gsmring.h"
#include "../gsmlogo.h"
#include "../../misc/coding/coding.h"
#include "../../debug.h"
#include "gsmems.h"
#include "../gsmdata.h"
#include "../gsmnet.h"

/* ----------------- Splitting SMS into parts ------------------------------ */

unsigned char GSM_MakeSMSIDFromTime(void)
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

void GSM_Find_Free_Used_SMS2(GSM_Debug_Info *di, GSM_Coding_Type Coding,GSM_SMSMessage SMS, size_t *UsedText, size_t *FreeText, size_t *FreeBytes)
{
	size_t UsedBytes;

	switch (Coding) {
	case SMS_Coding_Default_No_Compression:
		FindDefaultAlphabetLen(SMS.Text,&UsedBytes,UsedText,500);
		UsedBytes = *UsedText * 7 / 8;
		if (UsedBytes * 8 / 7 != *UsedText) UsedBytes++;
		*FreeBytes = GSM_MAX_8BIT_SMS_LENGTH - SMS.UDH.Length - UsedBytes;
		*FreeText = (GSM_MAX_8BIT_SMS_LENGTH - SMS.UDH.Length) * 8 / 7 - *UsedText;
		break;
	case SMS_Coding_Unicode_No_Compression:
		*UsedText = UnicodeLength(SMS.Text);
		UsedBytes = *UsedText * 2;
		*FreeBytes = GSM_MAX_8BIT_SMS_LENGTH - SMS.UDH.Length - UsedBytes;
		*FreeText = *FreeBytes / 2;
		break;
	case SMS_Coding_8bit:
		*UsedText = UsedBytes = SMS.Length;
		*FreeBytes = GSM_MAX_8BIT_SMS_LENGTH - SMS.UDH.Length - UsedBytes;
		*FreeText = *FreeBytes;
		break;
	default:
		break;
	}
	smfprintf(di, "UDH len %i, UsedBytes " SIZE_T_FORMAT ", FreeText " SIZE_T_FORMAT ", UsedText " SIZE_T_FORMAT ", FreeBytes " SIZE_T_FORMAT "\n",SMS.UDH.Length,UsedBytes,*FreeText,*UsedText,*FreeBytes);
}

GSM_Error GSM_AddSMS_Text_UDH(GSM_Debug_Info *di,
				GSM_MultiSMSMessage 	*SMS,
		      		GSM_Coding_Type		Coding,
		      		char 			*Buffer,
		      		size_t			BufferLen,
		      		bool 			UDH,
		      		size_t 			*UsedText,
		      		size_t			*CopiedText,
		      		size_t			*CopiedSMSText)
{
	size_t FreeText,FreeBytes,Copy,i,j;

	smfprintf(di, "Checking used\n");
	GSM_Find_Free_Used_SMS2(di, Coding,SMS->SMS[SMS->Number], UsedText, &FreeText, &FreeBytes);

	if (UDH) {
		smfprintf(di, "Adding UDH\n");
		if (FreeBytes - BufferLen <= 0) {
			smfprintf(di, "Going to the new SMS\n");
			SMS->Number++;
			GSM_Find_Free_Used_SMS2(di, Coding,SMS->SMS[SMS->Number], UsedText, &FreeText, &FreeBytes);
		}
		if (SMS->SMS[SMS->Number].UDH.Length == 0) {
			SMS->SMS[SMS->Number].UDH.Length  = 1;
			SMS->SMS[SMS->Number].UDH.Text[0] = 0x00;
		}
		memcpy(SMS->SMS[SMS->Number].UDH.Text+SMS->SMS[SMS->Number].UDH.Length,Buffer,BufferLen);
		SMS->SMS[SMS->Number].UDH.Length  	+= BufferLen;
		SMS->SMS[SMS->Number].UDH.Text[0] 	+= BufferLen;
		SMS->SMS[SMS->Number].UDH.Type 		=  UDH_UserUDH;
		smfprintf(di, "UDH added " SIZE_T_FORMAT "\n",BufferLen);
	} else {
		smfprintf(di, "Adding text\n");
		if (FreeText == 0) {
			smfprintf(di, "Going to the new SMS\n");
			SMS->Number++;
			GSM_Find_Free_Used_SMS2(di, Coding,SMS->SMS[SMS->Number], UsedText, &FreeText, &FreeBytes);
		}

		Copy = FreeText;
		smfprintf(di, "copy " SIZE_T_FORMAT "\n",Copy);
		if (BufferLen < Copy) Copy = BufferLen;
		smfprintf(di, "copy " SIZE_T_FORMAT "\n",Copy);

		switch (Coding) {
		case SMS_Coding_Default_No_Compression:
			FindDefaultAlphabetLen(Buffer,&i,&j,FreeText);
			smfprintf(di, "def length " SIZE_T_FORMAT " " SIZE_T_FORMAT "\n",i,j);
			SMS->SMS[SMS->Number].Text[UnicodeLength(SMS->SMS[SMS->Number].Text)*2+i*2]   = 0;
			SMS->SMS[SMS->Number].Text[UnicodeLength(SMS->SMS[SMS->Number].Text)*2+i*2+1] = 0;
			memcpy(SMS->SMS[SMS->Number].Text+UnicodeLength(SMS->SMS[SMS->Number].Text)*2,Buffer,i*2);
			*CopiedText 	= i;
			*CopiedSMSText 	= j;
			SMS->SMS[SMS->Number].Length += i;
			break;
		case SMS_Coding_Unicode_No_Compression:
			SMS->SMS[SMS->Number].Text[UnicodeLength(SMS->SMS[SMS->Number].Text)*2+Copy*2]   = 0;
			SMS->SMS[SMS->Number].Text[UnicodeLength(SMS->SMS[SMS->Number].Text)*2+Copy*2+1] = 0;
			memcpy(SMS->SMS[SMS->Number].Text+UnicodeLength(SMS->SMS[SMS->Number].Text)*2,Buffer,Copy*2);
			*CopiedText = *CopiedSMSText = Copy;
			SMS->SMS[SMS->Number].Length += Copy;
			break;
		case SMS_Coding_8bit:
			memcpy(SMS->SMS[SMS->Number].Text+SMS->SMS[SMS->Number].Length,Buffer,Copy);
			SMS->SMS[SMS->Number].Length += Copy;
			*CopiedText = *CopiedSMSText = Copy;
			break;
		default:
			break;
		}
		smfprintf(di, "Text added\n");
	}

	smfprintf(di, "Checking on the end\n");
	GSM_Find_Free_Used_SMS2(di, Coding,SMS->SMS[SMS->Number], UsedText, &FreeText, &FreeBytes);

	return ERR_NONE;
}

void GSM_MakeMultiPartSMS(GSM_Debug_Info *di, GSM_MultiSMSMessage	*SMS,
			  unsigned char		*MessageBuffer,
			  size_t		MessageLength,
			  GSM_UDH		UDHType,
			  GSM_Coding_Type	Coding,
			  int			Class,
			  unsigned char		ReplaceMessage)
{
	size_t 		Len,UsedText,CopiedText,CopiedSMSText;
	int		j;
	unsigned char 	UDHID;
	GSM_DateTime 	Date;

	Len = 0;
	while(1) {
		GSM_SetDefaultSMSData(&SMS->SMS[SMS->Number]);
		SMS->SMS[SMS->Number].Class    = Class;
		SMS->SMS[SMS->Number].Coding   = Coding;

		SMS->SMS[SMS->Number].UDH.Type = UDHType;
		GSM_EncodeUDHHeader(di, &SMS->SMS[SMS->Number].UDH);

		if (Coding == SMS_Coding_8bit) {
			GSM_AddSMS_Text_UDH(di, SMS,Coding,MessageBuffer+Len,MessageLength - Len,false,&UsedText,&CopiedText,&CopiedSMSText);
		} else {
			GSM_AddSMS_Text_UDH(di, SMS,Coding,MessageBuffer+Len*2,MessageLength - Len,false,&UsedText,&CopiedText,&CopiedSMSText);
		}
		Len += CopiedText;
		smfprintf(di, SIZE_T_FORMAT " " SIZE_T_FORMAT "\n",Len,MessageLength);
		if (Len == MessageLength) break;
		if (SMS->Number == GSM_MAX_MULTI_SMS) break;
		SMS->Number++;
	}

	SMS->Number++;

	UDHID = GSM_MakeSMSIDFromTime();
	GSM_GetCurrentDateTime (&Date);
	for (j=0;j<SMS->Number;j++) {
		SMS->SMS[j].UDH.Type 		= UDHType;
		SMS->SMS[j].UDH.ID8bit 		= UDHID;
		SMS->SMS[j].UDH.ID16bit		= UDHID + 256 * Date.Hour;
		SMS->SMS[j].UDH.PartNumber 	= j+1;
		SMS->SMS[j].UDH.AllParts 	= SMS->Number;
		GSM_EncodeUDHHeader(di, &SMS->SMS[j].UDH);
	}
	if (SMS->Number == 1) SMS->SMS[0].ReplaceMessage = ReplaceMessage;
}

/* Calculates number of SMS and number of left chars in SMS */
void GSM_SMSCounter(GSM_Debug_Info *di, size_t 		MessageLength,
		    unsigned char 	*MessageBuffer,
		    GSM_UDH	 	UDHType,
		    GSM_Coding_Type 	Coding,
		    int 		*SMSNum,
		    size_t 		*CharsLeft)
{
	size_t			UsedText,FreeBytes;
	GSM_MultiSMSMessage 	MultiSMS;

	MultiSMS.Number = 0;
	GSM_MakeMultiPartSMS(di, &MultiSMS,MessageBuffer,MessageLength,UDHType,Coding,-1,false);
	GSM_Find_Free_Used_SMS2(di, Coding,MultiSMS.SMS[MultiSMS.Number-1], &UsedText, CharsLeft, &FreeBytes);
	*SMSNum = MultiSMS.Number;
}

/* Nokia Smart Messaging 3.0 */
static void GSM_EncodeSMS30MultiPartSMS(GSM_MultiPartSMSInfo *Info,
					char *Buffer, size_t *Length)
{
	size_t len;

	/*SM version. Here 3.0*/
	Buffer[(*Length)++] = 0x30;

	if (Info->Entries[0].ID == SMS_NokiaProfileLong) {
		if (Info->Entries[0].Buffer != NULL) {
			if (Info->Entries[0].Buffer[0]!=0x00 || Info->Entries[0].Buffer[1]!=0x00) {
				Buffer[(*Length)++] = SM30_PROFILENAME;
				Buffer[(*Length)++] = 0x00;
				Buffer[(*Length)++] = 2*UnicodeLength(Info->Entries[0].Buffer);
				CopyUnicodeString(Buffer+(*Length),Info->Entries[0].Buffer);
				*Length = *Length + 2*UnicodeLength(Info->Entries[0].Buffer);
			}
		}
		if (Info->Entries[0].Ringtone != NULL) {
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
	}
	if (Info->Entries[0].Bitmap != NULL) {
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
				Buffer[(*Length)++] = UnicodeLength(Info->Entries[0].Bitmap->Bitmap[0].Text)*2;
				memcpy(Buffer+(*Length),Info->Entries[0].Bitmap->Bitmap[0].Text,UnicodeLength(Info->Entries[0].Bitmap->Bitmap[0].Text)*2);
				*Length = *Length + UnicodeLength(Info->Entries[0].Bitmap->Bitmap[0].Text)*2;
			} else {
				/*ID for ISO-8859-1 text*/
				Buffer[(*Length)++] = SM30_ISOTEXT;
				Buffer[(*Length)++] = 0x00;
				Buffer[(*Length)++] = UnicodeLength(Info->Entries[0].Bitmap->Bitmap[0].Text);
				memcpy(Buffer+(*Length),DecodeUnicodeString(Info->Entries[0].Bitmap->Bitmap[0].Text),UnicodeLength(Info->Entries[0].Bitmap->Bitmap[0].Text));
				*Length = *Length +UnicodeLength(Info->Entries[0].Bitmap->Bitmap[0].Text);
			}
		}
	}
}

/* Alcatel docs from www.alcatel.com/wap/ahead */
GSM_Error GSM_EncodeAlcatelMultiPartSMS(GSM_Debug_Info *di, GSM_MultiSMSMessage 	*SMS,
					unsigned char 		*Data,
					size_t			Len,
					unsigned char		*Name,
					size_t			Type)
{
	unsigned char 	buff[100],UDHID;
	size_t		i, p;
	GSM_UDHHeader	MyUDH;

	for (i=0;i<GSM_MAX_MULTI_SMS;i++) {
		GSM_SetDefaultSMSData(&SMS->SMS[i]);
		SMS->SMS[i].UDH.Type    = UDH_UserUDH;
		SMS->SMS[i].UDH.Text[1] = 0x80;	/* Alcatel */
		p 			= UnicodeLength(Name);
		EncodeDefault(buff, Name, &p, true, NULL);
		SMS->SMS[i].UDH.Text[2]	= GSM_PackSevenBitsToEight(0, buff, SMS->SMS[i].UDH.Text+3, p) + 4;
		SMS->SMS[i].UDH.Text[3]	= GSM_PackSevenBitsToEight(0, buff, SMS->SMS[i].UDH.Text+3, p);
		SMS->SMS[i].UDH.Text[4] = Type;
		SMS->SMS[i].UDH.Text[5] = Len / 256;
		SMS->SMS[i].UDH.Text[6] = Len % 256;
		SMS->SMS[i].UDH.Text[0] = 6 + SMS->SMS[i].UDH.Text[3];
		SMS->SMS[i].UDH.Length  = SMS->SMS[i].UDH.Text[0] + 1;

		if (Len > (size_t)(140 - SMS->SMS[i].UDH.Length)) {
			MyUDH.Type = UDH_ConcatenatedMessages;
			GSM_EncodeUDHHeader(di, &MyUDH);

			memcpy(SMS->SMS[i].UDH.Text+SMS->SMS[i].UDH.Length,MyUDH.Text+1,MyUDH.Length-1);
			SMS->SMS[i].UDH.Text[0] += MyUDH.Length-1;
			SMS->SMS[i].UDH.Length  += MyUDH.Length-1;
		}

		SMS->SMS[i].Coding = SMS_Coding_8bit;
		SMS->SMS[i].Class  = 1;
	}

	p = 0;
	while (p != Len) {
		i = 140-SMS->SMS[SMS->Number].UDH.Length;
		if (Len - p < i) i = Len - p;
		memcpy(SMS->SMS[SMS->Number].Text,Data+p,i);
		p += i;
		SMS->SMS[SMS->Number].Length = i;
		SMS->Number++;

	}

	/* Linked sms UDH */
	if (SMS->Number != 1) {
		UDHID = GSM_MakeSMSIDFromTime();
		for (i = 0; i < (size_t)SMS->Number; i++) {
			SMS->SMS[i].UDH.Text[SMS->SMS[i].UDH.Length-3] = UDHID;
			SMS->SMS[i].UDH.Text[SMS->SMS[i].UDH.Length-2] = SMS->Number;
			SMS->SMS[i].UDH.Text[SMS->SMS[i].UDH.Length-1] = i+1;
		}
	}

        return ERR_NONE;
}

/* Alcatel docs from www.alcatel.com/wap/ahead and other */
GSM_Error GSM_EncodeMultiPartSMS(GSM_Debug_Info *di,
				 GSM_MultiPartSMSInfo		*Info,
			    	 GSM_MultiSMSMessage		*SMS)
{
	unsigned char	Buffer[GSM_MAX_SMS_LENGTH*2*GSM_MAX_MULTI_SMS];
	unsigned char	Buffer2[GSM_MAX_SMS_LENGTH*2*GSM_MAX_MULTI_SMS];
	int		i, Class = -1, j;
	size_t p;
	size_t Length = 0, smslen;
	GSM_Error	error;
	GSM_Coding_Type Coding 	= SMS_Coding_8bit;
	GSM_UDH		UDH	= UDH_NoUDH;
	GSM_UDHHeader 	UDHHeader;
	bool		EMS	= false;
	int		textnum = 0;

	SMS->Number = 0;

	if (Info->Entries[0].ID == SMS_AlcatelSMSTemplateName) {
		Buffer[Length++] = 0x00; /* number of elements */
		for (i=1;i<Info->EntriesNum;i++) {
		switch (Info->Entries[i].ID) {
		case SMS_EMSSound10:
		case SMS_EMSSound12:
		case SMS_EMSSonyEricssonSound:
		case SMS_EMSSound10Long:
		case SMS_EMSSound12Long:
		case SMS_EMSSonyEricssonSoundLong:
		case SMS_EMSVariableBitmap:
		case SMS_EMSAnimation:
		case SMS_EMSVariableBitmapLong:
			break;
		case SMS_EMSPredefinedSound:
			Buffer[0]++;
			Buffer[Length++] 	= 0x01; 	/* type of data */
			Buffer[Length++] 	= 1 % 256;	/* len */
			Buffer[Length++] 	= 1 / 256;      /* len */
			Buffer[Length++] 	= Info->Entries[i].Number;
			break;
		case SMS_EMSPredefinedAnimation:
			Buffer[0]++;
			Buffer[Length++] 	= 0x02; 	/* type of data */
			Buffer[Length++] 	= 1 % 256;	/* len */
			Buffer[Length++] 	= 1 / 256;      /* len */
			Buffer[Length++] 	= Info->Entries[i].Number;
			break;
		case SMS_ConcatenatedTextLong:
			Buffer[0]++;
			p 	= UnicodeLength(Info->Entries[i].Buffer);
			EncodeDefault(Buffer2, Info->Entries[i].Buffer, &p, true, NULL);
			Buffer[Length++]   	= 0x00; 	/* type of data */
			Length 			= Length + 2;
			smslen			= GSM_PackSevenBitsToEight(0, Buffer2, Buffer+Length, p);
			Buffer[Length-2] 	= smslen % 256; /* len */
			Buffer[Length-1] 	= smslen / 256; /* len */
			Length 			= Length + smslen;
			break;
		default:
			return ERR_UNKNOWN;
		}
		}
		Buffer[0] = Buffer[0] * 2;
		return GSM_EncodeAlcatelMultiPartSMS(di, SMS,Buffer,Length,Info->Entries[0].Buffer,ALCATELTDD_SMSTEMPLATE);
	}

	for (i=0;i<Info->EntriesNum;i++) {
		switch (Info->Entries[i].ID) {
		case SMS_EMSPredefinedAnimation:
		case SMS_EMSPredefinedSound:
		case SMS_EMSSound10:
		case SMS_EMSSound12:
		case SMS_EMSSonyEricssonSound:
		case SMS_EMSSound10Long:
		case SMS_EMSSound12Long:
		case SMS_EMSSonyEricssonSoundLong:
		case SMS_EMSFixedBitmap:
		case SMS_EMSVariableBitmap:
		case SMS_EMSAnimation:
		case SMS_EMSVariableBitmapLong:
			EMS = true;
			break;
		case SMS_ConcatenatedTextLong:
		case SMS_ConcatenatedTextLong16bit:

			/* This covers situation, when somebody will call function
			 * with two or more SMS_Concatenated.... entries only.
			 * It will be still only linked sms, but functions below
			 * will pack only first entry according to own limits.
			 * We redirect to EMS functions, because they are more generic
			 * here and will handle it correctly and produce linked sms
			 * from all entries
			 */
			textnum ++;
			if (textnum > 1) EMS = true;

			if (Info->Entries[i].Left   || Info->Entries[i].Right      ||
			    Info->Entries[i].Center || Info->Entries[i].Large      ||
			    Info->Entries[i].Small  || Info->Entries[i].Bold       ||
			    Info->Entries[i].Italic || Info->Entries[i].Underlined ||
			    Info->Entries[i].Strikethrough) {
				EMS = true;
			}
		default:
			break;
		}
		if (EMS) break;
	}
	if (EMS) {
		error=GSM_EncodeEMSMultiPartSMS(di, Info,SMS,UDH_NoUDH);
		if (error != ERR_NONE) return error;
		if (SMS->Number != 1) {
			SMS->Number = 0;
			for (i=0;i<Info->EntriesNum;i++) {
				if (Info->Entries[i].ID == SMS_ConcatenatedTextLong16bit) {
					return GSM_EncodeEMSMultiPartSMS(di, Info,SMS,UDH_ConcatenatedMessages);
				}
			}
			return GSM_EncodeEMSMultiPartSMS(di, Info,SMS,UDH_ConcatenatedMessages16bit);
		}
		return error;
	}

	if (Info->EntriesNum != 1) return ERR_UNKNOWN;

	switch (Info->Entries[0].ID) {
	case SMS_AlcatelMonoBitmapLong:
		Buffer[0] = Info->Entries[0].Bitmap->Bitmap[0].BitmapWidth;
		Buffer[1] = Info->Entries[0].Bitmap->Bitmap[0].BitmapHeight;
		PHONE_EncodeBitmap(GSM_AlcatelBMMIPicture, Buffer+2, &Info->Entries[0].Bitmap->Bitmap[0]);
		Length = PHONE_GetBitmapSize(GSM_AlcatelBMMIPicture,Info->Entries[0].Bitmap->Bitmap[0].BitmapWidth,Info->Entries[0].Bitmap->Bitmap[0].BitmapHeight)+2;
		return GSM_EncodeAlcatelMultiPartSMS(di, SMS,Buffer,Length,Info->Entries[0].Bitmap->Bitmap[0].Text,ALCATELTDD_PICTURE);
	case SMS_AlcatelMonoAnimationLong:
		/* Number of sequence words */
		Buffer[0] = (Info->Entries[0].Bitmap->Number+1) % 256;
		Buffer[1] = (Info->Entries[0].Bitmap->Number+1) / 256;
		/* Picture display time 1 second (1 = 100ms) */
		Buffer[2] = 10 % 256;
		Buffer[3] = 10 / 256 + 0xF0;

		Length    = 4;
		j	  = 0;

		/* Offsets to bitmaps */
		for (i=0;i<Info->Entries[0].Bitmap->Number;i++) {
			Buffer[Length++] = (4+j+Info->Entries[0].Bitmap->Number*2) % 256;
			Buffer[Length++] = (4+j+Info->Entries[0].Bitmap->Number*2) / 256;
			j += PHONE_GetBitmapSize(GSM_AlcatelBMMIPicture,Info->Entries[0].Bitmap->Bitmap[i].BitmapWidth,Info->Entries[0].Bitmap->Bitmap[i].BitmapHeight)+2;
		}

		/* Bitmaps */
		for (i=0;i<Info->Entries[0].Bitmap->Number;i++) {
			Buffer[Length++] = Info->Entries[0].Bitmap->Bitmap[i].BitmapWidth;
			Buffer[Length++] = Info->Entries[0].Bitmap->Bitmap[i].BitmapHeight;
			PHONE_EncodeBitmap(GSM_AlcatelBMMIPicture, Buffer+Length, &Info->Entries[0].Bitmap->Bitmap[i]);
			Length += PHONE_GetBitmapSize(GSM_AlcatelBMMIPicture,Info->Entries[0].Bitmap->Bitmap[i].BitmapWidth,Info->Entries[0].Bitmap->Bitmap[i].BitmapHeight);
		}
		return GSM_EncodeAlcatelMultiPartSMS(di, SMS,Buffer,Length,Info->Entries[0].Bitmap->Bitmap[0].Text,ALCATELTDD_ANIMATION);
	case SMS_MMSIndicatorLong:
		Class	= 1;
		UDH	= UDH_MMSIndicatorLong;
		GSM_EncodeMMSIndicatorSMSText(Buffer,&Length,*Info->Entries[0].MMSIndicator);
		break;
	case SMS_WAPIndicatorLong:
		Class	= 1;
		UDH	= UDH_MMSIndicatorLong;
		GSM_EncodeWAPIndicatorSMSText(Buffer,&Length,Info->Entries[0].MMSIndicator->Title,Info->Entries[0].MMSIndicator->Address);
		break;
	case SMS_NokiaRingtoneLong:
	case SMS_NokiaRingtone:
		UDH	= UDH_NokiaRingtone;
		Class	= 1;
		/* 7 = length of UDH_NokiaRingtone UDH header */
		Length  = GSM_MAX_8BIT_SMS_LENGTH-7;
		Info->Entries[0].RingtoneNotes = GSM_EncodeNokiaRTTLRingtone(*Info->Entries[0].Ringtone,Buffer,&Length);
		if (Info->Entries[0].ID == SMS_NokiaRingtone) break;
		if (Info->Entries[0].RingtoneNotes != Info->Entries[0].Ringtone->NoteTone.NrCommands) {
			UDH    = UDH_NokiaRingtoneLong;
			Length = (GSM_MAX_8BIT_SMS_LENGTH-12)*3;
			Info->Entries[0].RingtoneNotes = GSM_EncodeNokiaRTTLRingtone(*Info->Entries[0].Ringtone,Buffer,&Length);
		}
		break;
	case SMS_NokiaOperatorLogoLong:
		if (Info->Entries[0].Bitmap->Bitmap[0].BitmapWidth > 72 || Info->Entries[0].Bitmap->Bitmap[0].BitmapHeight > 14) {
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
		NOKIA_EncodeWAPBookmarkSMSText(Buffer,&Length,Info->Entries[0].Bookmark);
		/* 7 = length of UDH_NokiaWAP UDH header */
		if (Length>(GSM_MAX_8BIT_SMS_LENGTH-7)) {
			UDH=UDH_NokiaWAPLong;
		} else {
			UDH=UDH_NokiaWAP;
		}
		break;
	case SMS_NokiaWAPSettingsLong:
		Class	= 1;
		UDH	= UDH_NokiaWAPLong;
		NOKIA_EncodeWAPMMSSettingsSMSText(Buffer,&Length,Info->Entries[0].Settings,false);
		break;
	case SMS_NokiaMMSSettingsLong:
		Class	= 1;
		UDH	= UDH_NokiaWAPLong;
		NOKIA_EncodeWAPMMSSettingsSMSText(Buffer,&Length,Info->Entries[0].Settings,true);
		break;
	case SMS_NokiaVCARD10Long:
		error = GSM_EncodeVCARD(di, Buffer, sizeof(Buffer), &Length,Info->Entries[0].Phonebook,true,Nokia_VCard10);
		if (error != ERR_NONE) return error;
		/* is 1 SMS ? 8 = length of ..SCKE2 */
		if (Length<=GSM_MAX_SMS_LENGTH-8) {
			sprintf(Buffer,"//SCKE2 ");
			Length = 8;
			error = GSM_EncodeVCARD(di, Buffer, sizeof(Buffer), &Length,Info->Entries[0].Phonebook,true,Nokia_VCard10);
			if (error != ERR_NONE) return error;
		} else {
			/* FIXME: It wasn't checked */
			UDH = UDH_NokiaPhonebookLong;
		}
		Coding = SMS_Coding_Default_No_Compression;
		memcpy(Buffer2,Buffer,Length);
		EncodeUnicode(Buffer,Buffer2,Length);
		break;
	case SMS_NokiaVCARD21Long:
		error = GSM_EncodeVCARD(di, Buffer, sizeof(Buffer), &Length,Info->Entries[0].Phonebook,true,Nokia_VCard21);
		if (error != ERR_NONE) return error;
		/* Is 1 SMS ? 12 = length of ..SCKL23F4 */
		if (Length<=GSM_MAX_SMS_LENGTH-12) {
			sprintf(Buffer,"//SCKL23F4%c%c",13,10);
			Length = 12;
			error = GSM_EncodeVCARD(di, Buffer, sizeof(Buffer), &Length,Info->Entries[0].Phonebook,true,Nokia_VCard21);
			if (error != ERR_NONE) return error;
		} else {
			UDH = UDH_NokiaPhonebookLong;
			/* Here can be also 8 bit coding */
		}
		Coding = SMS_Coding_Default_No_Compression;
		memcpy(Buffer2,Buffer,Length);
		EncodeUnicode(Buffer,Buffer2,Length);
		break;
	case SMS_VCARD10Long:
		error = GSM_EncodeVCARD(di, Buffer, sizeof(Buffer), &Length,Info->Entries[0].Phonebook,true,Nokia_VCard10);
		if (error != ERR_NONE) return error;
		if (Length>GSM_MAX_SMS_LENGTH) UDH = UDH_ConcatenatedMessages;
		Coding = SMS_Coding_Default_No_Compression;
		memcpy(Buffer2,Buffer,Length);
		EncodeUnicode(Buffer,Buffer2,Length);
		break;
	case SMS_VCARD21Long:
		error = GSM_EncodeVCARD(di, Buffer, sizeof(Buffer), &Length,Info->Entries[0].Phonebook,true,Nokia_VCard21);
		if (error != ERR_NONE) return error;
		if (Length>GSM_MAX_SMS_LENGTH) UDH = UDH_ConcatenatedMessages;
		Coding = SMS_Coding_Default_No_Compression;
		memcpy(Buffer2,Buffer,Length);
		EncodeUnicode(Buffer,Buffer2,Length);
		break;
	case SMS_NokiaVCALENDAR10Long:
		error=GSM_EncodeVCALENDAR(Buffer, sizeof(Buffer),&Length,Info->Entries[0].Calendar,true,Nokia_VCalendar);
		if (error != ERR_NONE) return error;
		/* Is 1 SMS ? 8 = length of ..SCKE4 */
		if (Length<=GSM_MAX_SMS_LENGTH-8) {
			sprintf(Buffer,"//SCKE4 ");
			Length = 8;
			GSM_EncodeVCALENDAR(Buffer, sizeof(Buffer),&Length,Info->Entries[0].Calendar,true,Nokia_VCalendar);
		} else {
			UDH = UDH_NokiaCalendarLong;
			/* can be here 8 bit coding ? */
		}
		Coding = SMS_Coding_Default_No_Compression;
		memcpy(Buffer2,Buffer,Length);
		EncodeUnicode(Buffer,Buffer2,Length);
		break;
	case SMS_NokiaVTODOLong:
		error=GSM_EncodeVTODO(Buffer, sizeof(Buffer),&Length,Info->Entries[0].ToDo,true,Nokia_VToDo);
		if (error != ERR_NONE) return error;
		UDH = UDH_NokiaCalendarLong;
		Coding = SMS_Coding_Default_No_Compression;
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
		GSM_EncodeUDHHeader(di, &UDHHeader);
		memcpy(Buffer,Info->Entries[0].Buffer,UnicodeLength(Info->Entries[0].Buffer)*2+2);
		if (Info->UnicodeCoding) {
			Coding = SMS_Coding_Unicode_No_Compression;
			Length = UnicodeLength(Info->Entries[0].Buffer);
			if (Length > (size_t)(140 - UDHHeader.Length) / 2) {
				Length = (140 - UDHHeader.Length) / 2;
			}
		} else {
			Coding = SMS_Coding_Default_No_Compression;
			FindDefaultAlphabetLen(Info->Entries[0].Buffer,&Length,&smslen,(GSM_MAX_8BIT_SMS_LENGTH-UDHHeader.Length)*8/7);
		}
		break;
	case SMS_ConcatenatedAutoTextLong:
	case SMS_ConcatenatedAutoTextLong16bit:
		smslen = UnicodeLength(Info->Entries[0].Buffer);
		memcpy(Buffer,Info->Entries[0].Buffer,smslen*2);
		EncodeDefault(Buffer2, Buffer, &smslen, true, NULL);
		DecodeDefault(Buffer,  Buffer2, smslen, true, NULL);
#ifdef DEBUG
		if (GSM_global_debug.dl == DL_TEXTALL || GSM_global_debug.dl == DL_TEXTALLDATE) {
			smfprintf(di, "Info->Entries[0].Buffer:\n");
			DumpMessage(&GSM_global_debug, Info->Entries[0].Buffer, UnicodeLength(Info->Entries[0].Buffer)*2);
			smfprintf(di, "Buffer:\n");
			DumpMessage(&GSM_global_debug, Buffer, UnicodeLength(Buffer)*2);
		}
#endif
		Info->UnicodeCoding = false;
		for (smslen = 0; smslen < UnicodeLength(Info->Entries[0].Buffer) * 2; smslen++) {
			if (Info->Entries[0].Buffer[smslen] != Buffer[smslen]) {
				Info->UnicodeCoding = true;
				smfprintf(di, "Setting to Unicode " SIZE_T_FORMAT "\n",smslen);
				break;
			}
		}
		/* No break here - we go to the SMS_ConcatenatedTextLong */
	case SMS_ConcatenatedTextLong:
	case SMS_ConcatenatedTextLong16bit:
		Class = Info->Class;
		memcpy(Buffer,Info->Entries[0].Buffer,UnicodeLength(Info->Entries[0].Buffer)*2+2);
		UDH = UDH_NoUDH;
		if (Info->UnicodeCoding) {
			Coding = SMS_Coding_Unicode_No_Compression;
			Length = UnicodeLength(Info->Entries[0].Buffer);
			if (Info->Entries[0].ID == SMS_ConcatenatedTextLong16bit ||
			    Info->Entries[0].ID == SMS_ConcatenatedAutoTextLong16bit) {
				if (Length>70) UDH=UDH_ConcatenatedMessages16bit;
			} else {
				if (Length>70) UDH=UDH_ConcatenatedMessages;
			}
		} else {
			Coding = SMS_Coding_Default_No_Compression;
			FindDefaultAlphabetLen(Info->Entries[0].Buffer,&Length,&smslen,5000);
			if (Info->Entries[0].ID == SMS_ConcatenatedTextLong16bit ||
			    Info->Entries[0].ID == SMS_ConcatenatedAutoTextLong16bit) {
				if (smslen>GSM_MAX_SMS_LENGTH) UDH=UDH_ConcatenatedMessages16bit;
			} else {
				if (smslen>GSM_MAX_SMS_LENGTH) UDH=UDH_ConcatenatedMessages;
			}
		}
	default:
		break;
	}
	GSM_MakeMultiPartSMS(di, SMS,Buffer,Length,UDH,Coding,Class,Info->ReplaceMessage);
	return ERR_NONE;
}

void GSM_ClearMultiPartSMSInfo(GSM_MultiPartSMSInfo *Info)
{
	int i;

	for (i=0;i<GSM_MAX_MULTI_SMS;i++) {
		Info->Entries[i].Number		= 0;
		Info->Entries[i].Ringtone	= NULL;
		Info->Entries[i].Bitmap		= NULL;
		Info->Entries[i].Bookmark	= NULL;
		Info->Entries[i].File		= NULL;
		Info->Entries[i].Settings	= NULL;
		Info->Entries[i].MMSIndicator	= NULL;
		Info->Entries[i].Phonebook	= NULL;
		Info->Entries[i].Calendar	= NULL;
		Info->Entries[i].ToDo		= NULL;
		Info->Entries[i].Protected	= false;

		Info->Entries[i].Buffer		= NULL;
		Info->Entries[i].Left		= false;
		Info->Entries[i].Right		= false;
		Info->Entries[i].Center		= false;
		Info->Entries[i].Large		= false;
		Info->Entries[i].Small		= false;
		Info->Entries[i].Bold		= false;
		Info->Entries[i].Italic		= false;
		Info->Entries[i].Underlined	= false;
		Info->Entries[i].Strikethrough	= false;

		Info->Entries[i].RingtoneNotes	= 0;
	}
	Info->Unknown		= false;
	Info->EntriesNum	= 0;
	Info->Class		= -1;
	Info->ReplaceMessage	= 0;
	Info->UnicodeCoding	= false;
}

void GSM_FreeMultiPartSMSInfo(GSM_MultiPartSMSInfo *Info)
{
	int i;

	for (i=0;i<GSM_MAX_MULTI_SMS;i++) {
		free(Info->Entries[i].Ringtone);
		Info->Entries[i].Ringtone = NULL;
		free(Info->Entries[i].Bitmap);
		Info->Entries[i].Bitmap = NULL;
		free(Info->Entries[i].Bookmark);
		Info->Entries[i].Bookmark = NULL;
		free(Info->Entries[i].Settings);
		Info->Entries[i].Settings = NULL;
		free(Info->Entries[i].MMSIndicator);
		Info->Entries[i].MMSIndicator = NULL;
		free(Info->Entries[i].Phonebook);
		Info->Entries[i].Phonebook = NULL;
		free(Info->Entries[i].Calendar);
		Info->Entries[i].Calendar = NULL;
		free(Info->Entries[i].ToDo);
		Info->Entries[i].ToDo = NULL;
		free(Info->Entries[i].Buffer);
		Info->Entries[i].Buffer = NULL;
	}
}

/* ----------------- Joining SMS from parts -------------------------------- */

bool GSM_DecodeMultiPartSMS(GSM_Debug_Info *di,
			    GSM_MultiPartSMSInfo	*Info,
			    GSM_MultiSMSMessage		*SMS,
			    bool			ems)
{
	int 			i, Length = 0;
	unsigned int		j;
	char			Buffer[GSM_MAX_SMS_LENGTH*2*GSM_MAX_MULTI_SMS];
	bool 			emsexist = false;
	GSM_SiemensOTASMSInfo	SiemensInfo;

	GSM_ClearMultiPartSMSInfo(Info);
	if (ems) {
		emsexist = true;
		for (i=0;i<SMS->Number;i++) {
			if (SMS->SMS[i].UDH.Type != UDH_ConcatenatedMessages 		&&
			    SMS->SMS[i].UDH.Type != UDH_ConcatenatedMessages16bit 	&&
			    SMS->SMS[i].UDH.Type != UDH_UserUDH) {
				emsexist = false;
				break;
			}
		}
	}

	/* EMS decoding */
	if (emsexist) return GSM_DecodeEMSMultiPartSMS(di, Info,SMS);

	/* Siemens OTA */
	if (GSM_DecodeSiemensOTASMS(di, &SiemensInfo,&SMS->SMS[0])) {
		Info->Entries[0].File = (GSM_File *)malloc(sizeof(GSM_File));
		if (Info->Entries[0].File == NULL) return false;
		Info->Entries[0].File->Buffer 	= NULL;
		Info->Entries[0].File->Used 	= 0;
		for (i=0;i<SMS->Number;i++) {
			GSM_DecodeSiemensOTASMS(di, &SiemensInfo,&SMS->SMS[i]);
			j = SiemensInfo.AllDataLen - Info->Entries[0].File->Used;
			if (j>SiemensInfo.DataLen) j = SiemensInfo.DataLen;
			Info->Entries[0].File->Buffer = realloc(Info->Entries[0].File->Buffer,j+Info->Entries[0].File->Used);
			memcpy(Info->Entries[0].File->Buffer+Info->Entries[0].File->Used,SiemensInfo.Data,j);
			Info->Entries[0].File->Used += j;
		}
		if (SiemensInfo.AllDataLen == Info->Entries[0].File->Used) {
			Info->Entries[0].ID 	= SMS_SiemensFile;
			Info->EntriesNum	= 1;
			EncodeUnicode(Info->Entries[0].File->Name,SiemensInfo.DataName,strlen(SiemensInfo.DataName));
			return true;
		}
		free(Info->Entries[0].File->Buffer);
	}

	/* Smart Messaging decoding */
	if (SMS->SMS[0].UDH.Type == UDH_NokiaRingtone && SMS->Number == 1) {
		Info->Entries[0].Ringtone = (GSM_Ringtone *)malloc(sizeof(GSM_Ringtone));
		if (Info->Entries[0].Ringtone == NULL) return false;
		if (GSM_DecodeNokiaRTTLRingtone(Info->Entries[0].Ringtone, SMS->SMS[0].Text, SMS->SMS[0].Length)==ERR_NONE) {
			Info->Entries[0].ID 	= SMS_NokiaRingtone;
			Info->EntriesNum	= 1;
			return true;
		}
	}
	if (SMS->SMS[0].UDH.Type == UDH_NokiaCallerLogo && SMS->Number == 1) {
		Info->Entries[0].Bitmap = (GSM_MultiBitmap *)malloc(sizeof(GSM_MultiBitmap));
		if (Info->Entries[0].Bitmap == NULL) return false;
		Info->Entries[0].Bitmap->Number = 1;
		PHONE_DecodeBitmap(GSM_NokiaCallerLogo, SMS->SMS[0].Text+4, &Info->Entries[0].Bitmap->Bitmap[0]);
#ifdef DEBUG
		if (GSM_global_debug.dl == DL_TEXTALL || GSM_global_debug.dl == DL_TEXTALLDATE)
			GSM_PrintBitmap(GSM_global_debug.df,&Info->Entries[0].Bitmap->Bitmap[0]);
#endif
		Info->Entries[0].ID 	= SMS_NokiaCallerLogo;
		Info->EntriesNum	= 1;
		return true;
	}
	if (SMS->SMS[0].UDH.Type == UDH_NokiaOperatorLogo && SMS->Number == 1) {
		Info->Entries[0].Bitmap = (GSM_MultiBitmap *)malloc(sizeof(GSM_MultiBitmap));
		if (Info->Entries[0].Bitmap == NULL) return false;
		Info->Entries[0].Bitmap->Number = 1;
		PHONE_DecodeBitmap(GSM_NokiaOperatorLogo, SMS->SMS[0].Text+7, &Info->Entries[0].Bitmap->Bitmap[0]);
		NOKIA_DecodeNetworkCode(SMS->SMS[0].Text, Info->Entries[0].Bitmap->Bitmap[0].NetworkCode);
#ifdef DEBUG
		if (GSM_global_debug.dl == DL_TEXTALL || GSM_global_debug.dl == DL_TEXTALLDATE)
			GSM_PrintBitmap(GSM_global_debug.df,&Info->Entries[0].Bitmap->Bitmap[0]);
#endif
		Info->Entries[0].ID 	= SMS_NokiaOperatorLogo;
		Info->EntriesNum	= 1;
		return true;
	}
	if (SMS->SMS[0].UDH.Type == UDH_NokiaProfileLong) {
		for (i=0;i<SMS->Number;i++) {
			if (SMS->SMS[i].UDH.Type != UDH_NokiaProfileLong ||
			    SMS->SMS[i].UDH.Text[11] != i+1		 ||
			    SMS->SMS[i].UDH.Text[10] != SMS->Number) {
				return false;
			}
			memcpy(Buffer+Length,SMS->SMS[i].Text,SMS->SMS[i].Length);
			Length = Length + SMS->SMS[i].Length;
		}
		Info->EntriesNum    = 1;
		Info->Entries[0].ID = SMS_NokiaPictureImageLong;
		Info->Entries[0].Bitmap = (GSM_MultiBitmap *)malloc(sizeof(GSM_MultiBitmap));
		if (Info->Entries[0].Bitmap == NULL) return false;
		Info->Entries[0].Bitmap->Number = 1;
		Info->Entries[0].Bitmap->Bitmap[0].Text[0] = 0;
		Info->Entries[0].Bitmap->Bitmap[0].Text[1] = 0;
		i=1;
		while (i!=Length) {
			switch (Buffer[i]) {
			case SM30_ISOTEXT:
				smfprintf(di, "ISO 8859-2 text\n");
				break;
			case SM30_UNICODETEXT:
				smfprintf(di, "Unicode text\n");
				break;
			case SM30_OTA:
				smfprintf(di, "OTA bitmap as Picture Image\n");
				PHONE_DecodeBitmap(GSM_NokiaPictureImage, Buffer + i + 7, &Info->Entries[0].Bitmap->Bitmap[0]);
#ifdef DEBUG
				if (GSM_global_debug.dl == DL_TEXTALL || GSM_global_debug.dl == DL_TEXTALLDATE)
					GSM_PrintBitmap(GSM_global_debug.df,&Info->Entries[0].Bitmap->Bitmap[0]);
#endif
				break;
			case SM30_RINGTONE:
				smfprintf(di, "RTTL ringtone\n");
				Info->Unknown = true;
				break;
			case SM30_PROFILENAME:
				smfprintf(di, "Profile Name\n");
				Info->Entries[0].ID = SMS_NokiaProfileLong;
				Info->Unknown = true;
				break;
			case SM30_SCREENSAVER:
				smfprintf(di, "OTA bitmap as Screen Saver\n");
				PHONE_DecodeBitmap(GSM_NokiaPictureImage, Buffer + i + 7, &Info->Entries[0].Bitmap->Bitmap[0]);
#ifdef DEBUG
				if (GSM_global_debug.dl == DL_TEXTALL || GSM_global_debug.dl == DL_TEXTALLDATE)
					GSM_PrintBitmap(GSM_global_debug.df,&Info->Entries[0].Bitmap->Bitmap[0]);
#endif
				Info->Entries[0].ID = SMS_NokiaScreenSaverLong;
				break;
			}
			i = i + Buffer[i+1]*256 + Buffer[i+2] + 3;
			smfprintf(di, "%i %i\n",i,Length);
		}
		i=1;
		while (i!=Length) {
			switch (Buffer[i]) {
			case SM30_ISOTEXT:
				smfprintf(di, "ISO 8859-2 text\n");
				EncodeUnicode (Info->Entries[0].Bitmap->Bitmap[0].Text, Buffer+i+3, Buffer[i+2]);
				smfprintf(di, "ISO Text \"%s\"\n",DecodeUnicodeString(Info->Entries[0].Bitmap->Bitmap[0].Text));
				break;
			case SM30_UNICODETEXT:
				smfprintf(di, "Unicode text\n");
				memcpy(Info->Entries[0].Bitmap->Bitmap[0].Text,Buffer+i+3,Buffer[i+1]*256+Buffer[i+2]);
				Info->Entries[0].Bitmap->Bitmap[0].Text[Buffer[i+1]*256 + Buffer[i+2]] 	= 0;
				Info->Entries[0].Bitmap->Bitmap[0].Text[Buffer[i+1]*256 + Buffer[i+2]+ 1] 	= 0;
				smfprintf(di, "Unicode Text \"%s\"\n",DecodeUnicodeString(Info->Entries[0].Bitmap->Bitmap[0].Text));
				break;
			case SM30_OTA:
				smfprintf(di, "OTA bitmap as Picture Image\n");
				break;
			case SM30_RINGTONE:
				smfprintf(di, "RTTL ringtone\n");
				break;
			case SM30_PROFILENAME:
				smfprintf(di, "Profile Name\n");
				break;
			case SM30_SCREENSAVER:
				smfprintf(di, "OTA bitmap as Screen Saver\n");
				break;
			}
			i = i + Buffer[i+1]*256 + Buffer[i+2] + 3;
			smfprintf(di, "%i %i\n",i,Length);
		}
		return true;
	}

	/* Linked sms */
	if (SMS->SMS[0].UDH.Type == UDH_ConcatenatedMessages ||
	    SMS->SMS[0].UDH.Type == UDH_ConcatenatedMessages16bit) {
		Info->EntriesNum    = 1;
		Info->Entries[0].ID = SMS_ConcatenatedTextLong;
	 	if (SMS->SMS[0].UDH.Type == UDH_ConcatenatedMessages16bit) {
			Info->Entries[0].ID = SMS_ConcatenatedTextLong16bit;
		}

		for (i=0;i<SMS->Number;i++) {
			switch (SMS->SMS[i].Coding) {
			case SMS_Coding_8bit:
				Info->Entries[0].Buffer = realloc(Info->Entries[0].Buffer, Length + SMS->SMS[i].Length + 2);
				if (Info->Entries[0].Buffer == NULL) return false;

				memcpy(Info->Entries[0].Buffer + Length, SMS->SMS[i].Text, SMS->SMS[i].Length);
				Length=Length+SMS->SMS[i].Length;
				break;
			case SMS_Coding_Unicode_No_Compression:
				if (Info->Entries[0].ID == SMS_ConcatenatedTextLong) {
					Info->Entries[0].ID = SMS_ConcatenatedAutoTextLong;
				}
				if (Info->Entries[0].ID == SMS_ConcatenatedTextLong16bit) {
					Info->Entries[0].ID = SMS_ConcatenatedAutoTextLong16bit;
				}
			case SMS_Coding_Default_No_Compression:
				Info->Entries[0].Buffer = realloc(Info->Entries[0].Buffer, Length + UnicodeLength(SMS->SMS[i].Text)*2 + 2);
				if (Info->Entries[0].Buffer == NULL) return false;

				memcpy(Info->Entries[0].Buffer+Length,SMS->SMS[i].Text,UnicodeLength(SMS->SMS[i].Text)*2);
				Length=Length+UnicodeLength(SMS->SMS[i].Text)*2;
				break;
			default:
				break;
			}
		}
		Info->Entries[0].Buffer[Length]	  = 0;
		Info->Entries[0].Buffer[Length+1] = 0;
		return true;
	}

	return false;
}

GSM_Error GSM_LinkSMS(GSM_Debug_Info *di, GSM_MultiSMSMessage **INPUT, GSM_MultiSMSMessage **OUTPUT, bool ems)
{
	bool			*INPUTSorted, copyit,OtherNumbers[GSM_SMS_OTHER_NUMBERS+1],wrong=false;
	int			i,OUTPUTNum,z,w,m,p;
	int			j;
	GSM_SiemensOTASMSInfo	SiemensOTA,SiemensOTA2;

	i = 0;
	while (INPUT[i] != NULL) i++;

	INPUTSorted = calloc(i, sizeof(bool));
	if (INPUTSorted == NULL) return ERR_MOREMEMORY;

	OUTPUTNum = 0;
	OUTPUT[0] = NULL;

	if (ems) {
		i=0;
		while (INPUT[i] != NULL) {
			if (INPUT[i]->SMS[0].UDH.Type == UDH_UserUDH) {
				w=1;
				while (w < INPUT[i]->SMS[0].UDH.Length) {
					switch(INPUT[i]->SMS[0].UDH.Text[w]) {
					case 0x00:
						smfprintf(di, "Adding ID to user UDH - linked SMS with 8 bit ID\n");
						INPUT[i]->SMS[0].UDH.ID8bit	= INPUT[i]->SMS[0].UDH.Text[w+2];
						INPUT[i]->SMS[0].UDH.ID16bit	= -1;
						INPUT[i]->SMS[0].UDH.AllParts	= INPUT[i]->SMS[0].UDH.Text[w+3];
						INPUT[i]->SMS[0].UDH.PartNumber	= INPUT[i]->SMS[0].UDH.Text[w+4];
						break;
					case 0x08:
						smfprintf(di, "Adding ID to user UDH - linked SMS with 16 bit ID\n");
						INPUT[i]->SMS[0].UDH.ID8bit	= -1;
						INPUT[i]->SMS[0].UDH.ID16bit	= INPUT[i]->SMS[0].UDH.Text[w+2]*256+INPUT[i]->SMS[0].UDH.Text[w+3];
						INPUT[i]->SMS[0].UDH.AllParts	= INPUT[i]->SMS[0].UDH.Text[w+4];
						INPUT[i]->SMS[0].UDH.PartNumber	= INPUT[i]->SMS[0].UDH.Text[w+5];
						break;
					default:
						smfprintf(di, "Block %02x\n",INPUT[i]->SMS[0].UDH.Text[w]);
					}
					smfprintf(di, "%i %i %i %i\n",
						INPUT[i]->SMS[0].UDH.ID8bit,
						INPUT[i]->SMS[0].UDH.ID16bit,
						INPUT[i]->SMS[0].UDH.PartNumber,
						INPUT[i]->SMS[0].UDH.AllParts);
					w=w+INPUT[i]->SMS[0].UDH.Text[w+1]+2;
				}
			}
			i++;
		}
	}

	i=0;
	while (INPUT[i]!=NULL) {
		/* If this one SMS was sorted earlier, do not touch */
		if (INPUTSorted[i]) {
			i++;
			continue;
		}
		/* We have 1'st part of SIEMENS sms. It's single.
		 * We will try to find other parts
		 */
		if (GSM_DecodeSiemensOTASMS(di, &SiemensOTA,&INPUT[i]->SMS[0]) &&
		    SiemensOTA.PacketNum == 1) {
			OUTPUT[OUTPUTNum] = malloc(sizeof(GSM_MultiSMSMessage));
			if (OUTPUT[OUTPUTNum] == NULL) {
				free(INPUTSorted);
				return ERR_MOREMEMORY;
			}
			OUTPUT[OUTPUTNum+1] = NULL;

			memcpy(&OUTPUT[OUTPUTNum]->SMS[0],&INPUT[i]->SMS[0],sizeof(GSM_SMSMessage));
			OUTPUT[OUTPUTNum]->Number = 1;
			INPUTSorted[i]	= true;
			j		= 1;
			/* We're searching for other parts in sequence */
			while (j!=(int)SiemensOTA.PacketsNum) {
				z=0;
				while(INPUT[z]!=NULL) {
					/* This was sorted earlier or is not single */
					if (INPUTSorted[z] || INPUT[z]->Number != 1) {
						z++;
						continue;
					}
					if (!GSM_DecodeSiemensOTASMS(di, &SiemensOTA2,&INPUT[z]->SMS[0])) {
						z++;
						continue;
					}
					if (SiemensOTA2.SequenceID != SiemensOTA.SequenceID ||
					    (int)SiemensOTA2.PacketNum != j+1 ||
					    SiemensOTA2.PacketsNum != SiemensOTA.PacketsNum ||
					    strcmp(SiemensOTA2.DataType,SiemensOTA.DataType) ||
					    strcmp(SiemensOTA2.DataName,SiemensOTA.DataName)) {
						z++;
						continue;
					}
					/* For SMS_Deliver compare also SMSC and Sender numbers */
					if (INPUT[z]->SMS[0].PDU == SMS_Deliver &&
					    strcmp(DecodeUnicodeString(INPUT[z]->SMS[0].SMSC.Number),DecodeUnicodeString(INPUT[i]->SMS[0].SMSC.Number))) {
						z++;
						continue;
					}
					if (INPUT[z]->SMS[0].PDU == SMS_Deliver &&
					    INPUT[z]->SMS[0].OtherNumbersNum!=INPUT[i]->SMS[0].OtherNumbersNum) {
						z++;
						continue;
					}
					if (INPUT[z]->SMS[0].PDU == SMS_Deliver) {
						for (m=0;m<GSM_SMS_OTHER_NUMBERS+1;m++) {
							OtherNumbers[m]=false;
						}
						for (m=0;m<INPUT[z]->SMS[0].OtherNumbersNum+1;m++) {
							wrong=true;
							for (p=0;p<INPUT[i]->SMS[0].OtherNumbersNum+1;p++) {
								if (OtherNumbers[p]) continue;
								if (m==0 && p==0 && !strcmp(DecodeUnicodeString(INPUT[z]->SMS[0].Number),DecodeUnicodeString(INPUT[i]->SMS[0].Number))) {
									OtherNumbers[0]=true;
									wrong=false;
									break;
								}
								if (m==0 && p!=0 && !strcmp(DecodeUnicodeString(INPUT[z]->SMS[0].Number),DecodeUnicodeString(INPUT[i]->SMS[0].OtherNumbers[p-1]))) {
									OtherNumbers[p]=true;
									wrong=false;
									break;
								}
								if (m!=0 && p==0 && !strcmp(DecodeUnicodeString(INPUT[z]->SMS[0].OtherNumbers[m-1]),DecodeUnicodeString(INPUT[i]->SMS[0].Number))) {
									OtherNumbers[0]=true;
									wrong=false;
									break;
								}
								if (m!=0 && p!=0 && !strcmp(DecodeUnicodeString(INPUT[z]->SMS[0].OtherNumbers[m-1]),DecodeUnicodeString(INPUT[i]->SMS[0].OtherNumbers[p-1]))) {
									OtherNumbers[p]=true;
									wrong=false;
									break;
								}
							}
							if (wrong) break;
						}
						if (wrong) {
							z++;
							continue;
						}
					}
					/* DCT4 Outbox: SMS Deliver. Empty number and SMSC. We compare dates */
					if (INPUT[z]->SMS[0].PDU == SMS_Deliver 		&&
					    UnicodeLength(INPUT[z]->SMS[0].SMSC.Number)==0 	&&
					    UnicodeLength(INPUT[z]->SMS[0].Number)==0 		&&
					    (INPUT[z]->SMS[0].DateTime.Day    != INPUT[i]->SMS[0].DateTime.Day 	  ||
	   				     INPUT[z]->SMS[0].DateTime.Month  != INPUT[i]->SMS[0].DateTime.Month  ||
					     INPUT[z]->SMS[0].DateTime.Year   != INPUT[i]->SMS[0].DateTime.Year   ||
					     INPUT[z]->SMS[0].DateTime.Hour   != INPUT[i]->SMS[0].DateTime.Hour   ||
					     INPUT[z]->SMS[0].DateTime.Minute != INPUT[i]->SMS[0].DateTime.Minute ||
					     INPUT[z]->SMS[0].DateTime.Second != INPUT[i]->SMS[0].DateTime.Second)) {
						z++;
						continue;
					}
					smfprintf(di, "Found Siemens SMS %i\n",j);
					/* We found correct sms. Copy it */
					memcpy(&OUTPUT[OUTPUTNum]->SMS[j],&INPUT[z]->SMS[0],sizeof(GSM_SMSMessage));
					OUTPUT[OUTPUTNum]->Number++;
					INPUTSorted[z]=true;
					break;
				}
				/* Incomplete sequence */
				if (OUTPUT[OUTPUTNum]->Number==j) {
					smfprintf(di, "Incomplete sequence\n");
					break;
				}
				j++;
			}
			OUTPUTNum++;
			i = 0;
			continue;
		}
		/* We have some next Siemens sms from sequence */
		if (GSM_DecodeSiemensOTASMS(di, &SiemensOTA,&INPUT[i]->SMS[0]) &&
		    SiemensOTA.PacketNum > 1) {
			j = 0;
			while (INPUT[j]!=NULL) {
				if (INPUTSorted[j]) {
					j++;
					continue;
				}
				/* We have some not unassigned first sms from sequence.
				 * We can't touch other sms from sequences
				 */
				if (GSM_DecodeSiemensOTASMS(di, &SiemensOTA,&INPUT[j]->SMS[0]) &&
				    SiemensOTA.PacketNum == 1) {
					break;
				}
				j++;
			}
			if (INPUT[j]==NULL) {
				OUTPUT[OUTPUTNum] = malloc(sizeof(GSM_MultiSMSMessage));
				if (OUTPUT[OUTPUTNum] == NULL) {
					free(INPUTSorted);
					return ERR_MOREMEMORY;
				}
				OUTPUT[OUTPUTNum+1] = NULL;

				memcpy(OUTPUT[OUTPUTNum],INPUT[i],sizeof(GSM_MultiSMSMessage));
				INPUTSorted[i]=true;
				OUTPUTNum++;
				i = 0;
				continue;
			} else i++;
		}
		copyit = false;
		/* If we have:
		 * - linked sms returned by phone driver
		 * - sms without linking
		 * we copy it to OUTPUT
		 */
		if (INPUT[i]->Number 			!= 1 	       	||
		    INPUT[i]->SMS[0].UDH.Type 		== UDH_NoUDH   	||
                    INPUT[i]->SMS[0].UDH.PartNumber 	== -1) {
			copyit = true;
		}
		/* If we have unknown UDH, we copy it to OUTPUT */
		if (INPUT[i]->SMS[0].UDH.Type == UDH_UserUDH) {
			if (!ems) copyit = true;
			if (ems && INPUT[i]->SMS[0].UDH.PartNumber == -1) copyit = true;
		}
		if (copyit) {
			OUTPUT[OUTPUTNum] = malloc(sizeof(GSM_MultiSMSMessage));
			if (OUTPUT[OUTPUTNum] == NULL) {
				free(INPUTSorted);
				return ERR_MOREMEMORY;
			}
			OUTPUT[OUTPUTNum+1] = NULL;

			memcpy(OUTPUT[OUTPUTNum],INPUT[i],sizeof(GSM_MultiSMSMessage));
			INPUTSorted[i]=true;
			OUTPUTNum++;
			i = 0;
			continue;
		}
		/* We have 1'st part of linked sms. It's single.
		 * We will try to find other parts
		 */
		if (INPUT[i]->SMS[0].UDH.PartNumber == 1) {
			OUTPUT[OUTPUTNum] = malloc(sizeof(GSM_MultiSMSMessage));
			if (OUTPUT[OUTPUTNum] == NULL) {
				free(INPUTSorted);
				return ERR_MOREMEMORY;
			}
			OUTPUT[OUTPUTNum+1] = NULL;

			memcpy(&OUTPUT[OUTPUTNum]->SMS[0],&INPUT[i]->SMS[0],sizeof(GSM_SMSMessage));
			OUTPUT[OUTPUTNum]->Number = 1;
			INPUTSorted[i]	= true;
			j		= 1;
			/* We're searching for other parts in sequence */
			while (j != INPUT[i]->SMS[0].UDH.AllParts) {
				z=0;
				while(INPUT[z]!=NULL) {
					/* This was sorted earlier or is not single */
					if (INPUTSorted[z] || INPUT[z]->Number != 1) {
						z++;
						continue;
					}
					if (ems && INPUT[i]->SMS[0].UDH.Type != UDH_ConcatenatedMessages &&
					    INPUT[i]->SMS[0].UDH.Type != UDH_ConcatenatedMessages16bit   &&
					    INPUT[i]->SMS[0].UDH.Type != UDH_UserUDH 			 &&
					    INPUT[z]->SMS[0].UDH.Type != UDH_ConcatenatedMessages 	 &&
					    INPUT[z]->SMS[0].UDH.Type != UDH_ConcatenatedMessages16bit   &&
					    INPUT[z]->SMS[0].UDH.Type != UDH_UserUDH) {
						if (INPUT[z]->SMS[0].UDH.Type != INPUT[i]->SMS[0].UDH.Type) {
							z++;
							continue;
						}
					}
					if (!ems && INPUT[z]->SMS[0].UDH.Type != INPUT[i]->SMS[0].UDH.Type) {
						z++;
						continue;
					}
					smfprintf(di, "compare %i         %i %i %i %i",
						j+1,
						INPUT[i]->SMS[0].UDH.ID8bit,
						INPUT[i]->SMS[0].UDH.ID16bit,
						INPUT[i]->SMS[0].UDH.PartNumber,
						INPUT[i]->SMS[0].UDH.AllParts);
					smfprintf(di, "         %i %i %i %i\n",
						INPUT[z]->SMS[0].UDH.ID8bit,
						INPUT[z]->SMS[0].UDH.ID16bit,
						INPUT[z]->SMS[0].UDH.PartNumber,
						INPUT[z]->SMS[0].UDH.AllParts);
					if (INPUT[z]->SMS[0].UDH.ID8bit      != INPUT[i]->SMS[0].UDH.ID8bit	||
							INPUT[z]->SMS[0].UDH.ID16bit     != INPUT[i]->SMS[0].UDH.ID16bit	||
							INPUT[z]->SMS[0].UDH.AllParts    != INPUT[i]->SMS[0].UDH.AllParts 	||
							(INPUT[z]->SMS[0].UDH.PartNumber) != j + 1) {
						z++;
						continue;
					}
					/* For SMS_Deliver compare also SMSC and Sender numbers */
					if (INPUT[z]->SMS[0].PDU == SMS_Deliver &&
					    strcmp(DecodeUnicodeString(INPUT[z]->SMS[0].SMSC.Number),DecodeUnicodeString(INPUT[i]->SMS[0].SMSC.Number))) {
						z++;
						continue;
					}
					if (INPUT[z]->SMS[0].PDU == SMS_Deliver &&
					    INPUT[z]->SMS[0].OtherNumbersNum!=INPUT[i]->SMS[0].OtherNumbersNum) {
						z++;
						continue;
					}
					if (INPUT[z]->SMS[0].PDU == SMS_Deliver) {
						for (m=0;m<GSM_SMS_OTHER_NUMBERS+1;m++) {
							OtherNumbers[m]=false;
						}
						for (m=0;m<INPUT[z]->SMS[0].OtherNumbersNum+1;m++) {
							wrong=true;
							for (p=0;p<INPUT[i]->SMS[0].OtherNumbersNum+1;p++) {
								if (OtherNumbers[p]) continue;
								if (m==0 && p==0 && !strcmp(DecodeUnicodeString(INPUT[z]->SMS[0].Number),DecodeUnicodeString(INPUT[i]->SMS[0].Number))) {
									OtherNumbers[0]=true;
									wrong=false;
									break;
								}
								if (m==0 && p!=0 && !strcmp(DecodeUnicodeString(INPUT[z]->SMS[0].Number),DecodeUnicodeString(INPUT[i]->SMS[0].OtherNumbers[p-1]))) {
									OtherNumbers[p]=true;
									wrong=false;
									break;
								}
								if (m!=0 && p==0 && !strcmp(DecodeUnicodeString(INPUT[z]->SMS[0].OtherNumbers[m-1]),DecodeUnicodeString(INPUT[i]->SMS[0].Number))) {
									OtherNumbers[0]=true;
									wrong=false;
									break;
								}
								if (m!=0 && p!=0 && !strcmp(DecodeUnicodeString(INPUT[z]->SMS[0].OtherNumbers[m-1]),DecodeUnicodeString(INPUT[i]->SMS[0].OtherNumbers[p-1]))) {
									OtherNumbers[p]=true;
									wrong=false;
									break;
								}
							}
							if (wrong) break;
						}
						if (wrong) {
							z++;
							continue;
						}
					}
					/* DCT4 Outbox: SMS Deliver. Empty number and SMSC. We compare dates */
					if (INPUT[z]->SMS[0].PDU == SMS_Deliver 		&&
					    UnicodeLength(INPUT[z]->SMS[0].SMSC.Number)==0 	&&
					    UnicodeLength(INPUT[z]->SMS[0].Number)==0 		&&
					    (INPUT[z]->SMS[0].DateTime.Day    != INPUT[i]->SMS[0].DateTime.Day 	  ||
	   				     INPUT[z]->SMS[0].DateTime.Month  != INPUT[i]->SMS[0].DateTime.Month  ||
					     INPUT[z]->SMS[0].DateTime.Year   != INPUT[i]->SMS[0].DateTime.Year   ||
					     INPUT[z]->SMS[0].DateTime.Hour   != INPUT[i]->SMS[0].DateTime.Hour   ||
					     INPUT[z]->SMS[0].DateTime.Minute != INPUT[i]->SMS[0].DateTime.Minute ||
					     INPUT[z]->SMS[0].DateTime.Second != INPUT[i]->SMS[0].DateTime.Second)) {
						z++;
						continue;
					}
					/* We found correct sms. Copy it */
					memcpy(&OUTPUT[OUTPUTNum]->SMS[j],&INPUT[z]->SMS[0],sizeof(GSM_SMSMessage));
					OUTPUT[OUTPUTNum]->Number++;
					INPUTSorted[z]=true;
					break;
				}
				/* Incomplete sequence */
				if (OUTPUT[OUTPUTNum]->Number==j) {
					smfprintf(di, "Incomplete sequence\n");
					break;
				}
				j++;
			}
			OUTPUTNum++;
			i = 0;
			continue;
		}
		/* We have some next linked sms from sequence */
		if (INPUT[i]->SMS[0].UDH.PartNumber > 1) {
			j = 0;
			while (INPUT[j]!=NULL) {
				if (INPUTSorted[j]) {
					j++;
					continue;
				}
				/* We have some not unassigned first sms from sequence.
				 * We can't touch other sms from sequences
				 */
				if (INPUT[j]->SMS[0].UDH.PartNumber == 1) break;
				j++;
			}
			if (INPUT[j]==NULL) {
				OUTPUT[OUTPUTNum] = malloc(sizeof(GSM_MultiSMSMessage));
				if (OUTPUT[OUTPUTNum] == NULL) {
					free(INPUTSorted);
					return ERR_MOREMEMORY;
				}
				OUTPUT[OUTPUTNum+1] = NULL;

				memcpy(OUTPUT[OUTPUTNum],INPUT[i],sizeof(GSM_MultiSMSMessage));
				INPUTSorted[i]=true;
				OUTPUTNum++;
				i = 0;
				continue;
			} else i++;
		}
	}
	free(INPUTSorted);
	return ERR_NONE;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
