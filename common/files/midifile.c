/* a little modified code from http://iki.fi/too/sw/xring/ */

/* embedding modified midifile.h and midifile.c into this file */

/***** midifile.h ******/

struct MF {
/* definitions for MIDI file parsing code */
  int (*Mf_getc)(struct MF *);
  void (*Mf_header)(struct MF *, int, int, int);
  void (*Mf_trackstart)(struct MF *);
  void (*Mf_trackend)(struct MF *);
  void (*Mf_noteon)(struct MF *, int, int, int);
  void (*Mf_noteoff)(struct MF *, int, int, int);
  void (*Mf_pressure)(struct MF *, int, int, int);
  void (*Mf_parameter)(struct MF *, int, int, int);
  void (*Mf_pitchbend)(struct MF *, int, int, int);
  void (*Mf_program)(struct MF *, int, int);
  void (*Mf_chanpressure)(struct MF *, int, int);
  void (*Mf_sysex)(struct MF *, int, char *);
  void (*Mf_metamisc)(struct MF *, int, int, char * );
  void (*Mf_seqspecific)(struct MF *, int, int, char *);
  void (*Mf_seqnum)(struct MF *, int);
  void (*Mf_text)(struct MF *, int, int, char *);
  void (*Mf_eot)(struct MF *);
  void (*Mf_timesig)(struct MF *, int, int, int, int);
  void (*Mf_smpte)(struct MF *, int, int, int, int, int);
  void (*Mf_tempo)(struct MF *, long);
  void (*Mf_keysig)(struct MF *, int, int);
  void (*Mf_arbitrary)(struct MF *, int, char *);
  void (*Mf_error)(struct MF *, char * );
#if 0
/* definitions for MIDI file writing code */
  void (*Mf_putc)(struct MF *);
  void (*Mf_writetrack)(struct MF *);
  void (*Mf_writetempotrack)(struct MF *);
#endif
  /* variables */
  int Mf_nomerge;             /* 1 => continue'ed system exclusives are */
  					/* not collapsed. */
  long Mf_currtime;          /* current time in delta-time units */

/* private stuff */
  long Mf_toberead;
  long Mf_numbyteswritten;

  char *Msgbuff;	/* message buffer */
  int Msgsize;		/* Size of currently allocated Msg */
  int Msgindex;		/* index of next available location in Msg */

};

float mf_ticks2sec(unsigned long ticks,int division,unsigned int tempo);
unsigned long mf_sec2ticks(float secs,int division, unsigned int tempo);

void mferror(struct MF * mf, char * s);

/*void mfwrite(); */


/* MIDI status commands most significant bit is 1 */
#define note_off         	0x80
#define note_on          	0x90
#define poly_aftertouch  	0xa0
#define control_change    	0xb0
#define program_chng     	0xc0
#define channel_aftertouch      0xd0
#define pitch_wheel      	0xe0
#define system_exclusive      	0xf0
#define delay_packet	 	(1111)

/* 7 bit controllers */
#define damper_pedal            0x40
#define portamento	        0x41 	
#define sostenuto	        0x42
#define soft_pedal	        0x43
#define general_4               0x44
#define	hold_2		        0x45
#define	general_5	        0x50
#define	general_6	        0x51
#define general_7	        0x52
#define general_8	        0x53
#define tremolo_depth	        0x5c
#define chorus_depth	        0x5d
#define	detune		        0x5e
#define phaser_depth	        0x5f

/* parameter values */
#define data_inc	        0x60
#define data_dec	        0x61

/* parameter selection */
#define non_reg_lsb	        0x62
#define non_reg_msb	        0x63
#define reg_lsb		        0x64
#define reg_msb		        0x65

/* Standard MIDI Files meta event definitions */
#define	meta_event		0xFF
#define	sequence_number 	0x00
#define	text_event		0x01
#define copyright_notice 	0x02
#define sequence_name    	0x03
#define instrument_name 	0x04
#define lyric	        	0x05
#define marker			0x06
#define	cue_point		0x07
#define channel_prefix		0x20
#define	end_of_track		0x2f
#define	set_tempo		0x51
#define	smpte_offset		0x54
#define	time_signature		0x58
#define	key_signature		0x59
#define	sequencer_specific	0x74

/* Manufacturer's ID number */
#define Seq_Circuits (0x01) /* Sequential Circuits Inc. */
#define Big_Briar    (0x02) /* Big Briar Inc.           */
#define Octave       (0x03) /* Octave/Plateau           */
#define Moog         (0x04) /* Moog Music               */
#define Passport     (0x05) /* Passport Designs         */
#define Lexicon      (0x06) /* Lexicon 			*/
#define Tempi        (0x20) /* Bon Tempi                */
#define Siel         (0x21) /* S.I.E.L.                 */
#define Kawai        (0x41) 
#define Roland       (0x42)
#define Korg         (0x42)
#define Yamaha       (0x43)

