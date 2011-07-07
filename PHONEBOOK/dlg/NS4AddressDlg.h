#if !defined(AFX_NS4ADDRESSDLG_H__B4DBDFD7_BE17_4696_B857_87CAC4A57120__INCLUDED_)
#define AFX_NS4ADDRESSDLG_H__B4DBDFD7_BE17_4696_B857_87CAC4A57120__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NS4AddressDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNS4AddressDlg dialog
#include "..\include\myedit.h"

class CNS4AddressDlg : public CDialog
{
// Construction
public:
	~CNS4AddressDlg();
	void FillData();
	void SetAddress(CString strExt ,CString strStreet,CString strCity,
									CString strState ,CString strZip,CString strCountry);
	CNS4AddressDlg(CWnd* pParent = NULL);   // standard constructor
	HFONT	  m_hFont;

// Dialog Data
	//{{AFX_DATA(CNS4AddressDlg)
	enum { IDD = IDD_NS4_ADDRESSDLG };
	CRescaleButton	m_rbnCancel;
	CRescaleButton	m_rbnOK;
	CStaticEx	m_scStreet;
	CStaticEx	m_scState;
	CStaticEx	m_scPostCode;
	CStaticEx	m_scExtension;
	CStaticEx	m_scCountry;
	CStaticEx	m_scCity;
	CMyEdit	m_edStreet;
	CMyEdit	m_edState;
	CMyEdit	m_edCountry;
	CMyEdit	m_edCity;
	CMyEdit	m_edPOBox;
	CMyEdit	m_edPostCode;
	CMyEdit	m_edExternsion;
	//}}AFX_DATA
	COLORREF	m_crBg;
	CString m_strExt;
	CString m_strStreet;
	CString m_strCity;
	CString m_strState;
	CString m_strZip;
	CString m_strCountry;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNS4AddressDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNS4AddressDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NS4ADDRESSDLG_H__B4DBDFD7_BE17_4696_B857_87CAC4A57120__INCLUDED_)
