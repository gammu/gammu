
#ifndef n6510_h
#define n6510_h

#include "../ncommon.h"
#include "../../../service/gsmsms.h"

typedef struct {
	int				LastCalendarYear;
	int				LastCalendarPos;
	GSM_NOKIACalToDoLocations	LastCalendar;
	int				FirstCalendarPos;
	unsigned char			CalendarIcons[10];
	GSM_CalendarNoteType		CalendarIconsTypes[10];
	int				CalendarIconsNum;

	GSM_NOKIASMSFolder		LastSMSFolder;
	GSM_SMSFolders			LastSMSFolders;

	GSM_NOKIACalToDoLocations	LastToDo;

	unsigned char			RingtoneID;	/* When set with preview */

	int				FilesLocations[500];
	int				FilesParents[500];
	int				FilesLevels[500];
	int				FilesLocationsUsed;
	int				FilesLocationsCurrent;

	unsigned char			FMStatus[4000];
	int				FMStatusLength;
} GSM_Phone_N6510Data;

#ifndef GSM_USED_MBUS2
#  define GSM_USED_MBUS2
#endif
#ifndef GSM_USED_FBUS2
#  define GSM_USED_FBUS2
#endif
#ifndef GSM_USED_FBUS2DLR3
#  define GSM_USED_FBUS2DLR3
#endif
#ifndef GSM_USED_PHONETBLUE
#  define GSM_USED_PHONETBLUE
#endif
#ifndef GSM_USED_IRDAPHONET
#  define GSM_USED_IRDAPHONET
#endif
#ifndef GSM_USED_BLUEPHONET
#  define GSM_USED_BLUEPHONET
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
