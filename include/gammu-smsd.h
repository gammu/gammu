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
 *
 * \ingroup SMSD
 */
typedef struct _GSM_SMSDConfig GSM_SMSDConfig;

/**
 * Length of texts in GSM_SMSDStatus structure.
 */
#define SMSD_TEXT_LENGTH 255

/**
 * Status structure, which can be found in shared memory (if supported
 * on platform).
 *
 * \ingroup SMSD
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
	GSM_BatteryCharge Charge;
	/**
	 * Current network state.
	 */
	GSM_SignalQuality Network;
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
 * \param NewID Pointer to string where ID of new message will be
 * written. Can be NULL and then it is ignored.
 *
 * \return Error code
 *
 * \ingroup SMSD
 */
GSM_Error SMSD_InjectSMS(GSM_SMSDConfig * Config, GSM_MultiSMSMessage * sms, char *NewID);

/**
 * Gets SMSD status via shared memory.
 *
 * \param Config SMSD configuration pointer.
 * \param status pointer where status will be copied
 *
 * \return Error code
 *
 * \ingroup SMSD
 */
GSM_Error SMSD_GetStatus(GSM_SMSDConfig * Config, GSM_SMSDStatus * status);

/**
 * Flags SMSD daemon to terminate itself gracefully.
 *
 * \param Config Pointer to SMSD configuration data.
 *
 * \return Error code
 *
 * \ingroup SMSD
 */
GSM_Error SMSD_Shutdown(GSM_SMSDConfig * Config);

/**
 * Reads SMSD configuration.
 *
 * \param filename File name of configuration.
 * \param Config Pointer to SMSD configuration data.
 * \param uselog Whether to log errors to configured log.
 *
 * \return Error code
 *
 * \ingroup SMSD
 */
GSM_Error SMSD_ReadConfig(const char *filename, GSM_SMSDConfig * Config,
			  gboolean uselog);

/**
 * Main SMS daemon loop. It connects to phone, scans for messages and
 * sends messages from inbox. Can be interrupted by SMSD_Shutdown.
 *
 * \see SMSD_Shutdown
 *
 * \param Config Pointer to SMSD configuration data.
 * \param exit_on_failure Whether failure should lead to terminaton of
 * program.
 * \param max_failures Maximal number of failures after which SMSD will
 * terminate. Use 0 to not terminate on failures.
 *
 * \return Error code
 *
 * \ingroup SMSD
 */
GSM_Error SMSD_MainLoop(GSM_SMSDConfig * Config, gboolean exit_on_failure, int max_failures);

/**
 * Creates new SMSD configuration.
 *
 * \param name Name of process, will be used for logging. If NULL,
 * gammu-smsd text is used.
 *
 * \return Pointer to SMSD configuration data block.
 *
 * \ingroup SMSD
 */
GSM_SMSDConfig *SMSD_NewConfig(const char *name);

/**
 * Frees SMSD configuration.
 *
 * \param config Pointer to SMSD configuration data.
 *
 * \ingroup SMSD
 */
void SMSD_FreeConfig(GSM_SMSDConfig * config);
#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
