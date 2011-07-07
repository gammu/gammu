// sonyerisontdlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "sonyerisontdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSonyEricssonTDlg dialog


CSonyEricssonTDlg::CSonyEricssonTDlg(CWnd* pParent /*=NULL*/)
	: CBaseDlg(CSonyEricssonTDlg::IDD, pParent),m_iState(0)
{
	//{{AFX_DATA_INIT(CSonyEricssonTDlg)
	//}}AFX_DATA_INIT
	m_hFont = NULL;
}
CSonyEricssonTDlg::~CSonyEricssonTDlg()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}

void CSonyEricssonTDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSonyEricssonTDlg)
	DDX_Control(pDX, IDOK, m_rbnOk);
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDC_SC_WORK, m_scWork);
	DDX_Control(pDX, IDC_SC_TITLE, m_scTitle);
	DDX_Control(pDX, IDC_SC_POSITION, m_scPosition);
	DDX_Control(pDX, IDC_SC_OTHER, m_scOther);
	DDX_Control(pDX, IDC_SC_MOBILE, m_scMobile);
	DDX_Control(pDX, IDC_SC_MAIL, m_scMail);
	DDX_Control(pDX, IDC_SC_LAST_NAME, m_scLastName);
	DDX_Control(pDX, IDC_SC_HOME_TEL, m_scHomeTel);
	DDX_Control(pDX, IDC_SC_FIRST_NAME, m_scFirstName);
	DDX_Control(pDX, IDC_SC_FAX, m_scFax);
	DDX_Control(pDX, IDC_SC_COMPANY, m_scCompany);
	DDX_Control(pDX, IDC_ED_WORK, m_edWork);
	DDX_Control(pDX, IDC_ED_TITLE, m_edTitle);
	DDX_Control(pDX, IDC_ED_OTHER, m_edOther);
	DDX_Control(pDX, IDC_ED_MOBILE, m_edMobile);
	DDX_Control(pDX, IDC_ED_MAIL, m_edMail);
	DDX_Control(pDX, IDC_ED_LAST_NAME, m_edLastName);
	DDX_Control(pDX, IDC_ED_HOME, m_edHome);
	DDX_Control(pDX, IDC_ED_FIRST_NAME, m_edFirstName);
	DDX_Control(pDX, IDC_ED_FAX, m_edFax);
	DDX_Control(pDX, IDC_ED_COMPANY, m_edCompany);
	DDX_Control(pDX, IDC_RD_SIM, m_rdSIM);
	DDX_Control(pDX, IDC_RD_MEMORY, m_rdME);
	DDX_Control(pDX, IDC_SC_LINE, m_scLine);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSonyEricssonTDlg, CBaseDlg)
	//{{AFX_MSG_MAP(CSonyEricssonTDlg)
	ON_BN_CLICKED(IDC_RD_MEMORY, OnRdMobile)
	ON_BN_CLICKED(IDC_RD_SIM, OnRdSim)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSonyEricssonTDlg message handlers

void CSonyEricssonTDlg::OnOK() 
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
	
	DEQPHONEDATA deqData;
	CPhoneData data;
	TCHAR szFN[MAX_PATH],szLN[MAX_PATH],szTxt[MAX_PATH];
	//name or first name
	if(m_edFirstName.GetWindowText(szFN,MAX_PATH) > 0){
		if(m_iState == 0){
			data.SetType(PBK_Text_FirstName);
		}
		else
			data.SetType(PBK_Text_Name);
		
		data.SetText(szFN);
		deqData.push_back(data);
	}
	//last name
	if(m_edLastName.GetWindowText(szLN,MAX_PATH) > 0 && m_iState == 0){
		data.SetType(PBK_Text_LastName);
		data.SetText(szLN);
		deqData.push_back(data);
	}
	//combine the first name and last name
/*	if(m_iState == 0){
		TCHAR szName[MAX_PATH];
		strcpy(szName,szLN);
		strcat(szName," ");
		strcat(szName,szFN);
		data.SetType(PBK_Text_Name);
		data.SetText(szName);
		deqData.push_back(data);
	}*/
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

void CSonyEricssonTDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CBaseDlg::OnCancel();
}

void CSonyEricssonTDlg::OnRdMobile() 
{
	m_rdME.SetCheck(TRUE);
	m_rdSIM.SetCheck(FALSE);
	xEnableControl(ENABLE_ME);
}

void CSonyEricssonTDlg::OnRdSim() 
{
	m_rdME.SetCheck(FALSE);
	m_rdSIM.SetCheck(TRUE);
	xEnableControl(ENABLE_SIM);
}

