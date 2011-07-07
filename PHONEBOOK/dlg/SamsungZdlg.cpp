// SharpT300Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "SamsungZdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSamsungZDlg dialog


CSamsungZDlg::CSamsungZDlg(CWnd* pParent /*=NULL*/)
	: CBaseDlg(CSamsungZDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSamsungZDlg)
	//}}AFX_DATA_INIT
	m_hFont = NULL;
}
CSamsungZDlg::~CSamsungZDlg()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}

void CSamsungZDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSamsungZDlg)
	DDX_Control(pDX, IDC_SC_DEP, m_scDep);
	DDX_Control(pDX, IDC_DEP_EDIT, m_edDep);
	DDX_Control(pDX, IDC_URL_EDIT, m_edURL);
	DDX_Control(pDX, IDC_SC_URL, m_scURL);
	DDX_Control(pDX, IDC_JOBTITLE_EDIT, m_edJobTitle);
	DDX_Control(pDX, IDC_SC_JOBTITLE, m_scJobTitle);
	DDX_Control(pDX, IDC_NICKNAME_EDIT, m_edNickName);
	DDX_Control(pDX, IDC_SC_NICKNAME, m_scNickName);
	DDX_Control(pDX, IDC_SC_COMPANY, m_scCompany);
	DDX_Control(pDX, IDC_COMPANY_EDIT, m_edCompany);
	DDX_Control(pDX, IDC_ED_LASTNAME_, m_edLastName);
	DDX_Control(pDX, IDC_ED_FIRSTNAME_, m_edFirstName);
	DDX_Control(pDX, IDC_SC_LASTNAME, m_scLastName);
	DDX_Control(pDX, IDC_SC_FIRSTNAME_, m_scFirstName);
	DDX_Control(pDX, IDOK, m_rbnOK);
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDC_SC_DOT5, m_scDot5);
	DDX_Control(pDX, IDC_SC_DOT4, m_scDot4);
	DDX_Control(pDX, IDC_SC_DOT3, m_scDot3);
	DDX_Control(pDX, IDC_SC_DOT2, m_scDot2);
	DDX_Control(pDX, IDC_SC_DOT1, m_scDot1);
	DDX_Control(pDX, IDC_SC_LINE, m_scLine);
	DDX_Control(pDX, IDC_SC_STORAGE, m_scStorage);
	DDX_Control(pDX, IDC_SC_PHONE5, m_scPhone5);
	DDX_Control(pDX, IDC_SC_PHONE4, m_scPhone4);
	DDX_Control(pDX, IDC_SC_PHONE3, m_scPhone3);
	DDX_Control(pDX, IDC_SC_PHONE2, m_scPhone2);
	DDX_Control(pDX, IDC_SC_PHONE1, m_scPhone1);
	DDX_Control(pDX, IDC_SC_NOTE, m_scNote);
	DDX_Control(pDX, IDC_SC_EMAIL2, m_scEmail2);
	DDX_Control(pDX, IDC_SC_EMAIL1, m_scEmail1);
	DDX_Control(pDX, IDC_COMBO_PHONE5, m_cbPhone5);
	DDX_Control(pDX, IDC_COMBO_PHONE4, m_cbPhone4);
	DDX_Control(pDX, IDC_COMBO_PHONE3, m_cbPhone3);
	DDX_Control(pDX, IDC_COMBO_PHONE2, m_cbPhone2);
	DDX_Control(pDX, IDC_COMBO_PHONE1, m_cbPhone1);
	DDX_Control(pDX, IDC_NOTE_EDIT, m_edNote);
	DDX_Control(pDX, IDC_PHONE_EDIT5, m_edPhone5);
	DDX_Control(pDX, IDC_PHONE_EDIT4, m_edPhone4);
	DDX_Control(pDX, IDC_PHONE_EDIT3, m_edPhone3);
	DDX_Control(pDX, IDC_PHONE_EDIT2, m_edPhone2);
	DDX_Control(pDX, IDC_PHONE_EDIT1, m_edPhone1);
	DDX_Control(pDX, IDC_EMAIL_EDIT2, m_edEmail2);
	DDX_Control(pDX, IDC_EMAIL_EDIT1, m_edEmail1);
	DDX_Control(pDX, IDC_RD_SIM, m_rdSIM);
	DDX_Control(pDX, IDC_RD_PHONE, m_rdME);
	DDX_Control(pDX, IDC_DP_BIRTHDAY, m_dpBirthday);
	DDX_Control(pDX, IDC_CH_BIRTHDAY, m_chBirthday);
	DDX_Control(pDX, IDC_SC_BIRTHDAY, m_scBirthday);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSamsungZDlg, CBaseDlg)
	//{{AFX_MSG_MAP(CSamsungZDlg)
	ON_BN_CLICKED(IDC_RD_PHONE, OnRdMobile)
	ON_BN_CLICKED(IDC_RD_SIM, OnRdSim)
	ON_BN_CLICKED(IDC_CH_BIRTHDAY, OnChBirthday)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSamsungZDlg message handlers

