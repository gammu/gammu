/* (c) 2002-2004 by Marcin Wiacek */

#include <ctype.h>
#include <string.h>
#include <time.h>

#include "../../gsmcomon.h"
#include "../../misc/coding/coding.h"
#include "../gsmcal.h"
#include "../gsmpbk.h"
#include "../gsmlogo.h"
#include "../gsmring.h"
#include "../gsmdata.h"
#include "../gsmnet.h"
#include "gsmsms.h"
#include "gsmmulti.h"
					
/* EMS Developers' Guidelines from www.sonyericsson.com
 * docs from Alcatel
 */
GSM_Error GSM_EncodeEMSMultiPartSMS(GSM_MultiPartSMSInfo 	*Info,
				    GSM_MultiSMSMessage 	*SMS,
				    GSM_UDH			UDHType)
{
	unsigned char		Buffer[GSM_MAX_SMS_LENGTH*2*MAX_MULTI_SMS];
	int 			i,UsedText,j,Length,Width,Height,z,x,y;
	unsigned int		Len;
	int 			Used,FreeText,FreeBytes,Width2,CopiedText,CopiedSMSText;
	unsigned char		UDHID;
	GSM_Bitmap		Bitmap,Bitmap2;
	GSM_Ringtone		Ring;
	GSM_Coding_Type 	Coding 	= SMS_Coding_Default;
	GSM_Phone_Bitmap_Types	BitmapType;
	MultiPartSMSEntry 	*Entry;
	bool			start;
	GSM_DateTime		Date;

#ifdef DEBUG
	if (UDHType != UDH_NoUDH) dbgprintf("linked EMS\n");
#endif

	if (Info->UnicodeCoding) Coding = SMS_Coding_Unicode;

	/* Cleaning on the start */
	for (i=0;i<MAX_MULTI_SMS;i++) {
		GSM_SetDefaultSMSData(&SMS->SMS[i]);
		SMS->SMS[i].UDH.Type = UDHType;
		GSM_EncodeUDHHeader(&SMS->SMS[i].UDH);
		SMS->SMS[i].Coding = Coding;
	}

	/* Packing */
	for (i=0;i<Info->EntriesNum;i++) {
		Entry = &Info->Entries[i];

		switch (Entry->ID) {
		case SMS_ConcatenatedTextLong:
		case SMS_ConcatenatedTextLong16bit:
			Len = 0;
			while(1) {
				if (Entry->Left   || Entry->Right      ||
				    Entry->Center || Entry->Large      ||
				    Entry->Small  || Entry->Bold       ||
				    Entry->Italic || Entry->Underlined ||
				    Entry->Strikethrough) {
					Buffer[0] = 0x0A;	/* ID for text format   */
					Buffer[1] = 0x03;	/* length of rest	*/
					Buffer[2] = 0x00; 	/* Position in EMS msg	*/
					Buffer[3] = 0x00;	/* how many chars 	*/
					Buffer[4] = 0x00;	/* formatting bits	*/
					if (Entry->Left) {
					} else if (Entry->Right) {	Buffer[4] |= 1;
					} else if (Entry->Center) {	Buffer[4] |= 2;
					} else 				Buffer[4] |= 3;
					if (Entry->Large) {		Buffer[4] |= 4;
					} else if (Entry->Small) {	Buffer[4] |= 8;}
					if (Entry->Bold) 		Buffer[4] |= 16;
				    	if (Entry->Italic) 		Buffer[4] |= 32;
					if (Entry->Underlined) 		Buffer[4] |= 64;
				    	if (Entry->Strikethrough) 	Buffer[4] |= 128;
					GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,5,true,&UsedText,&CopiedText,&CopiedSMSText);
					GSM_Find_Free_Used_SMS2(Coding,SMS->SMS[SMS->Number], &UsedText, &FreeText, &FreeBytes);
					if (FreeText == 0) continue;
				}
				GSM_AddSMS_Text_UDH(SMS,Coding,Entry->Buffer+Len*2,UnicodeLength(Entry->Buffer) - Len,false,&UsedText,&CopiedText,&CopiedSMSText);
				if (Entry->Left   || Entry->Right      ||
				    Entry->Center || Entry->Large      ||
				    Entry->Small  || Entry->Bold       ||
				    Entry->Italic || Entry->Underlined ||
				    Entry->Strikethrough) {
					SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-3] = UsedText;
					SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-2] = CopiedSMSText;
				}
				Len += CopiedText;
				if (Len == UnicodeLength(Entry->Buffer)) break;
				dbgprintf("%i %i\n",Len,UnicodeLength(Entry->Buffer));
			}
			break;
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
			GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,4,true,&UsedText,&CopiedText,&CopiedSMSText);
			SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-2] = UsedText;
			break;
		case SMS_EMSSonyEricssonSound:
		case SMS_EMSSound10:
		case SMS_EMSSound12:
			if (Entry->Protected) {
				Buffer[0] = 0x17; /* ID for ODI 		 */
				Buffer[1] = 2;	  /* Length of rest 		 */
				Buffer[2] = 1;	  /* Number of protected objects */
				Buffer[3] = 1;	  /* 1=Protected,0=Not protected */
				GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,4,true,&UsedText,&CopiedText,&CopiedSMSText);
			}

			Length = 128; /* 128 bytes is maximal length from specs */
			switch (Entry->ID) {
			case SMS_EMSSound10:
				Entry->RingtoneNotes = GSM_EncodeEMSSound(*Entry->Ringtone, Buffer+3, &Length, 1.0, true);
				break;
			case SMS_EMSSound12:
				Entry->RingtoneNotes = GSM_EncodeEMSSound(*Entry->Ringtone, Buffer+3, &Length, 1.2, true);
				break;
			case SMS_EMSSonyEricssonSound:
				Entry->RingtoneNotes = GSM_EncodeEMSSound(*Entry->Ringtone, Buffer+3, &Length, 0, true);
				break;
			default:
				break;
			}

			Buffer[0] = 0x0C;	/* ID for EMS sound 	*/
			Buffer[1] = Length+1;	/* Length of rest 	*/
			Buffer[2] = 0x00; 	/* Position in EMS msg 	*/
			GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,Length+3,true,&UsedText,&CopiedText,&CopiedSMSText);
			SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-Length-1] = UsedText;
			break;
		case SMS_EMSSonyEricssonSoundLong:
		case SMS_EMSSound10Long:
		case SMS_EMSSound12Long:
			Ring = *Entry->Ringtone;

			/* First check if we can use classic format */
			Length = 128; /* 128 bytes is maximal length from specs */
			switch (Entry->ID) {
			case SMS_EMSSound10Long:
				Entry->RingtoneNotes = GSM_EncodeEMSSound(Ring, Buffer+3, &Length, 1.0, true);
				break;
			case SMS_EMSSound12Long:
				Entry->RingtoneNotes = GSM_EncodeEMSSound(Ring, Buffer+3, &Length, 1.2, true);
				break;
			case SMS_EMSSonyEricssonSoundLong:
				Entry->RingtoneNotes = GSM_EncodeEMSSound(Ring, Buffer+3, &Length, 0, true);
				break;
			default:
				break;
			}
			if (Entry->RingtoneNotes == Ring.NoteTone.NrCommands) {
				if (Entry->Protected) {
					Buffer[0] = 0x17; /* ID for ODI 		 */
					Buffer[1] = 2;	  /* Length of rest 		 */
					Buffer[2] = 1;	  /* Number of protected objects */
					Buffer[3] = 1;	  /* 1=Protected,0=Not protected */
					GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,4,true,&UsedText,&CopiedText,&CopiedSMSText);
				}

				Buffer[0] = 0x0C;	/* ID for EMS sound 	*/
				Buffer[1] = Length+1;	/* Length of rest 	*/
				Buffer[2] = 0x00; 	/* Position in EMS msg 	*/
				GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,Length+3,true,&UsedText,&CopiedText,&CopiedSMSText);
				SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-Length-1] = UsedText;
				break;
			}

			/* Find free place in first SMS */
			GSM_Find_Free_Used_SMS2(Coding,SMS->SMS[SMS->Number], &UsedText, &FreeText, &FreeBytes);
			Length = FreeBytes - 3;
			if (Entry->Protected) 	Length = Length - 4;
			if (Length < 0) 	Length = 128;
			if (Length > 128) 	Length = 128;

			Ring = *Entry->Ringtone;

			/* Checking number of SMS */
			Used 		= 0;
			FreeBytes 	= 0;
			start		= true;
			while (1) {
				if (FreeBytes != 0) {
					z = 0;
					for (j=FreeBytes;j<Ring.NoteTone.NrCommands;j++) {
						Ring.NoteTone.Commands[z] = Ring.NoteTone.Commands[j];
						z++;
					}
					Ring.NoteTone.NrCommands -= FreeBytes;
					if (Ring.NoteTone.NrCommands == 0) break;
					Length = 128; /* 128 bytes is maximal length from specs */
				}
				switch (Entry->ID) {
				case SMS_EMSSound10Long:
					FreeBytes = GSM_EncodeEMSSound(Ring, Buffer+3, &Length, 1.0, start);
					break;
				case SMS_EMSSound12Long:
					FreeBytes = GSM_EncodeEMSSound(Ring, Buffer+3, &Length, 1.2, start);
					break;
				case SMS_EMSSonyEricssonSoundLong:
					FreeBytes = GSM_EncodeEMSSound(Ring, Buffer+3, &Length, 0, start);
					break;
				default:
					break;
				}
				start = false;
				Used++;
			}
			dbgprintf("Used SMS: %i\n",Used);

			if (Entry->Protected) {
				Buffer[0] = 0x17;   /* ID for ODI 		   */
				Buffer[1] = 2;	    /* Length of rest 		   */
				Buffer[2] = Used+1; /* Number of protected objects */
				Buffer[3] = 1;	    /* 1=Protected,0=Not protected */
				GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,4,true,&UsedText,&CopiedText,&CopiedSMSText);
			}

			/* Save UPI UDH */
			Buffer[0] = 0x13;	/* ID for UPI		*/
			Buffer[1] = 1;		/* Length of rest 	*/
			Buffer[2] = Used; 	/* Number of used parts	*/
			GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,3,true,&UsedText,&CopiedText,&CopiedSMSText);

			/* Find free place in first SMS */
			GSM_Find_Free_Used_SMS2(Coding,SMS->SMS[SMS->Number], &UsedText, &FreeText, &FreeBytes);
			Length 	= FreeBytes - 3;
			if (Length < 0) 	Length = 128;
			if (Length > 128) 	Length = 128;

			Ring = *Entry->Ringtone;

			/* Saving */
			FreeBytes = 0;
			start	  = true;
			while (1) {
				if (FreeBytes != 0) {
					z = 0;
					for (j=FreeBytes;j<Ring.NoteTone.NrCommands;j++) {
						Ring.NoteTone.Commands[z] = Ring.NoteTone.Commands[j];
						z++;
					}
					Ring.NoteTone.NrCommands -= FreeBytes;
					if (Ring.NoteTone.NrCommands == 0) break;
					Length = 128; /* 128 bytes is maximal length from specs */
				}
				switch (Entry->ID) {
				case SMS_EMSSound10Long:
					FreeBytes = GSM_EncodeEMSSound(Ring, Buffer+3, &Length, 1.0, start);
					break;
				case SMS_EMSSound12Long:
					FreeBytes = GSM_EncodeEMSSound(Ring, Buffer+3, &Length, 1.2, start);
					break;
				case SMS_EMSSonyEricssonSoundLong:
					FreeBytes = GSM_EncodeEMSSound(Ring, Buffer+3, &Length, 0, start);
					break;
				default:
					break;
				}
				Buffer[0] = 0x0C;	/* ID for EMS sound 	*/
				Buffer[1] = Length+1;	/* Length of rest 	*/
				Buffer[2] = 0x00; 	/* Position in EMS msg 	*/
				GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,Length+3,true,&UsedText,&CopiedText,&CopiedSMSText);
				SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-Length-1] = UsedText;
				start = false;
			}

			Entry->RingtoneNotes = Entry->Ringtone->NoteTone.NrCommands;

			break;
		case SMS_EMSAnimation:
			if (Entry->Protected) {
				Buffer[0] = 0x17; /* ID for ODI 		 */
				Buffer[1] = 2;	  /* Length of rest 		 */
				Buffer[2] = 1;	  /* Number of protected objects */
				Buffer[3] = 1;	  /* 1=Protected,0=Not protected */
				GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,4,true,&UsedText,&CopiedText,&CopiedSMSText);
			}

			if (Entry->Bitmap->Bitmap[0].BitmapWidth > 8 || Entry->Bitmap->Bitmap[0].BitmapHeight > 8) {
				BitmapType = GSM_EMSMediumPicture;	/* Bitmap 16x16 */
				Buffer[0]  = 0x0E;			/* ID for 16x16 animation */
			} else {		
				BitmapType = GSM_EMSSmallPicture;	/* Bitmap 8x8 */
				Buffer[0]  = 0x0F;			/* ID for 8x8 animation */
			}
			Length = PHONE_GetBitmapSize(BitmapType,0,0);
	
			Buffer[1] = Length*Entry->Bitmap->Number + 1;	/* Length of rest 	 */
			Buffer[2] = 0x00; 	 		 	/* Position in EMS msg	 */
			for (j=0;j<Entry->Bitmap->Number;j++) {
				PHONE_EncodeBitmap(BitmapType, Buffer+3+j*Length, &Entry->Bitmap->Bitmap[j]);
			}
			GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,3+Length*Entry->Bitmap->Number,true,&UsedText,&CopiedText,&CopiedSMSText);
			SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-1-Length*Entry->Bitmap->Number] = UsedText;
			break;
		case SMS_EMSFixedBitmap:
			if (Entry->Protected) {
				Buffer[0] = 0x17; /* ID for ODI 		 */
				Buffer[1] = 2;	  /* Length of rest 		 */
				Buffer[2] = 1;	  /* Number of protected objects */
				Buffer[3] = 1;	  /* 1=Protected,0=Not protected */
				GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,4,true,&UsedText,&CopiedText,&CopiedSMSText);
			}

			if (Entry->Bitmap->Bitmap[0].BitmapWidth > 16 || Entry->Bitmap->Bitmap[0].BitmapHeight > 16) {
				BitmapType = GSM_EMSBigPicture;		/* Bitmap 32x32 	*/
				Buffer[0]  = 0x10;			/* ID for EMS bitmap	*/
			} else {
				BitmapType = GSM_EMSMediumPicture;	/* Bitmap 16x16 	*/
				Buffer[0]  = 0x11;			/* ID for EMS bitmap	*/
			}
			Length = PHONE_GetBitmapSize(BitmapType,0,0);
			PHONE_GetBitmapWidthHeight(BitmapType, &Width, &Height);
	
			Buffer[1] = Length + 1;		/* Length of rest 	*/
			Buffer[2] = 0x00; 		/* Position in EMS msg	*/
			PHONE_EncodeBitmap(BitmapType,Buffer+3, &Entry->Bitmap->Bitmap[0]);
			GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,3+Length,true,&UsedText,&CopiedText,&CopiedSMSText);
			SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-1-Length] = UsedText;
			break;
		case SMS_EMSVariableBitmapLong:
			BitmapType 	= GSM_EMSVariablePicture;
			Width 		= Entry->Bitmap->Bitmap[0].BitmapWidth;
			Height 		= Entry->Bitmap->Bitmap[0].BitmapHeight;
			Bitmap = Entry->Bitmap->Bitmap[0];

			/* First check if we can use classical format */
			while (1) {
				/* Width should be multiply of 8 */
				while (Width % 8 != 0) Width--;

				/* specs */
				if (Width <= 96 && Height <= 128) break;

				Height--;
			}
			Length = PHONE_GetBitmapSize(BitmapType,Width,Height);
			if (Length <= 128) {
				if (Entry->Protected) {
					Buffer[0] = 0x17; /* ID for ODI 		 */
					Buffer[1] = 2;	  /* Length of rest 		 */
					Buffer[2] = 1;	  /* Number of protected objects */
					Buffer[3] = 1;    /* 1=Protected,0=Not protected */
					GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,4,true,&UsedText,&CopiedText,&CopiedSMSText);
				}

				Buffer[0] = 0x12;		/* ID for EMS bitmap	*/
				Buffer[1] = Length + 3;		/* Length of rest 	*/
				Buffer[2] = 0x00; 		/* Position in EMS msg	*/
				Buffer[3] = Width/8;		/* Bitmap width/8 	*/
				Buffer[4] = Height;		/* Bitmap height  	*/

				GSM_ResizeBitmap(&Bitmap, &Entry->Bitmap->Bitmap[0], Width, Height);
