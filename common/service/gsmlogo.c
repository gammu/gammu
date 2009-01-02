/* (c) 2001-2004 by Marcin Wiacek */

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <gammu-debug.h>

#include "../misc/coding/coding.h"
#include "gsmlogo.h"
#include "gsmnet.h"

/**
 * Helper define to check error code from fwrite.
 */
#define chk_fwrite(data, size, count, file) \
	if (fwrite(data, size, count, file) != count) goto fail;

void PHONE_GetBitmapWidthHeight(GSM_Phone_Bitmap_Types Type, size_t *width, size_t *height)
{
	*width  = 0;
	*height	= 0;
	switch (Type) {
		case GSM_EMSSmallPicture	: *width=8;  *height=8;  break;
		case GSM_EMSMediumPicture	: *width=16; *height=16; break;
		case GSM_EMSBigPicture		: *width=32; *height=32; break;
		case GSM_NokiaOperatorLogo	:
		case GSM_NokiaCallerLogo	: *width=72; *height=14; break;
		case GSM_NokiaPictureImage	: *width=72; *height=28; break;
		case GSM_Nokia7110OperatorLogo	:
		case GSM_Nokia6510OperatorLogo	: *width=78; *height=21; break;
		case GSM_NokiaStartupLogo	: *width=84; *height=48; break;
		case GSM_Nokia6210StartupLogo	: *width=96; *height=60; break;
		case GSM_Nokia7110StartupLogo	: *width=96; *height=65; break;
		case GSM_EMSVariablePicture	: 			 break;
		case GSM_AlcatelBMMIPicture	:			 break;
	}
}

size_t PHONE_GetBitmapSize(GSM_Phone_Bitmap_Types Type, size_t Width, size_t Height)
{
	size_t width, height, x;

	PHONE_GetBitmapWidthHeight(Type, &width, &height);
	if (width == 0 && height == 0) {
		width  = Width;
		height = Height;
	}
	switch (Type) {
		case GSM_Nokia6510OperatorLogo:
			x = width * height;
			return x/8 + (x%8 > 0);
		case GSM_Nokia7110OperatorLogo:
			return (width*height + 7)/8;
		case GSM_NokiaStartupLogo:
		case GSM_NokiaOperatorLogo:
		case GSM_NokiaCallerLogo:
		case GSM_NokiaPictureImage:
		case GSM_EMSSmallPicture:
		case GSM_EMSMediumPicture:
		case GSM_EMSBigPicture:
		case GSM_EMSVariablePicture:
			return height*width/8;
		case GSM_Nokia7110StartupLogo:
		case GSM_Nokia6210StartupLogo:
			return (height+7)/8*width;
		case GSM_AlcatelBMMIPicture:
			return width*((height+7)/8);
	}
	return 0;
}

static bool PHONE_IsPointBitmap(GSM_Phone_Bitmap_Types Type, char *buffer, int x, int y, int width, int height)
{
	int i=0, pixel;

	if (x > width || y > height) return false;

	switch (Type) {
	case GSM_NokiaStartupLogo:
	case GSM_Nokia6210StartupLogo:
	case GSM_Nokia7110StartupLogo:
	case GSM_Nokia6510OperatorLogo:
		i=(buffer[(y / 8 * width) + x] & (1 << (y % 8)));
		break;
	case GSM_NokiaOperatorLogo:
	case GSM_Nokia7110OperatorLogo:
	case GSM_NokiaCallerLogo:
	case GSM_EMSVariablePicture:
	case GSM_EMSSmallPicture:
	case GSM_EMSMediumPicture:
	case GSM_EMSBigPicture:
		pixel=width*y + x;
		i=(buffer[pixel / 8] & (1 << (7 - (pixel % 8))));
		break;
	case GSM_NokiaPictureImage:
		i=(buffer[(9 * y) + (x / 8)] & (1 << (7 - (x % 8))));
		break;
	case GSM_AlcatelBMMIPicture:
		break;
	}
	if (i) return true; else return false;
}

static void PHONE_SetPointBitmap(GSM_Phone_Bitmap_Types Type, char *buffer, int x, int y, int width, int height)
{
	int pixel;

	switch (Type) {
	case GSM_NokiaStartupLogo:
	case GSM_Nokia6210StartupLogo:
	case GSM_Nokia7110StartupLogo:
	case GSM_Nokia6510OperatorLogo:
		buffer[(y / 8 * width)+x] |= 1 << (y % 8);
		break;
	case GSM_NokiaOperatorLogo:
	case GSM_Nokia7110OperatorLogo:
	case GSM_NokiaCallerLogo:
	case GSM_EMSSmallPicture:
	case GSM_EMSMediumPicture:
	case GSM_EMSBigPicture:
	case GSM_EMSVariablePicture:
		pixel = width*y + x;
		buffer[pixel / 8] |= 1 << (7 - (pixel % 8));
		break;
	case GSM_NokiaPictureImage:
		buffer[(9 * y) + (x / 8)] |= 1 << (7 - (x % 8));
		break;
	case GSM_AlcatelBMMIPicture:
		pixel = height / 8;
		if ((height % 8) != 0) pixel++;
		buffer[(pixel * x) + (y / 8)] |= 1 << (7 - (y % 8));
		break;
	}
}

