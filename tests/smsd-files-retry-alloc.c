#include <gammu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../smsd/core.h"
#include "common.h"

static gboolean fail_allocation;

static void *retry_malloc(size_t size)
{
	if (fail_allocation) {
		fail_allocation = FALSE;
		return NULL;
	}
	return malloc(size);
}

/* Inject failure only into allocations made by the Files backend. */
#define malloc retry_malloc
#define SMSDFiles TestSMSDFiles
#define SMSD_Check_Dir TestSMSD_Check_Dir
#define SMSDFiles_ReadConfiguration TestSMSDFiles_ReadConfiguration
#include "../smsd/services/files.c"
#undef malloc

GSM_Error SMSD_SendSMS(GSM_SMSDConfig *config);

int main(int argc, char **argv)
{
	GSM_SMSDConfig config;
	GSM_SMSDStatus status;
	GSM_MultiSMSMessage *sms = malloc(sizeof(*sms));
	char path[4096], id[GSM_MAX_FILENAME_LENGTH + 1];
	FILE *file;
	int fd;

	test_result(argc == 2);
	test_result(sms != NULL);
	memset(&config, 0, sizeof(config));
	memset(&status, 0, sizeof(status));
	config.gsm = GSM_AllocStateMachine();
	test_result(config.gsm != NULL);
	config.Service = &TestSMSDFiles;
	config.Status = &status;
	config.outboxpath = argv[1];
	/* The error path equals the outbox, so a mistaken MoveSMS deletes it. */
	config.errorsmspath = argv[1];
	config.sentsmspath = argv[1];
	config.transmitformat = "auto";
	test_result((size_t)snprintf(path, sizeof(path), "%sOUT12345.txt", argv[1]) < sizeof(path));
	fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	test_result(fd >= 0);
	file = fdopen(fd, "wb");
	if (file == NULL) {
		close(fd);
	}
	test_result(file != NULL);
	test_result(fputs("preserve this message", file) >= 0);
	test_result(fclose(file) == 0);

	fail_allocation = TRUE;
	test_result(SMSD_SendSMS(&config) == ERR_MOREMEMORY);
	test_result(!fail_allocation);
	test_result(config.SMSID[0] == '\0');
	test_result(status.Failed == 0);
	test_result(config.files_retries == NULL);
	/* A later scan must still find the valid message, with no attempts used. */
	test_result(TestSMSDFiles.FindOutboxSMS(sms, &config, id) == ERR_NONE);
	test_result(strcmp(id, "OUT12345.txt") == 0);
	test_result(config.retries == 0);
	test_result(strcmp(DecodeUnicodeString(sms->SMS[0].Text), "preserve this message") == 0);
	test_result(remove(path) == 0);
	test_result(TestSMSDFiles.Free(&config) == ERR_NONE);
	GSM_FreeStateMachine(config.gsm);
	free(sms);
	return 0;
}
