// LeftView.cpp : implementation file
//

#include "stdafx.h"
#include "Calendar.h"
#include "LeftView.h"
#include "loadstring.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IMAGE_WIDTH 30
#define IMAGE_HEIGHT 30

#define ILI_MOBILE				0 
#define ILI_PC					3

/////////////////////////////////////////////////////////////////////////////
// CLeftView

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)

CLeftView::CLeftView()
{
	m_crText = RGB(  0,   0,   0);	// Black
	m_crBack = RGB(200, 200, 255);	// White
	m_crLine = RGB(132, 132, 132);	// Light grey

	m_brush.CreateSolidBrush(m_crBack);

	m_hIconBtn[0] = m_hIconBtn[1] = NULL;

	m_crItemBgNor = RGB(0,0,0);
	m_crItemBgSel = RGB(0,0,0);
	m_crItemBgFocus = RGB(0,0,0);
	m_crItemTextNor = RGB(0,0,0);
	m_crItemTextSel = RGB(0,0,0);
	m_crItemTextFocus = RGB(0,0,0);
	m_ptClick = CPoint(0,0);
	m_hFont = NULL;
}

CLeftView::~CLeftView()
{
}


BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
	//{{AFX_MSG_MAP(CLeftView)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT( NM_CUSTOMDRAW, OnCustomDraw )
	ON_WM_LBUTTONDOWN()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_TOOLBAR_RESTORE,OnLoadFile)
	ON_MESSAGE(WM_CHECKFILE,OnCheckFile)
	ON_MESSAGE(WM_LOADMOBILE,OnLoadMobile)
	ON_MESSAGE(WM_REFRESH_LEFT_PANEL,OnRefresh)
	ON_MESSAGE(WM_CHECK_LEFTTREEITEM, OnCheckTreeItem)
	ON_MESSAGE(WM_TOOLBAR_RELOAD_MSOT,OnToolbarReloadMSOT)
END_MESSAGE_MAP()

void CLeftView::OnPaint()
{	
	
	//设置树的背景图片  
/*	CPaintDC dc(this);
	CRect rc;
	GetClientRect(&rc);
	CBitmap bmp;
	CBitmap *pOldBmp;
	bmp.LoadBitmap(IDB_BMP_TREEBK);
	CDC *pMemDC = new CDC;
	pMemDC->CreateCompatibleDC(&dc);
	pOldBmp = (CBitmap*)pMemDC->SelectObject(bmp);
	dc.BitBlt(0, 0, 100, 100, pMemDC, 0, 0, SRCCOPY);
	pMemDC->SelectObject(pOldBmp);
	delete pMemDC;
*/
	CTreeView::OnPaint();

// CBrush brush(RGB(0,255,0));
// rc.FillRect(rc,&brush);

	
}
HRESULT CLeftView::OnToolbarReloadMSOT(WPARAM wParam,LPARAM lParam)
{
 	FileData *pFolderData = reinterpret_cast<FileData*>(lParam);
	HTREEITEM hMSOTROOT = NULL;
	HTREEITEM hMSOTFolder = NULL;
	FileData  *pOTFolderInfo = NULL;
	if(_tcscmp(MSOUTLOOK,pFolderData->sMode.c_str()) == 0 && pFolderData->iIndex >=0)
	{
		if(theApp.m_bLoadMSOutlook == true)
		{
			HTREEITEM hRoot = GetTreeCtrl().GetRootItem();
			while(hRoot)
			{
				//get item data 
				FileData *pData = reinterpret_cast<FileData*>(GetTreeCtrl().GetItemData(hRoot));
				//compare the string . If they are the same , set the flag to be true.
				if(_tcscmp(MSOUTLOOK,pData->sMode.c_str()) == 0)
				{
					hMSOTROOT = hRoot;
					break;
				}
				//get next tree item
				hRoot = GetTreeCtrl().GetNextItem(hRoot,TVGN_NEXT);
			}
		}

		if(hMSOTROOT)
		{
	/////////////		/
			if(pFolderData->iIndex == 0)
			{
				HTREEITEM hChild = GetTreeCtrl().GetChildItem(hMSOTROOT);
				while(hChild)
				{
					//get item data
					HTREEITEM hTemp = 	hChild;		
					FileData *pData = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hChild));
					//Get Second level
					HTREEITEM hFolderChild = GetTreeCtrl().GetChildItem(hChild);
					while(hFolderChild)
					{
						HTREEITEM hTemp2 = 	hFolderChild;		
						FileData *pFolderData = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hFolderChild));

			//			 pFolderData->OTDataList.clear();
						POSITION pos = 	pFolderData->CalendarDataList.GetHeadPosition();
						while(pos)
						{
							SchedulesRecord *pSRData = (SchedulesRecord*) pFolderData->CalendarDataList.GetNext(pos);
							if(pSRData)
							{
								delete pSRData;
								pSRData = NULL;
							}

						}
						pFolderData->CalendarDataList.RemoveAll();


						hFolderChild = GetTreeCtrl().GetNextItem(hFolderChild,TVGN_NEXT);	
						SAFE_DELPTR(pFolderData);
						GetTreeCtrl().DeleteItem(hTemp2);
					}
					//////
//					pData->OTDataList.clear();
					POSITION pos = 	pData->CalendarDataList.GetHeadPosition();
					while(pos)
					{
						SchedulesRecord *pSRData = (SchedulesRecord*) pData->CalendarDataList.GetNext(pos);
						if(pSRData)
						{
							delete pSRData;
							pSRData = NULL;
						}

					}
					pData->CalendarDataList.RemoveAll();


					hChild = GetTreeCtrl().GetNextItem(hChild,TVGN_NEXT);
					SAFE_DELPTR(pData);
					GetTreeCtrl().DeleteItem(hTemp);

				}
				InsertMSOTTree(hMSOTROOT);
				return 1L;
			}
