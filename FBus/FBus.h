// FBus.h : main header file for the FBUS DLL
//

#if !defined(AFX_FBUS_H__C15C387E_D085_414F_B038_29632A703147__INCLUDED_)
#define AFX_FBUS_H__C15C387E_D085_414F_B038_29632A703147__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "DeviceAPI.h"
#include "fbus2.h"
#include "commfun.h"

/////////////////////////////////////////////////////////////////////////////
// CFBusApp
// See FBus.cpp for the implementation of this class
//

class CFBusApp : public CWinApp
{
public:
	CFBusApp();
	GSM_ATMultiAnwser *m_pATMultiAnwser;
	CDeviceAPI	m_DeviceAPI;
	Debug_Info	*m_pDebugInfo;
	BOOL m_bFinishRequest;
	GSM_Error m_DispatchError;
	GSM_Protocol_FBUS2Data m_FBUS2Data;
	GSM_Reply_MsgType* m_pReplyCheckType;
	GSM_Error (*CommandCallBackFun)    (GSM_Protocol_Message msg);
	GSM_Error DispatchMessage(GSM_Protocol_Message* msg);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFBusApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CFBusApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FBUS_H__C15C387E_D085_414F_B038_29632A703147__INCLUDED_)
