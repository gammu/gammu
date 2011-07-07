#if !defined(AFX_PREVVIEW_H__D460FCE3_5635_4C1A_A1B8_82A1DFCDE60E__INCLUDED_)
#define AFX_PREVVIEW_H__D460FCE3_5635_4C1A_A1B8_82A1DFCDE60E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrevView.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CPrevView view
#define RC_ITEM_NONE		0x0000 // Void, indicates invalid items only
#define	RC_ITEM_ALL			0x0001 // All items regardless of states
#define	RC_ITEM_SELECTED	0x0002 // Selected items
#define	RC_ITEM_UNSELECTED	0x0004 // Unselected items
#define	RC_ITEM_CHECKED		0x0008 // Checked items
#define	RC_ITEM_UNCHECKED	0x0010 // Unchecked items
#define	RC_ITEM_FOCUSED		0x0020 // Focused item
#define	RC_ITEM_UNFOCUSED	0x0040 // Unfocused items

#define COLOR_INVALID	0xffffffff



class CPrevView : public CListView
{
protected:
	CPrevView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPrevView)

	CFont m_font;
	int m_nSortCol;
	BOOL m_bSortAscending; // Is sort ascending?

// Attributes
public:
	CSkinHeaderCtrl m_SkinHeaderCtrl;

	

private:
	int			m_nCurrentItem;
	int			m_nDragItem;
	BOOL		m_bIsDragging, m_nDragTarget;;
	int			m_cxImage, m_cyImage;
	HTREEITEM	m_hCurSelItem;


	CFont		m_screenFont;
    CFont		m_printerFont;
    UINT		m_cyScreen;
    UINT		m_cyPrinter;
    UINT		m_cxOffset;
    UINT		m_cxWidth;
    UINT		m_nLinesTotal;
    UINT		m_nLinesPerPage;

	HFONT		m_hFont;

	SMS_PARAM	*m_SMSInfo;
	RECT		m_rRect;

	Tstring		m_sProfile;
	bool		m_CtrlKeyDown;

	void		init();
	bool		SetHeaderControlProfile(LPCTSTR lpszProfile,LPCTSTR sec);

// Operations
public:
	//void AddDataToList(LPCSTR lpsContent, LPCSTR lpsSender, LPCSTR lpsTime);
	//bool GetData(int iIndex,CListData &data);
	void	AddToListView(SMS_PARAM *smsInfo);
	void	DeleteAllItems();
	BOOL	GetSelectedItem(LV_ITEM *lvi, int iWhichItem);
	void	ShowItems(SMS_PARAM *smsInfo, int nShow, int nItemCounts);

	bool	xMoveData(void) ;

	static int CALLBACK CompareMe(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	static	CPrevView* s_ActivePrev;

	
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrevView)
	public:
	virtual void OnDraw(CDC* pDC); 
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPrevView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPrevView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSave();
	afx_msg void OnReSend();
	afx_msg void OnSMSPrint();
	afx_msg void OnDelete();
	afx_msg void OnUpdateSave(CCmdUI* pCmdUI);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateBnResend(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePrint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDelete(CCmdUI* pCmdUI);
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnReply();
	afx_msg void OnUpdateBnReply(CCmdUI* pCmdUI);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};



/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVVIEW_H__D460FCE3_5635_4C1A_A1B8_82A1DFCDE60E__INCLUDED_)
