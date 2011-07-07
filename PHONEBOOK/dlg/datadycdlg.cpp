// DataDycDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "DataDycDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataDycDlg dialog
enum{
	CB_GENERAL = 0,
	CB_MOBILE = 1,
	CB_HOME,
	CB_OFFICE,
	CB_FAX
};

enum{
	CB_EMAIL = 0,
	CB_ADDRESS = 1,
	CB_NOTE,
	CB_WEB
};

CDataDycDlg::CDataDycDlg(CWnd* pParent /*=NULL*/)
	: CBaseDlg(IDD_DATA_DYC, pParent)
{
	//{{AFX_DATA_INIT(CDataDycDlg)
	//}}AFX_DATA_INIT
	m_hFont = NULL;
}
CDataDycDlg::~CDataDycDlg()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}

void CDataDycDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDataDycDlg)
	DDX_Control(pDX, IDC_SC_GROUP, m_scGroup);
	DDX_Control(pDX, IDC_COMBO_GROUP, m_cbGroup);
	DDX_Control(pDX, IDC_SC_DOT11, m_scDot11);
	DDX_Control(pDX, IDOK, m_rbnOK);
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDC_SC_TITLE, m_scTitle);
	DDX_Control(pDX, IDC_SC_STORAGE, m_scStorage);
	DDX_Control(pDX, IDC_SC_NUMBER9, m_scNum9);
	DDX_Control(pDX, IDC_SC_NUMBER8, m_scNum8);
	DDX_Control(pDX, IDC_SC_NUMBER7, m_scNum7);
	DDX_Control(pDX, IDC_SC_NUMBER6, m_scNum6);
	DDX_Control(pDX, IDC_SC_NUMBER5, m_scNum5);
	DDX_Control(pDX, IDC_SC_NUMBER4, m_scNum4);
	DDX_Control(pDX, IDC_SC_NUMBER3, m_scNum3);
	DDX_Control(pDX, IDC_SC_NUMBER2, m_scNum2);
	DDX_Control(pDX, IDC_SC_NUMBER1, m_scNum1);
	DDX_Control(pDX, IDC_SC_NAME, m_scName);
	DDX_Control(pDX, IDC_SC_DOT9, m_scDot9);
	DDX_Control(pDX, IDC_SC_DOT8, m_scDot8);
	DDX_Control(pDX, IDC_SC_DOT7, m_scDot7);
	DDX_Control(pDX, IDC_SC_DOT6, m_scDot6);
	DDX_Control(pDX, IDC_SC_DOT5, m_scDot5);
	DDX_Control(pDX, IDC_SC_DOT4, m_scDot4);
	DDX_Control(pDX, IDC_SC_DOT3, m_scDot3);
	DDX_Control(pDX, IDC_SC_DOT2, m_scDot2);
	DDX_Control(pDX, IDC_SC_DOT10, m_scDot10);
	DDX_Control(pDX, IDC_SC_DOT1, m_scDot1);
	DDX_Control(pDX, IDC_SC_DEFAULT, m_scDefault);
	DDX_Control(pDX, IDC_EDIT9, m_ed9);
	DDX_Control(pDX, IDC_EDIT8, m_ed8);
	DDX_Control(pDX, IDC_EDIT7, m_ed7);
	DDX_Control(pDX, IDC_EDIT6, m_ed6);
	DDX_Control(pDX, IDC_EDIT5, m_ed5);
	DDX_Control(pDX, IDC_EDIT4, m_ed4);
	DDX_Control(pDX, IDC_EDIT3, m_ed3);
	DDX_Control(pDX, IDC_EDIT2, m_ed2);
	DDX_Control(pDX, IDC_ED_NAME, m_edName);
	DDX_Control(pDX, IDC_COMBO9, m_cb9);
	DDX_Control(pDX, IDC_COMBO8, m_cb8);
	DDX_Control(pDX, IDC_COMBO7, m_cb7);
	DDX_Control(pDX, IDC_COMBO6, m_cb6);
	DDX_Control(pDX, IDC_COMBO5, m_cb5);
	DDX_Control(pDX, IDC_COMBO4, m_cb4);
	DDX_Control(pDX, IDC_COMBO3, m_cb3);
	DDX_Control(pDX, IDC_COMBO2, m_cb2);
	DDX_Control(pDX, IDC_COMBO_DEFAULT, m_cbDefault);
	DDX_Control(pDX, IDC_CHECK9, m_ch9);
	DDX_Control(pDX, IDC_CHECK8, m_ch8);
	DDX_Control(pDX, IDC_CHECK7, m_ch7);
	DDX_Control(pDX, IDC_CHECK6, m_ch6);
	DDX_Control(pDX, IDC_CHECK5, m_ch5);
	DDX_Control(pDX, IDC_CHECK4, m_ch4);
	DDX_Control(pDX, IDC_CHECK3, m_ch3);
	DDX_Control(pDX, IDC_RD_SIM, m_rdSIM);
	DDX_Control(pDX, IDC_RD_PHONE, m_rdME);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDataDycDlg, CBaseDlg)
	//{{AFX_MSG_MAP(CDataDycDlg)
	ON_BN_CLICKED(IDC_CHECK3, OnCheck3)
	ON_BN_CLICKED(IDC_CHECK4, OnCheck4)
	ON_BN_CLICKED(IDC_CHECK5, OnCheck5)
	ON_BN_CLICKED(IDC_CHECK6, OnCheck6)
	ON_BN_CLICKED(IDC_CHECK7, OnCheck7)
	ON_BN_CLICKED(IDC_CHECK8, OnCheck8)
	ON_BN_CLICKED(IDC_CHECK9, OnCheck9)
	ON_BN_CLICKED(IDC_RD_SIM, OnRdSim)
	ON_BN_CLICKED(IDC_RD_PHONE, OnRdMobile)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnSelchangeCombo2)
	ON_CBN_SELCHANGE(IDC_COMBO3, OnSelchangeCombo3)
	ON_CBN_SELCHANGE(IDC_COMBO4, OnSelchangeCombo4)
	ON_CBN_SELCHANGE(IDC_COMBO5, OnSelchangeCombo5)
	ON_CBN_SELCHANGE(IDC_COMBO6, OnSelchangeCombo6)
	ON_CBN_SELCHANGE(IDC_COMBO7, OnSelchangeCombo7)
	ON_CBN_SELCHANGE(IDC_COMBO8, OnSelchangeCombo8)
	ON_CBN_SELCHANGE(IDC_COMBO9, OnSelchangeCombo9)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataDycDlg message handlers


