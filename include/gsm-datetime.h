/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#ifndef __gsm_datetime_h
#define __gsm_datetime_h

/* Define enum used to describe what sort of date/time support is
   available. */
typedef enum {
  GDT_None,     /* The mobile phone doesn't support time and date. */
  GDT_TimeOnly, /* The mobile phone supports only time. */
  GDT_DateOnly, /* The mobile phone supports only date. */
  GDT_DateTime  /* Wonderful phone - it supports date and time. */
} GSM_DateTimeSupport;

/* Structure used for passing dates/times to date/time functions such as
   GSM_GetTime and GSM_GetAlarm etc. */
typedef struct {
  bool IsSet;      /* for time=is set and available ? for alarm=is set ? */
  int Year;          /* The complete year specification - e.g. 1999. Y2K :-) */
  int Month;	     /* January = 1 */
  int Day;
  int Hour;
  int Minute;
  int Second;
  int Timezone;      /* The difference between local time and GMT */
} GSM_DateTime;

char *DayOfWeek (int year, int month, int day);
void GetMachineDateTime( char nowdate[], char nowtime[] );

void EncodeDateTime(unsigned char* buffer, GSM_DateTime *datetime);
void DecodeDateTime(unsigned char* buffer, GSM_DateTime *datetime);

#endif
