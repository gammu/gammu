// Serial.h : main header file for the SERIAL DLL
//

#if !defined(AFX_SERIAL_H__278F6212_66FF_4169_8654_7F4F9E6060BB__INCLUDED_)
#define AFX_SERIAL_H__278F6212_66FF_4169_8654_7F4F9E6060BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSerialApp
// See Serial.cpp for the implementation of this class
//

class CSerialApp : public CWinApp
{
public:
	CSerialApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSerialApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSerialApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERIAL_H__278F6212_66FF_4169_8654_7F4F9E6060BB__INCLUDED_)
