/**
 * Main SMSD program
 */
/* Copyright (c) 2009 Michal Cihar <michal@cihar.com> */

#include <signal.h>
#include <gammu-smsd.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

GSM_SMSDConfig *config;

void smsd_interrupt(int signum)
{
	SMSD_Shutdown(config);
	signal(signum, SIG_IGN);
}

#ifdef HAVE_GETOPT
void help(void)
{
    printf("usage: gammu-smsd [-c CONFIG_FILE] [-h] [-d]\n");
}
#else
void help(void)
{
    printf("usage: gammu-smsd CONFIG_FILE\n");
}
#endif

int main(int argc, char **argv)
{
	GSM_Error error;
	char *filename = NULL;
    int opt;
    bool daemonize = false;

	config = SMSD_NewConfig();
	assert(config != NULL);

#ifdef HAVE_GETOPT
    while ((opt = getopt(argc, argv, "h?dc:")) != -1) {
        switch (opt) {
            case 'c':
                filename = optarg;
                break;
            case 'd':
                daemonize = true;
                break;
            case '?':
            case 'h':
                help();
                exit(0);
        }
    }

#else
    if (argc != 2) {
        help();
        exit(1);
    }

	/* FIXME: This is cruel hack */
	filename = argv[1];
#endif

    if (filename == NULL) {
        printf("No config file specified!\n");
        exit(1);
    }

	error = SMSD_ReadConfig(filename, config, true, NULL);
	if (error != ERR_NONE) {
		SMSD_Terminate(config, "Failed to read config", error, true, 2);
	}

	signal(SIGINT, smsd_interrupt);
	signal(SIGTERM, smsd_interrupt);

    if (daemonize) {
#ifdef HAVE_DAEMON
        daemon(1, 0);
#else
        printf("daemon mode is not supported on your platform!\n");
        exit(1);
#endif
    }

	error = SMSD_MainLoop(config);
	if (error != ERR_NONE) {
		SMSD_Terminate(config, "Failed to run SMSD", error, true, 2);
	}

	SMSD_FreeConfig(config);

	return 0;
}
