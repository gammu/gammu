/* (c) 2003 by Marcin Wiacek */

#ifndef n3320_h
#define n3320_h

#include "../../../gsmcomon.h"
#include "../../../gsmstate.h"
#include "../../../service/gsmmisc.h"
#include "../../../service/sms/gsmsms.h"

#ifndef GSM_USED_MBUS2
#  define GSM_USED_MBUS2
#endif
#ifndef GSM_USED_FBUS2
#  define GSM_USED_FBUS2
#endif

typedef struct {
	int				FileEntries;
} GSM_Phone_N3320Data;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
