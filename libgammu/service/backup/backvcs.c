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
		error = GSM_EncodeVCALENDAR(Buffer, sizeof(Buffer),&Length,backup->Calendar[i],FALSE,Nokia_VCalendar);
		if (error != ERR_NONE) return error;
		chk_fwrite(Buffer,1,Length,file);
		i++;
	}
	i=0;
	while (backup->ToDo[i]!=NULL) {
		sprintf(Buffer, "%c%c",13,10);
		chk_fwrite(Buffer,1,2,file);
		Length = 0;
		error = GSM_EncodeVTODO(Buffer, sizeof(Buffer),&Length,backup->ToDo[i],FALSE,Nokia_VToDo);
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

GSM_Error LoadVCalendarPrivate(char *FileName, GSM_Backup *backup, GSM_VCalendarVersion CalVer, GSM_VToDoVersion ToDoVer)
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
		error = GSM_DecodeVCALENDAR_VTODO(NULL, File.Buffer, &Pos, &Calendar, &ToDo, CalVer, ToDoVer);
		if (error == ERR_EMPTY) {
			error = ERR_NONE;
			break;
		}
		if (error != ERR_NONE) break;
		if (Calendar.EntriesNum != 0) {
			if (numCal < GSM_MAXCALENDARTODONOTES) {
				backup->Calendar[numCal] = (GSM_CalendarEntry *)malloc(sizeof(GSM_CalendarEntry));
			        if (backup->Calendar[numCal] == NULL) {
					error = ERR_MOREMEMORY;
					break;
				}
				backup->Calendar[numCal + 1] = NULL;
			} else {
				dbgprintf(NULL, "Increase GSM_MAXCALENDARTODONOTES\n");
				error = ERR_MOREMEMORY;
				break;
			}
			memcpy(backup->Calendar[numCal],&Calendar,sizeof(GSM_CalendarEntry));
			backup->Calendar[numCal]->Location = numCal + 1;
			numCal++;
		}
		if (ToDo.EntriesNum != 0) {
			if (numToDo < GSM_MAXCALENDARTODONOTES) {
				backup->ToDo[numToDo] = (GSM_ToDoEntry *)malloc(sizeof(GSM_ToDoEntry));
			        if (backup->ToDo[numToDo] == NULL) {
					error = ERR_MOREMEMORY;
					break;
				}
				backup->ToDo[numToDo + 1] = NULL;
			} else {
				dbgprintf(NULL, "Increase GSM_MAXCALENDARTODONOTES\n");
				error = ERR_MOREMEMORY;
				break;
			}
			memcpy(backup->ToDo[numToDo],&ToDo,sizeof(GSM_ToDoEntry));
			backup->ToDo[numToDo]->Location = numToDo + 1;
			numToDo++;
		}
	}

	free(File.Buffer);
	File.Buffer=NULL;
	return error;
}

GSM_Error LoadVCalendar(char *FileName, GSM_Backup *backup)
{
	return LoadVCalendarPrivate(FileName, backup, Nokia_VCalendar, Nokia_VToDo);
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
