/*
 * File:	exif.h
 * Purpose:	cpp EXIF reader
 * 16/Mar/2003 <ing.davide.pizzolato@libero.it>
 * based on jhead-1.8 by Matthias Wandel <mwandel(at)rim(dot)net>
 */

#if !defined(__exif_h)
#define __exif_h

#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <time.h>
#include "_defanwimageext.h"

#define MAX_COMMENT 1000
#define MAX_SECTIONS 20
typedef struct tag_RATIONAL
{
	unsigned long N;
	unsigned long D;
}TYPERATIONAL, * pTYPERATIONAL;

typedef struct tag_ExifInfo 
{
	char  Version				[5];
    char  CameraMake			[32];
    char  CameraModel			[40];
    char  DateTime				[20];
    char  DateTimeOriginal      [20];
    char  DateTimeDigitized     [20];
    int   Height, Width;
    int   Orientation;
    int   IsColor;
    int   Process;
    int   FlashUsed;
	//int   ThumbnailFileOffset;//New Version
    double FocalLength;
    double ExposureTime;
    double ApertureFNumber;
	double ApertureValue;
	double MaxApertureRatio;
    double Distance;
    float CCDWidth;
    double ExposureBias;
    int		Whitebalance;
    int		MeteringMode;
    int		ExposureProgram;
    int		ISOequivalent;
    int		CompressionLevel;
	double		FocalplaneXRes;
	double		FocalplaneYRes;
	double		FocalplaneUnits;
	int		Xresolution;
	int		Yresolution;
	int		ResolutionUnit;
	double  Brightness;
    char  Comments[MAX_COMMENT];

    unsigned char *ThumbnailPointer;  /* Pointer at the thumbnail */
    unsigned ThumbnailSize;     /* Size of thumbnail. */
	//for write back the exif information
	BOOL				Has_ExifVersion;	// Version 1.0
	BOOL				Has_DateTimeOriginal;
	BOOL				Has_DateTimeDigitized;
	BOOL				Has_ShutterSpeed;
	BOOL				Has_Aperture;
	BOOL				Has_Brightness;
	BOOL				Has_ExposureBias;
	BOOL				Has_MaxApertureRatio;
	BOOL				Has_SubjectDistance;
	BOOL				Has_MeteringMode;
	BOOL				Has_LightSource;
	BOOL				Has_Flash;
	BOOL				Has_FocalLength;
	BOOL				Has_ExposureTime;
	BOOL				Has_Fnumber;
	BOOL				Has_ComponentsConfiguration;
	BOOL				Has_CompressedBitsPerPixel;
	BOOL				Has_MakerNote;
	BOOL				Has_UserComment;
	BOOL				Has_FlashPixVersion;	
	BOOL				Has_ColorSpace;
	BOOL				Has_PixelXDimension;
	BOOL				Has_PixelYDimension;
	BOOL				Has_RelatedSoundFile;
	BOOL				Has_SubjectLocation;
	BOOL				Has_FileSource;
	BOOL				bIsR98;
	
	char				Reserved[7];
	//-------------------------------
	bool  IsExif;
} EXIFINFO;

//--------------------------------------------------------------------------
// JPEG markers consist of one or more 0xFF unsigned chars, followed by a marker
// code unsigned char (which is not an FF).  Here are the marker codes of interest
// in this program.  (See jdmarker.c for a more complete list.)
//--------------------------------------------------------------------------

#define M_SOF0  0xC0            // Start Of Frame N
#define M_SOF1  0xC1            // N indicates which compression process
#define M_SOF2  0xC2            // Only SOF0-SOF2 are now in common use
#define M_SOF3  0xC3
#define M_SOF5  0xC5            // NB: codes C4 and CC are NOT SOF markers
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8            // Start Of Image (beginning of datastream)
#define M_EOI   0xD9            // End Of Image (end of datastream)
#define M_SOS   0xDA            // Start Of Scan (begins compressed data)
#define M_JFIF  0xE0            // Jfif marker
#define M_EXIF  0xE1            // Exif marker
#define M_COM   0xFE            // COMment 

class ANWIMAGE_EXT CExif
{
	typedef struct tag_Section_t
	{
		unsigned char*    Data;
		int      Type;
		unsigned Size;
	} Section_t;

public:
	EXIFINFO *m_exifinfo;
	char m_szLastError[256];
	CExif(EXIFINFO* info = NULL);
	~CExif();
	bool DecodeExif(FILE *hFile);
	void GetDateTime(struct tm *dt);

protected:
	bool process_EXIF(unsigned char * CharBuf, unsigned int length);
	void process_COM(const unsigned char * Data, int length);
	void process_SOFn(const unsigned char * Data, int marker);
	int Get16u(void * Short);
	int Get16m(void * Short);
	long Get32s(void * Long);
	unsigned long Get32u(void * Long);
	double ConvertAnyFormat(void * ValuePtr, int Format);
	bool ProcessExifDir(unsigned char * DirStart, unsigned char * OffsetBase, unsigned ExifLength,
                        EXIFINFO * const pInfo, unsigned char ** const LastExifRefdP);
	int GetExifString(char *szBuf, bool bTitle = true);

	int ExifImageWidth;
	int MotorolaOrder;
	Section_t Sections[MAX_SECTIONS];
	int SectionsRead;
	bool freeinfo;
};

#endif