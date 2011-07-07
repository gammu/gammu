// PrevView.cpp : implementation file
//

#include "stdafx.h"
#include "SMSUtility.h"
#include "PrevView.h"
#include "MainFrm.h"
#include "LeftView.h"
#include "NewSMS.h"
#include "OpenSMS.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PRINTMARGIN 2


#define SMS_Item	255


static bool bchangeColor = true;

CPrevView*	CPrevView::s_ActivePrev = NULL;

/////////////////////////////////////////////////////////////////////////////
// CPrevView

IMPLEMENT_DYNCREATE(CPrevView, CListView)

CPrevView::CPrevView()
{
	m_cxImage = m_cyImage = m_bIsDragging = 0;//m_CurSubItem = 0;
    m_nDragTarget = m_nDragItem = -1;

	m_CtrlKeyDown = false;
	m_hFont = NULL;
	//m_SMSInfo = new SMS_PARAM[SMS_Item];
	//memset(m_SMSInfo, 0, sizeof(SMS_PARAM)*SMS_Item);

	s_ActivePrev = this;
	m_nSortCol = -1;
}

CPrevView::~CPrevView()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}


BEGIN_MESSAGE_MAP(CPrevView, CListView)
	//{{AFX_MSG_MAP(CPrevView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_COMMAND(ID_BN_SAVE, OnSave)
	ON_COMMAND(ID_BN_RESEND,OnReSend)
	ON_COMMAND(ID_BN_PRINT,OnSMSPrint)
	ON_COMMAND(ID_BN_DELETE,OnDelete)
	ON_UPDATE_COMMAND_UI(ID_BN_SAVE, OnUpdateSave)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	ON_UPDATE_COMMAND_UI(ID_BN_RESEND, OnUpdateBnResend)
	ON_UPDATE_COMMAND_UI(ID_BN_PRINT, OnUpdatePrint)
	ON_UPDATE_COMMAND_UI(ID_BN_DELETE, OnUpdateDelete)
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_COMMAND(ID_BN_REPLY, OnBnReply)
	ON_UPDATE_COMMAND_UI(ID_BN_REPLY, OnUpdateBnReply)
	ON_WM_KEYUP()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
	//ON_MESSAGE(WM_TOOLBAR_DELSEL,OnToolbarDelSel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrevView drawing

/*void CPrevView::OnPaint()
{
	CPaintDC dc(this);
	// TODO: add draw code here
	//CListCtrl& refCtrl = GetListCtrl();
	//refCtrl.InsertItem(0, "Item!");
}*/

void CPrevView::OnDraw(CDC* pDC)
{
}


/////////////////////////////////////////////////////////////////////////////
// CPrevView message handlers


BOOL CPrevView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if(!CListView::PreCreateWindow(cs))
		return FALSE;

	cs.style |= LVS_REPORT | WS_BORDER;
	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	return true;//CListView::PreCreateWindow(cs);
}

int CPrevView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;


	TCHAR  szProfile[_MAX_PATH], szAppPath[_MAX_PATH];
	al_GetModulePath(NULL, szAppPath);
	wsprintf(szProfile, _T("%s%s"), szAppPath, _T("skin\\default\\font.ini"));

	//Load font setting for ComboBox control
