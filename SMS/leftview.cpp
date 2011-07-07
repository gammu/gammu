// LeftView.cpp : implementation file
//

#include "stdafx.h"
#include "SMSUtility.h"
#include "LeftView.h"
#include "PrevView.h"
#include "MainFrm.h"
#include "ProcessDlg.h"
#include "CommUIExportFun.h"
#include "ProgressDlg.h"


#include "NewSMS.h"




#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IMAGE_WIDTH 30
#define IMAGE_HEIGHT 30

#define ILI_MOBILE				0 
#define ILI_PC					1


static int  g_nDeletePosition = 0;
static bool g_ConnectFail = false;


HTREEITEM gMobileItem;
HTREEITEM gPCItem;

HTREEITEM gMobileSIMCardItem;
HTREEITEM gMobileSIMCardInboxItem;
HTREEITEM gMobileSIMCardOutboxItem;
//HTREEITEM gMobileSIMCardSendBKItem;
//HTREEITEM gMobileSIMCardDraftItem;
//HTREEITEM gMobileSIMCardGarbageItem;

HTREEITEM gMobileHandsetItem;
HTREEITEM gMobileHandsetInboxItem;
HTREEITEM gMobileHandsetOutboxItem;
// Folder1 - Folder2
HTREEITEM gMobileHandsetFolder1Item;
HTREEITEM gMobileHandsetFolder2Item;
HTREEITEM gMobileHandsetFolder3Item;
HTREEITEM gMobileHandsetFolder4Item;
HTREEITEM gMobileHandsetFolder5Item;
//HTREEITEM gMobileHandsetSendBKItem;
//HTREEITEM gMobileHandsetDraftItem;
//HTREEITEM gMobileHandsetGarbageItem;

// PC
HTREEITEM gPCInboxItem;
HTREEITEM gPCOutboxItem;
HTREEITEM gPCSendBKItem;
HTREEITEM gPCDraftItem;
HTREEITEM gPCGarbageItem;

// Folder Name
#define SMS_FOLDER		_T("\\SMSUtility")
#define INBOX_FOLDER	_T("\\inbox")
#define OUTBOX_FOLDER	_T("\\outbox")
#define BACKUP_FOLDER	_T("\\sendbox")
#define DRAFT_FOLDER	_T("\\draftbox")
#define TRASH_FOLDER	_T("\\trashbox")



anwOpenSMS			ANWOpenSMS;
anwCloseSMS			ANWCloseSMS;
anwGetMobileSMSInfo ANWGetMobileSMSInfo;
anwGetMobileAllSMS	ANWGetMobileAllSMS;
anwGetSMSStartData	ANWGetSMSStartData;
anwGetSMSOneData	ANWGetSMSOneData;
anwGetSMSNextData	ANWGetSMSNextData;
anwSendSMSData		ANWSendSMSData;
anwPutSMSData		ANWPutSMSData;
anwDeleteSMSData	ANWDeleteSMSData;
anwDeleteAllSMSData ANWDeleteAllSMSData;
anwInitSMS 			ANWInitSMS;
#ifdef ASUSM303
anwGetMobileSMSFolders	ANWGetMobileSMSFolders;
#endif

OpenPhoneBookProc			OpenPhoneBookfn;
ClosePhoneBookProc			ClosePhoneBookfn;
GetPhoneBookInfoProc		GetPhoneBookInfofn;
GetMobileInfoProc			GetMobileInfofn;
GetPhoneBookStartDataProc	GetPhoneBookStartDatafn;
GetPhoneBookNextDataProc	GetPhoneBookNextDatafn;
InitPhoneBookProc			InitPhoneBookfn;
InitialMemoryStatusProc		InitialMemoryStatusfn;

//CheckSIMProc				CheckSIMfn;
//GetIMEIProc			ANWGetIMEI;


CLeftView*	CLeftView::s_ActiveDlg = NULL;
//bool	CLeftView::s_HasSIMCard = false;
//DLL CONTRUCT
bool bHasConnected = false;
/////////////////////////////////////////////////////////////////////////////
// CLeftView
//const int SMS_MOBILE_ItemCount = 300;//255;
//const int SMS_ItemCount = 1000;//255;

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)
int afxMessageBox( LPCTSTR lpszText)
{
	CWnd *pWnd = afxGetMainWnd();
	if(pWnd)
	{
		CString strTitle;
		strTitle.Empty();
		pWnd->GetWindowText(strTitle);
		return ::MessageBox(pWnd->m_hWnd,lpszText,strTitle,MB_OK|MB_ICONWARNING);
	}
	return AfxMessageBox(lpszText);
}
CLeftView::CLeftView()
{
	memset(m_TreeItemBackup, 0, 255);

	m_SIMSMS_Inbox = new SMS_PARAM[SMS_MOBILE_ItemCount];
	memset(m_SIMSMS_Inbox, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	m_SIMSMS_Outbox = new SMS_PARAM[SMS_MOBILE_ItemCount];
	memset(m_SIMSMS_Outbox, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	m_HandsetSMS_Inbox = new SMS_PARAM[SMS_MOBILE_ItemCount];
	memset(m_HandsetSMS_Inbox, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	m_HandsetSMS_Outbox = new SMS_PARAM[SMS_MOBILE_ItemCount];
	memset(m_HandsetSMS_Outbox, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);

	m_Mobile_SMS = new SMS_PARAM[SMS_MOBILE_ItemCount];
	memset(m_Mobile_SMS, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	m_Mobile_DstSMS = NULL;

#ifdef ASUSM303
	m_HandsetSMS_Folder1 = new SMS_PARAM[SMS_MOBILE_ItemCount];
	memset(m_HandsetSMS_Folder1, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	m_HandsetSMS_Folder2 = new SMS_PARAM[SMS_MOBILE_ItemCount];
	memset(m_HandsetSMS_Folder2, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	m_HandsetSMS_Folder3 = new SMS_PARAM[SMS_MOBILE_ItemCount];
	memset(m_HandsetSMS_Folder3, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	m_HandsetSMS_Folder4 = new SMS_PARAM[SMS_MOBILE_ItemCount];
	memset(m_HandsetSMS_Folder4, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	//m_HandsetSMS_Folder5 = new SMS_PARAM[SMS_MOBILE_ItemCount];
	//memset(m_HandsetSMS_Folder5, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);

	memset(&mfolders, 0, sizeof(GSM_SMSFolders));
#endif
	
	

	m_PC_Inbox = new SMS_PARAM[SMS_ItemCount];
	memset(m_PC_Inbox, 0, sizeof(SMS_PARAM)*SMS_ItemCount);
	m_PC_Outbox = new SMS_PARAM[SMS_ItemCount];
	memset(m_PC_Outbox, 0, sizeof(SMS_PARAM)*SMS_ItemCount);
	m_PC_SendBK = new SMS_PARAM[SMS_ItemCount];
	memset(m_PC_SendBK, 0, sizeof(SMS_PARAM)*SMS_ItemCount);
	m_PC_Draft = new SMS_PARAM[SMS_ItemCount];
	memset(m_PC_Draft, 0, sizeof(SMS_PARAM)*SMS_ItemCount);
	m_PC_Garbage = new SMS_PARAM[SMS_ItemCount];
	memset(m_PC_Garbage, 0, sizeof(SMS_PARAM)*SMS_ItemCount);

	memset(m_Empty_box, 0, sizeof(SMS_PARAM)*10);
	memset(&m_arrayDeletePosition, 0, SMS_MOBILE_ItemCount);
	


	memset(m_strPort, 0, sizeof(TCHAR)*MAX_PATH);
	memset(m_strPhone, 0, sizeof(TCHAR)*MAX_PATH);
	memset(m_strConnectMode, 0, sizeof(TCHAR)*MAX_PATH);
	memset(m_strMobileName, 0, sizeof(TCHAR)*MAX_PATH);
	memset(m_strMobileName_ori, 0, sizeof(TCHAR)*MAX_PATH);
	memset(m_strCompany, 0, sizeof(TCHAR)*MAX_PATH);
	memset(m_strIMEI, 0, sizeof(TCHAR)*MAX_PATH);
	memset(m_strMobileDLL, 0, sizeof(TCHAR)*MAX_PATH);


	memset(&m_sms_Tal_Num, 0, sizeof(SMS_Tal_Num));


	m_crText = RGB(  0,   0,   0);	// Black
	m_crBack = RGB(255, 255, 255);	// White
	m_crLine = RGB(132, 132, 132);	// Light grey

	m_brush.CreateSolidBrush(m_crBack);

	m_hIconBtn[0] = m_hIconBtn[1] = NULL;

	s_ActiveDlg = this;
	m_bStart = true;
	m_bFirst = true;
	AsusNewTreeBuild = false;
	m_hFont = NULL;

	m_CtrlKeyDown = false;
	m_bCapture = FALSE;
	m_bDisplayConnectStatus = FALSE;
	m_bRuningDlg = false;
}

CLeftView::~CLeftView()
{
	//ANWCloseSMS();
	delete []m_SIMSMS_Inbox;
	delete []m_SIMSMS_Outbox;
	delete []m_HandsetSMS_Inbox;
	delete []m_HandsetSMS_Outbox;

	delete []m_Mobile_SMS;
	//delete []m_Mobile_DstSMS;
	

#ifdef ASUSM303
	delete []m_HandsetSMS_Folder1;
	delete []m_HandsetSMS_Folder2;
	delete []m_HandsetSMS_Folder3;
	delete []m_HandsetSMS_Folder4;
	//delete []m_HandsetSMS_Folder5;
#endif

	delete []m_PC_Inbox;
	delete []m_PC_Outbox;
	delete []m_PC_SendBK;
	delete []m_PC_Draft;
	delete []m_PC_Garbage;

	ANWCloseSMS();
	if(m_hMobileDll != NULL)
	{	
		FreeLibrary(m_hMobileDll);
	}

	if(m_hFont)
		::DeleteObject(m_hFont);
}


BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
	//{{AFX_MSG_MAP(CLeftView)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(TVN_SELCHANGING, OnSelchanging)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY_REFLECT(TVN_BEGINRDRAG, OnBeginrdrag)
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT( NM_CUSTOMDRAW, OnCustomDraw )
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	ON_WM_LBUTTONDOWN()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// DLL CONTRUCT
void CLeftView::FreeLibrary(HMODULE m_hDriver)
{
   if(!afxContextIsDLL)
	   ::FreeLibrary(m_hDriver);
}


void CLeftView::OnPaint()
{
	/*CPaintDC dc(this);*/

	CTreeView::OnPaint();
}
CPrevView *gPreListView;
int CLeftView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;


	m_DropTarget.Register(this);


	TCHAR szTmp[MAX_PATH];
	// Get Mobile Setting from mobile_setting.ini
	GetMobileSetting();


	TCHAR  szProfile[_MAX_PATH], szAppPath[_MAX_PATH];
	al_GetModulePath(NULL, szAppPath);
	wsprintf(szProfile, _T("%s%s"), szAppPath, _T("skin\\default\\font.ini"));
	//Load font setting for ComboBox control

/*	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfCharSet = theApp.m_lf.lfCharSet; 
	lf.lfHeight = theApp.m_lf.lfHeight;
	if(lf.lfHeight > 0)
		lf.lfHeight *= -1;
	strcpy(lf.lfFaceName, theApp.m_lf.lfFaceName);
	if(m_hFont==NULL)
		m_hFont = ::CreateFontIndirect(&lf);*/
	int iSize = 12;
	if(theApp.m_lf.lfHeight > 0)
		iSize = -theApp.m_lf.lfHeight ;
	else
		iSize = theApp.m_lf.lfHeight ;
	m_hFont = GetFontEx(theApp.m_lf.lfFaceName,iSize);

	CFont* pFont = CFont::FromHandle(m_hFont);
	
	GetTreeCtrl().SetFont(pFont);


	// Load Mobile Library
	//AfxOleInit();
	//m_droptarget.Register(this);

	// Initial PC fodler
	GetPCSMSInfo();
	

	//Create Imagelist
	m_ImgList.Create(IMAGE_WIDTH,IMAGE_WIDTH, ILC_COLOR16, 9, 1);
	
	m_sProfile = Tstring(theApp.m_szSkin) + Tstring(_T("SMSUtility\\LeftView.ini"));

	//load tree icon
	TCHAR szSkin[MAX_PATH];
	TCHAR szSec[TREE_ITEM_NUM][32] = { _T("mobile_item"),		// 0. Mobile
									   _T("pc_item"),			// 1. PC
									   _T("memory_item"),		// 2. Handset Memeor
									   _T("sim_item"),			// 3. SIM Card
									   _T("inbox_item"),		// 4. Inbox
									   _T("outbox_item"),		// 5. Outbox
									   _T("outbox_backup_item"),// 6. Outbox copy
									   _T("draft_item"),		// 7. Draft
									   _T("delete_item")};		// 8. Delete

										
	for(int i = 0 ; i < TREE_ITEM_NUM ; i++){
		if(al_GetSettingString(szSec[i],_T("image"),const_cast<TCHAR *>(m_sProfile.c_str()),szSkin)){
			Tstring sSkin(theApp.m_szSkin);		
			sSkin += szSkin;
			xAddToImageList(sSkin.c_str());
		}
	}
	//Set image to the tree control
	GetTreeCtrl().SetImageList(&m_ImgList , TVSIL_NORMAL);


	//Add TreeItems
	// Mobile
	CString strMobileName;
	strMobileName = m_strMobileName;
	HTREEITEM hParentMobile = GetTreeCtrl().InsertItem(strMobileName, ILI_MOBILE, ILI_MOBILE);
	gMobileItem = hParentMobile;

	//GetTreeCtrl().SetItemState(gMobileItem, TVIS_SELECTED, TVIS_SELECTED);
	GetTreeCtrl().SelectItem(gMobileItem);

	TCHAR strSetString[MAX_PATH];

	HTREEITEM hChildMobile[2];
	al_GetSettingString(_T("public"), _T("IDS_MOBILE"), theApp.m_szRes, szTmp);
	hChildMobile[0] = GetTreeCtrl().InsertItem(szTmp, 2, 2, hParentMobile);
	gMobileHandsetItem = hChildMobile[0];
	al_GetSettingString(_T("public"), _T("IDS_SIMCARD"), theApp.m_szRes, szTmp);
	hChildMobile[1] = GetTreeCtrl().InsertItem(szTmp, 3, 3, hParentMobile);
	gMobileSIMCardItem = hChildMobile[1];

	
	al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, 0);
	gMobileHandsetInboxItem = GetTreeCtrl().InsertItem(strSetString, 4, 4, hChildMobile[0]);
	al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, 0);
	gMobileHandsetOutboxItem = GetTreeCtrl().InsertItem(strSetString, 5, 5, hChildMobile[0]);
	
	
#ifdef ASUSM303
	// Foler1 - Foler5
	/*
	al_GetSettingString(_T("public"), "IDS_OUTBOX", theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, 0);
	gMobileHandsetFolder1Item = GetTreeCtrl().InsertItem("Other1(0)", 5, 5,hChildMobile[0]);
	GetTreeCtrl().SetItemState(gMobileHandsetFolder1Item, TVIS_OVERLAYMASK, TVIS_OVERLAYMASK);

	al_GetSettingString(_T("public"), "IDS_OUTBOX", theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, 0);
	gMobileHandsetFolder2Item = GetTreeCtrl().InsertItem("Other2(0)", 5, 5,hChildMobile[0]);
	GetTreeCtrl().SetItemState(gMobileHandsetFolder2Item, TVIS_OVERLAYMASK, TVIS_OVERLAYMASK);

	al_GetSettingString(_T("public"), "IDS_OUTBOX", theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, 0);
	gMobileHandsetFolder3Item = GetTreeCtrl().InsertItem("Other3(0)", 5, 5,hChildMobile[0]);
	GetTreeCtrl().SetItemState(gMobileHandsetFolder3Item, TVIS_OVERLAYMASK, TVIS_OVERLAYMASK);

	al_GetSettingString(_T("public"), "IDS_OUTBOX", theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, 0);
	gMobileHandsetFolder4Item = GetTreeCtrl().InsertItem("Other4(0)", 5, 5,hChildMobile[0]);
	GetTreeCtrl().SetItemState(gMobileHandsetFolder4Item, TVIS_OVERLAYMASK, TVIS_OVERLAYMASK);

	al_GetSettingString(_T("public"), "IDS_OUTBOX", theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, 0);
	gMobileHandsetFolder5Item = GetTreeCtrl().InsertItem("Other5(0)", 5, 5,hChildMobile[0]);
	GetTreeCtrl().SetItemState(gMobileHandsetFolder5Item, TVIS_OVERLAYMASK, TVIS_OVERLAYMASK);
	*/
#endif

	al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, 0);
	gMobileSIMCardInboxItem = GetTreeCtrl().InsertItem(strSetString, 4, 4, hChildMobile[1]);
	al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, 0);
	gMobileSIMCardOutboxItem = GetTreeCtrl().InsertItem(strSetString, 5, 5, hChildMobile[1]);


	// PC
	al_GetSettingString(_T("public"), _T("IDS_PC"), theApp.m_szRes, szTmp);
	HTREEITEM hParentPC = GetTreeCtrl().InsertItem(szTmp, ILI_PC, ILI_PC);
	gPCItem = hParentPC;

	al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, 0);
	gPCInboxItem = GetTreeCtrl().InsertItem(strSetString, 4, 4, hParentPC);
 
	al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, 0);
	gPCOutboxItem = GetTreeCtrl().InsertItem(strSetString, 5, 5,hParentPC);

	al_GetSettingString(_T("public"), _T("IDS_SENDBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, 0);
	gPCSendBKItem = GetTreeCtrl().InsertItem(strSetString, 6, 6, hParentPC);

	al_GetSettingString(_T("public"), _T("IDS_DRAFTBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, 0);
	gPCDraftItem = GetTreeCtrl().InsertItem(strSetString, 7, 7, hParentPC);

	al_GetSettingString(_T("public"), _T("IDS_TRASHBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, 0);
	gPCGarbageItem = GetTreeCtrl().InsertItem(strSetString, 8, 8,hParentPC);

	GetTreeCtrl().Expand(hParentPC, TVE_EXPAND); 
	GetTreeCtrl().Expand(hChildMobile[0], TVE_EXPAND); 
	GetTreeCtrl().Expand(hChildMobile[1], TVE_EXPAND); 
	GetTreeCtrl().Expand(hParentMobile, TVE_EXPAND);
	GetTreeCtrl().SelectSetFirstVisible(hParentMobile);
	

	GetTreeCtrl().SetIndent(10);
	GetTreeCtrl().SetItemHeight(35); 

	UpdateAllTreeItemFirst();
	
	CRect rc;
	GetTreeCtrl().GetWindowRect(&rc);
	ScreenToClient(&rc);
	rc.OffsetRect(200,0);
	GetTreeCtrl().MoveWindow(rc);


	cursor_arr = LoadCursor(NULL, IDC_UPARROW);
	cursor_no = LoadCursor(NULL, IDC_NO);

	GetTreeCtrl().SetBkColor(RGB(255,255,255));


	gPreListView = ((CMainFrame*)s_ActiveDlg->GetParentFrame())->GetRightPane();
	return 0;
}

void CLeftView::CreateNewTreeList(void)
{
	if(AsusNewTreeBuild == true)
		return;

#ifdef ASUSM303
	AsusNewTreeBuild = true;

	HTREEITEM rootItem, childItem;
	rootItem = GetTreeCtrl().GetRootItem();
	childItem = GetTreeCtrl().GetChildItem(rootItem);

	// Foler1 - Foler5
	if( _tcslen(mfolders.Folder[0].Name) != 0)
	{
		gMobileHandsetFolder1Item = GetTreeCtrl().InsertItem(mfolders.Folder[0].Name, 5, 5,childItem);
		GetTreeCtrl().SetItemState(gMobileHandsetFolder1Item, TVIS_OVERLAYMASK, TVIS_OVERLAYMASK);
	}

	if( _tcslen(mfolders.Folder[1].Name) != 0)
	{
		gMobileHandsetFolder2Item = GetTreeCtrl().InsertItem(mfolders.Folder[1].Name, 5, 5,childItem);
		GetTreeCtrl().SetItemState(gMobileHandsetFolder2Item, TVIS_OVERLAYMASK, TVIS_OVERLAYMASK);
	}


	if( _tcslen(mfolders.Folder[2].Name) != 0)
	{
		gMobileHandsetFolder3Item = GetTreeCtrl().InsertItem(mfolders.Folder[2].Name, 5, 5,childItem);
		GetTreeCtrl().SetItemState(gMobileHandsetFolder3Item, TVIS_OVERLAYMASK, TVIS_OVERLAYMASK);
	}

	if( _tcslen(mfolders.Folder[3].Name) != 0)
	{
		gMobileHandsetFolder4Item = GetTreeCtrl().InsertItem(mfolders.Folder[3].Name, 5, 5,childItem);
		GetTreeCtrl().SetItemState(gMobileHandsetFolder4Item, TVIS_OVERLAYMASK, TVIS_OVERLAYMASK);
	}

	/*if( _tcslen(mfolders.Folder[4].Name) != 0)
	{
		gMobileHandsetFolder5Item = GetTreeCtrl().InsertItem(mfolders.Folder[4].Name, 5, 5,childItem);
		GetTreeCtrl().SetItemState(gMobileHandsetFolder5Item, TVIS_OVERLAYMASK, TVIS_OVERLAYMASK);
	}*/
	
#endif
}

void CLeftView::xAddToImageList(LPCTSTR lpszPath)
{
	CImage img(lpszPath);    
	HBITMAP hbm;
	CBitmap Bmp;

	//Get Bitmap handle
	img.GetHBITMAP( Color::White, &hbm);

	//Attach to CBitmap class
	Bmp.Attach( hbm);

	//Add to CImageList class
	int ret = m_ImgList.Add(&Bmp,(CBitmap*)NULL);

	//Detach CBitmap
	Bmp.Detach();
}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if(!CTreeView::PreCreateWindow(cs))
		return false;
	// TODO: Add your specialized code here and/or call the base class
	cs.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS | WS_BORDER ;// | TVS_NOTOOLTIPS;//  /*TVS_LINESATROOT */;
	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	return true;//CTreeView::PreCreateWindow(cs);
}
/////////////////////////////////////////////////////////////////////////////




void CLeftView::OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here

	CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();


	HTREEITEM hCurSelItem = pNMTreeView->itemNew.hItem;
	int nCurSelItem = GetSelectItem(hCurSelItem);

	
	if(nCurSelItem == itMOBILE)
	{ 
		PreListView->ShowItems(m_Empty_box, nCurSelItem, 0); 
	}	
	else if(nCurSelItem == itMOBILE_SIMCARD)
	{ 
		PreListView->ShowItems(m_Empty_box, nCurSelItem, 0); 
	}
	else if(nCurSelItem == itMOBILE_SIMCARD_INBOX)
	{
		PreListView->ShowItems(m_SIMSMS_Inbox, nCurSelItem, m_TreeItemBackup[itMOBILE_SIMCARD_INBOX]);
	}
	else if(nCurSelItem == itMOBILE_SIMCARD_OUTBOX)
	{
		PreListView->ShowItems(m_SIMSMS_Outbox, nCurSelItem, m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX]);
	}
	else if(nCurSelItem == itMOBILE_HANDSET)
	{ 
		PreListView->ShowItems(m_Empty_box, nCurSelItem, 0); 
	}
	else if(nCurSelItem == itMOBILE_HANDSET_INBOX)
	{
		PreListView->ShowItems(m_HandsetSMS_Inbox, nCurSelItem, m_TreeItemBackup[itMOBILE_HANDSET_INBOX]);
	}
	else if(nCurSelItem == itMOBILE_HANDSET_OUTBOX)
	{
		PreListView->ShowItems(m_HandsetSMS_Outbox, nCurSelItem, m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX]);	
	}
	else if(nCurSelItem == itPC)
	{ 
		PreListView->ShowItems(m_Empty_box, nCurSelItem, 0); 
	}
	else if(nCurSelItem == itPC_INBOX)
	{
		PreListView->ShowItems(m_PC_Inbox, nCurSelItem, m_TreeItemBackup[itPC_INBOX]);
	}
	else if(nCurSelItem == itPC_OUTBOX)
	{
		PreListView->ShowItems(m_PC_Outbox, nCurSelItem, m_TreeItemBackup[itPC_OUTBOX]);
	}
	else if(nCurSelItem == itPC_SENDBK)
	{
		PreListView->ShowItems(m_PC_SendBK, nCurSelItem, m_TreeItemBackup[itPC_SENDBK]);
	}
	else if(nCurSelItem == itPC_DRAFT)
	{
		PreListView->ShowItems(m_PC_Draft, nCurSelItem, m_TreeItemBackup[itPC_DRAFT]);
	}
	else if(nCurSelItem == itPC_GARBAGE)
	{
		PreListView->ShowItems(m_PC_Garbage, nCurSelItem, m_TreeItemBackup[itPC_GARBAGE]);
	}
#ifdef ASUSM303
	else if(nCurSelItem == itMOBILE_HANDSET_OTHER1)
	{
		PreListView->ShowItems(m_HandsetSMS_Folder1, nCurSelItem, m_TreeItemBackup[itMOBILE_HANDSET_OTHER1]);
	}
	else if(nCurSelItem == itMOBILE_HANDSET_OTHER2)
	{
		PreListView->ShowItems(m_HandsetSMS_Folder2, nCurSelItem, m_TreeItemBackup[itMOBILE_HANDSET_OTHER2]);
	}
	else if(nCurSelItem == itMOBILE_HANDSET_OTHER3)
	{
		PreListView->ShowItems(m_HandsetSMS_Folder3, nCurSelItem, m_TreeItemBackup[itMOBILE_HANDSET_OTHER3]);
	}
	else if(nCurSelItem == itMOBILE_HANDSET_OTHER4)
	{
		PreListView->ShowItems(m_HandsetSMS_Folder4, nCurSelItem, m_TreeItemBackup[itMOBILE_HANDSET_OTHER4]);
	}
	/*else if(nCurSelItem == itMOBILE_HANDSET_OTHER5)
	{
		PreListView->ShowItems(m_HandsetSMS_Folder5, nCurSelItem, m_TreeItemBackup[itPC_GARBAGE]);
	}*/
#endif
	else
	{ 
		PreListView->ShowItems(m_Empty_box, nCurSelItem, 0); 
	}
	
	*pResult = 0;
}


void CLeftView::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView= (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	*pResult = 0;

}



void CLeftView::MoveItemsToPC(SMS_PARAM *SrcSMS, SMS_PARAM *DstSMS, int nPreSelItem, int nCurSelItem, int items)
{
	CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();

	POSITION pos = PreListView->GetListCtrl().GetFirstSelectedItemPosition();

	SMS_PARAM *TempSMS = new SMS_PARAM[SMS_ItemCount];
	SMS_PARAM *TempSMSDelete = new SMS_PARAM[SMS_ItemCount];
	memset(TempSMS, 0, sizeof(SMS_PARAM)*SMS_ItemCount);
	memcpy(TempSMS, SrcSMS, sizeof(SMS_PARAM)*SMS_ItemCount);
	memset(TempSMSDelete, 0, sizeof(SMS_PARAM)*SMS_ItemCount);
	memcpy(TempSMSDelete, SrcSMS, sizeof(SMS_PARAM)*SMS_ItemCount);
	bool bCheckindex = false;
	if(   nPreSelItem == itMOBILE_SIMCARD_INBOX
	   || nPreSelItem == itMOBILE_SIMCARD_OUTBOX
	   || nPreSelItem == itMOBILE_HANDSET_INBOX
	   || nPreSelItem == itMOBILE_HANDSET_OUTBOX)
	{
		bCheckindex = true;
		DeleteMobileSMS_from_trash(TempSMSDelete, nPreSelItem);
	}

	for(int i = 0; i < items; i++)
	{
		if(m_TreeItemBackup[nCurSelItem] >= 1000 && nCurSelItem !=itPC_GARBAGE)
		{
			TCHAR str1[MAX_PATH];
			al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
			afxMessageBox(str1);
			return;
		}

		int itemPosition = PreListView->GetListCtrl().GetNextSelectedItem(pos);
	
		if(bCheckindex == false || (bCheckindex == true  && m_Mobile_SMS[itemPosition].index == -1 ))
		{
			if(m_TreeItemBackup[nCurSelItem] < 1000)
			{
				memcpy(&DstSMS[m_TreeItemBackup[nCurSelItem]], &SrcSMS[itemPosition], sizeof(SMS_PARAM));
				m_TreeItemBackup[nCurSelItem]++;
			}
			memset(&TempSMS[itemPosition], 0, sizeof(SMS_PARAM));
			TempSMS[itemPosition].index = -1;
		}
	}


	int nTotal = m_TreeItemBackup[nPreSelItem];
	memset(SrcSMS, 0, sizeof(SMS_PARAM)*SMS_ItemCount);
	int nNextItem = 0;
	for(int k = 0; k < nTotal; k++)
	{
		if(TempSMS[k].index != -1)
		{
			memcpy(&SrcSMS[nNextItem], &TempSMS[k], sizeof(SMS_PARAM));
			nNextItem++;
		}
	}

//	m_TreeItemBackup[nPreSelItem] -= items;
	m_TreeItemBackup[nPreSelItem] = nNextItem;


/*	if(   nPreSelItem == itMOBILE_SIMCARD_INBOX
	   || nPreSelItem == itMOBILE_SIMCARD_OUTBOX
	   || nPreSelItem == itMOBILE_HANDSET_INBOX
	   || nPreSelItem == itMOBILE_HANDSET_OUTBOX)
	{
		DeleteMobileSMS_from_trash(TempSMSDelete, nPreSelItem);
	}
*/
	delete []TempSMS;
	delete []TempSMSDelete;
}

void CLeftView::CopyItemsToPC(SMS_PARAM *SrcSMS, SMS_PARAM *DstSMS, int nPreSelItem, int nCurSelItem, int items)
{
	CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();

	POSITION pos = PreListView->GetListCtrl().GetFirstSelectedItemPosition();

	
	for(int i = 0; i < items; i++)
	{
		if(m_TreeItemBackup[nCurSelItem] >= 1000)
		{
			TCHAR str1[MAX_PATH];
			al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
			afxMessageBox(str1);
			return;
		}

		int itemPosition = PreListView->GetListCtrl().GetNextSelectedItem(pos);
	
		memcpy(&DstSMS[m_TreeItemBackup[nCurSelItem]], &SrcSMS[itemPosition], sizeof(SMS_PARAM));
		m_TreeItemBackup[nCurSelItem]++;
	}

}

int CLeftView::HasHighByte(CString pstrString) 
{
	if(pstrString.GetLength()<=0)
		return false;
	unsigned char *pChar;
	int nLength  = 0;
#ifdef _UNICODE
	nLength = pstrString.GetLength();
	pChar = (unsigned char *)pstrString.GetBuffer(MAX_PATH);
#else
	USES_CONVERSION;
	char *str = pstrString.GetBuffer(MAX_PATH);
	WCHAR* wstr = A2W(str);
	nLength = wcslen(wstr);
	pChar = (unsigned char *)wstr;
#endif


	for(int i = 0; i < nLength*2; i++)
	{
		if(pChar[i] != 0 && pChar[i+1] != 0)
		{
			pstrString.ReleaseBuffer();
			return true;
		}
	}

	pstrString.ReleaseBuffer();
	return false;
}

int g_nPutCurSelItem;
int g_nMemtype;
#ifdef NOKIA
int CLeftView::MoveMobileOneSMS(int nNeed)
{
	// Put to Mobile(Copy) Begin
	SMS_Data_Strc smsSend;
	memset(&smsSend, 0, sizeof(SMS_Data_Strc));
	memcpy(&smsSend, &s_ActiveDlg->m_Mobile_SMS[s_ActiveDlg->m_arrayDeletePosition[g_nDeletePosition]], sizeof(SMS_PARAM));

	// How to Check Unicode ???
	if( s_ActiveDlg->HasHighByte(smsSend.TP_UD) )
		smsSend.TP_DCS = SMS_Coding_Unicode;
	else
		smsSend.TP_DCS = SMS_Coding_Default;

	// How to Check Mem type ??
	smsSend.memType	= g_nMemtype;

	if(g_nPutCurSelItem == itMOBILE_SIMCARD_INBOX)
		return true; //smsSend.whichFolder = 1;	// Inbox SIM
	else if(g_nPutCurSelItem == itMOBILE_SIMCARD_OUTBOX)
		smsSend.whichFolder = 2;	// Outbox SIM
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_INBOX)
		return true; //smsSend.whichFolder = 3;	// Inbox phone
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_OUTBOX)

		smsSend.whichFolder = 4;	// Outbox phone
	if(g_nPutCurSelItem == itMOBILE_SIMCARD_OUTBOX || g_nPutCurSelItem == itMOBILE_SIMCARD_INBOX)
	{
		if(s_ActiveDlg->m_TreeItemBackup[itMOBILE_SIMCARD_INBOX] 
			+ s_ActiveDlg->m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX] 
			>= s_ActiveDlg->m_sms_Tal_Num.SIMSMSTotalNum)
		{
			TCHAR str1[MAX_PATH];
			al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
			afxMessageBox(str1);
			return false;
		}
	}
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_OUTBOX || g_nPutCurSelItem == itMOBILE_HANDSET_INBOX)
	{
		if(s_ActiveDlg->m_TreeItemBackup[itMOBILE_HANDSET_INBOX] 
			+ s_ActiveDlg->m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX] 
			>= s_ActiveDlg->m_sms_Tal_Num.MESMSTotalNum)
		{
			TCHAR str1[MAX_PATH];
			al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
			afxMessageBox(str1);
			return false;
		}
	}
	if(_tcslen(smsSend.TPA) <=0)
	{
		TCHAR szMsg[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_ERR_NOPHONENO"),theApp.m_szRes,szMsg);
		afxMessageBox(szMsg);
		g_nDeletePosition--;
		return true;
	}

	int ret = ANWPutSMSData(smsSend.memType, smsSend.whichFolder, &smsSend);
	s_ActiveDlg->xErrorHandling(ret);
	if(ret != 1)
	{
		/*TCHAR str1[MAX_PATH];
		al_GetSettingString(_T("public"), "IDS_PUT2MOBILE_FAIL", theApp.m_szRes, str1);
		AfxMessageBox(str1);*/
		g_nDeletePosition--;
		if(ret == Anw_TRIAL_LIMITEDFUNCTION)
			return false ;
		return true;
	}
	else
	{
		memcpy(&s_ActiveDlg->m_Mobile_DstSMS[s_ActiveDlg->m_TreeItemBackup[g_nPutCurSelItem]], &s_ActiveDlg->m_Mobile_SMS[g_nPutCurSelItem], sizeof(SMS_PARAM));
		s_ActiveDlg->m_TreeItemBackup[g_nPutCurSelItem]++;	
	}
	// Put to Mobile(Copy) End

	g_nDeletePosition--;

	return true;
}
#endif

