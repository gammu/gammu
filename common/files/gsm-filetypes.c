/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Copyright (C) 1999, 2000 Hugh Blemings & Pavel Janík ml. 

  Released under the terms of the GNU GPL, see file COPYING for more details.
	
  Functions to read and write common file types.
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#ifndef VC6
#include <unistd.h>
#endif

#include "gsm-common.h"
#include "gsm-ringtones.h"
#include "gsm-bitmaps.h"
#include "files/gsm-filetypes.h"
#include "files/midifile.h"
#include "gsm-coding.h"
#include "misc.h"
#include "newmodules/n7110.h"
#include "newmodules/n6110.h"

#ifdef XPM
  #include <X11/xpm.h>
#endif

/**
* GetvCalTime
*
* Fills vCalendar time string into GSM_DateTime structure
*
* in:
*   dt:  datetime structure
*   time:  string in format yyyymmddThhmmss
* out:
*   <>0 if error
*/
int GetvCalTime(GSM_DateTime *dt, char *time)
{
  char year[5]="", month[3]="", day[3]="", hour[3]="", minute[3]="", second[3]="";
  dt->Year=dt->Month=dt->Day=dt->Hour=dt->Minute=dt->Second=dt->Timezone=0;

  strncpy(year, time, 4);
  strncpy(month, time+4, 2);
  strncpy(day, time+6, 2);
  strncpy(hour, time+9, 2);
  strncpy(minute, time+11, 2);
  strncpy(second, time+13, 2);

/* FIXME: Should check ranges... */
  dt->Year=atoi(year);
  dt->Month=atoi(month);
  dt->Day=atoi(day);
  dt->Hour=atoi(hour);
  dt->Minute=atoi(minute);
  dt->Second=atoi(second);
/* FIXME */
  dt->Timezone=0;

  return 0;
}

/**
* FillCalendarNote
*
* Fills calendar data from strings into calendar note
*
* in:
*   note:  calendar note structure
*   type:  type of calendar note
*   text:  text or phonenumber
*   time:  string in format yyyymmddThhmmss
*   alarm: dito
* out:
*   <>0 if error
*/
int FillCalendarNote(GSM_CalendarNote *note, char *type,
                       char *text, char *time, char *alarm)
{
  GetvCalTime(&note->Time, time);
  GetvCalTime(&note->Alarm, alarm);

  note->Location=0; 

  strncpy(note->Text, text, MAX_CALENDAR_TEXT_LENGTH);
  strcpy(note->Phone, ""); /* correct in most cases */

  /* FIXME: Handle additional strings, maybe from configuration file */

  if(!strcmp(type, "PHONE CALL"))
  {
    strncpy(note->Phone, text, MAX_CALENDAR_PHONE_LENGTH);
    note->Type=GCN_CALL;
  }
  else if(!strcmp(type, "MEETING"))
      note->Type=GCN_MEETING;
  else if(!strcmp(type, "SPECIAL OCCASION"))
      note->Type=GCN_BIRTHDAY;
  else
      note->Type=GCN_REMINDER;

  return 0;
}

/**
* GSM_ReadVCalendarFile
*
* Reads vCalendar file
*
* in:
*   FileName: name of vCalendar file 
*   cnote:  pointer to calendar note
*   number:  number in file of calendar note to read
* out:
*   <>0 if error
*/
GSM_Error GSM_ReadVCalendarFile(char *FileName, GSM_CalendarNote *cnote, int *number)
{
  FILE *file;
  char type[21]="", text[40]="", time[16]="", alarm[16]="";
	char phone[40]="";
	long recurr=0L;
	char altype=0x00;

  int veventcounter=0;
  int isOK=0;
  
  bool NoteOK=false;

  char *Line, OLine[1024], BackLine[1024];

  Line = OLine;

  file=fopen(FileName, "r");    
  if (!file) {
#ifdef DEBUG
    fprintf(stderr, _("File cannot be opened!\n"));
#endif
    return GE_CANTOPENFILE;
  }

  /* Go through data from file. */
  while (GetLine(file, Line, sizeof(OLine))!=-1) {

    strcpy(BackLine, Line);

    switch (isOK) {
      case 0:
        if (!strcmp(Line,"BEGIN:VCALENDAR"))
	  isOK++;
	break;
      case 1:
        if (!strcmp(Line,"BEGIN:VEVENT")) {
          isOK++;
          veventcounter++;
	}
        if (!strcmp(Line,"END:VCALENDAR"))
          isOK--;	
	break;
      case 2:
        if (veventcounter==*number) {
          if (!strncmp(Line,"CATEGORIES:",11)) {
            strncpy(type,Line+11,strlen(Line)-11);
	    type[strlen(Line)-11]=0;
	  }
	  if (!strncmp(Line,"DESCRIPTION:",12)) {
	    strncpy(phone,Line+12,strlen(Line)-12);
	    phone[strlen(Line)-12]=0;
	  }
	  if (!strncmp(Line,"SUMMARY:",8)) {
	    strncpy(text,Line+8,strlen(Line)-8);
	    text[strlen(Line)-8]=0;
	  }
	  if (!strncmp(Line,"SUMMARY;CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:",48)) {
	    DecodeUTF8(text,Line+48,strlen(Line)-48);
	    text[strlen(Line)-48]=0;
	  }
	  if (!strncmp(Line,"DTSTART:",8)) {
	    strncpy(time,Line+8,strlen(Line)-8);
	    time[strlen(Line)-8]=0;
	  }
	  if (!strncmp(Line,"DALARM:",7)) {
	    strncpy(alarm,Line+7,strlen(Line)-7);
	    alarm[strlen(Line)-7]=0;
	  }

          /* Obsolete */
	  if (!strncmp(Line,"RECURR:",7)) {
	    recurr=mem_to_int(Line+7,strlen(Line)-7);
	  }

	  if (!strncmp(Line,"RRULE:D1 :",9)) {
	    recurr=1;
	  }
	  if (!strncmp(Line,"RRULE:W1 :",9)) {
	    recurr=7;
	  }
          if (!strncmp(Line,"RRULE:W2 :",9)) {
	    recurr=14;
	  }
          if (!strncmp(Line,"RRULE:YD1 :",10)) {
	    recurr=365;
	  }

	  if (!strncmp(Line,"ALTYPE:",7)) {
	    altype=(!strncmp("TONE",Line+7,4)) ? 0x00 : 0x01;
	  }
        }
	if (!strcmp(Line,"END:VEVENT")) {
          if (veventcounter==*number) NoteOK=true;
	  isOK--;
	}
	break;
    }
    
    if (NoteOK) break;
  }
    
  if (!NoteOK) {
    *number=veventcounter;
#ifdef DEBUG
    fprintf(stdout,_("Note not found in VCalendarfile\n"));
#endif
    return GE_TOOSHORT;
  }
  
  FillCalendarNote(cnote, type, text, time, alarm);

  cnote->Recurrance = recurr*24; /* it was in days. I convert in hours */
  cnote->AlarmType = altype;

  if( strcmp( phone, "" ) ) { /* Invert data if CALL ... */
    strcpy( cnote->Text, phone );
    strcpy( cnote->Phone, text ); // alread FillCalendar does it ..
  }

  fclose(file);
  
  return 0;
}

GSM_Error GSM_ReadBinRingtoneFile(char *FileName, GSM_BinRingtone *ringtone)
{
  FILE *file;
  int i;

  file = fopen(FileName, "rb");

  if (!file)
    return(GE_CANTOPENFILE);

  ringtone->length=fread(ringtone->frame, 1, 500, file);

  fclose(file);

  if (ringtone->frame[0]==0x00 && ringtone->frame[1]==0x00 &&
      ringtone->frame[2]==0x0C && ringtone->frame[3]==0x01) {
    i=5;
    while (ringtone->frame[i]!=0x00) {
      ringtone->name[i-5]=ringtone->frame[i];
      i++;
    }
    ringtone->name[i-5]=0x00;

    return GE_NONE;
  }

  if (ringtone->frame[0]==0x00 && ringtone->frame[1]==0x00 &&
      ringtone->frame[2]==0x00 && ringtone->frame[4]==0x00) {

    /* DCT3, Unicode subformat, 62xx & 7110 */
    if (ringtone->frame[18]==0x00 && ringtone->frame[21]!=0x02) {

      DecodeUnicode(ringtone->name,ringtone->frame+18,ringtone->frame[17]);

      ringtone->frame[0]=0x00;
      ringtone->frame[1]=0x00;
      ringtone->frame[2]=0x0c;
      ringtone->frame[3]=0x01;
      ringtone->frame[4]=0x2c;   
      i=5;
      memcpy(ringtone->frame+i,ringtone->name,strlen(ringtone->name));
      i=i+strlen(ringtone->name);
      ringtone->frame[i++]=0x00;       
      ringtone->frame[i++]=0x00;
      memcpy(ringtone->frame+i,ringtone->frame+(strlen(ringtone->name)*2+21),ringtone->length-(strlen(ringtone->name)*2+21));
      ringtone->length=i+ringtone->length-(strlen(ringtone->name)*2+21);

    } else {
      /* DCT3, normal subformat, 32xx/33xx/51xx/5210/5510/61xx/8xxx */      

      for (i=0;i<ringtone->frame[16];i++) ringtone->name[i]=ringtone->frame[i+17];
      ringtone->name[i]=0;

      ringtone->frame[0]=0x00;
      ringtone->frame[1]=0x00;
      ringtone->frame[2]=0x0c;
      ringtone->frame[3]=0x01;
      ringtone->frame[4]=0x2c;   
      i=5;
      memcpy(ringtone->frame+i,ringtone->name,strlen(ringtone->name));
      i=i+strlen(ringtone->name);
      ringtone->frame[i++]=0x00;       
      ringtone->frame[i++]=0x00;
      memcpy(ringtone->frame+i,ringtone->frame+(strlen(ringtone->name)+19),ringtone->length-(strlen(ringtone->name)+19));
      ringtone->length=i+ringtone->length-(strlen(ringtone->name)+19);
    
    }

#ifdef DEBUG
    fprintf(stdout,_("\"%s\"\n"),ringtone->name);

    for (i=0;i<ringtone->length;i++) fprintf(stdout,_("%02x "),ringtone->frame[i]);
#endif

    return GE_NONE;
  }

  return GE_NOTSUPPORTED;
}

