#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>

GSM_StateMachine *s;
INI_Section *cfg;
GSM_Error error;
char buffer[100];

/* Function to handle errors */
void error_handler()
{
	if (error != ERR_NONE) {
		printf("%s\n", GSM_ErrorString(error));
		if (GSM_IsConnected(s))
			GSM_TerminateConnection(s);
		exit(error);
	}
}

int main(int argc, char **argv)
{
	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL)
		return 3;

	/* Find configuration file */
	error = GSM_FindGammuRC(&cfg);
	error_handler();

	/* Read it */
	error = GSM_ReadConfig(cfg, GSM_GetConfig(s, 0), 0);
	error_handler();

	/* We have one valid configuration */
	GSM_SetConfigNum(s, 1);

	/* Connect to phone */
	error = GSM_InitConnection(s, 3);	/* 3 means number of replies you want to wait for */
	error_handler();

	/* Here you can do some stuff with phone... */

	/* As example we read some information about phone: */

	/* Manufacturer name */
	error = GSM_GetManufacturer(s, buffer);
	error_handler();
	printf("Manufacturer  : %s\n", buffer);

	/* Model name */
	error = GSM_GetModel(s, buffer);
	error_handler();
	printf("Model         : %s (%s)\n", GSM_GetModelInfo(s)->model, buffer);

	/* Terminate connection */
	error = GSM_TerminateConnection(s);
	error_handler();
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
