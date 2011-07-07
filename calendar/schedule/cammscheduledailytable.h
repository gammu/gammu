#if !defined(AFX_CCAMMSCHEDULEDAILYTABLE_H__F3DF54D9_6EAF_44BF_A3B0_AD9452E695BD__INCLUDED_)
#define AFX_CCAMMSCHEDULEDAILYTABLE_H__F3DF54D9_6EAF_44BF_A3B0_AD9452E695BD__INCLUDED_

#pragma once

//#include "..\resource.h"
#include "XCell.h"
#include "..\UnicodeString\UnicodeString.h"
//#include "..\commonDeclaration.h"
//#include "CommonDeclaration.h"	// Added by ClassView
#include "..\Organizer\OrganizeDeclaration.h"
#include "..\Organizer\Schedule3MonthInfo.h"
#include "..\Organizer\ScheduleMonthInfo.h"
//#include ".\ListPrint.h"
#include ".\PrintSchedule.h"

const int MAX_ROWS1 = MAX_SCHEDULE_COUNT + 60;
//const int MAX_COLS1 = 16;
const int MAX_COLS1 = 7;

// CCAMMScheduleDailyTable
class CCAMMScheduleDailyTable : public CWnd
{
	friend class CCAMMScheduleDayView;
	friend class CCAMMScheduleList;

	DECLARE_DYNAMIC(CCAMMScheduleDailyTable)

public:
	CCAMMScheduleDailyTable();
	CCAMMScheduleDailyTable(int rows, int cols);
	virtual ~CCAMMScheduleDailyTable();

    BOOL Create(const RECT& rect, CWnd* parent, UINT nID,
                DWORD dwStyle = WS_CHILD | WS_BORDER | WS_TABSTOP | WS_VISIBLE | WS_VSCROLL);
public:
	BOOL m_bCreateMemDCSuc;
	XCell* cells;
	int* rowHeight;
	int* colWidth;

	int rows;
	int cols;

	int focusRow;
	int focusCol;

	XCell defaultCell;
	int defaultHeight;
	int defaultwidth;

	//Added by Nono =>
    int PrintRecord();//[2004_0513]
//	CListPrint *m_pPrintRecord;//2004_0513
	//Added by Nono <=

	//Calendar
	bool m_bMobileConnected;
	BOOL m_bMonthLastDay;
	COleDateTime m_dtOldMonthfocusDay;

public:
	int SetRows(int rows);
	int GetRows();
	int SetCols(int cols);
	int GetCols();

	int SetRowHeight(int row, int height);
	int GetRowHeight(int row);

	int SetColWidth(int col, int width);
	int GetColWidth(int col);

	int GetRowsHeight(int startRow, int endRow);
	int GetColsWidth(int startCol, int endCol);

	int AppendRow() {return SetRows(GetRows() + 1);}
	int AppendCol() {return SetCols(GetCols() + 1);}
	int deleteRow() {return SetRows(GetRows() - 1);}
	int deleteCol() {return SetCols(GetCols() - 1);}

	int JoinCells (int startRow, int startCol, int endRow, int endCol);
	int UnjoinCells (int row, int col);

	int SetCells(int row, int col, XCell& cell);
	XCell* GetCells(int row, int col);

	int SetAlarm(int row, int col, BOOL bAlarm);
	int SetAlarm(int row, int col,int idx, BOOL bAlarm);
	int SetAlarm(int row, int col, BOOL bAlarm[48],int count);

	int SetRepeat(int row, int col, BOOL bRepeat);
	int SetRepeat(int row, int col,int idx, BOOL bRepeat);
	int SetRepeat(int row, int col, BOOL bRepeat[48],int count);

	int SetText(int row, int col, CString str);
	int SetText(int row, int col,int idx, CString str);
	int SetText(int row, int col, CString strText[48],int strCount);
	CString GetText(int row, int col);
	CString GetText(int row, int col,int idx);

	int SetTextColor(int row, int col, COLORREF color);
	COLORREF GetTextColor(int row, int col);

	int SetTextFont(int row, int col, CFont& font);
	CFont* GetTextFont(int row, int col);

	int SetTextFontSize(int row, int col, int size);
	int GetTextFontSize(int row, int col);

	int SetBackColor(int row, int col, COLORREF color);
	COLORREF GetBackColor(int row, int col);
	int SetDrawUpLine(int row, int col, BOOL bDraw);
	int SetDrawDnLine(int row, int col, BOOL bDraw);
	int SetDrawLeftLine(int row, int col, BOOL bDraw);
	int SetDrawRightLine(int row, int col, BOOL bDraw);

	int SetBorderColor(int row, int col, COLORREF color);

	int SetBackMode(int row, int col, int mode);
	int GetBackMode(int row, int col);
	int SetTextLine(int row, int col,BOOL bTextLinw);
	int SetOverlap (int row, int col, bool enable);
	bool GetOverlap (int row, int col);

