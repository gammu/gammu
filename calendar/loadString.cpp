#include "stdafx.h"
#include "Calendar.h"
#include "loadstring.h"
CString  LoadStringFromFile(CString strSection, CString strKey,CString strFilename)
{
	if(strFilename.IsEmpty())
	{
		CCalendarApp* pApp=(CCalendarApp*)afxGetApp();

		strFilename.Format(_T("%s"),pApp->m_szRes);
	}

	if(strFilename.IsEmpty() || strSection.IsEmpty() || strKey.IsEmpty()) return _T("");

	TCHAR szSection[MAX_PATH];
	TCHAR szKey[MAX_PATH];
	TCHAR szFileName[MAX_PATH];
	TCHAR szStr[MAX_PATH*5];
	wsprintf(szSection,_T("%s"),strSection);
	wsprintf(szKey,_T("%s"),strKey);
	wsprintf(szFileName,_T("%s"),strFilename);

	::GetPrivateProfileString(szSection, szKey, _T(""), szStr, MAX_PATH*5, szFileName);
	CString strTemp,strTemp2;
	strTemp=szStr;
	strTemp2=szStr;
	int n,m;
	n=strTemp.Find(_T("\\"));
	m=n;
	do
	{
		if(m!=-1 && n<strTemp.GetLength())
		{
			if(strTemp.GetAt(n+1)=='t')
			{
				strTemp=strTemp.Left(n)+CString(_T("\t"))+strTemp.Right(strTemp.GetLength()-n-2);
			}
			else if(strTemp.GetAt(n+1)=='n')
			{
				strTemp=strTemp.Left(n)+CString(_T("\n"))+strTemp.Right(strTemp.GetLength()-n-2);
			}
			strTemp2=strTemp.Right(strTemp.GetLength()-n-1);
		}
		m=strTemp2.Find(_T("\\"));
		n=(strTemp.GetLength()-strTemp2.GetLength())+m;
	}while(m!=-1);
	return strTemp;

}
int CalendarStringMessageBox(HWND hWnd, LPCTSTR lpMessageText,UINT uType )
{
	CString strTitle = LoadStringFromFile(_T("Message"),_T("MESSAGE_TITLE"));
	return ::MessageBox(hWnd, lpMessageText, strTitle,uType);
}
int CalendarMessageBox(HWND hWnd, LPCTSTR lpSectionText,UINT uType)
{
	CString strMsg = LoadStringFromFile(_T("Message"),lpSectionText);
	CString strTitle = LoadStringFromFile(_T("Message"),_T("MESSAGE_TITLE"));
	return ::MessageBox(hWnd, strMsg, strTitle,uType);
}
