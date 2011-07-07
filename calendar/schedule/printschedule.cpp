// PrintSchedule.cpp: implementation of the CPrintSchedule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PrintSchedule.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPrintSchedule::CPrintSchedule()
{
    m_pOutputtedDataList = NULL;
}

CPrintSchedule::~CPrintSchedule()
{

}

/////////////////////////////////////////////////////////////////////////////
// CPrintSchedule printing
BOOL CPrintSchedule::OnPreparePrinting(CPrintInfo* pInfo)
{
   UNUSED_ALWAYS(pInfo);
   m_RatioX = 0; 
   m_rectPaper  = CRect(0,0,0,0);
   m_rectPage   = CRect(0,0,0,0);
   m_rectHeader = CRect(0,0,0,0);
   m_rectFooter = CRect(0,0,0,0);
   m_rectBody   = CRect(0,0,0,0);
   m_rectText   = CRect(0,0,0,0);
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
   //ZeroMemory(m_nColumns,sizeof(m_nColumns));

   
	return TRUE;
}


void CPrintSchedule::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
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
   GetColumnOrderArray(m_nColumns);
   m_TitleColumnWidth = GetColumnWidth(m_nColumns.GetAt(0));
   m_FieldColumnWidth = GetColumnWidth(m_nColumns.GetAt(1));

   m_rectPaper  = GetPaperRect(pDC);
   m_rectPage   = GetPageRect();
   m_rectHeader = GetHeaderRect();
   m_rectFooter = GetFooterRect();
   m_rectBody   = GetBodyRect();
   m_rectText   = GetTextRect();
   m_RatioX     = GetTextRatioX(pDC);   
   m_nRowCount  = GetRowCount();
   m_nPageCols  = GetPageColumns();
   m_nPageRows  = GetPageRows();
   m_nPageCount = (20+m_nRowCount + m_nPageRows - 1) / m_nPageRows;	

   // Column order
   //ASSERT(m_pListCtrl);
   //m_pListCtrl->GetColumnOrderArray(m_nColumns);

   // How many pages?
   pInfo->SetMaxPage(m_nPageCount);   
}

void CPrintSchedule::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
#ifdef _DEBUG
   DrawRect(pDC, m_rectPaper,RGB(0,0,0));
   DrawRect(pDC, m_rectPage,RGB(0,0,255));
#endif
   PrintHeader(pDC, pInfo);
   PrintBody(pDC, pInfo);
   //Nono
   PrintText(pDC, pInfo);

   PrintFooter(pDC, pInfo);
}

void CPrintSchedule::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
   delete m_pFontHeader;
   delete m_pFontFooter;
   delete m_pFontColumn;
   delete m_pFontBody;
}

/////////////////////////////////////////////////////////////////////////////
// CPrintSchedule diagnostics

#ifdef _DEBUG
void CPrintSchedule::DrawRect(CDC* pDC, CRect rect, COLORREF color)
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
// CPrintSchedule helpers

void CPrintSchedule::DrawRow(CDC* pDC, int nRow)
{
   ASSERT(m_pListView != NULL);
   for (int nCol = 0; nCol < m_nPageCols; nCol++)
   {
      CRect rect = GetCellRect(nRow,nCol);
      //CString strText = m_pListCtrl->GetItemText(nRow, m_nColumns[nCol]);
	  POSITION pos = m_pOutputtedDataList->FindIndex(nRow);
	  CString* pCUString = (CString*) m_pOutputtedDataList->GetAt(pos);
      CString strText(pCUString[nCol].GetBuffer(20));

      DrawText(pDC->m_hDC, strText, -1, rect, DT_LEFT | DT_VCENTER);
   }
}

/////////////////////
// Functions to prepare layout =>
CRect CPrintSchedule::GetPaperRect(CDC* pDC)
{
   ASSERT(pDC != NULL);
   CSize paperSize = CSize(pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));
   return CRect(0, 0, paperSize.cx, paperSize.cy);
}

