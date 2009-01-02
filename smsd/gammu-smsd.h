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

void      SMSDaemon		(int argc, char *argv[]);
GSM_Error SMSDaemonSendSMS	(char *service, char *filename, GSM_MultiSMSMessage *sms);

#endif
