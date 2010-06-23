/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides an API for accessing functions on the 7110 and similar
  phones.

*/

/* "Turn on" prototypes in n-7110.h */

#define __n_7110_c 

/* System header files */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
 
#ifndef WIN32
  #include "devices/device.h"
#endif

#ifdef WIN32
  #include "misc_win32.h"
#endif

/* Various header file */
#ifndef VC6
  #include "config.h"
#endif

#include "gsm-api.h"
#include "gsm-coding.h"
#include "newmodules/n6110.h"
#include "newmodules/n7110.h"
#include "protocol/fbus.h"

/* Global variables used by code in gsm-api.c to expose the functions
   supported by this model of phone. */

/* Here we initialise model specific functions. */

GSM_Functions N7110_Functions = {
  N7110_Initialise,
  N7110_DispatchMessage,
  NULL_Terminate,
  NULL_KeepAlive,
  N7110_GetMemoryLocation,
  N7110_WritePhonebookLocation,
  N7110_GetSpeedDial,
  N7110_SetSpeedDial,
  N7110_GetMemoryStatus,
  N7110_GetSMSStatus,
  N6110_GetSMSCenter,
  N6110_SetSMSCenter,
  N7110_GetSMSMessage,
  N7110_DeleteSMSMessage,
  N6110_SendSMSMessage,
  N7110_SaveSMSMessage,
  N7110_GetRFLevel,
  N7110_GetBatteryLevel,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  N7110_GetSecurityCode,
  N6110_GetIMEI,
  N6110_GetRevision,
  N6110_GetModel,
  N7110_GetDateTime,
  N7110_SetDateTime,
  N7110_GetAlarm,
  N7110_SetAlarm,
  N6110_DialVoice,
  N6110_DialData,
  N6110_GetIncomingCallNr,
  N6110_GetNetworkInfo,
  N7110_GetCalendarNote,
  N7110_WriteCalendarNote,
  N7110_DeleteCalendarNote,
  N6110_NetMonitor,
  UNIMPLEMENTED,
  N7110_GetBitmap,
  N7110_SetBitmap,
  N7110_SetRingTone,
  N7110_SetBinRingTone,
  N7110_GetBinRingTone,
  N6110_Reset,
  N7110_GetProfile,
  UNIMPLEMENTED,
  N6110_SendRLPFrame,
  N6110_CancelCall,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  N6110_EnableCellBroadcast,
  N6110_DisableCellBroadcast,
  N6110_ReadCellBroadcast,
  N6110_PlayTone,
  N6110_GetProductProfileSetting,
  N6110_SetProductProfileSetting,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  N7110_GetVoiceMailbox,
  N6110_Tests,
  N6110_SimlockInfo,
  N7110_GetCalendarNotesInfo,
  N7110_GetSMSFolders,
  UNIMPLEMENTED,
  N7110_GetWAPBookmark,
  N7110_SetWAPBookmark,
  N7110_GetWAPSettings,
  N6110_CallDivert,
  UNIMPLEMENTED,
  N6110_GetManufacturer
};

/* Mobile phone information */

GSM_Information N7110_Information = {
  "6210|6250|7110", /* Supported models in FBUS */
  "6210|6250|7110", /* Supported models in MBUS */
  "",               /* Supported models in FBUS over infrared */
  "6210|6250|7110", /* Supported models in FBUS over DLR3 */
  "",
  "6210|6250|7110", /* Supported models in FBUS over Irda sockets */
  "",
  "",
  5,                     /* Max RF Level */
  0,                     /* Min RF Level */
  GRF_Arbitrary,         /* RF level units */
  5,                     /* Max Battery Level */
  0,                     /* Min Battery Level */
  GBU_Arbitrary,         /* Battery level units */
  GDT_DateTime,          /* Have date/time support */
  GDT_TimeOnly,	         /* Alarm supports time only */
  1                      /* Only one alarm available */
};

const char *N7110_MemoryType_String [] = {
  "", 	/* 0x00 */
  "DC", /* 0x01 */
  "MC", /* 0x02 */
  "RC", /* 0x03 */
  "FD", /* 0x04 */
  "ME", /* 0x05 */
  "SM", /* 0x06 */
  "ON", /* 0x07 */
  "EN", /* 0x08 */
  "MT", /* 0x09 */
};

int PictureImageNum; //required during reading Picture Images
int PictureImageIndex;
int PictureImageLoc;

void N7110_ReplyEnableIncomingSMSInfo(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
  switch (MessageBuffer[3]) {
  case 0x0e:
#ifdef DEBUG
    fprintf(stdout,_("Message: Enabled info for incoming SMS\n"));
#endif /* DEBUG */

    CurrentMagicError=GE_NONE;
    break;
  case 0x0f:
#ifdef DEBUG
    fprintf(stdout,_("Message: error enabling for incoming SMS\n"));
    switch (MessageBuffer[4]) {
      case 0x0c:fprintf(stdout,_("   No PIN\n"));break;
      default  :fprintf(stdout,_("   unknown\n"));break;
    }
#endif /* DEBUG */

    CurrentMagicError=GE_UNKNOWN;
    break;
  }
}

GSM_Error N7110_EnableIncomingSMSInfo ()
{
  unsigned char req[] = {N6110_FRAME_HEADER, 0x0d, 0x00, 0x00, 0x02};

  return NULL_SendMessageSequence
    (50, &CurrentMagicError, 8, 0x02, req);
}

