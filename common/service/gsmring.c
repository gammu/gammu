
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#ifdef WIN32
#  include <windows.h>
#endif

#include "../gsmcomon.h"
#include "../misc/coding.h"
#include "../gsmstate.h"
#include "gsmring.h"
#include "gsmsms.h"

int GSM_RingNoteGetFrequency(GSM_RingNote Note)
{
	double freq=0;

	/* Values according to the software from http://iki.fi/too/sw/xring/
	 * generated with:
	 * perl -e 'print int(4400 * (2 **($_/12)) + .5)/10, "\n" for(3..14)'
	*/
	switch (Note.Note) {
		case Note_C  : freq = 523.3; break;
		case Note_Cis: freq = 554.4; break;
		case Note_D  : freq = 587.3; break;
		case Note_Dis: freq = 622.3; break;
		case Note_E  : freq = 659.3; break;
		case Note_F  : freq = 698.5; break;
		case Note_Fis: freq = 740;   break;
		case Note_G  : freq = 784;   break;
		case Note_Gis: freq = 830.6; break;
		case Note_A  : freq = 880;   break;
		case Note_Ais: freq = 932.3; break;
		case Note_H  : freq = 987.8; break;
		case Note_Pause:	     break;
	}
	switch (Note.Scale) {
		case Scale_440 : freq = freq / 2;	break;
		case Scale_880 :			break;
		case Scale_1760: freq = freq * 2;	break;
		case Scale_3520: freq = freq * 4;	break;
		default	       :			break;
	}
	return (int)freq;
}

int GSM_RingNoteGetFullDuration(GSM_RingNote Note)
{
	int duration = 1;

	switch (Note.Duration) {
		case Duration_Full : duration = 128;	break;
		case Duration_1_2  : duration = 64;	break;
		case Duration_1_4  : duration = 32;	break;
		case Duration_1_8  : duration = 16;	break;
		case Duration_1_16 : duration = 8;	break;
		case Duration_1_32 : duration = 4;	break;
	}
	switch (Note.DurationSpec) {
		case NoSpecialDuration	: 				break;
		case DottedNote		: duration = duration * 3/2;	break;
		case DoubleDottedNote	: duration = duration * 9/4;	break;
		case Length_2_3		: duration = duration * 2/3;	break;
	}
	return duration;
}

#ifndef PI
#  define PI 3.141592654
#endif

#define WAV_SAMPLE_RATE 44100

static GSM_Error savewav(FILE *file, GSM_Ringtone *ringtone)
{
	unsigned char 	WAV_Header[] = {
			'R','I','F','F',
			0x00,0x00,0x00,0x00,	/* Length */
			'W','A','V','E'};
	unsigned char 	FMT_Header[] = {'f','m','t',' ',
			0x10,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x44,0xac,
			0x00,0x00,0x88,0x58,0x01,0x00,0x02,0x00,0x10,0x00};
	unsigned char 	DATA_Header[] = {
			'd','a','t','a',
			0x00,0x00,0x00,0x00};	/* Length */
	short		DATA_Buffer[60000];
	long		wavfilesize;
	GSM_RingNote 	*Note;
	long		i,j,length=0;
	double 		phase=0,phase_step;

	fwrite(&WAV_Header,	1, sizeof(WAV_Header),	file);
	fwrite(&FMT_Header,	1, sizeof(FMT_Header),	file);
	fwrite(&DATA_Header,	1, sizeof(DATA_Header),	file);

	for (i=0;i<ringtone->NoteTone.NrCommands;i++) {
		if (ringtone->NoteTone.Commands[i].Type == RING_Note) {
			Note 		= &ringtone->NoteTone.Commands[i].Note;
			phase_step 	= GSM_RingNoteGetFrequency(*Note)*WAV_SAMPLE_RATE*1.5;
			for (j=0;j<((long)(GSM_RingNoteGetFullDuration(*Note)*WAV_SAMPLE_RATE/70));j++) {
				DATA_Buffer[j] 	= ((int)(sin(phase*PI)*50000));
			      	phase		= phase + phase_step;
				length++;
			}
			fwrite(&DATA_Buffer,sizeof(short),j,file);
		}
	}

	wavfilesize 	= sizeof(WAV_Header) + sizeof(FMT_Header) + sizeof(DATA_Header) + length*2;
	WAV_Header[4] 	= wavfilesize % 256;
	WAV_Header[5] 	= wavfilesize / 256;
	WAV_Header[6] 	= wavfilesize / (256*256);
	WAV_Header[7] 	= wavfilesize / (256*256*256);
	wavfilesize 	= wavfilesize - 54;
	DATA_Header[4] 	= wavfilesize % 256;
	DATA_Header[5] 	= wavfilesize / 256;
	DATA_Header[6] 	= wavfilesize / (256*256);
	DATA_Header[7] 	= wavfilesize / (256*256*256);

	fseek( file, 0, SEEK_SET);
	fwrite(&WAV_Header,	1, sizeof(WAV_Header),	file);
	fwrite(&FMT_Header,	1, sizeof(FMT_Header),	file);
	fwrite(&DATA_Header,	1, sizeof(DATA_Header),	file);

	return GE_NONE;
}

static GSM_Error savebin(FILE *file, GSM_Ringtone *ringtone)
{
	char nullchar=0x00;

	fwrite(&nullchar,1,1,file);
	fwrite(&nullchar,1,1,file);
	fprintf(file,"\x0C\x01\x2C");
	fprintf(file,"%s",DecodeUnicodeString(ringtone->Name));
	fwrite(&nullchar,1,1,file);
	fwrite(&nullchar,1,1,file);
	fwrite(ringtone->NokiaBinary.Frame,1,ringtone->NokiaBinary.Length,file);
	return GE_NONE;
}

static GSM_Error savepuremidi(FILE *file, GSM_Ringtone *ringtone)
{
 	fwrite(ringtone->NokiaBinary.Frame,1,ringtone->NokiaBinary.Length,file);
	return GE_NONE;
}

