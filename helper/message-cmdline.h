#ifndef __helper_message_cmdline_h__
#define __helper_message_cmdline_h__

#include <gammu-message.h>
#include <gammu-statemachine.h>

/**
 * Type of parameters to process.
 */
typedef enum {
    SMS_Send,
    SMS_Save,
    SMS_SendSaved,
    SMS_Display,
    SMS_SMSD
} GSM_Message_Type;

/**
 * Creates SMS message based on command line parameters.
 *
 * \param type Determines what kind of message will be created (parameters
 *		are different for each kind). This can be updated to
 *		SMS_SendSaved for SMS_Save.
 * \param typearg Determinets where in argv parameters for SMS start (eg.
 *		where type of message is placed).
 */
GSM_Error CreateMessage(GSM_Message_Type *type, GSM_MultiSMSMessage *sms, int argc, int typearg, char *argv[], GSM_StateMachine *sm);
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=78:
 */
