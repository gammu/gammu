#if !defined(AFX_SEARCHDLG_H__874BF1CC_7289_4D5E_94FB_C6CADE94472F__INCLUDED_)
#define AFX_SEARCHDLG_H__874BF1CC_7289_4D5E_94FB_C6CADE94472F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SearchDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSearchDlg dialog

class CSearchDlg : public CDialog
{
// Construction
public:
	CSearchDlg(CWnd* pParent = NULL);   // standard constructor
	~CSearchDlg();
	HFONT	  m_hFont;
	
	void SetSearchStrVec(deque<Tstring> *pdeq){m_pdeqSearchStr = pdeq;}
// Dialog Data
	//{{AFX_DATA(CSearchDlg)
//	enum { IDD = IDD_SEARCH };
	enum { IDD = 0};
	CRescaleButton	m_rbnOk;
	CRescaleButton	m_rbnFindNext;
	CComboBox		m_cbSearch;
	CStaticEx		m_scTitle;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSearchDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	COLORREF	m_crBg;
	deque<Tstring>	*m_pdeqSearchStr;
	CWnd *m_pParent;
	Tstring m_sOldTextl;
	int		m_iStart;
	// Generated message map functions
	//{{AFX_MSG(CSearchDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBnFindnext();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEARCHDLG_H__874BF1CC_7289_4D5E_94FB_C6CADE94472F__INCLUDED_)
