#include <afxwin.h>
#include <afxext.h>
#include <afxcview.h>

#include "resource.h"

class CSendSMSDlg : public CDialog
{
public:
	CString 	Text;
	CComboBox	ValidityBox;
	CComboBox	SendingPhoneBox;
	CComboBox	ReportBox;
	enum {IDD = IDD_SENDSMS};
	CSendSMSDlg();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);
};