GSM_Error saverttl(FILE *file, GSM_Ringtone *ringtone)
{
	GSM_RingNoteScale	DefNoteScale;
	GSM_RingNoteDuration	DefNoteDuration;

	GSM_RingNoteStyle	DefNoteStyle=0;
	int			DefNoteTempo=0;

	bool 			started = false, firstcomma = true;
	GSM_RingNote 		*Note;

	unsigned char 		buffer[15];
	int			i,j,k=0;

  	/* Saves ringtone name */
	fprintf(file,"%s:",DecodeUnicodeString(ringtone->Name));

	/* Find the most frequently used duration */ 
	for (i=0;i<6;i++) buffer[i]=0;
	for (i=0;i<ringtone->NoteTone.NrCommands;i++) {
		if (ringtone->NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone->NoteTone.Commands[i].Note;
			/* some durations need 2 bytes in file, some 1 */
			if (Note->Duration >= Duration_Full && Note->Duration <= Duration_1_8) {
				buffer[Note->Duration/32]++;
			}
			if (Note->Duration >= Duration_1_16 && Note->Duration <= Duration_1_32) {
				buffer[Note->Duration/32]+=2;
			}
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
	DefNoteDuration = k * 32;
	dprintf("DefNoteDuration=%d\n", DefNoteDuration);
	switch (DefNoteDuration) {
		case Duration_Full:fprintf(file,"d=1"); break;
		case Duration_1_2 :fprintf(file,"d=2"); break;
		case Duration_1_4 :fprintf(file,"d=4"); break;
		case Duration_1_8 :fprintf(file,"d=8"); break;
		case Duration_1_16:fprintf(file,"d=16");break;
		case Duration_1_32:fprintf(file,"d=32");break;
	}

	/* Find the most frequently used scale */
	for (i=0;i<9;i++) buffer[i]=0;
	for (i=0;i<ringtone->NoteTone.NrCommands;i++) {
		if (ringtone->NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone->NoteTone.Commands[i].Note;
			if (Note->Note!=Note_Pause &&
			    Note->Scale >= Scale_55 && Note->Scale <= Scale_14080) {
				buffer[Note->Scale - 1]++;
			}
		}
	}
	j=0;
	for (i=0;i<9;i++) {
		if (buffer[i]>j) {
			k = i;
			j=buffer[i];
		}
	}
	DefNoteScale = k + 1;
	/* Save the default scale */
	fprintf(file,",o=%i,",DefNoteScale);
	dprintf("DefNoteScale=%d\n", DefNoteScale);

	for (i=0;i<ringtone->NoteTone.NrCommands;i++) {
		if (ringtone->NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone->NoteTone.Commands[i].Note;

			/* Trick from PPM Edit */
			if (Note->DurationSpec == DoubleDottedNote) {
				switch (Note->Duration) {
					case Duration_Full:Note->Duration = Duration_Full;break;
					case Duration_1_2 :Note->Duration = Duration_Full;break;
					case Duration_1_4 :Note->Duration = Duration_1_2; break;
					case Duration_1_8 :Note->Duration = Duration_1_4; break;
					case Duration_1_16:Note->Duration = Duration_1_8; break;
					case Duration_1_32:Note->Duration = Duration_1_16;break;
				}
				Note->DurationSpec = NoSpecialDuration;
			}

			if (!started) {
				DefNoteTempo=Note->Tempo;
				DefNoteStyle=Note->Style;
				switch (Note->Style) {
					case StaccatoStyle	: fprintf(file,"s=S,"); break;
					case NaturalStyle 	: fprintf(file,"s=N,"); break;
					case ContinuousStyle	:			break;
				}
				/* Save the default tempo */
				fprintf(file,"b=%i:",DefNoteTempo);
				dprintf("DefNoteTempo=%d\n", DefNoteTempo);
				started 	= true;
				firstcomma 	= true;
			}
			if (started) {
				if (Note->Style!=DefNoteStyle) {
					/* And a separator */
					if (!firstcomma) fprintf(file,",");
					firstcomma = false;
					DefNoteStyle=Note->Style;
					switch (Note->Style) {
						case StaccatoStyle  : fprintf(file,"s=S"); break;
						case NaturalStyle   : fprintf(file,"s=N"); break;
						case ContinuousStyle: fprintf(file,"s=C"); break;
					}
				}    
				if (Note->Tempo!=DefNoteTempo) {
					/* And a separator */
					if (!firstcomma) fprintf(file,",");
					firstcomma = false;
					DefNoteTempo=Note->Tempo;
					fprintf(file,"b=%i",DefNoteTempo);
				} 
		    		/* This note has a duration different than the default. We must save it */
				if (Note->Duration!=DefNoteDuration) {
					/* And a separator */
					if (!firstcomma) fprintf(file,",");
					firstcomma = false;
					switch (Note->Duration) {
						case Duration_Full:fprintf(file,"1"); break;
						case Duration_1_2 :fprintf(file,"2"); break;
						case Duration_1_4 :fprintf(file,"4"); break;
						case Duration_1_8 :fprintf(file,"8"); break;
						case Duration_1_16:fprintf(file,"16");break;
						case Duration_1_32:fprintf(file,"32");break;
					}
				} else {
					/* And a separator */
					if (!firstcomma) fprintf(file,",");
					firstcomma = false;
				}
		    		/* Now save the actual note */
				switch (Note->Note) {
					case Note_C  :fprintf(file,"c");	break;
					case Note_Cis:fprintf(file,"c#");	break;
					case Note_D  :fprintf(file,"d");	break;
					case Note_Dis:fprintf(file,"d#");	break;
					case Note_E  :fprintf(file,"e");	break;
					case Note_F  :fprintf(file,"f");	break;
					case Note_Fis:fprintf(file,"f#");	break;
					case Note_G  :fprintf(file,"g");	break;
					case Note_Gis:fprintf(file,"g#");	break;
					case Note_A  :fprintf(file,"a");	break;
					case Note_Ais:fprintf(file,"a#");	break;
					case Note_H  :fprintf(file,"h");	break;
					default      :fprintf(file,"p");	break; /*Pause ?*/
				}
				switch (Note->DurationSpec) {
					case DottedNote	: fprintf(file,".");	break;
					default		:			break;
				}
				if (Note->Note!=Note_Pause && Note->Scale != DefNoteScale) {
					fprintf(file,"%i",Note->Scale);
				}
			}
		}
	}  
	return GE_NONE;
}

static void saveimelody(FILE *file, GSM_Ringtone *ringtone)
{
	char 	Buffer[2000];
  	int 	i=2000;
    
	GSM_EncodeEMSSound(*ringtone, Buffer, &i, (float)1.2);
  
	fwrite(Buffer, 1, i, file);
}

static void WriteVarLen(unsigned char* midifile, int* current, long value)
{
	long buffer;

	buffer = value & 0x7f;

	while (value >>= 7) {
		buffer <<= 8;
		buffer |= 0x80;
		buffer += (value & 0x7f);
	}

	while (1) {
		midifile[(*current)++] = (unsigned char)buffer;
		if (buffer & 0x80)
			buffer >>= 8;
		else
			break;
	}
}

#define singlepauses

/* FIXME: need adding tempo before each note and scale too ? */
static void savemid(FILE* file, GSM_Ringtone *ringtone)
{
	int 		pause 	= 0, current = 26, duration, i, note=0, length = 20;
	bool		started = false;
	GSM_RingNote 	*Note;
	unsigned char midifile[3000] = {
		0x4D, 0x54, 0x68, 0x64, // MThd
		0x00, 0x00, 0x00, 0x06, // chunk length
		0x00, 0x00,             // format 0
		0x00, 0x01,             // one track
		0x00, 0x20,             // 32 per quarter note
		0x4D, 0x54, 0x72, 0x6B, // MTrk
		0x00, 0x00, 0x00, 0x00, // chunk length
		0x00, 0xFF, 0x51, 0x03, // tempo meta event
		0x00, 0x00, 0x00};      // 3 bytes for us for a quarter note
                        
	for (i = 0; i < ringtone->NoteTone.NrCommands; i++) {
		if (ringtone->NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone->NoteTone.Commands[i].Note;
			if (!started) {
				/* readmid does not read pauses at the beginning */
				if (Note->Note != Note_Pause) {
					/* FIXME: we need add tempo before each note or so... */
					long duration=60000000/Note->Tempo;

					midifile[current++] = (unsigned char)(duration >> 16);
					midifile[current++] = (unsigned char)(duration >> 8);
					midifile[current++] = (unsigned char)duration;

					started = true;
				}
			}
			if (started) {
				duration = GSM_RingNoteGetFullDuration(*Note);
				if (Note->Note == Note_Pause) {
					pause += duration;
#ifdef singlepauses
					WriteVarLen(midifile,&current,pause);
					pause=0;
					midifile[current++]=0x00;   // pause
					midifile[current++]=0x00;
#endif
				} else {
					if (Note->Note >= Note_C && Note->Note <= Note_H) {
						note = Note->Note/16 + 12 * Note->Scale - 1;
					}

					WriteVarLen(midifile,&current,pause);
					pause=0;
					midifile[current++]=0x90;   // note on
					midifile[current++]=note;
					midifile[current++]=0x64;   // forte

					WriteVarLen(midifile,&current,duration);
					midifile[current++]=0x80;   // note off
					midifile[current++]=note;
					midifile[current++]=0x64; 
				}
			}
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
	midifile[length++] = (current-22) >> 8;
	midifile[length++] = current-22;

	fwrite(midifile,1,current,file);
}

static void saveott(FILE *file, GSM_Ringtone *ringtone)
{
	char 	Buffer[2000];
  	int 	i=2000;
    
	GSM_EncodeNokiaRTTLRingtone(*ringtone, Buffer, &i);
  
	fwrite(Buffer, 1, i, file);
}

GSM_Error GSM_SaveRingtoneFile(char *FileName, GSM_Ringtone *ringtone)
{
	FILE *file;
  
	file = fopen(FileName, "wb");      
	if (!file) return(GE_CANTOPENFILE);

	switch (ringtone->Format) {
	case RING_NOTETONE:
		if (strstr(FileName,".mid")) {
			savemid(file,ringtone);
		} else if (strstr(FileName,".ott")) {
			saveott(file,ringtone);
		} else if (strstr(FileName,".imy")) {
			saveimelody(file,ringtone);
		} else if (strstr(FileName,".ime")) {
			saveimelody(file,ringtone);
		} else if (strstr(FileName,".wav")) {
			savewav(file,ringtone);
		} else {
			saverttl(file, ringtone);
		}
		break;
	case RING_NOKIABINARY:
		savebin(file, ringtone);
		break;
	case RING_MIDI: 
		savepuremidi(file, ringtone);
		break;
	}   	

	fclose(file);
   
	return GE_NONE;
}

/* Defines the character that separates fields in rtttl files. */
#define RTTTL_SEP ":"

static GSM_Error loadrttl(FILE *file, GSM_Ringtone *ringtone)
{
	GSM_RingNoteScale	DefNoteScale	= Scale_880;
	GSM_RingNoteDuration	DefNoteDuration	= Duration_1_4;
	GSM_RingNoteStyle	DefNoteStyle	= NaturalStyle;
	int			DefNoteTempo	= 63;

	unsigned char		buffer[2000],Name[100];
	unsigned char		*def, *notes, *ptr;
	GSM_RingNote		*Note;

	fread(buffer, 2000, 1, file);

	/* This is for RTTL ringtones without name. */
	if (buffer[0] != RTTTL_SEP[0]) {
		strtok(buffer, RTTTL_SEP);
		sprintf(Name, "%s", buffer);
		EncodeUnicode(ringtone->Name,Name,strlen(Name));
		def=strtok(NULL, RTTTL_SEP);
		notes=strtok(NULL, RTTTL_SEP);
	} else {
		EncodeUnicode(ringtone->Name,"Gammu",5);
		def=strtok(buffer, RTTTL_SEP);
		notes=strtok(NULL, RTTTL_SEP);
	}
	ptr=strtok(def, ", ");
	/* Parsing the <defaults> section. */
	while (ptr) {
		switch(*ptr) {
			case 'd': case 'D':
				switch (atoi(ptr+2)) {
					case  1: DefNoteDuration = Duration_Full; break;
					case  2: DefNoteDuration = Duration_1_2 ; break;
					case  4: DefNoteDuration = Duration_1_4 ; break;
					case  8: DefNoteDuration = Duration_1_8 ; break;
					case 16: DefNoteDuration = Duration_1_16; break;
					case 32: DefNoteDuration = Duration_1_32; break;
				}
				break;
			case 'o': case 'O':
				switch (atoi(ptr+2)) {
					case  4: DefNoteScale = Scale_440 ; break;
					case  5: DefNoteScale = Scale_880 ; break;
					case  6: DefNoteScale = Scale_1760; break;
					case  7: DefNoteScale = Scale_3520; break;
				}
				break;
 			case 'b': case 'B':
				DefNoteTempo=atoi(ptr+2);
				dprintf("Tempo = %i\n",DefNoteTempo);
				break;
			case 's': case 'S':
				switch (*(ptr+1)) {
					case 'C': case 'c': DefNoteStyle=ContinuousStyle;	break;
					case 'N': case 'n': DefNoteStyle=NaturalStyle;		break;
					case 'S': case 's': DefNoteStyle=StaccatoStyle;		break;	
				}
				switch (*(ptr+2)) {
					case 'C': case 'c': DefNoteStyle=ContinuousStyle;	break;
					case 'N': case 'n': DefNoteStyle=NaturalStyle;		break;
					case 'S': case 's': DefNoteStyle=StaccatoStyle;		break;	
				}
				break;
		}
		ptr=strtok(NULL,", ");
	}
	dprintf("DefNoteDuration=%d\n", DefNoteDuration);
	dprintf("DefNoteScale=%d\n", DefNoteScale);
	ptr=strtok(notes, ", ");
	/* Parsing the <note-command>+ section. */
	while (ptr && ringtone->NoteTone.NrCommands<MAX_RINGTONE_NOTES) {
		switch(*ptr) {
			case 'o': case 'O':
				switch (atoi(ptr+2)) {
					case  4: DefNoteScale = Scale_440 ; break;
					case  5: DefNoteScale = Scale_880 ; break;
					case  6: DefNoteScale = Scale_1760; break;
					case  7: DefNoteScale = Scale_3520; break;
				}
			case 's': case 'S':
				switch (*(ptr+1)) {
					case 'C': case 'c': DefNoteStyle=ContinuousStyle;	break;
					case 'N': case 'n': DefNoteStyle=NaturalStyle;		break;
					case 'S': case 's': DefNoteStyle=StaccatoStyle;		break;	
				}
				switch (*(ptr+2)) {
					case 'C': case 'c': DefNoteStyle=ContinuousStyle;	break;
					case 'N': case 'n': DefNoteStyle=NaturalStyle;		break;
					case 'S': case 's': DefNoteStyle=StaccatoStyle;		break;	
				}
				break;
			default:
				ringtone->NoteTone.Commands[ringtone->NoteTone.NrCommands].Type = RING_Note;
				Note = &ringtone->NoteTone.Commands[ringtone->NoteTone.NrCommands].Note;

				/* [<duration>] */
				Note->Duration = DefNoteDuration;
				switch (atoi(ptr)) {
					case  1: Note->Duration = Duration_Full  ; break;
					case  2: Note->Duration = Duration_1_2   ; break;
					case  4: Note->Duration = Duration_1_4   ; break;
					case  8: Note->Duration = Duration_1_8   ; break;
					case 16: Note->Duration = Duration_1_16  ; break;
					case 32: Note->Duration = Duration_1_32	 ; break;
				}
				/* Skip all numbers in duration specification. */
				while(isdigit(*ptr)) ptr++;

				/* <note> */
				/* B or b is not in specs, but I decided to put it, because
				 * it's in some RTTL files. It's the same to H note */
				Note->Note = Note_Pause;
				switch (*ptr) {
					case 'A': case 'a': Note->Note = Note_A; break;
					case 'B': case 'b': Note->Note = Note_H; break;
					case 'C': case 'c': Note->Note = Note_C; break;
					case 'D': case 'd': Note->Note = Note_D; break;
					case 'E': case 'e': Note->Note = Note_E; break;
					case 'F': case 'f': Note->Note = Note_F; break;
					case 'G': case 'g': Note->Note = Note_G; break;
					case 'H': case 'h': Note->Note = Note_H; break;
				}
				ptr++;

				if ((*ptr)=='#') {
					switch (Note->Note) {
						case Note_A : Note->Note = Note_Ais; break;
						case Note_C : Note->Note = Note_Cis; break;
						case Note_D : Note->Note = Note_Dis; break;
						case Note_F : Note->Note = Note_Fis; break;
						case Note_G : Note->Note = Note_Gis; break;
						default	    :			     break;
					}
					ptr++;
				}

				/* Check for dodgy rttl */
				/* [<special-duration>] */
				Note->DurationSpec = NoSpecialDuration;
				if (*ptr=='.') {
					Note->DurationSpec = DottedNote;
					ptr++;
				}

				/* [<scale>] */
				if (Note->Note!=Note_Pause) {
					Note->Scale = DefNoteScale;
					if (isdigit(*ptr)) {
						switch (atoi(ptr)) {
							case  4: Note->Scale = Scale_440 ; break;
							case  5: Note->Scale = Scale_880 ; break;
							case  6: Note->Scale = Scale_1760; break;
							case  7: Note->Scale = Scale_3520; break;
						}
						ptr++;
					}
				}

				/* Style */
				Note->Style=DefNoteStyle;
				/* Tempo */
				Note->Tempo=DefNoteTempo;

				ringtone->NoteTone.NrCommands++;
				break;
		}
		ptr=strtok(NULL, ", ");
	}
	return GE_NONE;
}

static GSM_Error loadott(FILE *file, GSM_Ringtone *ringtone)
{
	char 	Buffer[2000];
	int 	i;
  
	i=fread(Buffer, 1, 2000, file);

	return GSM_DecodeNokiaRTTLRingtone(ringtone, Buffer, i);
}

static GSM_Error loadcommunicator(FILE *file, GSM_Ringtone *ringtone)
{
	char 	Buffer[4000];
	int 	i,j;
  
	i=fread(Buffer, 1, 4000, file);

	i=0;j=0;
	while (true) {
		if (Buffer[j]  ==0x00 && Buffer[j+1]==0x02 &&
		    Buffer[j+2]==0x4a && Buffer[j+3]==0x3a) break;
		if (j==i-4) return GE_UNKNOWN;
		j++;
	}
	j++;

	return GSM_DecodeNokiaRTTLRingtone(ringtone, Buffer+j, i-j);  
}

static GSM_Error loadbin(FILE *file, GSM_Ringtone *ringtone)
{
	int		i;
	unsigned char	buffer[2000];

	dprintf("loading binary\n");
	ringtone->NokiaBinary.Length=fread(buffer, 1, 500, file);
	i=5;
	while (buffer[i]!=0x00) i++;
	EncodeUnicode(ringtone->Name,buffer+5,i-5);
	while (buffer[i]!=0x02 && buffer[i+1]!=0xFC && buffer[i+2]!=0x09) {
		i++;
	}
	ringtone->NokiaBinary.Length=ringtone->NokiaBinary.Length-i;
	memcpy(ringtone->NokiaBinary.Frame,buffer+i,ringtone->NokiaBinary.Length);
	dprintf("Length %i name \"%s\"\n",ringtone->NokiaBinary.Length,DecodeUnicodeString(ringtone->Name));
	return GE_NONE;
}

static GSM_Error loadpuremidi(FILE *file, GSM_Ringtone *ringtone)
{
	unsigned char	buffer[1024];

	dprintf("loading midi\n");
	ringtone->NokiaBinary.Length=fread(buffer, 1, 1024, file);
	memcpy(ringtone->NokiaBinary.Frame,buffer,ringtone->NokiaBinary.Length);
	dprintf("Length %i name \"%s\"\n",ringtone->NokiaBinary.Length,DecodeUnicodeString(ringtone->Name));
	return GE_NONE;
}

static GSM_Error loadre(FILE *file, GSM_Ringtone *ringtone)
{
	unsigned char buffer[2000];

	ringtone->NokiaBinary.Length=fread(buffer, 1, 500, file);

	if (buffer[18]==0x00 && buffer[21]!=0x02) {
		/* DCT3, Unicode subformat, 62xx & 7110 */
		CopyUnicodeString(ringtone->Name,buffer+18);
		ringtone->NokiaBinary.Length = ringtone->NokiaBinary.Length - (21+strlen(DecodeUnicodeString(ringtone->Name))*2);
		memcpy(ringtone->NokiaBinary.Frame,buffer+21+strlen(DecodeUnicodeString(ringtone->Name))*2,ringtone->NokiaBinary.Length);
	} else {
		/* DCT3, normal subformat, 32xx/33xx/51xx/5210/5510/61xx/8xxx */      
		EncodeUnicode(ringtone->Name,buffer+17,buffer[16]);
		ringtone->NokiaBinary.Length = ringtone->NokiaBinary.Length - (19+strlen(DecodeUnicodeString(ringtone->Name)));
		memcpy(ringtone->NokiaBinary.Frame,buffer+19+strlen(DecodeUnicodeString(ringtone->Name)),ringtone->NokiaBinary.Length);
	}
	dprintf("Name \"%s\"\n",DecodeUnicodeString(ringtone->Name));
	return GE_NONE;
}

GSM_Error GSM_ReadRingtoneFile(char *FileName, GSM_Ringtone *ringtone)
{
	FILE		*file;
	unsigned char	buffer[300];
	GSM_Error	error = GE_UNKNOWN;

	dprintf("Loading ringtone %s\n",FileName);
	file = fopen(FileName, "rb");
	if (!file) return(GE_CANTOPENFILE);

	/* Read the header of the file. */
	fread(buffer, 1, 4, file);
	if (ringtone->Format == 0x00) {
		ringtone->Format = RING_NOTETONE;
		if (buffer[0]==0x00 && buffer[1]==0x00 &&
		    buffer[2]==0x0C && buffer[3]==0x01)
		{
			ringtone->Format = RING_NOKIABINARY;
		}
		if (buffer[0]==0x00 && buffer[1]==0x00 &&
		    buffer[2]==0x00)
		{
			ringtone->Format = RING_NOKIABINARY;
		}
		if (buffer[0]==0x4D && buffer[1]==0x54 &&
		    buffer[2]==0x68 && buffer[3]==0x64)
		{
			ringtone->Format = RING_MIDI;
		}


	}
	rewind(file);
	switch (ringtone->Format) {
	case RING_NOTETONE:
		if (buffer[0]==0x02 && buffer[1]==0x4A) {
			error=loadott(file,ringtone);
		} else if (buffer[0]==0xC7 && buffer[1]==0x45) {
			error=loadcommunicator(file,ringtone);
		} else {
			error=loadrttl(file,ringtone);
		}
		ringtone->NoteTone.AllNotesScale=false;
		break;
	case RING_NOKIABINARY:
		if (buffer[0]==0x00 && buffer[1]==0x00 &&
		    buffer[2]==0x0C && buffer[3]==0x01)
		{
			error=loadbin(file,ringtone);
		}
		if (buffer[0]==0x00 && buffer[1]==0x00 &&
		    buffer[2]==0x00)
		{
			error=loadre(file,ringtone);
		}
		break;
	case RING_MIDI:
		EncodeUnicode(ringtone->Name,FileName,strlen(FileName));
		error = loadpuremidi(file,ringtone);
	}
	fclose(file);
	return(error);
}

/* -------------------------- required with Nokia & RTTL ------------------- */

/* Beats-per-Minute Encoding */
static int BeatsPerMinute[] = {
	25,	28,	31,	35,	40,	45,	50,	56,	63,	70,
	80,	90,	100,	112,	125,	140,	160,	180,	200,	225,
	250,	285,	320,	355,	400,	450,	500,	565,	635,	715,
	800,	900
};

int GSM_RTTLGetTempo(int Beats)
{
	size_t i=0;

	while ( i < sizeof(BeatsPerMinute)/sizeof(BeatsPerMinute[0])) {
		if (Beats<=BeatsPerMinute[i]) break;
		i++;
	}
	return i<<3;
}    

/* This function packs the ringtone from the structure "ringtone" to
   "package", where maxlength means length of package.
   Function returns number of packed notes and change maxlength to
   number of used chars in "package" */
unsigned char GSM_EncodeNokiaRTTLRingtone(GSM_Ringtone ringtone, unsigned char *package, int *maxlength)
{
	unsigned char		CommandLength 	= 0x02;
	unsigned char		Loop		= 0x15;	/* Infinite */

	unsigned char		Buffer[200];
	int			StartBit=0, OldStartBit;
	int			StartBitHowManyCommands;
	int			HowManyCommands	= 0;	/* How many instructions packed */
	int			HowManyNotes	= 0;
	int			i;
	bool			started;
	GSM_RingNote 		*Note;

	GSM_RingNoteScale 	DefScale = 255;
	GSM_RingNoteStyle 	DefStyle = 255;
	int			DefTempo = 255;
	
	StartBit=BitPackByte(package, StartBit, CommandLength, 8);
	StartBit=BitPackByte(package, StartBit, RingingToneProgramming, 7);

	/* <command-part> is always octet-aligned. */
	StartBit=OctetAlign(package, StartBit);

	StartBit=BitPackByte(package, StartBit, Sound, 7);
	StartBit=BitPackByte(package, StartBit, BasicSongType, 3);

	/* Packing the name of the tune. */
	EncodeUnicodeSpecialNOKIAChars(Buffer, ringtone.Name, strlen(DecodeUnicodeString(ringtone.Name)));
	StartBit=BitPackByte(package, StartBit, ((unsigned char)(strlen(DecodeUnicodeString(Buffer))<<4)), 4);
	StartBit=BitPack(package, StartBit, DecodeUnicodeString(Buffer), 8*strlen(DecodeUnicodeString(Buffer)));

	/* Info about song pattern */
	StartBit=BitPackByte(package, StartBit, 0x01, 8); /* One song pattern */
	StartBit=BitPackByte(package, StartBit, PatternHeaderId, 3);
	StartBit=BitPackByte(package, StartBit, A_part, 2);
	StartBit=BitPackByte(package, StartBit, ((unsigned char)(Loop<<4)), 4);

	/* Later here will be HowManyCommands */
	StartBitHowManyCommands=StartBit;
	StartBit = StartBit + 8;

	started = false;
	for (i=0; i<ringtone.NoteTone.NrCommands; i++) {
		if (ringtone.NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone.NoteTone.Commands[i].Note;
			if (!started) {
				/* First note can't be Pause - it makes problems
				 * for example with PC Composer
				 */
				if (Note->Note != Note_Pause) started = true;
			}
			if (started) {
				OldStartBit = StartBit;
				/* we don't write Scale & Style info before "Pause" note - it saves place */
				if (Note->Note!=Note_Pause) {
					if (DefScale != Note->Scale || ringtone.NoteTone.AllNotesScale) {
						if ((OctetAlignNumber(StartBit+5+8)/8)>(*maxlength)) {
							StartBit = OldStartBit;
							break;
						}
						DefScale = Note->Scale;
						StartBit=BitPackByte(package, StartBit, ScaleInstructionId, 3);
						StartBit=BitPackByte(package, StartBit, ((unsigned char)((DefScale-4)<<6)), 2);
						HowManyCommands++;
					}
					if (DefStyle != Note->Style) {
						if ((OctetAlignNumber(StartBit+5+8)/8)>(*maxlength)) {
							StartBit = OldStartBit;
							break;
						}
						DefStyle = Note->Style;
						StartBit=BitPackByte(package, StartBit, StyleInstructionId, 3);
						StartBit=BitPackByte(package, StartBit, ((unsigned char)DefStyle), 2);
						HowManyCommands++;
					}
				}
				/* Beats per minute/tempo of the tune */
				if (DefTempo != GSM_RTTLGetTempo(Note->Tempo)) {
					if ((OctetAlignNumber(StartBit+8+8)/8)>(*maxlength)) {
						StartBit = OldStartBit;
						break;
					}
					DefTempo=GSM_RTTLGetTempo(Note->Tempo);
					/* Beats per minute/tempo of the tune */
					StartBit=BitPackByte(package, StartBit, TempoInstructionId, 3);
					StartBit=BitPackByte(package, StartBit, ((unsigned char)DefTempo), 5);
					HowManyCommands++;
				}     
				if ((OctetAlignNumber(StartBit+12+8)/8)>(*maxlength)) {
					StartBit = OldStartBit;
					break;
				}
				/* Note */
				StartBit=BitPackByte(package, StartBit, NoteInstructionId, 3);
				StartBit=BitPackByte(package, StartBit, ((unsigned char)Note->Note), 4);
				StartBit=BitPackByte(package, StartBit, ((unsigned char)Note->Duration), 3);
				StartBit=BitPackByte(package, StartBit, ((unsigned char)Note->DurationSpec), 2);
				HowManyCommands++;
				/* We are sure, we pack it for SMS or setting to phone, not for OTT file */    
				if (*maxlength<1000) {
					/* Like Pc Composer say - before of phone limitations...*/
					if (HowManyNotes==130-1) break;
				}
			}
		}
		HowManyNotes++;
	}

	StartBit=OctetAlign(package, StartBit);
	StartBit=BitPackByte(package, StartBit, CommandEnd, 8);

	OldStartBit = StartBit;
	StartBit = StartBitHowManyCommands;
	/* HowManyCommands */
	StartBit = BitPackByte(package, StartBit, ((unsigned char)HowManyCommands), 8);
	StartBit = OldStartBit;

	*maxlength=StartBit/8;  

	return(i);
}

GSM_Error GSM_DecodeNokiaRTTLRingtone(GSM_Ringtone *ringtone, unsigned char *package, int maxlength)
{
	int 			StartBit=0, HowMany, l, q, i, spec;
	char 			Buffer[100];
	GSM_RingNote 		*Note;

	/* Default ringtone parameters */
	GSM_RingNoteScale 	DefScale	= Scale_880;
	GSM_RingNoteStyle 	DefStyle 	= NaturalStyle;
	int			DefTempo	= 63;

	ringtone->Format 		= RING_NOTETONE;
	ringtone->NoteTone.NrCommands 	= 0;
      
	StartBit=BitUnPackInt(package,StartBit,&l,8);
	if (l!=0x02) {
		dprintf("Not header\n");
		return GE_NOTSUPPORTED;
	}

	StartBit=BitUnPackInt(package,StartBit,&l,7);    
	if (l!=RingingToneProgramming) {
		dprintf("Not RingingToneProgramming\n");
		return GE_NOTSUPPORTED;
	}
    
	/* <command-part> is always octet-aligned. */
	StartBit=OctetUnAlign(StartBit);

	StartBit=BitUnPackInt(package,StartBit,&l,7);    
	if (l!=Sound) {
		dprintf("Not Sound\n");
		return GE_NOTSUPPORTED;
	}

	StartBit=BitUnPackInt(package,StartBit,&l,3);    
	if (l!=BasicSongType) {
		dprintf("Not BasicSongType\n");  
		return GE_NOTSUPPORTED;
	}

	/* Getting length of the tune name */
	StartBit=BitUnPackInt(package,StartBit,&l,4);
	l=l>>4;

	/* Unpacking the name of the tune. */
	StartBit=BitUnPack(package, StartBit, Buffer, 8*l);
	Buffer[l]=0;
	EncodeUnicode(ringtone->Name,Buffer,strlen(Buffer));
	DecodeUnicodeSpecialNOKIAChars(Buffer, ringtone->Name, strlen(DecodeUnicodeString(ringtone->Name)));
	CopyUnicodeString(ringtone->Name,Buffer);

	StartBit=BitUnPackInt(package,StartBit,&l,8);    
	dprintf("Number of song patterns: %i\n",l);
	/* we support only one song pattern */
	if (l!=1) return GE_NOTSUPPORTED;

	StartBit=BitUnPackInt(package,StartBit,&l,3);          
	if (l!=PatternHeaderId) {
		dprintf("Not PatternHeaderId\n");
		return GE_NOTSUPPORTED;
	}

	/* Pattern ID - we ignore it */
	StartBit+=2;

	StartBit=BitUnPackInt(package,StartBit,&l,4);          
	l=l>>4;
	dprintf("Loop value: %i\n",l);

	HowMany=0;
	StartBit=BitUnPackInt(package, StartBit, &HowMany, 8);

	for (i=0;i<HowMany;i++) {
		StartBit=BitUnPackInt(package,StartBit,&q,3);              
		switch (q) {
		case VolumeInstructionId:
			StartBit+=4;
			break;
		case StyleInstructionId:
			StartBit=BitUnPackInt(package,StartBit,&l,2);
			if (l>=NaturalStyle && l<=StaccatoStyle) DefStyle = l;
			break;
		case TempoInstructionId:
			StartBit=BitUnPackInt(package,StartBit,&l,5);              	
			DefTempo=BeatsPerMinute[l>>3];
			break;
		case ScaleInstructionId:
			StartBit=BitUnPackInt(package,StartBit,&l,2);
			DefScale=(l>>6)+4;
			break;
		case NoteInstructionId:
			Note = &ringtone->NoteTone.Commands[ringtone->NoteTone.NrCommands].Note;
			ringtone->NoteTone.Commands[ringtone->NoteTone.NrCommands].Type = RING_Note;

			StartBit=BitUnPackInt(package,StartBit,&l,4);
			Note->Note=Note_Pause;
			if (l >= Note_C && l <= Note_H) Note->Note = l;

			StartBit=BitUnPackInt(package,StartBit,&l,3);
			if (l >= Duration_Full && l <= Duration_1_32) Note->Duration = l;

			StartBit=BitUnPackInt(package,StartBit,&spec,2);
			if (spec >= NoSpecialDuration && spec <= Length_2_3) {
				Note->DurationSpec = spec;
			}

			Note->Scale = DefScale;
			Note->Style = DefStyle;
			Note->Tempo = DefTempo;
			if (ringtone->NoteTone.NrCommands==MAX_RINGTONE_NOTES) break;	
			ringtone->NoteTone.NrCommands++;
			break;
		default:
			dprintf("Unsupported block %i %i\n",q,i);  
			return GE_NOTSUPPORTED;
		} 
	}
	return GE_NONE;
}

static void RTTL2Binary(GSM_Ringtone *dest, GSM_Ringtone *src)
{
	int 			current = 0, i, note, lastnote = 0, duration;
	GSM_RingNote 		*Note;

	unsigned char tail[] =
			{0x40, 0x7D, 0x40, 0x5C, 0x0A, 0xFE, 0x40,
			 0x20, 0x40, 0x7D, 0x40, 0x37, 0x0A, 0xFE,
			 0x0A, 0x0A, 0x40, 0x32, 0x07, 0x0B};

	strcpy(dest->NokiaBinary.Frame+current,"\x02\xFC\x09");	current=current+3;
	dest->NokiaBinary.Frame[current++]=0x00;

/*	This command can be used to loop, where 0xLL = 0x01 - 0x10
 *	0x01=loop once [...] 0x10=loop infinite
 *	Commented now

 	dest->NokiaBinary.Frame[current++]=0x05;
	dest->NokiaBinary.Frame[current++]=0xLL;
 */
	strcpy(dest->NokiaBinary.Frame+current,"\x0A\x01");	current=current+2;

	for (i=0; i<src->NoteTone.NrCommands; i++) {
		switch (src->NoteTone.Commands[i].Type) {
		case RING_Note:
			Note = &src->NoteTone.Commands[i].Note;
			note = 64; /* Pause */
			if (Note->Note!=Note_Pause) {
				if (Note->Note >= Note_C && Note->Note <= Note_H) {
					note = 113 + Note->Note/16;
				}
				switch (Note->Scale) {
					case Scale_440 : 			break;
					case Scale_880 : note = note + 12; 	break;
					case Scale_1760: note = note + 24;	break;
					case Scale_3520: note = note + 36; 	break;
					default	       :			break;
				}
			}

			/* in 8 ms ticks of 7110 */
			duration = 60000 * GSM_RingNoteGetFullDuration(*Note) / Note->Tempo / 256;

			switch (Note->Style) {
			case StaccatoStyle:
				if (duration) {
					/* note only for one tick */
					dest->NokiaBinary.Frame[current++] = note;
					dest->NokiaBinary.Frame[current++] = 1;
					duration--;
				}
				/* rest pause */
				note = 0x40;             
			case NaturalStyle:
				if (note != 0x40 && duration) {
					dest->NokiaBinary.Frame[current++] = 0x40;
					/* small pause between notes */
					dest->NokiaBinary.Frame[current++] = 1;      
					duration--;
				}
			default:
				if (note != 0x40 && note == lastnote && duration) {
					dest->NokiaBinary.Frame[current++] = 0x40;
					/* small pause between same notes */
					dest->NokiaBinary.Frame[current++] = 1;      
					duration--;
				}
				while (duration > 125) {
					dest->NokiaBinary.Frame[current++] = note;
					dest->NokiaBinary.Frame[current++] = 125;
					duration -= 125;
				}
				dest->NokiaBinary.Frame[current++] = note;
				dest->NokiaBinary.Frame[current++] = duration;
			}
			lastnote = note;
		default:
			break;
		}
	}
	for (i = 0; i < (int)sizeof(tail); i++) dest->NokiaBinary.Frame[current++] = tail[i];
	dest->NokiaBinary.Length=current;
}

static void Binary2RTTL(GSM_Ringtone *dest, GSM_Ringtone *src)
{
	int 			 i = 3, j, z, NrNotes = 0, repeat = 0, accuracy;
	int			 StartRepeat = 0, EndRepeat, Speed;
	unsigned char 		 command,length=0;
	int			 NotesLen[500];
	GSM_RingNoteScale	 NotesScale[500];
	GSM_RingNoteNote	 Notes[500];
	int			 Lengths[6*4];
	GSM_RingNoteDurationSpec DurationSpec[6*4];
	GSM_RingNoteDuration	 Duration[6*4];
	bool			 foundlen;
	GSM_RingNote 		 *Note;

	while (i<src->NokiaBinary.Length) {
		command = src->NokiaBinary.Frame[i];
		i++;
		if (command != 0x06 && command != 0x00 && command != 0x09) {
			length = src->NokiaBinary.Frame[i];
			i++;
			dprintf("Block %02x %02x - ",length,command);
		} else dprintf("Block %02x - ",command);
		if (command >= 114 && command <= 161) {
			dprintf("note\n");
			if (command >= 114 && command <= 124) {
				NotesScale[NrNotes] = Scale_440;   command -= 114;
			} else if (command >= 125 && command <= 137) {
				NotesScale[NrNotes] = Scale_880;   command -= 126;
			} else if (command >= 138 && command <= 149) {
				NotesScale[NrNotes] = Scale_1760;  command -= 138;
			} else if (command >= 150 && command <= 161) {
				NotesScale[NrNotes] = Scale_3520;  command -= 150;
			}
			switch (command) {
				case   0 : Notes[NrNotes] = Note_C;	break;
				case   1 : Notes[NrNotes] = Note_Cis;	break;
				case   2 : Notes[NrNotes] = Note_D;	break;
				case   3 : Notes[NrNotes] = Note_Dis;	break;
				case   4 : Notes[NrNotes] = Note_E;	break;
				case   5 : Notes[NrNotes] = Note_F;	break;
				case   6 : Notes[NrNotes] = Note_Fis;	break;
				case   7 : Notes[NrNotes] = Note_G;	break;
				case   8 : Notes[NrNotes] = Note_Gis;	break;
				case   9 : Notes[NrNotes] = Note_A;	break;
				case  10 : Notes[NrNotes] = Note_Ais;	break;
				case  11 : Notes[NrNotes] = Note_H;	break;
			}              
			if (NrNotes > 0) {
				if (Notes[NrNotes-1] == Notes[NrNotes] &&
				    NotesScale[NrNotes-1] == NotesScale[NrNotes]) {
					NotesLen[NrNotes-1]+=length;
				} else {
					NotesLen[NrNotes]=length;
					NrNotes++;
				}
			} else {
				NotesLen[NrNotes]=length;
				NrNotes++;
			}
		} else switch (command) {
			case 0x00:
				dprintf("Unknown\n");
				break;
			case 0x05:
				dprintf("repeat %i times\n",length);
				repeat 		= length;
				StartRepeat 	= NrNotes;
				break;
			case 0x06:
				dprintf("end repeat\n");
				EndRepeat = NrNotes;
				for (z=0;z<repeat-1;z++) {
					for (j=StartRepeat;j<EndRepeat;j++) {
						Notes[NrNotes] 		= Notes[j];
						NotesScale[NrNotes] 	= NotesScale[j];
						NotesLen[NrNotes] 	= NotesLen[j];
						NrNotes++;
						dprintf("Adding repeat note %i %i\n",Notes[j],NotesLen[j]);
					}
				}
				break;
			case 0x07:
				if (length == 0x0B) {
					dprintf("Ringtone end\n");
					i = src->NokiaBinary.Length + 1;
				}
				break;
			case 0x09:
				dprintf("Unknown\n");
				break;
			case 0x0A:
				if (length == 0x01) {
					dprintf("Let's start our song\n");
					break;
				}
				if (length == 0x0A) {
					dprintf("Ending joining note\n");
					break;
				}
				if (length == 0xFE) {
					dprintf("Starting joining note\n");
					break;
				}
				break;
			case 0x40:
				dprintf("Pause\n");
				Notes[NrNotes] = Note_Pause;
				if (NrNotes > 0) {
					if (Notes[NrNotes-1] == Notes[NrNotes] &&
					    NotesScale[NrNotes-1] == NotesScale[NrNotes]) {
						NotesLen[NrNotes-1]+=length;
					} else {
						NotesLen[NrNotes]=length;
						NrNotes++;
					}
				} else {
					NotesLen[NrNotes]=length;
					NrNotes++;
				}
				break;
			default:
				dprintf("Unknown\n");
			}
	}

	while (NrNotes>0) {
		if (Notes[NrNotes-1] == Note_Pause) {
			NrNotes--;
		} else break;		
	}

	for (accuracy=1; accuracy<5; accuracy++)
	{
		i = 1;
		while (i < 1000)  {
		    	Lengths[0] = 30000/i;
			for (j=0;j<5;j++) Lengths[j+1] 	= Lengths[j] / 2;
			for (j=0;j<6;j++) Lengths[6+j] 	= Lengths[j] * 3/2;
			for (j=0;j<6;j++) Lengths[12+j] = Lengths[j] * 9/4;
			for (j=0;j<6;j++) Lengths[18+j] = Lengths[j] * 2/3;

#ifdef DEBUG
			dprintf("Length matrix (%i) : ",i);
			for (j=0;j<6*4;j++) dprintf("%i ",Lengths[j]);
			dprintf("\n");
#endif
			foundlen = false;

			for (j=0;j<NrNotes;j++) {
				dprintf("Comparing to %i\n",NotesLen[j]);
				foundlen = false;
				for (z=0;z<6*4;z++) {
					if (NotesLen[j] - Lengths[z] > -accuracy &&
					    NotesLen[j] - Lengths[z] < accuracy) {
						foundlen = true;
						break;
					}
				}
				if (!foundlen) break;
			}		
			if (foundlen) break;
			i++;
		}

		if (foundlen)
		{
			Speed = i;
			Duration[5] = Duration_1_32; Duration[4] = Duration_1_16;
			Duration[3] = Duration_1_8;  Duration[2] = Duration_1_4;
			Duration[1] = Duration_1_2;  Duration[0] = Duration_Full;
			for (i=0;i<6;i++) Duration[i] 		= Duration[i];
			for (i=0;i<6;i++) Duration[i+6] 	= Duration[i];
			for (i=0;i<6;i++) Duration[i+12] 	= Duration[i];
			for (i=0;i<6;i++) Duration[i+18] 	= Duration[i];
			for (i=0;i<6;i++) DurationSpec[i] 	= NoSpecialDuration;
			for (i=0;i<6;i++) DurationSpec[i+6] 	= DottedNote;
			for (i=0;i<6;i++) DurationSpec[i+12] 	= DoubleDottedNote;
			for (i=0;i<6;i++) DurationSpec[i+18] 	= Length_2_3;

			for (i=0;i<NrNotes;i++) {
				dest->NoteTone.Commands[i].Type	= RING_Note;
				Note = &dest->NoteTone.Commands[i].Note;
				Note->Note		= Notes[i];
				Note->Tempo 		= Speed;
				Note->Style 		= ContinuousStyle;
				if (Notes[i] != Note_Pause) Note->Scale = NotesScale[i];
				for (z=0;z<6*4;z++) {
					if (NotesLen[i] - Lengths[z] > -accuracy &&
					    NotesLen[i] - Lengths[z] < accuracy) {
						Note->Duration 		= Duration[z];
						Note->DurationSpec 	= DurationSpec[z];
						/* Trick from PPM Edit */
						if (Note->DurationSpec == DoubleDottedNote) {
							switch (Note->Duration) {
							case Duration_Full:Note->Duration = Duration_Full;break;
							case Duration_1_2 :Note->Duration = Duration_Full;break;
							case Duration_1_4 :Note->Duration = Duration_1_2; break;
							case Duration_1_8 :Note->Duration = Duration_1_4; break;
							case Duration_1_16:Note->Duration = Duration_1_8; break;
							case Duration_1_32:Note->Duration = Duration_1_16;break;
							}
							Note->DurationSpec = NoSpecialDuration;
						}
						/* Here happy creation */
						if (Note->DurationSpec == Length_2_3) {
							Note->DurationSpec = NoSpecialDuration;
						}

						break;
					}
				}
			}
			dest->NoteTone.NrCommands = NrNotes;
			dprintf("speed = %i\n",Speed);
			break;
		}
	}
	
	if (!foundlen) dest->NoteTone.NrCommands = 0;
}

GSM_Error GSM_RingtoneConvert(GSM_Ringtone *dest, GSM_Ringtone *src, GSM_RingtoneFormat	Format)
{
	dest->Format = Format;
	CopyUnicodeString(dest->Name,src->Name);
	if (src->Format==RING_NOTETONE && Format==RING_NOKIABINARY)
	{
		RTTL2Binary(dest, src);
		return GE_NONE;
	}
	if (src->Format==RING_NOKIABINARY && Format==RING_NOTETONE)
	{
		Binary2RTTL(dest, src);
		return GE_NONE;
	}
	/* The same source and target format */
	if (src->Format==Format)
	{
		memcpy(dest,src,sizeof(GSM_Ringtone));
		return GE_NONE;
	}
	return GE_NOTIMPLEMENTED;
}


unsigned char GSM_EncodeEMSSound(GSM_Ringtone ringtone, unsigned char *package, int *maxlength, double version)
{
	int 			i, NrNotes = 0, Len, Max = *maxlength, j, k=0;

	GSM_RingNote 		*Note;	

	GSM_RingNoteScale	DefNoteScale;
	GSM_RingNoteStyle	DefNoteStyle=0;
	int			DefNoteTempo=0;
	unsigned char 		buffer[15];

	bool 			started = false;

	*maxlength=sprintf(package,"BEGIN:IMELODY%c%c",13,10);
	if (version == 1.0) {
		*maxlength+=sprintf(package+(*maxlength),"VERSION:1.0%c%c",13,10);
	} else {
		*maxlength+=sprintf(package+(*maxlength),"VERSION:1.2%c%c",13,10);
	}

	*maxlength+=sprintf(package+(*maxlength),"FORMAT:CLASS1.0%c%c",13,10);

	if (version == 1.2) {
		*maxlength+=sprintf(package+(*maxlength),"NAME:%s%c%c",DecodeUnicodeString(ringtone.Name),13,10);

		/* Find the most frequently used style */ 
		for (i=0;i<3;i++) buffer[i]=0;
		for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
			if (ringtone.NoteTone.Commands[i].Type == RING_Note) {
				buffer[ringtone.NoteTone.Commands[i].Note.Style]++; // Styles: 0;1;2
			}
		}
		/* Now find the most frequently used */
		j=0;
		for (i=0;i<3;i++) {
			if (buffer[i]>j) {
				k=i; 
				j=buffer[i];
	          	}
		}
		DefNoteStyle = k; /* Save default style */
	}

	DefNoteScale = Scale_880; /* by iMelody definition */

	for (i=0;i<ringtone.NoteTone.NrCommands;i++) {
		if (ringtone.NoteTone.Commands[i].Type == RING_Note) {
			Note = &ringtone.NoteTone.Commands[i].Note;
			Len  = *maxlength;
			if (!started) {
				if (version == 1.2) {
					DefNoteTempo=Note->Tempo;
					/* Save the default tempo */
					Len+=sprintf(package+Len,"BEAT:%i%c%c",DefNoteTempo,13,10);
					dprintf("DefNoteTempo=%d\n",DefNoteTempo);
				
					/* Save default style */
					switch (DefNoteStyle) {
						case NaturalStyle   :Len+=sprintf(package+Len,"STYLE:S0%c%c",13,10); break;
						case ContinuousStyle:Len+=sprintf(package+Len,"STYLE:S1%c%c",13,10); break;
						case StaccatoStyle  :Len+=sprintf(package+Len,"STYLE:S2%c%c",13,10); break;
					}
				}
				Len+=sprintf(package+Len,"MELODY:");
				started = true;
			}

			if (Note->Note!=Note_Pause && Note->Scale != DefNoteScale)
			{
				Len+=sprintf(package+Len,"*%i",Note->Scale-1);
			}
			switch (Note->Note) {
				case Note_C  	:Len+=sprintf(package+Len,"c");	break;
				case Note_Cis	:Len+=sprintf(package+Len,"#c");break;
				case Note_D  	:Len+=sprintf(package+Len,"d");	break;
				case Note_Dis	:Len+=sprintf(package+Len,"#d");break;
				case Note_E  	:Len+=sprintf(package+Len,"e");	break;
				case Note_F  	:Len+=sprintf(package+Len,"f");	break;
				case Note_Fis	:Len+=sprintf(package+Len,"#f");break;
				case Note_G  	:Len+=sprintf(package+Len,"g");	break;
				case Note_Gis	:Len+=sprintf(package+Len,"#g");break;
				case Note_A  	:Len+=sprintf(package+Len,"a");	break;
				case Note_Ais	:Len+=sprintf(package+Len,"#a");break;
				case Note_H  	:Len+=sprintf(package+Len,"b");	break;
				case Note_Pause	:Len+=sprintf(package+Len,"r");	break;
			}
			switch (Note->Duration) {
				case Duration_Full : package[Len++]='0';	break;
				case Duration_1_2  : package[Len++]='1';	break;
				case Duration_1_4  : package[Len++]='2';	break;
				case Duration_1_8  : package[Len++]='3';	break;
				case Duration_1_16 : package[Len++]='4';	break;
				case Duration_1_32 : package[Len++]='5';	break;
				default		   :				break;
			}
			switch (Note->DurationSpec) {
				case DottedNote		: package[Len++] = '.'; break;
				case DoubleDottedNote	: package[Len++] = ':'; break;
				case Length_2_3		: package[Len++] = ';'; break;
				default		   	:			break;
			}
			/* 15 = Len of END:IMELODY... */
			if ((Len+15) > Max) break;
			*maxlength = Len;
		}
		NrNotes ++;
	}
	*maxlength+=sprintf(package+(*maxlength),"%c%cEND:IMELODY%c%c",13,10,13,10);
	return NrNotes;
}

char *GSM_GetRingtoneName(GSM_AllRingtonesInfo *Info, int ID)
{
	int 		i;
	static char 	ala[2];

	for (i=0;i<Info->Number;i++) {
		if (Info->Ringtone[i].ID == ID) return Info->Ringtone[i].Name;
	}

	ala[0] = 0;
	ala[1] = 0;
	return ala;
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