/////////////
			HTREEITEM hChild = GetTreeCtrl().GetChildItem(hMSOTROOT);
			while(hChild)
			{
				//get item data
				FileData *pData = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hChild));
				//compare the file path and item data
				if(_tcscmp(pFolderData->sFile.c_str(),pData->sFile.c_str()) == 0)
				{
					hMSOTFolder = hChild;
					pOTFolderInfo = pData;
					break;
				}
				hChild = GetTreeCtrl().GetNextItem(hChild,TVGN_NEXT);	
			}
		}

		if(hMSOTFolder)
		{
			HTREEITEM hChild = GetTreeCtrl().GetChildItem(hMSOTFolder);
			while(hChild)
			{
				HTREEITEM hTemp = 	hChild;		
				FileData *pFolderData = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hChild));
				POSITION pos = 	pFolderData->CalendarDataList.GetHeadPosition();
				while(pos)
				{
					SchedulesRecord *pData = (SchedulesRecord*) pFolderData->CalendarDataList.GetNext(pos);
					if(pData)
					{
						delete pData;
						pData = NULL;
					}

				}
				pFolderData->CalendarDataList.RemoveAll();

				hChild = GetTreeCtrl().GetNextItem(hChild,TVGN_NEXT);	
				SAFE_DELPTR(pFolderData);
				GetTreeCtrl().DeleteItem(hTemp);
			}
			CPtrList OTFolderList;
			OTFolderList.RemoveAll();
			theApp.m_MSOTDll.MSOT_InitDLL();
			theApp.m_MSOTDll.MSOT_GetFolderList(pOTFolderInfo->sEntryID,CalendarFolder,&OTFolderList);
			POSITION folderpos = OTFolderList.GetHeadPosition();
			int nFolderIndex = 1;
			while(folderpos)
			{
				OutlookFolder *OTFolderData =(OutlookFolder *) OTFolderList.GetNext(folderpos);
				FileData *pData = new FileData;
				pData->sMode = MSOUTLOOK;
				pData->sFile = OTFolderData->sEntryID;
				pData->iIndex = (pOTFolderInfo->iIndex*100+nFolderIndex)*(-1);

				pData->sName.Format(_T("%s"),OTFolderData->sName);
				pData->sEntryID.Format(_T("%s"),OTFolderData->sEntryID);
				pData->sStoreID.Format(_T("%s"),OTFolderData->sStoreID);
				pData->dwTypeFlag = OTFolderData->dwTypeFlag;
//				pData->OTDataList.clear();
				pData->CalendarDataList.RemoveAll();
				pData->bLoad = false;
				HTREEITEM hFolderItem = GetTreeCtrl().InsertItem(pData->sName,7,7,hMSOTFolder);
				GetTreeCtrl().SetItemData(hFolderItem,reinterpret_cast<DWORD>(pData));
				nFolderIndex++;

			}
			folderpos = OTFolderList.GetHeadPosition();
			while(folderpos)
			{
				OutlookFolder* pData = (OutlookFolder*) OTFolderList.GetNext(folderpos);
				if(pData)
					delete pData;
			}
			OTFolderList.RemoveAll();
			theApp.m_MSOTDll.MSOT_TerminateDLL();

		}
	}
	return 1;
}
void CLeftView::InsertMSOTTree(HTREEITEM hRoot)
{
//		TCHAR szText[MAX_PATH];
//		if(al_GetSettingString(_T("public"),"IDS_COMPUTER",theApp.m_szRes,szText))
		if(theApp.m_bLoadMSOutlook == true)
		{
			//Set item data
/*			FileData *pData = new FileData;
			pData->sMode = MSOUTLOOK;
			pData->iIndex = 0;
			pData->bLoad = true;
			GetTreeCtrl().SetItemData(hRoot,reinterpret_cast<DWORD>(pData));
*/
			int iIndex = 1;
			CPtrList OTProfileList;
		//	CPtrList OTFolderList;
			OTProfileList.RemoveAll();
	//		OTFolderList.RemoveAll();
			theApp.m_MSOTDll.MSOT_InitDLL();
			theApp.m_MSOTDll.MSOT_GetProfileListWithoutFolderType(&OTProfileList);
			POSITION pos = OTProfileList.GetHeadPosition();
			while(pos)
			{
				OutlookFolder* pFolderInfo = (OutlookFolder*) OTProfileList.GetNext(pos);
		//		if(pFolderInfo->dwTypeFlag & CalendarFolder_FLAG)
				{
			//		int nIndex = m_cmxOutlook.AddString(pFolderInfo->sName);
			//		m_cmxOutlook.SetItemData(nIndex,(DWORD)pFolderInfo);
					
					HTREEITEM hTreeItem = GetTreeCtrl().InsertItem(pFolderInfo->sName,6,6,hRoot);
					//set item data
					FileData *pData = new FileData;
					pData->sMode = MSOUTLOOK;
					pData->sFile = pFolderInfo->sEntryID;
					pData->iIndex = iIndex;

					pData->sName.Format(_T("%s"),pFolderInfo->sName);
					pData->sEntryID.Format(_T("%s"),pFolderInfo->sEntryID);
					pData->sStoreID.Format(_T("%s"),pFolderInfo->sStoreID);
					pData->dwTypeFlag = pFolderInfo->dwTypeFlag;
//					pData->OTDataList.clear();
					pData->CalendarDataList.RemoveAll();
					pData->bLoad = false;
					GetTreeCtrl().SetItemData(hTreeItem,reinterpret_cast<DWORD>(pData));
					iIndex ++;
			/*		OTFolderList.RemoveAll();
					theApp.m_MSOTDll.MSOT_GetFolderList(pFolderInfo->sEntryID,CalendarFolder,&OTFolderList);
					POSITION folderpos = OTFolderList.GetHeadPosition();
					int nFolderIndex = 1;
					while(folderpos)
					{
						OutlookFolder *OTFolderData =(OutlookFolder *) OTFolderList.GetNext(folderpos);
						FileData *pData = new FileData;
						pData->sMode = MSOUTLOOK;
						pData->sFile = OTFolderData->sEntryID;
						pData->iIndex = iIndex*100+nFolderIndex;

						pData->sName.Format(_T("%s"),OTFolderData->sName);
						pData->sEntryID.Format(_T("%s"),OTFolderData->sEntryID);
						pData->sStoreID.Format(_T("%s"),OTFolderData->sStoreID);
						pData->dwTypeFlag = OTFolderData->dwTypeFlag;
//						pData->OTDataList.clear();
						pData->CalendarDataList.RemoveAll();
						pData->bLoad = false;
						HTREEITEM hFolderItem = GetTreeCtrl().InsertItem(pData->sName,7,7,hTreeItem);
						GetTreeCtrl().SetItemData(hFolderItem,reinterpret_cast<DWORD>(pData));
						nFolderIndex++;

					}
					folderpos = OTFolderList.GetHeadPosition();
					while(folderpos)
					{
						OutlookFolder* pData = (OutlookFolder*) OTFolderList.GetNext(folderpos);
						if(pData)
							delete pData;
					}
					OTFolderList.RemoveAll();*/

				}
			}
			pos = OTProfileList.GetHeadPosition();
			while(pos)
			{
				OutlookFolder* pData = (OutlookFolder*) OTProfileList.GetNext(pos);
				if(pData)
					delete pData;
			}
			OTProfileList.RemoveAll();
			theApp.m_MSOTDll.MSOT_TerminateDLL();

			GetTreeCtrl().Expand(hRoot, TVE_EXPAND); 
			//set tree indent
			GetTreeCtrl().SetIndent(10);
		}
}
int CLeftView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;

	//？？
	m_DropTarget.Register(this);

	//Create Imagelist
	m_ImgList.Create(IMAGE_WIDTH,IMAGE_WIDTH, ILC_COLOR16,6,1);
	//profile path 
	//m_sProfile = string(theApp.m_szSkin) + string("Organize\\LeftView.ini");
	m_sProfile = Tstring(theApp.m_szSkin) + Tstring(_T("Calendar\\LeftView.ini"));//nono, 2004_1101

	//load tree icon
	TCHAR szSkin[MAX_PATH];
	TCHAR szSec[TREE_ITEM_NUM][32] = {_T("mobile_item"),_T("memory_item"),_T("sim_item"),_T("computer_item"),_T("save_item"),_T("OT_item"),_T("OT_Calendar"),_T("OT_Calendar_Folder")};
	for(int i = 0 ; i < TREE_ITEM_NUM ; i ++){
		//get item image path 
		if(al_GetSettingString(szSec[i],_T("image"),const_cast<TCHAR *>(m_sProfile.c_str()),szSkin)){
			Tstring sSkin(theApp.m_szSkin);		
			sSkin += szSkin;
			//add image file to list
			xAddToImageList(sSkin.c_str());
		}
	}
	//Set image to the tree control
	GetTreeCtrl().SetImageList(&m_ImgList , TVSIL_NORMAL);
	//调整树的位置
	CRect rc;
	GetTreeCtrl().GetWindowRect(&rc);
	ScreenToClient(&rc);	//必须有这个函数 不然窗口坐标是整个屏幕 而不是leftview
	rc.OffsetRect(200,0);	//位置？
	GetTreeCtrl().MoveWindow(rc);
	//设置树的背景颜色在这里，没有通过读取ini
	GetTreeCtrl().SetBkColor(RGB(255,255,255));
	//get tree item color
	al_GetSettingColor(_T("tree"),_T("color_text_nor"),const_cast<TCHAR *>(m_sProfile.c_str()),m_crItemTextNor);
	al_GetSettingColor(_T("tree"),_T("color_text_sel"),const_cast<TCHAR *>(m_sProfile.c_str()),m_crItemTextSel);
	al_GetSettingColor(_T("tree"),_T("color_text_focusl"),const_cast<TCHAR *>(m_sProfile.c_str()),m_crItemTextFocus);
	al_GetSettingColor(_T("tree"),_T("color_bg_nor"),const_cast<TCHAR *>(m_sProfile.c_str()),m_crItemBgNor);
	al_GetSettingColor(_T("tree"),_T("color_bg_sel"),const_cast<TCHAR *>(m_sProfile.c_str()),m_crItemBgSel);
	al_GetSettingColor(_T("tree"),_T("color_bg_focus"),const_cast<TCHAR *>(m_sProfile.c_str()),m_crItemBgFocus);
	//get item font 
	int iSize = 0;
	TCHAR szName[MAX_PATH];
	if(GetProfileFont(NULL,NULL,iSize,szName))
	{
//		iSize = iSize * 4 / 3 ;
		if(iSize >0) iSize = -1*iSize;
		m_hFont = GetFontEx(szName,iSize);
	}
	//将手机中calendar的条数读出然后添加树的根节点 与一个子节点
	OnLoadMobile(1,0);
	return 0;
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
	m_ImgList.Add(&Bmp,(CBitmap*)NULL);

	//Detach CBitmap
	Bmp.Detach();
}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS /*|TVS_DISABLEDRAGDROP */| WS_BORDER ;
	return CTreeView::PreCreateWindow(cs);
}

