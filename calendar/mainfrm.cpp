// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Calendar.h"

#include "MainFrm.h"

#include "PrevView.h"
#include "leftview.h"
#include "loadstring.h"
#include "TopView.h"

//#include "MobileDevice\CAMMSync.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define FRM_WIDTH 722
#define FRM_HEIGHT 542

bool static_bMobileConnected=false;
static int static_nCountFailureConecttoMobile=0;


//讽菲公Ω秈跌怠
// OnDragEnter is called by OLE dll's when drag cursor enters
// a window that is REGISTERed with the OLE dll's
//
DROPEFFECT CDropTgt::OnDragEnter(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{
    // if the control key is held down, return a drop effect COPY 
    if((dwKeyState & MK_CONTROL) == MK_CONTROL)
        return DROPEFFECT_NONE;
    // Otherwise return a ?/..drop effect of MOVE
    else
        return DROPEFFECT_MOVE;    
}

//讽菲公跌怠ず,狡砆秸ノ
// OnDragOver is called by OLE dll's when cursor is dragged over 
// a window that is REGISTERed with the OLE dll's
//
DROPEFFECT CDropTgt::OnDragOver(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{     
    if((dwKeyState & MK_CONTROL) == MK_CONTROL)
        return DROPEFFECT_NONE;  
    else
        return DROPEFFECT_MOVE;  // move source
}

//讽菲公簿跌怠
// OnDragLeave is called by OLE dll's when drag cursor leaves
// a window that is REGISTERed with the OLE dll's
//
void CDropTgt::OnDragLeave(CWnd *pWnd)
{
    // Call base class implementation
    COleDropTarget::OnDragLeave(pWnd);
}

//讽菲公跌怠ず辅 
// OnDrop is called by OLE dll's when item is dropped in a window
// that is REGISTERed with the OLE dll's
//
BOOL CDropTgt::OnDrop(CWnd *pWnd, COleDataObject *pDataObject, DROPEFFECT dropEffect, CPoint point)
{
    // If the dropEffect requested is not a MOVE, return FALSE to 
    // signal no drop. (No COPY into trashcan)
    if((dropEffect & DROPEFFECT_MOVE) != DROPEFFECT_MOVE)
        return FALSE;

    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_COMMAND(ID_BN_NEW,OnNew)
	ON_COMMAND(ID_BN_EDIT,OnEdit)
	ON_COMMAND(ID_BN_DELETE,OnDelete)
	ON_COMMAND(ID_BN_SEARCH,OnSearch)
	ON_COMMAND(ID_BN_BACKUP,OnBackup)
	ON_COMMAND(ID_BN_RESTORE,OnRestore)
	ON_COMMAND(ID_BN_REFRESH,OnRefresh)
	ON_COMMAND(ID_BN_PHONEBOOK,OnPhonebook)
	ON_COMMAND(ID_BN_SCHEDULE,OnSchedule)
	ON_COMMAND(ID_BN_MEMO,OnMemo)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_BN_BACKUP, OnUpdateBnBackup)
	ON_UPDATE_COMMAND_UI(ID_BN_DELETE, OnUpdateBnDelete)
	ON_UPDATE_COMMAND_UI(ID_BN_EDIT, OnUpdateBnEdit)
	ON_UPDATE_COMMAND_UI(ID_BN_NEW, OnUpdateBnNew)
	ON_UPDATE_COMMAND_UI(ID_BN_REFRESH, OnUpdateBnRefresh)
	ON_UPDATE_COMMAND_UI(ID_BN_RESTORE, OnUpdateBnRestore)
	ON_UPDATE_COMMAND_UI(ID_BN_SEARCH, OnUpdateBnSearch)
	ON_COMMAND(ID_BN_EXIT,OnExit)
	ON_COMMAND(ID_BN_HELP,OnHelp)
	ON_UPDATE_COMMAND_UI(ID_BN_EXIT, OnUpdateBnExit)
	ON_UPDATE_COMMAND_UI(ID_BN_HELP, OnUpdateBnHelp)
	ON_UPDATE_COMMAND_UI(ID_BN_ABOUT, OnUpdateBnAbout)
	ON_WM_ERASEBKGND()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_CONNECT, OnUpdateStatusConnect)	
	ON_MESSAGE(WM_CARDPANEL_GETGROUP,OnCardPanelGetGroup)
	ON_MESSAGE(WM_CARDPANEL_GROUPCOUNT,OnCardPanelGroupCount)
	ON_MESSAGE(WM_LEFT_CHGMODE,OnLeftChangeMode)
	ON_MESSAGE(WM_GETLEFTMODE,OnGetLeftMode)
	ON_MESSAGE(WM_ERR_OPENSYNC,OnErrOpenSync)
	ON_MESSAGE(WM_GETSYNCFLAG,OnGetSyncflag)
	ON_MESSAGE(WM_LEFT_DROPDATA,OnLeftDropData)
	ON_MESSAGE(WM_SETLOADFILE,OnSetLoadFileFlag)
	ON_MESSAGE(WM_SETLINKMOBILE,OnSetLinkMobile)
	ON_MESSAGE(WM_SEARCH_SETFLAG,OnSetSearchFlag)
	ON_MESSAGE(WM_MOBILECONNECTED, OnLoadCalendar)
	ON_MESSAGE(WM_LOADCALENDAR, OnReload)
    ON_MESSAGE(WM_REFRESH_LEFT_PANEL, OnRefreshLeftView)
    ON_MESSAGE(WM_CHECK_LEFTTREEITEM, OnCheckTreeItem)
    ON_MESSAGE(WM_CHECK_DELETE, OnCheckDelete)
	//ON_MESSAGE(WM_MOBILECONNECTED, OnAddCalendar)
	ON_MESSAGE(WM_OPENCALENDAR, OnOpenCalendar)
	ON_MESSAGE(WM_SETLEFTITEMCOUNT,OnSetLeftItemCount)
	ON_MESSAGE(WM_LEFT_DROPDATATOMSOT,OnLeftDropData2MSOT)
	ON_MESSAGE(WM_LEFT_UPDATEDATA,OnLeftUpdateData)
END_MESSAGE_MAP()

static UINT indicators[] =	//状态栏的定义
{
	ID_CONNECT,
/*	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
*/
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction
int CMainFrame::m_iStatus = 0;	//全局变量， 表示？？状态

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	//定义三个功能id 电话本 进度表 存储 分别放到m_nFuncID[i]中
	UINT nFuncID[FUNC_BN_NUMBER] = {ID_BN_PHONEBOOK,ID_BN_SCHEDULE,ID_BN_MEMO};
	for(int i = 0 ; i < FUNC_BN_NUMBER ; i ++)
		m_nFuncID[i] = nFuncID[i];
	//同步为true   搜索为false
	m_bOpenSync = true;
	m_bSearch = false;
	//m_ScheduleDayView 为进度表day的视图类
	m_ScheduleDayView = NULL;
	//连接情况初始化为 -1
	m_bMobileConnected = -1;
	m_bCapture = FALSE;
    //DLL CONTRUCT
	m_iStatus = 0;
	//静态的连接标志 初始化为false
    static_bMobileConnected=false;
	//int型全局变量 ？（连接手机的失败次数？）
    static_nCountFailureConecttoMobile=0;
	//？ 连接失败
	m_bMobileGetFail = false;

}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;


	//Empty the menu bar
	SetMenu(NULL);

	//Set profile path
	//指定主界面的文件路径m_sProfile = Skin\\calendar\\main.ini
	m_sProfile = Tstring(theApp.m_szSkin) + Tstring(_T("Calendar\\main.ini"));//nono, 2004_1101
	//-------------------------------------------------------------------

	//kerm add 2.13 
/*	m_RightDlgBar.Create(this, IDD_DLGBAR, CBRS_TOP, IDD_DLGBAR, NULL, RGB(255, 0, 255));
	m_RightDlgBar.m_sizeDefault.cx = 200;
	m_RightDlgBar.m_sizeDefault.cy = 71;
	//增加dlgbar的背景图片 kerm add 2.14
	TCHAR szSkin[MAX_PATH];
	if(al_GetSettingString(_T("dlgbar_top"),_T("bkimage"),const_cast<TCHAR *>(m_sProfile.c_str()),szSkin))
	{
		Tstring sSkin(theApp.m_szSkin);
		sSkin += Tstring(szSkin);
		m_RightDlgBar.SetImgBk(sSkin.c_str(), true);	
	}
*/

	//Create rebar
	//创建rebar然后将两个toolbar放进去
	m_wndRebar.Create(this,RBS_FIXEDORDER );//RBS_FIXEDORDER
	//首先建立left工具条 放入按钮 载入按钮的图片、文字 、tooltip以及工具条的背景图片
	if (!m_wndToolBarLeft.CreateEx(&m_wndRebar, TBSTYLE_BUTTON, WS_CHILD | WS_VISIBLE | CBRS_TOP) ||
		!m_wndToolBarLeft.LoadToolBar(IDR_TOOLBAR_LEFTHANDSIDE))	//your toolbar1 resource id
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	// Create tool bar 
	CMyToolBar& tb = m_wndToolBarLeft;
	VERIFY(tb.LoadToolBar(IDR_TOOLBAR_LEFTHANDSIDE)); 	//your toolbar1 resource id 
	// use transparent so coolbar bitmap will show through
	tb.ModifyStyle(0, TBSTYLE_TRANSPARENT);
	// show button text on right
	tb.ModifyStyle(0, TBSTYLE_LIST);
	//
	CToolBarCtrl& tbc = tb.GetToolBarCtrl();
    DWORD dwStyleEx1 = tbc.GetExtendedStyle();
    tbc.SetExtendedStyle(dwStyleEx1 | TBSTYLE_EX_MIXEDBUTTONS);
    tbc.SetMaxTextRows(1);	
	//Set button image and button count
	TCHAR szSkin[MAX_PATH];
	if(al_GetSettingString(_T("toolbar_left"),_T("image"),const_cast<TCHAR *>(m_sProfile.c_str()),szSkin))
	{
		Tstring sSkin = Tstring(theApp.m_szSkin) + Tstring(szSkin);
		//4是btn的数量，只影响btnicon的大小，不会影响其数量  
		//在toolbar的setbuttonimage中根据btn的数量确定btnicon的大小尺寸，btnicon[4]分别表示工具条的4个状态
		m_wndToolBarLeft.SetButtonImage(sSkin.c_str(), 4);	
	}


	
//	m_LeftDlgBar.Create(this, IDD_DLGBAR, CBRS_LEFT | CBRS_FLYBY, IDD_DLGBAR, NULL, RGB(255, 255, 0));
//	m_LeftDlgBar.m_sizeDefault.cx = 20;
//	m_LeftDlgBar.m_sizeDefault.cy = 200;

	//set tooltip		
	//load your button tooltip from language file
	TCHAR szTipSec[4][32] = {_T("ID_BN_NEW"),_T("ID_BN_EDIT"),_T("ID_BN_DELETE"),_T("ID_BN_REFRESH")};
	for(int i = 0 ,j = 0; i < 4 ; i ++,j++)
	{
		TCHAR szTxt[MAX_PATH];
		if(m_wndToolBarLeft.GetItemID(j) == 0)
			j ++;
		if(al_GetSettingString(_T("public"),szTipSec[i],theApp.m_szRes,szTxt))	
			m_wndToolBarLeft.SetToolTipText(szTxt,j);
	}
	//Set the background image
	//load your toolbar background
	if(al_GetSettingString(_T("toolbar_left"),_T("bkimage"),const_cast<TCHAR *>(m_sProfile.c_str()),szSkin))
	{
		Tstring sSkin(theApp.m_szSkin);
		sSkin += Tstring(szSkin);
		m_wndToolBarLeft.SetImgBk(sSkin.c_str());	//设置背景图片 进入mytoolbar 第二次
	}

	//建立right 工具条 与left的建立相似
	if (!m_wndToolBarRight.CreateEx(&m_wndRebar, TBSTYLE_BUTTON, WS_CHILD | WS_VISIBLE | CBRS_TOP) ||
		!m_wndToolBarRight.LoadToolBar(IDR_TOOLBAR_RIGHTHANDSIDE))	//your toolbar1 resource id
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	// Create tool bar
	CMyToolBar& tb2 = m_wndToolBarRight;
	VERIFY(tb2.LoadToolBar(IDR_TOOLBAR_RIGHTHANDSIDE)); 	//your toolbar1 resource id
	// use transparent so coolbar bitmap will show through
	tb2.ModifyStyle(0, TBSTYLE_TRANSPARENT);
	// show button text on right
	tb2.ModifyStyle(0, TBSTYLE_LIST);
	CToolBarCtrl& tbc2 = tb2.GetToolBarCtrl();
    DWORD dwStyleEx2 = tbc2.GetExtendedStyle();
    tbc2.SetExtendedStyle(dwStyleEx2 | TBSTYLE_EX_MIXEDBUTTONS);
    tbc2.SetMaxTextRows(1);
	//Set button image and button count
	if(al_GetSettingString(_T("toolbar_right"),_T("image"),const_cast<TCHAR *>(m_sProfile.c_str()),szSkin)){
		Tstring sSkin = Tstring(theApp.m_szSkin) + Tstring(szSkin);
	//	m_wndToolBar1.SetButtonImage(sSkin.c_str(), 7);
		m_wndToolBarRight.SetButtonImage(sSkin.c_str(), 3);		//right有3个按钮
	}	
	//set tooltip
	//load your button tooltip from language file
	TCHAR szTipSec1[3][32] = {_T("ID_BN_EXIT"),_T("ID_BN_ABOUT"),_T("ID_BN_HELP")};	//3个按钮的名字
	for( i = 0 ,j = 0; i < 3 ; i ++,j++){
		TCHAR szTxt[MAX_PATH];
		if(m_wndToolBarRight.GetItemID(j) == 0)
			j ++;
		if(al_GetSettingString(_T("public"),szTipSec1[i],theApp.m_szRes,szTxt))
			m_wndToolBarRight.SetToolTipText(szTxt,j);
	}
	//Set the background image
	//load your toolbar background
	if(al_GetSettingString(_T("toolbar_right"),_T("bkimage"),const_cast<TCHAR *>(m_sProfile.c_str()),szSkin))
	{
		Tstring sSkin(theApp.m_szSkin);
		sSkin += Tstring(szSkin);
		m_wndToolBarRight.SetImgBk(sSkin.c_str());
	}

	//测试 加入新的工具条
	if (!m_wndToolBarTest.CreateEx(&m_wndRebar, TBSTYLE_BUTTON, WS_CHILD | WS_VISIBLE | CBRS_TOP) ||
		!m_wndToolBarTest.LoadToolBar(IDR_TOOLBAR_RIGHTHANDSIDE))	//your toolbar1 resource id
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	// Create tool bar
	CMyToolBar& tb3 = m_wndToolBarTest;
	VERIFY(tb3.LoadToolBar(IDR_TOOLBAR_RIGHTHANDSIDE)); 	//your toolbar1 resource id
	// use transparent so coolbar bitmap will show through
	tb3.ModifyStyle(0, TBSTYLE_TRANSPARENT);
	// show button text on right
	tb3.ModifyStyle(0, TBSTYLE_LIST);
	CToolBarCtrl& tbc3 = tb3.GetToolBarCtrl();
    DWORD dwStyleEx3 = tbc3.GetExtendedStyle();
    tbc3.SetExtendedStyle(dwStyleEx3 | TBSTYLE_EX_MIXEDBUTTONS);
    tbc3.SetMaxTextRows(1);
	//Set button image and button count
	if(al_GetSettingString(_T("toolbar_right"),_T("image"),const_cast<TCHAR *>(m_sProfile.c_str()),szSkin)){
		Tstring sSkin = Tstring(theApp.m_szSkin) + Tstring(szSkin);
		//	m_wndToolBar1.SetButtonImage(sSkin.c_str(), 7);
		m_wndToolBarTest.SetButtonImage(sSkin.c_str(), 3);		//right有3个按钮
	}	
	//set tooltip
	//load your button tooltip from language file
	TCHAR szTipSec2[3][32] = {_T("ID_BN_EXIT"),_T("ID_BN_ABOUT"),_T("ID_BN_HELP")};	//3个按钮的名字
	for( i = 0 ,j = 0; i < 3 ; i ++,j++){
		TCHAR szTxt[MAX_PATH];
		if(m_wndToolBarTest.GetItemID(j) == 0)
			j ++;
		if(al_GetSettingString(_T("public"),szTipSec2[i],theApp.m_szRes,szTxt))
			m_wndToolBarTest.SetToolTipText(szTxt,j);
	}
	//Set the background image
	//load your toolbar background
	if(al_GetSettingString(_T("toolbar_right"),_T("bkimage"),const_cast<TCHAR *>(m_sProfile.c_str()),szSkin))
	{
		Tstring sSkin(theApp.m_szSkin);
		sSkin += Tstring(szSkin);
		m_wndToolBarTest.SetImgBk(sSkin.c_str());
	}

	//m_BmpToolBar相当于一个toolbar图片的temp
	CImage img;
	Tstring sSkin(theApp.m_szSkin);
	sSkin += Tstring(szSkin);
	img.LoadFile(sSkin.c_str());
	HBITMAP	hbm;
	img.GetHBITMAP( Color::White, &hbm );
	m_BmpToolBar.Attach( hbm );

//	m_wndRebar.AddBar(&m_wndToolBarRight,NULL,&m_BmpToolBar,RBBS_CHILDEDGE);
	m_wndRebar.AddBar(&m_wndToolBarLeft,NULL,&m_BmpToolBar,RBBS_FIXEDBMP);//RBBS_FIXEDBMP
	m_wndRebar.AddBar(&m_wndToolBarRight,NULL,&m_BmpToolBar,RBBS_FIXEDSIZE);
	//for test
	//m_wndRebar.AddBar(&m_wndToolBarTest,NULL,&m_BmpToolBar,RBBS_BREAK);


	REBARBANDINFO rbbi;
	rbbi.cbSize = sizeof(REBARBANDINFO);
	rbbi.fMask = RBBIM_STYLE;
	m_wndRebar.GetReBarCtrl().GetBandInfo(1, &rbbi);
	rbbi.fStyle |= RBBS_NOGRIPPER;
	//设置rebar band1的属性  也就是工具条2 right的属性
	m_wndRebar.GetReBarCtrl().SetBandInfo(1, &rbbi);
/**/
	

////////////////////////////
	COLORREF color;

	//Create the status bar
	//初始化状态栏显示文字为空
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create     SBPS_POPOUT
	}
	m_wndStatusBar.SetPaneInfo(0,0,SBPS_NOBORDERS,800);		
	m_wndStatusBar.SetPaneText( 0, _T(""), TRUE );

	//set window pos
	CRect rect;
	if(al_GetSettingRect(_T("panel"),_T("rect"),const_cast<TCHAR *>(m_sProfile.c_str()),rect))
	{
	//	int iCaption = ::GetSystemMetrics(SM_CYCAPTION);
	//	int iBorder = ::GetSystemMetrics(SM_CXBORDER);
	//	rect.right += 2 * iBorder;
	//	rect.bottom += iCaption + iBorder;
		MoveWindow(rect);
		CenterWindow();
	}
	//Maximize Window or Normal size
	int	nMaxView = 1;
	al_GetSettingInt(_T("panel"),_T("maxview"),const_cast<TCHAR *>(m_sProfile.c_str()),nMaxView);
	if(nMaxView==1)
	{
		ShowWindow(SW_MAXIMIZE);
	}

	//Set window title
	TCHAR szTmp[MAX_PATH];
	if(al_GetSettingString(_T("public"),_T("IDS_TITLE"),theApp.m_szRes,szTmp))
		SetWindowText(szTmp);
	
	//set spliter window background color
	if(al_GetSettingColor(_T("splitter"),_T("color"),const_cast<TCHAR *>(m_sProfile.c_str()),color))
		m_wndSplitter.SetBackground(color);

	//load data from mobile
	ShowWindow(SW_SHOW);
