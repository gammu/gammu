// PrevView.cpp : implementation file
//

#include "stdafx.h"
#include "..\PhoneBook.h"
#include "PrevView.h"
//#include "..\Dlg\databasedlg.h"
//#include "..\Dlg\databasedlg2.h"
#include "..\DriverWrapper.h"
#include "..\include\CSVHandle.h"
#include "..\MainFrm.h"
#include "..\Dlg\DataDycDlg.h"
#include "..\Dlg\S6DataDycDlg.h"
#include "..\Dlg\ProgDlg.h"
#include "..\Dlg\ASUSDlg.h"
#include "..\Dlg\BaseDlg.h"
#include "..\Dlg\MotoDlg.h"
#include "..\Dlg\SonyErisonTDlg.h"
#include "..\Dlg\SiemensSDlg.h"
#include "..\Dlg\sonyericcsonskdlg.h"
#include "..\Dlg\sonyericssonatdlg.h" // added by mingfa
#include "..\Dlg\SamsungDlg.h"
#include "..\Dlg\OtherDlg.h"
#include "..\Dlg\SharpT300Dlg.h"
#include "..\Dlg\Sharp90Xdlg.h"
#include "..\Dlg\progressDlg.h"
#include "..\Dlg\SagemDlg.h"
#include "..\Dlg\SamsungZdlg.h"
#include "..\Dlg\S4DataDycDlg.h"
#include "..\Dlg\S4DataDycDlgEx.h"
#include "..\Dlg\MotoEDlg.h"
#include "..\Dlg\MotoE2Dlg.h"
#include "..\Dlg\NewSMS.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


enum{
	PHONE_ICON = 0,
	SIM_ICON = 1
};

/////////////////////////////////////////////////////////////////////////////
// CPrevView

IMPLEMENT_DYNCREATE(CPrevView, CView)

CPrevView* CPrevView::m_pActivate = NULL;

CPrevView::CPrevView()
{
	m_pSearchDlg = NULL;
	m_LeftMode.sMode = MOBILE_PHONE;
	m_crItemBgNor = RGB(0,0,0);
	m_crItemBgSel = RGB(0,0,0);
	m_crItemBgFocus = RGB(0,0,0);
	m_crItemTextNor = RGB(0,0,0);
	m_crItemTextSel = RGB(0,0,0);
	m_crItemTextFocus = RGB(0,0,0);
	m_crCol1 = 0;
	m_crCol2 = 0;
	m_pActivate = this;
	m_pThread = NULL;
	m_bDeleteDatainME = false;
	m_lpFolder = NULL;
}

CPrevView::~CPrevView()
{
	//close driver
	m_Driver.Close();
	m_Driver.FreeDriver();
	FreeNeedSendSMS();
}


BEGIN_MESSAGE_MAP(CPrevView, CView)
	//{{AFX_MSG_MAP(CPrevView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_KILLFOCUS()
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST, OnBeginDrag)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, OnCustomDraw)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SEARCH_STR,OnSearchDlgStr)
	ON_MESSAGE(WM_VIEWMODE_CHANGE,OnViewModeChange)
	ON_MESSAGE(WM_TOOLBAR_DELSEL,OnToolbarDelSel)
	ON_MESSAGE(WM_TOOLBAR_SEARCH,OnToolbarSearch)
	ON_MESSAGE(WM_TOOLBAR_NEWEDIT,OnToolbarNewEdit)
	ON_MESSAGE(WM_TOOLBAR_RELOAD,OnToolbarReload)
	ON_MESSAGE(WM_TOOLBAR_BACKUP,OnToolbarBackup)
	ON_MESSAGE(WM_TOOLBAR_RESTORE,OnToolbarRestore)
	ON_MESSAGE(WM_SEARCH_END,OnSearchDlgEnd)
	ON_MESSAGE(WM_CARDPANEL_GETGROUP,OnCardPanelGetGroup)
	ON_MESSAGE(WM_CARDPANEL_GROUPCOUNT,OnCardPanelGroupCount)
	ON_MESSAGE(WM_LEFT_CHGMODE,OnLeftChange)
	ON_MESSAGE(WM_LEFT_DROPDATA,OnLeftDropData)
	ON_MESSAGE(WM_LOADMOBILE,OnLoadMobile)
	ON_MESSAGE(WM_OPENDRIVER,OnOpenDriver)
	ON_MESSAGE(WM_THREADBREAK,OnThreadBreak)
	ON_MESSAGE(WM_THREADEND,OnThreadEnd)
//	ON_MESSAGE(WM_GETLISTSIZE,OnGetListSize)
	ON_MESSAGE(WM_SEARCH_GETSTART,OnSearchDlgGetStart)
	ON_MESSAGE(WM_LEFT_DROPDATATOMSOT,OnLeftDropData2MSOT)
	ON_MESSAGE(WM_TOOLBAR_RELOAD_MSOT,OnToolbarReloadMSOT)
	ON_MESSAGE(WM_TOOLBAR_SENDSMS,OnToolbarSendSMS)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrevView drawing

/*void CPrevView::OnPaint()
{
	CPaintDC dc(this);
	
}*/


/////////////////////////////////////////////////////////////////////////////
// CPrevView message handlers

BOOL CPrevView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CView::PreCreateWindow(cs))
		return FALSE;

	//set window style
	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	//register window class
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

int CPrevView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	CRect rect;
	GetClientRect(&rect);
	
	CRect rcList(rect.left,rect.top,rect.right,rect.bottom - 22);
	CRect rc(rect.left ,rect.bottom - 22,rect.right,rect.bottom);

	//create the list control
	m_List.Create(WS_CHILD | WS_VISIBLE /*| WS_BORDER*/  |LVS_AUTOARRANGE | LVS_REPORT,rect,this,IDC_LIST);
	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT );
	m_List.init();

	//create the view mode change panel
	m_wndMode.Create(NULL, _T(""), WS_VISIBLE, rc, this, -1, NULL);
	//get tree item color
	Tstring sProfile = Tstring(theApp.m_szSkin) + Tstring(_T("Organize\\List.ini"));

	al_GetSettingColor(_T("list"),_T("color_text_nor"),const_cast<TCHAR*>(sProfile.c_str()),m_crItemTextNor);
	al_GetSettingColor(_T("list"),_T("color_text_sel"),const_cast<TCHAR*>(sProfile.c_str()),m_crItemTextSel);
	al_GetSettingColor(_T("list"),_T("color_text_focusl"),const_cast<TCHAR*>(sProfile.c_str()),m_crItemTextFocus);
	al_GetSettingColor(_T("list"),_T("color_bg_nor"),const_cast<TCHAR*>(sProfile.c_str()),m_crItemBgNor);
	al_GetSettingColor(_T("list"),_T("color_bg_sel"),const_cast<TCHAR*>(sProfile.c_str()),m_crItemBgSel);
	al_GetSettingColor(_T("list"),_T("color_bg_focus"),const_cast<TCHAR*>(sProfile.c_str()),m_crItemBgFocus);

	al_GetSettingColor(_T("background"),_T("color1"),const_cast<TCHAR*>(sProfile.c_str()),m_crCol1);
	al_GetSettingColor(_T("background"),_T("color2"),const_cast<TCHAR*>(sProfile.c_str()),m_crCol2);
	
	TCHAR szPath[MAX_PATH];
	if(al_GetSettingString(_T("icon"),_T("sim"),const_cast<TCHAR*>(sProfile.c_str()),szPath)){
		Tstring sPath = Tstring(theApp.m_szSkin) + Tstring(szPath);
		m_imgIcon[SIM_ICON].LoadFile(sPath.c_str());
	}
	if(al_GetSettingString(_T("icon"),_T("mobile"),const_cast<TCHAR*>(sProfile.c_str()),szPath)){
		Tstring sPath = Tstring(theApp.m_szSkin) + Tstring(szPath);
		m_imgIcon[PHONE_ICON].LoadFile(sPath.c_str());
	}
	m_ilMeasure.Create(THUMB_WIDTH,THUMB_HEIGHT,ILC_COLOR24,0,0);
	m_List.SetImageList(&m_ilMeasure,NULL);
	m_dlgProg.Create(IDD_DLG_PROG,this);
	m_dlgProg.SetParent(this);
	return 0;
}

void CPrevView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	//move listctrl and viewmode panel to correct pos
	if(m_wndMode.GetSafeHwnd()){
		CRect rect;
		GetClientRect(&rect);
		CRect rc(rect.left ,rect.bottom - 22,rect.right,rect.bottom);
		m_wndMode.MoveWindow(rc);
	}
	if(m_List.GetSafeHwnd()){
		CRect rect;
		GetClientRect(&rect);
		CRect rcList(rect.left,rect.top,rect.right,rect.bottom - 22);
		m_List.MoveWindow(rcList);
	}
}


LRESULT CPrevView::OnViewModeChange(WPARAM wParam,LPARAM lParam)
{
	//set listctrl view mode
	m_List.SetViewMode(wParam);
	m_List.SetFocus();
	return 1L;
}

void CPrevView::AddDataToList(DATALIST &lsData)
{
	for(DATALIST::iterator iter = lsData.begin() ; iter != lsData.end() ; iter ++){
		(*iter).init();
		m_List.SetData((*iter));
	}
	m_List.DrawThumbnails();
}

