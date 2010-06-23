#include <afxwin.h>
#include <afxext.h>
#include <afxcview.h>

#include "resource.h"

class CEditPbkDlg : public CDialog
{
public:
	enum {IDD = IDD_EDITPBK};
	CEditPbkDlg();
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);
};
