#if !defined(AFX_LEFTVIEW_H__9589190E_E96E_47FD_8823_1055CB8ABCB4__INCLUDED_)
#define AFX_LEFTVIEW_H__9589190E_E96E_47FD_8823_1055CB8ABCB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LeftView.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CLeftView view
#define TREE_ITEM_NUM	8

enum { bDot, bSolid };

class CLeftView : public CTreeView
{
protected:
	CLeftView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CLeftView)

// Attributes
public:
	COleDropTarget	m_DropTarget;
	CPoint		m_ptClick;
	HFONT		m_hFont;
	void SetItemCount(FileData &fd, int iCount);

	//TestCalendar
    //void OnGetMobileCal();
	//static int ConnectStatusCallBack(int State);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLeftView)
	public:
	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual void OnDragLeave();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CLeftView();
	void		PaintParentLine(HTREEITEM hParent, HDC hDC, RECT &rc);
	void		PaintItemLines(HTREEITEM hItem, HTREEITEM hParent, HDC hDC, RECT &rc);
	void		PaintButton(HDC hDC, RECT &rc, BOOL bExpanded);

	void LineVert(HDC hDC, int x, int y0, int y1, bool nLine);
	void LineHorz(HDC hDC, int x0, int x1, int y, bool nLine);
	void FreeChildItem(HTREEITEM hParent);
	void InsertMSOTTree(HTREEITEM hMSOTRoot);

	// Generated message map functions
protected:
	void xAddToImageList(LPCTSTR lpszPath);
	CImageList  m_ImgList;
	Tstring m_sProfile;

	CBrush		m_brush;
	DWORD		m_dwStyle;
	COLORREF	m_crBack, m_crText, m_crLine;
	UINT		m_nIndent;
	HICON		m_hIconBtn[2];
	HIMAGELIST	m_hImgList;
	COLORREF	m_crItemBgNor,m_crItemBgSel,m_crItemBgFocus;
	COLORREF	m_crItemTextNor,m_crItemTextSel,m_crItemTextFocus;
	//{{AFX_MSG(CLeftView)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnCustomDraw( NMHDR* pxNMHDR, LRESULT* plResult );
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	afx_msg HRESULT OnLoadFile(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnCheckFile(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnLoadMobile(WPARAM wParam,LPARAM lParam);
	afx_msg void    OnRefresh(WPARAM wParam,LPARAM lParam);
	afx_msg HRESULT OnToolbarReloadMSOT(WPARAM wParam,LPARAM lParam);
	afx_msg void    OnCheckTreeItem(WPARAM wParam,LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEFTVIEW_H__9589190E_E96E_47FD_8823_1055CB8ABCB4__INCLUDED_)
