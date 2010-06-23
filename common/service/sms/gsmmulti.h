/* (c) 2002-2004 by Marcin Wiacek */

#ifndef __gsm_multi_h
#define __gsm_multi_h

#if defined(_MSC_VER) && defined(__cplusplus)
    extern "C" {
#endif

#include "../../gsmcomon.h"
#include "../gsmlogo.h"
#include "../gsmcal.h"
#include "../gsmpbk.h"
#include "../gsmdata.h"
#include "../gsmring.h"
#include "gsmsms.h"

/* ---------------------- multi SMS --------------------------------------- */

/* Identifiers for Smart Messaging 3.0 multipart SMS */

#define SM30_ISOTEXT      0 /* ISO 8859-1 text */
#define SM30_UNICODETEXT  1
#define SM30_OTA	  2
#define SM30_RINGTONE     3
#define SM30_PROFILENAME  4
/* ... */
#define SM30_SCREENSAVER  6

/* Identifiers for Alcatel Terminal Data Download */
#define ALCATELTDD_PICTURE     4
#define ALCATELTDD_ANIMATION   5
#define ALCATELTDD_SMSTEMPLATE 6

void GSM_SMSCounter(int		 	MessageLength,
		    unsigned char       *MessageBuffer,
		    GSM_UDH	     	UDHType,
		    GSM_Coding_Type     Coding,
		    int		 	*SMSNum,
		    int		 	*CharsLeft);

#define MAX_MULTI_SMS 10

/**
 * Multiple SMS messages, used for Smart Messaging 3.0/EMS.
 */
typedef struct {
	/**
	 * Sender or recipient number.
	 */
	unsigned char   Number;
	/**
	 * Array of SMSes.
	 */
	GSM_SMSMessage  SMS[MAX_MULTI_SMS];
} GSM_MultiSMSMessage;

GSM_Error GSM_AddSMS_Text_UDH(GSM_MultiSMSMessage       *SMS,
				GSM_Coding_Type	 	Coding,
				char		    	*Buffer,
				int		     	BufferLen,
				bool		    	UDH,
				int		     	*UsedText,
				int		     	*CopiedText,
				int		     	*CopiedSMSText);

void GSM_MakeMultiPartSMS(GSM_MultiSMSMessage   *SMS,
			  unsigned char	 	*MessageBuffer,
			  int		   	MessageLength,
			  GSM_UDH	       	UDHType,
			  GSM_Coding_Type       Coding,
			  int		   	Class,
			  unsigned char	 	RejectDuplicates);

void GSM_Find_Free_Used_SMS2(GSM_Coding_Type Coding,GSM_SMSMessage SMS, int *UsedText, int *FreeText, int *FreeBytes);

unsigned char GSM_MakeSMSIDFromTime(void);

/**
 * ID during packing SMS for Smart Messaging 3.0, EMS and other
 */
typedef enum {
	/**
	 * 1 text SMS.
	 */
	SMS_Text = 1,
	/**
	 * Contacenated SMS, when longer than 1 SMS.
	 */
	SMS_ConcatenatedTextLong,
	/**
	 * Contacenated SMS, auto Default/Unicode coding.
	 */
	SMS_ConcatenatedAutoTextLong,
	SMS_ConcatenatedTextLong16bit,
	SMS_ConcatenatedAutoTextLong16bit,
	/**
	 * Nokia profile = Name, Ringtone, ScreenSaver
	 */
	SMS_NokiaProfileLong,
	/**
	 * Nokia Picture Image + (text)
	 */
	SMS_NokiaPictureImageLong,
	/**
	 * Nokia screen saver + (text)
	 */
	SMS_NokiaScreenSaverLong,
	/**
	 * Nokia ringtone - old SM2.0 format, 1 SMS
	 */
	SMS_NokiaRingtone,
	/**
	 * Nokia ringtone contacenated, when very long
	 */
	SMS_NokiaRingtoneLong,
	/**
	 * Nokia 72x14 operator logo, 1 SMS
	 */
	SMS_NokiaOperatorLogo,
	/**
	 * Nokia 72x14 op logo or 78x21 in 2 SMS
	 */
	SMS_NokiaOperatorLogoLong,
	/**
	 * Nokia 72x14 caller logo, 1 SMS
	 */
	SMS_NokiaCallerLogo,
	/**
	 * Nokia WAP bookmark in 1 or 2 SMS
	 */
	SMS_NokiaWAPBookmarkLong,
	/**
	 * Nokia WAP settings in 2 SMS
	 */
	SMS_NokiaWAPSettingsLong,
	/**
	 * Nokia MMS settings in 2 SMS
	 */
	SMS_NokiaMMSSettingsLong,
	/**
	 * Nokia VCARD 1.0 - only name and default number
	 */
	SMS_NokiaVCARD10Long,
	/**
	 * Nokia VCARD 2.1 - all numbers + text
	 */
	SMS_NokiaVCARD21Long,
	/**
	 * Nokia VCALENDAR 1.0 - can be in few sms
	 */
	SMS_NokiaVCALENDAR10Long,
	SMS_NokiaVTODOLong,
	SMS_VCARD10Long,
	SMS_VCARD21Long,
	SMS_DisableVoice,
	SMS_DisableFax,
	SMS_DisableEmail,
	SMS_EnableVoice,
	SMS_EnableFax,
	SMS_EnableEmail,
	SMS_VoidSMS,
	/**
	 * IMelody 1.0
	 */
	SMS_EMSSound10,
	/**
	 * IMelody 1.2
	 */
	SMS_EMSSound12,
	/**
	 * IMelody without header - SonyEricsson extension
	 */
	SMS_EMSSonyEricssonSound,
	/**
	 * IMelody 1.0 with UPI.
	 */
	SMS_EMSSound10Long,
	/**
	 * IMelody 1.2 with UPI.
	 */
	SMS_EMSSound12Long,
	/**
	 * IMelody without header with UPI.
	 */
	SMS_EMSSonyEricssonSoundLong,
	SMS_EMSPredefinedSound,
	SMS_EMSPredefinedAnimation,
	SMS_EMSAnimation,
	/**
	 * Fixed bitmap of size 16x16 or 32x32.
	 */
	SMS_EMSFixedBitmap,
	SMS_EMSVariableBitmap,
	SMS_EMSVariableBitmapLong,
	SMS_MMSIndicatorLong,
	SMS_WAPIndicatorLong,
	/**
	 * Variable bitmap with black and white colors
	 */
	SMS_AlcatelMonoBitmapLong,
	/**
	 * Variable animation with black and white colors
	 */
	SMS_AlcatelMonoAnimationLong,
	SMS_AlcatelSMSTemplateName
} EncodeMultiPartSMSID;

typedef struct {
	EncodeMultiPartSMSID    ID;

	int		     	Number;
	GSM_Ringtone	    	*Ringtone;
	GSM_MultiBitmap	 	*Bitmap;
	GSM_WAPBookmark	 	*Bookmark;
	GSM_WAPSettings	 	*Settings;
	GSM_MMSIndicator	*MMSIndicator;
	GSM_MemoryEntry 	*Phonebook;
	GSM_CalendarEntry       *Calendar;
	GSM_ToDoEntry	   	*ToDo;
	bool		    	Protected;

	unsigned char	   	*Buffer;
	bool		    	Left;
	bool		    	Right;
	bool		    	Center;
	bool		    	Large;
	bool		    	Small;
	bool		    	Bold;
	bool		    	Italic;
	bool		    	Underlined;
	bool		    	Strikethrough;

	/* Return values */
	int		     	RingtoneNotes;
} MultiPartSMSEntry;

typedef struct {
	MultiPartSMSEntry	Entries[MAX_MULTI_SMS];
	int			EntriesNum;
	bool			UnicodeCoding;
	int			Class;
	unsigned char		ReplaceMessage;
	bool			Unknown;
} GSM_MultiPartSMSInfo;

/**
 * Encodes multi part SMS from "readable" format.
 */
GSM_Error GSM_EncodeMultiPartSMS (GSM_MultiPartSMSInfo *Info, GSM_MultiSMSMessage *SMS);

/**
 * Decodes multi part SMS to "readable" format.
 */
bool GSM_DecodeMultiPartSMS (GSM_MultiPartSMSInfo *Info, GSM_MultiSMSMessage *SMS, bool ems);

/**
 * Clears @ref GSM_MultiPartSMSInfo to default values.
 */
void GSM_ClearMultiPartSMSInfo (GSM_MultiPartSMSInfo *Info);

/**
 * Frees any allocated structures inside @ref GSM_MultiPartSMSInfo.
 */
void GSM_FreeMultiPartSMSInfo (GSM_MultiPartSMSInfo *Info);

/**
 * Links SMS messages according to IDs.
 */
GSM_Error GSM_LinkSMS(GSM_MultiSMSMessage **INPUT, GSM_MultiSMSMessage **OUTPUT, bool ems);

#if defined(_MSC_VER) && defined(__cplusplus)
    }
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
