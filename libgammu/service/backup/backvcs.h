/* (c) 2003 by Marcin Wiacek */

#ifndef __gsm_backvcs_h
#define __gsm_backvcs_h

#include <gammu-error.h>
#include <gammu-backup.h>

#ifdef GSM_ENABLE_BACKUP
GSM_Error SaveVCalendar(const char *FileName, GSM_Backup *backup);
GSM_Error LoadVCalendar(const char *FileName, GSM_Backup *backup);
GSM_Error LoadVCalendarPrivate(const char *FileName, GSM_Backup *backup, GSM_VCalendarVersion CalVer, GSM_VToDoVersion ToDoVer);
#endif

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
