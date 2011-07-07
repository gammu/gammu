// ContactsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SMSUtility.h"
#include "ContactsDlg.h"
#include "ProcessDlg.h"
#include "AviDlg.h"
#include "ProgressDlg.h"

//#include "AnwMobile\phonebook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern OpenPhoneBookProc			OpenPhoneBookfn;
extern ClosePhoneBookProc			ClosePhoneBookfn;
extern GetPhoneBookInfoProc		GetPhoneBookInfofn;
extern GetMobileInfoProc			GetMobileInfofn;
extern GetPhoneBookStartDataProc	GetPhoneBookStartDatafn;
extern GetPhoneBookNextDataProc	GetPhoneBookNextDatafn;
extern InitPhoneBookProc			InitPhoneBookfn;
extern InitialMemoryStatusProc		InitialMemoryStatusfn;


CContactsDlg*	CContactsDlg::s_ActiveDlg = NULL;
/////////////////////////////////////////////////////////////////////////////
// CContactsDlg dialog


CContactsDlg::CContactsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CContactsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CContactsDlg)
	//}}AFX_DATA_INIT
	m_hBrush		= NULL;
//	m_pContactInfo	= NULL;
	m_nSelectPhoneCount = 0;

	memset(m_strPort, 0, sizeof(TCHAR)*MAX_PATH);
	memset(m_strPhone, 0, sizeof(TCHAR)*MAX_PATH);
	memset(m_strConnectMode, 0, sizeof(TCHAR)*MAX_PATH);
	memset(m_strMobileName, 0, sizeof(TCHAR)*MAX_PATH);
	memset(m_strCompany, 0, sizeof(TCHAR)*MAX_PATH);
	memset(m_strIMEI, 0, sizeof(TCHAR)*MAX_PATH);
	memset(m_strMobileDLL, 0, sizeof(TCHAR)*MAX_PATH);

	m_TotalItems = 0;
	m_nPos = 0;
	m_nNextPost = 0;
	m_bStart = true;
	m_nMemFlag = MEM_ME;

	s_ActiveDlg = this;
	m_ContactInfoList.RemoveAll();
	m_hFont = NULL;
}

CContactsDlg::~CContactsDlg()
{
	//if(m_hMobileDll)
	//	FreeLibrary(m_hMobileDll);
	POSITION pos = m_ContactInfoList.GetHeadPosition();
	while(pos)
	{
		ContactList* pContactInfo = (ContactList*) m_ContactInfoList.GetNext(pos);
		if(pContactInfo)
			delete pContactInfo;
	}
	m_ContactInfoList.RemoveAll();
	if(m_hFont)
		::DeleteObject(m_hFont);
}


void CContactsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CContactsDlg)
	DDX_Control(pDX, IDOK, m_BTN_OK);
	DDX_Control(pDX, IDCANCEL, m_BTN_Cancel);
	DDX_Control(pDX, IDC_LIST_SELPHONE, m_LIST_SeletePhone);
	DDX_Control(pDX, IDC_LIST_CONTACTS, m_LIST_Contacts);
	DDX_Control(pDX, IDC_BTN_DELETE, m_BTN_Delete);
	DDX_Control(pDX, IDC_BTN_ADD, m_BTN_Add);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CContactsDlg, CDialog)
	//{{AFX_MSG_MAP(CContactsDlg)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_ADD, OnBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DELETE, OnBtnDelete)
	ON_NOTIFY(NM_CLICK, IDC_LIST_CONTACTS, OnClickListContacts)
	ON_NOTIFY(NM_CLICK, IDC_LIST_SELPHONE, OnClickListSelphone)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CContactsDlg message handlers