void CPrevView::AddDataToList(CCardPanel &data)
{
	CCardPanel dataTemp;
	dataTemp = data;
	dataTemp.init();
	m_List.AddData(dataTemp);
	m_List.DrawThumbnails();
	dataTemp.Release();

}
LRESULT CPrevView::OnToolbarDelSel(WPARAM wParam,LPARAM lParam)
{
	if(m_LeftMode.sMode == COMPUTER || m_LeftMode.sMode == CSVFILE
		||m_LeftMode.sMode == MSOUTLOOK){
		return 1L;
	}

	if( m_List.GetNextItem(-1,LVNI_SELECTED) != -1)	
	{
		TCHAR szMsg[MAX_PATH],szMsg1[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_DELCONTACT"),theApp.m_szRes,szMsg);
		al_GetSettingString(_T("public"),_T("IDS_DELCONTACT1"),theApp.m_szRes,szMsg1);
		Tstring sFormat = Tstring(szMsg) + Tstring(_T("\n")) + Tstring(szMsg1);

		if(AfxMessageBox(sFormat.c_str(),MB_OKCANCEL) != IDOK)
			return 1L;
	}
	else//08060o1libaoliu
	{
		return 1L;

	}

//	m_dlgProg.ShowWindow(SW_SHOW);
//	m_dlgProg.SetFocus();
	::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_PROGDLGSHOW,0,0);
	m_bDeleteDatainME = false;

	xStartDriverThread(DEL_DATA);
	return 1L;
}
void CPrevView::FreeNeedSendSMS()
{
	POSITION pos = m_NeedSendSMSList.GetHeadPosition();
	while(pos)
	{
		SMS_PARAM	*pData = (SMS_PARAM	*)m_NeedSendSMSList.GetNext(pos);
		if(pData) delete pData;
	}
	m_NeedSendSMSList.RemoveAll();

}
int CPrevView::xSendSMS(CString strGetMsg, CString PhoneNumberList, int nTotalPage, bool bMultiByte)
{
	FreeNeedSendSMS();
	CString PhoneList;
	TCHAR strSendMsg[MAX_CONTENT+1];
	int nPos = 0;
	for(int nSendPage = 1; nSendPage <= nTotalPage; nSendPage++)
	{
		memset(strSendMsg, 0, (MAX_CONTENT+1)*sizeof(TCHAR));
		if(bMultiByte)
		{
			MultiByteCut(strGetMsg, strSendMsg, nSendPage);

			// Check Phone Number Lsit
			int ret = 0;
			PhoneList.Format(_T("%s"),PhoneNumberList);
			TCHAR *PhoneNumber = _tcstok(PhoneList.GetBuffer(MAX_PATH), _T(";"));
			while(1)
			{
				SMS_PARAM	*Mobile_SMS = new SMS_PARAM;
				Mobile_SMS->TP_DCS = 1; // Unicode
				_tcscpy(Mobile_SMS->TP_UD, strSendMsg); 
				_tcscpy(Mobile_SMS->TPA, PhoneNumber);

				m_NeedSendSMSList.AddTail(Mobile_SMS);

				PhoneNumber = _tcstok(NULL, _T(";"));
				if(PhoneNumber == NULL)	
					break;
			}
			PhoneList.ReleaseBuffer();
		}
		else
		{
			SingleByteCut(strGetMsg, strSendMsg, nSendPage);

			// Check Phone Number Lsit
			int ret = 0;
			PhoneList.Format(_T("%s"),PhoneNumberList);
			TCHAR *PhoneNumber = _tcstok(PhoneList.GetBuffer(MAX_PATH), _T(";"));
			while(1)
			{
				SMS_PARAM	*Mobile_SMS = new SMS_PARAM;
				Mobile_SMS->TP_DCS = 2; // Default
				_tcscpy(Mobile_SMS->TP_UD, strSendMsg); 
				_tcscpy(Mobile_SMS->TPA, PhoneNumber);
				m_NeedSendSMSList.AddTail(Mobile_SMS);

				PhoneNumber = _tcstok(NULL, _T(";"));
				if(PhoneNumber == NULL)	
					break;
			}
			PhoneList.ReleaseBuffer();
		}

		
	}
	::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_PROGDLGSHOW,0,0);
	xStartSendMessageThread();
	return true;
}
CString CPrevView::GetCurrentSelectPhoneNoString()
{
	CString ShowPhoneList;
	ShowPhoneList.Empty();
	if( m_List.GetNextItem(-1,LVNI_SELECTED) != -1)	
	{
		DATALIST lsData;
		if(m_pActivate->m_List.GetSelectedData(lsData))
		{
			int i = 0;
			int iErr =1;
			for(DATALIST::iterator iter = lsData.begin() ; iter != lsData.end() ; iter ++,i++)
			{
				DEQPHONEDATA *pDeqData = (*iter).GetPhoneData();
				bool nMobilePhone = false;	
				CString strMobileNo,strGeneralNo,strHomeNo,strWorkNo,strOtherNo,strPagerNo,strFaxNo;
				strMobileNo.Empty();
				strGeneralNo.Empty();
				strHomeNo.Empty();
				strWorkNo.Empty();
				strOtherNo.Empty();
				strPagerNo.Empty();
				strFaxNo.Empty();
				for(DEQPHONEDATA::iterator iterData = pDeqData->begin() ; iterData != pDeqData->end() ; iterData ++)
				{
					switch((*iterData).GetType())
					{
						case PBK_Number_Mobile : 
							strMobileNo.Format(_T("%s"),(*iterData).GetText());
							break;
						case PBK_Number_General : 
							strGeneralNo.Format(_T("%s"),(*iterData).GetText());
							break;
						case PBK_Number_Home : 
							strHomeNo.Format(_T("%s"),(*iterData).GetText());
							break;
						case PBK_Number_Work : 
							strWorkNo.Format(_T("%s"),(*iterData).GetText());
							break;
						case PBK_Number_Other : 
							strOtherNo.Format(_T("%s"),(*iterData).GetText());
							break;
						case PBK_Number_Pager : 
							strPagerNo.Format(_T("%s"),(*iterData).GetText());
							break;
						case PBK_Number_Fax : 
							strFaxNo.Format(_T("%s"),(*iterData).GetText());
							break;

					}
				}

				CString strNo;
				strNo.Empty();
				if(strMobileNo.GetLength()>0)
					strNo.Format(_T("%s"),strMobileNo);
				else if(strGeneralNo.GetLength()>0)
					strNo.Format(_T("%s"),strGeneralNo);
				else if(strWorkNo.GetLength()>0)
					strNo.Format(_T("%s"),strWorkNo);
				else if(strHomeNo.GetLength()>0)
					strNo.Format(_T("%s"),strHomeNo);
				else if(strOtherNo.GetLength()>0)
					strNo.Format(_T("%s"),strOtherNo);
				else if(strPagerNo.GetLength()>0)
					strNo.Format(_T("%s"),strPagerNo);
				else if(strFaxNo.GetLength()>0)
					strNo.Format(_T("%s"),strFaxNo);
				if(_tcslen(strNo) >0)
				{
					ShowPhoneList += strNo;
					ShowPhoneList += ";";
				}
			}
		}
	}
	return ShowPhoneList;
}
HRESULT CPrevView::OnToolbarSendSMS(WPARAM wParam,LPARAM lParam)
{
	if( m_List.GetNextItem(-1,LVNI_SELECTED) != -1)	
	{
		CString ShowPhoneList = GetCurrentSelectPhoneNoString();
		CNewSMS SendSMSDlg;

		SendSMSDlg.SetPhoneNo(ShowPhoneList);
		if(SendSMSDlg.DoModal() == IDOK)
		{
			xSendSMS(SendSMSDlg.m_strGetMsg, SendSMSDlg.m_strPhone, SendSMSDlg.m_nPage, SendSMSDlg.m_bMultiByte);
		}
		m_List.SetFocus();
		m_List.Invalidate(FALSE);

	}

	return 1L;
}

