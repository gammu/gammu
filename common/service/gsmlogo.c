
#include <string.h>
#include <stdlib.h>

#include "gsmlogo.h"
#include "../misc/misc.h"
#include "../misc/coding.h"
#include "gsmnet.h"

void PHONE_GetBitmapWidthHeight(GSM_Phone_Bitmap_Types Type, int *width, int *height)
{
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
	}
}

int PHONE_GetBitmapSize(GSM_Phone_Bitmap_Types Type)
{
	int width, height, x;

	PHONE_GetBitmapWidthHeight(Type, &width, &height);
	switch (Type) {
		case GSM_Nokia6510OperatorLogo:
			x = width * height;
			return (x / 8) + (x % 8 > 0);
		case GSM_Nokia7110OperatorLogo:
			return (width*height + 7)/8;
		case GSM_NokiaStartupLogo:
		case GSM_NokiaOperatorLogo:
		case GSM_NokiaCallerLogo:
		case GSM_NokiaPictureImage:
		case GSM_EMSSmallPicture:
		case GSM_EMSMediumPicture:
		case GSM_EMSBigPicture:
			return height*width/8;
		case GSM_Nokia7110StartupLogo:
		case GSM_Nokia6210StartupLogo:
			return (height+7)/8*width;
	}
	return 0;
}

static bool PHONE_IsPointBitmap(GSM_Phone_Bitmap_Types Type, char *buffer, int x, int y, int width, int height)
{
	int i=0, pixel;

	switch (Type) {
	case GSM_NokiaStartupLogo:
	case GSM_Nokia6210StartupLogo:
	case GSM_Nokia7110StartupLogo:
	case GSM_Nokia6510OperatorLogo:
		i=(buffer[((y/8)*width) + x] & 1<<((y%8)));
		break;
	case GSM_NokiaOperatorLogo:
	case GSM_Nokia7110OperatorLogo:
	case GSM_NokiaCallerLogo:
		pixel=width*y + x;
		i=(buffer[pixel/8] & 1<<(7-(pixel%8)));
		break;
	case GSM_NokiaPictureImage:
		i=(buffer[9*y + (x/8)] & 1<<(7-(x%8)));
		break;
	case GSM_EMSSmallPicture:
	case GSM_EMSMediumPicture:
	case GSM_EMSBigPicture:
		break;
	}
	if (i) return true; else return false;
}

void PHONE_SetPointBitmap(GSM_Phone_Bitmap_Types Type, char *buffer, int x, int y)
{
	int pixel, width, height;

	PHONE_GetBitmapWidthHeight(Type, &width, &height);
	switch (Type) {
	case GSM_NokiaStartupLogo:
	case GSM_Nokia6210StartupLogo:
	case GSM_Nokia7110StartupLogo:
	case GSM_Nokia6510OperatorLogo:
		buffer[((y/8)*width)+x] |= 1 << (y%8);		
		break;
	case GSM_NokiaOperatorLogo:
	case GSM_Nokia7110OperatorLogo:
	case GSM_NokiaCallerLogo:
	case GSM_EMSSmallPicture:
	case GSM_EMSMediumPicture:
	case GSM_EMSBigPicture:
		pixel=width*y + x;
		buffer[pixel/8] |= 1 << (7-(pixel%8));
		break;
	case GSM_NokiaPictureImage:
		buffer[9*y + (x/8)] |= 1 << (7-(x%8));
		break;
	}
}

