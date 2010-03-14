/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file for the various functions, definitions etc. used to implement
  the handset interface.  See fbus-6110.c for more details.

*/

#ifndef __n_6110_h
#define __n_6110_h

#include "gsm-api.h"

/* Nokia 6110 supports phonebook entries of max. 16 characters and numbers of
   max. 30 digits */
#define N6110_MAX_PHONEBOOK_NAME_LENGTH   (16)
#define N6110_MAX_PHONEBOOK_NUMBER_LENGTH (30)

/* Alternate defines for memory types.  Two letter codes follow GSM 07.07
   release 6.2.0, AT+CPBS "Select Phonebook Memory Storage" */
#define N6110_MEMORY_MT    0x01    /* combined ME and SIM phonebook */
#define N6110_MEMORY_ME    0x02    /* ME (Mobile Equipment) phonebook */
#define N6110_MEMORY_SM    0x03    /* SIM phonebook */
#define N6110_MEMORY_FD    0x04    /* SIM fixdialling-phonebook */
#define N6110_MEMORY_ON    0x05    /* SIM (or ME) own numbers list */
#define N6110_MEMORY_EN    0x06    /* SIM (or ME) emergency number */
#define N6110_MEMORY_DC    0x07    /* ME dialled calls list */
#define N6110_MEMORY_RC    0x08    /* ME received calls list */
#define N6110_MEMORY_MC    0x09    /* ME missed (unanswered received) calls list */
#define N6110_MEMORY_VOICE 0x0b    /* Voice Mailbox */
#define N6110_MEMORY_XX    0xff    /* This is used when the memory type is unknown. */

/* This byte is used to synchronize FBUS channel over cable or IR. */
#define N6110_SYNC_BYTE 0x55

/* This byte is send after all N6110_SYNC_BYTE bytes when init infrared. */
#define N6110_IR_END_SYNC_BYTE  0xc1

/* Every (well, almost every) frame from the computer starts with this
   sequence. */
#define N6110_FRAME_HEADER 0x00, 0x01, 0x00

/* Global variables */
extern bool            N6110_LinkOK;
extern GSM_Functions   N6110_Functions;
extern GSM_Information N6110_Information;

/* Prototypes for the functions designed to be used externally. */
GSM_Error N6110_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame));

void      N6110_DispatchMessage(u16 MessageLength, u8 *MessageBuffer,
                              u8 MessageType);

GSM_Error N6110_SendStatusRequest(void);

int       N6110_GetMemoryType(GSM_MemoryType memory_type);

GSM_Error N6110_GetVoiceMailbox ( GSM_PhonebookEntry *entry);  

GSM_Error N6110_GetOperatorName ( GSM_Network *operator);  
GSM_Error N6110_SetOperatorName ( GSM_Network *operator);    

GSM_Error N6110_GetMemoryLocation(GSM_PhonebookEntry *entry);
GSM_Error N6110_WritePhonebookLocation(GSM_PhonebookEntry *entry);

GSM_Error N6110_GetSpeedDial(GSM_SpeedDial *entry);
GSM_Error N6110_SetSpeedDial(GSM_SpeedDial *entry);

GSM_Error N6110_GetMemoryStatus(GSM_MemoryStatus *Status);
GSM_Error N6110_GetSMSStatus(GSM_SMSStatus *Status);
GSM_Error N6110_GetSMSCenter(GSM_MessageCenter *MessageCenter);
  
GSM_Error N6110_GetSMSMessage(GSM_SMSMessage *Message);
GSM_Error N6110_DeleteSMSMessage(GSM_SMSMessage *Message);
GSM_Error N6110_SendSMSMessage(GSM_SMSMessage *Message);
GSM_Error N6110_SaveSMSMessage(GSM_SMSMessage *Message);

GSM_Error N6110_GetRFLevel(GSM_RFUnits *units, float *level);
GSM_Error N6110_GetBatteryLevel(GSM_BatteryUnits *units, float *level);
GSM_Error N6110_GetPowerSource(GSM_PowerSource *source);
GSM_Error N6110_GetDisplayStatus(int *Status);

