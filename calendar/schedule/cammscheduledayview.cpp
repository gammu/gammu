// CAMMScheduleDayView.cpp : implementation file
//

#include "stdafx.h"
#include "CAMMScheduleDayView.h"
#include "..\UserExitDlg.h"
//#include "..\CAMMViewController.h"
#include "CAMMScheduleEditorDlg.h"
#include "..\ProcessDlg.h"
#include "..\loadstring.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Added by nono =>
static UINT BASED_CODE indicators[] =
{
	/*
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_OVR,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM
	*/
	ID_SEPARATOR,          // status line indicator
	ID_SEPARATOR,
	ID_SEPARATOR
};

#define Height_of_StatusBar 0
static int static_nMemoRecordCount_ = 0;
static int static_nPhoneRecordCount = 0;

static int _TimeInterval_ = 5;
static int _SkipTimer_ = FALSE;

#define _SystemTrayAppName_ "SystemTrayDemo"
//Added by nono <=

CCAMMScheduleDayView*	CCAMMScheduleDayView::s_pCalendarDayView = NULL;

#define CCAMMScheduleDayView_CLASSNAME    _T("CCAMMScheduleDayView")  // Window class name

/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleDayView

IMPLEMENT_DYNCREATE(CCAMMScheduleDayView, CWnd)

CCAMMScheduleDayView::CCAMMScheduleDayView()
{

	m_MobileCalendarDataList.RemoveAll();
	RegisterWindowClass();

	//CCAMMViewController::m_pScheduleView = this;
	m_Table = NULL;
	m_ScheduleList = NULL;
	m_ClipboardFormatId = ::RegisterClipboardFormat(_T("CF_ORG_SCHEDULE"));
	m_DatePeriod = DATEPERIOD_DAILY;

//	m_pPrintRecord = NULL;//Nono, 2004_0512
//	m_pListAlert = NULL;//Nono, 2004_0612
//	m_nIDTimer = 0;//Nono, 2004_0614

	//Calendar
	m_bMobileConnected = NULL;


	m_nRecordCount = 0;
	m_pScheduleRecord = NULL;

	s_pCalendarDayView = this;

}

CCAMMScheduleDayView::~CCAMMScheduleDayView()
{
//	if (ghSkinResource) 
//   	FreeLibrary(ghSkinResource);


	if (m_pScheduleRecord ) {
    	delete [] m_pScheduleRecord;
		m_pScheduleRecord = NULL;
	}
///
	if (m_Table) {
		delete m_Table;
		m_Table = NULL;
	}
	FreeMobileCalendarDataList();
   

}


BEGIN_MESSAGE_MAP(CCAMMScheduleDayView, CWnd)
	//{{AFX_MSG_MAP(CCAMMScheduleDayView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_DESTROY()
	ON_COMMAND(ID_ORG_SCH_DELETE, OnOrgSchDelete)
	ON_COMMAND(ID_ORG_SCH_ADD, OnOrgSchAdd)
	ON_COMMAND(ID_ORG_SCH_EDIT, OnOrgSchEdit)
/*	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_DELETE, OnUpdateOrgSchDelete)
	ON_COMMAND(ID_ORG_SCH_SAVE, OnOrgSchSave)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_SAVE, OnUpdateOrgSchSave)
	ON_COMMAND(ID_ORG_SCH_SAVEAS, OnOrgSchSaveas)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_SAVEAS, OnUpdateOrgSchSaveas)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_ADD, OnUpdateOrgSchAdd)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_EDIT, OnUpdateOrgSchEdit)
	ON_COMMAND(ID_ORG_SCH_OPEN, OnOrgSchOpen)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_OPEN, OnUpdateOrgSchOpen)
	ON_COMMAND(ID_ORG_SCH_NEW, OnOrgSchNew)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_NEW, OnUpdateOrgSchNew)
	ON_COMMAND(ID_ORG_SCH_CUT, OnOrgSchCut)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_CUT, OnUpdateOrgSchCut)
	ON_COMMAND(ID_ORG_SCH_COPY, OnOrgSchCopy)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_COPY, OnUpdateOrgSchCopy)
	ON_COMMAND(ID_ORG_SCH_PASTE, OnOrgSchPaste)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_PASTE, OnUpdateOrgSchPaste)
*/	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
/*	ON_COMMAND(ID_ORG_SCH_PRINT, OnOrgPrnSchedule)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_PRINT, OnUpdateOrgPrnSchedule)
	ON_COMMAND(ID_ORG_SCH_PRINT_LIST, OnOrgPrnScheduleList)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_PRINT_LIST, OnUpdateOrgPrnScheduleList)

	ON_COMMAND(ID_FILE_PRINT, CWnd::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CWnd::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CWnd::OnFilePrintPreview)
*/	ON_WM_TIMER()
	ON_MESSAGE(WM_LOADCALENDAR, OnLoadCalendar)
	ON_MESSAGE(WM_OPENCALENDAR, OpenCalendar)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleDayView drawing