void CLeftView::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
//	TCHAR *pTxt = reinterpret_cast<TCHAR*>(GetTreeCtrl().GetItemData(GetTreeCtrl().GetSelectedItem()));
//	::PostMessage(afxGetMainWnd()->GetSafeHwnd(),WM_LEFT_CHGMODE,reinterpret_cast<WPARAM>(pTxt),0);
//	GetTreeCtrl().Select(GetTreeCtrl().HitTest(m_ptClick),TVGN_CARET);
//	::PostMessage(afxGetMainWnd()->GetSafeHwnd(),WM_LEFT_CHGMODE,0,0);
	CWaitCursor wait;
	HTREEITEM hHitItem = GetTreeCtrl().HitTest(m_ptClick);
	if(hHitItem == NULL) return;
	if(theApp.m_bLoadMSOutlook)
	{
	//	HTREEITEM	hCurSel = GetTreeCtrl().GetSelectedItem();
		FileData *pFolderData = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hHitItem));
		if(pFolderData )
		{
			if(pFolderData->sMode == MSOUTLOOK && pFolderData->bLoad == false && pFolderData->iIndex>0)
			{
				CPtrList OTFolderList;
				OTFolderList.RemoveAll();
				theApp.m_MSOTDll.MSOT_InitDLL();

				theApp.m_MSOTDll.MSOT_GetFolderList(pFolderData->sEntryID,CalendarFolder,&OTFolderList);
				POSITION folderpos = OTFolderList.GetHeadPosition();
				int nFolderIndex = 1;
				while(folderpos)
				{
					OutlookFolder *OTFolderData =(OutlookFolder *) OTFolderList.GetNext(folderpos);
					FileData *pData = new FileData;
					pData->sMode = MSOUTLOOK;
					pData->sFile = OTFolderData->sEntryID;
					pData->iIndex = ((pFolderData->iIndex)*100+nFolderIndex)*(-1);

					pData->sName.Format(_T("%s"),OTFolderData->sName);
					pData->sEntryID.Format(_T("%s"),OTFolderData->sEntryID);
					pData->sStoreID.Format(_T("%s"),OTFolderData->sStoreID);
					pData->dwTypeFlag = OTFolderData->dwTypeFlag;
			//		pData->OTDataList.clear();
					pData->CalendarDataList.RemoveAll();
					pData->bLoad = false;
					HTREEITEM hFolderItem = GetTreeCtrl().InsertItem(pData->sName,7,7,hHitItem);
					GetTreeCtrl().SetItemData(hFolderItem,reinterpret_cast<DWORD>(pData));
					nFolderIndex++;

				}
				folderpos = OTFolderList.GetHeadPosition();
				while(folderpos)
				{
					OutlookFolder* pOTData = (OutlookFolder*) OTFolderList.GetNext(folderpos);
					if(pOTData)
						delete pOTData;
				}
				OTFolderList.RemoveAll();
				pFolderData->bLoad = true;
				GetTreeCtrl().Expand(hHitItem, TVE_EXPAND); 
				theApp.m_MSOTDll.MSOT_TerminateDLL();
			}
		}
	}
 	 GetTreeCtrl().Select(hHitItem,TVGN_CARET);
	::PostMessage(afxGetMainWnd()->GetSafeHwnd(),WM_LEFT_CHGMODE,0,0);

	*pResult = 0;
}

