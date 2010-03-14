/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.
  
  Functions to read and write common file types.

*/

#include "gsm-api.h"
#include "gsm-calendar.h"
#include "gsm-ringtones.h"
#include "gsm-bitmaps.h"

/* ---------- Calendar file --------- */

GSM_Error GSM_ReadVCalendarFile(char *FileName, GSM_CalendarNote *cnote,
  int *number);

int GetvCalTime(GSM_DateTime *dt, char *time);
int FillCalendarNote(GSM_CalendarNote *note, char *type,
  char *text, char *time, char *alarm);

/* ----------- Text files ----------- */

int GSM_SaveTextFile(char *FileName, char *text, int mode);

/* ----------- Ringtones functions and constans ----------- */

int GetScale (char *num);
int GetDuration (char *num);

/* Defines the character that separates fields in rtttl files. */
#define RTTTL_SEP ":"

/* ----------- Ringtones files ----------- */

GSM_Error GSM_ReadBinRingtoneFile(char *FileName, GSM_BinRingtone *ringtone);
GSM_Error GSM_SaveBinRingtoneFile(char *FileName, GSM_BinRingtone *ringtone);

GSM_Error GSM_ReadRingtoneFile(char *FileName, GSM_Ringtone *ringtone);
GSM_Error GSM_SaveRingtoneFile(char *FileName, GSM_Ringtone *ringtone);

void saverttl(FILE *file, GSM_Ringtone *ringtone);
void saveott(FILE *file, GSM_Ringtone *ringtone);
void savemid(FILE* file, GSM_Ringtone *ringtone);

GSM_Error loadrttl(FILE *file, GSM_Ringtone *ringtone);
GSM_Error loadott(FILE *file, GSM_Ringtone *ringtone);
GSM_Error loadcommunicator(FILE *file, GSM_Ringtone *ringtone);

/* ----------- Bitmap files ----------- */

GSM_Error GSM_ReadBitmapFile(char *FileName, GSM_Bitmap *bitmap);
GSM_Error GSM_SaveBitmapFile(char *FileName, GSM_Bitmap *bitmap);

void savenol(FILE *file, GSM_Bitmap *bitmap);
void savengg(FILE *file, GSM_Bitmap *bitmap);
void savensl(FILE *file, GSM_Bitmap *bitmap);
void savenlm(FILE *file, GSM_Bitmap *bitmap);
void saveota(FILE *file, GSM_Bitmap *bitmap);
void savebmp(FILE *file, GSM_Bitmap *bitmap);
void savexpm(FILE *file, GSM_Bitmap *bitmap);

GSM_Error loadngg(FILE *file, GSM_Bitmap *bitmap);
GSM_Error loadnol(FILE *file, GSM_Bitmap *bitmap);
GSM_Error loadnsl(FILE *file, GSM_Bitmap *bitmap);
GSM_Error loadnlm(FILE *file, GSM_Bitmap *bitmap);
GSM_Error loadota(FILE *file, GSM_Bitmap *bitmap);
GSM_Error loadbmp(FILE *file, GSM_Bitmap *bitmap);

#ifdef XPM
  GSM_Error loadxpm(char *filename, GSM_Bitmap *bitmap);
#endif

/* ----------- Backup files ----------- */

GSM_Error GSM_ReadBackupFile(char *FileName, GSM_Backup *backup);
GSM_Error GSM_SaveBackupFile(char *FileName, GSM_Backup *backup);

void savelmb(FILE *file, GSM_Backup *backup);

GSM_Error loadlmb(FILE *file, GSM_Backup *backup);

typedef enum {
  None=0,
  NOL,
  NGG,
  NSL,
  NLM,
  BMP,
  OTA,
  XPMF,
  RTTL,
  OTT,
  MIDI,
  COMMUNICATOR,
  LMB
} GSM_Filetypes;
