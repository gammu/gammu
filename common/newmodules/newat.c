/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides an API for accessing functions on the at and similar
  phones.

*/

/* "Turn on" prototypes in n-at.h */

#define __n_at_c 

/* System header files */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
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
#include "newmodules/newat.h"
#include "newmodules/n6110.h"
/* Global variables used by code in gsm-api.c to expose the functions
   supported by this model of phone. */






/* Here we initialise model specific functions. */

GSM_Functions Nat_Functions = {
  Nat_Initialise,
  Nat_DispatchMessage,
  NULL_Terminate,
  NULL_KeepAlive,
  Nat_GetMemoryLocation,
  Nat_WritePhonebookLocation,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  Nat_GetMemoryStatus,
  Nat_GetSMSStatus,
  Nat_GetSMSCenter,
  Nat_SetSMSCenter,
  Nat_GetSMSMessage,
  Nat_DeleteSMSMessage,
  Nat_SendSMSMessage,
  Nat_SaveSMSMessage,
  Nat_GetRFLevel,
  Nat_GetBatteryLevel,
  Nat_GetPowerSource,
  Nat_GetDisplayStatus, //fill it
  Nat_EnterSecurityCode,
  Nat_GetSecurityCodeStatus,
  UNIMPLEMENTED,
  N6110_GetIMEI,
  N6110_GetRevision,
  N6110_GetModel,
  Nat_GetDateTime,
  Nat_SetDateTime,
  Nat_GetAlarm,
  Nat_SetAlarm, //doesn't work correctly...
  Nat_DialVoice,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  Nat_GetNetworkInfo,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  Nat_SendDTMF,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  Nat_CancelCall,
  Nat_PressKey,  //fill it
  UNIMPLEMENTED, //DisplayOutput= AT+CDIS
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  N6110_GetSMSFolders,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  UNIMPLEMENTED,
  Nat_CallDivert,
  UNIMPLEMENTED,
  Nat_GetManufacturer
};

/* Mobile phone information */

