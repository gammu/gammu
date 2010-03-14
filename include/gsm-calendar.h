/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

*/

#ifndef __gsm_calendar_h
#define __gsm_calendar_h

#include "gsm-datetime.h"
#include "gsm-sms.h"

/* Define enums for Calendar Note types */
typedef enum {
  GCN_REMINDER=1, /* Reminder */
  GCN_CALL,       /* Call */
  GCN_MEETING,    /* Meeting */
  GCN_BIRTHDAY    /* Birthday */
} GSM_CalendarNoteType;

#define MAX_CALENDAR_TEXT_LENGTH 0x3a /* In 6210 max. 48 chars. 10 more precaution bytes*/
#define MAX_CALENDAR_PHONE_LENGTH 0x24

/* Calendar note type */

typedef struct {
  int Location;              /* The number of the note in the phone memory */
  GSM_CalendarNoteType Type; /* The type of the note */
  GSM_DateTime Time;         /* The time of the note */
  GSM_DateTime Alarm;        /* The alarm of the note */
  char Text[MAX_CALENDAR_TEXT_LENGTH+1];   /* The text of the note */
  char Phone[MAX_CALENDAR_PHONE_LENGTH+1]; /* For Call only: the phone number */

  /* the following are only used for N7110/N6210 */
  int Recurrance; /* after how many hours note repeats. 0x0000 = No Repeat */
  int YearOfBirth; /* Only in GCN_BIRTHDAY for 62/71 - The Year of birth of recurrance */
  u8 AlarmType;    /* Only in GCN_BIRTHDAY for 62/71 - 0x00 = Tone, 0x01 = Silent */

  bool ReadNotesInfo; /* Important in 6210/7110 - if we should read notes info or not */
} GSM_CalendarNote;

/* NotesInfo type : Nowadays only N6210/7110 */
#define MAX_NOTES_INFO_ELEMS 1024

typedef struct {
    u16 HowMany;                        /* How many notes info used */
    u16 Location[MAX_NOTES_INFO_ELEMS]; /* In N6210/7110 adresses for notes */
} GSM_NotesInfo;

char *GSM_GetVCALENDARStart(int version);
char *GSM_GetVCALENDARNote(GSM_CalendarNote *note, int version);
char *GSM_GetVCALENDAREnd(int version);

int GSM_SaveCalendarNoteToSMS(GSM_MultiSMSMessage *SMS,
                              GSM_CalendarNote *note);

#endif	/* __gsm_calendar_h */
