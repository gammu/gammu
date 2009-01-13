/* (c) 2002-2004 by Marcin Wiacek & Michal Cihar */

#define _GNU_SOURCE
#include <string.h>
#include <ctype.h>

#include "../../phone/nokia/nfunc.h"
#include "../../phone/nokia/dct3/n7110.h"
#include "../../misc/coding/coding.h"
#include "../gsmlogo.h"
#include "../gsmmisc.h"
#include "../gsmcal.h"
#include "gsmback.h"
#include "backtext.h"
#include "backldif.h"
#include "backlmb.h"
#include "backvcs.h"
#include "backvcf.h"
#include "backics.h"

#ifdef GSM_ENABLE_BACKUP

void GSM_FreeBackup(GSM_Backup *backup)
{
	int i, j;

	i=0;
	while (backup->PhonePhonebook[i]!=NULL) {
		for (j = 0; j < backup->PhonePhonebook[i]->EntriesNum; j++) {
			if (backup->PhonePhonebook[i]->Entries[j].EntryType == PBK_Photo) {
				free(backup->PhonePhonebook[i]->Entries[j].Picture.Buffer);
			}
		}

		free(backup->PhonePhonebook[i]);
		backup->PhonePhonebook[i] = NULL;
		i++;
	}
	i=0;
	while (backup->SIMPhonebook[i]!=NULL) {
		free(backup->SIMPhonebook[i]);
		backup->SIMPhonebook[i] = NULL;
		i++;
	}
	i=0;
	while (backup->Calendar[i]!=NULL) {
		free(backup->Calendar[i]);
		backup->Calendar[i] = NULL;
		i++;
	}
	i=0;
	while (backup->CallerLogos[i]!=NULL) {
		free(backup->CallerLogos[i]);
		backup->CallerLogos[i] = NULL;
		i++;
	}
	i=0;
	while (backup->SMSC[i]!=NULL) {
		free(backup->SMSC[i]);
		backup->SMSC[i] = NULL;
		i++;
	}
	i=0;
	while (backup->WAPBookmark[i]!=NULL) {
		free(backup->WAPBookmark[i]);
		backup->WAPBookmark[i] = NULL;
		i++;
	}
	i=0;
	while (backup->WAPSettings[i]!=NULL) {
		free(backup->WAPSettings[i]);
		backup->WAPSettings[i] = NULL;
		i++;
	}
	i=0;
	while (backup->MMSSettings[i]!=NULL) {
		free(backup->MMSSettings[i]);
		backup->MMSSettings[i] = NULL;
		i++;
	}
	i=0;
	while (backup->SyncMLSettings[i]!=NULL) {
		free(backup->SyncMLSettings[i]);
		backup->SyncMLSettings[i] = NULL;
		i++;
	}
	i=0;
	while (backup->ChatSettings[i]!=NULL) {
		free(backup->ChatSettings[i]);
		backup->ChatSettings[i] = NULL;
		i++;
	}
	i=0;
	while (backup->Ringtone[i]!=NULL) {
		free(backup->Ringtone[i]);
		backup->Ringtone[i] = NULL;
		i++;
	}
	i=0;
	while (backup->ToDo[i]!=NULL) {
		free(backup->ToDo[i]);
		backup->ToDo[i] = NULL;
		i++;
	}
	i=0;
	while (backup->Profiles[i]!=NULL) {
		free(backup->Profiles[i]);
		backup->Profiles[i] = NULL;
		i++;
	}
	i=0;
	while (backup->FMStation[i]!=NULL) {
		free(backup->FMStation[i]);
		backup->FMStation[i] = NULL;
		i++;
	}
	if (backup->StartupLogo!=NULL) {
		free(backup->StartupLogo);
		backup->StartupLogo = NULL;
	}
	if (backup->OperatorLogo!=NULL) {
		free(backup->OperatorLogo);
		backup->OperatorLogo = NULL;
	}
	i=0;
	while (backup->GPRSPoint[i]!=NULL) {
		free(backup->GPRSPoint[i]);
		backup->GPRSPoint[i] = NULL;
		i++;
	}
	i=0;
	while (backup->Note[i]!=NULL) {
		free(backup->Note[i]);
		backup->Note[i] = NULL;
		i++;
	}
}

