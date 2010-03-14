
#ifndef phone_nokia2_h
#define phone_nokia2_h

#include "ncommon.h"
#include "../../gsmcomon.h"
#include "../../gsmstate.h"

#ifdef DEBUG
GSM_Error N71_65_ReplyGetNextCalendar2		(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error N71_65_ReplyGetCalendarNotePos1	(GSM_Protocol_Message msg, GSM_StateMachine *s,int *FirstCalendarPos);
GSM_Error N71_65_ReplyAddCalendar1		(GSM_Protocol_Message msg, GSM_StateMachine *s);

GSM_Error N71_65_GetNextCalendar2		(GSM_StateMachine *s, GSM_CalendarEntry *Note, bool start, int *LastCalendarYear, int *LastCalendarPos);
GSM_Error N71_65_AddCalendar1			(GSM_StateMachine *s, GSM_CalendarEntry *Note, int *FirstCalendarPos, bool Past);
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
