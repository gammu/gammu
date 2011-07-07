// ProcessDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ProcessDlg.h"
#include "calendar.h"
//#include "UIMessage.h"
#include "MainFrm.h"
#include "PrevView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
BOOL g_bCancel = FALSE;
static CProcessDlg *sActivateDlg;
extern  Calendar_Data_Struct _Calendar_Data_Strc_;

int CProcessDlg::ProgressCallback( int nProgress )
{
	if(::IsWindow(sActivateDlg->m_hWnd) && ::IsWindow(sActivateDlg->m_progressBar.m_hWnd))
		sActivateDlg->m_progressBar.SetPos(nProgress);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CProcessDlg dialog
UINT AddCalendarDataToMSOTProc(CProcessDlg*  pDlg)
{
	CPtrList *pCalendarDataList = pDlg->m_pCalendarDataList;

	pDlg->m_progressBar.SetRange(0,pCalendarDataList->GetCount());
	pDlg->m_progressBar.SetPos(0);
	OleInitialize(NULL);
	theApp.m_MSOTDll.MSOT_InitDLL();
	LPDISPATCH lpFolder;
	lpFolder = theApp.m_MSOTDll.MSOT_GetFolderByName( (pDlg->m_pFolderInfoData)->sEntryID,  (pDlg->m_pFolderInfoData)->sStoreID,  CalendarFolder);
	if(lpFolder== NULL)
	{
		theApp.m_MSOTDll.MSOT_TerminateDLL();
		::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);
		return false;
	}
	lpFolder->AddRef();

	LPDISPATCH dispItem = NULL;
	dispItem = (LPDISPATCH)lpFolder;
	dispItem->AddRef();	// AddRef to prevent crashes; COM is inscrutible
//	CMainFrame* pFrame =(CMainFrame* ) afxGetMainWnd();
//	CPrevView *pWnd = reinterpret_cast<CPrevView*>(pFrame->m_wndSplitter.GetPane(0,1));
//	CCAMMScheduleDailyTable* pTable =(pWnd->GetCalendarDayView())->GetTable();

	int nErrorCode=0;
	POSITION pos = pCalendarDataList->GetHeadPosition();
	while(pos&& !g_bCancel)
	{
		CScheduleInfo* psi  = (CScheduleInfo*)pCalendarDataList->GetNext(pos);
//		CScheduleInfo si ;
//		si.SetData(*pSchedulesRecord);
		CalendarData CData;
  		int ret = (pDlg->m_pCalendarDataWrapper)->ConvertPCDataToMobile(*psi,&CData.Data, nErrorCode);
		if (!ret || !nErrorCode) 
		{
			theApp.m_MSOTDll.MSOT_AddCalendar(dispItem,&CData);
			CData.Data.NodeType = 0;
			theApp.m_MSOTDll.MSOT_UpdateCalendarToMobileSupport(&CData.Data);
			SchedulesRecord* pMSOTSchedulesRecord = new SchedulesRecord;
			int nCount = pDlg->m_pFolderInfoData->CalendarDataList.GetCount();
			itoa(nCount+1,CData.Data.szIndex,10);
			(pDlg->m_pCalendarDataWrapper)->ConvertMoblieDataToPC(CData.Data, pMSOTSchedulesRecord, nErrorCode);
			pDlg->m_pFolderInfoData->CalendarDataList.AddTail(pMSOTSchedulesRecord);
	//		si.SetData(*pSchedulesRecord);
	//		pTable->AddScheduleInfo(si);
		}

	}
//	 pTable->GetRecordCount(g_numTotalCalendars);
//	::SendMessage(afxGetMainWnd()->GetSafeHwnd(), WM_REFRESH_LEFT_PANEL, (DWORD)true, NULL);

	theApp.m_MSOTDll.MSOT_TerminateDLL();
	::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);
   return 0;
}
UINT AddCalendarDataToMobileProc(CProcessDlg*  pDlg)
{
	CPtrList *pCalendarDataList = pDlg->m_pCalendarDataList;

	pDlg->m_progressBar.SetRange(0,pCalendarDataList->GetCount());
	pDlg->m_progressBar.SetPos(0);
	int nErrorCode=0;
	Control_Type nType = Type_Start;
	CCAMMSync sc;
	int nIndex = 0;
	POSITION pos = pCalendarDataList->GetHeadPosition();
	while(pos && !g_bCancel)
	{
		CScheduleInfo* psi  = (CScheduleInfo*)pCalendarDataList->GetNext(pos);
		sprintf(psi->szuiIndex ,"%d",0);
//		CScheduleInfo si ;
//		si.SetData(*pSchedulesRecord);
		SchedulesRecord* pMobileSchedulesRecord = new SchedulesRecord;
	
		int nRet = sc.AddCalendar(*psi,nType,pMobileSchedulesRecord);
		if(nRet !=Anw_SUCCESS) //081027libaoliu
		{
			delete pMobileSchedulesRecord;
			break;
		}
		else if(nRet == TRUE) //add suc
			(pDlg->m_pMobileDataList)->AddTail(pMobileSchedulesRecord);
		else //add fail
			delete pMobileSchedulesRecord;
		nIndex++;
		pDlg->m_progressBar.SetPos(nIndex);

	}
	if(nType != Type_Start) 
	{
		CScheduleInfo info;
		nType = Type_End;
		sc.AddCalendar(info,nType,NULL);
	}
	g_numTotalCalendars = (pDlg->m_pMobileDataList)->GetCount();
	::SendMessage(afxGetMainWnd()->GetSafeHwnd(), WM_REFRESH_LEFT_PANEL, (DWORD)true, NULL);

	::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);
   return 0;
}

