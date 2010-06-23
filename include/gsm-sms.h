/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#ifndef __gsm_sms_h
#define __gsm_sms_h

#include "gsm-common.h"
#include "gsm-datetime.h"

/* This data-type is used to specify the type of the number. See the official
   GSM specification 03.40, version 5.3.0, section 9.1.2.5, page 33. */
typedef enum {
  GNT_UNKNOWN=0x81,       /* Unknown number */
  GNT_INTERNATIONAL=0x91, /* International number */
  GNT_ALPHANUMERIC=0xD0   /* Alphanumeric number */
} GSM_NumberType;

/* For folder table */
typedef struct {
  int smsnum;
  u8 folder;
  int location;
//future use  GSM_SMSMessage *message;
} GSM_SMS2Foldertable;

/* This data type is used to hold the current SMS status. */
typedef struct {
  int UnRead;                           /* The number of unread messages */
  int Number;                           /* The number of messages */
  GSM_SMS2Foldertable foldertable[255]; /* for 6210/7110 */
} GSM_SMSStatus;

/* Define datatype for SMS Message Type */
typedef enum {
  GST_SMS,
  GST_DR, /* Delivery Report */
  GST_UN  /* Unknown */
} GSM_SMSMessageType;

#define GST_INBOX       0;    /* Mobile Terminated (MT) message - Inbox message  */
#define GST_OUTBOX      1;    /* Mobile Originated (MO) message - Outbox message */
#define GST_7110_INBOX  0x08; /* Inbox in 6210/7110 */
#define GST_7110_OUTBOX 0x10; /* Outbox in 6210/7110 */

/* Datatype for SMS status */
typedef enum {
  GSS_SENTREAD       = 0x01, /* Sent or read message */
  GSS_NOTSENTREAD    = 0x03, /* Not sent or not read message */
  GSS_UNKNOWN	     = 0x05, /* RTH FIXME: ? */
  GSS_TEMPLATE	     = 0x07  /* Template ? */
} GSM_SMSMessageStatus;

/* SMS Messages sent as... */
typedef enum {
  GSMF_Text   = 0x00, /* Plain text message. */
  GSMF_Fax    = 0x22, /* Fax message. */
  GSMF_Voice  = 0x24, /* Voice mail message. */
  GSMF_ERMES  = 0x25, /* ERMES message. */
  GSMF_Paging = 0x26, /* Paging. */
  GSMF_UCI    = 0x2d, /* Email message in 8110i. */
  GSMF_Email  = 0x32, /* Email message. */
  GSMF_X400   = 0x31  /* X.400 message. */
} GSM_SMSMessageFormat;

/* Validity of SMS Messages. */
/* Ready values for TP-VP. Don't give them for function packing SMS validity
   (creating TP-VP from validity value in minutes) */
typedef enum {
  GSMV_1_Hour   = 0x0b,
  GSMV_6_Hours  = 0x47,
  GSMV_24_Hours = 0xa7,
  GSMV_72_Hours = 0xa9,
  GSMV_1_Week   = 0xad,
  GSMV_Max_Time = 0xff
} GSM_SMSMessageValidity;

/* 7 bit SMS Coding type */
typedef enum {
  GSM_Coding_Unicode = 0x01,
  GSM_Coding_Default = 0x02,
  GSM_Coding_8bit    = 0x03
} GSM_Coding_Type;

/* Define datatype for SMS Message Center */
typedef struct {
  int No;                                          /* Number of the SMSC in the phone memory. */
  char Name[GSM_MAX_SMS_CENTER_NAME_LENGTH];       /* Name of the SMSC. */
  GSM_SMSMessageFormat Format;                     /* SMS is sent as text/fax/paging/email. */
  GSM_SMSMessageValidity Validity;                 /* Validity of SMS Message. */
  char Number[GSM_MAX_SMS_CENTER_LENGTH];          /* Number of the SMSC. */
  char DefaultRecipient[GSM_MAX_RECIPIENT_LENGTH]; /* Number of default recipient */
} GSM_MessageCenter;

/* Define datatype for Cell Broadcast message */
typedef struct {
  int Channel;                                      /* channel number */
  char Message[GSM_MAX_CB_MESSAGE + 1];
  int New;
} GSM_CBMessage;