void CDataDycDlg::OnCheck3() 
{
	if(m_ch3.GetCheck()){
		m_cb3.EnableWindow(TRUE);
		m_ed3.EnableWindow(TRUE);
	}
	else{
		m_cb3.EnableWindow(FALSE);
		m_ed3.EnableWindow(FALSE);
	}
	xSetDefaultCombo();	
	xSetSupportColumnLength();

}

void CDataDycDlg::OnCheck4() 
{
	if(m_ch4.GetCheck()){
		m_cb4.EnableWindow(TRUE);
		m_ed4.EnableWindow(TRUE);
	}
	else{
		m_cb4.EnableWindow(FALSE);
		m_ed4.EnableWindow(FALSE);
	}
	xSetDefaultCombo();	
	xSetSupportColumnLength();
	
}

void CDataDycDlg::OnCheck5() 
{
	if(m_ch5.GetCheck()){
		m_cb5.EnableWindow(TRUE);
		m_ed5.EnableWindow(TRUE);
	}
	else{
		m_cb5.EnableWindow(FALSE);
		m_ed5.EnableWindow(FALSE);
	}
	xSetDefaultCombo();	
	xSetSupportColumnLength();
}

void CDataDycDlg::OnCheck6() 
{
	if(m_ch6.GetCheck()){
		m_cb6.EnableWindow(TRUE);
		m_ed6.EnableWindow(TRUE);
	}
	else{
		m_cb6.EnableWindow(FALSE);
		m_ed6.EnableWindow(FALSE);
	}
	xSetDefaultCombo();	
	xSetSupportColumnLength();
}

void CDataDycDlg::OnCheck7() 
{
	if(m_ch7.GetCheck()){
		m_cb7.EnableWindow(TRUE);
		m_ed7.EnableWindow(TRUE);
	}
	else{
		m_cb7.EnableWindow(FALSE);
		m_ed7.EnableWindow(FALSE);
	}
	xSetSupportColumnLength();
}

void CDataDycDlg::OnCheck8() 
{
	if(m_ch8.GetCheck()){
		m_cb8.EnableWindow(TRUE);
		m_ed8.EnableWindow(TRUE);
	}
	else{
		m_cb8.EnableWindow(FALSE);
		m_ed8.EnableWindow(FALSE);
	}
	xSetSupportColumnLength();
}

void CDataDycDlg::OnCheck9() 
{
	if(m_ch9.GetCheck()){
		m_cb9.EnableWindow(TRUE);
		m_ed9.EnableWindow(TRUE);
	}
	else{
		m_cb9.EnableWindow(FALSE);
		m_ed9.EnableWindow(FALSE);
	}
	xSetSupportColumnLength();
}

void CDataDycDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CBaseDlg::OnCancel();
}

