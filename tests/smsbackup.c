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
	GSM_MultiSMSMessage SMS;
        int i, step = 0;

	/* Check parameters */
	if (argc != 2) {
		printf("Not enough parameters!\nUsage: smsbackup file.smsbackup\n");
		return 1;
	}

	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Allocates state machine */
	error = GSM_ReadSMSBackupFile(argv[1], &Backup);
	gammu_test_result(error, "GSM_ReadSMSBackupFile");


	do {
		for (i = step; Backup.SMS[i] != NULL && i - step < GSM_MAX_MULTI_SMS; i++) {
			SMS.Number = i + 1 - step;
			SMS.SMS[i - step] = *Backup.SMS[i];
		}
		if (i - step == GSM_MAX_MULTI_SMS) {
			step = i;
		} else {
			step = 0;
		}

		DisplayMultiSMSInfo(&SMS, TRUE, TRUE, NULL, NULL);
	} while (step > 0);

	/* We don't need this anymore */
	GSM_FreeSMSBackup(&Backup);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