GSM_BackupFormat GSM_GuessBackupFormat(const char *FileName, const bool UseUnicode) {
	GSM_BackupFormat Format = -1;

	if (strcasestr(FileName,".lmb")) {
		Format = GSM_Backup_LMB;
	} else if (strcasestr(FileName,".vcs")) {
		Format = GSM_Backup_VCalendar;
	} else if (strcasestr(FileName,".vcf")) {
		Format = GSM_Backup_VCard;
	} else if (strcasestr(FileName,".ldif")) {
		Format = GSM_Backup_LDIF;
	} else if (strcasestr(FileName,".ics")) {
		Format = GSM_Backup_ICS;
	} else {
		if (UseUnicode) {
			Format = GSM_Backup_GammuUCS2;
		} else {
			Format = GSM_Backup_Gammu;
		}
	}
	return Format;
}

GSM_Error GSM_SaveBackupFile(char *FileName, GSM_Backup *backup, GSM_BackupFormat Format)
{
	if (Format == GSM_Backup_Auto || Format == GSM_Backup_AutoUnicode) {
		Format = GSM_GuessBackupFormat(FileName, Format == GSM_Backup_AutoUnicode);
	}

	switch (Format) {
		case GSM_Backup_LMB:
			return SaveLMB(FileName,backup);
		case GSM_Backup_VCalendar:
			return SaveVCalendar(FileName,backup);
		case GSM_Backup_VCard:
			return SaveVCard(FileName,backup);
		case GSM_Backup_LDIF:
			return SaveLDIF(FileName,backup);
		case GSM_Backup_ICS:
			return SaveICS(FileName,backup);
		case GSM_Backup_Gammu:
			return SaveBackup(FileName,backup, false);
		case GSM_Backup_GammuUCS2:
			return SaveBackup(FileName,backup, true);
		default:
			return ERR_FILENOTSUPPORTED;
	}
}

GSM_Error GSM_ReadBackupFile(char *FileName, GSM_Backup *backup, GSM_BackupFormat Format)
{
	GSM_ClearBackup(backup);

	if (Format == GSM_Backup_Auto || Format == GSM_Backup_AutoUnicode) {
		Format = GSM_GuessBackupFormat(FileName, Format == GSM_Backup_AutoUnicode);
	}

	switch (Format) {
		case GSM_Backup_LMB:
			return LoadLMB(FileName,backup);
		case GSM_Backup_VCalendar:
			return LoadVCalendar(FileName,backup);
		case GSM_Backup_VCard:
			return LoadVCard(FileName,backup);
		case GSM_Backup_LDIF:
			return LoadLDIF(FileName,backup);
		case GSM_Backup_ICS:
			return LoadICS(FileName,backup);
		case GSM_Backup_Gammu:
			return LoadBackup(FileName,backup);
		case GSM_Backup_GammuUCS2:
			return LoadBackup(FileName,backup);
		default:
			return ERR_FILENOTSUPPORTED;
	}
}

void GSM_ClearBackup(GSM_Backup *backup)
{
	backup->PhonePhonebook	[0] = NULL;
	backup->SIMPhonebook	[0] = NULL;
	backup->Calendar	[0] = NULL;
	backup->CallerLogos	[0] = NULL;
	backup->SMSC		[0] = NULL;
	backup->WAPBookmark	[0] = NULL;
	backup->WAPSettings	[0] = NULL;
	backup->MMSSettings	[0] = NULL;
	backup->SyncMLSettings	[0] = NULL;
	backup->ChatSettings	[0] = NULL;
	backup->Ringtone	[0] = NULL;
	backup->Profiles	[0] = NULL;
	backup->ToDo		[0] = NULL;
	backup->GPRSPoint	[0] = NULL;
	backup->FMStation	[0] = NULL;
	backup->Note		[0] = NULL;
	backup->StartupLogo	    = NULL;
	backup->OperatorLogo	    = NULL;

	backup->Creator		[0] = 0;
	backup->IMEI		[0] = 0;
	backup->Model		[0] = 0;
	backup->DateTimeAvailable   = false;
	backup->MD5Original	[0] = 0;
	backup->MD5Calculated	[0] = 0;
}

