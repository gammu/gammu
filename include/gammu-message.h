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
#include <gammu-debug.h>

/**
 * \defgroup Message Messages
 * Messages manipulations.
 */

/**
 * \defgroup USSD USSD messages
 * USSD messages manipulations.
 * \ingroup Message
 */

/**
 * \defgroup CB CB messages
 * Cell broadcast messages manipulations.
 * \ingroup Message
 */

/**
 * \defgroup SMS SMS messages
 * SMS messages manipulations.
 * \ingroup Message
 */

/**
 * \defgroup MMS MMS messages
 * MMS messages manipulations.
 * \ingroup Message
 */

/**
 * MMS message class.
 */
typedef enum {
	/**
	 * None class specified.
	 */
	GSM_MMS_None = 0,
	/**
	 * Personal message.
	 */
	GSM_MMS_Personal = 1,
	/**
	 * Advertisement message.
	 */
	GSM_MMS_Advertisement,
	/**
	 * Informational message.
	 */
	GSM_MMS_Info,
	/**
	 * Automatic message class.
	 */
	GSM_MMS_Auto,
} GSM_MMS_Class;

/**
 * MMS indicator data.
 *
 * \ingroup MMS
 */
typedef struct {
	/**
	 * Message address (URL for download).
	 */
	char Address[500];
	/**
	 * Message title (subject).
	 */
	char Title[200];
	/**
	 * Message sender.
	 */
	char Sender[200];
	/**
	 * Message size, if 0 it won't be decoded or was not decoded.
	 */
	size_t MessageSize;
	/**
	 * Class of a message.
	 */
	GSM_MMS_Class Class;
} GSM_MMSIndicator;

/**
 * Structure for Cell Broadcast messages.
 *
 * \ingroup CB
 */
typedef struct {
	/**
	 * Channel number.
	 */
	int Channel;
	/**
	 * Message text.
	 */
	char Text[300];
} GSM_CBMessage;

/**
 * Status of USSD message.
 *
 * \ingroup USSD
 */
typedef enum {
	/**
	 * Unknown status
	 */
	USSD_Unknown = 1,
	/**
	 * No action is needed, maybe network initiated USSD
	 */
	USSD_NoActionNeeded,
	/**
	 * Reply is expected
	 */
	USSD_ActionNeeded,
	/**
	 * USSD dialog terminated
	 */
	USSD_Terminated,
	/**
	 * Another client replied
	 */
	USSD_AnotherClient,
	/**
	 * Operation not supported
	 */
	USSD_NotSupported,
	/**
	 * Network timeout
	 */
	USSD_Timeout,
} GSM_USSDStatus;

/**
 * Structure for USSD messages.
 *
 * \ingroup USSD
 */
typedef struct {
	/**
	 * Message text.
	 */
	unsigned char Text[2 * (GSM_MAX_USSD_LENGTH + 1)];
	/**
	 * Message status.
	 */
	GSM_USSDStatus Status;
} GSM_USSDMessage;

/**
 * Status of SMS memory.
 *
 * \ingroup SMS
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

/**
 * Enum defines format of SMS messages. See GSM 03.40 section 9.2.3.9
 *
 * \ingroup SMS
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
 * more values.
 *
 * \ingroup SMS
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
 *
 * \ingroup SMS
 */
typedef enum {
	SMS_Validity_NotAvailable = 1,
	SMS_Validity_RelativeFormat
	    /* Specification gives also other possibilities */
} GSM_ValidityPeriodFormat;

/**
 * Structure for validity of SMS messages
 *
 * \ingroup SMS
 */
typedef struct {
	GSM_ValidityPeriodFormat Format;
	/**
	 * Value defines period for relative format
	 */
	GSM_ValidityPeriod Relative;
} GSM_SMSValidity;

/**
 * Structure for SMSC (SMS Center) information.
 *
 * \ingroup SMS
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

/**
 * Status of SMS message.
 *
 * \ingroup SMS
 */
typedef enum {
	SMS_Sent = 1,
	SMS_UnSent,
	SMS_Read,
	SMS_UnRead
} GSM_SMS_State;

/**
 * Coding type of SMS.
 *
 * \ingroup SMS
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
 * Converts SMS coding to string.
 *
 * \return Pointer to static string, NULL on failure.
 */
const char *GSM_SMSCodingToString(GSM_Coding_Type type);

/**
 * Converts SMS coding to type.
 */
GSM_Coding_Type GSM_StringToSMSCoding(const char *s);

