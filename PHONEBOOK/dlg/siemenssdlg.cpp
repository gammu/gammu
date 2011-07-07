// siemenssdlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "siemenssdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSiemensSDlg dialog


CSiemensSDlg::CSiemensSDlg(CWnd* pParent /*=NULL*/)
	: CBaseDlg(CSiemensSDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSiemensSDlg)
	//}}AFX_DATA_INIT
	m_hFont = NULL;
}
CSiemensSDlg::~CSiemensSDlg()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}

void CSiemensSDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSiemensSDlg)
	DDX_Control(pDX, IDC_ED_WEBPAGE, m_edURL);
	DDX_Control(pDX, IDC_SC_WEBPAGE, m_scURL);
	DDX_Control(pDX, IDC_SC_SLASTNAME, m_scLastName);
	DDX_Control(pDX, IDC_SC_SFIRSTNAME, m_scFirstName);
	DDX_Control(pDX, IDC_SC_FAX2, m_scFax2);
	DDX_Control(pDX, IDC_SC_EMAIL4, m_scEmail2);
	DDX_Control(pDX, IDC_ED_SLASTNAME, m_edLastName);
	DDX_Control(pDX, IDC_ED_SFIRSTNAME, m_edFirstName);
	DDX_Control(pDX, IDC_ED_MAIL4, m_edMail2);
	DDX_Control(pDX, IDC_ED_FAX2, m_edFax2);
	DDX_Control(pDX, IDOK, m_rbnOk);
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDC_SC_WV_USER_ID, m_scWV);
	DDX_Control(pDX, IDC_SC_STREET, m_scStreet);
	DDX_Control(pDX, IDC_SC_POST_CODE, m_scPostCode);
	DDX_Control(pDX, IDC_SC_POSITION, m_scPosition);
	DDX_Control(pDX, IDC_SC_PH_NUM, m_scGeneral);
	DDX_Control(pDX, IDC_SC_OFFICE, m_scOffice);
	DDX_Control(pDX, IDC_SC_NICKNAME, m_scNickName);
	DDX_Control(pDX, IDC_SC_NAME, m_scName);
	DDX_Control(pDX, IDC_SC_MOBILE, m_scMobile);
	DDX_Control(pDX, IDC_SC_ICQ, m_scICQ);
	DDX_Control(pDX, IDC_SC_GROUP, m_scGroup);
	DDX_Control(pDX, IDC_SC_FAX, m_scFax);
	DDX_Control(pDX, IDC_SC_EMAIL, m_scMail);
	DDX_Control(pDX, IDC_SC_COUNTRY, m_scCountry);
	DDX_Control(pDX, IDC_SC_COMPANY, m_scCompany);
	DDX_Control(pDX, IDC_SC_COM_SERVICES, m_scComServices);
	DDX_Control(pDX, IDC_SC_CITY, m_scCity);
	DDX_Control(pDX, IDC_SC_BIRTHDAY, m_scBirthday);
	DDX_Control(pDX, IDC_SC_AIM, m_scAIM);
	DDX_Control(pDX, IDC_SC_ADDRESS, m_scAddress);
	DDX_Control(pDX, IDC_ED_WV, m_edWV);
	DDX_Control(pDX, IDC_ED_STREET, m_edStreet);
	DDX_Control(pDX, IDC_ED_POSTCODE, m_edPostCode);
	DDX_Control(pDX, IDC_ED_PH_OFFICE, m_edOffice);
	DDX_Control(pDX, IDC_ED_PH_NUM, m_edGeneral);
	DDX_Control(pDX, IDC_ED_PH_MOBILE, m_edMobile);
	DDX_Control(pDX, IDC_ED_NICKNAME, m_edNickName);
	DDX_Control(pDX, IDC_ED_NAME, m_edName);
	DDX_Control(pDX, IDC_ED_MAIL, m_edMail);
	DDX_Control(pDX, IDC_ED_ICQ, m_edICQ);
	DDX_Control(pDX, IDC_ED_FAX, m_edFax);
	DDX_Control(pDX, IDC_ED_COUNTRY, m_edCountry);
	DDX_Control(pDX, IDC_ED_COMPANY, m_edCompany);
	DDX_Control(pDX, IDC_ED_CITY, m_edCity);
	DDX_Control(pDX, IDC_ED_AIM, m_edAIM);
	DDX_Control(pDX, IDC_DP_BIRTHDAY, m_dpBirthday);
	DDX_Control(pDX, IDC_CH_BIRTHDAY, m_chBirthday);
	DDX_Control(pDX, IDC_CB_GROUP, m_cbGroup);
	DDX_Control(pDX, IDC_RD_SIMCARD, m_rdSIM);
	DDX_Control(pDX, IDC_RD_MEMORY, m_rdME);
	DDX_Control(pDX, IDC_SC_LINE, m_scLine);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSiemensSDlg, CBaseDlg)
	//{{AFX_MSG_MAP(CSiemensSDlg)
	ON_BN_CLICKED(IDC_RD_MEMORY, OnRdMobile)
	ON_BN_CLICKED(IDC_RD_SIMCARD, OnRdSim)
	ON_BN_CLICKED(IDC_CH_BIRTHDAY, OnChBirthday)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSiemensSDlg message handlers

