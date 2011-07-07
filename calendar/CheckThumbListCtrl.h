#if !defined(AFX_CHECKTHUMBLISTCTRL_H__7CE21585_0FD3_48DF_8BF1_79C4B0D3F23C__INCLUDED_)
#define AFX_CHECKTHUMBLISTCTRL_H__7CE21585_0FD3_48DF_8BF1_79C4B0D3F23C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CheckThumbListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCheckThumbListCtrl window

class CCheckThumbListCtrl : public CListCtrl
{
// Construction
public:
	CCheckThumbListCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCheckThumbListCtrl)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCheckThumbListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CCheckThumbListCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHECKTHUMBLISTCTRL_H__7CE21585_0FD3_48DF_8BF1_79C4B0D3F23C__INCLUDED_)