/* miscellaneous definitions */
#define MThd 0x4d546864
#define MTrk 0x4d54726b
#define lowerbyte(x) ((unsigned char)(x & 0xff))
#define upperbyte(x) ((unsigned char)((x & 0xff00)>>8))

/* the midifile interface */
void midifile(struct MF * mf);

/***** midifile.c ******/

/*
 * midifile 1.11
 * 
 * Read and write a MIDI file.  Externally-assigned function pointers are 
 * called upon recognizing things in the file.
 *
 * Original release ?
 * June 1989 - Added writing capability, M. Czeiszperger.
 *
 *          The file format implemented here is called
 *          Standard MIDI Files, and is part of the Musical
 *          instrument Digital Interface specification.
 *          The spec is avaiable from:
 *
 *               International MIDI Association
 *               5316 West 57th Street
 *               Los Angeles, CA 90056
 *
 *          An in-depth description of the spec can also be found
 *          in the article "Introducing Standard MIDI Files", published
 *          in Electronic Musician magazine, April, 1989.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#ifdef WIN32

  #include <windows.h>

#else

  #include <unistd.h>
#endif

#include "gsm-common.h"
#include "gsm-ringtones.h"

#define NULLFUNC NULL

static void readheader(struct MF * mf);
static int readtrack(struct MF * mf);
static void chanmessage(struct MF * mf,int status,int c1,int c2);
static void msginit(struct MF * mf);
static void msgadd(struct MF * mf,int c);
static void metaevent(struct MF * mf, int type);
static void sysex(struct MF * mf);
static int msgleng(struct MF * mf);
static void badbyte(struct MF * mf,int c);
static void biggermsg(struct MF * mf);


static long readvarinum(struct MF * mf);
static long read32bit(struct MF * mf);
static long to32bit(int, int, int, int);
static int read16bit(struct MF * mf);
static int to16bit(int, int);
static char * msg(struct MF * mf);

/* The only non-static function in this file. */
void mfread(struct MF * mf)
{
	if ( mf->Mf_getc == NULLFUNC )
		mferror(mf, "mfread() called without setting Mf_getc"); 

	readheader(mf);
	while ( readtrack(mf) )
		;
}

/* for backward compatibility with the original lib */
void midifile(struct MF * mf)
{
    mfread(mf);
}

/* read through the "MThd" or "MTrk" header string */
static int readmt(struct MF * mf, char * s)
{
	int n = 0;
	char *p = s;
	int c=0;

	while ( n++<4 && (c=mf->Mf_getc(mf)) != EOF ) {
		if ( c != *p++ ) {
			char buff[32];
			(void) strcpy(buff, "expecting ");
			(void) strcat(buff, s);
			mferror(mf, buff);
		}
	}
	return c;
}

/* read a single character and abort on EOF */
static int egetc(struct MF * mf)
{
	int c = mf->Mf_getc(mf);

	if ( c == EOF )
		mferror(mf, "premature EOF");
	mf->Mf_toberead--;
	return c;
}

/* read a header chunk */
static void readheader(struct MF * mf)
{
	int format, ntrks, division;

	if ( readmt(mf, "MThd") == EOF )
		return;

	mf->Mf_toberead = read32bit(mf);
	format = read16bit(mf);
	ntrks = read16bit(mf);
	division = read16bit(mf);

	if ( mf->Mf_header )
		(*mf->Mf_header)(mf, format,ntrks,division);

	/* flush any extra stuff, in case the length of header is not 6 */
	while ( mf->Mf_toberead > 0 )
		(void) egetc(mf);
}

