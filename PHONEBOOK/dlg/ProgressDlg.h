#if !defined(AFX_PROGRESSDLG_H__6EEB7898_6B0F_4CAF_B081_3F73A34EA483__INCLUDED_)
#define AFX_PROGRESSDLG_H__6EEB7898_6B0F_4CAF_B081_3F73A34EA483__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog

class CProgressDlg : public CDialog
{
// Construction
public:
	CProgressDlg(CWnd* pParent = NULL);   // standard constructor
	COLORREF	m_crBg;
	CWinThread* m_pThread;
	LPDISPATCH m_dispItem ;
	CPtrList* m_pDataList;
	Tstring		m_sProfile;
	CBrush		m_brBar;
	FileData *m_pFolderInfoData;
	DATALIST* m_pAddDataList;
	int m_nType;
	void SeGetDataToDlg(FileData *pData,CPtrList* pDataList)
	{
		m_pFolderInfoData = pData;
		m_nType = 0;
		
		m_pDataList = pDataList;

	}
	void SetAddDataToDlg(FileData *pData,DATALIST *pAddDataList,CPtrList* pOTDataList)
	{
		m_pFolderInfoData = pData;
		m_nType = 1;
		
		m_pAddDataList = pAddDataList;
		m_pDataList = pOTDataList;

	}
	static int ProgressCallback( int nProgress );
//	int			GetPBOneData(char *strMemType, int nMemtype, int nPos, bool bStart);
// Dialog Data
	//{{AFX_DATA(CProgressDlg)
	enum { IDD = IDD_DLG_PROG };
//	CStaticEx	m_staticText;
	CStaticEx	m_scText;
	CProgressCtrlEx		m_Prog;
	CRescaleButton		m_rbnCancel;
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
public:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESSDLG_H__6EEB7898_6B0F_4CAF_B081_3F73A34EA483__INCLUDED_)
