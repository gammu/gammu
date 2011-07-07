//
// XImage.h
//

#ifndef __ANWIMAGE_XIMAGE_H_
#define __ANWIMAGE_XIMAGE_H_

//#define UNICODE
#include <gdiplus.h>
//#include "GdiplusH.h"
#include <comdef.h>		// for _bstr_t
#include "_defanwimageext.h"

using namespace Gdiplus;

class GdiPlusSys
{
public:
	GdiPlusSys()
	{
		GdiplusStartupInput gdiplusStartupInput;

		// Initialize GDI+.
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	};

	~GdiPlusSys()
	{
		GdiplusShutdown(gdiplusToken);
	};

private:
	ULONG_PTR gdiplusToken;
};

typedef enum 
{ 
	DRAW_NORMAL, 
	DRAW_CENTER, 
	DRAW_TILE, 
	DRAW_STRETCH, 
	DRAW_STRETCHPROP,
	DRAW_TILE9,
	DRAW_STRETCH9
};

#define PT_2INDEX		PixelFormat1bppIndexed		// black & white
#define PT_16INDEX		PixelFormat4bppIndexed		// index 16 color
#define PT_256INDEX		PixelFormat8bppIndexed		// index 256 color
#define PT_16BITSARGB	PixelFormat16bppARGB1555	// ARGB 1-5-5-5 high color
#define PT_16BITSGRAY	PixelFormat16bppGrayScale   // 16bit=65536 gray levels
#define PT_16BITS555	PixelFormat16bppRGB555 		// RGB 5-5-5 high color
#define PT_16BITS565	PixelFormat16bppRGB565      // RGB 5-6-5 high color
#define PT_24BITSRGB	PixelFormat24bppRGB			// RGB 8-8-8 true color
#define PT_32BITSARGB	PixelFormat32bppARGB		// aRGB 8-8-8-8 true color
#define PT_48BITSRGB	PixelFormat48bppRGB			// RGB 16-16-16 true color
#define PT_DONTCARE		PixelFormatDontCare			// don't care

#define ARGB_A(argb)	((BYTE)(argb >> 24))
#define ARGB_R(argb)	((BYTE)(argb >> 16) & 0x000000ff)
#define ARGB_G(argb)	((BYTE)(argb >> 8) & 0x000000ff)
#define ARGB_B(argb)	((BYTE)(argb) & 0x000000ff)

#define BPP(a)			((a)->biBitCount >> 3)


typedef HANDLE (* FN_OPENPNG)(const char *, int *, BOOL);
PixelFormat BitCountToPixelFormat( int nBitCount );

class ANWIMAGE_EXT CImage : public Bitmap
{
public:
	// constructor
	CImage(CImage *pImg = NULL);
	CImage(LPCTSTR filename);
	CImage(INT width, INT height, PixelFormat format = PT_24BITSRGB, ARGB color = 0);
	CImage(INT width, INT height, INT stride, PixelFormat format, BYTE* scan0 );
	CImage(const UINT nIDResource, HINSTANCE hInst = NULL);
	CImage(HBITMAP hBmp, HPALETTE hPal = NULL);
    CImage(const BITMAPINFO *pBitmapInfo, void *pBits);
	CImage(HICON hIcon);
	CImage(HANDLE hMem);
	CImage(IStream* stream);

	virtual ~CImage();

	static char m_szModuleName[_MAX_PATH];
	BOOL LogtoFile(TCHAR *string,long data,bool	havedata);

private:
	void Init();

	HINSTANCE	m_hLibPng;
	FN_OPENPNG	m_fnOpenPng;
	HANDLE		m_hDib;

protected:
	Status Stream(LPCTSTR filename);
	void Free(void);
	void SetImage(GpBitmap *bitmap);
	int  GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

	// for reduce color
	BOOL CreateRGBLut();
	BYTE FindNearestMatch0(int r, int g, int b);
	BYTE FindNearestMatch(int r, int g, int b);

public:
	// load file
	inline BOOL Load(LPCTSTR filename);
	inline BOOL Open(LPCTSTR filename);
	Status LoadPng(LPCTSTR filename);
	Status LoadFile(LPCTSTR filename );

	// save file
	BOOL Save(LPCTSTR filename, EncoderParameters *encoderParams = NULL);
	BOOL SaveJpg(LPCTSTR filename, int nQuality = -1);
	BOOL SaveFile(LPCTSTR filename, int nQuality = -1);
	BOOL SaveAdd(LPCTSTR filename);