/**
 * Types of UDH (User Data Header).
 *
 * \ingroup SMS
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
 *
 * \ingroup SMS
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
 *
 * \ingroup SMS
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
 * SMS message data.
 *
 * \ingroup SMS
 */
typedef struct {
	/**
	 * Message to be replaced.
	 */
	unsigned char ReplaceMessage;
	/**
	 * Whether to reject duplicates.
	 */
	gboolean RejectDuplicates;
	/**
	 * UDH (User Data Header)
	 */
	GSM_UDHHeader UDH;
	/**
	 * Sender or recipient number.
	 */
	unsigned char Number[(GSM_MAX_NUMBER_LENGTH + 1) * 2];

	unsigned char
	 OtherNumbers[GSM_SMS_OTHER_NUMBERS][(GSM_MAX_NUMBER_LENGTH + 1) * 2];
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
	gboolean InboxFolder;
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
	gboolean ReplyViaSameSMSC;
	/**
	 * SMS class (0 is flash SMS, 1 is normal one).
	 */
	signed char Class;
	/**
	 * Message reference.
	 */
	unsigned char MessageReference;
} GSM_SMSMessage;

/* In layouts are saved locations for some SMS part. Below are listed
 * specs, which describe them
 *
 * \ingroup SMS
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

/**
 * Decodes PDU data.
 *
 * \param di Debug information structure.
 * \param SMS Pointer where to store parsed message.
 * \param buffer PDU data.
 * \param length Length of PDU data.
 * \param final_pos Optional pointer where end position will be stored.
 * \param SMSC Whether PDU includes SMSC data.
 *
 * \ingroup SMS
 */
GSM_Error GSM_DecodePDUFrame(GSM_Debug_Info *di, GSM_SMSMessage *SMS,
			const unsigned char *buffer, size_t length,
			size_t *final_pos, gboolean SMSC);

/**
 * Decodes SMS frame.
 *
 * \ingroup SMS
 */
GSM_Error GSM_DecodeSMSFrame(GSM_Debug_Info * di, GSM_SMSMessage * SMS,
			     unsigned char *buffer,
			     GSM_SMSMessageLayout Layout);

/**
 * Finds out coding type based on TPDCS header byte as defined by GSM
 * 03.38.
 */
GSM_Coding_Type GSM_GetMessageCoding(GSM_Debug_Info * di, const char TPDCS);

/**
 * Encodes SMS frame.
 *
 * \ingroup SMS
 */
GSM_Error GSM_EncodeSMSFrame(GSM_Debug_Info * di, GSM_SMSMessage * SMS,
			     unsigned char *buffer, GSM_SMSMessageLayout Layout,
			     int *length, gboolean clear);

/**
 * Decodes SMS frame for status report.
 *
 * \ingroup SMS
 */
GSM_Error GSM_DecodeSMSFrameStatusReportData(GSM_Debug_Info * di,
					     GSM_SMSMessage * SMS,
					     unsigned char *buffer,
					     GSM_SMSMessageLayout Layout);

/**
 * Decodes SMS frame in textual representation.
 *
 * \ingroup SMS
 */
GSM_Error GSM_DecodeSMSFrameText(GSM_Debug_Info * di, GSM_SMSMessage * SMS,
				 unsigned char *buffer,
				 GSM_SMSMessageLayout Layout);

/**
 * Decodes UDH header.
 *
 * \ingroup SMS
 */
void GSM_DecodeUDHHeader(GSM_Debug_Info * di, GSM_UDHHeader * UDH);

/**
 * Encodes UDH header.
 *
 * \ingroup SMS
 */
void GSM_EncodeUDHHeader(GSM_Debug_Info * di, GSM_UDHHeader * UDH);

/**
 * Sets default content for SMS except for changing locations.
 * Use this for clearing structure while keeping location of message.
 *
 * \param SMS Pointer to structure which should be cleaned up.
 *
 * \ingroup SMS
 */
void GSM_SetDefaultReceivedSMSData(GSM_SMSMessage * SMS);

/**
 * Sets default content for SMS. Use this for clearing structure.
 *
 * \param SMS Pointer to structure which should be cleaned up.
 *
 * \ingroup SMS
 */
void GSM_SetDefaultSMSData(GSM_SMSMessage * SMS);

/**
 * Information about SMS folder.
 *
 * \ingroup SMS
 */
