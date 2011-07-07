// ProgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "ProgDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgDlg dialog


CProgDlg::CProgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DLG_PROG, pParent)
{
	m_crBg = 0;
	m_pParent = NULL;
	//{{AFX_DATA_INIT(CProgDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CProgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgDlg)
	DDX_Control(pDX, IDC_SC_TEXT, m_scText);
	DDX_Control(pDX, IDC_PROG, m_Prog);
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDC_ANIMATE, m_AnCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProgDlg, CDialog)
	//{{AFX_MSG_MAP(CProgDlg)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgDlg message handlers

BOOL CProgDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	//get profile path
	m_sProfile = Tstring(theApp.m_szSkin) + Tstring(_T("Organize\\ProgDlg.ini"));
	//load cancel button
	GetButtonFromSetting(&m_rbnCancel,_T("cancel"),_T("IDS_CANCEL"),0,const_cast<TCHAR *>(m_sProfile.c_str()));
	GetStaticFromSetting(&m_scText,_T("text_sc"),NULL,const_cast<TCHAR *>(m_sProfile.c_str()));
	//get progress rect 
	CRect rect;
	if(al_GetSettingRect(_T("progress"),_T("rect"),const_cast<TCHAR *>(m_sProfile.c_str()),rect)){
		m_Prog.MoveWindow(rect);
	}
	//load text static
	
	//get dialog rect
	if(al_GetSettingRect(_T("panel") , _T("rect"),const_cast<TCHAR *>(m_sProfile.c_str()), rect)){
		LONG style = GetWindowLong(GetSafeHwnd(),GWL_STYLE);
		LONG dwstyle = GetWindowLong(GetSafeHwnd(),GWL_EXSTYLE);
		AdjustWindowRectEx(rect,style,FALSE,dwstyle);
		MoveWindow(rect);
		CenterWindow();
	}
	//get background color
	al_GetSettingColor(_T("panel") , _T("brush"),const_cast<TCHAR *>(m_sProfile.c_str()), m_crBg);

	//Mark by Asion for Dialog Layout
	if(al_GetSettingRect(_T("cancel"),_T("rect"),const_cast<TCHAR *>(m_sProfile.c_str()),rect)){
		m_rbnCancel.MoveWindow(rect);
	}
/*
	//Set cancel button position
	CRect rcProg,rc;
	GetClientRect(rc);
	m_rbnCancel.GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.OffsetRect(rc.right - rect.right - 8,0);
	m_rbnCancel.MoveWindow(rect);

	m_Prog.GetWindowRect(&rcProg);
	ScreenToClient(&rcProg);
	rcProg.right = rect.left - 8;
	m_Prog.MoveWindow(rcProg);
*/
	//set progress bar setting
	COLORREF white, blue;
	white = RGB(255,255,255);
	blue =  RGB(0,0,255);
	m_brBar.CreateSolidBrush( blue );
	m_Prog.SetBarBrush(&m_brBar);
	m_Prog.SetShowPercent();
	m_Prog.SetBkColor( white );
	m_Prog.SetTextColor( white, blue );
	
	OpenAvi();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProgDlg::OnOK()
{
}

void CProgDlg::OnCancel()
{
	::PostMessage(m_pParent->GetSafeHwnd(),WM_THREADBREAK,0,0);
}


void CProgDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rect;
	GetClientRect(rect);
	CBrush brush(m_crBg);
	dc.FillRect(rect,&brush);
}

void CProgDlg::SetRange(int iLow, int iUp)
{
	m_Prog.SetRange(iLow,iUp);
}

void CProgDlg::SetPos(int iPos)
{
	m_Prog.SetPos(iPos);
}