static int readtrack(struct MF * mf)		 /* read a track chunk */
{
	/* This array is indexed by the high half of a status byte.  It's */
	/* value is either the number of bytes needed (1 or 2) for a channel */
	/* message, or 0 (meaning it's not  a channel message). */
	static int chantype[] = {
		0, 0, 0, 0, 0, 0, 0, 0,		/* 0x00 through 0x70 */
		2, 2, 2, 2, 1, 1, 2, 0		/* 0x80 through 0xf0 */
	};
	long lookfor;
	int c, c1, type;
	int sysexcontinue = 0;	/* 1 if last message was an unfinished sysex */
	int running = 0;	/* 1 when running status used */
	int status = 0;		/* status value (e.g. 0x90==note-on) */
	int needed;

	if ( readmt(mf, "MTrk") == EOF )
		return(0);

	mf->Mf_toberead = read32bit(mf);
	mf->Mf_currtime = 0;

	if ( mf->Mf_trackstart )
		(*mf->Mf_trackstart)(mf);

	while ( mf->Mf_toberead > 0 ) {

		mf->Mf_currtime += readvarinum(mf);	/* delta time */

		c = egetc(mf);

		if ( sysexcontinue && c != 0xf7 )
			mferror(mf, "didn't find expected continuation of a sysex");

		if ( (c & 0x80) == 0 ) {	 /* running status? */
			if ( status == 0 )
				mferror(mf, "unexpected running status");
			running = 1;
		}
		else {
			status = c;
			running = 0;
		}

		needed = chantype[ (status>>4) & 0xf ];

		if ( needed ) {		/* ie. is it a channel message? */

			if ( running )
				c1 = c;
			else
				c1 = egetc(mf);
			chanmessage(mf, status, c1, (needed>1)? egetc(mf): 0);
			continue;;
		}

		switch ( c ) {

		case 0xff:			/* meta event */

			type = egetc(mf);
			lookfor = mf->Mf_toberead - readvarinum(mf);
			msginit(mf);

			while ( mf->Mf_toberead > lookfor )
				msgadd(mf, egetc(mf));

			metaevent(mf, type);
			break;

		case 0xf0:		/* start of system exclusive */

			lookfor = mf->Mf_toberead - readvarinum(mf);
			msginit(mf);
			msgadd(mf, 0xf0);

			while ( mf->Mf_toberead > lookfor )
				msgadd(mf, c=egetc(mf));

			if ( c==0xf7 || mf->Mf_nomerge==0 )
				sysex(mf);
			else
				sysexcontinue = 1;  /* merge into next msg */
			break;

		case 0xf7:	/* sysex continuation or arbitrary stuff */

			lookfor = mf->Mf_toberead - readvarinum(mf);

			if ( ! sysexcontinue )
				msginit(mf);

			while ( mf->Mf_toberead > lookfor )
				msgadd(mf, c=egetc(mf));

			if ( ! sysexcontinue ) {
				if ( mf->Mf_arbitrary )
					(*mf->Mf_arbitrary)(mf, msgleng(mf),msg(mf));
			}
			else if ( c == 0xf7 ) {
				sysex(mf);
				sysexcontinue = 0;
			}
			break;
		default:
			badbyte(mf, c);
			break;
		}
	}
	if ( mf->Mf_trackend )
		(*mf->Mf_trackend)(mf);
	return(1);
}

static void badbyte(struct MF * mf,int c)
{
	char buff[32];

	(void) sprintf(buff,"unexpected byte: 0x%02x",c);
	mferror(mf, buff);
}

static void metaevent(struct MF * mf, int type)
{
	int leng = msgleng(mf);
	char *m = msg(mf);

	switch  ( type ) {
	case 0x00:
		if ( mf->Mf_seqnum )
			(*mf->Mf_seqnum)(mf, to16bit(m[0],m[1]));
		break;
	case 0x01:	/* Text event */
	case 0x02:	/* Copyright notice */
	case 0x03:	/* Sequence/Track name */
	case 0x04:	/* Instrument name */
	case 0x05:	/* Lyric */
	case 0x06:	/* Marker */
	case 0x07:	/* Cue point */
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
		/* These are all text events */
		if ( mf->Mf_text )
			(*mf->Mf_text)(mf, type,leng,m);
		break;
	case 0x2f:	/* End of Track */
		if ( mf->Mf_eot )
			(*mf->Mf_eot)(mf);
		break;
	case 0x51:	/* Set tempo */
		if ( mf->Mf_tempo )
			(*mf->Mf_tempo)(mf, to32bit(0,m[0],m[1],m[2]));
		break;
	case 0x54:
		if ( mf->Mf_smpte )
			(*mf->Mf_smpte)(mf, m[0],m[1],m[2],m[3],m[4]);
		break;
	case 0x58:
		if ( mf->Mf_timesig )
			(*mf->Mf_timesig)(mf, m[0],m[1],m[2],m[3]);
		break;
	case 0x59:
		if ( mf->Mf_keysig )
			(*mf->Mf_keysig)(mf, m[0],m[1]);
		break;
	case 0x7f:
		if ( mf->Mf_seqspecific )
			(*mf->Mf_seqspecific)(mf, type, leng, m);
		break;
	default:
		if ( mf->Mf_metamisc )
			(*mf->Mf_metamisc)(mf, type,leng,m);
	}
}

static void sysex(struct MF * mf)
{
	if ( mf->Mf_sysex )
		(*mf->Mf_sysex)(mf, msgleng(mf),msg(mf));
}