void CLeftView::FreeChildItem(HTREEITEM hParent)
{
	HTREEITEM hChild = GetTreeCtrl().GetChildItem(hParent);
	while(hChild)
	{
		//delete child item data
		FileData *ptr = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hChild));
		if(ptr){
			ptr->sFile = _T("");
			ptr->sMode = _T("");
			POSITION pos = 	ptr->CalendarDataList.GetHeadPosition();
			while(pos)
			{
				SchedulesRecord *pData = (SchedulesRecord*)ptr->CalendarDataList.GetNext(pos);
				if(pData)
				{
					delete pData;
					pData = NULL;
				}

			}
			ptr->CalendarDataList.RemoveAll();
//			for(list<CCardPanel>::iterator iter = ptr->OTDataList.begin() ; iter != ptr->OTDataList.end() ; iter ++){
	//			(*iter).Release();
//			}
		//	 ptr->OTDataList.clear();

			SAFE_DELPTR(ptr);
		}
		FreeChildItem(hChild);
		//Get next item
		hChild = GetTreeCtrl().GetNextItem(hChild,TVGN_NEXT);
	}
}
void CLeftView::OnDestroy() 
{
	m_DropTarget.Revoke();
	
	//Get root item handle
	HTREEITEM hRoot = GetTreeCtrl().GetRootItem();
		
	while(hRoot){
		//delete root item data
		FileData *ptr = reinterpret_cast<FileData*>(GetTreeCtrl().GetItemData(hRoot));
		if(ptr){
			ptr->sFile = _T("");
			ptr->sMode = _T("");
			POSITION pos = 	ptr->CalendarDataList.GetHeadPosition();
			while(pos)
			{
				SchedulesRecord *pData =(SchedulesRecord*) ptr->CalendarDataList.GetNext(pos);
				if(pData)
				{
					delete pData;
					pData = NULL;
				}

			}
			ptr->CalendarDataList.RemoveAll();
//				for(list<CCardPanel>::iterator iter = ptr->OTDataList.begin() ; iter != ptr->OTDataList.end() ; iter ++){
//					(*iter).Release();
//				}
//				 ptr->OTDataList.clear();
			SAFE_DELPTR(ptr);
		}
		//Get child item handle
		FreeChildItem(hRoot);
		//Get next root
		hRoot = GetTreeCtrl().GetNextItem(hRoot,TVGN_NEXT);
	}

	CTreeView::OnDestroy();
}


