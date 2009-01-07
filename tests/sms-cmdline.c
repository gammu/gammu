#include <gammu.h>
#include <stdlib.h>

#include "common.h"

#include "../helper/message-display.h"
#include "../helper/message-cmdline.h"

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_MultiSMSMessage sms;
	GSM_Error error;
	GSM_Message_Type type = SMS_Display;

	error = CreateMessage(&type, &sms, argc, 1, argv, NULL);
	test_result(error == ERR_NONE);

	DisplayMultiSMSInfo(&sms, false, true, NULL, NULL);
	DisplayMultiSMSInfo(&sms, true, true, NULL, NULL);

	printf("\n");
	printf("Number of messages: %i", sms.Number);
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
