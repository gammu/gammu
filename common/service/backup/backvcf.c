/* (c) 2003 by Marcin Wiacek */

#include <string.h>
#include <ctype.h>

#include "../../phone/nokia/nfunc.h"
#include "../../phone/nokia/dct3/n7110.h"
#include "../../misc/coding/coding.h"
#include "../gsmlogo.h"
#include "../gsmmisc.h"
#include "backvcf.h"

#ifdef GSM_ENABLE_BACKUP

GSM_Error SaveVCard(char *FileName, GSM_Backup *backup)
{
	int 		i, Length = 0;
	unsigned char 	Buffer[1000];
	FILE 		*file;
 
	file = fopen(FileName, "wb");      
	if (file == NULL) return ERR_CANTOPENFILE;

	i=0;
	while (backup->PhonePhonebook[i]!=NULL) {
		sprintf(Buffer, "%c%c",13,10);
		fwrite(Buffer,1,2,file);
		Length = 0;
		GSM_EncodeVCARD(Buffer,&Length,backup->PhonePhonebook[i],true,Nokia_VCard21);
		fwrite(Buffer,1,Length,file);
		i++;
	}

	fclose(file);
	return ERR_NONE;
}

GSM_Error LoadVCard(char *FileName, GSM_Backup *backup)
{
	GSM_File 		File;
	GSM_Error		error;
	GSM_MemoryEntry		Pbk;
	int			numPbk = 0, Pos;

	File.Buffer = NULL;
	error = GSM_ReadFile(FileName, &File);
	if (error != ERR_NONE) return error;

	Pos = 0;
	while (1) {
		error = GSM_DecodeVCARD(File.Buffer, &Pos, &Pbk, Nokia_VCard21);
		if (error == ERR_EMPTY) break;
		if (error != ERR_NONE) return error;
		if (numPbk < GSM_BACKUP_MAX_PHONEPHONEBOOK) {
			backup->PhonePhonebook[numPbk] = malloc(sizeof(GSM_MemoryEntry));
		        if (backup->PhonePhonebook[numPbk] == NULL) return ERR_MOREMEMORY;
			backup->PhonePhonebook[numPbk + 1] = NULL;
		} else {
			dbgprintf("Increase GSM_BACKUP_MAX_PHONEPHONEBOOK\n");
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
