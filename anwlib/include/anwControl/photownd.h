#if !defined(AFX_PHOTOWND_H__3E8E17E8_746B_4609_9A86_1AF07986284F__INCLUDED_)
#define AFX_PHOTOWND_H__3E8E17E8_746B_4609_9A86_1AF07986284F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PhotoWnd.h : header file
//
#include "_defControlext.h"
#include "_anwImage.h"
/////////////////////////////////////////////////////////////////////////////
// CPhotoWnd window

#define PSM_NONE		0
#define PSM_PAN			1

#define CURSOR_PALM		0
#define CURSOR_GRAB		1

class CONTROL_EXT CPhotoWnd : public CWnd
{
// Construction
public:
	CPhotoWnd();

// Attributes
public:

protected:
	CImage	*m_pImg;
	HWND	m_hWndNotify;

	float	m_Zoom;
	BOOL    m_bNoZoom;
	BOOL	m_bFitToWindow;
	int 	m_nSelMode;
	CRect	m_rcBorder;
	//pan
	//HCURSOR m_hCurPalm;
	//HCURSOR m_hCurGrab;
	HCURSOR	m_hCursor[2];

	CPoint  m_ptDown;
	CPoint	m_ptOff, m_ptOffDown;
	BOOL	m_bResetPos;

	BOOL	m_bPan;
// Operations
public:
/*	virtual void DefCursor(int nType, UINT nID)
		{	m_hCursor[nType] = AfxGetApp()->LoadCursor(nID);
		}
*/		
	void SetNotifyWindow(HWND hWnd) { m_hWndNotify = hWnd; }

	void ResetPos(BOOL bReset)
		{	m_bResetPos = bReset;	}

	BOOL Create(const RECT& rect, CRect* pRcMax, CWnd *pParentWnd, UINT nID);
	BOOL Show(BOOL bShow = TRUE);

	CImage *_SetImage(CImage *pImg) 
	{
		CImage *pImgOld = m_pImg;
		m_pImg = pImg;
		return pImgOld;
	}

	CImage *SetImage(CImage *pImg);
	CImage *GetImage() { return m_pImg; }

	void SetZoom(float zoom, BOOL bShow=TRUE);
	float GetZoom() { return m_Zoom; }

	CPoint GetZoomPos(float zoom);
	CSize GetZoomSize(float zoom);
	
	BOOL IsZoomable() { return m_bNoZoom; }
	BOOL IsFitToWindow() { return m_bFitToWindow; }

	void SetZoomable(BOOL bEnable) { m_bNoZoom = !bEnable; }
	void FitToWindow(int cx = 0, int cy = 0, BOOL bEnable = TRUE);
	void OnParentSize(int cx, int cy);
	void PreDraw100();
	void Paint(CDC *pDC);
	void Paint(Graphics& graph);
	void ScaleToWindow(int cx = 0, int cy = 0, BOOL bEnable = TRUE);

	//
	virtual void SetSelMode(int nSelMode);
	int GetSelMode(){	return	m_nSelMode; };
	void SetPanMode(BOOL bPan) { SetSelMode(bPan ? PSM_PAN : PSM_NONE); }

	void SetBorder( CRect border )
		{	m_rcBorder = border;	}

	// pan
	void SetOffset(CPoint pt, BOOL bShow = TRUE );
	CPoint& GetOffset() { return m_ptOff; }
	void ShiftOffset(CPoint pt) { SetOffset(m_ptOff + pt); }
	void SetZoomOffset(CPoint pt, BOOL bShow = TRUE );
	void ShiftZoomOffset(CSize size);
	void GetViewRect( LPRECT lpRect )
		{	GetClientRect(lpRect);	}

	virtual void SetCurCursor();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhotoWnd)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPhotoWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPhotoWnd)
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wparam, LPARAM lparam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHOTOWND_H__3E8E17E8_746B_4609_9A86_1AF07986284F__INCLUDED_)
