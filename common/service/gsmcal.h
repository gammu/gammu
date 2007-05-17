/* (c) 2002-2004 by Marcin Wiacek, 2005-2007 by Michal Cihar */

/**
 * @file gsmcal.h
 * @author Michal Čihař <michal@cihar.com>
 * @author Marcin Wiacek
 * @author Frederick Ros
 * @date 2002-2007
 */
/**
 * @defgroup Calendar Calendar parsing and encoding
 *
 * This module implements calendar related opreations.
 *
 * @see http://www.imc.org/pdi/
 * @{
 */

#ifndef __gsm_cal_h
#define __gsm_cal_h

#include "../gsmcomon.h"

/* ---------------------------- calendar ----------------------------------- */

#include <gammu-calendar.h>

/**
 * Format of vCalendar export.
 */
typedef enum {
	Nokia_VCalendar = 1, /**< vCalendar specially hacked for Nokia */
	Siemens_VCalendar, /**< vCalendar specially hacked for Nokia */
	SonyEricsson_VCalendar, /**< Standard vCalendar (which works for Sony-Ericsson phones) */
	Mozilla_iCalendar /**< iCalendar as compatible with Mozilla */
} GSM_VCalendarVersion;

typedef enum {
	TRANSL_TO_GSM = 1,
	TRANSL_TO_VCAL,
} GSM_CatTranslation;

GSM_Error GSM_EncodeVCALENDAR(char *Buffer, int *Length, GSM_CalendarEntry *note, bool header, GSM_VCalendarVersion Version);

bool IsCalendarNoteFromThePast(GSM_CalendarEntry *note);



void GSM_GetCalendarRecurranceRepeat(unsigned char *rec, unsigned char *endday, GSM_CalendarEntry *entry);
void GSM_SetCalendarRecurranceRepeat(unsigned char *rec, unsigned char *endday, GSM_CalendarEntry *entry);

/* ------------------------------ to-do ------------------------------------ */


void GSM_ToDoFindDefaultTextTimeAlarmCompleted(GSM_ToDoEntry *entry, int *Text, int *Alarm, int *Completed, int *EndTime, int *Phone);

typedef enum {
	Nokia_VToDo = 1,
	SonyEricsson_VToDo,
	Mozilla_VToDo
} GSM_VToDoVersion;

GSM_Error GSM_EncodeVTODO(char *Buffer, int *Length, GSM_ToDoEntry *note, bool header, GSM_VToDoVersion Version);


/* --------------------------- note ---------------------------------------- */


GSM_Error GSM_EncodeVNTFile(unsigned char *Buffer, int *Length, GSM_NoteEntry *Note);

/* --------------------------- alarm --------------------------------------- */

/* --------------------------- calendar & todo ----------------------------- */

GSM_Error GSM_DecodeVCALENDAR_VTODO(unsigned char *Buffer, int *Pos, GSM_CalendarEntry *Calendar, GSM_ToDoEntry *ToDo, GSM_VCalendarVersion CalVer, GSM_VToDoVersion ToDoVer);

#endif
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
