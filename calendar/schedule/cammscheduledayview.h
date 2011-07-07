#if !defined(AFX_CAMMSCHEDULEDAYVIEW_H__EF45CFCF_B723_4EAB_BD57_6FA75C91861F__INCLUDED_)
#define AFX_CAMMSCHEDULEDAYVIEW_H__EF45CFCF_B723_4EAB_BD57_6FA75C91861F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "cammschedulelist.h"
//#include "..\CommonDeclaration.h"	// Added by ClassView

// CAMMScheduleDayView.h : header file
//
#include "..\MobileDevice\CAMMSync.h"

//Nono =>
/*
class CListAlert
{
public:
	CListAlert();
	~CListAlert();

public:
	CList<CScheduleInfo, CScheduleInfo> listAlerts;


};
*/
//Nono <=
/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleDayView view

class CCAMMScheduleDayView : public CWnd
{
	friend class CCAMMScheduleList;
	friend class CCAMMScheduleDailyTable;
protected:
//	CCAMMScheduleDayView();           // protected constructor used by dynamic creation
//	DECLARE_DYNCREATE(CCAMMScheduleDayView)

// Attributes
public:
	CCAMMScheduleDayView();           // protected constructor used by dynamic creation
	~CCAMMScheduleDayView();
	DECLARE_DYNCREATE(CCAMMScheduleDayView)
	CString GetCurrentFile() { return m_usFileName;}
	CString GetCurrentFilePath() { return m_usFilePath;}
	void SetFileName(CString usFileName);
	CAMMError Save(CString usFileName) {return xSave(usFileName);}
	void Reset();
//    BOOL SetSkinResource();
	CPtrList m_MobileCalendarDataList;

	//Added by Nono =>
//    int PrintRecord();//[2004_0512]
//	CListPrint *m_pPrintRecord;//2004_0512
//	UINT    m_nIDTimer;
/*	CListAlert *m_pListAlert;
	void    SetAlert(CScheduleInfo &ScheduleInfo);
	int     RescanAlerts();
	void    CheckAllAlerts();
    void    ShowAlertInfo(CScheduleInfo &ScheduleInfo, POSITION pos=NULL);
*/	//Added by Nono <=

	//Calendar
	Schedule_DatePeriod GetDatePeriod() {return m_DatePeriod;}
	CCAMMScheduleDailyTable* GetTable() {return m_Table;}
	CCAMMScheduleList* GetScheduleList() {return m_ScheduleList;}
//    void OpenCalendar();
	//Calendar
	bool m_bMobileConnected;
  //  int LoadCalendarfromMobile();

//    static int LoadScheduleCallBack(int nNeed);
//    int xLoadScheduleCallBack();
    void ClearTable();

	//For cprogress bar dialog
	int m_nRecordCount;
	SchedulesRecord *m_pScheduleRecord;
    CCAMMSync m_CalendarDataWrapper;
	static CCAMMScheduleDayView* s_pCalendarDayView;

// Operations
public:
	static BOOL RegisterWindowClass();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCAMMScheduleDayView)
	public:
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
public:
//protected:
	//Nono, 2004_0413
	CStatusBar  m_wndStatusBar;

//	virtual ~CCAMMScheduleDayView();
/*	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
*/
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CCAMMScheduleDayView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnOrgSchDelete();
	afx_msg void OnOrgSchSave();
	afx_msg void OnUpdateOrgSchSave(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchSaveas();
	afx_msg void OnUpdateOrgSchSaveas(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchAdd();
	afx_msg void OnUpdateOrgSchAdd(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchEdit();
	afx_msg void OnUpdateOrgSchEdit(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchOpen();
	afx_msg void OnUpdateOrgSchOpen(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchNew();
	afx_msg void OnUpdateOrgSchNew(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchCut();
	afx_msg void OnUpdateOrgSchCut(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchCopy();
	afx_msg void OnUpdateOrgSchCopy(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchPaste();
	afx_msg void OnUpdateOrgSchPaste(CCmdUI* pCmdUI);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
/*	afx_msg void OnOrgPrnSchedule();
	afx_msg void OnUpdateOrgPrnSchedule(CCmdUI* pCmdUI);
	afx_msg void OnOrgPrnScheduleList();
	afx_msg void OnUpdateOrgPrnScheduleList(CCmdUI* pCmdUI);
*/	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLoadCalendar(WPARAM wParam,LPARAM lParam);
	afx_msg void OpenCalendar(WPARAM wParam,LPARAM lParam);

	DECLARE_MESSAGE_MAP()

public:
	void UpdateTableCalendarData(CPtrList* pOldData , CPtrList* pDataList);
	void FreeMobileCalendarDataList();
	UINT GetClipboardFormatID();
	BOOL GetDataSectionFlag();
	void SetAllUnDraw();
	void SetAllRedraw();
	void SetClipboardFormatId(UINT id) {m_ClipboardFormatId = id;}
	void ShowScheduleMenu(POINT ptAction);
	BOOL IsDataDirty();
	CAMMError OpenOrganzerFile(CString csFilename);
	CAMMError LoadScheduleFromFile(CString csFileName);
	CAMMError SaveScheduleToFile(CString csFileName);
	void ChangeDatePeriod(Schedule_DatePeriod sd,BOOL bUpdateData = TRUE);
	BOOLEAN UserExiting();
	void GetScheduleRecordCount(int &cnt);
	void GetScheduleRecord(SchedulesRecord *psr,int &cnt);

	void GotoOnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	//Nono, 2004_0413
	BOOL CreateStatusBar();
    BOOL SetInfomationonStatusBar(void);

private:
	void xPostMessage(UINT msg,WPARAM wparam,LPARAM lparam);
	CAMMError xGetFilePath(CString &szFilePath,CString csDBPath);
	CString m_usFilePath;
	CAMMError xInitialize(CString csFilename);
	CString m_usFileName;
	CCAMMScheduleDailyTable* m_Table;
	CCAMMScheduleList* m_ScheduleList;
//	CCAMMScheduleList m_ScheduleList;
	CAMMError xSave();
	CAMMError xSaveAs();
	CAMMError xSave(CString usFileName);
	void xGetFileName();
	Schedule_DatePeriod m_DatePeriod;
	CMenu m_Menu;
	UINT m_ClipboardFormatId;


};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMMSCHEDULEDAYVIEW_H__EF45CFCF_B723_4EAB_BD57_6FA75C91861F__INCLUDED_)
