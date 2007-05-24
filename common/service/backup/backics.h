/* (c) 2003 by Marcin Wiacek */

#ifndef __gsm_backics_h
#define __gsm_backics_h

#include <gammu-error.h>
#include <gammu-backup.h>

#ifdef GSM_ENABLE_BACKUP
GSM_Error SaveICS(char *FileName, GSM_Backup *backup);
GSM_Error LoadICS(char *FileName, GSM_Backup *backup);
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
