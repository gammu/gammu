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

void GSM_Find_Free_Used_SMS2(GSM_Debug_Info *di, GSM_Coding_Type Coding,GSM_SMSMessage *SMS, size_t *UsedText, size_t *FreeText, size_t *FreeBytes)
{
	size_t UsedBytes = 0;

	switch (Coding) {
	case SMS_Coding_Default_No_Compression:
		FindDefaultAlphabetLen(SMS->Text,&UsedBytes,UsedText,500);
		UsedBytes = *UsedText * 7 / 8;
		if (UsedBytes * 8 / 7 != *UsedText) UsedBytes++;
		*FreeBytes = GSM_MAX_8BIT_SMS_LENGTH - SMS->UDH.Length - UsedBytes;
		*FreeText = (GSM_MAX_8BIT_SMS_LENGTH - SMS->UDH.Length) * 8 / 7 - *UsedText;
		break;
	case SMS_Coding_Unicode_No_Compression:
		*UsedText = UnicodeLength(SMS->Text);
		UsedBytes = *UsedText * 2;
		*FreeBytes = GSM_MAX_8BIT_SMS_LENGTH - SMS->UDH.Length - UsedBytes;
		*FreeText = *FreeBytes / 2;
		break;
	case SMS_Coding_8bit:
		*UsedText = UsedBytes = SMS->Length;
		*FreeBytes = GSM_MAX_8BIT_SMS_LENGTH - SMS->UDH.Length - UsedBytes;
		*FreeText = *FreeBytes;
		break;
	default:
		break;
	}
	smfprintf(di, "UDH len %i, UsedBytes %ld, FreeText %ld, UsedText %ld, FreeBytes %ld\n",
		SMS->UDH.Length,
		(long)UsedBytes,
		(long)*FreeText,
		(long)*UsedText,
		(long)*FreeBytes);
}

unsigned int ReassembleCharacter(char *Buffer, size_t character_index)
{
	size_t offset = character_index * 2;

	return (
	  (unsigned int) ((unsigned char) Buffer[offset] << 8)
	    + (unsigned char) Buffer[offset + 1]
	);
}

int AlignIfSurrogatePair(GSM_Debug_Info	*di,
			 size_t		*Copy,
			 char		*Buffer,
			 size_t		BufferLen)
{
	int rv = 0;
	unsigned int n;

	/* Precondition:
	 *   Resulting copy must always be non-zero. */

	if (*Copy <= 1) {
		return rv;
	}

	/* Don't split a UTF-16 surrogate pair:
	 *   If the final code unit to be copied is a lead surrogate, save
	 *   it for the next message segment. This allows recipients to view
	 *   the proper four-byte UTF-16 character even if they're unable to
	 *   reassemble the message (e.g. if a telecom strips off the UDH). */

	n = ReassembleCharacter(Buffer, *Copy - 1);

	/* UTF-16 leading surrogate:
	 *   First character in pair is always between U+D800 and U+DBFF */

	if (n >= 0xd800 && n <= 0xdbff) {
		*Copy -= 1;
		++rv;
	}

	return rv;
}

int AlignIfCombinedCharacter(GSM_Debug_Info	*di,
			      size_t		*Copy,
			      char		*Buffer,
			      size_t		BufferLen)
{
	int rv = 0;
	unsigned int n;

	/* Precondition:
	 *   If we only have one character to copy, or if there isn't any
	 *   code unit following our copy window, don't change anything. */

	if (*Copy <= 1 || *Copy >= BufferLen) {
		return rv;
	}

	/* Don't split up a combining sequence:
	 *   Peek at the next message segment to see if it begins with
	 *   a combining character (e.g. a discritical mark). If it does,
	 *   push the final character of this message segment in to the
	 *   next message segment. This ensures that the recipient can
	 *   visually combine the sequence, even if reassembly fails. */

	n = ReassembleCharacter(Buffer, *Copy);

	/* Unicode combining characters:
	 *   Combining Half Marks (U+FE20 - U+FE2F)
	 *   Combining Diacritical Marks (U+300 - U+36F)
	 *   Combining Diacritical Marks Extended (U+1AB0 - U+1AFF)
	 *   Combining Diacritical Marks Supplement (U+1DC0 - U+1DFF)
	 *   Combining Diacritical Marks for Symbols (U+20D0 - U+20FF) */

	if ((n >= 0xfe20 && n <= 0xfe2f) ||
	    (n >= 0x300 && n <= 0x36f) || (n >= 0x1ab0 && n <= 0x1aff) ||
	    (n >= 0x1dc0 && n <= 0x1dff) || (n >= 0x20d0 && n <= 0x20ff)) {
		*Copy -= 1;
		++rv;
	}

	return rv;
}

int AlignIfCombinedSurrogate(GSM_Debug_Info	*di,
			     size_t		*Copy,
			     char		*Buffer,
			     size_t		BufferLen)
{
	int rv = 0;
	unsigned int l1, l2, r1, r2;

	/* Precondition:
	 *   If we have two or fewer characters to copy, omitting two
	 *   of them would cause us to send empty message segments. If
	 *   there aren't at least two characters remaining *after* the
	 *   copy boundary, then there can't possibly be space for a
	 *   second surrogate pair there. In either case, send as-is. */

	if (*Copy <= 2 || (*Copy + 2) >= BufferLen) {
		return rv;
	}

	/* Fetch characters:
	 *   We retrieve two UTF-16 characters directly preceeding the
	 *   copy boundary, and two directly following the copy boundary. */

	l1 = ReassembleCharacter(Buffer, *Copy - 2);
	l2 = ReassembleCharacter(Buffer, *Copy - 1);
	r1 = ReassembleCharacter(Buffer, *Copy);
	r2 = ReassembleCharacter(Buffer, *Copy + 1);

	/* Regional Indicator Symbol (U+1F1E6 - U+1F1FF)
	 *   UTF-16 surrogate pairs: 0xd83c 0xdde6 - 0xd83c 0xddff */

	if (l1 == 0xd83c && r1 == 0xd83c &&
	    l2 >= 0xdde6 && l2 <= 0xddff && r2 >= 0xdde6 && r2 <= 0xddff) {
		*Copy -= 2;
		rv += 2;
	}

	return rv;
}

int AlignSegmentForContent(GSM_Debug_Info	*di,
			   size_t		*Copy,
			   char			*Buffer,
			   size_t		BufferLen)
{
	int rv = 0;

	if (!(rv += AlignIfSurrogatePair(di, Copy, Buffer, BufferLen))) {
		rv += AlignIfCombinedCharacter(di, Copy, Buffer, BufferLen);
	}

	rv += AlignIfCombinedSurrogate(di, Copy, Buffer, BufferLen);
	return rv;
}

