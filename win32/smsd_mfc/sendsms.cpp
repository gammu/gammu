
#include "common.h"

#include <afxwin.h>
#include <afxext.h>
#include <afxcview.h>

#include "../../common/service/sms/gsmsms.h"
#include "../../common/service/sms/gsmmulti.h"
#include "../../common/misc/coding/coding.h"

#include "resource.h"
#include "sendsms.h"

const CString Validity[] = {
	"default (taken from sending phone)",
	"5 minutes",
	"10 minutes",
	"max. time",
	""};

const CString Report[] = {
	"default (depends on sending daemon)",
	"yes",
	"no",
	""};

CSendSMSDlg::CSendSMSDlg() : CDialog(CSendSMSDlg::IDD)
{
}

void CSendSMSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT1, Text);
	DDX_Control(pDX, IDC_COMBO1, ValidityBox);
	DDX_Control(pDX, IDC_COMBO2, SendingPhoneBox);
	DDX_Control(pDX, IDC_COMBO3, ReportBox);
}

void CSendSMSDlg::OnEnChangeEdit1()
{
	char 	*text[20000];
	char 	*text2[41000];
	int 	SMSNum, int CharsLeft;
	char    buf[1000];

	UpdateData(TRUE);

	_tcscpy((char *)text, Text);

	EncodeUnicode((unsigned char *)text2,(unsigned char *)text,Text.GetLength());

	GSM_SMSCounter(Text.GetLength(),(unsigned char *)text2,UDH_NoUDH,SMS_Coding_Default_No_Compression,&SMSNum,&CharsLeft);
	if (SMSNum > 1) {
		GSM_SMSCounter(Text.GetLength(),(unsigned char *)text2,UDH_ConcatenatedMessages,SMS_Coding_Default_No_Compression,&SMSNum,&CharsLeft);
	}

	sprintf(buf,"SMS text (%i chars left/%i SMS)",CharsLeft,SMSNum);
	GetDlgItem(IDC_STATIC2)->SetWindowText(buf);
}

void CSendSMSDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	int i = 0;
	while (Validity[i]!="") {
		ValidityBox.AddString(Validity[i]);
		i++;
	}
	ValidityBox.SetCurSel(0);

	i = 0;
	while (Report[i]!="") {
		ReportBox.AddString(Report[i]);
		i++;
	}
	ReportBox.SetCurSel(0);

	SendingPhoneBox.AddString("any phone");
	SendingPhoneBox.SetCurSel(0);

	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
}

BEGIN_MESSAGE_MAP(CSendSMSDlg, CDialog)
	ON_WM_SHOWWINDOW()
	ON_EN_CHANGE(IDC_EDIT1, OnEnChangeEdit1)
END_MESSAGE_MAP()
