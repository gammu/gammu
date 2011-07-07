// NS6AddressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "NS4AddressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNS4AddressDlg dialog


CNS4AddressDlg::CNS4AddressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNS4AddressDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNS4AddressDlg)
	//}}AFX_DATA_INIT
	m_hFont = NULL;
	m_strExt.Empty();
	m_strStreet.Empty();
	m_strCity.Empty();
	m_strState.Empty();
	m_strZip.Empty();
	m_strCountry.Empty();
}
CNS4AddressDlg::~CNS4AddressDlg()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}

void CNS4AddressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNS4AddressDlg)
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDOK, m_rbnOK);
	DDX_Control(pDX, IDC_SC_STREET, m_scStreet);
	DDX_Control(pDX, IDC_SC_STATE, m_scState);
	DDX_Control(pDX, IDC_SC_POST_CODE, m_scPostCode);
	DDX_Control(pDX, IDC_SC_EXTENSION, m_scExtension);
	DDX_Control(pDX, IDC_SC_COUNTRY, m_scCountry);
	DDX_Control(pDX, IDC_SC_CITY, m_scCity);
	DDX_Control(pDX, IDC_ED_STREET, m_edStreet);
	DDX_Control(pDX, IDC_ED_STATE, m_edState);
	DDX_Control(pDX, IDC_ED_COUNTRY, m_edCountry);
	DDX_Control(pDX, IDC_ED_CITY, m_edCity);
	DDX_Control(pDX, IDC_ED_POSTCODE, m_edPostCode);
	DDX_Control(pDX, IDC_ED_EXTERNSION, m_edExternsion);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNS4AddressDlg, CDialog)
	//{{AFX_MSG_MAP(CNS4AddressDlg)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNS4AddressDlg message handlers

BOOL CNS4AddressDlg::OnInitDialog() 
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
	sProfile += Tstring(_T("organize\\S4AdressDlg.ini"));
	//load static
	CStaticEx *pStaticEx[6] = {&m_scExtension,&m_scStreet,&m_scPostCode,&m_scCity,&m_scState,&m_scCountry};
	TCHAR szScSec[6][32] = {_T("extension_sc"),_T("street_sc"),_T("postcode_sc"),_T("city_sc"),_T("state_sc"),_T("country_sc")};
	TCHAR szScID[6][32] = {_T("IDS_EXTENSION"),_T("IDS_S_STREET"),_T("IDS_ZIP"),_T("IDS_CITY"),_T("IDS_SE_STATE"),_T("IDS_COUNTRY")};
	if(theApp.m_iMobileCompany == CO_MOTO)	
	{
		wsprintf(szScID[0],_T("IDS_M_STREET1"));
		wsprintf(szScID[1],_T("IDS_M_STREET2"));
	}

	for(int i = 0 ; i < 6 ; i ++)
		GetStaticFromSetting(pStaticEx[i],szScSec[i],szScID[i],const_cast<TCHAR*>(sProfile.c_str()));
	//load button 
	CRescaleButton	*pButton[2] = {&m_rbnOK,&m_rbnCancel};
	TCHAR szBnSec[2][32] = {_T("ok"),_T("cancel")};
	TCHAR szBnID[2][32] = {_T("IDS_OK"),_T("IDS_CANCEL")};
	for(i = 0 ; i < 2 ; i ++)
		GetButtonFromSetting(pButton[i],szBnSec[i],szBnID[i],0,const_cast<TCHAR*>(sProfile.c_str()));

	//set other control position and size
	CWnd *pWnd[6] = {&m_edExternsion,&m_edStreet,&m_edPostCode,&m_edCity,&m_edState,&m_edCountry};
	TCHAR szWndSec[6][32] = {_T("extension_ed"),_T("street_ed"),_T("postcode_ed"),_T("city_ed"),_T("state_ed"),_T("country_ed")};
	for(i = 0 ; i < 6 ; i ++){
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

void CNS4AddressDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString strStreet,strState,strCountry,strCity,strPOBox,strPostCode,strExternsion;
	m_edStreet.GetWindowText(m_strStreet);
	m_edState.GetWindowText(m_strState);
	m_edCountry.GetWindowText(m_strCountry);
	m_edCity.GetWindowText(m_strCity);
	m_edPostCode.GetWindowText(m_strZip);
	m_edExternsion.GetWindowText(m_strExt);
	CDialog::OnOK();
}

void CNS4AddressDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rect;
	GetClientRect(rect);
	CBrush brush(m_crBg);
	dc.FillRect(rect,&brush);
}

void CNS4AddressDlg::SetAddress(CString strExt ,CString strStreet,CString strCity,
								CString strState ,CString strZip,CString strCountry)
{
	m_strExt = strExt;
	m_strStreet = strStreet;
	m_strCity = strCity;
	m_strState = strState;
	m_strZip = strZip ;
	m_strCountry = strCountry;
}


void CNS4AddressDlg::FillData()
{
	m_edExternsion.SetWindowText(m_strExt);
	m_edStreet.SetWindowText(m_strStreet);
	m_edCity.SetWindowText(m_strCity);
	m_edState.SetWindowText(m_strState);
	m_edPostCode.SetWindowText(m_strZip);
	m_edCountry.SetWindowText(m_strCountry);
	
}
