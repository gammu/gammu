// ProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\phonebook.h"
#include "progressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
BOOL g_bCancel = FALSE;
/////////////////////////////////////////////////////////////////////////////
// CProgressDlg dialog
static CProgressDlg *sActivateDlg;
int CProgressDlg::ProgressCallback( int nProgress )
{
	if(::IsWindow(sActivateDlg->m_hWnd) && ::IsWindow(sActivateDlg->m_Prog.m_hWnd))
		sActivateDlg->m_Prog.SetPos(nProgress);
	return 0;
}
UINT AddThreadProc(CProgressDlg*  pDlg)
{
	DATALIST lsData = *pDlg->m_pAddDataList;

	pDlg->m_Prog.SetRange(0,lsData.size());
	pDlg->m_Prog.SetPos(0);
	OleInitialize(NULL);
	theApp.m_MSOTDll.MSOT_InitDLL();
	LPDISPATCH lpFolder;
	lpFolder = theApp.m_MSOTDll.MSOT_GetFolderByName( (pDlg->m_pFolderInfoData)->sEntryID,  (pDlg->m_pFolderInfoData)->sStoreID,  ContactFolder);
	if(lpFolder== NULL)
	{
		theApp.m_MSOTDll.MSOT_TerminateDLL();
		::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);
		return false;
	}
	lpFolder->AddRef();

	LPDISPATCH dispItem = NULL;
	dispItem = (LPDISPATCH)lpFolder;
	dispItem->AddRef();	// AddRef to prevent crashes; COM is inscrutible

	int nAddCount = 0;
	for(DATALIST::iterator iter = lsData.begin() ; iter != lsData.end() ; iter ++)
	{
		CCardPanel data =*iter;
		GSM_MemoryEntry Entry;
		memset(&Entry,0,sizeof(GSM_MemoryEntry));
		data.GetGSMMemoryEntry(Entry);
		ContactData* pContact_Data = new ContactData;
		MemoryEntry2ContactDataStruct(&Entry,&pContact_Data->Data);
		int nRet = 0;
		if(strcmp(data.GetStorageType() , SIM_NAME) == 0)
		{
			nRet = theApp.m_MSOTDll.MSOT_AddSIMContact(dispItem,pContact_Data);
		}
		else
		{
			nRet = theApp.m_MSOTDll.MSOT_AddContact(dispItem,pContact_Data);
		}
		if(nRet == 1)
		{
			theApp.m_MSOTDll.MSOT_UpdateContactToMobileSupport(&pContact_Data->Data);
			(pDlg->m_pDataList)->AddTail(pContact_Data);
		}
		pDlg->m_Prog.SetPos(nAddCount);
		nAddCount++;
	}

	theApp.m_MSOTDll.MSOT_TerminateDLL();
	::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);
   return 0;
}

UINT GetThreadProc(CProgressDlg*  pDlg)
{
	OleInitialize(NULL);
	pDlg->m_Prog.SetRange(0,100);
	pDlg->m_Prog.SetPos(0);
	theApp.m_MSOTDll.MSOT_InitDLL();
	LPDISPATCH lpFolder;
	lpFolder = theApp.m_MSOTDll.MSOT_GetFolderByName( (pDlg->m_pFolderInfoData)->sEntryID,  (pDlg->m_pFolderInfoData)->sStoreID,  ContactFolder);
	if(lpFolder== NULL)
	{
		theApp.m_MSOTDll.MSOT_TerminateDLL();
		::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);
		return false;
	}
	lpFolder->AddRef();

	LPDISPATCH dispItem = NULL;
	dispItem = (LPDISPATCH)lpFolder;
	dispItem->AddRef();	// AddRef to prevent crashes; COM is inscrutible

	theApp.m_MSOTDll.MSOT_GetContact(dispItem,pDlg->m_pDataList,&g_bCancel,CProgressDlg::ProgressCallback,0,100);
	theApp.m_MSOTDll.MSOT_TerminateDLL();
	::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, 0);
   return 0;
}

CProgressDlg::CProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgressDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_crBg = 0;
	m_pThread=NULL;
}


void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressDlg)
	DDX_Control(pDX, IDC_SC_TEXT, m_scText);
	DDX_Control(pDX, IDC_PROG, m_Prog);
	DDX_Control(pDX, IDCANCEL, m_rbnCancel);
	DDX_Control(pDX, IDC_ANIMATE, m_AnCtrl);
