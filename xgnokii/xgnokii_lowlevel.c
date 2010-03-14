/*

  X G N O K I I

  A Linux/Unix GUI for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <glib.h>
#include "gsm-api.h"
#include "xgnokii_lowlevel.h"
#include "xgnokii.h"
#include "xgnokii_common.h"

pthread_t monitor_th;
PhoneMonitor phoneMonitor;
pthread_mutex_t memoryMutex;
pthread_cond_t  memoryCond;
pthread_mutex_t calendarMutex;
pthread_cond_t  calendarCond;
pthread_mutex_t smsMutex;
pthread_mutex_t sendSMSMutex;
pthread_cond_t  sendSMSCond;
pthread_mutex_t callMutex;
pthread_mutex_t netMonMutex;
pthread_mutex_t speedDialMutex;
pthread_cond_t  speedDialCond;
pthread_mutex_t callerGroupMutex;
pthread_cond_t  callerGroupCond;
pthread_mutex_t smsCenterMutex;  
pthread_cond_t  smsCenterCond;
pthread_mutex_t alarmMutex;  
pthread_cond_t  alarmCond;
pthread_mutex_t getBitmapMutex;
pthread_cond_t  getBitmapCond;
pthread_mutex_t setBitmapMutex;
pthread_cond_t  setBitmapCond;
pthread_mutex_t getNetworkInfoMutex;
pthread_cond_t  getNetworkInfoCond;
static pthread_mutex_t eventsMutex;
static GSList *ScheduledEvents = NULL;


inline void GUI_InsertEvent (PhoneEvent *event)
{
  while (phoneMonitor.working) {
    usleep(1);
  }

# ifdef XDEBUG
  g_print ("Inserting Event: %d\n", event->event);
# endif
  pthread_mutex_lock (&eventsMutex);
  ScheduledEvents = g_slist_prepend (ScheduledEvents, event);
  pthread_mutex_unlock (&eventsMutex);
}


inline static PhoneEvent *RemoveEvent (void)
{
  GSList *list;
  PhoneEvent *event = NULL;

  pthread_mutex_lock (&eventsMutex);
  list = g_slist_last (ScheduledEvents);
  if (list)
  {
    event = (PhoneEvent *) list->data;
    ScheduledEvents = g_slist_remove_link (ScheduledEvents, list);
    g_slist_free_1 (list);
  }
  pthread_mutex_unlock (&eventsMutex);

  return (event);
}


static void InitModelInf (void)
{
  gchar buf[64];
  GSM_Error error;
  register gint i = 0;

  while ((error = GSM->GetModel(buf)) != GE_NONE && i++ < 15)
    sleep(1);

  if (error == GE_NONE)
  {
    g_free (phoneMonitor.phone.model);
    phoneMonitor.phone.version = g_strdup (buf);
    phoneMonitor.phone.model = GetModelName (buf);
    if (phoneMonitor.phone.model == NULL)
      phoneMonitor.phone.model = g_strdup (_("unknown"));
  }

  i = 0;
  while ((error = GSM->GetRevision (buf)) != GE_NONE && i++ < 5)
    sleep(1);

  if (error == GE_NONE)
  {
    g_free (phoneMonitor.phone.revision);
    phoneMonitor.phone.revision = g_strdup (buf);
  }

  i = 0;
  while ((error = GSM->GetIMEI (buf)) != GE_NONE && i++ < 5)
    sleep(1);

  if (error == GE_NONE)
  {
    g_free (phoneMonitor.phone.imei);
    phoneMonitor.phone.imei = g_strdup (buf);
  }


#ifdef XDEBUG
  g_print ("Version: %s\n", phoneMonitor.phone.version);
  g_print ("Model: %s\n", phoneMonitor.phone.model);
  g_print ("IMEI: %s\n", phoneMonitor.phone.imei);
  g_print ("Revision: %s\n", phoneMonitor.phone.revision);
#endif
}

static GSM_Error fbusinit(bool enable_monitoring)
{
  int count=0;
  static GSM_Error error=GE_NOLINK;

#ifndef WIN32
  if (strcmp(GetMygnokiiVersion(),VERSION)!=0)
    fprintf(stderr,_("WARNING: version of installed libmygnokii.so (%s) is different to version of xgnokii (%s)\n"),GetMygnokiiVersion(),VERSION);
#endif

  /* Initialise the code for the GSM interface. */     
  if (error == GE_NOLINK)
    error = GSM_Initialise (xgnokiiConfig.model, xgnokiiConfig.port,
                            xgnokiiConfig.initlength,
			    GetConnectionTypeFromString(xgnokiiConfig.connection),
			    RLP_DisplayF96Frame,
			    xgnokiiConfig.synchronizetime);

