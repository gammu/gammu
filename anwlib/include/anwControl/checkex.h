#if !defined(AFX_CHECKEX_H__18D017C0_D2EC_408E_9E56_EFD62A70CE01__INCLUDED_)
#define AFX_CHECKEX_H__18D017C0_D2EC_408E_9E56_EFD62A70CE01__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CheckEx.h : header file
//
#include "ButtonEx.h"

/////////////////////////////////////////////////////////////////////////////
// CCheckEx window

class CONTROL_EXT CCheckEx : public CButtonEx
{
	DECLARE_DYNAMIC(CCheckEx);

// Construction
public:
	CCheckEx();

// Attributes
public:
	BOOL m_bLButtonDown;
// Operations
public:
	int GetCheck() { return m_bChecked; }
	void SetCheck(BOOL check = TRUE)
	{
		BOOL bRedraw = (m_bChecked != check);
		m_bChecked = check;
		if(bRedraw) Invalidate(FALSE);
	}
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCheckEx)
	protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCheckEx();
	virtual BOOL LoadBitmap(LPCTSTR pszFileName);
	virtual BOOL LoadBitmap(CImageArray* pImage, BOOL bSafeDel = TRUE);

	// Generated message map functions
protected:
	//{{AFX_MSG(CCheckEx)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHECKEX_H__18D017C0_D2EC_408E_9E56_EFD62A70CE01__INCLUDED_)