CRect CPrintSchedule::GetPageRect()
{
   ASSERT(m_rectPaper != CRect(0,0,0,0));
   CRect rect = m_rectPaper;
   rect.DeflateRect(m_marginLeft, m_marginTop, m_marginRight, m_marginBottom);
   return rect;
}

CRect CPrintSchedule::GetHeaderRect()
{
   ASSERT(m_rectPage != CRect(0,0,0,0));
   CRect rect = m_rectPage;
   CSize charSize = m_CharSizeHeader;
   rect.DeflateRect(0, 0, 0, rect.Height() - m_headerLines * charSize.cy);
   return rect;
}

CRect CPrintSchedule::GetFooterRect()
{
   ASSERT(m_rectPage != CRect(0,0,0,0));
   CRect rect = m_rectPage;
   CSize charSize = m_CharSizeFooter;
   rect.DeflateRect(0, rect.Height() - m_footerLines * charSize.cy, 0, 0);
   return rect;
}

CRect CPrintSchedule::GetBodyRect()
{
   ASSERT(m_rectPage != CRect(0,0,0,0));
   ASSERT(m_rectHeader != CRect(0,0,0,0));
   ASSERT(m_rectFooter != CRect(0,0,0,0));
   CRect rectPage = m_rectPage;
   CRect rectHead = m_rectHeader;
   CRect rectFoot = m_rectFooter;
   CSize charSize = m_CharSizeBody;
   int nRowCount = GetRowCount();
   rectPage.DeflateRect(0, rectHead.Height(), 0, (rectFoot.top-(nRowCount * charSize.cy))+rectFoot.Height());
   return rectPage;
}

CRect CPrintSchedule::GetTextRect()
{
   ASSERT(m_rectPage != CRect(0,0,0,0));
   ASSERT(m_rectHeader != CRect(0,0,0,0));
   ASSERT(m_rectBody != CRect(0,0,0,0));
   ASSERT(m_rectFooter != CRect(0,0,0,0));
   CRect rectPage = m_rectPage;
   CRect rectHead = m_rectHeader;
   CRect rectBody = m_rectBody;
   CRect rectFoot = m_rectFooter;
   rectPage.DeflateRect(0, rectBody.Height()+rectHead.Height(), 0, rectFoot.Height());
   return rectPage;

}

double CPrintSchedule::GetTextRatioX(CDC* pDC)
{
   ASSERT(pDC != NULL);
   ASSERT(m_pListCtrl);
   CDC* pCurrentDC = m_pListCtrl->GetDC();

   TEXTMETRIC tmSrc;
   pCurrentDC->GetTextMetrics(&tmSrc);
   m_pListCtrl->ReleaseDC(pCurrentDC);
   return ((double)m_CharSizeBody.cx) / ((double)tmSrc.tmAveCharWidth);
}

int CPrintSchedule::GetRowCount()
{
   //ASSERT(m_pListCtrl);
   //return m_pListCtrl->GetItemCount();
 
	return (m_pOutputtedDataList->GetCount());
}

int CPrintSchedule::GetPageColumns()
{
/*   ASSERT(m_pListCtrl);
   CHeaderCtrl* pHeaderCtrl = m_pListCtrl->GetHeaderCtrl();
   ASSERT(pHeaderCtrl != NULL);
   return pHeaderCtrl->GetItemCount();
*/
    return PAGE_COLUMNS;
	//return m_nColumns.GetSize();
}

