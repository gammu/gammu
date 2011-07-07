// motodlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "motodlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMotoDlg dialog


CMotoDlg::CMotoDlg(CWnd* pParent /*=NULL*/)
	: CBaseDlg(CMotoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMotoDlg)
	//}}AFX_DATA_INIT
	m_hFont = NULL;
}
CMotoDlg::~CMotoDlg()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}


void CMotoDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMotoDlg)
	DDX_Control(pDX, IDC_LINE, m_scLine);
	DDX_Control(pDX, IDC_DP_BIRTHDAY, m_dpBirthday);
	DDX_Control(pDX, IDC_SC_ZIP, m_scZip);
	DDX_Control(pDX, IDC_SC_STREET2, m_scStreet2);
	DDX_Control(pDX, IDC_SC_STREET1, m_scStreet1);
	DDX_Control(pDX, IDC_SC_STATE, m_scState);
	DDX_Control(pDX, IDC_SC_NICKNAME, m_scNickName);
	DDX_Control(pDX, IDC_SC_COUNTRY, m_scCountry);
	DDX_Control(pDX, IDC_SC_CITY, m_scCity);
	DDX_Control(pDX, IDC_SC_BIRTHDAY, m_scBirthday);
	DDX_Control(pDX, IDC_CH_BIRTHDAY, m_chBirthday);
	DDX_Control(pDX, IDC_ET_NICKNAME, m_edNickName);
	DDX_Control(pDX, IDC_ED_ZIP, m_edZip);
	DDX_Control(pDX, IDC_ED_STREET2, m_edStreet2);
	DDX_Control(pDX, IDC_ED_STREET1, m_edStreet1);
	DDX_Control(pDX, IDC_ED_STATE, m_edState);
	DDX_Control(pDX, IDC_ED_COUNTRY, m_edCountry);
	DDX_Control(pDX, IDC_ED_CITY, m_edCity);
	DDX_Control(pDX, IDOK, m_rbnOK);
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDC_SC_TYPE, m_scType);
	DDX_Control(pDX, IDC_SC_STORAGE, m_scStorage);
	DDX_Control(pDX, IDC_SC_NAME, m_scName);
	DDX_Control(pDX, IDC_SC_GROUP, m_scGroup);
	DDX_Control(pDX, IDC_SC_DATA, m_scData);
	DDX_Control(pDX, IDC_ED_NAME, m_edName);
	DDX_Control(pDX, IDC_ED_DATA, m_edData);
	DDX_Control(pDX, IDC_CB_TYPE, m_cbType);
	DDX_Control(pDX, IDC_CB_GROUP, m_cbGroup);
	DDX_Control(pDX, IDC_RD_SIM, m_rdSIM);
	DDX_Control(pDX, IDC_RD_MEMORY, m_rdME);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMotoDlg, CBaseDlg)
	//{{AFX_MSG_MAP(CMotoDlg)
	ON_BN_CLICKED(IDC_RD_SIM, OnRdSim)
	ON_BN_CLICKED(IDC_RD_MEMORY, OnRdMobile)
	ON_CBN_SELCHANGE(IDC_CB_TYPE, OnSelchangeCbType)
	ON_BN_CLICKED(IDC_CH_BIRTHDAY, OnChBirthday)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMotoDlg message handlers

BOOL CMotoDlg::OnInitDialog() 
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
	Tstring sSkin(theApp.m_szSkin);
