// PrevView.cpp : implementation file
//

#include "stdafx.h"
#include "Calendar.h"
#include "PrevView.h"
#include "loadstring.h"
//#include "databasedlg.h"
//#include "databasedlg2.h"
//#include "DriverWrapper.h"
//#include "include\CSVHandle.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Test Calendar
//
#include "ConvertEngine.h"


/////////////////////////////////////////////////////////////////////////////
// CPrevView

IMPLEMENT_DYNCREATE(CPrevView, CView)

CPrevView::CPrevView()
{
	m_pSearchDlg = NULL;
	m_LeftMode.sMode = MOBILE_PHONE;
	m_crItemBgNor = RGB(0,0,0);
	m_crItemBgSel = RGB(0,0,0);
	m_crItemBgFocus = RGB(0,0,0);
	m_crItemTextNor = RGB(0,0,0);
	m_crItemTextSel = RGB(0,0,0);
	m_crItemTextFocus = RGB(0,0,0);
	m_crCol1 = 0;
	m_crCol2 = 0;

	//Calendar
	m_bOpened = false;
	hMobileDll = NULL;
}

CPrevView::~CPrevView()
{
	//close driver
//	m_Driver.Close();
//	m_Driver.FreeDriver();

	//FreeCalendarDriver();

	//m_CalendarDriverWrapper.CloseCalendarDriver();
}


BEGIN_MESSAGE_MAP(CPrevView, CView)
	//{{AFX_MSG_MAP(CPrevView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST, OnBeginDrag)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, OnCustomDraw)
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SEARCH_STR,OnSearchDlgStr)
	ON_MESSAGE(WM_VIEWMODE_CHANGE,OnViewModeChange)
	ON_MESSAGE(WM_TOOLBAR_DELSEL,OnToolbarDelSel)
	ON_MESSAGE(WM_TOOLBAR_SEARCH,OnToolbarSearch)
	ON_MESSAGE(WM_TOOLBAR_NEWEDIT,OnToolbarNewEdit)
	ON_MESSAGE(WM_TOOLBAR_RELOAD,OnToolbarReload)
	ON_MESSAGE(WM_TOOLBAR_BACKUP,OnToolbarBackup)
	ON_MESSAGE(WM_TOOLBAR_RESTORE,OnToolbarRestore)
	ON_MESSAGE(WM_SEARCH_END,OnSearchDlgEnd)
	ON_MESSAGE(WM_CARDPANEL_GETGROUP,OnCardPanelGetGroup)
	ON_MESSAGE(WM_CARDPANEL_GROUPCOUNT,OnCardPanelGroupCount)
	ON_MESSAGE(WM_LEFT_CHGMODE,OnLeftChange)
	ON_MESSAGE(WM_LEFT_DROPDATA,OnLeftDropData)
	ON_MESSAGE(WM_LOADMOBILE,OnLoadMobile)
	ON_MESSAGE(WM_OPENDRIVER,OnOpenDriver)
	ON_MESSAGE(WM_LOADCALENDAR, OnLoadCalendar)
	ON_MESSAGE(WM_OPENCALENDAR, OnOpenCalendar)
	ON_MESSAGE(WM_SWITHTODAYVIEW, OnSwitchToDayView)
	ON_MESSAGE(WM_LEFT_DROPDATATOMSOT,OnLeftDropData2MSOT)
	ON_MESSAGE(WM_TOOLBAR_RELOAD_MSOT,OnToolbarReloadMSOT)
	ON_MESSAGE(WM_LEFT_UPDATEDATA,OnLeftUpdateData)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrevView drawing

/*void CPrevView::OnPaint()
{
	CPaintDC dc(this);
	
}*/


/////////////////////////////////////////////////////////////////////////////
// CPrevView message handlers

BOOL CPrevView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CView::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

int CPrevView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	CRect rect;
	GetClientRect(&rect);
	
	CRect rcList(rect.left,rect.top,rect.right,rect.bottom - 22);
	CRect rc(rect.left ,rect.bottom - 22,rect.right,rect.bottom);
 
	//create the Calendar Panel
	m_CalendarDayView.Create(NULL,					// class name
					NULL,							// title
					(WS_CHILD | WS_VISIBLE),		// style
					rect,							// rectangle
					this,							// parent
					IDC_SCHEDULEDAYVIEW,			// control ID
					NULL);							// frame/doc context not use

	//create the view mode change panel
	m_wndMode.Create(NULL, _T(""), WS_VISIBLE, rc, this, -1, NULL);
	//get tree item color
	//string sProfile = string(theApp.m_szSkin) + string("Organize\\List.ini");
	Tstring sProfile = Tstring(theApp.m_szSkin) + Tstring(_T("Calendar\\List.ini"));//nono, 2004_1101

	al_GetSettingColor(_T("list"),_T("color_text_nor"),const_cast<TCHAR *>(sProfile.c_str()),m_crItemTextNor);
	al_GetSettingColor(_T("list"),_T("color_text_sel"),const_cast<TCHAR *>(sProfile.c_str()),m_crItemTextSel);
	al_GetSettingColor(_T("list"),_T("color_text_focusl"),const_cast<TCHAR *>(sProfile.c_str()),m_crItemTextFocus);
	al_GetSettingColor(_T("list"),_T("color_bg_nor"),const_cast<TCHAR *>(sProfile.c_str()),m_crItemBgNor);
	al_GetSettingColor(_T("list"),_T("color_bg_sel"),const_cast<TCHAR *>(sProfile.c_str()),m_crItemBgSel);
	al_GetSettingColor(_T("list"),_T("color_bg_focus"),const_cast<TCHAR *>(sProfile.c_str()),m_crItemBgFocus);

	al_GetSettingColor(_T("background"),_T("color1"),const_cast<TCHAR *>(sProfile.c_str()),m_crCol1);
	al_GetSettingColor(_T("background"),_T("color2"),const_cast<TCHAR *>(sProfile.c_str()),m_crCol2);
	
	return 0;
}

