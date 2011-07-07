#if !defined(AFX_PROGRESSCTRLEX_H__1C773B57_C7D6_4E94_9AAD_BABA3C7652DF__INCLUDED_)
#define AFX_PROGRESSCTRLEX_H__1C773B57_C7D6_4E94_9AAD_BABA3C7652DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressCtrlEx.h : header file
//
#include "_defControlext.h"

// To set text format use "SetTextFormat" and "HideText"
#define PBS_SHOW_PERCENT         0x0100
#define PBS_SHOW_POSITION        0x0200
#define PBS_SHOW_TEXTONLY        0x0300
#define PBS_TEXTMASK             0x0300

// To set this attributes use	ModifyStyle() or appropriated functions
#define PBS_TIED_TEXT            0x1000
#define PBS_RUBBER_BAR           0x2000
#define PBS_REVERSE              0x4000
#define PBS_SNAKE                0x8000

/////////////////////////////////////////////////////////////////////////////
// CProgressCtrlEx window

class CONTROL_EXT CProgressCtrlEx : public CProgressCtrl
{
// Construction
public:
	CProgressCtrlEx();

// Attributes
public:
	inline void SetGradientColors(COLORREF clrStart, COLORREF clrEnd);
	inline void GetGradientColors(COLORREF& clrStart, COLORREF& clrEnd);
	void SetGradientColorsX(int nCount, COLORREF clrFirst, COLORREF clrNext, ...);
	inline const CDWordArray& GetGradientColorsX();
	
	inline void SetBarBrush(CBrush* pbrBar);
	inline CBrush* GetBarBrush();

	inline void SetBarColor(COLORREF clrBar);
	// bk
	inline void SetBkColor(COLORREF clrBk);
	inline COLORREF GetBkColor();
	inline void SetBkBrush(CBrush* pbrBk);
	inline CBrush* GetBkBrush();

	// text
	inline void SetTextColor(COLORREF clrTextOnBar, COLORREF clrTextOnBk = -1);
	inline COLORREF GetTextColor();
	inline COLORREF GetTextColorOnBk();

	// show percent
	inline void SetShowPercent(BOOL fShowPercent = TRUE);
	inline BOOL GetShowPercent();

	// show Position
	//20031211 janice add
	inline void SetShowPosition(LPCTSTR szSum = NULL, BOOL fShowPercent = TRUE);
	inline BOOL GetShowPosition();


	// text
	void SetTextFormat(LPCTSTR szFormat, DWORD ffFormat = PBS_SHOW_TEXTONLY);
	inline void HideText();
	inline void SetTiedText(BOOL fTiedText = TRUE);
	inline BOOL GetTiedText();

	// rubberbar
	inline void SetRubberBar(BOOL fRubberBar = TRUE);
	inline BOOL GetRubberBar();

	//reserve
	inline void SetReverse(BOOL fReverse = TRUE);
	inline BOOL GetReverse();

	//snake
	inline void SetSnake(BOOL fSnake = TRUE);
	inline BOOL GetSnake();
	inline void SetSnakeTail(int nTailSize);
	inline int  GetSnakeTail();

	// border
	inline void SetBorders(const CRect& rcBorders);
	inline const CRect& GetBorders();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressCtrlEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CProgressCtrlEx();

protected:
	struct CDrawInfo
	{
		CDC *pDC;
		DWORD dwStyle;
		CRect rcClient;
		int nCurPos;
		int nLower;
		int nUpper;
	};
	
	virtual void DrawMultiGradient(const CDrawInfo& info, const CRect &rcGrad, const CRect &rcClip);
	virtual void DrawGradient(const CDrawInfo& info, const CRect &rcGrad, const CRect &rcClip, COLORREF clrStart, COLORREF clrEnd);
	virtual void DrawText(const CDrawInfo& info, const CRect &rcMax, const CRect &rcGrad);
	virtual void DrawClippedText(const CDrawInfo& info, const CRect& rcClip, CString& sText, const CPoint& ptWndOrg);
	CRect ConvertToReal(const CDrawInfo& info, const CRect& rcVirt);
	virtual BOOL SetSnakePos(int& nOldPos, int nNewPos, BOOL fIncrement = FALSE);

	// color atributes
	BOOL	m_bNoneGradColors;
	CDWordArray m_ardwGradColors;
	COLORREF m_clrBar;
	CBrush* m_pbrBar; 
	COLORREF m_clrBk;
	CBrush* m_pbrBk;

	COLORREF m_clrTextOnBar;
	COLORREF m_clrTextOnBk;

	// snake attributes
	int m_nTail;
	int m_nTailSize;
	int m_nStep;

