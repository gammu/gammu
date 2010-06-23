
#ifndef n6510_h
#define n6510_h

#include "../ncommon.h"
#include "../../../service/gsmsms.h"

typedef struct {
	int				LastCalendarYear;
	int				LastCalendarPos;

	GSM_NOKIASMSFolder		LastSMSFolder;
	GSM_SMSFolders			LastSMSFolders;
	GSM_NOKIACalendarLocations	LastCalendar;
	int				FirstCalendarPos;
	GSM_NOKIAToDoLocations		LastToDo;
	unsigned char			RingtoneID;	/* When set with preview */
} GSM_Phone_N6510Data;

#ifndef GSM_USED_FBUS2
#  define GSM_USED_FBUS2
#endif
#ifndef GSM_USED_MBUS2
#  define GSM_USED_MBUS2
#endif
#ifndef GSM_USED_IRDA
#  define GSM_USED_IRDA
#endif
#ifndef GSM_USED_DLR3AT
#  define GSM_USED_DLR3AT
#endif
#ifndef GSM_USED_DLR3BLUETOOTH
#  define GSM_USED_DLR3BLUETOOTH
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
