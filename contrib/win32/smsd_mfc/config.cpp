
#include "common.h"

#include <afxwin.h>
#include <afxext.h>
#include <afxcview.h>

#include "resource.h"
#include "config.h"

CConfigDlg::CConfigDlg() : CDialog(CConfigDlg::IDD)
{
}

void CConfigDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
//	HTREEITEM hChapter;

	CDialog::OnShowWindow(bShow, nStatus);

//    	CTreeCtrl& tree = (CTreeView*)GetDlgItem(IDC_TREE1)->  ->GetTreeCtrl();

	//hChapter = tree.InsertItem(Row0[0]);
	    	//tree.InsertItem( "Phones", hChapter);
	    	//tree.InsertItem( "Inbox", hChapter);
	    	//tree.InsertItem( "Outbox", hChapter );
	    	//tree.InsertItem( "Sent items", hChapter );
}

BEGIN_MESSAGE_MAP(CConfigDlg, CDialog)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()
