/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.
	
  Provides a generic API for accessing functions on the phone, wherever
  possible hiding the model specific details.

  The underlying code should run in it's own thread to allow communications to
  the phone to be run independantly of mailing code that calls these API
  functions.

  Unless otherwise noted, all functions herein block until they complete.  The
  functions themselves are defined in a structure in gsm-common.h.

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef WIN32
  #include <windows.h>
  #include "misc_win32.h"
#endif

#include "gsm-api.h"

#include "newmodules/n3110.h"
#include "newmodules/n6110.h"
#include "newmodules/n7110.h"
#include "newmodules/newat.h"
#ifdef DEBUG
  #include "newmodules/sniff/sniff.h"
#endif
#include "protocol/fbusirda.h"
#include "protocol/fbus.h"
#include "protocol/fbus3110.h"
#include "protocol/mbus.h"
#include "protocol/at.h"
#include "files/cfgreader.h"

#ifndef WIN32
  #include "devices/device.h"
#endif

#ifdef VC6
  /* for VC6 make scripts save VERSION constant in mversion.h file */
  #include "mversion.h"
#endif

/* GSM_LinkOK is set to true once normal communications with the phone have
   been established. */

bool *GSM_LinkOK;

/* Define pointer to the GSM_Functions structure used by external code to call
   relevant API functions. This structure is defined in gsm-common.h. */

GSM_Functions *GSM;

/* Define pointer to the GSM_Information structure used by external code to
   obtain information that varies from model to model. This structure is also
   defined in gsm-common.h */

GSM_Information		*GSM_Info;

/* Initialise interface to the phone. Model number should be a string such as
   3810, 5110, 6110 etc. Device is the serial port to use e.g. /dev/ttyS0, the
   user must have write permission to the device. */

GSM_Protocol *Protocol;

/* Local variables used by get/set phonebook entry code. Buffer is used as a
   source or destination for phonebook data and other functions... Error is
   set to GE_NONE by calling function, set to GE_COMPLETE or an error code by
   handler routines as appropriate. */
		   	   	   
GSM_PhonebookEntry *CurrentPhonebookEntry;
GSM_Error          CurrentPhonebookError;

GSM_SpeedDial      *CurrentSpeedDialEntry;
GSM_Error          CurrentSpeedDialError;

unsigned char      Current_IMEI[GSM_MAX_IMEI_LENGTH];
unsigned char      Current_Revision[GSM_MAX_REVISION_LENGTH];
unsigned char      Current_Model[GSM_MAX_MODEL_LENGTH];

GSM_SMSMessage     *CurrentSMSMessage;
GSM_Error          CurrentSMSMessageError;
int                CurrentSMSPointer;

GSM_SMSFolders      *CurrentSMSFolders;
GSM_Error          CurrentSMSFoldersError;
int                CurrentSMSFoldersCount;

GSM_OneSMSFolder   CurrentSMSFolder;
GSM_Error          CurrentSMSFolderError;
int                CurrentSMSFolderID;

GSM_MemoryStatus   *CurrentMemoryStatus;
GSM_Error          CurrentMemoryStatusError;

GSM_NetworkInfo    *CurrentNetworkInfo;
GSM_Error          CurrentNetworkInfoError;

GSM_SMSStatus      *CurrentSMSStatus;
GSM_Error          CurrentSMSStatusError;

GSM_MessageCenter  *CurrentMessageCenter;
GSM_Error          CurrentMessageCenterError;

int                *CurrentSecurityCodeStatus;
GSM_Error          CurrentSecurityCodeError;
GSM_SecurityCode   *CurrentSecurityCode;

GSM_DateTime       *CurrentDateTime;
GSM_Error          CurrentDateTimeError;

GSM_DateTime       *CurrentAlarm;
GSM_Error          CurrentAlarmError;

GSM_CalendarNote   *CurrentCalendarNote;
GSM_Error          CurrentCalendarNoteError;

