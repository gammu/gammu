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

#include "../service/sms/gsmsms.h"

extern GSM_SMSMessageLayout PHONE_SMSSubmit;
extern GSM_SMSMessageLayout PHONE_SMSDeliver;
extern GSM_SMSMessageLayout PHONE_SMSStatusReport;

GSM_Error PHONE_GetSMSFolders		(GSM_StateMachine *s, GSM_SMSFolders *folders);

/**
 * Parses string firmware number into numeric.
 */
void 	  GSM_CreateFirmwareNumber	(GSM_StateMachine *s);

/**
 * Encodes SMS frame according to layout.
 */
GSM_Error PHONE_EncodeSMSFrame		(GSM_StateMachine *s, GSM_SMSMessage *SMS, unsigned char *buffer, GSM_SMSMessageLayout Layout, int *length, bool clear);

/**
 * Generic function for terminating phone connection.
 * Can be used if phone does not require any special actions.
 */
GSM_Error PHONE_Terminate		(GSM_StateMachine *s);

/**
 * Play one note using state machine interface.
 */
GSM_Error PHONE_RTTLPlayOneNote		(GSM_StateMachine *s, GSM_RingNote note, bool first);

/**
 * Makes phone beek using state machine interface.
 */
GSM_Error PHONE_Beep			(GSM_StateMachine *s);

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
