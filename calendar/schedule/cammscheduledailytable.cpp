// CCAMMScheduleDailyTable.cpp : implementation file
//

#include "stdafx.h"
//#include "Table.h"
#include "CAMMScheduleDailyTable.h"
#include "CAMMScheduleEditorDlg.h"
#include "CAMMScheduleCalendar.h"
#include "CAMMScheduleDayView.h"
#include "..\loadstring.h"
#include "..\MainFrm.h"
#include "..\OpenrepeatQuestDlg.h"

//Test, Nono_2004_0407
//#include "..\CAMMAnimationDlg.h"

#define CCAMMScheduleDailyTable_CLASSNAME    _T("CCAMMScheduleDailyTable")  // Window class name
#define WM_FPSDATECTRL_NEWTEXT	WM_USER+1001

//Added by Nono=>
static UINT BASED_CODE indicators[] =
{
	/*
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_OVR,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM
	*/
	ID_SEPARATOR,          // status line indicator
	ID_INDICATOR_OVR,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM
};
#define Height_of_StatusBar 30
//Added by Nono<=

// CCAMMScheduleDailyTable

IMPLEMENT_DYNAMIC(CCAMMScheduleDailyTable, CWnd)


CCAMMScheduleDailyTable::CCAMMScheduleDailyTable()
{
	cells = NULL;
	xInit(-1,-1);

//	m_pPrintRecord = NULL;//Nono, 2004_0513
	m_pScheduleRecord = NULL;//nono, 2004_1028

	m_bShowWindow = true;//nono, 2004_1102
	m_bMobileConnected = false;
	m_bInitmemDC = FALSE;
	 m_brWhite.CreateSolidBrush(RGB(255,255,255));
	 m_bCreateMemDCSuc = FALSE;


}

CCAMMScheduleDailyTable::CCAMMScheduleDailyTable(int rows, int cols)
{
	m_bCreateMemDCSuc= FALSE;
	cells = NULL;
	xInit(rows,cols);

//	m_pPrintRecord = NULL;//Nono, 2004_0513

	m_pScheduleRecord = NULL;//nono, 2004_1029

	m_bShowWindow = true;//nono, 2004_1102

	m_bMobileConnected = false;

	m_bInitmemDC = FALSE;
	 m_brWhite.CreateSolidBrush(RGB(255,255,255));

}

CCAMMScheduleDailyTable::~CCAMMScheduleDailyTable()
{
	m_sMonthInfo = NULL;

	if(cells)
	{
		delete [] cells;

		cells = NULL;
	}

	if (rowHeight)
	{
		delete [] rowHeight;
		rowHeight = NULL;
	}

	if (colWidth)
	{
		delete [] colWidth;
		colWidth = NULL;
	}

	m_Menu.DestroyMenu();

//	if (m_pPrintRecord) delete m_pPrintRecord;//Nono, 2004_0513

	int recordCount=0;
	UninitMemDC();
	if(	 m_brWhite.GetSafeHandle())
		m_brWhite.DeleteObject();


/*
	m_s3MonthInfo.GetScheduleData(m_pScheduleRecord,recordCount);
	if (m_pScheduleRecord) {
		delete [] m_pScheduleRecord;
		m_pScheduleRecord = NULL;
	}
*/
}


// creates the control - use like any other window create control
BOOL CCAMMScheduleDailyTable::Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwStyle)
{
    ASSERT(pParentWnd->GetSafeHwnd());

    if (!CWnd::Create(CCAMMScheduleDailyTable_CLASSNAME, NULL, dwStyle, rect, pParentWnd, nID))
        return FALSE;

    return TRUE;
}

int CCAMMScheduleDailyTable::SetRows(int rows)
{
	if (rows <0 || rows >= MAX_ROWS1) return -1;
	
	if (this->rows == rows) return 0;

	if (this->rows < rows)
	{
		for (int i = this->rows; i < rows; i++)
		{
			for(int j = 0; j < this->cols; j++)
			{
				cells [i * MAX_COLS1 + j] = defaultCell;
				cells [i * MAX_COLS1 + j].SetSpan(1,1);				
			}

			rowHeight [i] = defaultHeight;
		}
	}
	else
	{
		for (int i = this->rows; i > rows; i--)
		{
			for (int j = 0; j < this->cols; j++)
			{
				cells [i * MAX_COLS1 + j].SetSpan(0,10);				
			}
		}
	}

	this->rows = rows;

	return 0;    
}

int CCAMMScheduleDailyTable::GetRows()
{
	return rows;
}

int CCAMMScheduleDailyTable::SetCols(int cols)
{
	if (cols < 0 || cols >= MAX_COLS1) return -1;

	if (this->cols == cols) return 0;

	if(this->cols < cols)
	{
		for (int i = this->cols; i < cols; i ++)
		{
			for (int j = 0; j < rows; j ++)
			{
				cells[j*MAX_COLS1+i] = defaultCell;
				cells[j*MAX_COLS1+i].SetSpan(1,1);
			}

			colWidth [i] = defaultwidth;
		}
	}
	else
	{
		for (int i = this->cols; i > cols; i --)
		{
			for (int j = 0; j < rows; j ++)
			{
				cells[j*MAX_COLS1+i].SetSpan(1,1);
			}
		}
	}
	
	this->cols = cols;

	return 0;
}

int CCAMMScheduleDailyTable::GetCols()
{
	return cols;
}

int CCAMMScheduleDailyTable::JoinCells (int startRow, int startCol, int endRow, int endCol)
{
	if (startRow > endRow || startCol > endCol) return -1;

	if (startCol < 0 || startCol >= cols) return -1;
	if (endCol < 0 || endCol >= cols) return -1;

	if (startRow < 0 || startRow >= rows) return -1;
	if (endRow < 0 || startRow >= rows) return -1;

	//	int rowSpan,colSpan;
	for (int i = startRow; i <= endRow; i++)
	{
		for (int j = startCol; j <=endCol;  j++)
		{
			//			cells [i * MAX_COLS1 + j].GetSpan(rowSpan,colSpan);
			//			if (rowSpan != 0) {
			//				UnjoinCells(i,j);
			//			}
			//				for (int k=i;k<i+rowSpan;k++) {
			//					for (int l=j;l<j+colSpan;l++) {
			//						cells [k * MAX_COLS1 + l].SetSpan(1,1);
			//					}
			//				}
			//			}
			cells [i * MAX_COLS1 + j].SetSpan(0,0);
		}
	}
	
	cells [startRow * MAX_COLS1 + startCol].SetSpan(endRow - startRow+1, endCol - startCol+1);
	
	return 0;
}

int CCAMMScheduleDailyTable::UnjoinCells (int row, int col)
{
	if (row < 0 || row >= this->rows) return -1;
	if (col < 0 || col >= this->cols) return -1;

	if (cells [row * MAX_COLS1 + col].rowSpan <= 1 && cells [row * MAX_COLS1 + col].colSpan <= 1 ) return -1;

	for (int i = row; i <= row + cells [row * MAX_COLS1 + col].rowSpan; i++)
	{
		for (int j = col; j <= col + cells [row * MAX_COLS1 + col].colSpan; j++)
		{
			cells[i*MAX_COLS1+j] = defaultCell;

			cells [i * MAX_COLS1 + j].SetSpan(1,1);
		}
	}

	return 0;
}

int CCAMMScheduleDailyTable::SetRowHeight(int row, int height)
{
	if ((row < 0) || (row >= GetRows()))
		return -1;
	if (rowHeight)
		rowHeight [row] = height;

	return 0;
}

int CCAMMScheduleDailyTable::GetRowHeight(int row)
{
	return rowHeight [row];
}

int CCAMMScheduleDailyTable::SetColWidth(int col, int width)
{
	if ((col < 0) || (col >= GetCols()))
		return -1;
	if (colWidth)
		colWidth [col] = width;

	return 0;
}

int CCAMMScheduleDailyTable::GetColWidth(int col)
{
	return colWidth [col];
}

int CCAMMScheduleDailyTable::GetRowsHeight(int startRow, int endRow)
{
	if (startRow < 0 || startRow >= rows) return 0;
	if (endRow < 0 || endRow >= rows) return 0;
	if (startRow > endRow) return 0;

	int height = 0;
	for (int i = startRow; i <= endRow; i++)
	{
		height += rowHeight[i];
	}

	return height;
}

int CCAMMScheduleDailyTable::GetColsWidth(int startCol, int endCol)
{
	if (startCol < 0 || startCol >= cols) return 0;
	if (endCol < 0 || endCol >= cols) return 0;
	if (startCol > endCol) return 0;

	int width = 0;
	for (int i = startCol; i <= endCol; i++)
	{
		width += colWidth[i];
	}

	return width;
}

int CCAMMScheduleDailyTable::SetCells(int row, int col, XCell& cell)
{
	cells[row * MAX_COLS1 + col] = cell;

	return 0;
}

XCell* CCAMMScheduleDailyTable::GetCells(int row, int col)
{
	return &cells [row * MAX_COLS1 + col];
}

int CCAMMScheduleDailyTable::SetText(int row, int col, CString str)
{

	XCell* cell = GetCells (row, col);
	if (!cell) return -1;

	return cell->SetText (str);
}

int CCAMMScheduleDailyTable::SetText(int row, int col,int idx, CString str)
{

	XCell* cell = GetCells (row, col);
	if (!cell) return -1;

	return cell->SetText (idx,str);
}

int CCAMMScheduleDailyTable::SetText(int row, int col, CString strText[48],int strCount)
{

	XCell* cell = GetCells (row, col);
	if (!cell) return -1;

	return cell->SetText (strText,strCount);
}

int CCAMMScheduleDailyTable::SetAlarm(int row, int col, BOOL bAlarm)
{

	XCell* cell = GetCells (row, col);
	if (!cell) return -1;

	return cell->SetAlarm (bAlarm);
}

int CCAMMScheduleDailyTable::SetAlarm(int row, int col,int idx, BOOL bAlarm)
{

	XCell* cell = GetCells (row, col);
	if (!cell) return -1;

	return cell->SetAlarm (idx,bAlarm);
}

int CCAMMScheduleDailyTable::SetAlarm(int row, int col, BOOL bAlarm[48],int count)
{

	XCell* cell = GetCells (row, col);
	if (!cell) return -1;

	return cell->SetAlarm (bAlarm,count);
}

int CCAMMScheduleDailyTable::SetRepeat(int row, int col, BOOL bRepeat)
{

	XCell* cell = GetCells (row, col);
	if (!cell) return -1;

	return cell->SetRepeat (bRepeat);
}

int CCAMMScheduleDailyTable::SetRepeat(int row, int col,int idx, BOOL bRepeat)
{

	XCell* cell = GetCells (row, col);
	if (!cell) return -1;

	return cell->SetRepeat (idx,bRepeat);
}

int CCAMMScheduleDailyTable::SetRepeat(int row, int col, BOOL bRepeat[48],int count)
{

	XCell* cell = GetCells (row, col);
	if (!cell) return -1;

	return cell->SetRepeat (bRepeat,count);
}
CString CCAMMScheduleDailyTable::GetText(int row, int col)
{
	return	cells[row * MAX_COLS1 + col].GetText();
}

CString CCAMMScheduleDailyTable::GetText(int row, int col,int idx)
{
	return	cells[row * MAX_COLS1 + col].GetText(idx);
}

int CCAMMScheduleDailyTable::SetTextColor(int row, int col, COLORREF color)
{
	cells[row * MAX_COLS1 + col].SetTextColor(color);

	return 0;
}

COLORREF CCAMMScheduleDailyTable::GetTextColor(int row, int col)
{
	return 	cells[row * MAX_COLS1 + col].GetTextColor();
}

int CCAMMScheduleDailyTable::SetTextFont(int row, int col, CFont& font)
{
	cells[row * MAX_COLS1 + col].SetTextFont(&font);

	return 0;

}
CFont* CCAMMScheduleDailyTable::GetTextFont(int row, int col)
{
	return	cells[row * MAX_COLS1 + col].GetTextFont();
}

int CCAMMScheduleDailyTable::SetTextFontSize(int row, int col, int size)
{
	cells[row * MAX_COLS1 + col].SetTextFontSize(size);

	return 0;
}

int CCAMMScheduleDailyTable::GetTextFontSize(int row, int col)
{
	return 	cells[row * MAX_COLS1 + col].GetTextFontSize();
}

int CCAMMScheduleDailyTable::SetOverlap (int row, int col, bool enable)
{
	XCell* cell = GetCells (row, col);
	if (!cell) return -1;

	return cell->SetOverlap (enable);
}

bool CCAMMScheduleDailyTable::GetOverlap (int row, int col)
{
	XCell* cell = GetCells (row, col);
	if (!cell) return false;

	return cell->GetOverlap ();
}

int CCAMMScheduleDailyTable::SetAlignment (int row, int col, int align)
{
	XCell* cell = GetCells (row, col);
	if (!cell) return -1;

	return cell->SetAlignment (align);
}

int CCAMMScheduleDailyTable::GetAlignment (int row, int col)
{
	XCell* cell = GetCells (row, col);
	if (!cell) return -1;

	return cell->GetAlignment ();
}

int CCAMMScheduleDailyTable::SetWordbreak (int row, int col, bool wordbreak)
{
	XCell* cell = GetCells (row, col);
	if (!cell) return -1;

	return cell->SetWordbreak (wordbreak);
}

bool CCAMMScheduleDailyTable::GetWordbreak (int row, int col)
{
	XCell* cell = GetCells (row, col);
	if (!cell) return false;

	return cell->GetWordbreak ();
}

int CCAMMScheduleDailyTable::SetBackColor(int row, int col, COLORREF color)
{
	cells[row * MAX_COLS1 + col].SetBackColor(color);

	return 0;
}
int CCAMMScheduleDailyTable::SetDrawUpLine(int row, int col, BOOL bDraw)
{
	cells[row * MAX_COLS1 + col].SetDrawUpLine(bDraw);

	return 0;
}
int CCAMMScheduleDailyTable::SetDrawDnLine(int row, int col, BOOL bDraw)
{
	cells[row * MAX_COLS1 + col].SetDrawDnLine(bDraw);

	return 0;
}
int CCAMMScheduleDailyTable::SetDrawLeftLine(int row, int col, BOOL bDraw)
{
	cells[row * MAX_COLS1 + col].SetDrawLeftLine(bDraw);

	return 0;
}
int CCAMMScheduleDailyTable::SetDrawRightLine(int row, int col, BOOL bDraw)
{
	cells[row * MAX_COLS1 + col].SetDrawRightLine(bDraw);

	return 0;
}

int CCAMMScheduleDailyTable::SetTextLine(int row, int col,BOOL bTextLine)
{
	cells[row * MAX_COLS1 + col].SetTextLine(bTextLine);
	cells[row * MAX_COLS1 + col].SetTextLineColor(RGB(220,220,220));

	return 0;
}

int CCAMMScheduleDailyTable::SetBorderColor(int row, int col, COLORREF color)
{
	cells[row * MAX_COLS1 + col].SetBorderColor(color);

	return 0;
}

COLORREF CCAMMScheduleDailyTable::GetBackColor(int row, int col)
{
	return 	cells[row * MAX_COLS1 + col].GetBackColor();
}

int CCAMMScheduleDailyTable::SetBackMode(int row, int col, int mode)
{
	cells[row * MAX_COLS1 + col].SetBackMode(mode);

	return 0;
}

int CCAMMScheduleDailyTable::GetBackMode(int row, int col)
{
	return 	cells[row * MAX_COLS1 + col].GetBackMode();
}

RECT CCAMMScheduleDailyTable::GetRect(int row, int col)
{
	RECT rect;
	int rowSpan = GetCells(row, col)->rowSpan;
	int colSpan = GetCells(row, col)->colSpan;
	rect.top = GetRowsHeight(0, row-1);
	rect.left = GetColsWidth(0, col-1);

	rect.bottom = rect.top + GetRowsHeight (row, row + rowSpan-1);
	rect.right = rect.left + GetColsWidth (col, col + colSpan-1);

	return rect;
}


BEGIN_MESSAGE_MAP(CCAMMScheduleDailyTable, CWnd)
	//{{AFX_MSG_MAP(CCAMMScheduleDailyTable)
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_ORG_SCH_DELETE, OnOrgSchDelete)
	ON_WM_MOUSEWHEEL()
//	ON_COMMAND(ID_ORG_SCH_EDIT, OnOrgSchEdit)
/*	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_DELETE, OnUpdateOrgSchDelete)
	ON_COMMAND(ID_ORG_SCH_DAILY, OnOrgSchDaily)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_DAILY, OnUpdateOrgSchDaily)
	ON_COMMAND(ID_ORG_SCH_WEEKLY, OnOrgSchWeekly)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_WEEKLY, OnUpdateOrgSchWeekly)
	ON_COMMAND(ID_ORG_SCH_MONTHLY, OnOrgSchMonthly)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_MONTHLY, OnUpdateOrgSchMonthly)
	ON_WM_SHOWWINDOW()
	ON_COMMAND(ID_ORG_SCH_ADD, OnOrgSchAdd)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_ADD, OnUpdateOrgSchAdd)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_EDIT, OnUpdateOrgSchEdit)
	ON_WM_MOVE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_ORG_SCH_CUT, OnOrgSchCut)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_CUT, OnUpdateOrgSchCut)
	ON_COMMAND(ID_ORG_SCH_COPY, OnOrgSchCopy)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_COPY, OnUpdateOrgSchCopy)
	ON_COMMAND(ID_ORG_SCH_PASTE, OnOrgSchPaste)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_PASTE, OnUpdateOrgSchPaste)*/
	//}}AFX_MSG_MAP
/*	ON_COMMAND(ID_ORG_SCH_PRINT, OnOrgPrnSchedule)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_PRINT, OnUpdateOrgPrnSchedule)
	ON_COMMAND(ID_ORG_SCH_PRINT_LIST, OnOrgPrnScheduleList)
	ON_UPDATE_COMMAND_UI(ID_ORG_SCH_PRINT_LIST, OnUpdateOrgPrnScheduleList)
*/
	ON_MESSAGE(WM_FPSDATECTRL_NEWTEXT, OnNewText)
	ON_MESSAGE(WM_INVALIDATEWND, OnInvalidate)
END_MESSAGE_MAP()


// CCAMMScheduleDailyTable message handlers
void CCAMMScheduleDailyTable::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CDC* pDC = &dc;

	Draw(pDC);
}

void CCAMMScheduleDailyTable::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
//	CPoint point;

//	RECT rect = GetRect(focusRow,focusCol);
//	point.x = rect.left;
//	point.y = rect.top;

	switch (nChar)
	{
		case VK_DOWN:
	//		point.y =rect.bottom + 1;
			break;

		case VK_UP:
	//		point.y =point.y -2;
			break;

		case VK_LEFT:
	//		point.x = point.x - 2;
			break;

		case VK_RIGHT:
	//		point.x =rect.right + 1;
			break;
		case VK_DELETE:
			OnOrgSchDelete();
			break;

		default:
			return ;
	}

	//	m_bHitTest = HitTest (point,focusRow,focusCol);
	//	xInvalidate(FALSE);
	
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CCAMMScheduleDailyTable::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bShowCalendar = FALSE;
	//Test, Nono_2004_0407
	m_bShowGotoToday = FALSE;
	int oldfocusRow = focusRow;
	int oldfocusCol = focusCol ;
	m_bHitTest = HitTest (point,focusRow,focusCol);
	BOOL bInvalidate = FALSE;
	if (m_bShowCalendar)
	{
		if(xShowCalendar(point,m_CurrentDate))
		{
			m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);
			ChangeDatePeriod(m_DatePeriod);
			xInvalidate(TRUE);
		}
		else
			xInvalidate(FALSE);

	//Added by Nono, 2004_0407=>
	}
	else if (m_bShowGotoToday)
	{
		CCAMMScheduleCalendar Calendardlg;
		COleDateTime currentDate;
		currentDate.SetDate(m_CurrentDate.GetYear(),m_CurrentDate.GetMonth(),m_CurrentDate.GetDay());

		Calendardlg.SetCurrentDate(m_CurrentDate);
		Calendardlg.Create(CCAMMScheduleCalendar::IDD/*IDD_ORG_SCHEDULE*/, NULL);
		Calendardlg.GetTodayInfo(m_CurrentDate);

		if(m_CurrentDate.GetYear() != currentDate.GetYear() ||
			m_CurrentDate.GetMonth() != currentDate.GetMonth() ||
			m_CurrentDate.GetDay() != currentDate.GetDay() )
		{
			m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);
			ChangeDatePeriod(m_DatePeriod);
		}
		else
			xInvalidate(FALSE);
    //Added by Nono, 2004_0407<=
	} 
	else 
	{
		if ((m_DatePeriod == DATEPERIOD_DAILY) && (focusRow < m_InfoRowCnt)) {
			return;
		}
		if (m_bGotoDaily)
			GotoDailyView();
		else
		{
			xInvalidatecell(oldfocusRow,oldfocusCol);
			xInvalidatecell(oldfocusRow+1,oldfocusCol);
			xInvalidatecell(focusRow,focusCol);
			//			SetFocus ();

		//	if (m_bPopupMenu) {
		//		xInvalidate(TRUE);
		//	} else {
		//		xInvalidate(FALSE);
		//	}
		}
	}
	m_bPopupMenu = FALSE;

	CWnd::OnLButtonDown(nFlags, point);
}


BOOL CCAMMScheduleDailyTable::RegisterWindowClass()
{
    WNDCLASS wndcls;
    HINSTANCE hInst = AfxGetInstanceHandle();

    if (!(::GetClassInfo(hInst, CCAMMScheduleDailyTable_CLASSNAME, &wndcls)))
    {
        // otherwise we need to register a new class
        wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wndcls.lpfnWndProc      = ::DefWindowProc;
        wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
        wndcls.hInstance        = hInst;
        wndcls.hIcon            = NULL;
        wndcls.hCursor          = afxGetApp()->LoadStandardCursor(IDC_ARROW);
        wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wndcls.lpszMenuName     = NULL;
        wndcls.lpszClassName    = CCAMMScheduleDailyTable_CLASSNAME;

        if (!AfxRegisterClass(&wndcls))
        {
            AfxThrowResourceException();
            return FALSE;
        }
    }

    return TRUE;
}

BOOLEAN CCAMMScheduleDailyTable::GetFocusCell(int &row, int &col)
{
	if (focusRow == -1)
		return FALSE;
	if (focusCol == -1)
		return FALSE;

	row = focusRow;
	col = focusCol;

	return TRUE;
}

void CCAMMScheduleDailyTable::SetDatePeriod(Schedule_DatePeriod sDp)
{
	m_DatePeriod = sDp;
}






