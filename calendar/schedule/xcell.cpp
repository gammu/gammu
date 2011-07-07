#include "StdAfx.h"
#include "xcell.h"
#include "..\..\AnWLib\include\anwImage\ximage.h"

CBitmap XCell::m_AlarmBmp;
CBitmap XCell::m_RepeatBmp;
CBitmap XCell::m_AlarmGrayBmp;
CBitmap XCell::m_RepeatGrayBmp;
CBitmap XCell::m_MoreBmp;
CBitmap XCell::m_MoreGrayBmp;

//BOOL XCell::m_bTextLine=FALSE;


XCell::XCell()
{
	textFont = NULL;
	Init();

}

void XCell::Init() {
	rowSpan = 0;
	colSpan = 0;

	backColor = RGB (255,255,255);
	textColor = RGB (0,0,0);
	borderColor = RGB (0xDC,0xDC,0xDC);
	textlineColor = borderColor;

	borderStyle = 0;

	LOGFONT lf;
	SystemParametersInfo(SPI_GETICONTITLELOGFONT,sizeof(LOGFONT),&lf,0);
	//	font->GetLogFont(&lf);
	//	AfxMessageBox(lf.lfFaceName);

	textFontSize = 14;
	textFontFace = "Tahoma";//lf.lfFaceName;//"Arial";
	//	if (!textFont) {
	//		delete textFont;
	//		textFont = NULL;
	//		textFont = new CFont();
	//		textFont->CreateFontIndirect(&lf);
	//	}

	backMode = TRANSPARENT;
	format = 0;
	overlap = false;

	leftMargin = 2;
	rightMargin = 2;

	for (int i=0;i<48;i++) {
		text[i] = _T("");
		m_bAlarm[i]=FALSE;
		m_bRepeat[i]=FALSE;
	}
	m_bTextLine=FALSE;
	m_bDrawUpLine = TRUE;
	m_bDrawDnLine = TRUE;
	m_bDrawLeftLine = TRUE;
	m_bDrawRightLine = TRUE;

}

XCell::~XCell()
{
	//DLL CONTRUCT
	m_AlarmBmp.DeleteObject();
	m_RepeatBmp.DeleteObject();
	m_AlarmGrayBmp.DeleteObject();
	m_RepeatGrayBmp.DeleteObject();
	m_MoreBmp.DeleteObject();
	m_MoreGrayBmp.DeleteObject();
}

XCell* XCell::operator = (XCell& cell)
{
	rowSpan = cell.rowSpan;
	colSpan = cell.colSpan;

	backColor = cell.backColor;
	textColor = cell.textColor;
	backMode = cell.backMode;

	borderColor = cell.borderColor;
	borderStyle = cell.borderStyle;

	if (textFont)
	{
		delete textFont;
		textFont = NULL;
	}

	if (cell.textFont)
	{
		LOGFONT logFont;
		cell.textFont->GetLogFont (&logFont);

		textFont = new CFont;
		logFont.lfCharSet = DEFAULT_CHARSET;
		textFont->CreateFontIndirect (&logFont);
	}
	
	format = cell.format;

	leftMargin = cell.leftMargin;
	rightMargin = cell.rightMargin;

	return this;
}

int XCell::SetSpan(int rows, int cols)
{
	rowSpan = rows;
	colSpan = cols;

	return 0;
}

int XCell::SetText(CString str)
{
	text[0] = (LPCTSTR)str;

	SetTextCount(1);

	return 0;
}

CString XCell::GetText()
{
	return text[0];
}

int XCell::SetText(int idx,CString str)
{
	if ((idx<0) || (idx>47)) {
		return -1;
	}
	text[idx] =str;

	return 0;
}

int XCell::SetText(CString strText[48],int strCount)
{
	if ((strCount<0) || (strCount>48)) {
		return -1;
	}
	textCount = strCount;
	for (int i=0;i<strCount;i++) {
		text[i] = strText[i];
	}

	return 0;
}

