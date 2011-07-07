// OpenSMS.cpp : implementation file
//

#include "stdafx.h"
#include "SMSUtility.h"
#include "OpenSMS.h"
#include "NEWSMS.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COpenSMS dialog


COpenSMS::COpenSMS(CWnd* pParent /*=NULL*/)
	: CDialog(COpenSMS::IDD, pParent)
{
	//{{AFX_DATA_INIT(COpenSMS)
	//}}AFX_DATA_INIT

	m_hBrush= NULL;
	m_hFont = NULL;
	m_bSender = false;
}
COpenSMS::~COpenSMS()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}

void COpenSMS::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpenSMS)
	DDX_Control(pDX, IDC_BTN_NEXT, m_BTN_Next);
	DDX_Control(pDX, IDC_BTN_REPLY, m_BTN_Reply);
	DDX_Control(pDX, IDC_BTN_PREV, m_BTN_Prev);
	DDX_Control(pDX, IDCANCEL, m_BTN_Cancel);
	DDX_Control(pDX, IDC_EDIT_CONTENT, m_EDIT_Content);
	DDX_Control(pDX, IDC_EDIT_SENDER, m_EDIT_Sender);
	DDX_Control(pDX, IDC_EDIT_TIME, m_EDIT_Time);
	DDX_Control(pDX, IDC_ST_MSGCONTENT, m_ST_MsgContent);
	DDX_Control(pDX, IDC_ST_SENDER, m_ST_Sender);
	DDX_Control(pDX, IDC_ST_TIME, m_ST_Time);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpenSMS, CDialog)
	//{{AFX_MSG_MAP(COpenSMS)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_PREV, OnBtnPrev)
	ON_BN_CLICKED(IDC_BTN_NEXT, OnBtnNext)
	ON_BN_CLICKED(IDC_BTN_REPLY, OnBtnReply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpenSMS message handlers

BOOL COpenSMS::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	al_GetModulePath(NULL, m_szAppPath);
    LoadControl(m_szAppPath);
	InitValue();
	InitString();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COpenSMS::LoadControl(LPCTSTR pSkinPath)
{
	TCHAR  szFileName[_MAX_PATH], szProfile[_MAX_PATH], buf[_MAX_PATH];
	int iSize = 0;
	TCHAR szName[MAX_PATH];
	CFont* pFont = NULL;
	if(GetProfileFont(NULL,NULL,iSize,szName))
	{
		iSize = -iSize ;
		if(m_hFont==NULL)
			m_hFont = GetFontEx(szName,iSize);
		pFont = CFont::FromHandle(m_hFont);
	}

	wsprintf(szProfile, _T("%s%s"), pSkinPath, _T("skin\\default\\SMSUtility\\OPENSMS.ini"));

	//load panel brush
	COLORREF bksColor;
	if( !al_GetSettingColor(_T("panel"), _T("color"), szProfile, bksColor) )
		bksColor = 11012976;
		
	if(m_hBrush==NULL)	
		m_hBrush = ::CreateSolidBrush(bksColor);

	// Button(Contacts, Send, Caancel)
	m_pButton[0] = &m_BTN_Prev;
	m_pButton[1] = &m_BTN_Next;
	m_pButton[2] = &m_BTN_Reply;
	m_pButton[3] = &m_BTN_Cancel;

	TCHAR szSkin[ITEM_BTN_NUM][32] = {_T("Previous"), _T("Next"), _T("Reply"), _T("Cancel")};
	UINT nToolTip[ITEM_BTN_NUM] = {IDS_123, IDS_123, IDS_123, IDS_123};

	int nWidth = 0;	//store the maxium button width
	for(int i = 0 ; i < ITEM_BTN_NUM ; i++)
	{
		BOOL bRet = FALSE;
		GetButtonFromSetting(m_pButton[i], szSkin[i], nToolTip[i], 0, (LPTSTR)(LPCTSTR)szProfile);
		CRect rc;
		m_pButton[i]->GetWindowRect(&rc);
		nWidth = rc.Width() > nWidth ? rc.Width() : nWidth;
	}

	for(i = 0 ; i < ITEM_BTN_NUM ; i ++)
	{
		CRect rc,rcPrev;
		m_pButton[i]->GetWindowRect(&rc);
		ScreenToClient(&rc);
		rc.right = rc.left + nWidth + 6;
		/*if(i > 0){
			m_pButton[i-1]->GetWindowRect(&rcPrev);
			ScreenToClient(&rcPrev);
			rc.OffsetRect(rcPrev.right + SSD_BN_OFFSET - rc.left,0);
		}
		else{
			rc.OffsetRect(SSD_BN_OFFSET - rc.left,0);
		}*/
		m_pButton[i]->MoveWindow(&rc);	
	}


	// Static Text(Receiver, MsgContent, count[0/160])
	////int nSize;
	if(al_GetSettingString(_T("static"), _T("image"), szProfile, buf))
	{	
		wsprintf(szFileName, _T("%s\\%s"), pSkinPath, buf);
		////al_GetSettingString( "button_font", "font_name", (LPTSTR)(LPCTSTR)szProfile, buf );
		////al_GetSettingInt( "button_font", "font_size", (LPTSTR)(LPCTSTR)szProfile, nSize );

		CRect stRect;
		al_GetSettingRect(_T("SenderST"), _T("rect"), (LPTSTR)(LPCTSTR)szProfile, stRect);
		m_ST_Sender.LoadBitmap(szFileName);
		m_ST_Sender.SetTextFont( CString(theApp.m_lf.lfFaceName), theApp.m_lf.lfHeight );
		m_ST_Sender.SetBrush(m_hBrush);
		m_ST_Sender.MoveWindow(&stRect);	

		al_GetSettingRect(_T("TimeST"), _T("rect"), (LPTSTR)(LPCTSTR)szProfile, stRect);
		m_ST_Time.LoadBitmap(szFileName);
		m_ST_Time.SetTextFont( CString(theApp.m_lf.lfFaceName), theApp.m_lf.lfHeight );
		m_ST_Time.SetBrush(m_hBrush);
		m_ST_Time.MoveWindow(&stRect);	

		al_GetSettingRect(_T("MsgContentST"), _T("rect"), (LPTSTR)(LPCTSTR)szProfile, stRect);
		m_ST_MsgContent.LoadBitmap(szFileName);
		m_ST_MsgContent.SetTextFont( CString(theApp.m_lf.lfFaceName), theApp.m_lf.lfHeight );
		m_ST_MsgContent.SetBrush(m_hBrush);
		m_ST_MsgContent.MoveWindow(&stRect);

		
	}


	CRect rc,rect;
	// Edit(Msg Content)
	al_GetSettingRect(_T("SenderEdit"),_T("rect"),(LPTSTR)(LPCTSTR)szProfile,rect);
	m_EDIT_Sender.MoveWindow(&rect);
	
	al_GetSettingRect(_T("TimeEdit"),_T("rect"),(LPTSTR)(LPCTSTR)szProfile,rect);
	m_EDIT_Time.MoveWindow(&rect);

	al_GetSettingRect(_T("ContentEdit"),_T("rect"),(LPTSTR)(LPCTSTR)szProfile,rect);
	m_EDIT_Content.MoveWindow(&rect);

	if(pFont)
	{
		m_EDIT_Sender.SetFont(pFont);
		m_EDIT_Time.SetFont(pFont);
		m_EDIT_Content.SetFont(pFont);
	}
	// Panel
	al_GetSettingRect(_T("panel"),_T("rect"),(LPTSTR)(LPCTSTR)szProfile,rect);

	GetWindowRect(&rc);
	int nCapHeight = ::GetSystemMetrics(SM_CYCAPTION);
	int nDlgFrmX = ::GetSystemMetrics(SM_CXDLGFRAME);
	int nDlgFrmY = ::GetSystemMetrics(SM_CYDLGFRAME);
	::MoveWindow( GetSafeHwnd(), rc.left, rc.top,  rect.Width() + nDlgFrmX*2, 
				nCapHeight + nDlgFrmY*2 + rect.Height(), TRUE );
	
}

void COpenSMS::InitString(void)
{
	CString str;
	TCHAR szTmp[MAX_PATH];
	
	al_GetSettingString(_T("public"), _T("IDS_DISPLAYMSG"), theApp.m_szRes, szTmp);			// Title
	SetWindowText(szTmp);

	if(m_bSender == true)
		al_GetSettingString(_T("public"), _T("IDS_RECEIVER_"), theApp.m_szRes, szTmp);
	else
		al_GetSettingString(_T("public"), _T("IDS_SENDER"), theApp.m_szRes, szTmp);
	m_ST_Sender.SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_TIME"), theApp.m_szRes, szTmp);	
	m_ST_Time.SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_CONTENTS"), theApp.m_szRes, szTmp);
	m_ST_MsgContent.SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_PREVIOUS"), theApp.m_szRes, szTmp);	
	m_BTN_Prev.SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_NEXT"), theApp.m_szRes, szTmp);	
	m_BTN_Next.SetWindowText(szTmp);
   	if(m_bSender == true)
    {
	 al_GetSettingString(_T("public"), _T("IDS_SEND"), theApp.m_szRes, szTmp);
    }
	else
	al_GetSettingString(_T("public"), _T("IDS_REPLY"), theApp.m_szRes, szTmp);	
	m_BTN_Reply.SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_CANCEL"), theApp.m_szRes, szTmp);
	m_BTN_Cancel.SetWindowText(szTmp);

}

void COpenSMS::InitValue()
{
	m_EDIT_Sender.SetWindowText(m_MsgInfo[m_nCurrentItem].TPA);
	m_EDIT_Time.SetWindowText(m_MsgInfo[m_nCurrentItem].TP_SCTS);
	m_EDIT_Content.SetWindowText(m_MsgInfo[m_nCurrentItem].TP_UD);

	if(m_nCurrentItem == 0)
		m_BTN_Prev.EnableWindow(false);

	if(m_nCurrentItem == m_MaxItems-1)
		m_BTN_Next.EnableWindow(false);
}

void COpenSMS::OnPaint() 
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


void COpenSMS::SetData(SMS_PARAM *msgInfo, int nCurrentItem, int nMaxItems,bool bSender)
{
	m_MsgInfo		= msgInfo;
	m_nCurrentItem	= nCurrentItem;
	m_MaxItems		= nMaxItems;
	m_bSender = bSender;
}

void COpenSMS::OnBtnPrev() 
{
	// TODO: Add your control notification handler code here
	m_nCurrentItem--;

	m_EDIT_Sender.SetWindowText(m_MsgInfo[m_nCurrentItem].TPA);
	m_EDIT_Time.SetWindowText(m_MsgInfo[m_nCurrentItem].TP_SCTS);
	m_EDIT_Content.SetWindowText(m_MsgInfo[m_nCurrentItem].TP_UD);

	
	if(m_nCurrentItem == 0)
		m_BTN_Prev.EnableWindow(false);

	m_BTN_Next.EnableWindow(true);
}

void COpenSMS::OnBtnNext() 
{
	// TODO: Add your control notification handler code here
	m_nCurrentItem++;

	m_EDIT_Sender.SetWindowText(m_MsgInfo[m_nCurrentItem].TPA);
	m_EDIT_Time.SetWindowText(m_MsgInfo[m_nCurrentItem].TP_SCTS);
	m_EDIT_Content.SetWindowText(m_MsgInfo[m_nCurrentItem].TP_UD);


	if(m_nCurrentItem == m_MaxItems-1)
		m_BTN_Next.EnableWindow(false);
	
	m_BTN_Prev.EnableWindow(true);
}

void COpenSMS::OnBtnReply() 
{
	// TODO: Add your control notification handler code here
	CNewSMS	ResendSMSDlg;

	ResendSMSDlg.SetData(m_MsgInfo[m_nCurrentItem], SMSRpely, false,m_bSender);
	ResendSMSDlg.DoModal();
	
}
