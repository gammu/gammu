// cammschedulelist.cpp : implementation file
//

#include "stdafx.h"
#include "cammschedulelist.h"
#include "..\ProcessDlg.h"
#include "CAMMScheduleEditorDlg.h"
#include "CAMMScheduleDayView.h"
#include "..\loadstring.h"
#include "..\MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum {Print_Record, Print_Record_List} Print_Record_Mode;
static int static_nPrint_Record_Mode = 0; 


BOOLEAN CCAMMScheduleList::m_bFlag = FALSE ;
BOOLEAN CCAMMScheduleList::m_bSTC=FALSE;
static int CALLBACK compare ( LPARAM lparam1, LPARAM lparam2, LPARAM lparamsort );
//static function
static int CALLBACK compare(LPARAM lparam1, LPARAM lparam2, LPARAM lparamsort )
{
    int res = 0 ;

	if (lparam1 == NULL) 
		return -1;
	if (lparam2 == NULL)
		return 1;

	CScheduleInfo *sr1 = (CScheduleInfo *)lparam1;
	CScheduleInfo *sr2 = (CScheduleInfo *)lparam2;
	CScheduleInfo *sr ;

    if (CCAMMScheduleList::m_bFlag ) {
        sr = sr2;
        sr2 = sr1;
        sr1 = sr;
    }

	CString us1,us2;
	int n1,n2,nResult;
	switch(lparamsort)
	{
		case SCH_TITLE : 
			us1 = sr1->csTitle;
			us2 = sr2->csTitle;
			break ;
		case SCH_STARTDATE : 
	//		us1 = sr1->dtStartDate.Format(VAR_DATEVALUEONLY);
	//		us2 = sr2->dtStartDate.Format(VAR_DATEVALUEONLY);
			if(sr1->dtStartDate < sr2->dtStartDate) return -1;
			else if(sr2->dtStartDate < sr1->dtStartDate)
				return 1;
			else
				return 0;
			break ;			
		case SCH_STARTTIME :
		//	us1 = sr1->hiRealStartTime.FormatString();
		//	us2 = sr2->hiRealStartTime.FormatString();
			if (sr1->hiRealStartTime < sr2->hiRealStartTime)
				return -1;
			else if (sr2->hiRealStartTime < sr1->hiRealStartTime)
				return 1;
			return 0;
			break ;
		case SCH_STOPTIME :
		//	us1 = sr1->hiRealStopTime.FormatString();
		//	us2 = sr2->hiRealStopTime.FormatString();
			if (sr1->hiRealStopTime < sr2->hiRealStopTime)
				return -1;
			else if (sr2->hiRealStopTime < sr1->hiRealStopTime)
				return 1;
			return 0;
			break ;
		case SCH_REPEAT :
			n1 = sr1->repeatType;
			n2 = sr2->repeatType;
			if(n1 > n2) nResult = 1;
			else if(n1 < n2) nResult = -1;
			else nResult=0;
			return nResult;
			break ;

    }
		if (us1.IsEmpty()) {
			res = -1;
		} else if (us2.IsEmpty()) {
			res = 1;
		} else {
			res = us1.Compare(us2);
		}
		//	}


    return res ;

}

/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleList

//IMPLEMENT_DYNCREATE(CCAMMScheduleList, CListView)

CCAMMScheduleList::CCAMMScheduleList()
{
	m_Table = NULL;
	m_nCurrentItem = 0;
	m_bInDataSection = FALSE;

//	m_pPrintRecord = NULL; //Nono. 2004_0513
//	m_PrintSchedule = NULL;//Nono. 2004_0517

	m_bMobileConnected = false;
	m_bCreateImageList = FALSE;
}

CCAMMScheduleList::~CCAMMScheduleList()
{
	m_ScheduleView = NULL;
	m_Table = NULL;
	m_Menu.DestroyMenu();
	m_ImageList.Detach();

//	if (m_pPrintRecord) delete m_pPrintRecord;//Nono. 2004_0513
//	if (m_PrintSchedule) delete m_PrintSchedule;//Nono. 2004_0517
}


BEGIN_MESSAGE_MAP(CCAMMScheduleList, CListView)
	//{{AFX_MSG_MAP(CCAMMScheduleList)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
    ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDBLCLK()
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_COMMAND(ID_ORG_SCH_DELETE, OnOrgSchDelete)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemchanged)
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBegindrag)
	//}}AFX_MSG_MAP
/*	ON_COMMAND(ID_ORG_SCH_PRINT, OnOrgPrnSchedule)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_PRINT, OnUpdateOrgPrnSchedule)
	ON_COMMAND(ID_ORG_SCH_PRINT_LIST, OnOrgPrnScheduleList)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_PRINT_LIST, OnUpdateOrgPrnScheduleList)
*/
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_MESSAGE(WM_ADD_CALENDAR, OnOrgSchAdd)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleList drawing

void CCAMMScheduleList::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}


/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleList diagnostics

#ifdef _DEBUG
void CCAMMScheduleList::AssertValid() const
{
	CListView::AssertValid();
}

void CCAMMScheduleList::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleList message handlers

BOOL CCAMMScheduleList::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= LVS_REPORT | LVS_SHOWSELALWAYS;// | LVS_SINGLESEL;
	
	return CListView::PreCreateWindow(cs);
}

int CCAMMScheduleList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;
	GetListCtrl().SetExtendedStyle(LVS_EX_FULLROWSELECT /*| LVS_EX_GRIDLINES*/);// | LVS_EX_HEADERDRAGDROP);
	GetListCtrl().SetBkColor(::GetSysColor(COLOR_WINDOW));
