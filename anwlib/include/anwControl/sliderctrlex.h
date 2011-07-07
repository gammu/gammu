#if !defined(AFX_SLIDERCTRLEX_H__4E8A2D38_C586_4134_BF21_1A6D4AC0B61F__INCLUDED_)
#define AFX_SLIDERCTRLEX_H__4E8A2D38_C586_4134_BF21_1A6D4AC0B61F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SliderCtrlEx.h : header file
//

#include "_anwImage.h"
#include "buttonex.h"

/////////////////////////////////////////////////////////////////////////////
// CSliderCtrlEx window
typedef struct _tagSLDCTRLEX
{
	CRect	rect;
	TCHAR	bar_Image[_MAX_PATH];
	CRect	thumb_range;
	CBorder bar_border;
	TCHAR	thumb_Image[_MAX_PATH];
	CSize	range;
	int		pos;
	CPoint  left_pos;

}SLDCTRLEX;

typedef struct _tagSLDCTRLEXBUF
{
	CRect	rect;
	CRect	thumb_range;
	CBorder bar_border;
	CSize	range;
	int		pos;
	CPoint  left_pos;
	CImageArray*	pImgBar;
	CImageArray*	pImgThumb;

}SLDCTRLEX_BUF;

class CONTROL_EXT CSliderCtrlEx : public CSliderCtrl
{
	DECLARE_DYNAMIC(CSliderCtrlEx);

// Construction
public:
	CSliderCtrlEx();

// Attributes
public:
	COLORREF m_crBk;

	CImageArray *m_pImgBk;

	//thumb
	CImageArray *m_pImg;
	CSize m_ButtonSize;
	CRect m_BtnRect;
	CRect m_ThumbRange;
	int	  m_index;
	CPoint m_downPt;

	int	  m_diff;
	float m_Unit;
	int	  m_half;

	CBorder m_border;
	BOOL m_bHitDown;
	BOOL m_bSafeDel;

// Operations
public:
	int GetHeight()	{	return m_pImgBk->GetHeight();	}
	BOOL LoadSetting( LPCTSTR SkinPath, LPCTSTR profile, LPCTSTR key );
	BOOL LoadSetting( SLDCTRLEX& slddata );
	BOOL LoadSetting( SLDCTRLEX_BUF& slddata, BOOL bSafeDel=TRUE );

	BOOL LoadBitmap(LPCTSTR pszBarName, LPCTSTR pszThumbName);
	BOOL LoadBitmap(CImageArray* pImgBar, CImageArray* pImgThumb, BOOL bSafeDel);

	void DrawHorizontalThumb( Graphics& graphics );
	void SetBgColor( COLORREF color )
			{	m_crBk = color; };
	void SetBorder( int l, int t, int r, int b )
			{	if(m_pImgBk)m_pImgBk->SetBorder( l, t, r, b);	}
	void SetBorder( CBorder& border )
			{	if(m_pImgBk)m_pImgBk->SetBorder(border);	}
	void SetThumbRange( CRect rect )
				{	m_ThumbRange = rect;
					m_BtnRect.top = rect.top;
					m_BtnRect.bottom = rect.bottom;
				}
	void SetPos( int nPos );
	void SetRange( int nMin, int nMax );
	virtual void xDraw();
	virtual void DrawChannel( Graphics& graphics );

	//////////////////////////////////////////////
	BOOL xHitThum( CPoint point );
	BOOL xAdjustBtn( CPoint point );
	void xPosToBtnPosition();
	void xBtnPositionToPos();

	void UnitOffset( CPoint point, CRect& rect );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSliderCtrlEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSliderCtrlEx();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSliderCtrlEx)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SLIDERCTRLEX_H__4E8A2D38_C586_4134_BF21_1A6D4AC0B61F__INCLUDED_)
