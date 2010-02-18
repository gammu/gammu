/**
 * SMS backup testing program.
 */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>

#include "../helper/message-display.h"

#include "common.h"

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Debug_Info *debug_info;
	GSM_Error error;
	GSM_SMS_Backup Backup;
	GSM_MultiSMSMessage **SortedSMS, **InputSMS;
	int i, count;

	/* Check parameters */
	if (argc != 2) {
		printf("Not enough parameters!\nUsage: smsbackup file.smsbackup\n");
		return 1;
	}

	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Read the backup */
	error = GSM_ReadSMSBackupFile(argv[1], &Backup);
	gammu_test_result(error, "GSM_ReadSMSBackupFile");

	/* Calculate number of messages */
	count = 0;
	while (Backup.SMS[count] != NULL) {
		count++;
	}

	/* Allocate memory for sorted ones */
	SortedSMS = (GSM_MultiSMSMessage **)malloc((count + 1) * sizeof(GSM_MultiSMSMessage *));
	InputSMS = (GSM_MultiSMSMessage **)malloc((count + 1) * sizeof(GSM_MultiSMSMessage *));

	/* Copy messages to multi message buffers */
	for (i = 0; i < count; i++) {
		InputSMS[i] = (GSM_MultiSMSMessage *)malloc(sizeof(GSM_MultiSMSMessage));
		InputSMS[i]->Number = 1;
		InputSMS[i]->SMS[0] = *Backup.SMS[i];
	}
	InputSMS[i] = NULL;

	/* Sort linked messages */
	error = GSM_LinkSMS(debug_info, InputSMS, SortedSMS, TRUE);

	/* Display messages */
	for (i = 0; SortedSMS[i] != NULL; i++) {
		DisplayMultiSMSInfo(SortedSMS[i], TRUE, TRUE, NULL, NULL);
	}

	/* We don't need this anymore */
	GSM_FreeSMSBackup(&Backup);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

