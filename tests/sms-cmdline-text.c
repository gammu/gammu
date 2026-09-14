#include <gammu.h>
#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifdef WIN32
#include <io.h>
#define NULL_DEVICE "NUL"
#else
#include <unistd.h>
#define NULL_DEVICE "/dev/null"
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#include "common.h"
#include "../helper/message-cmdline.h"

typedef struct {
	const char *name;
	const char *text;
	const char *options[10];
	const char *expected;
	int parts;
	GSM_Coding_Type coding;
	GSM_UDH udh;
	GSM_Error error;
	int replace;
} TextTest;

static gboolean SetUTF8Locale(void)
{
	static const char *locales[] = {
		"", "C.UTF-8", "C.utf8", "en_US.UTF-8", ".UTF-8"
	};
	size_t i;
	wchar_t decoded;

	for (i = 0; i < sizeof(locales) / sizeof(locales[0]); i++) {
		if (setlocale(LC_CTYPE, locales[i]) == NULL || MB_CUR_MAX < 2) {
			continue;
		}
		mbtowc(NULL, NULL, 0);
		if (mbtowc(&decoded, "\xc3\xa9", 2) == 2 && decoded == 0xe9) {
			return TRUE;
		}
	}
	return FALSE;
}

static void CheckText(const TextTest *test, int source, gboolean options_first,
		      const char *input_path, GSM_Message_Type type)
{
	GSM_MultiSMSMessage sms;
	GSM_Error error;
	unsigned char expected[10004];
	const char *raw_args[20] = {"sms-cmdline-text", "TEXT", "213"};
	char *args[20];
	int argc = 3;
	int i, pass;
	size_t offset = 0, length, expected_length;
	FILE *input;
	int fd;

	printf("%s: source=%d options_first=%d type=%d\n",
	       test->name, source, options_first, type);
	if (source == 0) {
		fd = open(input_path, O_WRONLY | O_CREAT | O_EXCL | O_BINARY, 0600);
		test_result(fd >= 0);
#ifndef WIN32
		{
			struct stat status;

			test_result(fstat(fd, &status) == 0);
			test_result((status.st_mode & 077) == 0);
		}
#endif
		input = fdopen(fd, "wb");
		test_result(input != NULL);
		length = strlen(test->text);
		test_result(fwrite(test->text, 1, length, input) == length);
		test_result(fclose(input) == 0);
		test_result(freopen(input_path, "rb", stdin) != NULL);
	}
	for (pass = 0; pass < 2; pass++) {
		if ((pass == 0) == options_first) {
			for (i = 0; test->options[i] != NULL; i++) {
				raw_args[argc++] = test->options[i];
			}
		} else if (source != 0) {
			raw_args[argc++] = source == 1 ? "-text" : "-textutf8";
			raw_args[argc++] = test->text;
		}
	}
	for (i = 0; i < argc; i++) {
		args[i] = malloc(strlen(raw_args[i]) + 1);
		test_result(args[i] != NULL);
		strcpy(args[i], raw_args[i]);
	}
	args[argc] = NULL;
	error = CreateMessage(&type, &sms, argc, 1, args, NULL);
	if (source == 0) {
		test_result(freopen(NULL_DEVICE, "rb", stdin) != NULL);
		test_result(remove(input_path) == 0);
	}
	for (i = 0; i < argc; i++) {
		free(args[i]);
	}
	gammu_test_result_code(error, test->name, test->error);
	if (error != ERR_NONE) {
		return;
	}
	test_result(DecodeUTF8Checked(expected, test->expected, strlen(test->expected)));
	expected_length = UnicodeLength(expected);
	test_result(sms.Number == test->parts);
	for (i = 0; i < sms.Number; i++) {
		test_result(sms.SMS[i].Coding == test->coding);
		test_result(sms.SMS[i].UDH.Type == test->udh);
		test_result(sms.SMS[i].ReplaceMessage == test->replace);
		if (test->parts > 1) {
			test_result(sms.SMS[i].UDH.AllParts == test->parts);
			test_result(sms.SMS[i].UDH.PartNumber == i + 1);
			test_result(sms.SMS[i].UDH.ID8bit == sms.SMS[0].UDH.ID8bit);
			test_result(sms.SMS[i].UDH.ID16bit == sms.SMS[0].UDH.ID16bit);
		}
		length = UnicodeLength(sms.SMS[i].Text);
		test_result(offset + length <= expected_length);
		test_result(memcmp(sms.SMS[i].Text, expected + offset * 2, length * 2) == 0);
		offset += length;
	}
	test_result(offset == expected_length);
}

