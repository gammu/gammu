/* (c) 2003 by Marcin Wiacek */

#ifndef mroutgen_h
#define mroutgen_h

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "../../service/gsmmisc.h"
#include "../../service/sms/gsmsms.h"

#ifndef GSM_USED_MROUTERBLUE
#  define GSM_USED_MROUTERBLUE
#endif
#ifndef GSM_USED_IRDAMROUTER
#  define GSM_USED_IRDAMROUTER
#endif
#ifndef GSM_USED_BLUEMROUTER
#  define GSM_USED_BLUEMROUTER
#endif

typedef struct {
	int fake;
} GSM_Phone_MROUTERGENData;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
