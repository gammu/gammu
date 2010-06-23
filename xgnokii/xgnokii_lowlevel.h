/*

  X G N O K I I

  A Linux/Unix GUI for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#ifndef XGNOKII_LOWLEVEL_H
#define XGNOKII_LOWLEVEL_H

#include <pthread.h>
#include <glib.h>
#include "gsm-api.h"

#define INCALL_NUMBER_LENGTH	20
#define NETMON_SCREEN_LENGTH	60

typedef enum {
  CS_Idle,
  CS_Waiting,
  CS_InProgress
} CallState;

typedef enum {
  Event_GetMemoryStatus,
  Event_GetMemoryLocation,
  Event_GetMemoryLocationAll,
  Event_WriteMemoryLocation,
  Event_WriteMemoryLocationAll,
  Event_GetCalendarNote,
  Event_GetCalendarNoteAll,
  Event_WriteCalendarNote,
  Event_DeleteCalendarNote,
  Event_GetCallerGroup,
  Event_SendCallerGroup,
  Event_GetSMSCenter,
  Event_SetSMSCenter,
  Event_SendSMSMessage,
  Event_DeleteSMSMessage,
  Event_GetSpeedDial,
  Event_SendSpeedDial,
  Event_SendDTMF,
  Event_NetMonitorOnOff,
  Event_NetMonitor,
  Event_DialVoice,
  Event_GetAlarm,
  Event_SetAlarm,
  Event_SendKeyStroke,
  Event_GetBitmap,
  Event_SetBitmap,
  Event_GetNetworkInfo,
  Event_Exit
} PhoneAction;

typedef struct {
  PhoneAction event;
  gpointer    data;
} PhoneEvent;

typedef struct {
  GSM_SpeedDial entry;
  GSM_Error status;
} D_SpeedDial;

typedef struct {
  GSM_SMSMessage *sms;
  GSM_Error status;
} D_SMSMessage;

typedef struct {
  GSM_MessageCenter *center;
  GSM_Error status;
} D_SMSCenter;

typedef struct {
  guchar number;
  gchar  text[256];
  gint   status;
} D_CallerGroup;

typedef struct {
  GSM_DateTime time;
  gint status;
} D_Alarm;

typedef struct {
  GSM_MemoryStatus memoryStatus;
  gint status;
} D_MemoryStatus;

typedef struct {
  GSM_PhonebookEntry *entry;
  gint status;
} D_MemoryLocation;

typedef struct {
  gint min;
  gint max;
  gint used;
  GSM_MemoryType type;
  gint status;
  gint (*InsertEntry)(GSM_PhonebookEntry *);
  gint (*ReadFailed)(gint);
} D_MemoryLocationAll;

typedef struct {
  GSM_CalendarNote *entry;
  gint status;
} D_CalendarNote;

typedef struct {
  gint status;
  gint (*InsertEntry)(GSM_CalendarNote *);
  gint (*ReadFailed)(gint);
} D_CalendarNoteAll;

typedef struct {
  GSM_Error status;
  GSM_Bitmap *bitmap;
} D_Bitmap;

typedef struct {
  GSM_Error status;
  GSM_NetworkInfo *info;
} D_NetworkInfo;

typedef struct {
  gfloat rfLevel;
  gfloat batteryLevel;
  GSM_PowerSource powerSource;
  gchar *working;
  bool alarm;
  struct {
    gchar *model;
    gchar *imei;
    gchar *revision;
    gchar *version;
  } phone;
  struct {
    gint    unRead;
    gint    number;
    gint    number2;
    GSList *messages;
  } sms;
  struct {
    CallState callInProgress;
    gchar     callNum[INCALL_NUMBER_LENGTH];
  } call;
  struct {
    gint  number;
    gchar screen[NETMON_SCREEN_LENGTH];
    gchar screen3[NETMON_SCREEN_LENGTH];
    gchar screen4[NETMON_SCREEN_LENGTH];
    gchar screen5[NETMON_SCREEN_LENGTH];
  } netmonitor;
} PhoneMonitor;

extern pthread_t monitor_th;
extern PhoneMonitor phoneMonitor;
extern pthread_mutex_t memoryMutex;
extern pthread_cond_t  memoryCond;
extern pthread_mutex_t calendarMutex;
extern pthread_cond_t  calendarCond;
extern pthread_mutex_t smsMutex;
extern pthread_mutex_t sendSMSMutex;
extern pthread_cond_t  sendSMSCond;
extern pthread_mutex_t callMutex;
extern pthread_mutex_t netMonMutex;
extern pthread_mutex_t speedDialMutex;
extern pthread_cond_t  speedDialCond;
extern pthread_mutex_t callerGroupMutex;
extern pthread_cond_t  callerGroupCond;
extern pthread_mutex_t smsCenterMutex;
extern pthread_cond_t  smsCenterCond;
extern pthread_mutex_t alarmMutex;
extern pthread_cond_t  alarmCond;
extern pthread_mutex_t getBitmapMutex;
extern pthread_cond_t  getBitmapCond;
extern pthread_mutex_t setBitmapMutex;
extern pthread_cond_t  setBitmapCond;
extern pthread_mutex_t getNetworkInfoMutex;
extern pthread_cond_t  getNetworkInfoCond;
extern void GUI_InitPhoneMonitor (void);
extern void *GUI_Connect (void *a);
extern void GUI_InsertEvent (PhoneEvent *event);

extern void RefreshSMS (const gint number);

#endif
