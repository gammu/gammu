// At.h : main header file for the AT DLL
//

#if !defined(AFX_AT_H__D72D5296_437C_41FB_9739_9D46D319448B__INCLUDED_)
#define AFX_AT_H__D72D5296_437C_41FB_9739_9D46D319448B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "DeviceAPI.h"
#include "commfun.h"
#include "atprotocol.h"
#include "obexProtocol.h"

/////////////////////////////////////////////////////////////////////////////
// CAtApp
// See At.cpp for the implementation of this class
//

class CAtApp : public CWinApp
{
public:
	CAtApp();
	GSM_ATMultiAnwser *m_pATMultiAnwser;
	CDeviceAPI	m_DeviceAPI;
	Debug_Info	*m_pDebugInfo;
	BOOL m_bFinishRequest;
	GSM_Error m_DispatchError;
	GSM_Protocol_ATData m_ATProtocolData;
	GSM_Protocol_OBEXData m_ObexProtocolData;
	GSM_Reply_MsgType* m_pReplyCheckType;
	GSM_Error (*CommandCallBackFun)    (GSM_Protocol_Message msg);
	GSM_Error DispatchMessage(GSM_Protocol_Message *msg);

	bool  m_bCurrentObexMode;
	int m_Speed ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAtApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CAtApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AT_H__D72D5296_437C_41FB_9739_9D46D319448B__INCLUDED_)
