// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "PhoneBook.h"
#include "CommUIExportFun.h"

#include "MainFrm.h"


#include "include\csvhandle.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define FRM_WIDTH 722
#define FRM_HEIGHT 542

//·í·Æ¹«­º¦¸¶i¤Jµøµ¡®É
// OnDragEnter is called by OLE dll's when drag cursor enters
// a window that is REGISTERed with the OLE dll's
//
DROPEFFECT CDropTgt::OnDragEnter(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{
    // if the control key is held down, return a drop effect COPY 
    if((dwKeyState & MK_CONTROL) == MK_CONTROL)
        return DROPEFFECT_NONE;
    // Otherwise return a Œ/..drop effect of MOVE
    else
        return DROPEFFECT_MOVE;    
}

//·í·Æ¹«¦bµøµ¡¤º,­«½Æ³Q½Õ¥Î
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

//·í·Æ¹«²¾¥Xµøµ¡®É
// OnDragLeave is called by OLE dll's when drag cursor leaves
// a window that is REGISTERed with the OLE dll's
//
void CDropTgt::OnDragLeave(CWnd *pWnd)
{
    // Call base class implementation
    COleDropTarget::OnDragLeave(pWnd);
}

//·í·Æ¹«¦bµøµ¡¤º¸¨¤U 
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
	ON_UPDATE_COMMAND_UI(ID_BN_EXIT, OnUpdateBnExit)
	ON_COMMAND(ID_BN_HELP, OnBnHelp)
	ON_COMMAND(ID_BN_ABOUT, OnBnAbout)
	ON_WM_ERASEBKGND()
	ON_WM_CLOSE()
	ON_WM_ACTIVATE()
	ON_UPDATE_COMMAND_UI(ID_BN_SENDSMS, OnUpdateBnSendsms)
	ON_COMMAND(ID_BN_SENDSMS, OnBnSendsms)
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
	ON_MESSAGE(WM_PROGDLGSHOW,OnProgDlgShow)
	ON_MESSAGE(WM_SETLEFTITEMCOUNT,OnSetLeftItemCount)
	ON_MESSAGE(WM_DOWNLOAD_FAIL,OnDownloadFail)
	ON_MESSAGE(WM_MOBILE_CONNECT, OnMobileConnectStatus)
	ON_MESSAGE(WM_CHECKLEFTTREEITEM,OnCheckLeftTreeItem)
	ON_MESSAGE(WM_LEFT_DROPDATATOMSOT,OnLeftDropData2MSOT)
END_MESSAGE_MAP()

static UINT indicators[] =
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
int CMainFrame::m_iStatus = 0;
//CMainFrame* CMainFrame::m_pActive = NULL;


CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	UINT nFuncID[FUNC_BN_NUMBER] = {ID_BN_PHONEBOOK,ID_BN_SCHEDULE,ID_BN_MEMO};
	for(int i = 0 ; i < FUNC_BN_NUMBER ; i ++)
		m_nFuncID[i] = nFuncID[i];
	m_bOpenSync = true;
	m_bSearch = false;
//	m_pActive = this;
	m_bFirst = true;
	// DLL CONTRUCT
	m_iStatus = 0;
//	m_iStatus &= ~ST_MOBILE_LINK;
	m_iStatus |= ST_WAIT_INIT;
	m_bCapture = FALSE;
	m_bSupportDirectSMS = true;
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
	m_sProfile = Tstring(theApp.m_szSkin) + Tstring(_T("Organize\\main.ini"));

	//Create rebar
	m_wndRebar.Create(this,RBS_FIXEDORDER);
	//Create toolbar1 and toolbar2
//	if(m_bSupportDirectSMS)
	{
		if (!m_wndToolBar1.CreateEx(&m_wndRebar, TBSTYLE_BUTTON, WS_CHILD | WS_VISIBLE | CBRS_TOP) ||
			!m_wndToolBar1.LoadToolBar(IDR_TB_TOOLBAR1_SMS))
		{
			TRACE0("Failed to create toolbar\n");
			return -1;      // fail to create
		}
	}
/*	else
	{
		if (!m_wndToolBar1.CreateEx(&m_wndRebar, TBSTYLE_BUTTON, WS_CHILD | WS_VISIBLE | CBRS_TOP) ||
			!m_wndToolBar1.LoadToolBar(IDR_TB_TOOLBAR1))
		{
			TRACE0("Failed to create toolbar\n");
			return -1;      // fail to create
		}
	}*/
	
	// Create tool bar
	CMyToolBar& tb = m_wndToolBar1;
	if(m_bSupportDirectSMS)
		VERIFY(tb.LoadToolBar(IDR_TB_TOOLBAR1_SMS)); 
	else
		VERIFY(tb.LoadToolBar(IDR_TB_TOOLBAR1)); 

	// use transparent so coolbar bitmap will show through
	tb.ModifyStyle(0, TBSTYLE_TRANSPARENT);

	// show button text on right
	tb.ModifyStyle(0, TBSTYLE_LIST);

	CToolBarCtrl& tbc = tb.GetToolBarCtrl();

    DWORD dwStyleEx1 = tbc.GetExtendedStyle();
    tbc.SetExtendedStyle(dwStyleEx1 | TBSTYLE_EX_MIXEDBUTTONS);
    tbc.SetMaxTextRows(1);
	
	//Set button image and button count
	TCHAR szSkin[MAX_PATH];
//if(m_bSupportDirectSMS)
{
	if(al_GetSettingString(_T("toolbar1_SMS"),_T("image"),const_cast<TCHAR *>(m_sProfile.c_str()),szSkin)){
		Tstring sSkin = Tstring(theApp.m_szSkin) + Tstring(szSkin);
		m_wndToolBar1.SetButtonImage(sSkin.c_str(), 8);
	}
	//set tooltip
	TCHAR szTipSec[8][32] = {_T("ID_BN_NEW"),_T("ID_BN_EDIT"),_T("ID_BN_DELETE"),_T("ID_BN_SEARCH"),
		_T("ID_BN_BACKUP"),_T("ID_BN_RESTORE"),_T("ID_BN_REFRESH"),_T("ID_BN_SENDSMS")};
	for(int i = 0 ,j = 0; i < 8 ; i ++,j++){
		TCHAR szTxt[MAX_PATH];
		if(m_wndToolBar1.GetItemID(j) == 0)
			j ++;
		if(al_GetSettingString(_T("public"),szTipSec[i],theApp.m_szRes,szTxt))
			m_wndToolBar1.SetToolTipText(szTxt,j);
	}
}
/*else
{
	if(al_GetSettingString("toolbar1","image",const_cast<TCHAR *>(m_sProfile.c_str()),szSkin)){
		string sSkin = string(theApp.m_szSkin) + string(szSkin);
		m_wndToolBar1.SetButtonImage(sSkin.c_str(), 7);
	}
	//set tooltip
	TCHAR szTipSec[7][32] = {"ID_BN_NEW","ID_BN_EDIT","ID_BN_DELETE","ID_BN_SEARCH",
		"ID_BN_BACKUP","ID_BN_RESTORE","ID_BN_REFRESH"};
	for(int i = 0 ,j = 0; i < 7 ; i ++,j++){
		TCHAR szTxt[MAX_PATH];
		if(m_wndToolBar1.GetItemID(j) == 0)
			j ++;
		if(al_GetSettingString(_T("public"),szTipSec[i],theApp.m_szRes,szTxt))
			m_wndToolBar1.SetToolTipText(szTxt,j);
	}
}*/
int i,j;

	//Set the background image
	if(al_GetSettingString(_T("toolbar1"),_T("image_bk"),const_cast<TCHAR *>(m_sProfile.c_str()),szSkin)){
		Tstring sSkin(theApp.m_szSkin);
		sSkin += Tstring(szSkin);
		m_wndToolBar1.SetImgBk(sSkin.c_str());
	}

	//Create toolbar1 and toolbar2
	if (!m_wndToolBar2.CreateEx(&m_wndRebar, TBSTYLE_BUTTON, WS_CHILD | WS_VISIBLE | CBRS_TOP) ||
		!m_wndToolBar2.LoadToolBar(IDR_TB_TOOLBAR2))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	
	// Create tool bar
	CMyToolBar& tb2 = m_wndToolBar2;
	VERIFY(tb2.LoadToolBar(IDR_TB_TOOLBAR2)); 

	// use transparent so coolbar bitmap will show through
	tb2.ModifyStyle(0, TBSTYLE_TRANSPARENT);

	// show button text on right
	tb2.ModifyStyle(0, TBSTYLE_LIST);

	CToolBarCtrl& tbc2 = tb2.GetToolBarCtrl();

    DWORD dwStyleEx2 = tbc2.GetExtendedStyle();
    tbc2.SetExtendedStyle(dwStyleEx2 | TBSTYLE_EX_MIXEDBUTTONS);
    tbc2.SetMaxTextRows(1);
	
	//Set button image and button count
	if(al_GetSettingString(_T("toolbar2"),_T("image"),const_cast<TCHAR *>(m_sProfile.c_str()),szSkin)){
		Tstring sSkin = Tstring(theApp.m_szSkin) + Tstring(szSkin);
		m_wndToolBar2.SetButtonImage(sSkin.c_str(), 3);
	}

	//set tooltip
	TCHAR szTipSec1[3][32] = {_T("ID_BN_EXIT"),_T("ID_BN_ABOUT"),_T("ID_BN_HELP")};
	for( i = 0 ,j = 0; i < 3 ; i ++,j++){
		TCHAR szTxt[MAX_PATH];
		if(m_wndToolBar2.GetItemID(j) == 0)
			j ++;
		if(al_GetSettingString(_T("public"),szTipSec1[i],theApp.m_szRes,szTxt))
			m_wndToolBar2.SetToolTipText(szTxt,j);
	}

	//Set the background image
	if(al_GetSettingString(_T("toolbar2"),_T("image_bk"),const_cast<TCHAR *>(m_sProfile.c_str()),szSkin)){
		Tstring sSkin(theApp.m_szSkin);
		sSkin += Tstring(szSkin);
		m_wndToolBar2.SetImgBk(sSkin.c_str());
	}

	CImage img;
	Tstring sSkin(theApp.m_szSkin);
	sSkin += Tstring(szSkin);
	img.LoadFile(sSkin.c_str());

	HBITMAP	hbm;
	img.GetHBITMAP( Color::White, &hbm );
	m_BmpToolBar.Attach( hbm );

	m_wndRebar.AddBar(&m_wndToolBar1,NULL,&m_BmpToolBar,RBBS_FIXEDBMP);
	m_wndRebar.AddBar(&m_wndToolBar2,NULL,&m_BmpToolBar,RBBS_FIXEDBMP);

	//set the second band fixed.
	REBARBANDINFO rbbi;
	rbbi.cbSize = sizeof(REBARBANDINFO);
	rbbi.fMask = RBBIM_STYLE;
	m_wndRebar.GetReBarCtrl().GetBandInfo(1, &rbbi);
	rbbi.fStyle |= RBBS_NOGRIPPER;

	m_wndRebar.GetReBarCtrl().SetBandInfo(1, &rbbi);

	//Create the status bar
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	//set window pos
	CRect rect;
	if(al_GetSettingRect(_T("panel"),_T("rect"),const_cast<TCHAR *>(m_sProfile.c_str()),rect)){
		int iCaption = ::GetSystemMetrics(SM_CYCAPTION);
		int iBorder = ::GetSystemMetrics(SM_CXBORDER);
		rect.right += 2 * iBorder;
		rect.bottom += iCaption + iBorder;
		MoveWindow(rect);
	}
	CenterWindow();

	//Set window title
	TCHAR szTmp[MAX_PATH];
	if(al_GetSettingString(_T("public"),_T("IDS_TITLE"),theApp.m_szRes,szTmp))
		SetWindowText(szTmp);
	
	//set spliter window background color
	COLORREF color;
	if(al_GetSettingColor(_T("splitter"),_T("color"),const_cast<TCHAR *>(m_sProfile.c_str()),color))
		m_wndSplitter.SetBackground(color);

	//Maximize Window or Normal size
	int	nMaxView;
	if(!al_GetSettingInt(_T("panel"),_T("maxview"),const_cast<TCHAR *>(m_sProfile.c_str()),nMaxView))
		nMaxView = 1;
	ShowWindow((nMaxView==1) ?SW_MAXIMIZE :SW_SHOW);
	SetForegroundWindow();

	//load data from mobile
	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_OPENDRIVER,0,0);
	//set the statusbar text
	OnSetLinkMobile((m_iStatus & ST_MOBILE_LINK) ? 1 : 0,0);
