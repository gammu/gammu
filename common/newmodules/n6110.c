/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides an API for accessing functions on the 6110 and similar
  phones.

*/

/* "Turn on" prototypes in n-6110.h */

#define __n_6110_c 

/* System header files */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
#include "devices/device.h"
/* Global variables used by code in gsm-api.c to expose the functions
   supported by this model of phone. */







/* Here we initialise model specific functions. */
GSM_Functions N6110_Functions = {
  N6110_Initialise,
  N6110_DispatchMessage,
  NULL_Terminate,
  NULL_KeepAlive,
  N6110_GetMemoryLocation,
  N6110_WritePhonebookLocation,
  N6110_GetSpeedDial,
  N6110_SetSpeedDial,
  N6110_GetMemoryStatus,
  N6110_GetSMSStatus,
  N6110_GetSMSCenter,
  N6110_SetSMSCenter,
  N6110_GetSMSMessage,
  N6110_DeleteSMSMessage,
  N6110_SendSMSMessage,
  N6110_SaveSMSMessage,
  N6110_GetRFLevel,
  N6110_GetBatteryLevel,
  N6110_GetPowerSource,
  N6110_GetDisplayStatus,
  N6110_EnterSecurityCode,
  N6110_GetSecurityCodeStatus,
  N6110_GetSecurityCode,
  N6110_GetIMEI,
  N6110_GetRevision,
  N6110_GetModel,
  N6110_GetDateTime,
  N6110_SetDateTime,
  N6110_GetAlarm,
  N6110_SetAlarm,
  N6110_DialVoice,
  N6110_DialData,
  N6110_GetIncomingCallNr,
  N6110_GetNetworkInfo,
  N6110_GetCalendarNote,
  N6110_WriteCalendarNote,
  N6110_DeleteCalendarNote,
  N6110_NetMonitor,
  N6110_SendDTMF,
  N6110_GetBitmap,
  N6110_SetBitmap,
  N6110_SetRingTone,
  N6110_SetBinRingTone,
  N6110_GetBinRingTone,
  N6110_Reset,
  N6110_GetProfile,
  N6110_SetProfile,
  N6110_SendRLPFrame,
  N6110_CancelCall,
  N6110_PressKey,
  N6110_EnableDisplayOutput,
  N6110_DisableDisplayOutput,
  N6110_EnableCellBroadcast,
  N6110_DisableCellBroadcast,
  N6110_ReadCellBroadcast,
  N6110_PlayTone,
  N6110_GetProductProfileSetting,
  N6110_SetProductProfileSetting,
  N6110_GetOperatorName,
  N6110_SetOperatorName,
  N6110_GetVoiceMailbox,  N6110_Tests,
  N6110_SimlockInfo,
  UNIMPLEMENTED,                 //GetCalendarNotesInfo
  N6110_GetSMSFolders,
  N6110_ResetPhoneSettings,
  N7110_GetWAPBookmark,
  N7110_SetWAPBookmark,
  N7110_GetWAPSettings,
  N6110_CallDivert,
  N6110_AnswerCall,
  N6110_GetManufacturer
};

/* Mobile phone information */

GSM_Information N6110_Information = {
  "3210|3310|3330|5110|5130|5190|6110|6130|6150|6190|8210|8850",
     /* Supported models in FBUS */
  "3210|3310|3330|5110|5130|5190|6110|6130|6150|6190|8210|8850|9210",
     /* Supported models in MBUS */
  "6110|6130|6150|8210|8850",
     /* Supported models in FBUS over infrared */
  "",
     /* Supported models in FBUS over DLR3 */
  "",
     /* AT commands */
  "8210|8850",
     /* infrared sockets */
  "6110|6130|6150|8210|8850",
     /* Supported models in FBUS over infrared with Tekram device */  "",
  4,                     /* Max RF Level */
  0,                     /* Min RF Level */
  GRF_Arbitrary,         /* RF level units */
  4,                     /* Max Battery Level */
  0,                     /* Min Battery Level */
  GBU_Arbitrary,         /* Battery level units */
  GDT_DateTime,          /* Have date/time support */
  GDT_TimeOnly,	         /* Alarm supports time only */
  1                      /* Only one alarm available */
};

const char *N6110_MemoryType_String [] = {
  "", 	/* 0x00 */
  "MT", /* 0x01 */
  "ME", /* 0x02 */
  "SM", /* 0x03 */
  "FD", /* 0x04 */
  "ON", /* 0x05 */
  "EN", /* 0x06 */
  "DC", /* 0x07 */
  "RC", /* 0x08 */
  "MC", /* 0x09 */
};

/* Magic bytes from the phone. */
unsigned char MagicBytes[4] = { 0x00, 0x00, 0x00, 0x00 };

/* For DisplayOutput */
char               PhoneScreen[5+1][27+1];
int                OldX=1000,OldY=0,NewX=0,NewY=0;

void N6110_ReplyEnableExtendedCommands(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: Answer for EnableExtendedSecurityCommands frame, meaning not known :-(\n"));
#endif /* DEBUG */

  CurrentEnableExtendedCommandsError=GE_NONE;	
}

/* If you set make some things (for example,
   change Security Code from phone's menu, disable and enable
   phone), it won't answer for 0x40 frame - you won't be able
   to play tones, get netmonitor, etc.

   This function do thing called "Enabling extended security commands" -
   it enables 0x40 frame functions.

   This frame can also some other things - see below */
GSM_Error N6110_EnableExtendedCommands (unsigned char status)
{
  unsigned char req[4] = { 0x00,
                           0x01,0x64, /* Enable extended commands request */
 			   0x01 };    /* 0x01 - on, 0x00 - off,
		                         0x03 & 0x04 - soft & hard reset,
                                         0x06 - CONTACT SERVICE */

  /* 0x06 MAKES CONTACT SERVICE! BE CAREFULL! */
  /* When use 0x03 and had during session changed time & date
     some phones (like 6150 or 6210) can ask for time & date after reset
     or disable clock on the screen */
  if (status!=0x06) req[3] = status;

  return NULL_SendMessageSequence
    (50, &CurrentEnableExtendedCommandsError, 4, 0x40, req);
}

void N6110_ReplyIMEI(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#if defined WIN32 || !defined HAVE_SNPRINTF
  sprintf(Current_IMEI, "%s", MessageBuffer+4);
#else
  snprintf(Current_IMEI, GSM_MAX_IMEI_LENGTH, "%s", MessageBuffer+4);
#endif

#ifdef DEBUG
  fprintf(stdout, _("Message: IMEI %s received\n"),Current_IMEI);
#endif

  CurrentGetIMEIError=GE_NONE;       
}

GSM_Error N6110_SendIMEIFrame()
{
  unsigned char req[4] = {0x00, 0x01, 0x66, 0x00};  

  GSM_Error error;

  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;
  
  return NULL_SendMessageSequence (20, &CurrentGetIMEIError, 4, 0x40, req);
}

void N6110_ReplyHW(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int i, j;
    
  if (MessageBuffer[3]==0x05) {

#ifdef DEBUG
    fprintf(stdout,_("Message: Hardware version received: "));
#endif

    j=strlen(Current_Revision);
    Current_Revision[j++]=',';
    Current_Revision[j++]=' ';
    Current_Revision[j++]='H';
    Current_Revision[j++]='W';
            
    for (i=5;i<9;i++) {
#ifdef DEBUG
      fprintf(stdout,_("%c"), MessageBuffer[i]);
#endif
      Current_Revision[j++]=MessageBuffer[i];
    }

#ifdef DEBUG
    fprintf(stdout,_("\n"));
#endif

    CurrentGetHWError=GE_NONE;
  }
}

GSM_Error N6110_SendHWFrame()
{
  unsigned char req[4] = {0x00, 0x01, 0xc8, 0x05};  

  GSM_Error error;

  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;
  
  return NULL_SendMessageSequence (20, &CurrentGetHWError, 4, 0x40, req);
}

void N6110_ReplyID(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int i, j;
 
#ifdef DEBUG
  fprintf(stdout, _("Message: Mobile phone model identification received:\n"));
  fprintf(stdout, _("   Firmware: "));
#endif

  strcpy(Current_Revision,"SW");
    
  i=6;j=2;
  while (MessageBuffer[i]!=0x0a) {
    Current_Revision[j]=MessageBuffer[i];
#ifdef DEBUG
    fprintf(stdout, _("%c"),MessageBuffer[i]);
#endif
    if (j==GSM_MAX_REVISION_LENGTH-1) {
#ifdef DEBUG
      fprintf(stderr,_("ERROR: increase GSM_MAX_REVISION_LENGTH!\n"));
#endif  
      break;
    }
    j++;
    i++;
  }
  Current_Revision[j+1]=0;

#ifdef DEBUG
  fprintf(stdout, _("\n   Firmware date: "));
#endif

  i++;
  while (MessageBuffer[i]!=0x0a) {
#ifdef DEBUG
    fprintf(stdout, _("%c"),MessageBuffer[i]);
#endif
    i++;
  }

#ifdef DEBUG
  fprintf(stdout, _("\n   Model: "));
#endif /* DEBUG */

  i++;j=0;
  while (MessageBuffer[i]!=0x0a) {
    Current_Model[j]=MessageBuffer[i];
#ifdef DEBUG
    fprintf(stdout, _("%c"),MessageBuffer[i]);
#endif
    if (j==GSM_MAX_MODEL_LENGTH-1) {
#ifdef DEBUG
      fprintf(stderr,_("ERROR: increase GSM_MAX_MODEL_LENGTH!\n"));
#endif  
      break;
    }
    j++;
    i++;
  }
  Current_Model[j+1]=0;

#ifdef DEBUG
  fprintf(stdout, _("\n"));
#endif /* DEBUG */
    
  CurrentMagicError=GE_NONE;
}

GSM_Error N6110_SendIDFrame()
{
  unsigned char req[5] = {N6110_FRAME_HEADER, 0x03, 0x00};  

  return NULL_SendMessageSequence (50, &CurrentMagicError, 5, 0xd1, req);
}

/* This function send the status request to the phone. */
/* Seems to be ignored in N3210 */
GSM_Error N6110_SendStatusRequest(void)
{
  unsigned char req[] = {N6110_FRAME_HEADER, 0x01};

  Protocol->SendMessage(4, 0x04, req);

  return (GE_NONE);
}

void N6110_ReplyGetAuthentication(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#if defined WIN32 || !defined HAVE_SNPRINTF
  sprintf(Current_IMEI, "%s", MessageBuffer+9);
  sprintf(Current_Model, "%s", MessageBuffer+25);
  sprintf(Current_Revision, "SW%s, HW%s", MessageBuffer+44, MessageBuffer+39);
#else
  snprintf(Current_IMEI, GSM_MAX_IMEI_LENGTH, "%s", MessageBuffer+9);
  snprintf(Current_Model, GSM_MAX_MODEL_LENGTH, "%s", MessageBuffer+25);
  snprintf(Current_Revision, GSM_MAX_REVISION_LENGTH, "SW%s, HW%s", MessageBuffer+44, MessageBuffer+39);
#endif

#ifdef DEBUG
  fprintf(stdout, _("Message: Mobile phone identification received:\n"));
  fprintf(stdout, _("   IMEI: %s\n"), Current_IMEI);
  fprintf(stdout, _("   Model: %s\n"), Current_Model);
  fprintf(stdout, _("   Production Code: %s\n"), MessageBuffer+31);
  fprintf(stdout, _("   HW: %s\n"), MessageBuffer+39);
  fprintf(stdout, _("   Firmware: %s\n"), MessageBuffer+44);

  /* These bytes are probably the source of the "Accessory not connected"
     messages on the phone when trying to emulate NCDS... I hope....
     UPDATE: of course, now we have the authentication algorithm. */
  fprintf(stdout, _("   Magic bytes: %02x %02x %02x %02x\n"), MessageBuffer[50], MessageBuffer[51], MessageBuffer[52], MessageBuffer[53]);
#endif /* DEBUG */

  MagicBytes[0]=MessageBuffer[50];
  MagicBytes[1]=MessageBuffer[51];
  MagicBytes[2]=MessageBuffer[52];
  MagicBytes[3]=MessageBuffer[53];

  CurrentMagicError=GE_NONE;
}

/* This function provides Nokia authentication protocol.

   This code is written specially for gnokii project by Odinokov Serge.
   If you have some special requests for Serge just write him to
   apskaita@post.omnitel.net or serge@takas.lt

   Reimplemented in C by Pavel Janík ml.

   Nokia authentication protocol is used in the communication between Nokia
   mobile phones (e.g. Nokia 6110) and Nokia Cellular Data Suite software,
   commercially sold by Nokia Corp.

   The authentication scheme is based on the token send by the phone to the
   software. The software does it's magic (see the function
   FB61_GetNokiaAuth()) and returns the result back to the phone. If the
   result is correct the phone responds with the message "Accessory
   connected!" displayed on the LCD. Otherwise it will display "Accessory not
   supported" and some functions will not be available for use.

   The specification of the protocol is not publicly available, no comment. */
void N6110_GetNokiaAuth(unsigned char *Imei, unsigned char *MagicBytes, unsigned char *MagicResponse)
{

  int i, j, CRC=0;

  /* This is our temporary working area. */

  unsigned char Temp[16];

  /* Here we put FAC (Final Assembly Code) and serial number into our area. */

  Temp[0]  = Imei[6];
  Temp[1]  = Imei[7];
  Temp[2]  = Imei[8];
  Temp[3]  = Imei[9];
  Temp[4]  = Imei[10];
  Temp[5]  = Imei[11];
  Temp[6]  = Imei[12];
  Temp[7]  = Imei[13];

  /* And now the TAC (Type Approval Code). */

  Temp[8]  = Imei[2];
  Temp[9]  = Imei[3];
  Temp[10] = Imei[4];
  Temp[11] = Imei[5];

  /* And now we pack magic bytes from the phone. */

  Temp[12] = MagicBytes[0];
  Temp[13] = MagicBytes[1];
  Temp[14] = MagicBytes[2];
  Temp[15] = MagicBytes[3];

  for (i=0; i<=11; i++)
    if (Temp[i + 1]& 1)
      Temp[i]<<=1;

  switch (Temp[15] & 0x03) {

  case 1:
  case 2:
    j = Temp[13] & 0x07;

    for (i=0; i<=3; i++)
      Temp[i+j] ^= Temp[i+12];

    break;

  default:
    j = Temp[14] & 0x07;

    for (i=0; i<=3; i++)
      Temp[i + j] |= Temp[i + 12];
  }

  for (i=0; i<=15; i++)
    CRC ^= Temp[i];

  for (i=0; i<=15; i++) {

    switch (Temp[15 - i] & 0x06) {

    case 0:
      j = Temp[i] | CRC;
      break;

    case 2:
    case 4:
      j = Temp[i] ^ CRC;
      break;

    case 6:
      j = Temp[i] & CRC;
      break;
    }
  
    if (j == CRC)
      j = 0x2c;

    if (Temp[i] == 0)
      j = 0;

    MagicResponse[i] = j;

  }
}

GSM_Error N6110_Authentication()
{
  unsigned char connect1[] = {N6110_FRAME_HEADER, 0x0d, 0x00, 0x00, 0x02};
  unsigned char connect2[] = {N6110_FRAME_HEADER, 0x20, 0x02};
  unsigned char connect3[] = {N6110_FRAME_HEADER, 0x0d, 0x01, 0x00, 0x02};
  unsigned char connect4[] = {N6110_FRAME_HEADER, 0x10};
  
  unsigned char magic_connect[] = {N6110_FRAME_HEADER,
  0x12,

  /* The real magic goes here ... These bytes are filled in with the
     function N6110_GetNokiaAuth(). */

  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

  /* NOKIA&GNOKII Accessory */

  0x4e, 0x4f, 0x4b, 0x49, 0x41, 0x26, 0x4e, 0x4f, 0x4b, 0x49, 0x41, 0x20,
  0x61, 0x63, 0x63, 0x65, 0x73, 0x73, 0x6f, 0x72, 0x79,
  
  0x00, 0x00, 0x00, 0x00};

#ifdef DEBUG
  fprintf(stdout,_("Making authentication!\n"));
#endif

  usleep(100); Protocol->SendMessage(7, 0x02, connect1);
  usleep(100); Protocol->SendMessage(5, 0x02, connect2);
  usleep(100); Protocol->SendMessage(7, 0x02, connect3);
      
  CurrentMagicError = GE_BUSY;

  usleep(100); Protocol->SendMessage(4, 0x64, connect4);
  if (NULL_WaitUntil(50,&CurrentMagicError)!=GE_NONE) return GE_TIMEOUT;

  N6110_GetNokiaAuth(Current_IMEI, MagicBytes, magic_connect+4);

  Protocol->SendMessage(45, 0x64, magic_connect);

#ifdef DEBUG
  fprintf(stdout,_("End of authentication!\n"));
#endif

  return GE_NONE;
}

/* Initialise variables and state machine. */
GSM_Error N6110_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame))
{
  unsigned char init_char = N6110_SYNC_BYTE;
  unsigned char end_init_char = N6110_IR_END_SYNC_BYTE;

  int count;
  int InitLength;
  
  if (Protocol->Initialise(port_device,initlength,connection,rlp_callback)!=GE_NONE)
  {
    return GE_NOTSUPPORTED;
  }

  switch (CurrentConnectionType) {
    case GCT_Irda:
    case GCT_MBUS:
      /* We don't think about authentication in Irda, because
         AFAIK there are no phones working over sockets
	 and having authentication. In MBUS it doesn't work */
      usleep(100);

      if (N6110_SendIDFrame()!=GE_NONE) return GE_TIMEOUT;
    
      if (N6110_SendIMEIFrame()!=GE_NONE) return GE_TIMEOUT;    

      if (N6110_SendHWFrame()!=GE_NONE) return GE_TIMEOUT;    

      CurrentLinkOK = true;                                 
      break;
      
    case GCT_FBUS:
    case GCT_Infrared:
    case GCT_Tekram:
      InitLength = atoi(initlength);

      if ((strcmp(initlength, "default") == 0) || (InitLength == 0)) {
        InitLength = 250;	/* This is the usual value, lower may work. */
      }

      if (CurrentConnectionType==GCT_Infrared ||
          CurrentConnectionType==GCT_Tekram) {
#ifdef DEBUG
        fprintf(stdout,_("Setting infrared for FBUS communication...\n"));
#endif
        device_changespeed(9600);
      }

#ifdef DEBUG
      fprintf(stdout,_("Writing init chars...."));
#endif

      /* Initialise link with phone or what have you */
      /* Send init string to phone, this is a bunch of 0x55 characters. Timing is
         empirical. */
      for (count = 0; count < InitLength; count ++) {
        if (CurrentConnectionType!=GCT_Infrared &&
            CurrentConnectionType!=GCT_Tekram)         usleep(100);
        Protocol->WritePhone(1,&init_char);
      }

      if (CurrentConnectionType==GCT_Infrared ||
          CurrentConnectionType==GCT_Tekram)      {
        Protocol->WritePhone(1,&end_init_char);
        usleep(200000);
      }

#ifdef DEBUG
      fprintf(stdout,_("Done\n"));  
#endif

      if (CurrentConnectionType==GCT_Infrared ||
          CurrentConnectionType==GCT_Tekram)      {
        device_changespeed(115200);    
      }

      N6110_SendStatusRequest();
    
      usleep(100);

      if (N6110_SendIDFrame()!=GE_NONE) return GE_TIMEOUT;
    
      /* N51xx/61xx have something called authentication.
         After making it phone display "Accessory connected"
         and probably give access to some function (I'm not too sure about it !)
         Anyway, I make it now for N51xx/61xx */
      if (GetModelFeature (FN_AUTHENTICATION)!=0) {
        if (N6110_Authentication()!=GE_NONE) return GE_TIMEOUT;
      } else {        /* No authentication */
        if (N6110_SendIMEIFrame()!=GE_NONE) return GE_TIMEOUT;    

        if (N6110_SendHWFrame()!=GE_NONE) return GE_TIMEOUT;    
      }
      
      break;
    default:
#ifdef DEBUG
      fprintf(stdout,_("Unknown connection type in n6110.c!\n"));
#endif
      break;
  }

  return (GE_NONE);
}

/* This function translates GMT_MemoryType to N6110_MEMORY_xx */
int N6110_GetMemoryType(GSM_MemoryType memory_type)
{

  int result;

  switch (memory_type) {

     case GMT_MT: result = N6110_MEMORY_MT; break;
     case GMT_ME: result = N6110_MEMORY_ME; break;
     case GMT_SM: result = N6110_MEMORY_SM; break;
     case GMT_FD: result = N6110_MEMORY_FD; break;
     case GMT_ON: result = N6110_MEMORY_ON; break;
     case GMT_EN: result = N6110_MEMORY_EN; break;
     case GMT_DC: result = N6110_MEMORY_DC; break;
     case GMT_RC: result = N6110_MEMORY_RC; break;
     case GMT_MC: result = N6110_MEMORY_MC; break;
     default    : result = N6110_MEMORY_XX;

   }

   return (result);
}

void N6110_ReplyCallDivert(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch (MessageBuffer[3]) {

  case 0x02:

#ifdef DEBUG
    fprintf(stdout, _("Message: Call divert status received\n"));
    fprintf(stdout, _("   Divert type: "));
    switch (MessageBuffer[6]) {
      case 0x43: fprintf(stdout, _("when busy"));break;
      case 0x3d: fprintf(stdout, _("when not answered"));break;
      case 0x3e: fprintf(stdout, _("when phone off or no coverage"));break;
      case 0x15: fprintf(stdout, _("all types of diverts"));break; //?
      case 0x02: fprintf(stdout, _("all types of diverts"));break; //?
      default:   fprintf(stdout, _("unknown %i"),MessageBuffer[6]);break;
    }
    fprintf(stdout, _("\n   Calls type : "));
    if (MessageBuffer[6]==0x02)
      fprintf(stdout, _("voice, fax & data")); //?
    else {
      switch (MessageBuffer[8]) {
        case 0x0b: fprintf(stdout, _("voice"));break;
        case 0x0d: fprintf(stdout, _("fax"));break;
        case 0x19: fprintf(stdout, _("data"));break;
        default:   fprintf(stdout, _("unknown %i"),MessageBuffer[8]);break;
      }
    }
    fprintf(stdout, _("\n"));     
    if (MessageBuffer[10]==0x01) {
      fprintf(stdout, _("   Status     : active\n"));
      fprintf(stdout, _("   Timeout    : %i seconds\n"),MessageBuffer[45]);
      fprintf(stdout, _("   Number     : %s\n"),GSM_UnpackSemiOctetNumber(MessageBuffer+12,true));
    } else {
      fprintf(stdout, _("   Status     : deactivated\n"));     
    }
#endif /* DEBUG */

    if (CurrentCallDivert!=NULL) { 
      switch (MessageBuffer[6]) {
        case 0x43: CurrentCallDivert->DType=GSM_CDV_Busy;break;
        case 0x3d: CurrentCallDivert->DType=GSM_CDV_NoAnswer;break;
        case 0x3e: CurrentCallDivert->DType=GSM_CDV_OutOfReach;break;
        case 0x15: CurrentCallDivert->DType=GSM_CDV_AllTypes;break; //?
        case 0x02: CurrentCallDivert->DType=GSM_CDV_AllTypes;break; //?
      }

      if (MessageBuffer[6]==0x02) //?
        CurrentCallDivert->CType=GSM_CDV_AllCalls;
      else {
        switch (MessageBuffer[8]) {
          case 0x0b: CurrentCallDivert->CType=GSM_CDV_VoiceCalls;break;
          case 0x0d: CurrentCallDivert->CType=GSM_CDV_FaxCalls;  break;
          case 0x19: CurrentCallDivert->CType=GSM_CDV_DataCalls; break;
        }
      }

      if (MessageBuffer[10]==0x01) {
        CurrentCallDivert->Enabled=true;
        CurrentCallDivert->Timeout=MessageBuffer[45];
        strcpy(CurrentCallDivert->Number,GSM_UnpackSemiOctetNumber(MessageBuffer+12,true));
      } else {
        CurrentCallDivert->Enabled=false;
      }
      CurrentCallDivertError=GE_NONE;
    }
    break;

  case 0x03:
#ifdef DEBUG
    fprintf(stdout, _("Message: Call divert status receiving error ?\n"));
#endif
    CurrentCallDivertError=GE_UNKNOWN;
    break;
  }
}