UINT DeleteCalendarDataFromMobileProc(CProcessDlg*  pDlg)
{
	CCAMMScheduleList *pSchduleListView = pDlg->m_pSchduleListView;
	if(pSchduleListView)
	{
		int nIdx,nDeleteCount = 0;
		CCAMMSync sc;
		CListCtrl& theCtrl = pSchduleListView->GetListCtrl();

		pDlg->m_progressBar.SetRange(0,theCtrl.GetSelectedCount());

		CScheduleInfo si;
		CList<CScheduleInfo,CScheduleInfo> DeleteItemList;
		DeleteItemList.RemoveAll();
		BOOL bStart = TRUE;
		POSITION pos = theCtrl.GetFirstSelectedItemPosition();
		while (pos && !g_bCancel)
		{
			nIdx = theCtrl.GetNextSelectedItem(pos);
			pSchduleListView->GetScheduleInfo(nIdx,si);
			//Calendar
	//		int nIndexDeleted = (int) si.uiIndex;
			int ret ;
			if(bStart)
			{
				ret= sc.DeleteCalendar(si,Type_Start);
				bStart = FALSE;
			}
			else
				ret= sc.DeleteCalendar(si,Type_Continune);
			if(ret == true)
			{
				DeleteItemList.AddTail(si);
				pSchduleListView->RemoveScheduleInfo(si,false);
			}
			else if(ret != Anw_SUCCESS) //081027libaoliu
				break;
			else
				bStart = TRUE;
			nDeleteCount ++;
			pDlg->m_progressBar.SetPos(nDeleteCount);
		}
		if(bStart ==false) sc.DeleteCalendar(si,Type_End);
		pos = DeleteItemList.GetHeadPosition();
		while(pos)
		{
		//	pSchduleListView->RemoveScheduleInfo(si,false);
			pSchduleListView->RemoveSchduleInfoInList(DeleteItemList.GetNext(pos));	
		}
		pSchduleListView->UpdateScheduleData();
	}

    ::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);

   return 0;
}
UINT GetCalendarDataFromMSOTProc(CProcessDlg*  pDlg)
{
	CAMMError ret=CAMM_ERROR_SUCCESS;
	int nErrorCode=0;
	OleInitialize(NULL);
	pDlg->m_progressBar.SetRange(0,100);
	pDlg->m_progressBar.SetPos(0);
	theApp.m_MSOTDll.MSOT_InitDLL();
	LPDISPATCH lpFolder;
	lpFolder = theApp.m_MSOTDll.MSOT_GetFolderByName( (pDlg->m_pFolderInfoData)->sEntryID,  (pDlg->m_pFolderInfoData)->sStoreID,  CalendarFolder);
	if(lpFolder== NULL)
	{
		theApp.m_MSOTDll.MSOT_TerminateDLL();
		::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);
		return false;
	}
	lpFolder->AddRef();

	LPDISPATCH dispItem = NULL;
	dispItem = (LPDISPATCH)lpFolder;
	dispItem->AddRef();	// AddRef to prevent crashes; COM is inscrutible

	int nIndex = 1;
	CPtrList CalendarDataList;
	theApp.m_MSOTDll.MSOT_GetCalendar(dispItem,&CalendarDataList,&g_bCancel,CProcessDlg::ProgressCallback,0,100);
	POSITION pos = CalendarDataList.GetHeadPosition();
	while(pos)
	{
		CalendarData* pData = (CalendarData*)CalendarDataList.GetNext(pos);
		theApp.m_MSOTDll.MSOT_UpdateCalendarToMobileSupport(&pData->Data);

		itoa(nIndex,pData->Data.szIndex,10);
 		SchedulesRecord* pSchedulesRecord = new SchedulesRecord;
   		ret = (pDlg->m_pCalendarDataWrapper)->ConvertMoblieDataToPC(pData->Data, pSchedulesRecord, nErrorCode);
		if (ret != CAMM_ERROR_SUCCESS)
		{
			delete pSchedulesRecord;
		}
		else
			(pDlg->m_pCalendarDataList)->AddTail(pSchedulesRecord);
		nIndex ++ ;

	}
	
	pos = CalendarDataList.GetHeadPosition();
	while(pos)
	{
		CalendarData* pData = (CalendarData*)CalendarDataList.GetNext(pos);
		if(pData) delete pData;
	}
	CalendarDataList.RemoveAll();

	theApp.m_MSOTDll.MSOT_TerminateDLL();
	::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);
   return 0;
}

