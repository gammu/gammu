/* (c) 2003 by Marcin Wiacek */

#ifndef __gsm_backvcs_h
#define __gsm_backvcs_h

#include "backgen.h"

#ifdef GSM_ENABLE_BACKUP
GSM_Error SaveVCalendar(char *FileName, GSM_Backup *backup);
GSM_Error LoadVCalendar(char *FileName, GSM_Backup *backup);
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