//	SetFocus();

      
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
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
		return -1;

	if (!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CPrevView), CSize(100, 100),pContext) ||
	   !m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CLeftView), CSize(100, 100),pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}
	m_pView = reinterpret_cast<CPrevView*>(m_wndSplitter.GetPane(0,1));
	m_pLeft = reinterpret_cast<CLeftView*>(m_wndSplitter.GetPane(0,0));
	m_wndSplitter.SetColumnInfo(0, 200, 100);	

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
	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_NEWEDIT,0,0);
}

void CMainFrame::OnEdit()
{
	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_NEWEDIT,1,0);
}

void CMainFrame::OnDelete()
{
	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_DELSEL,0,0);
}

void CMainFrame::OnSearch()
{
	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_SEARCH,0,0);
}

void CMainFrame::OnBackup()
{
	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_BACKUP,0,0);
}

void CMainFrame::OnRestore()
{
	//the filter
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
		RedrawWindow();
		POSITION pos = pDlg->GetStartPosition();
		while (pos != NULL){
			//Get full path 
			TCHAR szPath[MAX_PATH];
			memset(szPath,0,sizeof(TCHAR) * MAX_PATH);
			pDlg->GetNextPathName(pos,szPath);
			
			CCSVHandle csv(szPath);
			TCHAR szText[MAX_PATH];
			if(!csv.CheckFormat()){
				if(al_GetSettingString(_T("public"),_T("IDS_ERR_LOADCONTRACT"),theApp.m_szRes,szText)){
					TCHAR szFormat[MAX_PATH * 2];
					wsprintf(szFormat,szText,szPath);
					AfxMessageBox(szFormat);
					continue;
				}
			}
			//Check the file exist or not
			int iExist = -1;
			::SendMessage(m_wndSplitter.GetPane(0,0)->GetSafeHwnd(),WM_CHECKFILE,reinterpret_cast<WPARAM>(szPath),reinterpret_cast<LPARAM>(&iExist));

			//To left tree view
			::SendMessage(m_wndSplitter.GetPane(0,0)->GetSafeHwnd(),WM_TOOLBAR_RESTORE,reinterpret_cast<WPARAM>(szPath),iExist);
			//To right list view
			::SendMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_RESTORE,reinterpret_cast<WPARAM>(szPath),iExist);
		}
		SAFE_DELPTR(pDlg);
	}
}

