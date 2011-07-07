#if !defined(AFX_TOPBARDLG_H__F5D09A58_AE95_47AE_99AE_2370C1B2816F__INCLUDED_)
#define AFX_TOPBARDLG_H__F5D09A58_AE95_47AE_99AE_2370C1B2816F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TopBarDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTopBarDlg dialog

class CTopBarDlg : public CDialog
{
// Construction
public:
	CTopBarDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTopBarDlg)
	enum { IDD = IDD_DLG_TOPBAR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTopBarDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTopBarDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOPBARDLG_H__F5D09A58_AE95_47AE_99AE_2370C1B2816F__INCLUDED_)