#ifdef XDEBUG
  g_print ("fbusinit: error %d\n", error);
#endif

  if (error != GE_NONE) {
    g_print (_("GSM/FBUS init failed! (Unknown model ?). Quitting.\n"));
    /* FIXME: should popup some message... */
    return (error);
  }

  while (count++ < 40 && *GSM_LinkOK == false)
    usleep(50000);
#ifdef XDEBUG
  g_print("After usleep. GSM_LinkOK: %d\n", *GSM_LinkOK);
#endif

  if (*GSM_LinkOK == true)
    InitModelInf ();

  return *GSM_LinkOK;
}


void GUI_InitPhoneMonitor (void)
{
  phoneMonitor.phone.model = g_strdup (_("unknown"));
  phoneMonitor.phone.version = phoneMonitor.phone.model;
  phoneMonitor.phone.revision = g_strdup (_("unknown"));
  phoneMonitor.phone.imei = g_strdup (_("unknown"));
  phoneMonitor.rfLevel = phoneMonitor.batteryLevel = -1;
  phoneMonitor.powerSource = GPS_BATTERY;
  phoneMonitor.working = FALSE;
  phoneMonitor.alarm = FALSE;
  phoneMonitor.sms.unRead = phoneMonitor.sms.number = phoneMonitor.sms.number2 = 0;
  phoneMonitor.sms.messages = NULL;
  phoneMonitor.call.callInProgress = CS_Idle;
  *phoneMonitor.call.callNum = '\0';
  phoneMonitor.netmonitor.number = 0;
  *phoneMonitor.netmonitor.screen = *phoneMonitor.netmonitor.screen3 = 
  *phoneMonitor.netmonitor.screen4 = *phoneMonitor.netmonitor.screen5 = '\0';
  pthread_mutex_init (&memoryMutex, NULL);
  pthread_cond_init (&memoryCond, NULL);
  pthread_mutex_init (&calendarMutex, NULL);
  pthread_cond_init (&calendarCond, NULL);
  pthread_mutex_init (&smsMutex, NULL);
  pthread_mutex_init (&sendSMSMutex, NULL);
  pthread_cond_init (&sendSMSCond, NULL);
  pthread_mutex_init (&callMutex, NULL);
  pthread_mutex_init (&eventsMutex, NULL);
  pthread_mutex_init (&callMutex, NULL);
  pthread_mutex_init (&netMonMutex, NULL);
  pthread_mutex_init (&speedDialMutex, NULL);
  pthread_cond_init (&speedDialCond, NULL);
  pthread_mutex_init (&callerGroupMutex, NULL);
  pthread_cond_init (&callerGroupCond, NULL);
  pthread_mutex_init (&smsCenterMutex, NULL);
  pthread_cond_init (&smsCenterCond, NULL);
  pthread_mutex_init (&getBitmapMutex, NULL);
  pthread_cond_init (&getBitmapCond, NULL);
  pthread_mutex_init (&setBitmapMutex, NULL);
  pthread_cond_init (&setBitmapCond, NULL);
  pthread_mutex_init (&getNetworkInfoMutex, NULL);
  pthread_cond_init (&getNetworkInfoCond, NULL);
}


static inline void FreeElement (gpointer data, gpointer userData)
{
  g_free ((GSM_SMSMessage *) data);
}


static inline void FreeArray (GSList **array)
{
  if (*array)
  {
    g_slist_foreach (*array, FreeElement, NULL);
    g_slist_free (*array);
    *array = NULL;
  }
}


