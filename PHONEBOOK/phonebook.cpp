// PhoneBook.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "PhoneBook.h"

#include "MainFrm.h"
#include "CommUIExportFun.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPhoneBookApp

#define ORGANIZE_CLASSNAME	_T("A&W Organize")

BEGIN_MESSAGE_MAP(CPhoneBookApp, CWinApp)
	//{{AFX_MSG_MAP(CPhoneBookApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhoneBookApp construction

CPhoneBookApp::CPhoneBookApp()
{
	memset(m_szModule,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szSkin,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szFont,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szConnectMode,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szMobileName,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szMobileName_ori,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szLang,0,sizeof(TCHAR) * MAX_PATH);
	memset(m_szRes,0,sizeof(TCHAR) * MAX_PATH);
	m_iCodepage = 1;
	m_bNotSupportME = FALSE;
	m_bNotSupportSM = FALSE;
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPhoneBookApp object

CPhoneBookApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPhoneBookApp initialization

BOOL CPhoneBookApp::InitInstance()
{
	AfxEnableControlContainer();

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
	_tcscpy(m_szClassName ,ORGANIZE_CLASSNAME);

	TCHAR szTmp[MAX_PATH];
	if(al_GetSettingString(_T("public"),_T("IDS_TITLE"),theApp.m_szRes,szTmp)){
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
	al_GetModulePath(m_hInstance,m_szModule);
	//get language profile path
	_tcscpy(m_szLang,m_szModule);
	_tcscat(m_szLang,_T("lang.ini"));
	//get language resource profile 
	memset(szTmp,0,sizeof(TCHAR) * MAX_PATH);
	_tcscpy(m_szRes,m_szModule);
	al_GetLangProfile(m_szLang,_T("Phonebook"),szTmp);
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
	if(!al_GetSettingInt(_T("mobile"),_T("company"),const_cast<TCHAR *>(sProfile.c_str()),m_iMobileCompany))
		m_iMobileCompany = 1;
	al_GetSettingString(_T("mobile"),_T("phone"),const_cast<TCHAR *>(sProfile.c_str()),m_szPhone);
	al_GetSettingString(_T("mobile"),_T("port"),const_cast<TCHAR *>(sProfile.c_str()),m_szPortNum);
	al_GetSettingString(_T("mobile"),_T("connect_mode"),const_cast<TCHAR *>(sProfile.c_str()),m_szConnectMode);
	al_GetSettingString(_T("mobile"),_T("displayname"),const_cast<TCHAR *>(sProfile.c_str()),m_szMobileName);
	al_GetSettingString(_T("mobile"),_T("name"),const_cast<TCHAR *>(sProfile.c_str()),m_szMobileName_ori);
	al_GetSettingString(_T("mobile"),_T("IMEI"),const_cast<TCHAR *>(sProfile.c_str()),m_szIMEI);
	if(_tcslen(m_szMobileName)<=0)
		wsprintf(m_szMobileName,m_szMobileName_ori);
	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.
	

	//add resource
	//load exe
//	FindResource(AfxGetResourceHandle(),
/*	TCHAR szText[MAX_PATH];
	al_GetSettingString(_T("public"),_T("ID_BN_NEW"),m_szRes,szText);
	string sText = string("\n") + string(szText);
	HANDLE hUpdateRes = AfxGetResourceHandle();  // update resource handle 
	LPCTSTR lpType = RT_STRING;
	LPTSTR lpName=MAKEINTRESOURCE(ID_BN_NEW);
	WORD wLanguage=MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
	LPCSTR pStr = sText.c_str();
	DWORD  dwSize=sizeof(TCHAR) * sText.size();

	BOOL result = UpdateResource(hUpdateRes,       // update resource handle 
		lpType,                   // change dialog box resource 
		lpName,                  // 
		wLanguage,  // neutral language
		(void *)pStr,                   // ptr to resource info 
		dwSize ); // size of resource info. 
*/

//-----------------------------------------------------------

	//set app title name
	free(const_cast<TCHAR*>(m_pszAppName));
	m_pszAppName = new TCHAR[MAX_PATH];
	al_GetSettingString(_T("public"),_T("IDS_TITLE"),m_szRes,const_cast<TCHAR*>(m_pszAppName));
	CMainFrame* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// create and load the frame with its resources

	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);




	// The one and only window has been initialized, so show and update it.
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPhoneBookApp message handlers



// App command to run the dialog
void CPhoneBookApp::OnAppAbout()
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
// CPhoneBookApp message handlers


int CPhoneBookApp::ExitInstance() 
{
	OleUninitialize();
	
	return CWinApp::ExitInstance();
}


