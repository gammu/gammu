
#ifndef atgen_h
#define atgen_h

#include "../../gsmcomon.h"
#include "../../service/gsmsms.h"

#ifndef GSM_USED_AT
#  define GSM_USED_AT
#endif
#ifndef GSM_USED_ATBLUETOOTH
#  define GSM_USED_ATBLUETOOTH
#endif

typedef enum {
	SMS_AT_PDU = 1,
	SMS_AT_TXT
} GSM_AT_SMS_Modes;

typedef enum {
	AT_Reply_OK = 1,
	AT_Reply_Error,
	AT_Reply_Unknown,
	AT_Reply_CMSError
} GSM_AT_Reply_State;

typedef enum {
	AT_Nokia = 1,
	AT_Alcatel,
	AT_Siemens,
	AT_HP,
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

typedef struct {
	GSM_AT_Manufacturer	Manufacturer;	  /* Who is manufacturer			*/
	GSM_Lines		Lines;		  /* Allow to simply get each line in response 	*/
	GSM_AT_Reply_State	ReplyState;	  /* What response type - error, OK, etc. 	*/
	int			ErrorCode;	  /* Error codes from responses			*/

	GSM_MemoryType		PBKMemory;	  /* Last read PBK memory			*/
	char			PBKMemories[200]; /* Supported by phone PBK memories		*/
	GSM_AT_PBK_Charset	PBKCharset;	  /* Last read PBK charset			*/

	GSM_SMSMemoryStatus	LastSMSStatus;
	int			LastSMSRead;
	GSM_AT_PHONE_SMSMemory	PhoneSMSMemory;	  /* Is phone SMS memory available ? 		*/
	GSM_MemoryType		SMSMemory;	  /* Last read SMS memory 			*/
	GSM_AT_SMS_Modes	SMSMode;	  /* PDU or TEXT mode for SMS ? 		*/
} GSM_Phone_ATGENData;

#endif
