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

/* ---------------------------- calendar ----------------------------------- */

#include <gammu-calendar.h>

/**
 * Time Units.
 */
typedef enum {
	GSM_TimeUnit_Unknown = 0,
	GSM_TimeUnit_Days,
	GSM_TimeUnit_Hours,
	GSM_TimeUnit_Minutes,
	GSM_TimeUnit_Seconds
} GSM_TimeUnit;

typedef enum {
	TRANSL_TO_GSM = 1,
	TRANSL_TO_VCAL,
} GSM_CatTranslation;

void GSM_GetCalendarRecurranceRepeat(GSM_Debug_Info *di, unsigned char *rec, unsigned char *endday, GSM_CalendarEntry *entry);
void GSM_SetCalendarRecurranceRepeat(GSM_Debug_Info *di, unsigned char *rec, unsigned char *endday, GSM_CalendarEntry *entry);

/* ------------------------------ to-do ------------------------------------ */

void GSM_ToDoFindDefaultTextTimeAlarmCompleted(GSM_ToDoEntry *entry, int *Text, int *Alarm, int *Completed, int *EndTime, int *Phone);


#endif
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
