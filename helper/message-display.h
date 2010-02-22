#ifndef __helper_message_display_h__
#define __helper_message_display_h__

#include <gammu-message.h>
#include <gammu-statemachine.h>
#include <gammu-backup.h>

/**
 * Prints location information about message.
 */
void PrintSMSLocation(const GSM_SMSMessage *sms, const GSM_SMSFolders *folders);

#ifdef GSM_ENABLE_BACKUP
/**
 * Prints single phone number optionally showing name of contact from backup data.
 */
void PrintPhoneNumber(const unsigned char *number, const GSM_Backup *Info);
#else
/**
 * Prints single phone number optionally showing name of contact from backup data.
 */
void PrintPhoneNumber(const unsigned char *number, const void *Info);
#endif

#ifdef GSM_ENABLE_BACKUP
void DisplayMultiSMSInfo (GSM_MultiSMSMessage *sms, gboolean eachsms, gboolean ems, const GSM_Backup *Info, GSM_StateMachine *sm);
#else
void DisplayMultiSMSInfo (GSM_MultiSMSMessage *sms, gboolean eachsms, gboolean ems, const void *Info, GSM_StateMachine *sm);
#endif

void DisplaySMSFrame(GSM_SMSMessage *SMS, GSM_StateMachine *sm);
#endif