/* Initialise variables and state machine. */
GSM_Error N7110_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame))
{

  unsigned char init_char = N6110_SYNC_BYTE;
  int count;
  int InitLength;
    
  if (Protocol->Initialise(port_device,initlength,connection,rlp_callback)!=GE_NONE)
  {
    return GE_NOTSUPPORTED;
  }
  
  if (connection!=GCT_MBUS && connection!=GCT_Irda) {

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

  if (N6110_SendIDFrame()!=GE_NONE)   return GE_TIMEOUT;

  if (N6110_SendIMEIFrame()!=GE_NONE) return GE_TIMEOUT;    

  if (N6110_SendHWFrame()!=GE_NONE)   return GE_TIMEOUT;    

//  N7110_EnableIncomingSMSInfo();
    
  if (connection==GCT_MBUS || connection==GCT_Irda) {
    /* In MBUS doesn't have any init strings, etc. Phone answered with frame,
       so connection should be enabled ;-) */
    /* Similiar for Irda */
    CurrentLinkOK = true;                           
  }
  
  CurrentSMSFoldersCount=1000;

  return (GE_NONE);
}

/* This function translates GMT_MemoryType to N7110_MEMORY_xx */
int N7110_GetMemoryType(GSM_MemoryType memory_type)
{

  int result;

  switch (memory_type) {

//     case GMT_MT:
//	result = N7110_MEMORY_MT;
//        break;

     case GMT_ME: result = N7110_MEMORY_ME; break;
     case GMT_SM: result = N7110_MEMORY_SM; break;
     case GMT_FD: result = N7110_MEMORY_FD; break;
     case GMT_ON: result = N7110_MEMORY_ON; break;
     case GMT_EN: result = N7110_MEMORY_EN; break;
     case GMT_DC: result = N7110_MEMORY_DC; break;
     case GMT_RC: result = N7110_MEMORY_RC; break;
     case GMT_MC: result = N7110_MEMORY_MC; break;
     case GMT_CG: result = N7110_MEMORY_CG; break;
     default    : result = N6110_MEMORY_XX;
   }

   return (result);
}

GSM_Error N7110_GetVoiceMailbox ( GSM_PhonebookEntry *entry)
{
  unsigned char req[] = {N7110_FRAME_HEADER, 0x07, 0x01, 0x01, 0x00, 0x01,
                         0x02, //memory type
                         0x05,
                         0x00, 0x00, //location
                         0x00, 0x00};

  CurrentPhonebookEntry = entry;

  req[9] = N7110_MEMORY_VM;
  req[10] = (1>>8);
  req[11] = 1 & 0xff;

  return NULL_SendMessageSequence
    (50, &CurrentPhonebookError, 14, 0x03, req);
}

void N7110_ReplyEnableWAPCommands(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  switch(MessageBuffer[3]) {

  case 0x01:
#ifdef DEBUG
    fprintf(stdout, _("Message: WAP functions enabled\n"));
#endif /* DEBUG */
    CurrentGetWAPBookmarkError=GE_NONE;
    break;
    
  case 0x02:
#ifdef DEBUG
    fprintf(stdout, _("Message: WAP functions enabled\n"));
#endif /* DEBUG */
    CurrentGetWAPBookmarkError=GE_NONE;
    break;

  }
}

/* To enable WAP frames in phone */
GSM_Error N7110_EnableWAPCommands ()
{
  unsigned char req0[] = { N6110_FRAME_HEADER, 0x00 };

  return NULL_SendMessageSequence
    (50, &CurrentGetWAPBookmarkError, 4, 0x3f, req0);
}

void N7110_ReplyGetWAPBookmark(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  int tmp;

  switch(MessageBuffer[3]) {

  case 0x07:

#ifdef DEBUG
    fprintf(stdout, _("Message: WAP bookmark received OK\n"));
#endif /* DEBUG */

    DecodeUnicode(WAPBookmark->title, MessageBuffer + 7, MessageBuffer[6] );

#ifdef DEBUG
    fprintf(stdout, _("   Title: \"%s\"\n"),WAPBookmark->title);      
#endif /* DEBUG */

    tmp=MessageBuffer[6]*2+7;

    DecodeUnicode(WAPBookmark->address, MessageBuffer + tmp+1, MessageBuffer[tmp] );

#ifdef DEBUG
    fprintf(stdout, _("   Address: \"%s\"\n"),WAPBookmark->address);      
#endif /* DEBUG */

    CurrentGetWAPBookmarkError=GE_NONE;
    break;

  case 0x08:

#ifdef DEBUG
    fprintf(stdout, _("Message: WAP bookmark receiving error\n"));
    switch (MessageBuffer[4]) {
      case 0x01:fprintf(stdout, _("   Inside Bookmarks menu. Must leave it\n"));break;
      case 0x02:fprintf(stdout, _("   Invalid or empty location\n"));break;
      default  :fprintf(stdout, _("   Unknown error. Please report it\n"));
    }
#endif /* DEBUG */

    switch (MessageBuffer[4]) {
      case 0x01:CurrentGetWAPBookmarkError=GE_INSIDEBOOKMARKSMENU;break;
      case 0x02:CurrentGetWAPBookmarkError=GE_INVALIDBOOKMARKLOCATION;break;
      default:CurrentGetWAPBookmarkError=GE_UNKNOWN;
    }

    break;
  }
}

GSM_Error N7110_GetWAPBookmark (GSM_WAPBookmark *bookmark)
{
  unsigned char req[] = { N6110_FRAME_HEADER, 0x06, 0x00, 0x00};

  GSM_Error error;

  /* We have to enable WAP frames in phone */
  error=N7110_EnableWAPCommands ();
  if (error!=GE_NONE) return error;

  req[2]=0x00;
  req[5]=bookmark->location-1;

  WAPBookmark=bookmark;
  
  return NULL_SendMessageSequence
    (50, &CurrentGetWAPBookmarkError, 6, 0x3f, req);
}

void N7110_ReplySetWAPBookmark(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  switch(MessageBuffer[3]) {

  case 0x0a:

#ifdef DEBUG
    fprintf(stdout, _("Message: WAP bookmark set OK\n"));
#endif /* DEBUG */

    CurrentSetWAPBookmarkError=GE_NONE;
    break;
    
  case 0x0b:

#ifdef DEBUG
    fprintf(stdout, _("Message: Error setting WAP bookmark\n"));
#endif /* DEBUG */

    CurrentSetWAPBookmarkError=GE_UNKNOWN;
    break;
  }
}

GSM_Error N7110_SetWAPBookmark (GSM_WAPBookmark *bookmark)
{
	unsigned char req[64] = { N6110_FRAME_HEADER, 0x09 };
	GSM_Error error;
	int count;

	/* We have to enable WAP frames in phone */
	error = N7110_EnableWAPCommands ();
	if (error != GE_NONE)
		return error;

	count = 4;
	req[count++] = (bookmark->location & 0xff00) >> 8;
	req[count++] = (bookmark->location & 0x00ff);

	req[count++] = strlen(bookmark->title);
	EncodeUnicode (req+count,bookmark->title ,strlen(bookmark->title));
	count=count+2*strlen(bookmark->title);

	req[count++] = strlen(bookmark->address);
	EncodeUnicode (req+count,bookmark->address ,strlen(bookmark->address));
	count=count+2*strlen(bookmark->address);

	/* ??? */
	req[count++] = 0x01; req[count++] = 0x80; req[count++] = 0x00;
	req[count++] = 0x00; req[count++] = 0x00; req[count++] = 0x00;
	req[count++] = 0x00; req[count++] = 0x00; req[count++] = 0x00;

	WAPBookmark = bookmark;

	return NULL_SendMessageSequence(50, &CurrentSetWAPBookmarkError,
					count, 0x3f, req);
}

void N7110_ReplyGetWAPSettings(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  int tmp;

  switch(MessageBuffer[3]) {

  case 0x16:

#ifdef DEBUG
    fprintf(stdout, _("Message: WAP settings received OK\n"));
#endif /* DEBUG */

    DecodeUnicode(WAPSettings->title, MessageBuffer + 5, MessageBuffer[4] );

#ifdef DEBUG
    fprintf(stdout, _("   Title: \"%s\"\n"),WAPSettings->title);      
#endif /* DEBUG */

    tmp=5+MessageBuffer[4]*2;

    DecodeUnicode(WAPSettings->homepage, MessageBuffer + tmp+1, MessageBuffer[tmp] );

#ifdef DEBUG
    fprintf(stdout, _("   Homepage: \"%s\"\n"),WAPSettings->homepage);      
#endif /* DEBUG */

    tmp=tmp+MessageBuffer[tmp]*2+1;

#ifdef DEBUG
    fprintf(stdout, _("   Connection type: "));      
    switch (MessageBuffer[tmp]) {
      case 0x00: fprintf(stdout,_("temporary"));break;
      case 0x01: fprintf(stdout,_("continuous"));break;
      default:   fprintf(stdout,_("unknown"));
    }
    fprintf(stdout, _("\n"));      

    fprintf(stdout, _("   Connection security: "));      
    switch (MessageBuffer[tmp+13]) {
      case 0x00: fprintf(stdout,_("off"));break;
      case 0x01: fprintf(stdout,_("on"));break;
      default:   fprintf(stdout,_("unknown"));
    }
    fprintf(stdout, _("\n"));      

#endif /* DEBUG */

    switch (MessageBuffer[tmp]) {
      case 0x00: WAPSettings->iscontinuous=false;break;
      case 0x01: WAPSettings->iscontinuous=true;break;
    }
    switch (MessageBuffer[tmp+13]) {
      case 0x00: WAPSettings->issecurity=false;break;
      case 0x01: WAPSettings->issecurity=true;break;
    }

    WAPSettings->location=MessageBuffer[tmp+7];//location for second part

    CurrentGetWAPSettingsError=GE_NONE;
    break;

  case 0x17:

#ifdef DEBUG
    fprintf(stdout, _("Message: WAP settings receiving error\n"));
    switch (MessageBuffer[4]) {
      case 0x01:fprintf(stdout, _("   Inside Settings menu. Must leave it\n"));break;
      case 0x02:fprintf(stdout, _("   Invalid or empty location\n"));break;
      default  :fprintf(stdout, _("   Unknown error. Please report it\n"));
    }
#endif /* DEBUG */

    switch (MessageBuffer[4]) {
      case 0x01:CurrentGetWAPSettingsError=GE_INSIDESETTINGSMENU;break;
      case 0x02:CurrentGetWAPSettingsError=GE_INVALIDSETTINGSLOCATION;break;
      default  :CurrentGetWAPSettingsError=GE_UNKNOWN;
    }

    break;

  case 0x1c:

#ifdef DEBUG
    fprintf(stdout, _("Message: WAP settings received OK\n"));
#endif /* DEBUG */

    switch (MessageBuffer[5]) {
      case 0x00:

        WAPSettings->bearer=WAPSETTINGS_BEARER_SMS;
	
#ifdef DEBUG
        fprintf(stdout, _("  Settings for SMS bearer:\n"));
#endif /* DEBUG */

        tmp=6;

        DecodeUnicode(WAPSettings->service, MessageBuffer + tmp+1, 
                       MessageBuffer[tmp] );

#ifdef DEBUG
        fprintf(stdout, _("      Service number: \"%s\"\n"),WAPSettings->service);      
#endif /* DEBUG */

        tmp=tmp+MessageBuffer[tmp]*2+1;

        DecodeUnicode(WAPSettings->server, MessageBuffer + tmp+ 1, 
                       MessageBuffer[tmp] );

#ifdef DEBUG
        fprintf(stdout, _("      Server number: \"%s\"\n"),WAPSettings->server);      
#endif /* DEBUG */

        break;

      case 0x01:

        WAPSettings->bearer=WAPSETTINGS_BEARER_DATA;
	
#ifdef DEBUG
        fprintf(stdout, _("  Settings for data bearer:\n"));
#endif /* DEBUG */

        tmp=10;

        DecodeUnicode(WAPSettings->ipaddress, MessageBuffer + tmp+ 1, 
                       MessageBuffer[tmp] );

#ifdef DEBUG
        fprintf(stdout, _("      IP address: \"%s\"\n"),WAPSettings->ipaddress);      
#endif /* DEBUG */

        tmp=tmp+MessageBuffer[tmp]*2+1;

        DecodeUnicode(WAPSettings->dialup, MessageBuffer + tmp+1, 
                       MessageBuffer[tmp] );

#ifdef DEBUG
        fprintf(stdout, _("      Dial-up number: \"%s\"\n"),WAPSettings->dialup);      
#endif /* DEBUG */

        tmp=tmp+MessageBuffer[tmp]*2+1;

        DecodeUnicode(WAPSettings->user, MessageBuffer + tmp+1, 
                       MessageBuffer[tmp] );

#ifdef DEBUG
        fprintf(stdout, _("      User name: \"%s\"\n"),WAPSettings->user);      
#endif /* DEBUG */

        tmp=tmp+MessageBuffer[tmp]*2+1;

        DecodeUnicode(WAPSettings->password, MessageBuffer + tmp+1, 
                       MessageBuffer[tmp] );

#ifdef DEBUG
        fprintf(stdout, _("      Password: \"%s\"\n"),WAPSettings->password);      
#endif /* DEBUG */

#ifdef DEBUG
        fprintf(stdout, _("      Authentication type: "));      
        switch (MessageBuffer[6]) {
          case 0x00: fprintf(stdout,_("normal"));break;
          case 0x01: fprintf(stdout,_("secure"));break;
          default:   fprintf(stdout,_("unknown"));break;
        }
        fprintf(stdout, _("\n"));      

        fprintf(stdout, _("      Data call type: "));      
        switch (MessageBuffer[7]) {
          case 0x00: fprintf(stdout,_("analogue"));break;
          case 0x01: fprintf(stdout,_("IDSN"));break;
          default:   fprintf(stdout,_("unknown"));break;
        }
        fprintf(stdout, _("\n"));      

        fprintf(stdout, _("      Data call speed: "));      
        switch (MessageBuffer[9]) {
          case 0x01: fprintf(stdout,_("9600"));break;
          case 0x02: fprintf(stdout,_("14400"));break;
          default:   fprintf(stdout,_("unknown"));break;
        }
        fprintf(stdout, _("\n"));      

#endif /* DEBUG */                             

        switch (MessageBuffer[6]) {
          case 0x00: WAPSettings->isnormalauthentication=true;break;
          case 0x01: WAPSettings->isnormalauthentication=false;break;
        }
        switch (MessageBuffer[7]) {
          case 0x00: WAPSettings->isISDNcall=false;break;
          case 0x01: WAPSettings->isISDNcall=true;break;
        }
        switch (MessageBuffer[9]) {
          case 0x01: WAPSettings->isspeed14400=false;break;
          case 0x02: WAPSettings->isspeed14400=true;break;
        }

        break;
	
      case 0x02:

        WAPSettings->bearer=WAPSETTINGS_BEARER_USSD;
	
#ifdef DEBUG
        fprintf(stdout, _("  Settings for USSD bearer:\n"));
#endif /* DEBUG */

        tmp=7;

        DecodeUnicode(WAPSettings->service, MessageBuffer + tmp+1, 
                       MessageBuffer[tmp] );

#ifdef DEBUG
        if (MessageBuffer[6]==0x01) 
          fprintf(stdout, _("      Service number: \"%s\"\n"),WAPSettings->service);      
	else 
          fprintf(stdout, _("      IP address: \"%s\"\n"),WAPSettings->service);      
#endif /* DEBUG */

        WAPSettings->isIP=true;
        if (MessageBuffer[6]==0x01) WAPSettings->isIP=false;
	
        tmp=tmp+MessageBuffer[tmp]*2+1;

        DecodeUnicode(WAPSettings->code, MessageBuffer + tmp+1, 
                       MessageBuffer[tmp] );

#ifdef DEBUG
        fprintf(stdout, _("      Service code: \"%s\"\n"),WAPSettings->code);      
#endif /* DEBUG */

    }

    CurrentGetWAPSettingsError=GE_NONE;
    break;
  }
}

GSM_Error N7110_GetWAPSettings (GSM_WAPSettings *settings)
{
  unsigned char req[] = { N6110_FRAME_HEADER, 0x15, 0x00};
  unsigned char req2[] = { N6110_FRAME_HEADER, 0x1b, 0x00};

  GSM_Error error;

  /* We have to enable WAP frames in phone */
  error=N7110_EnableWAPCommands ();
  if (error!=GE_NONE) return error;

  req[2]=0x00;
  req[4]=settings->location-1;

  WAPSettings=settings;
  
  error=NULL_SendMessageSequence
    (50, &CurrentGetWAPSettingsError, 6, 0x3f, req);
  if (error!=GE_NONE) return error;

  req2[2]=0x00;

  req2[4]=settings->location;

  return NULL_SendMessageSequence
    (50, &CurrentGetWAPSettingsError, 6, 0x3f, req2);
}

void N7110_ReplyGetMemoryStatus(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
#ifdef DEBUG
  fprintf(stdout, _("Message: Memory status received:\n"));
#endif

  if (CurrentMemoryStatus && CurrentMemoryStatusError == GE_BUSY) {
    /* first Loc. (MessageBuffer[10]<<8) + MessageBuffer[11]; */
    CurrentMemoryStatus->Free = (MessageBuffer[14]<<8) + MessageBuffer[15];
    CurrentMemoryStatus->Used = (MessageBuffer[16]<<8) + MessageBuffer[17];
    CurrentMemoryStatus->Free -= CurrentMemoryStatus->Used;

    CurrentMemoryStatusError = GE_NONE;

#ifdef DEBUG
    fprintf(stdout, _("   Memory Type: %s\n"), N7110_MemoryType_String[MessageBuffer[5]]);
    fprintf(stdout, _("   Used: %d\n"), CurrentMemoryStatus->Used);
    fprintf(stdout, _("   Free: %d\n"), CurrentMemoryStatus->Free);
#endif /* DEBUG */
  }
}

/* This function is used to get storage status from the phone. It currently
   supports two different memory areas - internal and SIM. */
GSM_Error N7110_GetMemoryStatus(GSM_MemoryStatus *Status)
{
  unsigned char req[] = { N6110_FRAME_HEADER,
                          0x03, /* MemoryStatus request */
                          0x02,
                          0x05  /* MemoryType */
                        };

  switch (Status->MemoryType) {

     case GMT_ME:
     case GMT_SM:
       CurrentMemoryStatus = Status;

       req[5] = N7110_GetMemoryType(Status->MemoryType);

       return NULL_SendMessageSequence
         (20, &CurrentMemoryStatusError, 6, 0x03, req);
       break;

     case GMT_DC:
     case GMT_RC:
     case GMT_MC:
       Status->Free = 0;
       Status->Used = 20;
       return GE_NONE;

     default:
       return GE_NOTSUPPORTED;
   }
}

void N7110_ReplyGetProfile(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  switch(MessageBuffer[3]) {

  case 0x02:

#ifdef DEBUG
     fprintf(stdout,_("Profile feature %02x received\n"),MessageBuffer[6]);
#endif /* DEBUG */

     switch (MessageBuffer[6]) {
       case 0xff: /* Profile Name */
         DecodeUnicode (CurrentProfile->Name, MessageBuffer+10, MessageBuffer[9]);      
#ifdef DEBUG
         fprintf(stdout,_("   Name \"%s\"\n"),CurrentProfile->Name);
#endif /* DEBUG */

         break;

       case 0x00: /* Keypad tone (Off, Level 1 ... Level 3) */
         switch( MessageBuffer[10] ) {
           case 0:
             CurrentProfile->KeypadTone = PROFILE_KEYPAD_OFF; break;
           case 1:
           case 2:
           case 3:
             CurrentProfile->KeypadTone = MessageBuffer[10]-1; break;
         }
         break;

       case 0x02: /* Call Alert type (Ringing, Ascending, ..., Off) */
         /* I make it compatible with GetProfileCallAlertString */
         switch( MessageBuffer[10] ) {
           case 0: CurrentProfile->CallAlert = PROFILE_CALLALERT_RINGING; break;
           case 1: CurrentProfile->CallAlert = PROFILE_CALLALERT_ASCENDING; break;
           case 2: CurrentProfile->CallAlert = PROFILE_CALLALERT_RINGONCE; break;
           case 3: CurrentProfile->CallAlert = PROFILE_CALLALERT_BEEPONCE; break;
           case 5: CurrentProfile->CallAlert = PROFILE_CALLALERT_OFF; break;
         }
         break;
       case 0x03: /* Ringtone number */
         CurrentProfile->Ringtone = MessageBuffer[10];
         break;
       case 0x04: /* Ringtone volume (from level 1 to level 5) */
         CurrentProfile->Volume = MessageBuffer[10] + 6;
         break;
       case 0x05: /* MessageTone Type (Off,Standard,...,Ascending) */
         CurrentProfile->MessageTone = MessageBuffer[10];
         break;
       case 0x06: /* Vibration (On/Off) */
         CurrentProfile->Vibration = MessageBuffer[10];
         break;
       case 0x07: /* WarningTone (On/Off) */
         switch( MessageBuffer[10] ) {
           case 0:
             CurrentProfile->WarningTone = PROFILE_WARNING_OFF; break;
           case 1:
             CurrentProfile->WarningTone = PROFILE_WARNING_ON; break;
         }
         break;
       case 0x08: /* Alert for (caller groups) */
         CurrentProfile->CallerGroups = MessageBuffer[10];
         break;
       case 0x09: /* Auto Answer for Handsfree and Headset (On/Off) */
         CurrentProfile->AutomaticAnswer = MessageBuffer[10];
         break;
     }

     CurrentProfileError=GE_NONE;
     break;
   default:

#ifdef DEBUG
    fprintf(stdout, _("Message: Unknown message of type 0x39\n"));
#endif /* DEBUG */
    AppendLogText("Unknown msg\n",false);
    break;	/* Visual C Don't like empty cases */

  }
}

GSM_Error N7110_GetProfile(GSM_Profile *Profile)
{
  int i;
  
  unsigned char req[] = { N6110_FRAME_HEADER, 0x01, 0x01, 0x01, 0x01,
                          0x00,  //profile number
                          0xff}; //feature. Here 0xff=name

  unsigned char req2[11] = {0x03,0x04,0x05,0x06,0x07,0x08,0x0a,0x22,
														0x00,0x02,0x09 };

  GSM_Error error;
  
  CurrentProfile = Profile;

  /* When after sending all frames feature==253, it means, that it is not
     supported */
  CurrentProfile->KeypadTone=253;
  CurrentProfile->Lights=253;    
  CurrentProfile->CallAlert=253; 
  CurrentProfile->Ringtone=253;  
  CurrentProfile->Volume=253;    
  CurrentProfile->MessageTone=253;
  CurrentProfile->WarningTone=253;
  CurrentProfile->Vibration=253;  
  CurrentProfile->CallerGroups=253;
  CurrentProfile->ScreenSaver=253; 
  CurrentProfile->AutomaticAnswer=253;

  req[7] = Profile->Number+1;

  error=NULL_SendMessageSequence
    (20, &CurrentProfileError, 9, 0x39, req);
  if (error!=GE_NONE) return error;

  for (i = 0; i < 11; i++) {

    req[7] = Profile->Number+1;
    
    req[8] = req2[i];

    error=NULL_SendMessageSequence
      (20, &CurrentProfileError, 9, 0x39, req);
    if (error!=GE_NONE) return error;
  }
  
  return (GE_NONE);
}

void N7110_ReplyGetCalendarNotesInfo(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  int i;

  CurrentCalendarNotesInfo.HowMany = MessageBuffer[4]*256+MessageBuffer[5];
  CurrentCalendarNotesInfo2->HowMany = CurrentCalendarNotesInfo.HowMany;

  for(i=0;i<CurrentCalendarNotesInfo.HowMany && i<MAX_NOTES_INFO_ELEMS;i++) {
    CurrentCalendarNotesInfo.Location[i] = MessageBuffer[8+i*2]*256+ MessageBuffer[8+i*2+1];
    CurrentCalendarNotesInfo2->Location[i]=CurrentCalendarNotesInfo.Location[i];
  }

  CurrentCalendarNotesInfoError=GE_NONE;
}

GSM_Error N7110_GetCalendarNotesInfo(GSM_NotesInfo *NotesInfo)
{
  unsigned char req[] = { N6110_FRAME_HEADER,
	0x3a, /* get notes info */
	0xFF, 0xFE //fixed
  };
  GSM_Error error;

  CurrentCalendarNotesInfo2=NotesInfo;
  
  error=NULL_SendMessageSequence(50, &CurrentCalendarNotesInfoError, 6, 0x13, req);

  return error;
}

void P7110_GetNoteAlarm(int alarmdiff, GSM_DateTime *time, GSM_DateTime *alarm, int alarm2)
{
  time_t     t_alarm;
  struct tm  tm_time;
  struct tm  *tm_alarm;

#ifdef DEBUG			
  if (alarmdiff == 0xffff) fprintf(stdout,"   No alarm");
                      else fprintf(stdout,"   Alarm is %i seconds before date", alarmdiff*alarm2);
  fprintf(stdout,"\n");
#endif

  if (alarmdiff != 0xffff) {	

    memset(&tm_time, 0, sizeof(tm_time));
    tm_time.tm_year = time->Year - 1900;
    tm_time.tm_mon = time->Month - 1;
    tm_time.tm_mday = time->Day;
    tm_time.tm_hour = time->Hour;
    tm_time.tm_min = time->Minute;
    tm_time.tm_sec = time->Second;

    tzset();
    t_alarm = mktime(&tm_time);
    t_alarm -= alarmdiff*alarm2;

    tm_alarm = localtime(&t_alarm);

    alarm->Year = tm_alarm->tm_year + 1900;
    alarm->Month = tm_alarm->tm_mon + 1;
    alarm->Day = tm_alarm->tm_mday;
    alarm->Hour = tm_alarm->tm_hour;
    alarm->Minute = tm_alarm->tm_min;
    alarm->Second = tm_alarm->tm_sec;

#ifdef DEBUG
    fprintf(stdout, "   Alarm: %02i-%02i-%04i %02i:%02i:%02i\n",
                 alarm->Day,alarm->Month,alarm->Year,
                 alarm->Hour,alarm->Minute,alarm->Second);
#endif
  }
}

void P7110_GetNoteTimes(unsigned char *block, GSM_CalendarNote *c)
{
  time_t     alarmdiff;
	
  c->Time.Year = block[8]*256+block[9];
  c->Time.Month = block[10];
  c->Time.Day = block[11];
  if (c->Type != GCN_REMINDER) {
    c->Time.Hour = block[12];
    c->Time.Minute = block[13];
  } else {
    c->Time.Hour = 0;
    c->Time.Minute = 0;
  }
  c->Time.Second = 0;

#ifdef DEBUG
  fprintf(stdout, "   Date: %02i-%02i-%04i %02i:%02i:%02i\n",
              c->Time.Day,c->Time.Month,c->Time.Year,
              c->Time.Hour,c->Time.Minute,c->Time.Second);
#endif

  if (c->Type != GCN_REMINDER) {
    alarmdiff = block[14]*256 + block[15];
    P7110_GetNoteAlarm(alarmdiff, &(c->Time), &(c->Alarm), 60);

    c->Recurrance = block[16]*256 + block[17];
    DecodeUnicode(c->Text, block+20, block[18]);
  } else {
    c->Recurrance = block[12]*256 + block[13];
    DecodeUnicode(c->Text, block+16, block[14]);
  }

  /* 0xffff -> 1 Year (8760 hours) */
  if (c->Recurrance == 0xffff) c->Recurrance=8760;

#ifdef DEBUG
  fprintf(stdout, "   Recurrance: %i hours\n   Text: \"%s\"\n",
            c->Recurrance,c->Text);
#endif
}

void N7110_ReplyGetCalendarNote(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  int alarm;

  CurrentCalendarNote->YearOfBirth=0; //for other than birthday
  CurrentCalendarNote->AlarmType=0x00;//for other than birthday
  CurrentCalendarNote->Phone[0]=0;    //for other than call

  CurrentCalendarNote->Alarm.Year = 0;
  CurrentCalendarNote->Alarm.Month = 0;
  CurrentCalendarNote->Alarm.Day = 0;
  CurrentCalendarNote->Alarm.Hour = 0;
  CurrentCalendarNote->Alarm.Minute = 0;
  CurrentCalendarNote->Alarm.Second = 0;

#ifdef DEBUG
  fprintf(stdout, "Message: calendar note received\n");
#endif

  switch (MessageBuffer[6]) /* entry type */ {
    case 0x01: /* Meeting */
#ifdef DEBUG
      fprintf(stdout, "   Note type: meeting\n");
#endif
      CurrentCalendarNote->Type = GCN_MEETING;
      P7110_GetNoteTimes(MessageBuffer, CurrentCalendarNote);
      CurrentCalendarNoteError=GE_NONE;
      break;

    case 0x02: /* Call */
#ifdef DEBUG
      fprintf(stdout, "   Note type: call\n");
#endif
      CurrentCalendarNote->Type = GCN_CALL;
      P7110_GetNoteTimes(MessageBuffer, CurrentCalendarNote);
      DecodeUnicode(CurrentCalendarNote->Phone,
            MessageBuffer + 20 + MessageBuffer[18] * 2, MessageBuffer[19]);
#ifdef DEBUG
      fprintf(stdout, "   Phone number: \"%s\"\n",CurrentCalendarNote->Phone);
#endif
      CurrentCalendarNoteError=GE_NONE;
      break;

    case 0x04: /* Birthday */
#ifdef DEBUG
      fprintf(stdout, "   Note type: birthday\n");
#endif
      CurrentCalendarNote->Type = GCN_BIRTHDAY;

      alarm  = ((unsigned int)MessageBuffer[14]) << 24;
      alarm += ((unsigned int)MessageBuffer[15]) << 16;
      alarm += ((unsigned int)MessageBuffer[16]) << 8;
      alarm += MessageBuffer[17];

      /* CurrentCalendarNote->Time.Year is set earlier */
      CurrentCalendarNote->Time.Month = MessageBuffer[10];
      CurrentCalendarNote->Time.Day = MessageBuffer[11];
      CurrentCalendarNote->Time.Hour = 23;
      CurrentCalendarNote->Time.Minute = 59;
      CurrentCalendarNote->Time.Second = 58;
      P7110_GetNoteAlarm(alarm, &(CurrentCalendarNote->Time), &(CurrentCalendarNote->Alarm) ,1);

      CurrentCalendarNote->YearOfBirth = MessageBuffer[18]*256 + MessageBuffer[19];
      CurrentCalendarNote->Time.Year = CurrentCalendarNote->YearOfBirth;

      CurrentCalendarNote->AlarmType = MessageBuffer[20];

#ifdef DEBUG
      fprintf(stdout,_("   Alarm type: %s\n"), 
        (CurrentCalendarNote->AlarmType==0x00) ? "Tone  " : "Silent");

      fprintf(stdout, "   Birthday date: %02i-%02i-%04i (age %d)\n",CurrentCalendarNote->Time.Day,
              CurrentCalendarNote->Time.Month,CurrentCalendarNote->Time.Year,
              CurrentCalendarNote->Alarm.Year - CurrentCalendarNote->Time.Year);
#endif

      DecodeUnicode( CurrentCalendarNote->Text,MessageBuffer+22,MessageBuffer[21]);
#ifdef DEBUG
      fprintf(stdout, "   Text: \"%s\"\n",CurrentCalendarNote->Text);
#endif

      CurrentCalendarNote->Recurrance = 0;
      CurrentCalendarNoteError=GE_NONE;
      break;

    case 0x08: /* Reminder */
#ifdef DEBUG
      fprintf(stdout, "   Note type: reminder\n");
#endif
      CurrentCalendarNote->Type = GCN_REMINDER;
      P7110_GetNoteTimes(MessageBuffer, CurrentCalendarNote);
      CurrentCalendarNoteError=GE_NONE;
      break;

    default: /* unknown */
#ifdef DEBUG
      fprintf(stdout, "   Note type: UNKNOWN\n");
#endif
      break;
  }
}
  
GSM_Error N7110_GetCalendarNote(GSM_CalendarNote *CalendarNote)
{
  unsigned char req[] = { N6110_FRAME_HEADER,
 	0x19, /* get calendar note */
 	0x00, 0x00 //location
  };
 
  GSM_Error error;
  GSM_DateTime date_time;
  struct tm *now;
  time_t nowh;
  
  if (CalendarNote->ReadNotesInfo || CurrentCalendarNotesInfo.HowMany==2000) {
    error=N7110_GetCalendarNotesInfo(&CurrentCalendarNotesInfo);
    if (error!=GE_NONE) return error;
  }

#ifdef DEBUG
  fprintf(stdout, _("Calendar Notes Location Logical = %d.\n"),
             CalendarNote->Location);
  fprintf(stdout, _("Calendar Notes Location Phisical = %d.\n"),
             CurrentCalendarNotesInfo.Location[CalendarNote->Location-1]);
#endif

  /* this is for making xgnokii work.. */
  if (CalendarNote->Location > CurrentCalendarNotesInfo.HowMany )
    return GE_INVALIDCALNOTELOCATION;
  
  req[4] = CurrentCalendarNotesInfo.Location[CalendarNote->Location-1]>>8;
  req[5] = CurrentCalendarNotesInfo.Location[CalendarNote->Location-1]&0xff;
  CurrentCalendarNote = CalendarNote;

  /* We have to get current year. It's NOT written in frame for Birthday */
  error=N7110_GetDateTime(&date_time);
  if (error!=GE_NONE) return error;
  if (!date_time.IsSet) {
    nowh=time(NULL);
    now=localtime(&nowh);

    /* I have 100 (for 2000) Year now :-) */
    if (now->tm_year>99 && now->tm_year<1900) {
      now->tm_year=now->tm_year+1900;
    }
    date_time.Year=now->tm_year;
  }
  CurrentCalendarNote->Time.Year = date_time.Year;
  
  error=NULL_SendMessageSequence(50, &CurrentCalendarNoteError, 6, 0x13, req);
  
  CurrentCalendarNote = NULL;
  
  return error;
}

void N7110_ReplyWriteCalendarNote(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
#ifdef DEBUG
  char kz_types[][6] = { "MEET", "CALL", "BIRTH", "REM" };

  fprintf(stdout, 
      "Succesfully written Calendar Note Type %s on location %d\n",
        kz_types[(MessageBuffer[3]/2)-1], 
        MessageBuffer[4]*256+MessageBuffer[5] );

  fprintf(stdout, "--> Exit Status is %d (maybe is the size of buffer written to phone)\n", 
        MessageBuffer[6]*256+MessageBuffer[7] );
#endif
  CurrentCalendarNoteError=GE_NONE;
}

long P7110_GetNoteAlarmDiff(GSM_DateTime *time, GSM_DateTime *alarm)
{
  time_t     t_alarm;
  time_t     t_time;
  struct tm  tm_alarm;
  struct tm  tm_time;

  tzset();

  tm_alarm.tm_year=alarm->Year-1900;
  tm_alarm.tm_mon=alarm->Month-1;
  tm_alarm.tm_mday=alarm->Day;
  tm_alarm.tm_hour=alarm->Hour;
  tm_alarm.tm_min=alarm->Minute;
  tm_alarm.tm_sec=alarm->Second;
  tm_alarm.tm_isdst=0;
  t_alarm = mktime(&tm_alarm);

  tm_time.tm_year=time->Year-1900;
  tm_time.tm_mon=time->Month-1;
  tm_time.tm_mday=time->Day;
  tm_time.tm_hour=time->Hour;
  tm_time.tm_min=time->Minute;
  tm_time.tm_sec=time->Second;
  tm_time.tm_isdst=0;
  t_time = mktime(&tm_time);

#ifdef DEBUG
  fprintf(stdout, "   Alarm: %02i-%02i-%04i %02i:%02i:%02i\n",
                 alarm->Day,alarm->Month,alarm->Year,
                 alarm->Hour,alarm->Minute,alarm->Second);
  fprintf(stdout, "   Date: %02i-%02i-%04i %02i:%02i:%02i\n",
                 time->Day,time->Month,time->Year,
                 time->Hour,time->Minute,time->Second);
  fprintf(stdout,_("Difference in alarm time is %f\n"),difftime( t_time, t_alarm )+3600);
#endif

  return difftime( t_time ,t_alarm )+3600;
}

GSM_Error N7110_WriteCalendarNote(GSM_CalendarNote *CalendarNote)
{
  unsigned char req[200] = { N6110_FRAME_HEADER,
                             0x01,       /* note type ... */
                             0x00, 0x00, /* location */
                             0x00,       /* entry type */
                             0x00,       //fixed
                             0x00, 0x00, 0x00, 0x00, /* Year(2bytes), Month, Day */
                             /* here starts block */
                             0x00, 0x00, 0x00, 0x00,0x00, 0x00 /* ... depends on note type ... */
                           };

  int count=0;
  long seconds, minutes;
 
  GSM_Error error;
  int firstFreeLocation;

  /*
   * 6210/7110 needs to seek the first free pos to inhabit with next note
   */
  error=N7110_FirstCalendarFreePos(&firstFreeLocation);
  if (error!=GE_NONE) return error;

#ifdef DEBUG
  fprintf(stdout, _("First free calendar location is = %d.\n"),
             firstFreeLocation);
#endif

  /* Location */
  req[4]=0x00;
  req[5]=0x00;

  switch( CalendarNote->Type ) {
    case GCN_MEETING : req[6]=0x01; req[3]=0x01; break;
    case GCN_CALL    : req[6]=0x02; req[3]=0x03; break;
    case GCN_BIRTHDAY: req[6]=0x04; req[3]=0x05; break;
    case GCN_REMINDER: req[6]=0x08; req[3]=0x07; break;
  }

  req[8]=CalendarNote->Time.Year>>8;
  req[9]=CalendarNote->Time.Year&0xff;
  req[10]=CalendarNote->Time.Month;
  req[11]=CalendarNote->Time.Day;

  /* From here starts BLOCK */
  count=12;
  switch( CalendarNote->Type ) {

    case GCN_MEETING:
      req[count++]=CalendarNote->Time.Hour;   // 12
      req[count++]=CalendarNote->Time.Minute; // 13
      /* Alarm .. */
      req[count++]=0xff; // 14
      req[count++]=0xff; // 15
      if( CalendarNote->Alarm.Year )
      {
        seconds= P7110_GetNoteAlarmDiff(&CalendarNote->Time, 
                                        &CalendarNote->Alarm);
        if( seconds>=0L ) { /* Otherwise it's an error condition.... */
          minutes=seconds/60L;
          count-=2;
          req[count++]=minutes>>8;
          req[count++]=minutes&0xff;
        }
      }
      /* Recurrance */
      if( CalendarNote->Recurrance >= 8760 )
        CalendarNote->Recurrance = 0xffff; /* setting  1 Year repeat */
      req[count++]=CalendarNote->Recurrance>>8;   // 16
      req[count++]=CalendarNote->Recurrance&0xff; // 17
      /* len of text */
      req[count++]=strlen(CalendarNote->Text);    // 18
      /* fixed 0x00 */
      req[count++]=0x00; // 19
      /* Text */
#ifdef DEBUG
      fprintf(stdout, "Count before encode = %d\n", count );
      fprintf(stdout, "Meeting Text is = \"%s\"\n", CalendarNote->Text );
#endif

      EncodeUnicode (req+count,CalendarNote->Text ,strlen(CalendarNote->Text));// 20->N
      count=count+2*strlen(CalendarNote->Text);
      break;

    case GCN_CALL:
      req[count++]=CalendarNote->Time.Hour;   // 12
      req[count++]=CalendarNote->Time.Minute; // 13
      /* Alarm .. */
      req[count++]=0xff; // 14
      req[count++]=0xff; // 15
      if( CalendarNote->Alarm.Year )
      {
        seconds= P7110_GetNoteAlarmDiff(&CalendarNote->Time, 
                                        &CalendarNote->Alarm);
        if( seconds>=0L ) { /* Otherwise it's an error condition.... */
          minutes=seconds/60L;
          count-=2;
          req[count++]=minutes>>8;
          req[count++]=minutes&0xff;
        }
      }
      /* Recurrance */
      if( CalendarNote->Recurrance >= 8760 )
        CalendarNote->Recurrance = 0xffff; /* setting  1 Year repeat */
      req[count++]=CalendarNote->Recurrance>>8;   // 16
      req[count++]=CalendarNote->Recurrance&0xff; // 17
      /* len of text */
      req[count++]=strlen(CalendarNote->Text);    // 18
      /* fixed 0x00 */
      req[count++]=strlen(CalendarNote->Phone);   // 19
      /* Text */
      EncodeUnicode (req+count,CalendarNote->Text ,strlen(CalendarNote->Text));// 20->N
      count=count+2*strlen(CalendarNote->Text);
      EncodeUnicode (req+count,CalendarNote->Phone ,strlen(CalendarNote->Phone));// (N+1)->n
      count=count+2*strlen(CalendarNote->Phone);
      break;

    case GCN_BIRTHDAY:
      req[count++]=0x00; // 12 Fixed
      req[count++]=0x00; // 13 Fixed

      /* Alarm .. */
      req[count++]=0x00; req[count++]=0x00; // 14, 15
      req[count++]=0xff; // 16
      req[count++]=0xff; // 17
      if( CalendarNote->Alarm.Year ) {
        // I try with Time.Year = Alarm.Year. If negative, I increase 1 year,
        // but only once ! This thing, because I may have Alarm period across
        // a year. (eg. Birthday on 2001-01-10 and Alarm on 2000-12-27)

        CalendarNote->Time.Year = CalendarNote->Alarm.Year;
        if( (seconds= P7110_GetNoteAlarmDiff(&CalendarNote->Time, 
                                        &CalendarNote->Alarm)) < 0L ) {
          CalendarNote->Time.Year++;
          seconds= P7110_GetNoteAlarmDiff(&CalendarNote->Time,
              &CalendarNote->Alarm);
        }
        if( seconds>=0L ) { /* Otherwise it's an error condition.... */
          count-=4;
          req[count++]=seconds>>24;              // 14
          req[count++]=(seconds>>16) & 0xff;     // 15
          req[count++]=(seconds>>8) & 0xff;      // 16
          req[count++]=seconds&0xff;             // 17
        }
      }

      req[count++]=CalendarNote->AlarmType; // 18

      /* len of text */
      req[count++]=strlen(CalendarNote->Text); // 19

      /* Text */
#ifdef DEBUG
      fprintf(stdout, "Count before encode = %d\n", count );
      fprintf(stdout, "Meeting Text is = \"%s\" Altype is 0x%02x \n", CalendarNote->Text , CalendarNote->AlarmType );
#endif
      EncodeUnicode (req+count,CalendarNote->Text ,strlen(CalendarNote->Text));// 22->N
      count=count+2*strlen(CalendarNote->Text);
      break;

    case GCN_REMINDER:
      /* Recurrance */
      if( CalendarNote->Recurrance >= 8760 )
        CalendarNote->Recurrance = 0xffff; /* setting  1 Year repeat */
      req[count++]=CalendarNote->Recurrance>>8;   // 12
      req[count++]=CalendarNote->Recurrance&0xff; // 13
      /* len of text */
      req[count++]=strlen(CalendarNote->Text);    // 14
      /* fixed 0x00 */
      req[count++]=0x00; // 15
      /* Text */
      EncodeUnicode (req+count,CalendarNote->Text ,strlen(CalendarNote->Text));// 16->N
      count=count+2*strlen(CalendarNote->Text);
      break;
  }

  /* padding */
  req[count]=0x00;
#ifdef DEBUG
  fprintf(stdout, "Count after padding = %d\n", count );
#endif
  
  CurrentCalendarNote = CalendarNote;
  
  error=NULL_SendMessageSequence(50, &CurrentCalendarNoteError, count, 0x13, req);
  
  CurrentCalendarNote = NULL;

  return error;
}

void N7110_ReplyFirstCalendarFreePos(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  *CurrentFirstCalendarFreePos = MessageBuffer[4]*256+MessageBuffer[5];
  CurrentFirstCalendarFreePosError=GE_NONE;
}

GSM_Error N7110_FirstCalendarFreePos(int *FreePos)
{
  unsigned char req[] = { N6110_FRAME_HEADER, 0x31 };
 
  GSM_Error error;

  CurrentFirstCalendarFreePos = FreePos;
  
  error=NULL_SendMessageSequence(50, &CurrentFirstCalendarFreePosError, 
      4, 0x13, req);
  
  CurrentFirstCalendarFreePos = NULL;
  
  return error;
}

void N7110_ReplyDeleteCalendarNote(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
#ifdef DEBUG
  fprintf(stdout, 
       "Succesfully Delete Calendar Note on location %d\n",
       MessageBuffer[4]*256+MessageBuffer[5] );

  fprintf(stdout, "--> Other data are :\n" );
  fprintf(stdout, " '%c'[0x%02x](%3d), '%c'[0x%02x](%3d), '%c'[0x%02x](%3d), '%c'[0x%02x](%3d)\n",
      MessageBuffer[6], MessageBuffer[6], MessageBuffer[6],
      MessageBuffer[7], MessageBuffer[7], MessageBuffer[7],
      MessageBuffer[8], MessageBuffer[8], MessageBuffer[8],
      MessageBuffer[9], MessageBuffer[9], MessageBuffer[9] );
#endif
  CurrentCalendarNoteError=GE_NONE;
}

GSM_Error N7110_DeleteCalendarNote(GSM_CalendarNote *CalendarNote)
{
  unsigned char req[] = { N6110_FRAME_HEADER,
                          0x0b,      /* delete calendar note */
                          0x00, 0x00 //location
  };
 
  GSM_Error error;
  
  if (CalendarNote->ReadNotesInfo || CurrentCalendarNotesInfo.HowMany==2000) {
    error=N7110_GetCalendarNotesInfo(&CurrentCalendarNotesInfo);
    if (error!=GE_NONE) return error;
  }

  /* this is for making xgnokii work.. */
  if (CalendarNote->Location > CurrentCalendarNotesInfo.HowMany )
    return GE_INVALIDCALNOTELOCATION;
  
  req[4] = CurrentCalendarNotesInfo.Location[CalendarNote->Location-1]>>8;
  req[5] = CurrentCalendarNotesInfo.Location[CalendarNote->Location-1]&0xff;

  CurrentCalendarNote = CalendarNote;
  
  error=NULL_SendMessageSequence(50, &CurrentCalendarNoteError, 6, 0x13, req);
  
  CurrentCalendarNote = NULL;
  
  return error;
}

void N7110_ReplyGetSMSFolders(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  wchar_t wc;

  int i, j, tmp;

#ifdef DEBUG
  fprintf(stdout, _("Message: SMS Folders received:\n"));
#endif /* DEBUG */
 
  i=5;
      
  CurrentSMSFoldersCount=MessageBuffer[4];
      
  for (j=0;j<MessageBuffer[4];j++) {
    strcpy(CurrentSMSFolders->Folder[j].Name,"               ");
#ifdef DEBUG
    fprintf(stdout, _("   Folder Index: %d"),MessageBuffer[i]);
#endif /* DEBUG */
    CurrentSMSFolders->FoldersID[j]=MessageBuffer[i];

    i=i+2;

#ifdef DEBUG
    fprintf(stdout, _(", folder name: "));
#endif /* DEBUG */
    tmp=0;
    while ((MessageBuffer[i]!=0x00) & (MessageBuffer[i+1]==0x00)) {

      wc = MessageBuffer[i] | (MessageBuffer[i+1] << 8);
	
      CurrentSMSFolders->Folder[j].Name[tmp]=DecodeWithUnicodeAlphabet(wc);
#ifdef DEBUG
      fprintf(stdout, _("%c"),CurrentSMSFolders->Folder[j].Name[tmp]);
#endif /* DEBUG */
      tmp++;
      i=i+2;
    }
#ifdef DEBUG
    fprintf(stdout, _("\n"));
#endif /* DEBUG */
    tmp=0;
    i=i+1;
  }
      
  CurrentSMSFoldersError=GE_NONE;      
}

GSM_Error N7110_GetSMSFolders ( GSM_SMSFolders *folders)
{
  unsigned char req[] = { N6110_FRAME_HEADER, 0x7A, 0x00, 0x00};
  
  GSM_Error error;
  
  CurrentSMSFolders=folders;
  
  error=NULL_SendMessageSequence(20, &CurrentSMSFoldersError, 6, 0x14, req);

  folders->number=CurrentSMSFoldersCount;
    
  return error;
}

void N7110_ReplyGetSMSFolderStatus(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  int i;

#ifdef DEBUG
  fprintf(stdout, _("Message: SMS Folder status received\n"));
  fprintf(stdout, _("   Number of Entries: %i"),MessageBuffer[4]*256+MessageBuffer[5]);
#endif /* DEBUG */
  CurrentSMSFolder.number=MessageBuffer[4]*256+MessageBuffer[5];
#ifdef DEBUG
  fprintf(stdout, _(" (indexes "));
#endif /* DEBUG */

  for (i=0;i<MessageBuffer[4]*256+MessageBuffer[5];i++) {
#ifdef DEBUG
    fprintf(stdout, _("%i, "), MessageBuffer[6+(i*2)]*256+MessageBuffer[(i*2)+7]);
#endif /* DEBUG */
    CurrentSMSFolder.locations[i]=MessageBuffer[6+(i*2)]*256+MessageBuffer[(i*2)+7];
  }

#ifdef DEBUG
  fprintf(stdout, _(")\n"));
#endif /* DEBUG */
      
  CurrentSMSFolderError=GE_NONE;      
}

GSM_Error N7110_GetSMSFolderStatus ( GSM_OneSMSFolder *folder, u8 ID)
{
        unsigned char req[] = { N7110_FRAME_HEADER, 
                          0x6b, 
                          0x08, //folderID 
                          0x0F, 
                          0x01};
  
        GSM_Error error;
  
        CurrentSMSFolder = *folder;
        CurrentSMSFolderID = ID;
        req[4] = ID;
  
        error=NULL_SendMessageSequence(20, &CurrentSMSFolderError, 7, 0x14, req);

	*folder=CurrentSMSFolder;

        return error;
}

void N7110_ReplyDeleteSMSMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{

#ifdef DEBUG
  fprintf(stdout, _("Message: SMS deleted succesfully\n"));
#endif /* DEBUG */
  CurrentSMSMessageError = GE_NONE;
}

GSM_Error N7110_DeleteSMSMessage(GSM_SMSMessage *message)
{
  unsigned char req[] = {
    N7110_FRAME_HEADER, 0x0a, /* delete SMS request */
    0x00, /* folder (req[4])*/
    0x00, /* location */
    0x00, /* location (req[6])*/
    0x01 };

    GSM_Error error;
    int smsnum, location;
    u8 folderid;

    smsnum = message->Location;
    /* We made "fake" SMS numbering for SMS in 7110/6210/etc. */
    if ((error = N7110_HandleSMSLocation(smsnum, &folderid, &location, NULL, GSH_DELETE))!=GE_NONE)
            return(error);
 
    req[4]=folderid;
    req[5]=location / 256;
    req[6]=location;

#ifdef DEBUG
    printf("delete sms: folder %d, location %d\n",folderid,location);
#endif

    return NULL_SendMessageSequence
      (50, &CurrentSMSMessageError, 8, 0x14, req);
}

GSM_Error N7110_GetSMSMessage(GSM_SMSMessage *message)
{

        unsigned char req[] = { N6110_FRAME_HEADER,
                          0x07, 
                          0x08, // folder ID
                          0x00, 0x05, // location
                          0x01, 
                          0x65, 
                          0x01};

        int smsnum, location;
	u8 folderid;

        GSM_Error error;
        CurrentSMSMessage = message;
        CurrentSMSMessageError = GE_BUSY;


        smsnum = message->Location;

	/* we make central handling of real location */
	error = N7110_HandleSMSLocation(smsnum, &folderid, &location, message, GSH_GET);
        /* if smsnum is 0 (next sms) we need real smsnum */
	N7110_SMS2FakeLocation( &smsnum, folderid, location);
	message->Location = smsnum;
        switch(error)
        {
         case GE_SMSISINMEM: /* future use: get already reed sms from mem */
				return GE_NONE;
				break;
	 case GE_NONE:		req[4]=folderid;
        			req[5]=location / 256;
        			req[6]=location;
#ifdef DEBUG
    fprintf(stdout, _("GetSMSMessage: read folder %d, location %d\n"),folderid,location);
#endif
        			return NULL_SendMessageSequence(100, &CurrentSMSMessageError, 10, 0x14, req);
				break;
	 default: 		break;
	 }

    return(error);
}

void N7110_ReplySaveSMSMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  int smsnum;

  switch (MessageBuffer[3]) {

  /* save sms */
  case 0x05:      
#ifdef DEBUG
    fprintf(stdout, _("Message: SMS Message stored at folder %d, location %d\n"), MessageBuffer[4], MessageBuffer[6]);
#endif

    if (CurrentSMSMessage!=NULL) {     
      N7110_SMS2FakeLocation(&smsnum, (u8) MessageBuffer[4], (int) MessageBuffer[6]);
      CurrentSMSMessage->MessageNumber=smsnum;
    }
    
    CurrentSMSMessageError = GE_NONE;
    break;

  /* save sms failed */
  case 0x06:
      
#ifdef DEBUG
    fprintf(stdout, _("Message: SMS Message save failed\n"));
#endif
    CurrentSMSMessageError = GE_SMSSAVEFAILED;
    break;

  case 0x84:
#ifdef DEBUG
    fprintf(stdout, _("Message: Changed name for SMS Message\n"));
#endif
    CurrentSMSMessageError = GE_NONE;
    break;

  }
}