void CPrevView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	if(m_wndMode.GetSafeHwnd()){
		CRect rect;
		GetClientRect(&rect);
		CRect rc(rect.left ,rect.bottom - 22,rect.right,rect.bottom);
		m_wndMode.MoveWindow(rc);
	}
 
	if(m_CalendarDayView.GetSafeHwnd()){
		CRect rect;
		GetClientRect(&rect);
		CRect rcList(rect.left,rect.top,rect.right,rect.bottom - 22);
		m_CalendarDayView.MoveWindow(rcList);
	}
}

LRESULT CPrevView::OnViewModeChange(WPARAM wParam,LPARAM lParam)
{
//Calendar
 
	if (DATEPERIOD_DAILY == (int)wParam) {
    	m_CalendarDayView.ChangeDatePeriod(DATEPERIOD_DAILY,FALSE);
		m_CalendarDayView.SetFocus();
	}else if (DATEPERIOD_WEEKLY == (int)wParam) {
    	m_CalendarDayView.ChangeDatePeriod(DATEPERIOD_WEEKLY,FALSE);
		m_CalendarDayView.SetFocus();
	}else if (DATEPERIOD_MONTHLY == (int)wParam) {
    	m_CalendarDayView.ChangeDatePeriod(DATEPERIOD_MONTHLY,FALSE);
		m_CalendarDayView.SetFocus();
	}else if (DATEPERIOD_LIST == (int)wParam) {
    	m_CalendarDayView.ChangeDatePeriod(DATEPERIOD_LIST,FALSE);
		m_CalendarDayView.GetScheduleList()->SetFocus();
	}else {
		AfxMessageBox(_T("None valid Schedule_DatePeriod! \n[CPrevView::OnViewModeChange]"));
	    return false;
	}

	return 1L;
}

void CPrevView::AddDataToList(int iIndex,LPCSTR lpsFirstName, LPCSTR lpsLastName, LPCSTR lpsMobileTel, int iGroup, LPCSTR lpsMemoryType)
{
/*	CCardPanel data(iIndex,lpsFirstName,lpsLastName,lpsMobileTel,iGroup,lpsMemoryType);
	data.init();
	m_List.AddData(data);
	m_List.DrawThumbnails();
	data.Release();*/
}
/*
void CPrevView::AddDataToList(DATALIST &lsData)
{
	for(DATALIST::iterator iter = lsData.begin() ; iter != lsData.end() ; iter ++){
		(*iter).init();
		m_List.AddData((*iter));
	}
	m_List.DrawThumbnails();
}*/
/*
void CPrevView::AddDataToList(CCardPanel &data)
{
	CCardPanel dataTemp;
	dataTemp = data;
	dataTemp.init();
	m_List.AddData(dataTemp);
	m_List.DrawThumbnails();
	dataTemp.Release();

}*/
#include "processdlg.h"
LRESULT CPrevView::OnToolbarDelSel(WPARAM wParam,LPARAM lParam)
{
	//if (!m_List) return false;

	//ID_ORG_SCH_DELETE
	//this->m_CalendarDayView.PostMessage(ID_ORG_SCH_DELETE, 0, 0);
	this->m_CalendarDayView.PostMessage(WM_COMMAND, ID_ORG_SCH_DELETE, 0);
/*
	if(m_List.GetSelectedCount() <= 1 && m_List.GetNextItem(-1,LVNI_SELECTED) != -1)	{
		if(AfxMessageBox(IDS_DELONECONTACT,MB_OKCANCEL) != IDOK)
			return 1L;
	}
	else if(m_List.GetSelectedCount() > 1){
		if(AfxMessageBox(IDS_DELCONTACTS,MB_OKCANCEL) != IDOK)
			return 1L;
	}

	//Get the selected data
	DATALIST lsData;
	if(m_List.GetSelectedData(lsData)){
		//delete data from driver
		for(DATALIST::iterator iter = lsData.begin() ; iter != lsData.end() ; iter ++){
		//check is mobile 
			if(m_LeftMode.sMode == MOBILE_PHONE || m_LeftMode.sMode == MEMORY || m_LeftMode.sMode == SIM_CARD){
				if(m_Driver.DeleteContractData(*iter) == 1)
					xDelListItem(*iter);
			}
			else
				xDelListItem(*iter);
		}
		//delete data from list
		m_List.DeleteSel();
		m_List.RedrawItems(0,m_List.GetItemCount()-1);
	}
*/	return 1L;
}

HRESULT CPrevView::OnToolbarNewEdit(WPARAM wParam,LPARAM lParam)
{
//	CDataBaseDlg2 dlg(this);
	CMainFrame *pWnd = (CMainFrame*)afxGetMainWnd();
	if(pWnd->m_bMobileGetFail== true)
		return false;
	bool bMobile = true;

/*	if(m_LeftMode.sMode == MOBILE_PHONE || m_LeftMode.sMode == MEMORY || m_LeftMode.sMode == SIM_CARD){
		dlg.SetIsNew(wParam == 0 ? true: false);
		bMobile = true;
	}
	else{
		dlg.SetIsNew(false);
		bMobile = false;
	}
*/	


///// Calendar /////
    if(wParam == 0)
	{        	//create new item
	
		m_CalendarDayView.PostMessage(WM_COMMAND, ID_ORG_SCH_ADD,  0);
		/*
        if (DATEPERIOD_LIST == m_CalendarDayView.GetDatePeriod()) {
        	m_CalendarDayView.GetScheduleList()->OnAddCalendar();
		}else {
        	m_CalendarDayView.ChangeDatePeriod(DATEPERIOD_LIST);
    	    return false;
		}
		*/
	}
	else if (wParam == 1)
	{    //edit selected item
		m_CalendarDayView.PostMessage(WM_COMMAND, ID_ORG_SCH_EDIT, 0);
	}
	else 
	{                     //edit selected item
		AfxMessageBox(_T("None valid WMessage! [CPrevView::OnToolbarNewEdit]"));
     	return false;
	}

	return 1L;
}

