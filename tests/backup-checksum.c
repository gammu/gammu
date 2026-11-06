#include <gammu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

static void check_checksum(const char *path, size_t length, int invalid_hex,
			   GSM_Error expected)
{
	GSM_Backup *backup = malloc(sizeof(*backup));
	FILE *file = fopen(path, "wb");
	size_t i;

	test_result(backup != NULL);
	test_result(file != NULL);
	test_result(fputs("[Backup]\nFormat=1.04\n[Checksum]\nMD5=", file) >= 0);
	for (i = 0; i < length; i++) {
		test_result(fputc(invalid_hex && i == length / 2 ? 'G' : 'a', file) != EOF);
	}
	test_result(fputc('\n', file) != EOF);
	test_result(fclose(file) == 0);

	test_result(GSM_ReadBackupFile(path, backup, GSM_Backup_Gammu) == expected);
	if (expected == ERR_NONE) {
		test_result(strlen(backup->MD5Original) == 32);
	}
	GSM_FreeBackup(backup);
	free(backup);
	test_result(remove(path) == 0);
}

int main(int argc, char **argv)
{
	test_result(argc == 2);
	check_checksum(argv[1], 32, 0, ERR_NONE);
	check_checksum(argv[1], 31, 0, ERR_CORRUPTED);
	check_checksum(argv[1], 33, 0, ERR_CORRUPTED);
	check_checksum(argv[1], 99, 0, ERR_CORRUPTED);
	check_checksum(argv[1], 100, 0, ERR_CORRUPTED);
	check_checksum(argv[1], 400, 0, ERR_CORRUPTED);
	check_checksum(argv[1], 32, 1, ERR_CORRUPTED);
	return 0;
}