void RefreshSMS (const gint number)
{
  GSM_Error error;
  GSM_SMSMessage *msg;
  register gint i;

# ifdef XDEBUG
  g_print ("RefreshSMS is running...\n");
# endif

  pthread_mutex_lock (&smsMutex);
  FreeArray (&(phoneMonitor.sms.messages));
  phoneMonitor.sms.number = 0;
  phoneMonitor.sms.number2 = 0;
  pthread_mutex_unlock (&smsMutex);

  i = 1;
  while (1)
  {
    msg = g_malloc (sizeof (GSM_SMSMessage));
    msg->MemoryType = GMT_SM;
    msg->Location = i;
#ifdef XDEBUG
    fprintf(stdout, _("test: %d %i %i %i\n"),msg->Location,phoneMonitor.sms.number,phoneMonitor.sms.number2,number);
#endif
    pthread_mutex_lock (&smsMutex);
    if( GetModelFeature(FN_SMS)==F_SMS71 ) msg->Location = 0;  /* read next sms for 6210/7110 */
    error = GSM->GetSMSMessage (msg);
    pthread_mutex_unlock (&smsMutex);
    switch (error) {
      case GE_NONE:
        pthread_mutex_lock (&smsMutex);
#ifdef XDEBUG
    fprintf(stdout, _("Refresh SMS: g_slist_append: sms message"));
    fprintf(stdout, _(" Location: %d"),msg->Location);
    fprintf(stdout, _(" folder: %d"),msg->folder);
    if ( msg-> SMSData) fprintf(stdout, _(" SMSData true\n"));
    	else  fprintf(stdout, _(" SMSData false\n"));
#endif
	/* RTH:  unread sms from folder 0 to INBOX */
        phoneMonitor.sms.messages = g_slist_append (phoneMonitor.sms.messages, msg);
        phoneMonitor.sms.number++;
        phoneMonitor.sms.number2++;
        pthread_mutex_unlock (&smsMutex);
        if (phoneMonitor.sms.number2 == number) return;
	break;
      case GE_SMSTOOLONG: /* Picture Image in 7110 comp phone */
        g_free (msg);
        phoneMonitor.sms.number2++;
        if (phoneMonitor.sms.number2 == number) return;
	break;
      default:
        g_free (msg);
	break;
    }
    i++;
  }
}


static gint A_GetMemoryStatus (gpointer data)
{
  GSM_Error error;
  D_MemoryStatus *ms = (D_MemoryStatus *) data;

  error = ms->status = GE_UNKNOWN;

  if (ms)
  {
    pthread_mutex_lock (&memoryMutex);
    error = ms->status = GSM->GetMemoryStatus (&(ms->memoryStatus));
    pthread_cond_signal (&memoryCond);
    pthread_mutex_unlock (&memoryMutex);
  }

  return (error);
}


static gint A_GetMemoryLocation (gpointer data)
{
  GSM_Error error;
  D_MemoryLocation *ml = (D_MemoryLocation *) data;

  error = ml->status = GE_UNKNOWN;

  if (ml)
  {
    pthread_mutex_lock (&memoryMutex);
    error = ml->status = GSM->GetMemoryLocation (ml->entry);
    pthread_cond_signal (&memoryCond);
    pthread_mutex_unlock (&memoryMutex);
  }

  return (error);
}


static gint A_GetMemoryLocationAll (gpointer data)
{
  GSM_PhonebookEntry entry;
  GSM_Error error;
  D_MemoryLocationAll *mla = (D_MemoryLocationAll *) data;
  register gint i;
  gint readed=0;

  error = mla->status = GE_NONE;
  entry.MemoryType = mla->type;

  pthread_mutex_lock (&memoryMutex);
  for (i = mla->min; i <= mla->max; i++)
  {
    entry.Location = i;
    if (readed<mla->used) {
      error = GSM->GetMemoryLocation (&entry);
      if (error != GE_NONE)
      {
        gint err_count = 0;

        while (error != GE_NONE &&
	       error != GE_INVALIDPHBOOKLOCATION && error != GE_UNKNOWN) //n7110.c
        {
          g_print (_("%s: line %d: Can't get memory entry number %d from memory %d! %d\n"),
                   __FILE__, __LINE__, i, entry.MemoryType, error);
          if (err_count++ > 3)
          {
            mla->ReadFailed (i);
            mla->status = error;
            pthread_cond_signal (&memoryCond);
            pthread_mutex_unlock (&memoryMutex);
            return (error);
          }

          error = GSM->GetMemoryLocation (&entry);
          sleep (2);
        }
      }
      
      if (strcmp(entry.Number, "\0") || strcmp(entry.Name, "\0"))
        readed++;

    } else {
      entry.Number[0]='\0';
      entry.Name[0]='\0';
    }

    error = mla->InsertEntry (&entry);
    if (error != GE_NONE)
      break;
  }
  mla->status = error;
  pthread_cond_signal (&memoryCond);
  pthread_mutex_unlock (&memoryMutex);
  return (error);
}


