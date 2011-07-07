// CheckThumbListCtrl.cpp : implementation file
//

//#include <wbemtime.h>

#include "stdafx.h"
#include "..\Phonebook.h"
#include "CheckThumbListCtrl.h"
#include "sortclass.h"
#include "shlwapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define ICON_WIDTH		32
#define ICON_HEIGHT		32

#define LISTITEM_BOLDER	2
#define LISTITEM_GAPX	2
#define LISTITEM_GAPY	2

#define ITEMFRAME_COLOR	RGB(200,200,255)
#define ITEMSELECT_COLOR	RGB(255,0,0)

/////////////////////////////////////////////////////////////////////////////
// CCheckThumbListCtrl

CCheckThumbListCtrl::CCheckThumbListCtrl()
{
	m_bUsePic =false;
	m_iLastClickCol = -1;
	m_bSelItem = false;
	m_iItemHeight = 0;
	m_crCol1 = 0;
	m_crCol2 = 0;
	m_pImageListThumb = NULL;
	m_pImageListReport = NULL;
	m_pImgBuffer = NULL;
}

CCheckThumbListCtrl::~CCheckThumbListCtrl()
{
}


BEGIN_MESSAGE_MAP(CCheckThumbListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CCheckThumbListCtrl)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemchanged)
	ON_NOTIFY_REFLECT(HDN_ITEMCHANGING, OnItemchanging)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCheckThumbListCtrl message handlers
