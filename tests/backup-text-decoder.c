#include "../libgammu/misc/private-file.h"
#include <gammu.h>
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>
#endif

#include "common.h"

static void put_char(FILE *file, int ch, int unicode)
{
	test_result(fputc(ch, file) != EOF);
	if (unicode) test_result(fputc(0, file) != EOF);
}

static void put_text(FILE *file, const char *text, int unicode)
{
	while (*text) put_char(file, *text++, unicode);
}

static void check_text(const char *path, int unicode, size_t length,
		       int escaped, int accepted)
{
	GSM_Backup *backup = malloc(sizeof(*backup));
	FILE *file;
	int fd;
	size_t i;

	test_result(backup != NULL);
	test_result(remove(path) == 0 || errno == ENOENT);
	fd = GSM_CreatePrivateFileExclusive(path, 1);
	test_result(fd >= 0);
#ifdef _WIN32
	file = _fdopen(fd, "wb");
#else
	file = fdopen(fd, "wb");
#endif
	test_result(file != NULL);
	if (unicode) {
		test_result(fputc(0xff, file) != EOF);
		test_result(fputc(0xfe, file) != EOF);
	}
	put_text(file, "[Backup]\r\nFormat=1.04\r\n[Calendar001]\r\nLocation=1\r\nType=Memo\r\nText=\"", unicode);
	for (i = 0; i < length; i++) {
		put_text(file, escaped == 1 ? "\\n" :
				  escaped == 2 ? "\xc3\xa9" : "A", unicode);
	}
	put_text(file, "\"\r\n", unicode);
	test_result(fclose(file) == 0);

	test_result(GSM_ReadBackupFile(path, backup, GSM_Backup_Gammu) == ERR_NONE);
	test_result(backup->Calendar[0] != NULL);
	if (accepted) {
		test_result(backup->Calendar[0]->EntriesNum > 0);
		test_result(UnicodeLength(backup->Calendar[0]->Entries[0].Text) == length);
		if (escaped == 1) {
			test_result(backup->Calendar[0]->Entries[0].Text[0] == 0);
			test_result(backup->Calendar[0]->Entries[0].Text[1] == '\n');
		}
		if (escaped == 2) {
			test_result(backup->Calendar[0]->Entries[0].Text[0] == 0);
			test_result(backup->Calendar[0]->Entries[0].Text[1] == 0xe9);
		}
	} else {
		test_result(backup->Calendar[0]->EntriesNum == 0);
	}
	GSM_FreeBackup(backup);
	free(backup);
	test_result(remove(path) == 0);
}

int main(int argc, char **argv)
{
	test_result(argc == 2);
	check_text(argv[1], 0, 128, 0, 1);
	check_text(argv[1], 0, 4096, 0, 1);
	check_text(argv[1], 0, 4096, 1, 1);
	check_text(argv[1], 0, 4097, 0, 0);
	check_text(argv[1], 0, 20000, 0, 0);
	check_text(argv[1], 1, 128, 0, 1);
	check_text(argv[1], 1, 3000, 1, 1);
	check_text(argv[1], 1, 4096, 0, 1);
	check_text(argv[1], 1, 9990, 0, 0);
	if (setlocale(LC_CTYPE, "C.UTF-8") != NULL) {
		check_text(argv[1], 0, 3000, 2, 1);
	}
	return 0;
}
