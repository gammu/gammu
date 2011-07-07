// DIB.CPP handles device independant bitmaps both to the screen and the printer


#include "stdafx.h"
#include "dib.h"


static DWORD          dwOffset;
static RECT           rcClip;
static BOOL    bMemoryDIB    = TRUE; /* Load Entire DIB into memory in CF_DIB format */
static BOOL    bLegitDraw    = FALSE; /* We have a valid bitmap to draw               */
static HANDLE   hdibCurrent   = NULL;         /* Handle to current memory DIB         */
static HANDLE   hbiCurrent    = NULL;         /* Handle to current bitmap info struct */

BOOL WINAPI PrintTheBitMap(PRTTYPE *ps)
{
	BITMAPINFOHEADER bi;
    RECT             Rect;
	if(InitDIB(ps)==FALSE)
		return FALSE;   
    DibInfo(hbiCurrent, &bi);

    if (!IsRectEmpty(&rcClip))
    {
          bi.biWidth  = rcClip.right  - rcClip.left;
          bi.biHeight = rcClip.bottom - rcClip.top;
    }
          /* Fix bounding rectangle for the picture .. */

    Rect.top    = ps->rc.top;
    Rect.left   = ps->rc.left;
    Rect.bottom = ps->rc.bottom-ps->rc.top;
    Rect.right  = ps->rc.right-ps->rc.left;
	ps->rc.right=Rect.right;
	ps->rc.bottom=Rect.bottom;
	/* ... and inform the driver */
    ps->pDC->Escape(SET_BOUNDS, sizeof(RECT), (LPSTR)&Rect, NULL);
    PrintDIB(ps, ps->rc.left, ps->rc.top, ps->rc.right, ps->rc.bottom);
    FreeDib();
 	return TRUE;
}

BOOL WINAPI ScreenPrintTheBitMap(PRTTYPE *ps)
{
	BITMAPINFOHEADER bi;
    RECT             Rect;
    if(hbiCurrent==NULL)
    {
		if(InitDIB(ps)==FALSE)
			return FALSE;   
	}			
    DibInfo(hbiCurrent, &bi);

    if (!IsRectEmpty(&rcClip))
    {
          bi.biWidth  = rcClip.right  - rcClip.left;
          bi.biHeight = rcClip.bottom - rcClip.top;
    }
          /* Fix bounding rectangle for the picture .. */
    Rect.top    = ps->rc.top;
    Rect.left   = ps->rc.left;
    Rect.bottom = ps->rc.bottom-ps->rc.top;
    Rect.right  = ps->rc.right-ps->rc.left;
                /* ... and inform the driver */
    ps->pDC->Escape(SET_BOUNDS, sizeof(RECT), (LPSTR)&Rect, NULL);
//    PrintDIB(ps, ps->rc.left, ps->rc.top, ps->rc.right, ps->rc.bottom);
    PrintDIB(ps, ps->rc.left, ps->rc.top, (int)bi.biWidth, (int)bi.biHeight);    
    //FreeDib();
 	return TRUE;
}

int WINAPI InitDIB(PRTTYPE *ps)
{
    unsigned           fh;
    BITMAPINFOHEADER   bi;
    OFSTRUCT           of;

    FreeDib();

    /* Open the file and get a handle to it's BITMAPINFO */
#ifdef _UNICODE
	USES_CONVERSION;
    fh = OpenFile (W2A(ps->Text), (LPOFSTRUCT)&of, OF_READ);
#else
    fh = OpenFile ((LPCTSTR)ps->Text, (LPOFSTRUCT)&of, OF_READ);
#endif
    if (fh == -1) {
        return FALSE;
    }
    hbiCurrent = ReadDibBitmapInfo(fh);

    dwOffset = _llseek(fh, 0L, SEEK_CUR);
    _lclose (fh);

    if (hbiCurrent == NULL) {
        return FALSE;
    }
    DibInfo(hbiCurrent,&bi);

    bLegitDraw = TRUE;

    /*  If the input bitmap is not in RGB FORMAT the banding code will
     *  not work!  we need to load the DIB bits into memory.
     *  if memory DIB, load it all NOW!  This will avoid calling the
     *  banding code.
     */
       if (bMemoryDIB || bi.biCompression != BI_RGB)
        hdibCurrent = OpenDIB(ps->Text);

    /*  If the RLE could not be loaded all at once, exit gracefully NOW,
     *  to avoid calling the banding code
     */
    if ((bi.biCompression != BI_RGB) && !hdibCurrent){
        FreeDib();
        return FALSE;
    }

    return TRUE;
}

