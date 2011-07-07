#include "stdafx.h"
#include "CPrinter.h"
//	This file contains the printer primitives. This is a very low level implemtation
//	and should never be called directly. A member of this class is included in the 
//	CPage class and is used to print the page. Information in the PRTTYPE struct is
//	modified here and is reflected in the structure.
CPrinter::CPrinter()
{
	RotationAngle=0;
}

CPrinter::~CPrinter()
{
}

void CPrinter::FillRect(PRTTYPE *ps)
{
	    HBRUSH hOldBrush, hBrush;
	    RECT  lpRect;

	    hBrush = (HBRUSH)::GetStockObject((ps->uFillFlags & FILL_GRAY)   ? GRAY_BRUSH   :
				    (ps->uFillFlags & FILL_LTGRAY) ? LTGRAY_BRUSH :
				    (ps->uFillFlags & FILL_DKGRAY) ? DKGRAY_BRUSH :
				    (ps->uFillFlags & FILL_BLACK)  ? BLACK_BRUSH  : HOLLOW_BRUSH);

	    hOldBrush = (HBRUSH)::SelectObject(ps->pDC->GetSafeHdc(), hBrush);


	    lpRect.left= ps->rc.left+ps->MarginOffset;
	    lpRect.top = ps->rc.top;
	    lpRect.right= ps->rc.right+ps->MarginOffset;
	    lpRect.bottom= ps->rc.bottom;
		
		::FillRect(ps->pDC->GetSafeHdc(),&lpRect,hBrush);
	    ::DeleteObject(SelectObject(ps->pDC->GetSafeHdc(), hOldBrush));
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//	Desc:
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPrinter::DrawRect(PRTTYPE *ps,int LineSize)
{
	    HPEN   hOldPen, hPen;
	    HBRUSH hOldBrush, hBrush;
	    int   nLineWidth;
	    POINT  lpPT[4];

	    hBrush = (HBRUSH)::GetStockObject((ps->uFillFlags & FILL_GRAY)   ? GRAY_BRUSH   :
				    (ps->uFillFlags & FILL_LTGRAY) ? LTGRAY_BRUSH :
				    (ps->uFillFlags & FILL_DKGRAY) ? DKGRAY_BRUSH :
				    (ps->uFillFlags & FILL_BLACK)  ? BLACK_BRUSH  : HOLLOW_BRUSH);

	    hOldBrush = (HBRUSH)::SelectObject(ps->pDC->GetSafeHdc(), hBrush);
        
        nLineWidth = LineSize;
        
	    hPen = ::CreatePen((ps->uPenFlags & PEN_DASH) ? PS_DASH  :
			     (ps->uPenFlags & PEN_DOT)	? PS_DOT   : PS_SOLID,
			     nLineWidth, ps->pDC->GetTextColor());

	    hOldPen = (HPEN)::SelectObject(ps->pDC->GetSafeHdc(), hPen);

	    lpPT[0].x = ps->rc.left+ps->MarginOffset;
	    lpPT[0].y = ps->rc.top;
	    lpPT[1].x = ps->rc.right+ps->MarginOffset;
	    lpPT[1].y = ps->rc.top;
	    lpPT[2].x = ps->rc.right+ps->MarginOffset;
	    lpPT[2].y = ps->rc.bottom;
	    lpPT[3].x = ps->rc.left+ps->MarginOffset;
	    lpPT[3].y = ps->rc.bottom;

	    Polygon(ps->pDC->GetSafeHdc(), lpPT, sizeof(lpPT) / sizeof(POINT));

	    ::DeleteObject(SelectObject(ps->pDC->GetSafeHdc(), hOldPen));
	    ::DeleteObject(SelectObject(ps->pDC->GetSafeHdc(), hOldBrush));
}

////////////////////////////////////////////////////////////////////////////
//	Desc:
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
void CPrinter::DrawLine(PRTTYPE *ps,int LineSize)                    
{
	    HPEN  hOldPen, hPen;
	    POINT pt[2];
	    int   nPenStyle;
	    int   nLineWidth;

	    nPenStyle = (ps->uPenFlags & PEN_DASH)	 ? PS_DASH	 :
			(ps->uPenFlags & PEN_DOT)	 ? PS_DOT	 :
			(ps->uPenFlags & PEN_DASHDOT)	 ? PS_DASHDOT	 :
			(ps->uPenFlags & PEN_DASHDOTDOT) ? PS_DASHDOTDOT :
			 PS_SOLID;


	    nLineWidth = LineSize;

	    //hPen = ::CreatePen(nPenStyle, nLineWidth, RGB(0,0,0));
		hPen = ::CreatePen(nPenStyle, nLineWidth, ps->pDC->GetTextColor( ));
	    
        pt[0].x = ps->rc.left+ps->MarginOffset;
        pt[0].y = ps->rc.top;
        pt[1].x = ps->rc.right+ps->MarginOffset;
        pt[1].y = ps->rc.bottom;
        
	    hOldPen = (HPEN)::SelectObject(ps->pDC->GetSafeHdc(), hPen);                   
	    
        ::Polyline(ps->pDC->GetSafeHdc(), pt, sizeof(pt) / sizeof(POINT));
        
	    ::DeleteObject(SelectObject(ps->pDC->GetSafeHdc(), hOldPen));
}

////////////////////////////////////////////////////////////////////////////
//	Desc:
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
int CPrinter::PrintText(PRTTYPE *ps,double LineSpacing)
{         
	    LOGFONT	      lf;
	    HFONT	      hOldFont, hFont;
	    COLORREF 	  OldColor;
	    int		      saveContext,OldBkMode;
	    RECT	      rc,LastPos;
	    HBRUSH	      hOldBrush, hBrush;
	    UINT	      fuFormat = DT_NOPREFIX;
	    
        int uiLineSpacing;
        
        if(_tcslen(ps->Text)==0)
        	return	2 * ps->PointSize;
        	
	      //initialize the rect structure
	    rc.left   = ps->rc.left+ps->MarginOffset;
	    rc.top    = ps->rc.top;
	    rc.right  = ps->rc.right+ps->MarginOffset;
		if(rc.right > (signed)ps->n_maxWidth-ps->MarginOffset)
			rc.right=ps->n_maxWidth-ps->MarginOffset;
	    rc.bottom = ps->rc.bottom;

	      //convert logical coordinates to device coordinates
	    ps->pDC->LPtoDP((LPPOINT)&rc, 2);

	      //save the dc and change map mode to MM_TEXT.  Much
	    saveContext = ps->pDC->SaveDC();
	    ps->pDC->SetMapMode(MM_TEXT);

	    lf.lfHeight = -MulDiv(ps->PointSize,
				  ps->pDC->GetDeviceCaps(LOGPIXELSY), 72);
	    lf.lfWidth = 0;
	    lf.lfEscapement = RotationAngle;
	    lf.lfOrientation = 0;
	    lf.lfWeight = (ps->uTextFlags & TEXT_BOLD) ? FW_BOLD : FW_NORMAL;
	    lf.lfItalic = (ps->uTextFlags & TEXT_ITALIC) ? 1 : 0;
	    lf.lfUnderline = (ps->uTextFlags & TEXT_UNDERLINED) ? 1 : 0;
	    lf.lfStrikeOut = (ps->uTextFlags & TEXT_STRIKEOUT) ? 1 : 0;
	    lf.lfCharSet = DEFAULT_CHARSET;
	    lf.lfOutPrecision = OUT_STROKE_PRECIS;
		lf.lfClipPrecision = CLIP_STROKE_PRECIS;
		
	    lf.lfQuality = PROOF_QUALITY;
	    lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	    lstrcpy((LPTSTR)lf.lfFaceName, (LPCTSTR)ps->FontName);
	    hFont = ::CreateFontIndirect(&lf);

	    hOldFont = (HFONT)::SelectObject(ps->pDC->GetSafeHdc(), hFont);


	      //set the alignment flags in the format bitfield
	    fuFormat |= (ps->uTextFlags & TEXT_CENTER) ? DT_CENTER :
					(ps->uTextFlags & TEXT_RIGHT)  ? DT_RIGHT  :DT_LEFT;

		if(ps->uTextFlags & TEXT_VCENTER) 
			fuFormat|=DT_VCENTER;
			
	      //expand tabs if indicated
	    if (ps->uTextFlags & TEXT_EXPANDTABS)
		fuFormat |=  DT_EXPANDTABS;

	      //set text clipping
	    if (ps->uTextFlags & TEXT_NOCLIP)
		fuFormat |=  DT_NOCLIP;

	      //if this is a single line then set alignment to bottom
	    if (ps->uTextFlags & TEXT_SINGLELINE)
			fuFormat |=  DT_SINGLELINE;
	    else
			fuFormat |=  DT_WORDBREAK;

	      //if the extents of rect haven't been specified then
	      //calculate them
	    if (ps->rc.right == 0 && ps->rc.bottom == 0)
			ps->pDC->DrawText(ps->Text, -1,&rc, fuFormat | DT_CALCRECT);
				 
				 
		LastPos=rc;


		if(    ps->uTextFlags & TEXT_RECT &&
	  		((ps->uFillFlags & 	FILL_BLACK) || 
			(ps->uFillFlags & 	FILL_LTGRAY) ||
            (ps->uFillFlags & 	FILL_DKGRAY)))
				OldBkMode=ps->pDC->SetBkMode(TRANSPARENT);
	
		if(    ps->uTextFlags & TEXT_RECT &&
	  		((ps->uFillFlags & 	FILL_BLACK) ||
            (ps->uFillFlags & 	FILL_DKGRAY)))
	   {
			OldColor= ps->pDC->SetTextColor(COLOR_WHITE);
	   }

	    if (ps->uTextFlags & TEXT_RECT)  
	    {

			hBrush = (HBRUSH)::GetStockObject((ps->uFillFlags & FILL_GRAY)   ? GRAY_BRUSH   :
				      (ps->uFillFlags & FILL_LTGRAY) ? LTGRAY_BRUSH :
				      (ps->uFillFlags & FILL_DKGRAY) ? DKGRAY_BRUSH :
				      (ps->uFillFlags & FILL_BLACK)  ? BLACK_BRUSH  : HOLLOW_BRUSH);
			hOldBrush = (HBRUSH)::SelectObject(ps->pDC->GetSafeHdc(), hBrush);

			ps->pDC->Rectangle(rc.left, rc.top, rc.right, rc.bottom);

	    }

		//now draw the text
	    ps->pDC->DrawText(ps->Text, -1,&rc, fuFormat);


		if(    ps->uTextFlags & TEXT_RECT &&
	  		((ps->uFillFlags & 	FILL_BLACK) || 
			(ps->uFillFlags & 	FILL_LTGRAY) ||
            (ps->uFillFlags & 	FILL_DKGRAY)))
				OldBkMode=ps->pDC->SetBkMode(OldBkMode);
        
	  if(    ps->uTextFlags & TEXT_RECT &&
	  		((ps->uFillFlags & 	FILL_BLACK) ||
            (ps->uFillFlags & 	FILL_DKGRAY)))
	   {
			ps->pDC->SetTextColor(OldColor);
	   }
        
	   	ps->pDC->DrawText(ps->Text, -1,&LastPos, fuFormat | DT_CALCRECT);
    	uiLineSpacing = (int)(ps->PointSize*LineSpacing);
	    
	    if (ps->uTextFlags & TEXT_RECT)
	    	::DeleteObject(::SelectObject(ps->pDC->GetSafeHdc(), hOldBrush));
	    ::DeleteObject(::SelectObject(ps->pDC->GetSafeHdc(), hOldFont));
	    if (saveContext)
			ps->pDC->RestoreDC(saveContext);

	    ps->pDC->DPtoLP((LPPOINT)&LastPos, 2);
		ps->m_MinDisplacement=LastPos.bottom-LastPos.top;	    
		
		ps->pDC->GetTextMetrics( ps->tm );
	    ps->m_NextCharPos=LastPos.right;
		ps->LastPrintArea=LastPos;
		RotationAngle=0;
		return uiLineSpacing;
}         






////////////////////////////////////////////////////////////////////////////
//	Desc:
//	params:
//	Returns:
///////////////////////////////////////////////////////////////////////////
int CPrinter::GetPrintInfo(PRTTYPE *ps,double LineSpacing)
{         
	    LOGFONT	      lf;
	    HFONT	      hOldFont, hFont;
	    int		      saveContext;
	    RECT	      rc,LastPos;
	    UINT	      fuFormat = DT_NOPREFIX;
	    
        int uiLineSpacing;
        
        	
	      //initialize the rect structure
	    rc.left   = ps->rc.left+ps->MarginOffset;
	    rc.top    = ps->rc.top;
	    rc.right  = ps->rc.right+ps->MarginOffset;
	    rc.bottom = ps->rc.bottom;

	      //convert logical coordinates to device coordinates
	    ps->pDC->LPtoDP((LPPOINT)&rc, 2);

	      //save the dc and change map mode to MM_TEXT.  Much
	    saveContext = ps->pDC->SaveDC();

	    ps->pDC->SetMapMode(MM_TEXT);

	    lf.lfHeight = -MulDiv(ps->PointSize,
				  ps->pDC->GetDeviceCaps(LOGPIXELSY), 72);
	    lf.lfWidth = 0;
	    lf.lfEscapement = 0;
	    lf.lfOrientation = 0;
	    lf.lfWeight = (ps->uTextFlags & TEXT_BOLD) ? FW_BOLD : FW_NORMAL;
	    lf.lfItalic = (ps->uTextFlags & TEXT_ITALIC) ? 1 : 0;
	    lf.lfUnderline = (ps->uTextFlags & TEXT_UNDERLINED) ? 1 : 0;
	    lf.lfStrikeOut = (ps->uTextFlags & TEXT_STRIKEOUT) ? 1 : 0;
	    lf.lfCharSet = DEFAULT_CHARSET;
	    lf.lfOutPrecision = OUT_STROKE_PRECIS;
	    lf.lfClipPrecision = CLIP_STROKE_PRECIS;
	    lf.lfQuality = PROOF_QUALITY;
	    lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	    lstrcpy((LPTSTR)lf.lfFaceName, (LPCTSTR)ps->FontName);

	    hFont = ::CreateFontIndirect(&lf);

	    hOldFont = (HFONT)::SelectObject(ps->pDC->GetSafeHdc(), hFont);


	      //set the alignment flags in the format bitfield
	    fuFormat |= (ps->uTextFlags & TEXT_CENTER) ? DT_CENTER :
					(ps->uTextFlags & TEXT_RIGHT)  ? DT_RIGHT  :DT_LEFT;

		if(ps->uTextFlags & TEXT_VCENTER) 
			fuFormat|=DT_VCENTER;
			
	      //expand tabs if indicated
	    if (ps->uTextFlags & TEXT_EXPANDTABS)
		fuFormat |=  DT_EXPANDTABS;

	      //set text clipping
	    if (ps->uTextFlags & TEXT_NOCLIP)
		fuFormat |=  DT_NOCLIP;

	      //if this is a single line then set alignment to bottom
	    if (ps->uTextFlags & TEXT_SINGLELINE)
		fuFormat |=  DT_SINGLELINE;
	    else
		fuFormat |=  DT_WORDBREAK;

		LastPos=rc;

		ps->pDC->DrawText(ps->Text, -1,
		 &LastPos, fuFormat | DT_CALCRECT);
        
	    uiLineSpacing = (int)(ps->PointSize*LineSpacing);

	    ::DeleteObject(::SelectObject(ps->pDC->GetSafeHdc(), hOldFont));
	    if (saveContext)
			ps->pDC->RestoreDC(saveContext);

	    ps->pDC->DPtoLP((LPPOINT)&LastPos, 2);
		ps->m_MinDisplacement=LastPos.bottom-LastPos.top;	    
	    ps->m_NextCharPos=LastPos.right;
		ps->LastPrintArea=LastPos;
		return uiLineSpacing;
}         