GSM_Information Nat_Information = {
  "",   /* Supported models in FBUS */
  "at", /* Supported models in MBUS */
  "",   /* Supported models in FBUS over infrared */
  "",   /* Supported models in FBUS over DLR3 */
  "at", /* Supported models in AT commands mode */
  "",
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

/* Type of SMS mode. 0=PDU mode; 1=text mode */
int SMSATCMGF=-1; //-1 when not used earlier
#define SMS_AT_PDU  0
#define SMS_AT_TEXT 1

/* Type of charset for phonebook 0=GSM; 1=HEX */
int PBKATCSCS=-1; //-1 when not used earlier
#define PBK_AT_GSM 0
#define PBK_AT_HEX 1

/* Manufacturer of connected AT compatible phone */
int ATMan=-1; // -1 when not used earlier
#define AT_MAN_NOKIA   0
#define AT_MAN_SIEMENS 1

int ExtractOneParameter(unsigned char *input, unsigned char *output)
{
  int position=0;

  while (*input!=',' && *input!=0x0d) {
    *output=*input;
    input++;
    output++;
    position++;
  }
  *output=0;
  position++;
  return position;
}

void ATDecodeDateTime(GSM_DateTime *dt, unsigned char *input)
{
  dt->Year=2000+(*input-'0')*10;     input++;
  dt->Year=dt->Year+(*input-'0');    input++;

  input++;
  dt->Month=(*input-'0')*10;         input++;
  dt->Month=dt->Month+(*input-'0');  input++;

  input++;
  dt->Day=(*input-'0')*10;           input++;
  dt->Day=dt->Day+(*input-'0');      input++;

  input++;
  dt->Hour=(*input-'0')*10;          input++;
  dt->Hour=dt->Hour+(*input-'0');    input++;

  input++;
  dt->Minute=(*input-'0')*10;        input++;
  dt->Minute=dt->Minute+(*input-'0');input++;

  input++;
  dt->Second=(*input-'0')*10;        input++;
  dt->Second=dt->Second+(*input-'0');input++;

  if (input!=NULL) {
    input++;
    dt->Timezone=(*input-'0')*10;          input++;
    dt->Timezone=dt->Timezone+(*input-'0');input++;
    input=input-2;
    if (*input=='-') dt->Timezone=-dt->Timezone;
  }
}

void Nat_ReplyGetIMEI(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  strncpy(Current_IMEI,MessageBuffer+10,15);

#ifdef DEBUG
  fprintf(stdout, _("Message: IMEI %s received\n"),Current_IMEI);
#endif

  CurrentGetIMEIError=GE_NONE;
}

GSM_Error Nat_SendIMEIFrame()
{
  unsigned char req[8] = {"AT+CGSN\r"};  

  return NULL_SendMessageSequence
    (50, &CurrentGetIMEIError, 8, 0x00, req);
}

void Nat_ReplyGetID(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int i;

  i=10;
  if (strncmp("Nokia Communicator GSM900/1800",MessageBuffer+i,30)==0) {
#ifdef DEBUG
    fprintf(stdout, _("Message: Mobile phone model identification received:\n"));
    fprintf(stdout, _("   Model: RAE-3\n"));
#endif
    strcpy(Current_Model,"RAE-3");
    CurrentMagicError=GE_NONE;
    return;
  }
  if (strncmp("Nokia 6210",MessageBuffer+i,10)==0) {
#ifdef DEBUG
    fprintf(stdout, _("Message: Mobile phone model identification received:\n"));
    fprintf(stdout, _("   Model: NPE-3\n"));
#endif
    strcpy(Current_Model,"NPE-3");
    CurrentMagicError=GE_NONE;
    return;
  }

  while (MessageBuffer[i]!=0x0d && MessageBuffer[i]!=0x0a) {
    Current_Model[i-10]=MessageBuffer[i];
    i++;
  }
  Current_Model[i-9]=0;

#ifdef DEBUG
  fprintf(stdout, _("Message: Mobile phone model identification received:\n"));
  fprintf(stdout, _("   Model: %s\n"),Current_Model);
#endif

  CurrentMagicError=GE_NONE;
}

GSM_Error Nat_SendIDFrame()
{
  unsigned char req[8] = {"AT+CGMM\r"};  

  return NULL_SendMessageSequence
    (50, &CurrentMagicError, 8, 0x00, req);
}

void Nat_ReplyGetHW(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  int i,current;

#ifdef DEBUG
  fprintf(stdout, _("Message: Mobile phone model identification received:\n"));
#endif

  switch (ATMan) {
    case AT_MAN_NOKIA:
#ifdef DEBUG
      fprintf(stdout, _("   Firmware: "));
      for (i=12;i<17;i++) fprintf(stdout,_("%c"),MessageBuffer[i]);
      fprintf(stdout, _("\n"));
      /* Some Nokia phones like 9210 return only firmware */
      if (MessageLength>24) {
        fprintf(stdout, _("   Hardware: "));
        for (i=21;i<25;i++) fprintf(stdout,_("%c"),MessageBuffer[i]);
        fprintf(stdout, _("\n"));
      }
#endif

      strcpy(Current_Revision,"SW");current=2;
      for (i=12;i<17;i++) Current_Revision[current++]=MessageBuffer[i];
      Current_Revision[current++]=',';
      Current_Revision[current++]=' ';
      Current_Revision[current++]='H';
      Current_Revision[current++]='W';
      if (MessageLength>24) {
        for (i=21;i<25;i++) Current_Revision[current++]=MessageBuffer[i];
      } else {
        /* Some Nokia phones like 9210 return only firmware */
        /* Here we copy firmware as hardware */
        for (i=12;i<17;i++) Current_Revision[current++]=MessageBuffer[i];
      }
      break;
    case AT_MAN_SIEMENS:
      current=0;i=18;
      strcpy(Current_Revision,"SW");current=2;
      while (MessageBuffer[i]!=0x0d && MessageBuffer[i]!=0x0a) {
        Current_Revision[current++]=MessageBuffer[i];
        i++;
      }
      Current_Revision[current++]=',';
      Current_Revision[current++]=' ';
      Current_Revision[current++]='H';
      Current_Revision[current++]='W';
      i=18;
      while (MessageBuffer[i]!=0x0d && MessageBuffer[i]!=0x0a) {
        Current_Revision[current++]=MessageBuffer[i];
        i++;
      }

      break;
    default:
      strcpy(Current_Revision,"SWxx.xx, HWxxxx");
      break;
  }

  CurrentGetHWError=GE_NONE;
}

GSM_Error Nat_SendHWFrame()
{
  unsigned char req[8] = {"AT+CGMR\r"};  

  return NULL_SendMessageSequence
    (50, &CurrentGetHWError, 8, 0x00, req);
}

void Nat_ReplyGetManufacturer(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: phone manufacturer received\n"));
#endif

  if (strstr(MessageBuffer,"Nokia")) {
#ifdef DEBUG
    fprintf(stdout, _("   Nokia\n"));
#endif
    ATMan=AT_MAN_NOKIA;
  }

  if (strstr(MessageBuffer,"SIEMENS")) {
#ifdef DEBUG
    fprintf(stdout, _("   Siemens\n"));
#endif
    ATMan=AT_MAN_SIEMENS;
  }

  CurrentMagicError=GE_NONE;
}

GSM_Error Nat_SendManufacturerFrame()
{
  unsigned char req[8] = {"AT+CGMI\r"};

  return NULL_SendMessageSequence
    (50, &CurrentMagicError, 8, 0x00, req);
}

/* Initialise variables and state machine. */
GSM_Error Nat_Initialise(char *port_device, char *initlength,
                          GSM_ConnectionType connection,
                          void (*rlp_callback)(RLP_F96Frame *frame))
{
  if (Protocol->Initialise(port_device,initlength,connection,rlp_callback)!=GE_NONE)
  {
    return GE_NOTSUPPORTED;
  }


  /* We try to escape AT+CMGS mode, at least Siemens M20 then needs to get some rest
	 */
//  WRITEPHONE(PortFD,"\x1B\r",2);
//  usleep(500000);

  usleep(100);

  if (Nat_SendManufacturerFrame()!=GE_NONE) return GE_TIMEOUT;

  if (Nat_SendIMEIFrame()!=GE_NONE) return GE_TIMEOUT;

  if (Nat_SendIDFrame()!=GE_NONE) return GE_TIMEOUT;

  if (Nat_SendHWFrame()!=GE_NONE) return GE_TIMEOUT;

  /* In AT doesn't have any init strings, etc. Phone answered with frame,
     so connection should be enabled ;-) */
  CurrentLinkOK = true;                           

  return (GE_NONE);
}

GSM_Error Nat_GetManufacturer(char* manufacturer)
{
  switch (ATMan) {
    case AT_MAN_NOKIA  :strcpy (manufacturer, "Nokia");  break;
    case AT_MAN_SIEMENS:strcpy (manufacturer, "Siemens");break;
    default            :strcpy (manufacturer, "unknown");break;
  }

  return GE_NONE;
}

void Nat_ReplyPBKSetMemoryTypeError(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: memory setting error\n"));
#endif

  CurrentPhonebookError=GE_UNKNOWN;
}

void Nat_ReplyPBKSetMemoryType(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: memory type set OK\n"));
#endif

  CurrentPhonebookError=GE_NONE;
}

static GSM_Error PBKSetMemoryType(GSM_MemoryType type)
{
  char req[] = "AT+CPBS=\"XX\"\r";
  char req2[10];

  GetMemoryTypeString(req2, &type);

  req[9]=req2[0];
  req[10]=req2[1];

  return NULL_SendMessageSequence
    (50, &CurrentPhonebookError, strlen(req), 0x00, req);
}

void Nat_ReplySetCharset(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  if (strncmp("AT+CSCS=\"HEX\"",MessageBuffer,13)==0) {
#ifdef DEBUG
    fprintf(stdout, _("Message: charset set to HEX\n"));
#endif
    CurrentPhonebookError=GE_NONE;
  }
  if (strncmp("AT+CSCS=\"GSM\"",MessageBuffer,13)==0) {
#ifdef DEBUG
    fprintf(stdout, _("Message: charset set to GSM\n"));
#endif
    CurrentPhonebookError=GE_NONE;
  }
}

GSM_Error SetCharset()
{
  unsigned char req [14] = {"AT+CSCS=\"HEX\"\r"};  // HEX charset
  unsigned char req2[14] = {"AT+CSCS=\"GSM\"\r"};  // GSM charset
  GSM_Error error;

  error=NULL_SendMessageSequence
    (50, &CurrentPhonebookError, 14, 0x00, req);
  if (error==GE_NONE) {
    PBKATCSCS=PBK_AT_HEX;
    return GE_NONE;
  }

  error=NULL_SendMessageSequence
    (50, &CurrentPhonebookError, 14, 0x00, req2);
  if (error==GE_NONE) {
    PBKATCSCS=PBK_AT_GSM;
    return GE_NONE;
  }

  return error;
}

void Nat_ReplyGetMemoryLocationError(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: phonebook entry receiving error\n"));
#endif

  CurrentPhonebookError=GE_UNKNOWN;
}

/* FIXME: M20 can have " inside name. We can't get endpos by finding first " */
void Nat_ReplyGetMemoryLocation(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  char *pos, *endpos;
  int l;

#ifdef DEBUG
  fprintf(stdout, _("Message: phonebook entry received\n"));
#endif

  CurrentPhonebookEntry->Empty = true;
  *(CurrentPhonebookEntry->Name) = '\0';
  *(CurrentPhonebookEntry->Number) = '\0';
  CurrentPhonebookEntry->SubEntriesCount=0;
  CurrentPhonebookEntry->Group = 0;

  pos = strchr(MessageBuffer, '\"');
  endpos = NULL;
  if (pos) {	
    endpos = strchr(++pos, '\"');
    if (endpos) {
      *endpos = '\0';
      strcpy(CurrentPhonebookEntry->Number, pos);
    }
    pos = NULL;
    if (endpos) pos = strchr(++endpos, '\"');
    endpos = NULL;
    if (pos) {
      pos++;
      l = pos - (char *)MessageBuffer;
      endpos = memchr(pos, '\"', MessageLength - l);
    }
    if (endpos) {
      l = endpos - pos;
      switch (PBKATCSCS) {
	case PBK_AT_GSM:
	  DecodeDefault(CurrentPhonebookEntry->Name, pos, l);
	  break;
	case PBK_AT_HEX:
	  DecodeHexBin(CurrentPhonebookEntry->Name, pos, l);
	  break;
      }
    }
  }

  CurrentPhonebookError=GE_NONE;
}

GSM_Error Nat_GetMemoryLocation(GSM_PhonebookEntry *entry)
{
  GSM_Error error;
  char req[] = "AT+CPBR=00000\r";

  if (PBKATCSCS==-1) {
    error=SetCharset();
    if (error!=GE_NONE) return error;
  }

  error = PBKSetMemoryType(entry->MemoryType);
  if (error != GE_NONE) return error;

  CurrentPhonebookEntry = entry;

  sprintf(req + 8, "%5d\r", entry->Location);

  return NULL_SendMessageSequence
    (50, &CurrentPhonebookError, strlen(req), 0x00, req);
}

void Nat_ReplyWritePhonebookLocation(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: phonebook entry set OK\n"));
#endif

  CurrentPhonebookError=GE_NONE;
}

GSM_Error Nat_WritePhonebookLocation(GSM_PhonebookEntry * entry)
{
  GSM_Error error;
  char req[128];
  char Name[128];
  char Number[100];
  u8 i;

  switch (entry->MemoryType) {
    case GMT_ME:
      /* FIXME: the 7110, 6210... supports long phonebookentries. in lack
         of documentation we only support SIM memory */
      if(GetModelFeature (FN_PHONEBOOK)==F_PBK71INT) return GE_NOTIMPLEMENTED;

      break;
    default:
      break;
  }

  if (PBKATCSCS==-1) {
    error=SetCharset();
    if (error!=GE_NONE) return error;
  }

  error = PBKSetMemoryType(entry->MemoryType);
  if (error != GE_NONE) return error;

  switch (PBKATCSCS) {
    case PBK_AT_HEX:
      EncodeHexBin (Name, entry->Name, strlen(entry->Name));
      Name[strlen(entry->Name)*2]=0;
      break;
    case PBK_AT_GSM:
      EncodeDefault (Name, entry->Name, strlen(entry->Name));
      Name[strlen(entry->Name)]=0;
      break;
  }

  GSM_PackSemiOctetNumber(entry->Number,Number,false);
  i=Number[0];

  sprintf(req, "AT+CPBW=%d, \"%s\", %i, \"%s\"\r",
	        entry->Location, entry->Number, i, Name);

  return NULL_SendMessageSequence
    (50, &CurrentPhonebookError, strlen(req), 0x00, req);
}

void Nat_ReplyGetMemoryStatusCPBS(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  char *start;

#ifdef DEBUG
  fprintf(stdout, _("Message: memory status received\n"));
#endif

  CurrentMemoryStatus->Used = 0;
  CurrentMemoryStatus->Free = 0;

  start = strchr(MessageBuffer, ',');

  if (start) {
    start++;
    CurrentMemoryStatus->Used = atoi(start);
    start = strchr(start, ',');
    if (start) {
      start++;
      CurrentMemoryStatus->Free = atoi(start) - CurrentMemoryStatus->Used;
      CurrentMemoryStatusError=GE_NONE;
    } else CurrentMemoryStatusError=GE_UNKNOWN;
  } else CurrentMemoryStatusError=GE_UNKNOWN;
}

void Nat_ReplyGetMemoryStatusCPBSError(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
#ifdef DEBUG
  fprintf(stdout, _("Message: memory status receiving error\n"));
#endif
  CurrentMemoryStatusError=GE_UNKNOWN;
}

void Nat_ReplyGetMemoryStatusCPBR(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  char *start;

#ifdef DEBUG
  fprintf(stdout, _("Message: memory size received\n"));
#endif

  CurrentMemoryStatus->Used = 0;
  CurrentMemoryStatus->Free = 0;
 
  start = strchr(MessageBuffer, '-');

  if (start) {
    start++;
    /* Parse +CPBR: (first-last),max_number_len,max_name_len */ 
    /* We cannot get Used/Free info. We can get only size of memory
       we don't have size in memory status and because of it
       we make assigment used=size_of_memory, free=0.
       It's better than nothing */
    CurrentMemoryStatus->Used = atoi(start);
    CurrentMemoryStatusError=GE_NONE;
  } else CurrentMemoryStatusError=GE_UNKNOWN;
}

GSM_Error Nat_GetMemoryStatus(GSM_MemoryStatus *Status)
{
  GSM_Error error;
  char req [] ="AT+CPBS?\r"; /* in some phones doesn't work or doesn't return
                                memory status inside */
  char req2[] ="AT+CPBR=?\r";

  error = PBKSetMemoryType(Status->MemoryType);
  if (error != GE_NONE) return error;

  CurrentMemoryStatus = Status;

  error=NULL_SendMessageSequence
    (20, &CurrentMemoryStatusError, strlen(req), 0x00, req);
  if (error==GE_NONE) {
    CurrentMemoryStatus = NULL;
    return error;
  }

  error=NULL_SendMessageSequence
    (20, &CurrentMemoryStatusError, strlen(req2), 0x00, req2);

  CurrentMemoryStatus = NULL;

  return error;
}

void Nat_ReplyCallDivert(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
  int current;

#ifdef DEBUG
  fprintf(stdout, _("Message: call forwarding info\n"));
#endif
  if (MessageLength>0x20) {
    current=27;
    while (MessageBuffer[current]!='"') {
      CurrentCallDivert->Number[current-27]=MessageBuffer[current];
      current++;
    }
    CurrentCallDivert->Enabled=true;
    CurrentCallDivert->Number[current-27]=0x00;
    CurrentCallDivert->Timeout=999;//where is it in frame ?
  } else {
    CurrentCallDivert->Enabled=false;
  }
  CurrentCallDivertError=GE_NONE;
}

GSM_Error Nat_CallDivert(GSM_CallDivert *cd)
{
  char req[64];
  GSM_Error error;
  char Number[100];
  int i;

  sprintf(req, "AT+CCFC=");

  switch (cd->DType) {
    case GSM_CDV_AllTypes  : strcat(req, "4"); break;
    case GSM_CDV_Busy      : strcat(req, "1"); break;
    case GSM_CDV_NoAnswer  : strcat(req, "2"); break;
    case GSM_CDV_OutOfReach: strcat(req, "3"); break;
    default                : return GE_NOTIMPLEMENTED;
  }

  if (cd->Operation == GSM_CDV_Register) {
    GSM_PackSemiOctetNumber(cd->Number,Number,false);
    i=Number[0];

    sprintf(req, "%s,%d,\"%s\",%i,,,%d", req,
	cd->Operation,cd->Number,i,cd->Timeout);
  } else
    sprintf(req, "%s,%d", req, cd->Operation);

  strcat(req, "\r\n");

  CurrentCallDivert = cd;

  error=NULL_SendMessageSequence
    (100, &CurrentCallDivertError, strlen(req), 0x00, req);

  CurrentCallDivert = NULL;

  return error;
}

void Nat_ReplyGetNetworkInfo(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
  int i,current;
  GSM_NetworkInfo NullNetworkInfo;

  if (!strncmp(MessageBuffer,"AT+CREG=",8)) return;

  /* TODO: checking if phone really registered to network */

  current=23;

  /* Make sure we are expecting NetworkInfo frame */
  if (CurrentNetworkInfo && CurrentNetworkInfoError == GE_BUSY) {
#ifdef DEBUG
    fprintf(stdout, _("Message: network info received\n"));
#endif

    NullNetworkInfo.NetworkCode[0]=0; //no exist

    for (i=0;i<4;i++) NullNetworkInfo.CellID[i]=MessageBuffer[i+current];

    NullNetworkInfo.CellID[4]=0;

    current=current+7;

    for (i=0;i<4;i++) NullNetworkInfo.LAC[i]=MessageBuffer[i+current];

    NullNetworkInfo.LAC[4]=0;

#ifdef DEBUG
    fprintf(stdout, _("   CellID: %s\n"), NullNetworkInfo.CellID);
    fprintf(stdout, _("   LAC: %s\n"), NullNetworkInfo.LAC);
#endif

  }
      
  /* Make sure we are expecting NetworkInfo frame */
  if (CurrentNetworkInfo && CurrentNetworkInfoError == GE_BUSY)
     *CurrentNetworkInfo=NullNetworkInfo;

  CurrentNetworkInfoError = GE_NONE;      
}

void Nat_ReplyGetNetworkInfoCOPS(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
  /* Make sure we are expecting NetworkInfo frame */
  if (CurrentNetworkInfo && CurrentNetworkInfoError == GE_BUSY) {
#ifdef DEBUG
    fprintf(stdout, _("Message: network info received\n"));
#endif
    if (MessageBuffer[20]=='2') {
      CurrentNetworkInfo->NetworkCode[0]=MessageBuffer[23];
      CurrentNetworkInfo->NetworkCode[1]=MessageBuffer[24];
      CurrentNetworkInfo->NetworkCode[2]=MessageBuffer[25];
      CurrentNetworkInfo->NetworkCode[3]=' ';
      CurrentNetworkInfo->NetworkCode[4]=MessageBuffer[26];
      CurrentNetworkInfo->NetworkCode[5]=MessageBuffer[27];
      CurrentNetworkInfo->NetworkCode[6]=0;

      CurrentNetworkInfoError = GE_NONE;      
    }
  }
}

GSM_Error Nat_GetNetworkInfo(GSM_NetworkInfo *NetworkInfo)
{
  GSM_Error error;

  Protocol->SendMessage(10, 0x00, "AT+CREG=2\r");
  
  CurrentNetworkInfo = NetworkInfo;
  
  error=NULL_SendMessageSequence
    (20, &CurrentNetworkInfoError, 9, 0x00, "AT+CREG?\r");
  if (error!=GE_NONE) return error;

  error=NULL_SendMessageSequence
    (20, &CurrentNetworkInfoError, 9, 0x00, "AT+COPS?\r");

  CurrentNetworkInfo = NULL;

  return error;
}

void Nat_ReplyGetBatteryLevel(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: power source and battery info received\n"));
#endif

  CurrentPowerSource=atoi(MessageBuffer+15);
  CurrentBatteryLevel=atoi(MessageBuffer+17);
}

GSM_Error Nat_GetBatteryLevel(GSM_BatteryUnits *units, float *level)
{
  int timeout=10;
  int batt_level;

  CurrentBatteryLevel=-1;

  Protocol->SendMessage(7, 0x00, "AT+CBC\r");

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
      if (batt_level >= 70) { *level = 4; return (GE_NONE); }
      if (batt_level >= 50) { *level = 3; return (GE_NONE); }
      if (batt_level >= 30) { *level = 2; return (GE_NONE); }
      if (batt_level >= 10) { *level = 1; return (GE_NONE); }
      *level = 0;
      return (GE_NONE);
    }

    return (GE_INTERNALERROR);
  }
  else
    return (GE_NOLINK);
}

