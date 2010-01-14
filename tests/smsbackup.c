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
        int i;

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

        for (i = 0; Backup.SMS[i] != NULL; i++) {
		SMS.Number = 1;
		SMS.SMS[0] = *Backup.SMS[i];
                DisplayMultiSMSInfo(&SMS, FALSE, TRUE, NULL, NULL);
	}
	/*
	 * TODO: Should link messages here and display concatenated
	 * ones.
	 */

	/* We don't need this anymore */
	GSM_FreeSMSBackup(&Backup);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