GSM_Error GSM_AddSMS_Text_UDH(GSM_Debug_Info *di,
				GSM_MultiSMSMessage 	*SMS,
		      		GSM_Coding_Type		Coding,
		      		char 			*Buffer,
		      		size_t			BufferLen,
		      		gboolean 			UDH,
		      		size_t 			*UsedText,
		      		size_t			*CopiedText,
		      		size_t			*CopiedSMSText)
{
	size_t FreeText=0,FreeBytes=0,Copy,i,j;

	smfprintf(di, "Checking used: ");
	GSM_Find_Free_Used_SMS2(
		di, Coding, &(SMS->SMS[SMS->Number]),
		UsedText, &FreeText, &FreeBytes
	);

	if (UDH) {
		smfprintf(di, "Adding UDH\n");
		if (FreeBytes - BufferLen <= 0) {
			smfprintf(di, "Going to the new SMS\n");
			SMS->Number++;
			GSM_Find_Free_Used_SMS2(
				di, Coding, &(SMS->SMS[SMS->Number]),
				UsedText, &FreeText, &FreeBytes
			);
		}
		if (SMS->SMS[SMS->Number].UDH.Length == 0) {
			SMS->SMS[SMS->Number].UDH.Length  = 1;
			SMS->SMS[SMS->Number].UDH.Text[0] = 0x00;
		}
		memcpy(SMS->SMS[SMS->Number].UDH.Text+SMS->SMS[SMS->Number].UDH.Length,Buffer,BufferLen);
		SMS->SMS[SMS->Number].UDH.Length  	+= BufferLen;
		SMS->SMS[SMS->Number].UDH.Text[0] 	+= BufferLen;
		SMS->SMS[SMS->Number].UDH.Type 		=  UDH_UserUDH;
		smfprintf(di, "UDH added %ld\n", (long)BufferLen);
	} else {
		smfprintf(di, "Adding text\n");
		if (FreeText == 0) {
			smfprintf(di, "Going to the new SMS\n");
			SMS->Number++;
			GSM_Find_Free_Used_SMS2(
				di, Coding, &(SMS->SMS[SMS->Number]),
				UsedText, &FreeText, &FreeBytes
			);
		}

		Copy = FreeText;
		smfprintf(di, "Copy %ld (max %ld)\n", (long)Copy, (long)BufferLen);
		if (BufferLen < Copy) {
			Copy = BufferLen;
		}

		switch (Coding) {
		case SMS_Coding_Default_No_Compression:
			FindDefaultAlphabetLen(Buffer,&i,&j,FreeText);
			smfprintf(di, "Default text, length %ld %ld\n", (long)i, (long)j);
			SMS->SMS[SMS->Number].Text[UnicodeLength(SMS->SMS[SMS->Number].Text)*2+i*2]   = 0;
			SMS->SMS[SMS->Number].Text[UnicodeLength(SMS->SMS[SMS->Number].Text)*2+i*2+1] = 0;
			memcpy(SMS->SMS[SMS->Number].Text+UnicodeLength(SMS->SMS[SMS->Number].Text)*2,Buffer,i*2);
			*CopiedText 	= i;
			*CopiedSMSText 	= j;
			SMS->SMS[SMS->Number].Length += i;
			break;
		case SMS_Coding_Unicode_No_Compression:
			AlignSegmentForContent(di, &Copy, Buffer, BufferLen);
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

	smfprintf(di, "Checking at the end: ");
	GSM_Find_Free_Used_SMS2(
		di, Coding, &(SMS->SMS[SMS->Number]),
		UsedText, &FreeText, &FreeBytes
	);

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
	size_t 		Len,UsedText = 0,CopiedText = 0,CopiedSMSText = 0;
	int		j;
	unsigned char 	UDHID;
	GSM_DateTime 	Date;

	Len = 0;
	while(1) {
		if (SMS->Number >= GSM_MAX_MULTI_SMS) {
			break;
		}
		GSM_SetDefaultSMSData(&SMS->SMS[SMS->Number]);
		SMS->SMS[SMS->Number].Class    = Class;
		SMS->SMS[SMS->Number].Coding   = Coding;

		SMS->SMS[SMS->Number].UDH.Type = UDHType;
		GSM_EncodeUDHHeader(di, &SMS->SMS[SMS->Number].UDH);

		if (Coding == SMS_Coding_8bit) {
			GSM_AddSMS_Text_UDH(di, SMS,Coding,MessageBuffer+Len,MessageLength - Len,FALSE,&UsedText,&CopiedText,&CopiedSMSText);
		} else {
			GSM_AddSMS_Text_UDH(di, SMS,Coding,MessageBuffer+Len*2,MessageLength - Len,FALSE,&UsedText,&CopiedText,&CopiedSMSText);
		}
		Len += CopiedText;
		smfprintf(di, "%ld %ld\n", (long)Len, (long)MessageLength);
		SMS->Number++;
		if (Len == MessageLength) break;
	}

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
void GSM_SMSCounter(GSM_Debug_Info *di,
		    unsigned char 	*MessageBuffer,
		    GSM_UDH	 	UDHType,
		    GSM_Coding_Type 	Coding,
		    int 		*SMSNum,
		    size_t 		*CharsLeft)
{
	size_t			UsedText=0,FreeBytes=0;
	GSM_MultiSMSMessage 	MultiSMS;

	MultiSMS.Number = 0;
	GSM_MakeMultiPartSMS(di, &MultiSMS,MessageBuffer,UnicodeLength(MessageBuffer),UDHType,Coding,-1,FALSE);
	GSM_Find_Free_Used_SMS2(
		di, Coding, &(MultiSMS.SMS[MultiSMS.Number-1]),
		&UsedText, CharsLeft, &FreeBytes
	);
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
			Info->Entries[0].RingtoneNotes=GSM_EncodeNokiaRTTLRingtone(Info->Entries[0].Ringtone,Buffer+(*Length),&len);
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
		EncodeDefault(buff, Name, &p, TRUE, NULL);
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
	unsigned char	*Buffer;
	unsigned char	*Buffer2;
	size_t		buffer_size = GSM_MAX_SMS_LENGTH * 2 * GSM_MAX_MULTI_SMS;
	int		i, Class = -1, j;
	size_t p;
	size_t Length = 0, smslen;
	GSM_Error	error;
	GSM_Coding_Type Coding 	= SMS_Coding_8bit;
	GSM_UDH		UDH	= UDH_NoUDH;
	GSM_UDHHeader 	UDHHeader;
	gboolean		EMS	= FALSE;
	int		textnum = 0;

	if (Info->EntriesNum == 0) return ERR_EMPTY;

	Buffer = malloc(buffer_size);
	if (Buffer == NULL) {
		return ERR_MOREMEMORY;
	}
	Buffer2 = malloc(buffer_size);
	if (Buffer2 == NULL) {
		free(Buffer);
		return ERR_MOREMEMORY;
	}

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
			EncodeDefault(Buffer2, Info->Entries[i].Buffer, &p, TRUE, NULL);
			Buffer[Length++]   	= 0x00; 	/* type of data */
			Length 			= Length + 2;
			smslen			= GSM_PackSevenBitsToEight(0, Buffer2, Buffer+Length, p);
			Buffer[Length-2] 	= smslen % 256; /* len */
			Buffer[Length-1] 	= smslen / 256; /* len */
			Length 			= Length + smslen;
			break;
		default:
			error = ERR_UNKNOWN;
			goto out;
		}
		}
		Buffer[0] = Buffer[0] * 2;
		error = GSM_EncodeAlcatelMultiPartSMS(di, SMS,Buffer,Length,Info->Entries[0].Buffer,ALCATELTDD_SMSTEMPLATE);
		goto out;
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
			EMS = TRUE;
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
			if (textnum > 1) EMS = TRUE;

			if (Info->Entries[i].Left   || Info->Entries[i].Right      ||
			    Info->Entries[i].Center || Info->Entries[i].Large      ||
			    Info->Entries[i].Small  || Info->Entries[i].Bold       ||
			    Info->Entries[i].Italic || Info->Entries[i].Underlined ||
			    Info->Entries[i].Strikethrough) {
				EMS = TRUE;
			}
		default:
			break;
		}
		if (EMS) break;
	}
	if (EMS) {
		error=GSM_EncodeEMSMultiPartSMS(di, Info,SMS,UDH_NoUDH);
		if (error != ERR_NONE) {
			goto out;
		}
		if (SMS->Number != 1) {
			SMS->Number = 0;
			for (i=0;i<Info->EntriesNum;i++) {
				if (Info->Entries[i].ID == SMS_ConcatenatedTextLong16bit) {
					error = GSM_EncodeEMSMultiPartSMS(di, Info,SMS,UDH_ConcatenatedMessages);
					goto out;
				}
			}
			error = GSM_EncodeEMSMultiPartSMS(di, Info,SMS,UDH_ConcatenatedMessages16bit);
		}
		goto out;
	}

	if (Info->EntriesNum != 1) {
		error = ERR_UNKNOWN;
		goto out;
	}

	switch (Info->Entries[0].ID) {
	case SMS_AlcatelMonoBitmapLong:
		Buffer[0] = Info->Entries[0].Bitmap->Bitmap[0].BitmapWidth;
		Buffer[1] = Info->Entries[0].Bitmap->Bitmap[0].BitmapHeight;
		PHONE_EncodeBitmap(GSM_AlcatelBMMIPicture, Buffer+2, &Info->Entries[0].Bitmap->Bitmap[0]);
		Length = PHONE_GetBitmapSize(GSM_AlcatelBMMIPicture,Info->Entries[0].Bitmap->Bitmap[0].BitmapWidth,Info->Entries[0].Bitmap->Bitmap[0].BitmapHeight)+2;
		error = GSM_EncodeAlcatelMultiPartSMS(di, SMS,Buffer,Length,Info->Entries[0].Bitmap->Bitmap[0].Text,ALCATELTDD_PICTURE);
		goto out;
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
		error = GSM_EncodeAlcatelMultiPartSMS(di, SMS,Buffer,Length,Info->Entries[0].Bitmap->Bitmap[0].Text,ALCATELTDD_ANIMATION);
		goto out;
	case SMS_MMSIndicatorLong:
		Class	= 1;
		UDH	= UDH_MMSIndicatorLong;
		GSM_EncodeMMSIndicatorSMSText(Buffer,&Length,Info->Entries[0].MMSIndicator);
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
		Info->Entries[0].RingtoneNotes = GSM_EncodeNokiaRTTLRingtone(Info->Entries[0].Ringtone,Buffer,&Length);
		if (Info->Entries[0].ID == SMS_NokiaRingtone) break;
		if (Info->Entries[0].RingtoneNotes != Info->Entries[0].Ringtone->NoteTone.NrCommands) {
			UDH    = UDH_NokiaRingtoneLong;
			Length = (GSM_MAX_8BIT_SMS_LENGTH-12)*3;
			Info->Entries[0].RingtoneNotes = GSM_EncodeNokiaRTTLRingtone(Info->Entries[0].Ringtone,Buffer,&Length);
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
		FALLTHROUGH
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
		NOKIA_EncodeWAPMMSSettingsSMSText(Buffer,&Length,Info->Entries[0].Settings,FALSE);
		break;
	case SMS_NokiaMMSSettingsLong:
		Class	= 1;
		UDH	= UDH_NokiaWAPLong;
		NOKIA_EncodeWAPMMSSettingsSMSText(Buffer,&Length,Info->Entries[0].Settings,TRUE);
		break;
	case SMS_NokiaVCARD10Long:
		Coding = SMS_Coding_Default_No_Compression;
		UDH = UDH_NokiaPhonebookLong;
		error = GSM_EncodeVCARD(di, Buffer, buffer_size, &Length, Info->Entries[0].Phonebook, TRUE, Nokia_VCard10);
		if (error != ERR_NONE) {
			goto out;
		}
		memcpy(Buffer2,Buffer,Length);
		EncodeUnicode(Buffer,Buffer2,Length);
		break;
	case SMS_NokiaVCARD21Long:
		error = GSM_EncodeVCARD(di, Buffer, buffer_size, &Length, Info->Entries[0].Phonebook, TRUE, Nokia_VCard21);
		if (error != ERR_NONE) {
			goto out;
		}
		Coding = SMS_Coding_Default_No_Compression;
		UDH = UDH_NokiaPhonebookLong;
		memcpy(Buffer2,Buffer,Length);
		EncodeUnicode(Buffer,Buffer2,Length);
		break;
	case SMS_VCARD10Long:
		error = GSM_EncodeVCARD(di, Buffer, buffer_size, &Length,Info->Entries[0].Phonebook,TRUE,Nokia_VCard10);
		if (error != ERR_NONE) {
			goto out;
		}
		if (Length>GSM_MAX_SMS_CHARS_LENGTH) UDH = UDH_ConcatenatedMessages;
		Coding = SMS_Coding_Default_No_Compression;
		memcpy(Buffer2,Buffer,Length);
		EncodeUnicode(Buffer,Buffer2,Length);
		break;
	case SMS_VCARD21Long:
		error = GSM_EncodeVCARD(di, Buffer, buffer_size, &Length,Info->Entries[0].Phonebook,TRUE,Nokia_VCard21);
		if (error != ERR_NONE) {
			goto out;
		}
		if (Length>GSM_MAX_SMS_CHARS_LENGTH) UDH = UDH_ConcatenatedMessages;
		Coding = SMS_Coding_Default_No_Compression;
		memcpy(Buffer2,Buffer,Length);
		EncodeUnicode(Buffer,Buffer2,Length);
		break;
	case SMS_NokiaVCALENDAR10Long:
		error=GSM_EncodeVCALENDAR(Buffer, buffer_size,&Length,Info->Entries[0].Calendar,TRUE,Nokia_VCalendar);
		if (error != ERR_NONE) {
			goto out;
		}
		Coding = SMS_Coding_Default_No_Compression;
		/* Is 1 SMS ? 8 = length of ..SCKE4 */
		if (Length <= GSM_MAX_SMS_CHARS_LENGTH - 8) {
			sprintf(Buffer,"//SCKE4 ");
			Length = 8;
		} else {
			UDH = UDH_NokiaCalendarLong;
		}
		memcpy(Buffer2,Buffer,Length);
		EncodeUnicode(Buffer,Buffer2,Length);
		break;
	case SMS_NokiaVTODOLong:
		error=GSM_EncodeVTODO(Buffer, buffer_size,&Length,Info->Entries[0].ToDo,TRUE,Nokia_VToDo);
		if (error != ERR_NONE) {
			goto out;
		}
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
	case SMS_USSD:
		Class = Info->Class;
		break;
	case SMS_ConcatenatedAutoTextLong:
	case SMS_ConcatenatedAutoTextLong16bit:
		smslen = UnicodeLength(Info->Entries[0].Buffer);
		memcpy(Buffer,Info->Entries[0].Buffer,smslen*2);
		EncodeDefault(Buffer2, Buffer, &smslen, TRUE, NULL);
		DecodeDefault(Buffer,  Buffer2, smslen, TRUE, NULL);
#ifdef DEBUG
		if (di->dl == DL_TEXTALL || di->dl == DL_TEXTALLDATE) {
			smfprintf(di, "Info->Entries[0].Buffer:\n");
			DumpMessage(di, Info->Entries[0].Buffer, UnicodeLength(Info->Entries[0].Buffer)*2);
			smfprintf(di, "Buffer:\n");
			DumpMessage(di, Buffer, UnicodeLength(Buffer)*2);
		}
#endif
		Info->UnicodeCoding = FALSE;
		for (smslen = 0; smslen < UnicodeLength(Info->Entries[0].Buffer) * 2; smslen++) {
			if (Info->Entries[0].Buffer[smslen] != Buffer[smslen]) {
				Info->UnicodeCoding = TRUE;
				smfprintf(di, "Setting to Unicode %ld\n", (long)smslen);
				break;
			}
		}
		/* No break here - we go to the SMS_ConcatenatedTextLong */
		FALLTHROUGH
	case SMS_ConcatenatedTextLong:
	case SMS_ConcatenatedTextLong16bit:
		Class = Info->Class;
		if (Info->Entries[0].Buffer == NULL) {
			Buffer[0] = 0;
			Buffer[1] = 0;
		} else {
			memcpy(Buffer,Info->Entries[0].Buffer,UnicodeLength(Info->Entries[0].Buffer)*2+2);
		}
		UDH = UDH_NoUDH;
		if (Info->UnicodeCoding) {
			Coding = SMS_Coding_Unicode_No_Compression;
			Length = UnicodeLength(Buffer);
			if (Info->Entries[0].ID == SMS_ConcatenatedTextLong16bit ||
			    Info->Entries[0].ID == SMS_ConcatenatedAutoTextLong16bit) {
				if (Length>70) UDH=UDH_ConcatenatedMessages16bit;
			} else {
				if (Length>70) UDH=UDH_ConcatenatedMessages;
			}
		} else {
			Coding = SMS_Coding_Default_No_Compression;
			FindDefaultAlphabetLen(Buffer,&Length,&smslen,5000);
			if (Info->Entries[0].ID == SMS_ConcatenatedTextLong16bit ||
			    Info->Entries[0].ID == SMS_ConcatenatedAutoTextLong16bit) {
				if (smslen>GSM_MAX_SMS_CHARS_LENGTH) UDH=UDH_ConcatenatedMessages16bit;
			} else {
				if (smslen>GSM_MAX_SMS_CHARS_LENGTH) UDH=UDH_ConcatenatedMessages;
			}
		}
	default:
		break;
	}
	GSM_MakeMultiPartSMS(di, SMS,Buffer,Length,UDH,Coding,Class,Info->ReplaceMessage);
	error = ERR_NONE;
out:
	free(Buffer);
	free(Buffer2);
	return error;
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
		Info->Entries[i].Protected	= FALSE;

		Info->Entries[i].Buffer		= NULL;
		Info->Entries[i].Left		= FALSE;
		Info->Entries[i].Right		= FALSE;
		Info->Entries[i].Center		= FALSE;
		Info->Entries[i].Large		= FALSE;
		Info->Entries[i].Small		= FALSE;
		Info->Entries[i].Bold		= FALSE;
		Info->Entries[i].Italic		= FALSE;
		Info->Entries[i].Underlined	= FALSE;
		Info->Entries[i].Strikethrough	= FALSE;

		Info->Entries[i].RingtoneNotes	= 0;
	}
	Info->Unknown		= FALSE;
	Info->EntriesNum	= 0;
	Info->Class		= -1;
	Info->ReplaceMessage	= 0;
	Info->UnicodeCoding	= FALSE;
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