LRESULT CPrevView::OnToolbarSearch(WPARAM wParam,LPARAM lParam)
{
	if(!m_pSearchDlg){
		m_pSearchDlg = new CSearchDlg(this);
		m_pSearchDlg->SetSearchStrVec(&m_deqSearchStr);
		m_pSearchDlg->Create(IDD_SEARCH,this);
		m_pSearchDlg->ShowWindow(SW_SHOW);
		::PostMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SEARCH_SETFLAG,1,0);
	}
	else
		m_pSearchDlg->SetFocus();
	return 1L;
}

LRESULT CPrevView::OnSearchDlgStr(WPARAM wParam,LPARAM lParam)
{
/*	TCHAR *lpszText = reinterpret_cast<TCHAR *>(wParam);
	int *iStart = reinterpret_cast<int *>(lParam);

	*iStart = m_List.FindItemData(lpszText,(*iStart));

	if((*iStart) == -1)
		AfxMessageBox(IDS_ERR_SEARCH);
	else{
		for(int i = 0 ; i < m_List.GetItemCount() ; i ++)
			m_List.SetItemState(i,0,LVIS_SELECTED);
		m_List.SetItemState((*iStart),LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED);
		m_List.EnsureVisible((*iStart),TRUE);
	}
	m_List.SetFocus();*/
	return 1L;
}

HRESULT CPrevView::OnSearchDlgEnd(WPARAM wParam,LPARAM lParam)
{
/*	SAFE_DELPTR(m_pSearchDlg);
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(),WM_SEARCH_SETFLAG,0,0);*/
	return 1L;
}

void CPrevView::OnDestroy() 
{
	CView::OnDestroy();
	//save all loaded csv file
/*	SaveCSVFile();

	//releasee the CCardPanel object data in the SIM card deque
	for(list<CCardPanel>::iterator iter = m_lsSIMData.begin() ; iter != m_lsSIMData.end() ; iter ++){
		(*iter).Release();
	}
	m_lsSIMData.clear();
//	m_lsSIMData.~list();
	//releasee the CCardPanel object data in the ME card deque
	for(iter = m_lsqMEData.begin() ; iter != m_lsqMEData.end() ; iter ++){
		(*iter).Release();
	}
	m_lsqMEData.clear();
//	m_lsqMEData.~list();
	//clear the search dialog string 
	for(deque<string>::iterator iterStr = m_deqSearchStr.begin() ; iterStr != m_deqSearchStr.end() ; iterStr ++){
		(*iterStr) = _T("");
	}
	m_deqSearchStr.clear();
	//clear the group name deque
	for(iterStr = m_deqGroup.begin() ; iterStr != m_deqGroup.end() ; iterStr ++){
		(*iterStr) = _T("");
	}
	m_deqGroup.clear();	
	//cleat the file path deque
	for(iterStr = m_deqFilePath.begin() ; iterStr != m_deqFilePath.end() ; iterStr ++){
		(*iterStr) = _T("");
	}
	m_deqFilePath.clear();

	//search every file 
	for(deque<DATALIST>::iterator iterFile = m_deqFileData.begin() ; iterFile != m_deqFileData.end() ; iterFile ++){
		//search the item data in every file
		for(iter = (*iterFile).begin() ; iter != (*iterFile).end() ; iter ++){
			(*iter).Release();
		}
		(*iterFile).clear();
	}
	m_deqFileData.clear();*/

	m_LeftMode.sFile = _T("");
	m_LeftMode.sMode = _T("");

}

LPCTSTR CPrevView::GetGroupStr(int iIndex)
{
	if(iIndex < m_deqGroup.size()){
		deque<Tstring>::iterator iter = m_deqGroup.begin() + iIndex; 
		return (*iter).c_str();
	}
	return NULL;
}

HRESULT CPrevView::OnCardPanelGetGroup(WPARAM wParam,LPARAM lParam)
{
	TCHAR *lpTxt = reinterpret_cast<TCHAR *>(lParam);
	const TCHAR *lpszGroup = GetGroupStr(wParam);

	if(lpszGroup)
		_tcscpy(lpTxt,lpszGroup);
	else
		memset(lpTxt,0,sizeof(TCHAR) * MAX_PATH);
	return 1L;
}

HRESULT CPrevView::OnCardPanelGroupCount(WPARAM wParam,LPARAM lParam)
{
	int *iCount = reinterpret_cast<int*>(wParam);
	*iCount = m_deqGroup.size();
	return 1L;
}