//Daily ,Weekly , Monthly routing source code 
int CCAMMScheduleDailyTable::Draw(CDC* pDC) 
{

	if (!m_bShowWindow) {
		return 0;
	}
/*	switch (m_DatePeriod) {
	case DATEPERIOD_DAILY:
		nRet = Draw_Daily(pDC);
		break;
	case DATEPERIOD_WEEKLY:
		nRet = Draw_Weekly(pDC);
		break;
	case DATEPERIOD_MONTHLY:
		nRet = Draw_Monthly(pDC);
		break;
	}
	return 0;
*/
	if(!m_bInitmemDC)
		InitMemDC();
	if(m_bCreateMemDCSuc)
	{
		CDC *pMemeDC;
		CRect rect,rcClient,rectPaint;
		pDC->GetClipBox(&rect);
		GetClientRect(&rcClient);
		pMemeDC = &m_memDC;
		rectPaint.IntersectRect(&rect,&rcClient);
		pDC->BitBlt(rectPaint.left,rectPaint.top,rectPaint.Width(),rectPaint.Height(),pMemeDC,
			rectPaint.left,rectPaint.top,SRCCOPY);
	}
	else
	{
		int nRet = 0;
		switch (m_DatePeriod) {
		case DATEPERIOD_DAILY:
			nRet = Draw_Daily(pDC);
			break;
		case DATEPERIOD_WEEKLY:
			nRet = Draw_Weekly(pDC);
			break;
		case DATEPERIOD_MONTHLY:
			nRet = Draw_Monthly(pDC);
			break;
		}
		SetAllRedraw();
	}

	//SetAllRedraw();
	//SetAllUnDraw();
	m_bPopupMenu = FALSE;

	return 0;
}

int CCAMMScheduleDailyTable::CreateTable()
{
	int nRet;
	for (int i=0;i<GetRows();i++) {
		for (int j=0;j<GetCols();j++) {
			SetText(i,j,_T(""));
			UnjoinCells(i,j);
			GetCells(i,j)->Init();
		}
	}
	SetRows(0);
	SetCols(0);
	m_HitSameCell = FALSE;
	switch (m_DatePeriod) {
	case DATEPERIOD_DAILY:
//		XCell::SetTextLine(FALSE);
		nRet = CreateTable_Daily();
		xAssignDailyInfo();
		break;
	case DATEPERIOD_WEEKLY:
		m_nSelectedIndexInOneDay = -1;
//		XCell::SetTextLine(FALSE);
		nRet = CreateTable_Weekly();
		xAssignWeeklyInfo();
		break;
	case DATEPERIOD_MONTHLY:
		m_nSelectedIndexInOneDay = -1;
	//	XCell::SetTextLine(FALSE);
		nRet = CreateTable_Monthly();
		xAssignMonthlyInfo();
		break;
	}
	RECT rect;
	GetParent()->GetClientRect(&rect);
//	SetWindowPos(NULL,0,0,rect.right - rect.left-1,rect.bottom - rect.top-1,SWP_NOMOVE);
//	SetWindowPos(NULL,0,0,rect.right - rect.left,rect.bottom - rect.top,SWP_NOMOVE);
	xInvalidate(TRUE);

	//REFRESH LEFT_PANEL to change the totla number of calendars displayed on left view.
/*	if (m_bMobileConnected) {
		CCAMMSync sc;
    	g_numTotalCalendars = sc.GetCalendarCount();
        ::SendMessage(afxGetMainWnd()->GetSafeHwnd(), WM_REFRESH_LEFT_PANEL, (DWORD)true, NULL);
	}*/


	return nRet;
}

bool CCAMMScheduleDailyTable::HitTest (CPoint point, int& row, int& col)
{
	switch (m_DatePeriod) {
	case DATEPERIOD_DAILY:
		return HitTest_Daily(point,row,col);
		break;
	case DATEPERIOD_WEEKLY:
		return HitTest_Weekly(point,row,col);
		break;
	case DATEPERIOD_MONTHLY:
		return HitTest_Monthly(point,row,col);
		break;
	}
	return true;
}

void CCAMMScheduleDailyTable::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	switch (m_DatePeriod) {
	case DATEPERIOD_DAILY:
		OnVScroll_Daily(nSBCode,nPos,pScrollBar);
		break;
	case DATEPERIOD_WEEKLY:
		OnVScroll_Weekly(nSBCode,nPos,pScrollBar);
		break;
	case DATEPERIOD_MONTHLY:
		OnVScroll_Monthly(nSBCode,nPos,pScrollBar);
		break;
	}
}

void CCAMMScheduleDailyTable::OnSize(UINT nType, int cx, int cy) 
{
	m_HitSameCell = FALSE;
	switch (m_DatePeriod) {
	case DATEPERIOD_DAILY:
		OnSize_Daily(nType,cx,cy);
		break;
	case DATEPERIOD_WEEKLY:
		OnSize_Weekly(nType,cx,cy);
		break;
	case DATEPERIOD_MONTHLY:
		OnSize_Monthly(nType,cx,cy);
		break;
	}
	xInvalidate(TRUE);
}

void CCAMMScheduleDailyTable::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if (m_bMobileConnected == FALSE) 
	{
//		AfxMessageBox("Lose connecting to mobile!\nPlease check the connecting status of mobile. ");
		CalendarMessageBox(m_hWnd,_T("MSG_ERROR_DISCONNECT"));
		return;
	}
	if(((CMainFrame*)afxGetMainWnd())->m_bMobileGetFail== true)
		return ;

	switch (m_DatePeriod) {
	case DATEPERIOD_DAILY:
		OnLButtonDblClk_Daily(nFlags,point);
		break;
	case DATEPERIOD_WEEKLY:
		OnLButtonDblClk_Weekly(nFlags,point);
		break;
	case DATEPERIOD_MONTHLY:
		OnLButtonDblClk_Monthly(nFlags,point);
		break;
	}

}

//Daily source code
bool CCAMMScheduleDailyTable::HitTest_Daily (CPoint point, int& row, int& col)
{
	int InvisibleHeight = GetRowsHeight(0,m_StartRowForDisplay-1) - GetRowsHeight(0,m_InfoRowCnt-1);
	InvisibleHeight = InvisibleHeight == -1?0:InvisibleHeight;

	if (!m_bPopupMenu) {
		SetAllUnDraw();
	}
	if (focusRow >= m_InfoRowCnt) {
		m_bRedraw[focusRow - m_InfoRowCnt] = TRUE;
	}

	for (int i= 0; i < rows; i++) 
	{
		for(int j=0; j < cols; j++)
		{
			RECT rect = GetRect (i,j);
			if (i+j == 0) {
				if (rect.top <= point.y && rect.bottom > point.y && rect.left <= point.x && rect.right > point.x) {
					BITMAP bmpInfo;
					m_CalendarImage.GetBitmap(&bmpInfo);
					///////Test, Nono_2004_0407
					//int left = rect.left + ((rect.right - rect.left) - bmpInfo.bmWidth) / 2;
					//int right = left + bmpInfo.bmWidth;
					
					//int left = rect.right - bmpInfo.bmWidth;
					//int right = rect.right;
					int left = rect.right - ((rect.right - rect.left)/2 - bmpInfo.bmWidth) / 2 - bmpInfo.bmWidth;
					int right = left + bmpInfo.bmWidth;
                    ///////////////////////////////////////////////
					int top = rect.top + ((rect.bottom - rect.top) - bmpInfo.bmHeight) / 2;
					int bottom = top + bmpInfo.bmHeight;
					if ((point.x >= left) && (point.x <= right)){
						if ((point.y >= top) && (point.y <= bottom)){
							m_bShowCalendar = TRUE;
						}
					}

					///////Test, Nono_2004_0407
					m_GotoTodayImage.GetBitmap(&bmpInfo);
					//left = rect.left;
					//right = rect.left + bmpInfo.bmWidth;
					left = rect.left + ((rect.right - rect.left)/2 - bmpInfo.bmWidth) / 2;
					right = left + bmpInfo.bmWidth;
                    ///////////////////////////////////////////////
					top = rect.top + ((rect.bottom - rect.top) - bmpInfo.bmHeight) / 2;
					bottom = top + bmpInfo.bmHeight;
					if ((point.x >= left) && (point.x <= right)){
						if ((point.y >= top) && (point.y <= bottom)){
							m_bShowGotoToday = TRUE;
						}
					}
                    ///////////////////////////////////////////////

					return false;
				}
			}
			rect.top -= InvisibleHeight;
			rect.bottom -= InvisibleHeight;
			if (rect.top <= point.y && rect.bottom > point.y && rect.left <= point.x && rect.right > point.x)
			{
				//				if ((i >= m_StartRowForDisplay) && (i <= m_StartRowForDisplay + m_RowsPerPage)) {
				if ((i >= m_RealStartRow) && (i <= m_StartRowForDisplay + m_RowsPerPage)) {
					row = i;
					col = j;
					col = 2;
				} else {
					row = focusRow;
					col = focusCol;
				}

				if (row >= m_InfoRowCnt) {
					m_bRedraw[row - m_InfoRowCnt] = TRUE;
				}

				return true;
			}
		}
	}
	xInvalidate(TRUE);

	return false;
}

//int CCAMMScheduleDailyTable::Draw(CDC* pDC)
//{
//	for (int i = 0; i < rows; i++)
//	{
//		for (int j = 0; j < cols; j++)
//		{
//			XCell& cell = cells[i*MAX_COLS1+j];
//
//			if (cell.colSpan !=0 && cell.rowSpan != 0)
//			{
//				RECT rect = GetRect(i,j);
//
//				if (cell.GetOverlap())
//				{
//					RECT textRect = GetRect(i,j);
//
//					cell.CalcTextRect(pDC, &textRect);
//
//					if (textRect.right > rect.right)
//					{
//						for (j = j+1; j < cols; j ++)
//						{
//							if (cells[i*MAX_COLS1+j].text != _T(""))
//								break;
//							rect.right += colWidth [j];
//							if (rect.right > textRect.right)
//								break;
//						}
//						j --;
//					}
//
//				}
//
//				cell.Draw(pDC,rect);
//			}
//		}
//	}
//
//	RECT rect = GetRect (focusRow, focusCol);
//	GetCells (focusRow, focusCol)->DrawHitBorder(pDC, rect, RGB(255, 0, 20));
//
//	return 0;
//}
//


//int CCAMMScheduleDailyTable::Draw_Daily(CDC* pDC)
//{
//	CString hourStr,halfDayStr;
//	BOOLEAN bFirst30Min;
//	int i,j;
//	int InvisibleHeight = GetRowsHeight(0,m_StartRowForDisplay-1) - GetRowsHeight(0,m_InfoRowCnt-1);
//	InvisibleHeight = InvisibleHeight == -1?0:InvisibleHeight;
//	BOOL bRedraw = TRUE;
//
//	for (i = m_RealStartRow; i < m_StartRowForDisplay + m_RowsPerPage; i++)
//	{
//		if (!m_bRedraw[i-m_InfoRowCnt]) {
//			bRedraw = FALSE;
//			continue;
//		}
//		GetHourInfo(i,hourStr,halfDayStr,bFirst30Min);
//		if (bFirst30Min) {
//			SetText(i,1,halfDayStr);
//		}
//		if (i >= m_InfoRowCnt) {
//			CString title = m_sMonthInfo->GetInfo(i-m_InfoRowCnt).csTitle;
//			if (title.IsEmpty()) {
//				SetText(i,2,_T(""));
//			} else {
//				SetText(i,2,title);
//			}
//		}
//		for (j = 0; j < cols; j++)
//		{
//			XCell& cell = cells[i*MAX_COLS1+j];
//
//			if (cell.colSpan !=0 && cell.rowSpan != 0)
//			{
//				RECT rect = GetRect(i,j);
//				
//				if (cell.GetOverlap())
//				{
//					RECT textRect = GetRect(i,j);
//
//					cell.CalcTextRect(pDC, &textRect);
//
//					if (textRect.right > rect.right)
//					{
//						for (j = j+1; j < cols; j ++)
//						{
//							if (cells[i*MAX_COLS1+j].GetText() != _T(""))
//								break;
//							rect.right += colWidth [j];
//							if (rect.right > textRect.right)
//								break;
//						}
//						j --;
//					}
//
//				}
//				rect.top -= InvisibleHeight;
//				rect.bottom -= InvisibleHeight;
//				cell.Draw(pDC,rect);
//			}
//		}
//	}
//
//	//	if ((focusRow >= m_StartRowForDisplay) && (focusRow <= m_StartRowForDisplay + m_RowsPerPage)) {
//	if ((focusRow >= m_RealStartRow) && (focusRow <= m_StartRowForDisplay + m_RowsPerPage)) {
//		RECT rect = GetRect (focusRow, focusCol);
//		rect.top -= InvisibleHeight;
//		rect.bottom -= InvisibleHeight;
//		//		rect.bottom -= 5;
//		//		rect.right -= 18;
//		GetCells (focusRow, focusCol)->DrawHitBorder(pDC, rect, RGB(255, 0, 20));
//	}
//
//	if (!bRedraw) {
//		return 0;
//	}
//	for (i = 0; i < m_InfoRowCnt; i++)
//	{
//		for (j = 0; j < cols; j++)
//		{
//			XCell& cell = cells[i*MAX_COLS1+j];
//
//			if (cell.colSpan !=0 && cell.rowSpan != 0)
//			{
//				RECT rect = GetRect(i,j);
//				
//				if (cell.GetOverlap())
//				{
//					RECT textRect = GetRect(i,j);
//
//					cell.CalcTextRect(pDC, &textRect);
//
//					if (textRect.right > rect.right)
//					{
//						for (j = j+1; j < cols; j ++)
//						{
//							if (cells[i*MAX_COLS1+j].GetText() != _T(""))
//								break;
//							rect.right += colWidth [j];
//							if (rect.right > textRect.right)
//								break;
//						}
//						j --;
//					}
//
//				}
//				if (i+j == 0) {
//					BITMAP bitmap;
//					m_CalendarImage.GetBitmap(&bitmap);
//					cell.Draw(pDC,rect,ALIGN_CENTER,m_CalendarImage);
//				} else {
//					cell.Draw(pDC,rect);
//				}
//			}
//		}
//	}
//
//
//	return 0;
//}
int CCAMMScheduleDailyTable::Draw_Daily(CDC* pDC)
{
	CString hourStr,halfDayStr;
	int i,j;
	int InvisibleHeight = GetRowsHeight(0,m_StartRowForDisplay-1) - GetRowsHeight(0,m_InfoRowCnt-1);
	InvisibleHeight = InvisibleHeight == -1?0:InvisibleHeight;
	BOOL bRedraw = TRUE;


	for (i = m_RealStartRow; i < m_StartRowForDisplay + m_RowsPerPage; i++)
	{
		if (!m_bRedraw[i-m_InfoRowCnt]) {
			bRedraw = FALSE;
			continue;//	break;
		}
	}

	//Draw hour info and min info
	RECT bkRect;
	if (bRedraw)
	{
		XCell& cellHour = cells[m_InfoRowCnt*MAX_COLS1];
		bkRect = GetRect(m_InfoRowCnt,0);
		bkRect.right = bkRect.left + GetColsWidth(0,1);
		bkRect.bottom = bkRect.top + GetRowsHeight(m_InfoRowCnt,m_InfoRowCnt+m_RowsPerPage);
	//	bkRect.left += 1;
	//	bkRect.top += 1;
		cellHour.SetTextColor(RGB(1,33,72));
		xDraw_Background(pDC,cellHour,bkRect);
		cellHour.DrawBorder(pDC,bkRect);
	}
	for (i = m_RealStartRow; i < m_StartRowForDisplay + m_RowsPerPage; i++)
	{
///		if (!m_bRedraw[i-m_InfoRowCnt]) {
//			bRedraw = FALSE;
//			continue;
//		}
		for (j = 0; j < 2; j++)
		{
			DrawCell_Daily(pDC,i,j);
		}
	}

	if (bRedraw) {
		XCell& cellTitle = cells[m_InfoRowCnt*MAX_COLS1+2];
		bkRect = GetRect(m_InfoRowCnt,2);
		bkRect.right = bkRect.left + GetColWidth(2);
		bkRect.bottom = bkRect.top + GetRowsHeight(m_InfoRowCnt,m_InfoRowCnt+m_RowsPerPage);
	//	bkRect.left += 1;
	//	bkRect.top += 1;
		xDraw_Background(pDC,cellTitle,bkRect);
		cellTitle.DrawBorder(pDC,bkRect);
	}
	//Draw Info title
	for (i = m_RealStartRow; i < m_StartRowForDisplay + m_RowsPerPage; i++)
	{
		if (!m_bRedraw[i-m_InfoRowCnt]) {
			bRedraw = FALSE;
			continue;
		}
		XCell& cell = cells[i*MAX_COLS1+2];
		if (i >= m_InfoRowCnt) {
			CScheduleInfo si;
			m_sMonthInfo->GetScheduleInfo(i-m_InfoRowCnt,si);
			CString title = si.hiRealStartTime.FormatString();
			title += "-";
			title += si.hiRealStopTime.FormatString();
			title += " ";
			title += si.csTitle;
			if (si.csTitle.IsEmpty()) {
				SetText(i,2,_T(""));
			} else {
				SetText(i,2,title);
			}
			cell.SetAlarm(si.haveAlarm);
			cell.SetRepeat(si.haveRepeat);
		}

		if (cell.colSpan !=0 && cell.rowSpan != 0)
		{
			RECT rect = GetRect(i,2);	
			rect.top -= InvisibleHeight;
			rect.bottom -= InvisibleHeight;
			cell.Draw(pDC,rect);
		}
	}

	//Draw hit border
	//	if ((focusRow >= m_StartRowForDisplay) && (focusRow <= m_StartRowForDisplay + m_RowsPerPage)) {
/*	if ((focusRow >= m_RealStartRow) && (focusRow <= m_StartRowForDisplay + m_RowsPerPage)) {
		RECT rect = GetRect (focusRow, focusCol);
		rect.top -= InvisibleHeight;
		rect.bottom -= InvisibleHeight;
		//		rect.bottom -= 5;
		//		rect.right -= 18;
		GetCells (focusRow, focusCol)->DrawHitBorder(pDC, rect, RGB(255, 0, 20));
	}*/

	//	if (!bRedraw) {
	//		return 0;
	//	}

	//Draw Header
	if (bRedraw) {
		XCell& cellHeader = cells[0];
		bkRect = GetRect(0,0);
//		bkRect.left += 1;
//		bkRect.top += 1;
		bkRect.right = bkRect.left + GetColsWidth(0,2);
		bkRect.bottom = bkRect.top + GetRowsHeight(0,m_InfoRowCnt-1);
		xDraw_Background(pDC,cellHeader,bkRect);
	}
	for (i = 0; i < m_InfoRowCnt; i++)
	{
		for (j = 0; j < cols; j++)
		{
			XCell& cell = cells[i*MAX_COLS1+j];

			if (cell.colSpan !=0 && cell.rowSpan != 0)
			{
				RECT rect = GetRect(i,j);
				
				if (i+j == 0) {
					BITMAP bitmap;
					m_CalendarImage.GetBitmap(&bitmap);
					//cell.Draw(pDC,rect,ALIGN_CENTER,m_CalendarImage);
					//Test, Nono_2004_0407.
				//	cell.Draw(pDC,rect,ALIGN_RIGHT,m_CalendarImage);//Test, Nono_2004_0407.
					BITMAP bitmap2;
					m_GotoTodayImage.GetBitmap(&bitmap2);
					cell.Draw2Bitmap(pDC,rect, m_GotoTodayImage, m_CalendarImage);
				} else {
					cell.Draw(pDC,rect);
				}
			}
		}
	}
	DrawCell_Daily(pDC,focusRow, focusCol);
	return 0;
}

