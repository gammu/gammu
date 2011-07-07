#if !defined(AFX_SRLWNDPHOTO_H__8396B243_F069_4139_8294_DF8928C0EFE7__INCLUDED_)
#define AFX_SRLWNDPHOTO_H__8396B243_F069_4139_8294_DF8928C0EFE7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SrlWndPhoto.h : header file
//

#include "photownd.h"

/////////////////////////////////////////////////////////////////////////////
// CSrlWndPhoto window

class CONTROL_EXT CSrlWndPhoto : public CPhotoWnd
{
// Construction
public:
	CSrlWndPhoto();

// Attributes
public:
	BOOL m_bScrollWnd;
	void UseScroll( BOOL bScroll );
	void Refresh();
// Operations
public:
	BOOL Create(const RECT& rect, CWnd *pParentWnd, UINT nID);
	void OnParentSize(int cx, int cy);
	void SetZoom(float nZoom);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSrlWndPhoto)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSrlWndPhoto();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSrlWndPhoto)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SRLWNDPHOTO_H__8396B243_F069_4139_8294_DF8928C0EFE7__INCLUDED_)
