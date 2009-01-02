/**
 * \file gammu-smsd.h
 * \author Michal Čihař
 *
 * SMSD interaction
 */
#ifndef __gammu_smsd_h
#define __gammu_smsd_h

#include <gammu-error.h>
#include <gammu-message.h>
#include <gammu-misc.h>

/**
 * \defgroup SMSD SMSD
 * SMS daemon manipulations
 */

/**
 * SMSD configuration data, these are not expected to be manipulated
 * directly by application.
 */
typedef struct _GSM_SMSDConfig GSM_SMSDConfig;

/**
 * Creates SMSD daemon based on parameters. It performs no parameter
 * checking and expects argc = 2 and argv = ["service", "configfile"],
 * otherwise it will crash. Also you can not start more than one
 * instances of this function.
 *
 * \deprecated{Do not use this function,it is only included for
 * backward compatibility of Gammu binary and will be removed in
 * future versions!}
 *
 * \param argc number of arguments
 * \param argv argument strings
 */
void SMSDaemon(int argc, char *argv[]);

/**
 * Enqueues SMS message in SMS daemon queue.
 *
 * \param service Service name override, can be (and should be) NULL.
 * \param filename SMSD configuration file.
 * \param sms Message data to send.
 *
 * \return Error code
 */
GSM_Error SMSDaemonSendSMS(char *service, char *filename, GSM_MultiSMSMessage *sms);

/**
 * Logs a message to SMSD log.
 *
 * \param format printf like format string.
 */
/* GSM_SMSDConfig *Config, */
PRINTF_STYLE(1, 2)
void WriteSMSDLog(const char *format, ...);


/**
 * Flags SMSD daemon to terminate itself gracefully.
 *
 * \param Config Pointer to SMSD configuration data.
 */
void SMSD_Shutdown(GSM_SMSDConfig *Config);

GSM_Error SMSD_ReadConfig(char *filename, GSM_SMSDConfig *Config, bool uselog, char *service);

void SMSD_Terminate(GSM_SMSDConfig *Config, const char *msg, GSM_Error error, bool exitprogram, int rc);

GSM_Error SMSD_MainLoop(GSM_SMSDConfig *Config);

GSM_SMSDConfig *SMSD_NewConfig(void);

void SMSD_FreeConfig(GSM_SMSDConfig *config);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