void CDataDycDlg::OnOK() 
{
	TCHAR szTxt[MAX_PATH];
	m_ed2.GetWindowText(szTxt,MAX_PATH);
	int iLength = _tcslen(szTxt);
	if(iLength == 0){
		if(al_GetSettingString(_T("public"),_T("IDS_ERR_NONUMBER"),theApp.m_szRes,szTxt))
			AfxMessageBox(szTxt);
		return;
	}

	CPhoneData data;
	DEQPHONEDATA deqData;
	//get sel default 
	int iSel = m_cbDefault.GetCurSel();
	m_cbDefault.GetLBText(iSel,szTxt);
	int iIndex = _ttoi(&szTxt[0]);
	memset(szTxt,0,sizeof(TCHAR) * MAX_PATH);
	//get name
	//set data type
	data.SetType(PBK_Text_Name);
	//get data from edit control 
	m_edName.GetWindowText(szTxt,MAX_PATH);
	//set data to CPhoneData object
	data.SetText(szTxt);
	//push in the daque
	deqData.push_back(data);



	CCheckEx *pCh[7] = {&m_ch3,&m_ch4,&m_ch5,&m_ch6,&m_ch7,&m_ch8,&m_ch9};
	CComboBox *pCb[7] = {&m_cb3,&m_cb4,&m_cb5,&m_cb6,&m_cb7,&m_cb8,&m_cb9};
	CMyEdit *pEd[7] = {&m_ed3,&m_ed4,&m_ed5,&m_ed6,&m_ed7,&m_ed8,&m_ed9};

	//add default to deque
	if(iIndex == 2){	//combobox 2
		switch(m_cb2.GetCurSel())
		{
		case 0:
			data.SetType(PBK_Number_General);
			break;
		case 1:
			data.SetType(PBK_Number_Mobile);
			break;
		case 2:
			data.SetType(PBK_Number_Home);
			break;
		case 3:
			data.SetType(PBK_Number_Work);
			break;
		case 4:
			data.SetType(PBK_Number_Fax);
			break;
		}
		m_ed2.GetWindowText(szTxt,MAX_PATH);
	}
	else{	//other
		if(pCh[iIndex - 3]->IsWindowEnabled() && pCh[iIndex - 3]->GetCheck()){
			switch(pCb[iIndex - 3]->GetCurSel())
			{
			case 0:
				data.SetType(PBK_Number_General);
				break;
			case 1:
				data.SetType(PBK_Number_Mobile);
				break;
			case 2:
				data.SetType(PBK_Number_Home);
				break;
			case 3:
				data.SetType(PBK_Number_Work);
				break;
			case 4:
				data.SetType(PBK_Number_Fax);
				break;
			}
			pEd[iIndex - 3]->GetWindowText(szTxt,MAX_PATH);
		}
	}
	data.SetText(szTxt);
	deqData.push_back(data);

	//add other data to deque
	for(int i = 0 ; i < 9 ; i ++){
		bool bRet = false;
		data.initialize();
		if( i == 0){	//group
			data.SetType(PBK_Caller_Group);			
			data.SetNumber(m_cbGroup.GetCurSel());
			bRet = true;
		}
		else if( i == 1 && iIndex != 2){	//default number
			switch(m_cb2.GetCurSel())
			{
			case 0:
				data.SetType(PBK_Number_General);
				break;
			case 1:
				data.SetType(PBK_Number_Mobile);
				break;
			case 2:
				data.SetType(PBK_Number_Home);
				break;
			case 3:
				data.SetType(PBK_Number_Work);
				break;
			case 4:
				data.SetType(PBK_Number_Fax);
				break;
			}
			m_ed2.GetWindowText(szTxt,MAX_PATH);
			data.SetText(szTxt);
			bRet = true;
		}
		else if(iIndex != i + 1){	//other number and not default
			//check checkbox
			if(pCh[i - 2]->IsWindowEnabled() && pCh[i - 2]->GetCheck()){
				if(i < 6){
					switch(pCb[i - 2]->GetCurSel())
					{
					case 0:
						data.SetType(PBK_Number_General);
						break;
					case 1:
						data.SetType(PBK_Number_Mobile);
						break;
					case 2:
						data.SetType(PBK_Number_Home);
						break;
					case 3:
						data.SetType(PBK_Number_Work);
						break;
					case 4:
						data.SetType(PBK_Number_Fax);
						break;
					}
				}
				else{
					switch(pCb[i - 2]->GetCurSel())
					{
					case 0:
						data.SetType(PBK_Text_Email);
						break;
					case 1:
						data.SetType(PBK_Text_Postal);
						break;
					case 2:
						data.SetType(PBK_Text_Note);
						break;
					case 3:
						data.SetType(PBK_Text_URL);
						break;
					}
				}
				pEd[i - 2]->GetWindowText(szTxt,MAX_PATH);
				data.SetText(szTxt);
				bRet = true;
			}
		}
		if(bRet)
			deqData.push_back(data);
	}
	for(DEQPHONEDATA::iterator iterDetain = m_deqDetainData.begin() ; iterDetain != m_deqDetainData.end() ; iterDetain++){
		deqData.push_back((*iterDetain));
	}

	m_pData->SetPhoneData(deqData);

	if(m_rdSIM.GetCheck())
		m_pData->SetStorageType(SIM_NAME);
	else
		m_pData->SetStorageType(MEMORY_NAME);

	CBaseDlg::OnOK();
}

