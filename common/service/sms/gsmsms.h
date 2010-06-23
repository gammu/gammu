/* (c) 2001-2004 by Marcin Wiacek */
/* Based on some Pawel Kot and others work from Gnokii (www.gnokii.org)
 * (C) 1999-2000 Hugh Blemings & Pavel Janik ml. (C) 2001-2004 Pawel Kot 
 * GNU GPL version 2 or later
 */

#ifndef __gsm_sms_h
#define __gsm_sms_h

#include "../../gsmcomon.h"
#include "../gsmlogo.h"
#include "../gsmcal.h"
#include "../gsmpbk.h"
#include "../gsmdata.h"
#include "../gsmring.h"

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
	char    Text[300];
	/**
	 * Channel number.
	 */
	int     Channel;
} GSM_CBMessage;

/* ------------------------ SMS status ------------------------------------ */

/**
 * Status of SMS memory.
 */
typedef struct {
	/**
	 * Number of unread messages on SIM.
	 */
	int     SIMUnRead;
	/**
	 * Number of all saved messages (including unread) on SIM.
	 */
	int     SIMUsed;
	/**
	 * Number of all possible messages on SIM.
	 */
	int     SIMSize;
	/**
	 * Number of used templates (62xx/63xx/7110/etc.).
	 */
	int     TemplatesUsed;
	/**
	 * Number of unread messages in phone.
	 */
	int     PhoneUnRead;
	/**
	 * Number of all saved messages in phone.
	 */
	int     PhoneUsed;
	/**
	 * Number of all possible messages on phone.
	 */
	int     PhoneSize;
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
	SMS_VALID_1_Hour   = 0x0b,
	SMS_VALID_6_Hours  = 0x47,
	SMS_VALID_1_Day    = 0xa7,
	SMS_VALID_3_Days   = 0xa9,
	SMS_VALID_1_Week   = 0xad,
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
	GSM_ValidityPeriodFormat	Format;
	/**
	 * Value defines period for relative format
	 */
	GSM_ValidityPeriod	      	Relative;
} GSM_SMSValidity;

#define GSM_MAX_SMSC_NAME_LENGTH	30

/**
 * Structure for SMSC (SMS Center) information.
 */
typedef struct {
	/**
	 * Number of the SMSC on SIM
	 */
	int	     	Location;
	/**
	 * Name of the SMSC
	 */
	unsigned char   Name[(GSM_MAX_SMSC_NAME_LENGTH+1)*2];
	/**
	 * SMSC phone number.
	 */
	unsigned char   Number[(GSM_MAX_NUMBER_LENGTH+1)*2];
	/**
	 * Validity of SMS messages.
	 */
	GSM_SMSValidity Validity;
	/**
	 * Format of sent SMS messages.
	 */
	GSM_SMSFormat   Format;
	/**
	 * Default recipient number. In old DCT3 ignored
	 */
	unsigned char   DefaultNumber[(GSM_MAX_NUMBER_LENGTH+1)*2];
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
	SMS_Coding_Unicode = 1,
	/**
	 * Default GSM aplhabet.
	 */
	SMS_Coding_Default,
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
#define GSM_MAX_SMS_NAME_LENGTH	 40

/**
 * SMS message data.
 */
typedef struct {
	/**
	 * Message to be replaced.
	 */
	unsigned char	   	ReplaceMessage;
	/**
	 * Whether to reject duplicates.
	 */
	bool		    	RejectDuplicates;
	/**
	 * UDH (User Data Header)
	 */
	GSM_UDHHeader	   	UDH;
	/**
	 * Sender or recipient number.
	 */
	unsigned char	   	Number[(GSM_MAX_NUMBER_LENGTH+1)*2];
	/**
	 * SMSC (SMS Center)
	 */
	GSM_SMSC		SMSC;
	/**
	 * For saved SMS: where exactly it's saved (SIM/phone)
	 */
	GSM_MemoryType		Memory;
	/**
	 * For saved SMS: location of SMS in memory.
	 */
	int		     	Location;
	/**
	 * For saved SMS: number of folder, where SMS is saved
	 */
	int		     	Folder;
	/**
	 * For saved SMS: whether SMS is really in Inbox.
	 */
	bool		    	InboxFolder;
	/**
	 * Length of the SMS message.
	 */
	int		     	Length;
	/**                 	
	 * Status (read/unread/...) of SMS message.
	 */
	GSM_SMS_State	   	State;
	/**
	 * Name in Nokia with SMS memory (6210/7110, etc.) Ignored in other.
	 */
	unsigned char	   	Name[(GSM_MAX_SMS_NAME_LENGTH+1)*2];
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
	 * Date and time, when SMS was saved or sent
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
	 * SMS class.
	 */
	char		    	Class;
	/**
	 * Message reference.
	 */
	unsigned char	   	MessageReference;
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

GSM_Error GSM_DecodeSMSFrame(GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout);
GSM_Error GSM_EncodeSMSFrame(GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout, int *length, bool clear);

GSM_Error GSM_DecodeSMSFrameStatusReportData    (GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout);
GSM_Error GSM_DecodeSMSFrameText		(GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout);

void GSM_DecodeUDHHeader(GSM_UDHHeader *UDH);
void GSM_EncodeUDHHeader(GSM_UDHHeader *UDH);

void GSM_SetDefaultSMSData(GSM_SMSMessage *SMS);

/* ---------------------- SMS folders ------------------------------------- */

/**
 * Number of possible SMS folders.
 */
#define GSM_MAX_SMS_FOLDERS	     	24
/**
 * Maximal length of SMS folder name.
 */
#define GSM_MAX_SMS_FOLDER_NAME_LEN     20

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

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
