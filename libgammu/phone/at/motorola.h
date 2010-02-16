/* © 2007 - 2009 Michal Čihař */

/**
 * @file motorola.h
 * @author Michal Čihař
 */
/**
 * @ingroup Phone
 * @{
 */
/**
 * @addtogroup ATPhone
 * @{
 */

#ifndef atgen_motorola_h
#define atgen_motorola_h

#include <gammu-config.h>
#include "../../protocol/protocol.h"

#ifdef GSM_ENABLE_ATGEN

/**
 * Switches to correct mode to execute command.
 *
 * \param s State machine data.
 * \param command Command which should be checked.
 *
 * \return Error code.
 */
GSM_Error MOTOROLA_SetMode(GSM_StateMachine *s, const char *command);

/**
 * Reply handler for AT+MODE command.
 */
GSM_Error MOTOROLA_SetModeReply(GSM_Protocol_Message msg UNUSED, GSM_StateMachine *s);

/**
 * Catches +MBAN: reply and sets Mode according to it.
 */
GSM_Error MOTOROLA_Banner(GSM_Protocol_Message msg, GSM_StateMachine *s);

/**
 * Parses memory entry.
 */
GSM_Error MOTOROLA_ReplyGetMemory(GSM_Protocol_Message msg, GSM_StateMachine *s);

GSM_Error MOTOROLA_ParseCalendarSimple(GSM_StateMachine *s, const char *line);

GSM_Error MOTOROLA_ReplyGetMemoryInfo(GSM_Protocol_Message msg, GSM_StateMachine *s);

GSM_Error MOTOROLA_ReplyGetCalendar(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error MOTOROLA_GetNextCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note, gboolean start);
GSM_Error MOTOROLA_ReplyGetCalendarStatus(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error MOTOROLA_GetCalendarStatus(GSM_StateMachine *s, GSM_CalendarStatus *Status);
GSM_Error MOTOROLA_GetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note);
GSM_Error MOTOROLA_ReplySetCalendar(GSM_Protocol_Message msg, GSM_StateMachine *s);
GSM_Error MOTOROLA_DelCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note);
GSM_Error MOTOROLA_SetCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note);
GSM_Error MOTOROLA_AddCalendar(GSM_StateMachine *s, GSM_CalendarEntry *Note);
GSM_Error MOTOROLA_ReplyGetMPBRMemoryInfo(GSM_Protocol_Message msg, GSM_StateMachine *s);
#endif
#endif

/*@}*/
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
