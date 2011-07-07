// avidlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "avidlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAviDlg dialog


CAviDlg::CAviDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAviDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAviDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAviDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAviDlg)
	DDX_Control(pDX, IDC_ANIMATE, m_AniCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAviDlg, CDialog)
	//{{AFX_MSG_MAP(CAviDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAviDlg message handlers

BOOL CAviDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
