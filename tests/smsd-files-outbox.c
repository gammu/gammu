#include <gammu.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../smsd/core.h"
#include "../smsd/services/files.h"

#define TEST_FILENAME_LENGTH 180

static int TestFilename(GSM_StateMachine *state_machine, const char *outbox_path,
			const char *filename)
{
	GSM_MultiSMSMessage sms;
	GSM_SMSDConfig config;
	GSM_Error error;
	char id[GSM_MAX_FILENAME_LENGTH + 1];
	char *full_path;
	size_t full_path_length;
	FILE *file;
	int result = 0;

	full_path_length = strlen(outbox_path) + strlen(filename) + 1;
	full_path = malloc(full_path_length);
	if (full_path == NULL) {
		fprintf(stderr, "Failed to allocate outbox path\n");
		return 1;
	}
	snprintf(full_path, full_path_length, "%s%s", outbox_path, filename);

	file = fopen(full_path, "wb");
	if (file == NULL) {
		fprintf(stderr, "Failed to create outbox file: %s\n", full_path);
		free(full_path);
		return 1;
	}
	fputs("long filename regression", file);
	fclose(file);

	memset(&sms, 0, sizeof(sms));
	memset(&config, 0, sizeof(config));
	memset(id, 0, sizeof(id));
	config.gsm = state_machine;
	config.outboxpath = outbox_path;
	config.transmitformat = "auto";

	error = SMSDFiles.FindOutboxSMS(&sms, &config, id);
	if (error != ERR_NONE) {
		fprintf(stderr, "Failed to find outbox message: %s\n", GSM_ErrorString(error));
		result = 1;
	} else if (strcmp(id, filename) != 0) {
		fprintf(stderr, "Outbox filename was not preserved\n");
		result = 1;
	} else if (sms.Number == 0 || strcmp(DecodeUnicodeString(sms.SMS[0].Number), "12345") != 0) {
		fprintf(stderr, "Outbox recipient was not parsed correctly\n");
		result = 1;
	}

	remove(full_path);
	free(full_path);
	return result;
}

static int TestLongPath(GSM_StateMachine *state_machine)
{
	GSM_MultiSMSMessage sms;
	GSM_SMSDConfig config;
	GSM_Error error;
	char outbox_path[GSM_MAX_FILENAME_ID_LENGTH + 1];
	char id[GSM_MAX_FILENAME_LENGTH + 1];

	memset(outbox_path, 'x', sizeof(outbox_path) - 1);
	outbox_path[sizeof(outbox_path) - 1] = '\0';
	memset(&sms, 0, sizeof(sms));
	memset(&config, 0, sizeof(config));
	memset(id, 0, sizeof(id));
	config.gsm = state_machine;
	config.outboxpath = outbox_path;
	config.transmitformat = "auto";

	error = SMSDFiles.FindOutboxSMS(&sms, &config, id);
	if (error != ERR_CANTOPENFILE) {
		fprintf(stderr, "Overlong outbox path returned %s\n", GSM_ErrorString(error));
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	static const char prefix[] = "OUTC20260729_120000_00_12345_";
	static const char extension[] = ".txt";
	GSM_StateMachine *state_machine;
	char long_filename[GSM_MAX_FILENAME_LENGTH + 1];
	size_t filler_length;
	int result;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s OUTBOX_PATH\n", argv[0]);
		return 1;
	}

	filler_length = TEST_FILENAME_LENGTH - strlen(prefix) - strlen(extension);
	memcpy(long_filename, prefix, strlen(prefix));
	memset(long_filename + strlen(prefix), 'x', filler_length);
	memcpy(long_filename + strlen(prefix) + filler_length, extension, sizeof(extension));

	state_machine = GSM_AllocStateMachine();
	if (state_machine == NULL) {
		fprintf(stderr, "Failed to allocate state machine\n");
		return 1;
	}

	result = TestFilename(state_machine, argv[1], "OUT12345.txt");
	if (result == 0) {
		result = TestFilename(state_machine, argv[1], long_filename);
	}
	if (result == 0) {
		result = TestLongPath(state_machine);
	}

	GSM_FreeStateMachine(state_machine);
	return result;
}