GSM_Error Nat_GetPowerSource(GSM_PowerSource * source)
{
  *source = CurrentPowerSource;
  return (GE_NONE);
} 

void Nat_ReplyGetRFLevel(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: RF level info received\n"));
#endif
  CurrentRFLevel=atoi(MessageBuffer+15);
}

GSM_Error Nat_GetRFLevel(GSM_RFUnits *units, float *level)
{
  int timeout=10;
  int rf_level;
  
  CurrentRFLevel=-1;
    
  Protocol->SendMessage(7, 0x00, "AT+CSQ\r");

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

  if (*units == GRF_Arbitrary) {
    if (CurrentRFLevel == 99) { *level = 0;return (GE_NONE); }
    if (CurrentRFLevel  > 23) { *level = 4;return (GE_NONE); }
    if (CurrentRFLevel  > 17) { *level = 3;return (GE_NONE); }
    if (CurrentRFLevel  > 11) { *level = 2;return (GE_NONE); }
    if (CurrentRFLevel  >  5) { *level = 1;return (GE_NONE); }
    *level = 0;
    return (GE_NONE);
  }

  /* CSQ units. */
  if (*units == GRF_CSQ) {
    if ((CurrentRFLevel <= 31) || (CurrentRFLevel >= 0))
      *level = CurrentRFLevel;
    else
      *level = 99;	/* Unknown/undefined */
    return (GE_NONE);
  }

  /* Unit type is one we don't handle so return error */
  return (GE_INTERNALERROR);
}

