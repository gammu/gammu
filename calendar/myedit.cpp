// myedit.cpp : implementation file
//

#include "stdafx.h"
#include "myedit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyEdit

CMyEdit::CMyEdit()
{
	m_bNumFlag = false;
}

CMyEdit::~CMyEdit()
{
}


BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
	//{{AFX_MSG_MAP(CMyEdit)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyEdit message handlers

void CMyEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(m_bNumFlag && nChar != '*' && nChar != '#' && nChar != '?' && nChar != '+' && nChar != 'p' && nChar != 8
		&&  ( nChar < '0' || nChar > '9') )		
		return;
	else if(m_bNumFlag && nChar == '+'){
		int iStart,iEnd;
		GetSel(iStart,iEnd);
		if(iStart != 0)
			return;
	}

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}
