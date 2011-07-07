// SamsungDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "SamsungDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSamsungDlg dialog


CSamsungDlg::CSamsungDlg(CWnd* pParent /*=NULL*/)
	: CBaseDlg(CSamsungDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSamsungDlg)
	//}}AFX_DATA_INIT
	m_hFont = NULL;
	m_nFistNameBoundary = 0;
	m_nLastNameBoundary = 0;
}

CSamsungDlg::~CSamsungDlg()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}
void CSamsungDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSamsungDlg)
	DDX_Control(pDX, IDOK, m_rbnOk);
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDC_SC_WORK, m_scWork);
	DDX_Control(pDX, IDC_SC_OTHER, m_scOther);
	DDX_Control(pDX, IDC_SC_MOBILE, m_scMobile);
	DDX_Control(pDX, IDC_SC_MAIL, m_scMail);
	DDX_Control(pDX, IDC_SC_LAST_NAME, m_scLastName);
	DDX_Control(pDX, IDC_SC_HOME_TEL, m_scHomeTel);
	DDX_Control(pDX, IDC_SC_FIRST_NAME, m_scFirstName);
	DDX_Control(pDX, IDC_SC_FAX, m_scFax);
	DDX_Control(pDX, IDC_SC_POSITION, m_scPosition);
	DDX_Control(pDX, IDC_ED_WORK, m_edWork);
	DDX_Control(pDX, IDC_ED_OTHER, m_edOther);
	DDX_Control(pDX, IDC_ED_MOBILE, m_edMobile);
	DDX_Control(pDX, IDC_ED_MAIL, m_edMail);
	DDX_Control(pDX, IDC_ED_LAST_NAME, m_edLastName);
	DDX_Control(pDX, IDC_ED_HOME, m_edHome);
	DDX_Control(pDX, IDC_ED_FIRST_NAME, m_edFirstName);
	DDX_Control(pDX, IDC_ED_FAX, m_edFax);
	DDX_Control(pDX, IDC_RD_SIM, m_rdSIM);
	DDX_Control(pDX, IDC_RD_MEMORY, m_rdME);
	DDX_Control(pDX, IDC_SC_LINE, m_scLine);
	DDX_Control(pDX, IDC_ED_MEMO, m_edMemo);
	DDX_Control(pDX, IDC_SC_MEMO, m_scMemo);
	DDX_Control(pDX, IDC_SC_GROUP, m_scGroup);
	DDX_Control(pDX, IDC_ED_GROUP, m_cbGroup);
	DDX_Control(pDX, IDC_SC_BIRTHDAY, m_scBirthday);
	DDX_Control(pDX, IDC_DP_BIRTHDAY, m_dpBirthday);
	DDX_Control(pDX, IDC_CH_BIRTHDAY, m_chBirthday);
	DDX_Control(pDX, IDC_SC_PAGER,m_scPager);
	DDX_Control(pDX, IDC_SC_POSTCODE,m_scPostCode);
	DDX_Control(pDX, IDC_SC_NOTES,m_scNotes);
	DDX_Control(pDX, IDC_ED_PAGER,m_edPager);
	DDX_Control(pDX, IDC_ED_POSTCODE,m_edPostCode);
	DDX_Control(pDX, IDC_ED_NOTES,m_edNotes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSamsungDlg, CBaseDlg)
	//{{AFX_MSG_MAP(CSamsungDlg)
	ON_BN_CLICKED(IDC_RD_MEMORY, OnRdMobile)
	ON_BN_CLICKED(IDC_RD_SIM, OnRdSim)
	ON_EN_CHANGE(IDC_ED_FIRST_NAME, OnChangeEdFirstName)
	ON_EN_CHANGE(IDC_ED_LAST_NAME, OnChangeEdLastName)
	ON_BN_CLICKED(IDC_CH_BIRTHDAY, OnChBirthday)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSamsungDlg message handlers