#ifdef DEBUG
				if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) GSM_PrintBitmap(di.df,&Bitmap);
#endif
				PHONE_EncodeBitmap(BitmapType,Buffer+5, &Bitmap);
				GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,5+Length,true,&UsedText,&CopiedText,&CopiedSMSText);
				SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-3-Length] = UsedText;
				break;
			}

			/* Find free place in first SMS */
			GSM_Find_Free_Used_SMS2(Coding,SMS->SMS[SMS->Number], &UsedText, &FreeText, &FreeBytes);
			Used 	= 0;
			Length 	= FreeBytes - 3;
			if (Entry->Protected)	Length = Length - 4;
			if (Length < 0) 	Length = 128;
			if (Length > 128) 	Length = 128;

			/* Checking number of SMS */
			FreeBytes = 0;
			while (FreeBytes != Width) {
				Width2 = 8;
				while (FreeBytes + Width2 != Width) {
					if (PHONE_GetBitmapSize(BitmapType,Width2+8,Height) > Length) break;

					Width2 = Width2 + 8;
				}
				FreeBytes 	= FreeBytes + Width2;
				Length 		= 128;
				Used ++;
			}
			dbgprintf("Used SMS: %i\n",Used);

			if (Entry->Protected) {
				Buffer[0] = 0x17;   /* ID for ODI 		   */
				Buffer[1] = 2;	    /* Length of rest 		   */
				Buffer[2] = Used+1; /* Number of protected objects */
				Buffer[3] = 1;	    /* 1=Protected,0=Not protected */
				GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,4,true,&UsedText,&CopiedText,&CopiedSMSText);
			}

			/* Save UPI UDH */
			Buffer[0] = 0x13;	/* ID for UPI		*/
			Buffer[1] = 1;		/* Length of rest 	*/
			Buffer[2] = Used; 	/* Number of used parts	*/

			/* Find free place in first SMS */
			GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,3,true,&UsedText,&CopiedText,&CopiedSMSText);
			GSM_Find_Free_Used_SMS2(Coding,SMS->SMS[SMS->Number], &UsedText, &FreeText, &FreeBytes);
			Length 	= FreeBytes - 3;
			if (Length < 0) 	Length = 128;
			if (Length > 128) 	Length = 128;