void CProgDlg::SetTitle(int iTitle)
{
	 if(iTitle == PROG_TRANSFER || iTitle == PROG_SENDSMS)
	{
		//set title
		TCHAR szTxt[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_PROG_TRANSFER"),theApp.m_szRes,szTxt);
		SetWindowText(szTxt);
		//set description
		al_GetSettingString(_T("public"),_T("IDS_PROG_TRANSFER_TEXT"),theApp.m_szRes,szTxt);
		m_scText.SetWindowText(szTxt);

		m_AnCtrl.ShowWindow(SW_HIDE);
		m_Prog.ShowWindow(SW_SHOW);
		m_scText.ShowWindow(SW_SHOW);
		m_rbnCancel.ShowWindow(SW_SHOW);
	}
	else if(iTitle == PROG_DEL){
		//set title
		TCHAR szTxt[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_PROG_DEL"),theApp.m_szRes,szTxt);
		SetWindowText(szTxt);
		//set description
		al_GetSettingString(_T("public"),_T("IDS_PROG_DEL_TEXT"),theApp.m_szRes,szTxt);
		m_scText.SetWindowText(szTxt);	
		
		m_AnCtrl.ShowWindow(SW_HIDE);
		m_Prog.ShowWindow(SW_SHOW);
		m_scText.ShowWindow(SW_SHOW);
		m_rbnCancel.ShowWindow(SW_SHOW);
	}
	else if(iTitle == PROG_INIT){
		//set title
		TCHAR szTxt[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_PROG_INIT_TEXT"),theApp.m_szRes,szTxt);
		SetWindowText(szTxt);
	}
	else if(iTitle == PROG_TRAN_ME){
		//set title
		TCHAR szTxt[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_PROG_TRAN_ME"),theApp.m_szRes,szTxt);
		SetWindowText(szTxt);
		//set description
		al_GetSettingString(_T("public"),_T("IDS_PROG_TRANSFER_TEXT"),theApp.m_szRes,szTxt);
		m_scText.SetWindowText(szTxt);

		m_AnCtrl.ShowWindow(SW_HIDE);
		m_Prog.ShowWindow(SW_SHOW);
		m_scText.ShowWindow(SW_SHOW);
		m_rbnCancel.ShowWindow(SW_SHOW);

	}
	else if(iTitle == PROG_TRAN_SM){
		//set title
		TCHAR szTxt[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_PROG_TRAN_SM"),theApp.m_szRes,szTxt);
		SetWindowText(szTxt);
		//set description
		al_GetSettingString(_T("public"),_T("IDS_PROG_TRANSFER_TEXT"),theApp.m_szRes,szTxt);
		m_scText.SetWindowText(szTxt);

		m_AnCtrl.ShowWindow(SW_HIDE);
		m_Prog.ShowWindow(SW_SHOW);
		m_scText.ShowWindow(SW_SHOW);
		m_rbnCancel.ShowWindow(SW_SHOW);
	}
}

bool CProgDlg::PlayAvi()
{
	m_Prog.ShowWindow(SW_HIDE);
	m_scText.ShowWindow(SW_HIDE);
	m_rbnCancel.ShowWindow(SW_HIDE);

	m_AnCtrl.ShowWindow(SW_SHOW);
	m_AnCtrl.Play(0,-1,-1);

	return true;
}

bool CProgDlg::StopAvi()
{

	m_AnCtrl.Seek(0);
	m_AnCtrl.Stop();

	return true;
}	

bool CProgDlg::CloseAvi()
{

	m_AnCtrl.Close();
	m_AnCtrl.ShowWindow(SW_HIDE);

	m_Prog.ShowWindow(SW_SHOW);
	m_scText.ShowWindow(SW_SHOW);
	m_rbnCancel.ShowWindow(SW_SHOW);
	return true;
}

bool CProgDlg::OpenAvi()
{
	CRect rect;
	if(al_GetSettingRect(_T("avi_ctrl"),_T("rect"),const_cast<TCHAR *>(m_sProfile.c_str()),rect))
		m_AnCtrl.MoveWindow(rect);

	TCHAR szTxt[MAX_PATH],szPath[MAX_PATH];
	if(al_GetSettingString(_T("avi_ctrl"),_T("file"),const_cast<TCHAR *>(m_sProfile.c_str()),szTxt)){
		_tcscpy(szPath,theApp.m_szSkin);
		_tcscat(szPath,szTxt);
		if(!m_AnCtrl.Open(szPath))
			return false;
	}
	else 
		return false;
	
	return true;
}

void CProgDlg::ResetCtrlView()
{
	m_AnCtrl.ShowWindow(SW_SHOW);
	m_Prog.ShowWindow(SW_HIDE);
	m_scText.ShowWindow(SW_HIDE);
	m_rbnCancel.ShowWindow(SW_HIDE);
	OpenAvi();
}