UINT GetCalendarDataFromMobileProc(CProcessDlg*  pDlg)
{
	bool bRet = pDlg->OpenAvi();
	if(bRet)
		pDlg->PlayAvi();

	int nNeedCount = (pDlg->m_pCalendarDataWrapper)->GetCalendarCount();
	if(bRet)
	{
		pDlg->StopAvi();
		pDlg->CloseAvi();
	}
	if(nNeedCount < 0)
	{
		pDlg->m_nGetCount = -1;
		::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);
		   return 0;
	}

	int nRet=Anw_MOBILE_CONNECT_FAILED;
	int nUploadCount = 0;
	int nReadCount = 0;
	CAMMError ret=CAMM_ERROR_SUCCESS;
	int nErrorCode=0;
	Calendar_Data_Struct CalendarData;
	int index = 0;

	pDlg->m_progressBar.SetRange(0,nNeedCount);
	if(nNeedCount>0)
	{
		memset(&CalendarData, 0, sizeof(Calendar_Data_Struct));

		nRet = ANWGetScheduleStartData(1, &CalendarData, nReadCount);
	
//		测试dayofweekmask的读取
// 		CString st;
// 		st.Format(L"读取手机中的 dayofweekmask  %d",CalendarData.Repeat_DayOfWeekMask );
// 		AfxMessageBox(st);

		//测试时间的读取
//  		CString st;
// 		st.Format(L"ANWGetScheduleStartData之后 start time的值  %d-%d-%d  %d:%d",
// 			CalendarData.Start_DateTime.Year,
// 			CalendarData.Start_DateTime.Month,
// 			CalendarData.Start_DateTime.Day,
// 			CalendarData.Start_DateTime.Hour, 
// 			CalendarData.Start_DateTime.Minute);
//  		AfxMessageBox(st);
//  		st.Format(L"ANWGetScheduleStartData之后 end time的值  %d-%d-%d  %d:%d",
// 		   CalendarData.End_DateTime.Year,
// 		   CalendarData.End_DateTime.Month,
// 		   CalendarData.End_DateTime.Day,
// 		   CalendarData.End_DateTime.Hour, 
// 		   CalendarData.End_DateTime.Minute);
//  		AfxMessageBox(st);
		
//		测试repeat type的读取
//  		CString st;
//  		st.Format(L"读取手机中的 RecurrenceType(转换之前)  %d",CalendarData.RecurrenceFrequency );
//  		AfxMessageBox(st);

		if (Anw_SUCCESS==nRet && nReadCount >0) 
		{
 			SchedulesRecord* pSchedulesRecord = new SchedulesRecord;
   			ret = (pDlg->m_pCalendarDataWrapper)->ConvertMoblieDataToPC(CalendarData, pSchedulesRecord, nErrorCode);

			//增加对重复频率的限定  kerm add for 9a9u 1.08
			if(pSchedulesRecord->repeatFrequency != 0 && pSchedulesRecord->repeatFrequency != 1)
				pSchedulesRecord->repeatFrequency = 1;

			if (ret != CAMM_ERROR_SUCCESS)
			{
			}
			nUploadCount ++;
			index ++;
			pDlg->m_progressBar.SetPos(nUploadCount);
			(pDlg->m_pCalendarDataList)->AddTail(pSchedulesRecord);

			//测试插入的内容 加入链表的数据是正确的
// 			POSITION pos = pDlg->m_pCalendarDataList->GetHeadPosition();
// 			SchedulesRecord* prs = (SchedulesRecord*)pDlg->m_pCalendarDataList->GetNext(pos);
// 			CString st;
// 			st.Format(L"!!!!!%d",prs->repeatType);
// 			AfxMessageBox(st);
		}
		else
		{
			(pDlg->m_pCalendarDataWrapper)->xErrorHandling(nRet);
			g_bCancel = TRUE;
		}

	}

	while(nUploadCount < nNeedCount && !g_bCancel)
	{
		memset(&CalendarData, 0, sizeof(Calendar_Data_Struct));
		
		nRet = ANWGetScheduleNextData(1, &CalendarData, nReadCount);
        //CString st = CalendarData.szIndex;
// 		TCHAR tem[256];
// 		MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,CalendarData.szIndex,strlen(CalendarData.szIndex),tem,256);
//         AfxMessageBox(tem);

// 	 	CString st;	
// 		st.Format(L"process dlg 中repeat type %d",CalendarData.RecurrenceType );
// 		AfxMessageBox(st);
	
		if (Anw_SUCCESS==nRet && nReadCount >0) 
		{
   			SchedulesRecord* pSchedulesRecord = new SchedulesRecord;
	      	ret = (pDlg->m_pCalendarDataWrapper)->ConvertMoblieDataToPC(CalendarData,pSchedulesRecord , nErrorCode);

			//增加对重复频率的限定  kerm add for 9a9u 1.08
			if(pSchedulesRecord->repeatFrequency != 0 && pSchedulesRecord->repeatFrequency != 1)
				pSchedulesRecord->repeatFrequency = 1;

	    	if (ret != CAMM_ERROR_SUCCESS) 
			{
			}
			nUploadCount ++;
			pDlg->m_progressBar.SetPos(nUploadCount);
			index ++;
			(pDlg->m_pCalendarDataList)->AddTail(pSchedulesRecord);

		}
		else
		{
			(pDlg->m_pCalendarDataWrapper)->xErrorHandling(nRet);
			break;
		}

	}
			
	pDlg->m_nGetCount = index;
    ::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);

   return 0;
}