static void chanmessage(struct MF * mf,int status,int c1,int c2)
{
	int chan = status & 0xf;

	switch ( status & 0xf0 ) {
	case 0x80:
		if ( mf->Mf_noteoff )
			(*mf->Mf_noteoff)(mf, chan,c1,c2);
		break;
	case 0x90:
		if ( mf->Mf_noteon )
			(*mf->Mf_noteon)(mf, chan,c1,c2);
		break;
	case 0xa0:
		if ( mf->Mf_pressure )
			(*mf->Mf_pressure)(mf, chan,c1,c2);
		break;
	case 0xb0:
		if ( mf->Mf_parameter )
			(*mf->Mf_parameter)(mf, chan,c1,c2);
		break;
	case 0xe0:
		if ( mf->Mf_pitchbend )
			(*mf->Mf_pitchbend)(mf, chan,c1,c2);
		break;
	case 0xc0:
		if ( mf->Mf_program )
			(*mf->Mf_program)(mf, chan,c1);
		break;
	case 0xd0:
		if ( mf->Mf_chanpressure )
			(*mf->Mf_chanpressure)(mf, chan,c1);
		break;
	}
}

/* readvarinum - read a varying-length number, and return the */
/* number of characters it took. */

static long readvarinum(struct MF * mf)
{
	long value;
	int c;

	c = egetc(mf);
	value = c;
	if ( c & 0x80 ) {
		value &= 0x7f;
		do {
			c = egetc(mf);
			value = (value << 7) + (c & 0x7f);
		} while (c & 0x80);
	}
	return (value);
}

static long to32bit(int c1,int c2,int c3,int c4)
{
	long value = 0L;

	value = (c1 & 0xff);
	value = (value<<8) + (c2 & 0xff);
	value = (value<<8) + (c3 & 0xff);
	value = (value<<8) + (c4 & 0xff);
	return (value);
}

static int to16bit(int c1,int c2)
{
	return ((c1 & 0xff ) << 8) + (c2 & 0xff);
}

static long read32bit(struct MF * mf)
{
	int c1, c2, c3, c4;

	c1 = egetc(mf);
	c2 = egetc(mf);
	c3 = egetc(mf);
	c4 = egetc(mf);
	return to32bit(c1,c2,c3,c4);
}

static int read16bit(struct MF * mf)
{
	int c1, c2;
	c1 = egetc(mf);
	c2 = egetc(mf);
	return to16bit(c1,c2);
}

/* static */
void mferror(struct MF * mf, char * s)
{
	if ( mf->Mf_error )
		(*mf->Mf_error)(mf, s);
	exit(1);
}

/* The code below allows collection of a system exclusive message of */
/* arbitrary length.  The Msgbuff is expanded as necessary.  The only */
/* visible data/routines are msginit(), msgadd(), msg(), msgleng(). */

#define MSGINCREMENT 128

static void msginit(struct MF * mf)
{
	mf->Msgindex = 0;
}

static char * msg(struct MF * mf)
{
	return(mf->Msgbuff);
}

static int msgleng(struct MF * mf)
{
	return(mf->Msgindex);
}

static void msgadd(struct MF * mf,int c)
{
	/* If necessary, allocate larger message buffer. */
	if ( mf->Msgindex >= mf->Msgsize )
		biggermsg(mf);
	mf->Msgbuff[mf->Msgindex++] = c;
}

static void biggermsg(struct MF * mf)
{
	char *newmess;
	char *oldmess = mf->Msgbuff;
	int oldleng = mf->Msgsize;

	mf->Msgsize += MSGINCREMENT;
	newmess = (char *) malloc( (unsigned)(sizeof(char) * mf->Msgsize) );

	if(newmess == NULL)
		mferror(mf, "malloc error!");
		
	/* copy old message into larger new one */
	if ( oldmess != NULL ) {
		register char *p = newmess;
		register char *q = oldmess;
		register char *endq = &oldmess[oldleng];

		for ( ; q!=endq ; p++,q++ )
			*p = *q;
		free(oldmess);
	}
	mf->Msgbuff = newmess;
}

#if 0 /* saving time not converting write function at this time
       */
/*
 * mfwrite() - The only fuction you'll need to call to write out
 *             a midi file.
 *
 * format      0 - Single multi-channel track
 *             1 - Multiple simultaneous tracks
 *             2 - One or more sequentially independent
 *                 single track patterns                
 * ntracks     The number of tracks in the file.
 * division    This is kind of tricky, it can represent two
 *             things, depending on whether it is positive or negative
 *             (bit 15 set or not).  If  bit  15  of division  is zero,
 *             bits 14 through 0 represent the number of delta-time
 *             "ticks" which make up a quarter note.  If bit  15 of
 *             division  is  a one, delta-times in a file correspond to
 *             subdivisions of a second similiar to  SMPTE  and  MIDI
 *             time code.  In  this format bits 14 through 8 contain
 *             one of four values - 24, -25, -29, or -30,
 *             corresponding  to  the  four standard  SMPTE and MIDI
 *             time code frame per second formats, where  -29
 *             represents  30  drop  frame.   The  second  byte
 *             consisting  of  bits 7 through 0 corresponds the the
 *             resolution within a frame.  Refer the Standard MIDI
 *             Files 1.0 spec for more details.
 * fp          This should be the open file pointer to the file you
 *             want to write.  It will have be a global in order
 *             to work with Mf_putc.  
 */ 
