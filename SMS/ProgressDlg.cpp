// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ProgressDlg.h"
#include "SMSutility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
BOOL g_bCancel = FALSE;
/////////////////////////////////////////////////////////////////////////////
extern InitialMemoryStatusProc		InitialMemoryStatusfn;
// CProgressDlg dialog
UINT GetSMSThreadProc(CProgressDlg*  pDlg)
{
	int ret = 0;
	if(theApp.nIsConnected)
	{
		ANWGetMobileAllSMS(pDlg->m_pSMSStatus);
	}
    ::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);
   return 0;
}
UINT GetThreadProc(CProgressDlg*  pDlg)
{
	int ret = 0;
	if(pDlg->m_pContactsDlg && theApp.nIsConnected)
	{
//			pDlg->m_pContactsDlg->GetMobileSetting();

	
			InitPhoneBookfn();
			Contact_Tal_Num ncontact_Tal_Num;
			ret = GetPhoneBookInfofn(&ncontact_Tal_Num);
					
			if(ret != Anw_SUCCESS) goto _exit;

			int free =0 ;
			int nUsed = 0;
			pDlg->m_AnCtrl.Play(0,-1,-1);

			if(ncontact_Tal_Num.MEContactUsedNum == -1)
			{
				ret = InitialMemoryStatusfn("ME",&nUsed, &free);
				if(ret !=Anw_SUCCESS) goto _exit;
				pDlg->m_pContactsDlg->m_nMEUsedNum = nUsed;
			}
			else 
				pDlg->m_pContactsDlg->m_nMEUsedNum = ncontact_Tal_Num.MEContactUsedNum;

			int nNeedCount = pDlg->m_pContactsDlg->m_nMEUsedNum;// + s_ActiveDlg->m_nSIMUsedNum;
		//	BOOL bStart = TRUE;
			while(nNeedCount > 0 && g_bCancel == FALSE)
			{
			/*	ret = pDlg->GetPBOneData("ME\0", MEM_SM, pDlg->m_pContactsDlg->m_nNextPost, bStart);
				if(ret != 1)
				{
					TCHAR str1[MAX_PATH];
					al_GetSettingString(_T("public"), "IDS_ERR_READFAILED", theApp.m_szRes, str1);
					AfxMessageBox(str1);
					pDlg->m_pContactsDlg->m_nPos = pDlg->m_pContactsDlg->m_nMEUsedNum-1;
					break;
				}
				pDlg->m_pContactsDlg->m_nPos++;
				if(ret == ERR_NONE)
					pDlg->m_pContactsDlg->m_nNextPost++;

				bStart= false;
				nNeedCount--;*/
				pDlg->m_pContactsDlg->GetAllPBData(1);
				nNeedCount--;
			}


			if(g_bCancel) goto _exit;
			
			free =0 ;
			nUsed = 0;
			if(ncontact_Tal_Num.SIMContactUsedNum == -1)
			{
				ret = InitialMemoryStatusfn("SM",&nUsed, &free);
				if(ret !=Anw_SUCCESS) goto _exit;
				pDlg->m_pContactsDlg->m_nSIMUsedNum = nUsed;
			}
			else 
				pDlg->m_pContactsDlg->m_nSIMUsedNum = ncontact_Tal_Num.SIMContactUsedNum;

			nNeedCount = pDlg->m_pContactsDlg->m_nSIMUsedNum;// + s_ActiveDlg->m_nSIMUsedNum;
		//	bStart = TRUE;
			while(nNeedCount > 0 && g_bCancel == FALSE)
			{
			/*	ret = pDlg->GetPBOneData("SM\0", MEM_SM, pDlg->m_pContactsDlg->m_nNextPost,bStart);
				if(ret != 1)
				{
					TCHAR str1[MAX_PATH];
					al_GetSettingString(_T("public"), "IDS_ERR_READFAILED", theApp.m_szRes, str1);
					AfxMessageBox(str1);
					pDlg->m_pContactsDlg->m_nPos = pDlg->m_pContactsDlg->m_nSIMUsedNum-1;
					break;
				}
				pDlg->m_pContactsDlg->m_nPos++;
				if(ret == ERR_NONE)
					pDlg->m_pContactsDlg->m_nNextPost++;

				
				bStart= false;
				nNeedCount--;*/

				pDlg->m_pContactsDlg->GetAllPBData(1);
				nNeedCount--;
			}
	}
_exit:	
    ::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);

   return 0;
}

