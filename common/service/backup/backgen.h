/* (c) 2002-2004 by Marcin Wiacek */

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

#define GSM_BACKUP_MAX_PHONEPHONEBOOK 	501
#define GSM_BACKUP_MAX_SIMPHONEBOOK 	251
#define GSM_BACKUP_MAX_CALLER	 	6
#define GSM_BACKUP_MAX_SMSC		10
#define GSM_BACKUP_MAX_WAPBOOKMARK	40
#define GSM_BACKUP_MAX_WAPSETTINGS	30
#define GSM_BACKUP_MAX_MMSSETTINGS	30
#define GSM_BACKUP_MAX_SYNCMLSETTINGS	10
#define GSM_BACKUP_MAX_CHATSETTINGS	10
#define GSM_BACKUP_MAX_RINGTONES	30
#define GSM_BACKUP_MAX_PROFILES		10
#define GSM_BACKUP_MAX_FMSTATIONS	20
#define GSM_BACKUP_MAX_GPRSPOINT	10
#define GSM_BACKUP_MAX_NOTE		10	/* FIXME */

typedef struct {
	char			IMEI		[MAX_IMEI_LENGTH];
	char			Model		[MAX_MODEL_LENGTH+MAX_VERSION_LENGTH];
	char			Creator		[80];
	GSM_DateTime		DateTime;
	bool			DateTimeAvailable;
	char			MD5Original	[100];
	char			MD5Calculated	[100];
	GSM_MemoryEntry		*PhonePhonebook	[GSM_BACKUP_MAX_PHONEPHONEBOOK + 1];
	GSM_MemoryEntry		*SIMPhonebook	[GSM_BACKUP_MAX_SIMPHONEBOOK + 1];
	GSM_CalendarEntry	*Calendar	[GSM_MAXCALENDARTODONOTES + 1];
	GSM_Bitmap		*CallerLogos	[GSM_BACKUP_MAX_CALLER + 1];
	GSM_SMSC		*SMSC		[GSM_BACKUP_MAX_SMSC + 1];
	GSM_WAPBookmark		*WAPBookmark	[GSM_BACKUP_MAX_WAPBOOKMARK + 1];
	GSM_MultiWAPSettings	*WAPSettings	[GSM_BACKUP_MAX_WAPSETTINGS + 1];
	GSM_MultiWAPSettings	*MMSSettings	[GSM_BACKUP_MAX_MMSSETTINGS + 1];
	GSM_SyncMLSettings	*SyncMLSettings [GSM_BACKUP_MAX_SYNCMLSETTINGS + 1];
	GSM_ChatSettings	*ChatSettings 	[GSM_BACKUP_MAX_CHATSETTINGS + 1];
	GSM_Ringtone		*Ringtone	[GSM_BACKUP_MAX_RINGTONES + 1];
	GSM_ToDoEntry		*ToDo		[GSM_MAXCALENDARTODONOTES + 1];
	GSM_Profile		*Profiles	[GSM_BACKUP_MAX_PROFILES + 1];
 	GSM_FMStation		*FMStation	[GSM_BACKUP_MAX_FMSTATIONS +1];
	GSM_GPRSAccessPoint	*GPRSPoint	[GSM_BACKUP_MAX_GPRSPOINT + 1];
	GSM_NoteEntry		*Note		[GSM_BACKUP_MAX_NOTE + 1];
	GSM_Bitmap		*StartupLogo;
	GSM_Bitmap		*OperatorLogo;
} GSM_Backup;

#define GSM_BACKUP_MAX_SMS	500

typedef struct {
	GSM_SMSMessage		*SMS[GSM_BACKUP_MAX_SMS];
} GSM_SMS_Backup;

extern GSM_Error GSM_ReadSMSBackupFile(char *FileName, GSM_SMS_Backup *backup);
extern GSM_Error GSM_AddSMSBackupFile (char *FileName, GSM_SMS_Backup *backup);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