GSM_Error Nat_GetDisplayStatus(int *Status) {
  char req[128];

  sprintf(req, "AT+CIND?\r");

  return NULL_SendMessageSequence
    (50, &CurrentDisplayStatusError, strlen(req), 0x00, req);
}

GSM_Error Nat_PressKey(int key, int event)
{
  char req[128];

  sprintf(req, "AT+CKPD=\"1\",\"20\"\r");

  return NULL_SendMessageSequence
    (50, &CurrentPressKeyError, strlen(req), 0x00, req);
//  return GE_NOTSUPPORTED;
}

GSM_Error Nat_DialVoice(char *Number) {
  char req[39] = "ATDT";  

  if (strlen(Number) > 32)
    return (GE_INTERNALERROR);

  strcat(req, Number);
  strcat(req, ";\r");

  return NULL_SendMessageSequence
    (50, &CurrentDialVoiceError, 4+2+strlen(Number), 0x00, req);
}

void Nat_ReplyCancelCall(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: call hangup OK\n"));
#endif

  CurrentDialVoiceError=GE_NONE;
}

GSM_Error Nat_CancelCall(void)
{
  char req[] = "AT+CHUP\r";  

  return NULL_SendMessageSequence
    (50, &CurrentDialVoiceError, strlen(req), 0x00, req);
}  

GSM_Error Nat_SendDTMF(char *String)
{
  int n;
  char req[80] = "AT+VTS=";

  for (n = 0; n < 32; n++) {
     if (String[n] == '\0') break;
     if (n != 0) req[6 + 2 * n] = ',';
     req[7 + 2 * n] = String[n];
  }

  strcat(req, ";\r");

  return NULL_SendMessageSequence
    (50, &CurrentSendDTMFError,7+2+2*strlen(String) , 0x00, req);
}

GSM_Error Nat_EnterSecurityCode(GSM_SecurityCode SecurityCode)
{
  char req[128];

  if   ((SecurityCode.Type != GSCT_Pin)
     && (SecurityCode.Type != GSCT_Pin2)) return (GE_NOTIMPLEMENTED);

  sprintf(req, "AT+CPIN=\"%s\"\r", SecurityCode.Code);

  return NULL_SendMessageSequence
    (20, &CurrentSecurityCodeError, strlen(req), 0x00, req);
}

void Nat_ReplyGetSecurityCodeStatus(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  char *start;

#ifdef DEBUG
  fprintf(stdout, _("Message: security code status received\n"));
#endif

  start = strchr(MessageBuffer, ':');
  start+=2;

  *CurrentSecurityCodeStatus = 0;

  if (!strncmp(start, "READY", 5)) *CurrentSecurityCodeStatus = GSCT_None;

  if (!strncmp(start, "SIM ", 4)) {
    start += 4;
    if (!strncmp(start, "PIN2", 4)) *CurrentSecurityCodeStatus = GSCT_Pin2;
    if (!strncmp(start, "PUK2", 4)) *CurrentSecurityCodeStatus = GSCT_Puk2;
    if (!strncmp(start, "PIN",  3)) *CurrentSecurityCodeStatus = GSCT_Pin;
    if (!strncmp(start, "PUK",  3)) *CurrentSecurityCodeStatus = GSCT_Puk;
  }

  CurrentSecurityCodeError=GE_NONE;
}

GSM_Error Nat_GetSecurityCodeStatus(int *Status)
{
  CurrentSecurityCodeStatus = Status;

  return NULL_SendMessageSequence
    (20, &CurrentSecurityCodeError, 9, 0x00, "AT+CPIN?\r");
}

void Nat_ReplyGetDateTime(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
  int current;

  current=19;

#ifdef DEBUG
  fprintf(stdout,_("Message: date/time received\n"));
#endif

  if (MessageBuffer[current]==0x0d) {
    CurrentDateTime->IsSet=false;
  } else {
    CurrentDateTime->IsSet=true;

    ATDecodeDateTime(CurrentDateTime, MessageBuffer+(current));

#ifdef DEBUG
    fprintf(stdout,_("   %i/%i/%i %i:%i:%i\n"),
       CurrentDateTime->Day,CurrentDateTime->Month,CurrentDateTime->Year,
       CurrentDateTime->Hour,CurrentDateTime->Minute,CurrentDateTime->Second);
#endif
  }
  CurrentDateTimeError=GE_NONE;
}

GSM_Error Nat_GetDateTime(GSM_DateTime *date_time)
{
  unsigned char req[9] = {"AT+CCLK?\r"};  

  CurrentDateTime=date_time;

  return NULL_SendMessageSequence
    (50, &CurrentDateTimeError, 9, 0x00, req);
}

void Nat_ReplySetDateTime(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout,_("Message: date/time set OK\n"));
#endif
  CurrentSetDateTimeError=GE_NONE;
}

void Nat_ReplyDateTimeError(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
#ifdef DEBUG
  fprintf(stdout,_("Message: date & time functions error\n"));
#endif
  CurrentSetDateTimeError=GE_UNKNOWN;
  CurrentDateTimeError=GE_UNKNOWN;
}

GSM_Error Nat_SetDateTime(GSM_DateTime *date_time)
{
  char req[128];

  sprintf(req, "AT+CCLK=\"%02i/%02i/%02i,%02i:%02i:%02i+00\"\r",
     date_time->Year-2000,date_time->Month,date_time->Day,
     date_time->Hour,date_time->Minute,date_time->Second);

  return NULL_SendMessageSequence
    (20, &CurrentSetDateTimeError, strlen(req), 0x00, req);
}

void Nat_ReplyGetAlarm(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
  int current;

  current=19;
#ifdef DEBUG
  fprintf(stdout,_("Message: alarm info received\n"));
#endif
  if (MessageBuffer[current-1]==0x0d) {
    CurrentAlarm->IsSet=false;
    CurrentAlarm->Hour=0;
    CurrentAlarm->Minute=0;
  } else {
    CurrentAlarm->IsSet=true;

    ATDecodeDateTime(CurrentAlarm, MessageBuffer+(current));

#ifdef DEBUG
    fprintf(stdout,_("   %i:%i\n"),CurrentAlarm->Hour,CurrentAlarm->Minute);
#endif
  }
  CurrentAlarmError=GE_NONE;
}

GSM_Error Nat_GetAlarm(int alarm_number, GSM_DateTime *date_time)
{
  unsigned char req[9] = {"AT+CALA?\r"};  

  CurrentAlarm=date_time;

  return NULL_SendMessageSequence
    (50, &CurrentAlarmError, 9, 0x00, req);
}

