/**
 * vNote backup handling.
 */
/* Copyright (c) 2009 Michal Cihar <michal@cihar.com> */
/* Licensend under GNU GPL 2 */

#include <string.h>
#include <ctype.h>

#include <gammu-config.h>

#include "../../misc/coding/coding.h"
#include "../../debug.h"
#include "../gsmlogo.h"
#include "../gsmmisc.h"
#include "backics.h"
#include "backvcs.h"

#ifdef GSM_ENABLE_BACKUP

/**
 * Helper define to check error code from fwrite.
 */
#define chk_fwrite(data, size, count, file) \
	if (fwrite(data, size, count, file) != count) goto fail;

GSM_Error SaveVNT(char *FileName, GSM_Backup *backup)
{
	int 		i;
	size_t Length = 0;
	unsigned char 	Buffer[1000];
	FILE 		*file;
	GSM_Error error;

	file = fopen(FileName, "wb");
	if (file == NULL) return ERR_CANTOPENFILE;

	i=0;
	while (backup->Note[i]!=NULL) {
		Length = 0;
		error = GSM_EncodeVNTFile(Buffer, sizeof(Buffer), &Length, backup->Note[i]);
		if (error != ERR_NONE) return error;
		chk_fwrite(Buffer,1,Length,file);
		i++;
		sprintf(Buffer, "%c%c",13,10);
		chk_fwrite(Buffer,1,2,file);
	}

	fclose(file);
	return ERR_NONE;
fail:
	fclose(file);
	return ERR_WRITING_FILE;
}

GSM_Error LoadVNT(char *FileName, GSM_Backup *backup)
{
	GSM_File 		File;
	GSM_Error		error;
	GSM_NoteEntry		Note;
	int			num = 0;
	size_t Pos = 0;

	File.Buffer = NULL;
	error = GSM_ReadFile(FileName, &File);
	if (error != ERR_NONE) return error;

	while (1) {
		error = GSM_DecodeVNOTE(File.Buffer, &Pos, &Note);
		if (error == ERR_EMPTY) {
			error = ERR_NONE;
			break;
		}
		if (error != ERR_NONE) break;
		if (num < GSM_BACKUP_MAX_NOTE) {
			backup->Note[num] = malloc(sizeof(GSM_NoteEntry));
		        if (backup->Note[num] == NULL) {
				error = ERR_MOREMEMORY;
				break;
			}
			backup->Note[num + 1] = NULL;
		} else {
			dbgprintf(NULL, "Increase GSM_BACKUP_MAX_NOTE\n");
			error = ERR_MOREMEMORY;
			break;
		}
		memcpy(backup->Note[num], &Note, sizeof(GSM_NoteEntry));
		backup->Note[num]->Location 	= num + 1;
		num++;
	}

	free(File.Buffer);
	return error;
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
