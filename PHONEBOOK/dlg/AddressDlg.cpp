// AddressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "AddressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddressDlg dialog


CAddressDlg::CAddressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddressDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddressDlg)
	//}}AFX_DATA_INIT
	m_hFont = NULL;
	m_strStreet.Empty();
	m_strCity.Empty();
	m_strState.Empty();
	m_strZip.Empty();
	m_strCountry.Empty();
}
CAddressDlg::~CAddressDlg()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}

void CAddressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddressDlg)
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDOK, m_rbnOK);
	DDX_Control(pDX, IDC_SC_STREET, m_scStreet);
	DDX_Control(pDX, IDC_SC_STATE, m_scState);
	DDX_Control(pDX, IDC_SC_POSTALCODE, m_scPostCode);
	DDX_Control(pDX, IDC_SC_COUNTRY, m_scCountry);
	DDX_Control(pDX, IDC_SC_CITY, m_scCity);
	DDX_Control(pDX, IDC_ED_STREET, m_edStreet);
	DDX_Control(pDX, IDC_ED_STATE, m_edState);
	DDX_Control(pDX, IDC_ED_COUNTRY, m_edCountry);
	DDX_Control(pDX, IDC_ED_CITY, m_edCity);
	DDX_Control(pDX, IDC_ED_POSTALCODE, m_edPostCode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddressDlg, CDialog)
	//{{AFX_MSG_MAP(CAddressDlg)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddressDlg message handlers

BOOL CAddressDlg::OnInitDialog() 
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
	
	//profiel path 
	Tstring sProfile(theApp.m_szSkin);
	sProfile += Tstring(_T("organize\\AdressDlg.ini"));
	//load static
	CStaticEx *pStaticEx[5] = {&m_scStreet,&m_scPostCode,&m_scCity,&m_scState,&m_scCountry};
	TCHAR szScSec[5][32] = {_T("street_sc"),_T("postcode_sc"),_T("city_sc"),_T("state_sc"),_T("country_sc")};
	TCHAR szScID[5][32] = {_T("IDS_S_STREET"),_T("IDS_ZIP"),_T("IDS_CITY"),_T("IDS_SE_STATE"),_T("IDS_COUNTRY")};

	for(int i = 0 ; i < 5 ; i ++)
		GetStaticFromSetting(pStaticEx[i],szScSec[i],szScID[i],const_cast<TCHAR*>(sProfile.c_str()));
	//load button 
	CRescaleButton	*pButton[2] = {&m_rbnOK,&m_rbnCancel};
	TCHAR szBnSec[2][32] = {_T("ok"),_T("cancel")};
	TCHAR szBnID[2][32] = {_T("IDS_OK"),_T("IDS_CANCEL")};
	for(i = 0 ; i < 2 ; i ++)
		GetButtonFromSetting(pButton[i],szBnSec[i],szBnID[i],0,const_cast<TCHAR*>(sProfile.c_str()));

	//set other control position and size
	CWnd *pWnd[5] = {&m_edStreet,&m_edPostCode,&m_edCity,&m_edState,&m_edCountry};
	TCHAR szWndSec[5][32] = {_T("street_ed"),_T("postcode_ed"),_T("city_ed"),_T("state_ed"),_T("country_ed")};
	for(i = 0 ; i < 5 ; i ++){
		CRect rect;
		al_GetSettingRect(szWndSec[i],_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect);
		pWnd[i]->MoveWindow(rect);
		if(pFont)pWnd[i]->SetFont( pFont);
	}
	
	//get panel color
	al_GetSettingColor(_T("panel"),_T("color"),const_cast<TCHAR *>(sProfile.c_str()),m_crBg);


	//Set window pos
	CRect rect;
	if(al_GetSettingRect(_T("panel"),_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect)){
		UINT uStyle = ::GetWindowLong(GetSafeHwnd(),GWL_STYLE);
		UINT uStyleEx = ::GetWindowLong(GetSafeHwnd(),GWL_EXSTYLE);
		if(AdjustWindowRectEx(rect,uStyle,FALSE,uStyleEx)){
			MoveWindow(rect);
			CenterWindow();
		}
	}
	//set button pos and size
	CRect rc,rcBtn1,rcBtn2;
	m_rbnCancel.GetWindowRect(&rcBtn1);
	ScreenToClient(&rcBtn1);
	m_rbnOK.GetWindowRect(&rcBtn2);
	ScreenToClient(&rcBtn2);
	if(rcBtn1.Width() > rcBtn2.Width())
		rcBtn2.right = rcBtn2.left + rcBtn1.Width();
	else
		rcBtn1.right = rcBtn1.left + rcBtn2.Width();

	m_edCity.GetWindowRect(&rc);
	ScreenToClient(&rc);
	
	rcBtn1.OffsetRect(rc.right - rcBtn1.right  ,0);
	m_rbnCancel.MoveWindow(rcBtn1);

	rcBtn2.OffsetRect(rcBtn1.left - 20 - rcBtn2.right ,0);
	m_rbnOK.MoveWindow(rcBtn2);

	TCHAR szText[MAX_PATH];
	BOOL bRet = al_GetSettingString(_T("public"),_T("IDS_ADDRESS"),theApp.m_szRes,szText);
	if(bRet)
		SetWindowText(szText);
	FillData();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAddressDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString strStreet,strState,strCountry,strCity,strPOBox,strPostCode,strExternsion;
	m_edStreet.GetWindowText(m_strStreet);
	m_edState.GetWindowText(m_strState);
	m_edCountry.GetWindowText(m_strCountry);
	m_edCity.GetWindowText(m_strCity);
	m_edPostCode.GetWindowText(m_strZip);
	CDialog::OnOK();
}

void CAddressDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rect;
	GetClientRect(rect);
	CBrush brush(m_crBg);
	dc.FillRect(rect,&brush);
}

void CAddressDlg::SetAddress(CString strStreet,CString strCity,
								CString strState ,CString strZip,CString strCountry)
{
	m_strStreet = strStreet;
	m_strCity = strCity;
	m_strState = strState;
	m_strZip = strZip ;
	m_strCountry = strCountry;
}


void CAddressDlg::FillData()
{
	m_edStreet.SetWindowText(m_strStreet);
	m_edCity.SetWindowText(m_strCity);
	m_edState.SetWindowText(m_strState);
	m_edPostCode.SetWindowText(m_strZip);
	m_edCountry.SetWindowText(m_strCountry);
	
}