HRESULT CLeftView::OnLoadFile(WPARAM wParam,LPARAM lParam)
{/*
	//open file path
	string sFile(reinterpret_cast<TCHAR *>(wParam));

	//check the parent item insert 
	//get root item
	HTREEITEM hRoot = GetTreeCtrl().GetRootItem();
	
	bool bRootExist = false;

	while(hRoot){
		//get item data 
		FileData *pData = reinterpret_cast<FileData*>(GetTreeCtrl().GetItemData(hRoot));
		//compare the string . If they are the same , set the flag to be true.
		if(_tcscmp(COMPUTER,pData->sMode.c_str()) == 0){
			bRootExist = true;
			break;
		}
		//get next tree item
		hRoot = GetTreeCtrl().GetNextItem(hRoot,TVGN_NEXT);
	}
	//if root is not exist
	if(!bRootExist){
		//Add TreeItems
		TCHAR szText[MAX_PATH];
		if(al_GetSettingString(_T("public"),"IDS_COMPUTER",theApp.m_szRes,szText))
			hRoot = GetTreeCtrl().InsertItem(szText, ILI_PC, ILI_PC);

		//Set item data
		FileData *pData = new FileData;
		pData->sMode = COMPUTER;
		GetTreeCtrl().SetItemData(hRoot,reinterpret_cast<DWORD>(pData));
	}
	
	//check all item data , if there are existing ,do nothing
	HTREEITEM hChild = GetTreeCtrl().GetChildItem(hRoot);
	bool bItemExist = false;
	int iIndex = 0;
	//check  child item exist 
	while(hChild){
		//get item data
		FileData *pData = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hChild));
		//compare the file path and item data
		if(_tcscmp(sFile.c_str(),pData->sFile.c_str()) == 0){
			bItemExist = true;
			GetTreeCtrl().Select(hChild,TVGN_CARET);
			break;
		}
		//get next tree item
		hChild = GetTreeCtrl().GetNextItem(hChild,TVGN_NEXT);	
		iIndex ++;
	}

	if(!bItemExist){
		//add item to tree
		string sFileName = sFile.substr(sFile.find_last_of('\\') + 1);
		HTREEITEM hTreeItem = GetTreeCtrl().InsertItem(sFileName.c_str(),4,4,hRoot);

		//set item data
		FileData *pData = new FileData;
		pData->sMode = CSVFILE;
		pData->sFile = sFile;
		pData->iIndex = iIndex;
		GetTreeCtrl().SetItemData(hTreeItem,reinterpret_cast<DWORD>(pData));
		//set the tree item to be selected
		GetTreeCtrl().Select(hTreeItem,TVGN_CARET);
	}

	//expand the tree
	GetTreeCtrl().Expand(hRoot, TVE_EXPAND); 
	//set tree indent
	GetTreeCtrl().SetIndent(10);
*/
	return 1L;
}

HRESULT CLeftView::OnCheckFile(WPARAM wParam,LPARAM lParam)
{
	LPCTSTR lpszFile = reinterpret_cast<LPCTSTR>(wParam);
	bool *pbFlag = reinterpret_cast<bool*>(lParam);
	*pbFlag = false;
	//Get root item handle
	HTREEITEM hRoot = GetTreeCtrl().GetRootItem();

	while(hRoot){
		//delete root item data
		FileData *ptr = reinterpret_cast<FileData*>(GetTreeCtrl().GetItemData(hRoot));
		if(ptr){
			if(_tcscmp(ptr->sFile.c_str(),lpszFile) == 0)
				*pbFlag = true;
		}
		//Get child item handle
		HTREEITEM hChild = GetTreeCtrl().GetChildItem(hRoot);
		while(hChild){
			//delete child item data
			ptr = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hChild));
			if(ptr){
				if(_tcscmp(ptr->sFile.c_str(),lpszFile) == 0)
					*pbFlag = true;
			}
			//Get next item
			hChild = GetTreeCtrl().GetNextItem(hChild,TVGN_NEXT);
		}
		//Get next root
		hRoot = GetTreeCtrl().GetNextItem(hRoot,TVGN_NEXT);
	}

	return 1L;
}


DROPEFFECT CLeftView::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	// TODO: Add your specialized code here and/or call the base class
    if((dwKeyState & MK_CONTROL) == MK_CONTROL)
        return DROPEFFECT_NONE;
    else
        return DROPEFFECT_MOVE;    	
//	return CTreeView::OnDragEnter(pDataObject, dwKeyState, point);
}

DROPEFFECT CLeftView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	// TODO: Add your specialized code here and/or call the base class
	CMainFrame *pFrame =(CMainFrame *) afxGetMainWnd();
    if((dwKeyState & MK_CONTROL) == MK_CONTROL)
        return DROPEFFECT_NONE;  
	else if(!pFrame->m_bMobileConnected )
		return DROPEFFECT_NONE;  
    else{
		//Get droped tree item handle
		HTREEITEM hItem = GetTreeCtrl().HitTest(point);
		if(hItem){
			FileData *pData = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hItem));
			if(pData){
				if(pData->sMode == COMPUTER )
					return DROPEFFECT_NONE;
				else if(pData->sMode == MSOUTLOOK && pData->iIndex>=0)
					return DROPEFFECT_NONE;
				else 
					return DROPEFFECT_MOVE;
			}
		}
        return DROPEFFECT_MOVE;  // move source

	}
	
//	return CTreeView::OnDragOver(pDataObject, dwKeyState, point);
}

