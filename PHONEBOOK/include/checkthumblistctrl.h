#if !defined(AFX_CHECKTHUMBLISTCTRL_H__7369EEAC_C8B0_4549_9591_37D6F2C905D2__INCLUDED_)
#define AFX_CHECKTHUMBLISTCTRL_H__7369EEAC_C8B0_4549_9591_37D6F2C905D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CheckThumbListCtrl.h : header file
//
enum {
	REPORT_VIEW = 0,
	CARD_VIEW = 1,
	ICON_VIEW = 2
};

#include "..\Data Structure\CardPanel.h"
#include "SkinHeaderCtrl.h"
/////////////////////////////////////////////////////////////////////////////
// CCheckThumbListCtrl window
//for YCBCR2RGB.dll function, To Load YCbCr Tiff image

class CCheckThumbListCtrl : public CListCtrl
{
// Construction
public:
	CCheckThumbListCtrl();
	virtual ~CCheckThumbListCtrl();

	CSkinHeaderCtrl m_SkinHeaderCtrl;
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCheckThumbListCtrl)
	public:
//	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	virtual BOOL DestroyWindow();
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	bool SetData(CCardPanel &data);
	bool SetHeaderControlProfile(LPCTSTR lpszProfile,LPCTSTR sec);
	int GetSelectedCount();
	int GetItemImage(int nItem);
	bool SetSelectedData(CCardPanel &data);
	bool GetSelectedData(CCardPanel &data);
	bool GetSelectedData(DATALIST &lsData);
	int FindItemData(LPCTSTR lpszText,int iStart = 0,int iEnd = -1);	//if find nothing ,return -1.
	void SetViewMode(int iMode);
	void init(bool bPic = false);
	bool AddData(CCardPanel &data);
	void	RemoveAllData();
	BOOL	DeleteSel();
	int GetViewMode(){return m_iMode;}
	void	DrawThumbnails();
	
	CImageList*	GetImageListThumb()	{ return m_pImageListThumb; };
	CImageList*	GetImageListReport()	{ return m_pImageListReport; };

	//Set image List
	void SetImageList(CImageList *pImageListThumb,CImageList *pImageListReport)
	{
		m_pImageListThumb = pImageListThumb;
		m_pImageListReport = pImageListReport;
		CListCtrl::SetImageList(m_pImageListThumb,LVSIL_NORMAL);
		CListCtrl::SetImageList(m_pImageListReport,LVSIL_SMALL);
	}

	int HitTest( CPoint pt, UINT* pFlags = NULL );
	CImage *GetImageBuffer(){return m_pImgBuffer;}
protected:
	void	xRescaleItem();
	void	xSetImageListItem();	//set both two image list
	void xChangeViewMode(int iMode);
	//{{AFX_MSG(CCheckThumbListCtrl)
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) ;
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags,CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

//data member
protected:
	bool	m_bSelItem;
	CImageList		*m_pImageListThumb;		// image list holding the thumbnails
	CImageList		*m_pImageListReport;		//image list holding the report

	int m_iMode;	//0:report 1:card
	bool m_bUsePic;	//check this mobile use calling photos or not
	int m_iLastClickCol;
	CFont m_font;
	//list item height
	int m_iItemHeight;
	//background color
	COLORREF m_crCol1;
	COLORREF m_crCol2;
	//background image
	CImage *m_pImgBuffer;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHECKTHUMBLISTCTRL_H__7369EEAC_C8B0_4549_9591_37D6F2C905D2__INCLUDED_)
