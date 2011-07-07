// ProcessDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ProcessDlg.h"
#include "SMSutility.h"
#include "UIMessage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProcessDlg dialog

bool CALLBACK Start2Process(BOOL bDone, DWORD uUserData1, DWORD uUserData2)
{
	CProgressCtrlEx *pProcessCtrl = (CProgressCtrlEx*)uUserData1;
	CProcessDlg *pWorker = (CProcessDlg *) uUserData2;
	if(bDone == TRUE) 
	{
		pWorker->PostMessage(WM_COMMAND,  IDC_OK, 0);
		return true;
	}
	if(pWorker->m_pDownloadUtilityfn(1) == false)
		return false;
	// update progress bar
	if(::IsWindow(pProcessCtrl->m_hWnd))
		pProcessCtrl->OffsetPos(1);
	//pProcessCtrl-;
	return true;
}

CProcessDlg::CProcessDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProcessDlg)
	m_hBrush = NULL;
	m_pDownloadUtilityfn = NULL;
	//}}AFX_DATA_INIT
}
CProcessDlg::~CProcessDlg()
{
	if(m_hBrush)
		::DeleteObject(m_hBrush);
}

void CProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcessDlg)
	DDX_Control(pDX, IDC_STATIC_MSG, m_ReadMsg);
	DDX_Control(pDX, IDC_STOP, m_btnStop);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_progressBar);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcessDlg, CDialog)
	//{{AFX_MSG_MAP(CProcessDlg)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_OK, OnOk)
	ON_WM_CANCELMODE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcessDlg message handlers

void CProcessDlg::OnStop() 
{
	// TODO: Add your control notification handler code here
	if(m_Worker.InProgress() )
		m_Worker.Stop();
	//SendMessage(WM_COMMAND, IDOK,0 );
}

