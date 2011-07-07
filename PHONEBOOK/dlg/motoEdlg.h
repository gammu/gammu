#if !defined(AFX_MOTOEDLG_H__8755F190_4E37_4C90_B1C1_70AE66DB1CAE__INCLUDED_)
#define AFX_MOTOEDLG_H__8755F190_4E37_4C90_B1C1_70AE66DB1CAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// motodlg.h : header file
//
#include "..\include\myedit.h"
#include "BaseDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CMotoEDlg dialog

class CMotoEDlg : public CBaseDlg
{
// Construction
public:
	CMotoEDlg(CWnd* pParent = NULL);   // standard constructor
	~CMotoEDlg();
	HFONT	  m_hFont;
	TCHAR m_szFirstName[MAX_PATH];
	TCHAR m_szName[MAX_PATH];
	void EditAddress(CMyEdit* pEdit);

// Dialog Data
	//{{AFX_DATA(CMotoEDlg)
	enum { IDD = IDD_DLG_MOTOE };
	CStatic	m_scLine;
	CDateTimeCtrl	m_dpBirthday;
	CStaticEx	m_scNickName;
	CStaticEx	m_scBirthday;
	CCheckEx2	m_chBirthday;
	CEdit	m_edNickName;
	CEdit	m_edURL1;
	CEdit	m_edURL2;
	CEdit	m_edNote;
	CRescaleButton		m_rbnOK;
	CRescaleButton		m_rbnCancel;
	CStaticEx	m_scPhone1;
	CStaticEx	m_scPhone2;
	CStaticEx	m_scPhone3;
	CStaticEx	m_scPhone4;
	CStaticEx	m_scPhone5;
	CStaticEx	m_scPhone6;
	CStaticEx	m_scPhone7;
	CStaticEx	m_scAddress1;
	CStaticEx	m_scAddress2;
	CStaticEx	m_scEmail1;
	CStaticEx	m_scEmail2;
	CStaticEx	m_scURL1;
	CStaticEx	m_scURL2;
	CStaticEx	m_scStorage;
	CStaticEx	m_scFirstName;
	CStaticEx	m_scLastName;
	CStaticEx	m_scGroup;
	CStaticEx	m_scNote;
	CEdit		m_edFirstName;
	CEdit		m_edLastName;
	CMyEdit		m_edAddress1;
	CMyEdit		m_edAddress2;
	CMyEdit		m_edData1;
	CMyEdit		m_edData2;
	CMyEdit		m_edData3;
	CMyEdit		m_edData4;
	CMyEdit		m_edData5;
	CMyEdit		m_edData6;
	CMyEdit		m_edData7;
	CMyEdit		m_edEmail1;
	CMyEdit		m_edEmail2;
	CComboBox	m_cbType1;
	CComboBox	m_cbType2;
	CComboBox	m_cbType3;
	CComboBox	m_cbType4;
	CComboBox	m_cbType5;
	CComboBox	m_cbType6;
	CComboBox	m_cbType7;
	CComboBox	m_cbEmail1;
	CComboBox	m_cbEmail2;
	CComboBox	m_cbAddress1;
	CComboBox	m_cbAddress2;
	CComboBox	m_cbGroup;
	CRadioEx	m_rdSIM;
	CRadioEx	m_rdME;
	//}}AFX_DATA
	CString m_strSkinFile;
	void SetSkinFileName(CString strFilename)
	{
		m_strSkinFile.Format(_T("%s"),strFilename);
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMotoEDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL xCheckEmailData();
	xSetDataToDlgCtrl();
	xSetComboItem();
	void xEnableControl(int iState);
	// Generated message map functions
	//{{AFX_MSG(CMotoEDlg)
	virtual BOOL OnInitDialog();
	virtual void OnRdSim();
	virtual void OnRdMobile();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnChBirthday();
	afx_msg void OnSetfocusAddressEdit1();
	afx_msg void OnSetfocusAddressEdit2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOTOEDLG_H__8755F190_4E37_4C90_B1C1_70AE66DB1CAE__INCLUDED_)
