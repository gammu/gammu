#if !defined(AFX_TASKFRAMEDLG_H__E03A894B_2F20_4BE1_B681_FA1B90819556__INCLUDED_)
#define AFX_TASKFRAMEDLG_H__E03A894B_2F20_4BE1_B681_FA1B90819556__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef AFX_MOBILECOMMON_H
#include "MobileCommon.h"
#endif

//Data type for Manager call
typedef CList<CString, CString&> FileList;
typedef FileList *FileListP;

// TaskFrameDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Class for DLL Resource switcher
extern "C" AFX_EXTENSION_MODULE ExtensionDLL;

#ifndef SWITCH_RESOURCE
class DllInstanceSwitcher
{
public:
    DllInstanceSwitcher()
    {
        m_hInst = AfxGetResourceHandle();
        AfxSetResourceHandle(ExtensionDLL.hResource);
    }

    ~DllInstanceSwitcher()
    {
        AfxSetResourceHandle(m_hInst);
    }

private:
    HINSTANCE m_hInst;
};
#define SWITCH_RESOURCE  DllInstanceSwitcher __SwitchInstance;
#endif	//#ifndef SWITCH_RESOURCE

class CTaskMainDlg;
class CTitleBarPanel;
/////////////////////////////////////////////////////////////////////////////
// TaskFrameDlg dialog

class AFX_EXT_CLASS CTaskFrameDlg : public CDialog
{
public:
	//For m_nStatus
	void FromTask( int nTaskMode );	//use TaskMode,  in "MobileCommon.h"
	void FromMgr( int nTaskMode, FileListP pList ); //use TaskMode, in "MobileCommon.h"
	FileListP GetFileList();

// Construction
public:
	CTaskFrameDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTaskFrameDlg();

	BOOL	ShowWindow(int nCmdShow);
	int		DoModal();

//	void EndDialog(int nResult);

// Dialog Data
	//{{AFX_DATA(CTaskFrameDlg)
	enum { IDD = 0/*IDD_TASKFRAME_DLG*/ };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTaskFrameDlg)
	public:
	virtual BOOL Create(CWnd* pParentWnd = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	void xCreateAllPane();
	long xOnClose();
	long xOnOK();
	long xOnMin();

	CWnd	*m_TaskPanelDlg;
	CWnd	*m_TaskTitleBar;
	SWITCH_RESOURCE;		//for switch resource between Application and Dll.

	// Generated message map functions
	//{{AFX_MSG(CTaskFrameDlg)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnPanelButtonDown( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnExitSizeMove( WPARAM wParam, LPARAM lParam );
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClose();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int			m_nMode;
	FileListP	m_pList;
	FileList	m_OutList;

	//Support Modal and Modaless(Task is Modaless, Manager is Modal)
	BOOL	m_isDoModal;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TASKFRAMEDLG_H__E03A894B_2F20_4BE1_B681_FA1B90819556__INCLUDED_)
