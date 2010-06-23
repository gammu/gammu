#ifndef _gsm_call_h
#define _gsm_call_h

#include "../misc/misc.h"

/* ------------------ call info -------------------------------------------- */

typedef enum {
        GN_CALL_IncomingCall=1,	 /* Somebody calls to us 	 			*/
        GN_CALL_OutgoingCall,	 /* We call somewhere 		 			*/
        GN_CALL_CallStart,	 /* Call started 	 				*/
        GN_CALL_CallEnd,	 /* End of call from unknown side 			*/
        GN_CALL_CallRemoteEnd,	 /* End of call from remote side  			*/
        GN_CALL_CallLocalEnd, 	 /* End of call from our side	 			*/
	GN_CALL_CallEstablished, /* Call established. Waiting for answer or dropping 	*/
	GN_CALL_CallHeld,        /* Call held 						*/
	GN_CALL_CallResumed,     /* Call resumed 					*/ 
	GN_CALL_CallSwitched	 /* We switch to call xx 				*/
} GSM_CallStatus;

typedef struct {
        GSM_CallStatus 		Status;
        char 			PhoneNumber [(GSM_MAX_NUMBER_LENGTH+1)*2];
	int			CallID;
	bool			CallIDAvailable;
	int			StatusCode;
} GSM_Call;

/* --------------- Data structures for the call divert -------------------- */

typedef enum {
	GSM_CDV_Busy = 0x01,     /* Divert when busy */ 
	GSM_CDV_NoAnswer,        /* Divert when not answered */
	GSM_CDV_OutOfReach,      /* Divert when phone off or no coverage */
	GSM_CDV_AllTypes         /* Divert all calls without ringing */
} GSM_CDV_DivertTypes;

typedef enum {
	GSM_CDV_VoiceCalls = 0x01,
	GSM_CDV_FaxCalls,
	GSM_CDV_DataCalls,
	GSM_CDV_AllCalls
} GSM_CDV_CallTypes;

typedef struct {
	GSM_CDV_DivertTypes DType;
	GSM_CDV_CallTypes   CType;
        char                Number[(GSM_MAX_NUMBER_LENGTH+1)*2];
	unsigned int        Timeout;
} GSM_CallDivert;

typedef struct {
	GSM_CallDivert	    		Request;
	struct {
		int		    	EntriesNum;
		GSM_CallDivert	    	Entries[10];
	} Response;
} GSM_MultiCallDivert;

/* -------------------------------- dial voice ---------------------------- */

typedef enum {
	GN_CALL_ShowNumber = 1,
	GN_CALL_HideNumber,
	GN_CALL_Default
} GSM_CallShowNumber;

#endif /* _gsm_call_h */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