BOOL CSamsungZDlg::OnInitDialog() 
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
	sProfile += Tstring(_T("organize\\samsungzdlg.ini"));

	al_GetSettingString(_T("public"),_T("IDS_N_LASTNAME"),theApp.m_szRes,m_szLastName);
	al_GetSettingString(_T("public"),_T("IDS_NAME"),theApp.m_szRes,m_szName);

	CStaticEx *pStaticEx[22] = {&m_scPhone1,&m_scPhone2,&m_scPhone3,&m_scPhone4,&m_scPhone5,&m_scEmail1,&m_scEmail2
		,&m_scNickName,&m_scURL,&m_scJobTitle,&m_scNote,&m_scDot1,&m_scDot2,&m_scDot3,&m_scDot4,&m_scDot5,&m_scStorage,
	&m_scLastName,&m_scFirstName,&m_scCompany,&m_scBirthday,&m_scDep};
	TCHAR szScSec[22][32] = {_T("phone1_sc"),_T("phone2_sc"),_T("phone3_sc"),_T("phone4_sc"),_T("phone5_sc"),_T("email1_sc"),_T("email2_sc"),
		_T("nickname_sc"),_T("url_sc"),_T("jobtitle_sc"),_T("note_sc"),_T("dot1_sc"),_T("dot2_sc"),_T("dot3_sc"),_T("dot4_sc"),_T("dot5_sc"),_T("Storage_sc"),
	_T("lastname_sc"),_T("firstname_sc"),_T("company_sc"),_T("birthday_sc"),_T("Department_sc")};
	TCHAR szScID[22][32] = {_T("IDS_PHONE1"),_T("IDS_PHONE2"),_T("IDS_PHONE3"),_T("IDS_PHONE4"),_T("IDS_PHONE5"),_T("IDS_N_EMAIL"),_T("IDS_N_EMAIL2"),
		_T("IDS_NICKNAME"),_T("IDS_N_URL"),_T("IDS_N_JPBTITLE"),_T("IDS_N_NOTE"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_STORAGE"),
	_T("IDS_N_LASTNAME"),_T("IDS_N_FIRSTNAME"),_T("IDS_N_COMPANY"),_T("IDS_N_BIRTHDAY"),_T("IDS_N_CATEGORY")};

	for(int i = 0 ; i < 22 ; i ++)
		GetStaticFromSetting(pStaticEx[i],szScSec[i],szScID[i],const_cast<TCHAR*>(sProfile.c_str()));

	//load button 
	CRescaleButton	*pButton[2] = {&m_rbnOK,&m_rbnCancel};
	TCHAR szBnSec[2][32] = {_T("ok"),_T("cancel")};
	TCHAR szBnID[2][32] = {_T("IDS_N_OK"),_T("IDS_CANCEL")};
	for(i = 0 ; i < 2 ; i ++)
		GetButtonFromSetting(pButton[i],szBnSec[i],szBnID[i],0,const_cast<TCHAR*>(sProfile.c_str()));
	
	//load checkbox
	CCheckEx2	*pCheck[1] = {&m_chBirthday};
	TCHAR szChSec[1][32] = {_T("birthday_ch")};
	for(i = 0 ; i < 1 ; i ++){
		GetCheckFromSetting2(pCheck[i],szChSec[i],0,const_cast<TCHAR*>(sProfile.c_str()));
		pCheck[i]->SetCheck(0);
	}

	//load radio
	CRadioEx	*pRadio[2] = {&m_rdSIM,&m_rdME};
	TCHAR szRdSec[2][32] = {_T("sim_rd"),_T("me_rd")};
	TCHAR szRdID[2][32] = {_T("IDS_SIMCARD"),_T("IDS_MOBILE")};
	for(i = 0 ; i < 2 ; i ++){
		GetRadioFromSetting(pRadio[i],szRdSec[i],szRdID[i],const_cast<TCHAR*>(sProfile.c_str()));
		pRadio[i]->SetCheck(0);
	}
	CWnd *pWnd[22] = {&m_cbPhone1,&m_cbPhone2,&m_cbPhone3,&m_cbPhone4,&m_cbPhone5,
		&m_edPhone1,&m_edPhone2,&m_edPhone3,&m_edPhone4,&m_edPhone5,&m_edEmail1,&m_edEmail2,
		&m_edNickName,&m_edJobTitle,&m_edURL,&m_edNote,&m_edLastName,&m_edFirstName,&m_scLine,
		&m_edCompany,&m_dpBirthday,&m_edDep};
	TCHAR szWndSec[22][32] = {_T("phone1_cb"),_T("phone2_cb"),_T("phone3_cb"),_T("phone4_cb"),_T("phone5_cb"),
		_T("phone1_ed"),_T("phone2_ed"),_T("phone3_ed"),_T("phone4_ed"),_T("phone5_ed"),_T("email1_ed"),_T("email2_ed"),
		_T("nickname_ed"),_T("jobtitle_ed"),_T("url_ed"),_T("note_ed"),_T("lastname_ed"),_T("firstname_ed"),_T("line"),
		_T("company_ed"),_T("birthday_dp"),_T("department_ed")};
	for(i = 0 ; i < 22 ; i ++){
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

	m_edURL.GetWindowRect(&rc);
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
void CSamsungZDlg::xSetComboItem(LPCTSTR szProfile)
{
	//set combo text
	TCHAR szText[5][MAX_PATH];
//	TCHAR szEmailText[2][MAX_PATH];
	int nIndex;
//	TCHAR szTxtSec[4][32] = {"IDS_N_PRIVATE","IDS_GROUP_BUSINESS","IDS_FAX","IDS_M_MOBILE"};
//	int nPhoneDataType[4] = {PBK_Number_General,PBK_Number_Work,PBK_Number_Fax,PBK_Number_Mobile};
	TCHAR szTxtSec[5][32] = {_T("IDS_M_MOBILE"),_T("IDS_GROUP_BUSINESS"),_T("IDS_FAX"),_T("IDS_M_HOME"),_T("IDS_N_OTHER")};
	int nPhoneDataType[5] = {PBK_Number_Mobile,PBK_Number_Work,PBK_Number_Fax,PBK_Number_Home,PBK_Number_Other};
	//get string from language resource
	for(int i = 0 ; i < 5 ; i ++){
		al_GetSettingString(_T("public"),szTxtSec[i],theApp.m_szRes,szText[i]);
	}
	CComboBox *pCombo[5] = {&m_cbPhone1,&m_cbPhone2,&m_cbPhone3,&m_cbPhone4,&m_cbPhone5};
	for( i = 0 ; i <5 ; i ++)
	{
		for(int j = 0 ; j <5 ; j ++)
		{
			nIndex = pCombo[i]->AddString(szText[j]);
			pCombo[i]->SetItemData(nIndex,nPhoneDataType[j]);
		}
	}
}
void CSamsungZDlg::xSetDefaultControl()
{
	m_cbPhone1.SetCurSel(0);
	m_cbPhone2.SetCurSel(1);
	m_cbPhone3.SetCurSel(2);
	m_cbPhone4.SetCurSel(3);
	m_cbPhone5.SetCurSel(4);

	m_edPhone1.SetNumFlag(true);
	m_edPhone2.SetNumFlag(true);
	m_edPhone3.SetNumFlag(true);
	m_edPhone4.SetNumFlag(true);
	m_edPhone5.SetNumFlag(true);

	m_edPhone1.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edPhone2.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edPhone3.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edPhone4.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edPhone5.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edEmail1.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edEmail2.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);

	m_edNote.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edLastName.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edFirstName.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);

	m_edNickName.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edJobTitle.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edCompany.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edURL.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_edDep.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);

	xSetRadioBtn();
}

