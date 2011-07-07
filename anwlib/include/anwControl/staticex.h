#if !defined(AFX_STATICEX_H__44275CB9_F1E8_47C5_91B7_1FCC58354B37__INCLUDED_)
#define AFX_STATICEX_H__44275CB9_F1E8_47C5_91B7_1FCC58354B37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StaticEx.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStaticEx window
#include "_defControlext.h"
#include "_anwImage.h"

class CONTROL_EXT CStaticEx : public CStatic
{
	DECLARE_DYNAMIC(CStaticEx);

// Construction
public:
	CStaticEx();

// Attributes
public:
	HBRUSH m_hBrush;
	CImageArray *m_pImgBk;
	int m_nDrawType;

	UINT m_type;
	CString  m_sFacename;
	int      m_cyFont;
	HFONT	 m_hFont;

	COLORREF m_crText;
	COLORREF m_crGray;

	BOOL m_bHover;
	BOOL m_bTracking;

	BOOL m_bSafeDel;
// Operations
public:
	void SetDrawType(int type) { m_nDrawType = type; }
	void SetBrushColor(COLORREF color);
	void SetBrush(HBRUSH hbr);
	BOOL LoadBitmap(LPCTSTR pszFileName);
	BOOL LoadBitmap(CImageArray* pImage, BOOL bSafeDel);
	void SetBorder( int l, int t, int w, int h)
		{	if(m_pImgBk)	m_pImgBk->SetBorder(l, t, w, h);	}

	void SetTextFont( CFont *font);
	void SetTextFont( HFONT font);
	void SetTextFont( HWND hWnd);
	//void SetTextFont(CString sFacename, int nHeight, BOOL bBold);
	void SetTextFont(CString sFacename, int nHeight=-11, UINT fType=FT_NORMAL, BYTE lfQuality=DEFAULT_QUALITY);
	void SetTextType(UINT type)
			{	m_type = type;	}
	void SetTextColor( LPCOLORREF pCrText, LPCOLORREF pCrGray=NULL );

	void SetWindowText(LPCTSTR lpszString)
	{	if( IsWindowVisible() )
		{	SetRedraw(FALSE);
			CStatic::SetWindowText(lpszString);
			SetRedraw(TRUE);
			RedrawWindow();
		}
		else
			CStatic::SetWindowText(lpszString);
	}

	void EnableWindow( BOOL bEnable )
	{	if(!GetSafeHwnd())
			return;
		if(bEnable)
			ModifyStyle(WS_DISABLED,0);
		else
			ModifyStyle(0,WS_DISABLED);
		RedrawWindow();
	}
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStaticEx)
	protected:
//	virtual void PreSubclassWindow();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CStaticEx();

	// Generated message map functions
protected:
	//{{AFX_MSG(CStaticEx)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATICEX_H__44275CB9_F1E8_47C5_91B7_1FCC58354B37__INCLUDED_)
