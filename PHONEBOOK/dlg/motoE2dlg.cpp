// motodlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "motoe2dlg.h"
#include "AddressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMotoE2Dlg dialog


CMotoE2Dlg::CMotoE2Dlg(CWnd* pParent /*=NULL*/)
	: CBaseDlg(CMotoE2Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMotoE2Dlg)
	//}}AFX_DATA_INIT
	m_hFont = NULL;
	m_strSkinFile.Format(_T("%s"),_T("MotoE2Dlg.ini"));
	m_NameList.RemoveAll();

}
CMotoE2Dlg::~CMotoE2Dlg()
{
	RemoveNameList();
	if(m_hFont)
		::DeleteObject(m_hFont);
}
void CMotoE2Dlg::RemoveNameList()
{
	POSITION pos = m_NameList.GetHeadPosition();
	while(pos)
	{
		ContactInfo* pData = (ContactInfo*) m_NameList.GetNext(pos);
		if(pData)
		{
			delete pData;
			pData = NULL;
		}
	}
	m_NameList.RemoveAll();
}
void CMotoE2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMotoE2Dlg)
	DDX_Control(pDX, IDC_LINE, m_scLine);
	DDX_Control(pDX, IDC_DP_BIRTHDAY, m_dpBirthday);
	DDX_Control(pDX, IDC_DP_ANNIVERSARY, m_dpAnniversary);
	DDX_Control(pDX, IDC_SC_NOTES, m_scNote);
	DDX_Control(pDX, IDC_DOT, m_scDot);
	DDX_Control(pDX, IDC_DOT2, m_scDot2);
	DDX_Control(pDX, IDC_DOT3, m_scDot3);
	DDX_Control(pDX, IDC_DOT4, m_scDot4);
	DDX_Control(pDX, IDC_DOT5, m_scDot5);
	DDX_Control(pDX, IDC_DOT6, m_scDot6);
	DDX_Control(pDX, IDC_DOT7, m_scDot7);
	DDX_Control(pDX, IDC_DOT8, m_scDot8);
	DDX_Control(pDX, IDC_DOT9, m_scDot9);
	DDX_Control(pDX, IDC_SLASH, m_scSlash);
	DDX_Control(pDX, IDC_SC_COMPANYTITLE, m_scCompanyTitle);
	DDX_Control(pDX, IDC_SC_MANAGERNAME, m_scManagerName);
	DDX_Control(pDX, IDC_SC_ASSISTANTNAME, m_scAssistantName);
	DDX_Control(pDX, IDC_SC_SPOUSE, m_scSpous);
	DDX_Control(pDX, IDC_SC_CHILDREN, m_scChildren);
	DDX_Control(pDX, IDC_SC_URL, m_scURL);
	DDX_Control(pDX, IDC_SC_BIRTHDAY, m_scBirthday);
	DDX_Control(pDX, IDC_CH_BIRTHDAY, m_chBirthday);
	DDX_Control(pDX, IDC_SC_ANNIVERSARY, m_scAnniversary);
	DDX_Control(pDX, IDC_CH_ANNIVERSARY, m_chAnniversary);
	DDX_Control(pDX, IDC_ED_URL, m_edURL);
	DDX_Control(pDX, IDC_ED_NOTE, m_edNote);
	DDX_Control(pDX, IDC_ED_COMPANY, m_edCompany);
	DDX_Control(pDX, IDC_ED_TITLE, m_edTitle);
	DDX_Control(pDX, IDC_ED_MANAGERNAME, m_edManagerName);
	DDX_Control(pDX, IDC_ED_ASSISTANTNAME, m_edAssistantName);
	DDX_Control(pDX, IDC_ED_SPOUSE, m_edSpousName);
	DDX_Control(pDX, IDC_ED_CHILDREN, m_edChildernName);
	DDX_Control(pDX, IDC_ED_IMID, m_edIMID);
	DDX_Control(pDX, IDOK, m_rbnOK);
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDC_SC_PHONENO1, m_scPhone1);
	DDX_Control(pDX, IDC_SC_PHONENO2, m_scPhone2);
	DDX_Control(pDX, IDC_SC_PHONENO3, m_scPhone3);
	DDX_Control(pDX, IDC_SC_PHONENO4, m_scPhone4);
	DDX_Control(pDX, IDC_SC_ADDRESS1, m_scAddress1);
	DDX_Control(pDX, IDC_SC_ADDRESS2, m_scAddress2);
	DDX_Control(pDX, IDC_SC_EMAIL1, m_scEmail1);
	DDX_Control(pDX, IDC_SC_EMAIL2, m_scEmail2);
	DDX_Control(pDX, IDC_SC_STORAGE, m_scStorage);
	DDX_Control(pDX, IDC_SC_NAME, m_scName);
	DDX_Control(pDX, IDC_SC_IMID, m_scIMID);
	DDX_Control(pDX, IDC_ED_NAMES, m_edName);
	DDX_Control(pDX, IDC_ED_DATA1, m_edData1);
	DDX_Control(pDX, IDC_ED_DATA2, m_edData2);
	DDX_Control(pDX, IDC_ED_DATA3, m_edData3);
	DDX_Control(pDX, IDC_ED_DATA4, m_edData4);
	DDX_Control(pDX, IDC_ED_EMAIL1, m_edEmail1);
	DDX_Control(pDX, IDC_ED_EMAIL2, m_edEmail2);
	DDX_Control(pDX, IDC_ED_ADDRESS1, m_edAddress1);
	DDX_Control(pDX, IDC_ED_ADDRESS2, m_edAddress2);
	DDX_Control(pDX, IDC_CB_NUMBERTYPE1, m_cbType1);
	DDX_Control(pDX, IDC_CB_NUMBERTYPE2, m_cbType2);
	DDX_Control(pDX, IDC_CB_NUMBERTYPE3, m_cbType3);
	DDX_Control(pDX, IDC_CB_NUMBERTYPE4, m_cbType4);
	DDX_Control(pDX, IDC_CB_EMAILTYPE1, m_cbEmail1);
	DDX_Control(pDX, IDC_CB_EMAILTYPE2, m_cbEmail2);
	DDX_Control(pDX, IDC_CB_ADDRESSTYPE1, m_cbAddress1);
	DDX_Control(pDX, IDC_CB_ADDRESSTYPE2, m_cbAddress2);
	DDX_Control(pDX, IDC_CB_NAMETYPE, m_cbNameType);
	DDX_Control(pDX, IDC_RD_SIM, m_rdSIM);
	DDX_Control(pDX, IDC_RD_MEMORY, m_rdME);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMotoE2Dlg, CBaseDlg)
	//{{AFX_MSG_MAP(CMotoE2Dlg)
	ON_BN_CLICKED(IDC_RD_SIM, OnRdSim)
	ON_BN_CLICKED(IDC_RD_MEMORY, OnRdMobile)
	ON_BN_CLICKED(IDC_CH_BIRTHDAY, OnChBirthday)
	ON_BN_CLICKED(IDC_CH_ANNIVERSARY, OnChAnniversary)
	ON_EN_SETFOCUS(IDC_ED_ADDRESS1, OnSetfocusAddressEdit1)
	ON_EN_SETFOCUS(IDC_ED_ADDRESS2, OnSetfocusAddressEdit2)

	ON_CBN_SELCHANGE(IDC_CB_NAMETYPE, OnSelchangeNameCombo)
	ON_CBN_DROPDOWN(IDC_CB_NAMETYPE, OnDropdownNameCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMotoE2Dlg message handlers

BOOL CMotoE2Dlg::OnInitDialog() 
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
	
	Tstring sSkin(theApp.m_szSkin);
	Tstring sProfile = sSkin + Tstring(_T("Organize\\"))+Tstring(m_strSkinFile);

//	al_GetSettingString(_T("public"),_T("IDS_N_FIRSTNAME"),theApp.m_szRes,m_szFirstName);
//	al_GetSettingString(_T("public"),_T("IDS_NAME"),theApp.m_szRes,m_szName);
	//Load static
	CStaticEx	*pStaticEx[3] = {&m_scName,&m_scIMID,&m_scStorage};
	TCHAR szScSec[3][32] = {_T("name_sc"),_T("IMID_sc"),_T("storage_sc")};
	TCHAR szScID[3][32] = {_T("IDS_NAME"),_T("IDS_IMID"),_T("IDS_STORAGE")};
	for(int i = 0 ; i < 3 ; i ++)
		GetStaticFromSetting(pStaticEx[i],szScSec[i],szScID[i],const_cast<TCHAR*>(sProfile.c_str()));

	CStaticEx	*pStaticPhoneNoEx[4] = {&m_scPhone1,&m_scPhone2,&m_scPhone3,&m_scPhone4};
	TCHAR szPhoneNoSec[4][32] = {_T("phoneNo1_sc"),_T("phoneNo2_sc"),_T("phoneNo3_sc"),_T("phoneNo4_sc")};
	for (i = 0  ; i< 4 ;i++)
	{
		TCHAR szTempText[MAX_PATH];
		TCHAR szText[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_TEL_NO"),theApp.m_szRes,szText);
		wsprintf(szTempText,_T("%s %d"),szText,i+1);
		GetStaticFromSettingWithText(pStaticPhoneNoEx[i],szPhoneNoSec[i],szTempText,const_cast<TCHAR*>(sProfile.c_str()));
	}
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
	CStaticEx	*pStaticEx2[19] = {&m_scBirthday,&m_scAnniversary,&m_scNote,&m_scCompanyTitle,&m_scURL,
		&m_scManagerName,&m_scAssistantName,&m_scSpous,&m_scChildren,
		&m_scDot,&m_scDot2,&m_scDot3,&m_scDot4,&m_scDot5,&m_scDot6,&m_scDot7,&m_scDot8,&m_scDot9,&m_scSlash};
	TCHAR szScSec2[19][32] = {_T("birthday_sc"),_T("anniversary_sc"),_T("note_sc"),_T("companytitle_sc"),_T("url_sc"),
		_T("Managername_sc"),_T("assistantname_sc"),_T("spous_sc"),_T("children_sc"),
		_T("dot_sc"),_T("dot2_sc"),_T("dot3_sc"),_T("dot4_sc"),_T("dot5_sc"),_T("dot6_sc"),_T("dot7_sc"),_T("dot8_sc"),_T("dot9_sc"),_T("slash_sc")};
	TCHAR szScID2[19][32] = {_T("IDS_N_BIRTHDAY"),_T("IDS_N_ANNIVERSARY"),_T("IDS_N_NOTE"),_T("IDS_N_COMPANYTITLE"),_T("IDS_N_URL"),
		_T("IDS_N_MANAGERNAME"),_T("IDS_N_ASSISTANTNAME"),_T("IDS_N_SPOUSE"),_T("IDS_N_CHILDREN"),
		_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_SLASH")};

	for(i = 0 ; i < 19 ; i ++)
		GetStaticFromSetting2(pStaticEx2[i],szScSec2[i],szScID2[i],const_cast<TCHAR*>(sProfile.c_str()));

	CStaticEx	*pStaticEmailEx[2] = {&m_scEmail1,&m_scEmail2};
	TCHAR szEmailSec[2][32] = {_T("email1_sc"),_T("email2_sc")};
	for (i = 0  ; i< 2 ;i++)
	{
		TCHAR szTempText[MAX_PATH];
		TCHAR szText[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_N_EMAIL"),theApp.m_szRes,szText);
		wsprintf(szTempText,_T("%s %d"),szText,i+1);
		GetStaticFromSettingWithText(pStaticEmailEx[i],szEmailSec[i],szTempText,const_cast<TCHAR*>(sProfile.c_str()));
	}

	CStaticEx	*pStaticAddressEx[2] = {&m_scAddress1,&m_scAddress2};
	TCHAR szAddressSec[2][32] = {_T("address1_sc"),_T("address2_sc")};
	for (i = 0  ; i< 2 ;i++)
	{
		TCHAR szTempText[MAX_PATH];
		TCHAR szText[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_ADDRESS"),theApp.m_szRes,szText);
		wsprintf(szTempText,_T("%s %d"),szText,i+1);
		GetStaticFromSettingWithText(pStaticAddressEx[i],szAddressSec[i],szTempText,const_cast<TCHAR*>(sProfile.c_str()));
	}
	CWnd *pEDWnd[11] = {&m_edURL,&m_edNote,&m_dpBirthday,&m_scLine,&m_dpAnniversary,&m_edCompany,&m_edTitle,
		&m_edManagerName,&m_edAssistantName,&m_edSpousName,&m_edChildernName};
	TCHAR szEDWndSec[11][32] = {_T("url_ed"),_T("note_ed"),_T("birthday_dp"),_T("line"),_T("Anniversary_dp"),_T("company_ed"),_T("title_ed"),
		_T("Manager_ed"),_T("assistant_ed"),_T("Spouse_ed"),_T("children_ed")};
	for(i = 0 ; i < 11 ; i ++){
		CRect rect;
		if(	al_GetSettingRect(szEDWndSec[i],_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect))
			pEDWnd[i]->MoveWindow(rect);
		else
			pEDWnd[i]->ShowWindow(SW_HIDE);

		if(pFont) pEDWnd[i]->SetFont(pFont);
	}
	CCheckEx2	*pCheck[2] = {&m_chBirthday,&m_chAnniversary};
	TCHAR szChSec[2][32] = {_T("birthday_ch"),_T("anniversary_ch")};
	for(i = 0 ; i < 2 ; i ++){
		GetCheckFromSetting2(pCheck[i],szChSec[i],0,const_cast<TCHAR*>(sProfile.c_str()));
		pCheck[i]->SetCheck(0);
	}

	//new Control ******************************************* ----

	//set other control position and size
	CWnd *pWnd[19] = {&m_edName,&m_cbType1,&m_cbType2,&m_cbType3,&m_cbType4,
		&m_edData1,&m_edData2,&m_edData3,&m_edData4,
		&m_cbEmail1,&m_cbEmail2,&m_edEmail1,&m_edEmail2,&m_cbAddress1,&m_cbAddress2,&m_edAddress1,&m_edAddress2,&m_edIMID,&m_cbNameType};
	TCHAR szWndSec[19][32] = {_T("name_ed"),_T("type1_cb"),_T("type2_cb"),_T("type3_cb"),_T("type4_cb"),
		_T("data1_ed"),_T("data2_ed"),_T("data3_ed"),_T("data4_ed"),
		_T("email1_cb"),_T("email2_cb"),_T("email1_ed"),_T("email2_ed"),_T("address1_cb"),_T("address2_cb"),_T("address1_ed"),_T("address2_ed"),_T("IMID_ed"),_T("nametype_cb")};
	for(i = 0 ; i < 19 ; i ++){
		CRect rect;
		al_GetSettingRect(szWndSec[i],_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect);
		pWnd[i]->MoveWindow(rect);
		if(pFont) pWnd[i]->SetFont(pFont);
	}

	m_edData1.SetNumFlag(true);
	m_edData2.SetNumFlag(true);
	m_edData3.SetNumFlag(true);
	m_edData4.SetNumFlag(true);
	m_edEmail1.SetNumFlag(false);
	m_edEmail2.SetNumFlag(false);
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

//	CRect rcLine,rcClient;
//	GetClientRect(&rcClient);
//	m_scLine.GetWindowRect(&rcLine);
//	ScreenToClient(&rcLine);
//	rcLine.left = rcClient.left;
//	rcLine.right = rcClient.right;
//	int nW = rcLine.Height();
//	rcLine.top = rcClient.bottom - rcBtn2.Height() -20;
//	rcLine.bottom = rcLine.top+nW;
//	m_scLine.MoveWindow(rcLine);


	if(rcBtn1.Width() > rcBtn2.Width())
		rcBtn2.right = rcBtn2.left + rcBtn1.Width();
	else
		rcBtn1.right = rcBtn1.left + rcBtn2.Width();

/*	if(m_edStreet1.GetStyle() & WS_VISIBLE)
	{
		m_edStreet1.GetWindowRect(&rc);
	}
	else*/
		m_scLine.GetWindowRect(&rc);
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
//	if(m_iIsME)
//		m_scFirstName.SetWindowText(m_szFirstName);
//	else
//		m_scFirstName.SetWindowText(m_szName);
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMotoE2Dlg::OnRdSim() 
{
	m_rdME.SetCheck(FALSE);
	m_rdSIM.SetCheck(TRUE);
	m_cbType1.SetCurSel(0);
	m_cbType1.EnableWindow(FALSE);
	xEnableControl(ENABLE_SIM);
}

void CMotoE2Dlg::OnRdMobile() 
{
	m_rdME.SetCheck(TRUE);
	m_rdSIM.SetCheck(FALSE);
	m_cbType1.EnableWindow(TRUE);
	xEnableControl(ENABLE_ME);
}


void CMotoE2Dlg::OnOK() 
{
	TCHAR szTxt[MAX_PATH],szTxt1[MAX_PATH],szTxt2[MAX_PATH],szTxt3[MAX_PATH],szTxt4[MAX_PATH];
	m_edData1.GetWindowText(szTxt1,MAX_PATH);
	m_edData2.GetWindowText(szTxt2,MAX_PATH);
	m_edData3.GetWindowText(szTxt3,MAX_PATH);
	m_edData4.GetWindowText(szTxt4,MAX_PATH);
	int iLength = _tcslen(szTxt1)+_tcslen(szTxt2)+_tcslen(szTxt3)+_tcslen(szTxt4);
	if(iLength == 0){
		if(al_GetSettingString(_T("public"),_T("IDS_ERR_NONUMBER"),theApp.m_szRes,szTxt))
			AfxMessageBox(szTxt);
		return;
	}

	if( !xCheckEmailData() )
		return;

	DEQPHONEDATA deqData;
	CPhoneData data;
	//name
	GSM_EntryType SelEntryType =(GSM_EntryType) m_cbNameType.GetItemData(m_cbNameType.GetCurSel());
	POSITION pos = m_NameList.GetHeadPosition();
	while(pos)
	{
		ContactInfo* pData = (ContactInfo*) m_NameList.GetNext(pos);
		if(pData->entryType == SelEntryType)
		{
			m_edName.GetWindowText(szTxt,MAX_PATH);
			if(_tcslen(szTxt) > 0)
			{
				data.SetType(SelEntryType);
				data.SetText(szTxt);
				deqData.push_back(data);
			}
		}
		else
		{
			if(_tcslen(pData->szData)>0)
			{
				data.SetType(pData->entryType);
				data.SetText(pData->szData);
				deqData.push_back(data);
			}
		}
	}

	//check type
	CWnd *pDataEdWnd[4] = {&m_edData1,&m_edData2,&m_edData3,&m_edData4};
	CComboBox *pDataTypeCbWnd[4] = {&m_cbType1,&m_cbType2,&m_cbType3,&m_cbType4};
	for(int i=0 ; i<4 ;i++)
	{
		if(pDataEdWnd[i]->GetWindowText(szTxt,MAX_PATH)>0)
		{
			switch(pDataTypeCbWnd[i]->GetCurSel()){
			case 0:		//mobile
				data.SetType(PBK_Number_Mobile_Home);
				data.SetText(szTxt);
				deqData.push_back(data);
				break;
			case 1:		//home
				data.SetType(PBK_Number_Home);
				data.SetText(szTxt);
				deqData.push_back(data);
				break;
			case 2:		//company
				data.SetType(PBK_Number_Work);
				data.SetText(szTxt);
				deqData.push_back(data);
				break;
			case 3:		//fax
				data.SetType(PBK_Number_Fax_Home);
				data.SetText(szTxt);
				deqData.push_back(data);
				break;
			case 4:		//general
				data.SetType(PBK_Number_General);
				data.SetText(szTxt);
				deqData.push_back(data);
				break;
			}
		}
	}
	CWnd *pEmailEdWnd[2] = {&m_edEmail1,&m_edEmail2};
	CComboBox *pEmailTypeCbWnd[2] = {&m_cbEmail1,&m_cbEmail2};
	for( i=0 ;i<2;i++)
	{
		if(pEmailEdWnd[i]->GetWindowText(szTxt,MAX_PATH)>0)
		{
			switch(pEmailTypeCbWnd[i]->GetCurSel()){
			case 0:		//personal
				data.SetType(PBK_Text_Email_Home);
				data.SetText(szTxt);
				deqData.push_back(data);
				break;
			case 1:		//work
				data.SetType(PBK_Text_Email_Work);
				data.SetText(szTxt);
				deqData.push_back(data);
				break;
			case 2:		//other
				data.SetType(PBK_Text_Email);
				data.SetText(szTxt);
				deqData.push_back(data);
				break;
			}
		}	
	}
	//group
	///-------------------------------------------
//	data.SetType(PBK_Caller_Group);			
//	data.SetNumber(m_cbGroup.GetCurSel()+1);
//	deqData.push_back(data);
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
	//Anniversary
	if(m_scAnniversary.GetStyle() & WS_VISIBLE)
	{
		if( m_chAnniversary.GetCheck() )	{
			COleDateTime tm;
			m_dpAnniversary.GetTime(tm);
			data.SetType(PBK_Date_Anniversary);
			data.SetDate(tm);
			deqData.push_back(data);
		}
	}
	//NickName
/*	if(m_edNickName.GetStyle() & WS_VISIBLE)
	{
		if(m_edNickName.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_Nickname);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}*/
//Address
	TCHAR szAddress[MAX_PATH*10];
	TCHAR szAddress2[MAX_PATH*10];
	if(m_edAddress1.GetStyle() & WS_VISIBLE && m_edAddress1.GetWindowText(szAddress,MAX_PATH) > 0)
	{
		int nCurSel = m_cbAddress1.GetCurSel();
		switch(nCurSel)
		{
		case 0:
			data.SetType(PBK_Text_Postal_Home);
			break;
		case 1:
			data.SetType(PBK_Text_Postal_Work);
			break;
		case 2:
			data.SetType(PBK_Text_Postal);
			break;
		}
		TCHAR szStreet[MAX_PATH], szCity[MAX_PATH], szState[MAX_PATH], szZIP[MAX_PATH], szCountry[MAX_PATH];
		wsprintf(szAddress2,_T(";%s"),szAddress);
		SpriteAddress( 0,
			szAddress2, 
			szStreet, 
			szCity,
			szState,
			szZIP, 
			szCountry );

		CString strAddress;
		strAddress.Format(_T(";%s;%s;%s;%s;%s"),szStreet,szCity,szState,szZIP,szCountry);

		data.SetText(strAddress);
		deqData.push_back(data);
	}
	if(m_edAddress2.GetStyle() & WS_VISIBLE && m_edAddress2.GetWindowText(szAddress,MAX_PATH) > 0)
	{
		int nCurSel = m_cbAddress2.GetCurSel();
		switch(nCurSel)
		{
		case 0:
			data.SetType(PBK_Text_Postal_Home);
			break;
		case 1:
			data.SetType(PBK_Text_Postal_Work);
			break;
		case 2:
			data.SetType(PBK_Text_Postal);
			break;
		}
		TCHAR szStreet[MAX_PATH], szCity[MAX_PATH], szState[MAX_PATH], szZIP[MAX_PATH], szCountry[MAX_PATH];
		wsprintf(szAddress2,_T(";%s"),szAddress);
		SpriteAddress( 0,
			szAddress2, 
			szStreet, 
			szCity,
			szState,
			szZIP, 
			szCountry );

		CString strAddress;
		strAddress.Format(_T(";%s;%s;%s;%s;%s"),szStreet,szCity,szState,szZIP,szCountry);

		data.SetText(strAddress);
		deqData.push_back(data);
	}
/*	TCHAR szStreet1[MAX_PATH],szStreet2[MAX_PATH], szCity[MAX_PATH], szState[MAX_PATH], szZIP[MAX_PATH], szCountry[MAX_PATH];
	szStreet1[0] = szStreet2[0] = szCity[0] = szState[0] = szZIP[0] = szCountry[0] ='\0';
	//street1
	if(m_edStreet1.GetStyle() & WS_VISIBLE)
		m_edStreet1.GetWindowText(szStreet1,MAX_PATH);
	if(m_edStreet2.GetStyle() & WS_VISIBLE)
		m_edStreet2.GetWindowText(szStreet2,MAX_PATH);
	if(m_edState.GetStyle() & WS_VISIBLE)
		m_edState.GetWindowText(szState,MAX_PATH);
	if(m_edZip.GetStyle() & WS_VISIBLE)
		m_edZip.GetWindowText(szZIP,MAX_PATH);
	if(m_edCity.GetStyle() & WS_VISIBLE)
		m_edCity.GetWindowText(szCity,MAX_PATH);
	if(m_edCountry.GetStyle() & WS_VISIBLE)
		m_edCountry.GetWindowText(szCountry,MAX_PATH);
	if( _tcslen(szStreet1) > 0 || _tcslen(szStreet2) > 0  || _tcslen(szState) > 0 ||  _tcslen(szZIP) > 0 || 
		_tcslen(szCity) > 0 || _tcslen(szCountry) > 0 )
	{
		int nCurSel = m_cbAddress.GetCurSel();
		switch(nCurSel)
		{
		case 0:
			data.SetType(PBK_Text_Postal_Home);
			break;
		case 1:
			data.SetType(PBK_Text_Postal_Work);
			break;
		case 2:
			data.SetType(PBK_Text_Postal);
			break;
		}
		MotoMargeAddress( szTxt, szStreet2, szStreet1, szCity, szState , szZIP, szCountry );
		data.SetText(szTxt);
		deqData.push_back(data);
	}*/
	///for new Data  ---
	if(m_edURL.GetStyle() & WS_VISIBLE)
	{
		if(m_edURL.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_URL_Work);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}

	if(m_edNote.GetStyle() & WS_VISIBLE)
	{
		if(m_edNote.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_Note);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	if(m_edCompany.GetStyle() & WS_VISIBLE)
	{
		if(m_edCompany.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_Company);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	if(m_edTitle.GetStyle() & WS_VISIBLE)
	{
		if(m_edTitle.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_JobTitle);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	if(m_edManagerName.GetStyle() & WS_VISIBLE)
	{
		if(m_edManagerName.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_Manager_Name);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	if(m_edAssistantName.GetStyle() & WS_VISIBLE)
	{
		if(m_edAssistantName.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_Assistant_Name);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	if(m_edSpousName.GetStyle() & WS_VISIBLE)
	{
		if(m_edSpousName.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_Spouse_Name);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	if(m_edChildernName.GetStyle() & WS_VISIBLE)
	{
		if(m_edChildernName.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_Children);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	if(m_edIMID.GetStyle() & WS_VISIBLE)
	{
		if(m_edIMID.GetWindowText(szTxt,MAX_PATH) > 0 ){
			data.SetType(PBK_Text_IMID);
			data.SetText(szTxt);
			deqData.push_back(data);
		}
	}
	m_pData->SetPhoneData(deqData);

	if(m_rdSIM.GetCheck())
		m_pData->SetStorageType(SIM_NAME);
	else
		m_pData->SetStorageType(MEMORY_NAME);

	CBaseDlg::OnOK();
}

void CMotoE2Dlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CBaseDlg::OnCancel();
}


void CMotoE2Dlg::xEnableControl(int iState)
{
	CWnd *pWnd[31] = {&m_edName,&m_cbType1,&m_cbType2,&m_cbType3,&m_cbType4,
		&m_edData1,&m_edData2,&m_edData3,&m_edData4,
		&m_cbEmail1,&m_cbEmail2,&m_edEmail1,&m_edEmail2,&m_cbAddress1,&m_cbAddress2,&m_edAddress1,&m_edAddress2,&m_edIMID ,//};
		&m_dpBirthday,&m_chBirthday,&m_edURL,&m_edNote,&m_dpAnniversary,&m_chAnniversary,&m_cbNameType,
	&m_edCompany,&m_edTitle,&m_edManagerName,&m_edAssistantName,&m_edSpousName,&m_edChildernName};
	if(iState == ENABLE_SIM){
		pWnd[0]->EnableWindow(TRUE);
		for(int i = 1 ; i < 31 ; i ++){
			pWnd[i]->EnableWindow(FALSE);
		}
		pWnd[5]->EnableWindow(TRUE);
	//	m_scFirstName.SetWindowText(m_szName);
	}
	else if(iState == ENABLE_ME){
		for(int i = 0 ; i < 31 ; i ++){
			pWnd[i]->EnableWindow(TRUE);
		}
//		m_scFirstName.SetWindowText(m_szFirstName);
		OnChBirthday();
		OnChAnniversary();
	}
	else if(iState == ENABLE_VIEW){
		for(int i = 0 ; i < 31 ; i ++){
			pWnd[i]->EnableWindow(FALSE);
		}
		m_rdSIM.EnableWindow(FALSE);
		m_rdME.EnableWindow(FALSE);
//		m_scFirstName.SetWindowText(m_szFirstName);
	}
}

CMotoE2Dlg::xSetComboItem()
{
	//set group combo item
	int iCount = 0;
	TCHAR szTxt[MAX_PATH];
	//get group count
/*	::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_CARDPANEL_GROUPCOUNT,reinterpret_cast<WPARAM>(&iCount),0);
	if(iCount > 0){
		for(int i = 1 ; i <= iCount ; i ++){
			//get group item
			::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_CARDPANEL_GETGROUP,i,reinterpret_cast<WPARAM>(szTxt));
			//add to combo
			m_cbGroup.AddString(szTxt);
		}
		m_cbGroup.SetCurSel(0);
	}
*/
	//set type item
	TCHAR szType[5][32]={_T("IDS_M_MOBILE"),_T("IDS_M_HOME"),_T("IDS_M_COMPANY"),_T("IDS_M_FAX"),_T("IDS_N_OTHER")};
	for(int i = 0 ; i < 5 ; i ++){
		if(al_GetSettingString(_T("public"),szType[i],theApp.m_szRes,szTxt))
			m_cbType1.AddString(szTxt);
			m_cbType2.AddString(szTxt);
			m_cbType3.AddString(szTxt);
			m_cbType4.AddString(szTxt);
	}
	m_cbType1.SetCurSel(0);
	m_cbType2.SetCurSel(1);
	m_cbType3.SetCurSel(2);
	m_cbType4.SetCurSel(3);

	//Email Type
	TCHAR szTypeTxt[3][MAX_PATH];
	szTypeTxt[0][0] = szTypeTxt[1][0] = szTypeTxt[2][0]  ='\0';
	TCHAR szTextType[3][32]={_T("IDS_HOME"),_T("IDS_M_WORK"),_T("IDS_N_OTHER")};
	for( i = 0 ; i < 3 ; i ++)
		al_GetSettingString(_T("public"),szTextType[i],theApp.m_szRes,szTypeTxt[i]);
	
	m_cbEmail1.AddString(szTypeTxt[0]);
	m_cbEmail1.AddString(szTypeTxt[1]);
	m_cbEmail1.AddString(szTypeTxt[2]);
	m_cbEmail1.SetCurSel(0);

	m_cbEmail2.AddString(szTypeTxt[0]);
	m_cbEmail2.AddString(szTypeTxt[1]);
	m_cbEmail2.AddString(szTypeTxt[2]);
	m_cbEmail2.SetCurSel(1);

	m_cbAddress1.AddString(szTypeTxt[0]);
	m_cbAddress1.AddString(szTypeTxt[1]);
	m_cbAddress1.AddString(szTypeTxt[2]);
	m_cbAddress1.SetCurSel(0);

	m_cbAddress2.AddString(szTypeTxt[0]);
	m_cbAddress2.AddString(szTypeTxt[1]);
	m_cbAddress2.AddString(szTypeTxt[2]);
	m_cbAddress2.SetCurSel(1);


	//NameCombo
	TCHAR szProfile[MAX_PATH];
	wsprintf(szProfile,_T("%s%s_%s.ini"),theApp.m_szSkin,_T("organize\\MOTOE2Combo"),theApp.m_szPhone);
	if(al_IsFileExist(szProfile) == FALSE)
	{
		wsprintf(szProfile,_T("%s%s.ini"),theApp.m_szSkin,_T("organize\\MOTOE2Combo"));
	}

	int nCount = 0;
	int index;
	TCHAR szkey[MAX_PATH];
	TCHAR szStringID[MAX_PATH];
	TCHAR szComboString[MAX_PATH];
	TCHAR szEntry[MAX_PATH];
	GSM_EntryType entryType;
	if(al_GetSettingInt(_T("NameCombo"),_T("Count"),szProfile,nCount) && nCount>0)
	{
		RemoveNameList();
		
		for(i = 1; i<=nCount ;i++)
		{
			wsprintf(szkey,_T("stringID%d"),i);
			al_GetSettingString(_T("NameCombo"),szkey,szProfile,szStringID);
			wsprintf(szkey,_T("EntryType%d"),i);
			al_GetSettingString(_T("NameCombo"),szkey,szProfile,szEntry);
			
			if(EntryTypeString2EntryType(szEntry,&entryType)==false)
				entryType = PBK_Text_Name;

			al_GetSettingString(_T("public"),szStringID,theApp.m_szRes,szComboString);
			index = m_cbNameType.AddString(szComboString);
			m_cbNameType.SetItemData(index,entryType);

			ContactInfo *pContactInfo =new ContactInfo;

			pContactInfo->entryType = entryType;
			pContactInfo->szData[0]='\0';
			m_NameList.AddTail(pContactInfo);
		}

	}
	m_cbNameType.SetCurSel(0);

}

CMotoE2Dlg::xSetDataToDlgCtrl()
{
	TCHAR szDataText[MAX_PATH];
	int iNumber = 0,iAddress = 0,iEmail = 0;
	szDataText[0] ='\0';
	CString strName,strfirstName,strLastName;
	strName.Empty();
	strfirstName.Empty();
	strLastName.Empty();
	CComboBox *pPhoneCombo[4] = {&m_cbType1,&m_cbType2,&m_cbType3,&m_cbType4};
	CMyEdit *pPhoneEdit[4] = {&m_edData1,&m_edData2,&m_edData3,&m_edData4};

	CComboBox *pEmailCombo[2] = {&m_cbEmail1,&m_cbEmail2};
	CMyEdit *pEmailEdit[2] = {&m_edEmail1,&m_edEmail2};

	CComboBox *pAddressCombo[2] = {&m_cbAddress1,&m_cbAddress2};
	CMyEdit *pAddressEdit[2] = {&m_edAddress1,&m_edAddress2};

	if(m_pData){
		//get data deq
		DEQPHONEDATA *pdeqData; 
		pdeqData = m_pData->GetPhoneData();
		for(DEQPHONEDATA::iterator iter = pdeqData->begin() ; iter != pdeqData->end() ; iter ++)
		{
			POSITION pos = m_NameList.GetHeadPosition();
			while(pos)
			{
				ContactInfo* pData = (ContactInfo*) m_NameList.GetNext(pos);
				if((*iter).GetType() == pData->entryType)
				{
					wsprintf(pData->szData,(*iter).GetText());
					break;
				}
			}
GSM_EntryType temp = (*iter).GetType() ;
			if((*iter).GetType() == PBK_Text_Name){		//full name
				strName.Format(_T("%s"),((*iter).GetText()));
			}
			else if((*iter).GetType() == PBK_Text_FirstName){		//name
				strfirstName.Format(_T("%s"),((*iter).GetText()));
			}
			else if((*iter).GetType() == PBK_Text_LastName){		//name
				strLastName.Format(_T("%s"),((*iter).GetText()));
			}
			else if((*iter).GetType() == PBK_Number_General && iNumber < 4){	//general
				pPhoneCombo[iNumber]->SetCurSel(4);
				pPhoneEdit[iNumber]->SetWindowText((*iter).GetText());
				iNumber ++;
			}
			else if((*iter).GetType() == PBK_Number_Mobile_Home && iNumber < 4){	//mobile
				pPhoneCombo[iNumber]->SetCurSel(0);
				pPhoneEdit[iNumber]->SetWindowText((*iter).GetText());
				iNumber ++;
			}
			else if((*iter).GetType() == PBK_Number_Fax_Home && iNumber < 4){	//fax
				pPhoneCombo[iNumber]->SetCurSel(3);
				pPhoneEdit[iNumber]->SetWindowText((*iter).GetText());
				iNumber ++;
			}
			else if((*iter).GetType() == PBK_Number_Work && iNumber < 4){	//company
				pPhoneCombo[iNumber]->SetCurSel(2);
				pPhoneEdit[iNumber]->SetWindowText((*iter).GetText());
				iNumber ++;
			}
			else if((*iter).GetType() == PBK_Number_Home && iNumber < 4){	//home
				pPhoneCombo[iNumber]->SetCurSel(1);
				pPhoneEdit[iNumber]->SetWindowText((*iter).GetText());
				iNumber ++;
			}
		/*	else if((*iter).GetType() == PBK_Caller_Group){		//group
				if(strcmp(m_pData->GetStorageType(),SIM_NAME) == 0)
					m_cbGroup.SetCurSel(0);
				else
					m_cbGroup.SetCurSel((*iter).GetNumber()-1);
			}*/
			//email
			else if((*iter).GetType() == PBK_Text_Email_Home && iEmail <2){	//email
				pEmailCombo[iEmail]->SetCurSel(0);
				pEmailEdit[iEmail]->SetWindowText((*iter).GetText());
				iEmail ++;

			}
			else if((*iter).GetType() == PBK_Text_Email_Work&& iEmail <2){	//email
				pEmailCombo[iEmail]->SetCurSel(1);
				pEmailEdit[iEmail]->SetWindowText((*iter).GetText());
				iEmail ++;
			}
			else if((*iter).GetType() == PBK_Text_Email&& iEmail <2){	//email
				pEmailCombo[iEmail]->SetCurSel(2);
				pEmailEdit[iEmail]->SetWindowText((*iter).GetText());
				iEmail ++;
			}
//////////
			else if(((*iter).GetType() == PBK_Text_Postal ||(*iter).GetType() == PBK_Text_Postal_Work ||
				(*iter).GetType() == PBK_Text_Postal_Home) && iAddress <2)
			{	//Postal other
				TCHAR szStreet[MAX_PATH], szCity[MAX_PATH], szState[MAX_PATH], szZIP[MAX_PATH], szCountry[MAX_PATH];
				SpriteAddress( 0,
					(*iter).GetText(), 
					szStreet, 
					szCity,
					szState,
					szZIP, 
					szCountry );

				CString strAddress;
				strAddress.Format(_T("%s;%s;%s;%s;%s"),szStreet,szCity,szState,szZIP,szCountry);
				pAddressEdit[iAddress]->SetWindowText(strAddress);

				if((*iter).GetType() == PBK_Text_Postal) pAddressCombo[iAddress]->SetCurSel(2);
				else if((*iter).GetType() == PBK_Text_Postal_Work)pAddressCombo[iAddress]->SetCurSel(1);
				else if((*iter).GetType() == PBK_Text_Postal_Home)pAddressCombo[iAddress]->SetCurSel(0);
				iAddress++;

			}

			else if((*iter).GetType() == PBK_Date){		//birthday
				COleDateTime tm;
				(*iter).GetTime(tm);
				m_dpBirthday.SetTime(tm);
				m_chBirthday.SetCheck(1);
				if(m_iMode!=2)
					m_dpBirthday.EnableWindow(true);
			}		
			else if((*iter).GetType() == PBK_Date_Anniversary){		//Anniversary
				COleDateTime tm;
				(*iter).GetTime(tm);
				m_dpAnniversary.SetTime(tm);
				m_chAnniversary.SetCheck(1);
				if(m_iMode!=2)
					m_dpAnniversary.EnableWindow(true);
			}		
			else if((*iter).GetType() == PBK_Text_URL_Work){
				m_edURL.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Note){
				m_edNote.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_Company)
				m_edCompany.SetWindowText((*iter).GetText());
			else if((*iter).GetType() == PBK_Text_JobTitle)
				m_edTitle.SetWindowText((*iter).GetText());
			else if((*iter).GetType() == PBK_Text_Manager_Name)
				m_edManagerName.SetWindowText((*iter).GetText());
			else if((*iter).GetType() == PBK_Text_Assistant_Name)
				m_edAssistantName.SetWindowText((*iter).GetText());
			else if((*iter).GetType() == PBK_Text_Spouse_Name)
				m_edSpousName.SetWindowText((*iter).GetText());
			else if((*iter).GetType() == PBK_Text_Children)
				m_edChildernName.SetWindowText((*iter).GetText());
			else if((*iter).GetType() == PBK_Text_IMID)
				m_edIMID.SetWindowText((*iter).GetText());

/////////
			else{	//others
				m_deqDetainData.push_back((*iter));
			}
		}
		if(strfirstName.IsEmpty() && strLastName.IsEmpty() && strName.IsEmpty()== false)
		{
			strfirstName.Format(_T("%s"),strName);
			POSITION pos = m_NameList.GetHeadPosition();
			while(pos)
			{
				ContactInfo* pData = (ContactInfo*) m_NameList.GetNext(pos);
				if((*iter).GetType() == PBK_Text_FirstName)
				{
					wsprintf(pData->szData,strfirstName);
					break;
				}
			}
		}
		GSM_EntryType nameentryType =PBK_Text_Name;
		POSITION pos = m_NameList.GetHeadPosition();
		while(pos)
		{
			ContactInfo* pData = (ContactInfo*) m_NameList.GetNext(pos);
			if(_tcslen(pData->szData)>0)
			{
				nameentryType = pData->entryType;
				break;
			}
		}
		for( int i = 0 ;i<m_cbNameType.GetCount(); i++)
		{
			GSM_EntryType type = (GSM_EntryType)m_cbNameType.GetItemData(i);;
			if(nameentryType == type)
			{
				m_cbNameType.SetCurSel(i);
				OnSelchangeNameCombo();
				break;
			}
		}

	}

}
void CMotoE2Dlg::OnSelchangeNameCombo() 
{
	// TODO: Add your control notification handler code here
	GSM_EntryType SelEntryType =(GSM_EntryType) m_cbNameType.GetItemData(m_cbNameType.GetCurSel());
	POSITION pos = m_NameList.GetHeadPosition();
	while(pos)
	{
		ContactInfo* pData = (ContactInfo*) m_NameList.GetNext(pos);
		if(pData->entryType == SelEntryType)
		{
			m_edName.SetWindowText(pData->szData);
			break;
		}
	}

}
void CMotoE2Dlg::OnDropdownNameCombo() 
{
	// TODO: Add your control notification handler code here
	GSM_EntryType SelEntryType =(GSM_EntryType) m_cbNameType.GetItemData(m_cbNameType.GetCurSel());
	POSITION pos = m_NameList.GetHeadPosition();
	while(pos)
	{
		ContactInfo* pData = (ContactInfo*) m_NameList.GetNext(pos);
		if(pData->entryType == SelEntryType)
		{
			m_edName.GetWindowText(pData->szData,(GSM_PHONEBOOK_TEXT_LENGTH+1)*2);
			break;
		}
	}
	
}

BOOL CMotoE2Dlg::xCheckEmailData()
{
	CString str,str2;
	TCHAR szText[MAX_PATH];

	m_edEmail1.GetWindowText(str);
	m_edEmail2.GetWindowText(str2);
	if(str.GetLength()<=0 && str2.GetLength()<=0) return TRUE;
	if(str.GetLength()>0)
	{
		int nPos = str.Find('@');
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
	}
	if(str2.GetLength()>0)
	{
		int nPos = str2.Find('@');
		if(nPos==-1 || nPos==0 || nPos==str2.GetLength()-1)	
		{
				al_GetSettingString(_T("public"), _T("IDS_ERR_EMAIL"), theApp.m_szRes, szText);
				AfxMessageBox(szText);
				return FALSE;
		}
		nPos = str2.Find(' ');
		if(nPos!=-1 )	{
			al_GetSettingString(_T("public"), _T("IDS_ERR_EMAIL"), theApp.m_szRes, szText);
			AfxMessageBox(szText);
			return FALSE;
		}
	}

	return TRUE;
}
void CMotoE2Dlg::OnChAnniversary() 
{
	// TODO: Add your control notification handler code here
	if(m_chAnniversary.GetCheck())	
		m_dpAnniversary.EnableWindow(TRUE);
	else
		m_dpAnniversary.EnableWindow(FALSE);
	
}
void CMotoE2Dlg::OnChBirthday() 
{
	// TODO: Add your control notification handler code here
	if(m_chBirthday.GetCheck())	
		m_dpBirthday.EnableWindow(TRUE);
	else
		m_dpBirthday.EnableWindow(FALSE);
	
}
void CMotoE2Dlg::EditAddress(CMyEdit* pEdit)
{
	CString strAddress;
	CString strAddress_2;
	pEdit->GetWindowText(strAddress);
	TCHAR szStreet[MAX_PATH], szCity[MAX_PATH], szState[MAX_PATH], szZIP[MAX_PATH], szCountry[MAX_PATH];

	szStreet[0] = szCity[0]=szState[0]=szZIP[0]=szCountry[0] ='\0';
	if(strAddress.GetLength() > 0)
	{
		strAddress_2.Format(_T(";%s"),strAddress);
		SpriteAddress( 0,
			strAddress_2, 
			szStreet, 
			szCity,
			szState,
			szZIP, 
			szCountry );
	}

	CAddressDlg addressDlg;
	addressDlg.SetAddress(szStreet,szCity,
		szState,szZIP,szCountry);
	if(addressDlg.DoModal() == IDOK)
	{
		CString strAddress2;
		strAddress2.Format(_T("%s;%s;%s;%s;%s"),addressDlg.m_strStreet,addressDlg.m_strCity,
			addressDlg.m_strState,addressDlg.m_strZip,addressDlg.m_strCountry);
		
		pEdit->SetWindowText(strAddress2);
	}
	SetFocus();



}

void CMotoE2Dlg::OnSetfocusAddressEdit1() 
{
	// TODO: Add your control notification handler code here
	static bInAddressdlg = false;
	if( !bInAddressdlg)
	{
		bInAddressdlg = true;
		EditAddress(&m_edAddress1);
	}
	bInAddressdlg= false;
}
void CMotoE2Dlg::OnSetfocusAddressEdit2() 
{
	// TODO: Add your control notification handler code here
	static bInAddressdlg = false;
	if( !bInAddressdlg)
	{
		bInAddressdlg = true;
		EditAddress(&m_edAddress2);
	}
	bInAddressdlg= false;
}