void CSamsungZDlg::xSetDataToDlgCtrl()
{
	DEQPHONEDATA *pDeqData = m_pData->GetPhoneData();
	CString strName;
	strName.Empty();
	m_pData->GetName(strName.GetBuffer(MAX_PATH));
	strName.ReleaseBuffer();
	//set data to control
	int iNumber = 0,iData = 0,iMail=0;
	CComboBox *pPhoneCombo[5] = {&m_cbPhone1,&m_cbPhone2,&m_cbPhone3,&m_cbPhone4,&m_cbPhone5};
	CMyEdit *pPhoneEdit[5] = {&m_edPhone1,&m_edPhone2,&m_edPhone3,&m_edPhone4,&m_edPhone5};
	CMyEdit *pEmail[2] = {&m_edEmail1,&m_edEmail2};

	for(DEQPHONEDATA::iterator iter = pDeqData->begin() ; iter != pDeqData->end() ; iter ++)
	{
		CCheckEx *pCh = NULL;
		CComboBox *pCb = NULL;
		CMyEdit *pEdit = NULL;
		if((*iter).GetType() == PBK_Text_FirstName){	//first name
				m_edFirstName.SetWindowText((*iter).GetText());
		}
		else if((*iter).GetType() == PBK_Text_LastName){	//last name
				m_edLastName.SetWindowText((*iter).GetText());
		}
		else if((*iter).GetType() == PBK_Number_General && iNumber < 5){
			pPhoneCombo[iNumber]->SetCurSel(0);
			pEdit = pPhoneEdit[iNumber];
			iNumber ++;
		}
		else if((*iter).GetType() == PBK_Number_Mobile && iNumber < 5){
			pPhoneCombo[iNumber]->SetCurSel(0);
			pEdit = pPhoneEdit[iNumber];
			iNumber ++;
		}
		else if((*iter).GetType() == PBK_Number_Work && iNumber < 5){
			pPhoneCombo[iNumber]->SetCurSel(1);
			pEdit = pPhoneEdit[iNumber];
			iNumber ++;
		}
		else if((*iter).GetType() == PBK_Number_Fax && iNumber < 5){
			pPhoneCombo[iNumber]->SetCurSel(2);
			pEdit = pPhoneEdit[iNumber];
			iNumber ++;
		}
		else if((*iter).GetType() == PBK_Number_Home && iNumber < 5){
			pPhoneCombo[iNumber]->SetCurSel(3);
			pEdit = pPhoneEdit[iNumber];
			iNumber ++;
		}
		else if((*iter).GetType() == PBK_Number_Other && iNumber < 5){
			pPhoneCombo[iNumber]->SetCurSel(4);
			pEdit = pPhoneEdit[iNumber];
			iNumber ++;
		}
		else if((*iter).GetType() == PBK_Text_Email && iMail <2){
			pEdit =pEmail[iMail];
			iMail++;
		}
		else if((*iter).GetType() == PBK_Text_Nickname){
			pEdit = &m_edNickName;
		}
		else if((*iter).GetType() == PBK_Text_JobTitle){
			pEdit = &m_edJobTitle;
		}
		else if((*iter).GetType() == PBK_Text_Custom1){
			pEdit = &m_edDep;
		}
		else if((*iter).GetType() == PBK_Text_URL){
			pEdit = &m_edURL;
		}
		else if((*iter).GetType() == PBK_Date){		//birthday
			COleDateTime tm;
			(*iter).GetTime(tm);
			m_dpBirthday.SetTime(tm);
			m_chBirthday.SetCheck(1);
		}	
		else if((*iter).GetType() == PBK_Text_Note){
			pEdit = &m_edNote;
		}
		else if((*iter).GetType() == PBK_Text_Company){
			pEdit = &m_edCompany;
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
		m_edLastName.SetWindowText(strName);
	}
	xSetRadioBtn();
}

void CSamsungZDlg::xEnableControl(int iState)
{
	CWnd *pWnd[22] = {&m_edLastName,&m_edFirstName,&m_cbPhone1,&m_edPhone1,&m_cbPhone2,&m_edPhone2,
		&m_cbPhone3,&m_edPhone3,&m_cbPhone4,&m_edPhone4,&m_cbPhone5,&m_edPhone5,&m_edEmail1, &m_edEmail2,&m_edNickName,&m_edJobTitle,&m_edURL,
	&m_edCompany,&m_edNote,&m_dpBirthday,&m_chBirthday,&m_edDep};
	if(iState == ENABLE_SIM){
		pWnd[0]->EnableWindow(TRUE);	
		pWnd[1]->EnableWindow(FALSE);	
		pWnd[2]->EnableWindow(FALSE);
		pWnd[3]->EnableWindow(TRUE);
		for(int i = 4 ; i < 22 ; i ++)
			pWnd[i]->EnableWindow(FALSE);
		m_scLastName.SetWindowText(m_szName);
	}
	else if(iState == ENABLE_ME)
	{
		for(int i = 0 ; i < 22 ; i ++){
			pWnd[i]->EnableWindow(TRUE);
		}
		m_scLastName.SetWindowText(m_szLastName);
	}
	else if(iState == ENABLE_VIEW)
	{
		for(int i = 0 ; i < 22 ; i ++)
		{
			pWnd[i]->EnableWindow(FALSE);
		}
		m_rdSIM.EnableWindow(FALSE);
		m_rdME.EnableWindow(FALSE);
	}
	OnChBirthday();
}
void CSamsungZDlg::xSetSupportColumnLength()
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
			m_edPhone4.SetLimitText((*iter).GetBoundary());
			m_edPhone5.SetLimitText((*iter).GetBoundary());
		}
		if((*iter).GetType() == PBK_Text_Email)
		{
			m_edEmail1.SetLimitText((*iter).GetBoundary());
			m_edEmail2.SetLimitText((*iter).GetBoundary());
		}
		if((*iter).GetType() == PBK_Text_Note)
			m_edNote.SetLimitText((*iter).GetBoundary());
		if((*iter).GetType() == PBK_Text_JobTitle)
		{
			m_edJobTitle.SetLimitText((*iter).GetBoundary());
			m_edNickName.SetLimitText((*iter).GetBoundary());
		}
		if((*iter).GetType() == PBK_Text_URL)
			m_edURL.SetLimitText((*iter).GetBoundary());
		if((*iter).GetType() == PBK_Text_Company)
		{
			m_edCompany.SetLimitText((*iter).GetBoundary());
			m_edDep.SetLimitText((*iter).GetBoundary());
		}
	}
}