//	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_OPENDRIVER,0,0);

	//set the status bar text
	/*向leftview发消息，检测手机连接状态 
	::PostMessage(m_wndSplitter.GetPane(0,0)->GetSafeHwnd(),WM_LOADMOBILE,m_bOpenSync == true ? 1 : 0,0);
	*/
	OnSetLinkMobile((m_iStatus & ST_MOBILE_LINK) ? 1 : 0,0);
	
//	::PostMessage(m_wndSplitter.GetPane(0,0)->GetSafeHwnd(),WM_LOADMOBILE, 1 ,0);

	return 0;
}

static LPCTSTR RegisterSimilarClass(LPCTSTR lpszNewClassName,
       LPCTSTR lpszOldClassName, UINT nIDResource)
{
       // Get class info for old class.
       //
       HINSTANCE hInst = AfxGetInstanceHandle();
       WNDCLASS wc;
       if (!::GetClassInfo(hInst, lpszOldClassName, &wc)) {
              TRACE(_T("Can't find window class %s\n"), lpszOldClassName);
              return NULL;
       }

       // Register new class with same info, but different name and icon.
       //
       wc.lpszClassName = lpszNewClassName;
       wc.hIcon = ::LoadIcon(hInst, MAKEINTRESOURCE(nIDResource));
       if (!AfxRegisterClass(&wc)) {
              TRACE(_T("Unable to register window class%s\n"), lpszNewClassName);
              return NULL;
       }
       return lpszNewClassName;
}