void  
CCheckThumbListCtrl::DrawThumbnails()
{
	int nListCount = GetItemCount();
	
		//Set the item text
		for(int i = 0 ; i < nListCount ; i ++){
			CCardPanel *pData = reinterpret_cast<CCardPanel*>(GetItemData(i));

			//Get group name
			TCHAR szTxt[MAX_PATH];
/*			if(theApp.m_iMobileCompany == CO_MOTO)
				::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_CARDPANEL_GETGROUP,pData->GetGroup()-1,reinterpret_cast<LPARAM>(szTxt));
			else */
				::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_CARDPANEL_GETGROUP,pData->GetGroup(),reinterpret_cast<LPARAM>(szTxt));
			if(_tcslen(szTxt)<=0)
				pData->GetGroupText(szTxt);
			//get the default phone number
			Tstring sTel;
			DEQPHONEDATA* pDeqData = pData->GetPhoneData();
			int nGeneral , nMobile, nWork, nHome,nFax,nOther,nPager;
			nGeneral= nMobile= nWork= nHome=nFax=nOther=nPager = -1;
			int 	nMobileH,nMobileW,nFaxH,nFaxW,nPagerH,nPagerW,nVideo,nVideoH,nVideoW;
			int nAssistant,nBusiness,nCallback,nCar,nISDN,nPrimary,nRadio,nTelix,nTTYTDD;
			nMobileH=nMobileW=nFaxH=nFaxW=nPagerH=nPagerW=nVideo=nVideoH=nVideoW=-1;
			nAssistant=nBusiness=nCallback=nCar=nISDN=nPrimary=nRadio=nTelix=nTTYTDD = -1;
			int nIndex = 0;
			for(DEQPHONEDATA::iterator iter = pDeqData->begin() ; iter != pDeqData->end() ; iter ++)
			{
				int nType = (*iter).GetType();
				switch(nType)
				{
					case PBK_Number_General:
						nGeneral = nIndex;
						break;
					case PBK_Number_Mobile:
						nMobile = nIndex;
						break;
					case PBK_Number_Work:
						nWork = nIndex;
						break;
					case PBK_Number_Home:
						nHome = nIndex;
						break;
					case PBK_Number_Fax:
						nFax = nIndex;
						break;
					case PBK_Number_Other:
						nOther = nIndex;
						break;
					case PBK_Number_Pager:
						nPager = nIndex;
						break;
					case PBK_Number_Mobile_Home:
						nMobileH = nIndex;
						break;	
					case PBK_Number_Mobile_Work:
						nMobileW = nIndex;
						break;	
					case PBK_Number_Fax_Home:
						nFaxH = nIndex;
						break;	
					case PBK_Number_Fax_Work:
						nFaxW = nIndex;
						break;	
					case PBK_Number_Pager_Home:
						nPagerH = nIndex;
						break;	
					case PBK_Number_Pager_Work:
						nPagerW = nIndex;
						break;	
					case PBK_Number_VideoCall:
						nVideo = nIndex;
						break;	
					case PBK_Number_VideoCall_Home:
						nVideoH = nIndex;
						break;	
					case PBK_Number_VideoCall_Work:
						nVideoW = nIndex;
						break;	
					//new add
					case PBK_Number_Assistant:
						nAssistant = nIndex;
						break;
					case PBK_Number_Business:
						nBusiness = nIndex;
						break;
					case PBK_Number_Callback:
						nCallback = nIndex;
						break;
					case PBK_Number_Car:
						nCar = nIndex;
						break;
					case PBK_Number_ISDN:
						nISDN = nIndex;
						break;
					case PBK_Number_Primary:
						nPrimary = nIndex;
						break;
					case PBK_Number_Radio:
						nRadio = nIndex;
						break;
					case PBK_Number_Telix:
						nTelix = nIndex;
						break;
					case PBK_Number_TTYTDD:
						nTTYTDD = nIndex;
						break;
				}
				nIndex ++;
			}

			DEQPHONEDATA::iterator iter2;
			if(nGeneral != -1) iter2 =  pDeqData->begin() + nGeneral;
			else if(nMobile != -1) iter2 =  pDeqData->begin() + nMobile;
			else if(nWork != -1) iter2 =  pDeqData->begin() + nWork;
			else if(nHome != -1) iter2 =  pDeqData->begin() + nHome;
			else if(nFax != -1) iter2 =  pDeqData->begin() + nFax;
			else if(nOther != -1) iter2 =  pDeqData->begin() + nOther;
			else if(nPager != -1) iter2 =  pDeqData->begin() + nPager;
			else if(nVideo != -1) iter2 =  pDeqData->begin() + nVideo;
			else if(nMobileH != -1) iter2 =  pDeqData->begin() + nMobileH;
			else if(nFaxH != -1) iter2 =  pDeqData->begin() + nFaxH;
			else if(nPagerH != -1) iter2 =  pDeqData->begin() + nPagerH;
			else if(nVideoH != -1) iter2 =  pDeqData->begin() + nVideoH;
			else if(nMobileW != -1) iter2 =  pDeqData->begin() + nMobileW;
			else if(nFaxW != -1) iter2 =  pDeqData->begin() + nFaxW;
			else if(nPagerW != -1) iter2 =  pDeqData->begin() + nPagerW;
			else if(nVideoW != -1) iter2 =  pDeqData->begin() + nVideoW;

			else if(nBusiness != -1) iter2 =  pDeqData->begin() + nBusiness;
			else if(nPrimary != -1) iter2 =  pDeqData->begin() + nPrimary;
			else if(nAssistant != -1) iter2 =  pDeqData->begin() + nAssistant;
			else if(nCallback != -1) iter2 =  pDeqData->begin() + nCallback;
			else if(nCar != -1) iter2 =  pDeqData->begin() + nCar;
			else if(nISDN != -1) iter2 =  pDeqData->begin() + nISDN;
			else if(nRadio != -1) iter2 =  pDeqData->begin() + nRadio;
			else if(nTelix != -1) iter2 =  pDeqData->begin() + nTelix;
			else if(nTTYTDD != -1) iter2 =  pDeqData->begin() + nTTYTDD;
			
			if(nGeneral != -1 ||nMobile != -1 ||nWork != -1 ||nHome != -1 ||nFax != -1 ||nOther != -1 ||nPager != -1
				||nVideo != -1 ||nMobileH != -1||nFaxH != -1||nPagerH != -1||nVideoH != -1||nMobileW != -1||
				nFaxW != -1||nPagerW != -1||nVideoW != -1 ||
				nBusiness != -1||nPrimary != -1||nAssistant != -1||nCallback != -1||nCar != -1||nISDN != -1||
				nRadio != -1 || nTelix != -1 ||nTTYTDD != -1)
				sTel = (*iter2).GetText();
			/*	if((*iter).GetType() == PBK_Number_General || (*iter).GetType() == PBK_Number_Mobile
					|| (*iter).GetType() == PBK_Number_Work || (*iter).GetType() == PBK_Number_Home
					|| (*iter).GetType() == PBK_Number_Fax || (*iter).GetType() == PBK_Number_Other 
					|| (*iter).GetType() == PBK_Number_Pager){
					sTel = (*iter).GetText();
					break;
				}*/
		//	}

			if(m_bUsePic){
				TCHAR szName[MAX_PATH];
				memset(szName,0,sizeof(TCHAR) * MAX_PATH);
				pData->GetName(szName);
				SetItemText(i , 1 , szName);
				SetItemText(i , 2 , sTel.c_str());
				SetItemText(i , 3 , szTxt);
			}
			else{
				if(m_iMode == 0){
					TCHAR szName[MAX_PATH];
					memset(szName,0,sizeof(TCHAR) * MAX_PATH);
					pData->GetName(szName);
					SetItemText(i , 0 , szName);
				}
				
				SetItemText(i , 1 , sTel.c_str());
				SetItemText(i , 2 , szTxt);
			}
		}

	xSetImageListItem();

	SetRedraw(TRUE); 
}
/*
BOOL CCheckThumbListCtrl::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}
*/
void CCheckThumbListCtrl::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CCheckThumbListCtrl::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	int nItem = GetNextItem(-1,LVNI_SELECTED);
	if(nItem != -1){
		//get the selected item
		::PostMessage(GetParent()->GetSafeHwnd(),WM_TOOLBAR_NEWEDIT,1,0);

		//Modify edit data code below this
	}
	else
		::PostMessage(GetParent()->GetSafeHwnd(),WM_TOOLBAR_NEWEDIT,0,0);

	*pResult = 0;
}