BOOL CSonyEricssonTDlg::OnInitDialog() 
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
	
	//get profile path
	if( _tcscmp(theApp.m_szPhone , _T("T610")) == 0 || _tcscmp(theApp.m_szPhone , _T("T630")) == 0
		|| _tcscmp(theApp.m_szPhone , _T("Z600")) == 0|| _tcscmp(theApp.m_szPhone , _T("T616")) == 0
		||_tcscmp(theApp.m_szPhone , _T("T637")) == 0||_tcscmp(theApp.m_szPhone , _T("T628")) == 0){
		sProfile += Tstring(_T("Organize\\SET610Dlg.ini"));
		m_iState = 1;
	}
	else{
		sProfile += Tstring(_T("Organize\\SET68Dlg.ini"));
		m_iState = 0;
	}
	
	//Load static
	CStaticEx	*pStaticEx[11] = {&m_scFirstName,&m_scLastName,&m_scMobile,&m_scHomeTel,
		&m_scWork,&m_scFax,&m_scOther,&m_scMail,&m_scTitle,&m_scCompany,&m_scPosition};
	TCHAR szScSec[11][32] = {_T("firstname_sc"),_T("lastname_sc"),_T("mobile_sc"),_T("home_sc"),
		_T("work_sc"),_T("fax_sc"),_T("other_sc"),_T("mail_sc"),_T("title_sc"),_T("company_sc"),_T("position_sc")};
	TCHAR szScID[11][32] = {_T("IDS_FIRSTNAME"),_T("IDS_LASTNAME"),_T("IDS_MOBILENO"),_T("IDS_HOMENO"),
		_T("IDS_SE_WORK"),_T("IDS_SE_FAX"),_T("IDS_SE_OTHER"),_T("IDS_EMAIL"),_T("IDS_HANDLE"),_T("IDS_COMPANY"),_T("IDS_A_STORAGE")};
	for(int i = 0 ; i < 11 ; i ++){
		GetStaticFromSetting(pStaticEx[i],szScSec[i],szScID[i],const_cast<TCHAR*>(sProfile.c_str()));
		pStaticEx[i]->ShowWindow(SW_SHOW);
	}
		

	//set other control position and size
	CWnd *pWnd[11] = {&m_edFirstName,&m_edLastName,&m_edMobile,&m_edHome,
		&m_edWork,&m_edFax,&m_edOther,&m_edMail,&m_edTitle,&m_edCompany,&m_scLine};
	TCHAR szWndSec[11][32] = {_T("firstname_ed"),_T("lastname_ed"),_T("mobile_ed"),_T("home_ed"),
		_T("work_ed"),_T("fax_ed"),_T("other_ed"),_T("mail_ed"),_T("title_ed"),_T("company_ed"),_T("line")};
	for(i = 0 ; i < 11 ; i ++){
		CRect rect;
		al_GetSettingRect(szWndSec[i],_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect);
		pWnd[i]->MoveWindow(rect);
		pWnd[i]->ShowWindow(SW_SHOW);
		if(pFont)pWnd[i]->SetFont(pFont);
	}

	if(m_iState == 1){
		TCHAR szName[MAX_PATH];
		if(al_GetSettingString(_T("public"),_T("IDS_A_NAME"),theApp.m_szRes,szName))
			m_scFirstName.SetWindowText(szName);
		m_scLastName.ShowWindow(SW_HIDE);
		m_edLastName.ShowWindow(SW_HIDE);
	}
	al_GetSettingString(_T("public"),_T("IDS_FIRSTNAME"),theApp.m_szRes,m_szFirstName);
	al_GetSettingString(_T("public"),_T("IDS_A_NAME"),theApp.m_szRes,m_szName);
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

	m_edFirstName.GetWindowRect(&rc);
	ScreenToClient(&rc);
	
	rcBtn1.OffsetRect(rc.right - rcBtn1.right  ,0);
	m_rbnCancel.MoveWindow(rcBtn1);

	rcBtn2.OffsetRect(rcBtn1.left - 20 - rcBtn2.right ,0);
	m_rbnOk.MoveWindow(rcBtn2);

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
	
	xSetDataToDlgCtrl();
	m_edFirstName.SetFocus();
	if(m_iMode == ENABLE_VIEW && m_iState != 1)
	{
		if(m_iIsME)
			m_scFirstName.SetWindowText(m_szFirstName);
		else
			m_scFirstName.SetWindowText(m_szName);

	}
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CSonyEricssonTDlg::xSetDataToDlgCtrl()
{
	CString strName;
	strName.Empty();
	if(m_pData){
		//get data deq
		DEQPHONEDATA *pdeqData; 
		pdeqData = m_pData->GetPhoneData();
		for(DEQPHONEDATA::iterator iter = pdeqData->begin() ; iter != pdeqData->end() ; iter ++){
			if((*iter).GetType() == PBK_Text_Name){		//full name
				if(m_iState == 1 || !m_iIsME )
					m_edFirstName.SetWindowText((*iter).GetText());
			
				strName.Format(_T("%s"),(char *)((*iter).GetText()));
			}
			else if((*iter).GetType() == PBK_Text_FirstName){	//first name
				if(m_iState == 0)
					m_edFirstName.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_LastName){	//last name
				if(m_iState == 0)
					m_edLastName.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_Mobile && m_iIsME){	//mobile no
				m_edMobile.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Number_General && !m_iIsME){	//mobile no
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
			else if((*iter).GetType() == PBK_Text_JobTitle){	//title
				m_edTitle.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Company){		//company
				m_edCompany.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Email){	//email
				m_edMail.SetWindowText((*iter).GetText());
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
				m_edFirstName.SetWindowText(strName);
			}
		}

	}

}

CSonyEricssonTDlg::xEnableControl(int iState)
{
	CWnd *pWnd[10] = {&m_edFirstName,&m_edLastName,&m_edMobile,&m_edHome,&m_edWork,
		&m_edFax,&m_edOther,&m_edMail,&m_edTitle,&m_edCompany};
	if(iState == ENABLE_SIM){
		for(int i = 3 ; i < 10 ; i ++)
			pWnd[i]->EnableWindow(FALSE);
		m_edLastName.EnableWindow(FALSE);
		if(m_iState != 1)
			m_scFirstName.SetWindowText(m_szName);
	}
	else if(iState == ENABLE_ME){
		for(int i = 0 ; i < 10 ; i ++){
			pWnd[i]->EnableWindow(TRUE);
		}
		if(m_iState != 1)
			m_scFirstName.SetWindowText(m_szFirstName);
	}
	else if(iState == ENABLE_VIEW){
		for(int i = 0 ; i < 10 ; i ++){
			pWnd[i]->EnableWindow(FALSE);
		}
		m_rdSIM.EnableWindow(FALSE);
		m_rdME.EnableWindow(FALSE);
	}
}