GSM_NotesInfo      CurrentCalendarNotesInfo,*CurrentCalendarNotesInfo2;
GSM_Error          CurrentCalendarNotesInfoError;

GSM_Error          CurrentSetDateTimeError;
GSM_Error          CurrentSetAlarmError;

GSM_Error          CurrentEnableExtendedCommandsError;

int                CurrentRFLevel,
                   CurrentBatteryLevel,
                   CurrentPowerSource;

int                CurrentDisplayStatus;
GSM_Error          CurrentDisplayStatusError;

GSM_Error          CurrentResetPhoneSettingsError;

char               *CurrentNetmonitor;
GSM_Error          CurrentNetmonitorError;

GSM_Bitmap         *CurrentGetBitmap=NULL;
GSM_Error          CurrentGetBitmapError;

GSM_Error          CurrentSetBitmapError;

GSM_Error          CurrentSendDTMFError;

GSM_Profile        *CurrentProfile;
GSM_Error          CurrentProfileError;

GSM_Error          CurrentDisplayOutputError;

GSM_CBMessage      *CurrentCBMessage;
GSM_Error          CurrentCBError;

int                CurrentPressKeyEvent;
GSM_Error          CurrentPressKeyError;

GSM_Error          CurrentPlayToneError=GE_UNKNOWN;

GSM_Error          CurrentDialVoiceError;

GSM_Error          CurrentGetOperatorNameError;
GSM_Network        *CurrentGetOperatorNameNetwork;
GSM_Error          CurrentSetOperatorNameError;

GSM_Error          CurrentGetIMEIError;

GSM_Error          CurrentGetHWError;

unsigned char      CurrentPPS[4];
GSM_Error          CurrentProductProfileSettingsError;

char               CurrentIncomingCall[20];

GSM_Error          CurrentBinRingtoneError;
GSM_BinRingtone    *CurrentGetBinRingtone=NULL;

GSM_Error          CurrentRingtoneError;

GSM_Error          CurrentMagicError;

GSM_Error          CurrentSimlockInfoError;
GSM_AllSimlocks    *CurrentSimLock;

GSM_Error          CurrentGetWAPBookmarkError;
GSM_Error          CurrentSetWAPBookmarkError;
GSM_WAPBookmark    *WAPBookmark;

GSM_Error          CurrentGetWAPSettingsError;
GSM_WAPSettings    *WAPSettings;

GSM_Error          CurrentCallDivertError;
GSM_CallDivert    *CurrentCallDivert;

char              *CurrentManufacturer;

/* This is the connection type used in gnokii. */
GSM_ConnectionType CurrentConnectionType;

/* Pointer to a callback function used to return changes to a calls status */
/* This saves unreliable polling */
void (*CurrentCallPassup)(char c);

/* Pointer to callback function in user code to be called when RLP frames
   are received. */
void (*CurrentRLP_RXCallback)(RLP_F96Frame *frame);

/* Used to disconnect the call */
u8 CurrentCallSequenceNumber;

bool CurrentLinkOK;

bool CurrentRequestTerminate;

bool CurrentDisableKeepAlive;

bool CheckModel (GSM_Information InfoToCheck, char *model, GSM_ConnectionType connection) {

  bool found_match=false;

  if (strstr(InfoToCheck.FBUSModels, model) != NULL) {
    if (connection==GCT_FBUS) found_match=true;
  }
  if (strstr(InfoToCheck.MBUSModels, model) != NULL) {
    if (connection==GCT_MBUS) found_match=true;
  }
  if (strstr(InfoToCheck.InfraredModels, model) != NULL) {
    if (connection==GCT_Infrared) found_match=true;
  }
  if (strstr(InfoToCheck.DLR3Models, model) != NULL) {
    if (connection==GCT_DLR3) found_match=true;
  }
  if (strstr(InfoToCheck.IrdaModels, model) != NULL) {
    if (connection==GCT_Irda) found_match=true;
  }
  if (strstr(InfoToCheck.ATModels, model) != NULL) {
    if (connection==GCT_AT) found_match=true;
  }
  if (strstr(InfoToCheck.TekramModels, model) != NULL) {
    if (connection==GCT_Tekram) found_match=true;
  }
  if (strstr(InfoToCheck.FBUS3110Models, model) != NULL) {
    if (connection==GCT_FBUS3110) found_match=true;
  }

  return found_match;
}
 