/*	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = theApp.m_lf.lfHeight;
	lf.lfCharSet = theApp.m_lf.lfCharSet; 
	//lf.lfHeight = (atoi(szFontSize)*4)/3;
	_tcscpy(lf.lfFaceName, theApp.m_lf.lfFaceName);
	if(lf.lfHeight > 0)
		lf.lfHeight *= -1;
	if(m_hFont==NULL)
		m_hFont = ::CreateFontIndirect(&lf);*/
	int iSize = 12;
	if(theApp.m_lf.lfHeight > 0)
		iSize = -theApp.m_lf.lfHeight ;
	else
		iSize = theApp.m_lf.lfHeight ;
	m_hFont = GetFontEx(theApp.m_lf.lfFaceName,iSize);

	CFont* pFont = CFont::FromHandle(m_hFont);

	GetListCtrl().SetFont(pFont);

	CRect rect;
	GetClientRect(&rect);

	TCHAR szTmp[MAX_PATH];
	al_GetSettingString(_T("public"), _T("IDS_CONTENT"), theApp.m_szRes, szTmp);
	GetListCtrl().InsertColumn(0, szTmp, LVCFMT_LEFT , 0,-1); 
		GetListCtrl().SetColumnWidth(0, 300);

	al_GetSettingString(_T("public"), _T("IDS_SENDER"), theApp.m_szRes, szTmp);
	GetListCtrl().InsertColumn(1, szTmp, LVCFMT_LEFT , 0,-1); 
		GetListCtrl().SetColumnWidth(1, 200);

	al_GetSettingString(_T("public"), _T("IDS_TIME"), theApp.m_szRes, szTmp);
	GetListCtrl().InsertColumn(2, szTmp, LVCFMT_LEFT , 0,-1); 
		GetListCtrl().SetColumnWidth(2, 200);

//	GetListCtrl().InsertColumn(3, "", LVCFMT_LEFT , 0,0); 

	GetListCtrl().SetExtendedStyle(LVS_EX_FULLROWSELECT );
	init();

	return 0;
}

void CPrevView::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
// 	if(GetListCtrl().GetSafeHwnd())
// 	{
// 		GetListCtrl().SetColumnWidth(0, cx / 2);
// 		GetListCtrl().SetColumnWidth(1, cx / 4);
// 		GetListCtrl().SetColumnWidth(2, cx / 4);
// 		GetListCtrl().SetColumnWidth(3, cx / 1);
// 	}//080605libaoliu
}

#include "ReportCtrl.h"
void CPrevView::ShowItems(SMS_PARAM *smsInfo, int nShow, int nItemCounts)
{
	//init();
	TCHAR szTmp[MAX_PATH];
	if(nShow == itMOBILE_SIMCARD_OUTBOX 
	   || nShow == itMOBILE_HANDSET_OUTBOX
	   || nShow == itPC_OUTBOX 
	   || nShow == itPC_SENDBK
	   || nShow == itPC_DRAFT)
	{
		al_GetSettingString(_T("public"), _T("IDS_RECEIVER_"), theApp.m_szRes, szTmp);
			m_SkinHeaderCtrl.AddColText(szTmp, 1);
	}
	else if ((nShow==0))
	{

	}
	else //080601libaoliu
	{
		
		al_GetSettingString(_T("public"), _T("IDS_SENDER"), theApp.m_szRes, szTmp);
			m_SkinHeaderCtrl.AddColText(szTmp, 1);
	}
//	m_SkinHeaderCtrl.AddColText(szTmp, 1);//080601libaoliu

	


	DeleteAllItems();
	m_nCurrentItem = nShow;

	m_SMSInfo = smsInfo;

	if(nItemCounts == 0)
	{
		SMS_PARAM smsInfo2;

		_tcscpy(smsInfo2.TP_UD, _T(""));
		_tcscpy(smsInfo2.TP_SCTS, _T(""));
		_tcscpy(smsInfo2.TPA, _T(""));

		AddToListView(&smsInfo2);

		GetListCtrl().GetItemRect(0, &m_rRect, LVIR_BOUNDS);
		DeleteAllItems();
	}
	else
	{
		for(int i = nItemCounts -1; i >= 0 ; i--)
		{
			AddToListView(&smsInfo[i]);
		}
	}
	
	
}

void CPrevView::DeleteAllItems()
{	
	GetListCtrl().DeleteAllItems();
}

