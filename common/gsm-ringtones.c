/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides support for ringtones.

*/

#ifdef WIN32
  #include <windows.h>
  #include "misc_win32.h"
#else
  #include <unistd.h>
#endif

#include "gsm-api.h"

GSM_Ringtone SMringtone;
GSM_BinRingtone ringtone;

/* Beats-per-Minute Encoding */

int BeatsPerMinute[] = {
   25,  28,  31,  35,  40,  45,  50,  56,  63,  70,
   80,  90, 100, 112, 125, 140, 160, 180, 200, 225,
  250, 285, 320, 355, 400, 450, 500, 565, 635, 715,
  800, 900
};

struct OneRingtone RingingTones[] = {
         {"",0,0},
/*  1 */ {"Uploaded #1",0,0},   /*  2 */ {"Ring ring",0,0},
/*  3 */ {"Low",0,0},           /*  4 */ {"Fly",0,0},
/*  5 */ {"Mosquito",0,0},      /*  6 */ {"Bee",0,0},
/*  7 */ {"Intro",0,0},         /*  8 */ {"Etude",0,0},
/*  9 */ {"Hunt",0,0},          /* 10 */ {"Going up",0,0},
/* 11 */ {"City bird",0,0},     /* 12 */ {"Chase",0,0},
/* 13 */ {"Scifi",0,0},         /* 14 */ {"Kick",0,0},
/* 15 */ {"Do-mi-so",0,0},      /* 16 */ {"Robo N1X",0,0},
/* 17 */ {"Dizzy",0,0},         /* 18 */ {"Playground",0,0},
/* 19 */ {"That's it!",0,0},    /* 20 */ {"Grande valse",0,0},
/* 21 */ {"Knock knock",0,0},   /* 22 */ {"Knock again",0,0},
/* 23 */ {"Helan",0,0},         /* 24 */ {"Fuga",0,0},          
/* 25 */ {"Menuet",0,0},        /* 26 */ {"Ode to Joy",0,0},
/* 27 */ {"Elise",0,0},         /* 28 */ {"Mozart 40",0,0},
/* 29 */ {"Piano Concerto",0,0},/* 30 */ {"William Tell",0,0},
/* 31 */ {"Badinerie",0,0},     /* 32 */ {"Polka",0,0},
/* 33 */ {"Attraction",0,0},    /* 34 */ {"Polite",0,0},
/* 35 */ {"Persuasion",0,0},    /* 36 */ {"Tick tick",0,0},
/* 37 */ {"Samba",0,0},         /* 38 */ {"Orient",0,0},
/* 39 */ {"Charleston",0,0},    /* 40 */ {"Songette",0,0},    
/* 41 */ {"Jumping",0,0},       /* 42 */ {"Lamb",0,0},
/* 43 */ {"Marry",0,0},         /* 44 */ {"Tango",0,0},
/* 45 */ {"Tangoed",0,0},       /* 46 */ {"Down",0,0},
/* 47 */ {"Polska",0,0},        /* 48 */ {"WalzeBrilliant",0,0},
/* 49 */ {"Cicada",0,0},        /* 50 */ {"Trio",0,0},
/* 51 */ {"Circles",0,0},       /* 52 */ {"Nokia tune",0,0},
/* 53 */ {"Sunny walks",0,0},   /* 54 */ {"Basic rock",0,0},
/* 55 */ {"Reveille",0,0},      /* 56 */ {"Groovy Blue",0,0},
/* 57 */ {"Brave Scotland",0,0},/* 58 */ {"Matilda",0,0},
/* 59 */ {"Bumblebee",0,0},     /* 60 */ {"Hungarian",0,0},
/* 61 */ {"Valkyrie",0,0},      /* 62 */ {"Bach #3",0,0},
/* 63 */ {"Toreador",0,0},      /* 64 */ {"9th Symphony",0,0},
/* 65 */ {"Uploaded #2",0,0},   /* 66 */ {"Uploaded #3",0,0},
/* 67 */ {"Uploaded #4",0,0},   /* 68 */ {"Uploaded #5",0,0},
         {"",0,0}
};

int OctetAlign(unsigned char *Dest, int CurrentBit)
{
  int i=0;

  while((CurrentBit+i)%8) {
    ClearBit(Dest, CurrentBit+i);
    i++;
  }

  return CurrentBit+i;
}

int OctetAlignNumber(int CurrentBit)
{
  int i=0;

  while((CurrentBit+i)%8) { i++; }

  return CurrentBit+i;
}

int BitPack(unsigned char *Dest, int CurrentBit, unsigned char *Source, int Bits)
{

  int i;

  for (i=0; i<Bits; i++)
    if (GetBit(Source, i))   SetBit(Dest, CurrentBit+i);
                      else ClearBit(Dest, CurrentBit+i);

  return CurrentBit+Bits;
}

int GSM_GetTempo(int Beats) {

  int i=0;

  while ( i < sizeof(BeatsPerMinute)/sizeof(BeatsPerMinute[0])) {

    if (Beats<=BeatsPerMinute[i]) break;
    i++;
  }

  return i<<3;
}    