void CSiemensSDlg::OnOK() 
{
	TCHAR szTxt[MAX_PATH];
	TCHAR szOfficeTxt[MAX_PATH];
	TCHAR szMobileTxt[MAX_PATH];
	TCHAR szFaxTxt[MAX_PATH];
	TCHAR szFax2Txt[MAX_PATH];
	szTxt[0]='\0';
	szOfficeTxt[0]='\0';
	szMobileTxt[0]='\0';
	szFaxTxt[0]='\0';
	szFax2Txt[0]='\0';
	if(m_edGeneral.GetStyle() & WS_VISIBLE)
		m_edGeneral.GetWindowText(szTxt,MAX_PATH);
	if(m_edOffice.GetStyle() & WS_VISIBLE)
		m_edOffice.GetWindowText(szOfficeTxt,MAX_PATH);
	if(m_edMobile.GetStyle() & WS_VISIBLE)
		m_edMobile.GetWindowText(szMobileTxt,MAX_PATH);
	if(m_edFax.GetStyle() & WS_VISIBLE)
		m_edFax.GetWindowText(szFaxTxt,MAX_PATH);
	if(m_edFax2.GetStyle() & WS_VISIBLE)
		m_edFax2.GetWindowText(szFax2Txt,MAX_PATH);
	int iLength = _tcslen(szTxt) + _tcslen(szOfficeTxt) + _tcslen(szMobileTxt) + _tcslen(szFaxTxt)  + _tcslen(szFax2Txt) ;
	if(iLength == 0){
		if(al_GetSettingString(_T("public"),_T("IDS_ERR_NONUMBER"),theApp.m_szRes,szTxt))
			AfxMessageBox(szTxt);
		return;
	}
//peggy add temp +
	DEQPHONEDATA deqData;
	CPhoneData data;
	TCHAR szStreet[MAX_PATH];
	TCHAR szCity[MAX_PATH];
	TCHAR szZIP[MAX_PATH];
	TCHAR szCountry[MAX_PATH];
	TCHAR szName[MAX_PATH];
	//name or first name
	if(m_edFirstName.GetStyle() & WS_VISIBLE)
	{
		if(m_edFirstName.GetWindowText(szName,MAX_PATH) > 0)
		{
			data.SetType(PBK_Text_FirstName);
			
			data.SetText(szName);
			deqData.push_back(data);
		}
	}
	//last name
	if(m_edLastName.GetStyle() & WS_VISIBLE)
	{
		if(m_edLastName.GetWindowText(szName,MAX_PATH) > 0 )
		{
			data.SetType(PBK_Text_LastName);
			data.SetText(szName);
			deqData.push_back(data);
		}
	}
	//name
	if(m_edName.GetStyle() & WS_VISIBLE)
	{
		if(m_edName.GetWindowText(szTxt,MAX_PATH) > 0){
			data.SetType(PBK_Text_Name);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	//General 
	if(m_edGeneral.GetWindowText(szTxt,MAX_PATH) > 0){
		if(m_iIsME)
			data.SetType(PBK_Number_Home);
		else
			data.SetType(PBK_Number_General);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//Office 
	if(m_edOffice.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Number_Work);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	
	//mobile 
	if(m_edMobile.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Number_Mobile);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//fax
	if(m_edFax.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Number_Fax);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edFax2.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Number_Fax_Home);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//url
	if(m_edURL.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_URL);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//E-mail
	if(m_edMail.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_Email);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edMail2.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_Email_Home);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//company
	if(m_edCompany.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_Company);
		data.SetText(szTxt);
		deqData.push_back(data);
	}

	//birthday
	if( m_chBirthday.GetCheck() )	{
		COleDateTime tm;
		m_dpBirthday.GetTime(tm);
		data.SetType(PBK_Date);
		data.SetDate(tm);
		deqData.push_back(data);
	}
	//Asion add for MBDRV test
	//把PBK_Text_Postal原封不動show在UI上，也把整個street以PBK_Text_Postal傳回手機

	m_edStreet.GetWindowText(szStreet,MAX_PATH);
	m_edCity.GetWindowText(szCity,MAX_PATH);
	m_edPostCode.GetWindowText(szZIP,MAX_PATH);
	m_edCountry.GetWindowText(szCountry,MAX_PATH);
	if( szStreet[0]!=0 || szCity[0]!=0 || szZIP[0]!=0 || szCountry[0]!=0 )	{
		data.SetType(PBK_Text_Postal);
		MargeAddress( 0, szTxt, szStreet, szCity, _T("") , szZIP, szCountry );
		data.SetText(szTxt);
		deqData.push_back(data);
	}

	/*
	TCHAR szTmp[5][MAX_PATH];
	for(int i=0 ;i<5 ;i++)
	{
		szTmp[i][0]='\0';
	}
	m_edStreet.GetWindowText(szTmp[0],MAX_PATH);
	m_edCity.GetWindowText(szTmp[1],MAX_PATH);
	m_edCountry.GetWindowText(szTmp[4],MAX_PATH);
	if(_tcslen(szTmp[0]) > 0 || _tcslen(szTmp[1]) > 0 || _tcslen(szTmp[2]) > 0 || _tcslen(szTmp[3]) > 0
		 || _tcslen(szTmp[4]) > 0){
		data.SetType(PBK_Text_Postal);
		memset(szTxt,0,sizeof(TCHAR)*MAX_PATH);
		for(int i = 0 ; i < 5 ; i ++ ){
			_tcscat(szTxt,";");
			if(_tcslen(szTmp[i]) > 0)
				_tcscat(szTxt,szTmp[i]);
		}
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	*/


/*	//street
	if(m_edStreet.GetWindowText(szTxt,MAX_PATH) > 0 ){
		data.SetType(PBK_Text_StreetAddress);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//state
	if(m_edState.GetWindowText(szTxt,MAX_PATH) > 0 ){
		data.SetType(PBK_Text_State);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//zip
	if(m_edZip.GetWindowText(szTxt,MAX_PATH) > 0 ){
		data.SetType(PBK_Text_Zip);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//city
	if(m_edCity.GetWindowText(szTxt,MAX_PATH) > 0 ){
		data.SetType(PBK_Text_City);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//country
	if(m_edCountry.GetWindowText(szTxt,MAX_PATH) > 0 ){
		data.SetType(PBK_Text_Country);
		data.SetText(szTxt);
		deqData.push_back(data);
	}*/
	//detain data
	for(DEQPHONEDATA::iterator iter = m_deqDetainData.begin() ; iter != m_deqDetainData.end() ; iter ++){
		deqData.push_back(*iter);
	}
	
	m_pData->SetPhoneData(deqData);

	if(m_rdSIM.GetCheck())
		m_pData->SetStorageType(SIM_NAME);
	else
		m_pData->SetStorageType(MEMORY_NAME);
	//peggy add temp -
	CBaseDlg::OnOK();
}