//dwm different
int CCAMMScheduleDailyTable::CreateTable_Daily()
{
	m_bGotoDaily = FALSE;


	SetInfoRowCnt(2);
	int startRow = m_InfoRowCnt;  //At pos of start at 0
	//	int endRow = 48 + startRow - 1;


	m_DailyDataRowCnt=0;
	m_sMonthInfo->GetCurrentDayInfo().GetRowCount(m_DailyDataRowCnt);
	int endRow = m_DailyDataRowCnt + startRow - 1;


	defaultHeight = 20;
	defaultwidth = 80;
	SetRows (endRow + 1);
	SetCols (3);
	xSetScrollRange(SB_VERT,0,GetRows(),TRUE);

	for (int i = m_InfoRowCnt; i < GetRows(); i ++) {
		SetBackColor (i, 0, RGB(187,199,255));
		SetBackColor (i, 1, RGB(187,199,255));
		SetRowHeight(i,20);
		SetBackColor(i,2,RGB(223,227,239));
		SetBorderColor(i,0,RGB(68,68,153));
		SetBorderColor(i,1,RGB(68,68,153));
		SetBorderColor(i,2,RGB(68,68,153));
	}
	for (i = 0; i < m_InfoRowCnt; i ++) 
	{
		SetBorderColor(i,0,RGB(68,68,153));
		SetBorderColor(i,1,RGB(68,68,153));
		SetBorderColor(i,2,RGB(68,68,153));
	}

	SetColWidth(0,40);
	SetColWidth (1, 30);
	RECT rect1;
	GetClientRect(&rect1);
	SetColWidth (2, rect1.right-rect1.left - GetColsWidth(0,1));

	//left-top corner
	JoinCells(0,0,m_InfoRowCnt-1,1);
	SetBackColor(0,0,RGB(187,199,255));

//	JoinCells (0,2,0,2); //Hour
	JoinCells (0,2,1,m_InfoRowCnt); //Hour
	COleDateTimeEx date = m_CurrentDate;
	CString strDate = date.Format(VAR_DATEVALUEONLY);
	//	FindScheduleDaily(strDate);

	SetText(0,2,strDate);
	SetAlignment(0,2,ALIGN_CENTER | ALIGN_TOP);
	//	SetTextFontSize(0,2,12);
	SetBackColor (0, 2, RGB(187,199,255));

	//JoinCells(1,2,1,2);
	SetBackColor(1,2,RGB(187,199,255));

	CString hourStr,halfDayStr;
	BOOLEAN bFirst30Min;
	int infoCntFirstHalfHour=0;
	int infoCntSecondHalfHour=0;
	int rowIdx=m_InfoRowCnt;
	int nOldRowIndex = m_InfoRowCnt;
	//	for (int row=startRow;row<=endRow;row++) {
	for (int row=m_InfoRowCnt;row<48+m_InfoRowCnt;row++) 
	{
		nOldRowIndex = row;
		GetHourInfo(rowIdx,hourStr,halfDayStr,bFirst30Min);
		if (bFirst30Min) {
		    infoCntFirstHalfHour = m_sMonthInfo->GetInfo(row-m_InfoRowCnt).GetRowCount() + 1;
		    infoCntSecondHalfHour = m_sMonthInfo->GetInfo(row-m_InfoRowCnt+1).GetRowCount() + 1;
	//		JoinCells (rowIdx,0,rowIdx+infoCntFirstHalfHour+infoCntSecondHalfHour-1,1); //Hour
			JoinCells (rowIdx,0,rowIdx+infoCntFirstHalfHour+infoCntSecondHalfHour-1,0); //Hour
			SetDrawRightLine(rowIdx,0,FALSE);

			SetText(rowIdx,0,hourStr);
			SetTextFontSize(rowIdx,0,32);
	//		SetAlignment(rowIdx,0,ALIGN_RIGHT | ALIGN_TOP);
			SetAlignment(rowIdx,0,ALIGN_LEFT | ALIGN_TOP);
			//		SetBackColor(row,0,RGB(0xC8, 0x96, 0x96));

			JoinCells(rowIdx,1,rowIdx+infoCntFirstHalfHour-1,1);
			SetDrawLeftLine(rowIdx,1,FALSE);
			SetDrawDnLine(rowIdx,1,FALSE);

			SetText(rowIdx,1,halfDayStr);
			//			SetTextFontSize(row,1,12);
			//		SetBackColor(row,0,RGB(0xC8, 0x96, 0x96));

			for (int i=rowIdx;i<rowIdx+infoCntFirstHalfHour;i++) {
				JoinCells(i,2,i,2);
				SetText(i,2,_T(""));
				SetBackColor(i,2,RGB(223,227,239));
			}
			rowIdx += infoCntFirstHalfHour;
		} else {
		    infoCntSecondHalfHour = m_sMonthInfo->GetInfo(row-m_InfoRowCnt).GetRowCount() + 1;
			JoinCells(rowIdx,1,rowIdx+infoCntSecondHalfHour-1,1);
			SetDrawLeftLine(rowIdx,1,FALSE);
			SetDrawUpLine(rowIdx,1,FALSE);

			SetText(rowIdx,1,_T(""));

			for (int i=rowIdx;i<rowIdx+infoCntSecondHalfHour;i++) {
				JoinCells(i,2,i,2);
				SetText(i,2,_T(""));
				SetBackColor(i,2,RGB(223,227,239));
			}
			rowIdx += infoCntSecondHalfHour;
		}
	//		JoinCells(nOldRowIndex,0,nOldRowIndex+infoCntSecondHalfHour+infoCntFirstHalfHour,1);

	}

	//	//Clear memo info in col 2
	//	for (i=m_InfoRowCnt;i<GetRows();i++) {
	//		JoinCells(i,2,i,2);
	//		SetText(i,2,_T(""));
	//		SetBackColor(i,2,RGB(0xCC,0xCC,0xCC));
	//		//					SetBorderColor(i,2,RGB(0x00,0x00,0x00));
	//	}

	//	int rc=0;
	int nData = 0;
	int nPrevData = 0;
	for (i=m_InfoRowCnt;i<GetRows();i++) 
	{
		//		if (!m_sMonthInfo->GetInfo(i-m_InfoRowCnt).IsEmpty()) {
		//			for (int j=0;j<max(m_sMonthInfo->GetInfo(i-m_InfoRowCnt).GetCount(),1);j++) {
		//				rc++;
				CScheduleInfo info;
				GetScheduleInfo(i-m_InfoRowCnt,info);

				nData = SetMemoCellInfo(i,info);
				
				if(nPrevData && nData)
					SetDrawUpLine(i,2,FALSE);
				else
				{
					if(nPrevData && !nData)
						SetTextLine(i-1,2,FALSE);

					SetDrawUpLine(i,2,TRUE);
				}
				if(nData )
					SetDrawDnLine(i,2,FALSE);
				else
					SetDrawDnLine(i,2,TRUE);
				nPrevData = nData;
				//			}
			//		}
	}

	// setup styles and class info
	//	DWORD dwExStyle = 0;
	//	DWORD dwStyle = WS_BORDER;// | ~WS_VISIBLE;//WS_CHILD | WS_EX_CLIENTEDGE;
/*1	LPCTSTR szWndCls = AfxRegisterWndClass(CS_BYTEALIGNCLIENT | CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS,
										   0,0,0);
*/
	//	RECT rect = {0,0,GetColsWidth(0,1),GetRowsHeight(0,m_InfoRowCnt -1)};

	focusRow = -1;
	focusCol = -1;

	//	SetAllRedraw();
	return 0;
}

//dwm different
void CCAMMScheduleDailyTable::OnVScroll_Daily(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	//SB_BOTTOM   Scroll to bottom. 
	//SB_ENDSCROLL   End scroll. 
	//SB_LINEDOWN   Scroll one line down. 
	//SB_LINEUP   Scroll one line up. 
	//SB_PAGEDOWN   Scroll one page down. 
	//SB_PAGEUP   Scroll one page up. 
	//SB_THUMBPOSITION   Scroll to the absolute position. The current position is provided in nPos. 
	//SB_THUMBTRACK   Drag scroll box to specified position. The current position is provided in nPos. 
	//SB_TOP   Scroll to top.
	SCROLLINFO si;
	GetScrollInfo(SB_VERT,&si,SIF_ALL);

	int delta = 0;
	BOOLEAN bInvalidate = TRUE;

	switch (nSBCode) {
	case SB_BOTTOM:
		delta = si.nMax - m_ScrollBarPos;
		break;
	case SB_ENDSCROLL:
		UpdateWindow();
		bInvalidate = FALSE;
		break;
	case SB_LINEDOWN:
		if (m_ScrollBarPos < m_InfoRowCnt)
			delta = 1 - (m_ScrollBarPos - m_InfoRowCnt);
		else 
			delta = 1;
		if (m_ScrollBarPos > m_MaxRowForDisplay)
			bInvalidate = FALSE;
		break;
	case SB_LINEUP:
		if (m_ScrollBarPos > m_MaxRowForDisplay)
			delta = -1 - (m_ScrollBarPos - m_MaxRowForDisplay)-m_InfoRowCnt;
		else 
			delta = -1;

		if (m_ScrollBarPos == 0)
			bInvalidate = FALSE;
		break;
	case SB_PAGEDOWN:
		delta = m_RowsPerPage;
		if (m_ScrollBarPos > m_MaxRowForDisplay)
			bInvalidate = FALSE;
		break;
	case SB_PAGEUP:
		if (m_ScrollBarPos > m_MaxRowForDisplay)
			delta = -m_RowsPerPage - (m_ScrollBarPos - m_MaxRowForDisplay)-m_InfoRowCnt;
		else 
			delta = -m_RowsPerPage;

		if (m_ScrollBarPos == 0)
			bInvalidate = FALSE;

		break;
	case SB_THUMBPOSITION:
		//		delta = nPos - m_ScrollBarPos;
		break;
	case SB_THUMBTRACK:
			delta = nPos - m_ScrollBarPos;
		break;
	case SB_TOP:
		delta = si.nMin - m_ScrollBarPos;
		break;
	}

	m_ScrollBarPos += delta;
	m_StartRowForDisplay = m_ScrollBarPos +m_InfoRowCnt ;
	if (m_ScrollBarPos <= si.nMin) {
		m_ScrollBarPos = si.nMin;
		m_StartRowForDisplay = m_InfoRowCnt;
	}
	if (m_ScrollBarPos <= m_InfoRowCnt) {
		m_ScrollBarPos = si.nMin;
		m_StartRowForDisplay = m_InfoRowCnt;
	}
	if (m_ScrollBarPos >= m_MaxRowForDisplay- m_InfoRowCnt) {
		m_StartRowForDisplay = m_MaxRowForDisplay ;
		m_ScrollBarPos= si.nMax;
	}
	if (m_ScrollBarPos >= si.nMax) {
		m_ScrollBarPos = si.nMax;
	}
	m_RealStartRow = m_StartRowForDisplay > m_InfoRowCnt ? m_StartRowForDisplay : m_InfoRowCnt;

	//	if (m_sMonthInfo->GetCurrentDayInfo().bDirtyFlag[m_RealStartRow-m_InfoRowCnt]) {
	//		//		if (!(m_sMonthInfo->GetInfo(m_RealStartRow - m_InfoRowCnt).bScheduleStart)) {
	//			for (int i=m_RealStartRow-m_InfoRowCnt;i>=0;i--) {
	//				if (!(m_sMonthInfo->GetCurrentDayInfo().bDirtyFlag[i])) {
	//					m_RealStartRow = i + 1;
	//					break;
	//				}
	//			}
	//			//		}
	//	}
	SetScrollPos(SB_VERT,m_ScrollBarPos,TRUE);
	if (bInvalidate && (m_StartRowForDisplay == m_MaxRowForDisplay)) {
		RECT rect;
		GetClientRect(&rect);
		rect.top = rect.bottom - GetRowHeight(m_StartRowForDisplay + m_RowsPerPage -1);
//		InvalidateRect(&rect,TRUE);
	} 
	if (bInvalidate) {
		xInvalidate(TRUE);
		//UpdateWindow();
	}

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

//dwm different
void CCAMMScheduleDailyTable::OnSize_Daily(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	xAssignDailyInfo();
}

//dwm different
void CCAMMScheduleDailyTable::OnLButtonDblClk_Daily(UINT nFlags, CPoint point) 
{
	//	GetParent()->SendMessage(WM_LBUTTONDBLCLK,(WPARAM)nFlags,MAKELPARAM(point.x,point.y));

	int row,col;
	BOOL bSameDate = TRUE;
	if (GetFocusCell(row,col)) 
	{
		CScheduleInfo si,si2;
		//得到信息，放到si中
		GetScheduleInfo(row - m_InfoRowCnt,si);
		
		if (si.csTitle.IsEmpty()) {
			if (xCheckRecordMaxLimit(1)) {
				return;
			}
		}
		if (xShowEditorDlg(si,si2,bSameDate)) 
		{
			if(si.hiStartTime !=si2.hiStartTime)
				SetScrollBarPosition(si2.rowIndex);
			CreateTable();

			//			SetText(si.rowIndex+m_InfoRowCnt,2,_T(""));
			//			for (int i=si.rowIndex + m_InfoRowCnt;i<si.rowIndex + m_InfoRowCnt+si.n30MinBlockCnt;i++) {
			//				JoinCells(i,2,i,2);
			//				SetBackColor(i,2,RGB(0xCC,0xCC,0xCC));
			//				//					SetBorderColor(i,2,RGB(0x00,0x00,0x00));
			//			}
			//			
			//			if (bSameDate) {
			//				CString title = si2.csTitle;
			//				SetText(si2.rowIndex + m_InfoRowCnt,2,title);
			//				JoinCells(si2.rowIndex + m_InfoRowCnt,2,si2.rowIndex + m_InfoRowCnt,2);
			//				SetBackColor(si2.rowIndex + m_InfoRowCnt,2,RGB(0xFF,0xFF,0xFF));
			//			} 
			xInvalidate(TRUE);
		}
		else
			xInvalidate(FALSE);
	}
	
	CWnd::OnLButtonDblClk(nFlags, point);
}

//dwm different
void CCAMMScheduleDailyTable::xSetScrollRange(int nBar, int nMinPos, int nMaxPos, BOOL bRedraw)
{
	SetScrollRange(nBar,nMinPos,nMaxPos,bRedraw);
	m_ScrollBarPos = m_StartRowForDisplay - m_InfoRowCnt;
	if (m_ScrollBarPos < nMinPos)
		m_ScrollBarPos = nMinPos;
	if (m_ScrollBarPos > nMaxPos)
		m_ScrollBarPos = nMaxPos;

	m_MaxRowForDisplay = GetRows() - m_RowsPerPage + 1;

	if (m_StartRowForDisplay < m_InfoRowCnt)
		m_StartRowForDisplay = m_InfoRowCnt;
	if (m_StartRowForDisplay > m_MaxRowForDisplay)
		m_StartRowForDisplay = m_MaxRowForDisplay;

	SetScrollPos(nBar,m_ScrollBarPos,bRedraw);
}

//dwm different
void CCAMMScheduleDailyTable::SetInfoRowCnt(int infoRowCnt)
{
	m_InfoRowCnt = infoRowCnt;
	//	m_StartRowForDisplay = m_InfoRowCnt;
	//	m_RowsPerPage = 25;
	//	m_RealStartRow = m_StartRowForDisplay;
}

//dwm different
void CCAMMScheduleDailyTable::SetTableTitle(CString csTitle)
{
	m_csTableTitle = csTitle;	
}


//dwm different
BOOLEAN CCAMMScheduleDailyTable::GetHourInfo(int rowIndex, CString& hourStr, CString& halfDayStr, BOOLEAN &bFirst30Min)
{
	CScheduleInfo si;
	m_sMonthInfo->GetScheduleInfo(rowIndex - m_InfoRowCnt,si);
	hourStr.Format(_T("%d"),si.hiStartTime.hour);
	if (hourStr.GetLength() == 1) {
		hourStr = "0" + hourStr;
	}
	if (si.hiStartTime.hour == 12) {
		if (si.hiStartTime.halfDay == HALFDAY_AM) 
			halfDayStr = m_MorningString;
		else 
			halfDayStr = m_AfternoonString;
	} else {
		if ((rowIndex - m_StartRowForDisplay) ==0  ||(rowIndex - m_StartRowForDisplay) ==1 /*<= 1*/) {
			if (si.hiStartTime.halfDay == HALFDAY_AM) 
				halfDayStr = m_MorningString;
			else 
				halfDayStr = m_AfternoonString;
		} else {
			halfDayStr = "00";
		}
	}

	if (si.hiStartTime.min <30) 
		bFirst30Min = TRUE;
	else 
		bFirst30Min = FALSE;

	return TRUE;
}


//dwm different
void CCAMMScheduleDailyTable::SetHalfDayString(CString morning, CString afternoon)
{
	m_MorningString = morning;
	m_AfternoonString = afternoon;
}


//dwm different
int CCAMMScheduleDailyTable::SetMemoCellInfo(int row, CScheduleInfo sd)
{
	int nRet = -1;
	if (row < m_InfoRowCnt) 
		return nRet;

	if (row > GetRows() - 1)
		return nRet;

	//	JoinCells(row,2,min(GetRows()-1,row+sd.n30MinBlockCnt-1),2);
	JoinCells(row,2,row,2);
	CString title = sd.csTitle;
	SetText(row,2,title);
	if (title.IsEmpty()) {
		SetBackColor(row,2,RGB(223,227,239));
		SetTextLine(row,2,FALSE);
		nRet = 0;

	} else 
	{
		SetBackColor(row,2,RGB(0xFF,0xFF,0xFF));
		SetTextLine(row,2,TRUE);
		nRet = 1;


	}
	SetAlignment(row,2,ALIGN_LEFT | ALIGN_TOP);// | ALIGN_MIDDLE);
	return nRet;
}

void CCAMMScheduleDailyTable::GetRepeatCurrentDateInfo(CScheduleInfo &si,COleDateTimeEx dtNewStopDate)
{
	COleDateTime oleStartData,oleStopData,oleAlarmtime,oleStartDateTime,oleNewStartDateTime,oleCurrentDate;
	int nHour=0, nMin=0;
	si.hiStartTime.Get24HourTime(nHour, nMin);
	oleStartData.SetDateTime(si.dtStartDate.GetYear(),si.dtStartDate.GetMonth(),si.dtStartDate.GetDay(),0,0,0);
	oleStopData.SetDateTime(si.dtStopDate.GetYear(),si.dtStopDate.GetMonth(),si.dtStopDate.GetDay(),0,0,0);
	oleStartDateTime.SetDateTime(si.dtStartDate.GetYear(),si.dtStartDate.GetMonth(),si.dtStartDate.GetDay(),nHour,nMin,0);
	oleCurrentDate.SetDateTime(m_CurrentDate.GetYear(),m_CurrentDate.GetMonth(),m_CurrentDate.GetDay(),0,0,0);

	if(si.dtStopDate.GetYear() != dtNewStopDate.GetYear() ||
		si.dtStopDate.GetMonth() != dtNewStopDate.GetMonth() ||
		si.dtStopDate.GetDay() != dtNewStopDate.GetDay())
	{
		si.originalData = FALSE;
	}
	else
		si.originalData = TRUE;

	COleDateTime oleAlarmspain;
	BOOL bAlarmBeforeStart = TRUE;
	oleAlarmspain.m_dt = 0;
	if(si.haveAlarm)
	{
		oleAlarmtime.SetDateTime(si.dtAlarmDateTime.GetYear(),si.dtAlarmDateTime.GetMonth(),si.dtAlarmDateTime.GetDay(),
			si.dtAlarmDateTime.GetHour(),si.dtAlarmDateTime.GetMinute(),si.dtAlarmDateTime.GetSecond());

		if(oleStartDateTime > oleAlarmtime)
		{
			oleAlarmspain = oleStartDateTime - oleAlarmtime ;
			bAlarmBeforeStart = TRUE;
		}
		else
		{
			oleAlarmspain = oleAlarmtime - oleStartDateTime;
			bAlarmBeforeStart = FALSE;
		}


	}
	COleDateTime  oleDayspain = oleStopData - oleStartData;

	if(oleDayspain.m_dt != 0)
	{
	//	if(oleCurrentDate > dtNewStopDate)
		{
			si.dtStopDate.SetDateTime(dtNewStopDate.GetYear(),dtNewStopDate.GetMonth(),dtNewStopDate.GetDay(),0,0,0);
			oleStartData = si.dtStopDate - oleDayspain;
			si.dtStartDate.SetDateTime(oleStartData.GetYear(),oleStartData.GetMonth(),oleStartData.GetDay(),0,0,0);
		}

	}
	else
	{
		si.dtStartDate.SetDateTime(m_CurrentDate.GetYear(),m_CurrentDate.GetMonth(),m_CurrentDate.GetDay(),0,0,0);
		si.dtStopDate = si.dtStartDate;
	}

	oleNewStartDateTime.SetDateTime(si.dtStartDate.GetYear(),si.dtStartDate.GetMonth(),si.dtStartDate.GetDay(),nHour,nMin,0);

	if(si.haveAlarm)
	{
		if(oleAlarmspain.m_dt >=0 )
		{
			if(bAlarmBeforeStart)
				oleAlarmtime = oleNewStartDateTime - oleAlarmspain;
			else
				oleAlarmtime = oleNewStartDateTime + oleAlarmspain;

			si.dtAlarmDateTime = oleAlarmtime;
		}
		else
			si.dtAlarmDateTime = oleNewStartDateTime;
	}

}
//dwm different
int CCAMMScheduleDailyTable::GetScheduleInfo(int row, CScheduleInfo &si)
{
	//	if (!m_pScheduleDaily)
	//		return -1;

	//	sd = m_pScheduleDaily[row - m_InfoRowCnt];
	m_sMonthInfo->GetScheduleInfo(row,si);
	if (/*si.haveRepeat||*/ !si.bEndDateSameToStartDate) 
	{
		COleDateTimeEx oleStopDate = si.dtStopDate;
		m_s3MonthInfo.GetRepeatOriginalInfo(si);
		GetRepeatCurrentDateInfo(si,oleStopDate);
	}

	return 0;
}

int CCAMMScheduleDailyTable::SetScheduleInfo(CScheduleInfo& si)
{
	//	m_sMonthInfo->SetInfo(si);

	return 0;
}
int CCAMMScheduleDailyTable::AddScheduleInfo(CScheduleInfo& si)
{

	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(si.dtStartDate);
	m_sMonthInfo->SetScheduleDate(si.dtStartDate);
//peggy
//	BOOL oldOriginalData = si.originalData;
//	si.originalData = TRUE;
	
	m_sMonthInfo->AddScheduleInfoBeforeSame(si);
	if(si.haveRepeat || si.bEndDateSameToStartDate == FALSE)
	m_s3MonthInfo.UpdateScheduleData();
//	si.originalData = oldOriginalData;

	m_CurrentDate = si.dtStartDate;
	m_dtOldMonthfocusDay = m_CurrentDate;
	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);
	m_sMonthInfo->SetScheduleDate(m_CurrentDate);

	return 0;
}
void CCAMMScheduleDailyTable::UpdateScheduleData()
{
	m_s3MonthInfo.UpdateScheduleData();
}
void CCAMMScheduleDailyTable::RemoveScheduleInfo(CScheduleInfo& si,bool bNeddUpdate)
 {
////
	CScheduleInfo original = si;
	if (si.haveRepeat|| !si.bEndDateSameToStartDate)
	{
		if (si.originalData == FALSE)
		{
			m_s3MonthInfo.GetRepeatOriginalInfo(original);
		}
	}


///
	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(original.dtStartDate);
	m_sMonthInfo->SetScheduleDate(original.dtStartDate);

	//remove originalData
	//	si.nRepeatId = -1; 
//	BOOL oldOriginalData = si.originalData;
	
//	si.originalData = TRUE;
	m_sMonthInfo->RemoveScheduleInfo(original);
	m_s3MonthInfo.RemoveScheduleInfo(original);
	if(bNeddUpdate && (si.haveRepeat || si.bEndDateSameToStartDate == FALSE))
	m_s3MonthInfo.UpdateScheduleData();
//	si.originalData = oldOriginalData;
	
	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);
	m_sMonthInfo->SetScheduleDate(m_CurrentDate);
}

//dwm different
LRESULT CCAMMScheduleDailyTable::OnNewText(WPARAM wParam, LPARAM)
{
	CString* pString = (CString*)wParam;

	if (pString->GetLength() > 0)
	{
		CString csStr = *pString;
		//		SetText(0,2,csStr);
		COleDateTimeEx dTime;
		dTime.ParseDateTime(*pString,VAR_DATEVALUEONLY);
		if (dTime.GetStatus() != COleDateTime::valid) {
			return 0;
		}
		m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(dTime);
		m_CurrentDate = dTime;
		m_dtOldMonthfocusDay = m_CurrentDate;


		CreateTable();
		//		//Clear memo info in col 2
		//		for (int i=m_InfoRowCnt;i<GetRows();i++) {
		//			JoinCells(i,2,i,2);
		//			SetBackColor(i,2,RGB(0xCC,0xCC,0xCC));
		//			//					SetBorderColor(i,2,RGB(0x00,0x00,0x00));
		//		}
		//
		//		for (i=m_InfoRowCnt;i<GetRows();i++) {
		//			SetText(i,2,m_sMonthInfo->GetInfo(i - m_InfoRowCnt).csTitle);
		//			if (m_sMonthInfo->GetInfo(i-m_InfoRowCnt).bScheduleStart) {
		//				JoinCells(i,2,i+m_sMonthInfo->GetInfo(i-m_InfoRowCnt).n30MinBlockCnt-1,2);
		//				SetBackColor(i,2,RGB(0xFF,0xFF,0xFF));
		//			}
		//		}

		delete pString;
		//xInvalidate(TRUE);
	}

	return 0;
}

