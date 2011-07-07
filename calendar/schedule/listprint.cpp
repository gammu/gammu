// ListDemoView.cpp : implementation of the CListPrint class
//

#include "stdafx.h"
#include "ListPrint.h"
#include "..\UnicodeString\UnicodeString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int _CurrentPrintCol_=0;
static int _CurrentPrintOrder_=0;
static int _nStartLeftCord_=0;

static int _CurrentPrintCol_Header_=0;
static int _CurrentPrintOrder_Header_=0;
static int _nStartLeftCord_Header_=0;
static int _CutRemainderWidth_=0;
static int _CutRemainderWidth_Header_=0;
/////////////////////////////////////////////////////////////////////////////
// CListPrint construction/destruction

CListPrint::CListPrint()
{	
   m_pListCtrl = NULL;
   m_pListView = NULL;

   m_pPrintInfo = NULL;
}

CListPrint::~CListPrint()
{
}

/////////////////////////////////////////////////////////////////////////////
// CListPrint printing

BOOL CListPrint::OnPreparePrinting(CPrintInfo* pInfo)
{
   UNUSED_ALWAYS(pInfo);
   m_RatioX = 0; 
   m_rectPaper  = CRect(0,0,0,0);
   m_rectPage   = CRect(0,0,0,0);
   m_rectHeader = CRect(0,0,0,0);
   m_rectFooter = CRect(0,0,0,0);
   m_rectBody   = CRect(0,0,0,0);
   m_pFontHeader = NULL;
   m_pFontFooter = NULL;
   m_pFontColumn = NULL;
   m_pFontBody   = NULL;
   m_CharSizeHeader = CSize(0,0);
   m_CharSizeFooter = CSize(0,0);
   m_CharSizeBody   = CSize(0,0);
   m_marginLeft   = MARGIN_LEFT;
   m_marginTop    = MARGIN_TOP;
   m_marginRight  = MARGIN_RIGHT;
   m_marginBottom = MARGIN_BOTTOM;
   m_headerHeight = 0;
   m_footerHeight = 0;
   m_headerLines = HEADER_LINES;
   m_footerLines = FOOTER_LINES;
   m_nPageCols = 0;
   m_nPageRows = 0;
   m_nPageCount = 0;
   m_nRowCount = 0;

   m_CurrentPrintCol = 0;

   ZeroMemory(m_nColumns,sizeof(m_nColumns));
	return TRUE;
}

void CListPrint::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{   
   // Create fonts
   ASSERT(pDC != NULL && pInfo != NULL);
   m_pFontHeader = CreateFont(pDC,_T("Arial"), 12, FW_BOLD);
   m_pFontFooter = CreateFont(pDC,_T("Arial"), 10);
   m_pFontColumn = CreateFont(pDC,_T("Arial"), 9, FW_BOLD);
   m_pFontBody   = CreateFont(pDC,_T("Times New Roman"), 10);

   // Calculate character size
   m_CharSizeHeader = GetCharSize(pDC, m_pFontHeader);
   m_CharSizeFooter = GetCharSize(pDC, m_pFontFooter);
   m_CharSizeBody   = GetCharSize(pDC, m_pFontBody);

   // Prepare layout 
   m_rectPaper  = GetPaperRect(pDC);
   m_rectPage   = GetPageRect();
   m_rectHeader = GetHeaderRect();
   m_rectFooter = GetFooterRect();
   m_rectBody = GetBodyRect();
   m_RatioX = GetTextRatioX(pDC);   
   m_nRowCount = GetRowCount();
   m_nPageCols = GetPageColumns();
   m_nPageRows = GetPageRows();
   m_nPageCount = (m_nRowCount + m_nPageRows - 1) / m_nPageRows;	
   m_nAdditionalPageRatio = AdditionalPageRatio(pDC);
   m_nRealPageCount = m_nPageCount * m_nAdditionalPageRatio;
   _CurrentPrintCol_=0;
   _CurrentPrintOrder_=0;
   _nStartLeftCord_=0;
 
   _CurrentPrintCol_Header_=0;
   _CurrentPrintOrder_Header_=0;
   _nStartLeftCord_Header_=0;
 
   // Column order
   ASSERT(m_pListCtrl);
   m_pListCtrl->GetColumnOrderArray(m_nColumns);

   // How many pages?
   pInfo->SetMaxPage(m_nRealPageCount);   
}