int BitPackByte(unsigned char *Dest, int CurrentBit, unsigned char Command, int Bits) {

  unsigned char Byte[]={Command};

  return BitPack(Dest, CurrentBit, Byte, Bits);
}


/* This is messy but saves using the math library! */

int GSM_GetDuration(int number, unsigned char *spec) {

  int duration=0;

  switch (number) {

  case 128*3/2: duration=Duration_Full; *spec=DottedNote;        break;  
  case 128*2/3: duration=Duration_Full; *spec=Length_2_3;        break;  
  case 128    : duration=Duration_Full; *spec=NoSpecialDuration; break;  
  case 64*9/4 : duration=Duration_1_2;  *spec=DoubleDottedNote;  break;    
  case 64*3/2 : duration=Duration_1_2;  *spec=DottedNote;        break;  
  case 64*2/3 : duration=Duration_1_2;  *spec=Length_2_3;        break;  
  case 64     : duration=Duration_1_2;  *spec=NoSpecialDuration; break;  
  case 32*9/4 : duration=Duration_1_4;  *spec=DoubleDottedNote;  break;    
  case 32*3/2 : duration=Duration_1_4;  *spec=DottedNote;        break;  
  case 32*2/3 : duration=Duration_1_4;  *spec=Length_2_3;        break;  
  case 32     : duration=Duration_1_4;  *spec=NoSpecialDuration; break;  
  case 16*9/4 : duration=Duration_1_8;  *spec=DoubleDottedNote;  break;    
  case 16*3/2 : duration=Duration_1_8;  *spec=DottedNote;        break;  
  case 16*2/3 : duration=Duration_1_8;  *spec=Length_2_3;        break;  
  case 16     : duration=Duration_1_8;  *spec=NoSpecialDuration; break;  
  case 8*9/4  : duration=Duration_1_16; *spec=DoubleDottedNote;  break;    
  case 8*3/2  : duration=Duration_1_16; *spec=DottedNote;        break;  
  case 8*2/3  : duration=Duration_1_16; *spec=Length_2_3;        break;  
  case 8      : duration=Duration_1_16; *spec=NoSpecialDuration; break;  
  case 4*9/4  : duration=Duration_1_32; *spec=DoubleDottedNote;  break;    
  case 4*3/2  : duration=Duration_1_32; *spec=DottedNote;        break;  
  case 4*2/3  : duration=Duration_1_32; *spec=Length_2_3;        break;  
  case 4      : duration=Duration_1_32; *spec=NoSpecialDuration; break;  
  }

  return duration;
}


int GSM_GetNote(int number) {
  
  int note=0;
 
  if (number!=255) {
    note=number%14;
    switch (note) {

    case  0: note=Note_C;   break;
    case  1: note=Note_Cis; break;
    case  2: note=Note_D;   break;
    case  3: note=Note_Dis; break;
    case  4: note=Note_E;   break;
    case  6: note=Note_F;   break;
    case  7: note=Note_Fis; break;
    case  8: note=Note_G;   break;
    case  9: note=Note_Gis; break;
    case 10: note=Note_A;   break;
    case 11: note=Note_Ais; break;
    case 12: note=Note_H;   break;
    }
  }
  else note = Note_Pause;

  return note;

}

int GSM_GetScale(int number) {

  int scale=-1;

  if (number!=255) {
    scale=number/14;

    /* Ensure the scale is valid */
    scale%=4;

    scale=scale<<6;
  }
  return scale;
}

/* This function packs the ringtone from the structure "ringtone" to
   "package", where maxlength means length of package.
   Function returns number of packed notes and change maxlength to
   number of used chars in "package" */