/*
UINT GetCalendarDataFromMobileProc(CProcessDlg*  pDlg)
{

	int nNeedCount = pDlg->m_nTotalGetCount;
	SchedulesRecord *pScheduleRecord = pDlg->m_pScheduleRecord;
	if(nNeedCount < 0 || pScheduleRecord == NULL)
	{
		::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);
		   return 0;
	}
	int nRet=Anw_MOBILE_CONNECT_FAILED;
	int nUploadCount = 0;
	int nReadCount = 0;
	CAMMError ret=CAMM_ERROR_SUCCESS;
	int nErrorCode=0;
	Calendar_Data_Struct CalendarData;
	int index = 0;

	pDlg->m_progressBar.SetRange(0,nNeedCount);
	if(nNeedCount>0)
	{
		memset(&CalendarData, 0, sizeof(Calendar_Data_Struct));

		nRet = ANWGetScheduleStartData(1, &CalendarData, nReadCount);
		if (Anw_SUCCESS==nRet && nReadCount >0) 
		{
    		ret = (pDlg->m_pCalendarDataWrapper)->ConvertMoblieDataToPC(CalendarData, &(pScheduleRecord[index]), nErrorCode);
			if (ret != CAMM_ERROR_SUCCESS)
			{
			}
			nUploadCount ++;
			index ++;
			pDlg->m_progressBar.SetPos(nUploadCount);
		}
		else
		{
			(pDlg->m_pCalendarDataWrapper)->xErrorHandling(nRet);
			g_bCancel = TRUE;
		}

	}
	while(nUploadCount < nNeedCount && !g_bCancel)
	{
		memset(&CalendarData, 0, sizeof(Calendar_Data_Struct));
		nRet = ANWGetScheduleNextData(1, &CalendarData, nReadCount);
		if (Anw_SUCCESS==nRet && nReadCount >0) 
		{
        	ret = (pDlg->m_pCalendarDataWrapper)->ConvertMoblieDataToPC(CalendarData, &(pScheduleRecord[index]) , nErrorCode);
	    	if (ret != CAMM_ERROR_SUCCESS) 
			{
			}
			nUploadCount ++;
			pDlg->m_progressBar.SetPos(nUploadCount);
			index ++;
		}
		else
		{
			(pDlg->m_pCalendarDataWrapper)->xErrorHandling(nRet);
			break;
		}

	}
	pDlg->m_nGetCount = index;
    ::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);

   return 0;
}*/

