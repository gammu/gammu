/**
 * Windows service support.
 */
#ifndef __smsd_service_h__
#define __smsd_service_h__
#include "common.h"

/**
 * Name of service.
 */
extern char *smsd_service_name;

/**
 * Starts service dispatcher.
 */
extern void start_smsd_service_dispatcher(void);

/**
 * Installs service.
 */
extern bool uninstall_smsd_service(void);

/**
 * Uninstalls service.
 */
extern bool install_smsd_service(SMSD_Parameters * params);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
