/**
 * \file gammu-backup.h
 * \author Michal Čihař
 * 
 * Backup data and functions.
 */
#ifndef __gammu_backup_h
#define __gammu_backup_h

/**
 * \defgroup Backup Backup
 * Backups manipulations.
 */

#include <gammu-message.h>
#include <gammu-memory.h>
#include <gammu-calendar.h>
#include <gammu-bitmap.h>
#include <gammu-wap.h>
#include <gammu-settings.h>
#include <gammu-ringtone.h>

/**
 * Maximal number of SMSes in backup.
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_SMS 1000

/**
 * SMS backup data.
 *
 * \ingroup Backup
 */
typedef struct {
	/**
	 * List of SMS messages.
	 */
	GSM_SMSMessage *SMS[GSM_BACKUP_MAX_SMS + 1];
} GSM_SMS_Backup;

/**
 * Reads SMS backup file.
 *
 * \ingroup Backup
 *
 * \param FileName file name
 * \param backup structure where backup will be stored
 *
 * \return Error code
 */
extern GSM_Error GSM_ReadSMSBackupFile(char *FileName, GSM_SMS_Backup * backup);

/**
 * Adds data to SMS backup file.
 *
 * \ingroup Backup
 *
 * \param FileName file name
 * \param backup structure holding backup data
 *
 * \return Error code
 */
extern GSM_Error GSM_AddSMSBackupFile(char *FileName, GSM_SMS_Backup * backup);

/**
 * Maximal number of phonebook entries in backup.
 *
 * \ingroup Backup
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_PHONEPHONEBOOK 1001
/**
 * Maximal number of SIM entries in backup.
 *
 * \ingroup Backup
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_SIMPHONEBOOK 251
/**
 * Maximal number of caller groups in backup.
 *
 * \ingroup Backup
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_CALLER 20
/**
 * Maximal number of SMSC in backup.
 *
 * \ingroup Backup
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_SMSC 10
/**
 * Maximal number of WAP bookmarks in backup.
 *
 * \ingroup Backup
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_WAPBOOKMARK 40
/**
 * Maximal number of WAP settings in backup.
 *
 * \ingroup Backup
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_WAPSETTINGS 30
/**
 * Maximal number of MMS settings in backup.
 *
 * \ingroup Backup
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_MMSSETTINGS 30
/**
 * Maximal number of SyncML settings in backup.
 *
 * \ingroup Backup
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_SYNCMLSETTINGS 30
/**
 * Maximal number of chat settings in backup.
 *
 * \ingroup Backup
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_CHATSETTINGS 30
/**
 * Maximal number of ringtones in backup.
 *
 * \ingroup Backup
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_RINGTONES 30
/**
 * Maximal number of profiles in backup.
 *
 * \ingroup Backup
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_PROFILES 10
/**
 * Maximal number of FM stations in backup.
 *
 * \ingroup Backup
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_FMSTATIONS 20
/**
 * Maximal number of GPRS points in backup.
 *
 * \ingroup Backup
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_GPRSPOINT 10
/**
 * Maximal number of notes in backup.
 *
 * \ingroup Backup
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_NOTE 50

/**
 * Backup data.
 *
 * \ingroup Backup
 */