/**
 * Decodes long MMS notification SMS.
 */
gboolean GSM_DecodeMMSIndication(GSM_Debug_Info *di,
			    GSM_MultiPartSMSInfo	*Info,
			    GSM_MultiSMSMessage		*SMS)
{
	int i, Length = 0, j;
	unsigned char Buffer[GSM_MAX_SMS_LENGTH*2*GSM_MAX_MULTI_SMS];

	/* Concatenate data */
	for (i = 0; i < SMS->Number; i++) {
		if (SMS->SMS[i].UDH.Type == UDH_MMSIndicatorLong) {
			if (SMS->SMS[i].UDH.Text[11] != i+1		 ||
			    SMS->SMS[i].UDH.Text[10] != SMS->Number) {
				return FALSE;
			}
		} else if (SMS->SMS[i].UDH.Type != UDH_UserUDH) {
			return FALSE;
		}
		memcpy(Buffer + Length, SMS->SMS[i].Text, SMS->SMS[i].Length);
		Length = Length + SMS->SMS[i].Length;
	}

	dbgprintf(di, "MMS data of length %d:\n", Length);
	DumpMessage(di, Buffer, Length);
	Info->Entries[0].MMSIndicator = (GSM_MMSIndicator *)malloc(sizeof(GSM_MMSIndicator));
	if (Info->Entries[0].MMSIndicator == NULL) {
		return FALSE;
	}
	Info->EntriesNum    = 1;
	Info->Entries[0].ID = SMS_MMSIndicatorLong;
	Info->Entries[0].MMSIndicator->Class = GSM_MMS_None;
	Info->Entries[0].MMSIndicator->MessageSize = 0;
	Info->Entries[0].MMSIndicator->Title[0] = 0;
	Info->Entries[0].MMSIndicator->Sender[0] = 0;
	Info->Entries[0].MMSIndicator->Address[0] = 0;

	/* First byte is the WSP transaction ID */
	/* Second byte is PUSH */
	if (Buffer[1] != 0x06) {
		dbgprintf(di, "Unsupported WSP PDU type: 0x%02x\n", Buffer[1]);
		return FALSE;
	}
	/*
	 * WSP Push PDU follows:
	 *
	 * Buffer[2] is length of content type and headers
	 * Buffer[3] is start of content type
	 *
	 * Process payload after headers per
	 * Multimedia Messaging Service Encapsulation Protocol
	 */
	for (i = 3 + Buffer[2]; i < Length; i++) {
		switch(Buffer[i]) {
			case 0x8c:
				/* X-Mms-Message-Type (Transaction type) */
				i++;
				/* We support only m-notification-ind (130) */
				if (Buffer[i] != 0x82) {
					dbgprintf(di, "Unsupported transaction type: 0x%02x\n", Buffer[i]);
					return FALSE;
				}
				break;
			case 0x98:
				/* X-Mms-Transaction-Id (Message ID) */
				dbgprintf(di, "Transaction ID: %s\n", Buffer + i + 1);
				while (Buffer[i] != 0 && i < Length) i++;
				break;
			case 0x8d:
				/*  X-Mms-MMS-Version (MMS version) */
				i++;
				if (Buffer[i] < 0x90 || Buffer[i] > 0x92) {
					dbgprintf(di, "Unsupported MMS version: 0x%02x\n", Buffer[i]);
					return FALSE;
				}
				break;
			case 0x89:
				/* From (Sender) */
				i++;
				/* Length */
				if (Buffer[i] == 0) continue;
				/* Address-present-token */
				if (Buffer[i + 1] == 0x80) {
					if (Buffer[i + 2] < 32) {
						/* String with length + encoding, we just ignore it for now */
						strcpy(Info->Entries[0].MMSIndicator->Sender, Buffer + i + 4);
					} else {
						strcpy(Info->Entries[0].MMSIndicator->Sender, Buffer + i + 2);
					}
				}
				i += Buffer[i];
				break;
			case 0x96:
				/* Subject (Title) */
				if (Buffer[i + 1] == 0x0a && Buffer[i + 2] == 0xea) {
					/* UTF-8 */
					strcpy(Info->Entries[0].MMSIndicator->Title, Buffer + i + 3);
					i += strlen(Info->Entries[0].MMSIndicator->Title) + 3;
				} else {
					strcpy(Info->Entries[0].MMSIndicator->Title, Buffer + i + 1);
					i += strlen(Info->Entries[0].MMSIndicator->Title) + 1;
				}
				break;
			case 0x8a:
				/* X-Mms-Message-Class (Class) */
				i++;
				switch (Buffer[i]) {
					case 0x80:
						Info->Entries[0].MMSIndicator->Class = GSM_MMS_Personal;
						break;
					case 0x81:
						Info->Entries[0].MMSIndicator->Class = GSM_MMS_Advertisement;
						break;
					case 0x82:
						Info->Entries[0].MMSIndicator->Class = GSM_MMS_Info;
						break;
					case 0x83:
						Info->Entries[0].MMSIndicator->Class = GSM_MMS_Auto;
						break;
					default:
						dbgprintf(di, "Unsupported MMS class: 0x%02x\n", Buffer[i]);
						break;
				}
				break;
			case 0x8e:
				/* X-Mms-Message-Size (Message size) */
				i++;
				for (j = i + 1; j < i + 1 + Buffer[i]; j++) {
					Info->Entries[0].MMSIndicator->MessageSize = (Info->Entries[0].MMSIndicator->MessageSize << 8) + Buffer[j];
				}
				i += Buffer[i];
				break;
			case 0x83:
				/* X-Mms-Content-Location (URL) */
				strcpy(Info->Entries[0].MMSIndicator->Address, Buffer + i + 1);
				i += strlen(Info->Entries[0].MMSIndicator->Address) + 1;
				break;

			/* Ignored variable length fields */
			case 0x87: /* X-Mms-Delivery-Time */
			case 0x88: /* X-Mms-Expiry */
			case 0x9d: /* X-Mms-Reply-Charging-Deadline */
			case 0xa0: /* X-Mms-Previously-Sent-By */
			case 0xa1: /* X-Mms-Previously-Sent-Date */
			case 0xa4: /* X-Mms-MM-Flags */
			case 0xaa: /* X-Mms-Mbox-Totals */
			case 0xac: /* X-Mms-Mbox-Quotas */
			case 0xb2: /* X-Mms-Element-Descriptor */
				i++;
				i += Buffer[i];
				break;

			/* Ignored long integer types */
			case 0x85: /* Date */
			case 0x9f: /* X-Mms-Reply-Charging-Size */
				i++;
				i += Buffer[i];
				break;

			/* Ignored integer types */
			case 0xad: /* X-Mms-Message-Count */
			case 0xaf: /* X-Mms-Start */
			case 0xb3: /* X-Mms-Limit */
				i++;
				break;

			/* Ignored octet types */
			case 0x86: /* X-Mms-Delivery-Report */
			case 0x8f: /* X-Mms-Priority */
			case 0x90: /* X-Mms-Read-Report */
			case 0x91: /* X-Mms-Report-Allowed */
			case 0x92: /* X-Mms-Response-Status */
			case 0x94: /* X-Mms-Sender-Visibility */
			case 0x95: /* X-Mms-Status */
			case 0x99: /* X-Mms-Retrieve-Status */
			case 0x9b: /* X-Mms-Read-Status */
			case 0x9c: /* X-Mms-Reply-Charging */
			case 0xa2: /* X-Mms-Store */
			case 0xa3: /* X-Mms-MM-State */
			case 0xa5: /* X-Mms-Store-Status */
			case 0xa7: /* X-Mms-Stored */
			case 0xa8: /* X-Mms-Attributes */
			case 0xa9: /* X-Mms-Totals */
			case 0xab: /* X-Mms-Quotas */
			case 0xb1: /* X-Mms-Distribution-Indicator */
			case 0xb4: /* X-Mms-Recommended-Retrieval-Mode */
			case 0xba: /* X-Mms-Content-Class */
			case 0xbb: /* X-Mms-DRM-Content */
			case 0xbc: /* X-Mms-Adaptation-Allowed */
			case 0xbf: /* X-Mms-Cancel-Status */
				i++;
				break;

			/* Ignored encoded string types */
			case 0x81: /* Bcc */
			case 0x82: /* Cc */
			case 0x84: /* Content-Type */
			case 0x97: /* To */
			case 0x93: /* X-Mms-Response-Text */
			case 0x9a: /* X-Mms-Retrieve-Text */
			case 0xa6: /* X-Mms-Store-Status-Text */
			case 0xb5: /* X-Mms-Recommended-Retrieval-Mode-Text */
			case 0xb6: /* X-Mms-Status-Text */
				while (Buffer[i] != 0 && i < Length) {
					i++;
				}
				break;

			/* Ignored string types */
			case 0x8b: /* Message-ID */
			case 0x9e: /* X-Mms-Reply-Charging-ID */
			case 0xb7: /* X-Mms-Applic-ID */
			case 0xb8: /* X-Mms-Reply-Applic-ID */
			case 0xb9: /* X-Mms-Aux-Applic-Info */
			case 0xbd: /* X-Mms-Replace-ID */
			case 0xbe: /* X-Mms-Cancel-ID */
				i++;
				i += Buffer[i];
				break;
			default:
				dbgprintf(di, "Unknown MMS tag: 0x%02x\n", Buffer[i]);
				break;
		}
	}

	return TRUE;
}

