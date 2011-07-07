#if !defined(AFX_SONYERISONTDLG_H__D2C9F5A3_927D_4385_A3F9_2B9705A6ABC9__INCLUDED_)
#define AFX_SONYERISONTDLG_H__D2C9F5A3_927D_4385_A3F9_2B9705A6ABC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// sonyerisontdlg.h : header file
//
#include "..\include\myedit.h"
#include "BaseDlg.h"


/////////////////////////////////////////////////////////////////////////////
// CSonyEricssonTDlg dialog

class CSonyEricssonTDlg : public CBaseDlg
{
// Construction
public:
	CSonyEricssonTDlg(CWnd* pParent = NULL);   // standard constructor
	~CSonyEricssonTDlg();
	HFONT	  m_hFont;
	TCHAR m_szFirstName[MAX_PATH];
	TCHAR m_szName[MAX_PATH];

// Dialog Data
	//{{AFX_DATA(CSonyEricssonTDlg)
	enum { IDD = IDD_DLG_SE_T68 };
	CRescaleButton	m_rbnOk;
	CRescaleButton	m_rbnCancel;
	CStaticEx	m_scWork;
	CStaticEx	m_scTitle;
	CStaticEx	m_scPosition;
	CStaticEx	m_scOther;
	CStaticEx	m_scMobile;
	CStaticEx	m_scMail;
	CStaticEx	m_scLastName;
	CStaticEx	m_scHomeTel;
	CStaticEx	m_scFirstName;
	CStaticEx	m_scFax;
	CStaticEx	m_scCompany;
	CMyEdit	m_edWork;
	CEdit	m_edTitle;
	CMyEdit	m_edOther;
	CMyEdit	m_edMobile;
	CEdit	m_edMail;
	CEdit	m_edLastName;
	CMyEdit	m_edHome;
	CEdit	m_edFirstName;
	CMyEdit	m_edFax;
	CEdit	m_edCompany;
	CRadioEx	m_rdSIM;
	CRadioEx	m_rdME;
	CStatic	m_scLine;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSonyEricssonTDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	xEnableControl(int iState);
	xSetDataToDlgCtrl();
	int m_iState;	//0: T68,T39 1:T610,T630,Z600
	// Generated message map functions
	//{{AFX_MSG(CSonyEricssonTDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual afx_msg void OnRdMobile();
	virtual afx_msg void OnRdSim();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SONYERISONTDLG_H__D2C9F5A3_927D_4385_A3F9_2B9705A6ABC9__INCLUDED_)
