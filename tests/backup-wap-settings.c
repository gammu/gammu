#include "../libgammu/misc/private-file.h"
#include <gammu.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

static void check_settings(const char *path, const char *kind, int count,
			   GSM_Error expected)
{
	GSM_Backup *backup = malloc(sizeof(*backup));
	GSM_MultiWAPSettings *settings;
	FILE *file = GSM_OpenPrivateFile(path, "wb");
	int i;

	test_result(backup != NULL);
	test_result(file != NULL);
	test_result(fputs("[Backup]\nFormat=1.04\n", file) >= 0);
	test_result(fprintf(file, "[%sSettings001]\nBearer=GPRS\n", kind) > 0);
	for (i = 0; i < count; i++) {
		test_result(fprintf(file, "Title%02d=TITLE%02d\n", i, i) > 0);
	}
	test_result(fclose(file) == 0);

	test_result(GSM_ReadBackupFile(path, backup, GSM_Backup_Gammu) == expected);
	if (expected == ERR_NONE) {
		settings = kind[0] == 'W' ? backup->WAPSettings[0] : backup->MMSSettings[0];
		test_result(settings != NULL);
		test_result(settings->Number == count);
	}
	GSM_FreeBackup(backup);
	free(backup);
	test_result(remove(path) == 0);
}

int main(int argc, char **argv)
{
	test_result(argc == 2);
	check_settings(argv[1], "WAP", 4, ERR_NONE);
	check_settings(argv[1], "WAP", 5, ERR_CORRUPTED);
	check_settings(argv[1], "WAP", 6, ERR_CORRUPTED);
	check_settings(argv[1], "WAP", 20, ERR_CORRUPTED);
	check_settings(argv[1], "MMS", 4, ERR_NONE);
	check_settings(argv[1], "MMS", 5, ERR_CORRUPTED);
	check_settings(argv[1], "MMS", 6, ERR_CORRUPTED);
	check_settings(argv[1], "MMS", 20, ERR_CORRUPTED);
	return 0;
}