//Weekly source code
bool CCAMMScheduleDailyTable::HitTest_Weekly (CPoint point, int& row, int& col)
{
	if (!m_bPopupMenu) {
		SetAllUnDraw();
	}
	m_bRedraw[xGetWeeklyIndex(focusRow,focusCol)] = TRUE;
	m_HitSameCell = FALSE;

	for (int i= 0; i < rows; i++) 
	{
		for(int j=0; j < cols; j++)
		{
			RECT rect = GetRect (i,j);
			if (rect.top <= point.y && rect.bottom > point.y && rect.left <= point.x && rect.right > point.x)
			{
				if (i+j == 0) {
					BITMAP bmpInfo;
					m_CalendarImage.GetBitmap(&bmpInfo);
					int left = rect.left;
					int right = left + bmpInfo.bmWidth;
					int top = rect.top + ((rect.bottom - rect.top) - bmpInfo.bmHeight) / 2;
					int bottom = top + bmpInfo.bmHeight;
					if ((point.x >= left) && (point.x <= right)){
						if ((point.y >= top) && (point.y <= bottom)){
							m_bShowCalendar = TRUE;
							return false;
						}
					}
				} 

				row = i;
				col = j;

				if ((row % 2) == 0) {
					row++;
					m_nSelectedIndexInOneDay = -1;
				} else {
					if (m_bMore[xGetWeeklyIndex(row,col)]) {
						BITMAP bitmap;
						m_Image.GetBitmap(&bitmap);
						if ((rect.bottom - bitmap.bmHeight) <= point.y 
							&& rect.bottom > point.y 
							&& (rect.right - bitmap.bmWidth) <= point.x 
							&& rect.right > point.x) {
							m_bGotoDaily = TRUE;
						}
					}

					rect = GetRect(row-1,col);
					COleDateTimeSpan dts;
					dts.SetDateTimeSpan(xGetWeeklyIndex(row,j),0,0,0);
					BOOL bWeekend = (dts.GetDays() >= 5);
					int maxDisplay = bWeekend ? m_nMaxDisplayWeekendRecordsPerDay : m_nMaxDisplayRecordsPerDay;
					if (!m_bGotoDaily) {
						m_nSelectedIndexInOneDay = (point.y-rect.bottom) / m_nHeightPerLineInOneDay;
						if (m_nSelectedIndexInOneDay >= maxDisplay) {
							m_nSelectedIndexInOneDay = -1;
						}
					}
				}
				if (m_bRedraw[xGetWeeklyIndex(row,col)]) {
					if (!m_bPopupMenu) {
						m_HitSameCell = TRUE;
					}
				}
				m_bRedraw[xGetWeeklyIndex(row,col)] = TRUE;

				COleDateTimeSpan dts;
				dts.SetDateTimeSpan((row/2) + (col * 3),0,0,0);
				m_CurrentDate = m_WeeklyStartDate;
				m_CurrentDate += dts;
				m_dtOldMonthfocusDay = m_CurrentDate;

				m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);

				return true;
			}
		}
	}

	return false;
}


int CCAMMScheduleDailyTable::Draw_Weekly(CDC* pDC)
{
	int i,j;
	BOOL bMore = FALSE;
	BOOL bDrawSubItem = FALSE;
	int orgHeightPerLine = m_nHeightPerLineInOneDay;
	BOOL bRedraw = TRUE;
	CString strText[48];
	int strCount=0;

	RECT textRect = GetRect(1,0);
	cells[1*MAX_COLS1+0].CalcTextRect(pDC,&textRect);
	m_nHeightPerLineInOneDay = (textRect.bottom - textRect.top);

	BOOL bDrawBG=TRUE;
	BOOL bDrawWholeBG=FALSE;
	RECT bkRect;
	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			if (!m_bRedraw[xGetWeeklyIndex(i,j)]) {
				bRedraw = FALSE;
				break;
			}
		}
	}
	if (bRedraw) {
		XCell& cellInfo = cells[1*MAX_COLS1+0];
		bkRect = GetRect(1,0);
		bkRect.right = bkRect.left + GetColsWidth(0,cols-1);
		bkRect.bottom = bkRect.top + GetRowsHeight(0,rows-1);
		bkRect.left += 1;
		xDraw_Background(pDC,cellInfo,bkRect);
		bDrawWholeBG=TRUE;
	}
	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			DrawCell_Weekly(pDC,i,j,bDrawSubItem);
		}
	}
/*
	if ((focusRow != 0) && (focusRow != 2) && (focusRow != 4) && (focusRow != 6) && !m_HitSameCell) {
		RECT rtHL = GetRect (focusRow-1, focusCol);
		COLORREF orgBkColor = 	GetCells (focusRow-1, focusCol)->GetBackColor();
		COLORREF orgTextColor = GetCells (focusRow-1, focusCol)->GetTextColor();
		GetCells (focusRow-1, focusCol)->SetBackColor(HIGH_LIGHT_BG_COLOR);
		GetCells (focusRow-1, focusCol)->SetTextColor(HIGH_LIGHT_TEXT_COLOR);
		if ((focusRow-1)+focusCol == 0) {
			BITMAP bitmap;
			m_CalendarImage.GetBitmap(&bitmap);
			GetCells (focusRow-1, focusCol)->Draw(pDC,rtHL,ALIGN_LEFT,m_CalendarImage);
		} else {
			GetCells (focusRow-1, focusCol)->Draw(pDC,rtHL,FALSE,rtHL,m_Image,TRUE);
		}
		GetCells (focusRow-1, focusCol)->SetBackColor(orgBkColor);
		GetCells (focusRow-1, focusCol)->SetTextColor(orgTextColor);
	}

	if ((focusRow != 0) && (focusRow != 2) && (focusRow != 4) && (focusRow != 6)) {
		RECT rect = GetRect (focusRow, focusCol);
		if (bDrawSubItem) {
			rect.top += m_nSelectedIndexInOneDay * m_nHeightPerLineInOneDay;
			rect.bottom = rect.top + m_nHeightPerLineInOneDay;
			GetCells (focusRow, focusCol)->DrawHitBorder(pDC, rect, RGB(255, 0, 20));
		}
	}
*/
	m_nHeightPerLineInOneDay = orgHeightPerLine;
	m_HitSameCell = FALSE;

	return 0;
}

int CCAMMScheduleDailyTable::CreateTable_Weekly()
{
	int startRow = 0;  //At pos of start at 0
	int endRow = 7;

	defaultHeight = 20;
	defaultwidth = 80;
	SetRows (endRow + 1);
	SetCols (2);
	m_ScrollBarPos = 182;
//	xSetScrollRange(SB_VERT,0,GetRows(),TRUE);

	SetScrollRange(SB_VERT,0,364,TRUE);
	SetScrollPos(SB_VERT,m_ScrollBarPos,TRUE);

	for (int i = 0; i < GetRows(); i ++) {
		SetBackColor (i, 0, RGB(187,199,255));
		SetBackColor (i, 1, RGB(187,199,255));
		SetRowHeight(i,20);
		SetBorderColor(i, 0,RGB(68,68,153));
		SetBorderColor(i, 1,RGB(68,68,153));
	
		SetTextLine(i,0,FALSE);
		SetTextLine(i,1,FALSE);


		//		SetBackColor(i,2,RGB(0xCC,0xCC,0xCC));
	}
	//	m_pScheduleDaily[GetRows()-m_InfoRowCnt-1].hiStopTime = m_pScheduleDaily[GetRows()-m_InfoRowCnt-1].hiStartTime;
	//
	//	for (i = 0; i < 9; i ++)
	//		SetBackColor (0, i, RGB(0xC8, 0x96, 0x96));
	

	SetColWidth(0,150);
	SetColWidth (1, 150);

	xCreateTableForWeekly(m_CurrentDate);

	return 0;
}

void CCAMMScheduleDailyTable::OnVScroll_Weekly(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	//SB_BOTTOM   Scroll to bottom. 
	//SB_ENDSCROLL   End scroll. 
	//SB_LINEDOWN   Scroll one line down. 
	//SB_LINEUP   Scroll one line up. 
	//SB_PAGEDOWN   Scroll one page down. 
	//SB_PAGEUP   Scroll one page up. 
	//SB_THUMBPOSITION   Scroll to the absolute position. The current position is provided in nPos. 
	//SB_THUMBTRACK   Drag scroll box to specified position. The current position is provided in nPos. 
	//SB_TOP   Scroll to top.
	SCROLLINFO si;
	GetScrollInfo(SB_VERT,&si,SIF_ALL);

	int delta = 0;
	int newPos = nPos;

	switch (nSBCode) {
	case SB_BOTTOM:
		delta = 7;
		break;
	case SB_ENDSCROLL:
		UpdateWindow();
		break;
	case SB_LINEDOWN:
		delta = 7;
		break;
	case SB_LINEUP:
		delta = -7;
		break;
	case SB_PAGEDOWN:
		delta = 7;
		break;
	case SB_PAGEUP:
		delta = -7;
		break;
	case SB_THUMBPOSITION:
		delta = newPos - si.nPos;
		break;
	case SB_THUMBTRACK:
		break;
	case SB_TOP:
		break;
	}

	if (delta == 0) {
		return;
	}
	int nScrollPos = si.nPos;
	nScrollPos += (delta/7)*7;
	if (nScrollPos > si.nMax) {
		SetScrollRange(SB_VERT,si.nMin,nScrollPos,FALSE);
	}
	if (nScrollPos < si.nMin) {
		nScrollPos = si.nMin;
		SetScrollRange(SB_VERT,nScrollPos,si.nMax,FALSE);
	}
	SetScrollPos(SB_VERT,nScrollPos,TRUE);

	//	xUpdateWeeklyTitle(delta);
	COleDateTimeSpan dts;
	dts.SetDateTimeSpan(abs((delta/7)*7),0,0,0);
	if (delta < 0) {
		m_CurrentDate -= dts;
	} else {
		m_CurrentDate += dts;
	}

	xCreateTableForWeekly(m_CurrentDate);

	xInvalidate(TRUE);
	
	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

//dwm different
void CCAMMScheduleDailyTable::OnSize_Weekly(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	xAssignWeeklyInfo();
}

void CCAMMScheduleDailyTable::OnLButtonDblClk_Weekly(UINT nFlags, CPoint point) 
{
	if (m_bHitTest) {
		int row,col;
		BOOL bSameDate = TRUE;
		if (GetFocusCell(row,col)) {
			//ChangeDatePeriod(DATEPERIOD_DAILY);
			CScheduleInfo si,si2;
			if (!xGetUpdatedScheduleInfo(m_nSelectedIndexInOneDay,si)) {
				return;
			}

			//增加对优先级的初始化 kerm add for 9a9u 08.1.02
			if(si.uiPrivate == 0)
				si.uiPrivate = 2;

			//xShowEditorDlg(si,si2,bSameDate);
			if (xShowEditorDlg(si,si2,bSameDate))
			{
	    		CreateTable();
				xInvalidate(TRUE);
			}
			else	xInvalidate(FALSE);
		}
	}

	CWnd::OnLButtonDblClk(nFlags, point);
}

void CCAMMScheduleDailyTable::xUpdateWeeklyTitle(int delta)
{
	COleDateTimeSpan dts(abs(delta),0,0,0);
	if (delta < 0) {
		m_WeeklyStartDate -= dts;
	} else {
		m_WeeklyStartDate += dts;
	}
	COleDateTimeEx lStartDate = m_WeeklyStartDate;
	dts.SetDateTimeSpan(3,0,0,0);
	COleDateTimeEx rStartDate = lStartDate + dts;
	dts.SetDateTimeSpan(1,0,0,0);
	CString strDate;

	int row = 0;
	strDate = lStartDate.Format(VAR_DATEVALUEONLY);
	SetText(row,0,strDate);
	strDate = rStartDate.Format(VAR_DATEVALUEONLY);
	SetText(row,1,strDate);
	
	row += 2;
	lStartDate += dts;
	strDate = lStartDate.Format(VAR_DATEVALUEONLY);
	SetText(row,0,strDate);
	rStartDate += dts;
	strDate = rStartDate.Format(VAR_DATEVALUEONLY);
	SetText(row,1,strDate);

	row += 2;
	lStartDate += dts;
	strDate = lStartDate.Format(VAR_DATEVALUEONLY);
	SetText(row,0,strDate);
	rStartDate += dts;
	strDate = rStartDate.Format(VAR_DATEVALUEONLY);
	SetText(row,1,strDate);

	row += 2;
	rStartDate += dts;
	strDate = rStartDate.Format(VAR_DATEVALUEONLY);
	SetText(row,1,strDate);

}

void CCAMMScheduleDailyTable::xCreateTableForWeekly(COleDateTimeEx dt)
{
	COleDateTimeEx date = dt;
	COleDateTimeSpan dts;
	int dayOfWeek = date.GetDayOfWeek(); //Sunday = 1,Monday =2  ,etc,.. ,Saturday = 7
	dayOfWeek -= 2;
	if (dayOfWeek < 0) {
		dayOfWeek += 7;
	}
	dts.SetDateTimeSpan(dayOfWeek,0,0,0);
	COleDateTimeEx lStartDate = date - dts;
	m_WeeklyStartDate = lStartDate;
	dts.SetDateTimeSpan(3,0,0,0);
	COleDateTimeEx rStartDate = lStartDate + dts;
	CString strDate;

	int row = 0;
	JoinCells (row,0,row,0); //Hour
	strDate = lStartDate.Format(VAR_DATEVALUEONLY);
	SetText(row,0,strDate);
	SetAlignment(row,0,ALIGN_CENTER | ALIGN_MIDDLE);
	//	SetTextFontSize(row,0,12);
	//		SetBackColor(row,0,RGB(0xC8, 0x96, 0x96));

	JoinCells(row,1,row,1);
	strDate = rStartDate.Format(VAR_DATEVALUEONLY);
	SetText(row,1,strDate);
	SetAlignment(row,1,ALIGN_CENTER | ALIGN_MIDDLE);
	//	SetTextFontSize(row,1,12);
	SetRowHeight(row,16);
	//		SetBackColor(row,0,RGB(0xC8, 0x96, 0x96));

	CString strInfo;
	row++;
	JoinCells (row,0,row,0); //Hour
	SetBackColor(row,0,RGB(0xFF,0xFF,0xFF));
	SetTextLine(row,0,TRUE);
	//	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(lStartDate);
	//	strInfo = xGetScheduleTitles(m_sMonthInfo,lStartDate);
	//	SetText(row,0,strInfo);

	JoinCells(row,1,row,1);
	SetBackColor(row,1,RGB(0xFF,0xFF,0xFF));
	SetTextLine(row,1,TRUE);
	//	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(rStartDate);
	//	strInfo = xGetScheduleTitles(m_sMonthInfo,rStartDate);
	//	SetText(row,1,strInfo);

	dts.SetDateTimeSpan(1,0,0,0);
	row++;
	JoinCells (row,0,row,0); //Hour
	lStartDate += dts;
	strDate = lStartDate.Format(VAR_DATEVALUEONLY);
	SetText(row,0,strDate);
	SetAlignment(row,0,ALIGN_CENTER | ALIGN_MIDDLE);
	//	SetTextFontSize(row,0,12);

	JoinCells(row,1,row,1);
	rStartDate += dts;
	strDate = rStartDate.Format(VAR_DATEVALUEONLY);
	SetText(row,1,strDate);
	SetAlignment(row,1,ALIGN_CENTER | ALIGN_MIDDLE);
	//	SetTextFontSize(row,1,12);
	SetRowHeight(row,16);

	row++;
	JoinCells (row,0,row,0); //Hour
	SetBackColor(row,0,RGB(0xFF,0xFF,0xFF));
	SetTextLine(row,0,TRUE);
	//	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(lStartDate);
	//	strInfo = xGetScheduleTitles(m_sMonthInfo,lStartDate);
	//	SetText(row,0,strInfo);

	JoinCells(row,1,row,1);
	SetBackColor(row,1,RGB(0xFF,0xFF,0xFF));
	SetTextLine(row,1,TRUE);
	//	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(rStartDate);
	//	strInfo = xGetScheduleTitles(m_sMonthInfo,rStartDate);
	//	SetText(row,1,strInfo);

	dts.SetDateTimeSpan(1,0,0,0);
	row++;
	JoinCells (row,0,row,0); //Hour
	lStartDate += dts;
	strDate = lStartDate.Format(VAR_DATEVALUEONLY);
	SetText(row,0,strDate);
	SetAlignment(row,0,ALIGN_CENTER | ALIGN_MIDDLE);
	//	SetTextFontSize(row,0,12);

	JoinCells(row,1,row,1);
	rStartDate += dts;
	strDate = rStartDate.Format(VAR_DATEVALUEONLY);
	SetText(row,1,strDate);
	SetAlignment(row,1,ALIGN_CENTER | ALIGN_MIDDLE);
	//	SetTextFontSize(row,1,12);
	SetRowHeight(row,16);

	row++;
	JoinCells (row,0,row+2,0); //Hour
	SetBackColor(row,0,RGB(0xFF,0xFF,0xFF));
	SetTextLine(row,0,TRUE);
	//	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(lStartDate);
	//	strInfo = xGetScheduleTitles(m_sMonthInfo,lStartDate);
	//	SetText(row,0,strInfo);

	JoinCells(row,1,row,1);
	SetBackColor(row,1,RGB(0xFF,0xFF,0xFF));
	SetTextLine(row,1,TRUE);
	//	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(rStartDate);
	//	strInfo = xGetScheduleTitles(m_sMonthInfo,rStartDate);
	//	SetText(row,1,strInfo);

	row++;
	JoinCells(row,1,row,1);
	rStartDate += dts;
	strDate = rStartDate.Format(VAR_DATEVALUEONLY);
	SetText(row,1,strDate);
	SetAlignment(row,1,ALIGN_CENTER | ALIGN_MIDDLE);
	//	SetTextFontSize(row,1,12);
	SetRowHeight(row,16);

	row++;
	JoinCells(row,1,row,1);
	SetBackColor(row,1,RGB(0xFF,0xFF,0xFF));
	SetTextLine(row,1,TRUE);
	//	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(rStartDate);
	//	strInfo = xGetScheduleTitles(m_sMonthInfo,rStartDate);
	//	SetText(row,1,strInfo);

	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);

	dts = m_CurrentDate - m_WeeklyStartDate;
	focusRow = dts.GetDays() < 3 ? (dts.GetDays() * 2) + 1 : (dts.GetDays() - 3) * 2 + 1;
	focusCol = dts.GetDays() < 3 ? 0 : 1;

}



//Monthly source code
bool CCAMMScheduleDailyTable::HitTest_Monthly (CPoint point, int& row, int& col)
{
	if (!m_bPopupMenu) {
		SetAllUnDraw();
	}
	m_bRedraw[xGetMonthlyIndex(focusRow,focusCol)] = TRUE;
	m_HitSameCell = FALSE;
	for (int i= 0; i < rows; i++) 
	{
		for(int j=0; j < cols; j++)
		{
			RECT rect = GetRect (i,j);
			if (rect.top <= point.y && rect.bottom > point.y && rect.left <= point.x && rect.right > point.x)
			{
				if (i > 0) {
					row = i;
					col = j;

					if (((row-1) % 2) == 0) {
						row++;
						m_nSelectedIndexInOneDay = -1;
					} else {
						if (m_bMore[xGetMonthlyIndex(row,col)]) {
							BITMAP bitmap;
							m_Image.GetBitmap(&bitmap);
							if ((rect.bottom - bitmap.bmHeight) <= point.y 
								&& rect.bottom > point.y 
								&& (rect.right - bitmap.bmWidth) <= point.x 
								&& rect.right > point.x) {
								m_bGotoDaily = TRUE;
							}
						}

						rect = GetRect(row-1,col);
						BOOL bWeekend = (j == 5);
						int maxDisplay = bWeekend ? m_nMaxDisplayWeekendRecordsPerDay : m_nMaxDisplayRecordsPerDay;
						if (!m_bGotoDaily) {
							m_nSelectedIndexInOneDay = (point.y-rect.bottom) / m_nHeightPerLineInOneDay;
							if (m_nSelectedIndexInOneDay >= maxDisplay) {
								m_nSelectedIndexInOneDay = -1;
							}
						}
					}
					if (m_bRedraw[xGetMonthlyIndex(row,col)]) {
						if (!m_bPopupMenu) {
							m_HitSameCell = TRUE;
						}
					}
					m_bRedraw[xGetMonthlyIndex(row,col)] = TRUE;
					//					m_bRedraw[xGetMonthlyIndex(row,col)] = !m_bRedraw[xGetMonthlyIndex(row,col)];

					COleDateTimeSpan dts;

					dts.SetDateTimeSpan(((row-1)/4) * (GetCols() + 1) + (col + (row-1)%4/2),0,0,0);
					m_CurrentDate = m_MonthlyStartDate;
					m_CurrentDate += dts;
					m_dtOldMonthfocusDay  = m_CurrentDate;
					m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);

					return true;
				} else if (i+j == 0) {
					BITMAP bmpInfo;
					m_CalendarImage.GetBitmap(&bmpInfo);
					int left = rect.left;
					int right = left + bmpInfo.bmWidth;
					int top = rect.top + ((rect.bottom - rect.top) - bmpInfo.bmHeight) / 2;
					int bottom = top + bmpInfo.bmHeight;
					if ((point.x >= left) && (point.x <= right)){
						if ((point.y >= top) && (point.y <= bottom)){
							m_bShowCalendar = TRUE;
						}
					}
					return false;
				}
			}
		}
	}

	return false;
}


int CCAMMScheduleDailyTable::Draw_Monthly(CDC* pDC)
{
	int i,j;
//	int nRecordsCount;
	BOOL bDrawSubItem = FALSE;
	int orgHeightPerLine = m_nHeightPerLineInOneDay;
	BOOL bRedraw = TRUE;
	CString strText[48];
//	BOOL bAlarm[48];
//	BOOL bRepeat[48];
	int strCount=0;
	COleDateTimeEx dt;
	
	RECT textRect = GetRect(2,0);
	cells[2*MAX_COLS1+0].CalcTextRect(pDC,&textRect);
	m_nHeightPerLineInOneDay = (textRect.bottom - textRect.top);
	
	BOOL bDrawBG=TRUE;
	BOOL bDrawWholeBG=FALSE;
	RECT bkRect;
	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			if (!m_bRedraw[xGetMonthlyIndex(i,j)]) {
				bRedraw = FALSE;
				break;
			}
		}
	}
	if (bRedraw) {
		XCell& cellInfo = cells[10*MAX_COLS1+0];
		bkRect = GetRect(1,0);
		bkRect.right = bkRect.left + GetColsWidth(0,cols-1);
		bkRect.bottom = bkRect.top + GetRowsHeight(0,rows-1);
		bkRect.left += 1;
		xDraw_Background(pDC,cellInfo,bkRect);
		
		XCell& celllastMonth = cells[1*MAX_COLS1+0];
		bkRect = GetRect(1,0);
		COleDateTimeSpan dts = m_MonthlyFirstDate - m_MonthlyStartDate;
		bkRect.right = bkRect.left + GetColsWidth(0,dts.GetDays());
		bkRect.bottom = bkRect.top + GetRowsHeight(0,4);
		bkRect.left += 1;
		xDraw_Background(pDC,celllastMonth,bkRect);
		
		XCell& cellNextMonth = cells[21*MAX_COLS1+0];
		bkRect = GetRect(21,0);
		bkRect.right = bkRect.left + GetColsWidth(0,cols-1);
		bkRect.bottom = bkRect.top + GetRowsHeight(0,4);
		bkRect.left += 1;
		xDraw_Background(pDC,cellNextMonth,bkRect);
		
		bDrawWholeBG=TRUE;
	}

	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			DrawCell_Monthly(pDC,i,j,bDrawSubItem);
		}
	}
