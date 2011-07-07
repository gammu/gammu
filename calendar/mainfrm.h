// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__893A55C4_3C71_478A_81E4_12F028233EC1__INCLUDED_)
#define AFX_MAINFRM_H__893A55C4_3C71_478A_81E4_12F028233EC1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "ChildView.h"

#include "toolbar\uiflatbar.h"
#include "include\tabsplitterWnd.h"
#include "DialogBarEx.h"

//ノ钡Μ╈Μンヘ夹跌怠
class CDropTgt : public COleDropTarget
{
// Construction
public:
    CDropTgt() {};

// Implementation
public:
    virtual ~CDropTgt() {};
    
   //
   // These members MUST be overridden for an OLE drop target
   // See DRAG and DROP section of OLE classes reference
   //
   DROPEFFECT OnDragEnter(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point);
   DROPEFFECT OnDragOver(CWnd *pWnd, COleDataObject *pDataObject, DWORD dwKeyState, CPoint point);
   void OnDragLeave(CWnd *pWnd);
   BOOL OnDrop(CWnd *pWnd, COleDataObject *pDataObject, DROPEFFECT dropEffect, CPoint point);
};

#define FUNC_BN_NUMBER 3
#include "Schedule\CAMMScheduleDayView.h"

class CMainFrame : public CFrameWnd
{
	
public:
	static LPCTSTR s_winClassName;
	
	CMainFrame();
	CCAMMScheduleDayView* m_ScheduleDayView;

	BOOL m_bCapture;
/*	//TestCalendar
    void OnGetMobileCal();
	static int ConnectStatusCallBack(int State);
*/
protected: 
	DECLARE_DYNAMIC(CMainFrame)
	
// Attributes
public:
    void OnLoadDataFromMobile();
    void UpdateStatusConnect();
    // Ken 7/28
	void UpdateMyDLLCmdUI();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	CString GetTreeSelectMode();
	static int ConnectStatusCallback(int iState);
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	CTabSplitterWnd m_wndSplitter;		//将mainframe下班部分分为leftview 和 TabSplitterWnd 
	//CTabSplitterWnd m_wndSplitterTop;		//将窗口分为上下两个部分
	int m_bMobileConnected;
	bool m_bMobileGetFail;
	//CDialogBarEx m_LeftDlgBar, m_RightDlgBar;

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
//	CMyToolBar	m_wndFuncBar;
	CMyToolBar	m_wndToolBar;

	//using rebar to contain two toolbar
	CReBar		m_wndRebar;
	CMyToolBar	m_wndToolBarLeft;
	CMyToolBar  m_wndToolBarRight;
	CMyToolBar  m_wndToolBarTest;
	
	CBitmap		m_BmpToolBar;

	Tstring	m_sProfile;
	UINT m_nFuncID[FUNC_BN_NUMBER];
	bool m_bOpenSync;
	static int m_iStatus;
	bool m_bSearch;

	//Calendar

    void DocViewClose(HWND AppHwnd);

// Generated message map functions
protected:
	void xSetFuncBarCheck(int iState);
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnNew();
	afx_msg void OnEdit();
	afx_msg void OnDelete();
	afx_msg void OnSearch();
	afx_msg void OnBackup();
	afx_msg void OnRestore();
	afx_msg void OnRefresh();
	afx_msg void OnPhonebook();
	afx_msg void OnSchedule();
	afx_msg void OnMemo();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnUpdateBnBackup(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBnDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBnEdit(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBnNew(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBnRefresh(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBnRestore(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBnSearch(CCmdUI* pCmdUI);
	afx_msg void OnExit();
	afx_msg void OnHelp();
	afx_msg void OnUpdateBnExit(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBnHelp(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBnAbout(CCmdUI* pCmdUI);	
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClose();
	//}}AFX_MSG
	afx_msg void    OnReload();
	afx_msg void    OnRefreshLeftView(WPARAM wParam,LPARAM lParam);
	afx_msg void    OnCheckTreeItem(WPARAM wParam,LPARAM lParam);
	afx_msg void    OnCheckDelete(WPARAM wParam,LPARAM lParam);
	afx_msg void    OnLoadCalendar(WPARAM wParam,LPARAM lParam);
	afx_msg void    OnUpdateStatusConnect(CCmdUI* pCmdUI);
	afx_msg HRESULT OnCardPanelGetGroup(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnCardPanelGroupCount(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnLeftChangeMode(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnGetLeftMode(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnErrOpenSync(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnGetSyncflag(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnLeftDropData(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnSetLoadFileFlag(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnSetLinkMobile(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnSetSearchFlag(WPARAM wParam,LPARAM lParam);
	afx_msg void    OnOpenCalendar(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnSetLeftItemCount(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnLeftDropData2MSOT(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnLeftUpdateData(WPARAM wParam,LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__893A55C4_3C71_478A_81E4_12F028233EC1__INCLUDED_)