void CPrevView::AddToListView(SMS_PARAM *smsInfo)
{
	LV_ITEM	lvitem;
	int iActualItem;
	CString sText;

	TCHAR strContent[MAX_CONTENT+1];
	_tcscpy(strContent , smsInfo->TP_UD);

	lvitem.mask = LVIF_TEXT | LVIF_IMAGE;
	lvitem.iItem = 0;
	lvitem.iSubItem = 0;
	lvitem.pszText =  strContent; //  fd.cFileName;
	iActualItem = GetListCtrl().InsertItem(&lvitem);

	TCHAR strSender[MAX_PATH];
	_tcscpy(strSender , smsInfo->TPA);
	// Add Attribute column
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iActualItem;
	lvitem.iSubItem = 1;
	lvitem.pszText = strSender; 
	GetListCtrl().SetItem(&lvitem);

	
	TCHAR strTime[MAX_PATH];
	_tcscpy(strTime , smsInfo->TP_SCTS);

	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iActualItem;
	lvitem.iSubItem = 2;
	lvitem.pszText = strTime;
	GetListCtrl().SetItem(&lvitem);

}


void CPrevView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CLeftView *saveTree = ((CMainFrame*)GetParentFrame())->GetLeftPane();
	HTREEITEM hCurSelItem = saveTree->GetTreeCtrl().GetSelectedItem();
	int nItems;
	SMS_PARAM *sms_Info = saveTree->GetTreeSMSInfo(hCurSelItem, nItems);

	POSITION pos = GetListCtrl().GetFirstSelectedItemPosition();
	int itemPosition = GetListCtrl().GetNextSelectedItem(pos);

	if(itemPosition < 0)	// leo 2004.10.18, If Click In the blank list
	{
		::PostMessage(((CMainFrame*)GetParentFrame())->m_hWnd,WM_COMMAND,ID_BN_NEW,0);
		return;
	}
	bool bSenderString = false;

	CLeftView *LeftTree = ((CMainFrame*)GetParentFrame())->GetLeftPane();
	HTREEITEM hCurSelTreeItem = (LeftTree->GetTreeCtrl()).GetSelectedItem();
	int nCurSelItem = LeftTree->GetSelectItem(hCurSelTreeItem);

	if(nCurSelItem == itMOBILE_SIMCARD_OUTBOX 
	   || nCurSelItem == itMOBILE_HANDSET_OUTBOX
	   || nCurSelItem == itPC_OUTBOX 
	   || nCurSelItem == itPC_SENDBK
	   || nCurSelItem == itPC_DRAFT)
	{
		bSenderString = true;
	}

	COpenSMS OpenSMSDlg;
	OpenSMSDlg.SetData(sms_Info, itemPosition, nItems,bSenderString);
	OpenSMSDlg.DoModal();

	
	*pResult = 0;
}


BOOL CPrevView::GetSelectedItem(LV_ITEM *lvi, int iWhichItem)
{
// Was something actually selected?
    POSITION pos = GetListCtrl().GetFirstSelectedItemPosition();
	if(pos == NULL)
		return FALSE;

	int iItem = GetListCtrl().GetNextSelectedItem(pos);

	memset(lvi,0,sizeof(lvi));
	lvi->mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi->iItem = iItem;
	lvi->iSubItem = iWhichItem;
	return(GetListCtrl().GetItem(lvi));
}



void CPrevView::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	/*CString cDataType;
	if((pNMListView->iSubItem == 0) || (pNMListView->iSubItem == 1))
		cDataType = "C";

	if(pNMListView->iSubItem == 2)
		cDataType = "N";

	if(pNMListView->iSubItem == 3)
		cDataType = "D";
	
	CSortClass csc(&GetListCtrl(), pNMListView->iSubItem, cDataType);
	csc.Sort(TRUE);*/

	
	CString cDataType;
	if(pNMListView->iSubItem == 0)
		cDataType = "C";

	if(pNMListView->iSubItem == 1)
		cDataType = "S";

	if(pNMListView->iSubItem == 2)
		cDataType = "T";

	if(m_nSortCol == pNMListView->iSubItem)
		m_bSortAscending = !m_bSortAscending ;
	else
		m_bSortAscending = TRUE;
	m_nSortCol = pNMListView->iSubItem;

	CLeftView *LeftTree = ((CMainFrame*)GetParentFrame())->GetLeftPane();
	LeftTree->xSortItem(cDataType,m_bSortAscending);

	*pResult = 0;
}


