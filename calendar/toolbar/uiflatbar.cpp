////////////////////////////////////////////////////////////////
// Copyright 1998 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
// CFlatToolBar implements dockable flat-style toolbars with "grippers"
//
#include "StdAfx.h"
#include "UIFlatBar.h"
#include "UIModulVer.h"
#include "..\Calendar.h"

//#include "ximage.h"
//#include "../utility/libs.h"

// if you want to see extra TRACE diagnostics, set below to TRUE
BOOL CFlatToolBar::bTRACE   = FALSE;

#ifdef _DEBUG
#define FBTRACEFN			\
	CTraceFn __fooble;	\
	if (bTRACE)				\
		TRACE
#define FBTRACE			\
	if (bTRACE)		      \
		TRACE
#else
#define FBTRACEFN TRACE
#define FBTRACE   TRACE
#endif

/////////////////
// One of these for each drop-down button
//
struct DROPDOWNBUTTON {
	DROPDOWNBUTTON* next;
	UINT idButton;								 // command ID of button
	UINT idMenu;								 // popup menu to display
};

// these define size of grippers
const GRIP_WIDTH   = 3;
const GRIP_MARGIN  = 5;

// flags stored in item data
#define	ITEMF_INITIALIZED	0x01 // item data initialized
#define	ITEMF_CHECKED		0x02 // item is checked

/*#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif*/

////////////////////////////////////////////////////////////////
// CFlatToolBar--does flat tool bar in MFC.
//
IMPLEMENT_DYNAMIC(CFlatToolBar, CFlatToolBarBase)

BEGIN_MESSAGE_MAP(CFlatToolBar, CFlatToolBarBase)
	ON_NOTIFY_REFLECT(TBN_DROPDOWN,  OnTbnDropDown)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_NCCALCSIZE()
	ON_WM_ERASEBKGND()
	ON_WM_NCCREATE()
	ON_WM_PAINT()
	ON_WM_CREATE()
END_MESSAGE_MAP()

CFlatToolBar::CFlatToolBar()
{
	FBTRACE(_T("CFlatToolBar::CFlatToolBar, comctl32 version = %d\n"),
		iVerComCtl32);

	m_bDrawDisabledButtonsInColor = FALSE; // don't use color
	m_bInCoolBar = FALSE;						// assume not inside coolbar
	m_pDropDownButtons = NULL;					// list of drop-down buttons
	m_bShowDropdownArrowWhenVertical = TRUE;
	m_bNoEntry = FALSE;

	m_hFont = NULL;
}

CFlatToolBar::~CFlatToolBar()
{
	while (m_pDropDownButtons) 
	{
		DROPDOWNBUTTON* pnext = m_pDropDownButtons->next;
		delete m_pDropDownButtons;
		m_pDropDownButtons = pnext;
	}

	if(m_hFont)
		::DeleteObject(m_hFont);
}

/////////////////
// Create handler: set flat style by default
//
int CFlatToolBar::OnCreate(LPCREATESTRUCT lpcs)
{
	if (CFlatToolBarBase::OnCreate(lpcs) == -1)
		return -1;

	ModifyStyle(0, TBSTYLE_FLAT);

	// set window font
	if(m_hFont)
		::DeleteObject(m_hFont);

//	m_hFont = CreateFont("Arial", 11, FALSE);
	int iSize = 0;
	TCHAR szFontName[MAX_PATH];
	GetProfileFont(NULL,NULL,iSize,szFontName);
	m_hFont = GetFontEx(szFontName,iSize);
	//m_hFont = CreateFont(g_pFaceName, 11, FALSE);
	SendMessage(WM_SETFONT, (WPARAM)m_hFont, 0);

	return 0;  // OK
}

////////////////
// Load function sets flat style after loading buttons.
//
BOOL CFlatToolBar::LoadToolBar(LPCTSTR lpszResourceName)
{
	// Set transparent/flat style before loading buttons to allow zero-height
	// border. This required because of bug in comctl32.dll that always adds
	// a border, unless flat/transparent.
	//
	DWORD dwStyle = GetStyle();
	ModifyStyle(0, TBSTYLE_FLAT|TBSTYLE_TRANSPARENT);
	BOOL bRet = CFlatToolBarBase::LoadToolBar(lpszResourceName);
	SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
	return bRet;
}