//	GetListCtrl().SetBkColor(RGB(217,224,255));
//	GetListCtrl().SetTextBkColor(RGB(217,224,255));
//	GetListCtrl().SetTextColor(RGB(1,33,72));
/*	CCAMMSync sc;

	if (sc.GetCalendarSupport(m_SizeofSupportNoteType, m_ArrayofSupportNoteType))
	{
		m_ImageList.Create(16 ,16 ,ILC_COLOR24 | ILC_MASK,m_SizeofSupportNoteType,0);
		for(int i =0 ;i<m_SizeofSupportNoteType ; i++)
		{
			HICON hIcon;
			char szIconFileName[MAX_PATH];
			wsprintf(szIconFileName, "%s%s", theApp.m_szSkin,m_ArrayofSupportNoteType[i].szIconfileName);
			hIcon =(HICON) LoadImage(NULL,szIconFileName,IMAGE_ICON,NULL,NULL,LR_LOADFROMFILE);
			m_ImageList.Add(hIcon);
		}
	}
	else
	{
		m_ImageList.Create(16 ,16 ,ILC_COLOR24 | ILC_MASK,5,0);
		HICON hIcon;
		
		hIcon = afxGetApp()->LoadIcon(IDI_REMIND);
		m_ImageList.Add(hIcon);

		hIcon = afxGetApp()->LoadIcon(IDI_PHONE);
		m_ImageList.Add(hIcon);
		hIcon = afxGetApp()->LoadIcon(IDI_METTING);
		m_ImageList.Add(hIcon);
		hIcon = afxGetApp()->LoadIcon(IDI_BIRTHDAY);
		m_ImageList.Add(hIcon);
		hIcon = afxGetApp()->LoadIcon(IDI_MEMO);
		m_ImageList.Add(hIcon);
	}

	GetListCtrl().SetImageList(&m_ImageList,LVSIL_SMALL);
*/
    CListCtrl& theCtrl = GetListCtrl();
	int Size[5]={102,100,100,100,100};
	int i=0;
    for (i=0;i<5;i++) 
	{
		theCtrl.DeleteColumn(0);
	}
    for (i=0;i<5;i++) //080612LIBAOLIU
	{		//增加优先级列 kerm  add for 9@9u
		CString StringLoaded;
//		StringLoaded = LoadStringFromFile(ghSkinResource,columnHeaderStringId[i]);
		switch(i)
		{
		case 0:
			StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_TITLE"));
			break;
		case 1:
			StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_DATE"));
			break;
		case 2:
			StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_STARTTIME"));
			break;
		case 3:
			StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_STOPTIME"));
			break;
		case 4:
			StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_REPEAT"));
			break;
		case 5:
			StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_PRIORITY"));
			break;
		}
		
		// Insert a column.
		theCtrl.InsertColumn(i, StringLoaded, LVCFMT_LEFT);
		// Set reasonable widths for our columns
		theCtrl.SetColumnWidth(i, Size[i]);//LVSCW_AUTOSIZE_USEHEADER);
	}
	
	theCtrl.SetCallbackMask(LVIS_FOCUSED);
	
	return 0;
}

void CCAMMScheduleList::OnDestroy() 
{
	
	//	DeInitialize();

	CListView::OnDestroy();
	
}

void CCAMMScheduleList::DeInitialize()
{
	int count = GetListCtrl( ).GetItemCount( ) ;
	int i=0;
	if (count > 0) {
		GetListCtrl().DeleteAllItems();
	}

	FreeScheduleList();
	/*
	int nColumnCount = GetListCtrl().GetHeaderCtrl()->GetItemCount();
	// Delete all of the columns.
	for (i=0;i < nColumnCount;i++)
	{
	   GetListCtrl().DeleteColumn(0);
	}
*/
}

void CCAMMScheduleList::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CListView::OnShowWindow(bShow, nStatus);
	
	if 	(bShow) {
	}
}


void CCAMMScheduleList::Initialize() 
{
    CListCtrl& theCtrl = GetListCtrl();
	DeInitialize();
	//	theCtrl.DeleteAllItems();
//	INT columnHeaderStringId[5] ;//= {IDS_ORG_SCH_TITLE,IDS_ORG_SCH_DATE,IDS_ORG_SCH_STARTTIME
								//	,IDS_ORG_SCH_STOPTIME,IDS_ORG_SCH_REPEAT};
	SetImageList();
	int i = 0;
	int cnt=0;
	FreeScheduleList();
	if (m_Table)
    	m_Table->GetRecordCount(cnt);
	if (cnt == 0) {
		return;
	}
	SchedulesRecord *psr = new SchedulesRecord[cnt];
	for (i=0;i<cnt;i++) {
		psr[i].Reset();
	}
	if (m_Table)
    	m_Table->GetScheduleData(psr,cnt);

//#ifdef _DEBUG
//    DumpInsertSchedule(psr,cnt);
//#endif
	int nListIndex = 0;

	int iImageIndex = 0;
	for (i=0;i<cnt;i++)
	{
		CScheduleInfo *psi = new CScheduleInfo;
		psi->SetData(psr[i]);

		if(psi->dtStartDate.GetStatus() == COleDateTime ::invalid)
		{
			delete psi;
			continue;
		}

		m_ScheduleList.AddTail(psi);
		LV_ITEM lvColItems;

		for(int i =0 ;i<m_SizeofSupportNoteType ; i++)
		{
			if(m_ArrayofSupportNoteType[i].nNodeType == psi->uiNodeType)
			{
				iImageIndex = i;
				break;
			}
		}
/*		switch(psi->uiNodeType)
		{
		case GSM_CAL_REMINDER:
			iImageIndex = 0;
			break;
		case GSM_CAL_CALL:
			iImageIndex = 1;
			break;
		case GSM_CAL_MEETING:
			iImageIndex = 2;
			break;
		case GSM_CAL_BIRTHDAY:
			iImageIndex = 3;
			break;
		case GSM_CAL_MEMO:
			iImageIndex = 4;
			break;
		}*/

		lvColItems.mask = LVIF_TEXT|LVIF_IMAGE;
		lvColItems.iItem = nListIndex;
		lvColItems.iSubItem = 0;
		lvColItems.iImage = iImageIndex;
		lvColItems.pszText = NULL;//new TCHAR[MAX_LENGTH];
		//		memset(lvColItems.pszText,0x00,MAX_LENGTH);
		//		lstrcpy(lvColItems.pszText,si.csTitle);
		lvColItems.lParam = NULL;
		int nIndex = theCtrl.InsertItem(&lvColItems);
		theCtrl.SetItemData(nIndex,(unsigned long)psi);

		//		theCtrl.SetItemText(i,1,si.csTitle);
		theCtrl.SetItemText(nListIndex,0,psi->csTitle);
  		theCtrl.SetItemText(nListIndex,1,psi->dtStartDate.Format(VAR_DATEVALUEONLY));
	//	if (MEETING == (INT) si.uiNodeType || CALL == (INT) si.uiNodeType) 
		{
	    	theCtrl.SetItemText(nListIndex,2,psi->hiRealStartTime.FormatString());
		}
	//	if (MEETING == (INT) si.uiNodeType) 
		{
	    	theCtrl.SetItemText(nListIndex,3,psi->hiRealStopTime.FormatString());
		}
// 		CString strlll;
// 		strlll.Format(L"%d",psi->uiPrivate);
// 		AfxMessageBox(strlll);
		CString StringLoaded;
		switch(psi->uiPrivate)		//kerm add for 9@9u 07.12.12
		{
		case 1:
			StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_STATIC_PRIORITY1"));
			theCtrl.SetItemText(nListIndex,5,StringLoaded);	
			break;
		case 2:
			StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_STATIC_PRIORITY2"));
			theCtrl.SetItemText(nListIndex,5,StringLoaded);	
			break;
		case 3:
			StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_STATIC_PRIORITY3"));
			theCtrl.SetItemText(nListIndex,5,StringLoaded);	
			break;
		}
		
		if (psi->haveRepeat) 
		{
			//CString StringLoaded = LoadStringFromFile(ghSkinResource,IDS_ORG_SCH_INTERVALUNIT0 + si.repeatType);
           	TCHAR szBuffer[10], szRepeatType[32];
			
			int nRepeat = psi->repeatType;
			
// 			CString st;
// 			st.Format(L"初始化列表时的repeat %d", psi->nDayofWeekMask);
// 			AfxMessageBox(st);

			//去掉下面两种类型，kerm for 9a9u 1.11
/*			if(nRepeat == WEEKLY && psi->nDayofWeekMask == 62)
				nRepeat = 7;
			else if(nRepeat == WEEKLY && psi->nDayofWeekMask == 65)
				nRepeat = 6;*/	

		    // kerm add for 9a9u 1.05
		    //增加日的显示
// 		    if(nRepeat == WEEKLY && psi->nDayofWeekMask != 0)
// 			{	nRepeat = 8; }//080613libaoliu

			_itot(nRepeat, szBuffer, 10);
			al_GetSettingString(_T("repeattype"), szBuffer, theApp.m_szRes,szRepeatType);
			theCtrl.SetItemText(nListIndex,4,szRepeatType);
		} 
		else 
		{

			CString  s = LoadStringFromFile(_T("public"),_T("none"));
			theCtrl.SetItemText(nListIndex,4,s);
		}
		nListIndex ++;
	}

	if (cnt > 0) {
		delete [] psr;
	}
	//	SetScrollRange(SB_VERT,0,cnt+12,TRUE);
	//	CSize size(0,1000);
	//	theCtrl.Scroll(size);
}


