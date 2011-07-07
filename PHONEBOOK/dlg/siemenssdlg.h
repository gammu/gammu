#if !defined(AFX_SIEMENSSDLG_H__21D682FE_C9A3_452B_948B_6D718D390183__INCLUDED_)
#define AFX_SIEMENSSDLG_H__21D682FE_C9A3_452B_948B_6D718D390183__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// siemenssdlg.h : header file
//
#include "..\include\myedit.h"
#include "BaseDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSiemensSDlg dialog

class CSiemensSDlg : public CBaseDlg
{
// Construction
public:
	CSiemensSDlg(CWnd* pParent = NULL);   // standard constructor
	~CSiemensSDlg();
	HFONT	  m_hFont;
	TCHAR m_szLastName[MAX_PATH];
	TCHAR m_szName[MAX_PATH];

// Dialog Data
	//{{AFX_DATA(CSiemensSDlg)
	enum { IDD = IDD_DLG_S_S65 };
	CEdit	m_edURL;
	CStaticEx	m_scURL;
	CStaticEx	m_scLastName;
	CStaticEx	m_scFirstName;
	CStaticEx	m_scFax2;
	CStaticEx	m_scEmail2;
	CEdit	m_edLastName;
	CEdit	m_edFirstName;
	CEdit	m_edMail2;
	CMyEdit	m_edFax2;
	CRescaleButton	m_rbnOk;
	CRescaleButton	m_rbnCancel;
	CStaticEx	m_scWV;
	CStaticEx	m_scStreet;
	CStaticEx	m_scPostCode;
	CStaticEx	m_scPosition;
	CStaticEx	m_scGeneral;
	CStaticEx	m_scOffice;
	CStaticEx	m_scNickName;
	CStaticEx	m_scName;
	CStaticEx	m_scMobile;
	CStaticEx	m_scICQ;
	CStaticEx	m_scGroup;
	CStaticEx	m_scFax;
	CStaticEx	m_scMail;
	CStaticEx	m_scCountry;
	CStaticEx	m_scCompany;
	CStaticEx	m_scComServices;
	CStaticEx	m_scCity;
	CStaticEx	m_scBirthday;
	CStaticEx	m_scAIM;
	CStaticEx	m_scAddress;
	CEdit	m_edWV;
	CEdit	m_edStreet;
	CEdit	m_edPostCode;
	CMyEdit	m_edOffice;
	CMyEdit	m_edGeneral;
	CMyEdit	m_edMobile;
	CEdit	m_edNickName;
	CEdit	m_edName;
	CEdit	m_edMail;
	CEdit	m_edICQ;
	CMyEdit	m_edFax;
	CEdit	m_edCountry;
	CEdit	m_edCompany;
	CEdit	m_edCity;
	CEdit	m_edAIM;
	CDateTimeCtrl	m_dpBirthday;
	CCheckEx2	m_chBirthday;
	CComboBox	m_cbGroup;
	CRadioEx	m_rdSIM;
	CRadioEx	m_rdME;
	CStatic	m_scLine;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSiemensSDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	xSetComboItem();
	xSetDataToDlgCtrl();
	xEnableControl(int iState);

	// Generated message map functions
	//{{AFX_MSG(CSiemensSDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnRdMobile();
	virtual void OnRdSim();
	virtual void OnChBirthday();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIEMENSSDLG_H__21D682FE_C9A3_452B_948B_6D718D390183__INCLUDED_)