GSM_Error N7110_SaveSMSMessage(GSM_SMSMessage *SMS)
{
  unsigned char req[256] = {
    N6110_FRAME_HEADER, 0x04, /* SMS save request*/
    0x03, /* default: mark sms as  GSS_NOTSENTREAD */
    0x10, /* folder (req[5])*/
    0x00, /* location */
    0x00, /* location (req[7])*/
    0x00  /* ??? */
  };

  unsigned char req2[200] = {N6110_FRAME_HEADER, 0x83};

  int length,smsnum,location;
  u8 folderid;
  GSM_Error error;
  SMS_MessageType PDU;

  smsnum = SMS->Location;
  
#ifdef DEBUG
  printf("save sms: smsnum is :%d\n",smsnum);
#endif

  if ( SMS->Status ==  GSS_SENTREAD) req[4] = 0x01;

  folderid = SMS->folder;

  PDU=SMS_Deliver;
  
  error=GSM_EncodeNokiaSMSFrame(SMS, req+9, &length, PDU);
  if (error != GE_NONE) return error;
 
  CurrentSMSMessage = SMS;

  error=N7110_HandleSMSLocation(smsnum, &folderid, &location, SMS, GSH_SAVE);
  switch (error)
  {

    case  GE_BUSYSMSLOCATION: /* delete old sms before save */
      error = N7110_DeleteSMSMessage(SMS);	
      if (error != GE_NONE) return error;
      break;
    case  GE_NONE:
      break;
    default:
      return error;
      break;
  }
    
#ifdef DEBUG
  printf("save sms: want to save at folder:%d , location:%d\n",folderid,location);
#endif

  req[5]=folderid;
  req[6]=location / 256;
  req[7]=location;
 
  error=NULL_SendMessageSequence
    (70, &CurrentSMSMessageError, 40+length, 0x14, req);

  if (error==GE_NONE && SMS->Name[0]!=0) {
    length=4;
    N7110_Fake2SMSLocation(SMS->MessageNumber, &folderid, &location);
    req2[length++]=folderid;
    req2[length++]=location / 256;
    req2[length++]=location;
    EncodeUnicode(req2+length, SMS->Name, strlen(SMS->Name));
    length=length+strlen(SMS->Name)*2;
    req2[length++]=0;
    req2[length++]=0;
    error=NULL_SendMessageSequence
      (70, &CurrentSMSMessageError, length, 0x14, req2);
  }

  return error;
}

