// HyperLink.h : header file

#if !defined(_HYPERLINK_H_)
#define _HYPERLINK_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "_defControlext.h"

/////////////////////////////////////////////////////////////////////////////
// CHyperLink window

class CONTROL_EXT CHyperLink
{

// Construction/destruction
public:
    CHyperLink();
    virtual ~CHyperLink();

public:
	BOOL Draw(CDC* pDC);
	
	void Init( HWND hWnd, CRect rect )
			{	m_hWnd = hWnd;
				m_rect = rect;
			}
	void SetURL(CString strURL, CString strLink="");
	void SetUnderLine(BOOL bUnderline )
			{	m_bUnderline = bUnderline;	}
	void SetColor(COLORREF crLink, COLORREF crVisited, COLORREF crHover=-1 );
    void FitText( UINT nType );

	BOOL OnClicked( CPoint point );
	BOOL OnMouseMove( CPoint point);
/*	void DefCursor(UINT nID)
		{	m_hLinkCursor = AfxGetApp()->LoadCursor(nID);	}
*/
// Attributes
public:

// Implementation
protected:
	HFONT CreateLinkFont();

// Protected attributes
protected:
	HWND	 m_hWnd;
    COLORREF m_crLink;
	COLORREF m_crVisited;
    COLORREF m_crHover;

    BOOL     m_bVisited;
    BOOL	 m_bHover;
	BOOL     m_bUnderline;

    CString  m_strURL; 
	CString  m_strLink;
	CRect	 m_rect;

    HCURSOR  m_hLinkCursor;
};

/////////////////////////////////////////////////////////////////////////////
#endif // !defined(_HYPERLINK_H_)