BOOL CSiemensSDlg::OnInitDialog() 
{
	CBaseDlg::OnInitDialog();
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

	//get profile path
//	string sSkin(theApp.m_szSkin);
//	string sProfile = sSkin + string("Organize\\SiemensSDlg.ini");
	Tstring sProfile(theApp.m_szSkin);
	
	TCHAR szProfile[MAX_PATH];
	wsprintf(szProfile,_T("%sOrganize\\Siemens %s.ini"),theApp.m_szSkin,theApp.m_szPhone);
	if(al_IsFileExist(szProfile))
	{
		TCHAR szReference[MAX_PATH];
		if(al_GetSettingString(_T("Dlg"),_T("ReferenceTO"),szProfile,szReference))
		{
			wsprintf(szProfile,_T("%sOrganize\\%s"), theApp.m_szSkin,szReference);
		}
		sProfile = Tstring(szProfile);
	}
	else
	{
		sProfile += Tstring(_T("Organize\\SiemensSDlg.ini"));
	}

	al_GetSettingString(_T("public"),_T("IDS_LASTNAME"),theApp.m_szRes,m_szLastName);
	al_GetSettingString(_T("public"),_T("IDS_NAME2"),theApp.m_szRes,m_szName);

	//Load static
	CStaticEx	*pStaticEx[25] = {&m_scName,&m_scGeneral,&m_scOffice,&m_scMobile,&m_scFax
		,&m_scMail,&m_scComServices,&m_scNickName,&m_scWV,&m_scICQ,&m_scAIM,&m_scGroup
		,&m_scCompany,&m_scAddress,&m_scStreet,&m_scPostCode,&m_scCity,&m_scCountry,
		&m_scBirthday,&m_scPosition,&m_scFirstName,&m_scLastName,&m_scEmail2,&m_scFax2,&m_scURL};
	TCHAR szScSec[25][32] = {_T("name_sc"),_T("general_sc"),_T("office_sc"),_T("mobile_sc"),_T("fax_sc")
		,_T("mail_sc"),_T("com_services_sc"),_T("nickname_sc"),_T("wv_sc"),_T("icq_sc"),_T("aim_sc"),_T("group_sc")
		,_T("company_sc"),_T("address_sc"),_T("street_sc"),_T("postcode_sc"),_T("city_sc"),_T("country_sc")
		,_T("birthday_sc"),_T("position_sc"),_T("firstname_sc"),_T("lastname_sc"),_T("mail2_sc"),_T("fax2_sc"),_T("url_sc")};
	TCHAR szScID[25][32] = {_T("IDS_NAME2"),_T("IDS_S_GENERAL"),_T("IDS_A_COMPANY"),_T("IDS_MOBILENO"),_T("IDS_A_FAX")
		,_T("IDS_A_EMAIL"),_T("IDS_S_COM_SERVICES"),_T("IDS_S_NICKNAME"),_T("IDS_S_WV"),_T("IDS_S_ICQ"),_T("IDS_S_AIM"),_T("IDS_GROUP_NAME")
		,_T("IDS_COMPANY"),_T("IDS_A_ADDRESS"),_T("IDS_S_STREET"),_T("IDS_S_POSTCODE"),_T("IDS_CITY"),_T("IDS_COUNTRY")
		,_T("IDS_BIRTHDAY"),_T("IDS_A_LOCATION"),_T("IDS_FIRSTNAME"),_T("IDS_LASTNAME"),_T("IDS_EMAIL2"),_T("IDS_A_FAX2"),_T("IDS_HOMEPAGE")};
	for(int i = 0 ; i < 25 ; i ++)
		GetStaticFromSetting2(pStaticEx[i],szScSec[i],szScID[i],const_cast<TCHAR*>(sProfile.c_str()));

	//load button 
	CRescaleButton	*pButton[2] = {&m_rbnOk,&m_rbnCancel};
	TCHAR szBnSec[2][32] = {_T("ok"),_T("cancel")};
	TCHAR szBnID[2][32] = {_T("IDS_N_OK"),_T("IDS_CANCEL")};
	for(i = 0 ; i < 2 ; i ++)
		GetButtonFromSetting(pButton[i],szBnSec[i],szBnID[i],0,const_cast<TCHAR*>(sProfile.c_str()));

	//load checkbox
	CCheckEx2	*pCheck[1] = {&m_chBirthday};
	TCHAR szChSec[1][32] = {_T("birthday_ch")};
	for(i = 0 ; i < 1 ; i ++){
		GetCheckFromSetting(pCheck[i],szChSec[i],0,const_cast<TCHAR*>(sProfile.c_str()));
		pCheck[i]->SetCheck(0);
	}

	//load radio
	CRadioEx	*pRadio[2] = {&m_rdSIM,&m_rdME};
	TCHAR szRdSec[2][32] = {_T("sim_rd"),_T("memory_rd")};
	TCHAR szRdID[2][32] = {_T("IDS_SIMCARD"),_T("IDS_MOBILE")};
	for(i = 0 ; i < 2 ; i ++){
		GetRadioFromSetting(pRadio[i],szRdSec[i],szRdID[i],const_cast<TCHAR*>(sProfile.c_str()));
		pRadio[i]->SetCheck(0);
	}
	//set other control position and size
	CWnd *pWnd[23] = {&m_edName,&m_edGeneral,&m_edOffice,&m_edMobile,&m_edFax,&m_edMail,&m_edNickName,
		&m_edWV,&m_edICQ,&m_edAIM,&m_cbGroup,&m_edCompany,&m_edStreet,&m_edPostCode,&m_edCity,
		&m_edCountry,&m_dpBirthday,&m_scLine,&m_edFirstName,&m_edLastName,&m_edMail2,&m_edFax2,&m_edURL};
	TCHAR szWndSec[23][32] = {_T("name_ed"),_T("general_ed"),_T("office_ed"),_T("mobile_ed"),_T("fax_ed"),_T("mail_ed"),_T("nickname_ed"),
		_T("wv_ed"),_T("icq_ed"),_T("aim_ed"),_T("group_cb"),_T("company_ed"),_T("street_ed"),_T("postcode_ed"),_T("city_ed"),_T("country_ed"),_T("birthday_dp"),_T("line"),
	_T("firstname_ed"),_T("lastname_ed"),_T("mail2_ed"),_T("fax2_ed"),_T("url_ed")};
	for(i = 0 ; i < 23 ; i ++){
		CRect rect;
		if(al_GetSettingRect(szWndSec[i],_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect))
		{
			pWnd[i]->MoveWindow(rect);
			if(pFont) pWnd[i]->SetFont(pFont);
		}
		else pWnd[i]->ShowWindow(SW_HIDE);
	}

	m_edMobile.SetNumFlag(true);
	m_edGeneral.SetNumFlag(true);
	m_edOffice.SetNumFlag(true);
	m_edFax.SetNumFlag(true);
	m_edFax2.SetNumFlag(true);

	//Get background color
	al_GetSettingColor(_T("panel"),_T("color"),const_cast<TCHAR*>(sProfile.c_str()),m_crBg);

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
	m_rbnOk.GetWindowRect(&rcBtn2);
	ScreenToClient(&rcBtn2);
	if(rcBtn1.Width() > rcBtn2.Width())
		rcBtn2.right = rcBtn2.left + rcBtn1.Width();
	else
		rcBtn1.right = rcBtn1.left + rcBtn2.Width();

	m_dpBirthday.GetWindowRect(&rc);
	ScreenToClient(&rc);
	
	rcBtn1.OffsetRect(rc.right - rcBtn1.right  ,0);
	m_rbnCancel.MoveWindow(rcBtn1);

	rcBtn2.OffsetRect(rcBtn1.left - 20 - rcBtn2.right ,0);
	m_rbnOk.MoveWindow(rcBtn2);

	xSetRadioBtn();
	//set title 
	TCHAR szText[MAX_PATH];
	BOOL bRet = FALSE;
	if(m_iMode == 0){	//new
		bRet = al_GetSettingString(_T("public"),_T("IDS_NEWCONTACT"),theApp.m_szRes,szText);
	}
	else if(m_iMode == 1){	//edit
		bRet = al_GetSettingString(_T("public"),_T("IDS_EDITCONTACT"),theApp.m_szRes,szText);
		m_rdME.EnableWindow(FALSE);
		m_rdSIM.EnableWindow(FALSE);
		//initialize control data
	}
	else if(m_iMode == 2){	//view file
		//hide save button 
		m_rbnOk.ShowWindow(SW_HIDE);

		al_GetSettingString(_T("public"),_T("IDS_OK"),theApp.m_szRes,szText);
		m_rbnCancel.SetWindowText(szText);

		//set title
		bRet = al_GetSettingString(_T("public"),_T("IDS_VIEWCONTACT"),theApp.m_szRes,szText);
		//initialize control data
		xEnableControl(ENABLE_VIEW);
	}
	if(bRet)
		SetWindowText(szText);
	
	xSetComboItem();
	xSetDataToDlgCtrl();
		//Set combobox string

	if(m_iMode == ENABLE_VIEW)
	{
		if(m_scLastName.GetStyle()& WS_VISIBLE)
		{
			if(m_iIsME)
				m_scLastName.SetWindowText(m_szLastName);
			else
				m_scLastName.SetWindowText(m_szName);
		}

	}

	if(m_edName.GetStyle()& WS_VISIBLE)
		m_edName.SetFocus();
	else if(m_edLastName.GetStyle()& WS_VISIBLE)
		m_edLastName.SetFocus();

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSiemensSDlg::OnRdMobile() 
{
	m_rdME.SetCheck(TRUE);
	m_rdSIM.SetCheck(FALSE);
	xEnableControl(ENABLE_ME);
}

void CSiemensSDlg::OnRdSim() 
{
	m_rdME.SetCheck(FALSE);
	m_rdSIM.SetCheck(TRUE);
	xEnableControl(ENABLE_SIM);
}

CSiemensSDlg::xEnableControl(int iState)
{
	CWnd *pWnd[23] = {&m_edName,&m_edGeneral,&m_edLastName,&m_edFirstName,&m_edOffice,&m_edMobile,&m_edFax,
		&m_edMail,&m_edNickName,&m_edWV,&m_edICQ,&m_edAIM,&m_edCompany,&m_edStreet,
		&m_edPostCode,&m_edCity,&m_edCountry,&m_chBirthday,&m_dpBirthday,&m_cbGroup,
	&m_edMail2,&m_edFax2,&m_edURL};
	if(iState == ENABLE_SIM){
		for(int i = 3 ; i < 23 ; i ++)
			pWnd[i]->EnableWindow(FALSE);

		m_scLastName.SetWindowText(m_szName);
	}
	else if(iState == ENABLE_ME){
		for(int i = 0 ; i < 23 ; i ++){
			pWnd[i]->EnableWindow(TRUE);
		}
		m_scLastName.SetWindowText(m_szLastName);
	}
	else if(iState == ENABLE_VIEW){
		for(int i = 0 ; i < 23 ; i ++){
			pWnd[i]->EnableWindow(FALSE);
		}
		m_rdSIM.EnableWindow(FALSE);
		m_rdME.EnableWindow(FALSE);
	}
	OnChBirthday();
}

CSiemensSDlg::xSetDataToDlgCtrl()
{
	CString strName;
	strName.Empty();
	if(m_pData){
		//get data deq
		DEQPHONEDATA *pdeqData; 
		pdeqData = m_pData->GetPhoneData();
		for(DEQPHONEDATA::iterator iter = pdeqData->begin() ; iter != pdeqData->end() ; iter ++){
			if((*iter).GetType() == PBK_Text_Name){		//first name
				m_edName.SetWindowText((*iter).GetText());
				strName.Format(_T("%s"),(char *)((*iter).GetText()));
			}
			else if((*iter).GetType() == PBK_Text_FirstName){		//FirstName
				m_edFirstName.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_LastName){		//LastName
				m_edLastName.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Home && m_iIsME){	//number on ME
				m_edGeneral.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_General && !m_iIsME){	//number on SIM
				m_edGeneral.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Work){	//work no
				m_edOffice.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Mobile){	//mobile no
				m_edMobile.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Fax){	//fax
				m_edFax.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Fax_Home){	//fax
				m_edFax2.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_URL){	
				m_edURL.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Email){	//email
				m_edMail.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Email_Home){	//email2
				m_edMail2.SetWindowText((*iter).GetText());
			}
			//lost NickName, WV used ID, ICQ ,AIM
			else if((*iter).GetType() == PBK_Caller_Group){		//group
				m_cbGroup.SetCurSel((*iter).GetNumber());
			}
			else if((*iter).GetType() == PBK_Text_Company){		//company
				m_edCompany.SetWindowText((*iter).GetText());
			}
			//Asion modify for MBDRV test
			else if((*iter).GetType() == PBK_Text_Postal){	//street
				TCHAR szStreet[MAX_PATH], szCity[MAX_PATH], szZIP[MAX_PATH], szCountry[MAX_PATH];
				SpriteAddress( 
					0, 
					(*iter).GetText(), 
					szStreet, 
					szCity,
					NULL,
					szZIP, 
					szCountry );
				m_edStreet.SetWindowText(szStreet);
				m_edCity.SetWindowText(szCity);
				m_edPostCode.SetWindowText(szZIP);
				m_edCountry.SetWindowText(szCountry);
			}
/*
			else if((*iter).GetType() == PBK_Text_StreetAddress){	//street
				m_edStreet.SetWindowText((*iter).GetText());
			}
*/
			else if((*iter).GetType() == PBK_Text_Zip){	//post code
				m_edPostCode.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_City){	//city
				m_edCity.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Country){	//country
				m_edCountry.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Date){		//birthday
				COleDateTime tm;
				(*iter).GetTime(tm);
				m_dpBirthday.SetTime(tm);
				m_chBirthday.SetCheck(1);
				if(m_iMode!=2)
					m_dpBirthday.EnableWindow(true);

			}		
			else{	//others
				m_deqDetainData.push_back((*iter));
			}
		}
		if(m_edLastName.GetStyle()& WS_VISIBLE)
		{
			CString strFirstName,strLastName;
			strFirstName.Empty();
			strLastName.Empty();
			m_edFirstName.GetWindowText(strFirstName);
			m_edLastName.GetWindowText(strLastName);
			if(strFirstName.IsEmpty() && strLastName.IsEmpty() && strName.IsEmpty()== false)
			{
				m_edLastName.SetWindowText(strName);
			}
		}

	}
	
}

void CSiemensSDlg::OnChBirthday() 
{
	if(m_chBirthday.GetCheck() && m_chBirthday.IsWindowEnabled())	
		m_dpBirthday.EnableWindow(TRUE);
	else
		m_dpBirthday.EnableWindow(FALSE);
}	

CSiemensSDlg::xSetComboItem()
{
	//set group combo item
	int iCount = 0;
	TCHAR szTxt[MAX_PATH];
	//get group count
	::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_CARDPANEL_GROUPCOUNT,reinterpret_cast<WPARAM>(&iCount),0);
	if(iCount > 0){
		for(int i = 0 ; i < iCount ; i ++){
			//get group item
			::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_CARDPANEL_GETGROUP,i,reinterpret_cast<WPARAM>(szTxt));
			//add to combo
			m_cbGroup.AddString(szTxt);
		}
		m_cbGroup.SetCurSel(0);
	}

}