void CMainFrame::OnRefresh()
{
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
					if(pData->sMode == MOBILE_PHONE || pData->sMode == MEMORY || pData->sMode== SIM_CARD)
						::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_RELOAD,0,0);
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
		::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_RELOAD,0,0);
}

void CMainFrame::OnExit()
{
	::PostMessage(GetSafeHwnd(),WM_CLOSE,0,0);
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
HRESULT CMainFrame::OnLeftDropData2MSOT(WPARAM wParam,LPARAM lParam)
{
	::SendMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_LEFT_DROPDATATOMSOT,wParam,lParam);
	return 1L;
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
	if( ( ((pLeftMode->sMode == MEMORY) || (pLeftMode->sMode == SIM_CARD)) || 
		(pLeftMode->sMode == MOBILE_PHONE) ) && (pWnd->IsItemInList()) && 
		(m_iStatus & ST_MOBILE_LINK))
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

	if( (m_iStatus & ST_MOBILE_LINK) && ((pLeftMode->sMode == MEMORY) ||
		(pLeftMode->sMode == SIM_CARD) || (pLeftMode->sMode == MOBILE_PHONE)) && 
		(pWnd->GetListItemSelectedCount() > 0))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
	
}

void CMainFrame::OnUpdateBnEdit(CCmdUI* pCmdUI) 
{
	//if there are no selected items ,set false.
	//else set true
	CPrevView *pWnd = reinterpret_cast<CPrevView*>(m_wndSplitter.GetPane(0,1));
	CTreeView *pView = static_cast<CTreeView*>(m_wndSplitter.GetPane(0,0));
	FileData *pItem = reinterpret_cast<FileData*>(pView->GetTreeCtrl().GetItemData(pView->GetTreeCtrl().GetSelectedItem()));


	if( (m_iStatus & ST_MOBILE_LINK) && (pWnd->GetListItemSelectedCount() == 1)&&(_tcscmp(MSOUTLOOK,pItem->sMode.c_str()))&&(_tcscmp(CSVFILE,pItem->sMode.c_str())))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);