void CDataDycDlg::OnRdSim() 
{
	m_rdSIM.SetCheck(1);
	m_rdME.SetCheck(0);	

	for(int i = 2 ; i < 9 ; i ++)
		xEnableControl(i,FALSE);
	m_cbDefault.EnableWindow(FALSE);
	xSetCombo2Item();
}

void CDataDycDlg::OnRdMobile() 
{
	m_rdSIM.SetCheck(0);
	m_rdME.SetCheck(1);	

	for(int i = 2 ; i < 9 ; i ++)
		xEnableControl(i,TRUE);
	m_cbDefault.EnableWindow(TRUE);
	xSetCombo2Item();
}

void CDataDycDlg::OnSelchangeCombo2() 
{
	// TODO: Add your control notification handler code here
	xSetSupportColumnLength();
	xSetDefaultCombo();	
}

void CDataDycDlg::OnSelchangeCombo3() 
{
	// TODO: Add your control notification handler code here
	xSetSupportColumnLength();
	xSetDefaultCombo();	
	
}

void CDataDycDlg::OnSelchangeCombo4() 
{
	// TODO: Add your control notification handler code here
	xSetSupportColumnLength();
	xSetDefaultCombo();	
	
}

void CDataDycDlg::OnSelchangeCombo5() 
{
	// TODO: Add your control notification handler code here
	xSetSupportColumnLength();
	xSetDefaultCombo();	
	
}

void CDataDycDlg::OnSelchangeCombo6() 
{
	// TODO: Add your control notification handler code here
	xSetSupportColumnLength();
	xSetDefaultCombo();	
}

void CDataDycDlg::OnSelchangeCombo7() 
{
	// TODO: Add your control notification handler code here
	xSetSupportColumnLength();
	
}

void CDataDycDlg::OnSelchangeCombo8() 
{
	// TODO: Add your control notification handler code here
	xSetSupportColumnLength();
	
}

void CDataDycDlg::OnSelchangeCombo9() 
{
	// TODO: Add your control notification handler code here
	xSetSupportColumnLength();
	
}

