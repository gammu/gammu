#include <gammu.h>

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libgammu/misc/array.h"
#include "../smsd/core.h"
#include "../smsd/services/files.h"

#define SKIP_TEST 77

static gboolean SetUTF8Locale(void)
{
	static const char *locales[] = {
		"C.UTF-8",
		"C.utf8",
		"en_US.UTF-8",
		".UTF-8",
	};
	size_t i;

	if (setlocale(LC_CTYPE, "") != NULL && MB_CUR_MAX > 1) {
		return TRUE;
	}

	for (i = 0; i < sizeof(locales) / sizeof(locales[0]); i++) {
		if (setlocale(LC_CTYPE, locales[i]) != NULL && MB_CUR_MAX > 1) {
			return TRUE;
		}
	}

	return FALSE;
}

int main(int argc, char **argv)
{
	static const unsigned char expected[] = {
		0xFE, 0xFF,
		0x00, 0x41,
		0x00, 0xF6,
		0x00, 0x42,
	};
	GSM_MultiSMSMessage sms;
	GSM_SMSDConfig config;
	GSM_StringArray locations;
	unsigned char actual[sizeof(expected) + 1];
	char *filename;
	size_t filename_length;
	size_t actual_length;
	FILE *file;
	GSM_Error error;
	int result = 0;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s INBOX_PATH\n", argv[0]);
		return 1;
	}

	if (!SetUTF8Locale()) {
		fprintf(stderr, "Skipping test: no UTF-8 locale available\n");
		return SKIP_TEST;
	}

	memset(&sms, 0, sizeof(sms));
	memset(&config, 0, sizeof(config));
	GSM_StringArray_New(&locations);

	config.inboxpath = argv[1];
	config.inboxformat = "unicode";
	config.deliveryreport = "sms";

	sms.Number = 1;
	sms.SMS[0].PDU = SMS_Deliver;
	sms.SMS[0].Coding = SMS_Coding_Unicode_No_Compression;
	sms.SMS[0].DateTime.Year = 2026;
	sms.SMS[0].DateTime.Month = 7;
	sms.SMS[0].DateTime.Day = 28;
	sms.SMS[0].DateTime.Hour = 12;
	sms.SMS[0].DateTime.Minute = 34;
	sms.SMS[0].DateTime.Second = 56;
	EncodeUnicode(sms.SMS[0].Number, "123", 3);
	memcpy(sms.SMS[0].Text, expected + 2, sizeof(expected) - 2);

	error = SMSDFiles.SaveInboxSMS(&sms, &config, &locations, NULL);
	if (error != ERR_NONE) {
		fprintf(stderr, "Failed to save inbox message: %s\n", GSM_ErrorString(error));
		result = 1;
		goto cleanup_locations;
	}
	if (locations.used != 1) {
		fprintf(stderr, "Expected one inbox file, got %lu\n", (unsigned long)locations.used);
		result = 1;
		goto cleanup_locations;
	}

	filename_length = strlen(argv[1]) + strlen(locations.data[0]) + 1;
	filename = malloc(filename_length);
	if (filename == NULL) {
		fprintf(stderr, "Failed to allocate inbox filename\n");
		result = 1;
		goto cleanup_locations;
	}
	snprintf(filename, filename_length, "%s%s", argv[1], locations.data[0]);

	file = fopen(filename, "rb");
	if (file == NULL) {
		fprintf(stderr, "Failed to open generated inbox file: %s\n", filename);
		result = 1;
		goto cleanup_filename;
	}
	actual_length = fread(actual, 1, sizeof(actual), file);
	if (ferror(file)) {
		fprintf(stderr, "Failed to read generated inbox file: %s\n", filename);
		result = 1;
	}
	fclose(file);

	if (actual_length != sizeof(expected)) {
		fprintf(stderr, "Expected %lu bytes, got %lu\n",
			(unsigned long)sizeof(expected), (unsigned long)actual_length);
		result = 1;
	} else if (memcmp(actual, expected, sizeof(expected)) != 0) {
		fprintf(stderr, "Generated inbox file has unexpected contents\n");
		result = 1;
	}

cleanup_filename:
	remove(filename);
	free(filename);
cleanup_locations:
	GSM_StringArray_Free(&locations);
	return result;
}
