// OtherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "OtherDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COtherDlg dialog


COtherDlg::COtherDlg(CWnd* pParent /*=NULL*/)
	: CBaseDlg(COtherDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COtherDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hFont = NULL;
	m_CtrlInfolist.RemoveAll();
	m_pFirstPhoneNoWnd = NULL;
	m_pName_infirstPos = NULL;
	m_pscNamefirstPos = NULL;

}
COtherDlg::~COtherDlg()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
	POSITION pos = m_CtrlInfolist.GetHeadPosition();
	while(pos)
	{
		MYCONTROLINFO * pCtlInfo = (MYCONTROLINFO *)m_CtrlInfolist.GetNext(pos);
		if(pCtlInfo)
		{
			pCtlInfo->pControlList.RemoveAll();
			delete pCtlInfo;
			pCtlInfo = NULL;
		}
	}
	m_CtrlInfolist.RemoveAll();
}


void COtherDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COtherDlg)
	DDX_Control(pDX, IDC_ED_HIDEPOSTAL, m_edHidePostal);
	DDX_Control(pDX, IDOK, m_rbnOk);
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDC_SC_TEXT9, m_scText9);
	DDX_Control(pDX, IDC_SC_TEXT8, m_scText8);
	DDX_Control(pDX, IDC_SC_TEXT7, m_scText7);
	DDX_Control(pDX, IDC_SC_TEXT6, m_scText6);
	DDX_Control(pDX, IDC_SC_TEXT5, m_scText5);
	DDX_Control(pDX, IDC_SC_TEXT4, m_scText4);
	DDX_Control(pDX, IDC_SC_TEXT3, m_scText3);
	DDX_Control(pDX, IDC_SC_TEXT2, m_scText2);
	DDX_Control(pDX, IDC_SC_TEXT10, m_scText10);
	DDX_Control(pDX, IDC_SC_TEXT11, m_scText11);
	DDX_Control(pDX, IDC_SC_TEXT12, m_scText12);
	DDX_Control(pDX, IDC_SC_TEXT13, m_scText13);
	DDX_Control(pDX, IDC_SC_TEXT14, m_scText14);
	DDX_Control(pDX, IDC_SC_TEXT15, m_scText15);
	DDX_Control(pDX, IDC_SC_TEXT1, m_scText1);
	DDX_Control(pDX, IDC_SC_POSITION, m_scPosition);
	DDX_Control(pDX, IDC_SC_PHONENO9, m_scPhoneNo9);
	DDX_Control(pDX, IDC_SC_PHONENO8, m_scPhoneNo8);
	DDX_Control(pDX, IDC_SC_PHONENO7, m_scPhoneNo7);
	DDX_Control(pDX, IDC_SC_PHONENO6, m_scPhoneNo6);
	DDX_Control(pDX, IDC_SC_PHONENO5, m_scPhoneNo5);
	DDX_Control(pDX, IDC_SC_PHONENO4, m_scPhoneNo4);
	DDX_Control(pDX, IDC_SC_PHONENO3, m_scPhoneNo3);
	DDX_Control(pDX, IDC_SC_PHONENO2, m_scPhoneNo2);
	DDX_Control(pDX, IDC_SC_PHONENO10, m_scPhoneNo10);
	DDX_Control(pDX, IDC_SC_PHONENO1, m_scPhoneNo1);
	DDX_Control(pDX, IDC_SC_NAME, m_scName);
	DDX_Control(pDX, IDC_SC_LINE, m_scLine);
	DDX_Control(pDX, IDC_SC_LASTNAME, m_scLastName);
	DDX_Control(pDX, IDC_SC_GROUP, m_scGroup);
	DDX_Control(pDX, IDC_SC_FIRSTNAME, m_scFirstName);
	DDX_Control(pDX, IDC_SC_BIRTHDAY, m_scBirthday);
	DDX_Control(pDX, IDC_ED_TEXT9, m_edText9);
	DDX_Control(pDX, IDC_ED_TEXT8, m_edText8);
	DDX_Control(pDX, IDC_ED_TEXT7, m_edText7);
	DDX_Control(pDX, IDC_ED_TEXT6, m_edText6);
	DDX_Control(pDX, IDC_ED_TEXT5, m_edText5);
	DDX_Control(pDX, IDC_ED_TEXT4, m_edText4);
	DDX_Control(pDX, IDC_ED_TEXT3, m_edText3);
	DDX_Control(pDX, IDC_ED_TEXT2, m_edText2);
	DDX_Control(pDX, IDC_ED_TEXT10, m_edText10);
	DDX_Control(pDX, IDC_ED_TEXT11, m_edText11);
	DDX_Control(pDX, IDC_ED_TEXT12, m_edText12);
	DDX_Control(pDX, IDC_ED_TEXT13, m_edText13);
	DDX_Control(pDX, IDC_ED_TEXT14, m_edText14);
	DDX_Control(pDX, IDC_ED_TEXT15, m_edText15);
	DDX_Control(pDX, IDC_ED_TEXT1, m_edText1);
	DDX_Control(pDX, IDC_ED_PHONENO9, m_edPhoneNo9);
	DDX_Control(pDX, IDC_ED_PHONENO8, m_edPhoneNo8);
	DDX_Control(pDX, IDC_ED_PHONENO7, m_edPhoneNo7);
	DDX_Control(pDX, IDC_ED_PHONENO6, m_edPhoneNo6);
	DDX_Control(pDX, IDC_ED_PHONENO5, m_edPhoneNo5);
	DDX_Control(pDX, IDC_ED_PHONENO4, m_edPhoneNo4);
	DDX_Control(pDX, IDC_ED_PHONENO3, m_edPhoneNo3);
	DDX_Control(pDX, IDC_ED_PHONENO2, m_edPhoneNo2);
	DDX_Control(pDX, IDC_ED_PHONENO10, m_edPhoneNo10);
	DDX_Control(pDX, IDC_ED_PHONENO1, m_edPhoneNo1);
	DDX_Control(pDX, IDC_ED_NAME, m_edName);
	DDX_Control(pDX, IDC_ED_LASTNAME, m_edLastName);
	DDX_Control(pDX, IDC_ED_FIRSTNAME, m_edFirstName);
	DDX_Control(pDX, IDC_DP_BIRTHDAY, m_dpBirthday);
	DDX_Control(pDX, IDC_CH_BIRTHDAY, m_chBirthday);
	DDX_Control(pDX, IDC_CB_GROUPCOMBO, m_cbGroup);
	DDX_Control(pDX, IDC_RD_SIM, m_rdSIM);
	DDX_Control(pDX, IDC_RD_ME, m_rdME);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COtherDlg, CBaseDlg)
	//{{AFX_MSG_MAP(COtherDlg)
	ON_BN_CLICKED(IDC_RD_SIM, OnRdSim)
	ON_BN_CLICKED(IDC_RD_ME, OnRdMobile)
	ON_BN_CLICKED(IDC_CH_BIRTHDAY, OnChBirthday)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COtherDlg message handlers