BOOL CDataDycDlg::OnInitDialog() 
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
	sProfile += Tstring(_T("organize\\DataDycDlg.ini"));
	//load static
	CStaticEx *pStaticEx[25] = {&m_scTitle,&m_scName,&m_scDefault,&m_scStorage,&m_scNum1,&m_scNum2
		,&m_scNum3,&m_scNum4,&m_scNum5,&m_scNum6,&m_scNum7,&m_scNum8,&m_scNum9,&m_scDot1,
		&m_scDot2,&m_scDot3,&m_scDot4,&m_scDot5,&m_scDot6,&m_scDot7,&m_scDot8,&m_scDot9,&m_scDot10
		,&m_scGroup,&m_scDot11};
	TCHAR szScSec[25][32] = {_T("title_sc"),_T("name_sc"),_T("default_sc"),_T("storage_sc"),_T("num1_sc"),_T("num2_sc")
		,_T("num3_sc"),_T("num4_sc"),_T("num5_sc"),_T("num6_sc"),_T("num7_sc"),_T("num8_sc"),_T("num9_sc"),_T("dot1_sc"),_T("dot2_sc")
		,_T("dot3_sc"),_T("dot4_sc"),_T("dot5_sc"),_T("dot6_sc"),_T("dot7_sc"),_T("dot8_sc"),_T("dot9_sc"),_T("dot10_sc")
		,_T("group_sc"),_T("dot11_sc")};
	TCHAR szScID[25][32] = {_T(""),_T("IDS_NAME"),_T("IDS_SETDEFAULT"),_T("IDS_STORAGE"),_T("IDS_1"),_T("IDS_2")
		,_T("IDS_3"),_T("IDS_4"),_T("IDS_5"),_T("IDS_6"),_T("IDS_7"),_T("IDS_8"),_T("IDS_9"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT")
		,_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_DOT"),_T("IDS_GROUP"),_T("IDS_DOT")};
	if(m_iMode == 0)
		_tcscpy(szScID[0],_T("IDS_NEWCONTACT"));
	else if(m_iMode == 1)
		_tcscpy(szScID[0],_T("IDS_EDITCONTACT"));
	else if(m_iMode == 2)
		_tcscpy(szScID[0],_T("IDS_VIEWCONTACT"));
	for(int i = 0 ; i < 25 ; i ++)
		GetStaticFromSetting(pStaticEx[i],szScSec[i],szScID[i],const_cast<TCHAR*>(sProfile.c_str()));

	//load button 
	CRescaleButton	*pButton[2] = {&m_rbnOK,&m_rbnCancel};
	TCHAR szBnSec[2][32] = {_T("ok"),_T("cancel")};
	TCHAR szBnID[2][32] = {_T("IDS_N_OK"),_T("IDS_CANCEL")};
	for(i = 0 ; i < 2 ; i ++)
		GetButtonFromSetting(pButton[i],szBnSec[i],szBnID[i],0,const_cast<TCHAR*>(sProfile.c_str()));

	//load checkbox
	CCheckEx2	*pCheck[7] = {&m_ch3,&m_ch4,&m_ch5,&m_ch6,&m_ch7,&m_ch8,&m_ch9};
	TCHAR szChSec[7][32] = {_T("ch3"),_T("ch4"),_T("ch5"),_T("ch6"),_T("ch7"),_T("ch8"),_T("ch9")};
	for(i = 0 ; i < 7 ; i ++){
		GetCheckFromSetting(pCheck[i],szChSec[i],0,const_cast<TCHAR*>(sProfile.c_str()));
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
	
	//set other control position and size
	CWnd *pWnd[19] = {&m_cbDefault,&m_cb2,&m_cb3,&m_cb4,&m_cb5,&m_cb6,&m_cb7,&m_cb8,&m_cb9
		,&m_edName,&m_ed2,&m_ed3,&m_ed4,&m_ed5,&m_ed6,&m_ed7,&m_ed8,&m_ed9,&m_cbGroup};
	TCHAR szWndSec[19][32] = {_T("default_cb"),_T("cb2"),_T("cb3"),_T("cb4"),_T("cb5"),_T("cb6"),_T("cb7"),_T("cb8"),_T("cb9")
		,_T("name_ed"),_T("ed2"),_T("ed3"),_T("ed4"),_T("ed5"),_T("ed6"),_T("ed7"),_T("ed8"),_T("ed9"),_T("group_cb")};
	for(i = 0 ; i < 19 ; i ++){
		CRect rect;
		al_GetSettingRect(szWndSec[i],_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect);
		pWnd[i]->MoveWindow(rect);
		if(pFont)pWnd[i]->SetFont(pFont);

	}
	
	//set group size and pos
	CRect rect;
	if(al_GetSettingRect(_T("group"),_T("rect"),const_cast<TCHAR *>(sProfile.c_str()),rect))
		GetDlgItem(IDC_GP)->MoveWindow(rect);

	//get panel color
	al_GetSettingColor(_T("panel"),_T("color"),const_cast<TCHAR *>(sProfile.c_str()),m_crBg);

	CRect rcTitle;
	CalcStaticRealRect(&m_scTitle,rcTitle);
	int iSpace = 0;
	al_GetSettingInt(_T("title_sc"),_T("space"),const_cast<TCHAR*>(sProfile.c_str()),iSpace);
	rcTitle.right += iSpace * 2;
	m_scTitle.GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.right = rect.left + rcTitle.Width();
	m_scTitle.MoveWindow(rect);

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

	m_ed2.GetWindowRect(&rc);
	ScreenToClient(&rc);
	
	rcBtn1.OffsetRect(rc.right - rcBtn1.right  ,0);
	m_rbnCancel.MoveWindow(rcBtn1);

	rcBtn2.OffsetRect(rcBtn1.left - 20 - rcBtn2.right ,0);
	m_rbnOK.MoveWindow(rcBtn2);

	//Set combobox string
	xSetComboItem();
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

		//gray all control
		for( i = 0 ; i < 2 ; i ++){
			pRadio[i]->EnableWindow(FALSE);
			pRadio[i]->SetCheck(0);
		}
		for( i = 0 ; i < 19 ; i ++ )
			pWnd[i]->EnableWindow(FALSE);
		for( i = 0 ; i < 7 ; i ++)
			pCheck[i]->EnableWindow(FALSE);

	}
	if(bRet)
		SetWindowText(szText);

	if(((CPhoneBookApp*)afxGetApp())->m_bNotSupportSM && m_iMode != 2)
	{
		m_rdME.EnableWindow(FALSE);
		m_rdSIM.EnableWindow(FALSE);
		OnRdMobile();
	}
	xSetDefaultCombo();	
	m_cbDefault.SetCurSel(0);

	xSetSupportColumnLength();
	xSetCombo2Item();
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


/*
void CDataDycDlg::SetMode(int iMode)
{
	m_iMode = iMode;
}

void CDataDycDlg::SetData(CCardPanel *pData)
{
	m_pData = pData;
}

void CDataDycDlg::SetIsME(int iFlag)
{
	m_iIsME = iFlag;
}
*/
void CDataDycDlg::xSetDataToDlgCtrl()
{
	//set name
	TCHAR szName[MAX_PATH];
	memset(szName,0,sizeof(TCHAR) * MAX_PATH);
	m_pData->GetName(szName);
	m_edName.SetWindowText(szName);
	DEQPHONEDATA *pDeqData = m_pData->GetPhoneData();
	
	//set data to control
	int iNumber = 0,iData = 0;
	for(DEQPHONEDATA::iterator iter = pDeqData->begin() ; iter != pDeqData->end() ; iter ++){
		CCheckEx *pCh = NULL;
		CComboBox *pCb = NULL;
		CMyEdit *pEdit = NULL;
		bool bDetain = false;
		if((*iter).GetType() == PBK_Number_General && iNumber < 5){
			//get control
			xGetUnusedControl(iNumber,&pCh,&pCb,&pEdit);
			iNumber ++;
			//set data
			if(pCb)
				pCb->SetCurSel(0);
			else
				bDetain = true;
		}
		else if((*iter).GetType() == PBK_Number_Mobile && iNumber < 5){
			//get control
			xGetUnusedControl(iNumber,&pCh,&pCb,&pEdit);
			iNumber ++;
			//set data
			if(pCb)
				pCb->SetCurSel(1);
			else
				bDetain = true;
		}
		else if((*iter).GetType() == PBK_Number_Home && iNumber < 5){
			//get control
			xGetUnusedControl(iNumber,&pCh,&pCb,&pEdit);
			iNumber ++;
			//set data
			if(pCb)
				pCb->SetCurSel(2);
			else
				bDetain = true;
		}
		else if((*iter).GetType() == PBK_Number_Work && iNumber < 5){
			//get control
			xGetUnusedControl(iNumber,&pCh,&pCb,&pEdit);
			iNumber ++;
			//set data
			if(pCb)
				pCb->SetCurSel(3);
			else
				bDetain = true;
		}
		else if((*iter).GetType() == PBK_Number_Fax && iNumber < 5){
			//get control
			xGetUnusedControl(iNumber,&pCh,&pCb,&pEdit);
			iNumber ++;
			//set data
			if(pCb)
				pCb->SetCurSel(4);
			else
				bDetain = true;
		}
		else if((*iter).GetType() == PBK_Text_Email && iData < 3){
			//get control
			xGetUnusedControl(iData + 5,&pCh,&pCb,&pEdit);
			iData ++;
			//set data
			if(pCb)
				pCb->SetCurSel(0);
			else
				bDetain = true;
		}
		else if((*iter).GetType() == PBK_Text_Postal && iData < 3){
			//get control
			xGetUnusedControl(iData + 5,&pCh,&pCb,&pEdit);
			iData ++;
			//set data
			if(pCb)
				pCb->SetCurSel(1);
			else
				bDetain = true;
		}
		else if((*iter).GetType() == PBK_Text_Note && iData < 3){
			//get control
			xGetUnusedControl(iData + 5,&pCh,&pCb,&pEdit);
			iData ++;
			//set data
			if(pCb)
				pCb->SetCurSel(2);
			else
				bDetain = true;
		}
		else if((*iter).GetType() == PBK_Text_URL && iData < 3){
			//get control
			xGetUnusedControl(iData + 5,&pCh,&pCb,&pEdit);
			iData ++;
			//set data
			if(pCb)
				pCb->SetCurSel(3);
			else
				bDetain = true;
		}
		else if((*iter).GetType() == PBK_Caller_Group){
			m_cbGroup.SetCurSel((*iter).GetNumber());
		}
		//keep other type data
		else if((*iter).GetType() != PBK_Text_Name){
			m_deqDetainData.push_back((*iter));
		}
			
		if(bDetain){
			m_deqDetainData.push_back((*iter));
		}

		if(pEdit)
			pEdit->SetWindowText((*iter).GetText());
		if(pCh)
			pCh->SetCheck(1);
	}
	
	xSetDefaultEnableControl();
	xSetRadioBtn();
}

void CDataDycDlg::xSetComboItem()
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
	}
	else{
		m_scGroup.EnableWindow(FALSE);
	}

	//set combo text
	TCHAR szText[5][MAX_PATH];
	TCHAR szTxtSec[5][32] = {_T("IDS_GENERAL"),_T("IDS_MOBILE"),_T("IDS_HOME"),_T("IDS_OFFICE"),_T("IDS_FAX")};
	//get string from language resource
	for(int i = 0 ; i < 5 ; i ++){
		al_GetSettingString(_T("public"),szTxtSec[i],theApp.m_szRes,szText[i]);
	}
	//add these to first 5 combo box
	CComboBox *pCombo[5] = {&m_cb2,&m_cb3,&m_cb4,&m_cb5,&m_cb6};
	for( i = 0 ; i < 5 ; i ++){
		for(int j = 0 ; j < 5 ; j ++)
			pCombo[i]->AddString(szText[j]);
	}

	//set text id
	_tcscpy(szTxtSec[0],_T("IDS_EMAIL3"));
	_tcscpy(szTxtSec[1],_T("IDS_ADDRESS"));
	_tcscpy(szTxtSec[2],_T("IDS_NOTE"));
	_tcscpy(szTxtSec[3],_T("IDS_WEBADDRESS"));
	//get string from resource
	for( i = 0 ; i < 4 ; i ++){
		memset(szText[i],0,sizeof(TCHAR) * MAX_PATH);
		al_GetSettingString(_T("public"),szTxtSec[i],theApp.m_szRes,szText[i]);
	}
	//set combo 
	pCombo[0] = &m_cb7;
	pCombo[1] = &m_cb8;
	pCombo[2] = &m_cb9;
	//add string to last 3 combo box
	for(i = 0 ; i < 3 ; i ++){
		for(int j = 0 ; j < 4 ; j ++)
			pCombo[i]->AddString(szText[j]);
	}

}

