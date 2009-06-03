/* (c) 2003 by Marcin Wiacek */

#ifndef __gsm_backtext_h
#define __gsm_backtext_h

#include <gammu-error.h>
#include <gammu-types.h>
#include <gammu-backup.h>

#ifdef GSM_ENABLE_BACKUP
GSM_Error LoadBackup(char *FileName, GSM_Backup *backup);
GSM_Error SaveBackup(char *FileName, GSM_Backup *backup, gboolean UseUnicode);
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