BOOL CContactsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	
	al_GetModulePath(NULL, m_szAppPath);
    LoadControl(m_szAppPath);
	InitValue();
	InitString();

	// Download
	if(theApp.nIsConnected != true)
		return false;

	CWnd *myParent = GetParent();

	xDownloadPB();

	myParent->EnableWindow(false);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CContactsDlg::LoadControl(LPCTSTR pSkinPath)
{
	TCHAR  szProfile[_MAX_PATH];// buf[_MAX_PATH], szFileName[_MAX_PATH]
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
	wsprintf(szProfile, _T("%s%s"), pSkinPath, _T("skin\\default\\SMSUtility\\PhoneNumber.ini"));

	// Button(Contacts, Send, Caancel)
	m_pButton[0] = &m_BTN_Add;
	m_pButton[1] = &m_BTN_Delete;
	m_pButton[2] = &m_BTN_OK;
	m_pButton[3] = &m_BTN_Cancel;

	TCHAR szSkin[PhoneBook_BTN_NUM][32] = {_T("add"), _T("delete"), _T("OK"), _T("Cancel")};
	UINT nToolTip[PhoneBook_BTN_NUM] = {IDS_112, IDS_112, IDS_112, IDS_112};

	int nWidth = 0;	//store the maxium button width
	for(int i = 0 ; i < PhoneBook_BTN_NUM ; i++)
	{
		BOOL bRet = FALSE;
		GetButtonFromSetting(m_pButton[i], szSkin[i], nToolTip[i], 0, (LPTSTR)(LPCTSTR)szProfile);
		CRect rc;
		m_pButton[i]->GetWindowRect(&rc);
		nWidth = rc.Width() > nWidth ? rc.Width() : nWidth;
	}

	for(i = 0 ; i < PhoneBook_BTN_NUM ; i ++)
	{
		CRect rc,rcPrev;
		m_pButton[i]->GetWindowRect(&rc);
		ScreenToClient(&rc);
		rc.right = rc.left + nWidth + 6;
		/*if(i > 0){
			m_pButton[i-1]->GetWindowRect(&rcPrev);
			ScreenToClient(&rcPrev);
			rc.OffsetRect(rcPrev.right + SSD_BN_OFFSET - rc.left,0);
		}
		else{
			rc.OffsetRect(SSD_BN_OFFSET - rc.left,0);
		}*/
		m_pButton[i]->MoveWindow(&rc);	
	}


	CRect rc,rect;
	// ListBox(Left, Right)
	al_GetSettingRect(_T("Left PhoneNumber"),_T("rect"),(LPTSTR)(LPCTSTR)szProfile,rect);
	m_LIST_Contacts.MoveWindow(&rect);

	al_GetSettingRect(_T("Right PhoneNumber"),_T("rect"),(LPTSTR)(LPCTSTR)szProfile,rect);
	m_LIST_SeletePhone.MoveWindow(&rect);

	if(pFont)
	{
		m_LIST_Contacts.SetFont(pFont);
		m_LIST_SeletePhone.SetFont(pFont);
	}
	// Panel
	al_GetSettingRect(_T("panel"),_T("rect"),(LPTSTR)(LPCTSTR)szProfile,rect);

	GetWindowRect(&rc);
	int nCapHeight = ::GetSystemMetrics(SM_CYCAPTION);
	int nDlgFrmX = ::GetSystemMetrics(SM_CXDLGFRAME);
	int nDlgFrmY = ::GetSystemMetrics(SM_CYDLGFRAME);
	::MoveWindow( GetSafeHwnd(), rc.left, rc.top,  rect.Width() + nDlgFrmX*2, 
				nCapHeight + nDlgFrmY*2 + rect.Height(), TRUE );
	//SetWindowTitle();

	//load panel brush
	COLORREF bksColor;
	if( !al_GetSettingColor(_T("panel"), _T("color"), szProfile, bksColor) )
		bksColor = 11012976;
		
	if(m_hBrush==NULL)	
		m_hBrush = ::CreateSolidBrush(bksColor);

}

void CContactsDlg::InitString(void)
{
	CString str;
	TCHAR szTmp[MAX_PATH];

	al_GetSettingString(_T("public"), _T("IDS_SELECT_CONTACTS"), theApp.m_szRes, szTmp);			// Title
	SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_ADD"), theApp.m_szRes, szTmp);
	m_BTN_Add.SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_DELETE"), theApp.m_szRes, szTmp);
	m_BTN_Delete.SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_OK"), theApp.m_szRes, szTmp);
	m_BTN_OK.SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_CANCEL"), theApp.m_szRes, szTmp);
	m_BTN_Cancel.SetWindowText(szTmp);
}

void CContactsDlg::InitValue()
{
	//ShowPhoneData();
	InitSelectPhoneList();

	m_BTN_Add.EnableWindow(FALSE);
	m_BTN_Delete.EnableWindow(FALSE);
}

void CContactsDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	CRect rect,rc;
	GetClientRect(&rect);
	int bBkMode = ::SetBkMode(dc, TRANSPARENT);

	if(m_hBrush)
		::FillRect(dc, &rect, m_hBrush);

	::SetBkMode(dc, bBkMode);
	
	// Do not call CDialog::OnPaint() for painting messages
}