void CPrevView::OnReSend()
{
	CNewSMS	ResendSMSDlg;

	SMS_PARAM msgInfo;
	LV_ITEM lvi;
	TCHAR buffer[MAX_CONTENT+1];

	lvi.pszText = buffer;
	lvi.cchTextMax = sizeof(buffer);
	if(!GetSelectedItem(&lvi, 0))
		return;
	else
		_tcscpy(msgInfo.TP_UD, lvi.pszText);

	if(!GetSelectedItem(&lvi, 1))
		return;
	else
		_tcscpy(msgInfo.TPA, lvi.pszText);
	

	ResendSMSDlg.SetData(msgInfo, SMSRpely, true,true);
	ResendSMSDlg.DoModal();
}

void CPrevView::OnSave()
{
	//Show FileSave Dialog to save sms messages
	CLeftView *saveTree = ((CMainFrame*)GetParentFrame())->GetLeftPane();
	HTREEITEM hCurSelItem = saveTree->GetTreeCtrl().GetSelectedItem();
	int nItems;
	SMS_PARAM *sms_Info = saveTree->GetTreeSMSInfo(hCurSelItem, nItems);

	saveTree->xSaveSMS(sms_Info);
}

void CPrevView::OnUpdateSave(CCmdUI* pCmdUI) 
{
	 //-- Get the number of selected rows
	int nSelRows = GetListCtrl().GetSelectedCount();

	if(!nSelRows) //-- If there are no rows selected,jump out here
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
	
}


void CPrevView::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	LPNMITEMACTIVATE  lplvcd = (LPNMITEMACTIVATE)pNMHDR;

	/*if(lplvcd->iItem%2)
		bchangeColor = false;
	else
		bchangeColor = true;*/
	
	*pResult = 0;
}


void CPrevView::OnSMSPrint() 
{
	//IndexUnit unitIndex(this);
	CLeftView *saveTree = ((CMainFrame*)GetParentFrame())->GetLeftPane();
	saveTree->xFilePrintSMS();
}


void CPrevView::OnDelete()
{
	//::PostMessage(m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_TOOLBAR_DELSEL,0,0);
	CLeftView *saveTree = ((CMainFrame*)GetParentFrame())->GetLeftPane();
	HTREEITEM hCurSelItem = saveTree->GetTreeCtrl().GetSelectedItem();
	int nItems;
	SMS_PARAM *sms_Info = saveTree->GetTreeSMSInfo(hCurSelItem, nItems);

	saveTree->xDeleteSMS(hCurSelItem, sms_Info);	
}

void CPrevView::OnUpdatePrint(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	int nSelRows = GetListCtrl().GetSelectedCount();

	if(!nSelRows) //-- If there are no rows selected,jump out here
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
	
}