bool CPrevView::LoadDataFromMobile()
{/*
	if(!m_Driver.GetGroupList(m_deqGroup)){
//		AfxMessageBox(IDS_ERR_GETGROUP);
//		return false;
	}
		
	int iSIMFree = 0 , iSIMUsed = 0, iMEFree = 0,iMEUsed = 0;
	if(!m_Driver.GetContractInfo(iSIMUsed,iSIMFree,iMEUsed,iMEFree)){
		AfxMessageBox(IDS_ERR_GETSPACE);
		return false;
	}
	CString sFormat;
	if(iMEUsed > 0){
		//Get contact data one by one
		bool bStart = true;
		for(int i = 0 ; i < iMEUsed ; i ++){
			CCardPanel data;
			if(!m_Driver.GetContractData(MEMORY_NAME,data,bStart)){
				TCHAR szText[MAX_PATH],szFormat[MAX_PATH],szTmp[MAX_PATH];
				if(al_GetSettingString(_T("public"),"IDS_MOBILE",theApp.m_szRes,szText))
					if(al_GetSettingString(_T("public"),"IDS_MOBILE",theApp.m_szRes,szTmp)){
						sprintf(szFormat,szTmp,szText);
						AfxMessageBox(szFormat);
					}
				return false;
			}
			else{
				m_lsqMEData.push_back(data);
				bStart = false;
			}
		}
	}
	if(iSIMUsed > 0){
		//Get contact data one by one
		bool bStart = true;
		for(int i = 0 ; i < iSIMUsed ; i ++){
			CCardPanel data;
			if(!m_Driver.GetContractData(SIM_NAME,data,bStart)){
				TCHAR szText[MAX_PATH],szFormat[MAX_PATH],szTmp[MAX_PATH];
				al_GetSettingString(_T("public"),"IDS_SIMCARD",theApp.m_szRes,szText);
				al_GetSettingString(_T("public"),"IDS_ERR_LOADCONTRACT",theApp.m_szRes,szTmp);
				sprintf(szFormat,szTmp,szText);
				AfxMessageBox(szFormat);
				return false;
			}
			else{
				m_lsSIMData.push_back(data);
				bStart = false;
			}
		}
	}*/
	return true;
}
void CPrevView::FreeDataList(CPtrList* pList)
{
 	POSITION pos = pList->GetHeadPosition();
	while(pos )
	{
		SchedulesRecord *pData =(SchedulesRecord *) pList->GetNext(pos);
		if(pData) delete pData;
	}
	pList->RemoveAll();

}


int CPrevView::GetOutlookData(FileData *pData) 
{

	CPtrList CalendarList;
	CProcessDlg ProgressDlg;
	FreeDataList(&pData->CalendarDataList);

	ProgressDlg.SetGetMSOTDataInfo(pData,&m_CalendarDriverWrapper,&pData->CalendarDataList);
	if(ProgressDlg.DoModal() == IDOK)
	{
		pData->bLoad = true;
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLEFTITEMCOUNT,
					reinterpret_cast<WPARAM>(pData),pData->CalendarDataList.GetCount());
	}

	return 1;
}
HRESULT CPrevView::OnLeftUpdateData(WPARAM wParam,LPARAM lParam)
{
	FileData *pData = reinterpret_cast<FileData*>(wParam);
	if(lParam == 0 && (pData->sMode == MOBILE_PHONE || pData->sMode == MEMORY) &&
		(m_LeftMode.sMode== MOBILE_PHONE || m_LeftMode.sMode == MEMORY))
	{
		m_CalendarDayView.UpdateTableCalendarData(NULL,&m_CalendarDayView.m_MobileCalendarDataList);
		m_wndMode.EnableDailyWeeklyMonthlyView(TRUE);
	}
	return 1L;
}
HRESULT CPrevView::OnLeftChange(WPARAM wParam,LPARAM lParam)
{
	FileData *pData = reinterpret_cast<FileData*>(wParam);
	if(pData->sMode == m_LeftMode.sMode)
	{
		if( lParam == 0 && (pData->sMode == MOBILE_PHONE || pData->sMode == MEMORY))
			return 1L;
		else if(pData->sMode == MSOUTLOOK && pData->iIndex == m_LeftMode.iIndex)
			return 1L;
	}
	if(lParam == 0 && (pData->sMode == MOBILE_PHONE || pData->sMode == MEMORY) &&
		(m_LeftMode.sMode== MOBILE_PHONE || m_LeftMode.sMode == MEMORY))
	{
	//	m_CalendarDayView.UpdateTableCalendarData(&m_CalendarDayView.m_MobileCalendarDataList,&m_CalendarDayView.m_MobileCalendarDataList);
	//	m_wndMode.EnableDailyWeeklyMonthlyView(TRUE);
	}
	else if(_tcscmp(MOBILE_PHONE,pData->sMode.c_str()) == 0 ||_tcscmp(MEMORY,pData->sMode.c_str()) == 0 )
	{	
		m_CalendarDayView.UpdateTableCalendarData(NULL,&m_CalendarDayView.m_MobileCalendarDataList);
		m_wndMode.EnableDailyWeeklyMonthlyView(TRUE);

	}
	else if(_tcscmp(MSOUTLOOK,pData->sMode.c_str()) == 0)
	{
		if( pData->bLoad == false )
		{
			GetOutlookData(pData);
		}
		if(_tcscmp(MOBILE_PHONE,m_LeftMode.sMode.c_str()) == 0 ||_tcscmp(MEMORY,m_LeftMode.sMode.c_str()) == 0 )
			m_CalendarDayView.UpdateTableCalendarData(&m_CalendarDayView.m_MobileCalendarDataList,&pData->CalendarDataList);
		else
			m_CalendarDayView.UpdateTableCalendarData(NULL,&pData->CalendarDataList);
		m_wndMode.SendMessage(WM_COMMAND,IDC_TBN_LIST);
		m_wndMode.EnableDailyWeeklyMonthlyView(FALSE);
	}
	m_LeftMode.iIndex = pData->iIndex;
	m_LeftMode.sFile = pData->sFile;
	m_LeftMode.sMode = pData->sMode;

/*
	FileData *pData = reinterpret_cast<FileData*>(wParam);

	if(pData->sMode == m_LeftMode.sMode){
		if(pData->sMode == CSVFILE && pData->iIndex == m_LeftMode.iIndex)
			return 1L;
	}

	if(strcmp(MOBILE_PHONE,pData->sMode.c_str()) == 0){	//show all data
		if (m_List) {
     		m_List.RemoveAllData();
    		AddDataToList(m_lsqMEData);
	    	AddDataToList(m_lsSIMData);
		}
	}
	else if(strcmp(MEMORY,pData->sMode.c_str()) == 0){	//show memory
		if (m_List) {
    		m_List.RemoveAllData();
    		AddDataToList(m_lsqMEData);
		}
	}
	else if(strcmp(SIM_CARD,pData->sMode.c_str()) == 0){	//show sim
		if (m_List) {
    		m_List.RemoveAllData();
    		AddDataToList(m_lsSIMData);
		}
	}
	else if(strcmp(COMPUTER,pData->sMode.c_str()) == 0)	//computer ,show nothing
	{
		if (m_List) {
	    	m_List.RemoveAllData();
		}
	}
	else if(strcmp(CSVFILE,pData->sMode.c_str()) == 0){	//file mode
		if (m_List) {
    		m_List.RemoveAllData();
    		deque<DATALIST>::iterator  iter = m_deqFileData.begin() + pData->iIndex;
	    	AddDataToList(*iter);
		}
	}
	m_LeftMode.iIndex = pData->iIndex;
	m_LeftMode.sFile = pData->sFile;
	m_LeftMode.sMode = pData->sMode;
*/	return 1L;
}

