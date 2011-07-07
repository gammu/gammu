// ATgen.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "ATgen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CATgenApp

BEGIN_MESSAGE_MAP(CATgenApp, CWinApp)
	//{{AFX_MSG_MAP(CATgenApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CATgenApp construction

CATgenApp::CATgenApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_ManufacturerID = AT_Unknown;

	GetTempPath(MAX_PATH,m_szPHKTempFile);
	int nLength = strlen(m_szPHKTempFile);
	if(m_szPHKTempFile[nLength-1] != '\\')
		strcat(m_szPHKTempFile,"\\anwPhk.tmp");
	else
		strcat(m_szPHKTempFile,"anwPhk.tmp");

	m_MAXSMSINFOLDER = PHONE_MAXSMSINFOLDER;
	m_PhoneSize = PHONE_MAXSMSINFOLDER;
	m_SIMSize = 0;
	//////////////////////////////////////////////////////////////////////////080424libaoliu
	GetTempPath(MAX_PATH,m_szPHKIndexFileTempFile);
	 nLength = strlen(m_szPHKIndexFileTempFile);
	if(m_szPHKIndexFileTempFile[nLength-1] != '\\')
		strcat(m_szPHKIndexFileTempFile,"\\anwSMSIndexFile.tmp");
	else
		strcat(m_szPHKIndexFileTempFile,"anwSMSIndexFile.tmp");

	GetTempPath(MAX_PATH,m_szPHKDataFileTempFile);
	 nLength = strlen(m_szPHKDataFileTempFile);
	if(m_szPHKDataFileTempFile[nLength-1] != '\\')
		strcat(m_szPHKDataFileTempFile,"\\anwSMSDataFile.tmp");
	else
		strcat(m_szPHKDataFileTempFile,"anwSMSDataFile.tmp");
	GetTempPath(MAX_PATH,m_szPHKAddDataFileTempFile);
	nLength=strlen(m_szPHKAddDataFileTempFile);
	if (m_szPHKAddDataFileTempFile[nLength-1] !='\\')
	{
		strcat(m_szPHKAddDataFileTempFile,"\\anwSMSAddDataFile.tmp");
	}
	else
		strcat(m_szPHKAddDataFileTempFile,"anwSMSAddDataFile.tmp");
	GetTempPath(MAX_PATH,	m_szSMSDataFileTempFile);
	nLength=strlen(	m_szSMSDataFileTempFile);
	if (m_szSMSDataFileTempFile[nLength-1] !='\\')
	{
		strcat(	m_szSMSDataFileTempFile,"\\anwSMSFile.tmp");
	}
	else
		strcat(	m_szSMSDataFileTempFile,"anwSMSFile.tmp");



	isFirst=false;
	isGetZero=false;
	m_unDecodeBase64=NULL;
	isMEsms=false;
	isbNewXP=false;
	//////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CATgenApp object

CATgenApp theApp;
