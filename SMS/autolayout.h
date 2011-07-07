#if !defined(AFX_AUTOLAYOUT_H__200025E8_A900_41A3_A2F6_C97CCD809B69__INCLUDED_)
#define AFX_AUTOLAYOUT_H__200025E8_A900_41A3_A2F6_C97CCD809B69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AutoLayoutDlg.h : header file
//
//#define dim(x) (sizeof(x) / sizeof(x[0]))
struct MYPROFILEMAP_ENTRY {
	UINT  nID;
	LPSTR pSecName;
	void* pControl;
};

struct MYRESIZEMAP_ENTRY {
	UINT  nID;
	RECT  rcRect;
};

struct MYTEXTMAP_ENTRY {
	UINT  nID;
	UINT  nTextID;
	UINT  nTipsID;
};

enum BitmapMode {
	BM_None = 0,
	BM_FillColor = 1,
	BM_SingleTiled = 2,
	BM_SingleStretched = 3,
	BM_HorizontalTiled = 4,
	BM_VerticalTiled = 5,
	BM_NinePalette = 6,
};

//Special color, assign this color in Bk_Color, mean not use Bk_Color
#define NBKCOLOR	RGB(1,2,3)
typedef struct FontInfoTag	{
	FontInfoTag()
	{
		_tcscpy(szFontName, _T("System"));
		nFontSize = 10;
		nFontStyle = 0;	//FT_BOLD, FT_ITALIC, FT_UNDERLINE, FT_STRIKEOUT
		nNormalColor = RGB(0, 0, 0);	//Black
		nDownColor	 = RGB(0, 0, 0);	//Black
		nGrayColor   = RGB(200, 200, 200);	//Gray Color
		nHightColor = RGB(0, 0, 0);	//Black
		nBackColor = NBKCOLOR;	//Black
	};
	TCHAR		szFontName[_MAX_FNAME];
	int			nFontSize;
	WORD		nFontStyle;
	COLORREF	nNormalColor;
	COLORREF	nDownColor;
	COLORREF	nGrayColor;
	COLORREF	nHightColor;
	COLORREF	nBackColor;
} FontInfo;

/////////////////////////////////////////////////////////////////////////////
// CAutoLayoutDlg dialog

class LAutoLayout
{
// Construction
public:
	LAutoLayout();   // standard constructor
	~LAutoLayout();   // standard constructor

	int	GetMinWidth()	{ return m_MinSize.cx + m_BorderSize.cx; };
	int	GetMinHeight()	{ return m_MinSize.cy; + m_BorderSize.cy; };
	int	GetInitWidth()	{ return m_InitSize.cx; };
	int	GetInitHeight()	{ return m_InitSize.cy; };

	BOOL	SetBitmapMode( LPCSTR szProfile, int nMode );

	static BOOL al_GetSettingResizeRect( LPCSTR sec, LPCSTR key, LPCSTR profile, RECT* rect );
// Implementation
protected:
	BOOL	xLoadControlText( MYTEXTMAP_ENTRY* pTable, CWnd* pWnd=NULL );
	void	xFreeImage();
	BOOL	xLoadProfileSetting( LPCSTR szProfile, MYPROFILEMAP_ENTRY* pTable, BOOL bIncImage=FALSE, CWnd* pWnd=NULL );
	BOOL	xDynamicRescale( CWnd* pWnd );
	BOOL	xStaticRescale( CWnd* pWnd );
	BOOL	xFillBackground( CSize winSize, CImage* pGraphics );
	void	xReadFontSection( LPCTSTR szProfile, LPCTSTR szSecName, FontInfo& outFontInfo );

	MYRESIZEMAP_ENTRY*	m_pResizeTable;
	int		m_nTableSize;
	CSize	m_BorderSize;
	CSize	m_MinSize;
	CSize	m_InitSize;
	CString	m_sSkinPath;
	CString	m_sModulePath;

	//for background image
	int		m_nBgMode;		//see define BitmapMode
//	SolidBrush* m_pBgBrush;
	Color*	m_pBgColor;
	CImage*	m_pImgBg1;
	CImage*	m_pImgBg2;
	CImage*	m_pImgBg3;
	CImage*	m_pImgBg4;
	CImage*	m_pImgBg5;
	CImage*	m_pImgBg6;
	CImage*	m_pImgBg7;
	CImage*	m_pImgBg8;
	CImage*	m_pImgBg9;
};

#endif // !defined(AFX_AUTOLAYOUT_H__200025E8_A900_41A3_A2F6_C97CCD809B69__INCLUDED_)
