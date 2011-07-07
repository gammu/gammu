#if !defined(AFX_CHECKEX2_H__EEF7BA21_9632_45C2_A8F3_9648D51C60E6__INCLUDED_)
#define AFX_CHECKEX2_H__EEF7BA21_9632_45C2_A8F3_9648D51C60E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CheckEx2.h : header file
//
#include "CheckEx.h"

/////////////////////////////////////////////////////////////////////////////
// CCheckEx2 window

class CONTROL_EXT CCheckEx2 : public CCheckEx
{
	DECLARE_DYNAMIC(CCheckEx2);

// Construction
public:
	CCheckEx2();

// Attributes
public:

// Operations
public:
	int GetCheck() { return m_bChecked; }
	void SetCheck(BOOL check = TRUE) 
	{ 
		m_bChecked = check;
		CButton::SetCheck(check);
		Invalidate(FALSE);
	}
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCheckEx2)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCheckEx2();

	// Generated message map functions
protected:
	//{{AFX_MSG(CCheckEx2)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHECKEX2_H__EEF7BA21_9632_45C2_A8F3_9648D51C60E6__INCLUDED_)
