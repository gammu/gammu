//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// You may use this source code, compile or redistribute it as part of your application 
// for free. You cannot redistribute it as a part of a software development 
// library without the agreement of the author. If the sources are 
// distributed along with the application, you should leave the original 
// copyright notes in the source code without any changes.
// This code can be used WITHOUT ANY WARRANTIES at your own risk.
// 
// For the latest updates to this code, check this site:
// http://www.masmex.com 
// after Sept 2000
// 
// Copyright(C) 2000 Philip Oldaker <email: philip@masmex.com>
//*******************************************************************************

#include "stdafx.h"
#include "TabSplitterWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CTabSplitterWnd message handlers

LPCTSTR CTabSplitterWnd::szSplitterSection = _T("Splitter");
LPCTSTR CTabSplitterWnd::szPaneWidthCurrent = _T("width");
LPCTSTR CTabSplitterWnd::szPaneWidthMinimum = _T("width_min");
LPCTSTR CTabSplitterWnd::szPaneHeightCurrent = _T("height");
LPCTSTR CTabSplitterWnd::szPaneHeightMinimum = _T("height_min");

IMPLEMENT_DYNAMIC(CTabSplitterWnd, CSplitterWnd)

BEGIN_MESSAGE_MAP(CTabSplitterWnd, CSplitterWnd)
	//{{AFX_MSG_MAP(CTabSplitterWnd)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_WM_MOUSEMOVE()	//kerm add,to control the size of the view
END_MESSAGE_MAP()

CTabSplitterWnd::CTabSplitterWnd()
	 : m_strSection(szSplitterSection)
{	
	m_nCurCol = m_nCurRow = 0;
	m_cxCur = m_cyCur = 0;
	m_cxMin = m_cyMin = 0;
	m_crBg = 0;

	//对分割条宽度重新定义，m_cxSplitter是基类的成员变量 kerm add 2.13
//	m_cxSplitter = 20; 
//	m_cxSplitterGap = 20;

	
}

void CTabSplitterWnd::SetSection(LPCTSTR szSection)
{
	//m_strSection = szSplitterSection;
	//m_strSection += _T("\\");
	//m_strSection += szSection;
	m_strSection = szSection;
}

CWnd *CTabSplitterWnd::GetActiveWnd()
{
	int row, col;
	return GetActivePane(row,col);
}

void CTabSplitterWnd::ActivateNext(BOOL bPrev)
{
	ASSERT_VALID(this);
	
	// find the coordinate of the current pane
	int row, col;
	if (GetActivePane(&row, &col) == NULL)
	{
		SetActivePane(0,0);
		return;
	}
	ASSERT(row >= 0 && row < m_nRows);
	ASSERT(col >= 0 && col < m_nCols);

	// determine next pane
	if (bPrev)
	{
		// prev
		if (--col < 0)
		{
			col = m_nCols - 1;
			if (--row < 0)
				row = m_nRows - 1;
		}
	}
	else
	{
		// next
		if (++col >= m_nCols)
		{
			col = 0;
			if (++row >= m_nRows)
				row = 0;
		}
	}

	// set newly active pane
	SetActivePane(row, col);
}

void CTabSplitterWnd::OnDrawSplitter(CDC *pDC, ESplitType nType, const CRect& rect)
{
	// Let CSplitterWnd handle everything but the border-drawing
	if(/*(nType != splitBorder) ||*/ (pDC == NULL))
	{
		CSplitterWnd::OnDrawSplitter(pDC, nType, rect);
		return;
	}

	//kerm test 2.13 for splitter window ,change color
//	COLORREF color = RGB(223, 0, 0); 
	COLORREF color =RGB(216, 221, 243); ////m_crBg;CRect(0, 0, 0, 0);
	CRect rc = rect;

	switch (nType)
	{
		case splitBorder:
			pDC->Draw3dRect(rc, color, color);
			rc.InflateRect(-1, -1);//-1
			pDC->Draw3dRect(rc, color, color);
			return;

		case splitIntersection:
			pDC->Draw3dRect(rc, color, color);
			rc.InflateRect(-1, -1);
			pDC->Draw3dRect(rc, color, color);
			break;

		case splitBox:
			pDC->Draw3dRect(rc, color, color);
			rc.InflateRect(-1, -1);
			pDC->Draw3dRect(rc, color, color);
			rc.InflateRect(-1, -1);
			break;

		case splitBar:
			pDC->Draw3dRect(rc, color, color);
			rc.InflateRect(-1, -1);
			pDC->Draw3dRect(rc, color, color);
			break;
	}

	// fill the middle
	pDC->FillSolidRect(rect, color);
	pDC->Draw3dRect(rect, GetSysColor(COLOR_BTNSHADOW), GetSysColor(COLOR_BTNHIGHLIGHT));
}

void CTabSplitterWnd::SaveSize()
{
#ifdef _DEBUG
	if (m_strSection == szSplitterSection)
		TRACE0("Warning: SetSection has not been called in IMSplitterWnd!\n");
#endif

	GetColumnInfo(0, m_cxCur, m_cxMin);

	if (m_cxCur)
		afxGetApp()->WriteProfileInt(m_strSection, szPaneWidthCurrent, m_cxCur);

	if (m_cxMin)
		afxGetApp()->WriteProfileInt(m_strSection, szPaneWidthMinimum, m_cxMin);

	GetRowInfo(0, m_cyCur, m_cyMin);

	if (m_cyCur)
		afxGetApp()->WriteProfileInt(m_strSection, szPaneHeightCurrent, m_cyCur);
	
	if (m_cyMin)
		afxGetApp()->WriteProfileInt(m_strSection, szPaneHeightMinimum, m_cyMin);
}

