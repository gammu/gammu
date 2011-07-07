// ATgen.h : main header file for the ATGEN DLL
//

#if !defined(AFX_ATGEN_H__E373CDED_10B0_4319_BB91_FC41149573D3__INCLUDED_)
#define AFX_ATGEN_H__E373CDED_10B0_4319_BB91_FC41149573D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "MbGlobals.h"		
#include "commfun.h"		
#include "AtGenFundef.h"		
#include "gsmlogo.h"		
#include "gsmsms.h"		
#include "gsmcal.h"		

/////////////////////////////////////////////////////////////////////////////
// CATgenApp
// See ATgen.cpp for the implementation of this class
//

class CATgenApp : public CWinApp
{
public:
	CATgenApp();
	Debug_Info	*m_pDebuginfo;
	GSM_Lines		m_Lines;
	GSM_AT_PBK_Charset m_PBKCharset;
	GSM_AT_Manufacturer m_ManufacturerID;
	bool			m_UCS2CharsetFailed;	/* Whether setting of UCS2 charset has already failed 		*/
	bool			m_NonUCS2CharsetFailed;	/* Whether setting of non-UCS2 charset has already failed 	*/
	char* m_pszTemp;
	GSM_DateTime *m_pdate_time;

	GSM_NetworkInfo *m_NetworkInfo;
	GSM_BatteryCharge *m_BatteryCharge;
	GSM_SignalQuality *m_SignalQuality;
//phonebook
	GSM_MemoryInfo m_MemoryInfo;
	GSM_MemoryStatus *m_MemoryStatus;
	GSM_MemoryEntry	  *m_Memory;
	GSM_MemoryEntry	  m_PBKMemoryEntry;
	int				m_NextMemoryEntry;
	GSM_AT_SBNR		m_PBKSBNR;
	char m_szPHKTempFile[MAX_PATH*2];
	int m_PHKNum;
	GSM_MemoryType		m_PBKMemory;	  	/* Last read PBK memory				 		*/
	char			m_PBKMemories[AT_PBK_MAX_MEMORIES + 1]; /* Supported by phone PBK memories 		*/

	OnePhoneModel * m_pMobileInfo;
//sms
	GSM_SMSMemoryStatus *m_SMSStatus;
	GSM_AT_SMSMemory	m_PhoneSMSMemory;	  	/* Is phone SMS memory available ? 				*/
	GSM_AT_SMSMemory	m_SIMSMSMemory;	  	/* Is SIM SMS memory available ? 				*/
	GSM_MemoryType		m_SMSMemory;	  	/* Last read SMS memory 					*/
	bool			m_CanSaveSMS;
	GSM_MultiSMSMessage	*m_GetSMSMessage;
	GSM_SMSMemoryStatus	m_LastSMSStatus;
	int			m_LastSMSRead;
	GSM_SMSMessage		*m_SaveSMSMessage;
	bool			m_EditMode;
	GSM_Error m_SendSMSStatus;
	bool      m_bGetSendSMSResult;
	GSM_SMSC		*m_SMSC;
	GSM_AT_SMS_Modes	m_SMSMode;	  	/* PDU or TEXT mode for SMS ? 					*/
	SMSUsedLocation m_SMSUsedLocation;
	int m_MAXSMSINFOLDER;
	int m_SIMSize;
	int m_PhoneSize;

	GSM_Error (* pSetATProtocolDatafn)(bool EditMode,bool bFastWrite,DWORD dwFlag);
	///080506libaoliu
	unsigned char *m_unDecodeBase64;
	char m_szPHKIndexFileTempFile[MAX_PATH*2];
	char m_szPHKDataFileTempFile[MAX_PATH*2];
	char m_szPHKAddDataFileTempFile[MAX_PATH*2];
	char m_szSMSDataFileTempFile[MAX_PATH*2];
	bool isFirst;
	bool isGetZero;
	bool isMEsms;
	bool isbNewXP;


// Overrides 
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CATgenApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CATgenApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ATGEN_H__E373CDED_10B0_4319_BB91_FC41149573D3__INCLUDED_)
