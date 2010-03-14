/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.
	
  Functions for manipulating WAP settings and bookmarks
  
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "gsm-common.h"
#include "gsm-wap.h"
#include "gsm-sms.h"
#include "gsm-coding.h"

int GSM_SaveWAPBookmarkToSMS(GSM_MultiSMSMessage *SMS,
                             GSM_WAPBookmark *bookmark)
{
  unsigned char Data[500];
  GSM_UDH UDHType;
  u16 current=0;
  u8 buffer[50*3];

  strcpy(Data+current,"\x01\x06-\x1F");current=current+4;
  strcpy(Data+current,"+application/x-wap-prov.browser-bookmarks");current=current+12+29;
  Data[current++]=0x00;

  strcpy(Data+current,"\x81\xEA");current=current+2;

  EncodeUTF8(buffer,bookmark->title,strlen(bookmark->title));
  if (strlen(bookmark->title)==strlen(buffer)) {
    strcpy(Data+current,"\x00");current=current+1;
    strcpy(Data+current,"\x01\x00");current=current+2;
  } else {
    strcpy(Data+current,"\x01\x01\x87\x68\x00");current=current+5;
  }

  strcpy(Data+current,"\x45\xC6\x7F");current=current+3;

  /* The same to "title" block in WAP settings */
  strcpy(Data+current,"\x01\x87\x15\x11\x03");current=current+5;
  if (strlen(bookmark->title)==strlen(buffer)) {
    strcpy(Data+current,bookmark->title);current=current+strlen(bookmark->title);
  } else {
    EncodeUnicode (Data+current, bookmark->title, strlen(bookmark->title));
    current=current+2*strlen(bookmark->title);
    Data[current++]=0x00;
  }
  Data[current++]=0x00;

  /* The same to "homepage" block in WAP settings */
  strcpy(Data+current,"\x01\x87\x17\x11\x03");current=current+5;
  if (strlen(bookmark->title)==strlen(buffer)) {
    strcpy(Data+current,bookmark->address);current=current+strlen(bookmark->address);
  } else {
    EncodeUnicode (Data+current, bookmark->address, strlen(bookmark->address));
    current=current+2*strlen(bookmark->address);
    Data[current++]=0x00;
  }
  Data[current++]=0x00;

  /* Mark of end */
  strcpy(Data+current,"\x01\x01\x01");current=current+3;

  if (current>(140-7)) { //more than 1 SMS
    UDHType=GSM_WAPBookmarkUDHLong;
  } else {
    UDHType=GSM_WAPBookmarkUDH;
  }

  GSM_MakeMultiPartSMS2(SMS, Data,current, UDHType, GSM_Coding_Default);
  
  return 0;
}

GSM_Error GSM_ReadWAPBookmark(GSM_SMSMessage *message, GSM_WAPBookmark *bookmark)
{
  unsigned char Data[250];
  int current=0,i,j;

  switch (message->UDHType) {
  case GSM_WAPBookmarkUDH:

    strcpy(Data+current,"\x01\x06-\x1F");current=current+4;
    strcpy(Data+current,"+application/x-wap-prov.browser-bookmarks");current=current+12+29;
    Data[current++]=0x00;

    strcpy(Data+current,"\x81\xEA\x00");current=current+3;

    strcpy(Data+current,"\x01\x00");current=current+2;
    strcpy(Data+current,"\x45\xC6\x7F");current=current+3;

    strcpy(Data+current,"\x01\x87\x15\x11\x03");current=current+5;

    for (i=0;i<current;i++)
      if (message->MessageText[i]!=Data[i]) return GE_UNKNOWN;

    j=0;
    while (message->MessageText[current]!=0x00) {
      bookmark->title[j++]=message->MessageText[current];
      current++;
    }
    bookmark->title[j]=0;    

    current=current+6;

    j=0;
    while (message->MessageText[current]!=0x00) {
      bookmark->address[j++]=message->MessageText[current];
      current++;
    }
    bookmark->address[j]=0;    

    break;

  default: /* error */
    return GE_UNKNOWN;
    break;
  }

  return GE_NONE;
}