void PHONE_DecodeBitmap(GSM_Phone_Bitmap_Types Type, char *buffer, GSM_Bitmap *Bitmap)
{
	int width, height, x,y;

	PHONE_GetBitmapWidthHeight(Type, &width, &height);
	if (Type != GSM_Nokia6510OperatorLogo && Type != GSM_Nokia7110OperatorLogo) {
		Bitmap->Height	= height;
		Bitmap->Width	= width;
	}
	switch (Type) {
		case GSM_NokiaOperatorLogo	:
		case GSM_Nokia7110OperatorLogo	:
		case GSM_Nokia6510OperatorLogo	: Bitmap->Type=GSM_OperatorLogo;	break;
		case GSM_NokiaCallerLogo	: Bitmap->Type=GSM_CallerLogo;		break;
		case GSM_NokiaStartupLogo	:
		case GSM_Nokia7110StartupLogo	:
		case GSM_Nokia6210StartupLogo	: Bitmap->Type=GSM_StartupLogo;		break;
		case GSM_NokiaPictureImage	: Bitmap->Type=GSM_PictureImage;	break;
    		case GSM_EMSSmallPicture	:
		case GSM_EMSMediumPicture	:
		case GSM_EMSBigPicture		:					break;
	}
	GSM_ClearBitmap(Bitmap);
	for (x=0;x<Bitmap->Width;x++) {
		for (y=0;y<Bitmap->Height;y++) {
			if (PHONE_IsPointBitmap(Type, buffer, x, y, Bitmap->Width, Bitmap->Height)) {
				GSM_SetPointBitmap(Bitmap,x,y);
			}
		}
	}
}

void PHONE_ClearBitmap(GSM_Phone_Bitmap_Types Type, char *buffer)
{
	memset(buffer,0,PHONE_GetBitmapSize(Type));
}

void PHONE_EncodeBitmap(GSM_Phone_Bitmap_Types Type, char *buffer, GSM_Bitmap *Bitmap)
{
	int		width, height;
	int		x, y;
	GSM_Bitmap	dest;

	PHONE_GetBitmapWidthHeight(Type, &width, &height);
	GSM_ResizeBitmap(&dest, Bitmap, width, height);
	PHONE_ClearBitmap(Type, buffer);
	for (x=0;x<width;x++) {
		for (y=0;y<height;y++) {
			if (GSM_IsPointBitmap(&dest,x,y)) PHONE_SetPointBitmap(Type, buffer, x, y);
		}
	}
}

void GSM_GetMaxBitmapWidthHeight(GSM_Bitmap_Types Type, unsigned char *width, unsigned char *height)
{
	switch (Type) {
		case GSM_CallerLogo	: *width=72; *height=14; break;
		case GSM_OperatorLogo	: *width=78; *height=21; break;
		case GSM_StartupLogo	: *width=96; *height=65; break;
		case GSM_PictureImage	: *width=72; *height=28; break;
		default			:			 break;
	}
}

void GSM_SetPointBitmap(GSM_Bitmap *bmp, int x, int y)
{
	SetBit(bmp->Bitmap,y*bmp->Width+x);
}

bool GSM_IsPointBitmap(GSM_Bitmap *bmp, int x, int y)
{
	if (GetBit(bmp->Bitmap,y*bmp->Width+x)) return true; else return false;
}

void GSM_ClearBitmap(GSM_Bitmap *bmp)
{
	memset(bmp->Bitmap,0,GSM_GetBitmapSize(bmp));
}

int GSM_GetBitmapSize(GSM_Bitmap *bmp)
{
	return bmp->Width*bmp->Height/8+1;
}

void GSM_PrintBitmap(FILE *file, GSM_Bitmap *bitmap)
{
	int x,y;

	for (y=0;y<bitmap->Height;y++) {
		for (x=0;x<bitmap->Width;x++) {
			if (GSM_IsPointBitmap(bitmap,x,y)) {
				fprintf(file,"#");
			} else {
				fprintf(file," ");
			}
		}
		fprintf(file,"\n");
	}
}

void GSM_ResizeBitmap(GSM_Bitmap *dest, GSM_Bitmap *src, int width, int height)
{
	int startx=0,endx=0,setx=0;
	int starty=0,endy=0,sety=0;
	int x, y;

	if (src->Width<=width) {
		startx	= 0;
		endx	= src->Width;
		setx	= (width-src->Width)/2;
	} else {
		startx	= (src->Width-width)/2;
		endx	= startx + width;
		setx	= 0;
	}
	if (src->Height<=height) {
		starty	= 0;
		endy	= src->Height;
		sety	= (height-src->Height)/2;
	} else {
		starty	= (src->Height-height)/2;
		endy	= starty + height;
		sety	= 0;
	}
	dest->Height=height;
	dest->Width=width;
	GSM_ClearBitmap(dest);
	for (x=startx;x<endx;x++) {
		for (y=starty;y<endy;y++) {
			if (GSM_IsPointBitmap(src,x,y))
				GSM_SetPointBitmap(dest,setx+x-startx,sety+y-starty);
		}
	}
}

