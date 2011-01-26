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
#include <gammu-types.h>
#include <gammu-error.h>
#include <gammu-statemachine.h>
#include <stdio.h>

/**
 * Binary picture types.
 *
 * \ingroup Bitmap
 */
typedef enum {
	PICTURE_BMP = 1,
	PICTURE_GIF,
	PICTURE_JPG,
	PICTURE_ICN,
	PICTURE_PNG
} GSM_BinaryPicture_Types;

/**
 * Binary picture data.
 *
 * \ingroup Bitmap
 */
typedef struct {
	GSM_BinaryPicture_Types Type;
	unsigned char *Buffer;
	size_t Length;
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

/**
 * Structure for all possible bitmaps, which are not saved in various filesystems
 *
 * \ingroup Bitmap
 */
typedef struct {
	/**
	 * For all: bitmap type
	 */
	GSM_Bitmap_Types Type;
	/**
	 * For caller group logos: number of group
	 * For startup logos: number of animated bitmap
	 */
	unsigned char Location;
	/**
	 * For dealer/welcome note text: text
	 * For caller group logo: name of group
	 * For picture images: text assigned to it
	 */
	unsigned char Text[2 * (GSM_BITMAP_TEXT_LENGTH + 1)];
	/**
	 * For caller group logo: TRUE, when logo is enabled in group
	 */
	gboolean BitmapEnabled;
	/**
	 * For caller group logo: TRUE, when group has default name
	 */
	gboolean DefaultName;
	/**
	 * For caller group logo: TRUE, when group has default bitmap
	 */
	gboolean DefaultBitmap;
	/**
	 * For caller group logo: TRUE, when group has default ringtone
	 */
	gboolean DefaultRingtone;
	/**
	 * For caller group logo: ringtone ID. Phone model specific
	 */
	unsigned char RingtoneID;
	gboolean FileSystemRingtone;
	/**
	 * For caller group logo: picture ID. Phone model specific
	 */
	int PictureID;
	gboolean FileSystemPicture;
	/**
	 * For mono bitmaps: body of bitmap
	 */
	unsigned char BitmapPoints[GSM_BITMAP_SIZE];
	/**
	 * For mono bitmaps: height specified in pixels
	 */
	size_t BitmapHeight;
	/**
	 * For mono bitmaps: width specified in pixels
	 */
	size_t BitmapWidth;
	/**
	 * For operator logos: Network operator code
	 */
	char NetworkCode[7];
	/**
	 * For picture images: number of sender
	 */
	unsigned char Sender[2 * (GSM_MAX_NUMBER_LENGTH + 1)];
	/**
	 * For colour bitmaps: ID
	 */
	unsigned char ID;
	/**
	 * For binary pictures (GIF, BMP, etc.): frame and length
	 */
	GSM_BinaryPicture BinaryPic;
	/**
	 * Bitmap name
	 */
	unsigned char Name[2 * (GSM_BITMAP_TEXT_LENGTH + 1)];
} GSM_Bitmap;

/**
 * Structure to handle more than one bitmap
 *
 * \ingroup Bitmap
 */
typedef struct {
	/**
	 * Number of bitmaps
	 */
	unsigned char Number;
	/**
	 * All bitmaps
	 */
	GSM_Bitmap Bitmap[GSM_MAX_MULTI_BITMAP];
} GSM_MultiBitmap;

/**
 * Gets bitmap from phone.
 *
 * \ingroup Bitmap
 */
GSM_Error GSM_GetBitmap(GSM_StateMachine * s, GSM_Bitmap * Bitmap);

/**
 * Sets bitmap in phone.
 *
 * \ingroup Bitmap
 */
GSM_Error GSM_SetBitmap(GSM_StateMachine * s, GSM_Bitmap * Bitmap);

/**
 * Prints bitmap to file descriptor.
 *
 * \param file Where to print.
 * \param bitmap Bitmap to print.
 *
 * \ingroup Bitmap
 */
void GSM_PrintBitmap(FILE * file, GSM_Bitmap * bitmap);

/**
 * Saves bitmap to file.
 *
 * \param FileName Where to save.
 * \param bitmap Bitmap to save.
 *
 * \return Error code
 *
 * \ingroup Bitmap
 */
GSM_Error GSM_SaveBitmapFile(char *FileName, GSM_MultiBitmap * bitmap);

/**
 * Reads bitmap from file.
 *
 * \param FileName Where to load from.
 * \param bitmap Pointer where to load bitmap.
 *
 * \return Error code
 *
 * \ingroup Bitmap
 */
GSM_Error GSM_ReadBitmapFile(char *FileName, GSM_MultiBitmap * bitmap);

/**
 * Checks whether point is set in bitmap.
 *
 * \param bmp Bitmap
 * \param x Horizontal coordinate.
 * \param y Vertical coordinate.
 * \return True if point is set.
 *
 * \ingroup Bitmap
 */
gboolean GSM_IsPointBitmap(GSM_Bitmap * bmp, int x, int y);

/**
 * Sets point in bitmap.
 *
 * \param bmp Bitmap
 * \param x Horizontal coordinate.
 * \param y Vertical coordinate.
 *
 * \ingroup Bitmap
 */
void GSM_SetPointBitmap(GSM_Bitmap * bmp, int x, int y);

/**
 * Clears point in bitmap.
 *
 * \param bmp Bitmap
 * \param x Horizontal coordinate.
 * \param y Vertical coordinate.
 *
 * \ingroup Bitmap
 */
void GSM_ClearPointBitmap(GSM_Bitmap * bmp, int x, int y);

/**
 * Clears bitmap.
 *
 * \param bmp Bitmap
 *
 * \ingroup Bitmap
 */
void GSM_ClearBitmap(GSM_Bitmap * bmp);

/**
 * Gets phone screenshot.
 *
 * \param s State machine pointer.
 * \param picture Structure which will hold data.
 *
 * \ingroup Bitmap
 */
GSM_Error GSM_GetScreenshot(GSM_StateMachine *s, GSM_BinaryPicture *picture);

#endif

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