/*	
	if ((focusRow > 0) && !m_HitSameCell) {
		RECT rtHL = GetRect (focusRow-1, focusCol);
		COLORREF orgBkColor = 	GetCells (focusRow-1, focusCol)->GetBackColor();
		COLORREF orgTextColor = GetCells (focusRow-1, focusCol)->GetTextColor();
		GetCells (focusRow-1, focusCol)->SetBackColor(HIGH_LIGHT_BG_COLOR);
		GetCells (focusRow-1, focusCol)->SetTextColor(HIGH_LIGHT_TEXT_COLOR);
		if ((focusRow-1)+focusCol == 0) {
			BITMAP bitmap;
			m_CalendarImage.GetBitmap(&bitmap);
			GetCells (focusRow-1, focusCol)->Draw(pDC,rtHL,ALIGN_LEFT,m_CalendarImage);
		} else {
			GetCells (focusRow-1, focusCol)->Draw(pDC,rtHL,FALSE,rtHL,m_Image,TRUE);
		}
		GetCells (focusRow-1, focusCol)->SetBackColor(orgBkColor);
		GetCells (focusRow-1, focusCol)->SetTextColor(orgTextColor);
	}
	if ((focusRow != 0)) {
		RECT rect = GetRect (focusRow, focusCol);
		if (bDrawSubItem) {
			rect.top += m_nSelectedIndexInOneDay * m_nHeightPerLineInOneDay;
			rect.bottom = rect.top + m_nHeightPerLineInOneDay;
			GetCells (focusRow, focusCol)->DrawHitBorder(pDC, rect, RGB(255, 0, 20));
		}
	}*/
	
	m_nHeightPerLineInOneDay = orgHeightPerLine;
	m_HitSameCell = FALSE;
	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);

	return 0;
}

//int CCAMMScheduleDailyTable::Draw_Monthly(CDC* pDC)
//{
//	int i,j;
//	int nRecordsCount;
//	BOOL bMore = FALSE;
//	BOOL bDrawSubItem = FALSE;
//	int orgHeightPerLine = m_nHeightPerLineInOneDay;
//	BOOL bRedraw = TRUE;
//	CString strText[48];
//	BOOL bAlarm[48];
//	BOOL bRepeat[48];
//	int strCount=0;
//
//	CDC dcMemory;
//	dcMemory.CreateCompatibleDC(pDC);
//
//	RECT textRect = GetRect(2,0);
//	cells[2*MAX_COLS1+0].CalcTextRect(pDC,&textRect);
//	m_nHeightPerLineInOneDay = (textRect.bottom - textRect.top);
//
//	CBitmap bmp;
//    BITMAP bmpInfo;
//	bmp.CreateCompatibleBitmap(&dcMemory,GetColsWidth(0,cols-1),GetRowsHeight(0,rows-1));
//	CBitmap* pOldBitmap = dcMemory.SelectObject(&bmp);
//	for (i = 0; i < rows; i++)
//	{
//		for (j = 0; j < cols; j++)
//		{
//			XCell& cell = cells[i*MAX_COLS1+j];
//			bMore = FALSE;
//
//			if (cell.colSpan !=0 && cell.rowSpan != 0)
//			{
//					if (((i-1) % 2) == 1) {
//						COleDateTimeEx dt;
//						COleDateTimeSpan dts;
//						CString strInfo;
//						int ts = xGetMonthlyIndex(i,j);//(i-1)%4==3?((i-3) / 4)*7 + j + 1:((i-1) / 4)*7 + j;
//						dts.SetDateTimeSpan(ts,0,0,0);
//						dt = m_MonthlyStartDate + dts;
//						m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(dt);
//						
//						BOOL bWeekend = (j == 5);
//						strInfo = xGetMonthlyScheduleTitles(m_sMonthInfo,dt,bWeekend,nRecordsCount,strText,strCount,bAlarm,bRepeat);
//						SetText(i,j,strText,strCount);
//						SetAlarm(i,j,bAlarm,strCount);
//						SetRepeat(i,j,bRepeat,strCount);
//						m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);
//						
//						int maxDisplay = bWeekend ? m_nMaxDisplayWeekendRecordsPerDay : m_nMaxDisplayRecordsPerDay;
//						bMore = (nRecordsCount > 0) && (nRecordsCount > maxDisplay) ;
//						m_bMore[dts.GetDays()] = bMore;
//
//						if ((focusRow == i) && (focusCol == j)) {
//							bDrawSubItem = (m_nSelectedIndexInOneDay >= 0) && (m_nSelectedIndexInOneDay < min(nRecordsCount,maxDisplay));
//						}
//
//						//						if (!bRedraw || bDrawWholeBG) {
//						//							if (dt.GetMonth() == m_MonthlyFirstDate.GetMonth()) {
//						//								bDrawBG = FALSE;
//						//							}
//						//						}
//					}
//
//					RECT rect = GetRect(i,j);
//					
//					BITMAP bitmap;
//					m_Image.GetBitmap(&bitmap);
//					RECT moreRect;
//					moreRect.left = rect.right - bitmap.bmWidth;
//					moreRect.top = rect.bottom - bitmap.bmHeight;
//					moreRect.right = moreRect.left + bitmap.bmWidth;
//					moreRect.bottom = moreRect.top + bitmap.bmHeight;
//						if (i+j == 0) {
//							BITMAP bitmap;
//							m_CalendarImage.GetBitmap(&bitmap);
//							cell.Draw(&dcMemory,rect,ALIGN_LEFT,m_CalendarImage);
//						} else {
//							cell.Draw(&dcMemory,rect,bMore,moreRect,m_Image,TRUE);
//						}
//			}
//		}
//	}
//
//	if ((focusRow > 0) && !m_HitSameCell) {
//		RECT rtHL = GetRect (focusRow-1, focusCol);
//		COLORREF orgBkColor = 	GetCells (focusRow-1, focusCol)->GetBackColor();
//		COLORREF orgTextColor = GetCells (focusRow-1, focusCol)->GetTextColor();
//		GetCells (focusRow-1, focusCol)->SetBackColor(HIGH_LIGHT_BG_COLOR);
//		GetCells (focusRow-1, focusCol)->SetTextColor(HIGH_LIGHT_TEXT_COLOR);
//		if ((focusRow-1)+focusCol == 0) {
//			BITMAP bitmap;
//			m_CalendarImage.GetBitmap(&bitmap);
//			GetCells (focusRow-1, focusCol)->Draw(&dcMemory,rtHL,ALIGN_LEFT,m_CalendarImage);
//		} else {
//			GetCells (focusRow-1, focusCol)->Draw(&dcMemory,rtHL,FALSE,rtHL,m_Image,TRUE);
//		}
//		GetCells (focusRow-1, focusCol)->SetBackColor(orgBkColor);
//		GetCells (focusRow-1, focusCol)->SetTextColor(orgTextColor);
//	}
//	if ((focusRow != 0)) {
//		RECT rect = GetRect (focusRow, focusCol);
//		if (bDrawSubItem) {
//			rect.top += m_nSelectedIndexInOneDay * m_nHeightPerLineInOneDay;
//			rect.bottom = rect.top + m_nHeightPerLineInOneDay;
//		}
//		GetCells (focusRow, focusCol)->DrawHitBorder(&dcMemory, rect, RGB(255, 0, 20));
//	}
//
//	m_nHeightPerLineInOneDay = orgHeightPerLine;
//	m_HitSameCell = FALSE;
//
//	pDC->BitBlt(0,0,GetColsWidth(0,cols-1),GetRowsHeight(0,rows-1),&dcMemory,0,0,SRCCOPY);
//    dcMemory.SelectObject(pOldBitmap);
//
//	return 0;
//}
//dwm different
int CCAMMScheduleDailyTable::CreateTable_Monthly()
{
	int startRow = 0;  //At pos of start at 0
//	int endRow = 24;
	int endRow = 20;

	defaultHeight = 20;
	defaultwidth = 80;
	SetRows (endRow + 1);
	SetCols (6);
	SetScrollRange(SB_VERT,0,730,FALSE);
	m_ScrollBarPos = 365;
	SetScrollPos(SB_VERT,m_ScrollBarPos,TRUE);

	int i,j;
	RECT rect;
	GetClientRect(&rect);
	CString strWeekDay[6] = {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday/Sunday"};

	strWeekDay[0] = LoadStringFromFile(_T("public"),_T("Monday"));
	strWeekDay[1] = LoadStringFromFile(_T("public"),_T("Tuesday"));
	strWeekDay[2] = LoadStringFromFile(_T("public"),_T("Wednesday"));
	strWeekDay[3] = LoadStringFromFile(_T("public"),_T("Thursday"));
	strWeekDay[4] = LoadStringFromFile(_T("public"),_T("Friday"));
	strWeekDay[5] = LoadStringFromFile(_T("public"),_T("Saturday"));


	strWeekDay[5] += _T("/");
	strWeekDay[5] += LoadStringFromFile(_T("public"),_T("Sunday"));

	int colWidth = (rect.right - rect.left - 20) / 6;
	int smallRowHeight = 15;
//	int bigRowHeight = ((rect.bottom - rect.top) - 13 * smallRowHeight) / 12;
	int bigRowHeight = ((rect.bottom - rect.top) - 11 * smallRowHeight) / 10;
	for (i = 0; i < GetCols(); i ++) {
		SetBackColor (0, i, RGB(187,199,255));
		SetColWidth(i,colWidth);
		SetText(0,i,strWeekDay[i]);
		SetAlignment(0,i,ALIGN_CENTER);
		SetAlignment(0,i,ALIGN_CENTER | ALIGN_TOP);
		//		SetTextFontSize(0,i,12);
		SetRowHeight(0,smallRowHeight);
		SetBorderColor(0,i,RGB(68,68,153));

	}
	for (i = 1; i < GetRows(); i ++) 
	{
		for (j = 0; j < GetCols(); j ++) 
		{
			if(i%2 == 0)
			{
				SetBackColor (i, j, RGB(187,199,255));
				SetTextLine(i,j,TRUE);
			}
			else
			{
				SetBackColor (i, j, RGB(0xFF, 0xFF, 0xFF));
				SetTextLine(i,j,FALSE);
			}
			SetBorderColor(i, j,RGB(68,68,153));
			SetRowHeight(i,smallRowHeight);
		//	SetBorderColor(i, j,RGB(68,68,153));
			//			SetTextFontSize(i,j,12);
		}
	}

	int row = 0;
	for (j=0;j<GetCols()-1;j++) {
		row = 0;
	//	for (i=0;i<6;i++) {
		for (i=0;i<5;i++) {
			row++;
			JoinCells(row,j,row,j);
			SetRowHeight(row,smallRowHeight);

			row++;
			JoinCells(row,j,row+2,j);
			SetRowHeight(row,bigRowHeight);
			SetRowHeight(row+1,smallRowHeight);
			SetRowHeight(row+2,bigRowHeight);
			row += 2;
		}
	}
	row = 0;
	for (row=1;row<GetRows()-1;row++) {
		JoinCells(row,GetCols()-1,row,GetCols()-1);
	}

	xCreateTableForMonthly(m_CurrentDate);

	return 0;
}

void CCAMMScheduleDailyTable::OnVScroll_Monthly(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	//SB_BOTTOM   Scroll to bottom. 
	//SB_ENDSCROLL   End scroll. 
	//SB_LINEDOWN   Scroll one line down. 
	//SB_LINEUP   Scroll one line up. 
	//SB_PAGEDOWN   Scroll one page down. 
	//SB_PAGEUP   Scroll one page up. 
	//SB_THUMBPOSITION   Scroll to the absolute position. The current position is provided in nPos. 
	//SB_THUMBTRACK   Drag scroll box to specified position. The current position is provided in nPos. 
	//SB_TOP   Scroll to top.
	SCROLLINFO si;
	GetScrollInfo(SB_VERT,&si,SIF_ALL);

	int delta = 0;
	int newPos = nPos;

	COleDateTimeSpan dts;

	switch (nSBCode) {
	case SB_BOTTOM:
		delta = 1;
		break;
	case SB_ENDSCROLL:
		//UpdateWindow();
		break;
	case SB_LINEDOWN:
		delta = 1;
		break;
	case SB_LINEUP:
		delta = -1;
		break;
	case SB_PAGEDOWN:
		delta = 3;
		break;
	case SB_PAGEUP:
		delta = -3;
		break;
	case SB_THUMBPOSITION:
		delta = (newPos - si.nPos) / 30;
		break;
	case SB_THUMBTRACK:
		break;
	case SB_TOP:
		break;
	}

	int year,month;
	year = m_MonthlyFirstDate.GetYear();
	month = m_MonthlyFirstDate.GetMonth();
	if (delta == 0) {
		return;
	} else if (delta < 0) {
		delta = abs(delta);
		year -= abs(((month - delta) - 12)/12);
		month = (month - delta) <= 0 ? 12 - ((delta - month) % 12): (month - delta);
	} else {
		year += ((month + delta - 1)/12);
		month = (month + delta) > 12 ? ((delta + month) % 12): (month + delta);
	}
	//	CreateTable();
	COleDateTimeEx dt = m_MonthlyFirstDate;
	m_MonthlyFirstDate.SetDate(year,month,1);


	int nDay = m_dtOldMonthfocusDay.GetDay();
	if(nDay <=28)
		m_CurrentDate.SetDate(year,month,nDay);
	else
	{
		COleDateTime temp;
		if(month <12)
			temp.SetDate(year,month+1,1);
		else
			temp.SetDate(year+1,1,1);
	
		COleDateTimeSpan dts;
		dts.SetDateTimeSpan(1,0,0,0);
		temp -= dts;

		int nLastDay = temp.GetDay();
		if(nDay > nLastDay) nDay = nLastDay;
		m_CurrentDate.SetDate(year,month,nDay);

	}

	int nScrollPos = si.nPos;
	if (delta < 0) {
		dts = dt - m_MonthlyFirstDate;
		nScrollPos -= dts.GetDays();
	} else {
		dts = m_MonthlyFirstDate - dt;
		nScrollPos += dts.GetDays();
	}
	if (nScrollPos > si.nMax) {
		SetScrollRange(SB_VERT,si.nMin,nScrollPos,FALSE);
	}
	if (nScrollPos < si.nMin) {
		nScrollPos = si.nMin;
		SetScrollRange(SB_VERT,nScrollPos,si.nMax,FALSE);
	}
	SetScrollPos(SB_VERT,nScrollPos,TRUE);

	xCreateTableForMonthly(m_CurrentDate);

	xInvalidate(TRUE);
	UpdateWindow();

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CCAMMScheduleDailyTable::OnSize_Monthly(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	xAssignMonthlyInfo();
}

void CCAMMScheduleDailyTable::OnLButtonDblClk_Monthly(UINT nFlags, CPoint point) 
{
	if (m_bHitTest) {
		int row,col;
		BOOL bSameDate = TRUE;
		if (GetFocusCell(row,col)) {
			//ChangeDatePeriod(DATEPERIOD_DAILY);
			CScheduleInfo si,si2;
			if (!xGetUpdatedScheduleInfo(m_nSelectedIndexInOneDay,si)) 
			{
				return;
			}
			//增加对优先级的初始化 kerm add for 9a9u 08.1.02
			if(si.uiPrivate == 0)
				si.uiPrivate = 2;

			//xShowEditorDlg(si,si2,bSameDate);
			if (xShowEditorDlg(si,si2,bSameDate))
			{
	    		CreateTable();
				xInvalidate(TRUE);
			}
			else
				xInvalidate(FALSE);
		}
	}

	CWnd::OnLButtonDblClk(nFlags, point);
}


void CCAMMScheduleDailyTable::xCreateTableForMonthly(COleDateTimeEx dt)
{
	COleDateTimeEx date = dt;
	COleDateTimeSpan dts;
	m_MonthlyFirstDate.SetDate(dt.GetYear(),dt.GetMonth(),1);
	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_MonthlyFirstDate);

	int firstDayOfWeek = m_MonthlyFirstDate.GetDayOfWeek(); //Sunday = 1,Monday =2  ,etc,.. ,Saturday = 7
	
	firstDayOfWeek -= 2;  //0 ~ 6
	if (firstDayOfWeek < 0) {
		firstDayOfWeek += 7;
	}
	dts.SetDateTimeSpan(firstDayOfWeek,0,0,0);
	m_MonthlyStartDate = m_MonthlyFirstDate - dts;

	dts.SetDateTimeSpan(1,0,0,0);
	COleDateTimeEx dt2 = m_MonthlyStartDate - dts;
	CString strDate;

	int col,row=1,i;
	CString strInfo;
//	for (i=0;i<6;i++) {
	for (i=0;i<5;i++) 
	{
		for (col=0;col<GetCols();col++) 
		{
			if (col == GetCols() -1)
			{
				dt2 += dts;
				strDate = dt2.Format(VAR_DATEVALUEONLY);
				SetText(row,col,strDate);
				//				SetTextFontSize(row,col,12);
				SetAlignment(row,col,ALIGN_CENTER | ALIGN_TOP);
				if (dt2.GetMonth() != m_MonthlyFirstDate.GetMonth()) {
					SetBackColor(row,col,RGB(223,227,239));
					SetBackColor(row+1,col,RGB(223,227,239));
				} else {
					SetBackColor(row,col,RGB(187,199,255));
					SetBackColor(row+1,col,RGB(0xFF,0xFF,0xFF));
				}
				//				m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(dt2);
				//				strInfo = xGetScheduleTitles(m_sMonthInfo,dt2);
				//				SetText(row+1,col,strInfo);
				
				dt2 += dts;
				strDate = dt2.Format(VAR_DATEVALUEONLY);
				SetText(row+2,col,strDate);
				SetAlignment(row+2,col,ALIGN_CENTER | ALIGN_TOP);
				if (dt2.GetMonth() != m_MonthlyFirstDate.GetMonth()) {
					SetBackColor(row+2,col,RGB(223,227,239));
					SetBackColor(row+2+1,col,RGB(223,227,239));
				} else {
					SetBackColor(row+2,col,RGB(187,199,255));
					SetBackColor(row+2+1,col,RGB(0xFF,0xFF,0xFF));
				}
				//				m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(dt2);
				//				strInfo = xGetScheduleTitles(m_sMonthInfo,dt2);
				//				SetText(row+2+1,col,strInfo);
			} else {
				dt2 += dts;
				strDate = dt2.Format(VAR_DATEVALUEONLY);
				SetText(row,col,strDate);
				//				SetTextFontSize(row,col,12);
				SetAlignment(row,col,ALIGN_CENTER | ALIGN_TOP);
				if (dt2.GetMonth() != m_MonthlyFirstDate.GetMonth()) {
					SetBackColor(row,col,RGB(223,227,239));
					SetBackColor(row+1,col,RGB(223,227,239));
				} else {
					SetBackColor(row,col,RGB(187,199,255));
					SetBackColor(row+1,col,RGB(0xFF,0xFF,0xFF));
				}
				//				m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(dt2);
				//				strInfo = xGetScheduleTitles(m_sMonthInfo,dt2);
				//				SetText(row+1,col,strInfo);
			}
		}
		row += 4;
	}
	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);

	dts = m_CurrentDate - m_MonthlyStartDate;
	focusRow = dts.GetDays() % 7 < 6 ? (dts.GetDays() / 7) * 4 + 2 : (dts.GetDays() / 7) * 4 + 4 ;
	focusCol = dts.GetDays() % 7 < 6 ? dts.GetDays() % 7 : dts.GetDays() % 7 - 1;

}



CString CCAMMScheduleDailyTable::xGetScheduleTitles(CScheduleMonthInfo *pMi, COleDateTimeEx dt)
{
	CString strRet;

	int nInfoSize = pMi->GetInfoSize();
	int nInfoCount = 0 ;

	for (int row=0;row<nInfoSize;row++)
	{
		nInfoCount = pMi->GetInfo(row).GetCount();
		for (int j=0;j<nInfoCount;j++) {
			CScheduleInfo si = pMi->GetInfo(row).GetScheduleInfo(j);
			if (si.haveRepeat|| !si.bEndDateSameToStartDate) {
				if (si.originalData) {
					continue;
				}
			}
			if (!strRet.IsEmpty()) {
				strRet += "\n";
			}
			strRet += si.hiRealStartTime.FormatString();
			strRet += "-";
			strRet += si.hiRealStopTime.FormatString();
			strRet += " ";
			strRet += si.csTitle;
		}
	}
	return strRet;
}

CString CCAMMScheduleDailyTable::xGetWeeklyScheduleTitles(CScheduleMonthInfo *pMi, COleDateTimeEx dt, BOOL bWeekend, int &nRecordsCount,CString strReturn[48],int &retCount,BOOL bAlarm[48],BOOL bRepeat[48])
{
	CString strRet;
	CString strTmp;
	nRecordsCount = 0;
	retCount = 0;
	int maxDisplay = bWeekend ? m_nMaxDisplayWeekendRecordsPerDay : m_nMaxDisplayRecordsPerDay;
	int nInfoSize = pMi->GetInfoSize();
	int nInfoCount = 0 ;
	for (int row=0;row<nInfoSize;row++)
	{
		nInfoCount =pMi->GetInfo(row).GetCount();
		for (int j=0;j<nInfoCount;j++) {
			//		if (pMi->GetInfo(row).bScheduleStart) {
			CScheduleInfo si = pMi->GetInfo(row).GetScheduleInfo(j);
			if (si.haveRepeat|| !si.bEndDateSameToStartDate) {
				if (si.originalData) {
					continue;
				}
			}
			if (nRecordsCount < maxDisplay) {
				strTmp = si.hiRealStartTime.FormatString();
				strTmp += "-";
				strTmp += si.hiRealStopTime.FormatString();
				strTmp += " ";
				strTmp += si.csTitle;
				bAlarm[nRecordsCount] = si.haveAlarm;
				bRepeat[nRecordsCount] = si.haveRepeat;
				if (!strRet.IsEmpty()) {
					strRet += "\n";
				}
				strRet += strTmp;
				strReturn[retCount++] = strTmp;
			} else {
				nRecordsCount++;
				return strRet;
			}
			nRecordsCount++;
		}
	}
	return strRet;
}