void CSamsungZDlg::OnOK() 
{
	// TODO: Add extra validation here
	TCHAR szPhoneNo1[MAX_PATH],szPhoneNo2[MAX_PATH],szPhoneNo3[MAX_PATH],szPhoneNo4[MAX_PATH],szPhoneNo5[MAX_PATH],szMobile[MAX_PATH];
	m_edPhone1.GetWindowText(szPhoneNo1,MAX_PATH);
	m_edPhone2.GetWindowText(szPhoneNo2,MAX_PATH);
	m_edPhone3.GetWindowText(szPhoneNo3,MAX_PATH);
	m_edPhone4.GetWindowText(szPhoneNo4,MAX_PATH);
	m_edPhone5.GetWindowText(szPhoneNo5,MAX_PATH);
	
	if(_tcslen(szPhoneNo1) == 0 && _tcslen(szPhoneNo2) == 0 && _tcslen(szPhoneNo3) == 0 && _tcslen(szPhoneNo4) == 0&& _tcslen(szPhoneNo5) == 0) {
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
	if(m_edPhone4.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		GSM_EntryType dataType = (GSM_EntryType)m_cbPhone4.GetItemData(m_cbPhone4.GetCurSel());
		data.SetType(dataType);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edPhone5.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		GSM_EntryType dataType = (GSM_EntryType)m_cbPhone5.GetItemData(m_cbPhone5.GetCurSel());
		data.SetType(dataType);
		data.SetText(szTxt);
		deqData.push_back(data);
	}

	if(m_edEmail1.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_Email);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edEmail2.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_Email);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edNickName.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_Nickname);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edJobTitle.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_JobTitle);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edDep.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_Custom1);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edURL.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_URL);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_scBirthday.GetStyle())
	{
		if( m_chBirthday.GetCheck() )
		{
			COleDateTime tm;
			m_dpBirthday.GetTime(tm);
			data.SetType(PBK_Date);
			data.SetDate(tm);
			deqData.push_back(data);
		}
	}
/*
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
	}*/
	if(m_edNote.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_Note);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edCompany.GetWindowText(szTxt,MAX_PATH) > 0 )
	{
		data.SetType(PBK_Text_Company);
		data.SetText(szTxt);
		deqData.push_back(data);
	}

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

void CSamsungZDlg::OnRdMobile() 
{
	// TODO: Add your control notification handler code here
	m_rdME.SetCheck(TRUE);
	m_rdSIM.SetCheck(FALSE);
	xEnableControl(ENABLE_ME);
	
}

void CSamsungZDlg::OnRdSim() 
{
	// TODO: Add your control notification handler code here
	m_rdME.SetCheck(FALSE);
	m_rdSIM.SetCheck(TRUE);
	xEnableControl(ENABLE_SIM);
	
}

void CSamsungZDlg::OnChBirthday() 
{
	// TODO: Add your control notification handler code here
	if(m_chBirthday.GetCheck() && m_iMode!=2)	
		m_dpBirthday.EnableWindow(TRUE);
	else
		m_dpBirthday.EnableWindow(FALSE);
	
}