void PHONE_DecodeBitmap(GSM_Phone_Bitmap_Types Type, char *buffer, GSM_Bitmap *Bitmap)
{
	size_t width, height, x,y;

	PHONE_GetBitmapWidthHeight(Type, &width, &height);
	if (Type != GSM_Nokia6510OperatorLogo && Type != GSM_Nokia7110OperatorLogo && Type != GSM_EMSVariablePicture) {
		Bitmap->BitmapHeight	= height;
		Bitmap->BitmapWidth	= width;
	}
	switch (Type) {
		case GSM_NokiaOperatorLogo	:
		case GSM_Nokia7110OperatorLogo	:
		case GSM_Nokia6510OperatorLogo	: Bitmap->Type=GSM_OperatorLogo;	break;
		case GSM_NokiaCallerLogo	: Bitmap->Type=GSM_CallerGroupLogo;	break;
		case GSM_AlcatelBMMIPicture     :
		case GSM_NokiaStartupLogo	:
		case GSM_Nokia7110StartupLogo	:
		case GSM_Nokia6210StartupLogo	: Bitmap->Type=GSM_StartupLogo;		break;
		case GSM_NokiaPictureImage	:
		case GSM_EMSVariablePicture	:
    		case GSM_EMSSmallPicture	:
		case GSM_EMSMediumPicture	:
		case GSM_EMSBigPicture		: Bitmap->Type=GSM_PictureImage;	break;
	}

	Bitmap->Location		= 0;
	Bitmap->Text[0]			= 0;
	Bitmap->Text[1]			= 0;
	Bitmap->BitmapEnabled		= false;
	Bitmap->DefaultName		= false;
	Bitmap->DefaultBitmap		= false;
	Bitmap->DefaultRingtone		= false;
	Bitmap->RingtoneID		= 0;
	Bitmap->NetworkCode[0]		= 0;
	Bitmap->Sender[0]		= 0;
	Bitmap->Sender[1]		= 0;
	Bitmap->ID			= 0;
	Bitmap->Name			= NULL;

	GSM_ClearBitmap(Bitmap);
	for (x=0;x<Bitmap->BitmapWidth;x++) {
		for (y=0;y<Bitmap->BitmapHeight;y++) {
			if (PHONE_IsPointBitmap(Type, buffer, x, y, Bitmap->BitmapWidth, Bitmap->BitmapHeight)) {
				GSM_SetPointBitmap(Bitmap,x,y);
			}
		}
	}
}

void PHONE_ClearBitmap(GSM_Phone_Bitmap_Types Type, char *buffer, size_t width, size_t height)
{
	memset(buffer,0,PHONE_GetBitmapSize(Type,width,height));
}

void PHONE_EncodeBitmap(GSM_Phone_Bitmap_Types Type, char *buffer, GSM_Bitmap *Bitmap)
{
	size_t		width, height, x, y;
	GSM_Bitmap	dest;

	PHONE_GetBitmapWidthHeight(Type, &width, &height);
	if (width == 0 && height == 0) {
		width  = Bitmap->BitmapWidth;
		height = Bitmap->BitmapHeight;
	}
	GSM_ResizeBitmap(&dest, Bitmap, width, height);
	PHONE_ClearBitmap(Type, buffer, width, height);

	for (x=0;x<width;x++) {
		for (y=0;y<height;y++) {
			if (GSM_IsPointBitmap(&dest,x,y)) PHONE_SetPointBitmap(Type, buffer, x, y, width, height);
		}
	}
}

void GSM_GetMaxBitmapWidthHeight(GSM_Bitmap_Types Type, size_t *width, size_t *height)
{
	switch (Type) {
		case GSM_CallerGroupLogo: *width=72; *height=14; break;
		case GSM_OperatorLogo	: *width=101;*height=21; break;
		case GSM_StartupLogo	: *width=96; *height=65; break;
		case GSM_PictureImage	: *width=72; *height=28; break;
		default			:			 break;
	}
}

void GSM_SetPointBitmap(GSM_Bitmap *bmp, int x, int y)
{
	SetBit(bmp->BitmapPoints,y*bmp->BitmapWidth+x);
}

void GSM_ClearPointBitmap(GSM_Bitmap *bmp, int x, int y)
{
	ClearBit(bmp->BitmapPoints,y*bmp->BitmapWidth+x);
}

bool GSM_IsPointBitmap(GSM_Bitmap *bmp, int x, int y)
{
	if (GetBit(bmp->BitmapPoints, (y * bmp->BitmapWidth) + x)) {
		return true;
	} else {
		return false;
	}
}

void GSM_ClearBitmap(GSM_Bitmap *bmp)
{
	memset(bmp->BitmapPoints,0,GSM_GetBitmapSize(bmp));
}

size_t GSM_GetBitmapSize(GSM_Bitmap *bmp)
{
	return ((bmp->BitmapWidth * bmp->BitmapHeight) / 8) + 1;
}

void GSM_PrintBitmap(FILE *file, GSM_Bitmap *bitmap)
{
	size_t x,y;

	for (y=0;y<bitmap->BitmapHeight;y++) {
		for (x=0;x<bitmap->BitmapWidth;x++) {
			if (GSM_IsPointBitmap(bitmap,x,y)) {
				fprintf(file,"#");
			} else {
				fprintf(file," ");
			}
		}
		fprintf(file,"\n");
	}
}

void GSM_ReverseBitmap(GSM_Bitmap *Bitmap)
{
	size_t x, y;

	for (x=0;x<Bitmap->BitmapWidth;x++) {
		for (y=0;y<Bitmap->BitmapHeight;y++) {
			if (GSM_IsPointBitmap(Bitmap,x,y)) {
				GSM_ClearPointBitmap(Bitmap, x, y);
			} else {
				GSM_SetPointBitmap(Bitmap, x, y);
			}
		}
	}
}

