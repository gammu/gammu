#if !defined(AFX_PROCESSDLG_H__DC78B4A1_D576_43B9_98F8_3BD69D4ED862__INCLUDED_)
#define AFX_PROCESSDLG_H__DC78B4A1_D576_43B9_98F8_3BD69D4ED862__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcessDlg.h : header file
//
#include "Worker.h"
#include "Resource.h"
#include "ContactsDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CProcessDlg dialog
typedef int (* DownloadUtilityProc)(int);
class CProcessDlg : public CDialog
{
// Construction
public:
	~CProcessDlg();
	void SetLoopCount( int nLoop ) { m_nTotal = nLoop ; }
	void OffsetCount( void ) { m_nStep++; }
	CProcessDlg(CWnd* pParent = NULL);   // standard constructor
	CWorker	m_Worker;
	HBRUSH    m_hBrush;
	void SetProgressInfo( int (*DownloadUtility)(int State), int nTotal ){ 
		m_pDownloadUtilityfn = DownloadUtility;
		m_nTotal = nTotal;
	}
	int m_nTotal;
	int m_nStep;
	CBrush		m_brBar;
	DownloadUtilityProc m_pDownloadUtilityfn;

// Dialog Data
	//{{AFX_DATA(CProcessDlg)
	enum { IDD = IDD_PROCESS_DLG };
	CStaticEx		m_ReadMsg;
	CRescaleButton	m_btnStop;
	CProgressCtrlEx	m_progressBar;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcessDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProcessDlg)
	afx_msg void OnStop();
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnStart();
	afx_msg void OnOk();
	afx_msg void OnClose();
	afx_msg void OnCancelMode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCESSDLG_H__DC78B4A1_D576_43B9_98F8_3BD69D4ED862__INCLUDED_)
