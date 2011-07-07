#if !defined(AFX_PROCESSDLG_H__DC78B4A1_D576_43B9_98F8_3BD69D4ED862__INCLUDED_)
#define AFX_PROCESSDLG_H__DC78B4A1_D576_43B9_98F8_3BD69D4ED862__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcessDlg.h : header file
//
#include "schedule\cammschedulelist.h"
#include "Worker.h"
#include "Resource.h"
//#include "ContactsDlg.h"
#include "MobileDevice\CAMMSync.h"
enum{
	PROG_MOBILE_GET = 0,
	PROG_MOBILE_DEL = 1,
	PROG_MOBILE_ADD = 2,
	PROG_MSOT_GET = 3,
	PROG_MSOT_ADD = 4,

};

/////////////////////////////////////////////////////////////////////////////
// CProcessDlg dialog
typedef int (* DownloadUtilityProc)(int);
class CProcessDlg : public CDialog
{
// Construction
public:
	void SetDeleteInfo(CCAMMScheduleList* pSchduleListView);
	~CProcessDlg();
	CProcessDlg(CWnd* pParent = NULL);   // standard constructor
	HBRUSH    m_hBrush;
	CWinThread* m_pThread;
	FileData *m_pFolderInfoData;
//	CWinThread* m_pGetCalendarThread;
//	CWinThread* m_pDeleteCalendarThread;
	void SetGetMobileDataInfo(CCAMMSync* pCAMMSync,CPtrList* pCalendardataList);
	void SetGetMSOTDataInfo(FileData *pData,CCAMMSync* pCAMMSync,CPtrList* pCalendardataList)
	{
		m_nProgressType = PROG_MSOT_GET;
		m_pCalendarDataWrapper = pCAMMSync;
		m_pCalendarDataList = pCalendardataList;
		m_pFolderInfoData = pData;
	}
	void SetAddMSOTDataInfo(FileData *pData,CCAMMSync* pCAMMSync,CPtrList* pCalendardataList)
	{
		m_nProgressType = PROG_MSOT_ADD;
		m_pCalendarDataWrapper = pCAMMSync;
		m_pCalendarDataList = pCalendardataList;
		m_pFolderInfoData = pData;
	}
	void SetAddMobileDataInfo(CPtrList* pMobileDataList , CCAMMSync* pCAMMSync,CPtrList* pCalendardataList)
	{
		m_nProgressType = PROG_MOBILE_ADD;
		m_pCalendarDataWrapper = pCAMMSync;
		m_pCalendarDataList = pCalendardataList;
		m_pMobileDataList = pMobileDataList;
	}

//	int m_nTotalGetCount;
//	SchedulesRecord *m_pScheduleRecord;
	CCAMMSync* m_pCalendarDataWrapper;

	void SetTitle(int iTitle);
	int m_nGetCount;
	int m_nProgressType ; 
	CCAMMScheduleList * m_pSchduleListView;
	CPtrList *m_pCalendarDataList;
	CPtrList *m_pMobileDataList;
	bool OpenAvi();
	bool CloseAvi();
	bool StopAvi();
	bool PlayAvi();
	static int ProgressCallback( int nProgress );

// Dialog Data
	//{{AFX_DATA(CProcessDlg)
	enum { IDD = IDD_DLG_PROG };
	CAnimateCtrl	m_AnCtrl;
	CRescaleButton	m_btnStop;
	CProgressCtrlEx	m_progressBar;
	CStaticEx         m_progressMessage;
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
	afx_msg void OnOk();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCESSDLG_H__DC78B4A1_D576_43B9_98F8_3BD69D4ED862__INCLUDED_)
