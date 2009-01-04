/**
 * Main SMSD program
 */
/* Copyright (c) 2009 Michal Cihar <michal@cihar.com> */
/* Licensend under GNU GPL 2 */

#include <signal.h>
#include <gammu-smsd.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#endif

#if !defined(WIN32) && (defined(HAVE_GETOPT) || defined(HAVE_GETOPT_LONG))
#define HAVE_DEFAULT_CONFIG
const char default_config[] = "/etc/gammu-smsdrc";
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
    fprintf(stderr, "Invalid parameter, use --help for help.\n");
    exit(1);
}

void process_commandline(int argc, char **argv, bool *daemonize, const char **pid_file, const char **config_file)
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
                *config_file = optarg;
                break;
            case 'p':
                *pid_file = optarg;
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

void process_commandline(int argc, char **argv, bool *daemonize, char **pid_file, char **config_file)
{
    if (argc != 2) {
        help();
        exit(1);
    }

    /* FIXME: This is cruel hack */
    config_file = argv[1];
}

#endif

#ifndef WIN32
/**
 * Check existing PID file if it points to existing application.
 */
void check_pid(const char *pid_file)
{
    FILE        *file;
    int         other;

    file = fopen(pid_file, "r");
    if (file != NULL) {
        if (fscanf(file, "%d", &other) == 1) {
            if (kill(other, 0) == 0) {
                fprintf(stderr, "Another instance is running, please stop it first!\n");
                exit(10);
            } else {
                fprintf(stderr, "Stale lock file, ignoring!\n");
            }
        } else {
            fprintf(stderr, "Can not parse pidfile, ignoring!\n");
        }
        fclose(file);
    }
}


/**
 * Write pid file.
 */
void write_pid(const char *pid_file) {
    FILE        *file;

    file = fopen(pid_file, "w");
    if (file != NULL) {
        fprintf(file, "%d\n", getpid());
        fclose(file);
    } else {
        fprintf(stderr, "Can not create pidfile!\n");
        exit(1);
    }
}
#endif

int main(int argc, char **argv)
{
	GSM_Error error;
	const char *config_file = NULL;
	const char *pid_file = NULL;
    bool daemonize = false;

	config = SMSD_NewConfig();
	assert(config != NULL);

    process_commandline(argc, argv, &daemonize, &pid_file, &config_file);

    if (config_file == NULL) {
#ifdef HAVE_DEFAULT_CONFIG
        config_file = default_config;
#else
        fprintf(stderr, "No config file specified!\n");
        help();
        exit(1);
#endif
    }

	error = SMSD_ReadConfig(config_file, config, true, NULL);
	if (error != ERR_NONE) {
		SMSD_Terminate(config, "Failed to read config", error, true, 2);
	}

	signal(SIGINT, smsd_interrupt);
	signal(SIGTERM, smsd_interrupt);

#ifndef WIN32
    if (pid_file != NULL && strlen(pid_file) > 0) {
        check_pid(pid_file);
        write_pid(pid_file);
    }
#endif

    if (daemonize) {
#ifdef HAVE_DAEMON
        daemon(1, 0);
#else
        fprintf(stderr, "daemon mode is not supported on your platform!\n");
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