//	string sProfile = sSkin + string("Organize\\MotoDlg.ini");
	Tstring sProfile = sSkin + Tstring(_T("Organize\\"))+Tstring(m_strSkinFile);

	//Load static
	CStaticEx	*pStaticEx[5] = {&m_scName,&m_scType,&m_scData,&m_scGroup,&m_scStorage};
	TCHAR szScSec[5][32] = {_T("name_sc"),_T("type_sc"),_T("data_sc"),_T("group_sc"),_T("storage_sc")};
	TCHAR szScID[5][32] = {_T("IDS_A_NAME"),_T("IDS_M_TYPE"),_T("IDS_M_DATA"),_T("IDS_A_GROUP"),_T("IDS_A_LOCATION")};
	for(int i = 0 ; i < 5 ; i ++)
		GetStaticFromSetting(pStaticEx[i],szScSec[i],szScID[i],const_cast<TCHAR*>(sProfile.c_str()));

	//load button 
	CRescaleButton	*pButton[2] = {&m_rbnOK,&m_rbnCancel};
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
	
	//new Control ******************************************* ++++
	CStaticEx	*pStaticEx2[8] = {&m_scNickName,&m_scBirthday,&m_scStreet1,&m_scStreet2,&m_scCity,&m_scState,&m_scZip,&m_scCountry};
	TCHAR szScSec2[8][32] = {_T("nickname_sc"),_T("birthday_sc"),_T("street1_sc"),_T("street2_sc"),_T("city_sc"),_T("state_sc"),_T("zip_sc"),_T("country_sc")};
	TCHAR szScID2[8][32] = {_T("IDS_S_NICKNAME"),_T("IDS_A_BIRTHDAY"),_T("IDS_M_STREET1"),_T("IDS_M_STREET2"),_T("IDS_CITY"),_T("IDS_SE_STATE"),_T("IDS_ZIP"),_T("IDS_COUNTRY")};

	if(_tcscmp(theApp.m_szPhone,_T("E1000")) == 0 )
	{
		wsprintf(szScID2[2],_T("%s"),_T("IDS_M_INFO"));// IDS_M_STREET1
		wsprintf(szScID2[3],_T("%s"),_T("IDS_A_ADDRESS"));// IDS_M_STREET2
	}

	for(i = 0 ; i < 8 ; i ++)
		GetStaticFromSetting2(pStaticEx2[i],szScSec2[i],szScID2[i],const_cast<TCHAR*>(sProfile.c_str()));


	CWnd *pEDWnd[8] = {&m_edNickName,&m_edZip,&m_edStreet1,&m_edStreet2,&m_edState,&m_edCountry,&m_edCity,&m_dpBirthday};
	TCHAR szEDWndSec[8][32] = {_T("nickname_ed"),_T("zip_ed"),_T("street1_ed"),_T("street2_ed"),_T("state_ed"),_T("country_ed"),_T("city_ed"),_T("birthday_dp")};
	for(i = 0 ; i < 8 ; i ++){
		CRect rect;
		if(	al_GetSettingRect(szEDWndSec[i],_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect))
			pEDWnd[i]->MoveWindow(rect);
		else
			pEDWnd[i]->ShowWindow(SW_HIDE);

		if(pFont) pEDWnd[i]->SetFont(pFont);
	}
	CCheckEx2	*pCheck[1] = {&m_chBirthday};
	TCHAR szChSec[1][32] = {_T("birthday_ch")};
	for(i = 0 ; i < 1 ; i ++){
		GetCheckFromSetting2(pCheck[i],szChSec[i],0,const_cast<TCHAR*>(sProfile.c_str()));
		pCheck[i]->SetCheck(0);
	}

	//new Control ******************************************* ----

	//set other control position and size
	CWnd *pWnd[4] = {&m_edName,&m_cbType,&m_edData,&m_cbGroup};
	TCHAR szWndSec[4][32] = {_T("name_ed"),_T("type_cb"),_T("data_ed"),_T("group_cb")};
	for(i = 0 ; i < 4 ; i ++){
		CRect rect;
		al_GetSettingRect(szWndSec[i],_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect);
		pWnd[i]->MoveWindow(rect);
		if(pFont) pWnd[i]->SetFont(pFont);
	}

	m_edData.SetNumFlag(true);

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
	m_rbnOK.GetWindowRect(&rcBtn2);
	ScreenToClient(&rcBtn2);

	CRect rcLine,rcClient;
	GetClientRect(&rcClient);
	m_scLine.GetWindowRect(&rcLine);
	ScreenToClient(&rcLine);
	rcLine.left = rcClient.left;
	rcLine.right = rcClient.right;
	int nW = rcLine.Height();
	rcLine.top = rcClient.bottom - rcBtn2.Height() -20;
	rcLine.bottom = rcLine.top+nW;
	m_scLine.MoveWindow(rcLine);


	if(rcBtn1.Width() > rcBtn2.Width())
		rcBtn2.right = rcBtn2.left + rcBtn1.Width();
	else
		rcBtn1.right = rcBtn1.left + rcBtn2.Width();

	if(m_edStreet1.GetStyle() & WS_VISIBLE)
	{
		m_edStreet1.GetWindowRect(&rc);
	}
	else
		m_cbGroup.GetWindowRect(&rc);
	ScreenToClient(&rc);
	
	rcBtn1.OffsetRect(rc.right - rcBtn1.right  ,0);
	m_rbnCancel.MoveWindow(rcBtn1);

	rcBtn2.OffsetRect(rcBtn1.left - 20 - rcBtn2.right ,0);
	m_rbnOK.MoveWindow(rcBtn2);

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
		m_rbnOK.ShowWindow(SW_HIDE);

		al_GetSettingString(_T("public"),_T("IDS_OK"),theApp.m_szRes,szText);
		m_rbnCancel.SetWindowText(szText);

		//set title
		bRet = al_GetSettingString(_T("public"),_T("IDS_VIEWCONTACT"),theApp.m_szRes,szText);
		xEnableControl(ENABLE_VIEW);
	}
	if(bRet)
		SetWindowText(szText);
	
	xSetComboItem();
	xSetDataToDlgCtrl();
	//Set combobox string

	m_edName.SetFocus();
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMotoDlg::OnRdSim() 
{
	m_rdME.SetCheck(FALSE);
	m_rdSIM.SetCheck(TRUE);
	m_cbType.SetCurSel(0);
	m_cbType.EnableWindow(FALSE);
	OnSelchangeCbType();
	xEnableControl(ENABLE_SIM);
}