HRESULT CPrevView::OnToolbarNewEdit(WPARAM wParam,LPARAM lParam)
{
	if(wParam == 0 && (m_LeftMode.sMode == CSVFILE || m_LeftMode.sMode == MSOUTLOOK))
		return 1L;
	CMainFrame* pMainFram = (CMainFrame*) afxGetMainWnd();

	if(wParam == 0 && (!(pMainFram->m_iStatus & ST_MOBILE_LINK)|| (pMainFram->m_iStatus & ST_DOWNLOAD_FAIL)))
		return 1L;
//	CDataBaseDlg2 dlg(this);
	CBaseDlg *pDlg = NULL;
	if(theApp.m_iMobileCompany == CO_NOKIA)	//nokia
	{
		if(_tcscmp(theApp.m_szPhone,_T("6270")) == 0 || _tcscmp(theApp.m_szPhone,_T("7370")) == 0|| 
			_tcscmp(theApp.m_szPhone,_T("6280")) == 0)
			pDlg = new CS4DataDycDlg(this);
		else if(_tcscmp(theApp.m_szPhone,_T("6125")) == 0|| _tcscmp(theApp.m_szPhone,_T("6131")) == 0 || _tcscmp(theApp.m_szPhone,_T("6288")) == 0 ||
			_tcscmp(theApp.m_szPhone,_T("6233")) == 0||_tcscmp(theApp.m_szPhone,_T("5300")) == 0||_tcscmp(theApp.m_szPhone,_T("6151")) == 0||_tcscmp(theApp.m_szPhone,_T("5200")) == 0||
			 _tcscmp(theApp.m_szPhone,_T("6300")) == 0)
			pDlg = new CS4DataDycDlgEx(this);
		else if(_tcsncicmp(theApp.m_szConnectMode, _T("SYNCML"),6) == 0)
			pDlg = new CS6DataDycDlg(this);
		else
			pDlg = new CDataDycDlg(this);
	}
	else if(theApp.m_iMobileCompany == CO_ASUS && (_tcscmp(theApp.m_szPhone,_T("J202")) != 0))	//asus
		pDlg = new CASUSDlg(this);
	else if(theApp.m_iMobileCompany == CO_MOTO)	
	{
		if((_tcscmp(theApp.m_szPhone,_T("E770")) == 0 ) || (_tcscmp(theApp.m_szPhone,_T("E1070")) == 0) || (_tcscmp(theApp.m_szPhone,_T("RAZR V3x")) == 0)
			|| (_tcscmp(theApp.m_szPhone,_T("KRZR K1")) == 0) ||(_tcscmp(theApp.m_szPhone,_T("RIZR Z3")) == 0)||(_tcscmp(theApp.m_szPhone,_T("RAZR V6")) == 0))
		{
			pDlg = new CMotoEDlg(this);
			if((_tcscmp(theApp.m_szPhone,_T("KRZR K1")) == 0) ||(_tcscmp(theApp.m_szPhone,_T("RIZR Z3")) == 0))
				((CMotoEDlg*)pDlg)->SetSkinFileName(_T("MotoEDlg_K1.ini"));
			else if((_tcscmp(theApp.m_szPhone,_T("RAZR V6")) == 0))
				((CMotoEDlg*)pDlg)->SetSkinFileName(_T("MotoEDlg_ex.ini"));

		}
		else if((_tcscmp(theApp.m_szPhone,_T("E2")) == 0))
			pDlg = new CMotoE2Dlg(this);
		else
		{
			pDlg = new CMotoDlg(this);
			if((_tcscmp(theApp.m_szPhone,_T("E550")) == 0 ) || (_tcscmp(theApp.m_szPhone,_T("V620")) == 0) || 
				(_tcscmp(theApp.m_szPhone,_T("V550")) == 0) || (_tcscmp(theApp.m_szPhone,_T("E1000")) == 0) ||
				(_tcscmp(theApp.m_szPhone,_T("V600i")) == 0)|| (_tcscmp(theApp.m_szPhone,_T("V360")) == 0) ||
				(_tcscmp(theApp.m_szPhone,_T("E1")) == 0)|| (_tcscmp(theApp.m_szPhone,_T("L6")) == 0)||
				(_tcscmp(theApp.m_szPhone,_T("L7")) == 0)|| (_tcscmp(theApp.m_szPhone,_T("V361")) == 0)||
				(_tcscmp(theApp.m_szPhone,_T("PEBL U6")) == 0) || (_tcscmp(theApp.m_szPhone,_T("V3i")) == 0)||
				(_tcscmp(theApp.m_szPhone,_T("V3ie")) == 0))
				((CMotoDlg*)pDlg)->SetSkinFileName(_T("MotoDlgNew.ini"));
			else
				((CMotoDlg*)pDlg)->SetSkinFileName(_T("MotoDlg.ini"));
		}
	}// add "F500i" and "K300i , by mingfa,  0425
	else if(theApp.m_iMobileCompany == CO_SONY_ERISON)
	{
		 if(((_tcscmp(theApp.m_szPhone,_T("T230")) == 0 ) || (_tcscmp(theApp.m_szPhone,_T("T290i")) == 0) ||
				(_tcscmp(theApp.m_szPhone,_T("T300")) == 0)|| (_tcscmp(theApp.m_szPhone,_T("T300c")) == 0)))
			{ // added by mingfa
				pDlg = new CSonyEricssonAtDlg(this);
				((CSonyEricssonAtDlg*)pDlg)->SetSkinFileName("SET230Dlg.ini");
			}
		 else if((_tcscmp(theApp.m_szPhone,_T("T610")) == 0 ) || (_tcscmp(theApp.m_szPhone,_T("T630")) == 0) ||
				(_tcscmp(theApp.m_szPhone,_T("Z600")) == 0)|| (_tcscmp(theApp.m_szPhone,_T("T68")) == 0)||(_tcscmp(theApp.m_szPhone,_T("T68i")) == 0)||
				(_tcscmp(theApp.m_szPhone,_T("T616")) == 0)|| (_tcscmp(theApp.m_szPhone,_T("T637")) == 0)|| (_tcscmp(theApp.m_szPhone,_T("T628")) == 0))
			pDlg = new CSonyEricssonTDlg(this);
		 else
			pDlg = new CSonyEricssonSKDlg(this);
	}
/*	else if(theApp.m_iMobileCompany == CO_SONY_ERISON && 
		((strcmp(theApp.m_szPhone,"S700i") == 0 ) || (strcmp(theApp.m_szPhone,"K700i") == 0) ||
		(strcmp(theApp.m_szPhone,"K500i") == 0) || (strcmp(theApp.m_szPhone,"Z1010") == 0) || 
		(strcmp(theApp.m_szPhone,"F500i") == 0) || (strcmp(theApp.m_szPhone,"K300i") == 0) ||
		(strcmp(theApp.m_szPhone,"P900") == 0) || (strcmp(theApp.m_szPhone,"V800") == 0) || 
		(strcmp(theApp.m_szPhone,"P910i") == 0)|| (strcmp(theApp.m_szPhone,"K750i") == 0)))
		pDlg = new CSonyEricssonSKDlg(this);
	else if( (theApp.m_iMobileCompany == CO_SONY_ERISON )&& 
		((strcmp(theApp.m_szPhone,"T230") == 0 ) || (strcmp(theApp.m_szPhone,"T290i") == 0) ||
	    (strcmp(theApp.m_szPhone,"T300") == 0)))
	{ // added by mingfa
        pDlg = new CSonyEricssonAtDlg(this);
		((CSonyEricssonAtDlg*)pDlg)->SetSkinFileName("SET230Dlg.ini");
	}	
	else if(theApp.m_iMobileCompany == CO_SONY_ERISON)
		pDlg = new CSonyEricssonTDlg(this);*/
	else if(theApp.m_iMobileCompany == CO_SIMENS)
	{
		if((_tcscmp(theApp.m_szPhone,_T("SL75")) == 0))
			pDlg = new COtherDlg(this);
		else
			pDlg = new CSiemensSDlg(this);
	}
	else if(theApp.m_iMobileCompany == CO_SAMSUNG)		//bobby
	{
		if(_tcscmp(theApp.m_szPhone,_T("Z-Series")) == 0)
			pDlg = new CSamsungZDlg(this);
		else 
			pDlg = new CSamsungDlg(this);			
	}//bobby
	else if(theApp.m_iMobileCompany == CO_SHARP && (_tcscmp(theApp.m_szPhone,_T("GXT300")) == 0))		
		pDlg = new CSharpT300Dlg(this);					
	else if(theApp.m_iMobileCompany == CO_SHARP && ((_tcscmp(theApp.m_szPhone,_T("902SH")) == 0) ||
		(_tcscmp(theApp.m_szPhone,_T("SX813")) == 0) ||
		(_tcscmp(theApp.m_szPhone,_T("903SH")) == 0)))		
		pDlg = new CSharp90XDlg(this);					
	else if(theApp.m_iMobileCompany == CO_SAGEM)
		pDlg = new CSagemDlg(this);
	else 
	//	pDlg = new COtherDlg(this);
	pDlg =new CSamsungDlg(this);

	pDlg->SetSupportColumn(m_SupportEntry);
	bool bMobile = true;
	bool bSupportGroup = m_deqGroup.size() == 0 ? false : true;
	if(wParam == 0){	//new item
		pDlg->SetMode(NEW_CONTACT);
		CCardPanel data;
		//set the new data in the dialog
		pDlg->SetData(&data);
		//Set the default radio
		int iFlag = -1;
		if(m_LeftMode.sMode == MEMORY)
			iFlag = TRUE;
		else if(m_LeftMode.sMode == SIM_CARD)
			iFlag = FALSE;
		if(iFlag != -1)
			pDlg->SetIsME(iFlag);

		if(pDlg->DoModal() == IDOK){
			afxGetMainWnd()->RedrawWindow();
			//add data to mobile
			if(bMobile){
				//add the data in the list ctrl
				int iErr= 1;
				bool bSuc = m_Driver.AddContactData(data,Type_Start,iErr);
			
				if(bSuc == 1 && iErr ==1)
				{
//					AddDataToList(data);
					//check the memory type ,add data to the list
					if(strcmp(data.GetStorageType(),SIM_NAME) == 0){
						m_lsSIMData.push_back(data);
					}
					else{
						m_lsMEData.push_back(data);
					}
					OnLeftChange(reinterpret_cast<WPARAM>(&m_LeftMode),1);
				}
				int err ;
				m_Driver.AddContactData(data,Type_End,err);
				if(iErr!=1)
					m_Driver.xErrorHandling(iErr);

			}
			//add data to file
			else{
				AddDataToList(data);
				//get the file data deque
				deque<DATALIST>::iterator iter = m_deqFileData.begin() + m_LeftMode.iIndex;
				(*iter).push_back(data);
			}
			xSetLeftMobileItemCount();
		}
		data.Release();
	}
	else{
		if(m_LeftMode.sMode == CSVFILE)
			pDlg->SetMode(VIEW_CONTACT);
		else if(m_LeftMode.sMode == MSOUTLOOK)
			pDlg->SetMode(VIEW_CONTACT);
		else
			pDlg->SetMode(EDIT_CONTACT);

		//check selected item is only one
		if(m_List.GetSelectedCount() > 1 || m_List.GetNextItem(-1,LVNI_SELECTED) == -1){
			TCHAR szMsg[MAX_PATH];
			al_GetSettingString(_T("public"),_T("IDS_ERR_SELITEM"),theApp.m_szRes,szMsg);
			AfxMessageBox(szMsg);
			return 1L;
		}
		CCardPanel data;
		CCardPanel Olddata;
		//Get selcted data form list ctrl
		if(m_List.GetSelectedData(data)){
			if(strcmp(data.GetStorageType(),SIM_NAME) == 0){
				pDlg->SetIsME(FALSE);
			}
			else{
				pDlg->SetIsME(TRUE);
			}
			Olddata = data;
			pDlg->SetData(&data);
			if(pDlg->DoModal() == IDOK){
				afxGetMainWnd()->RedrawWindow();
				if(m_LeftMode.sMode == MOBILE_PHONE || m_LeftMode.sMode == MEMORY || m_LeftMode.sMode == SIM_CARD){
					//update the data in the mobile
////////////////
					if(strcmp(data.GetStorageType(),SIM_NAME) != 0)
					{
						if((theApp.m_iMobileCompany == CO_ASUS && (_tcscmp(theApp.m_szPhone,_T("J202")) == 0)) ||
							theApp.m_iMobileCompany == CO_MTK ||
							(theApp.m_iMobileCompany == CO_TOSHIBA && (_tcscmp(theApp.m_szPhone,_T("TS30")) == 0)))
						{
							TCHAR szMsg[MAX_PATH];
							al_GetSettingString(_T("public"),_T("IDS_MODIFYMSG"),theApp.m_szRes,szMsg);
							if(AfxMessageBox(szMsg,MB_YESNO)== IDNO)
							{
								return 1L;
							}
						}
					}
///////////////////
					char szOldLocation[MAX_PATH];
					sprintf(szOldLocation,"%s",data.GetLocation());
			//		oldLocation = data.GetLocation();
					if(m_Driver.UpdateContactData(data,Olddata) == 1){
						//set modify data to the list ctrl 
						m_List.SetSelectedData(data);
						//set the modified data to list
						if(strcmp(data.GetStorageType(),SIM_NAME) == 0){
							//find the list,and check the item index.if the same,set data to item
							for(DATALIST::iterator iter = m_lsSIMData.begin() ; iter!= m_lsSIMData.end() ; iter ++){
						//		if((*iter).GetLocation() == data.GetLocation()){
							//	CString strLocation = (*iter).GetLocation();
								char szLocation[MAX_PATH];
								sprintf(szLocation,"%s",(*iter).GetLocation());

							//	if((*iter).GetLocation() == oldLocation){
							//	if(strLocation.Compare(oldLocation) == 0)
								if(strcmp(szLocation,szOldLocation) == 0)
								{
									(*iter) = data;
									break;
								}
							}
						}
						else{
							for(DATALIST::iterator iter = m_lsMEData.begin() ; iter!= m_lsMEData.end() ; iter ++){
							//	if((*iter).GetLocation() == data.GetLocation()){
		//						CString strLocation = (*iter).GetLocation();
							//	if((*iter).GetLocation() == oldLocation){
								char szLocation[MAX_PATH];
								sprintf(szLocation,"%s",(*iter).GetLocation());
							//	if(strLocation.Compare(oldLocation) == 0){
								if(strcmp(szLocation,szOldLocation) == 0)
								{
									(*iter) = data;
									break;
								}
							}
						}
					}
				}
				else{
					//set modify data to the list ctrl 
					m_List.SetSelectedData(data);
					char szDataLocation[MAX_PATH];
					sprintf(szDataLocation,"%s",data.GetLocation());
					deque<DATALIST>::iterator iterFile = m_deqFileData.begin() + m_LeftMode.iIndex;
					for(DATALIST::iterator iter = (*iterFile).begin() ; iter != (*iterFile).end() ; iter ++){
						char szLocation[MAX_PATH];
						sprintf(szLocation,"%s",(*iter).GetLocation());
					//	if((*iter).GetLocation() == data.GetLocation()){
						if(strcmp(szLocation,szDataLocation) == 0){
							(*iter) = data;
							break;
						}
					}
				}
				//Set focus to list control
		//		m_List.SetFocus();
			}
		}
		data.Release();
	}
	SAFE_DELPTR(pDlg);
	m_List.SetFocus();
	m_List.Invalidate(FALSE);
	return 1L;
}

LRESULT CPrevView::OnToolbarSearch(WPARAM wParam,LPARAM lParam)
{
	if(!m_pSearchDlg){
		m_pSearchDlg = new CSearchDlg(this);
		m_pSearchDlg->SetSearchStrVec(&m_deqSearchStr);
		m_pSearchDlg->Create(IDD_SEARCH,this);
		m_pSearchDlg->ShowWindow(SW_SHOW);
		::PostMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SEARCH_SETFLAG,1,0);
	}
	else
		m_pSearchDlg->SetFocus();
	return 1L;
}