void CPrevView::OnUpdateDelete(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CLeftView *LeftTree = ((CMainFrame*)GetParentFrame())->GetLeftPane();
	int nSelRows = GetListCtrl().GetSelectedCount();

	if(!nSelRows) //-- If there are no rows selected,jump out here
		pCmdUI->Enable(FALSE);
	else if(theApp.nIsConnected != true && m_nCurrentItem == itMOBILE_SIMCARD_INBOX)
		pCmdUI->Enable(FALSE);
	else if(theApp.nIsConnected != true && m_nCurrentItem == itMOBILE_HANDSET_INBOX)
		pCmdUI->Enable(FALSE);
	else if(theApp.nIsConnected != true && m_nCurrentItem == itMOBILE_SIMCARD_OUTBOX)
		pCmdUI->Enable(FALSE);
	else if(theApp.nIsConnected != true && m_nCurrentItem == itMOBILE_HANDSET_OUTBOX)
		pCmdUI->Enable(FALSE);
#ifdef ASUSM303
	else if(theApp.nIsConnected != true && m_nCurrentItem == itMOBILE_HANDSET_OTHER1)
		pCmdUI->Enable(FALSE);
	else if(theApp.nIsConnected != true && m_nCurrentItem == itMOBILE_HANDSET_OTHER2)
		pCmdUI->Enable(FALSE);
	else if(theApp.nIsConnected != true && m_nCurrentItem == itMOBILE_HANDSET_OTHER3)
		pCmdUI->Enable(FALSE);
	else if(theApp.nIsConnected != true && m_nCurrentItem == itMOBILE_HANDSET_OTHER4)
		pCmdUI->Enable(FALSE);
	//else if(theApp.nIsConnected != true && m_nCurrentItem == itMOBILE_HANDSET_OTHER5)
	//	pCmdUI->Enable(FALSE);
#endif
	else if((m_nCurrentItem == itMOBILE_SIMCARD_INBOX  || m_nCurrentItem == itMOBILE_SIMCARD_OUTBOX )&& 
		LeftTree->m_sms_Tal_Num.bSIMSMSReadOnly)
		pCmdUI->Enable(FALSE);
	else if((m_nCurrentItem == itMOBILE_HANDSET_INBOX  || m_nCurrentItem == itMOBILE_HANDSET_OUTBOX )&& 
		LeftTree->m_sms_Tal_Num.bMESMSReadOnly)
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
	
}

void CPrevView::OnUpdateBnResend(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	int nSelRows = GetListCtrl().GetSelectedCount();

	if(nSelRows<=0) //-- If there are no rows selected,jump out here
		pCmdUI->Enable(FALSE);
	else
	{
		if(m_nCurrentItem != itPC_DRAFT) 
			pCmdUI->Enable(FALSE);
		else
			pCmdUI->Enable(TRUE);
	}
	
}


void CPrevView::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	*pResult = 0;

	CSMSUtility *theApp = (CSMSUtility*)afxGetApp();

	CLeftView *LeftTree = ((CMainFrame*)GetParentFrame())->GetLeftPane();
	m_hCurSelItem = LeftTree->GetTreeCtrl().GetSelectedItem();

    CListCtrl &cList = GetListCtrl();

    //set source of drag:
    theApp->cListItemDragIndex = ((NM_LISTVIEW *)pNMHDR)->iItem;
	theApp->cTreeItemDrag = m_hCurSelItem;

	LV_ITEM lvi;
	TCHAR buffer[MAX_PATH];
	lvi.pszText = buffer;
	lvi.cchTextMax = sizeof(buffer);

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.iItem = theApp->cListItemDragIndex;
	lvi.iSubItem = 0;

	GetListCtrl().GetItem(&lvi);

	// Create the drag&drop source and data objects
   	COleDataSource ds;

	if(ds.DoDragDrop() == DROPEFFECT_MOVE )
	{
	}

}


void CPrevView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
/*	CSMSUtility *theApp = (CSMSUtility*)afxGetApp();

	//if(GetCapture() == this)
	{
		if( theApp->cDragging )
		{
			POINT   pt  = point;
			ClientToScreen(&pt);
        
			//Move the image:
			theApp->cpDragImage->DragMove(pt);
        
			//Get drop window:
			theApp->cpDragImage->DragShowNolock(FALSE);
			theApp->cpDropWnd = WindowFromPoint(pt);
			theApp->cpDropWnd->ScreenToClient(&pt);
			theApp->cpDragImage->DragShowNolock(TRUE);

			//Get tree:
			CLeftView *LeftTree = ((CMainFrame*)GetParentFrame())->GetLeftPane();
			CTreeCtrl &cTree = LeftTree->GetTreeCtrl();

			cTree.SelectDropTarget(NULL);

			m_hCurSelItem = cTree.GetSelectedItem();
		}
		
	}*/



    //Standardmethode:
	CListView::OnMouseMove(nFlags, point);
}