typedef struct {
	char IMEI[MAX_IMEI_LENGTH]; /**< IMEI of phone which has been backed up */
	char Model[MAX_MODEL_LENGTH + MAX_VERSION_LENGTH]; /**< Model of phone which has been backed up */
	char Creator[80]; /**< Name of program which created backup */
	GSM_DateTime DateTime; /**< Timestamp of backup */
	bool DateTimeAvailable; /**< Whether timestamp is present */
	char MD5Original[100]; /**< Original MD5 of backup from file */
	char MD5Calculated[100]; /**< Calculated MD5 of backup*/
	GSM_MemoryEntry *PhonePhonebook[GSM_BACKUP_MAX_PHONEPHONEBOOK + 1]; /**< Phone phonebook */
	GSM_MemoryEntry *SIMPhonebook[GSM_BACKUP_MAX_SIMPHONEBOOK + 1]; /**< SIM phonebook */
	GSM_CalendarEntry *Calendar[GSM_MAXCALENDARTODONOTES + 1]; /**< Calendar */
	GSM_Bitmap *CallerLogos[GSM_BACKUP_MAX_CALLER + 1]; /**< Caller logos */
	GSM_SMSC *SMSC[GSM_BACKUP_MAX_SMSC + 1]; /**< SMS configuration */
	GSM_WAPBookmark *WAPBookmark[GSM_BACKUP_MAX_WAPBOOKMARK + 1]; /**< WAP bookmarks */
	GSM_MultiWAPSettings *WAPSettings[GSM_BACKUP_MAX_WAPSETTINGS + 1]; /**< WAP settings */
	GSM_MultiWAPSettings *MMSSettings[GSM_BACKUP_MAX_MMSSETTINGS + 1]; /**< MMS settings */
	GSM_SyncMLSettings *SyncMLSettings[GSM_BACKUP_MAX_SYNCMLSETTINGS + 1]; /**< SyncMC settings */
	GSM_ChatSettings *ChatSettings[GSM_BACKUP_MAX_CHATSETTINGS + 1]; /**< Chat settings */
	GSM_Ringtone *Ringtone[GSM_BACKUP_MAX_RINGTONES + 1]; /**< Ringtones */
	GSM_ToDoEntry *ToDo[GSM_MAXCALENDARTODONOTES + 1]; /**< To do tasks */
	GSM_Profile *Profiles[GSM_BACKUP_MAX_PROFILES + 1]; /**< Progiles */
	GSM_FMStation *FMStation[GSM_BACKUP_MAX_FMSTATIONS + 1]; /**< FM stations */
	GSM_GPRSAccessPoint *GPRSPoint[GSM_BACKUP_MAX_GPRSPOINT + 1]; /**< GPRS configurations */
	GSM_NoteEntry *Note[GSM_BACKUP_MAX_NOTE + 1]; /**< Notes */
	GSM_Bitmap *StartupLogo; /**< Statup logo */
	GSM_Bitmap *OperatorLogo; /**< Operator logo */
} GSM_Backup;

/**
 * Save backup file.
 *
 * \ingroup Backup
 *
 * \param FileName Name of file (format is detected from it).
 * \param backup structure holding backup data
 * \param UseUnicode Whether to use unicode (if available).
 *
 * \return Error code
 */
GSM_Error GSM_SaveBackupFile(char *FileName, GSM_Backup * backup,
			     bool UseUnicode);
/**
 * Reads data from backup file.
 *
 * \ingroup Backup
 *
 * \param FileName Name of file (format is detected from it).
 * \param backup structure where backup data will be stored
 *
 * \return Error code
 */
GSM_Error GSM_ReadBackupFile(char *FileName, GSM_Backup * backup);

/**
 * Clears backup structure
 *
 * \ingroup Backup
 *
 * \param backup structure where backup data will be stored
 */
void GSM_ClearBackup(GSM_Backup * backup);

/**
 * Deallocates all members of backup structure
 *
 * \ingroup Backup
 *
 * \param backup structure where backup data will be stored
 */
void GSM_FreeBackup(GSM_Backup * backup);

/**
 * Information about supported backup features.
 *
 * \ingroup Backup
 */
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

/**
 * Gets information about format features.
 *
 * \param FileName Name of file (format is detected from it).
 * \param info Output information about backup features.
 *
 * \ingroup Backup
 */
void GSM_GetBackupFormatFeatures(char *FileName, GSM_Backup_Info * info);

/**
 * Gets information about backup data features (resp. which data it contains).
 *
 * \param FileName Name of file (format is detected from it).
 * \param info Output information about backup features.
 * \param backup Backup data to chech.
 *
 * \ingroup Backup
 */
void GSM_GetBackupFileFeatures(char *FileName, GSM_Backup_Info * info,
			       GSM_Backup * backup);
#endif
/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