static GSM_Error savebmp(FILE *file, GSM_MultiBitmap *bitmap)
{
	int		x,y,pos,i,sizeimage;
	unsigned char	buffer[1];
	div_t		division;
  
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

	header[22]=bitmap->Bitmap[0].Height;
	header[18]=bitmap->Bitmap[0].Width;
     
	pos=7;
	sizeimage=0;
	/*lines are written from the last to the first*/
	for (y=bitmap->Bitmap[0].Height-1;y>=0;y--) {
		i=1;
		for (x=0;x<bitmap->Bitmap[0].Width;x++) {
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
			while (i!=5)
			{
				sizeimage++;
				i++;
			}
		}
	}
	dprintf("Data size in BMP file: %i\n",sizeimage);
	division=div(sizeimage,256);
	header[35]=division.quot;
	header[34]=sizeimage-(division.quot*256);
  	sizeimage=sizeimage+sizeof(header);
	dprintf("Size of BMP file: %i\n",sizeimage);
	division=div(sizeimage,256);
	header[3]=division.quot;
	header[2]=sizeimage-(division.quot*256);
       
	fwrite(header,1,sizeof(header),file);

	pos=7;
	/*lines are written from the last to the first*/
	for (y=bitmap->Bitmap[0].Height-1;y>=0;y--) {
		i=1;
		for (x=0;x<bitmap->Bitmap[0].Width;x++) {
			/*new byte !*/
			if (pos==7) {
				if (x!=0) fwrite(buffer, 1, sizeof(buffer), file);
				i++;
				/*each line is written in multiply of 4 bytes*/
				if(i==5) i=1;
				buffer[0]=0;
			}
			if (!GSM_IsPointBitmap(&bitmap->Bitmap[0],x,y)) buffer[0]|=(1<<pos);
			pos--;
			/*going to new byte*/
			if (pos<0) pos=7;
		}
		/*going to new byte*/
		pos=7; 
		fwrite(buffer, 1, sizeof(buffer), file);
		if (i!=1) {
			/*each line is written in multiply of 4 bytes*/
			while (i!=5)
			{
				buffer[0]=0;
				fwrite(buffer, 1, sizeof(buffer), file);
				i++;
			}
		}
	}
	return GE_NONE;
}

static GSM_Error savenlm(FILE *file, GSM_MultiBitmap *bitmap)
{
	unsigned char	buffer[1000];
	int		x,y,pos,pos2,i;
	div_t		division;
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
		case GSM_CallerLogo      : header[5]=0x01; break;
		case GSM_StartupLogo     : header[5]=0x02; break;
		case GSM_PictureImage    : header[5]=0x03; break;
		default			 : return GE_UNKNOWN;
	}
	header[6] = bitmap->Number - 1;  
	header[7] = bitmap->Bitmap[0].Width;
	header[8] = bitmap->Bitmap[0].Height;
	fwrite(header,1,sizeof(header),file);
  
	for (i=0;i<bitmap->Number;i++) {
		pos=0;pos2=7;
		for (y=0;y<bitmap->Bitmap[i].Height;y++) {
			for (x=0;x<bitmap->Bitmap[i].Width;x++) {
				if (pos2==7) buffer[pos]=0;
	      			if (GSM_IsPointBitmap(&bitmap->Bitmap[i],x,y)) buffer[pos]|=(1<<pos2);
				pos2--;
				/* going to new line */
				if (pos2<0) {pos2=7;pos++;}
			}
			/* for startup logos - new line with new byte */
			if (pos2!=7) {pos2=7;pos++;}
		}
	  
		division=div(bitmap->Bitmap[i].Width,8);
		/* For startup logos */
		if (division.rem!=0) division.quot++;
	  
		fwrite(buffer,1,(division.quot*bitmap->Bitmap[i].Height),file);
	}

	return GE_NONE;
}