GSM_Error TryNewNokia(char *model, char *device, char *initlength, GSM_ConnectionType connection, void (*rlp_callback)(RLP_F96Frame *frame)) {
  int InitLength;
  int count;
  unsigned char init_char = N6110_SYNC_BYTE;

  /* Hopefully is 64 larger as FB38_MAX* / N6110_MAX* */
  char phonemodel[64];

  if (Protocol->Initialise(device,initlength,connection,rlp_callback)!=GE_NONE)
  {
    return GE_NOTSUPPORTED; 
  }

  if (connection!=GCT_MBUS) {
    InitLength = atoi(initlength);

    if ((strcmp(initlength, "default") == 0) || (InitLength == 0)) {
      InitLength = 250;	/* This is the usual value, lower may work. */
    }

#ifdef DEBUG
    fprintf(stdout,_("Writing init chars...."));
#endif

    /* Initialise link with phone or what have you */
    /* Send init string to phone, this is a bunch of 0x55 characters. Timing is
       empirical. */
    for (count = 0; count < InitLength; count ++) {
      usleep(100);
      Protocol->WritePhone(1,&init_char);
    }

#ifdef DEBUG
    fprintf(stdout,_("Done\n"));  
#endif

    N6110_SendStatusRequest();
  }

  usleep(100);

  if (N6110_SendIDFrame()!=GE_NONE)
    return GE_TIMEOUT;
 
  while (N6110_GetModel(phonemodel) != GE_NONE)
    sleep(1);

  if (!strcmp(phonemodel,"NPE-3") || !strcmp(phonemodel,"NSE-5") || 
      !strcmp(phonemodel,"NHM-3"))
  {
    GSM->Terminate();      
    
    /* Set pointers to relevant addresses */
    GSM = &N7110_Functions;
    GSM_Info = &N7110_Information;
    GSM_LinkOK = &CurrentLinkOK;
    return GE_NONE;
  }

  return GE_NONE;
}

