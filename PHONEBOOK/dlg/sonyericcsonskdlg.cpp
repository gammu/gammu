// sonyericcsonskdlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "sonyericcsonskdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSonyEricssonSKDlg dialog


CSonyEricssonSKDlg::CSonyEricssonSKDlg(CWnd* pParent /*=NULL*/)
	: CBaseDlg(CSonyEricssonSKDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSonyEricssonSKDlg)
	//}}AFX_DATA_INIT
	m_hFont = NULL;
}
CSonyEricssonSKDlg::~CSonyEricssonSKDlg()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}

void CSonyEricssonSKDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSonyEricssonSKDlg)
	DDX_Control(pDX, IDC_SC_ZIPW, m_scZipW);
	DDX_Control(pDX, IDC_SC_STREETW, m_scStreetW);
	DDX_Control(pDX, IDC_SC_STATEW, m_scStateW);
	DDX_Control(pDX, IDC_SC_COUNTRYW, m_scCountryW);
	DDX_Control(pDX, IDC_SC_CITYW, m_scCityW);
	DDX_Control(pDX, IDC_ED_ZIPW, m_edZipW);
	DDX_Control(pDX, IDC_ED_STREETW, m_edStreetW);
	DDX_Control(pDX, IDC_ED_STATEW, m_edStateW);
	DDX_Control(pDX, IDC_ED_COUNTRYW, m_edCountryW);
	DDX_Control(pDX, IDC_ED_CITYW, m_edCityW);
	DDX_Control(pDX, IDC_DP_BIRTHDAY, m_dpBirthday);
	DDX_Control(pDX, IDC_SC_MAIL3, m_scEmail3);
	DDX_Control(pDX, IDC_SC_MAIL2, m_scEmail2);
	DDX_Control(pDX, IDC_SC_BIRTHDAY, m_scBirthday);
	DDX_Control(pDX, IDC_ED_MAIL3, m_edEmail3);
	DDX_Control(pDX, IDC_ED_MAIL2, m_edEmail2);
	DDX_Control(pDX, IDC_CH_BIRTHDAY, m_chBirthday);
	DDX_Control(pDX, IDOK, m_rbnOk);
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDC_SC_ZIP, m_scZip);
	DDX_Control(pDX, IDC_SC_WORK, m_scWork);
	DDX_Control(pDX, IDC_SC_WEB, m_scWeb);
	DDX_Control(pDX, IDC_SC_TITLE, m_scTitle);
	DDX_Control(pDX, IDC_SC_STREET, m_scStreet);
	DDX_Control(pDX, IDC_SC_STATE, m_scState);
	DDX_Control(pDX, IDC_SC_POSITION, m_scPosition);
	DDX_Control(pDX, IDC_SC_OTHER, m_scOther);
	DDX_Control(pDX, IDC_SC_NAME, m_scName);
	DDX_Control(pDX, IDC_SC_MOBILE, m_scMobile);
	DDX_Control(pDX, IDC_SC_MAIL, m_scMail);
	DDX_Control(pDX, IDC_SC_INFO, m_scInfo);
	DDX_Control(pDX, IDC_SC_HOME, m_scHome);
	DDX_Control(pDX, IDC_SC_FAX, m_scFax);
	DDX_Control(pDX, IDC_SC_COUNTRY, m_scCountry);
	DDX_Control(pDX, IDC_SC_COMPANY, m_scCompany);
	DDX_Control(pDX, IDC_SC_CITY, m_scCity);
	DDX_Control(pDX, IDC_SC_LINE, m_scLine);
	DDX_Control(pDX, IDC_ED_ZIP, m_edZip);
	DDX_Control(pDX, IDC_ED_WORK, m_edWork);
	DDX_Control(pDX, IDC_ED_WEB, m_edWeb);
	DDX_Control(pDX, IDC_ED_TITLE, m_edTitle);
	DDX_Control(pDX, IDC_ED_STREET, m_edStreet);
	DDX_Control(pDX, IDC_ED_STATE, m_edState);
	DDX_Control(pDX, IDC_ED_OTHER, m_edOther);
	DDX_Control(pDX, IDC_ED_NAME, m_edName);
	DDX_Control(pDX, IDC_ED_MOBILE, m_edMobile);
	DDX_Control(pDX, IDC_ED_MAIL, m_edMail);
	DDX_Control(pDX, IDC_ED_INFO, m_edInfo);
	DDX_Control(pDX, IDC_ED_HOME, m_edHome);
	DDX_Control(pDX, IDC_ED_FAX, m_edFax);
	DDX_Control(pDX, IDC_ED_COUNTRY, m_edCountry);
	DDX_Control(pDX, IDC_ED_COMPANY, m_edCompany);
	DDX_Control(pDX, IDC_ED_CITY, m_edCity);
	DDX_Control(pDX, IDC_RD_SIM, m_rdSIM);
	DDX_Control(pDX, IDC_RD_ME, m_rdME);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSonyEricssonSKDlg, CBaseDlg)
	//{{AFX_MSG_MAP(CSonyEricssonSKDlg)
	ON_BN_CLICKED(IDC_RD_SIM, OnRdSim)
	ON_BN_CLICKED(IDC_RD_ME, OnRdMobile)
	ON_BN_CLICKED(IDC_CH_BIRTHDAY, OnChBirthday)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSonyEricssonSKDlg message handlers