GSM_Error N6110_EnterSecurityCode(GSM_SecurityCode SecurityCode);
GSM_Error N6110_GetSecurityCodeStatus(int *Status);
GSM_Error N6110_GetSecurityCode(GSM_SecurityCode *SecurityCode);

GSM_Error N6110_GetSMSCenter(GSM_MessageCenter *MessageCenter);
GSM_Error N6110_SetSMSCenter(GSM_MessageCenter *MessageCenter);

GSM_Error N6110_GetIMEI(char *imei);
GSM_Error N6110_GetRevision(char *revision);
GSM_Error N6110_GetModel(char *model);
GSM_Error N6110_GetManufacturer(char *manufacturer);

GSM_Error N6110_GetDateTime(GSM_DateTime *date_time);
GSM_Error N6110_SetDateTime(GSM_DateTime *date_time);

GSM_Error N6110_PrivGetDateTime(GSM_DateTime *date_time, int msgtype);
GSM_Error N6110_PrivSetDateTime(GSM_DateTime *date_time, int msgtype);

GSM_Error N6110_GetAlarm(int alarm_number, GSM_DateTime *date_time);
GSM_Error N6110_SetAlarm(int alarm_number, GSM_DateTime *date_time);

GSM_Error N6110_PrivGetAlarm(int alarm_number, GSM_DateTime *date_time, int msgtype);
GSM_Error N6110_PrivSetAlarm(int alarm_number, GSM_DateTime *date_time, int msgtype);

GSM_Error N6110_DialVoice(char *Number);
GSM_Error N6110_DialData(char *Number, char type,  void (* callpassup)(char c));
GSM_Error N6110_AnswerCall(char s);

GSM_Error N6110_GetIncomingCallNr(char *Number);

GSM_Error N6110_SendBitmap(char *NetworkCode, int width, int height,
                          unsigned char *bitmap);
GSM_Error N6110_GetNetworkInfo(GSM_NetworkInfo *NetworkInfo);

GSM_Error N6110_GetCalendarNote(GSM_CalendarNote *CalendarNote);
GSM_Error N6110_WriteCalendarNote(GSM_CalendarNote *CalendarNote);
GSM_Error N6110_DeleteCalendarNote(GSM_CalendarNote *CalendarNote);

GSM_Error N6110_NetMonitor(unsigned char mode, char *Screen);

GSM_Error N6110_PlayTone(int Herz, u8 Volume);

GSM_Error N6110_SetBitmap( GSM_Bitmap *Bitmap );
GSM_Error N6110_GetBitmap( GSM_Bitmap *Bitmap );

GSM_Error N6110_SetRingTone(GSM_Ringtone *ringtone, int *maxlength);
GSM_Error N6110_SetBinRingTone(GSM_BinRingtone *ringtone);
GSM_Error N6110_GetBinRingTone(GSM_BinRingtone *ringtone);

GSM_Error N6110_EnableDisplayOutput();
GSM_Error N6110_DisableDisplayOutput();

GSM_Error N6110_EnableCellBroadcast ();
GSM_Error N6110_DisableCellBroadcast(void);
GSM_Error N6110_ReadCellBroadcast (GSM_CBMessage *Message);

GSM_Error N6110_SendDTMF(char *String);

GSM_Error N6110_Reset(unsigned char type);

GSM_Error N6110_GetProfile(GSM_Profile *Profile);
GSM_Error N6110_SetProfile(GSM_Profile *Profile);
bool      N6110_SendRLPFrame(RLP_F96Frame *frame, bool out_dtx);

GSM_Error N6110_CancelCall(void);

GSM_Error N6110_PressKey(int key, int event);

GSM_Error N6110_GetProductProfileSetting (GSM_PPS *PPS);
GSM_Error N6110_SetProductProfileSetting (GSM_PPS *PPS);

GSM_Error N6110_CallDivert(GSM_CallDivert *cd);