LPCTSTR CMainFrame::s_winClassName = NULL;

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
//	cs.lpszClass = AfxRegisterWndClass(0);
	TCHAR szText[MAX_PATH];
	if(al_GetSettingString(_T("public"),_T("IDS_TITLE"),theApp.m_szRes,szText))
		cs.lpszName = szText;
  
	if(s_winClassName==NULL) 
	{      
		LPCTSTR lpClass = theApp.m_szClassName;
        s_winClassName = RegisterSimilarClass( lpClass, cs.lpszClass, IDR_MAINFRAME);
        if(!s_winClassName)
			return FALSE;
	}
    cs.lpszClass = s_winClassName;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	// forward focus to the view window
//	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
/*	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;
*/
	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	//Create splitter window by two views
/*	if(!m_wndSplitterTop.CreateStatic(this, 2, 1))    //| ~WS_BORDER
		return -1;
	if (!m_wndSplitter.CreateStatic(&m_wndSplitterTop, 1, 2, WS_CHILD | WS_VISIBLE , m_wndSplitterTop.IdFromRowCol(1, 0)))
		return -1;
	if(!m_wndSplitterTop.CreateView(0, 0, RUNTIME_CLASS(CTopView), CSize(100, 100),pContext))
	{
		m_wndSplitterTop.DestroyWindow();
		return false;
	}*/
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
		return -1;
	
	if (!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CPrevView), CSize(100, 100),pContext) ||
		!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CLeftView), CSize(100, 100),pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}
	
	m_wndSplitter.SetColumnInfo(0, 200, 100);
	//m_wndSplitter.SplitColumn(20);
	
	return CFrameWnd::OnCreateClient(lpcs, pContext);
}

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CFrameWnd::OnSize(nType, cx, cy);
	
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	// TODO: Add your message handler code here and/or call default
	lpMMI->ptMinTrackSize.x = FRM_WIDTH;
	lpMMI->ptMinTrackSize.y = FRM_HEIGHT;

	
	CFrameWnd::OnGetMinMaxInfo(lpMMI);
}

