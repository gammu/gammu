/*/////////////////////////////////////////////////////////////////////////////

Copyright (c) 2000-2002 Advanced & Wise Technology Corp.  All Rights Reserved.
File Name:		anwbase.h
Author:			max huang
Description:	global header file,
Platform:		Win32
Compiler:		MS Visual C++ 6.0 in sp5 with Platform SDK Nov 2001.
Data:			2001/09/11 v0.1			max huang start develop
				2003/01/30						  add to anwlib

/////////////////////////////////////////////////////////////////////////////*/

/*=============================================================================
Version history:
 	(1)1.0.0.1/05/2001	DLL	First version. 		Created by max huang.
=============================================================================*/
#ifndef __ANWBASE_H_
#define __ANWBASE_H_

enum OS_WIN32TYPE
{
	OS_NODEFINE=0,
	OS_WIN32S,
	OS_WINNT3,
	OS_WIN95,
	OS_WIN95_OSR2,
	OS_WIN98,
	OS_WIN98SE,
	OS_WINME,
	OS_WINNT4,
	OS_WIN2K,
	OS_WINXP
};

// safe delete pointer
template <class TYPE>
VOID* SAFE_DELPTR(TYPE& x)
{	if(x)
	{	delete x;
		x = NULL;
	}
	return x;
}

#ifndef __CBORDER_H_
#define __CBORDER_H_

class AFX_EXT_CLASS CBorder
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
#endif

///////////////////////////////////////////////////////////////////////////////
// ab_base.cpp
// ANSI string to other type

//
//	BOOL		str2bool( LPCTSTR str )			- string( "true/false" ) to BOOL
//	int			str2int( LPCTSTR str )			- string( "x" ) to int
//	double		str2double( LPCTSTR str )		- string( "x.x" ) to double
//	CPoint		str2point( LPCTSTR str )		- string( "x,y" ) to CPoint
//	CSize		str2size( LPCTSTR str )			- string( "cx,cy" ) to CSize
//	CRect		str2rect( LPCTSTR str )			- string( "l,t,r,b" ) to CRect
//	CRect		str2rect2( LPCTSTR str )		- string( "l,t,w,h" ) to CRect
//	COLORREF	str2colorref( LPCTSTR str )		- string( "r,g,b" ) to COLORREF
//	CBorder		str2border( LPCTSTR str )		- string( "l,t,r,b" ) to CBorder
//	WORD		str2fontstyle( LPCTSTR str )	- string( "normal" ) to font Style
//	BYTE		str2fontquality( LPCTSTR str )	- string( "default" ) to font Quality
//
BOOL str2bool( LPCTSTR str );
inline int str2int( LPCTSTR str )
{
#ifdef _UNICODE
     return ( str == NULL || str[0] == NULL )? 0: _wtoi(str);
#else 
     return ( str == NULL || str[0] == NULL )? 0: atoi(str);
#endif
}
   
_CRTIMP double __cdecl _wtof(const wchar_t *);

inline double str2double( LPCTSTR str )
{
#ifdef _UNICODE
    return ( str == NULL || str[0] == NULL )? 0.0 : _wtof(str);
#else
    return ( str == NULL || str[0] == NULL )? 0.0 : atof(str);
#endif
    
}

CPoint str2point( LPCTSTR str );
CSize str2size( LPCTSTR str );
CRect str2rect( LPCTSTR str );
CRect str2rect2( LPCTSTR str );
COLORREF str2colorref( LPCTSTR str );
CBorder str2border( LPCTSTR str );
WORD str2fontstyle( LPCTSTR str );
BYTE str2fontquality( LPCTSTR str );


// change type to ANSI string
//
//	void bool2str( BOOL bValue, LPSTR str )			- BOOL to string( "true/false" )
//	void int2str( int nValue, LPSTR str )			- int to string( "x" )
//	void double2str( double dValue, LPSTR str )		- double to string( "x.x" )
//	void point2str( LPPOINT pPoint, LPSTR str )		- CPoint to string( "x,y" )
//	void size2str( LPSIZE pSize, LPSTR str )		- CSize to string( "cx,cy" )
//	void border2str( CBorder* pBorddr, LPSTR str )	- CRect to string( "l,t,r,b" )
//	void rect2str( LPRECT pRect, LPSTR str )		- CRect to string( "l,t,w,h" )
//	void rect2str2( LPRECT pRect, LPSTR str )		- COLORREF to string( "r,g,b" )
//	void colorref2str( COLORREF clr, LPSTR str )	- CBorder ro string( "l,t,r,b" )
//	void fontstyle2str( WORD style, LPSTR str )		- font Style to string( "normal" )
//	fontquality2str( BYTE quality, LPSTR str )		- font Quality to string( "default" )
//
inline void bool2str( BOOL bValue, LPTSTR str )
{	if( str )	
        _tcscpy( str, (bValue)? _T("true") : _T("false") );
}

inline void int2str( int nValue, LPSTR str )
{	if( str )	sprintf( str, "%d", nValue );
}

inline void double2str( double dValue, LPSTR str )
{	if( str )	sprintf( str, "%f", dValue );
}

