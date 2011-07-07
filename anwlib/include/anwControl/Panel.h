#if !defined(AFX_PANEL_H__CC4C3E2D_A027_43D8_BCAA_EC56E1AC604A__INCLUDED_)
#define AFX_PANEL_H__CC4C3E2D_A027_43D8_BCAA_EC56E1AC604A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Panel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPanel window
#include "_defControlext.h"
#include "_anwImage.h"

class CONTROL_EXT CPanel : public CWnd
{
// Construction
public:
	CPanel();

protected: 
	DECLARE_DYNAMIC(CPanel)

// Attributes
public:
	HBRUSH m_hBrush;
	COLORREF m_crBrush;
	CImageArray *m_pImgBk;
	int m_nDrawType;

//	CImage *m_pImg;
	BOOL m_bSafeDel;
	CRect m_rect;
// Operations
public:
	void SetDrawType(int type) { m_nDrawType = type; }
	void SetBrushColor(COLORREF color);
	COLORREF GetBrushColor() { return m_crBrush; }
	void SetBrush(HBRUSH hbr);
	BOOL LoadBitmap(LPCTSTR pszFileName);
	BOOL LoadBitmap(CImageArray* pImage, BOOL bSafeDel = TRUE );

	BOOL HasBitmap(){	return(m_pImgBk)?TRUE:FALSE;	};

	void SetBorder( CBorder& border)
		{	if(m_pImgBk)
				m_pImgBk->SetBorder(border);
		}
	CBorder GetBorder()
		{	return (m_pImgBk)? m_pImgBk->GetBorder():CBorder(0,0,0,0);
		}
	virtual BOOL CreateFromSetting( CWnd* pParent, CRect& parRect, LPSTR file )
			{	return TRUE;	};

	void xPaint( CDC* pDC );
	void xSize();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPanel)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPanel();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPanel)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PANEL_H__CC4C3E2D_A027_43D8_BCAA_EC56E1AC604A__INCLUDED_)
