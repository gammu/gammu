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

#include <gammu-types.h>
#include <gammu-limits.h>
#include <gammu-error.h>
#include <gammu-statemachine.h>

/**
 * \defgroup Divert Divert
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
	 * Call ID
	 */
	int CallID;
	/**
	 * Whether Call ID is available.
	 */
	gboolean CallIDAvailable;
	/**
	 * Status code.
	 */
	int StatusCode;
	/**
	 * Remote phone number.
	 */
	unsigned char PhoneNumber[(GSM_MAX_NUMBER_LENGTH + 1) * 2];
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
	 * Timeout for diversion.
	 */
	unsigned int Timeout;
	/**
	 * Number where to divert.
	 */
	char Number[(GSM_MAX_NUMBER_LENGTH + 1) * 2];
} GSM_CallDivert;

/**
 * Multiple call diversions.
 *
 * \ingroup Divert
 */
typedef struct {
	int EntriesNum;
	GSM_CallDivert Entries[GSM_MAX_CALL_DIVERTS];
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
 * \param s State machine pointer.
 * \param Number Number to dial.
 * \param ShowNumber Whether we want to display number on phone.
 *
 * \return Error code
 *
 * \ingroup Call
 */
GSM_Error GSM_DialVoice(GSM_StateMachine * s, char *Number,
			GSM_CallShowNumber ShowNumber);
/**
 * Dials service number (usually for USSD).
 *
 * \param s State machine pointer.
 * \param Number Number to dial.
 *
 * \return Error code
 *
 * \ingroup Call
 */
GSM_Error GSM_DialService(GSM_StateMachine * s, char *Number);

/**
 * Accept current incoming call.
 *
 * \param s State machine pointer.
 * \param ID ID of call.
 * \param all Whether to handle all call and not only the one specified
 * by ID.
 *
 * \return Error code
 *
 * \ingroup Call
 */
GSM_Error GSM_AnswerCall(GSM_StateMachine * s, int ID, gboolean all);

/**
 * Deny current incoming call.
 *
 * \param s State machine pointer.
 * \param ID ID of call.
 * \param all Whether to handle all call and not only the one specified
 * by ID.
 *
 * \return Error code
 *
 * \ingroup Call
 */
GSM_Error GSM_CancelCall(GSM_StateMachine * s, int ID, gboolean all);

/**
 * Holds call.
 *
 * \param s State machine pointer.
 * \param ID ID of call.
 *
 * \return Error code
 *
 * \ingroup Call
 */
GSM_Error GSM_HoldCall(GSM_StateMachine * s, int ID);

/**
 * Unholds call.
 *
 * \param s State machine pointer.
 * \param ID ID of call.
 *
 * \return Error code
 *
 * \ingroup Call
 */
GSM_Error GSM_UnholdCall(GSM_StateMachine * s, int ID);

/**
 * Initiates conference call.
 *
 * \param s State machine pointer.
 * \param ID ID of call.
 *
 * \return Error code
 *
 * \ingroup Call
 */
GSM_Error GSM_ConferenceCall(GSM_StateMachine * s, int ID);

/**
 * Splits call.
 *
 * \param s State machine pointer.
 * \param ID ID of call.
 *
 * \return Error code
 *
 * \ingroup Call
 */
GSM_Error GSM_SplitCall(GSM_StateMachine * s, int ID);

/**
 * Transfers call.
 *
 * \param s State machine pointer.
 * \param ID ID of call.
 * \param next Switches next call and ignores ID.
 *
 * \return Error code
 *
 * \ingroup Call
 */
GSM_Error GSM_TransferCall(GSM_StateMachine * s, int ID, gboolean next);

/**
 * Switches call.
 *
 * \param s State machine pointer.
 * \param ID ID of call.
 * \param next Switches next call and ignores ID.
 *
 * \return Error code
 *
 * \ingroup Call
 */
GSM_Error GSM_SwitchCall(GSM_StateMachine * s, int ID, gboolean next);

/**
 * Gets call diverts.
 *
 * \param s State machine pointer.
 * \param request Which diverts to get.
 * \param result Storage for diversions information.
 *
 * \return Error code
 *
 * \ingroup Divert
 */
GSM_Error GSM_GetCallDivert(GSM_StateMachine *s, GSM_CallDivert *request, GSM_MultiCallDivert *result);

/**
 * Sets call diverts.
 *
 * \param s State machine pointer.
 * \param divert Diversions information to set.
 *
 * \return Error code
 *
 * \ingroup Divert
 */
GSM_Error GSM_SetCallDivert(GSM_StateMachine * s, GSM_CallDivert * divert);

/**
 * Cancels all diverts.
 *
 * \param s State machine pointer.
 *
 * \return Error code
 *
 * \ingroup Divert
 */
GSM_Error GSM_CancelAllDiverts(GSM_StateMachine * s);

/**
 * Activates/deactivates noticing about incoming calls.
 *
 * \param s State machine pointer.
 * \param enable Whether to enable notifications.
 *
 * \return Error code
 *
 * \ingroup Call
 */
GSM_Error GSM_SetIncomingCall(GSM_StateMachine * s, gboolean enable);

/**
 * Sends DTMF (Dual Tone Multi Frequency) tone.
 *
 * \param s State machine pointer.
 * \param sequence Sequence to press.
 *
 * \return Error code
 *
 * \ingroup Call
 */
GSM_Error GSM_SendDTMF(GSM_StateMachine * s, char *sequence);

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