/* FIXME: we should also allow to set the alarm off :-) */
/* Doesn't work ? */
GSM_Error Nat_SetAlarm(int alarm_number, GSM_DateTime *date_time)
{
  char req[128];
  int i=0;

  sprintf(req, "AT+CALA=\"%02i/%02i/%02i,%02i:%02i:%02i+00\",0,1\r",
     i,i,i,date_time->Hour,date_time->Minute,i);

  return NULL_SendMessageSequence
    (50, &CurrentSetAlarmError, strlen(req), 0x00, req);
}

void Nat_ReplyAlarmError(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
#ifdef DEBUG
  fprintf(stdout,_("Message: alarm functions error\n"));
#endif
  CurrentSetAlarmError=GE_UNKNOWN;
  CurrentAlarmError=GE_UNKNOWN;
}

void Nat_ReplyGetSMSCenter(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
  int i,start;
  unsigned char buffer[300];

  start=18;

#ifdef DEBUG
  fprintf(stdout, _("Message: SMSC data received\n"));
#endif

  /* FIXME: support for all formats */        
  start+=ExtractOneParameter(MessageBuffer+start, buffer);
  for (i=1;i<strlen(buffer)-1;i++) buffer[i-1]=buffer[i];
  buffer[i-1]=0;

  if (buffer[0]==0) CurrentMessageCenterError=GE_EMPTYSMSC;
  else {
    strcpy(CurrentMessageCenter->Number,buffer);

    /* Some default values. Is it possible to get them ? */
    CurrentMessageCenter->Name[0]=0;
    CurrentMessageCenter->DefaultRecipient[0]=0;
    CurrentMessageCenter->Format=GSMF_Text;
    CurrentMessageCenter->Validity=GSMV_Max_Time;

#ifdef DEBUG
    fprintf(stdout, _("   Number: '%s'\n"),buffer);
#endif

    CurrentMessageCenterError=GE_NONE;
  }
}

/* We only get SMSC number */
/* Is it possible to get more ? */
GSM_Error Nat_GetSMSCenter(GSM_MessageCenter *MessageCenter)
{
  unsigned char req[] = {"AT+CSCA?\r"};  

  if (MessageCenter->No!=1) return GE_NOTSUPPORTED;

  CurrentMessageCenter=MessageCenter;

  return NULL_SendMessageSequence
    (50, &CurrentMessageCenterError, strlen(req), 0x00, req);
}

void Nat_ReplySetSMSCenter(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: SMSC number set OK\n"));
#endif

  CurrentMessageCenterError=GE_NONE;
}

GSM_Error Nat_SetSMSCenter(GSM_MessageCenter *MessageCenter)
{
  unsigned char req[50];  

  if (MessageCenter->No!=1) return GE_NOTSUPPORTED;

  sprintf(req, "AT+CSCA=\"%s\"\r",MessageCenter->Number);

  return NULL_SendMessageSequence
    (50, &CurrentMessageCenterError, strlen(req), 0x00, req);
}

void Nat_ReplySMSGetPDUType(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  if (strncmp("AT+CSDH=1\r",MessageBuffer,10)==0) {
#ifdef DEBUG
    fprintf(stdout, _("Message: all SMS parameters will be displayed in text mode\n"));
#endif
    CurrentSMSMessageError=GE_NONE;
  }
  if (strncmp("AT+CMGF=0\r",MessageBuffer,10)==0) {
#ifdef DEBUG
    fprintf(stdout, _("Message: set PDU mode for SMS\n"));
#endif
    CurrentSMSMessageError=GE_NONE;
  }
  if (strncmp("AT+CMGF=1\r",MessageBuffer,10)==0) {
#ifdef DEBUG
    fprintf(stdout, _("Message: set text mode for SMS\n"));
#endif
    CurrentSMSMessageError=GE_NONE;
  }
}

GSM_Error SMSGetPDUType()
{
  unsigned char req [10] = {"AT+CMGF=0\r"};  //sets PDU mode

  unsigned char req2[10] = {"AT+CMGF=1\r"};  //sets text mode
  unsigned char req3[10] = {"AT+CSDH=1\r"};  //shows all parameters in text mode

  GSM_Error error;

  error=NULL_SendMessageSequence (50, &CurrentSMSMessageError, 10, 0x00, req);
  if (error==GE_NONE) {
    SMSATCMGF=SMS_AT_PDU;
    return GE_NONE;
  }

  error=NULL_SendMessageSequence (50, &CurrentSMSMessageError, 10, 0x00, req2);
  if (error==GE_NONE) {
    SMSATCMGF=SMS_AT_TEXT;

    error=NULL_SendMessageSequence (50, &CurrentSMSMessageError, 10, 0x00, req3);
  }

  return error;
}

GSM_Error GSM_DecodeETSISMSFrame(GSM_SMSMessage *SMS, unsigned char *req, int length)
{
  SMS_MessageType PDU=SMS_Deliver;
  GSM_ETSISMSMessage ETSI;
  int i,current=0,current2=0;
  GSM_Error error;

  for(i=0;i<req[0]+1;i++) ETSI.SMSCNumber[i]=req[current++];

  ETSI.firstbyte=req[current++];

  /* See GSM 03.40 section 9.2.3.1 */
  if ((ETSI.firstbyte & 0x03) == 0x01) PDU=SMS_Submit;
  if ((ETSI.firstbyte & 0x03) == 0x02) PDU=SMS_Status_Report;

  switch (PDU) {
    case SMS_Submit:
      SMS->Type = GST_SMS;
      SMS->folder= GST_OUTBOX;
      current++; //TPMR is ignored now
      current2=((req[current])+1)/2+1;
      for(i=0;i<current2+1;i++) ETSI.Number[i]=req[current++];
      ETSI.TPPID=req[current++];
      ETSI.TPDCS=req[current++];

      /* See GSM 03.40 9.2.3.3 TPVP can not exist in frame */
      if ((ETSI.firstbyte & 0x18)!=0) current++; //TPVP is ignored now

      ETSI.TPUDL=req[current++];
      current2=current;
      for(i=current2;i<length;i++) ETSI.MessageText[i-current2]=req[current++];
      break;
    case SMS_Deliver:

      SMS->Type = GST_SMS;
      SMS->folder= GST_INBOX;
      current2=((req[current])+1)/2+1;
      for(i=0;i<current2+1;i++) ETSI.Number[i]=req[current++];
      ETSI.TPPID=req[current++];
      ETSI.TPDCS=req[current++];
      for(i=0;i<7;i++) ETSI.DeliveryDateTime[i]=req[current++];
      ETSI.TPUDL=req[current++];
      current2=current;
      for(i=current2;i<length;i++) ETSI.MessageText[i-current2]=req[current++];
      break;
    case SMS_Status_Report: 

      SMS->Type = GST_DR;
      SMS->folder= GST_INBOX;
      current++; //TPMR is ignored now
      current2=((req[current])+1)/2+1;
      for(i=0;i<current2+1;i++) ETSI.Number[i]=req[current++];
      for(i=0;i<7;i++) ETSI.DeliveryDateTime[i]=req[current++];
      for(i=0;i<7;i++) ETSI.SMSCDateTime[i]=req[current++];
      ETSI.TPStatus=req[current];
      break;
    default:

      break;
  }

  error=GSM_DecodeETSISMS(SMS, &ETSI);
  if (error!=GE_NONE) return error;

  return GE_NONE;
}

void Nat_ReplyGetSMSMessageError(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
  int error;

  error=MessageBuffer[MessageLength-2]-'0';
  error=error+(MessageBuffer[MessageLength-3]-'0')*10;
  error=error+(MessageBuffer[MessageLength-4]-'0')*100;

#ifdef DEBUG
  fprintf(stdout, _("Message: SMS Reading failed\n"));

  switch (error) {
    case 311:fprintf(stdout, _("   No access to memory (no PIN on card ?)\n"));break;
    case 321:fprintf(stdout, _("   Empty location!\n"));break;
    default :fprintf(stdout, _("   Error code %i - please report it \n"),error);break;
  }

#endif

  switch (error) {
    case 311:CurrentSMSMessageError = GE_NOACCESS;break;
    case 321:CurrentSMSMessageError = GE_EMPTYSMSLOCATION;break;
    default :CurrentSMSMessageError = GE_UNKNOWN;break;
  }
}

void Nat_ReplyGetSMSMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
  int i,start,j,end;
  unsigned char buffer[300],buffer2[300];
  GSM_ETSISMSMessage ETSISMS;
  GSM_DateTime DT;

