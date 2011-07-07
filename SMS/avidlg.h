#if !defined(AFX_AVIDLG_H__04E51939_DDF6_4DB4_9542_1E5359E43B63__INCLUDED_)
#define AFX_AVIDLG_H__04E51939_DDF6_4DB4_9542_1E5359E43B63__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AviDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAviDlg dialog

class CAviDlg : public CDialog
{
// Construction
public:
	CAviDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAviDlg)
	enum { IDD = IDD_DLG_AVI };
	CAnimateCtrl	m_AniCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAviDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	COLORREF	m_crBg;

	// Generated message map functions
	//{{AFX_MSG(CAviDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AVIDLG_H__04E51939_DDF6_4DB4_9542_1E5359E43B63__INCLUDED_)
