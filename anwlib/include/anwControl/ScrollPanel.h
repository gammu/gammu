#if !defined(AFX_SCROLLPANEL_H__69C8A8AC_BDD6_419B_B0B6_3FDC36547627__INCLUDED_)
#define AFX_SCROLLPANEL_H__69C8A8AC_BDD6_419B_B0B6_3FDC36547627__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScrollPanel.h : header file
//
#include "panel.h"
/////////////////////////////////////////////////////////////////////////////
// CScrollPanel window

//SCROLL_MODE
#define SCP_NOAUTOBORTH	0x0001
#define SCP_NOAUTOHORZ	0x0002
#define SCP_NOAUTOVERT	0x0004
#define SCP_NOHORZ		0x0008
#define SCP_NOVERT		0x0010
#define SCP_CENTER		0x0020

class CONTROL_EXT CScrollPanel : public CPanel
{
// Construction
public:
	CScrollPanel();

protected: 
	DECLARE_DYNAMIC(CScrollPanel)

// Attributes
public:
	CSize	m_szView;
	CPanel*	m_pPanel;

	// SB_HORZ  0
	// SB_VERT	1
	int m_nScrlPos[2],
		m_nScrlMax[2],
		m_nScrlPage[2];

	BOOL m_bHasBar[2], m_bAuto[2];
	BOOL m_bFitCenter;

// Operations
public:
	void SetScrollMode( WORD nMode );
	void SetPanel( CPanel* pPanel ){	m_pPanel = pPanel;	};

	//void SetScrollbar( int nBar, BOOL bShow );
	//void RemoveScrollbar( int nBar );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScrollPanel)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CScrollPanel();

protected:
	virtual BOOL xSetViewSize( int cx, int cy );
	void xSetScrollInfo( int nBar, int nPos, int nMax, int nPage, BOOL bRedraw = TRUE );
	void xEnableScrollBar( int nBar, BOOL bEnable );

	BOOL xHScroll(UINT nSBCode, UINT nPos);
	BOOL xVScroll(UINT nSBCode, UINT nPos);

	// Generated message map functions
protected:
	//{{AFX_MSG(CScrollPanel)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCROLLPANEL_H__69C8A8AC_BDD6_419B_B0B6_3FDC36547627__INCLUDED_)