HRESULT CPrevView::OnToolbarReload(WPARAM wParam,LPARAM lParam)
{
	//Clear all data form mobile
	//Clear 
/*	for(DATALIST::iterator iter = m_lsSIMData.begin() ; iter != m_lsSIMData.end() ; iter ++){
		(*iter).Release();
	}
	m_lsSIMData.clear();

	for(iter = m_lsqMEData.begin() ; iter != m_lsqMEData.end() ; iter ++){
		(*iter).Release();
	}
	m_lsqMEData.clear();

	m_deqGroup.clear();	

	if (m_List)
    	m_List.RemoveAllData();

	LoadDataFromMobile();*/

	//Calendar
	//m_CalendarDayView.PostMessage(WM_OPENCALENDAR, 0,0);
    this->m_CalendarDayView.LoadScheduleFromFile(_T(""));


//	OnLeftChange(reinterpret_cast<WPARAM>(&m_LeftMode),0);
	return 1L;
}

void CPrevView::OnOpenCalendar(WPARAM wParam,LPARAM lPara)
{
	m_CalendarDayView.PostMessage(WM_OPENCALENDAR, 0,0);
}
LRESULT CPrevView::OnSwitchToDayView(WPARAM wParam,LPARAM lPara)
{
	m_wndMode.SendMessage(WM_COMMAND,IDC_TBN_REPORT);
	return 1L;
}
/*
bool CPrevView::xDelListItem(CCardPanel &data)
{
	//remove the data in sim card
	if(strcmp(data.GetMemoryType() , SIM_NAME) == 0){
		for(DATALIST::iterator iter = m_lsSIMData.begin() ; iter != m_lsSIMData.end() ; iter ++){
			if((*iter).GetIndex() == data.GetIndex()){
				(*iter)	.Release();
				m_lsSIMData.erase(iter);
				break;
			}
		}		
	}
	//remove the data in the memory
	else if(strcmp(data.GetMemoryType() , MEMORY_NAME) == 0){
		for(DATALIST::iterator iter = m_lsqMEData.begin() ; iter != m_lsqMEData.end() ; iter ++){
			if((*iter).GetIndex() == data.GetIndex()){
				(*iter)	.Release();
				m_lsqMEData.erase(iter);
				break;
			}
		}		
	}
	//remove the data in csv file
	if(strcmp(data.GetMemoryType(),CSVFILE) == 0){
		//get current file
		deque<DATALIST>::iterator iterFile = m_deqFileData.begin() + m_LeftMode.iIndex;
		//file the data index in the file
		for (DATALIST::iterator iter = (*iterFile).begin() ; iter != (*iterFile).end() ; iter ++){
			if((*iter).GetIndex() == data.GetIndex()){
				(*iter).Release();
				(*iterFile).erase(iter);
				break;
			}
		}
	}
	return true;
}
*/
HRESULT CPrevView::OnToolbarRestore(WPARAM wParam,LPARAM lParam)
{
/*	TCHAR *pFile = reinterpret_cast<TCHAR*>(wParam);

	if(pFile){
		//store the path
		m_deqFilePath.push_back(pFile);

		CCSVHandle csv(pFile);
		DATALIST lsData;
		//get contract data from file
		csv.GetContract(lsData);
		//push the file data in the file deque
		m_deqFileData.push_back(lsData);
		
		//get left mode from the main window
		FileData fd;

		::SendMessage(AfxGetMainWnd()->GetSafeHwnd(),WM_GETLEFTMODE,reinterpret_cast<WPARAM>(&fd),0);
		
		OnLeftChange(reinterpret_cast<WPARAM>(&fd),0);

		//send message to main frame to set the flag 
		::PostMessage(AfxGetMainWnd()->GetSafeHwnd(),WM_SETLOADFILE,1,0);
	}*/
	return 1L;
}

