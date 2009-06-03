/* Simple C program to start SMSD without all magic normal gammu-smsd does */
#include <gammu-smsd.h>
#include <assert.h>

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_SMSDConfig *config;
    GSM_Error error;
    char *config_file = NULL; /* Use default compiled in path */

    /* Initalize configuration with program name */
	config = SMSD_NewConfig("smsd-example");
	assert(config != NULL);

    /* Read configuration file */
	error = SMSD_ReadConfig(config_file, config, TRUE);
	if (error != ERR_NONE) {
		printf("Failed to read config!\n");
		SMSD_FreeConfig(config);
		return 2;
	}

    /* Start main SMSD loop which processes messages */
    /*
     * This normally never terminates, you need to signal it
     * by SMSD_Shutdown(config); (for example from signal handler)
     * to make it stop.
     */
	error = SMSD_MainLoop(config, FALSE);
	if (error != ERR_NONE) {
		printf("Failed to run SMSD!\n");
		SMSD_FreeConfig(config);
		return 2;
	}

    /* Free configuration structure */
	SMSD_FreeConfig(config);

    return 0;
}
