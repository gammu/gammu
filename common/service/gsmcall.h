/* (c) 2002-2003 by Marcin Wiacek */

#ifndef _gsm_call_h
#define _gsm_call_h

#include "../misc/misc.h"

/* ------------------ call info -------------------------------------------- */

/**
 * Enum with status of call.
 */
typedef enum {
	/**
	 * Somebody calls to us
	 */
	GSM_CALL_IncomingCall=1,
	/**
	 * We call somewhere
	 */
	GSM_CALL_OutgoingCall,
	/**
	 * Call started
	 */
	GSM_CALL_CallStart,
	/**
	 * End of call from unknown side
	 */
	GSM_CALL_CallEnd,
	/**
	 * End of call from remote side
	 */
	GSM_CALL_CallRemoteEnd,
	/**
	 * End of call from our side
	 */
	GSM_CALL_CallLocalEnd,
	/**
	 * Call established. Waiting for answer or dropping
	 */
	GSM_CALL_CallEstablished,
	/**
	 * Call held
	 */
	GSM_CALL_CallHeld,
	/**
	 * Call resumed
	 */
	GSM_CALL_CallResumed,
	/**
	 * We switch to call
	 */
	GSM_CALL_CallSwitched
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
	GSM_DIVERT_Busy = 0x01,
	/**
	 * Divert when not answered.
	 */
	GSM_DIVERT_NoAnswer,
	/**
	 * Divert when phone off or no coverage.
	 */
	GSM_DIVERT_OutOfReach,
	/**
	 * Divert all calls without ringing.
	 */
	GSM_DIVERT_AllTypes
} GSM_Divert_DivertTypes;

/**
 * Which type of calls should be diverted.
 */
typedef enum {
	/**
	 * Voice calls.
	 */
	GSM_DIVERT_VoiceCalls = 0x01,
	/**
	 * Fax calls.
	 */
	GSM_DIVERT_FaxCalls,
	/**
	 * Data calls.
	 */
	GSM_DIVERT_DataCalls,
	/**
	 * All calls.
	 */
	GSM_DIVERT_AllCalls
} GSM_Divert_CallTypes;

/**
 * Call diversion definition.
 */
typedef struct {
	/**
	 * When diversion is active.
	 */
	GSM_Divert_DivertTypes 	DivertType;
	/**
	 * Type of call to divert.
	 */
	GSM_Divert_CallTypes   	CallType;
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
	GSM_CALL_ShowNumber = 1,
	/**
	 * Hide number.
	 */
	GSM_CALL_HideNumber,
	/**
	 * Keep phone default settings.
	 */
	GSM_CALL_DefaultNumberPresence
} GSM_CallShowNumber;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
