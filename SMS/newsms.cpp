// NewSMS.cpp : implementation file
//

#include "stdafx.h"
#include "SMSUtility.h"
#include "NewSMS.h"

#include "ContactsDlg.h"

#include "PrevView.h"
#include "MainFrm.h"
#include "LeftView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CNewSMS dialog
//#define SINGLEBYTE		160
//#define MULTIBYTE		70

CNewSMS::CNewSMS(CWnd* pParent /*=NULL*/)
	: CDialog(CNewSMS::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewSMS)
	//}}AFX_DATA_INIT

	m_hBrush = NULL;
	m_bMultiByte = false;
	m_nPage = 0;
	m_bFirstCall = false;
	m_hFont = NULL;
	m_bIsSend=false;

	//m_strContent
	//m_strPhone
}
CNewSMS::~CNewSMS()
{
	if(m_hFont)
		::DeleteObject(m_hFont);
}

void CNewSMS::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewSMS)
	DDX_Control(pDX, IDC_BTN_DRAFT, m_BTN_Save2Draft);
	DDX_Control(pDX, IDC_BTN_SEND, m_BTN_Send);
	DDX_Control(pDX, IDCANCEL, m_BTN_Cancel);
	DDX_Control(pDX, IDC_BTN_CONTACTS, m_bt_Contacts);
	DDX_Control(pDX, IDC_ST_RECEIVER, m_ST_Receiver);
	DDX_Control(pDX, IDC_ST_MSGCONTENT, m_ST_MsgContent);
	DDX_Control(pDX, IDC_ST_COUNTS, m_ST_Counts);
	DDX_Control(pDX, IDC_EDIT_CONTENTS, m_EDIT_Contents);
	DDX_Control(pDX, IDC_EDIT_CONTACTSLIST, m_EDIT_ContactsList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewSMS, CDialog)
	//{{AFX_MSG_MAP(CNewSMS)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_CONTACTS, OnBtnContacts)
	ON_EN_CHANGE(IDC_EDIT_CONTENTS, OnChangeEditContents)
	ON_BN_CLICKED(IDC_BTN_SEND, OnBtnSend)
	ON_EN_CHANGE(IDC_EDIT_CONTACTSLIST, OnChangeEditContactslist)
	ON_BN_CLICKED(IDC_BTN_DRAFT, OnBtnDraft)
	ON_WM_CHAR()
	ON_EN_MAXTEXT(IDC_EDIT_CONTACTSLIST, OnMaxtextEditContactslist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewSMS message handlers

BOOL CNewSMS::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	al_GetModulePath(NULL, m_szAppPath);
    LoadControl(m_szAppPath);
// 	LONG style = GetWindowLong(m_EDIT_Contents.GetSafeHwnd(),GWL_STYLE);
// 	style |= ES_WANTRETURN|ES_MULTILINE ;
// 	SetWindowLong(m_EDIT_Contents.GetSafeHwnd(),GWL_STYLE,style);

	InitValue();
	InitString();

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewSMS::LoadControl(LPCTSTR pSkinPath)
{
	TCHAR  szFileName[_MAX_PATH], szProfile[_MAX_PATH], buf[_MAX_PATH];
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
	wsprintf(szProfile, _T("%s%s"), pSkinPath, _T("skin\\default\\SMSUtility\\NewSMS.ini"));

	//load panel brush
	COLORREF bksColor;
	if( !al_GetSettingColor(_T("panel"), _T("color"), szProfile, bksColor) )
		bksColor = 11012976;
		
	if(m_hBrush==NULL)	
		m_hBrush = ::CreateSolidBrush(bksColor);

	// Button(Contacts, Send, Caancel)
	m_pButton[0] = &m_bt_Contacts;
	m_pButton[1] = &m_BTN_Send;
	m_pButton[2] = &m_BTN_Save2Draft;
	m_pButton[3] = &m_BTN_Cancel;

	TCHAR szSkin[TOOLBAR_BTN_NUM][32] = {_T("Contact"), _T("Send"), _T("Save2Draft"), _T("Cancel")};
	UINT nToolTip[TOOLBAR_BTN_NUM] = {IDS_123, IDS_123, IDS_123, IDS_123};

	int nWidth = 0;	//store the maxium button width
	for(int i = 0 ; i < TOOLBAR_BTN_NUM ; i++)
	{
		BOOL bRet = FALSE;
		GetButtonFromSetting(m_pButton[i], szSkin[i], nToolTip[i], 0, (LPTSTR)(LPCTSTR)szProfile);
		CRect rc;
		m_pButton[i]->GetWindowRect(&rc);
		nWidth = rc.Width() > nWidth ? rc.Width() : nWidth;
	}

	for(i = 0 ; i < TOOLBAR_BTN_NUM ; i ++)
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


	// Static Text(Receiver, MsgContent, count[0/160])
	////int nSize;
	if(al_GetSettingString(_T("static"), _T("image"), szProfile, buf))
	{	
		wsprintf(szFileName, _T("%s\\%s"), pSkinPath, buf);
		////al_GetSettingString( _T("button_font"), "font_name", (LPTSTR)(LPCTSTR)szProfile, buf );
		///al_GetSettingInt( _T("button_font"), "font_size", (LPTSTR)(LPCTSTR)szProfile, nSize );

		CRect stRect;
		al_GetSettingRect(_T("receiver"), _T("rect"), (LPTSTR)(LPCTSTR)szProfile, stRect);
		m_ST_Receiver.LoadBitmap(szFileName);
		m_ST_Receiver.SetTextFont( CString(theApp.m_lf.lfFaceName), theApp.m_lf.lfHeight );
		m_ST_Receiver.SetBrush(m_hBrush);
		m_ST_Receiver.MoveWindow(&stRect);	

		al_GetSettingRect(_T("MsgContent"), _T("rect"), (LPTSTR)(LPCTSTR)szProfile, stRect);
		m_ST_MsgContent.LoadBitmap(szFileName);
		m_ST_MsgContent.SetTextFont( CString(theApp.m_lf.lfFaceName), theApp.m_lf.lfHeight );
		m_ST_MsgContent.SetBrush(m_hBrush);
		m_ST_MsgContent.MoveWindow(&stRect);

		al_GetSettingRect(_T("count"), _T("rect"), (LPTSTR)(LPCTSTR)szProfile, stRect);
		m_ST_Counts.LoadBitmap(szFileName);
		m_ST_Counts.SetTextFont( CString(theApp.m_lf.lfFaceName), theApp.m_lf.lfHeight );
		m_ST_Counts.SetBrush(m_hBrush);
		m_ST_Counts.MoveWindow(&stRect);
	}


	CRect rc,rect;
	// Combox(Phone Mumber List)
	al_GetSettingRect(_T("PBList"),_T("rect"),(LPTSTR)(LPCTSTR)szProfile,rect);
	m_EDIT_ContactsList.MoveWindow(&rect);

	// Edit(Msg Content)
	al_GetSettingRect(_T("MsgEdit"),_T("rect"),(LPTSTR)(LPCTSTR)szProfile,rect);
	m_EDIT_Contents.MoveWindow(&rect);

	if(pFont)
	{
		m_EDIT_ContactsList.SetFont(pFont);
		m_EDIT_Contents.SetFont(pFont);
	}
	// Panel
	al_GetSettingRect(_T("panel"),_T("rect"),(LPTSTR)(LPCTSTR)szProfile,rect);

	GetWindowRect(&rc);
	int nCapHeight = ::GetSystemMetrics(SM_CYCAPTION);
	int nDlgFrmX = ::GetSystemMetrics(SM_CXDLGFRAME);
	int nDlgFrmY = ::GetSystemMetrics(SM_CYDLGFRAME);
	if(m_bFirstCall == true)
	{
		::MoveWindow( GetSafeHwnd(), rc.left, rc.top,  rect.Width() + nDlgFrmX*2, 
					nCapHeight + nDlgFrmY*2 + rect.Height(), TRUE );
	}
	else
	{
		::MoveWindow( GetSafeHwnd(), rc.left + 10, rc.top + 10,  rect.Width() + nDlgFrmX*2, 
					nCapHeight + nDlgFrmY*2 + rect.Height(), TRUE );
	}
	
}

void CNewSMS::InitString(void)
{
	TCHAR szTmp[MAX_PATH];
	
	if(m_nReply == SMSRpely)
	{
		if (m_bIsSend)
		{
		al_GetSettingString(_T("public"), _T("IDS_SEND"), theApp.m_szRes, szTmp);	// Title

		}
		else
		al_GetSettingString(_T("public"), _T("IDS_REPLY"), theApp.m_szRes, szTmp);	// Title
	}
	else
		al_GetSettingString(_T("public"), _T("IDS_NEWSMS"), theApp.m_szRes, szTmp);	// Title
	SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_RECEIVER"), theApp.m_szRes, szTmp);
	m_ST_Receiver.SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_CONTENTS"), theApp.m_szRes, szTmp);
	m_ST_MsgContent.SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_CONTACTS"), theApp.m_szRes, szTmp);
	m_bt_Contacts.SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_SEND"), theApp.m_szRes, szTmp);
	m_BTN_Send.SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_SAVE2DRAFT"), theApp.m_szRes, szTmp);
	m_BTN_Save2Draft.SetWindowText(szTmp);

	al_GetSettingString(_T("public"), _T("IDS_CANCEL"), theApp.m_szRes, szTmp);
	m_BTN_Cancel.SetWindowText(szTmp);

}

void CNewSMS::InitValue()
{
	if (m_bIsSend)
	m_EDIT_Contents.SetWindowText(m_strContent);//081031libaoliu
	m_EDIT_ContactsList.SetWindowText(m_strPhone);


	m_EDIT_Contents.SetLimitText(MAX_CONTENT); 
	m_EDIT_ContactsList.SetLimitText(MAX_CONTENT); 


	//m_EDIT_Contents

	OnChangeEditContents();
}

void CNewSMS::OnPaint() 
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

void CNewSMS::GetPhoneNumberList(void)
{
//	char strPhoneList[MAX_CONTENT+1];
	CString strPhoneList;
//	memset(strPhoneList, 0, MAX_CONTENT+1);
//	int nLength = m_EDIT_ContactsList.GetWindowTextLength();
//	m_EDIT_ContactsList.GetWindowText(strPhoneList, nLength+1);
	m_EDIT_ContactsList.GetWindowText(strPhoneList);
/*	int nNext = 0;
	m_aryPhoneList[0] = _tcstok(strPhoneList.GetBuffer(MAX_PATH), ";");
	while(m_aryPhoneList[nNext].GetLength() != 0)
	{
		nNext++;
		m_aryPhoneList[nNext] = _tcstok(NULL, ";");
	}
*/
	m_aryPhoneList.RemoveAll();
	TCHAR *PhoneNumber = _tcstok(strPhoneList.GetBuffer(MAX_PATH), _T(";"));
	while(PhoneNumber)
	{
		if(_tcslen(PhoneNumber) >0)
		{
			CString str;
			str.Format(_T("%s"),PhoneNumber);
			m_aryPhoneList.AddTail(str);
		}
		PhoneNumber = _tcstok(NULL, _T(";"));
		if(PhoneNumber == NULL)	
			break;
	}
	strPhoneList.ReleaseBuffer();
}

void CNewSMS::OnBtnContacts() 
{
	// TODO: Add your control notification handler code here
	// Get PhoneBook From Mobile


	// Send Data to show Phone datas on the Dlg.
//	ContactList	pContactInfo[600];
//	memset(&pContactInfo, 0, sizeof(ContactList)*600);

	CContactsDlg ContactsDlg;
//	ContactsDlg.SetData(pContactInfo);
	
	// Get PhoneList from Edit Control
	GetPhoneNumberList();
	ContactsDlg.SetPhoneNumer(&m_aryPhoneList);

	int ret = ContactsDlg.DoModal();
	if(ret == IDOK)
	{
		CString ShowPhoneList;
	/*	int nNext = 0;
		while(m_aryPhoneList[nNext].GetLength() != 0)
		{
			ShowPhoneList += m_aryPhoneList[nNext];
			ShowPhoneList += ";";
			nNext++;
		}*/
		POSITION pos  = m_aryPhoneList.GetHeadPosition();
		while(pos)
		{
			CString strNo = m_aryPhoneList.GetNext(pos);
			ShowPhoneList += strNo;
			ShowPhoneList += _T(";");

		}
		m_EDIT_ContactsList.SetWindowText((LPCTSTR)ShowPhoneList);
	}
	else if(ret == IDCANCEL)
	{

	}

}


int CNewSMS::UnicodeString(CString pstrString)
{
#ifdef _UNICODE
	return pstrString.GetLength();
#else
	USES_CONVERSION;
	char *str = pstrString.GetBuffer(MAX_PATH);
	WCHAR* wstr = A2W(str);
	return wcslen(wstr);
#endif
}

int CNewSMS::InitializeFromString(LPCSTR pstrString)
{
	if(pstrString == NULL)
		return true;

	//ASSERT(IsValidString(pstrString));
	int nLength = MultiByteToWideChar(CP_ACP, 0, pstrString, strlen(pstrString), NULL, 0);
	int nMaximumLength = nLength;
	int bufferSize = (nLength + 1) * sizeof(WCHAR);

	LPWSTR strBuffer = NULL;
	strBuffer = (LPWSTR)malloc(bufferSize);
	memset(strBuffer, 0, bufferSize);
	int lengthWrite = MultiByteToWideChar(CP_ACP, 0, pstrString, strlen(pstrString), (unsigned short *)strBuffer, bufferSize);
	strBuffer[nLength] = L'\0';
	if(strBuffer)
	{
		free(strBuffer);
		return lengthWrite;
	}
	return false;
}
/*
void CNewSMS::MultiByteCut(LPCSTR lpszSrc, LPCSTR lpszDes, int nPage)
{
	DWORD dwszLen = MultiByteToWideChar(CP_ACP, 0, lpszSrc, strlen(lpszSrc), NULL, 0);
	LPWSTR lpwsz = (LPWSTR)malloc((dwszLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, lpszSrc, strlen(lpszSrc), lpwsz, dwszLen);
	memset((void*)lpszDes, 0, MAX_CONTENT+1);
	lpwsz[dwszLen] = L'\0';

	if(dwszLen > MULTIBYTE)
	{
		//int nCutWords = dwszLen - MULTIBYTE*(nPage-1);

		LPWSTR lpwszTmp[255];
		memset((void*)lpwszTmp, 0, 255);
		memcpy(lpwszTmp, lpwsz+(MULTIBYTE*(nPage-1)), MULTIBYTE*2);
		WideCharToMultiByte(CP_ACP, 0, (const unsigned short*)lpwszTmp, dwszLen, (char*)lpszDes, MULTIBYTE*2, NULL, NULL);

		/*if(nCutWords > MULTIBYTE)
			WideCharToMultiByte(CP_ACP, 0, lpwsz+(MULTIBYTE*(nPage-1)), dwszLen, (char*)lpszDes, MULTIBYTE*2, NULL, NULL);
		else
			WideCharToMultiByte(CP_ACP, 0, lpwsz+(MULTIBYTE*(nPage-1)), dwszLen, (char*)lpszDes, nCutWords*2, NULL, NULL);
			*/
/**	}
	else
	{
		WideCharToMultiByte(CP_ACP, 0, lpwsz, dwszLen, (char*)lpszDes, dwszLen*2, NULL, NULL);
	}
	
	free(lpwsz);
}

void CNewSMS::SingleByteCut(LPCSTR lpszSrc, LPCSTR lpszDes, int nPage)
{
	int nLength = m_EDIT_Contents.GetWindowTextLength();

	if(nLength > SINGLEBYTE)
	{
		memcpy((void *)lpszDes, lpszSrc+(SINGLEBYTE*(nPage-1)), SINGLEBYTE);
	}
	else
	{
		memcpy((void *)lpszDes, lpszSrc, nLength);
	}


}
*/

/*int CNewSMS::HasHighByte(LPCTSTR szString) 
{
	int nLen = strlen(szString);
	for( int i=0; i<nLen; i++ )	
		if( (BYTE)szString[i] > 0x7f )
			return true;

	return false;

}*/

int CNewSMS::HasHighByte(CString pstrString) 
{
	if(pstrString.GetLength()<=0)
		return false;
	unsigned char *pChar;
	int nLength  = 0;
#ifdef _UNICODE
	nLength = pstrString.GetLength();
	pChar = (unsigned char *)pstrString.GetBuffer(MAX_PATH);
#else
	USES_CONVERSION;
	char *str = pstrString.GetBuffer(MAX_PATH);
	WCHAR* wstr = A2W(str);
	nLength = wcslen(wstr);
	pChar = (unsigned char *)wstr;
#endif


	for(int i = 0; i < nLength*2; i++)
	{
		if(pChar[i] != 0 && pChar[i+1] != 0)
		{
			pstrString.ReleaseBuffer();
			return true;
		}
	}

	pstrString.ReleaseBuffer();
	return false;


}

#define BufferSize	20
void CNewSMS::OnChangeEditContents() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	int nLength = m_EDIT_Contents.GetWindowTextLength();
	int nPhoneLength = m_EDIT_ContactsList.GetWindowTextLength();
	
	if(nLength != 0 && nPhoneLength != 0)
	{
		if(theApp.nIsConnected == true)
		{
			m_BTN_Send.EnableWindow(true);
			m_bt_Contacts.EnableWindow(true);
		}
		else
		{
			m_BTN_Send.EnableWindow(false);
			m_bt_Contacts.EnableWindow(false);
		}

		m_BTN_Save2Draft.EnableWindow(true);
	}
	else
	{
		//081031libaoliu
		if (nPhoneLength != 0 && theApp.nIsConnected == true)
		{
			m_BTN_Send.EnableWindow(true);
		}
		else
		m_BTN_Send.EnableWindow(false);
		m_BTN_Save2Draft.EnableWindow(false);

		if(theApp.nIsConnected == true)				// 2005.02.23
			m_bt_Contacts.EnableWindow(true);		// 2005.02.23
		else										// 2005.02.23
			m_bt_Contacts.EnableWindow(false);		// 2005.02.23

	}

	memset(m_strGetMsg, 0, (MAX_CONTENT+1)*sizeof(TCHAR));
	m_EDIT_Contents.GetWindowText(m_strGetMsg, nLength+1);

	// Remove All line Feed(\n)
	/*CString strRemove = m_strGetMsg;
	strRemove.Remove(0x0a);
	nLength = strRemove.GetLength();
	memset(m_strGetMsg, 0, BufferSize);
	memcpy(m_strGetMsg, strRemove.GetBuffer(nLength), nLength);
	strRemove.ReleaseBuffer();*/
	

	TCHAR strCount[BufferSize];
	memset(strCount, 0, BufferSize*sizeof(TCHAR));
	if(HasHighByte(m_strGetMsg))
	{
		m_bMultiByte = true;

		int nWideCharLength = UnicodeString(m_strGetMsg);

		m_nPage = ((nWideCharLength-1) / MULTIBYTE) + 1;
		
		if(nWideCharLength == 0)
			wsprintf(strCount, _T("[%d / %d]"), 1, MULTIBYTE);
		else if((nWideCharLength % MULTIBYTE) == 0)
			wsprintf(strCount, _T("[%d / %d]"), m_nPage+1, MULTIBYTE);
		else
			wsprintf(strCount, _T("[%d / %d]"), m_nPage, MULTIBYTE*(m_nPage) - nWideCharLength);

		m_ST_Counts.SetWindowText(strCount);
	}
	else
	{
		m_bMultiByte = false;

		m_nPage = ((nLength-1) / SINGLEBYTE) + 1;

		if(nLength == 0)
			wsprintf(strCount, _T("[%d / %d]"), 1, SINGLEBYTE);
		else if((nLength % SINGLEBYTE) == 0)
			wsprintf(strCount, _T("[%d / %d]"), m_nPage+1, SINGLEBYTE);
		else
			wsprintf(strCount, _T("[%d / %d]"), m_nPage, SINGLEBYTE*(m_nPage) - nLength);

		m_ST_Counts.SetWindowText(strCount);
	}
	
	// TODO: Add your control notification handler code here
	
}


