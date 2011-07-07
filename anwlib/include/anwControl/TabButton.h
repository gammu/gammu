// TabButton.h: interface for the CTabButtonEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TABBUTTON_H__19EEF2C7_065D_4581_A90A_FD3ED365BCE3__INCLUDED_)
#define AFX_TABBUTTON_H__19EEF2C7_065D_4581_A90A_FD3ED365BCE3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "rescalebutton.h"

class CONTROL_EXT CTabButtonEx : public CRescaleButton  
{
public:
	CTabButtonEx();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabButtonEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTabButtonEx();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRescaleButton)
	protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CTabButtonEx)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

#endif // !defined(AFX_TABBUTTON_H__19EEF2C7_065D_4581_A90A_FD3ED365BCE3__INCLUDED_)