//////////////////
// Calcluate size of client area. Adds room for grippers
//
void CFlatToolBar::OnNcCalcSize(BOOL bCalc, NCCALCSIZE_PARAMS*	pncp)
{
	if (m_bInCoolBar) { // if I am in a coolbar (rebar):
		Default();		  // ..bypass CToolBar/CControlBar

	} else {
		CRect& rc = (CRect&)pncp->rgrc[0]; // rect to return

		// copied from MFC below:
		CRect rcMargin(0,0,0,0);
		CControlBar::CalcInsideRect(rcMargin, m_dwStyle & CBRS_ORIENT_HORZ);

		// adjust non-client area for border space
		rc.left  += rcMargin.left;
		rc.top   += rcMargin.top; // MFC has -2 here, bug for newer comctl32
		rc.right += rcMargin.right;
		rc.bottom+= rcMargin.bottom;
	}
}

//////////////////
// MFC doesn't handle moving a TBSTYLE_FLAT toolbar correctly. The simplest
// way to fix it is to repaint whatever was underneath whenever the toolbar
// moves. This is done in this and the following function. All this stuff is
// only required because flat toolbars paint transparently (don't paint their
// backgrounds).
// 
void CFlatToolBar::OnWindowPosChanging(LPWINDOWPOS lpwp)
{
	if (m_bInCoolBar)
		// inside coolbars, don't do MFC thing
		Default();
	else {
		CFlatToolBarBase::OnWindowPosChanging(lpwp);
		if (!(lpwp->flags & SWP_NOMOVE))
			GetWindowRect(&m_rcOldPos);		 // remember old position
	}
}

//////////////////
// Now toolbar has really moved: repaint area beneath old position
//
void CFlatToolBar::OnWindowPosChanged(LPWINDOWPOS lpwp)
{
	if (m_bInCoolBar) {
		Default();
	} else {
		CFlatToolBarBase::OnWindowPosChanged(lpwp);
		if (!(lpwp->flags & SWP_NOMOVE)) {	 // if moved:
			InvalidateOldPos(m_rcOldPos);		 // invalidate area of old position

			// Now paint my non-client area at the new location.
			// Without this, you will still have a partial display bug (try it!)
			SendMessage(WM_NCPAINT);
		}
	}
}

//////////////////
// Invalidate toolbar rectangle. Because flat toolbars are transparent,
// this requires invalidating parent and all siblings that intersect the
// rectangle.
//
void CFlatToolBar::InvalidateOldPos(const CRect& rcInvalid)
{
	// make parent paint the area beneath rectangle
	CWnd* pParent = GetParent();		// parent (dock bar/frame) window
	ASSERT_VALID(pParent);				// check
	CRect rc = rcInvalid;				// copy rectangle
	pParent->ScreenToClient(&rc);		// convert to parent client coords
	pParent->InvalidateRect(&rc);		// invalidate

	// now do same for each sibling too
	for (CWnd* pSib = pParent->GetWindow(GW_CHILD);
		  pSib;
		  pSib=pSib->GetNextWindow(GW_HWNDNEXT)) {

		CRect rc;										// window rect of sibling
		pSib->GetWindowRect(&rc);					// ...
		if (rc.IntersectRect(rc, rcInvalid)) {	// if intersects invalid rect
			pSib->ScreenToClient(&rc);				// convert to sibling coords
			pSib->InvalidateRect(&rc);				// invalidate
			pSib->SendMessage(WM_NCPAINT);		// nonclient area too!
		}
	}
}

////////////////
// Override to avoid MFC in case I'm inside a coolbar
//
BOOL CFlatToolBar::OnEraseBkgnd(CDC* pDC)
{
	return m_bInCoolBar ? Default() : CFlatToolBarBase::OnEraseBkgnd(pDC);
}