//	DDX_Control(pDX, IDC_PROGRESS_BAR, m_ProgressCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	//{{AFX_MSG_MAP(CProgressDlg)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg message handlers

BOOL CProgressDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
    g_bCancel = FALSE; 
	
	m_sProfile = Tstring(theApp.m_szSkin) + Tstring(_T("Organize\\ProgDlg.ini"));
	//load cancel button
	GetButtonFromSetting(&m_rbnCancel,_T("cancel"),_T("IDS_CANCEL"),0,const_cast<TCHAR *>(m_sProfile.c_str()));
	GetStaticFromSetting(&m_scText,_T("text_sc"),NULL,const_cast<TCHAR *>(m_sProfile.c_str()));
	//get progress rect 
	CRect rect;
	if(al_GetSettingRect(_T("progress"),_T("rect"),const_cast<TCHAR *>(m_sProfile.c_str()),rect)){
		m_Prog.MoveWindow(rect);
	}
	//load text static
	
	//get dialog rect
	if(al_GetSettingRect(_T("panel") , _T("rect"),const_cast<TCHAR *>(m_sProfile.c_str()), rect)){
		LONG style = GetWindowLong(GetSafeHwnd(),GWL_STYLE);
		LONG dwstyle = GetWindowLong(GetSafeHwnd(),GWL_EXSTYLE);
		AdjustWindowRectEx(rect,style,FALSE,dwstyle);
		MoveWindow(rect);
		CenterWindow();
	}
	//get background color
	al_GetSettingColor(_T("panel") , _T("brush"),const_cast<TCHAR *>(m_sProfile.c_str()), m_crBg);

	//Mark by Asion for Dialog Layout
	if(al_GetSettingRect(_T("cancel"),_T("rect"),const_cast<TCHAR *>(m_sProfile.c_str()),rect)){
		m_rbnCancel.MoveWindow(rect);
	}
	TCHAR szTxt[MAX_PATH];
	al_GetSettingString(_T("public"),_T("IDS_PROG_TRANSFER_TEXT"),theApp.m_szRes,szTxt);
	m_scText.SetWindowText(szTxt);
	al_GetSettingString(_T("public"),_T("IDS_PROG_TRANSFER"),theApp.m_szRes,szTxt);
	SetWindowText(szTxt);

	//set progress bar setting
	COLORREF white, blue;
	white = RGB(255,255,255);
	blue =  RGB(0,0,255);
	m_brBar.CreateSolidBrush( blue );
	m_Prog.SetBarBrush(&m_brBar);
	m_Prog.SetShowPercent();
	m_Prog.SetBkColor( white );
	m_Prog.SetTextColor( white, blue );
	m_AnCtrl.ShowWindow(SW_HIDE);
	m_Prog.ShowWindow(SW_SHOW);
	m_scText.ShowWindow(SW_SHOW);
	m_rbnCancel.ShowWindow(SW_SHOW);
	sActivateDlg = this;
	              // EXCEPTION: OCX Property Pages should return FALSE
	if(m_nType == 1)
		m_pThread = AfxBeginThread((AFX_THREADPROC)AddThreadProc,  this, THREAD_PRIORITY_NORMAL);
	else
		m_pThread = AfxBeginThread((AFX_THREADPROC)GetThreadProc,  this, THREAD_PRIORITY_NORMAL);
	ShowWindow(SW_SHOW);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CProgressDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	if(m_pThread)
	{
		m_pThread->SuspendThread();
		g_bCancel = TRUE;
		m_pThread->ResumeThread();
		UpdateData(FALSE);
		m_rbnCancel.EnableWindow(FALSE);
	}
	else CDialog::OnCancel();
	
//	CDialog::OnCancel();
}

void CProgressDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rect;
	GetClientRect(rect);
	CBrush brush(m_crBg);
	dc.FillRect(rect,&brush);
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CProgressDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	if(m_pThread)
	{
	//	m_pThread->SuspendThread();
		g_bCancel = TRUE;
	//	m_pThread->ResumeThread();
		UpdateData(FALSE);
		m_rbnCancel.EnableWindow(FALSE);
	}
	else
		CDialog::OnClose();	
}