BOOL COtherDlg::OnInitDialog() 
{
	CBaseDlg::OnInitDialog();
	int nRight = 0;
	// TODO: Add extra initialization here
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
	TCHAR szCompany[MAX_PATH];
	szCompany[0] = '\0';
	switch(theApp.m_iMobileCompany )
	{
		case CO_NOKIA:
			wsprintf(szCompany,_T("Nokia"));
			break;
		case CO_SONY_ERISON:
			wsprintf(szCompany,_T("SE"));
			break;
		case CO_SIMENS:
			wsprintf(szCompany,_T("Siemens"));
			break;
		case CO_ASUS:
			wsprintf(szCompany,_T("ASUS"));
			break;
		case CO_MOTO:
			wsprintf(szCompany,_T("MOTO"));
			break;
		case CO_SAMSUNG:
			wsprintf(szCompany,_T("Samsung"));
			break;
		case CO_SHARP:
			wsprintf(szCompany,_T("Sharp"));
			break;
		case CO_LG:
			wsprintf(szCompany,_T("LG"));
			break;
		case CO_PANASONIC:
			wsprintf(szCompany,_T("Panasonic"));
			break;
		case CO_SAGEM:
			wsprintf(szCompany,_T("SAGEM"));
			break;
		case CO_TOSHIBA:
			wsprintf(szCompany,_T("Toshiba"));
			break;
		case CO_MTK:
			wsprintf(szCompany,_T("MTK"));
			break;
	}
	//get profile path
	Tstring sProfile(theApp.m_szSkin);
	
	TCHAR szProfile[MAX_PATH];
	if(_tcslen(szCompany)>0)
		wsprintf(szProfile,_T("%sOrganize\\%s %s.ini"),theApp.m_szSkin,szCompany,theApp.m_szPhone);
	else
		wsprintf(szProfile,_T("%sOrganize\\%s.ini"),theApp.m_szSkin,theApp.m_szPhone);
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
		wsprintf(szProfile,_T("%sOrganize\\%s.ini"),theApp.m_szSkin,szCompany);
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
			sProfile += Tstring(_T("Organize\\Default.ini"));
	}

