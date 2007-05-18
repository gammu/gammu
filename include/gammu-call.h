/**
 * \file gammu-call.h
 * \author Michal Čihař
 * 
 * Call data and functions.
 */
#ifndef __gammu_call_h
#define __gammu_call_h

/**
 * \defgroup Call Call
 * Call entries manipulations.
 */

/**
 * \defgroup Divert Divert
 * \ingroup Call
 * Diversion entries manipulations.
 */

/**
 * Enum with status of call.
 *
 * \ingroup Call
 */
typedef enum {
	/**
	 * Somebody calls to us
	 */
	GSM_CALL_IncomingCall = 1,
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
 *
 * \ingroup Call
 */
typedef struct {
	/**
	 * Call status.
	 */
	GSM_CallStatus Status;
	/**
	 * Remote phone number.
	 */
	unsigned char PhoneNumber[(GSM_MAX_NUMBER_LENGTH + 1) * 2];
	/**
	 * Call ID
	 */
	int CallID;
	/**
	 * Whether Call ID is available.
	 */
	bool CallIDAvailable;
	/**
	 * Status code.
	 */
	int StatusCode;
} GSM_Call;

/**
 * Defines when diversion is active.
 *
 * \ingroup Divert
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
 *
 * \ingroup Divert
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
 *
 * \ingroup Divert
 */
typedef struct {
	/**
	 * When diversion is active.
	 */
	GSM_Divert_DivertTypes DivertType;
	/**
	 * Type of call to divert.
	 */
	GSM_Divert_CallTypes CallType;
	/**
	 * Number where to divert.
	 */
	char Number[(GSM_MAX_NUMBER_LENGTH + 1) * 2];
	/**
	 * Timeout for diversion.
	 */
	unsigned int Timeout;
} GSM_CallDivert;

/**
 * Multiple call diversions.
 *
 * \ingroup Divert
 */
typedef struct {
	GSM_CallDivert Request;
	struct {
		int EntriesNum;
		GSM_CallDivert Entries[10];
	} Response;
} GSM_MultiCallDivert;

/**
 * How to handle number when initiating voice call.
 *
 * \ingroup Call
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

/**
 * Dials number and starts voice call.
 *
 * \ingroup Call
 */
GSM_Error GSM_DialVoice(GSM_StateMachine * s, char *Number,
			GSM_CallShowNumber ShowNumber);
/**
 * Dials service number (usually for USSD).
 *
 * \ingroup Call
 */
GSM_Error GSM_DialService(GSM_StateMachine * s, char *Number);
/**
 * Accept current incoming call.
 *
 * \ingroup Call
 */
GSM_Error GSM_AnswerCall(GSM_StateMachine * s, int ID, bool all);
/**
 * Deny current incoming call.
 *
 * \ingroup Call
 */
GSM_Error GSM_CancelCall(GSM_StateMachine * s, int ID, bool all);
/**
 * Holds call.
 *
 * \ingroup Call
 */
GSM_Error GSM_HoldCall(GSM_StateMachine * s, int ID);
/**
 * Unholds call.
 *
 * \ingroup Call
 */
GSM_Error GSM_UnholdCall(GSM_StateMachine * s, int ID);
/**
 * Initiates conference call.
 *
 * \ingroup Call
 */
GSM_Error GSM_ConferenceCall(GSM_StateMachine * s, int ID);
/**
 * Splits call.
 *
 * \ingroup Call
 */
GSM_Error GSM_SplitCall(GSM_StateMachine * s, int ID);
/**
 * Transfers call.
 *
 * \ingroup Call
 */
GSM_Error GSM_TransferCall(GSM_StateMachine * s, int ID, bool next);
/**
 * Switches call.
 *
 * \ingroup Call
 */
GSM_Error GSM_SwitchCall(GSM_StateMachine * s, int ID, bool next);
/**
 * Gets call diverts.
 *
 * \ingroup Divert
 */
GSM_Error GSM_GetCallDivert(GSM_StateMachine * s, GSM_MultiCallDivert * divert);
/**
 * Sets call diverts.
 *
 * \ingroup Divert
 */
GSM_Error GSM_SetCallDivert(GSM_StateMachine * s, GSM_MultiCallDivert * divert);
/**
 * Cancels all diverts.
 *
 * \ingroup Divert
 */
GSM_Error GSM_CancelAllDiverts(GSM_StateMachine * s);
/**
 * Activates/deactivates noticing about incoming calls.
 *
 * \ingroup Call
 */
GSM_Error GSM_SetIncomingCall(GSM_StateMachine * s, bool enable);
/**
 * Sends DTMF (Dual Tone Multi Frequency) tone.
 *
 * \ingroup Call
 */
GSM_Error GSM_SendDTMF(GSM_StateMachine * s, char *sequence);

#endif
