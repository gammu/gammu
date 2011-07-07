// SharpT300Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "Sharp90Xdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSharp90XDlg dialog


CSharp90XDlg::CSharp90XDlg(CWnd* pParent /*=NULL*/)
	: CBaseDlg(CSharp90XDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSharp90XDlg)
	//}}AFX_DATA_INIT
	m_hFont = NULL;
}
CSharp90XDlg::~CSharp90XDlg()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}

void CSharp90XDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSharp90XDlg)
	DDX_Control(pDX, IDC_ED_LASTNAME_, m_edLastName);
	DDX_Control(pDX, IDC_ED_FIRSTNAME_, m_edFirstName);
	DDX_Control(pDX, IDC_SC_ZIPCODE, m_scZipCode);
	DDX_Control(pDX, IDC_SC_STREET, m_scStreet);
	DDX_Control(pDX, IDC_SC_STATE, m_scState);
	DDX_Control(pDX, IDC_SC_LASTNAME, m_scLastName);
	DDX_Control(pDX, IDC_SC_FIRSTNAME_, m_scFirstName);
	DDX_Control(pDX, IDC_SC_EMAIL3, m_scEmial3);
	DDX_Control(pDX, IDC_SC_COUNTRY, m_scCountry);
	DDX_Control(pDX, IDC_SC_CITY, m_scCity);
	DDX_Control(pDX, IDC_SC_BIRTHDAY, m_scBirthday);
	DDX_Control(pDX, IDC_EMAIL_EDIT3, m_edEmail3);
	DDX_Control(pDX, IDC_EMAIL_COMBO3, m_cbEmail3);
	DDX_Control(pDX, IDC_ED_ZIPCODE, m_edZipCode);
	DDX_Control(pDX, IDC_ED_STREET, m_edStreet);
	DDX_Control(pDX, IDC_ED_STATE, m_edState);
	DDX_Control(pDX, IDC_ED_COUNTRY, m_edCountry);
	DDX_Control(pDX, IDC_ED_CITY, m_edCity);
	DDX_Control(pDX, IDC_DP_BIRTHDAY, m_dpBirthday);
	DDX_Control(pDX, IDC_CH_BIRTHDAY, m_chBirthday);
	DDX_Control(pDX, IDOK, m_rbnOK);
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDC_SC_DOT6, m_scDot6);
	DDX_Control(pDX, IDC_SC_DOT5, m_scDot5);
	DDX_Control(pDX, IDC_SC_DOT4, m_scDot4);
	DDX_Control(pDX, IDC_SC_DOT3, m_scDot3);
	DDX_Control(pDX, IDC_SC_DOT2, m_scDot2);
	DDX_Control(pDX, IDC_SC_DOT1, m_scDot1);
	DDX_Control(pDX, IDC_SC_LINE, m_scLine);
	DDX_Control(pDX, IDC_SC_STORAGE, m_scStorage);
	DDX_Control(pDX, IDC_SC_PHONE3, m_scPhone3);
	DDX_Control(pDX, IDC_SC_PHONE2, m_scPhone2);
	DDX_Control(pDX, IDC_SC_PHONE1, m_scPhone1);
	DDX_Control(pDX, IDC_SC_NOTE, m_scNote);
	DDX_Control(pDX, IDC_SC_GROUP, m_scGroup);
	DDX_Control(pDX, IDC_SC_EMAIL2, m_scEmail2);
	DDX_Control(pDX, IDC_SC_EMAIL1, m_scEmail1);
	DDX_Control(pDX, IDC_GROUP_COMBO, m_cbGroup);
	DDX_Control(pDX, IDC_EMAIL_COMBO2, m_cbEmail2);
	DDX_Control(pDX, IDC_EMAIL_COMBO1, m_cbEmail1);
	DDX_Control(pDX, IDC_COMBO_PHONE3, m_cbPhone3);
	DDX_Control(pDX, IDC_COMBO_PHONE2, m_cbPhone2);
	DDX_Control(pDX, IDC_COMBO_PHONE1, m_cbPhone1);
	DDX_Control(pDX, IDC_NOTE_EDIT, m_edNote);
	DDX_Control(pDX, IDC_PHONE_EDIT3, m_edPhone3);
	DDX_Control(pDX, IDC_PHONE_EDIT2, m_edPhone2);
	DDX_Control(pDX, IDC_PHONE_EDIT1, m_edPhone1);
	DDX_Control(pDX, IDC_EMAIL_EDIT2, m_edEmail2);
	DDX_Control(pDX, IDC_EMAIL_EDIT1, m_edEmail1);
	DDX_Control(pDX, IDC_RD_SIM, m_rdSIM);
	DDX_Control(pDX, IDC_RD_PHONE, m_rdME);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSharp90XDlg, CBaseDlg)
	//{{AFX_MSG_MAP(CSharp90XDlg)
	ON_BN_CLICKED(IDC_RD_PHONE, OnRdMobile)
	ON_BN_CLICKED(IDC_RD_SIM, OnRdSim)
	ON_BN_CLICKED(IDC_CH_BIRTHDAY, OnChBirthday)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSharp90XDlg message handlers

