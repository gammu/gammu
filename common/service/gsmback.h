#ifndef __gsm_backup_h
#define __gsm_backup_h

#include "../misc/misc.h"
#include "gsmpbk.h"
#include "gsmcal.h"
#include "gsmlogo.h"
#include "gsmring.h"
#include "gsmsms.h"
#include "gsmwap.h"
#include "gsmprof.h"

#define GSM_BACKUP_MAX_PHONEPHONEBOOK 	501
#define GSM_BACKUP_MAX_SIMPHONEBOOK 	251
#define GSM_BACKUP_MAX_CALENDAR 	151
#define GSM_BACKUP_MAX_CALLER	 	6
#define GSM_BACKUP_MAX_SMSC		10
#define GSM_BACKUP_MAX_WAPBOOKMARK	40
#define GSM_BACKUP_MAX_WAPSETTINGS	6
#define GSM_BACKUP_MAX_RINGTONES	15
#define GSM_BACKUP_MAX_TODO		100
#define GSM_BACKUP_MAX_PROFILES		10

typedef struct {
	char			IMEI		[50];
	char			Model		[50];
	char			DateTime	[200];
	GSM_PhonebookEntry	*PhonePhonebook	[GSM_BACKUP_MAX_PHONEPHONEBOOK + 1];
	GSM_PhonebookEntry	*SIMPhonebook	[GSM_BACKUP_MAX_SIMPHONEBOOK + 1];
	GSM_CalendarNote	*Calendar	[GSM_BACKUP_MAX_CALENDAR + 1];
	GSM_Bitmap		*CallerLogos	[GSM_BACKUP_MAX_CALLER + 1];
	GSM_SMSC		*SMSC		[GSM_BACKUP_MAX_SMSC + 1];
	GSM_WAPBookmark		*WAPBookmark	[GSM_BACKUP_MAX_WAPBOOKMARK + 1];
	GSM_MultiWAPSettings	*WAPSettings	[GSM_BACKUP_MAX_WAPSETTINGS + 1];
	GSM_Ringtone		*Ringtone	[GSM_BACKUP_MAX_RINGTONES + 1];
	GSM_TODO		*ToDo		[GSM_BACKUP_MAX_TODO + 1];
	GSM_Profile		*Profiles	[GSM_BACKUP_MAX_PROFILES + 1];
	GSM_Bitmap		*StartupLogo;
	GSM_Bitmap		*OperatorLogo;
} GSM_Backup;

GSM_Error GSM_SaveBackupFile(char *FileName, GSM_Backup *backup);
GSM_Error GSM_ReadBackupFile(char *FileName, GSM_Backup *backup);

void GSM_ClearBackup(GSM_Backup *backup);
void GSM_FreeBackup(GSM_Backup *backup);

typedef struct {
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
	bool Ringtone;
	bool StartupLogo;
	bool OperatorLogo;
} GSM_Backup_Info;

void GSM_GetBackupFeatures(char *FileName, GSM_Backup_Info *backup);

#define GSM_BACKUP_MAX_SMS	500

typedef struct {
	GSM_SMSMessage		*SMS[GSM_BACKUP_MAX_SMS];
} GSM_SMS_Backup;

GSM_Error GSM_ReadSMSBackupFile(char *FileName, GSM_SMS_Backup *backup);
GSM_Error GSM_SaveSMSBackupFile(char *FileName, GSM_SMS_Backup *backup);

#endif	/* __gsm_backup_h */
