#include <gammu.h>
#include <stdlib.h>

#include "common.h"

#include "../helper/message-display.h"
#include "../helper/message-cmdline.h"

int main(int argc, char **argv)
{
	GSM_MultiSMSMessage sms;
	GSM_Error error;
	GSM_Message_Type type = SMS_Display;
	GSM_Debug_Info *debug_info;
	int i;

	/* Configure debugging */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	/* No debugging as it messes up checking results */
	GSM_SetDebugLevel("none", debug_info);

	error = CreateMessage(&type, &sms, argc, 1, argv, NULL);
	gammu_test_result(error, "CreateMessage");

	DisplayMultiSMSInfo(&sms, FALSE, TRUE, NULL, NULL);
	DisplayMultiSMSInfo(&sms, TRUE, TRUE, NULL, NULL);

	for (i = 0; i < sms.Number; i++) {
		printf("Message number: %i\n", i);
		sms.SMS[i].SMSC.Location = 0;
		DisplaySMSFrame(&sms.SMS[i], NULL);
	}

	printf("\n");
	printf("Number of messages: %i\n", sms.Number);
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