GSM_Error N6110_CallDivert(GSM_CallDivert *cd)
{
  char req[55] = { N6110_FRAME_HEADER, 0x01,
				       0x00, /* operation */
 				       0x00,
 				       0x00, /* divert type */
 				       0x00, /* call type */
 				       0x00 };
  GSM_Error error;

  int length = 0x09;

  switch (cd->Operation) {
    case GSM_CDV_Register:
    case GSM_CDV_Enable:
      req[4] = 0x03;
      req[8] = 0x01;
      req[29]= GSM_PackSemiOctetNumber(cd->Number, req + 9, false);
      req[52]= cd->Timeout;
      length = 55;
      break;
    case GSM_CDV_Erasure:
    case GSM_CDV_Disable:
      req[4] = 0x04;
      break;
    case GSM_CDV_Query:
      req[4] = 0x05;
      break;
    default:
      return GE_NOTIMPLEMENTED;
  }

  switch (cd->DType) {
    case GSM_CDV_AllTypes  : req[6] = 0x15; break;
    case GSM_CDV_Busy      : req[6] = 0x43; break;
    case GSM_CDV_NoAnswer  : req[6] = 0x3d; break;
    case GSM_CDV_OutOfReach: req[6] = 0x3e; break;
    default:                 return GE_NOTIMPLEMENTED;
  }

  if ((cd->DType == GSM_CDV_AllTypes) &&
      (cd->CType == GSM_CDV_AllCalls))
    req[6] = 0x02;

  switch (cd->CType) {
    case GSM_CDV_AllCalls  :                break;
    case GSM_CDV_VoiceCalls: req[7] = 0x0b; break;
    case GSM_CDV_FaxCalls  : req[7] = 0x0d; break;
    case GSM_CDV_DataCalls : req[7] = 0x19; break;
    default:                 return GE_NOTIMPLEMENTED;
  }

  CurrentCallDivert = cd;

  error=NULL_SendMessageSequence
    (100, &CurrentCallDivertError, length, 0x06, req);

  CurrentCallDivert = NULL;

  return error;
}

GSM_Error N6110_Tests()
{
  unsigned char buffer[3]={0x00,0x01,0xcf};
  unsigned char buffer3[8]={0x00,0x01,0xce,0x1d,0xfe,0x23,0x00,0x00};
  
  GSM_Error error;

  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;

  //make almost all tests
  Protocol->SendMessage(8, 0x40, buffer3);

  while (GSM->Initialise(PortDevice, "50", CurrentConnectionType, CurrentRLP_RXCallback)!=GE_NONE) {};

  sleep(2);

  return NULL_SendMessageSequence
    (200, &CurrentNetworkInfoError, 3, 0x40, buffer);  
}

void N6110_DisplayTestsInfo(u8 *MessageBuffer) {

  int i;

  CurrentNetworkInfoError=GE_NONE;

  for (i=0;i<MessageBuffer[3];i++) {
    switch (i) {
      case 0: fprintf(stdout,_("Unknown(%i)              "),i);break;
      case 1: fprintf(stdout,_("MCU ROM checksum        "));break;
      case 2: fprintf(stdout,_("MCU RAM interface       "));break;
      case 3: fprintf(stdout,_("MCU RAM component       "));break;
      case 4: fprintf(stdout,_("MCU EEPROM interface    "));break;
      case 5: fprintf(stdout,_("MCU EEPROM component    "));break;
      case 6: fprintf(stdout,_("Real Time Clock battery "));break;
      case 7: fprintf(stdout,_("CCONT interface         "));break;
      case 8: fprintf(stdout,_("AD converter            "));break;
      case 9: fprintf(stdout,_("SW Reset                "));break;
      case 10:fprintf(stdout,_("Power Off               "));break;
      case 11:fprintf(stdout,_("Security Data           "));break;
      case 12:fprintf(stdout,_("EEPROM Tune checksum    "));break;
      case 13:fprintf(stdout,_("PPM checksum            "));break;
      case 14:fprintf(stdout,_("MCU download DSP        "));break;
      case 15:fprintf(stdout,_("DSP alive               "));break;
      case 16:fprintf(stdout,_("COBBA serial            "));break;
      case 17:fprintf(stdout,_("COBBA paraller          "));break;
      case 18:fprintf(stdout,_("EEPROM security checksum"));break;
      case 19:fprintf(stdout,_("PPM validity            "));break;
      case 20:fprintf(stdout,_("Warranty state          "));break;
      case 21:fprintf(stdout,_("Simlock check           "));break;
      case 22:fprintf(stdout,_("IMEI check?             "));break;//from PC-Locals.is OK?
      default:fprintf(stdout,_("Unknown(%i)             "),i);break;
    }
    switch (MessageBuffer[4+i]) {
      case 0:   fprintf(stdout,_(" : done, result OK"));break;
      case 0xff:fprintf(stdout,_(" : not done, result unknown"));break;
      case 254: fprintf(stdout,_(" : done, result NOT OK"));break;
      default:  fprintf(stdout,_(" : result unknown(%i)"),MessageBuffer[4+i]);break;
    }
    fprintf(stdout,_("\n"));
  }
}

void N6110_ReplySimlockInfo(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int i, j;
  
  char uni[100];
    
#ifdef DEBUG
  fprintf(stdout, _("Message: Simlock info received\n"));

  j=0;
  for (i=0; i < 12; i++)
  {
    if (j<24) {
      fprintf(stdout,_("%c"), ('0' + (MessageBuffer[9+i] >> 4)));
      j++;
    }
    if (j==5 || j==15) fprintf(stdout, _("\n"));
    if (j!=15) {
      if (j<24) {
        fprintf(stdout,_("%c"), ('0' + (MessageBuffer[9+i] & 0x0f)));
	j++;
      }
    } else j++;
    if (j==20 || j==24) fprintf(stdout, _("\n"));
  }
      
  if ((MessageBuffer[6] & 1) == 1) fprintf(stdout,_("lock 1 closed\n"));
  if ((MessageBuffer[6] & 2) == 2) fprintf(stdout,_("lock 2 closed\n"));
  if ((MessageBuffer[6] & 4) == 4) fprintf(stdout,_("lock 3 closed\n"));
  if ((MessageBuffer[6] & 8) == 8) fprintf(stdout,_("lock 4 closed\n"));

  /* I'm not sure here at all */
  if ((MessageBuffer[5] & 1) == 1) fprintf(stdout,_("lock 1 - user\n"));
  if ((MessageBuffer[5] & 2) == 2) fprintf(stdout,_("lock 2 - user\n"));
  if ((MessageBuffer[5] & 4) == 4) fprintf(stdout,_("lock 3 - user\n"));
  if ((MessageBuffer[5] & 8) == 8) fprintf(stdout,_("lock 4 - user\n"));

  fprintf(stdout,_("counter for lock1: %i\n"),MessageBuffer[21]);
  fprintf(stdout,_("counter for lock2: %i\n"),MessageBuffer[22]);
  fprintf(stdout,_("counter for lock3: %i\n"),MessageBuffer[23]);
  fprintf(stdout,_("counter for lock4: %i\n"),MessageBuffer[24]);

#endif

  j=0;
  for (i=0; i < 12; i++)
  {
    if (j<24) {
      uni[j]='0' + (MessageBuffer[9+i] >> 4);
      j++;
    }
    if (j!=15) {
      if (j<24) {
  	uni[j]='0' + (MessageBuffer[9+i] & 0x0f);
        j++;
      }
    } else j++;
  }

  strncpy(CurrentSimLock->simlocks[0].data,uni,5);
  CurrentSimLock->simlocks[0].data[5]=0;
  strncpy(CurrentSimLock->simlocks[3].data,uni+5,10);
  CurrentSimLock->simlocks[3].data[10]=0;
  strncpy(CurrentSimLock->simlocks[1].data,uni+16,4);
  CurrentSimLock->simlocks[1].data[4]=0;
  strncpy(CurrentSimLock->simlocks[2].data,uni+20,4);
  CurrentSimLock->simlocks[2].data[4]=0;                                    

  CurrentSimLock->simlocks[0].enabled=((MessageBuffer[6] & 1) == 1);
  CurrentSimLock->simlocks[1].enabled=((MessageBuffer[6] & 2) == 2);
  CurrentSimLock->simlocks[2].enabled=((MessageBuffer[6] & 4) == 4);
  CurrentSimLock->simlocks[3].enabled=((MessageBuffer[6] & 8) == 8);

  CurrentSimLock->simlocks[0].factory=((MessageBuffer[5] & 1) != 1);
  CurrentSimLock->simlocks[1].factory=((MessageBuffer[5] & 2) != 2);
  CurrentSimLock->simlocks[2].factory=((MessageBuffer[5] & 4) != 4);
  CurrentSimLock->simlocks[3].factory=((MessageBuffer[5] & 8) != 8);

  CurrentSimLock->simlocks[0].counter=MessageBuffer[21];
  CurrentSimLock->simlocks[1].counter=MessageBuffer[22];
  CurrentSimLock->simlocks[2].counter=MessageBuffer[23];
  CurrentSimLock->simlocks[3].counter=MessageBuffer[24];

  CurrentSimlockInfoError=GE_NONE;
}

GSM_Error N6110_SimlockInfo(GSM_AllSimlocks *siml)
{
  GSM_Error error;
  unsigned char req[] = {0x00,0x01,0x8a,0x00};
  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;

  CurrentSimLock=siml;
 
  return NULL_SendMessageSequence (50, &CurrentSimlockInfoError, 4, 0x40, req);  
}

void N6110_ReplyResetPhoneSettings(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: Resetting phone settings\n"));
#endif /* DEBUG */

  CurrentResetPhoneSettingsError=GE_NONE;
}

GSM_Error N6110_ResetPhoneSettings()
{
  GSM_Error error;
  unsigned char req[] = {0x00,0x01,0x65,0x08,0x00};  
  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;

  return NULL_SendMessageSequence
    (50, &CurrentResetPhoneSettingsError, 5, 0x40, req);  
}
GSM_Error N6110_GetManufacturer(char *manufacturer)
{
	strcpy (manufacturer, "Nokia");
	return (GE_NONE);
}

GSM_Error N6110_GetVoiceMailbox ( GSM_PhonebookEntry *entry)
{
  unsigned char req[] = {N6110_FRAME_HEADER, 0x01, 0x00, 0x00, 0x00};

  GSM_Error error;
  
  CurrentPhonebookEntry = entry;

  req[4] = N6110_MEMORY_VOICE;
  req[5] = 0x00; /* Location - isn't important, but... */

  error=NULL_SendMessageSequence
    (20, &CurrentPhonebookError, 7, 0x03, req);
    
  CurrentPhonebookEntry = NULL;
  
  return error;
}

void N6110_ReplyGetOperatorName(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int i, count;
  
  GSM_Bitmap NullBitmap;

  DecodeNetworkCode(MessageBuffer+5, NullBitmap.netcode);
  
  count=8;

#ifdef DEBUG
  fprintf(stdout, _("Message: Info about downloaded operator name received: %s network (for gnokii \"%s\", for phone \""),
          NullBitmap.netcode,
	  GSM_GetNetworkName(NullBitmap.netcode));      
#endif
      
  i=count;
  while (MessageBuffer[count]!=0) {
#ifdef DEBUG
    fprintf(stdout,_("%c"),MessageBuffer[count]);
#endif
    count++;
  }
      
 strcpy(CurrentGetOperatorNameNetwork->Code, NullBitmap.netcode);
 strncpy(CurrentGetOperatorNameNetwork->Name, MessageBuffer+i,count-i+1);

#ifdef DEBUG
  fprintf(stdout,_("\")\n"));
#endif
          
  CurrentGetOperatorNameError=GE_NONE;
}

GSM_Error N6110_GetOperatorName (GSM_Network *operator)
{
  unsigned char req[] = { 0x00,0x01,0x8c,0x00};

  GSM_Error error;

  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;

  CurrentGetOperatorNameNetwork = operator;

  error=NULL_SendMessageSequence
    (20, &CurrentGetOperatorNameError, 4, 0x40, req);

  CurrentGetOperatorNameNetwork = NULL;
  
  return error;
}

void N6110_ReplySetOperatorName(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
    
#ifdef DEBUG
  fprintf(stdout, _("Message: Downloaded operator name changed\n"));
#endif    

  CurrentSetOperatorNameError=GE_NONE;      
}

GSM_Error N6110_SetOperatorName (GSM_Network *operator)
{
  unsigned char req[256] = { 0x00,0x01,0x8b,0x00,
                             0x00,0x00, /* MCC */
			     0x00};     /* MNC */

  GSM_Error error;

  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;

  EncodeNetworkCode(req+4,operator->Code);

  strncpy(req+7,operator->Name,200);
    
  return NULL_SendMessageSequence
    (20, &CurrentSetOperatorNameError, 8+strlen(operator->Name), 0x40, req);
}

void N6110_ReplyGetMemoryStatus(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch (MessageBuffer[3]) {

  case 0x08:

#ifdef DEBUG
    fprintf(stdout, _("Message: Memory status received:\n"));

    fprintf(stdout, _("   Memory Type: %s\n"), N6110_MemoryType_String[MessageBuffer[4]]);
    fprintf(stdout, _("   Used: %d\n"), MessageBuffer[6]);
    fprintf(stdout, _("   Free: %d\n"), MessageBuffer[5]);
#endif /* DEBUG */

    CurrentMemoryStatus->Used = MessageBuffer[6];
    CurrentMemoryStatus->Free = MessageBuffer[5];
    CurrentMemoryStatusError = GE_NONE;

    break;

  case 0x09:

#ifdef DEBUG
    switch (MessageBuffer[4]) {
      case 0x6f:
	fprintf(stdout, _("Message: Memory status error, phone is probably powered off.\n"));break;
      case 0x7d:
	fprintf(stdout, _("Message: Memory status error, memory type not supported by phone model.\n"));break;
      case 0x8d:
	fprintf(stdout, _("Message: Memory status error, waiting for security code.\n"));break;
      default:
	fprintf(stdout, _("Message: Unknown Memory status error, subtype (MessageBuffer[4]) = %02x\n"),MessageBuffer[4]);break;
    }
#endif

    switch (MessageBuffer[4]) {
      case 0x6f:CurrentMemoryStatusError = GE_TIMEOUT;break;
      case 0x7d:CurrentMemoryStatusError = GE_INTERNALERROR;break;
      case 0x8d:CurrentMemoryStatusError = GE_INVALIDSECURITYCODE;break;
      default:break;
    }

    break;

  }
}

/* This function is used to get storage status from the phone. It currently
   supports two different memory areas - internal and SIM. */
GSM_Error N6110_GetMemoryStatus(GSM_MemoryStatus *Status)
{
  unsigned char req[] = { N6110_FRAME_HEADER,
                          0x07, /* MemoryStatus request */
                          0x00  /* MemoryType */
                        };

  GSM_Error error;
  
  CurrentMemoryStatus = Status;

  req[4] = N6110_GetMemoryType(Status->MemoryType);

  error=NULL_SendMessageSequence
    (20, &CurrentMemoryStatusError, 5, 0x03, req);

  CurrentMemoryStatus = NULL;

  return error;
}