#ifdef DEBUG
  fprintf(stdout, _("Message: SMS Message Received\n"));
#endif
  switch (SMSATCMGF) {
    case SMS_AT_PDU: /* PDU mode */

      start=5;
      while(true) {
        if (MessageBuffer[start]==' ' ) break;
        start++;
      }
      start++;

      if (MessageBuffer[start]=='0' || MessageBuffer[start]=='2')
	CurrentSMSMessage->Status=GSS_NOTSENTREAD;
      if (MessageBuffer[start]=='1' || MessageBuffer[start]=='3')
        CurrentSMSMessage->Status=GSS_SENTREAD;

      while(true) {
        if (MessageBuffer[start]==0x0a) break;
        start++;
      }
      start++;

      /* We need to find last char of PDU text. We can't use MessageLength,
         because sometimes some phones (like Siemens M20) return
         some strange things after main PDU */
      end=start+1;
      while(true) {
        if (MessageBuffer[end]==0x0a) break;
        end++;
      }

      DecodeHexBin (buffer, MessageBuffer+start, end-start-1);

      GSM_DecodeETSISMSFrame(CurrentSMSMessage,buffer,(end-start-1)/2);

      CurrentSMSMessageError=GE_NONE;
      break;
    case SMS_AT_TEXT:
      start=19;
        
      start+=ExtractOneParameter(MessageBuffer+start, buffer);

      if (!strcmp(buffer,"\"0\"") || !strcmp(buffer,"\"REC UNREAD\"")) {
	CurrentSMSMessage->Type=GST_SMS;
        CurrentSMSMessage->folder=GST_INBOX;
        CurrentSMSMessage->SMSData=true;
	CurrentSMSMessage->Status=GSS_NOTSENTREAD;
      } else if (!strcmp(buffer,"\"1\"") || !strcmp(buffer,"\"REC READ\"")) {
	CurrentSMSMessage->Type=GST_SMS;
        CurrentSMSMessage->folder=GST_INBOX;
        CurrentSMSMessage->SMSData=true;
	CurrentSMSMessage->Status=GSS_SENTREAD;
      } else if (!strcmp(buffer,"\"2\"") || !strcmp(buffer,"\"STO UNSENT\"")) {
	CurrentSMSMessage->Type=GST_SMS;
        CurrentSMSMessage->folder=GST_OUTBOX;
        CurrentSMSMessage->SMSData=false;
	CurrentSMSMessage->Status=GSS_NOTSENTREAD;
      } else if (!strcmp(buffer,"\"3\"") || !strcmp(buffer,"\"STO SENT\"")) {
	CurrentSMSMessage->Type=GST_SMS;
        CurrentSMSMessage->folder=GST_OUTBOX;
        CurrentSMSMessage->SMSData=false;
	CurrentSMSMessage->Status=GSS_SENTREAD;
      }

      start+=ExtractOneParameter(MessageBuffer+start, buffer);

      /* It's delivery report according to Nokia AT standards */
      if (CurrentSMSMessage->folder==0 && buffer[0]!=0 && //GST_INBOX
          buffer[0]!='"') {

        /* ??? */
        start+=ExtractOneParameter(MessageBuffer+start, buffer);

        /* Sender number */
        /* FIXME: support for all formats */
        start+=ExtractOneParameter(MessageBuffer+start, buffer);
        for (i=1;i<strlen(buffer)-1;i++) buffer[i-1]=buffer[i];
        buffer[i-1]=0;
        strcpy(CurrentSMSMessage->Sender,buffer);

        /* ??? */
        start+=ExtractOneParameter(MessageBuffer+start, buffer);

        /* Sending datetime */
        start+=ExtractOneParameter(MessageBuffer+start, buffer);
        start+=ExtractOneParameter(MessageBuffer+start, buffer2);

        for (i=1;i<strlen(buffer)+1;i++) buffer[i-1]=buffer[i];

        i=strlen(buffer);
        buffer[i++]=',';
        for (j=0;j<strlen(buffer2);j++) buffer[i++]=buffer2[j];
        buffer[i]=0;

        ATDecodeDateTime(&CurrentSMSMessage->Time, buffer);

        /* Date of SMSC response */
        start+=ExtractOneParameter(MessageBuffer+start, buffer);
        start+=ExtractOneParameter(MessageBuffer+start, buffer2);

        for (i=1;i<strlen(buffer)+1;i++) buffer[i-1]=buffer[i];

        i=strlen(buffer);
        buffer[i++]=',';
        for (j=0;j<strlen(buffer2);j++) buffer[i++]=buffer2[j];
        buffer[i]=0;

        ATDecodeDateTime(&DT, buffer);

        /* for compatiblity with GSM_DecodeETSISMSStatusReportData */
        /* FIXME: TimeZone is cut */
        DT.Year=DT.Year-2000;
        GSM_EncodeSMSDateTime(&DT,ETSISMS.SMSCDateTime);

        /* TPStatus */
        start+=ExtractOneParameter(MessageBuffer+start, buffer);
        ETSISMS.TPStatus=0;
        j=1;
        for (i=strlen(buffer)-1;i>=0;i--) {
          ETSISMS.TPStatus=ETSISMS.TPStatus+(buffer[i]-'0')*j;
          j=j*10;
        }

        GSM_DecodeETSISMSStatusReportData(CurrentSMSMessage, &ETSISMS);

        /* NO SMSC number */
        CurrentSMSMessage->MessageCenter.Number[0]=0;

        CurrentSMSMessage->Type = GST_DR;

        /* FIXME: make support for it */
        CurrentSMSMessage->ReplyViaSameSMSC=false;
      } else {
        /* Sender number */
        /* FIXME: support for all formats */
        for (i=1;i<strlen(buffer)-1;i++) buffer[i-1]=buffer[i];
        buffer[i-1]=0;
        strcpy(CurrentSMSMessage->Sender,buffer);

        /* Sender number in alphanumeric format ? */
        start+=ExtractOneParameter(MessageBuffer+start, buffer);
        if (strlen(buffer)!=0) strcpy(CurrentSMSMessage->Sender,buffer);

        /* Sending datetime */
        if (CurrentSMSMessage->folder==0) { //GST_INBOX
          start+=ExtractOneParameter(MessageBuffer+start, buffer);
          start+=ExtractOneParameter(MessageBuffer+start, buffer2);

          for (i=1;i<strlen(buffer)+1;i++) buffer[i-1]=buffer[i];

          i=strlen(buffer);
          buffer[i++]=',';
          for (j=0;j<strlen(buffer2);j++) buffer[i++]=buffer2[j];
          buffer[i]=0;

          ATDecodeDateTime(&CurrentSMSMessage->Time, buffer);
        }

        /* Sender number format */
        start+=ExtractOneParameter(MessageBuffer+start, buffer);

        /* First byte */
        start+=ExtractOneParameter(MessageBuffer+start, buffer);
        ETSISMS.firstbyte=0;
        j=1;
        for (i=strlen(buffer)-1;i>=0;i--) {
          ETSISMS.firstbyte=ETSISMS.firstbyte+(buffer[i]-'0')*j;
          j=j*10;
        }

        CurrentSMSMessage->ReplyViaSameSMSC=false;
        /* GSM 03.40 section 9.2.3.17 (TP-Reply-Path) */
        if ((ETSISMS.firstbyte & 128)==128)
          CurrentSMSMessage->ReplyViaSameSMSC=true;

        /* TP PID.No support now */        
        start+=ExtractOneParameter(MessageBuffer+start, buffer);

        /* TP DCS */
        start+=ExtractOneParameter(MessageBuffer+start, buffer);
        ETSISMS.TPDCS=0;
        j=1;
        for (i=strlen(buffer)-1;i>=0;i--) {
          ETSISMS.TPDCS=ETSISMS.TPDCS+(buffer[i]-'0')*j;
          j=j*10;
        }

        if (CurrentSMSMessage->folder==1) { //GST_OUTBOX
          /*TP VP */
          start+=ExtractOneParameter(MessageBuffer+start, buffer);
        }

        /* SMSC number */
        /* FIXME: support for all formats */
        start+=ExtractOneParameter(MessageBuffer+start, buffer);
        for (i=1;i<strlen(buffer)-1;i++) buffer[i-1]=buffer[i];
        buffer[i-1]=0;
        strcpy(CurrentSMSMessage->MessageCenter.Number,buffer);

        /* Format of SMSC number */
        start+=ExtractOneParameter(MessageBuffer+start, buffer);

        /* TPUDL */
        start+=ExtractOneParameter(MessageBuffer+start, buffer);
        ETSISMS.TPUDL=0;
        j=1;
        for (i=strlen(buffer)-1;i>=0;i--) {
          ETSISMS.TPUDL=ETSISMS.TPUDL+(buffer[i]-'0')*j;
          j=j*10;
        }

        start++;

        CurrentSMSMessage->Coding = GSM_Coding_Default;

        /* GSM 03.40 section 9.2.3.10 (TP-Data-Coding-Scheme) and GSM 03.38 section 4 */
        if ((ETSISMS.TPDCS & 0xf4) == 0xf4) CurrentSMSMessage->Coding=GSM_Coding_8bit;
        if ((ETSISMS.TPDCS & 0x08) == 0x08) CurrentSMSMessage->Coding=GSM_Coding_Unicode;

        switch (CurrentSMSMessage->Coding) {          
          case GSM_Coding_Default:
            /* GSM 03.40 section 9.2.3.23 (TP-User-Data-Header-Indicator) */
            /* If not SMS with UDH, it's coded normal */
            /* If UDH available, treat it as Unicode or 8 bit */
            if ((ETSISMS.firstbyte & 0x40)!=0x40) {
              CurrentSMSMessage->UDHType=GSM_NoUDH;
              for (i=0;i<ETSISMS.TPUDL;i++) {
                if (i>GSM_MAX_SMS_LENGTH) break;
                CurrentSMSMessage->MessageText[i]=MessageBuffer[i+start];
              }
              CurrentSMSMessage->Length=ETSISMS.TPUDL;
              break;
            }
          case GSM_Coding_Unicode:
          case GSM_Coding_8bit:
            DecodeHexBin (ETSISMS.MessageText, MessageBuffer+start, ETSISMS.TPUDL*2);
            GSM_DecodeETSISMSSubmitData(CurrentSMSMessage, &ETSISMS);
            break;
        }
        CurrentSMSMessage->MessageText[CurrentSMSMessage->Length]=0;
      }

      CurrentSMSMessageError=GE_NONE;
  }
}