void CCAMMScheduleDayView::OnDraw(CDC* pDC)
{
	//CDocument* pDoc = GetDocument();

	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleDayView diagnostics

#ifdef _DEBUG
void CCAMMScheduleDayView::AssertValid() const
{
	CWnd::AssertValid();
}

void CCAMMScheduleDayView::Dump(CDumpContext& dc) const
{
	CWnd::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleDayView message handlers

int CCAMMScheduleDayView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	RECT rect;
	GetClientRect(&rect);

	m_Table = new CCAMMScheduleDailyTable(20,80);
	m_Table->Create(CRect(0,0,rect.right - rect.left,rect.bottom - rect.top), this, 1000);
	m_Table->SetClipboardFormatId(m_ClipboardFormatId);

	m_ScheduleList = new CCAMMScheduleList();

	m_ScheduleList->SetScheduleTablePtr(m_Table);
	m_ScheduleList->SetScheduleViewPtr(this);
	//	m_ScheduleList->Create(NULL,"ScheduleList",WS_CHILD | WS_VISIBLE,rect,this,1001);
	m_ScheduleList->Create(NULL,_T("ScheduleList"),WS_CHILD | WS_VISIBLE,rect,this,1001);
	//m_ScheduleList->Create(WS_CHILD | WS_VISIBLE /*| WS_BORDER*/  |LVS_AUTOARRANGE | LVS_REPORT,rect,this,IDC_LIST);
	m_ScheduleList->SetClipboardFormatId(m_ClipboardFormatId);

	//Nono, 2004_0413
	if (!CreateStatusBar())
		return -1;
/*
	// Set up a 60 second timer//Nono, 2004_0612
	m_nIDTimer = 0;
	m_nIDTimer = SetTimer(1, _TimeInterval_*1000, NULL);
	if (!m_nIDTimer)
		AfxMessageBox(AFX_IDS_APP_TITLE);

	//Close the systemtray App.
	char sTitle[30];
	_tcscpy(sTitle, _SystemTrayAppName_);
    CWnd* pCWnd=NULL;
	pCWnd = CWnd::FindWindow(NULL, sTitle);
	if (pCWnd) {
		pCWnd->PostMessage(WM_CLOSE);
	}
*/	

	OnInitialUpdate();

	return 0;
}


void CCAMMScheduleDayView::OnInitialUpdate() 
{
	//CWnd::OnInitialUpdate();
	
	if (!m_Table) {
		return;
	}
	if (!m_ScheduleList) {
		return;
	}


	xGetFileName();
	//Below lines were commented by Sanjeev
	//xInitialize(m_usFileName);
	//End of comment

	//xInitialize(m_usFileName);//Nono, 2004_1026
//	SetSkinResource();//Nono, 2004_1101
    //m_CalendarDataWrapper.LoadandOpenCalendarDriver();

    OpenOrganzerFile(m_usFileName);//Nono, 2004_1101

	//m_Menu.LoadMenu(IDR_ORG_SCH_DAILY);


}

void CCAMMScheduleDayView::OpenCalendar(WPARAM wParam,LPARAM lParam)
{
	//m_CalendarDataWrapper.LoadandOpenCalendarDriver();
/*
	if (!m_bMobileConnected) {
		m_CalendarDataWrapper.CloseCalendarDriver();
		//m_bMobileConnected = false;
	}
*/	m_CalendarDataWrapper.LoadandOpenCalendarDriver();


}
/*
int CCAMMScheduleDayView::xLoadScheduleCallBack() {
	int nErrorCode=0;


	nErrorCode = m_CalendarDataWrapper.LoadCalendarfromMobile(m_pScheduleRecord,m_nRecordCount);


	return nErrorCode;

}

int CCAMMScheduleDayView::LoadScheduleCallBack(int nNeed) {

	s_pCalendarDayView->xLoadScheduleCallBack();

	return 1;
}
*/
CAMMError CCAMMScheduleDayView::LoadScheduleFromFile(CString csFileName) {
	if (!m_Table) {
		return CAMM_ERROR_INITFAILED;
	}

	CAMMError Return = CAMM_ERROR_SUCCESS;

	//CCAMMSync cs;
	int recordCount=0, nErrorCode=0;
	//	SchedulesRecord *pScheduleRecord1;
	//SchedulesRecord *pScheduleRecord = NULL;
/*
	m_nRecordCount = 0;
	m_pScheduleRecord = NULL;
*/
	if ( m_CalendarDataWrapper.CheckMobileSetDateTime() == FALSE)
	{
		CalendarMessageBox(afxGetMainWnd()->m_hWnd,_T("MSG_ERROR_MOBILETIME"));
		return CAMM_ERROR_FAILURE;
	}
/*	m_nRecordCount = recordCount = m_CalendarDataWrapper.GetCalendarCount();
	if (recordCount > 0) 
	{
     	if (m_pScheduleRecord) 
		{
    		delete [] m_pScheduleRecord;
	    	m_pScheduleRecord = NULL;
		}
		m_pScheduleRecord = new SchedulesRecord[recordCount];
		for (int i=0;i<recordCount;i++) 
		{
			m_pScheduleRecord[i].Reset();
		}
		CProcessDlg dlg;

 	   	//Return = m_CalendarDataWrapper.LoadCalendarfromMobile(pScheduleRecord,recordCount,nErrorCode);
     	if(recordCount >= 0)
		{
	    	dlg.SetProgressInfo(m_pScheduleRecord, recordCount,&m_CalendarDataWrapper);
	  //  	dlg.SetProgressInfo(CCAMMScheduleDayView::LoadScheduleCallBack, recordCount);
//	    	dlg.SetLoopCount(recordCount);
            //dlg.SetWindowText("Calendar");
	    	dlg.DoModal();
			recordCount = dlg.m_nGetCount;
		}
	}*/

	FreeMobileCalendarDataList();
	
	if(theApp.m_iMobileCompany == CO_MOTO && (_tcscmp(theApp.m_szPhone,_T("E2")) == 0 ))
	{
		TCHAR szMsg[MAX_PATH*10];
		GetPrivateProfileString(_T("message"),_T("IDS_WARMING_MOTOE2"),_T(""),szMsg,MAX_PATH*10,theApp.m_szRes);
		AfxMessageBox(szMsg);

	}

	CProcessDlg dlg;
	dlg.SetGetMobileDataInfo(&m_CalendarDataWrapper,&m_MobileCalendarDataList);
	dlg.DoModal();

	
	
	if(dlg.m_nGetCount >=0)
	{
		recordCount = dlg.m_nGetCount ;	
		//下面这个函数造成crash   kerm 1.07
		UpdateTableCalendarData(NULL,&m_MobileCalendarDataList);
	}

	if (m_bMobileConnected == TRUE)
	{
		bool bHaveMobileItem = true;
		if(dlg.m_nGetCount <0) bHaveMobileItem = false;

		::SendMessage(afxGetMainWnd()->GetSafeHwnd(), WM_CHECK_LEFTTREEITEM, (DWORD)bHaveMobileItem, NULL);
		if(dlg.m_nGetCount >=0)
		{
    		g_numTotalCalendars = recordCount;
			::PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_REFRESH_LEFT_PANEL, (DWORD)true, NULL);
		}
	}
/*	
	if (recordCount > 0) 
	{
     	if (m_pScheduleRecord) 
		{
    		delete [] m_pScheduleRecord;
	    	m_pScheduleRecord = NULL;
		}
		m_pScheduleRecord = new SchedulesRecord[recordCount];
		for (int i=0;i<recordCount;i++) 
		{
			m_pScheduleRecord[i].Reset();
		}
		int index =0;
		POSITION pos = m_MobileCalendarDataList.GetHeadPosition();
		while(pos && index < recordCount)
		{
			SchedulesRecord *pData =(SchedulesRecord *) m_MobileCalendarDataList.GetNext(pos);
			if(pData)
			{
				m_pScheduleRecord[index].SetRecord(pData);
				index++;
			}

		}
	}
	//REFRESH LEFT_PANEL to change the totla number of calendars displayed on left view.
	if (m_bMobileConnected == TRUE) {
    	g_numTotalCalendars = recordCount;
        ::PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_REFRESH_LEFT_PANEL, (DWORD)true, NULL);
	}
	//
//	if (Return == CAMM_ERROR_SUCCESS) {
		if (m_Table && m_pScheduleRecord)
    		m_Table->SetScheduleData(m_pScheduleRecord,recordCount);
		xInitialize(_T(""));
//	}else {
//		AfxMessageBox("Failed to adding a new Calendar!\nPlease check the satus of connecting mobile.");
//	}

/*
	CCAMMSync cs;
	int recordCount=0;
	//	SchedulesRecord *pScheduleRecord1;
	SchedulesRecord *pScheduleRecord = NULL;


	m_usFileName = csFileName;
    xGetFilePath(m_usFilePath,csFileName);

	cs.GetRecordCount(MO_SCH,csFileName,recordCount);
	if (recordCount > 0) {
		pScheduleRecord = new SchedulesRecord[recordCount];
		for (int i=0;i<recordCount;i++) {
			pScheduleRecord[i].Reset();
		}
		Return = cs.LoadSchedule(pScheduleRecord,csFileName,recordCount);
	}
	if (Return == CAMM_ERROR_SUCCESS) {
		m_Table->SetScheduleData(pScheduleRecord,recordCount);
	}

	if (pScheduleRecord) {
		delete [] pScheduleRecord;
		pScheduleRecord = NULL;
	}

	//Nono, 2004_0612
	int nNumberOfAlerts = RescanAlerts();
*/

	return Return;
}