void CCAMMScheduleList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SCROLLINFO si;
	GetScrollInfo(SB_VERT,&si,SIF_ALL);
	switch (nSBCode) {
	case SB_ENDSCROLL:
		if ((si.nMax - GetListCtrl().GetItemCount()) < 2) {
			SetScrollRange(SB_VERT,0,GetListCtrl().GetItemCount(),TRUE);
		}
		return;
		break;
	}

	CListView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CCAMMScheduleList::OnOrgSchAdd() 
{
	// TODO: Add your command handler code here

	xCreateScheduleItem();

	
}

void CCAMMScheduleList::OnUpdateOrgSchAdd(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
		
	pCmdUI->Enable(TRUE); 
	CString txt = LoadStringFromFile(_T("public"),_T("Add"));
	ASSERT(&txt);
	pCmdUI->SetText(txt);

	
}


CAMMError CCAMMScheduleList::xCreateScheduleItem()
{
	//判断是不是超出条数限制
	if (xCheckRecordMaxLimit(1)) 
	{
		return CAMM_ERROR_SUCCESS;
	}
	//新建两个进度表对象...
	CScheduleInfo si,si2;
	
	if (xShowEditorDlg(si,si2)) //调用这个函数时，si2作为返回值（形参为引用）
	{
/*        //For Calendar
        CCAMMSync cs;
		if (!cs.AddCalendar(si2)) {
			AfxMessageBox("Failed to adding a new Calendar!\nPlease check the satus of connecting mobile.");
            return CAMM_ERROR_FAILURE;
		}else {
			AfxMessageBox("Succeed to adding a new Calendar!.", MB_OK);
		}	
*/		
//		Initialize();
		CListCtrl& theCtrl = GetListCtrl();
		SCROLLINFO si;
		GetScrollInfo(SB_VERT,&si,SIF_ALL);
		if (theCtrl.GetItemCount() > si.nPage) 
		{
			CSize size;
			size.cx=0;
			RECT rt;
			theCtrl.GetItemRect(theCtrl.GetItemCount()-1,&rt,LVIR_LABEL );
			size.cy = rt.bottom;
			theCtrl.Scroll(size);

			SendNotifyMessage(WM_VSCROLL,MAKEWPARAM(SB_BOTTOM,si.nPos),NULL);
			SendNotifyMessage(WM_VSCROLL,MAKEWPARAM(SB_ENDSCROLL,si.nPos),NULL);
			SendNotifyMessage(WM_VSCROLL,MAKEWPARAM(SB_BOTTOM,si.nPos),NULL);
			SendNotifyMessage(WM_VSCROLL,MAKEWPARAM(SB_ENDSCROLL,si.nPos),NULL);
			SendNotifyMessage(WM_VSCROLL,MAKEWPARAM(SB_BOTTOM,si.nPos),NULL);
			SendNotifyMessage(WM_VSCROLL,MAKEWPARAM(SB_ENDSCROLL,si.nPos),NULL);
		}
	}
		
	//Added by Nono
	//((CCAMMScheduleDayView *)GetParent())->SetInfomationonStatusBar();


	//REFRESH LEFT_PANEL to change the totla number of calendars displayed on left view.
//	if (m_bMobileConnected) {
//		CCAMMSync sc;
//    	g_numTotalCalendars = sc.GetCalendarCount();
   //     ::SendMessage(afxGetMainWnd()->GetSafeHwnd(), WM_REFRESH_LEFT_PANEL, (DWORD)true, NULL);
//	}	

	return CAMM_ERROR_SUCCESS;
}

BOOL CCAMMScheduleList::xCheckRecordMaxLimit(int newCnt)
{
	//Revise the limitation setting.[Nono,2004_0927]
	int rc = GetListCtrl( ).GetItemCount( ) ;
	rc += newCnt;
/*	if (rc > g_nMAX_SCHEDULE_COUNT ) {
		CString s = LoadStringFromFile(ghSkinResource,IDS_ORG_RECORD_LIMIT);
		CString msg;
		msg.Format((LPCTSTR)s,g_nMAX_SCHEDULE_COUNT);//Revise the limitation setting.[Nono,2004_0927]
		AfxMessageBox(msg);
		return TRUE;
	}*/
	if (rc > g_nMAX_SCHEDULE_COUNT )
	{
		CString  s = LoadStringFromFile(_T("message"),_T("MSG_RECORD_LIMIT"));
		CString msg;
		msg.Format(s,g_nMAX_SCHEDULE_COUNT);
		CalendarStringMessageBox(m_hWnd,msg);
	}

	return FALSE;
}


