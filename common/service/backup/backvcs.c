/* (c) 2003 by Marcin Wiacek */

#include <string.h>
#include <ctype.h>

#include <gammu-config.h>

#include "../../misc/coding/coding.h"
#include "../../debug.h"
#include "../gsmlogo.h"
#include "../gsmmisc.h"
#include "backvcs.h"

#ifdef GSM_ENABLE_BACKUP

/**
 * Helper define to check error code from fwrite.
 */
#define chk_fwrite(data, size, count, file) \
	if (fwrite(data, size, count, file) != count) goto fail;

GSM_Error SaveVCalendar(char *FileName, GSM_Backup *backup)
{
	int 		i;
	size_t Length = 0;
	unsigned char 	Buffer[1000];
	FILE 		*file;
	GSM_Error error;

	file = fopen(FileName, "wb");
	if (file == NULL) return ERR_CANTOPENFILE;

	Length=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);
	Length+=sprintf(Buffer+Length, "VERSION:1.0%c%c",13,10);
	chk_fwrite(Buffer,1,Length,file);

	i=0;
	while (backup->Calendar[i]!=NULL) {
		sprintf(Buffer, "%c%c",13,10);
		chk_fwrite(Buffer,1,2,file);
		Length = 0;
		error = GSM_EncodeVCALENDAR(Buffer, sizeof(Buffer),&Length,backup->Calendar[i],false,Nokia_VCalendar);
		if (error != ERR_NONE) return error;
		chk_fwrite(Buffer,1,Length,file);
		i++;
	}
	i=0;
	while (backup->ToDo[i]!=NULL) {
		sprintf(Buffer, "%c%c",13,10);
		chk_fwrite(Buffer,1,2,file);
		Length = 0;
		error = GSM_EncodeVTODO(Buffer, sizeof(Buffer),&Length,backup->ToDo[i],false,Nokia_VToDo);
		if (error != ERR_NONE) return error;
		chk_fwrite(Buffer,1,Length,file);
		i++;
	}

	Length=sprintf(Buffer, "%c%cEND:VCALENDAR%c%c",13,10,13,10);
	chk_fwrite(Buffer,1,Length,file);

	fclose(file);
	return ERR_NONE;
fail:
	fclose(file);
	return ERR_WRITING_FILE;
}

GSM_Error LoadVCalendar(char *FileName, GSM_Backup *backup)
{
	GSM_File 		File;
	GSM_Error		error;
	GSM_CalendarEntry	Calendar;
	GSM_ToDoEntry		ToDo;
	int			numCal = 0, numToDo = 0;
	size_t Pos = 0;

	File.Buffer = NULL;
	error = GSM_ReadFile(FileName, &File);
	if (error != ERR_NONE) return error;

	while (1) {
		error = GSM_DecodeVCALENDAR_VTODO(NULL, File.Buffer, &Pos, &Calendar, &ToDo, Nokia_VCalendar, Nokia_VToDo);
		if (error == ERR_EMPTY) break;
		if (error != ERR_NONE) return error;
		if (Calendar.EntriesNum != 0) {
			if (numCal < GSM_MAXCALENDARTODONOTES) {
				backup->Calendar[numCal] = malloc(sizeof(GSM_CalendarEntry));
			        if (backup->Calendar[numCal] == NULL) return ERR_MOREMEMORY;
				backup->Calendar[numCal + 1] = NULL;
			} else {
				dbgprintf(NULL, "Increase GSM_MAXCALENDARTODONOTES\n");
				return ERR_MOREMEMORY;
			}
			memcpy(backup->Calendar[numCal],&Calendar,sizeof(GSM_CalendarEntry));
			backup->Calendar[numCal]->Location = numCal + 1;
			numCal++;
		}
		if (ToDo.EntriesNum != 0) {
			if (numToDo < GSM_MAXCALENDARTODONOTES) {
				backup->ToDo[numToDo] = malloc(sizeof(GSM_ToDoEntry));
			        if (backup->ToDo[numToDo] == NULL) return ERR_MOREMEMORY;
				backup->ToDo[numToDo + 1] = NULL;
			} else {
				dbgprintf(NULL, "Increase GSM_MAXCALENDARTODONOTES\n");
				return ERR_MOREMEMORY;
			}
			memcpy(backup->ToDo[numToDo],&ToDo,sizeof(GSM_ToDoEntry));
			backup->ToDo[numToDo]->Location = numToDo + 1;
			numToDo++;
		}
	}

	return ERR_NONE;
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