GSM_Error Nat_GetSMSMessage(GSM_SMSMessage *message)
{
  unsigned char req2[20];
  GSM_Error error;

  if (SMSATCMGF==-1) {
    error=SMSGetPDUType();
    if (error!=GE_NONE) return error;
  }

  CurrentSMSMessage = message;

  sprintf(req2, "AT+CMGR=%i\r",message->Location); //gets SMS

  CurrentSMSMessage->MessageNumber=CurrentSMSMessage->Location;

  return NULL_SendMessageSequence
    (50, &CurrentSMSMessageError, strlen(req2), 0x00, req2);
}

/* FIXME: do we need more than SMS_Submit and SMS_Deliver ? */
GSM_Error GSM_EncodeETSISMSFrame(GSM_SMSMessage *SMS, unsigned char *req, int *length, SMS_MessageType PDU)
{
  GSM_ETSISMSMessage ETSI;
  GSM_Error error;
  int i,current=0;

  error=GSM_EncodeETSISMS(SMS, &ETSI, PDU, length);
  if (error!=GE_NONE) return error;

  for (i=0;i<ETSI.SMSCNumber[0]+1;i++)
    req[current++]=ETSI.SMSCNumber[i];

  req[current++]=ETSI.firstbyte;

  switch (PDU) {
    case SMS_Submit:

      req[current++]=0; //TPMR ignored now
      for (i=0;i<((ETSI.Number[0]+1)/2+1)+1;i++) req[current++]=ETSI.Number[i];
      req[current++]=0; //ETSI.TPPID;
      req[current++]=ETSI.TPDCS;
      req[current++]=ETSI.TPVP;
      req[current++]=ETSI.TPUDL;
      for(i=0;i<*length;i++) req[current++]=ETSI.MessageText[i];
      break;
    case SMS_Deliver:

      for (i=0;i<((ETSI.Number[0]+1)/2+1)+1;i++) req[current++]=ETSI.Number[i];
      req[current++]=0; //ETSI.TPPID;
      req[current++]=ETSI.TPDCS;
      for(i=0;i<7;i++) req[current++]=ETSI.DeliveryDateTime[i];
      req[current++]=ETSI.TPUDL;
      for(i=0;i<*length;i++) req[current++]=ETSI.MessageText[i];
      break;
    default:

      break;
  }

  *length=current;

  return GE_NONE;
}

void Nat_ReplyCSMP(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: SMS parameters set OK\n"));
#endif

  CurrentSMSMessageError=GE_NONE;
}

GSM_Error Nat_SendSaveSMSMessage(GSM_SMSMessage *message,bool savesms)
{
  unsigned char req2[500];
  unsigned char req3[256];
  unsigned char req4[256];
  unsigned char endchar[1];
  GSM_Error error;
  int length,stat,current=0;
  SMS_MessageType PDU;
  GSM_ETSISMSMessage ETSISMS;
  GSM_MessageCenter SMSC;

  if (SMSATCMGF==-1) {
    error=SMSGetPDUType();
    if (error!=GE_NONE) return error;
  }

  CurrentSMSMessage = message;

  if (message->folder==0x00) { //GST_INBOX
    PDU=SMS_Deliver;
    stat=1;
    if (message->Status==GSS_NOTSENTREAD) stat=0;
  } else {
    PDU=SMS_Submit;
    stat=3;
    if (message->Status==GSS_NOTSENTREAD) stat=2;
  }

  endchar[0]=26; //ctrl+z;

  switch (SMSATCMGF) {
    case SMS_AT_PDU: /* PDU mode */
      error=GSM_EncodeETSISMSFrame(message, req3, &length, PDU);
      if (error!=GE_NONE) return error;

      if (savesms) 
        sprintf(req2, "AT+CMGW=%i,%i\r",(length-(req3[0]+1)),stat); //saves SMS
      else
        sprintf(req2, "AT+CMGS=%i\r",(length-(req3[0]+1))); //sends SMS

      EncodeHexBin (req4, req3, length);
      current=length*2;

      Protocol->SendMessage(strlen(req2), 0x00, req2);  usleep(500);

      break;
    case SMS_AT_TEXT:

      if (ATMan!=AT_MAN_NOKIA) {
        switch (CurrentSMSMessage->Coding) {
          case GSM_Coding_Unicode:        
          case GSM_Coding_8bit:
            return GE_NOTSUPPORTED;
          default:
            break;
        }
      }

      error=GSM_EncodeETSISMS(message, &ETSISMS, PDU, &length);
      if (error!=GE_NONE) return error;

      sprintf(req4, "AT+CSMP=%i,%i,%i,%i\r",
                ETSISMS.firstbyte, ETSISMS.TPVP,
                ETSISMS.TPPID, ETSISMS.TPDCS);

      error=NULL_SendMessageSequence
        (50, &CurrentSMSMessageError, strlen(req4), 0x00, req4);
      if (error!=GE_NONE) return error;

      strcpy(SMSC.Number,message->MessageCenter.Number);
      SMSC.No=1;
      error=Nat_SetSMSCenter(&SMSC);
      if (error!=GE_NONE) return error;

      if (savesms) {
        /* Tested AT implementations don't support alphanumeric numbers */
        if ((CurrentSMSMessage->Destination[0]<'0'  ||
             CurrentSMSMessage->Destination[0]>'9') && 
             CurrentSMSMessage->Destination[0]!='+')
        {
          CurrentSMSMessage->Destination[0]='1';
          CurrentSMSMessage->Destination[1]=0;
        }

        sprintf(req4, "AT+CMGW=\"%s\"\r",CurrentSMSMessage->Destination);
      } else {
        /* Tested AT implementations don't support alphanumeric numbers */
        if ((CurrentSMSMessage->Destination[0]<'0'  ||
             CurrentSMSMessage->Destination[0]>'9') && 
             CurrentSMSMessage->Destination[0]!='+')
          return GE_NOTSUPPORTED;

        sprintf(req4, "AT+CMGS=\"%s\"\r",CurrentSMSMessage->Destination);
      }

      Protocol->SendMessage(strlen(req4), 0x00, req4);  usleep(500);

      switch (CurrentSMSMessage->Coding) {
        case GSM_Coding_Default:
          /* If not SMS with UDH, it's as normal text */
          if (CurrentSMSMessage->UDHType==GSM_NoUDH) {
            strcpy(req4,CurrentSMSMessage->MessageText);
            current=strlen(req4);
            break;
          }
        case GSM_Coding_Unicode:        
        case GSM_Coding_8bit:

          EncodeHexBin (req4, ETSISMS.MessageText, length);
          current=length*2;
          break;
      }

      break;
  }

  Protocol->SendMessage(current, 0x00, req4);  usleep(500);

  return NULL_SendMessageSequence
    (500, &CurrentSMSMessageError, 1, 0x00, endchar);
}

