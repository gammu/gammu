// SkinHeaderCtrl.cpp : implementation file
//

#include "stdafx.h"
//#include "SkinList.h"
#include "SkinHeaderCtrl.h"
//#include "memdc.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSkinHeaderCtrl

CSkinHeaderCtrl::CSkinHeaderCtrl()
{

	m_bEnable = TRUE;

	m_iHover = -1;
	m_iDown = -1;
	m_bTracking = FALSE;

	m_bShowText = FALSE;
	m_nAlignment = BA_CENTER;

	m_hFont = NULL;
	m_crText = GetSysColor(COLOR_BTNTEXT);
	m_crDText = GetSysColor(COLOR_BTNTEXT);
	m_crHText = GetSysColor(COLOR_HIGHLIGHTTEXT);
	m_crHDText = GetSysColor(COLOR_HIGHLIGHTTEXT);
	m_crGText = GetSysColor(COLOR_GRAYTEXT);
	m_crBk = GetSysColor(COLOR_WINDOW);


	m_nCount = 4;
	for(int i = 0 ; i < 3 ; i ++)
		m_pImg[i] = NULL;

	m_bNormalLook = FALSE;

	m_nNormalLook = 1;

	m_bButtonType = TRUE;
	m_nOffset = 0;
	m_nTxtOffset = 0;

	m_bDownNotify = FALSE;
	m_bUpNotify = FALSE;
	m_bSafeDel = TRUE;

	m_bTextLine = FALSE;
	m_TextShadowSize = CSize(0,0);
}

CSkinHeaderCtrl::~CSkinHeaderCtrl()
{
	if( m_bSafeDel){
		for(int i = 0 ; i < 3 ; i ++)
			SAFE_DELPTR(m_pImg[i]);
	}

	if(m_hFont)
		::DeleteObject(m_hFont);

	for(int i = 0 ; i < m_ayImgIcon.GetSize() ; i ++){
		SAFE_DELPTR(m_ayImgIcon[i]);
	}
	m_ayImgIcon.RemoveAll();

	m_ayiIcon.RemoveAll();
	m_ayStrCol.RemoveAll();
}

IMPLEMENT_DYNAMIC(CSkinHeaderCtrl, CHeaderCtrl)

BEGIN_MESSAGE_MAP(CSkinHeaderCtrl, CHeaderCtrl)
	//{{AFX_MSG_MAP(CSkinHeaderCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSkinHeaderCtrl message handlers

BOOL CSkinHeaderCtrl::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}


void CSkinHeaderCtrl::SetTextFont(CString sFacename, int nHeight, UINT fType, BYTE lfQuality)
{
	if(m_hFont)
		::DeleteObject(m_hFont);

	m_hFont = al_xCreateFont2(sFacename, nHeight, fType, lfQuality);
}

void CSkinHeaderCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
/*	//get all parts width and height 
	int cx = 0,cy = 0,Lcx = 0,Lcy = 0,Rcx = 0,Rcy = 0,Ccx = 0,Ccy  = 0;
	if(m_pImgIcon)
	{
		cx = m_pImgIcon->Width() / m_nCount;
		cy = m_pImgIcon->Height();
	}
	if(m_pImg[0])
	{
		Lcx = m_pImg[0]->Width() / m_nCount;
		Lcy = m_pImg[0]->Height();
	}
	if(m_pImg[1])
	{
		Ccx = m_pImg[1]->Width() / m_nCount;
		Ccy = m_pImg[1]->Height();
	}
	if(m_pImg[2])
	{
		Rcx = m_pImg[2]->Width() / m_nCount;
		Rcy = m_pImg[2]->Height();
	}
	//get the item rectangle
	CRect rcItem;
	GetItemRect(lpDrawItemStruct->itemID,rcItem);
	int iCenterWidth = rcItem.Width() - Lcx - Rcx;
	//create the background image
	CImage *pimg = new CImage(rcItem.Width(), rcItem.Height());
	if(!pimg)
		return;

	Graphics graphics(pimg);
	Rect rc(0, 0, cx, cy);	//Get the m_pImgIcon draw rectangle
	Rect rc1(rcItem.left,rcItem.top,rcItem.Width(),rcItem.Height());	//get the button draw rectangle

	//draw the background
	Color color;
	color.SetFromCOLORREF(m_crBk);
	pimg->FillColor(color);
	
	Rect rcCenter(Lcx,0,iCenterWidth,rcItem.Height());	//center rectangle
	Rect rcLeft(0,0,Lcx,rcItem.Height());	//left rectangle
	Rect rcRight(Lcx+iCenterWidth,0,Rcx,rcItem.Height());	//right rectangle

	if(m_bEnable == FALSE) //gray
	{
		int index = m_bNormalLook ? m_nNormalLook : 3;

		if(m_pImg[1]){
			int nCount = static_cast<int>(static_cast<float>(rcCenter.Width) / static_cast<float>(Ccx));
			for(int i = 0 ; i < nCount ; i ++){
				Rect rcTemp(Lcx + i * Ccx,0,Ccx,rcItem.Height());
				graphics.DrawImage((Image *)m_pImg[1],rcTemp,Ccx * index,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);
			}
			int nWidthTemp = rcCenter.Width - Ccx * nCount ;
			if( nWidthTemp > 0){
				Rect rcTemp(Lcx + nCount * Ccx,0,nWidthTemp,rcItem.Height());
				graphics.DrawImage((Image *)m_pImg[1],rcTemp,Ccx * index,0, nWidthTemp , Ccy , UnitPixel,NULL,NULL,NULL);
			}
		}

		if(m_pImg[2])
			graphics.DrawImage((Image *)m_pImg[2],rcRight,Rcx * index ,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

		if(m_pImg[0])	
			graphics.DrawImage((Image *)m_pImg[0],rcLeft,Lcx * index ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

		if(m_pImgIcon)
			graphics.DrawImage((Image *)m_pImgIcon, rc, cx * index, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
	}
	else if(lpDrawItemStruct->itemState & ODS_SELECTED)	//down
	{
		if(m_pImg[1]){
			int nCount = static_cast<int>(static_cast<float>(rcCenter.Width) / static_cast<float>(Ccx));
			for(int i = 0 ; i < nCount ; i ++){
				Rect rcTemp(Lcx + i * Ccx,0,Ccx,rcItem.Height());
				graphics.DrawImage((Image *)m_pImg[1],rcTemp,Ccx,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);
			}
			int nWidthTemp = rcCenter.Width - Ccx * nCount ;
			if( nWidthTemp > 0){
				Rect rcTemp(Lcx + nCount * Ccx,0,nWidthTemp,rcItem.Height());
				graphics.DrawImage((Image *)m_pImg[1],rcTemp,Ccx,0, nWidthTemp , Ccy , UnitPixel,NULL,NULL,NULL);
			}
		}

		if(m_pImg[2])
			graphics.DrawImage((Image *)m_pImg[2],rcRight,Rcx,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

		if(m_pImg[0])	
			graphics.DrawImage((Image *)m_pImg[0],rcLeft,Lcx ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

		if(m_pImgIcon)
			graphics.DrawImage((Image *)m_pImgIcon, rc, cx , 0, cx, cy, UnitPixel, NULL, NULL, NULL);
//		graphics.DrawImage((Image *)m_pImg, rc, cx, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
	}
/*	else if(m_bChecked || (lpDrawItemStruct->itemState & ODS_CHECKED))	//checked
	{
		if(m_pImg[1]){
			int nCount = static_cast<int>(static_cast<float>(rcCenter.Width) / static_cast<float>(Ccx));
			for(int i = 0 ; i < nCount ; i ++){
				Rect rcTemp(Lcx + i * Ccx,0,Ccx,rcItem.Height());
				graphics.DrawImage((Image *)m_pImg[1],rcTemp,Ccx * 1,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);
			}
			int nWidthTemp = rcCenter.Width - Ccx * nCount ;
			if( nWidthTemp > 0){
				Rect rcTemp(Lcx + nCount * Ccx,0,nWidthTemp,rcItem.Height());
				graphics.DrawImage((Image *)m_pImg[1],rcTemp,Ccx * 1,0, nWidthTemp , Ccy , UnitPixel,NULL,NULL,NULL);
			}
		}
			

		if(m_pImg[2])
			graphics.DrawImage((Image *)m_pImg[2],rcRight,Rcx * 1 ,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

		if(m_pImg[0])	
			graphics.DrawImage((Image *)m_pImg[0],rcLeft,Lcx * 1 ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

		if(m_pImgIcon)
			graphics.DrawImage((Image *)m_pImgIcon, rc, cx * 1, 0, cx, cy, UnitPixel, NULL, NULL, NULL);

//		graphics.DrawImage((Image *)m_pImg, rc, cx * 1, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
	}
*//*	else
	{
		if(m_iHover)	// hover
		{
			if(m_pImg[1]){
				int nCount = static_cast<int>(static_cast<float>(rcCenter.Width) / static_cast<float>(Ccx));
				for(int i = 0 ; i < nCount ; i ++){
					Rect rcTemp(Lcx + i * Ccx,0,Ccx,rcItem.Height());
					graphics.DrawImage((Image *)m_pImg[1],rcTemp,Ccx * 2,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);
				}
				int nWidthTemp = rcCenter.Width - Ccx * nCount ;
				if( nWidthTemp > 0){
					Rect rcTemp(Lcx + nCount * Ccx,0,nWidthTemp,rcItem.Height());
					graphics.DrawImage((Image *)m_pImg[1],rcTemp,Ccx * 2,0, nWidthTemp , Ccy , UnitPixel,NULL,NULL,NULL);
				}
			}		

			if(m_pImg[2])
				graphics.DrawImage((Image *)m_pImg[2],rcRight,Rcx * 2 ,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

			if(m_pImg[0])	
				graphics.DrawImage((Image *)m_pImg[0],rcLeft,Lcx * 2 ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

			if(m_pImg)
				graphics.DrawImage((Image *)m_pImg, rc, cx * 2, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
//			graphics.DrawImage((Image *)m_pImg, rc, cx * 2, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
		}
		else	//normal
		{
			if(m_pImg[1]){
				int nCount = static_cast<int>(static_cast<float>(rcCenter.Width) / static_cast<float>(Ccx));
				for(int i = 0 ; i < nCount ; i ++){
					Rect rcTemp(Lcx + i * Ccx,0,Ccx,rcItem.Height());
					graphics.DrawImage((Image *)m_pImg[1],rcTemp,0,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);
				}
				int nWidthTemp = rcCenter.Width - Ccx * nCount ;
				if( nWidthTemp > 0){
					Rect rcTemp(Lcx + nCount * Ccx,0,nWidthTemp,rcItem.Height());
					graphics.DrawImage((Image *)m_pImg[1],rcTemp,0,0, nWidthTemp , Ccy , UnitPixel,NULL,NULL,NULL);
				}
			}

			if(m_pImg[2])
				graphics.DrawImage((Image *)m_pImg[2],rcRight,0 ,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

			if(m_pImg[0])	
				graphics.DrawImage((Image *)m_pImg[0],rcLeft,0 ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

			if(m_pImg)
				graphics.DrawImage((Image *)m_pImg, rc, 0, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
//			graphics.DrawImage((Image *)m_pImg, rc, 0, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
		}	
	}

	Graphics grp(lpDrawItemStruct->hDC);
	grp.DrawImage((Image *)pimg, rc1, 0, 0, rcItem.Width(), rcItem.Height(), UnitPixel, NULL, NULL, NULL);
	// clean up
	delete pimg;

	if(m_bShowText)
	{
		// draw text
		CString sText;
		GetWindowText(sText);

		HFONT hFont, hOldFont = NULL;
		if(m_hFont == NULL)
			hFont = (HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0L);
		else
			hFont = m_hFont;
		
		if(hFont) hOldFont = (HFONT)::SelectObject(lpDrawItemStruct->hDC, hFont);
		

		CRect rc1;
		GetClientRect(&rc1);
		int offx = Lcx,offy = 0;
		rc1.left += Lcx;
		rc1.right = rc1.left + iCenterWidth;

		rcItem = rc1;
		InflateRect(&rcItem, -offx, -offy);
		
		int height;
		
		if(m_bTextLine)
			height = ::DrawText(lpDrawItemStruct->hDC, sText, -1, &rcItem, DT_CALCRECT);
		else
			height = ::DrawText(lpDrawItemStruct->hDC, sText, -1, &rcItem, DT_CALCRECT | DT_SINGLELINE);

		int width = rcItem.right - rcItem.left;

		offy = (rc1.Height() - height) / 2;

		rcItem.left = offx;
		rcItem.top =  offy;
		rcItem.right = rc1.right; // rcItem.left + width;
		rcItem.bottom = rcItem.top + height;

		if(lpDrawItemStruct->itemState & ODS_SELECTED)
			rcItem.OffsetRect(1, 1);

		::SetBkMode(lpDrawItemStruct->hDC, TRANSPARENT);

		UINT uFormat;
		if(m_bTextLine)
			uFormat = DT_LEFT | DT_WORDBREAK;
		else
			uFormat = DT_SINGLELINE | DT_LEFT;

		if(m_nAlignment == BA_CENTER)
			uFormat |= DT_CENTER;
		else if(m_nAlignment == BA_RIGHT)
			uFormat |= DT_RIGHT;

		if(m_bEnable == FALSE)
		{
			if(m_TextShadowSize != CSize(0,0))
			{
				::SetTextColor(lpDrawItemStruct->hDC, RGB(0,0,0));
				CRect rcText(&rcItem);
				rcText.OffsetRect(m_TextShadowSize);
				::DrawText(lpDrawItemStruct->hDC,sText, -1,&rcText,uFormat);
			}
			::SetTextColor(lpDrawItemStruct->hDC, m_crGText);
			::DrawText(lpDrawItemStruct->hDC, sText, -1, &rcItem, uFormat);
		}
		else
		{
			if(m_TextShadowSize != CSize(0,0))
			{
				::SetTextColor(lpDrawItemStruct->hDC, RGB(0,0,0));
				CRect rcText(&rcItem);
				rcText.OffsetRect(m_TextShadowSize);
				::DrawText(lpDrawItemStruct->hDC,sText, -1,&rcText,uFormat);
			}
			::SetTextColor(lpDrawItemStruct->hDC, m_iHover ? m_crHText : m_crText);

			::DrawText(lpDrawItemStruct->hDC, sText, -1, &rcItem, uFormat);
		}

		if(hOldFont) ::SelectObject(lpDrawItemStruct->hDC, hOldFont);
	}
*/	
}

void CSkinHeaderCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	int Lcx = 0,Lcy = 0,Rcx = 0,Rcy = 0,Ccx = 0,Ccy  = 0;
	if(m_pImg[0])
	{
		Lcx = m_pImg[0]->Width() / m_nCount;
		Lcy = m_pImg[0]->Height();
	}
	if(m_pImg[1])
	{
		Ccx = m_pImg[1]->Width() / m_nCount;
		Ccy = m_pImg[1]->Height();
	}
	if(m_pImg[2])
	{
		Rcx = m_pImg[2]->Width() / m_nCount;
		Rcy = m_pImg[2]->Height();
	}

	CRect rect;
	GetClientRect(&rect);

	CImage *pimg = new CImage(rect.Width(),rect.Height());
	if(!pimg)
		return;

	Graphics graphics(pimg);

	CRect rcLast;
	GetItemRect(GetItemCount()-1,rcLast);
	rcLast.SetRect(rcLast.right,rcLast.top,rect.right,rect.bottom);
	
	int iCenterWidth = rcLast.Width() - Lcx - Rcx;
	//get three parts rectangles		
	Rect rcCenter(rcLast.left + Lcx,rcLast.top,iCenterWidth,rcLast.Height());	//center rectangle
	Rect rcLeft(rcLast.left,rcLast.top,Lcx,rcLast.Height());	//left rectangle
	Rect rcRight(rcLast.left +Lcx + iCenterWidth,rcLast.top,Rcx,rcLast.Height());	//right rectangle
	//draw background
	if(m_pImg[1]){
		int nCount = static_cast<int>(static_cast<float>(rcCenter.Width) / static_cast<float>(Ccx));
		for(int i = 0 ; i < nCount ; i ++){
			Rect rcTemp(rcLast.left + Lcx + i * Ccx,rcLast.top,Ccx,rcLast.Height());
			graphics.DrawImage((Image *)m_pImg[1],rcTemp,0,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);
		}
		int nWidthTemp = rcCenter.Width - Ccx * nCount ;
		if( nWidthTemp > 0){
			Rect rcTemp(rcLast.left +Lcx + nCount * Ccx,rcLast.top,nWidthTemp,rcLast.Height());
			graphics.DrawImage((Image *)m_pImg[1],rcTemp,0,0, nWidthTemp , Ccy , UnitPixel,NULL,NULL,NULL);
		}
	}

	if(m_pImg[2])
		graphics.DrawImage((Image *)m_pImg[2],rcRight,0 ,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

	if(m_pImg[0])	
		graphics.DrawImage((Image *)m_pImg[0],rcLeft,0 ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

	
	for(int i = 0 ;i < GetItemCount() ; i ++){
		//get item rectangle
		CRect rcItem;
		GetItemRect(i,rcItem);
		//Get center part width	
		iCenterWidth = rcItem.Width() - Lcx - Rcx;
		
		//get three parts rectangles		
		rcCenter = Rect(rcItem.left + Lcx,rcItem.top,iCenterWidth,rcItem.Height());	//center rectangle
		rcLeft = Rect(rcItem.left,rcItem.top,Lcx,rcItem.Height());	//left rectangle
		rcRight = Rect(rcItem.left +Lcx + iCenterWidth,rcItem.top,Rcx,rcItem.Height());	//right rectangle
		
		if(m_bEnable == FALSE) //gray
		{
			int index = m_bNormalLook ? m_nNormalLook : 3;

			if(m_pImg[1]){
				int nCount = static_cast<int>(static_cast<float>(rcCenter.Width) / static_cast<float>(Ccx));
				for(int i = 0 ; i < nCount ; i ++){
					Rect rcTemp(rcItem.left + Lcx + i * Ccx,rcItem.top,Ccx,rcItem.Height());
					graphics.DrawImage((Image *)m_pImg[1],rcTemp,Ccx * index,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);
				}
				int nWidthTemp = rcCenter.Width - Ccx * nCount ;
				if( nWidthTemp > 0){
					Rect rcTemp(rcItem.left + Lcx + nCount * Ccx,rcItem.top,nWidthTemp,rcItem.Height());
					graphics.DrawImage((Image *)m_pImg[1],rcTemp,Ccx * index,0, nWidthTemp , Ccy , UnitPixel,NULL,NULL,NULL);
				}
			}

			if(m_pImg[2])
				graphics.DrawImage((Image *)m_pImg[2],rcRight,Rcx * index ,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

			if(m_pImg[0])	
				graphics.DrawImage((Image *)m_pImg[0],rcLeft,Lcx * index ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

/*			if(m_pImgIcon)
				graphics.DrawImage((Image *)m_pImgIcon, rc, cx * index, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
*/		}
		else if(m_iDown == i)	//down
		{
			if(m_pImg[1]){
				int nCount = static_cast<int>(static_cast<float>(rcCenter.Width) / static_cast<float>(Ccx));
				for(int i = 0 ; i < nCount ; i ++){
					Rect rcTemp(rcItem.left + Lcx + i * Ccx,rcItem.top,Ccx,rcItem.Height());
					graphics.DrawImage((Image *)m_pImg[1],rcTemp,Ccx,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);
				}
				int nWidthTemp = rcCenter.Width - Ccx * nCount ;
				if( nWidthTemp > 0){
					Rect rcTemp(rcItem.left + Lcx + nCount * Ccx,rcItem.top,nWidthTemp,rcItem.Height());
					graphics.DrawImage((Image *)m_pImg[1],rcTemp,Ccx,0, nWidthTemp , Ccy , UnitPixel,NULL,NULL,NULL);
				}
			}

			if(m_pImg[2])
				graphics.DrawImage((Image *)m_pImg[2],rcRight,Rcx,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

			if(m_pImg[0])	
				graphics.DrawImage((Image *)m_pImg[0],rcLeft,Lcx ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

//			if(m_pImgIcon)
//				graphics.DrawImage((Image *)m_pImgIcon, rc, cx , 0, cx, cy, UnitPixel, NULL, NULL, NULL);
		}
		else
		{
			if(m_iHover == i)	// hover
			{
				if(m_pImg[1]){
					int nCount = static_cast<int>(static_cast<float>(rcCenter.Width) / static_cast<float>(Ccx));
					for(int i = 0 ; i < nCount ; i ++){
						Rect rcTemp(rcItem.left + Lcx + i * Ccx,rcItem.top,Ccx,rcItem.Height());
						graphics.DrawImage((Image *)m_pImg[1],rcTemp,Ccx * 2,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);
					}
					int nWidthTemp = rcCenter.Width - Ccx * nCount ;
					if( nWidthTemp > 0){
						Rect rcTemp(rcItem.left + Lcx + nCount * Ccx,rcItem.top,nWidthTemp,rcItem.Height());
						graphics.DrawImage((Image *)m_pImg[1],rcTemp,Ccx * 2,0, nWidthTemp , Ccy , UnitPixel,NULL,NULL,NULL);
					}
				}		

				if(m_pImg[2])
					graphics.DrawImage((Image *)m_pImg[2],rcRight,Rcx * 2 ,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

				if(m_pImg[0])	
					graphics.DrawImage((Image *)m_pImg[0],rcLeft,Lcx * 2 ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

//				if(m_pImg)
//					graphics.DrawImage((Image *)m_pImg, rc, cx * 2, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
			}
			else	//normal
			{
				if(m_pImg[1]){
					int nCount = static_cast<int>(static_cast<float>(rcCenter.Width) / static_cast<float>(Ccx));
					for(int i = 0 ; i < nCount ; i ++){
						Rect rcTemp(rcItem.left + Lcx + i * Ccx,rcItem.top,Ccx,rcItem.Height());
						graphics.DrawImage((Image *)m_pImg[1],rcTemp,0,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);
					}
					int nWidthTemp = rcCenter.Width - Ccx * nCount ;
					if( nWidthTemp > 0){
						Rect rcTemp(rcItem.left +Lcx + nCount * Ccx,rcItem.top,nWidthTemp,rcItem.Height());
						graphics.DrawImage((Image *)m_pImg[1],rcTemp,0,0, nWidthTemp , Ccy , UnitPixel,NULL,NULL,NULL);
					}
				}

				if(m_pImg[2])
					graphics.DrawImage((Image *)m_pImg[2],rcRight,0 ,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

				if(m_pImg[0])	
					graphics.DrawImage((Image *)m_pImg[0],rcLeft,0 ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

//				if(m_pImg)
//					graphics.DrawImage((Image *)m_pImg, rc, 0, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
			}	
		}
		
		if(m_bShowText && iCenterWidth > 0)
		{
		
			//convert to wchar_t
			WCHAR wsText[MAX_PATH];
#ifdef _UNICODE
			wsprintf(wsText,m_ayStrCol[i]);
#else
			::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, m_ayStrCol[i], -1,
						wsText, sizeof(wsText)/sizeof(wsText[0]));
#endif
			//get font
			HFONT hFont, hOldFont = NULL;
			if(m_hFont == NULL)
				hFont = (HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0L);
			else
				hFont = m_hFont;
			//select font
			if(hFont) hOldFont = (HFONT)::SelectObject(dc.GetSafeHdc(), hFont);
			
			//mesaure the string layout size
			Font font(dc.GetSafeHdc(),hFont);
			RectF rcLayout(static_cast<float>(rcItem.left),static_cast<float>(rcItem.top)
				,static_cast<float>(rcItem.Width()),static_cast<float>(rcItem.Height()));
			RectF rcBoundingBox;
			graphics.MeasureString(wsText, wcslen(wsText), &font, rcLayout, &rcBoundingBox);

			//get offset length
			int offx = Lcx,offy = (rect.Height() - static_cast<int>(rcBoundingBox.Height))/2;
			
			//get the text rectangle
//			PointF point(static_cast<float>(rcItem.left + offx),static_cast<float>(rcItem.top + offy));
			RectF  rect(static_cast<float>(rcItem.left + offx),static_cast<float>(rcItem.top + offy),
				static_cast<float>(iCenterWidth + Rcx),static_cast<float>(rcItem.Height()));
			//if button down, move the text right and down 1 pixel
			if(m_iDown == i)
				rcItem.OffsetRect(1, 1);

			//set background transparent
			::SetBkMode(dc.GetSafeHdc(), TRANSPARENT);
			
			//set string format
			StringFormat format;
			format.SetAlignment(StringAlignmentNear);
			format.SetFormatFlags(StringFormatFlagsNoWrap);
			//if shadow size is not zero,draw shadow
			if(m_TextShadowSize != CSize(0,0))
			{
				//get black color
				Color color;
				color.SetFromCOLORREF(RGB(0,0,0));
				//set color to brush
				SolidBrush brush(color);
				//get the shadow pos
//				PointF pt(point);
//				pt.X -= static_cast<float>(m_TextShadowSize.cx);
//				pt.Y -= static_cast<float>(m_TextShadowSize.cy);
				RectF rc(rect.X,rect.Y,rect.Width,rect.Height);
				rc.Offset(static_cast<REAL>(m_TextShadowSize.cx),static_cast<REAL>(m_TextShadowSize.cx));
				graphics.DrawString(wsText, wcslen(wsText), &font, rc, &format, &brush);
			}

			//check item disable
			if(m_bEnable == FALSE)
			{
				//get text color
				Color color;
				color.SetFromCOLORREF(m_crGText);
				//set color to brush
				SolidBrush brush(color);
//				graphics.DrawString(wsText, wcslen(wsText), &font, point, &brush);
				graphics.DrawString(wsText, wcslen(wsText), &font, rect, &format, &brush);
			}
			else
			{
				//get text color
				Color color;
				color.SetFromCOLORREF(m_iHover ? m_crHText : m_crText);
				//set color to brush
				SolidBrush brush(color);
//				graphics.DrawString(wsText, wcslen(wsText), &font, point, &brush);
				graphics.DrawString(wsText, wcslen(wsText), &font, rect, &format, &brush);
			}
		}

	}	
	Graphics grp(dc.GetSafeHdc());
	Rect rc(0,0,rect.Width(),rect.Height());
	grp.DrawImage((Image *)pimg, rc, 0, 0, rect.Width(), rect.Height(), UnitPixel, NULL, NULL, NULL);
	// clean up
	delete pimg;
}

void CSkinHeaderCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(m_bEnable == FALSE)
		return;

	m_iDown = HitTest(point);

	if(m_bDownNotify)
	{	CWnd *pWnd = GetParent();
		if(pWnd)
			pWnd->PostMessage( GetWindowLong(m_hWnd, GWL_ID), (LPARAM)WM_LBUTTONDOWN, 0 );
	}

	CHeaderCtrl::OnLButtonDown(nFlags, point);
}

void CSkinHeaderCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(m_bEnable == FALSE)
		return;

	m_iDown = -1;
	
	if(m_bUpNotify)
	{	CWnd *pWnd = GetParent();
		if(pWnd)
			pWnd->PostMessage( GetWindowLong(m_hWnd, GWL_ID), (LPARAM)WM_LBUTTONUP, 0 );
	}

	CHeaderCtrl::OnLButtonUp(nFlags, point);
}

void CSkinHeaderCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	//	TODO: Add your message handler code here and/or call default
	if (!m_bTracking)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = 1;
		m_bTracking = _TrackMouseEvent(&tme) == TRUE ? TRUE : FALSE;
	}
	m_iHover = HitTest(point);
	Invalidate(FALSE);
	CHeaderCtrl::OnMouseMove(nFlags, point);
}

LRESULT CSkinHeaderCtrl::OnMouseHover(WPARAM wparam, LPARAM lparam) 
{
	// TODO: Add your message handler code here and/or call default
	//TRACE("ButtonEx -- MouseHover\n");

	CWnd *pWnd = GetParent();
	if(pWnd)
	{
		CRect clientRC;
		CRect parentRC;
		CPoint offset;
		pWnd->GetWindowRect(&parentRC);
		GetWindowRect(&clientRC);
		pWnd->ScreenToClient(&clientRC.TopLeft());
		pWnd->ScreenToClient(&clientRC.BottomRight());
	//	clientRC.InflateRect(5,5);
		//clientRC.OffsetRect(offset.x, offset.y);
		//ScreentoWindow(
		pWnd->InvalidateRect(&clientRC,FALSE);
	}

	if(!m_sStatus.IsEmpty())
	{
		TCHAR szOldText[1024] = _T("");
//		GetStatusText(szOldText);
		m_sOldStatus = CString(szOldText);
//		SetStatusText(m_sStatus);
	}
		Invalidate(FALSE);

	return 0;
}

LRESULT CSkinHeaderCtrl::OnMouseLeave(WPARAM wparam, LPARAM lparam)
{
	m_bTracking = FALSE;
	m_iHover = -1;
	CWnd *pWnd = GetParent();
	if(pWnd)
	{
		CRect clientRC;
		CRect parentRC;
		CPoint offset;
		pWnd->GetWindowRect(&parentRC);
		GetWindowRect(&clientRC);
		pWnd->ScreenToClient(&clientRC.TopLeft());
		pWnd->ScreenToClient(&clientRC.BottomRight());
		pWnd->InvalidateRect(&clientRC, FALSE);
	}

//	SetStatusText("");
	m_sOldStatus.Empty();
	Invalidate(FALSE);

	return 0;
}

BOOL CSkinHeaderCtrl::EnableWindow(BOOL bEnable)
{
	if( !m_hWnd )
		return FALSE;

	DWORD dwStyle = GetWindowLong(m_hWnd, GWL_STYLE) & BS_OWNERDRAW;
	if(dwStyle != BS_OWNERDRAW)
	{
		m_bEnable = bEnable;
		return CWnd::EnableWindow(bEnable);
	}
	else
	{
		BOOL bRedraw = (m_bEnable != bEnable);
		m_bEnable = bEnable;
		//if(bRedraw) 
		Invalidate(FALSE);
		//Invalidate(TRUE);
		return CWnd::EnableWindow(TRUE);
	}
}

BOOL CSkinHeaderCtrl::Enable(BOOL bEnable, BOOL bNormalLook)
{
	BOOL bRedraw = ( (m_bEnable != bEnable) || (m_bNormalLook != bNormalLook));
	m_bNormalLook = bNormalLook;
	m_bEnable = bEnable;
	if(bRedraw)
		Invalidate(FALSE);
	return CWnd::EnableWindow(TRUE);
}

void CSkinHeaderCtrl::SetTextColor(COLORREF crNormal, LPCOLORREF pCrDown, LPCOLORREF pCrGray)
{	
	m_crText = crNormal;
	m_crDText = ( pCrDown )? *pCrDown : m_crText;
	m_crGText = ( pCrGray )? *pCrGray : m_crText;
		
	m_crHText  = m_crText;
	m_crHDText = m_crDText;
}

BOOL CSkinHeaderCtrl::LoadBitmap( LPCTSTR lpszLeftImage, LPCTSTR lpszCenterImage , LPCTSTR lpszRightImage ,BOOL bSafeDel/* = TRUE*/)
{
	if(!lpszLeftImage && !lpszCenterImage && !lpszRightImage)
		return FALSE;

	LPCTSTR lpszFile[3] = {lpszLeftImage,lpszCenterImage,lpszRightImage};
	//delete old image ,if there are new image set
	if( m_bSafeDel) {
		for(int i = 0 ; i < 3 ; i ++)
			if(lpszFile[i])
				SAFE_DELPTR(m_pImg[i]);
	}

	//set safe delete flag
	m_bSafeDel = bSafeDel;
	
	//set all image 
	for(int i = 0 ; i < 3 ; i ++){
		if(lpszFile[i]){
			m_pImg[i] = new CImageArray(lpszFile[i]);
		}
	}
	return TRUE;

}

BOOL CSkinHeaderCtrl::LoadBitmap(CImageArray* pImgLeft,CImageArray* pImgCenter,CImageArray* pImgRight, BOOL bSafeDel/* = TRUE */)
{
	if(!pImgLeft && !pImgCenter && !pImgRight)
		return FALSE;

	CImageArray *pImg[3] = {pImgLeft,pImgCenter,pImgRight};
	//delete old image ,if there are new image set
	if( m_bSafeDel) {
		for(int i = 0 ; i < 3 ; i ++)
			if(pImg[i]){
				SAFE_DELPTR(m_pImg[i]);
				m_pImg[i] = pImg[i];
			}
	}

	m_bSafeDel = bSafeDel;

	return TRUE;
}

BOOL CSkinHeaderCtrl::LoadIcon(LPCTSTR lpszIcon,int iIndex)
{
	//check path null
	if(!lpszIcon)
		return FALSE;

	//check the index already has image?
	int i = 0;
	bool bExist = false;
	for(i = 0 ; i < m_ayiIcon.GetSize() ; i ++){
		if( iIndex == m_ayiIcon[i]){
			bExist = true;
			break;
		}
	}

	//if true ,replace it.
	//else add to array
	if(bExist){
		SAFE_DELPTR(m_ayImgIcon[i]);
		CImageArray *pImg = new CImageArray(lpszIcon);
		if(pImg){
			m_ayImgIcon[i] = pImg;
			return TRUE;
		}
	}
	else{
		CImageArray *pImg = new CImageArray(lpszIcon);
		if(pImg){
			m_ayImgIcon.Add(pImg);
			m_ayiIcon.Add(i);
			return TRUE;
		}
	}

	return FALSE;
}	

BOOL CSkinHeaderCtrl::LoadIcon(CImageArray *pImgIcon,int iIndex)
{
	//check image null
	if(!pImgIcon)
		return FALSE;

	//check the index already has image?
	int i = 0;
	bool bExist = false;
	for(i = 0 ; i < m_ayiIcon.GetSize() ; i ++){
		if( iIndex == m_ayiIcon[i]){
			bExist = true;
			break;
		}
	}
	//if true ,replace it.
	//else add to array
	if(bExist){
		SAFE_DELPTR(m_ayImgIcon[i]);
		m_ayImgIcon[i] = pImgIcon;
	}
	else{
		m_ayImgIcon.Add(pImgIcon);
		m_ayiIcon.Add(i);
	}

	return TRUE;
}

void CSkinHeaderCtrl::SetTextLine(TextStyle ts)
{
	if(ts == TEXT_SINGLELINE)
		m_bTextLine = FALSE;
	else if(ts == TEXT_MUTILINE)
		m_bTextLine = TRUE;
}

int CSkinHeaderCtrl::HitTest(CPoint &point)
{
	CPoint pt(point);
//	ScreenToClient(&pt);
	//check point in which item 
	for(int i = 0 ; i < GetItemCount() ; i ++){
		CRect rcItem;
		GetItemRect(i,rcItem);
		if(rcItem.PtInRect(pt))
			return i;
	}
	return -1;
}

BOOL CSkinHeaderCtrl::AddColText(LPCTSTR lpszText, int iIndex/* = -1*/)
{
	if(iIndex == -1){
		m_ayStrCol.Add(lpszText);
	}
	else if(iIndex < m_ayStrCol.GetSize()){
		m_ayStrCol.SetAt(iIndex,lpszText);
	}
	else{
		return FALSE;
	}
	return TRUE;
}
