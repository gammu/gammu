#include <gammu.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "../helper/message-display.h"
#include "../helper/message-cmdline.h"

#define SKIP_TEST 77

static gboolean SetUTF8Locale(void)
{
	static const char *locales[] = {
		"",
		"C.UTF-8",
		"C.utf8",
		"en_US.UTF-8",
		".UTF-8",
	};
	size_t i;
	wchar_t decoded;

	for (i = 0; i < sizeof(locales) / sizeof(locales[0]); i++) {
		if (setlocale(LC_CTYPE, locales[i]) == NULL ||
		    MB_CUR_MAX < 2) {
			continue;
		}
		mbtowc(NULL, NULL, 0);
		if (mbtowc(&decoded, "\xc3\xa9", 2) == 2 &&
		    decoded == 0xe9) {
			return TRUE;
		}
	}

	return FALSE;
}

int main(int argc, char **argv)
{
	static char valid_utf8[] = {
		'm', (char)0xc3, (char)0xa9, 't',
		(char)0xc3, (char)0xa9, 'o', 0
	};
	static char invalid_locale[] = {
		'a', (char)0xc3, 'z', 0
	};
	static char invalid_utf8[] = {
		'o', 'k', (char)0xff, 'r', 'e', 's', 't', 0
	};
	GSM_MultiSMSMessage sms;
	GSM_Error error;
	GSM_Message_Type type = SMS_Display;
	GSM_Debug_Info *debug_info;
	const char *invalid_input;
	int i;

	GSM_InitLocales(NULL);

	if (getenv("GAMMU_TEST_UTF8") != NULL) {
		if (argc < 2) {
			fprintf(stderr, "Missing message text argument\n");
			return 2;
		}
		argv[argc - 1] = valid_utf8;
	}

	invalid_input = getenv("GAMMU_EXPECT_INVALIDDATA");
	if (invalid_input != NULL) {
		if (argc < 2) {
			fprintf(stderr, "Missing message text argument\n");
			return 2;
		}
		if (strcmp(invalid_input, "locale") == 0) {
			if (!SetUTF8Locale()) {
				fprintf(stderr,
					"Skipping test: no UTF-8 locale available\n");
				return SKIP_TEST;
			}
			argv[argc - 1] = invalid_locale;
		} else if (strcmp(invalid_input, "utf8") == 0) {
			argv[argc - 1] = invalid_utf8;
		} else {
			fprintf(stderr, "Unknown invalid input type: %s\n",
				invalid_input);
			return 2;
		}
	}

	/* Configure debugging */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stdout, FALSE, debug_info);
	/* Include debugging to help diagnose test failures */
	GSM_SetDebugLevel("textall", debug_info);

	error = CreateMessage(&type, &sms, argc, 1, argv, NULL);
	if (invalid_input != NULL) {
		gammu_test_result_code(error, "CreateMessage", ERR_INVALIDDATA);
		return 0;
	}
	gammu_test_result(error, "CreateMessage");

	DisplayMultiSMSInfo(&sms, FALSE, TRUE, NULL, NULL);
	DisplayMultiSMSInfo(&sms, TRUE, TRUE, NULL, NULL);

	for (i = 0; i < sms.Number; i++) {
		printf("Message number: %i\n", i);
		sms.SMS[i].SMSC.Location = 0;
		error = DisplaySMSFrame(&sms.SMS[i], NULL);
		gammu_test_result(error, "DisplaySMSFrame");
	}

	printf("\n");
	printf("Number of messages: %i\n", sms.Number);
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
