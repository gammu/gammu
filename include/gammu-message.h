/**
 * \file gammu-message.h
 * \author Michal Čihař
 * 
 * Message data and functions.
 */
#ifndef __gammu_message_h
#define __gammu_message_h

#include <gammu-limits.h>
#include <gammu-memory.h>
#include <gammu-datetime.h>
#include <gammu-ringtone.h>
#include <gammu-bitmap.h>
#include <gammu-wap.h>
#include <gammu-calendar.h>
#include <gammu-file.h>

/**
 * \defgroup Message Message
 * Messages manipulations.
 */

typedef struct {
	char Address[500];
	char Title[200];
	char Sender[200];
} GSM_MMSIndicator;

/* --------------------- Some general definitions ------------------------- */

#define GSM_MAX_UDH_LENGTH		140
#define GSM_MAX_SMS_LENGTH		160
#define GSM_MAX_8BIT_SMS_LENGTH	 	140

/* -------------------- Cell Broadcast ------------------------------------ */

/**
 * Structure for Cell Broadcast messages.
 */
typedef struct {
	/**
	 * Message text.
	 */
	char Text[300];
	/**
	 * Channel number.
	 */
	int Channel;
} GSM_CBMessage;

/* -------------------- USSD ------------------------------------ */

/**
 * Status of USSD message.
 */
typedef enum {
	USSD_Unknown = 1, /**< Unknown status */
	USSD_NoActionNeeded, /**< No action is needed, maybe network initiated USSD */
	USSD_ActionNeeded, /**< Reply is expected */
	USSD_Terminated, /**< USSD dialog terminated */
	USSD_AnotherClient, /**< Another client replied */
	USSD_NotSupported, /**< Operation not supported */
	USSD_Timeout, /**< Network timeout */
} GSM_USSDStatus;

/**
 * Structure for USSD messages.
 */
typedef struct {
	/**
	 * Message text.
	 */
	unsigned char Text[2000];
	/**
	 * Message status.
	 */
	GSM_USSDStatus Status;
} GSM_USSDMessage;

/* ------------------------ SMS status ------------------------------------ */

/**
 * Status of SMS memory.
 */
typedef struct {
	/**
	 * Number of unread messages on SIM.
	 */
	int SIMUnRead;
	/**
	 * Number of all saved messages (including unread) on SIM.
	 */
	int SIMUsed;
	/**
	 * Number of all possible messages on SIM.
	 */
	int SIMSize;
	/**
	 * Number of used templates (62xx/63xx/7110/etc.).
	 */
	int TemplatesUsed;
	/**
	 * Number of unread messages in phone.
	 */
	int PhoneUnRead;
	/**
	 * Number of all saved messages in phone.
	 */
	int PhoneUsed;
	/**
	 * Number of all possible messages on phone.
	 */
	int PhoneSize;
} GSM_SMSMemoryStatus;

/* --------------------- SMS Center --------------------------------------- */

/**
 * Enum defines format of SMS messages. See GSM 03.40 section 9.2.3.9
 */
typedef enum {
	SMS_FORMAT_Pager = 1,
	SMS_FORMAT_Fax,
	SMS_FORMAT_Email,
	SMS_FORMAT_Text
	    /* Some values not handled here */
} GSM_SMSFormat;

/**
 * Enum defines some the most often used validity lengths for SMS messages
 * for relative validity format. See GSM 03.40 section 9.2.3.12.1 - it gives
 * more values
 */
typedef enum {
	SMS_VALID_1_Hour = 0x0b,
	SMS_VALID_6_Hours = 0x47,
	SMS_VALID_1_Day = 0xa7,
	SMS_VALID_3_Days = 0xa9,
	SMS_VALID_1_Week = 0xad,
	SMS_VALID_Max_Time = 0xff
} GSM_ValidityPeriod;

/**
 * Enum defines format of validity period for SMS messages.
 * See GSM 03.40 section 9.2.3.12
 */
typedef enum {
	SMS_Validity_NotAvailable = 1,
	SMS_Validity_RelativeFormat
	    /* Specification gives also other possibilities */
} GSM_ValidityPeriodFormat;

/**
 * Structure for validity of SMS messages
 */
typedef struct {
	GSM_ValidityPeriodFormat Format;
	/**
	 * Value defines period for relative format
	 */
	GSM_ValidityPeriod Relative;
} GSM_SMSValidity;

#define GSM_MAX_SMSC_NAME_LENGTH	50

/**
 * Structure for SMSC (SMS Center) information.
 */
