// PhoneBook.h : main header file for the PHONEBOOK application
//

#if !defined(AFX_SMSUtility_H__06E8D730_A748_49AF_9FA8_1C4771269F21__INCLUDED_)
#define AFX_SMSUtility_H__06E8D730_A748_49AF_9FA8_1C4771269F21__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CPhoneBookApp:
// See PhoneBook.cpp for the implementation of this class
//

class CSMSUtility : public CWinApp
{
public:
	CSMSUtility();
	BOOL GetStringINI();

	ULONG_PTR g_gdiplusToken;
	int m_iCodepage;
	bool 	m_bExit;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhoneBookApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	TCHAR			m_szLang[MAX_PATH];
	TCHAR			m_szRes[MAX_PATH];
	TCHAR			m_szModule[MAX_PATH];
	TCHAR			m_szSkin[MAX_PATH];
	TCHAR			m_szFont[MAX_PATH];
	TCHAR			m_szCharSet[MAX_PATH];

	LOGFONT			m_lf;
	
	
	//int				m_iCodepage;

	HTREEITEM       cTreeItemDrag;
    HTREEITEM       cTreeItemDrop;
	BOOL            cDragging;
	CImageList      *cpDragImage;
	CWnd            *cpDragWnd;
    CWnd            *cpDropWnd;
	int             cListItemDragIndex;
	int             cListItemDropIndex;
	CPoint          cDropPoint;  


	bool			nIsConnected;

    // DLL CONTRUCT
	// The Handle of Main Application
    HWND    AppHwnd;
	// The HINSTANCE of AnwMobile.dll
	HINSTANCE m_hInstAnwMobile;
	//{{AFX_MSG(CSMSUtility)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CSMSUtility theApp;
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHONEBOOK_H__06E8D730_A748_49AF_9FA8_1C4771269F21__INCLUDED_)
