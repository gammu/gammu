// ViewModePanel.h: interface for the CViewModePanel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIEWMODEPANEL_H__093BB7A0_5D68_4B94_92DE_FD7102C16D69__INCLUDED_)
#define AFX_VIEWMODEPANEL_H__093BB7A0_5D68_4B94_92DE_FD7102C16D69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\include\TabButtonEx2.h"

#define VIEWMODE_TBN_NUM	2
class CViewModePanel : public CPanel  
{
public:
	CViewModePanel();
	virtual ~CViewModePanel();
protected:
	CDrawObj  m_line;
	CRect m_rcLine;
	void xSetTabActive(UINT nID);
	CTabButtonEx2  m_tbReport;	
	CTabButtonEx2  m_tbCard;
	CTabButtonEx2 *m_pTabBtn[VIEWMODE_TBN_NUM];
	UINT	m_nID[VIEWMODE_TBN_NUM];
	CImage *m_pImgBuffer;
	//{{AFX_MSG(CViewModePanel)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnReport();
	afx_msg void OnCard();
	afx_msg void OnIcon();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy) ;
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_VIEWMODEPANEL_H__093BB7A0_5D68_4B94_92DE_FD7102C16D69__INCLUDED_)
