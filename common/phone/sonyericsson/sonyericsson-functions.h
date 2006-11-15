/* (c) 2006 by  Michal Cihar */

#ifndef sonyericsson_functions_h
#define sonyericsson_functions_h

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "../../service/sms/gsmsms.h"

/* Reply functions which are hooked into AT module */

extern GSM_Error SONYERICSSON_ReplyGetDateLocale(GSM_Protocol_Message msg, GSM_StateMachine *s);
extern GSM_Error SONYERICSSON_ReplyGetTimeLocale(GSM_Protocol_Message msg, GSM_StateMachine *s);
extern GSM_Error SONYERICSSON_ReplyGetFileSystemStatus(GSM_Protocol_Message msg, GSM_StateMachine *s);
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