GSM_Error GSM_Initialise(char *model, char *device, char *initlength, GSM_ConnectionType connection, void (*rlp_callback)(RLP_F96Frame *frame), char* SynchronizeTime)
{
  bool found_match=false;
  
  GSM_ConnectionType connection2;

  struct tm *now;
  time_t nowh;
  GSM_DateTime Date;
  GSM_Error error;
  
  connection2=connection;

  CurrentRLP_RXCallback = rlp_callback;
  CurrentCallPassup=NULL;
  CurrentCallDivert=NULL;
  CurrentPhonebookEntry=NULL;
  CurrentNetworkInfo = NULL;
  CurrentGetBitmap=NULL;
  CurrentPlayToneError=GE_UNKNOWN;
  strcpy(CurrentIncomingCall," ");
  CurrentGetBinRingtone=NULL;
  CurrentNetworkInfo=NULL;
  CurrentRequestTerminate=false;
  CurrentDisableKeepAlive=false;
  CurrentCalendarNotesInfo.HowMany=2000;
  CurrentSMSMessage=NULL;
  CurrentMagicError = GE_BUSY;  
  
  if (!strcmp(model,"auto")) {

    /* For now */
    GSM = &N6110_Functions;
    GSM_Info = &N6110_Information;
    GSM_LinkOK = &CurrentLinkOK;
#ifdef DEBUG
    fprintf(stdout,_("Trying FBUS for new Nokia phones...\n"));
#endif
    /* Trying FBUS */
    Protocol = &FBUS_Functions;
    CurrentConnectionType=GCT_FBUS;    
    connection2=GCT_FBUS;
    if (TryNewNokia(model,device,initlength,CurrentConnectionType,rlp_callback)==GE_NONE)
    {
      found_match=true;
    } else {
      GSM->Terminate();      
    }

    if (!found_match) {
      usleep(100);          
      
      /* For now */
      GSM = &N6110_Functions;
      GSM_Info = &N6110_Information;
      GSM_LinkOK = &CurrentLinkOK;
#ifdef DEBUG
      fprintf(stdout,_("Trying DLR3 for new Nokia phones...\n"));
#endif
      /* Trying DLR3 */
      Protocol = &FBUS_Functions;
      CurrentConnectionType=GCT_DLR3;    
      connection2=GCT_DLR3;
      if (TryNewNokia(model,device,initlength,CurrentConnectionType,rlp_callback)==GE_NONE)
      {
        found_match=true;
      } else {
        GSM->Terminate();      
      }
    }
    
    if (!found_match) {
      usleep(100);          
      
      /* For now */
      GSM = &N6110_Functions;
      GSM_Info = &N6110_Information;
      GSM_LinkOK = &CurrentLinkOK;
#ifdef DEBUG
      fprintf(stdout,_("Trying MBUS for new Nokia phones...\n"));
#endif
      /* Trying MBUS */
      Protocol = &MBUS_Functions;
      CurrentConnectionType=GCT_MBUS;    
      connection2=GCT_MBUS;
      if (TryNewNokia(model,device,initlength,CurrentConnectionType,rlp_callback)==GE_NONE)
      {
        found_match=true;
      } else {
        GSM->Terminate();      
      }
    }

    if (!found_match) return GE_NOTSUPPORTED;

    usleep(50);
        
  } else {
    if (!strcmp(model,"modelauto")) {
      /* For now */
      GSM = &N6110_Functions;
      GSM_Info = &N6110_Information;
      GSM_LinkOK = &CurrentLinkOK;
#ifdef DEBUG
      fprintf(stdout,_("Trying to find connected model...\n"));
#endif
      switch (connection) {
        case GCT_FBUS    : Protocol = &FBUS_Functions;    break;
        case GCT_Infrared: Protocol = &FBUS_Functions;    break;
        case GCT_Tekram  : Protocol = &FBUS_Functions;    break;
        case GCT_DLR3    : Protocol = &FBUS_Functions;    break;
        case GCT_MBUS    : Protocol = &MBUS_Functions;    break;
        case GCT_Irda    : Protocol = &FBUSIRDA_Functions;break;
        case GCT_AT      : Protocol = &AT_Functions;      break;
        case GCT_FBUS3110: Protocol = &FBUS3110_Functions;break;
      }
      CurrentConnectionType=connection;    
      connection2=connection;
      if (TryNewNokia(model,device,initlength,CurrentConnectionType,rlp_callback)==GE_NONE)
      {
        found_match=true;
      } else {
        GSM->Terminate();      
      }

      if (!found_match) return GE_NOTSUPPORTED;

      usleep(50);

    } else {
#ifdef DEBUG
      if (CheckModel (Nsniff_Information, model, connection)) {
        /* Set pointers to relevant addresses */
        GSM = &Nsniff_Functions;
        GSM_Info = &Nsniff_Information;
        GSM_LinkOK = &CurrentLinkOK;
        found_match=true;
      }
#endif

      if (CheckModel (N3110_Information, model, connection)) {
        /* Set pointers to relevant addresses */
        GSM = &N3110_Functions;
        GSM_Info = &N3110_Information;
        GSM_LinkOK = &CurrentLinkOK;
        found_match=true;
      }
      if (CheckModel (N6110_Information, model, connection)) {
        /* Set pointers to relevant addresses */
        GSM = &N6110_Functions;
        GSM_Info = &N6110_Information;
        GSM_LinkOK = &CurrentLinkOK;
        found_match=true;
      }
      if (CheckModel (N7110_Information, model, connection)) {
        /* Set pointers to relevant addresses */
        GSM = &N7110_Functions;
        GSM_Info = &N7110_Information;
        GSM_LinkOK = &CurrentLinkOK;
        found_match=true;
      }
      if (CheckModel (Nat_Information, model, connection)) {
        /* Set pointers to relevant addresses */
        GSM = &Nat_Functions;
        GSM_Info = &Nat_Information;
        GSM_LinkOK = &CurrentLinkOK;
        found_match=true;
      }

      if (found_match) {
        switch (connection) {
          case GCT_FBUS    : Protocol = &FBUS_Functions;    break;
          case GCT_Infrared: Protocol = &FBUS_Functions;    break;
          case GCT_Tekram  : Protocol = &FBUS_Functions;    break;
          case GCT_DLR3    : Protocol = &FBUS_Functions;    break;
          case GCT_MBUS    : Protocol = &MBUS_Functions;    break;
          case GCT_Irda    : Protocol = &FBUSIRDA_Functions;break;
          case GCT_AT      : Protocol = &AT_Functions;      break;
          case GCT_FBUS3110: Protocol = &FBUS3110_Functions;break;
        }
      } else
        return GE_NOTSUPPORTED;
    }
  }

    
  /* Now call model specific initialisation code. */
  error=(GSM->Initialise(device, initlength, connection2, rlp_callback));

  /* RTH: FIXME: second try for Irda (6210 only?)*/
  if ( error!=GE_NONE && connection == GCT_Irda)
  {
   #ifdef DEBUG
     fprintf(stdout,"Irda connection: second try!\n");
   #endif
   device_close();
   error=(GSM->Initialise(device, initlength, connection2, rlp_callback));
  }

  if (error==GE_NONE && !strcmp(SynchronizeTime,"yes"))
  {
    nowh=time(NULL);
    now=localtime(&nowh);

    Date.Year = now->tm_year;
    Date.Month = now->tm_mon+1;
    Date.Day = now->tm_mday;
    Date.Hour = now->tm_hour;
    Date.Minute = now->tm_min;
    Date.Second = now->tm_sec;

    if (Date.Year<1900)
    {

      /* Well, this thing is copyrighted in U.S. This technique is known as
         Windowing and you can read something about it in LinuxWeekly News:
         http://lwn.net/1999/features/Windowing.phtml. This thing is beeing
         written in Czech republic and Poland where algorithms are not allowed
         to be patented. */

      if (Date.Year>90)
        Date.Year = Date.Year+1900;
      else
        Date.Year = Date.Year+2000;
    }

    /* FIXME: Error checking should be here. */
    GSM->SetDateTime(&Date);
  }

  return error;
}