//////////////////
// If toolbar is inside a coolbar, need to make the parent frame
// my owner so it will get notifications.
//
BOOL CFlatToolBar::OnNcCreate(LPCREATESTRUCT lpcs)
{
	CWnd* pParent = GetParent();
	ASSERT(pParent);
	TCHAR classname[64];
	GetClassName(pParent->m_hWnd, classname, countof(classname));
	if(_tcscmp(classname, REBARCLASSNAME) == 0) 
	{
		CFrameWnd* pFrame = GetParentFrame();
		ASSERT_VALID(pFrame);
		SetOwner(pFrame);
		m_bInCoolBar = TRUE;
	}
	return CFlatToolBarBase::OnNcCreate(lpcs);
}

//////////////////
// Avoid MFC if I'm inside a coolbar
//
void CFlatToolBar::OnPaint()
{
	if (m_bInCoolBar)
		Default();	// bypass CToolBar/CControlBar
	else
		CFlatToolBarBase::OnPaint();
}

////////////////////////////////////////////////////////////////
// Stuff for handling drop-down buttons in toobars
//

//////////////////
// Add dropdown buttons.
// The manager takes care of setting appropriate styles, etc.
//
// Args:
//		- array of LONGs: MAKELONG(commandID, menuID)
//		- number of buttons
//
BOOL CFlatToolBar::AddDropDownButton(UINT nIDButton, UINT nIDMenu, BOOL bArrow)
{
	ASSERT_VALID(this);

	DROPDOWNBUTTON* pb = FindDropDownButton(nIDButton);
	if (!pb) {
		pb = new DROPDOWNBUTTON;
		ASSERT(pb);
		pb->next = m_pDropDownButtons;
		m_pDropDownButtons = pb;
	}
	pb->idButton = nIDButton;
	pb->idMenu   = nIDMenu;

	int iButton = CommandToIndex(nIDButton);

	DWORD dwStyle = GetButtonStyle(iButton);
	dwStyle |= TBSTYLE_DROPDOWN;
	SetButtonStyle(iButton, dwStyle);

	if (bArrow)
		SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);

	return TRUE;
}

//////////////////
// Find buttons structure for given ID
//
DROPDOWNBUTTON* CFlatToolBar::FindDropDownButton(UINT nID)
{
	for (DROPDOWNBUTTON* pb = m_pDropDownButtons; pb; pb = pb->next) {
		if (pb->idButton == nID)
			return pb;
	}
	return NULL;
}

//////////////////
// Handle TBN_DROPDOWN
// Default is to display the specified menu at the right place.
// You can override to generate dynamic menus
//
// Args:
//		- NMTOOLBAR struct from TBN_DROPDOWN
//		- command id of button
//		- point to display menu at
//
void CFlatToolBar::OnTbnDropDown(NMHDR* pNMHDR, LRESULT* pRes)
{
	const NMTOOLBAR& nmtb = *(NMTOOLBAR*)pNMHDR;

	// get location of button
	CRect rc;
	GetRect(nmtb.iItem, rc);
	ClientToScreen(&rc);

	// call virtual function to display dropdown menu
	OnDropDownButton(nmtb, nmtb.iItem, rc);
}

/////////////////
// Virtual fn you can override to hand drop-down button
// events with more friendly args
//
void CFlatToolBar::OnDropDownButton(const NMTOOLBAR& nmtb, UINT nID, CRect rc)
{
	DROPDOWNBUTTON* pb = FindDropDownButton(nmtb.iItem);
	if (pb && pb->idMenu) {

		// load and display popup menu
		CMenu menu;
		VERIFY(menu.LoadMenu(pb->idMenu));
		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup);
		pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_VERTICAL,
			rc.left, rc.bottom, GetOwner(), &rc);
	}
}


////////////////////////////////////////////////////////////////////////////////
// CMyToolBar
//
IMPLEMENT_DYNAMIC(CMyToolBar, CFlatToolBar)

BEGIN_MESSAGE_MAP(CMyToolBar, CFlatToolBar)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


CMyToolBar::CMyToolBar()
{
	// load button image
	m_nBtnCount = 1;
	m_pBtnBk = NULL;
	
	m_pImgBk = NULL;

	for(int i = 0; i < 4; i ++)
		m_pBtnIcon[i] = NULL;
	
	m_bUseBkImg = false;
	m_crBk = 0;
	m_bInitmemDC = FALSE;
}