typedef struct {
	/**
	 * Number of the SMSC on SIM
	 */
	int Location;
	/**
	 * Name of the SMSC
	 */
	unsigned char Name[(GSM_MAX_SMSC_NAME_LENGTH + 1) * 2];
	/**
	 * SMSC phone number.
	 */
	unsigned char Number[(GSM_MAX_NUMBER_LENGTH + 1) * 2];
	/**
	 * Validity of SMS messages.
	 */
	GSM_SMSValidity Validity;
	/**
	 * Format of sent SMS messages.
	 */
	GSM_SMSFormat Format;
	/**
	 * Default recipient number. In old DCT3 ignored
	 */
	unsigned char DefaultNumber[(GSM_MAX_NUMBER_LENGTH + 1) * 2];
} GSM_SMSC;

/* --------------------- single SMS --------------------------------------- */

/**
 * Status of SMS message.
 */
typedef enum {
	SMS_Sent = 1,
	SMS_UnSent,
	SMS_Read,
	SMS_UnRead
} GSM_SMS_State;

/**
 * Coding type of SMS.
 */
typedef enum {
	/**
	 * Unicode
	 */
	SMS_Coding_Unicode_No_Compression = 1,
	SMS_Coding_Unicode_Compression,
	/**
	 * Default GSM alphabet.
	 */
	SMS_Coding_Default_No_Compression,
	SMS_Coding_Default_Compression,
	/**
	 * 8-bit.
	 */
	SMS_Coding_8bit
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
 * Structure for User Data Header.
 */
typedef struct {
	/**
	 * UDH type.
	 */
	GSM_UDH Type;
	/**
	 * UDH length.
	 */
	int Length;
	/**
	 * UDH text.
	 */
	unsigned char Text[GSM_MAX_UDH_LENGTH];
	/**
	 * 8-bit ID, when required (-1 otherwise).
	 */
	int ID8bit;
	/**
	 * 16-bit ID, when required (-1 otherwise).
	 */
	int ID16bit;
	/**
	 * Number of current part.
	 */
	int PartNumber;
	/**
	 * Total number of parts.
	 */
	int AllParts;
} GSM_UDHHeader;

/**
 * TP-Message-Type-Indicator. See GSM 03.40 section 9.2.3.1.
 */
typedef enum {
	/**
	 * SMS in Inbox.
	 */
	SMS_Deliver = 1,
	/**
	 * Delivery Report
	 */
	SMS_Status_Report,
	/**
	 * SMS for sending or in Outbox
	 */
	SMS_Submit
	    /* specification gives more */
} GSM_SMSMessageType;

/**
 * Maximal length of SMS name.
 */
#define GSM_MAX_SMS_NAME_LENGTH	 200

#define GSM_SMS_OTHER_NUMBERS 5

/**
 * SMS message data.
 */
typedef struct {
	/**
	 * Message to be replaced.
	 */
	unsigned char ReplaceMessage;
	/**
	 * Whether to reject duplicates.
	 */
	bool RejectDuplicates;
	/**
	 * UDH (User Data Header)
	 */
	GSM_UDHHeader UDH;
	/**
	 * Sender or recipient number.
	 */
	unsigned char Number[(GSM_MAX_NUMBER_LENGTH + 1) * 2];

	unsigned char
	    OtherNumbers[GSM_SMS_OTHER_NUMBERS][(GSM_MAX_NUMBER_LENGTH +
						 1) * 2];
	int OtherNumbersNum;

	/**
	 * SMSC (SMS Center)
	 */
	GSM_SMSC SMSC;
	/**
	 * For saved SMS: where exactly it's saved (SIM/phone)
	 */
	GSM_MemoryType Memory;
	/**
	 * For saved SMS: location of SMS in memory.
	 */
	int Location;
	/**
	 * For saved SMS: number of folder, where SMS is saved
	 */
	int Folder;
	/**
	 * For saved SMS: whether SMS is really in Inbox.
	 */
	bool InboxFolder;
	/**
	 * Length of the SMS message.
	 */
	int Length;
	/**
	 * Status (read/unread/...) of SMS message.
	 */
	GSM_SMS_State State;
	/**
	 * Name in Nokia with SMS memory (6210/7110, etc.) Ignored in other.
	 */
	unsigned char Name[(GSM_MAX_SMS_NAME_LENGTH + 1) * 2];
	/**
	 * Text for SMS.
	 */
	unsigned char Text[(GSM_MAX_SMS_LENGTH + 1) * 2];
	/**
	 * Type of message.
	 */
	GSM_SMSMessageType PDU;
	/**
	 * Type of coding.
	 */
	GSM_Coding_Type Coding;
	/**
	 * Date and time, when SMS was saved or sent
	 */
	GSM_DateTime DateTime;
	/**
	 * Date of SMSC response in DeliveryReport messages.
	 */
	GSM_DateTime SMSCTime;
	/**
	 * In delivery reports: status.
	 */
	unsigned char DeliveryStatus;
	/**
	 * Indicates whether "Reply via same center" is set.
	 */
	bool ReplyViaSameSMSC;
	/**
	 * SMS class.
	 */
	signed char Class;
	/**
	 * Message reference.
	 */
	unsigned char MessageReference;
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
	 * TP-Data-Coding-Scheme. GSM 03.40 section 9.2.3.10.
	 * Contains alphabet type, SMS class (and some others)
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
	 * - TP-Message-Type-Indicator     (2 bits) GSM 03.40 section 9.2.3.1
	 * - TP-More-Messages-To-Send      (1 bit). GSM 03.40 section 9.2.3.2
	 * - TP-Reply-Path                 (1 bit). GSM 03.40 section 9.2.3.17
	 * - TP-User-Data-Header-Indicator (1 bit). GSM 03.40 section 9.2.3.23
	 * - TP-Status-Report-Indicator    (1 bit). GSM 03.40 section 9.2.3.4
	 *
	 * Byte contains in SMS-Submit:
	 * - TP-Message-Type-Indicator     (2 bits) GSM 03.40 section 9.2.3.1
	 * - TP-Reject-Duplicates          (1 bit). GSM 03.40 section
	 * - TP-Validity-Period-Format     (2 bits).GSM 03.40 section 9.2.3.3
	 * - TP-Reply-Path                 (1 bit). GSM 03.40 section 9.2.3.17
	 * - TP-User-Data-Header-Indicator (1 bit). GSM 03.40 section 9.2.3.23
	 * - TP-Status-Report-Request      (1 bit). GSM 03.40 section 9.2.3.5
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

GSM_Error GSM_DecodeSMSFrame(GSM_SMSMessage * SMS, unsigned char *buffer,
			     GSM_SMSMessageLayout Layout);
GSM_Error GSM_EncodeSMSFrame(GSM_SMSMessage * SMS, unsigned char *buffer,
			     GSM_SMSMessageLayout Layout, int *length,
			     bool clear);

GSM_Error GSM_DecodeSMSFrameStatusReportData(GSM_SMSMessage * SMS,
					     unsigned char *buffer,
					     GSM_SMSMessageLayout Layout);
GSM_Error GSM_DecodeSMSFrameText(GSM_SMSMessage * SMS, unsigned char *buffer,
				 GSM_SMSMessageLayout Layout);

void GSM_DecodeUDHHeader(GSM_UDHHeader * UDH);
void GSM_EncodeUDHHeader(GSM_UDHHeader * UDH);

void GSM_SetDefaultSMSData(GSM_SMSMessage * SMS);

/* ---------------------- SMS folders ------------------------------------- */

/**
 * Number of possible SMS folders.
 */
#define GSM_MAX_SMS_FOLDERS	     	50
/**
 * Maximal length of SMS folder name.
 */
#define GSM_MAX_SMS_FOLDER_NAME_LEN     20

/**
 * Information about SMS folder.
 */
typedef struct {
	bool InboxFolder;		     /**< Whether it is inbox. */
	bool OutboxFolder;		      /**< Whether it is outbox. */
	GSM_MemoryType Memory;		/**< Where exactly it's saved. */
	unsigned char Name[(GSM_MAX_SMS_FOLDER_NAME_LEN + 1) * 2];	 /**< Name of the folder */
} GSM_OneSMSFolder;

/**
 * List of SMS folders.
 */
typedef struct {
	/**
	 * Array of structures holding information about each folder.
	 */
	GSM_OneSMSFolder Folder[GSM_MAX_SMS_FOLDERS];
	/**
 	 * Number of SMS folders.
	 */
	unsigned char Number;
} GSM_SMSFolders;

typedef struct {
	unsigned long SequenceID;
	unsigned int PacketsNum;
	unsigned int PacketNum;
	unsigned long AllDataLen;

	unsigned char DataType[10];
	unsigned char DataName[40];
	unsigned int DataLen;
	unsigned char Data[140];
} GSM_SiemensOTASMSInfo;

bool GSM_DecodeSiemensOTASMS(GSM_SiemensOTASMSInfo * Info,
			     GSM_SMSMessage * SMS);

#define MAX_MULTI_SMS 10

/**
 * Multiple SMS messages, used for Smart Messaging 3.0/EMS.
 */
typedef struct {
	/**
	 * Sender or recipient number.
	 */
	unsigned char Number;
	/**
	 * Array of SMSes.
	 */
	GSM_SMSMessage SMS[MAX_MULTI_SMS];
} GSM_MultiSMSMessage;

/**
 * Number of possible MMS folders.
 */
#define GSM_MAX_MMS_FOLDERS	     	10
/**
 * Maximal length of MMS folder name.
 */
#define GSM_MAX_MMS_FOLDER_NAME_LEN     20

/**
 * Information about MMS folder.
 */
typedef struct {
	/**
	 * Whether it is really inbox.
	 */
	bool InboxFolder;
	/**
  	 * Name for MMS folder.
	 */
	char Name[(GSM_MAX_MMS_FOLDER_NAME_LEN + 1) * 2];
} GSM_OneMMSFolder;

/**
 * List of MMS folders.
 */
typedef struct {
	/**
	 * Array of structures holding information about each folder.
	 */
	GSM_OneMMSFolder Folder[GSM_MAX_MMS_FOLDERS];
	/**
 	 * Number of MMS folders.
	 */
	unsigned char Number;
} GSM_MMSFolders;

extern GSM_SMSMessageLayout PHONE_SMSSubmit;
extern GSM_SMSMessageLayout PHONE_SMSDeliver;
extern GSM_SMSMessageLayout PHONE_SMSStatusReport;

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
	SMS_AlcatelSMSTemplateName,
	/**
	 * Siemens OTA
	 */
	SMS_SiemensFile
} EncodeMultiPartSMSID;

typedef struct {
	EncodeMultiPartSMSID ID;

	int Number;
	GSM_Ringtone *Ringtone;
	GSM_MultiBitmap *Bitmap;
	GSM_WAPBookmark *Bookmark;
	GSM_WAPSettings *Settings;
	GSM_MMSIndicator *MMSIndicator;
	GSM_MemoryEntry *Phonebook;
	GSM_CalendarEntry *Calendar;
	GSM_ToDoEntry *ToDo;
	GSM_File *File;
	bool Protected;

	unsigned char *Buffer;
	bool Left;
	bool Right;
	bool Center;
	bool Large;
	bool Small;
	bool Bold;
	bool Italic;
	bool Underlined;
	bool Strikethrough;

	/* Return values */
	int RingtoneNotes;
} MultiPartSMSEntry;

typedef struct {
	MultiPartSMSEntry Entries[MAX_MULTI_SMS];
	int EntriesNum;
	bool UnicodeCoding;
	int Class;
	unsigned char ReplaceMessage;
	bool Unknown;
} GSM_MultiPartSMSInfo;

/**
 * Encodes SMS frame according to layout.
 */
GSM_Error PHONE_EncodeSMSFrame(GSM_StateMachine * s, GSM_SMSMessage * SMS,
			       unsigned char *buffer,
			       GSM_SMSMessageLayout Layout, int *length,
			       bool clear);

/**
 * Encodes multi part SMS from "readable" format.
 */
GSM_Error GSM_EncodeMultiPartSMS(GSM_MultiPartSMSInfo * Info,
				 GSM_MultiSMSMessage * SMS);

/**
 * Decodes multi part SMS to "readable" format.
 */
bool GSM_DecodeMultiPartSMS(GSM_MultiPartSMSInfo * Info,
			    GSM_MultiSMSMessage * SMS, bool ems);

/**
 * Clears @ref GSM_MultiPartSMSInfo to default values.
 */
void GSM_ClearMultiPartSMSInfo(GSM_MultiPartSMSInfo * Info);

/**
 * Frees any allocated structures inside @ref GSM_MultiPartSMSInfo.
 */
void GSM_FreeMultiPartSMSInfo(GSM_MultiPartSMSInfo * Info);

/**
 * Links SMS messages according to IDs.
 */
GSM_Error GSM_LinkSMS(GSM_MultiSMSMessage ** INPUT,
		      GSM_MultiSMSMessage ** OUTPUT, bool ems);

#define MAX_MULTI_MMS 20

typedef enum {
	MMSADDRESS_PHONE,
	MMSADDRESS_UNKNOWN
} MMSAddressType;

typedef struct {
	GSM_File File;
	unsigned char ContentType[400];	// CT in Unicode
	unsigned char SMIL[400];	// Smil ID in Unicode
} EncodedMultiPartMMSEntry2;

typedef struct {
	/* Subparts */
	EncodedMultiPartMMSEntry2 Entries[MAX_MULTI_MMS];
	int EntriesNum;

	unsigned char Source[200];	// in Unicode
	MMSAddressType SourceType;
	unsigned char Destination[200];	// in Unicode
	MMSAddressType DestinationType;
	unsigned char CC[200];	// in Unicode
	MMSAddressType CCType;

	unsigned char Subject[200];	// in Unicode
	unsigned char ContentType[400];	// CT in Unicode
	unsigned char MSGType[50];	// no Unicode

	bool DateTimeAvailable;
	GSM_DateTime DateTime;

	bool MMSReportAvailable;
	bool MMSReport;
} GSM_EncodedMultiPartMMSInfo2;

GSM_Error GSM_DecodeMMSFileToMultiPart(GSM_File * file,
				       GSM_EncodedMultiPartMMSInfo2 * info);
GSM_Error GSM_ClearMMSMultiPart(GSM_EncodedMultiPartMMSInfo2 * info);

/**
 * Gets SMS Service Center number and SMS settings.
 */
GSM_Error GSM_GetSMSC(GSM_StateMachine * s, GSM_SMSC * smsc);
/**
 * Sets SMS Service Center number and SMS settings.
 */
GSM_Error GSM_SetSMSC(GSM_StateMachine * s, GSM_SMSC * smsc);
/**
 * Gets information about SMS memory (read/unread/size of memory for
 * both SIM and phone).
 */
GSM_Error GSM_GetSMSStatus(GSM_StateMachine * s, GSM_SMSMemoryStatus * status);
/**
 * Reads SMS message.
 */
GSM_Error GSM_GetSMS(GSM_StateMachine * s, GSM_MultiSMSMessage * sms);
/**
 * Reads next (or first if start set) SMS message. This might be
 * faster for some phones than using \ref GSM_GetSMS for each message.
 */
GSM_Error GSM_GetNextSMS(GSM_StateMachine * s, GSM_MultiSMSMessage * sms,
			 bool start);
/**
 * Sets SMS.
 */
GSM_Error GSM_SetSMS(GSM_StateMachine * s, GSM_SMSMessage * sms);
/**
 * Adds SMS to specified folder.
 */
GSM_Error GSM_AddSMS(GSM_StateMachine * s, GSM_SMSMessage * sms);
/**
 * Deletes SMS.
 */
GSM_Error GSM_DeleteSMS(GSM_StateMachine * s, GSM_SMSMessage * sms);
/**
 * Sends SMS.
 */
GSM_Error GSM_SendSMS(GSM_StateMachine * s, GSM_SMSMessage * sms);
/**
 * Sends SMS already saved in phone.
 */
GSM_Error GSM_SendSavedSMS(GSM_StateMachine * s, int Folder, int Location);
/**
 * Configures fast SMS sending.
 */
GSM_Error GSM_SetFastSMSSending(GSM_StateMachine * s, bool enable);
/**
 * Enable/disable notification on incoming SMS.
 */
GSM_Error GSM_SetIncomingSMS(GSM_StateMachine * s, bool enable);
/**
 * Gets network information from phone.
 */
GSM_Error GSM_SetIncomingCB(GSM_StateMachine * s, bool enable);
/**
 * Returns SMS folders information.
 */
GSM_Error GSM_GetSMSFolders(GSM_StateMachine * s, GSM_SMSFolders * folders);
/**
 * Creates SMS folder.
 */
GSM_Error GSM_AddSMSFolder(GSM_StateMachine * s, unsigned char *name);
/**
 * Deletes SMS folder.
 */
GSM_Error GSM_DeleteSMSFolder(GSM_StateMachine * s, int ID);

/**
 * Lists MMS folders.
 */
GSM_Error GSM_GetMMSFolders(GSM_StateMachine * s, GSM_MMSFolders * folders);
/**
 * Retrieves next part of MMS file information.
 */
GSM_Error GSM_GetNextMMSFileInfo(GSM_StateMachine * s, unsigned char *FileID,
				 int *MMSFolder, bool start);
/**
 * Activates/deactivates noticing about incoming USSDs (UnStructured Supplementary Services).
 */
GSM_Error GSM_SetIncomingUSSD(GSM_StateMachine * s, bool enable);
#endif