HRESULT CPrevView::OnToolbarBackup(WPARAM wParam,LPARAM lParam)
{/*
	//the filter
	std::string file = "Comma Separated Values(*.CSV)";
	//file ext name
	std::string ext = "*.csv";
	//file format
	std::string szFormat = file + char(0) + ext + char(0); 


	TCHAR szMyDoc[MAX_PATH];
	SHGetSpecialFolderPath(NULL,szMyDoc,CSIDL_PERSONAL,FALSE);
	//new a CPreviewFileDlg
	CPreviewFileDlg* pDlg = new CPreviewFileDlg( FALSE, ext.c_str(), NULL, szMyDoc,
						 OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT // * | OFN_ALLOWMULTISELECT * //
						 ,szFormat.c_str(), this, ::AfxGetInstanceHandle() );

	//set filter index
	pDlg->SetFilterIndex(0);

	int nRet = pDlg->DoModal();

	//if not OK ,do nothing
	if(nRet != IDOK)
	{	
		SAFE_DELPTR(pDlg);
		return 1L;
	}
	
	//Get full path 
	TCHAR szPath[MAX_PATH];
	memset(szPath,0,sizeof(TCHAR) * MAX_PATH);
	pDlg->GetPathName(szPath);
	SAFE_DELPTR(pDlg);

	//Create the handle deque
	TCHAR TxtID[32][64] = {"IDS_FIRSTNAME","IDS_LASTNAME","IDS_COMPANYNO","IDS_MOBILENO"
		,"IDS_HOMENO","IDS_HPMENO2","IDS_EMAIL","IDS_EMAIL2","IDS_COFAX","IDS_HOMEFAX","IDS_OTHERFAX"
		,"IDS_CARTEL","IDS_COMMERCETEL","IDS_COMMERCETEL2","IDS_BBCALL","IDS_GROUP","IDS_BRANCH"
		,"IDS_COMPANY","IDS_HANDLE","IDS_COADDRESS","IDS_ZIP","IDS_CITY","IDS_TOWN"
		,"IDS_HOMEADDRESS","IDS_ZIP","IDS_CITY","IDS_TOWN","IDS_COUNTRY"
		,"IDS_HOMEPAGE","IDS_BETTERHALF","IDS_BIRTHDAY","IDS_WEDDINGDAY"};
	deque<string> deqHandle;
	for(int i = 0 ; i < 32 ; i ++){
		TCHAR szTxt[MAX_PATH];
		if(al_GetSettingString(_T("public"),TxtID[i],theApp.m_szRes,szTxt))
			deqHandle.push_back(szTxt);
	}

	CCSVHandle csv(szPath);
	//write handle deque to file
	csv.WriteHandle(deqHandle);

	//write the contract data to file
	if(m_LeftMode.sMode == MOBILE_PHONE){
		csv.WriteContract(m_lsqMEData);
		csv.WriteContract(m_lsSIMData);
	}
	else if(m_LeftMode.sMode == MEMORY){
		csv.WriteContract(m_lsqMEData);
	}
	else if(m_LeftMode.sMode == SIM_CARD){
		csv.WriteContract(m_lsSIMData);
	}
	else if(m_LeftMode.sMode == COMPUTER){
	}
	else if(m_LeftMode.sMode == CSVFILE){
		deque<DATALIST>::iterator iter = m_deqFileData.begin() + m_LeftMode.iIndex;
		csv.WriteContract(*iter);
	}
*/
	return 1L;
}
/*
void CPrevView::SaveCSVFile()
{
	//file path
	deque<string>::iterator iterPath = m_deqFilePath.begin();
	//file data list
	deque<DATALIST>::iterator iterFile = m_deqFileData.begin();

	for(int i = 0 ; i < m_deqFilePath.size() ; i++){
		//check end of the deque
		if(iterPath == m_deqFilePath.end() || iterFile == m_deqFileData.end())
			break;

		CCSVHandle csv((*iterPath).c_str());
		//get the handle
		deque<string> deqHandle;
		csv.GetHandle(deqHandle);
		//clear the file and write the handle
		csv.WriteHandle(deqHandle);
		//write the contract
		csv.WriteContract(*iterFile);
		//get next file and path
		iterPath ++;
		iterFile ++;
	}
}
*/
void CPrevView::OnLoadCalendar(WPARAM wParam,LPARAM lParam)
{
	
	m_bOpened = wParam ? true : false;

	m_CalendarDayView.m_bMobileConnected = m_bOpened;


	if (m_CalendarDayView.GetTable()) {
    	m_CalendarDayView.GetTable()->m_bMobileConnected = m_bOpened;
	}
	if (m_CalendarDayView.GetScheduleList()) {
    	m_CalendarDayView.GetScheduleList()->m_bMobileConnected = m_bOpened;
	}

	if (!m_bOpened) {
		m_CalendarDayView.ClearTable();
    	return; 
	}

	//m_CalendarDriverWrapper.InitCalendarDriver();


	m_CalendarDayView.LoadScheduleFromFile(_T(""));
}

void CPrevView::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
/*	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	*pResult = 0;
	
	// Create the drag&drop source and data objects
   	COleDataSource ds;

	POSITION pos = m_ThumbList.FindIndex(pNMListView->iItem);
	CP_TEMPLATE cp_template = m_ThumbList.GetNext(pos);

	char *cp = (char *)GlobalAlloc( GMEM_FIXED, sizeof(cp_template) );
	memcpy( cp, &cp_template, sizeof(cp_template) );

	// put the file object into the data object
	ds.CacheGlobalData(CF_TEXT, cp);	
	if(ds.DoDragDrop() == DROPEFFECT_MOVE )
	{
	}
*/
}
HRESULT CPrevView::OnToolbarReloadMSOT(WPARAM wParam,LPARAM lParam)
{
 	FileData *pData = reinterpret_cast<FileData*>(lParam);
	if(_tcscmp(MSOUTLOOK,pData->sMode.c_str()) == 0 && pData->iIndex < 0)
	{
		 GetOutlookData(pData);
		m_CalendarDayView.UpdateTableCalendarData(NULL,&pData->CalendarDataList);
	}
	return 1L;
}