BOOL CContactsDlg::InitImageList()
{
	// Create 256 color image lists
	HIMAGELIST hList = ImageList_Create(16, 16, ILC_COLOR24 | ILC_MASK , 2, 1);
	m_cImageListSmall.Attach(hList);

	m_cImageListSmall.Add(afxGetApp()->LoadIcon(IDI_PHONE));
	m_cImageListSmall.Add(afxGetApp()->LoadIcon(IDI_SIMCARD));
	


	// Attach them
	m_LIST_Contacts.SetImageList(&m_cImageListSmall);
	m_LIST_SeletePhone.SetImageList(&m_cImageListSmall);

	return TRUE;

}


void CContactsDlg::ShowPhoneData()
{
	TCHAR strName[MAX_PATH], strTel[MAX_PATH];
	al_GetSettingString(_T("public"), _T("IDS_NAME"), theApp.m_szRes, strName);
	al_GetSettingString(_T("public"), _T("IDS_PHONENUMBER"), theApp.m_szRes, strTel);

	CRect	rc;
	m_LIST_Contacts.GetClientRect(rc);

	m_LIST_Contacts.DeleteAllItems();
	m_LIST_Contacts.DeleteAllColumns();
	m_LIST_Contacts.InsertColumn(0, strName, LVCFMT_LEFT, 0, -1);
	m_LIST_Contacts.InsertColumn(1, strTel, LVCFMT_LEFT, 0, -1);

	m_LIST_Contacts.SetColumnWidth(0, rc.right / 2);
	m_LIST_Contacts.SetColumnWidth(1, rc.right / 2);
	

	InitImageList();

	int nNext = 0;

	POSITION pos = m_ContactInfoList.GetHeadPosition();
	while(pos)
	{
		ContactList* pContactInfo = (ContactList*) m_ContactInfoList.GetNext(pos);
		if(_tcslen(pContactInfo->strName) && _tcslen(pContactInfo->strTel))
		{
			const int IDX = m_LIST_Contacts.InsertItem(nNext, _T(""));
			m_LIST_Contacts.SetItemText(IDX, 0, pContactInfo->strName);
			m_LIST_Contacts.SetItemText(IDX, 1, pContactInfo->strTel);

			m_LIST_Contacts.SetItemImage(IDX, 0, pContactInfo->nStoreDevice-1); // subitem images
			m_LIST_Contacts.SetItemData(IDX,(unsigned long)pContactInfo);
			
			nNext++;
		}
	}

/*	for(int i = 0; i < m_nSIMUsedNum + m_nMEUsedNum; i++)
	{
		if(strlen(m_pContactInfo[i].strName) && strlen(m_pContactInfo[i].strTel))
		{
			const int IDX = m_LIST_Contacts.InsertItem(nNext, _T(""));
			m_LIST_Contacts.SetItemText(IDX, 0, m_pContactInfo[i].strName);
			m_LIST_Contacts.SetItemText(IDX, 1, m_pContactInfo[i].strTel);

			m_LIST_Contacts.SetItemImage(IDX, 0, m_pContactInfo[i].nStoreDevice); // subitem images
			
			nNext++;
		}
	}*/
}

void CContactsDlg::InitSelectPhoneList(void)
{
//	TCHAR szTmp[MAX_PATH];
//	al_GetSettingString(_T("public"), "IDS_PHONENUMBER ", theApp.m_szRes, szTmp);
	TCHAR strName[MAX_PATH], strTel[MAX_PATH];
	al_GetSettingString(_T("public"), _T("IDS_NAME"), theApp.m_szRes, strName);
	al_GetSettingString(_T("public"), _T("IDS_PHONENUMBER"), theApp.m_szRes, strTel);

	m_LIST_SeletePhone.DeleteAllItems();
	m_LIST_SeletePhone.DeleteAllColumns();
//	m_LIST_SeletePhone.InsertColumn(0, szTmp, LVCFMT_LEFT, 100);
//	m_LIST_SeletePhone.DeleteColumn(1);
	CRect rc;
	m_LIST_SeletePhone.GetClientRect(rc);
	m_LIST_SeletePhone.InsertColumn(0, strName, LVCFMT_LEFT, 0, -1);
	m_LIST_SeletePhone.InsertColumn(1, strTel, LVCFMT_LEFT, 0, -1);
	m_LIST_SeletePhone.SetColumnWidth(0, rc.right / 2);
	m_LIST_SeletePhone.SetColumnWidth(1, rc.right / 2);

/*	
	while(m_PhoneList[m_nSelectPhoneCount].GetLength() != 0)
	{
		const int IDX = m_LIST_SeletePhone.InsertItem(m_nSelectPhoneCount, _T(""));
		m_LIST_SeletePhone.SetItemText(IDX, 0, m_PhoneList[m_nSelectPhoneCount]);
		m_nSelectPhoneCount++;
	}*/
	POSITION pos = m_PhoneList->GetHeadPosition();
	while(pos)
	{
		CString strNo = m_PhoneList->GetNext(pos);
		const int IDX = m_LIST_SeletePhone.InsertItem(m_nSelectPhoneCount, _T(""));
		m_LIST_SeletePhone.SetItemText(IDX, 0, _T(""));
		m_LIST_SeletePhone.SetItemText(IDX, 1, strNo);
		m_LIST_SeletePhone.SetItemImage(IDX, 0, -1); // subitem images
		m_nSelectPhoneCount++;
	}

}

