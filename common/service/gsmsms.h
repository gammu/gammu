
#ifndef __gsm_sms_h
#define __gsm_sms_h

#include "../gsmcomon.h"
#include "gsmlogo.h"
#include "gsmcal.h"
#include "gsmpbk.h"
#include "gsmwap.h"
#include "gsmring.h"

/* --------------------- Some general definitions ------------------------- */

#define GSM_MAX_UDH_LENGTH		140
#define GSM_MAX_SMS_LENGTH		160
#define GSM_MAX_8BIT_SMS_LENGTH	 	140

/* -------------------- Cell Broadcast ------------------------------------ */

/**
 * Define datatype for Cell Broadcast message.
 */
typedef struct {
	/**
	 * Channel number.
	 */
	int     Channel;
	/**
	 * Message text.
	 */
	char    Text[300];
} GSM_CBMessage;

/* ------------------------ SMS status ------------------------------------ */

/**
 * Status of SMS memory.
 */
typedef struct {
	/**
	 * Number of unread messages in SIM.
	 */
	int     SIMUnRead;
	/**
	 * Number of all used messages in SIM.
	 */
	int     SIMUsed;
	/**
	 * Number of all possible messafes on SIM.
	 */
	int     SIMSize;
	/**
	 * Number of unread messages in phone.
	 */
	int     PhoneUnRead;
	/**
	 * Number of all used messages in phone.
	 */
	int     PhoneUsed;
	/**
	 * Number of all possible messages on phone.
	 */
	int     PhoneSize;
	/**
	 * Number of used templates (62xx/63xx/7110/etc.).
	 */
	int     TemplatesUsed;
} GSM_SMSMemoryStatus;

/* --------------------- SMS Center --------------------------------------- */

/**
 * SMS Messages sent as...
 */
typedef enum {
	GSMF_Pager = 1,
	GSMF_Fax,
	GSMF_Email,
	GSMF_Text
} GSM_SMSFormat;

/**
 * Length of validity for SMS messages.
 */
typedef enum {
	GSMV_1_Hour   = 0x0b,
	GSMV_6_Hours  = 0x47,
	GSMV_24_Hours = 0xa7,
	GSMV_72_Hours = 0xa9,
	GSMV_1_Week   = 0xad,
	GSMV_Max_Time = 0xff
} GSM_ValidityPeriod;

/**
 * Format of validity perion.
 */
typedef enum {
	GSM_NoValidityPeriod = 1,
	GSM_RelativeFormat
	/* ... */
} GSM_ValidityPeriodFormat;

/**
 * Validity of SMS message.
 */
typedef struct {
	GSM_ValidityPeriodFormat	VPF;
	GSM_ValidityPeriod	      	Relative;
} GSM_SMSValidity;

#define GSM_MAX_SMSC_NAME_LENGTH	30

/**
 * Structure for SMSC (SMS Center) information.
 */
typedef struct {
	/**
	 * Number of the SMSC in the phone memory.
	 */
	int	     	Location;
	/**
	 * Name of the SMSC.
	 */
	unsigned char   Name[(GSM_MAX_SMSC_NAME_LENGTH+1)*2];
	/**
	 * Format of sent SMSes.
	 */
	GSM_SMSFormat   Format;
	/**
	 * Validity of SMS.
	 */
	GSM_SMSValidity Validity;
	/**
	 * SMSC number.
	 */
	unsigned char   Number[(GSM_MAX_NUMBER_LENGTH+1)*2];
	/**
	 * Default recipient number.
	 */
	unsigned char   DefaultNumber[(GSM_MAX_NUMBER_LENGTH+1)*2];
} GSM_SMSC;

/* --------------------- single SMS --------------------------------------- */

/**
 * Status of SMS message.
 */
typedef enum {
	GSM_Sent = 1,
	GSM_UnSent,
	GSM_Read,
	GSM_UnRead
} GSM_SMS_State;

/**
 * Coding type of SMS.
 */
typedef enum {
	/**
	 * Unicode
	 */
	GSM_Coding_Unicode = 1,
	/**
	 * Default GSM aplhabet.
	 */
	GSM_Coding_Default,
	/**
	 * 8-bit.
	 */
	GSM_Coding_8bit
} GSM_Coding_Type;

/**
 * Types of UDH (User Data Header).
 */
typedef enum {
	UDH_NoUDH = 1,
	/**
	 * Linked SMS.
	 */
	UDH_ConcatenatedMessages,
	/**
	 * Linked SMS with 16 bit reference.
	 */
	UDH_ConcatenatedMessages16bit,
	UDH_DisableVoice,
	UDH_DisableFax,
	UDH_DisableEmail,
	UDH_EnableVoice,
	UDH_EnableFax,
	UDH_EnableEmail,
	UDH_VoidSMS,
	UDH_NokiaRingtone,
	UDH_NokiaRingtoneLong,
	UDH_NokiaOperatorLogo,
	UDH_NokiaOperatorLogoLong,
	UDH_NokiaCallerLogo,
	UDH_NokiaWAP,
	UDH_NokiaWAPLong,
	UDH_NokiaCalendarLong,
	UDH_NokiaProfileLong,
	UDH_NokiaPhonebookLong,
	UDH_UserUDH,
	UDH_MMSIndicatorLong
} GSM_UDH;