BOOL CCheckThumbListCtrl::DeleteSel()
{
	//Check select item count larger then 0
	int nItem = GetNextItem(-1, LVNI_SELECTED);
	if(nItem == -1)
		return FALSE;

	SetRedraw(FALSE);

	//release all selected item data
/*	while(nItem != -1){
		CCardPanel *pData = reinterpret_cast<CCardPanel*>(GetItemData(nItem));
		pData->Release();
		nItem = GetNextItem(nItem, LVNI_SELECTED);
	}
*/
	//get all selected item indexes
	vector<int> vecSel;
	nItem = GetNextItem(-1,LVNI_SELECTED);
	while(nItem != -1){
		vecSel.push_back(nItem);
		nItem = GetNextItem(nItem,LVNI_SELECTED);
	}
	//delete all selected item and move all item below it.
	for(std::vector<int>::iterator iterSel = vecSel.end() - 1; iterSel != vecSel.begin() - 1; iterSel--){
		//After delete item,you just could delete the data
		CCardPanel *pData = reinterpret_cast<CCardPanel*>(GetItemData((*iterSel)));
		pData->Release();
		DeleteItem((*iterSel));
		SAFE_DELPTR(pData);
	}
	
	//Redraw the thumbs
	SetRedraw(TRUE);

	return TRUE;

}

void CCheckThumbListCtrl::RemoveAllData()
{
	//Release all item and delete them
	for(int i = 0 ; i < GetItemCount() ; i ++){
		CCardPanel *pData = reinterpret_cast<CCardPanel*>(GetItemData(i));
		pData->Release();
		SAFE_DELPTR(pData);
	}

	DeleteAllItems();

	SetRedraw(FALSE);

/*	if(m_pImageListThumb->GetSafeHandle())
		m_pImageListThumb->SetImageCount(0);
	
	if(m_pImageListReport->GetSafeHandle())
		m_pImageListReport->SetImageCount(0);
*/
	SetRedraw(TRUE);
}


void CCheckThumbListCtrl::OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CCheckThumbListCtrl::xRescaleItem()
{
	if(GetItemCount()==0)
		return;
	static	int nLastRow_Count = 0;
	CRect	rcWnd, rcItem;
	GetWindowRect(&rcWnd);
	GetItemRect( 0, &rcItem, LVIR_BOUNDS );
	int nRow_Count = rcWnd.Width() / (rcItem.Width() + LISTITEM_GAPX);
	if( nRow_Count==nLastRow_Count )
		return;
	int nX=0, nY=0;
	for(int i=0; i<this->GetItemCount(); i++)	{
		CPoint pt(nX*(rcItem.Width()+LISTITEM_GAPX), nY*(rcItem.Height()+LISTITEM_GAPY));
		SetItemPosition( i, pt );
		
		if(++nX==nRow_Count)	{
			++nY;	
			nX=0;
		}
	}
	nLastRow_Count = nRow_Count;
}

void CCheckThumbListCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CListCtrl::OnSize(nType, cx, cy);
}

BOOL CCheckThumbListCtrl::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	return CListCtrl::DestroyWindow();
}

void CCheckThumbListCtrl::OnDestroy() 
{
	RemoveAllData();
	SAFE_DELPTR(m_pImgBuffer);
	CListCtrl::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

bool CCheckThumbListCtrl::AddData(CCardPanel &data)
{

	CCardPanel *pData = new CCardPanel;
	*pData = data;
//	pData->ResetImgBuffer();
	pData->init();

	int iIndex = GetItemCount();
/*	if(strcmp(pData->GetStorageType(),MEMORY_NAME) == 0)
		::SendMessage(GetParent()->GetSafeHwnd(),WM_GETLISTSIZE,reinterpret_cast<WPARAM>(pData->GetStorageType()),
			reinterpret_cast<LPARAM>(&iIndex));
*/
	//Set data
	InsertItem( LVIF_IMAGE | LVIF_PARAM | LVIF_STATE | LVIF_TEXT, iIndex,
		NULL, 0, LVIS_FOCUSED | LVIS_SELECTED, 0, reinterpret_cast<LPARAM>(pData) );
	//Set ImageList
	xSetImageListItem();
	return true;
}

void CCheckThumbListCtrl::init(bool bPic)
{
	m_bUsePic = bPic;

	//Get profile path
	Tstring sProfile = theApp.m_szSkin + Tstring(_T("Organize\\list.ini"));

	//create the list control column
	LV_COLUMN lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;

	TCHAR szText[MAX_PATH];

	//set the skin header control
	if(GetHeaderCtrl()){
		m_SkinHeaderCtrl.SubclassWindow(GetHeaderCtrl()->m_hWnd);
		//Load profile skin
		SetHeaderControlProfile(sProfile.c_str(),_T("header_control"));
		//get font to determine the column heiht
		int iSize = 0; 
		TCHAR szName[MAX_PATH];
		GetProfileFont(const_cast<TCHAR*>(sProfile.c_str()),_T("header_control"),iSize,szName);
		al_GetSettingInt(_T("header_control"),_T("height"),const_cast<TCHAR*>(sProfile.c_str()),iSize);
		//get font handle
		HFONT hFont = GetFontEx(szName,iSize);
		m_font.FromHandle(hFont);
		//set font to skin headerctrl
		m_SkinHeaderCtrl.SetFont(&m_font);
	}

	if(m_bUsePic){
		if(al_GetSettingString(_T("public"),_T("IDS_PIC"),theApp.m_szRes,szText)){
			lvColumn.iSubItem = 0;
			lvColumn.pszText = szText;
			al_GetSettingInt(_T("column1"),_T("width0"),const_cast<TCHAR*>(sProfile.c_str()),lvColumn.cx);
			InsertColumn(0,&lvColumn);
			m_SkinHeaderCtrl.AddColText(szText);
		}

		if(al_GetSettingString(_T("public"),_T("IDS_NAME"),theApp.m_szRes,szText)){
			lvColumn.iSubItem = 1;
			lvColumn.pszText = szText;
			al_GetSettingInt(_T("column1"),_T("width1"),const_cast<TCHAR*>(sProfile.c_str()),lvColumn.cx);
			InsertColumn(1,&lvColumn);
			m_SkinHeaderCtrl.AddColText(szText);
		}

		if(al_GetSettingString(_T("public"),_T("IDS_TEL_NO"),theApp.m_szRes,szText)){
			lvColumn.iSubItem = 2;
			lvColumn.pszText = szText;
			al_GetSettingInt(_T("column1"),_T("width2"),const_cast<TCHAR*>(sProfile.c_str()),lvColumn.cx);
			InsertColumn(2,&lvColumn);
			m_SkinHeaderCtrl.AddColText(szText);
		}

		if(al_GetSettingString(_T("public"),_T("IDS_GROUP"),theApp.m_szRes,szText)){
			lvColumn.iSubItem = 3;
			lvColumn.pszText = szText;
			al_GetSettingInt(_T("column1"),_T("width3"),const_cast<TCHAR*>(sProfile.c_str()),lvColumn.cx);
			InsertColumn(3,&lvColumn);
			m_SkinHeaderCtrl.AddColText(szText);
		}
	}
	else{
		if(al_GetSettingString(_T("public"),_T("IDS_NAME"),theApp.m_szRes,szText)){
			lvColumn.iSubItem = -1;
			lvColumn.pszText = szText;
			al_GetSettingInt(_T("column2"),_T("width0"),const_cast<TCHAR*>(sProfile.c_str()),lvColumn.cx);
			InsertColumn(0,&lvColumn);
			m_SkinHeaderCtrl.AddColText(szText);
		}

		if(al_GetSettingString(_T("public"),_T("IDS_TEL_NO"),theApp.m_szRes,szText)){
			lvColumn.iSubItem = -1;
			lvColumn.pszText = szText;
			al_GetSettingInt(_T("column2"),_T("width1"),const_cast<TCHAR*>(sProfile.c_str()),lvColumn.cx);
			InsertColumn(1,&lvColumn);
			m_SkinHeaderCtrl.AddColText(szText);
		}

		if(al_GetSettingString(_T("public"),_T("IDS_GROUP"),theApp.m_szRes,szText)){
			lvColumn.iSubItem = -1;
			lvColumn.pszText = szText;
			al_GetSettingInt(_T("column2"),_T("width2"),const_cast<TCHAR*>(sProfile.c_str()),lvColumn.cx);
			InsertColumn(2,&lvColumn);
			m_SkinHeaderCtrl.AddColText(szText);
		}

	}
	m_iMode = 0;
	//get the item rectangle height
	CRect rect;
	InsertItem(0,NULL);
	CImageList il;
	il.Create(REPORT_WIDTH, REPORT_HEIGHT, ILC_COLOR24 | ILC_MASK , 0, 1);
	CListCtrl::SetImageList(&il,LVSIL_SMALL);
	GetItemRect(0,rect,LVIR_BOUNDS);
	m_iItemHeight = rect.Height();
	DeleteItem(0);
	CListCtrl::SetImageList(NULL,LVSIL_SMALL);
	//get column color
	al_GetSettingColor(_T("background"),_T("color1"),const_cast<TCHAR*>(sProfile.c_str()),m_crCol1);
	al_GetSettingColor(_T("background"),_T("color2"),const_cast<TCHAR*>(sProfile.c_str()),m_crCol2);
	//create the background image
	int cx = GetSystemMetrics(SM_CXFULLSCREEN);
	int cy = GetSystemMetrics(SM_CYFULLSCREEN);
	m_pImgBuffer = new CImage(cx,cy);
	SetViewMode(0);
}

void CCheckThumbListCtrl::xSetImageListItem()
{
	//Get the data list size and image list item count
	int iImage = m_pImageListThumb ? m_pImageListThumb->GetImageCount() : 0;
	int iCount = GetItemCount();
	if(iImage == 0 && iCount != 0){
		CImage *pImg = new CImage(THUMB_WIDTH,THUMB_HEIGHT);
		if(pImg){
			HBITMAP	hbm1;
			pImg->GetHBITMAP( Color::White, &hbm1 );
			CBitmap	   Bmp1;
			Bmp1.Attach( hbm1 );

			//add it to image list
			SetRedraw(FALSE);
			m_pImageListThumb->Add(&Bmp1,RGB(0, 0, 0));
		}
		SAFE_DELPTR(pImg);
	}
}

void CCheckThumbListCtrl::SetViewMode(int iMode)
{
	m_iMode = iMode;
	//get list window style
	UINT uStyle = GetWindowLong(GetSafeHwnd(),GWL_STYLE);
	//remove icon and report style
	uStyle &= ~LVS_REPORT & ~LVS_ICON;

	switch(iMode){
	case 0:{
		uStyle |= LVS_REPORT;

		Graphics grap(m_pImgBuffer);

		//two base column color
		Color color;
		color.SetFromCOLORREF(m_crCol1);
		SolidBrush brush1(color);
		color.SetFromCOLORREF(m_crCol2);
		SolidBrush brush2(color);
/*
		SCROLLINFO si;
		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		GetScrollInfo(SB_HORZ, &si);
		rect.left -= si.nPos;
*/
		//get header height
		CRect rc;
		GetHeaderCtrl()->GetWindowRect(rc);
		Rect rect(0,rc.Height(),m_pImgBuffer->Width(),m_iItemHeight);
		int iCount = m_pImgBuffer->Height() / m_iItemHeight;

		for(int i=0; i<iCount; i++)
		{
			grap.FillRectangle(i%2 ? &brush2 : &brush1 ,rect);
			rect.Y += rect.Height;
		}
		break;
		   }
	case 1:{
		uStyle |= LVS_ICON;
		Graphics grap(m_pImgBuffer);

		//two base column color
		Color color;
		color.SetFromCOLORREF(m_crCol1);
		SolidBrush brush1(color);
		Rect rect(0,0,m_pImgBuffer->Width(),m_pImgBuffer->Height());
		grap.FillRectangle(&brush1,rect);
		break;
		   }
/*	case 2:
		uStyle |= LVS_ICON;
		break;*/
	}
	SetWindowLong(GetSafeHwnd(),GWL_STYLE, uStyle);
	xChangeViewMode(iMode);
}

void CCheckThumbListCtrl::xChangeViewMode(int iMode)
{
	//report mode
	if(iMode == 0){
		if(!m_bUsePic){
			for(int i = 0 ; i < GetItemCount() ; i ++){
				CCardPanel *pData = reinterpret_cast<CCardPanel*>(GetItemData(i));
				TCHAR szName[MAX_PATH];
				memset(szName,0,sizeof(TCHAR) * MAX_PATH);
				pData->GetName(szName);	
				SetItemText(i,0,szName);
			}	
		}
	}
	else if(iMode == 1){	//card mode 
		//empty the text bellow the card
		for(int i = 0 ; i < GetItemCount() ; i ++){
			SetItemText(i,0,NULL);
		}
		
		Arrange(LVA_DEFAULT);
	}
}

void CCheckThumbListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar == 46)
	{	//Delete button
		::PostMessage(GetParent()->GetSafeHwnd(),WM_TOOLBAR_DELSEL,0,0);
	}
	
	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

