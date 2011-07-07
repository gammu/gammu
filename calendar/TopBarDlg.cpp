// TopBarDlg.cpp : implementation file
//

#include "stdafx.h"
#include "calendar.h"
#include "TopBarDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTopBarDlg dialog


CTopBarDlg::CTopBarDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTopBarDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTopBarDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTopBarDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTopBarDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTopBarDlg, CDialog)
	//{{AFX_MSG_MAP(CTopBarDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTopBarDlg message handlers
