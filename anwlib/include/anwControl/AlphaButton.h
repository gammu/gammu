#if !defined(AFX_ALPHABUTTON_H__DA961638_195F_4EF7_A536_3E1650CC1B7B__INCLUDED_)
#define AFX_ALPHABUTTON_H__DA961638_195F_4EF7_A536_3E1650CC1B7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AlphaButton.h : header file
//
#include "ButtonEx.h"

enum	{
	Btn_Hide,
	Btn_Normal,
	Btn_Down,
	Btn_Hilight,
	Btn_Disable
};
/////////////////////////////////////////////////////////////////////////////
// CAlphaButton window

class CONTROL_EXT CAlphaButton : public CButtonEx
{
	DECLARE_DYNAMIC(CAlphaButton);
// Construction
public:
	CAlphaButton();

// Attributes
public:

// Operations
public:
	void SetTextOffset( int nX=0, int nY=0 )
	{
		m_nTextOffsetX = nX;
		m_nTextOffsetY = nY;
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlphaButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetBkImage(CImage *pImgBk);
	virtual ~CAlphaButton();
	virtual BOOL LoadBitmap( LPCTSTR pszFileName );
	virtual BOOL LoadBitmap(CImageArray* pImag, BOOL bSafeDel = TRUE );
	int		GetButtonStatus()	{ return m_nBtnStatus; };
	// Generated message map functions
protected:
	int	m_nBtnStatus;	//use Btn_Hide ...
	int m_nTextOffsetX;
	int m_nTextOffsetY;
	//{{AFX_MSG(CAlphaButton)
	afx_msg LRESULT OnMouseHover(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnMouseLeave(WPARAM wparam, LPARAM lparam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALPHABUTTON_H__DA961638_195F_4EF7_A536_3E1650CC1B7B__INCLUDED_)