LRESULT CPrevView::OnSearchDlgStr(WPARAM wParam,LPARAM lParam)
{
	TCHAR *lpszText = reinterpret_cast<TCHAR *>(wParam);
	int *iStart = reinterpret_cast<int *>(lParam);

	*iStart = m_List.FindItemData(lpszText,(*iStart));
		//080526libaoliu
	if((*iStart) == -1)
	{
	   *iStart = 0;
	   *iStart = m_List.FindItemData(lpszText,(*iStart));
	}

	if((*iStart) == -1)
	{
		TCHAR szMsg[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_ERR_SEARCH"),theApp.m_szRes,szMsg);
		AfxMessageBox(szMsg);
		*iStart = 0;//080526libaoliu
		for(int i = 0 ; i < m_List.GetItemCount() ; i ++)
		  m_List.SetItemState(i,0,LVIS_SELECTED);
		m_List.SetItemState((*iStart),LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED);
		m_List.EnsureVisible((*iStart),TRUE);

	}
	else
	{
		for(int i = 0 ; i < m_List.GetItemCount() ; i ++)
			m_List.SetItemState(i,0,LVIS_SELECTED);
		m_List.SetItemState((*iStart),LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED);
		m_List.EnsureVisible((*iStart),TRUE);
	}
	m_List.SetFocus();
	return 1L;
}

HRESULT CPrevView::OnSearchDlgEnd(WPARAM wParam,LPARAM lParam)
{
	SAFE_DELPTR(m_pSearchDlg);
	::PostMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SEARCH_SETFLAG,0,0);
	m_List.SetFocus();
	return 1L;
}

void CPrevView::OnDestroy() 
{
	CView::OnDestroy();
	//releasee the CCardPanel object data in the SIM card deque
	for(list<CCardPanel>::iterator iter = m_lsSIMData.begin() ; iter != m_lsSIMData.end() ; iter ++){
		(*iter).Release();
	}
	m_lsSIMData.clear();
	//releasee the CCardPanel object data in the ME card deque
	for(iter = m_lsMEData.begin() ; iter != m_lsMEData.end() ; iter ++){
		(*iter).Release();
	}
	m_lsMEData.clear();
	//clear the search dialog string 
	for(deque<Tstring>::iterator iterStr = m_deqSearchStr.begin() ; iterStr != m_deqSearchStr.end() ; iterStr ++){
		(*iterStr) = _T("");
	}
	m_deqSearchStr.clear();
	//clear the group name deque
	for(iterStr = m_deqGroup.begin() ; iterStr != m_deqGroup.end() ; iterStr ++){
		(*iterStr) = _T("");
	}
	m_deqGroup.clear();	
	//cleat the file path deque
	for(iterStr = m_deqFilePath.begin() ; iterStr != m_deqFilePath.end() ; iterStr ++){
		(*iterStr) = _T("");
	}
	m_deqFilePath.clear();

	//search every file 
	for(deque<DATALIST>::iterator iterFile = m_deqFileData.begin() ; iterFile != m_deqFileData.end() ; iterFile ++){
		//search the item data in every file
		for(iter = (*iterFile).begin() ; iter != (*iterFile).end() ; iter ++){
			(*iter).Release();
		}
		(*iterFile).clear();
	}
	m_deqFileData.clear();

	CCardPanel data;
	data.Release();
	data.ClearShareImage();

	m_LeftMode.sFile = _T("");
	m_LeftMode.sMode = _T("");

}

LPCTSTR CPrevView::GetGroupStr(int iIndex)
{
	if(iIndex < m_deqGroup.size()){
		deque<Tstring>::iterator iter = m_deqGroup.begin() + iIndex; 
		return (*iter).c_str();
	}
	return NULL;
}

HRESULT CPrevView::OnCardPanelGetGroup(WPARAM wParam,LPARAM lParam)
{
	TCHAR *lpTxt = reinterpret_cast<TCHAR *>(lParam);
	const TCHAR *lpszGroup = NULL;
	if(theApp.m_iMobileCompany == CO_MOTO)
	{
		if(wParam >=1)
			lpszGroup = GetGroupStr(wParam-1);
		else
			lpszGroup = GetGroupStr(wParam);
	}
	else
		lpszGroup = GetGroupStr(wParam);
	

	if(lpszGroup)
		_tcscpy(lpTxt,lpszGroup);
	else
		memset(lpTxt,0,sizeof(TCHAR) * MAX_PATH);
	return 1L;
}

HRESULT CPrevView::OnCardPanelGroupCount(WPARAM wParam,LPARAM lParam)
{
	int *iCount = reinterpret_cast<int*>(wParam);
	*iCount = m_deqGroup.size();
	return 1L;
}


bool CPrevView::LoadDataFromMobile(int iMode)
{
//	m_dlgProg.ShowWindow(SW_SHOW);
//	m_dlgProg.SetFocus();
	if(theApp.m_iMobileCompany == CO_MOTO && (_tcscmp(theApp.m_szPhone,_T("E2")) == 0 ))
	{
		TCHAR szMsg[MAX_PATH*10];
		GetPrivateProfileString(_T("public"),_T("IDS_WARMING_MOTOE2"),_T(""),szMsg,MAX_PATH*10,theApp.m_szRes);
		AfxMessageBox(szMsg);

	}
	::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_PROGDLGSHOW,0,0);
	xStartDriverThread(iMode);
	return true;
}

int CPrevView::GetOutlookData(FileData *pData) 
{
//	m_lpFolder = theApp.m_MSOTDll.MSOT_GetFolderByName( pData->sEntryID,  pData->sStoreID,  ContactFolder);
//	if(m_lpFolder== NULL)
//		return false;
//	m_lpFolder->AddRef();

//	LPDISPATCH dispItem = NULL;
//	dispItem = (LPDISPATCH)m_lpFolder;
//	dispItem->AddRef();	// AddRef to prevent crashes; COM is inscrutible

	CPtrList PhoneBookList;
	CProgressDlg ProgressDlg;
	pData->OTDataList.clear();

	ProgressDlg.SeGetDataToDlg(pData,&PhoneBookList);
	if(ProgressDlg.DoModal() == IDOK)
//	if(theApp.m_MSOTDll.MSOT_GetContact(dispItem,&PhoneBookList,&m_bStop,ProgressCallback,0,100))
	{
		POSITION pos = PhoneBookList.GetHeadPosition();
		while(pos)
		{
			ContactData *pPhonebookData =(ContactData *) PhoneBookList.GetNext(pos);
			if(pPhonebookData)
			{
				GSM_MemoryEntry Entry;
				memset(&Entry,0,sizeof(GSM_MemoryEntry));
				theApp.m_MSOTDll.MSOT_UpdateContactToMobileSupport(&pPhonebookData->Data);
				ContactDataStruct2MemoryEntry(&pPhonebookData->Data,&Entry);
				CCardPanel data;
				data = Entry;
				data.SetStorageType(MSOUTLOOK_NAME);
				pData->OTDataList.push_back(data);
			}
		}
		pData->bLoad = true;
		pos = PhoneBookList.GetHeadPosition();
		while(pos)
		{
			ContactData *pContactData =(ContactData *) PhoneBookList.GetNext(pos);
			if(pContactData) 
			{
				FreeContactSubEntryList(&pContactData->Data);
				delete pContactData;
			}
		}
		PhoneBookList.RemoveAll();
	}
	::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLEFTITEMCOUNT,
				reinterpret_cast<WPARAM>(pData),pData->OTDataList.size());

	return 1;
}
HRESULT CPrevView::OnLeftChange(WPARAM wParam,LPARAM lParam)
{
	FileData *pData = reinterpret_cast<FileData*>(wParam);

	if(pData->sMode == m_LeftMode.sMode){
		if(pData->sMode == CSVFILE && pData->iIndex == m_LeftMode.iIndex)
			return 1L;
		else if( lParam == 0 && (pData->sMode == MOBILE_PHONE || pData->sMode == SIM_CARD 
			|| pData->sMode == MEMORY))
			return 1L;
		else if(pData->sMode == MSOUTLOOK && pData->iIndex == m_LeftMode.iIndex)
			return 1L;
	}

	if(_tcscmp(MOBILE_PHONE,pData->sMode.c_str()) == 0){	//show all data
		m_List.RemoveAllData();
		AddDataToList(m_lsMEData);
		AddDataToList(m_lsSIMData);
	}
	else if(_tcscmp(MEMORY,pData->sMode.c_str()) == 0){	//show memory
		m_List.RemoveAllData();
		AddDataToList(m_lsMEData);
	}
	else if(_tcscmp(SIM_CARD,pData->sMode.c_str()) == 0){	//show sim
		m_List.RemoveAllData();
		AddDataToList(m_lsSIMData);
	}
	else if(_tcscmp(COMPUTER,pData->sMode.c_str()) == 0)	//computer ,show nothing
	{
		m_List.RemoveAllData();
	}
	else if(_tcscmp(CSVFILE,pData->sMode.c_str()) == 0){	//file mode
		m_List.RemoveAllData();
		deque<DATALIST>::iterator  iter = m_deqFileData.begin() + pData->iIndex;
		AddDataToList(*iter);
	}
	else if(_tcscmp(MSOUTLOOK,pData->sMode.c_str()) == 0)
	{
		if( pData->bLoad == false )
		{
			GetOutlookData(pData);
		}
		m_List.RemoveAllData();
	//	deque<DATALIST>::iterator  iter = pData->OTDataList;
		AddDataToList(pData->OTDataList);
	}
	m_LeftMode.iIndex = pData->iIndex;
	m_LeftMode.sFile = pData->sFile;
	m_LeftMode.sMode = pData->sMode;
	m_List.RedrawWindow();
	return 1L;
}

HRESULT CPrevView::OnToolbarReload(WPARAM wParam,LPARAM lParam)
{
	//Clear all data form mobile
	//Clear 
	DATALIST::iterator iter;
	if(m_LeftMode.sMode == MOBILE_PHONE || m_LeftMode.sMode == SIM_CARD)
	{
		for( iter = m_lsSIMData.begin() ; iter != m_lsSIMData.end() ; iter ++){
			(*iter).Release();
		}
		m_lsSIMData.clear();
	}
	if(m_LeftMode.sMode == MOBILE_PHONE || m_LeftMode.sMode == MEMORY)
	{
		for(iter = m_lsMEData.begin() ; iter != m_lsMEData.end() ; iter ++){
			(*iter).Release();
		}
		m_lsMEData.clear();

	}
	m_deqGroup.clear();	

	if(m_LeftMode.sMode == MOBILE_PHONE || m_LeftMode.sMode == MEMORY || m_LeftMode.sMode == SIM_CARD)
		m_List.RemoveAllData();

	if(m_LeftMode.sMode == MEMORY) LoadDataFromMobile(RELOAD_ME);
	else if(m_LeftMode.sMode == SIM_CARD) LoadDataFromMobile(RELOAD_SM);
	else LoadDataFromMobile();
	return 1L;
}