#ifdef ASUSM303
int CLeftView::MoveMobileOneSMS(int nNeed)
{
	// Put to Mobile(Copy) Begin
	SMS_Data_Strc smsSend;
	memset(&smsSend, 0, sizeof(SMS_Data_Strc));
	memcpy(&smsSend, &s_ActiveDlg->m_Mobile_SMS[s_ActiveDlg->m_arrayDeletePosition[g_nDeletePosition]], sizeof(SMS_PARAM));

	// How to Check Unicode ???
	if( s_ActiveDlg->HasHighByte(smsSend.TP_UD) )
		smsSend.TP_DCS = GSM_UCS2;
	else
		smsSend.TP_DCS = GSM_7BIT;

	// How to Check Mem type ??
	smsSend.memType	= g_nMemtype;

	if(g_nPutCurSelItem == itMOBILE_SIMCARD_INBOX)
		return true; //smsSend.whichFolder = 1;	// Inbox SIM
	else if(g_nPutCurSelItem == itMOBILE_SIMCARD_OUTBOX)
		smsSend.whichFolder = LLD_FOLDER_OUTBOX;	// Outbox SIM
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_INBOX)
		return true; //smsSend.whichFolder = 3;	// Inbox phone
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_OUTBOX)
		smsSend.whichFolder = LLD_FOLDER_OUTBOX;	// Outbox phone
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_OTHER1)
		smsSend.whichFolder = LLD_FOLDER_SMS_F1;	// Folder 1
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_OTHER2)
		smsSend.whichFolder = LLD_FOLDER_SMS_F2;	// Folder 2
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_OTHER3)
		smsSend.whichFolder = LLD_FOLDER_SMS_F3;	// Folder 3
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_OTHER4)
		smsSend.whichFolder = LLD_FOLDER_SMS_F4;	// Folder 4
	//else if(g_nPutCurSelItem == itMOBILE_HANDSET_OTHER5)
	//	smsSend.whichFolder = LLD_FOLDER_SMS_F5;	// Folder 5

	if( ( g_nPutCurSelItem == itMOBILE_SIMCARD_OUTBOX 
			  && s_ActiveDlg->m_TreeItemBackup[g_nPutCurSelItem] >= s_ActiveDlg->m_sms_Tal_Num.SIMSMSTotalNum)
		 || ( g_nPutCurSelItem == itMOBILE_HANDSET_OUTBOX
			  && s_ActiveDlg->m_TreeItemBackup[g_nPutCurSelItem] >= s_ActiveDlg->m_sms_Tal_Num.MESMSTotalNum) )

	{
		TCHAR str1[MAX_PATH];
		al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
		afxMessageBox(str1);
		return true;
	}

	int ret = ANWPutSMSData(smsSend.memType, smsSend.whichFolder, &smsSend);
	s_ActiveDlg->xErrorHandling(ret);
	if(ret != 1)
	{
		/*TCHAR str1[MAX_PATH];
		al_GetSettingString(_T("public"), "IDS_PUT2MOBILE_FAIL", theApp.m_szRes, str1);
		AfxMessageBox(str1);*/
		if(ret == Anw_TRIAL_LIMITEDFUNCTION)
			return false ;
		return true;
	}
	else
	{
		memcpy(&s_ActiveDlg->m_Mobile_DstSMS[s_ActiveDlg->m_TreeItemBackup[g_nPutCurSelItem]], &s_ActiveDlg->m_Mobile_SMS[g_nPutCurSelItem], sizeof(SMS_PARAM));
		s_ActiveDlg->m_TreeItemBackup[g_nPutCurSelItem]++;	
	}
	// Put to Mobile(Copy) End

	g_nDeletePosition--;

	return true;
}
#endif

void CLeftView::MoveItemsToMobile(SMS_PARAM *SrcSMS, SMS_PARAM *DstSMS, int nPreSelItem, int nCurSelItem, int items, short memtype)
{
	CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();

	POSITION pos = PreListView->GetListCtrl().GetFirstSelectedItemPosition();

	memset(m_Mobile_SMS, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	memcpy(m_Mobile_SMS, SrcSMS, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);

	m_Mobile_DstSMS = DstSMS;

	g_nPutCurSelItem = nCurSelItem;
	g_nMemtype = memtype;
	memset(&m_arrayDeletePosition, 0, SMS_MOBILE_ItemCount);
	for(int i = 0; i < items; i++)
	{
		m_arrayDeletePosition[i] = PreListView->GetListCtrl().GetNextSelectedItem(pos);
	}

	CProcessDlg dlg;
	if(items >= 0)
	{
		g_nDeletePosition = items-1;
		dlg.SetProgressInfo( CLeftView::MoveMobileOneSMS, items);
		dlg.SetLoopCount(items);
		m_bRuningDlg = true;
		dlg.DoModal();
		m_bRuningDlg = false;
	}


	int nTotal = m_TreeItemBackup[nPreSelItem];
	memset(SrcSMS, 0, sizeof(SMS_PARAM)*SMS_ItemCount);
	int nNextItem = 0;
	for(int k = 0; k < nTotal; k++)
	{
		if(m_Mobile_SMS[k].index != -1)
		{
			memcpy(&SrcSMS[nNextItem], &m_Mobile_SMS[k], sizeof(SMS_PARAM));
			nNextItem++;
		}
	}

	m_TreeItemBackup[nPreSelItem] -= items;

}

#ifdef NOKIA
int CLeftView::CopyMobileOneSMS(int nNeed)
{
	// Put to Mobile(Copy) Begin
	SMS_Data_Strc smsSend;
	memset(&smsSend, 0, sizeof(SMS_Data_Strc));
	memcpy(&smsSend, &s_ActiveDlg->m_Mobile_SMS[s_ActiveDlg->m_arrayDeletePosition[g_nDeletePosition]], sizeof(SMS_PARAM));

	// How to Check Unicode ???
	if( s_ActiveDlg->HasHighByte(smsSend.TP_UD) )
		smsSend.TP_DCS = SMS_Coding_Unicode;
	else
		smsSend.TP_DCS = SMS_Coding_Default;

	// How to Check Mem type ??
	smsSend.memType	= g_nMemtype;

	if(g_nPutCurSelItem == itMOBILE_SIMCARD_INBOX)
		return true; //smsSend.whichFolder = 1;	// Inbox SIM
	else if(g_nPutCurSelItem == itMOBILE_SIMCARD_OUTBOX)
		smsSend.whichFolder = 2;	// Outbox SIM
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_INBOX)
		return true; //smsSend.whichFolder = 3;	// Inbox phone
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_OUTBOX)
		smsSend.whichFolder = 4;	// Outbox phone

	if(g_nPutCurSelItem == itMOBILE_SIMCARD_OUTBOX || g_nPutCurSelItem == itMOBILE_SIMCARD_INBOX)
	{
		if(s_ActiveDlg->m_TreeItemBackup[itMOBILE_SIMCARD_INBOX] 
			+ s_ActiveDlg->m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX] 
			>= s_ActiveDlg->m_sms_Tal_Num.SIMSMSTotalNum)
		{
			TCHAR str1[MAX_PATH];
			al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
			afxMessageBox(str1);
			return false;
		}
	}
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_OUTBOX || g_nPutCurSelItem == itMOBILE_HANDSET_INBOX)
	{
		if(s_ActiveDlg->m_TreeItemBackup[itMOBILE_HANDSET_INBOX] 
			+ s_ActiveDlg->m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX] 
			>= s_ActiveDlg->m_sms_Tal_Num.MESMSTotalNum)
		{
			TCHAR str1[MAX_PATH];
			al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
			afxMessageBox(str1);
			return false;
		}
	}
	if(_tcslen(smsSend.TPA) <=0)
	{
		TCHAR szMsg[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_ERR_NOPHONENO"),theApp.m_szRes,szMsg);
		afxMessageBox(szMsg);
		g_nDeletePosition--;
		return true;
	}
	int ret = ANWPutSMSData(smsSend.memType, smsSend.whichFolder, &smsSend);
	s_ActiveDlg->xErrorHandling(ret);
	if(ret != 1)
	{
		/*TCHAR str1[MAX_PATH];
		al_GetSettingString(_T("public"), "IDS_PUT2MOBILE_FAIL", theApp.m_szRes, str1);
		AfxMessageBox(str1);*/
		g_nDeletePosition--;
		if(ret == Anw_TRIAL_LIMITEDFUNCTION)
			return false ;
		return true;
	}
	else
	{
		memcpy(&s_ActiveDlg->m_Mobile_DstSMS[s_ActiveDlg->m_TreeItemBackup[g_nPutCurSelItem]], &smsSend, sizeof(SMS_PARAM));
		s_ActiveDlg->m_TreeItemBackup[g_nPutCurSelItem]++;
	}
	// Put to Mobile(Copy) End

	g_nDeletePosition--;

	return true;
}
#endif