void CPrevView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CListView::OnLButtonDown(nFlags, point);
}


void CPrevView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	/*CSMSUtility *theApp = (CSMSUtility*)afxGetApp();
	if( theApp->cDragging )
    {
        //Initialize end dragging:
        theApp->cDragging = FALSE;
        theApp->cpDragImage->DragLeave(GetDesktopWindow());
        theApp->cpDragImage->EndDrag();
        
        //GET THE WINDOW UNDER THE DROP POINT
        theApp->cDropPoint = point;
        ClientToScreen(&theApp->cDropPoint);
        theApp->cpDropWnd = WindowFromPoint(theApp->cDropPoint);

        //Cancel if source and target are same:

		theApp->cpDragImage->DeleteImageList();
		delete theApp->cpDragImage;
		theApp->cpDragImage = NULL;
		
		ReleaseCapture();
    }*/

	CListView::OnLButtonUp(nFlags, point);
}



void CPrevView::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
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
			if(lpitat->ptAction.y%2)
			{
				lplvcd->clrTextBk = RGB(223,227,239);
			}
			else
			{
				lplvcd->clrTextBk = RGB(255,255,255);
			}

			// If you want the sub items the same as the item,
			// set *pResult to CDRF_NEWFONT
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
			return;
		}

		// Modify sub item text and/or background
		case CDDS_SUBITEM | CDDS_PREPAINT | CDDS_ITEM:
		{
			/*if(lplvcd->iSubItem /4 == 2)
			{
				lplvcd->clrTextBk = RGB(255,255,255);
			}
			else
			{
				lplvcd->clrTextBk = RGB(0,255,255);
				
			}*/

			*pResult = CDRF_NEWFONT;
			return;
		}
	}

}


BOOL CPrevView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rect;
	GetClientRect(rect);

#ifdef ASUSM303
	//CBrush brush(RGB(255, 255, 255));	// ipng modify in 20041217
	//pDC->FillRect(&rect, &brush);		// Solve the flash screen
#endif

	CBrush brush1(RGB(223, 227, 239));
	CBrush brush0(RGB(255, 255, 255));

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(SCROLLINFO));
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS;
	GetScrollInfo(SB_HORZ, &si);
	rect.left -= si.nPos;

	RECT	rRect;
	memset(&rRect, 0, sizeof(RECT));
	GetListCtrl().GetItemRect(0, &rRect, LVIR_BOUNDS);

	if(rRect.right == 0 && rRect.bottom ==0)
	{
		memcpy(&rRect, &m_rRect, sizeof(RECT));
		rRect.right = 1280;
	}

	if(rRect.top > rect.top)
	{
		CRect rect1;
		rect1.SetRect(rRect.left,rect.top,rRect.right,rRect.top);
		pDC->FillRect(&rect1, &brush0);
	}
	int nGrip = rRect.bottom - rRect.top;
	if(rRect.right < rect.right)
		rRect.right = rect.right;

	//for(int i=0; i<=GetListCtrl().GetHeaderCtrl()->GetItemCount(); i++)
	for(int i=0; i<255; i++)
	{
		pDC->FillRect(&rRect, i%2 ? &brush1 : &brush0);
		rRect.top = rRect.bottom;
		rRect.bottom += nGrip;
	}

  
	brush0.DeleteObject();
	brush1.DeleteObject();

	return FALSE;
}


