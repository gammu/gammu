// UserExitDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "camm.h"
#include "UserExitDlg.h"
#include "loadstring.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUserExitDlg dialog


CUserExitDlg::CUserExitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUserExitDlg::IDD, pParent)
{

	//{{AFX_DATA_INIT(CUserExitDlg)
	//}}AFX_DATA_INIT

}
CUserExitDlg::~CUserExitDlg()
{
	DeleteObject(m_hBrush);
}

void CUserExitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUserExitDlg)
	DDX_Control(pDX, IDC_PVI_5_UE_DISCARD, m_ctrlDiscard);
	DDX_Control(pDX, IDC_PVI_5_UE_SAVEAS, m_ctrlSaveAs);
	DDX_Control(pDX, IDC_PVI_5_UE_SAVE, m_ctrlOk);
	DDX_Control(pDX, IDC_PVI_5_UE_CANCEL, m_ctrlCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUserExitDlg, CDialog)
	//{{AFX_MSG_MAP(CUserExitDlg)
	ON_BN_CLICKED(IDC_PVI_5_UE_CANCEL, OnPvi5UeCancel)
	ON_BN_CLICKED(IDC_PVI_5_UE_DISCARD, OnPvi5UeDiscard)
	ON_BN_CLICKED(IDC_PVI_5_UE_SAVE, OnPvi5UeSave)
	ON_BN_CLICKED(IDC_PVI_5_UE_SAVEAS, OnPvi5UeSaveas)
	ON_WM_CTLCOLOR()

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUserExitDlg message handlers

BOOL CUserExitDlg::OnInitDialog() 
{

	CDialog::OnInitDialog();
	
	m_hBrush=::CreateSolidBrush(RGB(235, 243, 246));

	// TODO: Add extra initialization here
	CString StringLoaded;
	StringLoaded = LoadStringFromFile(_T("ExitDlg"),_T("DLG_SAVE_BTN"));
	SetDlgItemText(IDC_PVI_5_UE_SAVE,StringLoaded);
	
	StringLoaded = LoadStringFromFile(_T("ExitDlg"),_T("DLG_SAVEAS_BTN"));
	SetDlgItemText(IDC_PVI_5_UE_SAVEAS,StringLoaded);

	StringLoaded = LoadStringFromFile(_T("ExitDlg"),_T("DLG_DISCARD_BTN"));
	SetDlgItemText(IDC_PVI_5_UE_DISCARD,StringLoaded);

	StringLoaded = LoadStringFromFile(_T("ExitDlg"),_T("DLG_CANCEL_BTN"));
	SetDlgItemText(IDC_PVI_5_UE_CANCEL,StringLoaded);

	StringLoaded = LoadStringFromFile(_T("ExitDlg"),_T("DLG_MSG_STATIC"));
	SetDlgItemText(IDC_PVI_5_UE_MSG1,StringLoaded);

//	StringLoaded = LoadStringFromFile(ghSkinResource,IDS_PVI_5_UE_CAPTION);
//	SetWindowText(StringLoaded);

	Init();




	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}






void CUserExitDlg::OnPvi5UeCancel() 
{
	// TODO: Add your control notification handler code here

	m_nDecision = IDC_PVI_5_UE_CANCEL;
	CDialog::OnOK();

}

void CUserExitDlg::OnPvi5UeDiscard() 
{
	// TODO: Add your control notification handler code here

	m_nDecision = IDC_PVI_5_UE_DISCARD;
	CDialog::OnOK();

}

void CUserExitDlg::OnPvi5UeSave() 
{
	// TODO: Add your control notification handler code here

	m_nDecision = IDC_PVI_5_UE_SAVE;
	CDialog::OnOK();

}

void CUserExitDlg::OnPvi5UeSaveas() 
{
	// TODO: Add your control notification handler code here

	m_nDecision = IDC_PVI_5_UE_SAVEAS;
	CDialog::OnOK();

}

INT CUserExitDlg::GetDecision()
{
	return m_nDecision;
}

HBRUSH CUserExitDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	pDC->SetBkColor(RGB(235, 243, 246));
	return m_hBrush;
}