BOOL CCAMMScheduleList::xShowEditorDlg(CScheduleInfo si, CScheduleInfo &si2)
{
//	bool nbNewRecord = (0==si.uiIndex) ? true : false;
	bool nbNewRecord ;//= (0==atoi(si.szuiIndex)) ? true : false;
	//新建项目时si传来时是空的，所以下面这个条件是成立的
	if(strlen(si.szuiIndex) <= 0 || _stricmp(si.szuiIndex,"0") == 0)
		nbNewRecord = true;
	else
		nbNewRecord = false;

	if (nbNewRecord)	//新建项目时执行
	{
		int nH,nM;
		si.hiStartTime.Get24HourTime(nH,nM);
		si.haveAlarm = 0;
		si.dtAlarmDateTime.SetDateTime(si.dtStartDate.GetYear(),si.dtStartDate.GetMonth(),si.dtStartDate.GetDay(),nH,nM,0);
		//kerm add private for 99u 12.29
		si.uiPrivate = 2;	//新建立时默认为中
	}
	//	COleDateTimeEx orgDate = si.dtStartDate;
	//	COleDateTimeEx newDate;
	CMainFrame* pFrame =(CMainFrame* ) afxGetMainWnd();
	CString strTreeMode = pFrame->GetTreeSelectMode();
	CCAMMScheduleEditorDlg sdDlg(this);// = new CCAMMScheduleEditorDlg();
	sdDlg.SetScheduleInfo(si);			//设置editdlg内容为si内容
	if(strTreeMode.Compare(_T("MSOUTLOOK")) == 0)
		sdDlg.SetViewOnlyMode(true);

	if (m_Table)
    	sdDlg.SetMonthInfoPtr(m_Table->m_sMonthInfo);

	if ((sdDlg.DoModal()) == IDOK) 
	{
		DWORD dw = ::GetTickCount();
		CWaitCursor ww;
		sdDlg.GetScheduleInfo(si2);


				
		si.DumpScheduleConntent();
		si2.DumpScheduleConntent();
	

        CCAMMSync cs;
		if (nbNewRecord) 
		{
			
		
			if (!cs.AddCalendar(si2)) 
			{
			//AfxMessageBox("Failed to add a new Calendar!\nPlease check the satus of connecting mobile.");
            return CAMM_ERROR_FAILURE;
			}
			else
			{
			//AfxMessageBox("Succeed to adding a new Calendar!.", MB_OK);
			}
	
		}
		else 
		{
			if (! cs.EditCalendar(si2) ) 
			{
			//AfxMessageBox("Failed to edit a Calendar!\nPlease check the satus of connecting mobile.");
            return FALSE;
			}
			else 
			{
			//AfxMessageBox("Succeed to edit a new Calendar!.", MB_OK);
			}
		
		}
		
		//Save new data
		if (m_Table)
    		m_Table->RemoveScheduleInfo(si);

		si2.bScheduleStart = TRUE;
		if (m_Table) 
		{
	    	m_Table->AddScheduleInfo(si2);
	    	m_Table->SetDirtyFlag(TRUE);
		}
	
		if (nbNewRecord) 
		{
			AddSchduleInfoToList(si2);
		}
		else 
			ModifySchduleInfoToList(si,si2);

		if (m_Table)
		{
    		m_Table->GetRecordCount(g_numTotalCalendars);
			::SendMessage(afxGetMainWnd()->GetSafeHwnd(), WM_REFRESH_LEFT_PANEL, (DWORD)true, NULL);
		}
/*
		//Added by Nono
		if (si2.haveAlarm)
    		((CCAMMScheduleDayView *)GetParent())->SetAlert(si2);
*/

		return TRUE;
	}

	return FALSE;
}

void CCAMMScheduleList::GetScheduleInfo(int nItem,CScheduleInfo &si) 
{
	if ((nItem < 0) || (nItem >= m_ScheduleList.GetCount())) 
	{
		return;
	}
//	si = *((CScheduleInfo*)m_ScheduleList.GetAt(m_ScheduleList.FindIndex(nItem)));
	si = *((CScheduleInfo*)GetListCtrl().GetItemData(nItem));
	//test
// 	CString strll;
// 	strll.Format(L"检测GetScheduleInfo（）\n查看得到的重复类型\nsi.repeatType=%d", si.repeatType);
// 	AfxMessageBox(strll);
}

void CCAMMScheduleList::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	bool bMSOT = false;
  	CMainFrame* pFrame =(CMainFrame* ) afxGetMainWnd();
	CString strTreeMode = pFrame->GetTreeSelectMode();
	if(strTreeMode.Compare(_T("MSOUTLOOK")) == 0)
		bMSOT = true;

	CListCtrl& theCtrl = GetListCtrl();

	POSITION pos = theCtrl.GetFirstSelectedItemPosition();
	//Calendar
	if (m_bMobileConnected == TRUE || bMSOT)
	{
		//当没有项目被选中时
    	if (pos == NULL)
		{
	       TRACE0("No items were selected!\n");
		   //连接手机成功，则新建editdlg 
		   if(bMSOT == false && pFrame->m_bMobileGetFail == false)
		      xCreateScheduleItem();	//调用这个函数 show edit dlg
		} 
		else //如果有项目被选中，编辑之
		{
    		int nItem = theCtrl.GetNextSelectedItem(pos);	//得到选中的项目编号
// 			CString st;
// 			st.Format(L"Item %d was selected!\n", nItem);
// 			AfxMessageBox(st);
//	    	TRACE1("Item %d was selected!\n", nItem);
		    xEditScheduleItem(nItem);	//打开editdlg，传递的参数为进度表项目的编号
		}	
	}
	else 
	{
//		AfxMessageBox("Lose connecting to mobile!\nPlease check the connecting status of mobile. ");
		CalendarMessageBox(m_hWnd,_T("MSG_ERROR_DISCONNECT"));
	}

	
	CListView::OnLButtonDblClk(nFlags, point);

}

// void CCAMMScheduleList::OnRButtonDown(UINT nFlags, CPoint point) 
// {
// /*	// TODO: Add your message handler code here and/or call default
// 	CMenu menu;
// 	menu.LoadMenu(IDR_MENU_LISTRIGHTKEY);
// 	POINT pt;
// 	GetCursorPos(&pt);
// 	PostMessage(WM_LBUTTONDOWN, MK_LBUTTON, (LPARAM)pt);
// 	CListCtrl& theCtrl = GetListCtrl();	
// 	POSITION pos = theCtrl.GetFirstSelectedItemPosition();
// 
// 	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, (CWnd*)this);
// 	//CListView::OnRButtonDown(nFlags, point);
// 	*/
// }

void CCAMMScheduleList::OnAddCalendar() 
{
	// TODO: Add your message handler code here and/or call default
    CListCtrl& theCtrl = GetListCtrl();

	POSITION pos = theCtrl.GetFirstSelectedItemPosition();
	if (pos == NULL) 
	{
	   TRACE0("No items were selected!\n");
	   xCreateScheduleItem();
	} 
	else 
	{
		int nItem = theCtrl.GetNextSelectedItem(pos);
//		TRACE1("Item %d was selected!\n", nItem);
		xEditScheduleItem(nItem);
	}	
}

CAMMError CCAMMScheduleList::xEditScheduleItem(INT nItem)
{

	CAMMError Ret = CAMM_ERROR_SUCCESS;

	CScheduleInfo si,si2;
	GetScheduleInfo(nItem,si);

	//得到项目nItem 放到si中
// 	AfxMessageBox(L"edit scheduleitem");
// 	CString strllll;
// 	strllll.Format(L"xEditScheduleItem 里面si.repeatType ：%d", si.repeatType);
// 	AfxMessageBox(strllll);
	if (xShowEditorDlg(si,si2))		//显示editdlg 里面的内容为si
	{
/*      //For Calendar
        CCAMMSync cs;
        if (! cs.EditCalendar(si2) ) {
			AfxMessageBox("Failed to edit a new Calendar!\nPlease check the satus of connecting mobile.");
            return CAMM_ERROR_FAILURE;
		}else {
			AfxMessageBox("Succeed to edit a new Calendar!.", MB_OK);
		}	
*/
//		Initialize();
	}

	return Ret;
}


