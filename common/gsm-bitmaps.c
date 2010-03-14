/*

  G N O K I I

  A Linux/Unix toolset and driver for Nokia mobile phones.

  Released under the terms of the GNU GPL, see file COPYING for more details.
	
  Functions for manipulating bitmaps
  
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "gsm-common.h"
#include "gsm-bitmaps.h"
#include "gsm-sms.h"
#include "gsm-coding.h"
#include "gsm-networks.h"

void GSM_SetPointBitmap(GSM_Bitmap *bmp, int x, int y)
{
  int pixel;
  if (bmp->type == GSM_StartupLogo || bmp->type == GSM_6210StartupLogo || bmp->type == GSM_7110StartupLogo)
    bmp->bitmap[((y/8)*bmp->width)+x] |= 1 << (y%8);
  if (bmp->type == GSM_OperatorLogo || bmp->type == GSM_7110OperatorLogo || bmp->type == GSM_CallerLogo) {
    pixel=bmp->width*y + x;
    bmp->bitmap[pixel/8] |= 1 << (7-(pixel%8));
  }
  if (bmp->type == GSM_PictureImage) bmp->bitmap[9*y + (x/8)] |= 1 << (7-(x%8));
}
  
void GSM_ClearPointBitmap(GSM_Bitmap *bmp, int x, int y)
{
  int pixel;
  if (bmp->type == GSM_StartupLogo || bmp->type == GSM_6210StartupLogo || bmp->type == GSM_7110StartupLogo)
    bmp->bitmap[((y/8)*bmp->width)+x] &= 255 - (1 << (y%8));
  if (bmp->type == GSM_OperatorLogo || bmp->type == GSM_7110OperatorLogo || bmp->type == GSM_CallerLogo) {
    pixel=bmp->width*y + x;
    bmp->bitmap[pixel/8] &= ~(1 << (7-(pixel%8)));
  }
  if (bmp->type == GSM_PictureImage) bmp->bitmap[9*y + (x/8)] &= 255 - (1 << (7-(x%8)));
}

bool GSM_IsPointBitmap(GSM_Bitmap *bmp, int x, int y)
{
  int i=0;
  int pixel;

  if (bmp->type == GSM_StartupLogo || bmp->type == GSM_6210StartupLogo || bmp->type == GSM_7110StartupLogo)
    i=(bmp->bitmap[((y/8)*bmp->width) + x] & 1<<((y%8)));
  if (bmp->type == GSM_OperatorLogo || bmp->type == GSM_7110OperatorLogo || bmp->type == GSM_CallerLogo) {
    pixel=bmp->width*y + x;
    i=(bmp->bitmap[pixel/8] & 1<<(7-(pixel%8)));
  }
  if (bmp->type == GSM_PictureImage) i=(bmp->bitmap[9*y + (x/8)] & 1<<(7-(x%8)));

  if (i) return true; else return false;
}
  
void GSM_ClearBitmap(GSM_Bitmap *bmp)
{
  int i;
  for (i=0;i<bmp->size;i++) bmp->bitmap[i]=0;
}

int GSM_GetBitmapSize(GSM_Bitmap *bitmap)
{
  switch (bitmap->type) {
    case GSM_StartupLogo     : /*size 84*48*/
    case GSM_OperatorLogo    : /*size 72*14*/
    case GSM_CallerLogo      : /*size 72*14*/
    case GSM_PictureImage    : /*size 72*28*/
      return bitmap->height*bitmap->width/8;

    case GSM_7110OperatorLogo: /*size 78*21*/
      return (bitmap->width*bitmap->height + 7)/8;

    case GSM_7110StartupLogo: /*size 96*65*/
    case GSM_6210StartupLogo: /*size 96*60*/
      return (bitmap->height+7)/8*bitmap->width;

    default:
      return 0;
  }
}

GSM_Error GSM_ReadBitmap(GSM_SMSMessage *message, GSM_Bitmap *bitmap)
{
  int offset = 1;
  unsigned char buffer[20];

  switch (message->UDHType) {
  case GSM_OpLogo:
    EncodeUDHHeader(buffer, GSM_OperatorLogo);
    if (message->Length!=133) return GE_UNKNOWN;
    
    bitmap->type = GSM_OperatorLogo;

    DecodeNetworkCode(message->MessageText, bitmap->netcode);

    offset = 4;
    break;

  case GSM_CallerIDLogo:
    EncodeUDHHeader(buffer, GSM_CallerLogo);
    if (message->Length!=130) return GE_UNKNOWN;
    
    bitmap->type=GSM_CallerLogo;

    break;
  default: /* error */
    return GE_UNKNOWN;
    break;
  }
  bitmap->width = message->MessageText[offset];
  bitmap->height = message->MessageText[offset + 1];
  
  if (bitmap->width!=72 || bitmap->height!=14) return GE_INVALIDIMAGESIZE;
  
  bitmap->size = GSM_GetBitmapSize(bitmap);
  memcpy(bitmap->bitmap, message->MessageText + offset + 3, bitmap->size);

#ifdef DEBUG
  fprintf(stdout, _("Bitmap from SMS: width %i, height %i\n"),bitmap->width,bitmap->height);
#endif

  return GE_NONE;
}

