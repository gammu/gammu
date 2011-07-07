#if !defined(AFX_RADIOEX_H__18D017C0_D2EC_408E_9E56_EFD62A70CE01__INCLUDED_)
#define AFX_RADIOEX_H__18D017C0_D2EC_408E_9E56_EFD62A70CE01__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RadioEx.h : header file
//
#include "checkex.h"

/////////////////////////////////////////////////////////////////////////////
// CRadioEx window

class CONTROL_EXT CRadioEx : public CCheckEx
{
// Construction
public:
	CRadioEx();

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
	//{{AFX_VIRTUAL(CRadioEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRadioEx();

	// Generated message map functions
protected:
	//{{AFX_MSG(CRadioEx)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RADIOEX_H__18D017C0_D2EC_408E_9E56_EFD62A70CE01__INCLUDED_)