void GSM_ResizeBitmap(GSM_Bitmap *dest, GSM_Bitmap *src, size_t width, size_t height)
{
	size_t startx=0,endx=0,setx=0, starty=0,endy=0,sety=0, x, y;

	if (src->BitmapWidth<=width) {
		startx	= 0;
		endx	= src->BitmapWidth;
		setx	= (width-src->BitmapWidth)/2;
	} else {
		startx	= (src->BitmapWidth-width)/2;
		endx	= startx + width;
		setx	= 0;
	}
	if (src->BitmapHeight<=height) {
		starty	= 0;
		endy	= src->BitmapHeight;
		sety	= (height-src->BitmapHeight)/2;
	} else {
		starty	= (src->BitmapHeight-height)/2;
		endy	= starty + height;
		sety	= 0;
	}
	dest->BitmapHeight	= height;
	dest->BitmapWidth	= width;
	GSM_ClearBitmap(dest);
	for (x=startx;x<endx;x++) {
		for (y=starty;y<endy;y++) {
			if (GSM_IsPointBitmap(src,x,y))
				GSM_SetPointBitmap(dest,setx+x-startx,sety+y-starty);
		}
	}
}

GSM_Error Bitmap2BMP(unsigned char *buffer, FILE *file,GSM_Bitmap *bitmap)
{
	size_t		x,i,sizeimage,buffpos=0;
	ssize_t		y, pos;
	unsigned char	buff[1];
	div_t		division;
	bool		isfile=false;

	unsigned char header[]={
/*1'st header*/   'B','M',             /* BMP file ID */
                  0x00,0x00,0x00,0x00, /* Size of file */
		  0x00,0x00,           /* Reserved for future use */
		  0x00,0x00,           /* Reserved for future use */
	            62,0x00,0x00,0x00, /* Offset for image data */

/*2'nd header*/     40,0x00,0x00,0x00, /* Length of this part of header */
		  0x00,0x00,0x00,0x00, /* Width of image */
		  0x00,0x00,0x00,0x00, /* Height of image */
		     1,0x00,           /* How many planes in target device */
		     1,0x00,           /* How many colors in image. 1 means 2^1=2 colors */
		  0x00,0x00,0x00,0x00, /* Type of compression. 0 means no compression */
/*Sometimes */    0x00,0x00,0x00,0x00, /* Size of part with image data */
/*ttttttt...*/    0xE8,0x03,0x00,0x00, /* XPelsPerMeter */
/*hhiiiiissss*/   0xE8,0x03,0x00,0x00, /* YPelsPerMeter */
/*part of header*/0x02,0x00,0x00,0x00, /* How many colors from palette is used */
/*doesn't exist*/ 0x00,0x00,0x00,0x00, /* How many colors from palette is required to display image. 0 means all */

/*Color palette*/ 0x00,0x00,0x00,      /* First color in palette in Blue, Green, Red. Here white */
		  0x00,                /* Each color in palette is end by 4'th byte */
                   102, 204, 102,      /* Second color in palette in Blue, Green, Red. Here green */
		  0x00};               /* Each color in palette is end by 4'th byte */

	if (file!=NULL) isfile=true;

	header[22]=bitmap->BitmapHeight;
	header[18]=bitmap->BitmapWidth;

	pos	  = 7;
	sizeimage = 0;
	/*lines are written from the last to the first*/
	for (y = bitmap->BitmapHeight - 1; y >= 0; y--) {
		i=1;
		for (x=0;x<bitmap->BitmapWidth;x++) {
			/*new byte !*/
			if (pos==7) {
				if (x!=0) sizeimage++;
				i++;
				/*each line is written in multiply of 4 bytes*/
				if(i==5) i=1;
			}
			pos--;
			/*going to new byte*/
			if (pos<0) pos=7;
		}
		/*going to new byte*/
		pos=7;
		sizeimage++;
		if (i!=1) {
			/*each line is written in multiply of 4 bytes*/
			while (i!=5) {
				sizeimage++;
				i++;
			}
		}
	}
	dbgprintf("Data size in BMP file: " SIZE_T_FORMAT "\n",sizeimage);
	division=div(sizeimage,256);
	header[35]=division.quot;
	header[34]=sizeimage-(division.quot*256);
  	sizeimage=sizeimage+sizeof(header);
	dbgprintf("Size of BMP file: " SIZE_T_FORMAT "\n",sizeimage);
	division=div(sizeimage,256);
	header[3]=division.quot;
	header[2]=sizeimage-(division.quot*256);

	if (isfile) {
		chk_fwrite(header,1,sizeof(header),file);
	} else {
		memcpy(buffer,header,sizeof(header));
		buffpos += sizeof(header);
	}

	pos=7;
	/*lines are written from the last to the first*/
	for (y=bitmap->BitmapHeight-1;y>=0;y--) {
		i=1;
		for (x=0;x<bitmap->BitmapWidth;x++) {
			/*new byte !*/
			if (pos==7) {
				if (x!=0) {
					if (isfile) {
						chk_fwrite(buff, 1, sizeof(buff), file);
					} else {
						memcpy (buffer+buffpos,buff,1);
						buffpos++;
					}
				}
				i++;
				/*each line is written in multiply of 4 bytes*/
				if(i==5) i=1;
				buff[0]=0;
			}
			if (!GSM_IsPointBitmap(bitmap,x,y)) buff[0]|=(1<<pos);
			pos--;
			/*going to new byte*/
			if (pos<0) pos=7;
		}
		/*going to new byte*/
		pos=7;
		if (isfile) {
			chk_fwrite(buff, 1, sizeof(buff), file);
		} else {
			memcpy (buffer+buffpos,buff,1);
			buffpos++;
		}
		if (i!=1) {
			/*each line is written in multiply of 4 bytes*/
			while (i!=5) {
				buff[0]=0;
				if (isfile) {
					chk_fwrite(buff, 1, sizeof(buff), file);
				} else {
					memcpy (buffer+buffpos,buff,1);
					buffpos++;
				}
				i++;
			}
		}
	}
	return ERR_NONE;
fail:
	return ERR_WRITING_FILE;
}

