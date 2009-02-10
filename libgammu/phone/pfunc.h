/* (c) 2002-2003 by Marcin Wiacek */

/**
 * @file pfunc.h
 * @author Michal Čihař
 * @author Marcin Wiacek
 */
/**
 * @defgroup Phone Phone drivers
 *
 * Phone drivers
 *
 * @{
 */
/**
 * @defgroup GenericPhone Generic phone functions
 *
 * @{
 */
#ifndef phone_common2_h
#define phone_common2_h

#include <gammu-statemachine.h>
#include <gammu-message.h>
#include "../protocol/protocol.h"

GSM_Error PHONE_GetSMSFolders		(GSM_StateMachine *s, GSM_SMSFolders *folders);

/**
 * Parses string firmware number into numeric.
 */
void 	  GSM_CreateFirmwareNumber	(GSM_StateMachine *s);

/**
 * Generic function for terminating phone connection.
 * Can be used if phone does not require any special actions.
 */
GSM_Error PHONE_Terminate		(GSM_StateMachine *s);

/**
 * Generic reply function when no reply is requested.
 */
GSM_Error NoneReply(GSM_Protocol_Message msg, GSM_StateMachine *s);

#endif
/*@}*/
/*@}*/
/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