BOOL CSamsungDlg::xCheckData()
{
	CString str;
	TCHAR szText[MAX_PATH];

	int nPos;
	m_edMail.GetWindowText(str);	//Type
	if(str.GetLength()>0)
	{	//e-mail
		nPos = str.Find('@');
		if(nPos==-1 || nPos==0 || nPos==str.GetLength()-1)	
		{
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
void CSamsungDlg::OnOK() 
{
	TCHAR szMobile[MAX_PATH],szFax[MAX_PATH],szHome[MAX_PATH],szOther[MAX_PATH],szWork[MAX_PATH];
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
	TCHAR szNameLN[MAX_PATH];
			 if (m_edFirstName.GetWindowText(szNameLN,MAX_PATH)==0)
			 {
				 if(al_GetSettingString(_T("public"),_T("IDS_ERR_NONAME"),theApp.m_szRes,szMobile))
					 AfxMessageBox(szMobile);
				 return;
				 
			 }

	if( !xCheckData() )
		return;
	
	DEQPHONEDATA deqData;
	CPhoneData data;
	TCHAR szFN[MAX_PATH],szLN[MAX_PATH],szTxt[MAX_PATH];
	//name or first name
	if(m_edFirstName.GetWindowText(szFN,MAX_PATH) > 0 )//&& m_iState != 0)
	{
		data.SetType(PBK_Text_Name);
		data.SetText(szFN);
		deqData.push_back(data);
	}
	//last name
	if(m_edLastName.GetWindowText(szLN,MAX_PATH) > 0 )//&& m_iState != 0)
	{
		data.SetType(PBK_Text_LastName);
		data.SetText(szLN);
		deqData.push_back(data);
	}
	//combine the first name and last name
/*	if(m_iState == 0)
	{
		TCHAR szName[MAX_PATH];
		strcpy(szName,szFN);
		_tcscat(szName," ");
		_tcscat(szName,szLN);
		data.SetType(PBK_Text_Name);
		data.SetText(szName);
		deqData.push_back(data);
	}*/
	//mobile 
	if(m_edMobile.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Number_Mobile);
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
	if (m_edPager.GetWindowText(szTxt,MAX_PATH)>0)
	{
		data.SetType(PBK_Number_Pager);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if (m_edPostCode.GetWindowText(szTxt,MAX_PATH)>0)
	{
		data.SetType(PBK_Text_Postal);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if (m_edNotes.GetWindowText(szTxt,MAX_PATH)>0)
	{
		data.SetType(PBK_Text_Note);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	//E-mail
	if(m_edMail.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_Email);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_edMemo.GetWindowText(szTxt,MAX_PATH) > 0){
		data.SetType(PBK_Text_Company);
		data.SetText(szTxt);
		deqData.push_back(data);
	}
	if(m_scBirthday.GetStyle() & WS_VISIBLE)
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
	//Caller Group
/*	CString group;
	data.SetType(PBK_Caller_Group);
	switch (m_cbGroup.GetCurSel())
	{
		case 0: group = "0"; break;
		case 1: group = "1"; break;
		case 2: group = "2"; break;
		case 3: group = "3"; break;
	}
	data.SetText(group);
	deqData.push_back(data);
*/
	data.SetType(PBK_Caller_Group);			
	data.SetNumber(m_cbGroup.GetCurSel());
	deqData.push_back(data);
/*
	data.SetType(PBK_RingontID);
	data.SetType(PBK_Text_Picture);
	data.SetType(PBK_Text_Ring);
		
		data.SetType(PBK_PictureID);
*/
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

void CSamsungDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CBaseDlg::OnCancel();
}

void CSamsungDlg::OnRdMobile() 
{
	m_rdME.SetCheck(TRUE);
	m_rdSIM.SetCheck(FALSE);

	xEnableControl(ENABLE_ME);
	//m_dpBirthday.EnableWindow(TRUE);
	m_chBirthday.EnableWindow(TRUE);
	SetIsME(true);

}

void CSamsungDlg::OnRdSim() 
{
	m_rdME.SetCheck(FALSE);
	m_rdSIM.SetCheck(TRUE);

	xEnableControl(ENABLE_SIM);
	if (m_chBirthday.GetCheck())
	{
	m_chBirthday.SetCheck(FALSE);
	}
	
    m_dpBirthday.EnableWindow(FALSE);
	m_chBirthday.EnableWindow(FALSE);
	SetIsME(false);

}

BOOL CSamsungDlg::OnInitDialog() 
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

//	string sProfile(theApp.m_szSkin);
//	sProfile += string("Organize\\SamsungDlg.ini");
//////////
	Tstring sProfile(theApp.m_szSkin);
	
// 	TCHAR szProfile[MAX_PATH];
// 	wsprintf(szProfile,_T("%sOrganize\\Samsung %s.ini"),theApp.m_szSkin,theApp.m_szPhone);
// 	if(al_IsFileExist(szProfile))
// 	{
// 		TCHAR szReference[MAX_PATH];
// 		if(al_GetSettingString(_T("Dlg"),_T("ReferenceTO"),szProfile,szReference))
// 		{
// 			wsprintf(szProfile,_T("%sOrganize\\%s"), theApp.m_szSkin,szReference);
// 		}
// 		sProfile = Tstring(szProfile);
// 	}
// 	else
// 	{
		sProfile += Tstring(_T("Organize\\M5511Dlg.ini"));
	/*}*/

/////////	
	al_GetSettingString(_T("public"),_T("IDS_FIRSTNAME"),theApp.m_szRes,m_szFirstName);
	al_GetSettingString(_T("public"),_T("IDS_NAME2"),theApp.m_szRes,m_szName);

	//Load static
	CStaticEx	*pStaticEx[15] = {&m_scFirstName,&m_scLastName,&m_scMobile,&m_scHomeTel,
		&m_scWork,&m_scFax,&m_scOther,&m_scMail, &m_scPosition, &m_scGroup, &m_scMemo,	&m_scBirthday,&m_scPager,&m_scPostCode,&m_scNotes};
	TCHAR szScSec[15][32] = {_T("firstname_sc"),_T("lastname_sc"),_T("mobile_sc"),_T("home_sc"),
		_T("work_sc"),_T("fax_sc"),_T("other_sc"),_T("mail_sc"), _T("position_sc"), _T("group_sc"), _T("memo_sc"),_T("birthday_sc"),_T("pager_sc"),_T("postcode_sc"),_T("notes_sc")};
	TCHAR szScID[15][32] = {_T("IDS_NAME"),_T("IDS_LASTNAME"),_T("IDS_MOBILENO"),_T("IDS_HOMENO"),
		_T("IDS_SE_WORK"),_T("IDS_SE_FAX"),_T("IDS_SE_OTHER"),_T("IDS_EMAIL"), _T("IDS_A_STORAGE"), _T("IDS_A_GROUP"), _T("IDS_ADDRESS"),_T("IDS_N_BIRTHDAY"),_T("IDS_BBCALL"),_T("IDS_S_POSTCODE"),_T("IDS_S_NOTE")};
	for(int i = 0 ; i < 15 ; i ++){
		GetStaticFromSetting2(pStaticEx[i],szScSec[i],szScID[i],const_cast<TCHAR*>(sProfile.c_str()));
	//	pStaticEx[i]->ShowWindow(SW_SHOW);
	}
		
	//set other control position and size
	CWnd *pWnd[15] = {&m_edFirstName,&m_edLastName,&m_edMobile,&m_edHome,
		&m_edWork,&m_edFax,&m_edOther,&m_edMail,&m_scLine, &m_cbGroup, &m_edMemo,&m_dpBirthday,&m_edPager,&m_edPostCode,&m_edNotes};
	TCHAR szWndSec[15][32] = {_T("firstname_ed"),_T("lastname_ed"),_T("mobile_ed"),_T("home_ed"),
		_T("work_ed"),_T("fax_ed"),_T("other_ed"),_T("mail_ed"),_T("line"),_T("group_ed"),_T("memo_ed"),_T("birthday_dp"),_T("pager_ed"),_T("postcode_ed"),_T("notes_ed")};
	for(i = 0 ; i < 15 ; i ++){
		CRect rect;
		if(al_GetSettingRect(szWndSec[i],_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect))
		{
			pWnd[i]->MoveWindow(rect);
			pWnd[i]->ShowWindow(SW_SHOW);
		}
		else
			pWnd[i]->ShowWindow(SW_HIDE);

		if(pFont)pWnd[i]->SetFont(pFont);
	}
/*
	if(m_iState == 1){
		TCHAR szName[MAX_PATH];
		if(al_GetSettingString(_T("public"),_T("IDS_A_NAME"),theApp.m_szRes,szName))
			m_scFirstName.SetWindowText(szName);
		m_scLastName.ShowWindow(SW_HIDE);
		m_edLastName.ShowWindow(SW_HIDE);
	}*/
	//load button 
	CRescaleButton	*pButton[2] = {&m_rbnOk,&m_rbnCancel};
	TCHAR szBnSec[2][32] = {_T("ok"),_T("cancel")};
	TCHAR szBnID[2][32] = {_T("IDS_N_OK"),_T("IDS_CANCEL")};
	for(i = 0 ; i < 2 ; i ++)
		GetButtonFromSetting(pButton[i],szBnSec[i],szBnID[i],0,const_cast<TCHAR*>(sProfile.c_str()));
	//load checkbox
	CCheckEx2	*pCheck[1] = {&m_chBirthday};
	TCHAR szChSec[1][32] = {_T("birthday_ch")};
	for(i = 0 ; i < 1 ; i ++)
	{
		GetCheckFromSetting2(pCheck[i],szChSec[i],0,const_cast<TCHAR*>(sProfile.c_str()));
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
		
	m_edMobile.SetNumFlag(true);
	m_edHome.SetNumFlag(true);
	m_edWork.SetNumFlag(true);
	m_edFax.SetNumFlag(true);
	m_edOther.SetNumFlag(true);
	m_edPager.SetNumFlag(true);
	m_edPostCode.SetNumFlag(true);
//bobby add cbx ctrl

	TCHAR szGroup[MAX_PATH];
	if(al_GetSettingString(_T("public"),_T("IDS_GROUP_NOGROUP"),theApp.m_szRes,szGroup))
		m_cbGroup.InsertString(0,szGroup);
	
	if(al_GetSettingString(_T("public"),_T("IDS_SAMSUNG_GROUP_FAMILY"),theApp.m_szRes,szGroup))
		m_cbGroup.InsertString(1,szGroup);
	else
		m_cbGroup.InsertString(1,_T("Family"));

	if(al_GetSettingString(_T("public"),_T("IDS_SAMSUNG_GROUP_FRIENDS"),theApp.m_szRes,szGroup))
		m_cbGroup.InsertString(2,szGroup);
	else
		m_cbGroup.InsertString(2,_T("Friends"));

	if(al_GetSettingString(_T("public"),_T("IDS_GROUP_COLLEAGUE"),theApp.m_szRes,szGroup))
		m_cbGroup.InsertString(3,szGroup);
	else
		m_cbGroup.InsertString(3,_T("Colleague"));
		if(al_GetSettingString(_T("public"),_T("IDS_GROUP_SOCIETY"),theApp.m_szRes,szGroup))
		m_cbGroup.InsertString(4,szGroup);
	else
		m_cbGroup.InsertString(4,_T("Society"));

		if(al_GetSettingString(_T("public"),_T("IDS_PRIVATE"),theApp.m_szRes,szGroup))
		m_cbGroup.InsertString(5,szGroup);
	else
		m_cbGroup.InsertString(5,_T("Private"));


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

	m_edFirstName.GetWindowRect(&rc);
	ScreenToClient(&rc);
	
	rcBtn1.OffsetRect(rc.right - rcBtn1.right  ,0);
	m_rbnCancel.MoveWindow(rcBtn1);

	rcBtn2.OffsetRect(rcBtn1.left - 20 - rcBtn2.right ,0);
	m_rbnOk.MoveWindow(rcBtn2);

	xSetRadioBtn();
	//set title 
	TCHAR szText[MAX_PATH];
	BOOL bRet = FALSE;
	if(m_iMode == 0)
	{	//new
		 m_dpBirthday.EnableWindow(FALSE);//080527libaoliu
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
	
	xSetDataToDlgCtrl();
	if(m_iMode == ENABLE_VIEW)
	{
		if(m_iIsME)
			m_scFirstName.SetWindowText(m_szFirstName);
		else
			m_scFirstName.SetWindowText(m_szName);

	}
if(strcmp(m_pData->GetStorageType(),SIM_NAME) == 0)
{
	m_dpBirthday.EnableWindow(FALSE);
	m_chBirthday.EnableWindow(FALSE);

}
    m_scFirstName.SetWindowText(m_szName);
	m_edFirstName.SetFocus();
	xSetSupportColumnLength();
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CSamsungDlg::xSetDataToDlgCtrl()
{
	m_cbGroup.SetCurSel(0);
	if(m_pData)
	{
		//get data deq
		DEQPHONEDATA *pdeqData; 
		pdeqData = m_pData->GetPhoneData();
		for(DEQPHONEDATA::iterator iter = pdeqData->begin() ; iter != pdeqData->end() ; iter ++)
		{
			if((*iter).GetType() == PBK_Text_Name)
			{		//full name
		//		if(m_iState == 1 || !m_iIsME )
					m_edFirstName.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_FirstName){	//first name
			//	if(m_iState == 0)
					m_edFirstName.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_LastName){	//last name
			//	if(m_iState == 0)
					m_edLastName.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Mobile && m_iIsME){	//mobile no
				m_edMobile.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_General){//  && !m_iIsME){	//mobile no
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
			else if((*iter).GetType() == PBK_Number_Pager){
			m_edPager.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Postal){
			m_edPostCode.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Note){
			m_edNotes.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Email){	//email
				m_edMail.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Company)
			{	//Memo
				m_edMemo.SetWindowText((*iter).GetText());
					
			}
			else if((*iter).GetType() == PBK_Date)
			{		//birthday
			    
				COleDateTime tm;
				(*iter).GetTime(tm);
				m_dpBirthday.SetTime(tm);
				m_chBirthday.SetCheck(1);
				if(m_iMode == 1)
					m_dpBirthday.EnableWindow(TRUE);
			
			}	
			else if((*iter).GetType() == PBK_Caller_Group)
			{
				if(strcmp(m_pData->GetStorageType(),SIM_NAME) == 0)
					m_cbGroup.SetCurSel(0);
				else
					m_cbGroup.SetCurSel((*iter).GetNumber());
			}
			else{	//others
				m_deqDetainData.push_back((*iter));
			}
		}
	}

}

CSamsungDlg::xEnableControl(int iState)
{
	CWnd *pWnd[15] = {&m_edFirstName,&m_edLastName,&m_edMobile,&m_edHome,&m_edWork,
		&m_edFax,&m_edOther,&m_edMail, &m_cbGroup, &m_edMemo,&m_dpBirthday,&m_chBirthday,&m_edPager,&m_edPostCode,&m_edNotes};
	if(iState == ENABLE_SIM)
	{
		pWnd[1]->EnableWindow(FALSE);	//bobby add
		for(int i = 3 ; i < 15 ; i ++)
			pWnd[i]->EnableWindow(FALSE);

		m_scFirstName.SetWindowText(m_szName);
	}
	else if(iState == ENABLE_ME){
		for(int i = 0 ; i < 15 ; i ++){
			pWnd[i]->EnableWindow(TRUE);
		}
		m_scFirstName.SetWindowText(m_szFirstName);
	}
	else if(iState == ENABLE_VIEW)
	{
		for(int i = 0 ; i < 15 ; i ++)
		{
			pWnd[i]->EnableWindow(FALSE);
		}
		m_rdSIM.EnableWindow(FALSE);
		m_rdME.EnableWindow(FALSE);
	}
	OnChBirthday();

}
void CSamsungDlg::xSetSupportColumnLength()
{
	DEQPHBOUNDARY *pBoundary = m_se.GetPhBoundary();
	
	CEdit *pEd[8] = {&m_edLastName,&m_edFirstName,&m_edWork,&m_edHome,&m_edMobile,&m_edOther,&m_edFax,&m_edMail};

	for(DEQPHBOUNDARY::iterator iter = pBoundary->begin() ; iter != pBoundary->end() ; iter ++)
	{
		switch((*iter).GetType())
		{
		case PBK_Text_Name:
			m_nFistNameBoundary = (*iter).GetBoundary();
			break;
		case PBK_Text_LastName:
			m_nLastNameBoundary = (*iter).GetBoundary();
			break;
		case PBK_Number_Work:
			m_edWork.SetLimitText((*iter).GetBoundary());
			break;
		case PBK_Number_Home:
			m_edHome.SetLimitText((*iter).GetBoundary());
			break;
		case PBK_Number_Mobile:
			m_edMobile.SetLimitText((*iter).GetBoundary());
			m_edOther.SetLimitText((*iter).GetBoundary());
			break;
		case PBK_Number_Fax:
			m_edFax.SetLimitText((*iter).GetBoundary());
			break;
		case PBK_Text_Email:
			m_edMail.SetLimitText((*iter).GetBoundary());
			break;
		case PBK_Text_Company:
			m_edMemo.SetLimitText((*iter).GetBoundary());
			break;
		case PBK_Text_Note:
			m_edNotes.SetLimitText((*iter).GetBoundary());
			break;
		case PBK_Text_Postal:
			m_edPostCode.SetLimitText((*iter).GetBoundary());
			break;
		case PBK_Number_Pager:
			m_edPager.SetLimitText((*iter).GetBoundary());
			break;


			
		}
	}
}

void CSamsungDlg::OnChangeEdFirstName() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CBaseDlg::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	if(m_nFistNameBoundary == 0) return;

	CString strTxT;
	m_edFirstName.GetWindowText(strTxT);
	if(strTxT.GetLength() <= 0 ) return;
#ifdef _UNICODE
	if(m_iIsME)
	{

	
	
	if(strTxT.GetLength() > m_nFistNameBoundary)
	{
		m_edFirstName.SetWindowText(strTxT.Left(m_nFistNameBoundary));
	}
	m_edFirstName.SetLimitText(m_nFistNameBoundary);
	}
	else
	{
		if(strTxT.GetLength() > m_nLastNameBoundary)
	{
		m_edFirstName.SetWindowText(strTxT.Left(m_nLastNameBoundary));
	}
	m_edFirstName.SetLimitText(m_nLastNameBoundary);

	}
#else

	int nLength = (strTxT.GetLength()+1) *2;
	WCHAR *pwsTxt = new WCHAR[nLength] ;
	::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, (LPSTR)(LPCSTR)strTxT, -1,
				pwsTxt, nLength);
	//count text length
	if(wcslen(pwsTxt) > m_nFistNameBoundary)
	{
		//if string longer then limit,cut the string
		WCHAR *pwsFromat = new WCHAR[nLength];
		memset(pwsFromat,0,nLength);
		wcsncpy(pwsFromat,pwsTxt,m_nFistNameBoundary);

		::WideCharToMultiByte(CP_ACP,0,pwsFromat,-1,
					(LPSTR)(LPCSTR)strTxT,nLength,NULL,NULL);
		m_edFirstName.SetWindowText(strTxT);
		//set text limit
		m_edFirstName.SetLimitText(strTxT.GetLength());
		delete pwsFromat;
	}
	else if(wcslen(pwsTxt) == m_nFistNameBoundary)
	{
		m_edFirstName.SetLimitText(strTxT.GetLength());
	}
	else
		//set text limit
		m_edFirstName.SetLimitText(m_nFistNameBoundary * 2);
	delete pwsTxt;
#endif

}

void CSamsungDlg::OnChangeEdLastName() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CBaseDlg::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here

	
	if(m_nLastNameBoundary == 0) return;
	CString strTxT;
	m_edLastName.GetWindowText(strTxT);

	if(strTxT.GetLength() <= 0 ) return;
#ifdef _UNICODE
	if(strTxT.GetLength() > m_nLastNameBoundary)
	{
		m_edLastName.SetWindowText(strTxT.Left(m_nLastNameBoundary));
	}
	m_edLastName.SetLimitText(m_nLastNameBoundary);
#else

	int nLength = (strTxT.GetLength()+1) *2;
	WCHAR *pwsTxt = new WCHAR[nLength] ;
	::MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, (LPSTR)(LPCSTR)strTxT, -1,
				pwsTxt, nLength);
	//count text length
	if(wcslen(pwsTxt) > m_nLastNameBoundary)
	{
		//if string longer then limit,cut the string
		WCHAR *pwsFromat = new WCHAR[nLength];
		memset(pwsFromat,0,nLength);
		wcsncpy(pwsFromat,pwsTxt,m_nLastNameBoundary);

		::WideCharToMultiByte(CP_ACP,0,pwsFromat,-1,
					(LPSTR)(LPCSTR)strTxT,nLength,NULL,NULL);
		m_edLastName.SetWindowText(strTxT);
		//set text limit
		m_edLastName.SetLimitText(strTxT.GetLength());
		delete pwsFromat;
	}
	else if(wcslen(pwsTxt) == m_nLastNameBoundary)
	{
		m_edLastName.SetLimitText(strTxT.GetLength());
	}
	else
		//set text limit
		m_edLastName.SetLimitText(m_nLastNameBoundary * 2);
	delete pwsTxt;
#endif
	
}
void CSamsungDlg::OnChBirthday() 
{
	// TODO: Add your control notification handler code here
// 	if(m_chBirthday.GetCheck() && m_iMode == 0)	
// 	{
// 		m_dpBirthday.EnableWindow(TRUE);
// 		m_dpBirthday.EnableWindow(TRUE);
// 
// 	}
// 	else
// 	{
// 		m_dpBirthday.EnableWindow(FALSE);
// 		m_dpBirthday.EnableWindow(FALSE);
// 
// 	}
	if( m_iMode == 0)	
	{
		if (m_chBirthday.GetCheck())
		{
			m_dpBirthday.EnableWindow(true);
		}
		else
		m_dpBirthday.EnableWindow(FALSE);
	

	}
	else if (m_iMode==3)
	{
		if (m_chBirthday.GetCheck())
		{
			m_dpBirthday.EnableWindow(true);
		}
		else
		m_dpBirthday.EnableWindow(FALSE);
	

		
	}
	else if (m_chBirthday.GetCheck() && m_iMode ==4)
	{
		m_dpBirthday.EnableWindow(TRUE);
	
	}
	else if (m_chBirthday.GetCheck() && m_iMode == 1)
	{
		m_dpBirthday.EnableWindow(TRUE);
	}
	else
	{
		m_dpBirthday.EnableWindow(FALSE);
		
	}
	
}
