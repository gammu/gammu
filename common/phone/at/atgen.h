
#ifndef atgen_h
#define atgen_h

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "../../service/gsmsms.h"

#ifndef GSM_USED_AT
#  define GSM_USED_AT
#endif
#ifndef GSM_USED_ATBLUETOOTH
#  define GSM_USED_ATBLUETOOTH
#endif
#define MAX_VCALENDAR_LOCATION 30
typedef enum {
	SMS_AT_PDU = 1,
	SMS_AT_TXT
} GSM_AT_SMS_Modes;

typedef enum {
	AT_Reply_OK = 1,
	AT_Reply_Connect,
	AT_Reply_Error,
	AT_Reply_Unknown,
	AT_Reply_CMSError,
	AT_Reply_CMEError
} GSM_AT_Reply_State;

typedef enum {
	AT_Nokia = 1,
	AT_Alcatel,
	AT_Siemens,
	AT_HP,
	AT_Falcom,
	AT_Unknown
} GSM_AT_Manufacturer;

typedef enum {
	AT_PBK_HEX = 1,
	AT_PBK_GSM,
	AT_PBK_UCS2
} GSM_AT_PBK_Charset;

typedef enum {
	AT_PHONE_SMS_AVAILABLE = 1,
	AT_PHONE_SMS_NOTAVAILABLE
} GSM_AT_PHONE_SMSMemory;

typedef enum {
	AT_SBNR_AVAILABLE = 1,
	AT_SBNR_NOTAVAILABLE
} GSM_AT_SBNR;

typedef struct {
	GSM_AT_Manufacturer	Manufacturer;	  /* Who is manufacturer			*/
	GSM_Lines		Lines;		  /* Allow to simply get each line in response 	*/
	GSM_AT_Reply_State	ReplyState;	  /* What response type - error, OK, etc. 	*/
	int			ErrorCode;	  /* Error codes from responses			*/
    	char			*ErrorText;    	  /* Error description */

	GSM_MemoryType		PBKMemory;	  /* Last read PBK memory			*/
	char			PBKMemories[200]; /* Supported by phone PBK memories		*/
	GSM_AT_PBK_Charset	PBKCharset;	  /* Last read PBK charset			*/
	GSM_AT_SBNR		PBKSBNR;

	GSM_SMSMemoryStatus	LastSMSStatus;
	int			LastSMSRead;
	int			FirstCalendarPos;
	GSM_AT_PHONE_SMSMemory	PhoneSMSMemory;	  /* Is phone SMS memory available ? 		*/
	GSM_MemoryType		SMSMemory;	  /* Last read SMS memory 			*/
	GSM_AT_SMS_Modes	SMSMode;	  /* PDU or TEXT mode for SMS ? 		*/
} GSM_Phone_ATGENData;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
