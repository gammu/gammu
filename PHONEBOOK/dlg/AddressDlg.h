#if !defined(AFX_ADDRESSDLG_H__B4DBDFD7_BE17_4696_B857_87CAC4A57120__INCLUDED_)
#define AFX_ADDRESSDLG_H__B4DBDFD7_BE17_4696_B857_87CAC4A57120__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NS4AddressDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddressDlg dialog
#include "..\include\myedit.h"

class CAddressDlg : public CDialog
{
// Construction
public:
	~CAddressDlg();
	void FillData();
	void SetAddress(CString strStreet,CString strCity,
									CString strState ,CString strZip,CString strCountry);
	CAddressDlg(CWnd* pParent = NULL);   // standard constructor
	HFONT	  m_hFont;

// Dialog Data
	//{{AFX_DATA(CAddressDlg)
	enum { IDD = IDD_ADDRESSDLG };
	CRescaleButton	m_rbnCancel;
	CRescaleButton	m_rbnOK;
	CStaticEx	m_scStreet;
	CStaticEx	m_scState;
	CStaticEx	m_scPostCode;
	CStaticEx	m_scCountry;
	CStaticEx	m_scCity;
	CMyEdit	m_edStreet;
	CMyEdit	m_edState;
	CMyEdit	m_edCountry;
	CMyEdit	m_edCity;
	CMyEdit	m_edPOBox;
	CMyEdit	m_edPostCode;
	//}}AFX_DATA
	COLORREF	m_crBg;
	CString m_strStreet;
	CString m_strCity;
	CString m_strState;
	CString m_strZip;
	CString m_strCountry;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddressDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAddressDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDRESSDLG_H__B4DBDFD7_BE17_4696_B857_87CAC4A57120__INCLUDED_)