#ifdef ASUSM303
int CLeftView::CopyMobileOneSMS(int nNeed)
{
	// Put to Mobile(Copy) Begin
	SMS_Data_Strc smsSend;
	memset(&smsSend, 0, sizeof(SMS_Data_Strc));
	memcpy(&smsSend, &s_ActiveDlg->m_Mobile_SMS[s_ActiveDlg->m_arrayDeletePosition[g_nDeletePosition]], sizeof(SMS_PARAM));

	// How to Check Unicode ???
	if( s_ActiveDlg->HasHighByte(smsSend.TP_UD) )
		smsSend.TP_DCS = GSM_UCS2;
	else
		smsSend.TP_DCS = GSM_7BIT;

	// How to Check Mem type ??
	smsSend.memType	= g_nMemtype;

	if(g_nPutCurSelItem == itMOBILE_SIMCARD_INBOX)
		return true; //smsSend.whichFolder = 1;	// Inbox SIM
	else if(g_nPutCurSelItem == itMOBILE_SIMCARD_OUTBOX)
		smsSend.whichFolder = LLD_FOLDER_OUTBOX;	// Outbox SIM
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_INBOX)
		return true; //smsSend.whichFolder = 3;	// Inbox phone
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_OUTBOX)
		smsSend.whichFolder = LLD_FOLDER_OUTBOX;	// Outbox phone
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_OTHER1)
		smsSend.whichFolder = LLD_FOLDER_SMS_F1;	// Folder 1
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_OTHER2)
		smsSend.whichFolder = LLD_FOLDER_SMS_F2;	// Folder 2
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_OTHER3)
		smsSend.whichFolder = LLD_FOLDER_SMS_F3;	// Folder 3
	else if(g_nPutCurSelItem == itMOBILE_HANDSET_OTHER4)
		smsSend.whichFolder = LLD_FOLDER_SMS_F4;	// Folder 4
	//else if(g_nPutCurSelItem == itMOBILE_HANDSET_OTHER5)
	//	smsSend.whichFolder = LLD_FOLDER_SMS_F5;	// Folder 5

	if( ( g_nPutCurSelItem == itMOBILE_SIMCARD_OUTBOX 
			  && s_ActiveDlg->m_TreeItemBackup[g_nPutCurSelItem] >= s_ActiveDlg->m_sms_Tal_Num.SIMSMSTotalNum)
		 || ( g_nPutCurSelItem == itMOBILE_HANDSET_OUTBOX
			  && s_ActiveDlg->m_TreeItemBackup[g_nPutCurSelItem] >= s_ActiveDlg->m_sms_Tal_Num.MESMSTotalNum) )

	{
		TCHAR str1[MAX_PATH];
		al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
		afxMessageBox(str1);
		return true;
	}

	int ret = ANWPutSMSData(smsSend.memType, smsSend.whichFolder, &smsSend);
	s_ActiveDlg->xErrorHandling(ret);
	if(ret != 1)
	{
		/*TCHAR str1[MAX_PATH];
		al_GetSettingString(_T("public"), "IDS_PUT2MOBILE_FAIL", theApp.m_szRes, str1);
		AfxMessageBox(str1);*/
		if(ret == Anw_TRIAL_LIMITEDFUNCTION)
			return false ;
		return true;
	}
	else
	{
		memcpy(&s_ActiveDlg->m_Mobile_DstSMS[s_ActiveDlg->m_TreeItemBackup[g_nPutCurSelItem]], &smsSend, sizeof(SMS_PARAM));
		s_ActiveDlg->m_TreeItemBackup[g_nPutCurSelItem]++;
	}
	// Put to Mobile(Copy) End

	g_nDeletePosition--;

	return true;
}
#endif

void CLeftView::CopyItemsToMobile(SMS_PARAM *SrcSMS, SMS_PARAM *DstSMS, int nPreSelItem, int nCurSelItem, int items, short memtype)
{
	if(nCurSelItem == itMOBILE_SIMCARD_OUTBOX || nCurSelItem == itMOBILE_SIMCARD_INBOX)
	{
		if(s_ActiveDlg->m_TreeItemBackup[itMOBILE_SIMCARD_INBOX] 
			+ s_ActiveDlg->m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX] 
			>= s_ActiveDlg->m_sms_Tal_Num.SIMSMSTotalNum)
		{
			TCHAR str1[MAX_PATH];
			al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
			afxMessageBox(str1);
			return;
		}
	}
	else if(nCurSelItem == itMOBILE_HANDSET_OUTBOX || nCurSelItem == itMOBILE_HANDSET_INBOX)
	{
		if(s_ActiveDlg->m_TreeItemBackup[itMOBILE_HANDSET_INBOX] 
			+ s_ActiveDlg->m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX] 
			>= s_ActiveDlg->m_sms_Tal_Num.MESMSTotalNum)
		{
			TCHAR str1[MAX_PATH];
			al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
			afxMessageBox(str1);
			return;
		}
	}

	CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();

	POSITION pos = PreListView->GetListCtrl().GetFirstSelectedItemPosition();

	memset(m_Mobile_SMS, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	memcpy(m_Mobile_SMS, SrcSMS, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);

	m_Mobile_DstSMS = DstSMS;

	g_nPutCurSelItem = nCurSelItem;
	g_nMemtype = memtype;
	memset(&m_arrayDeletePosition, 0, SMS_MOBILE_ItemCount);
	for(int i = 0; i < items; i++)
	{
		m_arrayDeletePosition[i] = PreListView->GetListCtrl().GetNextSelectedItem(pos);
	}

	CProcessDlg dlg;
	if(items >= 0)
	{
		g_nDeletePosition = items-1;
		dlg.SetProgressInfo( CLeftView::CopyMobileOneSMS, items);
		dlg.SetLoopCount(items);
		m_bRuningDlg = true;
		dlg.DoModal();
		m_bRuningDlg = false;
	}
}


void CLeftView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default	/*if (m_hItemDrag != NULL)
}

void CLeftView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CTreeView::OnMouseMove(nFlags, point);
}

void CLeftView::UpdateOneTreeItem(HTREEITEM hCurSelItem, int nCurSelItem)
{
	int bNeedChange = true;
	bool AsusM303 = false;


	TCHAR szTmp[MAX_PATH];
	TCHAR strSetString[MAX_PATH];

	if(nCurSelItem == itMOBILE_SIMCARD_INBOX)
		al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp.m_szRes, szTmp);
	else if(nCurSelItem == itMOBILE_SIMCARD_OUTBOX)
		al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp.m_szRes, szTmp);
	else if(nCurSelItem == itMOBILE_HANDSET_INBOX)
		al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp.m_szRes, szTmp);
	else if(nCurSelItem == itMOBILE_HANDSET_OUTBOX)
		al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp.m_szRes, szTmp);
	else if(nCurSelItem == itPC_INBOX)
		al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp.m_szRes, szTmp);
	else if(nCurSelItem == itPC_OUTBOX)
		al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp.m_szRes, szTmp);
	else if(nCurSelItem == itPC_SENDBK)
		al_GetSettingString(_T("public"), _T("IDS_SENDBOX"), theApp.m_szRes, szTmp);
	else if(nCurSelItem == itPC_DRAFT)
		al_GetSettingString(_T("public"), _T("IDS_DRAFTBOX"), theApp.m_szRes, szTmp);
	else if(nCurSelItem == itPC_GARBAGE)
		al_GetSettingString(_T("public"), _T("IDS_TRASHBOX"), theApp.m_szRes, szTmp);
#ifdef ASUSM303
	else if(nCurSelItem == itMOBILE_HANDSET_OTHER1)
	{
		xprintf(mfolders.Folder[0].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER1]);
		AsusM303 = true;
	}
	else if(nCurSelItem == itMOBILE_HANDSET_OTHER2)
	{
		xprintf(mfolders.Folder[1].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER2]);
		AsusM303 = true;
	}
	else if(nCurSelItem == itMOBILE_HANDSET_OTHER3)
	{
		xprintf(mfolders.Folder[2].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER3]);
		AsusM303 = true;
	}
	else if(nCurSelItem == itMOBILE_HANDSET_OTHER3)
	{
		xprintf(mfolders.Folder[3].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER4]);
		AsusM303 = true;
	}
#endif
	else
		bNeedChange = false;

	

	if(bNeedChange)
	{
		if(AsusM303 == false)
		{
			xprintf(szTmp, strSetString, m_TreeItemBackup[nCurSelItem]);
			GetTreeCtrl().SetItemText(hCurSelItem, strSetString);
		}
		else
			GetTreeCtrl().SetItemText(hCurSelItem, strSetString);

		/*if(AsusM303 == false)
			xprintf(szTmp, strSetString, m_TreeItemBackup[nCurSelItem]);
		GetTreeCtrl().SetItemText(hCurSelItem, strSetString);*/
	}
}

void CLeftView::UpdateAllTreeItem(int nPreSelItem, int nCurSelItem)
{
	CSMSUtility *theApp = (CSMSUtility*)afxGetApp();
	CTreeCtrl &cTree = GetTreeCtrl();

	bool bNeedChange = true;
	bool AsusM303 = false;

	TCHAR szTmp[MAX_PATH];
	TCHAR strSetString[MAX_PATH];

	memset(&szTmp, 0, MAX_PATH);
	memset(&strSetString, 0, MAX_PATH);
	if(nPreSelItem == itMOBILE_SIMCARD_INBOX)
		al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp->m_szRes, szTmp);
	else if(nPreSelItem == itMOBILE_SIMCARD_OUTBOX)
		al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp->m_szRes, szTmp);
	else if(nPreSelItem == itMOBILE_HANDSET_INBOX)
		al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp->m_szRes, szTmp);
	else if(nPreSelItem == itMOBILE_HANDSET_OUTBOX)
		al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp->m_szRes, szTmp);
	else if(nPreSelItem == itPC_INBOX)
		al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp->m_szRes, szTmp);
	else if(nPreSelItem == itPC_OUTBOX)
		al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp->m_szRes, szTmp);
	else if(nPreSelItem == itPC_SENDBK)
		al_GetSettingString(_T("public"), _T("IDS_SENDBOX"), theApp->m_szRes, szTmp);
	else if(nPreSelItem == itPC_DRAFT)
		al_GetSettingString(_T("public"), _T("IDS_DRAFTBOX"), theApp->m_szRes, szTmp);
	else if(nPreSelItem == itPC_GARBAGE)
		al_GetSettingString(_T("public"), _T("IDS_TRASHBOX"), theApp->m_szRes, szTmp);
#ifdef ASUSM303
	else if(nPreSelItem == itMOBILE_HANDSET_OTHER1)
	{
		xprintf(mfolders.Folder[0].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER1]);
		AsusM303 = true;
	}
	else if(nPreSelItem == itMOBILE_HANDSET_OTHER2)
	{
		xprintf(mfolders.Folder[1].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER2]);
		AsusM303 = true;
	}
	else if(nPreSelItem == itMOBILE_HANDSET_OTHER3)
	{
		xprintf(mfolders.Folder[2].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER3]);
		AsusM303 = true;
	}
	else if(nPreSelItem == itMOBILE_HANDSET_OTHER4)
	{
		xprintf(mfolders.Folder[3].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER4]);
		AsusM303 = true;
	}
	/*else if(nPreSelItem == itMOBILE_HANDSET_OTHER5)
	{
		xprintf(mfolders.Folder[4].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER5]);
		AsusM303 = true;
	}*/
#endif
	else
		bNeedChange = false;

	if(bNeedChange)
	{
		if(AsusM303 == false)
		{
			xprintf(szTmp, strSetString, m_TreeItemBackup[nPreSelItem]);
			cTree.SetItemText(theApp->cTreeItemDrag, strSetString);
		}
		else
			cTree.SetItemText(theApp->cTreeItemDrag, strSetString);
	}


	memset(&szTmp, 0, MAX_PATH);
	memset(&strSetString, 0, MAX_PATH);
	bNeedChange = true;
	AsusM303 = false;
	if(nCurSelItem == itMOBILE_SIMCARD_INBOX)
		al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp->m_szRes, szTmp);
	else if(nCurSelItem == itMOBILE_SIMCARD_OUTBOX)
		al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp->m_szRes, szTmp);
	else if(nCurSelItem == itMOBILE_HANDSET_INBOX)
		al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp->m_szRes, szTmp);
	else if(nCurSelItem == itMOBILE_HANDSET_OUTBOX)
		al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp->m_szRes, szTmp);
	else if(nCurSelItem == itPC_INBOX)
		al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp->m_szRes, szTmp);
	else if(nCurSelItem == itPC_OUTBOX)
		al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp->m_szRes, szTmp);
	else if(nCurSelItem == itPC_SENDBK)
		al_GetSettingString(_T("public"), _T("IDS_SENDBOX"), theApp->m_szRes, szTmp);
	else if(nCurSelItem == itPC_DRAFT)
		al_GetSettingString(_T("public"), _T("IDS_DRAFTBOX"), theApp->m_szRes, szTmp);
	else if(nCurSelItem == itPC_GARBAGE)
		al_GetSettingString(_T("public"), _T("IDS_TRASHBOX"), theApp->m_szRes, szTmp);
#ifdef ASUSM303
	else if(nCurSelItem == itMOBILE_HANDSET_OTHER1)
	{
		xprintf(mfolders.Folder[0].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER1]);
		AsusM303 = true;
	}
	else if(nCurSelItem == itMOBILE_HANDSET_OTHER2)
	{
		xprintf(mfolders.Folder[1].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER2]);
		AsusM303 = true;
	}
	else if(nCurSelItem == itMOBILE_HANDSET_OTHER3)
	{
		xprintf(mfolders.Folder[2].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER3]);
		AsusM303 = true;
	}
	else if(nCurSelItem == itMOBILE_HANDSET_OTHER4)
	{
		xprintf(mfolders.Folder[3].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER4]);
		AsusM303 = true;
	}
	/*else if(nCurSelItem == itMOBILE_HANDSET_OTHER5)
	{
		xprintf(mfolders.Folder[4].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER5]);
		AsusM303 = true;
	}*/
#endif
	else
		bNeedChange = false;

	if(bNeedChange)
	{
		if(AsusM303 == false)
		{
			xprintf(szTmp, strSetString, m_TreeItemBackup[nCurSelItem]);
			cTree.SetItemText(theApp->cTreeItemDrop, strSetString);
		}
		else
			cTree.SetItemText(theApp->cTreeItemDrop, strSetString);
	}


	
}

void CLeftView::UpdateAllTreeItemFirst()
{
	//CSMSUtility *theApp = (CSMSUtility*)afxGetApp();
	CTreeCtrl &cTree = GetTreeCtrl();
	TCHAR szTmp[MAX_PATH];
	TCHAR strSetString[MAX_PATH];
	
	al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, m_TreeItemBackup[itMOBILE_SIMCARD_INBOX]);
	cTree.SetItemText(gMobileSIMCardInboxItem, strSetString);
	
	al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX]);
	cTree.SetItemText(gMobileSIMCardOutboxItem, strSetString);

	al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_INBOX]); 
	cTree.SetItemText(gMobileHandsetInboxItem, strSetString);
		
	al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX]);
	cTree.SetItemText(gMobileHandsetOutboxItem, strSetString);

	al_GetSettingString(_T("public"), _T("IDS_INBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, m_TreeItemBackup[itPC_INBOX]); 
	cTree.SetItemText(gPCInboxItem, strSetString);
	
	al_GetSettingString(_T("public"), _T("IDS_OUTBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, m_TreeItemBackup[itPC_OUTBOX]);
	cTree.SetItemText(gPCOutboxItem, strSetString);

	al_GetSettingString(_T("public"), _T("IDS_SENDBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, m_TreeItemBackup[itPC_SENDBK]);
	cTree.SetItemText(gPCSendBKItem, strSetString);

	al_GetSettingString(_T("public"), _T("IDS_DRAFTBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, m_TreeItemBackup[itPC_DRAFT]);
	cTree.SetItemText(gPCDraftItem, strSetString);

	al_GetSettingString(_T("public"), _T("IDS_TRASHBOX"), theApp.m_szRes, szTmp);
	xprintf(szTmp, strSetString, m_TreeItemBackup[itPC_GARBAGE]);
	cTree.SetItemText(gPCGarbageItem, strSetString);

#ifdef ASUSM303
	xprintf(mfolders.Folder[0].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER1]);
	cTree.SetItemText(gMobileHandsetFolder1Item, strSetString);
	xprintf(mfolders.Folder[1].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER2]);
	cTree.SetItemText(gMobileHandsetFolder2Item, strSetString);
	xprintf(mfolders.Folder[2].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER3]);
	cTree.SetItemText(gMobileHandsetFolder3Item, strSetString);
	xprintf(mfolders.Folder[3].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER4]);
	cTree.SetItemText(gMobileHandsetFolder4Item, strSetString);
	//xprintf(mfolders.Folder[4].Name, strSetString, m_TreeItemBackup[itMOBILE_HANDSET_OTHER5]);
	//cTree.SetItemText(gMobileHandsetFolder5Item, strSetString);
#endif

}

DROPEFFECT CLeftView::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	// TODO: Add your specialized code here and/or call the base class
	if((dwKeyState & MK_CONTROL) == MK_CONTROL)
        return DROPEFFECT_NONE;
    else
        return DROPEFFECT_MOVE; 



	/*if((dwKeyState & MK_CONTROL) == MK_CONTROL)
        return DROPEFFECT_MOVE;
    else
        return DROPEFFECT_MOVE; */

}

void CLeftView::OnDragLeave() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CTreeView::OnDragLeave();
}

DROPEFFECT CLeftView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	// TODO: Add your specialized code here and/or call the base class
/*	if((dwKeyState & MK_CONTROL) == MK_CONTROL)
	{	
		if(dwKeyState == 9)
		{
			CSMSUtility *theApp = (CSMSUtility*)afxGetApp();
			theApp->cTreeItemDrop = GetTreeCtrl().HitTest(point);;
			if(theApp->cTreeItemDrag == theApp->cTreeItemDrop)
			return DROPEFFECT_NONE;
		
			if( theApp->cTreeItemDrop != gMobileSIMCardOutboxItem
				&& theApp->cTreeItemDrop != gMobileHandsetOutboxItem 
	#ifdef ASUSM303
				&& theApp->cTreeItemDrop != gMobileHandsetFolder1Item
				&& theApp->cTreeItemDrop != gMobileHandsetFolder2Item
				&& theApp->cTreeItemDrop != gMobileHandsetFolder3Item
				&& theApp->cTreeItemDrop != gMobileHandsetFolder4Item
				//&& theApp->cTreeItemDrop != gMobileHandsetFolder5Item
	#endif
				&& theApp->cTreeItemDrop != gPCInboxItem 
				&& theApp->cTreeItemDrop != gPCOutboxItem 
				&& theApp->cTreeItemDrop != gPCSendBKItem 
				&& theApp->cTreeItemDrop != gPCDraftItem 
				&& theApp->cTreeItemDrop != gPCGarbageItem )
			{
					
				return DROPEFFECT_NONE;
			}
			else
			{
				if(theApp->nIsConnected == false &&
					(theApp->cTreeItemDrop == gMobileSIMCardOutboxItem
					|| theApp->cTreeItemDrop == gMobileHandsetOutboxItem
	#ifdef ASUSM303
					|| theApp->cTreeItemDrop == gMobileHandsetFolder1Item
					|| theApp->cTreeItemDrop == gMobileHandsetFolder2Item
					|| theApp->cTreeItemDrop == gMobileHandsetFolder3Item
					|| theApp->cTreeItemDrop == gMobileHandsetFolder4Item
					//|| theApp->cTreeItemDrop == gMobileHandsetFolder5Item
	#endif
					) )
				{
					return DROPEFFECT_NONE;
				}
				else
				{
					if(theApp->cTreeItemDrop == gMobileSIMCardOutboxItem && (m_sms_Tal_Num.SIMSMSTotalNum == 0 ||
				//	((m_TreeItemBackup[itMOBILE_SIMCARD_INBOX] +m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX])>= m_sms_Tal_Num.SIMSMSTotalNum)) ||
					m_sms_Tal_Num.bSIMSMSReadOnly))
						return DROPEFFECT_NONE;
					else if(theApp->cTreeItemDrop == gMobileHandsetOutboxItem && (m_sms_Tal_Num.MESMSTotalNum == 0 ||
					//	((m_TreeItemBackup[itMOBILE_HANDSET_INBOX] + m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX])>=m_sms_Tal_Num.MESMSTotalNum)) ||
						m_sms_Tal_Num.bMESMSReadOnly))
						return DROPEFFECT_NONE;
					else
						return DROPEFFECT_MOVE;
				}
			}
				
			
			return DROPEFFECT_MOVE;  // move source
		}
		else
			return DROPEFFECT_NONE; 
	}
	//else if(!theApp.nIsConnected)
	//	return DROPEFFECT_NONE;  
    else*/
	{
		//Get droped tree item handle
		//If dragging is active:
		CSMSUtility *theApp = (CSMSUtility*)afxGetApp();
		theApp->cTreeItemDrop = GetTreeCtrl().HitTest(point);;
		if(theApp->cTreeItemDrag == theApp->cTreeItemDrop)
			return DROPEFFECT_NONE;

		if( theApp->cTreeItemDrop != gMobileSIMCardOutboxItem
			/*&& theApp->cTreeItemDrop != gMobileHandsetOutboxItem *///080601liboliu
#ifdef ASUSM303
			&& theApp->cTreeItemDrop != gMobileHandsetFolder1Item
			&& theApp->cTreeItemDrop != gMobileHandsetFolder2Item
			&& theApp->cTreeItemDrop != gMobileHandsetFolder3Item
			&& theApp->cTreeItemDrop != gMobileHandsetFolder4Item
			//&& theApp->cTreeItemDrop != gMobileHandsetFolder5Item
#endif
			&& theApp->cTreeItemDrop != gPCInboxItem 
			&& theApp->cTreeItemDrop != gPCOutboxItem 
			&& theApp->cTreeItemDrop != gPCSendBKItem 
			&& theApp->cTreeItemDrop != gPCDraftItem 
			&& theApp->cTreeItemDrop != gPCGarbageItem )
		{
				
			return DROPEFFECT_NONE;
		}
		else
		{
			if(theApp->nIsConnected == false &&
				(theApp->cTreeItemDrop == gMobileSIMCardOutboxItem
			    /*|| theApp->cTreeItemDrop == gMobileHandsetOutboxItem*///080601liboliu
#ifdef ASUSM303
				|| theApp->cTreeItemDrop == gMobileHandsetFolder1Item
				|| theApp->cTreeItemDrop == gMobileHandsetFolder2Item
				|| theApp->cTreeItemDrop == gMobileHandsetFolder3Item
				|| theApp->cTreeItemDrop == gMobileHandsetFolder4Item
				//|| theApp->cTreeItemDrop == gMobileHandsetFolder5Item
#endif
				) )
			{
				return DROPEFFECT_NONE;
			}
			else
				{
					int nCompany	= _ttoi(m_strCompany);
					if(theApp->cTreeItemDrop == NULL) return DROPEFFECT_NONE;
	/*				if(theApp->cTreeItemDrop == gMobileSIMCardOutboxItem && m_sms_Tal_Num.SIMSMSTotalNum == 0)
						return DROPEFFECT_NONE;
					else if(theApp->cTreeItemDrop == gMobileHandsetOutboxItem && m_sms_Tal_Num.MESMSTotalNum == 0)
						return DROPEFFECT_NONE;
					else
						return DROPEFFECT_MOVE;*/
					if(theApp->cTreeItemDrop == gMobileSIMCardOutboxItem && (m_sms_Tal_Num.SIMSMSTotalNum == 0 ||
				/*	((m_TreeItemBackup[itMOBILE_SIMCARD_INBOX] +m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX])>= m_sms_Tal_Num.SIMSMSTotalNum)) ||*/
					m_sms_Tal_Num.bSIMSMSReadOnly || (nCompany == 9 && _tcsicmp(m_strPhone,_T("C3300")) == 0) ||
					(nCompany == 9 && _tcsicmp(m_strPhone,_T("KG800")) == 0) ||
					(nCompany == 5 && _tcsicmp(m_strPhone,_T("V547")) == 0)))
						return DROPEFFECT_NONE;
					else if(theApp->cTreeItemDrop == gMobileHandsetOutboxItem && (m_sms_Tal_Num.MESMSTotalNum == 0 ||
					/*	((m_TreeItemBackup[itMOBILE_HANDSET_INBOX] + m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX])>=m_sms_Tal_Num.MESMSTotalNum)) ||*/
						m_sms_Tal_Num.bMESMSReadOnly || (nCompany == 9 && _tcsicmp(m_strPhone,_T("C3300")) == 0)|| 
						(nCompany == 9 && _tcsicmp(m_strPhone,_T("KG800")) == 0)|| 
						(nCompany == 5 && _tcsicmp(m_strPhone,_T("V547")) == 0)))
						return DROPEFFECT_NONE;
					else
						return DROPEFFECT_MOVE;
				}

		}
			
		return DROPEFFECT_MOVE;  // move source

	}
}

