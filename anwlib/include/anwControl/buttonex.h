#if !defined(AFX_BUTTONEX_H__16C6D980_BD45_11D3_BDA3_00104B133581__INCLUDED_)
#define AFX_BUTTONEX_H__16C6D980_BD45_11D3_BDA3_00104B133581__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ButtonEx.h : header file
//
#include "_anwImage.h"
#include "_defControlext.h"

#define BA_LEFT		0
#define BA_CENTER	1
#define BA_RIGHT	2

/////////////////////////////////////////////////////////////////////////////
// CButtonEx by Niek Albers
// Thanks to some people for the tooltip.
// A cool CBitmapButton derived class with 3 states,
// Up/Down/Hover.


class CONTROL_EXT CButtonEx : public CButton
{
	DECLARE_DYNAMIC(CButtonEx);

public:
	// Construction
	CButtonEx();

	//
	BOOL IsWindowEnabled() { return m_bEnable; }
	BOOL EnableWindow(BOOL bEnable = TRUE);

	//
	void ShowText(BOOL bShow) { m_bShowText = bShow; }
	void SetTextAlignment(int align) 
		{ 
			m_nAlignment = max(min(align, 3), 0);
			Invalidate(FALSE);
		}

//	void SetTextFont(LPCTSTR lpFaceName, int nHeight = 11, BOOL bBold = FALSE);
	void SetTextFont(CString sFacename, int nHeight=-11, UINT fType=FT_NORMAL, BYTE lfQuality=DEFAULT_QUALITY);
	void SetTextColor(COLORREF crNormal, LPCOLORREF pCrDown=NULL, LPCOLORREF pCrGray=NULL);

	void SetHTextColor(COLORREF crNormalHigh, LPCOLORREF pCrDownHigh=NULL) 
		{	m_crHText = crNormalHigh;
			if(pCrDownHigh)	m_crHDText = *pCrDownHigh;
		}
	
	void SetTextOffset(int offset) { m_nTxtOffset = offset; }
	void SetBkColor(COLORREF crBk) 
		{	m_crBk = crBk; 
			m_bTransparentBk = FALSE;
		}
	void TransparentBk( BOOL bFlag ){ m_bTransparentBk = bFlag; }
	//
	void InitToolTip();
	void SetToolTipText(LPCTSTR pText, BOOL bActivate = TRUE);
	void SetToolTipText(int nId, BOOL bActivate = TRUE);
	void ActivateTooltip(BOOL bActivate = TRUE);

	int GetCheck() { return m_bChecked; }
	void SetCheck(BOOL check = TRUE);

	void SetDisableLook(int nLook)	// 0: UP, 1: DOWN, 2: HIGHLIGHT
		{	m_nNormalLook = max(0, min(nLook, 2));	}

	BOOL Enable(BOOL bEnable, BOOL bNormalLook = FALSE);

	void SetButtonType( BOOL bType )
		{	m_bButtonType = bType;	}

	// notify button
	void SetNotify(BOOL bDown, BOOL bUp)
		{	m_bDownNotify = bDown;
			m_bUpNotify = bUp;
		}

	int GetWidth();
	int GetHeight();

// Attributes
protected:
	CToolTipCtrl m_ToolTip;
	BOOL m_bEnable;
	//
	int  m_nCount;
	BOOL m_bHover;						// indicates if mouse is over the button
	BOOL m_bTracking;

	// button text
	BOOL m_bShowText;
	int  m_nAlignment;
	BOOL m_bTransparentBk;

	HFONT m_hFont;
	COLORREF m_crText, m_crDText, m_crGText;
	COLORREF m_crHText, m_crHDText;
	COLORREF m_crBk;
	int  m_nOffset;
	int  m_nTxtOffset;
	BOOL m_bButtonType;

	CImage *m_pImgBk;
	CImageArray *m_pImg;

	BOOL m_bChecked;
	BOOL m_bNormalLook;
	int  m_nNormalLook;

	BOOL m_bSafeDel;

	// keep old status text
	CString m_sStatus;
	CString m_sOldStatus;

	// notify button
	BOOL m_bDownNotify;
	BOOL m_bUpNotify;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CButtonEx)
	protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CButtonEx();
	virtual BOOL LoadBitmap( LPCTSTR pszFileName );
	virtual BOOL LoadBitmap(CImageArray* pImag, BOOL bSafeDel = TRUE );

	// Generated message map functions
public:

	//{{AFX_MSG(CButtonEx)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LRESULT OnMouseHover(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnMouseLeave(WPARAM wparam, LPARAM lparam);

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUTTONEX_H__16C6D980_BD45_11D3_BDA3_00104B133581__INCLUDED_)
