
#ifndef n6110_h
#define n6110_h

#include "../../../service/gsmsms.h"
#include "dct3comm.h"

typedef struct {
	unsigned char			MagicBytes[4];
	int				LastCalendarPos;
	DCT3_WAPSettings_Locations	WAPLocations;

	GSM_SMSMemoryStatus		LastSMSStatus;
	int				LastSMSRead;
} GSM_Phone_N6110Data;

#ifndef GSM_USED_FBUS2
#  define GSM_USED_FBUS2
#endif
#ifndef GSM_USED_MBUS2
#  define GSM_USED_MBUS2
#endif
#ifndef GSM_USED_IRDA
#  define GSM_USED_IRDA
#endif
#ifndef GSM_USED_INFRARED
#  define GSM_USED_INFRARED
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
