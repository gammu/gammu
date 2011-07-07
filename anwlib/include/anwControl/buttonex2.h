// ButtonEx2.h: interface for the CButtonEx2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUTTONEX2_H__30D01DA6_7316_475B_85BE_EE8EEEAFFF44__INCLUDED_)
#define AFX_BUTTONEX2_H__30D01DA6_7316_475B_85BE_EE8EEEAFFF44__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ButtonNotify.h : header file
//
#include "ButtonEx.h"

/////////////////////////////////////////////////////////////////////////////
// CButtonNotify window

class CONTROL_EXT CButtonEx2 : public CButtonEx
{
	DECLARE_DYNAMIC(CButtonEx2);

// Construction
public:
	CButtonEx2();

// Attributes
public:
	BOOL m_bLButtonDown;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CButtonEx2)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CButtonEx2();
	BOOL Enable(BOOL bEnable, BOOL bNormalLook = FALSE)
	{
		BOOL bRedraw = ( (m_bEnable != bEnable) || (m_bNormalLook != bNormalLook));
		m_bNormalLook = bNormalLook;
		m_bEnable = bEnable;
		if(bRedraw) 
			Invalidate(FALSE);

		return CWnd::EnableWindow(TRUE);
	}
	// Generated message map functions
protected:
	//{{AFX_MSG(CButtonEx2)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnClicked();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUTTONEX2_H__30D01DA6_7316_475B_85BE_EE8EEEAFFF44__INCLUDED_)