void CNewSMS::OnBtnSend() 
{
	// TODO: Add your control notification handler code here
	// If one of Message or Telephone has blank content, return and show a message, "lost data".
	CString str;

	int nLength = m_EDIT_Contents.GetWindowTextLength();
	if(nLength == 0)
		//return;081031libaoliu

	nLength = m_EDIT_ContactsList.GetWindowTextLength();
	if(nLength == 0)
		return;
	
	TCHAR str1[MAX_PATH], str2[MAX_PATH];
	al_GetSettingString(_T("public"), _T("IDS_PAGE"), theApp.m_szRes, str1);	// Title
	wsprintf(str1, str1, m_nPage);

	al_GetSettingString(_T("public"), _T("IDS_SEND_DIA"), theApp.m_szRes, str2);
	int ret = MessageBox(str1, str2, MB_YESNO);


	SMS_PARAM msgInfo;
	TCHAR strGetMsg[MAX_CONTENT+1], strSaveMsg[MAX_CONTENT+1];
	CString strPhoneList;
	memset(strGetMsg, 0, MAX_CONTENT+1);

	CLeftView *saveTree = ((CMainFrame*)GetParentFrame())->GetLeftPane();

	if(ret == IDYES)
	{
		m_EDIT_Contents.GetWindowText(strGetMsg, m_EDIT_Contents.GetWindowTextLength()+1);
		m_EDIT_ContactsList.GetWindowText(strPhoneList);
		// Send SMS
		int sendRet = saveTree->xSendSMS(strGetMsg, strPhoneList, m_nPage, m_bMultiByte); // Record Send Ok or Not..
			
		
		// Save SMS to Backup Folder
		memset(&msgInfo, 0, sizeof(SMS_PARAM));

		m_EDIT_Contents.GetWindowText(strSaveMsg, m_EDIT_Contents.GetWindowTextLength()+1);
		_tcscpy(msgInfo.TP_UD, strSaveMsg);

		m_EDIT_ContactsList.GetWindowText(strSaveMsg, m_EDIT_ContactsList.GetWindowTextLength()+1);

	//	saveTree->xSaveSMSToBackUpFolder(msgInfo, strSaveMsg); 

		CDialog::OnOK();
	}
	
}