void Nat_ReplySaveSMSMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
  int i;

#ifdef DEBUG
  fprintf(stdout, _("Message: SMS Message saved\n"));
#endif

  i=MessageLength-10;
  while(MessageBuffer[i]!=0x20) { i++; }

  i++;
  CurrentSMSMessage->MessageNumber=MessageBuffer[i]-'0';

  i++;
  if (MessageBuffer[i]!=0x0d)
    CurrentSMSMessage->MessageNumber=
      CurrentSMSMessage->MessageNumber*10+MessageBuffer[i]-'0';

#ifdef DEBUG
  fprintf(stdout, _("   location: %i\n"),CurrentSMSMessage->MessageNumber);
#endif

  CurrentSMSMessageError=GE_NONE;
}

GSM_Error Nat_SaveSMSMessage(GSM_SMSMessage *message)
{
  return Nat_SendSaveSMSMessage(message,true);
}

void Nat_ReplySendSMSMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: SMS Message sent OK\n"));
#endif

  CurrentSMSMessageError=GE_SMSSENDOK;
}

GSM_Error Nat_SendSMSMessage(GSM_SMSMessage *message)
{
  return Nat_SendSaveSMSMessage(message,false);
}

void Nat_ReplyDeleteSMSMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

#ifdef DEBUG
  fprintf(stdout, _("Message: SMS deleted OK\n"));
#endif

  CurrentSMSMessageError=GE_NONE;
}

GSM_Error Nat_DeleteSMSMessage(GSM_SMSMessage * message)
{
  char req[128];

  sprintf(req, "AT+CMGD=%d\r", message->Location);

  return NULL_SendMessageSequence
    (50, &CurrentSMSMessageError, strlen(req), 0x00, req);
}

void Nat_ReplyGetSMSStatus(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {
  char *start2;

#ifdef DEBUG
  fprintf(stdout, _("Message: SMS status received\n"));
#endif

  CurrentSMSStatus->UnRead=0;
  CurrentSMSStatus->Number=0;

  start2 = strstr(MessageBuffer, "+CPMS: ");
  start2 += 7;
    
  CurrentSMSStatus->Number=atoi(start2);

  CurrentSMSStatusError=GE_NONE;
}

GSM_Error Nat_GetSMSStatus(GSM_SMSStatus *Status)
{
  CurrentSMSStatus=Status;

  return NULL_SendMessageSequence
    (10, &CurrentSMSStatusError, 13, 0x00, "AT+CPMS=\"SM\"\r");
}

struct AT_OneReply ATRepliesError[] = {

{"AT+CALA"  ,Nat_ReplyAlarmError              },
{"AT+CCLK"  ,Nat_ReplyDateTimeError           },
{"AT+CMGR=" ,Nat_ReplyGetSMSMessageError      },
{"AT+CPBR=" ,Nat_ReplyGetMemoryLocationError  },
{"AT+CPBS?" ,Nat_ReplyGetMemoryStatusCPBSError},
{"AT+CPBS=" ,Nat_ReplyPBKSetMemoryTypeError   },

{"",NULL}
};

struct AT_OneReply ATRepliesOK[] = {

{"AT+CCFC=" ,Nat_ReplyCallDivert            },
{"AT+CGMI\r",Nat_ReplyGetManufacturer       },
{"AT+CPBS?" ,Nat_ReplyGetMemoryStatusCPBS   },
{"AT+CPBS=" ,Nat_ReplyPBKSetMemoryType      },
{"AT+CPBR=?",Nat_ReplyGetMemoryStatusCPBR   },
{"AT+CPBR=" ,Nat_ReplyGetMemoryLocation     },
{"AT+CPBW=" ,Nat_ReplyWritePhonebookLocation},
{"AT+CSCS=\"HEX\"",Nat_ReplySetCharset      },
{"AT+CSCS=\"GSM\"",Nat_ReplySetCharset      },
{"AT+CSCA?" ,Nat_ReplyGetSMSCenter          },
{"AT+CSCA=" ,Nat_ReplySetSMSCenter          },
{"AT+CSDH=1\r",Nat_ReplySMSGetPDUType       },
{"AT+CMGF=0\r",Nat_ReplySMSGetPDUType       },
{"AT+CMGF=1\r",Nat_ReplySMSGetPDUType       },
{"AT+CMGR=" ,Nat_ReplyGetSMSMessage         },
{"AT+CSMP=" ,Nat_ReplyCSMP                  },
{"AT+CMGW=" ,Nat_ReplySaveSMSMessage        },
{"AT+CMGS=" ,Nat_ReplySendSMSMessage        },
{"AT+CMGD=" ,Nat_ReplyDeleteSMSMessage      },
{"AT+CPMS=\"SM\"",Nat_ReplyGetSMSStatus     },
{"AT+CPIN?\r",Nat_ReplyGetSecurityCodeStatus},
{"AT+CCLK?" ,Nat_ReplyGetDateTime           },
{"AT+CCLK=" ,Nat_ReplySetDateTime           },
{"AT+CALA?" ,Nat_ReplyGetAlarm              },
{"AT+CGMM\r",Nat_ReplyGetID                 },
{"AT+CGMR\r",Nat_ReplyGetHW                 },
{"AT+CREG"  ,Nat_ReplyGetNetworkInfo        },
{"AT+COPS?" ,Nat_ReplyGetNetworkInfoCOPS    },
{"AT+CGSN\r",Nat_ReplyGetIMEI               },
{"AT+CHUP"  ,Nat_ReplyCancelCall            },
{"AT+CBC"   ,Nat_ReplyGetBatteryLevel       },
{"AT+CSQ"   ,Nat_ReplyGetRFLevel            },

{"",NULL}
};

void Nat_DispatchMessage(u16 MessageLength, u8 *MessageBuffer, u8 MessageType) {

  bool AnswerOK,AnswerError; //if last part is OK or ERROR

  int i,start=0;

  /* In 9210 first answer start with different, random bytes.
     Here we try to find first AT to find start byte */
  for (i=0;i<MessageLength-1;i++) {
    if (strncmp("AT",MessageBuffer+i,2)==0) {
      start=i;
      break;
    }
  }

  AnswerError=false;
  if (strncmp("ERROR\r",MessageBuffer+(MessageLength-6),6)==0) AnswerError=true;
  if (strncmp("+CMS ERROR: ",MessageBuffer+(MessageLength-16),12)==0) AnswerError=true;

  AnswerOK=false;
  if (strncmp("OK\r",MessageBuffer+(MessageLength-3),3)==0) AnswerOK=true;
  
  if (AnswerOK) {
    i=0;
    while (true) {
      if (ATRepliesOK[i].ReplyFunction==NULL) break;
      if (strncmp(ATRepliesOK[i].ReplyString,MessageBuffer+start,
           strlen(ATRepliesOK[i].ReplyString))==0) {
        ATRepliesOK[i].ReplyFunction(MessageLength-start,MessageBuffer+start,MessageType);
        break;
      }
      i++;
    }

#ifdef DEBUG
    if (ATRepliesOK[i].ReplyFunction==NULL)
      fprintf(stdout,_("Message: unknown AT command\n"));
#endif
  }

  if (AnswerError) {
    i=0;
    while (true) {
      if (ATRepliesError[i].ReplyFunction==NULL) break;
      if (strncmp(ATRepliesError[i].ReplyString,MessageBuffer+start,
           strlen(ATRepliesError[i].ReplyString))==0) {
        ATRepliesError[i].ReplyFunction(MessageLength-start,MessageBuffer+start,MessageType);
        break;
      }
      i++;
    }

#ifdef DEBUG
    if (ATRepliesError[i].ReplyFunction==NULL)
      fprintf(stdout,_("Message: unknown error AT command\n"));
#endif
  }
}
