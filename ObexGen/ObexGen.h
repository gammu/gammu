// ObexGen.h : main header file for the OBEXGEN DLL
//

#if !defined(AFX_OBEXGEN_H__A943B5ED_8517_47D3_A4B0_F6B1D0AF045E__INCLUDED_)
#define AFX_OBEXGEN_H__A943B5ED_8517_47D3_A4B0_F6B1D0AF045E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "MbGlobals.h"		
#include "commfun.h"		

/////////////////////////////////////////////////////////////////////////////
// CObexGenApp
// See ObexGen.cpp for the implementation of this class
//

class CObexGenApp : public CWinApp
{
public:
	CObexGenApp();

	OnePhoneModel *m_pMobileInfo;
	Debug_Info	*m_pDebuginfo;
	bool m_bObexMode;
	char			m_ConnectID[4]; //peggy add
//	unsigned char			m_ObexID[1];
	OBEX_Service		m_Service;
	int				m_FrameSize;
	bool				m_FileLastPart;
	GSM_File			m_Files[2000];
	int				m_FileLev;
	GSM_File		*m_File;
	int				m_FilesLocationsUsed;
	int				m_FilesLocationsCurrent;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObexGenApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CObexGenApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OBEXGEN_H__A943B5ED_8517_47D3_A4B0_F6B1D0AF045E__INCLUDED_)
