#ifndef __gsm_backupvcs_h
#define __gsm_backupvcs_h

#include "backgen.h"

GSM_Error SaveVCalendar(char *FileName, GSM_Backup *backup);
GSM_Error LoadVCalendar(char *FileName, GSM_Backup *backup);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
