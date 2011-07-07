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

#include "Panel\PrevView.h"
#include "Panel\leftview.h"

//用於接收拖收物件的目標視窗
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

class CMainFrame : public CFrameWnd
{
	
public:
	static LPCTSTR s_winClassName;
	
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)
	
// Attributes
public:
	BOOL    m_bCapture;
    void    UpdateMyDLLCmdUI();
	BOOL	m_bSupportDirectSMS;
	static int m_iStatus;
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
	static int ConnectStatusCallback(int iState);
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;

	//using rebar to contain two toolbar
	CReBar		m_wndRebar;
	CMyToolBar	m_wndToolBar1;
	CMyToolBar  m_wndToolBar2;
	
	CBitmap		m_BmpToolBar;

	CTabSplitterWnd m_wndSplitter;
	Tstring	m_sProfile;
	UINT m_nFuncID[FUNC_BN_NUMBER];
	bool m_bOpenSync;
	bool m_bSearch;
//	static CMainFrame*  m_pActive;
	bool m_bFirst;
	CPrevView *m_pView;
	CLeftView *m_pLeft;
	// DLL CONTRUCT
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
	afx_msg void OnUpdateBnExit(CCmdUI* pCmdUI);
	afx_msg void OnBnHelp();
	afx_msg void OnBnAbout();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClose();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnUpdateBnSendsms(CCmdUI* pCmdUI);
	afx_msg void OnBnSendsms();
	//}}AFX_MSG
	afx_msg void OnUpdateStatusConnect(CCmdUI* pCmdUI);
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
	afx_msg HRESULT OnProgDlgShow(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnSetLeftItemCount(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnCheckLeftTreeItem(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnDownloadFail(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnMobileConnectStatus( WPARAM wParam,LPARAM lParam );
	afx_msg HRESULT OnLeftDropData2MSOT(WPARAM wParam,LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__893A55C4_3C71_478A_81E4_12F028233EC1__INCLUDED_)
