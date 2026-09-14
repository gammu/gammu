#include <gammu.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "../smsd/core.h"
#include "../smsd/services/files.h"
#include "common.h"

GSM_Error SMSD_SendSMS(GSM_SMSDConfig *config);

static void WriteMessage(const char *directory, const char *id, gboolean backup)
{
	char path[4096];
	FILE *file;
	int fd;

	test_result((size_t)snprintf(path, sizeof(path), "%s%s", directory, id) < sizeof(path));
	if (backup) {
#ifdef GSM_ENABLE_BACKUP
		GSM_SMS_Backup *data = malloc(sizeof(*data));
		GSM_SMSMessage sms;

		test_result(data != NULL);
		GSM_ClearSMSBackup(data);
		GSM_SetDefaultSMSData(&sms);
		EncodeUnicode(sms.Number, "12345", 5);
		EncodeUnicode(sms.Text, "retry test", 10);
		sms.Length = 10;
		data->SMS[0] = &sms;
		test_result(GSM_AddSMSBackupFile(path, data) == ERR_NONE);
		free(data);
#else
		test_result(FALSE);
#endif
	} else {
		fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
		test_result(fd >= 0);
		file = fdopen(fd, "wb");
		if (file == NULL) {
			close(fd);
		}
		test_result(file != NULL);
		test_result(fputs("retry test", file) >= 0);
		test_result(fclose(file) == 0);
	}
}

static void RemoveMessage(const char *directory, const char *id)
{
	char path[4096];

	test_result((size_t)snprintf(path, sizeof(path), "%s%s", directory, id) < sizeof(path));
	test_result(remove(path) == 0);
}

static void CheckRetries(const char *outbox, const char *errorbox,
			 unsigned int maxretries, gboolean backup, gboolean read_smsc)
{
	GSM_SMSDConfig *config = SMSD_NewConfig("files-retries-test");
	GSM_SMSDStatus status;
	GSM_MultiSMSMessage *sms = malloc(sizeof(*sms));
	char id[GSM_MAX_FILENAME_LENGTH + 1];
	const char *first = backup ? "OUTB_12345_1.smsbackup" : "OUTB_12345_1.txt";
#ifndef WIN32
	const char *priority = "OUTA_12345_1.txt";
#endif
	const char *last = "OUTC_12345_1.txt";
	unsigned int attempt;

	test_result(config != NULL);
	test_result(sms != NULL);
	memset(&status, 0, sizeof(status));
	config->gsm = GSM_AllocStateMachine();
	test_result(config->gsm != NULL);
	config->Service = &SMSDFiles;
	config->connected = FALSE;
	config->Status = &status;
	config->outboxpath = outbox;
	config->sentsmspath = errorbox;
	config->errorsmspath = errorbox;
	config->transmitformat = "auto";
	config->deliveryreport = "no";
	config->maxretries = maxretries;
	config->checkbattery = FALSE;
	config->checksignal = FALSE;
	config->checknetwork = FALSE;
	memset(&config->SMSC, 0, sizeof(config->SMSC));
	memset(&config->SMSCCache, 0, sizeof(config->SMSCCache));
	memset(config->SkipMessage, 0, sizeof(config->SkipMessage));
	config->relativevalidity = -1;
	if (!read_smsc) {
		EncodeUnicode(config->SMSC.Number, "123", 3);
	}

	WriteMessage(outbox, first, backup);
#ifndef WIN32
	WriteMessage(outbox, last, FALSE);
#endif
	for (attempt = 0; attempt <= maxretries; attempt++) {
		/* A disconnected phone provides deterministic failures without hardware. */
		test_result(SMSD_SendSMS(config) == ERR_UNKNOWN);
		test_result(strcmp((char *)config->SMSID, first) == 0);
		test_result(SMSDFiles.FindOutboxSMS(sms, config, id) == ERR_NONE);
		test_result(config->retries == attempt + 1);
#ifndef WIN32
		if (attempt == 0) {
			/* A higher-priority message must not reset the older message's count. */
			WriteMessage(outbox, priority, FALSE);
			test_result(SMSDFiles.FindOutboxSMS(sms, config, id) == ERR_NONE);
			test_result(strcmp(id, priority) == 0);
			test_result(config->retries == 0);
			test_result(SMSDFiles.MoveSMS(sms, config, id, FALSE, TRUE) == ERR_NONE);
			RemoveMessage(errorbox, priority);
		}
#endif
	}
	/* The next scan exhausts the limit and moves the failed message aside. */
	test_result(SMSD_SendSMS(config) == ERR_UNKNOWN);
	RemoveMessage(errorbox, first);
#ifdef WIN32
	/* Windows uses directory enumeration order and prefers text files. */
	WriteMessage(outbox, last, FALSE);
#endif
	test_result(SMSDFiles.FindOutboxSMS(sms, config, id) == ERR_NONE);
	test_result(strcmp(id, last) == 0);
	test_result(config->retries == 0);

	/* External removal must release state, including on an empty scan. */
	config->retries = 5;
	test_result(SMSDFiles.UpdateRetries(config, id) == ERR_NONE);
	RemoveMessage(outbox, last);
	test_result(SMSDFiles.FindOutboxSMS(sms, config, id) == ERR_EMPTY);
	test_result(config->files_retries == NULL);
	WriteMessage(outbox, last, FALSE);
	test_result(SMSDFiles.FindOutboxSMS(sms, config, id) == ERR_NONE);
	test_result(config->retries == 0);

	/* Restarting the backend resets retry state; Free is safe to repeat. */
	config->retries = 5;
	test_result(SMSDFiles.UpdateRetries(config, id) == ERR_NONE);
	test_result(SMSDFiles.Free(config) == ERR_NONE);
	test_result(SMSDFiles.Free(config) == ERR_NONE);
	test_result(SMSDFiles.FindOutboxSMS(sms, config, id) == ERR_NONE);
	test_result(config->retries == 0);
	RemoveMessage(outbox, last);
	config->Status = NULL;
	test_result(SMSDFiles.Free(config) == ERR_NONE);
	SMSD_FreeConfig(config);
	free(sms);
}

int main(int argc, char **argv)
{
	test_result(argc == 3);
	CheckRetries(argv[1], argv[2], 0, FALSE, FALSE);
	CheckRetries(argv[1], argv[2], 1, FALSE, FALSE);
	CheckRetries(argv[1], argv[2], 3, FALSE, FALSE);
	CheckRetries(argv[1], argv[2], 1, FALSE, TRUE);
#ifdef GSM_ENABLE_BACKUP
	CheckRetries(argv[1], argv[2], 1, TRUE, FALSE);
#endif
	return 0;
}