int CPrintSchedule::GetPageRows()
{
   CSize charSize = m_CharSizeBody;
   CRect rectBody = m_rectBody;
   CRect rectCol = GetColumnRect(0);
   return (rectBody.Height() - rectCol.Height() + m_rectText.Height())/ charSize.cy;
}
// Functions to prepare layout <=
/////////////////////
CSize CPrintSchedule::GetCharSize(CDC* pDC, CFont* pFont)
{
   ASSERT(pDC != NULL && pFont != NULL);
   CFont *pOldFont = pDC->SelectObject(pFont);
   CSize charSize = pDC->GetTextExtent(_T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSATUVWXYZ"),52);
   charSize.cx /= 52;
   pDC->SelectObject(pOldFont);
   return charSize;
}

CString CPrintSchedule::GetColumnHeading(int col)
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

CRect CPrintSchedule::GetColumnRect(int nCol)
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
   if (rect.right > rectBody.right)
      rect.right = rectBody.right;
   rect.left += (int)(CELL_PADDING * m_RatioX);
   return rect;
}

CRect CPrintSchedule::GetCellRect(int nRow, int nCol)
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
   if (rect.right > rectBody.right)
      rect.right = rectBody.right;
   rect.left += (int)(CELL_PADDING * m_RatioX);
   return rect;
}

int CPrintSchedule::GetColumnWidth(int col)
{
/* 
   ASSERT(m_pListCtrl);
   CHeaderCtrl* pHeaderCtrl = m_pListCtrl->GetHeaderCtrl();
   ASSERT(pHeaderCtrl != NULL);
   HDITEM  hi;
   hi.mask = HDI_WIDTH;
   pHeaderCtrl->GetItem(col, &hi);
   //int jj = (int)((hi.cxy)* m_RatioX);
   return (int)((hi.cxy)* m_RatioX);
*/

	int nSumWidth = 0;
	POSITION Pos=NULL;
	Pos = m_pOutputtedDataList->GetHeadPosition();
	while(Pos) {
        CString* p_cuString;
	    p_cuString = (CString*) m_pOutputtedDataList->GetNext(Pos);
		nSumWidth += (m_CharSizeBody.cx) * (p_cuString[col].GetLength());
	}
	int  jj2 =  (int)((nSumWidth/(m_pOutputtedDataList->GetCount()))*(11/10)* m_RatioX);

   return (int)((nSumWidth/(m_pOutputtedDataList->GetCount()))/*(11/10)*/* m_RatioX);
}

CFont* CPrintSchedule::CreateFont(CDC* pDC, CString strName, int nPoints, int lfWeight, bool lfItalic)
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
// CPrintSchedule printing

void CPrintSchedule::PrintHeader(CDC* pDC, CPrintInfo* pInfo)
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

void CPrintSchedule::PrintBody(CDC* pDC, CPrintInfo* pInfo)
{
   ASSERT(m_pFontBody && m_pFontColumn && pInfo);
   ASSERT(m_pListView);

   int i=0;
   CFont *pOldFont=NULL;

#ifdef _DEBUG
   DrawRect(pDC, m_rectBody,RGB(255,255,0));
   for (int j=0; j < m_nPageCols; j++)
      DrawRect(pDC, GetColumnRect(j),RGB(128,64,0));
#endif
/*
   // Print column headings
   pOldFont = pDC->SelectObject(m_pFontColumn); 
   for (int i = 0; i < m_nPageCols; i++)
      DrawText(pDC->m_hDC, GetColumnHeading(i), -1, GetColumnRect(i), DT_LEFT | DT_VCENTER);
*/
   // Start and end row
   pDC->SelectObject(m_pFontBody);
   int nStartRow = (pInfo->m_nCurPage - 1) * m_nPageRows;
   int nEndRow = nStartRow + m_nPageRows;
   if(nEndRow > m_nRowCount)       
      nEndRow = m_nRowCount;

   // Prepare document and print rows
   for (i=nStartRow; i < nEndRow; i++)
      DrawRow(pDC, i);
   pDC->SelectObject(pOldFont);
}

void CPrintSchedule::PrintFooter(CDC* pDC, CPrintInfo* pInfo)
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
   strPages.Format(_T("Page %d of %d"),pInfo->m_nCurPage, m_nPageCount);
   CRect rectPage = m_rectFooter;
   rectPage.top = rectPage.bottom - m_CharSizeFooter.cy;
   pDC->DrawText(strPages, rectPage, DT_RIGHT | DT_BOTTOM);
   pDC->SelectObject(pOldFont);
}

