#if !defined(AFX_DATADYCDLG_H__2CA63119_1BC2_470E_9FAE_4B066D8C11B7__INCLUDED_)
#define AFX_DATADYCDLG_H__2CA63119_1BC2_470E_9FAE_4B066D8C11B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DataDycDlg.h : header file
//
#include "..\include\myedit.h"
#include "BaseDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CDataDycDlg dialog

class CDataDycDlg : public CBaseDlg
{
// Construction
public:
	HFONT	  m_hFont;
	CDataDycDlg(CWnd* pParent = NULL);   // standard constructor
	~CDataDycDlg();

// Dialog Data
	//{{AFX_DATA(CDataDycDlg)
	enum { IDD = IDD_DATA_DYC };
	CStaticEx	m_scGroup;
	CComboBox	m_cbGroup;
	CStaticEx	m_scDot11;
	CRescaleButton	m_rbnOK;
	CRescaleButton	m_rbnCancel;
	CStaticEx	m_scTitle;
	CStaticEx	m_scStorage;
	CStaticEx	m_scNum9;
	CStaticEx	m_scNum8;
	CStaticEx	m_scNum7;
	CStaticEx	m_scNum6;
	CStaticEx	m_scNum5;
	CStaticEx	m_scNum4;
	CStaticEx	m_scNum3;
	CStaticEx	m_scNum2;
	CStaticEx	m_scNum1;
	CStaticEx	m_scName;
	CStaticEx	m_scDot9;
	CStaticEx	m_scDot8;
	CStaticEx	m_scDot7;
	CStaticEx	m_scDot6;
	CStaticEx	m_scDot5;
	CStaticEx	m_scDot4;
	CStaticEx	m_scDot3;
	CStaticEx	m_scDot2;
	CStaticEx	m_scDot10;
	CStaticEx	m_scDot1;
	CStaticEx	m_scDefault;
	CMyEdit	m_ed9;
	CMyEdit	m_ed8;
	CMyEdit	m_ed7;
	CMyEdit	m_ed6;
	CMyEdit	m_ed5;
	CMyEdit	m_ed4;
	CMyEdit	m_ed3;
	CMyEdit	m_ed2;
	CMyEdit	m_edName;
	CComboBox	m_cb9;
	CComboBox	m_cb8;
	CComboBox	m_cb7;
	CComboBox	m_cb6;
	CComboBox	m_cb5;
	CComboBox	m_cb4;
	CComboBox	m_cb3;
	CComboBox	m_cb2;
	CComboBox	m_cbDefault;
	CCheckEx2	m_ch9;
	CCheckEx2	m_ch8;
	CCheckEx2	m_ch7;
	CCheckEx2	m_ch6;
	CCheckEx2	m_ch5;
	CCheckEx2	m_ch4;
	CCheckEx2	m_ch3;
	CRadioEx	m_rdSIM;
	CRadioEx	m_rdME;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataDycDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//enable control row 0-8
	void xEnableControl(int iIndex,BOOL bFlag);
	//set default combobox string
	void xSetDefaultCombo();
	//set control enable 
	void xSetDefaultEnableControl();
	//set data to control
	void xSetDataToDlgCtrl();
	//set default control status
	void xSetDefaultControl();
	//get the unused control
	void xGetUnusedControl(int iIndex,CCheckEx **pCh,CComboBox **pCb,CMyEdit **pEdit);
	//
	void xSetComboItem();
	int xGetComboSel(int iIndex);
	void xSetSupportColumnLength();
	void xSetCombo2Item();
	
protected:
/*	COLORREF	m_crBg;
	int m_iMode;	// set dlg mode
	int m_iIsME;	//set default storage type
	CCardPanel	*m_pData;
	//a member variable to store 
	CSupportEntry m_se;*/
	//app only support 5-3,so other data must be stored
	DEQPHONEDATA m_deqDetainData;
	// Generated message map functions
	//{{AFX_MSG(CDataDycDlg)
	afx_msg void OnCheck3();
	afx_msg void OnCheck4();
	afx_msg void OnCheck5();
	afx_msg void OnCheck6();
	afx_msg void OnCheck7();
	afx_msg void OnCheck8();
	afx_msg void OnCheck9();
	virtual void OnCancel();
	virtual void OnOK();
	virtual afx_msg void OnRdSim();
	virtual afx_msg void OnRdMobile();
	afx_msg void OnSelchangeCombo2();
	afx_msg void OnSelchangeCombo3();
	afx_msg void OnSelchangeCombo4();
	afx_msg void OnSelchangeCombo5();
	afx_msg void OnSelchangeCombo6();
	afx_msg void OnSelchangeCombo7();
	afx_msg void OnSelchangeCombo8();
	afx_msg void OnSelchangeCombo9();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATADYCDLG_H__2CA63119_1BC2_470E_9FAE_4B066D8C11B7__INCLUDED_)