BOOL CSharp90XDlg::OnInitDialog() 
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
	//profiel path 
	Tstring sProfile(theApp.m_szSkin);
	sProfile += Tstring(_T("organize\\Sharp 90X.ini"));

	al_GetSettingString(_T("public"),_T("IDS_N_LASTNAME"),theApp.m_szRes,m_szLastName);
	al_GetSettingString(_T("public"),_T("IDS_NAME"),theApp.m_szRes,m_szName);

	CStaticEx *pStaticEx[23] = {&m_scPhone1,&m_scPhone2,&m_scPhone3,&m_scEmail1,&m_scEmail2
		,&m_scEmial3,&m_scGroup,&m_scNote,&m_scDot1,&m_scDot2,&m_scDot3,&m_scDot4,&m_scDot5,&m_scDot6,&m_scStorage,
	&m_scZipCode,&m_scStreet,&m_scState,&m_scLastName,&m_scFirstName,&m_scCountry,&m_scCity,&m_scBirthday};
	TCHAR szScSec[23][32] = {_T("phone1_sc"),_T("phone2_sc"),_T("phone3_sc"),_T("email1_sc")
		,_T("email2_sc"),_T("email3_sc"),_T("group_sc"),_T("note_sc"),_T("dot1_sc"),_T("dot2_sc"),_T("dot3_sc"),_T("dot4_sc"),_T("dot5_sc"),_T("dot6_sc"),_T("Storage_sc"),
	_T("zipcode_sc"),_T("street_sc"),_T("state_sc"),_T("lastname_sc"),_T("firstname_sc"),_T("country_sc"),_T("city_sc"),_T("birthday_sc")};
	TCHAR szScID[23][32] = {_T("IDS_PHONE1"),_T("IDS_PHONE2"),_T("IDS_PHONE3"),_T("IDS_N_EMAIL"),_T("IDS_N_EMAIL2"),
		_T("IDS_N_EMAIL3"),_T("IDS_GROUP"),_T("IDS_N_NOTE"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT")
		,_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_STORAGE"),
	_T("IDS_N_POSTAL"),_T("IDS_N_STREETADDRESS"),_T("IDS_N_STATE"),_T("IDS_N_LASTNAME"),_T("IDS_N_FIRSTNAME"),_T("IDS_N_COUNTRY"),_T("IDS_N_CITY"),_T("IDS_N_BIRTHDAY")};

	for(int i = 0 ; i < 23 ; i ++)
		GetStaticFromSetting(pStaticEx[i],szScSec[i],szScID[i],const_cast<TCHAR*>(sProfile.c_str()));

	//load button 
	CRescaleButton	*pButton[2] = {&m_rbnOK,&m_rbnCancel};
	TCHAR szBnSec[2][32] = {_T("ok"),_T("cancel")};
	TCHAR szBnID[2][32] = {_T("IDS_N_OK"),_T("IDS_CANCEL")};
	for(i = 0 ; i < 2 ; i ++)
		GetButtonFromSetting(pButton[i],szBnSec[i],szBnID[i],0,const_cast<TCHAR*>(sProfile.c_str()));

	//load radio
	CRadioEx	*pRadio[2] = {&m_rdSIM,&m_rdME};
	TCHAR szRdSec[2][32] = {_T("sim_rd"),_T("me_rd")};
	TCHAR szRdID[2][32] = {_T("IDS_SIMCARD"),_T("IDS_MOBILE")};
	for(i = 0 ; i < 2 ; i ++){
		GetRadioFromSetting(pRadio[i],szRdSec[i],szRdID[i],const_cast<TCHAR*>(sProfile.c_str()));
		pRadio[i]->SetCheck(0);
	}
	//set other control position and size
	//load checkbox
	CCheckEx2	*pCheck[1] = {&m_chBirthday};
	TCHAR szChSec[1][32] = {_T("birthday_ch")};
	for(i = 0 ; i < 1 ; i ++){
		GetCheckFromSetting2(pCheck[i],szChSec[i],0,const_cast<TCHAR*>(sProfile.c_str()));
		pCheck[i]->SetCheck(0);
	}
	m_dpBirthday.EnableWindow(false);

	CWnd *pWnd[23] = {&m_cbPhone1,&m_cbPhone2,&m_cbPhone3,&m_cbEmail1,&m_cbEmail2,&m_cbEmail3,&m_cbGroup,
		&m_edPhone1,&m_edPhone2,&m_edPhone3,&m_edEmail1,&m_edEmail2,&m_edEmail3,&m_edNote,&m_edLastName,&m_edFirstName,&m_scLine,
	&m_edStreet,&m_edCity,&m_edState,&m_edZipCode,&m_edCountry,&m_dpBirthday};
	TCHAR szWndSec[23][32] = {_T("phone1_cb"),_T("phone2_cb"),_T("phone3_cb"),_T("email1_cb"),_T("email2_cb"),_T("email3_cb"),_T("group_cb"),
		_T("phone1_ed"),_T("phone2_ed"),_T("phone3_ed"),_T("email1_ed"),_T("email2_ed"),_T("email3_ed"),_T("note_ed"),_T("lastname_ed"),_T("firstname_ed"),_T("line"),
	_T("street_ed"),_T("city_ed"),_T("state_ed"),_T("zipcode_ed"),_T("country_ed"),_T("birthday_dp")};
	for(i = 0 ; i < 23 ; i ++){
		CRect rect;
		al_GetSettingRect(szWndSec[i],_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect);
		pWnd[i]->MoveWindow(rect);
		if(pFont)pWnd[i]->SetFont(pFont);

	}
	
	//get panel color
	al_GetSettingColor(_T("panel"),_T("color"),const_cast<TCHAR *>(sProfile.c_str()),m_crBg);

	CRect rect;
	//Set window pos
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

	m_edState.GetWindowRect(&rc);
	ScreenToClient(&rc);
	
	rcBtn1.OffsetRect(rc.right - rcBtn1.right  ,0);
	m_rbnCancel.MoveWindow(rcBtn1);

	rcBtn2.OffsetRect(rcBtn1.left - 20 - rcBtn2.right ,0);
	m_rbnOK.MoveWindow(rcBtn2);

	//Set combobox string
	xSetComboItem(const_cast<TCHAR *>(sProfile.c_str()));
	xSetDefaultControl();
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
		xSetDataToDlgCtrl();
	}
	else if(m_iMode == 2){	//view file
		//hide save button 
		m_rbnOK.ShowWindow(SW_HIDE);

		al_GetSettingString(_T("public"),_T("IDS_OK"),theApp.m_szRes,szText);
		m_rbnCancel.SetWindowText(szText);

		//set title
		bRet = al_GetSettingString(_T("public"),_T("IDS_VIEWCONTACT"),theApp.m_szRes,szText);
		//initialize control data
		xSetDataToDlgCtrl();

		xEnableControl(ENABLE_VIEW);

	}
	if(bRet)
		SetWindowText(szText);

	if(((CPhoneBookApp*)afxGetApp())->m_bNotSupportSM && m_iMode != 2)
	{
		m_rdME.EnableWindow(FALSE);
		m_rdSIM.EnableWindow(FALSE);
		OnRdMobile();
	}
	xSetSupportColumnLength();
	return FALSE;
}
void CSharp90XDlg::xSetComboItem(LPCTSTR szProfile)
{
	int iCount = 0;
	TCHAR szKey[MAX_PATH];
	TCHAR szTextID[MAX_PATH];
	TCHAR szGroupText[MAX_PATH];
	al_GetSettingInt(_T("Group"),_T("nCount"),szProfile,iCount);
	for(int i = 1 ; i <= iCount ; i ++)
	{
		wsprintf(szKey,_T("Group%d") ,i);
		if(al_GetSettingString(_T("Group"),szKey,szProfile,szTextID))
		{
			al_GetSettingString(_T("public"),szTextID,theApp.m_szRes,szGroupText);
			m_cbGroup.AddString(szGroupText);
		}
	}
	m_cbGroup.SetCurSel(0);

	//set combo text
	TCHAR szText[6][MAX_PATH];
	TCHAR szEmailText[2][MAX_PATH];
	int nIndex;
	TCHAR szTxtSec[6][32] = {_T("IDS_MOBILE_PRIVATE"),_T("IDS_MOBILE_BUSSINESS"),_T("IDS_HOME"),_T("IDS_OFFICE"),_T("IDS_M_VIDEO"),_T("IDS_FAX")};
	int nPhoneDataType[6] = {PBK_Number_Mobile,PBK_Number_Mobile_Work,PBK_Number_Home,PBK_Number_Work,PBK_Number_VideoCall,PBK_Number_Fax};
	TCHAR szEmailSec[2][32] = {_T("IDS_PRIVATE"),_T("IDS_GROUP_BUSINESS")};
	int nEmailDataType[2] = {PBK_Text_Email,PBK_Text_Email_Work};
	//get string from language resource
	for(i = 0 ; i < 6 ; i ++){
		al_GetSettingString(_T("public"),szTxtSec[i],theApp.m_szRes,szText[i]);
	}
	CComboBox *pCombo[3] = {&m_cbPhone1,&m_cbPhone2,&m_cbPhone3};
	for( i = 0 ; i <3 ; i ++)
	{
		for(int j = 0 ; j < 6 ; j ++)
		{
			nIndex = pCombo[i]->AddString(szText[j]);
			pCombo[i]->SetItemData(nIndex,nPhoneDataType[j]);
		}
	}

	for(i = 0 ; i < 2 ; i ++){
		al_GetSettingString(_T("public"),szEmailSec[i],theApp.m_szRes,szEmailText[i]);
	}

	for(i = 0 ; i < 2 ; i ++)
	{
		nIndex =m_cbEmail1.AddString(szEmailText[i]);
		m_cbEmail1.SetItemData(nIndex,nEmailDataType[i]);

		nIndex =m_cbEmail2.AddString(szEmailText[i]);
		m_cbEmail2.SetItemData(nIndex,nEmailDataType[i]);

		nIndex =m_cbEmail3.AddString(szEmailText[i]);
		m_cbEmail3.SetItemData(nIndex,nEmailDataType[i]);
	}
}
void CSharp90XDlg::xSetDefaultControl()
{
	m_cbPhone1.SetCurSel(0);
	m_cbPhone2.SetCurSel(1);
	m_cbPhone3.SetCurSel(2);
	m_cbEmail1.SetCurSel(0);
	m_cbEmail2.SetCurSel(1);
	m_cbEmail3.SetCurSel(0);
	m_cbGroup.SetCurSel(0);

	m_edPhone1.SetNumFlag(true);
	m_edPhone2.SetNumFlag(true);
	m_edPhone3.SetNumFlag(true);

	m_edPhone1.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edPhone2.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edPhone3.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edEmail1.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edEmail2.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edEmail3.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);

	m_edNote.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edLastName.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edFirstName.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edZipCode.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edStreet.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);

	m_edState.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edCountry.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edCity.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	
	xSetRadioBtn();
}