void 
mfwrite(format,ntracks,division,fp) 
int format,ntracks,division; 
FILE *fp; 
{
    int i; void mf_write_track_chunk(), mf_write_header_chunk();

    if ( mf->Mf_putc == NULLFUNC )
	    mferror(mf, "mfmf_write() called without setting Mf_putc");

    if ( mf->Mf_writetrack == NULLFUNC )
	    mferror(mf, "mfmf_write() called without setting Mf_mf_writetrack"); 

    /* every MIDI file starts with a header */
    mf_write_header_chunk(format,ntracks,division);

    /* In format 1 files, the first track is a tempo map */
    if(format == 1 && ( mf->Mf_writetempotrack ))
    {
	(*mf->Mf_writetempotrack)();
    }

    /* The rest of the file is a series of tracks */
    for(i = 0; i < ntracks; i++)
        mf_write_track_chunk(i,fp);
}

void 
mf_write_track_chunk(which_track,fp)
int which_track;
FILE *fp;
{
	unsigned long trkhdr,trklength;
	long offset, place_marker;
	void write16bit(),write32bit();
	
	
	trkhdr = MTrk;
	trklength = 0;

	/* Remember where the length was written, because we don't
	   know how long it will be until we've finished writing */
	offset = ftell(fp); 

#ifdef DEBUG
        printf("offset = %d\n",(int) offset);
#endif

	/* Write the track chunk header */
	write32bit(trkhdr);
	write32bit(trklength);

	mf->Mf_numbyteswritten = 0L; /* the header's length doesn't count */

	if( mf->Mf_writetrack )
	{
	    (*mf->Mf_writetrack)(which_track);
	}

	/* mf_write End of track meta event */
	eputc(mf, 0);
	eputc(mf, meta_event);
	eputc(mf, end_of_track);

 	eputc(mf, 0);
	 
	/* It's impossible to know how long the track chunk will be beforehand,
           so the position of the track length data is kept so that it can
           be written after the chunk has been generated */
	place_marker = ftell(fp);
	
	/* This method turned out not to be portable because the
           parameter returned from ftell is not guaranteed to be
           in bytes on every machine */
 	/* track.length = place_marker - offset - (long) sizeof(track); */

#ifdef DEBUG
printf("length = %d\n",(int) trklength);
#endif

 	if(fseek(fp,offset,0) < 0)
	    mferror(mf, "error seeking during final stage of write");

	trklength = mf->Mf_numbyteswritten;

	/* Re-mf_write the track chunk header with right length */
	write32bit(trkhdr);
	write32bit(trklength);

	fseek(fp,place_marker,0);
} /* End gen_track_chunk() */


void 
mf_write_header_chunk(format,ntracks,division)
int format,ntracks,division;
{
    unsigned long ident,length;
    void write16bit(),write32bit();
    
    ident = MThd;           /* Head chunk identifier                    */
    length = 6;             /* Chunk length                             */

    /* individual bytes of the header must be written separately
       to preserve byte order across cpu types :-( */
    write32bit(ident);
    write32bit(length);
    write16bit(format);
    write16bit(ntracks);
    write16bit(division);
} /* end gen_header_chunk() */


#ifdef WHENISTHISNEEDED
/*
 * mf_write_midi_event()
 * 
 * Library routine to mf_write a single MIDI track event in the standard MIDI
 * file format. The format is:
 *
 *                    <delta-time><event>
 *
 * In this case, event can be any multi-byte midi message, such as
 * "note on", "note off", etc.      
 *
 * delta_time - the time in ticks since the last event.
 * type - the type of meta event.
 * chan - The midi channel.
 * data - A pointer to a block of chars containing the META EVENT,
 *        data.
 * size - The length of the meta-event data.
 */
int 
mf_write_midi_event(delta_time, type, chan, data, size)
unsigned long delta_time;
unsigned int chan,type;
unsigned long size;
unsigned char *data;
{
    int i;
    void WriteVarLen();
    unsigned char c;

    WriteVarLen(delta_time);

    /* all MIDI events start with the type in the first four bits,
       and the channel in the lower four bits */
    c = type | chan;

    if(chan > 15)
        perror("error: MIDI channel greater than 16\n");

    eputc(mf, c);

    /* write out the data bytes */
    for(i = 0; i < size; i++)
	eputc(mf, data[i]);

    return(size);
} /* end mf_write MIDI event */

/*
 * mf_write_meta_event()
 *
 * Library routine to mf_write a single meta event in the standard MIDI
 * file format. The format of a meta event is:
 *
 *          <delta-time><FF><type><length><bytes>
 *
 * delta_time - the time in ticks since the last event.
 * type - the type of meta event.
 * data - A pointer to a block of chars containing the META EVENT,
 *        data.
 * size - The length of the meta-event data.
 */
