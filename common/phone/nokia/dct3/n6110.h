/* (c) 2002-2003 by Marcin Wiacek */

#ifndef n6110_h
#define n6110_h

#include "../../../config.h"
#include "../../../service/sms/gsmsms.h"
#include "dct3comm.h"

typedef struct {
#ifndef ENABLE_LGPL
	unsigned char			MagicBytes[4];
#endif
	int				LastCalendarPos;
	DCT3_WAPSettings_Locations	WAPLocations;

	GSM_SMSMemoryStatus		LastSMSStatus;
	int				LastSMSRead;

	int				PhoneLanguage;
} GSM_Phone_N6110Data;

typedef enum {
	N6110_Auto = 1,
	N6110_Europe
} N6110_Language;

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