void CContactsDlg::InitAllListStates(void)
{
	m_LIST_Contacts.SetAllItemStates(RC_ITEM_SELECTED, RC_ITEM_UNSELECTED);
	m_LIST_Contacts.SetAllItemStates(RC_ITEM_FOCUSED, RC_ITEM_UNFOCUSED);
	m_BTN_Add.EnableWindow(FALSE);

	m_LIST_SeletePhone.SetAllItemStates(RC_ITEM_SELECTED, RC_ITEM_UNSELECTED);
	m_LIST_SeletePhone.SetAllItemStates(RC_ITEM_FOCUSED, RC_ITEM_UNFOCUSED);
	m_BTN_Delete.EnableWindow(FALSE);
}

/*
void CContactsDlg::SetData(ContactList *pContactInfo)
{
	// Put in Dialog
	m_pContactInfo = pContactInfo;
}
*/
void CContactsDlg::GetPhoneNumer(void)
{
	// Get From Dlg
	int nFristIndex = m_LIST_SeletePhone.GetTopIndex();
	int nTotalIndex = nFristIndex + m_LIST_SeletePhone.GetItemCount();

	m_PhoneList->RemoveAll();
	for(int i = 0; i < m_nSelectPhoneCount; i++)
	{
	//	m_PhoneList = m_LIST_SeletePhone.GetItemText(i, 0);
		CString strNo  = m_LIST_SeletePhone.GetItemText(i, 1);
		m_PhoneList->AddTail(strNo);
	}

}

void CContactsDlg::SetPhoneNumer(CStringList *PhoneList)
{
	// Get From Dlg
	m_PhoneList = PhoneList;
}

void CContactsDlg::OnBtnAdd() 
{
	// TODO: Add your control notification handler code here
	//int nSelectItemCount = m_LIST_Contacts.GetSelectedCount();
	//int nFristIndex = m_LIST_Contacts.GetTopIndex();
	//int nTotalIndex = nFristIndex + m_LIST_Contacts.GetItemCount();
	int nSelectItemCount = m_LIST_Contacts.GetItemCount();

	int nMask = 0;
	for(int i = 0; i < nSelectItemCount; i++)
	{
		nMask = m_LIST_Contacts.GetItemState(i, LVIS_SELECTED);
		//nMask = m_LIST_Contacts.GetItemStates(i);
		if(nMask == LVIS_SELECTED)
		{
			const int IDX = m_LIST_SeletePhone.InsertItem(m_nSelectPhoneCount, _T(""));

			ContactList* pContactInfo = (ContactList*)m_LIST_Contacts.GetItemData(i);

			m_LIST_SeletePhone.SetItemText(IDX, 0, pContactInfo->strName);
			m_LIST_SeletePhone.SetItemText(IDX, 1, pContactInfo->strTel);
			m_LIST_SeletePhone.SetItemImage(IDX, 0, pContactInfo->nStoreDevice-1); // subitem images
			m_nSelectPhoneCount++;
		}
	}

	InitAllListStates();
}

void CContactsDlg::OnBtnDelete() 
{
	// TODO: Add your control notification handler code here
	//int nFristIndex = m_LIST_SeletePhone.GetTopIndex();
	//int nTotalIndex = nFristIndex + m_LIST_SeletePhone.GetItemCount();
	int nSelectItemCount = m_LIST_SeletePhone.GetItemCount();

	int nMask = 0;
	//for(int i = 0; i < nSelectItemCount; i++)
	for(int i = nSelectItemCount-1; i >=0 ; i--)
	{
		nMask = m_LIST_SeletePhone.GetItemState(i, LVIS_SELECTED);
		if(nMask == LVIS_SELECTED)
		{
			m_LIST_SeletePhone.DeleteItem(i);
			m_nSelectPhoneCount--;
		}
	}

	InitAllListStates();
}

void CContactsDlg::OnClickListContacts(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	int nSelectItems = m_LIST_Contacts.GetSelectedCount();
	if(nSelectItems > 0)
	{
		m_BTN_Add.EnableWindow(TRUE);
		m_BTN_Delete.EnableWindow(FALSE);
	}
	else
	{
		InitAllListStates();
	}
	
	*pResult = 0;
}

