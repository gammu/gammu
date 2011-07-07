// NS6AddressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "NS6AddressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNS6AddressDlg dialog


CNS6AddressDlg::CNS6AddressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNS6AddressDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNS6AddressDlg)
	//}}AFX_DATA_INIT
	m_hFont = NULL;
}
CNS6AddressDlg::~CNS6AddressDlg()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}

void CNS6AddressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNS6AddressDlg)
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDOK, m_rbnOK);
	DDX_Control(pDX, IDC_SC_STREET, m_scStreet);
	DDX_Control(pDX, IDC_SC_STATE, m_scState);
	DDX_Control(pDX, IDC_SC_POST_CODE, m_scPostCode);
	DDX_Control(pDX, IDC_SC_POBOX, m_scPOBox);
	DDX_Control(pDX, IDC_SC_EXTENSION, m_scExtension);
	DDX_Control(pDX, IDC_SC_COUNTRY, m_scCountry);
	DDX_Control(pDX, IDC_SC_CITY, m_scCity);
	DDX_Control(pDX, IDC_ED_STREET, m_edStreet);
	DDX_Control(pDX, IDC_ED_STATE, m_edState);
	DDX_Control(pDX, IDC_ED_COUNTRY, m_edCountry);
	DDX_Control(pDX, IDC_ED_CITY, m_edCity);
	DDX_Control(pDX, IDC_ED_POBOX, m_edPOBox);
	DDX_Control(pDX, IDC_ED_POSTCODE, m_edPostCode);
	DDX_Control(pDX, IDC_ED_EXTERNSION, m_edExternsion);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNS6AddressDlg, CDialog)
	//{{AFX_MSG_MAP(CNS6AddressDlg)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNS6AddressDlg message handlers

BOOL CNS6AddressDlg::OnInitDialog() 
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
	sProfile += Tstring(_T("organize\\S6AdressDlg.ini"));
	//load static
	CStaticEx *pStaticEx[7] = {&m_scPOBox,&m_scExtension,&m_scStreet,&m_scPostCode,&m_scCity,&m_scState,&m_scCountry};
	TCHAR szScSec[7][32] = {_T("pobox_sc"),_T("extension_sc"),_T("street_sc"),_T("postcode_sc"),_T("city_sc"),_T("state_sc"),_T("country_sc")};
	TCHAR szScID[30][32] = {_T("IDS_POBOX"),_T("IDS_EXTENSION"),_T("IDS_S_STREET"),_T("IDS_ZIP"),_T("IDS_CITY"),_T("IDS_SE_STATE"),_T("IDS_COUNTRY")};
	for(int i = 0 ; i < 7 ; i ++)
		GetStaticFromSetting(pStaticEx[i],szScSec[i],szScID[i],const_cast<TCHAR*>(sProfile.c_str()));

	//load button 
	CRescaleButton	*pButton[2] = {&m_rbnOK,&m_rbnCancel};
	TCHAR szBnSec[2][32] = {_T("ok"),_T("cancel")};;
	TCHAR szBnID[2][32] = {_T("IDS_OK"),_T("IDS_CANCEL")};
	for(i = 0 ; i < 2 ; i ++)
		GetButtonFromSetting(pButton[i],szBnSec[i],szBnID[i],0,const_cast<TCHAR*>(sProfile.c_str()));

	//set other control position and size
	CWnd *pWnd[7] = {&m_edPOBox,&m_edExternsion,&m_edStreet,&m_edPostCode,&m_edCity,&m_edState,&m_edCountry};
	TCHAR szWndSec[7][32] = {_T("pobox_ed"),_T("extension_ed"),_T("street_ed"),_T("postcode_ed"),_T("city_ed"),_T("state_ed"),_T("country_ed")};
	for(i = 0 ; i < 7 ; i ++){
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

	m_edPOBox.GetWindowRect(&rc);
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

void CNS6AddressDlg::OnOK() 
{
	// TODO: Add extra validation here
	CString strStreet,strState,strCountry,strCity,strPOBox,strPostCode,strExternsion;
	m_edStreet.GetWindowText(strStreet);
	m_edState.GetWindowText(strState);
	m_edCountry.GetWindowText(strCountry);
	m_edCity.GetWindowText(strCity);
	m_edPOBox.GetWindowText(strPOBox);
	m_edPostCode.GetWindowText(strPostCode);
	m_edExternsion.GetWindowText(strExternsion);

	strPOBox.Replace(_T(";"),_T("\\;"));
	strExternsion.Replace(_T(";"),_T("\\;"));
	strStreet.Replace(_T(";"),_T("\\;"));
	strCity.Replace(_T(";"),_T("\\;"));
	strState.Replace(_T(";"),_T("\\;"));
	strPostCode.Replace(_T(";"),_T("\\;"));
	strCountry.Replace(_T(";"),_T("\\;"));

	m_strAddress.Format(_T("%s;%s;%s;%s;%s;%s;%s"),strPOBox,strExternsion,strStreet,strCity,strState,strPostCode,
						strCountry);
	CDialog::OnOK();
}

void CNS6AddressDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rect;
	GetClientRect(rect);
	CBrush brush(m_crBg);
	dc.FillRect(rect,&brush);
}

void CNS6AddressDlg::SetAddress(CString strAddress)
{
	m_strAddress = strAddress;
}

CString CNS6AddressDlg::GetAddress()
{
	return m_strAddress;
}
void CNS6AddressDlg::AnaAddressString(LPCTSTR szString,CStringList *pstrlist )
{
	CString strall(szString);
	int  i, j;
	TCHAR szStr[128];
	strall +=';';
	pstrlist->RemoveAll();

	for( i = 0, j = 0; i < strall.GetLength(); i++ )
	{
	    if (strall[i] == ';')
	    {
			if(i>0)
			{
				if(strall[i-1]!= '\\')
				{
					szStr[j] = '\0';
				//	if(strlen(szStr)>0)
					pstrlist->AddTail(szStr);
					j = 0;
				}
				else szStr[j++] = strall[i];
			}
			else
			{
				szStr[j] = '\0';
			//	if(strlen(szStr)>0)
				pstrlist->AddTail(szStr);
				j = 0;
			}
	    }
	    else
			szStr[j++] = strall[i];
	}

}
void CNS6AddressDlg::FillData()
{
	CStringList strList;
	AnaAddressString(m_strAddress.GetBuffer(MAX_PATH),&strList);
	m_strAddress.ReleaseBuffer();
	int i = 0;
	CWnd *pWnd[7] = {&m_edPOBox,&m_edExternsion,&m_edStreet,&m_edCity,&m_edState,&m_edPostCode,&m_edCountry};
	POSITION pos = strList.GetHeadPosition();
	while(pos && i<7)
	{
		CString str = strList.GetNext(pos);
		str.Replace(_T("\\;"),_T(";"));
		pWnd[i]->SetWindowText(str);
		i++;
	}
}