inline void point2str( LPPOINT pPoint, LPSTR str )
{	if( str )	sprintf( str, "%d,%d", pPoint->x, pPoint->y );
}

inline void size2str( LPSIZE pSize, LPSTR str )
{	if( str )	sprintf( str, "%d,%d", pSize->cx, pSize->cy );
}

inline void border2str( CBorder* pBorddr, LPSTR str )
{	if( str )	sprintf( str, "%d,%d,%d,%d", 
					pBorddr->left, pBorddr->top, pBorddr->right, pBorddr->bottom );
}

/*  rect -> str:left,top,right,bottom */
inline void rect2str( LPRECT pRect, LPSTR str )
{	if( str )
		sprintf( str, "%d,%d,%d,%d", 
			pRect->left, pRect->top, pRect->right, pRect->bottom );
}

/*  rect -> str:x,y,w,h */
inline void rect2str2( LPRECT pRect, LPSTR str )
{	if( str )	sprintf( str, "%d,%d,%d,%d", 
					pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top );
}

inline void colorref2str( COLORREF clr, LPSTR str )
{	if( str )	sprintf( str, "%d,%d,%d", 
					GetRValue(clr), GetGValue(clr), GetBValue(clr) );
}

void fontstyle2str( WORD style, LPSTR str );
void fontquality2str( BYTE quality, LPSTR str );

// get setting from profile
//
//BOOL al_GetSettingRect		key = l,t,r,b -> rect;
//BOOL al_GetSettingInt			key = x -> int;
//BOOL al_GetSettingSize		key = cx,cy -> CSize;
//BOOL al_GetSettingBorder		key = l,t,r,b -> CBorder;
//BOOL al_GetSettingString		key = string -> string;
//BOOL al_GetSettingColor		key = r,g,b -> COLORREF;
//BOOL al_GetSettingFontStyle	key = "bold|italic" -> WORD;
//BOOL al_GetSettingQuality		key = "default" -> BYTE;
//
// get setting from profile
BOOL al_GetSettingRect( LPCTSTR sec, LPCTSTR key, LPCTSTR profile, CRect& rect );
BOOL al_GetSettingInt( LPCTSTR sec, LPCTSTR key, LPCTSTR profile, int& nValue );
BOOL al_GetSettingSize( LPCTSTR sec, LPCTSTR key, LPCTSTR profile, CSize& size );
BOOL al_GetSettingBorder( LPCTSTR sec, LPCTSTR key, LPCTSTR profile, CBorder& border );
BOOL al_GetSettingString( LPCTSTR sec, LPCTSTR key, LPCTSTR profile, LPTSTR szString );
BOOL al_GetSettingColor( LPCTSTR sec, LPCTSTR key, LPCTSTR profile, COLORREF& color );
BOOL al_GetSettingFontStyle( LPCTSTR sec, LPCTSTR key, LPCTSTR profile, WORD& style );
BOOL al_GetSettingQuality( LPCTSTR sec, LPCTSTR key, LPCTSTR profile, BYTE& quality );
BOOL al_GetSettingPoint(LPCTSTR sec, LPCTSTR key, LPCTSTR profile, CPoint& point);

// get setting from lang define
BOOL al_GetLangCodepage( LPCTSTR szLangPath, int& nCodepage);
BOOL al_GetLangProfile( LPCTSTR szLangPath, LPCTSTR szApName, LPTSTR str );
BOOL al_GetLangSkin( LPCTSTR szLangPath, LPTSTR str );

/////////////////////////////////////////////////
// ab_sys.cpp
BOOL al_GetLastErrorStr( TCHAR* pBuf );
void al_ShowLastError();
BOOL al_GetFileVersion( LPTSTR pFile, WORD& MajorVer, WORD& MinorVer, WORD& BuildNumber, WORD& Option );
DWORD al_GetDXVersion();
inline BOOL al_GetScreenWorkArea( LPRECT lprect )
{	return SystemParametersInfo( SPI_GETWORKAREA, 0, lprect, 0 );
};

void al_GetModulePath( HINSTANCE instance, LPTSTR str );
void al_GetScreenResolution( CDC* pDC, CSize& dpi );
int al_GetScreenColors();

/////////////////////////////////////////////////
// ab_io
void al_SplitPath( LPCTSTR full, LPTSTR path, LPTSTR file=NULL, TCHAR ch=_T('\\') );
/* file */
BOOL al_IsFileExist( LPCTSTR pFile );
void al_GetFileExt( LPCTSTR pFile, LPTSTR pExt );
void al_GetFileName( LPCTSTR pFullFile, LPTSTR pFile );
void al_GetExt( LPCTSTR pFullFile, LPTSTR pExt );
void al_GetName( LPCTSTR pFullFile, LPTSTR pName );//20031210 janice get file name not include ext.
FILETIME al_GetFileDate( LPCTSTR pFile );
void al_GetCurDate( FILETIME& curtime );
FILETIME al_GetFileCreateDate(LPCTSTR pFile);
///////////////////////////////////////////////////////////////////////////////
//	pStr: return string
//	dwSize: file size				default
//	pPrecision : precision			"%.2f"
//	pByteStr:	 str of unit		{"GB","MB","KB"}
//	nByteStrle:	 length of Bytestr	3 ( + null-terminated )
//	pSize:		 bytes				{1073741824, 1048576, 1024};
void al_FileSize2Str( LPTSTR pStr, DWORD dwSize, LPTSTR pPrecision=NULL, LPTSTR pByteStr=NULL, int nByteStrle=0, DWORD* pSize=NULL );