/**
 * Decodes long Nokia profile SMS.
 */
gboolean GSM_DecodeNokiaProfile(GSM_Debug_Info *di,
			    GSM_MultiPartSMSInfo	*Info,
			    GSM_MultiSMSMessage		*SMS)
{
	int i, Length = 0;
	char Buffer[GSM_MAX_SMS_LENGTH*2*GSM_MAX_MULTI_SMS];

	for (i=0;i<SMS->Number;i++) {
		if (SMS->SMS[i].UDH.Type != UDH_NokiaProfileLong ||
		    SMS->SMS[i].UDH.Text[11] != i+1		 ||
		    SMS->SMS[i].UDH.Text[10] != SMS->Number) {
			return FALSE;
		}
		memcpy(Buffer+Length,SMS->SMS[i].Text,SMS->SMS[i].Length);
		Length = Length + SMS->SMS[i].Length;
	}
	Info->EntriesNum    = 1;
	Info->Entries[0].ID = SMS_NokiaPictureImageLong;
	Info->Entries[0].Bitmap = (GSM_MultiBitmap *)malloc(sizeof(GSM_MultiBitmap));
	if (Info->Entries[0].Bitmap == NULL) return FALSE;
	Info->Entries[0].Bitmap->Number = 0;
	Info->Entries[0].Bitmap->Bitmap[0].Text[0] = 0;
	Info->Entries[0].Bitmap->Bitmap[0].Text[1] = 0;
	i=1;
	while (i < Length) {
		switch (Buffer[i]) {
		case SM30_ISOTEXT:
			smfprintf(di, "ISO 8859-2 text\n");
			break;
		case SM30_UNICODETEXT:
			smfprintf(di, "Unicode text\n");
			break;
		case SM30_OTA:
			smfprintf(di, "OTA bitmap as Picture Image\n");
			PHONE_DecodeBitmap(GSM_NokiaPictureImage, Buffer + i + 7, &Info->Entries[0].Bitmap->Bitmap[Info->Entries[0].Bitmap->Number]);
			Info->Entries[0].Bitmap->Number += 1;
#ifdef DEBUG
			if (di->dl == DL_TEXTALL || di->dl == DL_TEXTALLDATE) {
				GSM_PrintBitmap(di->df, &Info->Entries[0].Bitmap->Bitmap[0]);
			}
#endif
			break;
		case SM30_RINGTONE:
			smfprintf(di, "RTTL ringtone\n");
			Info->Unknown = TRUE;
			break;
		case SM30_PROFILENAME:
			smfprintf(di, "Profile Name\n");
			Info->Entries[0].ID = SMS_NokiaProfileLong;
			Info->Unknown = TRUE;
			break;
		case SM30_SCREENSAVER:
			smfprintf(di, "OTA bitmap as Screen Saver\n");
			PHONE_DecodeBitmap(GSM_NokiaPictureImage, Buffer + i + 7, &Info->Entries[0].Bitmap->Bitmap[Info->Entries[0].Bitmap->Number]);
			Info->Entries[0].Bitmap->Number += 1;
#ifdef DEBUG
			if (di->dl == DL_TEXTALL || di->dl == DL_TEXTALLDATE) {
				GSM_PrintBitmap(di->df, &Info->Entries[0].Bitmap->Bitmap[0]);
			}
#endif
			Info->Entries[0].ID = SMS_NokiaScreenSaverLong;
			break;
		}
		i = i + Buffer[i+1]*256 + Buffer[i+2] + 3;
		smfprintf(di, "Profile: pos=%i length=%i\n", i, Length);
	}
	i=1;
	while (i < Length) {
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
		smfprintf(di, "Profile: pos=%i length=%i\n", i, Length);
	}
	return TRUE;
}