void CContactsDlg::OnClickListSelphone(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	int nSelectItems = m_LIST_SeletePhone.GetSelectedCount();
	if(nSelectItems > 0)
	{
		m_BTN_Add.EnableWindow(FALSE);
		m_BTN_Delete.EnableWindow(TRUE);
	}
	else
	{
		InitAllListStates();
	}
	
	*pResult = 0;
}

void CContactsDlg::OnOK() 
{
	// TODO: Add extra validation here
	GetPhoneNumer();
	
	CDialog::OnOK();
}


#define SIM_NAME		"SM"
#define MEMORY_NAME		"ME"

bool CContactsDlg::InitialLibrary(void)
{
	//m_hMobileDll = LoadLibrary("AnwMobile.dll");
	// DLL CONTRUCT
	if(afxContextIsDLL)
	{
	  m_hMobileDll = ((CSMSUtility*)afxGetApp())->m_hInstAnwMobile;
	}
    else
	{
	  if(_tcslen(m_strMobileDLL) > 0)
	  	m_hMobileDll = LoadLibrary(m_strMobileDLL);
	  else
#ifdef _UNICODE
		m_hMobileDll = LoadLibrary(_T("AnwMobileU.dll"));
#else
		m_hMobileDll = LoadLibrary(_T("AnwMobile.dll"));
#endif
	}
	

	if (!m_hMobileDll) 
	{
		MessageBox(_T("Load AnwMobile.dll error"),_T("Error"),MB_ICONERROR|MB_OK);
		return false;
	}

	OpenPhoneBookfn = (OpenPhoneBookProc)::GetProcAddress(m_hMobileDll,"OpenPhoneBook");
	ClosePhoneBookfn = (ClosePhoneBookProc)::GetProcAddress(m_hMobileDll,"ClosePhoneBook");
	GetPhoneBookInfofn = (GetPhoneBookInfoProc)::GetProcAddress(m_hMobileDll,"GetPhoneBookInfo");
	GetPhoneBookStartDatafn = (GetPhoneBookStartDataProc)::GetProcAddress(m_hMobileDll,"GetPhoneBookStartData");
	GetPhoneBookNextDatafn = (GetPhoneBookNextDataProc)::GetProcAddress(m_hMobileDll,"GetPhoneBookNextData");
	InitPhoneBookfn = (InitPhoneBookProc)::GetProcAddress(m_hMobileDll,"InitPhoneBook");

	InitialMemoryStatusfn = (InitialMemoryStatusProc)GetProcAddress(m_hMobileDll,"InitialMemoryStatus");
	if(InitialMemoryStatusfn == NULL)
	{
		MessageBox(_T("Can't Get InitialMemoryStatusfn()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}

	if(InitPhoneBookfn == NULL)
	{
		MessageBox(_T("Can't Get InitPhoneBook()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}

	if(OpenPhoneBookfn == NULL)
	{
		MessageBox(_T("Can't Get OpenPhoneBook()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}

	if(ClosePhoneBookfn == NULL)
	{
		MessageBox(_T("Can't Get ClosePhoneBook()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}

	if (GetPhoneBookInfofn == NULL)
	{
		MessageBox(_T("Can't Get GetPhoneBookInfo()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}


	if (GetPhoneBookStartDatafn == NULL)
	{
		MessageBox(_T("Can't Get GetPhoneBookStartData()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}

	if (GetPhoneBookNextDatafn == NULL)
	{
		MessageBox(_T("Can't Get GetPhoneBookNextData()"),_T("Error"),MB_ICONERROR|MB_OK);
		FreeLibrary(m_hMobileDll);
		return false; 
	}




   return true;
}


// DLL CONTRUCT
void CContactsDlg::FreeLibrary(HMODULE m_hDriver)
{
   if(!afxContextIsDLL)
	   ::FreeLibrary(m_hDriver);
}


int CContactsDlg::OpenMobileConnect(void)
{
	//char Buffer[100];
	int ret = 0;

	//GetMobileSetting();

	int nCompany	= _ttoi(m_strCompany);


	/*if(strlen(m_strIMEI) == 0)
		ret = OpenPhoneBookfn(nCompany, LPTSTR(m_strPhone), LPTSTR(m_strConnectMode), LPTSTR(m_strPort), NULL, NULL);
	else		ret = OpenPhoneBookfn(nCompany, LPTSTR(m_strPhone), LPTSTR(m_strConnectMode), LPTSTR(m_strPort), LPTSTR(m_strIMEI), NULL);

	if (ret == ERR_NONE)
	{
		InitPhoneBookfn();
		int nGetRet = GetPBInfo();
		
	}*/
	if(theApp.nIsConnected != true)
		return false;
	
	InitPhoneBookfn();
	int nGetRet = GetPBInfo();


	return nGetRet;
}

int CContactsDlg::ConnectStatusCallBack(int State)
{
   return 0;
}

int CContactsDlg::GetMobileSetting(void)
{
	TCHAR FileName[MAX_PATH];
	CString strFileName;

	GetModuleFileName(NULL, FileName, MAX_PATH);
	strFileName = FileName;
	strFileName = strFileName.Left(strFileName.ReverseFind('\\'));
	strFileName +=_T("\\mobile_setting.ini");

	_tcscpy(FileName, strFileName.GetBuffer(strFileName.GetLength()));

	int ret = al_GetSettingString(_T("mobile"), _T("name"), FileName, m_strMobileName);
	ret = al_GetSettingString(_T("mobile"), _T("phone"), FileName, m_strPhone);
	ret = al_GetSettingString(_T("mobile"), _T("port"), FileName, m_strPort);
	ret = al_GetSettingString(_T("mobile"), _T("connect_mode"), FileName, m_strConnectMode);
	ret = al_GetSettingString(_T("mobile"), _T("company"), FileName, m_strCompany);
	ret = al_GetSettingString(_T("mobile"), _T("IMEI"), FileName, m_strIMEI);
	
	ret = al_GetSettingString(_T("lib"), _T("control_dll"), FileName, m_strMobileDLL);

	return true;
}

int CContactsDlg::GetPBInfo(void)
{
	int ret = 0;
	Contact_Tal_Num contact_Tal_Num;
	ret = GetPhoneBookInfofn(&contact_Tal_Num);
			
	if(ret != ERR_NONE)
	{
		TCHAR str1[MAX_PATH];
		al_GetSettingString(_T("public"), _T("IDS_ERR_READFAILED"), theApp.m_szRes, str1);
		AfxMessageBox(str1);
		return false;
	}
	else
	{
		//m_nSIMUsedNum	= contact_Tal_Num.SIMContactUsedNum;
		//m_nMEUsedNum	= contact_Tal_Num.MEContactUsedNum;
		m_nSIMUsedNum	= contact_Tal_Num.SIMContactTotalNum;
		m_nMEUsedNum	= contact_Tal_Num.MEContactTatalNum;
		//m_pContactInfo = new contact_Tal_Num
	}
	

	return true;
}


int CContactsDlg::GetPBData(char *strMemType, int nMemtype, int nPos, bool bStart)
{
	int ret = 0;

	
	return true;
}

int CContactsDlg::GetPBOneData(char *strMemType, int nMemtype, int nPos, bool bStart)
{
	int ret = 0;
	// Get ALl Phone Number
	GSM_MemoryEntry entryList;
	memset(&entryList, 0, sizeof(GSM_MemoryEntry));
	
	//get the one data
	if(bStart == true)
	{
		ret = GetPhoneBookStartDatafn(strMemType, &entryList); 
	}
	else
		ret = GetPhoneBookNextDatafn(strMemType, &entryList, bStart); 

	if(ret == ERR_NONE)
	{
		ContactList *pcontactList = new ContactList;
		memset(pcontactList, 0, sizeof(ContactList));
		ParserPBData(entryList, pcontactList);

		if(_tcslen(pcontactList->strName) != 0 && _tcslen(pcontactList->strTel) != 0)
		{
			pcontactList->nStoreDevice = nMemtype;
			m_ContactInfoList.AddTail(pcontactList);
		//	_tcscpy(m_pContactInfo[nPos].strName, contactList.strName);
		//	_tcscpy(m_pContactInfo[nPos].strTel, contactList.strTel);
		//	m_pContactInfo[nPos].nStoreDevice= nMemtype;
		}
		else
			delete pcontactList;
		//else
		//	return false;
	}

	return ret;
}

int CContactsDlg::ParserPBData(GSM_MemoryEntry entryList, ContactList *pcontactInfo)
{
	int EntriesNum = entryList.EntriesNum;


	bool nMobilePhone = false;	
	bool bHasName = false;
	int nMobliePhone ,nGeneral , nHome,nWork,nOther,nPager,nFax;
	nMobliePhone  = nGeneral = nHome = nWork = nOther = nPager = nFax =-1;
	int nFirstName ,nLastName;
	nFirstName = nLastName = -1;

	for(int Col_Num = 0; Col_Num < EntriesNum; Col_Num++)
	{
		switch(entryList.Entries[Col_Num].EntryType)
		{
			case PBK_Text_Name : 
				memcpy(pcontactInfo->strName, entryList.Entries[Col_Num].Text,( _tcslen((const TCHAR*)entryList.Entries[Col_Num].Text))*sizeof(TCHAR));
				bHasName = true;
				break;
			case PBK_Text_FirstName : 
				nFirstName = Col_Num;
				break;
			case PBK_Text_LastName : 
				nLastName = Col_Num;
				break;
			case PBK_Number_Mobile : 
				nMobliePhone = Col_Num;
				break;
			case PBK_Number_General : 
				nGeneral = Col_Num;
				break;
			case PBK_Number_Home : 
				nHome = Col_Num;
				break;
			case PBK_Number_Work : 
				nWork = Col_Num;
				break;
			case PBK_Number_Other : 
				nOther = Col_Num;
				break;
			case PBK_Number_Pager : 
				nPager = Col_Num;
				break;
			case PBK_Number_Fax : 
				nFax = Col_Num;
				break;
		}
	}
	if(bHasName == false && (nFirstName !=-1 || nLastName !=-1))
	{
		CString strName;
		strName.Empty();
		if(nFirstName !=-1 )
			strName.Format(_T("%s"),entryList.Entries[nFirstName].Text);
		if(nLastName !=-1 )
		{
			CString strLastName;
			strLastName.Format(_T("%s"),entryList.Entries[nLastName].Text);

			if(!strName.IsEmpty())
			{
				strName+=" ";
			}
			strName+=strLastName;
		}
		if(strName.GetLength()>254) strName.SetAt(254,'\0');
		wsprintf(pcontactInfo->strName,_T("%s"),strName);
		bHasName = true;
	}
	int nPhoneNumberIndex = -1;
	if(nMobliePhone != -1)
		nPhoneNumberIndex = nMobliePhone;
	else if(nGeneral != -1)
		nPhoneNumberIndex = nGeneral;
	else if(nWork != -1)
		nPhoneNumberIndex = nWork;
	else if(nHome != -1)
		nPhoneNumberIndex = nHome;
	else if(nOther != -1)
		nPhoneNumberIndex = nOther;
	else if(nPager != -1)
		nPhoneNumberIndex = nPager;
	else if(nFax != -1)
		nPhoneNumberIndex = nFax;
		
	if(nPhoneNumberIndex != -1)
		memcpy(pcontactInfo->strTel, entryList.Entries[nPhoneNumberIndex].Text, (_tcslen((const TCHAR*)entryList.Entries[nPhoneNumberIndex].Text))*sizeof(TCHAR));

  //	int len= sizeof(entryList.Entries[0].Text);
/*	for(int Col_Num = 0; Col_Num < EntriesNum; Col_Num++)
	{
		switch(entryList.Entries[Col_Num].EntryType)
		{
			case PBK_Text_Name : 
				memcpy(pcontactInfo->strName, entryList.Entries[Col_Num].Text, strlen((const char*)entryList.Entries[Col_Num].Text));
				bHasName = true;
				break;

			case PBK_Number_Mobile : 
				memcpy(pcontactInfo->strTel, entryList.Entries[Col_Num].Text, strlen((const char*)entryList.Entries[Col_Num].Text));
				nMobilePhone = true;
				break;

			case PBK_Number_General : 
				memcpy(pcontactInfo->strTel, entryList.Entries[Col_Num].Text, strlen((const char*)entryList.Entries[Col_Num].Text));
				nMobilePhone = true;
				break;

			case PBK_Number_Home : 
				memcpy(pcontactInfo->strTel, entryList.Entries[Col_Num].Text, strlen((const char*)entryList.Entries[Col_Num].Text));
				nMobilePhone = true;
				break;

			case PBK_Number_Work : 
				memcpy(pcontactInfo->strTel, entryList.Entries[Col_Num].Text, strlen((const char*)entryList.Entries[Col_Num].Text));
				nMobilePhone = true;
				break;

			case PBK_Number_Other : 
				memcpy(pcontactInfo->strTel, entryList.Entries[Col_Num].Text, strlen((const char*)entryList.Entries[Col_Num].Text));
				nMobilePhone = true;
				break;
			case PBK_Number_Pager : 
				memcpy(pcontactInfo->strTel, entryList.Entries[Col_Num].Text, strlen((const char*)entryList.Entries[Col_Num].Text));
				nMobilePhone = true;
				break;
		}

		if(nMobilePhone == true && bHasName)
			break;

	}
*/
	return true;
}

int CContactsDlg::GetAllPBData(int nNeedCount)
{
	int ret = 0;

	if(s_ActiveDlg->m_nMEUsedNum == 0  && s_ActiveDlg->m_nMemFlag == MEM_ME)
		s_ActiveDlg->m_nMemFlag++;

	if(s_ActiveDlg->m_nMemFlag == MEM_ME)
	{
		ret = s_ActiveDlg->GetPBOneData("ME\0", MEM_ME, s_ActiveDlg->m_nNextPost, s_ActiveDlg->m_bStart);
		if(ret != 1)
		{
			TCHAR str1[MAX_PATH];
			al_GetSettingString(_T("public"), _T("IDS_ERR_READFAILED"), theApp.m_szRes, str1);
			AfxMessageBox(str1);
			s_ActiveDlg->m_nPos = s_ActiveDlg->m_nMEUsedNum-1;
			return false;
		}


		s_ActiveDlg->m_bStart = false;

		if(s_ActiveDlg->m_nPos == s_ActiveDlg->m_nMEUsedNum - 1)
		{
			s_ActiveDlg->m_nMemFlag++;
			s_ActiveDlg->m_bStart = true;
		}
	}
	else if(s_ActiveDlg->m_nMemFlag == MEM_SM)
	{
		ret = s_ActiveDlg->GetPBOneData("SM\0", MEM_SM, s_ActiveDlg->m_nNextPost, s_ActiveDlg->m_bStart);
		if(ret != 1)
		{
			TCHAR str1[MAX_PATH];
			al_GetSettingString(_T("public"), _T("IDS_ERR_READFAILED"), theApp.m_szRes, str1);
			AfxMessageBox(str1);
			s_ActiveDlg->m_nPos = s_ActiveDlg->m_nMEUsedNum-1;
			return false;
		}

		s_ActiveDlg->m_bStart = false;

		if(s_ActiveDlg->m_nPos == s_ActiveDlg->m_nSIMUsedNum + s_ActiveDlg->m_nMEUsedNum - 1)
		{
			s_ActiveDlg->m_nMemFlag++;
			s_ActiveDlg->m_bStart = true;
		}
	}

	s_ActiveDlg->m_nPos++;
	
	if(ret == ERR_NONE)
		s_ActiveDlg->m_nNextPost++;
	else
		return false;

	
	return s_ActiveDlg->m_TotalItems;
}


int CContactsDlg::xDownloadPB(void)
{
	CProgressDlg dlg;
	dlg.SetDataToDlg(s_ActiveDlg);
	dlg.DoModal();
	s_ActiveDlg->ShowPhoneData();
	return true;
/*	CProcessDlg dlg;
	CProcessDlg dlg2;

	s_ActiveDlg->GetMobileSetting();

	// TODO: Add extra initialization here
	int ret = 1;//s_ActiveDlg->InitialLibrary();
	if(ret == false)
	{ 
		return false;	
	}
	else
	{
		ret = s_ActiveDlg->OpenMobileConnect();
	}

	//StartDownloadPB(NeedCount);
	int free;
	ret = InitialMemoryStatusfn("ME",&s_ActiveDlg->m_nMEUsedNum, &free);
	int nNeedCount = s_ActiveDlg->m_nMEUsedNum;// + s_ActiveDlg->m_nSIMUsedNum;
	if(nNeedCount >= 0)
	{
		dlg.SetProgressInfo( CContactsDlg::GetAllPBData, nNeedCount);
		dlg.SetLoopCount(nNeedCount);
		dlg.DoModal();
	}
	ret = InitialMemoryStatusfn("SM",&s_ActiveDlg->m_nSIMUsedNum, &free);
	nNeedCount = s_ActiveDlg->m_nSIMUsedNum;// + s_ActiveDlg->m_nSIMUsedNum;
	if(nNeedCount >= 0)
	{
		dlg2.SetProgressInfo( CContactsDlg::GetAllPBData, nNeedCount);
		dlg2.SetLoopCount(nNeedCount);
		dlg2.DoModal();
	}


	s_ActiveDlg->ShowPhoneData();

	return true;*/
}


DWORD WINAPI CContactsDlg::TransferProc(LPVOID lpParam)
{
	//ASSERT(lpParam);
	//CAviDlg* pObj = (CAviDlg*) lpParam;
	//::PostMessage(pObj->GetSafeHwnd(), WM_CLOSE, 0, 0);
	//Sleep(100);
	//s_ActiveDlg->Avidlg->EndDialog(1);
	ExitThread(0);
}
