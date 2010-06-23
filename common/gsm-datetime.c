/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#include <string.h>
#include <time.h>

#include "misc.h"
#include "gsm-datetime.h"
#include "gsm-common.h"

char DayOfWeekChar;

/* Based on article in Polish PC-Kurier 8/1998 page 104 */
/* Archive on http://www.pckurier.pl */
char *DayOfWeek (int year, int month, int day) {
  int p,q,r,w;

  p=(14-month) / 12;
  q=month+12*p-2;
  r=year-p;
  w=(day+(31*q) / 12 + r + r / 4 - r / 100 + r / 400) % 7;
  strcpy(&DayOfWeekChar,"");
  switch (w) {
    case 0: strcpy(&DayOfWeekChar,"Sun"); break;
    case 1: strcpy(&DayOfWeekChar,"Mon"); break;
    case 2: strcpy(&DayOfWeekChar,"Tue"); break;
    case 3: strcpy(&DayOfWeekChar,"Wed"); break;
    case 4: strcpy(&DayOfWeekChar,"Thu"); break;
    case 5: strcpy(&DayOfWeekChar,"Fri"); break;
    case 6: strcpy(&DayOfWeekChar,"Sat"); break;
  }
  return &DayOfWeekChar;
}

/* GetMachineDateTime ----------------------------------------------------------
Purpose:    Gets system date and time.
Use: -
Input:    -
Output:   date - System date in the format "aaaa-mm-gg".
          time  - System time in the format "hh:mm:ss".
Return:  -
----------------------------------------------------------------------------- */
void GetMachineDateTime( char nowdate[], char nowtime[] )
{
  struct tm *now;
  time_t nowh;

  nowh=time(NULL);
  now=localtime(&nowh);

  /* I have 100 (for 2000) Year now :-) */
  if (now->tm_year>99 && now->tm_year<1900) {
    now->tm_year=now->tm_year+1900;
  }

  sprintf(nowdate,"%04d-%02d-%02d",now->tm_year,now->tm_mon+1,now->tm_mday);
  sprintf(nowtime, "%02d:%02d:%02d",now->tm_hour,now->tm_min,now->tm_sec);

}

void EncodeDateTime(unsigned char* buffer, GSM_DateTime *datetime)
{
  buffer[0] = datetime->Year / 256;
  buffer[1] = datetime->Year % 256;
  buffer[2] = datetime->Month;
  buffer[3] = datetime->Day;

  buffer[4] = datetime->Hour;
  buffer[5] = datetime->Minute;
}

void DecodeDateTime(unsigned char* buffer, GSM_DateTime *datetime)
{
  datetime->Year = buffer[0] * 256 + buffer[1];
  datetime->Month = buffer[2];
  datetime->Day = buffer[3];

  datetime->Hour = buffer[4];
  datetime->Minute = buffer[5];
  datetime->Second = buffer[6];
}