/* handling for 7110 folders */
GSM_Error N7110_HandleSMSLocation(int smsnum, u8 *folder, int *location, GSM_SMSMessage *message, GSM_SMSHandleAction action)
{
    /* remember me */
    static int nextSMSmessage = -1;
    static GSM_SMSStatus Status;

    bool found = false;
    int folderid;
    GSM_OneSMSFolder onefolder;
    GSM_Error error;
    int ismsnum;  
    int i;

    switch (action) {
	case GSH_DELETE: /* future use: delete sms in mem -> free memory */
	                 /* for now we make same as in GSH_GET           */
	case GSH_GET:    /* if smsnum == 0 user wants to read next sms   */
          if (smsnum == 0 )
          {
   	    /* first time we read folderstatus */
  	    if (  nextSMSmessage == -1)
	    {
               nextSMSmessage = 0;
	       error =  N7110_GetSMSStatus( &Status);
               if (error!=GE_NONE) return error;
  	    }
            ismsnum = Status.foldertable[nextSMSmessage].smsnum;
            N7110_Fake2SMSLocation( ismsnum, folder, location);
            nextSMSmessage++;
            if(nextSMSmessage >= Status.Number)
	    { 
              nextSMSmessage = -1;
#ifdef DEBUG
              printf("HandleSMS: setting nextSMSmessage to -1\n");
#endif
	    }
          } else /* user give location -> make fake location */
          {
            /* future use: get already read sms from mem */
            N7110_Fake2SMSLocation( smsnum, folder, location);
          }
          break;
	case GSH_SAVE: /* if smsnum == 0 user wants to save in specific folder */
          if (smsnum == 0 )
          {
            /* calculate correct folderid */
            *folder = ( *folder + 1) * 8;

            *location = 0;
          } else /* user give location -> make fake location */
          {
            N7110_Fake2SMSLocation( smsnum, folder, location);
            folderid = *folder;

            error=N7110_GetSMSFolderStatus(&onefolder, folderid );
            if (error!=GE_NONE) return error;

            /* is there a sms at that location ? */
            for (i=0; i<CurrentSMSFolder.number; i++)
              if ( CurrentSMSFolder.locations[i] == *location ) found = true;
    
            if (found == true) return GE_BUSYSMSLOCATION;
    	                  else return GE_NONE;
          }
          break;  
        default:
          return GE_UNKNOWN;
    } //switch
    return GE_NONE;
}

/* input: fake smsnum, output: folderid & location */
void  N7110_Fake2SMSLocation(int smsnum, u8 *folderid, int *location)
{
  int ifolderid;

  ifolderid = smsnum / N7110_MAXSMSINFOLDER;
  *folderid = ifolderid * 0x08;
  *location = smsnum -  ifolderid * N7110_MAXSMSINFOLDER;
}

/* input; folderid & location, output: fake smsnum */
void N7110_SMS2FakeLocation(int *smsnum, u8 folderid, int location)
{
  int ifolderid;
  
  ifolderid = folderid / 0x08;
  *smsnum = ifolderid * N7110_MAXSMSINFOLDER + location;
}

GSM_Error N7110_GetRFLevel(GSM_RFUnits *units, float *level)
{
  unsigned char request[] = {N6110_FRAME_HEADER, 0x81};

  int timeout=10;
  int rf_level;

  CurrentRFLevel=-1;

  Protocol->SendMessage(4, 0x0a, request);

  /* Wait for timeout or other error. */
  while (timeout != 0 && CurrentRFLevel == -1 ) {

    if (--timeout == 0)
      return (GE_TIMEOUT);

    usleep (100000);
  }

  /* Make copy in case it changes. */
  rf_level = CurrentRFLevel;

  if (rf_level == -1)
    return (GE_NOLINK);

  /* Now convert between the different units we support. */

  /* Arbitrary units. */
  *units = GRF_Percentage;
  *level = rf_level;
  return (GE_NONE);
}

GSM_Error N7110_GetBatteryLevel(GSM_BatteryUnits *units, float *level)
{
  unsigned char request[] = {N6110_FRAME_HEADER, 0x02};

  int timeout=10;
  int batt_level;

  CurrentBatteryLevel=-1;

  Protocol->SendMessage(4, 0x17, request);

  /* Wait for timeout or other error. */
  while (timeout != 0 && CurrentBatteryLevel == -1 ) {

    if (--timeout == 0)
      return (GE_TIMEOUT);

    usleep (100000);
  }

  /* Take copy in case it changes. */
  batt_level = CurrentBatteryLevel;

  if (batt_level == -1)
    return (GE_NOLINK);
    
  /* Only units we handle at present are GBU_Arbitrary */
  *units = GBU_Percentage;
  *level = batt_level;
  return (GE_NONE);
}