typedef struct {
	/**
	 * Whether it is inbox.
	 */
	gboolean InboxFolder;
	/**
	 * Whether it is outbox.
	 */
	gboolean OutboxFolder;
	/**
	 * Where exactly it's saved.
	 */
	GSM_MemoryType Memory;
	/**
	 * Name of the folder
	 */
	unsigned char Name[(GSM_MAX_SMS_FOLDER_NAME_LEN + 1) * 2];
} GSM_OneSMSFolder;

/**
 * List of SMS folders.
 *
 * \ingroup SMS
 */
typedef struct {
	/**
	 * Array of structures holding information about each folder.
	 */
	GSM_OneSMSFolder Folder[GSM_MAX_SMS_FOLDERS];
	/**
 	 * Number of SMS folders.
	 */
	int Number;
} GSM_SMSFolders;

/**
 * Siemens OTA data.
 *
 * \ingroup SMS
 */
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

/**
 * Decodes Siemens OTA data.
 *
 * \ingroup SMS
 */
gboolean GSM_DecodeSiemensOTASMS(GSM_Debug_Info * di,
			     GSM_SiemensOTASMSInfo * Info,
			     GSM_SMSMessage * SMS);

/**
 * Multiple SMS messages, used for Smart Messaging 3.0/EMS.
 *
 * \ingroup SMS
 */
typedef struct {
	/**
	 * Number of messages.
	 */
	int Number;
	/**
	 * Array of SMSes.
	 */
	GSM_SMSMessage SMS[GSM_MAX_MULTI_SMS];
} GSM_MultiSMSMessage;

/**
 * Information about MMS folder.
 *
 * \ingroup MMS
 */
typedef struct {
	/**
	 * Whether it is really inbox.
	 */
	gboolean InboxFolder;
	/**
  	 * Name for MMS folder.
	 */
	char Name[(GSM_MAX_MMS_FOLDER_NAME_LEN + 1) * 2];
} GSM_OneMMSFolder;

/**
 * List of MMS folders.
 *
 * \ingroup MMS
 */
typedef struct {
	/**
 	 * Number of MMS folders.
	 */
	unsigned char Number;
	/**
	 * Array of structures holding information about each folder.
	 */
	GSM_OneMMSFolder Folder[GSM_MAX_MMS_FOLDERS];
} GSM_MMSFolders;

/**
 * Layout for submit message.
 *
 * \ingroup SMS
 */
extern GSM_SMSMessageLayout PHONE_SMSSubmit;

/**
 * Layout for deliver message.
 *
 * \ingroup SMS
 */
extern GSM_SMSMessageLayout PHONE_SMSDeliver;

/**
 * Layout for status report message.
 *
 * \ingroup SMS
 */
extern GSM_SMSMessageLayout PHONE_SMSStatusReport;

/**
 * ID during packing SMS for Smart Messaging 3.0, EMS and other
 *
 * \ingroup SMS
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
	/**
	 * MMS message indicator.
	 */
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

/**
 * Entry of multipart SMS.
 *
 * \ingroup SMS
 */
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
	gboolean Protected;

	unsigned char *Buffer;
	gboolean Left;
	gboolean Right;
	gboolean Center;
	gboolean Large;
	gboolean Small;
	gboolean Bold;
	gboolean Italic;
	gboolean Underlined;
	gboolean Strikethrough;

	/* Return values */
	int RingtoneNotes;
} GSM_MultiPartSMSEntry;

/**
 * Multipart SMS information.
 *
 * \ingroup SMS
 */
typedef struct {
	int EntriesNum;
	gboolean UnicodeCoding;
	int Class;
	unsigned char ReplaceMessage;
	gboolean Unknown;
	GSM_MultiPartSMSEntry Entries[GSM_MAX_MULTI_SMS];
} GSM_MultiPartSMSInfo;

/**
 * Encodes SMS frame according to layout.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error PHONE_EncodeSMSFrame(GSM_StateMachine * s, GSM_SMSMessage * SMS,
			       unsigned char *buffer,
			       GSM_SMSMessageLayout Layout, int *length,
			       gboolean clear);

/**
 * Encodes multi part SMS from "readable" format.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_EncodeMultiPartSMS(GSM_Debug_Info * di,
				 GSM_MultiPartSMSInfo * Info,
				 GSM_MultiSMSMessage * SMS);

/**
 * Decodes multi part SMS to "readable" format.
 *
 * \ingroup SMS
 */
gboolean GSM_DecodeMultiPartSMS(GSM_Debug_Info * di,
			    GSM_MultiPartSMSInfo * Info,
			    GSM_MultiSMSMessage * SMS, gboolean ems);

