/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.

  This file provides definitions of macros from the Smart Messaging
  Specification. It is mainly rewrite of the spec to C :-) Viva Nokia!

*/

#ifndef __gsm_ringtones_h
#define __gsm_ringtones_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "misc.h"
#include "gsm-common.h"
#include "gsm-sms.h"

#define MAX_RINGTONE_NOTES 255

/* Structure to hold note of ringtone. */
/* NoteValue is encoded as octave(scale)*14 + note */
/* where for note: c=0, d=2, e=4 .... */
/* ie. c#=1 and 5 and 13 are invalid */
/* note=255 means a pause */
typedef struct {
  u8 duration;
  u8 note;
  int tempo;
  u8 style;
} GSM_RingtoneNote;

/* Structure to hold ringtones. */
typedef struct {
  char name[20];
  unsigned char Loop;
  u8 NrNotes;
  GSM_RingtoneNote notes[256];
  bool allnotesscale; /* Some Nokia phones doesn't receive correctly some
                         ringtones without it. N3310 4.02 */
  int location;
} GSM_Ringtone;
  
/* Structure to hold binary ringtones. */

typedef struct {
  char name[20];
  unsigned char frame[1000];
  int length;
  int location;
} GSM_BinRingtone;

#define GetBit(Stream,BitNr) Stream[(BitNr)/8] & 1<<(7-((BitNr)%8))
#define SetBit(Stream,BitNr) Stream[(BitNr)/8] |= 1<<(7-((BitNr)%8))
#define ClearBit(Stream,BitNr) Stream[(BitNr)/8] &= 255 - (1 << (7-((BitNr)%8)))

/* These values are from Smart Messaging Specification Revision 2.0.0 pages
   3-23, ..., 3-29 */

/* Command-Part Encoding */

#define CancelCommand          (0x05<<1) /* binary 0000 101 */
#define RingingToneProgramming (0x25<<1) /* binary 0100 101 */
#define Sound                  (0x1d<<1) /* binary 0011 101 */
/* MW: I didn't find phone, which can unpack ringtone with Unicode
   command. Tested 3310, 6150 */
#define Unicode                (0x22<<1) /* binary 0100 010 */

/* Song-Type Encoding */

#define BasicSongType     (0x01<<5) /* binary 001 */
#define TemporarySongType (0x02<<5) /* binary 010 */
#define MidiSongType      (0x03<<5) /* binary 011 */
#define DigitizedSongType (0x04<<5) /* binary 100 */

/* Instruction ID Encoding */

#define PatternHeaderId      (0x00<<5) /* binary 000 */
#define NoteInstructionId    (0x01<<5) /* binary 001 */
#define ScaleInstructionId   (0x02<<5) /* binary 010 */
#define StyleInstructionId   (0x03<<5) /* binary 011 */
#define TempoInstructionId   (0x04<<5) /* binary 100 */
#define VolumeInstructionId  (0x05<<5) /* binary 101 */

/* Style-Value Encoding*/

#define NaturalStyle    (0x00<<6) /* binary 00 */
#define ContinuousStyle (0x01<<6) /* binary 01 */
#define StaccatoStyle   (0x02<<6) /* binary 10 */

/* Note-Scale Encoding  */

#define Scale1 (0x00<<6) /* binary 00 */
#define Scale2 (0x01<<6) /* binary 01 */
#define Scale3 (0x02<<6) /* binary 10 */
#define Scale4 (0x03<<6) /* binary 11 */

/* Note-Value Encoding */

#define Note_Pause (0x00<<4) /* binary 0000 */
#define Note_C     (0x01<<4) /* binary 0001 */
#define Note_Cis   (0x02<<4) /* binary 0010 */
#define Note_D     (0x03<<4) /* binary 0011 */
#define Note_Dis   (0x04<<4) /* binary 0100 */
#define Note_E     (0x05<<4) /* binary 0101 */
#define Note_F     (0x06<<4) /* binary 0110 */
#define Note_Fis   (0x07<<4) /* binary 0111 */
#define Note_G     (0x08<<4) /* binary 1000 */
#define Note_Gis   (0x09<<4) /* binary 1001 */
#define Note_A     (0x0a<<4) /* binary 1010 */
#define Note_Ais   (0x0b<<4) /* binary 1011 */
#define Note_H     (0x0c<<4) /* binary 1100 */

/* Note-Duration Encoding */

#define Duration_Full (0x00<<5) /* binary 000 */
#define Duration_1_2  (0x01<<5) /* binary 001 */
#define Duration_1_4  (0x02<<5) /* binary 010 */
#define Duration_1_8  (0x03<<5) /* binary 011 */
#define Duration_1_16 (0x04<<5) /* binary 100 */
#define Duration_1_32 (0x05<<5) /* binary 101 */

/* Note-Duration-Specifier Encoding */

#define NoSpecialDuration (0x00<<6) /* binary 00 */
#define DottedNote        (0x01<<6) /* binary 01 */
#define DoubleDottedNote  (0x02<<6) /* binary 10 */
#define Length_2_3        (0x03<<6) /* binary 11 */

/* Pattern ID Encoding */
#define A_part (0x00<<6) /* binary 00 */
#define B_part (0x01<<6) /* binary 01 */
#define C_part (0x02<<6) /* binary 10 */
#define D_part (0x03<<6) /* binary 11 */

/* Command-End */
#define CommandEnd (0x00) /* binary 00000000 */

u8 GSM_PackRingtone(GSM_Ringtone *ringtone, unsigned char *package, int *maxlength);
GSM_Error GSM_UnPackRingtone(GSM_Ringtone *ringtone, char *package, int maxlength);

int GSM_GetDuration(int number, unsigned char *spec);
int GSM_GetNote(int number);
int GSM_GetScale(int number);
int GSM_GetTempo(int Beats);

void GSM_PlayOneNote (GSM_RingtoneNote note);
void GSM_PlayRingtone (GSM_Ringtone *ringtone);

int GSM_GetFrequency(int number);

GSM_Error GSM_ReadRingtone(GSM_SMSMessage *message, GSM_Ringtone *ringtone);

int GSM_SaveRingtoneToSMS(GSM_MultiSMSMessage *SMS,
                          GSM_Ringtone *ringtone,bool profilestyle);

/* FIXME: For each phone probaby different */
#define FB61_MAX_RINGTONE_FRAME_LENGTH 200

/* Smart Messaging 3.0 says: 16*9=144 bytes,
   but on 3310 4.02 it was possible to save about 196 chars (without cutting) */
#define SM30_MAX_RINGTONE_FRAME_LENGTH 196

/* Like PC Composer help say */
#define FB61_MAX_RINGTONE_NOTES 130

/* Nokia ringtones codes. */

struct OneRingtone {
  char name[30];
  int menu;
  int code;
};

void PrepareRingingTones(char model[64], char rev[64]);

/* returns names from code or number in menu */
char *RingingToneName(int code, int menu);

/* returns code from number in menu */
int RingingToneCode(int menu);

/* returns number in menu from code */
int RingingToneMenu(int code);

int NumberOfRingtones();

GSM_Error GSM_GetPhoneRingtone(GSM_BinRingtone *ringtone,GSM_Ringtone *SMringtone);

#endif	/* __gsm_ringtones_h */