static GSM_Error savebmp(FILE *file, GSM_MultiBitmap *bitmap)
{
	GSM_Error error;

	error=Bitmap2BMP(NULL,file,&bitmap->Bitmap[0]);
	return error;
}

static GSM_Error PrivSaveNLMWBMP(FILE *file, GSM_Bitmap *Bitmap)
{
	unsigned char	buffer[1000];
	size_t		x,y,pos;
	ssize_t		pos2;
	div_t		division;

	pos=0;pos2=7;
	for (y=0;y<Bitmap->BitmapHeight;y++) {
		for (x=0;x<Bitmap->BitmapWidth;x++) {
			if (pos2==7) buffer[pos]=0;
      			if (GSM_IsPointBitmap(Bitmap,x,y)) buffer[pos]|=(1<<pos2);
			pos2--;
			/* going to new line */
			if (pos2<0) {pos2=7;pos++;}
		}
		/* for startup logos - new line with new byte */
		if (pos2!=7) {pos2=7;pos++;}
	}

	division=div(Bitmap->BitmapWidth,8);
	/* For startup logos */
	if (division.rem!=0) division.quot++;

	chk_fwrite(buffer,1,(size_t)(division.quot*Bitmap->BitmapHeight),file);
	return ERR_NONE;
fail:
	return ERR_WRITING_FILE;
}

static GSM_Error savenlm(FILE *file, GSM_MultiBitmap *bitmap)
{
	int  i;
	GSM_Error error;
	char header[]={
		'N','L','M',' ', /* Nokia Logo Manager file ID. */
		0x01,
		0x00,		 /* 0x00 (OP), 0x01 (CLI), 0x02 (Startup), 0x03 (Picture)*/
		0x00,		 /* Number of images inside file - 1. 0x01==2 images, 0x03==4 images, etc. */
		0x00,		 /* Width. */
		0x00,		 /* Height. */
		0x01};

	switch (bitmap->Bitmap[0].Type) {
		case GSM_OperatorLogo    : header[5]=0x00; break;
		case GSM_CallerGroupLogo : header[5]=0x01; break;
		case GSM_StartupLogo     : header[5]=0x02; break;
		case GSM_PictureImage    : header[5]=0x03; break;
		default			 : return ERR_UNKNOWN;
	}
	header[6] = bitmap->Number - 1;
	header[7] = bitmap->Bitmap[0].BitmapWidth;
	header[8] = bitmap->Bitmap[0].BitmapHeight;
	chk_fwrite(header,1,sizeof(header),file);

	for (i=0;i<bitmap->Number;i++) {
		error = PrivSaveNLMWBMP(file, &bitmap->Bitmap[i]);
		if (error != ERR_NONE) {
			return error;
		}
	}

	return ERR_NONE;
fail:
	return ERR_WRITING_FILE;
}

static GSM_Error PrivSaveNGGNOL(FILE *file, GSM_MultiBitmap *bitmap)
{
	char 	buffer[GSM_BITMAP_SIZE];
	size_t	x,y;
	size_t	current=0;

	for (y=0;y<bitmap->Bitmap[0].BitmapHeight;y++) {
		for (x=0;x<bitmap->Bitmap[0].BitmapWidth;x++) {
			if (GSM_IsPointBitmap(&bitmap->Bitmap[0],x,y)) {
				buffer[current++] = '1';
			} else {
				buffer[current++] = '0';
			}
		}
	}
	chk_fwrite(buffer,1,current,file);
	return ERR_NONE;
fail:
	return ERR_WRITING_FILE;
}

static GSM_Error savengg(FILE *file, GSM_MultiBitmap *bitmap)
{
  	char header[]={
		'N','G','G',0x00,0x01,0x00,
		0x00,0x00,	/* Width */
		0x00,0x00,	/* Height */
		0x01,0x00,0x01,0x00,
		0x00,		/* Unknown.Can't be checksum - for */
				/* the same logo files can be different */
		0x00};

	header[6] = bitmap->Bitmap[0].BitmapWidth;
	header[8] = bitmap->Bitmap[0].BitmapHeight;
	chk_fwrite(header,1,sizeof(header),file);

	return PrivSaveNGGNOL(file,bitmap);

fail:
	return ERR_WRITING_FILE;
}

static GSM_Error savenol(FILE *file, GSM_MultiBitmap *bitmap)
{
	int 	country,net;
	char 	header[]={
			'N','O','L',0x00,0x01,0x00,
			0x00,0x00,		/* MCC */
			0x00,0x00,		/* MNC */
			0x00,0x00,		/* Width */
			0x00,0x00,		/* Height */
			0x01,0x00,0x01,0x00,
			0x00,			/* Unknown.Can't be checksum - for */
						/* the same logo files can be different */
			0x00};

	if (bitmap->Bitmap[0].Type == GSM_OperatorLogo) sscanf(bitmap->Bitmap[0].NetworkCode, "%d %d", &country, &net);

	header[6]	= country%256;
	header[7]	= country/256;
	header[8]	= net%256;
	header[9]	= net/256;
	header[10]	= bitmap->Bitmap[0].BitmapWidth;
	header[12]	= bitmap->Bitmap[0].BitmapHeight;
	chk_fwrite(header,1,sizeof(header),file);

	return PrivSaveNGGNOL(file,bitmap);

fail:
	return ERR_WRITING_FILE;
}