/**
 * Clears @ref GSM_MultiPartSMSInfo to default values.
 *
 * \ingroup SMS
 */
void GSM_ClearMultiPartSMSInfo(GSM_MultiPartSMSInfo * Info);

/**
 * Frees any allocated structures inside @ref GSM_MultiPartSMSInfo.
 *
 * \ingroup SMS
 */
void GSM_FreeMultiPartSMSInfo(GSM_MultiPartSMSInfo * Info);

/**
 * Links SMS messages according to IDs.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_LinkSMS(GSM_Debug_Info * di,
		      GSM_MultiSMSMessage ** INPUT,
		      GSM_MultiSMSMessage ** OUTPUT, gboolean ems);

/**
 * MMS address type.
 *
 * \ingroup MMS
 */
typedef enum {
	MMSADDRESS_PHONE,
	MMSADDRESS_UNKNOWN
} MMSAddressType;

/**
 * MMS entry.
 *
 * \ingroup MMS
 */
typedef struct {
	GSM_File File;
	/**
	 * CT in Unicode
	 */
	unsigned char ContentType[400];
	/**
	 * Smil ID in Unicode
	 */
	unsigned char SMIL[400];
} GSM_EncodedMultiPartMMSEntry;

/**
 * MMS part.
 *
 * \ingroup MMS
 */
typedef struct {
	int EntriesNum;

	/**
	 * in Unicode
	 */
	unsigned char Source[200];
	MMSAddressType SourceType;
	/**
	 * in Unicode
	 */
	unsigned char Destination[200];
	MMSAddressType DestinationType;
	/**
	 * in Unicode
	 */
	unsigned char CC[200];
	MMSAddressType CCType;

	/**
	 * in Unicode
	 */
	unsigned char Subject[200];
	/**
	 * CT in Unicode
	 */
	unsigned char ContentType[400];
	/**
	 * no Unicode
	 */
	unsigned char MSGType[50];

	gboolean DateTimeAvailable;
	GSM_DateTime DateTime;

	gboolean MMSReportAvailable;
	gboolean MMSReport;
	/**
	 * Subparts.
	 */
	GSM_EncodedMultiPartMMSEntry Entries[GSM_MAX_MULTI_MMS];
} GSM_EncodedMultiPartMMSInfo;

/**
 * Decodes MMS data.
 *
 * \ingroup MMS
 */
GSM_Error GSM_DecodeMMSFileToMultiPart(GSM_Debug_Info * di, GSM_File * file,
				       GSM_EncodedMultiPartMMSInfo * info);

/**
 * Clears MMS data, used to initialize structure.
 *
 * \ingroup MMS
 */
GSM_Error GSM_ClearMMSMultiPart(GSM_EncodedMultiPartMMSInfo * info);

/**
 * Gets SMS Service Center number and SMS settings.
 *
 * \param s State machine pointer.
 * \param[in,out] smsc SMSC structure, should contain location.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_GetSMSC(GSM_StateMachine * s, GSM_SMSC * smsc);

/**
 * Sets SMS Service Center number and SMS settings.
 *
 * \param s State machine pointer.
 * \param[in] smsc SMSC structure.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_SetSMSC(GSM_StateMachine * s, GSM_SMSC * smsc);

/**
 * Gets information about SMS memory (read/unread/size of memory for
 * both SIM and phone).
 *
 * \param s State machine pointer.
 * \param[out] status Pointer to SMS status structure.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_GetSMSStatus(GSM_StateMachine * s, GSM_SMSMemoryStatus * status);

/**
 * Reads SMS message.
 *
 * \param s State machine pointer.
 * \param[in,out] sms SMS message data read from phone, location and
 * folder should be set.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_GetSMS(GSM_StateMachine * s, GSM_MultiSMSMessage * sms);

/**
 * Reads next (or first if start set) SMS message. This might be
 * faster for some phones than using \ref GSM_GetSMS for each message.
 *
 * Please note that this commend does not have to mark message as read
 * in phone. To do so, you have to call \ref GSM_GetSMS.
 *
 * \param s State machine pointer.
 * \param[in,out] sms SMS message data read from phone, for subsequent
 * reads, location and folder might be used by phone driver to determine
 * reading state.
 * \param[in] start Whether we start reading from beginning.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_GetNextSMS(GSM_StateMachine * s, GSM_MultiSMSMessage * sms,
			 gboolean start);
/**
 * Sets SMS.
 *
 * \param s State machine pointer.
 * \param[in] sms SMS message data.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_SetSMS(GSM_StateMachine * s, GSM_SMSMessage * sms);

/**
 * Adds SMS to specified folder.
 *
 * \param s State machine pointer.
 * \param[in,out] sms SMS message data, location will be updated.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_AddSMS(GSM_StateMachine * s, GSM_SMSMessage * sms);

/**
 * Deletes SMS.
 *
 * \param s State machine pointer.
 * \param[in] sms SMS structure with SMS location and folder.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_DeleteSMS(GSM_StateMachine * s, GSM_SMSMessage * sms);

/**
 * Sends SMS.
 *
 * \param s State machine pointer.
 * \param[in] sms SMS structure with SMS data to send.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_SendSMS(GSM_StateMachine * s, GSM_SMSMessage * sms);

/**
 * Sends SMS already saved in phone.
 *
 * \param s State machine pointer.
 * \param[in] Folder Folder, where message is stored.
 * \param[in] Location Location, where message is stored.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_SendSavedSMS(GSM_StateMachine * s, int Folder, int Location);

/**
 * Configures fast SMS sending.
 *
 * \param s State machine pointer.
 * \param[in] enable Whether to enable notifications.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_SetFastSMSSending(GSM_StateMachine * s, gboolean enable);

/**
 * Enable/disable notification on incoming SMS.
 *
 * \param s State machine pointer.
 * \param[in] enable Whether to enable notifications.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_SetIncomingSMS(GSM_StateMachine * s, gboolean enable);

/**
 * Gets network information from phone.
 *
 * \param s State machine pointer.
 * \param[in] enable Whether to enable notifications.
 *
 * \return Error code.
 *
 * \ingroup CB
 */