//必須將字串轉換成為wchar_t才能進行比較
//若未轉換,則在多國字串比較將發生錯誤
int CCheckThumbListCtrl::FindItemData(LPCTSTR lpszText, int iStart, int iEnd)
{
	if(GetItemCount() == 0)
		return -1;
	if(iStart < 0 )
		return -1;
	if(iEnd < -1 )
		iEnd = -1;

	int iFindEnd = 0;
	if(iEnd == -1 )	iFindEnd = GetItemCount();
	for(int i = iStart ; i < iFindEnd ; i ++){
		CCardPanel *pData = reinterpret_cast<CCardPanel*>(GetItemData(i));		
		//search the substring in the data name
		//exchange char to wchar_t
		WCHAR wTxt[MAX_PATH] , wName[MAX_PATH];
		//initialize 
		memset(wTxt,0,sizeof(WCHAR) * MAX_PATH);
		memset(wName,0,sizeof(WCHAR) * MAX_PATH);
		//convert to unicode
		TCHAR szName[MAX_PATH];
		memset(szName,0,sizeof(TCHAR) * MAX_PATH);
		pData->GetName(szName);
#ifdef _UNICODE
		wsprintf(wTxt ,lpszText); 
		wsprintf(wName ,szName); 
#else
		MultiByteToWideChar(CP_ACP,0,lpszText,strlen(lpszText) + 1,wTxt,sizeof(wTxt)/sizeof(wTxt[0]) );
		MultiByteToWideChar(CP_ACP,0,szName,strlen(szName) + 1,wName,sizeof(wName)/sizeof(wName[0]) );
#endif
		TCHAR *pos=NULL;
	    pos=StrStrI(wName,wTxt);//080715libaoliou
		if (pos)
		{
			return i;
		}
// 		CString strDestName(wName);//080704libaoliu
// 		if (strDestName.Find(wTxt,0)!=-1)
// 		{
// 			return i;	
// 		}
		
// 		//use wstring
// 		wstring s1(wTxt);
// 		wstring s2(wName);
// 		wstring::iterator pos;
// 
// 		pos = search(s2.begin(),s2.end(),s1.begin(),s1.end(),nocase_compare);
// 		if(pos != s2.end())
// 			return i;	
	}
	return -1;
}