void CCAMMScheduleDayView::GetScheduleRecordCount(int &cnt) {
	if (m_Table) {
		m_Table->GetRecordCount(cnt);
	} else {
		cnt = 0;
	}
}

void CCAMMScheduleDayView::GetScheduleRecord(SchedulesRecord *psr,int &cnt) {
	if (!psr) {
		return;
	}
	if (cnt > 0) {
		if (m_Table)
		    m_Table->GetScheduleData(psr,cnt);
	}
}

CAMMError CCAMMScheduleDayView::SaveScheduleToFile(CString csFileName) {
	if (!m_Table) {
		return CAMM_ERROR_INITFAILED;
	}

	CAMMError Return = CAMM_ERROR_SUCCESS;

/*	CCAMMSync cs;
	int recordCount=0;
	SchedulesRecord *pScheduleRecord = NULL;

	m_usFileName = csFileName;
	xGetFilePath(m_usFilePath,m_usFileName);

	m_Table->GetRecordCount(recordCount);
	if (recordCount > 0) {
		pScheduleRecord = new SchedulesRecord[recordCount];
		for (int i=0;i<recordCount;i++) {
			pScheduleRecord[i].Reset();
		}
		m_Table->GetScheduleData(pScheduleRecord,recordCount);
	}
	CAMMError Return = cs.SaveSchedule(pScheduleRecord,csFileName,recordCount);

	if (Return == CAMM_ERROR_SUCCESS) {
		m_Table->SetDirtyFlag(FALSE);
	}
	if (pScheduleRecord) {
		delete [] pScheduleRecord;
		pScheduleRecord = NULL;
	}
*/

	return Return;
}

void CCAMMScheduleDayView::xGetFileName() 
{
	TCHAR path[MAX_PATH+1];
	CString strPath;
	memset(path,0x00,sizeof(TCHAR)*(MAX_PATH+1));

	DWORD type =0;
	DWORD size=MAX_PATH;
	BOOL bKeyExist = FALSE;
/*	HKEY hKey;
	TCHAR myScheduleFolder[MAX_PATH]="FolderSettings";
	if(::RegOpenKeyEx(HKEY_LOCAL_MACHINE,gstrRegistryPath,NULL,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
	{
		if(::RegOpenKeyEx(hKey,myScheduleFolder,NULL,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS)
		{
			if(::RegQueryValueEx(hKey,LPCTSTR("My Organizer"),NULL,&type,(LPBYTE)path,&size) == ERROR_SUCCESS)
			{
				bKeyExist = TRUE;
			}
			::RegCloseKey(hKey);
		}
	}

	if(bKeyExist)
	{
		strPath.Format(path);
	}
	else
	{
		//		AfxMessageBox("no file");
	}
*/	//This must be changed to CAMMSkin.dll for loading based on OS language
	strPath+="\\";
	m_usFilePath = strPath;
	strPath += "Default.iog";

	m_usFileName = strPath;

}
void CCAMMScheduleDayView::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	RECT rect;
	GetClientRect(&rect);
	//Modified by Nono=>
	if (m_Table)
    	m_Table->SetWindowPos(NULL,0,0,cx, cy-Height_of_StatusBar,SWP_NOMOVE);
    //	m_Table->SetWindowPos(NULL,0,0,rect.right - rect.left,rect.bottom - rect.top-Height_of_StatusBar,SWP_NOMOVE);
	if (m_ScheduleList)
     	m_ScheduleList->SetWindowPos(NULL,0,0,cx, cy-Height_of_StatusBar,SWP_NOMOVE);
   //  	m_ScheduleList->SetWindowPos(NULL,0,0,rect.right - rect.left,rect.bottom - rect.top-Height_of_StatusBar,SWP_NOMOVE);
	//Modified by Nono<=

	//Added by Nono=>
	m_wndStatusBar.SetWindowPos(NULL, 0, rect.bottom-rect.top-Height_of_StatusBar, rect.right-rect.left, Height_of_StatusBar, SWP_SHOWWINDOW);
	//Added by Nono<=
}