/*	CPrevView *pWnd = reinterpret_cast<CPrevView*>(m_wndSplitter.GetPane(0,1));
	FileData *pLeftMode = pWnd->GetLeftMode();

	if( (m_iStatus & ST_MOBILE_LINK) && ((pLeftMode->sMode == MEMORY) ||
		(pLeftMode->sMode == SIM_CARD)||(pLeftMode->sMode == MOBILE_PHONE)) && 
		(pWnd->GetListItemSelectedCount() == 1))
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);*/
}

void CMainFrame::OnUpdateBnNew(CCmdUI* pCmdUI) 
{
	CPrevView *pWnd = reinterpret_cast<CPrevView*>(m_wndSplitter.GetPane(0,1));
	FileData *pLeftMode = pWnd->GetLeftMode();

	if( (m_iStatus & ST_MOBILE_LINK) && ((pLeftMode->sMode == MEMORY) ||
		(pLeftMode->sMode == SIM_CARD) ||(pLeftMode->sMode == MOBILE_PHONE)) &&
		!(m_iStatus & ST_DOWNLOAD_FAIL))	
	{
		if(pLeftMode->sMode == MEMORY && ((CPhoneBookApp*)afxGetApp())->m_bNotSupportME )
			pCmdUI->Enable(FALSE);
		else if(pLeftMode->sMode == SIM_CARD && ((CPhoneBookApp*)afxGetApp())->m_bNotSupportSM )
			pCmdUI->Enable(FALSE);
		else
			pCmdUI->Enable(TRUE);
	}
	else
		pCmdUI->Enable(FALSE);
}

