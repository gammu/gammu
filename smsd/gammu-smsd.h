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

typedef struct _GSM_SMSDConfig GSM_SMSDConfig;

void      SMSDaemon		(int argc, char *argv[]);
GSM_Error SMSDaemonSendSMS	(char *service, char *filename, GSM_MultiSMSMessage *sms);

/* GSM_SMSDConfig *Config, */
PRINTF_STYLE(1, 2)
void WriteSMSDLog(const char *format, ...);


#endif
