/* (c) 2002-2004 by Marcin Wiacek */

#ifndef __gsm_bitmaps_h
#define __gsm_bitmaps_h

#include "../gsmcomon.h"

typedef enum {
	PICTURE_BMP = 1,
	PICTURE_GIF,
	PICTURE_JPG,
	PICTURE_ICN,
	PICTURE_PNG
} GSM_BinaryPicture_Types;

typedef struct {
	GSM_BinaryPicture_Types Type;
	unsigned char 		*Buffer;
	int 			Length;
} GSM_BinaryPicture;

/**
 * Enum to handle all possible bitmaps, which are not saved in various filesystems.
 */
typedef enum {
	GSM_None = 1,
	/**
	 * ID of static file in filesystem displayed during startup
	 */
	GSM_ColourStartupLogo_ID,
	/**
	 * Static mono bitmap/ID of animated mono bitmap displayed during startup
	 */
	GSM_StartupLogo,
	/**
	 * ID of static file in filesystem displayed instead of operator name
	 */
	GSM_ColourOperatorLogo_ID,
	/**
	 * Mono bitmap displayed instead of operator name
	 */
	GSM_OperatorLogo,
	/**
	 * ID of static file in filesystem displayed as wallpaper
	 */
	GSM_ColourWallPaper_ID,
	/**
	 * Mono bitmap assigned to caller group
	 */
	GSM_CallerGroupLogo,
	/**
	 * Text displayed during startup, which can't be removed from phone menu
	 */
	GSM_DealerNote_Text,
	/**
	 * Text displayed during startup
	 */
	GSM_WelcomeNote_Text,
	/**
	 * Image defined in Smart Messaging specification
	 */
	GSM_PictureImage,
	/**
	 * Binary picture (BMP, GIF, etc.)
	 */
	GSM_PictureBinary
} GSM_Bitmap_Types;

#define GSM_BITMAP_SIZE	(65+7)/8*96
#define GSM_BITMAP_TEXT_LENGTH 128

/**
 * Structure for all possible bitmaps, which are not saved in various filesystems
 */
typedef struct {
	/**
	 * For all: bitmap type
	 */
	GSM_Bitmap_Types 	Type;
	/**
	 * For caller group logos: number of group
	 * For startup logos: number of animated bitmap
	 */
	unsigned char 		Location;
	/**
	 * For dealer/welcome note text: text
	 * For caller group logo: name of group
         * For picture images: text assigned to it
	 */
	unsigned char 		Text[2 * (GSM_BITMAP_TEXT_LENGTH + 1)];
	/**
	 * For caller group logo: true, when logo is enabled in group
	 */
	bool 			BitmapEnabled;
	/**
	 * For caller group logo: true, when group has default name
	 */
	bool 			DefaultName;
	/**
	 * For caller group logo: true, when group has default bitmap
	 */
	bool 			DefaultBitmap;
	/**
	 * For caller group logo: true, when group has default ringtone
	 */
	bool 			DefaultRingtone;
	/**
	 * For caller group logo: ringtone ID. Phone model specific
	 */
	unsigned char 		RingtoneID;
	bool 			FileSystemRingtone;
	/**
	 * For mono bitmaps: body of bitmap
	 */ 
	unsigned char 		BitmapPoints[GSM_BITMAP_SIZE];
	/**
	 * For mono bitmaps: height specified in pixels
	 */
	unsigned char 		BitmapHeight;
	/**
	 * For mono bitmaps: width specified in pixels
	 */
	unsigned char 		BitmapWidth;
	/**
	 * For operator logos: Network operator code
	 */
	char			NetworkCode[7];
	/**
	 * For picture images: number of sender
	 */
	unsigned char 		Sender[2 * (GSM_MAX_NUMBER_LENGTH + 1)];
	/**
	 * For colour bitmaps: ID
	 */
	unsigned char 		ID;
	/**
	 * For binary pictures (GIF, BMP, etc.): frame and length
	 */
	GSM_BinaryPicture 	BinaryPic;
	/**
	 * Bitmap name
	 */
	char			*Name;
} GSM_Bitmap;

#define MAX_MULTI_BITMAP 6

/**
 * Structure to handle more than one bitmap
 */
typedef struct {
	/**
	 * Number of bitmaps
	 */
	unsigned char		Number;
	/**
	 * All bitmaps
	 */
	GSM_Bitmap		Bitmap[MAX_MULTI_BITMAP];
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
	GSM_AlcatelBMMIPicture,
	GSM_EMSSmallPicture,		/*size  8* 8*/
	GSM_EMSMediumPicture,		/*size 16*16*/
	GSM_EMSBigPicture,		/*size 32*32*/
	GSM_EMSVariablePicture
} GSM_Phone_Bitmap_Types;

bool GSM_IsPointBitmap		(GSM_Bitmap 		*bmp, int x, int y);
void GSM_SetPointBitmap		(GSM_Bitmap 		*bmp, int x, int y);
void GSM_ClearPointBitmap	(GSM_Bitmap 		*bmp, int x, int y);
void GSM_ClearBitmap		(GSM_Bitmap 		*bmp);
void GSM_ResizeBitmap		(GSM_Bitmap 		*dest, GSM_Bitmap *src, int width, int height);
void GSM_ReverseBitmap		(GSM_Bitmap 		*Bitmap);
void GSM_GetMaxBitmapWidthHeight(GSM_Bitmap_Types 	Type, unsigned char *width, unsigned char *height);
int  GSM_GetBitmapSize		(GSM_Bitmap 		*bmp);
void GSM_PrintBitmap		(FILE 			*file, GSM_Bitmap *bitmap);

GSM_Error GSM_SaveBitmapFile	(char *FileName, GSM_MultiBitmap *bitmap);
GSM_Error GSM_ReadBitmapFile	(char *FileName, GSM_MultiBitmap *bitmap);

GSM_Error BMP2Bitmap            (unsigned char *buffer, FILE *file,GSM_Bitmap *bitmap);
GSM_Error Bitmap2BMP            (unsigned char *buffer, FILE *file,GSM_Bitmap *bitmap);

void PHONE_GetBitmapWidthHeight	(GSM_Phone_Bitmap_Types Type, int *width, int *height);
int  PHONE_GetBitmapSize	(GSM_Phone_Bitmap_Types Type, int width, int height);
void PHONE_ClearBitmap		(GSM_Phone_Bitmap_Types Type, char *buffer, int width, int height);
void PHONE_DecodeBitmap		(GSM_Phone_Bitmap_Types Type, char *buffer, GSM_Bitmap *Bitmap);
void PHONE_EncodeBitmap		(GSM_Phone_Bitmap_Types Type, char *buffer, GSM_Bitmap *Bitmap);

void NOKIA_CopyBitmap		(GSM_Phone_Bitmap_Types Type, GSM_Bitmap *Bitmap, char *Buffer, int *Length);

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