void N6110_ReplyGetNetworkInfo(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  GSM_NetworkInfo NullNetworkInfo;
  
  /* Make sure we are expecting NetworkInfo frame */
  if (CurrentNetworkInfo && CurrentNetworkInfoError == GE_BUSY) {
#ifdef DEBUG
    fprintf(stdout, _("Message: Network informations:\n"));
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
  fprintf(stdout, _("   Network name: %s (%s)\n"),
                     GSM_GetNetworkName(NullNetworkInfo.NetworkCode),
                     GSM_GetCountryName(NullNetworkInfo.NetworkCode));
  fprintf(stdout, _("   Status: "));

  switch (MessageBuffer[8]) {
    case 0x01: fprintf(stdout, _("home network selected")); break;
    case 0x02: fprintf(stdout, _("roaming network")); break;
    case 0x03: fprintf(stdout, _("requesting network")); break;
    case 0x04: fprintf(stdout, _("not registered in the network")); break;
    default: fprintf(stdout, _("unknown"));
  }

  fprintf(stdout, "\n");

  fprintf(stdout, _("   Network selection: %s\n"), MessageBuffer[9]==1?_("manual"):_("automatic"));
#endif /* DEBUG */

  /* Make sure we are expecting NetworkInfo frame */
  if (CurrentNetworkInfo && CurrentNetworkInfoError == GE_BUSY)
    *CurrentNetworkInfo=NullNetworkInfo;

  CurrentNetworkInfoError = GE_NONE;      
}

GSM_Error N6110_GetNetworkInfo(GSM_NetworkInfo *NetworkInfo)
{
  unsigned char req[] = { N6110_FRAME_HEADER,
                          0x70
                        };

  GSM_Error error;
  
  CurrentNetworkInfo = NetworkInfo;
  
  error=NULL_SendMessageSequence
    (20, &CurrentNetworkInfoError, 4, 0x0a, req);

  CurrentNetworkInfo = NULL;

  return error;
}

void N6110_ReplyGetProductProfileSetting(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int i;
  
#ifdef DEBUG
  fprintf(stdout, _("Message: Product Profile Settings received -"));
  for (i=0;i<4;i++) fprintf(stdout, _(" %02x"),MessageBuffer[3+i]);
  fprintf(stdout, _("\n"));  
#endif

  for (i=0;i<4;i++) CurrentPPS[i]=MessageBuffer[3+i];

  CurrentProductProfileSettingsError=GE_NONE;      
}

GSM_Error N6110_GetProductProfileSetting (GSM_PPS *PPS)
{
  unsigned char req[] = { 0x00, 0x01,0x6a };
  
  int i,j;

  GSM_Error error;

  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;

  error=NULL_SendMessageSequence
    (20, &CurrentProductProfileSettingsError, 3, 0x40, req);
  if (error!=GE_NONE) return error;    
  
  switch (PPS->Name) {
    case PPS_ALS      : PPS->bool_value=(CurrentPPS[1]&32); break;
    case PPS_GamesMenu: PPS->bool_value=(CurrentPPS[3]&64); break;
    case PPS_HRData   : PPS->bool_value=(CurrentPPS[0]&64); break;
    case PPS_14400Data: PPS->bool_value=(CurrentPPS[0]&128);break;
    case PPS_EFR      : PPS->int_value =(CurrentPPS[0]&1)    +(CurrentPPS[0]&2);    break;
    case PPS_FR       : PPS->int_value =(CurrentPPS[0]&16)/16+(CurrentPPS[0]&32)/16;break;
    case PPS_HR       : PPS->int_value =(CurrentPPS[0]&4)/4  +(CurrentPPS[0]&8)/4;  break;
    case PPS_VibraMenu: PPS->bool_value=(CurrentPPS[4]&64); break;
    case PPS_LCDContrast:
         PPS->int_value=0;
         j=1;
         for (i=0;i<5;i++) {
	   if (CurrentPPS[3]&j) PPS->int_value=PPS->int_value+j;
	   j=j*2;
	 }
	 PPS->int_value=PPS->int_value*100/32;
         break;

  }
  
  return (GE_NONE);
}

void N6110_ReplySetProductProfileSetting(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  int i;
  
  fprintf(stdout, _("Message: Product Profile Settings set to"));
  for (i=0;i<4;i++) fprintf(stdout, _(" %02x"),CurrentPPS[i]);
  fprintf(stdout, _("\n"));  
#endif
     
  CurrentProductProfileSettingsError=GE_NONE;     
}

GSM_Error N6110_SetProductProfileSetting (GSM_PPS *PPS)
{
  unsigned char req[] = { 0x00, 0x01,0x6b, 
                          0x00, 0x00, 0x00, 0x00 }; /* bytes with Product Profile Setings */
  unsigned char settings[32];
  
  GSM_PPS OldPPS;
  
  int i,j,z;
  
  GSM_Error error;

  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;
  
  OldPPS.Name=PPS_ALS;
  error=N6110_GetProductProfileSetting(&OldPPS);
  if (error!=GE_NONE) return error;
  
  j=128;z=0;
  for (i=0;i<32;i++) {
    if (CurrentPPS[z]&j)
      settings[i]='1';
    else
      settings[i]='0';    
    if (j==1) {
      j=128;
      z++;
    } else j=j/2;
  }
  
#ifdef DEBUG
  fprintf(stdout,_("Current settings: "));
  for (i=0;i<32;i++) {
    fprintf(stdout,_("%c"),settings[i]);    
  }
  fprintf(stdout,_("\n"));
#endif

  switch (PPS->Name) {
    case PPS_ALS      :settings[10]=PPS->bool_value?'1':'0';break;
    case PPS_HRData   :settings[ 5]=PPS->bool_value?'1':'0';break;
    case PPS_14400Data:settings[ 6]=PPS->bool_value?'1':'0';break;
    default           :break;
  }
    
  j=128;z=0;
  for (i=0;i<32;i++) {
    if (settings[i]=='1') req[z+3]=req[z+3]+j;
    if (j==1) {
      j=128;
      z++;
    } else j=j/2;
  }  

#ifdef DEBUG
  fprintf(stdout,_("Current settings: "));
  for (i=0;i<4;i++) {
    fprintf(stdout,_("%i "),req[i+3]);    
  }
  fprintf(stdout,_("\n"));
#endif

  for (i=0;i<4;i++) {
   CurrentPPS[i]=req[i+3];    
  }

  return NULL_SendMessageSequence
    (20, &CurrentProductProfileSettingsError, 7, 0x40, req);
}

void N6110_ReplyPressKey(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

    if (MessageBuffer[4]==CurrentPressKeyEvent) CurrentPressKeyError=GE_NONE;
                                           else CurrentPressKeyError=GE_UNKNOWN; /* MessageBuffer[4] = 0x05 */
#ifdef DEBUG
    fprintf(stdout, _("Message: Result of key "));
    switch (MessageBuffer[4])
    {
      case PRESSPHONEKEY:   fprintf(stdout, _("press OK\n"));break;
      case RELEASEPHONEKEY: fprintf(stdout, _("release OK\n"));break;
      default:              fprintf(stdout, _("press or release - error\n"));break;
    }
#endif /* DEBUG */
}

GSM_Error N6110_PressKey(int key, int event)
{
  unsigned char req[] = {N6110_FRAME_HEADER, 0x42, 0x01, 0x00, 0x01};
  
  req[4]=event; /* if we press or release key */
  req[5]=key;
  
  CurrentPressKeyEvent=event;
  
  return NULL_SendMessageSequence
    (10, &CurrentPressKeyError, 7, 0x0c, req);
}

void N6110_ReplyDisplayOutput(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  /* Hopefully is 64 larger as FB38_MAX* / N6110_MAX* */
  char model[64];

  int i, j;

  char uni[100];
    
  switch(MessageBuffer[3]) {

  /* Phone sends displayed texts */
  case 0x50:
    NewX=MessageBuffer[6];
    NewY=MessageBuffer[5];

    DecodeUnicode (uni, MessageBuffer+8, MessageBuffer[7]);

#ifdef DEBUG
    fprintf(stdout, _("New displayed text (%i %i): \"%s\"\n"),NewX,NewY,uni);      
#endif /* DEBUG */

    while (N6110_GetModel(model)  != GE_NONE)
      sleep(1);

    /* With these rules it works almost excellent with my N5110 */
    /* I don't have general rule :-(, that's why you must experiment */
    /* with your phone. Nokia could make it better. MW */
    /* It's almost OK for N5110*/
    /* FIX ME: it will be the same for N5130 and 3210 too*/
    if (!strcmp(model,"NSE-1"))
    {
      /* OldX==1000 means - it's first time */
      if (OldX==1000) {
      
        /* Clean table */
        for (i=0;i<5+1;i++) {
          for (j=0;j<27+1;j++) {PhoneScreen[i][j]=' ';}
      }
      OldX=0;
    }

    if ((OldX==0 && OldY==31 && NewX==29 && NewY==46) ||
        (OldX==0 && OldY==13 && NewX==23 && NewY==46)) {
      /* Clean the line with current text */
      for (j=0;j<27+1;j++) {PhoneScreen[NewY/(47/5)][j]=' ';}
      
      /* Inserts text into table */
      for (i=0; i<MessageBuffer[7];i++) {	
        PhoneScreen[NewY/(47/5)][NewX/(84/27)+i]=uni[i];
      }

    }

    if ((OldX==0 && OldY==21 && NewX==0 && NewY==10) ||
        (OldX==0 && OldY==10 && NewX==35 && NewY==46)) {
    } else {
      if ((OldX!=0 && NewX==0 && NewY!=6) ||
          (OldX==0 && NewX!=0 && OldY!=13 && OldY!=22) ||
          (OldX==0 && NewX==0 && NewY<OldY && (NewY!=13 || OldY!=26)) ||
          (OldY==5 && NewY!=5) ||
          (OldX==0 && OldY==13 && NewX==23 && NewY==46)) {

        /* Writes "old" screen */
        for (i=0;i<5+1;i++) {
          for (j=0;j<27+1;j++) {fprintf(stdout,_("%c"),PhoneScreen[i][j]);}
	    fprintf(stdout,_("\n"));
	  }
	
	  /* Clean table */
          for (i=0;i<5+1;i++) {
	    for (j=0;j<27+1;j++) {PhoneScreen[i][j]=' ';}
	  }
        }
      }
      
      /* Clean the line with current text */
      for (j=0;j<27+1;j++) {PhoneScreen[NewY/(47/5)][j]=' ';}
      
      /* Inserts text into table */
      for (i=0; i<MessageBuffer[7];i++) {	
        PhoneScreen[NewY/(47/5)][NewX/(84/27)+i]=uni[i];
      }
      
      OldY=NewY;
      OldX=NewX;
    } else {
#ifndef DEBUG
      fprintf(stdout, _("%s\n"),uni);      
#endif
    }

    break;
 
  case 0x54:
      
    if (MessageBuffer[4]==1)
    {
      
#ifdef DEBUG
      fprintf(stdout, _("Display output successfully disabled/enabled.\n"));
#endif /* DEBUG */

      CurrentDisplayOutputError=GE_NONE;
    }
       
    break;
  }
}

GSM_Error SetDisplayOutput(unsigned char state)
{
  unsigned char req[] = { N6110_FRAME_HEADER,
                          0x53, 0x00};

  req[4]=state;
  
  return NULL_SendMessageSequence
    (30, &CurrentDisplayOutputError, 5, 0x0d, req);
}

GSM_Error N6110_EnableDisplayOutput()
{
  return SetDisplayOutput(0x01);
}
 
GSM_Error N6110_DisableDisplayOutput()
{
  return SetDisplayOutput(0x02);
}

/* If it is interesting for somebody: we can use 0x40 msg for it
   and it will work for all phones. See n6110.txt for details */
GSM_Error N6110_AnswerCall(char s)
{
	unsigned char req0[] = { N6110_FRAME_HEADER, 0x42,0x05,0x01,0x07,				 0xa2,0x88,0x81,0x21,0x15,0x63,0xa8,0x00,0x00,
	                    0x07,0xa3,0xb8,0x81,0x20,0x15,0x63,0x80};
	unsigned char req[] = { N6110_FRAME_HEADER, 0x06, 0x00, 0x00};

	req[4]=s;

#ifdef DEBUG
	fprintf(stdout,_("Answering call %d\n\r"),s);
#endif

	Protocol->SendMessage(sizeof(req0), 0x01, req0);
	sleep(1);

	return NULL_SendMessageSequence
		(20, &CurrentMagicError, sizeof(req) , 0x01, req);
}

void N6110_ReplyGetProfile(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch (MessageBuffer[3]) {

  /* Profile feature */
  case 0x14:   

    switch(GetModelFeature (FN_PROFILES)) {
      case F_PROF33:
        switch (MessageBuffer[6]) {
          case 0x00: CurrentProfile->KeypadTone  = MessageBuffer[8]; break;
          case 0x01: CurrentProfile->CallAlert   = MessageBuffer[8]; break;
          case 0x02: CurrentProfile->Ringtone    = MessageBuffer[8]; break;
          case 0x03: CurrentProfile->Volume      = MessageBuffer[8]; break;
          case 0x04: CurrentProfile->MessageTone = MessageBuffer[8]; break;
          case 0x05: CurrentProfile->Vibration   = MessageBuffer[8]; break;
          case 0x06: CurrentProfile->WarningTone = MessageBuffer[8]; break;
          case 0x07: CurrentProfile->ScreenSaver = MessageBuffer[8]; break;	  
          default:
#ifdef DEBUG
	    fprintf(stdout,_("feature %i = value %i\n\n"),MessageBuffer[6],MessageBuffer[8]);
#endif
            break;
        }
        break;
      default:
        switch (MessageBuffer[6]) {
          case 0x00: CurrentProfile->KeypadTone      = MessageBuffer[8];break;
          case 0x01: CurrentProfile->Lights          = MessageBuffer[8];break;
          case 0x02: CurrentProfile->CallAlert       = MessageBuffer[8];break;
          case 0x03: CurrentProfile->Ringtone        = MessageBuffer[8];break;
          case 0x04: CurrentProfile->Volume          = MessageBuffer[8];break;
          case 0x05: CurrentProfile->MessageTone     = MessageBuffer[8];break;
          case 0x06: CurrentProfile->Vibration       = MessageBuffer[8];break;
          case 0x07: CurrentProfile->WarningTone     = MessageBuffer[8];break;
          case 0x08: CurrentProfile->CallerGroups    = MessageBuffer[8];break;
          case 0x09: CurrentProfile->AutomaticAnswer = MessageBuffer[8];break;
          default:
#ifdef DEBUG
	    fprintf(stdout,_("feature %i = value %i\n\n"),MessageBuffer[6],MessageBuffer[8]);
#endif
            break;
        }
        break;
    }

    CurrentProfileError = GE_NONE;
    break;

  /* Incoming profile name */
  case 0x1b:   

    if (MessageBuffer[9] == 0x00) {
      CurrentProfile->DefaultName=MessageBuffer[8];
    } else {
      CurrentProfile->DefaultName=-1;      
	
      /* Here name is in Unicode */
      if (GetModelFeature (FN_PROFILES)==F_PROF33) {
	DecodeUnicode (CurrentProfile->Name, MessageBuffer+10, MessageBuffer[9]/2);
      } else {
        /* ...here not */
        sprintf(CurrentProfile->Name, MessageBuffer + 10, MessageBuffer[9]);
        CurrentProfile->Name[MessageBuffer[9]] = '\0';
      }
    }

    CurrentProfileError = GE_NONE;
    break;

  }
}

/* Needs SIM card with PIN in phone */
GSM_Error N6110_GetProfile(GSM_Profile *Profile)
{
  int i;
  
  unsigned char name_req[] = { N6110_FRAME_HEADER, 0x1a, 0x00};
  unsigned char feat_req[] = { N6110_FRAME_HEADER, 0x13, 0x01, 0x00, 0x00};

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

  name_req[4] = Profile->Number;

  error=NULL_SendMessageSequence
    (20, &CurrentProfileError, 5, 0x05, name_req);
  if (error!=GE_NONE) return error;

  for (i = 0x00; i <= 0x09; i++) {

    feat_req[5] = Profile->Number;
    
    feat_req[6] = i;

    error=NULL_SendMessageSequence
      (20, &CurrentProfileError, 7, 0x05, feat_req);
    if (error!=GE_NONE) return error;
  }

  if (Profile->DefaultName > -1)
  {
    switch(GetModelFeature (FN_PROFILES)) {
      case F_PROF33:
        switch (Profile->DefaultName) {
          case 0x00: sprintf(Profile->Name, "General");break;
          case 0x01: sprintf(Profile->Name, "Silent");break;
          case 0x02: sprintf(Profile->Name, "Descreet");break;
          case 0x03: sprintf(Profile->Name, "Loud");break;
          case 0x04: sprintf(Profile->Name, "My style");break;
          case 0x05: Profile->Name[0]=0;break;
          default  : sprintf(Profile->Name, "Unknown (%i)", Profile->DefaultName);break;
	}
        break;
      case F_PROF51:
        switch (Profile->DefaultName) {
          case 0x00: sprintf(Profile->Name, "Personal");break;
          case 0x01: sprintf(Profile->Name, "Car");break;
          case 0x02: sprintf(Profile->Name, "Headset");break;
          default  : sprintf(Profile->Name, "Unknown (%i)", Profile->DefaultName);break;
        }
        break;
      case F_PROF61:
        switch (Profile->DefaultName) {
          case 0x00: sprintf(Profile->Name, "General");break;
          case 0x01: sprintf(Profile->Name, "Silent");break;
          case 0x02: sprintf(Profile->Name, "Meeting");break;
          case 0x03: sprintf(Profile->Name, "Outdoor");break;
          case 0x04: sprintf(Profile->Name, "Pager");break;
          case 0x05: sprintf(Profile->Name, "Car");break;
          case 0x06: sprintf(Profile->Name, "Headset");break;
          default  : sprintf(Profile->Name, "Unknown (%i)", Profile->DefaultName);break;
	}
        break;
    }
  }
  
  return (GE_NONE);

}

void N6110_ReplySetProfile(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch (MessageBuffer[3]) {

  /* Profile feature change result */
  case 0x11:   
#ifdef DEBUG
    fprintf(stdout, _("Message: Profile feature change result.\n"));
#endif /* DEBUG */
    CurrentProfileError = GE_NONE;
    break;

  /* Profile name set result */
  case 0x1d:   
#ifdef DEBUG
    fprintf(stdout, _("Message: Profile name change result.\n"));
#endif /* DEBUG */
    CurrentProfileError = GE_NONE;
    break;

  }
}

GSM_Error N6110_SetProfileFeature(u8 profile, u8 feature, u8 value)
{
  unsigned char feat_req[] = { N6110_FRAME_HEADER, 0x10, 0x01,
                               0x00, 0x00, 0x00};

  feat_req[5]=profile;
  feat_req[6]=feature;
  feat_req[7]=value;

  return NULL_SendMessageSequence
    (20, &CurrentProfileError, 8, 0x05, feat_req);
}

GSM_Error N6110_SetProfile(GSM_Profile *Profile)
{
  int i,value;

  unsigned char name_req[40] = { N6110_FRAME_HEADER, 0x1c, 0x01, 0x03,
                                 0x00, 0x00, 0x00};

  GSM_Error error;
  
  name_req[7] = Profile->Number;
  name_req[8] = strlen(Profile->Name);
  name_req[6] = name_req[8] + 2;

  for (i = 0; i < name_req[8]; i++)
    name_req[9 + i] = Profile->Name[i];

  error=NULL_SendMessageSequence
    (20, &CurrentProfileError, name_req[8] + 9, 0x05, name_req);
  if (error!=GE_NONE) return error;

  for (i = 0x00; i <= 0x09; i++) {

    switch (i) {
      case 0x00: value = Profile->KeypadTone; break;
      case 0x01: value = Profile->Lights; break;
      case 0x02: value = Profile->CallAlert; break;
      case 0x03: value = Profile->Ringtone; break;
      case 0x04: value = Profile->Volume; break;
      case 0x05: value = Profile->MessageTone; break;
      case 0x06: value = Profile->Vibration; break;
      case 0x07: value = Profile->WarningTone; break;
      case 0x08: value = Profile->CallerGroups; break;
      case 0x09: value = Profile->AutomaticAnswer; break;
      default  : value = 0; break;
    }

    error=N6110_SetProfileFeature(Profile->Number,i,value);
    if (error!=GE_NONE) return error;
  }

  return (GE_NONE);
}

bool N6110_SendRLPFrame(RLP_F96Frame *frame, bool out_dtx)
{
  u8 req[60] = { 0x00, 0xd9 };
		
  /* Discontinuos transmission (DTX).  See section 5.6 of GSM 04.22 version
     7.0.1. */
       
  if (out_dtx)
    req[1]=0x01;

  memcpy(req+2, (u8 *) frame, 32);

  return (Protocol->SendFrame(32, 0xf0, req));
}

void N6110_ReplyGetCalendarNote(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int i, j;

  u8 mychar1;

  wchar_t wc;  
    
  switch (MessageBuffer[4]) {

    case 0x01:
      
      CurrentCalendarNote->Type=MessageBuffer[8];

      DecodeDateTime(MessageBuffer+9, &CurrentCalendarNote->Time);

      DecodeDateTime(MessageBuffer+16, &CurrentCalendarNote->Alarm);

      CurrentCalendarNote->Text[0]=0;
      
      if (GetModelFeature (FN_CALENDAR)==F_CAL33) {
        i=0;
        if (CurrentCalendarNote->Type == GCN_REMINDER) i=1; //first char is subset
        switch (MessageBuffer[24]) {
          case 3:
#ifdef DEBUG
            fprintf(stdout,_("Subset 3 in reminder note !\n"));
#endif
            while (i!=MessageBuffer[23]) {
              j=0;
              if (i!=MessageBuffer[23]-1) {
	        if (MessageBuffer[24+i]>=0xc2) {
	          DecodeWithUTF8Alphabet(MessageBuffer[24+i], MessageBuffer[24+i+1], &mychar1);
                  CurrentCalendarNote->Text[strlen(CurrentCalendarNote->Text)+1]=0;
                  CurrentCalendarNote->Text[strlen(CurrentCalendarNote->Text)]=mychar1;
		  j=-1;
		  i++;
		}
  	      }
              if (j!=-1) {
                CurrentCalendarNote->Text[strlen(CurrentCalendarNote->Text)+1]=0;
                CurrentCalendarNote->Text[strlen(CurrentCalendarNote->Text)]=MessageBuffer[24+i];
              }
 	      i++;
            }
            break;
          case 2:
#ifdef DEBUG
            fprintf(stdout,_("Subset 2 in reminder note !\n"));
#endif
            while (i!=MessageBuffer[23]) {
	      wc = MessageBuffer[24+i] | (0x00 << 8);
              CurrentCalendarNote->Text[strlen(CurrentCalendarNote->Text)+1]=0;
              CurrentCalendarNote->Text[strlen(CurrentCalendarNote->Text)]=
		      DecodeWithUnicodeAlphabet(wc);
  	      i++;
            }
            break;
          case 1:
#ifdef DEBUG
            fprintf(stdout,_("Subset 1 in reminder note !\n"));
#endif
            memcpy(CurrentCalendarNote->Text,MessageBuffer+24+i,MessageBuffer[23]-i);
            CurrentCalendarNote->Text[MessageBuffer[23]-i]=0;
            break;
          default:
#ifdef DEBUG
            fprintf(stdout,_("Unknown subset in reminder note !\n"));
#endif
            memcpy(CurrentCalendarNote->Text,MessageBuffer+24+i,MessageBuffer[23]-i);
            CurrentCalendarNote->Text[MessageBuffer[23]-i]=0;
            break;
        }
      } else {
        memcpy(CurrentCalendarNote->Text,MessageBuffer+24,MessageBuffer[23]);
        CurrentCalendarNote->Text[MessageBuffer[23]]=0;
      }
      
      if (CurrentCalendarNote->Type == GCN_CALL) {
        memcpy(CurrentCalendarNote->Phone,MessageBuffer+24+MessageBuffer[23]+1,MessageBuffer[24+MessageBuffer[23]]);
        CurrentCalendarNote->Phone[MessageBuffer[24+MessageBuffer[23]]]=0;
      }

      CurrentCalendarNote->Recurrance=0;

      CurrentCalendarNote->AlarmType=0;

#ifdef DEBUG
      fprintf(stdout, _("Message: Calendar note received.\n"));

      fprintf(stdout, _("   Date: %d-%02d-%02d\n"), CurrentCalendarNote->Time.Year,
                                           CurrentCalendarNote->Time.Month,
                                           CurrentCalendarNote->Time.Day);

      fprintf(stdout, _("   Time: %02d:%02d:%02d\n"), CurrentCalendarNote->Time.Hour,
                                           CurrentCalendarNote->Time.Minute,
                                           CurrentCalendarNote->Time.Second);

      /* Some messages do not have alarm set up */
      if (CurrentCalendarNote->Alarm.Year != 0) {
        fprintf(stdout, _("   Alarm date: %d-%02d-%02d\n"), CurrentCalendarNote->Alarm.Year,
	                                         CurrentCalendarNote->Alarm.Month,
	                                         CurrentCalendarNote->Alarm.Day);

	fprintf(stdout, _("   Alarm time: %02d:%02d:%02d\n"), CurrentCalendarNote->Alarm.Hour,
	                                         CurrentCalendarNote->Alarm.Minute,
                                                 CurrentCalendarNote->Alarm.Second);
      }

      fprintf(stdout, _("   Type: %d\n"), CurrentCalendarNote->Type);
      fprintf(stdout, _("   Text: %s\n"), CurrentCalendarNote->Text);

      if (CurrentCalendarNote->Type == GCN_CALL)
        fprintf(stdout, _("   Phone: %s\n"), CurrentCalendarNote->Phone);
#endif /* DEBUG */

      CurrentCalendarNoteError=GE_NONE;
      break;

    case 0x93:

#ifdef DEBUG
      fprintf(stdout, _("Message: Calendar note not available\n"));
#endif /* DEBUG */

      CurrentCalendarNoteError=GE_INVALIDCALNOTELOCATION;
      break;

    default:

#ifdef DEBUG
      fprintf(stdout, _("Message: Calendar note error\n"));
#endif /* DEBUG */

      CurrentCalendarNoteError=GE_INTERNALERROR;
      break;

  }
}

GSM_Error N6110_GetCalendarNote(GSM_CalendarNote *CalendarNote)
{

  unsigned char req[] = { N6110_FRAME_HEADER,
                          0x66, 0x00
                        };
  GSM_Error error;

  req[4]=CalendarNote->Location;

  CurrentCalendarNote = CalendarNote;

  error=NULL_SendMessageSequence
    (20, &CurrentCalendarNoteError, 5, 0x13, req);

  CurrentCalendarNote = NULL;

  return error;
}

void N6110_ReplyWriteCalendarNote(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  switch(MessageBuffer[4]) {
    /* This message is also sent when the user enters the new entry on keypad */
    case 0x01:
      fprintf(stdout, _("Message: Calendar note write succesfull!\n"));break;      
    case 0x73:
      fprintf(stdout, _("Message: Calendar note write failed!\n"));break;
    case 0x7d:
      fprintf(stdout, _("Message: Calendar note write failed!\n"));break;
    default:
      fprintf(stdout, _("Unknown message of type 0x13 and subtype 0x65\n"));break;
  }
#endif

  switch(MessageBuffer[4]) {
    case 0x01: CurrentCalendarNoteError=GE_NONE; break;      
    case 0x73: CurrentCalendarNoteError=GE_INTERNALERROR; break;
    case 0x7d: CurrentCalendarNoteError=GE_INTERNALERROR; break;
    default  : AppendLogText("Unknown msg\n",false); break;
  }
}

GSM_Error N6110_WriteCalendarNote(GSM_CalendarNote *CalendarNote)
{

  unsigned char req[200] = { N6110_FRAME_HEADER,
                             0x64, 0x01, 0x10,
                             0x00, /* Length of the rest of the frame. */
                             0x00, /* The type of calendar note */
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                        };

  typedef struct {
    char *model;
    unsigned char call;
    unsigned char meeting;
    unsigned char birthday;
    unsigned char reminder;
  } calendar_model_length;
  
  /* Length of entries */
  calendar_model_length calendar_lengths[] =
  {
    /*model,CallTo,Meeting,Birthday,Reminder*/
    {"NHM-5",0x24,0x24,0x24,0x24}, //Reminder from phone, other quesses
    {"NHM-6",0x24,0x24,0x24,0x24}, //Reminder from phone, other quesses
    {"NSE-3",0x1e,0x14,0x14,0x1e}, //from NCDS3 [HKEY_LOCAL_MACHINE\Software\Nokia\Data Suite\3.0\Calendar]
    {"NSM-1",0x1e,0x18,0x18,0x24}, //from NCDS3 
    {"NSK-3",0x1e,0x14,0x14,0x1e}, //from NCDS3 
    {"NSB-3",0x20,0x14,0x14,0x1e}, //from NCDS3
    {"",     0,   0,   0,   0   }  //end of table
  };

  int i, j, current;

  u8 mychar;
  
  u8 mychar1,mychar2;
  
  GSM_Error error;
  
  /* Hopefully is 64 larger as FB38_MAX* / N6110_MAX* */
  char model[64];

  req[7]=CalendarNote->Type;

  EncodeDateTime(req+8, &CalendarNote->Time);
  req[14] = CalendarNote->Time.Timezone;

  if (CalendarNote->Alarm.Year) {
    EncodeDateTime(req+15, &CalendarNote->Alarm);
    req[21] = CalendarNote->Alarm.Timezone;
  }

  req[22]=strlen(CalendarNote->Text);
  
  current=23;

  if (GetModelFeature (FN_CALENDAR)==F_CAL33 && CalendarNote->Type==GCN_REMINDER) {
    req[22]++;           // one additional char
    req[current++]=0x01; //we use now subset 1
  }
    
  for (i=0; i<strlen(CalendarNote->Text); i++) {
    j=0;
    mychar=CalendarNote->Text[i];
    if (GetModelFeature (FN_CALENDAR)==F_CAL33 && CalendarNote->Type==GCN_REMINDER) {
      if (EncodeWithUTF8Alphabet(mychar,&mychar1,&mychar2)) {
          req[current++]=mychar1;
	  req[current++]=mychar2;
          req[23]=0x03; //use subset 3
          req[22]++;    // one additional char
	  j=-1;
      }
    }
    if (j!=-1) {
      /* Enables/disables blinking */
      if (mychar=='~') req[current++]=0x01;
                  else req[current++]=mychar;
    }
  }

  req[current++]=strlen(CalendarNote->Phone);

  for (i=0; i<strlen(CalendarNote->Phone); i++)
    req[current++]=CalendarNote->Phone[i];

  while (N6110_GetModel(model)  != GE_NONE)
    sleep(1);

  /* Checking maximal length */
  i=0;
  while (strcmp(calendar_lengths[i].model,"")) {
    if (!strcmp(calendar_lengths[i].model,model)) {
      switch (CalendarNote->Type) {
        case GCN_REMINDER:if (req[22]>calendar_lengths[i].reminder) return GE_TOOLONG;break;
        case GCN_MEETING :if (req[22]>calendar_lengths[i].meeting)  return GE_TOOLONG;break;
        case GCN_BIRTHDAY:if (req[22]>calendar_lengths[i].birthday) return GE_TOOLONG;break;
        case GCN_CALL    :if (strlen(CalendarNote->Phone)>calendar_lengths[i].call) return GE_TOOLONG;break;
      }
      break;
    }
    i++;
  }

  CurrentCalendarNote = CalendarNote;

  error=NULL_SendMessageSequence
    (20, &CurrentCalendarNoteError, current, 0x13, req);

  CurrentCalendarNote = NULL;

  return error;
}

void N6110_ReplyDeleteCalendarNote(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  switch (MessageBuffer[4]) {
    /* This message is also sent when the user deletes an old entry on
       keypad or moves an old entry somewhere (there is also `write'
       message). */
    case 0x01:fprintf(stdout, _("Message: Calendar note deleted\n"));break;
    case 0x93:fprintf(stdout, _("Message: Calendar note can't be deleted\n"));break;
    default  :fprintf(stdout, _("Message: Calendar note deleting error\n"));break;
  }
#endif

  switch (MessageBuffer[4]) {
    case 0x01:CurrentCalendarNoteError=GE_NONE;break;
    case 0x93:CurrentCalendarNoteError=GE_INVALIDCALNOTELOCATION;break;
    default  :CurrentCalendarNoteError=GE_INTERNALERROR;break;
  }
}

GSM_Error N6110_DeleteCalendarNote(GSM_CalendarNote *CalendarNote)
{

  unsigned char req[] = { N6110_FRAME_HEADER,
                          0x68, 0x00
                        };

  req[4]=CalendarNote->Location;

  return NULL_SendMessageSequence (20, &CurrentCalendarNoteError, 5, 0x13, req);
}

void N6110_ReplyRFBatteryLevel(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
    fprintf(stdout, _("Message: Phone status received:\n"));
    fprintf(stdout, _("   Mode: "));

    switch (MessageBuffer[4]) {

      case 0x01:

	fprintf(stdout, _("registered within the network\n"));
	break;
	      
      /* I was really amazing why is there a hole in the type of 0x02, now I
	 know... */
      case 0x02: fprintf(stdout, _("call in progress\n"));          break; /* ringing or already answered call */
      case 0x03: fprintf(stdout, _("waiting for security code\n")); break;
      case 0x04: fprintf(stdout, _("powered off\n"));               break;
      default  : fprintf(stdout, _("unknown\n"));

    }

    fprintf(stdout, _("   Power source: "));

    switch (MessageBuffer[7]) {

      case 0x01: fprintf(stdout, _("AC/DC\n"));   break;
      case 0x02: fprintf(stdout, _("battery\n")); break;
      default  : fprintf(stdout, _("unknown\n"));

    }

    fprintf(stdout, _("   Battery Level: %d\n"), MessageBuffer[8]);
    fprintf(stdout, _("   Signal strength: %d\n"), MessageBuffer[5]);
#endif /* DEBUG */

    CurrentRFLevel=MessageBuffer[5];
    CurrentBatteryLevel=MessageBuffer[8];
    CurrentPowerSource=MessageBuffer[7];
}


GSM_Error N6110_GetRFLevel(GSM_RFUnits *units, float *level)
{

  /* FIXME - these values are from 3810 code, may be incorrect.  Map from
     values returned in status packet to the the values returned by the AT+CSQ
     command. */
  float	csq_map[5] = {0, 8, 16, 24, 31};

  int timeout=10;
  int rf_level;
  
  char screen[NM_MAX_SCREEN_WIDTH];

  CurrentRFLevel=-1;
    
  if (GetModelFeature (FN_NOPOWERFRAME)==F_NOPOWER) {

    if (N6110_NetMonitor(1, screen)!=GE_NONE)
      return GE_INTERNALERROR;
    
    rf_level=4;
    
    if (screen[4]!='-') {
      if (screen[5]=='9' && screen[6]>'4') rf_level=1;
      if (screen[5]=='9' && screen[6]<'5') rf_level=2;
      if (screen[5]=='8' && screen[6]>'4') rf_level=3;      
    } else rf_level=0;

    /* Arbitrary units. */
    if (*units == GRF_Arbitrary) {
      *level = rf_level;
      return (GE_NONE);
    }
    
  } else {
    N6110_SendStatusRequest();

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
    if (*units == GRF_Arbitrary) {
      *level = rf_level;
      return (GE_NONE);
    }

    /* CSQ units. */
    if (*units == GRF_CSQ) {

      if (rf_level <=4)
        *level = csq_map[rf_level];
      else
        *level = 99; /* Unknown/undefined */

      return (GE_NONE);
    }
  }

  /* Unit type is one we don't handle so return error */
  return (GE_INTERNALERROR);
}


GSM_Error N6110_GetBatteryLevel(GSM_BatteryUnits *units, float *level)
{
  int timeout=10;
  int batt_level;

  char screen[NM_MAX_SCREEN_WIDTH];

  CurrentBatteryLevel=-1;

  if (GetModelFeature (FN_NOPOWERFRAME)==F_NOPOWER) {

    if (N6110_NetMonitor(23, screen)!=GE_NONE)
      return GE_NOLINK;
    
    batt_level=4;
    
    if (screen[29]=='7') batt_level=3;
    if (screen[29]=='5') batt_level=2;
    if (screen[29]=='2') batt_level=1;
    
    /* Only units we handle at present are GBU_Arbitrary */
    if (*units == GBU_Arbitrary) {
      *level = batt_level;
      return (GE_NONE);
    }

    return (GE_INTERNALERROR);    
    
  } else {
    N6110_SendStatusRequest();

    /* Wait for timeout or other error. */
    while (timeout != 0 && CurrentBatteryLevel == -1 ) {

      if (--timeout == 0)
        return (GE_TIMEOUT);

      usleep (100000);
    }

    /* Take copy in case it changes. */
    batt_level = CurrentBatteryLevel;

    if (batt_level != -1) {

      /* Only units we handle at present are GBU_Arbitrary */
      if (*units == GBU_Arbitrary) {
        *level = batt_level;
        return (GE_NONE);
      }

      return (GE_INTERNALERROR);
    }
    else
      return (GE_NOLINK);
  }
}

GSM_Error N6110_GetPowerSource(GSM_PowerSource *source)
{

  int timeout=10;

  char screen[NM_MAX_SCREEN_WIDTH];

  CurrentPowerSource=-1;

  if (GetModelFeature (FN_NOPOWERFRAME)==F_NOPOWER) {    

    if (N6110_NetMonitor(20, screen)!=GE_NONE)
      return GE_NOLINK;
    
    CurrentPowerSource=GPS_ACDC;

    if (screen[6]=='x') CurrentPowerSource=GPS_BATTERY;

    *source=CurrentPowerSource;        
    
    return GE_NONE;    
  } else {
    N6110_SendStatusRequest();

    /* Wait for timeout or other error. */
    while (timeout != 0 && CurrentPowerSource == -1 ) {

      if (--timeout == 0)
        return (GE_TIMEOUT);

      usleep (100000);
    }

    if (CurrentPowerSource != -1) {
      *source=CurrentPowerSource;
      return (GE_NONE);
    }
    else
      return (GE_NOLINK);
  }
}

void N6110_ReplyGetDisplayStatus(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int i;

  for (i=0; i<MessageBuffer[4];i++)
    if (MessageBuffer[2*i+6]==2)
      CurrentDisplayStatus|=1<<(MessageBuffer[2*i+5]-1);
    else
      CurrentDisplayStatus&= (0xff - (1<<(MessageBuffer[2*i+5]-1)));

#ifdef DEBUG
  fprintf(stdout, _("Call in progress: %s\n"), CurrentDisplayStatus & (1<<DS_Call_In_Progress)?"on":"off");
  fprintf(stdout, _("Unknown: %s\n"),          CurrentDisplayStatus & (1<<DS_Unknown)?"on":"off");
  fprintf(stdout, _("Unread SMS: %s\n"),       CurrentDisplayStatus & (1<<DS_Unread_SMS)?"on":"off");
  fprintf(stdout, _("Voice call: %s\n"),       CurrentDisplayStatus & (1<<DS_Voice_Call)?"on":"off");
  fprintf(stdout, _("Fax call active: %s\n"),  CurrentDisplayStatus & (1<<DS_Fax_Call)?"on":"off");
  fprintf(stdout, _("Data call active: %s\n"), CurrentDisplayStatus & (1<<DS_Data_Call)?"on":"off");
  fprintf(stdout, _("Keyboard lock: %s\n"),    CurrentDisplayStatus & (1<<DS_Keyboard_Lock)?"on":"off");
  fprintf(stdout, _("SMS storage full: %s\n"), CurrentDisplayStatus & (1<<DS_SMS_Storage_Full)?"on":"off");
#endif /* DEBUG */

  CurrentDisplayStatusError=GE_NONE;
}

GSM_Error N6110_GetDisplayStatus(int *Status) {

  unsigned char req[4]={ N6110_FRAME_HEADER, 0x51 };

  GSM_Error error;

  error=NULL_SendMessageSequence
    (10, &CurrentDisplayStatusError, 4, 0x0d, req);
  if (error!=GE_NONE) return error;
  
  *Status=CurrentDisplayStatus;

  return GE_NONE;
}

GSM_Error N6110_DialVoice(char *Number) {
/* This commented sequence doesn't work on N3210/3310/6210/7110 */
//  unsigned char req[64]={N6110_FRAME_HEADER, 0x01};
//  unsigned char req_end[]={0x05, 0x01, 0x01, 0x05, 0x81, 0x01, 0x00, 0x00, 0x01};
//  int i=0;
//  req[4]=strlen(Number);
//  for(i=0; i < strlen(Number) ; i++)
//   req[5+i]=Number[i];
//  memcpy(req+5+strlen(Number), req_end, 10);
//  return NULL_SendMessageSequence
//    (20, &CurrentDialVoiceError, 13+strlen(Number), 0x01, req);

  unsigned char req[64]={0x00,0x01,0x7c,
                         0x01}; //call command

  int i=0;			 
  
  GSM_Error error;

  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;

  for(i=0; i < strlen(Number) ; i++) req[4+i]=Number[i];
  
  req[4+i+1]=0;
  
  return NULL_SendMessageSequence
    (20, &CurrentDialVoiceError, 4+strlen(Number)+1, 0x40, req);  
}

/* Dial a data call - type specifies request to use: 
     type 0 should normally be used
     type 1 should be used when calling a digital line - corresponds to ats35=0
     Maybe one day we'll know what they mean!
*/
GSM_Error N6110_DialData(char *Number, char type, void (* callpassup)(char c))
{
	unsigned char req[100]   = { N6110_FRAME_HEADER, 0x01 };
	unsigned char *req_end;
	unsigned char req_end0[] = { 0x01,  /* make a data call = type 0x01 */
				     0x02,0x01,0x05,0x81,0x01,0x00,0x00,0x01,0x02,0x0a,
				     0x07,0xa2,0x88,0x81,0x21,0x15,0x63,0xa8,0x00,0x00 };
	unsigned char req_end1[] = { 0x01,
				     0x02,0x01,0x05,0x81,0x01,0x00,0x00,0x01,0x02,0x0a,
				     0x07,0xa1,0x88,0x89,0x21,0x15,0x63,0xa0,0x00,0x06,
				     0x88,0x90,0x21,0x48,0x40,0xbb };
	unsigned char req2[]     = { N6110_FRAME_HEADER, 0x42,0x05,0x01,
				     0x07,0xa2,0xc8,0x81,0x21,0x15,0x63,0xa8,0x00,0x00,
				     0x07,0xa3,0xb8,0x81,0x20,0x15,0x63,0x80,0x01,0x60 };
	unsigned char req3[]     = { N6110_FRAME_HEADER, 0x42,0x05,0x01,
				     0x07,0xa2,0x88,0x81,0x21,0x15,0x63,0xa8,0x00,0x00,
				     0x07,0xa3,0xb8,0x81,0x20,0x15,0x63,0x80 };
	unsigned char req4[]     = { N6110_FRAME_HEADER, 0x42,0x05,0x81,
				     0x07,0xa1,0x88,0x89,0x21,0x15,0x63,0xa0,0x00,0x06,
				     0x88,0x90,0x21,0x48,0x40,0xbb,0x07,0xa3,0xb8,0x81,
				     0x20,0x15,0x63,0x80 };

	int i = 0;
	u8 size;

	CurrentCallPassup=callpassup;

	switch (type) {
	case 0:
		req_end = req_end0;
		size = sizeof(req_end0);
		break;
	case 1:
		Protocol->SendMessage(sizeof(req3), 0x01, req3);
		usleep(1000000);
	        Protocol->SendMessage(sizeof(req4), 0x01, req4);
		usleep(1000000);
		req_end = req_end1;
		size = sizeof(req_end1);
		break;
	case -1:   /* Just used to set the call passup */
		return GE_NONE;
		break;
	default:
		req_end = req_end0;
		size = sizeof(req_end0);
		break;
	}

	req[4] = strlen(Number);

	for(i = 0; i < strlen(Number) ; i++)
		req[5+i] = Number[i];

	memcpy(req + 5 + strlen(Number), req_end, size);

	Protocol->SendMessage(5 + size + strlen(Number), 0x01, req);
        usleep(1000000);

	if (type != 1) {
          Protocol->SendMessage(26, 0x01, req2);
	  usleep(1000000);
        }

	return (GE_NONE);
}

GSM_Error N6110_GetIncomingCallNr(char *Number)
{

  if (*CurrentIncomingCall != ' ') {
    strcpy(Number, CurrentIncomingCall);
    return GE_NONE;
  }
  else
    return GE_BUSY;
}

GSM_Error N6110_CancelCall(void)
{
//  This frame & method works only on 61xx/51xx
//  unsigned char req[] = { N6110_FRAME_HEADER, 0x08, 0x00, 0x85};
//  req[4]=CurrentCallSequenceNumber;
//  Protocol->SendMessage(6, 0x01, req);
//  return GE_NONE;
 
  GSM_Error error;

  unsigned char req[]={0x00,0x01,0x7c,0x03};
    
  /* Checking */
  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;

  return NULL_SendMessageSequence (20, &CurrentDialVoiceError, 4, 0x40, req);   
}  

void N6110_ReplyEnterSecurityCode(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
    
  switch(MessageBuffer[3]) {

  case 0x0b:
#ifdef DEBUG
    fprintf(stdout, _("Message: Security code accepted.\n"));
#endif /* DEBUG */
    CurrentSecurityCodeError = GE_NONE;
    break;

  default:
#ifdef DEBUG
    fprintf(stdout, _("Message: Security code is wrong. You're not my big owner :-)\n"));
#endif /* DEBUG */
    CurrentSecurityCodeError = GE_INVALIDSECURITYCODE;
  }
}

GSM_Error N6110_EnterSecurityCode(GSM_SecurityCode SecurityCode)
{

  unsigned char req[15] = { N6110_FRAME_HEADER,
                            0x0a, /* Enter code request. */
                            0x00  /* Type of the entered code. */
                            };
  int i=0;

  req[4]=SecurityCode.Type;

  for (i=0; i<strlen(SecurityCode.Code);i++)
    req[5+i]=SecurityCode.Code[i];

  req[5+strlen(SecurityCode.Code)]=0x00;
  req[6+strlen(SecurityCode.Code)]=0x00;

  return NULL_SendMessageSequence
    (20, &CurrentSecurityCodeError, 7+strlen(SecurityCode.Code), 0x08, req);
}

void N6110_ReplyGetSecurityCodeStatus(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
    
  *CurrentSecurityCodeStatus = MessageBuffer[4];

#ifdef DEBUG
  fprintf(stdout, _("Message: Security Code status received: "));

  switch(*CurrentSecurityCodeStatus) {

    case GSCT_SecurityCode: fprintf(stdout, _("waiting for Security Code.\n")); break;
    case GSCT_Pin         : fprintf(stdout, _("waiting for PIN.\n")); break;
    case GSCT_Pin2        : fprintf(stdout, _("waiting for PIN2.\n")); break;
    case GSCT_Puk         : fprintf(stdout, _("waiting for PUK.\n")); break;
    case GSCT_Puk2        : fprintf(stdout, _("waiting for PUK2.\n")); break;
    case GSCT_None        : fprintf(stdout, _("nothing to enter.\n")); break;
    default               : fprintf(stdout, _("Unknown!\n"));
  }
      
#endif /* DEBUG */

  CurrentSecurityCodeError = GE_NONE;
}

GSM_Error N6110_GetSecurityCodeStatus(int *Status)
{

  unsigned char req[4] = { N6110_FRAME_HEADER,
                           0x07
                         };

  CurrentSecurityCodeStatus=Status;

  return NULL_SendMessageSequence
    (20, &CurrentSecurityCodeError, 4, 0x08, req);
}

void N6110_ReplyGetSecurityCode(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int i;
  
#ifdef DEBUG
  fprintf(stdout, _("Message: Security code received: "));
  switch (MessageBuffer[3]) {
    case GSCT_SecurityCode: fprintf(stdout, _("Security code"));break;
    case GSCT_Pin:  fprintf(stdout, _("PIN"));break;
    case GSCT_Pin2: fprintf(stdout, _("PIN2"));break;
    case GSCT_Puk:  fprintf(stdout, _("PUK"));break;
    case GSCT_Puk2: fprintf(stdout, _("PUK2"));break;
    default: fprintf(stdout, _("unknown !"));break;
  }
  if (MessageBuffer[4]==1) {
    fprintf(stdout, _(" allowed, value \""));
    if (MessageBuffer[3]==GSCT_SecurityCode) {
      for (i=0;i<5;i++) {fprintf(stdout, _("%c"), MessageBuffer[5+i]);}
    }
    if (MessageBuffer[3]==GSCT_Pin || MessageBuffer[3]==GSCT_Pin2 ||
	MessageBuffer[3]==GSCT_Puk || MessageBuffer[3]==GSCT_Puk2) {
      for (i=0;i<4;i++) {fprintf(stdout, _("%c"), MessageBuffer[5+i]);}
    }
    fprintf(stdout, _("\""));
  } else {
    fprintf(stdout, _(" not allowed"));  
  }
  fprintf(stdout, _("\n"));  
#endif /* DEBUG */
      
  if (CurrentSecurityCode->Type==MessageBuffer[3] /* We wanted this code */
          && MessageBuffer[4]==1) {                      /* It's allowed */
    if (MessageBuffer[3]==GSCT_SecurityCode) {
      for (i=0;i<5;i++) {CurrentSecurityCode->Code[i]=MessageBuffer[5+i];}
      CurrentSecurityCode->Code[5]=0;
    }
    if (MessageBuffer[3]==GSCT_Pin || MessageBuffer[3]==GSCT_Pin2 ||
	MessageBuffer[3]==GSCT_Puk || MessageBuffer[3]==GSCT_Puk2) {
      for (i=0;i<4;i++) {CurrentSecurityCode->Code[i]=MessageBuffer[5+i];}
      CurrentSecurityCode->Code[4]=0;
    }
    CurrentSecurityCodeError=GE_NONE;
  } else
    CurrentSecurityCodeError=GE_INVALIDSECURITYCODE;
}

GSM_Error N6110_GetSecurityCode(GSM_SecurityCode *SecurityCode)
{

  unsigned char req[4] = { 0x00,
                           0x01,0x6e, /* Get code request. */
			   0x00 };    /* Type of the requested code. */

  GSM_Error error;
  
  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;
  
  req[3]=SecurityCode->Type;

  CurrentSecurityCode=SecurityCode;

  return NULL_SendMessageSequence
    (20, &CurrentSecurityCodeError, 4, 0x40, req);
}

void N6110_ReplyPlayTone(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: answer for PlayTone frame\n"));      
#endif
      
  CurrentPlayToneError=GE_NONE;      
}

GSM_Error N6110_PlayTone(int Herz, u8 Volume)
{
  unsigned char req[6] = { 0x00,0x01,0x8f,
                           0x00,   /* Volume */
			   0x00,   /* HerzLo */
			   0x00 }; /* HerzHi */

  GSM_Error error;

  /* PlayTone wasn't used earlier */
  if (CurrentPlayToneError==GE_UNKNOWN) {
    if (CurrentConnectionType!=GCT_MBUS)
      CurrentDisableKeepAlive=true;

    error=N6110_EnableExtendedCommands(0x01);
    if (error!=GE_NONE) return error;
  }

  /* For Herz==255*255 we have silent */  
  if (Herz!=255*255) {
    req[3]=Volume;

    req[5]=Herz%256;
    req[4]=Herz/256;
  } else {
    req[3]=0;

    req[5]=0;
    req[4]=0;
  }

#ifdef WIN32
  /* For Herz==255*255 we have silent and additionaly
     we wait for phone answer - it's important for MBUS */
  if (Herz==255*255) {
    error=NULL_SendMessageSequence
      (20, &CurrentPlayToneError, 6, 0x40, req);

    CurrentPlayToneError=GE_UNKNOWN;
    CurrentDisableKeepAlive=false;

    if (error!=GE_NONE) return error;
  } else {
    Protocol->SendMessage(6,0x40,req);
  }
#else
  error=NULL_SendMessageSequence
    (20, &CurrentPlayToneError, 6, 0x40, req);

  /* For Herz==255*255 we wait for phone answer - it's important for MBUS */
  if (Herz==255*255) {
    CurrentPlayToneError=GE_UNKNOWN;
    CurrentDisableKeepAlive=false;
  }
  
  if (error!=GE_NONE) return error;

#endif
    
  return(GE_NONE);
}

void N6110_ReplyGetDateTime(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  if (MessageBuffer[4]==0x01) {
    DecodeDateTime(MessageBuffer+8, CurrentDateTime);

#ifdef DEBUG
    fprintf(stdout, _("Message: Date and time\n"));
    fprintf(stdout, _("   Time: %02d:%02d:%02d\n"), CurrentDateTime->Hour, CurrentDateTime->Minute, CurrentDateTime->Second);
    fprintf(stdout, _("   Date: %4d/%02d/%02d\n"), CurrentDateTime->Year, CurrentDateTime->Month, CurrentDateTime->Day);
#endif /* DEBUG */

    CurrentDateTime->IsSet=true;
  } else {

#ifdef DEBUG
    fprintf(stdout, _("Message: Date and time not set in phone\n"));
#endif

    CurrentDateTime->IsSet=false;
  }
      
  CurrentDateTimeError=GE_NONE;
}

GSM_Error N6110_GetDateTime(GSM_DateTime *date_time)
{
  return N6110_PrivGetDateTime(date_time,0x11);
}

GSM_Error N6110_PrivGetDateTime(GSM_DateTime *date_time, int msgtype)
{
  unsigned char req[] = {N6110_FRAME_HEADER, 0x62};

  CurrentDateTime=date_time;

  return NULL_SendMessageSequence
    (50, &CurrentDateTimeError, 4, msgtype, req);
}

void N6110_ReplyGetAlarm(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: Alarm\n"));
  fprintf(stdout, _("   Alarm: %02d:%02d\n"), MessageBuffer[9], MessageBuffer[10]);
  fprintf(stdout, _("   Alarm is %s\n"), (MessageBuffer[8]==2) ? _("on"):_("off"));
#endif /* DEBUG */

  CurrentAlarm->Hour=MessageBuffer[9];
  CurrentAlarm->Minute=MessageBuffer[10];
  CurrentAlarm->Second=0;

  CurrentAlarm->IsSet=(MessageBuffer[8]==2);

  CurrentAlarmError=GE_NONE;
}

GSM_Error N6110_GetAlarm(int alarm_number, GSM_DateTime *date_time)
{
  return N6110_PrivGetAlarm(alarm_number,date_time,0x11);
}

GSM_Error N6110_PrivGetAlarm(int alarm_number, GSM_DateTime *date_time, int msgtype)
{
  unsigned char req[] = {N6110_FRAME_HEADER, 0x6d};

  CurrentAlarm=date_time;

  return NULL_SendMessageSequence
    (50, &CurrentAlarmError, 4, msgtype, req);
}

void N6110_ReplyGetSMSCenter(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
  
  switch (MessageBuffer[3]) {

  case 0x34:

    CurrentMessageCenter->No=MessageBuffer[4];
    CurrentMessageCenter->Format=MessageBuffer[6];
    CurrentMessageCenter->Validity=MessageBuffer[8];
    sprintf(CurrentMessageCenter->Name, "%s", MessageBuffer+33);

    sprintf(CurrentMessageCenter->DefaultRecipient, "%s", GSM_UnpackSemiOctetNumber(MessageBuffer+9,false));

    sprintf(CurrentMessageCenter->Number, "%s", GSM_UnpackSemiOctetNumber(MessageBuffer+21,false));
      
#ifdef DEBUG
    fprintf(stdout, _("Message: SMS Center received:\n"));
    fprintf(stdout, _("   %d. SMS Center name is %s\n"), CurrentMessageCenter->No, CurrentMessageCenter->Name);
    fprintf(stdout, _("   SMS Center number is %s\n"), CurrentMessageCenter->Number);
    fprintf(stdout, _("   Default recipient number is %s\n"), CurrentMessageCenter->DefaultRecipient);
      
    fprintf(stdout, _("   SMS Center message format is "));

    switch (CurrentMessageCenter->Format) {

      case GSMF_Text  : fprintf(stdout, _("Text"));   break;
      case GSMF_Paging: fprintf(stdout, _("Paging")); break;
      case GSMF_Fax   : fprintf(stdout, _("Fax"));    break;
      case GSMF_Email : fprintf(stdout, _("Email"));  break;
      default         : fprintf(stdout, _("Unknown"));
    }

    fprintf(stdout, "\n");

    fprintf(stdout, _("   SMS Center message validity is "));

    switch (CurrentMessageCenter->Validity) {

      case GSMV_1_Hour  : fprintf(stdout, _("1 hour"));      break;
      case GSMV_6_Hours : fprintf(stdout, _("6 hours"));     break;
      case GSMV_24_Hours: fprintf(stdout, _("24 hours"));    break;
      case GSMV_72_Hours: fprintf(stdout, _("72 hours"));    break;
      case GSMV_1_Week  : fprintf(stdout, _("1 week"));      break;
      case GSMV_Max_Time: fprintf(stdout, _("Maximum time"));break;
      default           : fprintf(stdout, _("Unknown"));
    }

    fprintf(stdout, "\n");

#endif /* DEBUG */

    CurrentMessageCenterError=GE_NONE;

    break;

  case 0x35:

    /* Number of entries depends on SIM card */

#ifdef DEBUG
    fprintf(stdout, _("Message: SMS Center error received:\n"));
    fprintf(stdout, _("   The request for SMS Center failed.\n"));
#endif /* DEBUG */

    /* FIXME: appropriate error. */
    CurrentMessageCenterError=GE_INTERNALERROR;

    break;  

  }
}

/* This function sends to the mobile phone a request for the SMS Center */
GSM_Error N6110_GetSMSCenter(GSM_MessageCenter *MessageCenter)
{
  unsigned char req[] = { N6110_FRAME_HEADER, 0x33, 0x64,
                          0x00 /* SMS Center Number. */
                        };

  req[5]=MessageCenter->No;

  CurrentMessageCenter=MessageCenter;

  return NULL_SendMessageSequence
    (50, &CurrentMessageCenterError, 6, 0x02, req);
}

void N6110_ReplySetSMSCenter(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: SMS Center correctly set.\n"));
#endif
  CurrentMessageCenterError=GE_NONE;
}

/* This function set the SMS Center profile on the phone. */
GSM_Error N6110_SetSMSCenter(GSM_MessageCenter *MessageCenter)
{
  unsigned char req[64] = { N6110_FRAME_HEADER, 0x30, 0x64,
                            0x00, /* SMS Center Number. */
                            0x00, /* Unknown. */
                            0x00, /* SMS Message Format. */
                            0x00, /* Unknown. */
                            0x00, /* Validity. */
                            0,0,0,0,0,0,0,0,0,0,0,0, /* Default recipient number */
                            0,0,0,0,0,0,0,0,0,0,0,0 /* Message Center Number. */
                            /* Message Center Name. */
                          };

  req[5]=MessageCenter->No;
  req[7]=MessageCenter->Format;
  req[9]=MessageCenter->Validity;

  req[10]=GSM_PackSemiOctetNumber(MessageCenter->DefaultRecipient, req+11, false);

  req[22]=GSM_PackSemiOctetNumber(MessageCenter->Number, req+23, false);

  sprintf(req+34, "%s", MessageCenter->Name);

  CurrentMessageCenter=MessageCenter;

  return NULL_SendMessageSequence
    (50, &CurrentMessageCenterError, 35+strlen(MessageCenter->Name), 0x02, req);
}

void N6110_ReplyGetSMSStatus(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch (MessageBuffer[3]) {

  case 0x37:

#ifdef DEBUG
    fprintf(stdout, _("Message: SMS Status Received\n"));
    fprintf(stdout, _("   The number of messages: %d\n"), MessageBuffer[10]);
    fprintf(stdout, _("   Unread messages: %d\n"), MessageBuffer[11]);
#endif /* DEBUG */

    CurrentSMSStatus->UnRead = MessageBuffer[11];
    CurrentSMSStatus->Number = MessageBuffer[10];
    
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

GSM_Error N6110_GetSMSStatus(GSM_SMSStatus *Status)
{
  unsigned char req[] = {N6110_FRAME_HEADER, 0x36, 0x64};

  CurrentSMSStatus = Status;

  return NULL_SendMessageSequence
    (10, &CurrentSMSStatusError, 5, 0x14, req);
}

GSM_Error N6110_GetSMSFolders ( GSM_SMSFolders *folders)
{
  folders->number=2;

  strcpy(folders->Folder[0].Name,"Inbox");
  strcpy(folders->Folder[1].Name,"Outbox");
  
  return GE_NONE;
}

GSM_Error N6110_GetIMEI(char *imei)
{
  if (strlen(Current_IMEI)>0) {
    strncpy (imei, Current_IMEI, GSM_MAX_IMEI_LENGTH);
    return (GE_NONE);
  }
  else
    return (GE_TRYAGAIN);
}

GSM_Error N6110_GetRevision(char *revision)
{

  if (strlen(Current_Revision)>0) {
    strncpy (revision, Current_Revision, GSM_MAX_REVISION_LENGTH);
    return (GE_NONE);
  }
  else
    return (GE_TRYAGAIN);
}

GSM_Error N6110_GetModel(char *model)
{
  if (strlen(Current_Model)>0) {
    strncpy (model, Current_Model, GSM_MAX_MODEL_LENGTH);
    return (GE_NONE);
  }
  else
    return (GE_TRYAGAIN);
}

void N6110_ReplySetDateTime(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch (MessageBuffer[4]) {

    case 0x01:
#ifdef DEBUG
      fprintf(stdout, _("Message: Date and time set correctly\n"));
#endif /* DEBUG */
      CurrentSetDateTimeError=GE_NONE;
      break;
      
    default:
#ifdef DEBUG
      fprintf(stdout, _("Message: Date and time setting error\n"));
#endif /* DEBUG */
      CurrentSetDateTimeError=GE_INVALIDDATETIME;

  }
}

/* Needs SIM card with PIN in phone */
GSM_Error N6110_SetDateTime(GSM_DateTime *date_time)
{
  return N6110_PrivSetDateTime(date_time,0x11);
}

/* Needs SIM card with PIN in phone */
GSM_Error N6110_PrivSetDateTime(GSM_DateTime *date_time, int msgtype)
{

  unsigned char req[] = { N6110_FRAME_HEADER,
			  0x60, /* set-time subtype */
			  0x01, 0x01, 0x07, /* unknown */
			  0x00, 0x00, /* Year (0x07cf = 1999) */
			  0x00, 0x00, /* Month Day */
			  0x00, 0x00, /* Hours Minutes */
			  0x00 /* Unknown, but not seconds - try 59 and wait 1 sec. */
			};

  EncodeDateTime(req+7, date_time);

  return NULL_SendMessageSequence
    (20, &CurrentSetDateTimeError, 14, msgtype, req);
}

void N6110_ReplySetAlarm(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch (MessageBuffer[4]) {

    case 0x01:
#ifdef DEBUG
      fprintf(stdout, _("Message: Alarm set correctly\n"));
#endif /* DEBUG */
      CurrentSetAlarmError=GE_NONE;
      break;
      
    default:
#ifdef DEBUG
      fprintf(stdout, _("Message: Alarm setting error\n"));
#endif /* DEBUG */
      CurrentSetAlarmError=GE_INVALIDDATETIME;

  }
}

/* FIXME: we should also allow to set the alarm off :-) */
GSM_Error N6110_SetAlarm(int alarm_number, GSM_DateTime *date_time)
{
  return N6110_PrivSetAlarm(alarm_number,date_time, 0x11);
}

/* FIXME: we should also allow to set the alarm off :-) */
GSM_Error N6110_PrivSetAlarm(int alarm_number, GSM_DateTime *date_time, int msgtype)
{

  unsigned char req[] = { N6110_FRAME_HEADER,
			  0x6b, /* set-alarm subtype */
			  0x01, 0x20, 0x03, /* unknown */
			  0x02,       /* should be alarm on/off, but it don't works */
			  0x00, 0x00, /* Hours Minutes */
			  0x00 /* Unknown, but not seconds - try 59 and wait 1 sec. */
			};

  req[8] = date_time->Hour;
  req[9] = date_time->Minute;

  return NULL_SendMessageSequence
    (50, &CurrentSetAlarmError, 11, msgtype, req);
}

void N6110_ReplyGetMemoryLocation(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  /* Hopefully is 64 larger as FB38_MAX* / N6110_MAX* */
  char model[64];

  int i, tmp, count;
    
  switch (MessageBuffer[3]) {

  case 0x02:

    CurrentPhonebookEntry->Empty = true;

    count=MessageBuffer[5];
	  
#ifdef DEBUG
    fprintf(stdout, _("Message: Phonebook entry received:\n"));
    fprintf(stdout, _("   Name: "));

    for (tmp=0; tmp <count; tmp++)
    {
      if (MessageBuffer[6+tmp]==1) fprintf(stdout, "%c", '~'); else //enables/disables blinking
      if (MessageBuffer[6+tmp]==0) fprintf(stdout, "%c", '`'); else //hides rest ot contents
      fprintf(stdout, "%c", MessageBuffer[6+tmp]);
    }

    fprintf(stdout, "\n");
#endif /* DEBUG */

    while (N6110_GetModel(model)  != GE_NONE)
      sleep(1);
        
    if (GetModelFeature (FN_PHONEBOOK)==F_PBK33SIM ||
        GetModelFeature (FN_PHONEBOOK)==F_PBK33INT) {//pbk with Unicode
      DecodeUnicode (CurrentPhonebookEntry->Name, MessageBuffer+6, count/2);
      CurrentPhonebookEntry->Name[count/2] = 0x00;
    } else {
      memcpy(CurrentPhonebookEntry->Name, MessageBuffer + 6, count);
      CurrentPhonebookEntry->Name[count] = 0x00;
    }

    CurrentPhonebookEntry->Empty = false;

    for (tmp=0; tmp <count; tmp++)
    {
      if (GetModelFeature (FN_PHONEBOOK)==F_PBK33INT ||
          GetModelFeature (FN_PHONEBOOK)==F_PBK33SIM) {//pbk with Unicode
        /* We check only 1'st, 3'rd, ... char */
        if (tmp%2!=0 && MessageBuffer[6+tmp]==1) CurrentPhonebookEntry->Name[tmp/2]='~'; //enables/disables blinking
        if (tmp%2!=0 && MessageBuffer[6+tmp]==0) CurrentPhonebookEntry->Name[tmp/2]='`'; //hides rest ot contents
      } else {
        if (MessageBuffer[6+tmp]==1) CurrentPhonebookEntry->Name[tmp]='~'; //enables/disables blinking
        if (MessageBuffer[6+tmp]==0) CurrentPhonebookEntry->Name[tmp]='`'; //hides rest ot contents
      }
    }

    i=7+count;
    count=MessageBuffer[6+count];

#ifdef DEBUG
    fprintf(stdout, _("   Number: "));

    for (tmp=0; tmp <count; tmp++)
      fprintf(stdout, "%c", MessageBuffer[i+tmp]);

    fprintf(stdout, "\n");
#endif /* DEBUG */

    memcpy(CurrentPhonebookEntry->Number, MessageBuffer + i, count);
    CurrentPhonebookEntry->Number[count] = 0x00;
    CurrentPhonebookEntry->Group = MessageBuffer[i+count];
      
    /* Phone doesn't have entended phonebook */
    CurrentPhonebookEntry->SubEntriesCount = 0;

    /* But for these memories data is saved and we can save it using 7110/6210 style */
    if (CurrentPhonebookEntry->MemoryType==GMT_DC ||
        CurrentPhonebookEntry->MemoryType==GMT_RC ||
        CurrentPhonebookEntry->MemoryType==GMT_MC) {
        CurrentPhonebookEntry->SubEntriesCount = 1;
        CurrentPhonebookEntry->SubEntries[0].EntryType=N7110_ENTRYTYPE_DATE;
        CurrentPhonebookEntry->SubEntries[0].NumberType=0;
        CurrentPhonebookEntry->SubEntries[0].BlockNumber=1;
        DecodeDateTime(MessageBuffer+(i+count+2),&CurrentPhonebookEntry->SubEntries[0].data.Date);

#ifdef DEBUG
      fprintf(stdout, _("   Date: "));
      fprintf(stdout, "%02u.%02u.%04u\n",
          CurrentPhonebookEntry->SubEntries[0].data.Date.Day,
          CurrentPhonebookEntry->SubEntries[0].data.Date.Month,
          CurrentPhonebookEntry->SubEntries[0].data.Date.Year);
      fprintf(stdout, _("   Time: "));
      fprintf(stdout, "%02u:%02u:%02u\n",
          CurrentPhonebookEntry->SubEntries[0].data.Date.Hour,
          CurrentPhonebookEntry->SubEntries[0].data.Date.Minute,
          CurrentPhonebookEntry->SubEntries[0].data.Date.Second);
#endif /* DEBUG */

      /* These values are set, when date and time unavailable in phone.
         Values from 3310 - in other can be different */
      if (CurrentPhonebookEntry->SubEntries[0].data.Date.Day==20 &&
          CurrentPhonebookEntry->SubEntries[0].data.Date.Month==1 &&
          CurrentPhonebookEntry->SubEntries[0].data.Date.Year==2118 &&
          CurrentPhonebookEntry->SubEntries[0].data.Date.Hour==3 &&
	  CurrentPhonebookEntry->SubEntries[0].data.Date.Minute==14 &&
	  CurrentPhonebookEntry->SubEntries[0].data.Date.Second==7)
          CurrentPhonebookEntry->SubEntriesCount = 0;
    }

    /* Signal no error to calling code. */
    CurrentPhonebookError = GE_NONE;

    break;

  case 0x03:

#ifdef DEBUG
    fprintf(stdout, _("Message: Phonebook read entry error received:\n"));
#endif /* DEBUG */

    switch (MessageBuffer[4]) {

      case 0x7d:
#ifdef DEBUG
	fprintf(stdout, _("   Invalid memory type!\n"));
#endif /* DEBUG */
	CurrentPhonebookError = GE_INVALIDMEMORYTYPE;
	break;

      default:
#ifdef DEBUG
	fprintf(stdout, _("   Unknown error!\n"));
#endif /* DEBUG */
	CurrentPhonebookError = GE_INTERNALERROR;
    }

    break;

  }
}

/* Routine to get specifed phone book location.  Designed to be called by
   application.  Will block until location is retrieved or a timeout/error
   occurs. */
GSM_Error N6110_GetMemoryLocation(GSM_PhonebookEntry *entry)
{
  unsigned char req[] = {N6110_FRAME_HEADER, 0x01, 0x00, 0x00, 0x00};

  CurrentPhonebookEntry = entry;

  req[4] = N6110_GetMemoryType(entry->MemoryType);
  req[5] = entry->Location;

  return NULL_SendMessageSequence
    (50, &CurrentPhonebookError, 7, 0x03, req);
}

void N6110_ReplyWritePhonebookLocation(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch (MessageBuffer[3]) {

  case 0x05:

#ifdef DEBUG
    fprintf(stdout, _("Message: Phonebook written correctly.\n"));
#endif /* DEBUG */
    CurrentPhonebookError = GE_NONE;
    break;

  case 0x06:

    switch (MessageBuffer[4]) {
      /* FIXME: other errors? When I send the phonebook with index of 350 it
         still report error 0x7d :-( */
      case 0x7d:
#ifdef DEBUG
        fprintf(stdout, _("Message: Phonebook not written - name is too long.\n"));
#endif /* DEBUG */
	CurrentPhonebookError = GE_PHBOOKNAMETOOLONG;
	break;

      default:
#ifdef DEBUG
	fprintf(stdout, _("   Unknown error!\n"));
#endif /* DEBUG */
	CurrentPhonebookError = GE_INTERNALERROR;
    }
  }
}

/* Routine to write phonebook location in phone. Designed to be called by
   application code. Will block until location is written or timeout
   occurs. */
GSM_Error N6110_WritePhonebookLocation(GSM_PhonebookEntry *entry)
{
  unsigned char req[128] = { N6110_FRAME_HEADER, 0x04, 0x00, 0x00 };
  int i=0, current=0;

  req[4] = N6110_GetMemoryType(entry->MemoryType);
  req[5] = entry->Location;

  current=7;

  if (GetModelFeature (FN_PHONEBOOK)==F_PBK33INT ||
      GetModelFeature (FN_PHONEBOOK)==F_PBK33SIM) {

     req[6] = strlen(entry->Name)*2;

     EncodeUnicode (req+current,entry->Name ,strlen(entry->Name));
     
     for (i=0; i<strlen(entry->Name); i++)
     {
       /* here we encode "special" chars */
       if (entry->Name[i]=='~') req[current+i*2]=1; //enables/disables blinking
       if (entry->Name[i]=='`') req[current+i*2]=0; //hides rest ot contents
     }

     current+=strlen(entry->Name)*2;
  } else {

    req[6] = strlen(entry->Name);

    for (i=0; i<strlen(entry->Name); i++)
    {
      req[current+i] = entry->Name[i];

      /* here we encode "special" chars */
      if (entry->Name[i]=='~') req[current+i]=1; //enables/disables blinking
      if (entry->Name[i]=='`') req[current+i]=0; //hides rest ot contents
    }

    current+=strlen(entry->Name);
  }

  req[current++]=strlen(entry->Number);

  for (i=0; i<strlen(entry->Number); i++)
    req[current+i] = entry->Number[i];

  current+=strlen(entry->Number);

  /* Jano: This allow to save 14 characters name into SIM memory, when
     No Group is selected. */
  if (entry->Group == 5)
    req[current++]=0xff;
  else
    req[current++]=entry->Group;

  return NULL_SendMessageSequence
    (50, &CurrentPhonebookError, current, 0x03, req);
}

void N6110_ReplyNetmonitor(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch(MessageBuffer[3]) {

    case 0x00:
#ifdef DEBUG
      fprintf(stdout, _("Message: Netmonitor correctly set.\n"));
#endif /* DEBUG */
      CurrentNetmonitorError=GE_NONE;  
      break;
      
    default:
#ifdef DEBUG
      fprintf(stdout, _("Message: Netmonitor menu %d received:\n"), MessageBuffer[3]);
      fprintf(stdout, "%s\n", MessageBuffer+4);
#endif /* DEBUG */

      strcpy(CurrentNetmonitor, MessageBuffer+4);

      CurrentNetmonitorError=GE_NONE;  
  }
}

GSM_Error N6110_NetMonitor(unsigned char mode, char *Screen)
{
  unsigned char req[] = { 0x00, 0x01, 0x7e, 0x00 };
  
  GSM_Error error;
  
  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;

  CurrentNetmonitor=Screen;

  req[3]=mode;

  return NULL_SendMessageSequence
    (20, &CurrentNetmonitorError, 4, 0x40, req);
}

/* Doesn't work in N3210. */
/* In other allow to access phone menu without SIM card (just send any sequence) */
GSM_Error N6110_SendDTMF(char *String)
{
  unsigned char req[64] = { N6110_FRAME_HEADER, 0x50,
                            0x00 /* Length of DTMF string. */
                          };
			  
  u8 length=strlen(String);

  if (length>59) length=59;
  
  req[4] = length;
  
  memcpy(req+5,String,length);

  return NULL_SendMessageSequence
    (20, &CurrentSendDTMFError, 5+length, 0x01, req);
}

void N6110_ReplyGetSpeedDial(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch (MessageBuffer[3]) {

  case 0x17:

    switch (MessageBuffer[4]) {
      case 0x02: CurrentSpeedDialEntry->MemoryType = GMT_ME;
      default  : CurrentSpeedDialEntry->MemoryType = GMT_SM;
    }
      
    CurrentSpeedDialEntry->Location = MessageBuffer[5];

#ifdef DEBUG
    fprintf(stdout, _("Message: Speed dial entry received:\n"));
    fprintf(stdout, _("   Location: %d\n"), CurrentSpeedDialEntry->Location);
    fprintf(stdout, _("   MemoryType: %s\n"), N6110_MemoryType_String[CurrentSpeedDialEntry->MemoryType]);
    fprintf(stdout, _("   Number: %d\n"), CurrentSpeedDialEntry->Number);
#endif /* DEBUG */

    CurrentSpeedDialError=GE_NONE;
    break;

  case 0x18:

#ifdef DEBUG
    fprintf(stdout, _("Message: Speed dial entry error\n"));
#endif /* DEBUG */
    CurrentSpeedDialError=GE_INVALIDSPEEDDIALLOCATION;
    break;

  }
}

GSM_Error N6110_GetSpeedDial(GSM_SpeedDial *entry)
{

  unsigned char req[] = { N6110_FRAME_HEADER,
                          0x16,
                          0x00  /* The number of speed dial. */
                        };

  CurrentSpeedDialEntry = entry;

  req[4] = entry->Number;

  return NULL_SendMessageSequence
    (20, &CurrentSpeedDialError, 5, 0x03, req);
}

void N6110_ReplySetSpeedDial(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch (MessageBuffer[3]) {

  case 0x1a:

#ifdef DEBUG
    fprintf(stdout, _("Message: Speed dial entry set.\n"));
#endif /* DEBUG */
    CurrentSpeedDialError=GE_NONE;
    break;

  case 0x1b:

#ifdef DEBUG
    fprintf(stdout, _("Message: Speed dial entry setting error.\n"));
#endif /* DEBUG */
    CurrentSpeedDialError=GE_INVALIDSPEEDDIALLOCATION;
    break;

  }
}

GSM_Error N6110_SetSpeedDial(GSM_SpeedDial *entry)
{

  unsigned char req[] = { N6110_FRAME_HEADER,
                          0x19,
                          0x00, /* Number */
                          0x00, /* Memory Type */
                          0x00  /* Location */
                        };

  req[4] = entry->Number;

  switch (entry->MemoryType) {
    case GMT_ME: req[5] = 0x02;
    default    : req[5] = 0x03;
  }

  req[6] = entry->Location;

  return NULL_SendMessageSequence
    (20, &CurrentSpeedDialError, 7, 0x03, req);
}

/* This function finds parts of SMS in frame used in new Nokia phones
   in internal protocols (they're coded according to GSM 03.40), copies them
   to GSM_ETSISMSMessage and calls GSM_DecodeETSISMS to decode
   GSM_ETSISMSMessage to GSM_SMSMessage structure */
GSM_Error GSM_DecodeNokiaSMSFrame(GSM_SMSMessage *SMS, unsigned char *req, int length)
{
  SMS_MessageType PDU=SMS_Deliver;
  GSM_ETSISMSMessage ETSI;
  int offset=0,i;

  ETSI.firstbyte=req[12];

  /* See GSM 03.40 section 9.2.3.1 */
  if ((ETSI.firstbyte & 0x03) == 0x01) PDU=SMS_Submit;
  if ((ETSI.firstbyte & 0x03) == 0x02) PDU=SMS_Status_Report;

  switch (PDU) {
    case SMS_Submit       : offset=5;break;
    case SMS_Deliver      : offset=4;break;
    case SMS_Status_Report: offset=3;break;
    default:                break;
  }

  for (i=0;i<req[0]+1;i++)
    ETSI.SMSCNumber[i]=req[i];

  for (i=0;i<((req[12+offset]+1)/2+1)+1;i++)
    ETSI.Number[i]=req[i+12+offset];

  switch (PDU) {
    case SMS_Submit:
      ETSI.TPDCS=req[10+offset];
      ETSI.TPUDL=req[11+offset];
      ETSI.TPVP=0;  //no support for now
      ETSI.TPPID=0; //no support for now
      for(i=31+offset;i<length;i++)
        ETSI.MessageText[i-31-offset]=req[i];
      break;
    case SMS_Deliver:
      ETSI.TPDCS=req[10+offset];
      ETSI.TPUDL=req[11+offset];
      ETSI.TPPID=0; //no support for now
      for(i=31+offset;i<length;i++)
        ETSI.MessageText[i-31-offset]=req[i];
      for(i=0;i<7;i++)
        ETSI.DeliveryDateTime[i]=req[i+24+offset];
      break;
    case SMS_Status_Report:
      for(i=0;i<7;i++)
        ETSI.DeliveryDateTime[i]=req[i+24+offset];
      ETSI.TPStatus=req[14];
      for(i=0;i<7;i++)
        ETSI.SMSCDateTime[i]=req[i+34];
      break;
    default:
      break;
  }

  GSM_DecodeETSISMS(SMS, &ETSI);

  SMS->Name[0]=0;

  return GE_NONE;
}

void N6110_ReplyGetSMSMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int offset;
  
  switch (MessageBuffer[3]) {

  case 0x08:

    switch (MessageBuffer[7]) {

      case 0x00:
	CurrentSMSMessage->Type = GST_SMS;
        CurrentSMSMessage->folder=GST_INBOX;
	offset=4;
        break;

      case 0x01:
        CurrentSMSMessage->Type = GST_DR;
	CurrentSMSMessage->folder=GST_INBOX;
	offset=3;
        break;

      case 0x02:
	CurrentSMSMessage->Type = GST_SMS;
	CurrentSMSMessage->folder=GST_OUTBOX;
	offset=5;
        break;

      default:
        CurrentSMSMessage->Type = GST_UN;
	offset=4;
        break;

    }

    /* Field Short Message Status - MessageBuffer[4] seems not to be
       compliant with GSM 07.05 spec.
       Meaning     Nokia protocol	GMS spec
       ----------------------------------------------------
       MO Sent     0x05			0x07 or 0x01
       MO Not sent 0x07			0x06 or 0x00
       MT Read	   0x01			0x05 or 0x01
       MT Not read 0x03			0x04 or 0x00
       ----------------------------------------------------
       See GSM 07.05 section 2.5.2.6 and correct me if I'm wrong.
       
				         Pawel Kot */

    if (MessageBuffer[4] & 0x02) CurrentSMSMessage->Status = GSS_NOTSENTREAD;
                            else CurrentSMSMessage->Status = GSS_SENTREAD;

#ifdef DEBUG
    fprintf(stdout, _("Number: %d\n"), MessageBuffer[6]);

    if (CurrentSMSMessage->folder!=1) { //GST_OUTBOX
      fprintf(stdout, _("Message: Received SMS (mobile terminated)\n"));
    } else {
      fprintf(stdout, _("Message: Outbox message (mobile originated)\n"));
    }

    if (CurrentSMSMessage->Type == GST_DR) fprintf(stdout, _("   Delivery Report\n"));
    if (CurrentSMSMessage->Type == GST_UN) fprintf(stdout, _("   Unknown type\n"));

    if (CurrentSMSMessage->folder==1) { //GST_OUTBOX
      if (CurrentSMSMessage->Status) fprintf(stdout, _("   Sent\n"));
                                else fprintf(stdout, _("   Not sent\n"));
    } else {
      if (CurrentSMSMessage->Status) fprintf(stdout, _("   Read\n"));
                                else fprintf(stdout, _("   Not read\n"));
    }
#endif

    CurrentSMSPointer=GSM_DecodeNokiaSMSFrame(CurrentSMSMessage, MessageBuffer+8, MessageLength-8);

    CurrentSMSMessage->MemoryType = MessageBuffer[5];
    CurrentSMSMessage->MessageNumber = MessageBuffer[6];
 
    /* Signal no error to calling code. */
    CurrentSMSMessageError = GE_NONE;

#ifdef DEBUG
    fprintf(stdout, "\n");
#endif

    break;

  case 0x09:

    /* We have requested invalid or empty location. */

#ifdef DEBUG
    fprintf(stdout, _("Message: SMS reading failed\n"));

    switch (MessageBuffer[4]) {
      case 0x02:fprintf(stdout, _("   Invalid location!\n"));break;
      case 0x07:fprintf(stdout, _("   Empty SMS location.\n"));break;
      case 0x0c:fprintf(stdout, _("   No access to memory (no PIN on card ?)\n"));break;
      default  :fprintf(stdout, _("   Error code %i - please report it \n"),MessageBuffer[4]);break;
    }
#endif /* DEBUG */

    switch (MessageBuffer[4]) {
      case 0x02:CurrentSMSMessageError = GE_INVALIDSMSLOCATION;break;
      case 0x07:CurrentSMSMessageError = GE_EMPTYSMSLOCATION;break;
      case 0x0c:CurrentSMSMessageError = GE_NOACCESS;break;
      default  :CurrentSMSMessageError = GE_UNKNOWN;break;
    }

    break;

  }
}

GSM_Error N6110_GetSMSMessage(GSM_SMSMessage *message)
{

  unsigned char req[] = { N6110_FRAME_HEADER,
                          0x07,
                          0x02, /* Unknown */
                          0x00, /* Location */
                          0x01, 0x64};

  int timeout = 60;

  /* State machine code writes data to these variables when it comes in. */

  CurrentSMSMessage = message;
  CurrentSMSMessageError = GE_BUSY;

  req[5] = message->Location;

  /* Send request */
  Protocol->SendMessage(8, 0x02, req);

  /* Wait for timeout or other error. */
  while (timeout != 0 && (CurrentSMSMessageError == GE_BUSY || CurrentSMSMessageError == GE_SMSWAITING)) {

    if (--timeout == 0)
      return (GE_TIMEOUT);

    usleep (100000);
  }

  return (CurrentSMSMessageError);
}

void N6110_ReplyDeleteSMSMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: SMS deleted successfully.\n"));
#endif /* DEBUG */

  CurrentSMSMessageError = GE_NONE;	
}

GSM_Error N6110_DeleteSMSMessage(GSM_SMSMessage *message)
{
  unsigned char req[] = {N6110_FRAME_HEADER, 0x0a, 0x02, 0x00};

  req[5] = message->Location;

  return NULL_SendMessageSequence
    (50, &CurrentSMSMessageError, 6, 0x14, req);
}

/* FIXME: do we need more than SMS_Submit and SMS_Deliver ? */
GSM_Error GSM_EncodeNokiaSMSFrame(GSM_SMSMessage *SMS, unsigned char *req, int *length, SMS_MessageType PDU)
{
  GSM_ETSISMSMessage ETSI;
  int i,offset=0;

  GSM_EncodeETSISMS(SMS, &ETSI, PDU, length);

  /* Cleaning */
  for (i=0;i<36;i++) req[i]=0;

  req[12]=ETSI.firstbyte;

  for (i=0;i<ETSI.SMSCNumber[0]+1;i++)
    req[i]=ETSI.SMSCNumber[i];

  switch (PDU) {
    case SMS_Submit:
      offset=5;
      for (i=0;i<((ETSI.Number[0]+1)/2+1)+1;i++) req[i+12+offset]=ETSI.Number[i];
      req[10+offset]=ETSI.TPDCS;
      req[11+offset]=ETSI.TPUDL;
      req[24+offset]=ETSI.TPVP;
#ifdef DEBUG
//      fprintf(stdout,_("   First byte: %02x\n"),ETSI.firstbyte);
//      fprintf(stdout,_("   TP-VP: %02x\n"),ETSI.TPVP);
//      fprintf(stdout,_("   TP-DCS: %02x\n"),ETSI.TPDCS);
#endif
//    req[]=ETSI.TPPID;
      for(i=0;i<*length;i++) req[i+31+offset]=ETSI.MessageText[i];
      break;

    case SMS_Deliver:
      offset=4;
      for (i=0;i<((ETSI.Number[0]+1)/2+1)+1;i++) req[i+12+offset]=ETSI.Number[i];
      req[10+offset]=ETSI.TPDCS;
      req[11+offset]=ETSI.TPUDL;
//    req[]=ETSI.TPPID;
      for(i=0;i<*length;i++) req[i+31+offset]=ETSI.MessageText[i];
      for (i=0;i<7;i++) req[24+offset+i]=ETSI.DeliveryDateTime[i];
      break;
    default:
      break;
  }
  
  *length=*length+offset;
  
  return GE_NONE;
}

void N6110_ReplySendSMSMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
    
  switch (MessageBuffer[3]) {

  /* SMS message correctly sent to the network */
  case 0x02:
#ifdef DEBUG
    fprintf(stdout, _("Message: SMS Message correctly sent.\n"));
#endif /* DEBUG */
    CurrentSMSMessageError = GE_SMSSENDOK;
    break;

  /* SMS message send to the network failed */
  case 0x03:

#ifdef DEBUG
    fprintf(stdout, _("Message: Sending SMS Message failed, error: %i"),MessageBuffer[6]);
      
    switch (MessageBuffer[6]) {
      case 1: fprintf(stdout,_(" (info \"Number not in use\")"));break;
      case 21: fprintf(stdout,_(" (info \"Message not sent this time\")"));break;
      case 28: fprintf(stdout,_(" (info \"Number not in use\")"));break;
      case 38: fprintf(stdout,_(" (info \"Message not sent this time\")"));break;       case 50: fprintf(stdout,_(" (info \"Check operator services\")"));break;	
      case 96: fprintf(stdout,_(" (info \"Message sending failed\")"));break;	
      case 111: fprintf(stdout,_(" (info \"Message sending failed\")"));break;	
      case 166: fprintf(stdout,_(" (info \"Message sending failed\")"));break;	
      case 178: fprintf(stdout,_(" (info \"Message sending failed\")"));break;	
      case 252: fprintf(stdout,_(" (info \"Message sending failed\")"));break;	       case 253: fprintf(stdout,_(" (info \"Message sending failed\")"));break;	
    }

    fprintf(stdout,_("\n   For more details with errors see netmonitor manual (test 65) on www.marcin-wiacek.topnet.pl"));
    fprintf(stdout,_("\n   If know their meaning, GSM specs decribing them, contact with me on marcin-wiacek@topnet.pl. THX\n"));
#endif /* DEBUG */

    CurrentSMSMessageError = GE_SMSSENDFAILED;
    break;

  }
}

