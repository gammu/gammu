// PhoneT.h : main header file for the PHONET DLL
//

#if !defined(AFX_PHONET_H__4FB5A0F9_8339_403A_9839_5443780D1568__INCLUDED_)
#define AFX_PHONET_H__4FB5A0F9_8339_403A_9839_5443780D1568__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "DeviceAPI.h"
#include "commfun.h"
#include "phonetprotocol.h"

/////////////////////////////////////////////////////////////////////////////
// CPhoneTApp
// See PhoneT.cpp for the implementation of this class
//

class CPhoneTApp : public CWinApp
{
public:
	CPhoneTApp();
	GSM_ATMultiAnwser *m_pATMultiAnwser;
	CDeviceAPI	m_DeviceAPI;
	Debug_Info	*m_pDebugInfo;
	BOOL m_bFinishRequest;
	GSM_Error m_DispatchError;
	GSM_Protocol_PHONETData m_PhonetData;
	GSM_Reply_MsgType* m_pReplyCheckType;
	GSM_Error (*CommandCallBackFun)    (GSM_Protocol_Message msg);
	GSM_Error DispatchMessage(GSM_Protocol_Message* msg);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhoneTApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CPhoneTApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHONET_H__4FB5A0F9_8339_403A_9839_5443780D1568__INCLUDED_)