void CCAMMScheduleDayView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	//	AfxMessageBox("TEST");
	CWnd::OnLButtonDown(nFlags, point);
}

void CCAMMScheduleDayView::ChangeDatePeriod(Schedule_DatePeriod sd,BOOL bUpdateData) 
{
	if(m_Table && !bUpdateData)
	{
		if(m_Table->IsWindowVisible())
		{
			if(m_Table->m_DatePeriod == sd) 
			{
				 m_Table->SetActiveWindow();
				return;
			}
		}
		else
		{
			if(sd == DATEPERIOD_LIST)
			{
				return;
			}
		}
	}

	if (sd == DATEPERIOD_LIST) {
		if (m_Table)
    		m_Table->ShowWindow(SW_HIDE);
		if (m_ScheduleList) {
		    m_ScheduleList->Initialize();
	     	m_ScheduleList->ShowWindow(SW_SHOW);
		}
	} else {

		if (m_Table) {
	    	m_Table->ChangeDatePeriod(sd);
	    	m_Table->ShowWindow(SW_SHOW);
            m_Table->SetActiveWindow();
		}
		if (m_ScheduleList)
     		m_ScheduleList->ShowWindow(SW_HIDE);
	}
	m_DatePeriod = sd;
}
void CCAMMScheduleDayView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	//	AfxMessageBox("TEST DBCLK");

	CWnd::OnLButtonDblClk(nFlags, point);
}

void CCAMMScheduleDayView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CWnd::OnRButtonDown(nFlags, point);
}

void CCAMMScheduleDayView::ShowScheduleMenu(POINT ptAction) {
	CMenu *pbMenu = m_Menu.GetSubMenu(0);
	ASSERT(pbMenu);
	
	int no=pbMenu->GetMenuItemCount();
	if (pbMenu != NULL && pbMenu->GetMenuItemCount() > 0)
	
	DrawMenuBar();
	
	pbMenu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,ptAction.x,ptAction.y,afxGetMainWnd(),0);
}

BOOLEAN CCAMMScheduleDayView::UserExiting()
{
	if (!IsDataDirty()) {
		return TRUE;
	}

	CUserExitDlg ueDlg(this);
	ueDlg.DoModal();
	INT nDecision = ueDlg.GetDecision();
	BOOLEAN bRet = FALSE;
	switch (nDecision) {
	case IDC_PVI_5_UE_SAVE:
		xSave();
		bRet = TRUE;
		break;
	case IDC_PVI_5_UE_SAVEAS:
		if (xSaveAs() == CAMM_ERROR_SUCCESS) 
			bRet = TRUE;
		else 
			bRet = FALSE;  //When save as but cancel for file selection
		break;
	case IDC_PVI_5_UE_DISCARD:
		LoadScheduleFromFile(m_usFileName);
		if (m_Table)
    		m_Table->CreateTable();
		bRet = TRUE;
		break;
	case IDC_PVI_5_UE_CANCEL:
		bRet = FALSE;
		break;
	}


	return bRet;
}

CAMMError CCAMMScheduleDayView::xSave()
{

	if (m_usFileName.IsEmpty()) {
		return xSaveAs();
	} else {
		return xSave(m_usFileName);
	}

}

CAMMError CCAMMScheduleDayView::xGetFilePath(CString &szFilePath,CString csDBPath)
{
	if (csDBPath.IsEmpty()) {
		return CAMM_ERROR_NONE;
	}
	int pos=0,pos2=0;
	TCHAR fol ='\\';

	pos2 = csDBPath.Find(fol);
	if (pos2 <= 0) {
		//		szFilePath = _T("");
		return CAMM_ERROR_NONE;
	}
	while (pos2 > 0) {
		pos = pos2;
		pos2 = csDBPath.Find(fol,pos+1);
	}

	szFilePath = csDBPath.Left(pos+1);

	return CAMM_ERROR_SUCCESS;
}