void CCAMMScheduleList::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	INT nItem=0;
	if (pNMListView) {
	    nItem = pNMListView->iItem;
	}

	if ((nItem < 0) || (nItem >= GetListCtrl().GetItemCount())) 
	{
		TRACE0("No items were selected!\n");
		m_bInDataSection = FALSE;
		m_nCurrentItem = GetListCtrl().GetItemCount();
	} 
	else 
	{
		m_bInDataSection = TRUE;
		m_nCurrentItem = nItem;
/*		//kerm add for 9@9u right pop menu
		CMenu menu;
		//menu.LoadMenu(IDR_MENU_LISTRIGHTKEY);
		menu.CreateMenu();
		menu.InsertMenu(0, )
		POINT pt;
		GetCursorPos(&pt);
		CListCtrl& theCtrl = GetListCtrl();	
		POSITION pos = theCtrl.GetFirstSelectedItemPosition();
		
		menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, (CWnd*)this);
		//end pop menu*/
	}
	POINT ptAction;
	if (pNMListView)
    	ptAction = pNMListView->ptAction;

	ClientToScreen(&ptAction);
	
	//((CCAMMScheduleDayView *)m_ScheduleView)->ShowScheduleMenu(ptAction);

	*pResult = 0;
}

void CCAMMScheduleList::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	//m_Menu.LoadMenu(IDR_ORG_SCH_DAILY);
	
}


void CCAMMScheduleList::OnOrgSchEdit() 
{
	// TODO: Add your command handler code here
	xEditScheduleItem(m_nCurrentItem);

}

void CCAMMScheduleList::OnUpdateOrgSchEdit(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_bInDataSection);
	CString txt = LoadStringFromFile(_T("public"),_T("Edit"));
	ASSERT(&txt);
	pCmdUI->SetText(txt);

	
}

void CCAMMScheduleList::xDeleteScheduleItem() 
{
/*	int nIdx;
	CListCtrl& theCtrl = GetListCtrl();
	POSITION pos = theCtrl.GetFirstSelectedItemPosition();
	CScheduleInfo si;
	CList<CScheduleInfo,CScheduleInfo> DeleteItemList;
	DeleteItemList.RemoveAll();
	while (pos)
	{
		nIdx = theCtrl.GetNextSelectedItem(pos);
		GetScheduleInfo(nIdx,si);
		//Calendar
		int nIndexDeleted = (int) si.uiIndex;
		DeleteItemList.AddTail(si);
		CCAMMSync sc;
		int ret = sc.DeleteCalendar(si);

		m_Table->RemoveScheduleInfo(si);
	}
	pos = DeleteItemList.GetHeadPosition();
	while(pos)
	{
		RemoveSchduleInfoInList(DeleteItemList.GetNext(pos));	
	}
	*/
	CProcessDlg ProgressDlg;
	ProgressDlg.SetDeleteInfo(this);
	ProgressDlg.DoModal();

	if (m_Table)
	{
     	m_Table->SetDirtyFlag(TRUE);
    	m_Table->GetRecordCount(g_numTotalCalendars);
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(), WM_REFRESH_LEFT_PANEL, (DWORD)true, NULL);
	}


}

void CCAMMScheduleList::OnOrgSchDelete() 
{
	// TODO: Add your command handler code here
	BOOL bCanDelete = TRUE;
	::SendMessage(afxGetMainWnd()->GetSafeHwnd(), WM_CHECK_DELETE,NULL,(LPARAM)(&bCanDelete));
	if(bCanDelete)
	{
		CString  s = LoadStringFromFile(_T("message"),_T("MSG_DELETE"));
		int selectedCount = GetListCtrl().GetSelectedCount();
		if (selectedCount > 0)
		{
			CString Msg;
			Msg.Format(s,selectedCount);
		
			if(CalendarStringMessageBox(m_hWnd,Msg,MB_YESNO|MB_ICONQUESTION) == IDYES)
			{
				xDeleteScheduleItem();
			}
		}
	}
}

void CCAMMScheduleList::OnUpdateOrgSchDelete(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
		
	pCmdUI->Enable(m_bInDataSection);
	CString txt = LoadStringFromFile(_T("public"),_T("Delete"));
	ASSERT(&txt);
	pCmdUI->SetText(txt);
	
}

void CCAMMScheduleList::OnUpdateOrgSchSave(CCmdUI* pCmdUI) 
{
	((CCAMMScheduleDayView *)m_ScheduleView)->OnUpdateOrgSchSave(pCmdUI);
}

void CCAMMScheduleList::OnOrgSchSave() 
{
	((CCAMMScheduleDayView *)m_ScheduleView)->OnOrgSchSave();
}

void CCAMMScheduleList::OnOrgSchSaveas() 
{
	((CCAMMScheduleDayView *)m_ScheduleView)->OnOrgSchSaveas();
}

void CCAMMScheduleList::OnUpdateOrgSchSaveas(CCmdUI* pCmdUI) 
{
	((CCAMMScheduleDayView *)m_ScheduleView)->OnUpdateOrgSchSaveas(pCmdUI);
}

void CCAMMScheduleList::OnOrgSchOpen() 
{
	((CCAMMScheduleDayView *)m_ScheduleView)->OnOrgSchOpen();
}

void CCAMMScheduleList::OnUpdateOrgSchOpen(CCmdUI* pCmdUI) 
{
	((CCAMMScheduleDayView *)m_ScheduleView)->OnUpdateOrgSchOpen(pCmdUI);
}

void CCAMMScheduleList::OnOrgSchNew() 
{
	((CCAMMScheduleDayView *)m_ScheduleView)->OnOrgSchNew();
}

void CCAMMScheduleList::OnUpdateOrgSchNew(CCmdUI* pCmdUI) 
{
	((CCAMMScheduleDayView *)m_ScheduleView)->OnUpdateOrgSchNew(pCmdUI);
}

void CCAMMScheduleList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
		case VK_DOWN:
			GetListCtrl().GetNextItem(m_nCurrentItem,LVNI_BELOW);
			break;

		case VK_UP:
			GetListCtrl().GetNextItem(m_nCurrentItem,LVNI_ABOVE);
			break;

		case VK_DELETE:
			OnOrgSchDelete();
			break;

		default:
			return ;
	}
	
	
	CListView::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CCAMMScheduleList::xCopyDataToClipboard(CString cpStr) {
    HGLOBAL hglbCopy; 
    LPTSTR  lptstrCopy; 

    if (!::OpenClipboard(afxGetMainWnd()->GetSafeHwnd())) {
        return FALSE; 
	}
    ::EmptyClipboard(); 
	
    hglbCopy = GlobalAlloc(GMEM_MOVEABLE,(_tcslen((LPCTSTR)cpStr)+1) * sizeof(TCHAR)); 
    if (hglbCopy == NULL) 
    { 
        ::CloseClipboard(); 
        return FALSE; 
    } 

    lptstrCopy = (LPTSTR)GlobalLock(hglbCopy); 
    memcpy(lptstrCopy, (LPCTSTR)cpStr, (_tcslen((LPCTSTR)cpStr)) * sizeof(TCHAR)); 
    lptstrCopy[_tcslen((LPCTSTR)cpStr)] = (TCHAR) 0;    // null character 
    GlobalUnlock(hglbCopy); 

    // Place the handle on the clipboard. 

    ::SetClipboardData(m_ClipboardFormatId, hglbCopy); 
    ::CloseClipboard(); 

	return TRUE;
}