int
mf_write_meta_event(delta_time, type, data, size)
unsigned long delta_time;
unsigned char *data,type;
unsigned long size;
{
    int i;

    WriteVarLen(delta_time);
    
    /* This marks the fact we're writing a meta-event */
    eputc(mf, meta_event);

    /* The type of meta event */
    eputc(mf, type);

    /* The length of the data bytes to follow */
    WriteVarLen(size); 

    for(i = 0; i < size; i++)
    {
	if(eputc(mf, data[i]) != data[i])
	    return(-1); 
    }
    return(size);
} /* end mf_write_meta_event */

void 
mf_write_tempo(tempo)
unsigned long tempo;
{
    /* Write tempo */
    /* all tempos are written as 120 beats/minute, */
    /* expressed in microseconds/quarter note     */
    eputc(mf, 0);
    eputc(mf, meta_event);
    eputc(mf, set_tempo);

    eputc(mf, 3);
    eputc(mf, (unsigned)(0xff & (tempo >> 16)));
    eputc(mf, (unsigned)(0xff & (tempo >> 8)));
    eputc(mf, (unsigned)(0xff & tempo));
}

#endif
/*
 * Write multi-length bytes to MIDI format files
 */
void 
WriteVarLen(value)
unsigned long value;
{
  unsigned long buffer;

  buffer = value & 0x7f;
  while((value >>= 7) > 0)
  {
	buffer <<= 8;
	buffer |= 0x80;
	buffer += (value & 0x7f);
  }
  while(1){
       eputc(mf, (unsigned)(buffer & 0xff));
       
	if(buffer & 0x80)
		buffer >>= 8;
	else
		return;
	}
}/* end of WriteVarLen */


/*
 * write32bit()
 * write16bit()
 *
 * These routines are used to make sure that the byte order of
 * the various data types remains constant between machines. This
 * helps make sure that the code will be portable from one system
 * to the next.  It is slightly dangerous that it assumes that longs
 * have at least 32 bits and ints have at least 16 bits, but this
 * has been true at least on PCs, UNIX machines, and Macintosh's.
 *
 */
void 
write32bit(data)
unsigned long data;
{
    eputc(mf, x(unsigned)((data >> 24) & 0xff));
    eputc(mf, (unsigned)((data >> 16) & 0xff));
    eputc(mf, (unsigned)((data >> 8 ) & 0xff));
    eputc(mf, (unsigned)(data & 0xff));
}

void 
write16bit(data)
int data;
{
    eputc(mf, (unsigned)((data & 0xff00) >> 8));
    eputc(mf, (unsigned)(data & 0xff));
}

/* write a single character and abort on error */
eputc(mf, c)			
unsigned char c;
{
	int return_val;
	
	if((mf->Mf_putc) == NULLFUNC)
	{
		mferror(mf, "Mf_putc undefined");
		return(-1);
	}
	
	return_val = (mf->Mf_putc)(mf, c);

	if ( return_val == EOF )
		mferror(mf, "error writing");
		
	mf->Mf_numbyteswritten++;
	return(return_val);
}

#endif

unsigned long mf_sec2ticks(float secs,int division, unsigned int tempo)
{    
     return (long)(((secs * 1000.0) / 4.0 * division) / tempo);
}


/* 
 * This routine converts delta times in ticks into seconds. The
 * else statement is needed because the formula is different for tracks
 * based on notes and tracks based on SMPTE times.
 *
 */
float mf_ticks2sec(unsigned long ticks,int division,unsigned int tempo)
{
    float smpte_format, smpte_resolution;

    if(division > 0)
        return ((float) (((float)(ticks) * (float)(tempo)) / ((float)(division) * 1000000.0)));
    else
    {
       smpte_format = upperbyte(division);
       smpte_resolution = lowerbyte(division);
       return (float) ((float) ticks / (smpte_format * smpte_resolution * 1000000.0));
    }
} /* end of ticks2sec() */

/* code to utilize the interface */

#define TRACE(x, y) do { if (x) printf y; } while (0)


typedef unsigned long ulg;
typedef unsigned char uch;
typedef unsigned int  ui;

struct NoteInfo
{
  int beats;
  int nrnotes;
  struct Notes {
    ui note : 4;
    ui scale : 4;
    ui length : 4;
    ui lextra : 4;
  } note[1];
};

#define IBUFSIZE 1024
struct MFX
{
  struct MF mfi;
  struct NoteInfo * ni;
  int allocated;

  int division;
  int trackstate;

  int prevnoteonpitch; /* -1, nothing, 0 pause, 1-x note. */
  ulg prevnoteontime;

  struct {
    int fd;
    uch buf[IBUFSIZE];
    int len;
    int p;
  } istrm;
};

enum { TRK_NONE, TRK_READING, TRK_FINISHED };

#define ALLOCSIZE 256

