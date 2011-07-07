#if !defined(AFX_AVIDLG_H__7205D995_B269_46F4_87CD_5891B636DCF3__INCLUDED_)
#define AFX_AVIDLG_H__7205D995_B269_46F4_87CD_5891B636DCF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// avidlg.h : header file
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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AVIDLG_H__7205D995_B269_46F4_87CD_5891B636DCF3__INCLUDED_)
