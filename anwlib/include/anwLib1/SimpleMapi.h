#if !defined(AFX_SIMPLEMAPI_H__101DFA67_D507_4BF0_BFB0_A0631AB05B03__INCLUDED_)
#define AFX_SIMPLEMAPI_H__101DFA67_D507_4BF0_BFB0_A0631AB05B03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SimpleMapi.h : header file
//
#include "mapi.h"
#include <afxtempl.h>
#include "_defanwlib1ext.h"

typedef ULONG (FAR PASCAL *FnType_MAPILogoff) (LHANDLE lhSession, ULONG ulUIParam, 
											   FLAGS flFlags, ULONG ulReserved);

typedef ULONG (FAR PASCAL *FnType_MAPILogon) (ULONG ulUIParam, LPTSTR lpszProfileName, 
											  LPTSTR lpszPassword, FLAGS flFlags, 
											  ULONG ulReserved, LPLHANDLE lplhSession);

typedef ULONG (FAR PASCAL *FnType_MAPISendDocuments) (ULONG ulUIParam, LPTSTR lpszDelimChar, 
													  LPTSTR lpszFullPaths, LPTSTR lpszFileNames, 
													  ULONG ulReserved);

typedef ULONG (FAR PASCAL *FnType_MAPISendMail) ( LHANDLE lhSession, ULONG ulUIParam,
												  lpMapiMessage lpMessage, FLAGS flFlags,
												  ULONG ulReserved );

/////////////////////////////////////////////////////////////////////////////
// CSimpleMapi

class ANWLIB1_EXT CSimpleMapi
{

// Construction
public:
	CSimpleMapi();


protected:
	static BOOL _Connect_MapiLibrary();
	static void _Disconnect_MapiLibrary();

	ULONG FAR PASCAL MAPILogoff(LHANDLE lhSession, ULONG ulUIParam, FLAGS flFlags, ULONG ulReserved);
	static BOOL GetMapiDll(TCHAR* dllname );

// Attributes
public:
	LHANDLE	m_hSession;

	static HMODULE					m_hMAPILibrary;
	static BOOL						m_bConnected;
	static FnType_MAPILogon			m_fnMAPILogon;
	static FnType_MAPILogoff		m_fnMAPILogoff;
	static FnType_MAPISendDocuments m_fnMAPISendDocuments;
	static FnType_MAPISendMail		m_fnMAPISendMail;

// Operations
public:
	ULONG Logon(HWND hParentWnd, LPTSTR lpszProfileName, LPTSTR lpszPassword);
	ULONG Logoff(HWND hParentWnd = NULL);
	ULONG SendDocument(HWND hParentWnd, LPTSTR lpszDelimChar, LPTSTR lpszFullPaths, LPTSTR lpszFileNames=NULL);
	ULONG SendMail( HWND hParentWnd, CList<CString,CString&>* attached, FLAGS flFlags );

// Implementation
public:
	virtual ~CSimpleMapi();

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMPLEMAPI_H__101DFA67_D507_4BF0_BFB0_A0631AB05B03__INCLUDED_)