#ifdef DEBUG
			if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) GSM_PrintBitmap(di.df,&Bitmap);
#endif

			/* Saving SMS */
			FreeBytes = 0;
			while (FreeBytes != Width) {
				Width2 = 8;
				while (FreeBytes + Width2 != Width) {
					if (PHONE_GetBitmapSize(BitmapType,Width2+8,Height) > Length) break;

					Width2 = Width2 + 8;
				}

				/* Copying part of bitmap to new structure */
				Bitmap2.BitmapWidth  = Width2;
				Bitmap2.BitmapHeight = Height;				
				GSM_ClearBitmap(&Bitmap2);
				for (x=0;x<Width2;x++) {
					for (y=0;y<Height;y++) {
						if (GSM_IsPointBitmap(&Bitmap,x+FreeBytes,y)) {
							GSM_SetPointBitmap(&Bitmap2, x, y);
						}
					}
				}
#ifdef DEBUG
				if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) GSM_PrintBitmap(di.df,&Bitmap2);
#endif

				/* Adding new bitmap to SMS */
				Length = PHONE_GetBitmapSize(BitmapType,Width2,Height);
				Buffer[0] = 0x12;		/* ID for EMS bitmap	*/
				Buffer[1] = Length + 3;		/* Length of rest 	*/
				Buffer[2] = 0x00; 		/* Position in EMS msg	*/
				Buffer[3] = Width2/8;		/* Bitmap width/8 	*/
				Buffer[4] = Height;		/* Bitmap height  	*/
				PHONE_EncodeBitmap(BitmapType,Buffer+5, &Bitmap2);
				GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,5+Length,true,&UsedText,&CopiedText,&CopiedSMSText);
				SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-3-Length] = UsedText;

				FreeBytes 	= FreeBytes + Width2;
				Length 	= 128;
			}
			break;
		case SMS_EMSVariableBitmap:
			if (Entry->Protected) {
				Buffer[0] = 0x17; /* ID for ODI 		 */
				Buffer[1] = 2;	  /* Length of rest 		 */
				Buffer[2] = 1;	  /* Number of protected objects */
				Buffer[3] = 1;	  /* 1=Protected,0=Not protected */
				GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,4,true,&UsedText,&CopiedText,&CopiedSMSText);
			}

			BitmapType 	= GSM_EMSVariablePicture;
			Width 		= Entry->Bitmap->Bitmap[0].BitmapWidth;
			Height 		= Entry->Bitmap->Bitmap[0].BitmapHeight;

			while (1) {
				/* Width should be multiply of 8 */
				while (Width % 8 != 0) Width--;

				/* specs */
				if (PHONE_GetBitmapSize(BitmapType,Width,Height) <= 128) break;

				Height--;
			}

			Length = PHONE_GetBitmapSize(BitmapType,Width,Height);

			Buffer[0] = 0x12;		/* ID for EMS bitmap	*/
			Buffer[1] = Length + 3;		/* Length of rest 	*/
			Buffer[2] = 0x00; 		/* Position in EMS msg	*/
			Buffer[3] = Width/8;		/* Bitmap width/8 	*/
			Buffer[4] = Height;		/* Bitmap height  	*/

			GSM_ResizeBitmap(&Bitmap, &Entry->Bitmap->Bitmap[0], Width, Height);
