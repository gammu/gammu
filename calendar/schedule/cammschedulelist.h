#if !defined(AFX_CAMMSCHEDULELIST_H__D4869A8C_2E31_4871_8101_2385F8C01EB3__INCLUDED_)
#define AFX_CAMMSCHEDULELIST_H__D4869A8C_2E31_4871_8101_2385F8C01EB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// cammschedulelist.h : header file
//
#include "CAMMScheduleDailyTable.h"
#include "..\MobileDevice\CAMMSync.h"

#define SCH_TITLE 0
#define SCH_STARTDATE 1
#define SCH_STARTTIME 2		
#define SCH_STOPTIME 3
#define SCH_REPEAT 4

/////////////////////////////////////////////////////////////////////////////
// CCAMMScheduleList view

class CCAMMScheduleList : public CListView
//class CCAMMScheduleList : public CListCtrl
{
	friend class CCAMMScheduleDayView;
protected:
	CCAMMScheduleList();           // protected constructor used by dynamic creation
//	DECLARE_DYNCREATE(CCAMMScheduleList)

// Attributes
public:

	//Added by Nono =>
    int PrintRecord();//[2004_0513]
//	CListPrint *m_pPrintRecord;//Print the total list of schedule,2004_0513
	void AutoSizeColumns();
	CPrintSchedule *m_PrintSchedule;//[Print the single schedul,,2004_0517]
	//Added by Nono <=

	//Calendar
    void OnAddCalendar();
	bool m_bMobileConnected;

	//for sort colummn of listview
	static BOOLEAN m_bFlag;
	static BOOLEAN m_bSTC;

// Operations
public:
	void SetImageList();
	void RemoveScheduleInfo(CScheduleInfo& si,bool bNeedUpdate = true);
	void RemoveSchduleInfoInList(CScheduleInfo si);
	void AddSchduleInfoToList(CScheduleInfo info);
	void ModifySchduleInfoToList(CScheduleInfo oldinfo,CScheduleInfo newinfo);
	UINT GetClipboardFormatID();
	BOOL GetDataSectionFlag();
	void DeInitialize();
	BOOL xPasteData(CString &cpStr);
	BOOL xGetSelectedClipboardString(CString &cpStr);
	BOOL xCopyDataToClipboard(CString cpStr);
	BOOL xGetDataFromClipboard(CString &cpStr);
	void SetClipboardFormatId(UINT id) {m_ClipboardFormatId = id;}
	void xDeleteScheduleItem();
	CAMMError xCreateScheduleItem();
	BOOL xCheckRecordMaxLimit(int newCnt=0);
	BOOL xShowEditorDlg(CScheduleInfo si, CScheduleInfo &si2);
	void GetScheduleInfo(int nItem,CScheduleInfo &si);
	CAMMError xEditScheduleItem(INT nItem);
	void Initialize();
	void SetScheduleTablePtr(CCAMMScheduleDailyTable* table) {m_Table = table;}
	void SetScheduleViewPtr(CWnd* view) {m_ScheduleView = view;}
	void FreeScheduleList();
	void UpdateScheduleData();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCAMMScheduleList)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CCAMMScheduleList();
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CCAMMScheduleList)
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);	//Ë«»÷ÏìÓ¦
	//afx_msg void OnRButtonDown(UINT nFlags, CPoint point);		//kerm add for 9@9u right click
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnOrgSchDelete();
	afx_msg void OnUpdateOrgSchDelete(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchAdd();
	afx_msg void OnUpdateOrgSchAdd(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchEdit();
	afx_msg void OnUpdateOrgSchEdit(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchSave();
	afx_msg void OnUpdateOrgSchSave(CCmdUI* pCmdUI);
	afx_msg void OnOrgSchSaveas();
	afx_msg void OnUpdateOrgSchSaveas(CCmdUI* pCmdUI);
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
	afx_msg void OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CImageList	m_ImageList;
	CMenu m_Menu;
	CCAMMScheduleDailyTable* m_Table;
	CWnd* m_ScheduleView;
//	CList<CScheduleInfo,CScheduleInfo> m_ScheduleList;
	CPtrList m_ScheduleList;
	BOOL m_bInDataSection;
	int m_nCurrentItem;
	UINT m_ClipboardFormatId;
    CArray<NoteTypeSupportInfo,NoteTypeSupportInfo> m_ArrayofSupportNoteType;
	int m_SizeofSupportNoteType;
	BOOL m_bCreateImageList;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMMSCHEDULELIST_H__D4869A8C_2E31_4871_8101_2385F8C01EB3__INCLUDED_)
