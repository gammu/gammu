/* (c) 2003 by Marcin Wiacek */

#include <string.h>
#include <ctype.h>

#include <gammu-config.h>

#include "../../misc/coding/coding.h"
#include "../../debug.h"
#include "../gsmlogo.h"
#include "../gsmmisc.h"
#include "backvcf.h"

#ifdef GSM_ENABLE_BACKUP

/**
 * Helper define to check error code from fwrite.
 */
#define chk_fwrite(data, size, count, file) \
	if (fwrite(data, size, count, file) != count) goto fail;

GSM_Error SaveVCard(char *FileName, GSM_Backup *backup)
{
	int 		i;
	size_t Length = 0;
	char		Buffer[100000];
	FILE 		*file;
	GSM_Error	error;

	file = fopen(FileName, "wb");
	if (file == NULL) return ERR_CANTOPENFILE;

	i=0;
	while (backup->PhonePhonebook[i]!=NULL) {
		Length = 0;
		error = GSM_EncodeVCARD(NULL, Buffer, sizeof(Buffer), &Length,backup->PhonePhonebook[i],true,Nokia_VCard21);
		if (error != ERR_NONE) return error;
		chk_fwrite(Buffer,1,Length,file);
		sprintf(Buffer, "%c%c",13,10);
		chk_fwrite(Buffer,1,2,file);
		i++;
	}

	fclose(file);
	return ERR_NONE;
fail:
	fclose(file);
	return ERR_WRITING_FILE;
}

GSM_Error LoadVCard(char *FileName, GSM_Backup *backup)
{
	GSM_File 		File;
	GSM_Error		error;
	GSM_MemoryEntry		Pbk;
	int			numPbk = 0;
	size_t Pos = 0;

	File.Buffer = NULL;
	error = GSM_ReadFile(FileName, &File);
	if (error != ERR_NONE) return error;

	while (1) {
		error = GSM_DecodeVCARD(NULL, File.Buffer, &Pos, &Pbk, Nokia_VCard21);
		if (error == ERR_EMPTY) break;
		if (error != ERR_NONE) return error;
		if (numPbk < GSM_BACKUP_MAX_PHONEPHONEBOOK) {
			backup->PhonePhonebook[numPbk] = malloc(sizeof(GSM_MemoryEntry));
		        if (backup->PhonePhonebook[numPbk] == NULL) return ERR_MOREMEMORY;
			backup->PhonePhonebook[numPbk + 1] = NULL;
		} else {
			dbgprintf(NULL, "Increase GSM_BACKUP_MAX_PHONEPHONEBOOK\n");
			return ERR_MOREMEMORY;
		}
		memcpy(backup->PhonePhonebook[numPbk],&Pbk,sizeof(GSM_MemoryEntry));
		backup->PhonePhonebook[numPbk]->Location 	= numPbk + 1;
		backup->PhonePhonebook[numPbk]->MemoryType 	= MEM_ME;
		numPbk++;
	}

	return ERR_NONE;
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