void GSM_ResizeBitmap(GSM_Bitmap *bitmap, GSM_Bitmap_Types target)
{
  GSM_Bitmap backup;
  int x,y,width,height;
  
  backup=*bitmap;
      
  if (target==GSM_StartupLogo) {
    bitmap->width=84;
    bitmap->height=48;
  }
  if (target==GSM_7110StartupLogo) {
    bitmap->width=96;
    bitmap->height=65;
  }
  if (target==GSM_6210StartupLogo) {
    bitmap->width=96;
    bitmap->height=60;
  }
  if (target==GSM_OperatorLogo || target==GSM_CallerLogo) {
    bitmap->width=72;
    bitmap->height=14;
  }
  if (target==GSM_PictureImage ) {
    bitmap->width=72;
    bitmap->height=28;
  }
  if (target==GSM_7110OperatorLogo) {
    bitmap->width=78;
    bitmap->height=21;
  }
  bitmap->type=target;
  bitmap->size=GSM_GetBitmapSize(bitmap);
  
  width=backup.width;
  if (bitmap->width<width) {
    width=bitmap->width;
#ifdef DEBUG
    fprintf(stdout,_("We lost some part of image - it's cut (width from %i to %i) !\n"),backup.width,width);
#endif /* DEBUG */
  }
  
  height=backup.height;
  if (bitmap->height<height) {
    height=bitmap->height; 
#ifdef DEBUG
    fprintf(stdout,_("We lost some part of image - it's cut (height from %i to %i) !\n"),backup.height,height);
#endif /* DEBUG */
  }
  
  GSM_ClearBitmap(bitmap);
  
  for (y=0;y<height;y++) {
    for (x=0;x<width;x++)
      if (GSM_IsPointBitmap(&backup,x,y)) GSM_SetPointBitmap(bitmap,x,y);
  }
  
//GSM_PrintBitmap(&backup);
//GSM_PrintBitmap(bitmap);
}

void GSM_PrintBitmap(GSM_Bitmap *bitmap)
{
  int x,y;

  for (y=0;y<bitmap->height;y++) {
    for (x=0;x<bitmap->width;x++) {
      if (GSM_IsPointBitmap(bitmap,x,y)) {
        fprintf(stdout, _("#"));
      } else {
        fprintf(stdout, _(" "));
      }
    }
    fprintf(stdout, _("\n"));
  }
}

int GSM_SaveBitmapToSMS(GSM_MultiSMSMessage *SMS, GSM_Bitmap *bitmap,
                        bool ScreenSaver, bool UnicodeText)
{
  char MessageBuffer[GSM_MAX_SMS_8_BIT_LENGTH*4];
  int MessageLength=0;
  GSM_UDH UDHType=GSM_NoUDH;
  
  switch (bitmap->type) {
    case GSM_OperatorLogo:
      UDHType=GSM_OpLogo;

      EncodeNetworkCode(MessageBuffer, bitmap->netcode);
      MessageLength=3;

      /* Set the logo size */
      MessageBuffer[MessageLength++] = 0x00;
      MessageBuffer[MessageLength++] = bitmap->width;
      MessageBuffer[MessageLength++] = bitmap->height;
      MessageBuffer[MessageLength++] = 0x01;

      memcpy(MessageBuffer+MessageLength,bitmap->bitmap,bitmap->size);
      MessageLength=MessageLength+bitmap->size;

      break;
    case GSM_CallerLogo:
      UDHType=GSM_CallerIDLogo;

      /* Set the logo size */
      MessageBuffer[MessageLength++] = 0x00;
      MessageBuffer[MessageLength++] = bitmap->width;
      MessageBuffer[MessageLength++] = bitmap->height;
      MessageBuffer[MessageLength++] = 0x01;

      memcpy(MessageBuffer+MessageLength,bitmap->bitmap,bitmap->size);
      MessageLength=MessageLength+bitmap->size;

      break;
    case GSM_PictureImage:
      UDHType=GSM_ProfileUDH;

      MessageBuffer[MessageLength++]=0x30;     //SM version. Here 3.0

      if (!ScreenSaver)
        MessageBuffer[MessageLength++]=SM30_OTA; //ID for OTA bitmap
      else
        MessageBuffer[MessageLength++]=SM30_SCREENSAVER; //ID for screen saver

      /* Length for picture part */
      MessageBuffer[MessageLength++]=0x01;     //length hi
      MessageBuffer[MessageLength++]=0x00;     //length lo

      /* Set the logo size */
      MessageBuffer[MessageLength++] = 0x00;
      MessageBuffer[MessageLength++] = bitmap->width;
      MessageBuffer[MessageLength++] = bitmap->height;
      MessageBuffer[MessageLength++] = 0x01;

      memcpy(MessageBuffer+MessageLength,bitmap->bitmap,bitmap->size);
      MessageLength=MessageLength+bitmap->size;

      if (strlen(bitmap->text)!=0) {
        if (UnicodeText) {
          MessageBuffer[MessageLength++]=SM30_UNICODETEXT; //ID for Unicode text

          /* Length for text part */
          MessageBuffer[MessageLength++]=0x00;             //length of text1
          MessageBuffer[MessageLength++]=strlen(bitmap->text)*2;//length of text2

          EncodeUnicode (MessageBuffer+MessageLength,bitmap->text,strlen(bitmap->text));
          MessageLength=MessageLength+2*strlen(bitmap->text);
        } else {
          MessageBuffer[MessageLength++]=SM30_ISOTEXT;    //ID for ISO-8859-1 text

          /* Length for text part */
          MessageBuffer[MessageLength++]=0x00;            //length of text1
          MessageBuffer[MessageLength++]=strlen(bitmap->text); //length of text2

          memcpy(MessageBuffer+MessageLength,bitmap->text,strlen(bitmap->text));
          MessageLength=MessageLength+strlen(bitmap->text);
       }
      }
      break;
    
    default: /* error */
      break;
  }
        
  GSM_MakeMultiPartSMS2(SMS,MessageBuffer,MessageLength, UDHType, GSM_Coding_Default);

  return 0;
}

