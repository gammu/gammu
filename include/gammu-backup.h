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
#include <gammu-config.h>

#ifdef GSM_ENABLE_BACKUP

/**
 * Maximal number of SMSes in backup.
 *
 * \todo This should not be hardcoded.
 */
#define GSM_BACKUP_MAX_SMS 100000

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
GSM_Error GSM_ReadSMSBackupFile(const char *FileName, GSM_SMS_Backup * backup);

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
GSM_Error GSM_AddSMSBackupFile(const char *FileName, GSM_SMS_Backup * backup);

/**
 * Clears SMS backup structure
 *
 * \ingroup Backup
 *
 * \param backup structure where backup data will be stored
 */
void GSM_ClearSMSBackup(GSM_SMS_Backup * backup);

/**
 * Deallocates all members of SMS backup structure
 *
 * \ingroup Backup
 *
 * \param backup structure where backup data will be stored
 */
void GSM_FreeSMSBackup(GSM_SMS_Backup * backup);

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
#define GSM_BACKUP_MAX_CALLER 101
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
	/**
	 * IMEI of phone which has been backed up
	 */
	char IMEI[GSM_MAX_IMEI_LENGTH];
	/**
	 * Model of phone which has been backed up
	 */
	char Model[GSM_MAX_MODEL_LENGTH + GSM_MAX_VERSION_LENGTH];
	/**
	 * Name of program which created backup
	 */
	char Creator[80];
	/**
	 * Timestamp of backup
	 */
	GSM_DateTime DateTime;
	/**
	 * Whether timestamp is present
	 */
	gboolean DateTimeAvailable;
	/**
	 * Original MD5 of backup from file
	 */
	char MD5Original[100];
	/**
	 * Calculated MD5 of backup
	 */
	char MD5Calculated[100];
	/**
	 * Phone phonebook
	 */
	GSM_MemoryEntry *PhonePhonebook[GSM_BACKUP_MAX_PHONEPHONEBOOK + 1];
	/**
	 * SIM phonebook
	 */
	GSM_MemoryEntry *SIMPhonebook[GSM_BACKUP_MAX_SIMPHONEBOOK + 1];
	/**
	 * Calendar
	 */
	GSM_CalendarEntry *Calendar[GSM_MAXCALENDARTODONOTES + 1];
	/**
	 * Caller logos
	 */
	GSM_Bitmap *CallerLogos[GSM_BACKUP_MAX_CALLER + 1];
	/**
	 * SMS configuration
	 */
	GSM_SMSC *SMSC[GSM_BACKUP_MAX_SMSC + 1];
	/**
	 * WAP bookmarks
	 */
	GSM_WAPBookmark *WAPBookmark[GSM_BACKUP_MAX_WAPBOOKMARK + 1];
	/**
	 * WAP settings
	 */
	GSM_MultiWAPSettings *WAPSettings[GSM_BACKUP_MAX_WAPSETTINGS + 1];
	/**
	 * MMS settings
	 */
	GSM_MultiWAPSettings *MMSSettings[GSM_BACKUP_MAX_MMSSETTINGS + 1];
	/**
	 * SyncMC settings
	 */
	GSM_SyncMLSettings *SyncMLSettings[GSM_BACKUP_MAX_SYNCMLSETTINGS + 1];
	/**
	 * Chat settings
	 */
	GSM_ChatSettings *ChatSettings[GSM_BACKUP_MAX_CHATSETTINGS + 1];
	/**
	 * Ringtones
	 */
	GSM_Ringtone *Ringtone[GSM_BACKUP_MAX_RINGTONES + 1];
	/**
	 * To do tasks
	 */
	GSM_ToDoEntry *ToDo[GSM_MAXCALENDARTODONOTES + 1];
	/**
	 * Progiles
	 */
	GSM_Profile *Profiles[GSM_BACKUP_MAX_PROFILES + 1];
	/**
	 * FM stations
	 */
	GSM_FMStation *FMStation[GSM_BACKUP_MAX_FMSTATIONS + 1];
	/**
	 * GPRS configurations
	 */
	GSM_GPRSAccessPoint *GPRSPoint[GSM_BACKUP_MAX_GPRSPOINT + 1];
	/**
	 * Notes
	 */
	GSM_NoteEntry *Note[GSM_BACKUP_MAX_NOTE + 1];
	/**
	 * Statup logo
	 */
	GSM_Bitmap *StartupLogo;
	/**
	 * Operator logo
	 */
	GSM_Bitmap *OperatorLogo;
} GSM_Backup;