CString XCell::GetText(int idx)
{
	if ((idx<0) || (idx>47)) {
		return _T("");
	}
	return text[idx];
}

int XCell::SetTextColor(COLORREF color)
{
	textColor = color;

	return 0;
}
int XCell::SetTextLineColor(COLORREF color)
{
	textlineColor = color;

	return 0;
}

COLORREF XCell::GetTextColor()
{
	return textColor;
}

int XCell::SetAlignment (int align)
{
	if ( (align & ~ALIGN_MASK) != 0) return -1;

	format = (format & ~ALIGN_MASK) | align;

	return 0;
}

int XCell::GetAlignment ()
{
	return format & ALIGN_MASK;
}

int XCell::SetFormat (int format)
{
	this->format = format;

	return 0;
}

int XCell::GetFormat ()
{
	return format;
}

int XCell::SetSingleLine (bool enable)
{
	if (enable)
		format |= DT_SINGLELINE;
	else
		format &= ~DT_SINGLELINE;

	return 0;
}

bool XCell::GetSingleLine ()
{
	return (format & DT_SINGLELINE) != 0;
}

int XCell::SetWordbreak (bool enable)
{
	if (enable)
		format |= DT_WORDBREAK;
	else
		format &= ~DT_WORDBREAK;

	return 0;
}

bool XCell::GetWordbreak ()
{
	return (format & DT_WORDBREAK) != 0;
}

int XCell::SetEllipsis (bool enable)
{
	if (enable)
		format |= DT_END_ELLIPSIS;
	else
		format &= ~DT_END_ELLIPSIS;

	return 0;
}

bool XCell::GetEllipsis ()
{
	return (format & DT_END_ELLIPSIS) != 0;
}

int XCell::SetLeftMargin (int pixels)
{
	leftMargin = pixels;

	return 0;
}

int XCell::GetLeftMargin ()
{
	return leftMargin;
}

int XCell::SetRightMargin (int pixels)
{
	rightMargin = pixels;

	return 0;
}

int XCell::GetRightMargin ()
{
	return rightMargin;
}



int XCell::SetLabel (CString str)
{
	label = str;

	return 0;
}

CString XCell::GetLabel ()
{
	return label;
}

int XCell::SetOverlap (bool enable)
{
	overlap = enable;

	return 0;
}

bool XCell::GetOverlap ()
{
	return overlap;
}

int XCell::SetTextFont(CFont* font)
{
	LOGFONT logfont;

	if (!font->GetLogFont(&logfont))
		return -1;

	if (textFont)
	{
		delete textFont;
		textFont = NULL;
	}
	
	textFont = new CFont;
	textFont->CreateFontIndirect(&logfont);

	return 0;
}

CFont* XCell::GetTextFont()
{
	return textFont;
}

int XCell::SetTextFontSize(int size)
{
	textFontSize = size;

	return 0;
}

int XCell::GetTextFontSize()
{
	return textFontSize;
}



int XCell::SetLabelColor(COLORREF color)
{
	labelColor = color;

	return 0;
}

COLORREF XCell::GetLabelColor()
{
	return labelColor;
}

int XCell::SetLabelFont(CFont* font)
{
	LOGFONT logfont;

	if (!font->GetLogFont(&logfont))
		return -1;

	if (labelFont)
	{
		labelFont->DeleteObject();
		labelFont = NULL;
	}

	labelFont = new CFont;
	labelFont->CreateFontIndirect(&logfont);

	return 0;
}

CFont* XCell::GetLabelFont()
{
	return labelFont;
}

int XCell::SetLabelFontSize(int size)
{
	labelFontSize = size;

	return 0;
}

int XCell::GetLabelFontSize()
{
	return labelFontSize;
}

int XCell::SetBackMode(int mode)
{
	backMode = mode;

	return 0;
}

int XCell::GetBackMode()
{
	return backMode;
}

int XCell::SetBackColor(COLORREF color)
{
	backColor = color;

	return 0;
}

COLORREF XCell::GetBackColor()
{
	return backColor;
}