void N6110_RX_HandleRLPMessage(u8 *MessageBuffer);

GSM_Error N6110_SendIMEIFrame();
GSM_Error N6110_SendIDFrame();
GSM_Error N6110_SendHWFrame();

GSM_Error N6110_EnableExtendedCommands (unsigned char status);

GSM_Error N6110_Tests();

void N6110_DisplayTestsInfo(u8 *MessageBuffer);

GSM_Error N6110_SimlockInfo(GSM_AllSimlocks *siml);
GSM_Error N6110_GetSMSFolders ( GSM_SMSFolders *folders);

GSM_Error N6110_ResetPhoneSettings();

void N6110_Dispatch0x01Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType);

void N6110_ReplySendSMSMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType);
void N6110_ReplySetSMSCenter  (u16 MessageLength, u8 *MessageBuffer, u8 MessageType);
void N6110_ReplyGetSMSCenter  (u16 MessageLength, u8 *MessageBuffer, u8 MessageType);
void N6110_Dispatch0x02Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType);
void N6110_ReplySetCellBroadcast (u16 MessageLength, u8 *MessageBuffer, u8 MessageType);
void N6110_ReplyReadCellBroadcast(u16 MessageLength, u8 *MessageBuffer, u8 MessageType);

void N6110_ReplyCallDivert    (u16 MessageLength, u8 *MessageBuffer, u8 MessageType);
void N6110_Dispatch0x06Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType);

void N6110_ReplySetDateTime(u16 MessageLength,u8 *MessageBuffer, u8 MessageType);
void N6110_ReplyGetDateTime(u16 MessageLength,u8 *MessageBuffer, u8 MessageType);
void N6110_ReplySetAlarm   (u16 MessageLength,u8 *MessageBuffer, u8 MessageType);
void N6110_ReplyGetAlarm   (u16 MessageLength,u8 *MessageBuffer, u8 MessageType);

void N6110_ReplyEnableExtendedCommands  (u16 MessageLength,u8 *MessageBuffer,u8 MessageType);
void N6110_ReplyResetPhoneSettings      (u16 MessageLength,u8 *MessageBuffer,u8 MessageType);
void N6110_ReplyIMEI                    (u16 MessageLength,u8 *MessageBuffer,u8 MessageType);
void N6110_ReplyGetProductProfileSetting(u16 MessageLength,u8 *MessageBuffer,u8 MessageType);
void N6110_ReplySetProductProfileSetting(u16 MessageLength,u8 *MessageBuffer,u8 MessageType);
void N6110_ReplyNetmonitor              (u16 MessageLength,u8 *MessageBuffer,u8 MessageType);
void N6110_ReplySimlockInfo             (u16 MessageLength,u8 *MessageBuffer,u8 MessageType);
void N6110_ReplyPlayTone                (u16 MessageLength,u8 *MessageBuffer,u8 MessageType);
void N6110_ReplyHW                      (u16 MessageLength,u8 *MessageBuffer,u8 MessageType);
void N6110_Dispatch0x40Message          (u16 MessageLength,u8 *MessageBuffer,u8 MessageType);

void N6110_DispatchACKMessage (u16 MessageLength, u8 *MessageBuffer, u8 MessageType);
void N6110_ReplyID(u16 MessageLength, u8 *MessageBuffer, u8 MessageType);

GSM_Error GSM_EncodeNokiaSMSFrame(GSM_SMSMessage *SMS, unsigned char *req, int *length, SMS_MessageType PDU);
GSM_Error GSM_DecodeNokiaSMSFrame(GSM_SMSMessage *SMS, unsigned char *req, int length);

int N6110_MakeCallerGroupFrame (unsigned char *req,GSM_Bitmap Bitmap);
int N6110_MakeOperatorLogoFrame(unsigned char *req,GSM_Bitmap Bitmap);
int N6110_MakeStartupLogoFrame (unsigned char *req,GSM_Bitmap Bitmap);

#endif /* __n_6110_h */