BOOL CLeftView::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
	// TODO: Add your specialized code here and/or call the base class
	if((dropEffect & DROPEFFECT_MOVE) != DROPEFFECT_MOVE)
        return FALSE;
	
	//Get droped tree item handle
	CSMSUtility *theApp = (CSMSUtility*)afxGetApp();
	HTREEITEM   hitem = GetTreeCtrl().HitTest(point);;
	//check droped item
	if(hitem)
	{
		//Get the droped item data
		int nPreSelItem = GetSelectItem(theApp->cTreeItemDrag);
		theApp->cTreeItemDrop = hitem;
		int nCurSelItem = GetSelectItem(theApp->cTreeItemDrop);

		if(nPreSelItem == nCurSelItem)
			return false;

		if(nCurSelItem == itMOBILE || nCurSelItem == itPC 
			|| nCurSelItem == itMOBILE_SIMCARD || nCurSelItem == itMOBILE_HANDSET)
		{
			return false;	// Need not do anything
		}
		else 
		{
			// Move item and change
			CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();
			int nMoveItemCounts = PreListView->GetListCtrl().GetSelectedCount();

			// 1. Move item
			int nItems;
			SMS_PARAM *SrcSMS = GetTreeSMSInfo(theApp->cTreeItemDrag, nItems);
			SMS_PARAM *DstSMS = GetTreeSMSInfo(theApp->cTreeItemDrop, nItems);

			if( nCurSelItem == itPC_GARBAGE )
			{
				MoveItemsToPC(SrcSMS, DstSMS, nPreSelItem, nCurSelItem, nMoveItemCounts);
			}
			else if( nCurSelItem == itPC_INBOX		// PC Item
				|| nCurSelItem == itPC_OUTBOX
				|| nCurSelItem == itPC_SENDBK 
				|| nCurSelItem == itPC_DRAFT 
				/*|| nCurSelItem == itPC_GARBAGE*/)
			{
				if(m_CtrlKeyDown == true)
					MoveItemsToPC(SrcSMS, DstSMS, nPreSelItem, nCurSelItem, nMoveItemCounts);
				else
					CopyItemsToPC(SrcSMS, DstSMS, nPreSelItem, nCurSelItem, nMoveItemCounts);
			}
			else // Mobile Item
			{
				if(theApp->nIsConnected != true)
					return false;

				if(m_CtrlKeyDown == true)
				{
					if( nCurSelItem == itMOBILE_SIMCARD_INBOX || nCurSelItem == itMOBILE_SIMCARD_OUTBOX) // SIM CARD
						MoveItemsToMobile(SrcSMS, DstSMS, nPreSelItem, nCurSelItem, nMoveItemCounts, MEM_SM);
					else // HandSet
						MoveItemsToMobile(SrcSMS, DstSMS, nPreSelItem, nCurSelItem, nMoveItemCounts, MEM_ME);
				}
				else
				{
#ifdef ASUSM303
					if( nCurSelItem == itMOBILE_SIMCARD_INBOX || nCurSelItem == itMOBILE_SIMCARD_OUTBOX) // SIM CARD
						CopyItemsToMobile(SrcSMS, DstSMS, nPreSelItem, nCurSelItem, nMoveItemCounts, LLD_MEMORY_TYPE_SIM);
					else // HandSet
						CopyItemsToMobile(SrcSMS, DstSMS, nPreSelItem, nCurSelItem, nMoveItemCounts, LLD_MEMORY_TYPE_HANDSET);

#else
					if( nCurSelItem == itMOBILE_SIMCARD_INBOX || nCurSelItem == itMOBILE_SIMCARD_OUTBOX) // SIM CARD
						CopyItemsToMobile(SrcSMS, DstSMS, nPreSelItem, nCurSelItem, nMoveItemCounts, MEM_SM);
					else // HandSet
						CopyItemsToMobile(SrcSMS, DstSMS, nPreSelItem, nCurSelItem, nMoveItemCounts, MEM_ME);
#endif
				}

			}

			// 2. Update data
			UpdateAllTreeItem(nPreSelItem, nCurSelItem);
			
			PreListView->ShowItems(SrcSMS, nCurSelItem, m_TreeItemBackup[nPreSelItem]);
		}
		
	}

	return FALSE;
}