bool CPrevView::xDelListItem(CCardPanel &data)
{
	//remove the data in sim card
	char szDataLocation[MAX_PATH];
	sprintf(szDataLocation,"%s",data.GetLocation());
	if(strcmp(data.GetStorageType() , SIM_NAME) == 0){
		for(DATALIST::iterator iter = m_lsSIMData.begin() ; iter != m_lsSIMData.end() ; iter ++){
			char szLocation[MAX_PATH];
			sprintf(szLocation,"%s",(*iter).GetLocation());
		//	if((*iter).GetLocation() == data.GetLocation()){
			if(strcmp(szLocation,szDataLocation) == 0){
		//	if(strLocation.Compare(data.GetLocation()) == 0){
				(*iter)	.Release();
				m_lsSIMData.erase(iter);
				break;
			}
		}		
	}
	//remove the data in the memory
	else if(strcmp(data.GetStorageType() , MEMORY_NAME) == 0){
		for(DATALIST::iterator iter = m_lsMEData.begin() ; iter != m_lsMEData.end() ; iter ++){
			char szLocation[MAX_PATH];
			sprintf(szLocation,"%s",(*iter).GetLocation());
		//	CString strLocation = (*iter).GetLocation();
	//		if((*iter).GetLocation() == data.GetLocation()){
			if(strcmp(szLocation,szDataLocation) == 0){
				(*iter)	.Release();
				m_lsMEData.erase(iter);
				break;
			}
		}		
	}
	//remove the data in csv file
	if(strcmp(data.GetStorageType(),CSVFILE_NAME) == 0){
		//get current file
		deque<DATALIST>::iterator iterFile = m_deqFileData.begin() + m_LeftMode.iIndex;
		//file the data index in the file
		for (DATALIST::iterator iter = (*iterFile).begin() ; iter != (*iterFile).end() ; iter ++){
//			if((*iter).GetLocation() == data.GetLocation()){
			char szLocation[MAX_PATH];
			sprintf(szLocation,"%s",(*iter).GetLocation());
			if(strcmp(szLocation,szDataLocation) == 0){
				(*iter).Release();
				(*iterFile).erase(iter);
				break;
			}
		}
	}
	return true;
}

HRESULT CPrevView::OnToolbarRestore(WPARAM wParam,LPARAM lParam)
{
	TCHAR *pFile = reinterpret_cast<TCHAR*>(wParam);

	if(pFile){
		//store the path
		if(lParam == -1)
			m_deqFilePath.push_back(pFile);

		CCSVHandle csv(pFile);
		DATALIST lsData;
		//get Contact data from file
		if(csv.GetContact(lsData)){
			//push the file data in the file deque
			if(lParam != -1){
				deque<DATALIST>::iterator iter = m_deqFileData.begin() + lParam;
				*iter = lsData;
			}				
			else
				m_deqFileData.push_back(lsData);
			
			//get left mode from the main window
			FileData fd;

			::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_GETLEFTMODE,reinterpret_cast<WPARAM>(&fd),0);
			
			OnLeftChange(reinterpret_cast<WPARAM>(&fd),1);

			//send message to main frame to set the flag 
			::PostMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLOADFILE,1,0);
			::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLEFTITEMCOUNT,
				reinterpret_cast<WPARAM>(&fd),lsData.size());
		}
	}
	return 1L;
}

HRESULT CPrevView::OnToolbarBackup(WPARAM wParam,LPARAM lParam)
{
	//the filter
#ifdef _UNICODE
	std::wstring file = _T("Comma Separated Values(*.CSV)");
	//file ext name
	std::wstring ext = _T("*.csv");
	//file format
	std::wstring szFormat = file + TCHAR(0) + ext + TCHAR(0); 
#else
	std::string file = _T("Comma Separated Values(*.CSV)");
	//file ext name
	std::string ext = _T("*.csv");
	//file format
	std::string szFormat = file + TCHAR(0) + ext + TCHAR(0); 
#endif
	//get my document path
	TCHAR szMyDoc[MAX_PATH];
	SHGetSpecialFolderPath(NULL,szMyDoc,CSIDL_PERSONAL,FALSE);

	//file default name
	CTime tm = CTime::GetCurrentTime();
	CString str = tm.Format(_T("%Y%m%d"));
	Tstring sFile = Tstring(theApp.m_szMobileName) + Tstring(_T("_")) + Tstring(str);
	Tstring sExt(_T(".csv"));
	//check file exist
	Tstring sPath = Tstring(szMyDoc) + Tstring(_T("\\")) + sFile + sExt;
	CFileFind ff;
	int i = 1;
	while(ff.FindFile(sPath.c_str())){
		TCHAR szTxt[16];
		_stprintf(szTxt,_T("_%d"),i);
		sPath = Tstring(szMyDoc) + Tstring(_T("\\")) + sFile + Tstring(szTxt) + sExt;
		i++;
	}

	//new a CPreviewFileDlg
	CPreviewFileDlg* pDlg = new CPreviewFileDlg( FALSE, ext.c_str(), sPath.c_str(), szMyDoc,
						 OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT /*| OFN_ALLOWMULTISELECT*/, 
						 szFormat.c_str(), this, ::AfxGetInstanceHandle() );

	//set filter index
	pDlg->SetFilterIndex(0);

	int nRet = pDlg->DoModal();

	//if not OK ,do nothing
	if(nRet != IDOK)
	{	
		SAFE_DELPTR(pDlg);
		return 1L;
	}
	
	//Get full path 
	TCHAR szPath[MAX_PATH];
	memset(szPath,0,sizeof(TCHAR) * MAX_PATH);
	pDlg->GetPathName(szPath);
	SAFE_DELPTR(pDlg);

	TCHAR szID[5][64] = {_T("IDS_RECOGNITION"),_T("IDS_MOBILENAME"),_T("IDS_NAME"),_T("IDS_ID_PD"),_T("IDS_TEXT_PD")};
	deque<Tstring> deqHandle;
	int iCount(0);
	if(theApp.m_iMobileCompany == CO_NOKIA)	//nokia
		iCount = 19;
	else if(theApp.m_iMobileCompany == CO_ASUS && (_tcscmp(theApp.m_szPhone,_T("J202")) != 0))	//asus
		iCount = 31;
	else if(theApp.m_iMobileCompany == CO_MOTO)	//moto
	{
		if((_tcscmp(theApp.m_szPhone,_T("E550")) == 0 ) || (_tcscmp(theApp.m_szPhone,_T("V620")) == 0) || 
			(_tcscmp(theApp.m_szPhone,_T("V550")) == 0) || (_tcscmp(theApp.m_szPhone,_T("E1000")) == 0) ||
			(_tcscmp(theApp.m_szPhone,_T("V600i")) == 0)|| (_tcscmp(theApp.m_szPhone,_T("V360")) == 0)||
			(_tcscmp(theApp.m_szPhone,_T("E1")) == 0)|| (_tcscmp(theApp.m_szPhone,_T("L6")) == 0)||
			(_tcscmp(theApp.m_szPhone,_T("L7")) == 0)|| (_tcscmp(theApp.m_szPhone,_T("V361")) == 0)||
			(_tcscmp(theApp.m_szPhone,_T("PEBL U6")) == 0)|| (_tcscmp(theApp.m_szPhone,_T("V3i")) == 0))
		iCount = 13;
		else iCount = 5;
	}
	else if(theApp.m_iMobileCompany == CO_SONY_ERISON)
	{
		if(_tcscmp(theApp.m_szPhone,_T("T610")) == 0 || _tcscmp(theApp.m_szPhone,_T("T630")) == 0|| _tcscmp(theApp.m_szPhone,_T("Z600")) == 0
			|| _tcscmp(theApp.m_szPhone,_T("T616")) == 0|| _tcscmp(theApp.m_szPhone,_T("T637")) == 0 || _tcscmp(theApp.m_szPhone,_T("T68i")) == 0
			|| _tcscmp(theApp.m_szPhone,_T("T628")) == 0) // add Z600 , by mingfa
			iCount = 9;
/*		else if(strcmp(theApp.m_szPhone,_T("K700i")) == 0 || strcmp(theApp.m_szPhone,_T("S700i")) == 0 || strcmp(theApp.m_szPhone,_T("K500i")) == 0 ||
			    strcmp(theApp.m_szPhone,_T("F500i")) == 0 || strcmp(theApp.m_szPhone,_T("K300i")) == 0 || strcmp(theApp.m_szPhone,_T("V800")) == 0  ||
				strcmp(theApp.m_szPhone,_T("Z1010")) == 0 || strcmp(theApp.m_szPhone,_T("P900")) == 0 || strcmp(theApp.m_szPhone,_T("P900i")) == 0 ||
				strcmp(theApp.m_szPhone,_T("K750i")) == 0)  // add new model( F500i ~ P900i) , by mingfa
			iCount = 16;
		else 
			iCount = 10;*/
		else if(_tcscmp(theApp.m_szPhone,_T("T230")) == 0 || _tcscmp(theApp.m_szPhone,_T("T290i")) == 0
			|| _tcscmp(theApp.m_szPhone,_T("T300")) == 0 || _tcscmp(theApp.m_szPhone,_T("T300c")) == 0|| _tcscmp(theApp.m_szPhone,_T("T68")) == 0) 
			iCount = 10;
		else 
			iCount = 16;
	}
	else if(theApp.m_iMobileCompany == CO_SIMENS)	//simens
		iCount = 17;
	else if(theApp.m_iMobileCompany == CO_SAMSUNG)	
		iCount = 10;
	else
		iCount = 20;


	for( i = 0 ; i < iCount ; i ++)
	{
		TCHAR szTxt[MAX_PATH];
		if(i < 3){
			if(al_GetSettingString(_T("public"),szID[i],theApp.m_szRes,szTxt)){
				deqHandle.push_back(szTxt);
			}
		}
		else if( i % 2 == 1){	//ID
			TCHAR szTmp[MAX_PATH];
			if(al_GetSettingString(_T("public"),szID[3],theApp.m_szRes,szTmp)){
				wsprintf(szTxt,szTmp,(i - 1) / 2);
				deqHandle.push_back(szTxt);
			}
		}
		else{	//text
			TCHAR szTmp[MAX_PATH];
			if(al_GetSettingString(_T("public"),szID[4],theApp.m_szRes,szTmp)){
				wsprintf(szTxt,szTmp,(i - 1) / 2);
				deqHandle.push_back(szTxt);
			}
		}
	}
	


	CCSVHandle csv(szPath);
	//write handle deque to file
	csv.WriteHandle(deqHandle);

	//write the Contact data to file
	if(m_LeftMode.sMode == MOBILE_PHONE){
		csv.WriteContact(m_lsMEData);
		csv.WriteContact(m_lsSIMData);
	}
	else if(m_LeftMode.sMode == MEMORY){
		csv.WriteContact(m_lsMEData);
	}
	else if(m_LeftMode.sMode == SIM_CARD){
		csv.WriteContact(m_lsSIMData);
	}
	if(theApp.m_iMobileCompany == CO_NOKIA)
		csv.WriterDescription();

	return 1L;
}

void CPrevView::SaveCSVFile()
{
	//file path
	deque<Tstring>::iterator iterPath = m_deqFilePath.begin();
	//file data list
	deque<DATALIST>::iterator iterFile = m_deqFileData.begin();

	for(int i = 0 ; i < m_deqFilePath.size() ; i++){
		//check end of the deque
		if(iterPath == m_deqFilePath.end() || iterFile == m_deqFileData.end())
			break;

		CCSVHandle csv((*iterPath).c_str());
		//get the handle
		deque<Tstring> deqHandle;
		csv.GetHandle(deqHandle);
		//clear the file and write the handle
		csv.WriteHandle(deqHandle);
		//write the Contact
		csv.WriteContact(*iterFile);
		//get next file and path
		iterPath ++;
		iterFile ++;
	}
}


