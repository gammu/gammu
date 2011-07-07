#if !defined(AFX_NS6ADDRESSDLG_H__B4DBDFD7_BE17_4696_B857_87CAC4A57120__INCLUDED_)
#define AFX_NS6ADDRESSDLG_H__B4DBDFD7_BE17_4696_B857_87CAC4A57120__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NS6AddressDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNS6AddressDlg dialog
#include "..\include\myedit.h"

class CNS6AddressDlg : public CDialog
{
// Construction
public:
	~CNS6AddressDlg();
	void FillData();
	CString GetAddress();
	void SetAddress(CString strAddress);
	CNS6AddressDlg(CWnd* pParent = NULL);   // standard constructor
	HFONT	  m_hFont;
	void AnaAddressString(LPCTSTR szString,CStringList *pstrlist );

// Dialog Data
	//{{AFX_DATA(CNS6AddressDlg)
	enum { IDD = IDD_NS6_ADDRESSDLG };
	CRescaleButton	m_rbnCancel;
	CRescaleButton	m_rbnOK;
	CStaticEx	m_scStreet;
	CStaticEx	m_scState;
	CStaticEx	m_scPostCode;
	CStaticEx	m_scPOBox;
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
	CString m_strAddress;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNS6AddressDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNS6AddressDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NS6ADDRESSDLG_H__B4DBDFD7_BE17_4696_B857_87CAC4A57120__INCLUDED_)