/* Function to convert scale field in to correct number. */
int GetRTTLDuration (char *num)
{

int duration=0;

 switch (atoi(num)) {
 
 case  1: duration=128; break;
 case  2: duration= 64; break;
 case  4: duration= 32; break;
 case  8: duration= 16; break;
 case 16: duration=  8; break;
 case 32: duration=  4; break;
 }
   
 return (duration);

}

int GetRTTLScale (char *num)
{

  /* This may well need improving. */

  int scale=0;

  if ((atoi(num))<4) scale=(atoi(num));
  if ((atoi(num))>4) scale=(atoi(num))-4;

  return (scale);
}

GSM_Error GSM_ReadRingtoneFile(char *FileName, GSM_Ringtone *ringtone)
{
  FILE *file;
  unsigned char buffer[300];
  GSM_Error error;
  GSM_Filetypes filetype=RTTL;

  file = fopen(FileName, "rb");

  if (!file)
    return(GE_CANTOPENFILE);

  fread(buffer, 1, 4, file); /* Read the header of the file. */

  /* Attempt to identify filetype */

  if (memcmp(buffer, "MThd",3)==0)  /* MIDI files have 'MThd' at the start */
    filetype=MIDI;

  if (buffer[0]==0xc7 && buffer[1]==0x45 && buffer[2]==0xc1 && buffer[3]==0x53)
    filetype=COMMUNICATOR;

  if (strstr(FileName,".ott")) filetype=OTT; /* OTT files saved by NCDS3 */
  
  error=GE_NONE;
  
  rewind(file);

  switch (filetype) {
  case RTTL:
    error=loadrttl(file,ringtone);
    fclose(file);
    break;
  case OTT:
    error=loadott(file,ringtone);
    fclose(file);
    break;
  case COMMUNICATOR:
    error=loadcommunicator(file,ringtone);
    fclose(file);
    break;
  case MIDI:
    fclose(file);
    error=loadmid(FileName,ringtone);
    break;
  default:
    error=GE_INVALIDFILEFORMAT;
  }

  return(error);

}

GSM_Error loadott(FILE *file, GSM_Ringtone *ringtone)
{
  char Buffer[2000];
  int i;
  
  i=fread(Buffer, 1, 2000, file);

  if (!feof(file)) return GE_TOOLONG;
  
  return GSM_UnPackRingtone(ringtone, Buffer, i);
}

GSM_Error loadcommunicator(FILE *file, GSM_Ringtone *ringtone)
{
  char Buffer[4000];
  int i,j;
  
  i=fread(Buffer, 1, 4000, file);

  if (!feof(file)) return GE_TOOLONG;
  
  i=0;j=0;
  while (true) {
    if (Buffer[j]==0x00 && Buffer[j+1]==0x02 &&
        Buffer[j+2]==0x4a && Buffer[j+3]==0x3a) break;
    if (j==i-4) return GE_INTERNALERROR;
    j++;
  }
  j++;
  
  return GSM_UnPackRingtone(ringtone, Buffer+j, i-j);
}

/* TODO: spaces should not be interpreted */
/* Note: ringtone have to be in one line (without 0x13 and 0x10 chars) */
GSM_Error loadrttl(FILE *file, GSM_Ringtone *ringtone)
{
  int NrNote=0;
  
  u8 DefNoteScale=2, DefNoteDuration=4;
  int DefNoteTempo=63;
  u8 DefNoteStyle=ContinuousStyle;

  unsigned char buffer[2000];
  unsigned char *def, *notes, *ptr;

  ringtone->Loop=15; //default value

  fread(buffer, 2000, 1, file);

  /* This is for buggy RTTTL ringtones without name. */
  if (buffer[0] != RTTTL_SEP[0]) {
    strtok(buffer, RTTTL_SEP);
    sprintf(ringtone->name, "%s", buffer);
    def=strtok(NULL, RTTTL_SEP);
    notes=strtok(NULL, RTTTL_SEP);
  }
  else {
    sprintf(ringtone->name, "GNOKII");
    def=strtok(buffer, RTTTL_SEP);
    notes=strtok(NULL, RTTTL_SEP);
  }

  ptr=strtok(def, ", ");

  /* Parsing the <defaults> section. */
  while (ptr) {

    switch(*ptr) {
    case 'd':
    case 'D':
      DefNoteDuration=GetRTTLDuration(ptr+2);
      break;
    case 'o':
    case 'O':
      DefNoteScale=GetRTTLScale(ptr+2);
      break;
    case 'b':
    case 'B':
      DefNoteTempo=atoi(ptr+2);
      break;
    case 'l':
    case 'L':
      ringtone->Loop=atoi(ptr+2);
      break;
    case 's':
    case 'S':
      switch (*(ptr+1)) {
        case 'C':
        case 'c':
	  DefNoteStyle=ContinuousStyle;
	  break;
        case 'N':
	case 'n':
	  DefNoteStyle=NaturalStyle;
	  break;
        case 'S':
	case 's':
	  DefNoteStyle=StaccatoStyle;
	  break;	
      }
      switch (*(ptr+2)) {
        case 'c':
	case 'C':
	  DefNoteStyle=ContinuousStyle;
	  break;
        case 'n':
	case 'N':
	  DefNoteStyle=NaturalStyle;
	  break;
        case 's':
	case 'S':
	  DefNoteStyle=StaccatoStyle;
	  break;	
      }
      break;
    }

    ptr=strtok(NULL,", ");
  }

#ifdef DEBUG
  printf("DefNoteDuration=%d\n", DefNoteDuration);
  printf("DefNoteScale=%d\n", DefNoteScale);
#endif

  ptr=strtok(notes, ", ");

  /* Parsing the <note-command>+ section. */
  while (ptr && NrNote<MAX_RINGTONE_NOTES) {

    switch(*ptr) {
      case 'o':
      case 'O':
        DefNoteScale=GetRTTLScale(ptr+2);
        break;
      case 's':
      case 'S':
        switch (*(ptr+1)) {
          case 'C':
          case 'c':
	    DefNoteStyle=ContinuousStyle;
	    break;
          case 'N':
	  case 'n':
	    DefNoteStyle=NaturalStyle;
	    break;
          case 'S':
	  case 's':
	    DefNoteStyle=StaccatoStyle;
	    break;	
        }
        switch (*(ptr+2)) {
          case 'C':
          case 'c':
	    DefNoteStyle=ContinuousStyle;
	    break;
          case 'N':
	  case 'n':
	    DefNoteStyle=NaturalStyle;
	    break;
          case 'S':
	  case 's':
	    DefNoteStyle=StaccatoStyle;
	    break;	
        }
        break;
      default:
        /* [<duration>] */
        ringtone->notes[NrNote].duration=GetRTTLDuration(ptr);
        if (ringtone->notes[NrNote].duration==0)
           ringtone->notes[NrNote].duration=DefNoteDuration;
	
        /* Skip all numbers in duration specification. */
        while(isdigit(*ptr))
          ptr++;

        /* <note> */
	/* B or b is not in specs, but I decided to put it, because
	   it's in some RTTL files. It's the same to H note */
	     if ((*ptr=='B') || (*ptr=='b')) ringtone->notes[NrNote].note=12;
        else if ((*ptr=='H') || (*ptr=='h')) ringtone->notes[NrNote].note=12;	     
        else if ((*ptr>='a') && (*ptr<='g')) ringtone->notes[NrNote].note=((*ptr-'a')*2)+10;
        else if ((*ptr>='A') && (*ptr<='G')) ringtone->notes[NrNote].note=((*ptr-'A')*2)+10;
        else ringtone->notes[NrNote].note=255;

        if ((ringtone->notes[NrNote].note>13)&&(ringtone->notes[NrNote].note!=255))
          ringtone->notes[NrNote].note-=14;

        ptr++;
      
        if ((*ptr)=='#') {
          ringtone->notes[NrNote].note++;
          if ((ringtone->notes[NrNote].note==5) || (ringtone->notes[NrNote].note==13))
            ringtone->notes[NrNote].note++;
          ptr++;
        }

        /* Check for dodgy rttl */
        /* [<special-duration>] */
        if (*ptr=='.') {
          ringtone->notes[NrNote].duration*=1.5;
          ptr++;
        }

        /* [<scale>] */
        if (ringtone->notes[NrNote].note!=255) {
          if (isdigit(*ptr)) {
            ringtone->notes[NrNote].note+=GetRTTLScale(ptr)*14;
            ptr++;
          } else
	    ringtone->notes[NrNote].note+=DefNoteScale*14;
        }

        /* [<special-duration>] */
        if (*ptr=='.') {
          ringtone->notes[NrNote].duration*=1.5;
          ptr++;
        }

        /* Style */
        ringtone->notes[NrNote].style=DefNoteStyle;
	
	/* Tempo */
	ringtone->notes[NrNote].tempo=DefNoteTempo;

        NrNote++;            
	
	break;
    }
    ptr=strtok(NULL, ", ");
  }

  ringtone->NrNotes=NrNote;

  return GE_NONE;
}

