/*
This source code was copied from a microsoft example program 
and modified to work in the context of this printing function
library. I DO NOT claim authorship of this code only the minor
modifications made to the parameters and elimination of some
of the code that was superflious fo our implementation. We did
modify the code slightly to work in a 32 bit enviornment
RWS
*/




#ifndef	__DIB__
#define __DIB__
#include "cprinter.h"


/* flags for _lseek */
#define  SEEK_CUR 1
#define  SEEK_END 2
#define  SEEK_SET 0


#define MAXREAD  32768             /* Number of bytes to be read during */

/* Header signatutes for various resources */
#define BFT_ICON   0x4349   /* 'IC' */
#define BFT_BITMAP 0x4d42   /* 'BM' */
#define BFT_CURSOR 0x5450   /* 'PT' */

/* macro to determine if resource is a DIB */
#define ISDIB(bft) ((bft) == BFT_BITMAP)

/* Macro to align given value to the closest DWORD (unsigned long ) */
#define ALIGNULONG(i)   ((i+3)/4*4)

/* Macro to determine to round off the given value to the closest byte */
#define WIDTHBYTES(i)   ((i+31)/32*4)





BOOL 		WINAPI  PrintTheBitMap(PRTTYPE *ps);
int   		WINAPI InitDIB(PRTTYPE *ps);
void   		WINAPI FreeDib(void);
void   		WINAPI PrintDIB (PRTTYPE *ps, int x, int y, int dx, int dy);
HANDLE  	WINAPI OpenDIB (LPCTSTR szFile);
BOOL   		WINAPI DibInfo (HANDLE hbi,LPBITMAPINFOHEADER lpbi);
HPALETTE   	WINAPI CreateBIPalette ( LPBITMAPINFOHEADER lpbi);
HPALETTE   	WINAPI CreateDibPalette (HANDLE hbi);
HANDLE   	WINAPI ReadDibBitmapInfo (int fh); 
WORD   		WINAPI PaletteSize (VOID FAR *pv);
WORD   		WINAPI DibNumColors (VOID FAR *pv);
DWORD   	PASCAL lread (int fh,VOID far * pv,DWORD  ul);
BOOL   		WINAPI StretchDibBlt (HDC hdc, int x, int y, int dx, int dy,HANDLE  hdib,int x0, int y0,int dx0, int dy0, LONG rop);
BOOL   		WINAPI ScreenPrintTheBitMap(PRTTYPE *ps);
void		WINAPI CenterDialog( HWND dlg_hwnd );

#endif