BOOL CLeftView::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) 
{
	// TODO: Add your specialized code here and/or call the base class
    // If the dropEffect requested is not a MOVE, return FALSE to 
    // signal no drop. (No COPY into trashcan)
    if((dropEffect & DROPEFFECT_MOVE) != DROPEFFECT_MOVE)
        return FALSE;
	
	//Get droped tree item handle
	HTREEITEM hItem = GetTreeCtrl().HitTest(point);
	//check droped item
	if(hItem){
		//Get the droped item data
		FileData *pData = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hItem));
		if(pData){
			//check tree item
/*			if(pData->sMode !=  SIM_CARD && pData->sMode != MEMORY){
				TCHAR szMsg[MAX_PATH];
				al_GetSettingString(_T("public"),"IDS_ERR_DROPTOTREE",theApp.m_szRes,szMsg);
//				AfxMessageBox(szMsg);
				CalendarStringMessageBox(m_hWnd,szMsg);
				return FALSE;
			}
			else{
				HTREEITEM hSelItem = GetTreeCtrl().GetSelectedItem();
				if(hItem != hSelItem){
					::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_LEFT_DROPDATA,reinterpret_cast<WPARAM>(pData->sMode.c_str()),0);
					return TRUE;
				}*/
			HTREEITEM hSelItem = GetTreeCtrl().GetSelectedItem();
			if(hItem != hSelItem){
				if(_tcscmp(MSOUTLOOK,pData->sMode.c_str()) == 0)
				{
					::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_LEFT_DROPDATATOMSOT,0,reinterpret_cast<LPARAM>(pData));
					return TRUE;
				}
				::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_LEFT_DROPDATA,reinterpret_cast<WPARAM>(pData->sMode.c_str()),0);
				FileData *pSelData = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hSelItem));
				if(_tcscmp(MEMORY,pSelData->sMode.c_str()) == 0 || _tcscmp(MOBILE_PHONE,pSelData->sMode.c_str()) == 0 )
				{
						::PostMessage(afxGetMainWnd()->GetSafeHwnd(),WM_LEFT_UPDATEDATA,0,0);

				}
				return TRUE;
			}
		}
		
	}
	return FALSE;
//	return CTreeView::OnDrop(pDataObject, dropEffect, point);
}

void CLeftView::OnDragLeave() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CTreeView::OnDragLeave();
}
void CLeftView::OnCheckTreeItem(WPARAM wParam,LPARAM lParam)
{
	bool bHaveMobileItem = (bool)wParam;
	HTREEITEM hMEItem = NULL;
	HTREEITEM hMobileItem = NULL;

	HTREEITEM hRoot = GetTreeCtrl().GetRootItem();
	while(hRoot && ( hMEItem == NULL || hMobileItem == NULL))
	{
		FileData *ptr = reinterpret_cast<FileData*>(GetTreeCtrl().GetItemData(hRoot));
		//get child item handle
		if(ptr && ptr->sMode == MOBILE_PHONE)
			hMobileItem = hRoot;
		HTREEITEM hChild = GetTreeCtrl().GetChildItem(hRoot);
		while(hChild)
		{
			ptr = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hChild));
			if(ptr && ptr->sMode == MOBILE_PHONE)
				hMobileItem = hChild;
			if(ptr && ptr->sMode == MEMORY)
				hMEItem = hChild;
			//Get next item
			hChild = GetTreeCtrl().GetNextItem(hChild,TVGN_NEXT);
		}
		//Get next root
		hRoot = GetTreeCtrl().GetNextItem(hRoot,TVGN_NEXT);
	}


	if(hMEItem && bHaveMobileItem == false)
	{ // remove item
		FileData *pItem = reinterpret_cast<FileData*>(GetTreeCtrl().GetItemData(hMEItem));
		if(pItem)
		{
			POSITION pos = 	pItem->CalendarDataList.GetHeadPosition();
			while(pos)
			{
				SchedulesRecord *pSRData = (SchedulesRecord*) pItem->CalendarDataList.GetNext(pos);
				if(pSRData)
				{
					delete pSRData;
					pSRData = NULL;
				}
			}
			pItem->CalendarDataList.RemoveAll();
			SAFE_DELPTR(pItem);
		}
		GetTreeCtrl().DeleteItem(hMEItem);
	}
	if(hMEItem== NULL && bHaveMobileItem == true  && hMobileItem)
	{//Add Item
		TCHAR szText[MAX_PATH];
		szText[0] = '\0';
		HTREEITEM hTreeItem;
		if(al_GetSettingString(_T("public"),_T("IDS_MOBILE"),theApp.m_szRes,szText)){
			//attach the total number of Calendars. 
			TCHAR chBuffer[10];
			_tcscat(szText, _T(" ("));
			_tcscat(szText, _itot(g_numTotalCalendars, chBuffer, 10));
			_tcscat(szText, _T(")"));

			hTreeItem = GetTreeCtrl().InsertItem(szText,1,1,hMobileItem);
		
			//set memory child item data
			FileData* pData = new FileData;
			pData->sMode = MEMORY;
			GetTreeCtrl().SetItemData(hTreeItem,reinterpret_cast<DWORD>(pData));
		}
	}
}