void CPrevView::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	*pResult = 0;
	
	// Create the drag&drop source and data objects
   	COleDataSource ds;

	if(ds.DoDragDrop() == DROPEFFECT_MOVE )
	{
	}

}
HRESULT CPrevView::OnToolbarReloadMSOT(WPARAM wParam,LPARAM lParam)
{
 	FileData *pData = reinterpret_cast<FileData*>(lParam);
	if(_tcscmp(MSOUTLOOK,pData->sMode.c_str()) == 0 && pData->iIndex < 0)
	{
		 GetOutlookData(pData);
		m_List.RemoveAllData();
		AddDataToList(pData->OTDataList);
	}
	return 1L;
}
HRESULT CPrevView::OnLeftDropData2MSOT(WPARAM wParam,LPARAM lParam)
{
	FileData *pData = reinterpret_cast<FileData*>(lParam);
	CPtrList PhoneBookList;
	PhoneBookList.RemoveAll();
	DATALIST lsData;
	int i = 0 ;
	if(m_List.GetSelectedData(lsData))
	{
	/*	for(DATALIST::iterator iter = lsData.begin() ; iter != lsData.end() ; iter ++,i++)
		{
			CCardPanel data =*iter;
			GSM_MemoryEntry Entry;
			memset(&Entry,0,sizeof(GSM_MemoryEntry));
			data.GetGSMMemoryEntry(Entry);
			ContactData* pContact_Data = new ContactData;
			MemoryEntry2ContactDataStruct(&Entry,&pContact_Data->Data);
			PhoneBookList.AddTail(pContact_Data);
		}*/
//	}

		CProgressDlg ProgressDlg;
		ProgressDlg.SetAddDataToDlg(pData,&lsData,&PhoneBookList);
		if(ProgressDlg.DoModal() == IDOK)
		{
			POSITION pos = PhoneBookList.GetHeadPosition();
			while(pos)
			{
				ContactData* pCData = (ContactData*) PhoneBookList.GetNext(pos);
				GSM_MemoryEntry Entry;
				memset(&Entry,0,sizeof(GSM_MemoryEntry));
				ContactDataStruct2MemoryEntry(&pCData->Data,&Entry);
				CCardPanel *data= new CCardPanel;
				*data = Entry;
				data->SetStorageType(MSOUTLOOK_NAME);
				pData->OTDataList.push_back(*data);
				data->Release();
				SAFE_DELPTR(data);
			}
		}
		POSITION pos = PhoneBookList.GetHeadPosition();
		while(pos)
		{
			ContactData* pCData = (ContactData*) PhoneBookList.GetNext(pos);
			if(pCData) 
			{
				FreeContactSubEntryList(&pCData->Data);		
				delete pCData;
			}

		}
		PhoneBookList.RemoveAll();
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLEFTITEMCOUNT,
			reinterpret_cast<WPARAM>(pData),pData->OTDataList.size());

	}
	return 1L;
}

HRESULT CPrevView::OnLeftDropData(WPARAM wParam,LPARAM lParam)
{
	m_sAddMode = reinterpret_cast<LPCTSTR>(wParam);
//	m_dlgProg.ShowWindow(SW_SHOW);
//	m_dlgProg.SetFocus();
	::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_PROGDLGSHOW,0,0);
	xStartDriverThread(ADD_DATA);
	return 1L;
}

HRESULT CPrevView::OnLoadMobile(WPARAM wParam,LPARAM lParam)
{
	m_Driver.init();
	LoadDataFromMobile();
	return 1L;
}

bool CPrevView::IsItemInList()
{
	return m_List.GetItemCount() > 0 ? true : false;
}

int CPrevView::GetListItemSelectedCount()
{
	return m_List.GetSelectedCount();
}

void CPrevView::OnDraw(CDC* pDC)
{
}

HRESULT CPrevView::OnOpenDriver(WPARAM wParam,LPARAM lParam)
{
	//open the sync driver
	CMainFrame* pFrame=(CMainFrame*)afxGetMainWnd();
	if(!m_Driver.Open(theApp.m_iMobileCompany,theApp.m_szPhone,theApp.m_szPortNum,theApp.m_szConnectMode,static_cast<CMainFrame*>(afxGetMainWnd())->ConnectStatusCallback)){
		TCHAR szText[MAX_PATH],szTmp[MAX_PATH];
		al_GetSettingString(_T("public"),_T("IDS_ERR_OPEN"),theApp.m_szRes,szTmp);
		wsprintf(szText,szTmp,theApp.m_szMobileName);
		AfxMessageBox(szText);
		::SendMessage(pFrame->GetSafeHwnd(),WM_SETLINKMOBILE,0,0);
	}		
/*	else
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLINKMOBILE,1,0);*/
	else
	{
		::SetCapture(pFrame->m_hWnd);
		SetCursor( LoadCursor(NULL, IDC_WAIT) );
		pFrame->m_bCapture = TRUE;
	}
	pFrame->m_bSupportDirectSMS = m_Driver.IsSupportPhonebookDirectSMS();

	return 1L;
}

void CPrevView::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;

	LPNMITEMACTIVATE  lpitat = (LPNMITEMACTIVATE)pNMHDR;

	switch(lplvcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT :
		{
			*pResult = CDRF_NOTIFYITEMDRAW;
			return;
		}
		// Modify item text and or background
		case CDDS_ITEMPREPAINT:
		{
   			HDC			hDC			= lplvcd->nmcd.hdc;
			LV_DISPINFO lvdi;
	   		int			iItem		= lplvcd->nmcd.dwItemSpec;
	   		CRect		rc			/*= lplvcd->nmcd.rc*/;
   			TCHAR		szText[1024];
			m_List.GetItemRect(iItem,&rc,LVIR_ICON);
			BOOL bSel = m_List.GetItemState(iItem,LVIS_SELECTED) & LVIS_SELECTED;

			memset(&lvdi,0,sizeof(LV_DISPINFO));
			//get the item to redraw
   			lvdi.item.mask			= LVIF_IMAGE | LVIF_PARAM | 
   									  LVIF_STATE | LVIF_TEXT;
   			lvdi.item.iItem			= iItem;
   			lvdi.item.pszText		= szText;
   			lvdi.item.cchTextMax	= 1024;
   			if(!m_List.GetItem(&lvdi.item))
   				goto error_exit;		// Exit silently (should never happen, though)

   			//
   			// Now, check for callback items
   			//
   			if(lvdi.item.iImage			== I_IMAGECALLBACK ||
   			   lvdi.item.pszText		== LPSTR_TEXTCALLBACK )
   			{
   				HWND	hWndParent;

   				hWndParent = ::GetParent(m_hWnd);
   				if(hWndParent)
   				{
   					lvdi.hdr.hwndFrom	= m_hWnd;
   					lvdi.hdr.idFrom		= ::GetDlgCtrlID(m_hWnd);
   					lvdi.hdr.code		= LVN_GETDISPINFO;

   					::SendMessage(hWndParent, WM_NOTIFY, lvdi.hdr.idFrom,
   									(LPARAM)&lvdi);
   				}
   			}

			//draw in report mode
			if(m_List.GetViewMode() == 0){
				CCardPanel *pData = reinterpret_cast<CCardPanel*>(lvdi.item.lParam);
/*				if(pData && ( m_LeftMode.sMode == MOBILE_PHONE || m_LeftMode.sMode == MEMORY 
					|| m_LeftMode.sMode == SIM_CARD)){
					rc.left += 2;
				}*/
				//fill background
				int iLength = 0;
				//get all column length
				for(int i = 0 ; i < 4 ; i ++) 
					iLength += m_List.GetColumnWidth(i);

				rc.right = rc.left + iLength;
				//fill each row 
				CRect rcTmp(rc),rcClient;
				GetClientRect(rcClient);
				rcTmp.left = 0;
				rcTmp.right = rcTmp.left + rcClient.Width();
				if(lpitat->ptAction.y % 2 && hDC){
					CBrush brush(m_crCol2);
					::FillRect(hDC,rcTmp,brush);

				}
				else{
					CBrush brush(m_crCol1);
					::FillRect(hDC,rcTmp,brush);
				}
				//draw icon
				int iIconWidth = 0 , iIconHeight = 0;
//				CCardPanel *pData = reinterpret_cast<CCardPanel*>(lvdi.item.lParam);
				if(pData && ( m_LeftMode.sMode == MOBILE_PHONE || m_LeftMode.sMode == MEMORY 
					|| m_LeftMode.sMode == SIM_CARD)){
					CImage *pimg = NULL;
					if(strcmp(pData->GetStorageType(),SIM_NAME) == 0)
						pimg = &m_imgIcon[SIM_ICON];
					else if(strcmp(pData->GetStorageType(),MEMORY_NAME) == 0)
						pimg = &m_imgIcon[PHONE_ICON];
					int iWidth = m_List.GetColumnWidth(0);
					if(pimg){
						pimg-> Display(hDC, rc.left, rc.top, iWidth - 2 > pimg->Width() ? pimg->Width() :
							iWidth - 2, pimg->Height());
						//set icon width and height
						iIconWidth = iWidth > pimg->Width() ? pimg->Width() : iWidth - 2;
						iIconHeight = pimg->Height();
					}
				}

				//draw string
				CRect rcText(rc.left+iIconWidth , rc.top ,rc.left+iIconWidth,rc.bottom);
				::SetBkMode(hDC, TRANSPARENT);
				COLORREF crOldText = 0 ;
				for(i = 0 ; i < 3 ; i ++){
					//get subitem text
					TCHAR szText[MAX_PATH];
					int iLeng = m_List.GetItemText(lvdi.item.iItem,i,szText,MAX_PATH);
					if(iLeng == 0)
						_tcscpy(szText,_T(" "));
					//get subcolumn width
					int iWidth = m_List.GetColumnWidth(i);
					//if it's first one , decrase icon width
					if( i == 0)
					{
						iWidth -= iIconWidth;
						rcText.right+=2;
					}
					//set subitem rect
					rcText.left = rcText.right;
					rcText.right = rcText.left + iWidth;
					
   					//
   					// Clear the background
   					//
					CRect rcTemp;
					if( i == 0){
						HBRUSH hBrush = NULL;
						//check every style and set color
						//focus and selected
    					if(lplvcd->nmcd.uItemState & CDIS_SELECTED )
   						{
   							crOldText = ::SetTextColor	(hDC, m_crItemTextSel);
					//		m_crItemBgSel = ::GetSysColor(COLOR_HIGHLIGHT);
							hBrush = CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHT));
   						}
  						else if(lplvcd->nmcd.uItemState & CDIS_FOCUS && lplvcd->nmcd.uItemState & CDIS_SELECTED)
   						{
   							crOldText = ::SetTextColor	(hDC, m_crItemTextFocus);
					//		m_crItemBgFocus = ::GetSysColor(COLOR_HIGHLIGHT);
							hBrush = CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHT)); 
   						}
						else if(bSel)
						{
   							crOldText = ::SetTextColor	(hDC, m_crItemTextNor);
							hBrush = CreateSolidBrush(::GetSysColor(COLOR_INACTIVEBORDER));
						}
						//selected
						//otherwise
   						else
   						{
							crOldText = ::SetTextColor	(hDC, m_crItemTextNor);
   						}

						//if it is selected , draw focus rectangle to first subitem
						//measure draw string length
						
   						::DrawText(hDC, szText, -1, &rcTemp, DT_LEFT | DT_NOPREFIX | 
   									DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);
						
						CRect rc(rcText);
						//compare string length and column width 
						if(rcTemp.Width() < iWidth)
							rc.right = rcText.left + rcTemp.Width();
						else
							rc.right = rcText.left + iWidth - 3;
						//if item has been selected and focus,fill the focus rectangle 				
						if(lplvcd->nmcd.uItemState & CDIS_FOCUS || lplvcd->nmcd.uItemState & CDIS_SELECTED||bSel){
					//		CDC *pDC = m_List.GetDC();
							CRect rcFocus;
							m_List.GetItemRect(iItem,&rcFocus,LVIR_ICON);
							int iLength = 0;
							//get all column length
							for(int i = 0 ; i < 3 ; i ++) 
								iLength += m_List.GetColumnWidth(i);

							rcFocus.right = rcFocus.left + iLength;
							rcFocus.left = rc.left;
							rcFocus.right-=4;
							if(hBrush)
								::FillRect(hDC,rcFocus,hBrush);//for LVS_EX_FULLROWSELECT
							if(lplvcd->nmcd.uItemState & CDIS_FOCUS)
							{
								::DrawFocusRect(hDC, &rcFocus);//for LVS_EX_FULLROWSELECT
							}
						//	ReleaseDC(pDC);
					//		if(hBrush)
					//			::FillRect(hDC,rc,hBrush);
					//		::DrawFocusRect(hDC, &rc);
						}
						DeleteObject(hBrush);
					}
					//draw string to dc
					rcTemp = rcText;
					rcTemp.right -= 3;
					::DrawText(hDC,szText,_tcslen(szText),rcTemp,DT_LEFT | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE);
				//	if( i == 0)
			//		{
			//			::SetTextColor	(hDC, crOldText);
				//	}
				}

				::SetTextColor	(hDC, crOldText);

				*pResult = CDRF_SKIPDEFAULT;
				return;
			}
