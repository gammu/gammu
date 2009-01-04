#ifndef __smsd_common_h__
#define __smsd_common_h__

/**
 * Stucture holding Gammu SMSD command line parameters.
 */
typedef struct {
    const char *config_file;
    const char *pid_file;
    bool daemonize;
    bool start_service;
    bool install_service;
    bool uninstall_service;
} SMSD_Parameters;

/**
 * SMSD configuration used globally all around SMSD.
 */
extern GSM_SMSDConfig *config;
#endif
