/* (c) 2006 by  Michal Cihar */

#ifndef atobex_functions_h
#define atobex_functions_h

#include "../../gsmstate.h"

/* Reply functions which are hooked into AT module */

extern GSM_Error ATOBEX_ReplyGetDateLocale(GSM_Protocol_Message msg, GSM_StateMachine *s);
extern GSM_Error ATOBEX_ReplyGetTimeLocale(GSM_Protocol_Message msg, GSM_StateMachine *s);
extern GSM_Error ATOBEX_ReplyGetFileSystemStatus(GSM_Protocol_Message msg, GSM_StateMachine *s);
extern GSM_Error ATOBEX_ReplyGetBatteryCharge(GSM_Protocol_Message msg, GSM_StateMachine *s);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