GSM_Error GSM_SetIncomingCB(GSM_StateMachine * s, gboolean enable);

/**
 * Returns SMS folders information.
 *
 * \param s State machine pointer.
 * \param[out] folders folders Pointer to folders structure, which will be
 * filled in.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_GetSMSFolders(GSM_StateMachine * s, GSM_SMSFolders * folders);

/**
 * Creates SMS folder.
 *
 * \param s State machine pointer.
 * \param[in] name Name of SMS folder which should be created.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_AddSMSFolder(GSM_StateMachine * s, unsigned char *name);

/**
 * Deletes SMS folder.
 *
 * \param s State machine pointer.
 * \param[in] ID ID of SMS folder to delete.
 *
 * \return Error code.
 *
 * \ingroup SMS
 */
GSM_Error GSM_DeleteSMSFolder(GSM_StateMachine * s, int ID);

/**
 * Lists MMS folders.
 *
 * \param s State machine pointer.
 * \param folders Pointer to structure, whehe folder information will be
 * stored.
 *
 * \return Error code.
 *
 * \ingroup MMS
 */
GSM_Error GSM_GetMMSFolders(GSM_StateMachine * s, GSM_MMSFolders * folders);

/**
 * Retrieves next part of MMS file information.
 *
 * \param s State machine pointer.
 * \param[in,out] FileID File ID will be stored here, might be
 * used for consequent reads.
 * \param[in,out] MMSFolder MMS folder ID will be stored here, might be
 * used for consequent reads.
 * \param[in] start Whether to start reading.
 *
 * \return Error code.
 *
 * \ingroup MMS
 */
GSM_Error GSM_GetNextMMSFileInfo(GSM_StateMachine * s, unsigned char *FileID,
				 int *MMSFolder, gboolean start);
/**
 * Activates/deactivates noticing about incoming USSDs (UnStructured Supplementary Services).
 *
 * \param s State machine pointer.
 * \param[in] enable Whether to enable notifications.
 *
 * \return Error code.
 *
 * \ingroup USSD
 */
GSM_Error GSM_SetIncomingUSSD(GSM_StateMachine * s, gboolean enable);

/**
 * Calculates number of messages and free chars needed for text.
 *
 * \param di Debug settings.
 * \param[in] MessageBuffer Actual message text in unicode.
 * \param[in] UDHType UDH type.
 * \param[in] Coding GSM Encoding type.
 * \param[out] SMSNum Number of messages needed to store the text.
 * \param[out] CharsLeft Number of free chars in the message.
 *
 * \ingroup SMS
 */
void GSM_SMSCounter(GSM_Debug_Info *di, unsigned char *MessageBuffer,
	GSM_UDH UDHType, GSM_Coding_Type Coding, int *SMSNum, size_t *CharsLeft);

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
