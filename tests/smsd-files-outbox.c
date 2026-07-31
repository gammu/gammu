#include <gammu.h>

#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef WIN32
#include <io.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

#include "../smsd/core.h"
#include "../smsd/services/files.h"

#define TEST_FILENAME_LENGTH 180

#ifdef PATH_MAX
#define TEST_FILES_PATH_MAX PATH_MAX
#else
#define TEST_FILES_PATH_MAX 4069
#endif

static const char sent_info_filename[] = "OUT12345.smsbackup";

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

static int IsTemporaryFilename(const char *filename)
{
	return filename[0] == '.' && strlen(filename) == sizeof(".XXXXXX") - 1;
}

static int HasTemporaryFile(const char *outbox_path)
{
#ifdef WIN32
	struct _finddata_t entry;
	intptr_t handle;
	char *pattern;
	size_t pattern_length;
	int found = 0;

	pattern_length = strlen(outbox_path) + sizeof(".*");
	pattern = malloc(pattern_length);
	if (pattern == NULL) {
		fprintf(stderr, "Failed to allocate temporary-file pattern\n");
		return 1;
	}
	snprintf(pattern, pattern_length, "%s.*", outbox_path);
	handle = _findfirst(pattern, &entry);
	free(pattern);
	if (handle == -1) {
		return 0;
	}
	do {
		if (IsTemporaryFilename(entry.name)) {
			found = 1;
			break;
		}
	} while (_findnext(handle, &entry) == 0);
	_findclose(handle);
	return found;
#else
	DIR *directory;
	struct dirent *entry;
	int found = 0;

	directory = opendir(outbox_path);
	if (directory == NULL) {
		fprintf(stderr, "Failed to open outbox directory\n");
		return 1;
	}
	while ((entry = readdir(directory)) != NULL) {
		if (IsTemporaryFilename(entry->d_name)) {
			found = 1;
			break;
		}
	}
	closedir(directory);
	return found;
#endif
}

