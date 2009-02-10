
#include "common.h"

#include <afxwin.h>
#include <afxext.h>
#include <afxcview.h>

#include "resource.h"
#include ".\loguser.h"

CLogUserDlg::CLogUserDlg() : CDialog(CLogUserDlg::IDD)
{
}

void CLogUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT1, IP);
	DDX_Text(pDX, IDC_EDIT2, User);
	DDX_Text(pDX, IDC_EDIT3, Pass);
}

BEGIN_MESSAGE_MAP(CLogUserDlg, CDialog)
	ON_WM_SHOWWINDOW()
	ON_EN_CHANGE(IDC_EDIT1, OnEnChangeEdit1)
	ON_EN_CHANGE(IDC_EDIT2, OnEnChangeEdit2)
END_MESSAGE_MAP()

void CLogUserDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	CheckIDOK();

	GetDlgItem(IDC_EDIT1)->SetFocus();
}

void CLogUserDlg::CheckIDOK()
{
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	
	if (User != "" && IP != "") GetDlgItem(IDOK)->EnableWindow(TRUE);
}

void CLogUserDlg::OnEnChangeEdit1()
{
	UpdateData(TRUE);
	CheckIDOK();
}

void CLogUserDlg::OnEnChangeEdit2()
{
	UpdateData(TRUE);
	CheckIDOK();
}