GSM_Error Unimplemented(void)
{
	return GE_NOTIMPLEMENTED;
}

GSM_Error NotSupported(void)
{
	return GE_NOTSUPPORTED;
}

/* Applications should call N6110_Terminate to shut down the N6110 thread and
   close the serial port. */
void NULL_Terminate(void)
{
  Protocol->Terminate();
}

#ifdef WIN32
/* Here are things made for keeping connecting */
void NULL_KeepAlive()
{
}
#else
/* Here are things made for keeping connecting */
void NULL_KeepAlive()
{
}
#endif

#ifdef DEBUG
void NULL_TX_DisplayMessage(u16 MessageLength, u8 *MessageBuffer)
{
  fprintf(stdout, _("PC: "));

  txhexdump(MessageLength,MessageBuffer);
}
#endif

bool NULL_WritePhone (u16 length, u8 *buffer) {
  if (device_write(buffer,length)!=length) return false;
                                      else return true;
}

GSM_Error NULL_WaitUntil (int time, GSM_Error *value)
{
  int timeout;

  timeout=time;
  
  /* Wait for timeout or other error. */
  while (timeout != 0 && *value == GE_BUSY ) {
          
    if (--timeout == 0)
      return (GE_TIMEOUT);
                    
    usleep (100000);
  }

  return *value;
}