void CDataDycDlg::xGetUnusedControl(int iIndex, CCheckEx **pCh, CComboBox **pCb, CMyEdit **pEdit)
{
	switch(iIndex){
	case 0:
		*pCh = NULL;
		*pCb = &m_cb2;
		*pEdit = &m_ed2;
		break;
	case 1:	//combo3
		*pCh = &m_ch3;
		*pCb = &m_cb3;
		*pEdit = &m_ed3;
		break;
	case 2:	//combo4
		*pCh = &m_ch4;
		*pCb = &m_cb4;
		*pEdit = &m_ed4;
		break;
	case 3:	//combo5
		*pCh = &m_ch5;
		*pCb = &m_cb5;
		*pEdit = &m_ed5;
		break;
	case 4:	//combo6
		*pCh = &m_ch6;
		*pCb = &m_cb6;
		*pEdit = &m_ed6;
		break;
	case 5:	//combo7
		*pCh = &m_ch7;
		*pCb = &m_cb7;
		*pEdit = &m_ed7;
		break;
	case 6:	//combo8
		*pCh = &m_ch8;
		*pCb = &m_cb8;
		*pEdit = &m_ed8;
		break;
	case 7:	//combo9
		*pCh = &m_ch9;
		*pCb = &m_cb9;
		*pEdit = &m_ed9;
		break;
	default :
		*pCh = NULL;
		*pCb = NULL;
		*pEdit = NULL;
		break;
	}
}