	// resolution
	inline void GetResolution(float& xdpi, float& ydpi);
	inline void GetDPI(float& xdpi, float& ydpi);	// dot/inch
	inline void GetDPM(float& xdpm, float& ydpm );	// dot/meter

	inline void SetResolution(CImage* pImage);
	inline void SetResolution(float xdpi, float ydpi);
	inline void SetDPI(float xdpi, float ydpi);
	inline void SetDPM(float xdpm, float ydpm);

	// dump
	//void DumpProperty();

// Attributes
public:
	ColorPalette*	m_pPalette;
	BYTE*			m_pRGBTable;
	HRGN			m_pMaskRegion;
	inline UINT Width();
	inline UINT Height();
//	inline PixelFormat Type();
	inline PixelFormat Format();
	int	BitCount();
	UINT BytesPerLine();

// functions
public:
	BYTE ColorTable2Palette(RGBQUAD* pClrTable );
	inline Status SetPalette(const ColorPalette* palette);
	BOOL SetPalette(RGBQUAD *pPal);
	CImage *ReduceColor(PixelFormat format);

	CImage *Copy();
	CImage *Copy(INT x, INT y, INT width, INT height);
	inline CImage *Copy(Rect& rect);

	BOOL CloneRect(INT x, INT y, INT width, INT height);
	BOOL CloneRect(Rect& rect);

	CImage *GetThumbnail(int cx, int cy, InterpolationMode nMode = InterpolationModeHighQualityBicubic, PixelFormat format = PT_24BITSRGB );
	CImage *GetPropThumbnail(int cx, int cy, BOOL bFit=TRUE, InterpolationMode nMode = InterpolationModeHighQualityBicubic );

	// attach bitmap
	BOOL SetBitmap(LPCTSTR filename);
    BOOL SetBitmap(UINT nIDResource, HINSTANCE hInst = NULL);
    BOOL SetBitmap(HBITMAP hBitmap, HPALETTE hPal = NULL);
    BOOL SetBitmap(BITMAPINFO *pBitmapInfo, void *pBits);
    BOOL SetBitmap(HICON hIcon);

	// property rect

	void ShrinkPropRect( CRect rcSrc, CRect& rcDst, BOOL bAdjust=FALSE, BOOL bFit=TRUE );
	void CropPropRect( CRect& rcSrc, CRect rcDst, BOOL bAdjust=FALSE, BOOL bFit=TRUE );

	inline void GetPropRect( CRect& rcSrc, CRect& rcDst, BOOL bCrop=FALSE, BOOL bAdjust=FALSE, BOOL bFit=TRUE );
	inline void GetPropRect( CRect& rect, BOOL bCrop=FALSE );
	inline void GetPropRect( Rect& rect, BOOL bCrop=FALSE );
	//Add new Mask feature
	Status SetMaskregion( HRGN pMaskRgn, BYTE iTransparency=0 );
	HRGN GetMaskregion( void )  {  return  m_pMaskRegion; };
	//2004/01/15
	inline void DrawString(HDC hDC,const WCHAR* string,INT length,const Font* font,
    const PointF &origin, const StringFormat* stringFormat,const Brush* brush);
	inline void DrawString( Graphics& graphics, const WCHAR* string,INT length,const Font* font,
    const PointF &origin, const StringFormat* stringFormat,const Brush* brush);
	inline void DrawString(CImage& Img, const WCHAR* string,INT length,const Font* font,
    const PointF &origin, const StringFormat* stringFormat,const Brush* brush);

	//2003/12/08
	// display & draw
	inline void DrawImage(HDC hDC, INT xDst, INT yDst, INT wDst, INT hDst, 
			  INT xSrc, INT ySrc, INT wSrc, INT hSrc);
	inline void DrawImage(Graphics& graphics, INT xDst, INT yDst, INT wDst, INT hDst, 
			  INT xSrc, INT ySrc, INT wSrc, INT hSrc);
	inline void DrawImage(CImage& Img, INT xDst, INT yDst, INT wDst, INT hDst, 
			  INT xSrc, INT ySrc, INT wSrc, INT hSrc);
	inline void Draw( CImage& Img, INT xDst, INT yDst, INT wDst, INT hDst );
	inline void Draw( CImage& Img, RECT rect );

	void Display(Graphics& graphics, INT x, INT y, INT cx, INT cy, int nType = DRAW_NORMAL);
	inline void Display(HDC hDC, INT x, INT y, INT cx, INT cy, int nType = DRAW_NORMAL);
	inline void Display(Graphics& graphics, RECT *lpDCRect, int nType = DRAW_NORMAL);
	inline void Display(HDC hDC, RECT *lpDCRect, int nType = DRAW_NORMAL);