CAMMError CCAMMScheduleDayView::xSaveAs()
{

	CAMMError Ret = CAMM_ERROR_FAILURE;
/*	char szFilters[]=
      "Orgnizer Files (*.iog)\0*.iog\0";

	CString str = (LPCSTR)m_usFileName;
	TCHAR szFile[MAX_PATH];
	memset(szFile,0x00,sizeof(TCHAR)*MAX_PATH);
	if (!str.IsEmpty()) {
		CString io4 = "io4";
		if (str.Right(3).CompareNoCase((LPCTSTR)io4) == 0) {
			str.Replace(".io4",".iog");
		}
		memcpy(szFile,str.GetBuffer(str.GetLength()),str.GetLength());
	}

	OPENFILENAME ofn;
	memset (&ofn, 0, sizeof (ofn));
	ofn.lStructSize = sizeof(OPENFILENAME); 
	//	ofn.hwndOwner = GetSafeHwnd(); 
    ofn.hwndOwner = afxGetMainWnd()->GetSafeHwnd(); 
	ofn.lpstrFilter = szFilters; 
	ofn.lpstrFile= szFile; 
	ofn.nMaxFile = MAX_PATH; 
	ofn.lpstrDefExt = ".iog";
	//	ofn.lpstrFileTitle = (LPSTR)NULL; 
	//	ofn.nMaxFileTitle = sizeof(szFileTitle); 
	ofn.lpstrInitialDir = m_usFilePath; 
	ofn.Flags = OFN_OVERWRITEPROMPT; 
	//	ofn.lpstrTitle = TEXT("Save a File"); 
 	ofn.nFilterIndex = 1;

	// Display the Filename common dialog box.
 	if (GetSaveFileName(&ofn)) {
		m_usFileName = ofn.lpstrFile;
		m_usFileName.MakeLower();
		xGetFilePath(m_usFilePath,m_usFileName);
		//		Ret = xSave(m_usFileName);
		g_usTitle = m_usFileName;
		g_usTitle+=" - ";
		g_usTitle+=g_usAppName;
		afxGetMainWnd()->SetWindowText(g_usTitle);
		if (CCAMMViewController::Save(m_usFileName)) {
			return CAMM_ERROR_SUCCESS;
		}
		//		AfxMessageBox(m_usFileName);
	}
	SetAllRedraw();
	m_Table->Invalidate(TRUE);
	m_Table->UpdateWindow();
*/	return Ret;

	//	CFileDialog fileDlg (FALSE, "iog", "*.iog",
	//      OFN_HIDEREADONLY, szFilters, this);
	//   
	//   // Display the file dialog. When user clicks OK, fileDlg.DoModal() 
	//   // returns IDOK.
	//	if( fileDlg.DoModal ()==IDOK )
	//	{
	//		CString pathName = fileDlg.GetPathName();
	//		m_usFileName = pathName;
	//		Ret = xSave(pathName);
	//	}
	//
	//
	//
	//	return Ret;
}

CAMMError CCAMMScheduleDayView::xSave(CString usFileName)
{

	SaveScheduleToFile(usFileName);


	return CAMM_ERROR_SUCCESS;
}

void CCAMMScheduleDayView::OnDestroy() 
{
	//Invoke the systemtray App.
/*	char sTitle[30];
	_tcscpy(sTitle, _SystemTrayAppName_);
    CWnd* pCWnd=NULL;
	pCWnd = CWnd::FindWindow(NULL, sTitle);
	CString AppName;
	AppName.Format("%s.exe", _SystemTrayAppName_);
	if (!pCWnd) {
    	WinExec(AppName.GetBuffer(20), SW_HIDE);
	}
	
*/
	if (m_ScheduleList) {
		m_ScheduleList->DeInitialize();
		m_ScheduleList->DestroyWindow();
	}
	
	m_Menu.DestroyMenu();
/*
		if (m_ScheduleList) {
	    	m_ScheduleList->DeInitialize();
			delete m_ScheduleList;
			m_ScheduleList = NULL;
		}
*/	if (m_Table) {
		delete m_Table;
		m_Table = NULL;
	}
	m_CalendarDataWrapper.CloseCalendarDriver();
	CWnd::OnDestroy();	

}

BOOL CCAMMScheduleDayView::IsDataDirty()
{
	BOOL bRet=false;
	if (m_Table)
    	bRet = m_Table->IsDataDirty();
	return bRet;
}

CAMMError CCAMMScheduleDayView::OpenOrganzerFile(CString csFilename)
{
	SetFileName(csFilename);
	return xInitialize(csFilename);
}

CAMMError CCAMMScheduleDayView::xInitialize(CString csFilename)
{
	//Daily
	CAMMError Return = CAMM_ERROR_FAILURE;

	//OpenCalendar();

	//Return = LoadScheduleFromFile(csFilename);

	//	m_Table->SetDirtyFlag(FALSE);
	//	m_Table->SetInfoRowCnt(2);
	//	m_Table->SetHalfDayString("AM","PM");
	//	m_Table->SetDatePeriod(DATEPERIOD_DAILY);
	//	m_Table->CreateTable();

	ChangeDatePeriod(m_DatePeriod);

   	//Added by Nono => 2004/0415
//   	SetInfomationonStatusBar();


	return Return;
}

void CCAMMScheduleDayView::OnOrgSchDelete() 
{
	// TODO: Add your command handler code here
	if (m_DatePeriod == DATEPERIOD_LIST) {
		if (m_ScheduleList)
	    	m_ScheduleList->OnOrgSchDelete();
	} else {
		if (m_Table)
	    	m_Table->OnOrgSchDelete();
	}
}

void CCAMMScheduleDayView::OnOrgSchSave() 
{
	// TODO: Add your command handler code here
	//	xSave();
//	CCAMMViewController::Save(m_usFileName);
}

void CCAMMScheduleDayView::OnUpdateOrgSchSave(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
}

void CCAMMScheduleDayView::OnOrgSchSaveas() 
{
	// TODO: Add your command handler code here
	xSaveAs();
}

void CCAMMScheduleDayView::OnUpdateOrgSchSaveas(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	//	pCmdUI->Enable(IsDataDirty());	
	pCmdUI->Enable(TRUE);	
	CString txt = LoadStringFromFile(_T("public"),_T("SaveAs"));
	ASSERT(&txt);
	pCmdUI->SetText(txt);
}


void CCAMMScheduleDayView::OnOrgSchAdd() 
{
	// TODO: Add your command handler code here
	if (m_DatePeriod == DATEPERIOD_LIST) 
	{
		if (m_ScheduleList)
	    	m_ScheduleList->OnOrgSchAdd();
	} 
	else 
	{
		if (m_Table)
	    	m_Table->OnOrgSchAdd();
	}

}