void WINAPI FreeDib(void)
{
    if (hdibCurrent)
        GlobalFree(hdibCurrent);

    if (hbiCurrent && hbiCurrent != hdibCurrent)
        GlobalFree(hbiCurrent);

    bLegitDraw  = FALSE;
    hdibCurrent = NULL;
    hbiCurrent  = NULL;
    SetRectEmpty (&rcClip);
}

void WINAPI PrintDIB (PRTTYPE *ps, int x, int y, int dx, int dy)
{
    BITMAPINFOHEADER bi;
    int dibX,  dibY;
    int dibDX, dibDY;

    if (!bLegitDraw)
        return;

    DibInfo (hbiCurrent, &bi);

    if (IsRectEmpty (&rcClip)){
        dibX  = 0;
        dibY  = 0;
        dibDX = (int)bi.biWidth;
        dibDY = (int)bi.biHeight;
    }
    else{
        dibX  = rcClip.left;
        dibY  = (int)bi.biHeight - 1 - rcClip.bottom;
        dibDX = rcClip.right  - rcClip.left;
        dibDY = rcClip.bottom - rcClip.top;
    }

    if (hdibCurrent){
        /* Stretch the DIB to printer DC */
        StretchDibBlt ( ps->pDC->GetSafeHdc(),
                        x,
                        y,
                        dx,
                        dy,
                        hdibCurrent,
                        dibX,
                        dibY,
                        dibDX,
                        dibDY,
                        SRCCOPY);
    }
 
}

HANDLE WINAPI OpenDIB (LPCTSTR szFile)
{
    unsigned            fh;
    BITMAPINFOHEADER    bi;
    LPBITMAPINFOHEADER  lpbi;
    DWORD               dwLen = 0;
    DWORD               dwBits;
    HANDLE              hdib;
    HANDLE              h;
    OFSTRUCT            of;

    /* Open the file and read the DIB information */
 #ifdef _UNICODE
	USES_CONVERSION;
   fh = OpenFile(W2A(szFile), &of, OF_READ);
#else
   fh = OpenFile(szFile, &of, OF_READ);
#endif
    if (fh == -1)
        return NULL;

    hdib = ReadDibBitmapInfo(fh);
    if (!hdib)
        return NULL;
    DibInfo(hdib,&bi);

    /* Calculate the memory needed to hold the DIB */
    dwBits = bi.biSizeImage;
    dwLen  = bi.biSize + (DWORD)PaletteSize (&bi) + dwBits;

    /* Try to increase the size of the bitmap info. buffer to hold the DIB */
    h = GlobalReAlloc(hdib, dwLen, GHND);
    if (!h){
        GlobalFree(hdib);
        hdib = NULL;
    }
    else
        hdib = h;

    /* Read in the bits */
    if (hdib){

        lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib);
        lread(fh, (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize(lpbi), dwBits);
        GlobalUnlock(hdib);
    }
    _lclose(fh);

    return hdib;
}

