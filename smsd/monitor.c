/**
 * SMSD message monitor program
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

#include "common.h"

#if !defined(WIN32) && (defined(HAVE_GETOPT) || defined(HAVE_GETOPT_LONG))
#define HAVE_DEFAULT_CONFIG
const char default_config[] = "/etc/gammu-smsdrc";
#endif

volatile bool terminate = false;
int delay_seconds = 20;
int limit_loops = -1;
bool compact = false;

void smsd_interrupt(int signum)
{
	terminate = true;
}

NORETURN void version(void)
{
	printf("Gammu-smsd-monitor version %s\n", VERSION);
	printf("Built %s on %s using %s\n", __TIME__, __DATE__, GetCompiler());
	printf("Compiled in features:\n");
	printf("OS support:\n");
#ifdef HAVE_SHM
	printf("  - %s\n", "SHM");
#endif
#ifdef HAVE_GETOPT
	printf("  - %s\n", "GETOPT");
#endif
#ifdef HAVE_GETOPT_LONG
	printf("  - %s\n", "GETOPT_LONG");
#endif
	printf("Backend services:\n");
	printf("  - %s\n", "FILES");
#ifdef HAVE_MYSQL_MYSQL_H
	printf("  - %s\n", "MYSQL");
#endif
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	printf("  - %s\n", "POSTGRESQL");
#endif
#ifdef LIBDBI_FOUND
	printf("  - %s\n", "DBI");
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
	printf("usage: gammu-smsd-monitor [OPTION]...\n");
	printf("options:\n");
	print_option("h", "help", "shows this help");
	print_option("v", "version", "shows version information");
	print_option("C", "csv", "CSV output");
	print_option_param("c", "config", "CONFIG_FILE",
			   "defines path to config file");
	print_option_param("d", "delay", "DELAY",
			   "delay in seconds between loops");
	print_option_param("l", "loops", "NUMBER",
			   "delay in seconds between loops");
}

NORETURN void wrong_params(void)
{
	fprintf(stderr, "Invalid parameter, use -h for help.\n");
	exit(1);
}

int process_commandline(int argc, char **argv, SMSD_Parameters * params)
{
	int opt;

#ifdef HAVE_GETOPT_LONG
	struct option long_options[] = {
		{"help", 0, 0, 'h'},
		{"version", 0, 0, 'v'},
		{"config", 1, 0, 'c'},
		{"delay", 1, 0, 'd'},
		{"loops", 1, 0, 'l'},
		{0, 0, 0, 0}
	};
	int option_index;

	while ((opt =
		getopt_long(argc, argv, "+hv?c:d:l:C", long_options,
			    &option_index)) != -1) {
#elif defined(HAVE_GETOPT)
	while ((opt = getopt(argc, argv, "+hv?c:d:l:C")) != -1) {
#else
	/* Poor mans getopt replacement */
	int i, optind = -1;

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
			case 'v':
				version();
				break;
			case 'C':
				compact = true;
				break;
			case 'd':
				delay_seconds = atoi(optarg);
				break;
			case 'l':
				limit_loops = atoi(optarg);
				break;
			case '?':
			case 'h':
				help();
				exit(0);
			default:
#if defined(HAVE_GETOPT) || defined(HAVE_GETOPT_LONG)
				wrong_params();
#else
				optind = 1;
#endif
				break;
		}
#if !defined(HAVE_GETOPT) && !defined(HAVE_GETOPT_LONG)
		if (optind != -1) break;
#endif
	}

	return optind;

}

#ifndef WIN32
#endif

int main(int argc, char **argv)
{
	GSM_Error error;
	int startarg;
	GSM_SMSDConfig *config;
	GSM_SMSDStatus status;
	const char program_name[] = "gammu-smsd-monitor";
	SMSD_Parameters params = {
		NULL,
		NULL,
		-1,
		-1,
		NULL,
		NULL,
		false,
		false,
		false,
		false,
		false,
		false
	};


	startarg = process_commandline(argc, argv, &params);

	if (params.config_file == NULL) {
#ifdef HAVE_DEFAULT_CONFIG
		params.config_file = default_config;
#else
		fprintf(stderr, "No config file specified!\n");
		help();
		exit(1);
#endif
	}

	signal(SIGINT, smsd_interrupt);
	signal(SIGTERM, smsd_interrupt);

	config = SMSD_NewConfig(program_name);
	assert(config != NULL);

	error = SMSD_ReadConfig(params.config_file, config, true);
	if (error != ERR_NONE) {
		SMSD_Terminate(config, "Failed to read config", error, true, 2);
	}

	while (!terminate && (limit_loops == -1 || limit_loops-- > 0)) {
		error = SMSD_GetStatus(config, &status);
		if (error != ERR_NONE) {
			printf("Failed to get status: %s\n", GSM_ErrorString(error));
			return 2;
		}
		if (compact) {
			printf("%s;%s;%s;%d;%d;%d;%d;%d\n",
				 status.Client,
				 status.PhoneID,
				 status.IMEI,
				 status.Sent,
				 status.Received,
				 status.Failed,
				 status.Charge.BatteryPercent,
				 status.Network.SignalPercent);
		} else {
			printf("Client: %s\n", status.Client);
			printf("PhoneID: %s\n", status.PhoneID);
			printf("IMEI: %s\n", status.IMEI);
			printf("Sent: %d\n", status.Sent);
			printf("Received: %d\n", status.Received);
			printf("Failed: %d\n", status.Failed);
			printf("BatterPercent: %d\n", status.Charge.BatteryPercent);
			printf("NetworkSignal: %d\n", status.Network.SignalPercent);
			printf("\n");
		}
		sleep(delay_seconds);
	}

	SMSD_FreeConfig(config);

	return 0;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