GSM_Error GSM_SaveRingtoneFile(char *FileName, GSM_Ringtone *ringtone)
{

  FILE *file;
  bool done=false;
  
   file = fopen(FileName, "wb");
      
   if (!file)
     return(GE_CANTOPENFILE);
   	
   if (strstr(FileName,".ott"))
   {
     saveott(file, ringtone);
     done=true;
   }
   if (strstr(FileName,".mid"))
   {
     savemid(file, ringtone);
     done=true;
   }   

   if (!done) saverttl(file, ringtone);

   fclose(file);
   
   return GE_NONE;
}

void saveott(FILE *file, GSM_Ringtone *ringtone)
{
  char Buffer[2000];
  
  int i=2000;
    
  GSM_PackRingtone(ringtone, Buffer, &i);
  
  fwrite(Buffer, 1, i, file);
}

void saverttl(FILE *file, GSM_Ringtone *ringtone)
{
  u8 DefNoteScale=2, DefNoteDuration=4;
  int DefNoteTempo=63;
  u8 DefNoteStyle=ContinuousStyle;

  int CurrentNote;
  int buffer[6];
  int i,j,k=0;
  
  /* Saves ringtone name */
  fprintf(file,_("%s:"),ringtone->name);

  /* Find the most frequently used duration and use this for the default */
 
  for (i=0;i<6;i++) buffer[i]=0;
  for (i=0;i<ringtone->NrNotes;i++) {
    switch (ringtone->notes[i].duration) {
      case 192:buffer[0]++; break;
      case 128:buffer[0]++; break;
      case  96:buffer[1]++; break;
      case  64:buffer[1]++; break;
      case  48:buffer[2]++; break;
      case  32:buffer[2]++; break;
      case  24:buffer[3]++; break;
      case  16:buffer[3]++; break;
      case  12:buffer[4]++; break;
      case   8:buffer[4]++; break;
      case   6:buffer[5]++; break;
      case   4:buffer[5]++; break;
    }
  }

  /* Now find the most frequently used */
  j=0;
  for (i=0;i<6;i++) {
    if (buffer[i]>j) {
      k=i; 
      j=buffer[i];
    }
  }

  /* Finally convert and save the default duration */

  switch (k) {
      case 0: DefNoteDuration=128; fprintf(file, _("d=1,")); break;	
      case 1: DefNoteDuration= 64; fprintf(file, _("d=2,")); break;	
      case 2: DefNoteDuration= 32; fprintf(file, _("d=4,")); break;	
      case 3: DefNoteDuration= 16; fprintf(file, _("d=8,")); break;	
      case 4: DefNoteDuration=  8; fprintf(file,_("d=16,")); break;	
      case 5: DefNoteDuration=  4; fprintf(file,_("d=32,")); break;	
     default: DefNoteDuration= 16; fprintf(file, _("d=8,")); break;	
  }  


  /* Find the most frequently used scale and use this for the default */

  for (i=0;i<6;i++) buffer[i]=0;
  for (i=0;i<ringtone->NrNotes;i++) {
    if (ringtone->notes[i].note!=255) {
      buffer[ringtone->notes[i].note/14]++;
    }
  }
  j=0;
  for (i=0;i<6;i++) {
    if (buffer[i]>j) {
      DefNoteScale=i;
      j=buffer[i];
    }
  }

  if (ringtone->NrNotes!=0) {
    DefNoteTempo=ringtone->notes[0].tempo;
    DefNoteStyle=ringtone->notes[0].style;
  }

  /* Save the default scale */
  fprintf(file,_("o=%i,"),DefNoteScale+4);  

  switch (DefNoteStyle) {
    case StaccatoStyle: fprintf(file,_("s=S,")); break;
    case NaturalStyle : fprintf(file,_("s=N,")); break;
  }

  /* Save the default tempo */
  fprintf(file,_("b=%i,"),DefNoteTempo);

  /* Save the default loop */
  fprintf(file,_("l=%i:"),ringtone->Loop);

#ifdef DEBUG
  printf("DefNoteDuration=%d\n", DefNoteDuration);
  printf("DefNoteScale=%d\n", DefNoteScale);
  printf("Number of notes=%d\n",ringtone->NrNotes);
#endif
  
  /* Now loop round for each note */

  for (i=0;i<ringtone->NrNotes;i++) {
    CurrentNote=ringtone->notes[i].note;

    if (ringtone->notes[i].style!=DefNoteStyle) {
      DefNoteStyle=ringtone->notes[i].style;
      switch (DefNoteStyle) {
        case StaccatoStyle  : fprintf(file,_("s=S")); break;
        case NaturalStyle   : fprintf(file,_("s=N")); break;
        case ContinuousStyle: fprintf(file,_("s=C")); break;
      }
      /* And a separator before next note */
      if (i!=ringtone->NrNotes-1)
        fprintf(file,_(","));
    }
    
    if (ringtone->notes[i].tempo!=DefNoteTempo) {
      DefNoteTempo=ringtone->notes[i].tempo;
      fprintf(file,_("b=%i"),DefNoteTempo);
      if (i!=ringtone->NrNotes-1)
        fprintf(file,_(","));
    }    
    
    /* This note has a duration different than the default. We must save it */
    if (ringtone->notes[i].duration!=DefNoteDuration) {
      switch (ringtone->notes[i].duration) {
        case 192: fprintf(file, _("1")); break; //192=128*1.5
        case 128: fprintf(file, _("1")); break;
        case  96: fprintf(file, _("2")); break; //96=64*1.5
        case  64: fprintf(file, _("2")); break;
        case  48: fprintf(file, _("4")); break; //48=32*1.5
        case  32: fprintf(file, _("4")); break;
        case  24: fprintf(file, _("8")); break; //24=16*1.5
        case  16: fprintf(file, _("8")); break;
        case  12: fprintf(file,_("16")); break; //12=8*1.5
        case   8: fprintf(file,_("16")); break;
        case   6: fprintf(file,_("32")); break; //6=4*1.5
        case   4: fprintf(file,_("32")); break;
        default: 
	  break;
      }
    }
    
    /* Now save the actual note */
    switch (GSM_GetNote(CurrentNote)) {
      case Note_C  :fprintf(file,_("c"));break;
      case Note_Cis:fprintf(file,_("c#"));break;
      case Note_D  :fprintf(file,_("d"));break;
      case Note_Dis:fprintf(file,_("d#"));break;
      case Note_E  :fprintf(file,_("e"));break;
      case Note_F  :fprintf(file,_("f"));break;
      case Note_Fis:fprintf(file,_("f#"));break;
      case Note_G  :fprintf(file,_("g"));break;
      case Note_Gis:fprintf(file,_("g#"));break;
      case Note_A  :fprintf(file,_("a"));break;
      case Note_Ais:fprintf(file,_("a#"));break;
      case Note_H  :fprintf(file,_("h"));break;
      default      :fprintf(file,_("p"));break; //Pause ?
    }

    /* Saving info about special duration */
    if (ringtone->notes[i].duration==128*1.5 ||
        ringtone->notes[i].duration==64*1.5 ||
        ringtone->notes[i].duration==32*1.5 ||
	ringtone->notes[i].duration==16*1.5 ||
        ringtone->notes[i].duration==8*1.5 ||
        ringtone->notes[i].duration==4*1.5)
      fprintf(file,_("."));
    
    /* This note has a scale different than the default, so save it */
    if ( (CurrentNote!=255) && (CurrentNote/14!=DefNoteScale))
        fprintf(file,_("%i"),(CurrentNote/14)+4);
    
    /* And a separator before next note */
    if (i!=ringtone->NrNotes-1)
      fprintf(file,_(","));

  }
}

void WriteVarLen(char* midifile, int* current, long value)
{
   long buffer;

   buffer = value & 0x7f;

   while (value >>= 7) {
      buffer <<= 8;
      buffer |= 0x80;
      buffer += (value & 0x7f);
   }

   while (1) {
     midifile[(*current)++] = buffer;
     if (buffer & 0x80)
       buffer >>= 8;
     else
       break;
   }
}

#define singlepauses

/* FIXME: need adding tempo before each note and scale too ? */
void savemid(FILE* file, GSM_Ringtone *ringtone)
{
  char midifile[3000] = { 0x4D, 0x54, 0x68, 0x64, // MThd
                          0x00, 0x00, 0x00, 0x06, // chunk length
                          0x00, 0x00,             // format 0
                          0x00, 0x01,             // one track
                          0x00, 0x20,             // 32 per quarter note
                          0x4D, 0x54, 0x72, 0x6B, // MTrk
                          0x00, 0x00, 0x00, 0x00, // chunk length
                          0x00, 0xFF, 0x51, 0x03, // tempo meta event
                          0x00, 0x00, 0x00        // 3 bytes for us for a quarter note
                        };

//{ "c", "c#", "d", "d#", "e",      "f", "f#", "g", "g#", "a", "a#", "h" };
char midinotes[14] =
  { 0,    1,    2,   3,    4,   4,   5,   6,    7,   8,    9,  10 ,   11,   11 };

  int length = 20;
  int start = 22;
  int current = 26, i, note, pause = 0;
  bool notesexisting = false;

  /* FIXME: we need add tempo before each note or so... */
  long duration=60000000/63;  // us for a quarter note
  if (ringtone->NrNotes!=0)
    duration=60000000/ringtone->notes[0].tempo;
  midifile[current++] = duration >> 16;
  midifile[current++] = duration >> 8;
  midifile[current++] = duration;

  for (i = 0; i < ringtone->NrNotes; i++) {

    note = ringtone->notes[i].note;
    if (note == 255) {   // readmid does not read pauses at the beginning

      if (notesexisting) {
        pause += ringtone->notes[i].duration;
#ifdef singlepauses
        WriteVarLen(midifile,&current,pause);
        pause=0;
        midifile[current++]=0x00;   // pause
        midifile[current++]=0x00;
#endif
      }
      
    } else {

      notesexisting = true;
      note = 48+12*((note/14)%4) + midinotes[note%14];

      WriteVarLen(midifile,&current,pause);
      pause=0;
      midifile[current++]=0x90;   // note on
      midifile[current++]=note;
      midifile[current++]=0x64;   // forte

      WriteVarLen(midifile,&current,ringtone->notes[i].duration);
      midifile[current++]=0x80;   // note off
      midifile[current++]=note;
      midifile[current++]=0x64; 

    }
  }

  if (pause) {
    WriteVarLen(midifile,&current,pause);
    midifile[current++]=0x00;   // pause
    midifile[current++]=0x00;   //
  }
  midifile[current++] = 0x00;
  midifile[current++] = 0xFF;   // track end
  midifile[current++] = 0x2F;
  midifile[current++] = 0x00;
  midifile[length++] = (current-start) >> 8;
  midifile[length++] = current-start;

  fwrite(midifile,1,current,file);
}