BOOL CCAMMScheduleList::xGetDataFromClipboard(CString &cpStr) {
    HGLOBAL hglb; 
    LPTSTR    lptstr; 

    if (!::OpenClipboard(afxGetMainWnd()->GetSafeHwnd())) {
        return FALSE; 
	}
    if (!::IsClipboardFormatAvailable(m_ClipboardFormatId)) {
		return FALSE;
	}
	
    hglb = ::GetClipboardData(m_ClipboardFormatId); 
    if (hglb != NULL) 
    { 
        lptstr = (LPTSTR)GlobalLock(hglb); 
        if (lptstr != NULL) 
        { 
            // Call the application-defined ReplaceSelection 
            // function to insert the text and repaint the 
            // window. 
			cpStr = lptstr;
            GlobalUnlock(hglb); 
        } else {
			return FALSE;
		}
    } 
    ::CloseClipboard(); 

	return TRUE;
}

void CCAMMScheduleList::OnOrgSchCut() 
{
	CString cpStr;
	if (xGetSelectedClipboardString(cpStr)) {
		if (xCopyDataToClipboard(cpStr)) {
			xDeleteScheduleItem();
		}
	}
}

void CCAMMScheduleList::OnUpdateOrgSchCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bInDataSection);
}

void CCAMMScheduleList::OnOrgSchCopy() 
{
	CString cpStr;
	if (xGetSelectedClipboardString(cpStr)) {
		xCopyDataToClipboard(cpStr);
	}
}

void CCAMMScheduleList::OnUpdateOrgSchCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bInDataSection);
}

void CCAMMScheduleList::OnOrgSchPaste() 
{
	CString cpStr;
	if (xGetDataFromClipboard(cpStr)) {
		xPasteData(cpStr);
	}
}

void CCAMMScheduleList::OnUpdateOrgSchPaste(CCmdUI* pCmdUI) 
{
    if (!::OpenClipboard(afxGetMainWnd()->GetSafeHwnd())) {
		pCmdUI->Enable(FALSE);
        return; 
	}
    if (::IsClipboardFormatAvailable(m_ClipboardFormatId)) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
    ::CloseClipboard(); 
}

BOOL CCAMMScheduleList::xGetSelectedClipboardString(CString &cpStr) {
	int nIdx;
	CListCtrl& theCtrl = GetListCtrl();
	POSITION pos = theCtrl.GetFirstSelectedItemPosition();
	CScheduleInfo si;
	while (pos) {
		nIdx = theCtrl.GetNextSelectedItem(pos);
//		si = *((CScheduleInfo*) m_ScheduleList.GetAt(m_ScheduleList.FindIndex(nIdx)));
		GetScheduleInfo(nIdx,si);
		cpStr += Object_Seperator;
		cpStr += si.GetClipboardString();
	}

	if (!cpStr.IsEmpty()) {
		cpStr += Object_Seperator;
		return TRUE;
	}
	
	return FALSE;
}

BOOL CCAMMScheduleList::xPasteData(CString &cpStr) {

	CListCtrl& theCtrl = GetListCtrl();
	CScheduleInfo si2;
	CString sep = Object_Seperator;
	int pos=0,pos2=0;
	int idx=0;

	pos2 = cpStr.Find(sep,pos);
	if (pos2 < 0) {
		return FALSE;
	}
	pos2 += sep.GetLength();
	if (pos2 >= cpStr.GetLength()) {
		return FALSE;
	}
	CList<CScheduleInfo,CScheduleInfo> pbList;
	while (pos2 > 0) {
		pos = pos2;
		pos2 = cpStr.Find(sep,pos);
		if (pos2 == -1) {
			break;
		}
		si2.ParseClipboardString(cpStr.Mid(pos,pos2-pos));
		pbList.AddTail(si2);
		pos2 += sep.GetLength();
		if (pos2 >= cpStr.GetLength()) {
			break;
		}
	}

	if (xCheckRecordMaxLimit(pbList.GetCount())) {
		return FALSE;
	}

	for (int i=0;i<pbList.GetCount();i++) {
		si2 = pbList.GetAt(pbList.FindIndex(i));
		if (m_Table)
    		m_Table->AddScheduleInfo(si2);
	}
	pbList.RemoveAll();

	Initialize();

	SCROLLINFO si;
	GetScrollInfo(SB_VERT,&si,SIF_ALL);
	if (theCtrl.GetItemCount() > si.nPage) {
		CSize size;
		size.cx=0;
		RECT rt;
		theCtrl.GetItemRect(theCtrl.GetItemCount()-1,&rt,LVIR_LABEL );
		size.cy = rt.bottom;
		theCtrl.Scroll(size);

		SendNotifyMessage(WM_VSCROLL,MAKEWPARAM(SB_BOTTOM,si.nPos),NULL);
		SendNotifyMessage(WM_VSCROLL,MAKEWPARAM(SB_ENDSCROLL,si.nPos),NULL);
		SendNotifyMessage(WM_VSCROLL,MAKEWPARAM(SB_BOTTOM,si.nPos),NULL);
		SendNotifyMessage(WM_VSCROLL,MAKEWPARAM(SB_ENDSCROLL,si.nPos),NULL);
		SendNotifyMessage(WM_VSCROLL,MAKEWPARAM(SB_BOTTOM,si.nPos),NULL);
		SendNotifyMessage(WM_VSCROLL,MAKEWPARAM(SB_ENDSCROLL,si.nPos),NULL);
	}

	if (m_Table)
    	m_Table->SetDirtyFlag(TRUE);

	return TRUE;
}


BOOL CCAMMScheduleList::GetDataSectionFlag()
{
	return m_bInDataSection;
}

UINT CCAMMScheduleList::GetClipboardFormatID()
{
	return m_ClipboardFormatId;
}

void CCAMMScheduleList::OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	POSITION pos = GetListCtrl().GetFirstSelectedItemPosition();
	int nItem = GetListCtrl().GetNextSelectedItem(pos);
	
	if ((nItem < 0) || (nItem >= GetListCtrl().GetItemCount())) 
	{
		m_bInDataSection = FALSE;
		m_nCurrentItem = GetListCtrl().GetItemCount();
	} 
	else 
	{
		m_bInDataSection = TRUE;
		m_nCurrentItem = nItem;
	}
	
	*pResult = 0;
}




void CCAMMScheduleList::AutoSizeColumns()
{      
   CListCtrl& listCtrl = GetListCtrl();
   listCtrl.SetRedraw(false);

   CHeaderCtrl* pHeaderCtrl = listCtrl.GetHeaderCtrl();
   ASSERT(pHeaderCtrl != NULL);
   for (int i=0; i < pHeaderCtrl->GetItemCount(); i++)
      listCtrl.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);

   listCtrl.SetRedraw(true);
}

int CCAMMScheduleList::PrintRecord()
{
    AutoSizeColumns();
/*	
	if (m_pPrintRecord) {
		m_pPrintRecord->SetListCtrl(&GetListCtrl());
		m_pPrintRecord->SetListView(this);
		m_pPrintRecord->SetAppName(AfxGetAppName());
		//CString cuDocName;
		//cuDocName = "[ ";
		//cuDocName += _DocTitle_;
		//cuDocName += " ]";
		//m_pPrintRecord->SetDocTitle((LPCTSTR)cuDocName);
		static_nPrint_Record_Mode = Print_Record_List;
    	OnFilePrintPreview();

		return 1;
	}
*/
	return 0;
}