static void PrivSaveNGGNOL(FILE *file, GSM_MultiBitmap *bitmap)
{
	char 	buffer[GSM_BITMAP_SIZE];
	int	x,y,current=0;

	for (y=0;y<bitmap->Bitmap[0].Height;y++) {
		for (x=0;x<bitmap->Bitmap[0].Width;x++) {
			if (GSM_IsPointBitmap(&bitmap->Bitmap[0],x,y)) {
				buffer[current++] = '1';
			} else {
				buffer[current++] = '0';
			}
		}
	}
	fwrite(buffer,1,current,file);
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

	header[6] = bitmap->Bitmap[0].Width;
	header[8] = bitmap->Bitmap[0].Height;
	fwrite(header,1,sizeof(header),file);

	PrivSaveNGGNOL(file,bitmap);

	return GE_NONE;
}

static GSM_Error savenol(FILE *file, GSM_MultiBitmap *bitmap)
{
	int country,net;
	char header[]={
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
	header[10]	= bitmap->Bitmap[0].Width;
	header[12]	= bitmap->Bitmap[0].Height;
	fwrite(header,1,sizeof(header),file);

	PrivSaveNGGNOL(file,bitmap);  

	return GE_NONE;
}

static GSM_Error savexpm(FILE *file, GSM_MultiBitmap *bitmap)
{
	int x,y;

	fprintf(file,"/* XPM */\n");
	fprintf(file,"static char * ala_xpm[] = {\n");
	fprintf(file,"\"%i %i 2 1\",\n",bitmap->Bitmap[0].Width,bitmap->Bitmap[0].Height);
	fprintf(file,"\".	s c	m #000000	g4 #000000	g #000000	c #000000\",\n");
	fprintf(file,"\"#	s c	m #ffffff	g4 #ffffff	g #ffffff	c #ffffff\",\n");

	for (y=0;y<bitmap->Bitmap[0].Height;y++) {
		fprintf(file,"\"");
		for (x=0;x<bitmap->Bitmap[0].Width;x++)
			if (GSM_IsPointBitmap(&bitmap->Bitmap[0],x,y))
				fprintf(file,".");
			else
				fprintf(file,"#");
		fprintf(file,"\"");
		if (y==bitmap->Bitmap[0].Height-1)
			fprintf(file,"};\n");
		else
			fprintf(file,",\n");
	}

	return GE_NONE;
}

static GSM_Error savensl(FILE *file, GSM_MultiBitmap *bitmap)
{
	char buffer[GSM_BITMAP_SIZE];
	unsigned char header[]={
		'F','O','R','M', 0x01,0xFE,	/* File ID block,      size 1*256+0xFE=510*/
		'N','S','L','D', 0x01,0xF8};	/* Startup Logo block, size 1*256+0xF8=504*/

	fwrite(header,1,sizeof(header),file);
	PHONE_EncodeBitmap(GSM_NokiaStartupLogo, buffer, &bitmap->Bitmap[0]);
	fwrite(buffer,1,PHONE_GetBitmapSize(GSM_NokiaStartupLogo),file);

	return GE_NONE;
}

GSM_Error GSM_SaveBitmapFile(char *FileName, GSM_MultiBitmap *bitmap)
{
	FILE		*file;
	GSM_Error 	error=GE_NONE;
  
	file = fopen(FileName, "wb");      
	if (!file) return(GE_CANTOPENFILE);

	/* Attempt to identify filetype */
	if (strstr(FileName,".nlm")) {
		error=savenlm(file,bitmap);
	} else if (strstr(FileName,".ngg")) {
		error=savengg(file,bitmap);
	} else if (strstr(FileName,".nol")) {
		error=savenol(file,bitmap);
	} else if (strstr(FileName,".xpm")) {
		error=savexpm(file,bitmap);
	} else if (strstr(FileName,".nsl")) {
		error=savensl(file,bitmap);
	} else {
		error=savebmp(file,bitmap);
	}
	fclose(file);
   
	return error;
}

static GSM_Error loadbmp(FILE *file, GSM_MultiBitmap *bitmap)
{
	unsigned char	buffer[34];
	bool		first_white;
	int		w,h,pos,y,x,i;
#ifdef DEBUG
	int		sizeimage=0;
#endif

	if (bitmap->Bitmap[0].Type == GSM_None) bitmap->Bitmap[0].Type = GSM_StartupLogo;

	/* required part of header */
	fread(buffer, 1, 34, file);

	/* height and width of image in the file */
	h=buffer[22]+256*buffer[21]; 
	w=buffer[18]+256*buffer[17];
	dprintf("Image Size in BMP file: %dx%d\n",w,h);

	GSM_GetMaxBitmapWidthHeight(bitmap->Bitmap[0].Type, &bitmap->Bitmap[0].Width, &bitmap->Bitmap[0].Height);
	if (h<bitmap->Bitmap[0].Height)	bitmap->Bitmap[0].Height=h;
	if (w<bitmap->Bitmap[0].Width)	bitmap->Bitmap[0].Width=w;
	dprintf("Height %i %i, width %i %i\n",h,bitmap->Bitmap[0].Height,w,bitmap->Bitmap[0].Width);

	GSM_ClearBitmap(&bitmap->Bitmap[0]);  

#ifdef DEBUG
	dprintf("Number of colors in BMP file: ");
	switch (buffer[28]) {
		case 1	: dprintf("2 (supported)\n");		   break;
		case 4	: dprintf("16 (NOT SUPPORTED)\n");	   break;
		case 8	: dprintf("256 (NOT SUPPORTED)\n");	   break;
		case 24	: dprintf("True Color (NOT SUPPORTED)\n"); break;
		default	: dprintf("unknown\n");			   break;
	}
#endif
	if (buffer[28]!=1) {
		dprintf("Wrong number of colors\n");
		return GE_FILENOTSUPPORTED;
	}

#ifdef DEBUG
	dprintf("Compression in BMP file: ");
	switch (buffer[30]) {
		case 0	:dprintf("no compression (supported)\n"); break;
		case 1	:dprintf("RLE8 (NOT SUPPORTED)\n");	  break;
		case 2	:dprintf("RLE4 (NOT SUPPORTED)\n");	  break;
		default	:dprintf("unknown\n");			  break;
	}
#endif  
	if (buffer[30]!=0) {
		dprintf("Compression type not supported\n");
		return GE_FILENOTSUPPORTED;
	}

	/* read rest of header (if exists) and color palette */
	pos=buffer[10]-34;
	fread(buffer, 1, pos, file);

#ifdef DEBUG  
	dprintf("First color in BMP file: %i %i %i ",buffer[pos-8], buffer[pos-7], buffer[pos-6]);
	if (buffer[pos-8]==0    && buffer[pos-7]==0    && buffer[pos-6]==0)    dprintf("(white)");
	if (buffer[pos-8]==0xFF && buffer[pos-7]==0xFF && buffer[pos-6]==0xFF) dprintf("(black)");
	if (buffer[pos-8]==102  && buffer[pos-7]==204  && buffer[pos-6]==102)  dprintf("(green)");
	dprintf("\n");
	dprintf("Second color in BMP file: %i %i %i ",buffer[pos-4], buffer[pos-3], buffer[pos-2]);
	if (buffer[pos-4]==0    && buffer[pos-3]==0    && buffer[pos-2]==0)    dprintf("(white)");
	if (buffer[pos-4]==0xFF && buffer[pos-3]==0xFF && buffer[pos-2]==0xFF) dprintf("(black)");
	dprintf("\n");  
#endif
	first_white=true;
	if (buffer[pos-8]!=0 || buffer[pos-7]!=0 || buffer[pos-6]!=0) first_white=false;
 
	pos=7;
	/* lines are written from the last to the first */
	for (y=h-1;y>=0;y--)
	{
		i=1;
		for (x=0;x<w;x++)
		{
			/* new byte ! */
			if (pos==7)
			{
				fread(buffer, 1, 1, file);
#ifdef DEBUG
				sizeimage++;
#endif
				i++;
				/* each line is written in multiply of 4 bytes */
				if(i==5) i=1;
			}
			/* we have top left corner ! */
			if (x<=bitmap->Bitmap[0].Width && y<=bitmap->Bitmap[0].Height)
			{
				if (first_white) {
					if ((buffer[0]&(1<<pos))<=0) GSM_SetPointBitmap(&bitmap->Bitmap[0],x,y);
				} else {
					if ((buffer[0]&(1<<pos))>0) GSM_SetPointBitmap(&bitmap->Bitmap[0],x,y);
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
			while (i!=5)
			{
				fread(buffer, 1, 1, file);
#ifdef DEBUG
				sizeimage++;
#endif
				i++;
			}
		}
	}
#ifdef DEBUG
	dprintf("Data size in BMP file: %i\n",sizeimage);
#endif
	bitmap->Number = 1;
	return(GE_NONE);
}

static GSM_Error loadnlm (FILE *file, GSM_MultiBitmap *bitmap)
{
	unsigned char 	buffer[1000];
	int 		pos,pos2,x,y,h,w,i,number;
	div_t		division;

	fread(buffer,1,5,file);

	fread(buffer,1,1,file);
	switch (buffer[0]) {
		case 0x00:
			dprintf("Operator logo\n");
			if (bitmap->Bitmap[0].Type == GSM_None) bitmap->Bitmap[0].Type = GSM_OperatorLogo;
			break;
		case 0x01:
			dprintf("Caller logo\n");
			if (bitmap->Bitmap[0].Type == GSM_None) bitmap->Bitmap[0].Type = GSM_CallerLogo;
			break;
		case 0x02:
			dprintf("Startup logo\n");
			if (bitmap->Bitmap[0].Type == GSM_None) bitmap->Bitmap[0].Type = GSM_StartupLogo;
			break;
		case 0x03:
			dprintf("Picture Image logo\n");
			if (bitmap->Bitmap[0].Type == GSM_None) bitmap->Bitmap[0].Type = GSM_PictureImage;
			break;
	}

	bitmap->Number = 0;
	fread(buffer,1,4,file);
	number 	= buffer[0] + 1;
	w 	= buffer[1];
	h 	= buffer[2];
	for (i=0;i<number;i++) {
		bitmap->Bitmap[i].Type = bitmap->Bitmap[0].Type;
		GSM_GetMaxBitmapWidthHeight(bitmap->Bitmap[i].Type, &bitmap->Bitmap[i].Width, &bitmap->Bitmap[i].Height);
		if (h < bitmap->Bitmap[i].Height)	bitmap->Bitmap[i].Height	= h;
		if (w < bitmap->Bitmap[i].Width)	bitmap->Bitmap[i].Width		= w;

		division=div(w,8);
		/* For startup logos */
		if (division.rem!=0) division.quot++;
	  	if (fread(buffer,1,(division.quot*h),file)!=(unsigned int)(division.quot*h)) return GE_UNKNOWN;
    
		GSM_ClearBitmap(&bitmap->Bitmap[i]);
  
		pos=0;pos2=7;
		for (y=0;y<h;y++) {
			for (x=0;x<w;x++) {
				if ((buffer[pos]&(1<<pos2))>0) {
					if (y<bitmap->Bitmap[i].Height && x<bitmap->Bitmap[i].Width) GSM_SetPointBitmap(&bitmap->Bitmap[i],x,y);
				}
				pos2--;
				/* going to new byte */
				if (pos2<0) {pos2=7;pos++;}
			}
			/* for startup logos-new line means new byte */
			if (pos2!=7) {pos2=7;pos++;}
		}
		bitmap->Number++;
		if (bitmap->Number == MAX_MULTI_BITMAP) break;
	}
	return (GE_NONE);
}

static GSM_Error loadnolngg(FILE *file, GSM_MultiBitmap *bitmap, bool nolformat)
{
	unsigned char 	buffer[2000];
	int		i,h,w,x,y;
  
	fread(buffer, 1, 6, file);

	if (bitmap->Bitmap[0].Type == GSM_None) bitmap->Bitmap[0].Type = GSM_CallerLogo;
	if (nolformat) {
		fread(buffer, 1, 4, file);
		sprintf(bitmap->Bitmap[0].NetworkCode, "%d %02d", buffer[0]+256*buffer[1], buffer[2]);
		if (bitmap->Bitmap[0].Type == GSM_None) bitmap->Bitmap[0].Type = GSM_OperatorLogo;
	}

	fread(buffer, 1, 4, file);
	w = buffer[0];
	h = buffer[2];
	GSM_GetMaxBitmapWidthHeight(bitmap->Bitmap[0].Type, &bitmap->Bitmap[0].Width, &bitmap->Bitmap[0].Height);
	if (h < bitmap->Bitmap[0].Height)	bitmap->Bitmap[0].Height	= h;
	if (w < bitmap->Bitmap[0].Width)	bitmap->Bitmap[0].Width	= w;

	/* Unknown bytes. */
	fread(buffer, 1, 6, file);

	GSM_ClearBitmap(&bitmap->Bitmap[0]);

	x=0; y=0;
	for (i=0; i<w*h; i++) {
		if (fread(buffer, 1, 1, file)!=1) return GE_UNKNOWN;
		if (buffer[0]=='1') GSM_SetPointBitmap(&bitmap->Bitmap[0],x,y);
		x++;
		if (x==w) {x=0; y++;}
	}

#ifdef DEBUG
	/* Some programs writes here fileinfo */
	if (fread(buffer, 1, 1, file)==1) {
		dprintf("Fileinfo: %c",buffer[0]);
		while (fread(buffer, 1, 1, file)==1) {
			if (buffer[0]!=0x0A) dprintf("%c",buffer[0]);
		} 
		dprintf("\n");
	}
#endif
	bitmap->Number = 1;
	return(GE_NONE);
}

static GSM_Error loadnsl(FILE *file, GSM_MultiBitmap *bitmap)
{
	unsigned char 		block[6],buffer[505];
	int 			block_size;
	GSM_Bitmap_Types	OldType;

	while (fread(block,1,6,file)==6) {
		block_size = block[4]*256 + block[5];
		dprintf("Block %c%c%c%c, size %i\n",block[0],block[1],block[2],block[3],block_size);
		if (!strncmp(block, "FORM", 4)) {
			dprintf("File ID\n");
		} else {
			if (block_size>504) return(GE_UNKNOWN);
			if (block_size!=0) {
				fread(buffer,1,block_size,file);
				/* if it's string, we end it with 0 */
				buffer[block_size]=0;
#ifdef DEBUG
				if (!strncmp(block, "VERS", 4)) dprintf("File saved by: %s\n",buffer);
				if (!strncmp(block, "MODL", 4)) dprintf("Logo saved from: %s\n",buffer);
				if (!strncmp(block, "COMM", 4)) dprintf("Phone was connected to COM port: %s\n",buffer);
#endif
				if (!strncmp(block, "NSLD", 4)) {          
					bitmap->Bitmap[0].Height = 48;
					bitmap->Bitmap[0].Width	 = 84;
					OldType = bitmap->Bitmap[0].Type;
					PHONE_DecodeBitmap(GSM_NokiaStartupLogo, buffer, &bitmap->Bitmap[0]);
					if (OldType != GSM_None) bitmap->Bitmap[0].Type = OldType;
					dprintf("Startup logo (size %i)\n",block_size);
				}
			}
		}
	}  
	bitmap->Number = 1;
	return(GE_NONE);
}

GSM_Error GSM_ReadBitmapFile(char *FileName, GSM_MultiBitmap *bitmap)
{
	FILE		*file;
	unsigned char	buffer[300];

	file = fopen(FileName, "rb");
	if (!file) return(GE_CANTOPENFILE);

	fread(buffer, 1, 9, file); /* Read the header of the file. */
	rewind(file);

	/* Attempt to identify filetype */
	if (memcmp(buffer, "BM",2)==0) { 
		return loadbmp(file,bitmap);
	} else if (memcmp(buffer, "NLM",3)==0) {
		return loadnlm(file,bitmap);
	} else if (memcmp(buffer, "NOL",3)==0) {
		return loadnolngg(file,bitmap,true);
	} else if (memcmp(buffer, "NGG",3)==0) {
		return loadnolngg(file,bitmap,false);
	} else if (memcmp(buffer, "FORM",4)==0) {
		return loadnsl(file,bitmap);
	}
	return GE_UNKNOWN;
}

void NOKIA_CopyBitmap(GSM_Phone_Bitmap_Types Type, GSM_Bitmap *Bitmap, char *Buffer, int *Length)
{
	int Width, Height;

	Buffer[(*Length)++] = 0x00;
	PHONE_GetBitmapWidthHeight(Type, &Width, &Height);
	Buffer[(*Length)++] = Width;
	Buffer[(*Length)++] = Height;
	Buffer[(*Length)++] = 0x01;
	PHONE_EncodeBitmap(Type, Buffer + (*Length), Bitmap);
	(*Length) = (*Length) + PHONE_GetBitmapSize(Type);
}