#ifdef DEBUG
			if (di.dl == DL_TEXTALL || di.dl == DL_TEXTALLDATE) GSM_PrintBitmap(di.df,&Bitmap);
#endif
			PHONE_EncodeBitmap(BitmapType,Buffer+5, &Bitmap);
			GSM_AddSMS_Text_UDH(SMS,Coding,Buffer,5+Length,true,&UsedText,&CopiedText,&CopiedSMSText);
			SMS->SMS[SMS->Number].UDH.Text[SMS->SMS[SMS->Number].UDH.Length-3-Length] = UsedText;
			break;
		default:
			break;
		}
	}

	SMS->Number++;

	if (UDHType == UDH_ConcatenatedMessages) {
		UDHID = GSM_MakeSMSIDFromTime();
		for (i=0;i<SMS->Number;i++) {
			SMS->SMS[i].UDH.Text[2+1] = UDHID;
			SMS->SMS[i].UDH.Text[3+1] = SMS->Number;
			SMS->SMS[i].UDH.Text[4+1] = i+1;
		}
	}
	if (UDHType == UDH_ConcatenatedMessages16bit) {
		UDHID = GSM_MakeSMSIDFromTime();
		GSM_GetCurrentDateTime (&Date);	
		for (i=0;i<SMS->Number;i++) {
			SMS->SMS[i].UDH.Text[2+1] = Date.Hour;
			SMS->SMS[i].UDH.Text[3+1] = UDHID;
			SMS->SMS[i].UDH.Text[4+1] = SMS->Number;
			SMS->SMS[i].UDH.Text[5+1] = i+1;
		}
	}

