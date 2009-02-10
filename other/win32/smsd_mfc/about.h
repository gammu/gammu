#include <afxwin.h>
#include <afxext.h>
#include <afxcview.h>

#include "resource.h"

class CAboutDlg : public CDialog
{
public:
	CStatic Version;
	CAboutDlg();
	enum {IDD = IDD_ABOUT};
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
protected:
	DECLARE_MESSAGE_MAP()
};
