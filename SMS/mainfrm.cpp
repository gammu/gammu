// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "SMSUtility.h"
#include "MainFrm.h"


#include "NewSMS.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define FRM_WIDTH 722
//#define FRM_HEIGHT 600
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


	/*if((dwKeyState & MK_CONTROL) == MK_CONTROL)
        return DROPEFFECT_MOVE;
    // Otherwise return a Œ/..drop effect of MOVE
    else
        return DROPEFFECT_MOVE;   */
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
	ON_COMMAND(ID_BN_REFRESH,OnRefresh)
	ON_COMMAND(ID_BN_BACK, OnBnBack)
	ON_WM_CLOSE()
	ON_COMMAND(ID_BN_HELP, OnBnHelp)
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI(IDS_CONNECT_STATUS, OnUpdateStatusConnect)	
	ON_WM_HSCROLL()
	ON_UPDATE_COMMAND_UI(ID_BN_REFRESH, OnUpdateBnRefresh)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOBILE_CONNECT, OnMobileConnectStatus)
	ON_MESSAGE(WM_START_CONNECT, OnStartConnect)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	IDS_CONNECT_STATUS,
	//ID_SEPARATOR,
	//IDS_COMPORT,
	/*ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,*/
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	//m_iStatus = 0;
		
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
	m_sProfile = Tstring(theApp.m_szSkin) + Tstring(_T("SMSUtility\\main.ini"));
	
	
	//Create rebar
	m_wndRebar.Create(this,RBS_FIXEDORDER);
	//Create the tool bar
	if (!m_wndToolBar1.CreateEx(&m_wndRebar, TBSTYLE_BUTTON, WS_CHILD | WS_VISIBLE | CBRS_TOP) ||
		!m_wndToolBar1.LoadToolBar(IDR_TB_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	
	
	// Create tool bar
	CMyToolBar& tb = m_wndToolBar1;
	VERIFY(tb.LoadToolBar(IDR_TB_TOOLBAR)); 

	// use transparent so coolbar bitmap will show through
	tb.ModifyStyle(0, TBSTYLE_TRANSPARENT);

	// show button text on right
	tb.ModifyStyle(0, TBSTYLE_LIST);

	CToolBarCtrl& tbc = tb.GetToolBarCtrl();


    DWORD dwStyleEx1 = tbc.GetExtendedStyle();
    tbc.SetExtendedStyle(dwStyleEx1 | TBSTYLE_EX_MIXEDBUTTONS);
    tbc.SetMaxTextRows(1);

	
	TCHAR szSkin[MAX_PATH];
	if(al_GetSettingString(_T("toolbar1"), _T("image"), const_cast<TCHAR *>(m_sProfile.c_str()),szSkin)){
		Tstring sSkin = Tstring(theApp.m_szSkin) + Tstring(szSkin);
		m_wndToolBar1.SetButtonImage(sSkin.c_str(), 7);
	}

		//set tooltip
	TCHAR szTipSec[7][32] = {_T("ID_BN_NEW"),_T("ID_BN_REPLY"),_T("ID_BN_SAVE"),_T("ID_BN_PRINT"),
		_T("ID_BN_DELETE"),_T("ID_BN_RESEND"),_T("ID_BN_REFRESH")};
	for(int i = 0 ,j = 0; i < 7 ; i ++,j++){
		TCHAR szTxt[MAX_PATH];
		if(m_wndToolBar1.GetItemID(j) == 0)
			j ++;
		if(al_GetSettingString(_T("public"),szTipSec[i],theApp.m_szRes,szTxt))
			m_wndToolBar1.SetToolTipText(szTxt,j);
	}
	//Set button image and button count
	//m_wndToolBar1.SetButtonImage("tool1", 7);

	//Set background color
/*	COLORREF color;
	if(al_GetSettingColor("toolbar","color",const_cast<char *>(m_sProfile.c_str()),color))
		m_wndToolBar.SetColorBk(color);
*/
	TCHAR buf[_MAX_PATH];
	if(al_GetSettingString(_T("funcbar"),_T("image"),const_cast<TCHAR *>(m_sProfile.c_str()), buf))
	{
		//m_wndToolBar.SetColorBk(color);
		Tstring sSkin(theApp.m_szSkin);
		sSkin += Tstring(buf);
		m_wndToolBar1.SetImgBk(sSkin.c_str());
	}

	//Create toolbar1 and toolbar2
	if (!m_wndToolBar2.CreateEx(&m_wndRebar, TBSTYLE_BUTTON, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS) ||
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
	//m_wndToolBar2.SetButtonImage("tool2", 3);
	if(al_GetSettingString(_T("toolbar2"), _T("image"), const_cast<TCHAR *>(m_sProfile.c_str()),szSkin)){
		Tstring sSkin = Tstring(theApp.m_szSkin) + Tstring(szSkin);
		m_wndToolBar2.SetButtonImage(sSkin.c_str(), 3);
	}


	//set tooltip
	TCHAR szTipSec1[3][32] = {_T("ID_BN_BACK"),_T("ID_BN_ABOUT"),_T("ID_BN_HELP")};
	for(i = 0 , j = 0; i < 3 ; i ++,j++)
	{
		TCHAR szTxt[MAX_PATH];
		if(m_wndToolBar2.GetItemID(j) == 0)
			j ++;

		if(al_GetSettingString(_T("public"), szTipSec1[i], theApp.m_szRes, szTxt))
			m_wndToolBar2.SetToolTipText(szTxt, j);
	}

	//Set the background image
	if(al_GetSettingString(_T("funcbar"),_T("image"),const_cast<TCHAR *>(m_sProfile.c_str()), buf))
	{
		//m_wndToolBar.SetColorBk(color);
		Tstring sSkin(theApp.m_szSkin);
		sSkin += Tstring(buf);
		m_wndToolBar2.SetImgBk(sSkin.c_str());
	}

	CImage img;
	Tstring sSkin(theApp.m_szSkin);
	sSkin += Tstring(buf);
	img.LoadFile(sSkin.c_str());

	HBITMAP	hbm;
	img.GetHBITMAP( Color::White, &hbm );
	m_BmpToolBar.Attach( hbm );

	m_wndRebar.AddBar(&m_wndToolBar1,NULL,&m_BmpToolBar,RBBS_FIXEDBMP);
	m_wndRebar.AddBar(&m_wndToolBar2,NULL,&m_BmpToolBar,RBBS_FIXEDBMP);

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

	//Maximize Window or Normal size
	int	nMaxView = 1;
	al_GetSettingInt(_T("panel"),_T("maxview"),const_cast<TCHAR *>(m_sProfile.c_str()),nMaxView);
	if(nMaxView==1)
	{
		ShowWindow(SW_MAXIMIZE);
	}


//	m_nTimer = SetTimer(1, 500, 0);
	//m_droptarget.Register(this);
	m_wndSplitter.SetBackground(RGB(212,222,234));

	PostMessage(WM_START_CONNECT,0,0);

	//m_wndToolBar.SetButtonText(9, "TEST");

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	//cs.dwExStyle |= WS_BORDER;
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	//cs.lpszClass = AfxRegisterWndClass(0);
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

	m_wndSplitter.SetColumnInfo(0, 246, 100);	
	m_wndSplitter.SetBackground(RGB(212,222,234));

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
	//CNewSMS	NewSMSDlg;

	//NewSMSDlg.DoModal();
	GetLeftPane()->xNewSMS();

}

/*void CMainFrame::OnSave()
{
}*/

/*void CMainFrame::OnDelete()
{
	//::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_DELSEL,0,0);
}*/

/*void CMainFrame::OnPrint()
{
}*/

/*void CMainFrame::OnReSend()
{
}*/

void CMainFrame::OnRefresh()
{
	GetLeftPane()->xRefreshSMS();
}


CLeftView* CMainFrame::GetLeftPane()
{
	CWnd* pWnd = m_wndSplitter.GetPane(0, 0);
	CLeftView *pTree = DYNAMIC_DOWNCAST(CLeftView, pWnd);
	return pTree;
}

CPrevView* CMainFrame::GetRightPane()
{
	CWnd* pWnd = m_wndSplitter.GetPane(0, 1);
	CPrevView* pView = DYNAMIC_DOWNCAST(CPrevView, pWnd);
	return pView;
}






void CMainFrame::OnBnBack() 
{
	// TODO: Add your command handler code here
	OnClose();
}

void CMainFrame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	// DLL CONTRUCT
	if(afxContextIsDLL)
	{
	  if(GetActiveDocument())
	  {
	    DocViewClose(((CSMSUtility *)afxGetApp())->AppHwnd);
	  }
      else
  	    CFrameWnd::OnClose();

	  // Ken 7/28
	  // What will happen if the DLL cause unexpected quit???
      //::PostMessage(((CSMSUtility *)afxGetApp())->AppHwnd,ID_EXIT_DLL_MESSAGE,0,0);
	}
	else
	  CFrameWnd::OnClose();
}

void CMainFrame::OnBnHelp() 
{
	// TODO: Add your command handler code here
	CString str, str1;
	//make fullpath of help.ini
	CString	sHelpProfile = CString(theApp.m_szModule) + CString(_T("Help.ini"));
	//read lang key from lang.ini
	if(!al_GetSettingString(_T("Language"), _T("AP_Lang"), theApp.m_szLang, str.GetBuffer(MAX_PATH) ) )
		::AfxMessageBox( _T("Error lang.ini!"));
	//read help path from help.ini
	//MainPanel, Phonebook, Calendar, SMS, MMS, PhotoManager, RingtoneManager, Sync
	if(!al_GetSettingString( (LPTSTR)(LPCTSTR)str, _T("SMS"), (LPTSTR)(LPCTSTR)sHelpProfile, str1.GetBuffer(MAX_PATH) ) )
		::AfxMessageBox( _T("Error Help.ini!"));

	//make fullpath of help file, and full command line
	str.Format( _T("hh.exe \"%s%s\""), theApp.m_szModule, str1 );
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



LONG CMainFrame::GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
    HKEY hkey;

    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if(retval == ERROR_SUCCESS) 
	{
        long datasize = MAX_PATH;
        TCHAR data[MAX_PATH];

        RegQueryValue(hkey, NULL, data, &datasize);

        lstrcpy(retdata,data);

        RegCloseKey(hkey);
    }

    return retval;
}

void CMainFrame::UpdateMyDLLCmdUI()
{
	   m_wndStatusBar.OnIdleUpdateCmdUI(TRUE, 0L);
	   m_wndToolBar1.OnIdleUpdateCmdUI(TRUE, 0L);
	   m_wndToolBar2.OnIdleUpdateCmdUI(TRUE, 0L);
}


void CMainFrame::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
//	KillTimer(m_nTimer); 

//	GetLeftPane()->xStartConnect();

	CFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::OnUpdateStatusConnect(CCmdUI* pCmdUI)
{
	//Set status bar text
	TCHAR szTmp[MAX_PATH];// = "%d is connected";	
	memset(&szTmp, 0, MAX_PATH);


	GetLeftPane()->XGetConnectStatus(szTmp);
	pCmdUI->SetText(szTmp);
	//pCmdUI->SetText("");
	
	CClientDC dc(this);
	SIZE size = dc.GetTextExtent(szTmp);
	m_wndStatusBar.SetPaneInfo(pCmdUI->m_nIndex,pCmdUI->m_nID,SBPS_NOBORDERS,size.cx);

}

void CMainFrame::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	
	CFrameWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

/*BOOL CMainFrame::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	//Invalidate(false);
	//m_wndSplitter.
	return CFrameWnd::OnEraseBkgnd(pDC);
}*/

void CMainFrame::OnUpdateBnRefresh(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(theApp.nIsConnected == true)
		pCmdUI->Enable(true);
	else
		pCmdUI->Enable(false);
}

int CMainFrame::GetMobileSetting(void)
{
	TCHAR FileName[MAX_PATH];
	CString strFileName;

	GetModuleFileName(NULL, FileName, MAX_PATH);
	strFileName = FileName;
	strFileName = strFileName.Left(strFileName.ReverseFind('\\'));
	strFileName +=_T("\\mobile_setting.ini");

	_tcscpy(FileName, strFileName.GetBuffer(strFileName.GetLength()));

	int ret = al_GetSettingString(_T("mobile"), _T("name"), FileName, m_strMobileName);
	ret = al_GetSettingString(_T("mobile"), _T("phone"), FileName, m_strPhone);
	
	return true;
}
HRESULT CMainFrame::OnMobileConnectStatus(WPARAM wParam,LPARAM lParam)
{
	int iState =(int) wParam; 
	CLeftView* pLeftView =  CMainFrame::GetLeftPane();
	pLeftView->UpdateMobileConnectStatus(iState);

	return 1L;
}
HRESULT CMainFrame::OnStartConnect(WPARAM wParam,LPARAM lParam)
{
	CLeftView* pLeftView =  CMainFrame::GetLeftPane();
	pLeftView->xStartConnect();
	return 1L;
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

void CMainFrame::OnDestroy() 
{
	theApp.m_bExit = true;
	CFrameWnd::OnDestroy();
	
	// TODO: Add your message handler code here
	
}
