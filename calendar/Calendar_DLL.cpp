// Calendar_DLL.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "Calendar.h"

#include "MainFrm.h"
#include "CommUIExportFun.h"

//#include "TraceWin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HWND g_hMutexWnd = NULL;

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
// CCalendar_DLLApp
#define ORGANIZE_CLASSNAME	_T("A&W Organize")
int WINAPI StartDllEntry(HWND AppHwnd,HINSTANCE &m_hInstAnwMobile,CWinApp **ppDllApp);

BEGIN_MESSAGE_MAP(CCalendarApp, CWinApp)
	//{{AFX_MSG_MAP(CCalendarApp)
	ON_COMMAND(ID_BN_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCalendarApp construction

CCalendarApp::CCalendarApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	memset(m_szModule,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szSkin,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szFont,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szConnectMode,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szMobileName,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szMobileName_ori,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szLang,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szRes,0,sizeof(TCHAR) * MAX_PATH);
	m_iCodepage = 1;
	m_szIMEI[0] = '\0';
	wsprintf(m_szDriverDllName,_T("%s"),_T("AnwMobile.dll"));
	
	m_bOpenDriverFinal = FALSE;
	// DLL CONTRUCT
	m_hInstAnwMobile = NULL;
	m_bExit = false;
	m_bMobileSetDateTime = FALSE;
	m_bLoadMSOutlook = false;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCalendarApp object

CCalendarApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCalendarApp initialization

BOOL CCalendarApp::InitInstance()
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


/////////////////////////////////////////////////////////////////////////////
// App command to run the dialog
void CCalendarApp::OnAppAbout()
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
// CCalendarApp message handlers

int CCalendarApp::ExitInstance() 
{
	OleUninitialize();
	
	//free the resource lib

	return CWinApp::ExitInstance();
}

// DLL CONTRUCT
void CCalendarApp::LoadStringSetting()
{
	//Check the window exist or not
	_tcscpy(m_szClassName ,ORGANIZE_CLASSNAME);

	TCHAR szTmp[MAX_PATH];
	if(al_GetSettingString(_T("public"),_T("IDS_TITLE"),theApp.m_szRes,szTmp)){
		HWND hWnd = ::FindWindow(m_szClassName,szTmp);
		if(hWnd)
			exit(1);
	}
	//Get the default path
	//get module path
	al_GetModulePath(m_hInstance,m_szModule);



	//get language profile path
	_tcscpy(m_szLang,m_szModule);
	_tcscat(m_szLang,_T("lang.ini"));
	//get language resource profile 
	memset(szTmp,0,sizeof(TCHAR) * MAX_PATH);
	_tcscpy(m_szRes,m_szModule);
	//al_GetLangProfile(m_szLang,"Phonebook",szTmp);

    //Get Language setting
	//typedef enum{liNone = 0, liEnglish, liTC, liSC} LOCALE_INFO;
	CString csLanguage;
	al_GetSettingString(_T("Language"),_T("AP_Lang"), m_szLang, csLanguage.GetBuffer(MAX_PATH));

	al_GetLangProfile(m_szLang,_T("Calendar"),szTmp);//nono, 2004_1101
	_tcscat(m_szRes,szTmp);
	//get codepage
	al_GetLangCodepage(m_szLang,m_iCodepage);
	//get language skin folder path
	_tcscpy(m_szSkin,m_szModule);
	memset(szTmp,0,sizeof(TCHAR) * MAX_PATH);
	al_GetLangSkin(m_szLang,szTmp);
	_tcscat(m_szSkin,szTmp);
	//get font profile path
	_tcscpy(m_szFont,m_szSkin);
	_tcscat(m_szFont,_T("font.ini"));

	//Get the data from profile
	Tstring sProfile = Tstring(m_szModule) + Tstring(_T("mobile_setting.ini"));
	al_GetSettingInt(_T("mobile"),_T("company"),const_cast<TCHAR *>(sProfile.c_str()),m_iMobileCompany);
	switch(m_iMobileCompany)
	{
	case CO_MOTO://
		m_nStartYear = 2000;
		break;
	case CO_SONY_ERISON://
		m_nStartYear = 1969;
		break;
	case CO_NOKIA://
	default:
		m_nStartYear = 2002;
		break;
	}
	al_GetSettingString(_T("mobile"),_T("phone"),const_cast<TCHAR *>(sProfile.c_str()),m_szPhone);
//	al_GetSettingInt("mobile","phone",const_cast<TCHAR *>(sProfile.c_str()),m_iPhone);
//	al_GetSettingInt("mobile","port",const_cast<TCHAR *>(sProfile.c_str()),m_iPortNum);
	al_GetSettingString(_T("mobile"),_T("port"),const_cast<TCHAR *>(sProfile.c_str()),m_szPortName);//nono, 2004_1030
	al_GetSettingString(_T("mobile"),_T("connect_mode"),const_cast<TCHAR *>(sProfile.c_str()),m_szConnectMode);
//	al_GetSettingString(_T("mobile"),_T("name"),const_cast<TCHAR *>(sProfile.c_str()),m_szMobileName);
	al_GetSettingString(_T("mobile"),_T("displayname"),const_cast<TCHAR *>(sProfile.c_str()),m_szMobileName);
	al_GetSettingString(_T("mobile"),_T("name"),const_cast<TCHAR *>(sProfile.c_str()),m_szMobileName_ori);
	if(_tcslen(m_szMobileName)<=0)
		wsprintf(m_szMobileName,m_szMobileName_ori);
	
	al_GetSettingString(_T("mobile"),_T("IMEI"),const_cast<TCHAR *>(sProfile.c_str()),m_szIMEI);

	al_GetSettingString(_T("lib"),_T("control_dll"),const_cast<TCHAR *>(sProfile.c_str()),m_szDriverDllName);

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.
	
	free(const_cast<TCHAR*>(m_pszAppName));
	m_pszAppName = new TCHAR[MAX_PATH];
	al_GetSettingString(_T("public"),_T("IDS_TITLE"),m_szRes,const_cast<TCHAR*>(m_pszAppName));
}

// DLL CONTRUCT
int WINAPI StartDllEntry(HWND AppHwnd,HINSTANCE &m_hInstAnwMobile,CWinApp **ppDllApp)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    *ppDllApp = afxGetApp();

	//_tcscpy(theApp.m_lpCmdLine,cmdLine);

    theApp.AppHwnd = AppHwnd;

	theApp.m_hInstAnwMobile = m_hInstAnwMobile;
    theApp.LoadStringSetting();

	CString sProductInfoProfile;
	sProductInfoProfile.Format(_T("%s%s"), theApp.m_szModule, _T("ProductInfo.ini"));
	int bSupportMSOT = false;
	al_GetSettingInt(_T("PIM"), _T("FreeStyle"),(LPTSTR)(LPCTSTR)sProductInfoProfile ,bSupportMSOT);
	if(bSupportMSOT)
	{
#ifdef _UNICODE
		theApp.m_MSOTDll.LoadDll(_T("MSOTU.dll"));
#else
		theApp.m_MSOTDll.LoadDll(_T("MSOT.dll"));
#endif
		if(theApp.m_MSOTDll.MSOT_InitDLL() == SyncErr_No_Err)
		{
			theApp.m_bLoadMSOutlook = true;
			theApp.m_MSOTDll.MSOT_SetMobileName(theApp.m_szMobileName_ori);
			theApp.m_MSOTDll.MSOT_TerminateDLL();
		}
	}
	else 
		theApp.m_bLoadMSOutlook = false;
   /*
    HANDLE hMutexInstance = ::CreateMutex(NULL,TRUE,_T("Calendar"));
	bool bRunning = (GetLastError() == ERROR_ALREADY_EXISTS);
	if(hMutexInstance != NULL) ::ReleaseMutex(hMutexInstance);
	if(bRunning)
	{
	
		if(g_hMutexWnd)
		{
			BringWindowToTop(g_hMutexWnd);
			if(::SetForegroundWindow(g_hMutexWnd) != TRUE)
			{
			}
			if(IsIconic(g_hMutexWnd)) ::ShowWindow(g_hMutexWnd,SW_RESTORE);
		} return TRUE;
	}
    */
	
	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.
	CMainFrame* pFrame = new CMainFrame;
	theApp.m_pMainWnd = pFrame;
	//Set Logfile
	// create and load the frame with its resources
	pFrame->LoadFrame(
		IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
		NULL,
		NULL);

	// The one and only window has been initialized, so show and update it.
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	g_hMutexWnd = pFrame->m_hWnd;

	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, 0, SPIF_SENDCHANGE);
	pFrame->SetForegroundWindow();

	//send message to mainframe to laod data from Mobile.
	pFrame->PostMessage(WM_OPENCALENDAR, 0,0);
	//::PostMessage(pFrame->GetSafeHwnd(), WM_MOBILERELOAD, ID_BN_REFRESH, 0);
	//pFrame->OnLoadDataFromMobile();
	
	// Ken 7/28 : Use message loop for DLL
	MSG msg;
    while(GetMessage(&msg,NULL,0,0))
    {
 	   pFrame->UpdateMyDLLCmdUI();
       TranslateMessage(&msg);
       DispatchMessage(&msg);
 //      if(pFrame->FromHandlePermanent(pFrame->GetSafeHwnd()) == NULL)
	   if(theApp.m_bExit || ::IsWindow(pFrame->GetSafeHwnd()) == false)
	   {
          //  TranslateMessage(&msg);
           // DispatchMessage(&msg);
            return 1;
	   }
    }

	return 1;
}