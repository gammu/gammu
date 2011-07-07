// LeftView.cpp : implementation file
//

#include "stdafx.h"
#include "..\PhoneBook.h"
#include "LeftView.h"

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

CLeftView::CLeftView():m_bDLFail(false),m_bConnect(false)
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
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_TOOLBAR_RESTORE,OnLoadFile)
	ON_MESSAGE(WM_CHECKFILE,OnCheckFile)
	ON_MESSAGE(WM_LOADMOBILE,OnLoadMobile)
	ON_MESSAGE(WM_TOOLBAR_RELOAD_MSOT,OnToolbarReloadMSOT)
	ON_MESSAGE(WM_LOAD_MSOTTREE,OnLoadMSOTTree)
END_MESSAGE_MAP()

void CLeftView::OnPaint()
{
	CTreeView::OnPaint();
}
HRESULT CLeftView::OnLoadMSOTTree(WPARAM wParam,LPARAM lParam)
{
	CWaitCursor ww;
	HTREEITEM hMSOTRoot;
	hMSOTRoot = GetTreeCtrl().InsertItem(_T("MS Outlook"), 5, 5);

	FileData *pData = new FileData;
	pData->sMode = MSOUTLOOK;
	pData->iIndex = 0 ;
	pData->bLoad = true;
	GetTreeCtrl().SetItemData(hMSOTRoot,reinterpret_cast<DWORD>(pData));

	InsertMSOTTree(hMSOTRoot);
	return 1;
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

					pFolderData->sFile = _T("");
					pFolderData->sMode = _T("");
					for(list<CCardPanel>::iterator iter = pFolderData->OTDataList.begin() ; iter != pFolderData->OTDataList.end() ; iter ++){
							(*iter).Release();
						}
						 pFolderData->OTDataList.clear();

						hFolderChild = GetTreeCtrl().GetNextItem(hFolderChild,TVGN_NEXT);	
						SAFE_DELPTR(pFolderData);
						GetTreeCtrl().DeleteItem(hTemp2);
					}
					//////
					pData->OTDataList.clear();
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

				for(list<CCardPanel>::iterator iter = pFolderData->OTDataList.begin() ; iter != pFolderData->OTDataList.end() ; iter ++){
					(*iter).Release();
				}
				 pFolderData->OTDataList.clear();

				hChild = GetTreeCtrl().GetNextItem(hChild,TVGN_NEXT);	
				SAFE_DELPTR(pFolderData);
				GetTreeCtrl().DeleteItem(hTemp);
			}
			CPtrList OTFolderList;
			OTFolderList.RemoveAll();
			theApp.m_MSOTDll.MSOT_InitDLL();
			theApp.m_MSOTDll.MSOT_GetFolderList(pOTFolderInfo->sEntryID,ContactFolder,&OTFolderList);
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
				pData->OTDataList.clear();
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
//		if(al_GetSettingString(_T("public"),_T("IDS_COMPUTER",theApp.m_szRes,szText))
		if(theApp.m_bLoadMSOutlook == true)
		{
			//Set item data
	/*		FileData *pData = new FileData;
			pData->sMode = MSOUTLOOK;
			pData->iIndex = 0 ;
			pData->bLoad = true;
			GetTreeCtrl().SetItemData(hRoot,reinterpret_cast<DWORD>(pData));
*/
			int iIndex = 1;
			CPtrList OTProfileList;
		//	CPtrList OTFolderList;
			OTProfileList.RemoveAll();
		//	OTFolderList.RemoveAll();
			theApp.m_MSOTDll.MSOT_InitDLL();
			theApp.m_MSOTDll.MSOT_GetProfileListWithoutFolderType(&OTProfileList);
			POSITION pos = OTProfileList.GetHeadPosition();
			while(pos)
			{
				OutlookFolder* pFolderInfo = (OutlookFolder*) OTProfileList.GetNext(pos);
			//	if(pFolderInfo->dwTypeFlag & ContactFolder_FLAG)
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
					pData->OTDataList.clear();
					pData->bLoad = false;
					GetTreeCtrl().SetItemData(hTreeItem,reinterpret_cast<DWORD>(pData));
					iIndex ++;
			/*		OTFolderList.RemoveAll();
					theApp.m_MSOTDll.MSOT_GetFolderList(pFolderInfo->sEntryID,ContactFolder,&OTFolderList);
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
						pData->OTDataList.clear();
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
	
	m_DropTarget.Register(this);

	//Create Imagelist
	m_ImgList.Create(IMAGE_WIDTH,IMAGE_WIDTH, ILC_COLOR16,6,1);
	//profile path 
	m_sProfile = Tstring(theApp.m_szSkin) + Tstring(_T("Organize\\LeftView.ini"));

	//load tree icon
	TCHAR szSkin[MAX_PATH];
	TCHAR szSec[TREE_ITEM_NUM][32] = {_T("mobile_item"),_T("memory_item"),_T("sim_item"),_T("computer_item"),_T("save_item"),_T("OT_item"),_T("OT_contacts"),_T("OT_contacts_Folder")};
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
	CRect rc;
	GetTreeCtrl().GetWindowRect(&rc);
	ScreenToClient(&rc);
	rc.OffsetRect(200,0);
	GetTreeCtrl().MoveWindow(rc);;
	//get tree item color
	al_GetSettingColor(_T("tree"),_T("color_text_nor"),const_cast<TCHAR *>(m_sProfile.c_str()),m_crItemTextNor);
	al_GetSettingColor(_T("tree"),_T("color_text_sel"),const_cast<TCHAR *>(m_sProfile.c_str()),m_crItemTextSel);
	al_GetSettingColor(_T("tree"),_T("color_text_focusl"),const_cast<TCHAR *>(m_sProfile.c_str()),m_crItemTextFocus);
	al_GetSettingColor(_T("tree"),_T("color_bg_nor"),const_cast<TCHAR *>(m_sProfile.c_str()),m_crItemBgNor);
	al_GetSettingColor(_T("tree"),_T("color_bg_sel"),const_cast<TCHAR *>(m_sProfile.c_str()),m_crItemBgSel);
	al_GetSettingColor(_T("tree"),_T("color_bg_focus"),const_cast<TCHAR *>(m_sProfile.c_str()),m_crItemBgFocus);

	
	//Add TreeItems
	TCHAR szTxt[MAX_PATH];
	wsprintf(szTxt,_T("%s (0)"),theApp.m_szMobileName);
	HTREEITEM hParent = GetTreeCtrl().InsertItem(szTxt, ILI_MOBILE, ILI_MOBILE);
	
	//set mobile root item data
	FileData *pData = new FileData;
	pData->sMode = MOBILE_PHONE;
	GetTreeCtrl().SetItemData(hParent,reinterpret_cast<DWORD>(pData));
	
	//insert memory child item
	TCHAR szText[MAX_PATH];
	HTREEITEM hTreeItem;
	if(al_GetSettingString(_T("public"),_T("IDS_MOBILE"),theApp.m_szRes,szText)){
		wsprintf(szTxt,_T("%s (0)"),szText);
		hTreeItem = GetTreeCtrl().InsertItem(szTxt,1,1,hParent);
	
		//set memory child item data
		pData = new FileData;
		pData->sMode = MEMORY;
		GetTreeCtrl().SetItemData(hTreeItem,reinterpret_cast<DWORD>(pData));
	}

	//insert sim card item 
	if(al_GetSettingString(_T("public"),_T("IDS_SIMCARD"),theApp.m_szRes,szText)){
		wsprintf(szTxt,_T("%s (0)"),szText);
		hTreeItem = GetTreeCtrl().InsertItem(szTxt,2,2,hParent);

		//set item data
		pData = new FileData;
		pData->sMode = SIM_CARD;
		GetTreeCtrl().SetItemData(hTreeItem,reinterpret_cast<DWORD>(pData));
	}

	GetTreeCtrl().Expand(hParent, TVE_EXPAND); 
	
	GetTreeCtrl().SetIndent(10);
	GetTreeCtrl().SetItemHeight(35);; //peggy

	GetTreeCtrl().SelectItem(hParent);

	//get item font 
	int iSize = 0;
	TCHAR szName[MAX_PATH];
	if(GetProfileFont(NULL,NULL,iSize,szName)){
		iSize = -iSize ;
		m_hFont = GetFontEx(szName,iSize);
	}
	
	GetTreeCtrl().SetBkColor(RGB(255,255,255));

	if(theApp.m_bLoadMSOutlook)
	{
		HTREEITEM hMSOTRoot;
		hMSOTRoot = GetTreeCtrl().InsertItem(_T("MS Outlook"), 5, 5);
		FileData *pFData = new FileData;
		pFData->sMode = MSOUTLOOK;
		pFData->iIndex = 0 ;
		pFData->bLoad = true;
		GetTreeCtrl().SetItemData(hMSOTRoot,reinterpret_cast<DWORD>(pFData));
		InsertMSOTTree(hMSOTRoot);
	}
//	PostMessage(WM_LOAD_MSOTTREE,0,0);
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
	CWaitCursor wait;
	HTREEITEM hHitItem = GetTreeCtrl().HitTest(m_ptClick);
	if(hHitItem == NULL) return;
//	GetTreeCtrl().Select(GetTreeCtrl().HitTest(m_ptClick),TVGN_CARET);
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

				theApp.m_MSOTDll.MSOT_GetFolderList(pFolderData->sEntryID,ContactFolder,&OTFolderList);
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
					pData->OTDataList.clear();
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
			for(list<CCardPanel>::iterator iter = ptr->OTDataList.begin() ; iter != ptr->OTDataList.end() ; iter ++){
				(*iter).Release();
			}
			 ptr->OTDataList.clear();

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
			ptr->sFile =_T("");
			ptr->sMode = _T("");
			for(list<CCardPanel>::iterator iter = ptr->OTDataList.begin() ; iter != ptr->OTDataList.end() ; iter ++){
				(*iter).Release();
			}
			 ptr->OTDataList.clear();
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
{
	//open file path
	Tstring sFile(reinterpret_cast<TCHAR *>(wParam));

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
		if(al_GetSettingString(_T("public"),_T("IDS_COMPUTER"),theApp.m_szRes,szText))
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
		Tstring sFileName = sFile.substr(sFile.find_last_of('\\') + 1);
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

	return 1L;
}

HRESULT CLeftView::OnCheckFile(WPARAM wParam,LPARAM lParam)
{
	LPCTSTR lpszFile = reinterpret_cast<LPCTSTR>(wParam);
	int *piFlag = reinterpret_cast<int*>(lParam);
	*piFlag = -1;
	//Get root item handle
	HTREEITEM hRoot = GetTreeCtrl().GetRootItem();

	while(hRoot){
		//delete root item data
		FileData *ptr = reinterpret_cast<FileData*>(GetTreeCtrl().GetItemData(hRoot));
		if(ptr){
			if(_tcscmp(ptr->sFile.c_str(),lpszFile) == 0)
				*piFlag = ptr->iIndex;
		}
		//Get child item handle
		HTREEITEM hChild = GetTreeCtrl().GetChildItem(hRoot);
		while(hChild){
			//delete child item data
			ptr = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hChild));
			if(ptr){
				if(_tcscmp(ptr->sFile.c_str(),lpszFile) == 0)
					*piFlag = ptr->iIndex;
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
    if((dwKeyState & MK_CONTROL) == MK_CONTROL)
        return DROPEFFECT_NONE;  
	else if(!m_bConnect || m_bDLFail)
		return DROPEFFECT_NONE;  
    else{
		//Get droped tree item handle
		HTREEITEM hItem = GetTreeCtrl().HitTest(point);
		if(hItem){
			FileData *pData = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hItem));
			if(pData){
				if(pData->sMode ==  CSVFILE || pData->sMode == COMPUTER || pData->sMode == MOBILE_PHONE)
					return DROPEFFECT_NONE;
				else if(pData->sMode == MEMORY && ((CPhoneBookApp*)afxGetApp())->m_bNotSupportME )
					return DROPEFFECT_NONE;
				else if(pData->sMode == SIM_CARD && ((CPhoneBookApp*)afxGetApp())->m_bNotSupportSM )
					return DROPEFFECT_NONE;
				else if(pData->sMode == MSOUTLOOK && pData->iIndex >=0)
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
			HTREEITEM hSelItem = GetTreeCtrl().GetSelectedItem();
			if(hItem != hSelItem){
				if(_tcscmp(MSOUTLOOK,pData->sMode.c_str()) == 0)
				{
					::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_LEFT_DROPDATATOMSOT,0,reinterpret_cast<LPARAM>(pData));
					return TRUE;
				}
				::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_LEFT_DROPDATA,reinterpret_cast<WPARAM>(pData->sMode.c_str()),0);
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

HRESULT CLeftView::OnLoadMobile(WPARAM wParam,LPARAM lParam)
{
	if(wParam){
		//Add TreeItems
		HTREEITEM hParent = GetTreeCtrl().InsertItem(theApp.m_szMobileName, ILI_MOBILE, ILI_MOBILE);
		
		//set mobile root item data
		FileData *pData = new FileData;
		pData->sMode = MOBILE_PHONE;
		GetTreeCtrl().SetItemData(hParent,reinterpret_cast<DWORD>(pData));
		
		//insert memory child item
		TCHAR szText[MAX_PATH];
		HTREEITEM hTreeItem;
		if(al_GetSettingString(_T("public"),_T("IDS_MOBILE"),theApp.m_szRes,szText)){
			hTreeItem = GetTreeCtrl().InsertItem(szText,1,1,hParent);
		
			//set memory child item data
			pData = new FileData;
			pData->sMode = MEMORY;
			GetTreeCtrl().SetItemData(hTreeItem,reinterpret_cast<DWORD>(pData));
		}

		//insert sim card item 
		if(al_GetSettingString(_T("public"),_T("IDS_SIMCARD"),theApp.m_szRes,szText)){
			hTreeItem = GetTreeCtrl().InsertItem(szText,2,2,hParent);

			//set item data
			pData = new FileData;
			pData->sMode = SIM_CARD;
			GetTreeCtrl().SetItemData(hTreeItem,reinterpret_cast<DWORD>(pData));
		}

		GetTreeCtrl().Expand(hParent, TVE_EXPAND); 
		
		GetTreeCtrl().SetIndent(10);

		GetTreeCtrl().SelectItem(hParent);

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
	if(hGrand) //peggy
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
				xm	= m_nIndent / 2 +4, //peggy
				ym	= (rc.bottom - rc.top) / 2 ;

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
   		m_nIndent	= GetTreeCtrl().GetIndent();// / 3; //peggy
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
		if(rc.bottom - rc.top == 0 && rc.right - rc.left ==0)
			return;

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
		{ //peggy

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
		}//peggy
   		//
   		// If we have buttons or line, we must make room for them
   		//
		if(hParent)	// Leo
   			if(m_dwStyle & (TVS_HASBUTTONS | TVS_HASLINES))
   				rc.left += m_nIndent;

   		//
   		// Check if we have any normal icons to draw
   		//
   		rc.left += 4; // peggy
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

		//Set drawing text font 

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
   			::DrawFocusRect(hDC, &rcText);
		
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

void CLeftView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CTreeView::OnMouseMove(nFlags, point);
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
		//set root item count,only MOBILE_PHONE
		FileData *ptr = reinterpret_cast<FileData*>(GetTreeCtrl().GetItemData(hRoot));
		if(ptr && ptr->sMode == fd.sMode && fd.sMode == MOBILE_PHONE){
			TCHAR szTxt[MAX_PATH];
			wsprintf(szTxt,_T("%s (%d)"),theApp.m_szMobileName,iCount);
			GetTreeCtrl().SetItemText(hRoot,szTxt);
			break;
		}
		//get child item handle
		HTREEITEM hChild = GetTreeCtrl().GetChildItem(hRoot);
		while(hChild){
			//set child item count,SIM_CARD,MEMORY,CSVFILE
			ptr = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hChild));
			if(ptr && ptr->sMode == fd.sMode){
				//csvfile
				if(fd.sMode == CSVFILE && fd.iIndex == ptr->iIndex){
					TCHAR szTxt[MAX_PATH];
					Tstring sFileName = ptr->sFile.substr(ptr->sFile.find_last_of('\\') + 1);
					wsprintf(szTxt,_T("%s (%d)"),sFileName.c_str(),iCount);
					GetTreeCtrl().SetItemText(hChild,szTxt);
					break;
				}
				else if(fd.sMode == SIM_CARD){
					TCHAR szTxt[MAX_PATH],szText[MAX_PATH];
					if(al_GetSettingString(_T("public"),_T("IDS_SIMCARD"),theApp.m_szRes,szText)){
						wsprintf(szTxt,_T("%s (%d)"),szText,iCount);
						GetTreeCtrl().SetItemText(hChild,szTxt);
					}
					break;
				}
				else if(fd.sMode == MEMORY){
					TCHAR szTxt[MAX_PATH],szText[MAX_PATH];
					if(al_GetSettingString(_T("public"),_T("IDS_MOBILE"),theApp.m_szRes,szText)){
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

void CLeftView::CheckTreeItem()
{
	HTREEITEM hSMItem = NULL;
	HTREEITEM hMEItem = NULL;
	HTREEITEM hMobileItem = NULL;
//	if(((CPhoneBookApp*)afxGetApp())->m_bNotSupportSM || ((CPhoneBookApp*)afxGetApp())->m_bNotSupportME)
	{
		HTREEITEM hRoot = GetTreeCtrl().GetRootItem();
		while(hRoot && (hSMItem == NULL || hMEItem == NULL || hMobileItem == NULL))
		{
			FileData *ptr = reinterpret_cast<FileData*>(GetTreeCtrl().GetItemData(hRoot));
			//get child item handle
			if(ptr && ptr->sMode == MOBILE_PHONE)
				hMobileItem = hRoot;
			HTREEITEM hChild = GetTreeCtrl().GetChildItem(hRoot);
			while(hChild)
			{
				//set child item count,SIM_CARD,MEMORY,CSVFILE
				ptr = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hChild));
				if(ptr && ptr->sMode == MOBILE_PHONE)
					hMobileItem = hChild;
				if(ptr && ptr->sMode == SIM_CARD)
					hSMItem = hChild;
				if(ptr && ptr->sMode == MEMORY)
					hMEItem = hChild;
				//Get next item
				hChild = GetTreeCtrl().GetNextItem(hChild,TVGN_NEXT);
			}
			//Get next root
			hRoot = GetTreeCtrl().GetNextItem(hRoot,TVGN_NEXT);
		}

		if(hSMItem && ((CPhoneBookApp*)afxGetApp())->m_bNotSupportSM )
		{
			FileData *	ptr = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hSMItem));
			SAFE_DELPTR(ptr);
			GetTreeCtrl().DeleteItem(hSMItem);
		}
		if(hMEItem && ((CPhoneBookApp*)afxGetApp())->m_bNotSupportME )
		{
			FileData *	ptr = reinterpret_cast<FileData *>(GetTreeCtrl().GetItemData(hMEItem));
			SAFE_DELPTR(ptr);
			GetTreeCtrl().DeleteItem(hMEItem);
		}
		TCHAR szText[MAX_PATH];
		TCHAR szTxt[MAX_PATH];
		HTREEITEM hTreeItem;
		FileData *pData;
		if(((CPhoneBookApp*)afxGetApp())->m_bNotSupportME == false && hMEItem == NULL && hMobileItem)
		{
			if(al_GetSettingString(_T("public"),_T("IDS_MOBILE"),theApp.m_szRes,szText)){
				wsprintf(szTxt,_T("%s (0)"),szText);
				hTreeItem = GetTreeCtrl().InsertItem(szTxt,1,1,hMobileItem);
			
				//set memory child item data
				pData = new FileData;
				pData->sMode = MEMORY;
				GetTreeCtrl().SetItemData(hTreeItem,reinterpret_cast<DWORD>(pData));
			}
		}
		if(((CPhoneBookApp*)afxGetApp())->m_bNotSupportSM == false && hSMItem == NULL && hMobileItem)
		{
			if(al_GetSettingString(_T("public"),_T("IDS_SIMCARD"),theApp.m_szRes,szText)){
				wsprintf(szTxt,_T("%s (0)"),szText);
				hTreeItem = GetTreeCtrl().InsertItem(szTxt,2,2,hMobileItem);

				//set item data
				pData = new FileData;
				pData->sMode = SIM_CARD;
				GetTreeCtrl().SetItemData(hTreeItem,reinterpret_cast<DWORD>(pData));
			}
		}

	}
}