BOOL CCAMMScheduleList::OnPreparePrinting(CPrintInfo* pInfo)
{/*
	switch(static_nPrint_Record_Mode) {
    case Print_Record_List: 
		{
			if (m_pPrintRecord) {
                m_pPrintRecord->OnPreparePrinting(pInfo);
			}
		}

		break;
	default:
		break;
	}
*/   return DoPreparePrinting(pInfo);
}

void CCAMMScheduleList::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{/*   
	switch(static_nPrint_Record_Mode) {
    case Print_Record_List: 
		{
			if (m_pPrintRecord) {
                m_pPrintRecord->OnBeginPrinting(pDC, pInfo);
			}
		}
		break;
	default:
		break;
	}*/
}

void CCAMMScheduleList::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{/*
	switch(static_nPrint_Record_Mode) {
    case Print_Record_List: 
		{
			if (m_pPrintRecord) {
                m_pPrintRecord->OnPrint(pDC, pInfo);
			}
		}
		break;
	default:
		break;
	}*/
}

void CCAMMScheduleList::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{/*
	switch(static_nPrint_Record_Mode) {
    case Print_Record_List: 
		{
			if (m_pPrintRecord) {
                m_pPrintRecord->OnEndPrinting(pDC, pInfo);
			}
		}
		break;
	default:
		break;
	}*/
}

void CCAMMScheduleList::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	if ( m_bFlag == FALSE )
        m_bFlag = TRUE ;
    else
        m_bFlag = FALSE ;
	GetListCtrl( ).SortItems ( compare, pNMListView -> iSubItem );

	*pResult = 0;

}


//Added by Nono <=
void CCAMMScheduleList::FreeScheduleList()
{
	POSITION pos = m_ScheduleList.GetHeadPosition();
	while(pos)
	{
		CScheduleInfo* pdata =(CScheduleInfo* ) m_ScheduleList.GetNext(pos);
		if(pdata)
			delete pdata;
	}
	m_ScheduleList.RemoveAll();
}

void CCAMMScheduleList::AddSchduleInfoToList(CScheduleInfo info)
{
    CListCtrl& theCtrl = GetListCtrl();
	CScheduleInfo *psi = new CScheduleInfo;
	psi->SetScheduleInfoData(info);


	if(psi->dtStartDate.GetStatus() == COleDateTime::invalid)
	{
		delete psi;
	
		return ;
	}

	m_ScheduleList.AddTail(psi);
	LV_ITEM lvColItems;
	int iImageIndex = 0;


/**/	for(int i =0 ;i<m_SizeofSupportNoteType ; i++)
	{
		if(m_ArrayofSupportNoteType[i].nNodeType == psi->uiNodeType)
		{
			iImageIndex = i;
			break;
		}
	}
/*switch(psi->uiNodeType)
	{
	case GSM_CAL_REMINDER:
		iImageIndex = 0;
		break;
	case GSM_CAL_CALL:
		iImageIndex = 1;
		break;
	case GSM_CAL_MEETING:
		iImageIndex = 2;
		break;
	case GSM_CAL_BIRTHDAY:
		iImageIndex = 3;
		break;
	case GSM_CAL_MEMO:
		iImageIndex = 4;
		break;
	}*/

	lvColItems.mask = LVIF_TEXT|LVIF_IMAGE;
	lvColItems.iItem = theCtrl.GetItemCount();
	lvColItems.iSubItem = 0;
	lvColItems.iImage = iImageIndex;
	lvColItems.pszText = NULL;//new TCHAR[MAX_LENGTH];
	//		memset(lvColItems.pszText,0x00,MAX_LENGTH);
	//		lstrcpy(lvColItems.pszText,si.csTitle);
	lvColItems.lParam = NULL;
	int nIndex = theCtrl.InsertItem(&lvColItems);
	theCtrl.SetItemData(nIndex,(unsigned long)psi);

	theCtrl.SetItemText(nIndex,0,psi->csTitle);
  	theCtrl.SetItemText(nIndex,1,psi->dtStartDate.Format(VAR_DATEVALUEONLY));
	theCtrl.SetItemText(nIndex,2,psi->hiRealStartTime.FormatString());
	theCtrl.SetItemText(nIndex,3,psi->hiRealStopTime.FormatString());
	

	if (psi->haveRepeat) 
	{
		//CString StringLoaded = LoadStringFromFile(ghSkinResource,IDS_ORG_SCH_INTERVALUNIT0 + si.repeatType);
        TCHAR szBuffer[10], szRepeatType[32];
		int nRepeat = psi->repeatType;
		//去掉下面两种类型，kerm for 9a9u 1.11
/*		if(nRepeat == WEEKLY && psi->nDayofWeekMask == 62)
			nRepeat = 7;
		else if(nRepeat == WEEKLY && psi->nDayofWeekMask == 65)
			nRepeat = 6;*/
		
		//增加对自定义日的处理，有两个条件：repeat==weekly nDayofWeekMask!=0
		//上面是工作日重复，特点是nDayofWeekMask==62      kerm add for 9a9u 08.1.04
// 		if(nRepeat == WEEKLY && psi->nDayofWeekMask != 0 )
// 		{	nRepeat = 8;	
// 		}080613libaoliu
		

// 		CString strlllll;
// 		strlllll.Format(L"看看重复类型repeat type=%d", nRepeat);
//  	 	AfxMessageBox(strlllll);

		_itot(nRepeat, szBuffer, 10);
		al_GetSettingString(_T("repeattype"), szBuffer, theApp.m_szRes,szRepeatType);
		theCtrl.SetItemText(nIndex,4,szRepeatType);
	} 
	else 
	{

		CString  s = LoadStringFromFile(_T("public"),_T("none"));
		theCtrl.SetItemText(nIndex,4,s);
	}
	
	//在这里增加对优先级的显示，跟initialize中的类似
	/*//080612libaoliu
	CString StringLoaded;
	switch(psi->uiPrivate)		//kerm add for 9@9u 07.12.12
	{
	case 1:
		StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_STATIC_PRIORITY1"));
		theCtrl.SetItemText(nIndex,5,StringLoaded);	
		break;
	case 2:
		StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_STATIC_PRIORITY2"));
		theCtrl.SetItemText(nIndex,5,StringLoaded);	
		break;
	case 3:
		StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_STATIC_PRIORITY3"));
		theCtrl.SetItemText(nIndex,5,StringLoaded);	
		break;
	}
	*/
//	theCtrl.SetItemState(nIndex, LVIS_SELECTED|LVIS_FOCUSED , LVIS_SELECTED|LVIS_FOCUSED);


}