void CLeftView::OnRefresh(WPARAM wParam,LPARAM lParam)
{
	CTreeCtrl* ptheTree =  &GetTreeCtrl();

	ASSERT(ptheTree);

    HTREEITEM hRoot = ptheTree->GetRootItem();

	if (hRoot) {
    	HTREEITEM hChild = ptheTree->GetChildItem(hRoot);
		if (hChild) {
			TCHAR szText[MAX_PATH];
			CString csMobileItemName;
			csMobileItemName = ptheTree->GetItemText(hChild);
			if (al_GetSettingString(_T("public"),_T("IDS_MOBILE"),theApp.m_szRes,szText)) {
	    		if (0<=csMobileItemName.Find(szText)) {
	            	//attach the total number of Calendars. 
	         		TCHAR chBuffer[10];
	        		_tcscat(szText, _T(" ("));
	        		_tcscat(szText, _itot(g_numTotalCalendars, chBuffer, 10));
		        	_tcscat(szText, _T(")"));
                	ptheTree->SetItemText(hChild, szText);
				}else {
		    	//	AfxMessageBox(_T("None valid item!?"));
				}
			}else {
			//	AfxMessageBox(_T("failed to get valid item text from profile!?"));
			}
		}
	}
}

HRESULT CLeftView::OnLoadMobile(WPARAM wParam,LPARAM lParam)
{
	if(wParam)
	{
		//读取手机数据，插入lefttree中
		//插入树的根节点，leftview继承了treeview 直接使用gettreectrl得到树的句柄
		//首先从setting.ini读取手机的名字
		HTREEITEM hParent = GetTreeCtrl().InsertItem(theApp.m_szMobileName, ILI_MOBILE, ILI_MOBILE);
				
/*      filedata的定义如下
		typedef struct tagFileData{	
			Tstring sMode;		//store this item is which mode	
			int iIndex;			//store file index 
			Tstring sFile;		//store file full path 
			CString	sName;
			CString	sEntryID;
			CString	sStoreID;
			DWORD dwTypeFlag;	//FolderTypeFlag
			CPtrList CalendarDataList;
			bool     bLoad;
			}FileData;
*/
		//set mobile root item data
		FileData *pData = new FileData;
		pData->sMode = MOBILE_PHONE;
		GetTreeCtrl().SetItemData(hParent,reinterpret_cast<DWORD>(pData));
		
		//insert memory child item
		TCHAR szText[MAX_PATH];
		HTREEITEM hTreeItem;
		//读取lang\calendar.ini得到手机名称
		if(al_GetSettingString(_T("public"),_T("IDS_MOBILE"),theApp.m_szRes,szText))
		{
			//attach the total number of Calendars. 
			TCHAR chBuffer[10];
			_tcscat(szText, _T(" ("));
			_tcscat(szText, _itot(g_numTotalCalendars, chBuffer, 10));
			_tcscat(szText, _T(")"));
			//图片列表的关联在leftview的oncreate 函数中实现
			hTreeItem = GetTreeCtrl().InsertItem(szText,1,1,hParent);
		
			//set memory child item data
			//此处代码可以放在if外面，leftview的树只有一个字节点
			pData = new FileData;
			pData->sMode = MEMORY;
			//hTreeItem ？？
			GetTreeCtrl().SetItemData(hTreeItem,reinterpret_cast<DWORD>(pData));
		}
/*
		//insert sim card item 
		if(al_GetSettingString(_T("public"),"IDS_SIMCARD",theApp.m_szRes,szText)){
			hTreeItem = GetTreeCtrl().InsertItem(szText,2,2,hParent);

			//set item data
			pData = new FileData;
			pData->sMode = SIM_CARD;
			GetTreeCtrl().SetItemData(hTreeItem,reinterpret_cast<DWORD>(pData));
		}
*/		//展开树的父节点
		GetTreeCtrl().Expand(hParent, TVE_EXPAND); 
		//设置树的缩进宽度
		GetTreeCtrl().SetIndent(10);
		//设置节点高度
		GetTreeCtrl().SetItemHeight(35);
		GetTreeCtrl().SelectItem(hParent);
		if(theApp.m_bLoadMSOutlook)
		{
			HTREEITEM hMSOTRoot;
			//插入另外一棵树，5 5为图标列表中图片的编号，后面省略两个默认参数 root last...
			hMSOTRoot = GetTreeCtrl().InsertItem(_T("MS Outlook"), 5, 5);

			/*   下面是File Data的结构
			typedef struct tagFileData
			{			
				Tstring sMode;		//store this item is which mode
				int iIndex;			//store file index only for csv file mode
				Tstring sFile;		//store file full path 
				CString	sName;
				CString	sEntryID;
				CString	sStoreID;
				DWORD dwTypeFlag;	//FolderTypeFlag
				CPtrList CalendarDataList;
				bool     bLoad;
			}FileData;
			*/
			FileData *pFData = new FileData;
			pFData->sMode = MSOUTLOOK;
			pFData->iIndex = 0;
			pFData->bLoad = true;
			GetTreeCtrl().SetItemData(hMSOTRoot,reinterpret_cast<DWORD>(pFData));
			InsertMSOTTree(hMSOTRoot);
		}

	}
	return 1L;
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
				xm	= m_nIndent / 2 +4,
				ym	= (rc.bottom - rc.top) / 2;

	x	= rc.left + m_nIndent / 2;
	y	= rc.top;
	if(GetTreeCtrl().GetPrevSiblingItem(hItem) || hParent)
	{
		if(!hParent)								// Root node?
		{
			if(m_dwStyle & TVS_LINESATROOT)			// Lines at root?
				LineVert(hDC, x, y, y + ym+2, bDot);		// Connect to prev / parent
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
   		HBRUSH		hOldBrush;
   		HTREEITEM	hItem		= (HTREEITEM)pCD->nmcd.dwItemSpec,
   					hParent		= GetTreeCtrl().GetParentItem(hItem);
   		RECT		rc			= pCD->nmcd.rc;
   		TV_DISPINFO	tvdi;
   		RECT		rcText , rcTmp;
   		TCHAR		szText[1024];

		SCROLLINFO si;
		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		GetScrollInfo(SB_HORZ, &si);
		rc.left -= si.nPos;

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
   								  TVIF_SELECTEDIMAGE |
   								  TVIF_STATE | TVIF_TEXT;
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
 /* 			if(m_dwStyle & TVS_HASBUTTONS)
   			{
   				if(tvdi.item.cChildren == 1)
   					PaintButton(hDC, rc, tvdi.item.state & TVIS_EXPANDED);
   				else if(tvdi.item.cChildren == I_CHILDRENCALLBACK)
   					PaintButton(hDC, rc, FALSE);
   			}*/
   			if(m_dwStyle & TVS_HASBUTTONS)
   			{
				CRect rcBtn;
				rcBtn.CopyRect(&rc);
				rcBtn.top -=1;
				FileData *pData = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hItem));
				 if(pData->sMode == MSOUTLOOK && pData->bLoad == false && pData->iIndex >0)
 					PaintButton(hDC, rcBtn, tvdi.item.state & TVIS_EXPANDED);
   				else if(tvdi.item.cChildren == 1)
   					PaintButton(hDC, rcBtn, tvdi.item.state & TVIS_EXPANDED);
   				else if(tvdi.item.cChildren == I_CHILDRENCALLBACK)
   					PaintButton(hDC, rcBtn, FALSE);
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
		//get the drawing text rectangle ,but top is not correct
   		::DrawText(hDC, szText, -1, &rcText, DT_LEFT | DT_NOPREFIX | 
   					DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);
		//get the text real size
   		::DrawText(hDC, szText, -1, &rcTmp, DT_LEFT | DT_NOPREFIX | 
   					DT_SINGLELINE | DT_CALCRECT);
		//mesure rectangle top
		rcText.top = rcText.bottom - ( rcTmp.bottom - rcTmp.top);
		rcText.bottom += 1;
		rcText.left -= 1;
		rcText.top -= 1;
		rcText.right += 1;

   		//
   		// Clear the background
   		//
   		if(pCD->nmcd.uItemState & CDIS_FOCUS)
   		{
   			::SetTextColor	(hDC, m_crItemTextFocus);
   			::SetBkColor	(hDC, m_crItemBgFocus);
   		}
   		else if(pCD->nmcd.uItemState & CDIS_SELECTED)
   		{
   			::SetTextColor	(hDC, m_crItemTextSel);
   			::SetBkColor	(hDC, m_crItemBgSel);
   		}
   		else
   		{
			::SetTextColor	(hDC, m_crItemTextNor);
   			::SetBkColor	(hDC, m_crItemBgNor);
   		}

   		//
   		// Now, draw the text
   		//
   		::DrawText(hDC, szText, -1, &rc, DT_LEFT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER );

   		//
   		// Draw the focus rect
   		//
   		if(pCD->nmcd.uItemState & CDIS_FOCUS || pCD->nmcd.uItemState & CDIS_SELECTED)	// leo
		{
			rcText.left +=1;
			rcText.right -=1;
			rcText.top +=1;
			rcText.bottom -=1;
			::DrawFocusRect(hDC, &rcText);
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

void CLeftView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_ptClick = point;
	CTreeView::OnLButtonDown(nFlags, point);
}
void CLeftView::SetItemCount(FileData &fd, int iCount)
{
	HTREEITEM hRoot = GetTreeCtrl().GetRootItem();
	while(hRoot){
		//get child item handle
		HTREEITEM hChild = GetTreeCtrl().GetChildItem(hRoot);
		while(hChild){
			//set child item count,SIM_CARD,MEMORY,CSVFILE
			FileData *ptr = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hChild));
			if(ptr && ptr->sMode == fd.sMode){
				if(fd.sMode == MEMORY)
				{
					TCHAR szTxt[MAX_PATH],szText[MAX_PATH];
					if(al_GetSettingString(_T("public"),_T("IDS_MOBILE"),theApp.m_szRes,szText))
					{
						wsprintf(szTxt,_T("%s (%d)"),szText,iCount);
						GetTreeCtrl().SetItemText(hChild,szTxt);
					}
					break;
				}
				else if(fd.sMode == MSOUTLOOK)
				{
					HTREEITEM hFolder = GetTreeCtrl().GetChildItem(hChild);
					while(hFolder)
					{
						FileData *pFolder = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hFolder));
						if(fd.iIndex == pFolder->iIndex)
						{
							TCHAR szTxt[MAX_PATH];
							wsprintf(szTxt,_T("%s (%d)"),fd.sName,iCount);
							GetTreeCtrl().SetItemText(hFolder,szTxt);
							break;
						}
						hFolder = GetTreeCtrl().GetNextItem(hFolder,TVGN_NEXT);
					}

				}
			}
			//Get next item
			hChild = GetTreeCtrl().GetNextItem(hChild,TVGN_NEXT);
		}
		//Get next root
		hRoot = GetTreeCtrl().GetNextItem(hRoot,TVGN_NEXT);
	}
}

BOOL CLeftView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
		//设置树的背景图片  
/*	//CPaintDC dc(this);
	CRect rc;
	GetClientRect(&rc);
	CBitmap bmp;
	CBitmap *pOldBmp;
	bmp.LoadBitmap(IDB_BMP_TREEBK);
	CDC *pMemDC = new CDC;
	pMemDC->CreateCompatibleDC(pDC);
	pOldBmp = (CBitmap*)pMemDC->SelectObject(&bmp);
	pDC->StretchBlt(0, 0, 200, 500, pMemDC, 0, 0, 50, 50 ,SRCCOPY);
	pMemDC->SelectObject(pOldBmp);
	delete pMemDC;
    return TRUE;*/
	return CTreeView::OnEraseBkgnd(pDC);
}