CMyToolBar::~CMyToolBar()
{
	if(m_pBtnBk)
		delete m_pBtnBk;

	for(int i = 0; i < 4; i ++)
	{
		if(m_pBtnIcon[i])
			delete m_pBtnIcon[i];
	}
	UninitMemDC();
	SAFE_DELPTR(m_pImgBk);
}

void CMyToolBar::SetButtonBk(LPCTSTR pBkFile)
{
	if(m_pBtnBk)
	{
		delete m_pBtnBk;
		m_pBtnBk = NULL;
	}

	if(pBkFile)
	{
		CString sAppPath(theApp.m_szModule);

		//m_pBtnBk = new CImageArray(sAppPath + "skin\\default\\Organize\\" + pBkFile, 4);
		CString csImgPath = /*sAppPath + "skin\\default\\Calendar\\" + */pBkFile;
		csImgPath.Replace(_T("\\\\"), _T("\\"));
		m_pBtnBk = new CImageArray(csImgPath.GetBuffer(MAX_PATH), 4);//nono, 1004_1101

		if(m_pBtnBk)
		{
			m_pBtnBk->SetBorder(10, m_pBtnBk->Height(), 10, 0);
		}
	}
}

void CMyToolBar::SetButtonImage(LPCTSTR pBtnFile, int nCount)
{
	for(int i = 0; i < 4; i ++)
	{
		if(m_pBtnIcon[i])
			delete m_pBtnIcon[i];
	}

	m_nBtnCount = nCount;

	if(pBtnFile)
	{
		CString sAppPath(theApp.m_szModule);
		//传递过来的是按钮图片名称的前半部分，加上后面的部分合成全文见名称。
		//在根据传递过来的按钮数量生成imagearray
		// load button image
		CString sBtnFn[4] = {_T("_normal.png"), _T("_down.png"), _T("_hl.png"), _T("_gray.png")};
		for(int i = 0; i < 4; i ++)
		{
			CString sBtnImg = /*sAppPath + "skin\\default\\Calendar\\" + */pBtnFile + sBtnFn[i];
			sBtnImg.Replace(_T("\\\\"), _T("\\"));
			m_pBtnIcon[i] = new CImageArray(sBtnImg, m_nBtnCount);
		}
	}
}
/*----------------------------------------------------------------
下面消息响应实现toolbar的重画 其中用到了自定义结构体，如下
typedef struct _NMTBCUSTOMDRAW {
			NMCUSTOMDRAW nmcd;
			HBRUSH hbrMonoDither;
			HBRUSH hbrLines;					 // For drawing lines on buttons
			HPEN hpenLines;						 // For drawing lines on buttons

			COLORREF clrText;					 // Color of text
			COLORREF clrMark;					 // Color of text bk when marked. (only if TBSTATE_MARKED)
			COLORREF clrTextHighlight;	   	     // Color of text when highlighted
			COLORREF clrBtnFace;				 // Background of the button
			COLORREF clrBtnHighlight;			 // 3D highlight
			COLORREF clrHighlightHotTrack;		 // In conjunction with fHighlightHotTrack
												 // will cause button to highlight like a menu
			RECT rcText;                         // Rect for text
  
			int nStringBkMode;
			int nHLStringBkMode;
#if (_WIN32_WINNT >= 0x501)
			int iListGap;
#endif
} NMTBCUSTOMDRAW, * LPNMTBCUSTOMDRAW;

-----------------------------------------------------------------*/

