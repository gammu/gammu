#if !defined(AFX_MOTOE2DLG_H__8755F190_4E37_4C90_B1C1_70AE66DB1CAE__INCLUDED_)
#define AFX_MOTOE2DLG_H__8755F190_4E37_4C90_B1C1_70AE66DB1CAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// motodlg.h : header file
//
#include "..\include\myedit.h"
#include "BaseDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CMotoE2Dlg dialog

class CMotoE2Dlg : public CBaseDlg
{
// Construction
public:
	CMotoE2Dlg(CWnd* pParent = NULL);   // standard constructor
	~CMotoE2Dlg();
	HFONT	  m_hFont;
//	TCHAR m_szFirstName[MAX_PATH];
//	TCHAR m_szName[MAX_PATH];
	void EditAddress(CMyEdit* pEdit);
	CPtrList m_NameList;
	void RemoveNameList();

// Dialog Data
	//{{AFX_DATA(CMotoE2Dlg)
	enum { IDD = IDD_DLG_MOTOE2 };
	CStatic	m_scLine;
	CDateTimeCtrl	m_dpBirthday;
	CDateTimeCtrl	m_dpAnniversary;
	CStaticEx	m_scBirthday;
	CCheckEx2	m_chBirthday;
	CStaticEx	m_scAnniversary;
	CCheckEx2	m_chAnniversary;
	CEdit	m_edURL;
	CEdit	m_edNote;
	CEdit	m_edCompany;
	CEdit	m_edTitle;
	CEdit	m_edManagerName;
	CEdit	m_edAssistantName;
	CEdit	m_edSpousName;
	CEdit	m_edChildernName;
	CEdit	m_edIMID;
	CRescaleButton		m_rbnOK;
	CRescaleButton		m_rbnCancel;
	CStaticEx	m_scPhone1;
	CStaticEx	m_scPhone2;
	CStaticEx	m_scPhone3;
	CStaticEx	m_scPhone4;
	CStaticEx	m_scAddress1;
	CStaticEx	m_scAddress2;
	CStaticEx	m_scEmail1;
	CStaticEx	m_scEmail2;
	CStaticEx	m_scURL;
	CStaticEx	m_scStorage;
	CStaticEx	m_scName;
	CStaticEx	m_scIMID;
	CStaticEx	m_scNote;
	CStaticEx	m_scCompanyTitle;
	CStaticEx	m_scManagerName;
	CStaticEx	m_scAssistantName;
	CStaticEx	m_scSpous;
	CStaticEx	m_scChildren;
	CStaticEx	m_scDot;
	CStaticEx	m_scDot2;
	CStaticEx	m_scDot3;
	CStaticEx	m_scDot4;
	CStaticEx	m_scDot5;
	CStaticEx	m_scDot6;
	CStaticEx	m_scDot7;
	CStaticEx	m_scDot8;
	CStaticEx	m_scDot9;
	CStaticEx	m_scSlash;
	CEdit		m_edName;
	CMyEdit		m_edAddress1;
	CMyEdit		m_edAddress2;
	CMyEdit		m_edData1;
	CMyEdit		m_edData2;
	CMyEdit		m_edData3;
	CMyEdit		m_edData4;
	CMyEdit		m_edEmail1;
	CMyEdit		m_edEmail2;
	CComboBox	m_cbType1;
	CComboBox	m_cbType2;
	CComboBox	m_cbType3;
	CComboBox	m_cbType4;
	CComboBox	m_cbEmail1;
	CComboBox	m_cbEmail2;
	CComboBox	m_cbAddress1;
	CComboBox	m_cbAddress2;
	CComboBox	m_cbNameType;
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
	//{{AFX_VIRTUAL(CMotoE2Dlg)
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
	//{{AFX_MSG(CMotoE2Dlg)
	virtual BOOL OnInitDialog();
	virtual void OnRdSim();
	virtual void OnRdMobile();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnChBirthday();
	afx_msg void OnChAnniversary();
	afx_msg void OnSetfocusAddressEdit1();
	afx_msg void OnSetfocusAddressEdit2();
	afx_msg void OnSelchangeNameCombo();
	afx_msg void OnDropdownNameCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOTOE2DLG_H__8755F190_4E37_4C90_B1C1_70AE66DB1CAE__INCLUDED_)
