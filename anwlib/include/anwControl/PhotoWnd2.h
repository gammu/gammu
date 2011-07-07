#if !defined(AFX_PHOTOWND2_H__38B210AD_26B0_4DE8_BFB3_88F1B285E9A0__INCLUDED_)
#define AFX_PHOTOWND2_H__38B210AD_26B0_4DE8_BFB3_88F1B285E9A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PhotoWnd2.h : header file
//
#include "photownd.h"
/////////////////////////////////////////////////////////////////////////////
// CPhotoWnd2 window

#define CURSOR_DROPPER		2
	
static UINT WM_ANW_PW2_PANDOWN		= ::RegisterWindowMessage(_T("ANW_PHOTOWND2_PANDOWN"));
static UINT WM_ANW_PW2_PANUP		= ::RegisterWindowMessage(_T("ANW_PHOTOWND2_PANUP"));
static UINT WM_ANW_PW2_DROPPERPOS	= ::RegisterWindowMessage(_T("ANW_PHOTOWND2_DROPPERPOS"));

class CONTROL_EXT CPhotoWnd2 : public CPhotoWnd
{
// Construction
public:
	CPhotoWnd2();

// Attributes
public:
	HCURSOR m_hDropper;
	BOOL	m_bMouseTracking;
// Operations
public:
	virtual void SetSelMode(int nSelMode);
	virtual void SetCurCursor();

	// Dropper
	BOOL m_bDropper;
	void SetDropper( BOOL bEnable )
	{	m_bDropper = bEnable; 
		SetCurCursor();
	}

/*	virtual void DefCursor(int nType, UINT nID)
	{	if( nType == CURSOR_DROPPER )
			m_hDropper = AfxGetApp()->LoadCursor(nID);
		else
			CPhotoWnd::DefCursor(nType, nID);
	}
*/
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhotoWnd2)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPhotoWnd2();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPhotoWnd2)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM wparam, LPARAM lparam);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHOTOWND2_H__38B210AD_26B0_4DE8_BFB3_88F1B285E9A0__INCLUDED_)