GSM_Error N7110_GetSecurityCode(GSM_SecurityCode *SecurityCode)
{
  unsigned char req[] = {0x00, 0x01, 0x01, 0xee, 0x1c};

  if (SecurityCode->Type!=GSCT_SecurityCode) return GE_NOTSUPPORTED;

  CurrentSecurityCode=SecurityCode;

  return NULL_SendMessageSequence
    (50, &CurrentSecurityCodeError, 5, 0x7a, req);
}

GSM_Error N7110_GetDateTime(GSM_DateTime *date_time)
{
  return N6110_PrivGetDateTime(date_time,0x19);
}

/* Needs SIM card with PIN in phone */
GSM_Error N7110_SetDateTime(GSM_DateTime *date_time)
{
  return N6110_PrivSetDateTime(date_time,0x19);
}

GSM_Error N7110_GetAlarm(int alarm_number, GSM_DateTime *date_time)
{
  return N6110_PrivGetAlarm(alarm_number,date_time,0x19);
}

/* FIXME: we should also allow to set the alarm off :-) */
GSM_Error N7110_SetAlarm(int alarm_number, GSM_DateTime *date_time)
{
  return N6110_PrivSetAlarm(alarm_number,date_time, 0x19);
}

void N7110_ReplyGetSMSStatus(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  switch (MessageBuffer[3]) {

  /* sms status */
  case 0x37:

#ifdef DEBUG
    fprintf(stdout, _("Message: SMS Status Received\n"));
    fprintf(stdout, _("  Used msg in phone memory: %i\n"),MessageBuffer[10]*256+MessageBuffer[11]);
    fprintf(stdout, _("  Unread msg in phone memory: %i\n"),MessageBuffer[12]*256+MessageBuffer[13]);
    fprintf(stdout, _("  Used msg in SIM: %i\n"),MessageBuffer[14]*256+MessageBuffer[15]);
    fprintf(stdout, _("  Unread msg in SIM: %i\n"),MessageBuffer[16]*256+MessageBuffer[17]);
#endif /* DEBUG */
    CurrentSMSStatus->UnRead = MessageBuffer[13];
    CurrentSMSStatusError = GE_NONE;
    break;

  case 0x38:

#ifdef DEBUG
    fprintf(stdout, _("Message: SMS Status error, probably not authorized by PIN\n"));
#endif /* DEBUG */
    CurrentSMSStatusError = GE_INTERNALERROR;
    break;

  }
}

GSM_Error N7110_GetSMSStatus(GSM_SMSStatus *Status)
{
  /* RTH FIXME: what is exact meaning of 0x0037 answer ? */
  /* we check all folders, but get sum of unread sms via 0x0036 request */
  unsigned char req[] = {N6110_FRAME_HEADER, 0x36, 0x64};

  GSM_SMSFolders folders;
  GSM_OneSMSFolder folder;
  GSM_Error error;
  u8 ifolder;
  int ismsnum,ilocation;
  int i,j,smsmaxnum;

  CurrentSMSStatus = Status;

  /* read all SMS folders */
  /* check for unread messages in folder 0xf8 */
  error = N7110_GetSMSFolderStatus(&folder, 0xf8);
  if (error != GE_NONE) return error;
    
  smsmaxnum=0;
  for(j=0; j<folder.number; j++)
  {
    ifolder = 0; /*read unread messages from folder 0 */
    ilocation = folder.locations[j];
    N7110_SMS2FakeLocation( &ismsnum, ifolder, ilocation) ;
    CurrentSMSStatus->foldertable[smsmaxnum].smsnum = ismsnum; 
    CurrentSMSStatus->foldertable[smsmaxnum].folder = ifolder; 
    CurrentSMSStatus->foldertable[smsmaxnum].location = ilocation; 
    smsmaxnum++;
  } 
 
  /* read standard folders */
  N7110_GetSMSFolders (&folders);
  for(i=0; i<CurrentSMSFoldersCount; i++)
  {
    error = N7110_GetSMSFolderStatus(&CurrentSMSFolders->Folder[i], CurrentSMSFolders->FoldersID[i]);
    if (error != GE_NONE) return error;
     
    for(j=0; j<CurrentSMSFolders->Folder[i].number; j++)
    {
      ifolder = CurrentSMSFolders->FoldersID[i];
      ilocation = CurrentSMSFolders->Folder[i].locations[j];
      N7110_SMS2FakeLocation( &ismsnum, ifolder, ilocation);
      CurrentSMSStatus->foldertable[smsmaxnum].smsnum = ismsnum; 
      CurrentSMSStatus->foldertable[smsmaxnum].folder = ifolder; 
      CurrentSMSStatus->foldertable[smsmaxnum].location = ilocation; 
      smsmaxnum++;
    }
  }
  CurrentSMSStatus->Number = smsmaxnum;

  return NULL_SendMessageSequence(10, &CurrentSMSStatusError, 5, 0x14, req);
}

void N7110_DecodePhonebookFrame(GSM_PhonebookEntry *entry,u8 *MessageBuffer,u16 MessageLength)
{
  int blockcount=0;
  unsigned char *pBlock;
  int length=0;

#ifdef DEBUG
  int j;
#endif

  pBlock = &MessageBuffer[0];

  while (length!=MessageLength) {
    GSM_SubPhonebookEntry* pEntry = &entry->SubEntries[blockcount];

#ifdef DEBUG
    fprintf(stdout,_("    "));
    for (j=5;j<(pBlock[3]-6)+5;j++) fprintf(stdout,_("%02x "),pBlock[j]);
    fprintf(stdout,_("\n"));
#endif

    switch( pBlock[0] ) {
    case N7110_ENTRYTYPE_SPEEDDIAL:
    
      CurrentSpeedDialEntry->MemoryType = GMT_SM;
      if (pBlock[4]==0x02) CurrentSpeedDialEntry->MemoryType = GMT_ME;
      
      CurrentSpeedDialEntry->Location = pBlock[7]+pBlock[6]*256;

#ifdef DEBUG
      fprintf(stdout, _("    Speed dial\n"));
      fprintf(stdout, _("      Location: %d\n"), CurrentSpeedDialEntry->Location);
      fprintf(stdout, _("      MemoryType: %i\n"), CurrentSpeedDialEntry->MemoryType);
      fprintf(stdout, _("      Number: %d\n"), CurrentSpeedDialEntry->Number);
#endif /* DEBUG */

      CurrentSpeedDialError=GE_NONE;
      break;

    case N7110_ENTRYTYPE_NAME:
      DecodeUnicode (entry->Name, pBlock+6, pBlock[5]/2);
      if (CurrentGetBitmap && CurrentGetBitmapError == GE_BUSY)
        strncpy(CurrentGetBitmap->text,entry->Name,sizeof(CurrentGetBitmap->text));
      entry->Empty = false;
#ifdef DEBUG
      fprintf(stdout, _("    Name:\n"));
      fprintf(stdout, _("      Name: %s\n"), entry->Name);
#endif /* DEBUG */
      break;

    case N7110_ENTRYTYPE_NUMBER:
      pEntry->EntryType   = pBlock[0];
      pEntry->NumberType  = pBlock[5];
      pEntry->BlockNumber = pBlock[4];

      DecodeUnicode (pEntry->data.Number, pBlock+10, pBlock[9]/2);

#ifdef DEBUG
      fprintf(stdout, _("    Number:\n"));
      fprintf(stdout, _("      Type: %d (%02x)\n"),pEntry->NumberType,pEntry->NumberType);
      fprintf(stdout, _("      Number: %s\n"),pEntry->data.Number);
#endif /* DEBUG */
      if( pEntry->EntryType == GSM_Number &&
         ((pEntry->NumberType == GSM_General && !strcmp(entry->Number,""))
	     || pEntry->NumberType == GSM_SIM)) {
        strcpy( entry->Number, pEntry->data.Number );
        *pEntry->data.Number = 0;
      } else
        blockcount++;
      break;
    case N7110_ENTRYTYPE_DATE:
      pEntry->EntryType        = pBlock[0];
      pEntry->NumberType       = pBlock[5];
      pEntry->BlockNumber      = pBlock[4];
      DecodeDateTime(pBlock+6, &pEntry->data.Date);
#ifdef DEBUG
      fprintf(stdout, _("    Date:\n"));
      fprintf(stdout, _("      Date: %02u.%02u.%04u\n"), pEntry->data.Date.Day,
        pEntry->data.Date.Month, pEntry->data.Date.Year );
      fprintf(stdout, _("      Time: %02u:%02u:%02u\n"), pEntry->data.Date.Hour,
        pEntry->data.Date.Minute, pEntry->data.Date.Second);
#endif /* DEBUG */
      blockcount++;
      break;
    case N7110_ENTRYTYPE_NOTE:
    case N7110_ENTRYTYPE_POSTAL:
    case N7110_ENTRYTYPE_EMAIL:
      pEntry->EntryType   = pBlock[0];
      pEntry->NumberType  = 0;
      pEntry->BlockNumber = pBlock[4];

      DecodeUnicode (pEntry->data.Number, pBlock+6, pBlock[5]/2);

#ifdef DEBUG
      fprintf(stdout, _("    Email or note or postal:\n"));
      fprintf(stdout, _("      Type: %d (%02x)\n"),pEntry->EntryType,pEntry->EntryType);
      fprintf(stdout, _("      Text: %s\n"),pEntry->data.Number);
#endif /* DEBUG */
      blockcount++;
      break;
    case N7110_ENTRYTYPE_GROUP:
      entry->Group = pBlock[5]-1;  /* 0 = family as for 6110 */
      if (CurrentGetBitmap && CurrentGetBitmapError == GE_BUSY)
        CurrentGetBitmap->number = entry->Group;
#ifdef DEBUG
      fprintf(stdout, _("    Group: %d\n"), entry->Group);
#endif /* DEBUG */
      break;
    case N7110_ENTRYTYPE_RINGTONE:
      if (CurrentGetBitmap && CurrentGetBitmapError == GE_BUSY)
        CurrentGetBitmap->ringtone = pBlock[5];
#ifdef DEBUG
      fprintf(stdout, _("    Group ringtone number %d received.\n"), pBlock[5]);
#endif /* DEBUG */
      break;
    case N7110_ENTRYTYPE_LOGOON:
      if (CurrentGetBitmap && CurrentGetBitmapError == GE_BUSY)
        CurrentGetBitmap->enabled = pBlock[5];
#ifdef DEBUG
      fprintf(stdout, _("    Logo enabled = %d received.\n"), pBlock[5]);
#endif /* DEBUG */
      break;
    case N7110_ENTRYTYPE_GROUPLOGO:
#ifdef DEBUG
      fprintf(stdout, _("    Caller group logo received.\n"));
#endif /* DEBUG */
      if (CurrentGetBitmap && CurrentGetBitmapError == GE_BUSY) {
        CurrentGetBitmap->width = pBlock[5];
        CurrentGetBitmap->height= pBlock[6];
        CurrentGetBitmap->size  = pBlock[9];
        if (CurrentGetBitmap->size > sizeof(CurrentGetBitmap->bitmap))
          CurrentGetBitmap->size = CurrentGetBitmap->size;
        memcpy(CurrentGetBitmap->bitmap,pBlock+10,CurrentGetBitmap->size);
      }
      break;

    default:
#ifdef DEBUG
      fprintf(stdout, _("    Unknown Entry Code (%u) received.\n"), pBlock[0] );
#endif /* DEBUG */
      break;
    }
       
#ifdef DEBUG
    fprintf(stdout, _("    Blocksize was: %d (%02x)\n"), (int) pBlock[3], pBlock[3]);
#endif
    length=length+pBlock[3];

    pBlock = &pBlock[(int) pBlock[3]];
  }

  entry->SubEntriesCount = blockcount;

#ifdef DEBUG
  fprintf(stdout, _("  SubBlocks: %d\n"),entry->SubEntriesCount);
#endif /* DEBUG */
}