static gint A_WriteMemoryLocation (gpointer data)
{
  GSM_Error error;
  D_MemoryLocation *ml = (D_MemoryLocation *) data;

  error = ml->status = GE_UNKNOWN;

  if (ml)
  {
    pthread_mutex_lock (&memoryMutex);
    error = ml->status = GSM->WritePhonebookLocation (ml->entry);
    pthread_cond_signal (&memoryCond);
    pthread_mutex_unlock (&memoryMutex);
  }

  return (error);
}


static gint A_WriteMemoryLocationAll (gpointer data)
{
/*  GSM_PhonebookEntry entry; */
  GSM_Error error;
  D_MemoryLocationAll *mla = (D_MemoryLocationAll *) data;
/*  register gint i;
*/
  error = mla->status = GE_NONE;
/*  entry.MemoryType = mla->type;

  pthread_mutex_lock (&memoryMutex);
  for (i = mla->min; i <= mla->max; i++)
  {
    entry.Location = i;
    error = GSM->GetMemoryLocation (&entry);
    if (error != GE_NONE)
    {
      gint err_count = 0;

      while (error != GE_NONE)
      {
        g_print (_("%s: line %d: Can't get memory entry number %d from memory %d! %d\n"),
                 __FILE__, __LINE__, i, entry.MemoryType, error);
        if (err_count++ > 3)
        {
          mla->ReadFailed (i);
          mla->status = error;
          pthread_cond_signal (&memoryCond);
          pthread_mutex_unlock (&memoryMutex);
          return (error);
        }

        error = GSM->GetMemoryLocation (&entry);
        sleep (2);
      }
    }
    error = mla->InsertEntry (&entry);
    if (error != GE_NONE)
      break;
  }
  mla->status = error;
  pthread_cond_signal (&memoryCond);
  pthread_mutex_unlock (&memoryMutex); */
  return (error);
}


static gint A_GetCalendarNote (gpointer data)
{
  GSM_Error error;
  D_CalendarNote *cn = (D_CalendarNote *) data;

  error = cn->status = GE_UNKNOWN;

  if (cn)
  {
    pthread_mutex_lock (&calendarMutex);
    error = cn->status = GSM->GetCalendarNote (cn->entry);
    pthread_cond_signal (&calendarCond);
    pthread_mutex_unlock (&calendarMutex);
  }

  return (error);
}


static gint A_GetCalendarNoteAll (gpointer data)
{
  GSM_CalendarNote entry;
  D_CalendarNoteAll *cna = (D_CalendarNoteAll *) data;
  GSM_Error e;
  register gint i = 1;

  pthread_mutex_lock (&calendarMutex);
  while (1)
  {
    entry.Location = i++;

    if ((e = GSM->GetCalendarNote (&entry)) != GE_NONE)
      break;

    if (cna->InsertEntry (&entry) != GE_NONE)
      break;
  }

  pthread_mutex_unlock (&calendarMutex);
  g_free (cna);
  if (e == GE_INVALIDCALNOTELOCATION)
    return (GE_NONE);
  else
    return (e);
}

static gint A_WriteCalendarNote (gpointer data)
{
  GSM_Error error;
  D_CalendarNote *cn = (D_CalendarNote *) data;

  error = cn->status = GE_UNKNOWN;

  if (cn)
  {
    pthread_mutex_lock (&calendarMutex);
    error = cn->status = GSM->WriteCalendarNote (cn->entry);
    pthread_cond_signal (&calendarCond);
    pthread_mutex_unlock (&calendarMutex);
  }

  return (error);
}


static gint A_DeleteCalendarNote (gpointer data)
{
  GSM_CalendarNote *note = (GSM_CalendarNote *) data;
  GSM_Error error = GE_UNKNOWN;

  if (note)
  {
    error = GSM->DeleteCalendarNote (note);
    g_free (note);
  }

  return (error);
}

static gint A_GetCallerGroup (gpointer data)
{
  GSM_Bitmap bitmap;
  GSM_Error error;
  D_CallerGroup *cg = (D_CallerGroup *) data;

  error = cg->status = GE_UNKNOWN;

  if (cg)
  {
    bitmap.type = GSM_CallerLogo;
    bitmap.number = cg->number;

    pthread_mutex_lock (&callerGroupMutex);
    error = cg->status = GSM->GetBitmap (&bitmap);
    strncpy (cg->text, bitmap.text, 256);
    cg->text[255] = '\0';
    pthread_cond_signal (&callerGroupCond);
    pthread_mutex_unlock (&callerGroupMutex);
  }

  return (error);
}