#define NIALLOC(size) (struct NoteInfo *)malloc(sizeof (struct NoteInfo) + ((size) - 1) * sizeof (struct Notes))

#define NIREALLOC(ni, size) (struct NoteInfo *)realloc((ni), sizeof (struct NoteInfo) + ((size) - 1) * sizeof (struct Notes))


static void lm_error(struct MF * mf, char * s);

static int  lm_getc(struct MF * mf);
static void lm_header(struct MF * mf, int, int, int);
static void lm_trackstart(struct MF * mf);
static void lm_trackend(struct MF * mf);
static void lm_tempo(struct MF *, long);
static void lm_noteon(struct MF *, int, int, int);
static void lm_noteoff(struct MF *, int, int, int);


struct NoteInfo * readmidi(int fd)
{
  struct MFX mfxi = { { 0 } };
  struct MF * mf = (struct MF *)&mfxi;
  
  mfxi.ni = NIALLOC(ALLOCSIZE);
  mfxi.allocated = ALLOCSIZE;

  /* set variables to their initial values */
  mfxi.division = 0;
  mfxi.trackstate = TRK_NONE;
  mfxi.prevnoteonpitch = -1;
  mfxi.ni->nrnotes = 0;
  mfxi.ni->beats = 120; 

  mfxi.istrm.fd = fd;
  mfxi.istrm.p = mfxi.istrm.len = 0;
  mf->Mf_getc = lm_getc;
  
  mf->Mf_header = lm_header;
  mf->Mf_tempo = lm_tempo;
  mf->Mf_trackstart = lm_trackstart;
  mf->Mf_trackend = lm_trackend;
  mf->Mf_noteon = lm_noteon;
  mf->Mf_noteoff = lm_noteoff;

  mf->Mf_error = lm_error;

  midifile(mf);

  return mfxi.ni;
}

static void lm_error(struct MF * mf, char * s)
{
  fprintf(stderr, "%s\n", s);
}

static int lm_getc(struct MF * mf)
{
  struct MFX * mfx = (struct MFX *)mf;

  /* printf("p %d, len %d\n", mfx->istrm.p, mfx->istrm.len); */
  if (mfx->istrm.p == mfx->istrm.len)
  {
    mfx->istrm.len = read(mfx->istrm.fd, mfx->istrm.buf, IBUFSIZE);
    /* printf("readlen %d\n", mfx->istrm.len); */
    if (mfx->istrm.len <= 0)
      return -1;

    mfx->istrm.p = 1;
    return mfx->istrm.buf[0];
  }
  /* else */
  return mfx->istrm.buf[mfx->istrm.p++];
}

static void lm_header(struct MF * mf, int format, int ntrks, int division)
{
  struct MFX * mfx = (struct MFX *)mf;

  TRACE(0, ("lm_header(%p, %d, %d, %d)\n", mf, format, ntrks, division));

  mfx->division = division;
}

/* this is just a quess */
static void lm_tempo(struct MF * mf, long tempo)
{
  struct MFX * mfx = (struct MFX *)mf;

  TRACE(0, ("lm_tempo(%p, %ld)\n", mf, tempo));

  if (mfx->trackstate != TRK_FINISHED)
    mfx->ni->beats = 60000000 / tempo;
}


static void addnote(struct MFX * mfx, int pitch, int duration, int special)
{
  int nr, p, s;
  struct NoteInfo * ni;
  
  if (mfx->ni->nrnotes == mfx->allocated)
  {
    mfx->allocated += ALLOCSIZE;
    mfx->ni = NIREALLOC(mfx->ni, mfx->allocated);
    if (mfx->ni == NULL)
      exit(1);
  }
  ni = mfx->ni; /* mfx->ni pointer value may have changed above */
  nr = ni->nrnotes++;

  
  if (pitch == 0)  { p = 0; s = 0; }
  else  { pitch--; p = (pitch % 12) + 1; s = pitch / 12; }
  
  ni->note[nr].note = p;
  ni->note[nr].scale = s;

  ni->note[nr].length = duration;
  ni->note[nr].lextra = special;
}
  
/* currently supported */
static /*     N  32  32.  16  16.   8    8.   4    4.   2    2.   1     1.  */
int vals[] = { 15, 38,  54, 78,  109, 156, 218, 312, 437, 625, 875, 1250 };

static void writenote(struct MFX * mfx, int delta)
{
  ulg millinotetime = delta * 250 / mfx->division;
  int i;
  int duration;
  int special;
  
  for(i = 0; i < sizeof vals / sizeof vals[0]; i++)
  {
    if (millinotetime < vals[i])
      break;
  }

  if (i == 0)
    return;

  i--;
  duration = i / 2;
  special = i & 1;

  addnote(mfx, mfx->prevnoteonpitch, duration, special); /* XXX think this */
}
  