CProgressDlg::CProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgressDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pGetThread=NULL;
	m_hBrush = NULL;
	m_nProgressType = 1;
}


void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressDlg)
//	DDX_Control(pDX, IDC_STATIC_MSG, m_staticText);
	DDX_Control(pDX, IDCANCEL, m_CancelButton);
	DDX_Control(pDX, IDC_ANIMATE, m_AnCtrl);
//	DDX_Control(pDX, IDC_PROGRESS_BAR, m_ProgressCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	//{{AFX_MSG_MAP(CProgressDlg)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg message handlers

BOOL CProgressDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
    g_bCancel = FALSE; 
	
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

/*	if( al_GetSettingRect( "progress", "rect", szProfile, rect ) )
	{	
		m_ProgressCtrl.MoveWindow( &rect, TRUE);
		if(al_GetSettingColor( "progress", "color", szProfile, color ) )
		{
			m_ProgressCtrl.SetBarColor(color);
			m_ProgressCtrl.SetShowPercent();
		}
	}*/

/*	int nSize;
	TCHAR  szFileName[_MAX_PATH], buf[_MAX_PATH], stopbuf[_MAX_PATH];
	if(al_GetSettingString("static", _T("image"), szProfile, buf))
	{	
		wsprintf(szFileName, "%s\\%s", pSkinPath, buf);
		al_GetSettingString( _T("button_font"), _T("font_name"), (LPTSTR)(LPCTSTR)szProfile, buf );
		al_GetSettingInt( _T("button_font"), _T("font_size"), (LPTSTR)(LPCTSTR)szProfile, nSize );

		CRect stRect;
		al_GetSettingRect("ShowText", "rect", (LPTSTR)(LPCTSTR)szProfile, stRect);
		m_ReadMsg.LoadBitmap(szFileName);
		m_ReadMsg.SetTextFont( CString(buf), nSize );
		m_ReadMsg.SetBrush(m_hBrush);
		m_ReadMsg.MoveWindow(&stRect);	
		
		TCHAR szTmp[MAX_PATH];
		al_GetSettingString(_T("public"), "ShowMsg", theApp.m_szRes, szTmp);
		//if( !al_GetSettingColor("version", "font_color_normal", szProfile, color) )
			//m_ReadMsg.SetTextColor(&color);
		m_ReadMsg.SetWindowText(szTmp);

		al_GetSettingString(_T("public"), "progreTitle", theApp.m_szRes, szTmp);
		SetWindowText(szTmp);

		al_GetSettingString(_T("public"), "IDS_CANCEL", theApp.m_szRes, stopbuf);
		m_btnStop.SetTextFont( CString(theApp.m_lf.lfFaceName), theApp.m_lf.lfHeight );
		m_btnStop.SetWindowText(stopbuf);
	
	}

	
*/
	int nSize;
	TCHAR  szFileName[_MAX_PATH], buf[_MAX_PATH], stopbuf[_MAX_PATH];
	if(al_GetSettingString(_T("static"), _T("image"), szProfile, buf))
	{	
		wsprintf(szFileName, _T("%s\\%s"), pSkinPath, buf);
		al_GetSettingString( _T("button_font"), _T("font_name"), (LPTSTR)(LPCTSTR)szProfile, buf );
		al_GetSettingInt( _T("button_font"), _T("font_size"), (LPTSTR)(LPCTSTR)szProfile, nSize );
		al_GetSettingString(_T("public"), _T("IDS_CANCEL"), theApp.m_szRes, stopbuf);
		m_CancelButton.SetTextFont( CString(theApp.m_lf.lfFaceName), theApp.m_lf.lfHeight );
		m_CancelButton.SetWindowText(stopbuf);
	}
	TCHAR szTmp[MAX_PATH];
	if(m_nProgressType == 2)
		al_GetSettingString(_T("public"), _T("progreTitle"), theApp.m_szRes, szTmp);
	else
		al_GetSettingString(_T("public"), _T("readPhonebookTitle"), theApp.m_szRes, szTmp);
	SetWindowText(szTmp);

	m_CancelButton.SetButtonStyle(BS_OWNERDRAW);
	GetButtonFromSetting(&m_CancelButton , _T("stop") , IDS_456 , 0, szProfile );
	m_CancelButton.SetTextFont( CString(buf), 12 );
	m_CancelButton.SetWindowText(stopbuf);
	if( al_GetSettingRect( _T("stop"), _T("rect"), szProfile, rect ) )
	{	
		m_CancelButton.MoveWindow( &rect, TRUE);
	}
	m_CancelButton.ShowWindow(SW_HIDE);

	// Panel
	al_GetSettingRect(_T("panel"),_T("rect"),(LPTSTR)(LPCTSTR)szProfile,rect);

	//set progress bar setting