static gint A_SendCallerGroup (gpointer data)
{
  GSM_Bitmap bitmap;
  D_CallerGroup *cg = (D_CallerGroup *) data;
  GSM_Error error;

  if (!cg)
    return (GE_UNKNOWN);

  bitmap.type = GSM_CallerLogo;
  bitmap.number = cg->number;
  if ((error = GSM->GetBitmap (&bitmap)) != GE_NONE)
  {
    g_free (cg);
    return (error);
  }
  strncpy (bitmap.text, cg->text, 256);
  bitmap.text[255] = '\0';
  g_free (cg);
  return (GSM->SetBitmap (&bitmap));
}


static gint A_GetSMSCenter (gpointer data)
{
  D_SMSCenter *c = (D_SMSCenter *) data;
  GSM_Error error;

  error = c->status = GE_UNKNOWN;
  if (c)
  {
    pthread_mutex_lock (&smsCenterMutex);
    error = c->status = GSM->GetSMSCenter (c->center);
    pthread_cond_signal (&smsCenterCond);
    pthread_mutex_unlock (&smsCenterMutex);
  }

  return (error);
}


static gint A_SetSMSCenter (gpointer data)
{
  D_SMSCenter *c = (D_SMSCenter *) data;
  GSM_Error error;

  error = c->status = GE_UNKNOWN;
  if (c)
  {
    //pthread_mutex_lock (&smsCenterMutex);
    error = c->status = GSM->SetSMSCenter (c->center);
    g_free (c);
    //pthread_cond_signal (&smsCenterCond);
    //pthread_mutex_unlock (&smsCenterMutex);
  }

  return (error);
}


static gint A_SendSMSMessage (gpointer data)
{
  D_SMSMessage *d = (D_SMSMessage *) data;
  GSM_Error error;

  error = d->status = GE_UNKNOWN;
  if (d)
  {
    pthread_mutex_lock (&sendSMSMutex);
    error = d->status = GSM->SendSMSMessage (d->sms);
    pthread_cond_signal (&sendSMSCond);
    pthread_mutex_unlock (&sendSMSMutex);
  }

  if (d->status == GE_SMSSENDOK)
    return (GE_NONE);
  else
    return (error);
}


static gint A_DeleteSMSMessage (gpointer data)
{
  GSM_SMSMessage *sms = (GSM_SMSMessage *) data;
  GSM_Error error = GE_UNKNOWN;

  if (sms)
  {
    error = GSM->DeleteSMSMessage(sms);
    g_free (sms);
  }

  return (error);
}


static gint A_GetSpeedDial (gpointer data)
{
  D_SpeedDial *d = (D_SpeedDial *) data;
  GSM_Error error;

  error = d->status = GE_UNKNOWN;

  if (d)
  {
    pthread_mutex_lock (&speedDialMutex);
    error = d->status = GSM->GetSpeedDial (&(d->entry));
    pthread_cond_signal (&speedDialCond);
    pthread_mutex_unlock (&speedDialMutex);
  }

  return (error);
}


static gint A_SendSpeedDial (gpointer data)
{
  D_SpeedDial *d = (D_SpeedDial *) data;
  GSM_Error error;

  error = d->status = GE_UNKNOWN;

  if (d)
  {
    //pthread_mutex_lock (&speedDialMutex);
    error = d->status = GSM->SetSpeedDial (&(d->entry));
    g_free (d);
    //pthread_cond_signal (&speedDialCond);
    //pthread_mutex_unlock (&speedDialMutex);
  }

  return (error);
}


static gint A_SendDTMF (gpointer data)
{
  gchar *buf = (gchar *) data;
  GSM_Error error = GE_UNKNOWN;

  if (buf) 
  {
    error = GSM->SendDTMF (buf);
    g_free (buf);
  }

  return (error);
}


static gint A_NetMonOnOff (gpointer data)
{
  gchar screen[50];
  gint mode = GPOINTER_TO_INT (data);
  GSM_Error error = GE_UNKNOWN;

  if (mode)
    error = GSM->NetMonitor (0xf3, screen);
  else
    error = GSM->NetMonitor (0xf1, screen);

  return (error);
}


