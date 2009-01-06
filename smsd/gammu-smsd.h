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
 * Enqueues SMS message in SMS daemon queue.
 *
 * \param filename SMSD configuration file.
 * \param sms Message data to send.
 *
 * \return Error code
 */
GSM_Error SMSD_InjectSMS(const char *filename, GSM_MultiSMSMessage *sms);

/**
 * Flags SMSD daemon to terminate itself gracefully.
 *
 * \param Config Pointer to SMSD configuration data.
 */
void SMSD_Shutdown(GSM_SMSDConfig *Config);

GSM_Error SMSD_ReadConfig(const char *filename, GSM_SMSDConfig *Config, bool uselog);

void SMSD_Terminate(GSM_SMSDConfig *Config, const char *msg, GSM_Error error, bool exitprogram, int rc);

GSM_Error SMSD_MainLoop(GSM_SMSDConfig *Config);

GSM_SMSDConfig *SMSD_NewConfig(void);

void SMSD_FreeConfig(GSM_SMSDConfig *config);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
