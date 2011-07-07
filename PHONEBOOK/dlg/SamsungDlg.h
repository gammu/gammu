#if !defined(AFX_SAMSUNGDLG_H__D2C9F5A3_927D_4385_A3F9_2B9705A6ABC9__INCLUDED_)
#define AFX_SAMSUNGDLG_H__D2C9F5A3_927D_4385_A3F9_2B9705A6ABC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\include\myedit.h"
#include "BaseDlg.h"


/////////////////////////////////////////////////////////////////////////////
// CSamsungDlg dialog

class CSamsungDlg : public CBaseDlg
{
// Construction
public:
	CSamsungDlg(CWnd* pParent = NULL);   // standard constructor
	~CSamsungDlg();
	HFONT	  m_hFont;
	int m_nFistNameBoundary;
	int m_nLastNameBoundary;

	TCHAR m_szFirstName[MAX_PATH];
	TCHAR m_szName[MAX_PATH];

// Dialog Data
	//{{AFX_DATA(CSamsungDlg)
	enum { IDD = IDD_DLG_SG };
	CRescaleButton	m_rbnOk;
	CRescaleButton	m_rbnCancel;
	CStaticEx	m_scWork;
	CStaticEx	m_scOther;
	CStaticEx	m_scMobile;
	CStaticEx	m_scMail;
	CStaticEx	m_scLastName;
	CStaticEx	m_scHomeTel;
	CStaticEx	m_scFirstName;
	CStaticEx	m_scFax;
	CStaticEx	m_scPosition;
	CStaticEx	m_scGroup;
	CComboBox	m_cbGroup;
	CStaticEx	m_scMemo;
	CStaticEx   m_scPager;
	CStaticEx   m_scPostCode;
	CStaticEx   m_scNotes;
	CMyEdit     m_edPager;
	CMyEdit     m_edPostCode;
	CEdit       m_edNotes;
	CEdit	m_edMemo;
	CMyEdit	m_edWork;
	CMyEdit	m_edOther;
	CMyEdit	m_edMobile;
	CEdit	m_edMail;
	CEdit	m_edLastName;
	CMyEdit	m_edHome;
	CEdit	m_edFirstName;
	CMyEdit	m_edFax;
	CRadioEx	m_rdSIM;
	CRadioEx	m_rdME;
	CStatic	m_scLine;
	CStaticEx	m_scBirthday;
	CDateTimeCtrl	m_dpBirthday;
	CCheckEx2	m_chBirthday;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSamsungDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL xCheckData();
	xEnableControl(int iState);
	xSetDataToDlgCtrl();
	void xSetSupportColumnLength();
	int m_iState;	//0: D500
	// Generated message map functions
	//{{AFX_MSG(CSamsungDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual void OnRdMobile();
	virtual void OnRdSim();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEdFirstName();
	afx_msg void OnChangeEdLastName();
	afx_msg void OnChBirthday();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAMSUNGDLG_H__D2C9F5A3_927D_4385_A3F9_2B9705A6ABC9__INCLUDED_)