static gint A_NetMonitor (gpointer data)
{
  gint number = GPOINTER_TO_INT (data);

  if (data == 0)
    phoneMonitor.netmonitor.number = 0;
  else
    phoneMonitor.netmonitor.number = number;
    
  return (0);
}


static gint A_DialVoice (gpointer data)
{
  gchar *number = (gchar *) data;
  GSM_Error error = GE_UNKNOWN;

  if (number)
  {
    error = GSM->DialVoice (number);
    g_free (number);
  }

  return (error);
}


static gint A_GetAlarm (gpointer data)
{
  D_Alarm *a = (D_Alarm *) data;
  GSM_Error error;

  error = a->status = GE_UNKNOWN;

  if (a)
  {
    pthread_mutex_lock (&alarmMutex);
    error = a->status = GSM->GetAlarm (0, &(a->time));
    pthread_cond_signal (&alarmCond);
    pthread_mutex_unlock (&alarmMutex);
  }

  return (error);
}


static gint A_SetAlarm (gpointer data)
{
  D_Alarm *a = (D_Alarm *) data;
  GSM_Error error;

  error = a->status = GE_UNKNOWN;

  if (a)
  {
    error = a->status = GSM->SetAlarm (0, &(a->time));
    g_free (a);
  }

  return (error);
}


static gint A_SendKeyStroke (gpointer data)
{
  gchar *buf = (gchar *) data;

  if (buf) 
  {
    GSM->PressKey(buf[1], buf[0]);
    g_free (buf);
  }

  return (0);
}

static gint A_GetBitmap(gpointer data) {
  GSM_Error error;
  D_Bitmap *d = (D_Bitmap *)data;

  pthread_mutex_lock(&getBitmapMutex);
  error = d->status = GSM->GetBitmap(d->bitmap);
  pthread_cond_signal(&getBitmapCond);
  pthread_mutex_unlock(&getBitmapMutex);
  return error;
}

static gint A_SetBitmap(gpointer data) {
  GSM_Error error;
  D_Bitmap *d = (D_Bitmap *)data;
  GSM_Bitmap bitmap;
  
  pthread_mutex_lock(&setBitmapMutex);
  if (d->bitmap->type == GSM_CallerLogo) {
    bitmap.type = d->bitmap->type;
    bitmap.number = d->bitmap->number;
    error = d->status = GSM->GetBitmap(&bitmap);
    if (error == GE_NONE) {
      strncpy(d->bitmap->text,bitmap.text,sizeof(bitmap.text));
      d->bitmap->ringtone = bitmap.ringtone;
      error = d->status = GSM->SetBitmap(d->bitmap);
    }
  } else {
    error = d->status = GSM->SetBitmap(d->bitmap);
  }
  pthread_cond_signal(&setBitmapCond);
  pthread_mutex_unlock(&setBitmapMutex);
  return error;
}

static gint A_GetNetworkInfo(gpointer data) {
  GSM_Error error;
  D_NetworkInfo *d = (D_NetworkInfo *)data;

  pthread_mutex_lock(&getNetworkInfoMutex);
  error = d->status = GSM->GetNetworkInfo(d->info);
  pthread_cond_signal(&getNetworkInfoCond);
  pthread_mutex_unlock(&getNetworkInfoMutex);
  return error;
}

static gint A_Exit (gpointer data)
{
  pthread_exit (0);
  return (0); /* just to be proper */
}


gint (*DoAction[])(gpointer) = {
  A_GetMemoryStatus,
  A_GetMemoryLocation,
  A_GetMemoryLocationAll,
  A_WriteMemoryLocation,
  A_WriteMemoryLocationAll,
  A_GetCalendarNote,
  A_GetCalendarNoteAll,
  A_WriteCalendarNote,
  A_DeleteCalendarNote,
  A_GetCallerGroup,
  A_SendCallerGroup,
  A_GetSMSCenter,
  A_SetSMSCenter,
  A_SendSMSMessage,
  A_DeleteSMSMessage,
  A_GetSpeedDial,
  A_SendSpeedDial,
  A_SendDTMF,
  A_NetMonOnOff,
  A_NetMonitor,
  A_DialVoice,
  A_GetAlarm,
  A_SetAlarm,
  A_SendKeyStroke,
  A_GetBitmap,
  A_SetBitmap,
  A_GetNetworkInfo,
  A_Exit
};

