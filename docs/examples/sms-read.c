#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

GSM_StateMachine *s;
INI_Section *cfg;
GSM_Error error;
volatile GSM_Error sms_send_status;
volatile gboolean gshutdown = FALSE;

/* Function to handle errors */
void error_handler(void)
{
	if (error != ERR_NONE) {
		printf("ERROR: %s\n", GSM_ErrorString(error));
		if (GSM_IsConnected(s))
			GSM_TerminateConnection(s);
		exit(error);
	}
}

/* Interrupt signal handler */
void interrupt(int sign)
{
	signal(sign, SIG_IGN);
	gshutdown = TRUE;
}

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_Debug_Info *debug_info;
	gboolean start;
	GSM_MultiSMSMessage 	sms;
	int i;

	/* Register signal handler */
	signal(SIGINT, interrupt);
	signal(SIGTERM, interrupt);

	/*
	 * We don't need gettext, but need to set locales so that
	 * charset conversion works.
	 */
	GSM_InitLocales(NULL);

	/* Enable global debugging to stderr */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, TRUE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL)
		return 3;

	/*
	 * Enable state machine debugging to stderr
	 * Same could be achieved by just using global debug config.
	 */
	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(FALSE, debug_info);
	GSM_SetDebugFileDescriptor(stderr, TRUE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/*
	 * Find configuration file (first command line parameter or
	 * defaults)
	 */
	error = GSM_FindGammuRC(&cfg, argc == 2 ? argv[1] : NULL);
	error_handler();

	/* Read it */
	error = GSM_ReadConfig(cfg, GSM_GetConfig(s, 0), 0);
	error_handler();

	/* Free config file structures */
	INI_Free(cfg);

	/* We have one valid configuration */
	GSM_SetConfigNum(s, 1);

	/* Connect to phone */
	/* 1 means number of replies you want to wait for */
	error = GSM_InitConnection(s, 1);
	error_handler();

	/* Read all messages */
	error = ERR_NONE;
	start = TRUE;
	sms.Number = 0;
	sms.SMS[0].Location = 0;
	sms.SMS[0].Folder = 0;
	while (error == ERR_NONE && !gshutdown) {
		error = GSM_GetNextSMS(s, &sms, start);
		if (error == ERR_EMPTY) break;
		error_handler();
		start = FALSE;

		/* Now we can do something with the message */
		for (i = 0; i < sms.Number; i++) {
			printf("Location: %d, Folder: %d\n", sms.SMS[i].Location, sms.SMS[i].Folder);
			printf("Number: \"%s\"\n", DecodeUnicodeConsole(sms.SMS[i].Number));
			/*
			 * Decoding with GSM_DecodeMultiPartSMS is also an option here,
			 * but for simplicity we use this approach which will handle only
			 * text messages.
			 */
			if (sms.SMS[i].Coding == SMS_Coding_8bit) {
				printf("8-bit message, can not display\n");
			} else {
				printf("Text: \"%s\"\n", DecodeUnicodeConsole(sms.SMS[i].Text));
			}
			printf("\n");
		}
	}

	/* Terminate connection */
	error = GSM_TerminateConnection(s);
	error_handler();

	/* Free up used memory */
	GSM_FreeStateMachine(s);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */

