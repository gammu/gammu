// asusdlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "asusdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CASUSDlg dialog


CASUSDlg::CASUSDlg(CWnd* pParent /*=NULL*/)
	: CBaseDlg(IDD_DLG_ASUS, pParent)
{
	m_crBg = 0;
	AfxInitRichEdit();

	//{{AFX_DATA_INIT(CASUSDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CASUSDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CASUSDlg)
	DDX_Control(pDX, IDC_DP_BD, m_dpBirthday);
	DDX_Control(pDX, IDC_RD_SIM, m_rdSIM);
	DDX_Control(pDX, IDC_RD_MOBILE, m_rdME);
	DDX_Control(pDX, IDOK, m_rbnOk);
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDC_SC_TITLE, m_scTitle);
	DDX_Control(pDX, IDC_SC_TEL4, m_scTel4);
	DDX_Control(pDX, IDC_SC_TEL3, m_scTel3);
	DDX_Control(pDX, IDC_SC_TEL2, m_scTel2);
	DDX_Control(pDX, IDC_SC_TEL1, m_scTel1);
	DDX_Control(pDX, IDC_SC_STORAGE, m_scStorage);
	DDX_Control(pDX, IDC_SC_SEX, m_scSex);
	DDX_Control(pDX, IDC_SC_NAME, m_scName);
	DDX_Control(pDX, IDC_SC_MEMO, m_scMemo);
	DDX_Control(pDX, IDC_SC_MAIL, m_scMail);
	DDX_Control(pDX, IDC_SC_LOCATION, m_scLocation);
	DDX_Control(pDX, IDC_SC_LOCA, m_scLocalView);
	DDX_Control(pDX, IDC_SC_LINE, m_scLine);
	DDX_Control(pDX, IDC_SC_INDICATOR, m_scLED);
	DDX_Control(pDX, IDC_SC_GROUP, m_scGrouop);
	DDX_Control(pDX, IDC_SC_FAX, m_scFax);
	DDX_Control(pDX, IDC_SC_CO, m_scCompany);
	DDX_Control(pDX, IDC_SC_BIRTHDAY, m_scBirthday);
	DDX_Control(pDX, IDC_SC_ADDRESS, m_scAddress);
	DDX_Control(pDX, IDC_RED_MEMO, m_edMemo);
	DDX_Control(pDX, IDC_ED_TITLE, m_edTitle);
	DDX_Control(pDX, IDC_ED_TEL4, m_edTel4);
	DDX_Control(pDX, IDC_ED_TEL3, m_edTel3);
	DDX_Control(pDX, IDC_ED_TEL2, m_edTel2);
	DDX_Control(pDX, IDC_ED_TEL1, m_edTel1);
	DDX_Control(pDX, IDC_ED_NAME, m_edName);
	DDX_Control(pDX, IDC_ED_MAIL, m_edMail);
	DDX_Control(pDX, IDC_ED_FAX, m_edFax);
	DDX_Control(pDX, IDC_ED_CO, m_edCompany);
	DDX_Control(pDX, IDC_ED_ADDRESS, m_edAddress);
	DDX_Control(pDX, IDC_COMBO_SEX, m_cbSex);
	DDX_Control(pDX, IDC_COMBO_INDICATOR, m_cbLED);
	DDX_Control(pDX, IDC_COMBO_GROUP, m_cbGroup);
	DDX_Control(pDX, IDC_CH_BIRHDAY, m_chBirthday);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CASUSDlg, CBaseDlg)
	//{{AFX_MSG_MAP(CASUSDlg)
	ON_BN_CLICKED(IDC_RD_MOBILE, OnRdMobile)
	ON_BN_CLICKED(IDC_RD_SIM, OnRdSim)
	ON_BN_CLICKED(IDC_CH_BIRHDAY, OnChBirthday)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CASUSDlg message handlers

BOOL CASUSDlg::OnInitDialog() 
{
	CBaseDlg::OnInitDialog();

	//get profile path
	Tstring sSkin(theApp.m_szSkin);
	Tstring sProfile = sSkin + Tstring(_T("Organize\\ASUSDlg.ini"));

	//Load static
	CStaticEx	*pStaticEx[19] = {&m_scName,&m_scTel1,&m_scTel2,&m_scTel3,&m_scTel4
		,&m_scFax,&m_scTitle,&m_scCompany,&m_scStorage,&m_scMail,&m_scAddress
		,&m_scBirthday,&m_scSex,&m_scGrouop,&m_scLED,&m_scMemo,&m_scLocation
		,&m_scLocalView,&m_scLine};
	TCHAR szScSec[19][32] = {_T("name_sc"),_T("tel1_sc"),_T("tel2_sc"),_T("te3_sc")
		,_T("te4_sc"),_T("fax_sc"),_T("title_sc"),_T("company_sc"),_T("storage_sc"),_T("email_sc")
		,_T("address_sc"),_T("birthday_sc"),_T("sex_sc"),_T("group_sc"),_T("led_sc"),_T("memo_sc")
		,_T("location_sc"),_T("locationview_sc"),_T("line")};
	TCHAR szScID[19][32] = {_T("IDS_A_NAME"),_T("IDS_A_TEL1"),_T("IDS_A_TEL2"),_T("IDS_A_TEL3")
		,_T("IDS_A_TEL4"),_T("IDS_A_FAX"),_T("IDS_A_TITLE"),_T("IDS_A_COMPANY"),_T("IDS_A_STORAGE"),_T("IDS_A_EMAIL"),_T("IDS_A_ADDRESS")
		,_T("IDS_A_BIRTHDAY"),_T("IDS_A_SEX"),_T("IDS_A_GROUP"),_T("IDS_A_LED"),_T("IDS_A_MEMO"),_T("IDS_A_LOCATION"),_T(""),_T("")};
	for(int i = 0 ; i < 19 ; i ++)
		GetStaticFromSetting(pStaticEx[i],szScSec[i],szScID[i],const_cast<TCHAR*>(sProfile.c_str()));

	//load button 
	CRescaleButton	*pButton[2] = {&m_rbnOk,&m_rbnCancel};
	TCHAR szBnSec[2][32] = {_T("ok"),_T("cancel")};
	TCHAR szBnID[2][32] = {_T("IDS_OK"),_T("IDS_CANCEL")};
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
	CWnd *pWnd[15] = {&m_edName,&m_edTel1,&m_edTel2,&m_edTel3,&m_edTel4,&m_edFax,&m_edTitle,
		&m_edCompany,&m_edMail,&m_edAddress,&m_dpBirthday,&m_cbSex,&m_cbGroup,&m_cbLED,&m_edMemo};
	TCHAR szWndSec[15][32] = {_T("name_ed"),_T("tel1_ed"),_T("tel2_ed"),_T("tel3_ed"),_T("tel4_ed"),_T("fax_ed"),
		_T("title_ed"),_T("company_ed"),_T("mail_ed"),_T("address_ed"),_T("birthday_dp"),_T("sex_cb"),_T("group_cb"),
		_T("led_cb"),_T("memo_ed")};
	for(i = 0 ; i < 15 ; i ++){
		CRect rect;
		al_GetSettingRect(szWndSec[i],_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect);
		pWnd[i]->MoveWindow(rect);
	}

	m_edTel1.SetNumFlag(true);
	m_edTel2.SetNumFlag(true);
	m_edTel3.SetNumFlag(true);
	m_edTel4.SetNumFlag(true);
	m_edFax.SetNumFlag(true);

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

	m_edMail.GetWindowRect(&rc);
	ScreenToClient(&rc);
	
	rcBtn1.OffsetRect(rc.right - rcBtn1.right  ,0);
	m_rbnCancel.MoveWindow(rcBtn1);

	rcBtn2.OffsetRect(rcBtn1.left - 20 - rcBtn2.right ,0);
	m_rbnOk.MoveWindow(rcBtn2);

	//Set combobox string
//	xSetComboItem();
//	xSetDefaultControl();
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
//		xSetDataToDlgCtrl();
	}
	else if(m_iMode == 2){	//view file
		//hide save button 
		m_rbnOk.ShowWindow(SW_HIDE);

		al_GetSettingString(_T("public"),_T("IDS_OK"),theApp.m_szRes,szText);
		m_rbnCancel.SetWindowText(szText);

		//set title
		bRet = al_GetSettingString(_T("public"),_T("IDS_VIEWCONTACT"),theApp.m_szRes,szText);
		//initialize control data
//		xSetDataToDlgCtrl();
		xEnableControl(ENABLE_VIEW);
	}
	if(bRet)
		SetWindowText(szText);
	
	xSetComboItem();
	xSetDataToDlgCtrl();
	xSetSupportColumnLength();
	m_edName.SetFocus();
	return FALSE;
//	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CASUSDlg::OnRdMobile() 
{
	m_rdSIM.SetCheck(0);
	m_rdME.SetCheck(1);	

	xEnableControl(ENABLE_ME);
/*	for(int i = 2 ; i < 9 ; i ++)
		xEnableControl(i,FALSE);*/
}

void CASUSDlg::OnRdSim() 
{
	m_rdSIM.SetCheck(1);
	m_rdME.SetCheck(0);	

	xEnableControl(ENABLE_SIM);

/*	for(int i = 2 ; i < 9 ; i ++)
		xEnableControl(i,FALSE);*/
}

void CASUSDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CBaseDlg::OnCancel();
}

void CASUSDlg::OnOK() 
{
	DEQPHONEDATA deqData;
	CPhoneData data;
	TCHAR szTxt[MAX_PATH];
	//name
	if(m_edName.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_Name);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//tel1
	if(m_edTel1.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Number_Mobile);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//tel2
	if(m_edTel2.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Number_Home);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//tel3
	if(m_edTel3.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Number_Work);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//tel4
	if(m_edTel4.GetWindowText(szTxt,MAX_PATH) > 0){
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
	//E-mail
	if(m_edMail.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_Email);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//address
	if(m_edAddress.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_Postal);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//memo
	if(m_edMemo.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_Note);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//birthday
	if(m_chBirthday.GetCheck()){
		COleDateTime tm;
		m_dpBirthday.GetTime(tm);
		data.SetType(PBK_Date);
		data.SetDate(tm);
		deqData.push_back(data);
	}
	//group
	data.SetType(PBK_Caller_Group);			
	data.SetNumber(m_cbGroup.GetCurSel());
	deqData.push_back(data);
	//sex
	data.SetType(PBK_Number_Sex);			
	data.SetNumber(xGetSexType(m_cbSex.GetCurSel()));
	deqData.push_back(data);
	//LED
	data.SetType(PBK_Number_Light);			
	data.SetNumber(xGetLEDType(m_cbLED.GetCurSel()));
	deqData.push_back(data);
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

void CASUSDlg::xEnableControl(int iState)
{
	CWnd *pWnd[15] = {&m_edName,&m_edTel1,&m_edTel2,&m_edTel3,&m_edTel4,&m_edFax,&m_edTitle,
		&m_edCompany,&m_edMail,&m_edAddress,&m_dpBirthday,&m_cbSex,&m_cbGroup,&m_cbLED,&m_edMemo};
	if(iState == ENABLE_SIM){
		for(int i = 2 ; i < 15 ; i++){
			pWnd[i]->EnableWindow(FALSE);
		}
		m_chBirthday.EnableWindow(FALSE);
	}
	else if(iState == ENABLE_ME){
		for(int i = 0 ; i < 15 ; i ++){
			pWnd[i]->EnableWindow(TRUE);
		}
		m_chBirthday.EnableWindow(TRUE);
	}
	else if(iState == ENABLE_VIEW){
		for(int i = 0 ; i < 15 ; i ++){
			pWnd[i]->EnableWindow(FALSE);
		}
		m_chBirthday.EnableWindow(FALSE);
		m_rdSIM.EnableWindow(FALSE);
		m_rdME.EnableWindow(FALSE);
	}
	OnChBirthday();
}

void CASUSDlg::xSetDataToDlgCtrl()
{
	if(m_pData){
		m_chBirthday.SetCheck(0);
		//get data deq
		DEQPHONEDATA *pdeqData; 
		pdeqData = m_pData->GetPhoneData();
		for(DEQPHONEDATA::iterator iter = pdeqData->begin() ; iter != pdeqData->end() ; iter ++){
			if((*iter).GetType() == PBK_Text_Name){		//name
				m_edName.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Mobile){	//phone no.1
				m_edTel1.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Home){	//phone no.2
				m_edTel2.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Work){	//phone no.3
				m_edTel3.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Other){	//phone no.4
				m_edTel4.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Fax){	//fax
				m_edFax.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_JobTitle){	//title
				m_edTitle.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Company){		//company
				m_edCompany.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Email){	//email
				m_edMail.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Postal){	//address
				m_edAddress.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Date){		//birthday
				COleDateTime tm;
				(*iter).GetTime(tm);
				m_dpBirthday.SetTime(tm);
				m_chBirthday.SetCheck(1);
			}
			else if((*iter).GetType() == PBK_Number_Sex){		//sex
				m_cbSex.SetCurSel(xGetSexItem((*iter).GetNumber()));
			}
			else if((*iter).GetType() == PBK_Number_Light){		//LED
				m_cbLED.SetCurSel(xGetLEDItem((*iter).GetNumber()));
			}
			else if((*iter).GetType() == PBK_Caller_Group){		//group
				m_cbGroup.SetCurSel((*iter).GetNumber());
			}
			else if((*iter).GetType() == PBK_Text_Note){	//note
				m_edMemo.SetWindowText((*iter).GetText());
			}
			else{	//others
				m_deqDetainData.push_back((*iter));
			}
		}
		OnChBirthday();
	}
}

void CASUSDlg::OnChBirthday() 
{
	if(m_rdME.GetCheck()){
		if(m_chBirthday.GetCheck() == 0){
			m_dpBirthday.EnableWindow(FALSE);
		}
		else{
			m_dpBirthday.EnableWindow(TRUE);
		}
	}
	else
		m_dpBirthday.EnableWindow(FALSE);
}

void CASUSDlg::xSetComboItem()
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

	//set sex item
	TCHAR szSex[3][32]={_T("IDS_A_NONE"),_T("IDS_A_MALE"),_T("IDS_A_FEMALE")};
	for(int i = 0 ; i < 3 ; i ++){
		if(al_GetSettingString(_T("public"),szSex[i],theApp.m_szRes,szTxt))
			m_cbSex.AddString(szTxt);
	}
	m_cbSex.SetCurSel(0);
	//set led item
	TCHAR szLED[8][32]={_T("IDS_A_DEFAULTCOLOR"),_T("IDS_A_RED"),_T("IDS_A_BLUE"),_T("IDS_A_YELLOW"),_T("IDS_A_GREEN"),
		_T("IDS_A_PURPLE"),_T("IDS_A_LIGHTBLUE"),_T("IDS_A_BLUEGREEN")};
	for( i = 0 ; i < 8 ; i ++){
		if(al_GetSettingString(_T("public"),szLED[i],theApp.m_szRes,szTxt))
			m_cbLED.AddString(szTxt);
	}
	m_cbLED.SetCurSel(0);
}

int CASUSDlg::xGetSexType(int iItem)
{
	if(iItem == 1)
		return ASUS_SEX_Male;
	else if(iItem == 2)
		return ASUS_SEX_Female;
	else
		return ASUS_SEX_None;
}

int CASUSDlg::xGetLEDType(int iItem)
{
	if(iItem == 1)
		return ASUS_LIGHT_Red;
	else if(iItem == 2)
		return  ASUS_LIGHT_Blue;
	else if(iItem == 3)
		return  ASUS_LIGHT_Yellow;
	else if(iItem == 4)
		return  ASUS_LIGHT_Green;
	else if(iItem == 5)
		return  ASUS_LIGHT_Purple;
	else if(iItem == 6)
		return  ASUS_LIGHT_LightBlue;
	else if(iItem == 7)
		return  ASUS_LIGHT_BlueGreen;
	else
		return  ASUS_LIGHT_DefaultColor;
}

int CASUSDlg::xGetSexItem(int iType)
{
	if(iType == ASUS_SEX_Male)
		return 1;
	else if(iType == ASUS_SEX_Female)
		return 2;
	else
		return 0;
}

int CASUSDlg::xGetLEDItem(int iType)
{
	if(iType == ASUS_LIGHT_DefaultColor)
		return 0;
	else if(iType == ASUS_LIGHT_Red)
		return 1;
	else if(iType == ASUS_LIGHT_Blue)
		return 2;
	else if(iType == ASUS_LIGHT_Yellow)
		return 3;
	else if(iType == ASUS_LIGHT_Green)
		return 4;
	else if(iType == ASUS_LIGHT_Purple)
		return 5;
	else if(iType == ASUS_LIGHT_LightBlue)
		return 6;
	else
		return 7;
}

void CASUSDlg::xSetSupportColumnLength()
{
	DEQPHBOUNDARY *pBoundary = m_se.GetPhBoundary();
	
	for(DEQPHBOUNDARY::iterator iter = pBoundary->begin() ; iter != pBoundary->end() ; iter ++){
		if((*iter).GetType() == PBK_Text_Name)
			m_edName.SetLimitText((*iter).GetBoundary());
		else if((*iter).GetType() == PBK_Number_Mobile)
			m_edTel1.SetLimitText((*iter).GetBoundary());
		else if((*iter).GetType() == PBK_Number_Home)
			m_edTel2.SetLimitText((*iter).GetBoundary());
		else if((*iter).GetType() == PBK_Number_Work)
			m_edTel3.SetLimitText((*iter).GetBoundary());
		else if((*iter).GetType() == PBK_Number_Other)
			m_edTel4.SetLimitText((*iter).GetBoundary());
		else if((*iter).GetType() == PBK_Number_Fax)
			m_edFax.SetLimitText((*iter).GetBoundary());
		else if((*iter).GetType() == PBK_Text_JobTitle)
			m_edTitle.SetLimitText((*iter).GetBoundary());
		else if((*iter).GetType() == PBK_Text_Company)
			m_edCompany.SetLimitText((*iter).GetBoundary());
		else if((*iter).GetType() == PBK_Text_Email)
			m_edMail.SetLimitText((*iter).GetBoundary());
		else if((*iter).GetType() == PBK_Text_Postal)
			m_edAddress.SetLimitText((*iter).GetBoundary());
		else if((*iter).GetType() == PBK_Text_Note)
			m_edMemo.SetLimitText((*iter).GetBoundary());
	}
}
