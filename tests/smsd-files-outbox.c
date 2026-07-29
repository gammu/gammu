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

#include "../smsd/core.h"
#include "../smsd/services/files.h"

#define TEST_FILENAME_LENGTH 180

static char *CreateOutboxFile(const char *outbox_path, const char *filename)
{
	char *full_path;
	size_t full_path_length;
	FILE *file;
	int fd;

	full_path_length = strlen(outbox_path) + strlen(filename) + 1;
	full_path = malloc(full_path_length);
	if (full_path == NULL) {
		fprintf(stderr, "Failed to allocate outbox path\n");
		return NULL;
	}
	snprintf(full_path, full_path_length, "%s%s", outbox_path, filename);

	remove(full_path);
	fd = open(full_path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (fd < 0) {
		fprintf(stderr, "Failed to create outbox file: %s\n", full_path);
		free(full_path);
		return NULL;
	}
	file = fdopen(fd, "wb");
	if (file == NULL) {
		fprintf(stderr, "Failed to open outbox stream: %s\n", full_path);
		close(fd);
		remove(full_path);
		free(full_path);
		return NULL;
	}
	fputs("long filename regression", file);
	fclose(file);
#ifndef WIN32
	{
		struct stat status;

		if (stat(full_path, &status) != 0 || (status.st_mode & 077) != 0) {
			fprintf(stderr, "Outbox file permissions are not restrictive\n");
			remove(full_path);
			free(full_path);
			return NULL;
		}
	}
#endif
	return full_path;
}

static void SetupConfig(GSM_SMSDConfig *config, GSM_StateMachine *state_machine,
			const char *outbox_path, const char *sent_path,
			const char *error_path)
{
	memset(config, 0, sizeof(*config));
	config->gsm = state_machine;
	config->outboxpath = outbox_path;
	config->sentsmspath = sent_path;
	config->errorsmspath = error_path;
	config->transmitformat = "auto";
}

static int TestFilename(GSM_StateMachine *state_machine, const char *outbox_path,
			const char *filename)
{
	GSM_MultiSMSMessage sms;
	GSM_SMSDConfig config;
	GSM_Error error;
	char id[GSM_MAX_FILENAME_LENGTH + 1];
	char *full_path;
	int result = 0;

	full_path = CreateOutboxFile(outbox_path, filename);
	if (full_path == NULL) {
		return 1;
	}

	memset(&sms, 0, sizeof(sms));
	memset(id, 0, sizeof(id));
	SetupConfig(&config, state_machine, outbox_path, outbox_path, outbox_path);

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

static int TestInvalidRecipient(GSM_StateMachine *state_machine,
				const char *outbox_path)
{
	GSM_MultiSMSMessage sms;
	GSM_SMSDConfig config;
	GSM_Error error;
	char filename[GSM_MAX_FILENAME_LENGTH + 1];
	char id[GSM_MAX_FILENAME_LENGTH + 1];
	char *full_path;
	size_t recipient_length = GSM_MAX_NUMBER_LENGTH + 1;
	int result = 0;

	memcpy(filename, "OUT", 3);
	memset(filename + 3, '1', recipient_length);
	strcpy(filename + 3 + recipient_length, ".txt");
	full_path = CreateOutboxFile(outbox_path, filename);
	if (full_path == NULL) {
		return 1;
	}

	memset(&sms, 0, sizeof(sms));
	memset(id, 0, sizeof(id));
	SetupConfig(&config, state_machine, outbox_path, outbox_path, outbox_path);

	error = SMSDFiles.FindOutboxSMS(&sms, &config, id);
	if (error != ERR_INVALIDDATA) {
		fprintf(stderr, "Oversized recipient returned %s\n", GSM_ErrorString(error));
		result = 1;
	} else if (strcmp(id, filename) != 0) {
		fprintf(stderr, "Invalid outbox filename was not preserved\n");
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
	memset(id, 0, sizeof(id));
	SetupConfig(&config, state_machine, outbox_path, outbox_path, outbox_path);

	error = SMSDFiles.FindOutboxSMS(&sms, &config, id);
	if (error != ERR_CANTOPENFILE) {
		fprintf(stderr, "Overlong outbox path returned %s\n", GSM_ErrorString(error));
		return 1;
	}
	return 0;
}

static int TestLongDestinationPath(GSM_StateMachine *state_machine,
				   const char *outbox_path, gboolean sent)
{
	static const char filename[] = "OUT12345.txt";
	GSM_MultiSMSMessage sms;
	GSM_SMSDConfig config;
	GSM_Error error;
	char destination_path[GSM_MAX_FILENAME_ID_LENGTH + 1];
	char id[GSM_MAX_FILENAME_LENGTH + 1];
	char *full_path;
	int result = 0;

	full_path = CreateOutboxFile(outbox_path, filename);
	if (full_path == NULL) {
		return 1;
	}
	memset(destination_path, 'x', sizeof(destination_path) - 1);
	destination_path[sizeof(destination_path) - 1] = '\0';
	memset(&sms, 0, sizeof(sms));
	memset(id, 0, sizeof(id));
	SetupConfig(&config, state_machine, outbox_path,
		    sent ? destination_path : outbox_path,
		    sent ? outbox_path : destination_path);

	error = SMSDFiles.FindOutboxSMS(&sms, &config, id);
	if (error != ERR_CANTOPENFILE) {
		fprintf(stderr, "Overlong %s path returned %s\n",
			sent ? "sent" : "error", GSM_ErrorString(error));
		result = 1;
	} else if (id[0] != '\0') {
		fprintf(stderr, "Message ID was set before path validation\n");
		result = 1;
	}

	remove(full_path);
	free(full_path);
	return result;
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
		result = TestInvalidRecipient(state_machine, argv[1]);
	}
	if (result == 0) {
		result = TestLongPath(state_machine);
	}
	if (result == 0) {
		result = TestLongDestinationPath(state_machine, argv[1], TRUE);
	}
	if (result == 0) {
		result = TestLongDestinationPath(state_machine, argv[1], FALSE);
	}

	GSM_FreeStateMachine(state_machine);
	return result;
}
