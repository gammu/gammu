/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file for the various functions, definitions etc. used to implement
  the handset interface.  See fbus-at.c for more details.

*/

#ifndef __n_at_h
#define __n_at_h

#include "gsm-api.h"

/* Global variables */
extern bool            Nat_LinkOK;
extern GSM_Functions   Nat_Functions;
extern GSM_Information Nat_Information;

/* Prototypes for the functions designed to be used externally. */
GSM_Error Nat_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame));

void      Nat_DispatchMessage(u16 MessageLength, u8 *MessageBuffer,
                              u8 MessageType);

GSM_Error Nat_DialVoice(char *Number);
GSM_Error Nat_CancelCall(void);

GSM_Error Nat_SetDateTime(GSM_DateTime *date_time);
GSM_Error Nat_GetDateTime(GSM_DateTime *date_time);

GSM_Error Nat_GetAlarm(int alarm_number, GSM_DateTime *date_time);

GSM_Error Nat_SendDTMF(char *String);

GSM_Error Nat_GetSMSMessage(GSM_SMSMessage *message);
GSM_Error Nat_SaveSMSMessage(GSM_SMSMessage *message);
GSM_Error Nat_DeleteSMSMessage(GSM_SMSMessage *message);
GSM_Error Nat_GetSMSStatus(GSM_SMSStatus *Status);
GSM_Error Nat_SendSMSMessage(GSM_SMSMessage *message);

GSM_Error Nat_EnterSecurityCode(GSM_SecurityCode SecurityCode);
GSM_Error Nat_GetSecurityCodeStatus(int *Status);
GSM_Error Nat_GetSecurityCodeStatus(int *Status);

GSM_Error Nat_GetMemoryLocation(GSM_PhonebookEntry *entry);
GSM_Error Nat_WritePhonebookLocation(GSM_PhonebookEntry * entry);
GSM_Error Nat_GetMemoryStatus(GSM_MemoryStatus *Status);

GSM_Error Nat_GetBatteryLevel(GSM_BatteryUnits *units, float *level);
GSM_Error Nat_GetRFLevel(GSM_RFUnits *units, float *level);
GSM_Error Nat_GetPowerSource(GSM_PowerSource * source);

GSM_Error Nat_GetSMSCenter(GSM_MessageCenter *MessageCenter);
GSM_Error Nat_SetSMSCenter(GSM_MessageCenter *MessageCenter);

GSM_Error Nat_GetNetworkInfo(GSM_NetworkInfo *NetworkInfo);

GSM_Error Nat_GetManufacturer(char* manufacturer);

GSM_Error Nat_CallDivert(GSM_CallDivert *cd);

/* FIXME: not functional or returns errors with tested AT implementation */
GSM_Error Nat_GetDisplayStatus(int *Status);
GSM_Error Nat_PressKey(int key, int event);
GSM_Error Nat_SetAlarm(int alarm_number, GSM_DateTime *date_time);

struct AT_OneReply {
  char *ReplyString;
  void (*ReplyFunction) (u16 message_length, u8 *buffer, u8 message_type);
};

#endif /* __n_at_h */
