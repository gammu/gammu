/**
 * Main SMSD program
 */
/* Copyright (c) 2009 Michal Cihar <michal@cihar.com> */
/* Licensend under GNU GPL 2 */

#include <gammu-smsd.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#ifndef WIN32
#include <unistd.h>
#endif
#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#endif
#ifdef WIN32
#include "service.h"
#endif
#ifdef HAVE_KILL
#include "pidfile.h"
#endif
#include "common.h"

#if !defined(WIN32) && (defined(HAVE_GETOPT) || defined(HAVE_GETOPT_LONG))
#define HAVE_DEFAULT_CONFIG
const char default_config[] = "/etc/gammu-smsdrc";
#endif

GSM_SMSDConfig *config;
volatile bool reconfigure = false;

void smsd_interrupt(int signum)
{
	SMSD_Shutdown(config);
	signal(signum, SIG_IGN);
}

void smsd_reconfigure(int signum)
{
	reconfigure = true;
	SMSD_Shutdown(config);
	signal(signum, SIG_IGN);
}

NORETURN void version(void)
{
	printf("Gammu-smsd version %s\n", VERSION);
	printf("Built %s on %s using %s\n", __TIME__, __DATE__, GetCompiler());
	printf("Compiled in features:\n");
#ifdef HAVE_DAEMON
	printf("  - %s\n", "DAEMON");
#endif
#ifdef HAVE_KILL
	printf("  - %s\n", "PID");
#endif
#ifdef WIN32
	printf("  - %s\n", "WINDOWS_SERVICE");
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

#ifdef HAVE_GETOPT_LONG
#define print_option(name, longname, help) \
	printf("-%s / --%s - %s\n", name, longname, help);
#define print_option_param(name, longname, paramname, help) \
	printf("-%s / --%s %s - %s\n", name, longname, paramname, help);
#else
#define print_option(name, longname, help) \
	printf("-%s - %s\n", name, help);
#define print_option_param(name, longname, paramname, help) \
	printf("-%s %s - %s\n", name, paramname, help);
#endif

void help(void)
{
	printf("usage: gammu-smsd [OPTION]...\n");
	printf("options:\n");
	print_option("h", "help", "shows this help");
	print_option("v", "version", "shows version information");
	print_option_param("c", "config", "CONFIG_FILE",
			   "defines path to config file");
#ifdef HAVE_KILL
	print_option("d", "daemon", "daemonizes program after startup");
#endif
#ifdef HAVE_KILL
	print_option_param("p", "pid", "PID_FILE", "defines path to pid file");
#endif
#ifdef WIN32
	print_option("i", "install-service",
		     "installs SMSD as a Windows service");
	print_option("u", "uninstall-service",
		     "uninstalls SMSD as a Windows service");
	print_option("s", "start-service", "starts SMSD Windows service");
#endif
}

NORETURN void wrong_params(void)
{
	fprintf(stderr, "Invalid parameter, use -h for help.\n");
	exit(1);
}

void process_commandline(int argc, char **argv, SMSD_Parameters * params)
{
	int opt;

#ifdef HAVE_GETOPT_LONG
	struct option long_options[] = {
		{"help", 0, 0, 'h'},
		{"version", 0, 0, 'v'},
		{"config", 1, 0, 'c'},
		{"daemon", 0, 0, 'd'},
		{"pid", 1, 0, 'p'},
		{"install-service", 0, 0, 'i'},
		{"uninstall-service", 0, 0, 'u'},
		{"start-service", 0, 0, 's'},
		{0, 0, 0, 0}
	};
	int option_index;

	while ((opt =
		getopt_long(argc, argv, "hv?dc:p:ius", long_options,
			    &option_index)) != -1) {
#elif defined(HAVE_GETOPT)
	while ((opt = getopt(argc, argv, "hv?dc:p:ius")) != -1) {
#else
	/* Poor mans getopt replacement */
	int i;

#define optarg argv[++i]

	for (i = 1; i < argc; i++) {
		if (strlen(argv[i]) != 2 || argv[i][0] != '-') {
			wrong_params();
		}
		opt = argv[i][1];
#endif
		switch (opt) {
			case 'c':
				params->config_file = optarg;
				break;
			case 'p':
				params->pid_file = optarg;
				break;
			case 'd':
				params->daemonize = true;
				break;
			case 'i':
				params->install_service = true;
				break;
			case 'u':
				params->uninstall_service = true;
				break;
			case 's':
				params->start_service = true;
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

#if defined(HAVE_GETOPT) || defined(HAVE_GETOPT_LONG)
	if (optind < argc) {
		wrong_params();
	}
#endif

}

#ifndef WIN32
#endif

int main(int argc, char **argv)
{
	GSM_Error error;

	SMSD_Parameters params = {
		NULL,
		NULL,
		false,
		false,
		false,
		false
	};

	process_commandline(argc, argv, &params);

#ifdef WIN32
	if (params.uninstall_service) {
		if (uninstall_smsd_service()) {
			printf("Service %s uninstalled sucessfully\n",
			       smsd_service_name);
			exit(0);
		} else {
			printf("Error uninstalling %s service\n",
			       smsd_service_name);
			exit(1);
		}
	}
#endif

	if (params.config_file == NULL) {
#ifdef HAVE_DEFAULT_CONFIG
		params.config_file = default_config;
#else
		fprintf(stderr, "No config file specified!\n");
		help();
		exit(1);
#endif
	}
#if WIN32
	if (params.install_service) {
		if (install_smsd_service(&params)) {
			printf("Service %s installed sucessfully\n",
			       smsd_service_name);
			exit(0);
		} else {
			printf("Error installing %s service\n",
			       smsd_service_name);
			exit(1);
		}
	}
#endif

read_config:
	config = SMSD_NewConfig();
	assert(config != NULL);

	error = SMSD_ReadConfig(params.config_file, config, true, NULL);
	if (error != ERR_NONE) {
		SMSD_Terminate(config, "Failed to read config", error, true, 2);
	}

	signal(SIGINT, smsd_interrupt);
	signal(SIGTERM, smsd_interrupt);
	signal(SIGHUP, smsd_reconfigure);

#ifdef HAVE_KILL
	if (params.pid_file != NULL && strlen(params.pid_file) > 0) {
		check_pid(params.pid_file);
		write_pid(params.pid_file);
	}
#endif

	if (params.daemonize) {
#ifdef HAVE_DAEMON
		daemon(1, 0);
#else
		fprintf(stderr,
			"daemon mode is not supported on your platform!\n");
		exit(1);
#endif
	}
#if WIN32
	if (params.start_service) {
		start_smsd_service_dispatcher();

		SMSD_FreeConfig(config);

		exit(0);
	}
#endif

	error = SMSD_MainLoop(config);
	if (error != ERR_NONE) {
		SMSD_Terminate(config, "Failed to run SMSD", error, true, 2);
	}

	SMSD_FreeConfig(config);

	if (reconfigure) {
		reconfigure = false;
		goto read_config;
	}

	return 0;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