void CMyToolBar::OnCustomDraw(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTBCUSTOMDRAW lpNMCustomDraw = (LPNMTBCUSTOMDRAW)pNMHDR;

    switch(lpNMCustomDraw->nmcd.dwDrawStage)
    {
		/*
		dwDrawStage 当前绘制状态，：		
		  CDDS_POSTERASE 擦除循环结束
		  CDDS_POSTPAINT 绘制循环结束 
		  CDDS_PREERASE 准备开始擦除循环 				
		  CDDS_PREPAINT 准备开始绘制循环 				  
		  CDDS_ITEM 指定dwItemSpec, uItemState, lItemlParam参数有效 					
		  CDDS_ITEMPOSTERASE 列表项擦除结束 					  
		  CDDS_ITEMPOSTPAINT 列表项绘制结束 						
		  CDDS_ITEMPREERASE 准备开始列表项擦除 						  
		  CDDS_ITEMPREPAINT 准备开始列表项绘制 
		  CDDS_SUBITEM 指定列表子项
		*/
        case CDDS_PREPAINT:		//准备开始绘制循环
		{
            *pResult = CDRF_NOTIFYITEMDRAW;
				
			NMCUSTOMDRAW& nmcd = lpNMCustomDraw->nmcd;
			CDC* pDC = CDC::FromHandle(nmcd.hdc);
			Graphics graphics(pDC->GetSafeHdc());	

			CRect rect;
			GetClientRect(rect);

/*			if(m_bUseBkImg){
				m_pImgBk->Display(graphics,rect,DRAW_TILE);
			}
			else{
				Rect rcClient(rect.left,rect.top,rect.Width(),rect.Height());
				Color color;
				color.SetFromCOLORREF(m_crBk);
				SolidBrush blackBrush(color);

				graphics.FillRectangle(&blackBrush, rcClient);
			}*/
			if(!m_bInitmemDC)
				InitMemDC();
			pDC->BitBlt(rect.left,rect.top,rect.Width(),rect.Height(),&m_memDC[4],
					rect.left,rect.top,SRCCOPY);
			
			break;
		}
        case CDDS_ITEMPREPAINT:	//准备开始列表项绘制
        {
	if(!m_bInitmemDC)
		InitMemDC();
            NMCUSTOMDRAW& nmcd = lpNMCustomDraw->nmcd;
            CDC* pDC = CDC::FromHandle(nmcd.hdc);
            UINT nID = nmcd.dwItemSpec;
			int iButton = CommandToIndex(nID);
			UINT nStyle;
			int iImage;

			GetButtonInfo(iButton, nID, nStyle, iImage);
			//TRACE("Command = %d, iImage = %d\n", nID, iImage);

//			Graphics graphics(pDC->GetSafeHdc());
			//if(m_pBtnBk)	//eagle120503 : For Coolbar
			{
				CRect rc(nmcd.rc);
			//	rc.left += 2;
			//	rc.right -= 2;

				int index;

				if((nmcd.uItemState & CDIS_SELECTED) || (nmcd.uItemState & CDIS_CHECKED))
					index = 1;
				else if(nmcd.uItemState & CDIS_HOT)
					index = 2;
				else if((nmcd.uItemState & CDIS_DISABLED) || (nmcd.uItemState & CDIS_GRAYED) ||
					    (nmcd.uItemState & CDIS_INDETERMINATE))
					index = 3;
				else 
					index = 0;

				// draw button background
//				m_pBtnBk->Draw(graphics, rc.left, rc.top, rc.Width(), rc.Height(), index, DRAW_TILE9);	//eagle120503* : For Coolbar
//				if(m_pBtnBk)
//					m_pBtnBk->Draw(graphics, rc.left, rc.top, rc.Width(), rc.Height(), index, DRAW_TILE9);	//eagle120503&

				// draw button icon
				if(iImage < m_nBtnCount)
				{
/*					m_pBtnIcon[index]->Draw(graphics, rc.left + 7, rc.top + 2, 
								m_pBtnIcon[index]->GetItemWidth(), m_pBtnIcon[index]->GetItemHeight(), 
								iImage, DRAW_NORMAL);
*/
					pDC->BitBlt(rc.left , rc.top, 
								m_pBtnIcon[index]->GetItemWidth(), m_pBtnIcon[index]->GetItemHeight(),&m_memDC[index],
								iImage*m_pBtnIcon[index]->GetItemWidth() ,0, SRCCOPY);

				}

				// draw button text
				CString sText = GetButtonText(iButton);
				if(!sText.IsEmpty())
				{
					CRect rect = rc;
					rect.left += m_pBtnIcon[index]->GetItemWidth() + 4;

					CFont *pFont = GetFont();
					CFont *pOldFont = pDC->SelectObject(pFont);

					pDC->SetTextColor((index == 3) ? RGB(192, 192, 192) : RGB(0, 0, 0));
					pDC->SetBkMode(TRANSPARENT);
					pDC->DrawText(sText, -1, rect, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

					pDC->SelectObject(pOldFont);
				}

			}

            *pResult = CDRF_SKIPDEFAULT;
            break;
        }

        default:
            *pResult = CDRF_DODEFAULT;
            break;
    }
}

void CMyToolBar::OnPaint() 
{
	CFlatToolBar::OnPaint();
}

void CMyToolBar::SetColorBk(COLORREF &cr)
{
	m_crBk = cr;
}

void CMyToolBar::SetImgBk(LPCTSTR lpszPath, bool bUseBK)
{
	SAFE_DELPTR(m_pImgBk);
	m_pImgBk = new CImage(lpszPath);
	m_bUseBkImg = bUseBK;
}

void CMyToolBar::SetImgBk(CImage *pImg, bool bUseBK)
{
	SAFE_DELPTR(m_pImgBk);
	m_pImgBk = pImg;
	m_bUseBkImg = bUseBK;
	
}

BOOL CMyToolBar::OnEraseBkgnd(CDC* pDC)		//在toolbar重绘之前调用，显示背景图片或是使用背景颜色
{
	//return false;
	Graphics graphics(pDC->GetSafeHdc());	

	CRect rect;
	GetClientRect(rect);

	if(m_bUseBkImg)
	{
		m_pImgBk->Display(graphics,rect,DRAW_TILE);
	}
	else
	{
		Rect rcClient(rect.left,rect.top,rect.Width(),rect.Height());
		Color color;
		color.SetFromCOLORREF(m_crBk);
		SolidBrush blackBrush(color);

		graphics.FillRectangle(&blackBrush, rcClient);
	}

	return TRUE;
}

BOOL CMyToolBar::PreTranslateMessage(MSG* pMsg) 
{
	// 只与tooltip有关系
	InitToolTip();
	if( pMsg->message== WM_LBUTTONDOWN ||
        pMsg->message== WM_LBUTTONUP ||
        pMsg->message== WM_MOUSEMOVE)

	m_ToolTip.RelayEvent(pMsg);		
	return CFlatToolBar::PreTranslateMessage(pMsg);
}

// Set the tooltip with a string resource
void CMyToolBar::SetToolTipText(int nId,int nToolID, BOOL bActivate)
{
	CString sText, sTip;

	// load string resource
	sText.LoadString(nId);
	int index = sText.FindOneOf(_T("\n"));
	if(index >= 0)
	{
		sTip = sText.Right(sText.GetLength() - index - 1);
		sText.SetAt(index, 0);
//		m_sStatus = sText;
	}
	else 
	{
		sTip = sText;
//		m_sStatus.Empty();
	}

	// If string resource is not empty
	if (sTip.IsEmpty() == FALSE) 
	{
		SetToolTipText((LPCTSTR)sTip, bActivate);
	}
}

// Set the tooltip with a CString
void CMyToolBar::SetToolTipText(LPCTSTR pText,int nToolID, BOOL bActivate)
{
	// We cannot accept NULL pointer
	if (pText == NULL) return;

	// Initialize ToolTip
	InitToolTip();

	// If there is no tooltip defined then add it
	UINT nID = GetItemID(nToolID);

	CRect rectBtn; 
	GetItemRect(nToolID,rectBtn);
	m_ToolTip.AddTool(this, pText, rectBtn, nID);

	// Set text for tooltip
	m_ToolTip.UpdateTipText(pText, this, nID);
	m_ToolTip.Activate(bActivate);
}

void CMyToolBar::InitToolTip()
{
	if (m_ToolTip.m_hWnd == NULL)
	{
		// Create ToolTip control
		m_ToolTip.Create(this,TTS_ALWAYSTIP);

		// Create inactive
		m_ToolTip.Activate(FALSE);
//		SetToolTips(&m_ToolTip);
	}
}

// Activate the tooltip
void CMyToolBar::ActivateTooltip(BOOL bActivate)
{
	// If there is no tooltip then do nothing
	if (m_ToolTip.GetToolCount() == 0) 
		return;

	// Activate tooltip
	m_ToolTip.Activate(bActivate);
}

void CMyToolBar::OnDestroy()
{
	m_ToolTip.DestroyWindow();
	CFlatToolBar::OnDestroy();
}

/*----------------------------------------------
重绘toolbar因为有多个控件需要重绘，所以采用cdc缓冲的方法
先将需要使用的cd准备好，在需要绘制时直接使用
这样可以防止画面的闪烁
-----------------------------------------------*/
void CMyToolBar::InitMemDC()
{
	if(!m_bInitmemDC)	//初始化为false
	{
		CRect rect;
		GetClientRect(&rect);
		CDC *pDC = GetDC();
		CBitmap*	oldBitmap;
		//下面循环初始化前4个memDC 因为按钮只有4个状态
		for(int i = 0;i<4;i++)
		{
			m_memDC[i].CreateCompatibleDC(pDC);
			//每个btnicon是4幅图，所以创建的bitmap也是4个图标的组合
			m_bitmap[i].CreateCompatibleBitmap(pDC, m_pBtnIcon[i]->Width(), m_pBtnIcon[i]->Height());
			//将工具条（可能有多个按钮）的4种状态存入dc
			oldBitmap = m_memDC[i].SelectObject(&m_bitmap[i]);
			Graphics g(m_memDC[i].m_hDC);
			Rect rcClient(0,0,m_pBtnIcon[i]->Width(),m_pBtnIcon[i]->Height());
			if(m_bUseBkImg)
			{
				CRect rcDest;
				rcDest.SetRect(0,0,m_pBtnIcon[i]->Width(),m_pBtnIcon[i]->Height());
				//将背景图片显示出来
				m_pImgBk->Display(g,rcDest,DRAW_TILE);

			}
			else
			{
				Color color;
				color.SetFromCOLORREF(m_crBk);
				SolidBrush blackBrush(color);
				g.FillRectangle(&blackBrush, rcClient);
			}
			//根据不同的状态画按钮的图标
			g.DrawImage(m_pBtnIcon[i],rcClient,0,0,rcClient.Width,rcClient.Height,UnitPixel,NULL,NULL,NULL);
		}

		//	for Bk Image
		// 下面这个dc用来绘制背景
		m_memDC[4].CreateCompatibleDC(pDC);
		// 第二个参数指定背景的大小，尽量大些
		m_bitmap[4].CreateCompatibleBitmap(pDC, 2480, rect.Height());
		oldBitmap = m_memDC[4].SelectObject(&m_bitmap[4]);
		Graphics g(m_memDC[4].m_hDC);
		if(m_bUseBkImg)
		{
			CRect rcDest;
			rcDest.SetRect(0,0,2480,rect.Height());//2480

			m_pImgBk->Display(g,rcDest,DRAW_TILE);

		}
		else
		{
			Rect rcClient(0,0,2480,rect.Height());
			Color color;
			color.SetFromCOLORREF(m_crBk);
			SolidBrush blackBrush(color);

			g.FillRectangle(&blackBrush, rcClient);
		}
		ReleaseDC(pDC);
		m_bInitmemDC = TRUE;
	}

}

void CMyToolBar::UninitMemDC()
{
	if(m_bInitmemDC)
	{
		for(int i =0 ;i <5 ;i++)
		{
		if(m_bitmap[i].GetSafeHandle())
			m_bitmap[i].DeleteObject();
		}
		m_bInitmemDC = FALSE;
	}
}

// DLL CONTRUCT
void CMyToolBar::SendMegToToolTip(UINT message,UINT nFlags,CPoint point) 
{
    MSG msg;
    msg.hwnd = m_hWnd;
    msg.message = message;
    msg.wParam = nFlags;
    msg.lParam = MAKELONG(point.x, point.y);
    InitToolTip();
	m_ToolTip.RelayEvent(&msg);		
}
void CMyToolBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SendMegToToolTip(WM_LBUTTONDOWN,nFlags,point);
	CFlatToolBar::OnLButtonDown(nFlags, point);
}
void CMyToolBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	SendMegToToolTip(WM_LBUTTONUP,nFlags,point);
	CFlatToolBar::OnLButtonUp(nFlags, point);
}
void CMyToolBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	SendMegToToolTip(WM_MOUSEMOVE,nFlags,point);
	CFlatToolBar::OnMouseMove(nFlags, point);
}