/**
 * Decodes long linked text SMS.
 */
gboolean GSM_DecodeLinkedText(GSM_Debug_Info *di,
			    GSM_MultiPartSMSInfo	*Info,
			    GSM_MultiSMSMessage		*SMS)
{
	int i, Length = 0;

	Info->EntriesNum    = 1;
	Info->Entries[0].ID = SMS_ConcatenatedTextLong;
	if (SMS->SMS[0].UDH.Type == UDH_ConcatenatedMessages16bit) {
		Info->Entries[0].ID = SMS_ConcatenatedTextLong16bit;
	}

	for (i=0;i<SMS->Number;i++) {
		switch (SMS->SMS[i].Coding) {
		case SMS_Coding_8bit:
			Info->Entries[0].Buffer = (unsigned char *)realloc(Info->Entries[0].Buffer, Length + SMS->SMS[i].Length + 2);
			if (Info->Entries[0].Buffer == NULL) return FALSE;

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
			FALLTHROUGH
		case SMS_Coding_Default_No_Compression:
			Info->Entries[0].Buffer = (unsigned char *)realloc(Info->Entries[0].Buffer, Length + UnicodeLength(SMS->SMS[i].Text)*2 + 2);
			if (Info->Entries[0].Buffer == NULL) return FALSE;

			memcpy(Info->Entries[0].Buffer+Length,SMS->SMS[i].Text,UnicodeLength(SMS->SMS[i].Text)*2);
			Length=Length+UnicodeLength(SMS->SMS[i].Text)*2;
			break;
		default:
			break;
		}
	}
	Info->Entries[0].Buffer[Length]	  = 0;
	Info->Entries[0].Buffer[Length+1] = 0;
	return TRUE;
}

/* ----------------- Joining SMS from parts -------------------------------- */

