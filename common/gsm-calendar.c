/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.
	
  Functions for manipulating calendar

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef WIN32
  #include "misc_win32.h"
#else
  #include <unistd.h>
#endif

#include "gsm-api.h"
#include "gsm-coding.h"

char *GSM_GetVCALENDARStart(int version) {

  static char Buffer[1000]="";
  int len=0;

  len+=sprintf(Buffer, "BEGIN:VCALENDAR%c%c",13,10);

  if (version==10) sprintf(Buffer+len, "VERSION:1.0%c%c",13,10);
  if (version==30) sprintf(Buffer+len, "VERSION:3.0%c%c",13,10);

  return Buffer;
}

char *GSM_GetVCALENDARNote(GSM_CalendarNote *note, int version) {

  static char Buffer[1000]="";

  int len=0;

  u8 buffer[50*3];

  len+=sprintf(Buffer+len, "BEGIN:VEVENT%c%c",13,10);
  len+=sprintf(Buffer+len, "CATEGORIES:");
  switch (note->Type) {
    case GCN_REMINDER:
      len+=sprintf(Buffer+len, "MISCELLANEOUS%c%c",13,10);
      break;
    case GCN_CALL:
      len+=sprintf(Buffer+len, "PHONE CALL%c%c",13,10);
      break;
    case GCN_MEETING:
      len+=sprintf(Buffer+len, "MEETING%c%c",13,10);
      break;
    case GCN_BIRTHDAY:
      len+=sprintf(Buffer+len, "SPECIAL OCCASION%c%c",13,10);
      break;
    default:
      len+=sprintf(Buffer+len, "UNKNOWN%c%c",13,10);
      break;
  }

  if( note->Type == GCN_CALL && strcmp( note->Phone, "" ) ) {

    if (note->Text[0]!=0 && version==30)
      len+=sprintf(Buffer+len, "DESCRIPTION:%s%c%c",note->Text,13,10);

    EncodeUTF8(buffer,note->Phone,strlen(note->Phone));
    if (strlen(note->Phone)==strlen(buffer)) {
      len+=sprintf(Buffer+len, "SUMMARY:%s%c%c",note->Phone,13,10);
    } else {
      len+=sprintf(Buffer+len, "SUMMARY;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c",buffer,13,10);
    }
	    
  } else {

    EncodeUTF8(buffer,note->Text,strlen(note->Text));
    if (strlen(note->Text)==strlen(buffer)) {
      len+=sprintf(Buffer+len, "SUMMARY:%s%c%c",note->Text,13,10);
    } else {
      len+=sprintf(Buffer+len, "SUMMARY;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:%s%c%c",buffer,13,10);
    }

  }

  /* For 3310: date is set to 2090! */
  if (GetModelFeature (FN_CALENDAR)==F_CAL33) {
    len+=sprintf(Buffer+len, "DTSTART:1999%02d%02dT%02d%02d%02d%c%c",
            note->Time.Month, note->Time.Day, note->Time.Hour,
            note->Time.Minute, note->Time.Second,13,10);
  } else {
    len+=sprintf(Buffer+len, "DTSTART:%04d%02d%02dT%02d%02d%02d%c%c", note->Time.Year,
            note->Time.Month, note->Time.Day, note->Time.Hour,
            note->Time.Minute, note->Time.Second,13,10);
  }

  if (note->Alarm.Year!=0) {
    len+=sprintf(Buffer+len, "DALARM:%04d%02d%02dT%02d%02d%02d%c%c", note->Alarm.Year,
            note->Alarm.Month, note->Alarm.Day, note->Alarm.Hour,
            note->Alarm.Minute, note->Alarm.Second,13,10);
  }

  if ( GetModelFeature (FN_CALENDAR)==F_CAL71 && version==30 && note->Type == GCN_BIRTHDAY ) {
        len+=sprintf(Buffer+len, "ALTYPE:%s%c%c", (note->AlarmType==0x00) ?
                                              "TONE" : "SILENT",13,10);
  }

  if ( GetModelFeature (FN_CALENDAR)==F_CAL71 && note->Recurrance!= 0 ) {
    switch(note->Recurrance/24) {
      case 1  :len+=sprintf(Buffer+len, "RRULE:D1 #0%c%c",13,10);break;
      case 7  :len+=sprintf(Buffer+len, "RRULE:W1 #0%c%c",13,10);break;
      case 14 :len+=sprintf(Buffer+len, "RRULE:W2 #0%c%c",13,10);break;
      case 365:len+=sprintf(Buffer+len, "RRULE:YD1 #0%c%c",13,10);break;
    }
  }

  len+=sprintf(Buffer+len, "END:VEVENT%c%c",13,10);

  return Buffer;
}

char *GSM_GetVCALENDAREnd(int version) {

  static char Buffer[1000]="";

  sprintf(Buffer, "END:VCALENDAR%c%c",13,10);

  return Buffer;
}

int GSM_SaveCalendarNoteToSMS(GSM_MultiSMSMessage *SMS,
                              GSM_CalendarNote *note)
{
  char Buffer[1000]="";
  int length;
  GSM_UDH UDH=GSM_NoUDH;

  sprintf(Buffer,GSM_GetVCALENDARStart(10));
  sprintf(Buffer+strlen(Buffer),GSM_GetVCALENDARNote(note,10));  
  sprintf(Buffer+strlen(Buffer),GSM_GetVCALENDAREnd(10));

  /* 1 SMS: no UDH */
  if (strlen(Buffer)<=160-8) { //8==length of ..SCKE4.
    sprintf(Buffer,"//SCKE4 ");  
    sprintf(Buffer+strlen(Buffer),GSM_GetVCALENDARStart(10));
    sprintf(Buffer+strlen(Buffer),GSM_GetVCALENDARNote(note,10));  
    sprintf(Buffer+strlen(Buffer),GSM_GetVCALENDAREnd(10));
  } else {
    UDH=GSM_CalendarNoteUDH;
  }

  length=strlen(Buffer);
  GSM_MakeMultiPartSMS2(SMS,Buffer,length, UDH, GSM_Coding_Default);

  return 0;
}
