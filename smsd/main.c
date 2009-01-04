/**
 * Main SMSD program
 */
/* Copyright (c) 2009 Michal Cihar <michal@cihar.com> */

#include <signal.h>
#include <gammu-smsd.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#endif

GSM_SMSDConfig *config;

void smsd_interrupt(int signum)
{
	SMSD_Shutdown(config);
	signal(signum, SIG_IGN);
}

NORETURN
void version(void)
{
	printf("Gammu-smsd version %s\n", VERSION);
    printf("Built %s on %s using %s\n", __TIME__, __DATE__, GetCompiler());
    printf("Compiled in features:\n");
#ifdef HAVE_DAEMON
	printf("  - %s\n", "DAEMON");
#endif
#ifdef HAVE_GETOPT
	printf("  - %s\n", "GETOPT");
#endif
#ifdef HAVE_GETOPT_LONG
	printf("  - %s\n", "GETOPT_LONG");
#endif
#ifdef HAVE_MYSQL_MYSQL_H
	printf("  - %s\n", "MYSQL");
#endif
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	printf("  - %s\n", "POSTGRESQL");
#endif
    exit(0);
}

#if defined(HAVE_GETOPT) || defined(HAVE_GETOPT_LONG)

void help(void)
{
    printf("usage: gammu-smsd [-c CONFIG_FILE] [-h] [-d]\n");
}

NORETURN
void wrong_params(void)
{
    printf("Invalid parameter, use --help for help.\n");
    exit(1);
}

void process_commandline(int argc, char **argv, bool *daemonize, char **filename)
{
    int opt;

#ifdef HAVE_GETOPT_LONG
    struct option long_options[] = {
       {"help"      , 0, 0, 'h'},
       {"version"   , 0, 0, 'v'},
       {"config"    , 1, 0, 'c'},
       {"daemon"    , 0, 0, 'd'},
       {"pid"       , 1, 0, 'p'},
       {0           , 0, 0, 0}
    };
    int option_index;

    while ((opt = getopt_long(argc, argv, "h?dc:p:", long_options, &option_index)) != -1) {
#else
    while ((opt = getopt(argc, argv, "h?dc:p:")) != -1) {
#endif
        switch (opt) {
            case 'c':
                *filename = optarg;
                break;
            case 'd':
                *daemonize = true;
                break;
            case 'v':
                version();
                break;
            case '?':
            case 'h':
                help();
                exit(0);
            default:
                wrong_params();
                break;
        }
    }

    if (optind < argc) {
        wrong_params();
    }

}

#else
void help(void)
{
    printf("usage: gammu-smsd CONFIG_FILE\n");
}

void process_commandline(int argc, char **argv, bool *daemonize, char **filename)
{
    if (argc != 2) {
        help();
        exit(1);
    }

	/* FIXME: This is cruel hack */
	filename = argv[1];
}

#endif

int main(int argc, char **argv)
{
	GSM_Error error;
	char *filename = NULL;
    bool daemonize = false;

	config = SMSD_NewConfig();
	assert(config != NULL);

    process_commandline(argc, argv, &daemonize, &filename);

    if (filename == NULL) {
        printf("No config file specified!\n");
        help();
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
