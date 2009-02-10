/* (c) 2003 by Marcin Wiacek */

#ifndef n3650_h
#define n3650_h

#include "../../../gsmstate.h"
#include "../../../service/gsmmisc.h"

#ifndef GSM_USED_MBUS2
#  define GSM_USED_MBUS2
#endif
#ifndef GSM_USED_FBUS2
#  define GSM_USED_FBUS2
#endif

typedef struct {
	int				FileLev;
	int				FilesLocationsUsed;
	int				FilesLocationsCurrent;
	GSM_File			*Files[10000];
	int				FileEntries;
} GSM_Phone_N3650Data;

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
