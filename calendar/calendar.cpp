// Calendar.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Calendar.h"
#include "CommUIExportFun.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HWND g_hMutexWnd = NULL;

/////////////////////////////////////////////////////////////////////////////
// CCalendarApp

#define ORGANIZE_CLASSNAME	_T("A&W Organize")

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
	memset(m_szModule,0,sizeof(TCHAR) * MAX_PATH);	//调用模块
	memset(m_szSkin,0,sizeof(TCHAR) * MAX_PATH);	//ui路径
	memset(m_szFont,0,sizeof(TCHAR) * MAX_PATH);	//字体路径
	memset(m_szConnectMode,0,sizeof(TCHAR) * MAX_PATH);	//连接模式
	memset(m_szMobileName,0,sizeof(TCHAR) * MAX_PATH);	//连接手机名称
	memset(m_szMobileName_ori,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szLang,0,sizeof(TCHAR) * MAX_PATH);		//语言包路径
	memset(m_szRes,0,sizeof(TCHAR) * MAX_PATH);			//？
	m_iCodepage = 1;		//？
	m_szIMEI[0] = '\0';		//？
#ifdef _UNICODE
	wsprintf(m_szDriverDllName,_T("%s"),_T("AnwMobileU.dll"));	//底层dll
#else
	wsprintf(m_szDriverDllName,_T("%s"),_T("AnwMobile.dll"));
#endif
	
	m_bOpenDriverFinal = FALSE;		//？

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCalendarApp object

CCalendarApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCalendarApp initialization

BOOL CCalendarApp::InitInstance()
{
	AfxEnableControlContainer();

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

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	//Check the window exist or not
	//如果程序已经运行，不重复启动程序
	_tcscpy(m_szClassName ,ORGANIZE_CLASSNAME);	
	TCHAR szTmp[MAX_PATH];
	if(al_GetSettingString(_T("public"),_T("IDS_TITLE"),theApp.m_szRes,szTmp))
	{
		HWND hWnd = ::FindWindow(m_szClassName,szTmp);
		if(hWnd)
			exit(1);
	}

	OleInitialize(NULL);
	if (!AfxOleInit())
	{
		AfxMessageBox(_T("COM Failed to initialize"));
		return FALSE;
	}

	//Get the default path
	//get module path
	//获得当前exe的路径 m_szModule 格式x:\\xx\\xx\\ 后面有\
	al_GetModulePath(m_hInstance,m_szModule);

	//get language profile path
	_tcscpy(m_szLang,m_szModule);
	_tcscat(m_szLang,_T("lang.ini"));	//m_szLang = x:\\xx\\lang.ini
	//get language resource profile 
	memset(szTmp,0,sizeof(TCHAR) * MAX_PATH);
	_tcscpy(m_szRes,m_szModule);
	//al_GetLangProfile(m_szLang,"Phonebook",szTmp);

    //Get Language setting 数值放入cslanguage
	//typedef enum{liNone = 0, liEnglish, liTC, liSC} LOCALE_INFO;
	CString csLanguage;
	al_GetSettingString(_T("Language"),_T("AP_Lang"), m_szLang, csLanguage.GetBuffer(MAX_PATH));
	//得到语言文件名
	al_GetLangProfile(m_szLang,_T("Calendar"),szTmp);//nono, 2004_1101
	_tcscat(m_szRes,szTmp);		//m_szRes = x:\\xx\\lang\calendar.ini
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

	//初始化以下参数 读取mobile_setting.ini
	/*-------------------------------
	m_iMobileCompany
	m_szPhone
	m_szPortName
	m_szConnectMode
	m_szMobileName
	m_szMobileName_ori
	m_szIMEI
	m_szDriverDllName
	--------------------------------*/
	Tstring sProfile = Tstring(m_szModule) + Tstring(_T("mobile_setting.ini"));
	al_GetSettingInt(_T("mobile"),_T("company"),const_cast<TCHAR *>(sProfile.c_str()),m_iMobileCompany);
	al_GetSettingString(_T("mobile"),_T("phone"),const_cast<TCHAR *>(sProfile.c_str()),m_szPhone);
	al_GetSettingString(_T("mobile"),_T("port"),const_cast<TCHAR *>(sProfile.c_str()),m_szPortName);//nono, 2004_1030
	al_GetSettingString(_T("mobile"),_T("connect_mode"),const_cast<TCHAR *>(sProfile.c_str()),m_szConnectMode);
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

	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

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
	//全局记录当前的运行句柄
	g_hMutexWnd = pFrame->m_hWnd;
	//使用下面函数将mainframe置为最顶层
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, 0, SPIF_SENDCHANGE);
	pFrame->SetForegroundWindow();

	/*向mainframe发消息，处理函数为mainframe的onpencalendar 
	  代码为： 
	     ::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_OPENCALENDAR,0,0);
	  向主窗口的Pane(0,1) 发消息...
	*/
	pFrame->PostMessage(WM_OPENCALENDAR, 0,0);
	//::PostMessage(pFrame->GetSafeHwnd(), WM_MOBILERELOAD, ID_BN_REFRESH, 0);
	//pFrame->OnLoadDataFromMobile();


	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CCalendarApp message handlers







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

// CCalendarApp message handlers


int CCalendarApp::ExitInstance() 
{
	OleUninitialize();
	
	//free the resource lib

	return CWinApp::ExitInstance();
}