/*void CNewSMS::OnBtnSend() 
{
	// TODO: Add your control notification handler code here
	// If one of Message or Telephone has blank content, return and show a message, "lost data".
	CString str;

	int nLength = m_EDIT_Contents.GetWindowTextLength();
	if(nLength == 0)
		return;

	nLength = m_EDIT_ContactsList.GetWindowTextLength();
	if(nLength == 0)
		return;
	

	TCHAR str1[MAX_PATH], str2[MAX_PATH];
	al_GetSettingString(_T("public"), "IDS_PAGE", theApp.m_szRes, str1);	// Title
	sprintf(str1, str1, m_nPage);
	//str.Format(IDS_PAGE, m_nPage);
	//int ret = AfxMessageBox(str, MB_YESNO);

	al_GetSettingString(_T("public"), "IDS_SEND_DIA", theApp.m_szRes, str2);
	int ret = MessageBox(str1, str2, MB_YESNO);


	SMS_PARAM msgInfo;
	char strGetMsg[gMsgCount], strSendMsg[255], strSaveMsg[gMsgCount], strPhoneList[gPhoneListSize];
	memset(strGetMsg, 0, gMsgCount);

	CLeftView *saveTree = ((CMainFrame*)GetParentFrame())->GetLeftPane();

	if(ret == IDYES)
	{
		for(int nSendPage = 1; nSendPage <= m_nPage; nSendPage++)
		{
			m_EDIT_Contents.GetWindowText(strGetMsg, m_EDIT_Contents.GetWindowTextLength()+1);
			//m_EDIT_Contents.GetWindowText(strGetMsg, m_EDIT_Contents.GetWindowTextLength()+1);
			memset(strSendMsg, 0, 255);

			memset(&msgInfo, 0, sizeof(SMS_PARAM));

			if(m_bMultiByte)
			{
				MultiByteCut(strGetMsg, strSendMsg, nSendPage);
				msgInfo.TP_DCS = 1; // Unicode
			}
			else
			{
				SingleByteCut(strGetMsg, strSendMsg, nSendPage);
				msgInfo.TP_DCS = 2; // Default
			}

			// Send SMS
			strcpy(msgInfo.TP_UD, strSendMsg);
			m_EDIT_ContactsList.GetWindowText(strPhoneList, m_EDIT_ContactsList.GetWindowTextLength()+1);
			int sendRet = saveTree->xSendSMS(msgInfo, strPhoneList); // Record Send Ok or Not..
			
		}

		// Save SMS to Backup Folder
		memset(&msgInfo, 0, sizeof(SMS_PARAM));

		m_EDIT_Contents.GetWindowText(strSaveMsg, m_EDIT_Contents.GetWindowTextLength()+1);
		strcpy(msgInfo.TP_UD, strSaveMsg);

		m_EDIT_ContactsList.GetWindowText(strSaveMsg, m_EDIT_ContactsList.GetWindowTextLength()+1);
		//strcpy(msgInfo.TPA, strSaveMsg);

		saveTree->xSaveSMSToBackUpFolder(msgInfo, strSaveMsg); 

		CDialog::OnOK();
	}
	
}*/

