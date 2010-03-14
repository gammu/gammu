
#include <string.h>
#include <ctype.h>

#include "../../../cfg/config.h"
#include "../../phone/nokia/nfunc.h"
#include "../../phone/nokia/dct3/n7110.h"
#include "../../misc/coding.h"
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
	}
}

GSM_Error GSM_SaveBackupFile(char *FileName, GSM_Backup *backup, bool UseUnicode)
{
	FILE *file;
 
	file = fopen(FileName, "wb");      
	if (file == NULL) return GE_CANTOPENFILE;

	if (strstr(FileName,".lmb")) {
		SaveLMB(file,backup);
	} else if (strstr(FileName,".vcs")) {
		SaveVCalendar(file,backup);
	} else if (strstr(FileName,".vcf")) {
		SaveVCard(file,backup);
	} else if (strstr(FileName,".ldif")) {
		SaveLDIF(file,backup);
	} else {
		SaveBackup(file,backup, UseUnicode);
	}

	fclose(file);
   
	return GE_NONE;
}

GSM_Error GSM_ReadBackupFile(char *FileName, GSM_Backup *backup)
{
	FILE		*file;
	unsigned char	buffer[300];
	GSM_Error	error;

	file = fopen(FileName, "rb");
	if (file == NULL) return(GE_CANTOPENFILE);

	fread(buffer, 1, 9, file); /* Read the header of the file. */
	fclose(file);

	GSM_ClearBackup(backup);

	/* Attempt to identify filetype */
	if (strstr(FileName,".vcs")) {
		error=LoadVCalendar(FileName,backup);
	} else if (strstr(FileName,".vcf")) {
		error=LoadVCard(FileName,backup);
	} else if (strstr(FileName,".ldif")) {
		error=LoadLDIF(FileName,backup);
	} else if (memcmp(buffer, "LMB ",4)==0) {
		error=LoadLMB(FileName,backup);
	} else if (buffer[0] == 0xFE && buffer[1] == 0xFF) {
		error=LoadBackup(FileName,backup,true);
	} else if (buffer[0] == 0xFF && buffer[1] == 0xFE) {
		error=LoadBackup(FileName,backup,true);
	} else {
		error=LoadBackup(FileName,backup,false);
	}

	return error;
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
	backup->StartupLogo	    = NULL;
	backup->OperatorLogo	    = NULL;

	backup->IMEI		[0] = 0;
	backup->Model		[0] = 0;
	backup->DateTimeAvailable   = false;
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
