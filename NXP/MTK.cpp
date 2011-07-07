// MTK.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "MTK.h"
#include "commfun.h"
#include "MTKfundef.h"

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
// CMTKApp

BEGIN_MESSAGE_MAP(CMTKApp, CWinApp)
	//{{AFX_MSG_MAP(CMTKApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMTKApp construction

CMTKApp::CMTKApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_pDebuginfo = NULL;
	m_ConnectionType = NULL;
	m_pMobileInfo = NULL;
	m_pWriteCommandfn = NULL;

	m_File = NULL;
	m_bSetPath = false;
	ZeroMemory(&m_ChangeFolderPath,sizeof(GSM_File));

	GetTempPath(MAX_PATH,m_szPHKTempFile);
	int nLength = strlen(m_szPHKTempFile);
	if(m_szPHKTempFile[nLength-1] != '\\')
		strcat(m_szPHKTempFile,"\\anwPhk.tmp");
	else
		strcat(m_szPHKTempFile,"anwPhk.tmp");
	//////////////////////////////////////////////////////////////////////////080424libaoliu
	GetTempPath(MAX_PATH,m_szPHKIndexFileTempFile);
	 nLength = strlen(m_szPHKIndexFileTempFile);
	if(m_szPHKIndexFileTempFile[nLength-1] != '\\')
		strcat(m_szPHKIndexFileTempFile,"\\anwPhkIndexFile.tmp");
	else
		strcat(m_szPHKIndexFileTempFile,"anwPhkIndexFile.tmp");

	GetTempPath(MAX_PATH,m_szPHKDataFileTempFile);
	 nLength = strlen(m_szPHKDataFileTempFile);
	if(m_szPHKDataFileTempFile[nLength-1] != '\\')
		strcat(m_szPHKDataFileTempFile,"\\anwPhkDataFile.tmp");
	else
		strcat(m_szPHKDataFileTempFile,"anwPhkDataFile.tmp");
	GetTempPath(MAX_PATH,m_szPHKAddDataFileTempFile);
	nLength=strlen(m_szPHKAddDataFileTempFile);
	if (m_szPHKAddDataFileTempFile[nLength-1] !='\\')
	{
		strcat(m_szPHKAddDataFileTempFile,"\\anwPhkAddDataFile.tmp");
	}
	else
		strcat(m_szPHKAddDataFileTempFile,"anwPhkAddDataFile.tmp");
	//////////////////////////////////////////////////////////////////////////080612libaoliu
	GetTempPath(MAX_PATH,m_szCalendarIndexFileTempFile);
	nLength=strlen(m_szCalendarIndexFileTempFile);
	if (m_szCalendarIndexFileTempFile[nLength-1] !='\\')
	{
		strcat(m_szCalendarIndexFileTempFile,"\\anwCalendarIndexFile.tmp");
	}
	else
		strcat(m_szCalendarIndexFileTempFile,"anwCalendarIndexFile.tmp");
	
	GetTempPath(MAX_PATH,m_szCalendarDataFileTempFile);
	nLength=strlen(m_szCalendarDataFileTempFile);
	if (m_szCalendarDataFileTempFile[nLength-1] !='\\')
	{
		strcat(m_szCalendarDataFileTempFile,"\\anwCalendarDataFile.tmp");
	}
	else
		strcat(m_szCalendarDataFileTempFile,"anwCalendarDataFile.tmp");


	


	isFirst=false;
	isGetZero=false;
	m_unDecodeBase64=NULL;
	position=NULL;
	isCancel=true;
	//////////////////////////////////////////////////////////////////////////


	// added by mingfa for 9@9u Calendar
	GetTempPath(MAX_PATH,m_szCalendarTempFile);
	nLength = strlen(m_szCalendarTempFile);
	if(m_szCalendarTempFile[nLength-1] != '\\')
		strcat(m_szCalendarTempFile,"\\anwCalendar.tmp");
	else
		strcat(m_szCalendarTempFile,"anwCalendar.tmp");



	m_pGetFiletatusfn = NULL;
	m_nPacketSize = 108;
	m_pContactEntry = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMTKApp object

CMTKApp theApp;