void CNewSMS::SetData(SMS_PARAM msgInfo, int nReply, bool bFirstCall,bool bIsSend)
{
	m_strContent = msgInfo.TP_UD;
	m_strPhone = msgInfo.TPA;
	m_nReply = nReply;
	m_bFirstCall = bFirstCall;
	m_bIsSend=bIsSend;
}

void CNewSMS::OnChangeEditContactslist() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	int nLength = m_EDIT_Contents.GetWindowTextLength();
	int nPhoneLength = m_EDIT_ContactsList.GetWindowTextLength();
	
	if(nLength != 0 && nPhoneLength != 0)
	{
		if(theApp.nIsConnected == true)
		{
			m_BTN_Send.EnableWindow(true);
			m_bt_Contacts.EnableWindow(true);
		}
		else
		{
			m_BTN_Send.EnableWindow(false);
			m_bt_Contacts.EnableWindow(false);
		}

		m_BTN_Save2Draft.EnableWindow(true);
	}
	else
	{
		if (nPhoneLength != 0 && theApp.nIsConnected == true)
		{
			m_BTN_Send.EnableWindow(true);
		}
		else
		m_BTN_Send.EnableWindow(false);
		m_BTN_Save2Draft.EnableWindow(false);
		
		if(theApp.nIsConnected == true)				// 2005.02.23
			m_bt_Contacts.EnableWindow(true);		// 2005.02.23
		else										// 2005.02.23
			m_bt_Contacts.EnableWindow(false);		// 2005.02.23
	}
	
	// TODO: Add your control notification handler code here
	
}

