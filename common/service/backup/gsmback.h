#ifndef __gsm_backupgen_h
#define __gsm_backupgen_h

#include "backgen.h"

GSM_Error GSM_SaveBackupFile(char *FileName, GSM_Backup *backup, bool UseUnicode);
GSM_Error GSM_ReadBackupFile(char *FileName, GSM_Backup *backup);

void GSM_ClearBackup(GSM_Backup *backup);
void GSM_FreeBackup(GSM_Backup *backup);

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
	bool Ringtone;
	bool StartupLogo;
	bool OperatorLogo;
 	bool FMStation;
	bool GPRSPoint;
	bool Note;
} GSM_Backup_Info;

void GSM_GetBackupFeatures(char *FileName, GSM_Backup_Info *backup);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