error_exit:
//   			*pResult = CDRF_SKIPDEFAULT;
			// If you want the sub items the same as the item,
			// set *pResult to CDRF_NEWFONT
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
			return;
		}

		// Modify sub item text and/or background
		case CDDS_SUBITEM | CDDS_PREPAINT | CDDS_ITEM:
		{

			*pResult = CDRF_NEWFONT;
			return;
		}
	}
 
}

void CPrevView::SetDriverOpenFlag(bool bOpen)
{
	m_Driver.SetOpenFlag(bOpen);
}

UINT CPrevView::xDriverThread(int iMode)
{
	m_pActivate->BeginWaitCursor();
	//initial progress bar
	m_pActivate->m_dlgProg.ShowWindow(SW_SHOW);
	m_pActivate->m_dlgProg.SetFocus();

	m_pActivate->m_dlgProg.SetRange(0,100);
	m_pActivate->m_dlgProg.SetPos(0);
	bool bFail = false;
	if(iMode == SEND_SMS)
	{
		int iErr =1;
		m_pActivate->m_dlgProg.SetTitle(PROG_SENDSMS);
		int nSendCount = m_pActivate->m_NeedSendSMSList.GetCount();
		if(nSendCount >0)
		{
			int i = 0;
			POSITION pos = m_pActivate->m_NeedSendSMSList.GetHeadPosition();
			while(pos)
			{
				int result = ::WaitForSingleObject(m_pActivate->m_evThreadBreak.m_hObject,0);
				if(result == WAIT_OBJECT_0)
						break;
				SMS_PARAM	*pData = (SMS_PARAM	*)m_pActivate->m_NeedSendSMSList.GetNext(pos);
				if(pData == NULL) continue;
				i++;
				m_pActivate->m_dlgProg.SetPos((( i * 100 ) / nSendCount));
				//Send SMS +
				SMS_Data_Strc smsSend;
				memset(&smsSend, 0, sizeof(SMS_Data_Strc));
				_tcscpy(smsSend.TP_UD, pData->TP_UD); 
				_tcscpy(smsSend.TPA, pData->TPA);
				smsSend.TP_DCS = pData->TP_DCS;

				iErr = m_pActivate->m_Driver.SendSMSData(MEM_SM, 0, &smsSend);
				//Send SMS -

			}
		}
	}
	else if(iMode == DEL_DATA)
	{
		//Set dialog title
		m_pActivate->m_dlgProg.SetTitle(PROG_DEL);
		//Get the selected data
		DATALIST lsData;
		if(m_pActivate->m_List.GetSelectedData(lsData)){
			//sort list
			SortList(lsData);
			//delete data from driver
			int i = 0;

			Control_Type type(Type_Start);
			int iErr =1;
			string sStorage;
			for(DATALIST::iterator iter = lsData.begin() ; iter != lsData.end() ; iter ++,i++){
				//check break or not
				int result = ::WaitForSingleObject(m_pActivate->m_evThreadBreak.m_hObject,0);
				if(result == WAIT_OBJECT_0)
						break;
				
				if(iter == lsData.begin()){
					sStorage = (*iter).GetStorageType();
					if(strcmp(sStorage.c_str(),"ME") == 0)
						m_pActivate->m_bDeleteDatainME= true;
					type = Type_Start;
				}
				else if(strcmp((*iter).GetStorageType(),sStorage.c_str()) != 0){	//end
					m_pActivate->m_Driver.EndDeleteContactData(sStorage.c_str());
					sStorage = (*iter).GetStorageType();
					if(strcmp(sStorage.c_str(),"ME") == 0)
						m_pActivate->m_bDeleteDatainME= true;
					type = Type_Start;
				}
				else
					type = Type_Continune;

				//set progress pos
				int iPos = ( (i + 1) * 100 ) / lsData.size();
				m_pActivate->m_dlgProg.SetPos(iPos);
				//delete data from mobile
				bool bSuc = m_pActivate->m_Driver.DeleteContactData((*iter),type,iErr);
				if(bSuc && iErr ==1){
					//delete list item from list
					m_pActivate->xDelListItem(*iter);				
				}
				else
					break;
			}
			m_pActivate->m_Driver.EndDeleteContactData(sStorage.c_str());
			m_pActivate->m_Driver.xErrorHandling(iErr);
			//delete data from listctrl
//			m_pActivate->m_List.DeleteSel();
			m_pActivate->m_List.RedrawItems(0,m_pActivate->m_List.GetItemCount()-1);
			m_pActivate->xSetLeftMobileItemCount();
		}		
	}
	else if(iMode == GET_DATA ||iMode == RELOAD_ME ||iMode == RELOAD_SM)
	{

		CString sFormat;
		bool bBreak = false;
		int iME = 0, iSIM = 0;
		int iSIMFree = 0 , iSIMUsed = 0, iMEFree = 0,iMEUsed = 0;
		//get group list name 
		m_pActivate->m_Driver.GetGroupList(m_pActivate->m_deqGroup);

		//get support column
		m_pActivate->m_Driver.GetSupportCol(m_pActivate->m_SupportEntry);

		//get free space in mobile
		switch(iMode)
		{
		case  RELOAD_ME:
			if(!m_pActivate->m_Driver.GetMEContactInfo(iMEUsed,iMEFree)){
				bFail = true;
				goto end_thread;
			}
			break;
		case  RELOAD_SM:
			if(!m_pActivate->m_Driver.GetSMContactInfo(iSIMUsed,iSIMFree)){
				bFail = true;
				goto end_thread;
			}
			break;
		default:
			if(!m_pActivate->m_Driver.GetContactInfo(iSIMUsed,iSIMFree,iMEUsed,iMEFree)){
				bFail = true;
				goto end_thread;
			}
			break;
		}


		if(iMode == GET_DATA ||iMode == RELOAD_ME)
		{

			if(iMEUsed < 0)
			{
				m_pActivate->m_dlgProg.SetTitle(PROG_INIT);
				bool bRet = m_pActivate->m_dlgProg.OpenAvi();
				bRet = m_pActivate->m_dlgProg.PlayAvi();
				bool bSuc = m_pActivate->m_Driver.InitialMemory(MEMORY_NAME,iMEUsed,iMEFree);
				if(bRet)
					m_pActivate->m_dlgProg.StopAvi();
					m_pActivate->m_dlgProg.CloseAvi();
				if(!bSuc)
				{
					bFail = true;
					((CPhoneBookApp*)afxGetApp())->m_bNotSupportSM = TRUE;
					((CPhoneBookApp*)afxGetApp())->m_bNotSupportME = TRUE;
					::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_CHECKLEFTTREEITEM,0,0);
					goto end_thread;
				}
			}
			if((iMEUsed+iMEFree) == 0)
				((CPhoneBookApp*)afxGetApp())->m_bNotSupportME = TRUE;
			else
				((CPhoneBookApp*)afxGetApp())->m_bNotSupportME = false;

			//Set dialog title
			m_pActivate->m_dlgProg.SetTitle(PROG_TRAN_ME);

			if(iMEUsed >= 0)
			{
				//Get contact data one by one
			
				bool bStart = true;
				for(iME = 0 ; iME < iMEUsed ; iME ++)
				{
					//check break;
					int result = ::WaitForSingleObject(m_pActivate->m_evThreadBreak.m_hObject,0);
					if(result == WAIT_OBJECT_0)
					{
						bBreak = true;
						bFail = true;
						break;
					}						

					//set progress pos
	//				int iPos = ( (iME + 1 + iSIMUsed) * 100 ) / (iMEUsed + iSIMUsed);
					int iPos = ( (iME + 1) * 100 ) / iMEUsed;
					m_pActivate->m_dlgProg.SetPos(iPos);

					//get contact data
					CCardPanel data;
					if(!m_pActivate->m_Driver.GetContactData(MEMORY_NAME,data,bStart))
					{
						bBreak = true;
						bFail = true;
						break;
					}
					else
					{
						
						//push data to list
						m_pActivate->m_lsMEData.push_back(data);
						
						bStart = false;
					}
				}
			}
		}
		if(iMode == GET_DATA ||iMode == RELOAD_SM)
		{

			if(iSIMUsed < 0)
			{
				m_pActivate->m_dlgProg.SetTitle(PROG_INIT);
				bool bRet = m_pActivate->m_dlgProg.OpenAvi();
				bRet = m_pActivate->m_dlgProg.PlayAvi();
				bool bSuc = m_pActivate->m_Driver.InitialMemory(SIM_NAME,iSIMUsed,iSIMFree);
				if(bRet)
				{
					m_pActivate->m_dlgProg.StopAvi();
					m_pActivate->m_dlgProg.CloseAvi();
				if(!bSuc)
				{
					bFail = true;
					((CPhoneBookApp*)afxGetApp())->m_bNotSupportSM = TRUE;
					((CPhoneBookApp*)afxGetApp())->m_bNotSupportME = TRUE;
					::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_CHECKLEFTTREEITEM,0,0);
					goto end_thread;
				}

				}
			}
			if((iSIMUsed+iSIMFree) == 0)
				((CPhoneBookApp*)afxGetApp())->m_bNotSupportSM = TRUE;
			else
				((CPhoneBookApp*)afxGetApp())->m_bNotSupportSM = false;


			//Set dialog title
			m_pActivate->m_dlgProg.SetTitle(PROG_TRAN_SM);

			if(iSIMUsed >= 0 && !bBreak){
				//Get contact data one by one
				bool bStart = true;
				for(iSIM = 0 ; iSIM < iSIMUsed ; iSIM ++){
					//check break;
					int result = ::WaitForSingleObject(m_pActivate->m_evThreadBreak.m_hObject,0);
					if(result == WAIT_OBJECT_0){
	//					bBreak = true;
						bFail = true;
						break;
					}

					//set progress pos
	//				int iPos = ( (iSIM + 1 ) * 100 ) / (iMEUsed + iSIMUsed);
					int iPos = ( (iSIM + 1 ) * 100 ) / iSIMUsed;
					m_pActivate->m_dlgProg.SetPos(iPos);

					CCardPanel data;
					//get contact data
					if(!m_pActivate->m_Driver.GetContactData(SIM_NAME,data,bStart)){
						bFail = true;
	//					bBreak = true;
						break;
					}
					else{
						//push data to list
						m_pActivate->m_lsSIMData.push_back(data);
						bStart = false;
					}
				}
			}
		}
		::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_CHECKLEFTTREEITEM,0,0);
		m_pActivate->xSetLeftMobileItemCount();
	}
	else{	//add data
		//Set dialog title
		m_pActivate->m_dlgProg.SetTitle(PROG_TRANSFER);
		//Get the selected data
		DATALIST lsData;
		int i = 0 ;
		if(m_pActivate->m_List.GetSelectedData(lsData)){
			//delete data from driver
			int iErr=1;
			Control_Type type(Type_Start);
			string sStorage;
			for(DATALIST::iterator iter = lsData.begin() ; iter != lsData.end() ; iter ++,i++){
				//check break;
				int result = ::WaitForSingleObject(m_pActivate->m_evThreadBreak.m_hObject,0);
				if(result == WAIT_OBJECT_0)
					break;

				//set progress pos
				int iPos = ( (i + 1) * 100 ) / lsData.size();
				m_pActivate->m_dlgProg.SetPos(iPos);

				
				//check the data memory type. If sMode and data are the same, do not add.
				//Set data memory type
				if(m_pActivate->m_sAddMode == SIM_CARD)
					if(strcmp((*iter).GetStorageType(),SIM_NAME) == 0)
						continue;
					else
						(*iter).SetStorageType(SIM_NAME);
				else
					if(strcmp((*iter).GetStorageType(),MEMORY_NAME) == 0)
						continue;
					else
						(*iter).SetStorageType(MEMORY_NAME);
				if(type == Type_Start)
					sStorage = (*iter).GetStorageType();

		/*		if(iter == lsData.begin()){
					sStorage = (*iter).GetStorageType();
					type = Type_Start;
				}
				else
					type = Type_Continune;*/
				//update data to mobile
				bool bSuc = m_pActivate->m_Driver.AddContactData((*iter),type,iErr);
				if(bSuc == 1) type = Type_Continune;

				if(bSuc == 1 && iErr ==1 ){
					//check the memory type ,add data to the list
					CCardPanel *pData = new CCardPanel;
					*pData = (*iter);
//					pData->ResetImgBuffer();
					if(m_pActivate->m_sAddMode == SIM_CARD){
						m_pActivate->m_lsSIMData.push_back(*pData);
					}
					else{
						m_pActivate->m_lsMEData.push_back(*pData);
					}
					pData->Release();
					SAFE_DELPTR(pData);
					type = Type_Continune;
				}
				else if(iErr != Anw_INVALIDDATA)
					break;
				else
				{
					if(bSuc == 1 ||type == Type_Continune )
						m_pActivate->m_Driver.EndAddContactData(sStorage.c_str());
					m_pActivate->m_Driver.xErrorHandling(iErr);
					iErr = 1;
					type = Type_Start;
				}
			}
			if(type == Type_Continune )
				m_pActivate->m_Driver.EndAddContactData(sStorage.c_str());
			m_pActivate->m_Driver.xErrorHandling(iErr);
			m_pActivate->xSetLeftMobileItemCount();
		}
	}