void CPrintSchedule::SetListCtrl(CListCtrl* pList)
{
   ASSERT(pList != NULL);
   m_pListCtrl = pList;
}

void CPrintSchedule::SetListView(CListView* pList)
{
   ASSERT(pList != NULL);
   m_pListView = pList;
}

void CPrintSchedule::SetAppName(const CString& strName)
{
   m_strAppName = strName;
}

void CPrintSchedule::SetDocTitle(const CString &strName)
{
   m_strDocName = strName;
}

int CPrintSchedule::GetColumnOrderArray(CArray<int, int>& cArray)
{
	ASSERT(m_pOutputtedDataList);
   	for (int i=0; i<2; i++) {
	    	cArray.Add(i);
	}

	return 0;
}

int CPrintSchedule::CountLinesInText(CString &text, int width, CDC *pDC)
{
	int	count = 0 ;
	int	pos = 0 ;

	while (pos < text.GetLength())
		{
		ExtractLine(text, pos, width, pDC) ;
		count++ ;
		}
	return count ;
}

// find and extract a line of text from the current position onwards in the string
CString CPrintSchedule::ExtractLine(CString &source, int &pos, int width, CDC *pDC)
{
	char CRLF[3] = {13, 10, 0};
	CString	line ;
	CSize	size ;
	bool	pBreak = false ;
	// first skip any whitepace (you may need to remove this if user formatting is required
	while (pos < source.GetLength())
		{
		if (source.GetAt(pos) == ' ')
			++pos ;
		else
			break ;
		}
	if (pos < source.GetLength())
		{
		// there is more data that could be printed
		do {
			if (source.GetAt(pos) == '\r')
				{
				// end of a line, we need to break it here
				if (pos < source.GetLength() - 1 && source.GetAt(pos + 1) == '\n')
					{
					// skip \r\n pair
					pos += 2 ;
					pBreak = true ;	// its a page break
					break ;		// got the line
					}
				else
					{
					// just break the line here
					// skip the \r
					++pos ;
					pBreak = true ;	// its a page break
					break ;
					}
				}
			if (source.GetAt(pos) == '\n')
				{
				// just break the line here
				// skip the \n
				pBreak = true ;		// its a page break
				++pos ;
				break ;
				}
			// add the character to the line to print
			line += source.GetAt(pos) ;
			++pos ;
			// measure the new text to see if it will fit on the page
			size = pDC->GetTextExtent(line) ;
			} while (size.cx < width && pos < source.GetLength()) ;
		}
	// if we get here and its not a page break, we may need to back track on the line to stop a
	// word being broken across 2 lines
	if (!pBreak && size.cx >= width)
		{
		// we may need to back track
		if (line.Right(1) == " ")
			{
			// last character was a space, no need to backtrack
			}
		else
			{
			// go back until we find a space or the begining of this line (which means the word covers
			// more than a whole line, so we should just break it here)
			int		p = pos ;
			int		i = line.GetLength() - 1 ;

			while (i >= 0 && line.GetAt(i) != ' ')
				{
				--i ;
				--pos ;
				}
			if (i < 0)
				{
				// the line is a whole word, just break at this position - 1
				line = line.Left(line.GetLength() - 1) ;
				pos = p - 1 ;
				}
			else
				{
				// we have trimmed to the correct position, but we should skip the space character
				++pos ;
				}
			}
		}
	return line ;
}


