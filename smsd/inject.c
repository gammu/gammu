/**
 * SMSD message inject program
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

#include "../helper/message-cmdline.h"

#if !defined(WIN32) && (defined(HAVE_GETOPT) || defined(HAVE_GETOPT_LONG))
#define HAVE_DEFAULT_CONFIG
const char default_config[] = "/etc/gammu-smsdrc";
#endif

NORETURN void version(void)
{
	printf("Gammu-smsd-inject version %s\n", VERSION);
	printf("Built %s on %s using %s\n", __TIME__, __DATE__, GetCompiler());
	printf("Compiled in features:\n");
	printf("OS support:\n");
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
	printf("usage: gammu-smsd-inject [OPTION]... MSGTYPE RECIPIENT [MESSAGE_PARAMETER]...\n");
	printf("options:\n");
	print_option("h", "help", "shows this help");
	print_option("v", "version", "shows version information");
	print_option_param("c", "config", "CONFIG_FILE",
			   "defines path to config file");
	printf("\n");
	printf("MSGTYPE and it's parameters are described in man page and Gammu documentation\n");
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
		{0, 0, 0, 0}
	};
	int option_index;

	while ((opt =
		getopt_long(argc, argv, "+hv?c:", long_options,
			    &option_index)) != -1) {
#elif defined(HAVE_GETOPT)
	while ((opt = getopt(argc, argv, "+hv?c:")) != -1) {
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
	GSM_MultiSMSMessage sms;
	GSM_Message_Type type = SMS_SMSD;
	GSM_SMSDConfig *config;

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

	error = CreateMessage(&type, &sms, argc, startarg, argv, NULL);
	if (error != ERR_NONE) {
		printf("Failed to create message: %s\n", GSM_ErrorString(error));
		return 1;
	}

	config = SMSD_NewConfig();
	assert(config != NULL);

	error = SMSD_ReadConfig(params.config_file, config, true);
	if (error != ERR_NONE) {
		SMSD_Terminate(config, "Failed to read config", error, true, 2);
	}

	error = SMSD_InjectSMS(config, &sms);
	if (error != ERR_NONE) {
		printf("Failed to inject message: %s\n", GSM_ErrorString(error));
		return 2;
	}

	SMSD_FreeConfig(config);

	return 0;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