void CSonyEricssonSKDlg::OnRdMobile() 
{
	m_rdME.SetCheck(TRUE);
	m_rdSIM.SetCheck(FALSE);
	xEnableControl(ENABLE_ME);
}

void CSonyEricssonSKDlg::OnRdSim() 
{
	m_rdME.SetCheck(FALSE);
	m_rdSIM.SetCheck(TRUE);
	xEnableControl(ENABLE_SIM);
}

void CSonyEricssonSKDlg::OnOK() 
{
	TCHAR szMobile[MAX_PATH],szFax[MAX_PATH],szHome[MAX_PATH],szOther[MAX_PATH],szWork[MAX_PATH],szTxt[MAX_PATH];
	m_edMobile.GetWindowText(szMobile,MAX_PATH);
	m_edFax.GetWindowText(szFax,MAX_PATH);
	m_edHome.GetWindowText(szHome,MAX_PATH);
	m_edOther.GetWindowText(szOther,MAX_PATH);
	m_edWork.GetWindowText(szWork,MAX_PATH);
	
	if(_tcslen(szMobile) == 0 && _tcslen(szFax) == 0 && _tcslen(szHome) == 0 && _tcslen(szOther) == 0 && _tcslen(szWork) == 0 ) {
		if(al_GetSettingString(_T("public"),_T("IDS_ERR_NONUMBER"),theApp.m_szRes,szMobile))
			AfxMessageBox(szMobile);
		return;
	}

	DEQPHONEDATA deqData;
	CPhoneData data;

	//name
	if(m_edName.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_Name);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//mobile 
	if(m_edMobile.GetWindowText(szTxt,MAX_PATH) > 0){
		if(m_iIsME)
			data.SetType(PBK_Number_Mobile);
		else 
			data.SetType(PBK_Number_General);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//home
	if(m_edHome.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Number_Home);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//work
	if(m_edWork.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Number_Work);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//other
	if(m_edOther.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Number_Other);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//fax
	if(m_edFax.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Number_Fax);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//E-mail
	if(m_edMail.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_Email);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//E-mail2
	if(m_edEmail2.GetStyle() & WS_VISIBLE)
	{
		if(m_edEmail2.GetWindowText(szTxt,MAX_PATH) > 0){
			data.SetType(PBK_Text_Email);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	//E-mail3
	if(m_edEmail3.GetStyle() & WS_VISIBLE)
	{
		if(m_edEmail3.GetWindowText(szTxt,MAX_PATH) > 0){
			data.SetType(PBK_Text_Email);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	//web
	if(m_edWeb.GetWindowText(szTxt,MAX_PATH) > 0 ){
		data.SetType(PBK_Text_URL);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//job title
	if(m_edTitle.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_JobTitle);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//company
	if(m_edCompany.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_Company);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
/*	TCHAR szTmp[5][MAX_PATH];
	m_edStreet.GetWindowText(szTmp[0],MAX_PATH);
	m_edCity.GetWindowText(szTmp[1],MAX_PATH);
	m_edState.GetWindowText(szTmp[2],MAX_PATH);
	m_edZip.GetWindowText(szTmp[3],MAX_PATH);
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
	}*/
	TCHAR szStreet[MAX_PATH],szCity[MAX_PATH],szState[MAX_PATH],szZIP[MAX_PATH],szCountry[MAX_PATH];
	m_edStreet.GetWindowText(szStreet,MAX_PATH);
	m_edCity.GetWindowText(szCity,MAX_PATH);
	m_edState.GetWindowText(szState,MAX_PATH);
	m_edZip.GetWindowText(szZIP,MAX_PATH);
	m_edCountry.GetWindowText(szCountry,MAX_PATH);
	if( szStreet[0]!=0 || szCity[0]!=0 || szZIP[0]!=0 || szCountry[0]!=0 || szState[0]!=0)	{
		data.SetType(PBK_Text_Postal);
		MargeAddress( 0, szTxt, szStreet, szCity, szState , szZIP, szCountry );
		data.SetText(szTxt);
		deqData.push_back(data);
	}

	if(m_edStreetW.GetStyle() & WS_VISIBLE)
	{
		m_edStreetW.GetWindowText(szStreet,MAX_PATH);
		m_edCityW.GetWindowText(szCity,MAX_PATH);
		m_edStateW.GetWindowText(szState,MAX_PATH);
		m_edZipW.GetWindowText(szZIP,MAX_PATH);
		m_edCountryW.GetWindowText(szCountry,MAX_PATH);
		if( szStreet[0]!=0 || szCity[0]!=0 || szZIP[0]!=0 || szCountry[0]!=0 || szState[0]!=0)	{
			data.SetType(PBK_Text_Postal_Work);
			MargeAddress( 0, szTxt, szStreet, szCity, szState , szZIP, szCountry );
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
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
	//info
	if(m_edInfo.GetWindowText(szTxt,MAX_PATH) > 0 ){
		data.SetType(PBK_Text_Note);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//birthday
	if(m_scBirthday.GetStyle() & WS_VISIBLE)
	{
		if( m_chBirthday.GetCheck() )	{
			COleDateTime tm;
			m_dpBirthday.GetTime(tm);
			data.SetType(PBK_Date);
			data.SetDate(tm);
			deqData.push_back(data);
		}
	}
	//detain data
	for(DEQPHONEDATA::iterator iter = m_deqDetainData.begin() ; iter != m_deqDetainData.end() ; iter ++){
		deqData.push_back(*iter);
	}
	
	m_pData->SetPhoneData(deqData);

	if(m_rdSIM.GetCheck())
		m_pData->SetStorageType(SIM_NAME);
	else
		m_pData->SetStorageType(MEMORY_NAME);

	CBaseDlg::OnOK();
}


BOOL CSonyEricssonSKDlg::OnInitDialog() 
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

	Tstring sProfile(theApp.m_szSkin);
	
	TCHAR szProfile[MAX_PATH];
	wsprintf(szProfile,_T("%sOrganize\\SE %s.ini"),theApp.m_szSkin,theApp.m_szPhone);
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
		sProfile += Tstring(_T("Organize\\SESKDlg.ini"));

	//Load static
	CStaticEx	*pStaticEx[25] = {&m_scName,&m_scMobile,&m_scHome,&m_scWork,&m_scFax,
		&m_scOther,&m_scMail,&m_scWeb,&m_scPosition,&m_scTitle,&m_scCompany,&m_scStreet,
		&m_scCity,&m_scZip,&m_scState,&m_scCountry,&m_scInfo,&m_scEmail2,&m_scEmail3,&m_scBirthday,
	&m_scStreetW,&m_scCityW,&m_scZipW,&m_scStateW,&m_scCountryW};
	TCHAR szScSec[25][32] = {_T("name_sc"),_T("mobile_sc"),_T("home_sc"),_T("work_sc"),_T("fax_sc"),
		_T("other_sc"),_T("mail_sc"),_T("web_sc"),_T("position_sc"),_T("title_sc"),_T("company_sc"),_T("street_sc"),
		_T("city_sc"),_T("zip_sc"),_T("state_sc"),_T("country_sc"),_T("info_sc"),_T("mail2_sc"),_T("mail3_sc"),_T("birthday_sc"),
	_T("streetW_sc"),_T("cityW_sc"),_T("zipW_sc"),_T("stateW_sc"),_T("countryW_sc")};
	TCHAR szScID[25][32] = {_T("IDS_NAME2"),_T("IDS_MOBILENO"),_T("IDS_HOMENO"),_T("IDS_SE_WORK"),_T("IDS_SE_FAX"),
		_T("IDS_SE_OTHER"),_T("IDS_EMAIL"),_T("IDS_HOMEPAGE"),_T("IDS_A_STORAGE"),_T("IDS_HANDLE"),_T("IDS_COMPANY"),_T("IDS_S_STREET"),
		_T("IDS_CITY"),_T("IDS_ZIP"),_T("IDS_SE_STATE"),_T("IDS_COUNTRY"),_T("IDS_SE_INFO"),_T("IDS_EMAIL2"),_T("IDS_EMAIL_3"),_T("IDS_BIRTHDAY"),
		_T("IDS_S_STREETW"),_T("IDS_CITYW"),_T("IDS_ZIPW"),_T("IDS_SE_STATEW"),_T("IDS_COUNTRYW")};
	for(int i = 0 ; i < 25 ; i ++){
		GetStaticFromSetting2(pStaticEx[i],szScSec[i],szScID[i],const_cast<TCHAR*>(sProfile.c_str()));
	}
		
	//set other control position and size
	CWnd *pWnd[25] = {&m_edName,&m_edMobile,&m_edHome,&m_edWork,&m_edFax,
		&m_edOther,&m_edMail,&m_edWeb,&m_edTitle,&m_edCompany,&m_edStreet,
		&m_edCity,&m_edZip,&m_edState,&m_edCountry,&m_edInfo,&m_scLine,&m_edEmail2,&m_edEmail3,&m_dpBirthday,
	&m_edStreetW,&m_edCityW,&m_edZipW,&m_edStateW,&m_edCountryW};
	TCHAR szWndSec[25][32] = {_T("name_ed"),_T("mobile_ed"),_T("home_ed"),_T("work_ed"),_T("fax_ed"),
		_T("other_ed"),_T("mail_ed"),_T("web_ed"),_T("title_ed"),_T("company_ed"),_T("street_ed"),
		_T("city_ed"),_T("zip_ed"),_T("state_ed"),_T("country_ed"),_T("info_ed"),_T("line"),_T("mail2_ed"),_T("mail3_ed"),_T("birthday_dp"),
	_T("streetW_ed"),_T("cityW_ed"),_T("zipW_ed"),_T("stateW_ed"),_T("countryW_ed")};
	for(i = 0 ; i < 25 ; i ++){
		CRect rect;
		if(al_GetSettingRect(szWndSec[i],_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect))
		{
			pWnd[i]->MoveWindow(rect);
			pWnd[i]->ShowWindow(SW_SHOW);
		}
		else pWnd[i]->ShowWindow(SW_HIDE);
		if(pFont)pWnd[i]->SetFont(pFont);
	}

	//load checkbox
	CCheckEx2	*pCheck[1] = {&m_chBirthday};
	TCHAR szChSec[1][32] = {_T("birthday_ch")};
	for(i = 0 ; i < 1 ; i ++){
		GetCheckFromSetting2(pCheck[i],szChSec[i],0,const_cast<TCHAR*>(sProfile.c_str()));
		pCheck[i]->SetCheck(0);
	}

	//load button 
	CRescaleButton	*pButton[2] = {&m_rbnOk,&m_rbnCancel};
	TCHAR szBnSec[2][32] = {_T("ok"),_T("cancel")};
	TCHAR szBnID[2][32] = {_T("IDS_N_OK"),_T("IDS_CANCEL")};
	for(i = 0 ; i < 2 ; i ++)
		GetButtonFromSetting(pButton[i],szBnSec[i],szBnID[i],0,const_cast<TCHAR*>(sProfile.c_str()));

	//load radio
	CRadioEx	*pRadio[2] = {&m_rdSIM,&m_rdME};
	TCHAR szRdSec[2][32] = {_T("sim_rd"),_T("memory_rd")};
	TCHAR szRdID[2][32] = {_T("IDS_SIMCARD"),_T("IDS_MOBILE")};
	for(i = 0 ; i < 2 ; i ++){
		GetRadioFromSetting(pRadio[i],szRdSec[i],szRdID[i],const_cast<TCHAR*>(sProfile.c_str()));
		pRadio[i]->SetCheck(0);
	}
		
	m_edMobile.SetNumFlag(true);
	m_edHome.SetNumFlag(true);
	m_edWork.SetNumFlag(true);
	m_edFax.SetNumFlag(true);
	m_edOther.SetNumFlag(true);

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

	m_edInfo.GetWindowRect(&rc);
	ScreenToClient(&rc);
	
	rcBtn1.OffsetRect(rc.right - rcBtn1.right  ,0);
	m_rbnCancel.MoveWindow(rcBtn1);

	rcBtn2.OffsetRect(rcBtn1.left - 20 - rcBtn2.right ,0);
	m_rbnOk.MoveWindow(rcBtn2);

	xSetDataToDlgCtrl();

	//Set combobox string
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
	if(((CPhoneBookApp*)afxGetApp())->m_bNotSupportME && m_iMode != 2)
	{
		m_rdME.EnableWindow(FALSE);
		m_rdSIM.EnableWindow(FALSE);
		OnRdSim();
	}
//	xSetDataToDlgCtrl();
	m_edName.SetFocus();
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CSonyEricssonSKDlg::xEnableControl(int iState)
{
	CWnd *pWnd[25] = {&m_edName,&m_edMobile,&m_edHome,&m_edWork,&m_edFax,&m_edOther,
		&m_edMail,&m_edWeb,&m_edTitle,&m_edCompany,&m_edStreet,&m_edCity,&m_edState,
		&m_edZip,&m_edCountry,&m_edInfo,&m_dpBirthday,&m_chBirthday,&m_edEmail2,&m_edEmail3,
	&m_edStreetW,&m_edCityW,&m_edStateW,&m_edZipW,&m_edCountryW};
	if(iState == ENABLE_SIM){
		for(int i = 2 ; i < 25 ; i ++)
			pWnd[i]->EnableWindow(FALSE);
	}
	else if(iState == ENABLE_ME){
		for(int i = 0 ; i < 25 ; i ++){
			pWnd[i]->EnableWindow(TRUE);
		}
	}
	else if(iState == ENABLE_VIEW){
		for(int i = 0 ; i < 25 ; i ++){
			pWnd[i]->EnableWindow(FALSE);
		}
		m_rdSIM.EnableWindow(FALSE);
		m_rdME.EnableWindow(FALSE);
	}
	OnChBirthday();

}

CSonyEricssonSKDlg::xSetDataToDlgCtrl()
{
	CWnd *pEmailWnd[3] = {&m_edMail,&m_edEmail2,&m_edEmail3};
	int nEmailIndex = 0;
	if(m_pData){
		//get data deq
		DEQPHONEDATA *pdeqData; 
		pdeqData = m_pData->GetPhoneData();
		for(DEQPHONEDATA::iterator iter = pdeqData->begin() ; iter != pdeqData->end() ; iter ++){
			if((*iter).GetType() == PBK_Text_Name){		//name
				m_edName.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Mobile && m_iIsME){	//mobile no on ME
				m_edMobile.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_General && !m_iIsME){ //General no on SM
				m_edMobile.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Home){	//home no
				m_edHome.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Work){	//work no
				m_edWork.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Other){	//other no
				m_edOther.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Fax){	//fax
				m_edFax.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Email){	//email
		//		m_edMail.SetWindowText((*iter).GetText());
				if(nEmailIndex >=0 && nEmailIndex <3 &&
					pEmailWnd[nEmailIndex]->GetStyle() & WS_VISIBLE)
				{
					pEmailWnd[nEmailIndex]->SetWindowText((*iter).GetText());
					nEmailIndex ++;
				}
				else 
					m_edMail.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_URL){	//web
				m_edWeb.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_JobTitle){	//title
				m_edTitle.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Company){		//company
				m_edCompany.SetWindowText((*iter).GetText());
			}
/*			else if((*iter).GetType() == PBK_Text_Postal){
				string sPostal = (*iter).GetText();
				CEdit *pEdit[5] = {&m_edStreet,&m_edCity,&m_edState,&m_edZip,&m_edCountry};
				string::size_type idx = 0,old_idx = 1;
				for(int i = 0 ; i < 5 ; i ++){
					idx = sPostal.find(';',old_idx);
					string sTxt = sPostal.substr(old_idx,idx-old_idx);
					pEdit[i]->SetWindowText(sTxt.c_str());
					old_idx = idx + 1;
				}
			}*/
			else if((*iter).GetType() == PBK_Text_Postal){	//street
				TCHAR szStreet[MAX_PATH], szCity[MAX_PATH], szState[MAX_PATH], szZIP[MAX_PATH], szCountry[MAX_PATH];
				SpriteAddress( 
					0, 
					(*iter).GetText(), 
					szStreet, 
					szCity,
					szState,
					szZIP, 
					szCountry );
				m_edStreet.SetWindowText(szStreet);
				m_edCity.SetWindowText(szCity);
				m_edState.SetWindowText(szState);
				m_edZip.SetWindowText(szZIP);
				m_edCountry.SetWindowText(szCountry);
			}
			else if((*iter).GetType() == PBK_Text_Postal_Work){	//street
				TCHAR szStreet[MAX_PATH], szCity[MAX_PATH], szState[MAX_PATH], szZIP[MAX_PATH], szCountry[MAX_PATH];
				SpriteAddress( 
					0, 
					(*iter).GetText(), 
					szStreet, 
					szCity,
					szState,
					szZIP, 
					szCountry );
				m_edStreetW.SetWindowText(szStreet);
				m_edCityW.SetWindowText(szCity);
				m_edStateW.SetWindowText(szState);
				m_edZipW.SetWindowText(szZIP);
				m_edCountryW.SetWindowText(szCountry);
			}

/*			else if((*iter).GetType() == PBK_Text_StreetAddress){	//street
				m_edStreet.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Zip){	//zip
				m_edZip.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_State){	//state
				m_edState.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_City){	//city
				m_edCity.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Country){	//country
				m_edCountry.SetWindowText((*iter).GetText());
			}*/
			else if((*iter).GetType() == PBK_Text_Note){	//info
				m_edInfo.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Date){		//birthday
				COleDateTime tm;
				(*iter).GetTime(tm);
				m_dpBirthday.SetTime(tm);
				m_chBirthday.SetCheck(1);
			}		
			else{	//others
				m_deqDetainData.push_back((*iter));
			}
		}
	}
}

void CSonyEricssonSKDlg::OnChBirthday() 
{
	// TODO: Add your control notification handler code here
	if(m_chBirthday.GetCheck() && m_iMode != 2)	
		m_dpBirthday.EnableWindow(TRUE);
	else
		m_dpBirthday.EnableWindow(FALSE);
	
}