int XCell::SetBorderSyle(int syle)
{
	borderStyle = syle;

	return 0;
}

int XCell::GetBorderSyle()
{
	return borderStyle;
}



int XCell::DrawText(CDC* pDC, RECT rect)
{
	//	static const WCHAR DefaultGuiFontName_950[] = {'M','i','n','g','L','i','u','\0'};

	COLORREF oldTextColor = pDC->SetTextColor (textColor);
	int oldBkMode = pDC->SetBkMode (backMode);
	CFont* oldFont;
	CFont tempFont;
	if (textFont)
		oldFont = pDC->SelectObject (textFont);
	else
	{
		tempFont.CreateFont (textFontSize,0,0, 0, FW_NORMAL, FALSE, FALSE, 0,
			DEFAULT_CHARSET,OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS,  textFontFace);
		oldFont = pDC->SelectObject (&tempFont);
	}

	if (leftMargin)
		rect.left += leftMargin;
	if (rightMargin)
		rect.right -= rightMargin;

	RECT rt = {rect.left,rect.top,rect.right,rect.bottom};
	CalcTextRect(pDC,&rt);
	int height = rt.bottom - rt.top;

	BITMAP alarmBitmap;
	BITMAP repeatBitmap;
	m_AlarmBmp.GetBitmap(&alarmBitmap);
	m_RepeatBmp.GetBitmap(&repeatBitmap);
	CPen linePen (PS_SOLID, 1, textlineColor);
	int width=0;
	if (textCount == 1) {
		int interval = ((rect.bottom - rect.top) - height) / 2;
		if (interval > height/2 ) {
			interval = 0;
		}
		rect.top += interval;
		rect.bottom = rect.top + height;
		RECT r2={rect.left,rect.top,rect.right,rect.bottom};
		if (m_bAlarm[0]) {
			r2.right = r2.left + alarmBitmap.bmWidth;
			if (backColor == RGB(223,227,239)) {
				DrawBitmap(pDC,r2,m_AlarmGrayBmp);
			} else {
				DrawBitmap(pDC,r2,m_AlarmBmp);
			}
			width = r2.right-r2.left;
			r2.left += width;
		}
		if (m_bRepeat[0]) {
			r2.right = r2.left + repeatBitmap.bmWidth;
			if (backColor == RGB(223,227,239)) {
				DrawBitmap(pDC,r2,m_RepeatGrayBmp);
			} else {
				DrawBitmap(pDC,r2,m_RepeatBmp);
			}
			width = r2.right-r2.left;
			r2.left += width + leftMargin;
		}
		r2.right = rect.right;
		pDC->DrawText (text[0], &r2, format | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
		if (m_bTextLine) {
			CPen* pOldPen = pDC->SelectObject(&linePen);
	//		pDC->MoveTo (r2.left, r2.bottom -1);
	//		pDC->LineTo (r2.right, r2.bottom -1);
			pDC->MoveTo (r2.left, rect.bottom);
			pDC->LineTo (r2.right, rect.bottom);
			pDC->SelectObject(pOldPen);
		}
	} else {
		rect.bottom = rect.top + height;
		RECT r3={rect.left,rect.top,rect.right,rect.bottom};
		for (int i=0;i<textCount;i++) {
			if (m_bAlarm[i]) {
				r3.right = r3.left + alarmBitmap.bmWidth;
				if (backColor == RGB(223,227,239)) {
					DrawBitmap(pDC,r3,m_AlarmGrayBmp);
				} else {
					DrawBitmap(pDC,r3,m_AlarmBmp);
				}
				width = r3.right-r3.left;
				r3.left += width;
			}
			if (m_bRepeat[i]) {
				r3.right = r3.left + repeatBitmap.bmWidth;
				if (backColor == RGB(223,227,239)) {
					DrawBitmap(pDC,r3,m_RepeatGrayBmp);
				} else {
					DrawBitmap(pDC,r3,m_RepeatBmp);
				}
				width = r3.right-r3.left;
				r3.left += width + leftMargin;
			}
			r3.right = rect.right;
			pDC->DrawText (text[i], &r3, format | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
			
			if (m_bTextLine) {
				CPen* pOldPen = pDC->SelectObject(&linePen);
				pDC->MoveTo (r3.left, r3.bottom-1 );
				pDC->LineTo (r3.right, r3.bottom-1);
				pDC->SelectObject(pOldPen);
			}

			r3.top += height;
			r3.bottom = r3.top + height;
			r3.left = rect.left;
			r3.right = rect.right;
		}
	}
	pDC->SetTextColor(oldTextColor);
	pDC->SetBkMode (oldBkMode);
	pDC->SelectObject (oldFont);

	tempFont.DeleteObject ();

	return 0;
}

int XCell::DrawLabel(CDC* pDC, RECT rect)
{
	return 0;
}

int XCell::Draw(CDC* pDC, RECT rect)
{

	RECT rect1;

	rect1.left = rect.left+1;
	rect1.top = rect.top+1;
	rect1.right = rect.right;
	rect1.bottom = rect.bottom;

	DrawBackground (pDC, rect);
	DrawBorder (pDC, rect);

	DrawText (pDC, rect1);

	return 0;
}

int XCell::Draw(CDC* pDC, RECT rect,BOOL bDrawBackground)
{

	RECT rect1;

	rect1.left = rect.left+1;
	rect1.top = rect.top+1;
	rect1.right = rect.right;
	rect1.bottom = rect.bottom;


	if (bDrawBackground) {
		DrawBackground (pDC, rect);
	}
	DrawBorder (pDC, rect);
	DrawText (pDC, rect1);

	return 0;
}
void XCell::SetBorderColor(COLORREF color) {
	borderColor = color;
}

int XCell::DrawBorder(CDC* pDC, RECT rect)
{
	if (colSpan ==0 && rowSpan == 0)
		return 1;

	if(rect.left>rect.right || rect.top >rect.bottom)
		return 1;

	CPen linePen (PS_SOLID, 1, borderColor);

	CPen* pOldPen = pDC->SelectObject(&linePen);
	if(m_bDrawUpLine)
	{
		pDC->MoveTo (rect.left, rect.top);
		pDC->LineTo (rect.right, rect.top);
	}
	else
		pDC->MoveTo(rect.right, rect.top -1);
	if(m_bDrawRightLine)
		pDC->LineTo (rect.right, rect.bottom);
	else
		pDC->MoveTo(rect.right, rect.bottom);

	if(m_bDrawDnLine)
		pDC->LineTo (rect.left, rect.bottom);
	else
		pDC->MoveTo(rect.left, rect.bottom);

	if(m_bDrawLeftLine)
	{
		if(m_bDrawUpLine)
			pDC->LineTo (rect.left, rect.top);
		else
			pDC->LineTo (rect.left, rect.top-1);
	}

	pDC->SelectObject(pOldPen);

	return 0;
}

int XCell::DrawHitBorder (CDC* pDC, RECT rect, COLORREF color) 
{
	CPen pen (PS_SOLID, 1, color);
	int in = 0;

	CPen* oldPen = pDC->SelectObject(&pen);
	if (leftMargin)
		rect.left += max(leftMargin-1,0);
	if (rightMargin)
		rect.right -= max(rightMargin-1,0);

/*	pDC->MoveTo (rect.left + in, rect.top+in);
	pDC->LineTo (rect.right-in, rect.top+in);
	pDC->LineTo (rect.right-in, rect.bottom-in);
	pDC->LineTo (rect.left + in, rect.bottom-in);
	pDC->LineTo (rect.left + in, rect.top+in);
*/
/*	if (m_bTextLine && textCount == 1)
	{
		rect.top -- ;
		rect.bottom --;
	}
*/
	pDC->MoveTo (rect.left , rect.top);
	pDC->LineTo (rect.right, rect.top);
	pDC->LineTo (rect.right, rect.bottom );
	pDC->LineTo (rect.left, rect.bottom);
	pDC->LineTo (rect.left , rect.top);
	pDC->SelectObject(oldPen);
	
	return 0;
}

int XCell::DrawBackground (CDC* pDC, RECT rect)
{
	CBrush bkBrush (backColor);
	pDC->FillRect (&rect, &bkBrush);

	return 0;
}

int XCell::DrawHighLight (CDC* pDC, RECT rect)
{
	CBrush bkBrush (HIGH_LIGHT_BG_COLOR);

	pDC->FillRect (&rect, &bkBrush);

	return 0;
}

int XCell::CalcTextRect (CDC* pDC, RECT* rect)
{
	//	static const WCHAR DefaultGuiFontName_950[] = {'M','i','n','g','L','i','u','\0'};

	CFont* oldFont;
	CFont tempFont;

	if (textFont)
		oldFont = pDC->SelectObject (textFont);
	else
	{
		tempFont.CreateFont (textFontSize,0,0, 0, FW_NORMAL, FALSE, FALSE, 0,
			DEFAULT_CHARSET,OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS,  textFontFace);
		oldFont = pDC->SelectObject (&tempFont);
	}

	if (leftMargin)
		rect->left += leftMargin;

	BOOL bEmpty = FALSE;
	if (text[0].IsEmpty()) {
		text[0] = "TEST";
		bEmpty = TRUE;
	}
	int height = pDC->DrawText (text[0], rect, format | DT_CALCRECT | DT_SINGLELINE);

	rect->bottom += leftMargin + 2;

	if (bEmpty) {
		text[0] = _T("");
	}

	pDC->SelectObject (oldFont);

	tempFont.DeleteObject ();

	return 0;
}

int XCell::GetSpan(int &rows, int &cols)
{
	rows = rowSpan;
	cols = colSpan;

	return 0;

}

int XCell::Draw(CDC *pDC, RECT rect, BOOL bMore,RECT moreRect, CBitmap &bmp,BOOL bdrawBackground)
{
	Draw(pDC,rect,bdrawBackground);

	if (bMore)
	{
		if (backColor == RGB(223,227,239)) {
			DrawBitmap(pDC,moreRect,m_MoreGrayBmp);
		} else {
			DrawBitmap(pDC,moreRect,m_MoreBmp);
		}
	/*	bmp.AssertValid();
		if (!bmp.GetSafeHandle()) {
			return -1;
		} 
		BITMAP bmpInfo;
		bmp.GetBitmap(&bmpInfo);

		// Create an in-memory DC compatible with the
		// display DC we're using to paint
		CDC dcMemory;
		dcMemory.CreateCompatibleDC(pDC);

		// Select the bitmap into the in-memory DC
		CBitmap* pOldBitmap = dcMemory.SelectObject(&bmp);


		// Copy the bits from the in-memory DC into the on-
		// screen DC to actually do the painting.
		CRect cr(rect);
		CPoint leftTop(moreRect.left+1,moreRect.top+1),rightBottom(moreRect.right-1,moreRect.bottom-1);
		
		if (cr.PtInRect(leftTop) && cr.PtInRect(rightBottom)) {
			pDC->BitBlt(moreRect.left,moreRect.top,bmpInfo.bmWidth,bmpInfo.bmHeight,&dcMemory,
			        0,0,SRCCOPY);
		}
		dcMemory.SelectObject(pOldBitmap);*/
		
	}
	return 0;
}

int XCell::Draw(CDC *pDC, RECT rect, int align, CBitmap &bmp)
{
	Draw(pDC,rect);

	if (bmp.GetSafeHandle()) {
		BITMAP bmpInfo;
		bmp.GetBitmap(&bmpInfo);

		// Create an in-memory DC compatible with the
		// display DC we're using to paint
		CDC dcMemory;
		dcMemory.CreateCompatibleDC(pDC);

		// Select the bitmap into the in-memory DC
		CBitmap* pOldBitmap = dcMemory.SelectObject(&bmp);


		// Copy the bits from the in-memory DC into the on-
		// screen DC to actually do the painting.
		CRect cr(rect);
		int x=0,y=0;
		
		if (align == ALIGN_LEFT) {
			x = 0;
		} else if (align == ALIGN_CENTER) {
			x = rect.left + ((rect.right - rect.left) - bmpInfo.bmWidth) / 2;
		} else if (align == ALIGN_RIGHT) {
			x = rect.right - bmpInfo.bmWidth;
		}
		//y is middle
		y = rect.top + ((rect.bottom - rect.top) - bmpInfo.bmHeight) / 2;
		pDC->BitBlt(x,y,bmpInfo.bmWidth,bmpInfo.bmHeight,&dcMemory,0,0,SRCCOPY);

		dcMemory.SelectObject(pOldBitmap);
		
	}
	return 0;
}
//Test, Nono_2004_0407
int XCell::Draw2Bitmap(CDC *pDC, RECT rect, CBitmap &bmpL, CBitmap &bmpR)
{
	Draw(pDC,rect);

	//set the gap arround the Bitmap.
	int nGap=2;

	//Draw the bitmap(bmpL) on the left side
	if (bmpL.GetSafeHandle()) {
		BITMAP bmpInfo;
		bmpL.GetBitmap(&bmpInfo);

		// Create an in-memory DC compatible with the
		// display DC we're using to paint
		CDC dcMemory;
		dcMemory.CreateCompatibleDC(pDC);

		// Select the bitmap into the in-memory DC
		CBitmap* pOldBitmap = dcMemory.SelectObject(&bmpL);


		// Copy the bits from the in-memory DC into the on-
		// screen DC to actually do the painting.
		CRect cr(rect);
		int x=0,y=0;
		
		//x = nGap;
        x = rect.left + ((rect.right - rect.left)/2 - bmpInfo.bmWidth) / 2;

		//y is middle
		y = rect.top + ((rect.bottom - rect.top) - bmpInfo.bmHeight) / 2;
		pDC->BitBlt(x,y,bmpInfo.bmWidth,bmpInfo.bmHeight,&dcMemory,0,0,SRCCOPY);

		dcMemory.SelectObject(pOldBitmap);
		
	}

	//Draw the bitmap(bmpR) on the right side
	if (bmpR.GetSafeHandle()) {
		BITMAP bmpInfo;
		bmpR.GetBitmap(&bmpInfo);

		// Create an in-memory DC compatible with the
		// display DC we're using to paint
		CDC dcMemory;
		dcMemory.CreateCompatibleDC(pDC);

		// Select the bitmap into the in-memory DC
		CBitmap* pOldBitmap = dcMemory.SelectObject(&bmpR);


		// Copy the bits from the in-memory DC into the on-
		// screen DC to actually do the painting.
		CRect cr(rect);
		int x=0,y=0;
		
        x = rect.right - ((rect.right - rect.left)/2 - bmpInfo.bmWidth) / 2 - bmpInfo.bmWidth;
		//y is middle
		y = rect.top + ((rect.bottom - rect.top) - bmpInfo.bmHeight) / 2;
		pDC->BitBlt(x,y,bmpInfo.bmWidth,bmpInfo.bmHeight,&dcMemory,0,0,SRCCOPY);

		dcMemory.SelectObject(pOldBitmap);
		
	}
	return 0;
}


int XCell::SetAlarm(BOOL bAlarm)
{
	m_bAlarm[0] = bAlarm;
	return 0;
}
int XCell::SetAlarm(int idx,BOOL bAlarm)
{
	if ((idx<0) || (idx>47)) {
		return -1;
	}
	m_bAlarm[idx] = bAlarm;

	return 0;
}
int XCell::SetAlarm(BOOL bAlarm[48],int count)
{
	if ((count<0) || (count>48)) {
		return -1;
	}
	for (int i=0;i<count;i++) {
		m_bAlarm[i] = bAlarm[i];
	}

	return 0;
}

int XCell::SetRepeat(BOOL bRepeat)
{
    m_bRepeat[0] = bRepeat;
	return 0;
}
int XCell::SetRepeat(int idx,BOOL bRepeat)
{
	if ((idx<0) || (idx>47)) {
		return -1;
	}
	m_bRepeat[idx] = bRepeat;

	return 0;
}
int XCell::SetRepeat(BOOL bRepeat[48],int count)
{
	if ((count<0) || (count>48)) {
		return -1;
	}
	for (int i=0;i<count;i++) {
		m_bRepeat[i] = bRepeat[i];
	}

	return 0;
}

void XCell::SetAlarmGrayBmp(int id) {
	m_AlarmGrayBmp.LoadBitmap(id);
}

void XCell::SetAlarmBmp(int id) {
	m_AlarmBmp.LoadBitmap(id);
	//	BITMAP bmp;
	//	m_AlarmBmp.GetBitmap(&bmp);
	//	
	//	CImage img(&m_AlarmBmp);
	//	img.
	//	Color bgColor;
	//	Color curBgColor;
	//	img.GetPixel(0,0,&bgColor);
	//	for (int i=0;i<bmp.bmWidth;i++) {
	//		for (int j=0;j<bmp.bmHeight;j++) {
	//			img.GetPixel(i,j,&curBgColor);
	//			if (curBgColor.ToCOLORREF() == bgColor.ToCOLORREF()) {
	//				img.SetPixel(i,j,RGB(0xCC,0xCC,0xCC));
	//			}
	//		}
	//	}
	//	HBITMAP hb;
	//	img.GetHBITMAP(NULL,&hb);
	//	m_AlarmGrayBmp.FromHandle(hb);
	//	//	m_AlarmGrayBmp.CreateBitmapIndirect(&bmp);
}

void XCell::SetRepeatBmp(int id) {
	m_RepeatBmp.LoadBitmap(id);
}
void XCell::SetRepeatGrayBmp(int id) {
	m_RepeatGrayBmp.LoadBitmap(id);
}
void XCell::SetMoreBmp(int id)
{
	m_MoreBmp.LoadBitmap(id);
}
void XCell::SetMoreGrayBmp(int id)
{
	m_MoreGrayBmp.LoadBitmap(id);
}

void XCell::DrawBitmap(CDC *pDC, RECT rect, CBitmap &bmp) {
		BITMAP bmpInfo;
		bmp.GetBitmap(&bmpInfo);

		// Create an in-memory DC compatible with the
		// display DC we're using to paint
		CDC dcMemory;
		dcMemory.CreateCompatibleDC(pDC);

		// Select the bitmap into the in-memory DC
		CBitmap* pOldBitmap = dcMemory.SelectObject(&bmp);


		// Copy the bits from the in-memory DC into the on-
		// screen DC to actually do the painting.
		rect.top += 2;
		rect.bottom -= 2;
		int width = min(rect.right-rect.left,rect.bottom-rect.top);
		pDC->StretchBlt(rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,&dcMemory,
			0,0,bmpInfo.bmWidth,bmpInfo.bmHeight,SRCCOPY);
		//		pDC->BitBlt(rect.left,rect.top,bmpInfo.bmWidth,bmpInfo.bmHeight,&dcMemory,
		//			    0,0,SRCCOPY);

		dcMemory.SelectObject(pOldBitmap);
}
int XCell::SetDrawUpLine(BOOL bDraw)
{
	m_bDrawUpLine = bDraw ? 1 : 0;
	return 0;
}
int XCell::SetDrawDnLine(BOOL bDraw)
{
	m_bDrawDnLine = bDraw ? 1 : 0;
	return 0;
}
int XCell::SetDrawLeftLine(BOOL bDraw)
{
	m_bDrawLeftLine = bDraw ? 1 : 0;
	return 0;
}
int XCell::SetDrawRightLine(BOOL bDraw)
{
	m_bDrawRightLine = bDraw ? 1 : 0;
	return 0;
}
