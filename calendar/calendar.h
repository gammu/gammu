// Calendar.h : main header file for the CALENDAR application
//

#if !defined(AFX_CALENDAR_H__AE697A6B_4915_4F8F_B191_DEA5A6C447A5__INCLUDED_)
#define AFX_CALENDAR_H__AE697A6B_4915_4F8F_B191_DEA5A6C447A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "MSOTDll.h"
enum{
	CO_NOKIA = 1,
	CO_SONY_ERISON = 2,
	CO_SIMENS = 3,
	CO_ASUS = 4,
	CO_MOTO = 5,	
	CO_SAMSUNG = 6,		//bobby 8.3.05
	CO_SHARP = 7,		
	CO_PANASONIC = 8,		
	CO_LG = 9,		
	CO_SAGEM = 10		
};
/////////////////////////////////////////////////////////////////////////////
// CCalendarApp:
// See Calendar.cpp for the implementation of this class
//

class CCalendarApp : public CWinApp
{
public:
	CCalendarApp();
	BOOL   m_bOpenDriverFinal;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCalendarApp)
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
	TCHAR m_szIMEI[MAX_PATH];
	int   m_iCodepage;
	bool m_bExit;
	BOOL				 m_bMobileSetDateTime;
	CMSOTDLL m_MSOTDll ;
	bool m_bLoadMSOutlook;

	//LOCALE_INFO  m_enumLanguage;
	int m_nStartYear;
	//member variable from main panel
	int	m_iMobileCompany;
	TCHAR	m_szPhone[MAX_PATH];
	TCHAR m_szConnectMode[MAX_PATH];
//	int m_iPortNum;
	TCHAR m_szPortName[MAX_PATH];//nono,2004_1030
	TCHAR m_szMobileName[MAX_PATH];
	TCHAR m_szMobileName_ori[MAX_PATH];
	TCHAR m_szDriverDllName[MAX_PATH];

	//member to store the class name
	TCHAR m_szClassName[MAX_PATH];

    // DLL CONTRUCT
	// The Handle of Main Application
    HWND    AppHwnd;
	// The HINSTANCE of AnwMobile.dll
	HINSTANCE m_hInstAnwMobile;
    // DLL CONTRUCT
    void LoadStringSetting();
	//{{AFX_MSG(CCalendarApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CCalendarApp theApp;
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CALENDAR_H__AE697A6B_4915_4F8F_B191_DEA5A6C447A5__INCLUDED_)