void CListPrint::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
#ifdef _DEBUG
   DrawRect(pDC, m_rectPaper,RGB(0,0,0));
   DrawRect(pDC, m_rectPage,RGB(0,0,255));
#endif

   m_pPrintInfo = pInfo;

   PrintHeader(pDC, pInfo);
   PrintBody(pDC, pInfo);
   PrintFooter(pDC, pInfo);
}

void CListPrint::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
   delete m_pFontHeader;
   delete m_pFontFooter;
   delete m_pFontColumn;
   delete m_pFontBody;
}

/////////////////////////////////////////////////////////////////////////////
// CListPrint diagnostics

#ifdef _DEBUG
void CListPrint::DrawRect(CDC* pDC, CRect rect, COLORREF color)
{
   UNUSED_ALWAYS(pDC);
   UNUSED_ALWAYS(rect);
   UNUSED_ALWAYS(color);

#if 0
   CPen pen;
   pen.CreatePen(PS_SOLID, 3, color);
   CPen* pOldPen = pDC->SelectObject(&pen);
   pDC->Rectangle(rect);
   pDC->SelectObject(pOldPen);
#endif
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CListPrint helpers

void CListPrint::DrawRow(CDC* pDC, int nRow)
{
   ASSERT(m_pListView != NULL);

   int nCol = _CurrentPrintCol_;
   int _nStartLeftCord_ = GetCellRect(nRow,nCol).left;

   for (nCol; nCol < m_nPageCols; nCol++)
   //for (int nCol = 0; nCol < m_nPageCols; nCol++)
   {
	   if (_CutRemainderWidth_) {//Remainder page.
		   if (_CutRemainderWidth_ > m_rectBody.right) {
			   _CutRemainderWidth_ -= m_rectBody.right;
	          break;//go to another remainder page.
		   }else {
			   //m_rectBody.left = _CutRemainderWidth_;//
			   _CutRemainderWidth_ = 0;
		   }
	   }
	   //
	   if (_CurrentPrintOrder_) {
           CString strText = m_pListCtrl->GetItemText(nRow, m_nColumns[nCol]);

           CRect rect  = GetCellRect(nRow,nCol);
    	  rect.right -= _nStartLeftCord_ - m_rectBody.left + _CutRemainderWidth_;
    	  rect.left  -= _nStartLeftCord_ - m_rectBody.left + _CutRemainderWidth_;
          rect.left += (int)(CELL_PADDING * m_RatioX);
    	  if (rect.right > m_rectBody.right) {
		      if (_CurrentPrintCol_ != nCol) {
	         	  break;
			  }else {
				   _CutRemainderWidth_ = rect.right - m_rectBody.right;
				   rect.right = m_rectBody.right;
			  }
		  }
          DrawText(pDC->m_hDC, strText, -1, rect, DT_SINGLELINE|DT_NOPREFIX|DT_LEFT|DT_VCENTER);
	   }else {
           CString strText = m_pListCtrl->GetItemText(nRow, m_nColumns[nCol]);
           CRect rect  = GetCellRect(nRow,nCol);
		   CSize nTextExtent = pDC->GetTextExtent(strText);
           if (rect.right > m_rectBody.right) {
		       if (_CurrentPrintCol_ != nCol) {
	         	  break;
			   }else {
				   _CutRemainderWidth_ = rect.right - m_rectBody.right;
				   rect.right = m_rectBody.right;
			   }
		   }
           DrawText(pDC->m_hDC, strText, -1, rect, DT_SINGLELINE|DT_NOPREFIX|DT_LEFT|DT_VCENTER);
	   }
   }


}

CRect CListPrint::GetPaperRect(CDC* pDC)
{
   ASSERT(pDC != NULL);
   CSize paperSize = CSize(pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));
   return CRect(0, 0, paperSize.cx, paperSize.cy);
}

