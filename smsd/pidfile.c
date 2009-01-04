/**
 * SMSD PID file handling
 */
/* Copyright (c) 2009 Michal Cihar <michal@cihar.com> */
/* Licensend under GNU GPL 2 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

#include "pidfile.h"

/**
 * Check existing PID file if it points to existing application.
 */
void check_pid(const char *pid_file)
{
	FILE *file;

	int other;

	file = fopen(pid_file, "r");
	if (file != NULL) {
		if (fscanf(file, "%d", &other) == 1) {
			if (kill(other, 0) == 0) {
				fprintf(stderr,
					"Another instance is running, please stop it first!\n");
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
 * Write a pid file.
 */
void write_pid(const char *pid_file)
{
	FILE *file;

	file = fopen(pid_file, "w");
	if (file != NULL) {
		fprintf(file, "%d\n", getpid());
		fclose(file);
	} else {
		fprintf(stderr, "Can not create pidfile!\n");
		exit(1);
	}
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