void CCAMMScheduleDayView::OnUpdateOrgSchAdd(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if (m_DatePeriod == DATEPERIOD_LIST) {
		if (m_ScheduleList)
	    	m_ScheduleList->OnUpdateOrgSchAdd(pCmdUI);
	} else {
		if (m_Table)
    		m_Table->OnUpdateOrgSchAdd(pCmdUI);
	}
	CString txt = LoadStringFromFile(_T("public"),"Add");
	ASSERT(&txt);
	pCmdUI->SetText(txt);

}

void CCAMMScheduleDayView::OnOrgSchEdit() 
{
	// TODO: Add your command handler code here
	if (m_DatePeriod == DATEPERIOD_LIST) {
		if (m_ScheduleList)
	    	m_ScheduleList->OnOrgSchEdit();
	} else {
		if (m_Table)
	     	m_Table->OnOrgSchEdit();
	}
}

void CCAMMScheduleDayView::OnUpdateOrgSchEdit(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if (m_DatePeriod == DATEPERIOD_LIST) {
		if (m_ScheduleList)
	     	m_ScheduleList->OnUpdateOrgSchEdit(pCmdUI);
	} else {
		if (m_Table)
	     	m_Table->OnUpdateOrgSchEdit(pCmdUI);
	}
	CString txt = LoadStringFromFile(_T("public"),_T("Edit"));
	ASSERT(&txt);
	pCmdUI->SetText(txt);
}

void CCAMMScheduleDayView::OnOrgSchOpen() 
{
	CAMMError Ret = CAMM_ERROR_FAILURE;
	if (IsDataDirty()) {
		if (!UserExiting()) {
	    	if (m_Table)
		    	m_Table->xInvalidate(TRUE);
			return;
		}
	}
	TCHAR szFilters[]=
      _T("Orgnizer Files (*.iog;*.io4)\0*.iog;*.io4\0");

	CString str = (LPCTSTR)m_usFileName;
	TCHAR szFile[MAX_PATH];
	memset(szFile,0x00,sizeof(TCHAR)*MAX_PATH);
	memcpy(szFile,str.GetBuffer(str.GetLength()),str.GetLength());

	OPENFILENAME ofn;
	memset (&ofn, 0, sizeof (ofn));
	ofn.lStructSize = sizeof(OPENFILENAME); 
	//	ofn.hwndOwner = GetSafeHwnd(); 
    ofn.hwndOwner = afxGetMainWnd()->GetSafeHwnd(); 
	ofn.lpstrFilter = szFilters; 
	ofn.lpstrFile= szFile; 
	ofn.nMaxFile = MAX_PATH; 
	ofn.lpstrDefExt = _T(".iog");
	//	ofn.lpstrFileTitle = (LPSTR)NULL; 
	//	ofn.nMaxFileTitle = sizeof(szFileTitle); 
	ofn.lpstrInitialDir = m_usFilePath; 
	ofn.Flags = OFN_FILEMUSTEXIST; 
	//	ofn.lpstrTitle = TEXT("Save a File"); 
 	ofn.nFilterIndex = 1;
/*
	// Display the Filename common dialog box.
 	if (GetOpenFileName(&ofn)) {
		m_Table->SetDirtyFlag(FALSE);
		m_usFileName = ofn.lpstrFile;
		//eagle120903*: Show file path on App title
		gpIEShellListCtrl->m_selFilePath = m_usFileName;
		g_usTitle = m_usFileName + " - " + g_usAppName;
		afxGetMainWnd()->SetWindowText(g_usTitle);
		//eagle120903&		
		xGetFilePath(m_usFilePath,m_usFileName);
		CCAMMViewController::OpenOrganzerFile(CCAMMViewController::Org_Schedule_View,m_usFileName);
		//		Ret = LoadScheduleFromFile(m_usFileName);
		//		m_Table->CreateTable();
	}
*/
}

void CCAMMScheduleDayView::SetFileName(CString usFileName) {
	m_usFileName = usFileName;
	xGetFilePath(m_usFilePath,m_usFileName);
}

void CCAMMScheduleDayView::OnUpdateOrgSchOpen(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TRUE);	
	CString txt = LoadStringFromFile(_T("public"),_T("Open"));
	ASSERT(&txt);
	pCmdUI->SetText(txt);
}

void CCAMMScheduleDayView::OnOrgSchNew() 
{
	CAMMError Ret = CAMM_ERROR_FAILURE;
/*2004_1021
	if (!CCAMMViewController::UserExiting()) {
		m_Table->xInvalidate(TRUE);
		return;
	}
	CCAMMViewController::OpenOrganzerFile(CCAMMViewController::Org_Schedule_View,_T(""));
*/		//	if (IsDataDirty()) {
		//		if (!UserExiting()) {
		//			return;
		//		}
		//	}
	//	m_Table->SetDirtyFlag(FALSE);
	//	m_usFileName = _T("");
	//	Ret = LoadScheduleFromFile(m_usFileName);
	//	m_Table->CreateTable();
}

void CCAMMScheduleDayView::OnUpdateOrgSchNew(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TRUE);	
	CString txt = LoadStringFromFile(_T("public"),_T("New"));
	ASSERT(&txt);
	pCmdUI->SetText(txt);
}

void CCAMMScheduleDayView::Reset() {
	LoadScheduleFromFile(m_usFileName);
	if (m_DatePeriod == DATEPERIOD_LIST) {
		if (m_ScheduleList)
    		m_ScheduleList->ShowWindow(TRUE);
	} else {
		if (m_Table)
	    	m_Table->CreateTable();
	}
	m_Table->SetDirtyFlag(FALSE);
}

void CCAMMScheduleDayView::OnOrgSchCut() 
{
	if (m_DatePeriod == DATEPERIOD_LIST) {
		if (m_ScheduleList)
    		m_ScheduleList->OnOrgSchCut();
	} else {
		if (m_Table)
	    	m_Table->OnOrgSchCut();
	}
}