static int TestSentInfo(GSM_StateMachine *state_machine,
			const char *outbox_path, gboolean read_only)
{
	static const char contents[] =
		"[SMSBackup000]\n"
		"MessageReference = 7\n"
		"FirstPart = preserved\n"
		"[SMSBackup001]\n"
		"BeforeReference = preserved\n"
		"MessageReference = 8\n"
		"AfterReference = preserved\n";
	static const char expected[] =
		"[SMSBackup000]\n"
		"MessageReference = 7\n"
		"FirstPart = preserved\n"
		"[SMSBackup001]\n"
		"BeforeReference = preserved\n"
		"MessageReference = 42\n"
		"AfterReference = preserved\n";
	GSM_MultiSMSMessage sms;
	GSM_SMSDConfig config;
	GSM_Error error;
	char *full_path;
	char buffer[sizeof(expected) + 1];
	size_t full_path_length;
	size_t length;
	FILE *file = NULL;
	int fd;
	int result = 0;
#ifdef WIN32
	BY_HANDLE_FILE_INFORMATION original_status;
	BY_HANDLE_FILE_INFORMATION updated_status;
#else
	struct stat original_status;
	struct stat updated_status;
#endif

	full_path_length = strlen(outbox_path) + strlen(sent_info_filename) + 1;
	full_path = malloc(full_path_length);
	if (full_path == NULL) {
		fprintf(stderr, "Failed to allocate sent-info path\n");
		return 1;
	}
	snprintf(full_path, full_path_length, "%s%s", outbox_path, sent_info_filename);

	remove(full_path);
#ifdef WIN32
	{
		HANDLE file_handle = CreateFileA(
			full_path, GENERIC_WRITE, 0, NULL, CREATE_NEW,
			read_only ? FILE_ATTRIBUTE_READONLY : FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (file_handle == INVALID_HANDLE_VALUE) {
			fprintf(stderr, "Failed to create sent-info file\n");
			result = 1;
			goto cleanup;
		}
		fd = _open_osfhandle((intptr_t)file_handle, _O_WRONLY | _O_BINARY);
		if (fd < 0) {
			CloseHandle(file_handle);
			fprintf(stderr, "Failed to create sent-info descriptor\n");
			result = 1;
			goto cleanup;
		}
	}
#else
	fd = open(full_path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (fd < 0) {
		fprintf(stderr, "Failed to create sent-info file\n");
		result = 1;
		goto cleanup;
	}
#endif
	file = fdopen(fd, "wb");
	if (file == NULL) {
#ifdef WIN32
		_close(fd);
#else
		close(fd);
#endif
		fprintf(stderr, "Failed to open sent-info stream\n");
		result = 1;
		goto cleanup;
	}
	if (fwrite(contents, 1, strlen(contents), file) != strlen(contents)) {
		fprintf(stderr, "Failed to create sent-info file\n");
		result = 1;
		goto cleanup;
	}
#ifdef WIN32
	if (!GetFileInformationByHandle((HANDLE)_get_osfhandle(fd), &original_status) ||
	    (((original_status.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0) !=
	     read_only)) {
		fprintf(stderr, "Sent-info file has unexpected attributes\n");
		result = 1;
		goto cleanup;
	}
#else
	if ((read_only && fchmod(fd, 0400) != 0) ||
	    fstat(fd, &original_status) != 0) {
		fprintf(stderr, "Failed to prepare sent-info file\n");
		result = 1;
		goto cleanup;
	}
#endif
	if (fclose(file) != 0) {
		file = NULL;
		fprintf(stderr, "Failed to close sent-info file\n");
		result = 1;
		goto cleanup;
	}
	file = NULL;

	SetupConfig(&config, state_machine, outbox_path, outbox_path, outbox_path);
	strcpy((char *)config.SMSID, sent_info_filename);
	memset(&sms, 0, sizeof(sms));
	sms.Number = 2;
	GSM_SetDefaultSMSData(&sms.SMS[0]);
	GSM_SetDefaultSMSData(&sms.SMS[1]);
	EncodeUnicode(sms.SMS[0].Number, "12345", strlen("12345"));

	error = SMSDFiles.AddSentSMSInfo(
		&sms, &config, (char *)config.SMSID, 2, SMSD_SEND_OK, 42);
	if (error != ERR_NONE) {
		fprintf(stderr, "Failed to update %ssent-info file: %s\n",
			read_only ? "non-writable " : "", GSM_ErrorString(error));
		result = 1;
		goto cleanup;
	}

	file = fopen(full_path, "rb");
	if (file == NULL) {
		fprintf(stderr, "Failed to read updated sent-info file\n");
		result = 1;
		goto cleanup;
	}
	length = fread(buffer, 1, sizeof(buffer) - 1, file);
	buffer[length] = '\0';

#ifdef WIN32
	if (!GetFileInformationByHandle(
			(HANDLE)_get_osfhandle(fileno(file)), &updated_status) ||
	    (updated_status.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0) {
		fprintf(stderr, "Sent-info update left the file read-only\n");
		result = 1;
		goto cleanup;
	}
	if (!read_only &&
	    (updated_status.dwVolumeSerialNumber != original_status.dwVolumeSerialNumber ||
	     updated_status.nFileIndexHigh != original_status.nFileIndexHigh ||
	     updated_status.nFileIndexLow != original_status.nFileIndexLow)) {
		fprintf(stderr, "Writable sent-info update replaced the file\n");
		result = 1;
		goto cleanup;
	}
#else
	if (fstat(fileno(file), &updated_status) != 0 ||
	    (updated_status.st_mode & 07777) != (original_status.st_mode & 07777) ||
	    updated_status.st_uid != original_status.st_uid ||
	    updated_status.st_gid != original_status.st_gid) {
		fprintf(stderr, "Sent-info update did not preserve file permissions\n");
		result = 1;
		goto cleanup;
	}
	if (!read_only &&
	    (updated_status.st_dev != original_status.st_dev ||
	     updated_status.st_ino != original_status.st_ino)) {
		fprintf(stderr, "Writable sent-info update replaced the file\n");
		result = 1;
		goto cleanup;
	}
#endif

	if (fclose(file) != 0) {
		file = NULL;
		fprintf(stderr, "Failed to close updated sent-info file\n");
		result = 1;
		goto cleanup;
	}
	file = NULL;
	if (strcmp(buffer, expected) != 0) {
		fprintf(stderr, "Sent-info update changed unexpected content\n");
		result = 1;
		goto cleanup;
	}
#ifdef WIN32
	if (read_only) {
		error = SMSDFiles.MoveSMS(
			&sms, &config, (char *)config.SMSID, FALSE, TRUE);
		if (error != ERR_NONE ||
		    GetFileAttributesA(full_path) != INVALID_FILE_ATTRIBUTES) {
			fprintf(stderr,
				"Failed to remove updated read-only outbox file\n");
			result = 1;
			goto cleanup;
		}
	}
#endif
	if (HasTemporaryFile(outbox_path)) {
		fprintf(stderr, "Sent-info update left a temporary file\n");
		result = 1;
		goto cleanup;
	}

cleanup:
	if (file != NULL) {
		fclose(file);
	}
#ifdef WIN32
	{
		DWORD attributes = GetFileAttributesA(full_path);

		if (attributes != INVALID_FILE_ATTRIBUTES) {
			attributes &= ~FILE_ATTRIBUTE_READONLY;
			SetFileAttributesA(
				full_path, attributes == 0 ? FILE_ATTRIBUTE_NORMAL : attributes);
		}
	}
#endif
	remove(full_path);
	free(full_path);
	return result;
}

static int TestTemporaryFileIgnored(GSM_StateMachine *state_machine,
				    const char *outbox_path)
{
	static const char filename[] = ".ABCDEF";
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
	if (error != ERR_EMPTY) {
		fprintf(stderr, "Temporary outbox file returned %s\n",
			GSM_ErrorString(error));
		result = 1;
	}

	remove(full_path);
	free(full_path);
	return result;
}

#ifndef WIN32
static int TestSymlinkIgnored(GSM_StateMachine *state_machine,
			      const char *outbox_path)
{
	static const char target_filename[] = "source.smsbackup";
	static const char symlink_filename[] = "OUT-symlink.smsbackup";
	GSM_MultiSMSMessage sms;
	GSM_SMSDConfig config;
	GSM_Error error;
	char id[GSM_MAX_FILENAME_LENGTH + 1];
	char *target_path;
	char *symlink_path;
	size_t symlink_path_length;
	int result = 0;

	target_path = CreateOutboxFile(outbox_path, target_filename);
	if (target_path == NULL) {
		return 1;
	}
	symlink_path_length = strlen(outbox_path) + strlen(symlink_filename) + 1;
	symlink_path = malloc(symlink_path_length);
	if (symlink_path == NULL) {
		remove(target_path);
		free(target_path);
		return 1;
	}
	snprintf(symlink_path, symlink_path_length, "%s%s",
		 outbox_path, symlink_filename);
	remove(symlink_path);
	if (symlink(target_path, symlink_path) != 0) {
		fprintf(stderr, "Failed to create outbox symlink\n");
		result = 1;
		goto cleanup;
	}

	memset(&sms, 0, sizeof(sms));
	memset(id, 0, sizeof(id));
	SetupConfig(&config, state_machine, outbox_path, outbox_path, outbox_path);
	error = SMSDFiles.FindOutboxSMS(&sms, &config, id);
	if (error != ERR_EMPTY) {
		fprintf(stderr, "Symlink outbox file returned %s\n",
			GSM_ErrorString(error));
		result = 1;
	}

cleanup:
	remove(symlink_path);
	remove(target_path);
	free(symlink_path);
	free(target_path);
	return result;
}

static void RemoveLongOutboxPath(char *path, size_t base_length)
{
	char *separator;
	size_t length;

	while ((length = strlen(path)) > base_length) {
		if (path[length - 1] == '/') {
			path[length - 1] = '\0';
		}
		rmdir(path);
		separator = strrchr(path, '/');
		if (separator == NULL) {
			break;
		}
		separator[1] = '\0';
	}
}

static char *CreateLongOutboxPath(const char *base_path)
{
	static const char directory_template[] = ".smsd-long-XXXXXX";
	char *path;
	size_t base_length = strlen(base_path);
	size_t component_length;
	size_t current_length;
	size_t remaining;
	size_t target_length;

	target_length = TEST_FILES_PATH_MAX - strlen(sent_info_filename) - 1;
	if (base_length + sizeof(directory_template) >= target_length) {
		fprintf(stderr, "Test outbox path is too long\n");
		return NULL;
	}
	path = malloc(target_length + 1);
	if (path == NULL) {
		fprintf(stderr, "Failed to allocate long outbox path\n");
		return NULL;
	}
	snprintf(path, target_length + 1, "%s%s", base_path, directory_template);
	if (mkdtemp(path) == NULL) {
		fprintf(stderr, "Failed to create long outbox root\n");
		free(path);
		return NULL;
	}
	current_length = strlen(path);
	path[current_length++] = '/';
	path[current_length] = '\0';

	while (current_length < target_length) {
		remaining = target_length - current_length;
		if (remaining < 2) {
			fprintf(stderr, "Could not construct long outbox path\n");
			RemoveLongOutboxPath(path, base_length);
			free(path);
			return NULL;
		}
		component_length = remaining > 201 ? 200 : remaining - 1;
		if (remaining - component_length - 1 == 1) {
			component_length--;
		}
		memset(path + current_length, 'd', component_length);
		current_length += component_length;
		path[current_length] = '\0';
		if (mkdir(path, 0700) != 0) {
			fprintf(stderr, "Failed to create long outbox directory\n");
			RemoveLongOutboxPath(path, base_length);
			free(path);
			return NULL;
		}
		path[current_length++] = '/';
		path[current_length] = '\0';
	}
	return path;
}

static int TestLongSentInfoPath(GSM_StateMachine *state_machine,
				const char *outbox_path)
{
	char *long_outbox_path;
	int result;

	long_outbox_path = CreateLongOutboxPath(outbox_path);
	if (long_outbox_path == NULL) {
		return 1;
	}
	result = TestSentInfo(state_machine, long_outbox_path, TRUE);
	RemoveLongOutboxPath(long_outbox_path, strlen(outbox_path));
	free(long_outbox_path);
	return result;
}
#endif

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
	if (result == 0) {
		result = TestTemporaryFileIgnored(state_machine, argv[1]);
	}
#ifndef WIN32
	if (result == 0) {
		result = TestSymlinkIgnored(state_machine, argv[1]);
	}
#endif
	if (result == 0) {
		result = TestSentInfo(state_machine, argv[1], FALSE);
	}
	if (result == 0) {
		result = TestSentInfo(state_machine, argv[1], TRUE);
	}
#ifndef WIN32
	if (result == 0) {
		result = TestLongSentInfoPath(state_machine, argv[1]);
	}
#endif

	GSM_FreeStateMachine(state_machine);
	return result;
}
