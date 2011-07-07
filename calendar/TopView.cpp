// TopView.cpp : implementation file
//

#include "stdafx.h"
#include "calendar.h"
#include "TopView.h"
#include "TopBarDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTopView

IMPLEMENT_DYNCREATE(CTopView, CView)

CTopView::CTopView()
{
}

CTopView::~CTopView()
{
}


BEGIN_MESSAGE_MAP(CTopView, CView)
	//{{AFX_MSG_MAP(CTopView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTopView drawing

void CTopView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
	pDC->TextOut(10, 20, "top view");
}

/////////////////////////////////////////////////////////////////////////////
// CTopView diagnostics

#ifdef _DEBUG
void CTopView::AssertValid() const
{
	CView::AssertValid();
}

void CTopView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTopView message handlers

void CTopView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
/*	*/
	// TODO: Add your specialized code here and/or call the base class
	CRect rect;
	GetClientRect(&rect);
	m_topBarDlg.Create(IDD_DLG_TOPBAR,this);//GetSystemMetrics(SM_CXSIZEFRAME)
	m_topBarDlg.MoveWindow(0, GetSystemMetrics(SM_CYCAPTION) + 30, rect.Width(), rect.Height() - 28);//rect);
	m_topBarDlg.ShowWindow(SW_SHOW);
}