void CCAMMScheduleDayView::OnUpdateOrgSchCut(CCmdUI* pCmdUI) 
{
	if (m_DatePeriod == DATEPERIOD_LIST) {
		if (m_ScheduleList)
    		m_ScheduleList->OnUpdateOrgSchCut(pCmdUI);
	} else {
		if (m_Table)
	    	m_Table->OnUpdateOrgSchCut(pCmdUI);
	}
}

void CCAMMScheduleDayView::OnOrgSchCopy() 
{
	if (m_DatePeriod == DATEPERIOD_LIST) {
		if (m_ScheduleList)
	    	m_ScheduleList->OnOrgSchCopy();
	} else {
		if (m_Table)
	    	m_Table->OnOrgSchCopy();
	}
}

void CCAMMScheduleDayView::OnUpdateOrgSchCopy(CCmdUI* pCmdUI) 
{
	if (m_DatePeriod == DATEPERIOD_LIST) {
		if (m_ScheduleList)
     		m_ScheduleList->OnUpdateOrgSchCopy(pCmdUI);
	} else {
		if (m_Table)
    		m_Table->OnUpdateOrgSchCopy(pCmdUI);
	}
}

void CCAMMScheduleDayView::OnOrgSchPaste() 
{
	if (m_DatePeriod == DATEPERIOD_LIST) {
		if (m_ScheduleList)
     		m_ScheduleList->OnOrgSchPaste();
	} else {
		if (m_Table)
    		m_Table->OnOrgSchPaste();
	}
}

void CCAMMScheduleDayView::OnUpdateOrgSchPaste(CCmdUI* pCmdUI) 
{
	if (m_DatePeriod == DATEPERIOD_LIST) {
		if (m_ScheduleList)
    		m_ScheduleList->OnUpdateOrgSchPaste(pCmdUI);
	} else {
		if (m_Table)
    		m_Table->OnUpdateOrgSchPaste(pCmdUI);
	}
}

BOOL CCAMMScheduleDayView::PreTranslateMessage(MSG* pMsg) 
{
	BOOL ret = FALSE;
	if((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN))
	{
		if ((GetKeyState(VK_CONTROL) < 0) && pMsg->wParam == 'X')		// Ctrl+X , Cut
		{
			OnOrgSchCut();
			ret = TRUE;
		}
		else if ((GetKeyState(VK_CONTROL) < 0) && pMsg->wParam == 'C')		// Ctrl+C , Copy
		{
			OnOrgSchCopy();
			ret = TRUE;
		}
		else if ((GetKeyState(VK_CONTROL) < 0) && pMsg->wParam == 'V')		// Ctrl+V , Paste
		{
			OnOrgSchPaste();
			ret = TRUE;
		}
		else 
		{
			ret = FALSE;
		}
	}
 
	
	if (ret) {
    	//Added by Nono => 2004/0415
      	SetInfomationonStatusBar();
		return ret;
	}
	
	return CWnd::PreTranslateMessage(pMsg);
}

void CCAMMScheduleDayView::xPostMessage(UINT msg,WPARAM wparam,LPARAM lparam) 
{
	if (m_DatePeriod == DATEPERIOD_LIST) {
		if (m_ScheduleList)
    		m_ScheduleList->PostMessage(msg,wparam,lparam);
	} else {
		if (m_Table)
	    	m_Table->PostMessage(msg,wparam,lparam);
	}
}

void CCAMMScheduleDayView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (m_DatePeriod == DATEPERIOD_LIST) {
		if (m_ScheduleList)
     		m_ScheduleList->OnKeyDown(nChar,nRepCnt,nFlags);
	} else {
		if (m_Table)
    		m_Table->OnKeyDown(nChar,nRepCnt,nFlags);
	}
	
    //CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CCAMMScheduleDayView::GotoOnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	OnKeyDown(nChar, nRepCnt, nFlags);
}