BOOL WINAPI DibInfo (HANDLE hbi,LPBITMAPINFOHEADER lpbi)
{
    if (hbi){
        *lpbi = *(LPBITMAPINFOHEADER)GlobalLock (hbi);

        /* fill in the default fields */
        if (lpbi->biSize != sizeof (BITMAPCOREHEADER)){
            if (lpbi->biSizeImage == 0L)
                lpbi->biSizeImage =
                    WIDTHBYTES(lpbi->biWidth*lpbi->biBitCount) * lpbi->biHeight;

            if (lpbi->biClrUsed == 0L)
                lpbi->biClrUsed = DibNumColors (lpbi);
        }
        GlobalUnlock (hbi);
        return TRUE;
    }
    return FALSE;
}

HANDLE WINAPI ReadDibBitmapInfo (int fh)
{
    DWORD     off;
    HANDLE    hbi = NULL;
    int       size;
    int       i;
    WORD      nNumColors;

    RGBQUAD FAR       *pRgb;
    BITMAPINFOHEADER   bi;
    BITMAPCOREHEADER   bc;
    LPBITMAPINFOHEADER lpbi;
    BITMAPFILEHEADER   bf;
    DWORD              dwWidth = 0;
    DWORD              dwHeight = 0;
    WORD               wPlanes, wBitCount;

    if (fh == -1)
        return NULL;

    /* Reset file pointer and read file header */
    off = _llseek(fh, 0L, SEEK_CUR);
    if (sizeof (bf) != _lread (fh, (LPSTR)&bf, sizeof (bf)))
        return FALSE;

    /* Do we have a RC HEADER? */
    if (!ISDIB (bf.bfType)) {
        bf.bfOffBits = 0L;
        _llseek (fh, off, SEEK_SET);
    }
    if (sizeof (bi) != _lread (fh, (LPSTR)&bi, sizeof(bi)))
        return FALSE;

    nNumColors = DibNumColors (&bi);

    /* Check the nature (BITMAPINFO or BITMAPCORE) of the info. block
     * and extract the field information accordingly. If a BITMAPCOREHEADER,
     * transfer it's field information to a BITMAPINFOHEADER-style block
     */
    switch (size = (int)bi.biSize){
        case sizeof (BITMAPINFOHEADER):
            break;

        case sizeof (BITMAPCOREHEADER):

            bc = *(BITMAPCOREHEADER*)&bi;

            dwWidth   = (DWORD)bc.bcWidth;
            dwHeight  = (DWORD)bc.bcHeight;
            wPlanes   = bc.bcPlanes;
            wBitCount = bc.bcBitCount;

            bi.biSize               = sizeof(BITMAPINFOHEADER);
            bi.biWidth              = dwWidth;
            bi.biHeight             = dwHeight;
            bi.biPlanes             = wPlanes;
            bi.biBitCount           = wBitCount;

            bi.biCompression        = BI_RGB;
            bi.biSizeImage          = 0;
            bi.biXPelsPerMeter      = 0;
            bi.biYPelsPerMeter      = 0;
            bi.biClrUsed            = nNumColors;
            bi.biClrImportant       = nNumColors;

            _llseek (fh, (LONG)sizeof (BITMAPCOREHEADER) - sizeof (BITMAPINFOHEADER), SEEK_CUR);
            break;

        default:
            /* Not a DIB! */
            return NULL;
    }

    /*  Fill in some default values if they are zero */
    if (bi.biSizeImage == 0){
        bi.biSizeImage = WIDTHBYTES ((DWORD)bi.biWidth * bi.biBitCount)
                         * bi.biHeight;
    }
    if (bi.biClrUsed == 0)
        bi.biClrUsed = DibNumColors(&bi);

    /* Allocate for the BITMAPINFO structure and the color table. */
    hbi = GlobalAlloc (GHND, (LONG)bi.biSize + nNumColors * sizeof(RGBQUAD));
    if (!hbi)
        return NULL;
    lpbi = (LPBITMAPINFOHEADER)GlobalLock (hbi);
    *lpbi = bi;

    /* Get a pointer to the color table */
    pRgb = (RGBQUAD FAR *)((LPSTR)lpbi + bi.biSize);
    if (nNumColors){
        if (size == sizeof(BITMAPCOREHEADER)){
            /* Convert a old color table (3 byte RGBTRIPLEs) to a new
             * color table (4 byte RGBQUADs)
             */
            _lread (fh, (LPSTR)pRgb, nNumColors * sizeof(RGBTRIPLE));

            for (i = nNumColors - 1; i >= 0; i--){
                RGBQUAD rgb;

                rgb.rgbRed      = ((RGBTRIPLE FAR *)pRgb)[i].rgbtRed;
                rgb.rgbBlue     = ((RGBTRIPLE FAR *)pRgb)[i].rgbtBlue;
                rgb.rgbGreen    = ((RGBTRIPLE FAR *)pRgb)[i].rgbtGreen;
                rgb.rgbReserved = (BYTE)0;

                pRgb[i] = rgb;
            }
        }
        else
            _lread(fh,(LPSTR)pRgb,nNumColors * sizeof(RGBQUAD));
    }

    if (bf.bfOffBits != 0L)
        _llseek(fh,off + bf.bfOffBits,SEEK_SET);

    GlobalUnlock(hbi);
    return hbi;
}