bool CCheckThumbListCtrl::GetSelectedData(CCardPanel &data)
{
	int iIndex = GetNextItem(-1,LVNI_SELECTED);
	if(iIndex != -1)
	{
		CCardPanel *pData = reinterpret_cast<CCardPanel*>(GetItemData(iIndex));
		data = *pData;
		return true;
	}
	return false;
}
bool SortIndexCallback ( CCardPanel elem1, CCardPanel elem2 )
{
	char szLoc1[MAX_PATH];
	char szLoc2[MAX_PATH];
	sprintf(szLoc1,"%s",elem1.GetLocation());
	sprintf(szLoc2,"%s",elem2.GetLocation());

	long nIndex1 = 	strtoul(szLoc1, NULL, 16);
	long nIndex2 = strtoul(szLoc2, NULL, 16);

	if(nIndex1 >= 0 && nIndex2 >=0)
	{
		return nIndex1 > nIndex2;
	}
	return strcmp(szLoc1,szLoc2) >0 ? true : false;
}


bool CCheckThumbListCtrl::GetSelectedData(DATALIST &lsData)
{
/*	int iIndex = GetNextItem(-1,LVNI_SELECTED);
	if(iIndex == -1)
		return false;
	while(iIndex != -1){
		CCardPanel *pData = reinterpret_cast<CCardPanel*>(GetItemData(iIndex));
		lsData.push_back(*pData);
		iIndex = GetNextItem(iIndex,LVNI_SELECTED);
	}
*/
	vector <CCardPanel> v1;

	int iIndex = GetNextItem(-1,LVNI_SELECTED);
	if(iIndex == -1)
		return false;
	while(iIndex != -1){
		CCardPanel *pData = reinterpret_cast<CCardPanel*>(GetItemData(iIndex));
		v1.push_back(*pData);
		iIndex = GetNextItem(iIndex,LVNI_SELECTED);
	}
	sort(v1.begin( ), v1.end( ),SortIndexCallback);
//	lsData.sort(greater<CCardPanel>() );
   vector <CCardPanel>::iterator Iter1;
   for ( Iter1 = v1.begin( ) ; Iter1 != v1.end( ) ; Iter1++ )
		lsData.push_back(*Iter1 );

	return true;
}

