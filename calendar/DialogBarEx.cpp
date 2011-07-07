// DialogBarEx.cpp : implementation file
//

#include "stdafx.h"
#include "calendar.h"
#include "DialogBarEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogBarEx dialog


CDialogBarEx::CDialogBarEx()
{
	m_pBkBmp = NULL;
	m_pImgBk = NULL;
	//{{AFX_DATA_INIT(CDialogBarEx)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDialogBarEx::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogBarEx)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogBarEx, CDialogBar)
	//{{AFX_MSG_MAP(CDialogBarEx)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//ON_MESSAGE(UM_SHOWTEXT, OnShowText)
END_MESSAGE_MAP()

BOOL CDialogBarEx::Create(CWnd *pWnd, UINT nIDtem, UINT nStyle, UINT nID, CBitmap *pBkBmp, COLORREF bkColor) 
{
	// TODO: Add your specialized code here and/or call the base class
	m_bkColor = bkColor;
	m_pBkBmp = pBkBmp;
	
	return CDialogBar::Create(pWnd, nIDtem, nStyle, nID);
}

void CDialogBarEx::SetImgBk(LPCTSTR lpszPath, bool bUseBK)
{
	SAFE_DELPTR(m_pImgBk);
	m_pImgBk = new CImage(lpszPath);
	m_bUseImgBk = bUseBK;
}

/////////////////////////////////////////////////////////////////////////////
// CDialogBarEx message handlers


BOOL CDialogBarEx::OnEraseBkgnd(CDC* pDC) 
{
	Graphics graphics(pDC->GetSafeHdc());		
	CRect rect;
	GetClientRect(rect);
	
	if(m_bUseImgBk)
	{
		//m_pImgBk->Display(graphics,rect,DRAW_TILE);
		CBrush bkBrush;
		bkBrush.CreateSolidBrush(RGB(255, 255, 255));
		CBrush *pOldBrush;
		pOldBrush = (CBrush*)pDC->SelectObject(&bkBrush);
		CRect rt;
		GetClientRect(&rt);
		//	ScreenToClient(&rt);
		pDC->Rectangle(-1, -1, rt.Width() + 1, rt.Height() + 1);
		pDC->SelectObject(pOldBrush);
		m_pImgBk->Display(graphics, rect.right - 180, 24, 118, 23, DRAW_NORMAL);
	}
	else
	{
		CBrush bkBrush;
		bkBrush.CreateSolidBrush(m_bkColor);
		CBrush *pOldBrush;
		pOldBrush = (CBrush*)pDC->SelectObject(&bkBrush);
		CRect rt;
		GetClientRect(&rt);
		//	ScreenToClient(&rt);
		pDC->Rectangle(-1, -1, rt.Width() + 1, rt.Height() + 1);
		pDC->SelectObject(pOldBrush);
	}
	pDC->TextOut(10, 24, L"Calendar");	
	return true;
}

void CDialogBarEx::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CDialogBar::OnPaint() for painting messages
}