CString CCAMMScheduleDailyTable::xGetMonthlyScheduleTitles(CScheduleMonthInfo *pMi, COleDateTimeEx dt, BOOL bWeekend, int &nRecordsCount,CString strReturn[48],int &retCount,BOOL bAlarm[48],BOOL bRepeat[48])
{
	CString strRet;
	CString strTmp;
	nRecordsCount = 0;
	retCount = 0;
	int maxDisplay = bWeekend ? m_nMaxDisplayWeekendRecordsPerDay : m_nMaxDisplayRecordsPerDay;
	int nInfoSize = pMi->GetInfoSize();
	int nInfoCount = 0 ;
	for (int row=0;row<nInfoSize;row++) 
	{
		nInfoCount = pMi->GetInfo(row).GetCount();
		for (int j=0;j<nInfoCount;j++) {
			//		if (pMi->GetInfo(row).bScheduleStart) {
			CScheduleInfo si = pMi->GetInfo(row).GetScheduleInfo(j);
			if (si.haveRepeat|| !si.bEndDateSameToStartDate) {
				if (si.originalData) {
					continue;
				}
			}
			if (nRecordsCount < maxDisplay) {
				//				strTmp = si.hiRealStartTime.FormatString();
				//				strTmp += "-";
				//				strTmp += si.hiRealStopTime.FormatString();
				//				strTmp += " ";
				strTmp = si.csTitle;
				bAlarm[nRecordsCount] = si.haveAlarm;
				bRepeat[nRecordsCount] = si.haveRepeat;
				if (!strRet.IsEmpty()) {
					strRet += "\n";
				}
				strRet += strTmp;
				strReturn[retCount++] = strTmp;
			} else {
				nRecordsCount++;
				return strRet;
			}
			nRecordsCount++;
		}
	}
	return strRet;
}



void CCAMMScheduleDailyTable::xClearTable() {
/*	switch (m_DatePeriod) {
	case DATEPERIOD_DAILY:
		break;
	case DATEPERIOD_WEEKLY:
		break;
	case DATEPERIOD_MONTHLY:
		break;
	}

	//REFRESH LEFT_PANEL to change the totla number of calendars displayed on left view.
	if (m_bMobileConnected) {
		CCAMMSync sc;
     	g_numTotalCalendars = sc.GetCalendarCount();
        ::SendMessage(afxGetMainWnd()->GetSafeHwnd(), WM_REFRESH_LEFT_PANEL, (DWORD)true, NULL);
	}*/
}

void CCAMMScheduleDailyTable::ChangeDatePeriod(Schedule_DatePeriod dp)
{
	m_bGotoDaily = FALSE;

	SetAllRedraw();

	xClearTable();
	//	m_DatePeriod = m_DatePeriod == DATEPERIOD_DAILY?DATEPERIOD_WEEKLY:DATEPERIOD_DAILY;
	m_DatePeriod = dp;
	CreateTable();

	//	focusRow = -1;
	//	focusCol = -1;
}


void CCAMMScheduleDailyTable::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	//	int row,col;
/*	HitTest (point,focusRow,focusCol);

	POINT ptAction = point;
	ClientToScreen(&ptAction);
	m_HitSameCell = FALSE;
	xInvalidate(FALSE);
//	m_bPopupMenu = TRUE;
*/
	CWnd::OnRButtonDown(nFlags, point);
}

void CCAMMScheduleDailyTable::OnOrgSchDelete() 
{
	// TODO: Add your command handler code here
	m_bPopupMenu = FALSE;
	int row,col;
	if (!GetFocusCell(row,col)) {
		return;
	}
	CScheduleInfo si;
	if (xGetSelectedItem(si)) {
	/*	CString s = LoadStringFromFile(ghSkinResource,IDS_ORG_RECORD_DELETE);
		CString Msg;
		Msg.Format((LPCTSTR)s,1);
		if(AfxMessageBox(Msg,MB_YESNO|MB_ICONQUESTION) == IDYES) {
			xDeleteScheduleItem();
		}*/
		CString  s = LoadStringFromFile(_T("message"),_T("MSG_DELETE"));
		CString msg ;
		msg.Format(s,1);
		if(CalendarStringMessageBox(m_hWnd,msg,MB_YESNO|MB_ICONQUESTION) == IDYES)
			xDeleteScheduleItem();
	}

	//Added by Nono
	((CCAMMScheduleDayView *)GetParent())->SetInfomationonStatusBar();
}


void CCAMMScheduleDailyTable::OnOrgSchDaily() 
{
	// TODO: Add your command handler code here
	ChangeDatePeriod(DATEPERIOD_DAILY);
}

void CCAMMScheduleDailyTable::OnUpdateOrgSchDaily(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if (m_DatePeriod != DATEPERIOD_DAILY) {
		pCmdUI->Enable(TRUE); //Always can Add
	} else {
		pCmdUI->Enable(FALSE); //Always can Add
	}
	
}

void CCAMMScheduleDailyTable::OnOrgSchWeekly() 
{
	// TODO: Add your command handler code here
	ChangeDatePeriod(DATEPERIOD_WEEKLY);
	
}

void CCAMMScheduleDailyTable::OnUpdateOrgSchWeekly(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if (m_DatePeriod != DATEPERIOD_WEEKLY) {
		pCmdUI->Enable(TRUE); //Always can Add
	} else {
		pCmdUI->Enable(FALSE); //Always can Add
	}
	
}

void CCAMMScheduleDailyTable::OnOrgSchMonthly() 
{
	// TODO: Add your command handler code here
	ChangeDatePeriod(DATEPERIOD_MONTHLY);
	
}

void CCAMMScheduleDailyTable::OnUpdateOrgSchMonthly(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if (m_DatePeriod != DATEPERIOD_MONTHLY) {
		pCmdUI->Enable(TRUE); //Always can Add
	} else {
		pCmdUI->Enable(FALSE); //Always can Add
	}
	
}

BOOL CCAMMScheduleDailyTable::xGetUpdatedScheduleInfo(int selectedIndex, CScheduleInfo &si)
{
	if (m_sMonthInfo->GetUpdatedInfo(selectedIndex,si)) 
	{
		if (si.csTitle.IsEmpty()) {
			if (xCheckRecordMaxLimit()) {
				return FALSE;
			}
		}
		if (/*si.haveRepeat|| */!si.bEndDateSameToStartDate) 
		{
			COleDateTimeEx oleStopDate = si.dtStopDate;
			m_s3MonthInfo.GetRepeatOriginalInfo(si);
			GetRepeatCurrentDateInfo(si,oleStopDate);
		}
		return TRUE;
	}

	return FALSE;
}
/*
void CCAMMScheduleDailyTable::LoadScheduleFromFile(CString csFileName) {
	CCAMMSync cs;
	int recordCount=0;

	m_csFileName = csFileName;

	cs.GetRecordCount(MO_SCH,csFileName,recordCount);
	if (recordCount > 0) {
	    //nono, 2004_1029
		if (m_pScheduleRecord) delete [] m_pScheduleRecord;
		//
		m_pScheduleRecord = new SchedulesRecord[recordCount];
		cs.LoadSchedule(m_pScheduleRecord,csFileName,recordCount);
		m_s3MonthInfo.SetScheduleData(m_pScheduleRecord,recordCount);
	
		m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);
	}
	m_pScheduleRecord = NULL;
}

void CCAMMScheduleDailyTable::SaveScheduleToFile(CString csFileName) {
	CCAMMSync cs;
	int recordCount=0;

	m_csFileName = csFileName;
//AfxMessageBox(csFileName);
	m_s3MonthInfo.GetScheduleData(m_pScheduleRecord,recordCount);
	if (recordCount > 0) {
		cs.SaveSchedule(m_pScheduleRecord,csFileName,recordCount);
	}
	m_DirtyFlag = FALSE;
}
*/
BOOL CCAMMScheduleDailyTable::IsDataDirty()
{
	return m_DirtyFlag;

}

void CCAMMScheduleDailyTable::xAssignDailyInfo()
{
	if (m_InfoRowCnt < 0) {
		return;
	}
	RECT rect;
	GetClientRect(&rect);

	int rowHeight = GetRowHeight(m_InfoRowCnt);
	m_RowsPerPage = (rect.bottom - rect.top) / rowHeight + 1;
	if (m_RowsPerPage >= GetRows()) {
		m_StartRowForDisplay = m_InfoRowCnt;
	}
	
	m_RowsPerPage -= m_InfoRowCnt;
	m_MaxRowForDisplay = GetRows() - m_RowsPerPage;

	if (m_MaxRowForDisplay < m_StartRowForDisplay) {
		m_StartRowForDisplay = m_MaxRowForDisplay;
	}
	m_RealStartRow = m_StartRowForDisplay > m_InfoRowCnt ? m_StartRowForDisplay-1 : m_InfoRowCnt;
	//	if (m_sMonthInfo->GetCurrentDayInfo().bDirtyFlag[m_RealStartRow-m_InfoRowCnt]) {
	//		if (!(m_sMonthInfo->GetInfo(m_RealStartRow - m_InfoRowCnt).bScheduleStart)) {
	//			for (int i=m_RealStartRow-m_InfoRowCnt;i>=0;i--) {
	//				if (!(m_sMonthInfo->GetCurrentDayInfo().bDirtyFlag[i])) {
	//					m_RealStartRow = i + 1;
	//					break;
	//				}
	//			}
	//		}
	//	}
	
	SCROLLINFO si;
	si.fMask = SIF_PAGE;
	si.nPage = m_RowsPerPage;
	SetScrollInfo(SB_VERT,&si,FALSE);

	int colsWidth = GetColsWidth(0,1);
	SetColWidth(2,(rect.right-rect.left-colsWidth));

	GetClientRect(&rect);
	SetRowHeight(m_InfoRowCnt -1,(rect.bottom - rect.top) - (m_RowsPerPage + m_InfoRowCnt -1) * GetRowHeight(m_InfoRowCnt));
	//	if (((rect.bottom - rect.top) - (m_RowsPerPage + m_InfoRowCnt -1) * GetRowHeight(m_InfoRowCnt)) < 0) {
	//		int i = 0;
	//	}

	SetAllRedraw();
}

void CCAMMScheduleDailyTable::xAssignWeeklyInfo()
{
	RECT rect;
	GetClientRect(&rect);

	int nWidth = (rect.right - rect.left) / 2;
	SetColWidth(0,nWidth);
	SetColWidth(1,nWidth);

	int bigHeight = rect.bottom - rect.top;
	bigHeight -= GetRowHeight(0) * 3;
	bigHeight /= 3;
	int smallHeight = bigHeight - GetRowHeight(0);
	smallHeight /= 2;

	SetRowHeight(1,bigHeight);
	SetRowHeight(3,bigHeight);
	SetRowHeight(5,smallHeight);
	SetRowHeight(7,smallHeight);

	CClientDC dc(this);
	RECT textRect = GetRect(1,0);
	cells[1*MAX_COLS1+0].CalcTextRect(&dc,&textRect);
	m_nHeightPerLineInOneDay = (textRect.bottom - textRect.top);

	m_nMaxDisplayWeekendRecordsPerDay = (smallHeight / m_nHeightPerLineInOneDay)-1;
	m_nMaxDisplayRecordsPerDay = (bigHeight / m_nHeightPerLineInOneDay)-1;
	m_bGotoDaily = FALSE;

	SetAllRedraw();
}

void CCAMMScheduleDailyTable::xAssignMonthlyInfo()
{
	RECT rect;
	GetClientRect(&rect);

	int nWidth = (rect.right - rect.left) / 6;
	for (int col=0;col<GetCols();col++) {
		SetColWidth(col,nWidth);
	}

	int smallRowHeight = GetRowHeight(1);
	int bigRowHeight = ((rect.bottom - rect.top) - 11 * smallRowHeight) / 10;

	int row = 0;
	int i,j;
	for (j=0;j<GetCols()-1;j++) {
		row = 0;
//		for (i=0;i<6;i++) {
		for (i=0;i<5;i++) {
			row++;
			SetRowHeight(row,smallRowHeight);

			row++;
			SetRowHeight(row,bigRowHeight);
			SetRowHeight(row+1,smallRowHeight);
			SetRowHeight(row+2,bigRowHeight);
			row += 2;
		}
	}

	CClientDC dc(this);
	RECT textRect = GetRect(2,0);
	cells[2*MAX_COLS1+0].CalcTextRect(&dc,&textRect);
	m_nHeightPerLineInOneDay = textRect.bottom - textRect.top;


	m_nMaxDisplayWeekendRecordsPerDay = bigRowHeight / m_nHeightPerLineInOneDay - 1;
	m_nMaxDisplayRecordsPerDay = 2 * bigRowHeight / m_nHeightPerLineInOneDay;
	m_bGotoDaily = FALSE;

	SetAllRedraw();

}


int CCAMMScheduleDailyTable::xGetWeeklyIndex(int row, int col)
{
	if ((row <0) || (row >= rows)) {
		return 0;
	}
	if ((col <0) || (col >= cols)) {
		return 0;
	}
	return (row / 2) + (col * 3);
}

int CCAMMScheduleDailyTable::xGetMonthlyIndex(int row, int col)
{
	if ((row <0) || (row >= rows)) {
		return 0;
	}
	if ((col <0) || (col >= cols)) {
		return 0;
	}
	return (row-1)%4>=2?((row-3) / 4)*7 + col + 1:((row-1) / 4)*7 + col;
	//	return (row-1)%4==3?((row-3) / 4)*7 + col + 1:((row-1) / 4)*7 + col;
	//((row-1)/4) * (GetCols() + 1) + (col + (row-1)%4/2)]
}          

void CCAMMScheduleDailyTable::SetAllRedraw()
{
	for (int i=0;i<sizeof(m_bRedraw)/sizeof(BOOL);i++) {
		m_bRedraw[i] = TRUE;
	}
}

void CCAMMScheduleDailyTable::SetAllUnDraw()
{
	for (int i=0;i<sizeof(m_bRedraw)/sizeof(BOOL);i++) {
		m_bRedraw[i] = FALSE;
	}
}

void CCAMMScheduleDailyTable::xInit(int row,int col)
{
	RegisterWindowClass();

	//cells = new XCell [MAX_ROWS1 * MAX_COLS1+1];//2004_1116
	if (cells) {
		delete [] cells;
	}
	cells = new XCell [MAX_ROWS1 * MAX_COLS1];
	rowHeight = new int [MAX_ROWS1+1];
	colWidth = new int [MAX_COLS1+1];

	
	if (row == -1) {
		row = 20;
		col = 80;
	}
	defaultHeight = 20;
	defaultwidth = 80;
	
	rows = 0;
	cols = 0;
	SetRows (row);
	SetCols (col);	

	focusRow = -1;
	focusCol = -1;

	m_bHitTest = FALSE;

	//Current month
    m_CurrentDate = COleDateTime::GetCurrentTime();
//************************************************************
	//增加以下代码 获取正确的当前时间 kerm add for 9a9u 1.17
	CCAMMScheduleCalendar Calendardlg;
	COleDateTime currentDate;
	currentDate.SetDate(m_CurrentDate.GetYear(),m_CurrentDate.GetMonth(),m_CurrentDate.GetDay());
	
	Calendardlg.SetCurrentDate(m_CurrentDate);
	Calendardlg.Create(CCAMMScheduleCalendar::IDD, NULL);
	Calendardlg.GetTodayInfo(m_CurrentDate);
	/**/
//************************************************************



	//kerm test for 9a9u 1.15
// 	CString st;
// 	st.Format(L"current time  %d-%d-%d", 
// 		m_CurrentDate.GetYear(),
// 		m_CurrentDate.GetMonth(),
// 		m_CurrentDate.GetDay());
// 	AfxMessageBox(st);

	m_dtOldMonthfocusDay = m_CurrentDate;

	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);
/*//NONO, for Calendar[2004_1026]
	m_Menu.LoadMenu(IDR_ORG_SCH_DAILY);
	m_DirtyFlag = FALSE;
*/
	if (!m_Image.LoadBitmap(IDB_SCH_MORE)) {
		TRACE0("ERROR: Where's IDB_SCH_MORE?\n");
	}
	m_Image.AssertValid();
	if (!m_CalendarImage.LoadBitmap(IDB_DATEPICKER_BUTTON)) {
		TRACE0("ERROR: Where's IDB_DATEPICKER_BUTTON?\n");
	}
	m_CalendarImage.AssertValid();

	//Nono_2004_0307
	if (!m_GotoTodayImage.LoadBitmap(IDB_SCH_GOTOTODAY_BUTTON)) {
		TRACE0("ERROR: Where's IDB_SCH_GOTOTODAY_BUTTON?\n");
	}
	m_GotoTodayImage.AssertValid();
	
	m_InfoRowCnt = 2;
	m_StartRowForDisplay = m_InfoRowCnt;
	m_RowsPerPage = 25;
	m_RealStartRow = m_StartRowForDisplay;

	m_nMaxDisplayRecordsPerDay = 0;

	SetAllRedraw();
	XCell::SetAlarmBmp(IDB_SCH_ALARM);
	XCell::SetRepeatBmp(IDB_SCH_REPEAT);
	XCell::SetAlarmGrayBmp(IDB_SCH_ALARM_GRAY);
	XCell::SetRepeatGrayBmp(IDB_SCH_REPEAT_GRAY);
	XCell::SetMoreBmp(IDB_SCH_MORE);
	XCell::SetMoreGrayBmp(IDB_SCH_MORE_GRAY);

	SetHalfDayString("AM","PM");
}
LRESULT CCAMMScheduleDailyTable::OnInvalidate(WPARAM wParam,LPARAM lParam)
{
	BOOL bErase = (BOOL) wParam; 
	int nRet;
	if(!m_bInitmemDC)
		InitMemDC();
	if(m_bCreateMemDCSuc)
	{
		if (bErase) 
		{
			SetAllRedraw();
			if (!m_bShowWindow) 
			{
				return 1;
			}
			switch (m_DatePeriod) {
			case DATEPERIOD_DAILY:
				m_memDC.FillRect(&m_rcMemDC,&m_brWhite);
				nRet = Draw_Daily(&m_memDC);
				break;
			case DATEPERIOD_WEEKLY:
				m_memDC.FillRect(&m_rcMemDC,&m_brWhite);
				nRet = Draw_Weekly(&m_memDC);
				break;
			case DATEPERIOD_MONTHLY:
				m_memDC.FillRect(&m_rcMemDC,&m_brWhite);
				nRet = Draw_Monthly(&m_memDC);
				break;
			}
			//SetAllRedraw();
		}
		Invalidate(FALSE);
	}
	else
	{
	//	if (bErase) 
			SetAllRedraw();
		Invalidate(FALSE);

	}
//	UpdateWindow();
	return 1;
}
void CCAMMScheduleDailyTable::xInvalidate(BOOL bErase)
{
	PostMessage(WM_INVALIDATEWND,(WPARAM)bErase,0);
}

void CCAMMScheduleDailyTable::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CWnd::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	m_bShowWindow = bShow;	
	SetAllRedraw();
	xInvalidate(TRUE);
}

void CCAMMScheduleDailyTable::SetScheduleData(SchedulesRecord *pScheduleRecord, int recordCount)
{   
	m_s3MonthInfo.SetScheduleData(pScheduleRecord,recordCount);
	m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);
}

void CCAMMScheduleDailyTable::GetScheduleData(SchedulesRecord *pScheduleRecord, int &recordCount)
{

	m_s3MonthInfo.GetScheduleData(pScheduleRecord,recordCount);

}

void CCAMMScheduleDailyTable::GetRecordCount(int &recordCount)
{

	m_s3MonthInfo.GetRecordCount(recordCount);

}

void CCAMMScheduleDailyTable::SetDirtyFlag(BOOL bDirty)
{
	m_DirtyFlag = bDirty;
}



void CCAMMScheduleDailyTable::OnOrgSchAdd() 
{
	if (xCheckRecordMaxLimit(1)) 
	{
		return;
	}

	CScheduleInfo si,si2;
	BOOL bSameDate = TRUE;
	if (m_DatePeriod == DATEPERIOD_DAILY) {
		//		int row,col;
		//		GetFocusCell(row,col);
		//		GetScheduleInfo(row - m_InfoRowCnt,si);
		m_sMonthInfo->GetFirstWorkingHourInfo(si);
	} else if (m_DatePeriod == DATEPERIOD_WEEKLY) {
		m_sMonthInfo->GetFirstWorkingHourInfo(si);
		//		m_sMonthInfo->GetFirstEmptyInfo(si);
	} else if (m_DatePeriod == DATEPERIOD_MONTHLY) {
		m_sMonthInfo->GetFirstWorkingHourInfo(si);
		//		m_sMonthInfo->GetFirstEmptyInfo(si);
	}
	if (xShowEditorDlg(si,si2,bSameDate)) 
	{
        //For Calendar
/*        CCAMMSync cs;
		if (!cs.AddCalendar(si2)) {
			AfxMessageBox("Failed to adding a new Calendar!\nPlease check the satus of connecting mobile.");
            return;
		}else {
			AfxMessageBox("Succeed to adding a new Calendar!.", MB_OK);
		}		
*/
		if (m_DatePeriod == DATEPERIOD_DAILY) {
			if (bSameDate) {
				CreateTable();
				//				CString title = si2.csTitle;
				//				SetText(si2.rowIndex + m_InfoRowCnt,2,title);
				//				//				JoinCells(si2.rowIndex + m_InfoRowCnt,2,si2.rowIndex + m_InfoRowCnt+si2.n30MinBlockCnt-1,2);
				//				JoinCells(si2.rowIndex + m_InfoRowCnt,2,si2.rowIndex + m_InfoRowCnt,2);
				//				SetBackColor(si2.rowIndex + m_InfoRowCnt,2,RGB(0xFF,0xFF,0xFF));
			} 
		} else if (m_DatePeriod == DATEPERIOD_WEEKLY) {
			//Do nothing
		} else if (m_DatePeriod == DATEPERIOD_MONTHLY) {
			//Do nothing
		}
	xInvalidate(TRUE);
	}
	else	xInvalidate(FALSE);

	//Added by Nono
	//((CCAMMScheduleDayView *)GetParent())->SetInfomationonStatusBar();
}

void CCAMMScheduleDailyTable::OnUpdateOrgSchAdd(CCmdUI* pCmdUI) 
{
	int row,col;
	int rc;
	if (GetFocusCell(row,col)) {
		if (1) {
			pCmdUI->Enable(TRUE);
		} else {
			m_s3MonthInfo.GetRecordCount(rc);
			if (rc >= g_nMAX_SCHEDULE_COUNT) {//Revise the limitation setting.[Nono,2004_0927]
			//if (rc >= MAX_SCHEDULE_COUNT) {
				pCmdUI->Enable(FALSE); 
			} else {
				CScheduleInfo si;
				if (m_DatePeriod == DATEPERIOD_DAILY) {
					GetScheduleInfo(row - m_InfoRowCnt,si);
					if (si.csTitle.IsEmpty()) {
						pCmdUI->Enable(TRUE); 
					} else {
						pCmdUI->Enable(FALSE); 
					}
				} else if (m_DatePeriod == DATEPERIOD_WEEKLY) {
					if (xGetUpdatedScheduleInfo(m_nSelectedIndexInOneDay,si)) {
						if (si.csTitle.IsEmpty()) {
							pCmdUI->Enable(TRUE); 
						} else {
							pCmdUI->Enable(FALSE); 
						}
					} else {
						pCmdUI->Enable(TRUE); 
					}
				} else if (m_DatePeriod == DATEPERIOD_MONTHLY) {
					if (xGetUpdatedScheduleInfo(m_nSelectedIndexInOneDay,si)) {
						if (si.csTitle.IsEmpty()) {
							pCmdUI->Enable(TRUE); 
						} else {
							pCmdUI->Enable(FALSE); 
						}
					} else {
						pCmdUI->Enable(TRUE); 
					}
				}
			}
		}
	} else {
		pCmdUI->Enable(FALSE); 
	}

}

