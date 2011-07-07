// PrintSchedule.h: interface for the CPrintSchedule class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PRINTSCHEDULE_H__3880F481_35CC_42E5_AC00_AA26BC5272A8__INCLUDED_)
#define AFX_PRINTSCHEDULE_H__3880F481_35CC_42E5_AC00_AA26BC5272A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MARGIN_LEFT     200
#define MARGIN_TOP      150
#define MARGIN_RIGHT    200
#define MARGIN_BOTTOM   150
#define HEADER_LINES   2
#define FOOTER_LINES   2
#define HEADER_LINE_SIZE  5
#define FOOTER_LINE_SIZE  5
#define CELL_PADDING 5
#define PAGE_COLUMNS 2

class CPrintSchedule  
{
public:
	CPrintSchedule();
	virtual ~CPrintSchedule();

// Operations
public:
	BOOL OnPreparePrinting(CPrintInfo* pInfo);
	void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	void OnPrint(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	void SetDocTitle(const CString& strName);
	void SetAppName(const CString& strName);

   //Should de deleted
	void SetListCtrl(CListCtrl* pList);

	//
    void SetListView(CListView* pList);
    void SetMarginLeftPix(int leftMargin){ m_marginLeft = leftMargin; }
    int GetMarginLeftPix(){ return m_marginLeft; }
    void SetMarginTopPix(int topMargin){ m_marginTop = topMargin; }
    int GetMarginTopPix(){ return m_marginTop; }
    void SetMarginRightPix(int rightMargin){ m_marginRight = rightMargin; }
    int GetMarginRightPix(){ return m_marginRight; }
    void SetMarginBottomPix(int bottomMargin){ m_marginBottom = bottomMargin; }
    int GetMarginBottomPix(){ return m_marginBottom; }
    void SetHeadLines(int lines){ m_headerLines = lines; }
    int GetHeadLines(){ return m_headerLines; }
    void SetFootLines(int lines){ m_footerLines = lines; }
    int GetFootLines(){ return m_footerLines; }
    int GetRowCount();

	//
    void GoPrintHeader(CDC* pDC, CPrintInfo* pInfo) {PrintHeader(pDC, pInfo);}
	void SetPrinttedData(CList<CString*, CString*>* p_cList) {m_pOutputtedDataList = p_cList;}

    CString ExtractLine(CString &source, int &pos, int width, CDC *pDC);
    int CountLinesInText(CString &text, int width, CDC *pDC);
    void OnBeginPrintingText(CDC* pDC, CPrintInfo* pInfo);
    void PrintText(CDC* pDC, CPrintInfo* pInfo);
    CString GetUserText() {return m_UserText;}
    void SetUserText(CString _UserText) {m_UserText = _UserText;}


protected:
#ifdef _DEBUG
   void DrawRect(CDC* pDC, CRect rect, COLORREF color);
#endif
   void DrawRow(CDC* pDC, int nRow);
	void PrintFooter(CDC* pDC, CPrintInfo* pInfo);
   void PrintBody(CDC* pDC, CPrintInfo* pInfo);
	void PrintHeader(CDC* pDC, CPrintInfo* pInfo);
   CFont* CreateFont(CDC* pDC, CString strName = _T(""), int nPoints = 0, int lfWeight = FW_NORMAL, bool lfItalic = false);
   double GetTextRatioX(CDC* pDC);
   CRect GetCellRect(int nRow, int nCol);
   CRect GetBodyRect();
   CRect GetPaperRect(CDC* pDC);
   CRect GetPageRect();
   CRect GetHeaderRect();
   CRect GetFooterRect();
   CRect GetTextRect();
   CSize GetCharSize(CDC* pDC, CFont* pFont);
   CRect GetColumnRect(int nCol);
   CString GetColumnHeading(int col);
   int GetColumnWidth(int col);
   int GetPageColumns();   
   int GetPageRows();
   //
   int GetColumnOrderArray(CArray<int, int>& cArray);

private:   
   CString m_strAppName;
   CString m_strDocName;
   CString m_UserText;

   //Should de deleted
   CListCtrl* m_pListCtrl;
   CListView* m_pListView;

   double m_RatioX; 
   CRect m_rectHeader;
   CRect m_rectFooter;
   CRect m_rectBody;
   CRect m_rectText;
   CRect m_rectPage;
	CRect m_rectPaper;
   CFont* m_pFontHeader;
   CFont* m_pFontFooter;
   CFont* m_pFontColumn;
   CFont* m_pFontBody;
   CSize m_CharSizeHeader;
   CSize m_CharSizeFooter;
   CSize m_CharSizeBody;
   int m_marginLeft;
   int m_marginTop;
   int m_marginRight;
   int m_marginBottom;
   int m_headerHeight;
   int m_headerLines;
   int m_footerHeight;
   int m_footerLines;
   int m_nPageCols;
   int m_nPageRows;
   int m_nPageCount;
   int m_nRowCount;
   //int m_nColumns[100]; 
   CArray<int, int> m_nColumns; 

   int m_TitleColumnWidth;
   int m_FieldColumnWidth;

   CList<CString*, CString*>* m_pOutputtedDataList;
   //CArray<int, int> m_cvColumns;
};

#endif // !defined(AFX_PRINTSCHEDULE_H__3880F481_35CC_42E5_AC00_AA26BC5272A8__INCLUDED_)
