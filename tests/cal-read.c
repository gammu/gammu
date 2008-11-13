/**
 * [iv]Calendar parser testing.
 *
 * First parameter is location of [iv]Calendar, second location of Gammu backup
 * how it should be parsed.
 *
 * Optional third parameter can be used to generate template backup
 * file.
 */
#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	size_t pos = 0;
	GSM_ToDoEntry todo;
	GSM_CalendarEntry cal;
	GSM_Error error;
	char buffer[65536];
	FILE *f;
	size_t len;
	bool generate = false;
	GSM_Backup backup;
	int i;
	GSM_Debug_Info *debug_info;
	bool skipcal, skiptodo;

	/* Configure debugging */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, false, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Check parameters */
	if (argc != 3 && argc != 4) {
		printf("Not enough parameters!\nUsage: cal-read file.[vi]cs file.backup\n");
		return 1;
	}

	/* Check for generating option */
	if (argc == 4 && strcmp(argv[3], "generate") == 0) {
		generate = true;
	}

	/* Open file */
	f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("Could not open %s\n", argv[1]);
		return 1;
	}

	/* Read data */
	len = fread(buffer, 1, sizeof(buffer) - 1, f);
	if (!feof(f)) {
		printf("Could not read whole file %s\n", argv[1]);
		return 1;
	}

	/* Zero terminate string */
	buffer[len] = 0;

	/* We don't need file any more */
	fclose(f);

	todo.Type = 0;
	cal.Type = 0;
	todo.Location = 0;
	cal.Location = 0;

	/* Parse [iv]Calendar */
	if (strstr(argv[1], ".ics") != NULL) {
		error = GSM_DecodeVCALENDAR_VTODO(buffer, &pos, &cal, &todo,
				Mozilla_iCalendar, Mozilla_VToDo);
	} else {
		error = GSM_DecodeVCALENDAR_VTODO(buffer, &pos, &cal, &todo,
				SonyEricsson_VCalendar, SonyEricsson_VToDo);
	}
	if (error != ERR_NONE) {
		printf("Parsing failed: %s\n", GSM_ErrorString(error));
		return 1;
	}

	/* Generate file if we should */
	if (generate) {
		GSM_ClearBackup(&backup);
		strcpy(backup.Creator, "[iv]Calendar tester");
		if (cal.EntriesNum != 0) {
			backup.Calendar[0] = &cal;
			backup.Calendar[1] = NULL;
		} else {
			backup.Calendar[0] = NULL;
		}
		if (todo.EntriesNum != 0) {
			backup.ToDo[0] = &todo;
			backup.ToDo[1] = NULL;
		} else {
			backup.ToDo[0] = NULL;
		}
		if (GSM_SaveBackupFile(argv[2], &backup, true) != ERR_NONE) {
			printf("Error saving backup to %s\n", argv[2]);
			return 1;
		}
	}

	/* Read file content */
	GSM_ClearBackup(&backup);
	if (GSM_ReadBackupFile(argv[2], &backup, GSM_Backup_GammuUCS2) != ERR_NONE) {
		printf("Error reading backup from %s\n", argv[2]);
		return 1;
	}

	/* Did we read something? */
	skipcal = false;
	if (cal.EntriesNum == 0 && backup.Calendar[0] == NULL) {
		skipcal = true;
	}
	skiptodo = false;
	if (todo.EntriesNum == 0 && backup.ToDo[0] == NULL) {
		skiptodo = true;
	}

	/* Compare size */
	if (!skipcal && cal.EntriesNum != backup.Calendar[0]->EntriesNum) {
		printf("Different number of entries!\n");
		return 1;
	}
	if (!skiptodo && todo.EntriesNum != backup.ToDo[0]->EntriesNum) {
		printf("Different number of entries!\n");
		return 1;
	}

	/* Compare content */
	if (!skipcal) {
		for (i = 0; i < cal.EntriesNum; i++) {
			switch (cal.Entries[i].EntryType) {
				case CAL_TEXT:
				case CAL_DESCRIPTION:
				case CAL_LOCATION:
				case CAL_PHONE:
				case CAL_LUID:
					if (!mywstrncmp(
						cal.Entries[i].Text,
						backup.Calendar[0]->Entries[i].Text,
						0)) {
						printf("Calendar field %d is not the same!\n", i);
						return 1;
					}
					break;
				case CAL_START_DATETIME:
				case CAL_END_DATETIME:
				case CAL_TONE_ALARM_DATETIME:
				case CAL_SILENT_ALARM_DATETIME:
				case CAL_PRIVATE:
				case CAL_CONTACTID:
				case CAL_REPEAT_DAYOFWEEK:
				case CAL_REPEAT_DAY:
				case CAL_REPEAT_DAYOFYEAR:
				case CAL_REPEAT_WEEKOFMONTH:
				case CAL_REPEAT_MONTH:
				case CAL_REPEAT_FREQUENCY:
				case CAL_REPEAT_STARTDATE:
				case CAL_REPEAT_STOPDATE:
				case CAL_REPEAT_COUNT:
				case CAL_LAST_MODIFIED:
					break;
			}
		}
	}
	if (!skiptodo) {
		for (i = 0; i < todo.EntriesNum; i++) {
			printf("%d,%d\n", todo.Entries[i].EntryType, backup.ToDo[0]->Entries[i].EntryType);
			switch (todo.Entries[i].EntryType) {
				case TODO_TEXT:
				case TODO_DESCRIPTION:
				case TODO_LOCATION:
				case TODO_PHONE:
				case TODO_LUID:
					if (!mywstrncmp(
						todo.Entries[i].Text,
						backup.ToDo[0]->Entries[i].Text,
						0)) {
						printf("Todo field %d is not the same!\n", i);
						return 1;
					}
					break;
				case TODO_END_DATETIME:
				case TODO_START_DATETIME:
				case TODO_ALARM_DATETIME:
				case TODO_SILENT_ALARM_DATETIME:
				case TODO_PRIVATE:
				case TODO_CONTACTID:
				case TODO_COMPLETED:
				case TODO_CATEGORY:
				case TODO_LAST_MODIFIED:
					break;
			}
		}
	}


	/* Free data */
	GSM_FreeBackup(&backup);

	/* We're done */
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