HRESULT CPrevView::OnLeftDropData2MSOT(WPARAM wParam,LPARAM lParam)
{
	
	FileData *pData = reinterpret_cast<FileData*>(lParam);
	CPtrList CalendarList;
	CalendarList.RemoveAll();
	CCAMMScheduleList *pSchduleListView = m_CalendarDayView.GetScheduleList();

	if(pSchduleListView)
	{
		int nIdx;
		CListCtrl& theCtrl = pSchduleListView->GetListCtrl();

		POSITION pos = theCtrl.GetFirstSelectedItemPosition();
		while (pos)
		{
			nIdx = theCtrl.GetNextSelectedItem(pos);
			CScheduleInfo* psi = new CScheduleInfo;
			pSchduleListView->GetScheduleInfo(nIdx,*psi);
			CalendarList.AddTail(psi);
		}

		CProcessDlg ProgressDlg;
		ProgressDlg.SetAddMSOTDataInfo(pData,&m_CalendarDriverWrapper,&CalendarList);
		if(ProgressDlg.DoModal() == IDOK)
		{
			POSITION pos = CalendarList.GetHeadPosition();
			while(pos)
			{
				CScheduleInfo* pCData = (CScheduleInfo*) CalendarList.GetNext(pos);
				SAFE_DELPTR(pCData);
			}
			CalendarList.RemoveAll();
		}
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLEFTITEMCOUNT,
					reinterpret_cast<WPARAM>(pData),pData->CalendarDataList.GetCount());
	}
	return 1L;
}
HRESULT CPrevView::OnLeftDropData(WPARAM wParam,LPARAM lParam)
{

	if(m_LeftMode.sMode == MOBILE_PHONE || m_LeftMode.sMode == MEMORY )
	{
	
		m_CalendarDayView.UpdateTableCalendarData(&m_CalendarDayView.m_MobileCalendarDataList,NULL);
	}
	

	string sMode = reinterpret_cast<LPCSTR>(wParam);
	CPtrList CalendarList;
	CalendarList.RemoveAll();
	CCAMMScheduleList *pSchduleListView = m_CalendarDayView.GetScheduleList();

	if(pSchduleListView)
	{
		int nIdx;
		CListCtrl& theCtrl = pSchduleListView->GetListCtrl();

		POSITION pos = theCtrl.GetFirstSelectedItemPosition();
		while (pos)
		{
			nIdx = theCtrl.GetNextSelectedItem(pos);
			CScheduleInfo* psi = new CScheduleInfo;
			pSchduleListView->GetScheduleInfo(nIdx,*psi);
			CalendarList.AddTail(psi);
		}

		CProcessDlg ProgressDlg;
		ProgressDlg.SetAddMobileDataInfo(&m_CalendarDayView.m_MobileCalendarDataList,&m_CalendarDriverWrapper,&CalendarList);
		if(ProgressDlg.DoModal() == IDOK)
		{
			POSITION pos = CalendarList.GetHeadPosition();
			while(pos)
			{
				CScheduleInfo* pCData = (CScheduleInfo*) CalendarList.GetNext(pos);
				SAFE_DELPTR(pCData);
			}
			CalendarList.RemoveAll();
		}
	}
	return 1L;
}

HRESULT CPrevView::OnLoadMobile(WPARAM wParam,LPARAM lParam)
{
//	OnOpenDriver(0,0);
//	LoadDataFromMobile();
	OnLeftChange(reinterpret_cast<WPARAM>(&m_LeftMode),0);
	return 1L;
}

bool CPrevView::IsItemInList()
{/*
	if (!m_List.m_hWnd) return false;//nono, 2004_1025

	return m_List.GetItemCount() > 0 ? true : false;*/

    return true;
}

int CPrevView::GetListItemSelectedCount()
{
//	return m_List.GetSelectedCount();
    return true;
}

void CPrevView::OnDraw(CDC* pDC)
{
}

HRESULT CPrevView::OnOpenDriver(WPARAM wParam,LPARAM lParam)
{
	//open the sync driver
#ifndef _CalendarMode_
/*//	if(!m_Driver.Open(theApp.m_iMobileCompany,theApp.m_iPhone,theApp.m_iPortNum,theApp.m_szConnectMode,static_cast<CMainFrame*>(AfxGetMainWnd())->ConnectStatusCallback)){
	if(!m_Driver.Open(theApp.m_iMobileCompany,theApp.m_iPhone,theApp.m_szPortName,theApp.m_szConnectMode,static_cast<CMainFrame*>(AfxGetMainWnd())->ConnectStatusCallback)){
		TCHAR szText[MAX_PATH],szTmp[MAX_PATH];
		al_GetSettingString(_T("public"),"IDS_ERR_OPEN",theApp.m_szRes,szTmp);
		sprintf(szText,szTmp,theApp.m_szMobileName);
		AfxMessageBox(szText);
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_ERR_OPENSYNC,0,0);
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLINKMOBILE,0,0);
	}		
	else
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLINKMOBILE,1,0);*/
#else
	//CCAMMSync CalendarDriverWrapper;
	if (!m_bOpened) {
//	if (!m_CalendarDriverWrapper.LoadandOpenCalendarDriver()) {
	    TCHAR szText[MAX_PATH],szTmp[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_ERR_OPEN"),theApp.m_szRes,szTmp);
		wsprintf(szText,szTmp,theApp.m_szMobileName);
//		AfxMessageBox(szText);
		CalendarStringMessageBox(m_hWnd,szText);
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_ERR_OPENSYNC,0,0);
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLINKMOBILE,0,0);
	}		
	else {
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLINKMOBILE,1,0);
	}