/* dir */
UINT al_GetDirFileCount( LPCTSTR pDir);
BOOL al_CopyDirectory( LPCTSTR pFrom, LPCTSTR pTo);
BOOL al_DeleteDirectory( LPCTSTR pDir, BOOL bDelDir=TRUE, FILEOP_FLAGS flags=FOF_SILENT | FOF_NOCONFIRMATION );
BOOL al_IsSubFolderExist( LPCTSTR pDir );

/////////////////////////////////////////////////
// ab_misc

inline void al_charToWchar( LPCSTR str, WCHAR* wstr )
{
    ::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, str, -1,
					wstr, sizeof(wstr)/sizeof(wstr[0]));
}

BOOL al_GetCopyRect(CRect* lpSrcImgRect, CRect *lpDstImgRect, LPPOINT lpDstPt, LPRECT lpSrcRect );

///////////////////////////////////////////////////////////////////////////////
//	get the proportional rect
//
//	rcSrc : Image rect
//	rcDst : output rect
//
//	bCrop = TRUE : Crop to fit.
//			FALSE : Shrink to fit.
//	bAdjust = TRUE: adjust to center.
//	bFit  = TRUE: fit to rcDst.
//			FALSE: if small than rcDst, keep original size.
double al_ShrinkPropRect( CRect rcSrc, CRect& rcDst, BOOL bAdjust=FALSE, BOOL bFit=TRUE, LPPOINT pPtShift = NULL );
double al_CropPropRect( CRect& rcSrc, CRect rcDst, BOOL bAdjust=FALSE, BOOL bFit=TRUE, LPPOINT pPtShift = NULL );

inline double al_GetPropRect( CRect& rcSrc, CRect& rcDst, BOOL bCrop=FALSE, BOOL bAdjust=FALSE, BOOL bFit=TRUE, LPPOINT pPtShift=NULL )
{
	return ( bCrop ) ? al_CropPropRect( rcSrc, rcDst, bAdjust, bFit, pPtShift )
					 : al_ShrinkPropRect( rcSrc, rcDst, bAdjust, bFit, pPtShift );
}

CRect al_AlignRect( LPRECT parentRect, LPRECT lpRect );

///////////////////////////////////////////////////////////////////////////////
/* font */
#define FT_NORMAL		0
#define FT_BOLD			1
#define FT_ITALIC		2
#define FT_UNDERLINE	4
#define FT_STRIKEOUT	8

typedef struct _FontSet
{
	BYTE	char_set;
	TCHAR	name[32];
	int		height;
	WORD	style;
	BYTE	quality;
} FONTSET;

void al_GetLogfont(LPCTSTR pFacename, LOGFONT *plf);

///////////////////////////////////////////////////////////////////////////////
// nType:		FT_NORMAL / 
//				FT_BOLD|FT_ITALIC|FT_UNDERLINE|FT_STRIKEOUT
//
// lfQuality:	DEFAULT_QUALITY /
//				DRAFT_QUALITY /
//				PROOF_QUALITY / 
//				ANTIALIASED_QUALITY
//
//				ref. CreateFont:fdwQuality

// set default font name, size & char set;

HFONT al_CreateFont( LPCTSTR lpFaceName, BYTE CharSet, int nHeight, BOOL bBold, 
					 BOOL bItalic=FALSE, BOOL bUnderline=FALSE, BOOL bStrikeOut=FALSE, BYTE lfQuality=DEFAULT_QUALITY);
HFONT al_CreateFont2( LPCTSTR lpFaceName, BYTE CharSet, int nHeight, UINT nType=FT_NORMAL, BYTE lfQuality=DEFAULT_QUALITY);
inline HFONT al_CreateFont3( FONTSET& fontset )
{	return al_CreateFont2(fontset.name, fontset.char_set, fontset.style, fontset.quality );
}

void al_SetFontInfo( LPCTSTR lpFaceName, BYTE CharSet, int FontSize );
void al_GetFontInfo( LPTSTR lpFaceName, BYTE& CharSet, int& FontSize );

HFONT al_xCreateFont( LPCTSTR lpFaceName, int nHeight, BOOL bBold, BOOL bItalic=FALSE,
						BOOL bUnderline=FALSE, BOOL bStrikeOut=FALSE, BYTE lfQuality=DEFAULT_QUALITY);
HFONT al_xCreateFont2( LPCTSTR lpFaceName, int nHeight, UINT nType=FT_NORMAL, BYTE lfQuality=DEFAULT_QUALITY);

///////////////////////////////////////////////////////////////////////////////
#endif //__ANWBASE_H_
