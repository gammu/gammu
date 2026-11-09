#include "../libgammu/misc/private-file.h"
#include <gammu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

static void write_unicode(FILE *file, const char *text)
{
	while (*text) {
		test_result(fputc(*text++, file) != EOF);
		test_result(fputc(0, file) != EOF);
	}
}

static void check_text(const char *input, const char *output, size_t length)
{
	static const char header[] =
		"[Backup]\r\nFormat = 1.04\r\n"
		"[Calendar001]\r\nLocation = 1\r\nType = Memo\r\nText = \"";
	GSM_Backup *backup = malloc(sizeof(*backup));
	FILE *file = GSM_OpenPrivateFile(input, "wb");
	char line[20000];
	size_t i;
	int found = 0;

	test_result(backup != NULL);
	test_result(file != NULL);
	test_result(fputc(0xff, file) != EOF);
	test_result(fputc(0xfe, file) != EOF);
	write_unicode(file, header);
	for (i = 0; i < length; i++) write_unicode(file, "A");
	write_unicode(file, "\"\r\n");
	test_result(fclose(file) == 0);

	test_result(GSM_ReadBackupFile(input, backup, GSM_Backup_GammuUCS2) == ERR_NONE);
	test_result(GSM_SaveBackupFile((char *)output, backup, GSM_Backup_Gammu) == ERR_NONE);
	GSM_FreeBackup(backup);
	free(backup);

	file = fopen(output, "rb");
	test_result(file != NULL);
	while (fgets(line, sizeof(line), file) != NULL) {
		if (strncmp(line, "TextUnicode = ", 14) == 0) {
			found = 1;
			test_result(strlen(line + 14) == length * 4 + 2);
		}
	}
	test_result(found);
	test_result(fclose(file) == 0);
	test_result(remove(input) == 0);
	test_result(remove(output) == 0);
}

int main(int argc, char **argv)
{
	test_result(argc == 3);
	check_text(argv[1], argv[2], 2400);
	check_text(argv[1], argv[2], 2499);
	check_text(argv[1], argv[2], 2500);
	check_text(argv[1], argv[2], 2600);
	check_text(argv[1], argv[2], 4090);
	return 0;
}
