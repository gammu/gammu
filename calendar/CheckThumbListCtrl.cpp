// CheckThumbListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "calendar.h"
#include "CheckThumbListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCheckThumbListCtrl

CCheckThumbListCtrl::CCheckThumbListCtrl()
{
}

CCheckThumbListCtrl::~CCheckThumbListCtrl()
{
}


BEGIN_MESSAGE_MAP(CCheckThumbListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CCheckThumbListCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCheckThumbListCtrl message handlers

void CCheckThumbListCtrl::PreSubclassWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CListCtrl::PreSubclassWindow();
}
