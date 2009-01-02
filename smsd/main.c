/**
 * Main SMSD program
 */
/* Copyright (c) 2009 Michal Cihar <michal@cihar.com> */

#include <signal.h>
#include <gammu-smsd.h>
#include <assert.h>
#include <stdlib.h>

GSM_SMSDConfig *config;

void smsd_interrupt(int signum)
{
	SMSD_Shutdown(config);
	signal(signum, SIG_IGN);
}

void help(void)
{
    printf("usage: gammu-smsd CONFIG_FILE\n");
}

int main(int argc, char **argv)
{
	GSM_Error error;
	char *filename;

	config = SMSD_NewConfig();
	assert(config != NULL);

    if (argc != 2) {
        help();
        exit(1);
    }

	/* FIXME: This is cruel hack */
	filename = argv[1];

	error = SMSD_ReadConfig(filename, config, true, NULL);
	if (error != ERR_NONE) {
		SMSD_Terminate(config, "Failed to read config", error, true, 2);
	}

	signal(SIGINT, smsd_interrupt);
	signal(SIGTERM, smsd_interrupt);

	error = SMSD_MainLoop(config);
	if (error != ERR_NONE) {
		SMSD_Terminate(config, "Failed to run SMSD", error, true, 2);
	}

	SMSD_FreeConfig(config);

	return 0;
}