void *GUI_Connect (void *a)
{
  /* Define required unit types for RF and Battery level meters. */
  GSM_RFUnits rf_units = GRF_Arbitrary;
  GSM_BatteryUnits batt_units = GBU_Arbitrary;

  GSM_DateTime Alarm;
  gchar number[INCALL_NUMBER_LENGTH];
  PhoneEvent *event=NULL;
  GSM_Error error;
  gint status;


# ifdef XDEBUG
  g_print ("Initializing connection...\n");
# endif

  phoneMonitor.working = _("Connecting...");
  while (!fbusinit (true))
    sleep (1);

# ifdef XDEBUG
  g_print ("Phone connected. Starting monitoring...\n");
# endif

  while (1)
  {
    if (event != NULL) {
      phoneMonitor.working = _("Working...");
    } else {
      phoneMonitor.working = NULL;

      if (GSM->GetRFLevel (&rf_units, &phoneMonitor.rfLevel) != GE_NONE)
        phoneMonitor.rfLevel = -1;

      if (rf_units == GRF_Arbitrary)
        phoneMonitor.rfLevel *= 25;

      if (GSM->GetPowerSource (&phoneMonitor.powerSource) == GE_NONE 
          && phoneMonitor.powerSource == GPS_ACDC)
        phoneMonitor.batteryLevel = ((gint) phoneMonitor.batteryLevel + 25) % 125;
      else
      {
        if (GSM->GetBatteryLevel (&batt_units, &phoneMonitor.batteryLevel) != GE_NONE)
          phoneMonitor.batteryLevel = -1;
        if (batt_units == GBU_Arbitrary)
          phoneMonitor.batteryLevel *= 25;
      }

      if (GSM->GetAlarm (0, &Alarm) == GE_NONE && Alarm.IsSet != 0)
        phoneMonitor.alarm = TRUE;
      else
        phoneMonitor.alarm = FALSE;

      if (GSM->GetIncomingCallNr (number) == GE_NONE)
      {
#   ifdef XDEBUG
        g_print ("Call in progress: %s\n", phoneMonitor.call.callNum);
#   endif

        if (GSM->GetDisplayStatus (&status)==GE_NONE) {
          if (status & (1<<DS_Call_In_Progress))
          {
            pthread_mutex_lock (&callMutex);
            phoneMonitor.call.callInProgress = CS_InProgress;
            pthread_mutex_unlock (&callMutex);
          }
          else
          {
            pthread_mutex_lock (&callMutex);
            phoneMonitor.call.callInProgress = CS_Waiting;
            strncpy (phoneMonitor.call.callNum, number, INCALL_NUMBER_LENGTH);
            pthread_mutex_unlock (&callMutex);
	  }
        } else {
          pthread_mutex_lock (&callMutex);
          phoneMonitor.call.callInProgress = CS_Idle;
          *phoneMonitor.call.callNum = '\0';
          pthread_mutex_unlock (&callMutex);
	}
      }
      else
      {
        pthread_mutex_lock (&callMutex);
        phoneMonitor.call.callInProgress = CS_Idle;
        *phoneMonitor.call.callNum = '\0';
        pthread_mutex_unlock (&callMutex);
      }

      pthread_mutex_lock (&netMonMutex);
      if (phoneMonitor.netmonitor.number)
      {
        GSM->NetMonitor (phoneMonitor.netmonitor.number,
                         phoneMonitor.netmonitor.screen);
        GSM->NetMonitor (3, phoneMonitor.netmonitor.screen3);
        GSM->NetMonitor (4, phoneMonitor.netmonitor.screen4);
        GSM->NetMonitor (5, phoneMonitor.netmonitor.screen5);
      }
      else
      {
        *phoneMonitor.netmonitor.screen = *phoneMonitor.netmonitor.screen3 = 
        *phoneMonitor.netmonitor.screen4 = *phoneMonitor.netmonitor.screen5 = '\0';
      }
      pthread_mutex_unlock (&netMonMutex);
    }
    while ((event = RemoveEvent ()) != NULL)
    {
#     ifdef XDEBUG      
      g_print ("Processing Event: %d\n", event->event);
#     endif
      phoneMonitor.working = _("Working...");
      if (event->event <= Event_Exit)
        if ((error = DoAction[event->event] (event->data)) != GE_NONE)
          g_print (_("Event %d failed with return code %d!\n"), event->event, error);
      g_free (event);
    }
  }
}