GSM_Error NULL_SendMessageSequence (int time, GSM_Error *value,
                 u16 message_length, u8 message_type, u8 *buffer)
{
  *value=GE_BUSY;
  
  Protocol->SendMessage(message_length, message_type, buffer);  

  return NULL_WaitUntil (time, value);
}

GSM_ConnectionType GetConnectionTypeFromString(char *Connection) {

  GSM_ConnectionType connection=GCT_FBUS;

  if (!strcmp(Connection, "irda"))     connection=GCT_Irda;
  if (!strcmp(Connection, "infrared")) connection=GCT_Infrared;
  if (!strcmp(Connection, "mbus"))     connection=GCT_MBUS;
  if (!strcmp(Connection, "dlr3"))     connection=GCT_DLR3;
  if (!strcmp(Connection, "fbus3110")) connection=GCT_FBUS3110;
  if (!strcmp(Connection, "at"))       connection=GCT_AT;
  if (!strcmp(Connection, "tekram210"))connection=GCT_Tekram;
  
  return connection;
}

bool GetMemoryTypeString(char *memorytext, GSM_MemoryType *type)
{
  int i=0;

  typedef struct {
    GSM_MemoryType type;
    char *name;
  } GSM_MTStrings;

  GSM_MTStrings mystring[] = {
    {GMT_ME,"ME"},
    {GMT_SM,"SM"},
    {GMT_FD,"FD"},
    {GMT_ON,"ON"},
    {GMT_EN,"EN"},
    {GMT_DC,"DC"},
    {GMT_RC,"RC"},  
    {GMT_MC,"MC"},
    {GMT_LD,"LD"},
    {GMT_MT,"MT"},
    {GMT_ME,"undefined"}
  };

  while (strcmp(mystring[i].name,"undefined")) {
    if (*type==mystring[i].type) {
      strcpy(memorytext,mystring[i].name);
      return true;
    }
    i++;
  }
  return false;
}

bool GetMemoryTypeID(char *memorytext, GSM_MemoryType *type)
{
  int i=0;

  typedef struct {
    GSM_MemoryType type;
    char *name;
  } GSM_MTStrings;

  GSM_MTStrings mystring[] = {
    {GMT_ME,"ME"},
    {GMT_SM,"SM"},
    {GMT_FD,"FD"},
    {GMT_ON,"ON"},
    {GMT_EN,"EN"},
    {GMT_DC,"DC"},
    {GMT_RC,"RC"},  
    {GMT_MC,"MC"},
    {GMT_LD,"LD"},
    {GMT_MT,"MT"},
    {GMT_ME,"undefined"}
  };

  while (strcmp(mystring[i].name,"undefined")) {
    if (strcmp(mystring[i].name,memorytext)==0) {
      *type=mystring[i].type;
      return true;
    }
    i++;
  }

  return false;
}

char *GetMygnokiiVersion() {

  static char Buffer[1000]="";

  sprintf(Buffer, "%s",VERSION);

  return Buffer;
}

/*
1.Name,2.CodeName,3.Calendar,4.Netmonitor,5.Caller groups,6.Phonebook,
7.Authentication 8.Datacalls 9.KeysPressing 10.SMSC Default Recipient
11.SpeedDials 12.ScreenSavers 13.DTMF 14.SMS 15.NoPowerFrame 16.StartUpLogo
17.Profiles 18.Ringtones 19.WAP 20.RIngtonesNumber
*/