end_thread:	
	m_pActivate->xDownloadFail(bFail);
	m_pActivate->EndWaitCursor();	
	::PostMessage(m_pActivate->GetSafeHwnd(),WM_THREADEND,0,0);
	
	return 0;
}
void CPrevView::xStartSendMessageThread()
{
	if(!m_pThread){
		m_iMode = SEND_SMS;
		m_evThreadBreak.ResetEvent();
		m_evThreadEnd.ResetEvent();
		m_pThread = ::AfxBeginThread((AFX_THREADPROC)xDriverThread,reinterpret_cast<LPVOID>(m_iMode),THREAD_PRIORITY_NORMAL);
		m_evThreadStart.SetEvent();
	}
}

void CPrevView::xStartDriverThread(int iMode)
{
	if(!m_pThread){
		m_iMode = iMode;
		m_List.RedrawWindow();
		m_evThreadBreak.ResetEvent();
		m_evThreadEnd.ResetEvent();
		m_pThread = ::AfxBeginThread((AFX_THREADPROC)xDriverThread,reinterpret_cast<LPVOID>(iMode),THREAD_PRIORITY_NORMAL);
		m_evThreadStart.SetEvent();
	}
}

HRESULT CPrevView::OnThreadBreak(WPARAM wParam,LPARAM lParam)
{
	m_evThreadBreak.SetEvent();
	return 1L;
}

HRESULT CPrevView::OnThreadEnd(WPARAM wParam,LPARAM lParam)
{
	m_dlgProg.ResetCtrlView();
	m_dlgProg.ShowWindow(SW_HIDE);
	::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_PROGDLGSHOW,1,0);
	afxGetMainWnd()->SetFocus();
	OnLeftChange(reinterpret_cast<WPARAM>(&m_LeftMode),1);
	m_pThread = NULL;
	m_List.SetFocus();
	m_List.Invalidate(FALSE);
//	m_List.RedrawItems(0,m_List.GetItemCount() - 1);
/*	if(m_iMode == DEL_DATA && 	m_bDeleteDatainME)
	{
		 if(theApp.m_iMobileCompany == CO_SAMSUNG && (strcmp(theApp.m_szPhone,"E738") == 0  || strcmp(theApp.m_szPhone,"E648") == 0))
		 {
		//	 OnToolbarReload(0,0);
			//Clear 
			for(DATALIST::iterator iter = m_lsSIMData.begin() ; iter != m_lsSIMData.end() ; iter ++){
				(*iter).Release();
			}
			m_lsSIMData.clear();

			for(iter = m_lsMEData.begin() ; iter != m_lsMEData.end() ; iter ++){
				(*iter).Release();
			}
			m_lsMEData.clear();

			m_deqGroup.clear();	

		//	if(m_LeftMode.sMode == MOBILE_PHONE || m_LeftMode.sMode == MEMORY || m_LeftMode.sMode == SIM_CARD)
		//		m_List.RemoveAllData();

			LoadDataFromMobile();
		//	return 1L;
		 }
	}*/

	return 1L;
}


BOOL CPrevView::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;	
//	return CView::OnEraseBkgnd(pDC);
}

/*
HRESULT CPrevView::OnGetListSize(WPARAM wParam,LPARAM lParam)
{
	TCHAR *pModel = reinterpret_cast<TCHAR*>(wParam);
	int *iNum = reinterpret_cast<int *>(lParam);

	if(strcmp(pModel,MEMORY_NAME) == 0){
		*iNum = m_lsMEData.size();
	}
	else if(strcmp(pModel,SIM_NAME) == 0){
		*iNum = m_lsSIMData.size();
	}
	return 1L;
}
*/
void CPrevView::SetConnectStatus(bool bFlag)
{
	if(!bFlag){
		if((m_LeftMode.sMode == MOBILE_PHONE || m_LeftMode.sMode == MEMORY ||m_LeftMode.sMode == SIM_CARD))
			m_List.RemoveAllData();

		for(DATALIST::iterator iter = m_lsSIMData.begin() ; iter != m_lsSIMData.end() ; iter ++){
			(*iter).Release();
		}
		m_lsSIMData.clear();

		for(iter = m_lsMEData.begin() ; iter != m_lsMEData.end() ; iter ++){
			(*iter).Release();
		}
		m_lsMEData.clear();

		m_deqGroup.clear();	

		xSetLeftMobileItemCount();
	}
}

void CPrevView::xSetLeftMobileItemCount()
{
	//send message to left view to store the count
	FileData fd;
	fd.sMode = MEMORY;
	::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLEFTITEMCOUNT,
		reinterpret_cast<WPARAM>(&fd),m_pActivate->m_lsMEData.size());
	fd.sMode = SIM_CARD;
	::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLEFTITEMCOUNT,
		reinterpret_cast<WPARAM>(&fd),m_pActivate->m_lsSIMData.size());
	fd.sMode = MOBILE_PHONE;
	::SendMessage(afxGetMainWnd()->GetSafeHwnd(),WM_SETLEFTITEMCOUNT,
		reinterpret_cast<WPARAM>(&fd),m_pActivate->m_lsMEData.size() + m_pActivate->m_lsSIMData.size());
}

bool CPrevView::CheckProgDlg()
{
	return m_dlgProg.IsWindowVisible() ? true : false;
}

void CPrevView::OnKillFocus( CWnd* pNewWnd )
{
	CView::OnKillFocus(pNewWnd);
	m_List.RedrawWindow();
}

void CPrevView::xDownloadFail(bool bFail)
{
	if(bFail){
		//clear the list control
		if((m_LeftMode.sMode == MOBILE_PHONE || m_LeftMode.sMode == MEMORY ||m_LeftMode.sMode == SIM_CARD))
			m_List.RemoveAllData();

		for(DATALIST::iterator iter = m_lsSIMData.begin() ; iter != m_lsSIMData.end() ; iter ++){
			(*iter).Release();
		}
		m_lsSIMData.clear();

		for(iter = m_lsMEData.begin() ; iter != m_lsMEData.end() ; iter ++){
			(*iter).Release();
		}
		m_lsMEData.clear();

		m_deqGroup.clear();	

		xSetLeftMobileItemCount();
		//post message to main frame and left panel
		::PostMessage(afxGetMainWnd()->GetSafeHwnd(),WM_DOWNLOAD_FAIL,1,0);
	}
	else
		::PostMessage(afxGetMainWnd()->GetSafeHwnd(),WM_DOWNLOAD_FAIL,0,0);
}

HRESULT CPrevView::OnSearchDlgGetStart(WPARAM wParam,LPARAM lParam)
{
	int *iStart = reinterpret_cast<int*>(wParam);
	*iStart = m_List.GetNextItem(-1,LVNI_SELECTED) + 1;
	return 1L;
}

void CPrevView::PlayAvi()
{

}
/*
void CPrevView::SyncData()
{
	int iMEUsed,iMEFree;
	m_Driver.InitialMemory(MEMORY_NAME,iMEUsed,iMEFree);
}
*/