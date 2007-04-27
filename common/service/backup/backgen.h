/* (c) 2002-2004 by Marcin Wiacek */
/** \file backgen.h
 * \defgroup Backup Backups
 *
 * Functions and structures for backups
 *
 * @author Marcin Wiacek
 * @author Michal Čihař
 * @date 2004-2007
 *
 * @{
 */

#ifndef __gsm_backgen_h
#define __gsm_backgen_h

#include "../../config.h"
#include "../../misc/misc.h"
#include "../gsmpbk.h"
#include "../gsmcal.h"
#include "../gsmlogo.h"
#include "../gsmring.h"
#include "../gsmdata.h"
#include "../gsmprof.h"
#include "../gsmmisc.h"
#include "../sms/gsmsms.h"

#define GSM_BACKUP_MAX_PHONEPHONEBOOK 	1001
#define GSM_BACKUP_MAX_SIMPHONEBOOK 	251
#define GSM_BACKUP_MAX_CALLER	 	20
#define GSM_BACKUP_MAX_SMSC		10
#define GSM_BACKUP_MAX_WAPBOOKMARK	40
#define GSM_BACKUP_MAX_WAPSETTINGS	30
#define GSM_BACKUP_MAX_MMSSETTINGS	30
#define GSM_BACKUP_MAX_SYNCMLSETTINGS	30
#define GSM_BACKUP_MAX_CHATSETTINGS	30
#define GSM_BACKUP_MAX_RINGTONES	30
#define GSM_BACKUP_MAX_PROFILES		10
#define GSM_BACKUP_MAX_FMSTATIONS	20
#define GSM_BACKUP_MAX_GPRSPOINT	10
#define GSM_BACKUP_MAX_NOTE		50	/* FIXME */

/**
 * Backup data.
 */
typedef struct {
	char			IMEI		[MAX_IMEI_LENGTH]; /**< IMEI of phone which has been backed up */
	char			Model		[MAX_MODEL_LENGTH+MAX_VERSION_LENGTH]; /**< Model of phone which has been backed up */
	char			Creator		[80]; /**< Name of program which created backup */
	GSM_DateTime		DateTime; /**< Timestamp of backup */
	bool			DateTimeAvailable; /**< Whether timestamp is present */
	char			MD5Original	[100]; /**< Original MD5 of backup from file */
	char			MD5Calculated	[100]; /**< Calculated MD5 of backup*/
	GSM_MemoryEntry		*PhonePhonebook	[GSM_BACKUP_MAX_PHONEPHONEBOOK + 1]; /**< Phone phonebook */
	GSM_MemoryEntry		*SIMPhonebook	[GSM_BACKUP_MAX_SIMPHONEBOOK + 1]; /**< SIM phonebook */
	GSM_CalendarEntry	*Calendar	[GSM_MAXCALENDARTODONOTES + 1]; /**< Calendar */
	GSM_Bitmap		*CallerLogos	[GSM_BACKUP_MAX_CALLER + 1]; /**< Caller logos */
	GSM_SMSC		*SMSC		[GSM_BACKUP_MAX_SMSC + 1]; /**< SMS configuration */
	GSM_WAPBookmark		*WAPBookmark	[GSM_BACKUP_MAX_WAPBOOKMARK + 1]; /**< WAP bookmarks */
	GSM_MultiWAPSettings	*WAPSettings	[GSM_BACKUP_MAX_WAPSETTINGS + 1]; /**< WAP settings */
	GSM_MultiWAPSettings	*MMSSettings	[GSM_BACKUP_MAX_MMSSETTINGS + 1]; /**< MMS settings */
	GSM_SyncMLSettings	*SyncMLSettings [GSM_BACKUP_MAX_SYNCMLSETTINGS + 1]; /**< SyncMC settings */
	GSM_ChatSettings	*ChatSettings 	[GSM_BACKUP_MAX_CHATSETTINGS + 1]; /**< Chat settings */
	GSM_Ringtone		*Ringtone	[GSM_BACKUP_MAX_RINGTONES + 1]; /**< Ringtones */
	GSM_ToDoEntry		*ToDo		[GSM_MAXCALENDARTODONOTES + 1]; /**< To do tasks */
	GSM_Profile		*Profiles	[GSM_BACKUP_MAX_PROFILES + 1]; /**< Progiles */
 	GSM_FMStation		*FMStation	[GSM_BACKUP_MAX_FMSTATIONS +1]; /**< FM stations */
	GSM_GPRSAccessPoint	*GPRSPoint	[GSM_BACKUP_MAX_GPRSPOINT + 1]; /**< GPRS configurations */
	GSM_NoteEntry		*Note		[GSM_BACKUP_MAX_NOTE + 1]; /**< Notes */
	GSM_Bitmap		*StartupLogo; /**< Statup logo */
	GSM_Bitmap		*OperatorLogo; /**< Operator logo */
} GSM_Backup;

#define GSM_BACKUP_MAX_SMS	1000

/**
 * SMS backup data.
 */
typedef struct {
	GSM_SMSMessage		*SMS[GSM_BACKUP_MAX_SMS + 1]; /**< SMS messages */
} GSM_SMS_Backup;

/**
 * Reads backup file.
 *
 * @param FileName file name
 * @param backup structure where backup will be stored
 */
extern GSM_Error GSM_ReadSMSBackupFile(char *FileName, GSM_SMS_Backup *backup);
/**
 * Adds data to bacup file.
 *
 * @param FileName file name
 * @param backup structure holding backup data
 */
extern GSM_Error GSM_AddSMSBackupFile (char *FileName, GSM_SMS_Backup *backup);

#endif
/*@}*/

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
