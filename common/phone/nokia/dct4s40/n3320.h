/* (c) 2004 by Marcin Wiacek */

#ifndef n3320_h
#define n3320_h

#include "../ncommon.h"

#ifndef GSM_USED_MBUS2
#  define GSM_USED_MBUS2
#endif
#ifndef GSM_USED_FBUS2
#  define GSM_USED_FBUS2
#endif

typedef struct {
	int				LastCalendarYear;
	int				LastCalendarPos;
	GSM_NOKIACalToDoLocations	LastCalendar;
	int				FirstCalendarPos;
	unsigned char			CalendarIcons[10];
	GSM_CalendarNoteType		CalendarIconsTypes[10];
	int				CalendarIconsNum;
} GSM_Phone_N3320Data;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