GSM_Error GSM_ReadBitmapFile(char *FileName, GSM_Bitmap *bitmap)
{

  FILE *file;
  unsigned char buffer[300];
  GSM_Error error;
  GSM_Filetypes filetype=None;

  file = fopen(FileName, "rb");

  if (!file)
    return(GE_CANTOPENFILE);

  fread(buffer, 1, 9, file); /* Read the header of the file. */

  /* Attempt to identify filetype */

  if (memcmp(buffer, "NOL",3)==0) {  /* NOL files have 'NOL' at the start */
    filetype=NOL;
  } else if (memcmp(buffer, "NGG",3)==0) {  /* NGG files have 'NGG' at the start */
    filetype=NGG;
  } else if (memcmp(buffer, "FORM",4)==0) {  /* NSL files have 'FORM' at the start */
    filetype=NSL;
  } else if (memcmp(buffer, "NLM",3)==0) {  /* NLM files have 'NLM' at the start */
    filetype=NLM;
  } else if (memcmp(buffer, "BM",2)==0) {  /* BMP, I61 and GGP files have 'BM' at the start */
    filetype=BMP;    
  } else if (memcmp(buffer, "/* XPM */",9)==0) {  /* XPM files have 'XPM' at the start */  
    filetype=XPMF;
  } else filetype=None;

  if (strstr(FileName,".otb")) filetype=OTA; /* OTA files saved by NCDS3 */
  
  error=GE_NONE;
  
  rewind(file);

  switch (filetype) {
    case NOL: error=loadnol(file,bitmap); fclose(file); break;
    case NGG: error=loadngg(file,bitmap); fclose(file); break;
    case NSL: error=loadnsl(file,bitmap); fclose(file); break;
    case NLM: error=loadnlm(file,bitmap); fclose(file); break;
    case OTA: error=loadota(file,bitmap); fclose(file); break;
    case BMP: error=loadbmp(file,bitmap); fclose(file); break;
#ifdef XPM
    case XPMF:fclose(file);error=loadxpm(FileName,bitmap);break;
#endif
    default : error=GE_INVALIDFILEFORMAT;
  }

  return(error);
}

#ifdef XPM

GSM_Error loadxpm(char *filename, GSM_Bitmap *bitmap)
{
  int y,x,error;
  XpmImage image;
  XpmInfo info;

  error=XpmReadFileToXpmImage(filename,&image,&info);

  switch (error) {
    case XpmColorError:  return GE_WRONGCOLORS;break;
    case XpmColorFailed: return GE_WRONGCOLORS;break;
    case XpmOpenFailed:  return GE_CANTOPENFILE;break;
    case XpmFileInvalid: return GE_INVALIDFILEFORMAT;break;
    case XpmSuccess: break;
  }

  if (image.ncolors!=2) {
    printf("Wrong number of colors\n");
    return GE_WRONGNUMBEROFCOLORS;
  }

  if ((image.height==48) && (image.width==84)) {
    bitmap->type=GSM_StartupLogo;
  }
  else if ((image.height==65) && (image.width==96)) {
    bitmap->type=GSM_7110StartupLogo;
  }
  else if ((image.height==60) && (image.width==96)) {
    bitmap->type=GSM_6210StartupLogo;
  }
  else if ((image.height==28) && (image.width==72)) {
    bitmap->type=GSM_PictureImage;
  }
  else if ((image.height==14) && (image.width==72)) {
    bitmap->type=GSM_CallerLogo;
  }
  else {
#ifdef DEBUG
    printf("Invalid Image Size (%dx%d).\n",image.width,image.height);
#endif
    return GE_INVALIDIMAGESIZE;
  }

  bitmap->height=image.height;
  bitmap->width=image.width;
  bitmap->size=GSM_GetBitmapSize(bitmap);

  GSM_ClearBitmap(bitmap);
  
  for(y=0;y<image.height;y++) {
    for(x=0;x<image.width;x++) {
      if (image.data[y*image.width+x]==0) GSM_SetPointBitmap(bitmap,x,y);        
    }
  }

  return GE_NONE;
}

#endif

/* Based on the article from the Polish Magazine "Bajtek" 11/92 */
                                     /* Marcin-Wiacek@Topnet.PL */
GSM_Error loadbmp(FILE *file, GSM_Bitmap *bitmap)
{
  unsigned char buffer[34];
  bool first_white;
  int w,h,pos,y,x,i,sizeimage;

  fread(buffer, 1, 34, file); //required part of header

  h=buffer[22]+256*buffer[21]; //height of image in the file
  w=buffer[18]+256*buffer[17]; //width of image in the file
#ifdef DEBUG
  printf("Image Size in BMP file: %dx%d\n",w,h);
#endif

  bitmap->type=GSM_7110StartupLogo;
  bitmap->width=96;
  bitmap->height=65;
  
  if (h==48 && w==84) {
    bitmap->width=84;
    bitmap->height=48;
    bitmap->type=GSM_StartupLogo;    
  }
  if (h==60 && w==96) {
    bitmap->width=96;
    bitmap->height=60;
    bitmap->type=GSM_6210StartupLogo;    
  }  
  if (h==14 && w==72) {
    bitmap->width=72;
    bitmap->height=14;
    bitmap->type=GSM_CallerLogo;    
  }    
  if (h==28 && w==72) {
    bitmap->width=72;
    bitmap->height=28;
    bitmap->type=GSM_PictureImage;    
  }    
  if (h==21 && w==78) {
    bitmap->width=78;
    bitmap->height=21;
    bitmap->type=GSM_7110OperatorLogo;    
  }    

  bitmap->size=GSM_GetBitmapSize(bitmap);

  GSM_ClearBitmap(bitmap);  

#ifdef DEBUG
  printf("Number of colors in BMP file: ");
  switch (buffer[28]) {
    case 1:printf("2 (supported by gnokii)\n");break;
    case 4:printf("16 (not supported by gnokii)\n");break;
    case 8:printf("256 (not supported by gnokii)\n");break;
    case 24:printf("True Color (not supported by gnokii)\n");break;
    default:printf("unknown\n");break;
  }
#endif
  if (buffer[28]!=1) {
    printf("Wrong number of colors\n"); //we support only 2 colors images !
    return GE_WRONGNUMBEROFCOLORS;
  }

#ifdef DEBUG
  printf("Compression in BMP file: ");
  switch (buffer[30]) {
    case 0:printf("no compression (supported by gnokii)\n");break;
    case 1:printf("RLE8 (not supported by gnokii)\n");break;
    case 2:printf("RLE4 (not supported by gnokii)\n");break;
    default:printf("unknown\n");break;
  }
#endif  
  if (buffer[30]!=0) {
#ifdef DEBUG
    printf("Subformat not supported\n"); //we don't support RLE compression
#endif
    return GE_SUBFORMATNOTSUPPORTED;
  }  
  
  pos=buffer[10]-34;
  fread(buffer, 1, pos, file); //rest of header (if exists) and color palette
  
#ifdef DEBUG
  printf("First color in BMP file: %i %i %i ",buffer[pos-8], buffer[pos-7], buffer[pos-6]);
  if (buffer[pos-8]==0 && buffer[pos-7]==0 && buffer[pos-6]==0) printf("(white)");
  if (buffer[pos-8]==0xFF && buffer[pos-7]==0xFF && buffer[pos-6]==0xFF) printf("(black)");
  if (buffer[pos-8]==102 && buffer[pos-7]==204 && buffer[pos-6]==102) printf("(green)");
  printf("\n");

  printf("Second color in BMP file: %i %i %i ",buffer[pos-4], buffer[pos-3], buffer[pos-2]);
  if (buffer[pos-4]==0 && buffer[pos-3]==0 && buffer[pos-2]==0) printf("(white)");
  if (buffer[pos-4]==0xFF && buffer[pos-3]==0xFF && buffer[pos-2]==0xFF) printf("(black)");
  printf("\n");  
#endif
  first_white=true;
  if (buffer[pos-8]!=0 || buffer[pos-7]!=0 || buffer[pos-6]!=0) first_white=false;
 
  sizeimage=0;
  pos=7;
  for (y=h-1;y>=0;y--) { //lines are written from the last to the first
    i=1;
    for (x=0;x<w;x++) {
      if (pos==7) { //new byte !
        fread(buffer, 1, 1, file);
	sizeimage++;
	i++;
	if(i==5) i=1; //each line is written in multiply of 4 bytes
      }
      if (x<=bitmap->width && y<=bitmap->height) { //we have top left corner !
        if (first_white) {
          if ((buffer[0]&(1<<pos))<=0) GSM_SetPointBitmap(bitmap,x,y);
	} else {
	  if ((buffer[0]&(1<<pos))>0) GSM_SetPointBitmap(bitmap,x,y);
	}
      }
      pos--;
      if (pos<0) pos=7; //going to new byte
    }
    pos=7; //going to new byte
    if (i!=1) {
      while (i!=5) //each line is written in multiply of 4 bytes
      {
        fread(buffer, 1, 1, file);
        sizeimage++;
        i++;
      }
    }
  }

#ifdef DEBUG
  printf("Data size in BMP file: %i\n",sizeimage);
#endif
    
  return(GE_NONE);
}

