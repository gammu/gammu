
#include "common.h"

#include <afxwin.h>
#include <afxext.h>
#include <afxcview.h>
#include <winsock.h>
#include <mysql.h>

#include "../../common/misc/misc.h"
#include <gammu-config.h>

#include "resource.h"
#include "about.h"

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CString txt;

	CDialog::OnShowWindow(bShow, nStatus);

	txt = "Built ";
	txt = txt + __TIME__;
	txt = txt + " ";
	txt = txt + __DATE__;
	if (strlen(GetCompiler()) != 0) {
		txt = txt + " in ";
		txt = txt + GetCompiler();
	}
	SetDlgItemText (IDC_STATIC3, txt);

	txt = "Using Gammu ";
	txt = txt + VERSION;
	SetDlgItemText (IDC_STATIC4, txt);

	txt = "Using MySQL library ";
	txt = txt + mysql_get_client_info();
	SetDlgItemText (IDC_STATIC5, txt);

	txt = "";
	if (strlen(GetOS()) != 0) {
		txt = "Run on ";
		txt = txt + GetOS();
	}
	SetDlgItemText (IDC_STATIC6, txt);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()