void CPrevView::init()
{
//get list profile path
	m_sProfile = Tstring(theApp.m_szSkin) + Tstring(_T("SMSUtility\\main.ini"));
	
	//Set profile path
	
// before Insertcolumn
	if(GetListCtrl().GetHeaderCtrl()){
		m_SkinHeaderCtrl.SubclassWindow(GetListCtrl().GetHeaderCtrl()->m_hWnd);
		//Load profile skin
		SetHeaderControlProfile(m_sProfile.c_str(),_T("header_control"));
		//get font to determine the column heiht
		int iSize = 0; 
		TCHAR szName[MAX_PATH];
		GetProfileFont(const_cast<TCHAR*>(m_sProfile.c_str()),_T("header_control"),iSize,szName);
		al_GetSettingInt(_T("header_control"),_T("height"),const_cast<TCHAR *>(m_sProfile.c_str()),iSize);
		//get font handle
		///HFONT hFont = GetFontEx(szName, iSize);
		int nHight = theApp.m_lf.lfHeight * (-1);
		HFONT hFont = GetFontEx(theApp.m_lf.lfFaceName, (int&)nHight);
		m_font.FromHandle(hFont);
		//set font to skin headerctrl
		m_SkinHeaderCtrl.SetFont(&m_font);
	}

//add this function after each "InsertColumn" function,"szText" is column text.

	TCHAR szTmp[MAX_PATH];
	al_GetSettingString(_T("public"), _T("IDS_CONTENT"), theApp.m_szRes, szTmp);
	m_SkinHeaderCtrl.AddColText(szTmp);
	al_GetSettingString(_T("public"), _T("IDS_SENDER"), theApp.m_szRes, szTmp);
	m_SkinHeaderCtrl.AddColText(szTmp);
	al_GetSettingString(_T("public"), _T("IDS_TIME"), theApp.m_szRes, szTmp);
	m_SkinHeaderCtrl.AddColText(szTmp);
	m_SkinHeaderCtrl.AddColText(_T(""));
}

bool CPrevView::SetHeaderControlProfile(LPCTSTR lpszProfile,LPCTSTR sec)
{
	if(!lpszProfile)
		return false;
	
	GetHeaderControlFromSetting(&m_SkinHeaderCtrl,const_cast<LPTSTR>(sec),const_cast<LPTSTR>(lpszProfile));
	
	return true;
}

void CPrevView::OnBnReply() 
{
	// TODO: Add your command handler code here
	CNewSMS	ResendSMSDlg;

	SMS_PARAM msgInfo;
	LV_ITEM lvi;
	TCHAR buffer[MAX_PATH];

	lvi.pszText = buffer;
	lvi.cchTextMax = sizeof(buffer);
	if(!GetSelectedItem(&lvi, 0))
		return;
	else
		_tcscpy(msgInfo.TP_UD, lvi.pszText);

	if(!GetSelectedItem(&lvi, 1))
		return;
	else
		_tcscpy(msgInfo.TPA, lvi.pszText);
	

	ResendSMSDlg.SetData(msgInfo, SMSRpely, true,false);
	ResendSMSDlg.DoModal();
}

void CPrevView::OnUpdateBnReply(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	int nSelRows = GetListCtrl().GetSelectedCount();

	if(!nSelRows) //-- If there are no rows selected,jump out here
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
// 	{
// 		if(m_nCurrentItem == itPC_DRAFT) 
// 			pCmdUI->Enable(FALSE);
// 		else
// 			pCmdUI->Enable(TRUE);
// 	}
}

void CPrevView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if(nChar == VK_CONTROL)
		m_CtrlKeyDown = false;
	
	CListView::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CPrevView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if(nChar == 46){	//Delete button
		PostMessage(WM_COMMAND,ID_BN_DELETE,0);
	}
	
	// true is Move, false is Copy
	if(nChar == VK_CONTROL)
		m_CtrlKeyDown = true;

	CListView::OnKeyDown(nChar, nRepCnt, nFlags);
}

bool CPrevView::xMoveData(void) 
{
	return m_CtrlKeyDown;
}

void CPrevView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class	
}




