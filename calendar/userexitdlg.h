#if !defined(AFX_USEREXITDLG_H__C3CD49FD_5D9A_4923_9D0E_967348C9C364__INCLUDED_)
#define AFX_USEREXITDLG_H__C3CD49FD_5D9A_4923_9D0E_967348C9C364__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "OrganizerModules.h"
#include "resource.h"
// UserExitDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUserExitDlg dialog

class CUserExitDlg : public CDialog
{
// Construction
private:
	HBRUSH m_hBrush;
public:
	INT GetDecision();
	CUserExitDlg(CWnd* pParent = NULL);   // standard constructor
	~CUserExitDlg();
// Dialog Data
	//{{AFX_DATA(CUserExitDlg)
	enum { IDD = IDD_PVI_5_USEREXIT };
	CButtonEx	m_ctrlDiscard;
	CButtonEx	m_ctrlSaveAs;
	CButtonEx	m_ctrlOk;
	CButtonEx	m_ctrlCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUserExitDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUserExitDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPvi5UeCancel();
	afx_msg void OnPvi5UeDiscard();
	afx_msg void OnPvi5UeSave();
	afx_msg void OnPvi5UeSaveas();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	BOOL Init();
	void LoadSkinEnglish();
	void LoadSkinTC();
	void LoadSkinSC();
	DECLARE_MESSAGE_MAP()

private:
	INT m_nDecision;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USEREXITDLG_H__C3CD49FD_5D9A_4923_9D0E_967348C9C364__INCLUDED_)
