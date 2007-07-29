/**
 * vCard parser testing.
 *
 * First parameter is location of vCard, second location of Gammu backup
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
	int pos = 0;
	GSM_MemoryEntry pbk;
	GSM_Error error;
	char buffer[2048];
	FILE *f;
	size_t len;
	bool generate = false;
	GSM_Backup backup;

	/* Check parameters */
	if (argc != 3 && argc != 4) {
		printf("Not enough parameters!\nUsage: vcard-read file.vcf file.backup\n");
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

	/* Parse vCard */
	error = GSM_DecodeVCARD(buffer, &pos, &pbk, SonyEricsson_VCard21);
	if (error != ERR_NONE) {
		printf("Parsing failed: %s\n", GSM_ErrorString(error));
		return 1;
	}

	/* Generate file if we should */
	if (generate) {
		GSM_ClearBackup(&backup);
		strcpy(backup.Creator, "vCard tester");
		backup.PhonePhonebook[0] = &pbk;
		backup.PhonePhonebook[1] = NULL;
		if (GSM_SaveBackupFile(argv[2], &backup, true) != ERR_NONE) {
			printf("Error saving backup to %s\n", argv[2]);
			return 1;
		}
	}

	/* Read file content */
	GSM_ClearBackup(&backup);
	if (GSM_ReadBackupFile(argv[2], &backup) != ERR_NONE) {
		printf("Error reading backup from %s\n", argv[2]);
		return 1;
	}

	/* Free data */
	GSM_FreeBackup(&backup);

	/* We're done */
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