//void CCAMMScheduleDailyTable::OnUpdateOrgSchAdd(CCmdUI* pCmdUI) 
//{
//	int row,col;
//	if (GetFocusCell(row,col)) {
//		CScheduleInfo si;
//		if (m_sMonthInfo->GetFirstEmptyInfo(si)) {
//			pCmdUI->Enable(TRUE); 
//		} else {
//			pCmdUI->Enable(FALSE); 
//		}
//	} else {
//		pCmdUI->Enable(FALSE); 
//	}
//
//}
BOOL CCAMMScheduleDailyTable::xGetSelectedItem(CScheduleInfo &si)
{
	if (m_DatePeriod == DATEPERIOD_DAILY) {
		int row,col;
		GetFocusCell(row,col);
		GetScheduleInfo(row - m_InfoRowCnt,si);
	} else if (m_DatePeriod == DATEPERIOD_WEEKLY) {
		xGetUpdatedScheduleInfo(m_nSelectedIndexInOneDay,si);
	} else if (m_DatePeriod == DATEPERIOD_MONTHLY) {
		xGetUpdatedScheduleInfo(m_nSelectedIndexInOneDay,si);
	}
	if (si.csTitle.IsEmpty()) {
		return FALSE;
	}
	return TRUE;
}

void CCAMMScheduleDailyTable::OnOrgSchEdit() 
{
	CScheduleInfo si,si2;
	BOOL bSameDate = TRUE;

	xGetSelectedItem(si); //new si also need

	if (xShowEditorDlg(si,si2,bSameDate)) {
//        CCAMMSync cs;
//        if ( cs.EditCalendar(si2) ) {
	    	if (m_DatePeriod == DATEPERIOD_DAILY) {
	    		CreateTable();
			} else if (m_DatePeriod == DATEPERIOD_WEEKLY) {
	      		CreateTable();
			} else if (m_DatePeriod == DATEPERIOD_MONTHLY) {
	    		CreateTable();
			}
//		}else {
//			AfxMessageBox("Failed to edit a new Calendar!\nPlease check the satus of connecting mobile.");
//		}
	xInvalidate(TRUE);
	}
	else	xInvalidate(FALSE);
}

BOOL CCAMMScheduleDailyTable::CheckIsDataSelected()
{
	return xCheckIsDataSelected();
}

BOOL CCAMMScheduleDailyTable::xCheckIsDataSelected() {
	int row,col;
	if (GetFocusCell(row,col)) {
		CScheduleInfo si;
		if (m_DatePeriod == DATEPERIOD_DAILY) {
			GetScheduleInfo(row - m_InfoRowCnt,si);
			if (si.csTitle.IsEmpty()) {
				return FALSE; //Always can Add
			} else {
				return TRUE; //Always can Add
			}
		} else if (m_DatePeriod == DATEPERIOD_WEEKLY) {
			if (xGetUpdatedScheduleInfo(m_nSelectedIndexInOneDay,si)) {
				if (si.csTitle.IsEmpty()) {
					return FALSE; //Always can Add
				} else {
					return TRUE; //Always can Add
				}
			} else {
				return FALSE; //Always can Add
			}
		} else if (m_DatePeriod == DATEPERIOD_MONTHLY) {
			if (xGetUpdatedScheduleInfo(m_nSelectedIndexInOneDay,si)) {
				if (si.csTitle.IsEmpty()) {
					return FALSE; //Always can Add
				} else {
					return TRUE; //Always can Add
				}
			} else {
				return FALSE; //Always can Add
			}
		}
	} else {
		return FALSE; //Always can Add
	}
	return FALSE;
}
void CCAMMScheduleDailyTable::OnUpdateOrgSchEdit(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(xCheckIsDataSelected());
}

BOOL CCAMMScheduleDailyTable::xShowEditorDlg(CScheduleInfo si, CScheduleInfo &si2,BOOL &bSameDate)
{
	//	COleDateTimeEx orgDate = si.dtStartDate;
	//	COleDateTimeEx newDate;
//	if (si.uiIndex == 0)
	if(atoi(si.szuiIndex) == 0)
	{
		int nH,nM;
		si.hiStartTime.Get24HourTime(nH,nM);
		si.haveAlarm = 0;
		si.dtAlarmDateTime.SetDateTime(si.dtStartDate.GetYear(),si.dtStartDate.GetMonth(),si.dtStartDate.GetDay(),nH,nM,0);
	}
	BOOL bNewScheduleInfo = FALSE;
	CScheduleInfo Originalsi = si;
	CScheduleInfo EditInfo = si;

	if(si.originalData == FALSE)
	{
		m_s3MonthInfo.GetRepeatOriginalInfo(Originalsi);
	}
	

	BOOL bOpenCurrent = TRUE;
	if(si.haveRepeat)
	{
		COpenrepeatQuestDlg  OpenrepeatQuestDlg;
		OpenrepeatQuestDlg.SetSubject(si.csTitle);
		if((OpenrepeatQuestDlg.DoModal()) == IDOK)
		{
			bOpenCurrent = OpenrepeatQuestDlg.m_bOpenCurrent;
		}
		else
			return FALSE;

	}

	if(!bOpenCurrent) 
		EditInfo = Originalsi;
//	else if(EditInfo.uiNodeType != GSM_CAL_BIRTHDAY)
	else
	{
		CCAMMSync cammsync;
		NoteTypeSupportInfo SupportInfo;
		ZeroMemory(&SupportInfo,sizeof(NoteTypeSupportInfo));
		cammsync.GetNoteTypeSupportInfo(EditInfo.uiNodeType,&SupportInfo);
		if(SupportInfo.nRepeatdefaulType == 0)
			EditInfo.haveRepeat = 0;
	}

	//kerm add for 9a9u priority 1.07
	if(EditInfo.uiPrivate == 0)
		EditInfo.uiPrivate = 2;
	

	m_HitSameCell = FALSE;
	CCAMMScheduleEditorDlg sdDlg(this);// = new CCAMMScheduleEditorDlg();
	sdDlg.SetScheduleInfo(EditInfo);
	sdDlg.SetMonthInfoPtr(m_sMonthInfo);
	//		sdDlg.SetCheckConflictFunc(lpCallBackFunc);
	if ((sdDlg.DoModal()) == IDOK) 
	{
		//		newDate = m_sMonthInfo->GetCurrentDate();

		//Clear original data in memory and screen
		//		m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(si.dtStartDate);
		//		m_sMonthInfo->SetScheduleDate(si.dtStartDate);
		//		m_sMonthInfo->RemoveScheduleInfo(si);
		if(si.haveRepeat && bOpenCurrent)
			bNewScheduleInfo = TRUE;

		if(!bNewScheduleInfo)
			RemoveScheduleInfo(si);

		//Save new data
		sdDlg.GetScheduleInfo(si2);
		if(bNewScheduleInfo)
		{
		//	si2.uiIndex = 0;
			itoa(0,si2.szuiIndex,10);
			si2.originalData = TRUE;
		}

		//		newDate = si2.dtStartDate;
		//		m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(newDate);
		//		m_sMonthInfo->SetScheduleDate(newDate);
		si2.bScheduleStart = TRUE;
			
	/*	DWORD dwUpdateFlag = si.GetDifferFlag(si2);
		
		Originalsi.UpdateData(si2,dwUpdateFlag);

		si2 = Originalsi;
*/
		// ** transfer calendar to Mobile ** //
        CCAMMSync cs;
//		if (!si2.uiIndex) {
		if (strlen(si2.szuiIndex) <=0 || atoi(si2.szuiIndex) == 0) {
			if (Anw_SUCCESS != cs.AddCalendar(si2)) {
				//if cannot transfer to Mobile, stop adding new calendar into UI.
	        	return FALSE;
			}
		}else {
			if (Anw_SUCCESS != cs.EditCalendar(si2)) {
				//if cannot transfer to Mobile, stop updating his calendar in UI.
	        	return FALSE;
			}
		}
		// ** transfer calendar to Mobile ** //


		AddScheduleInfo(si2);
		m_DirtyFlag = TRUE;
		//		m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(orgDate);
		//		m_sMonthInfo->SetScheduleDate(orgDate);
		
		bSameDate = (si.dtStartDate == si2.dtStartDate);
    	GetRecordCount(g_numTotalCalendars);
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(), WM_REFRESH_LEFT_PANEL, (DWORD)true, NULL);

/*
		//Added by Nono
		((CCAMMScheduleDayView *)GetParent())->SetInfomationonStatusBar();
		if (si2.haveAlarm)
    		((CCAMMScheduleDayView *)GetParent())->SetAlert(si2);
*/
		
		return TRUE;
	}

	return FALSE;
}

void CCAMMScheduleDailyTable::OnMove(int x, int y) 
{
	CWnd::OnMove(x, y);
	
	// TODO: Add your message handler code here
	
}

BOOL CCAMMScheduleDailyTable::xShowCalendar(CPoint point,COleDateTimeEx &currentDT)
{
	CCAMMScheduleCalendar sc;

	m_HitSameCell = FALSE;
	COleDateTimeEx ori = currentDT;
	sc.SetCurrentDate(currentDT);
	ClientToScreen(&point);
	sc.SetPos(point);
	sc.DoModal();
	sc.GetSelectedDate(currentDT);

	if((int)ori.m_dt == (int)currentDT.m_dt)
		return FALSE;

	//	CCAMMScheduleCalendar* sc = new CCAMMScheduleCalendar();
	//	if (sc != NULL) {
	//		sc->SetCurrentDate(currentDT);
	//		BOOL ret = sc->Create(CCAMMScheduleCalendar::IDD,this);
	//		if(!ret)   //Create failed.
	//			AfxMessageBox("Error creating Dialog");
	//		ClientToScreen(&point);
	//		//		sc->SetPos(point);
	//		sc->SetWindowPos(NULL,point.x,point.y,0,0,SWP_NOSIZE);
	//		sc->ShowWindow(SW_SHOW);
	//	} else {
	//      AfxMessageBox("Error Creating Dialog Object");
	//	}

	return TRUE;
}

void CCAMMScheduleDailyTable::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
	SetAllRedraw();
	xInvalidate(TRUE);
}

BOOL CCAMMScheduleDailyTable::xCheckRecordMaxLimit(int newCnt)
{
	int rc=0;
	m_s3MonthInfo.GetRecordCount(rc);
	rc += newCnt;
	if (rc > g_nMAX_SCHEDULE_COUNT ) {//Revise the limitation setting.[Nono,2004_0927]
	//if (rc >= MAX_SCHEDULE_COUNT) {
	/*	CString s = LoadStringFromFile(ghSkinResource,IDS_ORG_RECORD_LIMIT);
		CString msg;
		msg.Format((LPCTSTR)s,g_nMAX_SCHEDULE_COUNT);//Revise the limitation setting.[Nono,2004_0927]
		//msg.Format((LPCTSTR)s,MAX_SCHEDULE_COUNT);
		AfxMessageBox(msg);
		return TRUE;*/
		CString  s = LoadStringFromFile(_T("message"),_T("MSG_RECORD_LIMIT"));
		CString msg;
		msg.Format(s,g_nMAX_SCHEDULE_COUNT);
		CalendarStringMessageBox(m_hWnd,msg);
	}
	return FALSE;
}

int CCAMMScheduleDailyTable::xDraw_Background(CDC *pDC,XCell& cell,RECT bkRect)
{
	cell.DrawBackground(pDC,bkRect);
	return 0;
}


BOOL CCAMMScheduleDailyTable::xCopyDataToClipboard(CString cpStr) {
    HGLOBAL hglbCopy; 
    LPTSTR  lptstrCopy; 

    if (!::OpenClipboard(afxGetMainWnd()->GetSafeHwnd())) {
        return FALSE; 
	}
    ::EmptyClipboard(); 
	
    hglbCopy = GlobalAlloc(GMEM_MOVEABLE,(_tcslen((LPCTSTR)cpStr)+1) * sizeof(TCHAR)); 
    if (hglbCopy == NULL) 
    { 
        ::CloseClipboard(); 
        return FALSE; 
    } 

    lptstrCopy = (LPTSTR)GlobalLock(hglbCopy); 
    memcpy(lptstrCopy, (LPCTSTR)cpStr, (_tcslen((LPCTSTR)cpStr)) * sizeof(TCHAR)); 
    lptstrCopy[_tcslen((LPCTSTR)cpStr)] = (TCHAR) 0;    // null character 
    GlobalUnlock(hglbCopy); 

    // Place the handle on the clipboard. 

    ::SetClipboardData(m_ClipboardFormatId, hglbCopy); 
    ::CloseClipboard(); 

	return TRUE;
}

BOOL CCAMMScheduleDailyTable::xGetDataFromClipboard(CString &cpStr) {
    HGLOBAL hglb; 
    LPTSTR    lptstr; 

    if (!::OpenClipboard(afxGetMainWnd()->GetSafeHwnd())) {
        return FALSE; 
	}
    if (!::IsClipboardFormatAvailable(m_ClipboardFormatId)) {
		return FALSE;
	}
	
    hglb = ::GetClipboardData(m_ClipboardFormatId); 
    if (hglb != NULL) 
    { 
        lptstr = (LPTSTR)GlobalLock(hglb); 
        if (lptstr != NULL) 
        { 
            // Call the application-defined ReplaceSelection 
            // function to insert the text and repaint the 
            // window. 
			cpStr = lptstr;
            GlobalUnlock(hglb); 
        } else {
			return FALSE;
		}
    } 
    ::CloseClipboard(); 

	return TRUE;
}

void CCAMMScheduleDailyTable::OnOrgSchCut() 
{
	CString cpStr;
	if (xGetSelectedClipboardString(cpStr)) {
		if (xCopyDataToClipboard(cpStr)) {
			xDeleteScheduleItem();
		}
	}
}

void CCAMMScheduleDailyTable::xDeleteScheduleItem() 
{
	CScheduleInfo si;

	if (xGetSelectedItem(si)) {
		RemoveScheduleInfo(si);
		
		//Calendar
	//	int nIndexDeleted = (int) si.uiIndex;

		CCAMMSync sc;
		if ( !(sc.DeleteCalendar(si,Type_Start)) ) {
		//	sc.DeleteCalendar(si,Type_End);
			return;
		}
		sc.DeleteCalendar(si,Type_End);
/*		int ret = ANWDeleteScheduleData(nIndexDeleted, NULL);
    	if (Anw_SUCCESS != ret) {
			CString msg;
			msg.Format("Failed to delete Schedule-Data!! [ErrorCode: %d ]", ret);
			AfxMessageBox(msg);
		}
*/

		//

		if (m_DatePeriod == DATEPERIOD_DAILY) {
			xClearTable();
			CreateTable();
			focusRow = -1;
			focusCol = -1;
			m_DirtyFlag = TRUE;
		} else if (m_DatePeriod == DATEPERIOD_WEEKLY) {
			xClearTable();
			CreateTable();
			m_DirtyFlag = TRUE;
		} else if (m_DatePeriod == DATEPERIOD_MONTHLY) {
			xClearTable();
			CreateTable();
			m_DirtyFlag = TRUE;
		}
	}
   	//Added by Nono =>
   	//((CCAMMScheduleDayView *)GetParent())->SetInfomationonStatusBar();

	//REFRESH LEFT_PANEL to change the totla number of calendars displayed on left view.
	if (m_bMobileConnected == TRUE) {
//		CCAMMSync sc;
  //  	g_numTotalCalendars = sc.GetCalendarCount();
     //   ::SendMessage(afxGetMainWnd()->GetSafeHwnd(), WM_REFRESH_LEFT_PANEL, (DWORD)true, NULL);
    	GetRecordCount(g_numTotalCalendars);
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(), WM_REFRESH_LEFT_PANEL, (DWORD)true, NULL);
	}
   	//Added by Nono <=
}

void CCAMMScheduleDailyTable::OnUpdateOrgSchCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(xCheckIsDataSelected());
}

void CCAMMScheduleDailyTable::OnOrgSchCopy() 
{
	CString cpStr;
	if (xGetSelectedClipboardString(cpStr)) {
		xCopyDataToClipboard(cpStr);
	}
}

void CCAMMScheduleDailyTable::OnUpdateOrgSchCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(xCheckIsDataSelected());
}

void CCAMMScheduleDailyTable::OnOrgSchPaste() 
{
	CString cpStr;
	if (xGetDataFromClipboard(cpStr)) {
		if (!xPasteData(cpStr)) {
			AfxMessageBox(_T("Can't paste data"));
		}
	}
}

void CCAMMScheduleDailyTable::OnUpdateOrgSchPaste(CCmdUI* pCmdUI) 
{
    if (!::OpenClipboard(afxGetMainWnd()->GetSafeHwnd())) {
		pCmdUI->Enable(FALSE);
        return; 
	}
    if (::IsClipboardFormatAvailable(m_ClipboardFormatId)) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
    ::CloseClipboard(); 
}

BOOL CCAMMScheduleDailyTable::xGetSelectedClipboardString(CString &cpStr) {
	CScheduleInfo si;
	if (xGetSelectedItem(si)) {
		cpStr = Object_Seperator;
		cpStr += si.GetClipboardString();
		cpStr += Object_Seperator;
		return TRUE;
	}
	
	return FALSE;
}

BOOL CCAMMScheduleDailyTable::xPasteData(CString &cpStr) {

	CScheduleInfo si,si2;
	CString sep = Object_Seperator;
	int pos=0,pos2=0;
	int idx=0;

	pos2 = cpStr.Find(sep,pos);
	if (pos2 < 0) {
		return FALSE;
	}
	pos2 += sep.GetLength();
	if (pos2 >= cpStr.GetLength()) {
		return FALSE;
	}
	pos = pos2;
	pos2 = cpStr.Find(sep,pos);
	if (pos2 == -1) {
		return FALSE;
	}
	//	if (cpStr.Right(pos2) != sep) {
	//		return FALSE;  //when more than one schedule data in clipboard
	//	}
	xGetSelectedItem(si);
	si2.ParseClipboardString(cpStr.Mid(pos,pos2-pos));
	if ((m_DatePeriod == DATEPERIOD_WEEKLY) || (m_DatePeriod == DATEPERIOD_MONTHLY)) {
		si.hiRealStartTime = si2.hiRealStartTime;
		si.hiStartTime = si2.hiStartTime;
		si.hiRealStopTime = si2.hiRealStopTime;
		si.hiStopTime = si2.hiStopTime;
		si.n30MinBlockCnt = si2.n30MinBlockCnt;
		si.rowIndex = si2.rowIndex;
	}
	si.MergeAndValidateSchedule(si2);

	pos2 += sep.GetLength();
	if (pos2 > cpStr.GetLength()) {
		return TRUE;
	}

	if (xCheckRecordMaxLimit(1)) {
		return FALSE;
	}

	AddScheduleInfo(si);
	
	CreateTable();

	SetDirtyFlag(TRUE);
	xInvalidate(TRUE);

	return TRUE;
}

UINT CCAMMScheduleDailyTable::GetClipboardFormatID()
{
	return m_ClipboardFormatId;
}


BOOL CCAMMScheduleDailyTable::CreateStatusBar()
{
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return FALSE;       // fail to create
	}

	UINT nID, nStyle;
	int cxWidth;

	m_wndStatusBar.GetPaneInfo( 0, nID, nStyle, cxWidth);
	m_wndStatusBar.SetPaneInfo( 0, nID, SBPS_STRETCH|SBPS_NORMAL, cxWidth);
/*
	m_wndStatusBar.SetPaneText(0," Contact: ");
	m_wndStatusBar.SetPaneText(1," Schedule: ");
	m_wndStatusBar.SetPaneText(2," Memorandum: ");
*/
	return TRUE;
}

BOOL CCAMMScheduleDailyTable::xSetInfomationonStatusBar(void)
{
	//Set the information of "Contact", "Schedule" & "Memorandum".[Nono, 2004_0413]
	int nScheduleRecordCount = 0;
	int nMemoRecordCount = 0;
	int nPhoneRecordCount = 0;
	CString str;

    //str.Format("%s%d", " ヘ玡筿杠魁羆计[Total contact]:  ", nMemoRecordCount);
    str.Format(_T("%s%d"), " [Total contact]:  ", nMemoRecordCount);
	m_wndStatusBar.SetPaneText(0,str.GetBuffer(20));

	GetRecordCount(nScheduleRecordCount);
    //str.Format("%s%d", " ヘ玡︽祘羆计[Total schedule]:  ", nScheduleRecordCount);
    str.Format(_T("%s%d"), " [Total schedule]:  ", nScheduleRecordCount);
	m_wndStatusBar.SetPaneText(1,str.GetBuffer(20), TRUE);

    //str.Format("%s%d", " ヘ玡癘ㄆ羆计[Total memorandum]:  ", nMemoRecordCount);
    str.Format(_T("%s%d"), " [Total memorandum]:  ", nMemoRecordCount);
	m_wndStatusBar.SetPaneText(2,str.GetBuffer(20));
	str.ReleaseBuffer();

	return 0;
}


/*
int CCAMMScheduleDailyTable::PrintRecord()
{
    AutoSizeColumns();
	
	if (m_pPrintRecord) {
		ASSERT(m_ScheduleList);
		ASSERT(m_ScheduleList->m_ScheduleList);
		m_pPrintRecord->SetListCtrl(m_ScheduleList->m_ScheduleList);
		m_pPrintRecord->SetListView(m_ScheduleList->m_ScheduleList);
		m_pPrintRecord->SetAppName(AfxGetAppName());
		CString cuDocName;
		cuDocName = "[ ";
		cuDocName += _DocTitle_;
		cuDocName += " ]";
		m_pPrintRecord->SetDocTitle((LPCTSTR)cuDocName);
    	OnFilePrintPreview();

		return 1;
	}

	return 0;
}
*/
//Added by Nono <=

void CCAMMScheduleDailyTable::SetScrollBarPosition(int nRow)
{
	if(m_sMonthInfo)
	{
		int nDataRowCnt = 0;
		m_sMonthInfo->GetCurrentDayRowCountBeforeTime(nRow,nDataRowCnt);
		int nGotoRow = nRow + nDataRowCnt;//+ m_InfoRowCnt;
	//	if(nRow < m_StartRowForDisplay || nRow> m_StartRowForDisplay+m_RowsPerPage)
		if(nGotoRow < m_StartRowForDisplay || nGotoRow> m_StartRowForDisplay+m_RowsPerPage)
		{
	//		m_ScrollBarPos = nRow;
	//		m_StartRowForDisplay = nRow;
	//		SetScrollPos(SB_VERT,m_ScrollBarPos,TRUE);
		//	if (nGotoRow >= m_MaxRowForDisplay) 
		//		nGotoRow = m_MaxRowForDisplay;
			OnVScroll(SB_THUMBTRACK, nGotoRow,GetScrollBarCtrl(SB_VERT)) ;
		}
	}

}
BOOL CCAMMScheduleDailyTable::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return FALSE;
	return CWnd::OnEraseBkgnd(pDC);
}

