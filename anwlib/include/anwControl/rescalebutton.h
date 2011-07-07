// RescaleButton.h: interface for the CRescaleButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESCALEBUTTON_H__FE0434AC_FCC1_4E78_AC07_22D5DCEC720C__INCLUDED_)
#define AFX_RESCALEBUTTON_H__FE0434AC_FCC1_4E78_AC07_22D5DCEC720C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ButtonEx.h"

typedef enum {
	TEXT_SINGLELINE = 0,
    TEXT_MUTILINE = 1
} TextStyle;

class CONTROL_EXT CRescaleButton : public CButtonEx  
{
	DECLARE_DYNAMIC(CRescaleButton);
public:
	void InitImage();
	int GetTextLength();
	BOOL GetFixFlag();
	BOOL m_bLButtonDown;
	int GetButtonWidth();
	int GetRightWidth(){return m_rWidth;}
	int GetCenterWidth(){return m_cWidth;}
	int GetLeftWidth(){return m_lWidth;}
	CRescaleButton();
	virtual ~CRescaleButton();
	
	BOOL LoadRightBg(LPCTSTR pszFileName);
	BOOL LoadRightBg(CImageArray* pImag, BOOL bSafeDel = TRUE);
	
	BOOL LoadLeftBg(LPCTSTR pszFileName);
	BOOL LoadLeftBg(CImageArray* pImag, BOOL bSafeDel = TRUE);
	
	BOOL LoadCenterBg(LPCTSTR pszFileName);
	BOOL LoadCenterBg(CImageArray* pImag, BOOL bSafeDel = TRUE);	
	
	inline void SetButtonPartWidth(int lwidth,int cwidth,int rwidth,BOOL bFix = FALSE);

	void SetTextLine(TextStyle ts);

	inline BOOL Enable(BOOL bEnable, BOOL bNormalLook = FALSE);
	
	inline void SetTextShadowSize(CSize &size);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRescaleButton)
	protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL
 
protected:
	BOOL m_bTextLine;
	int m_nTextLength;
	CImage *m_pImgCBg, *m_pImgRBg, *m_pImgLBg;
	int  m_rWidth,m_lWidth,m_cWidth;
	BOOL m_bRSafeDel,m_bCSafeDel,m_bLSafeDel;
	BOOL m_bFix;	//if TRUE ,rescale the button by length of text.
	CSize m_TextShadowSize;
	//{{AFX_MSG(CRescaleButton)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

inline void CRescaleButton::SetButtonPartWidth(int lwidth,int cwidth,int rwidth,BOOL bFix)
{
	m_lWidth = lwidth;
	m_cWidth = cwidth;
	m_rWidth = rwidth;
	m_bFix = bFix;
}

inline BOOL CRescaleButton::Enable(BOOL bEnable, BOOL bNormalLook)
{
	BOOL bRedraw = ( (m_bEnable != bEnable) || (m_bNormalLook != bNormalLook));
	m_bNormalLook = bNormalLook;
	m_bEnable = bEnable;
	if(bRedraw) 
		Invalidate(FALSE);

	return CWnd::EnableWindow(TRUE);
}

inline void CRescaleButton::SetTextShadowSize(CSize &size)
{
	m_TextShadowSize = size;
}
#endif // !defined(AFX_RESCALEBUTTON_H__FE0434AC_FCC1_4E78_AC07_22D5DCEC720C__INCLUDED_)
