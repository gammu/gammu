#ifndef __smsd_common_h__
#define __smsd_common_h__

#include <gammu-smsd.h>

/**
 * Stucture holding Gammu SMSD command line parameters.
 */
typedef struct {
	const char *config_file;
	const char *pid_file;
	int uid;
	int gid;
	char *user;
	char *group;
	bool daemonize;
	bool start_service;
	bool stop_service;
	bool run_service;
	bool install_service;
	bool uninstall_service;
} SMSD_Parameters;
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