BOOL CUserExitDlg::Init()
{
	
	return TRUE;
}

void CUserExitDlg::LoadSkinEnglish()
{

	TCHAR imageLocation[MAX_PATH];
	_tcscpy(imageLocation,gstrImageLocation);
	_tcscat(imageLocation,_T("\\Dlg"));
	CString strPath;
	CRect rc;

	//Mod by nivas
	strPath.Format(_T("%s\\Save.png"),imageLocation);
	m_ctrlOk.LoadBitmap(strPath);
	m_ctrlOk.GetWindowRect(&rc);
	ScreenToClient(&rc);
	m_ctrlOk.MoveWindow(rc.left,rc.top,69,25);

	strPath.Format(_T("%s\\Save as.png"), imageLocation);
	m_ctrlSaveAs.LoadBitmap(strPath);
	m_ctrlSaveAs.MoveWindow(rc.left+89, rc.top, 69, 25);

	strPath.Format(_T("%s\\Discard.png"), imageLocation);
	m_ctrlDiscard.LoadBitmap(strPath);
	m_ctrlDiscard.MoveWindow(rc.left+178, rc.top, 69, 25);

	strPath.Format(_T("%s\\Cancel.png"),imageLocation);
	m_ctrlCancel.LoadBitmap(strPath);
	m_ctrlCancel.MoveWindow(rc.left+267,rc.top,69,25);
	//Mod by nivas

}

void CUserExitDlg::LoadSkinTC()
{

	TCHAR imageLocation[MAX_PATH];
	_tcscpy(imageLocation,gstrImageLocation);
	_tcscat(imageLocation,_T("\\Dlg"));
	CString strPath;
	CRect rc;

	//Mod by nivas
	strPath.Format(_T("%s\\tc-Save.png"),imageLocation);
	m_ctrlOk.LoadBitmap(strPath);
	m_ctrlOk.GetWindowRect(&rc);
	ScreenToClient(&rc);
	m_ctrlOk.MoveWindow(rc.left,rc.top,69,25);

	strPath.Format(_T("%s\\Save as.png"), imageLocation);
	m_ctrlSaveAs.LoadBitmap(strPath);
	m_ctrlSaveAs.MoveWindow(rc.left+89, rc.top, 69, 25);

	strPath.Format(_T("%s\\Discard.png"), imageLocation);
	m_ctrlDiscard.LoadBitmap(strPath);
	m_ctrlDiscard.MoveWindow(rc.left+178, rc.top, 69, 25);

	strPath.Format(_T("%s\\Cancel.png"),imageLocation);
	m_ctrlCancel.LoadBitmap(strPath);
	m_ctrlCancel.MoveWindow(rc.left+267,rc.top,69,25);
	//Mod by nivas

}

void CUserExitDlg::LoadSkinSC()
{
	TCHAR imageLocation[MAX_PATH];
	_tcscpy(imageLocation,gstrImageLocation);
	_tcscat(imageLocation,_T("\\Dlg"));
	CString strPath;
	CRect rc;

	//Mod by nivas
	strPath.Format(_T("%s\\sc-Save.png"),imageLocation);
	m_ctrlOk.LoadBitmap(strPath);
	m_ctrlOk.GetWindowRect(&rc);
	ScreenToClient(&rc);
	m_ctrlOk.MoveWindow(rc.left,rc.top,69,25);

	strPath.Format(_T("%s\\Save as.png"), imageLocation);
	m_ctrlSaveAs.LoadBitmap(strPath);
	m_ctrlSaveAs.MoveWindow(rc.left+89, rc.top, 69, 25);

	strPath.Format(_T("%s\\Discard.png"), imageLocation);
	m_ctrlDiscard.LoadBitmap(strPath);
	m_ctrlDiscard.MoveWindow(rc.left+178, rc.top, 69, 25);

	strPath.Format(_T("%s\\Cancel.png"),imageLocation);
	m_ctrlCancel.LoadBitmap(strPath);
	m_ctrlCancel.MoveWindow(rc.left+267,rc.top,69,25);
	//Mod by nivas

}