void N7110_ReplyGetMemoryLocation(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{

  CurrentPhonebookEntry->Empty = true;
  CurrentPhonebookEntry->Group = 5;     /* 5 = no group as 6110 */
  CurrentPhonebookEntry->Name[0] = '\0';
  CurrentPhonebookEntry->Number[0] = '\0';
  CurrentPhonebookEntry->SubEntriesCount = 0;

#ifdef DEBUG
  fprintf(stdout, _("Message: Phonebook entry received:\n"));
#endif

  if( MessageBuffer[6] == 0x0f ) // not found
  {
#ifdef DEBUG
    fprintf(stdout, _("   Error %i\n"),MessageBuffer[10]);
    switch (MessageBuffer[10]) {
      case 0x34:fprintf(stdout,_("   Invalid phonebook location\n"));break;
      case 0x3b:fprintf(stdout,_("   Speed dial not assigned\n"));break;
      default  :fprintf(stdout,_("   Unknown.Please report\n"));break;
    }
#endif
    switch (MessageBuffer[10]) {
      case 0x34:CurrentPhonebookError = GE_INVALIDPHBOOKLOCATION;break;
      case 0x3b:CurrentPhonebookError = GE_INVALIDSPEEDDIALLOCATION;break;
      default  :CurrentPhonebookError = GE_UNKNOWN;
    }
    CurrentSpeedDialError=GE_INVALIDSPEEDDIALLOCATION;

  } else {
    CurrentPhonebookEntry->Location=MessageBuffer[13]+MessageBuffer[12]*256;

#ifdef DEBUG
    fprintf(stdout,_("  Location: %i\n"),CurrentPhonebookEntry->Location);
#endif

    N7110_DecodePhonebookFrame(CurrentPhonebookEntry,MessageBuffer+18,MessageLength-18);

    CurrentPhonebookError = GE_NONE;
  }
}

/* Routine to get specifed phone book location.  Designed to be called by
   application.  Will block until location is retrieved or a timeout/error
   occurs. */
GSM_Error N7110_GetMemoryLocation(GSM_PhonebookEntry *entry) {

  unsigned char req[] = {N7110_FRAME_HEADER, 0x07, 0x01, 0x01, 0x00, 0x01,
                         0x02, //memory type
                         0x05,
                         0x00, 0x00, //location
                         0x00, 0x00};

  CurrentPhonebookEntry = entry;

  req[9] = N7110_GetMemoryType(entry->MemoryType);
  req[10] = (entry->Location>>8);
  req[11] = entry->Location & 0xff;

  return NULL_SendMessageSequence
    (50, &CurrentPhonebookError, 14, 0x03, req);
}

void N7110_ReplyWritePhonebookLocation(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  // [12,13] = Location
  // [14] = Memory

  if( MessageBuffer[6] == 0x0f ) // ERROR
  {
#ifdef DEBUG
    /* I didn't find any error when the Text,Name or Number was too long
       My Phone 7110; NSE-5; SW 04.84 */
    switch( MessageBuffer[10] ) {
      case 0x3d: fprintf(stdout, _("Error: Wrong Entry Type.\n")); break;
      case 0x3e: fprintf(stdout, _("Error: Too much entries.\n")); break;
      default  : fprintf(stdout, _("Error: Unknown error (%u).\n"), MessageBuffer[10]); break;
    }
#endif /* DEBUG */
    CurrentPhonebookError = GE_NONE;
  } else {
#ifdef DEBUG
    fprintf(stdout, _("Message: Phonebook written correctly.\n"));
#endif /* DEBUG */

    CurrentPhonebookError = GE_NONE;
  }
}

int N7110_PackPBKBlock(int id, int size, int no, unsigned char *buf, unsigned char *block)
{
#ifdef DEBUG
  fprintf(stdout,_("Adding block id:%i,number:%i,length:%i\n"),id,no+1,size+6);
#endif

  *(block++) = id;
  *(block++) = 0;
  *(block++) = 0;
  *(block++) = size + 6;
  *(block++) = no + 1;

  memcpy(block, buf, size);
  block += size;

  *(block++) = 0;

  return (size + 6);
}

int N7110_EncodePhonebookFrame(unsigned char *req, GSM_PhonebookEntry entry, int *block2)
{
  int count=0, len, i, block=0;

  char string[500];

  /* Name */
  len = strlen(entry.Name);
  string[0] = len * 2;       // Length ot the string (without Termination)
  EncodeUnicode((string + 1), entry.Name, len);
  string[len * 2 + 1] = 0;   // Terminating 0
  count += N7110_PackPBKBlock(N7110_ENTRYTYPE_NAME, len * 2 + 2, block++, string, req + count);

  if (*entry.Number) {
    len = strlen(entry.Number);
    string[0] = N7110_ENTRYTYPE_NUMBER;
    string[1] = string[2] = string[3] = 0;
    string[4] = len * 2;     // length (without Termination)
    EncodeUnicode((string + 5), entry.Number, len);
    string[len * 2 + 5] = 0; // Terminating 0
    count += N7110_PackPBKBlock(N7110_ENTRYTYPE_NUMBER, len * 2 + 6, block++, string, req + count);
  }
 		
  /* Rest of the subentries */
  for (i = 0; i < entry.SubEntriesCount; i++) {
    len = strlen(entry.SubEntries[i].data.Number);
    if (entry.SubEntries[i].EntryType != GSM_Number) {
      string[0] = len * 2;     // length (without Termination)
      EncodeUnicode((string + 1), entry.SubEntries[i].data.Number, len);
      string[len * 2 + 1] = 0; // Terminating 0
      count += N7110_PackPBKBlock(entry.SubEntries[i].EntryType, len * 2 + 2, block++, string, req + count);
    } else {
      string[0] = entry.SubEntries[i].NumberType;
      string[1] = string[2] = string[3] = 0;
      string[4] = len * 2;     //length (without Termination)
      EncodeUnicode((string + 5), entry.SubEntries[i].data.Number, len);
      string[len * 2 + 5] = 0; // Terminating 0
      count += N7110_PackPBKBlock(N7110_ENTRYTYPE_NUMBER, len * 2 + 6, block++, string, req + count);
    }
  } 

  if (entry.Group != 5) {
    /* Group */
    string[0] = entry.Group + 1;
    string[1] = 0;
    count += N7110_PackPBKBlock(N7110_ENTRYTYPE_GROUP, 2, block++, string, req + count);
  }

  *block2=block;

  return count;
}
 
/* Routine to write phonebook location in phone. */
GSM_Error N7110_WritePhonebookLocation(GSM_PhonebookEntry *entry)
{
  unsigned char req[500] = {N7110_FRAME_HEADER, 0x0b, 0x00, 0x01, 0x01, 0x00, 0x00, 0x0c,
                                   0x00, 0x00,  /* memory type */
                                   0x00, 0x00,  /* location */
                                   0x00, 0x00, 0x00};
  int count = 18, blocks;

  if (entry->Name[0] != '\0' || entry->Number[0] != '\0') {
    req[11] = N7110_GetMemoryType(entry->MemoryType); 
    req[12] = (entry->Location >> 8);
    req[13] = entry->Location & 0xff;

//    if (entry->MemoryType == GMT_SM) entry->SubEntriesCount = 0; 

    count=count+N7110_EncodePhonebookFrame(req+18, *entry, &blocks);

    req[17]=blocks;

#ifdef DEBUG
    fprintf(stdout, _("Writing phonebook entry %s...\n"),entry->Name);
#endif

    return NULL_SendMessageSequence(50, &CurrentPhonebookError, count, 0x03, req);

  } else {

    /* empty name & number => we have to delete the phonebook record! */
    return N7110_DeletePhonebookLocation( entry );

  }  
}

void N7110_ReplyDeletePhonebookLocation(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
#ifdef DEBUG
  fprintf(stdout, _("Message: Phonebook entry deleted correctly\n"));
#endif /* DEBUG */
  CurrentPhonebookError = GE_NONE;
}

/* delete phonebookentry */
/* Not used in this moment */
GSM_Error N7110_DeletePhonebookLocation(GSM_PhonebookEntry *entry)
{
  unsigned char req[256] = {
    N7110_FRAME_HEADER, 0x0f, 0x00, 0x01, 0x04,
    0x00, 0x00, 0x0c, 0x01, 0xff,
    0x00, /* location low*/
    0x01, /* location high*/
    0x05, /* mem location low*/
    0x00, /* mem location high*/
    0x00, 0x00
  };

  req[12] = (entry->Location >> 8);
  req[13] = entry->Location & 0xff;
  req[14] = N7110_GetMemoryType(entry->MemoryType);
 
#ifdef DEBUG
  fprintf(stdout, _("Deleting phonebook entry at location %d...\n"),entry->Location);
#endif
  
  return NULL_SendMessageSequence(50, &CurrentPhonebookError, 18, 0x03, req);
}

/* for saving group logos only */
GSM_Error N7110_WriteGroupDataLocation(GSM_Bitmap *bitmap)
{
  unsigned char req[500] = {N6110_FRAME_HEADER, 0x0b, 0x00, 0x01, 0x01, 0x00, 0x00, 0x0c,
                            0x00, 0x10,  /* memory type */
                            0x00, 0x00,  /* location */
                            0x00, 0x00, 0x00};
  char string[500];
  int block=0, i;
  unsigned int count = 18;

  req[13] = bitmap->number + 1;

  /* Logo on/off */
  string[0] = bitmap->enabled?1:0;
  string[1] = 0;
  count += N7110_PackPBKBlock(N7110_ENTRYTYPE_LOGOON, 2, block++, string, req + count);

  /* Ringtone */
  string[0] = bitmap->ringtone;
  string[1] = 0;
  count += N7110_PackPBKBlock(N7110_ENTRYTYPE_RINGTONE, 2, block++, string, req + count);

  /* Number of group */
  string[0] = bitmap->number+1;
  string[1] = 0;
  count += N7110_PackPBKBlock(N7110_ENTRYTYPE_GROUP, 2, block++, string, req + count);

  /* Name */
  if (*bitmap->text) {
    i = strlen(bitmap->text);
    string[0] = i * 2 + 2;
    EncodeUnicode((string + 1), bitmap->text, i);
    string[i * 2 + 1] = 0;   // Terminating 0
    count += N7110_PackPBKBlock(N7110_ENTRYTYPE_NAME, i * 2 + 2, block++, string, req + count);
  }

  /* Logo */
  string[0] = bitmap->width;
  string[1] = bitmap->height;
  string[2] = 0;
  string[3] = 0;
  string[4] = bitmap->size;
  memcpy(string + 5, bitmap->bitmap, bitmap->size);
  count += N7110_PackPBKBlock(N7110_ENTRYTYPE_GROUPLOGO, bitmap->size + 5, block++, string, req + count);

  req[17] = block; //number of blocks

  return NULL_SendMessageSequence
    (50, &CurrentPhonebookError, count, 0x03, req);
}

GSM_Error N7110_GetSpeedDial(GSM_SpeedDial *entry)
{
  unsigned char req[] = {N7110_FRAME_HEADER, 0x07, 0x01, 0x01, 0x00, 0x01,
                         0x02, //memory type
                         0x05,
                         0x00, 0x00, //location
                         0x00, 0x00};

  GSM_PhonebookEntry entry2;
  GSM_Error error;

  CurrentPhonebookEntry = &entry2;

  CurrentSpeedDialEntry = entry;

  req[9] = N7110_MEMORY_SD;
  req[10] = (entry->Number>>8);
  req[11] = entry->Number & 0xff;

  error=NULL_SendMessageSequence
    (50, &CurrentSpeedDialError, 14, 0x03, req);

  /* Full compatibility with 6110 */
  if (error==GE_INVALIDSPEEDDIALLOCATION) {
    entry->Location=0;
    entry->MemoryType=GMT_MT;
    return GE_NONE;
  } else return error;
}

/* Experimental ! */
GSM_Error N7110_SetSpeedDial(GSM_SpeedDial *entry)
{
  unsigned char req[500] = {N6110_FRAME_HEADER, 0x0b, 0x00, 0x01, 0x01, 0x00, 0x00, 0x0c,
                            0x00, 0x0e,  /* memory type */
                            0x00, 0x00,  /* location */
                            0x00, 0x00, 0x00};
  char string[500];
  int block=1;
  unsigned int count = 18;

  req[13] = entry->Number;

  string[0]= 0xff;
  string[1]= entry->Location/256;
  string[2]= entry->Location%256;
  string[3]= 0x05;
  string[4]= string[5] = 0;
  count += N7110_PackPBKBlock(N7110_ENTRYTYPE_SPEEDDIAL, 6, block++, string, req + count);

  req[17] = block - 1; //number of blocks

  return NULL_SendMessageSequence
    (50, &CurrentPhonebookError, count, 0x03, req);
}

/* Set a bitmap or welcome-note */
GSM_Error N7110_SetBitmap(GSM_Bitmap *Bitmap)
{
  unsigned char reqStartup[1000] = { N7110_FRAME_HEADER,
                                     0xec, 0x15, // Startup Logo
                                     0x00, 0x00, 0x00, 0x04,
                                     0xc0, 0x02, 0x00,
                                     0x00,       // Bitmap height
                                     0xc0, 0x03, 0x00,
                                     0x00,       // Bitmap width
                                     0xc0, 0x04, 0x03, 0x00
                                   };            // Bitmap following
  unsigned char reqOp[1000] = { N7110_FRAME_HEADER,
                                0xa3, 0x01,
                                0x00,              // logo disabled
                                0x00, 0xf0, 0x00,  // network code (000 00)
                                0x00 ,0x04,
                                0x08,              // length of rest
                                0x00, 0x00,        // Bitmap width / height
                                0x00,
                                0x00,              // Bitmap size
                                0x00, 0x00
                              };                   // Bitmap following

  /* Picture Images */
  unsigned char req2[7] = { N6110_FRAME_HEADER, 0x96,0x00,0x0f,0x07 };
  unsigned char req3[9] = { 0x09, 0x11, 0x19, 0x21, 0x29, 0x31, 0x39, 0x41, 0x49 };
  unsigned char req4[500] = { N6110_FRAME_HEADER, 0x50, 0x07,
                                0x00,  //location
                                0x00,0x00, //index
                                0x07};

  unsigned char req5[120] = {0x00, 0x01, 0x01, 0xec, 0x02,0x00};

  unsigned char req6[]= {0x00,0x01,0x00,0xaf,0x00};

  u16 count,i;

  int timeout=60;
  
  GSM_Error error;

  CurrentSetBitmapError=GE_BUSY;
  
  switch (Bitmap->type) {
  case GSM_WelcomeNoteText:

    EncodeUnicode (req5+5, Bitmap->text, strlen(Bitmap->text));
    count=5+strlen(Bitmap->text)*2;
    req5[count++]=0x00;
    req5[count++]=0x00;
    Protocol->SendMessage(count, 0x7a, req5);
    break;

  case GSM_DealerNoteText:
    CurrentSetBitmapError = GE_NOTIMPLEMENTED;
    break;

  case GSM_StartupLogo:
  case GSM_7110StartupLogo:
  case GSM_6210StartupLogo:
    reqStartup[12] = Bitmap->height;
    reqStartup[16] = Bitmap->width;
    count = 21;
    memcpy(reqStartup+count,Bitmap->bitmap,Bitmap->size);
    Protocol->SendMessage(count+Bitmap->size, 0x7a, reqStartup);
    break;

  case GSM_OperatorLogo:
  case GSM_7110OperatorLogo:
    count = 18;
    
    /* set logo */
    if (strcmp(Bitmap->netcode,"000 00")) {
      reqOp[5] = 0x01;      // Logo enabled
      EncodeNetworkCode(reqOp+6, Bitmap->netcode);
      reqOp[11] = 8+GSM_GetBitmapSize(Bitmap);
      reqOp[12]=Bitmap->width;
      reqOp[13]=Bitmap->height;
      reqOp[15]=GSM_GetBitmapSize(Bitmap);
      memcpy(reqOp+count,Bitmap->bitmap,Bitmap->size);
      count += Bitmap->size;
    } else {
      for (i=0;i<5;i++) {
        req6[4]=i;
        error=NULL_SendMessageSequence
          (50, &CurrentSetBitmapError, 5, 0x0a, req6);
        if (error!=GE_NONE) return error;
      }
    }
    Protocol->SendMessage(count, 0x0a, reqOp);
    break;

  case GSM_CallerLogo:
    CurrentSetBitmapError = N7110_WriteGroupDataLocation(Bitmap);
    break;

  case GSM_PictureImage:
    CurrentGetBitmap=Bitmap;
    PictureImageNum=0;
    count=0;
    while (count!=9) {
      req2[4]=req3[count];
      PictureImageLoc=req3[count];
      count++;
      if (NULL_SendMessageSequence (50, &CurrentGetBitmapError, 7, 0x14, req2)!=GE_NONE) break;
      if (PictureImageNum==Bitmap->number+1) break;
    }
    if (PictureImageNum!=Bitmap->number+1) {
      req4[5]=0x21;
      req4[6]=0;
      req4[7]=0;
    } else {
      req4[5]=PictureImageLoc;
      req4[6]=PictureImageIndex/256;
      req4[7]=PictureImageIndex%256;
    }

    /* Cleaning */
    for (i=0;i<36;i++) req4[i+9]=0;

    count=8;
    if (strlen(Bitmap->text)==0) {
      count+=2 ;req4[count]=0x0c;
      count+=2 ;req4[count]=0x0d;
      count+=2 ;req4[count]=0x0e;
      count+=2 ;req4[count]=0x0f;
      count+=2 ;req4[count]=0x10;
      count+=2 ;req4[count]=0x11;
      count+=23;req4[count]=0x02;
      count++  ;req4[count]=0x01;
      count+=2;
    } else {
      count+=2 ;req4[count]=0x54;
      count++  ;req4[count]=0xd4;
      count++  ;req4[count]=0x0d;
      count+=2 ;req4[count]=0x0e;
      count+=2 ;req4[count]=0x0f;
      count+=2 ;req4[count]=0x10;
      count+=2 ;req4[count]=0x11;
      count+=21;req4[count]=0x01;
      count+=3 ;req4[count]=0x01;
      count+=2;
    }

    req4[count++]=0x01;
    req4[count++]=Bitmap->width;
    req4[count++]=Bitmap->height;
    req4[count++]=Bitmap->size/256;
    req4[count++]=Bitmap->size%256;
    memcpy(req4+count,Bitmap->bitmap,Bitmap->size);
    count += Bitmap->size;

    if (strlen(Bitmap->text)!=0) {
      req4[count] = strlen(Bitmap->text);
      GSM_PackSevenBitsToEight(0, Bitmap->text, req4+count+1);
      count=count+req4[count];
    } else {
      req4[count++]=0x00;
    }
    req4[count++]=0x00;

    CurrentSetBitmapError = GE_BUSY;

    Protocol->SendMessage(count, 0x14, req4);

  case GSM_None:
    return GE_NONE;
  }

  /* Wait for timeout or other error. */
  while (timeout != 0 && CurrentSetBitmapError == GE_BUSY ) {
          
    if (--timeout == 0)
      return (GE_TIMEOUT);
                    
    usleep (100000);
  }

  if ((Bitmap->type==GSM_OperatorLogo || Bitmap->type==GSM_7110OperatorLogo)
     && CurrentSetBitmapError==GE_NONE && !strcmp(Bitmap->netcode,"000 00")) {
    return N6110_Reset(0x03);
  }

  return CurrentSetBitmapError;
}

/* Get a bitmap from the phone */
GSM_Error N7110_GetBitmap(GSM_Bitmap *Bitmap)
{
  unsigned char req[10] = { N6110_FRAME_HEADER };

  /* Picture Images */
  unsigned char req2[7] = { N6110_FRAME_HEADER, 0x96,0x00,0x0f,0x07 };
  unsigned char req3[9] = { 0x09, 0x11, 0x19, 0x21, 0x29, 0x31, 0x39, 0x41, 0x49 };
  unsigned char req4[9] = { N6110_FRAME_HEADER, 0x07,
                                0x00,  //location
                                0x00,0x00, //index
                                0x00, 0x64};

  /* Welcome note */
  unsigned char req5[] = {0x00, 0x01, 0x01, 0xee, 0x02};

  u8 count=3;
  int timeout;
  GSM_PhonebookEntry entry;

  CurrentGetBitmapError = GE_BUSY;

  CurrentGetBitmap=Bitmap;

  switch (CurrentGetBitmap->type) {
  case GSM_StartupLogo:
  case GSM_7110StartupLogo:
  case GSM_6210StartupLogo:
    req[count-1]=0x01;  /* change FRAME_HEADER */
    req[count++]=0xee;  /* to get 0x01ee */
    req[count++]=0x15;
    Protocol->SendMessage(count, 0x7a, req);
    break;

  case GSM_WelcomeNoteText:

    Protocol->SendMessage(5, 0x7a, req5);
    break;

  case GSM_DealerNoteText:

    return GE_NOTIMPLEMENTED;
    
  case GSM_OperatorLogo:
  case GSM_7110OperatorLogo:
    req[count++]=0x70; /* NetworkStatus */
    Protocol->SendMessage(count, 0x0a, req);
    break;

  case GSM_CallerLogo:
    entry.MemoryType = GMT_CG;
    entry.Location = Bitmap->number+1;
    *Bitmap->text=0;
    Bitmap->ringtone=0xFF;
    Bitmap->enabled=true;
    CurrentGetBitmapError = N7110_GetMemoryLocation(&entry);
    CurrentGetBitmap=NULL;
    if (entry.Location!=Bitmap->number+1) return GE_UNKNOWN;
    if (!strcmp(Bitmap->text,"")) {
      switch(Bitmap->number) {
        case 0:strcpy(Bitmap->text,"Family\0");break;
        case 1:strcpy(Bitmap->text,"VIP\0");break;
        case 2:strcpy(Bitmap->text,"Friends\0");break;
        case 3:strcpy(Bitmap->text,"Colleagues\0");break;
        case 4:strcpy(Bitmap->text,"Other\0");break;
      }
    }
    break;

  case GSM_PictureImage:

    PictureImageNum=0;
    count=0;
    while (count!=9) {
      req2[4]=req3[count];
      PictureImageLoc=req3[count];
      count++;
      if (NULL_SendMessageSequence (50, &CurrentGetBitmapError, 7, 0x14, req2)!=GE_NONE)
        return CurrentGetBitmapError;
      if (PictureImageNum==Bitmap->number+1) break;
    }
    if (CurrentGetBitmapError!=GE_NONE) return CurrentGetBitmapError;

    req4[4]=PictureImageLoc;
    req4[5]=PictureImageIndex/256;
    req4[6]=PictureImageIndex%256;

    CurrentGetBitmapError = GE_BUSY;

    Protocol->SendMessage(9, 0x14, req4);

    break;
  
  default:
    return GE_NOTSUPPORTED;
  }

  timeout=150;

  while (timeout != 0 && CurrentGetBitmapError == GE_BUSY) {

    if (--timeout == 0)
      return (GE_TIMEOUT);

    usleep (100000);
  }

  CurrentGetBitmap=NULL;

  return (CurrentGetBitmapError);
}

int ReturnBinRingLocation()
{
  char model[64];

  while (GSM->GetModel(model) != GE_NONE) sleep(1);

  if (strcmp(model,"NSE-5") == 0) return 0x74; //first 0x74 //7110
  if (strcmp(model,"NPE-3") == 0) return 0x89; //first is 0x89; //6210
  if (strcmp(model,"NHM-3") == 0) return 0x89; //quess for 6250
    
  return 0;
}

/*
this works only for 6210, the 7110 needs upload to a location ? 
*/
GSM_Error N7110_SetRingTone(GSM_Ringtone *ringtone, int *maxlength)
{  

  /* Buffer for preview uploading */
  unsigned char buffer[1000] = {0x7C, 0x01, 0x00, 0x0D,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00,
                                0x00  // Length
                               };
  int size=FB61_MAX_RINGTONE_FRAME_LENGTH;

  GSM_NetworkInfo NetworkInfo;

  char req[4000] = {N7110_FRAME_HEADER, 0x1F, 0x00,
                    0x87,  // Location
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x02, 0xFC, 0x09, 0x00, 0x0A, 0x01
                   };

  /* Info from Till Toenshoff [till@uni.de]

     One improvement - for looping you can use an alternative header
     normal
     0x02, 0xFC, 0x09, 0x00, 0x0A, 0x01

     loop
     0x02, 0xFC, 0x09, 0x00, 0x05, 0xLL, 0x0A, 0x01

     LL=0x01-0x10

     0x01=loop once
     [...]
     0x10=loop infinite
  */
  char tail[] = {0x40, 0x7D, 0x40, 0x5C, 0x0A, 0xFE, 0x40,
                 0x20, 0x40, 0x7D, 0x40, 0x37, 0x0A, 0xFE,
                 0x0A, 0x0A, 0x40, 0x32, 0x07, 0x0B         // 20 Bytes tail
                };

//{ "c", "c#", "d", "d#", "e",      "f", "f#", "g", "g#", "a", "a#", "h" };
  char N7110_notes[14] =
  { 0,    1,    2,   3,    4,   4,   5,   6,    7,   8,    9,  10 ,   11,   11 };

  int current = 6, i, note, lastnote = 0, duration;

  /* Preview uploading */
  if (ringtone->location==255) {
    *maxlength=GSM_PackRingtone(ringtone, buffer+11, &size);
    buffer[10] = size;
    Protocol->SendMessage(size+11, 0x00, buffer);
    GSM->GetNetworkInfo(&NetworkInfo); //need to make something
    sleep(1);
    return GE_NONE; //no answer from phone
  }

  req[5]=ReturnBinRingLocation()+ringtone->location;

  EncodeUnicode (req+current,ringtone->name ,strlen(ringtone->name));

  current = 43;

  for (i=0; i<ringtone->NrNotes; i++) {

    if (ringtone->notes[i].note == 255)
      note = 0x40;
    else
      note = 114+12*((ringtone->notes[i].note/14)%4) + N7110_notes[ringtone->notes[i].note%14];

    duration = 60000*ringtone->notes[i].duration/ringtone->notes[i].tempo/256;  // in 8 ms ticks of 7110

    switch (ringtone->notes[i].style) {
      case StaccatoStyle:
        if (duration) {
          req[current++] = note;   // note only for one tick
          req[current++] = 1;
          duration--;
        }
        note = 0x40;               // rest pause
      case NaturalStyle:
        if (note != 0x40 && duration) {
          req[current++] = 0x40;
          req[current++] = 1;      // small pause between notes
          duration--;
        }
      default:
        if (note != 0x40 && note == lastnote && duration) {
          req[current++] = 0x40;
          req[current++] = 1;      // small pause between same notes
          duration--;
        }

        while (duration > 125) {
          req[current++] = note;
          req[current++] = 125;
          duration -= 125;
        }
        req[current++] = note;
        req[current++] = duration;
    }
    lastnote = note;
  }

  for (i = 0; i < sizeof(tail); i++)
    req[current++] = tail[i];

  Protocol->SendMessage(current, 0x1F, req);

  sleep(1);  

  return GE_NONE; //no answer from phone
}

void N7110_ReplyGetBinRingtone(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  int i, tmp;

  switch (MessageBuffer[3]) {

  case 0x23:

    tmp=0;i=4;
    while (MessageBuffer[i]!=0 || MessageBuffer[i+1]!=0) {
      tmp++;
      i=i+2;
    }

    DecodeUnicode(CurrentGetBinRingtone->name,MessageBuffer+6,tmp);

#ifdef DEBUG
    fprintf(stdout, _("Message: Received ringtone \"%s\"\n"),CurrentGetBinRingtone->name);
#endif /* DEBUG */

    CurrentGetBinRingtone->frame[0]=0x00;
    CurrentGetBinRingtone->frame[1]=0x00;
    CurrentGetBinRingtone->frame[2]=0x0c;
    CurrentGetBinRingtone->frame[3]=0x01;
    CurrentGetBinRingtone->frame[4]=0x2c;

    memcpy(CurrentGetBinRingtone->frame+5,CurrentGetBinRingtone->name,strlen(CurrentGetBinRingtone->name));

    CurrentGetBinRingtone->length=5+strlen(CurrentGetBinRingtone->name);

    CurrentGetBinRingtone->frame[CurrentGetBinRingtone->length++]=0x00;
    CurrentGetBinRingtone->frame[CurrentGetBinRingtone->length++]=0x00;

    /* Looking for end */
    i=37;
    while (true) {
      if (MessageBuffer[i]==0x07 && MessageBuffer[i+1]==0x0b) {
  	i=i+2;break;
      }
      if (MessageBuffer[i]==0x0e && MessageBuffer[i+1]==0x0b) {
  	i=i+2;break;
      }
      i++;
      if (i==MessageLength) break;
    }
	  
    /* Copying frame */
    memcpy(CurrentGetBinRingtone->frame+CurrentGetBinRingtone->length,MessageBuffer+37,i-37);
    CurrentGetBinRingtone->length=CurrentGetBinRingtone->length+i-37;
      
    CurrentBinRingtoneError=GE_NONE;
    break;

  case 0x24:

#ifdef DEBUG
    fprintf(stdout, _("Message: Received empty ringtone\n"));
#endif /* DEBUG */

    CurrentBinRingtoneError=GE_INVALIDRINGLOCATION;
    break;
  }
}

GSM_Error N7110_GetBinRingTone(GSM_BinRingtone *ringtone)
{

  unsigned char req[6] = { N6110_FRAME_HEADER, 0x22, 0x00, 0x00 };

  req[2]=0x01;

  req[5]=ReturnBinRingLocation()+ringtone->location;

  CurrentGetBinRingtone=ringtone;

  return NULL_SendMessageSequence
    (50, &CurrentBinRingtoneError, 6, 0x1f, req);

}

GSM_Error N7110_SetBinRingTone(GSM_BinRingtone *ringtone)
{
  unsigned char req[1000] = { N6110_FRAME_HEADER, 0x1f, 0x00, 0x00 };

  GSM_NetworkInfo NetworkInfo;
  
  int i;

  req[2]=0x01;

  req[5]=ReturnBinRingLocation()+ringtone->location;

  for (i=6;i<35;i++) req[i]=0x00;

  i=6;

  EncodeUnicode (req+i,ringtone->frame+5 ,strlen(ringtone->frame+5));

  i=5;

  while (ringtone->frame[i]!=0 || ringtone->frame[i+1]!=0) i++;

  memcpy(req+35,ringtone->frame+i,ringtone->length-i);

  Protocol->SendMessage(35+ringtone->length-i, 0x1f, req);  

  GSM->GetNetworkInfo(&NetworkInfo); //need to make something    
  
  return GE_NONE; //no answer from phone
}

void N7110_ReplyIncomingSMS(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
  
  GSM_SMSMessage NullSMS;
    
#ifdef DEBUG
  fprintf(stdout,_("Message: Incoming SMS\n"));
#endif /* DEBUG */

  GSM_DecodeNokiaSMSFrame(&NullSMS, MessageBuffer+5, MessageLength-5);

#ifdef DEBUG
  fprintf(stdout, _("\n"));      
#endif /* DEBUG */      
}

void N7110_Dispatch0x0AMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  int count;

#ifdef DEBUG  
  char name[100];
#endif
  
  GSM_NetworkInfo NullNetworkInfo;

  switch (MessageBuffer[3]) {

  case 0xb0:

#ifdef DEBUG
    fprintf(stdout, _("Message: Clearing operator logo msg\n"));
#endif
    CurrentSetBitmapError=GE_NONE;
    break;

  case 0x71:

    /* Make sure we are expecting NetworkInfo frame */
    if ((CurrentNetworkInfo && CurrentNetworkInfoError == GE_BUSY) ||
        (CurrentGetBitmap && CurrentGetBitmapError == GE_BUSY)) {
#ifdef DEBUG
      fprintf(stdout, _("Message: Network informations and operator logo:\n"));
#endif
    } else {
#ifdef DEBUG
      fprintf(stdout, _("Message: Network informations not requested, but received:\n"));
#endif
    }

    sprintf(NullNetworkInfo.NetworkCode, "%x%x%x %x%x", MessageBuffer[14] & 0x0f, MessageBuffer[14] >>4, MessageBuffer[15] & 0x0f, MessageBuffer[16] & 0x0f, MessageBuffer[16] >>4);

    sprintf(NullNetworkInfo.CellID, "%02x%02x", MessageBuffer[10], MessageBuffer[11]);

    sprintf(NullNetworkInfo.LAC, "%02x%02x", MessageBuffer[12], MessageBuffer[13]);

#ifdef DEBUG
    fprintf(stdout, _("   CellID: %s\n"), NullNetworkInfo.CellID);
    fprintf(stdout, _("   LAC: %s\n"), NullNetworkInfo.LAC);
    fprintf(stdout, _("   Network code: %s\n"), NullNetworkInfo.NetworkCode);
    fprintf(stdout, _("   Network name for gnokii: %s (%s)\n"),
                  GSM_GetNetworkName(NullNetworkInfo.NetworkCode),
                  GSM_GetCountryName(NullNetworkInfo.NetworkCode));

    DecodeUnicode(name,MessageBuffer+18,MessageBuffer[17]);
    fprintf(stdout, _("   Network name for phone: %s\n"),name);

    fprintf(stdout, _("   Status: "));
    switch (MessageBuffer[8]) {
      case 0x01: fprintf(stdout, _("home network selected")); break;
      case 0x02: fprintf(stdout, _("roaming network")); break;
      case 0x03: fprintf(stdout, _("requesting network")); break;
      case 0x04: fprintf(stdout, _("not registered in the network")); break;
      default:   fprintf(stdout, _("unknown"));
    }

    fprintf(stdout, "\n");

    fprintf(stdout, _("   Network selection: %s\n"), MessageBuffer[9]==1?_("manual"):_("automatic"));
#endif /* DEBUG */

    /* Make sure we are expecting NetworkInfo frame */
    if (CurrentNetworkInfo && CurrentNetworkInfoError == GE_BUSY) {
      *CurrentNetworkInfo=NullNetworkInfo;
      CurrentNetworkInfoError = GE_NONE;
    }

    /* Make sure we are expecting an operator logo */
    if (CurrentGetBitmap && CurrentGetBitmapError == GE_BUSY) {

      strcpy(CurrentGetBitmap->netcode,NullNetworkInfo.NetworkCode);

#ifdef DEBUG
      if (MessageBuffer[4] == 0x02)
  	fprintf(stdout, _("Message: Operator Logo for %s (%s) network received.\n"),
	                   CurrentGetBitmap->netcode,
	                   GSM_GetNetworkName(CurrentGetBitmap->netcode));
      else
        fprintf(stdout, _("Message: No Operator Logo for %s (%s) network received.\n"),
                           CurrentGetBitmap->netcode,
                           GSM_GetNetworkName(CurrentGetBitmap->netcode));
#endif

      CurrentGetBitmap->type=GSM_7110OperatorLogo;
      if (MessageBuffer[4] == 0x02) {   /* logo present */
        count = 7;
        count += MessageBuffer[count];  /* skip network info */
        count ++;
        CurrentGetBitmap->width=MessageBuffer[count++];  /* 78 */
        CurrentGetBitmap->height=MessageBuffer[count++]; /* 21 */
        count+=4;
        CurrentGetBitmap->size=GSM_GetBitmapSize(CurrentGetBitmap); /* packed size */
        memcpy(CurrentGetBitmap->bitmap,MessageBuffer+count,CurrentGetBitmap->size);
      } else {
        CurrentGetBitmap->width=78;
        CurrentGetBitmap->height=21;
        CurrentGetBitmap->size=GSM_GetBitmapSize(CurrentGetBitmap); /* packed size */
        memset(CurrentGetBitmap->bitmap,0,CurrentGetBitmap->size);
      }
      CurrentGetBitmapError=GE_NONE;
    }

    break;

  case 0x82:
#ifdef DEBUG
    fprintf(stdout, _("Message: Network Level received:\n"));

    fprintf(stdout, _("   Network Level: %d\n"), MessageBuffer[4]);
#endif /* DEBUG */

    CurrentRFLevel=MessageBuffer[4];
    break;

  case 0xa4:

#ifdef DEBUG
    fprintf(stdout, _("Message: Operator logo set correctly.\n"));
#endif
    CurrentSetBitmapError = GE_NONE;      
    break;

  default:
#ifdef DEBUG
    fprintf(stdout, _("Message: Unknown message of type 0x0a\n"));
#endif /* DEBUG */
    AppendLogText("Unknown msg\n",false);
    break;	/* Visual C Don't like empty cases */
  }
}