/* types of User Data Header */
typedef enum {
  GSM_NoUDH,
  GSM_ConcatenatedMessages,
  GSM_OpLogo,
  GSM_CallerIDLogo,
  GSM_RingtoneUDH,
  GSM_EnableVoice,
  GSM_DisableVoice,
  GSM_EnableEmail,
  GSM_DisableEmail,
  GSM_EnableFax,
  GSM_DisableFax,
  GSM_VoidSMS,
  GSM_HangSMS,
  GSM_BugSMS,
  GSM_UnknownUDH, //Gnokii doesn't know this UDH
  GSM_WAPBookmarkUDH,
  GSM_WAPBookmarkUDHLong,
  GSM_WAPSettingsUDH,
  GSM_CalendarNoteUDH,
  GSM_CalendarNoteUDH2,
  GSM_PhonebookUDH,
  GSM_ProfileUDH,
} GSM_UDH;

/* Define datatype for SMS messages, used for getting SMS messages from the
   phones memory. */
typedef struct {
  GSM_Coding_Type Coding;
  GSM_DateTime Time;	                         /* Date of reception/response of messages. */
  GSM_DateTime SMSCTime;	                 /* Date of SMSC response if DeliveryReport messages. */
  int Length;                                    /* Length of the SMS message. */
  int Validity;                                  /* Validity Period of the SMS message. */
  GSM_UDH UDHType;                               /* If UDH is present - type of UDH */
  unsigned char UDH[GSM_MAX_USER_DATA_HEADER_LENGTH]; /* If UDH is present - content of UDH */
  unsigned char MessageText[GSM_MAX_SMS_LENGTH + 1];  /* Room for null term. */
  GSM_MessageCenter MessageCenter;               /* SMS Center. */
  char Sender[GSM_MAX_SENDER_LENGTH + 1];        /* Sender of the SMS message. */
  char Destination[GSM_MAX_DESTINATION_LENGTH+1];/* Destination of the message. */
  int MessageNumber;                             /* Location in the memory, where SMS is saved */
  GSM_MemoryType MemoryType;                     /* Type of memory message is stored in. */
  GSM_SMSMessageType Type;                       /* Type of the SMS message */
  GSM_SMSMessageStatus Status;                   /* Status of the SMS message */
  int Class;                                     /* Class Message: 0, 1, 2, 3 or none; see GSM 03.38 */
  bool Compression;                              /* Indicates whether SMS contains compressed data */
  int Location;                                  /* Location in the memory, when save SMS */
  bool ReplyViaSameSMSC;                         /* Indicates whether "Reply via same center" is set */
  int folder;                                    /* Folder: 0-Inbox,1-Outbox, etc. */
  bool SMSData;      				 /* if folder contains sender, SMSC number and sending date */
  unsigned char Name[25+1];                      /* Name in Nokia 6210/7110, etc. Ignored in other */
} GSM_SMSMessage;

typedef struct {
  int number;
  GSM_SMSMessage SMS[6];
} GSM_MultiSMSMessage;

/* Maximal number of SMS folders */
/* #define MAX_SMS_FOLDERS 10 */
#define MAX_SMS_FOLDERS 24

typedef struct {
  char Name[15];     /* Name for SMS folder */
  u8 locations[160]; /* locations of SMS messages in that folder (6210 specific) */
  u8 number;         /* number of SMS messages in that folder*/
} GSM_OneSMSFolder;

typedef struct {
  GSM_OneSMSFolder Folder[MAX_SMS_FOLDERS];
  u8 FoldersID[MAX_SMS_FOLDERS]; // ID specific for this folder and phone.
	    			 // Used in internal functions. Do not use it.
  u8 number; //number of SMS folders
} GSM_SMSFolders;

/* Identifiers for Smart Messaging 3.0 multipart SMS */
#define SM30_ISOTEXT      0 // ISO 8859-1 text
#define SM30_UNICODETEXT  1
#define SM30_OTA          2
#define SM30_RINGTONE     3			 
#define SM30_PROFILENAME  4
#define SM30_SCREENSAVER  6

/* TP-Message-Type-Indicator */
/* See GSM 03.40 version 6.1.0 Release 1997 Section 9.2.3.1 */
typedef enum {                 
	SMS_Deliver = 0x00, /* when we save SMS in Inbox */
	SMS_Deliver_Report, 
	SMS_Status_Report,  /* Delivery Report received by phone */
	SMS_Command,
	SMS_Submit,         /* when we send SMS or save it in Outbox */
	SMS_Submit_Report  
} SMS_MessageType;

/* Structure to hold UDH Header */
typedef struct {
  GSM_UDH UDHType;     /* Type */
  int Length;          /* Length */
  unsigned char *Text; /* Text */
} GSM_UDHHeader;

