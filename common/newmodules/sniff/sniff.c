/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides an API for accessing functions on the sniff and similar
  phones.

*/

/* "Turn on" prototypes in n-sniff.h */

#define __n_sniff_c 

/* System header files */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
#ifndef WIN32

  #include "devices/device.h"

#endif

/* Various header file */
#ifndef VC6
  #include "config.h"
#endif
#include "misc.h"
#include "gsm-api.h"
#include "gsm-common.h"
#include "files/cfgreader.h"
#include "newmodules/sniff/sniff.h"
#include "newmodules/n6110.h"
#include "newmodules/n7110.h"
#include "gsm-networks.h"

/* Global variables used by code in gsm-api.c to expose the functions
   supported by this model of phone. */


static char *realmodel;       /* Model from .gnokiirc file. */
static char *Port;            /* Port from .gnokiirc file */
static char *Initlength;      /* Init length from .gnokiirc file */
static char *Connection;      /* Connection type from .gnokiirc file */
static char *SynchronizeTime; /* If we set date and time from computer to phone (from .gnokiirc file) */
static char *BinDir;		      /* Binaries directory from .gnokiirc file - not used here yet */
static GSM_ConnectionType connectiontype;







/* Here we initialise model specific functions. */

GSM_Functions Nsniff_Functions = {
  Nsniff_Initialise,
  Nsniff_DispatchMessage,
  NULL_Terminate,
  NULL_KeepAlive,
  NOTSUPPORTED,
  NOTSUPPORTED,  
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED,
  NOTSUPPORTED
};



/* for all these Phones we support sniff */

GSM_Information Nsniff_Information = {
  "3210sniff|3310sniff|3330sniff|5110sniff|5130sniff|5190sniff|6110sniff|6130sniff|6150sniff|6190sniff|8210sniff|8850sniff|6210sniff|6250sniff|7110sniff",
     /* Supported models in FBUS */
  "3210sniff|3310sniff|3330sniff|5110sniff|5130sniff|5190sniff|6110sniff|6130sniff|6150sniff|6190sniff|8210sniff|8850sniff|6210sniff|6250sniff|7110sniff",
     /* Supported models in MBUS */
  "",
     /* Supported models in FBUS over infrared */
  "",
     /* Supported models in FBUS over DLR3 */
  "",
  "6210decode|6250decode|7110decode", /* Supported models in FBUS over Irda sockets */
  "",
  "",
  4,                     /* Max RF Level */
  0,                     /* Min RF Level */
  GRF_Arbitrary,         /* RF level units */
  4,                     /* Max Battery Level */
  0,                     /* Min Battery Level */
  GBU_Arbitrary,         /* Battery level units */
  GDT_None,              /* No date/time support */
  GDT_None,	         /* No alarm support */
  0                      /* Max alarms = 0 */
};

/* Initialise variables and state machine. */
GSM_Error Nsniff_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame))
{

  GSM_PhonebookEntry phonebook;
  GSM_SpeedDial speed;
  GSM_SMSMessage sms;
  GSM_MemoryStatus status;
  GSM_NetworkInfo netinfo;
  GSM_SMSStatus smsstatus;
  GSM_SMSFolders folders;
  GSM_MessageCenter smsc;
  int secstatus;
  GSM_SecurityCode seccode;
  GSM_DateTime date;
  GSM_DateTime alarm;
  GSM_CalendarNote note;
  char netmon;
  GSM_Bitmap logo;
  GSM_Profile profile;
  GSM_CBMessage cb;
  GSM_Network netname;
  GSM_BinRingtone binring;
  GSM_AllSimlocks siml;    if (Protocol->Initialise(port_device,initlength,connection,rlp_callback)!=GE_NONE)
  {
    return GE_NOTSUPPORTED;
  }

  CurrentLinkOK = true;                           

  CurrentPhonebookEntry=&phonebook;
  CurrentSpeedDialEntry=&speed;
  CurrentSMSMessage=&sms;
  CurrentMemoryStatus=&status;
  CurrentNetworkInfo=&netinfo;
  CurrentSMSStatus=&smsstatus;
  CurrentSMSFolders=&folders;
  CurrentMessageCenter=&smsc;
  CurrentSecurityCodeStatus=&secstatus;
  CurrentSecurityCode=&seccode;
  CurrentDateTime=&date;
  CurrentAlarm=&alarm;
  CurrentCalendarNote=&note;
  CurrentNetmonitor=&netmon;
  CurrentGetBitmap=&logo;
  CurrentProfile=&profile;
  CurrentCBMessage=&cb;
  CurrentGetOperatorNameNetwork=&netname;
  CurrentGetBinRingtone=&binring;
  CurrentSimLock=&siml;  

  /* Read config file, once again to get real model */
    if (CFG_ReadConfig(&realmodel, &Port, &Initlength, &Connection, &BinDir, &SynchronizeTime,false) < 0) {
        exit(-1);
    }

  connectiontype = GetConnectionTypeFromString(Connection);
  return (GE_NONE);
}


void Nsniff_DispatchMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  
/* switch to real Model */
  if (CheckModel (N6110_Information, realmodel, connectiontype)) 
	N6110_DispatchMessage( MessageLength, MessageBuffer, MessageType);

  if (CheckModel (N7110_Information, realmodel, connectiontype)) 
	N7110_DispatchMessage( MessageLength, MessageBuffer, MessageType);
}