void N7110_Dispatch0x14Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  int i, tmp;
  GSM_ETSISMSMessage ETSI;

  switch (MessageBuffer[3]) {

  /* We have requested invalid or empty location. */
  case 0x09:

#ifdef DEBUG
    fprintf(stdout, _("Message: SMS reading failed\n"));
#endif /* DEBUG */

   CurrentSMSMessageError = GE_INVALIDSMSLOCATION;
   CurrentGetBitmapError=GE_UNKNOWN;
   break;

  /* getsms or picture image */    
  case 0x08:

    if (MessageBuffer[8]==0x07) {

      if (CurrentGetBitmap!=NULL) {

        for (i=0;i<MessageBuffer[9]+1;i++)
          ETSI.SMSCNumber[i]=MessageBuffer[i+9];

        for (i=0;i<((MessageBuffer[22]+1)/2+1)+1;i++)
           ETSI.Number[i]=MessageBuffer[22+i];

#ifdef DEBUG
        fprintf(stdout, _("Message: Picture Image received\n"));
        fprintf(stdout, _("   SMS center number: %s\n"), GSM_UnpackSemiOctetNumber(ETSI.SMSCNumber,false));
        fprintf(stdout, _("   Sender number: %s\n"), GSM_UnpackSemiOctetNumber(ETSI.Number,true));
#endif /* DEBUG */

        strcpy(CurrentGetBitmap->Sender,GSM_UnpackSemiOctetNumber(ETSI.Number,true));

        CurrentGetBitmap->width=MessageBuffer[47];
        CurrentGetBitmap->height=MessageBuffer[48];
        CurrentGetBitmap->size=GSM_GetBitmapSize(CurrentGetBitmap);
      
        memcpy(CurrentGetBitmap->bitmap,MessageBuffer+51,CurrentGetBitmap->size);

        tmp=GSM_UnpackEightBitsToSeven(0, 121, 121, MessageBuffer+52+CurrentGetBitmap->size,
                                       CurrentGetBitmap->text);

        CurrentGetBitmap->text[MessageBuffer[51+CurrentGetBitmap->size]]=0;
 
        CurrentGetBitmapError=GE_NONE;
	  
      } else {
#ifdef DEBUG
        fprintf(stdout, _("Message: Picture Image received, but not reqested\n"));
#endif /* DEBUG */

        CurrentSMSMessageError = GE_SMSTOOLONG;
      }
	
      break;
    } else {
      /* sms message */
      /* status in byte 4 */
      CurrentSMSMessage->Status = MessageBuffer[4];

      CurrentSMSMessage->Type = GST_SMS;
      if (MessageBuffer[8]==0x01) CurrentSMSMessage->Type = GST_DR;
      
      CurrentSMSMessage->MessageNumber = CurrentSMSMessage->Location;
	
      /* RTH FIXME: old folder stuff for xgnokii */     
      CurrentSMSMessage->folder = (MessageBuffer[5] / 8)-1;
      /* if unreadMessage from folder 0, set INBOX folder */
      if (CurrentSMSMessage->folder==-1) CurrentSMSMessage->folder = 0;
#ifdef DEBUG
      fprintf(stdout, _("Message: SMS Message received: Type: %i Folder: %i Location: %i\n")
		,CurrentSMSMessage->Type, CurrentSMSMessage->folder,  MessageBuffer[7]);
#endif /* DEBUG */

      GSM_DecodeNokiaSMSFrame(CurrentSMSMessage, MessageBuffer+9, MessageLength-9);    

      /* Signal no error to calling code. */
      CurrentSMSMessageError = GE_NONE;
      break;
    }
  case 0x51:
#ifdef DEBUG
    fprintf(stdout, _("Message: Picture Image saved OK\n"));
    fprintf(stdout, _("  folder  : %02x\n"),MessageBuffer[4]);
    fprintf(stdout, _("  location: %02x\n"),MessageBuffer[5]*256+MessageBuffer[6]);
#endif /* DEBUG */
    CurrentSetBitmapError=GE_NONE;
    break;

  case 0x97:

#ifdef DEBUG
    fprintf(stdout, _("Message: Received index for Picture Images\n"));
#endif /* DEBUG */

    for (i=1;i<MessageBuffer[4]*256+MessageBuffer[5]+1;i++) {
      PictureImageNum++;
      if (PictureImageNum==CurrentGetBitmap->number+1) {
        PictureImageIndex=MessageBuffer[4+i*2]*256+MessageBuffer[5+i*2];
        break;
      }
    }

    CurrentGetBitmapError=GE_NONE;    
    CurrentSetBitmapError=GE_NONE;    

    break;
    
  default:

#ifdef DEBUG
    fprintf(stdout, _("Message: Unknown message of type 14\n"));
#endif /* DEBUG */
    AppendLogText("Unknown msg\n",false);
    break;	/* Visual C Don't like empty cases */
  }
}

