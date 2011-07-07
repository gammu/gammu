#if !defined(AFX_SLIDERCTRLEX2_H__57EC699A_9975_465A_BE5B_EC8B584D4E64__INCLUDED_)
#define AFX_SLIDERCTRLEX2_H__57EC699A_9975_465A_BE5B_EC8B584D4E64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SliderCtrlEx2.h : header file
//

#include "_anwImage.h"
#include "sliderctrlex.h"

#ifndef DEFSLD2
#define DEFSLD2
typedef struct _SLDCTRLEX2
{
	CRect	rect;
	
    TCHAR	bar_Image[_MAX_PATH];
	CRect	thumb_range;
	CBorder bar_border;
	TCHAR	thumb_Image[_MAX_PATH];
	CSize	range;
	int		pos;

	CPoint  left_pos;
	TCHAR	left_Button[_MAX_PATH];
	CPoint  right_pos;
	TCHAR	right_Button[_MAX_PATH];

}SLDCTRLEX2;

typedef struct _tagSLDCTRLEX2BUF
{
	CRect	rect;
	CRect	thumb_range;
	CBorder bar_border;
	CSize	range;
	int		pos;

	CPoint  left_pos;
	CPoint  right_pos;

	CImageArray*	pImgBar;
	CImageArray*	pImgThumb;
	CImageArray*	pImgRB;
	CImageArray*	pImgLB;

}SLDCTRLEX2_BUF;

#endif// EFSLD2

/////////////////////////////////////////////////////////////////////////////
// CSliderCtrlEx2 window

class CONTROL_EXT CSliderCtrlEx2 : public CSliderCtrlEx
{	
	DECLARE_DYNAMIC(CSliderCtrlEx2);

// Construction
public:
	CSliderCtrlEx2();

// Attributes
public:
	
	CButtonEx	m_btnLeft;
	CButtonEx	m_btnRight;

// Operations
public:
	BOOL LoadSetting( LPCTSTR SkinPath, LPCTSTR profile, LPCTSTR key );
	BOOL LoadSetting( SLDCTRLEX2& slddata );
	BOOL LoadSetting( SLDCTRLEX2_BUF& slddata, BOOL bSafeDel = TRUE );

	void DrawChannel( Graphics& graphics );
	void xDraw();
	BOOL EnableWindow( BOOL bEnable = TRUE );
	void SetRightPos( CPoint right_pos )	
	{
		m_btnRight.SetWindowPos(NULL, right_pos.x, right_pos.y, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER);
	};
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSliderCtrlEx2)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSliderCtrlEx2();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSliderCtrlEx2)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
//	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
//	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
//	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnClickLeft();
	afx_msg void OnClickRight();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SLIDERCTRLEX2_H__57EC699A_9975_465A_BE5B_EC8B584D4E64__INCLUDED_)
