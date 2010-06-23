#include <afxwin.h>
#include <afxext.h>
#include <afxcview.h>

#include "resource.h"

class CLogUserDlg : public CDialog
{
public:
	CString User;
	CString Pass;
	CString IP;
	enum {IDD = IDD_LOGUSER};
	CLogUserDlg();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnEnChangeEdit2();
protected:
	DECLARE_MESSAGE_MAP()
	void CheckIDOK();
	virtual void DoDataExchange(CDataExchange* pDX);
};