CProcessDlg::CProcessDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProcessDlg)
	m_hBrush = NULL;
	//}}AFX_DATA_INIT
	m_pThread=NULL;
//	m_pDeleteCalendarThread = NULL;
//	m_nTotalGetCount = 0;
//	m_pScheduleRecord = NULL;
	m_nGetCount = 0;
	m_nProgressType = PROG_MOBILE_GET;
	m_pSchduleListView = NULL;
//	m_CalendarDataList.RemoveAll();

}
CProcessDlg::~CProcessDlg()
{
	if(m_hBrush)
		::DeleteObject(m_hBrush);
}

void CProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcessDlg)
	DDX_Control(pDX, IDC_ANIMATE, m_AnCtrl);
	DDX_Control(pDX, IDC_STOP, m_btnStop);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_progressBar);
	DDX_Control(pDX, IDC_STATIC_MESSAGE, m_progressMessage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcessDlg, CDialog)
	//{{AFX_MSG_MAP(CProcessDlg)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_OK, OnOk)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcessDlg message handlers

void CProcessDlg::OnStop() 
{
	// TODO: Add your control notification handler code here
	if(m_pThread)
	{
		m_pThread->SuspendThread();
		g_bCancel = TRUE;
		m_pThread->ResumeThread();
		UpdateData(FALSE);
		m_btnStop.EnableWindow(FALSE);
	}
	else 
		CDialog::OnCancel();
}
void CProcessDlg::OnCancel()
{
	OnStop();
}
BOOL CProcessDlg::OnEraseBkgnd(CDC *pDC)
{
	CRect rect;
	GetClientRect(&rect);
	int bBkMode = ::SetBkMode(pDC->GetSafeHdc(), TRANSPARENT);
	
	Graphics grap(pDC->GetSafeHdc());

	if(m_hBrush)
		::FillRect(pDC->GetSafeHdc(), &rect, m_hBrush);

    /*COLORREF color=(0,0,0);
	HBRUSH frameBrush = CreateSolidBrush(color);
	if(frameBrush)
		::FrameRect(pDC->GetSafeHdc(), &rect, frameBrush);*/
	
	::SetBkMode(pDC->GetSafeHdc(), bBkMode);

	return TRUE;
}
BOOL CProcessDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
    g_bCancel = FALSE; 

	//this->MoveWindow(0,0,404,96);
	//this->CenterWindow();
	
	// TODO: Add extra initialization here
	TCHAR pSkinPath[_MAX_PATH];
	al_GetModulePath(NULL, pSkinPath);
	TCHAR  szProfile[_MAX_PATH];//, szFileName[_MAX_PATH], buf[_MAX_PATH];
	wsprintf(szProfile, _T("%s%s"), pSkinPath, _T("skin\\default\\Calendar\\ProgDlg.ini"));
	//load cancel button
	GetButtonFromSetting(&m_btnStop,_T("cancel"),_T("IDS_CANCEL"),0,szProfile);
	//load text static
	GetStaticFromSetting(&m_progressMessage,_T("text_sc"),NULL,szProfile);
	//get progress rect 
	CRect rect;
	if(al_GetSettingRect(_T("progress"),_T("rect"),szProfile,rect)){
		m_progressBar.MoveWindow(rect);
	}
	
	//get dialog rect
	if(al_GetSettingRect(_T("panel") , _T("rect"),szProfile, rect)){
		LONG style = GetWindowLong(GetSafeHwnd(),GWL_STYLE);
		LONG dwstyle = GetWindowLong(GetSafeHwnd(),GWL_EXSTYLE);
		AdjustWindowRectEx(rect,style,FALSE,dwstyle);
		MoveWindow(rect);
		CenterWindow();
	}
	//get background color
	COLORREF crBg;
	al_GetSettingColor(_T("panel") , _T("brush"),szProfile, crBg);
	m_hBrush = CreateSolidBrush(crBg);	

	//Set cancel button position
	CRect rcProg,rc;
	GetClientRect(rc);
	m_btnStop.GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.OffsetRect(rc.right - rect.right - 8,0);
	m_btnStop.MoveWindow(rect);

	m_progressBar.GetWindowRect(&rcProg);
	ScreenToClient(&rcProg);
	rcProg.right = rect.left - 8;
	m_progressBar.MoveWindow(rcProg);

	//set progress bar setting
	COLORREF white, blue;
	white = RGB(255,255,255);
	blue =  RGB(0,0,255);
	m_progressBar.SetBarColor(blue);
	m_progressBar.SetShowPercent();
	m_progressBar.SetBkColor( white );
	m_progressBar.SetTextColor( white, blue );
	sActivateDlg = this;
	switch(m_nProgressType)
	{
	case PROG_MOBILE_GET:
		SetTitle(PROG_MOBILE_GET);
		 m_pThread = AfxBeginThread((AFX_THREADPROC)GetCalendarDataFromMobileProc,  this, THREAD_PRIORITY_NORMAL);
		break;
	case PROG_MOBILE_DEL:
		m_AnCtrl.ShowWindow(SW_HIDE);
		SetTitle(PROG_MOBILE_DEL);
		 m_pThread = AfxBeginThread((AFX_THREADPROC)DeleteCalendarDataFromMobileProc,  this, THREAD_PRIORITY_NORMAL);
		break;
	case PROG_MOBILE_ADD:
		m_AnCtrl.ShowWindow(SW_HIDE);
		SetTitle(PROG_MOBILE_ADD);
		 m_pThread = AfxBeginThread((AFX_THREADPROC)AddCalendarDataToMobileProc,  this, THREAD_PRIORITY_NORMAL);
		break;
	case PROG_MSOT_GET:
		m_AnCtrl.ShowWindow(SW_HIDE);
		SetTitle(PROG_MSOT_GET);
		 m_pThread = AfxBeginThread((AFX_THREADPROC)GetCalendarDataFromMSOTProc,  this, THREAD_PRIORITY_NORMAL);
		break;
	case PROG_MSOT_ADD:
		m_AnCtrl.ShowWindow(SW_HIDE);
		SetTitle(PROG_MSOT_ADD);
		 m_pThread = AfxBeginThread((AFX_THREADPROC)AddCalendarDataToMSOTProc,  this, THREAD_PRIORITY_NORMAL);
		break;
	}