/*	COLORREF white, blue;
	white = RGB(255,255,255);
	blue =  RGB(0,0,255);
	m_brBar.CreateSolidBrush( blue );
	m_ProgressCtrl.SetBarBrush(&m_brBar);
	//m_ProgressCtrl.SetShowPercent();
	m_ProgressCtrl.SetBkColor( white );
	m_ProgressCtrl.SetTextColor( white, blue );
*/
	CRect rc;
	GetWindowRect(&rc);
	int nCapHeight = ::GetSystemMetrics(SM_CYCAPTION);
	int nDlgFrmX = ::GetSystemMetrics(SM_CXDLGFRAME);
	int nDlgFrmY = ::GetSystemMetrics(SM_CYDLGFRAME);
	::MoveWindow( GetSafeHwnd(), rc.left, rc.top,  rect.Width() + nDlgFrmX*2, 
				nCapHeight + nDlgFrmY*2 + rect.Height(), TRUE );
	
	
	OpenAvi();
	if(m_nProgressType == 2)
		m_pGetThread = AfxBeginThread((AFX_THREADPROC)GetSMSThreadProc,  this, THREAD_PRIORITY_NORMAL);
	else
		m_pGetThread = AfxBeginThread((AFX_THREADPROC)GetThreadProc,  this, THREAD_PRIORITY_NORMAL);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProgressDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	if(m_pGetThread)
	{
		m_pGetThread->SuspendThread();
		g_bCancel = TRUE;
		m_pGetThread->ResumeThread();
		UpdateData(FALSE);
		m_CancelButton.EnableWindow(FALSE);
	}
	else CDialog::OnCancel();
	
//	CDialog::OnCancel();
}

void CProgressDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rect,rc;
	GetClientRect(&rect);
	int bBkMode = ::SetBkMode(dc, TRANSPARENT);

	if(m_hBrush)
		::FillRect(dc, &rect, m_hBrush);

	::SetBkMode(dc, bBkMode);
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CProgressDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	if(m_pGetThread)
	{
	//	m_pGetThread->SuspendThread();
		g_bCancel = TRUE;
	//	m_pGetThread->ResumeThread();
		UpdateData(FALSE);
		m_CancelButton.EnableWindow(FALSE);
	}
	else
		CDialog::OnClose();	
}