	int SetAlignment (int row, int col, int align);
	int GetAlignment (int row, int col);

	int SetWordbreak (int row, int col, bool enable);
	bool GetWordbreak (int row, int col);

	void OnOrgSchAdd();
	void GetRepeatCurrentDateInfo(CScheduleInfo &si,COleDateTimeEx dtStopDate);

public:

	int Draw(CDC* pDC);
	bool HitTest (CPoint point, int& row, int& col);
	int Test ();
	int CreateTable();

private:
	int Draw_Daily(CDC* pDC);
	int Draw_Weekly(CDC* pDC);
	int Draw_Monthly(CDC* pDC);
	RECT GetRect(int row, int col);

	bool HitTest_Daily (CPoint point, int& row, int& col);
	bool HitTest_Weekly (CPoint point, int& row, int& col);
	bool HitTest_Monthly (CPoint point, int& row, int& col);

	int CreateTable_Daily();
	int CreateTable_Weekly();
	int CreateTable_Monthly();

	void OnVScroll_Daily(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	void OnVScroll_Weekly(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	void OnVScroll_Monthly(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	void OnSize_Daily(UINT nType, int cx, int cy);
	void OnSize_Weekly(UINT nType, int cx, int cy);
	void OnSize_Monthly(UINT nType, int cx, int cy);

	void OnLButtonDblClk_Daily(UINT nFlags, CPoint point);
	void OnLButtonDblClk_Weekly(UINT nFlags, CPoint point);
	void OnLButtonDblClk_Monthly(UINT nFlags, CPoint point);

//
	CDC m_memDC;
	CBitmap		m_bitmap;		// Offscreen bitmap
//	CBitmap*	m_oldBitmap;	// bitmap originally found in CMemDC
	BOOL		m_bInitmemDC;
	CRect m_rcMemDC;
	CBrush m_brWhite;
protected:
	// Generated message map functions
	//{{AFX_MSG(CCAMMScheduleDailyTable)
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnOrgSchDelete();
	afx_msg void OnOrgSchDaily();
	afx_msg void OnUpdateOrgSchDaily(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchWeekly();
	afx_msg void OnUpdateOrgSchWeekly(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchMonthly();
	afx_msg void OnUpdateOrgSchMonthly(CCmdUI* pCmdUI);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnUpdateOrgSchAdd(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchEdit();
	afx_msg void OnUpdateOrgSchEdit(CCmdUI* pCmdUI);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnOrgSchCut();
	afx_msg void OnUpdateOrgSchCut(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchCopy();
	afx_msg void OnUpdateOrgSchCopy(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchPaste();
	afx_msg void OnUpdateOrgSchPaste(CCmdUI* pCmdUI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	afx_msg LRESULT OnInvalidate(WPARAM wParam,LPARAM lParam);

	LRESULT OnNewText(WPARAM wParam, LPARAM);
	DECLARE_MESSAGE_MAP()

public:
//	void DrawFocusBorder();
	CRect DrawCell_Daily(CDC *pDC,int nrow,int nCol);
	CRect DrawCell_Monthly(CDC *pDC,int nrow,int nCol,BOOL& bDrawSubItem);
	CRect DrawCell_Weekly(CDC *pDC,int nrow,int nCol,BOOL& bDrawSubItem);
	void xInvalidatecell(int row,int Col);
	void UninitMemDC();
	void InitMemDC();
	void GotoDailyView();
	void SetScrollBarPosition(int nRow);
	UINT GetClipboardFormatID();
	void xInvalidate(BOOL bErase);
	void SetAllUnDraw();
	void SetAllRedraw();
	void SetClipboardFormatId(UINT id) {m_ClipboardFormatId = id;}
	void GetRecordCount(int &recordCount);
	void SetDirtyFlag(BOOL bDirty);
	void GetScheduleData(SchedulesRecord *pScheduleRecord,int &recordCount);
	void SetScheduleData(SchedulesRecord *pScheduleRecord,int recordCount);
	BOOL IsDataDirty();
	void SetDatePeriod(Schedule_DatePeriod sDp);
	int GetScheduleInfo(int row,CScheduleInfo& si);
	int SetScheduleInfo(CScheduleInfo& si);
	int AddScheduleInfo(CScheduleInfo& si);
	void RemoveScheduleInfo(CScheduleInfo& si,bool bNeddUpdate = true);
	int SetMemoCellInfo(int row,CScheduleInfo sd);
	BOOLEAN GetTimeInfo(int rowIndex,int& hour,int& min,HALF_DAY &halfDay);
	void SetHalfDayString(CString morning,CString afternoon);
	BOOLEAN GetFocusCell(int& row,int& col);
	BOOLEAN GetHourInfo(int rowIndex,CString& hourStr,CString& halfDayStr,BOOLEAN& bFirst30Min);
	void SetTableTitle(CString csTitle);
	void SetInfoRowCnt(int infoRowCnt);
	void UpdateScheduleData();
	static BOOL RegisterWindowClass();
	void ChangeDatePeriod(Schedule_DatePeriod dp);
//	void LoadScheduleFromFile(CString csFileName);
	void SaveScheduleToFile(CString csFileName);

	//Nono, 2004_0413
	BOOL CreateStatusBar();
	BOOL CheckIsDataSelected();

protected:
	CScheduleMonthInfo *m_sMonthInfo;
	//Nono, 2004_0413
	CStatusBar  m_wndStatusBar;

	//Calendar
	Schedule_DatePeriod GetDatePeriod() {return m_DatePeriod;}

private:
	BOOL xPasteData(CString &cpStr);
	BOOL xGetSelectedClipboardString(CString &cpStr);
	BOOL xCopyDataToClipboard(CString cpStr);
	BOOL xGetDataFromClipboard(CString &cpStr);
	BOOL xGetSelectedItem(CScheduleInfo &si);
	BOOL xCheckIsDataSelected();
	void xDeleteScheduleItem();
	int xDraw_Background(CDC* pDC,XCell& cell,RECT bkRect);
	BOOL xCheckRecordMaxLimit(int newCnt=0);
	BOOL xShowCalendar(CPoint point,COleDateTimeEx &currentDT);
	BOOL m_bShowCalendar;
	//Test, Nono_2004_0407
	BOOL m_bShowGotoToday;

	BOOL xShowEditorDlg(CScheduleInfo si,CScheduleInfo &si2,BOOL &bSameDate);
	BOOL m_bPopupMenu;
	BOOL m_bShowWindow;
	void xInit(int row,int col);
	BOOL m_bRedraw[MAX_ROWS1+50];
	int xGetMonthlyIndex(int row,int col);
	int xGetWeeklyIndex(int row,int col);
	int m_nMaxDisplayWeekendRecordsPerDay;
	BOOL m_bGotoDaily;
	int m_nHeightPerLineInOneDay;
	int m_nMaxDisplayRecordsPerDay;
	BOOL m_bMore[42];
	CBitmap m_Image;
	CBitmap m_CalendarImage;

	void xAssignMonthlyInfo();
	void xAssignWeeklyInfo();
	void xAssignDailyInfo();
	BOOL m_DirtyFlag;
	BOOL xGetUpdatedScheduleInfo(int selectedIndex,CScheduleInfo &si);
	int m_nSelectedIndexInOneDay;
	BOOL m_bHitTest;
	COleDateTimeEx m_MonthlyStartDate;  //Display start date  09/27
	COleDateTimeEx m_MonthlyFirstDate;  //First date of this month ex: 10/01
	void xCreateTableForMonthly(COleDateTimeEx dt);
	void xClearTable();
	CString xGetScheduleTitles(CScheduleMonthInfo* pMi,COleDateTimeEx dt);
	CString xGetWeeklyScheduleTitles(CScheduleMonthInfo *pMi, COleDateTimeEx dt, BOOL bWeekend,int &nRecordsCount,CString strReturn[48],int &retCount,BOOL bAlarm[48],BOOL bRepeat[48]);
	CString xGetMonthlyScheduleTitles(CScheduleMonthInfo *pMi, COleDateTimeEx dt, BOOL bWeekend,int &nRecordsCount,CString strReturn[48],int &retCount,BOOL bAlarm[48],BOOL bRepeat[48]);
	void xCreateTableForWeekly(COleDateTimeEx dt);
	void xUpdateWeeklyTitle(int delta);
	COleDateTimeEx m_WeeklyStartDate;
	COleDateTimeEx m_CurrentDate;
	CSchedule3MonthInfo m_s3MonthInfo;
	Schedule_DatePeriod m_DatePeriod;
	void ShowCalendar(CPoint point);
	CString m_AfternoonString;
	CString m_MorningString;
	int m_StartHour;
	CString m_csTableTitle;
	int m_MaxRowForDisplay;
	int m_RealStartRow;
	int m_InfoRowCnt;
	void xSetScrollRange(int nBar,int nMinPos,int nMaxPos,BOOL bRedraw = TRUE);
	int m_ScrollBarPos;
	int m_RowsPerPage;
	int m_StartRowForDisplay;
	CMenu m_Menu;
	CString m_csFileName;
	SchedulesRecord *m_pScheduleRecord;
	CButtonEx m_CtrlCalendar;
	int m_DailyDataRowCnt;
	UINT m_ClipboardFormatId;
	BOOL m_HitSameCell;


	//Test, Nono_2004_0407
	CBitmap m_GotoTodayImage;
	//Added by Nono =>
    BOOL xSetInfomationonStatusBar(void);//2004_0415
	//Added by Nono <=
};


#endif  //AFX_CCAMMSCHEDULEDAILYTABLE_H__F3DF54D9_6EAF_44BF_A3B0_AD9452E695BD__INCLUDED_
