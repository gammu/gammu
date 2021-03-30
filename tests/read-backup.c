#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"

GSM_StateMachine *s;

int main(int argc, char **argv)
{
	GSM_Error error;
	GSM_Backup backup;
	GSM_Debug_Info *debug_info;

    if (argc != 2) {
        printf("Usage: read-backup backup\n");
        exit(1);
    }

	/* Configure debugging */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	error = GSM_ReadBackupFile(argv[1], &backup, GSM_GuessBackupFormat(argv[1], FALSE));
	gammu_test_result(error, "Read backup");

    GSM_FreeBackup(&backup);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