//	al_GetSettingString(_T("public"),_T("IDS_N_FIRSTNAME",theApp.m_szRes,m_szFirstName);
	al_GetSettingString(_T("public"),_T("IDS_NAME"),theApp.m_szRes,m_szName);
	//Load static
	CStaticEx	*pStaticEx[6] = {&m_scName,&m_scLastName,&m_scFirstName,&m_scGroup,	&m_scBirthday,&m_scPosition};
	TCHAR szScSec[6][32] = {_T("name_sc"),_T("lastname_sc"),_T("firstname_sc"),_T("group_sc"),_T("birthday_sc"),_T("position_sc")};
	TCHAR szScID[6][32] = {_T("IDS_NAME"),_T("IDS_N_LASTNAME"),_T("IDS_N_FIRSTNAME"),_T("IDS_N_GROUP"),_T("IDS_N_BIRTHDAY"),_T("IDS_STORAGE")};
	for(int i = 0 ; i < 6 ; i ++)
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

	//set other control position and size
	CWnd *pWnd[6] = {&m_edName,&m_edLastName,&m_edFirstName,&m_dpBirthday,&m_cbGroup,&m_scLine};
	TCHAR szWndSec[6][32] = {_T("name_ed"),_T("lastname_ed"),_T("firstname_ed"),_T("birthday_dp"),_T("group_cb"),_T("line")};
	for(i = 0 ; i < 6 ; i ++){
		CRect rect;
		if(al_GetSettingRect(szWndSec[i],_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect))
		{
			pWnd[i]->ShowWindow(SW_SHOW);
			pWnd[i]->MoveWindow(rect);
			if(pFont) pWnd[i]->SetFont(pFont);
			if(rect.right > nRight) nRight =rect.right ;
		}
		else 
			pWnd[i]->ShowWindow(SW_HIDE);

	}
	if((m_edFirstName.GetStyle() & WS_VISIBLE) &&  (m_edLastName.GetStyle() & WS_VISIBLE))
	{
		CRect rc1,rc2;
		m_edFirstName.GetWindowRect(&rc1);
		ScreenToClient(&rc1);
		m_edLastName.GetWindowRect(&rc2);
		ScreenToClient(&rc2);
		if(rc1.top > rc2.top)
		{
			m_pName_infirstPos = &m_edLastName;
			m_pscNamefirstPos = &m_scLastName;
			al_GetSettingString(_T("public"),_T("IDS_N_LASTNAME"),theApp.m_szRes,m_szFirstName);
		}
		else
		{
			m_pName_infirstPos = &m_edFirstName;
			m_pscNamefirstPos = &m_scFirstName;
			al_GetSettingString(_T("public"),_T("IDS_N_FIRSTNAME"),theApp.m_szRes,m_szFirstName);

		}
	}

	//Load PhoneNo Control
	CMyEdit *pPhoneNoWnd[10] = {&m_edPhoneNo1,&m_edPhoneNo2,&m_edPhoneNo3,&m_edPhoneNo4,&m_edPhoneNo5,
		&m_edPhoneNo6,&m_edPhoneNo7,&m_edPhoneNo8,&m_edPhoneNo9,&m_edPhoneNo10};
	CStaticEx	*pPhoneNoStaticEx[10] = {&m_scPhoneNo1,&m_scPhoneNo2,&m_scPhoneNo3,&m_scPhoneNo4,&m_scPhoneNo5,
		&m_scPhoneNo6,&m_scPhoneNo7,&m_scPhoneNo8,&m_scPhoneNo9,&m_scPhoneNo10};
	int nPhoneNoCtrlCount = 0 ;
	int nControlIndex = 0;
	al_GetSettingInt(_T("PhoneNoCtrl"),_T("nCount"),const_cast<TCHAR*>(sProfile.c_str()),nPhoneNoCtrlCount);
	for(i = 1; i<=nPhoneNoCtrlCount ;i++)
	{
		TCHAR szKey[MAX_PATH];
		TCHAR szStaticStringID[MAX_PATH];
		TCHAR szStaticCtrlsection[MAX_PATH];
		TCHAR szEditCtrlsection[MAX_PATH];
		TCHAR szEntryType[MAX_PATH];
		wsprintf(szKey,_T("StringID%d"),i);
		if(al_GetSettingString(_T("PhoneNoCtrl"),szKey,const_cast<TCHAR*>(sProfile.c_str()),szStaticStringID))
		{
			wsprintf(szKey,_T("StaticControlSec%d"),i);
			if(al_GetSettingString(_T("PhoneNoCtrl"),szKey,const_cast<TCHAR*>(sProfile.c_str()),szStaticCtrlsection))
			{
				wsprintf(szKey,_T("EditControlSec%d"),i);
				if(al_GetSettingString(_T("PhoneNoCtrl"),szKey,const_cast<TCHAR*>(sProfile.c_str()),szEditCtrlsection))
				{
					wsprintf(szKey,_T("EntryType%d"),i);
					al_GetSettingString(_T("PhoneNoCtrl"),szKey,const_cast<TCHAR*>(sProfile.c_str()),szEntryType);

					GetStaticFromSetting2(pPhoneNoStaticEx[nControlIndex],szStaticCtrlsection,szStaticStringID,const_cast<TCHAR*>(sProfile.c_str()));

					CRect rect;
					if(al_GetSettingRect(szEditCtrlsection,_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect))
					{
						pPhoneNoWnd[nControlIndex]->ShowWindow(SW_SHOW);
						pPhoneNoWnd[nControlIndex]->MoveWindow(rect);
						if(pFont) pPhoneNoWnd[nControlIndex]->SetFont(pFont);
						if(rect.right > nRight) nRight =rect.right ;

					}
					else 
						pPhoneNoWnd[nControlIndex]->ShowWindow(SW_HIDE);
					pPhoneNoWnd[nControlIndex]->SetNumFlag(true);
					if(m_pFirstPhoneNoWnd == NULL) m_pFirstPhoneNoWnd = pPhoneNoWnd[nControlIndex];
					MYCONTROLINFO *pcontrolinfo = new MYCONTROLINFO;
					pcontrolinfo->nEntryType = GetEntryTypeID(szEntryType);;
					pcontrolinfo->bPhoneNo = true;
					pcontrolinfo->pControlList.RemoveAll();
					pcontrolinfo->pControlList.AddTail(pPhoneNoWnd[nControlIndex]);
					m_CtrlInfolist.AddTail(pcontrolinfo);
					nControlIndex++;
				}
			}

		}
	}
	for(i = nControlIndex; i<10 ;i++)
	{
		pPhoneNoWnd[i]->ShowWindow(SW_HIDE);
		pPhoneNoStaticEx[i]->ShowWindow(SW_HIDE);
	}


	//Load Text Control
	CWnd *pTextWnd[15] = {&m_edText1,&m_edText2,&m_edText3,&m_edText4,&m_edText5,
		&m_edText6,&m_edText7,&m_edText8,&m_edText9,&m_edText10,&m_edText11,&m_edText12,&m_edText13,&m_edText14,&m_edText15};
	CStaticEx	*pTextStaticEx[15] = {&m_scText1,&m_scText2,&m_scText3,&m_scText4,&m_scText5,
		&m_scText6,&m_scText7,&m_scText8,&m_scText9,&m_scText10,&m_scText11,&m_scText12,&m_scText13,&m_scText14,&m_scText15};
	int nTextCtrlCount = 0 ;
	nControlIndex = 0;
	int nPostalItemCount = 0;
	al_GetSettingInt(_T("TextCtrl"),_T("nCount"),const_cast<TCHAR*>(sProfile.c_str()),nTextCtrlCount);
	al_GetSettingInt(_T("PostalCtrlItem"),_T("nCount"),const_cast<TCHAR*>(sProfile.c_str()),nPostalItemCount);
	for(i = 1; i<=nTextCtrlCount ;i++)
	{
		TCHAR szKey[MAX_PATH];
		TCHAR szStaticStringID[MAX_PATH];
		TCHAR szStaticCtrlsection[MAX_PATH];
		TCHAR szEditCtrlsection[MAX_PATH];
		TCHAR szEntryType[MAX_PATH];

		wsprintf(szKey,_T("EntryType%d"),i);
		if(al_GetSettingString(_T("TextCtrl"),szKey,const_cast<TCHAR*>(sProfile.c_str()),szEntryType) == false) continue;
		GSM_EntryType nEntryType = GetEntryTypeID(szEntryType);
		if(nEntryType != PBK_Text_Postal || nPostalItemCount<=1)
		{
			wsprintf(szKey,_T("StringID%d"),i);
			if(al_GetSettingString(_T("TextCtrl"),szKey,const_cast<TCHAR*>(sProfile.c_str()),szStaticStringID))
			{
				wsprintf(szKey,_T("StaticControlSec%d"),i);
				if(al_GetSettingString(_T("TextCtrl"),szKey,const_cast<TCHAR*>(sProfile.c_str()),szStaticCtrlsection))
				{
					wsprintf(szKey,_T("EditControlSec%d"),i);
					if(al_GetSettingString(_T("TextCtrl"),szKey,const_cast<TCHAR*>(sProfile.c_str()),szEditCtrlsection))
					{
						GetStaticFromSetting2(pTextStaticEx[nControlIndex],szStaticCtrlsection,szStaticStringID,const_cast<TCHAR*>(sProfile.c_str()));
						CRect rect;
						if(al_GetSettingRect(szEditCtrlsection,_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect))
						{
							pTextWnd[nControlIndex]->ShowWindow(SW_SHOW);
							pTextWnd[nControlIndex]->MoveWindow(rect);
							if(pFont) pTextWnd[nControlIndex]->SetFont(pFont);
							if(rect.right > nRight) nRight =rect.right ;

						}
						else 
							pTextWnd[nControlIndex]->ShowWindow(SW_HIDE);

						MYCONTROLINFO *pcontrolinfo = new MYCONTROLINFO;
						pcontrolinfo->bPhoneNo = false;
						pcontrolinfo->nEntryType = nEntryType;
						pcontrolinfo->pControlList.RemoveAll();
						pcontrolinfo->pControlList.AddTail(pTextWnd[nControlIndex]);
						m_CtrlInfolist.AddTail(pcontrolinfo);
						nControlIndex++;
					}
				}
			}
		}//if(nEntryType != PBK_Text_Postal)
		else
		{
			MYCONTROLINFO *pcontrolinfo = new MYCONTROLINFO;
			pcontrolinfo->bPhoneNo = false;
			pcontrolinfo->nEntryType = nEntryType;
			pcontrolinfo->pControlList.RemoveAll();
			for(int Postalindex = 1 ; Postalindex<=nPostalItemCount; Postalindex++)
			{
				wsprintf(szKey,_T("StringID%d"),Postalindex);
				if(al_GetSettingString(_T("PostalCtrlItem"),szKey,const_cast<TCHAR*>(sProfile.c_str()),szStaticStringID))
				{
					wsprintf(szKey,_T("StaticControlSec%d"),Postalindex);
					if(al_GetSettingString(_T("PostalCtrlItem"),szKey,const_cast<TCHAR*>(sProfile.c_str()),szStaticCtrlsection))
					{
						wsprintf(szKey,_T("EditControlSec%d"),Postalindex);
						if(al_GetSettingString(_T("PostalCtrlItem"),szKey,const_cast<TCHAR*>(sProfile.c_str()),szEditCtrlsection))
						{
							GetStaticFromSetting2(pTextStaticEx[nControlIndex],szStaticCtrlsection,szStaticStringID,const_cast<TCHAR*>(sProfile.c_str()));
							CRect rect;
							if(al_GetSettingRect(szEditCtrlsection,_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect))
							{
								pTextWnd[nControlIndex]->ShowWindow(SW_SHOW);
								pTextWnd[nControlIndex]->MoveWindow(rect);
								if(pFont) pTextWnd[nControlIndex]->SetFont(pFont);
								if(rect.right > nRight) nRight =rect.right ;

							}
							else 
								pTextWnd[nControlIndex]->ShowWindow(SW_HIDE);

							pcontrolinfo->pControlList.AddTail(pTextWnd[nControlIndex]);
							nControlIndex++;
						}
					}
				}
				else
					pcontrolinfo->pControlList.AddTail(&m_edHidePostal);
			}//for
			m_CtrlInfolist.AddTail(pcontrolinfo);
		}
	}

	for(i = nControlIndex; i<15 ;i++)
	{
		pTextWnd[i]->ShowWindow(SW_HIDE);
		pTextStaticEx[i]->ShowWindow(SW_HIDE);
	}


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
	CRect rcBtn1,rcBtn2;
	m_rbnCancel.GetWindowRect(&rcBtn1);
	ScreenToClient(&rcBtn1);
	m_rbnOk.GetWindowRect(&rcBtn2);
	ScreenToClient(&rcBtn2);
	if(rcBtn1.Width() > rcBtn2.Width())
		rcBtn2.right = rcBtn2.left + rcBtn1.Width();
	else
		rcBtn1.right = rcBtn1.left + rcBtn2.Width();