static GSM_Error savexpm(FILE *file, GSM_MultiBitmap *bitmap)
{
	size_t x,y;

	fprintf(file,"/* XPM */\n");
	fprintf(file,"static char * ala_xpm[] = {\n");
	fprintf(file,"\"" SIZE_T_FORMAT " " SIZE_T_FORMAT " 2 1\",\n",
		bitmap->Bitmap[0].BitmapWidth,
		bitmap->Bitmap[0].BitmapHeight);
	fprintf(file,"\".	s c	m #000000	g4 #000000	g #000000	c #000000\",\n");
	fprintf(file,"\"#	s c	m #ffffff	g4 #ffffff	g #ffffff	c #ffffff\",\n");

	for (y=0;y<bitmap->Bitmap[0].BitmapHeight;y++) {
		fprintf(file,"\"");
		for (x=0;x<bitmap->Bitmap[0].BitmapWidth;x++)
			if (GSM_IsPointBitmap(&bitmap->Bitmap[0],x,y)) {
				fprintf(file,".");
			} else {
				fprintf(file,"#");
			}
		fprintf(file,"\"");
		if (y==bitmap->Bitmap[0].BitmapHeight-1) {
			fprintf(file,"};\n");
		} else {
			fprintf(file,",\n");
		}
	}

	return ERR_NONE;
}

static GSM_Error savensl(FILE *file, GSM_MultiBitmap *bitmap)
{
	char 		buffer[GSM_BITMAP_SIZE];
	unsigned char 	header[]={
		'F','O','R','M', 0x01,0xFE,	/* File ID block,      size 1*256+0xFE=510*/
		'N','S','L','D', 0x01,0xF8};	/* Startup Logo block, size 1*256+0xF8=504*/

	chk_fwrite(header,1,sizeof(header),file);
	PHONE_EncodeBitmap(GSM_NokiaStartupLogo, buffer, &bitmap->Bitmap[0]);
	chk_fwrite(buffer,1,PHONE_GetBitmapSize(GSM_NokiaStartupLogo,0,0),file);

	return ERR_NONE;
fail:
	return ERR_WRITING_FILE;
}

static GSM_Error savewbmp(FILE *file, GSM_MultiBitmap *bitmap)
{
	unsigned char buffer[4];

	buffer[0] = 0x00;
	buffer[1] = 0x00;
	buffer[2] = bitmap->Bitmap[0].BitmapWidth;
	buffer[3] = bitmap->Bitmap[0].BitmapHeight;
	chk_fwrite(buffer,1,4,file);

	return PrivSaveNLMWBMP(file, &bitmap->Bitmap[0]);

fail:
	return ERR_WRITING_FILE;
}

GSM_Error GSM_SaveBitmapFile(char *FileName, GSM_MultiBitmap *bitmap)
{
	FILE		*file;
	GSM_Error 	error=ERR_NONE;

	file = fopen(FileName, "wb");
	if (file == NULL) return ERR_CANTOPENFILE;

	/* Attempt to identify filetype */
	if (strcasestr(FileName,".nlm")) {
		error=savenlm(file,bitmap);
	} else if (strcasestr(FileName,".ngg")) {
		error=savengg(file,bitmap);
	} else if (strcasestr(FileName,".nol")) {
		error=savenol(file,bitmap);
	} else if (strcasestr(FileName,".xpm")) {
		error=savexpm(file,bitmap);
	} else if (strcasestr(FileName,".nsl")) {
		error=savensl(file,bitmap);
	} else if (strcasestr(FileName,".wbmp")) {
		error=savewbmp(file,bitmap);
	} else {
		error=savebmp(file,bitmap);
	}
	fclose(file);

	return error;
}