static OnePhoneModel allmodels[] = {

/*1,    2,       3,      4,       5,        6          7      8        9      10        11      12       13     14      15        16       17       18        19    20 */
{"3210","NSE-8",{      0,F_NETMON,        0,         0,     0,       0,     0,F_SMSCDEF,F_SPEED,       0,     0,F_SMS  ,F_NOPOWER,F_STANIM,F_PROF51,F_RINGBIN,    0, 2}},
{"3210","NSE-9",{      0,F_NETMON,        0,         0,     0,       0,     0,F_SMSCDEF,F_SPEED,       0,     0,F_SMS  ,F_NOPOWER,F_STANIM,F_PROF51,F_RINGBIN,    0, 2}},
{"3310","NHM-5",{F_CAL33,F_NETMON,        0,F_PBK33SIM,     0,       0,     0,F_SMSCDEF,F_SPEED,F_SCRSAV,     0,F_SMS  ,F_NOPOWER,F_STANIM,F_PROF33,F_RING_SM,    0, 7}},
{"3330","NHM-6",{F_CAL33,F_NETMON,        0,F_PBK33INT,     0,       0,F_KEYB,F_SMSCDEF,F_SPEED,F_SCRSAV,     0,F_SMS  ,        0,F_STANIM,F_PROF33,F_RING_SM,F_WAP, 7}},
{"5110","NSE-1",{      0,F_NETMON,        0,         0,F_AUTH,F_DATA61,F_KEYB,F_SMSCDEF,F_SPEED,       0,F_DTMF,F_SMS  ,        0,       0,F_PROF51,        0,    0, 0}},
{"5130","NSK-1",{      0,F_NETMON,        0,         0,F_AUTH,F_DATA61,F_KEYB,F_SMSCDEF,F_SPEED,       0,F_DTMF,F_SMS  ,        0,       0,F_PROF51,        0,    0, 0}},
{"5190","NSB-1",{      0,F_NETMON,        0,         0,F_AUTH,F_DATA61,F_KEYB,F_SMSCDEF,F_SPEED,       0,F_DTMF,F_SMS  ,        0,       0,F_PROF51,        0,    0, 0}},
{"6110","NSE-3",{F_CAL61,F_NETMON,F_CALER61,F_PBK61INT,F_AUTH,F_DATA61,F_KEYB,F_SMSCDEF,F_SPEED,       0,F_DTMF,F_SMS  ,        0,F_STA   ,F_PROF61,F_RINGBIN,    0, 1}},
{"6130","NSK-3",{F_CAL61,F_NETMON,F_CALER61,F_PBK61INT,F_AUTH,F_DATA61,F_KEYB,F_SMSCDEF,F_SPEED,       0,F_DTMF,F_SMS  ,        0,F_STA   ,F_PROF61,F_RINGBIN,    0, 1}},
{"6150","NSM-1",{F_CAL61,F_NETMON,F_CALER61,F_PBK61INT,F_AUTH,F_DATA61,F_KEYB,F_SMSCDEF,F_SPEED,       0,F_DTMF,F_SMS  ,        0,F_STA   ,F_PROF61,F_RINGBIN,    0, 1}},
{"6190","NSB-3",{F_CAL61,F_NETMON,F_CALER61,F_PBK61INT,F_AUTH,F_DATA61,F_KEYB,F_SMSCDEF,F_SPEED,       0,F_DTMF,F_SMS  ,        0,F_STA   ,F_PROF61,F_RINGBIN,    0, 1}},
{"6210","NPE-3",{F_CAL71,F_NETMON,F_CALER61,F_PBK71INT,     0,F_DATA71,     0,F_SMSCDEF,F_SPEED,       0,     0,F_SMS71,        0,F_STA62 ,F_PROF61,F_RINGBIN,F_WAP, 5}},
{"6250","NHM-3",{F_CAL71,F_NETMON,F_CALER61,F_PBK71INT,     0,F_DATA71,     0,F_SMSCDEF,F_SPEED,       0,     0,F_SMS71,        0,F_STA62 ,F_PROF61,F_RINGBIN,F_WAP, 5}},	
{"7110","NSE-5",{F_CAL71,F_NETMON,F_CALER61,F_PBK71INT,     0,F_DATA71,     0,F_SMSCDEF,F_SPEED,       0,     0,F_SMS71,        0,F_STA71 ,F_PROF61,F_RINGBIN,F_WAP, 5}},
{"8210","NSM-3",{      0,F_NETMON,F_CALER61,F_PBK61INT,     0,F_DATA61,F_KEYB,F_SMSCDEF,F_SPEED,       0,F_DTMF,F_SMS  ,        0,F_STA   ,F_PROF61,F_RINGBIN,    0, 1}},
{"8850","NSM-2",{      0,F_NETMON,F_CALER61,F_PBK61INT,     0,F_DATA61,F_KEYB,F_SMSCDEF,F_SPEED,       0,F_DTMF,F_SMS  ,        0,F_STA   ,F_PROF61,F_RINGBIN,    0, 1}},
{"9210","RAE-3",{      0,F_NETMON,F_CALER61,         0,     0,F_DATA61,F_KEYB,F_SMSCDEF,F_SPEED,       0,F_DTMF,F_SMS  ,        0,F_STA   ,F_PROF61,F_RINGBIN,    0, 0}},//quesses only !
{""    ,""     ,{      0,       0,        0,         0,     0,       0,     0,        0,      0,       0,     0,      0,        0,       0,       0,        0,    0, 0}}
};