//	m_dpBirthday.GetWindowRect(&rc);
//	ScreenToClient(&rc);
	if(nRight > 0)
	{

		rcBtn1.OffsetRect(nRight - rcBtn1.right  ,0);
		m_rbnCancel.MoveWindow(rcBtn1);

		rcBtn2.OffsetRect(rcBtn1.left - 20 - rcBtn2.right ,0);
		m_rbnOk.MoveWindow(rcBtn2);
	}

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
	
	xSetComboItem(const_cast<TCHAR *>(sProfile.c_str()));
	xSetDataToDlgCtrl();
	if(m_iMode == ENABLE_VIEW)
	{
		if(m_pscNamefirstPos)
		{
			if(m_iIsME)
				m_pscNamefirstPos->SetWindowText(m_szFirstName);
			else
				m_pscNamefirstPos->SetWindowText(m_szName);
		}

	}
	if(((CPhoneBookApp*)afxGetApp())->m_bNotSupportME && m_iMode != 2)
	{
		m_rdME.EnableWindow(FALSE);
		m_rdSIM.EnableWindow(FALSE);
		OnRdSim();
	}
	else
	{
		if(((CPhoneBookApp*)afxGetApp())->m_bNotSupportSM && m_iMode != 2)
		{
			m_rdME.EnableWindow(FALSE);
			m_rdSIM.EnableWindow(FALSE);
			OnRdMobile();
		}
	}


	if(m_edName.GetStyle()& WS_VISIBLE)
		m_edName.SetFocus();
	else if(m_pName_infirstPos)
		m_pName_infirstPos->SetFocus();

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
	
}
void COtherDlg::OnRdMobile() 
{
	m_rdME.SetCheck(TRUE);
	m_rdSIM.SetCheck(FALSE);
	xEnableControl(ENABLE_ME);
}

