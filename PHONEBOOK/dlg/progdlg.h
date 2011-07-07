#if !defined(AFX_PROGDLG_H__96DF7435_ADB3_4D40_8760_EDE658388A77__INCLUDED_)
#define AFX_PROGDLG_H__96DF7435_ADB3_4D40_8760_EDE658388A77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProgDlg dialog
enum{
	PROG_TRANSFER = 0,
	PROG_DEL = 1,
	PROG_INIT = 2,
	PROG_TRAN_ME,
	PROG_TRAN_SM,
	PROG_SENDSMS
};

class CProgDlg : public CDialog
{
// Construction
public:
	void ResetCtrlView();
	bool OpenAvi();
	bool CloseAvi();
	bool StopAvi();
	bool PlayAvi();

	void SetTitle(int iTitle);
	void SetPos(int iPos);
	void SetRange(int iLow,int iUp);
	CProgDlg(CWnd* pParent = NULL);   // standard constructor
	void SetParent(CWnd *pParent){m_pParent = pParent;}
// Dialog Data
	//{{AFX_DATA(CProgDlg)
	enum { IDD = IDD_DLG_PROG };
	CStaticEx	m_scText;
	CProgressCtrlEx		m_Prog;
	CRescaleButton		m_rbnCancel;
	CAnimateCtrl m_AnCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	COLORREF	m_crBg;
	CBrush		m_brBar;
	CWnd*		m_pParent;
	Tstring		m_sProfile;
	// Generated message map functions
	//{{AFX_MSG(CProgDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGDLG_H__96DF7435_ADB3_4D40_8760_EDE658388A77__INCLUDED_)