void CCAMMScheduleDailyTable::GotoDailyView()
{
	if (m_bGotoDaily)
	{
		m_bGotoDaily = FALSE;
//		ChangeDatePeriod(DATEPERIOD_DAILY);
		CMainFrame *pMain = (CMainFrame *)afxGetMainWnd();
		::SendMessage(pMain->m_wndSplitter.GetPane(0,1)->GetSafeHwnd(),WM_SWITHTODAYVIEW,0,0);
	} 

}

void CCAMMScheduleDailyTable::InitMemDC()
{
	if(!m_bInitmemDC)
	{
		CDC *pDC = GetDC();
		CBitmap*	oldBitmap;
		m_rcMemDC.SetRect(0,0,2480,1860);
		m_bCreateMemDCSuc = m_memDC.CreateCompatibleDC(pDC);
		if(m_bCreateMemDCSuc)
		{
			m_bCreateMemDCSuc = m_bitmap.CreateCompatibleBitmap(pDC, 2480, 1860);
			if(m_bCreateMemDCSuc)
			{
				oldBitmap = m_memDC.SelectObject(&m_bitmap);
				m_memDC.FillRect(&m_rcMemDC,&m_brWhite);
			}
//			m_bCreateMemDCSuc = FALSE;
		}
		ReleaseDC(pDC);
		m_bInitmemDC = TRUE;
	}

}

void CCAMMScheduleDailyTable::UninitMemDC()
{
	if(m_bInitmemDC)
	{
		if(m_bitmap.GetSafeHandle())
			m_bitmap.DeleteObject();
		m_bInitmemDC = FALSE;
	}

}

void CCAMMScheduleDailyTable::xInvalidatecell(int row, int Col)
{
	CDC *pMemeDC;
	CRect rect,rectTitle;
	rectTitle.SetRect(0,0,0,0);
	BOOL bDrawSubItem = FALSE;
	if(m_bCreateMemDCSuc)
	{
		pMemeDC = &m_memDC;

		switch (m_DatePeriod) {
		case DATEPERIOD_DAILY:
			rect = DrawCell_Daily(pMemeDC,row,Col);
			break;
		case DATEPERIOD_WEEKLY:
			rect = DrawCell_Weekly(pMemeDC,row,Col,bDrawSubItem);
			rectTitle = DrawCell_Weekly(pMemeDC,row-1,Col,bDrawSubItem);
			break;
		case DATEPERIOD_MONTHLY:
			rect =DrawCell_Monthly(pMemeDC,row,Col,bDrawSubItem);
			rectTitle = DrawCell_Monthly(pMemeDC,row-1,Col,bDrawSubItem);
			break;
		}
		if(!rect.IsRectEmpty())
		{
			rect.InflateRect(1,1,1,1);
			InvalidateRect(&rect,FALSE);
		}
		if(!rectTitle.IsRectEmpty())
		{
			rectTitle.InflateRect(1,1,1,1);
			InvalidateRect(&rectTitle,FALSE);
		}
	}
	else
	{
//		xInvalidate(FALSE);
		pMemeDC = GetDC();

		switch (m_DatePeriod) {
		case DATEPERIOD_DAILY:
			rect = DrawCell_Daily(pMemeDC,row,Col);
			break;
		case DATEPERIOD_WEEKLY:
			rect = DrawCell_Weekly(pMemeDC,row,Col,bDrawSubItem);
			rectTitle = DrawCell_Weekly(pMemeDC,row-1,Col,bDrawSubItem);
			break;
		case DATEPERIOD_MONTHLY:
			rect =DrawCell_Monthly(pMemeDC,row,Col,bDrawSubItem);
			rectTitle = DrawCell_Monthly(pMemeDC,row-1,Col,bDrawSubItem);
			break;
		}
		ReleaseDC(pMemeDC);
	}


}
CRect CCAMMScheduleDailyTable::DrawCell_Weekly(CDC *pDC ,int i, int j,BOOL& bDrawSubItem)
{
	CRect rect;
	rect.SetRect(0,0,0,0);
	if(i<0 || j<0)
		return rect;
	XCell& cell = cells[i*MAX_COLS1+j];
	BOOL bRedraw = TRUE;
	int nRecordsCount;
	CString strText[48];
	BOOL bAlarm[48];
	BOOL bRepeat[48];
	int strCount=0;
	BOOL bMore = FALSE;
	BOOL bDrawBG=TRUE;
	BOOL bDrawWholeBG=FALSE;
//	bMore = FALSE;
//	bDrawBG = TRUE;

	if (cell.colSpan !=0 && cell.rowSpan != 0)
	{
		bRedraw = m_bRedraw[xGetWeeklyIndex(i,j)];
		if (bRedraw)
		{
			if ((i % 2) == 1) 
			{
				COleDateTimeEx dt;
				COleDateTimeSpan dts;
				CString strInfo;
				dts.SetDateTimeSpan(xGetWeeklyIndex(i,j),0,0,0);
				dt = m_WeeklyStartDate + dts;
				m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(dt);

				BOOL bWeekend = (dts.GetDays() >= 5);
				strInfo = xGetWeeklyScheduleTitles(m_sMonthInfo,dt,bWeekend,nRecordsCount,strText,strCount,bAlarm,bRepeat);
				SetText(i,j,strText,strCount);
				SetAlarm(i,j,bAlarm,strCount);
				SetRepeat(i,j,bRepeat,strCount);
				m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);
				
				int maxDisplay = bWeekend ? m_nMaxDisplayWeekendRecordsPerDay : m_nMaxDisplayRecordsPerDay;
				bMore = (nRecordsCount > 0) && (nRecordsCount > maxDisplay) ;
				m_bMore[dts.GetDays()] = bMore;

				if ((focusRow == i) && (focusCol == j))
					bDrawSubItem = (m_nSelectedIndexInOneDay >= 0) && (m_nSelectedIndexInOneDay < nRecordsCount);
			//	bDrawBG = FALSE;
				if (!bRedraw || bDrawWholeBG)
					bDrawBG = FALSE;
			}
	
			if (m_HitSameCell && (((i) % 2) == 0)) 
				return rect;
			rect = GetRect(i,j);
			
			BITMAP bitmap;
			m_Image.GetBitmap(&bitmap);
			RECT moreRect;
			moreRect.left = rect.right - bitmap.bmWidth;
			moreRect.top = rect.bottom - bitmap.bmHeight;
			moreRect.right = moreRect.left + bitmap.bmWidth;
			moreRect.bottom = moreRect.top + bitmap.bmHeight;
			if (bRedraw)
			{
				if (i+j == 0) 
				{
					BITMAP bitmap;
					m_CalendarImage.GetBitmap(&bitmap);
					cell.Draw(pDC,rect,ALIGN_LEFT,m_CalendarImage);
				} 
				else 
					cell.Draw(pDC,rect,bMore,moreRect,m_Image,bDrawBG);
			}
		}
	}
	if((focusRow == i || focusRow-1 == i ) && focusCol == j)
	{
		if ((focusRow != 0) && (focusRow != 2) && (focusRow != 4) && (focusRow != 6) && !m_HitSameCell) 
		{
			RECT rtHL = GetRect (focusRow-1, focusCol);
			COLORREF orgBkColor = 	GetCells (focusRow-1, focusCol)->GetBackColor();
			COLORREF orgTextColor = GetCells (focusRow-1, focusCol)->GetTextColor();
			GetCells (focusRow-1, focusCol)->SetBackColor(HIGH_LIGHT_BG_COLOR);
			GetCells (focusRow-1, focusCol)->SetTextColor(HIGH_LIGHT_TEXT_COLOR);
			if ((focusRow-1)+focusCol == 0)
			{
				BITMAP bitmap;
				m_CalendarImage.GetBitmap(&bitmap);
				GetCells (focusRow-1, focusCol)->Draw(pDC,rtHL,ALIGN_LEFT,m_CalendarImage);
			} 
			else 
				GetCells (focusRow-1, focusCol)->Draw(pDC,rtHL,FALSE,rtHL,m_Image,TRUE);
			GetCells (focusRow-1, focusCol)->SetBackColor(orgBkColor);
			GetCells (focusRow-1, focusCol)->SetTextColor(orgTextColor);
		}

		if ((focusRow != 0) && (focusRow != 2) && (focusRow != 4) && (focusRow != 6)) 
		{
			RECT rect = GetRect (focusRow, focusCol);
			if (bDrawSubItem)
			{
				rect.top += m_nSelectedIndexInOneDay * m_nHeightPerLineInOneDay;
				rect.bottom = rect.top + m_nHeightPerLineInOneDay;
				GetCells (focusRow, focusCol)->DrawHitBorder(pDC, rect, RGB(255, 0, 20));
			}
		}
	}
	return rect;
}
CRect CCAMMScheduleDailyTable::DrawCell_Monthly(CDC *pDC ,int i, int j,BOOL& bDrawSubItem)
{
	CRect rect;
	rect.SetRect(0,0,0,0);
	if(i<0 || j<0)
		return rect;
	int nRecordsCount;
	CString strText[48];
	BOOL bAlarm[48];
	BOOL bRepeat[48];
	int strCount=0;
	BOOL bMore = FALSE;
	BOOL bDrawBG=TRUE;
	XCell& cell = cells[i*MAX_COLS1+j];
	BOOL bRedraw;
	COleDateTimeEx dt;
//	bMore = FALSE;
//	bDrawBG = TRUE;

	DWORD dw = ::GetTickCount();
			
	if (cell.colSpan !=0 && cell.rowSpan != 0)
	{
		bRedraw = m_bRedraw[xGetMonthlyIndex(i,j)];
		if (bRedraw) 
		{
			if (((i-1) % 2) == 1)
			{
				COleDateTimeSpan dts;
				CString strInfo;
				int ts = xGetMonthlyIndex(i,j);//(i-1)%4==3?((i-3) / 4)*7 + j + 1:((i-1) / 4)*7 + j;
				dts.SetDateTimeSpan(ts,0,0,0);
				dt = m_MonthlyStartDate + dts;

				if (!((dt.GetYear() == m_sMonthInfo->CurrentYear) && (dt.GetMonth() == m_sMonthInfo->CurrentMonth))) 
					m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(dt);
				m_sMonthInfo->SetScheduleDate(dt);
				
				BOOL bWeekend = (j == 5);
				strInfo = xGetMonthlyScheduleTitles(m_sMonthInfo,dt,bWeekend,nRecordsCount,strText,strCount,bAlarm,bRepeat);
				SetText(i,j,strText,strCount);
				SetAlarm(i,j,bAlarm,strCount);
				SetRepeat(i,j,bRepeat,strCount);
				//						m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(m_CurrentDate);
				
				int maxDisplay = bWeekend ? m_nMaxDisplayWeekendRecordsPerDay : m_nMaxDisplayRecordsPerDay;
				bMore = (nRecordsCount > 0) && (nRecordsCount > maxDisplay) ;
				m_bMore[dts.GetDays()] = bMore;
				
				if ((focusRow == i) && (focusCol == j)) 
					bDrawSubItem = (m_nSelectedIndexInOneDay >= 0) && (m_nSelectedIndexInOneDay < min(nRecordsCount,maxDisplay));
			}
		}
			
		if (m_HitSameCell && (((i-1) % 2) == 0)) 
			return rect;
		
		rect = GetRect(i,j);
		BITMAP bitmap;
		m_Image.GetBitmap(&bitmap);
		RECT moreRect;
		moreRect.left = rect.right - bitmap.bmWidth;
		moreRect.top = rect.bottom - bitmap.bmHeight;
		moreRect.right = moreRect.left + bitmap.bmWidth;
		moreRect.bottom = moreRect.top + bitmap.bmHeight;
		if (bRedraw) 
		{
			if (i+j == 0)
			{
				BITMAP bitmap;
				m_CalendarImage.GetBitmap(&bitmap);
				cell.Draw(pDC,rect,ALIGN_LEFT,m_CalendarImage);
			} else 
				cell.Draw(pDC,rect,bMore,moreRect,m_Image,bDrawBG);
		}
	} //if (cell.colSpan !=0 && cell.rowSpan != 0)
	if((focusRow == i || focusRow-1 == i ) && focusCol == j)
	{
		if ((focusRow > 0) && !m_HitSameCell)
		{
			RECT rtHL = GetRect (focusRow-1, focusCol);
			COLORREF orgBkColor = 	GetCells (focusRow-1, focusCol)->GetBackColor();
			COLORREF orgTextColor = GetCells (focusRow-1, focusCol)->GetTextColor();
			GetCells (focusRow-1, focusCol)->SetBackColor(HIGH_LIGHT_BG_COLOR);
			GetCells (focusRow-1, focusCol)->SetTextColor(HIGH_LIGHT_TEXT_COLOR);
			if ((focusRow-1)+focusCol == 0) {
				BITMAP bitmap;
				m_CalendarImage.GetBitmap(&bitmap);
				GetCells (focusRow-1, focusCol)->Draw(pDC,rtHL,ALIGN_LEFT,m_CalendarImage);
			} else {
				GetCells (focusRow-1, focusCol)->Draw(pDC,rtHL,FALSE,rtHL,m_Image,TRUE);
			}
			GetCells (focusRow-1, focusCol)->SetBackColor(orgBkColor);
			GetCells (focusRow-1, focusCol)->SetTextColor(orgTextColor);
		}
		if ((focusRow != 0)) 
		{
			RECT rect = GetRect (focusRow, focusCol);
			if (bDrawSubItem) 
			{
				rect.top += m_nSelectedIndexInOneDay * m_nHeightPerLineInOneDay;
				rect.bottom = rect.top + m_nHeightPerLineInOneDay;
				GetCells (focusRow, focusCol)->DrawHitBorder(pDC, rect, RGB(255, 0, 20));
			}
		}
	}
	return rect;
}

CRect CCAMMScheduleDailyTable::DrawCell_Daily(CDC *pDC ,int i, int j)
{

	CString hourStr,halfDayStr;
	BOOLEAN bFirst30Min;
	int InvisibleHeight = GetRowsHeight(0,m_StartRowForDisplay-1) - GetRowsHeight(0,m_InfoRowCnt-1);
	InvisibleHeight = InvisibleHeight == -1?0:InvisibleHeight;

	CRect rect;
	rect.SetRect(0,0,0,0);
	if(i<0 || j<0)
		return rect;
	XCell& cell = cells[i*MAX_COLS1+j];
	GetHourInfo(i,hourStr,halfDayStr,bFirst30Min);
	if ((bFirst30Min) && (j==1))
	{
		SetText(i,1,halfDayStr);
	}

/*	if( j == 0 || j ==1)
	{
//		GetHourInfo(i,hourStr,halfDayStr,bFirst30Min);
	//	if ((bFirst30Min) && (j==1))
	//	{
	//		SetText(i,1,halfDayStr);
	//	}
		if(j ==1 &&  && i%2 == 0)
		{
			rect.top = GetRowsHeight(0, i-1)+1;
			rect.left = GetColsWidth(0, j-1)+1;

			rect.bottom = rect.top + GetRowsHeight (i, i );
			rect.right = rect.left + GetColsWidth (j, j );
		}
		else
			rect = GetRect(i,j);

		rect.top -= InvisibleHeight;
		rect.bottom -= InvisibleHeight;
		if(!rect.IsRectEmpty())
			cell.Draw(pDC,rect);

	}*/

	if ((cell.colSpan !=0 && cell.rowSpan != 0) /*&& j!=0*/)
	{


		rect = GetRect(i,j);	
		rect.top -= InvisibleHeight;
		rect.bottom -= InvisibleHeight;
	/*	if ((i == m_RealStartRow)) 
		{
			RECT headerRect = GetRect(m_InfoRowCnt-1,2);
			rect.top = headerRect.bottom;
			RECT txtRt;
			cell.CalcTextRect(pDC,&txtRt);
			if ((txtRt.bottom-txtRt.top) > (rect.bottom - rect.top)) {
				rect.top = rect.bottom - (txtRt.bottom-txtRt.top);
			}
		}*/
		cell.Draw(pDC,rect);
	} 
	else if ((i == m_RealStartRow)/* && j!=0 && j!=1*/) 
	{
		int row = i;
		while (cells[row*MAX_COLS1+j].colSpan ==0 || cells[row*MAX_COLS1+j].rowSpan == 0) 
		{
			row--;
			if (row < m_InfoRowCnt)
				break;
		}
		GetHourInfo(row,hourStr,halfDayStr,bFirst30Min);
		if ((bFirst30Min) && (j==1)) 
			SetText(row,1,halfDayStr);

		rect = GetRect(row,j);	
		rect.top -= InvisibleHeight;
		rect.bottom -= InvisibleHeight;
		RECT headerRect = GetRect(m_InfoRowCnt-1,2);
		rect.top = headerRect.bottom;
		RECT txtRt;
		cells[row*MAX_COLS1+j].CalcTextRect(pDC,&txtRt);
		if ((txtRt.bottom-txtRt.top) > (rect.bottom - rect.top))
			rect.top = rect.bottom - (txtRt.bottom-txtRt.top);
		cells[row*MAX_COLS1+j].Draw(pDC,rect);
	} 
	if ((focusRow == i) && (focusCol ==j)) {
		RECT rectfocus = GetRect (focusRow, focusCol);
		rectfocus.top -= InvisibleHeight;
		rectfocus.bottom -= InvisibleHeight;
		//		rect.bottom -= 5;
		//		rect.right -= 18;
		GetCells (focusRow, focusCol)->DrawHitBorder(pDC, rectfocus, RGB(255, 0, 20));
	}

	return rect;

}
/*
void CCAMMScheduleDailyTable::DrawFocusBorder()
{

	CDC *pMemeDC;
	CRect rect;
	BOOL bDrawSubItem = FALSE;
	switch (m_DatePeriod) {
	case DATEPERIOD_DAILY:
		{
			pMemeDC = &m_memDC[0];
			int InvisibleHeight = GetRowsHeight(0,m_StartRowForDisplay-1) - GetRowsHeight(0,m_InfoRowCnt-1);
			InvisibleHeight = InvisibleHeight == -1?0:InvisibleHeight;
			rect = GetRect (focusRow, focusCol);
			rect.top -= InvisibleHeight;
			rect.bottom -= InvisibleHeight;
			GetCells (focusRow, focusCol)->DrawHitBorder(pMemeDC, rect, RGB(255, 0, 20));
		}
		break;
	case DATEPERIOD_WEEKLY:
		{
			pMemeDC = &m_memDC[1];
			COleDateTimeEx dt;
			COleDateTimeSpan dts;
			CString strInfo;
			dts.SetDateTimeSpan(xGetWeeklyIndex(focusRow,focusCol),0,0,0);
			dt = m_WeeklyStartDate + dts;
			m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(dt);

			BOOL bWeekend = (dts.GetDays() >= 5);
			strInfo = xGetWeeklyScheduleTitles(m_sMonthInfo,dt,bWeekend,nRecordsCount,strText,strCount,bAlarm,bRepeat);
			bDrawSubItem = (m_nSelectedIndexInOneDay >= 0) && (m_nSelectedIndexInOneDay < nRecordsCount);


			if ((focusRow != 0) && (focusRow != 2) && (focusRow != 4) && (focusRow != 6) && !m_HitSameCell)
			{
				RECT rtHL = GetRect (focusRow-1, focusCol);
				COLORREF orgBkColor = 	GetCells (focusRow-1, focusCol)->GetBackColor();
				COLORREF orgTextColor = GetCells (focusRow-1, focusCol)->GetTextColor();
				GetCells (focusRow-1, focusCol)->SetBackColor(HIGH_LIGHT_BG_COLOR);
				GetCells (focusRow-1, focusCol)->SetTextColor(HIGH_LIGHT_TEXT_COLOR);
				if ((focusRow-1)+focusCol == 0) 
				{
					BITMAP bitmap;
					m_CalendarImage.GetBitmap(&bitmap);
					GetCells (focusRow-1, focusCol)->Draw(pDC,rtHL,ALIGN_LEFT,m_CalendarImage);
				} else 
				{
					GetCells (focusRow-1, focusCol)->Draw(pDC,rtHL,FALSE,rtHL,m_Image,TRUE);
				}
				GetCells (focusRow-1, focusCol)->SetBackColor(orgBkColor);
				GetCells (focusRow-1, focusCol)->SetTextColor(orgTextColor);
			}

			if ((focusRow != 0) && (focusRow != 2) && (focusRow != 4) && (focusRow != 6))
			{
				RECT rect = GetRect (focusRow, focusCol);
				if (bDrawSubItem)
				{
					rect.top += m_nSelectedIndexInOneDay * m_nHeightPerLineInOneDay;
					rect.bottom = rect.top + m_nHeightPerLineInOneDay;
					GetCells (focusRow, focusCol)->DrawHitBorder(pDC, rect, RGB(255, 0, 20));
				}
			}
		}
		break;
	case DATEPERIOD_MONTHLY:
		{
			pMemeDC = &m_memDC[2];
			if (((focusRow-1) % 2) == 1)
			{
				COleDateTimeSpan dts;
				CString strInfo;
				int ts = xGetMonthlyIndex(focusRow,focusCol);
				dts.SetDateTimeSpan(ts,0,0,0);
				dt = m_MonthlyStartDate + dts;
				if (!((dt.GetYear() == m_sMonthInfo->CurrentYear) && (dt.GetMonth() == m_sMonthInfo->CurrentMonth))) 
					m_sMonthInfo = m_s3MonthInfo.GetScheduleMonthInfo(dt);
				m_sMonthInfo->SetScheduleDate(dt);
				
				BOOL bWeekend = (j == 5);
				strInfo = xGetMonthlyScheduleTitles(m_sMonthInfo,dt,bWeekend,nRecordsCount,strText,strCount,bAlarm,bRepeat);
				
				int maxDisplay = bWeekend ? m_nMaxDisplayWeekendRecordsPerDay : m_nMaxDisplayRecordsPerDay;
				bDrawSubItem = (m_nSelectedIndexInOneDay >= 0) && (m_nSelectedIndexInOneDay < min(nRecordsCount,maxDisplay));
			}
		}
		break;
	}

	rect.InflateRect(-1,-1,-1,-1);
	InvalidateRect(&rect,FALSE);

}
*/

BOOL CCAMMScheduleDailyTable::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	if(zDelta<0) 
		SendMessage(WM_VSCROLL,SB_LINEDOWN);
	else 
		SendMessage(WM_VSCROLL,SB_LINEUP);
	return TRUE;	
//	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}