void CCAMMScheduleList::RemoveSchduleInfoInList(CScheduleInfo si)
{
    CListCtrl& theCtrl = GetListCtrl();
	int nCount = theCtrl.GetItemCount();
	for(int i = 0 ; i<nCount ;i++)
	{
		CScheduleInfo* schduleinfo = (CScheduleInfo*)theCtrl.GetItemData(i);
	//	if(schduleinfo->uiIndex == si.uiIndex)
		if(_stricmp(schduleinfo->szuiIndex,si.szuiIndex) == 0)
		{
			theCtrl.DeleteItem(i);
			break;
		}
	}
}

void CCAMMScheduleList::ModifySchduleInfoToList(CScheduleInfo oldinfo,CScheduleInfo newinfo)
{
    CListCtrl& theCtrl = GetListCtrl();
	int nCount = theCtrl.GetItemCount();
	for(int i = 0 ; i<nCount ;i++)
	{
		CScheduleInfo* schduleinfo = (CScheduleInfo*)theCtrl.GetItemData(i);
//		if(schduleinfo->uiIndex == oldinfo.uiIndex)
		if(_stricmp(schduleinfo->szuiIndex,oldinfo.szuiIndex) == 0)
		{
			schduleinfo->SetScheduleInfoData(newinfo);

			LV_ITEM lvColItems;
			int iImageIndex =0;

			for(int j =0 ;j<m_SizeofSupportNoteType ; j++)
			{
				if(m_ArrayofSupportNoteType[j].nNodeType == schduleinfo->uiNodeType)
				{
					iImageIndex = j;
					break;
				}
			}
		/*	switch(schduleinfo->uiNodeType)
			{
			case GSM_CAL_REMINDER:
				iImageIndex = 0;
				break;
			case GSM_CAL_CALL:
				iImageIndex = 1;
				break;
			case GSM_CAL_MEETING:
				iImageIndex = 2;
				break;
			case GSM_CAL_BIRTHDAY:
				iImageIndex = 3;
				break;
			case GSM_CAL_MEMO:
				iImageIndex = 4;
				break;
			}*/

			lvColItems.mask = LVIF_IMAGE;
			lvColItems.iItem = i;
			lvColItems.iSubItem = 0;
			lvColItems.iImage = iImageIndex;
			lvColItems.pszText = NULL;
			lvColItems.lParam = NULL;
			theCtrl.SetItem(&lvColItems);

			theCtrl.SetItemText(i,0,schduleinfo->csTitle);
  			theCtrl.SetItemText(i,1,schduleinfo->dtStartDate.Format(VAR_DATEVALUEONLY));
			theCtrl.SetItemText(i,2,schduleinfo->hiRealStartTime.FormatString());
			theCtrl.SetItemText(i,3,schduleinfo->hiRealStopTime.FormatString());


			//在这里增加对优先级的显示，跟initialize中的类似
			/*//080612libaoliu
			CString StringLoaded;
			switch(schduleinfo->uiPrivate)		//kerm add for 9@9u 08.1.07
			{
			case 1:
				StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_STATIC_PRIORITY1"));
				theCtrl.SetItemText(i,5,StringLoaded);	
				break;
			case 2:
				StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_STATIC_PRIORITY2"));
				theCtrl.SetItemText(i,5,StringLoaded);	
				break;
			case 3:
				StringLoaded = LoadStringFromFile(_T("CalendarList"),_T("LIST_STATIC_PRIORITY3"));
				theCtrl.SetItemText(i,5,StringLoaded);	
				break;
			}
			*/

			if (schduleinfo->haveRepeat) 
			{
				//CString StringLoaded = LoadStringFromFile(ghSkinResource,IDS_ORG_SCH_INTERVALUNIT0 + si.repeatType);
				TCHAR szBuffer[10], szRepeatType[32];
				
				int nRepeat = schduleinfo->repeatType;
				//暂时不用下面两种类型 去掉 kerm change for 9a9u 1.05
				/**/
				if(nRepeat == WEEKLY && schduleinfo->nDayofWeekMask == 62)
					nRepeat = 7;
				else if(nRepeat == WEEKLY && schduleinfo->nDayofWeekMask == 65)
					nRepeat = 6;

				//需要增加代码处理列表的重复类型显示问题，kerm add for 9a9u 1.05
				if(nRepeat == WEEKDAYS && schduleinfo->nDayofWeekMask != 0)
				{	nRepeat = 8;}

				_itot(nRepeat, szBuffer, 10);
				al_GetSettingString(_T("repeattype"), szBuffer, theApp.m_szRes,szRepeatType);
				theCtrl.SetItemText(i,4,szRepeatType);
			} 
			else 
			{

				CString  s = LoadStringFromFile(_T("public"),_T("none"));
				theCtrl.SetItemText(i,4,s);
			}
			break;
		}
	}
}void CCAMMScheduleList::RemoveScheduleInfo(CScheduleInfo &si,bool bNeedUpdate)
{
	m_Table->RemoveScheduleInfo(si,bNeedUpdate);
}
void CCAMMScheduleList::UpdateScheduleData()
{
	m_Table->UpdateScheduleData();
}
/*
void CCAMMScheduleList::OnEndtrack(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	// TODO: Add your control notification handler code here
	 CListCtrl& theCtrl = GetListCtrl();
   theCtrl.Invalidate();
   *pResult = 0;
} 
*/

void CCAMMScheduleList::SetImageList()
{
	if(m_bCreateImageList)
		return;
	CCAMMSync sc;

	if (sc.GetCalendarSupport(m_SizeofSupportNoteType, m_ArrayofSupportNoteType))
	{
		m_ImageList.Create(16 ,16 ,ILC_COLOR24 | ILC_MASK,m_SizeofSupportNoteType,0);
		for(int i =0 ;i<m_SizeofSupportNoteType ; i++)
		{
			HICON hIcon;
			TCHAR szIconFileName[MAX_PATH];
			wsprintf(szIconFileName, _T("%s%s"), theApp.m_szSkin,m_ArrayofSupportNoteType[i].szIconfileName);
			hIcon =(HICON) LoadImage(NULL,szIconFileName,IMAGE_ICON,NULL,NULL,LR_LOADFROMFILE);
			m_ImageList.Add(hIcon);
		}
	}
	else
	{
		m_ImageList.Create(16 ,16 ,ILC_COLOR24 | ILC_MASK,5,0);
		HICON hIcon;
		
		hIcon = afxGetApp()->LoadIcon(IDI_REMIND);
		m_ImageList.Add(hIcon);

		hIcon = afxGetApp()->LoadIcon(IDI_PHONE);
		m_ImageList.Add(hIcon);
		hIcon = afxGetApp()->LoadIcon(IDI_METTING);
		m_ImageList.Add(hIcon);
		hIcon = afxGetApp()->LoadIcon(IDI_BIRTHDAY);
		m_ImageList.Add(hIcon);
		hIcon = afxGetApp()->LoadIcon(IDI_MEMO);
		m_ImageList.Add(hIcon);
	}

	GetListCtrl().SetImageList(&m_ImageList,LVSIL_SMALL);
	m_bCreateImageList = TRUE;

}

void CCAMMScheduleList::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
   	COleDataSource ds;

	if(ds.DoDragDrop() == DROPEFFECT_MOVE )
	{
	}
}