WORD WINAPI PaletteSize (VOID FAR *pv)
{
    LPBITMAPINFOHEADER lpbi;
    WORD               NumColors;

    lpbi      = (LPBITMAPINFOHEADER)pv;
    NumColors = DibNumColors(lpbi);

    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
        return NumColors * sizeof(RGBTRIPLE);
    else
        return NumColors * sizeof(RGBQUAD);
}

WORD WINAPI DibNumColors (VOID FAR *pv)
{
    int                 bits;
    LPBITMAPINFOHEADER  lpbi;
    LPBITMAPCOREHEADER  lpbc;

    lpbi = ((LPBITMAPINFOHEADER)pv);
    lpbc = ((LPBITMAPCOREHEADER)pv);

    /*  With the BITMAPINFO format headers, the size of the palette
     *  is in biClrUsed, whereas in the BITMAPCORE - style headers, it
     *  is dependent on the bits per pixel ( = 2 raised to the power of
     *  bits/pixel).
     */
    if (lpbi->biSize != sizeof(BITMAPCOREHEADER)){
        if (lpbi->biClrUsed != 0)
            return (WORD)lpbi->biClrUsed;
        bits = lpbi->biBitCount;
    }
    else
        bits = lpbc->bcBitCount;

    switch (bits){
        case 1:
                return 2;
        case 4:
                return 16;
        case 8:
                return 256;
        default:
                /* A 24 bitcount DIB has no color table */
                return 0;
    }
}

BOOL WINAPI StretchDibBlt (HDC hdc, int x, int y, int dx, int dy,HANDLE  hdib,int x0, int y0,int dx0, int dy0, LONG rop)

{
    LPBITMAPINFOHEADER lpbi;
    LPSTR        pBuf;
    BOOL         f;

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib);

    if (!lpbi)
        return FALSE;

    pBuf = (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize(lpbi);

    f = StretchDIBits ( hdc,
                        x, y,
                        dx, dy,
                        x0, y0,
                        dx0, dy0,
                        pBuf, (LPBITMAPINFO)lpbi,
                        DIB_RGB_COLORS,
                        rop);

    GlobalUnlock(hdib);
    return f;
}

DWORD PASCAL lread (int fh,VOID far * pv,DWORD  ul)
{
    DWORD     ulT = ul;
    BYTE  *hp = (unsigned char  *)pv;

    while (ul > (DWORD)MAXREAD) {
        if (_lread(fh, (LPSTR)hp, (WORD)MAXREAD) != MAXREAD)
                return 0;
        ul -= MAXREAD;
        hp += MAXREAD;
    }
    if (_lread(fh, (LPSTR)hp, (WORD)ul) != (WORD)ul)
        return 0;
    return ulT;
}