GSM_Error N6110_SendSMSMessage(GSM_SMSMessage *SMS)
{
  GSM_Error error;

  unsigned char req[256] = {
    N6110_FRAME_HEADER,
    0x01, 0x02, 0x00, /* SMS send request*/
  };

  int length;

  error=GSM_EncodeNokiaSMSFrame(SMS, req+6, &length, SMS_Submit);    
  if (error != GE_NONE) return error;

  return NULL_SendMessageSequence
    (200, &CurrentSMSMessageError, 42+length, 0x02, req);
}

void N6110_ReplySaveSMSMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch (MessageBuffer[3]) {

  case 0x05:
      
#ifdef DEBUG
    fprintf(stdout, _("SMS Message stored at %d\n"), MessageBuffer[5]);
#endif
      
    CurrentSMSMessage->MessageNumber=MessageBuffer[5];
      
    CurrentSMSMessageError = GE_NONE;
    break;

  case 0x06:
#ifdef DEBUG
    fprintf(stdout, _("SMS saving failed\n"));
    switch (MessageBuffer[4]) {
      case 0x02:fprintf(stdout, _("   All locations busy.\n"));break;
      case 0x03:fprintf(stdout, _("   Invalid location!\n"));break;
      default  :fprintf(stdout, _("   Unknown error.\n"));break;
    }
#endif      
      
    switch (MessageBuffer[4]) {
      case 0x02:CurrentSMSMessageError = GE_MEMORYFULL;break;
      case 0x03:CurrentSMSMessageError = GE_INVALIDSMSLOCATION;break;
      default  :CurrentSMSMessageError = GE_UNKNOWN;break;
    }
  }
}