void CMainFrame::OnUpdateBnRefresh(CCmdUI* pCmdUI) 
{
	//if mobile is connected ,set TRUE.
	//else set FALSE.
//	if(theApp.m_bLoadMSOutlook)
	{
		bool bEnable = false;
		CTreeView *pView = static_cast<CTreeView*>(m_wndSplitter.GetPane(0,0));
		if(pView)
		{
			HTREEITEM hItem = pView->GetTreeCtrl().GetSelectedItem();
			if(hItem)
			{
				FileData *pData = reinterpret_cast<FileData*>(pView->GetTreeCtrl().GetItemData(hItem));
				//send message to preview window with selected file data
				if(pData)
				{
					if(pData->sMode == MOBILE_PHONE || pData->sMode == MEMORY || pData->sMode== SIM_CARD)
					{
						if(m_iStatus & ST_MOBILE_LINK) bEnable = true;
					}
					else if(pData->sMode == MSOUTLOOK)
						bEnable = true;
				}
			}
		}
		pCmdUI->Enable(bEnable);
	}
/*	else
	{
		if(m_iStatus & ST_MOBILE_LINK)	
			pCmdUI->Enable(TRUE);
		else 
			pCmdUI->Enable(FALSE);
	}*/
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
	if(m_iStatus & ST_MOBILE_LINK && pWnd->IsItemInList())
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);	
}