static void lm_trackstart(struct MF * mf)
{
  struct MFX * mfx = (struct MFX *)mf;

  TRACE(0, ("lm_trackstart(%p)\n", mf));

  if (mfx->trackstate == TRK_NONE)
    mfx->trackstate = TRK_READING;

  mfx->prevnoteonpitch = -1;
}

static void lm_trackend(struct MF * mf)
{
  struct MFX * mfx = (struct MFX *)mf;
  long time = mf->Mf_currtime;

  TRACE(0, ("lm_trackend(%p)\n", mf));

  if (mfx->trackstate == TRK_READING && mfx->ni->nrnotes > 0)
    mfx->trackstate = TRK_FINISHED;

  if (mfx->prevnoteonpitch >= 0)
    writenote(mfx, time - mfx->prevnoteontime);
  
  mfx->prevnoteonpitch = -1;
}

static void lm_noteon(struct MF * mf, int chan, int pitch, int vol)
{
  struct MFX * mfx = (struct MFX *)mf;
  long time = mf->Mf_currtime;

  TRACE(0, ("lm_noteon(%p, %d, %d, %d)\n", mf, chan, pitch, vol));

  if (vol == 0) /* kludge? to handle some (format 1? midi files) */
    return; 
  
  if (mfx->trackstate != TRK_READING)
    return;
  
  if (mfx->prevnoteonpitch >= 0)
    writenote(mfx, time - mfx->prevnoteontime);

  if (vol == 0)
    mfx->prevnoteonpitch = 0;
  else
    mfx->prevnoteonpitch = pitch + 1;
  
  mfx->prevnoteontime = time;
}

static void lm_noteoff(struct MF * mf, int chan, int pitch, int vol)
{
  struct MFX * mfx = (struct MFX *)mf;
  long time = mf->Mf_currtime;
  
  TRACE(0, ("lm_noteoff(%p, %d, %d, %d)\n", mf, chan, pitch, vol));

  if (mfx->prevnoteonpitch >= 0)
  {
    writenote(mfx, time - mfx->prevnoteontime);
    mfx->prevnoteonpitch = -1;
  }
  mfx->prevnoteonpitch = 0;
  mfx->prevnoteontime = time;
}

//{ "p", "c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "h" };
char notes[] =
  { 255,  0,   1,   2,    3,   4,    6,   7,    8,   9,   10,  11 ,   12 };

u8 lengths[] =  { 4, 8, 16, 32, 64, 128 };

static void countdefaults(struct NoteInfo * ni, int * length_p, int * scale_p)
{
  int lengths[15] = { 0 };
  int scales[15] = { 0 };
  int maxlenval = 0; /* (*) */
  int maxscaleval = 0;
  int i;

  for (i = 0; i < ni->nrnotes; i++)
  {
    struct Notes * note = &ni->note[i];

    lengths[note->length]++;
    scales[note->scale]++;
  }

  maxlenval = lengths[0]; /* (*) smart compiler eliminates dead code */
  *length_p = 0;
  
  for (i = 1; i < 15; i++) /* `p' incremented scales[0], therefore ignored */
  {
    TRACE(0, ("%d - len: %d, scale: %d\n", i, lengths[i], scales[i]));
    
    if (lengths[i] > maxlenval) {
      *length_p = i;
      maxlenval = lengths[i];
    }
    if (scales[i] > maxscaleval) {
      *scale_p = i;
      maxscaleval = scales[i];
    }
  }
}

GSM_Error loadmid(char *filename, GSM_Ringtone *ringtone)
{
  int fd;
  struct NoteInfo * ni;
  int i;
  int deflen, defscale;

#ifdef WIN32
  if ((fd = open(filename, O_RDONLY | O_BINARY)) < 0)
  {
    perror("open");
    return GE_CANTOPENFILE;
  }
#else
  if ((fd = open(filename, O_RDONLY)) < 0)
  {
    perror("open");
    return GE_CANTOPENFILE;
  }
#endif
  
  ni = readmidi(fd);

  if (ni == NULL)
    return 0;

  countdefaults(ni, &deflen, &defscale);

  strcpy(ringtone->name,"GNOKII");

  if (ni->nrnotes<MAX_RINGTONE_NOTES)
    ringtone->NrNotes=ni->nrnotes;
  else
    ringtone->NrNotes=MAX_RINGTONE_NOTES;
    
  for (i = 0; i < ringtone->NrNotes; i++)
  {
    struct Notes * note = &ni->note[i];

    ringtone->notes[i].note=notes[note->note];
    ringtone->notes[i].note=notes[note->note]+(note->scale%4)*14;    
    
    ringtone->notes[i].duration=lengths[note->length];
    if (note->lextra)
      ringtone->notes[i].duration=ringtone->notes[i].duration*1.5;

    ringtone->notes[i].tempo=ni->beats;

  }

  ringtone->Loop=15;
  
  return 0;
}
