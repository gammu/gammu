#ifndef siemens_h
#define siemens_h

#include "../../gsmstate.h"

GSM_Error SIEMENS_ReplyGetBitmap	(GSM_Protocol_Message, GSM_StateMachine *);
GSM_Error SIEMENS_ReplySetBitmap	(GSM_Protocol_Message, GSM_StateMachine *);
GSM_Error SIEMENS_ReplyGetRingtone	(GSM_Protocol_Message, GSM_StateMachine *);
GSM_Error SIEMENS_ReplySetRingtone	(GSM_Protocol_Message, GSM_StateMachine *);
GSM_Error SIEMENS_GetBitmap		(GSM_StateMachine *, GSM_Bitmap *);
GSM_Error SIEMENS_SetBitmap		(GSM_StateMachine *, GSM_Bitmap *);
GSM_Error SIEMENS_GetRingtone		(GSM_StateMachine *, GSM_Ringtone *, bool);
GSM_Error SIEMENS_SetRingtone		(GSM_StateMachine *, GSM_Ringtone *, int *);

GSM_Error SIEMENS_ReplyGetNextCalendar	(GSM_Protocol_Message, GSM_StateMachine *);
GSM_Error SIEMENS_ReplyAddCalendarNote	(GSM_Protocol_Message, GSM_StateMachine *);
GSM_Error SIEMENS_ReplyDelCalendarNote	(GSM_Protocol_Message, GSM_StateMachine *);
GSM_Error SIEMENS_GetNextCalendar	(GSM_StateMachine *, GSM_CalendarEntry *, bool);
GSM_Error SIEMENS_SetCalendarNote	(GSM_StateMachine *, GSM_CalendarEntry *);
GSM_Error SIEMENS_AddCalendarNote	(GSM_StateMachine *, GSM_CalendarEntry *);
GSM_Error SIEMENS_DelCalendarNote	(GSM_StateMachine *, GSM_CalendarEntry *);
GSM_Error SIEMENS_GetCalendar		(GSM_StateMachine *, GSM_CalendarEntry *);

GSM_Error SIEMENS_ReplyGetMemory	(GSM_Protocol_Message, GSM_StateMachine *);
GSM_Error SIEMENS_ReplyGetMemoryInfo(GSM_Protocol_Message msg, GSM_StateMachine *s);

#endif