CRect CListPrint::GetPageRect()
{
   ASSERT(m_rectPaper != CRect(0,0,0,0));
   CRect rect = m_rectPaper;
   rect.DeflateRect(m_marginLeft, m_marginTop, m_marginRight, m_marginBottom);
   return rect;
}

CRect CListPrint::GetHeaderRect()
{
   ASSERT(m_rectPage != CRect(0,0,0,0));
   CRect rect = m_rectPage;
   CSize charSize = m_CharSizeHeader;
   rect.DeflateRect(0, 0, 0, rect.Height() - m_headerLines * charSize.cy);
   return rect;
}

CRect CListPrint::GetFooterRect()
{
   ASSERT(m_rectPage != CRect(0,0,0,0));
   CRect rect = m_rectPage;
   CSize charSize = m_CharSizeFooter;
   rect.DeflateRect(0, rect.Height() - m_footerLines * charSize.cy, 0, 0);
   return rect;
}

CRect CListPrint::GetBodyRect()
{
   ASSERT(m_rectPage != CRect(0,0,0,0));
   ASSERT(m_rectHeader != CRect(0,0,0,0));
   ASSERT(m_rectFooter != CRect(0,0,0,0));
   CRect rectPage = m_rectPage;
   CRect rectHead = m_rectHeader;
   CRect rectFoot = m_rectFooter;
   rectPage.DeflateRect(0, rectHead.Height(), 0, rectFoot.Height());
   return rectPage;
}

