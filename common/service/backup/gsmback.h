/* (c) 2003-2004 by Marcin Wiacek */

#ifndef __gsm_back_h
#define __gsm_back_h

#include "backgen.h"

#ifdef GSM_ENABLE_BACKUP

GSM_Error GSM_SaveBackupFile(char *FileName, GSM_Backup *backup, bool UseUnicode);
GSM_Error GSM_ReadBackupFile(char *FileName, GSM_Backup *backup);

void GSM_ClearBackup (GSM_Backup *backup);
void GSM_FreeBackup  (GSM_Backup *backup);

typedef struct {
	bool UseUnicode;

	bool IMEI;
	bool Model;
	bool DateTime;
	bool ToDo;
	bool PhonePhonebook;
	bool SIMPhonebook;
	bool Calendar;
	bool CallerLogos;
	bool SMSC;
	bool WAPBookmark;
	bool Profiles;
	bool WAPSettings;
	bool MMSSettings;
	bool SyncMLSettings;
	bool ChatSettings;
	bool Ringtone;
	bool StartupLogo;
	bool OperatorLogo;
 	bool FMStation;
	bool GPRSPoint;
	bool Note;
} GSM_Backup_Info;

void GSM_GetBackupFormatFeatures(char *FileName, GSM_Backup_Info *info);
void GSM_GetBackupFileFeatures  (char *FileName, GSM_Backup_Info *info, GSM_Backup *backup);

#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
