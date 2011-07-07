#if !defined(AFX_PREVVIEW_H__D460FCE3_5635_4C1A_A1B8_82A1DFCDE60E__INCLUDED_)
#define AFX_PREVVIEW_H__D460FCE3_5635_4C1A_A1B8_82A1DFCDE60E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrevView.h : header file
//
#include "ViewModePanel.h"
/////////////////////////////////////////////////////////////////////////////
// CPrevView view

//#include "ListData.h"
//#include "CardPanel.h"
//#include "include\CheckThumbListCtrl.h"
#include "searchdlg.h"
//#include "driverwrapper.h"

//Calendar
//#include ".\OrganizerModules.h"
#include ".\resource.h"
#include ".\Schedule\CAMMScheduleDayView.h"
#include ".\MobileDevice\CAMMSync.h"


class CPrevView : public CView
{
protected:
	CPrevView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPrevView)

// Attributes
public:
	//TestCalendar
/*    void OnGetMobileCal();
	static int ConnectStatusCallBack(int State);
	bool LoadandOpenCalendarDriver();
	void FreeCalendarDriver();
*/
	HMODULE	hMobileDll;
	bool m_bOpened;
	CCAMMSync m_CalendarDriverWrapper;
	CCAMMScheduleDayView* GetCalendarDayView() {return &m_CalendarDayView;}
    //CList<Calendar_Data_Struct, Calendar_Data_Struct> m_ListofCalendars;

	int GetOutlookData(FileData *pData) ;
	void FreeDataList(CPtrList* pList);


// Operations
public:
	//get left mode pointer
	FileData* GetLeftMode(){return &m_LeftMode;}
	//get selected items in the list
	int GetListItemSelectedCount();
	//Check the item count in the list.
	bool IsItemInList();
	//when close app ,call this function.
	//It will write item of m_deqFileData to this file path deque.
//	void SaveCSVFile();
	//load mobile data
	bool LoadDataFromMobile();	
	//get all group item name
	LPCTSTR GetGroupStr(int iIndex);		
	//add a CCardPanel object to list
//	void AddDataToList(CCardPanel& data);	
	//add a CCardPanel object deque to list
//	void AddDataToList(DATALIST &lsData);
	//add a object to list with some base item data
	void AddDataToList(int iIndex,LPCSTR lpsFirstName, LPCSTR lpsLastName, LPCSTR lpsMobileTel, int iGroup,LPCSTR lpsMemoryType);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrevView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPrevView();
	virtual void OnDraw(CDC* pDC);
	// Generated message map functions
protected:

	//delete one data in the list
//	bool xDelListItem(CCardPanel &data);
	//the tab button panel
	CViewModePanel m_wndMode;
	//a list ctrl to show all item data
//	CCheckThumbListCtrl m_List;
	//a string deque to store the search key words
	deque<Tstring>	m_deqSearchStr;
	//a string deque to store the group name
	deque<Tstring>	m_deqGroup;
	//a CCardPanel deque to store the SIM card data
//	DATALIST	m_lsSIMData;
	//a CCardPanel deque to store the Memory card data
//	DATALIST	m_lsqMEData;
	//a deque with CCardPanel deque ,every file will has a CCardPanel deque 
	//and use this deque to store every file
//	deque<DATALIST> m_deqFileData;	
	//a search dialog
	CSearchDlg	*m_pSearchDlg;
	//a flag is stored the left tree selected mode
	FileData m_LeftMode;
	//the driver wrapper object
//	CDriverWrapper m_Driver;
	//a string deque to store the file path
//	deque<string>	m_deqFilePath;
	COLORREF	m_crItemBgNor,m_crItemBgSel,m_crItemBgFocus;
	COLORREF	m_crItemTextNor,m_crItemTextSel,m_crItemTextFocus;
	//background color
	COLORREF m_crCol1;
	COLORREF m_crCol2;
	//{{AFX_MSG(CPrevView)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	afx_msg void    OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg LRESULT OnViewModeChange(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnToolbarDelSel(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnToolbarSearch(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnToolbarNewEdit(WPARAM wParam,LPARAM lParam);	
	afx_msg HRESULT OnToolbarBackup(WPARAM wParam,LPARAM lParam);	
	afx_msg HRESULT OnToolbarRestore(WPARAM wParam,LPARAM lParam);	
	afx_msg HRESULT OnSearchDlgStr(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnSearchDlgEnd(WPARAM wParam,LPARAM lParam);	
	afx_msg HRESULT OnCardPanelGetGroup(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnCardPanelGroupCount(WPARAM wParam,LPARAM lParam);
	afx_msg	HRESULT OnLeftChange(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnToolbarReload(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnLeftDropData(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnLoadMobile(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnOpenDriver(WPARAM wParam,LPARAM lParam);
	afx_msg void    OnLoadCalendar(WPARAM wParam,LPARAM lParam);
	afx_msg void    OnOpenCalendar(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnSwitchToDayView(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnLeftDropData2MSOT(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnToolbarReloadMSOT(WPARAM wParam,LPARAM lParam);
	afx_msg	HRESULT OnLeftUpdateData(WPARAM wParam,LPARAM lParam);
	DECLARE_MESSAGE_MAP()

//Calendar
	CCAMMScheduleDayView m_CalendarDayView;

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVVIEW_H__D460FCE3_5635_4C1A_A1B8_82A1DFCDE60E__INCLUDED_)
