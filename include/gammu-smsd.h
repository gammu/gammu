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

#define SMSD_TEXT_LENGTH 100

/**
 * Status structure, which can be found in shared memory (if supported
 * on platform).
 */
typedef struct {
	/**
	 * Version of this structure (1 for now).
	 */
	int Version;
	/**
	 * PhoneID from configuration.
	 */
	char PhoneID[SMSD_TEXT_LENGTH + 1];
	/**
	 * Client software name.
	 */
	char Client[SMSD_TEXT_LENGTH + 1];
	/**
	 * Current phone battery state.
	 */
	GSM_BatteryCharge  Charge;
	/**
	 * Current network state.
	 */
	GSM_SignalQuality  Network;
	/**
	 * Number of received messages.
	 */
	int Received;
	/**
	 * Number of sent messages.
	 */
	int Sent;
	/**
	 * Number of messages which failed to be send.
	 */
	int Failed;
	/**
	 * Phone IMEI.
	 */
	char IMEI[GSM_MAX_IMEI_LENGTH + 1];
} GSM_SMSDStatus;


/**
 * Enqueues SMS message in SMS daemon queue.
 *
 * \param Config SMSD configuration pointer.
 * \param sms Message data to send.
 *
 * \return Error code
 */
GSM_Error SMSD_InjectSMS(GSM_SMSDConfig *Config, GSM_MultiSMSMessage *sms);

/**
 * Gets SMSD status via shared memory.
 *
 * \param Config SMSD configuration pointer.
 * \param status pointer where status will be copied
 *
 * \return Error code
 */
GSM_Error SMSD_GetStatus(GSM_SMSDConfig *Config, GSM_SMSDStatus *status);

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
