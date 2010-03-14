#ifndef __gsm_bitmaps_h
#define __gsm_bitmaps_h

#include "../gsmcomon.h"

/* Bitmap types. */
typedef enum {
	GSM_None = 1,
	GSM_StartupLogo,
	GSM_OperatorLogo,
	GSM_CallerLogo,
	GSM_PictureImage,
	GSM_WelcomeNoteText,
	GSM_DealerNoteText
} GSM_Bitmap_Types;

#define GSM_BITMAP_SIZE	864

/* Structure to hold incoming/outgoing bitmaps (and welcome-notes). */
typedef struct {
	unsigned char		Location;                	  /* Caller group number */
	unsigned char		Height;				  /* Bitmap height (pixels) */
	unsigned char		Width;				  /* Bitmap width (pixels) */
	GSM_Bitmap_Types	Type;				  /* Bitmap type */
	char			NetworkCode[7];			  /* Network operator code */
	char			Text	[256];			  /* Text used for (dealer) welcome-note
								   * or callergroup name or Picture Image text */
	bool			DefaultName;			  /* When get caller group - is default name ? */
	unsigned char		Bitmap	[GSM_BITMAP_SIZE];	  /* Actual Bitmap ((65+7)/8*96=864) */ 
	unsigned char		Ringtone;			  /* Ringtone ID sent with caller group */
	bool			Enabled;                       	  /* With caller logos = displayed or not */
	char			Sender	[GSM_MAX_NUMBER_LENGTH+1];/* For Picture Images - number of sender */
} GSM_Bitmap;

#define MAX_MULTI_BITMAP 6

typedef struct {
	unsigned char	Number;
	GSM_Bitmap	Bitmap[MAX_MULTI_BITMAP];
} GSM_MultiBitmap;

typedef enum {
	GSM_NokiaStartupLogo = 1,	/*size 84*48*/
	GSM_NokiaOperatorLogo,		/*size 72*14*/
	GSM_Nokia7110OperatorLogo,	/*size 78*21*/
	GSM_Nokia6510OperatorLogo,	/*size 78*21*/
	GSM_NokiaCallerLogo,		/*size 72*14*/
	GSM_NokiaPictureImage,		/*size 72*28*/
	GSM_Nokia7110StartupLogo,	/*size 96*65*/
	GSM_Nokia6210StartupLogo,	/*size 96*60*/
	GSM_EMSSmallPicture,		/*size  8*8 */
	GSM_EMSMediumPicture,		/*size 16*16*/
	GSM_EMSBigPicture		/*size 32*32*/
} GSM_Phone_Bitmap_Types;

bool GSM_IsPointBitmap		(GSM_Bitmap 		*bmp, int x, int y);
void GSM_SetPointBitmap		(GSM_Bitmap 		*bmp, int x, int y);
void GSM_PrintBitmap		(FILE *file, 		GSM_Bitmap *bitmap);
void GSM_ClearBitmap		(GSM_Bitmap 		*bmp);
void GSM_ResizeBitmap		(GSM_Bitmap 		*dest, GSM_Bitmap *src, int width, int height);
void GSM_GetMaxBitmapWidthHeight(GSM_Bitmap_Types 	Type, unsigned char *width, unsigned char *height);
int  GSM_GetBitmapSize		(GSM_Bitmap 		*bmp);

GSM_Error GSM_SaveBitmapFile(char *FileName, GSM_MultiBitmap *bitmap);
GSM_Error GSM_ReadBitmapFile(char *FileName, GSM_MultiBitmap *bitmap);

void PHONE_ClearBitmap		(GSM_Phone_Bitmap_Types Type, char *buffer);
void PHONE_DecodeBitmap		(GSM_Phone_Bitmap_Types Type, char *buffer, GSM_Bitmap *Bitmap);
void PHONE_EncodeBitmap		(GSM_Phone_Bitmap_Types Type, char *buffer, GSM_Bitmap *Bitmap);
void PHONE_GetBitmapWidthHeight	(GSM_Phone_Bitmap_Types Type, int *width, int *height);
int  PHONE_GetBitmapSize	(GSM_Phone_Bitmap_Types Type);

void NOKIA_CopyBitmap		(GSM_Phone_Bitmap_Types Type, GSM_Bitmap *Bitmap, char *Buffer, int *Length);
void EMS_CopyBitmapUDH		(GSM_Phone_Bitmap_Types Type, GSM_Bitmap *Bitmap, char *Buffer);
void EMS_CopyAnimationUDH	(GSM_Phone_Bitmap_Types Type, GSM_MultiBitmap *Bitmap, char *Buffer);

#endif
