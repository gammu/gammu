// Common.h : main header file for the COMMON DLL
//

#if !defined(AFX_COMMON_H__C7DEA01F_E322_4DE8_B05C_076D0CCBBC6E__INCLUDED_)
#define AFX_COMMON_H__C7DEA01F_E322_4DE8_B05C_076D0CCBBC6E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCommonApp
// See Common.cpp for the implementation of this class
//
#include "MobileInfo.h"
#include "commfun.h"

class CCommonApp : public CWinApp
{
public:
	~CCommonApp();
	CCommonApp();
//	CList<OnePhoneModel,OnePhoneModel> m_SupportModelList;
	CPtrList m_SupportModelList;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommonApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CCommonApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMON_H__C7DEA01F_E322_4DE8_B05C_076D0CCBBC6E__INCLUDED_)