void COtherDlg::OnRdSim() 
{
	m_rdME.SetCheck(FALSE);
	m_rdSIM.SetCheck(TRUE);
	xEnableControl(ENABLE_SIM);
}
void COtherDlg::OnChBirthday() 
{
	// TODO: Add your control notification handler code here
	if(m_chBirthday.GetCheck())	
		m_dpBirthday.EnableWindow(TRUE);
	else
		m_dpBirthday.EnableWindow(FALSE);
	
}

void COtherDlg::OnOK() 
{
	// TODO: Add extra validation here
	bool bFillPhoneNo = false;
	POSITION pos = m_CtrlInfolist.GetHeadPosition();
	while(pos && bFillPhoneNo == false)
	{
		MYCONTROLINFO * pCtlInfo = (MYCONTROLINFO *)m_CtrlInfolist.GetNext(pos);
		if(pCtlInfo)
		{
			if(pCtlInfo->bPhoneNo == true)
			{
				POSITION WndPos = pCtlInfo->pControlList.GetHeadPosition();
				while(WndPos)
				{
					CWnd * pCtrlWnd = (CWnd *)pCtlInfo->pControlList.GetNext(WndPos);
					if(pCtrlWnd && pCtrlWnd != &m_edHidePostal)
					{
						CString str;
						pCtrlWnd->GetWindowText(str);
						if(str.IsEmpty() == false)
						{
							bFillPhoneNo = true;
							break;
						}

					}
				}
			}
		}
	}
	if(bFillPhoneNo == false)
	{
		TCHAR szMobile[MAX_PATH];
		if(al_GetSettingString(_T("public"),_T("IDS_ERR_NONUMBER"),theApp.m_szRes,szMobile))
			AfxMessageBox(szMobile);
		return;
	}
	DEQPHONEDATA deqData;
	CPhoneData data;

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
	if(m_edName.GetStyle() & WS_VISIBLE)
	{
		if(m_edName.GetWindowText(szName,MAX_PATH) > 0)
		{
			data.SetType(PBK_Text_Name);
			data.SetText(szName);
			deqData.push_back(data);
		}
	}
	//group
	if(m_cbGroup.GetStyle() & WS_VISIBLE)
	{
		data.SetType(PBK_Caller_Group);			
		data.SetNumber(m_cbGroup.GetCurSel());
		deqData.push_back(data);
	}
	//birthday
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
	CString strText;
	pos = m_CtrlInfolist.GetHeadPosition();
	while(pos)
	{
		MYCONTROLINFO * pCtlInfo = (MYCONTROLINFO *)m_CtrlInfolist.GetNext(pos);
		if(pCtlInfo)
		{
			if(pCtlInfo->nEntryType !=PBK_Text_Postal)
			{
				POSITION WndPos = pCtlInfo->pControlList.GetHeadPosition();
				while(WndPos)
				{
					CWnd * pCtrlWnd = (CWnd *)pCtlInfo->pControlList.GetNext(WndPos);
					if(pCtrlWnd && pCtrlWnd != &m_edHidePostal)
					{
						pCtrlWnd->GetWindowText(strText);
						if(strText.GetLength() > 0)
						{
							data.SetType(pCtlInfo->nEntryType);
							data.SetText(strText);
							deqData.push_back(data);
						}
						break;
					}
				}
			}
			else //PBK_Text_Postal
			{
				bool bFirst = true;
				strText.Empty();
				POSITION WndPos = pCtlInfo->pControlList.GetHeadPosition();
				while(WndPos)
				{
					CWnd * pCtrlWnd = (CWnd *)pCtlInfo->pControlList.GetNext(WndPos);
					if(pCtrlWnd && pCtrlWnd != &m_edHidePostal)
					{
						CString strtemp;
						pCtrlWnd->GetWindowText(strtemp);
						if(!bFirst) strText += ";";
						strText += strtemp;
					}
					else if(!bFirst)
						strText += ";";
					bFirst = false;

				}
				if(strText.GetLength() > 0)
				{
					data.SetType(pCtlInfo->nEntryType);
					data.SetText(strText);
					deqData.push_back(data);
				}

			}
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
void COtherDlg::xEnableControl(int iState)
{
	bool bEnableCtrl = false;

	CWnd *pWnd[6] = {&m_edName,&m_edFirstName,&m_edLastName,&m_dpBirthday,&m_chBirthday,&m_cbGroup};
	if(iState == ENABLE_SIM){
		for(int i = 1 ; i < 6 ; i ++)
			pWnd[i]->EnableWindow(FALSE);
		if(m_pscNamefirstPos && m_pName_infirstPos)
		{
			m_pName_infirstPos->EnableWindow(TRUE);
			m_pscNamefirstPos->SetWindowText(m_szName);
		}
	}
	else if(iState == ENABLE_ME){
		for(int i = 0 ; i < 6 ; i ++){
			pWnd[i]->EnableWindow(TRUE);
		}
		if(m_pscNamefirstPos) m_pscNamefirstPos->SetWindowText(m_szFirstName);
	}
	else if(iState == ENABLE_VIEW){
		for(int i = 0 ; i < 6 ; i ++){
			pWnd[i]->EnableWindow(FALSE);
		}
		m_rdSIM.EnableWindow(FALSE);
		m_rdME.EnableWindow(FALSE);
	}
	POSITION pos = m_CtrlInfolist.GetHeadPosition();
	while(pos)
	{
		MYCONTROLINFO * pCtlInfo = (MYCONTROLINFO *)m_CtrlInfolist.GetNext(pos);
		if(pCtlInfo)
		{
			POSITION WndPos = pCtlInfo->pControlList.GetHeadPosition();
			while(WndPos)
			{
				CWnd * pCtrlWnd = (CWnd *)pCtlInfo->pControlList.GetNext(WndPos);
				if(pCtrlWnd && pCtrlWnd != &m_edHidePostal)
				{
					if(iState == ENABLE_SIM && m_pFirstPhoneNoWnd !=pCtrlWnd )
						pCtrlWnd->EnableWindow(FALSE);
					else if(iState == ENABLE_ME)
						pCtrlWnd->EnableWindow(TRUE);
					else if(iState == ENABLE_VIEW)
						pCtrlWnd->EnableWindow(FALSE);

				}
			}
		}
	}
	OnChBirthday();

}
void COtherDlg::xSetDataToDlgCtrl()
{
	int nPostalIndex=0;
	CString strName;
	strName.Empty();
	if(m_pData){
		//get data deq
		DEQPHONEDATA *pdeqData; 
		pdeqData = m_pData->GetPhoneData();
		for(DEQPHONEDATA::iterator iter = pdeqData->begin() ; iter != pdeqData->end() ; iter ++){
			if((*iter).GetType() == PBK_Text_Name){		//name
				m_edName.SetWindowText((*iter).GetText());
				strName.Format(_T("%s"),(char *)((*iter).GetText()));
			}
			else if((*iter).GetType() == PBK_Text_FirstName){		//FirstName
				m_edFirstName.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Text_LastName){		//LastName
				m_edLastName.SetWindowText((*iter).GetText());
			}
			else if((*iter).GetType() == PBK_Date){		//birthday
				COleDateTime tm;
				(*iter).GetTime(tm);
				m_dpBirthday.SetTime(tm);
				m_chBirthday.SetCheck(1);
				if(m_iMode == 1)
					m_dpBirthday.EnableWindow(TRUE);
			}	
			else if((*iter).GetType() == PBK_Caller_Group){		//group
				if(strcmp(m_pData->GetStorageType(),SIM_NAME) == 0)
					m_cbGroup.SetCurSel(0);
				else
					m_cbGroup.SetCurSel((*iter).GetNumber());
			}
			else
			{
				bool bFind = false;
				CString strTemp;
				if(!m_iIsME && (*iter).GetType() == PBK_Number_General)
				{
					if(m_pFirstPhoneNoWnd)
					{
						m_pFirstPhoneNoWnd->GetWindowText(strTemp);
						if(strTemp.IsEmpty())
						{
							m_pFirstPhoneNoWnd->SetWindowText((*iter).GetText());
							bFind = true;
						}
					}
				}
				else
				{
					int nCurPostalCtrlIndex = 0;
					POSITION pos = m_CtrlInfolist.GetHeadPosition();
					while(pos)
					{
						MYCONTROLINFO * pCtlInfo = (MYCONTROLINFO *)m_CtrlInfolist.GetNext(pos);
						if(pCtlInfo)
						{
							if((*iter).GetType() == pCtlInfo->nEntryType)
							{
								if((*iter).GetType() == PBK_Text_Postal)
								{
									if(nCurPostalCtrlIndex < nPostalIndex)
									{
										nCurPostalCtrlIndex++;
										continue;
									}

								/*	if(pCtlInfo->pControlList.GetCount() ==1)
									{
										CWnd * pCtrlWnd = (CWnd *)pCtlInfo->pControlList.GetHead();
										if(pCtrlWnd && pCtrlWnd != &m_edHidePostal)
											pCtrlWnd->SetWindowText((TCHAR*)((*iter).GetText()));

									}
									else*/
									{
										CStringList strList;
										AnaString((TCHAR*)((*iter).GetText()),&strList);
										POSITION posWnd,posString;
										posWnd = pCtlInfo->pControlList.GetHeadPosition();
										posString = strList.GetHeadPosition();
										while(posWnd && posString)
										{
											CWnd * pCtrlWnd = (CWnd *)pCtlInfo->pControlList.GetNext(posWnd);
											CString str = strList.GetNext(posString);
											if(pCtrlWnd && pCtrlWnd != &m_edHidePostal)
												pCtrlWnd->SetWindowText(str);
										}
									}
									bFind = true;
									nCurPostalCtrlIndex++;
									nPostalIndex++;

								}
								else
								{
									POSITION WndPos = pCtlInfo->pControlList.GetHeadPosition();
									while(WndPos)
									{
										CWnd * pCtrlWnd = (CWnd *)pCtlInfo->pControlList.GetNext(WndPos);
										if(pCtrlWnd && pCtrlWnd != &m_edHidePostal)
										{
											pCtrlWnd->GetWindowText(strTemp);
											if(strTemp.IsEmpty())
											{
												pCtrlWnd->SetWindowText((*iter).GetText());
												bFind = true;
												break;
											}
										}
									}
								}
							}//if((*iter).GetType() == pCtlInfo->nEntryType)
						}//if(pCtlInfo)
					}//while(pos)
				}
				if(!bFind)
					m_deqDetainData.push_back((*iter));
			}//else

		} //for

		if(m_pName_infirstPos && m_pName_infirstPos->GetStyle()& WS_VISIBLE)
		{
			CString strFirstName,strLastName;
			strFirstName.Empty();
			strLastName.Empty();
			m_edFirstName.GetWindowText(strFirstName);
			m_edLastName.GetWindowText(strLastName);
			if(strFirstName.IsEmpty() && strLastName.IsEmpty() && strName.IsEmpty()== false)
			{
				m_pName_infirstPos->SetWindowText(strName);
			}
		}
	}//if
	xSetSupportColumnLength();

}
void COtherDlg::xSetComboItem(LPCTSTR szProfile)
{
	int iCount = 0;
	TCHAR szKey[MAX_PATH];
	TCHAR szTextID[MAX_PATH];
	TCHAR szText[MAX_PATH];
	al_GetSettingInt(_T("Group"),_T("nCount"),szProfile,iCount);
	for(int i = 1 ; i <= iCount ; i ++)
	{
		wsprintf(szKey,_T("Group%d") ,i);
		if(al_GetSettingString(_T("Group"),szKey,szProfile,szTextID))
		{
			al_GetSettingString(_T("public"),szTextID,theApp.m_szRes,szText);
			m_cbGroup.AddString(szText);
		}
	}
	m_cbGroup.SetCurSel(0);

}
GSM_EntryType COtherDlg::GetEntryTypeID(TCHAR* szTemp)
{
	GSM_EntryType EntryType = (GSM_EntryType)0;
	if(_tcslen(szTemp)<=0)
		return EntryType;
	if(_tcsicmp(szTemp,_T("PBK_Number_General")) == 0)
		return PBK_Number_General;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Mobile")) == 0)
		return PBK_Number_Mobile;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Work")) == 0)
		return PBK_Number_Work;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Fax")) == 0)
		return PBK_Number_Fax;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Home")) == 0)
		return PBK_Number_Home;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Pager")) == 0)
		return PBK_Number_Pager;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Other")) == 0)
		return PBK_Number_Other;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Note")) == 0)
		return PBK_Text_Note;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Postal")) == 0)
		return PBK_Text_Postal;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Email")) == 0)
		return PBK_Text_Email;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Email2")) == 0)
		return PBK_Text_Email2;
	else if(_tcsicmp(szTemp,_T("PBK_Text_URL")) == 0)
		return PBK_Text_URL;
	else if(_tcsicmp(szTemp,_T("PBK_Date")) == 0)
		return PBK_Date;
	else if(_tcsicmp(szTemp,_T("PBK_Caller_Group")) == 0)
		return PBK_Caller_Group;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Name")) == 0)
		return PBK_Text_Name;
	else if(_tcsicmp(szTemp,_T("PBK_Text_LastName")) == 0)
		return PBK_Text_LastName;
	else if(_tcsicmp(szTemp,_T("PBK_Text_FirstName")) == 0)
		return PBK_Text_FirstName;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Company")) == 0)
		return PBK_Text_Company;
	else if(_tcsicmp(szTemp,_T("PBK_Text_JobTitle")) == 0)
		return PBK_Text_JobTitle;
	else if(_tcsicmp(szTemp,_T("PBK_Category")) == 0)
		return PBK_Category;
	else if(_tcsicmp(szTemp,_T("PBK_Private")) == 0)
		return PBK_Private;
	else if(_tcsicmp(szTemp,_T("PBK_Text_StreetAddress")) == 0)
		return PBK_Text_StreetAddress;
	else if(_tcsicmp(szTemp,_T("PBK_Text_City")) == 0)
		return PBK_Text_City;
	else if(_tcsicmp(szTemp,_T("PBK_Text_State")) == 0)
		return PBK_Text_State;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Zip")) == 0)
		return PBK_Text_Zip;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Country")) == 0)
		return PBK_Text_Country;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Custom1")) == 0)
		return PBK_Text_Custom1;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Custom2")) == 0)
		return PBK_Text_Custom2;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Custom3")) == 0)
		return PBK_Text_Custom3;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Custom4")) == 0)
		return PBK_Text_Custom4;
	else if(_tcsicmp(szTemp,_T("PBK_RingtoneID")) == 0)
		return PBK_RingtoneID;
	else if(_tcsicmp(szTemp,_T("PBK_RingtoneFileSystemID")) == 0)
		return PBK_RingtoneFileSystemID;
	else if(_tcsicmp(szTemp,_T("PBK_PictureID")) == 0)
		return PBK_PictureID;
	else if(_tcsicmp(szTemp,_T("PBK_SMSListID")) == 0)
		return PBK_SMSListID;
	else if(_tcsicmp(szTemp,_T("PBK_Text_UserID")) == 0)
		return PBK_Text_UserID;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Picture")) == 0)
		return PBK_Text_Picture;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Ring")) == 0)
		return PBK_Text_Ring;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Sex")) == 0)
		return PBK_Number_Sex;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Light")) == 0)
		return PBK_Number_Light;
	else if(_tcsicmp(szTemp,_T("PBK_Push_Talk")) == 0)
		return PBK_Push_Talk;
	else if(_tcsicmp(szTemp,_T("PBK_Caller_Group_Text")) == 0)
		return PBK_Caller_Group_Text;
	else if(_tcsicmp(szTemp,_T("PBK_Text_StreetAddress2")) == 0)
		return PBK_Text_StreetAddress2;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Nickname")) == 0)
		return PBK_Text_Nickname;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Mobile_Home")) == 0)
		return PBK_Number_Mobile_Home;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Mobile_Work")) == 0)
		return PBK_Number_Mobile_Work;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Fax_Home")) == 0)
		return PBK_Number_Fax_Home;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Fax_Work")) == 0)
		return PBK_Number_Fax_Work;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Email_Home")) == 0)
		return PBK_Text_Email_Home;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Email_Mobile")) == 0)
		return PBK_Text_Email_Mobile;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Email_Unknown")) == 0)
		return PBK_Text_Email_Unknown;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Email_Work")) == 0)
		return PBK_Text_Email_Work;
	else if(_tcsicmp(szTemp,_T("PBK_Text_URL_Home")) == 0)
		return PBK_Text_URL_Home;
	else if(_tcsicmp(szTemp,_T("PBK_Text_URL_Work")) == 0)
		return PBK_Text_URL_Work;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Postal_Home")) == 0)
		return PBK_Text_Postal_Home;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Postal_Work")) == 0)
		return PBK_Text_Postal_Work;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Pager_Home")) == 0)
		return PBK_Number_Pager_Home;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Pager_Work")) == 0)
		return PBK_Number_Pager_Work;
	else if(_tcsicmp(szTemp,_T("PBK_Number_VideoCall")) == 0)
		return PBK_Number_VideoCall;
	else if(_tcsicmp(szTemp,_T("PBK_Number_VideoCall_Home")) == 0)
		return PBK_Number_VideoCall_Home;
	else if(_tcsicmp(szTemp,_T("PBK_Number_VideoCall_Work")) == 0)
		return PBK_Number_VideoCall_Work;
	else if(_tcsicmp(szTemp,_T("PBK_Text_MiddleName")) == 0)
		return PBK_Text_MiddleName;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Suffix")) == 0)
		return PBK_Text_Suffix;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Title")) == 0)
		return PBK_Text_Title;
	//new Add
	else if(_tcsicmp(szTemp,_T("PBK_Number_Assistant")) == 0)
		return PBK_Number_Assistant;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Business")) == 0)
		return PBK_Number_Business;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Callback")) == 0)
		return PBK_Number_Callback;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Car")) == 0)
		return PBK_Number_Car;
	else if(_tcsicmp(szTemp,_T("PBK_Number_ISDN")) == 0)
		return PBK_Number_ISDN;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Primary")) == 0)
		return PBK_Number_Primary;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Radio")) == 0)
		return PBK_Number_Radio;
	else if(_tcsicmp(szTemp,_T("PBK_Number_Telix")) == 0)
		return PBK_Number_Telix;
	else if(_tcsicmp(szTemp,_T("PBK_Number_TTYTDD")) == 0)
		return PBK_Number_TTYTDD;

	else if(_tcsicmp(szTemp,_T("PBK_Text_Department")) == 0)
		return PBK_Text_Department;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Office")) == 0)
		return PBK_Text_Office;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Profession")) == 0)
		return PBK_Text_Profession;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Manager_Name")) == 0)
		return PBK_Text_Manager_Name;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Assistant_Name")) == 0)
		return PBK_Text_Assistant_Name;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Spouse_Name")) == 0)
		return PBK_Text_Spouse_Name;
	else if(_tcsicmp(szTemp,_T("PBK_Date_Anniversary")) == 0)
		return PBK_Date_Anniversary;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Directory_Server")) == 0)
		return PBK_Text_Directory_Server;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Email_alias")) == 0)
		return PBK_Text_Email_alias;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Internet_Address")) == 0)
		return PBK_Text_Internet_Address;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Children")) == 0)
		return PBK_Text_Children;

	else if(_tcsicmp(szTemp,_T("PBK_Text_StreetAddress_Work")) == 0)
		return PBK_Text_StreetAddress_Work;
	else if(_tcsicmp(szTemp,_T("PBK_Text_City_Work")) == 0)
		return PBK_Text_City_Work;
	else if(_tcsicmp(szTemp,_T("PBK_Text_State_Work")) == 0)
		return PBK_Text_State_Work;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Zip_Work")) == 0)
		return PBK_Text_Zip_Work;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Country_Work")) == 0)
		return PBK_Text_Country_Work;

	else if(_tcsicmp(szTemp,_T("PBK_Text_StreetAddress_Home")) == 0)
		return PBK_Text_StreetAddress_Home;
	else if(_tcsicmp(szTemp,_T("PBK_Text_City_Home")) == 0)
		return PBK_Text_City_Home;
	else if(_tcsicmp(szTemp,_T("PBK_Text_State_Home")) == 0)
		return PBK_Text_State_Home;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Zip_Home")) == 0)
		return PBK_Text_Zip_Home;
	else if(_tcsicmp(szTemp,_T("PBK_Text_Country_Home")) == 0)
		return PBK_Text_Country_Home;
	else if(_tcsicmp(szTemp,_T("PBK_Text_IMID")) == 0)
		return PBK_Text_IMID;
	return EntryType;

}
void COtherDlg::xSetSupportColumnLength()
{
	DEQPHBOUNDARY *pBoundary = m_se.GetPhBoundary();

	for(DEQPHBOUNDARY::iterator iter = pBoundary->begin() ; iter != pBoundary->end() ; iter ++)
	{
		switch((*iter).GetType())
		{
		case PBK_Text_Name:
			m_edName.SetLimitText((*iter).GetBoundary());
			m_edLastName.SetLimitText((*iter).GetBoundary());
			break;
		case PBK_Text_LastName:
			m_edFirstName.SetLimitText((*iter).GetBoundary());
			break;
		case PBK_Number_Mobile:
			m_edPhoneNo1.SetLimitText((*iter).GetBoundary());
			m_edPhoneNo2.SetLimitText((*iter).GetBoundary());
			m_edPhoneNo3.SetLimitText((*iter).GetBoundary());
			m_edPhoneNo4.SetLimitText((*iter).GetBoundary());
			m_edPhoneNo5.SetLimitText((*iter).GetBoundary());
			m_edPhoneNo6.SetLimitText((*iter).GetBoundary());
			m_edPhoneNo7.SetLimitText((*iter).GetBoundary());
			m_edPhoneNo8.SetLimitText((*iter).GetBoundary());
			m_edPhoneNo9.SetLimitText((*iter).GetBoundary());
			m_edPhoneNo10.SetLimitText((*iter).GetBoundary());
			break;
		case PBK_Text_Email:
			m_edText1.SetLimitText((*iter).GetBoundary());
			m_edText2.SetLimitText((*iter).GetBoundary());
			m_edText3.SetLimitText((*iter).GetBoundary());
			m_edText4.SetLimitText((*iter).GetBoundary());
			m_edText5.SetLimitText((*iter).GetBoundary());
			m_edText6.SetLimitText((*iter).GetBoundary());
			m_edText7.SetLimitText((*iter).GetBoundary());
			m_edText8.SetLimitText((*iter).GetBoundary());
			m_edText9.SetLimitText((*iter).GetBoundary());
			m_edText10.SetLimitText((*iter).GetBoundary());
			m_edText11.SetLimitText((*iter).GetBoundary());
			m_edText12.SetLimitText((*iter).GetBoundary());
			m_edText13.SetLimitText((*iter).GetBoundary());
			m_edText14.SetLimitText((*iter).GetBoundary());
			m_edText15.SetLimitText((*iter).GetBoundary());
			break;
		}
	}
}