/* GST_DR and GST_UN not supported ! */
GSM_Error N6110_SaveSMSMessage(GSM_SMSMessage *SMS)
{
  unsigned char req[256] = {
    N6110_FRAME_HEADER, 0x04, /* SMS save request*/
    0x00, /* SMS Status. Different for Inbox and Outbox */
    0x02, /* ?? */
    0x00, /* SMS Location */
    0x02, /* SMS Type */
  };

  int length;
  SMS_MessageType PDU;
  GSM_Error error;

  if (SMS->Location) req[6] = SMS->Location;
    
  if (SMS->folder==0) { /*Inbox*/
    req[4]=1;      /* SMS Status */
    req[7] = 0x00; /* SMS Type */
    PDU=SMS_Deliver;
  } else {
    req[4]=5;      /* SMS Status */
    req[7] = 0x02; /* SMS Type */
    PDU=SMS_Submit;
  }
  
  if (SMS->Status == GSS_NOTSENTREAD) req[4] |= 0x02;  

  error=GSM_EncodeNokiaSMSFrame(SMS, req+8, &length, PDU);  
  if (error != GE_NONE) return error;

  CurrentSMSMessage = SMS;

  return NULL_SendMessageSequence
    (70, &CurrentSMSMessageError, 39+length, 0x14, req);
}