BOOL CProcessDlg::OnEraseBkgnd(CDC *pDC)
{
	CRect rect;
	GetClientRect(&rect);
	int bBkMode = ::SetBkMode(pDC->GetSafeHdc(), TRANSPARENT);
	
	Graphics grap(pDC->GetSafeHdc());

	if(m_hBrush)
		::FillRect(pDC->GetSafeHdc(), &rect, m_hBrush);

    /*COLORREF color=(0,0,0);
	HBRUSH frameBrush = CreateSolidBrush(color);
	if(frameBrush)
		::FrameRect(pDC->GetSafeHdc(), &rect, frameBrush);*/
	
	::SetBkMode(pDC->GetSafeHdc(), bBkMode);

	return TRUE;
}
BOOL CProcessDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	TCHAR pSkinPath[_MAX_PATH];
	al_GetModulePath(NULL, pSkinPath);
	TCHAR  szProfile[_MAX_PATH];//, szFileName[_MAX_PATH], buf[_MAX_PATH];
	wsprintf(szProfile, _T("%s%s"), pSkinPath, _T("skin\\default\\SMSUtility\\progressbar.ini"));

    CRect rect;
    COLORREF color;
	//load panel color
	if( !m_hBrush && al_GetSettingColor( _T("panel"), _T("color"), szProfile, color ) )
		m_hBrush = CreateSolidBrush(color);	

	if( al_GetSettingRect( _T("progress"), _T("rect"), szProfile, rect ) )
	{	
		m_progressBar.MoveWindow( &rect, TRUE);
		if(al_GetSettingColor( _T("progress"), _T("color"), szProfile, color ) )
		{
			m_progressBar.SetBarColor(color);
			m_progressBar.SetShowPercent();
		}
	}

	int nSize;
	TCHAR  szFileName[_MAX_PATH], buf[_MAX_PATH], stopbuf[_MAX_PATH];
	if(al_GetSettingString(_T("static"), _T("image"), szProfile, buf))
	{	
		wsprintf(szFileName, _T("%s\\%s"), pSkinPath, buf);
		al_GetSettingString( _T("button_font"), _T("font_name"), (LPTSTR)(LPCTSTR)szProfile, buf );
		al_GetSettingInt( _T("button_font"), _T("font_size"), (LPTSTR)(LPCTSTR)szProfile, nSize );

		CRect stRect;
		al_GetSettingRect(_T("ShowText"), _T("rect"), (LPTSTR)(LPCTSTR)szProfile, stRect);
		m_ReadMsg.LoadBitmap(szFileName);
		m_ReadMsg.SetTextFont( CString(buf), nSize );
		m_ReadMsg.SetBrush(m_hBrush);
		m_ReadMsg.MoveWindow(&stRect);	
		
		TCHAR szTmp[MAX_PATH];
		al_GetSettingString(_T("public"), _T("ShowMsg"), theApp.m_szRes, szTmp);
		//if( !al_GetSettingColor("version", "font_color_normal", szProfile, color) )
			//m_ReadMsg.SetTextColor(&color);
		m_ReadMsg.SetWindowText(szTmp);

		al_GetSettingString(_T("public"), _T("progreTitle"), theApp.m_szRes, szTmp);
		SetWindowText(szTmp);

		al_GetSettingString(_T("public"), _T("IDS_CANCEL"), theApp.m_szRes, stopbuf);
		m_btnStop.SetTextFont( CString(theApp.m_lf.lfFaceName), theApp.m_lf.lfHeight );
		m_btnStop.SetWindowText(stopbuf);
	
	}

	


	m_btnStop.SetButtonStyle(BS_OWNERDRAW);
	GetButtonFromSetting(&m_btnStop , _T("stop") , IDS_456 , 0, szProfile );
	m_btnStop.SetTextFont( CString(buf), 12 );
	m_btnStop.SetWindowText(stopbuf);
	if( al_GetSettingRect( _T("stop"), _T("rect"), szProfile, rect ) )
	{	
		m_btnStop.MoveWindow( &rect, TRUE);
	}
	m_btnStop.ShowWindow(SW_SHOW);
	//m_btnStop.ShowWindow(SW_HIDE);

	// Panel
	al_GetSettingRect(_T("panel"),_T("rect"),(LPTSTR)(LPCTSTR)szProfile,rect);

	//set progress bar setting
	COLORREF white, blue;
	white = RGB(255,255,255);
	blue =  RGB(0,0,255);
	m_brBar.CreateSolidBrush( blue );
	m_progressBar.SetBarBrush(&m_brBar);
	//m_progressBar.SetShowPercent();
	m_progressBar.SetBkColor( white );
	m_progressBar.SetTextColor( white, blue );

	CRect rc;
	GetWindowRect(&rc);
	int nCapHeight = ::GetSystemMetrics(SM_CYCAPTION);
	int nDlgFrmX = ::GetSystemMetrics(SM_CXDLGFRAME);
	int nDlgFrmY = ::GetSystemMetrics(SM_CYDLGFRAME);
	::MoveWindow( GetSafeHwnd(), rc.left, rc.top,  rect.Width() + nDlgFrmX*2, 
				nCapHeight + nDlgFrmY*2 + rect.Height(), TRUE );
	
	
	

	SendMessage(WM_COMMAND, IDC_START,0 );
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProcessDlg::OnStart() 
{
	// TODO: Add your control notification handler code here
	m_progressBar.SetRange(0, m_nTotal);
	m_progressBar.SetPos(1);

	m_Worker.SetLoopCount(m_nTotal);
	m_Worker.Start(Start2Process, (DWORD)(void *)&m_progressBar, (DWORD)this);
	
}

void CProcessDlg::OnOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();	
}

void CProcessDlg::OnCancelMode() 
{
	CDialog::OnCancelMode();
	
	// TODO: Add your message handler code here
	
}

void CProcessDlg::OnClose() 
{
	CDialog::OnClose();
	
	// TODO: Add your message handler code here
	
}


void CProcessDlg::OnCancel()
{
	if(m_Worker.InProgress() )
		m_Worker.Stop();
}