int main(int argc, char **argv)
{
	char ascii160[161], ascii161[162], ascii200[201];
	char extension80[81], extension81[82];
	char unicode70[141], unicode71[143];
	char stdin4999[5000], stdin5000[5001], stdin5001[5002];
	const char *length_options[] = {"-len", "-maxlen", "-autolen"};
	gboolean utf8_locale;
	size_t i, j;
	int source, order, mode, special_order;
	const TextTest tests[] = {
		{"ASCII boundary", ascii160, {NULL}, ascii160, 1,
		 SMS_Coding_Default_No_Compression, UDH_NoUDH, ERR_NONE, 0},
		{"ASCII multipart", ascii161, {NULL}, ascii161, 2,
		 SMS_Coding_Default_No_Compression, UDH_ConcatenatedMessages, ERR_NONE, 0},
		{"extension boundary", extension80, {NULL}, extension80, 1,
		 SMS_Coding_Default_No_Compression, UDH_NoUDH, ERR_NONE, 0},
		{"extension multipart", extension81, {NULL}, extension81, 2,
		 SMS_Coding_Default_No_Compression, UDH_ConcatenatedMessages, ERR_NONE, 0},
		{"Unicode boundary", unicode70, {"-unicode", NULL}, unicode70, 1,
		 SMS_Coding_Unicode_No_Compression, UDH_NoUDH, ERR_NONE, 0},
		{"Unicode multipart", unicode71, {"-unicode", NULL}, unicode71, 2,
		 SMS_Coding_Unicode_No_Compression, UDH_ConcatenatedMessages, ERR_NONE, 0},
		{"automatic Unicode", unicode71, {"-autolen", "200", NULL}, unicode71, 2,
		 SMS_Coding_Unicode_No_Compression, UDH_ConcatenatedMessages, ERR_NONE, 0},
		{"automatic alphabet", ascii200, {"-autolen", "200", NULL}, ascii200, 2,
		 SMS_Coding_Default_No_Compression, UDH_ConcatenatedMessages, ERR_NONE, 0},
		{"automatic coding after truncation", "a\xd0\x96", {"-autolen", "1", NULL}, "a", 1,
		 SMS_Coding_Default_No_Compression, UDH_NoUDH, ERR_NONE, 0},
		{"16-bit short message", "abc", {"-16bit", NULL}, "abc", 1,
		 SMS_Coding_Default_No_Compression, UDH_NoUDH, ERR_NONE, 0},
		{"16-bit default", ascii200, {"-16bit", NULL}, ascii200, 2,
		 SMS_Coding_Default_No_Compression, UDH_ConcatenatedMessages16bit, ERR_NONE, 0},
		{"16-bit before length", ascii200, {"-16bit", "-len", "200", NULL}, ascii200, 2,
		 SMS_Coding_Default_No_Compression, UDH_ConcatenatedMessages16bit, ERR_NONE, 0},
		{"16-bit after length", ascii200, {"-len", "200", "-16bit", NULL}, ascii200, 2,
		 SMS_Coding_Default_No_Compression, UDH_ConcatenatedMessages16bit, ERR_NONE, 0},
		{"16-bit automatic Unicode", unicode71, {"-16bit", "-autolen", "200", NULL}, unicode71, 2,
		 SMS_Coding_Unicode_No_Compression, UDH_ConcatenatedMessages16bit, ERR_NONE, 0},
		{"last limit wins", ascii200, {"-len", "1", "-maxlen", "160", NULL}, ascii160, 1,
		 SMS_Coding_Default_No_Compression, UDH_NoUDH, ERR_NONE, 0},
		{"last limit selects automatic coding", unicode71, {"-len", "1", "-autolen", "200", NULL}, unicode71, 2,
		 SMS_Coding_Unicode_No_Compression, UDH_ConcatenatedMessages, ERR_NONE, 0},
		{"last limit disables automatic coding", "abc", {"-autolen", "1", "-len", "3", "-unicode", NULL}, "abc", 1,
		 SMS_Coding_Unicode_No_Compression, UDH_NoUDH, ERR_NONE, 0},
		{"empty input", "", {NULL}, "", 1,
		 SMS_Coding_Default_No_Compression, UDH_NoUDH, ERR_NONE, 0},
		{"newline only", "\n", {NULL}, "", 1,
		 SMS_Coding_Default_No_Compression, UDH_NoUDH, ERR_NONE, 0},
		{"trailing newline", "abc\n", {NULL}, "abc", 1,
		 SMS_Coding_Default_No_Compression, UDH_NoUDH, ERR_NONE, 0},
		{"truncate after newline trimming", "ab\nc\n", {"-len", "3", NULL}, "ab\n", 1,
		 SMS_Coding_Default_No_Compression, UDH_NoUDH, ERR_NONE, 0},
		{"voice indicator", "abc", {"-enablevoice", NULL}, "abc", 1,
		 SMS_Coding_Default_No_Compression, UDH_EnableVoice, ERR_NONE, 0},
		{"void message", "abc", {"-voidsms", NULL}, "abc", 1,
		 SMS_Coding_Default_No_Compression, UDH_VoidSMS, ERR_NONE, 0},
		{"replacement message", "abc", {"-replacemessages", "3", NULL}, "abc", 1,
		 SMS_Coding_Default_No_Compression, UDH_NoUDH, ERR_NONE, 3},
		{"oversized replacement", ascii200, {"-replacemessages", "3", NULL}, NULL, 0,
		 0, 0, ERR_INVALIDDATA, 0},
		{"oversized indicator", ascii200, {"-enablevoice", NULL}, NULL, 0,
		 0, 0, ERR_INVALIDDATA, 0},
		{"maximum parts", ascii200, {"-maxsms", "1", NULL}, NULL, 0,
		 0, 0, ERR_MOREMEMORY, 0},
		{"single part allowed", ascii160, {"-maxsms", "1", NULL}, ascii160, 1,
		 SMS_Coding_Default_No_Compression, UDH_NoUDH, ERR_NONE, 0},
	};
	TextTest limit = {"decoded length limit", "", {NULL}, "", 1,
		SMS_Coding_Unicode_No_Compression, UDH_NoUDH, ERR_NONE, 0};
	const TextTest stdin_tests[] = {
		{"stdin capacity", stdin4999, {NULL}, stdin4999, 33,
		 SMS_Coding_Default_No_Compression, UDH_ConcatenatedMessages, ERR_NONE, 0},
		{"stdin terminator capacity", stdin5000, {NULL}, NULL, 0,
		 0, 0, ERR_INVALIDDATA, 0},
		{"stdin beyond old capacity", stdin5001, {NULL}, NULL, 0,
		 0, 0, ERR_INVALIDDATA, 0},
		{"oversized stdin with limit", stdin5001, {"-len", "1", NULL}, NULL, 0,
		 0, 0, ERR_INVALIDDATA, 0},
	};
	const char *special_options[] = {
		"-enablevoice", "-disablevoice", "-enablefax", "-disablefax",
		"-enableemail", "-disableemail", "-voidsms", "-replacemessages"
	};
	const GSM_UDH special_udh[] = {
		UDH_EnableVoice, UDH_DisableVoice, UDH_EnableFax, UDH_DisableFax,
		UDH_EnableEmail, UDH_DisableEmail, UDH_VoidSMS, UDH_NoUDH
	};

	test_result(argc == 2);
	GSM_InitLocales(NULL);
	utf8_locale = SetUTF8Locale();
	if (!utf8_locale) {
		fprintf(stderr, "No UTF-8 locale: testing non-ASCII input through -textutf8 only\n");
		setlocale(LC_CTYPE, "C");
	}
	memset(ascii160, 'a', sizeof(ascii160) - 1);
	ascii160[160] = 0;
	memset(ascii161, 'a', sizeof(ascii161) - 1);
	ascii161[161] = 0;
	memset(ascii200, 'a', sizeof(ascii200) - 1);
	ascii200[200] = 0;
	memset(extension80, '^', sizeof(extension80) - 1);
	extension80[80] = 0;
	memset(extension81, '^', sizeof(extension81) - 1);
	extension81[81] = 0;
	for (i = 0; i < 71; i++) {
		memcpy(unicode71 + i * 2, "\xd0\x96", 2);
	}
	unicode71[142] = 0;
	memcpy(unicode70, unicode71, 140);
	unicode70[140] = 0;
	memset(stdin4999, 'a', sizeof(stdin4999) - 1);
	stdin4999[4999] = 0;
	memset(stdin5000, 'a', sizeof(stdin5000) - 1);
	stdin5000[5000] = 0;
	memset(stdin5001, 'a', sizeof(stdin5001) - 1);
	stdin5001[5001] = 0;
	for (i = 0; i < sizeof(stdin_tests) / sizeof(stdin_tests[0]); i++) {
		CheckText(&stdin_tests[i], 0, TRUE, argv[1], SMS_Display);
	}

	for (source = 0; source < 3; source++) {
		for (order = 0; order < 2; order++) {
			for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
				if (!utf8_locale && source != 2) {
					for (j = 0; tests[i].text[j] != 0; j++) {
						if ((unsigned char)tests[i].text[j] >= 0x80) {
							break;
						}
					}
					if (tests[i].text[j] != 0) {
						continue;
					}
				}
				for (mode = 0; mode < 2; mode++) {
					CheckText(&tests[i], source, order, argv[1], mode == 0 ? SMS_Display : SMS_SMSD);
				}
			}
			for (j = 0; j < sizeof(length_options) / sizeof(length_options[0]); j++) {
				limit.options[0] = "-unicode";
				limit.options[1] = length_options[j];
				limit.options[2] = "1";
				limit.text = ascii200;
				limit.expected = "a";
				limit.coding = j == 2 ? SMS_Coding_Default_No_Compression : SMS_Coding_Unicode_No_Compression;
				CheckText(&limit, source, order, argv[1], SMS_Display);
				if (utf8_locale || source == 2) {
					limit.text = unicode71;
					limit.expected = "\xd0\x96";
					limit.coding = SMS_Coding_Unicode_No_Compression;
					CheckText(&limit, source, order, argv[1], SMS_Display);
					/* A limit beyond the decoded length must not add data. */
					limit.options[2] = "100";
					limit.expected = unicode71;
					limit.parts = 2;
					limit.udh = UDH_ConcatenatedMessages;
					CheckText(&limit, source, order, argv[1], SMS_Display);
					limit.parts = 1;
					limit.udh = UDH_NoUDH;
				}
				/*
				 * Locale conversion uses mbtowc, which cannot return a
				 * non-BMP code point in Windows' 16-bit wchar_t. Explicit
				 * UTF-8 decoding supports surrogate pairs on every platform.
				 */
				if ((utf8_locale && sizeof(wchar_t) > 2) || source == 2) {
					/* Drop the whole character if its surrogate pair would split. */
					limit.text = "\xf0\x9f\x98\x80";
					limit.options[2] = "1";
					limit.expected = "";
					limit.coding = j == 2 ? SMS_Coding_Default_No_Compression : SMS_Coding_Unicode_No_Compression;
					CheckText(&limit, source, order, argv[1], SMS_Display);
					limit.text = "a\xf0\x9f\x98\x80";
					limit.options[2] = "2";
					limit.expected = "a";
					CheckText(&limit, source, order, argv[1], SMS_Display);
					limit.options[2] = "3";
					limit.expected = limit.text;
					limit.coding = SMS_Coding_Unicode_No_Compression;
					CheckText(&limit, source, order, argv[1], SMS_Display);
				}
				for (i = 0; i < sizeof(special_options) / sizeof(special_options[0]); i++) {
					TextTest special = {"special mode with length", "abcdef", {NULL}, "abc", 1,
						SMS_Coding_Default_No_Compression, special_udh[i], ERR_NONE,
						i == 7 ? 3 : 0};

					for (special_order = 0; special_order < 2; special_order++) {
						int n = 0;
						int length_arg;

						if (special_order == 0) {
							special.options[n++] = length_options[j];
							special.options[n++] = "3";
						}
						special.options[n++] = special_options[i];
						if (special.replace != 0) {
							special.options[n++] = "3";
						}
						if (special_order == 1) {
							special.options[n++] = length_options[j];
							special.options[n++] = "3";
						}
						special.options[n] = NULL;
						CheckText(&special, source, order, argv[1], SMS_Display);
						/* Length options must not enable multipart special messages. */
						length_arg = special_order == 0 ? 1 : n - 1;
						special.options[length_arg] = "200";
						special.text = ascii200;
						special.expected = NULL;
						special.error = ERR_INVALIDDATA;
						CheckText(&special, source, order, argv[1], SMS_Display);
						special.options[length_arg] = "3";
						special.error = ERR_NONE;
						if (j == 2 && (utf8_locale || source == 2)) {
							special.text = "\xd0\x96";
							special.expected = special.text;
							special.coding = SMS_Coding_Unicode_No_Compression;
							CheckText(&special, source, order, argv[1], SMS_Display);
						}
						special.text = "abcdef";
						special.expected = "abc";
						special.coding = SMS_Coding_Default_No_Compression;
					}
				}
			}
		}
	}
	fclose(stdin);
	return 0;
}