/**
 * Structure to hold UDH Header.
 */
typedef struct {
	/**
	 * UDH type.
	 */
	GSM_UDH	 	Type;
	/**
	 * UDH length.
	 */
	int	     	Length;
	/**
	 * UDH text.
	 */
	unsigned char   Text[GSM_MAX_UDH_LENGTH];
	/**
	 * 8-bit ID, when required (-1 otherwise).
	 */
	int	     	ID8bit;
	/**
	 * 16-bit ID, when required (-1 otherwise).
	 */
	int	     	ID16bit;
	/**
	 * Number of current part.
	 */
	int	     	PartNumber;
	/**
	 * Total number of parts.
	 */
	int	     	AllParts;
} GSM_UDHHeader;

/**
 * TP-Message-Type-Indicator, see GSM 03.40 version 6.1.0 Release 1997 Section 9.2.3.1.
 */
typedef enum {		 
	/**
	 * Save SMS in Inbox.
	 */
	SMS_Deliver = 1,
	/**
	 * Delivery Report received by phone.
	 */
	SMS_Status_Report,
	/**
	 * Send SMS or save it in Outbox.
	 */
	SMS_Submit
	/*...*/
} GSM_SMSMessageType;

/**
 * Maximal length of SMS name.
 */
#define GSM_MAX_SMS_NAME_LENGTH	 40

/**
 * SMS message data.
 */
typedef struct {
	/**
	 * Message center.
	 */
	GSM_SMSC		SMSC;
	/**
	 * UDH (User Defined Header).
	 */
	GSM_UDHHeader	   	UDH;
	/**
	 * Folder, where SMS is saved.
	 */
	int		     	Folder;
	/**
	 * Whether it is really inbox.
	 */
	bool		    	InboxFolder;
	/**
	 * Where exactly it's saved
	 */
	GSM_MemoryType		Memory;
	/**
	 * Location of SMS in memory.
	 */
	int		     	Location;
	/**
	 * Length of the SMS message.
	 */
	int		     	Length;
	/**
	 * Name in Nokia 6210/7110, etc. Ignored in other.
	 */
	unsigned char	   	Name[(GSM_MAX_SMS_NAME_LENGTH+1)*2];
	/**
	 * Sender or recipient number.
	 */
	unsigned char	   	Number[(GSM_MAX_NUMBER_LENGTH+1)*2];
	/**
	 * Text for SMS.
	 */
	unsigned char	   	Text[(GSM_MAX_SMS_LENGTH+1)*2];
	/**
	 * Type of message.
	 */
	GSM_SMSMessageType      PDU;
	/**
	 * Type of coding.
	 */
	GSM_Coding_Type	 	Coding;
	/**
	 * Date of reception/response of messages.
	 */
	GSM_DateTime	    	DateTime;
	/**
	 * Date of SMSC response in DeliveryReport messages.
	 */
	GSM_DateTime	    	SMSCTime;
	/**
	 * In delivery reports: status.
	 */
	unsigned char	   	DeliveryStatus;
	/**
	 * Indicates whether "Reply via same center" is set.
	 */
	bool		    	ReplyViaSameSMSC;
	/**                 	
	 * State (read/unread/...) of SMS.
	 */
	GSM_SMS_State	   	State;
	/**
	 * SMS class.
	 */
	char		    	Class;
	/**
	 * Message reference.
	 */
	unsigned char	   	MessageReference;
	/**
	 * Message to be replaced.
	 */
	unsigned char	   	ReplaceMessage;
	/**
	 * Whether to reject duplicates.
	 */
	bool		    	RejectDuplicates;
} GSM_SMSMessage;

/* In layouts are saved locations for some SMS part. Below are listed
 * specs, which describe them
 */
