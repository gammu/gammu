/* (c) 2002-2003 by Marcin Wiacek */

#ifndef n7110_h
#define n7110_h

#include "../ncommon.h"
#include "dct3comm.h"

typedef struct {
	int				LastCalendarYear;
	int				LastCalendarPos;
	GSM_NOKIACalToDoLocations	LastCalendar;
	int				FirstCalendarPos;

	GSM_NOKIASMSFolder		LastSMSFolder;
	GSM_SMSFolders			LastSMSFolders;
	GSM_NOKIASMSFolder		LastPictureImageFolder;

	DCT3_WAPSettings_Locations	WAPLocations;
} GSM_Phone_N7110Data;

#ifndef GSM_USED_MBUS2
#  define GSM_USED_MBUS2
#endif
#ifndef GSM_USED_FBUS2
#  define GSM_USED_FBUS2
#endif
#ifndef GSM_USED_FBUS2DLR3
#  define GSM_USED_FBUS2DLR3
#endif
#ifndef GSM_USED_FBUS2BLUE
#  define GSM_USED_FBUS2BLUE
#endif
#ifndef GSM_USED_IRDAPHONET
#  define GSM_USED_IRDAPHONET
#endif
#ifndef GSM_USED_BLUEFBUS2
#  define GSM_USED_BLUEFBUS2
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
