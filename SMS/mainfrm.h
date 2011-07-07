// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__893A55C4_3C71_478A_81E4_12F028233EC1__INCLUDED_)
#define AFX_MAINFRM_H__893A55C4_3C71_478A_81E4_12F028233EC1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "ChildView.h"
#include "PrevView.h"
#include "LeftView.h"

#include "toolbar\uiflatbar.h"

#include "TabSplitterWnd.h"

#define FUNC_BN_NUMBER 10

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


class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)
	
// Attributes
public:
	CLeftView*	GetLeftPane();
	CPrevView*  GetRightPane();

	LONG		GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata);
	void		LaunchHTMLFile( LPCTSTR filename );
    void        DocViewClose(HWND AppHwnd);
    void        UpdateMyDLLCmdUI();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar		m_wndStatusBar;
	//CMyToolBar		m_wndFuncBar;
	//CMyToolBar		m_wndToolBar;

	//using rebar to contain two toolbar
	CReBar		m_wndRebar;
	CMyToolBar	m_wndToolBar1;
	CMyToolBar  m_wndToolBar2;
	
	CBitmap		m_BmpToolBar;

	//CSplitterWnd m_wndSplitter;
	CTabSplitterWnd	m_wndSplitter;
	Tstring			m_sProfile;
	UINT			m_nFuncID[FUNC_BN_NUMBER];

private:
//	UINT			m_nTimer;
	//int				m_iStatus;

	int			GetMobileSetting(void);
	TCHAR		m_strPhone[MAX_PATH], m_strMobileName[MAX_PATH];

// Generated message map functions
protected:
	void xSetFuncBarCheck(int iState);
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnNew();
	afx_msg void OnRefresh();
	afx_msg void OnBnBack();
	afx_msg void OnClose();
	afx_msg void OnBnHelp();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnUpdateStatusConnect(CCmdUI* pCmdUI);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnUpdateBnRefresh(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg HRESULT OnMobileConnectStatus( WPARAM wParam,LPARAM lParam );
	afx_msg HRESULT OnStartConnect( WPARAM wParam,LPARAM lParam );
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__893A55C4_3C71_478A_81E4_12F028233EC1__INCLUDED_)
