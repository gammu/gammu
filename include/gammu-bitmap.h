/**
 * \file gammu-bitmap.h
 * \author Michal Čihař
 * 
 * Bitmap data and functions.
 */
#ifndef __gammu_bitmap_h
#define __gammu_bitmap_h

/**
 * \defgroup Bitmap Bitmap
 * Bitmaps manipulations.
 */

#include <gammu-limits.h>
#include <stdio.h>

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
 *
 * \ingroup Bitmap
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
 *
 * \ingroup Bitmap
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
	 * For caller group logo: picture ID. Phone model specific
	 */
	int	 		PictureID;
	bool 			FileSystemPicture;
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
 *
 * \ingroup Bitmap
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

/**
 * Gets bitmap from phone.
 *
 * \ingroup Bitmap
 */
GSM_Error GSM_GetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap);
/**
 * Sets bitmap in phone.
 *
 * \ingroup Bitmap
 */
GSM_Error GSM_SetBitmap(GSM_StateMachine *s, GSM_Bitmap *Bitmap);

/**
 * Prints bitmap to file.
 *
 * \param file Where to print.
 * \param bitmap Bitmap to print.
 *
 * \ingroup Bitmap
 */
void GSM_PrintBitmap(FILE *file, GSM_Bitmap *bitmap);

GSM_Error GSM_SaveBitmapFile	(char *FileName, GSM_MultiBitmap *bitmap);
GSM_Error GSM_ReadBitmapFile	(char *FileName, GSM_MultiBitmap *bitmap);
#endif