void CDataDycDlg::xSetDefaultControl()
{
	m_cb2.SetCurSel(CB_GENERAL);
	m_cb3.SetCurSel(CB_MOBILE);
	m_cb4.SetCurSel(CB_HOME);
	m_cb5.SetCurSel(CB_OFFICE);
	m_cb6.SetCurSel(CB_FAX);
	m_cb7.SetCurSel(CB_EMAIL);
	m_cb8.SetCurSel(CB_ADDRESS);
	m_cb9.SetCurSel(CB_NOTE);
	m_cbGroup.SetCurSel(0);

	m_ed2.SetNumFlag(true);
	m_ed3.SetNumFlag(true);
	m_ed4.SetNumFlag(true);
	m_ed5.SetNumFlag(true);
	m_ed6.SetNumFlag(true);

	m_ed2.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_ed3.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_ed4.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_ed5.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_ed6.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_ed7.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_ed8.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);
	m_ed9.SetLimitText(GSM_PHONEBOOK_TEXT_LENGTH+1);

	xSetRadioBtn();
}


void CDataDycDlg::xSetDefaultEnableControl()
{
	CCheckEx *pCh[7] = {&m_ch3,&m_ch4,&m_ch5,&m_ch6,&m_ch7,&m_ch8,&m_ch9};
	CComboBox *pCb[7] = {&m_cb3,&m_cb4,&m_cb5,&m_cb6,&m_cb7,&m_cb8,&m_cb9};
	CMyEdit *pEd[7] = {&m_ed3,&m_ed4,&m_ed5,&m_ed6,&m_ed7,&m_ed8,&m_ed9};

	for(int i = 0 ; i < 7 ; i ++){
		if(pCh[i]->GetCheck()){
			pCb[i]->EnableWindow();
			pEd[i]->EnableWindow();
		}
		else{
			pCb[i]->EnableWindow(FALSE);
			pEd[i]->EnableWindow(FALSE);
		}
	}
}

void CDataDycDlg::xSetDefaultCombo()
{
	//get default combobox cur sel
	int iDefSel = m_cbDefault.GetCurSel();
	//get the cursel string
	TCHAR szText[MAX_PATH];
	m_cbDefault.GetLBText(iDefSel,szText);
	//get the first char to know which one has been selected
	TCHAR szKey = szText[0];
	int iKeySel = _ttoi(&szKey);
	//reset string
	m_cbDefault.ResetContent();

	CComboBox *pCb[5] = {&m_cb2,&m_cb3,&m_cb4,&m_cb5,&m_cb6};
	CCheckEx *pCh[4] = {&m_ch3,&m_ch4,&m_ch5,&m_ch6};

	bool bRet = false;
	for(int i = 0 ;i < 5 ; i ++){
		int iSel = pCb[i]->GetCurSel();
		pCb[i]->GetLBText(iSel,szText);
		TCHAR szFormat[MAX_PATH];
		wsprintf(szFormat,_T("%d."),i+2);
		Tstring sTxt = Tstring(szFormat) + Tstring(szText);
		if( i == 0){
			int iIndex = m_cbDefault.AddString(sTxt.c_str());
			if(iKeySel == i + 2){
				m_cbDefault.SetCurSel(iIndex);
				bRet = true;
			}
		}
		else{
			if(pCh[i - 1]->GetCheck()){
				int iIndex = m_cbDefault.AddString(sTxt.c_str());
				if(iKeySel == i + 2){
					m_cbDefault.SetCurSel(iIndex);
					bRet = true;
				}
			}
		}		
	}
	if(!bRet)
		m_cbDefault.SetCurSel(0);
}

