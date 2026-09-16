/**
 * INI parsing at EOF, with and without a final line ending.
 */

#include <gammu.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#include "common.h"

static void write_text(FILE *file, const char *text, int encoding)
{
	while (*text != '\0') {
		if (encoding == 2) {
			test_result(fputc(0, file) != EOF);
		}
		test_result(fputc((unsigned char)*text++, file) != EOF);
		if (encoding == 1) {
			test_result(fputc(0, file) != EOF);
		}
	}
}

static void check_file(const char *path, const char *text, const char *ending,
		       int encoding, GSM_Error expected_error, const char *expected)
{
	FILE *file;
	INI_Section *ini = NULL;
	GSM_Error error;
	unsigned char section[12], key[26];
	unsigned char *value;
	size_t i;
	int fd;

	fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_BINARY, 0600);
	test_result(fd >= 0);
#ifndef WIN32
	{
		struct stat status;

		test_result(fstat(fd, &status) == 0);
		test_result((status.st_mode & 077) == 0);
	}
#endif
	file = fdopen(fd, "wb");
	if (file == NULL) {
		close(fd);
		remove(path);
	}
	test_result(file != NULL);
	if (encoding != 0) {
		test_result(fputc(encoding == 1 ? 0xff : 0xfe, file) != EOF);
		test_result(fputc(encoding == 1 ? 0xfe : 0xff, file) != EOF);
	}
	write_text(file, text, encoding);
	write_text(file, ending, encoding);
	test_result(fclose(file) == 0);

	error = INI_ReadFile(path, encoding != 0, &ini);
	test_result(remove(path) == 0);
	gammu_test_result_code(error, "INI_ReadFile at EOF", expected_error);
	if (expected_error != ERR_NONE) {
		test_result(ini == NULL);
		return;
	}
	test_result(ini != NULL);
	if (encoding != 0) {
		EncodeUnicode(section, "smsd", 4);
		EncodeUnicode(key, "RunOnReceive", 12);
		value = INI_GetValue(ini, section, key, TRUE);
	} else {
		value = INI_GetValue(ini, "smsd", "RunOnReceive", FALSE);
	}
	if (expected == NULL) {
		test_result(value == NULL);
	} else {
		test_result(value != NULL);
		if (encoding == 0) {
			test_result(strcmp((char *)value, expected) == 0);
		} else {
			for (i = 0; i <= strlen(expected); i++) {
				test_result(value[2 * i] == 0);
				test_result(value[2 * i + 1] == (unsigned char)expected[i]);
			}
		}
	}
	INI_Free(ini);
}

int main(int argc, char **argv)
{
	static const struct {
		const char *text;
		GSM_Error error;
		const char *value;
	} cases[] = {
		{"[smsd]\r\nRunOnReceive = cmd.exe /d /c forward_sms.bat",
		 ERR_NONE, "cmd.exe /d /c forward_sms.bat"},
		{"[smsd]\nRunOnReceive = handler \t", ERR_NONE, "handler"},
		{"[smsd]\nRunOnReceive = handler\n# final comment", ERR_NONE, "handler"},
		{"[smsd]\nRunOnReceive = handler\n; final comment", ERR_NONE, "handler"},
		{"[smsd]", ERR_NONE, NULL},
		{"", ERR_FILENOTSUPPORTED, NULL},
		{"[smsd]\nRunOnReceive = \t", ERR_NONE, NULL},
		{"[smsd]\nRunOnReceive = first\nRunOnReceive = second", ERR_NONE, "second"}
	};
	static const char *endings[] = {"", "\n", "\r\n", "\r"};
	char long_value[1501], long_text[1600];
	size_t i, j;
	int encoding;

	test_result(argc == 2);
	memset(long_value, 'x', sizeof(long_value) - 1);
	long_value[sizeof(long_value) - 1] = '\0';
	snprintf(long_text, sizeof(long_text), "[smsd]\nRunOnReceive = %s", long_value);
	for (encoding = 0; encoding < 3; encoding++) {
		for (j = 0; j < sizeof(endings) / sizeof(endings[0]); j++) {
			for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
				printf("EOF case %lu, encoding %d, ending %lu\n",
				       (unsigned long)i, encoding, (unsigned long)j);
				check_file(argv[1], cases[i].text, endings[j], encoding,
					   cases[i].error, cases[i].value);
			}
			check_file(argv[1], long_text, endings[j], encoding, ERR_NONE, long_value);
		}
	}
	return 0;
}
