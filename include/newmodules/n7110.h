/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  Header file for the various functions, definitions etc. used to implement
  the handset interface.  See fbus-7110.c for more details.

*/

#ifndef __n_7110_h
#define __n_7110_h

#include "gsm-api.h"

/* Entry Types of 7110 for enhanced phonebook */
#define N7110_ENTRYTYPE_SPEEDDIAL 0x04 /* Speed dial */
#define N7110_ENTRYTYPE_NAME      0x07 /* Name always the only one */
#define N7110_ENTRYTYPE_EMAIL     0x08 /* Email Adress (TEXT) */
#define N7110_ENTRYTYPE_POSTAL    0x09 /* Postal Address (Text) */
#define N7110_ENTRYTYPE_NOTE      0x0a /* Note (Text) */
#define N7110_ENTRYTYPE_NUMBER    0x0b /* Phonenumber */
#define N7110_ENTRYTYPE_RINGTONE  0x0c /* Ringtone number */
#define N7110_ENTRYTYPE_DATE      0x13 /* Date for a Called List */
#define N7110_ENTRYTYPE_GROUPLOGO 0x1b /* Caller group logo */
#define N7110_ENTRYTYPE_LOGOON    0x1c /* Logo On? */
#define N7110_ENTRYTYPE_GROUP     0x1e /* Group number for phonebook entry */

/* Alternate defines for memory types.  Two letter codes follow GSM 07.07
   release 6.2.0, AT+CPBS "Select Phonebook Memory Storage" */
#define N7110_MEMORY_DC 0x01 /* ME dialled calls list */
#define N7110_MEMORY_MC 0x02 /* ME missed (unanswered received) calls list */
#define N7110_MEMORY_RC 0x03 /* ME received calls list */
#define N7110_MEMORY_FD 0x04 /* ?? SIM fixdialling-phonebook */
#define N7110_MEMORY_ME 0x05 /* ME (Mobile Equipment) phonebook */
#define N7110_MEMORY_SM 0x06 /* SIM phonebook */
#define N7110_MEMORY_ON 0x07 /* ?? SIM (or ME) own numbers list */
#define N7110_MEMORY_EN 0x08 /* ?? SIM (or ME) emergency number */
#define N7110_MEMORY_VM 0x09 /* voice mailbox */
#define N7110_MEMORY_SD 0x0e /* speed dials */
#define N7110_MEMORY_CG 0x10 /* Caller groups */

/* Every (well, almost every) frame from the computer starts with this
   sequence. */
#define N7110_FRAME_HEADER 0x00, 0x01, 0x01

/* some 6210/7110 folder stuff */
#define N7110_MAXSMSINFOLDER 160

/* Datatype for action in HandleSMSLocation */
typedef enum {
  GSH_GET,   /* Get SMS location */
  GSH_SAVE,  /* want to save at SMS location */
  GSH_INIT,  /* init folder table */
  GSH_DELETE /* Delete SMS location */
} GSM_SMSHandleAction;

/* Global variables */
extern bool            N7110_LinkOK;
extern GSM_Functions   N7110_Functions;
extern GSM_Information N7110_Information;

/* Prototypes for the functions designed to be used externally. */
GSM_Error N7110_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame));

void      N7110_DispatchMessage(u16 MessageLength, u8 *MessageBuffer,
                              u8 MessageType);

int N7110_GetMemoryType(GSM_MemoryType memory_type);

GSM_Error N7110_GetVoiceMailbox ( GSM_PhonebookEntry *entry);

GSM_Error N7110_GetMemoryStatus(GSM_MemoryStatus *Status);

GSM_Error N7110_GetProfile(GSM_Profile *Profile);

GSM_Error N7110_GetCalendarNote(GSM_CalendarNote *CalendarNote);
GSM_Error N7110_WriteCalendarNote(GSM_CalendarNote *CalendarNote);
GSM_Error N7110_DeleteCalendarNote(GSM_CalendarNote *CalendarNote);

GSM_Error N7110_GetRFLevel(GSM_RFUnits *units, float *level);
GSM_Error N7110_GetBatteryLevel(GSM_BatteryUnits *units, float *level);

GSM_Error N7110_DialVoice(char *Number);

GSM_Error N7110_GetSecurityCode(GSM_SecurityCode *SecurityCode);

GSM_Error N7110_GetDateTime(GSM_DateTime *date_time);
GSM_Error N7110_SetDateTime(GSM_DateTime *date_time);

GSM_Error N7110_GetAlarm(int alarm_number, GSM_DateTime *date_time);
GSM_Error N7110_SetAlarm(int alarm_number, GSM_DateTime *date_time);

GSM_Error N7110_GetSMSStatus(GSM_SMSStatus *Status);
GSM_Error N7110_DeleteSMSMessage(GSM_SMSMessage *Message);
GSM_Error N7110_SaveSMSMessage(GSM_SMSMessage *SMS);

GSM_Error N7110_GetMemoryLocation(GSM_PhonebookEntry *entry);
void N7110_DecodePhonebookFrame(GSM_PhonebookEntry *entry,u8 *MessageBuffer,u16 MessageLength);
GSM_Error N7110_WritePhonebookLocation(GSM_PhonebookEntry *entry);
int N7110_EncodePhonebookFrame(unsigned char *req, GSM_PhonebookEntry entry, int *block);
GSM_Error N7110_DeletePhonebookLocation(GSM_PhonebookEntry *entry);
GSM_Error N7110_WriteGroupDataLocation(GSM_Bitmap *bitmap);

GSM_Error N7110_SendDTMF(char *String);

GSM_Error N7110_GetSpeedDial(GSM_SpeedDial *entry);
GSM_Error N7110_SetSpeedDial(GSM_SpeedDial *entry);

GSM_Error N7110_GetSMSMessage(GSM_SMSMessage *message);

GSM_Error N7110_SetBitmap(GSM_Bitmap *Bitmap);
GSM_Error N7110_GetBitmap(GSM_Bitmap *Bitmap);

GSM_Error N7110_SetRingTone(GSM_Ringtone *ringtone, int *maxlength);

GSM_Error N7110_GetBinRingTone(GSM_BinRingtone *ringtone);
GSM_Error N7110_SetBinRingTone(GSM_BinRingtone *ringtone);

GSM_Error N7110_GetCalendarNotesInfo(GSM_NotesInfo *NotesInfo);

GSM_Error N7110_FirstCalendarFreePos(int *FirstFreePos);

GSM_Error N7110_GetSMSFolders (GSM_SMSFolders *folders);

GSM_Error N7110_GetWAPBookmark (GSM_WAPBookmark *bookmark);

GSM_Error N7110_SetWAPBookmark (GSM_WAPBookmark *bookmark);

GSM_Error N7110_GetWAPSettings (GSM_WAPSettings *settings);

GSM_Error N7110_HandleSMSLocation (int smsnum, u8 *folderid, int *location, GSM_SMSMessage *message, GSM_SMSHandleAction action);

void N7110_ReplyEnableWAPCommands(u16 MessageLength,u8 *MessageBuffer,u8 MessageType);
void N7110_ReplyGetWAPBookmark   (u16 MessageLength,u8 *MessageBuffer,u8 MessageType);
void N7110_ReplySetWAPBookmark   (u16 MessageLength, u8 *MessageBuffer, u8 MessageType);
void N7110_ReplyGetWAPSettings   (u16 MessageLength,u8 *MessageBuffer,u8 MessageType);

void N7110_Fake2SMSLocation(int smsnum, u8 *folder, int *location);
void N7110_SMS2FakeLocation(int *smsnum, u8 folder, int location);

#endif /* __n_7110_h */