/**
 * Backup data.
 *
 * \ingroup Backup
 */
typedef enum {
	/**
	 * Compatibility with old gboolean used instead of format.
	 *
	 * File type is guessed for extension, non unicode format used
	 * for Gammu backup.
	 */
	GSM_Backup_Auto = 0,
	/**
	 * Compatibility with old gboolean used instead of format.
	 *
	 * File type is guessed for extension, unicode format used
	 * for Gammu backup.
	 */
	GSM_Backup_AutoUnicode = 1,
	/**
	 * LMB format, compatible with Logo manager, can store
	 * phonebooks and logos.
	 */
	GSM_Backup_LMB,
	/**
	 * vCalendar standard, can store todo and calendar entries.
	 */
	GSM_Backup_VCalendar,
	/**
	 * vCard standard, can store phone phonebook entries.
	 */
	GSM_Backup_VCard,
	/**
	 * LDIF (LDAP Data Interchange Format), can store phone
	 * phonebook entries.
	 */
	GSM_Backup_LDIF,
	/**
	 * iCalendar standard, can store todo and calendar entries.
	 */
	GSM_Backup_ICS,
	/**
	 * Gammu own format can store almost anything from phone.
	 *
	 * This is ASCII version of the format, Unicode strings are HEX
	 * encoded. Use GSM_Backup_GammuUCS2 instead if possible.
	 */
	GSM_Backup_Gammu,
	/**
	 * Gammu own format can store almost anything from phone.
	 *
	 * This is UCS2-BE version of the format.
	 */
	GSM_Backup_GammuUCS2,
	/**
	 * vNote standard, can store phone notes.
	 */
	GSM_Backup_VNote,
} GSM_BackupFormat;

/**
 * Save backup file.
 *
 * \ingroup Backup
 *
 * \param FileName Name of file (format is detected from it).
 * \param Backup structure holding backup data
 * \param Format Backup format.
 *
 * \return Error code
 */
GSM_Error GSM_SaveBackupFile(char *FileName, GSM_Backup * Backup,
			     GSM_BackupFormat Format);

/**
 * Guesses backup format based on filename.
 *
 * \ingroup Backup
 *
 * \param FileName Name of backup filename.
 * \param UseUnicode Whether to prefer unicode variant when guessing.
 *
 * \return Backup format on success -1 on error.
 */
GSM_BackupFormat GSM_GuessBackupFormat(const char *FileName,
				       const gboolean UseUnicode);

/**
 * Reads data from backup file.
 *
 * \ingroup Backup
 *
 * \param FileName Name of file (format is detected from it).
 * \param backup structure where backup data will be stored
 * \param Format Format of backup. For Gammu backups, unicode subformats
 * are ignored.
 *
 * \return Error code
 */
GSM_Error GSM_ReadBackupFile(const char *FileName, GSM_Backup * backup,
			     GSM_BackupFormat Format);

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
	gboolean UseUnicode;
	gboolean IMEI;
	gboolean Model;
	gboolean DateTime;
	gboolean ToDo;
	gboolean PhonePhonebook;
	gboolean SIMPhonebook;
	gboolean Calendar;
	gboolean CallerLogos;
	gboolean SMSC;
	gboolean WAPBookmark;
	gboolean Profiles;
	gboolean WAPSettings;
	gboolean MMSSettings;
	gboolean SyncMLSettings;
	gboolean ChatSettings;
	gboolean Ringtone;
	gboolean StartupLogo;
	gboolean OperatorLogo;
	gboolean FMStation;
	gboolean GPRSPoint;
	gboolean Note;
} GSM_Backup_Info;

/**
 * Gets information about format features.
 *
 * \param Format Format of backup.
 * \param info Output information about backup features.
 *
 * \ingroup Backup
 */
void GSM_GetBackupFormatFeatures(GSM_BackupFormat Format,
				 GSM_Backup_Info * info);

/**
 * Gets information about backup data features (resp. which data it contains).
 *
 * \param Format Format of backup.
 * \param info Output information about backup features.
 * \param backup Backup data to chech.
 *
 * \ingroup Backup
 */
void GSM_GetBackupFileFeatures(GSM_BackupFormat Format, GSM_Backup_Info * info,
			       GSM_Backup * backup);

#endif

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