typedef struct {
	/**
	 * TP-User-Data. GSM 03.40 section 9.2.3.24. 
	 */
	unsigned char Text;
	/**
	 * - In SMS-Deliver:       TP-Originating-Address. GSM 03.40 section 9.2.3.7.
	 * - In SMS-Submit:        TP-Destination-Address. GSM 03.40 section 9.2.3.8.
	 * - In SMS-Status-Report: TP-Recipient-Address.   GSM 03.40 section 9.2.3.14.
	 */
	unsigned char Number;
	/**
	 * SMSC number
	 */
	unsigned char SMSCNumber;
	/**
	 * TP-Data-Coding-Scheme. GSM 03.40 section 9.2.3.10
	 */
	unsigned char TPDCS;
	/**
	 * - For SMS-Submit:        TP-Validity-Period. GSM 03.40 section 9.2.3.12.
	 * - For SMS-Status-Report: TP-Discharge Time.  GSM 03.40 section 9.2.3.13.
	 */
	unsigned char DateTime;
	/**
	 * TP-Service-Centre-Time-Stamp in SMS-Status-Report. GSM 03.40 section 9.2.3.11.
	 */
	unsigned char SMSCTime;
	/**
	 * TP-Status in SMS-Status-Report. GSM 03.40 section 9.2.3.15.
	 */
	unsigned char TPStatus;
	/**
	 * TP-User-Data-Length. GSM 03.40 section 9.2.3.16.
	 */
	unsigned char TPUDL;
	/**
	 * TP-Validity Period in SMS-Submit. GSM 03.40 section 9.2.3.12.
	 */
	unsigned char TPVP;
	/**
	 * Byte contains in SMS-Deliver:
	 *   - TP-Message-Type-Indicator     (2 bits) GSM 03.40 section 9.2.3.1
	 *   - TP-More-Messages-To-Send      (1 bit). GSM 03.40 section 9.2.3.2
	 *   - TP-Reply-Path                 (1 bit). GSM 03.40 section 9.2.3.17
	 *   - TP-User-Data-Header-Indicator (1 bit). GSM 03.40 section 9.2.3.23
	 *   - TP-Status-Report-Indicator    (1 bit). GSM 03.40 section 9.2.3.4
	 *
	 * Byte contains in SMS-Submit:
	 *   - TP-Message-Type-Indicator     (2 bits) GSM 03.40 section 9.2.3.1
	 *   - TP-Reject-Duplicates          (1 bit). GSM 03.40 section
	 *   - TP-Validity-Period-Format     (2 bits).GSM 03.40 section 9.2.3.3
	 *   - TP-Reply-Path                 (1 bit). GSM 03.40 section 9.2.3.17
	 *   - TP-User-Data-Header-Indicator (1 bit). GSM 03.40 section 9.2.3.23
	 *   - TP-Status-Report-Request      (1 bit). GSM 03.40 section 9.2.3.5
	 */
	unsigned char firstbyte;
	/**
	 * TP-Message Reference in SMS-Submit. GSM 03.40 section 9.2.3.6
	 */
	unsigned char TPMR;
	/**
	 * TP-Protocol-Identifier. GSM 03.40 section 9.2.3.9
	 */
	unsigned char TPPID;
} GSM_SMSMessageLayout;

GSM_Error GSM_DecodeSMSFrame(GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout);
GSM_Error GSM_EncodeSMSFrame(GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout, int *length, bool clear);

GSM_Error GSM_DecodeSMSFrameStatusReportData    (GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout);
GSM_Error GSM_DecodeSMSFrameText		(GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout);

void GSM_DecodeUDHHeader(GSM_UDHHeader *UDH);
void GSM_EncodeUDHHeader(GSM_UDHHeader *UDH);

void GSM_SetDefaultSMSData(GSM_SMSMessage *SMS);

/* ---------------------- SMS folders ------------------------------------- */

/**
 * Maximum number of SMS folders.
 */
#define GSM_MAX_SMS_FOLDERS	     	24
/**
 * Maximal length of SMS folder name.
 */
#define GSM_MAX_SMS_FOLDER_NAME_LEN     16

/**
 * Information about SMS folder.
 */
typedef struct {
	/**
	 * Whether it is really inbox.
	 */
	bool		    	InboxFolder;
	/**
	 * Where exactly it's saved
	 */
	GSM_MemoryType		Memory;
        /**
  	 * Name for SMS folder.
	 */
        char	    Name[(GSM_MAX_SMS_FOLDER_NAME_LEN+1)*2];
} GSM_OneSMSFolder;

/**
 * List of SMS folders.
 */
typedef struct {
        /**
	 * Array of structures holding information about each folder.
	 */
        GSM_OneSMSFolder	Folder[GSM_MAX_SMS_FOLDERS];
        /**
 	 * Number of SMS folders.
	 */
        unsigned char	   	Number;
} GSM_SMSFolders;

/* ---------------------- multi SMS --------------------------------------- */

/* Identifiers for Smart Messaging 3.0 multipart SMS */

#define SM30_ISOTEXT      0 /* ISO 8859-1 text */
#define SM30_UNICODETEXT  1
#define SM30_OTA	  2
#define SM30_RINGTONE     3
#define SM30_PROFILENAME  4
#define SM30_SCREENSAVER  6

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
	/***
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
	SMS_MMSIndicatorLong
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
} EncodeMultiPartSMSEntry;

typedef struct {
	/* Input values */
	EncodeMultiPartSMSEntry Entries[MAX_MULTI_SMS];
	int		     	EntriesNum;
	bool		    	UnicodeCoding;
	int		     	Class;
	unsigned char	   	ReplaceMessage;
} GSM_EncodeMultiPartSMSInfo;

GSM_Error GSM_EncodeMultiPartSMS	(GSM_EncodeMultiPartSMSInfo *Info, GSM_MultiSMSMessage *SMS);
bool      GSM_DecodeMultiPartSMS	(GSM_EncodeMultiPartSMSInfo *Info, GSM_MultiSMSMessage *SMS);
void      GSM_ClearMultiPartSMSInfo     (GSM_EncodeMultiPartSMSInfo *Info);

GSM_Error GSM_SortSMS(GSM_MultiSMSMessage *INPUT[200], GSM_MultiSMSMessage *OUTPUT[200]);

#endif  /* __gsm_sms_h */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