	inline void TileImage(HDC hDC, INT xDst, INT yDst, INT wDst, INT hDst, 
			  INT xSrc, INT ySrc, INT wSrc, INT hSrc);
	inline void TileImage(Graphics& graphics, INT xDst, INT yDst, INT wDst, INT hDst, 
			  INT xSrc, INT ySrc, INT wSrc, INT hSrc);

public:

	void FillColor(Color& color);

	BOOL GetBlock(UINT x1, UINT y1, UINT x2, UINT y2, LPBYTE lpData, 
				  PixelFormat format = PT_24BITSRGB);
	BOOL PutBlock(UINT x1, UINT y1, UINT x2, UINT y2, LPBYTE lpData, 
				  PixelFormat format = PT_24BITSRGB);

	inline BOOL GetHorLine(UINT y, UINT x1, UINT x2, LPBYTE lpData, PixelFormat format = PT_24BITSRGB);
	inline BOOL PutHorLine(UINT y, UINT x1, UINT x2, LPBYTE lpData, PixelFormat format = PT_24BITSRGB);
	inline BOOL GetVerLine(UINT x, UINT y1, UINT y2, LPBYTE lpData, PixelFormat format = PT_24BITSRGB);
	inline BOOL PutVerLine(UINT x, UINT y1, UINT y2, LPBYTE lpData, PixelFormat format = PT_24BITSRGB);

	CImage *GetResizeImage(int cx, int cy, ARGB color=0x00000000, InterpolationMode nMode = InterpolationModeHighQualityBicubic );
	CRect GetRotateRect( float Angle, float scaleX=1.0, float scaleY=1.0 );
	CImage* GetRotateImage( float Angle, PixelFormat format=PT_32BITSARGB );
	CImage* GetRescaleRotateImage(int cx, int cy, float Angle, PixelFormat format=PT_32BITSARGB );

	void Rotate90xFlip( float Angle, BOOL bFlipX=FALSE, BOOL bFlipY=FALSE, PixelFormat format=PT_32BITSARGB );
	void Flip( BOOL bFlipX, BOOL bFlipY );
/*
	// palette operations
	//bool	IsGrayScale();
	DWORD	GetPaletteSize();
	RGBQUAD* GetPalette() const;
	RGBQUAD GetPaletteColor(BYTE idx);

	bool	GetRGB(int i, BYTE *r, BYTE *g, BYTE *b);
	BYTE	GetNearestIndex(RGBQUAD c);

	//void	BlendPalette(COLORREF cr, long perc);
	void	SetGrayPalette();
	void	SetPalette(RGBQUAD *pPal, DWORD nColors = 256);
	void	SetPaletteIndex(BYTE idx, BYTE r, BYTE g, BYTE b);
	void	SetPaletteIndex(BYTE idx, RGBQUAD c);
	void	SetPaletteIndex(BYTE idx, COLORREF cr);
	void	SwapIndex(BYTE idx1, BYTE idx2);
	void	SetStdPalette();

	//
	void GetCopyRect(RECT *s, uint32 wSrc, uint32 hSrc, RECT *d, RECT *clip);
	void CopyImage(int32 xDst, int32 yDst, CImage *pImgSrc, int32 xSrc, int32 ySrc, 
				   uint32 wSrc, uint32 hSrc);
	void BlendImage(int32 xDst, int32 yDst, CImage *pImgSrc, int32 xSrc, int32 ySrc, 
					uint32 wSrc, uint32 hSrc, CImage *pImgMask = NULL);
	void TransImage(uint32 xDst, uint32 yDst, CImage *pImgSrc, uint32 xSrc, uint32 ySrc, 
					uint32 wSrc, uint32 hSrc, DWORD crTransparent);

protected:
	WORD GetNumColors(int nBitCount);
	LPRGBQUAD CreateStdPalette(int nImageType);
	void FreePalette(void *pPalette);
	BYTE FindNearestMatch0(int r, int g, int b, RGBQUAD *pPal, int nColor);
	BYTE FindNearestMatch(int r, int g, int b);
*/
	//DECLARE_IMAGE_DIB()
};

typedef CImage* LPCIMAGE;

////////////////////////////////////////////////////////////////////////////////
// inline functions

inline BOOL CImage::Load(LPCTSTR filename)
{	return SetBitmap(filename);
}

