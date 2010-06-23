/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.
  
  Functions for manipulating bitmaps

*/

#ifndef __gsm_bitmaps_h
#define __gsm_bitmaps_h

#include "gsm-sms.h"

/* Bitmap types. */
typedef enum {
  GSM_None=0,
  GSM_StartupLogo,      /*size 84*48*/
  GSM_OperatorLogo,     /*size 72*14*/
  GSM_CallerLogo,       /*size 72*14*/
  GSM_PictureImage,     /*size 72*28*/
  GSM_7110OperatorLogo, /*size 78*21*/
  GSM_7110StartupLogo,  /*size 96*65*/
  GSM_6210StartupLogo,  /*size 96*60*/
  GSM_WelcomeNoteText,
  GSM_DealerNoteText
} GSM_Bitmap_Types;

#define MAX_BITMAP_TEXT_LENGTH 256

/* Structure to hold incoming/outgoing bitmaps (and welcome-notes). */
typedef struct {
  u8 height;                /* Bitmap height (pixels) */
  u8 width;                 /* Bitmap width (pixels) */
  u16 size;                 /* Bitmap size (bytes) */
  GSM_Bitmap_Types type;    /* Bitmap type */
  char netcode[7];          /* Network operator code */
  char text[MAX_BITMAP_TEXT_LENGTH];   /* Text used for (dealer) welcome-note
                                          or callergroup name or Picture Image text */
  unsigned char bitmap[864];           /* Actual Bitmap ((65+7)/8*96=864) */ 
  unsigned char number;                /* Caller group number */
  char ringtone;                       /* Ringtone no sent with caller group */
  bool enabled;                        /* With caller logos = displayed or not */
  char Sender[GSM_MAX_SENDER_LENGTH+1];/* For Picture Images - number of sender */
} GSM_Bitmap;

void GSM_SetPointBitmap(GSM_Bitmap *bmp, int x, int y);
void GSM_ClearPointBitmap(GSM_Bitmap *bmp, int x, int y);
bool GSM_IsPointBitmap(GSM_Bitmap *bmp, int x, int y);
void GSM_ClearBitmap(GSM_Bitmap *bmp);
void GSM_ResizeBitmap(GSM_Bitmap *bitmap, GSM_Bitmap_Types target);
void GSM_PrintBitmap(GSM_Bitmap *bitmap);
GSM_Error GSM_ReadBitmap(GSM_SMSMessage *message, GSM_Bitmap *bitmap);
int GSM_SaveBitmapToSMS(GSM_MultiSMSMessage *SMS, GSM_Bitmap *bitmap,bool ScreenSaver, bool UnicodeText);
int GSM_GetBitmapSize(GSM_Bitmap *bitmap);

#endif