bool CCheckThumbListCtrl::SetSelectedData(CCardPanel &data)
{
	int iIndex =GetNextItem(-1,LVNI_SELECTED);
	if(iIndex != -1)
	{
		CCardPanel *pData = reinterpret_cast<CCardPanel*>(GetItemData(iIndex));
		*pData = data;
//		pData->ResetImgBuffer();
		pData->init();
		DrawThumbnails();
		xChangeViewMode(m_iMode);
		return true;
	}
	return false;

}

void CCheckThumbListCtrl::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	CSortClass csc(this , pNMListView->iSubItem , "C");
	if(pNMListView->iSubItem != m_iLastClickCol)
	{
		m_iLastClickCol = pNMListView->iSubItem;
		csc.Sort(TRUE);
	}
	else
	{
		m_iLastClickCol = -1;
		csc.Sort(FALSE);
	}

	*pResult = 0;
}

int CCheckThumbListCtrl::GetItemImage(int nItem)
{
	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE;
	lvi.iItem = nItem;
	lvi.iSubItem = 0;
	if(GetItem(&lvi) ==TRUE)
		return lvi.iImage;
	return -1;
}



int CCheckThumbListCtrl::GetSelectedCount()
{
	int iCount = 0 ;
	if(m_iMode == 1){
		for(int i = 0 ; i < GetItemCount(); i ++)
		{
			CCardPanel *pData = reinterpret_cast<CCardPanel*>(GetItemData(i));
			if(pData->GetSel())
				iCount ++;
		}
	}
	else if(m_iMode == 0)
	{
		iCount = CListCtrl::GetSelectedCount();
	}
	return iCount;
}

