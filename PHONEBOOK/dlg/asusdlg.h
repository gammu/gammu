#if !defined(AFX_ASUSDLG_H__43AB1130_FAF4_4348_A84C_8A2A5B1FB947__INCLUDED_)
#define AFX_ASUSDLG_H__43AB1130_FAF4_4348_A84C_8A2A5B1FB947__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// asusdlg.h : header file
//
#include "..\include\myedit.h"
#include "BaseDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CASUSDlg dialog

class CASUSDlg : public CBaseDlg
{
// Construction
public:
	CASUSDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CASUSDlg)
	enum { IDD = IDD_DLG_ASUS };
	CDateTimeCtrl	m_dpBirthday;
	CRadioEx m_rdSIM;
	CRadioEx m_rdME;
	CRescaleButton	m_rbnOk;
	CRescaleButton	m_rbnCancel;
	CStaticEx	m_scTitle;
	CStaticEx	m_scTel4;
	CStaticEx	m_scTel3;
	CStaticEx	m_scTel2;
	CStaticEx	m_scTel1;
	CStaticEx	m_scStorage;
	CStaticEx	m_scSex;
	CStaticEx	m_scName;
	CStaticEx	m_scMemo;
	CStaticEx	m_scMail;
	CStaticEx	m_scLocation;
	CStaticEx	m_scLocalView;
	CStaticEx	m_scLine;
	CStaticEx	m_scLED;
	CStaticEx	m_scGrouop;
	CStaticEx	m_scFax;
	CStaticEx	m_scCompany;
	CStaticEx	m_scBirthday;
	CStaticEx	m_scAddress;
	CEdit	m_edMemo;
	CEdit	m_edTitle;
	CMyEdit	m_edTel4;
	CMyEdit	m_edTel3;
	CMyEdit	m_edTel2;
	CMyEdit	m_edTel1;
	CEdit	m_edName;
	CEdit	m_edMail;
	CMyEdit	m_edFax;
	CEdit	m_edCompany;
	CEdit	m_edAddress;
	CComboBox	m_cbSex;
	CComboBox	m_cbLED;
	CComboBox	m_cbGroup;
	CCheckEx2	m_chBirthday;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CASUSDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void xSetDataToDlgCtrl();
	void xEnableControl(int iState);

protected:
	void xSetSupportColumnLength();
	int xGetLEDItem(int iType);
	int xGetSexItem(int iType);
	int xGetLEDType(int iItem);
	int xGetSexType(int iItem);
	void xSetComboItem();
	// Generated message map functions
	//{{AFX_MSG(CASUSDlg)
	virtual BOOL OnInitDialog();
	virtual afx_msg void OnRdMobile();
	virtual afx_msg void OnRdSim();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnChBirthday();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASUSDLG_H__43AB1130_FAF4_4348_A84C_8A2A5B1FB947__INCLUDED_)