/*
	if (!LoadandOpenCalendarDriver()) {
	    TCHAR szText[MAX_PATH],szTmp[MAX_PATH];
		al_GetSettingString(_T("public"),"IDS_ERR_OPEN",theApp.m_szRes,szTmp);
		sprintf(szText,szTmp,theApp.m_szMobileName);
		AfxMessageBox(szText);
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_ERR_OPENSYNC,0,0);
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLINKMOBILE,0,0);
	}		
	else
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLINKMOBILE,1,0);
*/
#endif

	return 1L;
}

void CPrevView::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{/*
	*pResult = 0;

	LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;

	LPNMITEMACTIVATE  lpitat = (LPNMITEMACTIVATE)pNMHDR;

	switch(lplvcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT :
		{
			*pResult = CDRF_NOTIFYITEMDRAW;
			return;
		}

		// Modify item text and or background
		case CDDS_ITEMPREPAINT:
		{
   			HDC			hDC			= lplvcd->nmcd.hdc;
			LV_DISPINFO lvdi;
	   		int			iItem		= lplvcd->nmcd.dwItemSpec;
	   		CRect		rc			// *= lplvcd->nmcd.rc* //;
   			TCHAR		szText[1024];
			m_List.GetItemRect(iItem,&rc,LVIR_ICON);
			CRect		rcTemp(0,0,THUMB_WIDTH,THUMB_HEIGHT);
			rc.left += (rc.Width() - rcTemp.Width())/2;
			rc.right = rc.left + rcTemp.Width();
			rc.top += (rc.Height() - rcTemp.Height())/2;
			rc.bottom = rc.top + rcTemp.Height();

			memset(&lvdi,0,sizeof(LV_DISPINFO));
			//get the item to redraw
   			lvdi.item.mask			= LVIF_IMAGE | LVIF_PARAM | 
   									  LVIF_STATE | LVIF_TEXT;
   			lvdi.item.iItem			= iItem;
   			lvdi.item.pszText		= szText;
   			lvdi.item.cchTextMax	= 1024;
   			if(!m_List.GetItem(&lvdi.item))
   				goto error_exit;		// Exit silently (should never happen, though)
			
   			//
   			// Now, check for callback items
   			//
   			if(lvdi.item.iImage			== I_IMAGECALLBACK ||
   			   lvdi.item.pszText		== LPSTR_TEXTCALLBACK )
   			{
   				HWND	hWndParent;

   				hWndParent = ::GetParent(m_hWnd);
   				if(hWndParent)
   				{
   					lvdi.hdr.hwndFrom	= m_hWnd;
   					lvdi.hdr.idFrom		= ::GetDlgCtrlID(m_hWnd);
   					lvdi.hdr.code		= LVN_GETDISPINFO;

   					::SendMessage(hWndParent, WM_NOTIFY, lvdi.hdr.idFrom,
   									(LPARAM)&lvdi);
   				}
   			}

			//
   			// Check if we have any normal icons to draw
   			//
   			if(lvdi.item.mask & LVIF_IMAGE == LVIF_IMAGE)
   			{
   				int	iImage,
   					cx, cy;

//   			if(lplvcd->nmcd.uItemState & CDIS_SELECTED)
//   					iImage = lvdi.item.iSelectedImage;
//   				else
   				iImage = lvdi.item.iImage;
				HIMAGELIST hImageList = NULL;
				if(m_List.GetViewMode() == 0)
					hImageList = m_List.GetImageListReport()->m_hImageList;
				else if(m_List.GetViewMode() == 1)
					hImageList = m_List.GetImageListThumb()->m_hImageList;
				
				if(hImageList)	{
   					ImageList_Draw(hImageList, iImage, hDC, rc.left, rc.top, ILD_NORMAL);
   					
   					ImageList_GetIconSize(hImageList, &cx, &cy);
   					rc.left += cx;
				}
   			}


   			//
   			// Draw the focus rect
   			//
//   			if(lplvcd->nmcd.uItemState & CDIS_FOCUS || lplvcd->nmcd.uItemState & CDIS_SELECTED)	// leo
//					::DrawFocusRect(hDC, &rcText);

			if(lpitat->ptAction.y%2)
			{
				lplvcd->clrTextBk = m_crCol2;

			}
			else
			{
				lplvcd->clrTextBk = m_crCol1;
			}

error_exit:
//   			*pResult = CDRF_SKIPDEFAULT;
			// If you want the sub items the same as the item,
			// set *pResult to CDRF_NEWFONT
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
			return;
		}

		// Modify sub item text and/or background
		case CDDS_SUBITEM | CDDS_PREPAINT | CDDS_ITEM:
		{

			*pResult = CDRF_NEWFONT;
			return;
		}
	}
 */
}


void CPrevView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
		case VK_DELETE:
			m_CalendarDayView.GotoOnKeyDown(nChar, nRepCnt, nFlags);
			break;

		default:
			return ;
	}
	
	
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CPrevView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return FALSE;
	return CView::OnEraseBkgnd(pDC);
}

BOOL CPrevView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	CCAMMScheduleDailyTable* pTable = m_CalendarDayView.GetTable();
	if(pTable && pTable->IsWindowVisible())
	{
		WPARAM wp=MAKEWPARAM(nFlags,zDelta);
		LPARAM lp=MAKELPARAM(pt.x,pt.y);

		pTable->SendMessage(WM_MOUSEWHEEL,wp,lp);
	}

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}
