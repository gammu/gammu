#if !defined(AFX_CSamsungZDlg_H__31F0EA41_7A90_4831_947D_67B1CBCC1159__INCLUDED_)
#define AFX_CSamsungZDlg_H__31F0EA41_7A90_4831_947D_67B1CBCC1159__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SharpT300Dlg.h : header file
//
#include "..\include\myedit.h"
#include "BaseDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSamsungZDlg dialog

class CSamsungZDlg : public CBaseDlg
{
// Construction
public:
	CSamsungZDlg(CWnd* pParent = NULL);   // standard constructor
	~CSamsungZDlg();
	HFONT	  m_hFont;
	TCHAR m_szLastName[MAX_PATH];
	TCHAR m_szName[MAX_PATH];

// Dialog Data
	//{{AFX_DATA(CSamsungZDlg)
	enum { IDD = IDD_SAMSUNGZDLG };
	CStaticEx	m_scDep;
	CMyEdit	m_edDep;
	CMyEdit	m_edURL;
	CStaticEx	m_scURL;
	CMyEdit	m_edJobTitle;
	CStaticEx	m_scJobTitle;
	CMyEdit	m_edNickName;
	CStaticEx	m_scNickName;
	CStaticEx	m_scCompany;
	CMyEdit	m_edCompany;
	CMyEdit	m_edLastName;
	CMyEdit	m_edFirstName;
	CStaticEx	m_scLastName;
	CStaticEx	m_scFirstName;
	CRescaleButton	m_rbnOK;
	CRescaleButton	m_rbnCancel;
	CStaticEx	m_scDot5;
	CStaticEx	m_scDot4;
	CStaticEx	m_scDot3;
	CStaticEx	m_scDot2;
	CStaticEx	m_scDot1;
	CStatic		m_scLine;
	CStaticEx	m_scStorage;
	CStaticEx	m_scPhone5;
	CStaticEx	m_scPhone4;
	CStaticEx	m_scPhone3;
	CStaticEx	m_scPhone2;
	CStaticEx	m_scPhone1;
	CStaticEx	m_scNote;
	CStaticEx	m_scEmail2;
	CStaticEx	m_scEmail1;
	CComboBox	m_cbPhone5;
	CComboBox	m_cbPhone4;
	CComboBox	m_cbPhone3;
	CComboBox	m_cbPhone2;
	CComboBox	m_cbPhone1;
	CMyEdit	m_edNote;
	CMyEdit	m_edPhone5;
	CMyEdit	m_edPhone4;
	CMyEdit	m_edPhone3;
	CMyEdit	m_edPhone2;
	CMyEdit	m_edPhone1;
	CMyEdit	m_edEmail2;
	CMyEdit	m_edEmail1;
	CRadioEx	m_rdSIM;
	CRadioEx	m_rdME;
	CDateTimeCtrl	m_dpBirthday;
	CCheckEx2	m_chBirthday;
	CStaticEx	m_scBirthday;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSamsungZDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void xSetComboItem(LPCTSTR szProfile);
	void xSetDefaultControl();
	void xSetDataToDlgCtrl();
	void xEnableControl(int iState);
	void xSetSupportColumnLength();

	// Generated message map functions
	//{{AFX_MSG(CSamsungZDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRdMobile();
	afx_msg void OnRdSim();
	afx_msg void OnChBirthday();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CSamsungZDlg_H__31F0EA41_7A90_4831_947D_67B1CBCC1159__INCLUDED_)