void CMotoDlg::OnRdMobile() 
{
	m_rdME.SetCheck(TRUE);
	m_rdSIM.SetCheck(FALSE);
	m_cbType.EnableWindow(TRUE);
	xEnableControl(ENABLE_ME);
}

void CMotoDlg::OnSelchangeCbType() 
{
	TCHAR szTxt[MAX_PATH];
	TCHAR szDataText[MAX_PATH];
	szDataText[0] ='\0';
	bool bOldNumFlag = false;
	bOldNumFlag = m_edData.GetNumFlag();
	m_edData.GetWindowText(szTxt,MAX_PATH);
	

	if(m_cbType.GetCurSel() == 6){	//e-mail
		m_edData.SetNumFlag(false);
		al_GetSettingString(_T("public"),_T("IDS_M_ADDRESS"),theApp.m_szRes,szDataText);

		if(bOldNumFlag)
			m_sTempNum = szTxt;
		else
			m_sTempStr = szTxt;

		if(!m_sTempStr.empty()){
			m_edData.SetWindowText(m_sTempStr.c_str());
		}
		else
			m_edData.SetWindowText(_T(""));
	}
	else{
		m_edData.SetNumFlag(true);
		al_GetSettingString(_T("public"),_T("IDS_M_PHONENO"),theApp.m_szRes,szDataText);

	//	m_sTempStr = szTxt;
		if(bOldNumFlag)
			m_sTempNum = szTxt;
		else
			m_sTempStr = szTxt;
		if(!m_sTempNum.empty()){
			m_edData.SetWindowText(m_sTempNum.c_str());
		}
		else
			m_edData.SetWindowText(_T(""));
	}
	if(_tcslen(szDataText) >0 )
		m_scData.SetWindowText(szDataText);

}