void CCheckThumbListCtrl::OnLButtonDown(UINT nFlags,CPoint point)
{
	SetFocus();
	int iItem = HitTest(point);
	if(iItem != -1)
		SetItemState(iItem,LVNI_FOCUSED,LVNI_FOCUSED);
	CListCtrl::OnLButtonDown(nFlags,point);
	Invalidate(FALSE);
}

void CCheckThumbListCtrl::OnItemchanging(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CCheckThumbListCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CListCtrl::OnLButtonUp(nFlags, point);
	Invalidate(FALSE);
}

int CCheckThumbListCtrl::HitTest( CPoint pt, UINT* pFlags)
{
	return CListCtrl::HitTest(pt,pFlags);
}

void CCheckThumbListCtrl::PreSubclassWindow() 
{
	CListCtrl::PreSubclassWindow();
}

bool CCheckThumbListCtrl::SetHeaderControlProfile(LPCTSTR lpszProfile,LPCTSTR sec)
{
	if(!lpszProfile)
		return false;
	
	GetHeaderControlFromSetting(&m_SkinHeaderCtrl,const_cast<TCHAR*>(sec),const_cast<TCHAR*>(lpszProfile));
	
	return true;
}

BOOL CCheckThumbListCtrl::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_pImgBuffer){
		if(m_iMode == 0){
			CRect rect,rcUsed;
			GetClientRect(rect);
			if(GetItemCount()>0)
			{
				CRect rcFirst;
				GetItemRect(0,rcFirst,LVIR_BOUNDS);
				if(rcFirst.top > rect.top)
				{
					m_pImgBuffer->DrawImage(pDC->GetSafeHdc(),rect.left,0, rect.Width(),rcFirst.top-rect.top,
						rect.left,0,rect.Width(),rcFirst.top-rect.top);
				}
			}
			if(GetItemRect(GetItemCount() - 1,rcUsed,LVIR_BOUNDS))
				rect.top = rcUsed.bottom;
			m_pImgBuffer->DrawImage(pDC->GetSafeHdc(),rect.left,rect.top,rect.Width(),rect.Height(),
				rect.left,rect.top,rect.Width(),rect.Height());
				return TRUE;
		}
	}
	return FALSE;
}

void CCheckThumbListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

bool CCheckThumbListCtrl::SetData(CCardPanel &data)
{
	CCardPanel *pData = new CCardPanel;
	*pData = data;
//	pData->ResetImgBuffer();
	pData->init();
	//Set data
	InsertItem( LVIF_IMAGE | LVIF_PARAM | LVIF_STATE | LVIF_TEXT, GetItemCount(),
		NULL, 0, LVIS_FOCUSED | LVIS_SELECTED, GetItemCount(), reinterpret_cast<LPARAM>(pData) );
	return true;
}

void CCheckThumbListCtrl::OnPaint()
{
	if(m_iMode == 1){
		CPaintDC dc(this);

		//get client rect
		CRect rect;
		GetClientRect(rect);
		//create the cimage buffer
		CImage img(rect.Width(),rect.Height());
		
		//create the rgn
		CRgn rgnView;
		rgnView.CreateRectRgnIndirect(rect);

		//draw background
		m_pImgBuffer->Draw(img,rect);
		
		//draw item
		for(int i = 0 ; i < GetItemCount() ; i ++){
			//get item rect
			CCardPanel *pData = reinterpret_cast<CCardPanel*>(GetItemData(i));
			BOOL bSel = GetItemState(i,LVIS_SELECTED) & LVIS_SELECTED;
			pData->SetSelect(bSel?true:false);

			CRect rcItem;
			if(GetItemRect(i,rcItem,LVIR_BOUNDS)){
				//offset all rectangle
				rcItem.OffsetRect(25,10);
				//check in the view 
				if(rgnView.RectInRegion(rcItem)){
					//draw in card mode
					if(pData){
						CImage *pimg = NULL;
						pData->GetDrawImage(&pimg);

						if(pimg){
							pimg->Draw(img, rcItem.left, rcItem.top,pimg->Width() , pimg->Height());						
							SAFE_DELPTR(pimg);
						}
					}
				}
			}
		}
		img.Display(dc.GetSafeHdc(),rect);
	}
	else
		CListCtrl::OnPaint();
}

