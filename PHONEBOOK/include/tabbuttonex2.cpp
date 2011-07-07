// TabButtonEx2.cpp: implementation of the CTabButtonEx2 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TabButtonEx2.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTabButtonEx2::CTabButtonEx2()
{

}

CTabButtonEx2::~CTabButtonEx2()
{
}

BEGIN_MESSAGE_MAP(CTabButtonEx2, CTabButtonEx)
	//{{AFX_MSG_MAP(CTabButtonEx2)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CTabButtonEx2::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your code to draw the specified item
	//Set all part width and height
	int cx =0,cy =0,Lcx = 0,Lcy = 0,Rcx = 0,Rcy = 0,Ccx = 0,Ccy  = 0;
	if(m_pImg)
	{
		cx = m_pImg->Width() / m_nCount;
		cy = m_pImg->Height();
	}
	if(m_pImgLBg)
	{
		Lcx = m_pImgLBg->Width() / m_nCount;
		Lcy = m_pImgLBg->Height();
	}
	if(m_pImgCBg)
	{
		Ccx = m_pImgCBg->Width() / m_nCount;
		Ccy = m_pImgCBg->Height();
	}
	if(m_pImgRBg)
	{
		Rcx = m_pImgRBg->Width() / m_nCount;
		Rcy = m_pImgRBg->Height();
	}
	
	//Get the button size
	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);

	BOOL bEnabled = m_bEnable;
	if( !bEnabled )
		int a=0;

	CImage *pimg = new CImage(rect.Width(), rect.Height(),PT_32BITSARGB);
	if(pimg == NULL)
		return;

	Graphics graphics(pimg);
	Rect rc(m_lWidth, 0, cx, cy);	//Get the m_Img draw rectangle
	Rect rc1(rect.left,rect.top,rect.Width(),rect.Height());	//get the button draw rectangle
	if( !m_bTransparentBk )
	{	Color color;
		color.SetFromCOLORREF(m_crBk);
		pimg->FillColor(color);
	}

	Rect rcCenter(m_lWidth,0,m_cWidth+m_rWidth,rect.Height());	//center rectangle
	Rect rcLeft(0,0,m_lWidth,rect.Height());	//left rectangle
	Rect rcRight(m_lWidth+m_cWidth,0,m_rWidth,rect.Height());	//right rectangle

	if(bEnabled == FALSE) //gray
	{
		if(m_bChecked)		//sel && gray ,index = 3
		{
			int index = m_bNormalLook ? m_nNormalLook : 3;

			if(m_pImgCBg)
				graphics.DrawImage((Image *)m_pImgCBg,rcCenter,Ccx * index,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);

			if(m_pImgRBg)
				graphics.DrawImage((Image *)m_pImgRBg,rcRight,Rcx * index ,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

			if(m_pImgLBg)	
				graphics.DrawImage((Image *)m_pImgLBg,rcLeft,Lcx * index ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

			if(m_pImg)
				graphics.DrawImage((Image *)m_pImg, rc, cx * index, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
		}
		else	//nosel && gray ,index = 4
		{
			int index = m_bNormalLook ? m_nNormalLook : 4;

			if(m_pImgCBg)
				graphics.DrawImage((Image *)m_pImgCBg,rcCenter,Ccx * index,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);

			if(m_pImgRBg)
				graphics.DrawImage((Image *)m_pImgRBg,rcRight,Rcx * index ,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

			if(m_pImgLBg)	
				graphics.DrawImage((Image *)m_pImgLBg,rcLeft,Lcx * index ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

			if(m_pImg)
				graphics.DrawImage((Image *)m_pImg, rc, cx * index, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
		}
	}
	else
	{
		if(m_bHover && !m_bChecked)	// hover
		{
			if(m_pImgCBg)
				graphics.DrawImage((Image *)m_pImgCBg,rcCenter,Ccx * 2,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);

			if(m_pImgRBg)
				graphics.DrawImage((Image *)m_pImgRBg,rcRight,Rcx * 2 ,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

			if(m_pImgLBg)	
				graphics.DrawImage((Image *)m_pImgLBg,rcLeft,Lcx * 2 ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

			if(m_pImg)
				graphics.DrawImage((Image *)m_pImg, rc, cx * 2, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
		}
		else	//normal
		{
			if(m_bChecked)	//sel && normal
			{
				if(m_pImgCBg)
					graphics.DrawImage((Image *)m_pImgCBg,rcCenter,0,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);

				if(m_pImgRBg)
					graphics.DrawImage((Image *)m_pImgRBg,rcRight,0 ,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

				if(m_pImgLBg)	
					graphics.DrawImage((Image *)m_pImgLBg,rcLeft,0 ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

				if(m_pImg)
					graphics.DrawImage((Image *)m_pImg, rc, 0, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
			}
			else	//nosel &&  normal
			{
				if(m_pImgCBg)
					graphics.DrawImage((Image *)m_pImgCBg,rcCenter,Ccx,0, Ccx , Ccy , UnitPixel,NULL,NULL,NULL);

				if(m_pImgRBg)
					graphics.DrawImage((Image *)m_pImgRBg,rcRight,Rcx ,0, Rcx , Rcy ,UnitPixel,NULL,NULL,NULL);

				if(m_pImgLBg)	
					graphics.DrawImage((Image *)m_pImgLBg,rcLeft,Lcx ,0, Lcx, Lcy ,UnitPixel,NULL,NULL,NULL);

				if(m_pImg)
					graphics.DrawImage((Image *)m_pImg, rc, cx, 0, cx, cy, UnitPixel, NULL, NULL, NULL);
			}
		}	
	}

	Graphics grp(lpDrawItemStruct->hDC);
	grp.DrawImage((Image *)pimg, rc1, 0, 0, rect.Width(), rect.Height(), UnitPixel, NULL, NULL, NULL);
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
		int offx = m_lWidth,offy = 0;
		rc1.left += m_lWidth;
		rc1.right = rc1.left + m_cWidth;

		rect = rc1;
		InflateRect(&rect, -offx, -offy);
		
		int height;
		
		if(m_bTextLine)
			height = ::DrawText(lpDrawItemStruct->hDC, sText, -1, &rect, DT_CALCRECT);
		else
			height = ::DrawText(lpDrawItemStruct->hDC, sText, -1, &rect, DT_CALCRECT | DT_SINGLELINE);

		int width = rect.right - rect.left;

		offy = (rc1.Height() - height) / 2;

		rect.left = offx;
		rect.top =  offy;
		rect.right = rc1.right; // rect.left + width;
		rect.bottom = rect.top + height;

		if(lpDrawItemStruct->itemState & ODS_SELECTED)
			rect.OffsetRect(1, 1);

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

		if(bEnabled == FALSE)
		{
		
			if(m_TextShadowSize != CSize(0,0))
			{
				::SetTextColor(lpDrawItemStruct->hDC, RGB(0,0,0));
				CRect rcText(&rect);
				rcText.OffsetRect(m_TextShadowSize);
				::DrawText(lpDrawItemStruct->hDC,sText, -1,&rcText,uFormat);
			}
			if(m_bChecked)		//sel && gray ,index = 3
				::SetTextColor(lpDrawItemStruct->hDC, m_crSGText);
			else
				::SetTextColor(lpDrawItemStruct->hDC, m_crGText);
			::DrawText(lpDrawItemStruct->hDC, sText, -1, &rect, uFormat);
		}
		else
		{
			if(m_TextShadowSize != CSize(0,0))
			{
				::SetTextColor(lpDrawItemStruct->hDC, RGB(0,0,0));
				CRect rcText(&rect);
				rcText.OffsetRect(m_TextShadowSize);
				::DrawText(lpDrawItemStruct->hDC,sText, -1,&rcText,uFormat);
			}
			if(m_bChecked || (lpDrawItemStruct->itemState & ODS_CHECKED))
				::SetTextColor(lpDrawItemStruct->hDC, m_bHover ? m_crHDText : m_crDText);
			else
				::SetTextColor(lpDrawItemStruct->hDC, m_bHover ? m_crHText : m_crText);

			::DrawText(lpDrawItemStruct->hDC, sText, -1, &rect, uFormat);
		}

		if(hOldFont) ::SelectObject(lpDrawItemStruct->hDC, hOldFont);
	}
}

void CTabButtonEx2::OnDestroy() 
{
	CTabButtonEx::OnDestroy();
	
	if(m_bCSafeDel && m_pImgCBg)
		SAFE_DELPTR(m_pImgCBg);
	if(m_bRSafeDel && m_pImgRBg)
		SAFE_DELPTR(m_pImgRBg);
	if(m_bLSafeDel && m_pImgLBg)
		SAFE_DELPTR(m_pImgLBg);
}