void N6110_ReplySetCellBroadcast(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: Cell Broadcast enabled/disabled successfully.\n")); fflush (stdout);
#endif

  CurrentCBError = GE_NONE;
}

/* Enable and disable Cell Broadcasting */
GSM_Error N6110_EnableCellBroadcast(void)
{
  unsigned char req[] = {N6110_FRAME_HEADER, 0x20,
                         0x01, 0x01, 0x00, 0x00, 0x01, 0x01};

#ifdef DEBUG
  fprintf (stdout,"Enabling CB\n");
#endif

  CurrentCBMessage = (GSM_CBMessage *)malloc(sizeof (GSM_CBMessage));
  CurrentCBMessage->Channel = 0;
  CurrentCBMessage->New = false;
  strcpy (CurrentCBMessage->Message,"");

  return NULL_SendMessageSequence
    (10, &CurrentCBError, 10, 0x02, req);
}


GSM_Error N6110_DisableCellBroadcast(void)
{
  /* Should work, but not tested fully */

  unsigned char req[] = {N6110_FRAME_HEADER, 0x20,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; /*VERIFY*/

  return NULL_SendMessageSequence
    (10, &CurrentCBError, 10, 0x02, req);
}

void N6110_ReplyReadCellBroadcast(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int i, tmp;
  unsigned char output[160];
  
  CurrentCBMessage->Channel = MessageBuffer[7];
  CurrentCBMessage->New = true;
  tmp=GSM_UnpackEightBitsToSeven(0, MessageBuffer[9], MessageBuffer[9], MessageBuffer+10, output);

#ifdef DEBUG
  fprintf(stdout, _("Message: CB received.\n")); fflush (stdout);

  fprintf(stdout, _("Message: channel number %i\n"),MessageBuffer[7]);

  fflush (stdout);

  for (i=0; i<tmp;i++) {
    fprintf(stdout, "%c", DecodeWithDefaultAlphabet(output[i]));
  }

  fprintf(stdout, "\n");
#endif
   
  for (i=0; i<tmp; i++) {
    CurrentCBMessage->Message[i] = DecodeWithDefaultAlphabet(output[i]);
  }
  CurrentCBMessage->Message[i]=0;
}

GSM_Error N6110_ReadCellBroadcast(GSM_CBMessage *Message)
{
#ifdef DEBUG
   fprintf(stdout,"Reading CB\n");
#endif

  if (CurrentCBMessage != NULL) 
  {
    if (CurrentCBMessage->New == true)
    {
#ifdef DEBUG
  fprintf(stdout,"New CB received\n");
#endif
      Message->Channel = CurrentCBMessage->Channel;
      strcpy(Message->Message,CurrentCBMessage->Message);
      CurrentCBMessage->New = false;
      return (GE_NONE);
    }
  }
  return (GE_NONEWCBRECEIVED);
}

int N6110_MakeCallerGroupFrame(unsigned char *req,GSM_Bitmap Bitmap)
{
  int count=0;

  req[count++]=Bitmap.number;
  req[count++]=strlen(Bitmap.text);
  memcpy(req+count,Bitmap.text,req[count-1]);
  count+=req[count-1];
  req[count++]=Bitmap.ringtone;

  /* Setting for graphic:
     0x00 - Off
     0x01 - On
     0x02 - View Graphics
     0x03 - Send Graphics
     0x04 - Send via IR
     You can even set it higher but Nokia phones (my
     6110 at least) will not show you the name of this
     item in menu ;-)) Nokia is really joking here. */
  if (Bitmap.enabled) req[count++]=0x01;
                 else req[count++]=0x00;

  req[count++]=(Bitmap.size+4)>>8;
  req[count++]=(Bitmap.size+4)%0xff;
  req[count++]=0x00;  /* Future extensions! */
  req[count++]=Bitmap.width;
  req[count++]=Bitmap.height;
  req[count++]=0x01;  /* Just BW */
  memcpy(req+count,Bitmap.bitmap,Bitmap.size);

  return count+Bitmap.size;
}

int N6110_MakeOperatorLogoFrame(unsigned char *req,GSM_Bitmap Bitmap)
{
  int count=0;

  EncodeNetworkCode(req+count, Bitmap.netcode);
  count=count+3;

  req[count++]=(Bitmap.size+4)>>8;
  req[count++]=(Bitmap.size+4)%0xff;
  req[count++]=0x00;  /* Infofield */
  req[count++]=Bitmap.width;
  req[count++]=Bitmap.height;
  req[count++]=0x01;  /* Just BW */    
  memcpy(req+count,Bitmap.bitmap,Bitmap.size);

  return count+Bitmap.size;
}

int N6110_MakeStartupLogoFrame(unsigned char *req,GSM_Bitmap Bitmap)
{
  int count=0;

  req[count++]=0x01;
  req[count++]=Bitmap.height;
  req[count++]=Bitmap.width;
  memcpy(req+count,Bitmap.bitmap,Bitmap.size);

  return count+Bitmap.size;
}

/* Set a bitmap or welcome-note */
GSM_Error N6110_SetBitmap(GSM_Bitmap *Bitmap) {

  unsigned char req[600] = { N6110_FRAME_HEADER };
  u16 count=3;
  u8 textlen;
  
  int timeout=50;

  /* Direct uploading variables */
  GSM_MultiSMSMessage SMS;
  unsigned char buffer[1000] = {0x0c,0x01};
  GSM_NetworkInfo NetworkInfo;

  GSM_Error error;
 
  /* Uploading with preview */
  if (Bitmap->number==255 &&
     (Bitmap->type==GSM_OperatorLogo || Bitmap->type==GSM_CallerLogo)) {
    GSM_SaveBitmapToSMS(&SMS,Bitmap,false,false);
    memcpy(buffer+2,SMS.SMS[0].UDH,SMS.SMS[0].UDH[0]+1);

    memcpy(buffer+2+SMS.SMS[0].UDH[0]+1,SMS.SMS[0].MessageText,SMS.SMS[0].Length);

    buffer[2+SMS.SMS[0].UDH[0]+1+SMS.SMS[0].Length]=0x00;

    Protocol->SendMessage(2+SMS.SMS[0].UDH[0]+1+SMS.SMS[0].Length+1, 0x12, buffer);

    GSM->GetNetworkInfo(&NetworkInfo); //need to make something
    return GE_NONE; //no answer from phone
  }
 
  CurrentSetBitmapError = GE_BUSY;  
  
  switch (Bitmap->type) {
  case GSM_WelcomeNoteText:
  case GSM_DealerNoteText:
    req[count++]=0x18;
    req[count++]=0x01; /* Only one block */

    if (Bitmap->type==GSM_WelcomeNoteText)
      req[count++]=0x02; /* Welcome text */
    else
      req[count++]=0x03; /* Dealer Welcome Note */

    textlen=strlen(Bitmap->text);
    req[count++]=textlen;
    memcpy(req+count,Bitmap->text,textlen);
      
    count+=textlen;

    Protocol->SendMessage(count, 0x05, req);
    
    break;

  case GSM_StartupLogo:
    if (Bitmap->number==0) {

      /* For 33xx we first set animated logo to default */
      if (GetModelFeature (FN_STARTUP)==F_STANIM) {
        error=N6110_SetProfileFeature(0, 0x29, Bitmap->number);
        if (error!=GE_NONE) return error;
      }

      req[count++]=0x18;
      req[count++]=0x01; /* Only one block */
      count=count+N6110_MakeStartupLogoFrame(req+5,*Bitmap); 
      Protocol->SendMessage(count, 0x05, req);
    } else {
      return N6110_SetProfileFeature(0, 0x29, Bitmap->number);
    }
    break;

  case GSM_OperatorLogo:
    req[count++]=0x30;  /* Store Op Logo */
    req[count++]=0x01;  /* Location */
    count=count+N6110_MakeOperatorLogoFrame(req+5,*Bitmap); 
    Protocol->SendMessage(count, 0x05, req);
    break;

  case GSM_CallerLogo:
    req[count++]=0x13;
    count=count+N6110_MakeCallerGroupFrame(req+4,*Bitmap);
    Protocol->SendMessage(count, 0x03, req);
    break;

  case GSM_PictureImage:
    req[count++]=0x03;
    req[count++]=Bitmap->number;
    if (strcmp(Bitmap->Sender,"")) {
       req[count]=GSM_PackSemiOctetNumber(Bitmap->Sender, req+count+1,true);

       /* Convert number of semioctets to number of chars and add count */
       textlen=req[count];
       if (textlen % 2) textlen++;
       count+=textlen / 2 + 1;

       count++;
    } else {
      req[count++]=0x00;
      req[count++]=0x00;
    }
    req[count++]=0x00;
    req[count++]=strlen(Bitmap->text);
    memcpy(req+count,Bitmap->text,strlen(Bitmap->text));
    count+=strlen(Bitmap->text);
    req[count++]=0x00;
    req[count++]=Bitmap->width;
    req[count++]=Bitmap->height;
    req[count++]=0x01;
    memcpy(req+count,Bitmap->bitmap,Bitmap->size);
    Protocol->SendMessage(count+Bitmap->size, 0x47, req);
    break;

  case GSM_7110OperatorLogo:
  case GSM_7110StartupLogo:
  case GSM_6210StartupLogo:
    return GE_NOTSUPPORTED;

  case GSM_None:
    return GE_NONE;
  }

  /* Wait for timeout or other error. */
  while (timeout != 0 && CurrentSetBitmapError == GE_BUSY ) {
          
    if (--timeout == 0)
      return (GE_TIMEOUT);
                    
    usleep (100000);
  }

  return CurrentSetBitmapError;
}

/* Get a bitmap from the phone */
GSM_Error N6110_GetBitmap(GSM_Bitmap *Bitmap) {

  unsigned char req[10] = { N6110_FRAME_HEADER };
  u8 count=3;
  
  int timeout=100;
  
  CurrentGetBitmap=Bitmap; 
  CurrentGetBitmapError = GE_BUSY;  
  
  switch (CurrentGetBitmap->type) {
  case GSM_StartupLogo:
  case GSM_WelcomeNoteText:
  case GSM_DealerNoteText:
    req[count++]=0x16;
    Protocol->SendMessage(count, 0x05, req);
    break;
  case GSM_OperatorLogo:
    req[count++]=0x33;
    req[count++]=0x01; /* Location 1 */
    Protocol->SendMessage(count, 0x05, req);
    break;
  case GSM_CallerLogo:
    req[count++]=0x10;
    req[count++]=Bitmap->number;
    Protocol->SendMessage(count, 0x03, req);
    break;
  case GSM_PictureImage:
    req[count++]=0x01;
    req[count++]=Bitmap->number;
    Protocol->SendMessage(count, 0x47, req);
    break;
  case GSM_7110OperatorLogo:
  case GSM_7110StartupLogo:
  case GSM_6210StartupLogo:
  default:
    return GE_NOTSUPPORTED;
  }

  /* Wait for timeout or other error. */
  while (timeout != 0 && CurrentGetBitmapError == GE_BUSY ) {
          
    if (--timeout == 0)
      return (GE_TIMEOUT);
                    
    usleep (100000);
  }

  CurrentGetBitmap=NULL;

  return CurrentGetBitmapError;
}

void N6110_ReplySetRingtone(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch (MessageBuffer[3]) {

  /* Set ringtone OK */
  case 0x37:       
#ifdef DEBUG
    fprintf(stdout, _("Message: Ringtone set OK!\n"));
#endif  
    CurrentRingtoneError=GE_NONE; 
    break;      

  /* Set ringtone error */
  case 0x38:       
#ifdef DEBUG
    fprintf(stdout, _("Message: Ringtone setting error !\n"));
#endif  
    CurrentRingtoneError=GE_NOTSUPPORTED; 
    break;      
  }
}

GSM_Error N6110_SetRingTone(GSM_Ringtone *ringtone, int *maxlength)
{
  
  char req[FB61_MAX_RINGTONE_FRAME_LENGTH+10] =
      {N6110_FRAME_HEADER,
       0x36,
       0x00,  /* Location */
       0x00,0x78};

  int size=FB61_MAX_RINGTONE_FRAME_LENGTH;
 
  /* Variables for preview uploading */
  unsigned char buffer[FB61_MAX_RINGTONE_FRAME_LENGTH+50];
  unsigned char buffer2[20];
  GSM_NetworkInfo NetworkInfo;

  /* Setting ringtone with preview */
  if (ringtone->location==255) {
    buffer[0]=0x0c;
    buffer[1]=0x01;
    EncodeUDHHeader(buffer2, GSM_RingtoneUDH);
    memcpy(buffer+2,buffer2,buffer2[0]+1); //copying UDH
    *maxlength=GSM_PackRingtone(ringtone, buffer+2+buffer2[0]+1, &size); //packing ringtone
    Protocol->SendMessage(2+buffer2[0]+1+size, 0x12, buffer); //sending frame
    GSM->GetNetworkInfo(&NetworkInfo); //need to make something
    sleep(1);
    return GE_NONE; //no answer from phone
  }
  
  *maxlength=GSM_PackRingtone(ringtone, req+7, &size);

  req[4]=ringtone->location-1;

  return NULL_SendMessageSequence
    (50, &CurrentRingtoneError, (size+7), 0x05, req);
}

void N6110_ReplyGetBinRingtone(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int i;
  
  switch (MessageBuffer[4]) {
    case 0x00: /* location supported. We have ringtone */

      /* Binary format used in N6150 */
      if (MessageBuffer[5]==0x0c && MessageBuffer[6]==0x01 && MessageBuffer[7]==0x2c) {
#ifdef DEBUG
        fprintf(stdout,_("Message: ringtone \""));
#endif      

        /* Copying name */
        i=8;
        while (true) {
#ifdef DEBUG
          if (MessageBuffer[i]!=0)
            fprintf(stdout,_("%c"),MessageBuffer[i]);
#endif
          CurrentGetBinRingtone->name[i-8]=MessageBuffer[i];
          if (MessageBuffer[i]==0) break;
          i++;
        }

#ifdef DEBUG    
        fprintf(stdout,_("\" received from location %i\n"),MessageBuffer[3]+1);
#endif
      
        /* Looking for end */
        i=0;
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
        memcpy(CurrentGetBinRingtone->frame,MessageBuffer+3,i-3);
        CurrentGetBinRingtone->length=i-3;
      
        CurrentBinRingtoneError=GE_NONE;
        break;
      }
	  
      /* Binary format used in N3210 */
      if (MessageBuffer[5]==0x10 && MessageBuffer[6]==0x01 && MessageBuffer[7]==0x2c) {      

#ifdef DEBUG
        fprintf(stdout,_("Message: ringtone \""));
#endif      

        /* Copying name */
        i=8;
        while (true) {
#ifdef DEBUG
          if (MessageBuffer[i]!=0)
            fprintf(stdout,_("%c"),MessageBuffer[i]);
#endif
          CurrentGetBinRingtone->name[i-8]=MessageBuffer[i];
          if (MessageBuffer[i]==0) break;
          i++;
        }

#ifdef DEBUG    
        fprintf(stdout,_("\" received from location %i\n"),MessageBuffer[3]+1);
#endif

        /* Here changes to get full compatibility with binary format used in N6150 */
        MessageBuffer[3]=0;
        MessageBuffer[4]=0;
        MessageBuffer[5]=0x0c;
        MessageBuffer[6]=0x01;
        MessageBuffer[7]=0x2c;

        /* Looking for end */
        i=0;
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
        memcpy(CurrentGetBinRingtone->frame,MessageBuffer+3,i-3);

        CurrentGetBinRingtone->length=i-3;
	    
        CurrentBinRingtoneError=GE_NONE;	  
	break;
      }

      /* Copying frame */
      memcpy(CurrentGetBinRingtone->frame,MessageBuffer,MessageLength);

      CurrentGetBinRingtone->length=MessageLength;

#ifdef DEBUG    
      fprintf(stdout,_("Message: unknown binary format for ringtone received from location %i\n"),MessageBuffer[3]+1);
#endif
      CurrentBinRingtoneError=GE_UNKNOWNMODEL;
      break;

    default:

#ifdef DEBUG
      fprintf(stdout,_("Message: Phone doesn't support downloaded ringtones at location %i\n"),MessageBuffer[3]+1);
#endif

      CurrentBinRingtoneError=GE_INVALIDRINGLOCATION;  
  }
}

GSM_Error N6110_GetBinRingTone(GSM_BinRingtone *ringtone)
{
  unsigned char req[] = { 0x00,0x01,0x9e,
                          0x00 }; //location

  GSM_Error error;
  
  CurrentGetBinRingtone=ringtone;
  
  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;

  req[3]=ringtone->location-1;
  
  return NULL_SendMessageSequence
    (50, &CurrentBinRingtoneError, 4, 0x40, req);
}

void N6110_ReplySetBinRingtone(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch (MessageBuffer[4]) {
    case 0x00: /* location supported. We set ringtone */
#ifdef DEBUG
      fprintf(stdout,_("Message: downloaded ringtone set at location %i\n"),MessageBuffer[3]+1);
#endif
      CurrentBinRingtoneError=GE_NONE;
      break;

    default:
#ifdef DEBUG
      fprintf(stdout,_("Message: Phone doesn't support downloaded ringtones at location %i\n"),MessageBuffer[3]+1);
#endif
      CurrentBinRingtoneError=GE_NOTSUPPORTED;	  
      break;
  }
}

GSM_Error N6110_SetBinRingTone(GSM_BinRingtone *ringtone)
{
  unsigned char req[1000] = { 0x00,0x01,0xa0};

  GSM_Error error;

  GSM_BinRingtone ring;

  /* Must be sure, that can upload ringtone to this phone */
  ring.location=ringtone->location;
  error=N6110_GetBinRingTone(&ring);
  if (error!=GE_NONE) return error;
    
  error=N6110_EnableExtendedCommands(0x01);
  if (error!=GE_NONE) return error;
  
  memcpy(req+3,ringtone->frame,ringtone->length);

  req[3]=ringtone->location-1;
  
  return NULL_SendMessageSequence
    (50, &CurrentBinRingtoneError, ringtone->length+3, 0x40, req);
}

GSM_Error N6110_Reset(unsigned char type)
{  
  return N6110_EnableExtendedCommands(type);
}

void N6110_Dispatch0x01Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int tmp, count;
    	  
  switch (MessageBuffer[3]) {

  /* Unknown message - it has been seen after the 0x07 message (call
     answered). Probably it has similar meaning. If you can solve
     this - just mail me. Pavel Janík ml.

     The message looks like this:

     Msg Destination: PC
     Msg Source: Phone
     Msg Type: 01
     Msg Unknown: 00
     Msg Len: 0e

     Phone: [01 ][08 ][00 ] is the header of the frame

     [03 ] is the call message subtype

     [05 ] is the call sequence number

     [05 ] unknown 

     [00 ][01 ][03 ][02 ][91][00] are unknown but has been
     seen in the Incoming call message (just after the
     caller's name from the phonebook). But never change
     between phone calls :-(
  */

  /* This may mean sequence number of 'just made' call - CK */
  case 0x02:

#ifdef DEBUG
    fprintf(stdout, _("Message: Call message, type 0x02:"));
    fprintf(stdout, _("   Exact meaning not known yet, sorry :-(\n"));
#endif /* DEBUG */

    break;

  /* Possibly call OK */
  /* JD: I think that this means "call in progress" (incomming or outgoing) */
  case 0x03:
    
#ifdef DEBUG
    fprintf(stdout, _("Message: Call message, type 0x03:"));
    fprintf(stdout, _("   Sequence nr. of the call: %d\n"), MessageBuffer[4]);
    fprintf(stdout, _("   Exact meaning not known yet, sorry :-(\n"));
#endif /* DEBUG */
    
    CurrentCallSequenceNumber=MessageBuffer[4];
    CurrentIncomingCall[0]='D';
    if (CurrentCallPassup) CurrentCallPassup('D');

    break;

  /* Remote end has gone away before you answer the call.  Probably your
     mother-in-law or banker (which is worse?) ... */
  case 0x04:

#ifdef DEBUG
    fprintf(stdout, _("Message: Remote end hang up.\n"));
    fprintf(stdout, _("   Sequence nr. of the call: %d, error: %i"), MessageBuffer[4],MessageBuffer[6]);

    switch (MessageBuffer[6]) {
      case 28: fprintf(stdout,_(" (info \"Invalid phone number\")"));break;
      case 34: fprintf(stdout,_(" (info \"Network busy\")"));break;
      case 42: fprintf(stdout,_(" (info \"Network busy\")"));break;
      case 47: fprintf(stdout,_(" (info \"Error in connection\")"));break;
      case 50: fprintf(stdout,_(" (info \"Check operator services\")"));break;       case 76: fprintf(stdout,_(" (info \"Check operator services\")"));break;
      case 111: fprintf(stdout,_(" (info \"Error in connection\")"));break;
    }
      
    fprintf(stdout,_("\n   For more details with errors see netmonitor manual (test 39) on www.marcin-wiacek.topnet.pl"));
    fprintf(stdout,_("\n   If know their meaning, GSM specs decribing them, contact with me on marcin-wiacek@topnet.pl. THX\n"));
#endif /* DEBUG */

    CurrentIncomingCall[0] = ' ';
    if (CurrentCallPassup) CurrentCallPassup(' ');

    break;

  /* Incoming call alert */
  case 0x05:

#ifdef DEBUG
    fprintf(stdout, _("Message: Incoming call alert:\n"));

    /* We can have more then one call ringing - we can distinguish between
       them */

    fprintf(stdout, _("   Sequence nr. of the call: %d\n"), MessageBuffer[4]);
    fprintf(stdout, _("   Number: "));

    count=MessageBuffer[6];

    for (tmp=0; tmp <count; tmp++)
      fprintf(stdout, "%c", MessageBuffer[7+tmp]);

    fprintf(stdout, "\n");

    fprintf(stdout, _("   Name: "));

    for (tmp=0; tmp <MessageBuffer[7+count]; tmp++)
      fprintf(stdout, "%c", MessageBuffer[8+count+tmp]);

    fprintf(stdout, "\n");
#endif /* DEBUG */

    count=MessageBuffer[6];

    CurrentIncomingCall[0] = 0;
    for (tmp=0; tmp <count; tmp++)
      sprintf(CurrentIncomingCall, "%s%c", CurrentIncomingCall, MessageBuffer[7+tmp]);

    break;

  /* Call answered. Probably your girlfriend...*/
  case 0x07:

#ifdef DEBUG
    fprintf(stdout, _("Message: Call answered.\n"));
    fprintf(stdout, _("   Sequence nr. of the call: %d\n"), MessageBuffer[4]);
#endif /* DEBUG */

    break;

  /* Call ended. Girlfriend is girlfriend, but time is money :-) */
  case 0x09:

#ifdef DEBUG
    fprintf(stdout, _("Message: Call ended by your phone.\n"));
    fprintf(stdout, _("   Sequence nr. of the call: %d\n"), MessageBuffer[4]);
#endif /* DEBUG */

    break;

  /* This message has been seen with the message of subtype 0x09
     after I hang the call.

  Msg Destination: PC
  Msg Source: Phone
  Msg Type: 01 
  Msg Unknown: 00
  Msg Len: 08
  Phone: [01 ][08 ][00 ][0a ][04 ][87 ][01 ][42B][1a ][c2 ]

  What is the meaning of 87? Can you spell some magic light into
  this issue?

  */

  /* Probably means call over - CK */
  case 0x0a:

#ifdef DEBUG
    fprintf(stdout, _("Message: Call message, type 0x0a:"));
    fprintf(stdout, _("   Sequence nr. of the call: %d\n"), MessageBuffer[4]);
    fprintf(stdout, _("   Exact meaning not known yet, sorry :-(\n"));
#endif /* DEBUG */

    CurrentIncomingCall[0] = ' ';
    if (CurrentCallPassup) CurrentCallPassup(' ');

    break;

  case 0x40:

#ifdef DEBUG
      fprintf(stdout, _("Message: Answer for send DTMF or dial voice command\n"));
#endif

    if (CurrentSendDTMFError!=GE_NONE) CurrentSendDTMFError=GE_NONE;

    if (CurrentDialVoiceError!=GE_NONE) CurrentDialVoiceError=GE_NONE;

    break;
     
  default:

#ifdef DEBUG
    fprintf(stdout, _("Message: Unknown message of type 0x01\n"));
#endif /* DEBUG */
    AppendLogText("Unknown msg\n",false);

    break;	/* Visual C Don't like empty cases */
  }
}

