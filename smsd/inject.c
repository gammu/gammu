/**
 * SMSD message inject program
 */
/* Copyright (c) 2009 - 2018 Michal Cihar <michal@cihar.com> */
/* Licensend under GNU GPL 2 */

#include <gammu-smsd.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
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
	printf("Gammu-smsd-inject version %s\n", GAMMU_VERSION);
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
	printf("  - %s\n", "NULL");
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
#ifdef ODBC_FOUND
	printf("  - %s\n", "ODBC");
#endif
	printf("\n");
	printf("Copyright (C) 2003 - 2018 Michal Cihar <michal@cihar.com> and other authors.\n");
	printf("\n");
	printf("License GPLv2: GNU GPL version 2 <https://spdx.org/licenses/GPL-2.0>.\n");
	printf("This is free software: you are free to change and redistribute it.\n");
	printf("There is NO WARRANTY, to the extent permitted by law.\n");
	printf("\n");
	printf("Check <https://wammu.eu/gammu/> for updates.\n");
	printf("\n");
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
	print_option("l", "use-log", "use logging configuration from config file");
	print_option("L", "no-use-log", "do not use logging configuration from config file (default)");
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
		{"use-log", 0, 0, 'l'},
		{"no-use-log", 0, 0, 'L'},
		{0, 0, 0, 0}
	};
	int option_index;

	while ((opt =
		getopt_long(argc, argv, "+hvc:lL", long_options,
			    &option_index)) != -1) {
#elif defined(HAVE_GETOPT)
	while ((opt = getopt(argc, argv, "+hvc:lL")) != -1) {
#else
	/* Poor mans getopt replacement */
	int i;

#define optarg argv[++i]

	for (i = 1; i < argc; i++) {
		if (strlen(argv[i]) != 2 || argv[i][0] != '-') {
			return i;
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
			case 'l':
				params->use_log = TRUE;
				break;
			case 'L':
				params->use_log = FALSE;
				break;
			case '?':
				wrong_params();
			case 'h':
				help();
				exit(0);
			default:
				fprintf(stderr, "Parameter -%c not known!\n", opt);
				wrong_params();
				break;
		}
	}

#if defined(HAVE_GETOPT_LONG) || defined(HAVE_GETOPT)
	return optind;
#else
	return i;
#endif

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
	const char program_name[] = "gammu-smsd-inject";
	char newid[200] = { 0 };

	SMSD_Parameters params = {
		NULL,
		NULL,
		-1,
		-1,
		NULL,
		NULL,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		0
	};

	/*
	 * We don't need gettext, but need to set locales so that
	 * charset conversion works.
	 */
	GSM_InitLocales(NULL);

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
		printf("Failed to create message: %s\n",
		       GSM_ErrorString(error));
		return 1;
	}

	config = SMSD_NewConfig(program_name);
	assert(config != NULL);

	error = SMSD_ReadConfig(params.config_file, config, params.use_log);
	if (error != ERR_NONE) {
		printf("Failed to read config: %s\n", GSM_ErrorString(error));
		SMSD_FreeConfig(config);
		return 2;
	}
	SMSD_EnableGlobalDebug(config);

	error = SMSD_InjectSMS(config, &sms, newid);
	if (error != ERR_NONE) {
		printf("Failed to inject message: %s\n",
		       GSM_ErrorString(error));
		SMSD_FreeConfig(config);
		return 3;
	}
	if (strlen(newid) == 0) {
		printf("Written message without ID\n");
	} else {
		printf("Written message with ID %s\n", newid);
	}

	SMSD_FreeConfig(config);

	return 0;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
