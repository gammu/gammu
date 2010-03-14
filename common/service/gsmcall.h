#ifndef _gsm_call_h
#define _gsm_call_h

#include "../misc/misc.h"

/* ------------------ call info -------------------------------------------- */

/**
 * Status of call.
 */
typedef enum {
	/**
	 * Somebody calls to us
	 */
	 GN_CALL_IncomingCall=1,
	/**
	 * We call somewhere
	 */
	 GN_CALL_OutgoingCall,
	/**
	 * Call started
	 */
	 GN_CALL_CallStart,
	/**
	 * End of call from unknown side
	 */
	 GN_CALL_CallEnd,
	/**
	 * End of call from remote side
	 */
	 GN_CALL_CallRemoteEnd,
	/**
	 * End of call from our side
	 */
	 GN_CALL_CallLocalEnd,
	/**
	 * Call established. Waiting for answer or dropping
	 */
	GN_CALL_CallEstablished,
	/**
	 * Call held
	 */
	GN_CALL_CallHeld,
	/**
	 * Call resumed
	 */
	GN_CALL_CallResumed,
	/**
	 * We switch to call
	 */
	GN_CALL_CallSwitched
} GSM_CallStatus;

/**
 * Call information.
 */
typedef struct {
	/**
	 * Call status.
	 */
	GSM_CallStatus	  	Status;
	/**
	 * Remote phone number.
	 */
	char		   	PhoneNumber [(GSM_MAX_NUMBER_LENGTH+1)*2];
	/**
	 * Call ID
	 */
	int		     	CallID;
	/**
	 * Whether Call ID is available.
	 */
	bool		    	CallIDAvailable;
	/**
	 * Status code.
	 */
	int		     	StatusCode;
} GSM_Call;

/* --------------- Data structures for the call divert -------------------- */

/**
 * Defines when diversion is active.
 */
typedef enum {
	/**
	 * Divert when busy.
	 */
	GSM_CDV_Busy = 0x01,
	/**
	 * Divert when not answered.
	 */
	GSM_CDV_NoAnswer,
	/**
	 * Divert when phone off or no coverage.
	 */
	GSM_CDV_OutOfReach,
	/**
	 * Divert all calls without ringing.
	 */
	GSM_CDV_AllTypes
} GSM_CDV_DivertTypes;

/**
 * Which type of calls should be diverted.
 */
typedef enum {
	/**
	 * Voice calls.
	 */
	GSM_CDV_VoiceCalls = 0x01,
	/**
	 * Fax calls.
	 */
	GSM_CDV_FaxCalls,
	/**
	 * Data calls.
	 */
	GSM_CDV_DataCalls,
	/**
	 * All calls.
	 */
	GSM_CDV_AllCalls
} GSM_CDV_CallTypes;

/**
 * Call diversion definition.
 */
typedef struct {
	/**
	 * When diversion is active.
	 */
	GSM_CDV_DivertTypes 	DType;
	/**
	 * Type of call to divert.
	 */
	GSM_CDV_CallTypes   	CType;
	/**
	 * Number where to divert.
	 */
	char			Number[(GSM_MAX_NUMBER_LENGTH+1)*2];
	/**
	 * Timeout for diversion.
	 */
	unsigned int		Timeout;
} GSM_CallDivert;

/**
 * Multiple call diversions.
 */
typedef struct {
	GSM_CallDivert		Request;
	struct {
		int		EntriesNum;
		GSM_CallDivert	Entries[10];
	} Response;
} GSM_MultiCallDivert;

/* -------------------------------- dial voice ---------------------------- */

/**
 * How to handle number when initiating voice call.
 */
typedef enum {
	/**
	 * Show number.
	 */
	GN_CALL_ShowNumber = 1,
	/**
	 * Hide number.
	 */
	GN_CALL_HideNumber,
	/**
	 * Keep phone default settings.
	 */
	GN_CALL_Default
} GSM_CallShowNumber;

#endif /* _gsm_call_h */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