void CMainFrame::OnUpdateStatusConnect(CCmdUI* pCmdUI)
{
	//Set status bar text
	TCHAR szText[MAX_PATH];
	memset(szText,0,sizeof(TCHAR) * MAX_PATH);
	//check if not
	if(!(m_iStatus & ST_WAIT_INIT)){
		if(m_iStatus & ST_MOBILE_LINK){
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
	
	pCmdUI->SetText(szText);
	
	CClientDC dc(this);
	SIZE size = dc.GetTextExtent(szText);
	m_wndStatusBar.SetPaneInfo(pCmdUI->m_nIndex,pCmdUI->m_nID,SBPS_NOBORDERS,size.cx);

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
	}
	else{
		m_iStatus &= ~ST_MOBILE_LINK;
		m_bOpenSync = false;
	}
	return 1L;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
	return CFrameWnd::PreTranslateMessage(pMsg);
}

HRESULT CMainFrame::OnMobileConnectStatus(WPARAM wParam,LPARAM lParam)
{
	int iState =(int) wParam; 
	if(iState == Anw_MOBILE_CONNECT_FAILED){
		m_iStatus &= ~ST_MOBILE_LINK;
		m_iStatus &= ~ST_WAIT_INIT;
		m_pView->SetDriverOpenFlag(false);
		m_pLeft->SetConnectFlag(false);
		m_pView->SetConnectStatus(false);
		m_bFirst = true;
	}	
	else if(iState == Anw_SUCCESS){
		m_iStatus |= ST_MOBILE_LINK;
		m_iStatus &= ~ST_WAIT_INIT;
		m_pView->SetDriverOpenFlag(true);
		m_pLeft->SetConnectFlag(true);
		if(m_bFirst){
			m_bFirst = false;
//			::PostMessage(m_pActive->m_pLeft->GetSafeHwnd(),WM_LOADMOBILE,m_pActive->m_bOpenSync == true ? 1 : 0,0);
			::PostMessage(m_pView->GetSafeHwnd(),WM_LOADMOBILE,0,0);
		}
	}
	if(m_bCapture)
	{
		SetCursor( LoadCursor(NULL, IDC_ARROW) );
		ReleaseCapture();
		m_bCapture = FALSE;
	}
	return 1L;
}
//The function set to driver.It can recive the connected status.
int CMainFrame::ConnectStatusCallback(int iState)
{
	CWnd* pWnd = afxGetMainWnd();
	if(pWnd)
	{
		if(::IsWindow(pWnd->m_hWnd))
			pWnd->SendMessage(WM_MOBILE_CONNECT,iState,NULL);
	}
	//Set status flag 
/*	if(iState == Anw_MOBILE_CONNECT_FAILED){
		m_iStatus &= ~ST_MOBILE_LINK;
		m_iStatus &= ~ST_WAIT_INIT;
		m_pActive->m_pView->SetDriverOpenFlag(false);
		m_pActive->m_pLeft->SetConnectFlag(false);
		m_pActive->m_pView->SetConnectStatus(false);
		m_pActive->m_bFirst = true;
	}	
	else if(iState == Anw_SUCCESS){
		m_iStatus |= ST_MOBILE_LINK;
		m_iStatus &= ~ST_WAIT_INIT;
		m_pActive->m_pView->SetDriverOpenFlag(true);
		m_pActive->m_pLeft->SetConnectFlag(true);
		if(m_pActive->m_bFirst){
			m_pActive->m_bFirst = false;
//			::PostMessage(m_pActive->m_pLeft->GetSafeHwnd(),WM_LOADMOBILE,m_pActive->m_bOpenSync == true ? 1 : 0,0);
			::PostMessage(m_pActive->m_pView->GetSafeHwnd(),WM_LOADMOBILE,0,0);
		}
	}
	if(m_pActive->m_bCapture)
	{
		SetCursor( LoadCursor(NULL, IDC_ARROW) );
		ReleaseCapture();
		m_pActive->m_bCapture = FALSE;
	}
*/

	return 0;
}

HRESULT CMainFrame::OnSetSearchFlag(WPARAM wParam,LPARAM lParam)
{
	m_bSearch = wParam == 0 ? false : true;
	return 1L;
}

LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
    HKEY hkey;
    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);	
	
    if (retval == ERROR_SUCCESS) {		
        long datasize = MAX_PATH;		
        TCHAR data[MAX_PATH];		
        RegQueryValue(hkey, NULL, data, &datasize);		
        lstrcpy(retdata,data);		
        RegCloseKey(hkey);		
    }
    return retval;
}
/*
void LaunchHTMLFile( LPCTSTR filename )
{
    TCHAR key[MAX_PATH + MAX_PATH];
	if (GetRegKey(HKEY_CLASSES_ROOT, _T(".htm"), key) == ERROR_SUCCESS) {
		lstrcat(key, _T("\\shell\\open\\command"));
		if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) {
			TCHAR *pos;
			pos = _tcsstr(key, _T("\"%1\""));
			if (pos == NULL) {                     // No quotes found
				pos = _tcsstr(key, _T("%1"));      // Check for %1, without quotes 				
				if (pos == NULL)                   // No parameter at all...					
					pos = key+lstrlen(key)-1;				
				else					
					*pos = '\0';                   // Remove the parameter				
			}			
			else				
				*pos = '\0';                       // Remove the parameter	
			lstrcat(pos, _T(" "));			
			lstrcat(pos, filename);			
			HINSTANCE result = (HINSTANCE) WinExec(T2A(key),SW_SHOW);			
		}		
    }	
}
*/
void CMainFrame::OnBnHelp() 
{
	CString str, str1;
	//make fullpath of help.ini
	CString	sHelpProfile = CString(theApp.m_szModule) + CString(_T("Help.ini"));
	//read lang key from lang.ini
	if(!al_GetSettingString(_T("Language"), _T("AP_Lang"), theApp.m_szLang, str.GetBuffer(MAX_PATH) ) )
		::AfxMessageBox( _T("Error lang.ini!"));
	//read help path from help.ini
	//MainPanel, Phonebook, Calendar, SMS, MMS, PhotoManager, RingtoneManager, Sync
	if(!al_GetSettingString( (LPTSTR)(LPCTSTR)str, _T("Phonebook"), (LPTSTR)(LPCTSTR)sHelpProfile, str1.GetBuffer(MAX_PATH) ) )
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

void CMainFrame::OnBnAbout() 
{
//	CAboutDlg dlg;
//	dlg.DoModal();
	TCHAR szText[MAX_PATH];
	TCHAR szFile[MAX_PATH];
	al_GetSettingString(_T("public"),_T("IDS_TITLE"),theApp.m_szRes,szText);
	GetModuleFileName(NULL,szFile,MAX_PATH);
	AboutDlg(m_hWnd,szText,szFile);
}

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;	
//	return CFrameWnd::OnEraseBkgnd(pDC);
}