void CNewSMS::OnBtnDraft() 
{
	// TODO: Add your control notification handler code here
	CString str;

	int nLength = m_EDIT_Contents.GetWindowTextLength();
	if(nLength == 0)
		return;

	nLength = m_EDIT_ContactsList.GetWindowTextLength();
	if(nLength == 0)
		return;


//	SMS_PARAM msgInfo;
	TCHAR strGetMsg[MAX_CONTENT*10+1];//,strPhoneList[MAX_CONTENT*10+1];
	CString strPhoneList;
	memset(strGetMsg, 0, MAX_CONTENT*10+1);
//	memset(strPhoneList, 0, MAX_CONTENT*10+1);
	strPhoneList.Empty();

	CLeftView *saveTree = ((CMainFrame*)GetParentFrame())->GetLeftPane();


	// Save a copy to PC's Draft Folder
//	memset(&msgInfo, 0, sizeof(SMS_PARAM));

	m_EDIT_Contents.GetWindowText(strGetMsg, m_EDIT_Contents.GetWindowTextLength()+1);
//	strcpy(msgInfo.TP_UD, strGetMsg);

//	m_EDIT_ContactsList.GetWindowText(strPhoneList, m_EDIT_ContactsList.GetWindowTextLength()+1);
	m_EDIT_ContactsList.GetWindowText(strPhoneList);
	//strcpy(msgInfo.TPA, strGetMsg);

	saveTree->xSaveSMSToDraftFolder(strGetMsg, strPhoneList, m_nPage, m_bMultiByte); 

	CDialog::OnOK();
}

void CNewSMS::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	/*if( (nChar < 0x30 && nChar > 0x39) && nChar != ';')
		return;*/
	
	CDialog::OnChar(nChar, nRepCnt, nFlags);
}

void CNewSMS::OnMaxtextEditContactslist() 
{
	// TODO: Add your control notification handler code here
	
}