void CMotoDlg::OnOK() 
{
	TCHAR szTxt[MAX_PATH];
	m_edData.GetWindowText(szTxt,MAX_PATH);
	int iLength = _tcslen(szTxt);
	if(iLength == 0){
		if(al_GetSettingString(_T("public"),_T("IDS_ERR_NONUMBER"),theApp.m_szRes,szTxt))
			AfxMessageBox(szTxt);
		return;
	}

	if( !xCheckData() )
		return;

	DEQPHONEDATA deqData;
	CPhoneData data;
	//name
	if(m_edName.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_Name);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//check type
	m_edData.GetWindowText(szTxt,MAX_PATH);
	switch(m_cbType.GetCurSel()){
	case 0:		//general
		data.SetType(PBK_Number_General);
		data.SetText(szTxt);
		deqData.push_back(data);
		break;
	case 1:		//mobile
		data.SetType(PBK_Number_Mobile);
		data.SetText(szTxt);
		deqData.push_back(data);
		break;
	case 2:		//fax
		data.SetType(PBK_Number_Fax);
		data.SetText(szTxt);
		deqData.push_back(data);
		break;
	case 3:		//pager
		data.SetType(PBK_Number_Pager);
		data.SetText(szTxt);
		deqData.push_back(data);
		break;
	case 4:		//company
		data.SetType(PBK_Number_Work);
		data.SetText(szTxt);
		deqData.push_back(data);
		break;
	case 5:		//home
		data.SetType(PBK_Number_Home);
		data.SetText(szTxt);
		deqData.push_back(data);
		break;
	case 6:		//e-mail
		data.SetType(PBK_Text_Email);
		data.SetText(szTxt);
		deqData.push_back(data);
		break;
	case 7:		//Video
		data.SetType(PBK_Number_Other);
		data.SetText(szTxt);
		deqData.push_back(data);
		break;

	}
	//group
	data.SetType(PBK_Caller_Group);			
	data.SetNumber(m_cbGroup.GetCurSel()+1);
	deqData.push_back(data);
	//detain data
	for(DEQPHONEDATA::iterator iter = m_deqDetainData.begin() ; iter != m_deqDetainData.end() ; iter ++){
		deqData.push_back(*iter);
	}

	///for new Data  ++
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
	//NickName
	if(m_edNickName.GetStyle() & WS_VISIBLE)
	{
		if(m_edNickName.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_Nickname);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	//street1
	if(m_edStreet1.GetStyle() & WS_VISIBLE)
	{
		if(m_edStreet1.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_StreetAddress);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	//street2
	if(m_edStreet2.GetStyle() & WS_VISIBLE)
	{
		if(m_edStreet2.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_StreetAddress2);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	//state
	if(m_edState.GetStyle() & WS_VISIBLE)
	{
		if(m_edState.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_State);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	//zip
	if(m_edZip.GetStyle() & WS_VISIBLE)
	{
		if(m_edZip.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_Zip);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	//city

	if(m_edCity.GetStyle() & WS_VISIBLE)
	{
		if(m_edCity.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_City);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	//country
	if(m_edCountry.GetStyle() & WS_VISIBLE)
	{
		if(m_edCountry.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_Country);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}

	///for new Data  ---
	
	m_pData->SetPhoneData(deqData);

	if(m_rdSIM.GetCheck())
		m_pData->SetStorageType(SIM_NAME);
	else
		m_pData->SetStorageType(MEMORY_NAME);

	CBaseDlg::OnOK();
}

void CMotoDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CBaseDlg::OnCancel();
}


void CMotoDlg::xEnableControl(int iState)
{
	CWnd *pWnd[13] = {&m_edName,&m_cbType,&m_edData,&m_cbGroup //};
		,&m_dpBirthday,&m_chBirthday,&m_edNickName,&m_edStreet1,&m_edStreet2,&m_edCity,&m_edState,
	&m_edZip,&m_edCountry};
	if(iState == ENABLE_SIM){
		pWnd[0]->EnableWindow(TRUE);
		pWnd[1]->EnableWindow(FALSE);
		pWnd[2]->EnableWindow(TRUE);
		pWnd[3]->EnableWindow(FALSE);
		for(int i = 4 ; i < 13 ; i ++){
			pWnd[i]->EnableWindow(FALSE);
		}
	}
	else if(iState == ENABLE_ME){
		for(int i = 0 ; i < 13 ; i ++){
			pWnd[i]->EnableWindow(TRUE);
		}
	}
	else if(iState == ENABLE_VIEW){
		for(int i = 0 ; i < 13 ; i ++){
			pWnd[i]->EnableWindow(FALSE);
		}
		m_rdSIM.EnableWindow(FALSE);
		m_rdME.EnableWindow(FALSE);
	}
	OnChBirthday();
}

CMotoDlg::xSetComboItem()
{
	//set group combo item
	int iCount = 0;
	TCHAR szTxt[MAX_PATH];
	//get group count
	::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_CARDPANEL_GROUPCOUNT,reinterpret_cast<WPARAM>(&iCount),0);
	if(iCount > 0){
		for(int i = 1 ; i <= iCount ; i ++){
			//get group item
			::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_CARDPANEL_GETGROUP,i,reinterpret_cast<WPARAM>(szTxt));
			//add to combo
			m_cbGroup.AddString(szTxt);
		}
		m_cbGroup.SetCurSel(0);
	}

	//set type item
	TCHAR szType[7][32]={_T("IDS_M_GENERAL"),_T("IDS_M_MOBILE"),_T("IDS_M_FAX"),_T("IDS_M_BBCALL")
		,_T("IDS_M_COMPANY"),_T("IDS_M_HOME"),_T("IDS_M_EMAIL")};
	for(int i = 0 ; i < 7 ; i ++){
		if(al_GetSettingString(_T("public"),szType[i],theApp.m_szRes,szTxt))
			m_cbType.AddString(szTxt);
	}
	if(_tcscmp(theApp.m_szPhone,_T("E1000")) == 0)
	{
		if(al_GetSettingString(_T("public"),_T("IDS_M_VIDEO"),theApp.m_szRes,szTxt))
			m_cbType.AddString(szTxt);
	}
	m_cbType.SetCurSel(0);

}

CMotoDlg::xSetDataToDlgCtrl()
{
	TCHAR szDataText[MAX_PATH];
	szDataText[0] ='\0';

	if(m_pData){
		//get data deq
		DEQPHONEDATA *pdeqData; 
		pdeqData = m_pData->GetPhoneData();
		for(DEQPHONEDATA::iterator iter = pdeqData->begin() ; iter != pdeqData->end() ; iter ++){
			if((*iter).GetType() == PBK_Text_Name){		//name
				m_edName.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_General){	//general
				m_cbType.SetCurSel(0);
				m_edData.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Mobile){	//mobile
				m_cbType.SetCurSel(1);
				m_edData.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Fax){	//fax
				m_cbType.SetCurSel(2);
				m_edData.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Pager){	//B.B. Call
				m_cbType.SetCurSel(3);
				m_edData.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Work){	//company
				m_cbType.SetCurSel(4);
				m_edData.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Home){	//home
				m_cbType.SetCurSel(5);
				m_edData.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Email){	//email
				m_cbType.SetCurSel(6);
				m_edData.SetNumFlag(false);
				m_edData.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Other){	//Video
				m_cbType.SetCurSel(7);
				m_edData.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Caller_Group){		//group
				if(strcmp(m_pData->GetStorageType(),SIM_NAME) == 0)
					m_cbGroup.SetCurSel(0);
				else
					m_cbGroup.SetCurSel((*iter).GetNumber()-1);
			}
//////////
			else if((*iter).GetType() == PBK_Text_Nickname){	//nickname
				m_edNickName.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_StreetAddress){	//street 1
				m_edStreet1.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_StreetAddress2){	//street 2
				m_edStreet2.SetWindowText((*iter).GetText());
			}

			else if((*iter).GetType() == PBK_Text_Zip){	//post code
				m_edZip.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_City){	//city
				m_edCity.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Country){	//country
				m_edCountry.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_State){	//State
				m_edState.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Date){		//birthday
				COleDateTime tm;
				(*iter).GetTime(tm);
				m_dpBirthday.SetTime(tm);
				m_chBirthday.SetCheck(1);
				if(m_iMode!=2)
					m_dpBirthday.EnableWindow(true);
			}		
/////////
			else{	//others
				m_deqDetainData.push_back((*iter));
			}
		}
		OnSelchangeCbType();
	}

}

BOOL CMotoDlg::xCheckData()
{
	CString str;
	TCHAR szText[MAX_PATH];

//	m_rbnCancel.SetWindowText(szText);
	int nPos;
	if(m_cbType.GetCurSel() == 6)	{	//e-mail
		m_edData.GetWindowText(str);	//Type
		nPos = str.Find('@');
		if(nPos==-1 || nPos==0 || nPos==str.GetLength()-1)	{
			al_GetSettingString(_T("public"), _T("IDS_ERR_EMAIL"), theApp.m_szRes, szText);
			AfxMessageBox(szText);
			return FALSE;
		}
		nPos = str.Find(' ');
		if(nPos!=-1 )	{
			al_GetSettingString(_T("public"), _T("IDS_ERR_EMAIL"), theApp.m_szRes, szText);
			AfxMessageBox(szText);
			return FALSE;
		}
		return TRUE;


	}
	return TRUE;

}

void CMotoDlg::OnChBirthday() 
{
	// TODO: Add your control notification handler code here
	if(m_chBirthday.GetCheck())	
		m_dpBirthday.EnableWindow(TRUE);
	else
		m_dpBirthday.EnableWindow(FALSE);
	
}