void N6110_Dispatch0x03Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int tmp, count;
    
  switch (MessageBuffer[3]) {

  case 0x04:

    /* AFAIK, this frame isn't used anywhere - it's rather for testing :-) */
    /* If you want see, if it works with your phone make something like that: */

    /* unsigned char connect5[] = {N6110_FRAME_HEADER, 0x03}; */
    /* Protocol->SendMessage(4, 0x04, connect5); */

    /*                                        Marcin-Wiacek@TopNet.PL */
    
#ifdef WIN32
    sprintf(Current_IMEI, "%s", MessageBuffer+5);
    sprintf(Current_Model, "%s", MessageBuffer+21);
    sprintf(Current_Revision, "SW%s, HW%s", MessageBuffer+41, MessageBuffer+35);
#else
    snprintf(Current_IMEI, GSM_MAX_IMEI_LENGTH, "%s", MessageBuffer+5);
    snprintf(Current_Model, GSM_MAX_MODEL_LENGTH, "%s", MessageBuffer+21);
    snprintf(Current_Revision, GSM_MAX_REVISION_LENGTH, "SW%s, HW%s", MessageBuffer+41, MessageBuffer+35);
#endif

#ifdef DEBUG
    fprintf(stdout, _("Message: Mobile phone identification received:\n"));
    fprintf(stdout, _("   IMEI: %s\n"), Current_IMEI);
    fprintf(stdout, _("   Model: %s\n"), Current_Model);
    fprintf(stdout, _("   Production Code: %s\n"), MessageBuffer+27);
    fprintf(stdout, _("   HW: %s\n"), MessageBuffer+35);
    fprintf(stdout, _("   Firmware: %s\n"), MessageBuffer+41);
#endif /* DEBUG */

    break;

  /* Get group data */    
  /* [ID],[name_len],[name].,[ringtone],[graphicon],[lenhi],[lenlo],[bitmap] */
  case 0x11:   
 
    if (CurrentGetBitmap!=NULL) {
      if (CurrentGetBitmap->number==MessageBuffer[4]) {
        count=MessageBuffer[5];
        memcpy(CurrentGetBitmap->text,MessageBuffer+6,count);
        CurrentGetBitmap->text[count]=0;

#ifdef DEBUG	
	fprintf(stdout, _("Message: Caller group datas\n"));
	fprintf(stdout, _("Caller group name: %s\n"),CurrentGetBitmap->text);
#endif /* DEBUG */

	count+=6;

	CurrentGetBitmap->ringtone=MessageBuffer[count++];
#ifdef DEBUG	
        fprintf(stdout, _("Caller group ringtone ID: %i"),CurrentGetBitmap->ringtone);
        if (CurrentGetBitmap->ringtone==16) fprintf(stdout,_(" (default)"));
	fprintf(stdout,_("\n"));
#endif /* DEBUG */

	CurrentGetBitmap->enabled=(MessageBuffer[count++]==1);
#ifdef DEBUG	
	fprintf(stdout, _("Caller group logo "));
	if (CurrentGetBitmap->enabled)
	  fprintf(stdout, _("enabled \n"));
	else
	  fprintf(stdout, _("disabled \n"));
#endif /* DEBUG */	

	CurrentGetBitmap->size=MessageBuffer[count++]<<8;
	CurrentGetBitmap->size+=MessageBuffer[count++];
#ifdef DEBUG	
	fprintf(stdout, _("Bitmap size=%i\n"),CurrentGetBitmap->size);
#endif /* DEBUG */

	count++;
	CurrentGetBitmap->width=MessageBuffer[count++];
        CurrentGetBitmap->height=MessageBuffer[count++];
	count++;
	tmp=GSM_GetBitmapSize(CurrentGetBitmap);
	if (CurrentGetBitmap->size>tmp) CurrentGetBitmap->size=tmp;
	memcpy(CurrentGetBitmap->bitmap,MessageBuffer+count,CurrentGetBitmap->size);
	CurrentGetBitmapError=GE_NONE;
      } else {
#ifdef DEBUG	
	fprintf(stdout, _("Message: Caller group datas received, but group number does not match (%i is not %i)\n"),MessageBuffer[4],CurrentGetBitmap->number);
#endif
      }
    } else {
#ifdef DEBUG
      fprintf(stdout, _("Message: Caller group data received but not requested!\n"));
#endif
    }
    break;

  /* Get group data error */
  case 0x12:   
      
    CurrentGetBitmapError=GE_UNKNOWN;   
#ifdef DEBUG
    fprintf(stdout, _("Message: Error attempting to get caller group data.\n"));
#endif   
    break;

  /* Set group data OK */      
  case 0x14:   
      
    CurrentSetBitmapError=GE_NONE;      
#ifdef DEBUG
    fprintf(stdout, _("Message: Caller group data set correctly.\n"));
#endif
    break;

  /* Set group data error */
  case 0x15:   
      
    CurrentSetBitmapError=GE_UNKNOWN;      
#ifdef DEBUG
    fprintf(stdout, _("Message: Error attempting to set caller group data\n"));
#endif
    break;  
  
  default:

#ifdef DEBUG
    fprintf(stdout, _("Message: Unknown message of type 0x03\n"));
#endif /* DEBUG */
    AppendLogText("Unknown msg\n",false);

    break;	/* Visual C Don't like empty cases */
  }
}

void N6110_Dispatch0x05Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int tmp, count, length;
  bool issupported;

#ifdef DEBUG
  int i;
