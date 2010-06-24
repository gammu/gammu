// IRDA.h : main header file for the IRDA DLL
//

#if !defined(AFX_IRDA_H__CD8DC7F1_E117_4820_B804_A31BE9222D62__INCLUDED_)
#define AFX_IRDA_H__CD8DC7F1_E117_4820_B804_A31BE9222D62__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CIRDAApp
// See IRDA.cpp for the implementation of this class
//

class CIRDAApp : public CWinApp
{
public:
	CIRDAApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIRDAApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CIRDAApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IRDA_H__CD8DC7F1_E117_4820_B804_A31BE9222D62__INCLUDED_)
