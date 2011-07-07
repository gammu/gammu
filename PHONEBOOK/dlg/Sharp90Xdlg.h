#if !defined(AFX_SHARP90XDLG_H__31F0EA41_7A90_4831_947D_67B1CBCC1159__INCLUDED_)
#define AFX_SHARP90XDLG_H__31F0EA41_7A90_4831_947D_67B1CBCC1159__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SharpT300Dlg.h : header file
//
#include "..\include\myedit.h"
#include "BaseDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSharp90XDlg dialog

class CSharp90XDlg : public CBaseDlg
{
// Construction
public:
	CSharp90XDlg(CWnd* pParent = NULL);   // standard constructor
	~CSharp90XDlg();
	HFONT	  m_hFont;
	TCHAR m_szLastName[MAX_PATH];
	TCHAR m_szName[MAX_PATH];

// Dialog Data
	//{{AFX_DATA(CSharp90XDlg)
	enum { IDD = IDD_SHARP_90X };
	CMyEdit	m_edLastName;
	CMyEdit	m_edFirstName;
	CMyEdit	m_edEmail3;
	CMyEdit	m_edZipCode;
	CMyEdit	m_edStreet;
	CMyEdit	m_edState;
	CMyEdit	m_edCountry;
	CMyEdit	m_edCity;
	CStaticEx	m_scZipCode;
	CStaticEx	m_scStreet;
	CStaticEx	m_scState;
	CStaticEx	m_scLastName;
	CStaticEx	m_scFirstName;
	CStaticEx	m_scEmial3;
	CStaticEx	m_scCountry;
	CStaticEx	m_scCity;
	CStaticEx	m_scBirthday;
	CComboBox	m_cbEmail3;
	CDateTimeCtrl	m_dpBirthday;
	CCheckEx2	m_chBirthday;

	CRescaleButton	m_rbnOK;
	CRescaleButton	m_rbnCancel;
	CStaticEx	m_scDot6;
	CStaticEx	m_scDot5;
	CStaticEx	m_scDot4;
	CStaticEx	m_scDot3;
	CStaticEx	m_scDot2;
	CStaticEx	m_scDot1;
	CStatic		m_scLine;
	CStaticEx	m_scStorage;
	CStaticEx	m_scPhone3;
	CStaticEx	m_scPhone2;
	CStaticEx	m_scPhone1;
	CStaticEx	m_scNote;
	CStaticEx	m_scGroup;
	CStaticEx	m_scEmail2;
	CStaticEx	m_scEmail1;
	CComboBox	m_cbGroup;
	CComboBox	m_cbEmail2;
	CComboBox	m_cbEmail1;
	CComboBox	m_cbPhone3;
	CComboBox	m_cbPhone2;
	CComboBox	m_cbPhone1;
	CMyEdit	m_edNote;
	CMyEdit	m_edPhone3;
	CMyEdit	m_edPhone2;
	CMyEdit	m_edPhone1;
	CMyEdit	m_edEmail2;
	CMyEdit	m_edEmail1;
	CRadioEx	m_rdSIM;
	CRadioEx	m_rdME;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSharp90XDlg)
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
	//{{AFX_MSG(CSharp90XDlg)
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

#endif // !defined(AFX_SHARP90XDLG_H__31F0EA41_7A90_4831_947D_67B1CBCC1159__INCLUDED_)