GSM_Error BMP2Bitmap(unsigned char *buffer, FILE *file,GSM_Bitmap *bitmap)
{
	bool		first_white,isfile=false;
	unsigned char 	buff[34];
	size_t		w,h,x,i,buffpos=0;
	ssize_t		y, pos;
	size_t		readbytes;
#ifdef DEBUG
	int		sizeimage=0;
#endif

	if (bitmap->Type == GSM_None) bitmap->Type = GSM_StartupLogo;
	if (file!=NULL) isfile=true;
	if (isfile) {
		readbytes = fread(buff, 1, 34, file);
		if (readbytes != 34) return ERR_FILENOTSUPPORTED;
	} else {
		memcpy(buff,buffer,34);
	}

	/* height and width of image in the file */
	h=buff[22]+256*buff[21];
	w=buff[18]+256*buff[17];
	dbgprintf("Image Size in BMP file: " SIZE_T_FORMAT "x" SIZE_T_FORMAT "\n",w,h);

	GSM_GetMaxBitmapWidthHeight(bitmap->Type, &bitmap->BitmapWidth, &bitmap->BitmapHeight);
	if (h<bitmap->BitmapHeight)	bitmap->BitmapHeight=h;
	if (w<bitmap->BitmapWidth)	bitmap->BitmapWidth=w;
	dbgprintf("Height " SIZE_T_FORMAT " " SIZE_T_FORMAT ", width " SIZE_T_FORMAT " " SIZE_T_FORMAT "\n",h,bitmap->BitmapHeight,w,bitmap->BitmapWidth);

	GSM_ClearBitmap(bitmap);

#ifdef DEBUG
	dbgprintf("Number of colors in BMP file: ");
	switch (buff[28]) {
		case 1	: dbgprintf("2 (supported)\n");		   	break;
		case 4	: dbgprintf("16 (NOT SUPPORTED)\n");	   	break;
		case 8	: dbgprintf("256 (NOT SUPPORTED)\n");	   	break;
		case 24	: dbgprintf("True Color (NOT SUPPORTED)\n"); 	break;
		default	: dbgprintf("unknown\n");			break;
	}
#endif
	if (buff[28]!=1) {
		dbgprintf("Wrong number of colors\n");
		return ERR_FILENOTSUPPORTED;
	}

#ifdef DEBUG
	dbgprintf("Compression in BMP file: ");
	switch (buff[30]) {
		case 0	:dbgprintf("no compression (supported)\n"); 	break;
		case 1	:dbgprintf("RLE8 (NOT SUPPORTED)\n");	  	break;
		case 2	:dbgprintf("RLE4 (NOT SUPPORTED)\n");	  	break;
		default	:dbgprintf("unknown\n");			break;
	}
#endif
	if (buff[30]!=0) {
		dbgprintf("Compression type not supported\n");
		return ERR_FILENOTSUPPORTED;
	}

	/* read rest of header (if exists) and color palette */
	if (isfile) {
		pos=buff[10]-34;
		readbytes = fread(buff, 1, pos, file);
		if (readbytes != (size_t)pos) return ERR_FILENOTSUPPORTED;
	} else {
		pos=buff[10]-34;
		buffpos=buff[10];
		memcpy (buff,buffer+34,pos);
	}

#ifdef DEBUG
	dbgprintf("First color in BMP file: %i %i %i ",buff[pos-8], buff[pos-7], buff[pos-6]);
	if (buff[pos-8]==0    && buff[pos-7]==0    && buff[pos-6]==0)    dbgprintf("(white)");
	if (buff[pos-8]==0xFF && buff[pos-7]==0xFF && buff[pos-6]==0xFF) dbgprintf("(black)");
	if (buff[pos-8]==102  && buff[pos-7]==204  && buff[pos-6]==102)  dbgprintf("(green)");
	dbgprintf("\n");
	dbgprintf("Second color in BMP file: %i %i %i ",buff[pos-38], buff[pos-37], buff[pos-36]);
	if (buff[pos-4]==0    && buff[pos-3]==0    && buff[pos-2]==0)    dbgprintf("(white)");
	if (buff[pos-4]==0xFF && buff[pos-3]==0xFF && buff[pos-2]==0xFF) dbgprintf("(black)");
	dbgprintf("\n");
#endif
	first_white=true;
	if (buff[pos-8]!=0 || buff[pos-7]!=0 || buff[pos-6]!=0) first_white=false;

	pos=7;
	/* lines are written from the last to the first */
	for (y=h-1;y>=0;y--) {
		i=1;
		for (x=0;x<w;x++) {
			/* new byte ! */
			if (pos==7) {
				if (isfile) {
					readbytes = fread(buff, 1, 1, file);
					if (readbytes != 1) return ERR_FILENOTSUPPORTED;
				} else {
					memcpy (buff,buffer+buffpos,1);
					buffpos++;
				}
#ifdef DEBUG
				sizeimage++;
#endif
				i++;
				/* each line is written in multiply of 4 bytes */
				if(i==5) i=1;
			}
			/* we have top left corner ! */
			if (x<=bitmap->BitmapWidth && (size_t)y<=bitmap->BitmapHeight) {
				if (first_white) {
					if ((buff[0]&(1<<pos))<=0) GSM_SetPointBitmap(bitmap,x,y);
				} else {
					if ((buff[0]&(1<<pos))>0) GSM_SetPointBitmap(bitmap,x,y);
				}
			}
			pos--;
			/* going to new byte */
			if (pos<0) pos=7;
		}
		/* going to new byte */
		pos=7;
		if (i!=1) {
			/* each line is written in multiply of 4 bytes */
			while (i!=5) {
				if (isfile) {
					readbytes = fread(buff, 1, 1, file);
					if (readbytes != 1) return ERR_FILENOTSUPPORTED;
				} else {
					memcpy (buff,buffer+buffpos,1);
					buffpos++;
				}
#ifdef DEBUG
				sizeimage++;
#endif
				i++;
			}
		}
	}
#ifdef DEBUG
	dbgprintf("Data size in BMP file: %i\n",sizeimage);
#endif
	return(ERR_NONE);
}

static GSM_Error loadbmp(FILE *file, GSM_MultiBitmap *bitmap)
{
	GSM_Error error;

	error=BMP2Bitmap(NULL,file,&bitmap->Bitmap[0]);
	bitmap->Number = 1;
	return error;
}