	CRect m_rcBorders;
	// Generated message map functions
protected:
	//{{AFX_MSG(CProgressCtrlEx)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg LRESULT OnSetBarColor(WPARAM, LPARAM);
	afx_msg LRESULT OnSetBkColor(WPARAM, LPARAM);
	afx_msg LRESULT OnSetPos(WPARAM, LPARAM);
	afx_msg LRESULT OnDeltaPos(WPARAM, LPARAM);
	afx_msg LRESULT OnStepIt(WPARAM, LPARAM);
	afx_msg LRESULT OnSetStep(WPARAM, LPARAM);	
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

inline void CProgressCtrlEx::SetGradientColors(COLORREF clrStart, COLORREF clrEnd) 
{	m_ardwGradColors.SetSize(2); 
	m_ardwGradColors.SetAt(0, clrStart); 
	m_ardwGradColors.SetAt(1, clrEnd); 
	m_bNoneGradColors = FALSE;
}

inline void CProgressCtrlEx::GetGradientColors(COLORREF& clrStart, COLORREF& clrEnd) 
{	clrStart = m_ardwGradColors[0]; 
	clrEnd = m_ardwGradColors[1];
}

inline const CDWordArray& CProgressCtrlEx::GetGradientColorsX()
{	return m_ardwGradColors; 
}

inline void CProgressCtrlEx::SetBarBrush(CBrush* pbrBar) 
{	m_pbrBar = pbrBar; 
}

inline CBrush* CProgressCtrlEx::GetBarBrush()
{	return m_pbrBar; 
}

inline void CProgressCtrlEx::SetBarColor(COLORREF clrBar)
{	m_clrBar = clrBar;
	m_bNoneGradColors = TRUE;
}

inline void CProgressCtrlEx::SetBkColor(COLORREF clrBk)
{	m_clrBk = clrBk; 
}

inline COLORREF CProgressCtrlEx::GetBkColor()
{	return m_clrBk; 
}

inline void CProgressCtrlEx::SetBkBrush(CBrush* pbrBk)
{	m_pbrBk = pbrBk; 
}

inline CBrush* CProgressCtrlEx::GetBkBrush()
{	return m_pbrBk; 
}

inline void CProgressCtrlEx::SetTextColor(COLORREF clrTextOnBar, COLORREF clrTextOnBk)
{	m_clrTextOnBar = m_clrTextOnBk = clrTextOnBar; 
	if(clrTextOnBk != -1) m_clrTextOnBk = clrTextOnBk;
}

inline COLORREF CProgressCtrlEx::GetTextColor() 
{	return m_clrTextOnBar; 
}

inline COLORREF CProgressCtrlEx::GetTextColorOnBk()
{	return m_clrTextOnBk; 
}

//20031211 janice add
////////////////////////////////////////////////////////
inline void CProgressCtrlEx::SetShowPosition(LPCTSTR szSum, BOOL fShowPercent)
{	CString str(szSum);
	TCHAR  szFormat[_MAX_PATH] = _T("%d/");
	_tcscat(szFormat, str);
	SetTextFormat(fShowPercent ? szFormat : NULL, PBS_SHOW_POSITION);
}

inline BOOL CProgressCtrlEx::GetShowPosition() 
{	return GetStyle()&PBS_SHOW_POSITION; 
}
////////////////////////////////////////////////////////

inline void CProgressCtrlEx::SetShowPercent(BOOL fShowPercent)
{	
    SetTextFormat(fShowPercent ? _T("%d%%") : NULL, PBS_SHOW_PERCENT); 
}

inline BOOL CProgressCtrlEx::GetShowPercent() 
{	return GetStyle()&PBS_SHOW_PERCENT; 
}

inline void CProgressCtrlEx::HideText() 
{	SetTextFormat(0);
}

inline void CProgressCtrlEx::SetTiedText(BOOL fTiedText)
{	ModifyStyle(fTiedText ? 0 : PBS_TIED_TEXT, fTiedText ? PBS_TIED_TEXT : 0, SWP_DRAWFRAME); 
}

inline BOOL CProgressCtrlEx::GetTiedText() 
{	return GetStyle()&PBS_TIED_TEXT; 
}

inline void CProgressCtrlEx::SetRubberBar(BOOL fRubberBar)
{	ModifyStyle(fRubberBar ? 0 : PBS_RUBBER_BAR, fRubberBar ? PBS_RUBBER_BAR : 0, SWP_DRAWFRAME); 
}

inline BOOL CProgressCtrlEx::GetRubberBar()
{	return GetStyle()&PBS_RUBBER_BAR; 
}

inline void CProgressCtrlEx::SetReverse(BOOL fReverse)
{	ModifyStyle(fReverse ? 0 : PBS_REVERSE, fReverse ? PBS_REVERSE : 0, SWP_DRAWFRAME); 
}

inline BOOL CProgressCtrlEx::GetReverse()
{	return GetStyle()&PBS_REVERSE; 
}

inline void CProgressCtrlEx::SetSnake(BOOL fSnake)
{	ModifyStyle(fSnake ? 0 : PBS_SNAKE|PBS_RUBBER_BAR, fSnake ? PBS_SNAKE|PBS_RUBBER_BAR : 0, SWP_DRAWFRAME); 
}

inline BOOL CProgressCtrlEx::GetSnake() 
{	return GetStyle()&PBS_SNAKE; 
}

inline void CProgressCtrlEx::SetSnakeTail(int nTailSize)
{	m_nTailSize = nTailSize; 
}

inline int CProgressCtrlEx::GetSnakeTail()
{	return m_nTailSize; 
}

inline void CProgressCtrlEx::SetBorders(const CRect& rcBorders) 
{	m_rcBorders = rcBorders; 
}

inline const CRect& CProgressCtrlEx::GetBorders()
{	return m_rcBorders; 
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSCTRLEX_H__1C773B57_C7D6_4E94_9AAD_BABA3C7652DF__INCLUDED_)