typedef struct {
  unsigned char MessageText[GSM_MAX_SMS_LENGTH + 1];
    /* TP-User-Data. See GSM 03.40 section 9.2.3.24. Room for null term. */

  unsigned char Number[GSM_MAX_SENDER_LENGTH + 1];
 /*In SMS-Deliver       TP-Originating-Address.See GSM 03.40 section 9.2.3.7 */
 /*In SMS-Submit        TP-Destination-Address.See GSM 03.40 section 9.2.3.8 */
 /*In SMS-Status-Report TP-Recipient-Address.  See GSM 03.40 section 9.2.3.14*/

  unsigned char SMSCNumber[GSM_MAX_SMS_CENTER_LENGTH];
    /* SMSC number */

  unsigned char TPPID;
    /* TP-Protocol-Identifier. See GSM 03.40 section 9.2.3.9 */

  unsigned char TPDCS;
    /* TP-Data-Coding-Scheme. See GSM 03.40 section 9.2.3.10 */

  unsigned char DeliveryDateTime[7];
 /* For SMS-Submit        TP-Validity-Period. See GSM 03.40 section 9.2.3.12 */
 /* For SMS-Status-Report TP-Discharge Time.  See GSM 03.40 section 9.2.3.13 */

  unsigned char SMSCDateTime[7];
    /* TP-Service-Centre-Time-Stamp in SMS-Status-Report.
       See GSM 03.40 section 9.2.3.11 */

  unsigned char TPStatus;
    /* TP-Status in SMS-Status-Report. See GSM 03.40 section 9.2.3.15 */

  unsigned char TPUDL;
    /* TP-User-Data-Length. See GSM 03.40 section 9.2.3.16 */

  unsigned char TPVP;
    /* TP-Validity Period in SMS-Submit. See GSM 03.40 section 9.2.3.12 */

  unsigned char TPMR;
    /* TP-Message Reference in SMS-Submit. See GSM 03.40 section 9.2.3.6 */

  unsigned char firstbyte;
    /* Byte contains in SMS-Deliver:
       TP-Message-Type-Indicator     (2 bits) See GSM 03.40 section 9.2.3.1

       TP-More-Messages-To-Send      (1 bit). See GSM 03.40 section 9.2.3.2

       TP-Reply-Path                 (1 bit). See GSM 03.40 section 9.2.3.17
       TP-User-Data-Header-Indicator (1 bit). See GSM 03.40 section 9.2.3.23
       TP-Status-Report-Indicator    (1 bit). See GSM 03.40 section 9.2.3.4

       Byte contains in SMS-Submit:

       TP-Message-Type-Indicator     (2 bits) See GSM 03.40 section 9.2.3.1

       TP-Reject-Duplicates          (1 bit). See GSM 03.40 section
       TP-Validity-Period-Format     (2 bits).See GSM 03.40 section 9.2.3.3

       TP-Reply-Path                 (1 bit). See GSM 03.40 section 9.2.3.17
       TP-User-Data-Header-Indicator (1 bit). See GSM 03.40 section 9.2.3.23
       TP-Status-Report-Request      (1 bit). See GSM 03.40 section 9.2.3.5 */
} GSM_ETSISMSMessage;

int   GSM_PackSemiOctetNumber  (u8 *Number, u8 *Output, bool semioctet);
char *GSM_UnpackSemiOctetNumber(u8 *Number, bool semioctet);

int GSM_UnpackEightBitsToSeven(int fillbits, int in_length, int out_length, unsigned char *input, unsigned char *output);
int GSM_PackSevenBitsToEight  (int offset, unsigned char *input, unsigned char *output);

GSM_Error GSM_EncodeETSISMS(GSM_SMSMessage *SMS, GSM_ETSISMSMessage *ETSI, SMS_MessageType PDU, int *length);
GSM_Error GSM_DecodeETSISMS(GSM_SMSMessage *SMS, GSM_ETSISMSMessage *ETSI);

GSM_Error GSM_EncodeSMSDateTime(GSM_DateTime *DT, unsigned char *req);

GSM_Error GSM_DecodeETSISMSSubmitData(GSM_SMSMessage *SMS, GSM_ETSISMSMessage *ETSI);

GSM_Error GSM_DecodeETSISMSStatusReportData(GSM_SMSMessage *SMS, GSM_ETSISMSMessage *ETSI);

void GSM_SetDefaultSMSData(GSM_SMSMessage *SMS);
GSM_Error EncodeUDHHeader(char *text, GSM_UDH UDHType);

int GSM_MakeSinglePartSMS2(GSM_SMSMessage *SMS,
    unsigned char *MessageBuffer,int cur, GSM_UDH UDHType, GSM_Coding_Type Coding);
void GSM_MakeMultiPartSMS2(GSM_MultiSMSMessage *SMS,
    unsigned char *MessageBuffer,int MessageLength, GSM_UDH UDHType, GSM_Coding_Type Coding);

#endif	/* __gsm_sms_h */