HRESULT CMainFrame::OnProgDlgShow(WPARAM wParam,LPARAM lParam)
{
//	m_wndToolBar.EnableWindow(wParam);
	m_wndRebar.EnableWindow(wParam);
	m_wndSplitter.EnableWindow(wParam);
	return 1L;
}

HRESULT CMainFrame::OnSetLeftItemCount(WPARAM wParam,LPARAM lParam)
{
	FileData *pData = reinterpret_cast<FileData*>(wParam);
	m_pLeft->SetItemCount(*pData,lParam);
	return 1L;
}
HRESULT CMainFrame::OnCheckLeftTreeItem(WPARAM wParam,LPARAM lParam)
{
	m_pLeft->CheckTreeItem();
	return 1L;
}

void CMainFrame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	// DLL CONTRUCT
	if(afxContextIsDLL)
	{
	  if(GetActiveDocument())
	  {
	    DocViewClose(((CPhoneBookApp *)afxGetApp())->AppHwnd);
	  }
      else
	  {
	    if(!m_pView->CheckProgDlg())
		    CFrameWnd::OnClose();
	  }

	  // What will happen if the DLL cause unexpected quit???
      //::PostMessage(((CPhoneBookApp *)afxGetApp())->AppHwnd,ID_EXIT_DLL_MESSAGE,0,0);
	}
	else
	{
	  if(!m_pView->CheckProgDlg())
	     CFrameWnd::OnClose();
    }
}

HRESULT CMainFrame::OnDownloadFail(WPARAM wParam,LPARAM lParam)
{
	if(wParam == 1){
		m_iStatus |= ST_DOWNLOAD_FAIL;
		m_pLeft->SetDLFailFlag(true);
	}
	else {
		m_iStatus &= ~ST_DOWNLOAD_FAIL;
		m_pLeft->SetDLFailFlag(false);
	}
	return 1L;
}

void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);
	if(m_pView && m_pView->m_pThread == NULL && m_pView->m_dlgProg && m_pView->m_dlgProg.GetSafeHwnd() )
		m_pView->m_dlgProg.ShowWindow(SW_HIDE);
	// TODO: Add your message handler code here
	
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

//Ken 7/28  DLL CONTRUCT
void CMainFrame::UpdateMyDLLCmdUI()
{
	   m_wndStatusBar.OnIdleUpdateCmdUI(TRUE, 0L);
	   m_wndToolBar1.OnIdleUpdateCmdUI(TRUE, 0L);
	   m_wndToolBar2.OnIdleUpdateCmdUI(TRUE, 0L);
}

void CMainFrame::OnUpdateBnSendsms(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here

	if(m_bSupportDirectSMS)
	{
		CPrevView *pWnd = reinterpret_cast<CPrevView*>(m_wndSplitter.GetPane(0,1));
		if( (m_iStatus & ST_MOBILE_LINK) && (pWnd->GetListItemSelectedCount() >= 1))
		{
			CString str = pWnd->GetCurrentSelectPhoneNoString();
			if(str.GetLength() >0)
				pCmdUI->Enable(TRUE);
			else
				pCmdUI->Enable(FALSE);
		}
		else
			pCmdUI->Enable(FALSE);
	}
	else	
		pCmdUI->Enable(FALSE);

	
}

void CMainFrame::OnBnSendsms() 
{
	// TODO: Add your command handler code here
	::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_SENDSMS,1,0);
	
}