/* Made from sniff of WAP settings sent from Logo Manager with such file:
[GSMCSD]
Name=Orange
URL=http://OtherSites/
Gateway=192.168.71.81
; 9200=Temporary,Normal 9201=Continuous,Normal 9202=Temp,Secure 9203=Continuous,Secure
Port=9201
DialString=+447973100501
; Auth Type can be PAP, CHAP or MS_CHAP
PPPAuthType=PAP
PPPAuthName=Orange
PPPAuthPass=Multimedia
; ISDN or ANALOGUE
CallType=ISDN
CallSpeed=14400

SMS contenst:

0106
,
1F
*application/x-wap-prov.browser-settings
00
81ea01016a
0045c606
0187121145
0187141101
0187131103
00
0187211103
00
0187281172
018729116b
01872211700101c608
0187151103
   settings->title
00
01
0186071103
   settings->homepage
0001c67f
0187151103
   settings->title
00
0187171103
   settings->homepage
00
010101
*/

/* Function created from software from
   http://www.tektonica.com/projects/gsmsms/ */
int GSM_SaveWAPSettingsToSMS(GSM_MultiSMSMessage *SMS,
                             GSM_WAPSettings *settings)
{
  unsigned char Data[500];
  u16 current=0;

  strcpy(Data+current,"\x01\x06\x04\x03\x94");current=current+5;

  strcpy(Data+current,"\x81\xEA\x00");current=current+3;

  strcpy(Data+current,"\x01\x00");current=current+2;
  strcpy(Data+current,"\x45\xC6\x06");current=current+3;

  strcpy(Data+current,"\x01\x87\x12");current=current+3;
  switch (settings->bearer) {
    case WAPSETTINGS_BEARER_DATA:Data[current++]=0x45;break;
    case WAPSETTINGS_BEARER_SMS :Data[current++]=0x41;break;
  }

  strcpy(Data+current,"\x01\x87\x13\x11\x03");current=current+5;
  strcpy(Data+current,settings->ipaddress);current=current+strlen(settings->ipaddress);
  Data[current++]=0x00;

  strcpy(Data+current,"\x01\x87\x14");current=current+3;
  if (settings->iscontinuous) { Data[current++]=0x61; } //continuous
                         else { Data[current++]=0x60; } //temporary

  strcpy(Data+current,"\x01\x87\x21\x11\x03");current=current+5;
  strcpy(Data+current,settings->dialup);current=current+strlen(settings->dialup);
  Data[current++]=0x00;

  strcpy(Data+current,"\x01\x87\x22");current=current+3;
  if (settings->isnormalauthentication) {
    Data[current++]=0x70;//OTA_CSD_AUTHTYPE_PAP. Correct ?
  } else {
    Data[current++]=0x71;//OTA_CSD_AUTHTYPE_CHAP. Correct ?
  }

  strcpy(Data+current,"\x01\x87\x23\x11\x03");current=current+5;
  strcpy(Data+current,settings->user);current=current+strlen(settings->user);
  Data[current++]=0x00;

  strcpy(Data+current,"\x01\x87\x24\x11\x03");current=current+5;
  strcpy(Data+current,settings->password);current=current+strlen(settings->password);
  Data[current++]=0x00;

  strcpy(Data+current,"\x01\x87\x28");current=current+3;
  if (settings->isISDNcall) { Data[current++]=0x73; } //ISDN
                       else { Data[current++]=0x72; } //analogue

  strcpy(Data+current,"\x01\x87\x29");current=current+3;
//use constant    OTA_CSD_CALLSPEED_AUTO => '6A';
  if (settings->isspeed14400) { Data[current++]=0x6C; } //14400
                         else { Data[current++]=0x6B; } //9600

  Data[current++]=0x01;
  strcpy(Data+current,"\x01\x86\x07\x11\x03");current=current+5;
  strcpy(Data+current,settings->homepage);current=current+strlen(settings->homepage);
  Data[current++]=0x00;

  strcpy(Data+current,"\x01\xC6\x08");current=current+3;
  strcpy(Data+current,"\x01\x87\x15\x11\x03");current=current+5;
  strcpy(Data+current,settings->title);current=current+strlen(settings->title);
  Data[current++]=0x00;

  /* Mark of end */
  strcpy(Data+current,"\x01\x01\x01");current=current+3;

  GSM_MakeMultiPartSMS2(SMS, Data,current, GSM_WAPSettingsUDH, GSM_Coding_Default);

  return 0;
}
