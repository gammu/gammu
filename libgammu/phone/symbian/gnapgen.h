/* (c) 2003 by Marcin Wiacek
 * (C) 2007 Matthias Lechner <matthias@lmme.de> */

#ifndef gnapgen_h
#define gnapgen_h

#include "../../gsmstate.h"
#include "../../service/gsmmisc.h"

#ifndef GSM_USED_BLUEGNAPBUS
#  define GSM_USED_BLUEGNAPBUS
#endif
#ifndef GSM_USED_IRDAGNAPBUS
#  define GSM_USED_IRDAGNAPBUS
#endif

typedef struct {
	int				byte1;
	int				byte2;
	int				byte3;
	int				byte4;
} TUint32;

typedef struct {
	int				LastCalendarYear;
	int				LastCalendarPos;
	GSM_NOKIACalToDoLocations	LastCalendar;
	int				FirstCalendarPos;
	unsigned char			CalendarIcons[10];
	GSM_CalendarNoteType		CalendarIconsTypes[10];
	int				CalendarIconsNum;

	gboolean				LastContactArrived;

	GSM_SMSFolders			CurrentSMSFolders;
	int				SMSFolderID[100];
	int				CurrentFolderNumber;

	TUint32				SMSIDs[5000];
	int				SMSCount;
	int				CurrentSMSNumber;

	GSM_NOKIACalToDoLocations	LastToDo;

	GSM_NOKIACalToDoLocations	LastNote;

	int				GNAPPLETVer;
} GSM_Phone_GNAPGENData;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