inline BOOL CImage::Open(LPCTSTR filename)
{	return SetBitmap(filename); 
}

inline void CImage::GetResolution(float& xdpi, float& ydpi)
{	xdpi = GetHorizontalResolution();
	ydpi = GetVerticalResolution();
}

inline void CImage::GetDPI(float& xdpi, float& ydpi)
{	xdpi = GetHorizontalResolution();
	ydpi = GetVerticalResolution();
}

// 1 inch = 2.54 cm
inline void CImage::GetDPM(float& xdpm, float& ydpm)
{	xdpm = (float)(GetHorizontalResolution()*254/10000);
	ydpm = (float)(GetVerticalResolution()*254/10000);
}

inline void CImage::SetResolution(CImage* pImage)
{	float xdpi, ydpi;
	pImage->GetResolution(xdpi, ydpi);
	Bitmap::SetResolution(xdpi, ydpi);
}

inline void CImage::SetResolution(float xdpi, float ydpi)
{	Bitmap::SetResolution(xdpi, ydpi);
}

inline void CImage::SetDPI( float xdpi, float ydpi )
{	Bitmap::SetResolution(xdpi, ydpi);
}

inline void CImage::SetDPM( float xdpm, float ydpm )
{	float xdpi = (xdpm*254)/10000L;
	float ydpi = (ydpm*254)/10000L;
	Bitmap::SetResolution(xdpi, ydpi);
}

inline UINT CImage::Width()
{	return GetWidth(); 
}

inline UINT CImage::Height()
{	return GetHeight(); 
}

inline PixelFormat CImage::Format()
{	return GetPixelFormat(); 
}

inline Status CImage::SetPalette(const ColorPalette* palette)
{	return Bitmap::SetPalette(palette);
}

inline CImage* CImage::Copy(Rect& rect)
{	return Copy( rect.X, rect.Y, rect.Width, rect.Height );	
}

///////////////////////////////////////////////////////////////////////////////
//	get the proportional rect
//
//	rcSrc : Image rect			/	rcDst : output rect
//	bCrop = TRUE : Crop to fit. /	FALSE : Shrink to fit.
//	bAdjust = TRUE: adjust to center.
//	bFit  = TRUE: fit to rcDst. /	FALSE: if small than rcDst, keep original size.
//
inline void CImage::GetPropRect( CRect& rcSrc, CRect& rcDst, BOOL bCrop, BOOL bAdjust, BOOL bFit )
{
	( bCrop ) ? CropPropRect( rcSrc, rcDst, bAdjust, bFit )
			  : ShrinkPropRect( rcSrc, rcDst, bAdjust, bFit );
}

inline void CImage::GetPropRect( CRect& rect, BOOL bCrop )
{
	CRect rcSrc( 0, 0, Width(), Height() );
	GetPropRect( rcSrc, rect, bCrop, TRUE, FALSE);
}

inline void CImage::GetPropRect( Rect& rect, BOOL bCrop )
{
	CRect rc( rect.X, rect.Y, rect.Width, rect.Height );
	CRect rcSrc( 0, 0, Width(), Height() );
	GetPropRect( rcSrc, rc, bCrop, TRUE, FALSE);
	rect = Rect( rc.left, rc.top, rc.Width(), rc.Height() );
}

//DrawString to the HDC
inline void CImage::DrawString(HDC hDC,const WCHAR* string,INT length,const Font* font,
    const PointF &origin, const StringFormat* stringFormat,const Brush* brush)
{
	Graphics graphics(hDC);
	DrawString( graphics, string, length, font, origin, stringFormat, brush);
}

//DrawString to the Graphics
inline void CImage::DrawString( Graphics& graphics, const WCHAR* string,INT length,const Font* font,
    const PointF &origin, const StringFormat* stringFormat,const Brush* brush)
{
	//Rect rc(xDst, yDst, wDst, hDst);
	graphics.DrawString(string, length, font, origin, stringFormat, brush);
}
//DrawStrin to your CImage
inline void CImage::DrawString(CImage& Img, const WCHAR* string,INT length,const Font* font,
    const PointF &origin, const StringFormat* stringFormat,const Brush* brush)
{
	Graphics graphics((Image*)&Img );

	//Rect rc(xDst, yDst, wDst, hDst);
	graphics.DrawString(string, length, font, origin, stringFormat, brush);
}

