#if !defined(AFX_SONYERICCSONSKDLG_H__100D2370_5EE4_4FDF_8AF7_D20EC9450875__INCLUDED_)
#define AFX_SONYERICCSONSKDLG_H__100D2370_5EE4_4FDF_8AF7_D20EC9450875__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// sonyericcsonskdlg.h : header file
//
#include "..\include\myedit.h"
#include "BaseDlg.h"


/////////////////////////////////////////////////////////////////////////////
// CSonyEricssonSKDlg dialog
class CSonyEricssonSKDlg : public CBaseDlg
{
// Construction
public:
	CSonyEricssonSKDlg(CWnd* pParent = NULL);   // standard constructor
	~CSonyEricssonSKDlg();
	HFONT	  m_hFont;

// Dialog Data
	//{{AFX_DATA(CSonyEricssonSKDlg)
	enum { IDD = IDD_DLG_SE_S700 };
	CStaticEx	m_scZipW;
	CStaticEx	m_scStreetW;
	CStaticEx	m_scStateW;
	CStaticEx	m_scCountryW;
	CStaticEx	m_scCityW;
	CEdit	m_edZipW;
	CEdit	m_edStreetW;
	CEdit	m_edStateW;
	CEdit	m_edCountryW;
	CEdit	m_edCityW;
	CDateTimeCtrl	m_dpBirthday;
	CStaticEx	m_scEmail3;
	CStaticEx	m_scEmail2;
	CStaticEx	m_scBirthday;
	CEdit	m_edEmail3;
	CEdit	m_edEmail2;
	CCheckEx2	m_chBirthday;
	CRescaleButton	m_rbnOk;
	CRescaleButton	m_rbnCancel;
	CStaticEx	m_scZip;
	CStaticEx	m_scWork;
	CStaticEx	m_scWeb;
	CStaticEx	m_scTitle;
	CStaticEx	m_scStreet;
	CStaticEx	m_scState;
	CStaticEx	m_scPosition;
	CStaticEx	m_scOther;
	CStaticEx	m_scName;
	CStaticEx	m_scMobile;
	CStaticEx	m_scMail;
	CStaticEx	m_scInfo;
	CStaticEx	m_scHome;
	CStaticEx	m_scFax;
	CStaticEx	m_scCountry;
	CStaticEx	m_scCompany;
	CStaticEx	m_scCity;
	CStatic		m_scLine;
	CEdit		m_edZip;
	CMyEdit		m_edWork;
	CEdit		m_edWeb;
	CEdit		m_edTitle;
	CEdit		m_edStreet;
	CEdit		m_edState;
	CMyEdit		m_edOther;
	CEdit		m_edName;
	CMyEdit		m_edMobile;
	CEdit		m_edMail;
	CEdit		m_edInfo;
	CMyEdit		m_edHome;
	CMyEdit		m_edFax;
	CEdit		m_edCountry;
	CEdit		m_edCompany;
	CEdit		m_edCity;
	CRadioEx	m_rdSIM;
	CRadioEx	m_rdME;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSonyEricssonSKDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	xSetDataToDlgCtrl();
	xEnableControl(int iState);

	// Generated message map functions
	//{{AFX_MSG(CSonyEricssonSKDlg)
	virtual void OnRdSim();
	virtual void OnRdMobile();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnChBirthday();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SONYERICCSONSKDLG_H__100D2370_5EE4_4FDF_8AF7_D20EC9450875__INCLUDED_)