OnePhoneModel *GetPhoneModelData (const char *num)
{
	register int i = 0;

	while (allmodels[i].number != "") {
		if (strcmp (num, allmodels[i].number) == 0) {
			return (&allmodels[i]);
		}
		i++;
	}

	return (&allmodels[i]);
}

char *GetModelName ()
{
  static char model[64];

  while (GSM->GetModel(model)  != GE_NONE) sleep(1);

  return (GetPhoneModelData(model)->model);
}

int GetModelFeature (featnum_index num)
{
  static char model[64];

  while (GSM->GetModel(model)  != GE_NONE) sleep(1);

  return (GetPhoneModelData(model)->features[num]);
}

int LogAvailable=-1; //-1 not checked earlier, 0 not, 1 yes
char *logfilename;

bool AppendLog(u8 *buffer, int length,bool format)
{
  FILE *file=NULL;
  char buffer2[50001];
  int i;

  struct CFG_Header *cfg_info;
  char *LogFile;	

  if (LogAvailable==-1) {

    LogAvailable=0;

    cfg_info=CFG_FindGnokiirc();
    if (cfg_info==NULL) return false;

    LogFile = CFG_Get(cfg_info, "global", "logfile");
    if (LogFile) {
      LogAvailable=1;
      logfilename=LogFile;

      file=fopen(logfilename, "a+");

      /* We have first entry in this session and too large file */
      if (fread( buffer2, 1, 50000,file )==50000) {
        fclose(file);
        file=fopen(logfilename, "w");
      }
    }
  } else {
    if (LogAvailable==1) {
      file=fopen(logfilename, "a");
    }
  }
  
  if (LogAvailable==1) {
    for (i=0;i<length;i++) {
      if (format) {
        fprintf(file, "%02x",buffer[i]);
        switch (buffer[i]) {
          case 0x09:fprintf(file,_(" |"));break;
          default:
            if (isprint(buffer[i])) fprintf(file, _("%c|"),buffer[i]);
                               else fprintf(file, _(" |"));
            break;
        } 
      } else {
        fprintf(file, "%c",buffer[i]);
      }
    }
    if (format) fprintf(file, "\n");
    fclose(file);
  }

  return (LogAvailable==1);
}

bool AppendLogText(u8 *buffer,bool format)
{
  return AppendLog(buffer,strlen(buffer),format);
}
