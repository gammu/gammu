#if !defined(AFX_MOTODLG_H__8755F190_4E37_4C90_B1C1_70AE66DB1CAE__INCLUDED_)
#define AFX_MOTODLG_H__8755F190_4E37_4C90_B1C1_70AE66DB1CAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// motodlg.h : header file
//
#include "..\include\myedit.h"
#include "BaseDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CMotoDlg dialog

class CMotoDlg : public CBaseDlg
{
// Construction
public:
	CMotoDlg(CWnd* pParent = NULL);   // standard constructor
	~CMotoDlg();
	HFONT	  m_hFont;

// Dialog Data
	//{{AFX_DATA(CMotoDlg)
	enum { IDD = IDD_DLG_M_V501 };
	CStatic	m_scLine;
	CDateTimeCtrl	m_dpBirthday;
	CStaticEx	m_scZip;
	CStaticEx	m_scStreet2;
	CStaticEx	m_scStreet1;
	CStaticEx	m_scState;
	CStaticEx	m_scNickName;
	CStaticEx	m_scCountry;
	CStaticEx	m_scCity;
	CStaticEx	m_scBirthday;
	CCheckEx2	m_chBirthday;
	CEdit	m_edNickName;
	CEdit	m_edZip;
	CEdit	m_edStreet2;
	CEdit	m_edStreet1;
	CEdit	m_edState;
	CEdit	m_edCountry;
	CEdit	m_edCity;
	CRescaleButton		m_rbnOK;
	CRescaleButton		m_rbnCancel;
	CStaticEx	m_scType;
	CStaticEx	m_scStorage;
	CStaticEx	m_scName;
	CStaticEx	m_scGroup;
	CStaticEx	m_scData;
	CEdit		m_edName;
	CMyEdit		m_edData;
	CComboBox	m_cbType;
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
	//{{AFX_VIRTUAL(CMotoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL xCheckData();
	xSetDataToDlgCtrl();
	xSetComboItem();
	void xEnableControl(int iState);
	Tstring m_sTempNum,m_sTempStr;
	// Generated message map functions
	//{{AFX_MSG(CMotoDlg)
	virtual BOOL OnInitDialog();
	virtual void OnRdSim();
	virtual void OnRdMobile();
	afx_msg void OnSelchangeCbType();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnChBirthday();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOTODLG_H__8755F190_4E37_4C90_B1C1_70AE66DB1CAE__INCLUDED_)
