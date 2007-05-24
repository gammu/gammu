/**
 * \file gammu-callback.h
 * \author Michal Čihař
 * 
 * Callback handlers.
 */
#ifndef __gammu_callback_h
#define __gammu_callback_h

/**
 * \defgroup Callback Callback
 * Generic state machine callback layer.
 */

#include <gammu-types.h>
#include <gammu-message.h>
#include <gammu-call.h>

/**
 * Callback for incoming calls.
 *
 * \ingroup Callback
 */
typedef void (*IncomingCallCallback) (GSM_StateMachine * s, GSM_Call call);

/**
 * Callback for incoming SMS.
 *
 * \ingroup Callback
 */
typedef void (*IncomingSMSCallback) (GSM_StateMachine * s, GSM_SMSMessage sms);

/**
 * Callback for incoming cell broadcast.
 *
 * \ingroup Callback
 */
typedef void (*IncomingCBCallback) (GSM_StateMachine * s, GSM_CBMessage cb);

/**
 * Callback for icoming USSD.
 *
 * \ingroup Callback
 */
typedef void (*IncomingUSSDCallback) (GSM_StateMachine * s,
				      GSM_USSDMessage ussd);

/**
 * Callback for sending SMS.
 *
 * \ingroup Callback
 */
typedef void (*SendSMSStatusCallback) (GSM_StateMachine * s, int status,
				       int MessageReference);

/**
 * Sets callback for incoming calls.
 *
 * \param s State machine.
 * \param callback Pointer to callback function.
 *
 * \ingroup Callback
 */
void GSM_SetIncomingCallCallback(GSM_StateMachine * s,
				 IncomingCallCallback callback);

/**
 * Sets callback for incoming SMSes.
 *
 * \param s State machine.
 * \param callback Pointer to callback function.
 *
 * \ingroup Callback
 */
void GSM_SetIncomingSMSCallback(GSM_StateMachine * s,
				IncomingSMSCallback callback);

/**
 * Sets callback for incoming CB.
 *
 * \param s State machine.
 * \param callback Pointer to callback function.
 *
 * \ingroup Callback
 */
void GSM_SetIncomingCBCallback(GSM_StateMachine * s,
			       IncomingCBCallback callback);

/**
 * Sets callback for incoming USSD.
 *
 * \param s State machine.
 * \param callback Pointer to callback function.
 *
 * \ingroup Callback
 */
void GSM_SetIncomingUSSDCallback(GSM_StateMachine * s,
				 IncomingUSSDCallback callback);

/**
 * Sets callback for sending SMS.
 *
 * \param s State machine.
 * \param callback Pointer to callback function.
 *
 * \ingroup Callback
 */
void GSM_SetSendSMSStatusCallback(GSM_StateMachine * s,
				  SendSMSStatusCallback callback);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
