#if !defined(AFX_TOPVIEW_H__B3B76FC7_F90A_4885_960E_88DD729BA228__INCLUDED_)
#define AFX_TOPVIEW_H__B3B76FC7_F90A_4885_960E_88DD729BA228__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TopView.h : header file
//
#include "TopBarDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CTopView view

class CTopView : public CView
{
protected:
	CTopView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTopView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTopView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
public:
	CTopBarDlg m_topBarDlg;

protected:
	virtual ~CTopView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CTopView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOPVIEW_H__B3B76FC7_F90A_4885_960E_88DD729BA228__INCLUDED_)
