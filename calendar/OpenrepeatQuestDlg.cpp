// OpenrepeatQuestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "calendar.h"
#include "OpenrepeatQuestDlg.h"
#include "loadstring.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COpenrepeatQuestDlg dialog


COpenrepeatQuestDlg::COpenrepeatQuestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COpenrepeatQuestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COpenrepeatQuestDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bOpenCurrent = TRUE;
	m_strSubject = _T("");
	m_hBrush = NULL;
}
COpenrepeatQuestDlg::~COpenrepeatQuestDlg()
{
	if(m_hBrush)
		DeleteObject(m_hBrush);

}

void COpenrepeatQuestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpenrepeatQuestDlg)
	DDX_Control(pDX, IDC_STATIC_TITLE, m_stMsg);
	DDX_Control(pDX, IDOK, m_btOK);
	DDX_Control(pDX, IDCANCEL, m_btCancel);
	DDX_Control(pDX, IDC_OPENCURRENTRADIO, m_rbOpenCurent);
	DDX_Control(pDX, IDC_OPENSERIALRADIO, m_rbOpenSerial);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpenrepeatQuestDlg, CDialog)
	//{{AFX_MSG_MAP(COpenrepeatQuestDlg)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_OPENCURRENTRADIO, OnOpencurrentradio)
	ON_BN_CLICKED(IDC_OPENSERIALRADIO, OnOpenserialradio)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpenrepeatQuestDlg message handlers

