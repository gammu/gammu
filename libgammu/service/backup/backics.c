/* (c) 2003 by Marcin Wiacek */

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

GSM_Error SaveICS(char *FileName, GSM_Backup *backup)
{
	int 		i;
	size_t Length = 0;
	unsigned char 	Buffer[1000];
	FILE 		*file;
	GSM_Error error;

	file = fopen(FileName, "wb");
	if (file == NULL) return ERR_CANTOPENFILE;

	Length=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);
	Length+=sprintf(Buffer+Length, "VERSION:2.0%c%c",13,10);
	chk_fwrite(Buffer,1,Length,file);

	i=0;
	while (backup->Calendar[i]!=NULL) {
		sprintf(Buffer, "%c%c",13,10);
		chk_fwrite(Buffer,1,2,file);
		Length = 0;
		error = GSM_EncodeVCALENDAR(Buffer, sizeof(Buffer),&Length,backup->Calendar[i],FALSE,Mozilla_iCalendar);
		if (error != ERR_NONE) {
			fclose(file);
			return error;
		}
		chk_fwrite(Buffer,1,Length,file);
		i++;
	}
	i=0;
	while (backup->ToDo[i]!=NULL) {
		sprintf(Buffer, "%c%c",13,10);
		chk_fwrite(Buffer,1,2,file);
		Length = 0;
		error = GSM_EncodeVTODO(Buffer, sizeof(Buffer), &Length,backup->ToDo[i],FALSE,Mozilla_VToDo);
		if (error != ERR_NONE) {
			fclose(file);
			return error;
		}
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

GSM_Error LoadICS(char *FileName, GSM_Backup *backup)
{
	return LoadVCalendarPrivate(FileName, backup, Mozilla_iCalendar, Mozilla_VToDo);
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