u8 GSM_PackRingtone(GSM_Ringtone *ringtone, unsigned char *package, int *maxlength)
{
  int StartBit=0;
  unsigned char CommandLength = 0x02;
  unsigned char spec;
  int oldscale=10, newscale=0, oldstyle=0, oldtempo=0;
  int HowMany=0;              /* How many instructions packed */
  int HowLong=0;              /* How many bits packed */
  int StartNote=0, EndNote=0; /* First and last packed note from ringtone */

  /* Default ringtone parameters */
  u8 DefNoteScale=2, DefNoteDuration=4;
  int DefNoteTempo=63;
  u8 DefNoteStyle=NaturalStyle;
  
  int buffer[6];              /* Used to find default ringtone parameters */
  int i,j,k=0,thisnote,thisnotelong;
  
  /* Find the most frequently used duration and use this for the default */
  for (i=0;i<6;i++) buffer[i]=0;
  for (i=0;i<ringtone->NrNotes;i++) {
    switch (ringtone->notes[i].duration) {
      case 192: buffer[0]++; break;
      case 128: buffer[0]++; break;
      case  96:	buffer[1]++; break;
      case  64: buffer[1]++; break;
      case  48: buffer[2]++; break;
      case  32: buffer[2]++; break;
      case  24: buffer[3]++; break;
      case  16: buffer[3]++; break;
      case  12: buffer[4]++; break;
      case   8: buffer[4]++; break;
      case   6: buffer[5]++; break;
      case   4: buffer[5]++; break;
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

  /* Finally convert the default duration */
  switch (k) {
      case 0: DefNoteDuration=128; break;	
      case 1: DefNoteDuration= 64; break;	
      case 2: DefNoteDuration= 32; break;	
      case 3: DefNoteDuration= 16; break;	
      case 4: DefNoteDuration=  8; break;	
      case 5: DefNoteDuration=  4; break;	
     default: DefNoteDuration= 16; break;	
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

  StartBit=BitPackByte(package, StartBit, CommandLength, 8);
  StartBit=BitPackByte(package, StartBit, RingingToneProgramming, 7);

  /* The page 3-23 of the specs says that <command-part> is always
     octet-aligned. */
  StartBit=OctetAlign(package, StartBit);

  StartBit=BitPackByte(package, StartBit, Sound, 7);
  StartBit=BitPackByte(package, StartBit, BasicSongType, 3);

  /* Set special chars in ringtone name */
  for (i=0;i<strlen(ringtone->name);i++) {
    if (ringtone->name[i]=='~') ringtone->name[i]=1; //enables/disables blinking
    if (ringtone->name[i]=='`') ringtone->name[i]=0; //hides rest ot contents
  }

  /* Packing the name of the tune. */
  StartBit=BitPackByte(package, StartBit, strlen(ringtone->name)<<4, 4);
  StartBit=BitPack(package, StartBit, ringtone->name, 8*strlen(ringtone->name));

  /* Set special chars in ringtone name */
  for (i=0;i<strlen(ringtone->name);i++) {
    if (ringtone->name[i]==1) ringtone->name[i]='~'; //enables/disables blinking
    if (ringtone->name[i]==0) ringtone->name[i]='`'; //hides rest ot contents
  }

  /* Info about song pattern */
  StartBit=BitPackByte(package, StartBit, 0x01, 8); /* One song pattern */
  StartBit=BitPackByte(package, StartBit, PatternHeaderId, 3);
  StartBit=BitPackByte(package, StartBit, A_part, 2);
  StartBit=BitPackByte(package, StartBit, ringtone->Loop<<4, 4);

  /* Info, how long is contents for SMS */
  HowLong=8+8+7+3+4+8*strlen(ringtone->name)+8+3+2+4+8+3+2+3+5;
  
  /* Calculating number of instructions in the tune.
     Each Note contains Note and (sometimes) Scale.
     Default Tempo and Style are instructions too. */
  HowMany=2; /* Default Tempo and Style */

  /* Default style and tempo */
  DefNoteStyle=ringtone->notes[0].style;
  DefNoteTempo=ringtone->notes[0].tempo;
  oldstyle=DefNoteStyle;
  oldtempo=DefNoteTempo;

  for(i=0; i<ringtone->NrNotes; i++) {

    /* PC Composer 2.0.010 doesn't like, when we start ringtone from pause:
       displays, that format is invalid and
       hangs, when you move mouse over place, where pause is */       
    if (GSM_GetNote(ringtone->notes[i].note)==Note_Pause && oldscale==10) {
      StartNote++;
    } else {

      thisnote=0;
      thisnotelong=0;
     
      /* we don't write Scale/Style info before "Pause" note - it saves place */
      if (GSM_GetNote(ringtone->notes[i].note)!=Note_Pause) {

        if (ringtone->allnotesscale ||
            oldscale!=(newscale=GSM_GetScale(ringtone->notes[i].note))) {

          /* We calculate, if we have space to add next scale instruction */
          if (((OctetAlignNumber(HowLong+5)+8)/8)<=(*maxlength)) {
            oldscale=newscale;
            HowLong+=5;
            HowMany++;
            thisnote++;
            thisnotelong+=5;
	  } else {
	    break;
	  }
	}
	if (ringtone->notes[i].style!=oldstyle) {
          /* We calculate, if we have space to add next style instruction */
          if (((OctetAlignNumber(HowLong+5)+8)/8)<=(*maxlength)) {
	    oldstyle=ringtone->notes[i].style;
            HowLong+=5;
            HowMany++;
            thisnote++;
            thisnotelong+=5;
	  } else {
            HowLong=HowLong-thisnotelong;
            HowMany=HowMany-thisnote;
            break;
	  }
	}
      }
      
      if (ringtone->notes[i].tempo!=oldtempo) {
        /* We calculate, if we have space to add next tempo instruction */
        if (((OctetAlignNumber(HowLong+8)+8)/8)<=(*maxlength)) {
          oldtempo=ringtone->notes[i].tempo;
          HowLong+=8;
          HowMany++;
          thisnote++;
          thisnotelong+=8;
	} else {
          HowLong=HowLong-thisnotelong;
          HowMany=HowMany-thisnote;
          break;
        }
      }
    
      /* We calculate, if we have space to add next note instruction */
      if (((OctetAlignNumber(HowLong+12)+8)/8)<=(*maxlength)) {
        HowMany++;
        EndNote++;
        HowLong+=12;
      } else {
        HowLong=HowLong-thisnotelong;
        HowMany=HowMany-thisnote;
        break;
      }
    }

    /* We are sure, we pack it for SMS or setting to phone, not for OTT file */    
    if (*maxlength<1000) {
       /* Like Pc Composer say - before of phone limitations...*/
      if ((EndNote-StartNote)==FB61_MAX_RINGTONE_NOTES-1) break;
    }
  }

  StartBit=BitPackByte(package, StartBit, HowMany, 8);
#ifdef DEBUG
//  fprintf(stdout,_("length of new pattern: %i %i\n"),HowMany,StartBit);
#endif

  /* Style */
  StartBit=BitPackByte(package, StartBit, StyleInstructionId, 3);
  StartBit=BitPackByte(package, StartBit, DefNoteStyle, 2);
    
  /* Beats per minute/tempo of the tune */
  StartBit=BitPackByte(package, StartBit, TempoInstructionId, 3);
  StartBit=BitPackByte(package, StartBit, GSM_GetTempo(DefNoteTempo), 5);
#ifdef DEBUG
//  fprintf(stdout,_("def temp: %i %i\n"),GSM_GetTempo(DefNoteTempo),StartBit);
#endif

  /* Default scale */
  oldscale=10;
  
  /* Default style */
  oldstyle=DefNoteStyle;

  /* Default tempo */
  oldtempo=DefNoteTempo;

  /* Notes packing */
  for(i=StartNote; i<(EndNote+StartNote); i++) {

    /* we don't write Scale info before "Pause" note - it saves place */
    if (GSM_GetNote(ringtone->notes[i].note)!=Note_Pause) {
      if (ringtone->allnotesscale ||
          oldscale!=(newscale=GSM_GetScale(ringtone->notes[i].note))) {
#ifdef DEBUG
//    fprintf(stdout,_("Scale\n"));
#endif
        oldscale=newscale;
        StartBit=BitPackByte(package, StartBit, ScaleInstructionId, 3);
        StartBit=BitPackByte(package, StartBit, GSM_GetScale(ringtone->notes[i].note), 2);
      }
      if (ringtone->notes[i].style!=oldstyle) {
        /* Style */
        StartBit=BitPackByte(package, StartBit, StyleInstructionId, 3);
        StartBit=BitPackByte(package, StartBit, ringtone->notes[i].style, 2);
        oldstyle=ringtone->notes[i].style;
      }
    }

    if (ringtone->notes[i].tempo!=oldtempo) {
      /* Beats per minute/tempo of the tune */
      StartBit=BitPackByte(package, StartBit, TempoInstructionId, 3);
      StartBit=BitPackByte(package, StartBit, GSM_GetTempo(ringtone->notes[i].tempo), 5);
      oldtempo=ringtone->notes[i].tempo;
    }    
    
    /* Note */
    StartBit=BitPackByte(package, StartBit, NoteInstructionId, 3);
    StartBit=BitPackByte(package, StartBit, GSM_GetNote(ringtone->notes[i].note), 4);
    StartBit=BitPackByte(package, StartBit, GSM_GetDuration(ringtone->notes[i].duration,&spec), 3);
    StartBit=BitPackByte(package, StartBit, spec, 2);

#ifdef DEBUG    
//    fprintf(stdout,_("note(%i): %i, scale: %i, duration: %i, spec: %i\n"),i,ringtone->notes[i].note,GSM_GetScale(ringtone->notes[i].note),GSM_GetDuration(ringtone->notes[i].duration,&spec),spec);
#endif

  }

  StartBit=OctetAlign(package, StartBit);

  StartBit=BitPackByte(package, StartBit, CommandEnd, 8);
  
#ifdef DEBUG
  if (StartBit!=OctetAlignNumber(HowLong)+8)
    fprintf(stdout,_("Error in PackRingtone - StartBit different to HowLong %d - %d)\n"),StartBit,OctetAlignNumber(HowLong)+8);
#endif

  *maxlength=StartBit/8;  

  return(EndNote+StartNote);
}

int BitUnPack(unsigned char *Dest, int CurrentBit, unsigned char *Source, int Bits)
{
  int i;

  for (i=0; i<Bits; i++)
    if (GetBit(Dest, CurrentBit+i)) {   SetBit(Source, i); }
                               else { ClearBit(Source, i); }

  return CurrentBit+Bits;
}

int BitUnPackInt(unsigned char *Src, int CurrentBit, int *integer, int Bits) {

  int l=0,z=128,i;

  for (i=0; i<Bits; i++) {
    if (GetBit(Src, CurrentBit+i)) l=l+z;
    z=z/2;
  }

  *integer=l;
  
  return CurrentBit+i;
}

int OctetUnAlign(int CurrentBit)
{
  int i=0;

  while((CurrentBit+i)%8) i++;

  return CurrentBit+i;
}

/* TODO: better checking, if contents of ringtone is OK */
GSM_Error GSM_UnPackRingtone(GSM_Ringtone *ringtone, char *package, int maxlength)
{
  int StartBit=0;
  int HowMany;
  int l,q,i;
  int spec;

  /* Default ringtone parameters */
  u8 DefNoteScale=2, DefNoteDuration=4;
  int DefNoteTempo=63;
  u8 DefNoteStyle=NaturalStyle;

  ringtone->allnotesscale=false;
  
  StartBit=BitUnPackInt(package,StartBit,&l,8);
#ifdef DEBUG
  if (l!=0x02)
    fprintf(stdout,_("Not header\n"));  
#endif
  if (l!=0x02) return GE_SUBFORMATNOTSUPPORTED;

  StartBit=BitUnPackInt(package,StartBit,&l,7);    
#ifdef DEBUG
  if (l!=RingingToneProgramming)
    fprintf(stdout,_("Not RingingToneProgramming\n"));  
#endif
  if (l!=RingingToneProgramming) return GE_SUBFORMATNOTSUPPORTED;
    
  /* The page 3-23 of the specs says that <command-part> is always
     octet-aligned. */
  StartBit=OctetUnAlign(StartBit);

  StartBit=BitUnPackInt(package,StartBit,&l,7);    
#ifdef DEBUG
  if (l!=Sound)
    fprintf(stdout,_("Not Sound\n"));  
#endif
  if (l!=Sound) return GE_SUBFORMATNOTSUPPORTED;

  StartBit=BitUnPackInt(package,StartBit,&l,3);    
#ifdef DEBUG
  if (l!=BasicSongType)
    fprintf(stdout,_("Not BasicSongType\n"));  
#endif
  if (l!=BasicSongType) return GE_SUBFORMATNOTSUPPORTED;

  /* Getting length of the tune name */
  StartBit=BitUnPackInt(package,StartBit,&l,4);
  l=l>>4;
#ifdef DEBUG
//  fprintf(stdout,_("Length of name: %i\n"),l);
#endif

  /* Unpacking the name of the tune. */
  StartBit=BitUnPack(package, StartBit, ringtone->name, 8*l);
  ringtone->name[l]=0;

  /* Set special chars in ringtone name */
  for (i=0;i<strlen(ringtone->name);i++) {
    if (ringtone->name[i]==1) ringtone->name[i]='~'; //enables/disables blinking
    if (ringtone->name[i]==0) ringtone->name[i]='`'; //hides rest ot contents
  }

#ifdef DEBUG
//   fprintf(stdout,_("Name: %s\n"),ringtone->name);
#endif

  StartBit=BitUnPackInt(package,StartBit,&l,8);    
#ifdef DEBUG
//  fprintf(stdout,_("Number of song patterns: %i\n"),l);
#endif
  if (l!=1) return GE_SUBFORMATNOTSUPPORTED; //we support only one song pattern

  StartBit=BitUnPackInt(package,StartBit,&l,3);          
#ifdef DEBUG
  if (l!=PatternHeaderId)
    fprintf(stdout,_("Not PatternHeaderId\n"));
#endif
  if (l!=PatternHeaderId) return GE_SUBFORMATNOTSUPPORTED;

  StartBit+=2; //Pattern ID - we ignore it

  StartBit=BitUnPackInt(package,StartBit,&l,4);          
  l=l>>4;
#ifdef DEBUG
  fprintf(stdout,_("Loop value: %i\n"),l);
#endif
  ringtone->Loop=l;

  HowMany=0;
  StartBit=BitUnPackInt(package, StartBit, &HowMany, 8);
#ifdef DEBUG
  fprintf(stdout,_("length of new pattern: %i %i\n"),HowMany,StartBit);
#endif

  ringtone->NrNotes=0;
    
  for (i=0;i<HowMany;i++) {

    StartBit=BitUnPackInt(package,StartBit,&q,3);              
    switch (q) {
      case VolumeInstructionId:
#ifdef DEBUG
//        fprintf(stdout,_("Volume\n"));
#endif
        StartBit+=4;
        break;
      case StyleInstructionId:
        StartBit=BitUnPackInt(package,StartBit,&l,2);              
#ifdef DEBUG
//	fprintf(stdout,_("Style %i\n"),l>>6);
#endif
	switch (l) {
	  case StaccatoStyle  : DefNoteStyle=StaccatoStyle;   break;
	  case ContinuousStyle: DefNoteStyle=ContinuousStyle; break;
	  case NaturalStyle   : DefNoteStyle=NaturalStyle;    break;
	}
	break;
      case TempoInstructionId:
        StartBit=BitUnPackInt(package,StartBit,&l,5);              	
        l=l>>3;
        DefNoteTempo=BeatsPerMinute[l];
#ifdef DEBUG
//	fprintf(stdout,_("Tempo %i\n"),l);
#endif
        break;
      case ScaleInstructionId:
        StartBit=BitUnPackInt(package,StartBit,&l,2);
	DefNoteScale=l>>6;
#ifdef DEBUG
//	fprintf(stdout,_("scale: %i %i\n"),DefNoteScale,ringtone->NrNotes);
#endif
	break;
      case NoteInstructionId:
        StartBit=BitUnPackInt(package,StartBit,&l,4);    

        switch (l) {
          case Note_C  :ringtone->notes[ringtone->NrNotes].note=0;break;
          case Note_Cis:ringtone->notes[ringtone->NrNotes].note=1;break;
          case Note_D  :ringtone->notes[ringtone->NrNotes].note=2;break;
          case Note_Dis:ringtone->notes[ringtone->NrNotes].note=3;break;
          case Note_E  :ringtone->notes[ringtone->NrNotes].note=4;break;
          case Note_F  :ringtone->notes[ringtone->NrNotes].note=6;break;
          case Note_Fis:ringtone->notes[ringtone->NrNotes].note=7;break;
          case Note_G  :ringtone->notes[ringtone->NrNotes].note=8;break;
          case Note_Gis:ringtone->notes[ringtone->NrNotes].note=9;break;
          case Note_A  :ringtone->notes[ringtone->NrNotes].note=10;break;
          case Note_Ais:ringtone->notes[ringtone->NrNotes].note=11;break;
          case Note_H  :ringtone->notes[ringtone->NrNotes].note=12;break;
          default      :ringtone->notes[ringtone->NrNotes].note=255;break; //Pause ?
        }
      
        if (ringtone->notes[ringtone->NrNotes].note!=255)
          ringtone->notes[ringtone->NrNotes].note=ringtone->notes[ringtone->NrNotes].note+DefNoteScale*14;

        StartBit=BitUnPackInt(package,StartBit,&l,3);    
	DefNoteDuration=l;

        StartBit=BitUnPackInt(package,StartBit,&spec,2);    

        if (DefNoteDuration==Duration_Full && spec==DottedNote)
            ringtone->notes[ringtone->NrNotes].duration=128*3/2;
        if (DefNoteDuration==Duration_Full && spec==Length_2_3)
            ringtone->notes[ringtone->NrNotes].duration=128*2/3;
        if (DefNoteDuration==Duration_Full && spec==NoSpecialDuration)
            ringtone->notes[ringtone->NrNotes].duration=128;
        if (DefNoteDuration==Duration_1_2 && spec==DottedNote)
            ringtone->notes[ringtone->NrNotes].duration=64*3/2;
        if (DefNoteDuration==Duration_1_2 && spec==Length_2_3)
            ringtone->notes[ringtone->NrNotes].duration=64*2/3;
        if (DefNoteDuration==Duration_1_2 && spec==NoSpecialDuration)
            ringtone->notes[ringtone->NrNotes].duration=64;
        if (DefNoteDuration==Duration_1_4 && spec==DottedNote)
            ringtone->notes[ringtone->NrNotes].duration=32*3/2;
        if (DefNoteDuration==Duration_1_4 && spec==Length_2_3)
            ringtone->notes[ringtone->NrNotes].duration=32*2/3;
        if (DefNoteDuration==Duration_1_4 && spec==NoSpecialDuration)
            ringtone->notes[ringtone->NrNotes].duration=32;
        if (DefNoteDuration==Duration_1_8 && spec==DottedNote)
            ringtone->notes[ringtone->NrNotes].duration=16*3/2;
        if (DefNoteDuration==Duration_1_8 && spec==Length_2_3)
            ringtone->notes[ringtone->NrNotes].duration=16*2/3;
        if (DefNoteDuration==Duration_1_8 && spec==NoSpecialDuration)
            ringtone->notes[ringtone->NrNotes].duration=16;
        if (DefNoteDuration==Duration_1_16 && spec==DottedNote)
            ringtone->notes[ringtone->NrNotes].duration=8*3/2;
        if (DefNoteDuration==Duration_1_16 && spec==Length_2_3)
            ringtone->notes[ringtone->NrNotes].duration=8*2/3;
        if (DefNoteDuration==Duration_1_16 && spec==NoSpecialDuration)
            ringtone->notes[ringtone->NrNotes].duration=8;
        if (DefNoteDuration==Duration_1_32 && spec==DottedNote)
            ringtone->notes[ringtone->NrNotes].duration=4*3/2;
        if (DefNoteDuration==Duration_1_32 && spec==Length_2_3)
            ringtone->notes[ringtone->NrNotes].duration=4*2/3;
        if (DefNoteDuration==Duration_1_32 && spec==NoSpecialDuration)
            ringtone->notes[ringtone->NrNotes].duration=4;

        ringtone->notes[ringtone->NrNotes].style=DefNoteStyle;

        ringtone->notes[ringtone->NrNotes].tempo=DefNoteTempo;
	
#ifdef DEBUG    
//    fprintf(stdout,_("note(%i): %i, scale: %i, duration: %i, spec: %i\n"),ringtone->NrNotes,ringtone->notes[ringtone->NrNotes].note,DefNoteScale,DefNoteDuration,spec);
#endif
        if (ringtone->NrNotes==FB61_MAX_RINGTONE_NOTES) break;
	
        ringtone->NrNotes++;
        break;
      default:
#ifdef DEBUG
    fprintf(stdout,_("Unsupported block %i %i\n"),q,i);  
#endif
        return GE_SUBFORMATNOTSUPPORTED;
    } 
  }

#ifdef DEBUG
//  printf("Number of notes=%d\n",ringtone->NrNotes);
#endif

  return GE_NONE;
}

GSM_Error GSM_ReadRingtone(GSM_SMSMessage *message, GSM_Ringtone *ringtone)
{
  if (message->UDHType==GSM_RingtoneUDH) {
    return GSM_UnPackRingtone(ringtone, message->MessageText, message->Length);
  } else return GE_SUBFORMATNOTSUPPORTED;
}

int GSM_GetFrequency(int number) {
  
  int freq=0;

  /* Values according to the software from http://iki.fi/too/sw/xring/
     generated with:
     perl -e 'print int(4400 * (2 **($_/12)) + .5)/10, "\n" for(3..14)'
  */ 
  if (number!=255) {
    freq=number%14;
    switch (freq) {

    case  0: freq=523.3; break; // C
    case  1: freq=554.4; break; // Cis

    case  2: freq=587.3; break; //D
    case  3: freq=622.3; break; //Dis
    
    case  4: freq=659.3; break; //E

    case  6: freq=698.5; break; //F
    case  7: freq=740;   break; //Fis

    case  8: freq=784;   break; //G
    case  9: freq=830.6; break; //Gis

    case 10: freq=880;   break; //A
    case 11: freq=932.3; break; //Ais
    
    case 12: freq=987.8; break; //H

    default: freq=0; break;
    }
  }
  else freq = 0;

  if ((number/14)!=0) freq=freq*(number/14);
	         else freq=freq/2;

  return freq;

}

/* Very fast hack. It should be written correctly ! */
void GSM_PlayOneNote (GSM_RingtoneNote note) {
  int Hz;

  Hz=GSM_GetFrequency(note.note);
  	
  GSM->PlayTone(Hz,5);

  /* Is it correct ? Experimental values here */
  switch (note.style) {
    case StaccatoStyle:
      usleep (7500);
      GSM->PlayTone(0,0);	
      usleep ((1500000/note.tempo*note.duration)-(7500));
      break;
    case ContinuousStyle:
      usleep  (1500000/note.tempo*note.duration);
      break;
    case NaturalStyle:
      usleep  (1500000/note.tempo*note.duration-50);
      GSM->PlayTone(0,0);	
      usleep (50);
      break;	
  }
}

void GSM_PlayRingtone (GSM_Ringtone *ringtone) {

  int i;
  
  for (i=0;i<ringtone->NrNotes;i++) {
     GSM_PlayOneNote(ringtone->notes[i]);
  }      

  /* Disables buzzer */
  GSM->PlayTone(255*255,0);  
}

/* Initializes one ringtone: first is number of ringtone in 
   RingingTones in gnokii.h, second its' code, last position in phone menu */
void RT(int number, int code, int menu) {
  RingingTones[number].code=code;
  RingingTones[number].menu=menu;
}

/* This function initializes structures with ringtones names adequate for
   your phone model and firmware; if your phone is not now supported:
   1.set first ringtone in 1'st profile in your phone
   2.make ./gnokii --getprofile 1
   3.read ringtone code
   4.see in gnokii.h, if ringtone name is gnokii.h in RingingTones (if not, add)
   5.put here RT(a,b,c), where a is number of name in RingingTones in gnokii.h,
     b is its' code and c is number of ringtone in phone menu
   6.repeat steps 1-5 for all ringtones
   7.send me (Marcin-Wiacek@Topnet.PL) all RT and phone model */
void PrepareRingingTones(char model[64], char rev[64]) {

  char rev2[64];
  int i;
  bool doit;

  if (!RingingTones[0].code) {
    if (!strcmp(model,"NSE-1")) //5110
    {
      RT( 2,18, 1);RT( 3,19, 2);RT( 7,23, 3);RT( 5,21, 4);RT( 9,25, 5);
                   RT(20,48, 7);RT(11,27, 8);RT(33,59, 9);RT(35,62,10);
      RT(46,60,11);RT(16,36,12);RT(17,37,13);RT(13,32,14);RT(14,34,15);
      RT(19,43,16);RT(18,39,17);RT(24,50,18);RT(31,57,19);RT(28,54,20);
      RT(30,56,21);RT(40,73,22);RT(39,72,23);RT(37,69,24);
      RT(23,49,26);RT(38,71,27);             RT(41,74,29);
      
      strcpy(rev2,"05.23"); //5.24 and higher
      doit=false;
      for(i=0;i<5;i++)
      {
        if (rev[i]<rev2[i]) break;
	if (rev[i]>rev2[i]) doit=true;
      }
      
      if (doit) {
         RT(22,47, 6);RT(47,58,25);RT(45,80,28);RT(43,75,30);
      } else
      {
         RT(21,47, 6);RT(32,58,25);RT(44,80,28);RT(42,75,30);
      }
      RingingTones[0].menu=30; /* How many ringtones in phone */
    }
    if (!strcmp(model,"NSM-1")) //6150
    {
      RT( 2,18, 1);RT( 3,19, 2);RT( 7,23, 3);RT( 6,22, 4);
      RT( 4,20, 5);RT( 5,21, 6);RT(15,35, 7);RT(12,30, 8);
      RT( 9,25, 9);RT(20,47,10);RT( 8,24,11);RT(11,27,12);
      RT(10,26,13);RT(34,60,14);RT(33,58,15);RT(35,61,16);
      RT(16,36,17);RT(13,32,18);RT(19,43,19);RT(18,39,20);
      RT(24,49,21);RT(31,56,22);RT(25,50,23);RT(27,52,24);
      RT(26,51,25);RT(28,53,26);RT(29,54,27);RT(30,55,28);
      RT(39,71,29);RT(37,68,30);RT(47,57,31);RT(23,48,32);
      RT(38,70,33);RT(36,67,34);RT(41,73,35);
      /*uploadable ringtone*/
      RT( 1,17,36);
      RingingTones[0].menu=36; /* How many ringtones in phone */
    }
    if (!strcmp(model,"NPE-3")) //6210
    {
      RT(19,64, 1);RT( 2,65, 2);RT( 3,66, 3);RT(15,67, 4);RT( 6,68, 5);
      RT(49,69, 6);RT(50,70, 7);RT( 7,71, 8);RT(35,72, 9);RT(33,73,10);
      RT(18,74,11);RT( 5,75,12);RT(51,76,13);RT(52,77,14);RT(53,78,15);
      RT(37,79,16);RT(54,80,17);RT(55,81,18);RT(56,82,19);RT(57,83,20);
      RT(58,84,21);RT(59,85,22);RT(25,86,23);RT(27,87,24);RT(30,88,25);
      RT(39,89,26);RT(24,90,27);RT( 8,91,28);RT(60,92,29);RT(61,93,30);
      RT(31,94,31);RT(62,95,32);RT(63,96,33);RT(64,97,34);RT(48,98,35);
      /* Uploadable ringtones */
      RT( 1,137,36);RT(65,138,37);RT(66,139,38);RT(67,140,39);RT(68,141,40);

      RingingTones[0].menu=40; /* How many ringtones in phone */
    }
    RingingTones[0].code=true;
  }
}

/* returns names from code or number in menu */
char *RingingToneName(int code, int menu)
{
  int index=1;
  GSM_Error error;

  if (code==0)
  {
    while (strcmp(RingingTones[index].name,"")) {
      if (RingingTones[index].menu==menu) break;
      index++;
    }
  } else
  {
    while (strcmp(RingingTones[index].name,"")) {
      if (RingingTones[index].code==code) break;
      index++;
    }
  }

  if (!strncmp(RingingTones[index].name,"Uploaded ",9)) {
    ringtone.location=atoi(&RingingTones[index].name[10]);

    error=GSM_GetPhoneRingtone(&ringtone,&SMringtone);
     
    if (error==GE_NONE) {
      if (GetModelFeature (FN_RINGTONES)==F_RING_SM) return SMringtone.name;
                                                else return ringtone.name;
    }
  }
  
  return RingingTones[index].name;
}

/* returns code from number in menu */
int RingingToneCode(int menu)
{
  int index=1;

  while ( RingingTones[index].menu!=menu) index++;

  return RingingTones[index].code;
}

/* returns number in menu from code */
int RingingToneMenu(int code)
{
  int index=1;

  while ( RingingTones[index].code!=code) index++;

  return RingingTones[index].menu;
}

int NumberOfRingtones()
{
  return RingingTones[0].menu;
}

int GSM_SaveRingtoneToSMS(GSM_MultiSMSMessage *SMS,
                          GSM_Ringtone *ringtone, bool profilestyle)
{  
  int i, j;
  unsigned char MessageBuffer[GSM_MAX_SMS_8_BIT_LENGTH*4];
  unsigned char MessageBuffer2[GSM_MAX_SMS_8_BIT_LENGTH*4];
  int MessageLength;
  GSM_UDH UDHType;
  
  EncodeUDHHeader(MessageBuffer, GSM_RingtoneUDH);
  MessageLength=GSM_MAX_SMS_8_BIT_LENGTH-(MessageBuffer[0]+1);
  i=GSM_PackRingtone(ringtone, MessageBuffer, &MessageLength);

  if (i!=ringtone->NrNotes && profilestyle)
  {
    MessageLength=0;
    MessageBuffer[MessageLength++]=0x30;          //SM version. Here 3.0
    MessageBuffer[MessageLength++]=SM30_RINGTONE; //ID for ringtone

    MessageBuffer[MessageLength++]=0x01;          //length hi.Later changed
    MessageBuffer[MessageLength++]=0x00;          //length lo.Later changed

    j=SM30_MAX_RINGTONE_FRAME_LENGTH;
    i=GSM_PackRingtone(ringtone, MessageBuffer2, &j);
    MessageLength=MessageLength+j;
    memcpy(MessageBuffer+4,MessageBuffer2,j);
      
    MessageBuffer[2]=j/256;
    MessageBuffer[3]=j%256;

    UDHType=GSM_ProfileUDH;
  } else
    UDHType=GSM_RingtoneUDH;

  GSM_MakeMultiPartSMS2(SMS,MessageBuffer,MessageLength, UDHType, GSM_Coding_Default);

  return i;
}

GSM_Error GSM_GetPhoneRingtone(GSM_BinRingtone *ringtone,GSM_Ringtone *SMringtone)
{
  GSM_Error error;
  int i;

  error=GSM->GetBinRingtone(ringtone);

  if (error==GE_UNKNOWNMODEL)
  {    
    /* In 3310,... we have normal "Smart Messaging" format */
    if (GetModelFeature (FN_RINGTONES)==F_RING_SM) {      
      i=7;
      if (ringtone->frame[9]==0x4a && ringtone->frame[10]==0x3a) i=8;
      ringtone->frame[i]=0x02;
	
      GSM_UnPackRingtone(SMringtone, ringtone->frame+i, ringtone->length-i);

      return GE_NONE;
    }
  }

  return error;
}
