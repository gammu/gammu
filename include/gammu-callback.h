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
typedef void (*IncomingCallCallback) (GSM_StateMachine * s, GSM_Call *call,
				      void *user_data);

/**
 * Callback for incoming SMS.
 *
 * \ingroup Callback
 */
typedef void (*IncomingSMSCallback) (GSM_StateMachine * s, GSM_SMSMessage *sms,
				     void *user_data);

/**
 * Callback for incoming cell broadcast.
 *
 * \ingroup Callback
 */
typedef void (*IncomingCBCallback) (GSM_StateMachine * s, GSM_CBMessage *cb,
				    void *user_data);

/**
 * Callback for icoming USSD.
 *
 * \ingroup Callback
 */
typedef void (*IncomingUSSDCallback) (GSM_StateMachine * s,
				      GSM_USSDMessage *ussd, void *user_data);

/**
 * Callback for sending SMS.
 *
 * \ingroup Callback
 */
typedef void (*SendSMSStatusCallback) (GSM_StateMachine * s, int status,
				       int MessageReference, void *user_data);

/**
 * Sets callback for incoming calls.
 *
 * \param s State machine.
 * \param callback Pointer to callback function.
 * \param user_data Second parameter which will be passed to callback.
 *
 * \ingroup Callback
 */
void GSM_SetIncomingCallCallback(GSM_StateMachine * s,
				 IncomingCallCallback callback,
				 void *user_data);

/**
 * Sets callback for incoming SMSes.
 *
 * \param s State machine.
 * \param callback Pointer to callback function.
 * \param user_data Second parameter which will be passed to callback.
 *
 * \ingroup Callback
 */
void GSM_SetIncomingSMSCallback(GSM_StateMachine * s,
				IncomingSMSCallback callback, void *user_data);

/**
 * Sets callback for incoming CB.
 *
 * \param s State machine.
 * \param callback Pointer to callback function.
 * \param user_data Second parameter which will be passed to callback.
 *
 * \ingroup Callback
 */
void GSM_SetIncomingCBCallback(GSM_StateMachine * s,
			       IncomingCBCallback callback, void *user_data);

/**
 * Sets callback for incoming USSD.
 *
 * \param s State machine.
 * \param callback Pointer to callback function.
 * \param user_data Second parameter which will be passed to callback.
 *
 * \ingroup Callback
 */
void GSM_SetIncomingUSSDCallback(GSM_StateMachine * s,
				 IncomingUSSDCallback callback,
				 void *user_data);

/**
 * Sets callback for sending SMS.
 *
 * \param s State machine.
 * \param callback Pointer to callback function.
 * \param user_data Second parameter which will be passed to callback.
 *
 * \ingroup Callback
 */
void GSM_SetSendSMSStatusCallback(GSM_StateMachine * s,
				  SendSMSStatusCallback callback,
				  void *user_data);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