#endif

  switch (MessageBuffer[3]) {

  /* Startup Logo */
  case 0x17:  

#ifdef DEBUG
    fprintf(stdout, _("Message: Startup Logo, welcome note and dealer welcome note received.\n"));
#endif

    if (CurrentGetBitmap!=NULL) {
       
      issupported=false;
       
      count=5;
       
      for (tmp=0;tmp<MessageBuffer[4];tmp++){
        switch (MessageBuffer[count++]) {
        case 0x01:
          if (CurrentGetBitmap->type==GSM_StartupLogo) {
            CurrentGetBitmap->height=MessageBuffer[count++];
	    CurrentGetBitmap->width=MessageBuffer[count++];
	    CurrentGetBitmap->size=GSM_GetBitmapSize(CurrentGetBitmap);
	    length=CurrentGetBitmap->size;
	    memcpy(CurrentGetBitmap->bitmap,MessageBuffer+count,length);
          } else {
            //bitmap size
	    length=MessageBuffer[count++];
	    length=length*MessageBuffer[count++]/8;
	  }
	  count+=length;
#ifdef DEBUG
	  fprintf(stdout, _("Startup logo supported - "));
	  if (length!=0) { fprintf(stdout, _("currently set\n"));   }
                    else { fprintf(stdout, _("currently empty\n")); }
#endif
          if (CurrentGetBitmap->type==GSM_StartupLogo) issupported=true;
	  break;
	case 0x02:
	  length=MessageBuffer[count];
	  if (CurrentGetBitmap->type==GSM_WelcomeNoteText) {
            memcpy(CurrentGetBitmap->text,MessageBuffer+count+1,length);
	    CurrentGetBitmap->text[length]=0;
	  }
#ifdef DEBUG
          fprintf(stdout, _("Startup Text supported - "));
          if (length!=0)
	  {
	    fprintf(stdout, _("currently set to \""));
	    for (i=0;i<length;i++) fprintf(stdout, _("%c"),MessageBuffer[count+1+i]);
	    fprintf(stdout, _("\"\n"));
	  } else {
            fprintf(stdout, _("currently empty\n"));
	  }
#endif
	  count+=length+1;
          if (CurrentGetBitmap->type==GSM_WelcomeNoteText) issupported=true;
	  break;
	case 0x03:
	  length=MessageBuffer[count];
	  if (CurrentGetBitmap->type==GSM_DealerNoteText) {
            memcpy(CurrentGetBitmap->text,MessageBuffer+count+1,length);
	    CurrentGetBitmap->text[length]=0;
	  }
#ifdef DEBUG
  	  fprintf(stdout, _("Dealer Welcome supported - "));
	  if (length!=0)
	  {
	    fprintf(stdout, _("currently set to \""));
	    for (i=0;i<length;i++) fprintf(stdout, _("%c"),MessageBuffer[count+1+i]);
	    fprintf(stdout, _("\"\n"));
	  } else {
            fprintf(stdout, _("currently empty\n"));
	  }
#endif
	  count+=length+1;
          if (CurrentGetBitmap->type==GSM_DealerNoteText) issupported=true;
	  break;
        }
      }
      if (issupported) CurrentGetBitmapError=GE_NONE;
                  else CurrentGetBitmapError=GE_NOTSUPPORTED;
    } else {
#ifdef DEBUG
      fprintf(stdout, _("Message: Startup logo received but not requested!\n"));
#endif
    }
    break;

  /* Set startup OK */
  case 0x19:   
    
    CurrentSetBitmapError=GE_NONE;    
#ifdef DEBUG
    fprintf(stdout, _("Message: Startup logo, welcome note or dealer welcome note correctly set.\n"));
#endif  
    break;      

  /* Set Operator Logo OK */
  case 0x31:   
      
#ifdef DEBUG
    fprintf(stdout, _("Message: Operator logo correctly set.\n"));
#endif  

    CurrentSetBitmapError=GE_NONE;      
    break;

  /* Set Operator Logo Error */      
  case 0x32:  
      
#ifdef DEBUG
    fprintf(stdout, _("Message: Error setting operator logo!\n"));
#endif

    CurrentSetBitmapError=GE_UNKNOWN;        
    break;

  /* Operator Logo */
  /* [location],[netcode x 3],[lenhi],[lenlo],[bitmap] */ 
  case 0x34:
 
    if (CurrentGetBitmap!=NULL) {

      count=5;  /* Location ignored. */

      DecodeNetworkCode(MessageBuffer+count, CurrentGetBitmap->netcode);
      count=count+3;

#ifdef DEBUG
      fprintf(stdout, _("Message: Operator Logo for %s (%s) network received.\n"),
	                   CurrentGetBitmap->netcode,
	                   GSM_GetNetworkName(CurrentGetBitmap->netcode));
#endif  

      CurrentGetBitmap->size=MessageBuffer[count++]<<8;
      CurrentGetBitmap->size+=MessageBuffer[count++];
      count++;
      CurrentGetBitmap->width=MessageBuffer[count++];
      CurrentGetBitmap->height=MessageBuffer[count++];
      count++;
      tmp=GSM_GetBitmapSize(CurrentGetBitmap);
      if (CurrentGetBitmap->size>tmp) CurrentGetBitmap->size=tmp;
      memcpy(CurrentGetBitmap->bitmap,MessageBuffer+count,CurrentGetBitmap->size);
      CurrentGetBitmapError=GE_NONE;
    } else {
#ifdef DEBUG
      fprintf(stdout, _("Message: Operator logo received but not requested!\n"));
#endif
    }
      
    break;

  /* Get op logo error */      
  case 0x35:
     
#ifdef DEBUG
    fprintf(stdout, _("Message: Error getting operator logo!\n"));
#endif  
    CurrentGetBitmapError=GE_UNKNOWN; 
    break;

  default:

#ifdef DEBUG
    fprintf(stdout, _("Message: Unknown message of type 0x05\n"));
#endif /* DEBUG */
    AppendLogText("Unknown msg\n",false);

    break;
  }
}

void N6110_Dispatch0x06Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int tmp;
  unsigned char output[160];

#ifdef DEBUG
  int i;
#endif
    
  switch (MessageBuffer[3]) {

  case 0x05:

    /* MessageBuffer[3] = 0x05
       MessageBuffer[4] = 0x00
       MessageBuffer[5] = 0x0f
       MessageBuffer[6] = 0x03
       MessageBuffer[7] = length of packed message

       This is all I have seen - Gerry Anderson */

    tmp=GSM_UnpackEightBitsToSeven(0, 82, 82, MessageBuffer+8, output);

#ifdef DEBUG

    fprintf(stdout, _("Message from Network operator: "));

    for (i=0; i<tmp; i++)
       fprintf(stdout, "%c", DecodeWithDefaultAlphabet(output[i]));

    fprintf(stdout, "\n");

#endif /* DEBUG */

    break;

  default:

#ifdef DEBUG
    fprintf(stdout, _("Message: Unknown message of type 0x06\n"));
#endif /* DEBUG */
    AppendLogText("Unknown msg\n",false);

    break;
  }
}

void N6110_Dispatch0x09Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
    
  switch (MessageBuffer[3]) {
    
  case 0x80:    
#ifdef DEBUG
    fprintf(stdout, _("Message: SIM card login\n"));
#endif
    break;

  case 0x81:    
#ifdef DEBUG
    fprintf(stdout, _("Message: SIM card logout\n"));
#endif
    break;
      
  default:
#ifdef DEBUG
    fprintf(stdout, _("Unknown message of type 0x09.\n"));      
#endif
    AppendLogText("Unknown msg\n",false);
    break;
  }
}

void N6110_Dispatch0x13Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  switch(MessageBuffer[3]) {
    
  case 0x6a:

#ifdef DEBUG
    fprintf(stdout, _("Message: Calendar Alarm active\n"));
    fprintf(stdout, _("   Item number: %d\n"), MessageBuffer[4]);
#endif /* DEBUG */

  default:
#ifdef DEBUG
    fprintf(stdout, _("Unknown message of type 0x13.\n"));      
#endif
    AppendLogText("Unknown msg\n",false);
    break;
  }
}

void N6110_Dispatch0x40Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int i;
  
  switch(MessageBuffer[2]) {

  case 0x02:

#ifdef DEBUG
    fprintf(stdout, _("Message: ACK for simlock opening part 1\n"));
#endif /* DEBUG */
    
    CurrentMagicError=GE_NONE;
    break;
    
  case 0x7c:

#ifdef DEBUG
    fprintf(stdout, _("Message: Answer for call commands.\n"));
#endif
    
    CurrentDialVoiceError=GE_NONE;      
    break;
      
  case 0x81:

#ifdef DEBUG
    fprintf(stdout, _("Message: ACK for simlock opening part 2\n"));
#endif /* DEBUG */
    
    CurrentMagicError=GE_NONE;
    break;

  case 0x82:

#ifdef DEBUG
      fprintf(stdout, _("Message: ACK for simlock closing\n"));
#endif /* DEBUG */
    
    CurrentMagicError=GE_NONE;
    break;

  case 0xd4:

    switch (MessageBuffer[5]) {
      case 0xa0:
#ifdef DEBUG
        fprintf(stdout,_("Message: EEPROM contest received\n"));
#endif

        if (MessageBuffer[8]!=0x00) {
          for (i=9;i<MessageLength;i++) {
            fprintf(stdout,_("%c"), MessageBuffer[i]);
        }

        CurrentMagicError=GE_NONE;
      }
      
      break;
    }
      
#ifdef DEBUG
    fprintf(stdout, _("Unknown message of type 0x40.\n"));
#endif /* DEBUG */
    AppendLogText("Unknown msg\n",false);      
    break;

  case 0xcf:

    N6110_DisplayTestsInfo(MessageBuffer);
    break;
      
  default:

#ifdef DEBUG
    fprintf(stdout, _("Unknown message of type 0x40.\n"));
#endif /* DEBUG */
    AppendLogText("Unknown msg\n",false);
    break;	/* Visual C Don't like empty cases */
  }
}

void N6110_Dispatch0x47Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int count;
  
  switch(MessageBuffer[3]) {
    
  case 0x02:

    count=5;
    
    if (MessageBuffer[5]!=0) {
      strcpy(CurrentGetBitmap->Sender,GSM_UnpackSemiOctetNumber(MessageBuffer+5,true));

      while (MessageBuffer[count]!=0) {
        count++;
      }

      count++;
    } else {
      strcpy(CurrentGetBitmap->Sender,"\0");

      count+=3;
    }

    memcpy(CurrentGetBitmap->text,MessageBuffer+count+1,MessageBuffer[count]);
    CurrentGetBitmap->text[MessageBuffer[count]]=0;

    if (MessageBuffer[count]!=0)
      count+=MessageBuffer[count];

    count++;

#ifdef DEBUG
    fprintf(stdout,_("Picture Image received, text \"%s\", sender %s\n"),CurrentGetBitmap->text,CurrentGetBitmap->Sender);
#endif

    CurrentGetBitmap->width=MessageBuffer[count+1];
    CurrentGetBitmap->height=MessageBuffer[count+2]; 
    CurrentGetBitmap->size=GSM_GetBitmapSize(CurrentGetBitmap);
      
    memcpy(CurrentGetBitmap->bitmap,MessageBuffer+count+4,CurrentGetBitmap->size);
      
    CurrentGetBitmapError=GE_NONE;
    break;

  case 0x04:

#ifdef DEBUG
    fprintf(stdout,_("Getting or setting Picture Image - OK\n"));
#endif
    CurrentSetBitmapError=GE_NONE;
    CurrentGetBitmapError=GE_NONE;
    break;	

  case 0x05:

#ifdef DEBUG
    fprintf(stdout,_("Setting Picture Image - invalid location or other error\n"));
#endif
    CurrentSetBitmapError=GE_UNKNOWN;
    break;	

  case 0x06:

#ifdef DEBUG
    fprintf(stdout,_("Getting Picture Image - invalid location or other error\n"));
#endif
    CurrentGetBitmapError=GE_UNKNOWN;
    break;	

  default:

#ifdef DEBUG
    fprintf(stdout, _("Unknown message of type 0x47.\n"));
#endif /* DEBUG */
    AppendLogText("Unknown msg\n",false);
    break;	/* Visual C Don't like empty cases */
  }
}

void N6110_DispatchACKMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
  char buffer[50];
  
  sprintf(buffer,"Received ACK %02x %02x\n",MessageBuffer[0],MessageBuffer[1]);
  AppendLog(buffer,strlen(buffer),false);

#ifdef DEBUG
  fprintf(stdout, _("[Received Ack of type %02x, seq: %2x]\n"), MessageBuffer[0],
                                                                MessageBuffer[1]);
#endif /* DEBUG */
  
  CurrentLinkOK = true;
}

void N6110_Dispatch0xD0Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
   
#ifdef DEBUG
  fprintf(stdout, _("Message: The phone is powered on - seq 1.\n"));
#endif /* DEBUG */

}

/* This function is used for parsing the RLP frame into fields. */
void N6110_RX_HandleRLPMessage(u8 *MessageBuffer)
{

  RLP_F96Frame frame;
  int count;
  int valid = true;

  /* We do not need RLP frame parsing to be done when we do not have callback
     specified. */
  if (CurrentRLP_RXCallback == NULL)
    exit;
    
  /* Anybody know the official meaning of the first two bytes?
     Nokia 6150 sends junk frames starting D9 01, and real frames starting
     D9 00. We'd drop the junk frames anyway because the FCS is bad, but
     it's tidier to do it here. We still need to call the callback function
     to give it a chance to handle timeouts and/or transmit a frame */
  if (MessageBuffer[0] == 0xd9 && MessageBuffer[1] == 0x01)
    valid = false;

  /* Nokia uses 240 bit frame size of RLP frames as per GSM 04.22
     specification, so Header consists of 16 bits (2 bytes). See section 4.1
     of the specification. */
    
  frame.Header[0] = MessageBuffer[2];
  frame.Header[1] = MessageBuffer[3];

  /* Next 200 bits (25 bytes) contain the Information. We store the
     information in the Data array. */

  for (count = 0; count < 25; count ++)
    frame.Data[count] = MessageBuffer[4 + count];

  /* The last 24 bits (3 bytes) contain FCS. */

  frame.FCS[0] = MessageBuffer[29];
  frame.FCS[1] = MessageBuffer[30];
  frame.FCS[2] = MessageBuffer[31];

  /* Here we pass the frame down in the input stream. */
  CurrentRLP_RXCallback(valid ? &frame : NULL);
}

void N6110_Dispatch0xF4Message(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: The phone is powered on - seq 2.\n"));
#endif /* DEBUG */

}

void N6110_ReplyIncomingSMS(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  GSM_SMSMessage NullSMS;

  switch (MessageBuffer[6]) {

    case 0x00: NullSMS.Type = GST_SMS; NullSMS.folder = GST_INBOX; break;
    case 0x01: NullSMS.Type = GST_DR;  NullSMS.folder = GST_INBOX; break;

    /* Is it possible ? */
    case 0x02: NullSMS.Type = GST_SMS; NullSMS.folder = GST_OUTBOX; break;      
    default:   NullSMS.Type = GST_UN;                               break;
  }

#ifdef DEBUG
  if (NullSMS.Type == GST_DR)
    fprintf(stdout, _("Message: SMS Message (Report) Received\n"));
  else 
    fprintf(stdout, _("Message: SMS Message Received\n"));  
#endif /* DEBUG */

  GSM_DecodeNokiaSMSFrame(&NullSMS, MessageBuffer+7, MessageLength-7);

#ifdef DEBUG
  fprintf(stdout, _("\n"));      
#endif /* DEBUG */
}

void N6110_DispatchMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  bool unknown=false;

  /* Switch on the basis of the message type byte */
  switch (MessageType) {
	  
  /* Call information */
  case 0x01:

    N6110_Dispatch0x01Message(MessageLength, MessageBuffer, MessageType);
    break;

  /* SMS handling */
  case 0x02:
    switch (MessageBuffer[3]) {
      case 0x02:
      case 0x03:N6110_ReplySendSMSMessage(MessageLength,MessageBuffer,MessageType);break;
      case 0x10:N6110_ReplyIncomingSMS(MessageLength,MessageBuffer,MessageType);break;
      case 0x21:N6110_ReplySetCellBroadcast(MessageLength, MessageBuffer, MessageType);break;
      case 0x23:N6110_ReplyReadCellBroadcast(MessageLength, MessageBuffer, MessageType);break;
      case 0x31:N6110_ReplySetSMSCenter(MessageLength,MessageBuffer,MessageType);break;
      case 0x34:
      case 0x35:N6110_ReplyGetSMSCenter(MessageLength,MessageBuffer,MessageType);break;
      default  :unknown=true;break;
    }
    break;

  /* Phonebook handling */
  case 0x03:
    switch (MessageBuffer[3]) {
      case 0x02:
      case 0x03:N6110_ReplyGetMemoryLocation(MessageLength,MessageBuffer,MessageType);break;
      case 0x05:
      case 0x06:N6110_ReplyWritePhonebookLocation(MessageLength,MessageBuffer,MessageType);break;
      case 0x08:
      case 0x09:N6110_ReplyGetMemoryStatus(MessageLength,MessageBuffer,MessageType);break;
      case 0x17:
      case 0x18:N6110_ReplyGetSpeedDial(MessageLength,MessageBuffer,MessageType);break;
      case 0x1a:
      case 0x1b:N6110_ReplySetSpeedDial(MessageLength,MessageBuffer,MessageType);break;
      default  :N6110_Dispatch0x03Message(MessageLength,MessageBuffer,MessageType);break;
    }
    break;

  /* Phone status */	 
  case 0x04:
    switch (MessageBuffer[3]) {
      case 0x02:N6110_ReplyRFBatteryLevel(MessageLength,MessageBuffer,MessageType);break;
      default  :unknown=true;break;
    }
    break;
      
  /* Startup Logo, Operator Logo and Profiles. */
  case 0x05:
    switch (MessageBuffer[3]) {
      case 0x11:N6110_ReplySetProfile    (MessageLength,MessageBuffer,MessageType);break;
      case 0x14:N6110_ReplyGetProfile    (MessageLength,MessageBuffer,MessageType);break;
      case 0x1b:N6110_ReplyGetProfile    (MessageLength,MessageBuffer,MessageType);break;
      case 0x1d:N6110_ReplySetProfile    (MessageLength,MessageBuffer,MessageType);break;
      case 0x37:N6110_ReplySetRingtone   (MessageLength,MessageBuffer,MessageType);break;
      case 0x38:N6110_ReplySetRingtone   (MessageLength,MessageBuffer,MessageType);break;
      default  :N6110_Dispatch0x05Message(MessageLength,MessageBuffer,MessageType);break;
    }
    break;

  /* Network Operator Message to handset -> Gerry Anderson & prepaid info */
  /* Call diverts */
  case 0x06:
    switch (MessageBuffer[3]) {
      case 0x02:
      case 0x03:N6110_ReplyCallDivert    (MessageLength,MessageBuffer,MessageType);break;
      default  :N6110_Dispatch0x06Message(MessageLength,MessageBuffer,MessageType);break;
    }
    break;

  /* Security code requests */
  case 0x08:
    switch (MessageBuffer[3]) {
      case 0x08:N6110_ReplyGetSecurityCodeStatus(MessageLength,MessageBuffer,MessageType);break;
      case 0x0b:N6110_ReplyEnterSecurityCode    (MessageLength,MessageBuffer,MessageType);break;
      default  :N6110_ReplyEnterSecurityCode    (MessageLength,MessageBuffer,MessageType);break;
    }
    break;

  /* SIM login */
  case 0x09:

    N6110_Dispatch0x09Message(MessageLength, MessageBuffer, MessageType);
    break;

  /* Network info */
  case 0x0a:
    switch (MessageBuffer[3]) {
      case 0x71:N6110_ReplyGetNetworkInfo(MessageLength,MessageBuffer,MessageType);break;
      default  :unknown=true;break;
    }
    break;

  /* Simulating key pressing */
  case 0x0c:
    switch (MessageBuffer[3]) {
      case 0x43:N6110_ReplyPressKey(MessageLength,MessageBuffer,MessageType);break;
      default  :unknown=true;break;
    }
    break;

  /* Display */
  case 0x0d:
    switch (MessageBuffer[3]) {
      case 0x50:N6110_ReplyDisplayOutput   (MessageLength,MessageBuffer,MessageType);break;
      case 0x52:N6110_ReplyGetDisplayStatus(MessageLength,MessageBuffer,MessageType);break;
      case 0x54:N6110_ReplyDisplayOutput   (MessageLength,MessageBuffer,MessageType);break;
      default  :unknown=true;break;
    }
    break;

  /* Phone Clock and Alarm */
  case 0x11:
    switch (MessageBuffer[3]) {
      case 0x61:N6110_ReplySetDateTime(MessageLength,MessageBuffer,MessageType);break;
      case 0x63:N6110_ReplyGetDateTime(MessageLength,MessageBuffer,MessageType);break;
      case 0x6c:N6110_ReplySetAlarm   (MessageLength,MessageBuffer,MessageType);break;
      case 0x6e:N6110_ReplyGetAlarm   (MessageLength,MessageBuffer,MessageType);break;
      default  :unknown=true;break;
    }
    break;

  /* Calendar notes handling */
  case 0x13:
    switch (MessageBuffer[3]) {
      case 0x65:N6110_ReplyWriteCalendarNote (MessageLength,MessageBuffer,MessageType);break;
      case 0x67:N6110_ReplyGetCalendarNote   (MessageLength,MessageBuffer,MessageType);break;
      case 0x69:N6110_ReplyDeleteCalendarNote(MessageLength,MessageBuffer,MessageType);break;
      default  :N6110_Dispatch0x13Message    (MessageLength,MessageBuffer,MessageType);break;
    }
    break;

  /* SMS Messages */
  case 0x14:
    switch (MessageBuffer[3]) {
      case 0x05:
      case 0x06:N6110_ReplySaveSMSMessage  (MessageLength,MessageBuffer,MessageType);break;
      case 0x08:
      case 0x09:N6110_ReplyGetSMSMessage   (MessageLength,MessageBuffer,MessageType);break;
      case 0x0b:N6110_ReplyDeleteSMSMessage(MessageLength,MessageBuffer,MessageType);break;
      case 0x37:
      case 0x38:N6110_ReplyGetSMSStatus    (MessageLength,MessageBuffer,MessageType);break;
      default  :unknown=true;break;
    }
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

  /* Internal phone functions? */
  case 0x40:
    switch (MessageBuffer[2]) {
      case 0x64:N6110_ReplyEnableExtendedCommands  (MessageLength,MessageBuffer,MessageType);break;
      case 0x65:N6110_ReplyResetPhoneSettings      (MessageLength,MessageBuffer,MessageType);break;
      case 0x66:N6110_ReplyIMEI                    (MessageLength,MessageBuffer,MessageType);break;
      case 0x6a:N6110_ReplyGetProductProfileSetting(MessageLength,MessageBuffer,MessageType);break;
      case 0x6b:N6110_ReplySetProductProfileSetting(MessageLength,MessageBuffer,MessageType);break;
      case 0x6e:N6110_ReplyGetSecurityCode         (MessageLength,MessageBuffer,MessageType);break;
      case 0x7e:N6110_ReplyNetmonitor              (MessageLength,MessageBuffer,MessageType);break;
      case 0x8a:N6110_ReplySimlockInfo             (MessageLength,MessageBuffer,MessageType);break;
      case 0x8b:N6110_ReplySetOperatorName         (MessageLength,MessageBuffer,MessageType);break;
      case 0x8c:N6110_ReplyGetOperatorName         (MessageLength,MessageBuffer,MessageType);break;
      case 0x8f:N6110_ReplyPlayTone                (MessageLength,MessageBuffer,MessageType);break;
      case 0x9e:N6110_ReplyGetBinRingtone          (MessageLength,MessageBuffer,MessageType);break;
      case 0xa0:N6110_ReplySetBinRingtone          (MessageLength,MessageBuffer,MessageType);break;
      case 0xc8:N6110_ReplyHW                      (MessageLength,MessageBuffer,MessageType);break;
      default  :N6110_Dispatch0x40Message          (MessageLength,MessageBuffer,MessageType);break;
    }
    break;

  /* Picture Images */
  case 0x47:

    N6110_Dispatch0x47Message(MessageLength, MessageBuffer, MessageType);
    break;

  /* Mobile phone identification */
  case 0x64:

    N6110_ReplyGetAuthentication(MessageLength, MessageBuffer, MessageType);
    break;

  /***** Acknowlegment of our frames. *****/
  case FBUS_FRTYPE_ACK:

    N6110_DispatchACKMessage(MessageLength, MessageBuffer, MessageType);
    break;

  /***** Power on message. *****/
  case 0xd0:

    N6110_Dispatch0xD0Message(MessageLength, MessageBuffer, MessageType);
    break;

  case 0xd2:

    N6110_ReplyID(MessageLength, MessageBuffer, MessageType);
    break;
  
  /***** RLP frame received. *****/
  case 0xf1:

    N6110_RX_HandleRLPMessage(MessageBuffer);
    break;

  /***** Power on message. *****/
  case 0xf4:

    N6110_Dispatch0xF4Message(MessageLength, MessageBuffer, MessageType);
    break;

  /***** Unknown message *****/
  /* If you think that you know the exact meaning of other messages - please
     let us know. */
  default:

#ifdef DEBUG
    fprintf(stdout, _("Message: Unknown message type.\n"));
#endif /* DEBUG */
    AppendLogText("Unknown msg type\n",false);

    unknown=false;
    break;

  }

  if (unknown) {
#ifdef DEBUG
    fprintf(stdout, _("Unknown message of type %02x.\n"),MessageType);
#endif
    AppendLogText("Unknown msg\n",false);
  }
}