//DrawImage
inline void CImage::DrawImage( Graphics& graphics, INT xDst, INT yDst, INT wDst, INT hDst, 
					     INT xSrc, INT ySrc, INT wSrc, INT hSrc)
{
	Rect rc(xDst, yDst, wDst, hDst);
	graphics.DrawImage((Image *)this, rc, xSrc, ySrc, wSrc, hSrc, UnitPixel, NULL, NULL, NULL);
}


inline void CImage::DrawImage(HDC hDC, INT xDst, INT yDst, INT wDst, INT hDst, 
					     INT xSrc, INT ySrc, INT wSrc, INT hSrc)
{
	Graphics graphics(hDC);
	DrawImage( graphics, xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc);
}

inline void CImage::DrawImage(CImage& Img, INT xDst, INT yDst, INT wDst, INT hDst, 
			  INT xSrc, INT ySrc, INT wSrc, INT hSrc)
{
	Graphics graphics((Image*)&Img );

	Rect rc(xDst, yDst, wDst, hDst);
	graphics.DrawImage((Image *)this, rc, xSrc, ySrc, wSrc, hSrc, UnitPixel, NULL, NULL, NULL);
}

inline void CImage::Draw( CImage& Img, INT xDst, INT yDst, INT wDst, INT hDst )
{	DrawImage(Img, xDst, yDst, wDst, hDst, 0, 0, GetWidth(), GetHeight() );
}

inline void CImage::Draw( CImage& Img, RECT rect )
{	Draw(Img, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top );
}

inline void CImage::Display(HDC hDC, INT x, INT y, INT cx, INT cy, int nType)
{	Graphics graphics(hDC);
	Display(graphics, x, y, cx, cy, nType);
}

inline void CImage::Display(Graphics& graphics, RECT *lpDCRect, int nType)
{	Display( graphics, lpDCRect->left, lpDCRect->top, 
		    lpDCRect->right - lpDCRect->left, lpDCRect->bottom - lpDCRect->top, nType);
}

inline void CImage::Display(HDC hDC, RECT *lpDCRect, int nType)
{	Display(hDC, lpDCRect->left, lpDCRect->top, 
		    lpDCRect->right - lpDCRect->left, lpDCRect->bottom - lpDCRect->top, nType);
}

inline void CImage::TileImage( Graphics& graphics, INT xDst, INT yDst, INT wDst, INT hDst, 
	INT xSrc, INT ySrc, INT wSrc, INT hSrc)
{
	TextureBrush tBrush( this, Rect( xSrc, ySrc, wSrc, hSrc ) );

	graphics.TranslateTransform( (REAL)xDst, (REAL)yDst, MatrixOrderAppend);
	graphics.FillRectangle( &tBrush, Rect( 0,0, wDst, hDst) );
	graphics.TranslateTransform( (REAL)(-xDst), (REAL)(-yDst), MatrixOrderAppend);
}

inline void CImage::TileImage(HDC hDC, INT xDst, INT yDst, INT wDst, INT hDst, 
			  INT xSrc, INT ySrc, INT wSrc, INT hSrc)
{
	Graphics graphics(hDC);
	TileImage( graphics, xDst, yDst, wDst, hDst, xSrc, ySrc, wSrc, hSrc );
}

inline BOOL CImage::GetHorLine(UINT y, UINT x1, UINT x2, LPBYTE lpData, PixelFormat format)
{	return GetBlock(x1, y, x2, y, lpData, format);
}

inline BOOL CImage::PutHorLine(UINT y, UINT x1, UINT x2, LPBYTE lpData, PixelFormat format)
{	return PutBlock(x1, y, x2, y, lpData, format);
}

inline BOOL CImage::GetVerLine(UINT x, UINT y1, UINT y2, LPBYTE lpData, PixelFormat format)
{	return GetBlock(x, y1, x, y2, lpData, format);
}

inline BOOL CImage::PutVerLine(UINT x, UINT y1, UINT y2, LPBYTE lpData, PixelFormat format)
{	return PutBlock(x, y1, x, y2, lpData, format);
}

////////////////////////////////////////////////////////////////////////////////
// CImageArray
#ifndef __CBORDER_H_
#define __CBORDER_H_

class ANWIMAGE_EXT CBorder
{
public:
	CBorder() { left = top = right = bottom = 0; }
	CBorder( int l, int t, int r, int b ) :left(l),top(t),right(r),bottom(b)
	{}
	CBorder( LPRECT pRect )
	{	CBorder( pRect->left, pRect->top, pRect->right, pRect->bottom );
	}

