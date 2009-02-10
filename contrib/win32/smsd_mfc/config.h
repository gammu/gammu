#include <afxwin.h>
#include <afxext.h>
#include <afxcview.h>

#include "resource.h"

class CConfigDlg : public CDialog
{
public:
	CConfigDlg();
	enum {IDD = IDD_CONFIG};
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
protected:
	DECLARE_MESSAGE_MAP()
};