gboolean GSM_DecodeMultiPartSMS(GSM_Debug_Info *di,
			    GSM_MultiPartSMSInfo	*Info,
			    GSM_MultiSMSMessage		*SMS,
			    gboolean			ems)
{
	int 			i;
	unsigned int		j;
	gboolean 			emsexist = FALSE, result;
	GSM_SiemensOTASMSInfo	SiemensInfo;

	GSM_ClearMultiPartSMSInfo(Info);
	if (ems) {
		emsexist = TRUE;
		for (i=0;i<SMS->Number;i++) {
			if (SMS->SMS[i].UDH.Type != UDH_ConcatenatedMessages 		&&
			    SMS->SMS[i].UDH.Type != UDH_ConcatenatedMessages16bit 	&&
			    SMS->SMS[i].UDH.Type != UDH_UserUDH) {
				emsexist = FALSE;
				break;
			}
		}
	}

	/* EMS decoding */
	if (emsexist) {
		return GSM_DecodeEMSMultiPartSMS(di, Info,SMS);
	}

	/* Siemens OTA */
	if (GSM_DecodeSiemensOTASMS(di, &SiemensInfo,&SMS->SMS[0])) {
		Info->Entries[0].File = (GSM_File *)malloc(sizeof(GSM_File));
		if (Info->Entries[0].File == NULL) return FALSE;
		Info->Entries[0].File->Buffer 	= NULL;
		Info->Entries[0].File->Used 	= 0;
		for (i=0;i<SMS->Number;i++) {
			if (GSM_DecodeSiemensOTASMS(di, &SiemensInfo,&SMS->SMS[i])) {
				j = SiemensInfo.AllDataLen - Info->Entries[0].File->Used;
				if (j>SiemensInfo.DataLen) j = SiemensInfo.DataLen;
				Info->Entries[0].File->Buffer = (unsigned char *)realloc(Info->Entries[0].File->Buffer,j+Info->Entries[0].File->Used);
				memcpy(Info->Entries[0].File->Buffer+Info->Entries[0].File->Used,SiemensInfo.Data,j);
				Info->Entries[0].File->Used += j;
			}
		}
		if (SiemensInfo.AllDataLen == Info->Entries[0].File->Used) {
			Info->Entries[0].ID 	= SMS_SiemensFile;
			Info->EntriesNum	= 1;
			EncodeUnicode(Info->Entries[0].File->Name,SiemensInfo.DataName,strlen(SiemensInfo.DataName));
			return TRUE;
		}
		free(Info->Entries[0].File->Buffer);
		Info->Entries[0].File->Buffer=NULL;
	}

	/* Smart Messaging decoding */
	if (SMS->SMS[0].UDH.Type == UDH_NokiaRingtone && SMS->Number == 1) {
		Info->Entries[0].Ringtone = (GSM_Ringtone *)malloc(sizeof(GSM_Ringtone));
		if (Info->Entries[0].Ringtone == NULL) return FALSE;
		if (GSM_DecodeNokiaRTTLRingtone(Info->Entries[0].Ringtone, SMS->SMS[0].Text, SMS->SMS[0].Length)==ERR_NONE) {
			Info->Entries[0].ID 	= SMS_NokiaRingtone;
			Info->EntriesNum	= 1;
			return TRUE;
		}
	}
	if (SMS->SMS[0].UDH.Type == UDH_NokiaCallerLogo && SMS->Number == 1) {
		Info->Entries[0].Bitmap = (GSM_MultiBitmap *)malloc(sizeof(GSM_MultiBitmap));
		if (Info->Entries[0].Bitmap == NULL) return FALSE;
		Info->Entries[0].Bitmap->Number = 1;
		PHONE_DecodeBitmap(GSM_NokiaCallerLogo, SMS->SMS[0].Text+4, &Info->Entries[0].Bitmap->Bitmap[0]);
#ifdef DEBUG
		if (di->dl == DL_TEXTALL || di->dl == DL_TEXTALLDATE)
			GSM_PrintBitmap(di->df,&Info->Entries[0].Bitmap->Bitmap[0]);
#endif
		Info->Entries[0].ID 	= SMS_NokiaCallerLogo;
		Info->EntriesNum	= 1;
		return TRUE;
	}
	if (SMS->SMS[0].UDH.Type == UDH_NokiaOperatorLogo && SMS->Number == 1) {
		Info->Entries[0].Bitmap = (GSM_MultiBitmap *)malloc(sizeof(GSM_MultiBitmap));
		if (Info->Entries[0].Bitmap == NULL) return FALSE;
		Info->Entries[0].Bitmap->Number = 1;
		PHONE_DecodeBitmap(GSM_NokiaOperatorLogo, SMS->SMS[0].Text+7, &Info->Entries[0].Bitmap->Bitmap[0]);
		NOKIA_DecodeNetworkCode(SMS->SMS[0].Text, Info->Entries[0].Bitmap->Bitmap[0].NetworkCode);
#ifdef DEBUG
		if (di->dl == DL_TEXTALL || di->dl == DL_TEXTALLDATE)
			GSM_PrintBitmap(di->df,&Info->Entries[0].Bitmap->Bitmap[0]);
#endif
		Info->Entries[0].ID 	= SMS_NokiaOperatorLogo;
		Info->EntriesNum	= 1;
		return TRUE;
	}
	if (SMS->SMS[0].UDH.Type == UDH_NokiaProfileLong) {
		return GSM_DecodeNokiaProfile(di, Info, SMS);
	}

	/* Linked sms */
	if (SMS->SMS[0].UDH.Type == UDH_ConcatenatedMessages ||
	    SMS->SMS[0].UDH.Type == UDH_ConcatenatedMessages16bit) {
		return GSM_DecodeLinkedText(di, Info, SMS);
	}
	/* Nokia vCard/vCalendar */
	if (SMS->SMS[0].UDH.Type == UDH_NokiaCalendarLong ||
	    SMS->SMS[0].UDH.Type == UDH_NokiaPhonebookLong) {
		result = GSM_DecodeLinkedText(di, Info, SMS);
		if (result) {
			if (SMS->SMS[0].UDH.Type == UDH_NokiaPhonebookLong) {
				Info->Entries[0].ID = SMS_NokiaVCARD10Long;
			} else {
				Info->Entries[0].ID = SMS_NokiaVCALENDAR10Long;
			}
		}
		return result;
	}
	/* MMS indication */
	if (SMS->SMS[0].UDH.Type == UDH_MMSIndicatorLong) {
		return GSM_DecodeMMSIndication(di, Info, SMS);
	}

	return FALSE;
}