	const CBorder& operator=(const CBorder& border)
		{
			this->left   = border.left;
			this->top    = border.top;
			this->right  = border.right;
			this->bottom = border.bottom;
			return *this;
		}

	const CBorder& operator=(const CRect& border)
		{	
			this->left   = border.left;
			this->top    = border.top;
			this->right  = border.right;
			this->bottom = border.bottom;
			return *this;
		}
public:
	int	left;
	int	top;
	int	right;
	int	bottom;
};
typedef CBorder* LPCBODER;

#endif __CBORDER_H_

class ANWIMAGE_EXT CImageArray : public CImage
{
public:
	CImageArray(LPCTSTR pFile = NULL, int count = 1, CBorder *pBorder = NULL);
	CImageArray(INT width, INT height, PixelFormat format = PT_24BITSRGB, ARGB color = 0);
	//2003/07/24 add by Andy
	CImageArray(const HBITMAP hBmp) : CImage(hBmp){}	

	inline void SetCount(int count);	// set image array count
	inline int  GetCount();				// get image array count
	inline int  GetItemWidth();			// get item image width
	inline int  GetItemHeight();		// get item image height
	inline CRect  GetItemRect();		// get item rect

	inline void SetBorder(int cx1, int cy1, int cx2, int cy2);
	inline void SetBorder(CBorder& border);
	inline CBorder GetBorder();

	BOOL Draw(Graphics& graphics, int x0, int y0, int w, int h, int index, int opt = DRAW_NORMAL, DWORD rop = SRCCOPY);
	BOOL Draw(Graphics& graphics, RECT rect, int index, int opt = DRAW_NORMAL, DWORD rop = SRCCOPY);
	BOOL Draw(HDC hDC, int x0, int y0, int w, int h, int index, int opt = DRAW_NORMAL, DWORD rop = SRCCOPY);
	BOOL Draw(HDC hDC, RECT rect, int index, int opt = DRAW_NORMAL, DWORD rop = SRCCOPY);
	BOOL xDrawTile9(Graphics& graphics, int x0, int y0, int w, int h, int index, int opt, DWORD rop = SRCCOPY);

private:
	int		m_nCount;
	CBorder	m_Border;

	//DECLARE_IMAGEARRAY_DIB()
};

typedef CImageArray* LPCIMAGEARRAY;

inline void CImageArray::SetCount(int count)
{	m_nCount = count;
}

inline int CImageArray::GetCount()
{	return m_nCount;
}

inline int CImageArray::GetItemWidth()
{	return Width() / m_nCount;
}

inline int CImageArray::GetItemHeight()
{	return Height();
}

inline CRect CImageArray::GetItemRect()
{	return CRect(0,0,GetItemWidth(),GetItemHeight());
}

inline void CImageArray::SetBorder(int cx1, int cy1, int cx2, int cy2)
{	m_Border.left   = cx1;
	m_Border.top    = cy1;
	m_Border.right  = cx2;
	m_Border.bottom = cy2;
}

inline void CImageArray::SetBorder(CBorder& border)
{	m_Border = border;
}

inline CBorder CImageArray::GetBorder()
{	return m_Border;
}

inline BOOL CImageArray::Draw(HDC hDC, int x0, int y0, int w, int h, int index, int opt, DWORD rop)
{
	Graphics graphics(hDC);
	return Draw( graphics, x0, y0, w, h, index, opt, rop);
}

inline BOOL CImageArray::Draw(HDC hDC, RECT rect, int index, int opt, DWORD rop)
{	return Draw(hDC, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, index, opt, rop);
}

inline BOOL CImageArray::Draw(Graphics& graphics, RECT rect, int index, int opt, DWORD rop)
{	return Draw( graphics, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, index, opt, rop);
}

///////////////////////////////////////////////////////////////////////
//
inline Status CreateImage( LPCIMAGE& pImage, LPTSTR filename )
{	
	Status status = OutOfMemory;
	if( pImage == NULL )
		pImage = new CImage();

	if( pImage )
	{	status = pImage->LoadFile(filename);
		if( status != Ok )
			delete pImage;
	}
	return status;
}

inline Status CreateImage( LPCIMAGEARRAY& pImage, LPTSTR filename )
{	
	Status status = OutOfMemory;
	if( pImage == NULL )
		pImage = new CImageArray();
	if( pImage )
	{	status = pImage->LoadFile(filename);
		if( status != Ok )
			delete pImage;
	}
	return status;
}


#endif //__ANWIMAGE_XIMAGE_H_