bool CLeftView::InitialLibrary(void)
{
	//m_hMobileDll = LoadLibrary("AnwMobile.dll");
	// DLL CONTRUCT
	bHasConnected = false;
	if(afxContextIsDLL)
	{
	  m_hMobileDll = ((CSMSUtility*)afxGetApp())->m_hInstAnwMobile;
	}
	else
	{
  	  if(_tcslen(m_strMobileDLL) > 0)
		 m_hMobileDll = LoadLibrary(m_strMobileDLL);
	  else
#ifdef _UNICODE
		m_hMobileDll = LoadLibrary(_T("AnwMobileU.dll"));
#else
		m_hMobileDll = LoadLibrary(_T("AnwMobile.dll"));
#endif
    }

	if (!m_hMobileDll) 
	{
		MessageBox(_T("Load AnwMobile.dll error"),_T("Error"),MB_ICONERROR|MB_OK);
		return false;
	}

	ANWOpenSMS = (anwOpenSMS)::GetProcAddress(m_hMobileDll,"OpenSMS");
	ANWGetMobileSMSInfo = (anwGetMobileSMSInfo)::GetProcAddress(m_hMobileDll,"GetMobileSMSInfo");
	ANWGetMobileAllSMS = (anwGetMobileAllSMS)::GetProcAddress(m_hMobileDll,"GetMobileAllSMS");
	ANWGetSMSStartData = (anwGetSMSStartData)::GetProcAddress(m_hMobileDll,"GetSMSStartData");
	ANWGetSMSOneData = (anwGetSMSOneData)::GetProcAddress(m_hMobileDll,"GetSMSOneData");
	ANWGetSMSNextData = (anwGetSMSNextData)::GetProcAddress(m_hMobileDll,"GetSMSNextData");
	ANWSendSMSData = (anwSendSMSData)::GetProcAddress(m_hMobileDll,"SendSMSData");
	ANWPutSMSData = (anwPutSMSData)::GetProcAddress(m_hMobileDll,"PutSMSData");
	ANWDeleteSMSData = (anwDeleteSMSData)::GetProcAddress(m_hMobileDll,"DeleteSMSData");
	ANWDeleteAllSMSData = (anwDeleteAllSMSData)::GetProcAddress(m_hMobileDll,"DeleteAllSMSData");
	ANWCloseSMS = (anwCloseSMS)::GetProcAddress(m_hMobileDll,"CloseSMS");
#ifdef ASUSM303
	ANWGetMobileSMSFolders = (anwGetMobileSMSFolders)::GetProcAddress(m_hMobileDll,"GetMobileSMSFolders");
	ANWInitSMS = (anwInitSMS)::GetProcAddress(m_hMobileDll,"InitSMS");
#endif
	//ANWGetIMEI = (GetIMEIProc)::GetProcAddress(m_hMobileDll,"GetIMEI");

#ifdef ASUSM303
	if (ANWInitSMS == NULL)
   {
      MessageBox(_T("Can't Get ANWInitSMS()"),_T("Error"),MB_ICONERROR|MB_OK);
	  FreeLibrary(m_hMobileDll);
	  return false; 
   }
#endif
	
   if (ANWOpenSMS == NULL)
   {
      MessageBox(_T("Can't Get ANWOpenSMS()"),_T("Error"),MB_ICONERROR|MB_OK);
	  FreeLibrary(m_hMobileDll);
	  return false; 
   }

   if (ANWGetMobileSMSInfo == NULL)
   {
      MessageBox(_T("Can't Get ANWGetMobileSMSInfo()"),_T("Error"),MB_ICONERROR|MB_OK);
	  FreeLibrary(m_hMobileDll);
	  return false; 
   }

   if (ANWGetSMSStartData == NULL)
   {
      MessageBox(_T("Can't Get ANWGetSMSStartData()"),_T("Error"),MB_ICONERROR|MB_OK);
	  FreeLibrary(m_hMobileDll);
	  return false; 
   }

   if (ANWGetSMSOneData == NULL)
   {
      MessageBox(_T("Can't Get ANWGetSMSOneData()"),_T("Error"),MB_ICONERROR|MB_OK);
	  FreeLibrary(m_hMobileDll);
	  return false; 
   }

   if (ANWGetSMSNextData == NULL)
   {
      MessageBox(_T("Can't Get ANWGetSMSNextData()"),_T("Error"),MB_ICONERROR|MB_OK);
	  FreeLibrary(m_hMobileDll);
	  return false; 
   }

   if (ANWSendSMSData == NULL)
   {
      MessageBox(_T("Can't Get ANWSendSMSData()"),_T("Error"),MB_ICONERROR|MB_OK);
	  FreeLibrary(m_hMobileDll);
	  return false; 
   }

   if (ANWPutSMSData == NULL)
   {
      MessageBox(_T("Can't Get ANWPutSMSData()"),_T("Error"),MB_ICONERROR|MB_OK);
	  FreeLibrary(m_hMobileDll);
	  return false; 
   }

   if (ANWDeleteSMSData == NULL)
   {
      MessageBox(_T("Can't Get ANWDeleteSMSData()"),_T("Error"),MB_ICONERROR|MB_OK);
	  FreeLibrary(m_hMobileDll);
	  return false; 
   }

   if (ANWDeleteAllSMSData == NULL)
   {
      MessageBox(_T("Can't Get ANWDeleteAllSMSData()"),_T("Error"),MB_ICONERROR|MB_OK);
	  FreeLibrary(m_hMobileDll);
	  return false; 
   }

   if (ANWCloseSMS == NULL)
   {
      MessageBox(_T("Can't Get ANWCloseSMS()"),_T("Error"),MB_ICONERROR|MB_OK);
	  FreeLibrary(m_hMobileDll);
	  return false; 
   }

#ifdef ASUSM303
   if (ANWGetMobileSMSFolders == NULL)
   {
      MessageBox(_T("Can't Get GetMobileSMSFolders()"),_T("Error"),MB_ICONERROR|MB_OK);
	  FreeLibrary(m_hMobileDll);
	  return false; 
   }
#endif
   /*if (ANWGetIMEI == NULL)
   {
      MessageBox("Can't Get ANWCloseSMS()","Error",MB_ICONERROR|MB_OK);
	  FreeLibrary(m_hMobileDll);
	  return false; 
   }*/


    OpenPhoneBookfn = (OpenPhoneBookProc)::GetProcAddress(m_hMobileDll,"OpenPhoneBook");
	ClosePhoneBookfn = (ClosePhoneBookProc)::GetProcAddress(m_hMobileDll,"ClosePhoneBook");
	GetPhoneBookInfofn = (GetPhoneBookInfoProc)::GetProcAddress(m_hMobileDll,"GetPhoneBookInfo");
	GetPhoneBookStartDatafn = (GetPhoneBookStartDataProc)::GetProcAddress(m_hMobileDll,"GetPhoneBookStartData");
	GetPhoneBookNextDatafn = (GetPhoneBookNextDataProc)::GetProcAddress(m_hMobileDll,"GetPhoneBookNextData");
	InitPhoneBookfn = (InitPhoneBookProc)::GetProcAddress(m_hMobileDll,"InitPhoneBook");
	//CheckSIMfn = (CheckSIMProc)::GetProcAddress(m_hMobileDll,"CheckSIM");
	
	InitialMemoryStatusfn = (InitialMemoryStatusProc)GetProcAddress(m_hMobileDll,"InitialMemoryStatus");
	if(InitialMemoryStatusfn == NULL)
	{
		MessageBox(_T("Can't Get InitialMemoryStatusfn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}

	if(InitPhoneBookfn == NULL)
	{
		MessageBox(_T("Can't Get InitPhoneBook()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}

	if(OpenPhoneBookfn == NULL)
	{
		MessageBox(_T("Can't Get OpenPhoneBook()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}

	if(ClosePhoneBookfn == NULL)
	{
		MessageBox(_T("Can't Get ClosePhoneBook()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}

	if (GetPhoneBookInfofn == NULL)
	{
		MessageBox(_T("Can't Get GetPhoneBookInfo()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}


	if (GetPhoneBookStartDatafn == NULL)
	{
		MessageBox(_T("Can't Get GetPhoneBookStartData()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}

	if (GetPhoneBookNextDatafn == NULL)
	{
		MessageBox(_T("Can't Get GetPhoneBookNextData()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}

	/*if (CheckSIMfn == NULL)
	{
		MessageBox("Can't Get CheckSIMProc()","Error",MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}*/

   return true;
}

int CLeftView::GetMobileSetting(void)
{
	TCHAR FileName[MAX_PATH];
	CString strFileName;

	GetModuleFileName(NULL, FileName, MAX_PATH);
	strFileName = FileName;
	strFileName = strFileName.Left(strFileName.ReverseFind('\\'));
	strFileName +=_T("\\mobile_setting.ini");

	_tcscpy(FileName, strFileName.GetBuffer(strFileName.GetLength()));

	int ret = al_GetSettingString(_T("mobile"), _T("displayname"), FileName, m_strMobileName);
	ret = al_GetSettingString(_T("mobile"), _T("name"), FileName, m_strMobileName_ori);
	ret = al_GetSettingString(_T("mobile"), _T("phone"), FileName, m_strPhone);
	ret = al_GetSettingString(_T("mobile"), _T("port"), FileName, m_strPort);
	ret = al_GetSettingString(_T("mobile"), _T("connect_mode"), FileName, m_strConnectMode);
	ret = al_GetSettingString(_T("mobile"), _T("company"), FileName, m_strCompany);
	ret = al_GetSettingString(_T("mobile"), _T("IMEI"), FileName, m_strIMEI);

	ret = al_GetSettingString(_T("lib"), _T("control_dll"), FileName, m_strMobileDLL);
	
	return true;
}


const int SMS_ITEMS = 255;
void CLeftView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	//m_droptarget.Register(this);

}


void CLeftView::SetFirstPosition(void)
{
	TCHAR szBuffer[MAX_PATH];
	if( SHGetSpecialFolderPath(NULL, szBuffer, CSIDL_PERSONAL, TRUE) == FALSE)	
	{
		return ;
	}

	BOOL bret = ::SetCurrentDirectory(szBuffer);
}


int CLeftView::GetPCSMSInfo(void)
{
	TCHAR szBuffer[MAX_PATH];
	if( SHGetSpecialFolderPath(NULL, szBuffer, CSIDL_PERSONAL, TRUE) == FALSE)	
	{
		//GetWindowsDirectory(szBuffer, MAX_PATH);
		//lstrcat(szBuffer, "\\Application Data");
		//CreateDirectory(szBuffer, NULL);
		return false;
	}

	// 1. Check SMSUtility's folder have exist
	s_ActiveDlg->SetPCSaveSMSFolder(szBuffer);
	
	// 2. Get All SMS form PC that we have save before.
	s_ActiveDlg->GetPCSaveSMSInfo(szBuffer);
	
	SetFirstPosition();

	return true;
}
int CLeftView::xGetPCSaveSMSInfo_ANSCII(CString szDir)
{
#ifdef _UNICODE
	USES_CONVERSION;
	CString strGetFile;

	CFileFind finder;
	BOOL bret = SetCurrentDirectory(szDir);
	BOOL bWorking = finder.FindFile(_T("*.sm"));
	int  i = 0;
	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		if(finder.IsDots())
		{

		}
		else
		{
			CString strFileName =  finder.GetFileName();
			
			strGetFile = szDir;
			strGetFile += _T("\\");
			strGetFile += strFileName;
			FILE *fptr = _tfopen(strGetFile, _T("rb"));
			if(fptr)
			{
				char TPA[40];			
				char TP_SCTS[40];		
				char TP_UD[MAX_CONTENT+2];	

				fread(TP_SCTS, 20, 1, fptr);
				fread(TPA, 20, 1, fptr);
				fread(TP_UD, MAX_CONTENT, 1, fptr);
				fclose(fptr);
				i++;
				
				CString strCurDir;
				strCurDir = strGetFile;
				strCurDir+= _T("u");
				FILE *fpwrite = _tfopen(strCurDir, _T("w+b"));
				if(fpwrite)
				{
					TCHAR wTPA[40];
					TCHAR wTP_SCTS[40];		
					TCHAR wTP_UD[MAX_CONTENT+2];	
				
					wsprintf(wTPA,A2W(TPA));
					wsprintf(wTP_SCTS,A2W(TP_SCTS));
					wsprintf(wTP_UD,A2W(TP_UD));

					fwrite(wTP_SCTS, sizeof(TCHAR), 40, fpwrite);
					fwrite(wTPA, sizeof(TCHAR), 40, fpwrite);
					fwrite(wTP_UD, sizeof(TCHAR), MAX_TP_UD+2, fpwrite);
					fclose(fpwrite);
					DeleteFile(strGetFile);
				}
			}


		}

	}
	return i;
#endif
	return 0;
}
int CLeftView::xGetPCSaveSMSInfo(CString szDir, SMS_PARAM *sms_info)
{
	CString strGetFile;

	CFileFind finder;
	BOOL bret = SetCurrentDirectory(szDir);
#ifdef _UNICODE
	xGetPCSaveSMSInfo_ANSCII(szDir);
	BOOL bWorking = finder.FindFile(_T("*.smu"));
#else
	BOOL bWorking = finder.FindFile(_T("*.sm"));
#endif
	int  i = 0;
	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		if(finder.IsDots())
		{

		}
		else
		{
			CString strFileName =  finder.GetFileName();
			
			strGetFile = szDir;
			strGetFile += _T("\\");
			strGetFile += strFileName;
			FILE *fptr = _tfopen(strGetFile, _T("rb"));
			if(fptr)
			{
				fread(sms_info[i].TP_SCTS, sizeof(TCHAR), 40, fptr);
				fread(sms_info[i].TPA,sizeof(TCHAR), 40, fptr);
				fread(sms_info[i].TP_UD, sizeof(TCHAR), MAX_TP_UD+2, fptr);
				fclose(fptr);

				//sms_info[i].position = i;

				i++;
			}
		}

	}
	return i;
	//bret = SetCurrentDirectory("c:\\");

}

void CLeftView::GetPCSaveSMSInfo(TCHAR *szBuffer)
{
	CString str;

	// PC's Inbox Fodler
	str = CString(szBuffer);
	str += CString(INBOX_FOLDER);
//	m_TreeItemBackup[itPC_INBOX] = al_GetDirFileCount(str);
	m_TreeItemBackup[itPC_INBOX] = xGetPCSaveSMSInfo(str, m_PC_Inbox);

	// PC's Outbox Fodler
	str = CString(szBuffer);
	str += CString(OUTBOX_FOLDER);
//	m_TreeItemBackup[itPC_OUTBOX] = al_GetDirFileCount(str);
	m_TreeItemBackup[itPC_OUTBOX] = xGetPCSaveSMSInfo(str, m_PC_Outbox);

	// PC's Backup Fodler
	str = CString(szBuffer);
	str += CString(BACKUP_FOLDER);
//	m_TreeItemBackup[itPC_SENDBK] = al_GetDirFileCount(str);
	m_TreeItemBackup[itPC_SENDBK] = xGetPCSaveSMSInfo(str, m_PC_SendBK);

	// PC's Draft Fodler
	str = CString(szBuffer);
	str += CString(DRAFT_FOLDER);
//	m_TreeItemBackup[itPC_DRAFT] = al_GetDirFileCount(str);
	m_TreeItemBackup[itPC_DRAFT] = xGetPCSaveSMSInfo(str, m_PC_Draft);

	// PC's Trash Fodler
	str = CString(szBuffer);
	str += CString(TRASH_FOLDER);
//	m_TreeItemBackup[itPC_GARBAGE] = al_GetDirFileCount(str);
	m_TreeItemBackup[itPC_GARBAGE] = xGetPCSaveSMSInfo(str, m_PC_Garbage);
	
}


void CLeftView::SetPCSaveSMSFolder(TCHAR *szBuffer)
{
	lstrcat(szBuffer, SMS_FOLDER);
	int ret = al_IsFileExist(szBuffer);
	CString str;
	if(ret != 1)
	{
		CreateDirectory(szBuffer, NULL);

		str = CString(szBuffer);
		str += CString(INBOX_FOLDER);
		CreateDirectory(str, NULL);

		str = CString(szBuffer);
		str += CString(OUTBOX_FOLDER);
		CreateDirectory(str, NULL);

		str = CString(szBuffer);
		str += CString(BACKUP_FOLDER);
		CreateDirectory(str, NULL);

		str = CString(szBuffer);
		str += CString(DRAFT_FOLDER);
		CreateDirectory(str, NULL);

		str = CString(szBuffer);
		str += CString(TRASH_FOLDER);
		CreateDirectory(str, NULL);
	}

	str = CString(szBuffer);
	str += CString(INBOX_FOLDER);
	ret = al_IsFileExist(str);
	if(ret != 1)
		CreateDirectory(str, NULL);
		
	str = CString(szBuffer);
	str += CString(OUTBOX_FOLDER);
	ret = al_IsFileExist(str);
	if(ret != 1)
		CreateDirectory(str, NULL);
		
	str = CString(szBuffer);
	str += CString(BACKUP_FOLDER);
	ret = al_IsFileExist(str);
	if(ret != 1)
		CreateDirectory(str, NULL);
		
	str = CString(szBuffer);
	str += CString(DRAFT_FOLDER);
	ret = al_IsFileExist(str);
	if(ret != 1)
		CreateDirectory(str, NULL);
		
	str = CString(szBuffer);
	str += CString(TRASH_FOLDER);
	ret = al_IsFileExist(str);
	if(ret != 1)
		CreateDirectory(str, NULL);
		

}

void CLeftView::OnBeginrdrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

int CLeftView::GetSelectItem(HTREEITEM hCurSelItem)
{
	if(hCurSelItem == gMobileItem)
	{
		return itMOBILE;
	}
	else if(hCurSelItem == gPCItem)
	{
		return itPC;
	}
	else if(hCurSelItem == gMobileSIMCardItem)
	{
		return itMOBILE_SIMCARD;
	}
	else if(hCurSelItem == gMobileSIMCardInboxItem)
	{
		return itMOBILE_SIMCARD_INBOX;
	}
	else if(hCurSelItem == gMobileSIMCardOutboxItem)
	{
		return itMOBILE_SIMCARD_OUTBOX;
	}
	else if(hCurSelItem == gMobileHandsetItem)
	{
		return itMOBILE_HANDSET;
	}
	else if(hCurSelItem == gMobileHandsetInboxItem)
	{
		return itMOBILE_HANDSET_INBOX;
	}
	else if(hCurSelItem == gMobileHandsetOutboxItem)
	{
		return itMOBILE_HANDSET_OUTBOX;
	}
	else if(hCurSelItem == gPCInboxItem)
	{
		return itPC_INBOX;
	}
	else if(hCurSelItem == gPCOutboxItem)
	{
		return itPC_OUTBOX;
	}
	else if(hCurSelItem == gPCSendBKItem)
	{
		return itPC_SENDBK;
	}
	else if(hCurSelItem == gPCDraftItem)
	{
		return itPC_DRAFT;
	}
	else if(hCurSelItem == gPCGarbageItem)
	{
		return itPC_GARBAGE;
	}
#ifdef ASUSM303
	else if(hCurSelItem == gMobileHandsetFolder1Item)
	{
		return itMOBILE_HANDSET_OTHER1;
	}
	else if(hCurSelItem == gMobileHandsetFolder2Item)
	{
		return itMOBILE_HANDSET_OTHER2;
	}
	else if(hCurSelItem == gMobileHandsetFolder3Item)
	{
		return itMOBILE_HANDSET_OTHER3;
	}
	else if(hCurSelItem == gMobileHandsetFolder4Item)
	{
		return itMOBILE_HANDSET_OTHER4;
	}
	/*else if(hCurSelItem == gMobileHandsetFolder5Item)
	{
		return itMOBILE_HANDSET_OTHER5;
	}*/
#endif

	return itNONE;
}

SMS_PARAM* CLeftView::GetTreeSMSInfo(HTREEITEM hCurSelItem, int &nItems)
{
	if(hCurSelItem == gMobileItem)
	{}
	else if(hCurSelItem == gPCItem)
	{}
	else if(hCurSelItem == gMobileSIMCardItem)
	{
	}
	else if(hCurSelItem == gMobileSIMCardInboxItem)
	{
		nItems = m_TreeItemBackup[itMOBILE_SIMCARD_INBOX];
		return m_SIMSMS_Inbox;
	}
	else if(hCurSelItem == gMobileSIMCardOutboxItem)
	{
		nItems = m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX];
		return m_SIMSMS_Outbox;
	}
	else if(hCurSelItem == gMobileHandsetItem)
	{}
	else if(hCurSelItem == gMobileHandsetInboxItem)
	{
		nItems = m_TreeItemBackup[itMOBILE_HANDSET_INBOX];
		return m_HandsetSMS_Inbox;
	}
	else if(hCurSelItem == gMobileHandsetOutboxItem)
	{
		nItems = m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX];
		return m_HandsetSMS_Outbox;
	}
#ifdef ASUSM303
	else if(hCurSelItem == gMobileHandsetFolder1Item)
	{
		nItems = m_TreeItemBackup[itMOBILE_HANDSET_OTHER1];
		return m_HandsetSMS_Folder1;
	}
	else if(hCurSelItem == gMobileHandsetFolder2Item)
	{
		nItems = m_TreeItemBackup[itMOBILE_HANDSET_OTHER2];
		return m_HandsetSMS_Folder2;
	}
	else if(hCurSelItem == gMobileHandsetFolder3Item)
	{
		nItems = m_TreeItemBackup[itMOBILE_HANDSET_OTHER3];
		return m_HandsetSMS_Folder3;
	}
	else if(hCurSelItem == gMobileHandsetFolder4Item)
	{
		nItems = m_TreeItemBackup[itMOBILE_HANDSET_OTHER4];
		return m_HandsetSMS_Folder4;
	}
	/*else if(hCurSelItem == gMobileHandsetFolder5Item)
	{
		nItems = m_TreeItemBackup[itMOBILE_HANDSET_OTHER4];
		return m_HandsetSMS_Folder5;
	}*/
#endif
	else if(hCurSelItem == gPCInboxItem)
	{
		nItems = m_TreeItemBackup[itPC_INBOX];
		return m_PC_Inbox;
	}
	else if(hCurSelItem == gPCOutboxItem)
	{
		nItems = m_TreeItemBackup[itPC_OUTBOX];
		return m_PC_Outbox;
	}
	else if(hCurSelItem == gPCSendBKItem)
	{
		nItems = m_TreeItemBackup[itPC_SENDBK];
		return m_PC_SendBK;
	}
	else if(hCurSelItem == gPCDraftItem)
	{
		nItems = m_TreeItemBackup[itPC_DRAFT];
		return m_PC_Draft;
	}
	else if(hCurSelItem == gPCGarbageItem)
	{
		nItems = m_TreeItemBackup[itPC_GARBAGE];
		return m_PC_Garbage;
	}

	nItems  = 0;
	return NULL;
}

void CLeftView::xNewSMS()
{
	CNewSMS	NewSMSDlg;
	
	SMS_PARAM msgInfo;
	memset(&msgInfo, 0, sizeof(SMS_PARAM));
	

	NewSMSDlg.SetData(msgInfo, SMSSend, true,false);
	int ret = NewSMSDlg.DoModal();
	if(ret == IDOK)
	{
		int i = 0;
	}
	else if(ret == IDCANCEL)
	{
		int i = 0;
	}

}

void CLeftView::MultiByteCut(CString lpszSrc, TCHAR* lpszDes, int nPage)
{
	WCHAR* pWsrc = NULL;
#ifdef _UNICODE
	pWsrc = lpszSrc.GetBuffer(MAX_PATH);
#else
	USES_CONVERSION;
	TCHAR* pSrc = lpszSrc.GetBuffer(MAX_PATH);
	pWsrc =  A2W(pSrc);
#endif
	int nLen = wcslen(pWsrc);
	if(nLen >= MULTIBYTE*nPage)
	{
#ifdef _UNICODE
		memcpy(lpszDes, pWsrc+(MULTIBYTE*(nPage-1)), MULTIBYTE*sizeof(TCHAR));
		lpszDes[MULTIBYTE] ='\0';
#else
		USES_CONVERSION;
		WCHAR wDet[MULTIBYTE+1];

		memcpy(wDet, pWsrc+(MULTIBYTE*(nPage-1)), MULTIBYTE*sizeof(WCHAR));
		wDet[MULTIBYTE] ='\0';
		sprintf(lpszDes,"%s",W2A(wDet));
#endif
	}
	else if(nLen > MULTIBYTE*(nPage-1))
	{
		int ncopylen = nLen -( MULTIBYTE*(nPage-1));

#ifdef _UNICODE
		memcpy(lpszDes, pWsrc+(MULTIBYTE*(nPage-1)), ncopylen*sizeof(TCHAR));
		lpszDes[ncopylen] ='\0';
#else
		USES_CONVERSION;
		WCHAR wDet[MULTIBYTE+1];

		memcpy(wDet, pWsrc+(MULTIBYTE*(nPage-1)), ncopylen*sizeof(WCHAR));
		wDet[ncopylen] ='\0';
		sprintf(lpszDes,"%s",W2A(wDet));
#endif
	}

	lpszSrc.ReleaseBuffer();
}

void CLeftView::SingleByteCut(CString lpszSrc, TCHAR* lpszDes, int nPage)
{
	WCHAR* pWsrc = NULL;
#ifdef _UNICODE
	pWsrc = lpszSrc.GetBuffer(MAX_PATH);
#else
	USES_CONVERSION;
	TCHAR* pSrc = lpszSrc.GetBuffer(MAX_PATH);
	pWsrc =  A2W(pSrc);
#endif
	int nLen = wcslen(pWsrc);
	if(nLen >= SINGLEBYTE*nPage)
	{
#ifdef _UNICODE
		memcpy(lpszDes, pWsrc+(SINGLEBYTE*(nPage-1)), SINGLEBYTE*sizeof(TCHAR));
		lpszDes[SINGLEBYTE] ='\0';
#else
		USES_CONVERSION;
		WCHAR wDet[SINGLEBYTE+1];

		memcpy(wDet, pWsrc+(SINGLEBYTE*(nPage-1)), SINGLEBYTE*sizeof(WCHAR));
		wDet[SINGLEBYTE] ='\0';
		sprintf(lpszDes,"%s",W2A(wDet));
#endif
	}
	else if(nLen > SINGLEBYTE*(nPage-1))
	{
		int ncopylen = nLen -( SINGLEBYTE*(nPage-1));
#ifdef _UNICODE
		memcpy(lpszDes, pWsrc+(SINGLEBYTE*(nPage-1)), ncopylen*sizeof(TCHAR));
		lpszDes[ncopylen] ='\0';
#else
		USES_CONVERSION;
		WCHAR wDet[SINGLEBYTE+1];

		memcpy(wDet, pWsrc+(SINGLEBYTE*(nPage-1)), ncopylen*sizeof(WCHAR));
		wDet[ncopylen] ='\0';
		sprintf(lpszDes,"%s",W2A(wDet));
#endif
	}

	lpszSrc.ReleaseBuffer();

}

static int gSendPos = 0;
static int gNextPos = 0;
int CLeftView::SendMobileOneSMS(int nNeed)
{
	SMS_Data_Strc smsSend;
	memset(&smsSend, 0, sizeof(SMS_Data_Strc));
	_tcscpy(smsSend.TP_UD, s_ActiveDlg->m_Mobile_SMS[gNextPos].TP_UD); 
	_tcscpy(smsSend.TPA, s_ActiveDlg->m_Mobile_SMS[gNextPos].TPA);
	smsSend.TP_DCS = s_ActiveDlg->m_Mobile_SMS[gNextPos].TP_DCS;

	short SendMemType = MEM_SM;
	short SendSMSfolderType = 0;

	// Check Phone Number Lsit
	int ret = 0;
	////Sleep(1000);
	ret = ANWSendSMSData(SendMemType, SendSMSfolderType, &smsSend);
	s_ActiveDlg->xErrorHandling(ret);
	if(ret == Anw_TRIAL_LIMITEDFUNCTION)
		return false ;
	gNextPos++;


	SMS_PARAM msgInfo;
	memcpy(&msgInfo, &smsSend, sizeof(SMS_PARAM));
	if(ret == Anw_SUCCESS)
	{
		s_ActiveDlg->xSaveSMSToBackUpFolder(msgInfo, msgInfo.TPA);
	}
	else
	{
		s_ActiveDlg->xSaveSMSToDraftFolder(msgInfo, msgInfo.TPA);
	}

//	return ret;
	return true;
}

int CLeftView::xSendSMS(TCHAR *strGetMsg, CString PhoneNumberList, int nTotalPage, bool bMultiByte)
{
	if(theApp.nIsConnected != true)
		return false;


//	char *PhoneList = new char[MAX_CONTENT+1];
	CString PhoneList;
	//memcpy(PhoneList, PhoneNumberList, strlen(PhoneNumberList));
	
	TCHAR strSendMsg[MAX_CONTENT+1];
	int nPos = 0;
	for(int nSendPage = 1; nSendPage <= nTotalPage; nSendPage++)
	{
		memset(strSendMsg, 0, (MAX_CONTENT+1)*sizeof(TCHAR));
		if(bMultiByte)
		{
			MultiByteCut(strGetMsg, strSendMsg, nSendPage);

			// Check Phone Number Lsit
			int ret = 0;
		//	memset(PhoneList, 0, MAX_CONTENT+1);
		//	memcpy(PhoneList, PhoneNumberList, strlen(PhoneNumberList));
			PhoneList.Format(_T("%s"),PhoneNumberList);
			TCHAR *PhoneNumber = _tcstok(PhoneList.GetBuffer(MAX_PATH), _T(";"));
			while(1)
			{
				s_ActiveDlg->m_Mobile_SMS[nPos].TP_DCS = 1; // Unicode
				_tcscpy(s_ActiveDlg->m_Mobile_SMS[nPos].TP_UD, strSendMsg);
				_tcscpy(s_ActiveDlg->m_Mobile_SMS[nPos].TPA, PhoneNumber);
				nPos++;

				PhoneNumber = _tcstok(NULL, _T(";"));
				if(PhoneNumber == NULL)	
					break;
			}
			PhoneList.ReleaseBuffer();
		}
		else
		{
			SingleByteCut(strGetMsg, strSendMsg, nSendPage);

			// Check Phone Number Lsit
			int ret = 0;
	//		memset(PhoneList, 0, MAX_CONTENT+1);
		//	memcpy(PhoneList, PhoneNumberList, strlen(PhoneNumberList));
			PhoneList.Format(_T("%s"),PhoneNumberList);
			TCHAR *PhoneNumber = _tcstok(PhoneList.GetBuffer(MAX_PATH), _T(";"));
			while(1)
			{
				s_ActiveDlg->m_Mobile_SMS[nPos].TP_DCS = 2; // Default
				_tcscpy(s_ActiveDlg->m_Mobile_SMS[nPos].TP_UD, strSendMsg);
				_tcscpy(s_ActiveDlg->m_Mobile_SMS[nPos].TPA, PhoneNumber);
				nPos++;

				PhoneNumber = _tcstok(NULL, _T(";"));
				if(PhoneNumber == NULL)	
					break;
			}
			PhoneList.ReleaseBuffer();
		}

		
	}

	int nNeedCount = nPos;
	gSendPos = nNeedCount;

	CProcessDlg dlg;
	if(nNeedCount >= 0)
	{
		gNextPos = 0;
		dlg.SetProgressInfo(CLeftView::SendMobileOneSMS, nNeedCount);
		dlg.SetLoopCount(nNeedCount);
		m_bRuningDlg = true;
		dlg.DoModal();
		m_bRuningDlg = false;
	}



	return true;
}


int CLeftView::xSaveSMSToDraftFolder(SMS_PARAM msgInfo, TCHAR *strPhoneList)
{
	TCHAR *PhoneNumber = _tcstok(strPhoneList,  _T(";"));

	if(m_TreeItemBackup[itPC_DRAFT] >= 1000)
	{
		TCHAR str1[MAX_PATH];
		al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
		afxMessageBox(str1);
		return true;
	}

	
	while(1)
	{
		_tcscpy(m_PC_Draft[m_TreeItemBackup[itPC_DRAFT]].TP_UD, msgInfo.TP_UD); 
		_tcscpy(m_PC_Draft[m_TreeItemBackup[itPC_DRAFT]].TPA, PhoneNumber);
		m_TreeItemBackup[itPC_DRAFT]++;

		PhoneNumber = _tcstok(NULL, _T(";"));
		if(PhoneNumber == NULL)	
			break;
	}


	HTREEITEM hCurSelItem = GetTreeCtrl().GetSelectedItem();
	int nCurSelItem = GetSelectItem(hCurSelItem);

	UpdateOneTreeItem(gPCDraftItem, itPC_DRAFT);

	int nItems;
	SMS_PARAM *showInfo = GetTreeSMSInfo(hCurSelItem, nItems);
	//CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();
	//PreListView->ShowItems(showInfo, nCurSelItem, m_TreeItemBackup[nCurSelItem]);
	gPreListView->ShowItems(showInfo, nCurSelItem, m_TreeItemBackup[nCurSelItem]);

	return true;
}
int CLeftView::xSaveSMSToDraftFolder(TCHAR *strGetMsg, CString PhoneNumberList, int nTotalPage, bool bMultiByte)
{
	//char *PhoneNumber = _tcstok(strPhoneList, ";");

	if(m_TreeItemBackup[itPC_DRAFT] >= 1000)
	{
		TCHAR str1[MAX_PATH];
		al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
		afxMessageBox(str1);
		return true;
	}

//	char PhoneList[MAX_CONTENT+1];
	CString PhoneList;
	//memcpy(PhoneList, PhoneNumberList, strlen(PhoneNumberList));
	
	TCHAR strSendMsg[MAX_CONTENT+1];
	int nPos = 0;
	for(int nSendPage = 1; nSendPage <= nTotalPage; nSendPage++)
	{
		if(m_TreeItemBackup[itPC_DRAFT] >= 1000)
		{
			TCHAR str1[MAX_PATH];
			al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
			afxMessageBox(str1);
			return true;
		}
		memset(strSendMsg, 0, MAX_CONTENT+1);
		if(bMultiByte)
		{
			MultiByteCut(strGetMsg, strSendMsg, nSendPage);

			// Check Phone Number Lsit
			int ret = 0;
			PhoneList.Format(_T("%s"),PhoneNumberList);
			TCHAR *PhoneNumber = _tcstok(PhoneList.GetBuffer(MAX_PATH),_T(";"));
			while(1)
			{
				if(m_TreeItemBackup[itPC_DRAFT] >= 1000)
				{
					TCHAR str1[MAX_PATH];
					al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
					afxMessageBox(str1);
					PhoneList.ReleaseBuffer();
					return true;
				}
				m_PC_Draft[m_TreeItemBackup[itPC_DRAFT]].TP_DCS = 1;
				_tcscpy(m_PC_Draft[m_TreeItemBackup[itPC_DRAFT]].TP_UD, strSendMsg); 
				_tcscpy(m_PC_Draft[m_TreeItemBackup[itPC_DRAFT]].TPA, PhoneNumber);
				m_TreeItemBackup[itPC_DRAFT]++;

				PhoneNumber = _tcstok(NULL, _T(";"));
				if(PhoneNumber == NULL)	
					break;
			}
			PhoneList.ReleaseBuffer();
		}
		else
		{
			SingleByteCut(strGetMsg, strSendMsg, nSendPage);

			// Check Phone Number Lsit
			int ret = 0;
			PhoneList.Format(_T("%s"),PhoneNumberList);
			TCHAR *PhoneNumber = _tcstok(PhoneList.GetBuffer(MAX_PATH), _T(";"));
			while(1)
			{
				if(m_TreeItemBackup[itPC_DRAFT] >= 1000)
				{
					TCHAR str1[MAX_PATH];
					al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
					afxMessageBox(str1);
					PhoneList.ReleaseBuffer();
					return true;
				}
				m_PC_Draft[m_TreeItemBackup[itPC_DRAFT]].TP_DCS = 2;
				_tcscpy(m_PC_Draft[m_TreeItemBackup[itPC_DRAFT]].TP_UD, strSendMsg); 
				_tcscpy(m_PC_Draft[m_TreeItemBackup[itPC_DRAFT]].TPA, PhoneNumber);
				m_TreeItemBackup[itPC_DRAFT]++;

				PhoneNumber = _tcstok(NULL, _T(";"));
				if(PhoneNumber == NULL)	
					break;
			}
			PhoneList.ReleaseBuffer();
		}

		
	}
/*	
	while(1)
	{
		_tcscpy(m_PC_Draft[m_TreeItemBackup[itPC_DRAFT]].TP_UD, msgInfo.TP_UD); 
		_tcscpy(m_PC_Draft[m_TreeItemBackup[itPC_DRAFT]].TPA, PhoneNumber);
		m_TreeItemBackup[itPC_DRAFT]++;

		PhoneNumber = _tcstok(NULL, _T(";"));
		if(PhoneNumber == NULL)	
			break;
	}
*/

	HTREEITEM hCurSelItem = GetTreeCtrl().GetSelectedItem();
	int nCurSelItem = GetSelectItem(hCurSelItem);

	UpdateOneTreeItem(gPCDraftItem, itPC_DRAFT);

	int nItems;
	SMS_PARAM *showInfo = GetTreeSMSInfo(hCurSelItem, nItems);
	//CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();
	//PreListView->ShowItems(showInfo, nCurSelItem, m_TreeItemBackup[nCurSelItem]);
	gPreListView->ShowItems(showInfo, nCurSelItem, m_TreeItemBackup[nCurSelItem]);

	return true;
}
int CLeftView::xSaveSMSToBackUpFolder(SMS_PARAM msgInfo, TCHAR *strPhoneList)
{
	TCHAR *PhoneNumber = _tcstok(strPhoneList, _T(";"));

	if(m_TreeItemBackup[itPC_SENDBK] >= 1000)
	{
		TCHAR str1[MAX_PATH];
		al_GetSettingString(_T("public"), _T("IDS_DATA_EXCESS"), theApp.m_szRes, str1);
		afxMessageBox(str1);
		return true;
	}
	
	while(1)
	{
		_tcscpy(m_PC_SendBK[m_TreeItemBackup[itPC_SENDBK]].TP_UD, msgInfo.TP_UD); 
		_tcscpy(m_PC_SendBK[m_TreeItemBackup[itPC_SENDBK]].TPA, PhoneNumber);
		m_TreeItemBackup[itPC_SENDBK]++;

		PhoneNumber = _tcstok(NULL, _T(";"));
		if(PhoneNumber == NULL)	
			break;
	}


	HTREEITEM hCurSelItem = GetTreeCtrl().GetSelectedItem();
	int nCurSelItem = GetSelectItem(hCurSelItem);

	UpdateOneTreeItem(gPCSendBKItem, itPC_SENDBK);
	
	int nItems;
	SMS_PARAM *showInfo = GetTreeSMSInfo(hCurSelItem, nItems);
	//CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();
	//PreListView->ShowItems(showInfo, nCurSelItem, m_TreeItemBackup[nCurSelItem]);
	gPreListView->ShowItems(showInfo, nCurSelItem, m_TreeItemBackup[nCurSelItem]);

	return true;
}

void CLeftView::ProduceAFile(CString strPName, int nNextFile, SMS_PARAM smsInfo)
{
	CTime g_startTime=0;
	CTime hCurrentTime = CTime::GetCurrentTime();
	CTimeSpan hElapsedTime = hCurrentTime - g_startTime;
	int nCurrentTime = (int)hElapsedTime.GetTotalSeconds();
	TCHAR strFile[20];
	_itot(nCurrentTime, strFile, 10);

	TCHAR strNext[4];
	_itot(nNextFile, strNext, 10);

	CString strCurDir = strPName;
	strCurDir += _T("\\");
	strCurDir += CString(strFile);
	strCurDir += CString(strNext);
#ifdef _UNICODE
	strCurDir += _T(".smu");
#else
	strCurDir += _T(".sm");
#endif
	FILE *fptr = _tfopen(strCurDir, _T("w+b"));
	if(fptr == NULL)
		return;

//	int n = strlen(smsInfo.TP_UD);
	fwrite(smsInfo.TP_SCTS, sizeof(TCHAR), 40, fptr);
	fwrite(smsInfo.TPA, sizeof(TCHAR), 40, fptr);
	fwrite(smsInfo.TP_UD, sizeof(TCHAR), MAX_TP_UD+2, fptr);
	fclose(fptr);
}


void CLeftView::BackupAllDataToPC(void)
{
	int nNext = 0;
	CString str;

	TCHAR szBuffer[MAX_PATH];
	if( SHGetSpecialFolderPath(NULL, szBuffer, CSIDL_PERSONAL, TRUE) == FALSE)	
	{
		// Other OS
		//GetWindowsDirectory(szBuffer, MAX_PATH); 
		//lstrcat(szBuffer, "\\Application Data");
		//CreateDirectory(szBuffer, NULL);
		return ;
	}
	else
	{
		//delete all data
		str = CString(szBuffer);
		str += SMS_FOLDER;
	//	str += _T("\\");

	//	RecursiveDelete(str);

		str += CString(INBOX_FOLDER);
		DeletePCSMSFile(str);

		str = CString(szBuffer);
		str += SMS_FOLDER;
		str += CString(OUTBOX_FOLDER);
		DeletePCSMSFile(str);

		str = CString(szBuffer);
		str += SMS_FOLDER;
		str += CString(BACKUP_FOLDER);
		DeletePCSMSFile(str);

		str = CString(szBuffer);
		str += SMS_FOLDER;
		str += CString(DRAFT_FOLDER);
		DeletePCSMSFile(str);

		str = CString(szBuffer);
		str += SMS_FOLDER;
		str += CString(TRASH_FOLDER);
		DeletePCSMSFile(str);

		TCHAR szSaveBuf[MAX_PATH];
		_tcscpy(szSaveBuf, szBuffer);
		SetPCSaveSMSFolder(szSaveBuf);
	}
		


	//m_PC_Inbox
	str = CString(szBuffer);
	str += SMS_FOLDER;
	str += CString(INBOX_FOLDER);
	for(nNext = 0; nNext < m_TreeItemBackup[itPC_INBOX]; nNext++)
	{
		ProduceAFile(str, nNext, m_PC_Inbox[nNext]);
	}
	

	//m_PC_Outbox
	str = CString(szBuffer);
	str += SMS_FOLDER;
	str += CString(OUTBOX_FOLDER);
	for(nNext = 0; nNext < m_TreeItemBackup[itPC_OUTBOX]; nNext++)
	{
		ProduceAFile(str, nNext, m_PC_Outbox[nNext]);
	}

	//m_PC_SendBK
	str = CString(szBuffer);
	str += SMS_FOLDER;
	str += CString(BACKUP_FOLDER);
	for(nNext = 0; nNext < m_TreeItemBackup[itPC_SENDBK]; nNext++)
	{
		ProduceAFile(str, nNext, m_PC_SendBK[nNext]);
	}

	//m_PC_Draft
	str = CString(szBuffer);
	str += SMS_FOLDER;
	str += CString(DRAFT_FOLDER);
	for(nNext = 0; nNext < m_TreeItemBackup[itPC_DRAFT]; nNext++)
	{
		ProduceAFile(str, nNext, m_PC_Draft[nNext]);
	}	

	//m_PC_Garbage
	str = CString(szBuffer);
	str += SMS_FOLDER;
	str += CString(TRASH_FOLDER);
	for(nNext = 0; nNext < m_TreeItemBackup[itPC_GARBAGE]; nNext++)
	{
		ProduceAFile(str, nNext, m_PC_Garbage[nNext]);
	}
	
}

void CLeftView::OnDestroy() 
{
	CTreeView::OnDestroy();
	
	// TODO: Add your message handler code here

	BackupAllDataToPC();
	
}
void CLeftView::DeletePCSMSFile(LPCTSTR pFoldername)
{
	CString strGetFile;
	CFileFind finder;
	BOOL bret = SetCurrentDirectory(pFoldername);
#ifdef _UNICODE
	BOOL bWorking = finder.FindFile(_T("*.smu"));
#else
	BOOL bWorking = finder.FindFile(_T("*.sm"));
#endif
	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		if(finder.IsDots()  || finder.IsDirectory())
		{

		}
		else
		{
			CString strFileName = finder.GetFilePath();
			int nth = strFileName.ReverseFind('.');
			CString strExt = strFileName.Right(strFileName.GetLength() - nth-1);
#ifdef _UNICODE
			if(strExt.CompareNoCase(_T("smu")) ==0)
#else
			if(strExt.CompareNoCase(_T("sm")) ==0)
#endif
				DeleteFile(finder.GetFilePath());
		}

	}
}

void CLeftView::RecursiveDelete(LPCTSTR pFoldername)
{
	CFileFind ff;
	CString path = pFoldername;
	
	if(path.Right(1) != "\\")
	path += "\\";

	path += "*.*";

	BOOL res = ff.FindFile(path);

	while(res)
	{
		res = ff.FindNextFile();
		path = ff.GetFilePath();
		if (!ff.IsDots() && !ff.IsDirectory())
		{
			CString strFileName = ff.GetFilePath();
			int nth = strFileName.ReverseFind('.');
			CString strExt = strFileName.Right(strFileName.GetLength() - nth-1);
#ifdef _UNICODE
			if(strExt.CompareNoCase(_T("smu")) ==0)
#else
			if(strExt.CompareNoCase(_T("sm")) ==0)
#endif
				DeleteFile(ff.GetFilePath());
		}
		else if( ff.IsDots())
		{	
			continue;
		}
		else if (ff.IsDirectory())
		{
			path = ff.GetFilePath();
			RecursiveDelete(path);
			int ret = RemoveDirectory(path);

			if(ret == 0)
				DWORD e = GetLastError();
		}
	}


}

void CLeftView::xSaveSMS(SMS_PARAM *srcSMS)
{
	CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();
	int nCount = PreListView->GetListCtrl().GetSelectedCount();

	POSITION pos = PreListView->GetListCtrl().GetFirstSelectedItemPosition();

	SMS_PARAM *TempSMS = new SMS_PARAM[nCount];
	memset(TempSMS, 0, sizeof(SMS_PARAM)*nCount);

	for(int i = 0; i < nCount; i++)
	{
		int itemPosition = PreListView->GetListCtrl().GetNextSelectedItem(pos);

		memcpy(&TempSMS[i], &srcSMS[itemPosition], sizeof(SMS_PARAM));
	}

	//Show FileSave Dialog to save sms messages
	CFileDialog fileDlg(FALSE, _T("txt"), _T(""), \
 			OFN_EXPLORER | OFN_PATHMUSTEXIST  \
 		  | OFN_OVERWRITEPROMPT, \
 		 _T("*.txt"));

 
	TCHAR szTmp1[MAX_PATH];
	al_GetSettingString(_T("public"), _T("IDS_DISPLAYMSG"), theApp.m_szRes, szTmp1);	
   	fileDlg.m_ofn.lpstrTitle = szTmp1;
  	fileDlg.m_ofn.lpstrFilter = _T("Text Files(*.txt)\0*.txt\0");
	//fileDlg.m_ofn.lpstrInitialDir = stDirName;
  	
	TCHAR szTmp[MAX_PATH];
  	if(fileDlg.DoModal()==IDOK)
	{
   		CString strFile=fileDlg.GetPathName();
		TCHAR *filename=strFile.GetBuffer(strFile.GetLength());
		strFile.ReleaseBuffer();
		
		FILE *fptr = _tfopen(filename, _T("w+b"));
		if(fptr)
		{	
#ifdef _UNICODE
			char sz[2];
			sz[0]=0xFF;
			sz[1]=0xFE;
			fwrite(sz,1,2,fptr);
#endif
			for(int nNext = 0; nNext < nCount; nNext++)
			{

				CString str;
				if (TempSMS[nNext].whichFolder==0)
				{
				al_GetSettingString(_T("public"), _T("IDS_RECEIVER_"), theApp.m_szRes, szTmp);

				}//090805libaoliu
				else
				al_GetSettingString(_T("public"), _T("IDS_SENDER"), theApp.m_szRes, szTmp);				_fputts(szTmp, fptr);
				_fputts(_T(" : "), fptr);
				_fputts(TempSMS[nNext].TPA, fptr);
				_fputts(_T("\r\n"), fptr);

				al_GetSettingString(_T("public"), _T("IDS_TIME"), theApp.m_szRes, szTmp);
				_fputts(szTmp, fptr);
				_fputts(_T(" : "), fptr);
				_fputts(TempSMS[nNext].TP_SCTS, fptr);
				_fputts(_T("\r\n"), fptr);

				al_GetSettingString(_T("public"), _T("IDS_CONTENT"), theApp.m_szRes, szTmp);
				_fputts(szTmp, fptr);
				_fputts(_T(" : "), fptr);
				_fputts(TempSMS[nNext].TP_UD, fptr);
				_fputts(_T("\r\n\r\n"), fptr);

				
			}

			fclose(fptr);
		}

	}


}

void CLeftView::xFilePrintSMS(void)
{
	CView::OnFilePrint();
}

void CLeftView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
/*	HTREEITEM hCurSelItem = GetTreeCtrl().GetSelectedItem();
	int nItems;
	SMS_PARAM *sms_Info = GetTreeSMSInfo(hCurSelItem, nItems);

	CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();
	int nCount = PreListView->GetListCtrl().GetSelectedCount();

	POSITION pos = PreListView->GetListCtrl().GetFirstSelectedItemPosition();

	SMS_PARAM *TempSMS = new SMS_PARAM[nCount];
	memset(TempSMS, 0, sizeof(SMS_PARAM)*nCount);

	for(int i = 0; i < nCount; i++)
	{
		int itemPosition = PreListView->GetListCtrl().GetNextSelectedItem(pos);

		memcpy(&TempSMS[i], &sms_Info[itemPosition], sizeof(SMS_PARAM));
	}


	CPage*	ps= new CPage(pInfo->m_rectDraw, pDC, MM_TEXT);
	PrintMessages(ps, TempSMS, nCount);*/
	CPage*	ps= new CPage(pInfo->m_rectDraw, pDC, MM_TEXT);

	POSITION pos = m_PrintPageList.FindIndex(pInfo->m_nCurPage-1);
	if(pos)
	{
		CString str = m_PrintPageList.GetAt(pos);
		double	row = 0.0;
		row  =ps->Print(row, 0.0, str);	
	}
	//CTreeView::OnPrint(pDC, pInfo);
}

void CLeftView::PrintMessages(CPage* ps, SMS_PARAM *srcSMS, int nCount)
{
	CString strPrintMsg = "SMSUtility Print\n--------------------------------\n";
	double	row = 0.0;

	
	for(int nNext = 0; nNext < nCount; nNext++)
	{
		strPrintMsg += "Sender  : ";
		strPrintMsg += srcSMS[nNext].TPA;
		strPrintMsg += "\n";
	
		strPrintMsg += "Time    : ";
		strPrintMsg += srcSMS[nNext].TP_SCTS;
		strPrintMsg += "\n";

		strPrintMsg += "Content : ";
		strPrintMsg += srcSMS[nNext].TP_UD;
		strPrintMsg += "\n\n";

		strPrintMsg += "\n--------------------------------\n";
	}

	row  =ps->Print(row, 0.0, strPrintMsg);	
}

BOOL CLeftView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	// TODO: call DoPreparePrinting to invoke the Print dialog box
	m_PrintPageList.RemoveAll();
	pInfo->SetMaxPage(1);
	bool bSuc = DoPreparePrinting(pInfo);
	if(bSuc)
	{
		int Width = GetDeviceCaps(pInfo->m_pPD->m_pd.hDC,HORZRES);
		int Heigh= GetDeviceCaps(pInfo->m_pPD->m_pd.hDC,VERTRES);
		UINT uTextFlags=TEXT_NOCLIP|TEXT_LEFT;	
	
		LOGFONT	      lf;
	    HFONT	      hOldFont, hFont;
		UINT	      fuFormat = DT_NOPREFIX;
		int logpixelsy = GetDeviceCaps(pInfo->m_pPD->m_pd.hDC,LOGPIXELSY);
	    lf.lfHeight = -MulDiv(10,logpixelsy, 72);
	    lf.lfWidth = 0;
	    lf.lfEscapement = 0;
	    lf.lfOrientation = 0;
	    lf.lfWeight = (uTextFlags & TEXT_BOLD) ? FW_BOLD : FW_NORMAL;
	    lf.lfItalic = (uTextFlags & TEXT_ITALIC) ? 1 : 0;
	    lf.lfUnderline = (uTextFlags & TEXT_UNDERLINED) ? 1 : 0;
	    lf.lfStrikeOut = (uTextFlags & TEXT_STRIKEOUT) ? 1 : 0;
	    lf.lfCharSet = DEFAULT_CHARSET;
	    lf.lfOutPrecision = OUT_STROKE_PRECIS;
		lf.lfClipPrecision = CLIP_STROKE_PRECIS;
		
	    lf.lfQuality = PROOF_QUALITY;
	    lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	    lstrcpy((LPTSTR)lf.lfFaceName, _T("Tahoma"));
	    hFont = ::CreateFontIndirect(&lf);

	    hOldFont = (HFONT)::SelectObject(pInfo->m_pPD->m_pd.hDC, hFont);

	    fuFormat |= (uTextFlags & TEXT_CENTER) ? DT_CENTER :
					(uTextFlags & TEXT_RIGHT)  ? DT_RIGHT  :DT_LEFT;

		if(uTextFlags & TEXT_VCENTER) 
			fuFormat|=DT_VCENTER;
			
	      //expand tabs if indicated
	    if (uTextFlags & TEXT_EXPANDTABS)
		fuFormat |=  DT_EXPANDTABS;

	      //set text clipping
	    if (uTextFlags & TEXT_NOCLIP)
		fuFormat |=  DT_NOCLIP;

	      //if this is a single line then set alignment to bottom
	    if (uTextFlags & TEXT_SINGLELINE)
			fuFormat |=  DT_SINGLELINE;
	    else
			fuFormat |=  DT_WORDBREAK;

		//ps->pDC->DrawText(ps->Text, -1,&rc, fuFormat | DT_CALCRECT);
		HTREEITEM hCurSelItem = GetTreeCtrl().GetSelectedItem();
		int nItems;
		SMS_PARAM *sms_Info = GetTreeSMSInfo(hCurSelItem, nItems);

		CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();
		int nCount = PreListView->GetListCtrl().GetSelectedCount();

		POSITION pos = PreListView->GetListCtrl().GetFirstSelectedItemPosition();

		CString strPrintMsg = _T("");
		CString strPage= "SMSUtility Print\n--------------------------------\n";;;
		for(int i = 0; i < nCount; i++)
		{
			int itemPosition = PreListView->GetListCtrl().GetNextSelectedItem(pos);

			strPrintMsg += "Sender  : ";
			strPrintMsg += sms_Info[itemPosition].TPA;
			strPrintMsg += "\n";
		
			strPrintMsg += "Time    : ";
			strPrintMsg += sms_Info[itemPosition].TP_SCTS;
			strPrintMsg += "\n";

			strPrintMsg += "Content : ";
			strPrintMsg += sms_Info[itemPosition].TP_UD;
			strPrintMsg += "\n\n";

			strPrintMsg += "\n--------------------------------\n";
			CRect rcText;
			rcText.SetRect(0,0,Width,Heigh);

			CString strText;
			strText.Format(_T("%s%s"),strPage,strPrintMsg);
			::DrawText(pInfo->m_pPD->m_pd.hDC, strText, -1, &rcText,  fuFormat |DT_CALCRECT);
			if(rcText.Height() >Heigh)
			{
				if(strPage.GetLength()>0)
					m_PrintPageList.AddTail(strPage);

				strPage.Format(_T("%s"),strPrintMsg);
				strPrintMsg =_T("");
			}
			else
			{
				strPage= strText;
				strPrintMsg =_T("");
			}
		}
		if(strPage.GetLength()>0)
			m_PrintPageList.AddTail(strPage);
		
		
		pInfo->SetMaxPage(m_PrintPageList.GetCount());
		pInfo->m_pPD->m_pd.nToPage = m_PrintPageList.GetCount();
	}
	return bSuc;
}

void CLeftView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
	CTreeView::OnPrepareDC(pDC, pInfo);
}

void CLeftView::xDeleteSMS(HTREEITEM hCurSelItem, SMS_PARAM *srcSMS)
{
	TCHAR str1[MAX_PATH], str2[MAX_PATH];
	al_GetSettingString(_T("public"), _T("IDS_DELETE_MESSAGES"), theApp.m_szRes, str1);
	al_GetSettingString(_T("public"), _T("IDS_DELETE_DIA"), theApp.m_szRes, str2);
	//int ret = AfxMessageBox(str, MB_YESNO);
	int ret = MessageBox(str1, str2, MB_YESNO);

	if(ret == IDYES)
	{
		int nCurSelItem = GetSelectItem(hCurSelItem);

		if(hCurSelItem == gPCGarbageItem)
		{
			DeletePCSMS(srcSMS, nCurSelItem);
		}
		else if(hCurSelItem == gPCInboxItem 
			|| hCurSelItem == gPCOutboxItem 
			|| hCurSelItem == gPCSendBKItem 
			|| hCurSelItem == gPCDraftItem 
			/*|| hCurSelItem == gPCGarbageItem*/ )
		{
			//DeletePCSMS(srcSMS, nCurSelItem);
			CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();
			int nMoveItemCounts = PreListView->GetListCtrl().GetSelectedCount();
			MoveItemsToPC(srcSMS, m_PC_Garbage, nCurSelItem, itPC_GARBAGE, nMoveItemCounts);
		}
		else
		{
			if(theApp.nIsConnected == true)
			{
				//DeleteMobileSMS(srcSMS, nCurSelItem);
				CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();
				int nMoveItemCounts = PreListView->GetListCtrl().GetSelectedCount();
				MoveItemsToPC(srcSMS, m_PC_Garbage, nCurSelItem, itPC_GARBAGE, nMoveItemCounts);
			}
				
		}

		// IDS_DRAFT -> IDS_SENDBK
		UpdateOneTreeItem(hCurSelItem, nCurSelItem);
		UpdateOneTreeItem(gPCGarbageItem, itPC_GARBAGE);
		
		CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();
		PreListView->ShowItems(srcSMS, nCurSelItem, m_TreeItemBackup[nCurSelItem]);
	}
}

void CLeftView::DeletePCSMS(SMS_PARAM *srcSMS, int nCurSelItem)
{
	CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();
	int nCount = PreListView->GetListCtrl().GetSelectedCount();

	SMS_PARAM *TempSMS = new SMS_PARAM[SMS_ItemCount];
	memset(TempSMS, 0, sizeof(SMS_PARAM)*SMS_ItemCount);
	memcpy(TempSMS, srcSMS, sizeof(SMS_PARAM)*SMS_ItemCount);

	POSITION pos = PreListView->GetListCtrl().GetFirstSelectedItemPosition();	
	for(int i = 0; i < nCount; i++)
	{
		int itemPosition = PreListView->GetListCtrl().GetNextSelectedItem(pos);
	
		TempSMS[itemPosition].index = -1;	
	}


	int nTotal = m_TreeItemBackup[nCurSelItem];
	memset(srcSMS, 0, sizeof(SMS_PARAM)*SMS_ItemCount);
	int nNextItem = 0;
	for(int k = 0; k < nTotal; k++)
	{
		if(TempSMS[k].index != -1)
		{
			memcpy(&srcSMS[nNextItem], &TempSMS[k], sizeof(SMS_PARAM));
			nNextItem++;
		}
	}

	m_TreeItemBackup[nCurSelItem] -= nCount;

}


int CLeftView::DeleteMobileOneSMS(int nNeed)
{
	int DeleteSMSfolderType = 0;

#ifdef NOKIA
	if(s_ActiveDlg->m_Mobile_SMS[s_ActiveDlg->m_arrayDeletePosition[g_nDeletePosition]].whichFolder == 1) // InBox Folder(1)
		DeleteSMSfolderType = 1;
#endif

#ifdef ASUSM303
		DeleteSMSfolderType = s_ActiveDlg->m_Mobile_SMS[s_ActiveDlg->m_arrayDeletePosition[g_nDeletePosition]].whichFolder;
#endif

	int ret = ANWDeleteSMSData( s_ActiveDlg->m_Mobile_SMS[s_ActiveDlg->m_arrayDeletePosition[g_nDeletePosition]].memType, 
								DeleteSMSfolderType, 
								s_ActiveDlg->m_Mobile_SMS[s_ActiveDlg->m_arrayDeletePosition[g_nDeletePosition]].index);


	static BOOL isbFirst=false;
	if(ret != 1)
	{

		if ( !isbFirst)//081024libaoliu
		{
			//s_ActiveDlg->xErrorHandling(ret);
		isbFirst=true;
		TCHAR str1[MAX_PATH];
		al_GetSettingString(_T("public"), _T("IDS_DELETE_FAIL"), theApp.m_szRes, str1);
		AfxMessageBox(str1);
		}
		if(ret == Anw_TRIAL_LIMITEDFUNCTION)
			return false ;
		return true;
	}
     isbFirst=false;
	s_ActiveDlg->m_Mobile_SMS[s_ActiveDlg->m_arrayDeletePosition[g_nDeletePosition]].index = -1;

	g_nDeletePosition--;

	return true;
}


void CLeftView::DeleteMobileSMS(SMS_PARAM *srcSMS, int nCurSelItem)
{
	CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();
	int nCount = PreListView->GetListCtrl().GetSelectedCount();

	memset(m_Mobile_SMS, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	memcpy(m_Mobile_SMS, srcSMS, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);

	POSITION pos = PreListView->GetListCtrl().GetFirstSelectedItemPosition();

	memset(&m_arrayDeletePosition, 0, SMS_MOBILE_ItemCount);
	for(int i = 0; i < nCount; i++)
	{
		m_arrayDeletePosition[i] = PreListView->GetListCtrl().GetNextSelectedItem(pos);	
	}


	// Delete thread
	CProcessDlg dlg;
	if(nCount >= 0)
	{
		g_nDeletePosition = nCount-1;
		dlg.SetProgressInfo( CLeftView::DeleteMobileOneSMS, nCount);
		dlg.SetLoopCount(nCount);
		m_bRuningDlg = true;
		dlg.DoModal();
		m_bRuningDlg = false;
	}

	// Delete thread


	int nTotal = m_TreeItemBackup[nCurSelItem];
	memset(srcSMS, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	int nNextItem = 0;
	for(int k = 0; k < nTotal; k++)
	{
		if(m_Mobile_SMS[k].index != -1)
		{
			memcpy(&srcSMS[nNextItem], &m_Mobile_SMS[k], sizeof(SMS_PARAM));
			nNextItem++;
		}
	}

	m_TreeItemBackup[nCurSelItem] -= nCount;
	
}

void CLeftView::DeleteMobileSMS_from_trash(SMS_PARAM *srcSMS, int nCurSelItem)
{
	CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();
	int nCount = PreListView->GetListCtrl().GetSelectedCount();

	memset(m_Mobile_SMS, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	memcpy(m_Mobile_SMS, srcSMS, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);

	POSITION pos = PreListView->GetListCtrl().GetFirstSelectedItemPosition();

	memset(&m_arrayDeletePosition, 0, SMS_MOBILE_ItemCount);
	for(int i = 0; i < nCount; i++)
	{
		m_arrayDeletePosition[i] = PreListView->GetListCtrl().GetNextSelectedItem(pos);	
	}


	// Delete thread
	CProcessDlg dlg;
	if(nCount >= 0)
	{
		g_nDeletePosition = nCount-1;
		dlg.SetProgressInfo( CLeftView::DeleteMobileOneSMS, nCount);
		dlg.SetLoopCount(nCount);
		m_bRuningDlg = true;
		dlg.DoModal();
		m_bRuningDlg = false;
	}

	// Delete thread
	
}

static int gGetDataFromM303 = 0;
void CLeftView::xRefreshSMS()
{
	if(theApp.nIsConnected == false)
	{
		//AfxMessageBox("Disconnected");
		return ;
	}
	else
	{
		//GetMobileSMSInfo();
#ifdef ASUSM303
		gGetDataFromM303 = 0;
		ANWInitSMS();
		memset(&mfolders, 0, sizeof(GSM_SMSFolders));
		ANWGetMobileSMSFolders(&mfolders);
#endif
	
		memset(m_SIMSMS_Inbox, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
		m_TreeItemBackup[itMOBILE_SIMCARD_INBOX] = 0;
		memset(m_SIMSMS_Outbox, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
		m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX] = 0;
		memset(m_HandsetSMS_Inbox, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
		m_TreeItemBackup[itMOBILE_HANDSET_INBOX] = 0;
		memset(m_HandsetSMS_Outbox, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
		m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX] = 0;
#ifdef ASUSM303
		memset(m_HandsetSMS_Folder1, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
		m_TreeItemBackup[itMOBILE_HANDSET_OTHER1] = 0;
		memset(m_HandsetSMS_Folder2, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
		m_TreeItemBackup[itMOBILE_HANDSET_OTHER2] = 0;
		memset(m_HandsetSMS_Folder3, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
		m_TreeItemBackup[itMOBILE_HANDSET_OTHER3] = 0;
		memset(m_HandsetSMS_Folder4, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
		m_TreeItemBackup[itMOBILE_HANDSET_OTHER4] = 0;
		//memset(m_HandsetSMS_Folder5, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
		//m_TreeItemBackup[itMOBILE_HANDSET_OTHER5] = 0;
#endif

		s_ActiveDlg->xDownloadSMS();
	
		UpdateAllTreeItemFirst();

		HTREEITEM hCurSelItem = GetTreeCtrl().GetSelectedItem();
		int nCurSelItem = GetSelectItem(hCurSelItem);

		UpdateOneTreeItem(gPCDraftItem, itPC_DRAFT);

		int nItems;
		SMS_PARAM *showInfo = GetTreeSMSInfo(hCurSelItem, nItems);
		CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();
		PreListView->ShowItems(showInfo, nCurSelItem, m_TreeItemBackup[nCurSelItem]);
	}
}

void CLeftView::xStartConnect(void) 
{
	int ret = InitialLibrary();
	if(ret == false)
	{
		TCHAR str1[MAX_PATH];
		al_GetSettingString(_T("public"), _T("IDS_DISCONNECT_"), theApp.m_szRes, str1);
		afxMessageBox(str1);
	}
	else
	{
		ret= OpenMobileConnect();
		if(ret == false)
		{
			TCHAR str1[MAX_PATH];
			al_GetSettingString(_T("public"), _T("IDS_DISCONNECT_"), theApp.m_szRes, str1);
			afxMessageBox(str1);
		}

		UpdateAllTreeItemFirst();
	}

}

// CColorTreeCtrl::PaintParentLine
//
// This method recursively paints the parent item lines (vertical).
// The lines are drawn with the pen and brush selected into the HDC.
//
void CLeftView::PaintParentLine(HTREEITEM hParent, HDC hDC, RECT &rc)
{
	HTREEITEM hGrand;

	//
	// Check if the parent has a parent itself and process it
	//
	hGrand = GetTreeCtrl().GetParentItem(hParent);
	if(hGrand)
		PaintParentLine(hGrand, hDC, rc);


	//
	// Check if the parent has a sibling. If so, draw the vertical line
	//
	if(GetTreeCtrl().GetNextSiblingItem(hParent) && (m_dwStyle & TVS_HASLINES))
	{
		//
		// Now, check if this is a root item. If it is, we have to make
		// sure that TVS_LINESATROOT is enabled
		//
		if(!hGrand)
		{
			if(m_dwStyle & TVS_LINESATROOT)
				LineVert(hDC, m_nIndent / 2 + rc.left, rc.top, rc.bottom, bDot);
		}
		else
			LineVert(hDC, m_nIndent / 2 + rc.left, rc.top, rc.bottom, bDot);
	}

	//
	// Advance the drawing position
	//
	if(hGrand)
		rc.left += m_nIndent;
}
    


// CColorTreeCtrl::PaintItemLines
//
// Paints the lines of an item
//
void CLeftView::PaintItemLines(HTREEITEM hItem, HTREEITEM hParent, HDC hDC, RECT &rc)
{
	int			x,
				y,
				xm	= m_nIndent / 2 + 4,
				ym	= (rc.bottom - rc.top) / 2;

	x	= rc.left + m_nIndent / 2;
	y	= rc.top;
	if(GetTreeCtrl().GetPrevSiblingItem(hItem) || hParent)
	{
		if(!hParent)								// Root node?
		{
			if(m_dwStyle & TVS_LINESATROOT)			// Lines at root?
				LineVert(hDC, x, y, y + ym +2, bDot);		// Connect to prev / parent
		}
		else
			LineVert(hDC, x, y, y + ym+2, bDot);			// Connect to prev / parent
	}
	
	y += ym;
	LineHorz(hDC, x, x + xm + 1, y+2, bDot);				// Connect to text / icon
	
	if(GetTreeCtrl().GetNextSiblingItem(hItem))
	{
		if(!hParent)								// Root node?
		{
			if(m_dwStyle & TVS_LINESATROOT)			// Lines at root?
				LineVert(hDC, x, y+3, y + ym, bDot);		// Connect to next
		}
		else
			LineVert(hDC, x, y+3, y + ym, bDot);			// Connect to next
	}
}


// CColorTreeCtrl::PaintButton
//
// Draws the open / close button
//
void CLeftView::PaintButton(HDC hDC, RECT &rc, BOOL bExpanded)
{
	if(m_hIconBtn[0])
	{
		::DrawIconEx(hDC, 
					 rc.left + (m_nIndent - 16) / 2, 
					 rc.top, 
					 m_hIconBtn[bExpanded ? 1 : 0],
					 16,
					 16,
					 0,
					 NULL,
					 DI_NORMAL);
	}
	else
	{
		HPEN		hBoxPen,
					hMrkPen,
					hOldPen;
		HBRUSH		hNewBrush,
					hOldBrush;
		
		int			h	= rc.bottom - rc.top,
					x	= rc.left + (m_nIndent - 9) / 2,
					y	= rc.top + (h - 9) / 2 + 1;

		hBoxPen		= ::CreatePen(PS_SOLID, 1, m_crLine);
		hMrkPen		= ::CreatePen(PS_SOLID, 1, RGB(  0,   0,   0));
		hNewBrush	= ::CreateSolidBrush(RGB(255, 255, 255));

		hOldPen		= (HPEN)	::SelectObject(hDC, hBoxPen);
		hOldBrush	= (HBRUSH)	::SelectObject(hDC, hNewBrush);

		//
		// Draw the box
		//
		::Rectangle(hDC, x, y, x+9, y+9);

		//
		// Now, the - or + sign
		//
		::SelectObject(hDC, hMrkPen);

		LineHorz(hDC, x + 2, x + 7, y + 4, bSolid);			// '-'

		if(!bExpanded)
			LineVert(hDC, x + 4, y + 2, y + 7, bSolid);		// '+'

		::SelectObject(hDC, hOldPen);
		::SelectObject(hDC, hOldBrush);

		::DeleteObject(hMrkPen);
		::DeleteObject(hBoxPen);
		::DeleteObject(hNewBrush);
	}

}



void CLeftView::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
   	NMTVCUSTOMDRAW*	pCD = (NMTVCUSTOMDRAW*)pNMHDR;
   	DWORD			dwDrawStage;

   	*pResult = CDRF_DODEFAULT;

	//SetFontSize();

   	dwDrawStage = pCD->nmcd.dwDrawStage;
	

   	if(dwDrawStage == CDDS_PREPAINT)
   	{
   		//
   		// This is the beginning of the drawing phase.
   		// Cache some properties for later.
   		//
   		m_nIndent	= GetTreeCtrl().GetIndent();// / 3;
   		m_dwStyle	= GetTreeCtrl().GetStyle();
   		m_hImgList	= TreeView_GetImageList(m_hWnd, TVSIL_NORMAL);

   		*pResult = CDRF_NOTIFYITEMDRAW;
   	}
   	else if(dwDrawStage == CDDS_ITEMPREPAINT)
   	{
		HDC			hDC			= pCD->nmcd.hdc;
   		HPEN		hLinPen,
   					hOldPen;
   		HBRUSH		hBackBrush,
   					hOldBrush;
   		HTREEITEM	hItem		= (HTREEITEM)pCD->nmcd.dwItemSpec,
   					hParent		= GetTreeCtrl().GetParentItem(hItem);
   		RECT		rc			= pCD->nmcd.rc;
   		TV_DISPINFO	tvdi;
   		RECT		rcText;
   		TCHAR		szText[1024];
		if(rc.bottom - rc.top == 0 && rc.right - rc.left ==0)
			return;

		//GetTreeCtrl().EnableToolTips(false);



		TCHAR  szProfile[_MAX_PATH], szAppPath[_MAX_PATH];
		al_GetModulePath(NULL, szAppPath);
		wsprintf(szProfile, _T("%s%s"), szAppPath, _T("skin\\default\\font.ini"));
		TCHAR  szFontSize[_MAX_PATH];
		al_GetSettingString(_T("Font"), _T("fontsize"), szProfile, szFontSize);
		//int nFonetSize = (atoi(szFontSize)*4)/3;
		int nFonetSize = (_ttoi(szFontSize));
		//if(nFonetSize > 0)
		//	nFonetSize *= -1;




		SCROLLINFO si;
		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		GetScrollInfo(SB_HORZ, &si);
		rc.left -= si.nPos;



		// Create a dotted monochrome bitmap
   		hLinPen = ::CreatePen(PS_DOT, 1, m_crLine);

		hOldPen		= (HPEN)	::SelectObject(hDC, hLinPen);
   		hOldBrush	= (HBRUSH)	::SelectObject(hDC, m_brush);
		HFONT hOldFont = reinterpret_cast<HFONT>(::SelectObject(hDC,m_hFont));

   		//
   		// Draw the parent lines, if any
   		//
   		if(hParent)
   			PaintParentLine(hParent, hDC, rc);

   		//
   		// Draw the lines connecting to the previous and next items, if any
   		//
		if(hParent)
   			if(m_dwStyle & TVS_HASLINES)
   				PaintItemLines(hItem, hParent, hDC, rc);

   		//
   		// Get the item information to draw the current item
   		//
   		tvdi.item.mask			= TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | 
   								  TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_TEXT;
   		tvdi.item.hItem			= hItem;
   		tvdi.item.pszText		= szText;
   		tvdi.item.cchTextMax	= 1024;
   		if(!GetTreeCtrl().GetItem(&tvdi.item))
   			goto error_exit;		// Exit silently (should never happen, though)

   		//
   		// Now, check for callback items
   		//
   		if(tvdi.item.iImage			== I_IMAGECALLBACK ||
   		   tvdi.item.iSelectedImage	== I_IMAGECALLBACK ||
   		   tvdi.item.pszText		== LPSTR_TEXTCALLBACK )
   		{
   			HWND	hWndParent;

   			hWndParent = ::GetParent(m_hWnd);
   			if(hWndParent)
   			{
   				tvdi.hdr.hwndFrom	= m_hWnd;
   				tvdi.hdr.idFrom		= ::GetDlgCtrlID(m_hWnd);
   				tvdi.hdr.code		= TVN_GETDISPINFO;

   				::SendMessage(hWndParent, WM_NOTIFY, tvdi.hdr.idFrom,
   								(LPARAM)&tvdi);
   			}
   		}
    	rc.top += 5; // peggy
   		
   		//
   		// Paint the buttons, if any
   		//
		if(hParent)
		{
   			if(m_dwStyle & TVS_HASBUTTONS)
   			{
   				if(tvdi.item.cChildren == 1)
   					PaintButton(hDC, rc, tvdi.item.state & TVIS_EXPANDED);
   				else if(tvdi.item.cChildren == I_CHILDRENCALLBACK)
   					PaintButton(hDC, rc, FALSE);
   			}
		}

   		//
   		// If we have buttons or line, we must make room for them
   		//
		if(hParent)	// Leo
   			if(m_dwStyle & (TVS_HASBUTTONS | TVS_HASLINES))
   				rc.left += m_nIndent;

   		//
   		// Check if we have any normal icons to draw
   		//
		rc.left += 4;
   		if(m_hImgList)
   		{
   			int	iImage,
   				cx, cy;

   			if(pCD->nmcd.uItemState & CDIS_SELECTED)
   				iImage = tvdi.item.iSelectedImage;
   			else
   				iImage = tvdi.item.iImage;

   			ImageList_Draw(m_hImgList, iImage, hDC, rc.left, rc.top, ILD_NORMAL);
   			
   			ImageList_GetIconSize(m_hImgList, &cx, &cy);
   			rc.left += cx;
   		}

   		rc.left += 4;

   		//
   		// Calculate the text drawing rectangle
   		//
   		rcText = rc;
		rcText.left		-= 2;
   		rcText.right	+= 2;
   		rcText.bottom	+= 1;
   		::DrawText(hDC, szText, -1, &rcText, DT_LEFT | DT_NOPREFIX | 
   					DT_SINGLELINE | DT_VCENTER| DT_CALCRECT);

   		//
   		// Clear the background
   		//
   		if(pCD->nmcd.uItemState & CDIS_FOCUS)
   		{
   			hBackBrush = ::CreateSolidBrush(RGB(59, 74, 140));

   			::SetTextColor	(hDC, RGB(255, 255, 255));
   			::SetBkColor	(hDC, RGB(59, 74, 140));
   		}
		else if(pCD->nmcd.uItemState & CDIS_SELECTED)
   		{
			hBackBrush = ::CreateSolidBrush(RGB(177,177,177));

   			::SetTextColor	(hDC, RGB(255, 255, 255));
			::SetBkColor	(hDC, RGB(177,177,177));
   		}
   		else
   		{
   			hBackBrush = ::CreateSolidBrush(m_crBack);

   			::SetTextColor	(hDC, m_crText);
   			::SetBkColor	(hDC, m_crBack);
   		}

		// Draw Text's Retangle // leo
		//RECT rcFillText = rcText;
		/*rcFillText.right	+= 4;
		rcFillText.top = ((rcFillText.bottom-nFonetSize));
   		::FillRect(hDC, &rcFillText, hBackBrush);	*/
   		::DeleteObject(hBackBrush);

   		//
   		// Now, draw the text
   		//
		RECT focusRect = rc;
   		::DrawText(hDC, szText, -1, &focusRect, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
		//::DrawText(hDC, szText, -1, &rc, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);

   		//
   		// Draw the focus rect
   		//
   		if(pCD->nmcd.uItemState & CDIS_FOCUS)	// leo
		{
			//rcText.top += (rcText.bottom - rcText.top)/2;
			TEXTMETRIC lptm;
			GetTextMetrics(hDC, &lptm);

			RECT rcFocusText = rcText;
			rcFocusText.left += 1;
			rcFocusText.right += 3;
			//rcFocusText.bottom += 1;
			rcFocusText.top = ((rcFocusText.bottom - lptm.tmHeight) - 1);
   			::DrawFocusRect(hDC, &rcFocusText);
		}

   error_exit:

   		//
   		// Clean up
   		//
   		::SelectObject(hDC, hOldBrush);
   		::SelectObject(hDC, hOldPen);
   		
   		::DeleteObject(hLinPen);
		::SelectObject(hDC,hOldFont);

   		*pResult = CDRF_SKIPDEFAULT;
   	}
	
}

void CLeftView::LineVert(HDC hDC, int x, int y0, int y1, bool nLine)
{
	if(nLine == bSolid)
	{
		POINT	line[2] = {{x,y0},{x,y1}};
		::Polyline(hDC, line, 2);
	}
	else
	{
		for(int i = y0; i < y1; i+=2)
			::SetPixel(hDC, x, i, RGB(0,0,0));
	}
}

void CLeftView::LineHorz(HDC hDC, int x0, int x1, int y, bool nLine)
{
	if(nLine == bSolid)
	{
		POINT	line[2] = {{x0,y},{x1,y}};
		::Polyline(hDC, line, 2);
	}
	else
	{
		for(int i = x0; i < x1; i+=2)
			::SetPixel(hDC, i, y, RGB(0,0,0));
	}
}


bool CLeftView::XGetConnectStatus(TCHAR *str)
{
//	static bool bFirstTime = true;
	TCHAR szTmp[MAX_PATH];	
	memset(&szTmp, 0, MAX_PATH);

	if(theApp.nIsConnected)
		al_GetSettingString(_T("public"), _T("IDS_CONNECTED"), theApp.m_szRes, szTmp);
	else
		al_GetSettingString(_T("public"), _T("IDS_DISCONNECTED"), theApp.m_szRes, szTmp);

/*	if(bFirstTime && !theApp.nIsConnected)	
	{
		strcpy( str, _T("") );
	}
	else	{
		bFirstTime = false;
		sprintf(str, szTmp, m_strMobileName);
	}*/
	if(m_bDisplayConnectStatus)
	{
		wsprintf(str, szTmp, m_strMobileName);
	}
	else
		_tcscpy( str, _T("") );

	return true;
}

void CLeftView::xSortItem(CString cDataType,bool bSortAscending)
{
	int items;
	SMS_PARAM *sortSmsInfo = GetTreeSMSInfo(GetTreeCtrl().GetSelectedItem(), items);

	CPrevView *PreListView = ((CMainFrame*)GetParentFrame())->GetRightPane();

	
	int  first = 0;
    int  last  = items-1;  
	
	qsort(sortSmsInfo, first, last, cDataType,bSortAscending);

	PreListView->ShowItems(sortSmsInfo, 0, items);
}

void CLeftView::qsort(SMS_PARAM input[], int first, int last, CString cDataType,bool bSortAscending)
{
	int  split_point;
	
	if (first < last) 
	{
		split(input, first, last, &split_point, cDataType,bSortAscending);
		if (split_point - first < last - split_point) 
		{
			qsort(input, first, split_point-1, cDataType,bSortAscending);
			qsort(input, split_point+1, last, cDataType,bSortAscending);
		}
		else 
		{
			qsort(input, split_point+1, last, cDataType,bSortAscending);
			qsort(input, first, split_point-1, cDataType,bSortAscending);
		}
	}
}

void CLeftView::split(SMS_PARAM input[], int first, int last, int *split_point, CString cDataType,bool bSortAscending)
{
	int current_split,  unknown;
	CString x;
	
	if(cDataType == _T("C"))
		x = input[first].TP_UD;
	else if(cDataType == _T("S"))
		x = input[first].TPA;
	else if(cDataType == _T("T"))
		x = input[first].TP_SCTS;

	current_split = first;
	for (unknown = first+1; unknown <= last; unknown++)
	{
		int nCompare = 0;
		if(cDataType == _T("C"))
			nCompare = _tcscmp(input[unknown].TP_UD, x);
		else if(cDataType == _T("S"))
			nCompare = _tcscmp(input[unknown].TPA, x);
		else if(cDataType == _T("T"))
			nCompare = _tcscmp(input[unknown].TP_SCTS, x);

		if (bSortAscending && nCompare < 0) 
		{
			(current_split)++;
			swap(&input[current_split], &input[unknown]);
		}
		else if (bSortAscending == false && nCompare > 0) 
		{
			(current_split)++;
			swap(&input[current_split], &input[unknown]);
		}
	}
		
	swap(&input[first], &input[current_split]);
	*split_point = current_split;
}

void CLeftView::swap(SMS_PARAM *src, SMS_PARAM *dst)
{
	SMS_PARAM smsInfo;
	
	memcpy(&smsInfo, src, sizeof(SMS_PARAM));
	memcpy(src, dst, sizeof(SMS_PARAM));
	memcpy(dst, &smsInfo, sizeof(SMS_PARAM));
}



BOOL CLeftView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
	GetClientRect(rect);

	CBrush brush(RGB(255, 255, 255));

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(SCROLLINFO));
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS;
	GetScrollInfo(SB_HORZ, &si);
	rect.left -= si.nPos;

	pDC->FillRect(&rect, &brush);

	return CTreeView::OnEraseBkgnd(pDC);
}

int CLeftView::GetAllSMSInfo(void)	
{
#ifdef ASUSM303
	memset(&mfolders, 0, sizeof(GSM_SMSFolders));
	ANWGetMobileSMSFolders(&mfolders);

	CreateNewTreeList();
#endif

//#ifdef NOKIA
	if(theApp.nIsConnected == true)  // ipng 20041130
		s_ActiveDlg->xDownloadSMS();
//#endif

	if(m_bFirst == true)
	{
		GetPCSMSInfo();
		m_bFirst = false;
	}

	UpdateAllTreeItemFirst();


	// Leo add to Show reget messget 2005.01.28
	HTREEITEM hCurSelItem = GetTreeCtrl().GetSelectedItem();
	int nCurSelItem = GetSelectItem(hCurSelItem);

	int nItems;
	SMS_PARAM *showInfo = GetTreeSMSInfo(hCurSelItem, nItems);
	gPreListView->ShowItems(showInfo, nCurSelItem, m_TreeItemBackup[nCurSelItem]);
	// Leo add to Show reget messget 2005.01.28

	return true;
}


#ifdef NOKIA
int CLeftView::GetMobileSMSInfo(void)
{
	
	SMS_Data_Strc *sms_Data_Strc = new SMS_Data_Strc;
	memset(sms_Data_Strc, 0, sizeof(SMS_Data_Strc));

	int FolderType = 1;//LLD_FOLDER_INBOX;
	int MemType = 2;//LLD_MEMORY_TYPE_HANDSET;

	int ret = ANWGetSMSOneData(MemType, FolderType, sms_Data_Strc, m_bStart);
	if(ret != 1)
	{
		if(ret == Anw_EMPTY)
			return -1;
		TCHAR str1[MAX_PATH];
		al_GetSettingString(_T("public"), _T("IDS_GETSMS_FAIL"), theApp.m_szRes, str1);
		afxMessageBox(str1);
		return false;
	}
	else
	{
		m_bStart = false;
		
		if(sms_Data_Strc->memType == MEM_SM) // SIM CARD
		{
			if(sms_Data_Strc->whichFolder == 1) // Inbox
			{
				memcpy(&m_SIMSMS_Inbox[m_TreeItemBackup[itMOBILE_SIMCARD_INBOX]], sms_Data_Strc, sizeof(SMS_Data_Strc));
				//m_SIMSMS_Inbox[m_TreeItemBackup[itMOBILE_SIMCARD_INBOX]].position = m_TreeItemBackup[itMOBILE_SIMCARD_INBOX];
				m_TreeItemBackup[itMOBILE_SIMCARD_INBOX]++;
			}
			else if(sms_Data_Strc->whichFolder == 0) // Outbox
			{
				memcpy(&m_SIMSMS_Outbox[m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX]], sms_Data_Strc, sizeof(SMS_Data_Strc));
				//m_SIMSMS_Outbox[m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX]].position = m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX];
				m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX]++;
			}	
		}
		else if(sms_Data_Strc->memType == MEM_ME) // HandSet
		{
			if(sms_Data_Strc->whichFolder == 1) // Inbox
			{
				memcpy(&m_HandsetSMS_Inbox[m_TreeItemBackup[itMOBILE_HANDSET_INBOX]], sms_Data_Strc, sizeof(SMS_Data_Strc));
				//m_HandsetSMS_Inbox[m_TreeItemBackup[itMOBILE_HANDSET_INBOX]].position = m_TreeItemBackup[itMOBILE_HANDSET_INBOX];
				m_TreeItemBackup[itMOBILE_HANDSET_INBOX]++;
			}
			else if(sms_Data_Strc->whichFolder == 0) // Outbox
			{
				memcpy(&m_HandsetSMS_Outbox[m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX]], sms_Data_Strc, sizeof(SMS_Data_Strc));
				//m_HandsetSMS_Outbox[m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX]].position = m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX];
				m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX]++;
			}
		}
	
	}


	delete sms_Data_Strc;

	return true;
}
#endif


#ifdef ASUSM303
int CLeftView::GetMobileSMSInfo(void)
{
	SMS_Data_Strc *sms_Data_Strc = new SMS_Data_Strc;
	memset(sms_Data_Strc, 0, sizeof(SMS_Data_Strc));

	int FolderType = LLD_FOLDER_INBOX;
	int MemType = LLD_MEMORY_TYPE_HANDSET;

	//ASUS_GetData[gGetDataFromM303];
	while(1)
	{
		if(gGetDataFromM303 == 9)
		{
			delete sms_Data_Strc;
			return true;
		}

		if(asus_GetData[gGetDataFromM303].Items == 0)
		{
			gGetDataFromM303++;
			m_bStart = true;
		}
		else
			break;
	}

	int ret = ANWGetSMSOneData(asus_GetData[gGetDataFromM303].MemType, 
							   asus_GetData[gGetDataFromM303].FolderType, 
							   sms_Data_Strc, m_bStart);
	xErrorHandling(ret);

	if(ret != 1)
	{
		/*TCHAR str1[MAX_PATH];
		al_GetSettingString(_T("public"), "IDS_GETSMS_FAIL", theApp.m_szRes, str1);
		AfxMessageBox(str1);*/

		delete sms_Data_Strc;
		return false;
	}
	else
	{
		m_bStart = false;
		asus_GetData[gGetDataFromM303].Items--;
		
		if(sms_Data_Strc->memType == LLD_MEMORY_TYPE_SIM) // SIM CARD
		{
			switch(sms_Data_Strc->whichFolder)
			{
			case LLD_FOLDER_INBOX:
				memcpy(&m_SIMSMS_Inbox[m_TreeItemBackup[itMOBILE_SIMCARD_INBOX]], sms_Data_Strc, sizeof(SMS_Data_Strc));
				m_TreeItemBackup[itMOBILE_SIMCARD_INBOX]++;
				break;

			case LLD_FOLDER_OUTBOX:
				memcpy(&m_SIMSMS_Outbox[m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX]], sms_Data_Strc, sizeof(SMS_Data_Strc));
				m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX]++;
				break;
			};
		
		}
		else if(sms_Data_Strc->memType == LLD_MEMORY_TYPE_HANDSET) // HandSet
		{
			switch(sms_Data_Strc->whichFolder)
			{
			case LLD_FOLDER_INBOX:
				memcpy(&m_HandsetSMS_Inbox[m_TreeItemBackup[itMOBILE_HANDSET_INBOX]], sms_Data_Strc, sizeof(SMS_Data_Strc));
				m_TreeItemBackup[itMOBILE_HANDSET_INBOX]++;
				break;

			case LLD_FOLDER_OUTBOX:
				memcpy(&m_HandsetSMS_Outbox[m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX]], sms_Data_Strc, sizeof(SMS_Data_Strc));
				m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX]++;
				break;

			case LLD_FOLDER_SMS_F1:
				memcpy(&m_HandsetSMS_Folder1[m_TreeItemBackup[itMOBILE_HANDSET_OTHER1]], sms_Data_Strc, sizeof(SMS_Data_Strc));
				m_TreeItemBackup[itMOBILE_HANDSET_OTHER1]++;
				break;

			case LLD_FOLDER_SMS_F2:
				memcpy(&m_HandsetSMS_Folder2[m_TreeItemBackup[itMOBILE_HANDSET_OTHER2]], sms_Data_Strc, sizeof(SMS_Data_Strc));
				m_TreeItemBackup[itMOBILE_HANDSET_OTHER2]++;
				break;

			case LLD_FOLDER_SMS_F3:
				memcpy(&m_HandsetSMS_Folder3[m_TreeItemBackup[itMOBILE_HANDSET_OTHER3]], sms_Data_Strc, sizeof(SMS_Data_Strc));
				m_TreeItemBackup[itMOBILE_HANDSET_OTHER3]++;
				break;

			case LLD_FOLDER_SMS_F4:
				memcpy(&m_HandsetSMS_Folder4[m_TreeItemBackup[itMOBILE_HANDSET_OTHER4]], sms_Data_Strc, sizeof(SMS_Data_Strc));
				m_TreeItemBackup[itMOBILE_HANDSET_OTHER4]++;
				break;

			//case LLD_FOLDER_SMS_F5:
			//	memcpy(&m_HandsetSMS_Folder5[m_TreeItemBackup[itMOBILE_HANDSET_OTHER5]], sms_Data_Strc, sizeof(SMS_Data_Strc));
			//	m_TreeItemBackup[itMOBILE_HANDSET_OTHER5]++;
			//	break;

			};
			
		
		}
	
	}


	delete sms_Data_Strc;

	return true;
}
#endif

int	CLeftView::GetMobileOneSMSInfo(int nNeed)
{
	int nRet = s_ActiveDlg->GetMobileSMSInfo();
	if(nRet == -1) return false;
	return true;
}
//bool bHasConnected = false;
void CLeftView::UpdateMobileConnectStatus(int State)
{
	if(m_bCapture)
	{
		SetCursor( LoadCursor(NULL, IDC_ARROW) );
		ReleaseCapture();
		m_bCapture = FALSE;
	}
	m_bDisplayConnectStatus = TRUE;
   if(State == Anw_SUCCESS)
   {
		theApp.nIsConnected = true;
		if(bHasConnected == false)
		{
#ifdef ASUSM303
			ANWInitSMS();
#endif
			bHasConnected = true;
			g_ConnectFail = false;
			GetAllSMSInfo();

			/*if( CheckSIMfn()==Anw_SUCCESS )
				s_HasSIMCard = true;
			else
				s_HasSIMCard = false;*/

		}
   }
   else if(State == Anw_MOBILE_CONNECT_FAILED)
   {
	    ///if(g_ConnectFail == false)
		///	s_ActiveDlg->xErrorHandling(State);

		g_ConnectFail = true;
		bHasConnected = false;
		theApp.nIsConnected = false;
   }



}
int CLeftView::ConnectStatusCallBack(int State)
{
	CWnd* pWnd = afxGetMainWnd();
	if(pWnd)
	{
		while(s_ActiveDlg->m_bRuningDlg == true)
		{
			Sleep(500);
		}

		pWnd->SendMessage(WM_MOBILE_CONNECT,State,NULL);
	}

   return 0;
}
int CLeftView::OpenMobileConnect(void)
{
	//char Buffer[100];
	int ret = 0;

	int nCompany	= _ttoi(m_strCompany);

	fpSMSCallBack = ConnectStatusCallBack;

	// Ken 7/29
	::SetCapture(m_hWnd);
	SetCursor( LoadCursor(NULL, IDC_WAIT) );
	m_bCapture = TRUE;

	if(_tcslen(m_strIMEI) == 0)
		ANWOpenSMS(nCompany, LPTSTR(m_strPhone), LPTSTR(m_strConnectMode), LPTSTR(m_strPort), NULL, fpSMSCallBack);
	else	
		ANWOpenSMS(nCompany, LPTSTR(m_strPhone), LPTSTR(m_strConnectMode), LPTSTR(m_strPort), LPTSTR(m_strIMEI), fpSMSCallBack);

	return true;
}


int CLeftView::xDownloadSMS(void)
{
	//SMS_Tal_Num sms_Tal_Num;
	memset(&m_sms_Tal_Num, 0, sizeof(SMS_Tal_Num));
	int ret = ANWGetMobileSMSInfo(&m_sms_Tal_Num);
	if(ret != 1)
	{
		//AfxMessageBox("GetMobileSMSInfo failed!");
		return false;
	}
	if(m_sms_Tal_Num.MESMSUsedNum == -1 || m_sms_Tal_Num.SIMSMStUsedNum == -1)
	{
	
		CProgressDlg dlg;
		dlg.SetDataToDlg(&m_sms_Tal_Num);
		dlg.DoModal();

	}
	/*char strmsg[255];
	sprintf(strmsg, "%d, %d", m_sms_Tal_Num.MESMSTotalNum, m_sms_Tal_Num.SIMSMSTotalNum);
	AfxMessageBox(strmsg);*/


#ifdef ASUSM303
	gGetDataFromM303 = 0;
	//memset(&ASUS_GetData, 0, sizeof(ASUS_GetData)*10);
	asus_GetData[0].MemType		= LLD_MEMORY_TYPE_HANDSET;
	asus_GetData[0].FolderType	= LLD_FOLDER_INBOX;
	asus_GetData[0].Items		= m_sms_Tal_Num.MEINUsedNum;

	asus_GetData[1].MemType		= LLD_MEMORY_TYPE_HANDSET;
	asus_GetData[1].FolderType	= LLD_FOLDER_OUTBOX;
	asus_GetData[1].Items		= m_sms_Tal_Num.MEOUTUsedNum;

	asus_GetData[2].MemType		= LLD_MEMORY_TYPE_HANDSET;
	asus_GetData[2].FolderType	= LLD_FOLDER_SMS_F1;
	asus_GetData[2].Items		= m_sms_Tal_Num.MEF1UsedNum;

	asus_GetData[3].MemType		= LLD_MEMORY_TYPE_HANDSET;
	asus_GetData[3].FolderType	= LLD_FOLDER_SMS_F2;
	asus_GetData[3].Items		= m_sms_Tal_Num.MEF2UsedNum;

	asus_GetData[4].MemType		= LLD_MEMORY_TYPE_HANDSET;
	asus_GetData[4].FolderType	= LLD_FOLDER_SMS_F3;
	asus_GetData[4].Items		= m_sms_Tal_Num.MEF3UsedNum;

	asus_GetData[5].MemType		= LLD_MEMORY_TYPE_HANDSET;
	asus_GetData[5].FolderType	= LLD_FOLDER_SMS_F4;
	asus_GetData[5].Items		= m_sms_Tal_Num.MEF4UsedNum;

	//asus_GetData[6].MemType		= LLD_MEMORY_TYPE_HANDSET;
	//asus_GetData[6].FolderType	= LLD_FOLDER_SMS_F5;
	//asus_GetData[6].Items		= m_sms_Tal_Num.MEF5UsedNum;

	asus_GetData[6].MemType		= LLD_MEMORY_TYPE_SIM;
	asus_GetData[6].FolderType	= LLD_FOLDER_INBOX;
	asus_GetData[6].Items		= m_sms_Tal_Num.SMINUsedNum;

	asus_GetData[7].MemType		= LLD_MEMORY_TYPE_SIM;
	asus_GetData[7].FolderType	= LLD_FOLDER_OUTBOX;
	asus_GetData[7].Items		= m_sms_Tal_Num.SMOUTUsedNum;
#endif

	memset(m_SIMSMS_Inbox, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	memset(m_SIMSMS_Outbox, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	memset(m_HandsetSMS_Inbox, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	memset(m_HandsetSMS_Outbox, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);

#ifdef ASUSM303
	memset(m_HandsetSMS_Folder2, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	memset(m_HandsetSMS_Folder1, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	memset(m_HandsetSMS_Folder3, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
	memset(m_HandsetSMS_Folder4, 0, sizeof(SMS_PARAM)*SMS_MOBILE_ItemCount);
#endif

	m_TreeItemBackup[itMOBILE_SIMCARD_INBOX] = 0;
	m_TreeItemBackup[itMOBILE_SIMCARD_OUTBOX] = 0;
	m_TreeItemBackup[itMOBILE_HANDSET_INBOX] = 0;
	m_TreeItemBackup[itMOBILE_HANDSET_OUTBOX] = 0;

#ifdef ASUSM303
	m_TreeItemBackup[itMOBILE_HANDSET_OTHER1] = 0;
	m_TreeItemBackup[itMOBILE_HANDSET_OTHER2] = 0;
	m_TreeItemBackup[itMOBILE_HANDSET_OTHER3] = 0;
	m_TreeItemBackup[itMOBILE_HANDSET_OTHER4] = 0;
#endif
	
	if(m_sms_Tal_Num.SIMSMSTotalNum ==0)
	{
		if(gMobileSIMCardItem)
		{
			if(gMobileSIMCardInboxItem)
			{
				GetTreeCtrl().DeleteItem(gMobileSIMCardInboxItem);
				gMobileSIMCardInboxItem = NULL;
			}
			if(gMobileSIMCardOutboxItem)
			{
				GetTreeCtrl().DeleteItem(gMobileSIMCardOutboxItem);
				gMobileSIMCardOutboxItem = NULL;
			}
			GetTreeCtrl().DeleteItem(gMobileSIMCardItem);
			gMobileSIMCardItem = NULL;
		}
	}
	if(m_sms_Tal_Num.MESMSTotalNum ==0)
	{
		if(gMobileHandsetItem)
		{
			if(gMobileHandsetInboxItem)
			{
				GetTreeCtrl().DeleteItem(gMobileHandsetInboxItem);
				gMobileHandsetInboxItem = NULL;
			}
			if(gMobileHandsetOutboxItem)
			{
				GetTreeCtrl().DeleteItem(gMobileHandsetOutboxItem);
				gMobileHandsetOutboxItem = NULL;
			}
			GetTreeCtrl().DeleteItem(gMobileHandsetItem);
			gMobileHandsetItem = NULL;
		}
	}
	// Get SMS From HandSet
	short MemType = MEM_ME;
	int nNeedCount = m_sms_Tal_Num.MESMSUsedNum + m_sms_Tal_Num.SIMSMStUsedNum;

	//StartDownloadSMS(NeedCount);
	m_bStart = true;
	CProcessDlg dlg;
	if(nNeedCount >= 0)
	{
	
		dlg.SetProgressInfo( CLeftView::GetMobileOneSMSInfo, nNeedCount);
		dlg.SetLoopCount(nNeedCount);
		m_bRuningDlg = true;
		dlg.DoModal();
		m_bRuningDlg = false;
	}
	/*else
	{
	
	}*/

	return true;
}

void CLeftView::xprintf(TCHAR *src, TCHAR *dst, int nNumber)
{
	memset(dst, 0, MAX_PATH);
	TCHAR s[10];
	CString str = src;
	str += '(';
	_itot(nNumber, s, 10);
	str += s;
	str += ')';
	
	memcpy(dst, str.GetBuffer(str.GetLength()), str.GetLength()*sizeof(TCHAR));
	str.ReleaseBuffer();
}




void CLeftView::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	GetTreeCtrl().Select(GetTreeCtrl().HitTest(m_ptClick),TVGN_CARET);
	//::PostMessage(afxGetMainWnd()->GetSafeHwnd(),WM_LEFT_CHGMODE,0,0);

	HTREEITEM hHitItem = GetTreeCtrl().HitTest(m_ptClick);
	if(hHitItem)
		GetTreeCtrl().Select(hHitItem,TVGN_CARET);

	*pResult = 0;
}

void CLeftView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_ptClick = point;

	CTreeView::OnLButtonDown(nFlags, point);
}

void CLeftView::xErrorHandling(int iErr)
{
	if(iErr == Anw_SUCCESS)
		return;

	TCHAR szMsg[MAX_PATH];
	switch(iErr)
	{
	case Anw_TRIAL_LIMITEDFUNCTION:
		{
			TCHAR szText[MAX_PATH];
			al_GetSettingString(_T("public"),_T("IDS_TITLE"),theApp.m_szRes,szText);
			BusyMsgDlg(theApp.m_pMainWnd->m_hWnd,szText);
		}
		return ;
		break;
	case Anw_MOBILE_BUSY:
		al_GetSettingString(_T("public"),_T("IDS_ERR_MOBILEBUSY"),theApp.m_szRes,szMsg);
		break;
	case Anw_TIMEOUT:
		al_GetSettingString(_T("public"),_T("IDS_ERR_TIMEOUT"),theApp.m_szRes,szMsg);
		break;
	case Anw_MOBILE_CONNECT_FAILED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_CONNECTFAILED"),theApp.m_szRes,szMsg);
		break;
	case Anw_MOBILE_WRITE_FAILED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_WRITEFAILED"),theApp.m_szRes,szMsg);
		break;
	case Anw_MOBILE_READ_FAILED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_READFAILED"),theApp.m_szRes,szMsg);
		break;
	case Anw_MOBILE_DELETE_FAILED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_DELFAILED"),theApp.m_szRes,szMsg);
		break;
	case Anw_ALLOCATE_MEMORY_FAILED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_ALLOCATEMEMORY"),theApp.m_szRes,szMsg);
		break;
/*	case Anw_PARAMETER_CHECK_ERROR:
		al_GetSettingString(_T("public"),"",theApp.m_szRes,szMsg);
		break;
	case Anw_MEMORY_RW_FAILED:
		al_GetSettingString(_T("public"),"IDS_ERR_MEMORYREADWRITEFAILED",theApp.m_szRes,szMsg);
		break;
	case Anw_MOBILE_POWER_OFF:
		al_GetSettingString(_T("public"),"IDS_ERR_POWREOFF",theApp.m_szRes,szMsg);
		break;
	case Anw_NOT_FOUNT_WINASPI:
		al_GetSettingString(_T("public"),"IDS_ERR_NOTFOUNDWINASPI",theApp.m_szRes,szMsg);
		break;
	case Anw_NOT_FOUND_MOBILE:
		al_GetSettingString(_T("public"),"IDS_ERR_NOTFOUNDMOBILE",theApp.m_szRes,szMsg);
		break;*/
	case Anw_NOT_LINK_MBdrv:
		al_GetSettingString(_T("public"),_T("IDS_ERR_NOTLINKDLL"),theApp.m_szRes,szMsg);
		break;
	case Anw_FileName_FAILED:
		al_GetSettingString(_T("public"),_T("IDS_ERR_FILENAMEFAILED"),theApp.m_szRes,szMsg);
		break;
	case Anw_Thread_Terminate:
		al_GetSettingString(_T("public"),_T("IDS_ERR_THREADTERMINATE"),theApp.m_szRes,szMsg);
		break;
//	case Anw_UNKNOW_ERROR:
	default:
		al_GetSettingString(_T("public"),_T("IDS_ERR_UNKNOW"),theApp.m_szRes,szMsg);
		break;
	}
//	AfxMessageBox(szMsg);
	CWnd *pWnd = afxGetMainWnd();
	if(pWnd)
	{
		CString strTitle;
		strTitle.Empty();
		pWnd->GetWindowText(strTitle);
		::MessageBox(pWnd->m_hWnd,szMsg,strTitle,MB_OK|MB_ICONWARNING);
	}

}


void CLeftView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CTreeView::OnChar(nChar, nRepCnt, nFlags);
}

void CLeftView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if(nChar == VK_CONTROL)
		m_CtrlKeyDown = true;
	
	CTreeView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CLeftView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if(nChar == VK_CONTROL)
		m_CtrlKeyDown = false;
	
	CTreeView::OnKeyUp(nChar, nRepCnt, nFlags);
}