GSM_Error loadnol(FILE *file, GSM_Bitmap *bitmap)
{

  unsigned char buffer[2000];
  int i,j;
  
  bitmap->type=GSM_OperatorLogo;

  fread(buffer, 1, 6, file);
  fread(buffer, 1, 4, file);
  sprintf(bitmap->netcode, "%d %02d", buffer[0]+256*buffer[1], buffer[2]);

  fread(buffer, 1, 4, file); /* Width and height of the icon. */
  bitmap->width=buffer[0];
  bitmap->height=buffer[2];
  bitmap->size=GSM_GetBitmapSize(bitmap);

  if ((bitmap->height!=14) || (bitmap->width!=72)) {
#ifdef DEBUG
    printf("Invalid Image Size (%dx%d).\n",bitmap->width,bitmap->height);
#endif
    return GE_INVALIDIMAGESIZE;
  }

  fread(buffer, 1, 6, file); /* Unknown bytes. */
  
  for (i=0; i<bitmap->size; i++) {
    if (fread(buffer, 1, 8, file)==8) {
      bitmap->bitmap[i]=0;
      for (j=7; j>=0;j--)
        if (buffer[7-j] == '1')
	  bitmap->bitmap[i]|=(1<<j);
    }
    else
      return (GE_TOOSHORT);
  }

#ifdef DEBUG
  /* Some programs writes here fileinfo */
  if (fread(buffer, 1, 1, file)==1) {
    fprintf(stdout, _("Fileinfo: %c"),buffer[0]);
    while (fread(buffer, 1, 1, file)==1) {
      if (buffer[0]!=0x0A) fprintf(stdout,_("%c"),buffer[0]);
    }  
    fprintf(stdout, _("\n"));
  }
#endif

  return(GE_NONE);
}

GSM_Error loadngg(FILE *file, GSM_Bitmap *bitmap)
{

  unsigned char buffer[2000];
  int i,j;

  bitmap->type=GSM_CallerLogo;

  fread(buffer, 1, 6, file);
  fread(buffer, 1, 4, file); /* Width and height of the icon. */
  bitmap->width=buffer[0];
  bitmap->height=buffer[2];
  bitmap->size=GSM_GetBitmapSize(bitmap);
  
  if ((bitmap->height!=14) || (bitmap->width!=72)) {
#ifdef DEBUG
    printf("Invalid Image Size (%dx%d).\n",bitmap->width,bitmap->height);
#endif
    return GE_INVALIDIMAGESIZE;
  }
  
  fread(buffer, 1, 6, file); /* Unknown bytes. */
    
  for (i=0; i<bitmap->size; i++) {
    if (fread(buffer, 1, 8, file)==8){
      bitmap->bitmap[i]=0;
      for (j=7; j>=0;j--)
	if (buffer[7-j] == '1')
	  bitmap->bitmap[i]|=(1<<j);
    }
    else
      return(GE_TOOSHORT);
  }

#ifdef DEBUG
  /* Some programs writes here fileinfo */
  if (fread(buffer, 1, 1, file)==1) {
    fprintf(stdout, _("Fileinfo: %c"),buffer[0]);
    while (fread(buffer, 1, 1, file)==1) {
      if (buffer[0]!=0x0A) fprintf(stdout,_("%c"),buffer[0]);
    }  
    fprintf(stdout, _("\n"));
  }
#endif
  
  return(GE_NONE);
}

GSM_Error loadnsl(FILE *file, GSM_Bitmap *bitmap)
{

  unsigned char block[6],buffer[505];
  int block_size;

  bitmap->size=0;
  
  while (fread(block,1,6,file)==6) {

    block_size=block[4]*256+block[5];

#ifdef DEBUG
    fprintf(stdout,_("Block %c%c%c%c, size %i\n"),block[0],block[1],block[2],block[3],block_size);
#endif

    if (!strncmp(block, "FORM", 4)) {
#ifdef DEBUG
      fprintf(stdout,_("  File ID\n"));
#endif
    } else
    {
      if (block_size>504) return(GE_INVALIDFILEFORMAT);

      if (block_size!=0) {

        fread(buffer,1,block_size,file);
        buffer[block_size]=0; //if it's string, we end it with \0

#ifdef DEBUG
        if (!strncmp(block, "VERS", 4)) fprintf(stdout,_("  File saved by: %s\n"),buffer);
        if (!strncmp(block, "MODL", 4)) fprintf(stdout,_("  Logo saved from: %s\n"),buffer);
        if (!strncmp(block, "COMM", 4)) fprintf(stdout,_("  Phone was connected to COM port: %s\n"),buffer);
#endif
	
        if (!strncmp(block, "NSLD", 4)) {          
          bitmap->type=GSM_StartupLogo;
          bitmap->height=48;
          bitmap->width=84;
          bitmap->size=GSM_GetBitmapSize(bitmap);

          memcpy(bitmap->bitmap,buffer,bitmap->size);

#ifdef DEBUG
          fprintf(stdout,_("  Startup logo (size %i)\n"),block_size);
#endif
        }
      }
    }
  }
  
  if (bitmap->size==0) return(GE_TOOSHORT);

  return(GE_NONE);
}

GSM_Error loadnlm (FILE *file, GSM_Bitmap *bitmap)
{
  unsigned char buffer[1000];
  int pos,pos2,x,y;
  div_t division;

  fread(buffer,1,5,file);
  fread(buffer,1,1,file);

  switch (buffer[0]) {
  case 0x00: bitmap->type=GSM_OperatorLogo; break;
  case 0x01: bitmap->type=GSM_CallerLogo;   break;
  case 0x02: bitmap->type=GSM_StartupLogo;  break;
  case 0x03: bitmap->type=GSM_PictureImage; break;
  default:
    return(GE_SUBFORMATNOTSUPPORTED);
  }
  
  fread(buffer,1,4,file);
  bitmap->width=buffer[1];
  bitmap->height=buffer[2];

  if (bitmap->type==GSM_StartupLogo  && bitmap->width==96 && bitmap->height==65)
    bitmap->type=GSM_7110StartupLogo;
  if (bitmap->type==GSM_StartupLogo  && bitmap->width==96 && bitmap->height==60)
    bitmap->type=GSM_6210StartupLogo;
  if (bitmap->type==GSM_OperatorLogo && bitmap->width==78 && bitmap->height==21)
    bitmap->type=GSM_7110OperatorLogo;

  bitmap->size=GSM_GetBitmapSize(bitmap);

  division=div(bitmap->width,8);
  if (division.rem!=0) division.quot++; /* For startup logos */
  
  if (fread(buffer,1,(division.quot*bitmap->height),file)!=(division.quot*bitmap->height))
    return(GE_TOOSHORT);
    
  GSM_ClearBitmap(bitmap);
  
  pos=0;pos2=7;
  for (y=0;y<bitmap->height;y++) {
    for (x=0;x<bitmap->width;x++) {
      if ((buffer[pos]&(1<<pos2))>0) GSM_SetPointBitmap(bitmap,x,y);
      pos2--;
      if (pos2<0) {pos2=7;pos++;} //going to new byte
    }
    if (pos2!=7) {pos2=7;pos++;} //for startup logos-new line means new byte
  }

  return (GE_NONE);
}

GSM_Error loadota(FILE *file, GSM_Bitmap *bitmap)
{

  char buffer[4];

  fread(buffer,1,4,file);

  bitmap->width=buffer[1];
  bitmap->height=buffer[2];

  if ((bitmap->height==48) && (bitmap->width==84)) {
    bitmap->type=GSM_StartupLogo;
  }
  else if ((bitmap->height==14) && (bitmap->width==72)) {
    bitmap->type=GSM_CallerLogo;
  }
  else {
#ifdef DEBUG
    printf("Invalid Image Size (%dx%d).\n",bitmap->width,bitmap->height);
#endif
    return GE_INVALIDIMAGESIZE;
  }

  bitmap->size=GSM_GetBitmapSize(bitmap);
  
  if (fread(bitmap->bitmap,1,bitmap->size,file)!=bitmap->size)
    return(GE_TOOSHORT);

  return(GE_NONE);
}

