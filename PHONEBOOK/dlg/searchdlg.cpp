// SearchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\PhoneBook.h"
#include "SearchDlg.h"
#include "..\Panel\PrevView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSearchDlg dialog


CSearchDlg::CSearchDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SEARCH, pParent)
{
	m_crBg = 0;
	m_iStart = -1;
	m_pParent = pParent;
	m_hFont = NULL;
	//{{AFX_DATA_INIT(CSearchDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}
CSearchDlg::~CSearchDlg()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}

void CSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSearchDlg)
	DDX_Control(pDX, IDC_BN_FINDNEXT, m_rbnFindNext);
	DDX_Control(pDX, IDCANCEL, m_rbnOk);
	DDX_Control(pDX, IDC_COMBO_NAME, m_cbSearch);
	DDX_Control(pDX, IDC_SC_ENTERNAME, m_scTitle);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSearchDlg, CDialog)
	//{{AFX_MSG_MAP(CSearchDlg)
	ON_BN_CLICKED(IDC_BN_FINDNEXT, OnBnFindnext)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSearchDlg message handlers

BOOL CSearchDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
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

	Tstring sSkin(theApp.m_szSkin);
	Tstring sProfile = sSkin + Tstring(_T("Organize\\search.ini"));
	
	//Load ok button setting
	GetButtonFromSetting(&m_rbnOk,_T("ok"),_T("IDS_CANCEL"),0,const_cast<TCHAR*>(sProfile.c_str()));
	//Load find next button setting
	GetButtonFromSetting(&m_rbnFindNext,_T("findnext"),_T("IDS_FINDNEXT"),0,const_cast<TCHAR*>(sProfile.c_str()));

	//laod title static setting

	GetStaticFromSetting(&m_scTitle,_T("title"),_T("IDS_ENTERNAME"),const_cast<TCHAR*>(sProfile.c_str()));
	//load combo box
	CRect rect;
	if(al_GetSettingRect(_T("search"),_T("rect"),const_cast<TCHAR*>(sProfile.c_str()),rect))
		m_cbSearch.MoveWindow(rect);
	//set combo limit text size
	m_cbSearch.LimitText(MAX_PATH);
	if(pFont) m_cbSearch.SetFont(pFont);
	//Set combo box item
	int i = 0 ;
	for(deque<Tstring>::iterator iter = m_pdeqSearchStr->begin() ; iter != m_pdeqSearchStr->end() ; iter ++, i ++){
		m_cbSearch.InsertString(i,(*iter).c_str());
	}

	//load dialog setting
	TCHAR szTxt[MAX_PATH];
	if(al_GetSettingRect(_T("panel"),_T("rect"),const_cast<TCHAR*>(sProfile.c_str()),rect)){
		int iY = ::GetSystemMetrics(SM_CYCAPTION);
		int iBorder = ::GetSystemMetrics(SM_CXBORDER);
		rect.bottom += iY + iBorder;
		rect.right += 2 * iBorder;
		MoveWindow(rect);
		CenterWindow();
	}
	//Set dialog title
	if(al_GetSettingString(_T("public"),_T("IDS_SEARCH"),theApp.m_szRes,szTxt))
		SetWindowText(szTxt);
	
	al_GetSettingColor(_T("panel"),_T("brush"),const_cast<TCHAR*>(sProfile.c_str()),m_crBg);

	//Set both two button size
	CRect rcbtn1,rcbtn2,rc;
	m_rbnOk.GetWindowRect(rcbtn1);
	ScreenToClient(rcbtn1);
	m_rbnFindNext.GetWindowRect(rcbtn2);
	ScreenToClient(rcbtn2);
	if(rcbtn1.Width() > rcbtn2.Width())
		rcbtn2.right = rcbtn2.left + rcbtn1.Width();
	else
		rcbtn1.right = rcbtn1.left + rcbtn2.Width();
	m_cbSearch.GetWindowRect(rc);
	ScreenToClient(&rc);
	rcbtn1.OffsetRect(rc.right - rcbtn1.right,0);
	m_rbnOk.MoveWindow(rcbtn1);
	rcbtn2.OffsetRect(rcbtn1.left - 10 - rcbtn2.right , 0);
	m_rbnFindNext.MoveWindow(rcbtn2);

	m_cbSearch.SetFocus();
	return FALSE;
//	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSearchDlg::OnBnFindnext() 
{
	//Add the string into combox
	int iIndex = m_cbSearch.GetCurSel();
	TCHAR szText[MAX_PATH];
	m_cbSearch.GetWindowText(szText,MAX_PATH);

	//reset the start index 
	::SendMessage(m_pParent->GetSafeHwnd(),WM_SEARCH_GETSTART,(WPARAM)&m_iStart,0);	
	
/*	if(m_sOldTextl != string(szText)){
		m_iStart = 0;
		m_sOldTextl = string(szText);
	}
	else if(m_iStart != -1)
		m_iStart ++;
*/

	//insert the string in the deque and combobox
	m_pdeqSearchStr->push_front(Tstring(szText));
	m_cbSearch.InsertString(0,szText);
	
	//Set combo box current select
	if(iIndex != -1){
		m_cbSearch.DeleteString(iIndex + 1);
		deque<Tstring> deqTemp;
		int i = 0;
		for(deque<Tstring>::iterator iter = m_pdeqSearchStr->begin() ; 
			iter != m_pdeqSearchStr->end() ; iter ++,i++){
				if(i != iIndex + 1){
					deqTemp.push_back((*iter));
				}
			}
		m_pdeqSearchStr->clear();
		*m_pdeqSearchStr = deqTemp;
		deqTemp.clear();
	}
	m_cbSearch.SetCurSel(0);

	::SendMessage(m_pParent->GetSafeHwnd(),WM_SEARCH_STR,(WPARAM)szText,(LPARAM)&m_iStart);	
}

void CSearchDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rect;
	GetClientRect(rect);
	CBrush brush(m_crBg);
	dc.FillRect(rect,&brush);
	// Do not call CDialog::OnPaint() for painting messages
}

void CSearchDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

void CSearchDlg::OnOK()
{
	OnBnFindnext();
}

void CSearchDlg::OnCancel()
{
	CDialog::OnCancel();
	::PostMessage(m_pParent->GetSafeHwnd(),WM_SEARCH_END,0,0);
}