void CMainFrame::OnNew()
{
	UpdateStatusConnect();
	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_NEWEDIT,0,0);

	//Test Calendar
//	OnGetMobileCal();
}

void CMainFrame::OnEdit()
{
	UpdateStatusConnect();
	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_NEWEDIT,1,0);
}

void CMainFrame::OnDelete()
{
	UpdateStatusConnect();
	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_DELSEL,0,0);
}

void CMainFrame::OnSearch()
{
	UpdateStatusConnect();
	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_SEARCH,0,0);
}

void CMainFrame::OnBackup()
{
	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_BACKUP,0,0);
}

void CMainFrame::OnRestore()
{
#ifdef _UNICODE
	std::wstring file = _T("Comma Separated Values(*.CSV)");
	//file ext name
	std::wstring ext = _T("*.csv");
	//file format
	std::wstring szFormat = file + TCHAR(0) + ext + TCHAR(0); 
#else
	std::string file = "Comma Separated Values(*.CSV)";
	//file ext name
	std::string ext = "*.csv";
	//file format
	std::string szFormat = file + char(0) + ext + char(0); 
#endif


	TCHAR szMyDoc[MAX_PATH];
	SHGetSpecialFolderPath(NULL,szMyDoc,CSIDL_PERSONAL,FALSE);
	//new a CPreviewFileDlg
	CPreviewFileDlg* pDlg = new CPreviewFileDlg( TRUE, ext.c_str(), NULL, szMyDoc,
						 OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT, 
						 szFormat.c_str(), this, ::AfxGetInstanceHandle() );

	//set filter index
	pDlg->SetFilterIndex(0);

	int nRet = pDlg->DoModal();

	//if not OK ,do nothing
	if(nRet != IDOK)	{	
		SAFE_DELPTR(pDlg);
	}
	else{
		POSITION pos = pDlg->GetStartPosition();
		while (pos != NULL){
			//Get full path 
			TCHAR szPath[MAX_PATH];
			memset(szPath,0,sizeof(TCHAR) * MAX_PATH);
			pDlg->GetNextPathName(pos,szPath);
			
			//Check the file exist or not
			bool bExist = false;
			::SendMessage(m_wndSplitter.GetPane(0,0)->GetSafeHwnd(),WM_CHECKFILE,reinterpret_cast<WPARAM>(szPath),reinterpret_cast<LPARAM>(&bExist));

			if(!bExist){
				//To left tree view
				::SendMessage(m_wndSplitter.GetPane(0,0)->GetSafeHwnd(),WM_TOOLBAR_RESTORE,reinterpret_cast<WPARAM>(szPath),0);
				//To right list view
				::SendMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_RESTORE,reinterpret_cast<WPARAM>(szPath),0);
			}
		}
		SAFE_DELPTR(pDlg);
	}
}