void CCAMMScheduleDayView::SetAllUnDraw() 
{
	if (m_DatePeriod == DATEPERIOD_LIST) {
	} else {
		if (m_Table)
    		m_Table->SetAllUnDraw();
	}
	
	//	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
void CCAMMScheduleDayView::SetAllRedraw() 
{
	if (m_DatePeriod == DATEPERIOD_LIST) {
	} else {
		if (m_Table)
    		m_Table->SetAllRedraw();
	}
	
	//	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
BOOL CCAMMScheduleDayView::GetDataSectionFlag()
{
	if (m_DatePeriod == DATEPERIOD_LIST) 
		return m_ScheduleList->GetDataSectionFlag();
	else 
		return m_Table->xCheckIsDataSelected();		
}

UINT CCAMMScheduleDayView::GetClipboardFormatID()
{
	if (m_DatePeriod == DATEPERIOD_LIST) 
		return m_ScheduleList->GetClipboardFormatID();
	else 
		return m_Table->GetClipboardFormatID();		
}

//Added by Nono=>
BOOL CCAMMScheduleDayView::CreateStatusBar()
{
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return FALSE;       // fail to create
	}

	UINT nID, nStyle;
	int cxWidth;

	m_wndStatusBar.GetPaneInfo( 0, nID, nStyle, cxWidth);
	m_wndStatusBar.SetPaneInfo( 0, nID, SBPS_STRETCH|SBPS_NORMAL, cxWidth);

	return TRUE;
}

BOOL CCAMMScheduleDayView::SetInfomationonStatusBar(void)
{
	//Set the information of "Contact", "Schedule" & "Memorandum".[Nono, 2004_0413]
	int nScheduleRecordCount = 0;
	CString str;
	return 0;
}



void CCAMMScheduleDayView::OnTimer(UINT nIDEvent)
{

/*	if (!_SkipTimer_) {
		CheckAllAlerts();
	}
*/

	CWnd::OnTimer(nIDEvent);
}


/*
BOOL CCAMMScheduleDayView::SetSkinResource()
{
		TCHAR path[MAX_PATH+1];
		memset(path,0x00,sizeof(TCHAR)*(MAX_PATH+1));

		al_GetModulePath(AfxGetInstanceHandle(), path);
		CString strPath(_T(""));// = path;

		int nLen = strlen(path);
		memcpy(gstrImageLocation, path, nLen);
		CString csTemp(_T(""));

		switch(gLocaleInfo)
		{
			case liEnglish:
				//strcat(gstrImageLocation,ENG_SKIN); 
				csTemp = gstrImageLocation;
				csTemp+="CAMMSkinEng.dll";
				break;
			case liTC:
				//strcat(gstrImageLocation,TC_SKIN); 
				csTemp = gstrImageLocation;
				csTemp+="CAMMSkinTC.dll";
				break;
			case liSC:
				//strcat(gstrImageLocation,SC_SKIN); 
				csTemp = gstrImageLocation;
				csTemp+="CAMMSkinSC.dll";
				break;
			default:
				//strcat(gstrImageLocation,ENG_SKIN); 
				csTemp = gstrImageLocation;
				csTemp+="CAMMSkinEng.dll";
		}

		csTemp.Replace("\\\\", "\\");
		strPath = csTemp;

		ghSkinResource = ::LoadLibrary(strPath);
		if(ghSkinResource == NULL)
		{
			CString csPath;
			csPath += "CMainFrame::Init LoadLibrary Failed\n";
			csPath += " => SkinPath: [ ";
			csPath += strPath;
			csPath += " ]";
			//Nono => To avoid running a APP without correct resource.[2004_0701]
			csPath += "\n=>Failed to load the CAMMSkinxxx.dll. [CMainFrame::Init LoadLibrary Failed]";
			AfxMessageBox(csPath);
	        return FALSE;
			//Nono <= 
		}

	       return TRUE;
}*/

void CCAMMScheduleDayView::OnLoadCalendar(WPARAM wParam,LPARAM lParam)
{
	CAMMError Return;


	Return = LoadScheduleFromFile(_T(""));

	return;
}

BOOL CCAMMScheduleDayView::RegisterWindowClass()
{
    WNDCLASS wndcls;
    HINSTANCE hInst = AfxGetInstanceHandle();

    if (!(::GetClassInfo(hInst, CCAMMScheduleDayView_CLASSNAME, &wndcls)))
    {
        // otherwise we need to register a new class
        wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wndcls.lpfnWndProc      = ::DefWindowProc;
        wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
        wndcls.hInstance        = hInst;
        wndcls.hIcon            = NULL;
        wndcls.hCursor          = afxGetApp()->LoadStandardCursor(IDC_ARROW);
        wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wndcls.lpszMenuName     = NULL;
        wndcls.lpszClassName    = CCAMMScheduleDayView_CLASSNAME;

        if (!AfxRegisterClass(&wndcls))
        {
            AfxThrowResourceException();
            return FALSE;
        }
    }

    return TRUE;
}

void CCAMMScheduleDayView::ClearTable()
{
	int recordCount = 0;
	if (m_Table) {
     	if (m_pScheduleRecord) {
    		delete [] m_pScheduleRecord;
	    	m_pScheduleRecord = NULL;
		}
    	m_Table->SetScheduleData(m_pScheduleRecord,recordCount);
		xInitialize(_T(""));
	}
	//REFRESH LEFT_PANEL to change the totla number of calendars displayed on left view.
   	g_numTotalCalendars = recordCount;
    ::PostMessage(afxGetMainWnd()->GetSafeHwnd(), WM_REFRESH_LEFT_PANEL, (DWORD)true, NULL);
    //OnInitialUpdate();
}

void CCAMMScheduleDayView::FreeMobileCalendarDataList()
{
 	POSITION pos = m_MobileCalendarDataList.GetHeadPosition();
	while(pos )
	{
		SchedulesRecord *pData =(SchedulesRecord *) m_MobileCalendarDataList.GetNext(pos);
		if(pData) delete pData;
	}
	m_MobileCalendarDataList.RemoveAll();

}

void CCAMMScheduleDayView::UpdateTableCalendarData(CPtrList* pOldData ,CPtrList *pDataList)
{
	if(pOldData)
	{
		int nOldCount = 0 ;
		GetScheduleRecordCount(nOldCount);
 		POSITION pos = pOldData->GetHeadPosition();
		while(pos )
		{
			SchedulesRecord *pData =(SchedulesRecord *) pOldData->GetNext(pos);
			if(pData) delete pData;
		}
		pOldData->RemoveAll();

		SchedulesRecord *pScheduleRecord= new SchedulesRecord[nOldCount];
		if(pScheduleRecord)
		{
			for (int i=0;i<nOldCount;i++) 
				pScheduleRecord[i].Reset();

			GetScheduleRecord(pScheduleRecord,nOldCount);
			for(i= 0 ; i <nOldCount; i++)
			{
				SchedulesRecord *pData = new SchedulesRecord;
				pData->SetRecord(&pScheduleRecord[i]);
				pOldData->AddTail(pData);
			}
			delete [] pScheduleRecord;
			pScheduleRecord = NULL;
		}

	}
	if(pDataList)
	{
		int recordCount = pDataList->GetCount();
		if (recordCount > 0) 
		{
     		if (m_pScheduleRecord) 
			{
    			delete [] m_pScheduleRecord;
	    		m_pScheduleRecord = NULL;
			}
			m_pScheduleRecord = new SchedulesRecord[recordCount];
			for (int i=0;i<recordCount;i++) 
			{
				m_pScheduleRecord[i].Reset();
			}
			int index =0;
			POSITION pos = pDataList->GetHeadPosition();
			while(pos && index < recordCount)
			{
				SchedulesRecord *pData =(SchedulesRecord *) pDataList->GetNext(pos);
				if(pData)
				{
					m_pScheduleRecord[index].SetRecord(pData);
					index++;
				}

			}
			
		}
		
		if (m_Table && m_pScheduleRecord)
    		m_Table->SetScheduleData(m_pScheduleRecord,recordCount);
		xInitialize(_T(""));
		
	}

}
