/* (c) 2002-2004 by Marcin Wiacek and Michal Cihar */

#ifndef atgen_h
#define atgen_h

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "../../service/sms/gsmsms.h"

#ifndef GSM_USED_AT
#  define GSM_USED_AT
#endif
#ifndef GSM_USED_BLUEAT
#  define GSM_USED_BLUEAT
#endif
#ifndef GSM_USED_IRDAAT
#  define GSM_USED_IRDAAT
#endif

#define MAX_VCALENDAR_LOCATION 50

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
	AT_Reply_CMEError,
	AT_Reply_SMSEdit
} GSM_AT_Reply_State;

typedef enum {
	AT_Nokia = 1,
	AT_Alcatel,
	AT_Siemens,
	AT_HP,
	AT_Falcom,
	AT_Ericsson,
	AT_Sagem,
	AT_Samsung,
	AT_Unknown
} GSM_AT_Manufacturer;

typedef enum {
	AT_PBK_HEX = 1,
	AT_PBK_GSM,
	AT_PBK_UCS2,
	AT_PBK_PCCP437
} GSM_AT_PBK_Charset;

typedef enum {
	AT_AVAILABLE = 1,
	AT_NOTAVAILABLE
} GSM_AT_SMSMemory;

typedef enum {
	AT_SBNR_AVAILABLE = 1,
	AT_SBNR_NOTAVAILABLE
} GSM_AT_SBNR;

typedef enum {
	AT_Status,
	AT_NextEmpty,
	AT_Total,
	AT_First,
	AT_Sizes
} GSM_AT_NeededMemoryInfo;

#define AT_PBK_MAX_MEMORIES	200

typedef struct {
	GSM_AT_Manufacturer	Manufacturer;	  	/* Who is manufacturer						*/
	GSM_Lines		Lines;		  	/* Allow to simply get each line in response 			*/
	GSM_AT_Reply_State	ReplyState;	  	/* What response type - error, OK, etc. 			*/
	int			ErrorCode;	  	/* Error codes from responses					*/
    	char			*ErrorText;    	  	/* Error description 						*/

	GSM_MemoryType		PBKMemory;	  	/* Last read PBK memory				 		*/
	char			PBKMemories[AT_PBK_MAX_MEMORIES + 1]; /* Supported by phone PBK memories 		*/
	int			NextMemoryEntry;  	/* Next empty memory entry					*/
	int			FirstMemoryEntry;	/* First memory entry to be read				*/
	GSM_AT_PBK_Charset	PBKCharset;	  	/* Last read PBK charset					*/
	bool			UCS2CharsetFailed;	/* Whether setting of UCS2 charset has already failed 		*/
	bool			NonUCS2CharsetFailed;	/* Whether setting of non-UCS2 charset has already failed 	*/
	GSM_AT_SBNR		PBKSBNR;
	int			NumberLength;
	int			TextLength;
	int			MemorySize;

	GSM_SMSMemoryStatus	LastSMSStatus;
	int			LastSMSRead;
	int			FirstCalendarPos;
	bool			CanSaveSMS;
	GSM_AT_SMSMemory	PhoneSMSMemory;	  	/* Is phone SMS memory available ? 				*/
	GSM_AT_SMSMemory	SIMSMSMemory;	  	/* Is SIM SMS memory available ? 				*/
	GSM_MemoryType		SMSMemory;	  	/* Last read SMS memory 					*/
	GSM_AT_SMS_Modes	SMSMode;	  	/* PDU or TEXT mode for SMS ? 					*/

	bool			OBEX;
	GSM_File		file;
} GSM_Phone_ATGENData;

GSM_Error ATGEN_HandleCMSError	(GSM_StateMachine *);
GSM_Error ATGEN_HandleCMEError	(GSM_StateMachine *);
GSM_Error ATGEN_DispatchMessage	(GSM_StateMachine *);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
