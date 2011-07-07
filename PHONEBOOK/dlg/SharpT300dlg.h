#if !defined(AFX_SHARPT300DLG_H__31F0EA41_7A90_4831_947D_67B1CBCC1159__INCLUDED_)
#define AFX_SHARPT300DLG_H__31F0EA41_7A90_4831_947D_67B1CBCC1159__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SharpT300Dlg.h : header file
//
#include "..\include\myedit.h"
#include "BaseDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSharpT300Dlg dialog

class CSharpT300Dlg : public CBaseDlg
{
// Construction
public:
	CSharpT300Dlg(CWnd* pParent = NULL);   // standard constructor
	~CSharpT300Dlg();
	HFONT	  m_hFont;

// Dialog Data
	//{{AFX_DATA(CSharpT300Dlg)
	enum { IDD = IDD_SHARP_T300 };
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
	CStaticEx	m_scName;
	CStaticEx	m_scGroup;
	CStaticEx	m_scEmail2;
	CStaticEx	m_scEmail1;
	CStaticEx	m_scAddress;
	CComboBox	m_cbGroup;
	CComboBox	m_cbEmail2;
	CComboBox	m_cbEmail1;
	CComboBox	m_cbPhone3;
	CComboBox	m_cbPhone2;
	CComboBox	m_cbPhone1;
	CComboBox	m_cbAddress;
	CMyEdit	m_edNote;
	CMyEdit	m_edPhone3;
	CMyEdit	m_edPhone2;
	CMyEdit	m_edPhone1;
	CMyEdit	m_edEmail2;
	CMyEdit	m_edEmail1;
	CMyEdit	m_edName;
	CMyEdit	m_edAddress;
	CRadioEx	m_rdSIM;
	CRadioEx	m_rdME;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSharpT300Dlg)
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
	//{{AFX_MSG(CSharpT300Dlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRdMobile();
	afx_msg void OnRdSim();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHARPT300DLG_H__31F0EA41_7A90_4831_947D_67B1CBCC1159__INCLUDED_)