void GSM_GetBackupFormatFeatures(GSM_BackupFormat Format, GSM_Backup_Info *info)
{
	info->UseUnicode	= false;
	info->IMEI 		= false;
	info->Model 		= false;
	info->DateTime 		= false;
	info->PhonePhonebook 	= false;
	info->SIMPhonebook 	= false;
	info->ToDo		= false;
	info->Calendar 		= false;
	info->CallerLogos 	= false;
	info->SMSC 		= false;
	info->WAPBookmark 	= false;
	info->WAPSettings 	= false;
	info->MMSSettings 	= false;
	info->SyncMLSettings 	= false;
	info->ChatSettings 	= false;
	info->Ringtone 		= false;
	info->StartupLogo 	= false;
	info->OperatorLogo 	= false;
	info->Profiles 		= false;
	info->FMStation 	= false;
	info->GPRSPoint		= false;
	info->Note		= false;

	switch (Format) {
		case GSM_Backup_LMB:
			info->PhonePhonebook 	= true;
			info->SIMPhonebook 	= true;
			info->CallerLogos 	= true;
			info->StartupLogo 	= true;
			break;
		case GSM_Backup_VCalendar:
			info->ToDo		= true;
			info->Calendar 		= true;
			break;
		case GSM_Backup_VCard:
			info->PhonePhonebook	= true;
			break;
		case GSM_Backup_LDIF:
			info->PhonePhonebook	= true;
			break;
		case GSM_Backup_ICS:
			info->ToDo		= true;
			info->Calendar 		= true;
			break;
		case GSM_Backup_Gammu:
		case GSM_Backup_GammuUCS2:
			info->UseUnicode	= true;
			info->IMEI 		= true;
			info->Model 		= true;
			info->DateTime 		= true;
			info->PhonePhonebook 	= true;
			info->SIMPhonebook 	= true;
			info->ToDo		= true;
			info->Calendar 		= true;
			info->CallerLogos 	= true;
			info->SMSC 		= true;
			info->WAPBookmark 	= true;
			info->WAPSettings 	= true;
			info->MMSSettings 	= true;
			info->SyncMLSettings 	= true;
			info->ChatSettings 	= true;
			info->Ringtone 		= true;
			info->StartupLogo 	= true;
			info->OperatorLogo 	= true;
			info->Profiles 		= true;
			info->FMStation 	= true;
			info->GPRSPoint		= true;
			info->Note		= true;
			break;
		default:
			break;
	}
}

void GSM_GetBackupFileFeatures(GSM_BackupFormat Format, GSM_Backup_Info *info, GSM_Backup *backup)
{
	GSM_GetBackupFormatFeatures(Format, info);

	if (info->PhonePhonebook && backup->PhonePhonebook[0] == NULL) info->PhonePhonebook = false;
	if (info->SIMPhonebook   && backup->SIMPhonebook[0]   == NULL) info->SIMPhonebook   = false;
	if (info->Calendar	 && backup->Calendar[0]       == NULL) info->Calendar       = false;
	if (info->ToDo		 && backup->ToDo[0]  	      == NULL) info->ToDo	    = false;
	if (info->WAPBookmark	 && backup->WAPBookmark[0]    == NULL) info->WAPBookmark    = false;
	if (info->WAPSettings    && backup->WAPSettings[0]    == NULL) info->WAPSettings    = false;
	if (info->MMSSettings    && backup->MMSSettings[0]    == NULL) info->MMSSettings    = false;
	if (info->FMStation      && backup->FMStation[0]      == NULL) info->FMStation      = false;
	if (info->GPRSPoint      && backup->GPRSPoint[0]      == NULL) info->GPRSPoint      = false;
	if (info->Profiles       && backup->Profiles[0]       == NULL) info->Profiles       = false;
	/* .... */
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