void CDataDycDlg::xEnableControl(int iIndex, BOOL bFlag)
{
	if(iIndex < 0 || iIndex > 9)	//out of range
		return ;

	CCheckEx *pCh[7] = {&m_ch3,&m_ch4,&m_ch5,&m_ch6,&m_ch7,&m_ch8,&m_ch9};
	CComboBox *pCb[8] = {&m_cb2,&m_cb3,&m_cb4,&m_cb5,&m_cb6,&m_cb7,&m_cb8,&m_cb9};
	CEdit *pEd[8] = {&m_ed2,&m_ed3,&m_ed4,&m_ed5,&m_ed6,&m_ed7,&m_ed8,&m_ed9};

	if(iIndex == 0){	//name
		m_edName.EnableWindow(bFlag);
	}
	else if(iIndex == 1){
		pCb[0]->EnableWindow(bFlag);
		pEd[0]->EnableWindow(bFlag);
	}
	else{
		pCh[iIndex - 2]->EnableWindow(bFlag);
		if(bFlag == TRUE){
			if(pCh[iIndex - 2]->GetCheck()){
				pCb[iIndex - 1]->EnableWindow(bFlag);
				pEd[iIndex - 1]->EnableWindow(bFlag);
			}
		}
		else{
			pCb[iIndex - 1]->EnableWindow(bFlag);
			pEd[iIndex - 1]->EnableWindow(bFlag);
		}
	}
}

void CDataDycDlg::xSetSupportColumnLength()
{
	DEQPHBOUNDARY *pBoundary = m_se.GetPhBoundary();
	
	CEdit *pEd[8] = {&m_ed2,&m_ed3,&m_ed4,&m_ed5,&m_ed6,&m_ed7,&m_ed8,&m_ed9};

	for(DEQPHBOUNDARY::iterator iter = pBoundary->begin() ; iter != pBoundary->end() ; iter ++){
		if((*iter).GetType() == PBK_Text_Name)
			m_edName.SetLimitText((*iter).GetBoundary());
		for(int i = 0 ; i < 8; i ++){
			if((*iter).GetType() == xGetComboSel(i))
				pEd[i]->SetLimitText((*iter).GetBoundary());
		}
	}
}

int CDataDycDlg::xGetComboSel(int iIndex)
{
	CComboBox *pCb[8] = {&m_cb2,&m_cb3,&m_cb4,&m_cb5,&m_cb6,&m_cb7,&m_cb8,&m_cb9};
	CCheckEx *pCh[8] = {NULL,&m_ch3,&m_ch4,&m_ch5,&m_ch6,&m_ch7,&m_ch8,&m_ch9};

	int iType;
	bool bCh = true;

	if(pCh[iIndex]){
		if(!pCh[iIndex]->GetCheck())
			bCh = false;
	}
	if(bCh){
		if(iIndex < 5){
			switch(pCb[iIndex]->GetCurSel()){
			case 0:
				iType = PBK_Number_General;
				break;
			case 1:
				iType = PBK_Number_Mobile;
				break;
			case 2:
				iType = PBK_Number_Home;
				break;
			case 3:
				iType = PBK_Number_Work;
				break;
			case 4:
				iType = PBK_Number_Fax;
				break;
			}
		}
		else{
			switch(pCb[iIndex]->GetCurSel()){
			case 0:
				iType = PBK_Text_Email;
				break;
			case 1:
				iType = PBK_Text_Postal;
				break;
			case 2:
				iType = PBK_Text_Note;
				break;
			case 3:
				iType = PBK_Text_URL;
				break;
			}
		}
		return iType;
	}
	else
		return -1;
}


void CDataDycDlg::xSetCombo2Item()
{
	if(m_rdSIM.GetCheck()){
		for(int i = 4 ; i > 0 ; i --)
			m_cb2.DeleteString(i);
		m_cb2.SetCurSel(0);
	}
	else if(m_cb2.GetCount() <= 1){
		//set combo text
		TCHAR szText[4][MAX_PATH];
		TCHAR szTxtSec[4][32] = {_T("IDS_MOBILE"),_T("IDS_HOME"),_T("IDS_OFFICE"),_T("IDS_FAX")};
		//get string from language resource
		for(int i = 0 ; i < 4 ; i ++){
			al_GetSettingString(_T("public"),szTxtSec[i],theApp.m_szRes,szText[i]);
		}
		//add these to first 5 combo box
		for(i = 0 ; i < 4 ; i ++)
			m_cb2.AddString(szText[i]);
		m_cb2.SetCurSel(0);
	}
	xSetDefaultCombo();
}
