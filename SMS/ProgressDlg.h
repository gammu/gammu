#if !defined(AFX_PROGRESSDLG_H__6EEB7898_6B0F_4CAF_B081_3F73A34EA483__INCLUDED_)
#define AFX_PROGRESSDLG_H__6EEB7898_6B0F_4CAF_B081_3F73A34EA483__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressDlg.h : header file
//
#include "Resource.h"
#include "ContactsDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog

class CProgressDlg : public CDialog
{
// Construction
public:
	CProgressDlg(CWnd* pParent = NULL);   // standard constructor
	CWinThread* m_pGetThread;
	HBRUSH m_hBrush;
	CContactsDlg* m_pContactsDlg;
	SMS_Tal_Num* m_pSMSStatus;
	int m_nProgressType;
	void SetDataToDlg(CContactsDlg* pdlg)
	{
		m_pContactsDlg = pdlg;
		m_nProgressType = 1;
	}
	void SetDataToDlg(SMS_Tal_Num* pData)
	{
		m_pSMSStatus = pData;
		m_nProgressType = 2;
	}
	bool OpenAvi();
//	int			GetPBOneData(char *strMemType, int nMemtype, int nPos, bool bStart);
// Dialog Data
	//{{AFX_DATA(CProgressDlg)
	enum { IDD = IDD_AVI_DLG };
//	CStaticEx	m_staticText;
	CRescaleButton	m_CancelButton;
//	CProgressCtrlEx	m_ProgressCtrl;
	CAnimateCtrl m_AnCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProgressDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSDLG_H__6EEB7898_6B0F_4CAF_B081_3F73A34EA483__INCLUDED_)