GSM_Error GSM_SaveBitmapFile(char *FileName, GSM_Bitmap *bitmap)
{

  FILE *file;
  bool done=false;
     
   file = fopen(FileName, "wb");
      
   if (!file)
     return(GE_CANTOPENFILE);

   if (strstr(FileName,".xpm")) { savexpm(file, bitmap); done=true; }	
   if (strstr(FileName,".nlm")) { savenlm(file, bitmap); done=true; }
   if (strstr(FileName,".ngg")) { savengg(file, bitmap); done=true; }
   if (strstr(FileName,".nsl")) { savensl(file, bitmap); done=true; }
   if (strstr(FileName,".otb")) { saveota(file, bitmap); done=true; }
   if (strstr(FileName,".nol")) { savenol(file, bitmap); done=true; }
   if (strstr(FileName,".bmp") ||
       strstr(FileName,".ggp") ||
       strstr(FileName,".i61"))
   {
     savebmp(file, bitmap);
     done=true;
   }
   
   if (!done)
   {
     switch (bitmap->type) {
       case GSM_CallerLogo      : savengg(file, bitmap); break;
       case GSM_OperatorLogo    : savenol(file, bitmap); break;
       case GSM_7110OperatorLogo: savebmp(file, bitmap); break;
       case GSM_7110StartupLogo : savebmp(file, bitmap); break;
       case GSM_6210StartupLogo : savebmp(file, bitmap); break;
       case GSM_StartupLogo     : savensl(file, bitmap); break;
       case GSM_PictureImage    : savenlm(file, bitmap); break;
       case GSM_WelcomeNoteText :                        break;
       case GSM_DealerNoteText  :                        break;
       case GSM_None            :                        break;
     }      
   }
  
   fclose(file);
   
   return GE_NONE;
}

void savexpm(FILE *file, GSM_Bitmap *bitmap)
{
  int x,y;

  fprintf(file,_("/* XPM */\n"));
  fprintf(file,_("static char * ala_xpm[] = {\n"));
  fprintf(file,_("\"%i %i 2 1\",\n"),bitmap->width,bitmap->height);
  fprintf(file,_("\".	s c	m #000000	g4 #000000	g #000000	c #000000\",\n"));
  fprintf(file,_("\"#	s c	m #ffffff	g4 #ffffff	g #ffffff	c #ffffff\",\n"));

  for (y=0;y<bitmap->height;y++) {
    fprintf(file,_("\""));
    for (x=0;x<bitmap->width;x++)
      if (GSM_IsPointBitmap(bitmap,x,y))
        fprintf(file,_("."));
      else
        fprintf(file,_("#"));
    fprintf(file,_("\""));
    if (y==bitmap->height-1)
      fprintf(file,_("};\n"));
    else
      fprintf(file,_(",\n"));
  }
}

/* Based on the article from the Polish Magazine "Bajtek" 11/92 */
                                     /* Marcin-Wiacek@Topnet.PL */
void savebmp(FILE *file, GSM_Bitmap *bitmap)
{
  int x,y,pos,i,sizeimage;
  unsigned char buffer[1];
  div_t division;
  
  unsigned char header[]={
/*1'st header*/   'B','M',             /* BMP file ID */
                  0x00,0x00,0x00,0x00, /* Size of file */
		  0x00,0x00,           /* Reserved for future use */
		  0x00,0x00,           /* Reserved for future use */
	            62,0x00,0x00,0x00, /* Offset for image data */
		 
/*2'nd header*/     40,0x00,0x00,0x00, /* Length of this part of header */
		  0x00,0x00,0x00,0x00, /* Width of image */
		  0x00,0x00,0x00,0x00, /* Height of image */		 
		     1,0x00,           /* How many planes in target device */
		     1,0x00,           /* How many colors in image. 1 means 2^1=2 colors */
		  0x00,0x00,0x00,0x00, /* Type of compression. 0 means no compression */
/*Sometimes */    0x00,0x00,0x00,0x00, /* Size of part with image data */
/*ttttttt...*/    0xE8,0x03,0x00,0x00, /* XPelsPerMeter */
/*hhiiiiissss*/   0xE8,0x03,0x00,0x00, /* YPelsPerMeter */		  
/*part of header*/2,0x00,0x00,0x00, /* How many colors from palette is used */
/*doesn't exist*/ 0x00,0x00,0x00,0x00, /* How many colors from palette is required to display image. 0 means all */
		 
/*Color palette*/ 0x00,0x00,0x00,      /* First color in palette in Blue, Green, Red. Here white */
		  0x00,                /* Each color in palette is end by 4'th byte */
                  102,204,102,      /* Second color in palette in Blue, Green, Red. Here green */
		  0x00};               /* Each color in palette is end by 4'th byte */

  header[22]=bitmap->height;
  header[18]=bitmap->width;
     
  pos=7;
  sizeimage=0;
  for (y=bitmap->height-1;y>=0;y--) { //lines are written from the last to the first
    i=1;
    for (x=0;x<bitmap->width;x++) {
      if (pos==7) { //new byte !
        if (x!=0) sizeimage++;
	i++;
	if(i==5) i=1; //each line is written in multiply of 4 bytes
      }
      pos--;
      if (pos<0) pos=7; //going to new byte
    }
    pos=7; //going to new byte
    sizeimage++;
    if (i!=1) {
      while (i!=5) //each line is written in multiply of 4 bytes
      {
        sizeimage++;
        i++;
      }
    }
  }
#ifdef DEBUG
  printf("Data size in BMP file: %i\n",sizeimage);
#endif
  division=div(sizeimage,256);
  header[35]=division.quot;
  header[34]=sizeimage-(division.quot*256);
  
  sizeimage=sizeimage+sizeof(header);
#ifdef DEBUG
  printf("Size of BMP file: %i\n",sizeimage);
#endif
  division=div(sizeimage,256);
  header[3]=division.quot;
  header[2]=sizeimage-(division.quot*256);
       
  fwrite(header,1,sizeof(header),file);

  pos=7;
  for (y=bitmap->height-1;y>=0;y--) { //lines are written from the last to the first
    i=1;
    for (x=0;x<bitmap->width;x++) {
      if (pos==7) { //new byte !
        if (x!=0) fwrite(buffer, 1, sizeof(buffer), file);
	i++;
	if(i==5) i=1; //each line is written in multiply of 4 bytes
	buffer[0]=0;
      }
      if (!GSM_IsPointBitmap(bitmap,x,y)) buffer[0]|=(1<<pos);
      pos--;
      if (pos<0) pos=7; //going to new byte
    }
    pos=7; //going to new byte
    fwrite(buffer, 1, sizeof(buffer), file);
    if (i!=1) {
      while (i!=5) //each line is written in multiply of 4 bytes
      {
        buffer[0]=0;
        fwrite(buffer, 1, sizeof(buffer), file);
        i++;
      }
    }
  }
}

void savengg(FILE *file, GSM_Bitmap *bitmap)
{

  char header[]={'N','G','G',0x00,0x01,0x00,
                 0x00,0x00,           /* Width */
		 0x00,0x00,           /* Height */
		 0x01,0x00,0x01,0x00,
		 0x00,                /* Unknown.Can't be checksum - for */
		                      /* the same logo files can be different */
		 0x00};  

  char buffer[8];
  int i,j;
  GSM_Bitmap copy;
  
  copy=*bitmap;
  
  GSM_ResizeBitmap(&copy,GSM_CallerLogo);
  
  header[6]=copy.width;
  header[8]=copy.height;

  fwrite(header,1,sizeof(header),file);

  for (i=0; i<copy.size; i++) {
    for (j=7; j>=0;j--)
      if ((copy.bitmap[i]&(1<<j))>0) {
	buffer[7-j] = '1';
      } else {
	buffer[7-j] = '0';
      }
    fwrite(buffer,1,8,file);
  }
}
  
void savenol(FILE *file, GSM_Bitmap *bitmap)
{

  char header[]={'N','O','L',0x00,0x01,0x00,
                 0x00,0x00,           /* MCC */
		 0x00,0x00,           /* MNC */
		 0x00,0x00,           /* Width */
		 0x00,0x00,           /* Height */
		 0x01,0x00,0x01,0x00,
		 0x00,                /* Unknown.Can't be checksum - for */
		                      /* the same logo files can be different */
		 0x00};
  char buffer[8];
  int i,j,country,net;
  GSM_Bitmap copy;
  
  copy=*bitmap;
  
  GSM_ResizeBitmap(&copy,GSM_OperatorLogo);
  
  sscanf(copy.netcode, "%d %d", &country, &net);

  header[6]=country%256;
  header[7]=country/256;
  header[8]=net%256;
  header[9]=net/256;
  header[10]=copy.width;
  header[12]=copy.height;

  fwrite(header,1,sizeof(header),file);
  
  for (i=0; i<copy.size; i++) {
    for (j=7; j>=0;j--)
      if ((copy.bitmap[i]&(1<<j))>0) {
	buffer[7-j] = '1';
      } else {
	buffer[7-j] = '0';
      }
    fwrite(buffer,1,8,file);
  }
}

void savensl(FILE *file, GSM_Bitmap *bitmap)
{

  u8 header[]={'F','O','R','M', 0x01,0xFE,  /* File ID block,      size 1*256+0xFE=510*/
	       'N','S','L','D', 0x01,0xF8}; /* Startup Logo block, size 1*256+0xF8=504*/
  GSM_Bitmap copy;
  
  copy=*bitmap;
  
  GSM_ResizeBitmap(&copy,GSM_StartupLogo);
  
  fwrite(header,1,sizeof(header),file);

  fwrite(copy.bitmap,1,copy.size,file);
}

void saveota(FILE *file, GSM_Bitmap *bitmap)
{

  char header[]={0x01,
                 0x00, /* Width */
		 0x00, /* Height */
		 0x01};
  GSM_Bitmap copy;
  
  copy=*bitmap;
  
  header[1]=copy.width;
  header[2]=copy.height;
    
  fwrite(header,1,sizeof(header),file);

  fwrite(copy.bitmap,1,copy.size,file);
}