CSize CListPrint::GetCharSize(CDC* pDC, CFont* pFont)
{
   ASSERT(pDC != NULL && pFont != NULL);
   CFont *pOldFont = pDC->SelectObject(pFont);
   CSize charSize = pDC->GetTextExtent(_T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSATUVWXYZ"),52);
   charSize.cx /= 52;
   pDC->SelectObject(pOldFont);
   return charSize;
}

int CListPrint::GetPageColumns()
{
   ASSERT(m_pListCtrl);
   CHeaderCtrl* pHeaderCtrl = m_pListCtrl->GetHeaderCtrl();
   ASSERT(pHeaderCtrl != NULL);
   return pHeaderCtrl->GetItemCount();
}

CString CListPrint::GetColumnHeading(int col)
{
   ASSERT(m_pListCtrl);
   CHeaderCtrl* pHeaderCtrl = m_pListCtrl->GetHeaderCtrl();
   ASSERT(pHeaderCtrl != NULL);
   TCHAR Buffer[1024];

   HDITEM  phi;
   phi.mask = HDI_TEXT;
   phi.cchTextMax = sizeof(Buffer);
   phi.pszText = Buffer;
   pHeaderCtrl->GetItem(m_nColumns[col], &phi);
   return CString(Buffer);
}

CRect CListPrint::GetColumnRect(int nCol)
{
   ASSERT(nCol >= 0 && nCol < m_nPageCols);
   CSize charSize = m_CharSizeBody;
   CRect rectBody = m_rectBody;
   for (int nLeft=0, i=0; i < nCol; i++) 
      nLeft += GetColumnWidth(m_nColumns[i]);
   int nRight = nLeft + GetColumnWidth(m_nColumns[i]);
   CRect rect(
      rectBody.left + nLeft, 
      rectBody.top, 
      rectBody.left + nRight, 
      rectBody.top + charSize.cy);
   if (rect.right > rectBody.right) {
       ;//rect.right = rectBody.right;
   }
   rect.left += (int)(CELL_PADDING * m_RatioX);
   return rect;
}

CRect CListPrint::GetCellRect(int nRow, int nCol)
{
   ASSERT(nCol >= 0 && nCol < m_nPageCols);
   ASSERT(nRow >= 0 && nRow < m_nRowCount);
   CSize charSize = m_CharSizeBody;
   CRect rectBody = m_rectBody;
   CRect rectCol = GetColumnRect(0);
   for (int nLeft=0, i=0; i < nCol; i++) 
      nLeft += GetColumnWidth(m_nColumns[i]);
   int nRight = nLeft + GetColumnWidth(m_nColumns[i]);   
   int nPageRow =  nRow % m_nPageRows;
   ASSERT(nPageRow <= m_nPageRows);
   CRect rect(
      rectBody.left + nLeft, 
      rectBody.top + rectCol.Height() + charSize.cy * nPageRow, 
      rectBody.left + nRight, 
      rectBody.top + rectCol.Height() + charSize.cy * (nPageRow + 1));
   if (rect.right > rectBody.right) {
      ;//rect.right = rectBody.right;
   }
   rect.left += (int)(CELL_PADDING * m_RatioX);
   return rect;
}

int CListPrint::GetPageRows()
{
   CSize charSize = m_CharSizeBody;
   CRect rectBody = m_rectBody;
   CRect rectCol = GetColumnRect(0);
   return (rectBody.Height() - rectCol.Height())/ charSize.cy;
}

int CListPrint::GetColumnWidth(int col)
{
   ASSERT(m_pListCtrl);
   CHeaderCtrl* pHeaderCtrl = m_pListCtrl->GetHeaderCtrl();
   ASSERT(pHeaderCtrl != NULL);
   HDITEM  hi;
   hi.mask = HDI_WIDTH;
   pHeaderCtrl->GetItem(col, &hi);
   return (int)((hi.cxy)* m_RatioX);
}

double CListPrint::GetTextRatioX(CDC* pDC)
{
   ASSERT(pDC != NULL);
   ASSERT(m_pListCtrl);
   CDC* pCurrentDC = m_pListCtrl->GetDC();
   TEXTMETRIC tmSrc;
   pCurrentDC->GetTextMetrics(&tmSrc);
   m_pListCtrl->ReleaseDC(pCurrentDC);
   return ((double)m_CharSizeBody.cx) / ((double)tmSrc.tmAveCharWidth);
}

int CListPrint::GetRowCount()
{
   ASSERT(m_pListCtrl);
   return m_pListCtrl->GetItemCount();
}

CFont* CListPrint::CreateFont(CDC* pDC, CString strName, int nPoints, int lfWeight, bool lfItalic)
{
   ASSERT(pDC && m_pListCtrl);   
   CDC* pCurrentDC = m_pListCtrl->GetDC();
   LOGFONT lf; 
   TEXTMETRIC tm;
   CFont* pCurrentFont = m_pListCtrl->GetFont();
   pCurrentFont->GetLogFont(&lf);
   pCurrentDC->GetTextMetrics(&tm);
   CFont* pFont = new CFont();
   CSize PaperPixelsPerInch(
      pDC->GetDeviceCaps(LOGPIXELSX), 
      pDC->GetDeviceCaps(LOGPIXELSY));
   CSize ScreenPixelsPerInch(
      pCurrentDC->GetDeviceCaps(LOGPIXELSX), 
      pCurrentDC->GetDeviceCaps(LOGPIXELSY));
   if (nPoints == 0)
      nPoints = MulDiv((tm.tmHeight - tm.tmInternalLeading),72,ScreenPixelsPerInch.cy);
   lf.lfWeight = lfWeight;
   lf.lfItalic = lfItalic;
   lf.lfHeight = -MulDiv(nPoints,PaperPixelsPerInch.cy,72);  
   if (!strName.IsEmpty())
      _tcscpy(lf.lfFaceName, strName);  
   m_pListCtrl->ReleaseDC(pCurrentDC);
   BOOL bResult = pFont->CreateFontIndirect(&lf);
   ASSERT(bResult == TRUE);
   return pFont;
}

/////////////////////////////////////////////////////////////////////////////
// CListPrint printing

void CListPrint::PrintHeader(CDC* pDC, CPrintInfo* pInfo)
{
   ASSERT(m_pFontHeader && pInfo);
   UNUSED_ALWAYS(pInfo);
   CFont *pOldFont = pDC->SelectObject(m_pFontHeader);

#ifdef _DEBUG
   DrawRect(pDC, m_rectHeader,RGB(0,255,0));
#endif

   // Create black brush
   CBrush brush;
   brush.CreateSolidBrush(RGB(0,0,0));
   CBrush* pOldBrush = pDC->SelectObject(&brush);

   // Draw line
   CRect rectLine = m_rectHeader;
   rectLine.top = rectLine.top + m_CharSizeHeader.cy * 3 / 2;
   rectLine.bottom = rectLine.top + HEADER_LINE_SIZE;
   pDC->FillRect(&rectLine, &brush);
   pDC->SelectObject(pOldBrush);

   // Draw "Program" and "Document"
   CRect rectHeader = m_rectHeader;
   rectHeader.bottom = rectHeader.top + m_CharSizeHeader.cy;
   pDC->DrawText(m_strAppName, rectHeader, DT_LEFT | DT_BOTTOM);
   pDC->DrawText(m_strDocName, rectHeader, DT_RIGHT | DT_BOTTOM);   
   pDC->SelectObject(pOldFont);
}

void CListPrint::PrintBody(CDC* pDC, CPrintInfo* pInfo)
{
   ASSERT(m_pFontBody && m_pFontColumn && pInfo);
   ASSERT(m_pListView);

#ifdef _DEBUG
   DrawRect(pDC, m_rectBody,RGB(255,255,0));
   for (int j=0; j < m_nPageCols; j++)
      DrawRect(pDC, GetColumnRect(j),RGB(128,64,0));
#endif

   // Print column headings
   CFont *pOldFont = pDC->SelectObject(m_pFontColumn); 

   int i = _CurrentPrintCol_Header_;
   _CurrentPrintCol_ = _CurrentPrintCol_Header_;
   _CurrentPrintOrder_ = _CurrentPrintOrder_Header_;
   _nStartLeftCord_Header_ = GetColumnRect(i).left;
   for (i; i < m_nPageCols; i++) {
   //for (int i = 0; i < m_nPageCols; i++)
	   if (_CutRemainderWidth_Header_) {//Remainder page.
		   if (_CutRemainderWidth_Header_ > m_rectBody.right) {
			   _CutRemainderWidth_Header_ -= m_rectBody.right;
	          break;//go to another remainder page.
		   }else {
			   //m_rectBody.left = _CutRemainderWidth_Header_;//
			   _CutRemainderWidth_Header_ = 0;
		   }
	   }
	   //
	   if (_CurrentPrintOrder_Header_) {//Repeat page.
	       CRect rect = GetColumnRect(i);
	       rect.right -= (_nStartLeftCord_Header_ - m_rectBody.left) + _CutRemainderWidth_;
	       rect.left  -= (_nStartLeftCord_Header_ - m_rectBody.left) + _CutRemainderWidth_;
           rect.left += (int)(CELL_PADDING * m_RatioX);
    	   if (rect.right > m_rectBody.right) {
	       	  _CurrentPrintOrder_Header_++;
			   if (_CurrentPrintCol_Header_ != i) {
	     	      break;
			   }else {
				   _CutRemainderWidth_Header_ = rect.right - m_rectBody.right;
				   rect.right = m_rectBody.right;
			   }
		   }
           DrawText(pDC->m_hDC, GetColumnHeading(i), -1, rect, DT_NOPREFIX|DT_LEFT | DT_VCENTER);
	   }else {//First page.
	       CRect rect = GetColumnRect(i);
    	   if (rect.right > m_rectBody.right) {
         	  _CurrentPrintOrder_Header_++;
			   if (_CurrentPrintCol_Header_ != i) {
	         	  break;
			   }else {
				   _CutRemainderWidth_Header_ = rect.right - m_rectBody.right;
				   rect.right = m_rectBody.right;
			   }
		   }

           DrawText(pDC->m_hDC, GetColumnHeading(i), -1, rect, DT_NOPREFIX|DT_LEFT | DT_VCENTER);
	   }
   }
   //
   if (i >= m_nPageCols) {
	   _CurrentPrintCol_Header_ = 0;
	   _CurrentPrintOrder_Header_ = 0;
   }
   else {
 	   _CurrentPrintCol_Header_ = i;
   }
 

   // Start and end row
   pDC->SelectObject(m_pFontBody);
   int nStartRow = 0;
   if (m_nAdditionalPageRatio) {
      nStartRow = //2004_1021
	   ( (int)((pInfo->m_nCurPage-1)/m_nAdditionalPageRatio) ) * m_nPageRows;
   }else {
      nStartRow = 
 	   ( pInfo->m_nCurPage - 1) * m_nPageRows;
   }
   int nEndRow = nStartRow + m_nPageRows;
   if(nEndRow > m_nRowCount)       
      nEndRow = m_nRowCount;

   // Prepare document and print rows
   for (i=nStartRow; i < nEndRow; i++)
      DrawRow(pDC, i);
   pDC->SelectObject(pOldFont);
}

void CListPrint::PrintFooter(CDC* pDC, CPrintInfo* pInfo)
{
   ASSERT(m_pFontFooter != NULL && pInfo != NULL);
   CFont *pOldFont = pDC->SelectObject(m_pFontFooter);

#ifdef _DEBUG
   DrawRect(pDC, m_rectFooter,RGB(0,255,0));
#endif

   // Create black brush
   CBrush brush;
   brush.CreateSolidBrush(RGB(0,0,0));
   CBrush* pOldBrush = pDC->SelectObject(&brush);

   // Draw line
   CRect rectLine = m_rectFooter;
   rectLine.top = rectLine.top + m_CharSizeHeader.cy * 1 / 2;
   rectLine.bottom = rectLine.top + HEADER_LINE_SIZE;
   pDC->FillRect(&rectLine, &brush);
   pDC->SelectObject(pOldBrush);

   // Draw "Pages"
   CString strPages;
   strPages.Format(_T("Page %d of %d"),pInfo->m_nCurPage, m_nRealPageCount);
   //strPages.Format(_T("Page %d of %d"),pInfo->m_nCurPage, m_nPageCount);
   CRect rectPage = m_rectFooter;
   rectPage.top = rectPage.bottom - m_CharSizeFooter.cy;
   pDC->DrawText(strPages, rectPage, DT_RIGHT | DT_BOTTOM);
   pDC->SelectObject(pOldFont);
}

void CListPrint::SetListCtrl(CListCtrl* pList)
{
   ASSERT(pList != NULL);
   m_pListCtrl = pList;
}

void CListPrint::SetListView(CListView* pList)
{
   ASSERT(pList != NULL);
   m_pListView = pList;
}

void CListPrint::SetAppName(const CString& strName)
{
   m_strAppName = strName;
}

void CListPrint::SetDocTitle(const CString &strName)
{
   m_strDocName = strName;
}

int CListPrint::AdditionalPageRatio(CDC* pDC)
{
    int nPrintDocExtend = 0;
    for (int nCol = 0; nCol < m_nPageCols; nCol++)
    {
       nPrintDocExtend += GetColumnWidth(nCol);
    }

	float kk =  (float) nPrintDocExtend/((float)(m_rectBody.right - m_rectBody.left));
	int AdditionalPageRatio =  (int) ceil(kk);

	return   AdditionalPageRatio; 
}

