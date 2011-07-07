// PhoneBook.h : main header file for the PHONEBOOK application
//

#if !defined(AFX_PHONEBOOK_H__06E8D730_A748_49AF_9FA8_1C4771269F21__INCLUDED_)
#define AFX_PHONEBOOK_H__06E8D730_A748_49AF_9FA8_1C4771269F21__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "MSOTDll.h"

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About


/////////////////////////////////////////////////////////////////////////////
// CPhoneBookApp:
// See PhoneBook.cpp for the implementation of this class
//

class CPhoneBookApp : public CWinApp
{
public:
	CPhoneBookApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhoneBookApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	TCHAR m_szModule[MAX_PATH];
	TCHAR m_szSkin[MAX_PATH];
	TCHAR m_szFont[MAX_PATH];
	TCHAR m_szLang[MAX_PATH];
	TCHAR m_szRes[MAX_PATH];
	int m_iCodepage;
	bool m_bExit;

	//member variable from main panel
	int	m_iMobileCompany;
	TCHAR	m_szPhone[MAX_PATH];
	TCHAR	m_szConnectMode[MAX_PATH];
	TCHAR	m_szPortNum[MAX_PATH];
	TCHAR	m_szMobileName[MAX_PATH];
	TCHAR	m_szMobileName_ori[MAX_PATH];
	TCHAR	m_szIMEI[MAX_PATH];

	//member to store the class name
	TCHAR m_szClassName[MAX_PATH];
	BOOL	m_bNotSupportME;
	BOOL	m_bNotSupportSM;
    // DLL CONTRUCT
	// The function for load the strings
    void    LoadStringSetting();
	// The Handle of Main Application
    HWND    AppHwnd;
	// The HINSTANCE of AnwMobile.dll
	HINSTANCE m_hInstAnwMobile;
	CMSOTDLL m_MSOTDll ;
	bool m_bLoadMSOutlook;

	//{{AFX_MSG(CPhoneBookApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CPhoneBookApp theApp;
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHONEBOOK_H__06E8D730_A748_49AF_9FA8_1C4771269F21__INCLUDED_)