GSM_Error GSM_LinkSMS(GSM_Debug_Info *di, GSM_MultiSMSMessage **InputMessages, GSM_MultiSMSMessage **OutputMessages, gboolean ems)
{
	gboolean			*InputMessagesSorted, copyit,OtherNumbers[GSM_SMS_OTHER_NUMBERS+1],wrong=FALSE;
	int			i,OutputMessagesNum,z,w,m,p;
	int			j;
	GSM_SiemensOTASMSInfo	SiemensOTA,SiemensOTA2;

	i = 0;
	while (InputMessages[i] != NULL) i++;

	OutputMessagesNum = 0;
	OutputMessages[0] = NULL;

	if (i == 0) {
		return ERR_NONE;
	}

	InputMessagesSorted = calloc(i, sizeof(gboolean));
	if (InputMessagesSorted == NULL) return ERR_MOREMEMORY;

	if (ems) {
		for (i = 0; InputMessages[i] != NULL; i++) {
			if (InputMessages[i]->SMS[0].UDH.Type == UDH_UserUDH) {
				w=1;
				while (w < InputMessages[i]->SMS[0].UDH.Length) {
					switch(InputMessages[i]->SMS[0].UDH.Text[w]) {
					case 0x00:
						smfprintf(di, "Adding ID to user UDH - linked SMS with 8 bit ID\n");
						InputMessages[i]->SMS[0].UDH.ID8bit	= InputMessages[i]->SMS[0].UDH.Text[w+2];
						InputMessages[i]->SMS[0].UDH.ID16bit	= -1;
						InputMessages[i]->SMS[0].UDH.AllParts	= InputMessages[i]->SMS[0].UDH.Text[w+3];
						InputMessages[i]->SMS[0].UDH.PartNumber	= InputMessages[i]->SMS[0].UDH.Text[w+4];
						break;
					case 0x08:
						smfprintf(di, "Adding ID to user UDH - linked SMS with 16 bit ID\n");
						InputMessages[i]->SMS[0].UDH.ID8bit	= -1;
						InputMessages[i]->SMS[0].UDH.ID16bit	= InputMessages[i]->SMS[0].UDH.Text[w+2]*256+InputMessages[i]->SMS[0].UDH.Text[w+3];
						InputMessages[i]->SMS[0].UDH.AllParts	= InputMessages[i]->SMS[0].UDH.Text[w+4];
						InputMessages[i]->SMS[0].UDH.PartNumber	= InputMessages[i]->SMS[0].UDH.Text[w+5];
						break;
					default:
						smfprintf(di, "Block %02x\n",InputMessages[i]->SMS[0].UDH.Text[w]);
					}
					smfprintf(di, "id8: %i, id16: %i, part: %i, parts count: %i\n",
						InputMessages[i]->SMS[0].UDH.ID8bit,
						InputMessages[i]->SMS[0].UDH.ID16bit,
						InputMessages[i]->SMS[0].UDH.PartNumber,
						InputMessages[i]->SMS[0].UDH.AllParts);
					w=w+InputMessages[i]->SMS[0].UDH.Text[w+1]+2;
				}
			}
		}
	}

	i=0;
	while (InputMessages[i]!=NULL) {
		/* If this one SMS was sorted earlier, do not touch */
		if (InputMessagesSorted[i]) {
			i++;
			continue;
		}
		/* We have 1'st part of SIEMENS sms. It's single.
		 * We will try to find other parts
		 */
		if (GSM_DecodeSiemensOTASMS(di, &SiemensOTA,&InputMessages[i]->SMS[0]) &&
		    SiemensOTA.PacketNum == 1) {
			OutputMessages[OutputMessagesNum] = (GSM_MultiSMSMessage *)malloc(sizeof(GSM_MultiSMSMessage));
			if (OutputMessages[OutputMessagesNum] == NULL) {
				free(InputMessagesSorted);
				InputMessagesSorted=NULL;
				return ERR_MOREMEMORY;
			}
			OutputMessages[OutputMessagesNum+1] = NULL;

			memcpy(&OutputMessages[OutputMessagesNum]->SMS[0],&InputMessages[i]->SMS[0],sizeof(GSM_SMSMessage));
			OutputMessages[OutputMessagesNum]->Number = 1;
			InputMessagesSorted[i]	= TRUE;
			j		= 1;
			/* We're searching for other parts in sequence */
			while (j!=(int)SiemensOTA.PacketsNum) {

        if(j >= GSM_MAX_MULTI_SMS) {
          smfprintf(di,
            "WARNING: Hard coded message parts limit of %d has been reached,"
						"skipping remaining parts.\n", GSM_MAX_MULTI_SMS);
          break;
        }

				z=0;
				while(InputMessages[z]!=NULL) {
					/* This was sorted earlier or is not single */
					if (InputMessagesSorted[z] || InputMessages[z]->Number != 1) {
						z++;
						continue;
					}
					if (!GSM_DecodeSiemensOTASMS(di, &SiemensOTA2,&InputMessages[z]->SMS[0])) {
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
					if (InputMessages[z]->SMS[0].PDU == SMS_Deliver &&
					    strcmp(DecodeUnicodeString(InputMessages[z]->SMS[0].SMSC.Number),DecodeUnicodeString(InputMessages[i]->SMS[0].SMSC.Number))) {
						z++;
						continue;
					}
					if (InputMessages[z]->SMS[0].PDU == SMS_Deliver &&
					    InputMessages[z]->SMS[0].OtherNumbersNum!=InputMessages[i]->SMS[0].OtherNumbersNum) {
						z++;
						continue;
					}
					if (InputMessages[z]->SMS[0].PDU == SMS_Deliver) {
						for (m=0;m<GSM_SMS_OTHER_NUMBERS+1;m++) {
							OtherNumbers[m]=FALSE;
						}
						for (m=0;m<InputMessages[z]->SMS[0].OtherNumbersNum+1;m++) {
							wrong=TRUE;
							for (p=0;p<InputMessages[i]->SMS[0].OtherNumbersNum+1;p++) {
								if (OtherNumbers[p]) continue;
								if (m==0 && p==0 && !strcmp(DecodeUnicodeString(InputMessages[z]->SMS[0].Number),DecodeUnicodeString(InputMessages[i]->SMS[0].Number))) {
									OtherNumbers[0]=TRUE;
									wrong=FALSE;
									break;
								}
								if (m==0 && p!=0 && !strcmp(DecodeUnicodeString(InputMessages[z]->SMS[0].Number),DecodeUnicodeString(InputMessages[i]->SMS[0].OtherNumbers[p-1]))) {
									OtherNumbers[p]=TRUE;
									wrong=FALSE;
									break;
								}
								if (m!=0 && p==0 && !strcmp(DecodeUnicodeString(InputMessages[z]->SMS[0].OtherNumbers[m-1]),DecodeUnicodeString(InputMessages[i]->SMS[0].Number))) {
									OtherNumbers[0]=TRUE;
									wrong=FALSE;
									break;
								}
								if (m!=0 && p!=0 && !strcmp(DecodeUnicodeString(InputMessages[z]->SMS[0].OtherNumbers[m-1]),DecodeUnicodeString(InputMessages[i]->SMS[0].OtherNumbers[p-1]))) {
									OtherNumbers[p]=TRUE;
									wrong=FALSE;
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
					if (InputMessages[z]->SMS[0].PDU == SMS_Deliver 		&&
					    UnicodeLength(InputMessages[z]->SMS[0].SMSC.Number)==0 	&&
					    UnicodeLength(InputMessages[z]->SMS[0].Number)==0 		&&
					    (InputMessages[z]->SMS[0].DateTime.Day    != InputMessages[i]->SMS[0].DateTime.Day 	  ||
	   				     InputMessages[z]->SMS[0].DateTime.Month  != InputMessages[i]->SMS[0].DateTime.Month  ||
					     InputMessages[z]->SMS[0].DateTime.Year   != InputMessages[i]->SMS[0].DateTime.Year   ||
					     InputMessages[z]->SMS[0].DateTime.Hour   != InputMessages[i]->SMS[0].DateTime.Hour   ||
					     InputMessages[z]->SMS[0].DateTime.Minute != InputMessages[i]->SMS[0].DateTime.Minute ||
					     InputMessages[z]->SMS[0].DateTime.Second != InputMessages[i]->SMS[0].DateTime.Second)) {
						z++;
						continue;
					}
					smfprintf(di, "Found Siemens SMS %i\n",j);
					/* We found correct sms. Copy it */
					memcpy(&OutputMessages[OutputMessagesNum]->SMS[j],&InputMessages[z]->SMS[0],sizeof(GSM_SMSMessage));
					OutputMessages[OutputMessagesNum]->Number++;
					InputMessagesSorted[z]=TRUE;
					break;
				}
				/* Incomplete sequence */
				if (OutputMessages[OutputMessagesNum]->Number==j) {
					smfprintf(di, "Incomplete sequence\n");
					break;
				}
				j++;
			}
			OutputMessagesNum++;
			i = 0;
			continue;
		}
		/* We have some next Siemens sms from sequence */
		if (GSM_DecodeSiemensOTASMS(di, &SiemensOTA,&InputMessages[i]->SMS[0]) &&
		    SiemensOTA.PacketNum > 1) {
			j = 0;
			while (InputMessages[j]!=NULL) {
				if (InputMessagesSorted[j]) {
					j++;
					continue;
				}
				/* We have some not unassigned first sms from sequence.
				 * We can't touch other sms from sequences
				 */
				if (GSM_DecodeSiemensOTASMS(di, &SiemensOTA,&InputMessages[j]->SMS[0]) &&
				    SiemensOTA.PacketNum == 1) {
					break;
				}
				j++;
			}
			if (InputMessages[j]==NULL) {
				OutputMessages[OutputMessagesNum] = (GSM_MultiSMSMessage *)malloc(sizeof(GSM_MultiSMSMessage));
				if (OutputMessages[OutputMessagesNum] == NULL) {
					free(InputMessagesSorted);
					InputMessagesSorted=NULL;
					return ERR_MOREMEMORY;
				}
				OutputMessages[OutputMessagesNum+1] = NULL;

				memcpy(OutputMessages[OutputMessagesNum],InputMessages[i],sizeof(GSM_MultiSMSMessage));
				InputMessagesSorted[i]=TRUE;
				OutputMessagesNum++;
				i = 0;
				continue;
			} else i++;
		}
		copyit = FALSE;
		/* If we have:
		 * - linked sms returned by phone driver
		 * - sms without linking
		 * we copy it to OutputMessages
		 */
		if (InputMessages[i]->Number 			!= 1 	       	||
		    InputMessages[i]->SMS[0].UDH.Type 		== UDH_NoUDH   	||
                    InputMessages[i]->SMS[0].UDH.PartNumber 	== -1) {
			copyit = TRUE;
		}
		/* If we have unknown UDH, we copy it to OutputMessages */
		if (InputMessages[i]->SMS[0].UDH.Type == UDH_UserUDH) {
			if (!ems) copyit = TRUE;
			if (ems && InputMessages[i]->SMS[0].UDH.PartNumber == -1) copyit = TRUE;
		}
		if (copyit) {
			OutputMessages[OutputMessagesNum] = (GSM_MultiSMSMessage *)malloc(sizeof(GSM_MultiSMSMessage));
			if (OutputMessages[OutputMessagesNum] == NULL) {
				free(InputMessagesSorted);
				InputMessagesSorted=NULL;
				return ERR_MOREMEMORY;
			}
			OutputMessages[OutputMessagesNum+1] = NULL;

			memcpy(OutputMessages[OutputMessagesNum],InputMessages[i],sizeof(GSM_MultiSMSMessage));
			InputMessagesSorted[i]=TRUE;
			OutputMessagesNum++;
			i = 0;
			continue;
		}
		/* We have 1'st part of linked sms. It's single.
		 * We will try to find other parts
		 */
		if (InputMessages[i]->SMS[0].UDH.PartNumber == 1) {
			OutputMessages[OutputMessagesNum] = (GSM_MultiSMSMessage *)malloc(sizeof(GSM_MultiSMSMessage));
			if (OutputMessages[OutputMessagesNum] == NULL) {
				free(InputMessagesSorted);
				InputMessagesSorted=NULL;
				return ERR_MOREMEMORY;
			}
			OutputMessages[OutputMessagesNum+1] = NULL;

			memcpy(&OutputMessages[OutputMessagesNum]->SMS[0],&InputMessages[i]->SMS[0],sizeof(GSM_SMSMessage));
			OutputMessages[OutputMessagesNum]->Number = 1;
			InputMessagesSorted[i]	= TRUE;
			j		= 1;
			/* We're searching for other parts in sequence */
			while (j != InputMessages[i]->SMS[0].UDH.AllParts) {

				if(j >= GSM_MAX_MULTI_SMS) {
					smfprintf(di,
						"WARNING: Hard coded message parts limit of %d has been reached,"
			      "skipping remaining parts.\n", GSM_MAX_MULTI_SMS);
					break;
				}

				z=0;
				while(InputMessages[z]!=NULL) {
					/* This was sorted earlier or is not single */
					if (InputMessagesSorted[z] || InputMessages[z]->Number != 1) {
						z++;
						continue;
					}
					if (ems && InputMessages[i]->SMS[0].UDH.Type != UDH_ConcatenatedMessages &&
					    InputMessages[i]->SMS[0].UDH.Type != UDH_ConcatenatedMessages16bit   &&
					    InputMessages[i]->SMS[0].UDH.Type != UDH_UserUDH 			 &&
					    InputMessages[z]->SMS[0].UDH.Type != UDH_ConcatenatedMessages 	 &&
					    InputMessages[z]->SMS[0].UDH.Type != UDH_ConcatenatedMessages16bit   &&
					    InputMessages[z]->SMS[0].UDH.Type != UDH_UserUDH) {
						if (InputMessages[z]->SMS[0].UDH.Type != InputMessages[i]->SMS[0].UDH.Type) {
							z++;
							continue;
						}
					}
					if (!ems && InputMessages[z]->SMS[0].UDH.Type != InputMessages[i]->SMS[0].UDH.Type) {
						z++;
						continue;
					}
					smfprintf(di, "compare %i         %i %i %i %i",
						j+1,
						InputMessages[i]->SMS[0].UDH.ID8bit,
						InputMessages[i]->SMS[0].UDH.ID16bit,
						InputMessages[i]->SMS[0].UDH.PartNumber,
						InputMessages[i]->SMS[0].UDH.AllParts);
					smfprintf(di, "         %i %i %i %i\n",
						InputMessages[z]->SMS[0].UDH.ID8bit,
						InputMessages[z]->SMS[0].UDH.ID16bit,
						InputMessages[z]->SMS[0].UDH.PartNumber,
						InputMessages[z]->SMS[0].UDH.AllParts);
					if (InputMessages[z]->SMS[0].UDH.ID8bit      != InputMessages[i]->SMS[0].UDH.ID8bit	||
							InputMessages[z]->SMS[0].UDH.ID16bit     != InputMessages[i]->SMS[0].UDH.ID16bit	||
							InputMessages[z]->SMS[0].UDH.AllParts    != InputMessages[i]->SMS[0].UDH.AllParts 	||
							(InputMessages[z]->SMS[0].UDH.PartNumber) != j + 1) {
						z++;
						continue;
					}
					/* For SMS_Deliver compare also SMSC and Sender numbers */
					if (InputMessages[z]->SMS[0].PDU == SMS_Deliver &&
					    strcmp(DecodeUnicodeString(InputMessages[z]->SMS[0].SMSC.Number),DecodeUnicodeString(InputMessages[i]->SMS[0].SMSC.Number))) {
						z++;
						continue;
					}
					if (InputMessages[z]->SMS[0].PDU == SMS_Deliver &&
					    InputMessages[z]->SMS[0].OtherNumbersNum!=InputMessages[i]->SMS[0].OtherNumbersNum) {
						z++;
						continue;
					}
					if (InputMessages[z]->SMS[0].PDU == SMS_Deliver) {
						for (m=0;m<GSM_SMS_OTHER_NUMBERS+1;m++) {
							OtherNumbers[m]=FALSE;
						}
						for (m=0;m<InputMessages[z]->SMS[0].OtherNumbersNum+1;m++) {
							wrong=TRUE;
							for (p=0;p<InputMessages[i]->SMS[0].OtherNumbersNum+1;p++) {
								if (OtherNumbers[p]) continue;
								if (m==0 && p==0 && !strcmp(DecodeUnicodeString(InputMessages[z]->SMS[0].Number),DecodeUnicodeString(InputMessages[i]->SMS[0].Number))) {
									OtherNumbers[0]=TRUE;
									wrong=FALSE;
									break;
								}
								if (m==0 && p!=0 && !strcmp(DecodeUnicodeString(InputMessages[z]->SMS[0].Number),DecodeUnicodeString(InputMessages[i]->SMS[0].OtherNumbers[p-1]))) {
									OtherNumbers[p]=TRUE;
									wrong=FALSE;
									break;
								}
								if (m!=0 && p==0 && !strcmp(DecodeUnicodeString(InputMessages[z]->SMS[0].OtherNumbers[m-1]),DecodeUnicodeString(InputMessages[i]->SMS[0].Number))) {
									OtherNumbers[0]=TRUE;
									wrong=FALSE;
									break;
								}
								if (m!=0 && p!=0 && !strcmp(DecodeUnicodeString(InputMessages[z]->SMS[0].OtherNumbers[m-1]),DecodeUnicodeString(InputMessages[i]->SMS[0].OtherNumbers[p-1]))) {
									OtherNumbers[p]=TRUE;
									wrong=FALSE;
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
					if (InputMessages[z]->SMS[0].PDU == SMS_Deliver 		&&
					    UnicodeLength(InputMessages[z]->SMS[0].SMSC.Number)==0 	&&
					    UnicodeLength(InputMessages[z]->SMS[0].Number)==0 		&&
					    (InputMessages[z]->SMS[0].DateTime.Day    != InputMessages[i]->SMS[0].DateTime.Day 	  ||
	   				     InputMessages[z]->SMS[0].DateTime.Month  != InputMessages[i]->SMS[0].DateTime.Month  ||
					     InputMessages[z]->SMS[0].DateTime.Year   != InputMessages[i]->SMS[0].DateTime.Year   ||
					     InputMessages[z]->SMS[0].DateTime.Hour   != InputMessages[i]->SMS[0].DateTime.Hour   ||
					     InputMessages[z]->SMS[0].DateTime.Minute != InputMessages[i]->SMS[0].DateTime.Minute ||
					     InputMessages[z]->SMS[0].DateTime.Second != InputMessages[i]->SMS[0].DateTime.Second)) {
						z++;
						continue;
					}
					/* We found correct sms. Copy it */
					memcpy(&OutputMessages[OutputMessagesNum]->SMS[j],&InputMessages[z]->SMS[0],sizeof(GSM_SMSMessage));
					OutputMessages[OutputMessagesNum]->Number++;
					InputMessagesSorted[z]=TRUE;
					break;
				}
				/* Incomplete sequence */
				if (OutputMessages[OutputMessagesNum]->Number==j) {
					smfprintf(di, "Incomplete sequence\n");
					break;
				}
				j++;
			}
			OutputMessagesNum++;
			i = 0;
			continue;
		}
		/* We have some next linked sms from sequence */
		if (InputMessages[i]->SMS[0].UDH.PartNumber > 1) {
			j = 0;
			while (InputMessages[j]!=NULL) {
				if (InputMessagesSorted[j]) {
					j++;
					continue;
				}
				/* We have some not unassigned first sms from sequence.
				 * We can't touch other sms from sequences
				 */
				if (InputMessages[j]->SMS[0].UDH.PartNumber == 1) break;
				j++;
			}
			if (InputMessages[j]==NULL) {
				OutputMessages[OutputMessagesNum] = (GSM_MultiSMSMessage *)malloc(sizeof(GSM_MultiSMSMessage));
				if (OutputMessages[OutputMessagesNum] == NULL) {
					free(InputMessagesSorted);
					InputMessagesSorted=NULL;
					return ERR_MOREMEMORY;
				}
				OutputMessages[OutputMessagesNum+1] = NULL;

				memcpy(OutputMessages[OutputMessagesNum],InputMessages[i],sizeof(GSM_MultiSMSMessage));
				InputMessagesSorted[i]=TRUE;
				OutputMessagesNum++;
				i = 0;
				continue;
			} else i++;
		}
	}
	free(InputMessagesSorted);
	InputMessagesSorted=NULL;
	return ERR_NONE;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