void savenlm(FILE *file, GSM_Bitmap *bitmap)
{

  char header[]={'N','L','M', /* Nokia Logo Manager file ID. */
                 0x20,
                 0x01,
                 0x00,        /* 0x00 (OP), 0x01 (CLI), 0x02 (Startup), 0x03 (Picture)*/
                 0x00,        /* Number of images inside file - 1. 0x01==2 images, 0x03==4 images, etc. */
                 0x00,        /* Width. */
                 0x00,        /* Height. */
                 0x01};
		 
  unsigned char buffer[1000];
  int x,y,pos,pos2;
  div_t division;
  GSM_Bitmap copy;
  
  copy=*bitmap;
  
  switch (copy.type) {
  case GSM_OperatorLogo    : header[5]=0x00; break;
  case GSM_7110OperatorLogo: header[5]=0x00; break;
  case GSM_CallerLogo      : header[5]=0x01; break;
  case GSM_StartupLogo     : header[5]=0x02; break;
  case GSM_7110StartupLogo : header[5]=0x02; break;
  case GSM_6210StartupLogo : header[5]=0x02; break;
  case GSM_PictureImage    : header[5]=0x03; break;
  case GSM_WelcomeNoteText :                 break;
  case GSM_DealerNoteText  :                 break;
  case GSM_None            :                 break;
  }
  
  header[7]=copy.width;
  header[8]=copy.height;
  
  pos=0;pos2=7;
  for (y=0;y<copy.height;y++) {
    for (x=0;x<copy.width;x++) {
      if (pos2==7) buffer[pos]=0;
      
      if (GSM_IsPointBitmap(&copy,x,y)) buffer[pos]|=(1<<pos2);
      
      pos2--;
      if (pos2<0) {pos2=7;pos++;} //going to new line
    }
    if (pos2!=7) {pos2=7;pos++;} //for startup logos - new line with new byte
  }
  
  division=div(copy.width,8);
  if (division.rem!=0) division.quot++; /* For startup logos */
  
  fwrite(header,1,sizeof(header),file);

  fwrite(buffer,1,(division.quot*copy.height),file);
}

/* mode == 0 -> overwrite
 * mode == 1 -> ask
 * mode == 2 -> append
 */
int GSM_SaveTextFile(char *FileName, char *text, int mode)
{

  FILE *file;

  if (mode == 2) file = fopen(FileName, "a");
            else file = fopen(FileName, "w");

  if (!file) return -1;
  
  fprintf(file, "%s\n\n", text);
  
  fclose(file);

  return mode;
}

GSM_Error GSM_SaveBackupFile(char *FileName, GSM_Backup *backup)
{

  FILE *file;
     
  file = fopen(FileName, "wb");
      
  if (!file) return(GE_CANTOPENFILE);

  savelmb(file, backup);

  fclose(file);
   
  return GE_NONE;
}

void savelmbstartupentry(FILE *file, GSM_Bitmap startup, GSM_Bitmap text, bool available)
{  
  /* Welcome note and logo header block */
  char req[1000] = {'W','E','L',' ',    /*block identifier*/
                    00,00,              /*block data size*/
	            0x02,00,00,00,00,00,

                    0x02};              /*number of blocks (like in 6110 frame)*/

  int count=13;

  if (!available) {
  } else {
    count=count+N6110_MakeStartupLogoFrame(req+13,startup);
  }

  req[count++]=0x02;
  req[count++]=strlen(text.text);
  memcpy(req+count,text.text,strlen(text.text));
  count=count+strlen(text.text);

  req[4]=(count-12)%256;
  req[5]=(count-12)/256;

  fwrite(req, 1, count, file);
}		     

void savelmbopentry(FILE *file, GSM_Bitmap bitmap)
{  
  /* Operator logo header block */
  char req[500] = {'O','L','G',' ',        /*block identifier*/
                   0x88,00,                /*block data size*/
		   0x02,00,00,00,00,00,

                   00};

  int count=13,i;

  count=count+N6110_MakeOperatorLogoFrame(req+13,bitmap);

  if (bitmap.width!=0x48) {
    req[4]=(count-7)%256;
    req[5]=(count-7)/256;

    for (i=1;i<=5;i++) req[count++]=0;

    req[17]=req[17]+5; //we fix size of logo block
  } else {
    req[4]=(count-12)%256;
    req[5]=(count-12)/256;
  }

  fwrite(req, 1, count, file);
}		     

/* Work in progress ! */
void savelmbspeedentry(FILE *file, GSM_SpeedDial speed)
{  
  /* Speed dial header block */	   
  char req[] = {'S','P','D',' ', /*block identifier*/
                0x03,00,         /*block data size*/
                0x02,00,
		00,              /*number of speed dial*/
		00,0xFF,00,

                00,              /*number of speed dial*/
                03,              /*memory type. ME=02;SM=03*/
                00};             /*number of location assigned to speed dial*/

  req[8]=req[12]=speed.Number;

  if (speed.MemoryType==GMT_ME) req[13]=2; //memory type=GMT_ME    

  req[14]=speed.Location;

  fwrite(req, 1, 15, file);
}		     

void savelmbcallerentry(FILE *file, GSM_Bitmap bitmap)
{  
  char req[500] = {'C','G','R',' ',    /*block identifier*/
                   00,00,              /*block data size*/
                   02,00,              
		   00,                 /*group number=0,1,etc.*/
		   00,00,00};

  int count=12;

  req[8]=bitmap.number;

  count=count+N6110_MakeCallerGroupFrame(req+12,bitmap);

  req[count++]=0;

  req[4]=(count-12)%256;
  req[5]=(count-12)/256;

  fwrite(req, 1, count, file);
}		     

void savelmbpbkentry(FILE *file, GSM_PhonebookEntry entry)
{
  char req[500] = {'P','B','E','2', /*block identifier*/
                   00,00,           /*block data size*/
		   00,00,           
		   00,00,           /*position of phonebook entry*/		                 
		   03,              /*memory type. ME=02;SM=03*/
		   00,

                   00,00,           /*position of phonebook entry*/                   
                   03,              /*memory type. ME=02;SM=03*/
                   00};

  int count = 16, blocks;

  req[9]=req[13] = (entry.Location >> 8);
  req[8]=req[12] = entry.Location & 0xff;

  if (entry.MemoryType==GMT_ME) req[10]=req[14]=2;

  count=count+N7110_EncodePhonebookFrame(req+16, entry, &blocks);

  req[4]=(count-12)%256;
  req[5]=(count-12)/256;
            
  fwrite(req, 1, count, file);	    
}

void savelmb(FILE *file, GSM_Backup *backup)
{
  int i;

  char LMBHeader[] = {'L','M','B',' '}; /*file identifier*/
    
  /* Phonebook header block */
  char PBKHeader[] = {'P','B','K',' ', /*block identifier*/
                      0x08,00,         /*block data size*/
		      0x02,00,         
		      03,              /*memory type. ME=02;SM=03*/
		      00,00,00,

                      00,00,           /*size of phonebook*/
                      14,              /*max length of each position*/
		      00,00,00,00,00};
		    		      
  fwrite(LMBHeader, 1, sizeof(LMBHeader), file); /* Write the header of the file. */

  if (backup->SIMPhonebookUsed!=0) {
    PBKHeader[12]=backup->SIMPhonebookSize%256;
    PBKHeader[13]=backup->SIMPhonebookSize/256;
    fwrite(PBKHeader, 1, sizeof(PBKHeader), file); 

    for (i=0;i<backup->SIMPhonebookUsed;i++)
      savelmbpbkentry(file, backup->SIMPhonebook[i]);
  }

  if (backup->PhonePhonebookUsed!=0) {
    PBKHeader[8]=2;     //memory type=GMT_ME
    PBKHeader[12]=backup->PhonePhonebookSize%256;
    PBKHeader[13]=backup->PhonePhonebookSize/256;
    PBKHeader[14]=0x16; //max size of one entry
    fwrite(PBKHeader, 1, sizeof(PBKHeader), file); 

    for (i=0;i<backup->PhonePhonebookUsed;i++)
      savelmbpbkentry(file, backup->PhonePhonebook[i]);
  }

  if (backup->CallerAvailable)
    for (i=0;i<5;i++) savelmbcallerentry(file,backup->CallerGroups[i]);

//  if (backup->SpeedAvailable)
//    for (i=0;i<8;i++) savelmbspeedentry(file,backup->SpeedDials[i]);

  if (backup->OperatorLogoAvailable) savelmbopentry(file,backup->OperatorLogo);

  savelmbstartupentry(file,backup->StartupLogo,backup->StartupText,backup->StartupLogoAvailable);
}

void loadlmbcallerentry(unsigned char *buffer, unsigned char *buffer2, GSM_Backup *backup, int number)
{ 
  int i;

  backup->CallerAvailable=true;

#ifdef DEBUG
  fprintf(stdout, _("  Number %i, name \""), buffer2[0]);
  for (i=0;i<buffer2[1];i++) {
    fprintf(stdout, _("%c"), buffer2[i+2]);
  }
  fprintf(stdout, _("\"\n"));

  fprintf(stdout,_("  Ringtone ID=%i\n"), buffer2[i+2]);

  if (buffer2[i+3]==1) fprintf(stdout,_("  Logo enabled\n"));
                  else fprintf(stdout,_("  Logo disabled\n"));	
#endif

  backup->CallerGroups[number].number=buffer2[0];
  backup->CallerGroups[number].type=GSM_CallerLogo;

  for (i=0;i<buffer2[1];i++) {
    backup->CallerGroups[number].text[i]=buffer2[i+2];
  }
  backup->CallerGroups[number].text[buffer2[1]]=0;
	
  backup->CallerGroups[number].ringtone=buffer2[i+2];
	
  backup->CallerGroups[number].enabled=false;
  if (buffer2[i+3]==1) backup->CallerGroups[number].enabled=true;

  backup->CallerGroups[number].width=buffer2[i+7];
  backup->CallerGroups[number].height=buffer2[i+8];
		
  backup->CallerGroups[number].size=GSM_GetBitmapSize(&backup->CallerGroups[number]);
      
  memcpy(backup->CallerGroups[number].bitmap,buffer2+i+10,backup->CallerGroups[number].size);

#ifdef DEBUG
  fprintf(stdout, _("  Caller logo"));
  fprintf(stdout, _(" (size %ix%i - %i bytes)\n"),
     backup->CallerGroups[number].width, backup->CallerGroups[number].height, backup->CallerGroups[number].size);
  GSM_PrintBitmap(&backup->CallerGroups[number]);
#endif

}		     

