/* (c) 2002-2004 by Marcin Wiacek & Michal Cihar */

#include <string.h>
#include <ctype.h>

#include "../../phone/nokia/nfunc.h"
#include "../../phone/nokia/dct3/n7110.h"
#include "../../misc/coding/coding.h"
#include "../../misc/cfg.h"
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
	int i;

	i=0;
	while (backup->PhonePhonebook[i]!=NULL) {
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

GSM_Error GSM_SaveBackupFile(char *FileName, GSM_Backup *backup, bool UseUnicode)
{
	if (mystrcasestr(FileName,".lmb")) {
		return SaveLMB(FileName,backup);
	} else if (mystrcasestr(FileName,".vcs")) {
		return SaveVCalendar(FileName,backup);
	} else if (mystrcasestr(FileName,".vcf")) {
		return SaveVCard(FileName,backup);
	} else if (mystrcasestr(FileName,".ldif")) {
		return SaveLDIF(FileName,backup);
	} else if (mystrcasestr(FileName,".ics")) {
		return SaveICS(FileName,backup);
	} else {
		return SaveBackup(FileName,backup, UseUnicode);
	}
}

GSM_Error GSM_ReadBackupFile(char *FileName, GSM_Backup *backup)
{
	FILE		*file;
	unsigned char	buffer[300];

	file = fopen(FileName, "rb");
	if (file == NULL) return ERR_CANTOPENFILE;
	fread(buffer, 1, 9, file); /* Read the header of the file. */
	fclose(file);

	GSM_ClearBackup(backup);

	/* Attempt to identify filetype */
	if (mystrcasestr(FileName,".vcs")) {
		return LoadVCalendar(FileName,backup);
	} else if (mystrcasestr(FileName,".vcf")) {
		return LoadVCard(FileName,backup);
	} else if (mystrcasestr(FileName,".ldif")) {
		return LoadLDIF(FileName,backup);
	} else if (mystrcasestr(FileName,".ics")) {
		return LoadICS(FileName,backup);
	} else if (memcmp(buffer, "LMB ",4)==0) {
		return LoadLMB(FileName,backup);
	} else if (buffer[0] == 0xFE && buffer[1] == 0xFF) {
		return LoadBackup(FileName,backup,true);
	} else if (buffer[0] == 0xFF && buffer[1] == 0xFE) {
		return LoadBackup(FileName,backup,true);
	} else {
		return LoadBackup(FileName,backup,false);
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

void GSM_GetBackupFormatFeatures(char *FileName, GSM_Backup_Info *info)
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

	if (strstr(FileName,".lmb")) {
		info->PhonePhonebook 	= true;
		info->SIMPhonebook 	= true;
		info->CallerLogos 	= true;
		info->StartupLogo 	= true;
	} else if (strstr(FileName,".vcs")) {
		info->ToDo		= true;
		info->Calendar 		= true;
	} else if (strstr(FileName,".vcf")) {
		info->PhonePhonebook	= true;
	} else if (strstr(FileName,".ics")) {
		info->ToDo		= true;
		info->Calendar 		= true;
	} else if (strstr(FileName,".ldif")) {
		info->PhonePhonebook	= true;
	} else {
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
	}
}

void GSM_GetBackupFileFeatures(char *FileName, GSM_Backup_Info *info, GSM_Backup *backup)
{
	GSM_GetBackupFormatFeatures(FileName, info);

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
