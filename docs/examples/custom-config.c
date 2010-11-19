/*
 * libGammu example to show how to set configuration manually instead
 * of parsing ~/.gammurc
 */
#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

GSM_StateMachine *s;
GSM_Error error;
char buffer[100];

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

int main(int argc, char **argv)
{
	GSM_Debug_Info *debug_info;
	GSM_Config *cfg;

	if (argc != 4) {
		printf("Usage: custom-config DEVICE CONNECTION MODEL\n");
	}

	/*
	 * We don't need gettext, but need to set locales so that
	 * charset conversion works.
	 */
	GSM_InitLocales(NULL);

	/* Enable global debugging to stderr */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL)
		return 3;

	/*
	 * Enable state machine debugging to same config as global one.
	 */
	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(TRUE, debug_info);

	/*
	 * Get pointer to config structure.
	 */
	cfg = GSM_GetConfig(s, 0);

	/*
	 * Set configuration, first freeing old values.
	 */
	free(cfg->Device);
	cfg->Device = strdup(argv[1]);
	free(cfg->Connection);
	cfg->Connection = strdup(argv[2]);
	/* For historical reasons this is not a pointer */
	strcpy(cfg->Model, argv[3]);

	/* We have one valid configuration */
	GSM_SetConfigNum(s, 1);

	/* Connect to phone */
	/* 1 means number of replies you want to wait for */
	error = GSM_InitConnection(s, 1);
	error_handler();

	/* Here you can do some stuff with phone... */

	/* As an example we read some information about phone: */

	/* Manufacturer name */
	error = GSM_GetManufacturer(s, buffer);
	error_handler();
	printf("Manufacturer  : %s\n", buffer);

	/* Model name */
	error = GSM_GetModel(s, buffer);
	error_handler();
	printf("Model         : %s (%s)\n",
		GSM_GetModelInfo(s)->model,
		buffer);

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