void CTabSplitterWnd::SetSize(int nCur,int nMin)
{
	if (m_nRows > 1) 
	{
		m_cyCur = nCur;
		m_cyMin = nMin;
	}
	if (m_nCols > 1) 
	{
		m_cxCur = nCur;
		m_cxMin = nMin;
	}
}

void CTabSplitterWnd::Apply()
{
	if (m_nRows > 1)
	{
		SetRowInfo(0,m_cyCur,m_cyMin);
		RecalcLayout();
	}
	else if (m_nCols > 1) 
	{
		SetColumnInfo(0,m_cxCur,m_cxMin);
		RecalcLayout();
	}
	else
		TRACE0("Applying splitter bar before creating it!\n");
}

BOOL CTabSplitterWnd::CreateView(int row, int col, CRuntimeClass* pViewClass, SIZE sizeInit, CCreateContext* pContext)
{
	if (m_nCols > 1) 
	{
		if (m_cxCur)
			sizeInit.cx = m_cxCur;
		else if (m_strSection != szSplitterSection)
			sizeInit.cx = afxGetApp()->GetProfileInt(m_strSection, szPaneWidthCurrent, sizeInit.cx);

		m_cxCur = sizeInit.cx;
	}

	if (m_nRows > 1) 
	{
		if (m_cyCur)
			sizeInit.cy = m_cyCur;
		else if (m_strSection != szSplitterSection)
			sizeInit.cy = afxGetApp()->GetProfileInt(m_strSection, szPaneHeightCurrent, sizeInit.cy);

		m_cyCur = sizeInit.cy;
	}

	return CSplitterWnd::CreateView(row, col, pViewClass, sizeInit, pContext);
}

void CTabSplitterWnd::StopTracking(BOOL bAccept)
{
	// save old active view
	CWnd* pOldActiveView = GetActivePane();
	CSplitterWnd::StopTracking(bAccept);
	if (bAccept) 
	{
		if (pOldActiveView == GetActivePane())
		{
			if (pOldActiveView == NULL)
			{
				if (m_nCols > 1)
					SetActivePane(0, 1); 
	//			pOldActiveView->SetFocus(); // make sure focus is restored
				if (m_nRows > 1)
					SetActivePane(0, 0); 
			}	
		}
		SaveSize();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTabSplitterWnd message handlers
void CTabSplitterWnd::OnDestroy()
{
	CSplitterWnd::OnDestroy();
	m_nCurRow = -1;
	m_nCurCol = -1;
}

void CTabSplitterWnd::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	SaveSize();
	CSplitterWnd::OnClose();
}

void CTabSplitterWnd::OnSetFocus(CWnd* pOldWnd) 
{
	CSplitterWnd::OnSetFocus(pOldWnd);
	
	// TODO: Add your message handler code here
	if (m_nCurRow >= 0 && m_nCurCol >= 0) 
	{
		SetActivePane(m_nCurRow,m_nCurCol);
		CWnd *pWnd = GetPane(m_nCurRow,m_nCurCol);
		pWnd->SetFocus();
	}
}

void CTabSplitterWnd::OnKillFocus(CWnd* pNewWnd) 
{
	CSplitterWnd::OnKillFocus(pNewWnd);
	
	// TODO: Add your message handler code here	
	GetActivePane(&m_nCurRow,&m_nCurCol);
}

// This currently only saves the first pane
void CTabSplitterWnd::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_cxCur;
		ar << m_cxMin;
		ar << m_cyCur;
		ar << m_cyMin;
	}
	else
	{
		ar >> m_cxCur;
		ar >> m_cxMin;
		ar >> m_cyCur;
		ar >> m_cyMin;
	}
}

// mouse wheel handled by the views
BOOL CTabSplitterWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	// TODO: Add your message handler code here and/or call default
	TRACE(_T("SplitterWnd mouse wheel message\n"));
/*	if (m_nCurRow >= 0 && m_nCurCol >= 0) 
	{
		SetActivePane(m_nCurRow,m_nCurCol);
		CWnd *pWnd = GetPane(m_nCurRow,m_nCurCol);
	}*/
	return TRUE;
}

BOOL CTabSplitterWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS, NULL, NULL, NULL);
	ASSERT(cs.lpszClass);
	
	return CSplitterWnd::PreCreateWindow(cs);
}

void CTabSplitterWnd::OnPaint()
{
	CSplitterWnd::OnPaint();
}

//去掉对鼠标移动的响应，不作处理，可以实现窗口的移动。 kerm 2.13
// void CTabSplitterWnd::OnMouseMove(UINT nFlags, CPoint point) 
// {
// 	// TODO: Add your message handler code here and/or call default
// /*	
// 	if(point.x<228||point.x>600 || point.y<100 || point.y>600) 
// 	{ 
// 		CWnd::OnMouseMove(nFlags, point); 
// 	} 
// 	else 
// 	{ 
// 		CSplitterWnd::OnMouseMove(nFlags, point); 
// 	} 
// 	*/
//}