void CMainFrame::OnRefresh()
{
//	_bReConnecttoMobile_ = true;//nono, 2004_1030
    //m_bMobileConnected = false;
	if(theApp.m_bLoadMSOutlook)
	{
		CTreeView *pView = static_cast<CTreeView*>(m_wndSplitter.GetPane(0,0));
		if(pView){
			HTREEITEM hItem = pView->GetTreeCtrl().GetSelectedItem();
			if(hItem){
				FileData *pData = reinterpret_cast<FileData*>(pView->GetTreeCtrl().GetItemData(hItem));
				//send message to preview window with selected file data
				if(pData)
				{
					if(pData->sMode == MOBILE_PHONE || pData->sMode == MEMORY )
					{
						UpdateStatusConnect();
						::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_RELOAD,0,0);
					}
					else if(pData->sMode == MSOUTLOOK)
					{
						if(pData->iIndex <0)
							::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_RELOAD_MSOT,0,reinterpret_cast<LPARAM>(pData));
						else
							::PostMessage(pView->GetSafeHwnd(),WM_TOOLBAR_RELOAD_MSOT,0,reinterpret_cast<LPARAM>(pData));
					}
				}
			}
		}
	}
	else
	{
		UpdateStatusConnect();
		::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_RELOAD,0,0);
	}
}
void CMainFrame::OnCheckDelete(WPARAM wParam,LPARAM lParam)
{
	BOOL* pbCanDelete = (BOOL*)lParam;
	CTreeView *pView = static_cast<CTreeView*>(m_wndSplitter.GetPane(0,0));
	if(pView)
	{
		HTREEITEM hItem = pView->GetTreeCtrl().GetSelectedItem();
		if(hItem)
		{
			FileData *pLeftMode = reinterpret_cast<FileData*>(pView->GetTreeCtrl().GetItemData(hItem));
			if(pLeftMode)
			{
				if((pLeftMode->sMode == MEMORY) || (pLeftMode->sMode == SIM_CARD) || (pLeftMode->sMode == MOBILE_PHONE) )
					*pbCanDelete  = true;
				else *pbCanDelete  = false;
			}
		}

	}

}
void CMainFrame::OnCheckTreeItem(WPARAM wParam,LPARAM lParam)
{
	bool bHaveMobileItem = (bool)wParam;
	m_bMobileGetFail = !bHaveMobileItem;
	::PostMessage(m_wndSplitter.GetPane(0,0)->GetSafeHwnd(),WM_CHECK_LEFTTREEITEM,wParam,lParam);
}

void CMainFrame::OnRefreshLeftView(WPARAM wParam,LPARAM lParam)
{
	::PostMessage(m_wndSplitter.GetPane(0,0)->GetSafeHwnd(),WM_REFRESH_LEFT_PANEL,1,0);
}

void CMainFrame::OnReload()
{
	if(theApp.m_iMobileCompany == CO_MOTO && (_tcscmp(theApp.m_szPhone,_T("E2")) == 0 ))
	{
		TCHAR szMsg[MAX_PATH*10];
		GetPrivateProfileString(_T("message"),_T("IDS_WARMING_MOTOE2"),_T(""),szMsg,MAX_PATH*10,theApp.m_szRes);
		AfxMessageBox(szMsg);

	}
	OnRefresh();
}

void CMainFrame::OnExit()
{
	::PostMessage(GetSafeHwnd(),WM_CLOSE,0,0);
}
void CMainFrame::OnHelp()
{
	CString str, str1;
	//make fullpath of help.ini
	CString	sHelpProfile = CString(theApp.m_szModule) + CString(_T("Help.ini"));
	//read lang key from lang.ini
	if(!al_GetSettingString(_T("Language"), _T("AP_Lang"), theApp.m_szLang, str.GetBuffer(MAX_PATH) ) )
		::AfxMessageBox( _T("Error lang.ini!"));
	//read help path from help.ini
	//MainPanel, Phonebook, Calendar, SMS, MMS, PhotoManager, RingtoneManager, Sync
	if(!al_GetSettingString( (LPTSTR)(LPCTSTR)str, _T("Calendar"), (LPTSTR)(LPCTSTR)sHelpProfile, str1.GetBuffer(MAX_PATH) ) )
		::AfxMessageBox( _T("Error Help.ini!"));

	//make fullpath of help file, and full command line
	str.Format( _T("hh.exe \"%s%s\""), theApp.m_szModule, str1 );
//	::WinExec( str, SW_MAXIMIZE );
#ifdef _UNICODE
	USES_CONVERSION;
	WCHAR *wstr = str.GetBuffer(MAX_PATH);
	str.ReleaseBuffer();
	char* sz = W2A(wstr);

	::WinExec( sz, SW_MAXIMIZE );	
#else
	::WinExec( str, SW_MAXIMIZE );	
#endif
}

void CMainFrame::OnPhonebook()
{
	xSetFuncBarCheck(0);
}

void CMainFrame::OnSchedule()
{
	xSetFuncBarCheck(1);
}

void CMainFrame::OnMemo()
{
	xSetFuncBarCheck(2);
}

void CMainFrame::xSetFuncBarCheck(int iState)
{
/*	CToolBarCtrl &tbc = m_wndFuncBar.GetToolBarCtrl();
	for(int i = 0 ; i < FUNC_BN_NUMBER ; i ++)
		tbc.SetState(m_nFuncID[i],TBSTATE_ENABLED);
	tbc.SetState(m_nFuncID[iState],TBSTATE_CHECKED);*/
}

HRESULT CMainFrame::OnCardPanelGetGroup(WPARAM wParam,LPARAM lParam)
{
	::SendMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_CARDPANEL_GETGROUP,wParam,lParam);
	return 1L;
}

HRESULT CMainFrame::OnCardPanelGroupCount(WPARAM wParam,LPARAM lParam)
{
	::SendMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_CARDPANEL_GROUPCOUNT,wParam,lParam);
	return 1L;
}

void CMainFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	// Do not call CFrameWnd::OnPaint() for painting messages
}
HRESULT CMainFrame::OnLeftUpdateData(WPARAM wParam,LPARAM lParam)
{
	CTreeView *pView = static_cast<CTreeView*>(m_wndSplitter.GetPane(0,0));
	if(pView){
		HTREEITEM hItem = pView->GetTreeCtrl().GetSelectedItem();
		if(hItem){
			FileData *pData = reinterpret_cast<FileData*>(pView->GetTreeCtrl().GetItemData(hItem));
			//send message to preview window with selected file data
			if(pData->sMode == MEMORY || pData->sMode ==MOBILE_PHONE)
				::SendMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_LEFT_UPDATEDATA,reinterpret_cast<WPARAM>(pData),lParam);
		}
	}
	return 1L;
}

HRESULT CMainFrame::OnLeftChangeMode(WPARAM wParam,LPARAM lParam)
{
//	TCHAR *pTxt = reinterpret_cast<TCHAR*>(wParam);
	//Get the selected tree item data 
	CTreeView *pView = static_cast<CTreeView*>(m_wndSplitter.GetPane(0,0));
	if(pView){
		HTREEITEM hItem = pView->GetTreeCtrl().GetSelectedItem();
		if(hItem){
			FileData *pData = reinterpret_cast<FileData*>(pView->GetTreeCtrl().GetItemData(hItem));
			//send message to preview window with selected file data
			if(pData)
				::SendMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_LEFT_CHGMODE,reinterpret_cast<WPARAM>(pData),lParam);
		}
	}
	return 1L;
}

HRESULT CMainFrame::OnGetLeftMode(WPARAM wParam,LPARAM lParam)
{
	CTreeView *pView = static_cast<CTreeView*>(m_wndSplitter.GetPane(0,0));
	FileData *pData = reinterpret_cast<FileData*>(wParam);
	FileData *pItem = reinterpret_cast<FileData*>(pView->GetTreeCtrl().GetItemData(pView->GetTreeCtrl().GetSelectedItem()));
	pData->iIndex = pItem->iIndex;
	pData->sFile = pItem->sFile;
	pData->sMode = pItem->sMode;
	return 1L;
}

HRESULT CMainFrame::OnErrOpenSync(WPARAM wParam,LPARAM lParam)
{
	if(wParam == 0)
		m_bOpenSync = false;
	return 1L;
}

HRESULT CMainFrame::OnGetSyncflag(WPARAM wParam,LPARAM lParam)
{
	bool *pFlag = reinterpret_cast<bool *>(wParam);
	*pFlag = m_bOpenSync;
	return 1L;
}

void CMainFrame::OnDestroy() 
{
	theApp.m_bExit = true;
	CFrameWnd::OnDestroy();
	m_sProfile = _T("");
	// TODO: Add your message handler code here
}

HRESULT CMainFrame::OnLeftDropData(WPARAM wParam,LPARAM lParam)
{
	::SendMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_LEFT_DROPDATA,wParam,lParam);
	return 1L;
}

