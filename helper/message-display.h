#ifndef __helper_message_display_h__
#define __helper_message_display_h__

#include <gammu-message.h>
#include <gammu-statemachine.h>
#include <gammu-backup.h>

/**
 * Prints location information about message.
 */
void PrintSMSLocation(const GSM_SMSMessage *sms, const GSM_SMSFolders *folders);

/**
 * Prints single phone number optionally showing name of contact from backup data.
 */
void PrintPhoneNumber(unsigned char *number, const GSM_Backup *Info);

void DisplayMultiSMSInfo (GSM_MultiSMSMessage *sms, gboolean eachsms, gboolean ems, const GSM_Backup *Info, GSM_StateMachine *sm);

void DisplaySMSFrame(GSM_SMSMessage *SMS, GSM_StateMachine *sm);
#endif
