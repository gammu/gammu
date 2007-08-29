/* (c) 2002-2003 by Marcin Wiacek */

#ifndef n6110_h
#define n6110_h

#include <gammu-config.h>
#include <gammu-message.h>
#include "dct3comm.h"

typedef enum {
	N6110_Auto = 1,
	N6110_Europe
} N6110_Language;

typedef struct {
	unsigned char			MagicBytes[4];
	int				LastCalendarPos;
	DCT3_WAPSettings_Locations	WAPLocations;

	GSM_SMSMemoryStatus		LastSMSStatus;
	int				LastSMSRead;

	N6110_Language			PhoneLanguage;
} GSM_Phone_N6110Data;

#ifndef GSM_USED_MBUS2
#  define GSM_USED_MBUS2
#endif
#ifndef GSM_USED_FBUS2
#  define GSM_USED_FBUS2
#endif
#ifndef GSM_USED_FBUS2IRDA
#  define GSM_USED_FBUS2IRDA
#endif
#ifndef GSM_USED_IRDAPHONET
#  define GSM_USED_IRDAPHONET
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
