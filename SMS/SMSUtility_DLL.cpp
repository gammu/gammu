// SMSUtility_DLL.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "SMSUtility.h"
#include "GetAppVersion.h"
#include "MainFrm.h"
//#include "TraceWin.h"
#include "CommUIExportFun.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int WINAPI StartDllEntry(HWND AppHwnd,HINSTANCE &m_hInstAnwMobile,CWinApp **ppDllApp);

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
// CSMSUtility_DLLApp

BEGIN_MESSAGE_MAP(CSMSUtility, CWinApp)
	//{{AFX_MSG_MAP(CSMSUtility)
	ON_COMMAND(ID_BN_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSMSUtility_DLLApp construction

CSMSUtility::CSMSUtility()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	return;
	memset(m_szModule,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szSkin,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szFont,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szLang,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szRes,0,sizeof(TCHAR) * MAX_PATH);
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

	nIsConnected = false;
	// DLL CONTRUCT
	m_hInstAnwMobile = NULL;
	m_iCodepage = 1;
	m_bExit = false;

}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSMSUtility_DLLApp object

CSMSUtility theApp;

/////////////////////////////////////////////////////////////////////////////
// CSMSUtility initialization

BOOL CSMSUtility::InitInstance()
{
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
/*
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
*/
	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	OleInitialize(NULL);
	/*
	if (!AfxOleInit())
	{
		AfxMessageBox(_T("COM Failed to initialize"));
		return FALSE;
	}
	*/
	m_bExit = false;

	return TRUE;
}

BOOL CSMSUtility::GetStringINI()
{
	TCHAR szTmp[MAX_PATH];
	//Get the default path
	//get module path
	al_GetModulePath(m_hInstance, m_szModule);
	//get language profile path
	_tcscpy(m_szLang, m_szModule);
	_tcscat(m_szLang, _T("lang.ini"));
	//get language resource profile 
	memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
	_tcscpy(m_szRes, m_szModule);
	al_GetLangProfile(m_szLang, _T("SMS"), szTmp);
	//_tcscat(m_szRes, _T("lang\\sms.ini"));
	_tcscat(m_szRes, szTmp);
	//get codepage
	al_GetLangCodepage(m_szLang,m_iCodepage);

	//Get the default path
	al_GetModulePath(m_hInstance, m_szModule);
	_tcscpy(m_szSkin, m_szModule);
	_tcscat(m_szSkin, _T("skin\\default\\"));
	_tcscpy(m_szFont, m_szSkin);
	_tcscat(m_szFont, _T("font.ini"));

	TCHAR  szProfile[_MAX_PATH], szAppPath[_MAX_PATH];
	al_GetModulePath(NULL, szAppPath);
	wsprintf(szProfile, _T("%s%s"), szAppPath, _T("skin\\default\\font.ini"));

	//Load font setting for ComboBox control

	TCHAR  szFaceName[_MAX_PATH], szFontSize[_MAX_PATH];
	
	al_GetSettingString(_T("Font"), _T("facename"), szProfile, szFaceName);
	al_GetSettingString(_T("Font"), _T("fontsize"), szProfile, szFontSize);
	al_GetSettingString(_T("Font"), _T("characterSet"), szProfile, m_szCharSet);

	memset(&m_lf, 0, sizeof(LOGFONT));
	m_lf.lfHeight = (_ttoi(szFontSize));
	m_lf.lfCharSet = (_ttoi(m_szCharSet));
	//lf.lfHeight = (atoi(szFontSize)*4)/3;
	_tcscpy(m_lf.lfFaceName, szFaceName);
	if(m_lf.lfHeight > 0)
		m_lf.lfHeight *= -1;

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About



// App command to run the dialog
void CSMSUtility::OnAppAbout()
{
//	CAboutDlg aboutDlg;
//	aboutDlg.DoModal();
	TCHAR szText[MAX_PATH];
	TCHAR szFile[MAX_PATH];
	al_GetSettingString(_T("public"),_T("IDS_TITLE"),theApp.m_szRes,szText);
	GetModuleFileName(NULL,szFile,MAX_PATH);
	AboutDlg(m_pMainWnd->m_hWnd,szText,szFile);
}

/////////////////////////////////////////////////////////////////////////////
// CSMSUtility message handlers
#include "AutoLayout.h"


// DLL CONTRUCT
int WINAPI StartDllEntry(HWND AppHwnd,HINSTANCE &m_hInstAnwMobile,CWinApp **ppDllApp)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    *ppDllApp = afxGetApp();

	//_tcscpy(theApp.m_lpCmdLine,cmdLine);

    theApp.AppHwnd = AppHwnd;

	theApp.m_hInstAnwMobile = m_hInstAnwMobile;

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.
	
	CMainFrame* pFrame = new CMainFrame;
	theApp.m_pMainWnd = pFrame;

	theApp.GetStringINI();

	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);

	// The one and only window has been initialized, so show and update it.
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	//LoadIcon(IDR_MAINFRAME);
	pFrame->SetIcon(theApp.LoadIcon(IDR_MAINFRAME), true);
	pFrame->SetIcon(theApp.LoadIcon(IDR_MAINFRAME), false);

	TCHAR str[MAX_PATH];
	al_GetSettingString(_T("public"),_T("IDS_TITLE"),theApp.m_szRes, str);
	pFrame->SetWindowText(str);

	free((void*)theApp.m_pszAppName);
	theApp.m_pszAppName=_tcsdup(str);

	// Ken 7/28 : Use message loop for DLL
	MSG msg;
    while(GetMessage(&msg,NULL,0,0))
    {
	   pFrame->UpdateMyDLLCmdUI();
       TranslateMessage(&msg);
       DispatchMessage(&msg);
  //     if(pFrame->FromHandlePermanent(pFrame->GetSafeHwnd()) == NULL)
	   if(theApp.m_bExit || ::IsWindow(pFrame->GetSafeHwnd()) == false)
	   {
       //     TranslateMessage(&msg);
         //   DispatchMessage(&msg);
            return 1;
	   }
    }

	return 1;
}

int CSMSUtility::ExitInstance() 
{
	OleUninitialize();
	
	return CWinApp::ExitInstance();
}
