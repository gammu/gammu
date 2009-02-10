/* (c) 2002-2006 by Marcin Wiacek and Michal Cihar */

#ifndef atgen_functions_h
#define atgen_functions_h

#include "../../gsmstate.h"

extern GSM_Reply_Function ATGENReplyFunctions[];

extern GSM_Error ATGEN_Initialise		(GSM_StateMachine *s);
extern GSM_Error ATGEN_Terminate		(GSM_StateMachine *s);
extern GSM_Error ATGEN_GetIMEI 			(GSM_StateMachine *s);
extern GSM_Error ATGEN_GetFirmware		(GSM_StateMachine *s);
extern GSM_Error ATGEN_GetModel			(GSM_StateMachine *s);
extern GSM_Error ATGEN_GetDateTime		(GSM_StateMachine *s, GSM_DateTime *date_time);
extern GSM_Error ATGEN_GetMemory		(GSM_StateMachine *s, GSM_MemoryEntry *entry);
extern GSM_Error ATGEN_GetNextMemory		(GSM_StateMachine *s, GSM_MemoryEntry *entry, bool start);
extern GSM_Error ATGEN_SetMemory		(GSM_StateMachine *s, GSM_MemoryEntry *entry);
extern GSM_Error ATGEN_AddMemory		(GSM_StateMachine *s, GSM_MemoryEntry *entry);
extern GSM_Error ATGEN_DeleteMemory		(GSM_StateMachine *s, GSM_MemoryEntry *entry);
extern GSM_Error ATGEN_GetMemoryStatus		(GSM_StateMachine *s, GSM_MemoryStatus *Status);
extern GSM_Error ATGEN_GetSMSC			(GSM_StateMachine *s, GSM_SMSC *smsc);
extern GSM_Error ATGEN_SetSMSC			(GSM_StateMachine *s, GSM_SMSC *smsc);
extern GSM_Error ATGEN_GetSMSFolders		(GSM_StateMachine *s, GSM_SMSFolders *folders);
extern GSM_Error ATGEN_GetSMSStatus		(GSM_StateMachine *s, GSM_SMSMemoryStatus *status);
extern GSM_Error ATGEN_GetSMS			(GSM_StateMachine *s, GSM_MultiSMSMessage *sms);
extern GSM_Error ATGEN_GetNextSMS		(GSM_StateMachine *s, GSM_MultiSMSMessage *sms, bool start);
extern GSM_Error ATGEN_SendSavedSMS		(GSM_StateMachine *s, int Folder, int Location);
extern GSM_Error ATGEN_SendSMS			(GSM_StateMachine *s, GSM_SMSMessage *sms);
extern GSM_Error ATGEN_DeleteSMS		(GSM_StateMachine *s, GSM_SMSMessage *sms);
extern GSM_Error ATGEN_AddSMS			(GSM_StateMachine *s, GSM_SMSMessage *sms);
extern GSM_Error ATGEN_GetBatteryCharge		(GSM_StateMachine *s, GSM_BatteryCharge *bat);
extern GSM_Error ATGEN_GetSignalQuality		(GSM_StateMachine *s, GSM_SignalQuality *sig);
extern GSM_Error ATGEN_DialVoice		(GSM_StateMachine *s, char *number, GSM_CallShowNumber ShowNumber);
extern GSM_Error ATGEN_DialService(GSM_StateMachine *s, char *number);
extern GSM_Error ATGEN_AnswerCall		(GSM_StateMachine *s, int ID, bool all);
extern GSM_Error ATGEN_CancelCall		(GSM_StateMachine *s, int ID, bool all);
extern GSM_Error ATGEN_SetDateTime		(GSM_StateMachine *s, GSM_DateTime *date_time);
extern GSM_Error ATGEN_EnterSecurityCode	(GSM_StateMachine *s, GSM_SecurityCode Code);
extern GSM_Error ATGEN_GetSecurityStatus	(GSM_StateMachine *s, GSM_SecurityCodeType *Status);
extern GSM_Error ATGEN_ResetPhoneSettings	(GSM_StateMachine *s, GSM_ResetSettingsType Type);
extern GSM_Error ATGEN_SendDTMF			(GSM_StateMachine *s, char *sequence);
extern GSM_Error ATGEN_GetSIMIMSI		(GSM_StateMachine *s, char *IMSI);
extern GSM_Error ATGEN_GetNetworkInfo		(GSM_StateMachine *s, GSM_NetworkInfo *netinfo);
extern GSM_Error ATGEN_Reset			(GSM_StateMachine *s, bool hard);
extern GSM_Error ATGEN_PressKey			(GSM_StateMachine *s, GSM_KeyCode Key, bool Press);
extern GSM_Error ATGEN_GetDisplayStatus		(GSM_StateMachine *s, GSM_DisplayFeatures *features);
extern GSM_Error ATGEN_SetAutoNetworkLogin	(GSM_StateMachine *s);
extern GSM_Error ATGEN_DeleteAllMemory		(GSM_StateMachine *s, GSM_MemoryType type);

extern GSM_Error ATGEN_SetFastSMSSending	(GSM_StateMachine *s, bool enable);
extern GSM_Error ATGEN_SetIncomingCall		(GSM_StateMachine *s, bool enable);
extern GSM_Error ATGEN_SetIncomingCB		(GSM_StateMachine *s, bool enable);
extern GSM_Error ATGEN_SetIncomingSMS		(GSM_StateMachine *s, bool enable);

extern GSM_Error ATGEN_GetManufacturer(GSM_StateMachine *s);
extern GSM_Error ATGEN_GetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm);
extern GSM_Error ATGEN_SetAlarm(GSM_StateMachine *s, GSM_Alarm *Alarm);
extern GSM_Error ATGEN_SetIncomingUSSD(GSM_StateMachine *s, bool enable);
extern GSM_Error ATGEN_GetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, bool PhoneRingtone);
extern GSM_Error ATGEN_SetRingtone(GSM_StateMachine *s, GSM_Ringtone *Ringtone, int *maxlength);
extern GSM_Error ATGEN_GetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap);
extern GSM_Error ATGEN_SetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