static GSM_Error loadnlm (FILE *file, GSM_MultiBitmap *bitmap)
{
	unsigned char 	buffer[1000];
	size_t 		pos,x,y,h,w,i,number;
	ssize_t		pos2;
	div_t		division;
	size_t		readbytes;

	readbytes = fread(buffer,1,5,file);

	readbytes = fread(buffer,1,1,file);
	if (readbytes != 1) return ERR_FILENOTSUPPORTED;

	switch (buffer[0]) {
		case 0x00:
			dbgprintf("Operator logo\n");
			if (bitmap->Bitmap[0].Type == GSM_None) bitmap->Bitmap[0].Type = GSM_OperatorLogo;
			break;
		case 0x01:
			dbgprintf("Caller logo\n");
			if (bitmap->Bitmap[0].Type == GSM_None) bitmap->Bitmap[0].Type = GSM_CallerGroupLogo;
			break;
		case 0x02:
			dbgprintf("Startup logo\n");
			if (bitmap->Bitmap[0].Type == GSM_None) bitmap->Bitmap[0].Type = GSM_StartupLogo;
			break;
		case 0x03:
			dbgprintf("Picture Image logo\n");
			if (bitmap->Bitmap[0].Type == GSM_None) bitmap->Bitmap[0].Type = GSM_PictureImage;
			break;
	}

	bitmap->Number = 0;
	readbytes = fread(buffer,1,4,file);
	if (readbytes != 4) return ERR_FILENOTSUPPORTED;
	number 	= buffer[0] + 1;
	w 	= buffer[1];
	h 	= buffer[2];
	for (i=0;i<number;i++) {
		bitmap->Bitmap[i].Type = bitmap->Bitmap[0].Type;
		GSM_GetMaxBitmapWidthHeight(bitmap->Bitmap[i].Type, &bitmap->Bitmap[i].BitmapWidth, &bitmap->Bitmap[i].BitmapHeight);
		if (h < bitmap->Bitmap[i].BitmapHeight)	bitmap->Bitmap[i].BitmapHeight	= h;
		if (w < bitmap->Bitmap[i].BitmapWidth)	bitmap->Bitmap[i].BitmapWidth		= w;

		division=div(w,8);
		/* For startup logos */
		if (division.rem!=0) division.quot++;
	  	if (fread(buffer,1,(division.quot*h),file)!=(unsigned int)(division.quot*h)) return ERR_UNKNOWN;

		GSM_ClearBitmap(&bitmap->Bitmap[i]);

		pos=0;pos2=7;
		for (y=0;y<h;y++) {
			for (x=0;x<w;x++) {
				if ((buffer[pos]&(1<<pos2))>0) {
					if (y<bitmap->Bitmap[i].BitmapHeight && x<bitmap->Bitmap[i].BitmapWidth) GSM_SetPointBitmap(&bitmap->Bitmap[i],x,y);
				}
				pos2--;
				/* going to new byte */
				if (pos2<0) {pos2=7;pos++;}
			}
			/* for startup logos-new line means new byte */
			if (pos2!=7) {pos2=7;pos++;}
		}
		bitmap->Number++;
		if (bitmap->Number == GSM_MAX_MULTI_BITMAP) break;
	}
	return (ERR_NONE);
}

static GSM_Error loadnolngg(FILE *file, GSM_MultiBitmap *bitmap, bool nolformat)
{
	unsigned char 	buffer[2000];
	size_t		i,h,w,x,y;
	size_t		readbytes;

	readbytes = fread(buffer, 1, 6, file);
	if (readbytes != 6) return ERR_FILENOTSUPPORTED;

	if (bitmap->Bitmap[0].Type == GSM_None) bitmap->Bitmap[0].Type = GSM_CallerGroupLogo;
	if (nolformat) {
		readbytes = fread(buffer, 1, 4, file);
		if (readbytes != 4) return ERR_FILENOTSUPPORTED;
		sprintf(bitmap->Bitmap[0].NetworkCode, "%d %02d", buffer[0]+256*buffer[1], buffer[2]);
		if (bitmap->Bitmap[0].Type == GSM_None) bitmap->Bitmap[0].Type = GSM_OperatorLogo;
	}

	readbytes = fread(buffer, 1, 4, file);
	if (readbytes != 4) return ERR_FILENOTSUPPORTED;

	w = buffer[0];
	h = buffer[2];
	GSM_GetMaxBitmapWidthHeight(bitmap->Bitmap[0].Type, &bitmap->Bitmap[0].BitmapWidth, &bitmap->Bitmap[0].BitmapHeight);
	if (h < bitmap->Bitmap[0].BitmapHeight)	bitmap->Bitmap[0].BitmapHeight	= h;
	if (w < bitmap->Bitmap[0].BitmapWidth)	bitmap->Bitmap[0].BitmapWidth	= w;

	/* Unknown bytes. */
	readbytes = fread(buffer, 1, 6, file);
	if (readbytes != 6) return ERR_FILENOTSUPPORTED;

	GSM_ClearBitmap(&bitmap->Bitmap[0]);

	x=0; y=0;
	for (i=0; i<w*h; i++) {
		if (fread(buffer, 1, 1, file)!=1) return ERR_FILENOTSUPPORTED;
		if (buffer[0]=='1') GSM_SetPointBitmap(&bitmap->Bitmap[0],x,y);
		x++;
		if (x==w) {x=0; y++;}
	}

#ifdef DEBUG
	/* Some programs writes here fileinfo */
	if (fread(buffer, 1, 1, file) == 1) {
		dbgprintf("Fileinfo: %c",buffer[0]);
		while (fread(buffer, 1, 1, file)==1) {
			if (buffer[0]!=0x0A) dbgprintf("%c",buffer[0]);
		}
		dbgprintf("\n");
	}
#endif
	bitmap->Number = 1;
	return(ERR_NONE);
}

