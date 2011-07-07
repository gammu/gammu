#if !defined(AFX_OTHERDLG_H__390F3E56_598F_4FC1_860D_92BE8C1F3F36__INCLUDED_)
#define AFX_OTHERDLG_H__390F3E56_598F_4FC1_860D_92BE8C1F3F36__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OtherDlg.h : header file
//
#include "..\include\myedit.h"
#include "BaseDlg.h"

/////////////////////////////////////////////////////////////////////////////
// COtherDlg dialog
typedef struct
{
	GSM_EntryType nEntryType;
	CPtrList pControlList;
	bool bPhoneNo;
}MYCONTROLINFO;
class COtherDlg : public CBaseDlg
{
// Construction
public:
	COtherDlg(CWnd* pParent = NULL);   // standard constructor
	~COtherDlg();
	HFONT	  m_hFont;
	CWnd*   m_pFirstPhoneNoWnd;
	GSM_EntryType GetEntryTypeID(TCHAR* sz);
	TCHAR m_szFirstName[MAX_PATH];
	TCHAR m_szName[MAX_PATH];
// Dialog Data
	//{{AFX_DATA(COtherDlg)
	enum { IDD = IDD_DLG_OTHER };
	CEdit	m_edHidePostal;
	CRescaleButton	m_rbnOk;
	CRescaleButton	m_rbnCancel;
	CStaticEx	m_scText9;
	CStaticEx	m_scText8;
	CStaticEx	m_scText7;
	CStaticEx	m_scText6;
	CStaticEx	m_scText5;
	CStaticEx	m_scText4;
	CStaticEx	m_scText3;
	CStaticEx	m_scText2;
	CStaticEx	m_scText10;
	CStaticEx	m_scText11;
	CStaticEx	m_scText12;
	CStaticEx	m_scText13;
	CStaticEx	m_scText14;
	CStaticEx	m_scText15;
	CStaticEx	m_scText1;
	CStaticEx	m_scPosition;
	CStaticEx	m_scPhoneNo9;
	CStaticEx	m_scPhoneNo8;
	CStaticEx	m_scPhoneNo7;
	CStaticEx	m_scPhoneNo6;
	CStaticEx	m_scPhoneNo5;
	CStaticEx	m_scPhoneNo4;
	CStaticEx	m_scPhoneNo3;
	CStaticEx	m_scPhoneNo2;
	CStaticEx	m_scPhoneNo10;
	CStaticEx	m_scPhoneNo1;
	CStaticEx	m_scName;
	CStatic	m_scLine;
	CStaticEx	m_scLastName;
	CStaticEx	m_scGroup;
	CStaticEx	m_scFirstName;
	CStaticEx	m_scBirthday;
	CEdit	m_edText9;
	CEdit	m_edText8;
	CEdit	m_edText7;
	CEdit	m_edText6;
	CEdit	m_edText5;
	CEdit	m_edText4;
	CEdit	m_edText3;
	CEdit	m_edText2;
	CEdit	m_edText10;
	CEdit	m_edText11;
	CEdit	m_edText12;
	CEdit	m_edText13;
	CEdit	m_edText14;
	CEdit	m_edText15;
	CEdit	m_edText1;
	CMyEdit	m_edPhoneNo9;
	CMyEdit	m_edPhoneNo8;
	CMyEdit	m_edPhoneNo7;
	CMyEdit	m_edPhoneNo6;
	CMyEdit	m_edPhoneNo5;
	CMyEdit	m_edPhoneNo4;
	CMyEdit	m_edPhoneNo3;
	CMyEdit	m_edPhoneNo2;
	CMyEdit	m_edPhoneNo10;
	CMyEdit	m_edPhoneNo1;
	CEdit	m_edName;
	CEdit	m_edLastName;
	CEdit	m_edFirstName;
	CDateTimeCtrl	m_dpBirthday;
	CCheckEx2	m_chBirthday;
	CComboBox	m_cbGroup;
	CRadioEx	m_rdSIM;
	CRadioEx	m_rdME;
	//}}AFX_DATA

	CPtrList m_CtrlInfolist;
	CEdit *m_pName_infirstPos;
	CStaticEx	*m_pscNamefirstPos;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COtherDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void xSetDataToDlgCtrl();
	void xEnableControl(int iState);
	void xSetSupportColumnLength();
	void xSetComboItem(LPCTSTR szProfile);

	// Generated message map functions
	//{{AFX_MSG(COtherDlg)
	virtual BOOL OnInitDialog();
	virtual void OnRdSim();
	virtual void OnRdMobile();
	afx_msg void OnChBirthday();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OTHERDLG_H__390F3E56_598F_4FC1_860D_92BE8C1F3F36__INCLUDED_)