BOOL COpenrepeatQuestDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	xUpdateText();
//	((CButton *)GetDlgItem(IDC_OPENCURRENTRADIO))->SetCheck(1);
	xUpdateControl();
	m_rbOpenCurent.SetCheck(TRUE);
	m_rbOpenSerial.SetCheck(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COpenrepeatQuestDlg::xUpdateText()
{
	CString str = LoadStringFromFile(_T("RepeatQuestDlg"),_T("DLG_STATIC_QUESTTEXT"));
	CString strTitle;
	strTitle.Format(str,m_strSubject);
	GetDlgItem(IDC_STATIC_TITLE)->SetWindowText(strTitle);

	str = LoadStringFromFile(_T("RepeatQuestDlg"),_T("DLG_RADIO_OPENCURENNT"));
	GetDlgItem(IDC_OPENCURRENTRADIO)->SetWindowText(str);
	str = LoadStringFromFile(_T("RepeatQuestDlg"),_T("DLG_RADIO_OPENSERIAL"));
	GetDlgItem(IDC_OPENSERIALRADIO)->SetWindowText(str);

	str = LoadStringFromFile(_T("public"),_T("IDS_OK"));
	GetDlgItem(IDOK)->SetWindowText(str);
	str = LoadStringFromFile(_T("public"),_T("IDS_CANCEL"));
	GetDlgItem(IDCANCEL)->SetWindowText(str);

	str = LoadStringFromFile(_T("RepeatQuestDlg"),_T("DLG_TITLE"));
	SetWindowText(str);

}



void COpenrepeatQuestDlg::OnOK() 
{
	// TODO: Add extra validation here
	int nCheck = ((CButtonEx *)GetDlgItem(IDC_OPENCURRENTRADIO))->GetCheck();
	if(nCheck == 1)
		m_bOpenCurrent = TRUE;
	else 
		m_bOpenCurrent = FALSE;
	CDialog::OnOK();
}

void COpenrepeatQuestDlg::SetSubject(CString str)
{
	m_strSubject = str;
	int i=0;
	int nLength = m_strSubject.GetLength();
	if(nLength >10)
	{
		nLength = 10;
		while (i < nLength)
		{
			if (::IsDBCSLeadByte((BYTE) m_strSubject[i]))
				i += 2;
			else
				i++;
		}
		if (i == (nLength + 1))	// double byte character is cut
			m_strSubject = m_strSubject.Left(nLength - 1);
		else
			m_strSubject = m_strSubject.Left(nLength);
		m_strSubject +="...";

	}
}

void COpenrepeatQuestDlg::xUpdateControl()
{
	TCHAR szProfile[MAX_PATH];
    CRect rect, rect2, rect3;

//	 TCHAR szFontName[MAX_PATH];
//	 int   nFontSize;
	//Load global font setting
//	if(!al_GetSettingString("Font","facename",theApp.m_szFont,szFontName))
//		_tcscpy(szFontName, "Arial");
//	if(!al_GetSettingInt("Font","fontsize",theApp.m_szFont,nFontSize))
//		nFontSize=12;


	wsprintf(szProfile, _T("%s%s"), theApp.m_szSkin, _T("Calendar\\OpenrepeatQuestDlg.ini"));
	//Resize Window by rect setting
	GetWindowRect(rect);
	GetClientRect(rect2);
	al_GetSettingRect( _T("panel"), _T("rect"), szProfile, rect3 );
	rect2.right = rect.Width() + rect3.Width() - rect2.right;
	rect2.bottom = rect.Height() + rect3.Height() - rect2.bottom;
	MoveWindow( &rect2 );	
	CenterWindow();

	//load cancel button
	GetButtonFromSetting(&m_btCancel,_T("cancel"),_T("IDS_CANCEL"),0,szProfile);
	GetButtonFromSetting(&m_btOK,_T("ok"),_T("IDS_OK"),0,szProfile);
	if(al_GetSettingRect(_T("cancel"),_T("rect"),szProfile,rect))
		m_btCancel.MoveWindow(rect);
	if(al_GetSettingRect(_T("ok"),_T("rect"),szProfile,rect))
		m_btOK.MoveWindow(rect);
//	CRect rect;
/*	m_rbOpenCurent.GetWindowRect(&rect);
	ScreenToClient(&rect);
	TRACE("m_rbOpenCurent ,%d,%d,%d,%d\n",rect.left,rect.top,rect.Width(),rect.Height());

	m_rbOpenSerial.GetWindowRect(&rect);
	ScreenToClient(&rect);
	TRACE("m_rbOpenSerial ,%d,%d,%d,%d\n",rect.left,rect.top,rect.Width(),rect.Height());
*/
	int nFontSize;
	TCHAR szFontName[MAX_PATH];
	
	//Get control font size and name.
	//get font color
	TCHAR szFontColor[4][32] = {_T("font_color_normal") , _T("font_color_down")
		,_T("font_color_hover"),_T("font_color_gray")};
	COLORREF fcolor[4];
	for(int i = 0 ; i < 4 ; i ++)
	{
		al_GetSettingColor(_T("button_font"),szFontColor[i],szProfile,fcolor[i]);
	}
	FONTCOLOR color = {fcolor[0],fcolor[1],fcolor[2],fcolor[3]};


	GetProfileFont(NULL,_T("button_font"),nFontSize,szFontName);
	TCHAR buf[MAX_PATH];
	TCHAR szFileName[MAX_PATH];
	if(al_GetSettingString(_T("OpenCurrentRadio"), _T("image"), szProfile, buf))
	{
		m_rbOpenCurent.SetButtonStyle(BS_OWNERDRAW);
	
		wsprintf(szFileName, _T("%s%s"), theApp.m_szSkin, buf);
		m_rbOpenCurent.LoadBitmap(szFileName);
		m_rbOpenCurent.ShowText(TRUE);
		if(al_GetSettingRect(_T("OpenCurrentRadio"),_T("rect"),szProfile,rect))
			m_rbOpenCurent.MoveWindow(rect);

		m_rbOpenCurent.SetTextFont(szFontName, nFontSize, FT_NORMAL, ANTIALIASED_QUALITY );
		m_rbOpenCurent.SetTextColor(color.crNormal, &color.crDown, &color.crGray);
	}
	if(al_GetSettingString(_T("OpenSerialRadio"), _T("image"), szProfile, buf))
	{
		m_rbOpenSerial.SetButtonStyle(BS_OWNERDRAW);

		wsprintf(szFileName, _T("%s%s"), theApp.m_szSkin, buf);
		m_rbOpenSerial.LoadBitmap(szFileName);
		m_rbOpenSerial.ShowText(TRUE);
		if(al_GetSettingRect(_T("OpenSerialRadio"),_T("rect"),szProfile,rect))
			m_rbOpenSerial.MoveWindow(rect);
		m_rbOpenSerial.SetTextFont(szFontName, nFontSize, FT_NORMAL, ANTIALIASED_QUALITY );
		m_rbOpenSerial.SetTextColor(color.crNormal, &color.crDown, &color.crGray);
	}

	//load text static
	GetStaticFromSetting(&m_stMsg,_T("Msg_text"),NULL,szProfile);
	m_stMsg.SetBrush(m_hBrush);

	//load panel color
	COLORREF bgcolor;
	if( !m_hBrush && al_GetSettingColor( _T("panel"), _T("brush"), szProfile, bgcolor ) )
		m_hBrush = CreateSolidBrush(bgcolor);	

}

void COpenrepeatQuestDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rect;
	GetClientRect(rect);
	::FillRect(dc,rect,m_hBrush);
	
	// Do not call CDialog::OnPaint() for painting messages
}

void COpenrepeatQuestDlg::OnOpencurrentradio() 
{
	// TODO: Add your control notification handler code here
	m_rbOpenCurent.SetCheck(TRUE);
	m_rbOpenSerial.SetCheck(FALSE);
	
}

void COpenrepeatQuestDlg::OnOpenserialradio() 
{
	// TODO: Add your control notification handler code here
	m_rbOpenCurent.SetCheck(FALSE);
	m_rbOpenSerial.SetCheck(TRUE);
	
}