void N7110_Dispatch0x17Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  switch (MessageBuffer[3]) {

  case 0x03:

#ifdef DEBUG
    fprintf(stdout, _("Message: Battery status received:\n"));

    fprintf(stdout, _("   Battery Level: %d\n"), MessageBuffer[5]);
#endif /* DEBUG */

    CurrentBatteryLevel=MessageBuffer[5];
    break;

  default:

#ifdef DEBUG
    fprintf(stdout, _("Message: Unknown message of type 0x17\n"));
#endif /* DEBUG */
    AppendLogText("Unknown msg\n",false);
    break;	/* Visual C Don't like empty cases */
  }
}

void N7110_Dispatch0x7AMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  int i, tmp;

  switch (MessageBuffer[3]) {

  /* Setting setting request */
  case 0xeb:

    switch (MessageBuffer[4]) {
      case 0x02:

#ifdef DEBUG
        fprintf(stdout, _("Message: Startup text set correctly.\n"));
#endif
        CurrentSetBitmapError = GE_NONE;
        break;

      case 0x15:
#ifdef DEBUG
        fprintf(stdout, _("Message: Startup logo set correctly.\n"));
#endif
        CurrentSetBitmapError = GE_NONE;
        break;
    }

    break;

  /* Setting received */
  case 0xed:

    switch (MessageBuffer[4]) {
      case 0x02:

#ifdef DEBUG
        fprintf(stdout, _("Message: Startup text received.\n"));
#endif

        tmp=0;i=6;
        while (MessageBuffer[i]!=0||MessageBuffer[i+1]!=0) {
          i=i+2;
          tmp++;
        }

        DecodeUnicode (CurrentGetBitmap->text, MessageBuffer+6, tmp);

#ifdef DEBUG
        fprintf(stdout, _("   Text: \"%s\"\n"),CurrentGetBitmap->text);
#endif

        CurrentGetBitmapError = GE_NONE;
        break;

      case 0x15:
#ifdef DEBUG
        if (CurrentGetBitmap && CurrentGetBitmapError == GE_BUSY)
          fprintf(stdout, _("Message: Startup logo received.\n"));
        else
          fprintf(stdout, _("Message: Startup logo not requested, but received.\n"));
#endif

        /* Make sure we are expecting a startup logo */
        if (CurrentGetBitmap && CurrentGetBitmapError == GE_BUSY) {
          CurrentGetBitmap->height=MessageBuffer[13];  /* 96 */
          CurrentGetBitmap->width=MessageBuffer[17];   /* 60/65 */
          CurrentGetBitmap->type=GSM_7110StartupLogo;
          if (CurrentGetBitmap->width==60) CurrentGetBitmap->type=GSM_6210StartupLogo;
          CurrentGetBitmap->size=GSM_GetBitmapSize(CurrentGetBitmap); /* unpacked size */
          if (CurrentGetBitmap->size > sizeof(CurrentGetBitmap->bitmap))
            CurrentGetBitmap->size=sizeof(CurrentGetBitmap->bitmap);
          memcpy(CurrentGetBitmap->bitmap,MessageBuffer+22,CurrentGetBitmap->size);
          CurrentGetBitmapError=GE_NONE;
        }
        break;
      case 0x1c:
#ifdef DEBUG
        fprintf(stdout, _("Message: security code \""));

	for (i=6;i<11;i++) {
          fprintf(stdout, _("%c"),MessageBuffer[i]);
        }
	  
	fprintf(stdout,_("\"received.\n"));
#endif
        strncpy(CurrentSecurityCode->Code,MessageBuffer+6,6);
	  
	CurrentSecurityCodeError=GE_NONE;	  
	break;
    }
    break;
      
  default:
#ifdef DEBUG
    fprintf(stdout, _("Unknown message of type 0x7a.\n"));
#endif
    AppendLogText("Unknown msg\n",false);
    break;	/* Visual C Don't like empty cases */
  }
}

void N7110_DispatchMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType)
{
  bool unknown=false;

  /* Switch on the basis of the message type byte */
  switch (MessageType) {

  /* Call information */
  /* Note, we use N6110_Dispatch0x40Message, but only some of 0x01
     old msg are available in new phones - other returns generally only
     errors */
  case 0x01:
	  
    N6110_Dispatch0x01Message(MessageLength, MessageBuffer, MessageType);
    break;

  /* SMS handling */
  case 0x02:
    switch (MessageBuffer[3]) {
      case 0x02:
      case 0x03:N6110_ReplySendSMSMessage(MessageLength,MessageBuffer,MessageType);break;
      case 0x0e:
      case 0x0f:N7110_ReplyEnableIncomingSMSInfo(MessageLength,MessageBuffer,MessageType);break;
      case 0x11:N7110_ReplyIncomingSMS          (MessageLength,MessageBuffer,MessageType);break;
      case 0x21:N6110_ReplySetCellBroadcast     (MessageLength, MessageBuffer, MessageType);break;
      case 0x23:N6110_ReplyReadCellBroadcast    (MessageLength, MessageBuffer, MessageType);break;
      case 0x31:N6110_ReplySetSMSCenter         (MessageLength,MessageBuffer,MessageType);break;
      case 0x34:
      case 0x35:N6110_ReplyGetSMSCenter(MessageLength,MessageBuffer,MessageType);break;
      default  :unknown=true;break;
    }
    break;

  /* Phonebook and speed dials */
  case 0x03:
    switch (MessageBuffer[3]) {
      case 0x04:N7110_ReplyGetMemoryStatus        (MessageLength,MessageBuffer,MessageType);break;
      case 0x08:N7110_ReplyGetMemoryLocation      (MessageLength,MessageBuffer,MessageType);break;
      case 0x10:N7110_ReplyDeletePhonebookLocation(MessageLength,MessageBuffer,MessageType);break;
      case 0x0C:N7110_ReplyWritePhonebookLocation (MessageLength,MessageBuffer,MessageType);break;
      default  :unknown=true;break;
    }
    break;

  /* This is call forwarding stuff */
  case 0x06:
    switch (MessageBuffer[3]) {
      case 0x02:
      case 0x03:N6110_ReplyCallDivert    (MessageLength,MessageBuffer,MessageType);break;
      default  :N6110_Dispatch0x06Message(MessageLength,MessageBuffer,MessageType);break;
    }
    break;

  /* Network and operator logo */
  case 0x0a:

    N7110_Dispatch0x0AMessage(MessageLength, MessageBuffer, MessageType);
    break;

  /* Calendar notes handling */
  case 0x13:
    switch (MessageBuffer[3]) {
      case 0x02:
      case 0x04:
      case 0x06:
      case 0x08:N7110_ReplyWriteCalendarNote   (MessageLength,MessageBuffer,MessageType);break;
      case 0x0c:N7110_ReplyDeleteCalendarNote  (MessageLength,MessageBuffer,MessageType);break;
      case 0x1A:N7110_ReplyGetCalendarNote     (MessageLength,MessageBuffer,MessageType);break;
      case 0x32:N7110_ReplyFirstCalendarFreePos(MessageLength,MessageBuffer,MessageType);break;
      case 0x3b:N7110_ReplyGetCalendarNotesInfo(MessageLength,MessageBuffer,MessageType);break;
      default  :unknown=true;break;
    }
    break;

  /* SMS stuff */
  case 0x14:
    switch (MessageBuffer[3]) {
      case 0x05:
      case 0x06:N7110_ReplySaveSMSMessage    (MessageLength,MessageBuffer,MessageType);break;
      case 0x0b:N7110_ReplyDeleteSMSMessage  (MessageLength,MessageBuffer,MessageType);break;
      case 0x37:
      case 0x38:N7110_ReplyGetSMSStatus      (MessageLength,MessageBuffer,MessageType);break;
      case 0x6C:N7110_ReplyGetSMSFolderStatus(MessageLength,MessageBuffer,MessageType);break;
      case 0x7B:N7110_ReplyGetSMSFolders     (MessageLength,MessageBuffer,MessageType);break;
      case 0x84:N7110_ReplySaveSMSMessage    (MessageLength,MessageBuffer,MessageType);break;
      default  :N7110_Dispatch0x14Message    (MessageLength, MessageBuffer, MessageType);break;
    }
    break;
 
  /* Battery status */
  case 0x17:

    N7110_Dispatch0x17Message(MessageLength, MessageBuffer, MessageType);
    break;

  /* Date and time */
  case 0x19:
    switch (MessageBuffer[3]) {
      case 0x61:N6110_ReplySetDateTime(MessageLength,MessageBuffer,MessageType);break;
      case 0x63:N6110_ReplyGetDateTime(MessageLength,MessageBuffer,MessageType);break;
      case 0x6c:N6110_ReplySetAlarm   (MessageLength,MessageBuffer,MessageType);break;
      case 0x6e:N6110_ReplyGetAlarm   (MessageLength,MessageBuffer,MessageType);break;
      default  :unknown=true;break;
    }
    break;

  /* Ringtones */
  case 0x1f:
    switch (MessageBuffer[3]) {
      case 0x23:
      case 0x24:N7110_ReplyGetBinRingtone(MessageLength,MessageBuffer,MessageType);break;
      default  :unknown=true;break;
    }
    break;

  /* Profiles */
  case 0x39:

    N7110_ReplyGetProfile(MessageLength, MessageBuffer, MessageType);
    break;

  /* WAP */
  case 0x3f:
    switch (MessageBuffer[3]) {
      case 0x01:
      case 0x02:N7110_ReplyEnableWAPCommands(MessageLength,MessageBuffer,MessageType);break;
      case 0x07:
      case 0x08:N7110_ReplyGetWAPBookmark   (MessageLength,MessageBuffer,MessageType);break;
      case 0x0a:
      case 0x0b:N7110_ReplySetWAPBookmark   (MessageLength,MessageBuffer,MessageType);break;
      case 0x16:
      case 0x17:
      case 0x1c:N7110_ReplyGetWAPSettings   (MessageLength,MessageBuffer,MessageType);break;
      default  :unknown=true;break;
    }
    break;

  /* Internal phone functions ? */
  /* Note, we use N6110_Dispatch0x40Message, but only some of 0x40
     old msg are available in new phones - other returns generally only
     errors */
  case 0x40:
    switch (MessageBuffer[2]) {
      case 0x64:N6110_ReplyEnableExtendedCommands  (MessageLength,MessageBuffer,MessageType);break;
      case 0x65:N6110_ReplyResetPhoneSettings      (MessageLength,MessageBuffer,MessageType);break;
      case 0x66:N6110_ReplyIMEI                    (MessageLength,MessageBuffer,MessageType);break;
      case 0x6a:N6110_ReplyGetProductProfileSetting(MessageLength,MessageBuffer,MessageType);break;
      case 0x6b:N6110_ReplySetProductProfileSetting(MessageLength,MessageBuffer,MessageType);break;
      case 0x7e:N6110_ReplyNetmonitor              (MessageLength,MessageBuffer,MessageType);break;
      case 0x8a:N6110_ReplySimlockInfo             (MessageLength,MessageBuffer,MessageType);break;
      case 0x8f:N6110_ReplyPlayTone                (MessageLength,MessageBuffer,MessageType);break;
      case 0xc8:N6110_ReplyHW                      (MessageLength,MessageBuffer,MessageType);break;
      default  :N6110_Dispatch0x40Message          (MessageLength,MessageBuffer,MessageType);break;
    }
    break;

  /* Settings */
  case 0x7a:

    N7110_Dispatch0x7AMessage(MessageLength, MessageBuffer, MessageType);
    break;

  /***** Acknowlegment of our frames. *****/
  case FBUS_FRTYPE_ACK:

    N6110_DispatchACKMessage(MessageLength, MessageBuffer, MessageType);
    break;

  case 0xd2:

    N6110_ReplyID(MessageLength, MessageBuffer, MessageType);
    break;

  /***** Unknown message *****/
  /* If you think that you know the exact meaning of other messages - please
     let us know. */
  default:

#ifdef DEBUG
    fprintf(stdout, _("Message: Unknown message type.\n"));
#endif /* DEBUG */
    AppendLogText("Unknown msg type\n",false);
    break;

  }

  if (unknown) {
#ifdef DEBUG
    fprintf(stdout, _("Unknown message of type %02x.\n"),MessageType);
#endif
    AppendLogText("Unknown msg\n",false);
  }
}
