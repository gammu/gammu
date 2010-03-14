
#include <string.h>
#include <ctype.h>

#include "../../../cfg/config.h"
#include "../../phone/nokia/nfunc.h"
#include "../../phone/nokia/dct3/n7110.h"
#include "../../misc/coding/coding.h"
#include "../../misc/cfg.h"
#include "../gsmlogo.h"
#include "../gsmmisc.h"
#include "gsmback.h"
#include "backtext.h"
#include "backldif.h"
#include "backlmb.h"
#include "backvcs.h"
#include "backvcf.h"

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

void GSM_GetBackupFeatures(char *FileName, GSM_Backup_Info *backup)
{
	backup->UseUnicode	= false;
	backup->IMEI 		= false;
	backup->Model 		= false;
	backup->DateTime 	= false;
	backup->PhonePhonebook 	= false;
	backup->SIMPhonebook 	= false;
	backup->ToDo		= false;
	backup->Calendar 	= false;
	backup->CallerLogos 	= false;
	backup->SMSC 		= false;
	backup->WAPBookmark 	= false;
	backup->WAPSettings 	= false;
	backup->MMSSettings 	= false;
	backup->Ringtone 	= false;
	backup->StartupLogo 	= false;
	backup->OperatorLogo 	= false;
	backup->Profiles 	= false;
	backup->FMStation 	= false;
	backup->GPRSPoint	= false;
	backup->Note		= false;

	if (strstr(FileName,".lmb")) {
		backup->PhonePhonebook 	= true;
		backup->SIMPhonebook 	= true;
		backup->CallerLogos 	= true;
		backup->StartupLogo 	= true;
	} else if (strstr(FileName,".vcs")) {
		backup->ToDo		= true;
		backup->Calendar 	= true;
	} else if (strstr(FileName,".vcf")) {
		backup->PhonePhonebook	= true;
	} else if (strstr(FileName,".ldif")) {
		backup->PhonePhonebook	= true;
	} else {
		backup->UseUnicode	= true;
		backup->IMEI 		= true;
		backup->Model 		= true;
		backup->DateTime 	= true;
		backup->PhonePhonebook 	= true;
		backup->SIMPhonebook 	= true;
		backup->ToDo		= true;
		backup->Calendar 	= true;
		backup->CallerLogos 	= true;
		backup->SMSC 		= true;
		backup->WAPBookmark 	= true;
		backup->WAPSettings 	= true;
		backup->MMSSettings 	= true;
		backup->Ringtone 	= true;
		backup->StartupLogo 	= true;
		backup->OperatorLogo 	= true;
		backup->Profiles 	= true;
 		backup->FMStation 	= true;
		backup->GPRSPoint	= true;
		backup->Note		= true;
	}
}

GSM_Error GSM_SaveBackupFile(char *FileName, GSM_Backup *backup, bool UseUnicode)
{
	if (strstr(FileName,".lmb")) {
		return SaveLMB(FileName,backup);
	} else if (strstr(FileName,".vcs")) {
		return SaveVCalendar(FileName,backup);
	} else if (strstr(FileName,".vcf")) {
		return SaveVCard(FileName,backup);
	} else if (strstr(FileName,".ldif")) {
		return SaveLDIF(FileName,backup);
	} else {
		return SaveBackup(FileName,backup, UseUnicode);
	}
}

GSM_Error GSM_ReadBackupFile(char *FileName, GSM_Backup *backup)
{
	FILE		*file;
	unsigned char	buffer[300];

	file = fopen(FileName, "rb");
	if (file == NULL) return(GE_CANTOPENFILE);
	fread(buffer, 1, 9, file); /* Read the header of the file. */
	fclose(file);

	GSM_ClearBackup(backup);

	/* Attempt to identify filetype */
	if (strstr(FileName,".vcs")) {
		return LoadVCalendar(FileName,backup);
	} else if (strstr(FileName,".vcf")) {
		return LoadVCard(FileName,backup);
	} else if (strstr(FileName,".ldif")) {
		return LoadLDIF(FileName,backup);
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
	backup->Ringtone	[0] = NULL;
	backup->Profiles	[0] = NULL;
	backup->ToDo		[0] = NULL;
	backup->GPRSPoint	[0] = NULL;
	backup->FMStation	[0] = NULL;
	backup->Note		[0] = NULL;
	backup->StartupLogo	    = NULL;
	backup->OperatorLogo	    = NULL;

	backup->IMEI		[0] = 0;
	backup->Model		[0] = 0;
	backup->DateTimeAvailable   = false;
	backup->MD5Original	[0] = 0;
	backup->MD5Calculated	[0] = 0;
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