void CPrintSchedule::OnBeginPrintingText(CDC* pDC, CPrintInfo* pInfo)
{
	CString	text ;
/*
	m_UserText.GetWindowText(text) ;			// get the text the user wants to print
	// determine the number of pages of output required
    pInfo->m_rectDraw.SetRect(0,0, 
                            pDC->GetDeviceCaps(HORZRES), 
                            pDC->GetDeviceCaps(VERTRES)) ;
*/               
    LOGFONT lf;     memset(&lf, 0, sizeof(LOGFONT));   // Clear out structure.
    ::ZeroMemory(&lf, sizeof(LOGFONT));

	// you can adjust the font size by changing the value of 12
    lf.lfHeight = -MulDiv(12, pDC->GetDeviceCaps(LOGPIXELSY), 72);
    _tcscpy(lf.lfFaceName, _T("Arial"));    //    with face name "Arial".

    CFont	font ;
	font.CreateFontIndirect(&lf);    // Create the font.
	pDC->SaveDC() ;
	pDC->SelectObject(&font);			// RIA, select he font into the DC

	int		num_lines = CountLinesInText(text, m_rectBody.Width(), pDC) ;

	text = "A" ;
	CSize	csText = pDC->GetTextExtent(text) ;	// get the height of a line of text
	int		lines_per_page = (int)(m_rectBody.Height() / csText.cy) ;
	pDC->RestoreDC(-1) ;
	double	pages = (double)num_lines / (double)lines_per_page ;
	// set correct number of pages, include partial page as required
	if (pages - (int)pages != 0.0)
		pInfo->SetMaxPage((int)pages + 1) ;
	else
		pInfo->SetMaxPage((int)pages) ;
	VERIFY(font.DeleteObject()) ;
}

void CPrintSchedule::PrintText(CDC* pDC, CPrintInfo* pInfo)
{
   ASSERT(m_pFontHeader && pInfo);
   UNUSED_ALWAYS(pInfo);
   CFont *pOldFont = pDC->SelectObject(m_pFontBody);

#ifdef _DEBUG
   DrawRect(pDC, m_rectText,RGB(255,255,0));
#endif

   CString	text("A") ;
/*	// Print each page of output required
    LOGFONT lf;     memset(&lf, 0, sizeof(LOGFONT));   // Clear out structure.
    ::ZeroMemory(&lf, sizeof(LOGFONT));

	// you can adjust he font size by changing the value of 12
    lf.lfHeight = -MulDiv(12, pDC->GetDeviceCaps(LOGPIXELSY), 72);
    //_tcscpy(lf.lfFaceName, "Arial");    //    with face name "Arial".
    _tcscpy(lf.lfFaceName, "Times New Roman");    //    with face name "Times New Roman".
	//

    CFont	font;
	font.CreateFontIndirect(&lf) ;    // Create the font.

	pDC->SaveDC() ;
	pDC->SelectObject(&font) ;
*/

	text = "A" ;
	CSize	csText = pDC->GetTextExtent(text) ;	// get the height of a line of text

//	m_UserText.GetWindowText(text) ;			// get the text the user wants to print
	text.Empty();
	text += m_UserText;

	int		page = pInfo->m_nCurPage ;
	CString	line(_T("")) ;
	// move through the text to get to the 1st line of the page to print
	int	pos = 0 ;			// position in output text
	int	lines_per_page = (int)(m_rectText.Height() / csText.cy) ;
	int	cLine = lines_per_page ;
	while (page > 1)
		{
		// there are pages of text we need to skip
		while (cLine > 0)
			{
			line = ExtractLine(text, pos, m_rectText.Width(), pDC) ;
			--cLine ;			// count a line skipped
			}
		--page ;				// count a page of output skipped
		cLine = lines_per_page ;
		}
	// we are now at the right point in the string to start printing
	int	y = m_rectText.top +1000;
	while (pos < text.GetLength() && cLine > 0)
		{
		line = ExtractLine(text, pos, m_rectText.Width(), pDC) ;
		pDC->TextOut(m_rectText.left, y, line) ;
		y += csText.cy ;				// move down a line
		--cLine ;						// count a line printed
		}
	// done!
	//pDC->RestoreDC(-1) ;
    pDC->SelectObject(pOldFont);
}