void CMainFrame::OnUpdateBnBackup(CCmdUI* pCmdUI) 
{
	//there are item(s)	in the list
	CPrevView *pWnd = reinterpret_cast<CPrevView*>(m_wndSplitter.GetPane(0,1));
	FileData *pLeftMode = pWnd->GetLeftMode();
	if( ( ((pLeftMode->sMode == MEMORY) || (pLeftMode->sMode == SIM_CARD)) || (pLeftMode->sMode == MOBILE_PHONE) ) && (pWnd->IsItemInList()))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateBnDelete(CCmdUI* pCmdUI) 
{
	//if there are no selected items ,set false.
	//else set true
	CPrevView *pWnd = reinterpret_cast<CPrevView*>(m_wndSplitter.GetPane(0,1));
	FileData *pLeftMode = pWnd->GetLeftMode();
	if(pLeftMode->sMode == MSOUTLOOK)
		pCmdUI->Enable(FALSE);
	else
	{
		if( (m_iStatus & ST_MOBILE_LINK) /*&& ((pLeftMode->sMode == MEMORY) ||(pLeftMode->sMode == SIM_CARD)||(pLeftMode->sMode == MOBILE_PHONE)) && (pWnd->GetListItemSelectedCount() == 1)*/) {
		//if( (m_iStatus & ST_MOBILE_LINK) && ((pLeftMode->sMode == MEMORY) ||(pLeftMode->sMode == SIM_CARD)) && (pWnd->GetListItemSelectedCount() > 0)) {
			if (DATEPERIOD_LIST == pWnd->GetCalendarDayView()->GetDatePeriod()) {
				if (pWnd->GetCalendarDayView() && pWnd->GetCalendarDayView()->GetScheduleList() && pWnd->GetCalendarDayView()->GetScheduleList()->GetListCtrl())
	    			pCmdUI->Enable(pWnd->GetCalendarDayView()->GetScheduleList()->GetListCtrl().GetSelectedCount());
			}else {
				if (pWnd->GetCalendarDayView() && pWnd->GetCalendarDayView()->GetTable())
	        		pCmdUI->Enable(pWnd->GetCalendarDayView()->GetTable()->CheckIsDataSelected());
			}
		}
		else
			pCmdUI->Enable(FALSE);
	}
	
}

void CMainFrame::OnUpdateBnEdit(CCmdUI* pCmdUI) 
{
	//if there are no selected items ,set false.
	//else set true

	CPrevView *pWnd = reinterpret_cast<CPrevView*>(m_wndSplitter.GetPane(0,1));
	FileData *pLeftMode = pWnd->GetLeftMode();

	if( (m_iStatus & ST_MOBILE_LINK)/* && ((pLeftMode->sMode == MEMORY) ||(pLeftMode->sMode == SIM_CARD)||(pLeftMode->sMode == MOBILE_PHONE)) && (pWnd->GetListItemSelectedCount() == 1)*/) {
		if (DATEPERIOD_LIST == pWnd->GetCalendarDayView()->GetDatePeriod()) {
			if (pWnd->GetCalendarDayView() && pWnd->GetCalendarDayView()->GetScheduleList() && pWnd->GetCalendarDayView()->GetScheduleList()->GetListCtrl())
	    		pCmdUI->Enable(pWnd->GetCalendarDayView()->GetScheduleList()->GetListCtrl().GetSelectedCount());
		}else {
			if (pWnd->GetCalendarDayView() && pWnd->GetCalendarDayView()->GetTable())
	         	pCmdUI->Enable(pWnd->GetCalendarDayView()->GetTable()->CheckIsDataSelected());
		}
	}
	else
		pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateBnNew(CCmdUI* pCmdUI) 
{
	CPrevView *pWnd = reinterpret_cast<CPrevView*>(m_wndSplitter.GetPane(0,1));
	FileData *pLeftMode = pWnd->GetLeftMode();
	if(pLeftMode->sMode == MSOUTLOOK)
		pCmdUI->Enable(FALSE);
	else
	{

		if( ((m_iStatus & ST_MOBILE_LINK) && m_bMobileGetFail == false)/* && ((pLeftMode->sMode == MEMORY) ||(pLeftMode->sMode == SIM_CARD) ||(pLeftMode->sMode == MOBILE_PHONE) )*/)	
			pCmdUI->Enable(TRUE);
		else
			pCmdUI->Enable(FALSE);
	}
}


void CMainFrame::OnUpdateBnHelp(CCmdUI* pCmdUI) 
{
	CPrevView *pWnd = reinterpret_cast<CPrevView*>(m_wndSplitter.GetPane(0,1));
	FileData *pLeftMode = pWnd->GetLeftMode();

//	if( (m_iStatus & ST_MOBILE_LINK) && ((pLeftMode->sMode == MEMORY) ||(pLeftMode->sMode == SIM_CARD) ||(pLeftMode->sMode == MOBILE_PHONE) ))	
		pCmdUI->Enable(TRUE);
//	else
//		pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateBnAbout(CCmdUI* pCmdUI) 
{
	CPrevView *pWnd = reinterpret_cast<CPrevView*>(m_wndSplitter.GetPane(0,1));
	FileData *pLeftMode = pWnd->GetLeftMode();

//	if( (m_iStatus & ST_MOBILE_LINK) && ((pLeftMode->sMode == MEMORY) ||(pLeftMode->sMode == SIM_CARD) ||(pLeftMode->sMode == MOBILE_PHONE) ))	
		pCmdUI->Enable(TRUE);
//	else
//		pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateBnRefresh(CCmdUI* pCmdUI) 
{
	//if mobile is connected ,set TRUE.
	//else set FALSE.
	if(m_iStatus & ST_MOBILE_LINK)	
		pCmdUI->Enable(TRUE);
	else 
		pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateBnRestore(CCmdUI* pCmdUI) 
{
	//always enable
	pCmdUI->Enable(TRUE);
}

void CMainFrame::OnUpdateBnSearch(CCmdUI* pCmdUI) 
{
	//there are item(s)	in the list
	CPrevView *pWnd = reinterpret_cast<CPrevView*>(m_wndSplitter.GetPane(0,1));
	if(pWnd->IsItemInList())
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);	
}

void CMainFrame::OnUpdateStatusConnect(CCmdUI* pCmdUI)
{
//	static bool bFirst = true;
	//Set status bar text
	TCHAR szText[MAX_PATH];
	
/*	if( bFirst && !m_bMobileConnected )	{
		_tcscpy(szText,"");
	}
	else	{
		bFirst = false;
		if (m_bMobileConnected) {
			TCHAR szTmp[MAX_PATH];
			al_GetSettingString(_T("public"),"IDS_CONNECTING",theApp.m_szRes,szTmp);
			sprintf(szText,szTmp,theApp.m_szMobileName);
		}
		else{
			TCHAR szTmp[MAX_PATH];
			al_GetSettingString(_T("public"),"IDS_DISCONNECTED",theApp.m_szRes,szTmp);
			sprintf(szText,szTmp,theApp.m_szMobileName);
		}
	}
*/
	if(theApp.m_bOpenDriverFinal)
	{
		if (m_bMobileConnected == TRUE) {
			TCHAR szTmp[MAX_PATH];
			al_GetSettingString(_T("public"),_T("IDS_CONNECTING"),theApp.m_szRes,szTmp);
			wsprintf(szText,szTmp,theApp.m_szMobileName);
		}
		else{
			TCHAR szTmp[MAX_PATH];
			al_GetSettingString(_T("public"),_T("IDS_DISCONNECTED"),theApp.m_szRes,szTmp);
			wsprintf(szText,szTmp,theApp.m_szMobileName);
		}
	}
	else
	{
		_tcscpy(szText,_T(""));
	}
	pCmdUI->SetText(szText);
	m_wndStatusBar.SetPaneText( 0, szText, TRUE );

}

void CMainFrame::OnUpdateBnExit(CCmdUI* pCmdUI) 
{
	if(m_bSearch)
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);

}


HRESULT CMainFrame::OnSetLoadFileFlag(WPARAM wParam,LPARAM lParam)
{
	if(wParam == 1)
		m_iStatus |= ST_FILE_LOAD;
	else
		m_iStatus &= ~ST_FILE_LOAD;
	return 1L;
}

HRESULT CMainFrame::OnSetLinkMobile(WPARAM wParam,LPARAM lParam)
{
	if(wParam == 1){
		m_iStatus |= ST_MOBILE_LINK;
		m_bOpenSync = true;
		//向leftview发送WM_LOADMOBILE消息 响应函数为onloadmobile()
		::PostMessage(m_wndSplitter.GetPane(0,0)->GetSafeHwnd(),WM_LOADMOBILE,m_bOpenSync == true ? 1 : 0,0);
		//::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_LOADMOBILE,0,0);
	}
	else
	{
		m_iStatus &= ~ST_MOBILE_LINK;
		m_bOpenSync = false;
	}
	return 1L;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
	return CFrameWnd::PreTranslateMessage(pMsg);
}

//The function set to driver.It can recive the connected status.
int CMainFrame::ConnectStatusCallback(int iState)
{
	//if (!_bReConnecttoMobile_) return 0;//nono, 2004_1030

	//Set status flag 
	if(iState == Anw_MOBILE_CONNECT_FAILED){
		m_iStatus &= ~ST_MOBILE_LINK;
		//display a messagebox
		TCHAR szText[MAX_PATH],szTmp[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_DISCONNECTED"),theApp.m_szRes,szTmp);
		wsprintf(szText,szTmp,theApp.m_szMobileName);
		AfxMessageBox(szText);
	}
	//_bReConnecttoMobile_ = false;//nono, 2004_1030

	return 0;
}

HRESULT CMainFrame::OnSetSearchFlag(WPARAM wParam,LPARAM lParam)
{
	m_bSearch = wParam == 0 ? false : true;
	return 1L;
}

void CMainFrame::OnLoadCalendar(WPARAM wParam,LPARAM lParam)
{
	bool _bMobileConnected = wParam ? true : false;
	if ((m_bMobileConnected == FALSE || m_bMobileConnected ==-1)&& _bMobileConnected) {
     	m_bMobileConnected = _bMobileConnected;
     	UpdateStatusConnect();
		//Send message to preview panel. [void CPrevView::OnLoadCalendar(WPARAM wParam,LPARAM lParam)]
    	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_LOADCALENDAR,1,0);
 
		//set the statusbar text]
		if(!static_bMobileConnected) {
        	//OnSetLinkMobile((m_iStatus & ST_MOBILE_LINK) ? 1 : 1,0);
     		static_bMobileConnected=true;
		}
	}else if ((m_bMobileConnected == TRUE || m_bMobileConnected ==-1) && !_bMobileConnected) {
		m_bMobileConnected = _bMobileConnected;
     	UpdateStatusConnect();
    	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_LOADCALENDAR,0,0);
		static_nCountFailureConecttoMobile++;
	//	if (3==static_nCountFailureConecttoMobile) {
	     	TCHAR szTmp[MAX_PATH];
			CString strMsg;
	    	al_GetSettingString(_T("public"),_T("IDS_ERR_OPEN"),theApp.m_szRes,szTmp);		
		//	AfxMessageBox(szTmp);
			strMsg.Format(szTmp,theApp.m_szMobileName);

		//	CalendarStringMessageBox(m_hWnd,strMsg);
			static_nCountFailureConecttoMobile=0;
	//	}
	}

}

void CMainFrame::UpdateStatusConnect()
{
	//Set status bar text
	TCHAR szText[MAX_PATH];
	if (m_bMobileConnected == TRUE) {
		TCHAR szTmp[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_CONNECTING"),theApp.m_szRes,szTmp);
		wsprintf(szText,szTmp,theApp.m_szMobileName);
		m_iStatus = true;
	}
	else{
		TCHAR szTmp[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_DISCONNECTED"),theApp.m_szRes,szTmp);
		wsprintf(szText,szTmp,theApp.m_szMobileName);
		m_iStatus = false;
	}

	m_wndStatusBar.SetPaneText(0 ,szText, TRUE);

}

void CMainFrame::OnOpenCalendar(WPARAM wParam,LPARAM lParam)
{
    ::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_OPENCALENDAR,0,0);
}
BOOL CMainFrame::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return FALSE;
	return CFrameWnd::OnEraseBkgnd(pDC);
}

void CMainFrame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	// DLL CONTRUCT
	if(afxContextIsDLL)
	{
	  if(GetActiveDocument())
	  {
	    DocViewClose(((CCalendarApp *)afxGetApp())->AppHwnd);
	  }
      else
	  {
 	    //::SendMessage(m_wndSplitter,WM_CLOSE,0,0);
 	    CFrameWnd::OnClose();
	  }

	  // Ken 7/28 : Use message loop for DLL
	  // What will happen if the DLL cause unexpected quit???
      //::PostMessage(((CCalendarApp *)afxGetApp())->AppHwnd,ID_EXIT_DLL_MESSAGE,0,0);
	}
	else
	{
	  CFrameWnd::OnClose();
	}
}

//Ken 7/28  DLL CONTRUCT
void CMainFrame::UpdateMyDLLCmdUI()
{
	m_wndStatusBar.OnIdleUpdateCmdUI(TRUE, 0L);
	m_wndToolBarLeft.OnIdleUpdateCmdUI(TRUE, 0L);
	m_wndToolBarRight.OnIdleUpdateCmdUI(TRUE, 0L);
}

// DLL CONTRUCT
void CMainFrame::DocViewClose(HWND AppHwnd)
{
	if (m_lpfnCloseProc != NULL && !(*m_lpfnCloseProc)(this))
		return;

	// Note: only queries the active document
	CDocument* pDocument = GetActiveDocument();
	if (pDocument != NULL && !pDocument->CanCloseFrame(this))
	{
		// document can't close right now -- don't close it
		return;
	}
	
	CWinApp *pApp = (CWinApp *)afxGetApp();

	if (pApp != NULL && pApp->m_pMainWnd == this)
	{
		// attempt to save all documents
		if (pDocument == NULL && !pApp->SaveAllModified())
			return;     // don't close it

		// hide the application's windows before closing all the documents
		pApp->HideApplication();

		// close all documents first
		pApp->CloseAllDocuments(FALSE);
		//pDocument->m_bAutoDelete = false;

		// don't exit if there are outstanding component objects
		if (!AfxOleCanExitApp())
		{
			// take user out of control of the app
			AfxOleSetUserCtrl(FALSE);

			// don't destroy the main window and close down just yet
			//  (there are outstanding component (OLE) objects)
			return;
		}
        /*
		// there are cases where destroying the documents may destroy the
		//  main window of the application.
		if (!afxContextIsDLL && pApp->m_pMainWnd == NULL)
		{
			AfxPostQuitMessage(0);
			return;
		}
		*/
	}
	delete pApp->m_pDocManager;
	pApp->m_pDocManager = NULL;
	// What will happen if the DLL cause unexpected quit???
    //::SendMessage(AppHwnd,ID_EXIT_DLL_MESSAGE,0,0);

	m_pCtrlSite = NULL;
	CFrameWnd::DestroyWindow();

}
HRESULT CMainFrame::OnLeftDropData2MSOT(WPARAM wParam,LPARAM lParam)
{
	::SendMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_LEFT_DROPDATATOMSOT,wParam,lParam);
	return 1L;
}

CString CMainFrame::GetTreeSelectMode()
{
	CString strSel ;
	strSel.Empty();
	CTreeView *pView = static_cast<CTreeView*>(m_wndSplitter.GetPane(0,0));
	if(pView)
	{
		HTREEITEM hItem = pView->GetTreeCtrl().GetSelectedItem();
		if(hItem)
		{
			FileData *pData = reinterpret_cast<FileData*>(pView->GetTreeCtrl().GetItemData(hItem));
			if(pData)
				strSel.Format(_T("%s"),pData->sMode.c_str());
		}
	}
	return strSel;

}
HRESULT CMainFrame::OnSetLeftItemCount(WPARAM wParam,LPARAM lParam)
{
	FileData *pData = reinterpret_cast<FileData*>(wParam);
	CLeftView *pView = static_cast<CLeftView*>(m_wndSplitter.GetPane(0,0));
	pView->SetItemCount(*pData,lParam);
	return 1L;
}