/*    CRect rect;
    COLORREF color;
	//load panel color
	if( !m_hBrush && al_GetSettingColor( _T("panel"), "color", szProfile, color ) ) {
		m_hBrush = CreateSolidBrush(color);	
     	m_progressBar.SetBkColor(color);
		//m_progressMessage.SetBkColor(color);
	}


	//load stop button
	m_btnStop.SetButtonStyle(BS_OWNERDRAW);
	GetButtonFromSetting(&m_btnStop , "stop" , "IDS_STOP" , 0,szProfile );
	if( al_GetSettingRect( "stop", _T("rect"), szProfile, rect ) )
	{	
		m_btnStop.MoveWindow( &rect, TRUE);
	}
	m_btnStop.ShowWindow(1);

	// Panel
	al_GetSettingRect(_T("panel"),_T("rect"),(LPSTR)(LPCSTR)szProfile,rect);
	CRect rc;
	GetWindowRect(&rc);
	int nCapHeight = ::GetSystemMetrics(SM_CYCAPTION);
	int nDlgFrmX = ::GetSystemMetrics(SM_CXDLGFRAME);
	int nDlgFrmY = ::GetSystemMetrics(SM_CYDLGFRAME);
	::MoveWindow( GetSafeHwnd(), rc.left, rc.top,  rect.Width() + nDlgFrmX*2, 
				nCapHeight + nDlgFrmY*2 + rect.Height(), TRUE );
	
	
	if( al_GetSettingRect( "progress", _T("rect"), szProfile, rect ) )
	{	
		m_progressBar.MoveWindow( &rect, TRUE);
		if(al_GetSettingColor( "progress", "color", szProfile, color ) )
		{
			m_progressBar.SetBarColor(color);
			m_progressBar.SetShowPercent();
		}


	}

	//prompt
	GetStaticFromSetting(&m_progressMessage,"prompt",0,szProfile);
    TCHAR szPrompt[_MAX_PATH];
	if (al_GetSettingString( "prompt", "message", szProfile, szPrompt) ) {
    	m_progressMessage.SetWindowText(szPrompt);
	}

	//set windows tile
    TCHAR szTitle[_MAX_PATH];
	if (al_GetSettingString( "panel", "title", szProfile,szTitle) ) {
    	this->SetWindowText(szTitle);
	}
	*/

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CProcessDlg::OnOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();	
}
void CProcessDlg::SetGetMobileDataInfo(CCAMMSync* pCAMMSync,CPtrList* pCalendardataList)
{
	m_nProgressType = PROG_MOBILE_GET;
//	m_nTotalGetCount = nTotalCount ;
//	m_pScheduleRecord = record;
	m_pCalendarDataWrapper = pCAMMSync;
	m_pCalendarDataList = pCalendardataList;

}

void CProcessDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rect;
	GetClientRect(rect);
	::FillRect(dc,rect,m_hBrush);
	
	// Do not call CDialog::OnPaint() for painting messages
}
void CProcessDlg::SetTitle(int iTitle)
{
	if(iTitle == PROG_MOBILE_GET ||iTitle == PROG_MOBILE_ADD ||
		iTitle == PROG_MSOT_GET ||iTitle == PROG_MSOT_ADD){
		//set title
		TCHAR szTxt[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_PROG_TRANSFER"),theApp.m_szRes,szTxt);
		SetWindowText(szTxt);
		//set description
		al_GetSettingString(_T("public"),_T("IDS_PROG_TRANSFER_TEXT"),theApp.m_szRes,szTxt);
		m_progressMessage.SetWindowText(szTxt);
	}
	else if(iTitle == PROG_MOBILE_DEL){
		//set title
		TCHAR szTxt[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_PROG_DEL"),theApp.m_szRes,szTxt);
		SetWindowText(szTxt);
		//set description
		al_GetSettingString(_T("public"),_T("IDS_PROG_DEL_TEXT"),theApp.m_szRes,szTxt);
		m_progressMessage.SetWindowText(szTxt);		
	}
}

void CProcessDlg::SetDeleteInfo(CCAMMScheduleList *pSchduleListView)
{
	m_nProgressType = PROG_MOBILE_DEL;
	m_pSchduleListView = pSchduleListView;
}
bool CProcessDlg::PlayAvi()
{
	m_progressBar.ShowWindow(SW_HIDE);
	m_progressMessage.ShowWindow(SW_HIDE);
	m_btnStop.ShowWindow(SW_HIDE);

	m_AnCtrl.ShowWindow(SW_SHOW);
	m_AnCtrl.Play(0,-1,-1);

	return true;
}

bool CProcessDlg::StopAvi()
{

	m_AnCtrl.Seek(0);
	m_AnCtrl.Stop();

	return true;
}	

bool CProcessDlg::CloseAvi()
{

	m_AnCtrl.Close();
	m_AnCtrl.ShowWindow(SW_HIDE);

	m_progressBar.ShowWindow(SW_SHOW);
	m_progressMessage.ShowWindow(SW_SHOW);
	m_btnStop.ShowWindow(SW_SHOW);
	return true;
}

bool CProcessDlg::OpenAvi()
{
	CRect rect;
	TCHAR pSkinPath[_MAX_PATH];
	al_GetModulePath(NULL, pSkinPath);
	TCHAR  szProfile[_MAX_PATH];
	wsprintf(szProfile, _T("%s%s"), pSkinPath, _T("skin\\default\\Calendar\\ProgDlg.ini"));


	if(al_GetSettingRect(_T("avi_ctrl"),_T("rect"),szProfile,rect))
		m_AnCtrl.MoveWindow(rect);

	TCHAR szTxt[MAX_PATH],szPath[MAX_PATH];
	if(al_GetSettingString(_T("avi_ctrl"),_T("file"),szProfile,szTxt)){
		wsprintf(szPath, _T("%s%s%s"), pSkinPath, _T("skin\\default\\"),szTxt);
		if(!m_AnCtrl.Open(szPath))
			return false;
	}
	else 
		return false;
	
	return true;
}