static GSM_Error loadnsl(FILE *file, GSM_MultiBitmap *bitmap)
{
	unsigned char 		block[6],buffer[505];
	size_t 			block_size;
	size_t			readbytes;
	GSM_Bitmap_Types	OldType;

	while (fread(block,1,6,file)==6) {
		block_size = block[4]*256 + block[5];
		dbgprintf("Block %c%c%c%c, size " SIZE_T_FORMAT "\n",block[0],block[1],block[2],block[3],block_size);
		if (!strncmp(block, "FORM", 4)) {
			dbgprintf("File ID\n");
		} else {
			if (block_size>504) return ERR_FILENOTSUPPORTED;
			if (block_size!=0) {
				readbytes = fread(buffer,1,block_size,file);
				if (readbytes != block_size) return ERR_FILENOTSUPPORTED;
				/* if it's string, we end it with 0 */
				buffer[block_size]=0;
#ifdef DEBUG
				if (!strncmp(block, "VERS", 4)) dbgprintf("File saved by: %s\n",buffer);
				if (!strncmp(block, "MODL", 4)) dbgprintf("Logo saved from: %s\n",buffer);
				if (!strncmp(block, "COMM", 4)) dbgprintf("Phone was connected to COM port: %s\n",buffer);
#endif
				if (!strncmp(block, "NSLD", 4)) {
					bitmap->Bitmap[0].BitmapHeight = 48;
					bitmap->Bitmap[0].BitmapWidth	 = 84;
					OldType = bitmap->Bitmap[0].Type;
					PHONE_DecodeBitmap(GSM_NokiaStartupLogo, buffer, &bitmap->Bitmap[0]);
					if (OldType != GSM_None) bitmap->Bitmap[0].Type = OldType;
					dbgprintf("Startup logo (size " SIZE_T_FORMAT  ")\n",block_size);
				}
			}
		}
	}
	bitmap->Number = 1;
	return(ERR_NONE);
}

static GSM_Error loadwbmp(FILE *file, GSM_MultiBitmap *bitmap)
{
	unsigned char buffer[10000];
	size_t readbytes;

	readbytes = fread(buffer,1,4,file);
	if (readbytes != 4) return ERR_FILENOTSUPPORTED;
	bitmap->Bitmap[0].BitmapWidth  = buffer[2];
	bitmap->Bitmap[0].BitmapHeight = buffer[3];
	bitmap->Number 		       = 1;

	readbytes = fread(buffer,1,10000,file);
	/* FIXME: Decode function should receive how long is the buffer! */
	PHONE_DecodeBitmap(GSM_Nokia7110OperatorLogo, buffer, &bitmap->Bitmap[0]);
	GSM_ReverseBitmap(&bitmap->Bitmap[0]);

	return ERR_NONE;
}

static GSM_Error loadgif(FILE *file, GSM_MultiBitmap *bitmap)
{
	GSM_Bitmap 	*bmap = &bitmap->Bitmap[0];
	char 		*buffer;
	struct stat 	st;
	size_t readbytes, length;

	dbgprintf("loading gif file\n");
	fstat(fileno(file), &st);
        bmap->BinaryPic.Length = length = st.st_size;
	bmap->BinaryPic.Buffer = buffer = malloc(length);
	if (bmap->BinaryPic.Buffer == NULL)
		return ERR_MOREMEMORY;

	readbytes = fread(buffer, 1, length, file);
	if (readbytes != length) return ERR_FILENOTSUPPORTED;
        dbgprintf("Length " SIZE_T_FORMAT  " name \"%s\"\n", length,
		DecodeUnicodeString(bmap->Name));

	bmap->Type = GSM_PictureBinary;
	bmap->BinaryPic.Type = PICTURE_GIF;
	bmap->BitmapWidth = 256 * buffer[7] + buffer[6];
	bmap->BitmapHeight = 256 * buffer[9] + buffer[8];
	bitmap->Number = 1;

	return ERR_NONE;
}

GSM_Error GSM_ReadBitmapFile(char *FileName, GSM_MultiBitmap *bitmap)
{
	FILE		*file;
	unsigned char	buffer[300];
	GSM_Error	error = ERR_FILENOTSUPPORTED;

	file = fopen(FileName, "rb");
	if (file == NULL) return ERR_CANTOPENFILE;

	bitmap->Bitmap[0].Name = malloc((strlen(FileName) + 1) * 2);
	if (bitmap->Bitmap[0].Name == NULL) {
		fclose(file);
		return ERR_MOREMEMORY;
	}
	EncodeUnicode(bitmap->Bitmap[0].Name, FileName, strlen(FileName));

	/* Read the header of the file. */
	if (fread(buffer, 1, 9, file) != 9) {
		fclose(file);
		return ERR_FILENOTSUPPORTED;
	}
	rewind(file);

	bitmap->Bitmap[0].DefaultBitmap = false;

	/* Attempt to identify filetype */
	if (memcmp(buffer, "BM",2)==0) {
		error = loadbmp(file,bitmap);
	} else if (buffer[0] == 0x00 && buffer[1] == 0x00) {
		error = loadwbmp(file,bitmap);
	} else if (memcmp(buffer, "NLM",3)==0) {
		error = loadnlm(file,bitmap);
	} else if (memcmp(buffer, "NOL",3)==0) {
		error = loadnolngg(file,bitmap,true);
	} else if (memcmp(buffer, "NGG",3)==0) {
		error = loadnolngg(file,bitmap,false);
	} else if (memcmp(buffer, "FORM",4)==0) {
		error = loadnsl(file,bitmap);
	} else if (memcmp(buffer, "GIF",3)==0) {
		error = loadgif(file,bitmap);
	}
	fclose(file);
	return error;
}

void NOKIA_CopyBitmap(GSM_Phone_Bitmap_Types Type, GSM_Bitmap *Bitmap, char *Buffer, size_t *Length)
{
	size_t Width, Height;

	Buffer[(*Length)++] = 0x00;
	PHONE_GetBitmapWidthHeight(Type, &Width, &Height);
	Buffer[(*Length)++] = Width;
	Buffer[(*Length)++] = Height;
	Buffer[(*Length)++] = 0x01;
	PHONE_EncodeBitmap(Type, Buffer + (*Length), Bitmap);
	(*Length) = (*Length) + PHONE_GetBitmapSize(Type,0,0);
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
