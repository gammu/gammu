/* (c) 2003 by Marcin Wiacek */

#ifndef gnapgen_h
#define gnapgen_h

#include "../../gsmcomon.h"
#include "../../gsmstate.h"
#include "../../service/gsmmisc.h"
#include "../../service/sms/gsmsms.h"

#ifndef GSM_USED_BLUEGNAPBUS
#  define GSM_USED_BLUEGNAPBUS
#endif
#ifndef GSM_USED_IRDAGNAPBUS
#  define GSM_USED_IRDAGNAPBUS
#endif

typedef struct {
	int fake;
} GSM_Phone_GNAPGENData;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