#ifdef DEBUG
	dbgprintf("SMS number is %i\n",SMS->Number);
	for (i=0;i<SMS->Number;i++) {
		dbgprintf("UDH length %i\n",SMS->SMS[i].UDH.Length);
		DumpMessage(di.df, di.dl, SMS->SMS[i].UDH.Text, SMS->SMS[i].UDH.Length);
		dbgprintf("SMS length %i\n",UnicodeLength(SMS->SMS[i].Text)*2);
		DumpMessage(di.df, di.dl, SMS->SMS[i].Text, UnicodeLength(SMS->SMS[i].Text)*2);
	}
#endif
	return ERR_NONE;
}

static bool AddEMSText(GSM_SMSMessage *SMS, GSM_MultiPartSMSInfo *Info, int *Pos, int Len)
{
	int BufferLen;

	if (Len==0) return true;

	if (Info->Entries[Info->EntriesNum].ID!=SMS_ConcatenatedTextLong &&
	    Info->Entries[Info->EntriesNum].ID!=0) {
		(Info->EntriesNum)++;
	}
	BufferLen = UnicodeLength(Info->Entries[Info->EntriesNum].Buffer)*2;
	switch (SMS->Coding) {
	case SMS_Coding_8bit:
//		memcpy(Info->Entries[Info->EntriesNum].Buffer+BufferLen,SMS->Text+(*Pos),Len);
//		BufferLen+=Len;
//		(*Pos)+=Len;
		break;
	case SMS_Coding_Unicode:
	case SMS_Coding_Default:
		Info->Entries[Info->EntriesNum].Buffer = realloc(Info->Entries[Info->EntriesNum].Buffer, BufferLen + (Len * 2) + 2);
		if (Info->Entries[Info->EntriesNum].Buffer == NULL) return false;
		memcpy(Info->Entries[Info->EntriesNum].Buffer + BufferLen, SMS->Text + (*Pos) *2, Len * 2);
		BufferLen += Len * 2;
		break;
	}
	(*Pos)+=Len;
	Info->Entries[Info->EntriesNum].Buffer[BufferLen]	= 0;
	Info->Entries[Info->EntriesNum].Buffer[BufferLen+1]	= 0;
	Info->Entries[Info->EntriesNum].ID			= SMS_ConcatenatedTextLong;
	return true;
}