bool CProgressDlg::OpenAvi()
{
	CRect rect;
	TCHAR pSkinPath[_MAX_PATH];
	al_GetModulePath(NULL, pSkinPath);
	TCHAR  szProfile[_MAX_PATH];//, szFileName[_MAX_PATH], buf[_MAX_PATH];
	wsprintf(szProfile, _T("%s%s"), pSkinPath, _T("skin\\default\\SMSUtility\\progressbar.ini"));
	if(al_GetSettingRect(_T("avi_ctrl"),_T("rect"),(LPTSTR)(LPCTSTR)szProfile,rect))
		m_AnCtrl.MoveWindow(rect);

	TCHAR szTxt[MAX_PATH],szPath[MAX_PATH];
	if(al_GetSettingString(_T("avi_ctrl"),_T("file"),(LPTSTR)(LPCTSTR)szProfile,szTxt)){
		_tcscpy(szPath,theApp.m_szSkin);
		_tcscat(szPath,szTxt);
		if(!m_AnCtrl.Open(szPath))
			return false;
	}
	else 
		return false;
	
	return true;
}
/*
int ParserPBData(GSM_MemoryEntry entryList, ContactList &contactInfo)
{
	int EntriesNum = entryList.EntriesNum;


	bool nMobilePhone = false;	
	bool bHasName = false;
//	int len= sizeof(entryList.Entries[0].Text);
	for(int Col_Num = 0; Col_Num < EntriesNum; Col_Num++)
	{
		switch(entryList.Entries[Col_Num].EntryType)
		{
			case PBK_Text_Name : 
				memcpy(contactInfo.strName, entryList.Entries[Col_Num].Text, strlen((const char*)entryList.Entries[Col_Num].Text));
				bHasName = true;
				break;

			case PBK_Number_Mobile : 
				memcpy(contactInfo.strTel, entryList.Entries[Col_Num].Text, strlen((const char*)entryList.Entries[Col_Num].Text));
				nMobilePhone = true;
				break;

			case PBK_Number_General : 
				memcpy(contactInfo.strTel, entryList.Entries[Col_Num].Text, strlen((const char*)entryList.Entries[Col_Num].Text));
				nMobilePhone = true;
				break;

			case PBK_Number_Home : 
				memcpy(contactInfo.strTel, entryList.Entries[Col_Num].Text, strlen((const char*)entryList.Entries[Col_Num].Text));
				nMobilePhone = true;
				break;

			case PBK_Number_Work : 
				memcpy(contactInfo.strTel, entryList.Entries[Col_Num].Text, strlen((const char*)entryList.Entries[Col_Num].Text));
				nMobilePhone = true;
				break;

			case PBK_Number_Other : 
				memcpy(contactInfo.strTel, entryList.Entries[Col_Num].Text, strlen((const char*)entryList.Entries[Col_Num].Text));
				nMobilePhone = true;
				break;
		}

		if(nMobilePhone == true && bHasName)
			break;

	}

	return true;
}

int CProgressDlg::GetPBOneData(char *strMemType, int nMemtype, int nPos, bool bStart)
{
	int ret = 0;
	// Get ALl Phone Number
	GSM_MemoryEntry entryList;
	memset(&entryList, 0, sizeof(GSM_MemoryEntry));
	
	//get the one data
	if(bStart == true)
	{
		ret = GetPhoneBookStartDatafn(strMemType, &entryList); 
	}
	else
		ret = GetPhoneBookNextDatafn(strMemType, &entryList, bStart); 

	if(ret == ERR_NONE)
	{
		ContactList contactList;
		memset(&contactList, 0, sizeof(ContactList));
		ParserPBData(entryList, contactList);

		if(strlen(contactList.strName) != 0 && strlen(contactList.strTel) != 0)
		{
			_tcscpy(m_pContactsDlg->m_pContactInfo[nPos].strName, contactList.strName);
			_tcscpy(m_pContactsDlg->m_pContactInfo[nPos].strTel, contactList.strTel);
			m_pContactsDlg->m_pContactInfo[nPos].nStoreDevice= nMemtype;
		}
		//else
		//	return false;
	}

	return ret;
}*/