void CSharp90XDlg::xSetDataToDlgCtrl()
{
	DEQPHONEDATA *pDeqData = m_pData->GetPhoneData();
	
	//set data to control
	int iNumber = 0,iData = 0,iEmail = 0;
	CComboBox *pPhoneCombo[3] = {&m_cbPhone1,&m_cbPhone2,&m_cbPhone3};
	CComboBox *pEmailCombo[3] = {&m_cbEmail1,&m_cbEmail2,&m_cbEmail3};
	CMyEdit *pPhoneEdit[3] = {&m_edPhone1,&m_edPhone2,&m_edPhone3};
	CMyEdit *pEmailEdit[3] = {&m_edEmail1,&m_edEmail2,&m_edEmail3};
	CString strName;
	strName.Empty();
	m_pData->GetName(strName.GetBuffer(MAX_PATH));


	for(DEQPHONEDATA::iterator iter = pDeqData->begin() ; iter != pDeqData->end() ; iter ++)
	{
		CCheckEx *pCh = NULL;
		CComboBox *pCb = NULL;
		CMyEdit *pEdit = NULL;
	//	if((*iter).GetType() == PBK_Text_Name){		//full name
	//		m_edFirstName.SetWindowText((*iter).GetText());
	//	}
	/*	else*/ if((*iter).GetType() == PBK_Text_FirstName){	//first name
				m_edFirstName.SetWindowText((*iter).GetText());
		}
		else if((*iter).GetType() == PBK_Text_LastName){	//last name
				m_edLastName.SetWindowText((*iter).GetText());
		}
		else if((*iter).GetType() == PBK_Number_General && iNumber < 3){
			pPhoneCombo[iNumber]->SetCurSel(0);
			pEdit = pPhoneEdit[iNumber];
			iNumber ++;
		}
		else if((*iter).GetType() == PBK_Number_Mobile && iNumber < 3){
			pPhoneCombo[iNumber]->SetCurSel(0);
			pEdit = pPhoneEdit[iNumber];
			iNumber ++;
		}
		else if((*iter).GetType() == PBK_Number_Mobile_Work && iNumber < 3){
			pPhoneCombo[iNumber]->SetCurSel(1);
			pEdit = pPhoneEdit[iNumber];
			iNumber ++;
		}
		else if((*iter).GetType() == PBK_Number_Home && iNumber < 3){
			pPhoneCombo[iNumber]->SetCurSel(2);
			pEdit = pPhoneEdit[iNumber];
			iNumber ++;
		}
		else if((*iter).GetType() == PBK_Number_Work && iNumber < 3){
			pPhoneCombo[iNumber]->SetCurSel(3);
			pEdit = pPhoneEdit[iNumber];
			iNumber ++;
		}
		else if((*iter).GetType() == PBK_Number_VideoCall && iNumber < 3){
			pPhoneCombo[iNumber]->SetCurSel(4);
			pEdit = pPhoneEdit[iNumber];
			iNumber ++;
		}
		else if((*iter).GetType() == PBK_Number_Fax && iNumber < 3){
			pPhoneCombo[iNumber]->SetCurSel(5);
			pEdit = pPhoneEdit[iNumber];
			iNumber ++;
		}
		else if((*iter).GetType() == PBK_Text_Email && iEmail < 3){
			pEmailCombo[iEmail]->SetCurSel(0);
			pEdit = pEmailEdit[iEmail];
			iEmail ++;
		}
		else if((*iter).GetType() == PBK_Text_Email_Work && iEmail < 3){
			pEmailCombo[iEmail]->SetCurSel(1);
			pEdit = pEmailEdit[iEmail];
			iEmail ++;
		}
		else if((*iter).GetType() == PBK_Text_StreetAddress) {
			pEdit = &m_edStreet;
		}
		else if((*iter).GetType() == PBK_Text_City) {
			pEdit = &m_edCity;
		}
		else if((*iter).GetType() == PBK_Text_State) {
			pEdit = &m_edState;
		}
		else if((*iter).GetType() == PBK_Text_Zip) {
			pEdit = &m_edZipCode;
		}
		else if((*iter).GetType() == PBK_Text_Country) {
			pEdit = &m_edCountry;
		}
		else if((*iter).GetType() == PBK_Text_Note){
			pEdit = &m_edNote;
		}
		else if((*iter).GetType() == PBK_Caller_Group){
			m_cbGroup.SetCurSel((*iter).GetNumber());
		}
		else if((*iter).GetType() == PBK_Date){		//birthday
			COleDateTime tm;
			(*iter).GetTime(tm);
			m_dpBirthday.SetTime(tm);
			m_chBirthday.SetCheck(1);
			m_dpBirthday.EnableWindow(true);
		}		
		//keep other type data
		else if((*iter).GetType() != PBK_Text_Name){
			m_deqDetainData.push_back((*iter));
		}
		if(pEdit)
			pEdit->SetWindowText((*iter).GetText());
	}
	CString strFirstName,strLastName;
	strFirstName.Empty();
	strLastName.Empty();
	m_edFirstName.GetWindowText(strFirstName);
	m_edLastName.GetWindowText(strLastName);
	if(strFirstName.IsEmpty() && strLastName.IsEmpty() && strName.IsEmpty()== false)
	{
		m_edFirstName.SetWindowText(strName);
	}

	xSetRadioBtn();
}

void CSharp90XDlg::xEnableControl(int iState)
{
	CWnd *pWnd[23] = {&m_edFirstName,&m_edLastName,&m_cbPhone1,&m_edPhone1,&m_cbPhone2,&m_edPhone2,
		&m_cbPhone3,&m_edPhone3,&m_cbEmail1, &m_edEmail1,&m_cbEmail2, &m_edEmail2,&m_cbEmail3, &m_edEmail3,
	&m_cbGroup,&m_edNote,&m_edStreet,&m_edCity,&m_edState,&m_edZipCode,&m_edCountry,&m_dpBirthday,&m_chBirthday};
	if(iState == ENABLE_SIM){
		pWnd[0]->EnableWindow(TRUE);	
		pWnd[1]->EnableWindow(FALSE);	
		pWnd[2]->EnableWindow(FALSE);
		pWnd[3]->EnableWindow(TRUE);
		for(int i = 4 ; i < 23 ; i ++)
			pWnd[i]->EnableWindow(FALSE);
		m_scLastName.SetWindowText(m_szName);
	}
	else if(iState == ENABLE_ME)
	{
		for(int i = 0 ; i < 23 ; i ++){
			pWnd[i]->EnableWindow(TRUE);
		}
		m_scLastName.SetWindowText(m_szLastName);
	}
	else if(iState == ENABLE_VIEW)
	{
		for(int i = 0 ; i < 23 ; i ++)
		{
			pWnd[i]->EnableWindow(FALSE);
		}
		m_rdSIM.EnableWindow(FALSE);
		m_rdME.EnableWindow(FALSE);
	}
	OnChBirthday();
}
void CSharp90XDlg::xSetSupportColumnLength()
{
	DEQPHBOUNDARY *pBoundary = m_se.GetPhBoundary();

	for(DEQPHBOUNDARY::iterator iter = pBoundary->begin() ; iter != pBoundary->end() ; iter ++)
	{
		if((*iter).GetType() == PBK_Text_Name)
			m_edFirstName.SetLimitText((*iter).GetBoundary());
		if((*iter).GetType() == PBK_Text_LastName)
			m_edLastName.SetLimitText((*iter).GetBoundary());
		if((*iter).GetType() == PBK_Number_General)
		{
			m_edPhone1.SetLimitText((*iter).GetBoundary());
			m_edPhone2.SetLimitText((*iter).GetBoundary());
			m_edPhone3.SetLimitText((*iter).GetBoundary());
		}
		if((*iter).GetType() == PBK_Text_Email)
		{
			m_edEmail1.SetLimitText((*iter).GetBoundary());
			m_edEmail2.SetLimitText((*iter).GetBoundary());
			m_edEmail3.SetLimitText((*iter).GetBoundary());
		}
		if((*iter).GetType() == PBK_Text_Note)
			m_edNote.SetLimitText((*iter).GetBoundary());
	}
}

void CSharp90XDlg::OnOK() 
{
	// TODO: Add extra validation here
	TCHAR szPhoneNo1[MAX_PATH],szPhoneNo2[MAX_PATH],szPhoneNo3[MAX_PATH],szMobile[MAX_PATH];
	m_edPhone1.GetWindowText(szPhoneNo1,MAX_PATH);
	m_edPhone2.GetWindowText(szPhoneNo2,MAX_PATH);
	m_edPhone3.GetWindowText(szPhoneNo3,MAX_PATH);
	
	if(_tcslen(szPhoneNo1) == 0 && _tcslen(szPhoneNo2) == 0 && _tcslen(szPhoneNo3) == 0 ) {
		if(al_GetSettingString(_T("public"),_T("IDS_ERR_NONUMBER"),theApp.m_szRes,szMobile))
			AfxMessageBox(szMobile);
		return;
	}
	DEQPHONEDATA deqData;
	CPhoneData data;
	TCHAR szLastName[MAX_PATH],szfirstName[MAX_PATH],szTxt[MAX_PATH];
	//name or first name
	if(m_edLastName.GetWindowText(szLastName,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_LastName);
		data.SetText(szLastName);
		deqData.push_back(data);
	}
	if(m_edFirstName.GetWindowText(szfirstName,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_FirstName);
		data.SetText(szfirstName);
		deqData.push_back(data);
	}

	if(m_edPhone1.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		GSM_EntryType dataType =(GSM_EntryType) m_cbPhone1.GetItemData(m_cbPhone1.GetCurSel());
		data.SetType(dataType);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edPhone2.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		GSM_EntryType dataType = (GSM_EntryType)m_cbPhone2.GetItemData(m_cbPhone2.GetCurSel());
		data.SetType(dataType);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edPhone3.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		GSM_EntryType dataType = (GSM_EntryType)m_cbPhone3.GetItemData(m_cbPhone3.GetCurSel());
		data.SetType(dataType);
		data.SetText(szTxt);
		deqData.push_back(data);
	}

	if(m_edEmail1.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		GSM_EntryType dataType = (GSM_EntryType)m_cbEmail1.GetItemData(m_cbEmail1.GetCurSel());
		data.SetType(dataType);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edEmail2.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		GSM_EntryType dataType =(GSM_EntryType) m_cbEmail2.GetItemData(m_cbEmail2.GetCurSel());
		data.SetType(dataType);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edEmail3.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		GSM_EntryType dataType =(GSM_EntryType) m_cbEmail3.GetItemData(m_cbEmail3.GetCurSel());
		data.SetType(dataType);
		data.SetText(szTxt);
		deqData.push_back(data);
	}

	if(m_edStreet.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_StreetAddress);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edCity.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_City);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edState.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_State);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edZipCode.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_Zip);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edCountry.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_Country);
		data.SetText(szTxt);
		deqData.push_back(data);
	}

	if(m_edNote.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_Note);
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
	data.SetType(PBK_Caller_Group);			
	data.SetNumber(m_cbGroup.GetCurSel());
	deqData.push_back(data);
	
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

void CSharp90XDlg::OnRdMobile() 
{
	// TODO: Add your control notification handler code here
	m_rdME.SetCheck(TRUE);
	m_rdSIM.SetCheck(FALSE);
	xEnableControl(ENABLE_ME);
	
}

void CSharp90XDlg::OnRdSim() 
{
	// TODO: Add your control notification handler code here
	m_rdME.SetCheck(FALSE);
	m_rdSIM.SetCheck(TRUE);
	xEnableControl(ENABLE_SIM);
	
}

void CSharp90XDlg::OnChBirthday() 
{
	// TODO: Add your control notification handler code here
	if(m_chBirthday.GetCheck() && m_iMode!=2)	
		m_dpBirthday.EnableWindow(TRUE);
	else
		m_dpBirthday.EnableWindow(FALSE);
	
}