void loadlmbopentry(unsigned char *buffer, unsigned char *buffer2, GSM_Backup *backup)
{  
  backup->OperatorLogoAvailable=true;

  DecodeNetworkCode(buffer2+1, backup->OperatorLogo.netcode);

  backup->OperatorLogo.text[0]=0;

  backup->OperatorLogo.width=buffer2[7];
  backup->OperatorLogo.height=buffer2[8];

  backup->OperatorLogo.type=GSM_OperatorLogo;      
  if (backup->OperatorLogo.width==78) backup->OperatorLogo.type=GSM_7110OperatorLogo;      

  backup->OperatorLogo.size=GSM_GetBitmapSize(&backup->OperatorLogo);      

#ifdef DEBUG
  fprintf(stdout, _("  GSM operator logo (size %ix%i - %i bytes) for %s (%s) network.\n"),
       backup->OperatorLogo.width, backup->OperatorLogo.height,
       backup->OperatorLogo.size, backup->OperatorLogo.netcode,
       GSM_GetNetworkName(backup->OperatorLogo.netcode));
#endif

  memcpy(backup->OperatorLogo.bitmap,buffer2+10,backup->OperatorLogo.size);

#ifdef DEBUG
  GSM_PrintBitmap(&backup->OperatorLogo);
#endif

}		     

void loadlmbpbkentry(unsigned char *buffer, unsigned char *buffer2, GSM_Backup *backup)
{
  GSM_PhonebookEntry pbk;

#ifdef DEBUG
  fprintf(stdout,_("  Memory : "));
  switch(buffer[10]) {
    case 2: fprintf(stdout, _("(internal)\n"));break;
    case 3: fprintf(stdout, _("(sim)\n"));break;
    default: fprintf(stdout, _("(unknown)\n"));break;
  }
  fprintf(stdout,_("  Location : %i\n"),buffer2[0]+buffer2[1]*256);
#endif

  pbk.Empty = true;
  pbk.Group = 5;     /* 5 = no group as 6110 */
  pbk.Name[0] = 0;
  pbk.Number[0] = 0;
  pbk.SubEntriesCount = 0;

  N7110_DecodePhonebookFrame(&pbk,buffer2+4,(buffer[4]+buffer[5]*256)-4);

  pbk.MemoryType=GMT_SM;
  if (buffer[10]==2) pbk.MemoryType=GMT_ME;

  pbk.Location=buffer2[0]+256*buffer2[1];

  if (buffer[10]==2) backup->PhonePhonebook[backup->PhonePhonebookUsed++]=pbk;
                else backup->SIMPhonebook  [backup->SIMPhonebookUsed++]  =pbk;
}

void loadlmbstartupentry(unsigned char *buffer, unsigned char *buffer2, GSM_Backup *backup)
{
  int i,j;
#ifdef DEBUG
  int z;
#endif

  j=1;
  for (i=0;i<buffer2[0];i++) {
    switch (buffer2[j++]) {
      case 1:
        backup->StartupLogoAvailable=true;
	backup->StartupLogo.height=buffer2[j++];
	backup->StartupLogo.width=buffer2[j++];
	backup->StartupLogo.text[0]=0;
	backup->StartupLogo.type=GSM_StartupLogo;
        switch (backup->StartupLogo.height) {
          case 65:backup->StartupLogo.type=GSM_7110StartupLogo;break;
          case 60:backup->StartupLogo.type=GSM_6210StartupLogo;break;
        }
        backup->StartupLogo.size=GSM_GetBitmapSize(&backup->StartupLogo);
	    
#ifdef DEBUG
        fprintf(stdout, _("  Block 1 - startup logo (size %ix%i - %i bytes)\n"),
              backup->StartupLogo.width, backup->StartupLogo.height, backup->StartupLogo.size);
#endif

        memcpy(backup->StartupLogo.bitmap,buffer2+j,backup->StartupLogo.size);
#ifdef DEBUG
        GSM_PrintBitmap(&backup->StartupLogo);
#endif
	j=j+backup->StartupLogo.size;

	break;            
      case 2:

#ifdef DEBUG
        fprintf(stdout, _("  Block 2 - welcome note \""));
        for (z=0;z<buffer2[j];z++) fprintf(stdout, _("%c"),buffer2[j+z+1]);
	fprintf(stdout, _("\"\n"));
#endif
        break;
      default:
#ifdef DEBUG
        fprintf(stdout, _("  Unknown block %02x\n"),buffer2[j]);
#endif
	break;
    }
  }
}

GSM_Error loadlmb(FILE *file, GSM_Backup *backup)
{
#ifdef DEBUG
  int i;
#endif
  int number=0;

  unsigned char buffer[12], buffer2[1000]; //for data from file

  fread(buffer, 1, 4, file); /* Read the header of the file. */

  while (fread(buffer, 1, 12, file)==12) { //while we have something to read

#ifdef DEBUG
    /* Info about block in the file */
    fprintf(stdout, _("Block \""));
    for (i=0;i<4;i++) {fprintf(stdout, _("%c"),buffer[i]);}
    fprintf(stdout, _("\" ("));
    if (memcmp(buffer, "PBK ",4)==0) {fprintf(stdout, _("Phonebook"));      } else {
    if (memcmp(buffer, "PBE2",4)==0) {fprintf(stdout, _("Phonebook entry"));} else {
    if (memcmp(buffer, "CGR ",4)==0) {fprintf(stdout, _("Caller group"));   } else {      
    if (memcmp(buffer, "SPD ",4)==0) {fprintf(stdout, _("Speed dial"));     } else {      
    if (memcmp(buffer, "OLG ",4)==0) {fprintf(stdout, _("Operator logo"));  } else {      
    if (memcmp(buffer, "WEL ",4)==0) {fprintf(stdout, _("Startup logo and welcome text")); } else {      
                                      fprintf(stdout, _("uknown - ignored"));}}}}}}

    fprintf(stdout, _(") - length %i\n"), buffer[4]+buffer[5]*256);
#endif
      
    fread(buffer2, 1, buffer[4]+buffer[5]*256, file); //reading block data

#ifdef DEBUG
    if (memcmp(buffer, "PBK ",4)==0) { //phonebook
      fprintf(stdout, _("  Size of phonebook %i, type %i "),(buffer2[0]+buffer2[1]*256),buffer[8]);
      switch(buffer[8]) {
	case 2 : fprintf(stdout, _("(internal)"));break;
	case 3 : fprintf(stdout, _("(sim)"))     ;break;
	default: fprintf(stdout, _("(unknown)")) ;break;
      }
      fprintf(stdout, _(", length of each position - %i\n"),buffer2[2]);
    }
#endif        

    if (memcmp(buffer, "PBE2",4)==0) //phonebook entry        
      loadlmbpbkentry(buffer,buffer2,backup);

    if (memcmp(buffer, "CGR ",4)==0) { //caller groups
      loadlmbcallerentry(buffer,buffer2,backup,number);
      number++;
    }

    if (memcmp(buffer, "OLG ",4)==0) //operator logo
      loadlmbopentry(buffer,buffer2,backup);

    if (memcmp(buffer, "WEL ",4)==0) //welcome blocks
      loadlmbstartupentry(buffer,buffer2,backup);
  }

  return(GE_NONE);
}

GSM_Error GSM_ReadBackupFile(char *FileName, GSM_Backup *backup)
{

  FILE *file;
  unsigned char buffer[300];
  GSM_Error error;
  GSM_Filetypes filetype=None;

  file = fopen(FileName, "rb");

  if (!file) return(GE_CANTOPENFILE);

  fread(buffer, 1, 4, file); /* Read the header of the file. */

  /* Attempt to identify filetype */

  if (memcmp(buffer, "LMB ",4)==0) {  /* LMB files have 'LMB ' at the start */
    filetype=LMB;
  } else filetype=None;

  error=GE_NONE;
  
  rewind(file);

  backup->PhonePhonebookUsed=0;
  backup->SIMPhonebookUsed=0;
  backup->StartupLogoAvailable=false;
  backup->StartupText.text[0]=0;
  backup->OperatorLogoAvailable=false;
  backup->CallerAvailable=false;
  backup->SpeedAvailable=false;

  switch (filetype) {
    case LMB: error=loadlmb(file,backup); fclose(file); break;
    default : error=GE_INVALIDFILEFORMAT;
  }

  return(error);
}

GSM_Error GSM_SaveBinRingtoneFile(char *FileName, GSM_BinRingtone *ringtone)
{

  FILE *file;

  file = fopen(FileName, "wb");
      
  if (!file) return(GE_CANTOPENFILE);
   	
  fwrite(ringtone->frame, 1, ringtone->length, file);   	

  fclose(file);
   
  return GE_NONE;
}