bool GSM_DecodeEMSMultiPartSMS(GSM_MultiPartSMSInfo 	*Info,
			       GSM_MultiSMSMessage 	*SMS)
{
	int  			i, w, Pos, z, UPI = 1, width, height;
 	bool 			RetVal = false, NewPicture = true;
	GSM_Phone_Bitmap_Types 	BitmapType;
	GSM_Bitmap		Bitmap,Bitmap2;

	for (i=0;i<MAX_MULTI_SMS;i++) {
		Info->Entries[i].ID	   = 0;
	}

	for (i=0;i<SMS->Number;i++) {
		Pos 	= 0;
		w	= 1;
		while (w < SMS->SMS[i].UDH.Length) {
			if (Info->EntriesNum + 1 == MAX_MULTI_SMS) {
				dbgprintf("Couldn't parse SMS, contains too many EMS parts!\n");
				return false;
			}
			switch(SMS->SMS[i].UDH.Text[w]) {
			case 0x00:
				dbgprintf("UDH part - linked SMS with 8 bit ID\n");
				break;
			case 0x08:
				dbgprintf("UDH part - linked SMS with 16 bit ID\n");
				break;
//			case 0x0A:
//				dbgprintf("UDH part - EMS text formatting\n");
//				break;
			case 0x0B:
				dbgprintf("UDH part - default EMS sound\n");
				if (SMS->SMS[i].UDH.Text[w+2] > Pos) {
					z = Pos;
					if (!AddEMSText(&SMS->SMS[i], Info, &Pos, SMS->SMS[i].UDH.Text[w+2]-z)) return false;
				}
				if (Info->Entries[Info->EntriesNum].ID != 0) (Info->EntriesNum)++;
				Info->Entries[Info->EntriesNum].Number 	= SMS->SMS[i].UDH.Text[w+3];
				Info->Entries[Info->EntriesNum].ID 	= SMS_EMSPredefinedSound;
				RetVal = true;
				break;
//			case 0x0C:
//				dbgprintf("UDH part - EMS sound\n");
//				break;
			case 0x0D:
				dbgprintf("UDH part - default EMS animation\n");
				if (SMS->SMS[i].UDH.Text[w+2] > Pos) {
					z = Pos;
					if (!AddEMSText(&SMS->SMS[i], Info, &Pos, SMS->SMS[i].UDH.Text[w+2]-z)) return false;
				}
				if (Info->Entries[Info->EntriesNum].ID != 0) (Info->EntriesNum)++;
				Info->Entries[Info->EntriesNum].Number 	= SMS->SMS[i].UDH.Text[w+3];
				Info->Entries[Info->EntriesNum].ID 	= SMS_EMSPredefinedAnimation;
				RetVal = true;
				break;
			case 0x0E:
			case 0x0F:
				if (SMS->SMS[i].UDH.Text[w] == 0x0E) {
					dbgprintf("UDH part - EMS 16x16 animation\n");
					BitmapType = GSM_EMSMediumPicture;
				} else {
					dbgprintf("UDH part - EMS 8x8 animation\n");
					BitmapType = GSM_EMSSmallPicture;
				}
				dbgprintf("Position - %i\n",SMS->SMS[i].UDH.Text[w+2]);
				if (SMS->SMS[i].UDH.Text[w+2] > Pos) {
					z = Pos;
					if (!AddEMSText(&SMS->SMS[i], Info, &Pos, SMS->SMS[i].UDH.Text[w+2]-z)) return false;
				}
				(Info->EntriesNum)++;
				Info->Entries[Info->EntriesNum].ID 		= SMS_EMSAnimation;
				Info->Entries[Info->EntriesNum].Bitmap = (GSM_MultiBitmap *)malloc(sizeof(GSM_MultiBitmap));
				if (Info->Entries[Info->EntriesNum].Bitmap == NULL) return false;
				Info->Entries[Info->EntriesNum].Bitmap->Number 	= 0;
				for (z=0;z<((SMS->SMS[i].UDH.Text[w+1]-1)/PHONE_GetBitmapSize(BitmapType,0,0));z++) {
					Info->Entries[Info->EntriesNum].Bitmap->Bitmap[z].Type = GSM_PictureImage;
					PHONE_DecodeBitmap(BitmapType, 
						SMS->SMS[i].UDH.Text + w + 3 + PHONE_GetBitmapSize(BitmapType,0,0) * z,
						&Info->Entries[Info->EntriesNum].Bitmap->Bitmap[z]);
					Info->Entries[Info->EntriesNum].Bitmap->Number++;
				}
				RetVal = true;
				break;
			case 0x10:
			case 0x11:
				if (SMS->SMS[i].UDH.Text[w] == 0x10) {
					dbgprintf("UDH part - EMS 32x32 picture\n");
					BitmapType = GSM_EMSBigPicture;
				} else {
					dbgprintf("UDH part - EMS 16x16 picture\n");
					BitmapType = GSM_EMSMediumPicture;
				}
				dbgprintf("Position - %i\n",SMS->SMS[i].UDH.Text[w+2]);
				if (SMS->SMS[i].UDH.Text[w+2] > Pos) {
					z = Pos;
					if (!AddEMSText(&SMS->SMS[i], Info, &Pos, SMS->SMS[i].UDH.Text[w+2]-z)) return false;
				}
				if (Info->Entries[Info->EntriesNum].ID != 0) (Info->EntriesNum)++;
				Info->Entries[Info->EntriesNum].Bitmap = (GSM_MultiBitmap *)malloc(sizeof(GSM_MultiBitmap));
				if (Info->Entries[Info->EntriesNum].Bitmap == NULL) return false;
				PHONE_DecodeBitmap(BitmapType, 
					SMS->SMS[i].UDH.Text + w + 3, 
					&Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0]);
				Info->Entries[Info->EntriesNum].Bitmap->Number 		  = 1;
				Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0].Text[0] = 0;
				Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0].Text[1] = 0;
				Info->Entries[Info->EntriesNum].ID = SMS_EMSFixedBitmap;
				RetVal = true;
				break;
			case 0x12:
				dbgprintf("UDH part - EMS variable width bitmap\n");
				if (SMS->SMS[i].UDH.Text[w+2] > Pos) {
					z = Pos;
					if (!AddEMSText(&SMS->SMS[i], Info, &Pos, SMS->SMS[i].UDH.Text[w+2]-z)) return false;
				}
				if (NewPicture) {
					(Info->EntriesNum)++;
					Info->Entries[Info->EntriesNum].Bitmap->Number = 0;
					Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0].BitmapWidth  = 0;
					Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0].BitmapHeight = 0;
				}
				Bitmap.BitmapWidth  = SMS->SMS[i].UDH.Text[w+3]*8;
				Bitmap.BitmapHeight = SMS->SMS[i].UDH.Text[w+4];
				Info->Entries[Info->EntriesNum].Bitmap = (GSM_MultiBitmap *)malloc(sizeof(GSM_MultiBitmap));
				if (Info->Entries[Info->EntriesNum].Bitmap == NULL) return false;
				if (NewPicture) {
					Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0].BitmapWidth  = Bitmap.BitmapWidth;
					Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0].BitmapHeight = Bitmap.BitmapHeight;
					PHONE_DecodeBitmap(GSM_EMSVariablePicture, 
						SMS->SMS[i].UDH.Text + w + 5, 
						&Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0]);
				} else {
					PHONE_DecodeBitmap(GSM_EMSVariablePicture, 
						SMS->SMS[i].UDH.Text + w + 5, 
						&Bitmap);
					Bitmap2 = Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0];
					Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0].BitmapWidth  = Bitmap.BitmapWidth+Bitmap2.BitmapWidth;
					Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0].BitmapHeight = Bitmap2.BitmapHeight;
					for (width=0;width<Bitmap2.BitmapWidth;width++) {
						for (height=0;height<Bitmap2.BitmapHeight;height++) {
							if (GSM_IsPointBitmap(&Bitmap2, width, height)) {
								GSM_SetPointBitmap(&Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0],width,height);
							} else {
								GSM_ClearPointBitmap(&Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0],width,height);
							}
						}
					}
					for (width=0;width<Bitmap.BitmapWidth;width++) {
						for (height=0;height<Bitmap2.BitmapHeight;height++) {
							if (GSM_IsPointBitmap(&Bitmap, width, height)) {
								GSM_SetPointBitmap(&Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0],width+Bitmap2.BitmapWidth,height);
							} else {
								GSM_ClearPointBitmap(&Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0],width+Bitmap2.BitmapWidth,height);
							}
						}
					}
				}
				if (UPI == 1) {
					Info->Entries[Info->EntriesNum].Bitmap->Number 		  = 1;
					Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0].Text[0] = 0;
					Info->Entries[Info->EntriesNum].Bitmap->Bitmap[0].Text[1] = 0;
					Info->Entries[Info->EntriesNum].ID = SMS_EMSVariableBitmap;
					RetVal 		= true;
					NewPicture 	= true;
					dbgprintf("New variable picture\n");
				} else {
					NewPicture = false;
					UPI--;
				}
				break;
			case 0x13:
				dbgprintf("UDH part - UPI\n");
				dbgprintf("Value %i\n",SMS->SMS[i].UDH.Text[w+2]);
				UPI = SMS->SMS[i].UDH.Text[w+2];
				break;
			case 0x17:
				dbgprintf("UDH part - Object Distribution Indicator (Media Rights Protecting) ignored now\n");
				break;
			default:
				dbgprintf("UDH part - block %02x\n",SMS->SMS[i].UDH.Text[w]);
				Info->Unknown = true;
			} /* switch */
			w=w+SMS->SMS[i].UDH.Text[w+1]+2;
		} /* while */
		if (!AddEMSText(&SMS->SMS[i], Info, &Pos, SMS->SMS[i].Length-Pos)) return false;
		RetVal = true;
	}
	if (RetVal) (Info->EntriesNum)++;
	return RetVal;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
