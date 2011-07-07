#if !defined(AFX_SKINHEADERCTRL_H__8B0847B1_B4E6_4372_A62D_038582FFEA5C__INCLUDED_)
#define AFX_SKINHEADERCTRL_H__8B0847B1_B4E6_4372_A62D_038582FFEA5C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SkinHeaderCtrl.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CSkinHeaderCtrl window

class CSkinHeaderCtrl : public CHeaderCtrl
{
	DECLARE_DYNAMIC(CSkinHeaderCtrl);
// Construction
public:
	CSkinHeaderCtrl();

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

	void SetTextFont(CString sFacename, int nHeight=-11, UINT fType=FT_NORMAL, BYTE lfQuality=DEFAULT_QUALITY);
	void SetTextColor(COLORREF crNormal, LPCOLORREF pCrDown=NULL, LPCOLORREF pCrGray=NULL);

	void SetHTextColor(COLORREF crNormalHigh, LPCOLORREF pCrDownHigh=NULL) 
		{	m_crHText = crNormalHigh;
			if(pCrDownHigh)	m_crHDText = *pCrDownHigh;
		}
	
	void SetTextOffset(int offset) { m_nTxtOffset = offset; }
	void SetBkColor(COLORREF crBk) 
		{	m_crBk = crBk; 
		}

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

	void SetTextLine(TextStyle ts);
	inline void SetTextShadowSize(CSize &size);
// Attributes
public:

// Operations
public:
	virtual BOOL LoadBitmap( LPCTSTR lpszLeftImage, LPCTSTR lpszCenterImage , LPCTSTR lpszRightImage ,BOOL bSafeDel = TRUE);
	virtual BOOL LoadBitmap(CImageArray* pImgLeft,CImageArray* pImgCenter,CImageArray* pImgRight, BOOL bSafeDel = TRUE );
	virtual BOOL LoadIcon(CImageArray *pImgIcon,int iIndex);
	virtual BOOL LoadIcon(LPCTSTR lpszIcon,int iIndex);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkinHeaderCtrl)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL AddColText(LPCTSTR lpszText,int iIndex = -1);
	int HitTest(CPoint &point);
	virtual ~CSkinHeaderCtrl();

	// Generated message map functions
protected:
	BOOL m_bEnable;
	//
	int  m_nCount;
	//index of the item which has been hover
	int	 m_iHover;		
	//index of item which has been down
	int  m_iDown;
	BOOL m_bTracking;

	// button text
	BOOL m_bShowText;
	int  m_nAlignment;
	
	//text
	HFONT m_hFont;
	COLORREF m_crText, m_crDText, m_crGText;
	COLORREF m_crHText, m_crHDText;
	COLORREF m_crBk;
	int  m_nOffset;
	int  m_nTxtOffset;
	BOOL m_bButtonType;

	//set three parts of the background image
	CArray<CImageArray *,CImageArray *> m_ayImgIcon;
	CArray<int ,int> m_ayiIcon;
	CArray<CString,CString> m_ayStrCol;

	CImageArray *m_pImg[3];

	BOOL m_bNormalLook;
	int  m_nNormalLook;

	BOOL m_bSafeDel;

	// keep old status text
	CString m_sStatus;
	CString m_sOldStatus;

	// notify button
	BOOL m_bDownNotify;
	BOOL m_bUpNotify;

	BOOL m_bTextLine;
	CSize m_TextShadowSize;

	//{{AFX_MSG(CSkinHeaderCtrl)
//	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	//}}AFX_MSG
	afx_msg LRESULT OnMouseHover(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnMouseLeave(WPARAM wparam, LPARAM lparam);

	DECLARE_MESSAGE_MAP()
};

inline void CSkinHeaderCtrl::SetTextShadowSize(CSize &size)
{
	m_TextShadowSize = size;
}
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKINHEADERCTRL_H__8B0847B1_B4E6_4372_A62D_038582FFEA5C__INCLUDED_)
