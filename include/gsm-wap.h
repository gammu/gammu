/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.
  
  Functions for manipulating WAP Settings and bookmarks

*/

#ifndef __gsm_wap_h
#define __gsm_wap_h

#include "gsm-sms.h"

typedef struct {
  char address[255+1];
  char title[50+1];
  u16 location;
} GSM_WAPBookmark;

typedef struct {
  u8 location;
  char homepage[100+1];
  char title[20+1];
  int bearer;
  bool issecurity;
  bool iscontinuous;
  
  /* for data bearer */
  char ipaddress[20+1];
  char dialup[20+1];
  char user[50+1];     //is length OK ?
  char password[50+1]; //is length OK ?
  bool isspeed14400;
  bool isISDNcall;
  bool isnormalauthentication;

  /* for sms bearer */
  char server[21+1];

  /* for sms or ussd bearer */
  char service[20+1];
  bool isIP;
  
  /* for ussd bearer */
  char code[10+1];
} GSM_WAPSettings;

#define WAPSETTINGS_BEARER_SMS  0x00
#define WAPSETTINGS_BEARER_DATA 0x01
#define WAPSETTINGS_BEARER_USSD 0x02

int GSM_SaveWAPBookmarkToSMS(GSM_MultiSMSMessage *SMS,
                             GSM_WAPBookmark *bookmark);

GSM_Error GSM_ReadWAPBookmark(GSM_SMSMessage *message, GSM_WAPBookmark *bookmark);

int GSM_SaveWAPSettingsToSMS(GSM_MultiSMSMessage *SMS,
                             GSM_WAPSettings *settings);

#endif
